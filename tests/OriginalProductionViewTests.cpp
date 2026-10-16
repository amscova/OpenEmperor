#include "EconomyStartViewFixture.h"
#include "OriginalInfluenceFixture.h"
#include "core/PerformanceDiagnostics.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <iostream>

namespace {
namespace oe=openemperor;
namespace sim=oe::simulation;
namespace orig=oe::original;
namespace f=oe_influence_fixture;
namespace fs=std::filesystem;
using oe::testing::economy::check;
void key(oe::SandboxView& view,SDL_Keycode code) {
    SDL_Event event{};event.type=SDL_EVENT_KEY_DOWN;event.key.key=code;
    bool running=true;view.handle_event(event,running);
}
void click(oe::SandboxView& view,sim::Cell cell) {
    double sx=0,sy=0;unsigned count=0;
    for(int y=80;y<660;y+=4)for(int x=8;x<1090;x+=4)
        if(view.pick({double(x),double(y)})==cell){sx+=x;sy+=y;++count;}
    check(count>0,"Production cell not visible");
    SDL_Event event{};event.type=SDL_EVENT_MOUSE_MOTION;
    event.motion.x=static_cast<float>(sx/count);event.motion.y=static_cast<float>(sy/count);
    bool running=true;view.handle_event(event,running);view.update(0);
    event={};event.type=SDL_EVENT_MOUSE_BUTTON_DOWN;event.button.button=SDL_BUTTON_LEFT;
    event.button.x=static_cast<float>(sx/count);event.button.y=static_cast<float>(sy/count);
    view.handle_event(event,running);event.type=SDL_EVENT_MOUSE_BUTTON_UP;view.handle_event(event,running);
}
}
int main(int argc,char** argv) {
    SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;
    try {
        check(argc<=2,"Expected at most capture output path");
        check(SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy") && SDL_SetHint(SDL_HINT_RENDER_DRIVER,"software") &&
            SDL_Init(SDL_INIT_VIDEO),"Production SDL init");
        check(SDL_CreateWindowAndRenderer("Original production",1100,700,SDL_WINDOW_HIDDEN,
            &window,&renderer),"Production SDL window");
        oe::testing::economy::Temp temp;const auto root=fs::canonical(temp.root),data=root/"data";
        auto model=f::authored_model_text();std::string prefix="1: Shelter,{,";
        for(int i=0;i<17;++i)prefix+="0,";
        const auto at=model.find(prefix+"0,");check(at!=std::string::npos,"Authored capacity field missing");
        model.replace(at,prefix.size()+2,prefix+"37,");
        fs::create_directories(data/"Model");f::write_text(data/"Model/EmperorBuildingModels.txt",model);
        const auto session=[&]{return oe::maps::load_stored_map_session(data,"Cities/A.map",
            oe::maps::FootprintPolicy::EdgeByte4x4Preview,oe::maps::StoredGraphicsProfile::Slot8);};
        const auto target=root/"saves/production.json";
        auto view=oe::SandboxView::create_original_production_save_fixture(session());
        view->configure_save(data,"Cities/A.map",target);view->initialize(window,renderer);
        check(view->world().rule_version()==10 && view->world().treasury()==12000 && view->paused(),"Production identity");
        check(std::string(SDL_GetWindowTitle(window)).find("save schema 26")!=std::string::npos,"Production title");
        check(!view->recovery_safe_point(),"Development production exposed recovery");
        const auto buy=[&](sim::CommandType type,sim::Cell cell) {
            const auto result=view->execute({type,cell});check(result.accepted && result.changed,result.reason.c_str());
        };
        for(int x=104;x<=120;++x)buy(sim::CommandType::PlaceRoad,{x,103});
        for(int x:{106,109,112,115})buy(sim::CommandType::PlaceHousehold,{x,101});
        buy(sim::CommandType::PlaceClaySource,{106,104});
        buy(sim::CommandType::PlacePottery,{110,104});
        buy(sim::CommandType::PlaceWarehouse,{114,104});
        check(view->world().total_population()==0 && view->world().original_goods_balance(orig::Goods::Ceramics).stock==0,
            "Construction injected population or goods");
        bool travelling=false;
        for(int tick=0;tick<2000 && !travelling;++tick) {
            view->tick_once();
            travelling=std::any_of(view->world().couriers().begin(),view->world().couriers().end(),[](const auto& c){
                return c.role==sim::CourierRole::Pottery && c.phase==sim::CourierPhase::ToWarehouse && c.cargo>0;});
        }
        check(travelling && view->world().total_population()>0,"Real staffed Pottery courier never departed");
        const auto saved=view->world().snapshot();key(*view,SDLK_F5);
        check(oe::persistence::read_save(target).source_schema_version==26,"Actual production F5 schema");
        for(int i=0;i<100;++i)view->tick_once();
        key(*view,SDLK_F9);check(view->world().snapshot()==saved && view->paused(),"Actual production F9 midcourier replay");
        for(int i=0;i<800;++i)view->tick_once();
        const auto balance=view->world().original_goods_balance(orig::Goods::Ceramics);
        check(balance.supported && balance.conserved && balance.totals.produced>0 && balance.stock>0,
            "Actual warehouse arrival did not produce typed Ceramics");
        key(*view,SDLK_0);check(view->tool()==0,"Market keyboard selection unavailable");
        click(*view,{118,104});
        check(view->world().object_at({118,104})==sim::Object::Market,"Paid Market mouse placement failed");
        bool typed_trip=false;
        for(int i=0;i<5000 && !typed_trip;++i) {
            view->tick_once();
            for(const auto& c:view->world().couriers()) if(c.role==sim::CourierRole::MarketPotteryDistribution &&
                view->world().original_courier_cargo(c.id)>0 && c.edge_progress>0) {
                check(c.cargo==0 && c.reserved==0,"Typed delivery mirrored cargo in legacy movement fields");
                const auto rows=view->walker_diagnostics();
                const auto row=std::find_if(rows.begin(),rows.end(),[&](const auto& value){return value.id==c.id;});
                check(row!=rows.end() && row->cargo==view->world().original_courier_cargo(c.id),
                    "Rendered walker diagnostics hid actual typed cargo");
                typed_trip=true;break;
            }
        }
        check(typed_trip,"No actual typed Market-to-House courier departed");
        const auto typed_saved=view->world().snapshot();key(*view,SDLK_F5);
        for(int i=0;i<80;++i)view->tick_once();
        key(*view,SDLK_F9);check(view->world().snapshot()==typed_saved,"Actual typed midcourier F5/F9 diverged");
        bool delivered=false;
        for(int i=0;i<2500 && !delivered;++i) {
            view->tick_once();
            for(const auto& house:view->world().original_economy_state()->houses) delivered|=house.house.ceramics>0;
        }
        check(delivered,"Actual Market arrival never updated the authoritative House stock");
        key(*view,SDLK_F5);const auto current=f::read_text(target);
        const auto before=view->world().snapshot();
        const auto model_before=view->world().original_rules_model();
        auto bad=nlohmann::json::parse(current);
        bad["world"]["original_production"]["development_revision"]=sim::kOriginalProductionDevelopmentRevision-1;
        f::write_text(target,bad.dump());key(*view,SDLK_F9);
        check(view->world().snapshot()==before && view->world().original_rules_model()==model_before,
            "Bad development revision published through F9");
        f::write_text(target,current);key(*view,SDLK_F9);
        view->set_tool(5);click(*view,{106,101});
        oe::performance::set_enabled(true);oe::performance::reset();
        for(int i=0;i<100;++i){view->update(0);check(view->render(),"Production idle frame");}
        check(view->world().snapshot()==before,"Production frame changed World");
        for(const auto counter:{oe::performance::Counter::WorldCopies,oe::performance::Counter::WorldExecutes,
            oe::performance::Counter::BfsCalls,oe::performance::Counter::AssetDecodes,oe::performance::Counter::TextureUploads,
            oe::performance::Counter::FileReads,oe::performance::Counter::FileWrites})
            check(oe::performance::counter(counter)==0,"Production frame prepared or mutated state");
        oe::performance::set_enabled(false);
        if(argc==2){auto* frame=SDL_RenderReadPixels(renderer,nullptr);check(frame!=nullptr,"Capture pixels");
            const bool okay=SDL_SavePNG(frame,argv[1]);SDL_DestroySurface(frame);check(okay,"Capture output");}
        view->shutdown();view.reset();
        SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();
        std::cout<<"Original production SDL paid Market arrival/typed midcourier F5/F9/100 idle frames PASS\n";return 0;
    }catch(const std::exception& error){
        std::cerr<<error.what()<<'\n';SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();return 1;
    }
}
