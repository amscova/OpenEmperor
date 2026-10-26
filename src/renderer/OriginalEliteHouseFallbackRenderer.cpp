#include "renderer/OriginalEliteHouseFallbackRenderer.h"
#include <SDL3/SDL.h>
#include <array>
#include <cmath>
#include <limits>

namespace openemperor {
namespace {
struct Face { std::array<SDL_FPoint,4> points; SDL_Color color; };
// The floor is the union of sixteen 80x40 cells: x[-160,160],y[-140,20].
// The modest roofs are deliberately authored and independent of house tier.
constexpr std::array<Face,13> faces{{
    {{{{0,-140},{160,-60},{0,20},{-160,-60}}},{122,113,98,255}},
    {{{{0,-126},{132,-60},{0,6},{-132,-60}}},{148,137,115,255}},
    {{{{-52,-70},{0,-96},{52,-70},{0,-44}}},{170,157,128,255}},
    {{{{-116,-66},{-36,-106},{-36,-120},{-116,-80}}},{133,106,92,255}},
    {{{{-36,-106},{-4,-90},{-4,-104},{-36,-120}}},{108,86,79,255}},
    {{{{-126,-82},{-40,-125},{-2,-106},{-88,-63}}},{93,91,104,255}},
    {{{{-40,-125},{-22,-128},{16,-109},{-2,-106}}},{114,109,120,255}},
    {{{{4,-90},{36,-106},{36,-120},{4,-104}}},{140,111,95,255}},
    {{{{36,-106},{116,-66},{116,-80},{36,-120}}},{106,85,78,255}},
    {{{{2,-106},{40,-125},{126,-82},{88,-63}}},{107,101,114,255}},
    {{{{40,-125},{22,-128},{-16,-109},{2,-106}}},{128,118,131,255}},
    {{{{-66,-24},{0,9},{0,-1},{-66,-34}}},{133,106,92,255}},
    {{{{0,9},{66,-24},{66,-34},{0,-1}}},{105,84,79,255}}
}};
constexpr auto mesh=[] {
    std::array<SDL_Vertex,faces.size()*4> out{};
    for (std::size_t i=0;i<faces.size();++i) for (std::size_t p=0;p<4;++p) {
        const auto c=faces[i].color;
        out[i*4+p]={faces[i].points[p],{c.r/255.F,c.g/255.F,c.b/255.F,1.F},{0,0}};
    }
    return out;
}();
constexpr auto indices=[] {
    std::array<int,faces.size()*6> out{};
    for (std::size_t i=0;i<faces.size();++i) {
        const int n=static_cast<int>(i*4);
        out[i*6]=n;out[i*6+1]=n+1;out[i*6+2]=n+2;
        out[i*6+3]=n;out[i*6+4]=n+2;out[i*6+5]=n+3;
    }
    return out;
}();
bool representable(scene::Point ground,double zoom) {
    constexpr double limit=static_cast<double>(std::numeric_limits<float>::max())/4;
    return std::isfinite(ground.x) && std::isfinite(ground.y) &&
        std::isfinite(zoom) && zoom>0 && std::abs(ground.x)<limit &&
        std::abs(ground.y)<limit && zoom<limit/320;
}
}
bool draw_original_elite_house_fallback(SDL_Renderer* renderer,scene::Point ground,double zoom) {
    if (!renderer || !representable(ground,zoom))
        return SDL_SetError("Elite courtyard requires finite ground and positive representable zoom");
    auto vertices=mesh;
    for (auto& vertex:vertices) vertex.position={static_cast<float>(ground.x+vertex.position.x*zoom),
        static_cast<float>(ground.y+vertex.position.y*zoom)};
    SDL_BlendMode previous;
    if (!SDL_GetRenderDrawBlendMode(renderer,&previous) ||
        !SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE)) return false;
    const bool drawn=SDL_RenderGeometry(renderer,nullptr,vertices.data(),static_cast<int>(vertices.size()),
        indices.data(),static_cast<int>(indices.size()));
    const bool restored=SDL_SetRenderDrawBlendMode(renderer,previous);
    return drawn && restored;
}
bool hit_original_elite_house_fallback(scene::Point screen,scene::Point ground,double zoom) {
    if (!representable(ground,zoom) || !std::isfinite(screen.x) || !std::isfinite(screen.y)) return false;
    const auto vertex=[&](int index) {
        const auto p=mesh[static_cast<std::size_t>(index)].position;
        return scene::Point{static_cast<float>(ground.x+p.x*zoom),static_cast<float>(ground.y+p.y*zoom)};
    };
    const auto cross=[](scene::Point a,scene::Point b,scene::Point p) {
        return (b.x-a.x)*(p.y-a.y)-(b.y-a.y)*(p.x-a.x);
    };
    for (std::size_t i=0;i<indices.size();i+=3) {
        const auto a=vertex(indices[i]),b=vertex(indices[i+1]),c=vertex(indices[i+2]);
        if (cross(a,b,c)==0) continue;
        const auto ab=cross(a,b,screen),bc=cross(b,c,screen),ca=cross(c,a,screen);
        if ((ab>=0 && bc>=0 && ca>=0) || (ab<=0 && bc<=0 && ca<=0)) return true;
    }
    return false;
}
}
