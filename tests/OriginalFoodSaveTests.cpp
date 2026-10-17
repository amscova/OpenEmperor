#include "OriginalProductionFixture.h"
#include "simulation/original/GoodsRules.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <tuple>

namespace {
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace save=openemperor::persistence;
namespace orig=openemperor::original;
using json=nlohmann::json;
using f::check;
std::size_t rejected=0;
template<class F> void refuses(F action) {
    try {action();} catch (const std::exception&) {++rejected;return;}
    throw std::runtime_error("Malformed Food save authority was admitted");
}
struct Fixture {
    f::Root root;
    std::shared_ptr<const sim::MapPermissions> policy=f::permissions();
    std::shared_ptr<const orig::OriginalRulesModel> model=[&] {
        oe_production_fixture::write_model(root.data,7);
        return save::prepare_original_production_rules_model(root.data);
    }();
    sim::World world=sim::World::create_original_production_development(policy,model);
    f::fs::path target=root.path/"food-schema26.json";
    sim::BuildingId wheat{},millet{},market{};
    Fixture() {
        for (const int y:{4,8}) for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,y}});
        for (int x=0;x<18;x+=3) f::buy(world,{sim::CommandType::PlaceHousehold,{x,2}});
        f::buy(world,{sim::CommandType::PlaceOriginalFoodFarm,{0,9}});
        f::buy(world,{sim::CommandType::PlaceOriginalFoodFarm,{4,9}});
        f::buy(world,{sim::CommandType::PlaceMarket,{12,9}});
        wheat=*world.building_owner_at({0,9});millet=*world.building_owner_at({4,9});market=*world.building_owner_at({12,9});
        check(world.construction_spent_total()==876 && world.total_population()==0,
              "Food scene did not pay for separate networks, initially empty Houses and real3x3 Farms");
    }
    save::SaveDocument document(const sim::World& value) const {
        return save::make_document(root.data,f::map_relative,policy->building_mask(),value);
    }
    sim::World restore(const save::SaveDocument& d) const {
        return save::restore_original_production_development(d,root.data,policy->building_mask(),policy);
    }
    std::string roundtrip(const sim::World& value) const {
        const auto before=value.snapshot();const auto d=document(value);
        save::write_save(target,d,root.data,policy->building_mask());const auto bytes=f::read_text(target);
        const auto parsed=save::read_save(target);f::same_document(d,parsed);
        check(parsed.source_schema_version==26 && parsed.world.original_production->development_revision==24,
              "Food save lost its explicit revision24 identity");
        auto copy=restore(parsed);check(copy.snapshot()==before,"Food F5/F9 changed the complete snapshot");
        save::write_save(target,document(copy),root.data,policy->building_mask());
        check(f::read_text(target)==bytes && value.snapshot()==before,"Food save rewrite changed bytes or World");
        return bytes;
    }
    void replay(const sim::World& value) const {
        roundtrip(value);auto expected=sim::World::restore_original_production_development(value.snapshot(),policy,model);
        auto copy=restore(save::read_save(target));
        for (int i=0;i<20000;++i) {expected.tick();copy.tick();}
        check(expected.snapshot()==copy.snapshot(),"Actual Food midtrip/quality20k replay diverged");
    }
};
const sim::OriginalFoodStock& stock(const sim::World& world,sim::BuildingId id) {
    const auto& rows=world.original_production_state()->food.stocks;
    const auto row=std::find_if(rows.begin(),rows.end(),[&](const auto& value){return value.building_id==id;});
    check(row!=rows.end(),"Paid Food owner lost its sole stock");return *row;
}
template<class P> void until(sim::World& world,P condition,int limit=30000) {
    for (int i=0;i<limit && !condition();++i) world.tick();
    check(condition(),"Paid Food route did not reach the required actual state");
}
sim::WorldSnapshot lifecycle(Fixture& f) {
    f.roundtrip(f.world);
    for (int i=0;i<300;++i) f.world.tick();
    check(f.world.total_population()==42,"Food staffing did not use real timed migration");
    for (const auto& farm:f.world.original_production_state()->food.farms)
        check(!farm.field_id && !farm.harvested,"Food Farm created an unpaid field or harvest");
    for (const auto& [command,cell,owner]:std::array{
            std::tuple{sim::CommandType::PlaceOriginalWheatField,sim::Cell{3,9},f.wheat},
            std::tuple{sim::CommandType::PlaceOriginalMilletField,sim::Cell{7,9},f.millet}}) {
        sim::Command field{command,cell};field.building_id=owner;f::buy(f.world,field);
    }
    check(f.world.construction_spent_total()==884,"Separate Wheat/Millet fields were not paid");
    sim::CourierId actor{};
    until(f.world,[&] {
        for (const auto& c:f.world.couriers()) if (c.role==sim::CourierRole::OriginalFoodToMarket &&
                c.phase==sim::CourierPhase::ToWarehouse && c.edge_progress>0 && f.world.original_courier_cargo(c.id)>0) {
            actor=c.id;return true;
        }
        return false;
    });
    f.replay(f.world);
    auto returning=sim::World::restore_original_production_development(f.world.snapshot(),f.policy,f.model);
    const auto owner=returning.courier(actor).owner;f::buy(returning,sim::set_building_operation(owner,false));
    until(returning,[&] {
        return std::any_of(returning.original_production_state()->food.trips.begin(),returning.original_production_state()->food.trips.end(),
            [&](const auto& trip){return trip.transfer.courier_id==actor && trip.phase==sim::OriginalGoodsTransferPhase::Returning;});
    });
    f.roundtrip(returning);
    auto replay=f.restore(save::read_save(f.target));
    for (int i=0;i<1000;++i) {returning.tick();replay.tick();}
    check(returning.snapshot()==replay.snapshot(),"Loaded Food return changed across persistence");
    bool returned=false;
    for (const auto& trip:returning.original_production_state()->food.trips)
        returned=returned || (trip.transfer.courier_id==actor && trip.phase==sim::OriginalGoodsTransferPhase::Returned &&
                             trip.returned[0]+trip.returned[1]>0);
    check(returned,"Paused Food Farm did not physically receive its loaded return");
    until(f.world,[&]{const auto& bin=stock(f.world,f.market);return bin.amounts[0]>0 && bin.amounts[1]>0;});
    f::buy(f.world,sim::set_building_operation(f.wheat,false));f::buy(f.world,sim::set_building_operation(f.millet,false));
    for (const auto& h:f.world.original_economy_state()->houses)
        check(!h.house.food_quality && !stock(f.world,h.building_id).quality_witness,
              "Disconnected House acquired Food quality without a route");
    f.roundtrip(f.world);
    for (int y=5;y<8;++y) f::buy(f.world,{sim::CommandType::PlaceRoad,{23,y}});
    check(f.world.construction_spent_total()==890,"Ordinary paid road connection was not charged");
    until(f.world,[&] {
        for (const auto& trip:f.world.original_production_state()->food.trips)
            if (trip.source==f.market && trip.phase==sim::OriginalGoodsTransferPhase::Loaded &&
                    trip.loaded[0]>0 && trip.loaded[1]>0) return true;
        return false;
    });
    f.roundtrip(f.world);
    until(f.world,[&] {
        for (const auto& h:f.world.original_economy_state()->houses) {
            const auto& bin=stock(f.world,h.building_id);
            if (bin.quality_witness && bin.quality_witness->accepted[0]>0 && bin.quality_witness->accepted[1]>0 &&
                    bin.amounts[0]+bin.amounts[1]>0 && h.house.food_quality>0) return true;
        }
        return false;
    });
    const auto mixed=f.world.snapshot();f.roundtrip(f.world);f.replay(f.world);
    f::buy(f.world,sim::set_building_operation(f.wheat,true));
    until(f.world,[&] {
        for (const auto& h:f.world.original_economy_state()->houses) {
            const auto& bin=stock(f.world,h.building_id);
            if (bin.quality_witness && bin.quality_witness->accepted[0]>0 && !bin.quality_witness->accepted[1] && h.house.food_quality==20) return true;
        }
        return false;
    });f.roundtrip(f.world);
    f::buy(f.world,sim::set_building_operation(f.wheat,false));f::buy(f.world,sim::set_building_operation(f.millet,true));
    until(f.world,[&] {
        for (const auto& h:f.world.original_economy_state()->houses)
            if (h.house.food_quality==80 && stock(f.world,h.building_id).quality_witness) return true;
        return false;
    });f.roundtrip(f.world);
    return mixed;
}
void hostile(Fixture& f,const sim::WorldSnapshot& actual) {
    auto world=sim::World::restore_original_production_development(actual,f.policy,f.model);
    const auto pristine=f.roundtrip(world);const auto good=json::parse(pristine);const auto badpath=f.root.path/"bad-food.json";
    const auto reject=[&](const json& j) {
        f::write_text(badpath,j.dump());
        try {refuses([&]{(void)f.restore(save::read_save(badpath));});}
        catch (const std::exception& e) {throw std::runtime_error(std::string(e.what())+": "+json::diff(good,j).dump());}
        check(f::read_text(f.target)==pristine,"Bad Food input replaced the protected save");
    };
    const std::string base="/world/original_production/food";
    std::vector<std::string> objects;
    const std::function<void(const json&,std::string)> visit=[&](const json& j,std::string path) {
        if (j.is_object()) {objects.push_back(path);for (const auto& [key,value]:j.items()) visit(value,path+"/"+key);}
        else if (j.is_array()) for (std::size_t i=0;i<j.size();++i) visit(j[i],path+"/"+std::to_string(i));
    };visit(good.at(json::json_pointer(base)),base);
    for (const auto& path:objects) {
        auto j=good;j.at(json::json_pointer(path))["unsupported_food_authority"]=true;reject(j);
        for (const auto& [key,value]:good.at(json::json_pointer(path)).items()) {
            (void)value;j=good;j.at(json::json_pointer(path)).erase(key);reject(j);
        }
    }
    for (const auto& [path,value]:std::vector<std::pair<std::string,json>>{
            {base,nullptr},{base+"/farms",json::array()},{base+"/stocks",json::array()},{base+"/trips",json::array()},
            {base+"/farms/0/farm_id",0},{base+"/farms/0/farm_id",UINT32_MAX},
            {base+"/farms/0/field_id",nullptr},{base+"/farms/0/field_id",0},{base+"/farms/0/crop",1},{base+"/farms/0/crop",2},
            {base+"/farms/0/harvested",UINT64_MAX},{base+"/farms/0/harvested",-1},
            {base+"/stocks/0/amounts",json::array({0})},{base+"/stocks/0/amounts",json::array({0,0,0})},
            {base+"/stocks/0/building_id",UINT32_MAX},{base+"/stocks/0/missed_demands",UINT64_MAX},
            {base+"/trips/0/transfer/courier_id",UINT32_MAX},{base+"/trips/0/transfer/sequence",0},
            {base+"/trips/0/source",0},{base+"/trips/0/target",UINT32_MAX},
            {base+"/trips/0/phase",0},{base+"/trips/0/phase",4},{base+"/trips/0/phase",6},
            {base+"/trips/0/loaded",json::array({0,0})},
            {"/world/original_production/development_revision",17},{"/world/original_production/development_revision",18},{"/world/original_production/development_revision",19},{"/world/original_production/development_revision",20},{"/world/original_production/development_revision",21},{"/world/original_production/development_revision",22},{"/world/original_production/development_revision",23},{"/world/original_production/development_revision",25}}) {
        auto j=good;j.at(json::json_pointer(path))=value;reject(j);
    }
    for (const char* key:{"farms","stocks","trips"}) {
        auto j=good;auto& rows=j.at(json::json_pointer(base))[key];rows.push_back(rows[0]);reject(j);
        j=good;auto& reversed=j.at(json::json_pointer(base))[key];
        if (reversed.size()>1) {std::swap(reversed[0],reversed[1]);reject(j);}
        j=good;auto& oversized=j.at(json::json_pointer(base))[key];
        const std::size_t maximum=std::string(key)=="farms" ? 2:std::string(key)=="stocks" ? 26:6;
        while (oversized.size()<=maximum) oversized.push_back(oversized[0]);reject(j);
    }
    for (const char* key:{"amounts","received","departed","returned","consumed"}) {
        for (const json& invalid:{json(-1),json(true),json(0.0),json("0"),json(nullptr)}) {
            auto j=good;j.at(json::json_pointer(base))["stocks"][0][key][0]=invalid;reject(j);
        }
        auto j=good;j.at(json::json_pointer(base))["stocks"][0][key]=json::array({0});reject(j);
    }
    std::size_t witness=0;
    while (good.at(json::json_pointer(base))["stocks"][witness]["quality_witness"].is_null()) ++witness;
    const auto witness_base=base+"/stocks/"+std::to_string(witness)+"/quality_witness";
    for (const auto& [path,value]:std::vector<std::pair<std::string,json>>{
            {witness_base,nullptr},{witness_base+"/transfer/courier_id",0},{witness_base+"/transfer/sequence",UINT64_MAX},
            {witness_base+"/tick",UINT64_MAX},{witness_base+"/previous_quality",256},
            {witness_base+"/tick",good["world"]["ticks"].get<std::uint64_t>()+1},
            {witness_base+"/transfer/sequence",good.at(json::json_pointer(witness_base))["transfer"]["sequence"].get<std::uint64_t>()+1},
            {witness_base+"/previous_quality",20},{witness_base+"/accepted",json::array({0,0})},
            {witness_base+"/accepted",json::array({1})},{witness_base+"/consumed_before",json::array({UINT64_MAX,UINT64_MAX})}}) {
        auto j=good;j.at(json::json_pointer(path))=value;reject(j);
    }
    const auto house=good.at(json::json_pointer(base))["stocks"][witness]["building_id"];
    for (std::size_t i=0;i<good["world"]["original_economy"]["houses"].size();++i)
        if (good["world"]["original_economy"]["houses"][i]["building_id"]==house) {
            auto j=good;j["world"]["original_economy"]["houses"][i]["house"]["food_quality"]=20;reject(j);
        }
    // Field and Farm coordinates must be rejected before signed footprint math.
    for (std::size_t i=0;i<good["world"]["buildings"].size();++i) {
        const auto kind=good["world"]["buildings"][i]["kind"].get<unsigned>();
        if (kind>=static_cast<unsigned>(sim::Object::OriginalFoodFarm))
            for (const int coordinate:{std::numeric_limits<int>::min(),std::numeric_limits<int>::max()}) {
                auto j=good;j["world"]["buildings"][i]["cell"]=json::array({coordinate,coordinate});reject(j);
            }
    }
    const auto doc=f.document(world);
    for (int fault=0;fault<5;++fault) {
        auto bad=doc;
        if (fault==0) bad.world.original_production->food.stocks.clear();
        if (fault==1) ++bad.world.original_production->food.farms[0].harvested;
        if (fault==2) bad.world.original_production->food.trips[0].transfer.sequence=UINT64_MAX;
        if (fault==3) bad.world.original_production->development_revision=17;
        if (fault==4) bad.world.original_production->food.stocks[witness].quality_witness.reset();
        refuses([&]{save::write_save(f.target,bad,f.root.data,f.policy->building_mask());});
        check(f::read_text(f.target)==pristine,"Bad Food writer replaced valid data");
    }
    auto duplicate=pristine;const auto marker=duplicate.find("\"quality_witness\": {");
    check(marker!=std::string::npos,"Actual quality witness duplicate-key probe absent");
    duplicate.insert(duplicate.find('{',marker)+1,"\"tick\":0,");f::write_text(badpath,duplicate);
    refuses([&]{(void)save::read_save(badpath);});
    auto bad=doc;++bad.world.construction_spent_total;--bad.world.treasury;
    sim::World::validate_original_production_snapshot(bad.world);
    refuses([&]{(void)f.restore(bad);});
    refuses([&]{save::write_save(f.target,bad,f.root.data,f.policy->building_mask());});
}
}
int main() {
    try {Fixture f;const auto actual=lifecycle(f);hostile(f,actual);
        std::cout<<"Food save PASS: paid2-crop chain, actual mixed quality/20/80 arrivals, midtrip/return/witness20k persistence and "
                 <<rejected<<" hostile refusals\n";return 0;}
    catch (const std::exception& e) {std::cerr<<"Original Food save FAIL: "<<e.what()<<'\n';return 1;}
}
