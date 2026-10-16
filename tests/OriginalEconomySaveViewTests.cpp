#include "EconomyStartViewFixture.h"
#include "OriginalEconomySaveFixture.h"

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
namespace fixture=oe::testing::economy;
namespace fs=std::filesystem;
using fixture::check;
const fs::path map_relative="Cities/A.map";

struct Sdl {
    SDL_Window* window=nullptr;
    SDL_Renderer* renderer=nullptr;
    Sdl() {
        check(SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy") &&
            SDL_SetHint(SDL_HINT_RENDER_DRIVER,"software") && SDL_Init(SDL_INIT_VIDEO),
            "Rule7 save view SDL initialization failed");
        check(SDL_CreateWindowAndRenderer("Rule7 persistence-only fixture",1100,700,
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
        !actual.migrated_from_schema1,"Rule7 full SaveDocument identity changed");
}
void saved_key(oe::SandboxView& sandbox) {
    const auto old=sandbox.save_generation();key(sandbox,SDLK_F5);
    if (sandbox.save_generation()!=old+1)
        throw std::runtime_error("Actual Rule7 F5 failed: "+sandbox.last_message());
}
void load_rejected(oe::SandboxView& sandbox,const sim::WorldSnapshot& live) {
    const auto io=sandbox.io_generation();key(sandbox,SDLK_F9);
    check(sandbox.io_generation()==io+1 && sandbox.last_message().starts_with("Load failed:") &&
        sandbox.world().snapshot()==live,"Invalid actual F9 changed the active Rule7 World");
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
    saved_key(*sandbox);same_identity(document,save::read_save(target));
    check(bytes(target)==before && bytes(source)==before,
        "Separate OS process changed canonical Rule7 save bytes");
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

    // No runtime advances are granted by the persistence-only SDL adapter.
    for (const auto value:{SDLK_SPACE,SDLK_PERIOD,SDLK_EQUALS,SDLK_MINUS,SDLK_R}) key(*sandbox,value);
    for (unsigned frame=0;frame<120;++frame) sandbox->update(0.25,{});
    check(sandbox->paused() && sandbox->world().snapshot()==original,
        "Persistence-only controls or updates advanced Rule7 runtime");
    bool tick_refused=false;
    try { sandbox->tick_once(); } catch (const std::logic_error&) { tick_refused=true; }
    check(tick_refused && sandbox->world().snapshot()==original,
        "Rule7 production tick guard was bypassed by the SDL fixture");
    check(sandbox->render(),"Rule7 empty-container fallback render failed");

    paid(*sandbox,sim::CommandType::PlaceRoad,{109,103});
    const auto changed=sandbox->world().snapshot();
    check(changed!=original,"F9 fixture has no intervening paid mutation");
    write(target,"{ malformed Rule7 save");
    const auto malformed=bytes(target);load_rejected(*sandbox,changed);
    check(bytes(target)==malformed,"Rejected malformed F9 overwrote its input");
    write(target,canonical_save);
    write(model_path,model_before+"\n");load_rejected(*sandbox,changed);
    check(bytes(target)==canonical_save,"Model-mismatch F9 changed the saved file");
    write(model_path,model_before);
    write(data/map_relative,source_map_before+"\n");load_rejected(*sandbox,changed);
    check(bytes(target)==canonical_save,"Map-mismatch F9 changed the saved file");
    write(data/map_relative,source_map_before);
    const auto io=sandbox->io_generation();key(*sandbox,SDLK_F9);
    if (sandbox->io_generation()!=io+1 || !sandbox->last_message().starts_with("Loaded tick"))
        throw std::runtime_error("Actual Rule7 F9 failed: "+sandbox->last_message());
    check(sandbox->paused() && sandbox->world().snapshot()==original &&
        sandbox->world().map_permissions()->canonical_state()==canonical_policy &&
        bytes(target)==canonical_save,"Actual F9 changed full Rule7 state, policy or bytes");
    same_identity(sandbox->capture_save_document(),document);
    saved_key(*sandbox);check(bytes(target)==canonical_save,"F5 after actual F9 changed canonical save bytes");
    sandbox->shutdown();

    bool normal_refused=false;
    try { oe::SandboxView normal(session(data),false,sim::RulesProfile::CityV16,7); }
    catch (const std::invalid_argument&) { normal_refused=true; }
    check(normal_refused,"Normal sandbox gained access to unfinished Rule7 gameplay");
    oe::SandboxView old(session(data),false,sim::RulesProfile::CityV16,6);
    bool initial_refused=false;
    try { old.configure_save(data,map_relative,target,document); }
    catch (const std::invalid_argument&) { initial_refused=true; }
    check(initial_refused,"Normal Rule6 view admitted a Rule7 initial save");
    old.configure_save(data,map_relative,target);
    old.initialize(sdl.window,sdl.renderer);
    const auto old_world=old.world().snapshot();load_rejected(old,old_world);
    check(bytes(target)==canonical_save,"Normal Rule6 F9 changed a refused Rule7 save");
    old.shutdown();

    const auto output=root/"saves/process.json";
    const auto command=shell_quote(executable.string())+" --resume "+shell_quote(data.string())+" "+
        shell_quote(target.string())+" "+shell_quote(output.string());
    check(std::system(command.c_str())==0,"Separate OS-process Rule7 SDL restore failed");
    same_identity(document,save::read_save(output));
    check(bytes(output)==canonical_save && bytes(target)==canonical_save &&
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
        std::cout<<"Rule7 schema23: actual SDL F5/F9, atomic refusals, paused runtime and separate OS-process canonical roundtrip PASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr<<"Rule7 save view FAIL: "<<error.what()<<'\n';return 1;
    }
}
