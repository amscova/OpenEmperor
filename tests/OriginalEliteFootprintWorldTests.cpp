#include "OriginalProductionFixture.h"
#include "core/PerformanceDiagnostics.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <string>

namespace {
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
namespace perf=openemperor::performance;
using f::check;
constexpr sim::Cell origin{4,5};
constexpr sim::Cell extension{6,7};
struct Fixture {
    f::Root root;
    std::shared_ptr<const orig::OriginalRulesModel> model;
    Fixture() {
        oe_production_fixture::write_model(root.data,7);
        model=f::save::prepare_original_production_rules_model(root.data);
    }
    sim::World world(std::shared_ptr<const sim::MapPermissions> policy=f::permissions()) const {
        return sim::World::create_original_production_development(std::move(policy),model);
    }
    sim::World restore(const sim::World& world) const {
        return sim::World::restore_original_production_development(world.snapshot(),world.map_permissions(),model);
    }
};
const sim::OriginalHouseState& home(const sim::World& world,sim::BuildingId id) {
    for (const auto& value:world.original_economy_state()->houses) if (value.building_id==id) return value;
    throw std::runtime_error("Elite footprint lost its paid House authority");
}
const sim::OriginalHousingHouse& history(const sim::World& world,sim::BuildingId id) {
    for (const auto& value:world.original_production_state()->housing.houses) if (value.building_id==id) return value;
    throw std::runtime_error("Elite footprint lost its paid transition history");
}
sim::BuildingId buy(sim::World& world,sim::CommandType type,sim::Cell cell) {
    f::buy(world,{type,cell});return *world.building_owner_at(cell);
}
void valid(const sim::World& world) {
    check(world.navigation_valid()&&world.population_valid()&&world.production_balance_valid()&&world.city_economy_valid(),
          "Elite transaction broke ordinary World invariants");
    for (unsigned g=0;g<6;++g) check(world.original_goods_balance(static_cast<orig::Goods>(g)).conserved,
                                  "Elite transaction lost actual typed goods");
}
void through(sim::World& world,std::uint64_t tick) {while(world.ticks()<tick){world.tick();valid(world);}}
void vacant(const sim::World& world,sim::BuildingId id) {
    const auto& state=home(world,id);
    check(world.building(id).kind==sim::Object::OriginalElitePlot&&state.house.type==orig::HouseType::EliteUnoccupied&&
          !state.house.occupants&&!state.access_enabled&&!state.original_status&&!history(world,id).first_admission_tick,
          "Blocked materialization admitted a body, resident or history");
    check(!history(world,id).last_evaluation&&
          std::none_of(history(world,id).first_transitions.begin(),history(world,id).first_transitions.end(),[](const auto& x){return bool(x);}),
          "Blocked materialization published a half transition");
    for(int y=0;y<2;++y)for(int x=0;x<2;++x)
        check(world.building_owner_at({world.building(id).cell.x+x,world.building(id).cell.y+y})==id,
              "Blocked materialization lost an old plot claim");
}
void materialized(const sim::World& world,sim::BuildingId id,std::uint64_t first) {
    const auto& body=world.building(id);const auto& state=home(world,id);
    check(body.kind==sim::Object::OriginalEliteHouse&&state.house.type>=orig::HouseType::EliteAbandoned&&
          state.house.type<=orig::HouseType::HeavenlyCompound&&history(world,id).first_admission_tick==first,
          "Materialized body lacks actual Elite admission/family");
    for(int y=0;y<4;++y)for(int x=0;x<4;++x)
        check(world.building_owner_at({body.cell.x+x,body.cell.y+y})==id&&
              world.object_at({body.cell.x+x,body.cell.y+y})==sim::Object::OriginalEliteHouse,
              "Atomic4x4 materialization omitted one of16 unique matching claims");
    check(!world.original_elite_materialization_status(id).pending,"Admitted Elite retained an extension preparation");
}
void refused_demolition(sim::World& world,sim::BuildingId id) {
    const auto before=world.snapshot();const auto command=sim::demolish_building(id);
    check(!world.validate(command).accepted&&!world.execute(command).accepted&&world.snapshot()==before,
          "Unsupported Elite demolition partially changed a preparation/live body");
}
std::shared_ptr<const sim::MapPermissions> policy(std::vector<sim::MapCellPermission> cells,
                                                std::vector<sim::FixedGatePassage> gates={}) {
    return std::make_shared<const sim::MapPermissions>(f::width,f::height,3,std::move(cells),std::move(gates));
}
std::size_t index(sim::Cell cell) {return static_cast<std::size_t>(cell.y*f::width+cell.x);}
sim::FixedGatePassage gate(std::vector<sim::MapCellPermission>& cells,int x) {
    sim::FixedGatePassage result;result.id={101};
    for(int y=7;y<10;++y)for(int dx=0;dx<5;++dx) {
        const sim::Cell cell{x+dx,y};result.protected_footprint.push_back(cell);
        cells[index(cell)]={false,false,true,0,sim::BuildBlocker::GateSolidPart,sim::BuildBlocker::OriginalStructure};
    }
    for(int y=7;y<10;++y) {const sim::Cell cell{x+2,y};result.corridor.push_back(cell);cells[index(cell)].road_allowed=true;}
    result.openings={sim::Cell{x+2,6},sim::Cell{x+2,10}};return result;
}
void nine_independent_conflicts() {
    constexpr std::array names{"other-paid-owner","paid-road","protected-original","gate-solid","gate-corridor",
                               "original-road","bridge-deck","disconnected-bridge-bank","disconnected-ramp-endpoint"};
    constexpr std::array blockers{sim::BuildBlocker::SandboxOccupied,sim::BuildBlocker::SandboxOccupied,
        sim::BuildBlocker::OriginalStructure,sim::BuildBlocker::GateSolidPart,sim::BuildBlocker::ProtectedGatePassage,
        sim::BuildBlocker::ProtectedOriginalRoad,sim::BuildBlocker::ProtectedBridge,sim::BuildBlocker::ProtectedBridge,
        sim::BuildBlocker::SandboxOccupied};
    for(unsigned variant=0;variant<names.size();++variant) {
        Fixture fixture;auto cells=f::permissions()->cells();std::vector<sim::FixedGatePassage> gates;
        if(variant==2)cells[index(extension)]={false,false,true,0,sim::BuildBlocker::OriginalStructure,sim::BuildBlocker::OriginalStructure};
        if(variant==3||variant==4)gates.push_back(gate(cells,variant==3?6:2));
        if(variant==5)cells[index(extension)]={true,false,false,0,sim::BuildBlocker::None,sim::BuildBlocker::ProtectedOriginalRoad,true,sim::OriginalRoadClassification::GroundRoad};
        if(variant==6||variant==7) {
            const sim::Cell water=variant==6?sim::Cell{4,7}:sim::Cell{7,7};
            cells[index(water)]={false,false,false,0,sim::BuildBlocker::UnsupportedBridgeWater,sim::BuildBlocker::UnsupportedTerrain,false,sim::OriginalRoadClassification::WaterOrBridge,true};
        }
        if(variant==8)cells[index({7,7})].height=1;
        auto world=fixture.world(policy(std::move(cells),std::move(gates)));
        f::buy(world,{sim::CommandType::PlaceRoad,{4,4}});
        const auto id=buy(world,sim::CommandType::PlaceOriginalElitePlot,origin);
        if(variant==0)(void)buy(world,sim::CommandType::PlaceOriginalInfluenceSource,extension);
        if(variant==1)f::buy(world,{sim::CommandType::PlaceRoad,extension});
        if(variant==6||variant==7) {
            const auto a=variant==6?sim::Cell{3,7}:sim::Cell{6,7};
            const auto b=variant==6?sim::Cell{5,7}:sim::Cell{8,7};
            f::buy(world,{sim::CommandType::PlaceRoad,a});f::buy(world,{sim::CommandType::PlaceRoad,b});
            f::buy(world,sim::place_bridge(a,b));
            if(variant==7)f::buy(world,{sim::CommandType::RemoveRoad,a});
        }
        if(variant==8) {
            f::buy(world,{sim::CommandType::PlaceRoad,extension});f::buy(world,{sim::CommandType::PlaceRoad,{7,7}});
            f::buy(world,sim::place_ramp(extension,{7,7}));
            f::buy(world,{sim::CommandType::RemoveRoad,extension});f::buy(world,{sim::CommandType::RemoveRoad,{7,7}});
        }
        const auto initial=world.snapshot();const auto status=world.original_elite_materialization_status(id);
        check(status.pending&&!status.ready&&status.diagnostic.blocker==blockers[variant],
              std::string("Independent extension conflict not diagnosed: ")+names[variant]);
        check(world.snapshot()==initial,"Geometry diagnosis mutated paid authority");
        refused_demolition(world,id);through(world,119);vacant(world,id);
        auto replay=fixture.restore(world);
        for(int i=0;i<202;++i){world.tick();replay.tick();vacant(world,id);valid(world);check(world.snapshot()==replay.snapshot(),"Blocked119 restart diverged over repeated admission attempts");}
        check(home(world,id)==initial.original_economy->houses[0]&&
              history(world,id)==initial.original_production->housing.houses[0]&&
              world.treasury()==initial.treasury&&world.construction_spent_total()==initial.construction_spent_total&&
              world.command_sequence()==initial.command_sequence&&world.road_revision()==initial.road_revision&&
              world.snapshot().next_building_id==initial.next_building_id&&world.snapshot().couriers==initial.couriers,
              "Repeated blocked extension changed residents/goods/timers/history/finance/IDs/paths");
        refused_demolition(world,id);
        const auto reset=fixture.world();check(reset.buildings().empty()&&reset.total_population()==0&&reset.treasury()==12000,
              "Fresh normal generation retained a pending extension or lost its funding contract");
    }
}
void paid_repair_future_entrance_and_replay() {
    Fixture fixture;auto world=fixture.world();
    // The road touches only the future bottom edge: current2x2 has no entrance.
    f::buy(world,{sim::CommandType::PlaceRoad,{4,9}});
    const auto id=buy(world,sim::CommandType::PlaceOriginalElitePlot,origin);
    f::buy(world,{sim::CommandType::PlaceRoad,extension});
    const auto purchase=world.snapshot();through(world,159);vacant(world,id);
    refused_demolition(world,id);
    f::buy(world,{sim::CommandType::RemoveRoad,extension});
    check(world.original_elite_materialization_status(id).ready,"Real road removal did not unblock extension");
    // Remove and genuinely pay to repair its only future entrance.
    f::buy(world,{sim::CommandType::RemoveRoad,{4,9}});through(world,160);vacant(world,id);
    check(world.original_elite_materialization_status(id).diagnostic.blocker==sim::BuildBlocker::InvalidEdge,
          "No future entrance admitted a clipped/inaccessible body");
    const auto before_repair=world.snapshot();f::buy(world,{sim::CommandType::PlaceRoad,{4,9}});
    check(world.treasury()==before_repair.treasury-2&&world.construction_spent_total()==before_repair.construction_spent_total+2,
          "Extension road repair was not a genuine paid command");
    const auto ready=world.snapshot();through(world,199);vacant(world,id);
    auto replay=fixture.restore(world);const auto refresh=world.route_refresh_count();world.tick();replay.tick();
    check(world.snapshot()==replay.snapshot(),"Pre-materialization restart changed the first atomic success");
    materialized(world,id,200);
    check(home(world,id).house.type==orig::HouseType::ModestSiheyuan&&home(world,id).house.occupants==2&&
          world.building(id).cell==origin&&world.building(id).placed_tick==purchase.buildings[0].placed_tick&&
          world.treasury()==ready.treasury&&world.construction_spent_total()==ready.construction_spent_total&&
          world.command_sequence()==ready.command_sequence&&world.snapshot().next_building_id==ready.next_building_id&&
          world.road_revision()==ready.road_revision+1&&world.route_refresh_count()==refresh+1,
          "Atomic success charged/moved/duplicated identity or invalidated topology more than once");
    refused_demolition(world,id);replay=fixture.restore(world);
    bool abandoned=false;
    for(int i=0;i<20000;++i) {
        world.tick();replay.tick();check(world.snapshot()==replay.snapshot(),"Materialized full20k restart changed entire authority");
        materialized(world,id,200);abandoned|=home(world,id).house.type==orig::HouseType::EliteAbandoned;
    }
    check(abandoned,"Real missing-goods13→12 devolution was never observed");valid(world);
}
void edges_heights_and_read_only() {
    for(const sim::Cell edge:{sim::Cell{22,5},sim::Cell{4,18},sim::Cell{22,18}}) {
        Fixture fixture;auto world=fixture.world();const auto id=buy(world,sim::CommandType::PlaceOriginalElitePlot,edge);
        check(world.original_elite_materialization_status(id).diagnostic.blocker==sim::BuildBlocker::OutsideMap,
              "2x2 edge plot expanded its4x4 claims outside the grid");through(world,200);vacant(world,id);
    }
    for(const int base:{-128,-1,0,126,127}) {
        Fixture fixture;auto cells=f::permissions()->cells();
        for(auto& cell:cells)cell.height=static_cast<std::int8_t>(base);
        cells[index(extension)].height=static_cast<std::int8_t>(base==127?126:base+1);
        auto world=fixture.world(policy(std::move(cells)));f::buy(world,{sim::CommandType::PlaceRoad,{4,4}});
        const auto id=buy(world,sim::CommandType::PlaceOriginalElitePlot,origin);
        check(world.original_elite_materialization_status(id).diagnostic.blocker==sim::BuildBlocker::UnsupportedHeightTransition,
              "Signed-height foundation mixed levels or overflowed");through(world,200);vacant(world,id);
    }
    Fixture fixture;auto world=fixture.world();f::buy(world,{sim::CommandType::PlaceRoad,{4,4}});
    const auto id=buy(world,sim::CommandType::PlaceOriginalElitePlot,origin);const auto before=world.snapshot();
    perf::set_enabled(true);perf::reset();
    for(int i=0;i<100;++i)check(world.original_elite_materialization_status(id).ready,"Clear prospective4x4 not ready");
    for(const auto counter:{perf::Counter::WorldCopies,perf::Counter::WorldRestores,perf::Counter::WorldExecutes,
         perf::Counter::BfsCalls,perf::Counter::RouteRefreshes,perf::Counter::SimulationTicks,perf::Counter::FileReads,
         perf::Counter::FileWrites,perf::Counter::AssetDecodes,perf::Counter::TextureUploads})
        check(perf::counter(counter)==0,"Read-only extension diagnosis performed gameplay/navigation/IO/preparation");
    perf::set_enabled(false);check(world.snapshot()==before,"Read-only extension diagnosis changed authority");
    for(unsigned version=1;version<=6;++version) {
        auto legacy=version<4?sim::World(f::width,f::height,std::vector<std::uint8_t>(f::width*f::height,1),sim::RulesProfile::CityV16,version):
            sim::World(sim::authored_simple_permissions(f::width,f::height,std::vector<std::uint8_t>(f::width*f::height,1),version==4?2:3),sim::RulesProfile::CityV16,version);
        f::buy(legacy,{sim::CommandType::PlaceHousehold,origin});const auto common=*legacy.building_owner_at(origin);
        check(sim::building_footprint(legacy.profile(),legacy.rule_version(),sim::Object::Household)==sim::BuildingFootprint{2,2},
              "Published City-v16 Common footprint changed");
        for(int y=0;y<2;++y)for(int x=0;x<2;++x)check(legacy.building_owner_at({origin.x+x,origin.y+y})==common,"Legacy Common lost exact four claims");
        check(!legacy.building_owner_at(extension),"Legacy Common gained an Elite extension claim");
    }
}
void simultaneous_and_competing_materializations() {
    Fixture fixture;
    {
        auto world=fixture.world();
        f::buy(world,{sim::CommandType::PlaceRoad,{4,4}});
        f::buy(world,{sim::CommandType::PlaceRoad,{10,4}});
        const auto a=buy(world,sim::CommandType::PlaceOriginalElitePlot,{4,5});
        const auto b=buy(world,sim::CommandType::PlaceOriginalElitePlot,{10,5});
        through(world,119);
        check(world.building(a).placed_tick==0&&world.building(b).placed_tick==0&&
              world.original_elite_materialization_status(a).ready&&world.original_elite_materialization_status(b).ready,
              "Two disjoint paid plots were not simultaneously ready at119");
        const auto before=world.snapshot();const auto refresh=world.route_refresh_count();
        world.tick();materialized(world,a,120);materialized(world,b,120);
        check(home(world,a).house.occupants==2&&home(world,b).house.occupants==2&&world.total_population()==4&&
              world.road_revision()==before.road_revision+2&&world.route_refresh_count()==refresh+1&&
              world.command_sequence()==before.command_sequence&&world.treasury()==before.treasury&&
              world.construction_spent_total()==before.construction_spent_total&&
              world.snapshot().next_building_id==before.next_building_id&&world.couriers().empty(),
              "Simultaneous disjoint growth duplicated identity/cost/residents or failed one coalesced refresh");
        std::size_t claims=0;
        for(int y=0;y<world.height();++y)for(int x=0;x<world.width();++x)
            if(const auto owner=world.building_owner_at({x,y});owner==a||owner==b)++claims;
        check(claims==32&&fixture.restore(world).snapshot()==world.snapshot(),
              "Simultaneous growth did not retain32 exact independent claims on restart");valid(world);
    }
    {
        auto world=fixture.world();
        f::buy(world,{sim::CommandType::PlaceRoad,{4,4}});
        f::buy(world,{sim::CommandType::PlaceRoad,{7,1}});
        const auto a=buy(world,sim::CommandType::PlaceOriginalElitePlot,{4,5});
        const auto b=buy(world,sim::CommandType::PlaceOriginalElitePlot,{7,2});
        through(world,119);
        check(a<b&&world.original_elite_materialization_status(a).ready&&world.original_elite_materialization_status(b).ready,
              "Corner-competing paid plots were not independently ready before the shared tick");
        const auto before=world.snapshot();const auto old_b=home(world,b);const auto old_history=history(world,b);
        const auto refresh=world.route_refresh_count();world.tick();materialized(world,a,120);vacant(world,b);
        const auto status=world.original_elite_materialization_status(b);
        check(status.pending&&!status.ready&&status.diagnostic.blocker==sim::BuildBlocker::SandboxOccupied&&
              status.diagnostic.cell==sim::Cell{7,5}&&world.building_owner_at({7,5})==a&&
              home(world,b)==old_b&&history(world,b)==old_history&&
              world.road_revision()==before.road_revision+1&&world.route_refresh_count()==refresh+1&&
              world.command_sequence()==before.command_sequence&&world.treasury()==before.treasury&&
              world.construction_spent_total()==before.construction_spent_total&&
              world.snapshot().next_building_id==before.next_building_id&&world.total_population()==2,
              "Same-tick competition failed its ordered recheck or partially admitted the later plot");
        std::size_t loser_claims=0;
        for(int y=0;y<world.height();++y)for(int x=0;x<world.width();++x)
            if(world.building_owner_at({x,y})==b) {
                ++loser_claims;
                check(x>=7&&x<9&&y>=2&&y<4&&world.object_at({x,y})==sim::Object::OriginalElitePlot,
                      "Competing loser acquired a partial future claim");
            }
        check(loser_claims==4&&fixture.restore(world).snapshot()==world.snapshot(),
              "Competing loser lost one of its original four claims or cannot restart");valid(world);
    }
}

void materialized_restore_policy_and_counter_refusals() {
    Fixture fixture;auto world=fixture.world();
    f::buy(world,{sim::CommandType::PlaceRoad,{4,4}});
    const auto id=buy(world,sim::CommandType::PlaceOriginalElitePlot,origin);
    through(world,120);materialized(world,id,120);
    const auto good=world.snapshot();
    check(fixture.restore(world).snapshot()==good,"Valid mature4x4 geometry cannot restore");
    const auto refuses=[&](const sim::WorldSnapshot& snapshot,std::shared_ptr<const sim::MapPermissions> changed) {
        bool rejected=false;
        try {(void)sim::World::restore_original_production_development(snapshot,std::move(changed),fixture.model);}
        catch(const std::invalid_argument&) {rejected=true;}
        check(rejected,"Mature Elite restore accepted invalid immutable geometry or autonomous topology");
        check(world.snapshot()==good,"Failed candidate restore mutated the genuine mature World");
    };
    // Direct World restore intentionally has no file-policy fingerprint gate:
    // each refusal must come from real16-cell reconstruction/foundation checks.
    for(unsigned variant=0;variant<6;++variant) {
        auto cells=f::permissions()->cells();std::vector<sim::FixedGatePassage> gates;
        if(variant==0)cells[index(extension)].height=1;
        if(variant==1)cells[index(extension)]={false,false,true,0,sim::BuildBlocker::OriginalStructure,sim::BuildBlocker::OriginalStructure};
        if(variant==2||variant==3)gates.push_back(gate(cells,variant==2?6:2));
        if(variant==4)cells[index(extension)]={false,false,false,0,sim::BuildBlocker::UnsupportedTerrain,sim::BuildBlocker::UnsupportedTerrain};
        if(variant==5)cells[index(extension)]={true,false,false,0,sim::BuildBlocker::None,sim::BuildBlocker::ProtectedOriginalRoad,true,sim::OriginalRoadClassification::GroundRoad};
        refuses(good,policy(std::move(cells),std::move(gates)));
    }
    for(const int delta:{-1,1}) {
        auto bad=good;
        bad.road_revision=static_cast<std::uint64_t>(static_cast<std::int64_t>(good.road_revision)+delta);
        refuses(bad,world.map_permissions());
    }
    auto legacy_policy=sim::authored_simple_permissions(f::width,f::height,
        std::vector<std::uint8_t>(f::width*f::height,1),3);
    sim::World legacy(legacy_policy,sim::RulesProfile::CityV16,6);
    f::buy(legacy,{sim::CommandType::PlaceHousehold,origin});
    const auto historical=legacy.snapshot();auto bad=historical;
    ++bad.command_sequence;++bad.road_revision;
    bool rejected=false;
    try {(void)sim::World::restore(bad,legacy_policy);}
    catch(const std::invalid_argument&) {rejected=true;}
    check(rejected&&sim::World::restore(historical,legacy_policy).snapshot()==historical,
          "Published rule6 gained a nonexistent autonomous House growth allowance");
}
void active_actor_paths_and_resources_survive() {
    Fixture fixture;auto world=fixture.world();
    for(int x=0;x<18;++x)f::buy(world,{sim::CommandType::PlaceRoad,{x,14}});
    (void)buy(world,sim::CommandType::PlaceHousehold,{0,15});(void)buy(world,sim::CommandType::PlaceHousehold,{3,15});
    (void)buy(world,sim::CommandType::PlaceClaySource,{0,12});(void)buy(world,sim::CommandType::PlacePottery,{14,12});
    f::buy(world,{sim::CommandType::PlaceRoad,{4,4}});
    // Buy at real edge progress2. Buying at progress1 phase-locks every
    //120+40k opportunity's previous tick to an edge boundary (119mod8=7),
    // which cannot satisfy the deliberately strict begun-edge assertion.
    for(int i=0;i<2000&&!std::any_of(world.couriers().begin(),world.couriers().end(),[](const auto& c){return c.cargo>0&&c.edge_progress==2;});++i)world.tick();
    check(std::any_of(world.couriers().begin(),world.couriers().end(),[](const auto& c){return c.cargo>0&&c.edge_progress==2;}),
          "Actor preservation control never reached genuine paid loaded edge progress2");
    const auto id=buy(world,sim::CommandType::PlaceOriginalElitePlot,origin);
    f::buy(world,{sim::CommandType::PlaceRoad,extension});
    const auto placed=world.building(id).placed_tick;
    const auto loaded_begun=[&] {
        return std::any_of(world.couriers().begin(),world.couriers().end(),[](const auto& c) {
            return c.cargo>0&&c.edge_progress>0&&c.path_vertex+2<c.path.size();
        });
    };
    for(int i=0;i<5000&&!(world.ticks()>=placed+119&&(world.ticks()-placed-119)%40==0&&loaded_begun());++i)world.tick();
    check(world.ticks()>=placed+119&&(world.ticks()-placed-119)%40==0&&loaded_begun(),
          "Actual loaded begun edge never aligned with the next legal growth opportunity");
    f::buy(world,{sim::CommandType::RemoveRoad,extension});
    const auto first=world.ticks()+1;
    auto blocked=fixture.restore(world);f::buy(blocked,{sim::CommandType::PlaceRoad,extension});
    const auto before=world.snapshot();world.tick();blocked.tick();
    materialized(world,id,first);vacant(blocked,id);
    const auto after=world.snapshot(),control=blocked.snapshot();
    check(after.couriers==control.couriers&&after.original_goods==control.original_goods&&
          after.original_production->industry==control.original_production->industry&&
          after.clay_extracted_total==control.clay_extracted_total,
          "Unrelated growth rewrote an actual path/progress/cargo/reservation/recipe owner");
    check(after.command_sequence==before.command_sequence&&after.treasury==before.treasury&&
          after.construction_spent_total==before.construction_spent_total,
          "Unrelated growth invented a command or charged funds");valid(world);valid(blocked);
}
}
int main() {
    try {
        nine_independent_conflicts();paid_repair_future_entrance_and_replay();edges_heights_and_read_only();
        simultaneous_and_competing_materializations();
        materialized_restore_policy_and_counter_refusals();
        active_actor_paths_and_resources_survive();
        std::cout<<"Elite footprint World PASS:9 independent paid/policy conflicts, signed bounds/foundation, future entrance, "
            <<"stagnation/paid repair, atomic16 claims, no13→12 shrink, demolition refusal, active actors and20k restart\n";
        return 0;
    } catch(const std::exception& error) {
        perf::set_enabled(false);std::cerr<<"Elite footprint World FAIL: "<<error.what()<<'\n';return 1;
    }
}
