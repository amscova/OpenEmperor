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
    throw std::runtime_error("Ancestor test lost a paid House");
}
sim::CourierId actor(const sim::World& world,sim::BuildingId owner) {
    for (const auto& c:world.couriers()) if (c.owner==owner) {
        check(c.role==sim::CourierRole::OriginalAncestor,"Shrine owns an unrelated Courier role");
        return c.id;
    }
    throw std::runtime_error("Paid Ancestor shrine has no Courier");
}
const sim::OriginalServiceTrip& trip(const sim::World& world,sim::CourierId id) {
    for (const auto& row:world.original_production_state()->services.trips)
        if (row.transfer.courier_id==id) return row;
    throw std::runtime_error("Ancestor test lost a real dispatched trip");
}
sim::BuildingId buy(sim::World& world,sim::CommandType type,sim::Cell cell) {
    f::buy(world,{type,cell});return *world.building_owner_at(cell);
}
void authority(const sim::World& world) {
    check(world.production_balance_valid()&&world.population_valid()&&world.navigation_valid()&&world.city_economy_valid(),
          "Ancestor World invariant failed");
    const auto& state=world.original_production_state()->services;
    check(state.providers.size()<=6&&state.trips.size()<=6&&state.witnesses.size()<=60,
          "Ancestor bounded service history exceeded");
    for (const auto& h:world.original_economy_state()->houses) {
        std::uint8_t expected=0;
        for (const auto& w:state.witnesses) if (w.building_id==h.building_id) {
            check(w.role==orig::ServiceRole::Ancestor&&w.arrival_tick>0&&w.arrival_tick<=world.ticks(),
                  "Ancestor-only World acquired an unrelated or impossible visit");
            const auto elapsed=countdowns(world.ticks())-countdowns(w.arrival_tick-1);
            expected=elapsed>=40 ? 0:static_cast<std::uint8_t>(40-elapsed);
        }
        check(h.house.ancestor==expected,"Ancestor timer lost its actual arrival and phase35 origin");
        for (unsigned i=0;i<9;++i) if (i!=5)
            check(!orig::service_timer(h.house,static_cast<orig::ServiceRole>(i)),
                  "Ancestor visit granted an unrelated service category");
    }
    for (const auto& c:world.couriers())
        check(c.role==sim::CourierRole::OriginalAncestor&&!c.cargo&&!c.reserved&&!world.original_courier_cargo(c.id),
              "Ancestor actor acquired another role or material cargo");
    for (const auto& b:world.buildings())
        check(!b.service_until_tick&&!b.health_protection_until_tick&&!b.health_risk&&!b.sick_until_tick,
              "Ancestor visit granted legacy service or health authority");
}
void ticks(sim::World& world,int count) {
    for (int i=0;i<count;++i) {world.tick();authority(world);}
}
template<class Predicate> void until(sim::World& world,Predicate done,int limit=20000) {
    for (int i=0;i<limit&&!done();++i) {world.tick();authority(world);}
    check(done(),"Paid Ancestor event did not occur within finite observation");
}
struct Fixture {
    f::Root root;
    std::shared_ptr<const sim::MapPermissions> policy=f::permissions();
    std::shared_ptr<const orig::OriginalRulesModel> model;
    explicit Fixture(int capacity=7) {
        oe_production_fixture::write_model(root.data,capacity,0,0,0,0,0,1);
        model=f::save::prepare_original_production_rules_model(root.data);
    }
    sim::World world() const {return sim::World::create_original_production_development(policy,model);}
    sim::World restore(const sim::World& world) const {
        return sim::World::restore_original_production_development(world.snapshot(),policy,model);
    }
};
void roads(sim::World& world) {
    for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,4}});
}
void homes(sim::World& world,int count=6) {
    for (int i=0;i<count;++i) (void)buy(world,sim::CommandType::PlaceHousehold,{i*3,2});
}
void paid_arrival_expiry_revisit_and_replay() {
    Fixture fixture;auto world=fixture.world();roads(world);homes(world);
    const auto home=*world.building_owner_at({0,2});
    const auto shrine=buy(world,sim::CommandType::PlaceOriginalAncestorShrine,{21,5});
    const auto id=actor(world,shrine);
    check(world.original_production_state()->development_revision==21,"Ancestor fixture requires active revision21");
    check(world.total_population()==0&&
          world.construction_spent_total()==568&&world.treasury()==11432,
          "Ancestor purchase did not pay24Roads/sixHouses/model40 without population grants");
    check(world.workforce_required(shrine)==4&&world.couriers().size()==1&&
          sim::building_footprint(world.profile(),world.rule_version(),sim::Object::OriginalAncestorShrine214)==sim::BuildingFootprint{2,2},
          "Ancestor body, actual workforce or sole actor binding changed");
    check(world.original_production_state()->services.providers.size()==1&&
          world.original_production_state()->services.trips.empty()&&world.original_production_state()->services.witnesses.empty(),
          "Ancestor purchase granted a visit or a trip");
    check(!orig::needs_met(fixture.model->house_rules(),orig::ModelHouse::Shelter,house(world,home)),
          "Unvisited House already satisfied its actual Ancestor requirement");
    ticks(world,119);check(world.original_production_state()->services.trips.empty(),"Unstaffed shrine dispatched");
    until(world,[&]{return world.courier(id).phase==sim::CourierPhase::ToWarehouse&&world.courier(id).edge_progress>0;});
    check(world.original_production_state()->services.witnesses.empty(),"Ancestor dispatch or begun edge granted religion");
    const auto moving=world.snapshot().couriers;f::buy(world,sim::set_building_operation(shrine,false));
    check(world.snapshot().couriers==moving,"Source pause cancelled the actual Ancestor tour");
    until(world,[&]{return trip(world,id).visit_applied;});
    check(trip(world,id).target==home&&house(world,home).ancestor>0&&
          orig::needs_met(fixture.model->house_rules(),orig::ModelHouse::Shelter,house(world,home)),
          "Real Ancestor arrival failed to satisfy only its matching need");
    until(world,[&]{return world.courier(id).phase==sim::CourierPhase::IdleAtWorkshop;});
    const auto visits=world.original_production_state()->services.witnesses;
    const auto sequence=trip(world,id).transfer.sequence;auto replay=fixture.restore(world);
    std::array<bool,16> steps{};bool saw_one_to_zero=false;
    for (int i=0;i<20000;++i) {
        const auto dispatch=orig::next_scheduler_dispatch(world.ticks());
        const auto before=house(world,home).ancestor;world.tick();replay.tick();authority(world);
        check(world.snapshot()==replay.snapshot(),"Full Ancestor20k restore replay diverged");
        const auto after=house(world,home).ancestor;
        if (dispatch&&dispatch->service_countdown) {
            steps[dispatch->position.step]=true;
            saw_one_to_zero|=before==1&&after==0;
        }
        check(after==(dispatch&&dispatch->service_countdown&&before ? before-1:before),
              "Ancestor countdown debited twice or ran on another scheduler phase");
    }
    check(std::all_of(steps.begin(),steps.end(),[](bool value){return value;})&&saw_one_to_zero,
          "Ancestor test missed a scheduler step or actual1->0 expiry");
    check(world.original_production_state()->services.witnesses==visits&&!house(world,home).ancestor&&
          !orig::needs_met(fixture.model->house_rules(),orig::ModelHouse::Shelter,house(world,home)),
          "Return/Idle/expiry granted or retained Ancestor coverage");
    f::buy(world,sim::set_building_operation(shrine,true));
    until(world,[&]{return house(world,home).ancestor>0;});
    check(trip(world,id).transfer.sequence>sequence&&
          orig::needs_met(fixture.model->house_rules(),orig::ModelHouse::Shelter,house(world,home)),
          "Real repeated paid-source tour did not restore expired Ancestor access");
}
void staffing_operation_route_and_purchase_limits() {
    Fixture fixture;
    for (const int count:{0,6}) {
        auto world=fixture.world();roads(world);homes(world,count);
        (void)buy(world,sim::CommandType::PlaceOriginalAncestorShrine,{21,count ? 8:5});ticks(world,900);
        check(world.original_production_state()->services.trips.empty()&&world.original_production_state()->services.witnesses.empty(),
              "Unstaffed or disconnected shrine manufactured a visit");
    }
    // One paid House with capacity3 cannot staff the4-worker source, even
    // though actual immigration made its target occupied and connected.
    Fixture small(3);auto partial=small.world();roads(partial);homes(partial,1);
    const auto unstaffed=buy(partial,sim::CommandType::PlaceOriginalAncestorShrine,{21,5});ticks(partial,900);
    check(partial.total_population()==3&&partial.workers_assigned(unstaffed)<4&&
          partial.original_production_state()->services.trips.empty(),"Partial shrine staffing dispatched a real actor");
    auto world=fixture.world();roads(world);homes(world);
    const auto shrine=buy(world,sim::CommandType::PlaceOriginalAncestorShrine,{21,5});
    f::buy(world,sim::set_building_operation(shrine,false));ticks(world,400);
    check(world.workforce_supply()==42&&world.original_production_state()->services.trips.empty(),"Paused staffed shrine dispatched");
    f::buy(world,sim::set_building_operation(shrine,true));const auto id=actor(world,shrine);
    until(world,[&]{return world.courier(id).phase==sim::CourierPhase::ToWarehouse&&world.courier(id).edge_progress>0;});
    const sim::Command cut{sim::CommandType::RemoveRoad,{12,4}};
    until(world,[&]{return world.validate(cut).accepted;});f::buy(world,cut);ticks(world,250);
    check(!trip(world,id).arrival_tick&&world.original_production_state()->services.witnesses.empty(),
          "Disconnected Ancestor actor supplied a House through the broken road");
    auto replay=fixture.restore(world);f::buy(world,{sim::CommandType::PlaceRoad,{12,4}});
    f::buy(replay,{sim::CommandType::PlaceRoad,{12,4}});
    for (int i=0;i<20000&&!trip(world,id).visit_applied;++i) {
        world.tick();replay.tick();authority(world);
        check(world.snapshot()==replay.snapshot(),"Interrupted Ancestor repair replay diverged");
    }
    check(trip(world,id).visit_applied&&house(world,trip(world,id).target).ancestor>0,
          "Paid repair did not restore actual Ancestor arrival");
    const auto second=buy(world,sim::CommandType::PlaceOriginalAncestorShrine,{18,5});
    check(actor(world,second)!=id,"Two paid shrines reused a Courier identity");
    const auto before=world.snapshot();const sim::Command third{sim::CommandType::PlaceOriginalAncestorShrine,{15,5}};
    check(!world.validate(third).accepted&&!world.execute(third).accepted&&world.snapshot()==before,
          "Third shrine exceeded the two-source bound or failed nonatomically");
}
void source_fire_after_dispatch_and_burning_target() {
    Fixture fixture;auto world=fixture.world();roads(world);
    const auto shrine=buy(world,sim::CommandType::PlaceOriginalAncestorShrine,{21,5});
    f::buy(world,sim::set_building_operation(shrine,false));ticks(world,600);homes(world);
    while (world.ticks()<1990) world.tick();
    f::buy(world,sim::set_building_operation(shrine,true));const auto id=actor(world,shrine);
    until(world,[&]{return world.courier(id).phase==sim::CourierPhase::ToWarehouse&&world.courier(id).edge_progress>0;});
    until(world,[&]{return world.building_on_fire(shrine);},30);
    check(!trip(world,id).arrival_tick,"Source-fire scene arrived before its intended incident");
    until(world,[&]{return trip(world,id).arrival_tick.has_value();},500);
    check(trip(world,id).visit_applied&&world.building_on_fire(shrine),"Later source fire cancelled actual Ancestor arrival");
    until(world,[&]{return world.courier(id).phase==sim::CourierPhase::IdleAtWorkshop;},500);
    const auto sequence=trip(world,id).transfer.sequence;ticks(world,30);
    check(trip(world,id).transfer.sequence==sequence&&world.courier_dispatch_status(id).status==sim::CourierDispatchStatus::OnFire,
          "Burning shrine dispatched another tour");
    auto target=fixture.world();roads(target);homes(target);ticks(target,1000);
    const auto delayed=buy(target,sim::CommandType::PlaceOriginalAncestorShrine,{21,5});
    f::buy(target,sim::set_building_operation(delayed,false));while (target.ticks()<1990)target.tick();
    f::buy(target,sim::set_building_operation(delayed,true));const auto visitor=actor(target,delayed);
    until(target,[&]{return target.courier(visitor).phase==sim::CourierPhase::ToWarehouse&&target.courier(visitor).edge_progress>0;});
    until(target,[&]{return trip(target,visitor).arrival_tick.has_value();},500);
    check(target.building_on_fire(trip(target,visitor).target)&&!trip(target,visitor).visit_applied&&
          target.original_production_state()->services.witnesses.empty(),"Burning target gained Ancestor religion");
}
void full_paid_collection_preserves_road_and_refuses_new_shrine_atomically() {
    Fixture fixture;
    constexpr int width=64,height=48;
    const auto policy=std::make_shared<const sim::MapPermissions>(width,height,3,
        std::vector<sim::MapCellPermission>(width*height,
            {true,true,false,0,sim::BuildBlocker::None,sim::BuildBlocker::None}),
        std::vector<sim::FixedGatePassage>{});
    auto world=sim::World::create_original_production_development(policy,fixture.model);
    // A larger explicitly authored rectangle keeps every paid footprint
    // disjoint. No stocks, residents, actors, IDs or capacities are injected.
    for (int row=0;row<4;++row) for (int col=0;col<5;++col)
        (void)buy(world,sim::CommandType::PlaceHousehold,{2+4*col,2+4*row});
    for (int i=0;i<4;++i) {
        (void)buy(world,sim::CommandType::PlaceClaySource,{30+4*i,2});
        (void)buy(world,sim::CommandType::PlacePottery,{30+4*i,6});
        (void)buy(world,sim::CommandType::PlaceMarket,{30+4*i,14});
    }
    for (int i=0;i<2;++i) {
        (void)buy(world,sim::CommandType::PlaceWarehouse,{30+4*i,10});
        const sim::Cell hemp_cell{2+6*i,24};
        const auto hemp=buy(world,sim::CommandType::PlaceOriginalHempFarm,hemp_cell);
        sim::Command field{sim::CommandType::PlaceOriginalHempField,{hemp_cell.x+2,hemp_cell.y}};
        field.building_id=hemp;f::buy(world,field);
        const sim::Cell food_cell{16+8*i,24};
        const auto food=buy(world,sim::CommandType::PlaceOriginalFoodFarm,food_cell);
        field={i ? sim::CommandType::PlaceOriginalMilletField:sim::CommandType::PlaceOriginalWheatField,
            {food_cell.x+3,food_cell.y}};
        field.building_id=food;f::buy(world,field);
        (void)buy(world,sim::CommandType::PlaceHealthPost,{36+4*i,24});
        (void)buy(world,sim::CommandType::PlaceServicePost,{36+4*i,28});
    }
    check(world.buildings().size()==46&&world.couriers().size()==26&&world.total_population()==0&&
          world.treasury()<12000&&world.construction_spent_total()==static_cast<std::uint64_t>(12000-world.treasury()),
          "Paid full collection did not reach exactly46 bodies/26 actors with conserved funds");
    auto restored=sim::World::restore_original_production_development(world.snapshot(),policy,fixture.model);
    check(restored.snapshot()==world.snapshot(),"Legal full paid collection did not restore exactly");
    const auto funds=world.treasury();
    f::buy(world,{sim::CommandType::PlaceRoad,{0,0}});
    f::buy(world,{sim::CommandType::RemoveRoad,{0,0}});
    f::buy(world,{sim::CommandType::PlaceRoad,{0,0}});
    check(world.treasury()==funds-4&&world.buildings().size()==46&&world.couriers().size()==26&&
          world.object_at({0,0})==sim::Object::Road,"Full body/actor capacity blocked ordinary paid road placement or repair");
    const auto before=world.snapshot();const auto cache_count=world.original_influence_cache_build_count();
    const sim::Command shrine{sim::CommandType::PlaceOriginalAncestorShrine,{48,28}};
    const auto validation=world.validate(shrine);
    check(!validation.accepted&&!world.execute(shrine).accepted&&world.snapshot()==before&&
          world.original_influence_cache_build_count()==cache_count,
          "Shrine beyond global capacity changed IDs, funds, providers, actors or any other authority");
    check(world.population_valid()&&world.production_balance_valid()&&world.city_economy_valid()&&world.navigation_valid(),
          "Full-capacity road/failed-shrine regression left an invalid World");
}
void aligned_actual_arrivals() {
    Fixture fixture(37);auto base=fixture.world();roads(base);homes(base,1);ticks(base,900);
    const auto home=*base.building_owner_at({0,2});
    const auto arrival=[&](sim::World& world) {
        const auto shrine=buy(world,sim::CommandType::PlaceOriginalAncestorShrine,{21,5});const auto id=actor(world,shrine);
        until(world,[&]{return !world.original_production_state()->services.trips.empty()&&trip(world,id).arrival_tick.has_value();});
        return *trip(world,id).arrival_tick;
    };
    auto probe=fixture.restore(base);const auto first=arrival(probe);
    for (const unsigned residue:{36u,37u}) {
        auto world=fixture.restore(base);ticks(world,static_cast<int>((residue+51-first%51)%51));
        const auto at=arrival(world);
        check(at%51==residue&&house(world,home).ancestor==(residue==36 ? 39:40),
              "Actual Ancestor arrival did not assign40 and apply exactly its phase35 countdown");
    }
}
}
int main() {
    try {
        paid_arrival_expiry_revisit_and_replay();staffing_operation_route_and_purchase_limits();
        source_fire_after_dispatch_and_burning_target();
        full_paid_collection_preserves_road_and_refuses_new_shrine_atomically();aligned_actual_arrivals();
        std::cout<<"Original Ancestor World tests PASS\n";return 0;
    } catch (const std::exception& error) {std::cerr<<"Original Ancestor World tests FAIL: "<<error.what()<<'\n';return 1;}
}
