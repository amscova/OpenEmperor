#include "renderer/RampRenderer.h"
#include "assets/RoadVisualProfile.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <array>
#include <iostream>
#include <set>
#include <stdexcept>
#include <vector>
namespace oe=openemperor;namespace s=oe::simulation;
namespace {
void check(bool value,const char* why){if(!value)throw std::runtime_error(why);}
std::vector<std::uint8_t> pixels(SDL_Renderer* renderer) {
    auto* source=SDL_RenderReadPixels(renderer,nullptr);check(source,"readback failed");
    auto* converted=SDL_ConvertSurface(source,SDL_PIXELFORMAT_RGBA32);SDL_DestroySurface(source);check(converted,"RGBA conversion failed");
    std::vector<std::uint8_t> result(std::size_t(converted->w*converted->h*4));
    for(int y=0;y<converted->h;++y)std::copy_n(static_cast<const std::uint8_t*>(converted->pixels)+y*converted->pitch,
        converted->w*4,result.data()+std::size_t(y*converted->w*4));
    SDL_DestroySurface(converted);return result;
}
void clear(SDL_Renderer* renderer,Uint8 r=21,Uint8 g=56,Uint8 b=101) {
    check(SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE)&&SDL_SetRenderDrawColor(renderer,r,g,b,255)&&SDL_RenderClear(renderer),"clear failed");
}
}
int main(){SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;try {
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy");check(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
    window=SDL_CreateWindow("Ramp material cache",700,600,SDL_WINDOW_HIDDEN);check(window,SDL_GetError());
    renderer=SDL_CreateRenderer(window,"software");check(renderer,SDL_GetError());
    // Independent authored fixtures identify each existing Road mask by red,
    // and have native one-pixel green/blue grain. No original pixels or IDs.
    oe::assets::RoadVisualProfile profile;profile.replaces_ground=true;
    for(unsigned mask=0;mask<16;++mask) {
        oe::assets::RgbaImage image;image.width=78;image.height=40;image.pixels.resize(78U*40U*4U);
        for(unsigned y=0;y<40;++y)for(unsigned x=0;x<78;++x) {
            const auto at=(y*78+x)*4;image.pixels[at]=std::uint8_t(150+mask*5);
            image.pixels[at+1]=std::uint8_t(115+(x*13+y*7)%53);image.pixels[at+2]=std::uint8_t(65+(x*7+y*19)%43);image.pixels[at+3]=255;
        }
        oe::assets::RoadVisualEntry entry;entry.image_index=profile.unique_images.size();entry.ground_x=39;entry.ground_y=20;
        profile.unique_images.push_back(std::move(image));profile.tiles[mask]=entry;
    }
    oe::RampPresentation presentation;
    const auto fallback_bytes=presentation.geometry_bytes();presentation.prepare(&profile);
    check(presentation.geometry_bytes()>fallback_bytes&&presentation.geometry_bytes()<3500000,"both prepared materials exceed the prior cache bound");
    oe::scene::Camera2D camera;camera.offset={350,230};const oe::scene::Point ground{};
    for(const bool axis:{false,true})for(const int delta:{-1,1})for(unsigned adjacency=0;adjacency<4;++adjacency) {
        s::RampLink ramp{{1},{10,10},axis?s::Cell{11,10}:s::Cell{10,11},-1,std::int8_t(-1+delta),40,0};
        const auto draw=[&](bool elevated,bool road) {
            for(int half=0;half<2;++half)for(const bool front:{false,true})check(presentation.draw_part(renderer,camera,ground,ramp,half,front,elevated,
                oe::BridgeTint::Normal,adjacency,road),"prepared ramp draw failed");
        };
        // F6 changes material only. Geometry and the flat comparison remain exact.
        clear(renderer);draw(true,false);const auto fallback=pixels(renderer);
        clear(renderer);draw(true,true);const auto material=pixels(renderer);check(fallback!=material,"runtime Road material ignored");
        const unsigned mask=axis?(0xa|((adjacency&1U)?1:0)|((adjacency&2U)?4:0)):(0x5|((adjacency&1U)?8:0)|((adjacency&2U)?2:0));
        const auto foot=camera.world_to_screen(oe::ramp_surface_point(ground,ramp,.25,0));
        const auto at=std::size_t((int(foot.y)*700+int(foot.x))*4);
        check(material[at]==150+mask*5,"ramp did not select the existing straight/T/cross Road mask");
        std::set<std::uint32_t> colors;
        for(std::size_t i=0;i<material.size();i+=4)if(material[i]!=21||material[i+1]!=56||material[i+2]!=101)
            colors.insert((std::uint32_t(material[i])<<16)|(std::uint32_t(material[i+1])<<8)|material[i+2]);
        check(colors.size()>=(delta<0?100U:15U),"native Road grain collapsed into a smooth slab");
        clear(renderer);draw(false,false);const auto flat=pixels(renderer);
        clear(renderer);draw(false,true);check(flat==pixels(renderer),"Road material changed flat Snapshot pixels");
        clear(renderer);for(int half=0;half<2;++half)for(const bool front:{false,true})check(oe::draw_ramp_part(renderer,camera,ground,ramp,half,front,false),"legacy flat draw failed");
        check(flat==pixels(renderer),"prepared flat Snapshot differs from preserved legacy comparison");
        if(adjacency&2U) {
            clear(renderer);const auto empty=pixels(renderer);
            for(int half=0;half<2;++half)check(presentation.draw_part(renderer,camera,ground,ramp,half,true,true,oe::BridgeTint::Normal,adjacency,true),"internal front draw failed");
            check(empty==pixels(renderer),"a joined interior retained its own front bank");
        }
        for(const double zoom:{1.,2.,4.,1.73}) {
            camera.zoom=zoom;clear(renderer);draw(true,true);const auto first=pixels(renderer);
            clear(renderer,200,20,70);draw(true,true);const auto second=pixels(renderer);
            for(const double t:{.1,.48,.5,.52,.9}) {
                const auto p=oe::ramp_surface_point(ground,ramp,t,0),screen=camera.world_to_screen(p);
                const auto offset=std::size_t((int(screen.y)*700+int(screen.x))*4);
                check(std::equal(first.begin()+std::ptrdiff_t(offset),first.begin()+std::ptrdiff_t(offset+3),second.begin()+std::ptrdiff_t(offset)),"prepared material exposes background through the tread");
                check(presentation.hit_part(p,ground,ramp,t<.5?0:1,false,true,adjacency),"material changed actual foot picking");
            }
        }
        camera.zoom=1;
    }
    // A junction fixture has explicit pale corner/curb material surrounding
    // a bounded central earth patch. Shared edges must use the earth patch in
    // the middle of a widened slope, while exterior/endpoint curbs stay exact.
    auto curbs=profile;
    for(auto& image:curbs.unique_images)for(int y=0;y<40;++y)for(int x=0;x<78;++x) {
        const auto at=std::size_t((y*78+x)*4);const bool earth=x>=33&&x<45&&y>=16&&y<24;
        image.pixels[at]=earth?Uint8(193+(x+y)%7):245;
        image.pixels[at+1]=earth?Uint8(157+(x*3+y)%9):242;image.pixels[at+2]=earth?109:230;
    }
    presentation.prepare(&curbs);camera.zoom=2;
    const s::RampLink widened{{1},{10,10},{11,10},1,0,40,0};
    const auto sample=[&](unsigned adjacency,double t,double side) {
        clear(renderer);
        for(int half=0;half<2;++half)for(const bool front:{false,true})check(presentation.draw_part(renderer,camera,ground,widened,
            half,front,true,oe::BridgeTint::Normal,adjacency,true),"curb fixture draw failed");
        const auto output=pixels(renderer);const auto p=camera.world_to_screen(oe::ramp_surface_point(ground,widened,t,side));
        return output[std::size_t((int(p.y)*700+int(p.x))*4)];
    };
    for(const double t:{.30,.40,.60,.70})for(const double side:{-.4,.4}) {
        check(sample(3,t,side)>=193&&sample(3,t,side)<=199,"joined corridor retained source corner-curb rows");
        check(sample(0,t,side)==245,"shared-edge UV treatment changed an isolated exterior curb");
        check(sample(side<0?2U:1U,t,side)==245,"shared-edge UV treatment changed the group's exterior flank");
    }
    for(const double t:{.02,.98})for(const double side:{-.4,.4})
        check(sample(3,t,side)==sample(0,t,side),"interior material treatment changed the endpoint road join");
    // Profile invalidation restores the precomputed authored style atomically.
    presentation.clear();check(presentation.geometry_bytes()==fallback_bytes,"profile clear retained runtime material storage");
    SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();
    std::cout<<"Prepared Road-mask materials, native grain, topology, F6/F8, signed projection, picking and bounded caches PASS\n";return 0;
}catch(const std::exception& e){SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();std::cerr<<e.what()<<'\n';return 1;}}
