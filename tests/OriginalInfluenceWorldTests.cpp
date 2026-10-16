#include "OriginalInfluenceFixture.h"
#include "core/PerformanceDiagnostics.h"
#include "simulation/original/Desirability.h"
#include <algorithm>
#include <array>
#include <iostream>
#include <limits>

namespace {
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
namespace perf=openemperor::performance;
using f::check;
template<class F> void refuses(F action) {
    try { action(); } catch (const std::exception&) { return; }
    throw std::runtime_error("Invalid influence factory or restore accepted");
}
orig::SpreadSpec spread(const sim::World& world) {
    const auto& model=*world.original_rules_model();
    return {model.building_value(orig::ModelBuilding::Well,orig::BuildingField::Desirability),
        model.building_value(orig::ModelBuilding::Well,orig::BuildingField::DesirabilityStep),
        model.building_value(orig::ModelBuilding::Well,orig::BuildingField::DesirabilityStepSize),
        model.building_value(orig::ModelBuilding::Well,orig::BuildingField::DesirabilityRange)};
}
std::vector<int> scores(const sim::World& world) {
    std::vector<int> result;
    for (const auto& b:world.buildings()) if (b.kind==sim::Object::Household)
        result.push_back(world.household_desirability(b.id));
    return result;
}
void compare_core_and_provenance(const sim::World& world) {
    orig::DesirabilityMap reference(f::width,f::height);
    const auto snapshot=world.snapshot();const auto spec=spread(world);
    for (const auto& record:snapshot.original_influence->sources) {
        const auto& b=world.building(record.building_id);
        reference.apply_source({{b.cell.x,b.cell.y,2},spec});
    }
    for (const auto& b:world.buildings()) if (b.kind==sim::Object::Household) {
        const auto value=reference.building_value({b.cell.x,b.cell.y,2});
        check(world.household_desirability(b.id)==value,"Actual paid source differs from unchanged pure core");
        const auto p=world.original_influence_provenance(b.id);
        check(p.household_id==b.id && p.value==value &&
            p.model_sha256==world.original_rules_model()->provenance().sha256 &&
            p.difficulty==world.original_rules_model()->difficulty(),"House provenance lost model/value binding");
        sim::Cell selected=b.cell;int maximum=std::numeric_limits<int>::min();
        for (int y=b.cell.y;y<b.cell.y+2;++y) for (int x=b.cell.x;x<b.cell.x+2;++x)
            if (reference.at(x,y)>maximum) { maximum=reference.at(x,y);selected={x,y}; }
        check(p.selected_cell==selected,"House maximum tie-break changed");
        orig::DesirabilityMap prefix(f::width,f::height);std::size_t i=0;
        check(p.sources.size()==snapshot.original_influence->sources.size(),"Provenance omitted a prepared source");
        for (const auto& record:snapshot.original_influence->sources) {
            const auto& source=world.building(record.building_id);const auto& c=p.sources[i++];
            const auto before=prefix.at(selected.x,selected.y);
            prefix.apply_source({{source.cell.x,source.cell.y,2},spec});
            check(c.building_id==record.building_id && c.type==record.type && c.origin==source.cell &&
                c.footprint==sim::BuildingFootprint{2,2} && c.spread.value==spec.value &&
                c.spread.step==spec.step && c.spread.step_size==spec.step_size && c.spread.range==spec.range &&
                c.before==before && c.after==prefix.at(selected.x,selected.y),
                "Provenance does not explain actual ordered saturation at selected cell");
        }
        // This is the unchanged City13 algorithm on the actual paid geometry,
        // with the explicitly approved authored Well72 -> legacy Well comparison.
        int legacy=0;
        for (const auto& source:world.buildings()) if (source.placed) {
            const auto kind=source.kind==sim::Object::OriginalInfluenceSource ? sim::Object::Well:source.kind;
            legacy+=sim::desirability_contribution(kind,sim::footprint_distance(b.cell,{2,2},
                source.cell,sim::building_footprint(world.profile(),world.rule_version(),source.kind)));
        }
        check(sim::clamp_desirability(legacy)==0,"City13 algorithm comparison changed its Well-neutral contract");
    }
}
void reject_atomic(sim::World& world,sim::Command command) {
    const auto before=world.snapshot();const auto cache=world.original_influence_cache_build_count();
    check(!world.validate(command).accepted,"Invalid influence command validated");
    check(!world.execute(command).accepted && world.snapshot()==before &&
        world.original_influence_cache_build_count()==cache,"Rejected influence command changed state/cache");
}
void paid_scene() {
    f::Root root;const auto policy=f::permissions();
    const auto model=f::save::prepare_original_rules_model(root.data);
    auto world=sim::World::create_original_influence_development(policy,model);
    const auto initial_cache=world.original_influence_cache_build_count();
    f::roads(world);f::houses(world);
    check(world.original_influence_cache_build_count()==initial_cache,"Road/House commands rebuilt influence cache");
    const auto empty=scores(world);
    check(empty.size()==20 && std::ranges::all_of(empty,[](int v){return v==0;}),"Empty map influenced houses");
    const auto a=f::source(world,{18,6}),b=f::source(world,{18,11});
    check(world.treasury()==200 && world.construction_spent_total()==1800 &&
        world.original_influence_cache_build_count()==initial_cache+2,"Two paid source events accounting/cache");
    const auto both=scores(world);compare_core_and_provenance(world);
    check(both!=empty,"Paid sources produced no real House value change");
    check(world.total_population()==0 && world.total_population_capacity()==0 && world.couriers().empty() &&
        world.workforce_required(a)==0 && !world.household_has_water(static_cast<sim::BuildingId>(20)) &&
        sim::maintenance_cost(world.profile(),sim::Object::OriginalInfluenceSource)==0,
        "Influence source granted water, population, workers, upkeep or actors");
    reject_atomic(world,{sim::CommandType::PlaceOriginalInfluenceSource,{2,2}});
    reject_atomic(world,{sim::CommandType::PlaceOriginalInfluenceSource,{23,19}});
    reject_atomic(world,{sim::CommandType::PlaceOriginalInfluenceSource,{-1,0}});
    reject_atomic(world,{sim::CommandType::PlaceHousehold,{0,17}}); // Already20 households.
    reject_atomic(world,{sim::CommandType::PlaceWell,{0,17}});
    reject_atomic(world,{sim::CommandType::PlaceHealthPost,{0,17}});
    reject_atomic(world,sim::demolish_building(static_cast<sim::BuildingId>(1)));
    auto wrong=sim::Command{sim::CommandType::PlaceOriginalInfluenceSource,{0,17}};
    wrong.source_type=static_cast<sim::OriginalInfluenceSourceType>(65535);reject_atomic(world,wrong);
    f::buy(world,sim::demolish_building(a));
    check(world.treasury()==200 && world.construction_spent_total()==1800 &&
        world.snapshot().demolition_history==sim::DemolitionHistory{} &&
        world.snapshot().original_influence->removed_sources.size()==1 && scores(world)!=both,
        "Source removal refunded money, erased history or retained influence");
    reject_atomic(world,sim::demolish_building(a));
    const auto replacement=f::source(world,{0,0});
    check(static_cast<unsigned>(replacement)==23 && replacement>b && world.treasury()==140 &&
        world.construction_spent_total()==1860 && world.original_influence_cache_build_count()==initial_cache+4,
        "Edge source rebuy cost/ID/cache changed");
    compare_core_and_provenance(world);
    const auto stable=world.snapshot();const auto builds=world.original_influence_cache_build_count();
    perf::set_enabled(true);perf::reset();
    for (int i=0;i<120;++i) {
        for (const auto& home:world.buildings()) if (home.kind==sim::Object::Household) {
            (void)world.household_desirability(home.id);(void)world.original_influence_provenance(home.id);
        }
        world.tick();
    }
    for (const auto c:{perf::Counter::WorldCopies,perf::Counter::WorldRestores,perf::Counter::WorldExecutes,
            perf::Counter::BfsCalls,perf::Counter::RouteRefreshes,perf::Counter::FileReads,
            perf::Counter::FileWrites,perf::Counter::AssetDecodes,perf::Counter::TextureUploads})
        check(perf::counter(c)==0,"Read queries/ticks prepared IO, assets, World copies or routes");
    perf::set_enabled(false);
    auto after=world.snapshot();check(after.ticks==stable.ticks+120,"Scheduler did not advance");
    after.ticks=stable.ticks;
    check(after==stable && world.original_influence_cache_build_count()==builds && world.original_rules_model()==model,
        "Scheduler changed non-time authority/cache/model");
    check(world.city_economy_valid() && world.population_valid() && world.navigation_valid(),"Paid scene invariants");
}
void limits_and_retired_finance() {
    f::Root root;const auto policy=f::permissions();const auto model=f::save::prepare_original_rules_model(root.data);
    auto limited=sim::World::create_original_influence_development(policy,model);
    for (const sim::Cell p:{sim::Cell{0,0},{3,0},{6,0},{9,0}}) (void)f::source(limited,p);
    reject_atomic(limited,{sim::CommandType::PlaceOriginalInfluenceSource,{12,0}});
    auto spent=sim::World::create_original_influence_development(policy,model);
    for (int i=0;i<33;++i) {
        const auto id=f::source(spent,{0,0});f::buy(spent,sim::demolish_building(id));
    }
    check(spent.treasury()==20 && spent.construction_spent_total()==1980 &&
        spent.snapshot().original_influence->removed_sources.size()==33 && spent.city_economy_valid(),
        "Retired source purchases escaped finite financing");
    reject_atomic(spent,{sim::CommandType::PlaceOriginalInfluenceSource,{0,0}});
    auto restored=sim::World::restore_original_influence_development(spent.snapshot(),policy,model);
    check(restored.snapshot()==spent.snapshot(),"Maximum finite retired ledger did not restore");
    auto old=sim::World::create_original_economy_development(policy,model);
    check(old.treasury()==1300,"Rule7 budget changed");
    const auto before=old.snapshot();check(!old.execute({sim::CommandType::PlaceOriginalInfluenceSource,{0,0}}).accepted &&
        old.snapshot()==before,"Rule7 gained influence source authority");
    refuses([&]{sim::World normal(policy,sim::RulesProfile::CityV16,8);});
    refuses([&]{(void)sim::World::create_original_influence_development(policy,nullptr);});
    std::vector<sim::MapCellPermission> cells(f::width*f::height,
        {true,true,false,0,sim::BuildBlocker::None,sim::BuildBlocker::None});
    auto& protected_cell=cells[static_cast<std::size_t>(6*f::width+18)];
    protected_cell.building_allowed=protected_cell.road_allowed=false;protected_cell.protected_original=true;
    protected_cell.building_blocker=sim::BuildBlocker::OriginalStructure;
    protected_cell.road_blocker=sim::BuildBlocker::OriginalStructure;
    auto protected_policy=std::make_shared<const sim::MapPermissions>(f::width,f::height,3,
        std::move(cells),std::vector<sim::FixedGatePassage>{});
    auto protected_world=sim::World::create_original_influence_development(protected_policy,model);
    // Origin itself is legal; the fourth real cell is protected.
    reject_atomic(protected_world,{sim::CommandType::PlaceOriginalInfluenceSource,{17,5}});
}
void terminal_command_sequence() {
    f::Root root;const auto policy=f::permissions();const auto model=f::save::prepare_original_rules_model(root.data);
    auto world=sim::World::create_original_influence_development(policy,model);
    const auto source=f::source(world,{0,0});
    const std::array commands={
        sim::Command{sim::CommandType::PlaceOriginalInfluenceSource,{6,0}},
        sim::Command{sim::CommandType::PlaceHousehold,{3,3}},
        sim::Command{sim::CommandType::PlaceRoad,{3,0}},
        sim::demolish_building(source)};
    for (const auto command:commands)
        check(world.validate(command).accepted,"Terminal regression setup would refuse before counter exhaustion");
    auto snapshot=world.snapshot();
    snapshot.command_sequence=std::numeric_limits<std::uint64_t>::max()-1;
    auto terminal=sim::World::restore_original_influence_development(snapshot,policy,model);
    check(terminal.snapshot()==snapshot && terminal.city_economy_valid(),
        "Terminal command sequence did not restore a valid funded source scene");
    for (const auto command:commands) reject_atomic(terminal,command);

    snapshot.command_sequence=std::numeric_limits<std::uint64_t>::max()-2;
    auto penultimate=sim::World::restore_original_influence_development(snapshot,policy,model);
    const std::array roads={sim::Cell{3,0},sim::Cell{4,0}};
    for (const auto cell:roads)
        check(penultimate.validate({sim::CommandType::PlaceRoad,cell}).accepted,
            "Road batch boundary requires individually legal paid roads");
    const auto before=penultimate.snapshot();const auto builds=penultimate.original_influence_cache_build_count();
    check(!penultimate.validate_road_batch(roads).accepted,
        "Road batch accepted more commands than the remaining savable sequence budget");
    const auto denied=penultimate.execute_road_batch(roads);
    check(!denied.accepted && !denied.changed && penultimate.snapshot()==before &&
        penultimate.original_influence_cache_build_count()==builds,
        "Exhausted road batch changed authority, financing or influence cache");
    const std::array single_road={roads.front()};
    check(penultimate.validate_road_batch(single_road).accepted &&
        penultimate.execute_road_batch(single_road).accepted &&
        penultimate.command_sequence()==std::numeric_limits<std::uint64_t>::max()-1 &&
        penultimate.original_influence_cache_build_count()==builds,
        "One remaining road command did not reach the savable terminal sequence exactly");
    const auto final=penultimate.snapshot();
    check(sim::World::restore_original_influence_development(final,policy,model).snapshot()==final,
        "Road batch produced an unrestorable terminal command sequence");
}
}
int main() {
    try { paid_scene();limits_and_retired_finance();terminal_command_sequence();
        std::cout<<"Rule8 influence World: paid20 Houses/40 Roads, source mutations, model/core provenance, isolation, finite ledger PASS\n";
        return 0;
    } catch (const std::exception& error) {
        perf::set_enabled(false);std::cerr<<"Rule8 influence World FAIL: "<<error.what()<<'\n';return 1;
    }
}
