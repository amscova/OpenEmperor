// OE36: ordinary paid commands and real World/View/persistence only.
#include "../tests/EconomyFireDiagnosisMetrics.h"
#include "app/SandboxView.h"
#include "app/VisualSelection.h"

#include <SDL3/SDL.h>
#include <charconv>
#include <cmath>
#include <iostream>
#include <map>

namespace {
namespace q=oe_starter_fixture;
namespace sim=openemperor::simulation;
namespace oe=openemperor;
namespace fs=std::filesystem;
using Json=nlohmann::json;
using q::check;
constexpr const char* usage="fire-protection-acceptance --mode create|resume|render|check --rule 10|11 --data DATA [--output NEW_PRIVATE_OE36_DIRECTORY] [--input PRIVATE_SAVE] [--ticks 6000..60000] [--resources ROOT]";
struct Args {std::string mode;fs::path data,output,input,resources;unsigned rule=11,ticks=60000;};
fs::path private_root(){return fs::canonical(fs::path(__FILE__).parent_path().parent_path())/".local/original-economy/oe36";}
bool below(const fs::path& p,const fs::path& parent){auto i=p.begin();for(auto j=parent.begin();j!=parent.end();++j,++i)if(i==p.end()||*i!=*j)return false;return true;}
void safe(const fs::path& p){check(p.is_absolute(),"Proof path must be absolute");fs::path part;for(const auto& s:p){check(s!="."&&s!="..","Proof path must be canonical");part/=s;check(!fs::is_symlink(fs::symlink_status(part)),"Proof path traverses symlink");}}
Args arguments(int argc,char** argv){
    Args a;std::map<std::string,std::string> v;
    for(int i=1;i<argc;i+=2){check(i+1<argc,usage);std::string key=argv[i];check(key=="--mode"||key=="--rule"||key=="--data"||key=="--output"||key=="--input"||key=="--ticks"||key=="--resources",usage);check(v.emplace(key,argv[i+1]).second,"Duplicate argument");}
    check(v.contains("--mode")&&v.contains("--data"),usage);a.mode=v.at("--mode");
    check(a.mode=="create"||a.mode=="resume"||a.mode=="render"||a.mode=="check",usage);
    for(auto [name,at]:{std::pair{"--data",&a.data},{"--output",&a.output},{"--input",&a.input},{"--resources",&a.resources}})if(v.contains(name))*at=fs::absolute(v.at(name));
    for(auto [name,at]:{std::pair{"--rule",&a.rule},{"--ticks",&a.ticks}})if(v.contains(name)){const auto& s=v.at(name);auto r=std::from_chars(s.data(),s.data()+s.size(),*at);check(r.ec==std::errc{}&&r.ptr==s.data()+s.size(),usage);}
    check((a.rule==10||a.rule==11)&&a.ticks>=6000&&a.ticks<=60000,usage);return a;
}
void preflight(const Args& a){
    safe(a.data);check(fs::is_directory(a.data),"BLOCKED: original installation missing");
    if(a.mode!="check"){safe(a.output);check(a.output!=private_root()&&below(a.output,private_root())&&!fs::exists(a.output),"Output must be a new private OE36 directory");check(!below(a.output,a.data)&&!below(a.data,a.output),"Output overlaps original installation");q::save::validate_save_target(a.output/"report.json",a.data);}
    if(a.mode=="resume"||a.mode=="render"){safe(a.input);check(below(a.input,private_root())&&fs::is_regular_file(a.input)&&!below(a.input,a.output),"Explicit private input required");}else check(a.input.empty(),"Creator/check cannot import state");
    if(a.mode=="render"){safe(a.resources);check(fs::is_directory(a.resources),"Rendering requires actual compatibility resources");}
}
void write(const fs::path& p,const Json& j){check(!fs::exists(p),"Preserve prior proof artifact");q::f::write_text(p,j.dump(2)+"\n");}
sim::World restore(const q::Context& c,const q::save::SaveDocument& d){return d.world.rule_version==11?q::save::restore_original_fire_protection(d,c.data,c.mask,c.policy):c.restore(d);}
void checkpoint(const q::Context& c,const fs::path& out,const std::string& label,const sim::World& w){
    const auto path=out/(label+".save.json");check(!fs::exists(path),"Preserve checkpoint");
    q::save::write_save(path,c.document(w),c.data,c.mask);const auto d=q::save::read_save(path);auto r=restore(c,d);
    check(d.world==w.snapshot()&&r.snapshot()==w.snapshot(),"Complete Rule11/Rule10 save restore differs");
}
struct FireMetrics {
    std::uint64_t arrivals=0,extinguished=0,starts=0,protected_risk_steps=0,goal_loss_with_fire=0,goal_loss_with_fire_nonnegative=0,excluded_tax=0;
    std::map<sim::BuildingId,bool> burning;
    Json visits=Json::array(),incidents=Json::array();
    void before(const sim::World& w){
        const auto next=w.ticks()+1;
        for(const auto& b:w.buildings()) {burning[b.id]=w.building_on_fire(b.id);}
        if(next%sim::original_tax_interval_ticks==0)for(const auto& h:w.original_economy_state()->houses){const auto s=w.original_house_tax_status(h.building_id);if(s.inhabited&&s.burning)excluded_tax+=static_cast<std::uint64_t>(s.occupants)*static_cast<std::uint64_t>(s.rate);}
    }
    void after(const sim::World& w){
        bool house_fire=false;
        for(const auto& b:w.buildings()){
            const bool now=w.building_on_fire(b.id);
            if(now&&!burning[b.id]){++starts;incidents.push_back({{"tick",w.ticks()},{"building_id",static_cast<unsigned>(b.id)},{"watch",b.kind==sim::Object::FireWatch}});}
            if(sim::original_house_kind(b.kind)&&now)house_fire=true;
            if(w.building_fire_protected(b.id)&&w.ticks()>b.placed_tick&&(w.ticks()-b.placed_tick)%sim::Rules::fire_risk_step_ticks==0)++protected_risk_steps;
        }
        if(w.original_fire_protection_enabled())for(auto i=w.original_production_state()->fire_protection->arrivals.rbegin();i!=w.original_production_state()->fire_protection->arrivals.rend()&&i->tick==w.ticks();++i){
            const auto& event=*i;const auto& b=w.building(event.trip.target);
            ++arrivals;if(event.extinguished)++extinguished;
            const auto& actor=w.courier(event.trip.courier_id);
            check(actor.role==sim::CourierRole::FireInspector&&actor.target==b.id&&actor.phase==sim::CourierPhase::Returning,"Protection appeared without actual returning Inspector");
            visits.push_back({{"tick",w.ticks()},{"target",static_cast<unsigned>(b.id)},{"couriers",Json::array({static_cast<unsigned>(actor.id)})},{"extinguished",event.extinguished},
                {"deadline",b.fire_protection_until_tick},{"risk_after",b.fire_risk}});
            check(!w.building_on_fire(b.id)&&b.fire_risk==0&&b.fire_protection_until_tick==w.ticks()+sim::Rules::fire_protection_ticks,"Actual arrival did not protect/extinguish");
        }
        if(!w.original_goal_status().reached&&house_fire){++goal_loss_with_fire;if(w.treasury()>=0)++goal_loss_with_fire_nonnegative;}
    }
    Json report()const{return {{"actual_arrivals",arrivals},{"actual_extinguishments",extinguished},{"actual_fire_starts",starts},
        {"protected_risk_steps",protected_risk_steps},{"goal_loss_with_burning_houses",goal_loss_with_fire},
        {"goal_loss_with_burning_houses_nonnegative_funds",goal_loss_with_fire_nonnegative},
        {"tax_excluded_at_actual_burning_inputs",excluded_tax},{"excluded_tax_is_income",false},{"visits",visits},{"incidents",incidents}};}
};
Json build(sim::World& w,const q::Context& c,unsigned rule){
    const auto receipt=sim::place_original_economy_starter(w);q::paid_empty(c,w,receipt);
    auto purchases=oe_paid_progression_strategy::expand(w,receipt,"supplied");
    if(rule==10){(void)oe_paid_progression_strategy::fire_refusal(w,receipt.origin);return purchases;}
    for(const auto relative:{sim::Cell{0,5},sim::Cell{23,10}}){const auto at=q::offset(receipt,relative.x,relative.y);const auto funds=w.treasury();const auto paid=w.construction_spent_total();q::execute(w,{sim::CommandType::PlaceFireWatch,at});const auto id=*w.building_owner_at(at);
        check(funds-w.treasury()==80&&w.construction_spent_total()==paid+80&&w.workforce_required(id)==2,"Watch did not use paid authored80/2 contract");
        purchases.push_back({{"command","FireWatch"},{"cell",{at.x,at.y}},{"relative_cell",{relative.x,relative.y}},{"tick",w.ticks()},{"cost",80},{"workers",2},{"id",static_cast<unsigned>(id)}});
    }return purchases;
}
void simulate(const Args& a){
    preflight(a);const auto c=q::Context::real(a.data);check(c.model->provenance().sha256=="7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6","BLOCKED: unchanged pinned Normal model required");
    const bool resumed=a.mode=="resume",output=a.mode!="check";
    const auto input=resumed?q::f::read_text(a.input):std::string{};
    auto w=resumed?restore(c,q::save::read_save(a.input)):a.rule==11?sim::World::create_original_fire_protection(c.policy,c.model):c.world();
    check(w.rule_version()==a.rule,"Requested rule differs from explicit saved rule");if(output)fs::create_directories(a.output);
    Json purchases=Json::array();bool active_saved=false;FireMetrics warm_fire;
    if(!resumed){purchases=build(w,c,a.rule);while(w.ticks()<816){warm_fire.before(w);w.tick();warm_fire.after(w);
        if(output&&!active_saved)for(const auto& courier:w.couriers())if(courier.role==sim::CourierRole::FireInspector&&courier.phase==sim::CourierPhase::ToWarehouse&&courier.edge_progress>0){checkpoint(c,a.output,"midflight",w);active_saved=true;break;}}
    }else check(w.ticks()==816,"Continuation requires exact earned816 seed");
    q::authority(w);check(w.original_goal_status().reached,"Actual World goal absent at816");
    if(output){checkpoint(c,a.output,"seed",w);if(resumed)check(q::f::read_text(a.output/"seed.save.json")==input,"Separate process seed bytes changed");}
    oe_economy_fire_metrics::Observation metrics(w);FireMetrics fire;
    const auto end=w.ticks()+a.ticks;
    while(w.ticks()<end){metrics.before(w);fire.before(w);w.tick();metrics.after(w);fire.after(w);
        if(output&&w.ticks()==20816)checkpoint(c,a.output,"plus20000",w);
        if(output&&w.ticks()==816+a.ticks/2)checkpoint(c,a.output,"midpoint",w);
    }
    q::authority(w);q::worker_oracle(w);const auto observed=metrics.report(w);
    if(a.rule==11)check(fire.arrivals>0&&fire.protected_risk_steps>0,"No actual Inspector arrival/protection effect observed");
    else check(fire.arrivals==0&&fire.starts>0,"Rule10 baseline no longer exposes natural fire without protection");
    if(output){checkpoint(c,a.output,"result",w);write(a.output/"metrics.json",observed);write(a.output/"fire.json",fire.report());
        write(a.output/"report.json",{{"format","openemperor-fire-protection-process-v1"},{"status","PASS"},{"mode",a.mode},
            {"rule",a.rule},{"schema",a.rule==11?27:26},{"policy",3},{"revision",27},{"snapshot_injection",false},
            {"first_goal",oe_paid_progression_strategy::goal(restore(c,q::save::read_save(a.output/"seed.save.json")))},
            {"begin_tick",816},{"end_tick",w.ticks()},{"purchases",purchases},{"warmup_fire",warm_fire.report()},
            {"summary",observed.at("summary")},{"fire",fire.report()},{"native_human","NOT_RUN"}});
    }
    if(resumed)check(q::f::read_text(a.input)==input,"Continuation mutated protected input");
    std::cout<<"Rule"<<a.rule<<" paid protection technical PASS "<<observed.at("summary").dump()<<'\n';
}
struct Sdl{SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;Sdl(){check(SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy")&&SDL_Init(SDL_INIT_VIDEO),SDL_GetError());window=SDL_CreateWindow("OE36 actual SandboxView",1600,1000,SDL_WINDOW_HIDDEN);check(window,SDL_GetError());renderer=SDL_CreateRenderer(window,"software");check(renderer,SDL_GetError());}~Sdl(){if(renderer)SDL_DestroyRenderer(renderer);if(window)SDL_DestroyWindow(window);SDL_Quit();}};
void key(oe::SandboxView& v,SDL_Keycode code){SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=code;check(SDL_PushEvent(&e),SDL_GetError());e={};e.type=SDL_EVENT_KEY_UP;e.key.key=code;check(SDL_PushEvent(&e),SDL_GetError());bool running=true;while(SDL_PollEvent(&e))v.handle_event(e,running);check(running,"View exited unexpectedly");}
void center(oe::SandboxView& v,oe::scene::Point at){for(unsigned i=0;i<1000;++i){const auto map=v.layout().map;const auto cam=v.camera();const double dx=map.x+map.w*.5-at.x*cam.zoom-cam.offset.x,dy=map.y+map.h*.5-at.y*cam.zoom-cam.offset.y;if(std::abs(dx)<.01&&std::abs(dy)<.01)return;oe::SandboxView::PanKeyState held{};if(std::abs(dx)>.01){held.left=dx>0;held.right=dx<0;v.update(std::min(.05,std::abs(dx)/400),held);}else{held.up=dy>0;held.down=dy<0;v.update(std::min(.05,std::abs(dy)/400),held);}}throw std::runtime_error("Actual camera steering failed");}
void render(const Args& a){
    preflight(a);const auto c=q::Context::real(a.data);const auto input=q::f::read_text(a.input);const auto d=q::save::read_save(a.input);check(d.world.rule_version==11,"Render requires real Rule11 save");auto restored=restore(c,d);q::authority(restored);
    fs::create_directories(a.output);const auto target=a.output/"view.save.json";q::f::write_text(target,input);Sdl sdl;
    auto session=oe::maps::load_stored_map_session(c.data,c.map_relative,oe::maps::FootprintPolicy::EdgeByte4x4Preview,oe::maps::StoredGraphicsProfile::Slot8);const oe::maps::MapGeometry geometry(session.map.declared_map_size);
    auto v=oe::SandboxView::create_original_fire_protection(std::move(session),oe::original::Difficulty::Normal,false);v->configure_save(c.data,c.map_relative,target);
    const auto p=oe::detect_and_select_visual_profiles(c.data,a.resources);check(!p.original_house.empty(),"Actual original House art unavailable");
    if(!p.walker.empty())v->set_walker_visuals(p.walker,p.walker_source);if(!p.building.empty())v->set_building_visuals(p.building,p.building_source);if(!p.road.empty())v->set_road_visuals(p.road,p.road_source);
    v->set_fire_visuals(p.fire,p.fire_source,p.fire_fallback_reason);v->set_fire_inspector_visuals(p.fire_inspector,p.fire_inspector_source,p.fire_inspector_fallback_reason);
    v->set_market_walker_visuals(p.market_walker,p.market_walker_source,p.market_walker_fallback_reason);v->set_service_walker_visuals(p.service_walker,p.service_walker_source,p.service_walker_fallback_reason);v->set_health_walker_visuals(p.health_walker,p.health_walker_source,p.health_walker_fallback_reason);v->set_original_house_visuals(p.original_house,p.original_house_source,p.original_house_fallback_reason);
    v->initialize(sdl.window,sdl.renderer);key(*v,SDLK_F9);check(v->paused()&&v->world().snapshot()==d.world,"Actual Sandbox F9 differs: "+v->last_message());key(*v,SDLK_F5);check(q::f::read_text(target)==input,"Actual Sandbox F5 bytes differ");
    SDL_Event focus{};focus.type=SDL_EVENT_WINDOW_FOCUS_GAINED;bool running=true;v->handle_event(focus,running);
    const auto active=std::find_if(v->world().couriers().begin(),v->world().couriers().end(),[](const auto& actor){return actor.role==sim::CourierRole::FireInspector&&actor.phase==sim::CourierPhase::ToWarehouse&&actor.edge_progress>0;});
    check(active!=v->world().couriers().end(),"Render input lacks an actually moving Inspector");
    const auto owner=active->owner;const auto owner_index=static_cast<std::size_t>(std::distance(v->world().buildings().begin(),std::find_if(v->world().buildings().begin(),v->world().buildings().end(),[&](const auto& b){return b.id==owner;})));
    const auto panel=v->layout().panel;const auto scale=v->layout().scale;float window_x=0,window_y=0;
    check(SDL_RenderCoordinatesToWindow(sdl.renderer,float(panel.x+30*scale),float(panel.y+(46+static_cast<int>(owner_index)*18+3)*scale),&window_x,&window_y),SDL_GetError());
    for(const auto type:{SDL_EVENT_MOUSE_MOTION,SDL_EVENT_MOUSE_BUTTON_DOWN,SDL_EVENT_MOUSE_BUTTON_UP}){SDL_Event e{};e.type=type;if(type==SDL_EVENT_MOUSE_MOTION){e.motion.x=window_x;e.motion.y=window_y;}else{e.button.button=SDL_BUTTON_LEFT;e.button.x=window_x;e.button.y=window_y;}v->handle_event(e,running);}
    check(v->selected_building()==owner,"Actual building-list input did not select the paid Watch");
    const auto at=active->path.at(active->path_vertex);const auto height=v->world().map_permissions()->cell_height(at);
    const oe::scene::Point point{double(at.x-at.y)*40,double(at.x+at.y-2*static_cast<int>(geometry.border))*20-double(height)*40};Json captures=Json::array();
    for(unsigned zoom=1;zoom<=4;zoom*=2){while(v->camera().zoom!=zoom)key(*v,SDLK_Z);center(*v,zoom==1?oe::scene::Point{point.x+320,point.y+200}:point);v->update(0);check(v->render(),"Production Sandbox render failed");const auto file=a.output/("sandbox-"+std::to_string(zoom)+"x.png");std::unique_ptr<SDL_Surface,decltype(&SDL_DestroySurface)> image(SDL_RenderReadPixels(sdl.renderer,nullptr),SDL_DestroySurface);check(image&&SDL_SavePNG(image.get(),file.string().c_str()),"Cannot capture actual Sandbox renderer");captures.push_back({{"file",file.filename().string()},{"zoom",zoom},{"tick",v->world().ticks()}});}
    check(v->world().snapshot()==d.world&&q::f::read_text(a.input)==input,"Rendering mutated live authority/input");
    write(a.output/"report.json",{{"format","openemperor-fire-protection-view-v1"},{"status","PASS"},{"rule",11},{"schema",27},{"actual_sandbox_renderer",true},{"backend",SDL_GetRendererName(sdl.renderer)},{"complete_f5_f9_equality",true},{"selected_watch",static_cast<unsigned>(owner)},{"inspection",v->inspection_lines()},{"captures",captures},{"native_human","NOT_RUN"}});v->shutdown();
}
}
int main(int argc,char** argv){if(argc==2&&std::string_view(argv[1])=="--help"){std::cout<<usage<<'\n';return 0;}try{const auto a=arguments(argc,argv);if(a.mode=="render")render(a);else simulate(a);return 0;}catch(const std::exception& e){std::cerr<<"Fire protection FAIL: "<<e.what()<<'\n';return 1;}}
