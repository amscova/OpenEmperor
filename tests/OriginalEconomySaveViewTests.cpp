#include "EconomyStartViewFixture.h"
#include "OriginalEconomySaveFixture.h"
#include "core/PerformanceDiagnostics.h"
#include "simulation/original/OriginalRulesModel.h"
#include "simulation/original/OriginalScheduler.h"

#include <cstdlib>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>

namespace {
namespace oe=openemperor;
namespace sim=oe::simulation;
namespace maps=oe::maps;
namespace save=oe::persistence;
namespace original=oe::original;
namespace fixture=oe::testing::economy;
namespace fs=std::filesystem;
using fixture::check;
const fs::path map_relative="Cities/A.map";
constexpr std::uint64_t resume_steps=127;

struct Sdl {
    SDL_Window* window=nullptr;
    SDL_Renderer* renderer=nullptr;
    Sdl() {
        check(SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy") &&
            SDL_SetHint(SDL_HINT_RENDER_DRIVER,"software") && SDL_Init(SDL_INIT_VIDEO),
            "Rule7 save view SDL initialization failed");
        check(SDL_CreateWindowAndRenderer("Rule7 scheduler-only development fixture",1100,700,
            SDL_WINDOW_HIDDEN,&window,&renderer),"Rule7 save view SDL window failed");
    }
    ~Sdl() { SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit(); }
};

std::string bytes(const fs::path& path) {
    std::ifstream in(path,std::ios::binary);
    check(in.good(),"Rule7 test byte input is missing");
    return {std::istreambuf_iterator<char>(in),std::istreambuf_iterator<char>()};
}
void write(const fs::path& path,std::string_view value) {
    std::ofstream out(path,std::ios::binary|std::ios::trunc);
    out.write(value.data(),static_cast<std::streamsize>(value.size()));
    check(out.good(),"Rule7 test byte output failed");
}
maps::StoredMapSession session(const fs::path& data) {
    return maps::load_stored_map_session(data,map_relative,
        maps::FootprintPolicy::EdgeByte4x4Preview,maps::StoredGraphicsProfile::Slot8);
}
std::unique_ptr<oe::SandboxView> view(const fs::path& data,const fs::path& target,
        Sdl& sdl,std::optional<save::SaveDocument> initial=std::nullopt) {
    auto result=oe::SandboxView::create_original_economy_save_fixture(session(data));
    result->configure_save(data,map_relative,target,std::move(initial));
    result->initialize(sdl.window,sdl.renderer);
    check(result->world().original_economy_enabled() && result->paused() &&
        result->world().map_permissions() && result->world().map_permissions()->policy_version()==3,
        "Explicit Rule7 save fixture did not retain paused complete policy3");
    const auto model=result->world().original_rules_model();
    const auto& source=result->world().original_economy_state()->model_source;
    check(model && source && source->sha256==model->provenance().sha256 &&
        source->byte_count==model->provenance().byte_count && source->difficulty==model->difficulty(),
        "Rule7 SDL session did not bind its immutable prepared model to saved source identity");
    return result;
}
void key(oe::SandboxView& sandbox,SDL_Keycode value) {
    SDL_Event event{};event.type=SDL_EVENT_KEY_DOWN;event.key.key=value;
    bool running=true;sandbox.handle_event(event,running);
    check(running,"Save fixture key unexpectedly exited");
}
void paid(oe::SandboxView& sandbox,sim::CommandType type,sim::Cell cell) {
    const auto old=sandbox.world().snapshot();
    const auto result=sandbox.execute({type,cell});
    if (!result.accepted || !result.changed)
        throw std::runtime_error("Rule7 paid view fixture: "+result.reason);
    check(sandbox.world().treasury()==old.treasury-sandbox.world().construction_cost(type) &&
        sandbox.world().command_sequence()==old.command_sequence+1,
        "Rule7 persistence fixture did not pay its normal construction cost");
}
void same_identity(const save::SaveDocument& expected,const save::SaveDocument& actual) {
    check(actual.source_schema_version==23 && actual.world==expected.world &&
        actual.map_relative==expected.map_relative && actual.map_sha256==expected.map_sha256 &&
        actual.buildable_sha256==expected.buildable_sha256 &&
        actual.map_permissions_policy_version==3 &&
        actual.map_permissions_sha256==expected.map_permissions_sha256 &&
        !actual.migrated_from_schema1 && !actual.prepared_original_rules_model,
        "Rule7 full SaveDocument identity changed or deserialized a runtime model");
}
void saved_key(oe::SandboxView& sandbox) {
    const auto model=sandbox.world().original_rules_model();
    const auto old=sandbox.save_generation();key(sandbox,SDLK_F5);
    if (sandbox.save_generation()!=old+1)
        throw std::runtime_error("Actual Rule7 F5 failed: "+sandbox.last_message());
    check(sandbox.world().original_rules_model()==model &&
        sandbox.capture_save_document().prepared_original_rules_model==model,
        "Actual Rule7 F5 replaced or lost the session model dependency");
}
void loaded_key(oe::SandboxView& sandbox) {
    const auto previous=sandbox.world().original_rules_model();
    const auto io=sandbox.io_generation();key(sandbox,SDLK_F9);
    if (sandbox.io_generation()!=io+1 || !sandbox.last_message().starts_with("Loaded tick"))
        throw std::runtime_error("Actual Rule7 F9 failed: "+sandbox.last_message());
    check(sandbox.paused(),"Actual Rule7 F9 did not pause and reset the clock");
    const auto current=sandbox.world().original_rules_model();
    check(previous && current && current!=previous &&
        current->provenance().sha256==previous->provenance().sha256 &&
        current->difficulty()==previous->difficulty() &&
        current->building_value(original::ModelBuilding::ClayPit,original::BuildingField::Cost)==
            previous->building_value(original::ModelBuilding::ClayPit,original::BuildingField::Cost) &&
        current->house_rules().value(original::ModelHouse::Shelter,original::HouseField::Capacity)==
            previous->house_rules().value(original::ModelHouse::Shelter,original::HouseField::Capacity),
        "Actual Rule7 F9 did not prepare an independent candidate with identical typed model data");
}
void time_only(const sim::WorldSnapshot& before,const sim::WorldSnapshot& after,
        std::uint64_t elapsed) {
    check(after.ticks==before.ticks+elapsed,"Rule7 fixed-step delivery changed");
    auto normalized=after;normalized.ticks=before.ticks;
    check(normalized==before,"Rule7 scheduler changed non-time World authority");
}
void steps(oe::SandboxView& sandbox,std::uint64_t count) {
    check(sandbox.paused(),"SDL Step fixture must start paused");
    const auto before=sandbox.world().snapshot();
    const auto model=sandbox.world().original_rules_model();
    for (std::uint64_t i=0;i<count;++i) key(sandbox,SDLK_PERIOD);
    time_only(before,sandbox.world().snapshot(),count);
    check(sandbox.world().original_rules_model()==model,"Rule7 SDL Steps replaced their prepared model");
}
void clock_controls(oe::SandboxView& sandbox) {
    const auto initial=sandbox.world().snapshot();
    const auto model=sandbox.world().original_rules_model();
    for (unsigned frame=0;frame<120;++frame) sandbox.update(0.25,{});
    check(sandbox.paused() && sandbox.world().snapshot()==initial,
        "Paused Rule7 scheduler advanced during frame updates");
    steps(sandbox,1);
    key(sandbox,SDLK_SPACE);check(!sandbox.paused(),"Rule7 SDL Continue remained disabled");
    sandbox.update(0.025,{});check(sandbox.world().ticks()==initial.ticks+1,"Rule7 half-frame advanced a full tick");
    sandbox.update(0.025,{});check(sandbox.world().ticks()==initial.ticks+2,"Rule7 partitioned 20Hz frame delivery drifted");
    key(sandbox,SDLK_PERIOD);check(sandbox.world().ticks()==initial.ticks+2,"Running SDL Step advanced Rule7");
    key(sandbox,SDLK_EQUALS);sandbox.update(0.05,{});
    check(sandbox.world().ticks()==initial.ticks+4,"Rule7 2x speed delivery changed");
    key(sandbox,SDLK_EQUALS);sandbox.update(0.05,{});
    check(sandbox.world().ticks()==initial.ticks+8,"Rule7 4x speed delivery changed");
    sandbox.update(100.0,{});
    check(sandbox.world().ticks()==initial.ticks+16,"Rule7 frame catch-up exceeded the fixed eight-tick limit");
    key(sandbox,SDLK_MINUS);sandbox.update(0.05,{});
    check(sandbox.world().ticks()==initial.ticks+18,"Rule7 descending 2x speed delivery changed");
    key(sandbox,SDLK_MINUS);sandbox.update(0.05,{});
    check(sandbox.world().ticks()==initial.ticks+19,"Rule7 1x speed delivery changed");
    key(sandbox,SDLK_SPACE);check(sandbox.paused(),"Rule7 SDL Pause remained disabled");
    const auto paused=sandbox.world().snapshot();key(sandbox,SDLK_R);
    check(sandbox.paused() && sandbox.world().snapshot()==paused,"Scheduler-only Reset changed World state");
    sandbox.tick_once();time_only(initial,sandbox.world().snapshot(),20);
    check(sandbox.world().original_rules_model()==model,
        "Rule7 frames or fixed ticks replaced the prepared session model");

    // Actual F9 must discard both running status and a partial frame accumulator.
    loaded_key(sandbox);check(sandbox.world().snapshot()==initial,"Rule7 F9 did not restore its saved clock tick");
    key(sandbox,SDLK_SPACE);sandbox.update(0.025,{});
    loaded_key(sandbox);sandbox.update(0.025,{});
    check(sandbox.paused() && sandbox.world().snapshot()==initial,"Paused F9 retained pending fractional time");
    key(sandbox,SDLK_SPACE);sandbox.update(0.025,{});
    check(sandbox.world().snapshot()==initial,"F9 did not clear the pre-load fractional clock accumulator");
    sandbox.update(0.025,{});key(sandbox,SDLK_SPACE);
    time_only(initial,sandbox.world().snapshot(),1);
}
void scheduler_checkpoint(oe::SandboxView& sandbox) {
    const auto before=sandbox.world().snapshot();
    const auto dispatch=original::next_scheduler_dispatch(before.ticks);
    saved_key(sandbox);
    const auto document=save::read_save(sandbox.save_path());
    const auto canonical=bytes(sandbox.save_path());
    same_identity(sandbox.capture_save_document(),document);
    steps(sandbox,1);const auto next=sandbox.world().snapshot();
    loaded_key(sandbox);
    check(sandbox.world().snapshot()==before && original::next_scheduler_dispatch(
        sandbox.world().ticks())==dispatch,"Rule7 F9 changed the next scheduler dispatch at a boundary");
    steps(sandbox,1);
    check(sandbox.world().snapshot()==next,"Rule7 Step replay after F9 changed complete World state");
    loaded_key(sandbox);saved_key(sandbox);
    check(sandbox.world().snapshot()==before && bytes(sandbox.save_path())==canonical,
        "Before/at/after boundary F5/F9 changed canonical Rule7 save bytes");
}
void scheduler_boundaries(oe::SandboxView& sandbox) {
    for (const std::uint64_t boundary:{51ULL,816ULL,9792ULL}) {
        check(sandbox.world().ticks()<boundary,"Scheduler boundary fixture is out of order");
        steps(sandbox,boundary-1-sandbox.world().ticks());
        scheduler_checkpoint(sandbox);
        steps(sandbox,1);scheduler_checkpoint(sandbox);
        steps(sandbox,1);scheduler_checkpoint(sandbox);
        const auto position=original::scheduler_position(boundary);
        check(position.phase==0 && (boundary==51 ?
            (position.step==1 && position.month==0 && position.year==0):position.step==0) &&
            (boundary!=816 || (position.month==1 && position.year==0)) &&
            (boundary!=9792 || (position.month==0 && position.year==1)),
            "Rule7 authored cycle/month/year rollover changed");
    }
}
void load_rejected(oe::SandboxView& sandbox,const sim::WorldSnapshot& live) {
    const auto model=sandbox.world().original_rules_model();
    const auto io=sandbox.io_generation();key(sandbox,SDLK_F9);
    check(sandbox.io_generation()==io+1 && sandbox.last_message().starts_with("Load failed:") &&
        sandbox.world().snapshot()==live && sandbox.world().original_rules_model()==model,
        "Invalid actual F9 changed the active Rule7 World or model binding");
}
void save_rejected(oe::SandboxView& sandbox,const std::string& saved) {
    const auto state=sandbox.world().snapshot();
    const auto model=sandbox.world().original_rules_model();
    const auto saved_generation=sandbox.save_generation();
    const auto io=sandbox.io_generation();key(sandbox,SDLK_F5);
    check(sandbox.io_generation()==io+1 && sandbox.save_generation()==saved_generation &&
        sandbox.last_message().starts_with("Save failed:") && sandbox.world().snapshot()==state &&
        sandbox.world().original_rules_model()==model && bytes(sandbox.save_path())==saved,
        "Source-invalid actual F5 changed World/model state or destroyed the saved file");
}
void missing_model_during_runtime(oe::SandboxView& sandbox,const fs::path& data,
        const fs::path& model_path,Sdl& sdl) {
    const auto saved=bytes(sandbox.save_path());
    const auto before=sandbox.world().snapshot();
    const auto model=sandbox.world().original_rules_model();
    const auto backup=model_path.parent_path()/"authored-model-temporarily-unavailable.txt";
    fs::rename(model_path,backup);
    try {
        bool create_refused=false;
        try { auto failed=view(data,sandbox.save_path().parent_path()/"failed-create.json",sdl); }
        catch (const std::exception&) { create_refused=true; }
        check(create_refused && sandbox.world().snapshot()==before &&
            sandbox.world().original_rules_model()==model && bytes(sandbox.save_path())==saved,
            "Source-missing session preparation affected an existing Rule7 session");
        namespace perf=oe::performance;
        perf::set_enabled(true);perf::reset();
        for (unsigned frame=0;frame<120;++frame) sandbox.update(0.05,{});
        steps(sandbox,2);
        check(sandbox.render(),"Prepared model did not survive a source-free render");
        for (const auto counter:{perf::Counter::FileReads,perf::Counter::FileWrites,
            perf::Counter::AssetDecodes,perf::Counter::TextureUploads})
            check(perf::counter(counter)==0,"Rule7 frames/ticks/render performed source or asset work");
        perf::set_enabled(false);
        time_only(before,sandbox.world().snapshot(),2);
        check(sandbox.world().original_rules_model()==model,
            "Source-free frames or ticks replaced the prepared model");
        save_rejected(sandbox,saved);load_rejected(sandbox,sandbox.world().snapshot());
        check(bytes(sandbox.save_path())==saved,"Source-missing F9 overwrote its save");
    } catch (...) {
        oe::performance::set_enabled(false);fs::rename(backup,model_path);throw;
    }
    fs::rename(backup,model_path);
    loaded_key(sandbox);
    check(sandbox.world().snapshot()==before && bytes(sandbox.save_path())==saved,
        "Source-restored F9 retry failed to restore exact saved World authority");
}
void separate_model_sessions(oe::SandboxView& first,const fs::path& first_data,Sdl& sdl) {
    const auto first_world=first.world().snapshot();
    const auto first_model=first.world().original_rules_model();
    const auto first_target=first.save_path();
    const auto first_saved=bytes(first_target);
    fixture::Temp other;
    const auto root=fs::canonical(other.root);
    const auto data=fs::canonical(root/"data");
    oe_save_fixture::write_model(data,1);
    auto second=view(data,root/"saves/independent-source.json",sdl);
    const auto second_model=second->world().original_rules_model();
    check(first_model && second_model && first_model!=second_model &&
        first_model->provenance().sha256!=second_model->provenance().sha256 &&
        first_model->provenance().source_path!=second_model->provenance().source_path &&
        first_model->house_rules().row_capacity(original::ModelHouse::Shelter)==0 &&
        second_model->house_rules().row_capacity(original::ModelHouse::Shelter)==37,
        "Independent Rule7 sessions shared or confused differently authored prepared model data");
    steps(*second,3);saved_key(*second);
    const auto second_saved=bytes(second->save_path());
    check(second->world().total_population_capacity()==0 &&
        first.world().snapshot()==first_world && first.world().original_rules_model()==first_model,
        "Second model session changed the first World or granted active housing capacity");
    first.configure_save(first_data,map_relative,second->save_path());
    load_rejected(first,first_world);
    check(bytes(second->save_path())==second_saved && bytes(first_target)==first_saved,
        "Cross-source F9 changed either independent session save");
    first.configure_save(first_data,map_relative,first_target);
    second->shutdown();
    check(first.world().snapshot()==first_world && first.world().original_rules_model()==first_model &&
        first_model->house_rules().row_capacity(original::ModelHouse::Shelter)==0 &&
        second_model->house_rules().row_capacity(original::ModelHouse::Shelter)==37,
        "Independent model lifetime or source failure altered an existing session");
}
std::string shell_quote(std::string_view value) {
    std::string result="'";
    for (const char c:value) result+=c=='\'' ? "'\"'\"'":std::string(1,c);
    return result+"'";
}
void process_resume(const fs::path& data,const fs::path& source,const fs::path& target,Sdl& sdl) {
    const auto document=save::read_save(source);
    const auto before=bytes(source);
    auto sandbox=view(data,target,sdl,document);
    check(sandbox->world().snapshot()==document.world,
        "Separate OS process did not reconstruct the full Rule7 snapshot");
    steps(*sandbox,resume_steps);saved_key(*sandbox);
    same_identity(sandbox->capture_save_document(),save::read_save(target));
    check(bytes(source)==before,"Separate OS process changed its Rule7 source save");
    sandbox->shutdown();
}
void run(const fs::path& executable,Sdl& sdl) {
    fixture::Temp temp;
    const auto root=fs::canonical(temp.root);
    const auto data=fs::canonical(root/"data");
    oe_save_fixture::write_model(data);
    const auto model_path=data/"Model/EmperorBuildingModels.txt";
    const auto model_before=bytes(model_path);
    const auto source_map_before=bytes(data/map_relative);
    const auto target=root/"saves/rule7.json";
    auto sandbox=view(data,target,sdl);
    paid(*sandbox,sim::CommandType::PlaceHousehold,{106,101});
    const auto inspected=sandbox->inspection_lines();
    check(std::any_of(inspected.begin(),inspected.end(),[](const auto& line) {
        return line.find("Developer empty container")!=std::string::npos;
    }),"Rule7 House inspector reused legacy level authority");
    check(sandbox->render(),"Rule7 selected empty-container fallback render failed");
    paid(*sandbox,sim::CommandType::PlaceRoad,{106,103});
    paid(*sandbox,sim::CommandType::PlaceRoad,{107,103});
    paid(*sandbox,sim::CommandType::PlaceRoad,{108,103});
    check(sandbox->world().original_economy_state() &&
        sandbox->world().original_economy_state()->houses.size()==1 &&
        sandbox->world().buildings().size()==1 && sandbox->world().couriers().empty(),
        "Rule7 paid empty OriginalHouse gained legacy actors or lost independent authority");
    const auto original=sandbox->world().snapshot();
    const auto canonical_policy=sandbox->world().map_permissions()->canonical_state();
    saved_key(*sandbox);
    const auto document=save::read_save(target);
    same_identity(sandbox->capture_save_document(),document);
    const auto canonical_save=bytes(target);
    check(document.world==original,"Actual F5 changed the full Rule7 snapshot");

    missing_model_during_runtime(*sandbox,data,model_path,sdl);
    separate_model_sessions(*sandbox,data,sdl);
    clock_controls(*sandbox);
    check(sandbox->render(),"Rule7 empty-container fallback render failed");

    paid(*sandbox,sim::CommandType::PlaceRoad,{109,103});
    const auto changed=sandbox->world().snapshot();
    check(changed!=original,"F9 fixture has no intervening paid mutation");
    write(target,"{ malformed Rule7 save");
    const auto malformed=bytes(target);load_rejected(*sandbox,changed);
    check(bytes(target)==malformed,"Rejected malformed F9 overwrote its input");
    write(target,canonical_save);
    write(model_path,model_before+"\n");save_rejected(*sandbox,canonical_save);
    load_rejected(*sandbox,changed);
    check(bytes(target)==canonical_save,"Model-mismatch F9 changed the saved file");
    write(model_path,model_before);
    write(data/map_relative,source_map_before+"\n");load_rejected(*sandbox,changed);
    check(bytes(target)==canonical_save,"Map-mismatch F9 changed the saved file");
    write(data/map_relative,source_map_before);
    loaded_key(*sandbox);
    check(sandbox->paused() && sandbox->world().snapshot()==original &&
        sandbox->world().map_permissions()->canonical_state()==canonical_policy &&
        bytes(target)==canonical_save,"Actual F9 changed full Rule7 state, policy or bytes");
    same_identity(sandbox->capture_save_document(),document);
    saved_key(*sandbox);check(bytes(target)==canonical_save,"F5 after actual F9 changed canonical save bytes");
    scheduler_boundaries(*sandbox);
    const auto resume_document=save::read_save(target);
    const auto resume_bytes=bytes(target);
    const auto expected=root/"saves/expected-process.json";
    steps(*sandbox,resume_steps);
    sandbox->configure_save(data,map_relative,expected);saved_key(*sandbox);
    const auto expected_document=save::read_save(expected);
    sandbox->shutdown();

    bool normal_refused=false;
    try { oe::SandboxView normal(session(data),false,sim::RulesProfile::CityV16,7); }
    catch (const std::invalid_argument&) { normal_refused=true; }
    check(normal_refused,"Normal sandbox gained access to unfinished Rule7 gameplay");
    oe::SandboxView old(session(data),false,sim::RulesProfile::CityV16,6);
    bool initial_refused=false;
    try { old.configure_save(data,map_relative,target,resume_document); }
    catch (const std::invalid_argument&) { initial_refused=true; }
    check(initial_refused,"Normal Rule6 view admitted a Rule7 initial save");
    old.configure_save(data,map_relative,target);
    old.initialize(sdl.window,sdl.renderer);
    const auto old_world=old.world().snapshot();load_rejected(old,old_world);
    check(bytes(target)==resume_bytes,"Normal Rule6 F9 changed a refused Rule7 save");
    old.shutdown();

    const auto output=root/"saves/process.json";
    const auto command=shell_quote(executable.string())+" --resume "+shell_quote(data.string())+" "+
        shell_quote(target.string())+" "+shell_quote(output.string());
    check(std::system(command.c_str())==0,"Separate OS-process Rule7 SDL restore failed");
    same_identity(expected_document,save::read_save(output));
    check(bytes(output)==bytes(expected) && bytes(target)==resume_bytes &&
        bytes(model_path)==model_before && bytes(data/map_relative)==source_map_before,
        "Rule7 OS-process roundtrip changed saved bytes or authored source inputs");
}
}

int main(int argc,char** argv) {
    try {
        Sdl sdl;
        if (argc==5 && std::string_view(argv[1])=="--resume")
            process_resume(argv[2],argv[3],argv[4],sdl);
        else {
            check(argc==1,"Unknown Rule7 save view test arguments");
            run(fs::absolute(argv[0]),sdl);
        }
        std::cout<<"Rule7 schema23: SDL pause/Step/20Hz, cycle/month/year F5/F9, time-only state and separate OS-process replay PASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr<<"Rule7 save view FAIL: "<<error.what()<<'\n';return 1;
    }
}
