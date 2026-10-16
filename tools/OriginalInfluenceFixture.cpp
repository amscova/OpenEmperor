#include "../tests/OriginalInfluenceFixture.h"
#include "simulation/original/OriginalScheduler.h"
#include <nlohmann/json.hpp>
#include <iostream>

namespace {
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace save=openemperor::persistence;
using json=nlohmann::json;
using f::check;
std::vector<int> values(const json& report) {
    std::vector<int> result;
    for (const auto& home:report.at("houses")) result.push_back(home.at("value").get<int>());
    return result;
}
json inspect(const sim::World& world) {
    json houses=json::array();
    for (const auto& b:world.buildings()) if (b.kind==sim::Object::Household) {
        const auto p=world.original_influence_provenance(b.id);int legacy=0;json sources=json::array();
        for (const auto& s:world.buildings()) if (s.placed) {
            const auto kind=s.kind==sim::Object::OriginalInfluenceSource ? sim::Object::Well:s.kind;
            legacy+=sim::desirability_contribution(kind,sim::footprint_distance(b.cell,{2,2},s.cell,
                sim::building_footprint(world.profile(),world.rule_version(),s.kind)));
        }
        check(sim::clamp_desirability(legacy)==0,"Unchanged City13 algorithm acquired Well influence");
        for (const auto& s:p.sources) sources.push_back({{"building_id",static_cast<unsigned>(s.building_id)},
            {"type",static_cast<unsigned>(s.type)},{"origin",{s.origin.x,s.origin.y}},
            {"footprint",{s.footprint.width,s.footprint.height}},
            {"spread",{s.spread.value,s.spread.step,s.spread.step_size,s.spread.range}},
            {"before",s.before},{"contribution",s.contribution},{"after",s.after}});
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
        {"model_sha256",world.original_rules_model()->provenance().sha256},
        {"evidence","Original model bytes; authored geography; paid production commands; City13 algorithm comparison only"}};
}
void save_world(const f::fs::path& data,const f::fs::path& target,const sim::World& world) {
    const auto policy=world.map_permissions();
    save::write_save(target,save::make_document(data,f::map_relative,policy->building_mask(),world),data,policy->building_mask());
}
void advance(sim::World& world) {
    const auto before=world.snapshot();const auto cache=world.original_influence_cache_build_count();
    const auto model=world.original_rules_model();
    for (int i=0;i<20000;++i) world.tick();
    auto normalized=world.snapshot();check(normalized.ticks==before.ticks+20000,"Fixture clock drift");
    normalized.ticks=before.ticks;
    check(normalized==before && world.original_influence_cache_build_count()==cache &&
        world.original_rules_model()==model,"20k scheduler mutated finance/sources/model/cache");
}
json checkpoint(const f::fs::path& data,const f::fs::path& output,const char* tag,const sim::World& world) {
    const auto path=output/(std::string(tag)+".json");save_world(data,path,world);
    const auto document=save::read_save(path);const auto policy=world.map_permissions();
    check(document.source_schema_version==24,"Fixture writer did not use strict schema24");
    auto restored=save::restore_original_influence_development(document,data,policy->building_mask(),policy);
    f::same_document(save::make_document(data,f::map_relative,policy->building_mask(),world),document);
    check(restored.snapshot()==world.snapshot() && inspect(restored)==inspect(world),"Checkpoint restore changed full state/provenance");
    const auto canonical=f::read_text(path);save_world(data,path,restored);
    check(f::read_text(path)==canonical,"Canonical schema24 rewrite drift");
    f::write_text(output/(std::string(tag)+"-report.json"),inspect(world).dump(2)+"\n");
    advance(restored);const auto expected=output/(std::string(tag)+"-next.json");
    save_world(data,expected,restored);
    f::write_text(output/(std::string(tag)+"-next-report.json"),inspect(restored).dump(2)+"\n");
    return {{"tag",tag},{"save",path.string()},{"expected_next",expected.string()},
        {"expected_report",(output/(std::string(tag)+"-next-report.json")).string()}};
}
void create(const f::fs::path& data,const f::fs::path& output) {
    const auto policy=f::permissions();
    auto world=sim::World::create_original_influence_development(policy,save::prepare_original_rules_model(data));
    const auto scene=f::build_scene(world);const auto both=inspect(world);
    check(both.at("houses").size()==20,"Paid fixture did not create20 real Houses");
    bool influenced=false;for (const auto& home:both.at("houses")) influenced=influenced || home.at("value")!=0;
    check(influenced,"Actual original Well model caused no House influence");
    f::buy(world,sim::demolish_building(scene.a));
    check(world.treasury()==200 && world.construction_spent_total()==1800 &&
        world.snapshot().original_influence->removed_sources.size()==1,"Remove ledger/refund changed");
    const auto removed=inspect(world);check(values(removed)!=values(both),"Source removal did not change actual House values");
    auto checkpoints=json::array();checkpoints.push_back(checkpoint(data,output,"removed",world));
    const auto replacement=f::source(world,{0,0});
    check(static_cast<unsigned>(replacement)==23 && world.treasury()==140 &&
        world.construction_spent_total()==1860,"Rebuy finance/ID contract changed");
    check(values(inspect(world))!=values(removed),"Edge source rebuy did not change actual House values");
    checkpoints.push_back(checkpoint(data,output,"rebought",world));
    f::write_text(output/"fixture.json",json{{"status","PASS"},{"authored_geography",true},
        {"snapshot_injection",false},{"paid_house_count",20},{"paid_road_count",40},
        {"well_purchases",3},{"house_value_changed",true},{"initial_both",both},
        {"final",inspect(world)},{"checkpoints",checkpoints}}.dump(2)+"\n");
}
void resume(const f::fs::path& data,const f::fs::path& output,const f::fs::path& input) {
    const auto before=f::read_text(input);const auto policy=f::permissions();
    const auto document=save::read_save(input);
    auto world=save::restore_original_influence_development(document,data,policy->building_mask(),policy);
    check(world.original_rules_model() && world.snapshot()==document.world,"Fresh-process complete restore changed");
    advance(world);save_world(data,output/"result.json",world);
    f::write_text(output/"result-report.json",inspect(world).dump(2)+"\n");
    check(f::read_text(input)==before,"Resume changed source save bytes");
}
}
int main(int argc,char** argv) {
    try {
        check((argc==4 && std::string(argv[1])=="--create") ||
            (argc==5 && std::string(argv[1])=="--resume"),
            "Usage: influence-fixture --create DATA OUT | --resume DATA OUT SAVE");
        const auto data=f::fs::canonical(argv[2]);const auto output=f::fs::absolute(argv[3]);
        f::fs::create_directories(output);
        if (argc==4) create(data,output);else resume(data,output,f::fs::canonical(argv[4]));
        std::cout<<"Original influence paid fixture PASS\n";return 0;
    } catch (const std::exception& error) { std::cerr<<"Original influence fixture FAIL: "<<error.what()<<'\n';return 1; }
}
