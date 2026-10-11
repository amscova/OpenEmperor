#include "assets/AssetCatalog.h"
#include "core/PerformanceDiagnostics.h"
#include "maps/LandscapeProvenance.h"
#include "maps/OrdinaryTowerMapPresentation.h"
#include "maps/RegeneratedMapRenderPlan.h"
#include "renderer/StoredGraphicsRenderer.h"
#include "scene/WorldDrawOrder.h"

#include <SDL3/SDL.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>

namespace {
namespace maps = openemperor::maps;
namespace scene = openemperor::scene;
namespace perf = openemperor::performance;
using Bytes = std::vector<std::uint8_t>;
using Color = std::array<std::uint8_t,4>;
constexpr std::size_t count = 228U * 228U;
constexpr maps::GridCell origin{100,110}, marker{100,111};
constexpr Color black{0,0,0,255}, red{255,0,0,255}, blue{0,0,255,255}, green{0,255,0,255};
void check(bool value,const char* message) {if (!value) throw std::runtime_error(message);}
std::size_t raw(maps::GridCell c) {return std::size_t(c.y)*228+c.x;}
void u16(Bytes& b,std::size_t p,unsigned n) {b.at(p)=std::uint8_t(n);b.at(p+1)=std::uint8_t(n>>8);}
void u32(Bytes& b,std::size_t p,unsigned n) {u16(b,p,n);u16(b,p+2,n>>16);}
void write(const std::filesystem::path& p,const Bytes& b) {
    std::ofstream f(p,std::ios::binary);f.write(reinterpret_cast<const char*>(b.data()),std::streamsize(b.size()));
    check(bool(f),"write authored Tower image fixture");
}
Bytes overlay(unsigned width,unsigned height,unsigned color,bool square) {
    // Authored pixels: a solid upper body with one transparent window, or an
    // independent green foreground/background item. No proprietary pixels.
    const auto opaque=[&](unsigned at) {
        const auto x=at%width,y=at/width;
        return square ? x>=45 && x<114 && y>=10 && y<100 && !(x>=100 && x<107 && y>=49 && y<57)
                      : y<100;
    };
    Bytes bytes;
    for (unsigned at=0;at<width*height;) {
        const auto solid=opaque(at);unsigned n=1;
        while (n<254 && at+n<width*height && opaque(at+n)==solid) ++n;
        if (!solid) {bytes.push_back(255);bytes.push_back(std::uint8_t(n));}
        else {bytes.push_back(std::uint8_t(n));for (unsigned p=0;p<n;++p) {
            bytes.push_back(std::uint8_t(color));bytes.push_back(std::uint8_t(color>>8));}}
        at+=n;
    }
    return bytes;
}
struct Temp {
    std::filesystem::path root=std::filesystem::temp_directory_path()/
        ("openemperor-tower-joins-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    explicit Temp(bool malformed_body=false) {
        std::filesystem::create_directories(root/"DATA");
        Bytes sg3(40680+12*72,0),bitmap(4,0);
        u32(sg3,0,unsigned(sg3.size()));u32(sg3,4,214);u32(sg3,12,12);u32(sg3,16,12);u32(sg3,20,1);
        const std::string name="authored.bmp";std::copy(name.begin(),name.end(),sg3.begin()+680);u32(sg3,804,4);
        for (unsigned i=1;i<=3;++i) {
            const unsigned side=i==3 ? 1:2,width=side*80-2,height=i==3 ? 120:180;
            Bytes image(3200*side*side);for (std::size_t p=0;p<image.size();p+=2) u16(image,p,i==3 ? 0x03e0:0x7c00);
            const auto upper=i==1 && malformed_body ? Bytes{255}:overlay(width,height,
                i==1 ? 0x001f:i==2 ? 0x7c00:0x03e0,i!=3);
            image.insert(image.end(),upper.begin(),upper.end());const auto p=40680+i*72;
            u32(sg3,p,unsigned(bitmap.size()));u32(sg3,p+4,unsigned(image.size()));u32(sg3,p+8,3200*side*side);
            u16(sg3,p+20,width);u16(sg3,p+22,height);u16(sg3,p+50,30);sg3[p+55]=std::uint8_t(side);
            bitmap.insert(bitmap.end(),image.begin(),image.end());
        }
        for(unsigned i=4;i<12;++i) {
            const unsigned width=i==4?40:12,height=10;Bytes image(width*height*2);
            // Independent asymmetry: a green rectangle, a transparent hole and a blue right edge.
            for(unsigned y=0;y<height;++y)for(unsigned x=0;x<width;++x)
                u16(image,(y*width+x)*2,(x>=2&&x<=4&&y>=3&&y<=5)?0xf81f:x>=width-3?0x001f:0x03e0);
            const auto p=40680+i*72;
            if(i==9) {
                image.clear();for(unsigned y=0;y<height;++y)for(unsigned x=0;x<width;++x) {
                    if(x>=2&&x<=4&&y>=3&&y<=5) {image.push_back(255);image.push_back(1);}
                    else {image.push_back(1);const unsigned c=x>=width-3?0x001f:0x03e0;
                        image.push_back(std::uint8_t(c));image.push_back(std::uint8_t(c>>8));}
                }
            }
            u32(sg3,p,unsigned(bitmap.size()));u32(sg3,p+4,unsigned(image.size()));
            u32(sg3,p+8,unsigned(image.size()));u16(sg3,p+20,width);u16(sg3,p+22,height);u16(sg3,p+50,i==9?256:0);
            bitmap.insert(bitmap.end(),image.begin(),image.end());
        }
        write(root/"DATA/authored.sg3",sg3);write(root/"DATA/authored.555",bitmap);
    }
    ~Temp() {std::error_code e;std::filesystem::remove_all(root,e);}
};
maps::StoredArchiveRegistrations registrations(const Temp& t) {
    maps::StoredArchiveRegistration r;r.slot=2;r.relative_path="DATA/authored.sg3";
    r.catalog=openemperor::assets::scan_asset_archive(t.root,r.relative_path);
    check(r.catalog->records.size()==12 && r.catalog->archive_errors.empty(),"authored side-two Type30 metadata");
    r.layout.emplace();auto& l=*r.layout;l.slot=2;l.sg3_version=213;l.verified_registration=true;
    l.first_physical_record=1;l.image_capacity=12;l.runtime_image_count=11;l.groups.resize(203);
    for (unsigned n=0;n<203;++n) l.groups[n]={n,80+2*n,0,0};
    l.groups[76].local_base=1; l.groups[200].local_base=3;l.groups[201].local_base=9;l.groups[202].local_base=11;
    return {{2,std::move(r)}};
}
maps::OriginalEntityRecord source(maps::GridCell start,unsigned index,unsigned id) {
    maps::OriginalEntityRecord e;e.manager_index=index;e.entity_class=maps::OriginalEntityClass::Tower;
    e.status=3;e.footprint_side=2;e.type=131;e.local_x=std::int16_t(start.x-29);e.local_y=std::int16_t(start.y-29);
    e.serialized_cell_reference=std::int32_t(raw(start));e.serialized_original_id=std::int32_t(id);
    e.original_id=maps::OriginalEntityId{id};e.provenance.logical_record_offset=9984;e.provenance.logical_base_offset=10000;
    e.provenance.base_schema=5;e.provenance.wrapper_schema=1;e.provenance.logical_extended_offset=10184;
    e.provenance.extended_schema=1;e.provenance.logical_class_wrapper_offset=10312;e.provenance.class_wrapper_schema=0;
    return e;
}
// Independent fixture constants. These do not call terrain projection,
// regenerated_instance_origin, stored_square_image_origin or painter helpers.
scene::Point body_top(maps::GridCell cell=origin,int height=0) {
    return {(int(cell.x)-int(cell.y))*40.-79.,(int(cell.x)+int(cell.y)-58)*20.-100.-40.*height};
}
scene::Point sample(unsigned x,unsigned y,int height=0,maps::GridCell start=origin) {
    const auto p=body_top(start,height);return {p.x+x+.5,p.y+y+.5};
}
void append_historical_square(maps::StoredGraphicsPlan& p,maps::GridCell start,std::size_t asset,
    unsigned side,unsigned image_height,scene::Point image_origin) {
    maps::PlacedFootprint f;f.id=p.footprints.size();f.asset_index=asset;f.origin=start;
    f.width_cells=f.height_cells=side;f.image_origin=image_origin;f.status=maps::StoredStatus::DecodePending;
    f.draw_cell_candidate={start.x,start.y+side-1};
    for (unsigned dy=0;dy<side;++dy) for (unsigned dx=0;dx<side;++dx) {
        const maps::GridCell cell{start.x+dx,start.y+dy};const auto at=raw(cell);
        maps::StoredCell c;c.storage=cell;c.cell_index=at;c.terrain_raw=p.raw_terrain[at];c.objects_raw=p.raw_objects[at];
        c.stored_id=p.raw_saved_ids[at];c.slot=2;c.local_index=77;c.asset_index=asset;c.footprint_index=f.id;
        c.status=maps::StoredStatus::DecodePending;c.world={(int(cell.x)-int(cell.y))*40.,(int(cell.x)+int(cell.y)-58)*20.};
        c.image_origin={(int(start.x)-int(start.y))*40.-double(side*40-1),
            (int(start.x)+int(start.y)-58)*20.+double(side*40)-double(image_height)};
        p.cell_by_storage[at]=p.cells.size();p.status_by_storage[at]=c.status;f.cell_indices.push_back(p.cells.size());p.cells.push_back(c);
    }
    p.footprints.push_back(std::move(f));
}
maps::StoredGraphicsPlan historical(const Temp& t,const maps::StoredArchiveRegistrations& r,bool two=false,bool foreign=false) {
    maps::StoredGraphicsPlan p;p.data_root=t.root;p.border=29;p.landscape_layers_available=true;
    p.raw_terrain.assign(count,0);p.raw_objects.assign(count,0);p.raw_saved_ids.assign(count,0xffffffff);
    p.raw_candidate_bytes.assign(count,255);p.draw_properties.assign(count,0);p.height_bytes.assign(count,0);
    p.variation_bytes.assign(count,213);p.fertility_bytes.assign(count,97);p.cell_by_storage.resize(count);
    p.status_by_storage.assign(count,maps::StoredStatus::Excluded);
    p.assets.push_back({r.at(2).catalog->records[2],maps::StoredStatus::DecodePending,false,false,{}});
    maps::OriginalMapEntities entities;entities.manager_schema=1;entities.declared_map_size=170;
    const std::vector<maps::GridCell> starts=two ? std::vector<maps::GridCell>{origin,{120,130}}:std::vector<maps::GridCell>{origin};
    for (const auto start:starts) {
        entities.records.push_back(source(start,11+unsigned(entities.records.size()),7+unsigned(entities.records.size())));
        for (unsigned dy=0;dy<2;++dy) for (unsigned dx=0;dx<2;++dx) p.raw_terrain[raw({start.x+dx,start.y+dy})]=0x8088;
        append_historical_square(p,start,0,2,180,body_top(start));
    }
    if (foreign) {
        p.assets.push_back({r.at(2).catalog->records[3],maps::StoredStatus::DecodePending,false,false,{}});
        const maps::GridCell cell{99,110};p.raw_terrain[raw(cell)]=0x100;
        const auto top=body_top();append_historical_square(p,cell,1,1,120,{top.x+41,top.y+10});
    }
    p.original_ordinary_towers=std::make_shared<maps::OrdinaryTowerMapPresentation>(
        maps::prepare_ordinary_tower_presentation(entities,p.raw_terrain,p.raw_objects,p.height_bytes));
    for(const auto start:starts) {
        p.raw_terrain[raw({start.x+2,start.y+1})]=0x4000;
        p.raw_terrain[raw({start.x+1,start.y+2})]=0x4000;
    }
    p.original_ordinary_towers=std::make_shared<maps::OrdinaryTowerMapPresentation>(
        maps::prepare_ordinary_tower_presentation(entities,p.raw_terrain,p.raw_objects,p.height_bytes));
    return p;
}
void clear(SDL_Renderer* r) {check(SDL_SetRenderDrawColor(r,0,0,0,255) && SDL_RenderClear(r),"clear authored pixels");}
void frame(openemperor::StoredGraphicsRenderer& v,SDL_Renderer* r,const scene::Camera2D& c) {
    clear(r);check(v.render(c,std::nullopt),"normal production renderer draws complete Tower body");
}
Color pixel(SDL_Renderer* r,scene::Point p) {
    auto* surface=SDL_RenderReadPixels(r,nullptr);check(surface,"test-only authored pixel readback");Color c{};
    const auto ok=SDL_ReadSurfacePixel(surface,int(p.x),int(p.y),&c[0],&c[1],&c[2],&c[3]);SDL_DestroySurface(surface);
    check(ok,"authored sample is within real renderer output");return c;
}
scene::Camera2D camera(double zoom=1) {
    scene::Camera2D c;c.viewport_width=1200;c.viewport_height=900;c.zoom=zoom;c.center_on(sample(79,75));return c;
}
// Original side-two rectangle property 1 adjusts the body caller by (40,-20),
// then its Overlay primitive by (-40,80-H). Callbacks use the original caller.
// Independent inversion of the complete body relation: (0,H-60).
scene::Point join_sample(unsigned variant,unsigned x=5,unsigned y=5,int elevation=0) {
    constexpr std::array<int,8> dx{127,1,1,114,75,39,28,79};
    constexpr std::array<int,8> dy{-60,-57,-46,-48,-28,-24,-71,-70};
    const auto p=body_top(origin,elevation);return {p.x+dx[variant]+x+.5,p.y+120+dy[variant]+y+.5};
}
std::array<std::optional<maps::TowerJoinNeighbor>,6> facts(const maps::OrdinaryTowerPresentation& tower) {
    std::array<std::optional<maps::TowerJoinNeighbor>,6> f;
    for(unsigned n=0;n<6;++n) f[n]=tower.joins.neighbors[n];return f;
}
void pure_selection() {
    Temp t;const auto reg=registrations(t);const auto p=historical(t,reg);const auto tower=p.original_ordinary_towers->towers[0];
    auto f=facts(tower);for(auto&c:f){c->terrain=0;c->objects=0;c->owner={};}
    const auto none=maps::select_ordinary_tower_joins(tower,f);
    check(none.status==maps::TowerJoinStatus::NoJoinRequired&&none.components.empty(),"isolated tower gets no invented connection or cap-only decoration");
    // Every single proved neighboring side, independently recorded expected variant sequence.
    constexpr std::array<const char*,6> expected{"1,2,3,4,5,","0,2,3,4,5,","0,1,6,3,4,5,","0,1,7,2,4,5,","0,1,7,2,3,5,","0,1,6,2,3,4,"};
    for(unsigned n=0;n<6;++n) {
        auto one=f;one[n]->terrain=0x4000;const auto s=maps::select_ordinary_tower_joins(tower,one);std::string signature;
        for(const auto&c:s.components){signature+=std::to_string(c.group.value==0x4ca?6+c.variant:c.variant)+",";
            check(c.owner.value==7&&c.manager_index==11&&c.source_cell==origin&&c.height_cell==marker,"components retain stable source and marker height without neighbor claims");}
        check(s.status==maps::TowerJoinStatus::SupportedJoin&&signature==expected[n],"independent single-side resource/callback sequence");
        for(const auto kind:{maps::TowerJoinOwnerKind::Tower,maps::TowerJoinOwnerKind::GateHouse}) {
            one[n]->terrain=0;one[n]->owner={kind,maps::OriginalEntityId{23},31};
            const auto owner=maps::select_ordinary_tower_joins(tower,one);std::string typed;
            for(const auto&c:owner.components)typed+=std::to_string(c.group.value==0x4ca?6+c.variant:c.variant)+",";
            check(typed==signature,"validated typed Tower/Gate ownership independently supplies the same presence predicate");
        }
    }
    auto bad=f;bad[0]->terrain=0x8000;
    check(maps::select_ordinary_tower_joins(tower,bad).status==maps::TowerJoinStatus::UnsupportedContext,"gate bit without original owner is not typed ownership");
    bad[0]->terrain=0;bad[0]->owner={maps::TowerJoinOwnerKind::Other,maps::OriginalEntityId{12},12};
    check(maps::select_ordinary_tower_joins(tower,bad).status==maps::TowerJoinStatus::NoJoinRequired,"wrong owner type cannot invent a connection");
    bad[0]->owner.kind=maps::TowerJoinOwnerKind::Unresolved;
    check(maps::select_ordinary_tower_joins(tower,bad).status==maps::TowerJoinStatus::UnsupportedContext,"ambiguous/incomplete typed ownership fails closed");
    bad=f;bad[0]->owner.kind=static_cast<maps::TowerJoinOwnerKind>(999);
    check(maps::select_ordinary_tower_joins(tower,bad).status==maps::TowerJoinStatus::InvalidInput,"invalid typed owner enum");
    bad=f;bad[3].reset();check(maps::select_ordinary_tower_joins(tower,bad).status==maps::TowerJoinStatus::InvalidInput,"missing neighbor input");
    bad=f;bad[0]->cell={0,0};check(maps::select_ordinary_tower_joins(tower,bad).status==maps::TowerJoinStatus::InvalidInput,"wrong neighbor coordinate");
    auto inactive=tower;inactive.source.status=0;
    check(maps::select_ordinary_tower_joins(inactive,f).status==maps::TowerJoinStatus::UnsupportedContext,"inactive Tower cannot produce joins");
    check(maps::select_ordinary_tower_joins(tower,f,1).status==maps::TowerJoinStatus::UnsupportedContext&&
        maps::select_ordinary_tower_joins(tower,f,0,false).status==maps::TowerJoinStatus::UnsupportedContext,"unknown view and placement preview remain unsupported");
    inactive=tower;inactive.origin={0,0};inactive.source.serialized_cell_reference=0;
    check(maps::select_ordinary_tower_joins(inactive,f).status==maps::TowerJoinStatus::InvalidInput,"edge neighbors never wrap");
    auto changed=p;changed.raw_saved_ids.assign(count,1234);changed.raw_candidate_bytes.assign(count,0);
    maps::build_regenerated_map_render_plan(changed,reg);check(changed.regenerated->tower_joins.size()==6,"saved graphics and marker bytes do not select joins");
}
void positive_join(SDL_Renderer*r) {
    Temp t;const auto reg=registrations(t);auto p=historical(t,reg);const auto raw_before=p.raw_terrain,objects=p.raw_objects;
    maps::build_regenerated_map_render_plan(p,reg);
    check(p.regenerated->instances.size()==1&&p.regenerated->tower_joins.size()==6&&p.footprints.size()==1&&
        p.regenerated->cells.size()==4&&p.raw_terrain==raw_before&&p.raw_objects==objects,"joins add six attachments, zero exclusive ground or historical replacement claims");
    openemperor::StoredGraphicsRenderer v(std::move(p));v.initialize(r);v.set_landscape_mode(openemperor::LandscapeDebugMode::Regenerated);v.set_debug_diagnostics(false);
    check(v.upload_count()==12&&v.plan().decoded_assets==8&&v.plan().logical_texture_bytes==2U*158U*180U*4U*3U+(40U+5U*12U)*10U*4U,
        "one texture per plain/Omega join, shared deduplicated pool; complete body still uses three");
    // The first attachment index collides with a historical cell index. A
    // road replacement mask must never suppress that unrelated spatial image.
    {
        std::vector<std::uint8_t> replacements(count,0);replacements[raw(origin)]=1;
        const auto c=camera();clear(r);v.begin_frame();
        for(std::size_t i=0;i<v.draw_items().size();++i) {
            if(v.draw_items()[i].tower_join)
                check(v.ground_replacement_role(i,replacements,228)==0,
                    "Tower attachment index must not alias a historical road-ground cell");
            if(v.ground_replacement_role(i,replacements,228)==0)
                check(v.draw_item(i,c),"normal Sandbox road-replacement spatial submission");
        }
        check(pixel(r,c.world_to_screen(join_sample(0)))==green,
            "road replacement preserves the unrelated exterior Tower attachment");
    }
    for(const int height:{0,1,-2,-128,127}) {
        if(height!=0)continue; // Separate load-time height sessions follow below.
        for(const double zoom:{1.,1.15,1.5,2.,4.,1.}) {
            const auto c=camera(zoom);frame(v,r,c);
            check(pixel(r,c.world_to_screen(join_sample(6)))==green,"missing external Tower wall join: independent key4ca variant0 pixel must be green in production");
            check(pixel(r,c.world_to_screen(join_sample(0)))==green&&pixel(r,c.world_to_screen(join_sample(2)))==green,"asymmetric outer cap and late front component appear once");
            check(pixel(r,c.world_to_screen(join_sample(6,9)))==blue,"unmirrored asymmetric right edge");
            check(pixel(r,c.world_to_screen(join_sample(7,3,4)))==blue,"transparent join hole admits the body underneath");
            check(pixel(r,c.world_to_screen(join_sample(4)))==blue,"present neighbor suppresses its absence-branch fragment");
            check(v.hit_test(c.world_to_screen(join_sample(0)),c)==marker,"outside-footprint join alpha selects the original Tower owner");
            const auto hit=v.hit_test_item(c.world_to_screen(join_sample(2)),c);
            check(hit&&hit->key.layer==scene::WorldVisualLayer::StoredTowerAfterActors,"draw and picking share late component phase");
            check(v.last_texture_draws()==7,"one body and six required attachment submissions, no duplicates");
        }
    }
    auto c=camera();for(unsigned variant:{6U,2U})for(unsigned role:{2U,3U})for(int depth:{3080,3100,3120}) {
        struct D{scene::WorldDrawKey key;};const std::array<D,1> dynamic{{{{double(depth),-400.,static_cast<scene::WorldVisualLayer>(role),99}}}};
        clear(r);v.begin_frame();for(std::size_t n=0;n<v.draw_items().size();++n)check(v.draw_ground_item(n,c),"ground before unified spatial scene");
        const auto pnt=c.world_to_screen(join_sample(variant));scene::WorldMergeStats merge;
        check(scene::merge_world_draw_streams(v.draw_items(),dynamic,[&](std::size_t n){return v.draw_item(n,c);},[&](std::size_t){
            const SDL_FRect rect{float(std::floor(pnt.x)),float(std::floor(pnt.y)),2,2};return SDL_SetRenderDrawColor(r,255,0,0,255)&&SDL_RenderFillRect(r,&rect);},merge),"existing linear merge");
        const auto expected=depth>3100||(depth==3100&&variant==6)?red:green;
        check(pixel(r,pnt)==expected,"independent House/Walker front/back and equal-ground callback phase oracle");
    }
    c.offset={2.5-join_sample(0,33).x,450.5-join_sample(0,33).y};frame(v,r,c);
    check(pixel(r,{2.5,450.5})==green&&v.hit_test({2.5,450.5},c)==marker,"visible authored attachment beyond full body bounds survives body culling");
    c=camera();for(const auto mode:{openemperor::LandscapeDebugMode::Ground,openemperor::LandscapeDebugMode::MountainsRocks,openemperor::LandscapeDebugMode::Snapshot}) {
        v.set_landscape_mode(mode);frame(v,r,c);check(pixel(r,c.world_to_screen(join_sample(0)))==black&&!v.hit_test(c.world_to_screen(join_sample(0)),c),"F8 and historical Snapshot hide every exterior attachment");
    }
    v.set_landscape_mode(openemperor::LandscapeDebugMode::WallsMonuments);frame(v,r,c);check(pixel(r,c.world_to_screen(join_sample(6)))==green,"F8 wall layer restores complete join assembly");
    const auto detail=maps::landscape_provenance(v.plan(),origin,true).at("ordinary_tower").at("wall_joins");
    check(detail.at("status")=="active"&&detail.at("active_components")==6&&detail.at("components")[0].at("owner")==7,"cached source and complete activation provenance");
    perf::set_enabled(true);perf::reset();for(unsigned n=0;n<20;++n){frame(v,r,c);(void)v.hit_test(c.world_to_screen(join_sample(0)),c);(void)maps::landscape_inspection_lines(v.plan(),origin,true);}
    for(std::size_t n=0;n<std::size_t(perf::Counter::Count);++n)check(perf::counter(static_cast<perf::Counter>(n))==0,"join frame/inspect/pick performs no IO, upload, World or navigation work");
    perf::set_enabled(false);v.shutdown();
}
void failures_height_reuse(SDL_Renderer*r) {
    Temp t;auto reg=registrations(t);
    for(const int height:{-128,-2,1,127}) {
        auto p=historical(t,reg,true);std::fill(p.height_bytes.begin(),p.height_bytes.end(),std::uint8_t(height));
        // Reprepare the source facts at this independently authored height.
        maps::build_regenerated_map_render_plan(p,reg);openemperor::StoredGraphicsRenderer v(std::move(p));v.initialize(r);v.set_debug_diagnostics(false);v.set_landscape_mode(openemperor::LandscapeDebugMode::Regenerated);
        check(v.plan().regenerated->tower_joins.size()==12,"two Towers share all six selected join assets, with twelve owner attachments");
        auto c=camera();c.center_on(join_sample(0,5,5,height));frame(v,r,c);
        check(pixel(r,c.world_to_screen(join_sample(0,5,5,height)))==green&&v.upload_count()==12,"signed height exactly once and repeated complete join/body physical deduplication");v.shutdown();
    }
    for(unsigned failure=0;failure<6;++failure) {
        auto p=historical(t,reg);auto rcopy=reg;
        if(failure==0)rcopy.at(2).layout->groups.resize(200);
        if(failure==1)rcopy.at(2).catalog->records[10].horizontal_mirror_offset=1;
        if(failure==2)rcopy.at(2).catalog->records[10].width=32767;
        if(failure==3)rcopy.at(2).layout->groups[201].local_base=10; // Only one ca variant: no out-of-group spill.
        if(failure==4)p.logical_texture_bytes=maps::stored_max_texture_bytes-2U*158U*180U*4U*3U;
        if(failure==5)rcopy.at(2).catalog->records[10].data_length=maps::stored_max_payload_bytes+1;
        maps::build_regenerated_map_render_plan(p,rcopy);
        check(p.regenerated->instances.size()==1&&p.regenerated->tower_joins.empty()&&!p.regenerated->tower_join_preparation.at(0).fallback.empty(),"optional join refusal publishes no partial attachments or assets, full body remains prepared");
        if(failure==4)continue; // Reserved-budget accounting does not create a fabricated texture pool.
        openemperor::StoredGraphicsRenderer v(std::move(p));v.initialize(r);v.set_landscape_mode(openemperor::LandscapeDebugMode::Regenerated);auto c=camera();frame(v,r,c);
        check(pixel(r,c.world_to_screen(sample(60,30)))==blue&&v.last_texture_draws()==1,"metadata failure retains entire current Tower body");v.shutdown();
    }
    {
        auto p=historical(t,reg);maps::build_regenerated_map_render_plan(p,reg);
        std::fstream f(t.root/"DATA/authored.sg3",std::ios::binary|std::ios::in|std::ios::out);f.seekp(40680+10*72+4);const char b[4]={1,0,0,0};f.write(b,4);f.close();
        openemperor::StoredGraphicsRenderer v(std::move(p));v.initialize(r);v.set_landscape_mode(openemperor::LandscapeDebugMode::Regenerated);auto c=camera();frame(v,r,c);
        check(std::none_of(v.plan().tower_join_active.begin(),v.plan().tower_join_active.end(),[](auto n){return n!=0;})&&
            pixel(r,c.world_to_screen(sample(60,30)))==blue&&v.last_texture_draws()==1,"one selected decode failure rolls back complete optional group and preserves body, historical images and shared assets");v.shutdown();
    }
    {
        auto p=historical(t,reg,false,true);auto bad=reg;bad.at(2).layout->groups.resize(200);
        maps::build_regenerated_map_render_plan(p,bad);openemperor::StoredGraphicsRenderer v(std::move(p));
        v.initialize(r);v.set_landscape_mode(openemperor::LandscapeDebugMode::Regenerated);auto c=camera();frame(v,r,c);
        check(v.plan().assets[1].status==maps::StoredStatus::Rendered&&v.last_texture_draws()==3,
            "optional join failure preserves ready foreign historical asset and its ordinary draw");v.shutdown();
    }
    {
        Temp omega;const auto rg=registrations(omega);auto p=historical(omega,rg);auto sources=std::make_shared<maps::OrdinaryTowerMapPresentation>(*p.original_ordinary_towers);
        auto input=facts(sources->towers[0]);input[5]->terrain=0;input[4]->terrain=0x4000;
        sources->towers[0].joins=maps::select_ordinary_tower_joins(sources->towers[0],input);p.original_ordinary_towers=sources;
        maps::build_regenerated_map_render_plan(p,rg);openemperor::StoredGraphicsRenderer v(std::move(p));v.initialize(r);v.set_landscape_mode(openemperor::LandscapeDebugMode::Regenerated);auto c=camera();frame(v,r,c);
        check(pixel(r,c.world_to_screen(join_sample(5)))==green&&v.hit_test(c.world_to_screen(join_sample(5)),c)==marker,"bounded Type256 variant5 uses existing Omega decoder, matching draw and cached alpha");v.shutdown();
    }
}

}
int main(){SDL_Window*w=nullptr;SDL_Renderer*r=nullptr;try{
    check(SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy")&&SDL_Init(SDL_INIT_VIDEO),"SDL init");
    w=SDL_CreateWindow("authored ordinary Tower joins",1200,900,SDL_WINDOW_HIDDEN);check(w,"window");
    r=SDL_CreateRenderer(w,"software");check(r,"renderer");pure_selection();positive_join(r);failures_height_reuse(r);
    check(openemperor::StoredGraphicsRenderer::live_texture_count()==0,"shutdown");
    SDL_DestroyRenderer(r);SDL_DestroyWindow(w);SDL_Quit();std::cout<<"ordinary Tower joins PASS\n";return 0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';if(r)SDL_DestroyRenderer(r);if(w)SDL_DestroyWindow(w);SDL_Quit();return 1;}}
