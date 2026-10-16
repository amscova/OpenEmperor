#include "original-ramp/CellTerrain.h"
#include "assets/RoadVisualProfile.h"
#include "assets/WalkerVisualProfile.h"
#include "assets/Sg3ImageLoader.h"
#include "assets/RgbaPngEncoder.h"
#include "maps/ResourceGroupLookup.h"
#include "renderer/TextureCompatibility.h"
#include "core/PerformanceDiagnostics.h"
#include <SDL3/SDL.h>
#include <nlohmann/json.hpp>
#include <zlib.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

// An excluded, standalone experiment. No SandboxView, World, RampRenderer,
// permissions, persistence, existing settings or production input path.
namespace oe=openemperor;
namespace p=oe::original_ramp;
namespace a=oe::assets;
namespace fs=std::filesystem;
using J=nlohmann::json;
namespace {
constexpr int window_width=1200,window_height=900;
void require(bool value,const std::string& message) {if(!value)throw std::runtime_error(message);}
const char* style_name(p::SurfaceStyle s) {
    switch(s){case p::SurfaceStyle::Earth:return "Earth";case p::SurfaceStyle::Stairs:return "Stairs";case p::SurfaceStyle::Cobbled:return "Cobbled";}
    return "?";
}
const char* direction_name(p::Direction d) {
    switch(d){case p::Direction::North:return "North";case p::Direction::East:return "East";case p::Direction::South:return "South";case p::Direction::West:return "West";}
    return "?";
}
p::Cell add(p::Cell c,p::Cell d,int scale=1){return {c.x+d.x*scale,c.y+d.y*scale};}
bool within(const fs::path& child,const fs::path& parent) {
    const auto relative=child.lexically_relative(parent);
    return !relative.empty()&&*relative.begin()!="..";
}
struct Camera {
    double zoom=2;
    p::Vec2 origin{600,300};
    p::Vec2 screen(p::Vec2 v) const{return {origin.x+v.x*zoom,origin.y+v.y*zoom};}
    p::Vec2 world(p::Vec2 v) const{return {(v.x-origin.x)/zoom,(v.y-origin.y)/zoom};}
};
struct Sprite {
    a::RgbaImage image;
    SDL_Texture* texture=nullptr;
    std::unique_ptr<Sprite> base,overlay;
    Sprite(a::RgbaImage rgba,SDL_Renderer* renderer):image(std::move(rgba)) {
        texture=SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGBA32,
            oe::texture_compatibility::eager_rgba_access(renderer),image.width,image.height);
        require(texture,SDL_GetError());
        if(!SDL_UpdateTexture(texture,nullptr,image.pixels.data(),int(image.width)*4)||
           !SDL_SetTextureBlendMode(texture,SDL_BLENDMODE_BLEND)||
           !SDL_SetTextureScaleMode(texture,SDL_SCALEMODE_NEAREST)) {
            const std::string error=SDL_GetError();SDL_DestroyTexture(texture);texture=nullptr;throw std::runtime_error(error);
        }
        oe::performance::increment(oe::performance::Counter::TextureUploads);
    }
    ~Sprite(){SDL_DestroyTexture(texture);}
    Sprite(const Sprite&)=delete;
    Sprite& operator=(const Sprite&)=delete;
};
struct Bank {
    SDL_Renderer* renderer;
    fs::path data;
    oe::maps::RuntimeArchiveLayout layout;
    a::RoadVisualProfile road;
    a::WalkerVisualProfile walkers;
    std::map<unsigned,std::unique_ptr<Sprite>> elevation;
    std::array<std::unique_ptr<Sprite>,8> ground;
    std::vector<std::unique_ptr<Sprite>> road_images,walker_images;
    Bank(SDL_Renderer* r,const fs::path& root):renderer(r),data(root) {
        for(const auto& name:{"China_Elevation.sg3","China_Elevation.555","China_Terrain.sg3","China_Terrain.555"}) {
            const auto path=fs::canonical(data/"DATA"/name);
            const auto relative=path.lexically_relative(data);
            require(!relative.empty()&&*relative.begin()!="..","original asset path escapes data root");
        }
        const auto archive=a::read_sg3_archive(data/"DATA/China_Elevation.sg3");
        const auto candidate=oe::maps::build_runtime_archive_layout(16,archive);
        require(candidate.has_value(),"verified v213 Elevation registration required");layout=*candidate;
        for(unsigned id=221;id<=244;++id) {
            if(id>228&&id<233)continue;
            const auto& record=archive.images.at(id);
            require(record.image_type==30&&record.width==78&&record.uncompressed_length==3200&&
                record.isometric_size_flag==1&&!record.animation_sprites&&!record.horizontal_mirror_offset&&
                !record.alpha_length&&!record.external_flag,"unsupported original ramp/cliff layout");
            a::Sg3ImageRequest request{data/"DATA/China_Elevation.sg3",id};request.split_isometric=true;
            auto loaded=a::load_sg3_image_with_source(request);
            require(loaded.base&&loaded.overlay,"original Type30 split unavailable");
            auto sprite=std::make_unique<Sprite>(std::move(loaded.rgba),renderer);
            sprite->base=std::make_unique<Sprite>(std::move(*loaded.base),renderer);
            sprite->overlay=std::make_unique<Sprite>(std::move(*loaded.overlay),renderer);
            elevation.emplace(id,std::move(sprite));
        }
        // Curated surrounding grass, not an inferred original terrain selector.
        for(unsigned i=0;i<ground.size();++i)
            ground[i]=std::make_unique<Sprite>(a::load_sg3_image({data/"DATA/China_Terrain.sg3",202+i}),renderer);
        const fs::path resources=OPENEMPEROR_PROTOTYPE_RESOURCES;
        road=a::load_road_visual_profile(data,resources/"roads.json");
        for(auto& image:road.unique_images)road_images.push_back(std::make_unique<Sprite>(std::move(image),renderer));
        walkers=a::load_walker_visual_profile(data,resources/"walkers.json");
        for(auto& image:walkers.unique_images)walker_images.push_back(std::make_unique<Sprite>(std::move(image),renderer));
        require(walkers.find(a::WalkerVisualRole::Clay),"prepared Clay walker preview required");
        for(const auto style:{p::SurfaceStyle::Earth,p::SurfaceStyle::Stairs,p::SurfaceStyle::Cobbled})
        for(const auto direction:{p::Direction::North,p::Direction::East,p::Direction::South,p::Direction::West}) {
            const auto info=p::resource_info(style,direction);
            const auto group=oe::maps::resolve_resource_group({info.resource_key},{{16,{&layout}}});
            require(group.local_base&&layout.physical_record_for_local(*group.local_base)==info.physical_id,
                    "original resource key and physical ramp identity differ");
        }
        std::size_t bytes=0;
        for(const auto& [id,sprite]:elevation){(void)id;bytes+=sprite->image.pixels.size()+sprite->base->image.pixels.size()+sprite->overlay->image.pixels.size();}
        for(const auto& sprite:ground)bytes+=sprite->image.pixels.size();
        for(const auto& sprite:road_images)bytes+=sprite->image.pixels.size();
        for(const auto& sprite:walker_images)bytes+=sprite->image.pixels.size();
        require(bytes<=64U*1024U*1024U,"combined prototype RGBA exceeds 64 MiB");
    }
};
a::RgbaImage pixels(SDL_Renderer* renderer) {
    SDL_Surface* source=SDL_RenderReadPixels(renderer,nullptr);require(source,SDL_GetError());
    SDL_Surface* rgba=SDL_ConvertSurface(source,SDL_PIXELFORMAT_RGBA32);SDL_DestroySurface(source);require(rgba,SDL_GetError());
    a::RgbaImage result;result.width=std::uint16_t(rgba->w);result.height=std::uint16_t(rgba->h);
    result.pixels.resize(std::size_t(rgba->w*rgba->h*4));
    for(int y=0;y<rgba->h;++y)std::copy_n(static_cast<const std::uint8_t*>(rgba->pixels)+y*rgba->pitch,
        rgba->w*4,result.pixels.data()+std::size_t(y*rgba->w*4));
    SDL_DestroySurface(rgba);return result;
}
void write_png(const fs::path& path,const a::RgbaImage& image) {
    // Lossless diagnostic compression; source RGBA and nearest pixels stay
    // exact. The production encoder's uncompressed DEFLATE would make this
    // multi-background matrix several GiB. No production encoder is changed.
    require(image.pixels.size()==std::size_t(image.width)*image.height*4&&image.pixels.size()<64U*1024U*1024U,"PNG dimensions exceed prototype bound");
    std::vector<std::uint8_t> scanlines;scanlines.reserve(image.pixels.size()+image.height);
    const auto stride=std::size_t(image.width)*4;
    for(unsigned y=0;y<image.height;++y){scanlines.push_back(0);scanlines.insert(scanlines.end(),image.pixels.begin()+std::ptrdiff_t(y*stride),image.pixels.begin()+std::ptrdiff_t((y+1)*stride));}
    uLongf size=compressBound(uLong(scanlines.size()));std::vector<std::uint8_t> compressed(size);
    require(compress2(compressed.data(),&size,scanlines.data(),uLong(scanlines.size()),Z_BEST_SPEED)==Z_OK,"diagnostic PNG compression failed");compressed.resize(size);
    std::vector<std::uint8_t> bytes{137,80,78,71,13,10,26,10};
    const auto append32=[](std::vector<std::uint8_t>& out,std::uint32_t value){for(int shift=24;shift>=0;shift-=8)out.push_back(std::uint8_t(value>>shift));};
    const auto chunk=[&](const char* name,const std::vector<std::uint8_t>& content) {
        append32(bytes,std::uint32_t(content.size()));const auto first=bytes.size();
        for(int i=0;i<4;++i)bytes.push_back(std::uint8_t(name[i]));
        bytes.insert(bytes.end(),content.begin(),content.end());
        append32(bytes,std::uint32_t(crc32(0,bytes.data()+first,uInt(4+content.size()))));
    };
    std::vector<std::uint8_t> header;append32(header,image.width);append32(header,image.height);header.insert(header.end(),{8,6,0,0,0});
    chunk("IHDR",header);chunk("IDAT",compressed);chunk("IEND",{});
    std::ofstream file(path,std::ios::binary);file.write(reinterpret_cast<const char*>(bytes.data()),std::streamsize(bytes.size()));require(bool(file),"PNG write failed");
}
void blit(SDL_Renderer* renderer,const Sprite& sprite,p::Vec2 origin,double zoom) {
    const SDL_FRect target{float(std::floor(origin.x)),float(std::floor(origin.y)),float(sprite.image.width*zoom),float(sprite.image.height*zoom)};
    require(SDL_RenderTexture(renderer,sprite.texture,nullptr,&target),SDL_GetError());
}
struct DrawCell {p::Cell cell;const Sprite* sprite;int physical=0;p::Vec2 origin;};
enum class Tool {Road,Ramp,Erase,Walker};
class Prototype {
public:
    SDL_Renderer* renderer;
    Bank bank;
    p::CellTerrain terrain{15,15};
    p::Walker walker;
    Camera camera;
    p::Direction higher=p::Direction::West;
    p::SurfaceStyle style=p::SurfaceStyle::Earth;
    int count=3,base=0;
    bool paused=false,loop=true,grid=false,feet=false,split_ground=false;
    Tool tool=Tool::Ramp;
    std::vector<p::Cell> ramps;
    std::vector<DrawCell> cells;
    p::Cell low_goal{},high_goal{};
    std::optional<p::Cell> hovered;
    std::optional<p::Cell> queued_goal;
    double elapsed=0;
    std::string message="Right click a road to walk there. P builds a source-cell ramp.";
    explicit Prototype(SDL_Renderer* r,const fs::path& data):renderer(r),bank(r,data){reset_scene();}
    p::Cell first_ramp(int lane) const {
        const int lateral=7-count/2+lane;
        switch(higher) {
        case p::Direction::North:return {lateral,10};case p::Direction::South:return {lateral,3};
        case p::Direction::East:return {3,lateral};case p::Direction::West:return {10,lateral};
        }return {};
    }
    void reset_scene() {
        terrain=p::CellTerrain(15,15,base);ramps.clear();
        for(int y=4;y<=9;++y)for(int x=4;x<=9;++x)require(terrain.set_plateau({x,y},base+1),"plateau preparation failed");
        const auto d=p::direction_delta(higher);
        for(int lane=0;lane<count;++lane) {
            const auto cell=first_ramp(lane);ramps.push_back(cell);
            for(int along=-3;along<=3;++along)require(terrain.set_road(add(cell,d,along)),"road preparation failed");
            require(terrain.place_ramp(cell,higher,style),"original source-cell ramp pattern rejected");
        }
        low_goal=add(ramps.front(),d,-2);high_goal=add(ramps.front(),d,2);
        require(walker.reset(terrain,low_goal)&&walker.target(terrain,high_goal),"prototype round trip unavailable");
        queued_goal.reset();elapsed=0;prepare_cells();center();
    }
    void center() {
        const auto first=ramps.front(),last=ramps.back();
        const auto aim=p::project({(first.x+last.x)*.5,(first.y+last.y)*.5,base+.5});
        camera.origin={600-aim.x*camera.zoom,485-aim.y*camera.zoom};
    }
    std::optional<unsigned> cliff_for(p::Cell cell) const {
        const auto h=terrain.at(cell).low_height;
        constexpr std::array<p::Direction,4> directions{p::Direction::North,p::Direction::East,p::Direction::South,p::Direction::West};
        constexpr std::array<unsigned,4> straight{226,228,222,224};
        for(std::size_t i=0;i<directions.size();++i) {
            const auto other=add(cell,p::direction_delta(directions[i]));
            if(terrain.contains(other)&&terrain.at(other).low_height==h+1)return straight[i];
        }
        constexpr std::array<p::Cell,4> diagonal{{{1,1},{-1,1},{-1,-1},{1,-1}}};
        constexpr std::array<unsigned,4> corner{221,223,225,227};
        for(std::size_t i=0;i<diagonal.size();++i) {
            const auto other=add(cell,diagonal[i]);
            if(terrain.contains(other)&&terrain.at(other).low_height==h+1)return corner[i];
        }
        return {};
    }
    void prepare_cells() {
        cells.clear();cells.reserve(225);
        for(int y=0;y<terrain.height();++y)for(int x=0;x<terrain.width();++x) {
            const p::Cell c{x,y};const auto& state=terrain.at(c);
            const Sprite* sprite=nullptr;int physical=0;
            if(state.higher) {
                physical=p::resource_info(state.road?state.style:p::SurfaceStyle::Earth,*state.higher).physical_id;
                sprite=bank.elevation.at(unsigned(physical)).get();
            } else if(const auto cliff=cliff_for(c)) {
                physical=int(*cliff);sprite=bank.elevation.at(*cliff).get();
            } else if(state.road) {
                unsigned mask=0;constexpr std::array<p::Cell,4> directions{{{0,-1},{1,0},{0,1},{-1,0}}};
                for(unsigned i=0;i<directions.size();++i)if(terrain.edge_connected(c,add(c,directions[i])))mask|=1U<<i;
                const auto* entry=bank.road.find(std::uint8_t(mask));require(entry,"missing road mask");
                sprite=bank.road_images.at(entry->image_index).get();
            } else sprite=bank.ground[std::size_t((x*3+y*5)%8)].get();
            const auto ground=p::project({double(x),double(y),double(state.low_height)});
            cells.push_back({c,sprite,physical,{ground.x-39,ground.y-(sprite->image.height-20)}});
        }
        std::stable_sort(cells.begin(),cells.end(),[](const auto& left,const auto& right){
            return std::pair{left.cell.x+left.cell.y,left.cell.x-left.cell.y}<std::pair{right.cell.x+right.cell.y,right.cell.x-right.cell.y};
        });
    }
    void set_case(p::Direction direction,int number,int height,p::SurfaceStyle material,double zoom) {
        higher=direction;count=number;base=height;style=material;camera.zoom=zoom;reset_scene();paused=true;
    }
    void update(double seconds) {
        if(paused)return;
        const bool was_moving=walker.moving();
        elapsed+=seconds;walker.update(terrain,seconds);
        if(was_moving&&!walker.moving()) {
            if(queued_goal) {if(!walker.target(terrain,*queued_goal))message="Destination is disconnected";queued_goal.reset();}
            else if(loop)(void)walker.target(terrain,walker.current_cell()==high_goal?low_goal:high_goal);
        }
    }
    void draw_walker() {
        const auto position=walker.position(terrain);
        const auto foot=camera.screen(p::project(position));
        const auto* role=bank.walkers.find(a::WalkerVisualRole::Clay);std::size_t frame=role->idle_frame;
        if(walker.moving()) {
            const auto from=walker.current_cell();const auto path=walker.path();
            const auto found=std::find(path.begin(),path.end(),from);
            if(found!=path.end()&&found+1!=path.end()) {
                const auto to=*(found+1);const auto direction=to.x>from.x?a::StorageDirection::PosX:to.x<from.x?a::StorageDirection::NegX:
                    to.y>from.y?a::StorageDirection::PosY:a::StorageDirection::NegY;
                const auto& clip=role->clips[a::direction_index(direction)];
                if(!clip.empty())frame=clip[std::size_t(elapsed*8)%clip.size()];
            }
        }
        const auto& selected=role->frames.at(frame);const auto& sprite=*bank.walker_images.at(selected.image_index);
        blit(renderer,sprite,{foot.x-selected.foot_x*camera.zoom,foot.y-selected.foot_y*camera.zoom},camera.zoom);
    }
    void draw_foot_marker() {
        const auto foot=camera.screen(p::project(walker.position(terrain)));
        if(feet) {
            require(SDL_SetRenderDrawColor(renderer,255,64,80,255),SDL_GetError());
            require(SDL_RenderLine(renderer,float(foot.x-5),float(foot.y),float(foot.x+5),float(foot.y))&&
                SDL_RenderLine(renderer,float(foot.x),float(foot.y-5),float(foot.x),float(foot.y+5)),SDL_GetError());
        }
    }
    void render(std::array<Uint8,3> background={22,32,42},bool hud=true,bool actor=true) {
        require(SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE)&&SDL_SetRenderDrawColor(renderer,background[0],background[1],background[2],255)&&SDL_RenderClear(renderer),SDL_GetError());
        const SDL_Rect area{0,108,window_width,window_height-156};require(SDL_SetRenderClipRect(renderer,&area),SDL_GetError());
        const auto position=walker.position(terrain);const auto owner=terrain.cell_at({position.x,position.y});
        // Explicit comparison only: a global native Base/Overlay split can
        // overpaint upper roads with neighboring cliff artwork. Keep the
        // visually reviewed per-cell whole sprites as the default experiment.
        if(split_ground)for(const auto& cell:cells)
            blit(renderer,cell.physical?*cell.sprite->base:*cell.sprite,camera.screen(cell.origin),camera.zoom);
        for(const auto& cell:cells) {
            if(!split_ground)blit(renderer,*cell.sprite,camera.screen(cell.origin),camera.zoom);
            else if(cell.physical)blit(renderer,*cell.sprite->overlay,camera.screen(cell.origin),camera.zoom);
            if(grid||hovered==cell.cell) {
                const auto& state=terrain.at(cell.cell);const auto center=camera.screen(p::project({double(cell.cell.x),double(cell.cell.y),double(state.low_height)}));
                require(SDL_SetRenderDrawColor(renderer,hovered==cell.cell?255:91,hovered==cell.cell?220:102,85,255),SDL_GetError());
                const std::array<p::Vec2,5> points{{{0,-20},{40,0},{0,20},{-40,0},{0,-20}}};
                for(std::size_t i=1;i<points.size();++i)require(SDL_RenderLine(renderer,float(center.x+points[i-1].x*camera.zoom),float(center.y+points[i-1].y*camera.zoom),
                    float(center.x+points[i].x*camera.zoom),float(center.y+points[i].y*camera.zoom)),SDL_GetError());
            }
            if(actor&&owner==cell.cell)draw_walker();
        }
        if(actor)draw_foot_marker();
        require(SDL_SetRenderClipRect(renderer,nullptr),SDL_GetError());
        if(hud)draw_hud();
    }
    void text(int x,int y,const std::string& value) {
        require(SDL_RenderDebugText(renderer,float(x),float(y),value.c_str()),SDL_GetError());
    }
    void draw_hud() {
        require(SDL_SetRenderDrawColor(renderer,12,20,28,255),SDL_GetError());
        const SDL_FRect top{0,0,float(window_width),108},bottom{0,float(window_height-48),float(window_width),48};
        require(SDL_RenderFillRect(renderer,&top)&&SDL_RenderFillRect(renderer,&bottom),SDL_GetError());
        require(SDL_SetRenderDrawColor(renderer,237,226,200,255),SDL_GetError());
        text(14,12,"ORIGINAL RAMP / ISOLATED PLAYABLE PROTOTYPE / CELL HEIGHT 40");
        text(14,30,std::string("1 Earth | 2 Stairs | 3 Cobbled    O Direction: ")+direction_name(higher)+"    +/- Width: "+std::to_string(count)+
            "    [ ] Height: "+std::to_string(base)+"    Z Zoom: "+std::to_string(int(camera.zoom))+"x");
        text(14,48,"P Ramp | B Road | X Erase | M Set walker    LEFT apply | RIGHT walk to road");
        text(14,66,"SPACE Pause | L Loop | . Step | F1 Grid | F2 Feet | F4 Painter | Arrows Pan | R Camera | ESC Quit");
        text(14,84,std::string(paused?"PAUSED":"RUNNING")+"  "+message);
        const auto foot=walker.position(terrain);
        text(14,window_height-34,"Walker surface h="+std::to_string(foot.height)+std::string(split_ground?"   Base/Overlay comparison":"   Native per-cell painter")+"   Isolated; no production save.");
        if(hovered) {
            const auto& c=terrain.at(*hovered);const auto item=std::find_if(cells.begin(),cells.end(),[&](const auto& d){return d.cell==*hovered;});
            text(14,window_height-18,"Cell "+std::to_string(hovered->x)+","+std::to_string(hovered->y)+" low="+std::to_string(c.low_height)+
                " center="+std::to_string(terrain.center_height(*hovered))+" physical Elevation="+std::to_string(item->physical));
        }
    }
    std::optional<p::Cell> pick(p::Vec2 point) const {
        if(point.y<108||point.y>=window_height-48)return {};
        const auto world=camera.world(point);
        // Cached native alpha selects even a back-facing ramp's visible bank.
        // Its bitmap anchor is separate from the analytical actor surface.
        for(const bool overlay:{true,false}) {
            for(auto it=cells.rbegin();it!=cells.rend();++it) {
                if(overlay&&(!split_ground||!it->physical))continue;
                const int x=int(std::floor(world.x-it->origin.x)),y=int(std::floor(world.y-it->origin.y));
                const auto& image=!split_ground?it->sprite->image:overlay?it->sprite->overlay->image:it->physical?it->sprite->base->image:it->sprite->image;
                if(x>=0&&y>=0&&x<image.width&&y<image.height&&image.pixels[std::size_t((y*image.width+x)*4+3)])return it->cell;
            }
        }
        return {};
    }
    void apply(p::Cell cell,bool destination) {
        if(destination) {loop=false;if(walker.moving()){queued_goal=cell;message="Destination queued until this route ends";}
            else message=walker.target(terrain,cell)?"Walking to selected road":"Destination is disconnected";
            return;}
        if(tool==Tool::Walker) {if(walker.reset(terrain,cell)){loop=false;queued_goal.reset();message="Walker placed on selected road";}else message="Walker needs a road";return;}
        if(walker.moving()&&tool==Tool::Erase){message="Use M to place the walker before erasing roads or ramps";return;}
        bool accepted=false;
        if(tool==Tool::Road)accepted=terrain.set_road(cell);
        if(tool==Tool::Erase)accepted=terrain.at(cell).higher?terrain.clear_ramp(cell):terrain.set_road(cell,false);
        if(tool==Tool::Ramp)accepted=terrain.place_ramp(cell,higher,style);
        message=accepted?"Source-cell edit applied":"Needs low cell beside exactly one higher plateau";
        if(accepted)prepare_cells();
    }
    void key(SDL_Keycode value) {
        if(value==SDLK_SPACE)paused=!paused;
        else if(value==SDLK_L){loop=!loop;if(loop&&!walker.moving())(void)walker.target(terrain,walker.current_cell()==high_goal?low_goal:high_goal);}
        else if(value==SDLK_PERIOD){const bool old=paused;paused=false;update(.05);paused=old;}
        else if(value==SDLK_F1)grid=!grid;
        else if(value==SDLK_F2)feet=!feet;
        else if(value==SDLK_F4)split_ground=!split_ground;
        else if(value==SDLK_B)tool=Tool::Road;
        else if(value==SDLK_P)tool=Tool::Ramp;
        else if(value==SDLK_X)tool=Tool::Erase;
        else if(value==SDLK_M)tool=Tool::Walker;
        else if(value==SDLK_R)center();
        else if(value==SDLK_Z){camera.zoom=camera.zoom==1?2:camera.zoom==2?4:1;center();}
        else if(value==SDLK_O){higher=p::Direction((int(higher)+1)%4);reset_scene();}
        else if(value==SDLK_1||value==SDLK_2||value==SDLK_3){style=p::SurfaceStyle(value-SDLK_1);reset_scene();}
        else if(value==SDLK_EQUALS||value==SDLK_PLUS||value==SDLK_KP_PLUS){count=std::min(4,count+1);reset_scene();}
        else if(value==SDLK_MINUS||value==SDLK_KP_MINUS){count=std::max(1,count-1);reset_scene();}
        else if(value==SDLK_LEFTBRACKET){base=std::max(-2,base-1);reset_scene();}
        else if(value==SDLK_RIGHTBRACKET){base=std::min(2,base+1);reset_scene();}
    }
    bool event(const SDL_Event& event) {
        if(event.type==SDL_EVENT_QUIT)return false;
        if(event.type==SDL_EVENT_KEY_DOWN&&!event.key.repeat) {if(event.key.key==SDLK_ESCAPE)return false;key(event.key.key);}
        else if(event.type==SDL_EVENT_MOUSE_MOTION)hovered=pick({event.motion.x,event.motion.y});
        else if(event.type==SDL_EVENT_MOUSE_BUTTON_DOWN&&(event.button.button==SDL_BUTTON_LEFT||event.button.button==SDL_BUTTON_RIGHT)) {
            if(const auto cell=pick({event.button.x,event.button.y}))apply(*cell,event.button.button==SDL_BUTTON_RIGHT);
        } else if(event.type==SDL_EVENT_MOUSE_WHEEL) {camera.zoom=event.wheel.y>0?std::min(4.,camera.zoom*2):std::max(1.,camera.zoom/2);center();}
        return true;
    }
};
J interaction_checks(Prototype& app) {
    const auto key=[&](SDL_Keycode value){SDL_Event event{};event.type=SDL_EVENT_KEY_DOWN;event.key.key=value;require(app.event(event),"unexpected prototype exit");};
    const auto click=[&](p::Cell cell,Uint8 button) {
        const auto item=std::find_if(app.cells.begin(),app.cells.end(),[&](const auto& entry){return entry.cell==cell;});
        require(item!=app.cells.end(),"interaction cell missing");
        const auto& image=item->sprite->image;
        for(int y=0;y<image.height;++y)for(int x=0;x<image.width;++x) {
            if(!image.pixels[std::size_t((y*image.width+x)*4+3)])continue;
            const auto point=app.camera.screen({item->origin.x+x+.5,item->origin.y+y+.5});
            if(app.pick(point)!=cell)continue;
            SDL_Event event{};event.type=SDL_EVENT_MOUSE_BUTTON_DOWN;event.button.x=float(point.x);event.button.y=float(point.y);event.button.button=button;
            require(app.event(event),"unexpected mouse interaction exit");return;
        }
        throw std::runtime_error("no visible native-alpha input pixel for source cell");
    };
    for(const auto value:{SDLK_1,SDLK_2,SDLK_3}) {key(value);require(int(app.style)==int(value-SDLK_1),"style input failed");}
    const auto direction=app.higher;for(int i=0;i<4;++i)key(SDLK_O);require(app.higher==direction,"orientation input cycle failed");
    const auto zoom=app.camera.zoom;for(int i=0;i<3;++i)key(SDLK_Z);require(app.camera.zoom==zoom,"zoom input cycle failed");
    app.set_case(p::Direction::West,1,0,p::SurfaceStyle::Earth,2);
    key(SDLK_M);click(app.low_goal,SDL_BUTTON_LEFT);require(!app.walker.moving(),"walker placement failed");
    const auto source=app.ramps.front();key(SDLK_X);click(source,SDL_BUTTON_LEFT);
    require(!app.terrain.at(source).higher&&!app.terrain.route(app.low_goal,app.high_goal),"erased source-cell ramp still traversable");
    key(SDLK_P);click(source,SDL_BUTTON_LEFT);require(app.terrain.at(source).higher.has_value(),"native-alpha ramp placement failed");
    key(SDLK_B);const auto road=add(app.low_goal,{0,1});click(road,SDL_BUTTON_LEFT);require(app.terrain.at(road).road,"road input failed");
    click(app.high_goal,SDL_BUTTON_RIGHT);require(app.walker.moving(),"right-click destination failed");
    app.paused=false;app.update(2);require(std::abs(app.walker.position(app.terrain).height-.5)<1e-9,"playable walker missed ramp center half height");
    app.update(2);require(app.walker.current_cell()==app.high_goal&&!app.walker.moving(),"playable walker did not reach upper road");
    return {{"result","PASS"},{"input_path","same SDL event handler as interactive window; scripted delivery"},
        {"checks",{"three native styles","four directions","three zooms","native alpha picking","source-cell erase/build","road placement","walker placement","right-click route","actual half-height passage","upper-road arrival"}}};
}
J capture(Prototype& app,const fs::path& root) {
    require(!fs::exists(root/"summary.json")&&!fs::exists(root/"scenes")&&!fs::exists(root/"stamps"),
            "capture output already contains prototype images");
    const auto local=fs::canonical(".local");
    require(root!=local&&within(root,local)&&!within(root,app.bank.data)&&!within(app.bank.data,root),
            "capture output must be beneath .local and separate from original data");
    fs::create_directories(root/"scenes");fs::create_directory(root/"stamps");
    J report={{"format","openemperor-original-ramp-prototype-v1"},{"cases",J::array()},{"sprite_stamps",J::array()},
        {"coverage",J::array()},{"feet",J::array()},
        {"scope","isolated native source-cell prototype; continuous analytical height supported by original actor offset trace; no production adoption"}};
    for(unsigned id=233;id<=244;++id)for(const int zoom:{1,2,4}) {
        require(SDL_SetRenderDrawColor(app.renderer,22,32,42,255)&&SDL_RenderClear(app.renderer),SDL_GetError());
        const p::Vec2 origin{400,220};blit(app.renderer,*app.bank.elevation.at(id),origin,zoom);
        const auto file="stamps/"+std::to_string(id)+"-z"+std::to_string(zoom)+".png";
        write_png(root/file,pixels(app.renderer));report["sprite_stamps"].push_back({{"physical_id",id},{"file",file},
            {"origin",{origin.x,origin.y}},{"zoom",zoom},{"background",{22,32,42,255}}});
    }
    for(const bool axis:{true,false})for(const int delta:{-1,1})for(const int base:{-2,0,2})for(int count=1;count<=4;++count)
    for(const auto style:{p::SurfaceStyle::Earth,p::SurfaceStyle::Stairs,p::SurfaceStyle::Cobbled})for(const int zoom:{1,2,4}) {
        const auto direction=axis?(delta>0?p::Direction::East:p::Direction::West):(delta>0?p::Direction::South:p::Direction::North);
        app.set_case(direction,count,base,style,zoom);
        const std::string name=std::string(axis?"X":"Y")+"-d"+std::to_string(delta)+"-h"+std::to_string(base)+"-n"+std::to_string(count)+"-"+style_name(style)+"-z"+std::to_string(zoom);
        const auto file="scenes/"+name+".png",second="scenes/"+name+"-alternate.png";
        app.render({22,32,42},true,false);write_png(root/file,pixels(app.renderer));
        app.render({181,35,93},true,false);write_png(root/second,pixels(app.renderer));
        J entry={{"axis",axis?"X":"Y"},{"delta",delta},{"base_height",base},{"count",count},{"style",style_name(style)},
            {"zoom",zoom},{"height_step",40},{"file",file},{"camera",{{"origin",{app.camera.origin.x,app.camera.origin.y}}}}};
        const auto d=p::direction_delta(direction);const p::Cell lateral=axis?p::Cell{0,1}:p::Cell{1,0};
        for(const auto cell:app.ramps)for(const double s:{-.5,-.25,0.,.25,.5})for(const double side:{0.}) {
            const p::Vec3 world{cell.x+d.x*s+lateral.x*side,cell.y+d.y*s+lateral.y*side,base+.5+s};
            const auto screen=app.camera.screen(p::project(world));
            report["coverage"].push_back({{"file",file},{"second_file",second},{"source_cell",{cell.x,cell.y}},
                {"local_progress",s},{"lateral_offset",side},{"xy",{int(std::floor(screen.x)),int(std::floor(screen.y))}},{"required_opaque",true}});
        }
        report["cases"].push_back(entry);
        // The actual same walker renderer, using a deterministic movement
        // state rather than a sprite marker injected at an arbitrary height.
        if(base==0&&count==3) {
            app.walker.update(app.terrain,2.0);app.elapsed=2;app.feet=false;app.render();
            write_png(root/("scenes/"+name+"-walker-unmarked.png"),pixels(app.renderer));
            app.feet=true;app.render();
            const auto walker_file="scenes/"+name+"-walker.png";write_png(root/walker_file,pixels(app.renderer));
            const auto position=app.walker.position(app.terrain);
            report["feet"].push_back({{"file",walker_file},{"source_cell",{app.ramps.front().x,app.ramps.front().y}},
                {"position",{position.x,position.y,position.height}},{"base_height",base},{"higher",{d.x,d.y}},
                {"camera",{{"origin",{app.camera.origin.x,app.camera.origin.y}},{"zoom",zoom}}}});
            app.feet=false;
            if(zoom==2) {
                app.split_ground=true;app.render();
                const auto alternate="scenes/"+name+"-base-overlay.png";write_png(root/alternate,pixels(app.renderer));app.split_ground=false;
                report["composition_comparisons"].push_back({{"native_per_cell","scenes/"+name+"-walker-unmarked.png"},{"base_overlay",alternate},
                    {"assessment","global split is diagnostic only; upper-road overpainting observed"}});
            }
        }
    }
    report["interaction_checks"]=interaction_checks(app);
    app.set_case(p::Direction::West,3,0,p::SurfaceStyle::Earth,2);app.feet=true;
    oe::performance::reset();oe::performance::set_enabled(true);
    for(int frame=0;frame<100;++frame)app.render();
    oe::performance::set_enabled(false);
    J counters=J::object();
    for(const auto counter:{oe::performance::Counter::AssetDecodes,oe::performance::Counter::TextureUploads,oe::performance::Counter::FileReads,
                           oe::performance::Counter::FileWrites,oe::performance::Counter::WorldCopies,oe::performance::Counter::WorldExecutes,
                           oe::performance::Counter::SimulationTicks,oe::performance::Counter::BfsCalls,oe::performance::Counter::RouteRefreshes})
        counters[std::string(oe::performance::counter_names[std::size_t(counter)])]=oe::performance::counter(counter);
    report["pure_frames"]=100;report["counters"]=counters;
    std::ofstream(root/"summary.json")<<report.dump(2)<<'\n';return report;
}
}
int main(int argc,char** argv) {
    SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;
    try {
        require(argc==2||(argc==4&&std::string(argv[2])=="--capture"),"Usage: openemperor-original-ramp-prototype DATA [--capture FRESH-.local-DIRECTORY]");
        const auto data=fs::canonical(argv[1]);const bool captures=argc==4;
        if(captures)SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy");
        require(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
        window=SDL_CreateWindow("OpenEmperor - Original Ramp Cell Prototype",window_width,window_height,captures?SDL_WINDOW_HIDDEN:0);require(window,SDL_GetError());
        renderer=SDL_CreateRenderer(window,captures?"software":nullptr);require(renderer,SDL_GetError());
        {
            Prototype app(renderer,data);
            if(captures) {
                const auto root=fs::weakly_canonical(argv[3]);const auto result=capture(app,root);
                std::cout<<"Native original source-cell prototype: "<<result["cases"].size()<<" scenes, "<<result["sprite_stamps"].size()<<" unchanged stamps\n";
            } else {
                bool running=true;auto previous=SDL_GetTicksNS();
                while(running) {
                    SDL_Event event;
                    while(SDL_PollEvent(&event)) {
                        running=app.event(event)&&running;
                    }
                    const auto now=SDL_GetTicksNS();const double seconds=std::min(.05,double(now-previous)/1e9);previous=now;
                    const auto* keys=SDL_GetKeyboardState(nullptr);
                    app.camera.origin.x+=(int(keys[SDL_SCANCODE_LEFT])-int(keys[SDL_SCANCODE_RIGHT]))*seconds*280;
                    app.camera.origin.y+=(int(keys[SDL_SCANCODE_UP])-int(keys[SDL_SCANCODE_DOWN]))*seconds*280;
                    app.update(seconds);app.render();require(SDL_RenderPresent(renderer),SDL_GetError());SDL_Delay(8);
                }
            }
        }
        SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();return 0;
    } catch(const std::exception& error) {
        std::cerr<<error.what()<<'\n';SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();return 1;
    }
}
