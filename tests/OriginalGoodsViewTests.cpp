#include "EconomyStartViewFixture.h"
#include "OriginalInfluenceFixture.h"
#include "core/PerformanceDiagnostics.h"
#include <nlohmann/json.hpp>
#include <algorithm>

namespace {
namespace oe=openemperor;
namespace sim=oe::simulation;
namespace fs=std::filesystem;
namespace f=oe_influence_fixture;
namespace perf=oe::performance;
using oe::testing::economy::check;
void key(oe::SandboxView& view,SDL_Keycode code) {
    SDL_Event event{};event.type=SDL_EVENT_KEY_DOWN;event.key.key=code;
    bool running=true;view.handle_event(event,running);
}
void click(oe::SandboxView& view,sim::Cell cell) {
    double sx=0,sy=0;unsigned count=0;
    for(int y=80;y<660;y+=4)for(int x=8;x<1090;x+=4)
        if(view.pick({double(x),double(y)})==cell){sx+=x;sy+=y;++count;}
    check(count>0,"Goods fixture cell not visible");
    SDL_Event event{};event.type=SDL_EVENT_MOUSE_MOTION;
    event.motion.x=static_cast<float>(sx/count);event.motion.y=static_cast<float>(sy/count);
    bool running=true;view.handle_event(event,running);view.update(0);
    event={};event.type=SDL_EVENT_MOUSE_BUTTON_DOWN;event.button.button=SDL_BUTTON_LEFT;
    event.button.x=static_cast<float>(sx/count);event.button.y=static_cast<float>(sy/count);
    view.handle_event(event,running);event.type=SDL_EVENT_MOUSE_BUTTON_UP;view.handle_event(event,running);
}
void zero_goods(const sim::World& world) {
    check(world.original_goods_enabled() && world.original_goods_state().has_value(),"Missing Rule9 goods authority");
    check(*world.original_goods_state()==sim::OriginalGoodsState{},"Unexpected goods authority in live World");
    const auto balances=world.original_goods_balances();
    for(unsigned i=0;i<balances.size();++i) {
        const auto& b=balances[i];
        check(static_cast<unsigned>(b.good)==i && !b.supported && b.conserved &&
            b.totals==sim::OriginalGoodsTotals{} && b.stock==0 && b.cargo==0 &&
            b.source_reserved==0 && b.incoming_reserved==0,"Live goods are not unsupported/0");
        check(world.original_goods_balance(b.good)==b,"Goods balance APIs disagree");
    }
}
}
int main(int argc,char** argv) {
    SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;
    try {
        check(argc<=2,"Expected at most capture output path");
        check(SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy") && SDL_SetHint(SDL_HINT_RENDER_DRIVER,"software") &&
            SDL_Init(SDL_INIT_VIDEO),"Goods SDL init");
        check(SDL_CreateWindowAndRenderer("Goods ledger developer test",1100,700,SDL_WINDOW_HIDDEN,
            &window,&renderer),"Goods SDL window");
        oe::testing::economy::Temp temp;const auto root=fs::canonical(temp.root),data=root/"data";
        f::write_model(data);
        const auto session=[&]{return oe::maps::load_stored_map_session(data,"Cities/A.map",
            oe::maps::FootprintPolicy::EdgeByte4x4Preview,oe::maps::StoredGraphicsProfile::Slot8);};
        const auto target=root/"saves/goods.json",old_target=root/"saves/influence.json";
        auto view=oe::SandboxView::create_original_goods_save_fixture(session());
        view->configure_save(data,"Cities/A.map",target);view->initialize(window,renderer);
        check(view->world().rule_version()==9 && view->world().treasury()==2000 && view->paused(),"Rule9 opt-in identity");
        check(std::string(SDL_GetWindowTitle(window)).find("save schema 25")!=std::string::npos,"Rule9 title identity");
        check(!view->recovery_safe_point(),"Goods fixture exposed recovery");
        zero_goods(view->world());
        check(view->execute({sim::CommandType::PlaceHousehold,{106,101}}).changed,"Paid Rule9 house");
        key(*view,SDLK_I);check(view->tool()==12 && view->render(),"Rule9 influence input/render");
        click(*view,{108,103});
        check(view->world().original_influence_state()->sources.size()==1 && view->world().treasury()==1860,"Paid Rule9 source mouse placement");
        const auto source=view->world().original_influence_state()->sources.front().building_id;
        const auto house=*view->world().building_owner_at({106,101});
        check(view->world().household_desirability(house)>0 && !view->world().household_has_water(house),"Rule9 source lost influence or granted water");
        zero_goods(view->world());
        const auto saved=view->world().snapshot();key(*view,SDLK_F5);
        check(fs::is_regular_file(target),"Actual goods F5 did not save");
        const auto valid_bytes=f::read_text(target);
        check(oe::persistence::read_save(target).source_schema_version==25,"Actual goods F5 schema mismatch");
        check(view->execute(sim::demolish_building(source)).changed,"Rule9 source removal");
        check(view->world().treasury()==1860 && view->world().household_desirability(house)==0,"Rule9 removal refund/influence");
        zero_goods(view->world());key(*view,SDLK_F9);
        check(view->world().snapshot()==saved && view->paused(),"Actual goods F9 full restore");
        auto bad=nlohmann::json::parse(valid_bytes);
        bad["world"]["original_goods"]["totals"][1]["produced"]=1;
        bad["world"]["original_economy"]["houses"][0]["house"]["ceramics"]=1;
        f::write_text(target,bad.dump());
        const auto model=view->world().original_rules_model();
        const auto cache=view->world().original_influence_cache_build_count();
        key(*view,SDLK_F9);
        check(view->world().snapshot()==saved && view->world().original_rules_model()==model &&
            view->world().original_influence_cache_build_count()==cache,"Balanced fake goods F9 published state");
        auto old=oe::SandboxView::create_original_influence_save_fixture(session());
        old->configure_save(data,"Cities/A.map",old_target);old->initialize(window,renderer);key(*old,SDLK_F5);
        const auto old_snapshot=old->world().snapshot();
        const auto old_bytes=f::read_text(old_target);
        f::write_text(target,old_bytes);key(*view,SDLK_F9);
        check(view->world().snapshot()==saved,"Rule9 accepted old Rule8 save");
        f::write_text(old_target,valid_bytes);key(*old,SDLK_F9);
        check(old->world().snapshot()==old_snapshot,"Rule8 accepted new Rule9 save");
        old->shutdown();old.reset();f::write_text(target,valid_bytes);
        key(*view,SDLK_F9);check(view->world().snapshot()==saved,"Rule9 recovery after refused loads");
        const auto before=view->world().snapshot();
        const auto before_cache=view->world().original_influence_cache_build_count();
        perf::set_enabled(true);perf::reset();
        for(int i=0;i<30;++i){zero_goods(view->world());view->update(0);check(view->render(),"Goods idle frame");}
        check(view->world().snapshot()==before && view->world().original_influence_cache_build_count()==before_cache,"Goods query/frame changed World/cache");
        for(const auto counter:{perf::Counter::WorldCopies,perf::Counter::WorldExecutes,perf::Counter::BfsCalls,
                perf::Counter::AssetDecodes,perf::Counter::TextureUploads,perf::Counter::FileReads,perf::Counter::FileWrites})
            check(perf::counter(counter)==0,"Goods query/frame did file/asset/World/navigation work");
        perf::set_enabled(false);view->set_tool(5);click(*view,{106,101});
        check(view->selected_building()==house && view->render(),"Goods house picking/render");
        if(argc==2){
            SDL_Surface* frame=SDL_RenderReadPixels(renderer,nullptr);check(frame!=nullptr,"Goods capture");
            const bool okay=SDL_SavePNG(frame,argv[1]);SDL_DestroySurface(frame);check(okay,"Goods PNG output");
        }
        view->shutdown();view.reset();
        bool denied=false;try{oe::SandboxView normal(session(),false,sim::RulesProfile::CityV16,9);}
        catch(const std::invalid_argument&){denied=true;}
        check(denied,"Normal app exposed Rule9");
        SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();
        std::cout<<"Goods accounting SDL: paid source/house, unsupported/0, actual F5/F9, crossload/refusal and idle queries PASS\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"Goods view FAIL: "<<e.what()<<'\n';
        SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();return 1;
    }
}
