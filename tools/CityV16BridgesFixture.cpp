// Original-map paid gameplay and persistence; explicit, excluded local runner.
#include "app/SandboxView.h"
#include "app/VisualSelection.h"
#include "maps/SandboxPlacement.h"
#include "simulation/RoadConnectivity.h"
#include <SDL3/SDL.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <array>
#include <deque>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
namespace oe=openemperor;namespace s=oe::simulation;namespace m=oe::maps;namespace p=oe::persistence;
namespace fs=std::filesystem;using J=nlohmann::json;
namespace {
void check(bool ok,const std::string& reason){if(!ok)throw std::runtime_error(reason);}
J point(s::Cell c){return J::array({c.x,c.y});}
void write(const fs::path& file,const J& value){std::ofstream out(file);check(bool(out<<value.dump(2)<<'\n'),"private report write failed");}
J read(const fs::path& file){std::ifstream in(file);J value;in>>value;return value;}
constexpr std::array<s::Cell,4> directions={s::Cell{0,-1},{-1,0},{1,0},{0,1}};
struct Sdl {
    SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;
    Sdl(){SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy");check(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
        window=SDL_CreateWindow("Bridge local acceptance",1280,720,SDL_WINDOW_HIDDEN);check(window,SDL_GetError());
        renderer=SDL_CreateRenderer(window,"software");check(renderer,SDL_GetError());}
    ~Sdl(){SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();}
};
struct Context {
    fs::path data,map;std::vector<std::uint8_t> mask;std::shared_ptr<const s::MapPermissions> policy;
    Context(const fs::path& root,const fs::path& relative,SDL_Renderer* renderer):data(root),map(relative){
        auto session=m::load_stored_map_session(data,map,m::FootprintPolicy::EdgeByte4x4Preview,m::StoredGraphicsProfile::Slot8,m::GreatWallPresentationMode::Automatic);
        const m::MapGeometry geometry{session.map.declared_map_size};oe::StoredGraphicsRenderer background{std::move(session.plan)};background.initialize(renderer);
        mask=m::make_sandbox_buildable_mask(background.plan(),geometry);
        policy=m::load_sandbox_map_permissions(session.map,background.plan(),geometry,mask,3,session.input_sha256);
    }
    void save(const s::World& world,const fs::path& path)const{
        const auto doc=p::make_document(data,map,mask,world);p::write_save(path,doc,data,mask);
        const auto loaded=p::read_save(path);auto restored=p::restore_save(loaded,data,mask);
        check(restored.snapshot()==world.snapshot(),"complete saved World differs after restore");
        check(loaded.map_relative==doc.map_relative&&loaded.map_sha256==doc.map_sha256&&loaded.buildable_sha256==doc.buildable_sha256&&
            loaded.map_permissions_policy_version==3&&loaded.map_permissions_sha256==doc.map_permissions_sha256&&loaded.source_schema_version==21,
            "complete SaveDocument identity differs after restore");
    }
    void checkpoint(const s::World& world,const fs::path& path)const {
        save(world,path);auto control=s::World::restore(world.snapshot(),policy);
        for(int tick=0;tick<100;++tick)control.tick();
        save(control,path.parent_path()/(path.stem().string()+"-plus100-control.json"));
    }
};
struct Builder {
    s::World world; s::Cell a,b;bool axis_x;J commands=J::array();
    Builder(std::shared_ptr<const s::MapPermissions> policy,s::Cell first,s::Cell last):world(policy,s::RulesProfile::CityV16,5),a(first),b(last),axis_x(a.x!=b.x){}
    bool side(s::Cell cell,bool far)const{return axis_x?(far?cell.x>=b.x:cell.x<=a.x):(far?cell.y>=b.y:cell.y<=a.y);}
    bool execute(s::Command command){const auto result=world.execute(command);if(!result.accepted)return false;
        commands.push_back({{"tick",world.ticks()},{"type",unsigned(command.type)},{"cell",point(command.cell)},
            {"bridge_end",point(command.bridge_end)},{"building_id",std::uint32_t(command.building_id)},
            {"enabled",command.operating_enabled},{"changed",result.changed},{"funds",world.treasury()}});return true;}
    std::optional<std::vector<s::Cell>> connection(s::Cell origin,s::Object kind,bool far)const {
        const auto bank=far?b:a;const auto footprint=s::building_footprint_cells(world.profile(),world.rule_version(),kind,origin);
        const auto excluded=[&](s::Cell c){return std::find(footprint.begin(),footprint.end(),c)!=footprint.end();};
        std::set<std::pair<int,int>> goals;
        for(const auto cell:footprint)for(const auto d:directions){const s::Cell q{cell.x+d.x,cell.y+d.y};
            if(!excluded(q)&&side(q,far)&&world.road_buildable(q)&&world.map_permissions()->cell_height(q)==world.map_permissions()->cell_height(bank)&&
                (world.object_at(q)==s::Object::Empty||world.road_cell(q)))goals.emplace(q.x,q.y);}
        std::vector<int> previous(std::size_t(world.width()*world.height()),-1);std::deque<s::Cell> queue;
        const auto idx=[&](s::Cell c){return c.y*world.width()+c.x;};previous[std::size_t(idx(bank))]=idx(bank);queue.push_back(bank);
        while(!queue.empty()) {
            const auto cell=queue.front();queue.pop_front();
            if(goals.contains({cell.x,cell.y})) {
                std::vector<s::Cell> path;int current=idx(cell);
                for(;;){path.push_back({current%world.width(),current/world.width()});if(current==idx(bank))break;current=previous[std::size_t(current)];}
                std::reverse(path.begin(),path.end());return path;
            }
            for(const auto d:directions){const s::Cell q{cell.x+d.x,cell.y+d.y};
                if(!world.in_bounds(q)||!side(q,far)||excluded(q)||previous[std::size_t(idx(q))]>=0||
                    std::abs(q.x-bank.x)+std::abs(q.y-bank.y)>12 || !world.road_buildable(q)||
                    world.map_permissions()->cell_height(q)!=world.map_permissions()->cell_height(bank)||
                    (world.object_at(q)!=s::Object::Empty&&!world.road_cell(q)))continue;
                previous[std::size_t(idx(q))]=idx(cell);queue.push_back(q);}
        }
        return {};
    }
    std::optional<s::BuildingId> place(s::CommandType type,bool far) {
        const auto kind=s::placed_object(type);check(bool(kind),"fixture placement kind missing");const auto bank=far?b:a;
        std::optional<s::Cell> best;std::vector<s::Cell> roads;int score=INT32_MAX;
        for(int dy=-10;dy<=10;++dy)for(int dx=-10;dx<=10;++dx){const s::Cell origin{bank.x+dx,bank.y+dy};
            if(!world.validate({type,origin}).accepted)continue;
            const auto footprint=s::building_footprint_cells(world.profile(),world.rule_version(),*kind,origin);
            if(!std::all_of(footprint.begin(),footprint.end(),[&](s::Cell cell){return side(cell,far);}))continue;
            std::optional<std::vector<s::Cell>> route=*kind==s::Object::Well?std::optional<std::vector<s::Cell>>{{}}:connection(origin,*kind,far);
            if(!route)continue;const auto count=std::count_if(route->begin(),route->end(),[&](s::Cell c){return !world.road_cell(c);});
            if(world.treasury()<world.construction_cost(type)+s::Rules::road_cost*count)continue;
            int candidate=int(count)*20+int(route->size())+std::abs(dx)+std::abs(dy);
            if(*kind==s::Object::Well){candidate=0;for(const auto& home:world.buildings())if(home.kind==s::Object::Household)
                candidate+=s::footprint_distance(origin,{2,2},home.cell,{2,2})*20;}
            if(candidate<score){best=origin;roads=*route;score=candidate;}
        }
        if(!best)return {};
        check(execute({type,*best}),"selected paid building failed revalidation");const auto id=world.building_owner_at(*best);
        for(const auto cell:roads)check(execute({s::CommandType::PlaceRoad,cell}),"selected paid connection failed revalidation");return id;
    }
    bool expansion_room(bool far,std::size_t required) const {
        const auto bank=far?b:a;std::vector<s::Cell> claimed;
        for(int dy=-10;dy<=10;++dy)for(int dx=-10;dx<=10;++dx) {
            const s::Cell origin{bank.x+dx,bank.y+dy};
            const auto footprint=s::building_footprint_cells(world.profile(),world.rule_version(),s::Object::Household,origin);
            if(!std::all_of(footprint.begin(),footprint.end(),[&](s::Cell cell){return side(cell,far)&&world.buildable(cell)&&
                world.object_at(cell)==s::Object::Empty&&!world.bridge_bank_at(cell)&&
                std::find(claimed.begin(),claimed.end(),cell)==claimed.end();})||!connection(origin,s::Object::Household,far))continue;
            claimed.insert(claimed.end(),footprint.begin(),footprint.end());if(--required==0)return true;
        }
        return false;
    }
    bool bridge_is_required() const {
        // Replay normal paid commands without the bridge. This is an actual
        // no-bridge control, not a modified snapshot or altered permission map.
        s::World control(world.map_permissions(),s::RulesProfile::CityV16,5);
        for(const auto& row:commands) {
            const auto type=static_cast<s::CommandType>(row.at("type").get<unsigned>());
            if(type==s::CommandType::PlaceBridge)continue;
            const auto c=row.at("cell");s::Command command{type,{c[0],c[1]}};
            if(!control.execute(command).accepted)return false;
        }
        for(const auto& courier:control.couriers())for(const auto& target:control.buildings())
            if(courier.owner!=target.id && s::courier_can_target(courier.role,target) &&
                side(control.building(courier.owner).cell,false)!=side(target.cell,false) &&
                control.find_building_route(courier.owner,target.id))return false;
        return true;
    }
};
void invariants(const s::World& world){check(world.navigation_valid()&&world.production_balance_valid()&&world.food_balance_valid()&&world.city_economy_valid()&&
    world.population_valid()&&world.service_state_valid()&&world.health_state_valid()&&world.fire_state_valid(),"bridge gameplay invariant failed at "+std::to_string(world.ticks()));}
void save_negatives(Context& context,const fs::path& root,const s::World& world) {
    const auto document=read(root/"end.json");const auto unchanged=world.snapshot();J results=J::array();
    for(int test=0;test<17;++test) {
        auto bad=document;std::string name;
        auto& span=bad["world"]["bridges"][0];
        switch(test) {
        case 0:name="wrong schema20";bad["schema_version"]=20;break;
        case 1:name="wrong rule4";bad["rules"]["version"]=4;break;
        case 2:name="wrong policy2";bad["profiles"]["map_permissions"]["version"]=2;break;
        case 3:name="wrong original map";bad["map"]["relative_path"]="Cities/Handan.map";break;
        case 4:name="wrong policy fingerprint";bad["profiles"]["map_permissions"]["sha256"]=std::string(64,'0');break;
        case 5:name="wrong building mask fingerprint";bad["profiles"]["buildable_sha256"]=std::string(64,'0');break;
        case 6:name="empty water span";span["water_cells"]=J::array();break;
        case 7:name="noncanonical endpoint";span["bank_b"][0]=span["bank_b"][0].get<int>()+1;break;
        case 8:name="wrong deck";span["deck_height"]=span["deck_height"].get<int>()+1;break;
        case 9:name="duplicate typed bridge ID";bad["world"]["bridges"].push_back(span);bad["world"]["next_bridge_id"]=3;break;
        case 10:name="wrong axis";span["axis"]=1-span["axis"].get<int>();break;
        case 11:name="wrong paid cost";span["paid_cost"]=0;break;
        case 12:name="out of grid cell";span["water_cells"][0]=J::array({511,511});break;
        case 13:name="invented derived mask";span["mask"]=J::array({1});break;
        case 14:name="future placement tick";span["placed_tick"]=world.ticks()+1;break;
        case 15:name="invalid next identity";bad["world"]["next_bridge_id"]=UINT32_MAX;break;
        case 16:name="overflowed command sequence";bad["world"]["command_sequence"]=UINT64_MAX;break;
        }
        const auto file=root/("invalid-"+std::to_string(test)+".json");write(file,bad);bool rejected=false;std::string reason;
        try{const auto parsed=p::read_save(file);(void)p::restore_save(parsed,context.data,context.mask);}
        catch(const std::exception& error){rejected=true;reason=error.what();}
        check(rejected&&world.snapshot()==unchanged,"malformed save accepted or changed current world: "+name);
        results.push_back({{"case",name},{"rejected",rejected},{"reason",reason},{"current_world_unchanged",true}});
    }
    write(root/"save-negative-cases.json",results);
}
void additional_role_controls(Context& context,const fs::path& root,s::Cell a,s::Cell b) {
    Builder control(context.policy,a,b);
    control.world=p::restore_save(p::read_save(root/"end.json"),context.data,context.mask);
    // A separate paid continuation buys the second Warehouse on bank A.
    // Pottery and its Market inbound courier now have real opposite-bank
    // destinations; stocks and reservations still arise only through play.
    const auto warehouse=control.place(s::CommandType::PlaceWarehouse,false);check(bool(warehouse),"paid opposite-bank Warehouse control failed");
    std::set<unsigned> outbound,returns;std::ofstream traffic(root/"additional-role-traffic.jsonl");
    for(int tick=0;tick<2400;++tick) {
        control.world.tick();invariants(control.world);
        for(const auto& courier:control.world.couriers())if(courier.phase!=s::CourierPhase::IdleAtWorkshop&&
            (courier.role==s::CourierRole::Pottery||courier.role==s::CourierRole::MarketPotteryInbound)&&
            control.world.bridge_at(courier.path.at(courier.path_vertex))) {
            const auto role=unsigned(courier.role);if(courier.phase==s::CourierPhase::Returning)returns.insert(role);else outbound.insert(role);
            traffic<<J{{"tick",control.world.ticks()},{"role",role},{"phase",s::courier_phase_name(courier.phase)},
                {"cargo",courier.cargo},{"reserved",courier.reserved},{"vertex",courier.path_vertex},{"edge_progress",courier.edge_progress}}.dump()<<'\n';
        }
    }
    check(outbound.contains(2)&&returns.contains(2)&&outbound.contains(6)&&returns.contains(6),"actual Pottery/MarketPotteryInbound bridge round trips missing");
    context.checkpoint(control.world,root/"additional-roles-end.json");write(root/"additional-role-commands.json",control.commands);
    write(root/"additional-role-controls.json",{{"evidence","separate normal paid continuation of end.json; no snapshot field edits"},
        {"tick",control.world.ticks()},{"outbound_roles",outbound},{"returning_roles",returns},{"funds",control.world.treasury()}});
}
void render_check(Context& context,const fs::path& root,const fs::path& checkpoint,Sdl& sdl) {
    auto session=m::load_stored_map_session(context.data,context.map,m::FootprintPolicy::EdgeByte4x4Preview,m::StoredGraphicsProfile::Slot8,m::GreatWallPresentationMode::Automatic);
    oe::SandboxView view(std::move(session),false,s::RulesProfile::CityV16,5);
    const auto visuals=oe::detect_and_select_visual_profiles(context.data,fs::current_path()/"resources");
    if(!visuals.building.empty())view.set_building_visuals(visuals.building,visuals.building_source);
    if(!visuals.road.empty())view.set_road_visuals(visuals.road,visuals.road_source);
    if(!visuals.walker.empty())view.set_walker_visuals(visuals.walker,visuals.walker_source);
    view.configure_save(context.data,context.map,root/"scripted-f5-f9.json",p::read_save(checkpoint));view.initialize(sdl.window,sdl.renderer);
    bool running=true;SDL_Event event{};event.type=SDL_EVENT_KEY_DOWN;event.key.key=SDLK_F5;view.handle_event(event,running);
    const auto before=view.world().snapshot();view.tick_once();event.key.key=SDLK_F9;view.handle_event(event,running);
    check(view.world().snapshot()==before,"scripted F5/F9 changed complete on-deck state");
    oe::performance::set_enabled(true);oe::performance::reset();
    const auto live=oe::StoredGraphicsRenderer::live_texture_count();
    for(int frame=0;frame<100;++frame)check(view.render(),"pure bridge frame failed");
    J counters=J::object();for(std::size_t i=0;i<std::size_t(oe::performance::Counter::Count);++i){const auto count=oe::performance::counter(static_cast<oe::performance::Counter>(i));
        counters[std::to_string(i)]=count;check(count==0,"pure bridge frame performed forbidden work");}
    check(live==oe::StoredGraphicsRenderer::live_texture_count(),"pure frames changed live texture count");oe::performance::set_enabled(false);
    write(root/"render-100.json",{{"backend","dummy/software"},{"frames",100},{"counters",counters},{"live_textures",live},
        {"scripted_F5_F9","full snapshot equality"},{"native_human_acceptance","not inferred"}});
    const auto capture=[&](const char* name) {
        check(view.render(),"private bridge capture draw failed");SDL_Surface* surface=SDL_RenderReadPixels(sdl.renderer,nullptr);check(surface,"private bridge capture readback failed");
        const bool saved=SDL_SaveBMP(surface,(root/name).c_str());SDL_DestroySurface(surface);check(saved,"private bridge BMP write failed");
    };
    capture("bridge-software-1x.bmp");event.key.key=SDLK_Z;view.handle_event(event,running);
    capture("bridge-software-2x.bmp");view.handle_event(event,running);capture("bridge-software-4x.bmp");
}
void user_build_check(Context& context,const fs::path& root,Sdl& sdl) {
    auto session=m::load_stored_map_session(context.data,context.map,m::FootprintPolicy::EdgeByte4x4Preview,
        m::StoredGraphicsProfile::Slot8,m::GreatWallPresentationMode::Automatic);
    const m::MapGeometry geometry{session.map.declared_map_size};
    oe::SandboxView view(std::move(session),false,s::RulesProfile::CityV16,5);
    const auto visuals=oe::detect_and_select_visual_profiles(context.data,fs::current_path()/"resources");
    if(!visuals.building.empty())view.set_building_visuals(visuals.building,visuals.building_source);
    if(!visuals.road.empty())view.set_road_visuals(visuals.road,visuals.road_source);
    if(!visuals.walker.empty())view.set_walker_visuals(visuals.walker,visuals.walker_source);
    view.configure_save(context.data,context.map,root/"user-built.json",p::read_save(root/"before-bridge.json"));
    view.initialize(sdl.window,sdl.renderer);check(view.paused()&&view.world().bridges().empty(),"construction save must start paused without a bridge");
    const auto layout=p::read_save(root/"after-bridge-before-dispatch.json");const auto& span=layout.world.bridges.at(0);
    auto control=p::restore_save(p::read_save(root/"before-bridge.json"),context.data,context.mask);
    check(control.execute(s::place_bridge(span.bank_a,span.bank_b)).accepted,"canonical paid construction control failed");
    const auto expected=p::make_document(context.data,context.map,context.mask,control);
    const auto raw=[&](s::Cell cell) {
        auto ground=m::terrain_ground({unsigned(cell.x),unsigned(cell.y)},geometry.border);
        ground.y-=context.policy->cell_height(cell)*40;
        const auto screen=view.camera().world_to_screen(ground);
        check(view.layout().map.contains(screen.x,screen.y),"construction bank is not visible on initial camera");
        check(view.pick(screen)==cell,"visible original bank did not ground-pick exactly");
        SDL_FPoint point{};check(SDL_RenderCoordinatesToWindow(sdl.renderer,float(screen.x),float(screen.y),&point.x,&point.y),"bank render-to-window conversion failed");
        return point;
    };
    const auto a=raw(span.bank_a),b=raw(span.bank_b);bool running=true;
    SDL_Event event{};event.type=SDL_EVENT_KEY_DOWN;event.key.key=SDLK_B;view.handle_event(event,running);
    check(view.tool()==14,"normal B key did not expose Bridge construction");
    const auto before=view.world().snapshot();
    event={};event.type=SDL_EVENT_MOUSE_MOTION;event.motion.x=a.x;event.motion.y=a.y;view.handle_event(event,running);
    check(view.render(),"construction bank hover failed");
    event={};event.type=SDL_EVENT_MOUSE_BUTTON_DOWN;event.button.button=SDL_BUTTON_LEFT;
    event.button.x=a.x;event.button.y=a.y;view.handle_event(event,running);
    event={};event.type=SDL_EVENT_MOUSE_MOTION;event.motion.x=b.x;event.motion.y=b.y;
    oe::performance::set_enabled(true);oe::performance::reset();
    for(int frame=0;frame<100;++frame){view.handle_event(event,running);view.update(0,{});check(view.render(),"original-map user bridge preview failed");}
    for(std::size_t i=0;i<std::size_t(oe::performance::Counter::Count);++i)
        check(oe::performance::counter(static_cast<oe::performance::Counter>(i))==0,"original-map construction preview performed forbidden work");
    oe::performance::set_enabled(false);check(view.world().snapshot()==before,"construction preview changed paid city state");
    event={};event.type=SDL_EVENT_MOUSE_BUTTON_UP;event.button.button=SDL_BUTTON_LEFT;
    event.button.x=b.x;event.button.y=b.y;view.handle_event(event,running);
    if(view.world().snapshot()!=expected.world) {
        context.save(view.world(),root/"user-build-actual.json");
        std::cerr<<"B drag message: "<<view.last_message()<<"; spans "<<view.world().bridges().size()<<"; funds "<<view.world().treasury()<<'\n';
        throw std::runtime_error("user B drag differs from canonical normally paid bridge command");
    }
    event={};event.type=SDL_EVENT_KEY_DOWN;event.key.key=SDLK_F5;view.handle_event(event,running);
    const auto saved=p::read_save(root/"user-built.json");check(saved.world==expected.world&&saved.source_schema_version==21,"user-built bridge F5 lost state");
    bool outbound=false,returned=false;
    for(int tick=0;tick<200;++tick) {
        view.tick_once();invariants(view.world());
        for(const auto& courier:view.world().couriers())if(courier.role==s::CourierRole::Clay&&
            courier.phase!=s::CourierPhase::IdleAtWorkshop&&view.world().bridge_at(courier.path.at(courier.path_vertex))) {
            if(courier.phase==s::CourierPhase::Returning)returned=true;
            else if(courier.cargo>0)outbound=true;
        }
    }
    check(outbound&&returned,"user-built original-map bridge did not carry an actual loaded Clay round trip");
    write(root/"user-build-check.json",{{"evidence","scripted normal B/MouseDown/MouseUp and F5 on paid original-map save; human input separate"},
        {"map",context.map.generic_string()},{"bank_a",point(span.bank_a)},{"bank_b",point(span.bank_b)},
        {"funds_before",before.treasury},{"funds_after",expected.world.treasury},{"paid_cost",span.paid_cost},
        {"full_world_equal_to_paid_command",true},{"preview_frames_without_forbidden_work",100},{"loaded_crossing",outbound},{"return_crossing",returned}});
}
}
int main(int argc,char** argv){try {
    check(argc==4||argc==6,"usage: bridge-fixture DATA FRESH-.local-ROOT WATER-AUDIT | DATA ROOT --resume SAVE TICKS");
    const auto data=fs::canonical(argv[1]),root=fs::weakly_canonical(argv[2]);
    check(root.string().starts_with(fs::canonical(fs::current_path()/".local/city-v16-bridges").string()+"/")&&
        (!fs::exists(root)||fs::is_empty(root)),"fresh bridge-private output root required");fs::create_directories(root);Sdl sdl;
    if(argc==6){check(std::string(argv[3])=="--resume","unknown mode");const auto doc=p::read_save(argv[4]);Context context(data,doc.map_relative,sdl.renderer);
        auto world=p::restore_save(doc,data,context.mask);const int ticks=std::stoi(argv[5]);check(ticks>=0&&ticks<=10000,"bounded resume ticks required");
        context.save(world,root/"restarted-immediate.json");for(int i=0;i<ticks;++i){world.tick();invariants(world);}context.save(world,root/"restarted-end.json");return 0;}
    const auto audit=read(argv[3]);std::unique_ptr<Context> context;std::unique_ptr<Builder> chosen;J rejected=J::array();
    std::optional<s::BuildingId> clay,pottery,warehouse,farm,market,service;
    std::vector<std::string> preferred_maps={"Cities/Xia.map","Cities/Handan.map","Cities/MP34.map"};
    for(const auto& row:audit.at("maps"))if(!row.contains("policy_error")&&!row.at("spans").empty()&&
        std::find(preferred_maps.begin(),preferred_maps.end(),row.at("map").get<std::string>())==preferred_maps.end())
        preferred_maps.push_back(row.at("map").get<std::string>());
    for(const auto& preferred:preferred_maps) {
        const auto maps=audit.at("maps");const auto row=std::find_if(maps.begin(),maps.end(),[&](const auto& value){return value.at("map")==preferred;});
        if(row==maps.end()||row->contains("policy_error"))continue;context=std::make_unique<Context>(data,preferred,sdl.renderer);
        auto spans=row->at("spans");std::stable_sort(spans.begin(),spans.end(),[](const auto& a,const auto& b){return a.at("water_cells").size()<b.at("water_cells").size();});
        for(const auto& span:spans) {
            const auto a=span.at("bank_a"),b=span.at("bank_b");auto candidate=std::make_unique<Builder>(context->policy,s::Cell{a[0],a[1]},s::Cell{b[0],b[1]});
            auto& build=*candidate;
            std::string failure;
            if(!build.execute({s::CommandType::PlaceRoad,build.a})||!build.execute({s::CommandType::PlaceRoad,build.b}))failure="bank road";
            else if(!build.place(s::CommandType::PlaceHousehold,false))failure="first House bank A";
            else if(!build.place(s::CommandType::PlaceHousehold,false))failure="second House bank A";
            else if(!(clay=build.place(s::CommandType::PlaceClaySource,false)))failure="Clay bank A";
            else if(!(pottery=build.place(s::CommandType::PlacePottery,true)))failure="Pottery bank B";
            else if(build.world.find_building_route(*clay,*pottery))failure="existing alternate actual road route";
            if(!failure.empty()) {rejected.push_back({{"map",preferred},{"bank_a",point(build.a)},{"bank_b",point(build.b)},{"reason",failure}});continue;}
            // The original-map before-bridge checkpoint has genuinely separate
            // paid road networks, zero progress/goods and the complete policy.
            const auto before=build.world.snapshot();
            if(!build.execute(s::place_bridge(build.a,build.b))) {
                rejected.push_back({{"map",preferred},{"bank_a",point(build.a)},{"bank_b",point(build.b)},{"reason",build.world.validate_bridge(build.a,build.b).reason}});continue;}
            if(!(warehouse=build.place(s::CommandType::PlaceWarehouse,true))||!(farm=build.place(s::CommandType::PlaceFarm,false))||
                !(market=build.place(s::CommandType::PlaceMarket,true))||!(service=build.place(s::CommandType::PlaceServicePost,true))||
                !build.place(s::CommandType::PlaceHousehold,false)||!build.place(s::CommandType::PlaceWell,false)) {
                rejected.push_back({{"map",preferred},{"bank_a",point(build.a)},{"bank_b",point(build.b)},{"reason","paid complete supply district did not fit original geometry/budget"}});continue;
            }
            if(!build.expansion_room(false,2)||!build.expansion_room(true,1)) {
                rejected.push_back({{"map",preferred},{"bank_a",point(build.a)},{"bank_b",point(build.b)},
                    {"reason","paid starter fits, but conservative future House/Health footprints do not"}});continue;
            }
            if(!build.bridge_is_required()) {
                rejected.push_back({{"map",preferred},{"bank_a",point(build.a)},{"bank_b",point(build.b)},
                    {"reason","complete paid bank districts expose an alternate original/land-road route"}});continue;
            }
            context->checkpoint(s::World::restore(before,context->policy),root/"before-bridge.json");chosen=std::move(candidate);break;
        }
        if(chosen)break;
        std::cerr<<preferred<<": no paid district candidate; checked "<<row->at("spans").size()<<" spans\n";
    }
    write(root/"candidate-rejections.json",rejected);check(bool(chosen),"No supported original crossing fits paid districts; original protections were not relaxed");
    auto& builder=*chosen;auto& world=builder.world;context->checkpoint(world,root/"after-bridge-before-dispatch.json");
    write(root/"connectivity.json",{{"before","no actual Clay->Pottery route"},{"after_route_cells",world.find_building_route(*clay,*pottery)->size()},
        {"map",context->map.generic_string()},{"banks",J::array({point(builder.a),point(builder.b)})}});
    std::ofstream traffic(root/"traffic.jsonl"),finance(root/"finance.jsonl");std::map<unsigned,std::size_t> outgoing,returning;
    bool outbound_saved=false,return_saved=false,delivery_saved=false,expanded=false,watch=false,health=false;
    std::uint64_t first_recipe=0;
    for(int i=0;i<12000;++i) {
        if(world.ticks()%20==0) {
            const auto& pot=world.building(*pottery);const bool source_needed=pot.input_clay<=2;
            if(!expanded) {
                if(world.building(*clay).operating_enabled!=source_needed)check(builder.execute(s::set_building_operation(*clay,source_needed)),"Clay operation failed");
                if(world.building(*pottery).operating_enabled==source_needed)check(builder.execute(s::set_building_operation(*pottery,!source_needed)),"Pottery operation failed");
                if(world.treasury()>=100 && builder.place(s::CommandType::PlaceHousehold,false)){expanded=true;
                    check(builder.execute(s::set_building_operation(*clay,true))&&builder.execute(s::set_building_operation(*pottery,true)),"full supply operation failed");}
            }
            if(expanded&&!watch&&world.treasury()>=100)watch=bool(builder.place(s::CommandType::PlaceFireWatch,true));
            if(watch&&!health&&world.treasury()>=220) {
                if(world.workforce_supply()<world.workforce_required()+2)(void)builder.place(s::CommandType::PlaceHousehold,false);
                if(world.treasury()>=120)health=bool(builder.place(s::CommandType::PlaceHealthPost,true));
            }
        }
        world.tick();invariants(world);
        for(const auto& courier:world.couriers())if(courier.phase!=s::CourierPhase::IdleAtWorkshop) {
            const auto cell=courier.path.at(courier.path_vertex);if(!world.bridge_at(cell))continue;
            const auto role=unsigned(courier.role);if(courier.phase==s::CourierPhase::Returning)++returning[role];else ++outgoing[role];
            traffic<<J{{"tick",world.ticks()},{"courier",std::uint32_t(courier.id)},{"role",role},{"phase",s::courier_phase_name(courier.phase)},
                {"cell",point(cell)},{"vertex",courier.path_vertex},{"edge_progress",courier.edge_progress},{"cargo",courier.cargo},{"reserved",courier.reserved}}.dump()<<'\n';
            if(courier.role==s::CourierRole::Clay) {
                if(!outbound_saved&&courier.phase==s::CourierPhase::ToWarehouse){context->checkpoint(world,root/"outbound-on-bridge.json");outbound_saved=true;}
                if(!return_saved&&courier.phase==s::CourierPhase::Returning){context->checkpoint(world,root/"return-on-bridge.json");return_saved=true;}
            }
        }
        if(!delivery_saved && world.building(*pottery).input_clay>0){context->checkpoint(world,root/"after-actual-delivery.json");delivery_saved=true;}
        if(!first_recipe&&world.building(*pottery).recipes_completed)first_recipe=world.ticks();
        if(world.ticks()%400==0)finance<<J{{"tick",world.ticks()},{"funds",world.treasury()},{"taxes",world.taxes_collected_total()},
            {"upkeep",world.maintenance_spent_total()},{"population",world.total_population()},{"workers",world.workforce_used()},
            {"pottery_recipes",world.building(*pottery).recipes_completed},{"food",world.food_produced_total()}}.dump()<<'\n';
        if(world.ticks()==10000)context->checkpoint(world,root/"ten-thousand.json");
    }
    context->checkpoint(world,root/"end.json");write(root/"commands.json",builder.commands);
    J summary={{"map",context->map.generic_string()},{"bank_a",point(builder.a)},{"bank_b",point(builder.b)},
        {"rule",5},{"policy",3},{"schema",21},{"ticks",world.ticks()},{"funds",world.treasury()},
        {"taxes",world.taxes_collected_total()},{"maintenance",world.maintenance_spent_total()},{"population",world.total_population()},
        {"first_recipe",first_recipe},{"outgoing_on_deck_ticks",outgoing},{"returning_on_deck_ticks",returning},
        {"fire_watch_built",watch},{"health_post_built",health},{"span_cost",world.bridges()[0].paid_cost},
        {"bridge_cells",J::array()},{"evidence","SCRIPTED paid original-map World gameplay; native/human distinct"}};
    for(const auto cell:world.bridges()[0].water_cells)summary["bridge_cells"].push_back(point(cell));write(root/"summary.json",summary);
    check(outbound_saved&&return_saved&&delivery_saved&&first_recipe,"real Clay crossing/delivery/return/recipe missing");
    check(outgoing[unsigned(s::CourierRole::MarketFoodInbound)]&&outgoing[unsigned(s::CourierRole::MarketPotteryDistribution)]&&
        outgoing[unsigned(s::CourierRole::Service)],"actual Food/Market/Service bridge roles missing");
    check(watch&&health&&outgoing[unsigned(s::CourierRole::FireInspector)]&&outgoing[unsigned(s::CourierRole::HealthWorker)],
        "actual FireInspector/HealthWorker bridge roles missing");
    // Real bank disconnection while a loaded courier is on the deck. Removal
    // cannot interrupt a begun edge. Waiting retains cargo/reservation and its
    // exact vertex; repairing the ordinary road resumes ordinary routing.
    auto blocked=s::World::restore(world.snapshot(),context->policy);bool tested=false;
    const auto bank=blocked.bridges()[0].bank_b;
    for(int ticks=0;ticks<1000&&!tested;++ticks) {
        blocked.tick();
        for(const auto& c:blocked.couriers())if(c.role==s::CourierRole::Clay&&c.phase==s::CourierPhase::ToWarehouse&&
            c.edge_progress==0&&blocked.bridge_at(c.path.at(c.path_vertex))&&blocked.validate({s::CommandType::RemoveRoad,bank}).accepted) {
            const auto courier_id=c.id;const auto cargo=c.cargo,reserved=c.reserved;const auto position=blocked.courier_position(courier_id);
            check(blocked.execute({s::CommandType::RemoveRoad,bank}).accepted,"bank removal revalidation failed");
            for(int wait=0;wait<60;++wait){blocked.tick();invariants(blocked);}
            const auto& waiting=blocked.courier(courier_id);
            check(waiting.route_pending&&waiting.cargo==cargo&&waiting.reserved==reserved&&blocked.courier_position(courier_id)==position,
                "disconnected bridge teleported or lost cargo/reservation");
            context->checkpoint(blocked,root/"waiting-bank-disconnected.json");
            check(blocked.execute({s::CommandType::PlaceRoad,bank}).accepted,"bank road repair failed");
            bool arrived=false;for(int finish=0;finish<500;++finish){blocked.tick();invariants(blocked);
                arrived|=blocked.courier(courier_id).phase==s::CourierPhase::Returning;}
            check(arrived,"repaired bridge did not resume real delivery and return");
            context->checkpoint(blocked,root/"after-bank-repair.json");tested=true;break;
        }
    }
    check(tested,"no occupied-deck disconnect scenario measured");
    save_negatives(*context,root,world);additional_role_controls(*context,root,builder.a,builder.b);
    user_build_check(*context,root,sdl);
    render_check(*context,root,root/"outbound-on-bridge.json",sdl);
    std::cout<<summary.dump(2)<<'\n';return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
