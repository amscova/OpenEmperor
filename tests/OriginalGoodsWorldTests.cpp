#include "OriginalInfluenceFixture.h"
#include "core/PerformanceDiagnostics.h"

#include <iostream>

namespace {
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
namespace perf=openemperor::performance;
using f::check;
template<class F> void refuses(F action) {
    bool denied=false;try {action();} catch (const std::exception&) {denied=true;}
    check(denied,"Invalid goods authority or runtime path accepted");
}
void zero(const sim::World& world) {
    const auto balances=world.original_goods_balances();
    for (std::size_t i=0;i<orig::goods_count;++i) {
        sim::OriginalGoodsBalance expected;expected.good=static_cast<orig::Goods>(i);
        check(balances[i]==expected && world.original_goods_balance(expected.good)==expected,
            "Rule9 granted a supported goods capability or nonzero quantity");
    }
}
void paid_world() {
    f::Root root;const auto policy=f::permissions();const auto model=f::save::prepare_original_rules_model(root.data);
    auto world=sim::World::create_original_goods_development(policy,model);
    auto old=sim::World::create_original_influence_development(policy,model);
    check(world.rule_version()==9 && world.original_goods_enabled() && world.original_goods_state() &&
        *world.original_goods_state()==sim::OriginalGoodsState{},"Rule9 goods authority missing at create");
    const auto scene=f::build_scene(world);(void)f::build_scene(old);zero(world);
    f::buy(world,sim::demolish_building(scene.a));f::buy(old,sim::demolish_building(scene.a));
    (void)f::source(world,{0,0});(void)f::source(old,{0,0});zero(world);
    auto inherited=world.snapshot();inherited.rule_version=8;inherited.original_goods.reset();
    check(inherited==old.snapshot(),"Rule9 changed inherited Rule8 paid source/ID/finance contract");
    const auto saved=world.snapshot();const auto count=world.original_influence_cache_build_count();
    for (const auto type:{sim::CommandType::PlaceClaySource,sim::CommandType::PlacePottery,
            sim::CommandType::PlaceWarehouse,sim::CommandType::PlaceMarket,sim::CommandType::PlaceFarm}) {
        check(!world.validate({type,{0,17}}).accepted && !world.execute({type,{0,17}}).accepted &&
            world.snapshot()==saved && world.original_influence_cache_build_count()==count,
            "Rule9 producer/cargo source command gained authority or changed state");
    }
    perf::set_enabled(true);perf::reset();
    for (int i=0;i<20000;++i) {zero(world);world.tick();}
    for (const auto counter:{perf::Counter::WorldCopies,perf::Counter::WorldRestores,perf::Counter::WorldExecutes,
            perf::Counter::BfsCalls,perf::Counter::RouteRefreshes,perf::Counter::FileReads,perf::Counter::FileWrites,
            perf::Counter::AssetDecodes,perf::Counter::TextureUploads})
        check(perf::counter(counter)==0,"Goods query/tick copied or prepared World/routes/IO/assets");
    perf::set_enabled(false);
    auto after=world.snapshot();check(after.ticks==saved.ticks+20000,"Rule9 scheduler did not advance");
    after.ticks=saved.ticks;check(after==saved && world.original_influence_cache_build_count()==count,
        "Rule9 time-only tick mutated goods, finance, House or cache");
    auto replay=sim::World::restore_original_goods_development(world.snapshot(),policy,model);
    check(replay.snapshot()==world.snapshot(),"Rule9 paid zero-only World did not restore exactly");
    for (std::size_t i=0;i<orig::goods_count;++i) {
        auto bad=saved;bad.original_goods->totals[i].produced=1;
        refuses([&]{(void)sim::World::restore_original_goods_development(bad,policy,model);});
    }
    auto balanced=saved;balanced.original_goods->totals[1].produced=1;
    balanced.original_economy->houses[0].house.ceramics=1;
    refuses([&]{(void)sim::World::restore_original_goods_development(balanced,policy,model);});
    auto bad=saved;bad.original_goods->stocks.push_back({scene.b,{}});
    refuses([&]{(void)sim::World::restore_original_goods_development(bad,policy,model);});
    bad=saved;bad.original_goods.reset();
    refuses([&]{(void)sim::World::restore_original_goods_development(bad,policy,model);});
    refuses([&]{(void)sim::World::restore_original_influence_development(saved,policy,model);});
    refuses([&]{(void)sim::World::restore(saved,policy);});
    refuses([&]{sim::World normal(policy,sim::RulesProfile::CityV16,9);});
    refuses([&]{(void)sim::World::create_original_goods_development(policy,nullptr);});
    refuses([&]{(void)world.original_goods_balance(static_cast<orig::Goods>(6));});
    f::Root second;const auto second_model=f::save::prepare_original_rules_model(second.data);
    const auto isolated=sim::World::create_original_goods_development(policy,second_model);zero(isolated);
    check(isolated.buildings().empty() && isolated.treasury()==2000 &&
        isolated.original_rules_model()==second_model && world.original_rules_model()==model,
        "Independent Rule9 sessions shared mutable authority or exchanged session models");
    for (const auto version:{7U,8U}) {
        auto legacy=version==7 ? sim::World::create_original_economy_development(policy,model):
            sim::World::create_original_influence_development(policy,model);
        auto hostile=legacy.snapshot();hostile.original_goods.emplace();
        refuses([&]{sim::World::validate_original_economy_snapshot(hostile);});
        check(!legacy.original_goods_state(),"Rule7/8 gained a neutral goods payload");
    }
}
void inherited_infrastructure() {
    f::Root root;const auto model=f::save::prepare_original_rules_model(root.data);
    std::vector<sim::MapCellPermission> cells(f::width*f::height,
        {true,true,false,0,sim::BuildBlocker::None,sim::BuildBlocker::None});
    for (int y=0;y<f::height;++y) for (int x=0;x<8;++x)
        cells[static_cast<std::size_t>(y*f::width+x)].height=1;
    auto& water=cells[static_cast<std::size_t>(5*f::width+11)];
    water.building_allowed=water.road_allowed=false;water.bridge_water=true;
    const auto policy=std::make_shared<const sim::MapPermissions>(f::width,f::height,3,
        std::move(cells),std::vector<sim::FixedGatePassage>{});
    const auto immutable=policy->canonical_state();
    auto world=sim::World::create_original_goods_development(policy,model);
    for (int x=4;x<=15;++x) if (x!=11) f::buy(world,{sim::CommandType::PlaceRoad,{x,5}});
    for (const auto cell:{sim::Cell{5,4},sim::Cell{14,4}}) f::buy(world,{sim::CommandType::PlaceRoad,cell});
    check(world.transport_edge_blocker({7,5},{8,5})==sim::BuildBlocker::UnsupportedHeightTransition,
        "Rule9 normal roads bypassed the signed height transition before ramp purchase");
    f::buy(world,sim::place_bridge({10,5},{12,5}));
    f::buy(world,sim::place_ramp({7,5},{8,5}));
    f::buy(world,{sim::CommandType::PlaceHousehold,{18,1}});(void)f::source(world,{18,8});
    check(world.treasury()==1734 && world.construction_spent_total()==266 &&
        world.bridges().size()==1 && world.bridges()[0].paid_cost==60 &&
        world.bridges()[0].water_cells.size()==1 && world.bridges()[0].deck_height==0 &&
        world.ramps().size()==1 && world.ramps()[0].paid_cost==40 &&
        world.ramps()[0].height_a==1 && world.ramps()[0].height_b==0,
        "Rule9 changed inherited Bridge60/Ramp40/House80/Source60 or signed geometry");
    check(world.transport_edge_allowed({10,5},{11,5}) && world.transport_edge_allowed({11,5},{12,5}) &&
        !world.transport_edge_allowed({11,5},{11,4}) && world.transport_edge_allowed({7,5},{8,5}) &&
        world.transport_edge_allowed({8,5},{7,5}) &&
        policy->transport_edge_blocker({7,5},{8,5})==sim::BuildBlocker::UnsupportedHeightTransition,
        "Rule9 inherited infrastructure did not retain exact owned longitudinal transitions");
    zero(world);const auto before=world.snapshot();const auto cache=world.original_influence_cache_build_count();
    check(!world.execute({sim::CommandType::PlaceOriginalInfluenceSource,{10,5}}).accepted &&
        world.snapshot()==before && world.original_influence_cache_build_count()==cache,
        "Rule9 source construction bypassed protected bridge/water or failed atomically");
    const auto ramp=world.ramps().front();const auto bridge=world.bridges().front();
    f::buy(world,{sim::CommandType::RemoveRoad,{7,5}});
    check(world.ramps().front()==ramp && !world.transport_edge_allowed({7,5},{8,5}),
        "Rule9 disconnected ramp lost identity or remained traversable");
    f::buy(world,{sim::CommandType::PlaceRoad,{7,5}});
    check(world.transport_edge_allowed({7,5},{8,5}) && world.treasury()==1732,
        "Rule9 ramp endpoint repair was free or did not restore its owned edge");
    f::buy(world,{sim::CommandType::RemoveRoad,{10,5}});
    check(world.bridges().front()==bridge && !world.transport_edge_allowed({10,5},{11,5}),
        "Rule9 disconnected bridge lost identity or retained live bank traversal");
    f::buy(world,{sim::CommandType::PlaceRoad,{10,5}});
    check(world.transport_edge_allowed({10,5},{11,5}) && world.treasury()==1730 &&
        world.construction_spent_total()==270 && world.original_influence_cache_build_count()==cache,
        "Rule9 bank repair changed inherited accounting or rebuilt the influence cache");
    zero(world);
    const auto document=f::save::make_document(root.data,f::map_relative,policy->building_mask(),world);
    const auto target=root.path/"rule9-infrastructure.json";
    f::save::write_save(target,document,root.data,policy->building_mask());
    const auto read=f::save::read_save(target);f::same_document(document,read);
    auto restored=f::save::restore_original_goods_development(read,root.data,policy->building_mask(),policy,model);
    check(read.source_schema_version==25 && restored.snapshot()==world.snapshot() &&
        restored.bridges().front()==bridge && restored.ramps().front()==ramp &&
        restored.original_rules_model()==model && policy->canonical_state()==immutable,
        "Rule9 combined Schema25 restore changed geometry, authority, permissions or model binding");
    zero(restored);
    const auto rewritten=root.path/"rule9-infrastructure-restored.json";
    f::save::write_save(rewritten,f::save::make_document(root.data,f::map_relative,policy->building_mask(),restored),
        root.data,policy->building_mask());
    check(f::read_text(rewritten)==f::read_text(target),"Rule9 combined Schema25 canonical bytes changed after restore");
}
}
int main() {
    try {paid_world();inherited_infrastructure();std::cout<<"Rule9 World: inherited paid scene and Bridge/Ramp repair, unsupported zero goods, strict origins and20k time-only replay PASS\n";return 0;}
    catch (const std::exception& error) {perf::set_enabled(false);std::cerr<<"Rule9 World FAIL: "<<error.what()<<'\n';return 1;}
}
