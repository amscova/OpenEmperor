#include "renderer/BridgeRenderer.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <array>
#include <cmath>
namespace openemperor {
namespace {
constexpr double post_width=2.0,rail_width=2.2,lower_rail_width=1.4,support_width=2.2;
std::array<scene::Point,4> deck(scene::Point ground,const simulation::BridgeSpan& span,simulation::Cell cell) {
    const bool x=span.axis==simulation::BridgeAxis::StorageX;
    const scene::Point along{x?40.0:-40.0,20},across{x?-40.0:40.0,20};
    const double first=cell==span.bank_a?0:-.5,last=cell==span.bank_b?0:.5;
    const auto p=[&](double a,double b){return scene::Point{ground.x+a*along.x+b*across.x,ground.y+a*along.y+b*across.y};};
    return {p(first,-.31),p(last,-.31),p(last,.31),p(first,.31)};
}
bool inside_polygon(scene::Point world,const std::array<scene::Point,4>& points) {
    double sign=0;
    for(std::size_t i=0;i<4;++i) {
        const auto a=points[i],b=points[(i+1)%4];const double cross=(b.x-a.x)*(world.y-a.y)-(b.y-a.y)*(world.x-a.x);
        if(std::abs(cross)<1e-9)continue;
        if(sign!=0 && (cross>0)!=(sign>0))return false;sign=cross;
    }
    return true;
}
}
bool hit_bridge_cell(scene::Point world,scene::Point ground,const simulation::BridgeSpan& span,simulation::Cell cell) {
    return inside_polygon(world,deck(ground,span,cell));
}
bool hit_bridge_part(scene::Point world,scene::Point ground,const simulation::BridgeSpan& span,
    simulation::Cell cell,BridgePart part,double tolerance) {
    const auto points=deck(ground,span,cell);
    if(part==BridgePart::DeckAndBackRail && hit_bridge_cell(world,ground,span,cell))return true;
    if(part==BridgePart::DeckAndBackRail && inside_polygon(world,
        {points[3],points[2],scene::Point{points[2].x,points[2].y+5},scene::Point{points[3].x,points[3].y+5}}))return true;
    const auto line=[&](scene::Point a,scene::Point b,double width=post_width) {
        const double dx=b.x-a.x,dy=b.y-a.y,length=dx*dx+dy*dy;
        const double t=length==0?0:std::clamp(((world.x-a.x)*dx+(world.y-a.y)*dy)/length,0.0,1.0);
        const double x=world.x-a.x-t*dx,y=world.y-a.y-t*dy;const double radius=tolerance+width*.5;return x*x+y*y<=radius*radius;
    };
    if(part==BridgePart::DeckAndBackRail && cell!=span.bank_a&&cell!=span.bank_b)
        for(const auto p:{points[0],points[3]})if(line({p.x,p.y+4},{p.x,p.y+23},support_width)||line({p.x+2,p.y+4},{p.x+2,p.y+23},support_width))return true;
    const auto a=part==BridgePart::FrontRail?points[3]:points[0],b=part==BridgePart::FrontRail?points[2]:points[1];
    for(const double fraction:{0.0,.5,1.0}) {
        const scene::Point p{a.x+(b.x-a.x)*fraction,a.y+(b.y-a.y)*fraction};
        if(line(p,{p.x,p.y-13}))return true;
    }
    return line({a.x,a.y-12},{b.x,b.y-12},rail_width)||line({a.x,a.y-6},{b.x,b.y-6},lower_rail_width);
}
bool draw_bridge_cell(SDL_Renderer* renderer,const scene::Camera2D& camera,scene::Point ground,
    const simulation::BridgeSpan& span,simulation::Cell cell,BridgePart part,BridgeTint tint) {
    SDL_BlendMode previous=SDL_BLENDMODE_NONE;
    if(!SDL_GetRenderDrawBlendMode(renderer,&previous)||!SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND))return false;
    struct RestoreBlend {SDL_Renderer* renderer;SDL_BlendMode mode;~RestoreBlend(){(void)SDL_SetRenderDrawBlendMode(renderer,mode);}} restore{renderer,previous};
    auto points=deck(ground,span,cell);const double alpha=tint==BridgeTint::Normal?1:.55;
    const SDL_FColor top=tint==BridgeTint::Valid?SDL_FColor{.20F,.70F,.42F,float(alpha)}:
        tint==BridgeTint::Unaffordable?SDL_FColor{.85F,.62F,.20F,float(alpha)}:
        tint==BridgeTint::Blocked?SDL_FColor{.80F,.25F,.23F,float(alpha)}:SDL_FColor{.53F,.40F,.26F,1};
    const auto line=[&](scene::Point a,scene::Point b,SDL_Color color,double width=post_width) {
        const double length=std::hypot(b.x-a.x,b.y-a.y);if(length==0)return true;
        const double x=-(b.y-a.y)*width*.5/length,y=(b.x-a.x)*width*.5/length;
        const std::array<scene::Point,4> corners={scene::Point{a.x+x,a.y+y},scene::Point{b.x+x,b.y+y},
            scene::Point{b.x-x,b.y-y},scene::Point{a.x-x,a.y-y}};
        std::array<SDL_Vertex,4> vertices{};const SDL_FColor tint_color{color.r/255.0F,color.g/255.0F,color.b/255.0F,float(alpha)};
        for(std::size_t i=0;i<4;++i){const auto screen=camera.world_to_screen(corners[i]);vertices[i]={{float(screen.x),float(screen.y)},tint_color,{}};}
        constexpr int indices[]={0,1,2,0,2,3};return SDL_RenderGeometry(renderer,nullptr,vertices.data(),4,indices,6);
    };
    const auto quad=[&](const std::array<scene::Point,4>& p,SDL_FColor color) {
        std::array<SDL_Vertex,4> vertices{};
        for(std::size_t i=0;i<4;++i){const auto screen=camera.world_to_screen(p[i]);vertices[i]={{float(screen.x),float(screen.y)},color,{}};}
        constexpr int indices[]={0,1,2,0,2,3};return SDL_RenderGeometry(renderer,nullptr,vertices.data(),4,indices,6);
    };
    if(part==BridgePart::DeckAndBackRail) {
        auto side=points;side[0]=points[3];side[1]=points[2];side[2]={points[2].x,points[2].y+5};side[3]={points[3].x,points[3].y+5};
        if(!quad(side,{.30F,.23F,.16F,float(alpha)})||!quad(points,top))return false;
        // Plank seams and transverse beams make the crossing visibly timber.
        for(int i=0;i<=5;++i) {
            const double t=i/5.0;
            const scene::Point a{points[0].x+(points[1].x-points[0].x)*t,points[0].y+(points[1].y-points[0].y)*t};
            const scene::Point b{points[3].x+(points[2].x-points[3].x)*t,points[3].y+(points[2].y-points[3].y)*t};
            if(!line(a,b,{85,63,39,255},.65))return false;
        }
        if(cell!=span.bank_a&&cell!=span.bank_b)
            for(const auto p:{points[0],points[3]})
                if(!line({p.x,p.y+4},{p.x,p.y+23},{69,58,45,255},support_width)||
                   !line({p.x+2,p.y+4},{p.x+2,p.y+23},{93,76,52,255},support_width))return false;
    }
    // The front rail is a separate local painter component, after an actor
    // at the same cell key. No global bridge-last ordering is introduced.
    const auto a=part==BridgePart::FrontRail?points[3]:points[0],b=part==BridgePart::FrontRail?points[2]:points[1];
    for(const double fraction:{0.0,.5,1.0}) {
        const scene::Point p{a.x+(b.x-a.x)*fraction,a.y+(b.y-a.y)*fraction};
        if(!line(p,{p.x,p.y-13},{88,70,47,255}))return false;
    }
    return line({a.x,a.y-12},{b.x,b.y-12},{148,116,75,255},rail_width)&&
        line({a.x,a.y-6},{b.x,b.y-6},{101,81,55,255},lower_rail_width);
}
} // namespace openemperor
