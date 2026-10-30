// Authored Rule15 contract checks. Positive movement/restore cases use only
// paid World commands and real ticks; synthetic selector cases are labelled.
#include "OriginalFinanceFixture.h"
#include "simulation/OriginalTerrainFacts.h"

#include <algorithm>
#include <iostream>

namespace {
namespace f=oe_influence_fixture;
namespace q=oe_finance_fixture;
namespace sim=openemperor::simulation;
using f::check;
struct Fixture {
    f::Root root;
    std::shared_ptr<const sim::MapPermissions> policy=std::make_shared<const sim::MapPermissions>(228,228,3,
        std::vector<sim::MapCellPermission>(228*228,
            {true,true,false,0,sim::BuildBlocker::None,sim::BuildBlocker::None}),
        std::vector<sim::FixedGatePassage>{});
    std::shared_ptr<const openemperor::original::OriginalRulesModel> model;
    std::shared_ptr<const sim::OriginalTerrainFacts> terrain=std::make_shared<const sim::OriginalTerrainFacts>(
        std::vector<std::uint32_t>(sim::OriginalTerrainFacts::cell_count,0x80U),std::string(64,'a'));
    Fixture() {
        f::write_text(root.data/"Model/EmperorBuildingModels.txt",q::authored_model());
        model=f::save::prepare_original_production_rules_model(root.data);
    }
    sim::World world() const {return sim::World::create_original_inspector_coordination(policy,model,terrain);}
    sim::World restore(const sim::World& w) const {
        return sim::World::restore_original_inspector_coordination(w.snapshot(),policy,model,terrain);
    }
};
template<class F> void refuses(F action,const char* why) {
    try {action();}catch(const std::exception&) {return;}
    throw std::runtime_error(why);
}
void ticks(sim::World& w,int count) {for(int i=0;i<count;++i)w.tick();}
sim::BuildingId buy(sim::World& w,sim::CommandType type,sim::Cell at) {
    f::buy(w,{type,at});return *w.building_owner_at(at);
}
sim::CourierId inspector(const sim::World& w,sim::BuildingId watch) {
    const auto it=std::find_if(w.couriers().begin(),w.couriers().end(),[&](const auto& c) {
        return c.owner==watch&&c.role==sim::CourierRole::FireInspector;
    });
    check(it!=w.couriers().end(),"Paid Watch lost its Inspector");return it->id;
}
struct Scene {sim::BuildingId house,first,second;sim::CourierId a,b;};
Scene scene(sim::World& w,bool pause_second=true) {
    for(int x=10;x<=22;++x)f::buy(w,{sim::CommandType::PlaceRoad,{x,12}});
    const auto house=buy(w,sim::CommandType::PlaceHousehold,{12,10});ticks(w,240);
    check(w.total_population()>=4,"Paid House did not admit both Watches' actual workers");
    const auto first=buy(w,sim::CommandType::PlaceFireWatch,{10,11});
    const auto second=buy(w,sim::CommandType::PlaceFireWatch,{18,11});
    if(pause_second)f::buy(w,sim::set_building_operation(second,false));
    return {house,first,second,inspector(w,first),inspector(w,second)};
}
void invariant(const sim::World& w) {
    check(w.population_valid()&&w.city_economy_valid()&&w.production_balance_valid()&&
        w.navigation_valid()&&w.fire_state_valid(),"Paid Rule15 invariant failed");
}
void exact_identity_and_historical_selection() {
    Fixture fxt;auto w=fxt.world();
    check(w.rule_version()==15&&w.active_rules().version==15&&w.original_inspector_coordination_enabled()&&
        w.original_cardinal_water_enabled()&&w.original_market_selection_enabled()&&
        w.original_sustainable_economy_enabled()&&w.original_fire_protection_enabled()&&
        w.original_production_enabled()&&w.original_worker_upkeep_interval()==816&&
        sim::original_food_farm_limit(w.profile(),15)==3&&w.treasury()==12000&&
        w.original_production_state()->development_revision==27,"Rule15 inherited authority changed");
    check(!sim::original_cardinal_water_enabled(w.profile(),15)&&
        !sim::original_inspector_coordination_enabled(w.profile(),14)&&
        sim::current_rule_version(sim::RulesProfile::CityV16)==4&&
        sim::current_rule_version(sim::RulesProfile::CityV11)==3,"New generation widened a historical identity/default");
    refuses([&]{sim::World ordinary(fxt.policy,sim::RulesProfile::CityV16,15);},"Ordinary constructor enabled Rule15");
    refuses([&]{(void)sim::World::create_original_inspector_coordination(fxt.policy,fxt.model,nullptr);},"Missing terrain accepted");
    auto old=sim::World::create_original_cardinal_water(fxt.policy,fxt.model,fxt.terrain);
    refuses([&]{(void)sim::World::restore_original_inspector_coordination(old.snapshot(),fxt.policy,fxt.model,fxt.terrain);},"Rule14 migrated to15");
    refuses([&]{(void)sim::World::restore_original_cardinal_water(w.snapshot(),fxt.policy,fxt.model,fxt.terrain);},"Rule15 restored through14");
    const auto old_scene=scene(old,false);old.tick();
    check(old.courier(old_scene.a).target==old_scene.house&&old.courier(old_scene.b).target==old_scene.house&&
        old.original_production_state()->fire_protection->trips.size()==2,"Historical Rule14 duplicate-target selector changed");
    auto old_restored=sim::World::restore_original_cardinal_water(old.snapshot(),fxt.policy,fxt.model,fxt.terrain);
    check(old_restored.snapshot()==old.snapshot(),"Historical duplicate physical trips stopped restoring");
    for(int i=0;i<80;++i){old.tick();old_restored.tick();check(old.snapshot()==old_restored.snapshot(),"Rule14 physical continuation diverged");}
}
void real_incoming_pause_arrival_and_no_grants() {
    Fixture fxt;auto w=fxt.world();const auto s=scene(w);w.tick();
    check(w.courier(s.a).phase==sim::CourierPhase::ToWarehouse&&w.courier(s.a).target==s.house&&
        !w.building_fire_protected(s.house),"Real departure fabricated protection or missed House");
    f::buy(w,sim::set_building_operation(s.second,true));
    const auto before=w.snapshot();
    const auto next=w.courier_dispatch_status(s.b);
    check(next.status==sim::CourierDispatchStatus::Ready&&next.selected_target==s.first&&w.snapshot()==before,
        "Second Inspector did not coalesce the actual incoming House trip purely");
    f::buy(w,sim::set_building_operation(s.first,false));
    check(w.courier_dispatch_status(s.b).selected_target==s.first,"Paused source erased a begun Inspector commitment");
    const auto edge=w.courier(s.a).edge_progress;w.tick();
    check(w.courier(s.a).edge_progress==edge+1&&!w.building_fire_protected(s.house),"Source pause stopped begun movement or granted early protection");
    auto restored=fxt.restore(w);check(restored.snapshot()==w.snapshot(),"Rule15 active trips changed on restore");
    while(!w.building_fire_protected(s.house)&&w.ticks()<300){
        w.tick();restored.tick();check(w.snapshot()==restored.snapshot(),"Rule15 actual arrival replay diverged");
    }
    check(w.building_fire_protected(s.house)&&w.fire_protection_remaining(s.house)==2400&&
        w.courier(s.a).phase==sim::CourierPhase::Returning,"Only actual arrival must grant the existing2400tick protection");
    invariant(w);
}
void real_waiting_replacement_repair_can_duplicate() {
    Fixture fxt;auto w=fxt.world();const auto s=scene(w);w.tick();
    f::buy(w,{sim::CommandType::RemoveRoad,{11,12}});ticks(w,4);
    check(w.courier(s.a).route_pending&&w.courier(s.a).edge_progress==0&&
        !w.building_fire_protected(s.house),"Real road cut did not leave an unserved waiting Inspector");
    f::buy(w,sim::set_building_operation(s.second,true));w.tick();
    check(w.courier(s.b).phase==sim::CourierPhase::ToWarehouse&&w.courier(s.b).target==s.house,
        "Road-waiting Inspector falsely blocked a reachable replacement");
    f::buy(w,{sim::CommandType::PlaceRoad,{11,12}});w.tick();
    check(!w.courier(s.a).route_pending&&w.courier(s.a).phase==sim::CourierPhase::ToWarehouse&&
        w.courier(s.a).target==s.house&&w.courier(s.b).target==s.house&&
        w.original_production_state()->fire_protection->trips.size()==2,"Ordinary repair did not preserve legal duplicate physical commitments");
    auto restored=fxt.restore(w);check(restored.snapshot()==w.snapshot(),"Rule15 imposed an invalid global no-duplicates restore invariant");
    for(int i=0;i<100;++i){w.tick();restored.tick();check(w.snapshot()==restored.snapshot(),"Repaired duplicate-trip continuation diverged");}
    invariant(w);
}
void real_burning_source_keeps_incoming_commitment() {
    Fixture fxt;auto w=fxt.world();
    for(int x=10;x<=22;++x)f::buy(w,{sim::CommandType::PlaceRoad,{x,12}});
    const auto home=buy(w,sim::CommandType::PlaceHousehold,{12,10});
    const auto first=buy(w,sim::CommandType::PlaceFireWatch,{10,11});
    f::buy(w,sim::set_building_operation(first,false));ticks(w,1998);
    const auto second=buy(w,sim::CommandType::PlaceFireWatch,{18,11});
    f::buy(w,sim::set_building_operation(second,false));f::buy(w,sim::set_building_operation(first,true));w.tick();
    const auto a=inspector(w,first),b=inspector(w,second);
    check(w.courier(a).phase==sim::CourierPhase::ToWarehouse&&w.courier(a).target==home,"Real pre-ignition departure absent");
    w.tick();check(w.building_on_fire(first)&&w.building_on_fire(home)&&w.courier(a).edge_progress==2,
        "Source ignition interrupted a begun real trip");
    f::buy(w,sim::set_building_operation(second,true));
    check(w.courier_dispatch_status(b).selected_target==first,"Burning source erased incoming commitment to another burning target");
    auto restored=fxt.restore(w);
    while(!w.building_fire_protected(home)&&w.ticks()<2050){
        w.tick();restored.tick();check(w.snapshot()==restored.snapshot(),"Burning-source continuation diverged");
    }
    check(!w.building_on_fire(home)&&w.building_fire_protected(home),"Begun trip from burning source failed to extinguish at actual arrival");
    invariant(w);
}

// Controlled synthetic selector-negative cases only. These edits are never
// saved, restored or presented as earned gameplay/physical-history evidence.
sim::CourierState& synthetic_actor(sim::World& w,sim::CourierId id) {
    return const_cast<sim::CourierState&>(w.courier(id));
}
sim::BuildingState& synthetic_body(sim::World& w,sim::BuildingId id) {
    return const_cast<sim::BuildingState&>(w.building(id));
}
void synthetic_selector_exclusions_and_priority() {
    Fixture fxt;auto base=fxt.world();const auto s=scene(base);base.tick();
    f::buy(base,sim::set_building_operation(s.second,true));
    for(int mode=0;mode<4;++mode){
        auto w=fxt.restore(base);auto& incoming=synthetic_actor(w,s.a);
        if(mode==0)incoming.enabled=false;
        if(mode==1)incoming.phase=sim::CourierPhase::Returning;
        if(mode==2)incoming.route_pending=true;
        if(mode==3)incoming.target=s.second; // Active saved trip no longer matches.
        check(w.courier_dispatch_status(s.b).selected_target==s.house,"Nonincoming/disabled/waiting/returning/mismatched actor blocked target");
    }
    {
        auto w=fxt.restore(base);
        synthetic_body(w,s.first).fire_protection_until_tick=w.ticks()+100;
        check(w.courier_dispatch_status(s.b).status==sim::CourierDispatchStatus::NoTarget,
            "All incoming/protected targets did not produce NoTarget");
        synthetic_body(w,s.first).fire_protection_until_tick=w.ticks();
        check(w.courier_dispatch_status(s.b).selected_target==s.first,"Protection expiry boundary remained incorrectly covered");
    }
    {
        auto w=fxt.restore(base);
        synthetic_actor(w,s.a).enabled=false;
        synthetic_body(w,s.first).fire_until_tick=w.ticks()+600;
        check(w.courier_dispatch_status(s.b).selected_target==s.first,
            "Unserved burning target lost priority to the earlier nonburning House");
    }
    {
        auto w=fxt.restore(base);
        // Both burn; the House is already physically incoming. Only the other
        // burning Watch remains unserved, despite an otherwise future deadline.
        synthetic_body(w,s.house).fire_until_tick=w.ticks()+600;
        synthetic_body(w,s.first).fire_until_tick=w.ticks()+600;
        synthetic_body(w,s.first).fire_protection_until_tick=w.ticks()+100;
        const auto snapshot=w.snapshot();
        check(w.courier_dispatch_status(s.b).selected_target==s.first&&w.snapshot()==snapshot,
            "Burning-first did not prioritize the unserved target or query granted coverage");
    }
    {
        auto w=fxt.restore(base);
        synthetic_actor(w,s.a).enabled=false;
        synthetic_body(w,s.house).fire_protection_until_tick=w.ticks()+100;
        check(w.courier_dispatch_status(s.b).selected_target==s.first,"Real nonburning protection was not skipped");
        synthetic_body(w,s.house).fire_protection_until_tick=w.ticks();
        synthetic_actor(w,s.b).last_dispatched_target=s.house;
        check(w.courier_dispatch_status(s.b).selected_target==s.first,"Complete canonical cursor did not retain its next target");
    }
}
}
int main(){
    try{
        exact_identity_and_historical_selection();real_incoming_pause_arrival_and_no_grants();
        real_waiting_replacement_repair_can_duplicate();real_burning_source_keeps_incoming_commitment();
        synthetic_selector_exclusions_and_priority();
        std::cout<<"Inspector coordination World checks passed\n";
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
