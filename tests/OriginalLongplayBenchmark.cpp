// Separate-process diagnostics. Stopwatches enclose only production tick/render
// calls; observers, invariant checks, saves, JSON and file work stay outside.
#include "OriginalLongplayFixture.h"
#include "app/SandboxView.h"
#include "app/VisualSelection.h"

#include <SDL3/SDL.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <charconv>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#if defined(__APPLE__) || defined(__unix__)
#include <sys/resource.h>
#endif

namespace {
namespace oe=openemperor;
namespace sim=oe::simulation;
namespace q=oe_longplay_fixture;
namespace fs=std::filesystem;
using Json=nlohmann::json;
using Clock=std::chrono::steady_clock;
using oe_starter_fixture::check;
struct Options {
    std::string scenario;
    fs::path data,output,resources;
    std::uint64_t ticks=20000;
    unsigned frames=100;
};
void private_output(const fs::path& output) {
    const auto root=fs::canonical(fs::path(__FILE__).parent_path().parent_path());
    const auto allowed=(root/".local/original-economy/oe32").lexically_normal();
    const auto relative=output.lexically_relative(allowed);
    check(!relative.empty()&&*relative.begin()!=".."&&relative!=fs::path("."),
          "Benchmark output must be a fresh file below this checkout's.local/original-economy/oe32");
    fs::path prefix;
    for(const auto& part:output) {
        prefix/=part;std::error_code error;const auto status=fs::symlink_status(prefix,error);
        check(!error||error==std::errc::no_such_file_or_directory,"Cannot inspect benchmark output path");
        check(!fs::is_symlink(status),"Benchmark output path traverses a symlink");
    }
}
std::uint64_t integer(const std::string& text,std::uint64_t low,std::uint64_t high) {
    std::uint64_t result=0;const auto parsed=std::from_chars(text.data(),text.data()+text.size(),result);
    check(parsed.ec==std::errc{}&&parsed.ptr==text.data()+text.size()&&result>=low&&result<=high,"Invalid bounded benchmark number");
    return result;
}
Options options(int argc,char** argv) {
    Options result;std::vector<std::string> seen;
    for(int i=1;i<argc;++i) {
        const std::string key=argv[i];check(i+1<argc,"Missing benchmark option value");
        check(std::find(seen.begin(),seen.end(),key)==seen.end(),"Duplicate benchmark option");seen.push_back(key);
        const std::string value=argv[++i];
        if(key=="--scenario")result.scenario=value;
        else if(key=="--data")result.data=fs::canonical(value);
        else if(key=="--output")result.output=fs::absolute(value).lexically_normal();
        else if(key=="--resources")result.resources=fs::canonical(value);
        else if(key=="--ticks")result.ticks=integer(value,1,1000000);
        else if(key=="--frames")result.frames=static_cast<unsigned>(integer(value,1,3000));
        else throw std::runtime_error("Unknown benchmark option: "+key);
    }
    check((result.scenario=="original-starter"||result.scenario=="original-districts"||result.scenario=="legacy")&&
          !result.data.empty()&&!result.output.empty(),
          "Usage: benchmark --scenario original-starter|original-districts|legacy --data DATA --output FRESH.json [--ticks N] [--frames N] [--resources ROOT]");
    check(!fs::exists(result.output),"Benchmark output must be fresh");
    private_output(result.output);
    oe::persistence::validate_save_target(result.output,result.data);
    return result;
}
std::uint64_t ns(Clock::duration duration) {
    const auto count=std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
    check(count>=0,"Steady benchmark clock moved backwards");return static_cast<std::uint64_t>(count);
}
Json summarize(std::vector<std::uint64_t> samples) {
    check(!samples.empty(),"No benchmark timing samples");std::sort(samples.begin(),samples.end());
    std::uint64_t total=0;for(const auto sample:samples){check(sample<=UINT64_MAX-total,"Timing sum overflow");total+=sample;}
    return {{"samples",samples.size()},{"total_ns",total},{"median_ns",samples[(samples.size()-1)/2]},
        {"p95_ns",samples[(samples.size()-1)*95/100]},{"max_ns",samples.back()},
        {"mean_ns",static_cast<double>(total)/static_cast<double>(samples.size())}};
}
struct Sdl {
    SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;
    Sdl() {
        check(SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy")&&SDL_Init(SDL_INIT_VIDEO),"Initialize benchmark SDL");
        window=SDL_CreateWindow("Bounded longplay benchmark",1280,800,SDL_WINDOW_HIDDEN);check(window!=nullptr,SDL_GetError());
        renderer=SDL_CreateRenderer(window,"software");check(renderer!=nullptr,SDL_GetError());
    }
    ~Sdl(){if(renderer)SDL_DestroyRenderer(renderer);if(window)SDL_DestroyWindow(window);SDL_Quit();}
};
struct Temp {
    fs::path root=fs::canonical(fs::temp_directory_path())/("openemperor-longplay-benchmark-"+
        std::to_string(Clock::now().time_since_epoch().count()));
    Temp(){check(fs::create_directory(root),"Create fresh isolated benchmark save root");}
    ~Temp(){std::error_code ignored;fs::remove_all(root,ignored);}
};
std::vector<sim::Command> legacy_recipe(sim::Cell origin) {
    std::vector<sim::Command> commands;commands.reserve(26);
    const auto place=[&](sim::CommandType type,int x,int y) {
        commands.push_back({type,{origin.x+x,origin.y+y}});
    };
    place(sim::CommandType::PlaceClaySource,0,2);place(sim::CommandType::PlacePottery,0,5);
    place(sim::CommandType::PlaceWarehouse,3,2);place(sim::CommandType::PlaceFarm,4,5);
    place(sim::CommandType::PlaceMarket,3,5);place(sim::CommandType::PlaceServicePost,5,5);
    for(int x=0;x<=14;++x)place(sim::CommandType::PlaceRoad,x,4);
    for(const auto cell:{sim::Cell{6,2},sim::Cell{6,5},sim::Cell{9,2},sim::Cell{9,5},sim::Cell{15,0}})
        place(sim::CommandType::PlaceHousehold,cell.x,cell.y);
    check(commands.size()==26,"Legacy paid recipe command count changed");return commands;
}
sim::Cell legacy_origin(const sim::World& world) {
    // Only unchanged Rule1 mask/command validation chooses this QA origin.
    // Unused rectangle cells and a fabricated source-height-zero predicate
    // must not exclude otherwise legal paid legacy footprints and roads.
    for(int y=0;y<world.height();++y)for(int x=0;x<world.width();++x) {
        const auto commands=legacy_recipe({x,y});
        if(std::all_of(commands.begin(),commands.end(),[&](sim::Command command){return world.validate(command).accepted;}))
            return{x,y};
    }
    throw std::runtime_error("Actual Xia legacy mask has no normally valid placement for the exact26-command paid control");
}
void build_legacy(sim::World& world,sim::Cell origin) {
    for(const auto command:legacy_recipe(origin)) {
        const auto result=world.execute(command);
        check(result.accepted&&result.changed,"Paid legacy command failed: "+result.reason);
    }
    check(world.buildings().size()==11&&world.construction_spent_total()==1280&&world.treasury()==20&&world.ticks()==0,
          "Paid unchanged Legacy representative differs from its1280-Funds construction");
}
void authority(const sim::World& world,bool original) {
    if(original) {q::check_authority(world);return;}
    check(world.production_balance_valid()&&world.food_balance_valid()&&world.city_economy_valid()&&
        world.service_state_valid()&&world.population_valid()&&world.navigation_valid()&&
        world.fire_state_valid()&&world.health_state_valid(),"Unchanged Legacy benchmark invariant failed");
    check(world.buildings().size()<=sim::Rules::city_v15_building_limit&&
          world.couriers().size()<=sim::Rules::city_v15_courier_limit,"Legacy collection bound changed");
}
Json capacities(const sim::World& world) {
    std::size_t active=0,cached=0,slots=0,permitted=0;
    for(const auto& courier:world.couriers()) {
        active+=courier.path.capacity();slots+=courier.dynamic_target_routes.capacity();
        if(courier.cached_route)cached+=courier.cached_route->capacity();
        for(const auto& route:courier.target_routes)if(route)cached+=route->capacity();
        for(const auto& [id,route]:courier.dynamic_target_routes){(void)id;if(route)cached+=route->capacity();}
        for(const auto& target:world.buildings())if(target.id!=courier.owner&&sim::courier_can_target(courier.role,target))++permitted;
    }
    check(world.route_cache_entries()==permitted,"Prepared route slots differ from independently enumerated role/target pairs");
    return {{"buildings",world.buildings().size()},{"couriers",world.couriers().size()},
        {"route_slots",world.route_cache_entries()},{"role_permitted_slots",permitted},{"route_refreshes",world.route_refresh_count()},
        {"active_path_cell_capacity",active},{"dynamic_target_slot_capacity",slots},{"cached_route_cell_capacity",cached},
        {"measured_route_cell_bytes",(active+cached)*sizeof(sim::Cell)},
        {"scope","public Courier vectors only; not full heap or private View scratch capacity"}};
}
struct Observer {
    std::vector<bool> burning,sick;
    std::uint64_t fire_starts=0,illness_starts=0,burning_owner_ticks=0,sick_house_ticks=0;
    std::uint64_t paused_unstaffed_ticks=0,enabled_staffed_ticks=0;
    explicit Observer(std::size_t count):burning(count,false),sick(count,false){}
    void sample(const sim::World& world,sim::BuildingId controlled,bool original) {
        for(std::size_t i=0;i<world.buildings().size();++i) {
            const auto& body=world.buildings()[i];const bool fire=world.building_on_fire(body.id);
            const bool illness=body.kind==sim::Object::Household&&world.household_sick(body.id);
            if(fire&&!burning[i])++fire_starts;if(illness&&!sick[i])++illness_starts;
            burning_owner_ticks+=fire;sick_house_ticks+=illness;burning[i]=fire;sick[i]=illness;
            if(original)check(body.health_risk==0&&body.sick_until_tick==0&&body.health_protection_until_tick==0,
                "Original benchmark acquired forbidden Legacy illness authority");
        }
        const auto& body=world.building(controlled);const auto workers=world.workers_assigned(controlled);
        if(!body.operating_enabled&&workers==0)++paused_unstaffed_ticks;
        if(body.operating_enabled&&workers>0)++enabled_staffed_ticks;
    }
    Json json() const {return {{"natural_fire_starts",fire_starts},{"burning_owner_ticks",burning_owner_ticks},
        {"natural_illness_starts",illness_starts},{"sick_house_ticks",sick_house_ticks},
        {"controlled_paused_unstaffed_ticks",paused_unstaffed_ticks},{"enabled_staffed_ticks",enabled_staffed_ticks}};}
};
void visuals(oe::SandboxView& view,const fs::path& data,const fs::path& resources) {
    if(resources.empty())return;const auto selected=oe::detect_and_select_visual_profiles(data,resources);
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
void f9(oe::SandboxView& view) {
    SDL_Event value{};value.type=SDL_EVENT_KEY_DOWN;value.key.key=SDLK_F9;
    check(SDL_PushEvent(&value),"Queue benchmark F9");bool running=true;
    while(SDL_PollEvent(&value))view.handle_event(value,running);check(running,"Benchmark F9 closed its View");
}
void event(oe::SandboxView& view,SDL_Event value) {
    check(SDL_PushEvent(&value),"Queue ordinary benchmark camera input");bool running=true;
    while(SDL_PollEvent(&value))view.handle_event(value,running);
    check(running,"Benchmark camera input closed its View");
}
void focus_city(oe::SandboxView& view,SDL_Renderer* renderer,const q::Context& context,
                unsigned border,const std::optional<q::Scene>& districts) {
    SDL_Event focus{};focus.type=SDL_EVENT_WINDOW_FOCUS_GAINED;event(view,focus);
    const auto map=view.layout().map;
    float raw_x=0,raw_y=0;
    check(SDL_RenderCoordinatesToWindow(renderer,static_cast<float>(map.x+map.w*.5),
        static_cast<float>(map.y+map.h*.5),&raw_x,&raw_y),"Transform benchmark wheel focus");
    for(unsigned i=0;i<3&&std::abs(view.camera().zoom-1.0)>1e-9;++i) {
        SDL_Event wheel{};wheel.type=SDL_EVENT_MOUSE_WHEEL;wheel.wheel.mouse_x=raw_x;wheel.wheel.mouse_y=raw_y;
        wheel.wheel.y=static_cast<float>(std::log(1.0/view.camera().zoom)/std::log(1.15));
        event(view,wheel);view.update(0);
    }
    check(std::abs(view.camera().zoom-1.0)<1e-9,"Ordinary benchmark mouse wheel did not reach1x");
    int min_x=view.world().width(),min_y=view.world().height(),max_x=0,max_y=0;
    for(const auto& body:view.world().buildings()) {
        if(districts&&std::find(districts->districts[0].begin(),districts->districts[0].end(),body.id)==districts->districts[0].end())continue;
        const auto footprint=sim::building_footprint(view.world().profile(),view.world().rule_version(),body.kind);
        min_x=std::min(min_x,body.cell.x);min_y=std::min(min_y,body.cell.y);
        max_x=std::max(max_x,body.cell.x+footprint.width-1);max_y=std::max(max_y,body.cell.y+footprint.height-1);
    }
    check(min_x<=max_x&&min_y<=max_y,"Benchmark has no paid city to focus");
    const double x=(min_x+max_x)*.5,y=(min_y+max_y)*.5;
    const auto* source=context.policy->cell_permission({static_cast<int>(std::floor(x)),static_cast<int>(std::floor(y))});
    check(source!=nullptr,"Benchmark city focus lacks source height");
    const oe::scene::Point projected{(x-y)*40,(x+y-2*border)*20-source->height*40};
    const oe::scene::Point wanted{map.x+map.w*.5-projected.x,map.y+map.h*.5-projected.y};
    bool reached=false;
    for(unsigned i=0;i<2000;++i) {
        const auto current=view.camera().offset;const auto dx=wanted.x-current.x,dy=wanted.y-current.y;
        if(std::abs(dx)<1e-6&&std::abs(dy)<1e-6){reached=true;break;}
        oe::SandboxView::PanKeyState held{};
        if(std::abs(dx)>=1e-6){held.left=dx>0;held.right=dx<0;view.update(std::min(.05,std::abs(dx)/400),held);}
        else {held.up=dy>0;held.down=dy<0;view.update(std::min(.05,std::abs(dy)/400),held);}
    }
    check(reached,"Ordinary benchmark held-key pan did not reach the paid city");
    view.update(0);
}
Json visible_load(const oe::SandboxView& view,const q::Context& context,unsigned border) {
    const auto map=view.layout().map;std::size_t witnessed_buildings=0,witnessed_cells=0;
    for(const auto& body:view.world().buildings()) {
        bool witnessed=false;
        for(const auto cell:sim::building_footprint_cells(view.world().profile(),view.world().rule_version(),body.kind,body.cell)) {
            const auto* source=context.policy->cell_permission(cell);check(source!=nullptr,"Benchmark footprint lacks source height");
            const auto screen=view.camera().world_to_screen({double(cell.x-cell.y)*40,
                double(cell.x+cell.y-2*static_cast<int>(border))*20-double(source->height)*40});
            bool selected=false;
            for(int dy=-8;dy<=8&&!selected;dy+=4)for(int dx=-12;dx<=12&&!selected;dx+=4) {
                const oe::scene::Point sample{screen.x+dx,screen.y+dy};
                selected=map.contains(sample.x,sample.y)&&view.pick(sample)==cell;
            }
            if(selected){++witnessed_cells;witnessed=true;}
        }
        if(witnessed)++witnessed_buildings;
    }
    check(witnessed_buildings>0,"Timed benchmark camera has no actually selectable paid building in its aperture");
    std::size_t sprite_bounds_in_aperture=0,submitted_walkers=0;
    for(const auto& row:view.walker_diagnostics()) {
        submitted_walkers+=row.submitted;
        if(row.sprite_drawn&&row.image_origin.x+row.image_width>map.x&&row.image_origin.y+row.image_height>map.y&&
           row.image_origin.x<map.x+map.w&&row.image_origin.y<map.y+map.h)++sprite_bounds_in_aperture;
    }
    return {{"selectable_building_witnesses",witnessed_buildings},{"selectable_footprint_cell_witnesses",witnessed_cells},
        {"submitted_walkers",submitted_walkers},{"native_walker_sprite_bounds_in_aperture",sprite_bounds_in_aperture},
        {"courier_draw_calls_last_frame",view.last_courier_draws()},
        {"scope","footprint witnesses use actual cached picking after render; sprite bounds may be occluded; draw calls include clipped/offscreen markers"}};
}
}

int main(int argc,char** argv) {
    try {
        const auto opt=options(argc,argv);const bool original=opt.scenario!="legacy";const auto began=Clock::now();
        const auto load_begin=Clock::now();
        const auto context=opt.scenario=="original-districts"?q::real_district_context(opt.data):q::Context::real(opt.data);
        const auto load_ns=ns(Clock::now()-load_begin);
        auto world=original?context.world():sim::World(context.policy->width(),context.policy->height(),context.mask,sim::RulesProfile::CityV16,1);
        const auto setup_begin=Clock::now();std::optional<q::Scene> districts;
        if(opt.scenario=="original-starter")sim::place_original_economy_starter(world);
        else if(original){districts=q::build(context,world);q::assert_independent(world,*districts);}
        else build_legacy(world,legacy_origin(world));
        authority(world,original);const auto setup_ns=ns(Clock::now()-setup_begin);
        const auto controlled=std::find_if(world.buildings().begin(),world.buildings().end(),
            [](const auto& body){return body.kind==sim::Object::Market;});
        check(controlled!=world.buildings().end(),"Paid benchmark scene has no normal operation-controlled Market");
        const auto controlled_id=controlled->id;const auto start_tick=world.ticks();const auto start_capacities=capacities(world);
        Observer observed(world.buildings().size());std::vector<std::uint64_t> tick_samples;tick_samples.reserve(static_cast<std::size_t>(opt.ticks));
        std::uint64_t checks_ns=0,observer_ns=0;
        for(std::uint64_t i=0;i<opt.ticks;++i) {
            if(i==500||i==900) {
                const auto result=world.execute(sim::set_building_operation(controlled_id,i==900));
                check(result.accepted&&result.changed,"Ordinary intermittent staffing control failed");
            }
            const auto before=Clock::now();world.tick();const auto elapsed=ns(Clock::now()-before);tick_samples.push_back(elapsed);
            const auto observation_begin=Clock::now();observed.sample(world,controlled_id,original);observer_ns+=ns(Clock::now()-observation_begin);
            if((i+1)%256==0||i+1==opt.ticks) {const auto check_begin=Clock::now();authority(world,original);checks_ns+=ns(Clock::now()-check_begin);}
        }
        check(world.ticks()==start_tick+opt.ticks,"Benchmark did not execute its exact requested production ticks");
        if(!original&&opt.ticks>=4600)check(observed.fire_starts>0&&observed.illness_starts>0&&
            observed.paused_unstaffed_ticks>0&&observed.enabled_staffed_ticks>0,
            "Paid unchanged Legacy control did not observe actual fire, illness and intermittent staffing");
        if(original)check(observed.illness_starts==0,"Original illness was incorrectly treated as implemented");
        const auto end_capacities=capacities(world);const auto view_begin=Clock::now();Temp private_save;Sdl sdl;
        auto session=oe::maps::load_stored_map_session(context.data,context.map_relative,
            oe::maps::FootprintPolicy::EdgeByte4x4Preview,oe::maps::StoredGraphicsProfile::Slot8);
        const auto border=oe::maps::MapGeometry(session.map.declared_map_size).border;
        auto view=original?oe::SandboxView::create_original_production(std::move(session),oe::original::Difficulty::Normal,false):
            std::make_unique<oe::SandboxView>(std::move(session),false,sim::RulesProfile::CityV16,1);
        const auto save=private_save.root/"earned.json";view->configure_save(context.data,context.map_relative,save);
        visuals(*view,context.data,opt.resources);view->initialize(sdl.window,sdl.renderer);
        check(view->buildable_mask()==context.mask,"Actual View legacy-derived mask differs from the timed World");
        const auto document=oe::persistence::make_document(context.data,context.map_relative,context.mask,world);
        oe::persistence::write_save(save,document,context.data,context.mask);
        f9(*view);
        check(view->world().snapshot()==world.snapshot()&&view->paused(),"Actual benchmark F9 failed complete earned-state restore");
        focus_city(*view,sdl.renderer,context,border,districts);
        for(unsigned i=0;i<5;++i)check(view->render(),"Warm benchmark frame failed");
        const auto visibility=visible_load(*view,context,border);
        const auto view_setup_ns=ns(Clock::now()-view_begin);const auto saved=view->world().snapshot();
        const auto before_frame_capacities=capacities(view->world());const auto texture_counts=std::pair{view->building_texture_count(),view->walker_texture_count()};
        std::vector<std::uint64_t> frame_samples;frame_samples.reserve(opt.frames);
        for(unsigned i=0;i<opt.frames;++i) {
            const auto before=Clock::now();const bool rendered=view->render();const auto elapsed=ns(Clock::now()-before);
            check(rendered,"Timed benchmark frame failed");frame_samples.push_back(elapsed);
        }
        check(view->world().snapshot()==saved&&capacities(view->world())==before_frame_capacities&&
            std::pair{view->building_texture_count(),view->walker_texture_count()}==texture_counts,
            "Timed paused benchmark frames changed authority, public capacities or textures");
        const auto painter=view->painter_stats();Json peak=nullptr;
#if defined(__APPLE__) || defined(__unix__)
        struct rusage usage{};if(getrusage(RUSAGE_SELF,&usage)==0) {
#if defined(__APPLE__)
            peak=usage.ru_maxrss;
#else
            peak=static_cast<std::uint64_t>(usage.ru_maxrss)*1024;
#endif
        }
#endif
        Json report={{"schema","openemperor-original-longplay-benchmark-v1"},{"budget_format",1},{"status","PASS"},
            {"scenario",opt.scenario},{"map",context.map_relative.generic_string()},{"map_sha256",document.map_sha256},
            {"original_map",context.original_map},{"original_model_used",original},{"model_sha256",context.model->provenance().sha256},
            {"profile",sim::rules_profile_name(world.profile())},{"rule",world.rule_version()},
            {"grid",{world.width(),world.height()}},{"viewport",{1280,800}},{"renderer",SDL_GetRendererName(sdl.renderer)},
            {"start_tick",start_tick},{"end_tick",world.ticks()},{"ticks",opt.ticks},{"frames",opt.frames},
            {"load_ns",load_ns},{"paid_setup_ns",setup_ns},{"view_prepare_save_restore_warmup_ns",view_setup_ns},
            {"tick_timing",summarize(std::move(tick_samples))},{"frame_timing",summarize(std::move(frame_samples))},
            {"invariant_check_ns_excluded",checks_ns},{"observation_ns_excluded",observer_ns},
            {"peak_rss_bytes",peak},{"process_wall_ns",ns(Clock::now()-began)},
            {"start_public_capacities",start_capacities},{"end_public_capacities",end_capacities},
            {"observed",observed.json()},{"construction_spent",world.construction_spent_total()},
            {"tax_collected",world.taxes_collected_total()},{"maintenance_spent",world.maintenance_spent_total()},
            {"funds",world.treasury()},{"population",world.total_population()},
            {"draw_load",{{"stored_items_visited",painter.stored_items_visited},{"dynamic_items",painter.sandbox_items},
                {"dynamic_items_scope","submitted painter descriptors including offscreen items"},
                {"visible_witnesses",visibility},{"building_textures",texture_counts.first},{"walker_textures",texture_counts.second}}},
            {"camera",{{"zoom",view->camera().zoom},{"offset",{view->camera().offset.x,view->camera().offset.y}},
                {"focus",districts?"first paid district center":"paid representative city center"},
                {"input","ordinary mouse wheel and held-key pan; excluded from frame timers"}}},
            {"comparison_scope","same software viewport; compare each map/grid/load explicitly; different legal economics and paid city size; diagnostic ratios only"},
            {"tick_window","World::tick only; operation commands/checks/observers/JSON/files excluded"},
            {"frame_window","SandboxView::render only; warm paused earned state; no readback or JSON"},
            {"normal_frame_instrumentation_added",false},{"full_heap_inventory_claim",false},{"native_human","NOT_RUN"}};
        if(original)report["budgets"]=q::budgets(world);
        if(districts)report["districts"]=q::describe(context,world,*districts);
        view->shutdown();view.reset();
        check(!fs::exists(opt.output),"Refuse replacing a benchmark output created during execution");fs::create_directories(opt.output.parent_path());
        private_output(opt.output);oe::persistence::validate_save_target(opt.output,opt.data);std::ofstream output(opt.output,std::ios::binary);
        output<<report.dump(2)<<'\n';output.close();check(static_cast<bool>(output),"Write fresh benchmark report");
        std::cout<<report.dump()<<'\n';return 0;
    } catch(const std::exception& error){std::cerr<<"Longplay benchmark: "<<error.what()<<'\n';return 1;}
}
