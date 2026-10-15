// Local paid reconstruction of the existing Authored Ramps map, not another map.
#include "app/SandboxView.h"
#include "app/VisualSelection.h"
#include "assets/RgbaPngEncoder.h"
#include "core/PerformanceDiagnostics.h"
#include "maps/LandscapeProvenance.h"
#include <SDL3/SDL.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
namespace oe=openemperor;namespace s=oe::simulation;namespace m=oe::maps;
namespace p=oe::persistence;namespace fs=std::filesystem;using J=nlohmann::json;
namespace {
void check(bool ok,const std::string& text){if(!ok)throw std::runtime_error(text);}
void key(oe::SandboxView& v,SDL_Keycode code){SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=code;bool running=true;v.handle_event(e,running);check(running,"unexpected quit");}
void park_pointer(oe::SandboxView& v){SDL_Event e{};e.type=SDL_EVENT_MOUSE_MOTION;e.motion.x=0;e.motion.y=0;bool running=true;v.handle_event(e,running);v.update(0,{});}
std::vector<std::uint8_t> pixels(SDL_Renderer* r,const SDL_Rect* area=nullptr) {
    auto* original=SDL_RenderReadPixels(r,area);check(original,SDL_GetError());
    auto* rgba=SDL_ConvertSurface(original,SDL_PIXELFORMAT_RGBA32);SDL_DestroySurface(original);check(rgba,SDL_GetError());
    oe::assets::RgbaImage image;image.width=std::uint16_t(rgba->w);image.height=std::uint16_t(rgba->h);image.pixels.resize(std::size_t(rgba->w*rgba->h*4));
    for(int y=0;y<rgba->h;++y)std::copy_n(static_cast<const std::uint8_t*>(rgba->pixels)+y*rgba->pitch,rgba->w*4,image.pixels.data()+y*rgba->w*4);
    SDL_DestroySurface(rgba);return oe::assets::encode_rgba_png(image);
}
void capture(oe::SandboxView& v,SDL_Renderer* r,const fs::path& file) {
    park_pointer(v);check(v.render(),SDL_GetError());const auto png=pixels(r);std::ofstream f(file,std::ios::binary);
    f.write(reinterpret_cast<const char*>(png.data()),std::streamsize(png.size()));check(bool(f),"PNG write failed");
}
void center(oe::SandboxView& v,oe::scene::Point world) {
    for(int i=0;i<800;++i) {
        const auto screen=v.camera().world_to_screen(world);const auto box=v.layout().map;
        const double dx=box.x+box.w*.5-screen.x,dy=box.y+box.h*.5-screen.y;
        if(std::abs(dx)<.01&&std::abs(dy)<.01)return;
        oe::SandboxView::PanKeyState keys;keys.left=dx>0;keys.right=dx<0;
        if(std::abs(dx)>.01)v.update(std::min(std::abs(dx)/400.,.05),keys);
        keys={};keys.up=dy>0;keys.down=dy<0;
        if(std::abs(dy)>.01)v.update(std::min(std::abs(dy)/400.,.05),keys);
    }
    throw std::runtime_error("camera did not reach production pan target");
}
void original_captures(const fs::path& data,const fs::path& root,SDL_Window* window,SDL_Renderer* renderer) {
    J report=J::array();
    for(const std::string name:{"Xia","Banpo","Anyi","Badaling"}) {
        const fs::path map="Cities/"+name+".map";
        auto session=m::load_stored_map_session(data,map,m::FootprintPolicy::EdgeByte4x4Preview,m::StoredGraphicsProfile::Slot8);
        std::vector<m::GridCell> elevation;for(const auto& c:session.plan.cells)if(c.slot==16&&c.physical_record)elevation.push_back(c.storage);
        check(!elevation.empty(),"original Elevation sample missing");
        const auto selected=name=="Xia"?m::GridCell{78,107}:elevation[elevation.size()/2];
        const auto focus=m::landscape_ground(session.plan,selected);
        const auto evidence=m::landscape_provenance(session.plan,selected,true);
        oe::SandboxView view(std::move(session),false,s::RulesProfile::CityV16,4);view.configure_save(data,map,root/(name+".json"));view.initialize(window,renderer);view.set_tool(5);
        if(!view.paused())key(view,SDLK_SPACE);const auto before=view.world().snapshot();
        check(view.authored_terrain_side_count()==0,"original map received authored cliff faces");
        for(int zoom=0;zoom<3;++zoom){center(view,focus);capture(view,renderer,root/(name+"-z"+std::to_string(zoom)+".png"));key(view,SDLK_Z);}
        key(view,SDLK_F1);for(int mode=0;mode<8;++mode){key(view,SDLK_F8);if(mode==0||mode==3)capture(view,renderer,root/(name+"-f8-"+std::to_string(mode)+".png"));}
        check(view.world().snapshot()==before,"original visual review changed World");view.shutdown();
        report.push_back({{"map",map.generic_string()},{"selected",{selected.x,selected.y}},{"provenance",evidence},{"authored_sides",0},{"original_first_draw_parity","unresolved"}});
    }
    std::ofstream(root/"originals.json")<<report.dump(2)<<'\n';
}
}
int main(int argc,char** argv){SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;try {
    check(argc==4||(argc==5&&(std::string(argv[4])=="--quick"||std::string(argv[4])=="--resume")),"usage: cliff-ramp-visual-fixture AUTHORED-DATA SAVE FRESH-.local-ROOT [--quick|--resume]; or DATA FRESH-ROOT --originals");
    const bool quick=argc==5;
    const bool resume=argc==5&&std::string(argv[4])=="--resume";
    const bool originals=std::string(argv[3])=="--originals";
    const auto data=fs::canonical(argv[1]),save=originals?fs::path{}:fs::canonical(argv[2]),root=fs::weakly_canonical(argv[originals?2:3]);
    const auto local=fs::canonical(".local");const auto relative=root.lexically_relative(local);
    check(!relative.empty()&&relative!="."&&*relative.begin()!=".."&&!fs::exists(root),"fresh ignored root required");fs::create_directories(root);
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy");check(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
    window=SDL_CreateWindow("Cliff Ramp evidence",1400,1000,SDL_WINDOW_HIDDEN);check(window,SDL_GetError());renderer=SDL_CreateRenderer(window,"software");check(renderer,SDL_GetError());
    if(originals){original_captures(data,root,window,renderer);SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();return 0;}
    const auto initial=p::read_save(save);check((resume||initial.world.ramps.empty())&&initial.world.bridges.size()==1,"requires existing authored Ramp/Bridge scene");
    const auto visuals=oe::detect_and_select_visual_profiles(data,fs::current_path()/"resources");J report={{"map",initial.map_relative.generic_string()},{"evidence","same authored map; normal paid commands; software/scripted only"},{"states",J::array()}};
    for(const bool sides:{true,false}) {
        if(quick&&!sides)continue;
        auto session=m::load_stored_map_session(data,initial.map_relative,m::FootprintPolicy::EdgeByte4x4Preview,m::StoredGraphicsProfile::Slot8);
        const auto border=session.plan.border;oe::SandboxView view(std::move(session),false,s::RulesProfile::CityV16,6);view.set_authored_terrain_sides(sides);
        if(!visuals.building.empty())view.set_building_visuals(visuals.building,visuals.building_source);
        if(!visuals.road.empty())view.set_road_visuals(visuals.road,visuals.road_source);
        if(!visuals.walker.empty())view.set_walker_visuals(visuals.walker,visuals.walker_source);
        const auto target=root/(sides?"manual.json":"manual-no-sides.json");view.configure_save(data,initial.map_relative,target,initial);view.initialize(window,renderer);view.set_tool(5);
        const auto command=[&](s::Command c){const auto result=view.execute(c);check(result.accepted,result.reason);};
        // The second specified Ramp uses the exact same source map, with real
        // paid road attachments around the existing House/Pottery footprints.
        if(!resume)for(const auto cell:{s::Cell{115,117},s::Cell{114,117},s::Cell{113,117},s::Cell{112,117},s::Cell{111,117},s::Cell{111,116},s::Cell{111,115},s::Cell{116,117},s::Cell{116,116},s::Cell{116,115}})
            command({s::CommandType::PlaceRoad,cell});
        for(int state=0;state<4;++state) {
            if(resume&&state!=(initial.world.ticks?3:int(initial.world.ramps.size())))continue;
            if(state==1&&!resume)command(s::place_ramp({115,114},{116,114}));
            if(state==2&&!resume)command(s::place_ramp({115,117},{116,117}));
            if(state==3&&!resume) {
                bool moving=false;for(int tick=0;tick<1000&&!moving;++tick){view.tick_once();for(const auto& c:view.world().couriers())
                    if(c.path_vertex+1<c.path.size()&&c.edge_progress==2&&view.world().ramp_between(c.path[c.path_vertex],c.path[c.path_vertex+1]))moving=true;}
                check(moving,"no actual loaded Walker crossed Ramp");
            }
            const auto before=view.world().snapshot();
            if(sides){view.save_now();fs::copy_file(target,root/("state-"+std::to_string(state)+".json"));}
            const auto focus=m::terrain_ground({115,115},border);const oe::scene::Point aim{focus.x,focus.y-10};
            for(int zoom=0;zoom<4;++zoom) {
                if(zoom==3){SDL_Event wheel{};wheel.type=SDL_EVENT_MOUSE_WHEEL;wheel.wheel.y=.5F;bool running=true;view.handle_event(wheel,running);}
                center(view,aim);const auto prefix=std::string(sides?"sides-":"plain-")+std::to_string(state)+"-z"+std::to_string(zoom);
                if(!resume&&(!quick||zoom==0||zoom==2))capture(view,renderer,root/(prefix+"-full.png"));
                if(zoom==0&&!quick) {
                    key(view,SDLK_F1);for(int mode=0;mode<8;++mode){key(view,SDLK_F8);if(mode==0||mode==1||mode==3)capture(view,renderer,root/(prefix+"-f8-"+std::to_string(mode)+".png"));}key(view,SDLK_F1);
                }
                if(zoom<3)key(view,SDLK_Z);
            }
            key(view,SDLK_R);center(view,aim); // Reset rebuild/camera stays view-only.
            // Road selection is deliberately view-local and clears on load.
            // Normalize it before comparing terrain pixels across a second load.
            key(view,SDLK_F5);key(view,SDLK_F9);
            park_pointer(view);oe::performance::set_enabled(true);oe::performance::reset();for(int i=0;i<100;++i)check(view.render(),"pure frame failed");
            for(std::size_t i=0;i<std::size_t(oe::performance::Counter::Count);++i)check(oe::performance::counter(static_cast<oe::performance::Counter>(i))==0,"forbidden render work");oe::performance::set_enabled(false);
            const auto box=view.layout().map;const SDL_Rect area{box.x,box.y,box.w,box.h};
            const auto image=pixels(renderer,&area);key(view,SDLK_F5);view.tick_once();key(view,SDLK_F9);check(view.world().snapshot()==before,"F5/F9 changed complete authority");
            if(sides){std::ofstream stable(root/("state-"+std::to_string(state)+"-stable.png"),std::ios::binary);stable.write(reinterpret_cast<const char*>(image.data()),std::streamsize(image.size()));check(bool(stable),"stable PNG write failed");}
            park_pointer(view);check(view.render(),"restore render failed");const auto restored=pixels(renderer,&area);
            if(restored!=image){std::ofstream a(root/"restore-before.png",std::ios::binary),b(root/"restore-after.png",std::ios::binary);a.write(reinterpret_cast<const char*>(image.data()),std::streamsize(image.size()));b.write(reinterpret_cast<const char*>(restored.data()),std::streamsize(restored.size()));throw std::runtime_error("F5/F9 changed exact terrain composition");}
            check(view.world().snapshot()==before,"visual controls changed World");
            if(sides)check(view.authored_terrain_open_edges()==before.ramps.size(),"committed visual openings differ from actual RampLinks");
            report["states"].push_back({{"sides",sides},{"state",state},{"tick",before.ticks},{"funds",before.treasury},{"ramps",before.ramps.size()},{"camera_zoom",view.camera().zoom},{"pure_frames",100},
                {"terrain_edges",view.authored_terrain_side_count()},{"terrain_geometry_bytes",view.authored_terrain_geometry_bytes()},{"cutouts",view.authored_terrain_open_edges()}});
        }
        view.shutdown();
    }
    check(oe::StoredGraphicsRenderer::live_texture_count()==0,"textures leaked");std::ofstream(root/"summary.json")<<report.dump(2)<<'\n';
    SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();std::cout<<"Cliff/Ramp same-scene paid reconstruction, captures, pure frames and F5/F9 PASS\n";return 0;
}catch(const std::exception& e){SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();std::cerr<<e.what()<<'\n';return 1;}}
