#include "renderer/RampRenderer.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>

namespace openemperor {
namespace {
using Point=scene::Point;
using Quad=std::array<Point,4>;
struct Face { Quad points; SDL_Color color; };
constexpr std::size_t face_capacity=2048;
Point between(Point a,Point b,double t) {return {a.x+(b.x-a.x)*t,a.y+(b.y-a.y)*t};}
struct Mesh {
    // Fixed, eagerly built material detail; never generated from map IDs or time.
    std::array<Face,face_capacity> faces{};
    std::size_t count=0;
    void quad(Quad points,SDL_Color color) {
        if(count>=faces.size())throw std::logic_error("authored ramp mesh capacity exceeded");
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
    void masonry(Quad p,int phase) {
        // Tapered stone infill with quiet courses and varied blocks; prepared
        // once, distinct from the timber deck grain and not a cubic bank pad.
        for(int row=0;row<5;++row)for(int brick=0;brick<4;++brick) {
            const double a=brick/4.0,b=(brick+1)/4.0,c=row/5.0,d=(row+1)/5.0;
            const auto top0=between(p[0],p[1],a),top1=between(p[0],p[1],b);
            const auto low0=between(p[3],p[2],a),low1=between(p[3],p[2],b);
            const int shade=(row*7+brick*11+phase*3)%13-6;
            quad({between(top0,low0,c),between(top1,low1,c),between(top1,low1,d),between(top0,low0,d)},
                {Uint8(91+shade),Uint8(87+shade),Uint8(69+shade),255});
        }
        for(int row=1;row<5;++row)stroke(between(p[0],p[3],row/5.0),between(p[1],p[2],row/5.0),.3,{58,58,46,255});
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
Mesh legacy_flat_geometry(bool axis_x,int delta,int half,bool front) {
    Mesh mesh;
    const Point along{axis_x?40.0:-40.0,20.0-delta*40.0},across{axis_x?-40.0:40.0,20};
    const double first=half*.5,last=first+.5;
    auto point=[&](double t,double side){return Point{along.x*t+across.x*side,along.y*t+across.y*side};};
    if(!front) {
        // Low stone rubble supports the actual graded surface. It tapers to
        // zero at the low road; no floating plate or raised bank cubes.
        for(const double side:{-.25,.25})for(int i=0;i<4;++i) {
            const double a=first+i*.125,b=a+.125;
            auto low=[&](double t){return Point{along.x*t+across.x*side,20*t+std::max(0,-delta)*40+across.y*side};};
            const auto a0=point(a,side),b0=point(b,side);
            if(std::hypot(a0.x-low(a).x,a0.y-low(a).y)>.1||std::hypot(b0.x-low(b).x,b0.y-low(b).y)>.1)
                mesh.masonry({a0,b0,low(b),low(a)},i+half*4);
        }
        constexpr SDL_Color planks[]={{117,103,77,255},{108,96,73,255},{122,106,81,255},{111,98,72,255}};
        for(int i=0;i<6;++i) {
            const double a=first+i/12.0,b=a+1/12.0;
            mesh.timber({point(a,-.25),point(a,.25),point(b,.25),point(b,-.25)},planks[(i+half*6)%4],12,2,i);
            mesh.stroke(point(a,-.25),point(a,.25),.5,{60,53,41,255});
        }
        for(const auto side:{-.25,.25})mesh.stroke(point(first,side),point(last,side),2.0,{76,66,48,255},true);
    }
    const double side=front?.25:-.25;
    for(const auto t:{first,last}) {
        const auto p0=point(t,side);mesh.stroke(p0,lift(p0,11),1.8,{82,71,52,255},true);mesh.post_cap(lift(p0,11));
    }
    mesh.stroke(lift(point(first,side),10),lift(point(last,side),10),1.7,{116,99,73,255},true);
    mesh.stroke(lift(point(first,side),5),lift(point(last,side),5),1,{83,72,52,255},true);
    return mesh;
}
Mesh make_geometry(bool axis_x,int delta,int half,bool front) {
    if(delta==0)return legacy_flat_geometry(axis_x,delta,half,front);
    Mesh mesh;const double first=half*.5;
    const simulation::RampLink r{{1},{0,0},axis_x?simulation::Cell{1,0}:simulation::Cell{0,1},0,std::int8_t(delta),40,0};
    const auto point=[&](double t,double side){return ramp_surface_point({},r,t,side*ramp_surface_width(t)/ramp_surface_half_width);};
    const auto native=[&](double t,double side){const double edge=side<0?-.46:.46;const auto p=ramp_surface_point({},r,t,edge);return Point{p.x,20*t-(half?delta*40:0)+20*edge};};
    const auto shade=[](int x,int y){auto n=std::uint32_t(x+71)*0x8da6b343U^std::uint32_t(y+91)*0xd8163841U;n^=n>>13;n*=0x85ebca6bU;return int(n%15)-7;};
    const auto flank=[&](double side) {
        // A cut bank on the upper half and a low supported embankment below.
        // Both use the same signed slope as the road and the terrain cutout.
        for(int along=0;along<12;++along)for(int row=0;row<5;++row) {
            const double a=first+along/24.,b=first+(along+1)/24.;
            const auto a0=point(a,side),a1=point(b,side),b0=native(a,side),b1=native(b,side);
            if(std::hypot(a0.x-b0.x,a0.y-b0.y)+std::hypot(a1.x-b1.x,a1.y-b1.y)<.01)continue;
            const int light=shade(along+half*12,row+int(side*20));
            const auto c=SDL_Color{Uint8(137+light),Uint8(130+light),Uint8(109+light),255};
            mesh.quad({between(a0,b0,row/5.),between(a1,b1,row/5.),between(a1,b1,(row+1)/5.),between(a0,b0,(row+1)/5.)},c);
        }
        // Low, broken gravel edging, not a handrail or a row of timber posts.
        for(int i=0;i<12;++i) {
            const double a=first+i/24.,b=first+(i+1)/24.;const double outer=side+(side<0?-.035:.035);
            const int light=shade(i,half+int(side*100));
            mesh.quad({point(a,side),point(b,side),point(b,outer),point(a,outer)},
                {Uint8(155+light),Uint8(142+light),Uint8(109+light),255});
        }
    };
    if(front){flank(ramp_surface_half_width);return mesh;}
    flank(-ramp_surface_half_width);
    // Continuous compacted earth/stone, with subtle irregular wear. No rungs,
    // plank seams, tall posts or independent artwork height offsets.
    for(int along=0;along<24;++along)for(int cross=0;cross<10;++cross) {
        const double a=first+along/48.,b=first+(along+1)/48.;
        const double c=-ramp_surface_half_width+cross*(2*ramp_surface_half_width/10.);
        const double d=c+2*ramp_surface_half_width/10.;
        const int light=shade(along+half*24,cross);
        const int edge=cross==0||cross==9?-9:0;
        const bool stone=shade(along+half*24,cross)>4;
        mesh.quad({point(a,c),point(b,c),point(b,d),point(a,d)},
            {Uint8((stone?192:208)+light+edge),Uint8((stone?178:187)+light+edge),Uint8((stone?143:136)+light+edge),255});
    }
    // Small irregular chips break up the compacted earth. Their independent
    // positions avoid transverse rungs or a checker of rectangular pavers.
    for(int i=0;i<65;++i) {
        const auto h=std::uint32_t(i+half*65+17)*0x9e3779b9U;
        const double t=first+.015+double(h%1000)/1000.*.47;
        const double s=-.23+double((h>>10)%1000)/1000.*.46;
        const double length=.006+double((h>>20)%7)*.0015,width=.008+double(h%5)*.002;
        const auto a=point(t-length,s),b=point(t,s-width),c=point(t+length,s+.003),d=point(t,s+width);
        const int light=shade(i,half);
        mesh.quad({a,b,c,d},{Uint8(225+light),Uint8(209+light),Uint8(169+light),255});
    }
    // At the exact 2:1 projection two ascending orientations are edge-on.
    // A shallow stone crest gives the unchanged foot line visible thickness;
    // it does not move the path, surface height or the camera projection.
    if(delta==1)for(int i=0;i<24;++i) {
        const double a=first+i/48.,b=first+(i+1)/48.;const int light=shade(i/2+half*12,3);
        mesh.stroke(point(a,0),point(b,0),2.2,{Uint8(188+light),Uint8(171+light),Uint8(122+light),255});
    }
    return mesh;
}
Mesh pixel_contours(const Mesh& input) {
    constexpr int side=192,origin=64;
    std::array<SDL_Color,side*side*4> samples{};
    for(std::size_t i=0;i<input.count;++i) {
        const auto& face=input.faces[i];
        double left=128,right=-64,top=128,bottom=-64;
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
// X/Y, descending/flat-debug/ascending, two halves and local back/front.
// All material/raster work precedes the frame loop, independent of maps.
const auto meshes=[] {
    std::array<Mesh,24> result;
    for(int axis=0;axis<2;++axis)for(int slope=-1;slope<=1;++slope)
        for(int half=0;half<2;++half)for(int front=0;front<2;++front)
            result[std::size_t(axis*12+(slope+1)*4+half*2+front)]=pixel_contours(make_geometry(axis==0,slope,half,front!=0));
    return result;
}();
const auto opaque=[] {
    std::array<std::array<bool,192*192>,6> result{};
    for(int axis=0;axis<2;++axis)for(int slope=-1;slope<=1;++slope)
        for(int half=0;half<2;++half)for(int front=0;front<2;++front) {
            const auto& mesh=meshes[std::size_t(axis*12+(slope+1)*4+half*2+front)];
            auto& pixels=result[std::size_t(axis*3+slope+1)];
            for(std::size_t i=0;i<mesh.count;++i)if(mesh.faces[i].color.a==255) {
                const auto& p=mesh.faces[i].points;
                for(int y=int(p[0].y)+64;y<int(p[2].y)+64;++y)for(int x=int(p[0].x)+64;x<int(p[2].x)+64;++x)
                    if(x>=0&&y>=0&&x<192&&y<192)pixels[std::size_t(y*192+x)]=true;
            }
        }
    return result;
}();
const Mesh& geometry(const simulation::RampLink& r,int half,bool front,bool elevated) {
    const int axis=r.endpoint_a.x!=r.endpoint_b.x?0:1;
    const int slope=elevated?int(r.height_b)-int(r.height_a):0;
    return meshes[std::size_t(axis*12+(slope+1)*4+half*2+int(front))];
}
SDL_FColor color(SDL_Color base,BridgeTint tint) {
    if(tint==BridgeTint::Normal)return {base.r/255.0F,base.g/255.0F,base.b/255.0F,base.a/255.0F};
    const SDL_FColor overlay=tint==BridgeTint::Valid?SDL_FColor{.20F,.70F,.42F,1}:
        tint==BridgeTint::Unaffordable?SDL_FColor{.85F,.62F,.20F,1}:SDL_FColor{.80F,.25F,.23F,1};
    return {overlay.r*.8F+base.r/255.0F*.2F,overlay.g*.8F+base.g/255.0F*.2F,overlay.b*.8F+base.b/255.0F*.2F,.55F*base.a/255.0F};
}
}
bool ramp_opaque_at(Point world,Point ground,const simulation::RampLink& r) {
    const int x=int(std::floor(world.x-ground.x))+64,y=int(std::floor(world.y-ground.y))+64;
    const int slope=int(r.height_b)-int(r.height_a);
    if(x<0||y<0||x>=192||y>=192||std::abs(slope)>1)return false;
    return opaque[std::size_t((r.endpoint_a.x!=r.endpoint_b.x?0:1)*3+slope+1)][std::size_t(y*192+x)];
}
bool hit_ramp_part(Point world,Point ground,const simulation::RampLink& ramp,int half,bool front,bool elevated) {
    const auto& mesh=geometry(ramp,half,front,elevated);world={world.x-ground.x,world.y-ground.y};
    for(std::size_t i=0;i<mesh.count;++i)if(inside_polygon(world,mesh.faces[i].points))return true;
    return false;
}
bool draw_ramp_part(SDL_Renderer* renderer,const scene::Camera2D& camera,Point ground,
    const simulation::RampLink& ramp,int half,bool front,bool elevated,BridgeTint tint) {
    SDL_BlendMode previous=SDL_BLENDMODE_NONE;
    if(!SDL_GetRenderDrawBlendMode(renderer,&previous)||!SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND))return false;
    struct RestoreBlend {SDL_Renderer* renderer;SDL_BlendMode mode;~RestoreBlend(){(void)SDL_SetRenderDrawBlendMode(renderer,mode);}} restore{renderer,previous};
    const auto& mesh=geometry(ramp,half,front,elevated);
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
