#include "OriginalFinanceFixture.h"
#include "app/AutosaveController.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <map>

namespace {
namespace oe=openemperor;
namespace sim=oe::simulation;
namespace save=oe::persistence;
namespace q=oe_finance_fixture;
namespace f=oe_influence_fixture;
namespace fs=std::filesystem;
using f::check;
using Access=save::RecoveryAccess;
using Json=nlohmann::json;
constexpr auto budget=save::recovery_storage_budget_bytes;
std::size_t refusals=0,roundtrips=0;
template<class Action> void refuses(Action action,const char* message) {
    try {action();} catch (const std::exception&) {++refusals;return;}
    throw std::runtime_error(message);
}
void through(sim::World& world,std::uint64_t tick) {
    while (world.ticks()<tick) world.tick();
    check(world.navigation_valid()&&world.population_valid()&&world.production_balance_valid()&&world.city_economy_valid(),
          "Ordinarily played recovery fixture broke World invariants");
}
std::map<std::string,std::string> files(const fs::path& root) {
    std::map<std::string,std::string> result;
    if (!fs::exists(root)) return result;
    for (const auto& entry:fs::recursive_directory_iterator(root))
        if (!fs::is_symlink(entry.symlink_status())&&entry.is_regular_file())
            result.emplace(entry.path().lexically_relative(root).generic_string(),f::read_text(entry.path()));
    return result;
}
void same(const save::SaveDocument& a,const save::SaveDocument& b) {f::same_document(a,b);++roundtrips;}
const save::RecoveryHistory& history(const save::RecoveryCatalog& catalog,const std::string& id) {
    const auto it=std::find_if(catalog.histories.begin(),catalog.histories.end(),[&](const auto& row){return row.history_id==id;});
    check(it!=catalog.histories.end(),"Expected recovery history missing from catalog");return *it;
}
const save::RecoveryEntry& latest(const save::RecoveryHistory& h) {
    check(h.error.empty()&&!h.entries.empty()&&h.entries.front().loadable(),"Expected a loadable current Original checkpoint");
    return h.entries.front();
}
const save::RecoveryEntry& sequence(const save::RecoveryHistory& h,std::uint64_t value) {
    const auto it=std::find_if(h.entries.begin(),h.entries.end(),[&](const auto& row){return row.sequence==value;});
    check(it!=h.entries.end(),"Expected recovery sequence missing from catalog");return *it;
}
void paid_population_and_binding() {
    q::Fixture fixture;auto world=fixture.world();const auto scene=q::build(world,true);through(world,120);
    check(scene.elite&&world.building(*scene.elite).kind==sim::Object::OriginalEliteHouse&&
          q::house(world,*scene.elite).occupants>0,"Recovery population test lacks a naturally materialized inhabited Elite");
    int common_people=0;for (const auto id:scene.commons) common_people+=q::house(world,id).occupants;
    check(world.total_population()==common_people+q::house(world,*scene.elite).occupants&&common_people>0,
          "Recovery population test did not establish both inhabited families");
    for (const auto& body:world.buildings()) if (sim::original_house_kind(body.kind))
        check(body.population==0,"Original recovery fixture unexpectedly used legacy population authority");
    const auto app=fixture.root.path/"population-app";const auto mask=fixture.policy->building_mask();
    save::RecoveryStore normal(app,fixture.root.data);
    const auto document=fixture.document(world);const auto untouched=files(normal.root());
    refuses([&]{normal.create_history(document,mask);},"Default recovery implicitly admitted Rule10");
    check(files(normal.root())==untouched,"Default refusal changed recovery files");
    save::RecoveryStore enabled(app,fixture.root.data,budget,Access::OriginalProduction);
    const auto id=enabled.create_history(document,mask);auto catalog=enabled.catalog();
    const auto entry=latest(history(catalog,id));
    check(entry.schema==26&&entry.rule_version==10&&entry.population==world.total_population()&&entry.treasury==world.treasury(),
          "Original recovery summary lost schema, population including Elite or signed funds");
    const auto saved=save::read_save(entry.path);same(document,saved);auto restored=fixture.restore(saved);
    through(world,333);through(restored,333);check(world.snapshot()==restored.snapshot(),"Original checkpoint continuation diverged");
    const auto default_catalog=normal.catalog();
    check(!history(default_catalog,id).entries.front().loadable(),"Default catalog silently exposed Original recovery");
}
void scheduling_rotation_branch_and_faults() {
    q::Fixture fixture;auto world=fixture.world();q::build(world,true);
    const auto mask=fixture.policy->building_mask();const auto app=fixture.root.path/"scheduled-app";
    oe::AutosaveController disabled(fixture.root.path/"disabled",fixture.root.data,false,budget,Access::OriginalProduction);
    check(disabled.begin(fixture.document(world),mask).kind==oe::AutosaveResult::Kind::None&&
          !fs::exists(fixture.root.path/"disabled"),"Disabled Original autosave wrote storage");
    oe::AutosaveController controller(app,fixture.root.data,true,budget,Access::OriginalProduction);
    check(controller.begin(fixture.document(world),mask).kind==oe::AutosaveResult::Kind::Saved,
          "Explicit Original autosave did not publish its paid start point");
    const auto id=controller.history_id();through(world,1199);
    check(controller.poll(fixture.document(world),mask,true).kind==oe::AutosaveResult::Kind::None,"Original autosave fired early");
    through(world,1200);check(controller.due(world.ticks()),"Original autosave deadline was not due");
    const auto before_unsafe=files(controller.store().root());
    check(controller.poll(fixture.document(world),mask,false).kind==oe::AutosaveResult::Kind::None&&
          files(controller.store().root())==before_unsafe,"Unsafe Original gesture/modal point wrote a checkpoint");
    through(world,1201);check(controller.poll(fixture.document(world),mask,true).kind==oe::AutosaveResult::Kind::Saved,
          "Original autosave did not save the next safe point");
    const auto once=files(controller.store().root());
    for (int i=0;i<20;++i) check(controller.poll(fixture.document(world),mask,true).kind==oe::AutosaveResult::Kind::None,
        "Paused/same-tick Original autosave wrote repeatedly");
    check(files(controller.store().root())==once,"Paused polls changed Original recovery bytes");
    const auto manual=fixture.root.path/"manual.json";save::write_save(manual,fixture.document(world),fixture.root.data,mask);
    const auto manual_bytes=f::read_text(manual);
    for (unsigned i=0;i<6;++i) {through(world,world.ticks()+1200);check(controller.poll(fixture.document(world),mask,true).kind==
        oe::AutosaveResult::Kind::Saved,"Original checkpoint rotation lost a scheduled save");}
    auto catalog=controller.store().catalog();const auto h=history(catalog,id);
    check(h.entries.size()==save::recovery_checkpoint_limit+1&&
          std::count_if(h.entries.begin(),h.entries.end(),[](const auto& e){return e.start_point;})==1,
          "Original rotation lost the protected start or exceeded five checkpoints");
    const auto source=latest(h);const auto source_bytes=f::read_text(source.path);const auto source_document=save::read_save(source.path);
    auto copy=fixture.restore(source_document);same(source_document,fixture.document(copy));
    oe::AutosaveController branch(app,fixture.root.data,true,budget,Access::OriginalProduction);
    check(branch.begin(fixture.document(copy),mask,save::RecoveryParent{id,source.sequence}).kind==oe::AutosaveResult::Kind::Saved,
          "Exact Original parent checkpoint could not create a separate branch");
    check(branch.history_id()!=id&&f::read_text(source.path)==source_bytes&&f::read_text(manual)==manual_bytes,
          "Original branch overwrote its checkpoint or an unrelated manual save");
    const auto branches=branch.store().catalog();const auto b=history(branches,branch.history_id());
    check(b.parent&&b.parent->history_id==id&&b.parent->sequence==source.sequence,
          "Original branch changed its selected source lineage");
    through(world,world.ticks()+1);
    const auto pristine=files(controller.store().root());
    for (const auto fault:{save::WriteFault::BeforeWrite,save::WriteFault::AfterPartialWrite,save::WriteFault::BeforeRename}) {
        refuses([&]{controller.store().write_checkpoint(id,fixture.document(world),mask,{fault,{}});},
                "Original checkpoint save fault did not refuse");
        check(files(controller.store().root())==pristine,"Failed Original save changed old checkpoints or rotation metadata");
        refuses([&]{controller.store().write_checkpoint(id,fixture.document(world),mask,{{},fault});},
                "Original metadata fault did not refuse");
        check(files(controller.store().root())==pristine,"Failed Original metadata publication changed earlier bytes");
    }
    through(world,controller.next_due_tick());const auto checkpoint_tick=controller.last_checkpoint_tick();
    const auto failed=controller.poll(fixture.document(world),mask,true,{save::WriteFault::BeforeRename,{}});
    check(failed.kind==oe::AutosaveResult::Kind::Failed&&controller.history_id()==id&&
          controller.last_checkpoint_tick()==checkpoint_tick&&controller.next_due_tick()==world.ticks()+1200&&
          files(controller.store().root())==pristine,"Failed Original autosave changed its history, old checkpoint or retry boundary");
    check(controller.poll(fixture.document(world),mask,true).kind==oe::AutosaveResult::Kind::None,
          "Failed Original autosave retried in the same paused frame");
    const auto source_world=fixture.restore(save::read_save(controller.store().root()/id/"start.json"));
    const auto snapshot=world.snapshot();
    refuses([&]{controller.store().write_checkpoint(id,fixture.document(source_world),mask);},"Original history accepted time reversal");
    refuses([&]{branch.store().create_history(fixture.document(world),mask,save::RecoveryParent{id,source.sequence});},
            "Original branch accepted a different World from its selected source");
    refuses([&]{branch.store().create_history(fixture.document(world),mask,save::RecoveryParent{"history-1-abcdef12",0});},
            "Original branch accepted a nonexistent parent history");
    refuses([&]{branch.store().create_history(fixture.document(world),mask,save::RecoveryParent{id,9999});},
            "Original branch accepted a nonexistent parent checkpoint");
    check(world.snapshot()==snapshot&&files(controller.store().root())==pristine,
          "Rejected Original time/lineage request changed World or files");
    save::RecoveryStore tiny(fixture.root.path/"tiny",fixture.root.data,1,Access::OriginalProduction);
    refuses([&]{tiny.create_history(fixture.document(world),mask);},"Original history exceeded its storage budget");
    check(fs::is_empty(tiny.root()),"Failed initial Original budget publication retained managed files");
    const auto failed_app=fixture.root.path/"failed-start";
    save::RecoveryStore failing(failed_app,fixture.root.data,budget,Access::OriginalProduction);
    for (const auto fault:{save::WriteFault::BeforeWrite,save::WriteFault::AfterPartialWrite,save::WriteFault::BeforeRename}) {
        refuses([&]{failing.create_history(fixture.document(world),mask,std::nullopt,{fault,{}});},"Initial Original save fault succeeded");
        refuses([&]{failing.create_history(fixture.document(world),mask,std::nullopt,{{},fault});},"Initial Original metadata fault succeeded");
        check(fs::is_empty(failing.root()),"Failed unpublished Original start retained files");
    }
}
void capture_failure_backoff() {
    q::Fixture fixture;auto world=fixture.world();q::build(world,true);
    const auto mask=fixture.policy->building_mask();
    oe::AutosaveController controller(fixture.root.path/"capture-backoff",fixture.root.data,true,budget,Access::OriginalProduction);
    check(controller.begin(fixture.document(world),mask).kind==oe::AutosaveResult::Kind::Saved,"Capture-backoff fixture lacks actual start");
    const auto id=controller.history_id();through(world,1250);const auto prior=files(controller.store().root());
    const auto failed=controller.capture_failed(world.ticks(),"actual source capture unavailable");
    check(failed.kind==oe::AutosaveResult::Kind::Failed && controller.history_id()==id && controller.last_checkpoint_tick()==0 &&
          controller.last_error()=="actual source capture unavailable" && !controller.due(2449) && controller.due(2450) &&
          files(controller.store().root())==prior,"Capture failure lost history/checkpoint, backoff or file invariants");
    oe::AutosaveController disabled(fixture.root.path/"capture-disabled",fixture.root.data,false,budget,Access::OriginalProduction);
    check(disabled.capture_failed(world.ticks(),"unused source").kind==oe::AutosaveResult::Kind::None &&
          !disabled.due(2450) && !fs::exists(fixture.root.path/"capture-disabled"),"Disabled recovery reported or wrote a capture failure");
}
void identity_metadata_and_model_refusals() {
    q::Fixture fixture;auto world=fixture.world();q::build(world,true);through(world,120);
    const auto app=fixture.root.path/"hostile-app";const auto mask=fixture.policy->building_mask();
    save::RecoveryStore store(app,fixture.root.data,budget,Access::OriginalProduction);
    const auto document=fixture.document(world);const auto id=store.create_history(document,mask);
    const auto pristine=files(store.root());
    auto bad=document;bad.world.original_production->development_revision=26;
    refuses([&]{store.create_history(bad,mask);},"Original recovery accepted an old revision");
    refuses([&]{store.write_checkpoint(id,bad,mask);},"Original checkpoint accepted an old revision");
    bad=document;bad.source_schema_version=25;
    refuses([&]{store.create_history(bad,mask);},"Original recovery silently reinterpreted a different source schema");
    bad=document;bad.world.original_economy->model_source->sha256=std::string(64,'0');
    refuses([&]{store.create_history(bad,mask);},"Original recovery accepted forged model provenance");
    refuses([&]{store.write_checkpoint(id,bad,mask);},"Original history accepted different model provenance");
    const auto hard=save::prepare_original_production_rules_model(fixture.root.data,oe::original::Difficulty::Hard);
    auto hard_world=sim::World::create_original_production_development(fixture.policy,hard);through(hard_world,120);
    refuses([&]{store.write_checkpoint(id,fixture.document(hard_world),mask);},"Original history changed saved difficulty");
    const auto other_map=fixture.root.data/"Cities/Other.map";
    f::write_text(other_map,f::read_text(fixture.root.data/f::map_relative));bad=document;bad.map_relative="Cities/Other.map";
    refuses([&]{store.write_checkpoint(id,bad,mask);},"Original history changed source map");
    const auto old_model=save::prepare_original_rules_model(fixture.root.data);
    for (const unsigned rule:{7U,8U,9U}) {
        const auto old=rule==7?sim::World::create_original_economy_development(fixture.policy,old_model):
            rule==8?sim::World::create_original_influence_development(fixture.policy,old_model):
                    sim::World::create_original_goods_development(fixture.policy,old_model);
        const auto old_document=fixture.document(old);
        refuses([&]{store.create_history(old_document,mask);},"Explicit Original capability opened a frozen developer generation");
        refuses([&]{store.write_checkpoint(id,old_document,mask);},"Frozen generation appended to an Original history");
    }
    sim::World legacy(f::width,f::height,mask);const auto legacy_document=fixture.document(legacy);
    refuses([&]{store.write_checkpoint(id,legacy_document,mask);},"Legacy snapshot appended to an Original history");
    const auto legacy_id=store.create_history(legacy_document,mask);
    const auto with_legacy=files(store.root());
    refuses([&]{store.write_checkpoint(legacy_id,document,mask);},"Original snapshot appended to a legacy history");
    check(files(store.root())==with_legacy,"Refused version/model/map change modified an existing history");
    const auto model=fixture.root.data/"Model/EmperorBuildingModels.txt";const auto model_bytes=f::read_text(model);
    f::write_text(model,model_bytes+"\n");
    auto catalog=store.catalog();check(!history(catalog,id).entries.front().loadable(),"Catalog trusted a changed current Original model");
    refuses([&]{store.write_checkpoint(id,document,mask);},"Checkpoint accepted a changed actual model source");
    refuses([&]{store.create_history(document,mask);},"Initial checkpoint accepted a changed actual model source");
    check(files(store.root())==with_legacy,"Changed-model refusal damaged prior recovery bytes");
    f::write_text(model,model_bytes);catalog=store.catalog();check(latest(history(catalog,id)).loadable(),"Catalog retained stale failed model state");
    const auto missing_model=fixture.root.path/"model-missing.txt";fs::rename(model,missing_model);
    check(!history(store.catalog(),id).entries.front().loadable(),"Catalog trusted an unavailable model source");
    refuses([&]{store.write_checkpoint(id,document,mask);},"Checkpoint accepted an unavailable source model");
    check(files(store.root())==with_legacy,"Missing-model failure changed an earlier checkpoint");fs::rename(missing_model,model);
    const auto metadata=store.root()/id/"history.json";const auto metadata_bytes=f::read_text(metadata);
    const auto original=Json::parse(metadata_bytes);
    const auto reject_metadata=[&](const std::string& bytes) {
        f::write_text(metadata,bytes);const auto hostile=files(store.root());
        const auto broken=store.catalog();const auto& h=history(broken,id);
        check(!h.error.empty()||std::any_of(h.entries.begin(),h.entries.end(),[](const auto& e){return !e.loadable();}),
              "Hostile Original recovery metadata remained loadable");
        refuses([&]{store.write_checkpoint(id,document,mask);},"Hostile recovery metadata allowed checkpoint publication");
        check(files(store.root())==hostile,"Metadata refusal overwrote the supplied hostile files or earlier saves");
        f::write_text(metadata,metadata_bytes);
    };
    const auto extra_array=[](std::string bytes,std::size_t depth) {
        bytes.insert(bytes.find('{')+1,"\"untrusted_extra\":"+std::string(depth,'[')+"0"+
                     std::string(depth,']')+",");
        return bytes;
    };
    const auto deep_original=extra_array(metadata_bytes,64);
    f::write_text(metadata,deep_original);
    check(history(store.catalog(),id).error.find("nesting")!=std::string::npos,
          "Deep Original metadata was not refused by the bounded parser");
    reject_metadata(deep_original);
    const auto legacy_metadata=store.root()/legacy_id/"history.json";
    const auto legacy_metadata_bytes=f::read_text(legacy_metadata);
    f::write_text(legacy_metadata,extra_array(legacy_metadata_bytes,4));
    const auto shallow_legacy_files=files(store.root());
    check(latest(history(store.catalog(),legacy_id)).loadable(),
          "Bounded metadata parser changed shallow legacy extra-field acceptance");
    check(files(store.root())==shallow_legacy_files,"Shallow legacy catalog inspection changed recovery files");
    f::write_text(legacy_metadata,extra_array(legacy_metadata_bytes,64));
    const auto deep_legacy_files=files(store.root());
    check(history(store.catalog(),legacy_id).error.find("nesting")!=std::string::npos,
          "Deep legacy metadata was not refused by the bounded parser");
    check(files(store.root())==deep_legacy_files,"Deep legacy metadata refusal changed recovery files");
    f::write_text(legacy_metadata,legacy_metadata_bytes);
    for (unsigned mutation=0;mutation<7;++mutation) {
        auto j=original;
        if(mutation==0) j["next_sequence"]=UINT64_MAX;
        if(mutation==1) j["next_sequence"]=2;
        if(mutation==2) j["checkpoints"][0]["population"]=world.total_population()+1;
        if(mutation==3) j["checkpoints"][0]["population"]=(std::uint64_t{1}<<32)+static_cast<unsigned>(world.total_population());
        if(mutation==4) j["checkpoints"][0]["treasury"]=UINT64_MAX;
        if(mutation==5) j["checkpoints"][0]["filename"]="../outside.json";
        if(mutation==6) j["checkpoints"][0]["sequence"]=1;
        reject_metadata(j.dump());
    }
    for (const auto& [name,value]:original.items()) { (void)value;auto j=original;j.erase(name);reject_metadata(j.dump()); }
    for (const auto& [name,value]:original["checkpoints"][0].items()) {
        (void)value;auto j=original;j["checkpoints"][0].erase(name);reject_metadata(j.dump());
    }
    {auto j=original;j["unexpected"]={{"original_house",1}};reject_metadata(j.dump());}
    {auto j=original;j["checkpoints"][0]["unexpected"]=0;reject_metadata(j.dump());}
    {auto j=original;j["parent"]={{"history_id",id},{"checkpoint_sequence",0},{"unexpected",true}};reject_metadata(j.dump());}
    {auto j=original;j["version"]=(std::uint64_t{1}<<32)+1;reject_metadata(j.dump());}
    {auto j=original;j["next_sequence"]=true;reject_metadata(j.dump());}
    {auto j=original;j["checkpoints"][0]["rule_version"]=(std::uint64_t{1}<<32)+10;reject_metadata(j.dump());}
    {auto j=original;j["checkpoints"][0]["schema"]=(std::uint64_t{1}<<32)+26;reject_metadata(j.dump());}
    {auto j=original;j["checkpoints"][0]["population"]=-1;reject_metadata(j.dump());}
    {auto j=original;j["checkpoints"][0]["treasury"]=true;reject_metadata(j.dump());}
    {auto j=original;j["checkpoints"].push_back(j["checkpoints"][0]);reject_metadata(j.dump());}
    {auto bytes=metadata_bytes;bytes.insert(bytes.find('{')+1,"\"version\":1,");reject_metadata(bytes);}
    {auto bytes=metadata_bytes;bytes.insert(bytes.find('{')+1,"\"parent\":null,");reject_metadata(bytes);}
    {auto bytes=metadata_bytes;const auto at=bytes.find("\"population\"");check(at!=std::string::npos,"Population key missing from fixture");
     bytes.insert(at,"\"population\":"+std::to_string(world.total_population())+",");reject_metadata(bytes);}
    check(files(store.root())==with_legacy&&pristine.size()<with_legacy.size(),"Metadata tests did not restore their private authored input");
    const auto source=store.root()/id/"start.json";const auto source_bytes=f::read_text(source);
    const auto sentinel=fixture.root.path/"outside-sentinel.json";f::write_text(sentinel,source_bytes);
    fs::remove(source);fs::create_symlink(sentinel,source);
    check(!history(store.catalog(),id).entries.front().loadable(),"Original catalog followed a checkpoint symlink");
    refuses([&]{store.write_checkpoint(id,document,mask);},"Original checkpoint followed a symlinked source");
    refuses([&]{store.create_history(document,mask,save::RecoveryParent{id,0});},"Original branch followed a parent symlink");
    check(f::read_text(sentinel)==source_bytes,"Recovery failure overwrote an external symlink target");
    fs::remove(source);f::write_text(source,source_bytes);
    const auto marker=store.root()/id/"keep-unknown.txt";f::write_text(marker,"private unknown file\n");
    const auto other_before=files(store.root()/legacy_id);store.delete_history(id);
    check(fs::exists(marker)&&f::read_text(marker)=="private unknown file\n"&&files(store.root()/legacy_id)==other_before&&
          f::read_text(sentinel)==source_bytes,"Bounded history deletion removed unknown/source/other-history bytes");
}
void catalog_identity_chronology_and_sequence() {
    q::Fixture fixture;auto world=fixture.world();q::build(world,true);through(world,120);
    const auto mask=fixture.policy->building_mask();
    save::RecoveryStore store(fixture.root.path/"catalog-app",fixture.root.data,budget,Access::OriginalProduction);
    const auto id=store.create_history(fixture.document(world),mask);
    for (unsigned i=0;i<5;++i) {through(world,world.ticks()+1);store.write_checkpoint(id,fixture.document(world),mask);}
    const auto original_catalog=store.catalog();const auto original_history=history(original_catalog,id);
    const auto checkpoint=sequence(original_history,1);
    const auto source_bytes=f::read_text(checkpoint.path);
    const auto metadata=store.root()/id/"history.json";
    const auto metadata_bytes=f::read_text(metadata);const auto parsed=Json::parse(metadata_bytes);
    const auto untouched=files(store.root());
    const auto replace_checkpoint=[&](const save::SaveDocument& replacement,int population) {
        save::write_save(checkpoint.path,replacement,fixture.root.data,mask);
        const auto saved=save::read_save(checkpoint.path);auto changed=parsed;
        for (auto& entry:changed["checkpoints"]) if(entry["sequence"]==1) {
            entry["tick"]=saved.world.ticks;entry["map"]=saved.map_relative.generic_string();
            entry["profile"]=sim::rules_profile_name(saved.world.profile);entry["rule_version"]=saved.world.rule_version;
            entry["schema"]=saved.source_schema_version;entry["population"]=population;entry["treasury"]=saved.world.treasury;
        }
        f::write_text(metadata,changed.dump());const auto hostile=files(store.root());
        const auto catalog=store.catalog();const auto& h=history(catalog,id);
        check(h.error.empty()&&!sequence(h,1).loadable()&&sequence(h,0).loadable()&&sequence(h,5).loadable(),
              "Catalog trusted mixed Original identity/backwards time or discarded independent valid points");
        refuses([&]{store.write_checkpoint(id,fixture.document(world),mask);},"Inconsistent Original history allowed append");
        bool parent_refused=false;
        try {store.create_history(fixture.document(world),mask,save::RecoveryParent{id,1});}
        catch (const std::exception& error) {
            check(std::string(error.what()).find("original recovery parent checkpoint is unavailable")!=std::string::npos,
                  "Original branch was refused for a different reason than its unavailable parent");
            parent_refused=true;++refusals;
        }
        check(parent_refused,"Inconsistent checkpoint remained a valid Original branch parent");
        check(files(store.root())==hostile,"Inconsistent history inspection/refusal changed prior bytes");
        const auto saved_good=save::read_save(sequence(h,5).path);
        const auto restored_good=fixture.restore(saved_good);const auto good=fixture.document(restored_good);
        same(saved_good,good);
        const auto branch=store.create_history(good,mask,save::RecoveryParent{id,5});
        check(latest(history(store.catalog(),branch)).loadable(),"Valid sibling checkpoint could not branch independently");
        store.delete_history(branch);
        f::write_text(checkpoint.path,source_bytes);f::write_text(metadata,metadata_bytes);
        check(files(store.root())==untouched,"Catalog identity test did not restore authored input");
    };
    const auto hard_model=save::prepare_original_production_rules_model(fixture.root.data,oe::original::Difficulty::Hard);
    auto hard_world=sim::World::create_original_production_development(fixture.policy,hard_model);
    f::buy(hard_world,{sim::CommandType::PlaceRoad,{0,4}});through(hard_world,121);
    replace_checkpoint(fixture.document(hard_world),hard_world.total_population());
    auto before_start=fixture.world();q::build(before_start,true);
    replace_checkpoint(fixture.document(before_start),before_start.total_population());
    sim::World legacy(f::width,f::height,mask,sim::RulesProfile::CityV15);f::buy(legacy,{sim::CommandType::PlaceRoad,{0,4}});
    through(legacy,121);replace_checkpoint(fixture.document(legacy),legacy.total_population());
    const auto other_map=fixture.root.data/"Cities/CatalogOther.map";
    f::write_text(other_map,f::read_text(fixture.root.data/f::map_relative));
    replace_checkpoint(save::make_document(fixture.root.data,"Cities/CatalogOther.map",mask,world),world.total_population());
    // An unreadable protected start must not become an identity or chronology reference.
    const auto start=sequence(original_history,0).path;const auto start_bytes=f::read_text(start);
    f::write_text(start,"{}\n");const auto corrupt_start_files=files(store.root());
    const auto partial=store.catalog();const auto& partial_history=history(partial,id);
    check(!sequence(partial_history,0).loadable()&&sequence(partial_history,1).loadable()&&sequence(partial_history,5).loadable(),
          "Invalid start point contaminated valid Original checkpoint references");
    const auto saved_good=save::read_save(sequence(partial_history,1).path);
    const auto restored_good=fixture.restore(saved_good);const auto good=fixture.document(restored_good);
    same(saved_good,good);
    const auto branch=store.create_history(good,mask,save::RecoveryParent{id,1});store.delete_history(branch);
    check(files(store.root())==corrupt_start_files,"Valid partial-history branch changed its original source files");
    f::write_text(start,start_bytes);
    // The final sequence can be published and remains readable, but cannot wrap.
    auto near_max=parsed;near_max["next_sequence"]=UINT64_MAX-1;
    for (auto& entry:near_max["checkpoints"]) if(entry["sequence"]!=0)
        entry["sequence"]=UINT64_MAX-7+entry["sequence"].get<std::uint64_t>();
    f::write_text(metadata,near_max.dump());
    const auto before_last=store.catalog();
    check(history(before_last,id).error.empty()&&std::all_of(history(before_last,id).entries.begin(),history(before_last,id).entries.end(),
        [](const auto& entry){return entry.loadable();}),"Valid near-exhausted sequence was not readable");
    through(world,world.ticks()+1);const auto last=store.write_checkpoint(id,fixture.document(world),mask);
    const auto exhausted=store.catalog();const auto& exhausted_history=history(exhausted,id);
    check(last.sequence==UINT64_MAX-1&&Json::parse(f::read_text(metadata))["next_sequence"]==UINT64_MAX&&
          exhausted_history.error.empty()&&latest(exhausted_history).sequence==UINT64_MAX-1&&
          std::all_of(exhausted_history.entries.begin(),exhausted_history.entries.end(),[](const auto& entry){return entry.loadable();}),
          "Last Original sequence publication made existing recovery points unreadable");
    const auto exhausted_files=files(store.root());
    refuses([&]{store.write_checkpoint(id,fixture.document(world),mask);},"Exhausted Original sequence wrapped or appended");
    check(files(store.root())==exhausted_files,"Exhausted Original append changed existing recovery bytes");
    const auto last_branch=store.create_history(fixture.document(world),mask,save::RecoveryParent{id,UINT64_MAX-1});
    check(latest(history(store.catalog(),last_branch)).loadable(),"Exhausted history could not be recovered into a new branch");
}
} // namespace

int main() {
    try {
        paid_population_and_binding();scheduling_rotation_branch_and_faults();identity_metadata_and_model_refusals();
        catalog_identity_chronology_and_sequence();capture_failure_backoff();
        std::cout<<"Original Recovery PASS: explicit Rule10 capability, Common/Elite population, exact model-bound histories, "
            "safe scheduling/rotation, immutable branches, "<<roundtrips<<" full roundtrips and "<<refusals<<" hostile/fault refusals\n";
        return 0;
    } catch (const std::exception& error) {std::cerr<<"Original Recovery FAIL: "<<error.what()<<'\n';return 1;}
}
