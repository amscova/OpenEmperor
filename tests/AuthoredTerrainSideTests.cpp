#include "renderer/AuthoredTerrainSides.h"
#include "renderer/RampRenderer.h"
#include "maps/LandscapeProvenance.h"
#include "core/PerformanceDiagnostics.h"
#include <SDL3/SDL.h>
#include <iostream>
#include <set>
#include <stdexcept>
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
}
int main(){SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;try {
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy");check(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
    window=SDL_CreateWindow("Authored terrain side pixels",720,640,SDL_WINDOW_HIDDEN);check(window,SDL_GetError());
    renderer=SDL_CreateRenderer(window,"software");check(renderer,SDL_GetError());
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
