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
            for(const double side:{-.46,.46}) {
                const oe::scene::Point begin{a.x+(axis?-40.:40.)*side,a.y+20*side};
                const oe::scene::Point end{begin.x+along.x,begin.y+along.y};
                for(const double t:{0.,.5,1.}) {
                    const auto bank=oe::ramp_bank_point(a,ramp,t,side);
                    check(std::hypot(bank.x-(begin.x+(end.x-begin.x)*t),bank.y-(begin.y+(end.y-begin.y)*t))<1e-9,
                        "bank height is not continuous between the two actual endpoint planes");
                }
                const auto left=oe::ramp_bank_point(a,ramp,.5-1e-9,side),right=oe::ramp_bank_point(a,ramp,.5+1e-9,side);
                check(std::hypot(left.x-right.x,left.y-right.y)<1e-6,"bank jumps at the painter half split");
            }
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
                    check(r>=120&&g>=110&&r>g&&b>=65&&b<g,"compacted earth/stone surface absent at actual interpolated endpoint heights");
                    shades.insert((std::uint32_t(r)<<16)|(std::uint32_t(g)<<8)|b);++samples;
                }
            }
            if(tint==oe::BridgeTint::Normal)check(samples==19&&shades.size()>=5,"authored surface lacks material variation");
            if(tint==oe::BridgeTint::Normal) {
                // An interior pixel must not reveal the clear color through
                // separately antialiased halves at their shared diagonal or
                // on the fixed-projection ascending crest.
                check(SDL_SetRenderDrawColor(renderer,210,30,80,255)&&SDL_RenderClear(renderer),"seam clear failed");
                for(int half=0;half<2;++half)for(const bool front:{false,true})check(oe::draw_ramp_part(renderer,camera,a,ramp,half,front,true),"seam draw failed");
                auto* alternate=SDL_RenderReadPixels(renderer,nullptr);check(alternate,"seam readback failed");
                for(const double t:{.48,.5,.52})for(const double side:{-.15,0.,.15,.4}) {
                    const auto screen=camera.world_to_screen(oe::ramp_surface_point(a,ramp,t,side));
                    Uint8 ar=0,ag=0,ab=0,aa=0,br=0,bg=0,bb=0,ba=0;
                    check(SDL_ReadSurfacePixel(pixels,int(screen.x),int(screen.y),&ar,&ag,&ab,&aa)&&
                          SDL_ReadSurfacePixel(alternate,int(screen.x),int(screen.y),&br,&bg,&bb,&ba),"seam sample failed");
                    if(ar!=br||ag!=bg||ab!=bb)std::cerr<<"seam axis="<<axis<<" delta="<<delta<<" height="<<height<<" zoom="<<zoom<<" t="<<t<<" side="<<side<<'\n';
                    check(ar==br&&ag==bg&&ab==bb,"shared ramp half raster exposes the background");
                }
                SDL_DestroySurface(alternate);
            }
            check(!oe::hit_ramp_part({a.x+150,a.y},a,ramp,0,false,true),"empty background selected as ramp");
            SDL_DestroySurface(pixels);
            if(tint==oe::BridgeTint::Normal) {
                // Independently probe the former cut/fill triangles. A near
                // shoulder must not paint across the drivable tread, nor form
                // a tall panel below an exactly edge-on ascending surface.
                check(SDL_SetRenderDrawColor(renderer,25,80,125,255)&&SDL_RenderClear(renderer),"bank clear failed");
                for(int half=0;half<2;++half)check(oe::draw_ramp_part(renderer,camera,a,ramp,half,true,true),"isolated bank draw failed");
                auto probe=oe::ramp_surface_point(a,ramp,delta<0?.3:.4,delta<0?.2:.36);
                if(delta>0)probe.y+=8;
                pixels=SDL_RenderReadPixels(renderer,nullptr);check(pixels,"bank readback failed");
                const auto screen=camera.world_to_screen(probe);Uint8 r=0,g=0,b=0,alpha=0;
                check(SDL_ReadSurfacePixel(pixels,int(screen.x),int(screen.y),&r,&g,&b,&alpha),"bank clearance sample failed");
                check(r==25&&g==80&&b==125,"triangular bank panel remains in rendered clearance");
                SDL_DestroySurface(pixels);
                check(!oe::hit_ramp_part(probe,a,ramp,0,true,true)&&!oe::hit_ramp_part(probe,a,ramp,1,true,true),
                    "discontinuous bank triangle intrudes into the tread/edge-on clearance");
            }
        }
    SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();std::cout<<"Four graded directions, signed heights, shared picking, tints and 1/2/4/fractional pixels PASS\n";return 0;
}catch(const std::exception& e){SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();std::cerr<<e.what()<<'\n';return 1;}}
