// Independently authored terrain paid gameplay and persistence; explicit, excluded local runner.
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
        window=SDL_CreateWindow("Ramp authored acceptance",1280,720,SDL_WINDOW_HIDDEN);check(window,SDL_GetError());
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
            loaded.map_permissions_policy_version==3&&loaded.map_permissions_sha256==doc.map_permissions_sha256&&loaded.source_schema_version==22,
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
    Builder(std::shared_ptr<const s::MapPermissions> policy,s::Cell first,s::Cell last):world(policy,s::RulesProfile::CityV16,6),a(first),b(last),axis_x(a.x!=b.x){}
    bool side(s::Cell cell,bool far)const{return axis_x?(far?cell.x>=b.x:cell.x<=a.x):(far?cell.y>=b.y:cell.y<=a.y);}
    bool execute(s::Command command){const auto result=world.execute(command);if(!result.accepted)return false;
        commands.push_back({{"tick",world.ticks()},{"type",unsigned(command.type)},{"cell",point(command.cell)},
            {"ramp_end",point(command.ramp_end)},{"building_id",std::uint32_t(command.building_id)},
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
        s::World control(world.map_permissions(),s::RulesProfile::CityV16,6);
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

void negatives(Context& context,const fs::path& root,const s::World& world) {
    const auto original=read(root/"end.json");const auto before=world.snapshot();J results=J::array();
    for(int test=0;test<23;++test) {
        auto bad=original;auto& r=bad["world"]["ramps"][0];
        switch(test) {
        case 0:bad["schema_version"]=21;break;case 1:bad["rules"]["version"]=5;break;
        case 2:bad["profiles"]["map_permissions"]["version"]=2;break;
        case 3:bad["map"]["relative_path"]="Cities/Missing.map";break;
        case 4:bad["profiles"]["map_permissions"]["sha256"]=std::string(64,'0');break;
        case 5:bad["map"]["sha256"]=std::string(64,'0');break;
        case 6:r["id"]=0;break;case 7:r["id"]=2;break;case 8:bad["world"]["next_ramp_id"]=1;break;
        case 9:std::swap(r["endpoint_a"],r["endpoint_b"]);break;case 10:r["endpoint_b"]=J::array({117,115});break;
        case 11:r["height_a"]=128;break;case 12:r["height_a"]=-128;r["height_b"]=127;break;
        case 13:r["paid_cost"]=0;break;case 14:r["placed_tick"]=world.ticks()+1;break;
        case 15:bad["world"]["ramps"].push_back(r);bad["world"]["next_ramp_id"]=3;break;
        case 16:r["endpoint_a"]=J::array({511,511});break;
        case 17:r["mask"]=J::array({1});break;case 18:bad["world"]["command_sequence"]=UINT64_MAX;break;
        case 19:bad["world"]["construction_spent_total"]=UINT64_MAX;break;
        case 20:bad["world"]["road_revision"]=UINT64_MAX;break;
        case 21:bad["world"]["ramps"]=J::array();bad["world"]["next_ramp_id"]=1;break;
        case 22:bad["profiles"]["buildable_sha256"]=std::string(64,'0');break;
        }
        const auto file=root/("invalid-ramp-"+std::to_string(test)+".json");write(file,bad);bool rejected=false;std::string reason;
        try{auto doc=p::read_save(file);(void)p::restore_save(doc,context.data,context.mask);}catch(const std::exception& e){rejected=true;reason=e.what();}
        check(rejected&&world.snapshot()==before,"malformed ramp save accepted or mutated active world");results.push_back({{"case",test},{"rejected",true},{"reason",reason}});
    }
    const auto file=root/"truncated.json";{std::ofstream out(file);out<<original.dump().substr(0,120);}
    bool rejected=false;try{(void)p::read_save(file);}catch(const std::exception&){rejected=true;}check(rejected,"truncated save accepted");
    write(root/"negative-saves.json",results);
}
void captures(Context& context,const fs::path& root,const fs::path& checkpoint,Sdl& sdl,bool build=false) {
    auto session=m::load_stored_map_session(context.data,context.map,m::FootprintPolicy::EdgeByte4x4Preview,m::StoredGraphicsProfile::Slot8);
    const m::MapGeometry geometry{session.map.declared_map_size};
    oe::SandboxView view(std::move(session),false,s::RulesProfile::CityV16,6);
    const auto visuals=oe::detect_and_select_visual_profiles(context.data,fs::current_path()/"resources");
    if(!visuals.building.empty())view.set_building_visuals(visuals.building,visuals.building_source);
    if(!visuals.road.empty())view.set_road_visuals(visuals.road,visuals.road_source);
    if(!visuals.walker.empty())view.set_walker_visuals(visuals.walker,visuals.walker_source);
    view.configure_save(context.data,context.map,root/"scripted-f5-f9.json",p::read_save(checkpoint));view.initialize(sdl.window,sdl.renderer);
    bool running=true;SDL_Event event{};
    if(build) {
        const s::Cell a{115,114},b{116,114};auto control=s::World::restore(view.world().snapshot(),context.policy);
        check(control.execute(s::place_ramp(a,b)).accepted,"normal UI control ramp rejected");
        const auto raw=[&](s::Cell c){auto point=m::terrain_ground({unsigned(c.x),unsigned(c.y)},geometry.border);
            point.y-=context.policy->cell_height(c)*40;const auto screen=view.camera().world_to_screen(point);
            check(view.pick(screen)==c,"visible ramp endpoint did not pick exactly");SDL_FPoint raw{};
            check(SDL_RenderCoordinatesToWindow(sdl.renderer,float(screen.x),float(screen.y),&raw.x,&raw.y),"coordinate conversion failed");return raw;};
        const auto a0=raw(a),b0=raw(b);event.type=SDL_EVENT_KEY_DOWN;event.key.key=SDLK_P;view.handle_event(event,running);
        check(view.tool()==15,"normal P key did not activate Ramp");
        event={};event.type=SDL_EVENT_MOUSE_BUTTON_DOWN;event.button.button=SDL_BUTTON_LEFT;event.button.x=a0.x;event.button.y=a0.y;view.handle_event(event,running);
        event={};event.type=SDL_EVENT_MOUSE_MOTION;event.motion.x=b0.x;event.motion.y=b0.y;view.handle_event(event,running);view.update(0,{});
        const auto before=view.world().snapshot();oe::performance::set_enabled(true);oe::performance::reset();
        for(int i=0;i<100;++i){view.handle_event(event,running);view.update(0,{});check(view.render(),"pure ramp preview failed");}
        for(std::size_t i=0;i<std::size_t(oe::performance::Counter::Count);++i)check(oe::performance::counter(static_cast<oe::performance::Counter>(i))==0,"preview performed forbidden work");
        oe::performance::set_enabled(false);check(view.world().snapshot()==before,"preview mutated World");
        event={};event.type=SDL_EVENT_MOUSE_BUTTON_UP;event.button.button=SDL_BUTTON_LEFT;event.button.x=b0.x;event.button.y=b0.y;view.handle_event(event,running);
        check(view.world().snapshot()==control.snapshot(),"normal P drag differs from paid PlaceRamp command: "+view.last_message());
    }
    event={};event.type=SDL_EVENT_KEY_DOWN;event.key.key=SDLK_F5;view.handle_event(event,running);
    const auto before=view.world().snapshot();view.tick_once();event.key.key=SDLK_F9;view.handle_event(event,running);
    check(view.world().snapshot()==before,"actual F5/ticks/F9 changed complete Ramp state");
    oe::performance::set_enabled(true);oe::performance::reset();const auto live=oe::StoredGraphicsRenderer::live_texture_count();
    for(int frame=0;frame<100;++frame)check(view.render(),"pure ramp frame failed");
    for(std::size_t i=0;i<std::size_t(oe::performance::Counter::Count);++i)check(oe::performance::counter(static_cast<oe::performance::Counter>(i))==0,"pure frame performed forbidden work");
    oe::performance::set_enabled(false);check(live==oe::StoredGraphicsRenderer::live_texture_count(),"render changed textures");
    const auto capture=[&](const char* name){check(view.render(),"capture render failed");auto* surface=SDL_RenderReadPixels(sdl.renderer,nullptr);check(surface,SDL_GetError());
        const bool saved=SDL_SaveBMP(surface,(root/name).c_str());SDL_DestroySurface(surface);check(saved,"private capture failed");};
    if(build)capture("user-build-1x.bmp");else {capture("ramp-1x.bmp");event.key.key=SDLK_Z;view.handle_event(event,running);capture("ramp-2x.bmp");view.handle_event(event,running);capture("ramp-4x.bmp");}
}
}
int main(int argc,char** argv){try {
    check(argc==3||argc==6,"usage: ramps-fixture AUTHORED-DATA FRESH-ROOT [--resume SAVE TICKS]");
    const auto data=fs::canonical(argv[1]),root=fs::weakly_canonical(argv[2]);
    const auto local=fs::canonical(".local");auto rel=root.lexically_relative(local);
    check(!rel.empty()&&*rel.begin()!=".."&&root!=local&&(!fs::exists(root)||fs::is_empty(root)),"fresh ignored root required");
    fs::create_directories(root);Sdl sdl;Context context(data,"Cities/Authored Ramps.map",sdl.renderer);
    if(argc==6) {
        check(std::string(argv[3])=="--resume","unknown mode");auto w=p::restore_save(p::read_save(argv[4]),data,context.mask);
        context.save(w,root/"restarted-immediate.json");for(int i=0;i<std::stoi(argv[5]);++i)w.tick();context.save(w,root/"restarted-end.json");return 0;
    }
    Builder builder(context.policy,{115,114},{120,114});auto& world=builder.world;
    const s::Cell ra{115,114},rb{116,114};
    for(const auto c:{ra,rb,s::Cell{117,114},s::Cell{118,114},s::Cell{120,114}})check(builder.execute({s::CommandType::PlaceRoad,c}),"normal paid trunk road failed");
    for(int x=121;x<=125;++x)check(builder.execute({s::CommandType::PlaceRoad,{x,114}}),"paid lower expansion trunk failed");
    for(int x=110;x<115;++x)check(builder.execute({s::CommandType::PlaceRoad,{x,114}}),"paid expansion trunk failed");
    check(builder.execute(s::place_bridge({118,114},{120,114})),"normal paid inherited bridge failed");
    check(builder.place(s::CommandType::PlaceHousehold,false).has_value()&&builder.place(s::CommandType::PlaceHousehold,false).has_value(),"paid starter houses failed");
    const auto clay=builder.place(s::CommandType::PlaceClaySource,true),pottery=builder.place(s::CommandType::PlacePottery,false);
    check(clay&&pottery,"paid industry failed");check(!world.find_building_route(*clay,*pottery),"height route existed before ramp");
    check(world.transport_edge_blocker(ra,rb)==s::BuildBlocker::UnsupportedHeightTransition,"source height negative absent");
    context.checkpoint(world,root/"before-ramp.json");const auto before=world.snapshot();check(builder.execute(s::place_ramp(ra,rb)),"paid ramp failed");
    check(world.treasury()==before.treasury-40&&world.ramps()[0].id.value==1&&world.bridges()[0].id.value==1,"independent paid identities failed");
    context.checkpoint(world,root/"after-ramp.json");
    for(const auto& [type,far]:std::vector<std::pair<s::CommandType,bool>>{{s::CommandType::PlaceWarehouse,true},{s::CommandType::PlaceFarm,false},
        {s::CommandType::PlaceMarket,true},{s::CommandType::PlaceServicePost,true},{s::CommandType::PlaceHousehold,false}}) {
        if(!builder.place(type,far)){write(root/"failed-commands.json",builder.commands);throw std::runtime_error("paid supply failed type "+std::to_string(unsigned(type))+" funds "+std::to_string(world.treasury()));}
    }
    auto report=s::diagnose_road_connectivity(world,*clay,*pottery);check(report.route&&report.ramps_on_route.size()==1&&report.bridges_on_route.size()==1,"combined actual route missing");
    std::map<unsigned,std::size_t> outgoing,returning;bool entered=false,middle=false,exit=false,back=false,delivery=false,expanded=false,watch=false,health=false,well=false;std::uint64_t first_recipe=0;
    std::ofstream traffic(root/"traffic.jsonl"),finance(root/"finance.jsonl");
    for(int t=0;t<12000;++t) {
        if(world.ticks()%20==0) {
            const bool needed=world.building(*pottery).input_clay<=2;
            if(!expanded || !watch) {
                if(world.building(*clay).operating_enabled!=needed)check(builder.execute(s::set_building_operation(*clay,needed)),"operation failed");
                if(world.building(*pottery).operating_enabled==needed)check(builder.execute(s::set_building_operation(*pottery,!needed)),"operation failed");
                if(watch&&world.treasury()>=100&&builder.place(s::CommandType::PlaceHousehold,false)){expanded=true;
                    check(builder.execute(s::set_building_operation(*clay,true))&&builder.execute(s::set_building_operation(*pottery,true)),"supply activation failed");}
            }
            if(expanded&&!well&&world.treasury()>=90)well=bool(builder.place(s::CommandType::PlaceWell,false));
            if(!watch&&world.treasury()>=100)watch=bool(builder.place(s::CommandType::PlaceFireWatch,true));
            if(watch&&!health&&world.treasury()>=220) {
                if(world.workforce_supply()<world.workforce_required()+2)(void)builder.place(s::CommandType::PlaceHousehold,false);
                if(world.treasury()>=120)health=bool(builder.place(s::CommandType::PlaceHealthPost,true));
            }
        }
        world.tick();invariants(world);
        for(const auto& c:world.couriers())if(c.path_vertex+1<c.path.size()) {
            const auto* r=world.ramp_between(c.path[c.path_vertex],c.path[c.path_vertex+1]);if(!r)continue;
            (c.phase==s::CourierPhase::Returning?returning:outgoing)[unsigned(c.role)]++;
            traffic<<J{{"tick",world.ticks()},{"courier",std::uint32_t(c.id)},{"role",unsigned(c.role)},{"from",point(c.path[c.path_vertex])},
                {"to",point(c.path[c.path_vertex+1])},{"vertex",c.path_vertex},{"progress",c.edge_progress},{"duration",world.active_rules().courier_edge_ticks},
                {"phase",unsigned(c.phase)},{"cargo",c.cargo},{"reserved",c.reserved},{"surface_height",*world.ramp_surface_height(*world.courier_position(c.id))}}.dump()<<'\n';
            if(c.role==s::CourierRole::Clay) {
                if(!entered&&c.phase==s::CourierPhase::ToWarehouse&&c.edge_progress==0){context.checkpoint(world,root/"before-entry.json");entered=true;}
                if(!middle&&c.phase==s::CourierPhase::ToWarehouse&&c.edge_progress==2){context.checkpoint(world,root/"mid-ramp.json");middle=true;}
                if(!back&&c.phase==s::CourierPhase::Returning&&c.edge_progress==2){context.checkpoint(world,root/"return-ramp.json");back=true;}
            }
        }
        const auto& c=world.courier(static_cast<s::CourierId>(1));
        if(middle&&!exit&&c.phase==s::CourierPhase::ToWarehouse&&c.path_vertex&&c.path_vertex<c.path.size()&&c.edge_progress==0&&c.path[c.path_vertex]==ra){context.checkpoint(world,root/"after-exit.json");exit=true;}
        if(!delivery&&world.building(*pottery).input_clay){context.checkpoint(world,root/"after-delivery.json");delivery=true;}
        if(!first_recipe&&world.building(*pottery).recipes_completed)first_recipe=world.ticks();
        if(world.ticks()%400==0)finance<<J{{"tick",world.ticks()},{"funds",world.treasury()},{"taxes",world.taxes_collected_total()},
            {"upkeep",world.maintenance_spent_total()},{"population",world.total_population()},{"workers_supply",world.workforce_supply()},
            {"workers_used",world.workforce_used()},{"road_revision",world.road_revision()},{"food_produced",world.food_produced_total()},
            {"pottery_recipes",world.building(*pottery).recipes_completed}}.dump()<<'\n';
        if(world.ticks()==10000)context.checkpoint(world,root/"ten-thousand.json");
    }
    context.checkpoint(world,root/"end.json");write(root/"commands.json",builder.commands);
    J summary={{"evidence","OPENEMPEROR AUTHORED RAMP; independent authored map, production source permissions, normally paid gameplay; no original-map PASS"},
        {"map",context.map.generic_string()},{"rule",6},{"policy",3},{"schema",22},{"ramp_a",point(ra)},{"ramp_b",point(rb)},
        {"height_a",1},{"height_b",0},{"bridge_bank_a",point({118,114})},{"bridge_bank_b",point({120,114})},{"bridge_deck",0},
        {"ticks",world.ticks()},{"funds",world.treasury()},{"taxes",world.taxes_collected_total()},{"maintenance",world.maintenance_spent_total()},
        {"population",world.total_population()},{"outgoing_ramp_ticks",outgoing},{"returning_ramp_ticks",returning},{"first_recipe",first_recipe},
        {"fire_watch",watch},{"health_post",health},{"native_human_acceptance","not inferred"}};write(root/"summary.json",summary);
    check(entered&&middle&&exit&&back&&delivery&&first_recipe,"actual paid climb/delivery/return missing");
    for(const auto role:{s::CourierRole::Clay,s::CourierRole::Service,s::CourierRole::MarketFoodInbound,s::CourierRole::MarketPotteryDistribution,
        s::CourierRole::FireInspector,s::CourierRole::HealthWorker})check(outgoing[unsigned(role)]&&returning[unsigned(role)],"required role did not actually cross ramp both ways");
    auto disconnected=s::World::restore(world.snapshot(),context.policy);bool tested=false;
    for(int t=0;t<1500&&!tested;++t){disconnected.tick();
        for(const auto& c:disconnected.couriers())if(c.role==s::CourierRole::Clay&&c.phase==s::CourierPhase::ToWarehouse&&c.cargo&&c.edge_progress==0&&
            c.path_vertex+2<c.path.size()&&disconnected.ramp_between(c.path[c.path_vertex+1],c.path[c.path_vertex+2])&&disconnected.validate({s::CommandType::RemoveRoad,ra}).accepted) {
            const auto id=c.id;const auto cargo=c.cargo,reserved=c.reserved;const auto position=disconnected.courier_position(id);
            check(disconnected.execute({s::CommandType::RemoveRoad,ra}).accepted,"safe endpoint removal failed");
            for(int i=0;i<60;++i)disconnected.tick();check(disconnected.courier(id).cargo==cargo&&disconnected.courier(id).reserved==reserved&&disconnected.courier_position(id)==position,"disconnect lost cargo/reservation or teleported");
            context.checkpoint(disconnected,root/"disconnect-wait.json");check(disconnected.execute({s::CommandType::PlaceRoad,ra}).accepted,"paid repair failed");
            context.checkpoint(disconnected,root/"after-repair.json");for(int i=0;i<500;++i)disconnected.tick();invariants(disconnected);tested=true;break;
        }}check(tested,"no actual disconnect/wait/repair measured");
    negatives(context,root,world);
    for(const char* checkpoint:{"before-ramp","after-ramp","before-entry","mid-ramp","after-exit","after-delivery","return-ramp","disconnect-wait","after-repair"})
        captures(context,root,root/(std::string(checkpoint)+".json"),sdl);
    captures(context,root,root/"mid-ramp.json",sdl);captures(context,root,root/"before-ramp.json",sdl,true);
    check(oe::StoredGraphicsRenderer::live_texture_count()==0,"textures leaked after sessions");
    std::cout<<summary.dump(2)<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
