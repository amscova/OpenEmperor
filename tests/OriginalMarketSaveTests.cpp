#include "OriginalProductionFixture.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <array>
#include <functional>
#include <iostream>

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
    throw std::runtime_error("Malformed Market save was admitted");
}
struct Fixture {
    f::Root root;
    std::shared_ptr<const sim::MapPermissions> policy=f::permissions();
    std::shared_ptr<const orig::OriginalRulesModel> model=[&] {
        oe_production_fixture::write_model(root.data,37);
        return save::prepare_original_production_rules_model(root.data);
    }();
    sim::World world=sim::World::create_original_production_development(policy,model);
    f::fs::path target=root.path/"market-schema26.json";
    sim::BuildingId warehouse{},market{};
    Fixture() {
        for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,8}});
        for (int x=0;x<12;x+=3) f::buy(world,{sim::CommandType::PlaceHousehold,{x,6}});
        f::buy(world,{sim::CommandType::PlaceClaySource,{0,9}});
        f::buy(world,{sim::CommandType::PlacePottery,{8,9}});
        f::buy(world,{sim::CommandType::PlaceWarehouse,{16,9}});
        f::buy(world,{sim::CommandType::PlaceMarket,{21,9}});
        warehouse=*world.building_owner_at({16,9});market=*world.building_owner_at({21,9});
        check(world.construction_spent_total()==958 && world.treasury()==11042 && world.couriers().size()==5 &&
              world.couriers()[3].owner==market && world.couriers()[3].role==sim::CourierRole::MarketPotteryDistribution &&
              world.couriers()[4].owner==market && world.couriers()[4].role==sim::CourierRole::OriginalFoodToHouse,
              "Market and its exact two distributors were not bought through normal commands");
    }
    save::SaveDocument document(const sim::World& value) const {
        return save::make_document(root.data,f::map_relative,policy->building_mask(),value);
    }
    sim::World restore(const save::SaveDocument& d) const {
        return save::restore_original_production_development(d,root.data,policy->building_mask(),policy,model);
    }
    std::string roundtrip() {
        const auto before=world.snapshot();const auto d=document(world);
        save::write_save(target,d,root.data,policy->building_mask());
        const auto bytes=f::read_text(target);const auto j=json::parse(bytes);
        check(j.at("schema_version")==26 && j.at("world").at("original_production").at("development_revision")==21,
              "Market save did not identify revision21");
        const auto parsed=save::read_save(target);f::same_document(d,parsed);
        auto restored=restore(parsed);check(restored.snapshot()==before,"Full typed Market restore changed authority");
        save::write_save(target,document(restored),root.data,policy->building_mask());
        check(f::read_text(target)==bytes && world.snapshot()==before,"Market rewrite was not canonical and atomic");
        return bytes;
    }
};
void conserved(const sim::World& world) {
    check(world.production_balance_valid() && world.city_economy_valid() && world.navigation_valid() && world.population_valid(),
          "Typed Market state violated an inherited World invariant");
    const auto balance=world.original_goods_balance(orig::Goods::Ceramics);
    check(balance.supported && balance.conserved && balance.stock+balance.cargo==balance.totals.produced &&
          balance.totals.produced==world.original_production_state()->pottery_handed_off,
          "Warehouse, Market, House and actual cargo did not share a single mass balance");
}
bool moving(const sim::World& world,sim::CourierRole role,int minimum=1) {
    for (const auto& c:world.couriers())
        if (c.role==role && c.phase==sim::CourierPhase::ToWarehouse && c.edge_progress>0 &&
            world.original_courier_cargo(c.id)>=minimum) return true;
    return false;
}
template<class P> void until(sim::World& world,P condition,int limit=20000) {
    for (int tick=0;tick<limit && !condition();++tick) {world.tick();conserved(world);}
    check(condition(),"Paid Market did not reach the required actual route state");
}
sim::WorldSnapshot deliveries_and_replay(Fixture& fixture) {
    auto& world=fixture.world;fixture.roundtrip();
    until(world,[&]{return moving(world,sim::CourierRole::MarketPotteryInbound);});
    fixture.roundtrip();const auto inbound=world.snapshot();auto copied=fixture.restore(save::read_save(fixture.target));
    for (int tick=0;tick<200;++tick) {world.tick();copied.tick();}
    check(world.snapshot()==copied.snapshot(),"Warehouse→Market typed mid-route replay diverged");
    until(world,[&]{return moving(world,sim::CourierRole::MarketPotteryDistribution,2);});
    fixture.roundtrip();const auto house_trip=world.snapshot();copied=fixture.restore(save::read_save(fixture.target));
    for (int tick=0;tick<20000;++tick) {world.tick();copied.tick();}
    check(world.snapshot()==copied.snapshot(),"Market→House 20k typed replay diverged");conserved(world);
    bool delivered=false;
    for (const auto& house:world.original_economy_state()->houses) {
        check(house.house.ceramics>=0 && house.house.ceramics<=sim::Rules::household_capacity,
              "Actual House delivery exceeded its authored bounded buffer");
        delivered=delivered || house.house.ceramics>0;
    }
    check(delivered,"Market arrival never reached the HouseState stock authority");
    fixture.roundtrip();
    // A used idle actor retains its terminal sequence highwater. Erasing that
    // history must not turn an earlier trip into a never-dispatched actor.
    bool checked_terminal=false;
    for (const auto& courier:world.couriers()) if (courier.phase==sim::CourierPhase::IdleAtWorkshop &&
            courier.last_dispatched_target) {
        auto bad=fixture.document(world);auto& rows=bad.world.original_goods->reservations;
        const auto before=rows.size();std::erase_if(rows,[&](const auto& row){return row.transfer.courier_id==courier.id;});
        if (before==rows.size()) continue;
        refuses([&]{(void)fixture.restore(bad);});
        const auto protected_bytes=f::read_text(fixture.target);
        refuses([&]{save::write_save(fixture.target,bad,fixture.root.data,fixture.policy->building_mask());});
        check(f::read_text(fixture.target)==protected_bytes,"Terminal-history refusal replaced the valid save");
        checked_terminal=true;break;
    }
    check(checked_terminal,"No real used idle typed actor supplied the terminal-highwater test");
    // Captured snapshots come from paid commands and real movement, never a
    // fabricated setup; they independently exercise each source's return path.
    for (const auto& [captured,source]:std::array{
            std::pair{inbound,fixture.warehouse},std::pair{house_trip,fixture.market}}) {
        auto returned=sim::World::restore_original_production_development(captured,fixture.policy,fixture.model);
        f::buy(returned,sim::set_building_operation(source,false));
        until(returned,[&]{
            for (const auto& row:returned.original_goods_state()->reservations)
                if (row.source==source && row.phase==sim::OriginalGoodsTransferPhase::Returning) return true;
            return false;
        });
        const auto d=fixture.document(returned);save::write_save(fixture.target,d,fixture.root.data,fixture.policy->building_mask());
        auto replay=fixture.restore(save::read_save(fixture.target));
        for (int tick=0;tick<1000;++tick) {returned.tick();replay.tick();}
        check(returned.snapshot()==replay.snapshot(),"Loaded paused-source return replay diverged");
        conserved(returned);
        bool completed_return=false;
        for (const auto& row:returned.original_goods_state()->reservations)
            completed_return=completed_return || (row.source==source && row.phase==sim::OriginalGoodsTransferPhase::Returned && row.returned>0);
        check(completed_return,"Loaded trip did not actually return to its reserved source space");
    }
    return house_trip;
}
void malformed_typed_payloads(Fixture& f,const sim::WorldSnapshot& midroute) {
    f.world=sim::World::restore_original_production_development(midroute,f.policy,f.model);
    const auto pristine=f.roundtrip();const auto good=json::parse(pristine);
    check(!good["world"]["original_goods"]["cargo"].empty() && !good["world"]["original_goods"]["reservations"].empty(),
          "Adversarial cases did not start from actual loaded typed cargo");
    const auto hostile=f.root.path/"hostile-market.json";
    const auto reject=[&](const json& j) {
        f::write_text(hostile,j.dump());
        try {refuses([&]{(void)f.restore(save::read_save(hostile));});}
        catch (const std::exception& error) {throw std::runtime_error(std::string(error.what())+": "+json::diff(good,j).dump());}
        check(f::read_text(f.target)==pristine,"Rejected typed save changed protected target");
    };
    std::vector<std::string> objects;
    const std::function<void(const json&,std::string)> visit=[&](const json& j,std::string path) {
        if (j.is_object()) {
            objects.push_back(path);for (const auto& [key,value]:j.items()) visit(value,path+"/"+key);
        } else if (j.is_array()) for (std::size_t i=0;i<j.size();++i) visit(j[i],path+"/"+std::to_string(i));
    };
    visit(good,"");
    for (const auto& path:objects) {
        auto j=good;j.at(json::json_pointer(path))["unsupported_extra_authority"]=1;reject(j);
        for (const auto& [key,value]:good.at(json::json_pointer(path)).items()) {
            (void)value;j=good;j.at(json::json_pointer(path)).erase(key);reject(j);
        }
    }
    for (const auto& [path,value]:std::vector<std::pair<std::string,json>>{
            {"/world/original_production/development_revision",15},{"/world/original_production/development_revision",18},{"/world/original_production/development_revision",19},{"/world/original_production/development_revision",20},{"/world/original_production/development_revision",22},
            {"/world/original_goods/cargo/0/transfer/courier_id",0},
            {"/world/original_goods/cargo/0/transfer/sequence",0},
            {"/world/original_goods/cargo/0/transfer/sequence",UINT64_MAX},
            {"/world/original_goods/cargo/0/good",6},{"/world/original_goods/cargo/0/good",0},
            {"/world/original_goods/cargo/0/source",UINT32_MAX},{"/world/original_goods/cargo/0/target",UINT32_MAX},
            {"/world/original_goods/cargo/0/amount",0},{"/world/original_goods/cargo/0/amount",-1},
            {"/world/original_goods/cargo/0/amount",INT32_MAX},
            {"/world/original_goods/reservations/0/transfer/courier_id",0},
            {"/world/original_goods/reservations/0/transfer/sequence",0},
            {"/world/original_goods/reservations/0/phase",6},
            {"/world/original_goods/reservations/0/reserved",0},
            {"/world/original_goods/reservations/0/loaded",INT32_MAX},
            {"/world/original_goods/reservations/0/delivered",INT32_MAX},
            {"/world/original_goods/reservations/0/returned",-1},
            {"/world/original_goods/stocks/0/building_id",UINT32_MAX},
            {"/world/original_economy/houses/0/house/ceramics",sim::Rules::household_capacity+1}}) {
        auto j=good;j.at(json::json_pointer(path))=value;reject(j);
    }
    for (const char* key:{"cargo","reservations"}) {
        auto j=good;auto& rows=j["world"]["original_goods"][key];rows.push_back(rows[0]);reject(j);
        j=good;j["world"]["original_goods"][key]=json::array();reject(j);
        for (const json& value:{json(false),json(0.0),json("0"),json(nullptr)}) {
            j=good;j["world"]["original_goods"][key][0][key==std::string("cargo") ? "amount":"reserved"]=value;reject(j);
        }
    }
    {auto j=good;const auto courier=j["world"]["original_goods"]["cargo"][0]["transfer"]["courier_id"];
        j["world"]["original_goods"]["cargo"][0]["transfer"]["sequence"]=UINT64_MAX;
        bool matched=false;
        for (auto& row:j["world"]["original_goods"]["reservations"])
            if (row["transfer"]["courier_id"]==courier) {row["transfer"]["sequence"]=UINT64_MAX;matched=true;}
        check(matched,"Coordinated highwater forgery did not have a real reservation");reject(j);}
    {auto j=good;bool changed=false;
        for (auto& item:j["world"]["original_goods"]["cargo"]) {
            if (item["amount"].get<int>()<2) continue;
            const auto courier=item["transfer"]["courier_id"],target=item["target"];
            item["amount"]=item["amount"].get<int>()-1;
            for (auto& row:j["world"]["original_goods"]["reservations"])
                if (row["transfer"]["courier_id"]==courier) row["delivered"]=row["delivered"].get<int>()+1;
            bool destination=false;
            for (auto& house:j["world"]["original_economy"]["houses"])
                if (house["building_id"]==target) {house["house"]["ceramics"]=house["house"]["ceramics"].get<int>()+1;destination=true;}
            for (auto& stock:j["world"]["original_goods"]["stocks"])
                if (stock["building_id"]==target) {stock["amounts"][1]=stock["amounts"][1].get<int>()+1;destination=true;}
            check(destination,"Actual loaded trip had no existing destination stock");changed=true;break;
        }
        check(changed,"Coordinated pre-arrival forgery requires actual cargo of at least two units");reject(j);}
    {auto j=good;auto& rows=j["world"]["original_goods"]["stocks"];std::swap(rows[0],rows[1]);reject(j);}
    {auto j=good;j["world"]["original_goods"]["stocks"][0]["amounts"][1]=sim::Rules::warehouse_capacity;reject(j);}
    const auto marker=pristine.find("\"transfer\"");const auto object=pristine.find('{',marker);
    check(object!=std::string::npos,"Typed duplicate-key control missing");auto duplicate=pristine;
    duplicate.insert(object+1,"\"sequence\":1,");f::write_text(hostile,duplicate);
    refuses([&]{(void)save::read_save(hostile);});
    const auto document=f.document(f.world);
    for (int fault=0;fault<4;++fault) {
        auto bad=document;
        if (fault==0) bad.world.original_goods->cargo[0].transfer.sequence++;
        if (fault==1) bad.world.original_goods->cargo[0].amount++;
        if (fault==2) bad.world.original_goods->reservations[0].reserved++;
        if (fault==3) bad.world.original_production->development_revision=14;
        try {refuses([&]{save::write_save(f.target,bad,f.root.data,f.policy->building_mask());});}
        catch (const std::exception& error) {throw std::runtime_error(std::string(error.what())+" DTO fault "+std::to_string(fault));}
        check(f::read_text(f.target)==pristine,"Malformed typed writer replaced good save");
    }
}
} // namespace
int main() {
    try {Fixture f;const auto midroute=deliveries_and_replay(f);malformed_typed_payloads(f,midroute);
        std::cout<<"Schema26 revision21 paid Market, typed midroute/return, House arrival, 20k replay and "
                 <<rejected<<" adversarial refusals PASS\n";return 0;}
    catch (const std::exception& error) {std::cerr<<"Original Market save FAIL: "<<error.what()<<'\n';return 1;}
}
