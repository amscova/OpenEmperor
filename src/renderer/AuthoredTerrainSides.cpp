#include "renderer/AuthoredTerrainSides.h"
#include "maps/LandscapeProvenance.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
namespace openemperor {
namespace {
using Point=scene::Point;
constexpr std::size_t capacity=4096,max_instances=512;
constexpr std::array<simulation::Cell,4> directions{{{1,0},{0,1},{-1,0},{0,-1}}};
struct Run { int x,y,width;SDL_Color color; };
struct Tile { std::array<Run,capacity> runs{};std::size_t size=0; };
bool same(SDL_Color a,SDL_Color b) {return a.r==b.r&&a.g==b.g&&a.b==b.b&&a.a==b.a;}
// Fixed material, prepared before main. The surface is rough soil/stone with
// a dark rooted grass lip; no original pixels, textures or frame-time noise.
SDL_Color material(double along,double depth,unsigned direction,unsigned variant) {
    const int u=int(std::floor(along*40)),v=int(std::floor(depth));
    const unsigned noise=(unsigned(u/3+17)*37U+unsigned(v/3+31)*19U+direction*7U+variant*13U)%11U;
    int r=119,g=109,b=79;
    if(depth<3) {r=72;g=75;b=39;}
    else if(depth<9) {r=99;g=91;b=61;}
    else if(depth>31) {r=102;g=95;b=69;}
    // Uneven exposed stone lenses in the soil, with their own darker edge.
    for(const auto stone:std::array<std::array<int,4>,5>{{{{5,16,4,3}},{{17,27,5,3}},{{31,14,4,4}},{{34,34,4,2}},{{11,36,3,2}}}}) {
        const double shift=int(variant)*2-3;
        const double x=(along*40-stone[0]-shift)/stone[2],y=(depth-stone[1]+(variant%2?2:-1))/stone[3];
        const double plane=std::abs(x)+std::abs(y)*.8;
        if(std::abs(x)<1&&std::abs(y)<1&&plane<1.2) {
            const bool rim=plane>1.02;
            const int light=x+y*.5<-.1?8:-3;
            r=rim?87:129+light;g=rim?81:122+light;b=rim?59:94+light;break;
        }
    }
    if(depth<7&&u%7==2){r-=15;g-=10;b-=9;} // Sparse roots.
    const int shade=int(noise)-5-(direction==1?9:0);
    return {Uint8(r+shade),Uint8(g+shade),Uint8(b+shade),255};
}
Tile make_tile(unsigned direction,unsigned variant) {
    const auto d=directions[direction];
    const Point normal{(d.x-d.y)*20.,(d.x+d.y)*10.};
    const Point across{d.x?-20.:20.,10.};
    const Point a{normal.x-across.x,normal.y-across.y},b{normal.x+across.x,normal.y+across.y};
    const double min_x=std::min(a.x,b.x),max_x=std::max(a.x,b.x);
    Tile tile;
    for(int y=-20;y<61;++y) {
        std::array<SDL_Color,80> row{};
        for(int x=-40;x<40;++x) {
            unsigned count=0,r=0,g=0,blue=0;
            for(int sample=0;sample<4;++sample) {
                const double px=x+(sample%2?.75:.25),py=y+(sample<2?.25:.75);
                if(px<min_x||px>=max_x)continue;
                const double t=(px-a.x)/(b.x-a.x),top=a.y+(b.y-a.y)*t,depth=py-top;
                if(depth<0||depth>=40)continue;
                const auto c=material(t,depth,direction,variant);++count;r+=c.r;g+=c.g;blue+=c.b;
            }
            if(count)row[std::size_t(x+40)]={Uint8(r/count),Uint8(g/count),Uint8(blue/count),Uint8(count*255/4)};
        }
        for(int x=0;x<80;) {
            const auto c=row[std::size_t(x)];if(!c.a){++x;continue;}
            int end=x+1;while(end<80&&same(c,row[std::size_t(end)]))++end;
            if(tile.size==capacity)throw std::logic_error("authored terrain side material exceeds bound");
            tile.runs[tile.size++]={x-40,y,end-x,c};x=end;
        }
    }
    return tile;
}
const auto tiles=[] {std::array<Tile,16> out;for(unsigned i=0;i<4;++i)for(unsigned v=0;v<4;++v)out[i*4+v]=make_tile(i,v);return out;}();
}
void AuthoredTerrainSides::prepare(const maps::StoredGraphicsPlan& plan,const maps::MapGeometry& geometry,
                                  const simulation::MapPermissions& permissions) {
    constexpr auto width=maps::stored_grid_width,height=maps::stored_grid_height;
    if(!geometry.supported||plan.border!=geometry.border||!plan.landscape_layers_available||
       plan.height_bytes.size()!=width*height||plan.raw_terrain.size()!=width*height||
       plan.raw_objects.size()!=width*height||permissions.width()!=int(width)||permissions.height()!=int(height)||
       permissions.policy_version()!=simulation::kBridgesMapPermissionsPolicyVersion)
        throw std::invalid_argument("authored terrain sides require complete policy3/source height geometry");
    std::vector<Instance> candidate;
    const auto land=[&](simulation::Cell c) {
        if(!permissions.in_bounds(c)||!geometry.contains({unsigned(c.x),unsigned(c.y)}))return false;
        const auto at=std::size_t(c.y)*width+unsigned(c.x);const auto raw=plan.raw_terrain[at];
        return (raw==0x80U||raw==0xc0U)&&plan.raw_objects[at]==0&&permissions.road_allowed(c)&&
            !permissions.protected_original(c)&&!permissions.fixed_gate_at(c)&&!permissions.bridge_water(c)&&
            int(maps::landscape_height(plan,{unsigned(c.x),unsigned(c.y)}))==permissions.cell_height(c);
    };
    for(unsigned y=0;y<height;++y)for(unsigned x=0;x<width;++x) {
        const simulation::Cell high{int(x),int(y)};if(!land(high))continue;
        for(unsigned i=0;i<4;++i) {
            const auto d=directions[i];const simulation::Cell low{high.x+d.x,high.y+d.y};
            if(!land(low)||int(permissions.cell_height(high))-int(permissions.cell_height(low))!=1)continue;
            if(candidate.size()==max_instances)throw std::invalid_argument("authored terrain side instance budget exceeded");
            candidate.push_back({maps::landscape_ground(plan,{x,y}),i*4+unsigned(candidate.size()%4)});
        }
    }
    faces_=std::move(candidate);
}
bool AuthoredTerrainSides::draw(SDL_Renderer* renderer,const scene::Camera2D& camera) const {
    if(faces_.empty())return true;
    SDL_BlendMode previous{};if(!SDL_GetRenderDrawBlendMode(renderer,&previous)||
        !SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND))return false;
    struct Restore {SDL_Renderer* renderer;SDL_BlendMode mode;~Restore(){(void)SDL_SetRenderDrawBlendMode(renderer,mode);}} restore{renderer,previous};
    int width=0,height=0;if(!SDL_GetCurrentRenderOutputSize(renderer,&width,&height))return false;
    std::array<SDL_Vertex,capacity*4> vertices;std::array<int,capacity*6> indices;
    for(const auto& instance:faces_) {
        const auto a=camera.world_to_screen({instance.ground.x-40,instance.ground.y-20});
        const auto b=camera.world_to_screen({instance.ground.x+40,instance.ground.y+61});
        if(b.x<0||b.y<0||a.x>=width||a.y>=height)continue;
        const auto& tile=tiles[instance.direction];
        for(std::size_t i=0;i<tile.size;++i) {
            const auto& run=tile.runs[i];const auto c=run.color;
            const SDL_FColor color{c.r/255.F,c.g/255.F,c.b/255.F,c.a/255.F};
            const std::array<Point,4> corners{{{double(run.x),double(run.y)},{double(run.x+run.width),double(run.y)},
                {double(run.x+run.width),double(run.y+1)},{double(run.x),double(run.y+1)}}};
            for(std::size_t j=0;j<4;++j) {
                const auto p=camera.world_to_screen({instance.ground.x+corners[j].x,instance.ground.y+corners[j].y});
                vertices[i*4+j]={{float(std::round(p.x)),float(std::round(p.y))},color,{}};
            }
            constexpr int triangle[]{0,1,2,0,2,3};for(std::size_t j=0;j<6;++j)indices[i*6+j]=int(i*4)+triangle[j];
        }
        if(!SDL_RenderGeometry(renderer,nullptr,vertices.data(),int(tile.size*4),indices.data(),int(tile.size*6)))return false;
    }
    return true;
}
} // namespace openemperor
