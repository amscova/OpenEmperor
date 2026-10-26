#include "renderer/OriginalEliteHouseFallbackRenderer.h"
#include <SDL3/SDL.h>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>

namespace {
void check(bool value,const char* message) { if (!value) throw std::runtime_error(message); }
using Color=std::array<std::uint8_t,4>;
constexpr Color terrain{27,53,39,255};
Color pixel(SDL_Surface* surface,int x,int y) {
    Color out{};
    check(SDL_ReadSurfacePixel(surface,x,y,&out[0],&out[1],&out[2],&out[3]),"read courtyard pixel");
    return out;
}
}
int main() {
    try {
        std::unique_ptr<SDL_Surface,decltype(&SDL_DestroySurface)> surface(
            SDL_CreateSurface(800,640,SDL_PIXELFORMAT_RGBA32),SDL_DestroySurface);
        check(surface!=nullptr,"courtyard surface");
        std::unique_ptr<SDL_Renderer,decltype(&SDL_DestroyRenderer)> renderer(
            SDL_CreateSoftwareRenderer(surface.get()),SDL_DestroyRenderer);
        check(renderer!=nullptr,"courtyard software renderer");
        for (const double zoom:{0.5,1.0,2.0}) {
            const openemperor::scene::Point ground{400,420};
            check(SDL_SetRenderDrawColor(renderer.get(),terrain[0],terrain[1],terrain[2],terrain[3]) &&
                SDL_RenderClear(renderer.get()) && SDL_SetRenderDrawBlendMode(renderer.get(),SDL_BLENDMODE_BLEND) &&
                openemperor::draw_original_elite_house_fallback(renderer.get(),ground,zoom) &&
                SDL_FlushRenderer(renderer.get()),"draw courtyard");
            SDL_BlendMode mode;
            check(SDL_GetRenderDrawBlendMode(renderer.get(),&mode) && mode==SDL_BLENDMODE_BLEND,
                "courtyard changed caller blend mode");
            unsigned visible=0;
            for (int y=0;y<640;++y) for (int x=0;x<800;++x) {
                if (pixel(surface.get(),x,y)==terrain) continue;
                ++visible;
                check(x>=ground.x-160*zoom && x<=ground.x+160*zoom &&
                    y>=ground.y-140*zoom && y<=ground.y+20*zoom,
                    "courtyard escaped independently derived4x4 bounds");
                // Triangle rasterization can include an edge pixel whose center
                // lies just beyond its analytic boundary. Use interior samples.
                if (x%7==0 && y%7==0 && pixel(surface.get(),x+1,y)==pixel(surface.get(),x-1,y) &&
                    pixel(surface.get(),x,y+1)==pixel(surface.get(),x,y-1))
                    check(openemperor::hit_original_elite_house_fallback({double(x),double(y)},ground,zoom),
                        "opaque courtyard interior has no matching visual hit");
            }
            check(visible>20000*zoom*zoom,"courtyard failed to occupy actual4x4 ground");
            for (const openemperor::scene::Point point:std::array<openemperor::scene::Point,4>{{
                    {-150,-60},{150,-60},{0,-130},{0,10}}}) {
                const openemperor::scene::Point screen{ground.x+point.x*zoom,ground.y+point.y*zoom};
                check(openemperor::hit_original_elite_house_fallback(screen,ground,zoom) &&
                    pixel(surface.get(),static_cast<int>(screen.x),static_cast<int>(screen.y))!=terrain,
                    "outer4x4 ground cell was neither drawn nor selectable");
            }
            for (const openemperor::scene::Point point:std::array<openemperor::scene::Point,5>{{
                    {-150,-130},{150,-130},{-150,10},{150,10},{0,22}}})
                check(!openemperor::hit_original_elite_house_fallback(
                    {ground.x+point.x*zoom,ground.y+point.y*zoom},ground,zoom),
                    "transparent courtyard bounds stole outside selection");
            // The renderer's existing map clip must preserve partial bodies.
            const SDL_Rect clip{400,0,400,640};
            check(SDL_SetRenderClipRect(renderer.get(),&clip) &&
                SDL_SetRenderDrawColor(renderer.get(),terrain[0],terrain[1],terrain[2],terrain[3]) &&
                SDL_RenderClear(renderer.get()) &&
                openemperor::draw_original_elite_house_fallback(renderer.get(),ground,zoom) &&
                SDL_FlushRenderer(renderer.get()),"clipped courtyard");
            check(pixel(surface.get(),static_cast<int>(ground.x+100*zoom),static_cast<int>(ground.y-60*zoom))!=terrain,
                "visible half of partially clipped courtyard vanished");
            check(SDL_SetRenderClipRect(renderer.get(),nullptr),"restore courtyard clip");
        }
        const double nan=std::numeric_limits<double>::quiet_NaN();
        const double inf=std::numeric_limits<double>::infinity();
        for (const double zoom:{0.0,-1.0,nan,inf})
            check(!openemperor::draw_original_elite_house_fallback(renderer.get(),{400,420},zoom) &&
                !openemperor::hit_original_elite_house_fallback({400,400},{400,420},zoom),
                "invalid courtyard transform accepted");
        check(!openemperor::draw_original_elite_house_fallback(nullptr,{400,420},1) &&
            !openemperor::hit_original_elite_house_fallback({nan,0},{400,420},1),
            "invalid courtyard renderer/hit accepted");
        std::cout<<"Authored Elite4x4 floor, outer picking, transparent corners,3zooms and clipping PASS\n";
        return 0;
    } catch (const std::exception& error) { std::cerr<<error.what()<<'\n';return 1; }
}
