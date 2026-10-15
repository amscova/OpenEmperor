// Isolated local acceptance: the existing authored terrain and normal paid
// commands through the actual SandboxView painter. No synthetic World edits.
#include "app/SandboxView.h"
#include "app/VisualSelection.h"
#include "assets/RgbaPngEncoder.h"
#include "core/PerformanceDiagnostics.h"
#include "renderer/RampRenderer.h"
#include <SDL3/SDL.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <set>

namespace oe=openemperor; namespace s=oe::simulation; namespace m=oe::maps;
namespace p=oe::persistence; namespace fs=std::filesystem; using J=nlohmann::json;
namespace {
void check(bool value,const std::string& message) { if(!value)throw std::runtime_error(message); }
void key(oe::SandboxView& view,SDL_Keycode code) {
    SDL_Event event{};event.type=SDL_EVENT_KEY_DOWN;event.key.key=code;
    bool running=true;view.handle_event(event,running);check(running,"unexpected fixture quit");
}
void park(oe::SandboxView& view) {
    SDL_Event event{};event.type=SDL_EVENT_MOUSE_MOTION;event.motion.x=0;event.motion.y=0;
    bool running=true;view.handle_event(event,running);view.update(0,{});
}
oe::assets::RgbaImage pixels(SDL_Renderer* renderer,const SDL_Rect* area=nullptr) {
    auto* source=SDL_RenderReadPixels(renderer,area);check(source,SDL_GetError());
    auto* rgba=SDL_ConvertSurface(source,SDL_PIXELFORMAT_RGBA32);SDL_DestroySurface(source);check(rgba,SDL_GetError());
    oe::assets::RgbaImage image;image.width=std::uint16_t(rgba->w);image.height=std::uint16_t(rgba->h);
    image.pixels.resize(std::size_t(rgba->w*rgba->h*4));
    for(int y=0;y<rgba->h;++y)std::copy_n(static_cast<const std::uint8_t*>(rgba->pixels)+y*rgba->pitch,
        rgba->w*4,image.pixels.data()+y*rgba->w*4);
    SDL_DestroySurface(rgba);return image;
}
void write_png(const oe::assets::RgbaImage& image,const fs::path& path) {
    const auto encoded=oe::assets::encode_rgba_png(image);std::ofstream file(path,std::ios::binary);
    file.write(reinterpret_cast<const char*>(encoded.data()),std::streamsize(encoded.size()));check(bool(file),"PNG write failed");
}
void center(oe::SandboxView& view,oe::scene::Point aim) {
    for(int i=0;i<800;++i) {
        const auto screen=view.camera().world_to_screen(aim);const auto map=view.layout().map;
        const double dx=map.x+map.w*.5-screen.x,dy=map.y+map.h*.5-screen.y;
        if(std::abs(dx)<.01&&std::abs(dy)<.01)return;
        oe::SandboxView::PanKeyState keys;keys.left=dx>0;keys.right=dx<0;
        if(std::abs(dx)>.01)view.update(std::min(std::abs(dx)/400.,.05),keys);
        keys={};keys.up=dy>0;keys.down=dy<0;
        if(std::abs(dy)>.01)view.update(std::min(std::abs(dy)/400.,.05),keys);
    }
    throw std::runtime_error("production camera pan failed");
}
J camera(const oe::SandboxView& view) {
    const auto c=view.camera();const auto box=view.layout().map;
    return {{"zoom",c.zoom},{"offset",{c.offset.x,c.offset.y}},{"map_rect",{box.x,box.y,box.w,box.h}}};
}
J scan(const oe::assets::RgbaImage& image,const oe::SandboxView& view,std::uint32_t border,int state) {
    // Capture literal composed pixels along actual projected shared boundaries.
    // No classifier changes art or selects an asset; values are diagnostics only.
    J lines=J::array();
    for(const int y:{114,117,118,119})for(const double side:{-.85,-.5,0.,.5,.85}) {
        if((state==1)!=(y==114))continue;
        s::RampLink link{};link.endpoint_a={115,y};link.endpoint_b={116,y};link.height_a=1;link.height_b=0;
        auto origin=m::terrain_ground({115,std::uint32_t(y)},border);origin.y-=40;
        J samples=J::array();std::set<std::array<int,3>> colors;double luminance=0;int dark=0,background=0;
        for(int i=0;i<25;++i) {
            const double t=.2+.6*double(i)/24;
            const auto world=oe::ramp_surface_point(origin,link,t,side),screen=view.camera().world_to_screen(world);
            const int x=int(std::floor(screen.x)),py=int(std::floor(screen.y));
            const auto box=view.layout().map;
            check(x>=box.x&&py>=box.y&&x<box.x+box.w&&py<box.y+box.h,"diagnostic outside composed map viewport");
            const auto index=std::size_t((py*image.width+x)*4);const int r=image.pixels[index],g=image.pixels[index+1],b=image.pixels[index+2];
            colors.insert({r,g,b});luminance+=.2126*r+.7152*g+.0722*b;
            if(r+g+b<300)++dark;if(r==22&&g==26&&b==32)++background;
            samples.push_back({r,g,b});
        }
        lines.push_back({{"endpoint_y",y},{"lateral_offset",side},{"unique_colors",colors.size()},
            {"mean_luminance",luminance/25},{"dark_samples",dark},{"background_samples",background},{"rgb",samples}});
    }
    return lines;
}
void paid_roads(oe::SandboxView& view,J& commands) {
    std::vector<s::Cell> roads;
    for(int y=115;y<=119;++y)roads.push_back({111,y});
    for(int x=112;x<=115;++x)roads.push_back({x,117});
    for(int y=115;y<=119;++y)roads.push_back({116,y});
    roads.push_back({115,118});roads.push_back({115,119});
    for(const auto cell:roads) {
        if(view.world().road_cell(cell))continue;
        const auto before=view.world().treasury();const auto result=view.execute({s::CommandType::PlaceRoad,cell});
        check(result.accepted,"paid road "+std::to_string(cell.x)+","+std::to_string(cell.y)+": "+result.reason);
        commands.push_back({{"kind","road"},{"cell",{cell.x,cell.y}},{"cost",before-view.world().treasury()}});
    }
}
void paid_ramp(oe::SandboxView& view,int y,J& commands) {
    const auto before=view.world().treasury();const auto result=view.execute(s::place_ramp({115,y},{116,y}));
    check(result.accepted,"paid ramp y="+std::to_string(y)+": "+result.reason);
    check(before-view.world().treasury()==40,"ramp did not charge exact unchanged price");
    commands.push_back({{"kind","ramp"},{"a",{115,y}},{"b",{116,y}},{"cost",40}});
}
J topology(const oe::SandboxView& view) {
    J result=J::array();
    for(const auto& ramp:view.world().ramps()) {
        const unsigned expected=ramp.endpoint_a.y==117?2U:ramp.endpoint_a.y==118?3U:ramp.endpoint_a.y==119?1U:0U;
        const unsigned actual=view.ramp_visual_adjacency(ramp.id,true);
        check(actual==expected,"production presentation topology differs from paid source links");
        result.push_back({{"id",ramp.id.value},{"endpoint_y",ramp.endpoint_a.y},{"adjacency",actual}});
    }
    return result;
}
J preview_checks(oe::SandboxView& view,SDL_Renderer* renderer,std::uint32_t border,const fs::path& root) {
    J commands=J::array();paid_ramp(view,117,commands);paid_ramp(view,119,commands);
    key(view,SDLK_F5);key(view,SDLK_F9);view.set_tool(5);
    auto aim=m::terrain_ground({115,118},border);aim.y-=10;center(view,aim);park(view);
    check(view.render(),SDL_GetError());const auto box=view.layout().map;const SDL_Rect area{box.x,box.y,box.w,box.h};
    const auto stable=pixels(renderer,&area);write_png(pixels(renderer),root/"preview-before.png");
    const auto before=view.world().snapshot();const auto first=view.world().ramps().front().id,last=view.world().ramps().back().id;
    check(view.ramp_visual_adjacency(first)==0&&view.ramp_visual_adjacency(last)==0,"nonadjacent outer links merged across a missing road slope");
    auto control=s::World::restore(before,view.world().map_permissions());
    check(control.execute(s::place_ramp({115,118},{116,118})).accepted,"preview control paid Ramp rejected");
    const auto point=[&](s::Cell cell) {
        auto ground=m::terrain_ground({unsigned(cell.x),unsigned(cell.y)},border);
        ground.y-=view.world().map_permissions()->cell_height(cell)*40;
        const auto screen=view.camera().world_to_screen(ground);check(view.pick(screen)==cell,"preview endpoint picking differs from projected feet");
        SDL_FPoint raw{};check(SDL_RenderCoordinatesToWindow(renderer,float(screen.x),float(screen.y),&raw.x,&raw.y),SDL_GetError());return raw;
    };
    const auto a=point({115,118}),b=point({116,118});
    const auto begin=[&] {
        key(view,SDLK_P);SDL_Event event{};bool running=true;
        event.type=SDL_EVENT_MOUSE_BUTTON_DOWN;event.button.button=SDL_BUTTON_LEFT;event.button.x=a.x;event.button.y=a.y;view.handle_event(event,running);
        event={};event.type=SDL_EVENT_MOUSE_MOTION;event.motion.x=b.x;event.motion.y=b.y;view.handle_event(event,running);view.update(0,{});
        check(view.world().snapshot()==before,"coalesced neighbor preview changed World");
        check(view.ramp_visual_adjacency(first)==2&&view.ramp_visual_adjacency(last)==1,"preview failed to merge adjacent existing slope presentation");
        check(view.ramp_visual_adjacency(first,true)==0&&view.ramp_visual_adjacency(last,true)==0,"preview mutated committed presentation topology");
    };
    begin();check(view.render(),SDL_GetError());write_png(pixels(renderer),root/"preview-middle.png");
    oe::performance::set_enabled(true);oe::performance::reset();
    for(int i=0;i<100;++i){view.update(0,{});check(view.render(),"pure neighbor preview render failed");}
    for(std::size_t i=0;i<std::size_t(oe::performance::Counter::Count);++i)
        check(oe::performance::counter(static_cast<oe::performance::Counter>(i))==0,"coalesced preview did forbidden work");
    oe::performance::set_enabled(false);key(view,SDLK_ESCAPE);view.set_tool(5);park(view);check(view.render(),SDL_GetError());
    check(view.world().snapshot()==before,"preview cancellation changed paid World");
    check(view.ramp_visual_adjacency(first)==0&&view.ramp_visual_adjacency(last)==0,"cancel retained merged preview adjacency");
    check(pixels(renderer,&area).pixels==stable.pixels,"cancel did not restore exact composed cliff and existing ramps");
    write_png(pixels(renderer),root/"preview-cancel.png");begin();
    SDL_Event release{};release.type=SDL_EVENT_MOUSE_BUTTON_UP;release.button.button=SDL_BUTTON_LEFT;
    release.button.x=b.x;release.button.y=b.y;bool running=true;view.handle_event(release,running);
    check(view.world().snapshot()==control.snapshot(),"normal paid preview release differs from production command");
    view.set_tool(5);key(view,SDLK_F5);key(view,SDLK_F9);park(view);check(view.render(),SDL_GetError());
    write_png(pixels(renderer),root/"preview-committed.png");const auto links=topology(view);
    return {{"paid_preparation",commands},{"preview_world_exact",true},{"cancel_map_pixels_exact",true},
        {"normal_release_equals_paid_command",true},{"coalesced_frames",100},{"committed_topology",links}};
}
}
int main(int argc,char** argv) {
    SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;
    try {
        check(argc==4||(argc==5&&(std::string(argv[4])=="--quick"||std::string(argv[4])=="--resume")),
            "usage: multi-ramp-visual-fixture AUTHORED-DATA PAID-PREBUILD-SAVE FRESH-.local-ROOT [--quick|--resume]");
        const bool quick=argc==5&&std::string(argv[4])=="--quick",resume=argc==5&&std::string(argv[4])=="--resume";
        const auto data=fs::canonical(argv[1]),save=fs::canonical(argv[2]),root=fs::weakly_canonical(argv[3]);
        const auto relative=root.lexically_relative(fs::canonical(".local"));
        check(!relative.empty()&&relative!="."&&*relative.begin()!=".."&&!fs::exists(root),"fresh ignored output root required");
        fs::create_directories(root);const auto initial=p::read_save(save);
        check((resume||initial.world.ramps.empty())&&initial.world.bridges.size()==1,"existing authored paid prebuild scene required");
        SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy");check(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
        window=SDL_CreateWindow("Paid multi-ramp production scene",1400,1000,SDL_WINDOW_HIDDEN);check(window,SDL_GetError());
        renderer=SDL_CreateRenderer(window,"software");check(renderer,SDL_GetError());
        const auto visuals=oe::detect_and_select_visual_profiles(data,fs::current_path()/"resources");
        check(!visuals.road.empty(),"real runtime road profile required");
        J report={{"evidence","actual SandboxView; existing authored terrain; paid commands; software/scripted only"},
            {"map",initial.map_relative.generic_string()},{"road_profile",visuals.road.generic_string()},{"states",J::array()}};
        const std::array<const char*,4> names{"zero","isolated","group","four"};
        for(int state=0;state<4;++state) {
            if(resume&&initial.world.ramps.size()!=std::array<std::size_t,4>{0,1,3,4}[std::size_t(state)])continue;
            auto session=m::load_stored_map_session(data,initial.map_relative,m::FootprintPolicy::EdgeByte4x4Preview,m::StoredGraphicsProfile::Slot8);
            const auto border=session.plan.border;oe::SandboxView view(std::move(session),false,s::RulesProfile::CityV16,6);
            view.set_authored_terrain_sides(true);view.set_road_visuals(visuals.road,visuals.road_source);
            if(!visuals.building.empty())view.set_building_visuals(visuals.building,visuals.building_source);
            if(!visuals.walker.empty())view.set_walker_visuals(visuals.walker,visuals.walker_source);
            const auto manual=root/"manual.json";view.configure_save(data,initial.map_relative,manual,initial);view.initialize(window,renderer);view.set_tool(5);
            if(!view.paused())key(view,SDLK_SPACE);J commands=J::array();
            if(!resume) {
                paid_roads(view,commands);
                if(state==1||state==3)paid_ramp(view,114,commands);
                if(state>=2)for(const int y:{117,118,119})paid_ramp(view,y,commands);
            }
            const auto authority=view.world().snapshot();const auto initial_topology=topology(view);
            check(view.authored_terrain_open_edges()==authority.ramps.size(),"prepared openings differ from owned ramp count");
            key(view,SDLK_F5);key(view,SDLK_F9);park(view);view.save_now();
            fs::copy_file(manual,root/(std::string(names[std::size_t(state)])+".json"));
            auto focus=m::terrain_ground({115,state==1?114U:117U},border);focus.y-=10;
            J captures=J::array();
            for(int zoom=0;zoom<4;++zoom) {
                if(zoom==3) {SDL_Event e{};e.type=SDL_EVENT_MOUSE_WHEEL;e.wheel.y=.5F;bool running=true;view.handle_event(e,running);}
                center(view,focus);park(view);check(view.render(),SDL_GetError());
                const auto prefix=std::string(names[std::size_t(state)])+"-z"+std::to_string(zoom);
                const auto rgba=pixels(renderer);
                if(!quick||zoom==0||zoom==2)write_png(rgba,root/(prefix+"-full.png"));
                captures.push_back({{"file",prefix+"-full.png"},{"camera",camera(view)},{"projected_lines",scan(rgba,view,border,state)}});
                if(!quick&&zoom==0) {
                    key(view,SDLK_F6);park(view);check(view.render(),SDL_GetError());write_png(pixels(renderer),root/(prefix+"-f6-off.png"));key(view,SDLK_F6);
                    key(view,SDLK_F1);
                    for(int mode=0;mode<8;++mode) {key(view,SDLK_F8);if(mode==0||mode==1||mode==3){park(view);check(view.render(),SDL_GetError());write_png(pixels(renderer),root/(prefix+"-f8-"+std::to_string(mode)+".png"));}}
                    key(view,SDLK_F1);
                }
                if(zoom<3)key(view,SDLK_Z);
            }
            key(view,SDLK_R);center(view,focus);park(view);check(view.render(),SDL_GetError());
            const auto box=view.layout().map;const SDL_Rect area{box.x,box.y,box.w,box.h};const auto before=pixels(renderer,&area);
            write_png(before,root/(std::string(names[std::size_t(state)])+"-stable.png"));
            key(view,SDLK_F5);view.tick_once();key(view,SDLK_F9);park(view);check(view.render(),SDL_GetError());
            check(view.world().snapshot()==authority,"F5/F9 changed complete authority");
            check(topology(view)==initial_topology,"F5/F9 did not reconstruct presentation topology exactly");
            check(pixels(renderer,&area).pixels==before.pixels,"F5/F9 changed composed map pixels");
            oe::performance::set_enabled(true);oe::performance::reset();const auto start=std::chrono::steady_clock::now();
            for(int frame=0;frame<100;++frame)check(view.render(),"render loop failed");
            const auto elapsed=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
            J counters=J::object();
            for(std::size_t i=0;i<std::size_t(oe::performance::Counter::Count);++i) {
                const auto value=oe::performance::counter(static_cast<oe::performance::Counter>(i));
                counters[std::string(oe::performance::counter_names[i])]=value;check(value==0,"forbidden production render work");
            }
            oe::performance::set_enabled(false);
            check(view.world().snapshot()==authority,"visual controls mutated World");
            report["states"].push_back({{"name",names[std::size_t(state)]},{"ramps",authority.ramps.size()},
                {"funds",authority.treasury},{"commands",commands},{"captures",captures},{"pure_frames",100},
                {"pure_ms",elapsed},{"counters",counters},{"terrain_edges",view.authored_terrain_side_count()},
                {"terrain_bytes",view.authored_terrain_geometry_bytes()},{"cutouts",view.authored_terrain_open_edges()},
                {"topology",initial_topology},{"f5_f9_authority_exact",true},{"f5_f9_map_pixels_exact",true}});
            if(state==0&&!resume)report["preview"]=preview_checks(view,renderer,border,root);
            std::ofstream(root/"summary.json")<<report.dump(2)<<'\n';view.shutdown();
        }
        check(oe::StoredGraphicsRenderer::live_texture_count()==0,"runtime textures leaked");
        SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();
        std::cout<<"Paid isolated/three-adjacent/four-ramp production scene, pure frames and F5/F9 PASS\n";return 0;
    } catch(const std::exception& error) {
        oe::performance::set_enabled(false);SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();std::cerr<<error.what()<<'\n';return 1;
    }
}
