#include "OriginalInfluenceFixture.h"
#include "persistence/RecoveryStore.h"
#include "simulation/original/OriginalScheduler.h"
#include <nlohmann/json.hpp>
#include <functional>
#include <iostream>
#include <limits>

namespace {
namespace fixture=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace save=openemperor::persistence;
namespace original=openemperor::original;
namespace fs=std::filesystem;
using json=nlohmann::json;
using fixture::check;
using fixture::read_text;
using fixture::write_text;
std::size_t rejected=0;
template<class F> void refuses(F action) {
    try { action(); } catch (const std::exception&) { ++rejected;return; }
    throw std::runtime_error("Schema24 admitted malformed or unauthorized authority");
}
struct Fixture {
    fixture::Root root;
    std::shared_ptr<const sim::MapPermissions> policy=fixture::permissions();
    std::shared_ptr<const original::OriginalRulesModel> model=save::prepare_original_rules_model(root.data);
    sim::World world=sim::World::create_original_influence_development(policy,model);
    fixture::Scene scene=fixture::build_scene(world);
    fs::path target=root.path/"protected-schema24.json";
    save::SaveDocument document(const sim::World& value) const {
        return save::make_document(root.data,fixture::map_relative,policy->building_mask(),value);
    }
    save::SaveDocument document() const { return document(world); }
    sim::World restore(const save::SaveDocument& d,
            std::shared_ptr<const original::OriginalRulesModel> prepared=nullptr) const {
        return save::restore_original_influence_development(d,root.data,policy->building_mask(),policy,std::move(prepared));
    }
    void write(const save::SaveDocument& d,save::WriteFault fault=save::WriteFault::None) const {
        save::write_save(target,d,root.data,policy->building_mask(),fault);
    }
    void roundtrip() {
        const auto d=document();write(d);const auto bytes=read_text(target);
        const auto j=json::parse(bytes);
        check(j.at("schema_version")==24 && j.at("rules").at("version")==8,
            "Rule8 was captured by the old Schema23 writer");
        check(j.at("world").at("original_economy").at("houses").size()==20 &&
            j.at("world").at("original_influence").at("sources").size()==
                world.original_influence_state()->sources.size(),"Missing house/source partition");
        auto parsed=save::read_save(target);fixture::same_document(d,parsed);
        check(parsed.source_schema_version==24 && !parsed.prepared_original_rules_model,
            "JSON acquired transient session dependencies");
        auto restored=restore(parsed);check(restored.snapshot()==world.snapshot(),"Complete Schema24 restore changed state");
        check(restored.original_rules_model()!=model &&
            restored.original_rules_model()->provenance().sha256==model->provenance().sha256,
            "F9 failed independent immutable model preparation");
        auto retained=restore(parsed,model);
        check(retained.original_rules_model()==model && d.prepared_original_rules_model==model,
            "Explicit session dependency was replaced during capture/restore");
        const auto before=restored.snapshot();restored.tick();retained.tick();
        auto expected=before;++expected.ticks;
        check(restored.snapshot()==expected && retained.snapshot()==expected,"Reload drifted on next clock-only tick");
        parsed.prepared_permissions=policy;parsed.prepared_original_rules_model=model;
        write(parsed);check(read_text(target)==bytes,"Canonical Schema24 rewrite changed bytes");
        refuses([&] { (void)save::restore_save(parsed,root.data,policy->building_mask(),policy); });
        refuses([&] { (void)save::restore_original_economy_development(parsed,root.data,policy->building_mask(),policy,model); });
    }
};
void paid_mutation_roundtrips(Fixture& f) {
    f.roundtrip();
    for (int i=0;i<51;++i) f.world.tick();
    fixture::buy(f.world,sim::demolish_building(f.scene.a));
    check(f.world.treasury()==200 && f.world.construction_spent_total()==1800 &&
        f.world.original_influence_state()->removed_sources.size()==1 &&
        f.world.original_influence_state()->removed_sources[0].building_id==f.scene.a &&
        f.world.original_influence_state()->removed_sources[0].removed_tick==51,
        "Paid removal lost purchase cost, retired identity or exact clock");
    f.roundtrip();
    f.world.tick();const auto replacement=fixture::source(f.world,{0,0});
    check(static_cast<unsigned>(replacement)==23 && f.world.treasury()==140 &&
        f.world.construction_spent_total()==1860 && f.world.snapshot().next_building_id==24 &&
        f.world.snapshot().demolition_history==sim::DemolitionHistory{},
        "Source replacement reused identity, refunded cost or entered old demolition history");
    f.roundtrip();
}
void strict_json(Fixture& f) {
    const auto pristine=read_text(f.target);const auto good=json::parse(pristine);
    const auto hostile=f.root.path/"hostile-schema24.json";
    const auto reject=[&](const json& value) {
        write_text(hostile,value.dump());
        refuses([&] { const auto parsed=save::read_save(hostile);(void)f.restore(parsed,f.model); });
    };
    // Unknown/missing nested fields must fail during parsing, before file/model
    // resolution can hide a permissive new-generation reader.
    for (const auto& path:std::vector<std::string>{"","/world","/world/original_influence",
            "/world/original_influence/sources/0","/world/original_influence/removed_sources/0",
            "/world/original_economy","/world/original_economy/model_source",
            "/world/original_economy/houses/0","/world/original_economy/houses/0/house",
            "/world/buildings/0","/profiles/map_permissions"}) {
        auto j=good;j.at(json::json_pointer(path))["unknown_authority"]=true;write_text(hostile,j.dump());
        refuses([&] { (void)save::read_save(hostile); });
        j=good;auto& object=j.at(json::json_pointer(path));object.erase(object.begin());write_text(hostile,j.dump());
        refuses([&] { (void)save::read_save(hostile); });
    }
    const std::string s="/world/original_influence/sources/0",r="/world/original_influence/removed_sources/0";
    for (const auto& [path,value]:std::vector<std::pair<std::string,json>>{
            {"/schema_version",23},{"/schema_version",22},{"/rules/version",7},{"/rules/version",6},
            {"/rules/id","city-v13"},{"/profiles/map_permissions/version",2},
            {"/world/treasury",2000},{"/world/construction_spent_total",1800},
            {"/world/taxes_collected_total",1},{"/world/maintenance_spent_total",1},
            {"/world/demolition_history/buildings",1},{"/world/next_building_id",23},
            {"/world/next_building_id",25},{"/world/road_revision",0},
            {s+"/building_id",0},{s+"/building_id",20},{s+"/building_id",999},
            {s+"/type",71},{s+"/type",73},{s+"/type",65536},
            {r+"/building_id",22},{r+"/building_id",20},{r+"/type",73},
            {r+"/placed_tick",52},{r+"/removed_tick",53},
            {"/world/buildings/20/kind",static_cast<int>(sim::Object::Well)},
            {"/world/buildings/20/population",1},
            {"/world/original_economy/houses/0/original_status",3},
            {"/world/original_economy/model_source/difficulty",5},
            {"/world/original_economy/model_source/format_version",2},
            {"/world/original_economy/model_source/byte_count",0},
            {"/world/ticks",std::numeric_limits<std::uint64_t>::max()}}) {
        auto j=good;j.at(json::json_pointer(path))=value;reject(j);
    }
    for (const auto& path:{s+"/building_id",s+"/type",r+"/placed_tick",r+"/removed_tick"})
        for (const json& value:{json(-1),json(72.0),json(true),json("72"),json(nullptr)}) {
            auto j=good;j.at(json::json_pointer(path))=value;reject(j);
        }
    for (const char* name:{"sources","removed_sources"}) {
        auto j=good;auto& records=j["world"]["original_influence"][name];records.push_back(records[0]);reject(j);
        j=good;j["world"]["original_influence"][name].erase(0);reject(j);
    }
    { auto j=good;auto& records=j["world"]["original_influence"]["sources"];std::swap(records[0],records[1]);reject(j); }
    { auto j=good;auto& records=j["world"]["original_influence"]["removed_sources"];
        while (records.size()<34) records.push_back(records[0]);reject(j); }
    for (const std::string& bytes:{std::string("{\"schema_version\":24,")+pristine.substr(1),
            [&] { auto text=pristine;const auto at=text.find("\"removed_sources\"");
                const auto object=text.find('{',at);text.insert(object+1,"\"type\":72,");return text; }()}) {
        write_text(hostile,bytes);refuses([&] { (void)save::read_save(hostile); });
    }
    { auto j=good;j["deep"]=json::array();auto* at=&j["deep"];
        for (int i=0;i<34;++i) { at->push_back(json::array());at=&(*at)[0]; }
        write_text(hostile,j.dump());refuses([&] { (void)save::read_save(hostile); }); }
    constexpr std::size_t limit=8U*1024U*1024U;
    write_text(hostile,std::string(limit-pristine.size(),' ')+pristine);
    fixture::same_document(f.document(),save::read_save(hostile));
    write_text(hostile,std::string(limit-pristine.size()+1,' ')+pristine);
    refuses([&] { (void)save::read_save(hostile); });
    check(read_text(f.target)==pristine,"Adversarial parser changed protected save");
}
void dependencies_and_atomicity(Fixture& f) {
    const auto d=f.document();const auto pristine=read_text(f.target);const auto state=f.world.snapshot();
    for (const auto fault:{save::WriteFault::BeforeWrite,save::WriteFault::AfterPartialWrite,save::WriteFault::BeforeRename}) {
        refuses([&] { f.write(d,fault); });check(read_text(f.target)==pristine,"Atomic write failure changed target");
    }
    for (int kind=0;kind<9;++kind) {
        auto bad=d;
        if (kind==0) bad.map_sha256=std::string(64,'0');
        if (kind==1) bad.buildable_sha256=std::string(64,'0');
        if (kind==2) bad.map_permissions_sha256=std::string(64,'0');
        if (kind==3) bad.world.original_economy->model_source->sha256=std::string(64,'0');
        if (kind==4) ++bad.world.original_economy->model_source->byte_count;
        if (kind==5) bad.source_schema_version=23;
        if (kind==6) bad.world.original_economy->model_source.reset();
        if (kind==7) bad.world.original_influence.reset();
        if (kind==8) bad.world.original_influence->removed_sources.clear();
        refuses([&] { f.write(bad); });check(read_text(f.target)==pristine,"Invalid authority overwrote target");
    }
    const auto hard=save::prepare_original_rules_model(f.root.data,original::Difficulty::Hard);
    refuses([&] { (void)f.restore(d,hard); });
    auto bad=d;bad.prepared_original_rules_model=hard;
    refuses([&] { f.write(bad); });
    auto changed_cells=f.policy->cells();changed_cells.back().height=1;
    const auto changed_policy=std::make_shared<const sim::MapPermissions>(fixture::width,fixture::height,3,
        std::move(changed_cells),std::vector<sim::FixedGatePassage>{});
    refuses([&] { (void)save::restore_original_influence_development(d,f.root.data,
        f.policy->building_mask(),changed_policy,f.model); });
    fixture::Root second;
    const auto identical=save::prepare_original_rules_model(second.data);
    auto copied=save::restore_original_influence_development(d,second.data,f.policy->building_mask(),f.policy,identical);
    check(copied.snapshot()==state && copied.original_rules_model()==identical && identical!=f.model,
        "Byte-identical source at another root could not bind independently");
    auto changed=fixture::authored_model_text();const std::string row="72,Well,{,0,12,";
    const auto at=changed.find(row);check(at!=std::string::npos,"Authored second session row missing");
    changed.replace(at,row.size(),"72,Well,{,0,13,");
    write_text(second.data/"Model/EmperorBuildingModels.txt",changed);
    const auto other=save::prepare_original_rules_model(second.data);
    auto isolated=sim::World::create_original_influence_development(f.policy,other);
    isolated.tick();
    check(isolated.original_rules_model()->building_value(original::ModelBuilding::Well,original::BuildingField::Desirability)==13 &&
        f.model->building_value(original::ModelBuilding::Well,original::BuildingField::Desirability)==12,
        "Two immutable sessions shared or overwrote prepared source values");
    refuses([&] { (void)f.restore(d,other); });
    const auto file=f.root.data/"Model/EmperorBuildingModels.txt";const auto text=read_text(file);
    write_text(file,"Independent changed provenance\n"+text);
    refuses([&] { (void)f.restore(d,f.model); });refuses([&] { (void)f.document(); });
    refuses([&] { f.write(d); });
    write_text(file,"invalid authored model");refuses([&] { (void)f.restore(d); });
    fs::remove(file);refuses([&] { (void)f.restore(d,f.model); });refuses([&] { f.write(d); });
    write_text(file,text);
    check(f.world.snapshot()==state && f.world.original_rules_model()==f.model && read_text(f.target)==pristine,
        "Dependency failures changed active World or protected save");
    f.write(d);check(read_text(f.target)==pristine,"Valid retry changed canonical bytes");
}
void history_and_clock_limits(Fixture& f) {
    auto history=sim::World::create_original_influence_development(f.policy,f.model);
    for (int i=0;i<33;++i) {
        const auto id=fixture::source(history,{0,0});fixture::buy(history,sim::demolish_building(id));
    }
    const auto full=history.snapshot();
    check(full.treasury==20 && full.construction_spent_total==1980 && full.next_building_id==34 &&
        full.original_influence->removed_sources.size()==33 && full.road_revision==66,
        "Full paid removal ledger changed lifetime accounting");
    check(!history.execute({sim::CommandType::PlaceOriginalInfluenceSource,{0,0}}).accepted &&
        history.snapshot()==full,"Exhausted budget granted a34th purchase or mutated history");
    auto document=f.document(history);f.write(document);
    check(f.restore(save::read_save(f.target)).snapshot()==full,"Maximum removal ledger failed file roundtrip");
    auto terminal=full;terminal.ticks=original::scheduler_max_tick;
    auto exhausted=sim::World::restore_original_influence_development(terminal,f.policy,f.model);
    f.write(f.document(exhausted));auto restored=f.restore(save::read_save(f.target));
    refuses([&] { restored.tick(); });
    check(restored.snapshot()==terminal,"Exhausted restored clock changed source or ledger state");
    terminal.ticks=std::numeric_limits<std::uint64_t>::max();
    refuses([&] { (void)sim::World::restore_original_influence_development(terminal,f.policy,f.model); });
    f.write(f.document());
}
void historical_boundaries(Fixture& f) {
    auto old=sim::World::create_original_economy_development(f.policy,f.model);
    fixture::buy(old,{sim::CommandType::PlaceHousehold,{2,2}});
    auto d=f.document(old);f.write(d);const auto bytes=read_text(f.target);
    check(json::parse(bytes).at("schema_version")==23 && old.treasury()==1220,
        "Historical Rule7 acquired Rule8 schema or budget");
    auto bad=d;bad.world.original_influence=f.world.snapshot().original_influence;
    refuses([&] { f.write(bad); });check(read_text(f.target)==bytes,"Rule8 payload changed protected Schema23");
    const auto hostile=f.root.path/"old-with-influence.json";
    for (const bool root_key:{false,true}) {
        auto j=json::parse(bytes);
        if (root_key) j["original_influence"]=json::object();else j["world"]["original_influence"]=json::object();
        write_text(hostile,j.dump());refuses([&] { (void)save::read_save(hostile); });
    }
    auto legacy=sim::World(f.policy,sim::RulesProfile::CityV16,6);
    const auto legacy_document=f.document(legacy);f.write(legacy_document);const auto legacy_bytes=read_text(f.target);
    for (const bool root_key:{false,true}) {
        auto j=json::parse(legacy_bytes);
        if (root_key) j["original_influence"]=json::object();else j["world"]["original_influence"]=json::object();
        write_text(hostile,j.dump());refuses([&] { (void)save::read_save(hostile); });
    }
    auto mixed=json::parse(bytes);mixed["world"]["buildings"][0]["kind"]=static_cast<int>(sim::Object::OriginalInfluenceSource);
    write_text(hostile,mixed.dump());refuses([&] { (void)save::read_save(hostile); });
    // Historical22 unknown-key tolerance and canonical output remain unchanged.
    auto extension=json::parse(legacy_bytes);extension["historical_extension"]=true;
    write_text(hostile,extension.dump());
    check(save::read_save(hostile).world==legacy.snapshot(),"Legacy reader lost historical tolerance");
    f.write(legacy_document);check(read_text(f.target)==legacy_bytes,"Legacy canonical writer changed");
    auto recovery_path=f.root.path/"recovery";fs::create_directory(recovery_path);
    save::RecoveryStore recovery(recovery_path,f.root.data);
    refuses([&] { (void)recovery.create_history(f.document(),f.policy->building_mask()); });
    f.write(f.document());
}
} // namespace
int main() {
    try {
        Fixture f;paid_mutation_roundtrips(f);strict_json(f);dependencies_and_atomicity(f);
        history_and_clock_limits(f);historical_boundaries(f);
        std::cout<<"Schema24 PASS: paid source lifecycle, strict persistence, historical boundaries and "
            <<rejected<<" rejected adversarial cases\n";return 0;
    } catch (const std::exception& error) { std::cerr<<"Schema24: "<<error.what()<<'\n';return 1; }
}
