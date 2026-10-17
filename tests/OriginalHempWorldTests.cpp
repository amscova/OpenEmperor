#include "OriginalInfluenceFixture.h"
#include "OriginalProductionFixture.h"
#include "simulation/original/OriginalRulesModel.h"

#include <algorithm>
#include <iostream>
#include <limits>

namespace {
namespace f=oe_influence_fixture;namespace sim=openemperor::simulation;namespace orig=openemperor::original;
using f::check;
template<class F> void refuses(F action){bool denied=false;try{action();}catch(const std::exception&){denied=true;}check(denied,"Invalid Hemp authority accepted");}
struct Fixture {
    f::Root root;std::shared_ptr<const sim::MapPermissions> policy=f::permissions();
    std::shared_ptr<const orig::OriginalRulesModel> model;
    Fixture(){oe_production_fixture::write_model(root.data,7);model=f::save::prepare_original_production_rules_model(root.data);}
    sim::World world()const{return sim::World::create_original_production_development(policy,model);}
};
sim::BuildingId buy(sim::World& w,sim::CommandType type,sim::Cell cell){f::buy(w,{type,cell});return *w.building_owner_at(cell);}
sim::Command field(sim::BuildingId farm,sim::Cell cell){sim::Command c{sim::CommandType::PlaceOriginalHempField,cell};c.building_id=farm;return c;}
int stock(const sim::World& w,sim::BuildingId id,orig::Goods good){
    for(const auto& s:w.original_goods_state()->stocks)if(s.building_id==id)return s.amounts[static_cast<std::size_t>(good)];
    throw std::runtime_error("Missing Hemp typed stock");
}
std::uint64_t harvested(const sim::World& w){return w.original_goods_balance(orig::Goods::Hemp).totals.produced;}
void conserved(const sim::World& w){
    check(w.production_balance_valid()&&w.navigation_valid()&&w.city_economy_valid()&&w.population_valid(),"Hemp World invariants failed");
    const auto balances=w.original_goods_balances();
    for(std::size_t i=0;i<balances.size();++i){check(balances[i].conserved&&balances[i].supported,"Hemp conservation/capability mismatch");if(i>=2)check(!balances[i].totals.produced&&!balances[i].stock&&!balances[i].cargo&&!balances[i].totals.consumed,"Hemp-only scene fabricated luxury goods");}
    check(w.original_goods_state()->reservations.size()<=8&&w.original_goods_state()->cargo.size()<=8,"Hemp latest-trip bound grew");
    for(const auto& b:w.buildings())check(!b.pottery_stock&&!b.food_stock&&!b.population&&!b.food_produced,"Hemp was mirrored into old Food/Pottery/resident fields");
    for(const auto& c:w.couriers())if(c.role==sim::CourierRole::OriginalHempToWarehouse)
        check(!c.cargo&&!c.reserved&&c.good==sim::Good::Goods,"Hemp cargo was mirrored or mislabeled as old Food/Pottery");
}
void ticks(sim::World& w,int count){for(int i=0;i<count;++i){w.tick();conserved(w);}}
struct Scene{sim::BuildingId farm{},field{},warehouse{},market{};};
Scene scene(sim::World& w,bool has_field=true,bool connected=true,bool houses=true){
    for(int x=0;x<24;++x)f::buy(w,{sim::CommandType::PlaceRoad,{x,8}});
    if(houses)for(int x=0;x<18;x+=3)(void)buy(w,sim::CommandType::PlaceHousehold,{x,6});
    (void)buy(w,sim::CommandType::PlaceClaySource,{0,9});
    const sim::Cell origin=connected?sim::Cell{4,9}:sim::Cell{4,11};
    Scene s;s.farm=buy(w,sim::CommandType::PlaceOriginalHempFarm,origin);
    if(has_field){f::buy(w,field(s.farm,{origin.x+2,origin.y}));s.field=*w.building_owner_at({origin.x+2,origin.y});}
    (void)buy(w,sim::CommandType::PlacePottery,{8,9});s.warehouse=buy(w,sim::CommandType::PlaceWarehouse,{16,9});
    s.market=buy(w,sim::CommandType::PlaceMarket,{21,9});return s;
}
sim::CourierId loaded_hemp(sim::World& w,int limit=5000){
    for(int i=0;i<limit;++i){for(const auto& c:w.couriers())if(c.role==sim::CourierRole::OriginalHempToWarehouse&&
        c.phase==sim::CourierPhase::ToWarehouse&&c.edge_progress>0&&w.original_courier_cargo(c.id)>0)return c.id;
        w.tick();conserved(w);}
    throw std::runtime_error("Real Hemp Farm courier never loaded");
}
void paid_two_goods(){
    Fixture f;auto w=f.world();const auto s=scene(w);
    check(w.original_production_state()->development_revision==sim::kOriginalProductionDevelopmentRevision&&w.construction_spent_total()==1162&&w.treasury()==10838&&
        w.workforce_required(s.farm)==18&&w.couriers().size()==7&&
        std::count_if(w.couriers().begin(),w.couriers().end(),[](const auto& c){return c.role==sim::CourierRole::OriginalFoodToHouse;})==1,
        "Paid Farm40/Field4/model18worker and additional Market Food actor facts mismatch");
    check(sim::building_footprint(w.profile(),w.rule_version(),sim::Object::OriginalHempFarm)==sim::BuildingFootprint{2,2}&&
        sim::building_footprint(w.profile(),w.rule_version(),sim::Object::OriginalHempField)==sim::BuildingFootprint{1,1},"Farm/Field geometry mismatch");
    ticks(w,119);check(!harvested(w)&&w.total_population()==0,"Hemp preceded genuine immigrant workforce");
    const auto id=loaded_hemp(w);check(w.original_courier_good(id)==orig::Goods::Hemp,"Typed inspection did not report actual Hemp cargo");
    auto replay=sim::World::restore_original_production_development(w.snapshot(),f.policy,f.model);
    for(int i=0;i<500&&!stock(w,s.warehouse,orig::Goods::Ceramics);++i){w.tick();replay.tick();}
    check(stock(w,s.warehouse,orig::Goods::Ceramics)>0&&stock(w,s.market,orig::Goods::Ceramics)==0,
        "Missing first actual Warehouse handoff before Ceramics supply");
    check(w.total_population()==42 && w.workforce_used()==34 && w.workers_assigned(s.farm)==18,
        "Genuine residents did not staff the complete paid two-good chain before natural Fire");
    for(const bool house:{false,true}){
        auto teleport=w.snapshot();
        for(auto& row:teleport.original_goods->stocks){if(row.building_id==s.warehouse)--row.amounts[1];if(!house&&row.building_id==s.market)++row.amounts[1];}
        if(house)++teleport.original_economy->houses.front().house.ceramics;
        refuses([&]{(void)sim::World::restore_original_production_development(teleport,f.policy,f.model);});
    }
    for(int i=0;i<20000;++i){w.tick();replay.tick();conserved(w);check(w.snapshot()==replay.snapshot(),"20k actual two-good replay diverged");}
    check(w.total_population()==42&&harvested(w)>0,"Replay lost actual residents or paid harvest history");
    for(const auto& h:w.original_economy_state()->houses)check(h.house.hemp==8&&h.house.ceramics==8,"Finite six-House two-good chain left a buffer unserved");
    check(w.original_goods_balance(orig::Goods::Hemp).totals.imported==0&&w.original_goods_balance(orig::Goods::Ceramics).totals.imported==0,
        "The real two-good chain imported a gift");
    const auto before=w.snapshot();
    for(const auto cmd:{field(s.farm,{6,10}),field(s.market,{7,9}),field(s.farm,{20,18})})
        check(!w.execute(cmd).accepted&&w.snapshot()==before,"Duplicate/foreign/distant Field command was not atomic");
    for(const auto owner:{s.farm,s.field})check(!w.execute(sim::demolish_building(owner)).accepted&&w.snapshot()==before,"Hemp demolition bypassed paid history");
    auto bad=before;bad.original_production->hemp_farms.front().field_id=s.market;
    refuses([&]{(void)sim::World::restore_original_production_development(bad,f.policy,f.model);});
    bad=before;bad.original_production->hemp_farms.front().field_id.reset();
    refuses([&]{(void)sim::World::restore_original_production_development(bad,f.policy,f.model);});
    bad=before;bad.original_production->development_revision=15;
    refuses([&]{(void)sim::World::restore_original_production_development(bad,f.policy,f.model);});
    bad=before;for(auto& b:bad.buildings)if(b.id==s.farm)b.progress=80;
    refuses([&]{(void)sim::World::restore_original_production_development(bad,f.policy,f.model);});
    auto old_model=f::save::prepare_original_rules_model(f.root.data);
    refuses([&]{(void)sim::World::create_original_production_development(f.policy,old_model);});
    auto old=sim::World::create_original_goods_development(f.policy,old_model);const auto saved=old.snapshot();
    check(!old.execute({sim::CommandType::PlaceOriginalHempFarm,{0,9}}).accepted&&old.snapshot()==saved,
        "Frozen Rule9 gained Hemp authority");
}
void missing_field_workers_and_pauses(){
    Fixture f;
    for(const auto flags:{std::pair{false,true},std::pair{true,false}}){auto w=f.world();(void)scene(w,flags.first,true,flags.second);ticks(w,1200);
        check(!harvested(w),"Missing paid Field or absent workforce generated Hemp");}
    auto no_field=f.world();const auto unassigned=scene(no_field,false);
    const auto untouched=no_field.snapshot();
    for(const auto cell:{sim::Cell{INT32_MIN,9},sim::Cell{INT32_MAX,9},sim::Cell{6,INT32_MIN},sim::Cell{6,INT32_MAX}})
        check(!no_field.execute(field(unassigned.farm,cell)).accepted&&no_field.snapshot()==untouched,
            "Extreme Field coordinates overflowed or changed paid World authority");
    auto w=f.world();const auto s=scene(w);f::buy(w,sim::set_building_operation(s.farm,false));ticks(w,900);
    check(!harvested(w)&&w.total_population()==42,"Stopped staffed Hemp Farm produced");
    f::buy(w,sim::set_building_operation(s.farm,true));ticks(w,300);check(harvested(w)>0,"Hemp did not resume through ordinary operation control");
    while(w.ticks()<2000)w.tick();check(w.building_on_fire(s.farm),"Natural Hemp Farm Fire did not occur");
    const auto previous=harvested(w);ticks(w,200);check(harvested(w)==previous,"Burning Hemp Farm ignored staffing/Fire suspension");
}
void full_disconnected_cancelled(){
    Fixture f;auto w=f.world();const auto s=scene(w,true,false);ticks(w,1500);
    check(harvested(w)==8&&stock(w,s.farm,orig::Goods::Hemp)==8&&w.building(s.farm).progress==0&&
        !stock(w,s.warehouse,orig::Goods::Hemp),"Disconnected Hemp Farm exceeded its buffer or teleported delivery");
    const auto unplanted=buy(w,sim::CommandType::PlaceOriginalHempFarm,{10,12});
    auto reassigned=w.snapshot();
    for(auto& row:reassigned.original_goods->stocks){if(row.building_id==s.farm)--row.amounts[0];if(row.building_id==unplanted)++row.amounts[0];}
    refuses([&]{(void)sim::World::restore_original_production_development(reassigned,f.policy,f.model);});
    f::buy(w,{sim::CommandType::PlaceRoad,{4,9}});f::buy(w,{sim::CommandType::PlaceRoad,{4,10}});
    const auto id=loaded_hemp(w);const auto before=w.courier(id);const auto amount=w.original_courier_cargo(id);
    f::buy(w,sim::set_building_operation(s.farm,false));
    check(w.courier(id).phase==sim::CourierPhase::Returning&&w.courier(id).edge_progress==before.edge_progress&&
        w.courier(id).path.front()==before.path[before.path_vertex]&&w.original_courier_cargo(id)==amount,
        "Paused Hemp courier changed its begun edge or lost cargo");
    auto replay=sim::World::restore_original_production_development(w.snapshot(),f.policy,f.model);
    for(int i=0;i<1000&&w.courier(id).phase!=sim::CourierPhase::IdleAtWorkshop;++i){w.tick();replay.tick();conserved(w);check(w.snapshot()==replay.snapshot(),"Hemp abort/return restore diverged");}
    check(!w.original_courier_cargo(id)&&stock(w,s.farm,orig::Goods::Hemp)==8&&harvested(w)==8,
        "Full-source reserved return headroom lost, duplicated or discarded Hemp");
    f::buy(w,sim::set_building_operation(s.farm,true));ticks(w,4000);
    check(std::any_of(w.original_economy_state()->houses.begin(),w.original_economy_state()->houses.end(),
        [](const auto& h){return h.house.hemp>0&&h.house.ceramics>0;}),"Paid Hemp road repair did not reach a real two-good House");
}
void historical_ceramics_cannot_create_hemp_arrivals(){
    Fixture f;auto w=f.world();const auto s=scene(w,false);
    f::buy(w,sim::set_building_operation(s.farm,false));
    bool previous_rotations=false;
    for(int i=0;i<5000 && !previous_rotations;++i){
        w.tick();conserved(w);bool inbound=false,outbound=false;
        for(const auto& c:w.couriers()) for(const auto& row:w.original_goods_state()->reservations)
            if(row.transfer.courier_id==c.id && row.transfer.sequence>1){
                inbound|=c.role==sim::CourierRole::MarketPotteryInbound;
                outbound|=c.role==sim::CourierRole::MarketPotteryDistribution;
            }
        previous_rotations=inbound&&outbound;
    }
    check(previous_rotations,"Real Ceramics did not exercise compacted two-stage delivery history");
    const auto unconnected=buy(w,sim::CommandType::PlaceOriginalHempFarm,{10,12});
    f::buy(w,field(unconnected,{12,12}));
    for(int i=0;i<1000 && !stock(w,unconnected,orig::Goods::Hemp);++i){w.tick();conserved(w);}
    check(stock(w,unconnected,orig::Goods::Hemp)>0,"Unconnected paid Farm never produced a real first Hemp unit");
    check(std::none_of(w.original_goods_state()->reservations.begin(),w.original_goods_state()->reservations.end(),
        [&](const auto& row){return row.source==unconnected;}),"No-visit control unexpectedly dispatched from its disconnected Farm");
    auto forged=w.snapshot();
    for(auto& row:forged.original_goods->stocks) if(row.building_id==unconnected)--row.amounts[0];
    ++forged.original_economy->houses.front().house.hemp;
    refuses([&]{(void)sim::World::restore_original_production_development(forged,f.policy,f.model);});
}
}
int main(){try{paid_two_goods();missing_field_workers_and_pauses();full_disconnected_cancelled();historical_ceramics_cannot_create_hemp_arrivals();
    std::cout<<"Rule10 paid Farm/Field two-good House supply, bounded harvest, actual workforce/transport and20k replay PASS\n";return 0;
}catch(const std::exception& e){std::cerr<<"Original Hemp World FAIL: "<<e.what()<<'\n';return 1;}}
