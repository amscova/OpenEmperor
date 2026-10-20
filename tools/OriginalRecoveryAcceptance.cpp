// Explicit local-only backend recovery/process acceptance. Actual F5/F9 SDL
// handling is tested separately through the normal MenuSession/View path.
#include "../tests/OriginalRecoveryFixture.h"
#include "app/AutosaveController.h"

#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <string_view>

namespace {
namespace q=oe_recovery_fixture;
namespace fs=std::filesystem;
namespace save=openemperor::persistence;
using Json=nlohmann::json;
using Files=std::map<std::string,std::string>;
constexpr std::array<unsigned,3> continuation_ticks{1,51,408};
constexpr auto access=save::RecoveryAccess::OriginalProduction;

bool below(const fs::path& child,const fs::path& parent) {
    return std::mismatch(parent.begin(),parent.end(),child.begin(),child.end()).first==parent.end();
}
void safe_components(const fs::path& path) {
    fs::path prefix;
    for (const auto& component:fs::absolute(path).lexically_normal()) {
        prefix/=component;std::error_code error;const auto status=fs::symlink_status(prefix,error);
        q::check(!error || error==std::errc::no_such_file_or_directory,"Cannot inspect recovery proof path");
        q::check(!fs::is_symlink(status),"Recovery proof path traverses a symlink");
    }
}
void output_target(const fs::path& out,const fs::path& data,bool fresh) {
    safe_components(out);
    const auto repository=fs::canonical(fs::absolute(__FILE__)).parent_path().parent_path();
    const auto allowed=fs::weakly_canonical(repository/".local/original-economy/oe31");
    const auto resolved=fs::weakly_canonical(out);
    q::check(resolved!=allowed && below(resolved,allowed) && !below(resolved,data) && !below(data,resolved),
        "Recovery proof output must be a private OE31 directory disjoint from original data");
    q::check(fresh ? !fs::exists(out):fs::is_directory(out),
        fresh ? "Preserve prior evidence: recovery proof output already exists":"Recovery proof output is missing");
    save::validate_save_target(out/"proof.json",data);
}
std::string read(const fs::path& path) {
    safe_components(path);return q::starter::f::read_text(path);
}
Json json(const fs::path& path) {return Json::parse(read(path));}
void write_json(const fs::path& path,const Json& value) {
    safe_components(path);q::check(!fs::exists(path),"Preserve existing recovery proof report");
    q::starter::f::write_text(path,value.dump(2)+"\n");
}
Files files(const fs::path& root) {
    Files result;
    if (!fs::exists(root)) return result;
    for (const auto& entry:fs::recursive_directory_iterator(root)) {
        q::check(!entry.is_symlink(),"Unexpected symlink inside owned recovery proof");
        if (entry.is_regular_file()) result.emplace(entry.path().lexically_relative(root).generic_string(),read(entry.path()));
    }
    return result;
}
template<class F> void refuses(F&& action,const char* reason) {
    bool rejected=false;try {action();} catch (const std::exception&) {rejected=true;}
    q::check(rejected,reason);
}
save::RecoveryStore store(const fs::path& app,const q::Context& context) {
    return {app,context.data,save::recovery_storage_budget_bytes,access};
}
void continuation(const q::Context& context,const save::SaveDocument& document,
                  const fs::path& out,const fs::path& expected={}) {
    fs::create_directories(out);auto world=q::restore(context,document);const auto begin=world.ticks();
    for (const auto elapsed:continuation_ticks) {
        q::through(world,begin+elapsed);
        const auto path=out/("after-"+std::to_string(elapsed)+".save.json");
        const auto doc=context.document(world);
        if (!expected.empty()) q::same_document(save::read_save(expected/path.filename()),doc);
        q::check(!fs::exists(path),"Continuation proof already exists");
        save::write_save(path,doc,context.data,context.mask);
        if (!expected.empty()) q::check(read(path)==read(expected/path.filename()),"Separate-process canonical save bytes differ");
    }
    write_json(out/"observed.json",q::observe(world));
}
Json atomic_failures(const q::Context& context,const q::Checkpoint& point,const fs::path& out) {
    auto recovery=store(out/"app",context);const auto history=recovery.create_history(point.document,context.mask);
    (void)recovery.write_checkpoint(history,point.document,context.mask);
    const auto protected_files=files(recovery.root());Json cases=Json::array();
    for (const auto fault:{save::WriteFault::BeforeWrite,save::WriteFault::AfterPartialWrite,save::WriteFault::BeforeRename}) {
        refuses([&]{recovery.write_checkpoint(history,point.document,context.mask,{fault,{}});},"Recovery save fault was accepted");
        q::check(files(recovery.root())==protected_files,"Recovery save fault changed earlier complete history bytes");
        cases.push_back({{"kind","checkpoint-save"},{"fault",static_cast<unsigned>(fault)},{"prior_files_exact",true}});
    }
    refuses([&]{recovery.write_checkpoint(history,point.document,context.mask,{{},save::WriteFault::BeforeRename});},
        "Recovery metadata publication fault was accepted");
    q::check(files(recovery.root())==protected_files,"Metadata publication fault changed earlier complete recovery bytes");
    cases.push_back({{"kind","checkpoint-metadata"},{"fault","BeforeRename"},{"prior_files_exact",true}});
    const auto manual=out/"manual.save.json";save::write_save(manual,point.document,context.data,context.mask);
    const auto literal=read(manual);auto future=q::restore(context,point.document);q::through(future,future.ticks()+1);
    for (const auto fault:{save::WriteFault::BeforeWrite,save::WriteFault::AfterPartialWrite,save::WriteFault::BeforeRename}) {
        refuses([&]{save::write_save(manual,context.document(future),context.data,context.mask,fault);},"Manual atomic save fault was accepted");
        q::check(read(manual)==literal,"Manual atomic failure rewrote its protected destination");
        cases.push_back({{"kind","manual-save"},{"fault",static_cast<unsigned>(fault)},{"prior_files_exact",true}});
    }
    return cases;
}
void create(const q::Context& context,const fs::path& out) {
    fs::create_directories(out);Json points=Json::array();
    const auto corpus=q::build_corpus(context,true,[&](const q::Checkpoint& point) {
        const auto folder=out/point.label;fs::create_directories(folder);
        const auto manual=folder/"input.save.json";save::write_save(manual,point.document,context.data,context.mask);
        const auto literal=read(manual);q::same_document(point.document,save::read_save(manual));
        auto recovery=store(folder/"app",context);const auto history=recovery.create_history(point.document,context.mask);
        const auto entry=recovery.write_checkpoint(history,point.document,context.mask);
        const auto catalog=recovery.catalog();
        q::check(catalog.histories.size()==1 && catalog.histories.front().entries.size()==2 && entry.sequence==1,
            "Real source recovery start/checkpoint was not published");
        q::check(std::all_of(catalog.histories.front().entries.begin(),catalog.histories.front().entries.end(),[](const auto& e){return e.loadable();}),
            "Real source recovery checkpoint is not loadable");
        auto restored=q::restore(context,save::read_save(entry.path));q::same_document(point.document,context.document(restored));
        q::check(entry.population==restored.total_population(),"Original recovery summary did not count authoritative House residents");
        continuation(context,point.document,folder/"expected");
        q::check(read(manual)==literal,"Recovery creation altered the independent manual save");
        write_json(folder/"observed.json",q::observe(restored));
        points.push_back({{"label",point.label},{"history_id",history},{"sequence",entry.sequence},
            {"checkpoint",entry.path.lexically_relative(folder).generic_string()},{"tick",point.document.world.ticks},
            {"population",entry.population},{"funds",entry.treasury}});
    });
    const auto faults=atomic_failures(context,corpus[2],out/"atomic-failures");
    write_json(out/"create.json",{{"status","PASS"},{"original_map",context.original_map},{"original_model",context.original_model},
        {"model_sha256",context.model->provenance().sha256},{"data",context.data.generic_string()},
        {"map",context.map_relative.generic_string()},{"snapshot_injection",false},{"points",points},{"atomic_failures",faults},
        {"continuation_ticks",continuation_ticks},{"rule",10},{"schema",26},{"policy",3},{"revision",27},
        {"actual_f5_f9_sdl","separate normal menu/view acceptance"},{"human_acceptance","NOT_RUN"}});
}
void resume(const q::Context& context,const fs::path& out,const std::string& label) {
    const auto manifest=json(out/"create.json");q::check(manifest.at("status")=="PASS" &&
        manifest.at("data")==context.data.generic_string() && manifest.at("model_sha256")==context.model->provenance().sha256,
        "Separate process source differs from the create process");
    const auto& points=manifest.at("points");const auto found=std::find_if(points.begin(),points.end(),[&](const auto& p){return p.at("label")==label;});
    q::check(found!=points.end(),"Unknown recovery proof case");
    const auto folder=out/label;safe_components(folder);const auto result=folder/"resumed";
    q::check(!fs::exists(result),"Preserve prior separate-process replay");
    auto recovery=store(folder/"app",context);const auto before=files(recovery.root());const auto manual=read(folder/"input.save.json");
    const auto catalog=recovery.catalog();const auto history=std::find_if(catalog.histories.begin(),catalog.histories.end(),
        [&](const auto& h){return h.history_id==found->at("history_id").get<std::string>();});
    q::check(history!=catalog.histories.end(),"Fresh process did not find the actual recovery history");
    const auto entry=std::find_if(history->entries.begin(),history->entries.end(),[&](const auto& e){return e.sequence==found->at("sequence").get<std::uint64_t>();});
    q::check(entry!=history->entries.end() && entry->loadable(),"Fresh process did not find a loadable checkpoint");
    const auto source=read(entry->path);const auto document=save::read_save(entry->path);
    q::same_document(save::read_save(folder/"input.save.json"),document);
    auto world=q::restore(context,document);q::same_document(document,context.document(world));
    continuation(context,document,result,folder/"expected");
    q::check(files(recovery.root())==before && read(entry->path)==source && read(folder/"input.save.json")==manual,
        "Separate-process recovery read/continuation changed any supplied recovery/manual bytes");
    const auto child=recovery.create_history(document,context.mask,save::RecoveryParent{entry->history_id,entry->sequence});
    const auto branched=recovery.catalog();q::check(branched.histories.size()==2,"Recovery resume replaced its parent history");
    const auto child_history=std::find_if(branched.histories.begin(),branched.histories.end(),[&](const auto& h){return h.history_id==child;});
    q::check(child_history!=branched.histories.end() && child_history->parent &&
        child_history->parent->history_id==entry->history_id && child_history->parent->sequence==entry->sequence,
        "Fresh process branch lost its selected real parent");
    q::same_document(document,save::read_save(child_history->entries.front().path));
    for (const auto& [name,literal]:before) q::check(read(recovery.root()/name)==literal,"Branch creation rewrote a parent history file");
    write_json(result/"resume.json",{{"status","PASS"},{"label",label},{"source_tick",document.world.ticks},
        {"full_document_equal",true},{"all_three_continuations_byte_equal",true},{"parent_bytes_exact",true},
        {"manual_bytes_exact",true},{"child_history",child},{"actual_new_process",true}});
}
q::Context isolated_source(const q::Context& real,const fs::path& out) {
    auto isolated=real;isolated.data=out/"data";
    isolated.model=save::prepare_original_production_rules_model(isolated.data,oe_recovery_fixture::orig::Difficulty::Normal);
    return isolated;
}
void source_create(const q::Context& context,const fs::path& out) {
    const auto folder=out/"strict-source";q::check(!fs::exists(folder),"Preserve earlier strict-source proof");
    fs::create_directories(folder/"data/Model");fs::create_directories(folder/"data/Cities");
    fs::copy_file(context.data/"Model/EmperorBuildingModels.txt",folder/"data/Model/EmperorBuildingModels.txt");
    fs::copy_file(context.data/context.map_relative,folder/"data"/context.map_relative);
    auto isolated=isolated_source(context,folder);
    const auto document=save::read_save(out/"rich-supplied/input.save.json");auto world=q::restore(isolated,document);
    auto recovery=store(folder/"app",isolated);const auto history=recovery.create_history(document,isolated.mask);
    const auto entry=recovery.write_checkpoint(history,document,isolated.mask);
    continuation(isolated,document,folder/"expected");
    write_json(folder/"create.json",{{"status","PASS"},{"history_id",history},{"sequence",entry.sequence},
        {"source_tick",world.ticks()},{"copied_only_to_ignored_oe31",true},{"original_source_modified",false}});
}
void source_resume(const q::Context& context,const fs::path& out) {
    const auto folder=out/"strict-source";q::check(!fs::exists(folder/"resume.json"),"Strict-source replay already exists");
    const auto manifest=json(folder/"create.json");auto isolated=isolated_source(context,folder);
    auto recovery=store(folder/"app",isolated);const auto catalog=recovery.catalog();
    q::check(catalog.histories.size()==1,"Strict-source process did not find its prior real checkpoint");
    const auto entry=std::find_if(catalog.histories.front().entries.begin(),catalog.histories.front().entries.end(),
        [&](const auto& e){return e.sequence==manifest.at("sequence").get<std::uint64_t>();});
    q::check(entry!=catalog.histories.front().entries.end() && entry->loadable(),"Strict-source restart checkpoint is unavailable");
    const auto document=save::read_save(entry->path);continuation(isolated,document,folder/"resumed",folder/"expected");
    auto offline=q::restore(isolated,document);auto control=q::restore(isolated,document);
    const auto model=isolated.data/"Model/EmperorBuildingModels.txt";const auto original=read(model);
    const auto source_original=read(context.data/"Model/EmperorBuildingModels.txt");const auto protected_history=files(recovery.root());
    const auto hidden=folder/"private-model.hidden";fs::rename(model,hidden);
    refuses([&]{(void)q::restore(isolated,document);},"Restore accepted an absent model despite its cached prepared source");
    auto missing=recovery.catalog();q::check(std::all_of(missing.histories.front().entries.begin(),missing.histories.front().entries.end(),
        [](const auto& e){return !e.loadable();}),"Missing model left an Original recovery entry loadable");
    q::through(offline,offline.ticks()+51);q::through(control,control.ticks()+51);
    q::check(offline.snapshot()==control.snapshot(),"Prepared offline World changed its deterministic continuation");
    q::starter::f::write_text(model,original+"\n");
    refuses([&]{(void)q::restore(isolated,document);},"Restore accepted different model source bytes");
    auto changed=recovery.catalog();q::check(std::all_of(changed.histories.front().entries.begin(),changed.histories.front().entries.end(),
        [](const auto& e){return !e.loadable();}),"Changed model left an Original recovery entry loadable");
    q::starter::f::write_text(model,original);fs::remove(hidden);
    q::same_document(document,isolated.document(q::restore(isolated,document)));
    q::check(files(recovery.root())==protected_history && read(context.data/"Model/EmperorBuildingModels.txt")==source_original,
        "Strict-source failures changed checkpoint bytes or the supplied original installation");
    write_json(folder/"resume.json",{{"status","PASS"},{"new_process_byte_replay",true},{"missing_model_refused",true},
        {"changed_model_refused",true},{"offline_51tick_world_exact",true},{"prepared_model_not_restore_authority",true},
        {"parent_recovery_bytes_exact",true},{"original_source_modified",false}});
}
} // namespace

int main(int argc,char** argv) {
    try {
        q::check(argc>=4,"Usage: original-recovery-acceptance --create|--source-create|--source-resume DATA OUT | --resume DATA OUT LABEL");
        const std::string_view mode=argv[1];q::check((mode=="--resume" && argc==5) ||
            ((mode=="--create" || mode=="--source-create" || mode=="--source-resume") && argc==4),"Unknown recovery acceptance mode/arity");
        const auto data=fs::canonical(argv[2]);const auto out=fs::absolute(argv[3]);output_target(out,data,mode=="--create");
        const auto context=q::Context::real(data);
        if (mode=="--create") create(context,out);
        else if (mode=="--resume") resume(context,out,argv[4]);
        else if (mode=="--source-create") source_create(context,out);
        else source_resume(context,out);
        std::cout<<"Original real-Xia recovery "<<mode<<" PASS; human/native NOT_RUN\n";return 0;
    } catch (const std::exception& error) {std::cerr<<"Original recovery acceptance FAIL: "<<error.what()<<'\n';return 1;}
}
