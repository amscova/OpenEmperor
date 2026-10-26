// Real SDL F5/F9 input against earned Rule10 states and the public View factory.
#include "OriginalRecoveryFixture.h"
#include "app/SandboxView.h"
#include "app/VisualSelection.h"
#include <SDL3/SDL.h>

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string_view>

namespace {
namespace q=oe_recovery_fixture;
namespace oe=openemperor;
namespace save=oe::persistence;
namespace fs=std::filesystem;
using Json=nlohmann::json;
using q::check;
constexpr std::array<std::string_view,10> labels{
    "paid-empty","moving-reserved","rich-supplied","before-consumption","after-consumption",
    "before-devolution","after-devolution","before-neutral-demolition","after-neutral-demolition","debt"};
struct Options {fs::path data,corpus,captures,resources;};
Options options(int argc,char** argv) {
    Options result;
    for (int i=1;i<argc;++i) {
        const std::string_view key=argv[i];
        check(i+1<argc,"Recovery View option requires a path");const fs::path value=argv[++i];
        if (key=="--real") {check(result.data.empty(),"Duplicate --real");result.data=value;}
        else if (key=="--corpus-dir") {check(result.corpus.empty(),"Duplicate --corpus-dir");result.corpus=value;}
        else if (key=="--capture") {check(result.captures.empty(),"Duplicate --capture");result.captures=value;}
        else if (key=="--resources") {check(result.resources.empty(),"Duplicate --resources");result.resources=value;}
        else throw std::runtime_error("Unknown Recovery View option: "+std::string(key));
    }
    check(!result.data.empty(),"Usage: original-recovery-view-tests --real DATA [--corpus-dir FRESH_TOOL_OUT] [--capture NEW_OUT] [--resources ROOT]");
    if (result.corpus.empty()) if (const auto* env=std::getenv("OPENEMPEROR_ORIGINAL_RECOVERY_CORPUS_DIR");env && *env) result.corpus=env;
    return result;
}
bool below(const fs::path& path,const fs::path& root) {
    return std::mismatch(root.begin(),root.end(),path.begin(),path.end()).first==root.end();
}
void no_symlinks(const fs::path& path) {
    fs::path prefix;
    for (const auto& component:fs::absolute(path).lexically_normal()) {
        prefix/=component;std::error_code error;const auto status=fs::symlink_status(prefix,error);
        check(!error || error==std::errc::no_such_file_or_directory,"Cannot inspect Recovery View path");
        check(!fs::is_symlink(status),"Recovery View path traverses a symlink");
    }
}
std::string read(const fs::path& path) {no_symlinks(path);return q::starter::f::read_text(path);}
void write(const fs::path& path,const std::string& bytes) {
    no_symlinks(path);q::starter::f::write_text(path,bytes);
}
struct Temp {
    fs::path root=fs::canonical(fs::temp_directory_path())/("openemperor-original-recovery-view-"+
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    Temp() {check(fs::create_directory(root),"Could not create fresh private Recovery View root");}
    ~Temp() {std::error_code ignored;fs::remove_all(root,ignored);}
};
struct Sdl {
    SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;
    Sdl() {
        check(SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy") && SDL_Init(SDL_INIT_VIDEO),"Initialize actual dummy SDL");
        window=SDL_CreateWindow("Original Recovery View acceptance",1280,800,SDL_WINDOW_HIDDEN);
        check(window!=nullptr,SDL_GetError());renderer=SDL_CreateRenderer(window,"software");check(renderer!=nullptr,SDL_GetError());
    }
    ~Sdl() {if(renderer)SDL_DestroyRenderer(renderer);if(window)SDL_DestroyWindow(window);SDL_Quit();}
};
void event(oe::SandboxView& view,SDL_Event value) {
    check(SDL_PushEvent(&value),"Queue actual SDL Recovery View input");bool running=true;
    while (SDL_PollEvent(&value)) view.handle_event(value,running);
    check(running,"Actual F5/F9 input unexpectedly closed its View");
}
void key(oe::SandboxView& view,SDL_Keycode code) {
    SDL_Event value{};value.type=SDL_EVENT_KEY_DOWN;value.key.key=code;event(view,value);
    value={};value.type=SDL_EVENT_KEY_UP;value.key.key=code;event(view,value);
}
void visuals(oe::SandboxView& view,const q::Context& context,const fs::path& resources) {
    if (resources.empty()) return;
    const auto selected=oe::detect_and_select_visual_profiles(context.data,resources);
    if (!selected.walker.empty()) view.set_walker_visuals(selected.walker,selected.walker_source);
    if (!selected.building.empty()) view.set_building_visuals(selected.building,selected.building_source);
    if (!selected.road.empty()) view.set_road_visuals(selected.road,selected.road_source);
    view.set_fire_visuals(selected.fire,selected.fire_source,selected.fire_fallback_reason);
    view.set_fire_inspector_visuals(selected.fire_inspector,selected.fire_inspector_source,selected.fire_inspector_fallback_reason);
    view.set_market_walker_visuals(selected.market_walker,selected.market_walker_source,selected.market_walker_fallback_reason);
    view.set_service_walker_visuals(selected.service_walker,selected.service_walker_source,selected.service_walker_fallback_reason);
    view.set_health_walker_visuals(selected.health_walker,selected.health_walker_source,selected.health_walker_fallback_reason);
    view.set_original_house_visuals(selected.original_house,selected.original_house_source,selected.original_house_fallback_reason);
}
void capture(oe::SandboxView& view,SDL_Renderer* renderer,const fs::path& folder,const std::string& label) {
    if (folder.empty() || (label!="moving-reserved" && label!="after-devolution" && label!="debt")) return;
    check(view.render(),"Render actual recovered View capture");
    std::unique_ptr<SDL_Surface,decltype(&SDL_DestroySurface)> image(SDL_RenderReadPixels(renderer,nullptr),SDL_DestroySurface);
    check(image && image->w==1280 && image->h==800,"Actual Recovery View frame is not1280x800");
    const auto path=folder/(label+".f9-restored.png");check(!fs::exists(path),"Preserve previous Recovery View capture");
    check(SDL_SavePNG(image.get(),path.string().c_str()),"Write actual recovered View PNG");
    auto report=q::observe(view.world());report["load_generation"]=view.load_generation();
    report["scripted_sdl"]=true;report["native_os_input"]=false;report["human_acceptance"]="NOT_RUN";
    write(folder/(label+".observed.json"),report.dump(2)+"\n");
}
std::vector<q::Checkpoint> corpus(const q::Context& context,const fs::path& directory,
                                 std::map<fs::path,std::string>& protected_inputs) {
    if (directory.empty()) return q::build_corpus(context,true);
    no_symlinks(directory);const auto root=fs::canonical(directory),manifest_path=root/"create.json";
    const auto bytes=read(manifest_path);const auto manifest=Json::parse(bytes);protected_inputs.emplace(manifest_path,bytes);
    check(manifest.at("status")=="PASS" && manifest.at("original_map")==true && manifest.at("original_model")==true &&
        manifest.at("snapshot_injection")==false && manifest.at("rule")==10 && manifest.at("schema")==26 &&
        manifest.at("policy")==3 && manifest.at("revision")==27 && manifest.at("data")==context.data.generic_string() &&
        manifest.at("map")==context.map_relative.generic_string() && manifest.at("model_sha256")==context.model->provenance().sha256,
        "Recovery View corpus is not the matching real Xia/Normal active Rule10 tool result");
    check(manifest.at("points").is_array() && manifest.at("points").size()==labels.size(),"Recovery View corpus must contain exactly ten earned states");
    std::vector<q::Checkpoint> result;
    for (const auto name:labels) {
        const auto matches=std::count_if(manifest.at("points").begin(),manifest.at("points").end(),
            [&](const auto& row){return row.at("label")==name;});
        check(matches==1,"Recovery View corpus omits or duplicates a required earned case");
        const auto path=root/std::string(name)/"input.save.json";const auto literal=read(path);
        check(below(fs::canonical(path),root),"Recovery View corpus input escapes its root");
        const auto document=save::read_save(path);auto world=q::restore(context,document);
        const auto described=std::find_if(manifest.at("points").begin(),manifest.at("points").end(),
            [&](const auto& row){return row.at("label")==name;});
        check(described->at("tick")==document.world.ticks && described->at("population")==world.total_population() &&
            described->at("funds")==world.treasury(),"Recovery View manifest summary differs from its actual checkpoint");
        q::same_document(document,context.document(world));protected_inputs.emplace(path,literal);
        result.push_back({std::string(name),document});
    }
    return result;
}
void earned_cases(const q::Context& context,const std::vector<q::Checkpoint>& points) {
    const auto named=[&](std::string_view label)->const save::SaveDocument& {
        const auto found=std::find_if(points.begin(),points.end(),[&](const auto& p){return p.label==label;});
        check(found!=points.end(),"Required earned Recovery View case is missing");return found->document;
    };
    const auto& empty=named("paid-empty");check(empty.world.ticks==0 && empty.world.original_economy &&
        std::all_of(empty.world.original_economy->houses.begin(),empty.world.original_economy->houses.end(),
            [](const auto& h){return h.house.type==q::orig::HouseType::Vacant && !h.house.occupants;}),
        "Paid-empty Recovery View case grants age or residents");
    const auto moving=q::restore(context,named("moving-reserved"));
    check(q::reserved_begun_edge(moving),"Moving Recovery View case lacks actual typed cargo/reservation on a begun edge");
    const auto rich=q::restore(context,named("rich-supplied"));
    check(q::consumed(rich)>0 && rich.total_population()>0 && !rich.original_production_state()->services.witnesses.empty(),
        "Rich Recovery View case lacks actual consumption/residents/visits");
    for (const auto& pair:std::array<std::pair<std::string_view,std::string_view>,2>{{
        {"before-consumption","after-consumption"},{"before-devolution","after-devolution"}}}) {
        const auto& before=named(pair.first);const auto& after=named(pair.second);
        check(after.world.ticks==before.world.ticks+1,"Recovery View boundary cases are not consecutive real ticks");
        const auto dispatch=q::orig::next_scheduler_dispatch(before.world.ticks);
        check(dispatch && dispatch->evolution && (pair.first!="before-consumption" ||
            (dispatch->consumption && dispatch->position.step==7)),"Recovery View boundary has the wrong original phase/step");
        auto expected=q::restore(context,before);expected.tick();q::same_document(after,context.document(expected));
        if (pair.first=="before-consumption") {
            const auto prior=q::restore(context,before);
            check(q::consumed(expected)>q::consumed(prior),"Consumption View boundary did not consume actually delivered goods");
        } else {
            bool actual_down=false;
            for (const auto& h:before.world.original_economy->houses)
                for (const auto& next:after.world.original_economy->houses) if (h.building_id==next.building_id &&
                    h.house.type==q::orig::HouseType::PlainCottage && next.house.type==q::orig::HouseType::Hut &&
                    next.house.occupants>0 && !next.house.ancestor) actual_down=true;
            check(actual_down,"Devolution View boundary did not retain an actual occupied Plain-to-Hut transition");
        }
    }
    const auto& before=named("before-neutral-demolition");const auto& after=named("after-neutral-demolition");
    auto expected=q::restore(context,before);std::optional<oe::simulation::BuildingId> removed;
    for (const auto& b:before.world.buildings)
        if (std::none_of(after.world.buildings.begin(),after.world.buildings.end(),[&](const auto& next){return next.id==b.id;})) {
            check(!removed,"Demolition View boundary removed more than one owner");removed=b.id;
        }
    check(removed.has_value(),"Demolition View boundary removed no actual paid owner");
    const auto command=expected.execute(oe::simulation::demolish_building(*removed));
    check(command.accepted && command.changed,"Demolition View boundary was not an ordinary legal command");
    q::same_document(after,context.document(expected));
    const auto debt=q::restore(context,named("debt"));
    check(debt.treasury()<0 && debt.maintenance_spent_total()>0 && debt.city_economy_valid(),
        "Debt Recovery View case lacks actual signed debt and conserved finance");
}
Json exercise(const q::Context& context,const q::Checkpoint& point,const fs::path& target,Sdl& sdl,
              const Options& selected) {
    check(!fs::exists(target),"F5 target must be a fresh private test file");
    auto view=oe::SandboxView::create_original_production(oe::maps::load_stored_map_session(context.data,context.map_relative,
        oe::maps::FootprintPolicy::EdgeByte4x4Preview,oe::maps::StoredGraphicsProfile::Slot8));
    view->configure_save(context.data,context.map_relative,target,point.document);visuals(*view,context,selected.resources);
    view->initialize(sdl.window,sdl.renderer);
    SDL_Event focus{};focus.type=SDL_EVENT_WINDOW_FOCUS_GAINED;event(*view,focus);
    check(view->paused(),"Earned Original View did not initialize paused");
    q::same_document(point.document,view->capture_save_document());
    const auto save_generation=view->save_generation(),load_generation=view->load_generation();
    key(*view,SDLK_F5);
    check(fs::is_regular_file(target) && view->save_generation()==save_generation+1 && view->load_generation()==load_generation,
        "Actual SDL F5 did not save once or incorrectly counted a load");
    const auto saved=save::read_save(target);const auto saved_bytes=read(target);q::same_document(point.document,saved);
    key(*view,SDLK_SPACE);check(!view->paused(),"Actual Space did not run the restored View");
    view->update(2.0/oe::simulation::Rules::ticks_per_second);
    auto expected=q::restore(context,saved);q::through(expected,saved.world.ticks+2);
    q::same_document(context.document(expected),view->capture_save_document());
    const auto advanced=view->capture_save_document();const auto running=view->paused();
    const auto failed_load=[&](const char* name) {
        const auto bytes_before=read(target);key(*view,SDLK_F9);
        q::same_document(advanced,view->capture_save_document());
        check(view->load_generation()==load_generation && view->save_generation()==save_generation+1 && view->paused()==running &&
            view->last_message().starts_with("Load failed:") && read(target)==bytes_before,
            std::string("Failed actual F9 changed World/generation/pause/source bytes: ")+name);
    };
    write(target,"{");failed_load("malformed-json");
    auto wrong_model=Json::parse(saved_bytes);wrong_model["world"]["original_economy"]["model_source"]["sha256"]=std::string(64,'0');
    write(target,wrong_model.dump(2));failed_load("wrong-model-sha256");
    const auto private_parked=target.parent_path()/(point.label+".parked");fs::rename(target,private_parked);
    key(*view,SDLK_F9);q::same_document(advanced,view->capture_save_document());
    check(view->load_generation()==load_generation && view->paused()==running && view->last_message().starts_with("Load failed:") &&
        !fs::exists(target),"Missing-target actual F9 changed World/generation or manufactured a save");
    fs::rename(private_parked,target);write(target,saved_bytes);
    key(*view,SDLK_F9);q::same_document(saved,view->capture_save_document());
    check(view->paused() && view->load_generation()==load_generation+1 && view->save_generation()==save_generation+1 &&
        read(target)==saved_bytes,"Successful actual F9 did not pause/count exactly one full restore");
    // Identical saved ticks still represent a second successful explicit load.
    key(*view,SDLK_F9);q::same_document(saved,view->capture_save_document());
    check(view->paused() && view->load_generation()==load_generation+2 && read(target)==saved_bytes,
        "Second same-tick successful actual F9 was not distinguished from a failed load");
    capture(*view,sdl.renderer,selected.captures,point.label);
    Json row{{"label",point.label},{"saved_tick",saved.world.ticks},{"advanced_tick",advanced.world.ticks},
        {"actual_sdl_f5",true},{"actual_sdl_f9_successes",2},{"actual_sdl_f9_refusals",3},
        {"load_generation_before",load_generation},{"load_generation_after",view->load_generation()},
        {"full_document_equal",true},{"regular_two_tick_world_equal",true},{"failed_load_world_generation_exact",true},
        {"manual_save_bytes_exact",true},{"paused_after_success",view->paused()}};
    view->shutdown();return row;
}
} // namespace

int main(int argc,char** argv) {
    try {
        auto selected=options(argc,argv);selected.data=fs::canonical(selected.data);
        // Context::real owns and closes its preparation SDL instance first.
        const auto context=q::Context::real(selected.data);std::map<fs::path,std::string> protected_inputs;
        const auto points=corpus(context,selected.corpus,protected_inputs);check(points.size()==labels.size(),"Unexpected earned corpus size");
        earned_cases(context,points);
        if (!selected.resources.empty()) selected.resources=fs::canonical(selected.resources);
        if (!selected.captures.empty()) {
            selected.captures=fs::absolute(selected.captures);no_symlinks(selected.captures);
            const auto resolved=fs::weakly_canonical(selected.captures);
            const auto repository=fs::canonical(fs::absolute(__FILE__)).parent_path().parent_path();
            const auto allowed=fs::weakly_canonical(repository/".local/original-economy/oe31");
            check(!fs::exists(selected.captures) && resolved!=allowed && below(resolved,allowed) &&
                !below(resolved,context.data) && !below(context.data,resolved),
                "Capture target must be fresh under ignored OE31 and disjoint from original data");
            if (!selected.corpus.empty()) {const auto source=fs::canonical(selected.corpus);check(!below(resolved,source) && !below(source,resolved),
                "Capture output must be disjoint from the immutable corpus input");}
            fs::create_directories(selected.captures);
        }
        Temp temp;Sdl sdl;Json cases=Json::array();
        for (const auto& point:points) cases.push_back(exercise(context,point,temp.root/(point.label+".manual.save.json"),sdl,selected));
        for (const auto& [path,literal]:protected_inputs) check(read(path)==literal,"Actual View F5/F9 altered an original earned corpus input");
        Json report{{"status","PASS"},{"cases",cases},{"case_count",cases.size()},{"f5_count",cases.size()},
            {"successful_f9_count",cases.size()*2},{"refused_f9_count",cases.size()*3},{"original_map",true},{"original_model",true},
            {"model_sha256",context.model->provenance().sha256},{"rule",10},{"schema",26},{"revision",27},
            {"native_os_input",false},{"human_acceptance","NOT_RUN"},{"scripted_queued_sdl",true},
            {"world_mutation_adapter_used",false},{"earned_boundary_preconditions_checked",true},
            {"corpus_inputs_unchanged",true},{"optional_visual_profiles",!selected.resources.empty()}};
        if (!selected.captures.empty()) write(selected.captures/"original-recovery-view.observed.json",report.dump(2)+"\n");
        std::cout<<report.dump()<<'\n';return 0;
    } catch (const std::exception& error) {std::cerr<<"Original Recovery View FAIL: "<<error.what()<<'\n';return 1;}
}
