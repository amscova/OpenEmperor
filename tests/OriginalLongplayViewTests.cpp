#include "OriginalLongplayFixture.h"
#include "app/SandboxView.h"
#include "app/VisualSelection.h"
#include "core/PerformanceDiagnostics.h"

#include <SDL3/SDL.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

namespace {
namespace oe=openemperor;
namespace sim=oe::simulation;
namespace q=oe_longplay_fixture;
namespace fs=std::filesystem;
namespace perf=oe::performance;
using Json=nlohmann::json;
using oe_starter_fixture::check;

struct Sdl {
    SDL_Window* window=nullptr;
    SDL_Renderer* renderer=nullptr;
    Sdl() {
        check(SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy") && SDL_Init(SDL_INIT_VIDEO),"Longplay View SDL initialization failed");
        window=SDL_CreateWindow("Original longplay frame contract",1280,800,SDL_WINDOW_HIDDEN);
        check(window!=nullptr,SDL_GetError());renderer=SDL_CreateRenderer(window,"software");
        check(renderer!=nullptr,SDL_GetError());
    }
    ~Sdl() {perf::set_enabled(false);if(renderer)SDL_DestroyRenderer(renderer);if(window)SDL_DestroyWindow(window);SDL_Quit();}
};
void event(oe::SandboxView& view,SDL_Event value) {
    check(SDL_PushEvent(&value),"Queue ordinary longplay View event");bool running=true;
    while(SDL_PollEvent(&value)) view.handle_event(value,running);
    check(running,"Longplay View input closed its session");
}
void key(oe::SandboxView& view,SDL_Keycode code) {
    SDL_Event value{};value.type=SDL_EVENT_KEY_DOWN;value.key.key=code;event(view,value);
    value={};value.type=SDL_EVENT_KEY_UP;value.key.key=code;event(view,value);
}
oe::scene::Point window_point(SDL_Renderer* renderer,oe::scene::Point point) {
    float x=0,y=0;
    check(SDL_RenderCoordinatesToWindow(renderer,static_cast<float>(point.x),static_cast<float>(point.y),&x,&y),
          "Invert actual View coordinates");
    return {x,y};
}
void motion(oe::SandboxView& view,oe::scene::Point raw) {
    SDL_Event value{};value.type=SDL_EVENT_MOUSE_MOTION;
    value.motion.x=static_cast<float>(raw.x);value.motion.y=static_cast<float>(raw.y);event(view,value);
}
void pan(oe::SandboxView& view,oe::scene::Point wanted) {
    SDL_Event focus{};focus.type=SDL_EVENT_WINDOW_FOCUS_GAINED;event(view,focus);
    for(unsigned i=0;i<2000;++i) {
        const auto current=view.camera().offset;const auto dx=wanted.x-current.x,dy=wanted.y-current.y;
        if(std::abs(dx)<1e-6 && std::abs(dy)<1e-6)return;
        oe::SandboxView::PanKeyState held{};
        if(std::abs(dx)>=1e-6) {held.left=dx>0;held.right=dx<0;view.update(std::min(.05,std::abs(dx)/400),held);}
        else {held.up=dy>0;held.down=dy<0;view.update(std::min(.05,std::abs(dy)/400),held);}
    }
    throw std::runtime_error("Ordinary camera steering did not converge");
}
oe::scene::Point center(const oe::SandboxView& view,sim::Cell cell,unsigned border) {
    const auto* permission=view.world().map_permissions()->cell_permission(cell);
    check(permission!=nullptr,"Longplay projection lacks actual immutable cell height");
    return {double(cell.x-cell.y)*40,double(cell.x+cell.y-2*static_cast<int>(border))*20-
        double(permission->height)*40};
}
void focus(oe::SandboxView& view,oe::scene::Point point) {
    const auto map=view.layout().map;const auto zoom=view.camera().zoom;
    pan(view,{map.x+map.w*.5-point.x*zoom,map.y+map.h*.5-point.y*zoom});
    view.update(0);check(view.render(),"Warm actual focused frame");
}
oe::scene::Point pixel_for(oe::SandboxView& view,SDL_Renderer* renderer,sim::Cell cell,unsigned border) {
    const auto at=view.camera().world_to_screen(center(view,cell,border));
    for(int dy=-12;dy<=12;dy+=2)for(int dx=-20;dx<=20;dx+=2) {
        const oe::scene::Point point{at.x+dx,at.y+dy};
        if(view.layout().map.contains(point.x,point.y) && view.pick(point)==cell)return window_point(renderer,point);
    }
    throw std::runtime_error("Actual projected cell has no visible selectable pixel");
}
void visuals(oe::SandboxView& view,const fs::path& data,const fs::path& resources) {
    if(resources.empty())return;
    const auto selected=oe::detect_and_select_visual_profiles(data,resources);
    if(!selected.walker.empty())view.set_walker_visuals(selected.walker,selected.walker_source);
    if(!selected.building.empty())view.set_building_visuals(selected.building,selected.building_source);
    if(!selected.road.empty())view.set_road_visuals(selected.road,selected.road_source);
    view.set_fire_visuals(selected.fire,selected.fire_source,selected.fire_fallback_reason);
    view.set_fire_inspector_visuals(selected.fire_inspector,selected.fire_inspector_source,selected.fire_inspector_fallback_reason);
    view.set_market_walker_visuals(selected.market_walker,selected.market_walker_source,selected.market_walker_fallback_reason);
    view.set_service_walker_visuals(selected.service_walker,selected.service_walker_source,selected.service_walker_fallback_reason);
    view.set_health_walker_visuals(selected.health_walker,selected.health_walker_source,selected.health_walker_fallback_reason);
    view.set_original_house_visuals(selected.original_house,selected.original_house_source,selected.original_house_fallback_reason);
}
void zero_counters(std::optional<perf::Counter> allowed=std::nullopt,std::uint64_t count=0) {
    for(std::size_t i=0;i<static_cast<std::size_t>(perf::Counter::Count);++i) {
        const auto counter=static_cast<perf::Counter>(i);const auto expected=allowed==counter?count:0;
        check(perf::counter(counter)==expected,"Unexpected longplay frame work: "+std::string(perf::counter_names[i]));
    }
}
Json visible_capacities(const sim::World& world) {
    std::size_t paths=0,slots=0,cached=0;
    for(const auto& c:world.couriers()) {
        paths+=c.path.capacity();slots+=c.dynamic_target_routes.capacity();
        if(c.cached_route)cached+=c.cached_route->capacity();
        for(const auto& target:c.target_routes)if(target)cached+=target->capacity();
        for(const auto& [id,route]:c.dynamic_target_routes) {(void)id;if(route)cached+=route->capacity();}
    }
    return {{"active_path_cell_capacity",paths},{"dynamic_target_slot_capacity",slots},
        {"cached_route_cell_capacity",cached},{"scope","public Courier vectors only; not total heap"}};
}
}

int main(int argc,char** argv) {
    try {
        fs::path data,resources;unsigned frames=100;std::string scenario="original-starter";
        for(int i=1;i<argc;++i) {
            const std::string argument=argv[i];check(i+1<argc,"Missing longplay View option value");
            if(argument=="--data" && data.empty())data=fs::canonical(argv[++i]);
            else if(argument=="--resources" && resources.empty())resources=fs::canonical(argv[++i]);
            else if(argument=="--scenario")scenario=argv[++i];
            else if(argument=="--frames") {const auto value=std::stoul(argv[++i]);check(value>=100&&value<=3000,"View frames must be100..3000");frames=static_cast<unsigned>(value);}
            else throw std::runtime_error("Usage: longplay-view --data DATA [--resources ROOT] [--frames100..3000]");
        }
        check(!data.empty(),"Longplay View requires explicit original data");
        check(scenario=="original-starter"||scenario=="original-districts","Unknown longplay View scenario");
        const bool full=scenario=="original-districts";
        const auto context=full?q::real_district_context(data):q::Context::real(data);Sdl sdl;
        auto session=oe::maps::load_stored_map_session(context.data,context.map_relative,
            oe::maps::FootprintPolicy::EdgeByte4x4Preview,oe::maps::StoredGraphicsProfile::Slot8);
        const auto border=oe::maps::MapGeometry(session.map.declared_map_size).border;
        auto view=oe::SandboxView::create_original_production(std::move(session),oe::original::Difficulty::Normal,!full);
        oe_influence_fixture::Root saved;const auto save_path=saved.path/"earned.json";
        view->configure_save(context.data,context.map_relative,save_path);
        visuals(*view,context.data,resources);view->initialize(sdl.window,sdl.renderer);
        if(full) {
            auto earned=context.world();const auto districts=q::build(context,earned);
            for(unsigned i=0;i<1200;++i)earned.tick();q::check_authority(earned);q::assert_independent(earned,districts);
            const auto document=context.document(earned);
            oe::persistence::write_save(save_path,document,context.data,context.mask);key(*view,SDLK_F9);
            check(view->world().snapshot()==earned.snapshot(),"Actual F9 did not restore the complete paid82/58 city");
        } else for(unsigned i=0;i<4000;++i)view->tick_once();
        check(view->paused() && view->world().buildings().size()==(full?82U:19U),"Actual View lost its paused paid workload");
        q::check_authority(view->world());
        const auto first=std::find_if(view->world().buildings().begin(),view->world().buildings().end(),
            [](const auto& body){return sim::original_house_kind(body.kind);});
        check(first!=view->world().buildings().end(),"Paid View has no actual House");
        view->set_tool(5);focus(*view,center(*view,first->cell,border));
        const auto raw=pixel_for(*view,sdl.renderer,first->cell,border);motion(*view,raw);view->update(0);
        SDL_Event press{};press.type=SDL_EVENT_MOUSE_BUTTON_DOWN;press.button.button=SDL_BUTTON_LEFT;
        press.button.x=static_cast<float>(raw.x);press.button.y=static_cast<float>(raw.y);event(*view,press);
        press.type=SDL_EVENT_MOUSE_BUTTON_UP;event(*view,press);
        check(view->original_house_inspector_active(),"Ordinary mouse input failed to select the paid House inspector");
        const auto before=view->world().snapshot();const auto capacities=visible_capacities(view->world());
        const auto refreshes=view->world().route_refresh_count();const auto entries=view->world().route_cache_entries();
        const auto influence=view->world().original_influence_cache_build_count(),plans=view->road_plan_build_count();
        const auto water=view->water_preview_build_count(),des=view->desirability_preview_build_count(),checks=view->road_connectivity_check_count();
        const auto textures=std::pair{view->building_texture_count(),view->walker_texture_count()};
        check(view->render(),"Warm longplay inspector frame");const auto order=view->painter_stats().stored_order_builds;
        perf::set_enabled(true);perf::reset();
        for(unsigned i=0;i<frames;++i) {
            key(*view,SDLK_PAGEDOWN);key(*view,SDLK_F10);
            if(i%10==0) {key(*view,SDLK_F2);key(*view,SDLK_F4);key(*view,SDLK_F6);}
            motion(*view,raw);view->update(0);check(view->render(),"Paused longplay frame failed");
        }
        zero_counters();perf::set_enabled(false);
        check(view->world().snapshot()==before && visible_capacities(view->world())==capacities &&
            view->world().route_refresh_count()==refreshes && view->world().route_cache_entries()==entries &&
            view->world().original_influence_cache_build_count()==influence && view->road_plan_build_count()==plans &&
            view->water_preview_build_count()==water && view->desirability_preview_build_count()==des &&
            view->road_connectivity_check_count()==checks && view->painter_stats().stored_order_builds==order &&
            std::pair{view->building_texture_count(),view->walker_texture_count()}==textures,
            "Paused frames changed World/cache/public capacities/textures or stored order");
        // Search only in the test, outside all frame/counter windows, using ordinary authoritative validation.
        std::optional<sim::Cell> start;
        for(int y=0;y<view->world().height()&&!start;++y)for(int x=0;x+2<view->world().width()&&!start;++x) {
            const std::array cells{sim::Cell{x,y},sim::Cell{x+1,y},sim::Cell{x+2,y}};
            const auto plan=view->world().validate_road_batch(cells);
            if(plan.accepted&&plan.new_road_count==3)start=cells.front();
        }
        check(start.has_value(),"Real source has no affordable legal3-cell preview example");
        const sim::Cell middle{start->x+1,start->y},last{start->x+2,start->y};
        view->set_tool(1);focus(*view,center(*view,middle,border));
        const auto a=pixel_for(*view,sdl.renderer,*start,border),b=pixel_for(*view,sdl.renderer,middle,border),c=pixel_for(*view,sdl.renderer,last,border);
        motion(*view,a);view->update(0);press={};press.type=SDL_EVENT_MOUSE_BUTTON_DOWN;
        press.button.button=SDL_BUTTON_LEFT;press.button.x=static_cast<float>(a.x);press.button.y=static_cast<float>(a.y);event(*view,press);
        motion(*view,b);view->update(0);check(view->road_preview().valid&&view->road_preview().cells.size()==2,"Actual held Road preview missing");
        check(view->render(),"Warm held preview frame");const auto held_plans=view->road_plan_build_count();
        const auto held_capacity=view->road_preview().cells.capacity();perf::set_enabled(true);perf::reset();
        for(unsigned i=0;i<500;++i)motion(*view,b);
        check(view->road_plan_build_count()==held_plans,"Motion burst prepared a RoadPlan in event handling");
        view->update(0);for(unsigned i=0;i<100;++i)check(view->render(),"Held preview frame failed");zero_counters();
        check(view->road_preview().cells.capacity()==held_capacity,"Unchanged held preview grew its public vector capacity");
        motion(*view,c);check(view->road_plan_build_count()==held_plans,"Changed endpoint bypassed coalesced update");
        view->update(0);zero_counters(perf::Counter::RoadPlans,1);
        check(view->road_plan_build_count()==held_plans+1&&view->road_preview().cells.size()==3,"Changed endpoint did not prepare exactly one plan");
        perf::reset();for(unsigned i=0;i<100;++i){view->update(0);check(view->render(),"Changed held preview frame failed");}zero_counters();
        perf::set_enabled(false);key(*view,SDLK_ESCAPE);
        check(view->world().snapshot()==before&&view->world().route_refresh_count()==refreshes&&
              view->world().original_influence_cache_build_count()==influence,"Unreleased preview changed World or routing");
        const auto budgets=q::budgets(view->world());view->shutdown();view.reset();
        check(oe::BuildingSprite::live_texture_count()==0&&oe::WalkerSpriteSet::live_texture_count()==0&&
              oe::RoadSpriteSet::live_texture_count()==0&&oe::FireSpriteSet::live_texture_count()==0&&
              oe::StoredGraphicsRenderer::live_texture_count()==0,"Owned textures survived longplay View shutdown");
        std::cout<<Json{{"schema","openemperor-original-longplay-view-v1"},{"status","PASS"},{"scenario",scenario},{"original_map",context.original_map},
            {"original_model",context.original_model},{"map",context.map_relative.generic_string()},
            {"paused_frames",frames},{"zero_forbidden_counters",12},{"motion_events",500},
            {"held_preview_frames",200},{"coalesced_new_plans",1},{"held_preview_cell_capacity",held_capacity},
            {"visible_capacities",capacities},{"budgets",budgets},{"allocation_free_claim",false},
            {"full_heap_inventory_claim",false},{"native_human","NOT_RUN"}}.dump()<<'\n';
        return 0;
    } catch(const std::exception& error) {perf::set_enabled(false);std::cerr<<"Original longplay View: "<<error.what()<<'\n';return 1;}
}
