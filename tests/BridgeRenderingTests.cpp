#include "renderer/BridgeRenderer.h"
#include <SDL3/SDL.h>
#include <iostream>
#include <stdexcept>
#include <set>
namespace oe=openemperor;namespace s=oe::simulation;
namespace {
void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
oe::scene::Point ground(s::Cell cell,int deck){return {(cell.x-cell.y)*40.0,(cell.x+cell.y)*20.0+20-deck*40};}
}
int main(){SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;try {
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy");check(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
    window=SDL_CreateWindow("Authored bridge pixels",1200,850,SDL_WINDOW_HIDDEN);check(window,SDL_GetError());
    renderer=SDL_CreateRenderer(window,"software");check(renderer,SDL_GetError());
    for(const auto axis:{s::BridgeAxis::StorageX,s::BridgeAxis::StorageY})for(const double zoom:{1.0,2.0,4.0,1.73}) {
        s::BridgeSpan span;span.id={1};span.axis=axis;span.bank_a={2,2};span.bank_b=axis==s::BridgeAxis::StorageX?s::Cell{5,2}:s::Cell{2,5};span.deck_height=1;
        span.water_cells=axis==s::BridgeAxis::StorageX?std::vector<s::Cell>{{3,2},{4,2}}:std::vector<s::Cell>{{2,3},{2,4}};
        const auto cell=span.water_cells[0];const auto center=ground(cell,1);oe::scene::Camera2D camera;camera.zoom=zoom;camera.offset={600-center.x*zoom,400-center.y*zoom};
        check(oe::hit_bridge_cell(center,center,span,cell),"deck center not pickable");
        const oe::scene::Point across=axis==s::BridgeAxis::StorageX?oe::scene::Point{-40,20}:oe::scene::Point{40,20};
        const oe::scene::Point piling{center.x+across.x*.31,center.y+across.y*.31+18};
        check(oe::hit_bridge_part(piling,center,span,cell,oe::BridgePart::DeckAndBackRail,0),"visible piling not inspectable");
        check(!oe::hit_bridge_part(piling,center,span,cell,oe::BridgePart::FrontRail,0),"piling incorrectly belongs to front rail");
        check(!oe::hit_bridge_part({center.x+across.x*1.6,center.y+across.y*1.6},center,span,cell,oe::BridgePart::DeckAndBackRail,0),"empty water selected as structural body");
        check(!oe::hit_bridge_cell({center.x+across.x*.6,center.y+across.y*.6},center,span,cell),"empty lateral water picked as deck");
        for(const auto tint:{oe::BridgeTint::Normal,oe::BridgeTint::Valid,oe::BridgeTint::Unaffordable,oe::BridgeTint::Blocked}) {
            check(SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE)&&SDL_SetRenderDrawColor(renderer,25,80,125,255)&&SDL_RenderClear(renderer),"blue authored water clear");
            for(const auto part:{oe::BridgePart::DeckAndBackRail,oe::BridgePart::FrontRail})
                for(const auto point:{span.bank_a,span.water_cells[0],span.water_cells[1],span.bank_b})
                    check(oe::draw_bridge_cell(renderer,camera,ground(point,1),span,point,part,tint),"bridge mesh draw failed");
            SDL_BlendMode mode{};check(SDL_GetRenderDrawBlendMode(renderer,&mode)&&mode==SDL_BLENDMODE_NONE,"bridge did not restore caller blend mode");
            SDL_Surface* pixels=SDL_RenderReadPixels(renderer,nullptr);check(pixels,"authored production pixel readback");
            std::uint8_t r=0,g=0,b=0,a=0;check(SDL_ReadSurfacePixel(pixels,600,400,&r,&g,&b,&a),"deck center readback");
            if(tint==oe::BridgeTint::Normal)check(r>60&&g>45&&b<100,"weathered timber deck absent or rendered as unchanged blue water");
            if(tint==oe::BridgeTint::Valid)check(g>r&&g>b,"valid green deck preview missing");
            if(tint==oe::BridgeTint::Unaffordable)check(r>g&&g>b,"unaffordable amber deck preview missing");
            if(tint==oe::BridgeTint::Blocked)check(r>g&&r>b,"blocked red deck preview missing");
            if(tint==oe::BridgeTint::Normal) {
                std::set<std::uint32_t> shades;
                for(int y=400-int(12*zoom);y<=400+int(12*zoom);++y)
                    for(int x=600-int(22*zoom);x<=600+int(22*zoom);++x) {
                        if(!oe::hit_bridge_cell(camera.screen_to_world({x+.5,y+.5}),center,span,cell))continue;
                        check(SDL_ReadSurfacePixel(pixels,x,y,&r,&g,&b,&a),"wood material readback");
                        if(r>g&&g>b)shades.insert((std::uint32_t(r)<<16)|(std::uint32_t(g)<<8)|b);
                    }
                check(shades.size()>=14,"timber material collapsed into flat untextured polygons");
                const auto point=camera.world_to_screen(piling);
                check(SDL_ReadSurfacePixel(pixels,int(point.x),int(point.y),&r,&g,&b,&a),"piling readback");
                check(r>60&&b<100,"structural piling missing below deck or occluded by water");
            }
            SDL_DestroySurface(pixels);
        }
    }
    SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();std::cout<<"Authored bridge deck/rails, both axes, signed deck, alpha colors and 1/2/4/fractional zoom pixels PASS\n";return 0;
}catch(const std::exception& e){SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();std::cerr<<e.what()<<'\n';return 1;}}
