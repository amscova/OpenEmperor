#include "simulation/World.h"
#include "simulation/original/OriginalScheduler.h"

#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>

namespace {
namespace sim=openemperor::simulation;
namespace original=openemperor::original;
void check(bool value,const char* message) { if (!value) throw std::runtime_error(message); }
template<class F> void refuses(F action) {
    try { action(); } catch (const std::exception&) { return; }
    throw std::runtime_error("Invalid scheduler state was admitted");
}
std::shared_ptr<const sim::MapPermissions> policy() {
    return std::make_shared<const sim::MapPermissions>(6,6,3,
        std::vector<sim::MapCellPermission>(36,
            {true,true,false,0,sim::BuildBlocker::None,sim::BuildBlocker::None}),
        std::vector<sim::FixedGatePassage>{});
}
void buy(sim::World& world,sim::CommandType type,sim::Cell cell) {
    const auto result=world.execute({type,cell});
    check(result.accepted && result.changed,"Authored scheduler fixture purchase failed");
}
void replay_20000() {
    const auto permissions=policy();
    auto uninterrupted=sim::World::create_original_economy_development(permissions);
    buy(uninterrupted,sim::CommandType::PlaceHousehold,{0,0});
    buy(uninterrupted,sim::CommandType::PlaceRoad,{2,0});
    auto restarted=sim::World::restore_original_economy_development(uninterrupted.snapshot(),permissions);
    const auto paid=uninterrupted.snapshot();
    const auto immutable=permissions->canonical_state();
    std::uint64_t countdowns=0,evolutions=0,consumptions=0;
    for (unsigned tick=0;tick<20000;++tick) {
        const auto saved=restarted.snapshot();
        restarted=sim::World::restore_original_economy_development(saved,permissions);
        check(restarted.snapshot()==saved,"Repeated restore changed complete scheduler authority");
        const auto next=original::next_scheduler_dispatch(uninterrupted.ticks());
        check(next && next==original::next_scheduler_dispatch(restarted.ticks()),
            "Repeated restore changed the next service/evolution/consumption dispatch");
        countdowns+=next->service_countdown;evolutions+=next->evolution;
        consumptions+=next->consumption;
        uninterrupted.tick();restarted.tick();
        check(uninterrupted.snapshot()==restarted.snapshot(),"20000-tick restored replay drifted");
    }
    auto normalized=uninterrupted.snapshot();normalized.ticks=paid.ticks;
    check(normalized==paid && permissions->canonical_state()==immutable &&
        countdowns==392 && evolutions==392 && consumptions==50,
        "Clock scheduling changed paid state, map policy or independent event totals");
    check(uninterrupted.population_valid() && uninterrupted.production_balance_valid() &&
        uninterrupted.navigation_valid() && uninterrupted.city_economy_valid(),
        "Time-only development World violated its ordinary invariants");
}
void placement_and_limits() {
    const auto permissions=policy();
    auto world=sim::World::create_original_economy_development(permissions);
    for (unsigned tick=0;tick<817;++tick) world.tick();
    buy(world,sim::CommandType::PlaceHousehold,{0,0});
    const auto placed=world.snapshot();
    check(placed.buildings.front().placed_tick==817 && placed.original_economy->houses.front().cycle_phase==0,
        "Late paid container lost its actual placement tick or acquired active house phase");
    check(sim::World::restore_original_economy_development(placed,permissions).snapshot()==placed,
        "Late container did not roundtrip at its actual clock time");
    auto invalid=placed;invalid.buildings.front().placed_tick=818;
    refuses([&] { (void)sim::World::restore_original_economy_development(invalid,permissions); });
    invalid=placed;invalid.original_economy->houses.front().cycle_phase=1;
    refuses([&] { (void)sim::World::restore_original_economy_development(invalid,permissions); });
    auto terminal=placed;terminal.ticks=original::scheduler_max_tick-1;
    world=sim::World::restore_original_economy_development(terminal,permissions);
    world.tick();terminal.ticks=original::scheduler_max_tick;
    check(world.snapshot()==terminal && !original::next_scheduler_dispatch(world.ticks()),
        "Last valid tick did not produce a stable terminal scheduler state");
    refuses([&] { world.tick(); });
    check(world.snapshot()==terminal,"Exhausted tick partially mutated the World");
    check(sim::World::restore_original_economy_development(terminal,permissions).snapshot()==terminal,
        "Terminal scheduler state cannot be restored");
    terminal.ticks=std::numeric_limits<std::uint64_t>::max();
    refuses([&] { (void)sim::World::restore_original_economy_development(terminal,permissions); });
}
void legacy_clock() {
    sim::World world(6,6,std::vector<std::uint8_t>(36,1),sim::RulesProfile::HouseholdV3);
    buy(world,sim::CommandType::PlaceHousehold,{1,1});
    sim::TickDriver clock;clock.pause_and_reset();
    for (unsigned tick=0;tick<399;++tick) clock.step_once(world);
    auto home=world.building(sim::BuildingId::Household);
    check(sim::Rules::ticks_per_second==20 && sim::Rules::household_demand_ticks==400 &&
        world.ticks()==399 && home.demand_progress==399 && home.missed_demand==0,
        "Legacy 400-tick demand advanced early");
    clock.step_once(world);home=world.building(sim::BuildingId::Household);
    check(world.ticks()==400 && home.demand_progress==0 && home.missed_demand==1 &&
        home.fulfilled_demand==0 && home.consumed_total==0,"Legacy demand cadence or goods changed");
    const auto before=world.snapshot();clock.update(100.0,world);
    check(world.snapshot()==before,"Paused legacy clock advanced");
    clock.toggle_pause();clock.update(1.0/20,world);
    check(world.ticks()==401,"Legacy running clock no longer delivers 20 ticks/s");
}
}
int main() {
    try {
        replay_20000();placement_and_limits();legacy_clock();
        std::cout<<"Scheduler World PASS: 20000 repeated restores/replay, late paid placement, terminal atomicity and legacy 20Hz/400t\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr<<"Scheduler World FAIL: "<<error.what()<<'\n';return 1;
    }
}
