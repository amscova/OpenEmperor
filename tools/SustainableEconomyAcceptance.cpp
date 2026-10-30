// OE37: actual paid World commands, exact persistence and finite observations.
#include "../tests/SustainableEconomyMetrics.h"
#include "app/SandboxView.h"
#include "app/VisualSelection.h"
#include "persistence/RecoveryStore.h"
#include <cmath>

#include <charconv>
#include <iostream>
#include <map>
#include <optional>

namespace {
namespace q=oe_starter_fixture;
namespace sim=openemperor::simulation;
namespace oe=openemperor;
namespace player=oe_sustainable_strategy;
namespace metrics=oe_sustainable_metrics;
namespace fs=std::filesystem;
using Json=nlohmann::json;
using q::check;
constexpr const char* usage="sustainable-economy-acceptance --mode create|resume|check|render --rule 11|12 --variant supplied12|lean12|compact-two|three-food|compact-three|balanced-three|compact-connected-three --data ABSOLUTE [--output NEW_PRIVATE_OE37_OR_OE43_DIRECTORY] [--ticks 6000..150000] [--input EARNED_SEED] [--resources ABSOLUTE]";
struct Args {std::string mode,variant;fs::path data,output,input,resources;unsigned rule=12,ticks=100000;};
#ifndef OE_SUSTAINABLE_PROOF_DIRECTORY
#define OE_SUSTAINABLE_PROOF_DIRECTORY "oe37"
#endif
fs::path private_root(){return fs::canonical(fs::path(__FILE__).parent_path().parent_path())/".local/original-economy"/OE_SUSTAINABLE_PROOF_DIRECTORY;}
bool below(const fs::path& p,const fs::path& parent){auto i=p.begin();for(auto j=parent.begin();j!=parent.end();++j,++i)if(i==p.end()||*i!=*j)return false;return true;}
// Shared by the reused OE39/OE40 QA executables. This adds an output root only;
// their protected earned-input roots and exact-generation restores stay separate.
bool private_output(const fs::path& path){
    const auto playtest=fs::canonical(fs::path(__FILE__).parent_path().parent_path())/".local/oe43";
    return (path!=private_root()&&below(path,private_root())) ||
        (path!=playtest&&below(path,playtest));
}
void safe(const fs::path& p){check(p.is_absolute(),"Proof path must be absolute");fs::path part;for(const auto& s:p){check(s!="."&&s!="..","Proof path must be canonical");part/=s;std::error_code e;const auto status=fs::symlink_status(part,e);check(!e||e==std::errc::no_such_file_or_directory,"Cannot inspect proof path");check(!fs::is_symlink(status),"Proof path traverses symlink");}}
Args arguments(int argc,char** argv){
    Args a;std::map<std::string,std::string> v;
    for(int i=1;i<argc;i+=2){check(i+1<argc,usage);const std::string key=argv[i];check(key=="--mode"||key=="--rule"||key=="--variant"||key=="--data"||key=="--output"||key=="--input"||key=="--ticks"||key=="--resources",usage);check(v.emplace(key,argv[i+1]).second,"Duplicate proof argument");}
    for(const auto* key:{"--mode","--variant","--data"})check(v.contains(key),usage);
    a.mode=v.at("--mode");a.variant=v.at("--variant");check(a.mode=="create"||a.mode=="resume"||a.mode=="check"||a.mode=="render",usage);check(player::known(a.variant),usage);
    for(auto [key,target]:{std::pair{"--data",&a.data},{"--output",&a.output},{"--input",&a.input},{"--resources",&a.resources}})if(v.contains(key))*target=fs::absolute(v.at(key));
    for(auto [key,target]:{std::pair{"--rule",&a.rule},{"--ticks",&a.ticks}})if(v.contains(key)){const auto& text=v.at(key);const auto result=std::from_chars(text.data(),text.data()+text.size(),*target);check(result.ec==std::errc{}&&result.ptr==text.data()+text.size(),usage);}
    check((a.rule==11||a.rule==12)&&a.ticks>=6000&&a.ticks<=150000,usage);return a;
}
void preflight(const Args& a){
    safe(a.data);check(fs::is_directory(a.data),"BLOCKED: original installation missing");
    if(a.mode!="check"){safe(a.output);check(private_output(a.output)&&!fs::exists(a.output),"Output must be new private feature/OE43 directory");check(!below(a.output,a.data)&&!below(a.data,a.output),"Output overlaps original installation");q::save::validate_save_target(a.output/"report.json",a.data);}
    if(a.mode=="resume"||a.mode=="render"){safe(a.input);check(below(a.input,private_root())&&fs::is_regular_file(a.input)&&!below(a.input,a.output),"Explicit private earned input required");}else check(a.input.empty(),"Creator/check cannot import state");
    if(a.mode=="render"){safe(a.resources);check(fs::is_directory(a.resources),"Rendering requires compatibility resources");}
}
void write(const fs::path& path,const Json& value){check(!fs::exists(path),"Preserve previous proof artifact");q::f::write_text(path,value.dump(2)+"\n");}
sim::World fresh(const q::Context& c,unsigned rule){
#ifdef OE_MARKET_LOGISTICS_QA
    if(rule==13)return sim::World::create_original_market_logistics(c.policy,c.model);
#endif
    if(rule==11)return sim::World::create_original_fire_protection(c.policy,c.model);
#ifdef OE37_BASELINE
    throw std::invalid_argument("Frozen OE36 baseline does not implement Rule12");
#else
    return sim::World::create_original_sustainable_economy(c.policy,c.model);
#endif
}
sim::World restore(const q::Context& c,const q::save::SaveDocument& d){
#ifdef OE_MARKET_LOGISTICS_QA
    if(d.world.rule_version==13)return q::save::restore_original_market_logistics(d,c.data,c.mask,c.policy,c.model);
#endif
    if(d.world.rule_version==11)return q::save::restore_original_fire_protection(d,c.data,c.mask,c.policy);
#ifdef OE37_BASELINE
    throw std::invalid_argument("Frozen OE36 baseline cannot restore Rule12");
#else
    return q::save::restore_original_sustainable_economy(d,c.data,c.mask,c.policy);
#endif
}
void checkpoint(const q::Context& c,const fs::path& output,const char* label,const sim::World& w){
    const auto path=output/(std::string(label)+".save.json");check(!fs::exists(path),"Preserve checkpoint");
    q::save::write_save(path,c.document(w),c.data,c.mask);const auto document=q::save::read_save(path);auto loaded=restore(c,document);
    check(document.world==w.snapshot()&&loaded.snapshot()==w.snapshot(),"Complete Rule11/12 SaveDocument restore differs");
}
void simulate(const Args& a){
    preflight(a);const auto c=q::Context::real(a.data);check(c.model->provenance().sha256=="7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6","BLOCKED: unchanged pinned Normal model required");
    const bool resumed=a.mode=="resume",output=a.mode!="check";const auto input=resumed?q::f::read_text(a.input):std::string{};
    auto w=resumed?restore(c,q::save::read_save(a.input)):fresh(c,a.rule);check(w.rule_version()==a.rule,"Explicit saved rule differs from request");
    if(output)fs::create_directories(a.output);Json commands=Json::array(),warmup=nullptr,first_goal=nullptr;
    if(!resumed){commands=player::build(w,a.variant,player::origin(c));metrics::Observation observed(w);
        while(w.ticks()<20000){observed.before(w);w.tick();observed.after(w);}
        warmup=observed.report(w);first_goal=warmup.at("first_goal");
    }else check(w.ticks()==20000,"Continuation requires exact earned tick20000 seed");
    q::authority(w);q::worker_oracle(w);const auto maturity=player::maturity(w);
    std::unique_ptr<q::save::RecoveryStore> recovery;std::string history;bool recovery_equal=false;
    if(output){checkpoint(c,a.output,"seed",w);if(resumed)check(q::f::read_text(a.output/"seed.save.json")==input,"Separate process changed literal seed bytes");
        recovery=std::make_unique<q::save::RecoveryStore>(a.output/"recovery-app",c.data,q::save::recovery_storage_budget_bytes,q::save::RecoveryAccess::OriginalProduction);
        history=recovery->create_history(c.document(w),c.mask);
    }
    metrics::Observation observed(w);const auto end=w.ticks()+a.ticks;
    while(w.ticks()<end){observed.before(w);w.tick();observed.after(w);if(output&&w.ticks()==20000+a.ticks/2)checkpoint(c,a.output,"midpoint",w);}
    q::authority(w);q::worker_oracle(w);const auto report=observed.report(w);const auto& summary=report.at("summary");
    if(a.mode=="check")check(maturity.at("ready").get<bool>()&&w.original_goal_status().reached&&
        summary.at("inspector_arrivals").get<unsigned>()>0,"Real paid12-House Food/goods/service/fire acceptance lacks actual maturity/goal/visits");
    if(output){checkpoint(c,a.output,"result",w);const auto entry=recovery->write_checkpoint(history,c.document(w),c.mask);
        check(entry.loadable()&&entry.schema==(a.rule==12?28U:27U)&&entry.rule_version==a.rule,"Real-data recovery changed exact rule/schema");
        auto recovered=restore(c,q::save::read_save(entry.path));check(recovered.snapshot()==w.snapshot(),"Real-data recovery differs from complete earned World");
        check(q::f::read_text(entry.path)==q::f::read_text(a.output/"result.save.json"),"Recovery complete SaveDocument bytes differ from final earned save");
        const auto catalog=recovery->catalog();check(catalog.histories.size()==1&&catalog.histories.front().error.empty(),"Real-data recovery catalog refused its own earned history");recovery_equal=true;
        write(a.output/"metrics.json",report);if(!resumed)write(a.output/"warmup.json",warmup);
        write(a.output/"report.json",{{"format","openemperor-sustainable-economy-process-v1"},{"status","PASS"},{"mode",a.mode},
            {"rule",a.rule},{"schema",a.rule==12?28:27},{"policy",3},{"revision",27},{"snapshot_injection",false},
            {"creator_mode",!resumed},{"restored_live_world",resumed},{"input_preserved",true},
            {"complete_recovery_equality",recovery_equal},
            {"variant",a.variant},{"begin_tick",20000},{"end_tick",w.ticks()},{"upkeep_interval",metrics::interval(w)},
            {"first_goal",first_goal},{"maturity",maturity},{"initial_commands",commands},{"summary",summary},
            {"sustainable_operating_windows",summary.at("two_consecutive_nonnegative_windows")},
            {"minimum_100000_post_maturity_ticks",a.ticks>=100000},{"native_human","NOT_RUN"}});
    }
    if(resumed)check(q::f::read_text(a.input)==input,"Continuation changed protected seed");
    std::cout<<"Rule"<<a.rule<<" "<<a.variant<<" technical PASS "<<summary.dump()<<'\n';
}
struct Sdl{SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;Sdl(){check(SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy")&&SDL_Init(SDL_INIT_VIDEO),SDL_GetError());window=SDL_CreateWindow("OE37 actual SandboxView",1600,1000,SDL_WINDOW_HIDDEN);check(window,SDL_GetError());renderer=SDL_CreateRenderer(window,"software");check(renderer,SDL_GetError());}~Sdl(){if(renderer)SDL_DestroyRenderer(renderer);if(window)SDL_DestroyWindow(window);SDL_Quit();}};
void key(oe::SandboxView& v,SDL_Keycode code){SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=code;check(SDL_PushEvent(&e),SDL_GetError());e={};e.type=SDL_EVENT_KEY_UP;e.key.key=code;check(SDL_PushEvent(&e),SDL_GetError());bool running=true;while(SDL_PollEvent(&e))v.handle_event(e,running);check(running,"View exited unexpectedly");}
void center(oe::SandboxView& v,oe::scene::Point at){for(unsigned i=0;i<1000;++i){const auto map=v.layout().map;const auto cam=v.camera();const double dx=map.x+map.w*.5-at.x*cam.zoom-cam.offset.x,dy=map.y+map.h*.5-at.y*cam.zoom-cam.offset.y;if(std::abs(dx)<.01&&std::abs(dy)<.01)return;oe::SandboxView::PanKeyState held{};if(std::abs(dx)>.01){held.left=dx>0;held.right=dx<0;v.update(std::min(.05,std::abs(dx)/400),held);}else{held.up=dy>0;held.down=dy<0;v.update(std::min(.05,std::abs(dy)/400),held);}}throw std::runtime_error("Actual camera steering failed");}
void render_house(const Args& a,const std::string& input,
        const q::save::SaveDocument& saved,oe::SandboxView& view,SDL_Renderer* renderer,sim::BuildingId target){
    check((a.rule==12
#ifdef OE_MARKET_LOGISTICS_QA
        ||a.rule==13
#endif
        )&&view.world().rule_version()==a.rule,"Focused House capture requires its explicit original economy generation");
    const auto& body=view.world().building(target);const auto& house=q::house(view.world(),target);
    const auto actual_type=static_cast<unsigned>(house.type);
    check(body.placed&&body.kind==sim::Object::Household&&house.occupants>0&&actual_type>=3&&actual_type<=10,
        "Focused capture requires an actually occupied Common House, without a projected promotion");
    check(!view.selected_building(),"Fresh F9 unexpectedly retained a building selection");
    const auto click=[&](float x,float y){
        float window_x=0,window_y=0;
        check(SDL_RenderCoordinatesToWindow(renderer,x,y,&window_x,&window_y),SDL_GetError());
        bool running=true;
        for(const auto type:{SDL_EVENT_MOUSE_MOTION,SDL_EVENT_MOUSE_BUTTON_DOWN,SDL_EVENT_MOUSE_BUTTON_UP}){
            SDL_Event event{};event.type=type;
            if(type==SDL_EVENT_MOUSE_MOTION){event.motion.x=window_x;event.motion.y=window_y;}
            else{event.button.button=SDL_BUTTON_LEFT;event.button.x=window_x;event.button.y=window_y;}
            view.handle_event(event,running);
        }
        check(running,"Focused House input exited the view");
    };
    const auto bodies=view.world().buildings();
    const auto found=std::find_if(bodies.begin(),bodies.end(),[&](const auto& value){return value.id==target;});
    check(found!=bodies.end(),"Focused paid House is missing from the actual building list");
    const auto index=static_cast<int>(std::distance(bodies.begin(),found));
    const auto panel=view.layout().panel;const auto scale=view.layout().scale;
    click(float(panel.x+30*scale),float(panel.y+(46+index*18+3)*scale));
    check(view.selected_building()==target&&view.original_house_inspector_active(),
        "Actual SDL building-list input failed to select the paid House inspector");
    const auto visual=view.original_house_visual_diagnostic(target);
    const auto initial_art=view.original_house_display_stats();
    check(visual.applicable&&visual.configured&&visual.active&&visual.asset&&visual.house_type==actual_type&&
        visual.actual_side==2&&visual.image_width>0&&visual.image_height>0&&
        initial_art.source==oe::VisualProfileSource::Builtin,
        "Focused actual Common House lacks its canonical original sprite");
    const oe::scene::Point image_center{visual.image_origin.x+visual.image_width*.5,
        visual.image_origin.y+visual.image_height*.5};
    check(std::isfinite(image_center.x)&&std::isfinite(image_center.y),"Focused sprite has an invalid production anchor");
    Json captures=Json::array(),sections=Json::object();
    const auto frozen=[&]{
        check(view.paused()&&view.world().snapshot()==saved.world&&q::f::read_text(a.input)==input&&
            q::f::read_text(a.output/"view.save.json")==input,
            "Focused House rendering/input changed complete World or save bytes");
    };
    const auto capture=[&](unsigned zoom,const char* filename,const char* section){
        for(unsigned attempts=0;attempts<3&&view.camera().zoom!=zoom;++attempts)key(view,SDLK_Z);
        check(view.camera().zoom==zoom,"Focused House zoom input failed");
        center(view,image_center);view.update(0);check(view.render(),"Actual focused House render failed");
        const auto art=view.original_house_display_stats();const auto slot=actual_type-2;
        check(art.drawn_instances.at(slot)>0&&art.authored_fallbacks.at(slot)==0,
            "Focused actual House type did not draw original art without an authored fallback");
        const auto file=a.output/filename;
        std::unique_ptr<SDL_Surface,decltype(&SDL_DestroySurface)> image(SDL_RenderReadPixels(renderer,nullptr),SDL_DestroySurface);
        check(image&&SDL_SavePNG(image.get(),file.string().c_str()),"Cannot capture actual focused House renderer");
        captures.push_back({{"file",file.filename().string()},{"zoom",zoom},{"tick",view.world().ticks()},
            {"section",section},{"target_house",static_cast<unsigned>(target)},{"actual_type",actual_type},
            {"width",image->w},{"height",image->h}});
        frozen();
    };
    check(view.original_house_section()==oe::SandboxView::OriginalHouseSection::State,
        "Fresh selected House did not open its actual State section");
    const auto inspection=view.original_house_section_lines();sections["State"]=inspection;
    const auto has_original=std::any_of(inspection.begin(),inspection.end(),[](const auto& line){
        return line.find("Art: original")!=std::string::npos;});
    check(has_original,"Focused State inspector failed to identify its actual original art");
    capture(1,"sandbox-1x.png","State");capture(2,"sandbox-2x.png","State");capture(4,"sandbox-4x.png","State");
    for(const auto section:{oe::SandboxView::OriginalHouseSection::Needs,oe::SandboxView::OriginalHouseSection::Goods,
            oe::SandboxView::OriginalHouseSection::Visits}){
        const auto rect=view.original_house_section_rect(section);
        check(rect.w>0&&rect.h>0,"Focused House section tab is not visible");
        click(float(rect.x)+float(rect.w)*.5F,float(rect.y)+float(rect.h)*.5F);
        check(view.original_house_section()==section&&view.selected_building()==target,
            "Actual SDL House tab input changed selection or missed the requested section");
        const std::string name(oe::original_house_ui::section_name(section));sections[name]=view.original_house_section_lines();
        const char* file=section==oe::SandboxView::OriginalHouseSection::Needs?"sandbox-needs-2x.png":
            section==oe::SandboxView::OriginalHouseSection::Goods?"sandbox-goods-2x.png":"sandbox-visits-2x.png";
        capture(2,file,name.c_str());
    }
    key(view,SDLK_F5);frozen();const auto art=view.original_house_display_stats();
    const auto slot=actual_type-2;
    write(a.output/"report.json",{{"format","openemperor-high-tier-view-v1"},{"status","PASS"},
        {"rule",a.rule},{"schema",a.rule==13?29:28},{"policy",3},{"revision",27},{"tick",view.world().ticks()},
        {"target_house",static_cast<unsigned>(target)},{"actual_type",actual_type},{"actual_sandbox_renderer",true},
        {"backend",SDL_GetRendererName(renderer)},{"complete_f5_f9_equality",true},{"world_unchanged",true},
        {"input_preserved",true},{"selected_via_sdl_list",true},{"sections_via_sdl_input",true},
        {"actual_state_only",true},{"original_house_art",{{"configured",visual.configured},{"active",visual.active},
            {"house_type",visual.house_type},{"actual_side",visual.actual_side},{"source",oe::visual_profile_source_name(art.source)},
            {"archive",visual.asset->archive_relative_path.generic_string()},{"image_index",visual.asset->image_index},
            {"selection",visual.selection},{"evidence",visual.evidence},{"image_width",visual.image_width},
            {"image_height",visual.image_height},{"ground",{visual.ground.x,visual.ground.y}},
            {"anchor",{visual.ground_x,visual.ground_y}},{"image_origin",{visual.image_origin.x,visual.image_origin.y}},
            {"original_draws",art.drawn_instances.at(slot)},{"authored_fallbacks",art.authored_fallbacks.at(slot)}}},
        {"inspection",inspection},{"sections",sections},{"captures",captures},{"native_human","NOT_RUN"}});
}
void render(const Args& a,std::optional<sim::BuildingId> focus_house=std::nullopt){
    preflight(a);const auto c=q::Context::real(a.data);const auto input=q::f::read_text(a.input);const auto d=q::save::read_save(a.input);check(d.world.rule_version==a.rule,"Render requires exact explicit saved rule");auto restored=restore(c,d);q::authority(restored);
    fs::create_directories(a.output);const auto target=a.output/"view.save.json";q::f::write_text(target,input);Sdl sdl;
    auto session=oe::maps::load_stored_map_session(c.data,c.map_relative,oe::maps::FootprintPolicy::EdgeByte4x4Preview,oe::maps::StoredGraphicsProfile::Slot8);const oe::maps::MapGeometry geometry(session.map.declared_map_size);
    auto v=[&] {
#ifdef OE_MARKET_LOGISTICS_QA
        if(a.rule==13)return oe::SandboxView::create_original_market_logistics(std::move(session),oe::original::Difficulty::Normal,false);
#endif
        if(a.rule==11)return oe::SandboxView::create_original_fire_protection(std::move(session),oe::original::Difficulty::Normal,false);
#ifdef OE37_BASELINE
        throw std::invalid_argument("Frozen OE36 renderer does not implement Rule12");
#else
        return oe::SandboxView::create_original_sustainable_economy(std::move(session),oe::original::Difficulty::Normal,false);
#endif
    }();v->configure_save(c.data,c.map_relative,target);
    const auto p=oe::detect_and_select_visual_profiles(c.data,a.resources);check(!p.original_house.empty(),"Actual original House art unavailable");
    if(!p.walker.empty())v->set_walker_visuals(p.walker,p.walker_source);if(!p.building.empty())v->set_building_visuals(p.building,p.building_source);if(!p.road.empty())v->set_road_visuals(p.road,p.road_source);
    v->set_fire_visuals(p.fire,p.fire_source,p.fire_fallback_reason);v->set_fire_inspector_visuals(p.fire_inspector,p.fire_inspector_source,p.fire_inspector_fallback_reason);
    v->set_market_walker_visuals(p.market_walker,p.market_walker_source,p.market_walker_fallback_reason);v->set_service_walker_visuals(p.service_walker,p.service_walker_source,p.service_walker_fallback_reason);v->set_health_walker_visuals(p.health_walker,p.health_walker_source,p.health_walker_fallback_reason);v->set_original_house_visuals(p.original_house,p.original_house_source,p.original_house_fallback_reason);
    v->initialize(sdl.window,sdl.renderer);key(*v,SDLK_F9);check(v->paused()&&v->world().snapshot()==d.world,"Actual Sandbox F9 differs: "+v->last_message());key(*v,SDLK_F5);check(q::f::read_text(target)==input,"Actual Sandbox F5 bytes differ");
    SDL_Event focus{};focus.type=SDL_EVENT_WINDOW_FOCUS_GAINED;bool running=true;v->handle_event(focus,running);
    if(focus_house){render_house(a,input,d,*v,sdl.renderer,*focus_house);v->shutdown();return;}
    const auto active=std::find_if(v->world().couriers().begin(),v->world().couriers().end(),[](const auto& actor){return actor.role==sim::CourierRole::FireInspector;});
    check(active!=v->world().couriers().end(),"Earned render checkpoint lacks its paid FireInspector");
    const auto owner=active->owner;const auto owner_index=static_cast<std::size_t>(std::distance(v->world().buildings().begin(),std::find_if(v->world().buildings().begin(),v->world().buildings().end(),[&](const auto& b){return b.id==owner;})));
    const auto panel=v->layout().panel;const auto scale=v->layout().scale;float window_x=0,window_y=0;
    check(SDL_RenderCoordinatesToWindow(sdl.renderer,float(panel.x+30*scale),float(panel.y+(46+static_cast<int>(owner_index)*18+3)*scale),&window_x,&window_y),SDL_GetError());
    for(const auto type:{SDL_EVENT_MOUSE_MOTION,SDL_EVENT_MOUSE_BUTTON_DOWN,SDL_EVENT_MOUSE_BUTTON_UP}){SDL_Event e{};e.type=type;if(type==SDL_EVENT_MOUSE_MOTION){e.motion.x=window_x;e.motion.y=window_y;}else{e.button.button=SDL_BUTTON_LEFT;e.button.x=window_x;e.button.y=window_y;}v->handle_event(e,running);}
    check(v->selected_building()==owner,"Actual building-list input did not select the paid Watch");
    const auto at=active->path.empty()?v->world().building(owner).cell:active->path.at(active->path_vertex);const auto height=v->world().map_permissions()->cell_height(at);
    const oe::scene::Point point{double(at.x-at.y)*40,double(at.x+at.y-2*static_cast<int>(geometry.border))*20-double(height)*40};Json captures=Json::array();
    for(unsigned zoom=1;zoom<=4;zoom*=2){while(v->camera().zoom!=zoom)key(*v,SDLK_Z);center(*v,zoom==1?oe::scene::Point{point.x+320,point.y+200}:point);v->update(0);check(v->render(),"Production Sandbox render failed");const auto file=a.output/("sandbox-"+std::to_string(zoom)+"x.png");std::unique_ptr<SDL_Surface,decltype(&SDL_DestroySurface)> image(SDL_RenderReadPixels(sdl.renderer,nullptr),SDL_DestroySurface);check(image&&SDL_SavePNG(image.get(),file.string().c_str()),"Cannot capture actual Sandbox renderer");captures.push_back({{"file",file.filename().string()},{"zoom",zoom},{"tick",v->world().ticks()}});}
    check(v->world().snapshot()==d.world&&q::f::read_text(a.input)==input,"Rendering mutated live authority/input");
    const auto inspection=v->inspection_lines();std::string inspection_text;
    for(const auto& line:inspection){if(!inspection_text.empty())inspection_text+=' ';inspection_text+=line;}
    // Independently bind the photographed Watch caption to the selected contract.
    // Reading World's helper here would let a shared presentation error pass.
    const unsigned expected_interval=a.rule==12?816U:400U;
    const auto expected_caption="Maintenance: 4 every "+std::to_string(expected_interval)+" ticks";
    check(inspection_text.find(expected_caption)!=std::string::npos,
        "Selected paid Watch inspection has the wrong versioned upkeep caption: "+inspection_text);
    write(a.output/"report.json",{{"format","openemperor-sustainable-economy-view-v1"},{"status","PASS"},{"rule",a.rule},{"schema",a.rule==12?28:27},{"actual_sandbox_renderer",true},{"backend",SDL_GetRendererName(sdl.renderer)},{"complete_f5_f9_equality",true},{"selected_watch",static_cast<unsigned>(owner)},{"inspection",inspection},{"maintenance_caption_verified",true},{"maintenance_interval",expected_interval},{"captures",captures},{"native_human","NOT_RUN"}});v->shutdown();
}
}
int main(int argc,char** argv){if(argc==2&&std::string_view(argv[1])=="--help"){std::cout<<usage<<'\n';return 0;}try{const auto a=arguments(argc,argv);if(a.mode=="render")render(a);else simulate(a);return 0;}catch(const std::exception& e){std::cerr<<"Sustainable economy FAIL: "<<e.what()<<'\n';return 1;}}
