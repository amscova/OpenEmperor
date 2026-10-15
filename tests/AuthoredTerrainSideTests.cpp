#include "renderer/AuthoredTerrainSides.h"
#include "renderer/RampRenderer.h"
#include "maps/LandscapeProvenance.h"
#include "core/PerformanceDiagnostics.h"
#include <SDL3/SDL.h>
#include <iostream>
#include <set>
#include <stdexcept>
#include <algorithm>
#include <vector>
namespace oe=openemperor;namespace m=oe::maps;namespace s=oe::simulation;
namespace {
void check(bool ok,const char* reason){if(!ok)throw std::runtime_error(reason);}
struct Fixture {
    m::MapGeometry geometry{84};m::StoredGraphicsPlan plan;
    std::vector<s::MapCellPermission> cells;
    Fixture(s::Cell high,s::Cell low,int height) {
        constexpr std::size_t count=228U*228U;
        plan.border=geometry.border;plan.landscape_layers_available=true;
        plan.height_bytes.assign(count,std::uint8_t(height-1));plan.raw_terrain.assign(count,0x80000);plan.raw_objects.assign(count,0);
        cells.resize(count);
        for(const auto cell:{high,low}) {
            const auto at=std::size_t(cell.y)*228+unsigned(cell.x);
            plan.raw_terrain[at]=0x80;auto& p=cells[at];p.road_allowed=p.building_allowed=true;
            p.road_blocker=p.building_blocker=s::BuildBlocker::None;p.height=std::int8_t(cell==high?height:height-1);
            plan.height_bytes[at]=std::uint8_t(p.height);
        }
    }
    std::shared_ptr<const s::MapPermissions> permissions(unsigned policy=3) const {
        return std::make_shared<const s::MapPermissions>(228,228,policy,cells,std::vector<s::FixedGatePassage>{});
    }
};
void topology_checks() {
    for(const bool axis_x:{false,true})for(const int delta:{-1,1})for(const int base_height:{-7,1}) {
        const s::Cell step=axis_x?s::Cell{0,1}:s::Cell{1,0};
        const s::Cell along=axis_x?s::Cell{1,0}:s::Cell{0,1};
        std::array<s::RampLink,3> links{};
        for(int i=0;i<3;++i)links[std::size_t(i)]={{unsigned(i+1)},
            {114+step.x*i,114+step.y*i},{114+step.x*i+along.x,114+step.y*i+along.y},
            std::int8_t(base_height),std::int8_t(base_height+delta),40,0};
        oe::RampTopology topology;topology.prepare(links);
        check(topology.effective_mask({1})==2&&topology.effective_mask({2})==3&&topology.effective_mask({3})==1,
            "canonical X/Y lateral group has wrong negative/positive masks");
        std::reverse(links.begin(),links.end());topology.prepare(links);
        check(topology.effective_mask({1})==2&&topology.effective_mask({2})==3&&topology.effective_mask({3})==1,
            "ramp slope group depends on source publication order");std::reverse(links.begin(),links.end());
        const std::array separated{links[0],links[2]};topology.prepare(separated,links[1]);
        check(topology.committed_mask({1})==0&&topology.committed_mask({3})==0&&topology.preview_mask()==3&&
              topology.effective_mask({1})==2&&topology.effective_mask({3})==1,
            "middle preview did not update both neighbors without committing");
        topology.prepare(separated);check(topology.effective_mask({1})==0&&topology.effective_mask({3})==0&&!topology.has_preview(),
            "cancelled preview retained merged side topology");
        topology.prepare(links,std::nullopt,[](const auto& r){return r.id.value!=2;});
        check(topology.effective_mask({1})==0&&topology.effective_mask({3})==0,
            "source-invalid middle link connected a presentation slope");
        for(int invalid=0;invalid<4;++invalid) {
            auto other=links[1];
            if(invalid==0){std::swap(other.height_a,other.height_b);}
            if(invalid==1){++other.height_a;++other.height_b;}
            if(invalid==2){other.endpoint_a.x+=along.x;other.endpoint_a.y+=along.y;other.endpoint_b.x+=along.x;other.endpoint_b.y+=along.y;}
            if(invalid==3){other.endpoint_b={other.endpoint_a.x+step.x,other.endpoint_a.y+step.y};}
            topology.prepare(std::array{links[0],other});check(topology.effective_mask({1})==0,
                "different gradient/height/longitudinal/orthogonal link merged into slope");
        }
    }
}
std::vector<std::uint8_t> frame_bytes(SDL_Renderer* renderer) {
    auto* raw=SDL_RenderReadPixels(renderer,nullptr);check(raw,"frame readback failed");
    auto* rgba=SDL_ConvertSurface(raw,SDL_PIXELFORMAT_RGBA32);SDL_DestroySurface(raw);check(rgba,"RGBA conversion failed");
    std::vector<std::uint8_t> result(std::size_t(rgba->w)*std::size_t(rgba->h)*4U);
    for(int y=0;y<rgba->h;++y)std::copy_n(static_cast<const std::uint8_t*>(rgba->pixels)+std::size_t(y)*std::size_t(rgba->pitch),
        std::size_t(rgba->w)*4U,result.data()+std::size_t(y)*std::size_t(rgba->w)*4U);
    SDL_DestroySurface(rgba);return result;
}
void adjacent_cut_checks(SDL_Renderer* renderer) {
    const std::array<Uint8,3> background{13,21,31};
    for(const bool axis_x:{false,true})for(const int delta:{-1,1})for(const int base_height:{-7,1}) {
        const s::Cell lateral=axis_x?s::Cell{0,1}:s::Cell{1,0},along=axis_x?s::Cell{1,0}:s::Cell{0,1};
        std::array<s::RampLink,3> links{};
        for(int i=0;i<3;++i)links[std::size_t(i)]={{unsigned(i+1)},
            {114+lateral.x*i,114+lateral.y*i},{114+lateral.x*i+along.x,114+lateral.y*i+along.y},
            std::int8_t(base_height),std::int8_t(base_height+delta),40,0};
        const auto high=delta<0?links[0].endpoint_a:links[0].endpoint_b,low=delta<0?links[0].endpoint_b:links[0].endpoint_a;
        Fixture fixture(high,low,base_height+std::max(0,delta));
        for(const auto& r:links)for(const auto c:{r.endpoint_a,r.endpoint_b}) {
            const auto at=std::size_t(c.y*228+c.x);auto& p=fixture.cells[at];
            fixture.plan.raw_terrain[at]=0x80;p.road_allowed=p.building_allowed=true;
            p.road_blocker=p.building_blocker=s::BuildBlocker::None;
            p.height=c==r.endpoint_a?r.height_a:r.height_b;fixture.plan.height_bytes[at]=std::uint8_t(p.height);
        }
        const auto permissions=fixture.permissions();oe::AuthoredTerrainSides faces;
        faces.prepare(fixture.plan,fixture.geometry,*permissions);check(faces.size()==3,"parallel source edge stream is not exactly three faces");
        const auto prepared_bytes=faces.logical_geometry_bytes();check(prepared_bytes<=8U*1024U*1024U,"adjacent cache exceeded unchanged8MiB bound");
        const auto ground=[&](s::Cell c){return m::landscape_ground(fixture.plan,{unsigned(c.x),unsigned(c.y)});};
        const auto focus=ground(links[1].endpoint_a);
        oe::RampTopology topology;topology.prepare(links);
        for(const double zoom:{1.,2.,4.,1.73}) {
            oe::scene::Camera2D camera;camera.zoom=zoom;camera.offset={350-focus.x*zoom,260-focus.y*zoom};
            const auto clear=[&]{check(SDL_SetRenderDrawColor(renderer,13,21,31,255)&&SDL_RenderClear(renderer),"adjacent clear failed");};
            const auto sample=[&](SDL_Surface* surface,oe::scene::Point p){const auto q=camera.world_to_screen(p);Uint8 r=0,g=0,b=0,a=0;
                check(SDL_ReadSurfacePixel(surface,int(q.x),int(q.y),&r,&g,&b,&a),"adjacent pixel sample failed");return std::array<Uint8,3>{r,g,b};};
            faces.set_preview(std::nullopt);faces.set_ramps({});clear();check(faces.draw(renderer,camera),"closed group draw failed");
            const auto closed=frame_bytes(renderer);
            faces.set_ramps(links);clear();check(faces.draw(renderer,camera),"joined cut draw failed");
            auto* cut_pixels=SDL_RenderReadPixels(renderer,nullptr);check(cut_pixels,"joined cut readback failed");
            const auto closed_sample=[&](oe::scene::Point p){const auto q=camera.world_to_screen(p);
                const auto at=(std::size_t(int(q.y))*720U+std::size_t(int(q.x)))*4U;
                return std::array<Uint8,3>{closed[at],closed[at+1],closed[at+2]};};
            for(int i=0;i<3;++i) {
                const auto g=ground(delta<0?links[std::size_t(i)].endpoint_a:links[std::size_t(i)].endpoint_b);
                const int sign=delta<0?1:-1;const oe::scene::Point normal{axis_x?sign*20.:-sign*20.,sign*10.};
                const oe::scene::Point below{g.x+normal.x,g.y+normal.y+30};
                check(sample(cut_pixels,below)!=background&&sample(cut_pixels,below)==closed_sample(below),
                    "group excavation removed or repainted lower cliff support");
                if(i==1)continue;
                const double side=i==0?-.42:.42;const oe::scene::Point flank{g.x+normal.x+(axis_x?-40.:40.)*side,g.y+normal.y+20*side+8};
                check(sample(cut_pixels,flank)!=background&&sample(cut_pixels,flank)==closed_sample(flank),
                    "group excavation removed or repainted an exterior rock flank");
            }
            for(int join=0;join<2;++join) {
                const auto& r=links[std::size_t(join)];const auto g=ground(delta<0?r.endpoint_a:r.endpoint_b);
                const int sign=delta<0?1:-1;const oe::scene::Point normal{axis_x?sign*20.:-sign*20.,sign*10.};
                const oe::scene::Point across{axis_x?-40.:40.,20.};unsigned excavated=0;
                for(int y=-22;y<65;++y)for(int x=-44;x<44;++x) {
                    const double side=(x+.5-normal.x)/across.x,depth=y+.5-normal.y-20*side;
                    if(side<=.35||side>=.49||depth<=0||depth>=20)continue;
                    const oe::scene::Point p{g.x+x+.5,g.y+y+.5};bool fully_cut=true;
                    for(int sub=0;sub<4;++sub) {
                        const double px=x+(sub%2?.75:.25),py=y+(sub<2?.25:.75);
                        const double s=(px-normal.x)/across.x,d=py-normal.y-20*s;
                        const oe::scene::Point world{g.x+px,g.y+py};
                        fully_cut&=oe::ramp_terrain_opening(s,d,topology.effective_mask(r.id))&&
                            (oe::ramp_opaque_at(world,ground(r.endpoint_a),r,topology.effective_mask(r.id))||
                             oe::ramp_opaque_at(world,ground(links[std::size_t(join+1)].endpoint_a),links[std::size_t(join+1)],1));
                    }
                    if(!fully_cut)continue;
                    check(sample(cut_pixels,p)==background,"internal rock ribbon remains between matching adjacent RampLinks");++excavated;
                }
                check(excavated>0,"adjacent cut test never sampled a real covered internal cliff opening");
            }
            SDL_DestroySurface(cut_pixels);
            // F8 partial modes must retain byte-identical closed cliffs even
            // while all three committed links exist.
            clear();check(faces.draw(renderer,camera,false),"partial layer closed draw failed");
            check(frame_bytes(renderer)==closed,"partial F8 state excavated hidden Ramp geometry");
            const std::array separated{links[0],links[2]};faces.set_ramps(separated);clear();check(faces.draw(renderer,camera),"separated cuts failed");
            const auto separated_image=frame_bytes(renderer);
            faces.set_preview(links[1]);check(faces.open_edges()==2,"preview committed an owned opening");
            clear();check(faces.draw(renderer,camera),"neighbor preview cuts failed");const auto preview_image=frame_bytes(renderer);
            faces.set_preview(std::nullopt);faces.set_ramps(links);clear();check(faces.draw(renderer,camera),"committed merged cuts failed");
            check(frame_bytes(renderer)==preview_image,"preview and committed neighboring excavation disagree");
            faces.set_ramps(separated);faces.set_preview(links[1]);faces.set_preview(std::nullopt);clear();check(faces.draw(renderer,camera),"cancel neighboring preview failed");
            check(frame_bytes(renderer)==separated_image,"cancel did not restore both outer banks' cliff states");
            auto wrong_height=links[1];++wrong_height.height_a;++wrong_height.height_b;faces.set_preview(wrong_height);
            clear();check(faces.draw(renderer,camera),"invalid preview cut draw failed");
            check(frame_bytes(renderer)==separated_image,"source height mismatch opened or merged a cliff face");
            faces.set_preview(std::nullopt);faces.set_ramps(links);
            check(faces.logical_geometry_bytes()==prepared_bytes,"command or preview generated cliff geometry");
        }
    }
}
}
int main(){SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;try {
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy");check(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
    window=SDL_CreateWindow("Authored terrain side pixels",720,640,SDL_WINDOW_HIDDEN);check(window,SDL_GetError());
    renderer=SDL_CreateRenderer(window,"software");check(renderer,SDL_GetError());
    topology_checks();adjacent_cut_checks(renderer);
    for(const s::Cell direction:{s::Cell{1,0},s::Cell{0,1},s::Cell{-1,0},s::Cell{0,-1}})
        for(const int height:{-127,-1,0,1,127})for(const double zoom:{1.,2.,4.,1.73}) {
            const s::Cell high{114,114},low{114+direction.x,114+direction.y};Fixture f(high,low,height);
            const auto permissions=f.permissions();const auto canonical=permissions->canonical_state();oe::AuthoredTerrainSides faces;
            faces.prepare(f.plan,f.geometry,*permissions);check(faces.size()==1,"exact unprotected single-height face not prepared");
            const auto ground=m::landscape_ground(f.plan,{114,114});
            oe::scene::Camera2D camera;camera.zoom=zoom;camera.offset={350-ground.x*zoom,260-ground.y*zoom};
            check(SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE)&&SDL_SetRenderDrawColor(renderer,13,21,31,255)&&SDL_RenderClear(renderer),"clear failed");
            oe::performance::set_enabled(true);oe::performance::reset();
            const int frames=direction.x==1&&height==1&&zoom==1.?100:1;
            for(int frame=0;frame<frames;++frame)check(faces.draw(renderer,camera),"prepared side draw failed");
            for(std::size_t i=0;i<std::size_t(oe::performance::Counter::Count);++i)
                check(oe::performance::counter(static_cast<oe::performance::Counter>(i))==0,"side frame did forbidden work");
            oe::performance::set_enabled(false);SDL_BlendMode blend{};
            check(SDL_GetRenderDrawBlendMode(renderer,&blend)&&blend==SDL_BLENDMODE_NONE,"caller blend lost");
            const auto normal=oe::scene::Point{(direction.x-direction.y)*20.,(direction.x+direction.y)*10.};
            auto* pixels=SDL_RenderReadPixels(renderer,nullptr);check(pixels,"pixel readback failed");std::set<std::uint32_t> colors;
            for(int depth=4;depth<=35;++depth) {
                const auto screen=camera.world_to_screen({ground.x+normal.x,ground.y+normal.y+depth});
                Uint8 r=0,g=0,b=0,a=0;check(SDL_ReadSurfacePixel(pixels,int(screen.x),int(screen.y),&r,&g,&b,&a),"side pixel readback failed");
                check(!(r==13&&g==21&&b==31),"black void remains in the height facet interior");colors.insert((unsigned(r)<<16)|(unsigned(g)<<8)|b);
            }
            check(colors.size()>=8,"soil/stone graphic collapsed to a flat rectangle");SDL_DestroySurface(pixels);
            s::RampLink link{{1},high,low,std::int8_t(height),std::int8_t(height-1),40,0};
            if(link.endpoint_a.x>link.endpoint_b.x||link.endpoint_a.y>link.endpoint_b.y){std::swap(link.endpoint_a,link.endpoint_b);std::swap(link.height_a,link.height_b);}
            faces.set_ramps(std::array{link});check(faces.open_edges()==1,"owned edge did not excavate exact terrain face");
            const auto clear=[&]{check(SDL_SetRenderDrawColor(renderer,13,21,31,255)&&SDL_RenderClear(renderer),"opening clear");};
            const auto sample=[&](SDL_Surface* surface,oe::scene::Point p){const auto q=camera.world_to_screen(p);Uint8 r=0,g=0,b=0,a=0;
                check(SDL_ReadSurfacePixel(surface,int(q.x),int(q.y),&r,&g,&b,&a),"opening sample");return std::array<Uint8,3>{r,g,b};};
            const std::array<Uint8,3> background{13,21,31};
            clear();check(faces.draw(renderer,camera),"opened cliff draw");pixels=SDL_RenderReadPixels(renderer,nullptr);check(pixels,"cutout readback");
            // Ascending canonical edges are exactly edge-on in the fixed 2:1
            // projection. Probe their shallow visible crest, not an imaginary
            // broad tread. Descending edges expose the full graded surface.
            unsigned excavated=0;
            for(int depth=2;depth<20;++depth)for(int lateral=-8;lateral<=8;++lateral) {
                const double t=.5+double(lateral)/40.;
                const auto across=oe::scene::Point{direction.x?-20.:20.,10.};
                const oe::scene::Point p{ground.x+normal.x+across.x*(t-.5)*2,ground.y+normal.y+across.y*(t-.5)*2+depth};
                excavated+=sample(pixels,p)==background;
            }
            check(excavated>0,"vertical wall still crosses the entire authorized opening");
            const oe::scene::Point across{direction.x?-20.:20.,10.};
            for(const double side:{-.84,.84})check(sample(pixels,{ground.x+normal.x+across.x*side,ground.y+normal.y+across.y*side+8})!=background,"lateral cliff flank was removed");
            check(sample(pixels,{ground.x+normal.x,ground.y+normal.y+30})!=background,"rock below the sloped crossing was removed");SDL_DestroySurface(pixels);
            const auto start=m::landscape_ground(f.plan,{unsigned(link.endpoint_a.x),unsigned(link.endpoint_a.y)});
            for(int half=0;half<2;++half)for(const bool front:{false,true})check(oe::draw_ramp_part(renderer,camera,start,link,half,front,true),"integrated surface draw");
            pixels=SDL_RenderReadPixels(renderer,nullptr);check(pixels,"joined surface readback");
            for(double along:{-.5,0.,.5})for(int depth:{4,8,12,16})
                check(sample(pixels,{ground.x+normal.x+across.x*along,ground.y+normal.y+across.y*along+depth})!=background,"transparent hole between cutout and graded surface");
            SDL_DestroySurface(pixels);
            faces.set_ramps({});faces.set_preview(link);check(faces.open_edges()==0,"preview published a committed cutout");
            faces.set_preview(std::nullopt);clear();check(faces.draw(renderer,camera),"cancelled cutout draw");pixels=SDL_RenderReadPixels(renderer,nullptr);
            check(sample(pixels,{ground.x+normal.x,ground.y+normal.y+8})!=background,"cancelled preview left a terrain hole");SDL_DestroySurface(pixels);
            check(permissions->canonical_state()==canonical,"visual preparation mutated permissions");
        }
    // A long edge must not repeat the same four painted wall segments. Compare
    // corresponding interior pixels four cells apart, independently of the
    // material implementation. The former four-phase renderer fails here.
    {
        Fixture f({114,110},{115,110},1);
        for(int y=110;y<=118;++y)for(int x=114;x<=115;++x) {
            const auto at=std::size_t(y*228+x);f.plan.raw_terrain[at]=0x80;
            auto& p=f.cells[at];p.road_allowed=p.building_allowed=true;p.road_blocker=p.building_blocker=s::BuildBlocker::None;
            p.height=std::int8_t(x==114?1:0);f.plan.height_bytes[at]=std::uint8_t(p.height);
        }
        oe::AuthoredTerrainSides faces;faces.prepare(f.plan,f.geometry,*f.permissions());
        const auto a=m::landscape_ground(f.plan,{114,112}),b=m::landscape_ground(f.plan,{114,116});
        oe::scene::Camera2D camera;camera.zoom=1;camera.offset={450-a.x,240-a.y};
        check(SDL_SetRenderDrawColor(renderer,13,21,31,255)&&SDL_RenderClear(renderer)&&faces.draw(renderer,camera),"long cliff draw failed");
        auto* pixels=SDL_RenderReadPixels(renderer,nullptr);check(pixels,"long cliff readback");unsigned different=0;
        for(int depth=6;depth<35;depth+=2) {
            const auto pa=camera.world_to_screen({a.x+20,a.y+10+depth}),pb=camera.world_to_screen({b.x+20,b.y+10+depth});
            Uint8 ar=0,ag=0,ab=0,aa=0,br=0,bg=0,bb=0,ba=0;
            check(SDL_ReadSurfacePixel(pixels,int(pa.x),int(pa.y),&ar,&ag,&ab,&aa)&&SDL_ReadSurfacePixel(pixels,int(pb.x),int(pb.y),&br,&bg,&bb,&ba),"long cliff sample failed");
            different+=ar!=br||ag!=bg||ab!=bb;
        }
        SDL_DestroySurface(pixels);check(different>=6,"cliff material visibly repeats every four cells");
    }
    // A raised L has an inner corner, outer corners and eight unique boundary
    // edges. Its two cuts must retain all other faces and lower support.
    {
        Fixture f({114,114},{115,114},1);
        for(int y=113;y<=116;++y)for(int x=113;x<=116;++x) {
            const auto i=std::size_t(y*228+x);auto& p=f.cells[i];
            const bool high=(x==114&&(y==114||y==115))||(x==115&&y==114);
            p.road_allowed=p.building_allowed=true;p.road_blocker=p.building_blocker=s::BuildBlocker::None;p.height=high?1:0;
            f.plan.height_bytes[i]=std::uint8_t(p.height);f.plan.raw_terrain[i]=0x80;
        }
        oe::AuthoredTerrainSides faces;faces.prepare(f.plan,f.geometry,*f.permissions());
        check(faces.size()==8,"inner/outer corners duplicate or omit a physical cliff edge");
        const s::RampLink first{{1},{114,115},{115,115},1,0,40,0},second{{2},{115,114},{116,114},1,0,40,0};
        faces.set_ramps(std::array{first});faces.set_preview(second);
        check(faces.open_edges()==1,"neighbor preview changed committed topology");
        faces.set_ramps(std::array{first,second});check(faces.open_edges()==2,"two independent owned edges did not open");
        auto invalid=second;invalid.height_b=-1;faces.set_preview(invalid);
        faces.set_ramps(std::array{first});check(faces.open_edges()==1,"removed visual edge state remained stale");
        faces.set_ramps({});check(faces.open_edges()==0,"cleared link set retained openings");
        check(faces.logical_geometry_bytes()<=8U*1024U*1024U,"prepared geometry exceeded its explicit memory bound");
    }
    for(int variant=0;variant<7;++variant) {
        Fixture f({114,114},{115,114},1);auto& p=f.cells[114U*228U+115U];
        switch(variant) {
        case 0:p.height=1;f.plan.height_bytes[114U*228U+115U]=1;break;
        case 1:p.height=-1;f.plan.height_bytes[114U*228U+115U]=255;break;
        case 2:p.road_allowed=p.building_allowed=false;p.protected_original=true;break;
        case 3:f.plan.raw_terrain[114U*228U+115U]=0x280;break;
        case 4:f.plan.raw_objects[114U*228U+115U]=1;break;
        case 5:p.road_allowed=p.building_allowed=false;p.bridge_water=true;f.plan.raw_terrain[114U*228U+115U]=4;break;
        case 6:f.plan.landscape_layers_available=false;break;
        }
        oe::AuthoredTerrainSides faces;bool rejected=false;
        try{faces.prepare(f.plan,f.geometry,*f.permissions());}catch(const std::exception&){rejected=true;}
        check(rejected||faces.size()==0,"unsupported terrain/multilevel/protected geometry was drawn");
    }
    SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();std::cout<<"Authored terrain sides: four directions, signed heights, zero void, material, zoom and pure frames PASS\n";return 0;
}catch(const std::exception& e){SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();std::cerr<<e.what()<<'\n';return 1;}}
