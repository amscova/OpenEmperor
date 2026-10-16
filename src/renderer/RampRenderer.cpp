#include "renderer/RampRenderer.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
#include <vector>
#include <bitset>
#include "assets/RoadVisualProfile.h"

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
template<class Emit> void pixel_contour_runs(const Mesh& input,Emit emit) {
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
            emit(Quad{Point{double(x-origin),double(y-origin)},Point{double(end-origin),double(y-origin)},
                Point{double(end-origin),double(y-origin+1)},Point{double(x-origin),double(y-origin+1)}},c);
            x=end;
        }
    }
}

// Compact immutable native-pixel runs replace the former 24 fixed Face arrays.
// Both materials and all four lateral topologies stay below the old cache size.
struct Run { std::int8_t x,y;std::uint8_t width,r,g,b,a; };
static_assert(sizeof(Run)==7);
using Part=std::vector<Run>;
struct Layout { std::array<Part,4> parts; };
using Layouts=std::array<Layout,16>; // X/Y * descending/ascending * adjacency.
constexpr int raster_side=192,raster_origin=64;
constexpr std::size_t max_part_runs=4096,max_cache_bytes=3U*1024U*1024U;
constexpr unsigned material_phases=8;
std::size_t index(bool x,int slope,unsigned adjacency) {
    return std::size_t((x?0:1)*8+(slope>0?4:0)+(adjacency&3U));
}
std::uint32_t material_hash(int x,int y) {
    auto n=std::uint32_t(x+137)*0x8da6b343U^std::uint32_t(y+193)*0xd8163841U;
    n^=n>>13;n*=0x85ebca6bU;n^=n>>16;return n;
}
// Native-pixel earth, embedded stones and irregular chipped rims. It is used
// without a compatible profile and fills only the source diamond's missing
// edge pixels. There is no smooth low-frequency beige shoulder material.
SDL_Color authored_material(bool axis,double t,double side,unsigned adjacency) {
    const double local=t<.5?t:t-1.;
    const int px=int(std::floor(39+(axis?40*local-40*side:40*side-40*local)));
    const int py=int(std::floor(20+20*local+20*side));
    const auto h=material_hash(px,py),grain=material_hash(px*3+1,py*3-1);
    int r=221,g=189,b=133;
    int light=int(h%37)-18;
    const bool exterior=(side<0?!(adjacency&1U):!(adjacency&2U));
    const double rim=.27+double(material_hash(int(std::floor(t*17)),side<0?13:29)%8)/100.;
    if(exterior&&std::abs(side)>rim&&std::abs(side)<rim+.095) {
        const auto stone=material_hash(px/3,py/2);
        r=220;g=210;b=174;light=int(stone%61)-30;
        if(grain%7==0){r=149;g=133;b=100;light=int(h%17);}
    } else if(grain%13<3) {r=236;g=208;b=157;light=int(h%25)-12;}
    else if(grain%19==0){r=162;g=139;b=102;light=int(h%21);}
    // Keep the actual foot line visible in the independently authored mode.
    return {Uint8(std::clamp(r+light,125,255)),Uint8(std::clamp(g+light,110,255)),
        Uint8(std::clamp(b+light,65,255)),255};
}
const assets::RgbaImage* road_image(const assets::RoadVisualProfile* profile,bool axis,unsigned adjacency) {
    if(!profile||!profile->replaces_ground)return nullptr;
    const std::uint8_t mask=axis?std::uint8_t(0xa|((adjacency&1U)?1:0)|((adjacency&2U)?4:0)):
        std::uint8_t(0x5|((adjacency&1U)?8:0)|((adjacency&2U)?2:0));
    const auto* entry=profile->find(mask);
    if(!entry||entry->ground_x!=39||entry->ground_y!=20||entry->image_index>=profile->unique_images.size())return nullptr;
    const auto& image=profile->unique_images[entry->image_index];
    if(image.width!=78||image.height!=40||image.pixels.size()!=78U*40U*4U)return nullptr;
    return &image;
}
SDL_Color surface_material(const assets::RgbaImage* image,bool axis,double t,double side,unsigned adjacency,unsigned phase) {
    if(image) {
        const double local=t<.5?t:t-1.;
        int x=int(std::floor(39+(axis?40*local-40*side:40*side-40*local)));
        int y=int(std::floor(20+20*local+20*side));
        // A coordinate interpolation between the road and its earth patch
        // magnified individual source texels into flowing beige streaks. Keep
        // native texels instead: each prepared pixel selects either the exact
        // endpoint/outer-road sample or a native central-earth sample.
        const auto smooth=[](double a,double b,double value) {
            const auto u=std::clamp((value-a)/(b-a),0.,1.);return u*u*(3-2*u);
        };
        const bool joined=side<0?(adjacency&1U):(adjacency&2U);
        const double weight=smooth(.04,.18,t)*(1-smooth(.82,.96,t))*
            (joined?1.:1-smooth(.20,.34,std::abs(side)));
        // Continuous ungraded coordinates cross both the half seam and a
        // lateral neighbor unchanged. Eight eagerly prepared phases prevent
        // the same small patch from recurring on every neighboring link.
        // Wrap the coordinate itself, so phase7/+0.5 and phase0/-0.5
        // address the same field at the cache boundary as well.
        const double material_side=std::fmod(side+phase+material_phases,double(material_phases));
        const int u=int(std::floor(39+(axis?40*t-40*material_side:40*material_side-40*t)));
        const int v=int(std::floor(20+20*t+20*material_side));
        const auto positive=[](int value,int divisor){const int r=value%divisor;return r<0?r+divisor:r;};
        const int chunk_x=int(std::floor(double(u)/6)),chunk_y=int(std::floor(double(v)/4));
        const auto chunk=material_hash(chunk_x,chunk_y),pixel=material_hash(u,v);
        if(double(pixel&65535U)/65535.<weight) {
            // Six-by-four native pieces preserve short grain runs. Offsets
            // select only this same image's bounded 12x8 earth rectangle;
            // there is no color classification or additional asset table.
            x=33+positive(positive(u,6)+int(chunk%12),12);
            y=16+positive(positive(v,4)+int((chunk>>8)%8),8);
        }
        if(x>=0&&y>=0&&x<78&&y<40) {
            const auto at=std::size_t((y*78+x)*4);
            if(image->pixels[at+3]==255)return {image->pixels[at],image->pixels[at+1],image->pixels[at+2],255};
        }
    }
    return authored_material(axis,t,side,adjacency);
}
struct Parameters { double t=0,side=0;bool inside=false; };
Parameters parameters(Point p,bool axis,int slope,unsigned adjacency) {
    const Point along{axis?40.:-40.,20.-slope*40.},across{axis?-40.:40.,20.};
    const double determinant=along.x*across.y-along.y*across.x;
    Parameters result;
    if(std::abs(determinant)<1e-9) {
        // Fixed 2:1/height40 ascending projection is exactly edge-on. Retain
        // its 2.2px authored crest and unchanged authoritative foot line.
        const double length=std::hypot(along.x,along.y);
        result.t=(p.x*along.x+p.y*along.y)/(length*length);
        const double distance=(p.x*along.y-p.y*along.x)/length;
        result.inside=result.t>=0&&result.t<1&&std::abs(distance)<1.1;
        return result;
    }
    result.t=(p.x*across.y-p.y*across.x)/determinant;
    result.side=(along.x*p.y-along.y*p.x)/determinant;
    const double t=result.t;
    // A physical shared boundary is exact and half-open. Exterior edges have
    // small deterministic erosion, without per-link overlapping bank meshes.
    const auto limit=[&](bool positive) {
        if(adjacency&(positive?2U:1U))return .5;
        const int phase=int(std::floor(std::clamp(t,0.,1.)*20));
        return .46-(double(material_hash(phase,positive?31:17)%4)/100.)*std::sin(std::clamp(t,0.,1.)*3.141592653589793);
    };
    result.inside=t>=0&&t<1&&result.side>=-limit(false)&&result.side<limit(true);
    return result;
}
struct Pixel { SDL_Color color{};int owner=0; };
Pixel slope_pixel(int x,int y,bool axis,int slope,unsigned adjacency,const assets::RgbaImage* image,unsigned phase) {
    const Point center{x+.5,y+.5};auto at=parameters(center,axis,slope,adjacency);
    unsigned count=0;Parameters sample=at;
    for(int i=0;i<4;++i) {
        auto point=parameters({x+(i%2?.75:.25),y+(i<2?.25:.75)},axis,slope,adjacency);
        if(point.inside){++count;if(!at.inside)sample=point;}
    }
    // Separately antialiased shared edges can expose 25% background. Interior
    // boundaries instead have one opaque pixel owner, just like the half seam.
    if(slope<0&&((adjacency&1U&&at.side<-.46)||(adjacency&2U&&at.side>.46))) {
        if(!at.inside)return {};
        count=4;
    }
    if(!count)return {};
    if(at.inside)sample=at;
    auto color=surface_material(image,axis,std::clamp(sample.t,0.,1.-1e-9),sample.side,adjacency,phase);
    color.a=Uint8(count*255/4);
    // Only the external near shoulder uses the front painter component. The
    // tread and every joined interior have one back/surface owner.
    const bool front=slope<0&&!(adjacency&2U)&&sample.side>.32;
    return {color,(sample.t<.5?0:2)+int(front)};
}
bool same(SDL_Color a,SDL_Color b){return a.r==b.r&&a.g==b.g&&a.b==b.b&&a.a==b.a;}
void append(Part& part,Run run) {
    if(part.size()==max_part_runs)throw std::logic_error("prepared ramp run bound exceeded");
    part.push_back(run);
}
Layout graded_layout(bool axis,int slope,unsigned adjacency,const assets::RgbaImage* image,unsigned phase=0) {
    Layout result;
    for(int y=-24;y<88;++y) {
        std::array<Pixel,128> row{};
        for(int x=-64;x<64;++x)row[std::size_t(x+64)]=slope_pixel(x,y,axis,slope,adjacency,image,phase);
        for(int x=0;x<128;) {
            const auto pixel=row[std::size_t(x)];if(!pixel.color.a){++x;continue;}
            int end=x+1;while(end<128&&row[std::size_t(end)].owner==pixel.owner&&same(row[std::size_t(end)].color,pixel.color))++end;
            const auto c=pixel.color;append(result.parts[std::size_t(pixel.owner)],{std::int8_t(x-64),std::int8_t(y),std::uint8_t(end-x),c.r,c.g,c.b,c.a});x=end;
        }
    }
    for(auto& part:result.parts)part.shrink_to_fit();
    return result;
}
std::size_t bytes(const Layouts& layouts) {
    std::size_t count=0;for(const auto& layout:layouts)for(const auto& part:layout.parts)count+=part.capacity()*sizeof(Run);return count;
}
const Layouts fallback=[] {
    Layouts result;
    for(const bool axis:{true,false})for(const int slope:{-1,1})for(unsigned adjacency=0;adjacency<4;++adjacency)
        result[index(axis,slope,adjacency)]=graded_layout(axis,slope,adjacency,nullptr);
    if(bytes(result)>max_cache_bytes)throw std::logic_error("authored ramp cache budget exceeded");
    return result;
}();
const auto flat=[] {
    std::array<Part,8> result;
    for(int axis=0;axis<2;++axis)for(int half=0;half<2;++half)for(int front=0;front<2;++front) {
        auto& out=result[std::size_t(axis*4+half*2+front)];
        pixel_contour_runs(legacy_flat_geometry(axis==0,0,half,front!=0),[&](const Quad& p,SDL_Color c) {
            append(out,{std::int8_t(p[0].x),std::int8_t(p[0].y),std::uint8_t(p[1].x-p[0].x),c.r,c.g,c.b,c.a});
        });out.shrink_to_fit();
    }
    return result;
}();
const auto opaque=[] {
    std::array<std::bitset<raster_side*raster_side>,16> result;
    for(std::size_t i=0;i<fallback.size();++i)for(const auto& part:fallback[i].parts)for(const auto& run:part)if(run.a==255)
        for(int x=int(run.x);x<int(run.x)+run.width;++x)result[i].set(std::size_t((int(run.y)+raster_origin)*raster_side+x+raster_origin));
    return result;
}();
SDL_FColor color(SDL_Color base,BridgeTint tint,bool flat_comparison) {
    if(tint==BridgeTint::Normal)return {base.r/255.0F,base.g/255.0F,base.b/255.0F,base.a/255.0F};
    const SDL_FColor overlay=tint==BridgeTint::Valid?SDL_FColor{.20F,.70F,.42F,1}:
        tint==BridgeTint::Unaffordable?SDL_FColor{.85F,.62F,.20F,1}:SDL_FColor{.80F,.25F,.23F,1};
    return {overlay.r*.8F+base.r/255.0F*.2F,overlay.g*.8F+base.g/255.0F*.2F,overlay.b*.8F+base.b/255.0F*.2F,
        (flat_comparison?.55F:1.F)*base.a/255.0F};
}
bool valid(const simulation::RampLink& r,int half) {
    return (half==0||half==1)&&std::abs(int(r.height_b)-int(r.height_a))==1;
}
const Part& selected(const Layouts& layouts,const simulation::RampLink& r,int half,bool front,bool elevated,unsigned adjacency) {
    const bool axis=r.endpoint_a.x!=r.endpoint_b.x;
    if(!elevated)return flat[std::size_t((axis?0:1)*4+half*2+int(front))];
    return layouts[index(axis,int(r.height_b)-int(r.height_a),adjacency)].parts[std::size_t(half*2+int(front))];
}
bool draw_runs(SDL_Renderer* renderer,const scene::Camera2D& camera,Point ground,const Part& part,BridgeTint tint,bool flat_comparison) {
    SDL_BlendMode previous=SDL_BLENDMODE_NONE;
    if(!SDL_GetRenderDrawBlendMode(renderer,&previous)||!SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND))return false;
    struct RestoreBlend {SDL_Renderer* renderer;SDL_BlendMode mode;~RestoreBlend(){(void)SDL_SetRenderDrawBlendMode(renderer,mode);}} restore{renderer,previous};
    std::array<SDL_Vertex,max_part_runs*4> vertices;std::array<int,max_part_runs*6> indices;
    for(std::size_t i=0;i<part.size();++i) {
        const auto& run=part[i];const auto c=color({run.r,run.g,run.b,run.a},tint,flat_comparison);
        const std::array<Point,4> corners{{{double(run.x),double(run.y)},{double(int(run.x)+run.width),double(run.y)},
            {double(int(run.x)+run.width),double(int(run.y)+1)},{double(run.x),double(int(run.y)+1)}}};
        for(std::size_t j=0;j<4;++j) {
            const auto p=camera.world_to_screen({ground.x+corners[j].x,ground.y+corners[j].y});
            vertices[i*4+j]={{float(std::round(p.x)),float(std::round(p.y))},c,{}};
        }
        constexpr int triangle[]={0,1,2,0,2,3};for(std::size_t j=0;j<6;++j)indices[i*6+j]=int(i*4)+triangle[j];
    }
    return part.empty()||SDL_RenderGeometry(renderer,nullptr,vertices.data(),int(part.size()*4),indices.data(),int(part.size()*6));
}
bool hit_runs(Point p,const Part& part) {
    for(const auto& run:part)if(p.x>=run.x&&p.x<=int(run.x)+run.width&&p.y>=run.y&&p.y<=int(run.y)+1)return true;
    return false;
}
}
struct RampPresentation::Data { std::array<Layouts,material_phases> phases; };
RampPresentation::RampPresentation()=default;
RampPresentation::~RampPresentation()=default;
RampPresentation::RampPresentation(RampPresentation&&) noexcept=default;
RampPresentation& RampPresentation::operator=(RampPresentation&&) noexcept=default;
void RampPresentation::prepare(const assets::RoadVisualProfile* profile) {
    if(!profile||!profile->replaces_ground){clear();return;}
    auto candidate=std::make_unique<Data>();
    std::size_t total=bytes(fallback);
    for(unsigned phase=0;phase<material_phases;++phase) {
        for(const bool axis:{true,false})for(const int slope:{-1,1})for(unsigned adjacency=0;adjacency<4;++adjacency) {
            const auto* image=road_image(profile,axis,adjacency);
            if(image)candidate->phases[phase][index(axis,slope,adjacency)]=graded_layout(axis,slope,adjacency,image,phase);
        }
        total+=bytes(candidate->phases[phase]);
    }
    if(total>max_cache_bytes)throw std::logic_error("combined ramp material cache budget exceeded");
    data_=std::move(candidate);
}
void RampPresentation::clear(){data_.reset();}
std::size_t RampPresentation::geometry_bytes() const {
    std::size_t count=bytes(fallback)+sizeof(opaque);
    if(data_)for(const auto& phase:data_->phases)count+=bytes(phase);
    for(const auto& part:flat)count+=part.capacity()*sizeof(Run);return count;
}
bool RampPresentation::draw_part(SDL_Renderer* renderer,const scene::Camera2D& camera,Point ground,const simulation::RampLink& ramp,
        int half,bool front,bool elevated,BridgeTint tint,unsigned adjacency,bool road_material) const {
    if(!valid(ramp,half))return false;
    const Layouts* layouts=&fallback;
    if(elevated&&road_material&&data_) {
        const int lateral=ramp.endpoint_a.x!=ramp.endpoint_b.x?ramp.endpoint_a.y:ramp.endpoint_a.x;
        const auto phase=unsigned((lateral%int(material_phases)+int(material_phases))%int(material_phases));
        const auto& prepared=data_->phases[phase];
        const auto i=index(ramp.endpoint_a.x!=ramp.endpoint_b.x,int(ramp.height_b)-int(ramp.height_a),adjacency);
        bool available=false;for(const auto& p:prepared[i].parts)available|=!p.empty();
        if(available)layouts=&prepared;
    }
    return draw_runs(renderer,camera,ground,selected(*layouts,ramp,half,front,elevated,adjacency),tint,!elevated);
}
bool RampPresentation::hit_part(Point world,Point ground,const simulation::RampLink& ramp,int half,bool front,bool elevated,unsigned adjacency) const {
    if(!valid(ramp,half))return false;
    return hit_runs({world.x-ground.x,world.y-ground.y},selected(fallback,ramp,half,front,elevated,adjacency));
}
bool ramp_opaque_at(Point world,Point ground,const simulation::RampLink& r,unsigned adjacency) {
    const int x=int(std::floor(world.x-ground.x))+raster_origin,y=int(std::floor(world.y-ground.y))+raster_origin;
    const int slope=int(r.height_b)-int(r.height_a);
    if(x<0||y<0||x>=raster_side||y>=raster_side||std::abs(slope)!=1)return false;
    return opaque[index(r.endpoint_a.x!=r.endpoint_b.x,slope,adjacency)][std::size_t(y*raster_side+x)];
}
bool hit_ramp_part(Point world,Point ground,const simulation::RampLink& ramp,int half,bool front,bool elevated) {
    const RampPresentation fallback_presentation;
    return fallback_presentation.hit_part(world,ground,ramp,half,front,elevated);
}
bool draw_ramp_part(SDL_Renderer* renderer,const scene::Camera2D& camera,Point ground,const simulation::RampLink& ramp,
        int half,bool front,bool elevated,BridgeTint tint) {
    const RampPresentation fallback_presentation;
    return fallback_presentation.draw_part(renderer,camera,ground,ramp,half,front,elevated,tint,0,false);
}
} // namespace openemperor
