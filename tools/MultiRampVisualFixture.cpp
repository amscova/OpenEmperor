// Isolated local acceptance: the existing authored terrain and normal paid
// commands through the actual SandboxView painter. No synthetic World edits.
#include "app/SandboxView.h"
#include "app/VisualSelection.h"
#include "assets/RgbaPngEncoder.h"
#include "assets/RoadVisualProfile.h"
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
void write_crop(const oe::assets::RgbaImage& image,SDL_Rect area,const fs::path& path) {
    area.x=std::clamp(area.x,0,int(image.width)-1);area.y=std::clamp(area.y,0,int(image.height)-1);
    area.w=std::min(area.w,int(image.width)-area.x);area.h=std::min(area.h,int(image.height)-area.y);
    oe::assets::RgbaImage crop;crop.width=std::uint16_t(area.w);crop.height=std::uint16_t(area.h);
    crop.pixels.resize(std::size_t(area.w*area.h*4));
    for(int y=0;y<area.h;++y)std::copy_n(image.pixels.data()+std::size_t(((area.y+y)*image.width+area.x)*4),
        area.w*4,crop.pixels.data()+std::size_t(y*area.w*4));
    write_png(crop,path);
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
        if((state==1||state==4)!=(y==114))continue;
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
J walker_witness(const oe::SandboxView& view) {
    for(const auto& c:view.world().couriers())if(c.path_vertex+1<c.path.size()&&c.edge_progress==2) {
        const auto a=c.path[c.path_vertex],b=c.path[c.path_vertex+1];
        if(const auto* ramp=view.world().ramp_between(a,b))return {{"courier",unsigned(c.id)},
            {"role",unsigned(c.role)},{"cargo",c.cargo},{"edge_progress",c.edge_progress},
            {"a",{a.x,a.y}},{"b",{b.x,b.y}},{"ramp_id",ramp->id.value}};
    }
    return nullptr;
}
// This atlas calls the production prepared RampPresentation directly. Its
// authored links are renderer inputs only, never injected into a World/save.
// The separate paid Sandbox captures above remain the scene-composition proof.
J material_atlas(SDL_Renderer* renderer,const fs::path& data,const fs::path& road,const fs::path& root,int lateral_start=10) {
    const auto profile=oe::assets::load_road_visual_profile(data,road);
    oe::RampPresentation presentation;presentation.prepare(&profile);J cases=J::array();
    const auto out=root/"material-atlas";fs::create_directories(out);
    for(const bool axis:{true,false})for(const int delta:{-1,1})for(const int base:{-2,0,2})
    for(const int count:{1,2,3,4})for(const double zoom:{1.,2.,4.}) {
        const auto name=std::string(axis?"x":"y")+"-d"+std::to_string(delta)+"-h"+std::to_string(base)+
            "-n"+std::to_string(count)+"-z"+std::to_string(int(zoom));
        std::vector<s::RampLink> links;std::vector<oe::scene::Point> origins;
        for(int i=0;i<count;++i) {
            const s::Cell a{axis?10:lateral_start+i,axis?lateral_start+i:10},b{a.x+(axis?1:0),a.y+(axis?0:1)};
            links.push_back({s::RampId{unsigned(i+1)},a,b,std::int8_t(base),std::int8_t(base+delta),40,0});
            origins.push_back({double((axis?-40:40)*i),double(20*i-base*40)});
        }
        oe::scene::Camera2D camera;camera.zoom=zoom;
        const auto midpoint=oe::ramp_surface_point(origins[std::size_t(count/2)],links[std::size_t(count/2)],.5,0);
        camera.offset={700-midpoint.x*zoom,480-midpoint.y*zoom};
        const auto draw=[&](Uint8 red,Uint8 green,Uint8 blue) {
            check(SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE)&&SDL_SetRenderDrawColor(renderer,red,green,blue,255)&&SDL_RenderClear(renderer),SDL_GetError());
            for(int i=0;i<count;++i) {
                const unsigned adjacency=(i?1U:0U)|(i+1<count?2U:0U);
                for(int half=0;half<2;++half)for(const bool front:{false,true})
                    check(presentation.draw_part(renderer,camera,origins[std::size_t(i)],links[std::size_t(i)],half,front,true,
                        oe::BridgeTint::Normal,adjacency,true),"atlas production Ramp draw failed");
            }
            return pixels(renderer);
        };
        const auto first=draw(21,56,101),second=draw(200,20,70);
        J witnesses=J::array();int xmin=1400,ymin=1000,xmax=0,ymax=0;
        for(int y=0;y<1000;++y)for(int x=0;x<1400;++x) {
            const auto at=std::size_t((y*1400+x)*4);
            if(first.pixels[at]==21&&first.pixels[at+1]==56&&first.pixels[at+2]==101)continue;
            xmin=std::min(xmin,x);ymin=std::min(ymin,y);xmax=std::max(xmax,x);ymax=std::max(ymax,y);
        }
        check(xmin>0&&ymin>0&&xmax<1399&&ymax<999,"atlas geometry clipped by viewport");
        for(int i=0;i<count;++i)for(const double t:{.10,.25,.48,.50,.52,.75,.90})
        for(const double side:{-.20,0.,.20}) {
            const auto point=camera.world_to_screen(oe::ramp_surface_point(origins[std::size_t(i)],links[std::size_t(i)],t,side));
            const int x=int(std::floor(point.x)),y=int(std::floor(point.y));const auto at=std::size_t((y*1400+x)*4);
            const bool opaque=std::equal(first.pixels.begin()+std::ptrdiff_t(at),first.pixels.begin()+std::ptrdiff_t(at+3),second.pixels.begin()+std::ptrdiff_t(at));
            // Ascending slopes are exactly edge-on under the unchanged2:1
            // projection. Only their authoritative center footline is an
            // interior witness; lateral coordinates collapse onto its border.
            const bool required=delta<0||side==0.;
            check(!required||opaque,"atlas slope/half seam depends on clear background: "+name+" t="+std::to_string(t)+" side="+std::to_string(side));
            witnesses.push_back({{"ramp",i},{"t",t},{"side",side},{"opaque",opaque},{"opacity_required",required},
                {"rgb",{first.pixels[at],first.pixels[at+1],first.pixels[at+2]}}});
        }
        const SDL_Rect crop{xmin-4,ymin-4,xmax-xmin+9,ymax-ymin+9};draw(21,56,101);
        write_png(pixels(renderer,&crop),out/(name+".png"));
        cases.push_back({{"file",std::string("material-atlas/")+name+".png"},{"axis",axis?"X":"Y"},
            {"delta",delta},{"base_height",base},{"ramps",count},{"zoom",zoom},
            {"two_background_witnesses",witnesses},{"crop",{crop.x,crop.y,crop.w,crop.h}}});
    }
    return {{"evidence","production RampPresentation; authored renderer inputs, no World/save authority"},
        {"cases",cases},{"lateral_start",lateral_start},{"geometry_bytes",presentation.geometry_bytes()}};
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
#ifndef OE_MULTI_RAMP_BASELINE
    for(const auto& ramp:view.world().ramps()) {
        const unsigned expected=ramp.endpoint_a.y==117?2U:ramp.endpoint_a.y==118?3U:ramp.endpoint_a.y==119?1U:0U;
        const unsigned actual=view.ramp_visual_adjacency(ramp.id,true);
        check(actual==expected,"production presentation topology differs from paid source links");
        result.push_back({{"id",ramp.id.value},{"endpoint_y",ramp.endpoint_a.y},{"adjacency",actual}});
    }
#else
    (void)view; // Exact old HEAD has no adjacency presentation API.
#endif
    return result;
}
J preview_checks(oe::SandboxView& view,SDL_Renderer* renderer,std::uint32_t border,const fs::path& root) {
#ifdef OE_MULTI_RAMP_BASELINE
    (void)view;(void)renderer;(void)border;(void)root;
    return {{"evidence","exact old HEAD; new presentation-topology assertions unavailable"}};
#else
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
#endif
}
std::size_t ramp_geometry_bytes(const oe::SandboxView& view) {
#ifdef OE_MULTI_RAMP_BASELINE
    (void)view;return 0;
#else
    return view.ramp_geometry_bytes();
#endif
}
}
int main(int argc,char** argv) {
    SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;
    try {
        check(argc==4||(argc==5&&(std::string(argv[4])=="--quick"||std::string(argv[4])=="--resume"||std::string(argv[4])=="--atlas"||std::string(argv[4])=="--atlas-wrap")),
            "usage: multi-ramp-visual-fixture AUTHORED-DATA PAID-PREBUILD-SAVE FRESH-.local-ROOT [--quick|--resume|--atlas|--atlas-wrap]");
        const bool quick=argc==5&&std::string(argv[4])=="--quick",resume=argc==5&&std::string(argv[4])=="--resume";
        const bool wrap=argc==5&&std::string(argv[4])=="--atlas-wrap";
        const bool atlas_only=argc==5&&(std::string(argv[4])=="--atlas"||wrap);
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
        const std::array<const char*,5> names{"zero","isolated","group","four","walker"};
        for(int state=0;state<5&&!atlas_only;++state) {
            if(resume&&(initial.world.ramps.size()!=std::array<std::size_t,5>{0,1,3,4,4}[std::size_t(state)]||
                (initial.world.ticks>0)!=(state==4)))continue;
            auto session=m::load_stored_map_session(data,initial.map_relative,m::FootprintPolicy::EdgeByte4x4Preview,m::StoredGraphicsProfile::Slot8);
            const auto border=session.plan.border;oe::SandboxView view(std::move(session),false,s::RulesProfile::CityV16,6);
            view.set_authored_terrain_sides(true);view.set_road_visuals(visuals.road,visuals.road_source);
            if(!visuals.building.empty())view.set_building_visuals(visuals.building,visuals.building_source);
            if(!visuals.walker.empty())view.set_walker_visuals(visuals.walker,visuals.walker_source);
            const auto manual=root/"manual.json";view.configure_save(data,initial.map_relative,manual,initial);view.initialize(window,renderer);view.set_tool(5);
            if(!view.paused())key(view,SDLK_SPACE);J commands=J::array();
            if(!resume) {
                paid_roads(view,commands);
                if(state==1||state>=3)paid_ramp(view,114,commands);
                if(state>=2)for(const int y:{117,118,119})paid_ramp(view,y,commands);
                if(state==4) {
                    for(int tick=0;tick<1000&&walker_witness(view).is_null();++tick)view.tick_once();
                    check(!walker_witness(view).is_null(),"no actual courier reached the paid Ramp");
                }
            }
            const auto authority=view.world().snapshot();const auto initial_topology=topology(view);
            check(view.authored_terrain_open_edges()==authority.ramps.size(),"prepared openings differ from owned ramp count");
            key(view,SDLK_F5);key(view,SDLK_F9);park(view);view.save_now();
            fs::copy_file(manual,root/(std::string(names[std::size_t(state)])+".json"));
            auto focus=m::terrain_ground({115,state==1||state==4?114U:117U},border);focus.y-=10;
            J captures=J::array();
            for(int zoom=0;zoom<4;++zoom) {
                if(zoom==3) {
                    const auto map=view.layout().map;SDL_FPoint raw{};
                    check(SDL_RenderCoordinatesToWindow(renderer,float(map.x+map.w/2),float(map.y+map.h/2),&raw.x,&raw.y),SDL_GetError());
                    SDL_Event e{};e.type=SDL_EVENT_MOUSE_WHEEL;e.wheel.y=.5F;e.wheel.mouse_x=raw.x;e.wheel.mouse_y=raw.y;
                    bool running=true;view.handle_event(e,running);
                    check(view.camera().zoom>1&&view.camera().zoom<2,"fractional wheel did not reach production map viewport");
                }
                center(view,focus);park(view);check(view.render(),SDL_GetError());
                const auto prefix=std::string(names[std::size_t(state)])+"-z"+std::to_string(zoom);
                const auto rgba=pixels(renderer);
                if(!quick||zoom==0||zoom==2)write_png(rgba,root/(prefix+"-full.png"));
                J joins=J::array();
                for(const int y:(state==1||state==4?std::vector<int>{114}:std::vector<int>{118}))
                for(const int x:{115,116}) {
                    auto ground=m::terrain_ground({unsigned(x),unsigned(y)},border);
                    ground.y-=view.world().map_permissions()->cell_height({x,y})*40;
                    const auto point=view.camera().world_to_screen(ground);const double scale=view.camera().zoom;
                    const auto file=prefix+(x==115?"-upper-join.png":"-lower-join.png");
                    const SDL_Rect crop{int(std::floor(point.x-48*scale)),int(std::floor(point.y-40*scale)),int(96*scale),int(80*scale)};
                    if(!quick)write_crop(rgba,crop,root/file);
                    joins.push_back({{"file",file},{"cell",{x,y}},{"screen",{point.x,point.y}},
                        {"height",view.world().map_permissions()->cell_height({x,y})}});
                }
                captures.push_back({{"file",prefix+"-full.png"},{"camera",camera(view)},
                    {"joins",joins},{"projected_lines",scan(rgba,view,border,state)}});
                if(!quick) {
                    key(view,SDLK_F6);park(view);check(view.render(),SDL_GetError());write_png(pixels(renderer),root/(prefix+"-f6-off.png"));key(view,SDLK_F6);
                }
                if(!quick&&zoom==0) {
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
                {"funds",authority.treasury},{"tick",authority.ticks},{"walker",walker_witness(view)},
                {"commands",commands},{"captures",captures},{"pure_frames",100},
                {"pure_ms",elapsed},{"counters",counters},{"terrain_edges",view.authored_terrain_side_count()},
                {"terrain_bytes",view.authored_terrain_geometry_bytes()},{"ramp_geometry_bytes",ramp_geometry_bytes(view)},
                {"cutouts",view.authored_terrain_open_edges()},
                {"topology",initial_topology},{"f5_f9_authority_exact",true},{"f5_f9_map_pixels_exact",true}});
            if(state==0&&!resume)report["preview"]=preview_checks(view,renderer,border,root);
            std::ofstream(root/"summary.json")<<report.dump(2)<<'\n';view.shutdown();
        }
        if(!resume&&!quick) {
            report["material_atlas"]=material_atlas(renderer,data,visuals.road,root,wrap?6:10);
            std::ofstream(root/"summary.json")<<report.dump(2)<<'\n';
        }
        check(oe::StoredGraphicsRenderer::live_texture_count()==0,"runtime textures leaked");
        SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();
        std::cout<<(atlas_only?"Production material atlas and two-background seam witnesses PASS\n":
            "Paid isolated/three-adjacent/four-ramp/Walker production scene, pure frames and F5/F9 PASS\n");return 0;
    } catch(const std::exception& error) {
        oe::performance::set_enabled(false);SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();std::cerr<<error.what()<<'\n';return 1;
    }
}
