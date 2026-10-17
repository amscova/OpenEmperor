#include "OriginalProductionFixture.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>

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
    throw std::runtime_error("Malformed Hemp source or save was admitted");
}
struct Fixture {
    f::Root root;
    std::shared_ptr<const sim::MapPermissions> policy=f::permissions();
    std::shared_ptr<const orig::OriginalRulesModel> model=[&] {
        oe_production_fixture::write_model(root.data,7);
        return save::prepare_original_production_rules_model(root.data);
    }();
    sim::World world=sim::World::create_original_production_development(policy,model);
    f::fs::path target=root.path/"hemp-schema26.json";
    sim::BuildingId farm{},field{};
    Fixture() {
        for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,8}});
        for (int x=0;x<18;x+=3) f::buy(world,{sim::CommandType::PlaceHousehold,{x,6}});
        f::buy(world,{sim::CommandType::PlaceClaySource,{0,9}});
        f::buy(world,{sim::CommandType::PlacePottery,{8,9}});
        f::buy(world,{sim::CommandType::PlaceWarehouse,{16,9}});
        f::buy(world,{sim::CommandType::PlaceMarket,{21,9}});
        f::buy(world,{sim::CommandType::PlaceOriginalHempFarm,{4,9}});
        farm=*world.building_owner_at({4,9});
        check(world.construction_spent_total()==1158 && world.treasury()==10842,
              "Hemp Farm, six Houses and the complete existing chain were not paid");
    }
    save::SaveDocument document(const sim::World& value) const {
        return save::make_document(root.data,f::map_relative,policy->building_mask(),value);
    }
    sim::World restore(const save::SaveDocument& d,
            std::shared_ptr<const orig::OriginalRulesModel> prepared=nullptr) const {
        return save::restore_original_production_development(d,root.data,policy->building_mask(),policy,std::move(prepared));
    }
    std::string roundtrip() {
        const auto before=world.snapshot();const auto d=document(world);
        save::write_save(target,d,root.data,policy->building_mask());const auto bytes=f::read_text(target);
        const auto parsed=save::read_save(target);f::same_document(d,parsed);
        check(parsed.source_schema_version==26 && parsed.world.original_production->development_revision==22,
              "Hemp save did not require its explicit development revision");
        auto restored=restore(parsed);check(restored.snapshot()==before,"Full Hemp save/restore changed authority");
        check(restored.original_rules_model()->supports_building(orig::ModelBuilding::HempFarm) &&
              restored.original_rules_model()->supports_building(orig::ModelBuilding::HempField),
              "F9 used the frozen seven-row preparation path for active production");
        save::write_save(target,document(restored),root.data,policy->building_mask());
        check(f::read_text(target)==bytes && world.snapshot()==before,"Hemp canonical rewrite mutated state or bytes");
        return bytes;
    }
};
void conserved(const sim::World& world) {
    check(world.production_balance_valid() && world.navigation_valid() && world.population_valid() && world.city_economy_valid(),
          "Paid two-good World invariant failed");
    std::uint64_t harvested=0;
    for (const auto& farm:world.original_production_state()->hemp_farms) harvested+=farm.harvested;
    for (std::size_t good=0;good<orig::goods_count;++good) {
        const auto b=world.original_goods_balance(static_cast<orig::Goods>(good));
        check(b.conserved && b.stock+b.cargo==b.totals.produced && b.supported==(good<2) &&
              !b.totals.imported && !b.totals.consumed && !b.totals.demolished,
              "Two-good stock/cargo/history conservation or capability failed");
        if (good==0) check(b.totals.produced==harvested,"Hemp production lacks its actual Farm harvest origin");
    }
}
template<class P> void until(sim::World& world,P condition,int limit=20000) {
    for (int tick=0;tick<limit && !condition();++tick) {world.tick();conserved(world);}
    check(condition(),"Paid Hemp chain did not reach the required actual state");
}
sim::WorldSnapshot lifecycle(Fixture& f) {
    f.roundtrip();check(!f.world.original_production_state()->hemp_farms[0].field_id,
                        "Farm purchase created an unpaid field");
    for (int tick=0;tick<300;++tick) f.world.tick();
    check(f.world.total_population()==42 && f.world.original_goods_balance(orig::Goods::Hemp).totals.produced==0,
          "Missing paid field did not prevent production or genuine immigration failed");
    sim::Command field{sim::CommandType::PlaceOriginalHempField,{6,9}};field.building_id=f.farm;f::buy(f.world,field);
    f.field=*f.world.building_owner_at({6,9});
    check(f.world.construction_spent_total()==1162 && f.world.treasury()==10838 &&
          f.world.original_production_state()->hemp_farms[0].field_id==f.field,"Separate Field purchase cost or owner binding failed");
    f::buy(f.world,sim::set_building_operation(f.farm,false));
    for (int tick=0;tick<200;++tick) f.world.tick();
    check(f.world.original_goods_balance(orig::Goods::Hemp).totals.produced==0,"Paused paid Farm produced Hemp");
    f.roundtrip();f::buy(f.world,sim::set_building_operation(f.farm,true));
    until(f.world,[&] {
        for (const auto& c:f.world.couriers()) if (c.role==sim::CourierRole::OriginalHempToWarehouse &&
                c.phase==sim::CourierPhase::ToWarehouse && c.edge_progress>0 && f.world.original_courier_cargo(c.id)>0) return true;
        return false;
    });
    const auto midroute=f.world.snapshot();f.roundtrip();auto replay=f.restore(save::read_save(f.target));
    for (int tick=0;tick<20000;++tick) {f.world.tick();replay.tick();}
    check(f.world.snapshot()==replay.snapshot(),"Actual mid-Hemp-courier 20k persistence replay diverged");
    until(f.world,[&] {
        for (const auto& h:f.world.original_economy_state()->houses) if (h.house.hemp>0 && h.house.ceramics>0) return true;
        return false;
    });
    f.roundtrip();conserved(f.world);
    const auto frozen_model=save::prepare_original_rules_model(f.root.data);
    check(!frozen_model->supports_building(orig::ModelBuilding::HempFarm),"Frozen preparation unexpectedly gained Hemp capability");
    refuses([&]{(void)f.restore(f.document(f.world),frozen_model);});
    auto returning=sim::World::restore_original_production_development(midroute,f.policy,f.model);
    f::buy(returning,sim::set_building_operation(f.farm,false));
    until(returning,[&] {
        for (const auto& row:returning.original_goods_state()->reservations)
            if (row.source==f.farm && row.phase==sim::OriginalGoodsTransferPhase::Returning) return true;
        return false;
    });
    save::write_save(f.target,f.document(returning),f.root.data,f.policy->building_mask());
    auto copy=f.restore(save::read_save(f.target));
    for (int tick=0;tick<1000;++tick) {returning.tick();copy.tick();}
    check(returning.snapshot()==copy.snapshot(),"Paused loaded Hemp return replay diverged");
    bool returned=false;
    for (const auto& row:returning.original_goods_state()->reservations)
        returned=returned || (row.source==f.farm && row.phase==sim::OriginalGoodsTransferPhase::Returned && row.returned>0);
    check(returned,"Actual Hemp cargo did not return to its protected Farm buffer");conserved(returning);
    return midroute;
}
void adversarial(Fixture& f,const sim::WorldSnapshot& captured) {
    f.world=sim::World::restore_original_production_development(captured,f.policy,f.model);
    const auto pristine=f.roundtrip();const auto good=json::parse(pristine);const auto hostile=f.root.path/"hostile-hemp.json";
    const auto reject=[&](const json& value) {
        f::write_text(hostile,value.dump());
        try {refuses([&]{(void)f.restore(save::read_save(hostile));});}
        catch (const std::exception& error) {throw std::runtime_error(std::string(error.what())+": "+json::diff(good,value).dump());}
        check(f::read_text(f.target)==pristine,"Malformed Hemp input changed protected target");
    };
    std::vector<std::string> objects;
    const std::function<void(const json&,std::string)> visit=[&](const json& j,std::string path) {
        if (j.is_object()) {objects.push_back(path);for (const auto& [key,value]:j.items()) visit(value,path+"/"+key);}
        else if (j.is_array()) for (std::size_t i=0;i<j.size();++i) visit(j[i],path+"/"+std::to_string(i));
    };visit(good,"");
    for (const auto& path:objects) {
        auto j=good;j.at(json::json_pointer(path))["unsupported_extra_authority"]=true;reject(j);
        for (const auto& [key,value]:good.at(json::json_pointer(path)).items()) {
            (void)value;j=good;j.at(json::json_pointer(path)).erase(key);reject(j);
        }
    }
    for (const auto& [path,value]:std::vector<std::pair<std::string,json>>{
            {"/world/original_production/development_revision",15},{"/world/original_production/development_revision",18},{"/world/original_production/development_revision",19},{"/world/original_production/development_revision",20},{"/world/original_production/development_revision",21},{"/world/original_production/development_revision",23},
            {"/world/original_production/hemp_farms/0/farm_id",0},
            {"/world/original_production/hemp_farms/0/farm_id",UINT32_MAX},
            {"/world/original_production/hemp_farms/0/field_id",nullptr},
            {"/world/original_production/hemp_farms/0/field_id",0},
            {"/world/original_production/hemp_farms/0/field_id",UINT32_MAX},
            {"/world/original_production/hemp_farms/0/harvested",-1},
            {"/world/original_production/hemp_farms/0/harvested",UINT64_MAX},
            {"/world/original_goods/cargo/0/good",5}}) {
        auto j=good;j.at(json::json_pointer(path))=value;reject(j);
    }
    {auto j=good;auto& rows=j["world"]["original_production"]["hemp_farms"];rows.push_back(rows[0]);reject(j);}
    {auto j=good;j["world"]["original_production"]["hemp_farms"]=json::array();reject(j);}
    for (const json& value:{json(0.0),json(true),json("0"),json(nullptr)}) {
        auto j=good;j["world"]["original_production"]["hemp_farms"][0]["harvested"]=value;reject(j);
    }
    for (std::size_t i=0;i<good["world"]["buildings"].size();++i) {
        const auto kind=good["world"]["buildings"][i]["kind"].get<unsigned>();
        if (kind==static_cast<unsigned>(sim::Object::OriginalHempFarm)) {
            auto j=good;j["world"]["buildings"][i]["progress"]=80;reject(j);
            j=good;j["world"]["buildings"][i]["cell"]=json::array({0,6});reject(j);
        } else if (kind==static_cast<unsigned>(sim::Object::OriginalHempField)) {
            auto j=good;j["world"]["buildings"][i]["cell"]=json::array({23,19});reject(j);
        }
    }
    {auto j=good;std::uint64_t field_tick=0;
        for (const auto& building:j["world"]["buildings"])
            if (building["id"]==static_cast<unsigned>(f.field)) field_tick=building["placed_tick"].get<std::uint64_t>();
        const auto impossible=(j["world"]["ticks"].get<std::uint64_t>()-field_tick)/80+1;
        auto& total=j["world"]["original_goods"]["totals"][0]["produced"];
        const auto extra=impossible-total.get<std::uint64_t>();total=impossible;
        auto& harvested=j["world"]["original_production"]["hemp_farms"][0]["harvested"];
        harvested=impossible;
        // A mathematically balanced fabricated harvest also needs enough
        // genuinely productive recipe ticks, not just matching three sums.
        auto& stock=j["world"]["original_goods"]["stocks"];
        for (auto& row:stock) if (row["building_id"]==static_cast<unsigned>(f.farm)) {
            row["amounts"][0]=row["amounts"][0].get<std::uint64_t>()+extra;break;
        }
        reject(j);}
    const auto document=f.document(f.world);
    for (int mutation=0;mutation<4;++mutation) {
        auto bad=document;
        if (mutation==0) bad.world.original_production->hemp_farms[0].field_id.reset();
        if (mutation==1) ++bad.world.original_production->hemp_farms[0].harvested;
        if (mutation==2) bad.world.original_production->hemp_farms[0].farm_id=f.field;
        if (mutation==3) bad.world.original_production->development_revision=15;
        refuses([&]{save::write_save(f.target,bad,f.root.data,f.policy->building_mask());});
        check(f::read_text(f.target)==pristine,"Malformed Hemp writer replaced good save");
    }
}
void geometry_bounds(Fixture& f) {
    const auto fresh=[&]{return sim::World::create_original_production_development(f.policy,f.model);};
    const auto add=[&](sim::World& world,sim::CommandType type,sim::Cell cell,sim::BuildingId owner=sim::BuildingId{}) {
        sim::Command command{type,cell};command.building_id=owner;f::buy(world,command);
        return *world.building_owner_at(cell);
    };
    save::SaveDocument good;
    sim::BuildingId farm{},field{};
    for (const auto origin:{sim::Cell{0,0},sim::Cell{f::width-2,0},sim::Cell{0,f::height-2},sim::Cell{f::width-2,f::height-2}}) {
        auto world=fresh();farm=add(world,sim::CommandType::PlaceOriginalHempFarm,origin);
        const sim::Cell beside{origin.x ? origin.x-1:origin.x+2,origin.y};
        field=add(world,sim::CommandType::PlaceOriginalHempField,beside,farm);
        good=f.document(world);save::write_save(f.target,good,f.root.data,f.policy->building_mask());
        check(f.restore(save::read_save(f.target)).snapshot()==world.snapshot(),
              "Legal edge Farm and explicitly adjacent Field did not roundtrip");
    }
    const auto pristine=f::read_text(f.target);const auto original=json::parse(pristine);
    const auto hostile=f.root.path/"hostile-hemp-geometry.json";
    constexpr auto low=std::numeric_limits<int>::min(),high=std::numeric_limits<int>::max();
    for (const auto& [farm_cell,field_cell]:std::vector<std::pair<sim::Cell,sim::Cell>>{
            {{high,0},{high-1,0}},{{low,0},{low+2,0}},
            {{0,high},{0,high-1}},{{0,low},{0,low+2}},
            {{f::width-1,f::height-2},{f::width-2,f::height-2}},
            {{f::width-2,f::height-1},{f::width-3,f::height-1}},
            {{f::width-2,f::height-2},{f::width,f::height-2}},
            {{f::width-2,f::height-2},{f::width-2,f::height}},
            {{0,0},{-1,0}},{{0,0},{0,-1}},
            {{0,0},{high,0}},{{0,0},{low,0}},{{0,0},{0,high}},{{0,0},{0,low}}}) {
        auto bad=good;auto encoded=original;
        for (auto& building:bad.world.buildings) {
            if (building.id==farm) building.cell=farm_cell;
            if (building.id==field) building.cell=field_cell;
        }
        for (auto& building:encoded["world"]["buildings"]) {
            if (building["id"]==static_cast<unsigned>(farm)) building["cell"]=json::array({farm_cell.x,farm_cell.y});
            if (building["id"]==static_cast<unsigned>(field)) building["cell"]=json::array({field_cell.x,field_cell.y});
        }
        refuses([&]{(void)sim::World::restore_original_production_development(bad.world,f.policy,f.model);});
        refuses([&]{save::write_save(f.target,bad,f.root.data,f.policy->building_mask());});
        f::write_text(hostile,encoded.dump());
        refuses([&]{(void)f.restore(save::read_save(hostile));});
        check(f::read_text(f.target)==pristine,"Invalid Farm or Field geometry replaced the legal edge save");
    }
}
} // namespace
int main() {
    try {Fixture f;const auto mid=lifecycle(f);adversarial(f,mid);geometry_bounds(f);
        std::cout<<"Hemp save PASS: paid Farm/Field, two actual goods, typed midroute/return, 20k replay and "
                 <<rejected<<" adversarial refusals\n";return 0;}
    catch (const std::exception& error) {std::cerr<<"Original Hemp save FAIL: "<<error.what()<<'\n';return 1;}
}
