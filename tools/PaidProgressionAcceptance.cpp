// OE34: observe only ordinary paid commands/ticks and production persistence/View.
#include "../tests/PaidProgressionMetrics.h"
#include "app/SandboxView.h"
#include "app/VisualSelection.h"

#include <SDL3/SDL.h>
#include <algorithm>
#include <charconv>
#include <cmath>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string_view>

namespace {
namespace q=oe_starter_fixture;
namespace sim=openemperor::simulation;
namespace oe=openemperor;
namespace fs=std::filesystem;
namespace metrics=oe_paid_progression_metrics;
using Json=nlohmann::json;
using q::check;
constexpr const char* model_sha="7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6";
constexpr const char* usage="paid-progression --mode create|resume|render|check --data DATA [--output NEW_OE34_DIRECTORY] [--input OE34_SAVE] [--plan direct|connected|food|supplied] [--ticks 1..100000] [--hold 1..100000] [--resources ROOT]";
struct Args {std::string mode,plan="supplied";fs::path data,out,input,resources;unsigned ticks=20000,hold=20000;};
bool below(const fs::path& child,const fs::path& parent) {
    auto c=child.begin();for(auto p=parent.begin();p!=parent.end();++p,++c)if(c==child.end()||*c!=*p)return false;
    return true;
}
void safe(const fs::path& path) {
    fs::path prefix;check(path.is_absolute(),"Expected absolute proof path");
    for(const auto& part:path) {
        check(part!=".."&&part!=".","Proof paths must be canonical");prefix/=part;
        std::error_code error;const auto status=fs::symlink_status(prefix,error);
        check(!error||error==std::errc::no_such_file_or_directory,"Cannot inspect proof path");
        check(!fs::is_symlink(status),"Proof path traverses a symlink");
    }
}
fs::path private_root() {return fs::canonical(fs::path(__FILE__).parent_path().parent_path())/".local/original-economy/oe34";}
Args arguments(int argc,char** argv) {
    std::map<std::string,std::string> values;
    for(int i=1;i<argc;i+=2) {
        check(i+1<argc,usage);const std::string key=argv[i];
        check(key=="--mode"||key=="--data"||key=="--output"||key=="--input"||key=="--plan"||key=="--ticks"||key=="--hold"||key=="--resources",usage);
        check(values.emplace(key,argv[i+1]).second,"Duplicate argument");
    }
    check(values.contains("--mode")&&values.contains("--data"),usage);Args a;
    a.mode=values.at("--mode");check(a.mode=="create"||a.mode=="resume"||a.mode=="render"||a.mode=="check",usage);
    const auto path=[&](const char* name){return values.contains(name)?fs::absolute(values.at(name)):fs::path{};};
    a.data=path("--data");a.out=path("--output");a.input=path("--input");a.resources=path("--resources");
    if(values.contains("--plan"))a.plan=values.at("--plan");
    check(a.plan=="direct"||a.plan=="connected"||a.plan=="food"||a.plan=="supplied",usage);
    for(const auto& [key,destination]:{std::pair{"--ticks",&a.ticks},{"--hold",&a.hold}})if(values.contains(key)) {
        const auto& text=values.at(key);const auto parsed=std::from_chars(text.data(),text.data()+text.size(),*destination);
        check(parsed.ec==std::errc{}&&parsed.ptr==text.data()+text.size()&&*destination>0&&*destination<=100000,usage);
    }
    return a;
}
void preflight(const Args& a) {
    safe(a.data);check(fs::is_directory(a.data),"BLOCKED: original installation missing");
    check(fs::is_regular_file(a.data/"Cities/Xia.map")&&fs::is_regular_file(a.data/"Model/EmperorBuildingModels.txt"),"BLOCKED: original Xia/model missing");
    if(a.mode!="check") {
        safe(a.out);const auto root=private_root();
        check(a.out!=root&&below(a.out,root)&&!fs::exists(a.out),"Output must be a new private OE34 directory");
        check(!below(a.out,a.data)&&!below(a.data,a.out),"Output overlaps original data");
        q::save::validate_save_target(a.out/"report.json",a.data);
    }
    if(a.mode=="resume"||a.mode=="render") {
        safe(a.input);check(below(a.input,private_root())&&fs::is_regular_file(a.input)&&!below(a.input,a.out),"Explicit private OE34 input required");
    }else check(a.input.empty(),"Creator/check cannot import a save");
    if(a.mode=="render") {safe(a.resources);check(fs::is_directory(a.resources),"Render requires actual compatibility resources");}
}
void write(const fs::path& path,const Json& value) {
    safe(path);check(!fs::exists(path),"Preserve existing proof artifact");q::f::write_text(path,value.dump(2)+"\n");
}
Json goal(const sim::World& w) {
    const auto g=w.original_goal_status();
    return {{"reached",g.reached},{"inhabited_houses",g.inhabited_houses},{"common_plain_or_higher",g.common_houses},
        {"population",g.population},{"positive_tax",g.positive_tax},{"nonnegative_funds",g.nonnegative_funds},
        {"tick",w.ticks()},{"funds",w.treasury()},{"taxes",w.taxes_collected_total()}};
}
void checkpoint(const q::Context& context,const fs::path& out,const std::string& label,const sim::World& w) {
    const auto document=context.document(w);const auto path=out/(label+".save.json");
    check(!fs::exists(path),"Preserve checkpoint");q::save::write_save(path,document,context.data,context.mask);
    const auto read=q::save::read_save(path);auto restored=context.restore(read);
    check(read.world==w.snapshot()&&restored.snapshot()==w.snapshot(),"Complete paid checkpoint restore differs");
}
Json fire_refusal(sim::World& w,sim::Cell origin) {
    const sim::Cell cell{origin.x,origin.y+5};
    check(w.buildable(cell)&&w.object_at(cell)==sim::Object::Empty&&w.treasury()>=80,"FireWatch control lacks legal funded spare ground");
    const auto before=w.snapshot();const sim::Command command{sim::CommandType::PlaceFireWatch,cell};
    const auto valid=w.validate(command),actual=w.execute(command);
    const std::string reason="Original production permits only its explicitly implemented paid chains";
    check(!valid.accepted&&!actual.accepted&&!actual.changed&&valid.reason==reason&&actual.reason==reason&&w.snapshot()==before,
        "Rule10 FireWatch refusal or atomicity changed; review its versioned contract");
    return {{"tick",w.ticks()},{"cell",{cell.x,cell.y}},{"reason",actual.reason},{"complete_snapshot_unchanged",true}};
}
Json expand(sim::World& w,const sim::OriginalEconomyStarterReceipt& receipt,const std::string& plan) {
    Json commands=Json::array();
    const auto buy=[&](sim::Command command,const char* name) {
        const auto relative=command.cell;command.cell=q::offset(receipt,relative.x,relative.y);
        const auto before=w.treasury();const auto sequence=w.command_sequence();const auto paid=w.construction_spent_total();
        const auto validated=w.validate(command);check(validated.accepted,"Paid plan validation refused: "+validated.reason);
        const auto result=w.execute(command);check(result.accepted&&result.changed,"Paid plan command refused: "+result.reason);
        check(w.command_sequence()==sequence+1&&w.treasury()>=0&&w.treasury()<=before&&
            w.construction_spent_total()-paid==static_cast<std::uint64_t>(before-w.treasury()),"Paid plan bypassed actual funds/accounting");
        q::authority(w);Json row{{"command",name},{"type",static_cast<unsigned>(command.type)},{"tick",w.ticks()},
            {"relative_cell",{relative.x,relative.y}},{"cell",{command.cell.x,command.cell.y}},
            {"funds_before",before},{"funds_after",w.treasury()},{"cost",before-w.treasury()}};
        if(const auto id=w.building_owner_at(command.cell))row["building_id"]=static_cast<unsigned>(*id);
        commands.push_back(std::move(row));
        return w.building_owner_at(command.cell);
    };
    buy({sim::CommandType::PlaceHousehold,{12,2}},"Household");
    buy({sim::CommandType::PlaceHousehold,{15,2}},"Household");
    if(plan!="direct")for(int y=5;y<=8;++y)buy({sim::CommandType::PlaceRoad,{2,y}},"Road");
    if(plan=="food"||plan=="supplied") {
        const auto farm=buy({sim::CommandType::PlaceOriginalFoodFarm,{9,10}},"FoodFarm");check(farm.has_value(),"Paid Farm owner missing");
        sim::Command field{sim::CommandType::PlaceOriginalMilletField,{12,10}};field.building_id=*farm;buy(field,"MilletField");
    }
    if(plan=="supplied") {
        const auto farm=buy({sim::CommandType::PlaceOriginalHempFarm,{14,10}},"HempFarm");check(farm.has_value(),"Paid Hemp Farm owner missing");
        sim::Command field{sim::CommandType::PlaceOriginalHempField,{16,10}};field.building_id=*farm;buy(field,"HempField");
    }
    return commands;
}
void save_metrics(const fs::path& out,const metrics::Metrics& m) {
    write(out/"evaluations.json",m.events());write(out/"evaluation-frames.json",m.evaluation_frames());write(out/"transport.json",m.transport_events());
}
void run(const Args& a) {
    preflight(a);const auto context=q::Context::real(a.data);
    check(context.model->provenance().sha256==model_sha,"Acceptance requires the pinned unchanged Normal model");
    const bool resume=a.mode=="resume";const auto input=resume?q::f::read_text(a.input):std::string{};
    auto world=resume?context.restore(q::save::read_save(a.input)):context.world();
    const bool output=a.mode!="check";if(output)fs::create_directories(a.out);
    Json purchases=Json::array(),refusal=nullptr;sim::OriginalEconomyStarterReceipt receipt;
    if(!resume) {
        receipt=sim::place_original_economy_starter(world);q::paid_empty(context,world,receipt);
        if(output)checkpoint(context,a.out,"starter",world);
        purchases=expand(world,receipt,a.plan);refusal=fire_refusal(world,receipt.origin);
        if(a.plan=="supplied")check(world.construction_spent_total()==1670&&world.treasury()==10330&&
            world.command_sequence()==97&&world.buildings().size()==25&&world.couriers().size()==17,
            "Paid supplied plan cost/command/body identity differs from the pinned Normal contract");
        if(output)checkpoint(context,a.out,"expanded",world);
    }else {
        check(world.snapshot()==q::save::read_save(a.input).world,"Separate process changed the complete input snapshot");
        checkpoint(context,a.out,"loaded",world);
        check(q::f::read_text(a.out/"loaded.save.json")==input,"Separate process reserialized a different complete SaveDocument");
    }
    q::authority(world);const auto begin=world.ticks();metrics::Metrics observed(world);
    std::unique_ptr<metrics::Metrics> held;
    std::optional<std::uint64_t> first;Json at_goal=nullptr,first_fire=nullptr,first_loss=nullptr,goal_changes=Json::array();
    std::uint64_t goal_ticks=0,goal_lost_ticks=0,held_run=0,longest_run=0;
    bool previous=world.original_goal_status().reached;
    if(resume) {check(previous,"Continuation input has not reached the actual goal");first=world.ticks();at_goal=goal(world);held=std::make_unique<metrics::Metrics>(world);}
    const auto limit=resume?world.ticks()+a.ticks:std::uint64_t(a.ticks)+a.hold;
    while(world.ticks()<limit) {
        observed.before(world);if(held)held->before(world);world.tick();q::authority(world);observed.after(world);if(held)held->after(world);
        const bool reached=world.original_goal_status().reached;
        if(reached!=previous) {
            auto transition=goal(world);Json burning=Json::array();
            for(const auto& b:world.buildings())if(world.building_on_fire(b.id))burning.push_back(static_cast<unsigned>(b.id));
            transition["burning_ids"]=burning;
            if(previous&&!reached&&first_loss.is_null())first_loss=transition;
            goal_changes.push_back(std::move(transition));previous=reached;
        }
        if(first_fire.is_null())for(const auto& b:world.buildings())if(world.building_on_fire(b.id)) {
            first_fire=Json{{"tick",world.ticks()},{"first_burning_id",static_cast<unsigned>(b.id)},
                {"staffing",world.workforce_used()},{"goal",goal(world)}};
            if(output&&!resume)checkpoint(context,a.out,"first-fire",world);break;
        }
        if(!first&&reached) {
            first=world.ticks();at_goal=goal(world);if(output)checkpoint(context,a.out,"goal",world);
            held=std::make_unique<metrics::Metrics>(world);
        }else if(first) {
            if(reached){++goal_ticks;++held_run;longest_run=std::max(longest_run,held_run);}else{++goal_lost_ticks;held_run=0;}
        }
        if(first&&world.ticks()>=*first+(resume?a.ticks:a.hold))break;
        if(!first&&world.ticks()>=a.ticks)break;
    }
    q::worker_oracle(world);const auto measured=observed.report(world);
    const auto continuation=held?held->report(world):Json{};
    const bool reached=first.has_value();const bool complete=reached&&world.ticks()==*first+(resume?a.ticks:a.hold);
    const bool supply_gaps=measured.at("actual_food_missed_demands").get<std::uint64_t>()>0||
        measured.at("unmet_current_evaluations").get<std::uint64_t>()>0;
    Json blocker=nullptr;
    if(!reached)blocker={{"category","goal_not_reached"},{"bound",a.ticks},{"state",goal(world)}};
    else if(!first_loss.is_null())blocker={{"category",!first_loss.at("burning_ids").empty()?"natural_fire_without_admitted_protection":
        !first_loss.at("nonnegative_funds").get<bool>()?"negative_funds":"live_goal_condition_lost"},{"state",first_loss}};
    Json report{{"format","openemperor-paid-progression-v1"},{"status",!complete?"FAIL":goal_lost_ticks||supply_gaps?"PARTIAL":"PASS"},
        {"goal_status",reached?"PASS":"FAIL"},{"continuity_status",goal_lost_ticks?"PARTIAL":"PASS"},
        {"supply_status",supply_gaps?"PARTIAL":"PASS"},{"mode",a.mode},{"plan",a.plan},{"begin_tick",begin},{"end_tick",world.ticks()},
        {"first_goal_tick",first?Json(*first):Json(nullptr)},{"first_goal",at_goal},{"final_goal",goal(world)},
        {"goal_ticks_in_continuation",goal_ticks},{"goal_lost_ticks_in_continuation",goal_lost_ticks},
        {"longest_continuous_goal_ticks",longest_run},{"first_natural_fire",first_fire},{"goal_changes",goal_changes},
        {"fire_watch_control",refusal},{"expansion_commands",purchases},{"metrics",measured},{"continuation_metrics",continuation},
        {"input_preserved",true},{"rule",10},{"policy",3},{"schema",26},{"revision",27},{"snapshot_injection",false},
        {"native_human","NOT_RUN"},{"first_blocker",blocker}};
    if(output) {checkpoint(context,a.out,"result",world);save_metrics(a.out,observed);write(a.out/"report.json",report);}
    if(resume)check(q::f::read_text(a.input)==input,"Continuation rewrote its input");
    check(complete,"Paid goal or declared continuation was not achieved; inspect report");
    if(a.mode=="check") {
        check(world.rule_version()==10&&world.original_production_state()->development_revision==27&&world.map_permissions()->policy_version()==3,
            "Paid progression changed authority versions");
        check(world.treasury()>=0&&world.total_population()>=50,"Regression lost paid funds/population");
        check(refusal.at("complete_snapshot_unchanged")==true,"Regression missed actual FireWatch refusal");
        check(!first_fire.is_null(),"Regression did not exercise natural Rule10 fire");
        check(goal_lost_ticks>0,"Regression hid temporary loss of the live goal during fire");
    }
    std::cout<<Json{{"status",report["status"]},{"goal",at_goal},{"final",goal(world)},
        {"food_misses",measured["actual_food_missed_demands"]},{"unmet_current",measured["unmet_current_evaluations"]},
        {"goal_lost_ticks",goal_lost_ticks}}.dump()<<'\n';
}
struct Sdl {
    SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;
    Sdl() {
        check(SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy")&&SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
        window=SDL_CreateWindow("Paid progression actual Sandbox renderer",1600,1000,SDL_WINDOW_HIDDEN);check(window,SDL_GetError());
        renderer=SDL_CreateRenderer(window,"software");check(renderer,SDL_GetError());
    }
    ~Sdl(){if(renderer)SDL_DestroyRenderer(renderer);if(window)SDL_DestroyWindow(window);SDL_Quit();}
};
void key(oe::SandboxView& v,SDL_Keycode code) {
    SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=code;check(SDL_PushEvent(&e),SDL_GetError());
    e={};e.type=SDL_EVENT_KEY_UP;e.key.key=code;check(SDL_PushEvent(&e),SDL_GetError());bool running=true;
    while(SDL_PollEvent(&e))v.handle_event(e,running);check(running,"View input exited unexpectedly");
}
void profiles(oe::SandboxView& v,const fs::path& data,const fs::path& resources) {
    const auto p=oe::detect_and_select_visual_profiles(data,resources);
    check(!p.original_house.empty(),"Actual original House profile unavailable");
    if(!p.walker.empty())v.set_walker_visuals(p.walker,p.walker_source);
    if(!p.building.empty())v.set_building_visuals(p.building,p.building_source);
    if(!p.road.empty())v.set_road_visuals(p.road,p.road_source);
    v.set_fire_visuals(p.fire,p.fire_source,p.fire_fallback_reason);
    v.set_fire_inspector_visuals(p.fire_inspector,p.fire_inspector_source,p.fire_inspector_fallback_reason);
    v.set_market_walker_visuals(p.market_walker,p.market_walker_source,p.market_walker_fallback_reason);
    v.set_service_walker_visuals(p.service_walker,p.service_walker_source,p.service_walker_fallback_reason);
    v.set_health_walker_visuals(p.health_walker,p.health_walker_source,p.health_walker_fallback_reason);
    v.set_original_house_visuals(p.original_house,p.original_house_source,p.original_house_fallback_reason);
}
void center(oe::SandboxView& v,oe::scene::Point at) {
    for(unsigned i=0;i<1000;++i) {
        const auto map=v.layout().map;const auto camera=v.camera();
        const oe::scene::Point desired{map.x+map.w*.5-at.x*camera.zoom,map.y+map.h*.5-at.y*camera.zoom};
        const auto dx=desired.x-camera.offset.x,dy=desired.y-camera.offset.y;
        if(std::abs(dx)<.01&&std::abs(dy)<.01)return;
        oe::SandboxView::PanKeyState held{};
        if(std::abs(dx)>.01){held.left=dx>0;held.right=dx<0;v.update(std::min(.05,std::abs(dx)/400),held);}
        else{held.up=dy>0;held.down=dy<0;v.update(std::min(.05,std::abs(dy)/400),held);}
    }
    throw std::runtime_error("Actual camera steering failed");
}
void render(const Args& a) {
    preflight(a);const auto context=q::Context::real(a.data);const auto input=q::f::read_text(a.input);
    const auto document=q::save::read_save(a.input);auto earned=context.restore(document);q::authority(earned);
    fs::create_directories(a.out);const auto target=a.out/"view.save.json";q::f::write_text(target,input);
    Sdl sdl;auto session=oe::maps::load_stored_map_session(context.data,context.map_relative,
        oe::maps::FootprintPolicy::EdgeByte4x4Preview,oe::maps::StoredGraphicsProfile::Slot8);
    const oe::maps::MapGeometry geometry(session.map.declared_map_size);
    auto view=oe::SandboxView::create_original_production(std::move(session),oe::original::Difficulty::Normal,false);
    view->configure_save(context.data,context.map_relative,target);profiles(*view,context.data,a.resources);
    view->initialize(sdl.window,sdl.renderer);key(*view,SDLK_F9);
    check(view->paused()&&view->world().snapshot()==document.world,"Actual Sandbox F9 did not restore the complete paid state: "+view->last_message());
    key(*view,SDLK_F5);check(q::f::read_text(target)==input,"Actual Sandbox F5 changed the complete saved document");
    SDL_Event focus{};focus.type=SDL_EVENT_WINDOW_FOCUS_GAINED;bool running=true;view->handle_event(focus,running);
    const auto& house=view->world().original_economy_state()->houses.front();const auto cell=view->world().building(house.building_id).cell;
    const auto height=view->world().map_permissions()->cell_height(cell);
    const oe::scene::Point point{double(cell.x-cell.y)*40,double(cell.x+cell.y-2*static_cast<int>(geometry.border))*20-double(height)*40};
    Json captures=Json::array();
    for(unsigned zoom=1;zoom<=4;zoom*=2) {
        while(view->camera().zoom!=zoom)key(*view,SDLK_Z);
        center(*view,zoom==1?oe::scene::Point{point.x+320,point.y+200}:point);view->update(0);check(view->render(),"Actual Sandbox rendering failed");
        const auto path=a.out/("sandbox-"+std::to_string(zoom)+"x.png");
        std::unique_ptr<SDL_Surface,decltype(&SDL_DestroySurface)> image(SDL_RenderReadPixels(sdl.renderer,nullptr),SDL_DestroySurface);
        check(image&&SDL_SavePNG(image.get(),path.string().c_str()),"Cannot capture actual Sandbox renderer");
        captures.push_back({{"file",path.filename().string()},{"zoom",zoom},{"width",image->w},{"height",image->h},{"tick",view->world().ticks()}});
    }
    check(view->world().snapshot()==document.world&&q::f::read_text(a.input)==input,"Rendering mutated authority/input");
    write(a.out/"report.json",{{"format","openemperor-paid-progression-view-v1"},{"status","PASS"},{"input_preserved",true},
        {"complete_f5_f9_equality",true},{"actual_sandbox_renderer",true},{"backend",SDL_GetRendererName(sdl.renderer)},
        {"native_human","NOT_RUN"},{"captures",captures},{"goal",goal(view->world())}});
    view->shutdown();std::cout<<"Actual Sandbox captures and literal F5/F9 equality PASS\n";
}
}
int main(int argc,char** argv) {
    if(argc==2&&std::string_view(argv[1])=="--help"){std::cout<<usage<<'\n';return 0;}
    try {const auto a=arguments(argc,argv);if(a.mode=="render")render(a);else run(a);return 0;}
    catch(const std::exception& e){std::cerr<<"Paid progression FAIL: "<<e.what()<<'\n';return 1;}
}
