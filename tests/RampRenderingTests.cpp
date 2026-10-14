#include "renderer/RampRenderer.h"
#include <SDL3/SDL.h>
#include <iostream>
#include <stdexcept>
#include <set>
namespace oe=openemperor;namespace s=oe::simulation;
namespace {
void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
}
int main(){SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;try {
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy");check(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
    window=SDL_CreateWindow("Authored ramp pixels",900,750,SDL_WINDOW_HIDDEN);check(window,SDL_GetError());
    renderer=SDL_CreateRenderer(window,"software");check(renderer,SDL_GetError());
    for(const bool axis:{false,true})for(const int delta:{-1,1})for(const int height:{-1,1})
        for(const double zoom:{1.,2.,4.,1.73})for(const auto tint:{oe::BridgeTint::Normal,oe::BridgeTint::Valid,oe::BridgeTint::Unaffordable,oe::BridgeTint::Blocked}) {
            s::RampLink ramp{{1},{2,2},axis?s::Cell{3,2}:s::Cell{2,3},std::int8_t(height),std::int8_t(height+delta),40,0};
            const oe::scene::Point a{0,100-height*40.},along{axis?40.:-40.,20.-delta*40.};
            oe::scene::Camera2D camera;camera.zoom=zoom;camera.offset={450-along.x*.5*zoom,350-(a.y+along.y*.5)*zoom};
            check(SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE)&&SDL_SetRenderDrawColor(renderer,25,80,125,255)&&SDL_RenderClear(renderer),"clear failed");
            for(int half=0;half<2;++half)for(const bool front:{false,true})check(oe::draw_ramp_part(renderer,camera,a,ramp,half,front,true,tint),"graded ramp drawing failed");
            SDL_BlendMode mode{};check(SDL_GetRenderDrawBlendMode(renderer,&mode)&&mode==SDL_BLENDMODE_NONE,"caller alpha mode lost");
            auto* pixels=SDL_RenderReadPixels(renderer,nullptr);check(pixels,"readback failed");
            std::set<std::uint32_t> shades;int samples=0;
            for(int step=1;step<20;++step) {
                const double t=step/20.;const oe::scene::Point p{a.x+along.x*t,a.y+along.y*t};
                check(oe::hit_ramp_part(p,a,ramp,t<.5?0:1,false,true),"actual graded foot surface not pickable");
                const auto point=camera.world_to_screen(p);std::uint8_t r=0,g=0,b=0,alpha=0;
                check(SDL_ReadSurfacePixel(pixels,int(point.x),int(point.y),&r,&g,&b,&alpha),"surface pixel missing");
                if(tint==oe::BridgeTint::Normal) {
                    check(r>50&&g>40&&b<110,"sloped deck absent at actual interpolated endpoint heights");
                    shades.insert((std::uint32_t(r)<<16)|(std::uint32_t(g)<<8)|b);++samples;
                }
            }
            if(tint==oe::BridgeTint::Normal)check(samples==19&&shades.size()>=5,"authored surface lacks material variation");
            check(!oe::hit_ramp_part({a.x+150,a.y},a,ramp,0,false,true),"empty background selected as ramp");
            SDL_DestroySurface(pixels);
        }
    SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();std::cout<<"Four graded directions, signed heights, shared picking, tints and 1/2/4/fractional pixels PASS\n";return 0;
}catch(const std::exception& e){SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();std::cerr<<e.what()<<'\n';return 1;}}
