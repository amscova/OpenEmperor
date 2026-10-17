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

std::uint64_t countdowns(std::uint64_t tick) {return tick<36 ? 0:1+(tick-36)/51;}
const orig::HouseState& house(const sim::World& world,sim::BuildingId id) {
    for (const auto& row:world.original_economy_state()->houses)
        if (row.building_id==id) return row.house;
    throw std::runtime_error("Medical test lost a paid House");
}
const sim::OriginalServiceTrip& trip(const sim::World& world,sim::CourierId id) {
    for (const auto& row:world.original_production_state()->services.trips)
        if (row.transfer.courier_id==id) return row;
    throw std::runtime_error("Medical test lost an actual dispatched trip");
}
sim::CourierId actor(const sim::World& world,sim::BuildingId owner) {
    for (const auto& c:world.couriers()) if (c.owner==owner) return c.id;
    throw std::runtime_error("Paid medical source has no Courier");
}
sim::BuildingId buy(sim::World& world,sim::CommandType type,sim::Cell cell) {
    f::buy(world,{type,cell});return *world.building_owner_at(cell);
}
void actual_authority(const sim::World& world) {
    check(world.production_balance_valid()&&world.population_valid()&&world.navigation_valid()&&world.city_economy_valid(),
          "Medical World invariant failed");
    const auto& state=world.original_production_state()->services;
    check(state.providers.size()<=4&&state.trips.size()<=4&&state.witnesses.size()<=40,
          "Medical latest-history bounds exceeded");
    for (const auto& h:world.original_economy_state()->houses) {
        for (unsigned index=0;index<9;++index) {
            const auto role=static_cast<orig::ServiceRole>(index);
            std::uint8_t expected=0;
            for (const auto& witness:state.witnesses) if (witness.building_id==h.building_id&&witness.role==role) {
                check(witness.arrival_tick>0&&witness.arrival_tick<=world.ticks(),"Medical witness has an impossible arrival");
                const auto elapsed=countdowns(world.ticks())-countdowns(witness.arrival_tick-1);
                expected=elapsed>=96 ? 0:static_cast<std::uint8_t>(96-elapsed);
            }
            check(orig::service_timer(h.house,role)==expected,"House timer lacks its exact actual-arrival/phase35 origin");
        }
    }
    for (const auto& b:world.buildings())
        check(!b.service_until_tick&&!b.health_protection_until_tick&&!b.health_risk&&!b.sick_until_tick,
              "Original medical visit changed legacy service/health authority");
    for (const auto& c:world.couriers())
        check(!c.cargo&&!c.reserved&&!world.original_courier_cargo(c.id),"Medical Courier created material cargo");
}
void ticks(sim::World& world,int count) {
    for (int i=0;i<count;++i) {world.tick();actual_authority(world);}
}
template<class Predicate> void until(sim::World& world,Predicate done,int limit=20000) {
    for (int i=0;i<limit&&!done();++i) {world.tick();actual_authority(world);}
    check(done(),"Paid medical event did not occur within finite observation");
}
struct Fixture {
    f::Root root;
    std::shared_ptr<const sim::MapPermissions> policy=f::permissions();
    std::shared_ptr<const orig::OriginalRulesModel> model;
    explicit Fixture(int herbalist_need=0,int acupuncture_need=0,int capacity=7) {
        oe_production_fixture::write_model(root.data,capacity,0,0,0,herbalist_need,acupuncture_need);
        model=f::save::prepare_original_production_rules_model(root.data);
    }
    sim::World world() const {return sim::World::create_original_production_development(policy,model);}
    sim::World restore(const sim::World& world) const {
        return sim::World::restore_original_production_development(world.snapshot(),policy,model);
    }
};
struct Scene {sim::BuildingId herbalist{},acupuncture{},first_house{};};
void roads(sim::World& world) {
    for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,4}});
}
void homes(sim::World& world,int count=6) {
    for (int index=0;index<count;++index) (void)buy(world,sim::CommandType::PlaceHousehold,{index*3,2});
}
Scene scene(sim::World& world,bool connected=true,int house_count=6) {
    roads(world);homes(world,house_count);
    Scene s;
    if (house_count) s.first_house=*world.building_owner_at({0,2});
    s.herbalist=buy(world,sim::CommandType::PlaceHealthPost,{18,connected ? 5:8});
    s.acupuncture=buy(world,sim::CommandType::PlaceServicePost,{21,connected ? 5:8});
    return s;
}
void paid_arrivals_pause_expiry_revisit_and_needs() {
    Fixture fixture(1,1);auto world=fixture.world();const auto s=scene(world);
    check(world.original_production_state()->development_revision==21,"Medical fixture requires active revision21");
    check(world.total_population()==0&&
          world.construction_spent_total()==608&&world.treasury()==11392,
          "Medical construction did not pay exact24Roads/sixHouses/30+50 model costs");
    check(world.original_production_state()->services.providers.size()==2&&world.couriers().size()==2&&
          world.workforce_required(s.herbalist)==7&&world.workforce_required(s.acupuncture)==8,
          "Medical body/provider/actor/model-workforce binding changed");
    check(sim::building_footprint(world.profile(),world.rule_version(),sim::Object::HealthPost)==sim::BuildingFootprint{2,2}&&
          sim::building_footprint(world.profile(),world.rule_version(),sim::Object::ServicePost)==sim::BuildingFootprint{2,2},
          "Paid medical source footprint differs from its2x2 contract");
    check(world.original_production_state()->services.trips.empty()&&world.original_production_state()->services.witnesses.empty(),
          "Purchase granted medical dispatch or coverage");
    ticks(world,119);check(world.original_production_state()->services.trips.empty(),"Medical source dispatched without real workers");
    const auto herbs=actor(world,s.herbalist),acu=actor(world,s.acupuncture);
    until(world,[&] {return world.courier(herbs).phase==sim::CourierPhase::ToWarehouse&&
        world.courier(acu).phase==sim::CourierPhase::ToWarehouse&&world.courier(acu).edge_progress>0;});
    check(world.original_production_state()->services.witnesses.empty(),"Outbound movement granted a service timer");
    const auto outgoing=world.snapshot().couriers;
    f::buy(world,sim::set_building_operation(s.herbalist,false));
    f::buy(world,sim::set_building_operation(s.acupuncture,false));
    check(world.snapshot().couriers==outgoing,
          "Pausing medical source cancelled an already dispatched tour");
    until(world,[&] {return trip(world,herbs).visit_applied;});
    check(house(world,s.first_house).herbalist>0&&!house(world,s.first_house).acupuncture&&
          !orig::needs_met(fixture.model->house_rules(),orig::ModelHouse::Shelter,house(world,s.first_house)),
          "Herbalist alone silently satisfied the separate Acupuncture requirement");
    until(world,[&] {return world.courier(herbs).phase==sim::CourierPhase::IdleAtWorkshop&&
        world.courier(acu).phase==sim::CourierPhase::IdleAtWorkshop;});
    check(trip(world,herbs).visit_applied&&trip(world,acu).visit_applied&&
          trip(world,herbs).target==s.first_house&&trip(world,acu).target==s.first_house,
          "Paused medical tours did not finish their fixed actual destinations");
    const auto& rules=fixture.model->house_rules();
    check(orig::needs_met(rules,orig::ModelHouse::Shelter,house(world,s.first_house)),
          "Actual two-category medical arrivals did not satisfy matching needs");
    const auto witnesses=world.original_production_state()->services.witnesses;
    auto replay=fixture.restore(world);
    std::array<bool,16> phase_steps{};
    for (int i=0;i<20000;++i) {
        const auto dispatch=orig::next_scheduler_dispatch(world.ticks());
        const auto before=house(world,s.first_house);world.tick();replay.tick();actual_authority(world);
        check(world.snapshot()==replay.snapshot(),"Medical countdown/replay diverged");
        if (dispatch&&dispatch->service_countdown) phase_steps[dispatch->position.step]=true;
        const auto after=house(world,s.first_house);
        for (const auto role:{orig::ServiceRole::Herbalist,orig::ServiceRole::Acupuncture}) {
            const auto old=orig::service_timer(before,role),now=orig::service_timer(after,role);
            const auto expected=dispatch&&dispatch->service_countdown&&old>0 ? old-1:old;
            check(now==expected,"Phase35 did not debit precisely once, or another phase debited service");
        }
    }
    check(std::all_of(phase_steps.begin(),phase_steps.end(),[](bool hit){return hit;}),"Not all16 scheduler steps executed service countdown");
    check(world.original_production_state()->services.witnesses==witnesses&&
          !house(world,s.first_house).herbalist&&!house(world,s.first_house).acupuncture&&
          !orig::needs_met(rules,orig::ModelHouse::Shelter,house(world,s.first_house)),
          "Idle/return/expired medical timer granted or retained coverage");
    f::buy(world,sim::set_building_operation(s.herbalist,true));
    f::buy(world,sim::set_building_operation(s.acupuncture,true));
    until(world,[&] {return house(world,s.first_house).herbalist>0&&house(world,s.first_house).acupuncture>0;});
    check(trip(world,herbs).transfer.sequence>1&&trip(world,acu).transfer.sequence>1&&
          orig::needs_met(rules,orig::ModelHouse::Shelter,house(world,s.first_house)),"Actual repeated tours did not renew medical needs");
}
void source_and_route_gates() {
    Fixture fixture;
    for (const int house_count:{0,2,6}) {
        auto world=fixture.world();const auto s=scene(world,house_count!=6,house_count);ticks(world,900);
        const auto& state=world.original_production_state()->services;
        if (house_count==0) check(state.trips.empty()&&state.witnesses.empty(),"Uninhabited World fabricated medical trips");
        if (house_count==2) {
            check(world.workers_assigned(s.acupuncture)<world.workforce_required(s.acupuncture)&&
                  world.courier(actor(world,s.acupuncture)).phase==sim::CourierPhase::IdleAtWorkshop,
                  "Partially staffed Acupuncture source dispatched");
            for (const auto& witness:state.witnesses)check(witness.role!=orig::ServiceRole::Acupuncture,"Incomplete staffing granted Acupuncture");
        }
        if (house_count==6) check(state.trips.empty()&&state.witnesses.empty(),"Medical visitor crossed missing source roads");
    }
    auto world=fixture.world();const auto s=scene(world);
    f::buy(world,sim::set_building_operation(s.herbalist,false));
    f::buy(world,sim::set_building_operation(s.acupuncture,false));ticks(world,400);
    check(world.workforce_supply()==42&&world.original_production_state()->services.trips.empty(),"Paused staffed medical provider dispatched");
    f::buy(world,sim::set_building_operation(s.herbalist,true));
    const auto id=actor(world,s.herbalist);
    until(world,[&]{return world.courier(id).phase==sim::CourierPhase::ToWarehouse&&world.courier(id).edge_progress>0;});
    const sim::Command cut{sim::CommandType::RemoveRoad,{12,4}};
    until(world,[&]{return world.validate(cut).accepted;});f::buy(world,cut);
    ticks(world,250);check(!trip(world,id).arrival_tick&&world.original_production_state()->services.witnesses.empty(),
          "Medical visitor granted coverage across an interrupted road");
    f::buy(world,{sim::CommandType::PlaceRoad,{12,4}});
    until(world,[&]{return trip(world,id).visit_applied;});
    check(house(world,s.first_house).herbalist>0,"Paid road repair failed to recover real medical arrival");
}
void natural_source_fire_and_burning_target() {
    Fixture fixture;
    // A source bought600 ticks before its Houses burns first; its already
    // dispatched visitor must finish at a still unburnt actual destination.
    auto world=fixture.world();roads(world);
    const auto provider=buy(world,sim::CommandType::PlaceHealthPost,{21,5});
    f::buy(world,sim::set_building_operation(provider,false));ticks(world,600);homes(world);
    while (world.ticks()<1990) world.tick();
    f::buy(world,sim::set_building_operation(provider,true));const auto id=actor(world,provider);
    until(world,[&]{return world.courier(id).phase==sim::CourierPhase::ToWarehouse&&world.courier(id).edge_progress>0;});
    until(world,[&]{return world.building_on_fire(provider);},30);
    check(!trip(world,id).arrival_tick,"Fire-after-dispatch scene arrived before the source incident");
    until(world,[&]{return trip(world,id).arrival_tick.has_value();},500);
    check(trip(world,id).visit_applied&&world.building_on_fire(provider),"Later source fire cancelled an actual medical visit");
    until(world,[&]{return world.courier(id).phase==sim::CourierPhase::IdleAtWorkshop;},500);
    const auto sequence=trip(world,id).transfer.sequence;
    ticks(world,30);check(trip(world,id).transfer.sequence==sequence&&
        world.courier_dispatch_status(id).status==sim::CourierDispatchStatus::OnFire,"Burning source dispatched another medical tour");

    auto target=fixture.world();roads(target);homes(target);ticks(target,1000);
    const auto delayed=buy(target,sim::CommandType::PlaceHealthPost,{21,5});
    f::buy(target,sim::set_building_operation(delayed,false));while (target.ticks()<1990)target.tick();
    f::buy(target,sim::set_building_operation(delayed,true));const auto visitor=actor(target,delayed);
    until(target,[&]{return target.courier(visitor).phase==sim::CourierPhase::ToWarehouse&&target.courier(visitor).edge_progress>0;});
    until(target,[&]{return trip(target,visitor).arrival_tick.has_value();},500);
    check(target.building_on_fire(trip(target,visitor).target)&&!trip(target,visitor).visit_applied&&
          target.original_production_state()->services.witnesses.empty(),"Burning target House received a medical visit");
}
void actual_arrival_on_countdown_tick() {
    Fixture fixture(1,0,37);auto base=fixture.world();roads(base);
    const auto home=buy(base,sim::CommandType::PlaceHousehold,{0,2});ticks(base,900);
    const auto arrival=[&](sim::World& world) {
        const auto provider=buy(world,sim::CommandType::PlaceHealthPost,{21,5});const auto id=actor(world,provider);
        until(world,[&]{return !world.original_production_state()->services.trips.empty()&&trip(world,id).arrival_tick.has_value();});
        return *trip(world,id).arrival_tick;
    };
    // Shift only the paid source placement/dispatch clock, never actor motion,
    // House timers or Worldticks through a positive snapshot mutation.
    auto probe=fixture.restore(base);const auto first=arrival(probe);
    auto world=fixture.restore(base);const auto delay=static_cast<int>((36+51-first%51)%51);ticks(world,delay);
    const auto at=arrival(world);
    check(at%51==36&&house(world,home).herbalist==95,
          "Actual phase35 arrival did not assign96 then count down exactly to95");
    check(orig::needs_met(fixture.model->house_rules(),orig::ModelHouse::Shelter,house(world,home)),
          "Matching Herbalist need ignored an actual same-tick visit");
    const auto wrong=house(world,home);check(!wrong.acupuncture,"Herbalist arrival granted another medical category");
}
}
int main() {
    try {
        paid_arrivals_pause_expiry_revisit_and_needs();source_and_route_gates();
        natural_source_fire_and_burning_target();actual_arrival_on_countdown_tick();
        std::cout<<"Revision19 paid207/208 arrivals, source/target/road gates, exact phase35 decay and replay PASS\n";
        return 0;
    } catch (const std::exception& error) {std::cerr<<"Original Services World FAIL: "<<error.what()<<'\n';return 1;}
}
