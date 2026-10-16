#include "EconomyStartViewFixture.h"
#include "OriginalInfluenceFixture.h"
#include "core/PerformanceDiagnostics.h"
#include <algorithm>
#include <cstdlib>

namespace {
namespace oe=openemperor;
namespace sim=oe::simulation;
namespace perf=oe::performance;
namespace fs=std::filesystem;
using openemperor::testing::economy::check;
void key(oe::SandboxView& view,SDL_Keycode code) {
    SDL_Event event{};event.type=SDL_EVENT_KEY_DOWN;event.key.key=code;
    bool running=true;view.handle_event(event,running);
}
oe::scene::Point point_for(oe::SandboxView& view,sim::Cell cell) {
    double xsum=0,ysum=0;unsigned count=0;
    for (int y=80;y<660;y+=4) for(int x=8;x<1090;x+=4)
        if(view.pick({double(x),double(y)})==cell) { xsum+=x;ysum+=y;++count; }
    check(count>0,"Influence test cell is not visible");
    return {xsum/count,ysum/count};
}
void click(oe::SandboxView& view,sim::Cell cell) {
    const auto p=point_for(view,cell);
    SDL_Event event{};event.type=SDL_EVENT_MOUSE_MOTION;
    event.motion.x=static_cast<float>(p.x);event.motion.y=static_cast<float>(p.y);
    bool running=true;view.handle_event(event,running);view.update(0);
    event={};event.type=SDL_EVENT_MOUSE_BUTTON_DOWN;event.button.button=SDL_BUTTON_LEFT;
    event.button.x=static_cast<float>(p.x);event.button.y=static_cast<float>(p.y);
    view.handle_event(event,running);event.type=SDL_EVENT_MOUSE_BUTTON_UP;view.handle_event(event,running);
}
bool contains(const oe::SandboxView& view,const std::string& text) {
    const auto lines=view.inspection_lines();
    return std::any_of(lines.begin(),lines.end(),[&](const auto& line){return line.find(text)!=std::string::npos;});
}
}
int main(int argc,char** argv) {
    SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;
    try {
        check(argc<=2,"Expected at most a screenshot output path");
        check(SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy") && SDL_SetHint(SDL_HINT_RENDER_DRIVER,"software") &&
            SDL_Init(SDL_INIT_VIDEO),"Influence SDL initialization");
        check(SDL_CreateWindowAndRenderer("Well72 influence developer test",1100,700,SDL_WINDOW_HIDDEN,
            &window,&renderer),"Influence SDL window");
        oe::testing::economy::Temp temp;
        const auto root=fs::canonical(temp.root);
        const auto data=root/"data";
        oe_influence_fixture::write_model(data);
        const auto load_session=[&] {return oe::maps::load_stored_map_session(data,"Cities/A.map",
            oe::maps::FootprintPolicy::EdgeByte4x4Preview,oe::maps::StoredGraphicsProfile::Slot8);};
        auto view=oe::SandboxView::create_original_influence_save_fixture(load_session());
        const auto target=root/"saves/influence.json";
        view->configure_save(data,"Cities/A.map",target);view->initialize(window,renderer);
        check(view->world().rule_version()==8 && view->world().treasury()==2000 && view->paused(),"Influence opt-in identity");
        check(!view->recovery_safe_point(),"Influence fixture exposed recovery");
        check(view->execute({sim::CommandType::PlaceHousehold,{106,101}}).changed,"Paid view house");
        check(contains(*view,"Desirability: 0"),"House inspector lacks cached value");
        key(*view,SDLK_I);check(view->tool()==12,"Influence I shortcut");
        check(view->render(),"Influence pre-placement render");
        click(*view,{108,103});
        if(view->world().original_influence_state()->sources.size()!=1)
            throw std::runtime_error("Influence click purchase failed: "+view->last_message());
        check(view->world().treasury()==1860 && !view->budget_warning_pending(),"Influence source payment");
        check(contains(*view,"Well72 influence source") && contains(*view,"no water"),"Source inspector misstates role");
        check(!view->predicted_well_coverage(),"Source preview claimed water coverage");
        const auto source=view->world().original_influence_state()->sources.front().building_id;
        const auto house=*view->world().building_owner_at({106,101});
        check(view->world().household_desirability(house)>0 && !view->world().household_has_water(house),"Source influence leaked water or stayed empty");
        const auto saved=view->world().snapshot();
        key(*view,SDLK_F5);
        if(!fs::is_regular_file(target))throw std::runtime_error("Actual influence F5: "+view->last_message());
        check(oe::persistence::read_save(target).source_schema_version==24,"View wrote wrong schema");
        check(view->execute(sim::demolish_building(source)).changed,"View source removal");
        check(view->world().household_desirability(house)==0 && view->world().treasury()==1860,"Removal retained influence or refunded");
        key(*view,SDLK_F9);check(view->world().snapshot()==saved && view->paused(),"Actual influence F9 full restore");
        const auto before=view->world().snapshot();
        const auto builds=view->world().original_influence_cache_build_count();
        perf::set_enabled(true);perf::reset();
        for(int i=0;i<30;++i) {view->update(0);check(view->render(),"Influence idle render");}
        check(view->world().snapshot()==before && view->world().original_influence_cache_build_count()==builds,"Frame mutated or rebuilt influence");
        for(const auto counter:{perf::Counter::WorldCopies,perf::Counter::WorldExecutes,perf::Counter::BfsCalls,
            perf::Counter::AssetDecodes,perf::Counter::TextureUploads,perf::Counter::FileReads,perf::Counter::FileWrites})
            check(perf::counter(counter)==0,"Idle influence view did file/asset/World/navigation work");
        perf::set_enabled(false);
        view->set_tool(5);check(view->render(),"Influence selection frame");click(*view,{106,101});
        check(view->selected_building()==house && contains(*view,"Source #") && contains(*view,"Model SHA"),"House provenance inspection");
        check(view->render(),"Influence inspector render");
        if(argc==2) {
            SDL_Surface* frame=SDL_RenderReadPixels(renderer,nullptr);check(frame!=nullptr,"Influence capture");
            const bool okay=SDL_SavePNG(frame,argv[1]);SDL_DestroySurface(frame);check(okay,"Influence PNG output");
        }
        view->shutdown();view.reset();
        bool denied=false;try{oe::SandboxView normal(load_session(),false,sim::RulesProfile::CityV16,8);}
        catch(const std::invalid_argument&){denied=true;}
        check(denied,"Normal SandboxView exposed Rule8");
        SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();
        std::cout<<"Well72 developer SDL: input, paid source, neutral water, inspector, actual F5/F9 and idle counters PASS\n";
        return 0;
    }catch(const std::exception& error){
        std::cerr<<"Influence view FAIL: "<<error.what()<<'\n';
        SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();return 1;
    }
}
