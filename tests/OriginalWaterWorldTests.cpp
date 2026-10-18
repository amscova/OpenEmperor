#include "OriginalProductionFixture.h"
#include "simulation/original/OriginalScheduler.h"
#include "simulation/original/ServiceRules.h"

#include <algorithm>
#include <array>
#include <iostream>

namespace {
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
using f::check;

const sim::OriginalHouseState& house(const sim::World& world,sim::BuildingId id) {
    for (const auto& row:world.original_economy_state()->houses) if (row.building_id==id) return row;
    throw std::runtime_error("Water test lost a paid House");
}
const sim::OriginalWaterHouseState& water(const sim::World& world,sim::BuildingId id) {
    for (const auto& row:world.original_production_state()->water.houses) if (row.building_id==id) return row;
    throw std::runtime_error("Water test lost a House authority row");
}
sim::CourierId actor(const sim::World& world,sim::BuildingId owner) {
    for (const auto& row:world.couriers()) if (row.owner==owner) {
        check(row.role==sim::CourierRole::OriginalWater,"Water source owns an unrelated actor");return row.id;
    }
    throw std::runtime_error("Paid Water source lost its actor");
}
const sim::OriginalWaterTrip& trip(const sim::World& world,sim::CourierId id) {
    for (const auto& row:world.original_production_state()->water.trips) if (row.transfer.courier_id==id) return row;
    throw std::runtime_error("Water test lost a real dispatched trip");
}
sim::BuildingId buy(sim::World& world,sim::CommandType type,sim::Cell cell) {
    f::buy(world,{type,cell});return *world.building_owner_at(cell);
}
void authority(const sim::World& world) {
    check(world.production_balance_valid()&&world.population_valid()&&world.navigation_valid()&&world.city_economy_valid(),
          "Water World invariant failed");
    const auto& state=world.original_production_state()->water;
    check(state.providers.size()<=2&&state.trips.size()<=2&&state.houses.size()==world.original_economy_state()->houses.size(),
          "Water bounded single authority changed");
    for (const auto& h:world.original_economy_state()->houses) {
        const auto& row=water(world,h.building_id);
        const auto expected=row.current_witness ? sim::original_water_timer_at(*row.current_witness,world.ticks()):0;
        const bool evaluated=row.evaluation_witness&&sim::original_water_timer_at(*row.evaluation_witness,
            sim::original_water_evaluation_tick(world.ticks()))>0;
        check(row.normal_timer==expected&&!row.special_timer&&h.house.water==evaluated,
              "Water raw timer/materialization lost its separate real clocks");
        for (unsigned role=0;role<9;++role)
            check(!orig::service_timer(h.house,static_cast<orig::ServiceRole>(role)),"Water visit granted a service role");
    }
    for (const auto& c:world.couriers())
        check(c.role==sim::CourierRole::OriginalWater&&!c.cargo&&!c.reserved&&!world.original_courier_cargo(c.id),
              "Water actor acquired material cargo");
}
void ticks(sim::World& world,int count) {for (int i=0;i<count;++i) {world.tick();authority(world);}}
template<class Predicate> void until(sim::World& world,Predicate done,int limit=20000) {
    for (int i=0;i<limit&&!done();++i) {world.tick();authority(world);}
    check(done(),"Paid Water event did not occur within finite observation");
}
struct Fixture {
    f::Root root;
    std::shared_ptr<const sim::MapPermissions> policy=f::permissions();
    std::shared_ptr<const orig::OriginalRulesModel> model;
    explicit Fixture(int capacity=7) {
        oe_production_fixture::write_model(root.data,capacity);
        model=f::save::prepare_original_production_rules_model(root.data);
    }
    sim::World world() const {return sim::World::create_original_production_development(policy,model);}
    sim::World restore(const sim::World& world) const {
        return sim::World::restore_original_production_development(world.snapshot(),policy,model);
    }
};
void roads(sim::World& world) {for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,4}});}
void homes(sim::World& world,int count=6) {
    for (int i=0;i<count;++i) (void)buy(world,sim::CommandType::PlaceHousehold,{i*3,2});
}
void paid_arrival_materialization_expiry_and_replay() {
    Fixture fixture;auto world=fixture.world();roads(world);homes(world);
    const auto home=*world.building_owner_at({0,2});
    const auto source=buy(world,sim::CommandType::PlaceOriginalWaterWell,{21,5});const auto id=actor(world,source);
    check(world.total_population()==0&&world.construction_spent_total()==553&&world.treasury()==11447&&
          world.workforce_required(source)==4&&world.couriers().size()==1&&
          world.original_influence_state()->sources.empty(),"Water purchase doubled accounting or granted population/influence identity");
    check(sim::building_footprint(world.profile(),world.rule_version(),sim::Object::OriginalWaterWell72)==sim::BuildingFootprint{2,2},
          "Water body lost its model72 footprint");
    check(!house(world,home).house.water&&world.original_production_state()->water.trips.empty(),"Water purchase granted coverage or a trip");
    ticks(world,119);check(world.original_production_state()->water.trips.empty(),"Unstaffed Water source dispatched");
    until(world,[&]{return world.courier(id).phase==sim::CourierPhase::ToWarehouse&&world.courier(id).edge_progress>0;});
    check(!water(world,home).current_witness&&!house(world,home).house.water,"Water dispatch granted coverage");
    const auto moving=world.snapshot().couriers;f::buy(world,sim::set_building_operation(source,false));
    check(world.snapshot().couriers==moving,"Water source pause cancelled the begun tour");
    until(world,[&]{return trip(world,id).visit_applied;});
    const auto arrival=*trip(world,id).arrival_tick;
    check(trip(world,id).target==home&&water(world,home).normal_timer==(arrival%51==36 ? 95:96),
          "Actual Water arrival failed to assign96 before the possible same-tick countdown");
    if (sim::original_water_evaluation_tick(arrival)<arrival)
        check(!house(world,home).house.water&&!water(world,home).evaluation_witness,"Arrival prematurely materialized Water");
    until(world,[&]{return house(world,home).house.water;},51);
    check(water(world,home).evaluation_witness==water(world,home).current_witness&&
          orig::needs_met(fixture.model->house_rules(),orig::ModelHouse::Hut,house(world,home).house),
          "Phase28 failed to expose actual Water to the pure House need");
    until(world,[&]{return world.courier(id).phase==sim::CourierPhase::IdleAtWorkshop;});
    auto replay=fixture.restore(world);std::array<bool,16> steps{};bool expired=false,delayed_clear=false;
    const auto witness=water(world,home).current_witness;
    for (int i=0;i<20000;++i) {
        const auto dispatch=orig::next_scheduler_dispatch(world.ticks());const auto before=water(world,home).normal_timer;
        world.tick();replay.tick();authority(world);
        check(world.snapshot()==replay.snapshot(),"Full Water20k replay diverged");
        const auto after=water(world,home).normal_timer;
        if (dispatch&&dispatch->service_countdown) steps[dispatch->position.step]=true;
        check(after==(dispatch&&dispatch->service_countdown&&before ? before-1:before),"Water timer debited twice or on another phase");
        if (before==1&&after==0) {expired=true;check(house(world,home).house.water,"Timer expiry cleared Water before Phase28");}
        if (expired&&!house(world,home).house.water) delayed_clear=true;
    }
    check(expired&&delayed_clear&&std::all_of(steps.begin(),steps.end(),[](bool value){return value;})&&
          water(world,home).current_witness==witness&&!house(world,home).house.water,
          "Water missed an expiry/evaluation boundary or one of16 scheduler steps");
    f::buy(world,sim::set_building_operation(source,true));
    until(world,[&]{return water(world,home).current_witness!=witness;});
    until(world,[&]{return house(world,home).house.water;},51);
}
void gates_roads_limits_and_old_influence() {
    Fixture fixture;
    for (const int count:{0,6}) {
        auto world=fixture.world();roads(world);homes(world,count);
        (void)buy(world,sim::CommandType::PlaceOriginalWaterWell,{21,count ? 8:5});ticks(world,900);
        check(world.original_production_state()->water.trips.empty(),"Unstaffed/disconnected source manufactured a Water trip");
    }
    Fixture small(3);auto partial=small.world();roads(partial);homes(partial,1);
    const auto understaffed=buy(partial,sim::CommandType::PlaceOriginalWaterWell,{21,5});ticks(partial,900);
    check(partial.total_population()==3&&partial.workers_assigned(understaffed)<4&&partial.original_production_state()->water.trips.empty(),
          "Partially staffed Water source dispatched");
    auto world=fixture.world();roads(world);homes(world);
    const auto source=buy(world,sim::CommandType::PlaceOriginalWaterWell,{21,5});
    f::buy(world,sim::set_building_operation(source,false));ticks(world,400);
    check(world.original_production_state()->water.trips.empty(),"Paused Water source dispatched");
    f::buy(world,sim::set_building_operation(source,true));const auto id=actor(world,source);
    until(world,[&]{return world.courier(id).phase==sim::CourierPhase::ToWarehouse&&world.courier(id).edge_progress>0;});
    const sim::Command cut{sim::CommandType::RemoveRoad,{12,4}};
    until(world,[&]{return world.validate(cut).accepted;});f::buy(world,cut);ticks(world,250);
    check(!trip(world,id).arrival_tick,"Water actor arrived through a broken road");
    auto replay=fixture.restore(world);f::buy(world,{sim::CommandType::PlaceRoad,{12,4}});f::buy(replay,{sim::CommandType::PlaceRoad,{12,4}});
    for (int i=0;i<20000&&!trip(world,id).visit_applied;++i) {world.tick();replay.tick();authority(world);check(world.snapshot()==replay.snapshot(),"Paid Water repair replay diverged");}
    check(trip(world,id).visit_applied,"Paid repair did not restore actual Water arrival");
    (void)buy(world,sim::CommandType::PlaceOriginalWaterWell,{18,5});
    const auto before=world.snapshot();const sim::Command third{sim::CommandType::PlaceOriginalWaterWell,{15,5}};
    check(!world.validate(third).accepted&&!world.execute(third).accepted&&world.snapshot()==before,"Third Water source changed authority");
    const auto removal=sim::demolish_building(source);
    check(!world.validate(removal).accepted&&!world.execute(removal).accepted&&world.snapshot()==before,"Water V1 demolition changed authority");
    auto neutral=fixture.world();roads(neutral);homes(neutral);const auto funds=neutral.treasury();
    const auto old=buy(neutral,sim::CommandType::PlaceOriginalInfluenceSource,{21,5});
    check(neutral.treasury()==funds-60&&neutral.couriers().empty()&&neutral.workforce_required(old)==0,"Historical influence source was reinterpreted as Water");
    ticks(neutral,1200);f::buy(neutral,sim::demolish_building(old));(void)buy(neutral,sim::CommandType::PlaceOriginalInfluenceSource,{21,5});
    auto restored=fixture.restore(neutral);ticks(restored,500);
    check(restored.original_production_state()->water.providers.empty()&&restored.couriers().empty(),"Influence remove/rebuy/reload granted a Water provider");
    for (const auto& h:restored.original_economy_state()->houses) check(!h.house.water&&!water(restored,h.building_id).normal_timer,"Old influence source supplied Water");
}
void aligned_arrivals_and_retained_evaluation() {
    Fixture fixture(37);auto base=fixture.world();roads(base);homes(base,1);ticks(base,900);
    const auto home=*base.building_owner_at({0,2});
    const auto first_arrival=[&](sim::World& world) {
        const auto source=buy(world,sim::CommandType::PlaceOriginalWaterWell,{3,5});const auto id=actor(world,source);
        until(world,[&]{return !world.original_production_state()->water.trips.empty()&&trip(world,id).arrival_tick.has_value();});
        return *trip(world,id).arrival_tick;
    };
    auto probe=fixture.restore(base);const auto initial=first_arrival(probe);
    for (const unsigned residue:{28u,29u,30u,35u,36u,37u}) {
        auto world=fixture.restore(base);ticks(world,static_cast<int>((residue+51-initial%51)%51));
        const auto arrived=first_arrival(world);
        check(arrived%51==residue&&water(world,home).normal_timer==(residue==36 ? 95:96),"Water arrival/countdown boundary changed");
        check(house(world,home).house.water==(residue==29),"Water arrival materialized outside actual Phase28");
        until(world,[&]{return house(world,home).house.water;},51);
    }
    auto world=fixture.restore(base);(void)first_arrival(world);
    until(world,[&]{const auto& row=water(world,home);return row.current_witness&&row.evaluation_witness&&
        row.current_witness!=row.evaluation_witness&&row.current_witness->arrival_tick>sim::original_water_evaluation_tick(world.ticks());},1800);
    const auto saved=world.snapshot();auto restored=fixture.restore(world);
    const auto prior=water(world,home).evaluation_witness;
    check(prior&&house(world,home).house.water,"New actual visit discarded the older positive evaluation origin");
    for (int i=0;i<60;++i) {world.tick();restored.tick();authority(world);check(world.snapshot()==restored.snapshot(),"Overwritten-current/retained-evaluation replay diverged");}
    check(saved.original_production->water.houses.front().current_witness!=saved.original_production->water.houses.front().evaluation_witness,
          "Retained-evaluation test did not exercise distinct actual origins");
}
void source_fire_and_target_fire() {
    Fixture fixture;auto world=fixture.world();roads(world);
    const auto source=buy(world,sim::CommandType::PlaceOriginalWaterWell,{21,5});f::buy(world,sim::set_building_operation(source,false));
    ticks(world,600);homes(world);while(world.ticks()<1990)world.tick();
    f::buy(world,sim::set_building_operation(source,true));const auto id=actor(world,source);
    until(world,[&]{return world.courier(id).phase==sim::CourierPhase::ToWarehouse&&world.courier(id).edge_progress>0;});
    until(world,[&]{return world.building_on_fire(source);},30);
    check(!trip(world,id).arrival_tick,"Source-fire Water scene arrived before its incident");
    until(world,[&]{return trip(world,id).arrival_tick.has_value();},500);
    check(trip(world,id).visit_applied&&world.building_on_fire(source),"Later source fire cancelled Water arrival");
    until(world,[&]{return world.courier(id).phase==sim::CourierPhase::IdleAtWorkshop;},500);
    const auto sequence=trip(world,id).transfer.sequence;ticks(world,30);
    check(trip(world,id).transfer.sequence==sequence,"Burning Water source dispatched again");
    auto target=fixture.world();roads(target);homes(target);ticks(target,1000);
    const auto delayed=buy(target,sim::CommandType::PlaceOriginalWaterWell,{21,5});f::buy(target,sim::set_building_operation(delayed,false));
    while(target.ticks()<1990) target.tick();
    f::buy(target,sim::set_building_operation(delayed,true));const auto visitor=actor(target,delayed);
    until(target,[&]{return !target.original_production_state()->water.trips.empty()&&trip(target,visitor).arrival_tick.has_value();},500);
    check(target.building_on_fire(trip(target,visitor).target)&&!trip(target,visitor).visit_applied,"Burning target received Water");
    for(const auto& row:target.original_production_state()->water.houses)check(!row.current_witness,"Burning target created a Water Witness");
}
void malformed_water_is_rejected_without_live_mutation() {
    Fixture fixture;auto world=fixture.world();roads(world);homes(world,1);
    const auto source=buy(world,sim::CommandType::PlaceOriginalWaterWell,{3,5});const auto id=actor(world,source);
    until(world,[&]{return !world.original_production_state()->water.trips.empty()&&trip(world,id).visit_applied;});
    f::buy(world,sim::set_building_operation(source,false));until(world,[&]{return house(world,*world.building_owner_at({0,2})).house.water;},51);
    const auto good=world.snapshot();
    const auto reject=[&](auto change) {
        auto bad=good;change(bad);bool refused=false;
        try {(void)sim::World::restore_original_production_development(bad,fixture.policy,fixture.model);}catch(const std::exception&){refused=true;}
        check(refused&&world.snapshot()==good,"Malformed Water restored or changed the live World");
    };
    reject([](auto& s){++s.original_production->water.houses[0].normal_timer;});
    reject([](auto& s){s.original_production->water.houses[0].special_timer=1;});
    reject([](auto& s){s.original_production->water.houses[0].current_witness.reset();});
    reject([](auto& s){s.original_production->water.houses[0].evaluation_witness.reset();});
    reject([](auto& s){s.original_economy->houses[0].house.water=false;});
    reject([](auto& s){s.original_production->water.houses[0].current_witness->arrival_tick=UINT64_MAX;});
    reject([](auto& s){s.original_production->water.houses[0].evaluation_witness->arrival_tick=s.ticks+1;});
    reject([](auto& s){s.original_production->water.houses[0].current_witness->transfer.sequence=0;});
    reject([](auto& s){s.original_production->water.providers[0].building_id=s.original_economy->houses[0].building_id;});
    reject([](auto& s){s.original_production->water.trips[0].dispatched_tick=0;});
    reject([](auto& s){s.original_production->water.trips[0].visit_applied=false;});
    reject([](auto& s){s.original_production->housing.houses[0].first_admission_tick=s.ticks;});
    reject([](auto& s){s.ticks=0;});
    reject([](auto& s){s.ticks=UINT64_MAX;});
    check(sim::original_water_countdowns(UINT64_MAX)==1+(UINT64_MAX-36)/51&&
          sim::original_water_evaluation_tick(UINT64_MAX)==UINT64_MAX-((UINT64_MAX-29)%51),"Water clock arithmetic overflowed");
    const sim::OriginalWaterWitness future{};
    check(!sim::original_water_timer_at(future,UINT64_MAX),"Missing arrival granted terminal-clock Water");
}
} // namespace
int main() {
    try {
        paid_arrival_materialization_expiry_and_replay();gates_roads_limits_and_old_influence();
        aligned_arrivals_and_retained_evaluation();source_fire_and_target_fire();malformed_water_is_rejected_without_live_mutation();
        std::cout<<"Original Water World tests PASS\n";return 0;
    }catch(const std::exception& error){std::cerr<<"Original Water World tests FAIL: "<<error.what()<<'\n';return 1;}
}
