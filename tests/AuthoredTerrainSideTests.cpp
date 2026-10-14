#include "renderer/AuthoredTerrainSides.h"
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
            check(permissions->canonical_state()==canonical,"visual preparation mutated permissions");
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
