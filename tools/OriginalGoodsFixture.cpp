#include "../tests/OriginalInfluenceFixture.h"
#include "simulation/OriginalGoodsState.h"
#include "simulation/original/GoodsRules.h"

#include <array>
#include <iostream>
#include <nlohmann/json.hpp>

namespace {
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace save=openemperor::persistence;
namespace original=openemperor::original;
using json=nlohmann::json;
using f::check;

json balances(const sim::World& world) {
    check(world.original_goods_enabled() && world.original_goods_state(),"Rule9 goods authority missing");
    const auto all=world.original_goods_balances();json result=json::array();
    for (std::size_t i=0;i<all.size();++i) {
        const auto& b=all[i];const auto one=world.original_goods_balance(static_cast<original::Goods>(i));
        check(b==one && static_cast<std::size_t>(b.good)==i && !b.supported && b.conserved &&
            !b.totals.produced && !b.totals.imported && !b.totals.consumed && !b.totals.demolished &&
            !b.stock && !b.cargo && !b.source_reserved && !b.incoming_reserved,
            "Rule9 created goods, reservations, historical credits or supported capability");
        result.push_back({{"good",i},{"supported",b.supported},{"produced",b.totals.produced},
            {"imported",b.totals.imported},{"consumed",b.totals.consumed},{"demolished",b.totals.demolished},
            {"stock",b.stock},{"cargo",b.cargo},{"source_reserved",b.source_reserved},
            {"incoming_reserved",b.incoming_reserved},{"conserved",b.conserved}});
    }
    check(result.size()==6 && world.total_population()==0 && world.total_population_capacity()==0 &&
        world.couriers().empty(),"Unsupported goods fixture gained people or actors");
    for (const auto& house:world.original_economy_state()->houses) {
        check(house.house.type==original::HouseType::Vacant && house.house.occupants==0 &&
            !original::accepts_goods_delivery(house.house),"Vacant House gained delivery admission");
        for (std::size_t i=0;i<original::goods_count;++i)
            check(original::goods_stock(house.house,static_cast<original::Goods>(i))==0,
                "House single-authority stock is not zero");
    }
    return result;
}

json inspect(const sim::World& world) {
    json houses=json::array();
    for (const auto& b:world.buildings()) if (b.kind==sim::Object::Household) {
        const auto p=world.original_influence_provenance(b.id);int legacy=0;json sources=json::array();
        for (const auto& source:world.buildings()) if (source.placed) {
            const auto kind=source.kind==sim::Object::OriginalInfluenceSource ? sim::Object::Well:source.kind;
            legacy+=sim::desirability_contribution(kind,sim::footprint_distance(b.cell,{2,2},source.cell,
                sim::building_footprint(world.profile(),world.rule_version(),source.kind)));
        }
        check(sim::clamp_desirability(legacy)==0,"City13 pure comparison changed");
        for (const auto& source:p.sources) sources.push_back({{"building_id",static_cast<unsigned>(source.building_id)},
            {"type",static_cast<unsigned>(source.type)},{"origin",{source.origin.x,source.origin.y}},
            {"footprint",{source.footprint.width,source.footprint.height}},
            {"spread",{source.spread.value,source.spread.step,source.spread.step_size,source.spread.range}},
            {"before",source.before},{"contribution",source.contribution},{"after",source.after}});
        houses.push_back({{"building_id",static_cast<unsigned>(b.id)},{"origin",{b.cell.x,b.cell.y}},
            {"value",p.value},{"selected_cell",{p.selected_cell.x,p.selected_cell.y}},
            {"city13_algorithm",legacy},{"model_sha256",p.model_sha256},
            {"difficulty",static_cast<unsigned>(p.difficulty)},{"sources",sources}});
    }
    const auto snapshot=world.snapshot();
    return {{"tick",world.ticks()},{"funds",world.treasury()},{"construction_spent",world.construction_spent_total()},
        {"taxes",world.taxes_collected_total()},{"houses",houses},
        {"live_sources",snapshot.original_influence->sources.size()},
        {"removed_sources",snapshot.original_influence->removed_sources.size()},
        {"next_building_id",snapshot.next_building_id},{"rule_version",world.rule_version()},
        {"model_sha256",world.original_rules_model()->provenance().sha256},{"balances",balances(world)},
        {"evidence","Original model; authored geography; paid World commands; unsupported goods remain zero"}};
}

std::vector<int> values(const json& report) {
    std::vector<int> result;
    for (const auto& house:report.at("houses")) result.push_back(house.at("value").get<int>());
    return result;
}
void save_world(const f::fs::path& data,const f::fs::path& target,const sim::World& world) {
    const auto policy=world.map_permissions();
    save::write_save(target,save::make_document(data,f::map_relative,policy->building_mask(),world),data,policy->building_mask());
}
void advance(sim::World& world) {
    const auto before=world.snapshot();const auto cache=world.original_influence_cache_build_count();
    const auto model=world.original_rules_model();const auto goods=balances(world);
    for (int i=0;i<20000;++i) world.tick();
    auto after=world.snapshot();check(after.ticks==before.ticks+20000,"Goods fixture clock drift");
    after.ticks=before.ticks;
    check(after==before && balances(world)==goods && world.original_rules_model()==model &&
        world.original_influence_cache_build_count()==cache,"20k ticks mutated non-clock authority");
}
void continuation(const f::fs::path& data,const f::fs::path& output,const std::string& prefix,sim::World& world) {
    advance(world);
    save_world(data,output/(prefix+"midpoint.json"),world);
    f::write_text(output/(prefix+"midpoint-report.json"),inspect(world).dump(2)+"\n");
    const auto before=world.snapshot();const auto cache=world.original_influence_cache_build_count();
    f::buy(world,{sim::CommandType::PlaceRoad,{23,19}});
    check(world.treasury()==before.treasury-2 && world.construction_spent_total()==before.construction_spent_total+2 &&
        world.command_sequence()==before.command_sequence+1 && world.original_influence_cache_build_count()==cache,
        "Actual paid command continuation changed cost, sequence or influence cache");
    advance(world);save_world(data,output/(prefix+"result.json"),world);
    f::write_text(output/(prefix+"result-report.json"),inspect(world).dump(2)+"\n");
}
json checkpoint(const f::fs::path& data,const f::fs::path& output,const char* tag,const sim::World& world) {
    const auto path=output/(std::string(tag)+".json");save_world(data,path,world);
    const auto document=save::read_save(path);const auto policy=world.map_permissions();
    check(document.source_schema_version==25 && document.world.rule_version==9,"Fixture did not write schema25/rule9");
    auto restored=save::restore_original_goods_development(document,data,policy->building_mask(),policy);
    f::same_document(save::make_document(data,f::map_relative,policy->building_mask(),world),document);
    check(restored.snapshot()==world.snapshot() && inspect(restored)==inspect(world),"Complete goods checkpoint restore changed");
    const auto bytes=f::read_text(path);save_world(data,path,restored);
    check(f::read_text(path)==bytes,"Canonical schema25 rewrite changed bytes");
    f::write_text(output/(std::string(tag)+"-report.json"),inspect(world).dump(2)+"\n");
    continuation(data,output,std::string(tag)+"-",restored);
    return {{"tag",tag},{"save",path.string()},{"expected_midpoint",(output/(std::string(tag)+"-midpoint.json")).string()},
        {"expected_midpoint_report",(output/(std::string(tag)+"-midpoint-report.json")).string()},
        {"expected_result",(output/(std::string(tag)+"-result.json")).string()},
        {"expected_report",(output/(std::string(tag)+"-result-report.json")).string()}};
}
void denied_economy(sim::World& world) {
    for (const auto type:{sim::CommandType::PlaceClaySource,sim::CommandType::PlacePottery,
            sim::CommandType::PlaceWarehouse,sim::CommandType::PlaceMarket}) {
        const auto before=world.snapshot();const auto cache=world.original_influence_cache_build_count();
        const sim::Command command{type,{0,17}};
        check(!world.validate(command).accepted && !world.execute(command).accepted && world.snapshot()==before &&
            world.original_influence_cache_build_count()==cache,"Unsupported production command mutated World");
    }
}
void create(const f::fs::path& data,const f::fs::path& output) {
    const auto policy=f::permissions();
    auto world=sim::World::create_original_goods_development(policy,save::prepare_original_rules_model(data));
    const auto scene=f::build_scene(world);const auto both=inspect(world);denied_economy(world);
    check(both.at("houses").size()==20 && std::ranges::any_of(values(both),[](int value){return value!=0;}),
        "Paid original-model scene has no actual House influence");
    f::buy(world,sim::demolish_building(scene.a));const auto removed=inspect(world);
    check(world.treasury()==200 && world.construction_spent_total()==1800 && values(removed)!=values(both),
        "Source removal failed to preserve paid history or change House values");
    auto checkpoints=json::array();checkpoints.push_back(checkpoint(data,output,"removed",world));
    const auto replacement=f::source(world,{0,0});
    check(static_cast<unsigned>(replacement)==23 && world.treasury()==140 && world.construction_spent_total()==1860 &&
        values(inspect(world))!=values(removed),"Paid edge rebuy cost, ID or House values changed");
    checkpoints.push_back(checkpoint(data,output,"rebought",world));denied_economy(world);
    f::write_text(output/"fixture.json",json{{"status","PASS"},{"authored_geography",true},{"snapshot_injection",false},
        {"paid_house_count",20},{"paid_road_count",40},{"well_purchases",3},{"house_value_changed",true},
        {"positive_runtime_goods",false},{"initial_both",both},{"final",inspect(world)},{"checkpoints",checkpoints}}.dump(2)+"\n");
}
void resume(const f::fs::path& data,const f::fs::path& output,const f::fs::path& input) {
    const auto before=f::read_text(input);const auto policy=f::permissions();
    const auto document=save::read_save(input);
    auto world=save::restore_original_goods_development(document,data,policy->building_mask(),policy);
    check(world.snapshot()==document.world,"New OS process failed full Rule9 restore");
    continuation(data,output,"",world);check(f::read_text(input)==before,"Process modified its source save");
}
void legacy7(const f::fs::path& output) {
    const auto data=output/"authored-data";f::fs::create_directories(data/"Cities");
    f::write_text(data/"Cities/Authored.map","Independent schema23 map identity; explicit prepared policy fixture\n");
    oe_save_fixture::write_model(data);
    constexpr int width=24,height=16;
    std::vector<sim::MapCellPermission> cells(width*height,
        {true,true,false,0,sim::BuildBlocker::None,sim::BuildBlocker::None});
    for (int y=0;y<height;++y) {
        for (int x=0;x<8;++x) cells[static_cast<std::size_t>(y*width+x)].height=1;
        auto& water=cells[static_cast<std::size_t>(y*width+11)];
        water.building_allowed=water.road_allowed=false;water.bridge_water=true;
    }
    const auto policy=std::make_shared<const sim::MapPermissions>(width,height,3,std::move(cells),std::vector<sim::FixedGatePassage>{});
    auto world=sim::World::create_original_economy_development(policy,save::prepare_original_rules_model(data));
    for (int x=4;x<=15;++x) if (x!=11) f::buy(world,{sim::CommandType::PlaceRoad,{x,5}});
    f::buy(world,sim::place_bridge({10,5},{12,5}));f::buy(world,sim::place_ramp({7,5},{8,5}));
    f::buy(world,{sim::CommandType::PlaceHousehold,{3,9}});f::buy(world,{sim::CommandType::PlaceHousehold,{14,9}});
    const auto target=output/"rule7.json";
    save::write_save(target,save::make_document(data,"Cities/Authored.map",policy->building_mask(),world),data,policy->building_mask());
    check(save::read_save(target).source_schema_version==23,"Legacy fixture writer changed schema23");
}
}
int main(int argc,char** argv) {
    try {
        const std::string mode=argc>1 ? argv[1]:"";
        check((argc==3 && mode=="--legacy7") || (argc==4 && mode=="--create") || (argc==5 && mode=="--resume"),
            "Usage: goods-fixture --legacy7 OUT | --create DATA OUT | --resume DATA OUT SAVE");
        const auto output=f::fs::absolute(argv[mode=="--legacy7" ? 2:3]);f::fs::create_directories(output);
        if (mode=="--legacy7") legacy7(output);
        else if (mode=="--create") create(f::fs::canonical(argv[2]),output);
        else resume(f::fs::canonical(argv[2]),output,f::fs::canonical(argv[4]));
        std::cout<<"Original goods unsupported paid fixture PASS\n";return 0;
    } catch (const std::exception& error) { std::cerr<<"Original goods fixture FAIL: "<<error.what()<<'\n';return 1; }
}
