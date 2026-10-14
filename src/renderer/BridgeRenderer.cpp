#include "renderer/BridgeRenderer.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>

namespace openemperor {
namespace {
using Point=scene::Point;
using Quad=std::array<Point,4>;
constexpr double rail_height=12,post_height=13;
struct Face { Quad points; SDL_Color color; };
constexpr std::size_t face_capacity=2048;
Point between(Point a,Point b,double t) {return {a.x+(b.x-a.x)*t,a.y+(b.y-a.y)*t};}
struct Mesh {
    // Fixed, eagerly built material detail; never generated from map IDs or time.
    std::array<Face,face_capacity> faces{};
    std::size_t count=0;
    void quad(Quad points,SDL_Color color) {
        if(count>=faces.size())throw std::logic_error("authored bridge mesh capacity exceeded");
        faces[count++]={points,color};
    }
    void timber(Quad p,SDL_Color base,int length,int width,int board=0) {
        constexpr int grain[]={-8,3,-4,6,-2,-11,1,5,-6,2,-3,7,-1,-9,4,0};
        for(int y=0;y<width;++y)for(int x=0;x<length;++x) {
            const auto a=between(p[0],p[1],double(x)/length),b=between(p[0],p[1],double(x+1)/length);
            const auto c=between(p[3],p[2],double(x+1)/length),d=between(p[3],p[2],double(x)/length);
            const int shade=grain[(x+board*3+y*5)%16]+(y==0?2:-2);
            const auto channel=[&](Uint8 value){return Uint8(std::clamp(int(value)+shade,0,255));};
            quad({between(a,d,double(y)/width),between(b,c,double(y)/width),
                between(b,c,double(y+1)/width),between(a,d,double(y+1)/width)},
                {channel(base.r),channel(base.g),channel(base.b),255});
        }
    }
    void stroke(Point a,Point b,double width,SDL_Color color,bool solid=false) {
        const double length=std::hypot(b.x-a.x,b.y-a.y);
        if(length==0)return;
        const double x=-(b.y-a.y)*width*.5/length,y=(b.x-a.x)*width*.5/length;
        const Quad top={Point{a.x+x,a.y+y},Point{b.x+x,b.y+y},Point{b.x-x,b.y-y},Point{a.x-x,a.y-y}};
        if(solid) {
            const auto dark=[&](double light){return SDL_Color{Uint8(color.r*light),Uint8(color.g*light),Uint8(color.b*light),255};};
            const Point shift{.5,.8};
            quad({top[3],top[2],Point{top[2].x+shift.x,top[2].y+shift.y},Point{top[3].x+shift.x,top[3].y+shift.y}},dark(.58));
            quad({top[1],top[2],Point{top[2].x+shift.x,top[2].y+shift.y},Point{top[1].x+shift.x,top[1].y+shift.y}},dark(.74));
        }
        if(solid)timber(top,color,std::max(1,int(std::ceil(length/2.2))),width>=2.5?2:1);
        else quad(top,color);
    }
    void post_cap(Point p) {
        constexpr double half=1.3,depth=.8;
        const Quad top={Point{p.x,p.y-half*.5},Point{p.x+half,p.y},Point{p.x,p.y+half*.5},Point{p.x-half,p.y}};
        quad({top[3],top[2],Point{top[2].x,top[2].y+depth},Point{top[3].x,top[3].y+depth}},
            {58,49,37,255});
        quad({top[1],top[2],Point{top[2].x,top[2].y+depth},Point{top[1].x,top[1].y+depth}},
            {75,65,48,255});
        quad(top,{112,100,76,255});
    }
};
Quad deck(Point ground,const simulation::BridgeSpan& span,simulation::Cell cell) {
    const bool x=span.axis==simulation::BridgeAxis::StorageX;
    const Point along{x?40.0:-40.0,20},across{x?-40.0:40.0,20};
    const double first=cell==span.bank_a?0:-.5,last=cell==span.bank_b?0:.5;
    const auto p=[&](double a,double b){return Point{ground.x+a*along.x+b*across.x,ground.y+a*along.y+b*across.y};};
    return {p(first,-.31),p(last,-.31),p(last,.31),p(first,.31)};
}
Point lift(Point p,double height) {return {p.x,p.y-height};}
bool inside_polygon(Point world,const Quad& points) {
    double sign=0;
    for(std::size_t i=0;i<4;++i) {
        const auto a=points[i],b=points[(i+1)%4];
        const double cross=(b.x-a.x)*(world.y-a.y)-(b.y-a.y)*(world.x-a.x);
        if(std::abs(cross)<1e-9)continue;
        if(sign!=0 && (cross>0)!=(sign>0))return false;
        sign=cross;
    }
    return true;
}
Mesh make_geometry(Point ground,const simulation::BridgeSpan& span,simulation::Cell cell,BridgePart part) {
    Mesh mesh;const auto p=deck(ground,span,cell);
    const bool bank=cell==span.bank_a||cell==span.bank_b;
    if(part==BridgePart::DeckAndBackRail) {
        // Supports and short diagonal knees stay below the unchanged deck.
        // The bank threshold meets the road without decorative raised cubes.
        if(!bank)for(const auto side:{0U,3U}) {
                const auto at=between(p[side],p[side==0?1:2],.5);
                mesh.stroke(lift(at,-22),lift(at,-3),3.0,{83,73,54,255},true);
                const auto end=between(p[side],p[side==0?1:2],.9);
                mesh.stroke(lift(at,-16),lift(between(at,end,.30),-4),2.3,{88,76,54,255},true);
        }
        mesh.timber({p[3],p[2],lift(p[2],-3),lift(p[3],-3)},{66,57,42,255},bank?10:20,2);
        mesh.stroke(lift(p[3],-2),lift(p[2],-2),1.4,{87,76,55,255},true);
        constexpr SDL_Color boards[]={{116,101,77,255},{109,96,74,255},{122,105,79,255},{112,99,77,255}};
        const int count=bank?4:8;
        for(int i=0;i<count;++i) {
            const double first=double(i)/count,last=double(i+1)/count;
            const auto a=between(p[0],p[1],first),b=between(p[0],p[1],last);
            const auto c=between(p[3],p[2],last),d=between(p[3],p[2],first);
            // Fibres run across the crossing, along the actual board direction.
            mesh.timber({a,d,c,b},boards[i%4],14,2,i);
            mesh.stroke(a,d,.45,{79,69,51,255});
            mesh.stroke(between(a,b,.5),between(d,c,.5),.25,{102,88,65,255});
        }
    }
    // Shared geometry drives drawing and picking. The near handrail retains
    // its local front component, after an actor at the same painter key.
    const auto a=part==BridgePart::FrontRail?p[3]:p[0],b=part==BridgePart::FrontRail?p[2]:p[1];
    // One post at each land threshold and water-cell centre, with continuous
    // rails. Half-bank pieces do not create a denser fence at the approaches.
    const double t=cell==span.bank_a?0:cell==span.bank_b?1:.5;
    {
        const auto at=between(a,b,t);
        mesh.stroke(lift(at,-1),lift(at,post_height),2.1,{78,67,49,255},true);
        mesh.post_cap(lift(at,post_height+.8));
    }
    mesh.stroke(lift(a,6),lift(b,6),1.4,{83,71,52,255},true);
    mesh.stroke(lift(a,rail_height),lift(b,rail_height),2.2,{114,98,70,255},true);
    return mesh;
}
Mesh pixel_contours(const Mesh& input) {
    constexpr int side=80,origin=40;
    std::array<SDL_Color,side*side*4> samples{};
    for(std::size_t i=0;i<input.count;++i) {
        const auto& face=input.faces[i];
        double left=40,right=-40,top=40,bottom=-40;
        for(const auto p:face.points){left=std::min(left,p.x);right=std::max(right,p.x);top=std::min(top,p.y);bottom=std::max(bottom,p.y);}
        const int x0=std::clamp(int(std::floor(left))+origin,0,side),x1=std::clamp(int(std::ceil(right))+origin,0,side);
        const int y0=std::clamp(int(std::floor(top))+origin,0,side),y1=std::clamp(int(std::ceil(bottom))+origin,0,side);
        for(int y=y0;y<y1;++y)for(int x=x0;x<x1;++x)for(int sample=0;sample<4;++sample) {
            const Point p{x-origin+(sample%2==0?.25:.75),y-origin+(sample<2?.25:.75)};
            if(inside_polygon(p,face.points))samples[std::size_t((y*side+x)*4+sample)]=face.color;
        }
    }
    Mesh result;
    for(int y=0;y<side;++y) {
        std::array<SDL_Color,side> row{};
        for(int x=0;x<side;++x) {
            unsigned r=0,g=0,b=0,count=0;
            for(int sample=0;sample<4;++sample) {
                const auto c=samples[std::size_t((y*side+x)*4+sample)];
                if(c.a){r+=c.r;g+=c.g;b+=c.b;++count;}
            }
            if(count)row[std::size_t(x)]={Uint8(r/count),Uint8(g/count),Uint8(b/count),Uint8(count*255/4)};
        }
        for(int x=0;x<side;) {
            if(!row[std::size_t(x)].a){++x;continue;}
            const auto c=row[std::size_t(x)];int end=x+1;
            while(end<side) {
                const auto next=row[std::size_t(end)];
                if(next.r!=c.r || next.g!=c.g || next.b!=c.b || next.a!=c.a)break;
                ++end;
            }
            result.quad({Point{double(x-origin),double(y-origin)},Point{double(end-origin),double(y-origin)},
                Point{double(end-origin),double(y-origin+1)},Point{double(x-origin),double(y-origin+1)}},c);
            x=end;
        }
    }
    return result;
}
// Two axes, bank/start-middle-end and back/front. Prepared before main; no
// allocation, texture work or material generation in draw or inspection.
const auto meshes=[] {
    std::array<Mesh,12> result;
    for(int axis=0;axis<2;++axis)for(int kind=0;kind<3;++kind)for(int part=0;part<2;++part) {
        simulation::BridgeSpan span;span.axis=axis==0?simulation::BridgeAxis::StorageX:simulation::BridgeAxis::StorageY;
        span.bank_a={0,0};span.bank_b=axis==0?simulation::Cell{2,0}:simulation::Cell{0,2};
        const auto cell=kind==0?span.bank_a:kind==2?span.bank_b:axis==0?simulation::Cell{1,0}:simulation::Cell{0,1};
        result[std::size_t(axis*6+kind*2+part)]=pixel_contours(make_geometry({},span,cell,part==0?BridgePart::DeckAndBackRail:BridgePart::FrontRail));
    }
    return result;
}();
const Mesh& geometry(const simulation::BridgeSpan& span,simulation::Cell cell,BridgePart part) {
    const int axis=span.axis==simulation::BridgeAxis::StorageX?0:1;
    const int kind=cell==span.bank_a?0:cell==span.bank_b?2:1;
    return meshes[std::size_t(axis*6+kind*2+(part==BridgePart::FrontRail?1:0))];
}
SDL_FColor color(SDL_Color base,BridgeTint tint) {
    if(tint==BridgeTint::Normal)return {base.r/255.0F,base.g/255.0F,base.b/255.0F,base.a/255.0F};
    const SDL_FColor overlay=tint==BridgeTint::Valid?SDL_FColor{.20F,.70F,.42F,1}:
        tint==BridgeTint::Unaffordable?SDL_FColor{.85F,.62F,.20F,1}:SDL_FColor{.80F,.25F,.23F,1};
    return {overlay.r*.8F+base.r/255.0F*.2F,overlay.g*.8F+base.g/255.0F*.2F,overlay.b*.8F+base.b/255.0F*.2F,.55F*base.a/255.0F};
}
}
bool hit_bridge_cell(Point world,Point ground,const simulation::BridgeSpan& span,simulation::Cell cell) {
    return inside_polygon(world,deck(ground,span,cell));
}
bool hit_bridge_part(Point world,Point ground,const simulation::BridgeSpan& span,
    simulation::Cell cell,BridgePart part,double tolerance) {
    const auto& mesh=geometry(span,cell,part);
    world={world.x-ground.x,world.y-ground.y};
    for(std::size_t i=0;i<mesh.count;++i) {
        const auto& p=mesh.faces[i].points;
        if(inside_polygon(world,p))return true;
        for(std::size_t edge=0;edge<4;++edge) {
            const auto a=p[edge],b=p[(edge+1)%4];const double dx=b.x-a.x,dy=b.y-a.y,length=dx*dx+dy*dy;
            const double t=length==0?0:std::clamp(((world.x-a.x)*dx+(world.y-a.y)*dy)/length,0.0,1.0);
            if(std::hypot(world.x-a.x-t*dx,world.y-a.y-t*dy)<=tolerance)return true;
        }
    }
    return false;
}
bool draw_bridge_cell(SDL_Renderer* renderer,const scene::Camera2D& camera,Point ground,
    const simulation::BridgeSpan& span,simulation::Cell cell,BridgePart part,BridgeTint tint) {
    SDL_BlendMode previous=SDL_BLENDMODE_NONE;
    if(!SDL_GetRenderDrawBlendMode(renderer,&previous)||!SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND))return false;
    struct RestoreBlend {SDL_Renderer* renderer;SDL_BlendMode mode;~RestoreBlend(){(void)SDL_SetRenderDrawBlendMode(renderer,mode);}} restore{renderer,previous};
    const auto& mesh=geometry(span,cell,part);
    std::array<SDL_Vertex,face_capacity*4> vertices;std::array<int,face_capacity*6> indices;
    for(std::size_t face=0;face<mesh.count;++face) {
        const auto tint_color=color(mesh.faces[face].color,tint);
        for(std::size_t i=0;i<4;++i) {
            const auto p=mesh.faces[face].points[i];
            const auto screen=camera.world_to_screen({p.x+ground.x,p.y+ground.y});
            // These are authored raster runs. Give shared pixel boundaries the
            // same nearest screen edge, including fractional zoom, so software
            // triangle rounding cannot open water-colored cracks between runs.
            vertices[face*4+i]={{float(std::round(screen.x)),float(std::round(screen.y))},tint_color,{}};
        }
        constexpr int triangle[]={0,1,2,0,2,3};
        for(std::size_t i=0;i<6;++i)indices[face*6+i]=int(face*4)+triangle[i];
    }
    return SDL_RenderGeometry(renderer,nullptr,vertices.data(),int(mesh.count*4),indices.data(),int(mesh.count*6));
}
} // namespace openemperor
