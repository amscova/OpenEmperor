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
        return square ? x>=45 && x<114 && y>=10 && y<100 && !(x>=77 && x<84 && y>=49 && y<57)
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
        ("openemperor-tower-body-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    explicit Temp(bool malformed_body=false) {
        std::filesystem::create_directories(root/"DATA");
        Bytes sg3(40680+4*72,0),bitmap(4,0);
        u32(sg3,0,unsigned(sg3.size()));u32(sg3,4,214);u32(sg3,12,4);u32(sg3,16,4);u32(sg3,20,1);
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
        write(root/"DATA/authored.sg3",sg3);write(root/"DATA/authored.555",bitmap);
    }
    ~Temp() {std::error_code e;std::filesystem::remove_all(root,e);}
};
maps::StoredArchiveRegistrations registrations(const Temp& t) {
    maps::StoredArchiveRegistration r;r.slot=2;r.relative_path="DATA/authored.sg3";
    r.catalog=openemperor::assets::scan_asset_archive(t.root,r.relative_path);
    check(r.catalog->records.size()==4 && r.catalog->archive_errors.empty(),"authored side-two Type30 metadata");
    r.layout.emplace();auto& l=*r.layout;l.slot=2;l.sg3_version=213;l.verified_registration=true;
    l.first_physical_record=1;l.image_capacity=4;l.runtime_image_count=1;l.groups.resize(76);
    for (unsigned n=0;n<76;++n) l.groups[n]={n,80+2*n,0,0};
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
nlohmann::json stats(const maps::StoredGraphicsPlan& p) {
    return maps::landscape_fidelity_report(p).at("regeneration").at("ordinary_tower_rendering");
}
void check_counts(const maps::StoredGraphicsPlan& p,unsigned source_count,unsigned supported,unsigned prepared,unsigned active) {
    const auto s=stats(p);check(s.at("source_tower_objects")==source_count && s.at("supported_static_tower_objects")==supported &&
        s.at("prepared_complete_tower_objects")==prepared && s.at("active_complete_tower_objects")==active &&
        s.at("active_components")==active && s.at("active_claim_cells")==4*active &&
        s.at("full_original_composition_verified")==0 && s.at("external_wall_attachments_reproduced")==0,
        "recognized, supported, metadata-prepared and active whole bodies are separate evidence");
}
void positive(SDL_Renderer* r) {
    Temp t;const auto reg=registrations(t);auto p=historical(t,reg);const auto before=p;
    {
        auto historical_only=before;historical_only.original_ordinary_towers.reset();
        maps::build_regenerated_map_render_plan(historical_only,reg);
        openemperor::StoredGraphicsRenderer baseline(std::move(historical_only));baseline.initialize(r);
        baseline.set_debug_diagnostics(false);baseline.set_landscape_mode(openemperor::LandscapeDebugMode::Regenerated);
        const auto c=camera();frame(baseline,r,c);
        check(pixel(r,c.world_to_screen(sample(60,30)))==red &&
            pixel(r,c.world_to_screen(sample(60,30)))!=blue,
            "independent visible-body oracle rejects the frozen historical-only path");
        clear(r);baseline.begin_frame();
        for (std::size_t n=0;n<baseline.draw_items().size();++n) check(baseline.draw_ground_item(n,c),"historical-only ground baseline");
        check(pixel(r,c.world_to_screen(sample(79,125)))==red,
            "historical-only baseline demonstrates the structural Base pixel that the Tower body keeps spatial");
    }
    maps::build_regenerated_map_render_plan(p,reg);
    check(p.regenerated->instances.size()==1 && p.assets.size()==2,"one physical complete Tower body is selected automatically");
    const auto& instance=p.regenerated->instances.front();
    check(instance.graphic.value==0x8000 && p.assets[instance.asset_index].record.id.image_index==1 &&
        instance.cell_indices.size()==4 && instance.composition_group==8011 &&
        instance.composition_policy==maps::LandscapeCompositionPolicy::SpatialCombined && !instance.great_wall_context,
        "key44c runtime translation and Tower composition remain separate from Gate/GreatWall domains");
    check(p.raw_terrain==before.raw_terrain && p.raw_objects==before.raw_objects && p.raw_saved_ids==before.raw_saved_ids &&
        p.raw_candidate_bytes==before.raw_candidate_bytes && p.height_bytes==before.height_bytes &&
        p.status_by_storage==before.status_by_storage,"body preparation does not mutate raw map or original authority");
    auto changed=before;changed.raw_saved_ids.assign(count,123);changed.raw_candidate_bytes.assign(count,0);
    for (auto& c:changed.cells) c.stored_id=123;
    maps::build_regenerated_map_render_plan(changed,reg);
    check(changed.regenerated->instances.front().graphic.value==instance.graphic.value &&
        changed.regenerated->instances.front().geometry.owned_cells==instance.geometry.owned_cells,
        "historical IDs and subtile bytes never choose Tower resource or complete renderer ownership");
    const auto cached=p.regenerated;maps::build_regenerated_map_render_plan(p,reg);
    check(p.regenerated==cached && p.assets.size()==2,"load-time Tower selection is immutable");
    const auto initial_textures=openemperor::StoredGraphicsRenderer::live_texture_count();
    openemperor::StoredGraphicsRenderer v(std::move(p));v.initialize(r);v.set_debug_diagnostics(false);
    v.set_landscape_mode(openemperor::LandscapeDebugMode::Regenerated);check_counts(v.plan(),1,1,1,1);
    check(v.upload_count()==6 && v.plan().decoded_assets==2 && v.plan().logical_texture_bytes==2U*158U*180U*4U*3U,
        "historical and one additional physical body each eagerly upload Combined/Base/Overlay once within existing budget");
    for (const double zoom:{1.,1.5,2.,4.}) {
        const auto c=camera(zoom);frame(v,r,c);
        check(v.last_texture_draws()==1 && pixel(r,c.world_to_screen(sample(60,30)))==blue &&
            pixel(r,c.world_to_screen(sample(79,125)))==red &&
            v.hit_test(c.world_to_screen(sample(60,30)),c)==marker &&
            v.hit_test(c.world_to_screen(sample(79,125)),c)==marker,
            "whole body and structural Base draw once at every supported zoom with the same alpha origin");
        check(pixel(r,c.world_to_screen(sample(79,51)))==black &&
            !v.hit_test(c.world_to_screen(sample(79,51)),c) &&
            !v.hit_test(c.world_to_screen(sample(5,5)),c),"transparent window/canvas never block a rear hit");
    }
    auto c=camera();clear(r);v.begin_frame();
    for (std::size_t n=0;n<v.draw_items().size();++n) check(v.draw_ground_item(n,c),"existing early ground pass");
    check(v.last_texture_draws()==0 && pixel(r,c.world_to_screen(sample(79,125)))==black,
        "Tower structural Base never leaks into the early road/ground pass");
    for (std::size_t n=0;n<v.draw_items().size();++n) check(v.draw_item(n,c),"existing spatial pass");
    check(v.last_texture_draws()==1 && pixel(r,c.world_to_screen(sample(79,125)))==red,
        "one Combined spatial submission includes the complete body base and roof");
    const auto hit=v.hit_test_item(c.world_to_screen(sample(60,30)),c);
    check(hit && hit->key.depth==3100 && hit->key.ground_x==-400,"Tower retains the independently calculated front-ground painter key");
    for (bool front:{false,true}) for (unsigned y:{30U,125U}) {
        struct Dynamic {scene::WorldDrawKey key;};
        const std::array<Dynamic,1> dynamic{{{{front ? 3120. : 3040.,-400.,scene::WorldVisualLayer::SandboxBuilding,7}}}};
        clear(r);v.begin_frame();for (std::size_t n=0;n<v.draw_items().size();++n) check(v.draw_ground_item(n,c),"merge ground");
        const auto sample_screen=c.world_to_screen(sample(60,y));scene::WorldMergeStats merge;
        check(scene::merge_world_draw_streams(v.draw_items(),dynamic,[&](std::size_t n){return v.draw_item(n,c);},
            [&](std::size_t){const SDL_FRect rect{float(sample_screen.x),float(sample_screen.y),2,2};
                return SDL_SetRenderDrawColor(r,0,255,0,255) && SDL_RenderFillRect(r,&rect);},merge),"merge an independent neighboring item");
        check(pixel(r,sample_screen)==(front ? green:y<100 ? blue:red),
            "rear walker/neighbor loses to the body while a front neighbor occludes roof and structural Base");
    }
    for (const auto mode:{openemperor::LandscapeDebugMode::Ground,openemperor::LandscapeDebugMode::MountainsRocks}) {
        v.set_landscape_mode(mode);frame(v,r,c);check(v.last_texture_draws()==0 &&
            !v.hit_test(c.world_to_screen(sample(60,30)),c),"F8 hides complete roof and structural foundation together");
    }
    v.set_landscape_mode(openemperor::LandscapeDebugMode::WallsMonuments);frame(v,r,c);
    check(v.last_texture_draws()==1 && pixel(r,c.world_to_screen(sample(60,30)))==blue,"F8 Wall/Monument layer includes one whole Tower");
    v.set_landscape_mode(openemperor::LandscapeDebugMode::Snapshot);frame(v,r,c);
    check(v.last_texture_draws()==1 && pixel(r,c.world_to_screen(sample(60,30)))==red,"explicit Snapshot retains only the complete old image");
    v.set_landscape_mode(openemperor::LandscapeDebugMode::Regenerated);
    const auto f1=maps::landscape_provenance(v.plan(),origin,true,&c).at("ordinary_tower");
    check(f1.at("original_id")==7 && f1.at("manager_index")==11 && f1.at("type")==131 &&
        f1.at("selected_components")==1 && f1.at("active_components")==1 && f1.at("preparation_status")=="active" &&
        f1.at("components").at(0).at("height_step_pixels")==40 && f1.at("external_wall_attachments").get<std::string>().find("UNRESOLVED")!=std::string::npos,
        "F1 reports typed source, one complete active body, raw height and remaining composition boundary");
    perf::set_enabled(true);perf::reset();
    for (unsigned n=0;n<20;++n) {frame(v,r,c);(void)v.hit_test(c.world_to_screen(sample(60,30)),c);
        (void)maps::landscape_provenance(v.plan(),origin,true,&c);(void)stats(v.plan());}
    for (std::size_t n=0;n<std::size_t(perf::Counter::Count);++n)
        check(perf::counter(static_cast<perf::Counter>(n))==0,"cached Tower render/pick/F1 adds zero asset/file/World/command/tick/navigation work");
    perf::set_enabled(false);v.shutdown();check(openemperor::StoredGraphicsRenderer::live_texture_count()==initial_textures,"Tower shutdown releases all owned textures");
}
void height_culling_and_reuse(SDL_Renderer* r) {
    Temp t;const auto reg=registrations(t);
    const auto initial_textures=openemperor::StoredGraphicsRenderer::live_texture_count();
    for (const int elevation:{-128,-2,1,127}) {
        auto p=historical(t,reg,true);for (auto& h:p.height_bytes) h=std::uint8_t(elevation);
        maps::build_regenerated_map_render_plan(p,reg);check(p.regenerated->instances.size()==2 && p.assets.size()==2,
            "two independent Towers share one physical static body and do not duplicate textures");
        check(p.regenerated->instances[0].asset_index==p.regenerated->instances[1].asset_index,
            "same body physical identity deduplicates across complete source objects");
        openemperor::StoredGraphicsRenderer v(std::move(p));v.initialize(r);v.set_debug_diagnostics(false);
        v.set_landscape_mode(openemperor::LandscapeDebugMode::Regenerated);check_counts(v.plan(),2,2,2,2);check(v.upload_count()==6,"shared bodies upload no per-object texture copies");
        for (const double zoom:{1.,1.5,2.,4.}) {
            auto c=camera(zoom);c.center_on(sample(79,75,elevation));frame(v,r,c);
            check(pixel(r,c.world_to_screen(sample(60,30,elevation)))==blue &&
                v.hit_test(c.world_to_screen(sample(60,30,elevation)),c)==marker,
                "signed height shifts all body pixels and alpha exactly once, independently of image height");
        }
        for (unsigned side=0;side<4;++side) {
            auto c=camera();const auto pnt=sample(60,30,elevation);
            const scene::Point desired{side==0 ? 2.5:side==1 ? 1197.5:600.5,side==2 ? 2.5:side==3 ? 897.5:450.5};
            c.offset={desired.x-pnt.x,desired.y-pnt.y};frame(v,r,c);
            check(pixel(r,desired)==blue && v.hit_test(desired,c)==marker,
                "full elevated image bounds retain visible upper body across every viewport side");
        }
        for (unsigned side=0;side<4;++side) {
            auto c=camera();const auto top=body_top(origin,elevation);
            // Both complete towers are moved beyond the same viewport boundary.
            c.offset={side==0 ? -2000.-top.x:side==1 ? 1500.-top.x:600.-top.x,
                side==2 ? -2000.-top.y:side==3 ? 1500.-top.y:-1000.-top.y};
            frame(v,r,c);check(v.last_texture_draws()==0,"wholly outside bodies are culled without leaving roof fragments");
        }
        auto c=camera();c.viewport_width=920;c.viewport_height=640;c.center_on(sample(79,75,elevation));frame(v,r,c);
        check(pixel(r,c.world_to_screen(sample(60,30,elevation)))==blue,"changed viewport dimensions retain the same common body origin");
        v.shutdown();check(openemperor::StoredGraphicsRenderer::live_texture_count()==initial_textures,"each isolated height/session releases its texture pool");
    }
}
void foreign_alpha_and_atomic_failures(SDL_Renderer* r) {
    {
        Temp t;auto reg=registrations(t);
        auto rock=reg.at(2);rock.slot=3;rock.layout->slot=3;rock.layout->first_physical_record=3;
        rock.layout->groups.resize(6);reg.emplace(3,std::move(rock));
        constexpr std::uint64_t body_rgba=158U*180U*4U*3U,rock_rgba=78U*120U*4U*3U;
        for (bool one_byte_short:{false,true}) {
            auto p=historical(t,reg);const maps::GridCell cell{113,115};const auto at=raw(cell);
            p.raw_terrain[at]=2;p.variation_bytes[at]=0;
            maps::StoredCell c;c.storage=cell;c.cell_index=at;c.terrain_raw=2;c.slot=2;
            c.status=maps::StoredStatus::DecodePending;c.world={-80,3400};
            p.cell_by_storage[at]=p.cells.size();p.status_by_storage[at]=c.status;p.cells.push_back(c);
            p.logical_texture_bytes=maps::stored_max_texture_bytes-2U*body_rgba-rock_rgba+(one_byte_short ? 1U:0U);
            const auto reserved=p.logical_texture_bytes;
            maps::build_regenerated_map_render_plan(p,reg);
            check(p.assets.size()==(one_byte_short ? 2U:3U) && p.logical_texture_bytes==reserved &&
                p.regenerated->instances.size()==(one_byte_short ? 1U:2U),
                "normal landscape assets selected later in the existing pipeline are fully reserved before optional Tower admission");
            const auto& retained=p.regenerated->instances.front();
            check(retained.geometry.selection.family==maps::LandscapeFamily::Rock &&
                p.assets[retained.asset_index].record.id.image_index==3 &&
                p.assets[retained.asset_index].status==maps::StoredStatus::DecodePending &&
                retained.cell_indices==std::vector<std::size_t>{4},
                "one-byte short Tower refusal preserves the newly prepared foreign Rock image and complete claim");
            check(one_byte_short ? p.regenerated->ordinary_tower_preparation.at(0).fallback.find("budget")!=std::string::npos:
                    p.regenerated->ordinary_tower_preparation.at(0).instance_indices.size()==1,
                "late foreign resource is included in exact-capacity and one-byte-short Tower budget accounting");
        }
    }
    {
        Temp t;const auto reg=registrations(t);
        constexpr std::uint64_t one_body_rgba=158U*180U*4U*3U;
        for (bool one_byte_short:{false,true}) {
            auto p=historical(t,reg);
            p.assets.push_back({reg.at(2).catalog->records[1],maps::StoredStatus::UnsupportedFootprintSize,false,false,{}});
            p.logical_texture_bytes=maps::stored_max_texture_bytes-2U*one_body_rgba+(one_byte_short ? 1U:0U);
            const auto reserved=p.logical_texture_bytes;
            maps::build_regenerated_map_render_plan(p,reg);
            check(p.assets.size()==2 && p.logical_texture_bytes==reserved &&
                p.regenerated->instances.size()==(one_byte_short ? 0U:1U) &&
                p.assets[1].status==(one_byte_short ? maps::StoredStatus::UnsupportedFootprintSize:maps::StoredStatus::DecodePending),
                "reusing an unsupported physical body reserves all three RGBA textures before changing its decode status");
            check(one_byte_short ? p.regenerated->ordinary_tower_preparation.at(0).fallback.find("budget")!=std::string::npos:
                    p.regenerated->ordinary_tower_preparation.at(0).fallback.empty(),
                "exact pooled capacity succeeds and one-byte short optional preflight retains the prior pool atomically");
        }
    }
    {
        Temp t;const auto reg=registrations(t);auto p=historical(t,reg,true);
        auto sources=std::make_shared<maps::OrdinaryTowerMapPresentation>(*p.original_ordinary_towers);
        sources->towers[1].components.front().owned_cells.pop_back();p.original_ordinary_towers=std::move(sources);
        maps::build_regenerated_map_render_plan(p,reg);
        openemperor::StoredGraphicsRenderer v(std::move(p));v.initialize(r);v.set_debug_diagnostics(false);
        v.set_landscape_mode(openemperor::LandscapeDebugMode::Regenerated);check_counts(v.plan(),2,2,1,1);
        auto c=camera();frame(v,r,c);check(pixel(r,c.world_to_screen(sample(60,30)))==blue,
            "a second source claim refusal cannot roll back the already prepared complete first body");
        c.center_on(sample(79,75,0,{120,130}));frame(v,r,c);
        check(pixel(r,c.world_to_screen(sample(60,30,0,{120,130})))==red && v.upload_count()==6,
            "the refused independent body retains its complete historical image and the shared valid physical pool");
    }
    for (bool corrupt:{false,true}) {
        Temp t(corrupt);const auto reg=registrations(t);auto p=historical(t,reg,false,true);
        maps::build_regenerated_map_render_plan(p,reg);
        openemperor::StoredGraphicsRenderer v(std::move(p));v.initialize(r);v.set_debug_diagnostics(false);
        v.set_landscape_mode(openemperor::LandscapeDebugMode::Regenerated);check_counts(v.plan(),1,1,1,corrupt ? 0:1);
        const auto c=camera();frame(v,r,c);
        check(pixel(r,c.world_to_screen(sample(60,30)))==(corrupt ? red:blue),
            "a failed selected-body decode atomically retains the complete old body");
        check(pixel(r,c.world_to_screen(sample(79,51)))==green &&
            v.hit_test(c.world_to_screen(sample(79,51)),c)==maps::GridCell{99,110},
            "transparent body window admits the genuinely visible rear stored alpha hit");
        check(v.plan().assets[1].status==maps::StoredStatus::Rendered && v.plan().assets[1].decode_succeeded,
            "failed optional Tower body never removes an unrelated prepared asset");
        if (corrupt) check(maps::landscape_provenance(v.plan(),origin,true).at("ordinary_tower").at("preparation_status")=="decode_failed",
            "F1 names decode failure separately from unsupported source or missing group");
    }
    for (unsigned failure=0;failure<12;++failure) {
        Temp t;auto reg=registrations(t);auto p=historical(t,reg);
        if (failure==0) reg.clear();
        if (failure==1) reg.at(2).layout->runtime_image_count=0;
        if (failure==2) reg.at(2).catalog->records[1].width=78;
        if (failure==3) reg.at(2).catalog->records[1].horizontal_mirror_offset=2;
        if (failure==4) reg.at(2).catalog->records[1].animation_sprites=1;
        if (failure==5 || failure==6) {
            auto selected=std::make_shared<maps::OrdinaryTowerMapPresentation>(*p.original_ordinary_towers);
            if (failure==5) selected->towers[0].components[0].selection.variant=1;
            else selected->towers[0].components[0].owned_cells.pop_back();
            p.original_ordinary_towers=std::move(selected);
        }
        if (failure==7) while(p.assets.size()<maps::stored_max_assets) {
            openemperor::assets::AssetRecord record;record.id={"DATA/budget-"+std::to_string(p.assets.size())+".sg3",0};
            p.assets.push_back({record,maps::StoredStatus::UnsupportedLayout,false,false,{}});
        }
        if (failure==8) p.logical_texture_bytes=maps::stored_max_texture_bytes;
        if (failure==9) reg.at(2).catalog->records[1].data_length=maps::stored_max_payload_bytes+1;
        if (failure==10) reg.at(2).catalog->records[1].alpha_length=maps::stored_max_payload_bytes+1;
        if (failure==11) reg.at(2).catalog->records[1].height=32767;
        const auto asset_count=p.assets.size();maps::build_regenerated_map_render_plan(p,reg);
        check(p.regenerated->instances.empty() && p.assets.size()==asset_count &&
            !p.regenerated->ordinary_tower_preparation.at(0).fallback.empty(),
            "missing group, bad resource/claim or unchanged physical/RGBA budget publishes no partial body or asset");
        if (failure==8) continue; // Authored reserved-byte accounting only; do not decode a fabricated full pool.
        openemperor::StoredGraphicsRenderer v(std::move(p));v.initialize(r);v.set_debug_diagnostics(false);
        v.set_landscape_mode(openemperor::LandscapeDebugMode::Regenerated);const auto c=camera();frame(v,r,c);
        check(pixel(r,c.world_to_screen(sample(60,30)))==red &&
            maps::landscape_provenance(v.plan(),origin,true).at("ordinary_tower").at("preparation_status")=="complete_preparation_unavailable",
            "metadata preflight refusal keeps the whole historical comparison body and named reason");
    }
}
void historical_overlap(SDL_Renderer* r) {
    Temp t;const auto reg=registrations(t);auto p=historical(t,reg);
    p.assets.push_back({reg.at(2).catalog->records[3],maps::StoredStatus::DecodePending,false,false,{}});
    const auto old_own=p.footprints.front().cell_indices;
    p.footprints.front().origin={99,110};p.footprints.front().cell_indices={old_own[0],old_own[2]};
    p.footprints.front().image_origin=body_top({99,110});
    for (const auto cell:std::array<maps::GridCell,2>{{{99,110},{99,111}}}) {
        p.raw_terrain[raw(cell)]=8;maps::StoredCell c;c.storage=cell;c.cell_index=raw(cell);c.terrain_raw=8;c.slot=2;
        c.asset_index=0;c.footprint_index=0;c.status=maps::StoredStatus::DecodePending;
        c.world={(int(cell.x)-int(cell.y))*40.,(int(cell.x)+int(cell.y)-58)*20.};
        p.cell_by_storage[raw(cell)]=p.cells.size();p.status_by_storage[raw(cell)]=c.status;
        p.footprints.front().cell_indices.push_back(p.cells.size());p.cells.push_back(c);
    }
    for (const auto n:{old_own[1],old_own[3]}) {
        auto& c=p.cells[n];maps::PlacedFootprint f;f.id=p.footprints.size();f.origin=c.storage;f.asset_index=1;
        f.cell_indices={n};f.status=maps::StoredStatus::DecodePending;f.image_origin={c.world.x-39,c.world.y-80};
        c.asset_index=1;c.footprint_index=f.id;p.footprints.push_back(std::move(f));
    }
    maps::build_regenerated_map_render_plan(p,reg);check(p.regenerated->instances.size()==1,"complete source body is selected before old-footprint readiness");
    openemperor::StoredGraphicsRenderer v(std::move(p));v.initialize(r);v.set_landscape_mode(openemperor::LandscapeDebugMode::Regenerated);
    check_counts(v.plan(),1,1,1,0);check(std::none_of(v.plan().regenerated_instance_active.begin(),v.plan().regenerated_instance_active.end(),
        [](auto ready){return bool(ready);}) &&
        maps::landscape_provenance(v.plan(),origin,true).at("ordinary_tower").at("preparation_status")=="atomic_activation_unavailable",
        "a historical footprint crossing a body boundary retains its entire old composition without arbitrary neighbor suppression");
}
} // namespace

int main() {
    SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;
    try {
        check(SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy") && SDL_Init(SDL_INIT_VIDEO),"SDL dummy authored Tower tests");
        window=SDL_CreateWindow("authored complete cTower tests",1200,900,SDL_WINDOW_HIDDEN);check(window,"hidden authored window");
        renderer=SDL_CreateRenderer(window,"software");check(renderer,"real software backend");
        positive(renderer);height_culling_and_reuse(renderer);foreign_alpha_and_atomic_failures(renderer);historical_overlap(renderer);
        check(openemperor::StoredGraphicsRenderer::live_texture_count()==0,"all Tower sessions terminate without owned textures");
        SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();std::cout<<"ordinary cTower production pixels/readiness/resource/picking PASS\n";return 0;
    } catch (const std::exception& e) {
        perf::set_enabled(false);std::cerr<<e.what()<<": "<<SDL_GetError()<<'\n';
        if(renderer) SDL_DestroyRenderer(renderer);
        if(window) SDL_DestroyWindow(window);
        SDL_Quit();return 1;
    }
}
