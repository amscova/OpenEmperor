#include "OriginalProductionFixture.h"
#include "persistence/SandboxSave.h"
#include "simulation/RoadConnectivity.h"

#include <algorithm>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

namespace {
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
using f::check;
struct Fixture {
    f::Root root;
    std::shared_ptr<const sim::MapPermissions> policy=f::permissions();
    std::shared_ptr<const openemperor::original::OriginalRulesModel> model;
    Fixture() {
        oe_production_fixture::write_model(root.data,6);
        model=f::save::prepare_original_production_rules_model(root.data);
    }
    sim::World world() const {return sim::World::create_original_fire_protection(policy,model);}
};
sim::BuildingId buy(sim::World& world,sim::CommandType type,sim::Cell cell) {
    f::buy(world,{type,cell});return *world.building_owner_at(cell);
}
void ticks(sim::World& world,int count) {for (int i=0;i<count;++i) world.tick();}
sim::CourierId inspector(const sim::World& world,sim::BuildingId watch) {
    const auto found=std::find_if(world.couriers().begin(),world.couriers().end(),[&](const auto& c) {
        return c.owner==watch && c.role==sim::CourierRole::FireInspector;
    });
    check(found!=world.couriers().end(),"Paid Watch lacks its single Inspector");return found->id;
}
void invariant(const sim::World& world) {
    check(world.population_valid() && world.city_economy_valid() && world.production_balance_valid() &&
        world.navigation_valid() && world.fire_state_valid(),"Rule11 production authority invariant");
}
void denies(sim::World& world,sim::Command command) {
    const auto before=world.snapshot();const auto result=world.execute(command);
    check(!result.accepted && !result.changed && world.snapshot()==before,"Rejected command mutated paid World");
}
template<class F> void refuses(F action) {
    bool rejected=false;try {action();} catch (const std::exception&) {rejected=true;}
    check(rejected,"Hostile or wrong-version Rule11 restore accepted");
}
void road(sim::World& world) {
    for (int x=0;x<13;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,8}});
}
void paid_construction_and_limits() {
    Fixture fixture;auto world=fixture.world();
    check(world.rule_version()==11 && world.original_fire_protection_enabled() && world.original_production_enabled() &&
        world.active_rules().version==11 && world.treasury()==12000,"Rule11 opt-in identity/default treasury");
    check(sim::current_rule_version(sim::RulesProfile::CityV16)==4 &&
        sim::current_rule_version(sim::RulesProfile::CityV11)==3,"Historical menu/World defaults changed");
    const auto footprint=sim::building_footprint(world.profile(),world.rule_version(),sim::Object::FireWatch);
    check(footprint.width==1 && footprint.height==1 && world.construction_cost(sim::CommandType::PlaceFireWatch)==80,
        "Authored Watch geometry/price changed");
    const auto a=buy(world,sim::CommandType::PlaceFireWatch,{4,9});
    check(world.treasury()==11920 && world.construction_spent_total()==80 && world.workforce_required(a)==2 &&
        world.couriers().size()==1 && world.courier(inspector(world,a)).owner==a,"Paid Watch purchase/ownership/workers");
    denies(world,{sim::CommandType::PlaceFireWatch,{4,9}});
    denies(world,{sim::CommandType::PlaceFireWatch,{-1,9}});
    denies(world,{sim::CommandType::PlaceFireWatch,{world.width(),9}});
    buy(world,sim::CommandType::PlaceFireWatch,{7,9});
    denies(world,{sim::CommandType::PlaceFireWatch,{10,9}});
    auto old=sim::World::create_original_production_development(fixture.policy,fixture.model);
    denies(old,{sim::CommandType::PlaceFireWatch,{4,9}});
    refuses([&]{(void)sim::World::restore_original_production_development(world.snapshot(),fixture.policy,fixture.model);});
    refuses([&]{(void)sim::World::restore_original_fire_protection(old.snapshot(),fixture.policy,fixture.model);});
    refuses([&]{sim::World ordinary(fixture.policy,sim::RulesProfile::CityV16,11);});
    const auto saved=world.snapshot();
    auto oversized=saved;oversized.buildings.resize(sim::Rules::original_production_building_limit+1);
    refuses([&]{(void)sim::World::restore_original_fire_protection(oversized,fixture.policy,fixture.model);});
    oversized=saved;oversized.couriers.resize(sim::Rules::original_production_courier_limit+1);
    refuses([&]{(void)sim::World::restore_original_fire_protection(oversized,fixture.policy,fixture.model);});
    check(world.snapshot()==saved,"Rejected oversized restore changed source World");
    invariant(world);
}
void terrain_and_insufficient_funds() {
    Fixture fixture;
    std::vector<sim::MapCellPermission> cells(f::width*f::height,
        {true,true,false,0,sim::BuildBlocker::None,sim::BuildBlocker::None});
    cells[10*f::width+10]={false,false,true,0,sim::BuildBlocker::OriginalStructure,sim::BuildBlocker::OriginalStructure};
    const auto policy=std::make_shared<const sim::MapPermissions>(f::width,f::height,3,std::move(cells),
        std::vector<sim::FixedGatePassage>{});
    auto world=sim::World::create_original_fire_protection(policy,fixture.model);
    denies(world,{sim::CommandType::PlaceFireWatch,{10,10}});
    // Independent authored model fixture with a large installed staffing bill.
    // No live Funds, tax, occupants or inventory are changed to reach debt.
    auto text=oe_production_fixture::authored_model_text(6);
    const std::string before="207,Herbalist's Stall,{,30,0,0,0,0,7,";
    const auto at=text.find(before);check(at!=std::string::npos,"Authored Herbalist row absent");
    text.replace(at,before.size(),"207,Herbalist's Stall,{,30,0,0,0,0,10000,");
    f::write_text(fixture.root.data/"Model/EmperorBuildingModels.txt",text);
    const auto expensive=f::save::prepare_original_production_rules_model(fixture.root.data);
    auto poor=sim::World::create_original_fire_protection(fixture.policy,expensive);
    buy(poor,sim::CommandType::PlaceHealthPost,{0,0});ticks(poor,400);
    check(poor.treasury()<0 && poor.maintenance_spent_total()==20000,"Real installed maintenance did not create deficit");
    check(poor.validate({sim::CommandType::PlaceFireWatch,{4,9}}).diagnostic.blocker==sim::BuildBlocker::InsufficientFunds,
        "Watch construction did not report its real insufficient-Funds blocker");
    denies(poor,{sim::CommandType::PlaceFireWatch,{4,9}});invariant(poor);
}
void staffing_pause_and_prevention() {
    Fixture fixture;auto empty=fixture.world();road(empty);
    const auto watch=buy(empty,sim::CommandType::PlaceFireWatch,{4,9});
    const auto id=inspector(empty,watch);ticks(empty,500);
    check(!empty.building_staffed(watch) && empty.courier(id).phase==sim::CourierPhase::IdleAtWorkshop &&
        empty.protected_buildings()==0,"Unstaffed Watch fabricated a visit");
    auto world=fixture.world();road(world);
    const auto home=buy(world,sim::CommandType::PlaceHousehold,{0,6});
    const auto staffed=buy(world,sim::CommandType::PlaceFireWatch,{4,9});
    const auto actor=inspector(world,staffed);
    f::buy(world,sim::set_building_operation(staffed,false));ticks(world,500);
    check(world.total_population()==6 && world.courier(actor).phase==sim::CourierPhase::IdleAtWorkshop &&
        !world.building_fire_protected(home) && world.maintenance_spent_total()==4,
        "Pause granted protection or waived installed authored upkeep");
    f::buy(world,sim::set_building_operation(staffed,true));world.tick();
    check(world.building_staffed(staffed) && world.courier(actor).phase==sim::CourierPhase::ToWarehouse &&
        !world.building_fire_protected(home),"Departure granted protection before arrival");
    while (!world.building_fire_protected(home) && world.ticks()<600) world.tick();
    check(world.building_fire_protected(home) && world.fire_protection_remaining(home)==2400 &&
        world.courier(actor).phase==sim::CourierPhase::Returning,"Actual Inspector arrival did not protect for2400ticks");
    while (world.ticks()<2000) world.tick();
    check(!world.building_on_fire(home) && world.building_fire_protected(home) && world.building_on_fire(staffed) &&
        !world.building_staffed(staffed),"Patrol did not prevent natural House ignition or Watch remained fireproof");
    invariant(world);
    auto old=sim::World::create_original_production_development(fixture.policy,fixture.model);road(old);
    const auto old_home=buy(old,sim::CommandType::PlaceHousehold,{0,6});ticks(old,2000);
    check(old.building_on_fire(old_home) && old.protected_buildings()==0,"Unchanged Rule10 natural fire baseline diverged");
    // Historical City-v12 Watch remains fireproof, including its serialized zero fields.
    sim::World legacy(f::width,f::height,std::vector<std::uint8_t>(f::width*f::height,1),sim::RulesProfile::CityV12,1);
    const auto old_watch=buy(legacy,sim::CommandType::PlaceFireWatch,{0,0});ticks(legacy,2000);
    check(!legacy.building_on_fire(old_watch) && legacy.building(old_watch).fire_risk==0 &&
        legacy.building(old_watch).fire_until_tick==0,"Rule11 Watch combustibility leaked into historical rules");
}
void actual_extinguishing_repair_and_restore() {
    Fixture fixture;auto world=fixture.world();road(world);
    const auto home=buy(world,sim::CommandType::PlaceHousehold,{0,6});ticks(world,1900);
    const auto watch=buy(world,sim::CommandType::PlaceFireWatch,{4,9});
    const auto id=inspector(world,watch);f::buy(world,sim::set_building_operation(watch,false));ticks(world,100);
    check(world.building_on_fire(home) && !world.building_on_fire(watch),"Natural staggered paid fire fixture");
    f::buy(world,sim::set_building_operation(watch,true));world.tick();
    check(world.courier(id).phase==sim::CourierPhase::ToWarehouse && world.courier(id).edge_progress==1 &&
        world.building_on_fire(home),"Burning-target priority/actual dispatch");
    // Operation changes govern new dispatches; the existing real trip completes normally.
    f::buy(world,sim::set_building_operation(watch,false));
    f::buy(world,{sim::CommandType::RemoveRoad,{2,8}});
    check(world.courier(id).route_pending && world.courier(id).edge_progress==1,"Cut interrupted begun safe edge");
    ticks(world,4);
    check(world.courier(id).route_pending && world.courier(id).edge_progress==0 && world.building_on_fire(home),
        "Inspector did not wait at last real reachable cell");
    auto checkpoint=world.snapshot();auto restored=sim::World::restore_original_fire_protection(checkpoint,fixture.policy,fixture.model);
    check(restored.snapshot()==checkpoint,"Paused/waiting complete World restore changed authority");
    const auto waiting_position=world.courier_position(id);
    for (int i=0;i<25;++i) {world.tick();restored.tick();check(world.snapshot()==restored.snapshot(),"Waiting replay divergence");}
    check(world.courier_position(id)==waiting_position && world.building_on_fire(home) && !world.building_fire_protected(home),
        "Disconnected Inspector teleported protection or moved");
    f::buy(world,{sim::CommandType::PlaceRoad,{2,8}});f::buy(restored,{sim::CommandType::PlaceRoad,{2,8}});
    bool moving_restored=false;
    while (world.building_on_fire(home) && world.ticks()<2200) {
        world.tick();restored.tick();check(world.snapshot()==restored.snapshot(),"Repaired route replay divergence");
        if (!moving_restored && !world.courier(id).route_pending && world.courier(id).edge_progress>0) {
            checkpoint=world.snapshot();auto moving=sim::World::restore_original_fire_protection(checkpoint,fixture.policy,fixture.model);
            check(moving.snapshot()==checkpoint,"In-flight complete World restore changed path or progress");
            restored=std::move(moving);moving_restored=true;
        }
    }
    check(moving_restored && !world.building_on_fire(home) && world.building_fire_protected(home) &&
        world.building(home).fire_until_tick==world.ticks() && world.fire_protection_remaining(home)==2400 &&
        world.courier(id).phase==sim::CourierPhase::Returning,"Actual repaired-route Inspector did not extinguish/protect/startreturn");
    check(!world.original_production_state()->fire_protection->arrivals.empty() &&
        world.original_production_state()->fire_protection->arrivals.back().extinguished,
        "Actual extinguishing lacks its versioned historical witness");
    for (int i=0;i<300;++i) {world.tick();restored.tick();check(world.snapshot()==restored.snapshot(),"Return/Idle replay divergence");}
    check(world.courier(id).phase==sim::CourierPhase::IdleAtWorkshop,"Paused Watch started a new patrol");
    invariant(world);
}
void already_dispatched_from_burning_watch() {
    Fixture fixture;auto world=fixture.world();road(world);
    const auto home=buy(world,sim::CommandType::PlaceHousehold,{0,6});
    const auto watch=buy(world,sim::CommandType::PlaceFireWatch,{4,9});
    const auto id=inspector(world,watch);f::buy(world,sim::set_building_operation(watch,false));
    ticks(world,1998);f::buy(world,sim::set_building_operation(watch,true));world.tick();
    check(world.courier(id).phase==sim::CourierPhase::ToWarehouse && !world.building_on_fire(watch),
        "Real staffed departure before source ignition absent");
    world.tick();check(world.building_on_fire(watch) && world.building_on_fire(home) && !world.building_staffed(watch),
        "Natural source ignition did not occur during its existing outbound trip");
    auto restored=sim::World::restore_original_fire_protection(world.snapshot(),fixture.policy,fixture.model);
    while (world.building_on_fire(home) && world.ticks()<2100) {
        world.tick();restored.tick();check(world.snapshot()==restored.snapshot(),"Burning-source in-flight replay diverged");
    }
    check(world.building_on_fire(watch) && !world.building_on_fire(home) && world.building_fire_protected(home) &&
        world.courier(id).phase==sim::CourierPhase::Returning,
        "Already dispatched real Inspector failed to complete while its source burns");
    invariant(world);
}
void arrival_on_natural_expiry() {
    Fixture fixture;auto world=fixture.world();road(world);
    const auto home=buy(world,sim::CommandType::PlaceHousehold,{0,6});ticks(world,1900);
    const auto watch=buy(world,sim::CommandType::PlaceFireWatch,{4,9});const auto id=inspector(world,watch);
    f::buy(world,sim::set_building_operation(watch,false));ticks(world,100);
    check(world.building_on_fire(home) && world.building(home).fire_until_tick==2600,
        "Natural expiry boundary fixture did not ignite");
    const auto& routes=world.courier(id).dynamic_target_routes;
    const auto route=std::find_if(routes.begin(),routes.end(),[&](const auto& r){return r.first==home;});
    check(route!=routes.end() && route->second.has_value(),"Real cached shared route absent");
    const auto duration=static_cast<std::uint64_t>(route->second->size()-1)*
        static_cast<std::uint64_t>(world.active_rules().courier_edge_ticks);
    const auto departure=2600-duration+1;
    while(world.ticks()+1<departure)world.tick();
    f::buy(world,sim::set_building_operation(watch,true));
    while(world.ticks()<2600)world.tick();
    check(world.courier(id).phase==sim::CourierPhase::Returning && world.building_fire_protected(home) &&
        world.original_production_state()->fire_protection->arrivals.back().tick==2600 &&
        !world.original_production_state()->fire_protection->arrivals.back().extinguished,
        "Exact natural expiry was falsely recorded as Inspector extinguishing");
    check(sim::World::restore_original_fire_protection(world.snapshot(),fixture.policy,fixture.model).snapshot()==world.snapshot(),
        "Lawful actual Inspector arrival at exact natural expiry could not restore");
}
void earned_history_boundary_is_atomic() {
    Fixture fixture;auto world=fixture.world();
    for (const int y:{4,8,12,16}) for (int x=0;x<24;++x)
        f::buy(world,{sim::CommandType::PlaceRoad,{x,y}});
    for (int y=5;y<16;++y) if (y!=8 && y!=12)
        f::buy(world,{sim::CommandType::PlaceRoad,{20,y}});
    for (const int y:{2,6,10,14}) for (int x=0;x<15;x+=3)
        buy(world,sim::CommandType::PlaceHousehold,{x,y});
    // All witnesses come from actual monthly assessments of legally admitted
    // occupants. No journal, clock, payment, population or fire value is set.
    const auto boundary=[&] {
        return (world.ticks()+1)%sim::original_tax_interval_ticks==0 &&
            world.original_production_state()->fire_protection->taxes.size()>
                sim::original_fire_tax_limit-world.original_economy_state()->houses.size();
    };
    while (!boundary() && world.ticks()<600000) world.tick();
    check(boundary() && world.total_population()==120 && world.couriers().empty() &&
        world.original_production_state()->fire_protection->taxes.size()<=sim::original_fire_tax_limit,
        "Real20-House simulation did not earn its bounded tax-history frontier");
    const auto saved=world.snapshot();bool overflow=false;
    try {world.tick();} catch (const std::overflow_error&) {overflow=true;}
    check(overflow && world.snapshot()==saved,"Finite Rule11 history overflow partially mutated tick/funds/World");
    const auto path=fixture.root.path/"earned-history-frontier.json";
    const auto document=f::save::make_document(fixture.root.data,f::map_relative,fixture.policy->building_mask(),world);
    f::save::write_save(path,document,fixture.root.data,fixture.policy->building_mask());
    auto restored=f::save::restore_original_fire_protection(f::save::read_save(path),fixture.root.data,
        fixture.policy->building_mask(),fixture.policy,fixture.model);
    check(restored.snapshot()==saved,"Earned near-cap schema27 save could not restore completely");
    overflow=false;try {restored.tick();} catch (const std::overflow_error&) {overflow=true;}
    check(overflow && restored.snapshot()==saved,"Restored history frontier tick did not reject atomically");
    std::cout<<"Real paid20-House tax-history boundary at tick"<<world.ticks()
        <<" with"<<world.original_production_state()->fire_protection->taxes.size()<<" actual witnesses\n";
}
void expanded_cached_candidates() {
    Fixture fixture;
    constexpr int width=50,height=36;
    const auto policy=std::make_shared<const sim::MapPermissions>(width,height,3,
        std::vector<sim::MapCellPermission>(width*height,
            {true,true,false,0,sim::BuildBlocker::None,sim::BuildBlocker::None}),std::vector<sim::FixedGatePassage>{});
    auto world=sim::World::create_original_fire_protection(policy,fixture.model);
    for (const int y:{4,10,16,22,28,34}) for (int x=0;x<width;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,y}});
    for (const int y:{2,8}) for (int i=0;i<10;++i) buy(world,sim::CommandType::PlaceHousehold,{3*i,y});
    for (int i=0;i<4;++i) buy(world,sim::CommandType::PlaceClaySource,{4*i,17});
    for (int i=0;i<4;++i) buy(world,sim::CommandType::PlacePottery,{4*i,23});
    for (int i=0;i<2;++i) buy(world,sim::CommandType::PlaceWarehouse,{20+4*i,23});
    for (int i=0;i<4;++i) buy(world,sim::CommandType::PlaceMarket,{4*i,29});
    const auto watch=buy(world,sim::CommandType::PlaceFireWatch,{20,29});
    buy(world,sim::CommandType::PlaceFireWatch,{24,29});
    for (int i=0;i<2;++i) buy(world,sim::CommandType::PlaceOriginalAncestorShrine,{30+4*i,2});
    for (int i=0;i<2;++i) buy(world,sim::CommandType::PlaceOriginalWaterWell,{38+4*i,2});
    for (int i=0;i<2;++i) buy(world,sim::CommandType::PlaceHealthPost,{30+4*i,20});
    for (int i=0;i<2;++i) buy(world,sim::CommandType::PlaceServicePost,{38+4*i,20});
    const auto& c=world.courier(inspector(world,watch));
    check(world.buildings().size()==44 && c.dynamic_target_routes.size()==43 &&
        c.dynamic_target_routes.size()>sim::Rules::city_v12_building_limit && world.fire_state_valid(),
        "Rule11 retained old40/38Inspector target-cache cap or omitted other combustible Watch");
    const auto candidates=sim::road_connectivity_targets(world,c.id);
    check(candidates.size()==43 && std::all_of(world.buildings().begin(),world.buildings().end(),[&](const auto& body){
        return body.kind!=sim::Object::FireWatch || body.id==watch ||
            std::find(candidates.begin(),candidates.end(),body.id)!=candidates.end();
    }),"Read-only Inspector route diagnostics omitted the other Rule11 Watch");
    const auto saved=world.snapshot();
    check(sim::World::restore_original_fire_protection(saved,policy,fixture.model).snapshot()==saved,
        "Legally paid44-body Inspector scene failed complete restore");invariant(world);
}
}
int main() {
    try {
        paid_construction_and_limits();terrain_and_insufficient_funds();staffing_pause_and_prevention();
        actual_extinguishing_repair_and_restore();already_dispatched_from_burning_watch();arrival_on_natural_expiry();expanded_cached_candidates();earned_history_boundary_is_atomic();
        std::cout<<"Rule11 paid authored Watch/staffing/pause/real arrival/prevention/extinguishing/self-fire/repair/replay/bounds PASS\n";
        return 0;
    } catch (const std::exception& error) {std::cerr<<"Rule11 fire protection World FAIL: "<<error.what()<<'\n';return 1;}
}
