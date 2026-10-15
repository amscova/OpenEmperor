#include "renderer/AuthoredTerrainSides.h"
#include "renderer/RampRenderer.h"
#include "maps/LandscapeProvenance.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <stdexcept>
#include <tuple>
namespace openemperor {
namespace {
using Point=scene::Point;
constexpr std::size_t capacity=4096,max_instances=512,max_geometry_bytes=8U*1024U*1024U;
constexpr std::array<simulation::Cell,4> directions{{{1,0},{0,1},{-1,0},{0,-1}}};
using Corner=std::tuple<int,int,int>;
std::optional<std::uint32_t> edge_key(simulation::Cell a,simulation::Cell b) {
    if(a.x<0||a.y<0||b.x<0||b.y<0||a.x>=228||a.y>=228||b.x>=228||b.y>=228||
       std::abs(a.x-b.x)+std::abs(a.y-b.y)!=1)return {};
    return std::uint32_t(std::min(a.y*228+a.x,b.y*228+b.x)*2+(a.y!=b.y));
}
Corner corner(simulation::Cell c,unsigned direction,int sign,int height) {
    const auto d=directions[direction];return {c.x*2+d.x+(d.y?sign:0),c.y*2+d.y+(d.x?sign:0),height};
}
std::uint32_t hash(int x,int y) {
    auto n=std::uint32_t(x)*0x8da6b343U^std::uint32_t(y)*0xd8163841U;
    n^=n>>13;n*=0x85ebca6bU;n^=n>>16;return n;
}
double noise(double x,double y,int scale) {
    x/=scale;y/=scale;const int ix=int(std::floor(x)),iy=int(std::floor(y));
    const double fx=x-ix,fy=y-iy;
    const auto value=[](int a,int b){return (hash(a,b)&65535U)/65535.;};
    return (value(ix,iy)*(1-fx)+value(ix+1,iy)*fx)*(1-fy)+(value(ix,iy+1)*(1-fx)+value(ix+1,iy+1)*fx)*fy;
}
// Continuous projected-world strata share boundary values. No repeating tile
// image, original pixels, frame-time randomness or map-name selection.
SDL_Color material(Point world,double depth,unsigned direction,bool end) {
    const double broad=noise(world.x,world.y,23),grain=noise(world.x,world.y,2);
    // Tall, broken rock facets, not horizontal mortar courses. Seed positions
    // are continuous across connected edges; narrow fissures have worn rims.
    const double u=world.x/5.+(noise(world.x,world.y,5)-.5)*.7,v=depth/12.;const int ix=int(std::floor(u)),iy=int(std::floor(v));
    double first=1e9,second=1e9,facet_x=0;std::uint32_t rock=0;
    for(int y=iy-1;y<=iy+1;++y)for(int x=ix-1;x<=ix+1;++x) {
        const auto h=hash(x,y);const double dx=u-x-.15-(h&255U)/365.,dy=v-y-.15-((h>>8)&255U)/365.;
        const double d=dx*dx+dy*dy;
        if(d<first){second=first;first=d;rock=h;facet_x=dx;}else second=std::min(second,d);
    }
    int r=137,g=129,b=116;
    if(rock%5==0){r=148;g=131;b=102;}else if(rock%5==1){r=125;g=119;b=119;}
    // Fine stepped highlights interrupt the facets. Only some fractures are
    // exposed; drawing every cell outline would look like fitted masonry.
    int shade=int(rock%27)-13+int((grain-.5)*29)+int((noise(world.x,world.y,1)-.5)*15);
    shade+=facet_x<-.15?8:facet_x>.3?-7:0;
    const double fissure=second-first;
    if(fissure<.09&&grain<.67){shade-=14+int(grain*10);}else if(fissure<.19&&grain>.38){shade+=11;}
    const double roots=noise(world.x+7,world.y,6);
    if(depth<1+roots*4){r=77;g=83;b=43;shade=int(grain*13)-7;}
    else if(depth<3+roots*5){r=111;g=105;b=76;}
    if(depth>31+broad*9){r=151;g=137;b=103;shade=int((grain-.5)*23)-4;}
    if(end)shade-=int((1-broad)*5);shade-=direction==1?8:direction==2?4:0;
    const auto c=[&](int v){return Uint8(std::clamp(v+shade,0,255));};return {c(r),c(g),c(b),255};
}
}
void AuthoredTerrainSides::clear(){faces_.clear();edge_index_.clear();committed_.clear();link_count_=0;preview_.reset();preview_face_.reset();topology_.prepare({});}
std::size_t AuthoredTerrainSides::logical_geometry_bytes() const {
    std::size_t bytes=0;for(const auto& f:faces_)bytes+=f.runs.capacity()*sizeof(Run);return bytes;
}
void AuthoredTerrainSides::prepare(const maps::StoredGraphicsPlan& plan,const maps::MapGeometry& geometry,
                                  const simulation::MapPermissions& permissions) {
    constexpr auto width=maps::stored_grid_width,height=maps::stored_grid_height;
    if(!geometry.supported||plan.border!=geometry.border||!plan.landscape_layers_available||
       plan.height_bytes.size()!=width*height||plan.raw_terrain.size()!=width*height||
       plan.raw_objects.size()!=width*height||permissions.width()!=int(width)||permissions.height()!=int(height)||
       permissions.policy_version()!=simulation::kBridgesMapPermissionsPolicyVersion)
        throw std::invalid_argument("authored terrain sides require complete policy3/source height geometry");
    std::vector<Instance> candidate;std::map<Corner,unsigned> joints;
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
            Instance f;f.ground=maps::landscape_ground(plan,{x,y});f.high=high;f.low=low;f.height=permissions.cell_height(high);f.direction=i;
            ++joints[corner(high,i,-1,f.height)];++joints[corner(high,i,1,f.height)];candidate.push_back(std::move(f));
        }
    }
    std::size_t bytes=0;
    // Only this local authored edge stream is ordered; the scene's painter and
    // all original landscape/Bridge descriptors remain unchanged.
    std::stable_sort(candidate.begin(),candidate.end(),[](const Instance& a,const Instance& b){
        return std::tuple(a.high.x+a.high.y,a.high.x-a.high.y,a.direction<2?1:0,a.direction)<
               std::tuple(b.high.x+b.high.y,b.high.x-b.high.y,b.direction<2?1:0,b.direction);
    });
    for(auto& f:candidate) {
        f.start_degree=joints.at(corner(f.high,f.direction,-1,f.height));f.end_degree=joints.at(corner(f.high,f.direction,1,f.height));
        const auto d=directions[f.direction];const Point normal{(d.x-d.y)*20.,(d.x+d.y)*10.};
        const Point across{d.x?-20.:20.,10.},a{normal.x-across.x,normal.y-across.y},b{normal.x+across.x,normal.y+across.y};
        simulation::RampLink ramp{{1},f.high,f.low,f.height,std::int8_t(int(f.height)-1),40,0};
        Point ramp_ground=f.ground;
        if(ramp.endpoint_a.x>ramp.endpoint_b.x||ramp.endpoint_a.y>ramp.endpoint_b.y) {
            std::swap(ramp.endpoint_a,ramp.endpoint_b);std::swap(ramp.height_a,ramp.height_b);
            ramp_ground={f.ground.x+d.x*40.-d.y*40.,f.ground.y+(d.x+d.y)*20.+40};
        }
        const auto lateral=ramp_lateral_step(ramp);
        const Point across_ramp{ramp.endpoint_a.x!=ramp.endpoint_b.x?-40.:40.,20.};
        std::array<simulation::RampLink,2> neighbors{ramp,ramp};
        for(unsigned n=0;n<2;++n) {
            const int sign=n==0?-1:1;
            neighbors[n].endpoint_a.x+=lateral.x*sign;neighbors[n].endpoint_a.y+=lateral.y*sign;
            neighbors[n].endpoint_b.x+=lateral.x*sign;neighbors[n].endpoint_b.y+=lateral.y*sign;
        }
        // All five states use the same material samples. Only sample coverage
        // differs; identical prepared runs share one visibility bitset.
        std::array<std::size_t,5> state_runs{};
        for(int y=-22;y<65;++y) {
            std::array<std::array<SDL_Color,88>,5> rows{};
            for(int x=-44;x<44;++x) {
                std::array<unsigned,5> counts{},red{},green{},blue{};
                for(int sample=0;sample<4;++sample) {
                    const double px=x+(sample%2?.75:.25),py=y+(sample<2?.25:.75);
                    const double t=(px-a.x)/(b.x-a.x),top=a.y+(b.y-a.y)*t,depth=py-top;
                    const Point world{f.ground.x+px,f.ground.y+py};
                    // Shared lattice joints and ground planes stay exact.
                    // Small relief tapers to zero at those boundaries.
                    if(t<0||t>=1||depth<0||depth>=40+1.5*std::sin(t*3.141592653589793))continue;
                    const double relief=(noise(world.x,world.y,8)-.5)*6*4*t*(1-t)*std::sin(std::clamp(depth,0.,40.)*3.141592653589793/40.);
                    const auto c=material(world,depth+relief,f.direction,(t<.15&&f.start_degree==1)||(t>.85&&f.end_degree==1));
                    for(unsigned state=0;state<5;++state) {
                        if(state) {
                            const unsigned adjacency=state-1;
                            const double side=t-.5;
                            if(ramp_terrain_opening(side,depth,adjacency)) {
                                bool covered=ramp_opaque_at(world,ramp_ground,ramp,adjacency);
                                if(!covered&&(adjacency&ramp_negative_neighbor))
                                    covered=ramp_opaque_at(world,{ramp_ground.x-across_ramp.x,ramp_ground.y-across_ramp.y},neighbors[0],ramp_positive_neighbor);
                                if(!covered&&(adjacency&ramp_positive_neighbor))
                                    covered=ramp_opaque_at(world,{ramp_ground.x+across_ramp.x,ramp_ground.y+across_ramp.y},neighbors[1],ramp_negative_neighbor);
                                if(covered)continue;
                            }
                        }
                        ++counts[state];red[state]+=c.r;green[state]+=c.g;blue[state]+=c.b;
                    }
                }
                for(unsigned state=0;state<5;++state)if(counts[state])
                    rows[state][std::size_t(x+44)]={Uint8(red[state]/counts[state]),Uint8(green[state]/counts[state]),
                        Uint8(blue[state]/counts[state]),Uint8(counts[state]*255/4)};
            }
            std::array<Run,88*5> row_runs{};std::size_t row_count=0;
            for(unsigned state=0;state<5;++state)for(int x=0;x<88;) {
                const auto c=rows[state][std::size_t(x)];if(!c.a){++x;continue;}
                int end=x+1;while(end<88){const auto n=rows[state][std::size_t(end)];if(n.r!=c.r||n.g!=c.g||n.b!=c.b||n.a!=c.a)break;++end;}
                if(state_runs[state]==capacity)throw std::invalid_argument("authored terrain side state run budget exceeded");
                ++state_runs[state];row_runs[row_count++]={std::int8_t(x-44),std::int8_t(y),std::uint8_t(end-x),c.r,c.g,c.b,c.a,std::uint8_t(1U<<state)};x=end;
            }
            const auto order=[](const Run& r){return std::tuple(r.x,r.width,r.r,r.g,r.b,r.a);};
            std::sort(row_runs.begin(),row_runs.begin()+std::ptrdiff_t(row_count),[&](const Run& l,const Run& r){return order(l)<order(r);});
            for(std::size_t i=0;i<row_count;++i) {
                if(!f.runs.empty()&&f.runs.back().y==y&&order(f.runs.back())==order(row_runs[i]))f.runs.back().visibility|=row_runs[i].visibility;
                else f.runs.push_back(row_runs[i]);
            }
        }
        bytes+=f.runs.capacity()*sizeof(Run);if(bytes>max_geometry_bytes)throw std::invalid_argument("authored cliff geometry exceeds 8 MiB");
    }
    std::vector<std::pair<std::uint32_t,std::size_t>> index;index.reserve(candidate.size());
    for(std::size_t i=0;i<candidate.size();++i)index.emplace_back(*edge_key(candidate[i].high,candidate[i].low),i);
    std::sort(index.begin(),index.end());
    faces_=std::move(candidate);edge_index_=std::move(index);committed_.clear();committed_.reserve(simulation::ramp_max_links);link_count_=0;preview_.reset();preview_face_.reset();topology_.prepare({});
}
std::optional<std::size_t> AuthoredTerrainSides::edge_for(const simulation::RampLink& r) const {
    const auto key=edge_key(r.endpoint_a,r.endpoint_b);if(!key)return {};
    const auto at=std::lower_bound(edge_index_.begin(),edge_index_.end(),*key,[](const auto& a,auto b){return a.first<b;});
    if(at!=edge_index_.end()&&at->first==*key){const auto i=at->second;const auto& f=faces_[i];
        if((r.endpoint_a==f.high&&r.endpoint_b==f.low&&r.height_a==f.height&&int(r.height_b)==int(f.height)-1)||
           (r.endpoint_b==f.high&&r.endpoint_a==f.low&&r.height_b==f.height&&int(r.height_a)==int(f.height)-1))return i;
    }return {};
}
void AuthoredTerrainSides::set_ramps(std::span<const simulation::RampLink> ramps) {
    if(ramps.size()>simulation::ramp_max_links)throw std::invalid_argument("authored cliff ramp-link bound exceeded");
    std::copy(ramps.begin(),ramps.end(),links_.begin());link_count_=ramps.size();refresh_states();
}
void AuthoredTerrainSides::set_preview(const std::optional<simulation::RampLink>& ramp){preview_=ramp;refresh_states();}
void AuthoredTerrainSides::refresh_states() {
    const auto links=std::span<const simulation::RampLink>(links_.data(),link_count_);
    topology_.prepare(links,preview_,[&](const auto& r){return edge_for(r).has_value();});
    for(const auto i:committed_)faces_[i].state=0;
    if(preview_face_)faces_[*preview_face_].state=0;
    preview_face_.reset();
    committed_.clear();
    for(const auto& r:links)if(const auto i=edge_for(r)) {
        if(std::find(committed_.begin(),committed_.end(),*i)==committed_.end())committed_.push_back(*i);
        faces_[*i].state=std::uint8_t(1U+topology_.effective_mask(r.id));
    }
    if(topology_.has_preview())if(const auto i=edge_for(*preview_)) {
        preview_face_=i;faces_[*i].state=std::uint8_t(1U+topology_.preview_mask());
    }
}
bool AuthoredTerrainSides::draw(SDL_Renderer* renderer,const scene::Camera2D& camera,bool openings) const {
    if(faces_.empty())return true;
    SDL_BlendMode previous{};if(!SDL_GetRenderDrawBlendMode(renderer,&previous)||!SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND))return false;
    struct Restore{SDL_Renderer* r;SDL_BlendMode m;~Restore(){(void)SDL_SetRenderDrawBlendMode(r,m);}} restore{renderer,previous};
    int width=0,height=0;if(!SDL_GetCurrentRenderOutputSize(renderer,&width,&height))return false;
    std::array<SDL_Vertex,capacity*4> vertices;std::array<int,capacity*6> indices;
    for(std::size_t n=0;n<faces_.size();++n) {
        const auto& f=faces_[n];const auto a=camera.world_to_screen({f.ground.x-44,f.ground.y-22}),b=camera.world_to_screen({f.ground.x+44,f.ground.y+65});
        if(b.x<0||b.y<0||a.x>=width||a.y>=height)continue;
        const unsigned visibility=1U<<(openings?f.state:0U);std::size_t count=0;
        for(const auto& r:f.runs)if(r.visibility&visibility) {
            if(count==capacity)throw std::logic_error("prepared authored terrain state exceeded draw capacity");
            const SDL_FColor color{r.r/255.F,r.g/255.F,r.b/255.F,r.a/255.F};
            const std::array<Point,4> corners{{{double(r.x),double(r.y)},{double(r.x+r.width),double(r.y)},{double(r.x+r.width),double(r.y+1)},{double(r.x),double(r.y+1)}}};
            for(std::size_t j=0;j<4;++j){const auto p=camera.world_to_screen({f.ground.x+corners[j].x,f.ground.y+corners[j].y});vertices[count*4+j]={{float(std::round(p.x)),float(std::round(p.y))},color,{}};}
            constexpr int triangle[]{0,1,2,0,2,3};for(std::size_t j=0;j<6;++j)indices[count*6+j]=int(count*4)+triangle[j];++count;
        }
        if(!SDL_RenderGeometry(renderer,nullptr,vertices.data(),int(count*4),indices.data(),int(count*6)))return false;
    }return true;
}
} // namespace openemperor
