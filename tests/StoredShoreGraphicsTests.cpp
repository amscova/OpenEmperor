#include "maps/StoredMapSession.h"
#include "maps/SandboxPlacement.h"
#include "maps/LandscapeProvenance.h"
#include "maps/RegeneratedMapRenderPlan.h"
#include "core/PerformanceDiagnostics.h"
#include "renderer/StoredGraphicsRenderer.h"

#include <SDL3/SDL.h>
#include <zlib.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <vector>

namespace {
namespace fs=std::filesystem;
namespace maps=openemperor::maps;
using Bytes=std::vector<std::uint8_t>;
void check(bool condition,const char* reason) {if(!condition)throw std::runtime_error(reason);}
void u16(Bytes& b,std::size_t at,std::uint16_t value) {
    b.at(at)=static_cast<std::uint8_t>(value);b.at(at+1)=static_cast<std::uint8_t>(value>>8U);
}
void u32(Bytes& b,std::size_t at,std::uint32_t value) {
    u16(b,at,static_cast<std::uint16_t>(value));u16(b,at+2,static_cast<std::uint16_t>(value>>16U));
}
void write(const fs::path& path,const Bytes& bytes) {
    std::ofstream out(path,std::ios::binary);
    out.write(reinterpret_cast<const char*>(bytes.data()),static_cast<std::streamsize>(bytes.size()));
    check(bool(out),"write authored shore fixture");
}
Bytes read(const fs::path& path) {
    std::ifstream input(path,std::ios::binary);check(bool(input),"read own fixture");
    return Bytes{std::istreambuf_iterator<char>{input},std::istreambuf_iterator<char>{}};
}
struct Fixture {
    fs::path root=fs::temp_directory_path()/
        ("openemperor-shore-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    Fixture(){fs::create_directories(root/"DATA");}
    ~Fixture(){std::error_code error;fs::remove_all(root,error);}
};
Bytes archive(unsigned count) {
    Bytes b(40680U+(count+2U)*64U,0);
    u32(b,0,static_cast<std::uint32_t>(b.size()));u32(b,4,213);
    u32(b,12,count+2);u32(b,16,count);u32(b,20,1);
    const std::string system="Zeus_system.bmp";std::copy(system.begin(),system.end(),b.begin()+680);
    u32(b,804,200);u32(b,808,1);u32(b,812,200);
    return b;
}
void record(Bytes& b,unsigned physical,unsigned offset,unsigned length,unsigned width,
            unsigned height,unsigned base,unsigned side) {
    const auto at=40680U+physical*64U;
    u32(b,at,offset);u32(b,at+4,length);u32(b,at+8,base);
    u16(b,at+20,static_cast<std::uint16_t>(width));u16(b,at+22,static_cast<std::uint16_t>(height));
    u16(b,at+50,30);b.at(at+55)=static_cast<std::uint8_t>(side);
}
Bytes color(unsigned bytes,std::uint16_t rgb) {
    Bytes b(bytes);for(std::size_t i=0;i<b.size();i+=2)u16(b,i,rgb);return b;
}
Bytes compressed(const Bytes& raw) {
    Bytes out{0xaa,0xba,0xdc,0xfe};
    for(std::size_t at=0;at<raw.size();at+=32768) {
        const auto n=std::min<std::size_t>(32768,raw.size()-at);
        uLongf length=compressBound(static_cast<uLong>(n));Bytes zipped(static_cast<std::size_t>(length));
        check(compress2(zipped.data(),&length,raw.data()+at,static_cast<uLong>(n),6)==Z_OK,"compress authored map");
        zipped.resize(static_cast<std::size_t>(length));const auto start=out.size();out.resize(start+12);
        u32(out,start,0x12345678);u32(out,start+4,static_cast<std::uint32_t>(zipped.size()));
        u32(out,start+8,static_cast<std::uint32_t>(n));out.insert(out.end(),zipped.begin(),zipped.end());
    }
    return out;
}
Bytes map_bytes() {
    Bytes raw(1093613U,0);
    const std::array<std::uint8_t,8> signature{5,0,0xfe,0xca,0,0,2,0};
    std::copy(signature.begin(),signature.end(),raw.begin());u32(raw,84,84);u16(raw,1093607,1);
    for(std::size_t at=0;at<228U*228U;++at) {
        u32(raw,std::size_t(maps::candidate_word_logical_offset)+at*4U,0xc000);
        raw.at(std::size_t(maps::candidate_byte_logical_offset)+at)=64;
        u32(raw,std::size_t(maps::terrain_logical_offset)+at*4U,0x0c);
    }
    // Independent literal saved-image parts: one complete canonical square.
    const std::array<std::uint8_t,4> parts{0,1,72,9};
    for(unsigned dy=0;dy<2;++dy)for(unsigned dx=0;dx<2;++dx) {
        const auto at=(114U+dy)*228U+114U+dx;
        u32(raw,std::size_t(maps::candidate_word_logical_offset)+at*4U,0x8000U+139U);
        raw.at(std::size_t(maps::candidate_byte_logical_offset)+at)=parts[dy*2U+dx];
        u32(raw,std::size_t(maps::terrain_logical_offset)+at*4U,dy ? 0x0c:0x88);
        raw.at(std::size_t(maps::landscape_height_offset)+at)=2;
    }
    const auto pier=116U*228U+115U;
    u32(raw,std::size_t(maps::candidate_word_logical_offset)+pier*4U,0x8000U+143U);
    raw.at(std::size_t(maps::landscape_height_offset)+pier)=2;
    return raw;
}
void square(Bytes& raw,unsigned x,unsigned y,unsigned local) {
    const std::array<std::uint8_t,4> parts{0,1,72,9};
    for(unsigned dy=0;dy<2;++dy)for(unsigned dx=0;dx<2;++dx) {
        const auto at=(y+dy)*228U+x+dx;
        u32(raw,std::size_t(maps::candidate_word_logical_offset)+at*4U,0x8000U+local);
        raw.at(std::size_t(maps::candidate_byte_logical_offset)+at)=parts[dy*2U+dx];
        u32(raw,std::size_t(maps::terrain_logical_offset)+at*4U,dy ? 0x0c:0x88);
        raw.at(std::size_t(maps::landscape_height_offset)+at)=2;
    }
}
void sources(Fixture& f) {
    auto terrain=archive(201);record(terrain,201,0,3200,78,40,3200,1);
    write(f.root/"DATA/China_Terrain.sg3",terrain);write(f.root/"DATA/China_Terrain.555",color(3200,0x001f));
    write(f.root/"DATA/China_Elevation.sg3",archive(200));write(f.root/"DATA/China_Elevation.555",{});
    auto general=archive(348);
    for(unsigned i=0;i<=139;++i)u16(general,80U+i*2U,static_cast<std::uint16_t>(201U+i));
    u16(general,80U+140U*2U,348); // key0x48c locals[139,147), exactly eight variants.
    auto body=color(12800,0x03e0);
    const Bytes upper{255,30,1,0x00,0x7c,255,62,1,0x1f,0x7c};
    body.insert(body.end(),upper.begin(),upper.end());
    record(general,340,0,static_cast<unsigned>(body.size()),158,100,12800,2);
    record(general,341,0,static_cast<unsigned>(body.size()),158,100,12800,2);
    record(general,342,0,static_cast<unsigned>(body.size()),158,96,12800,2);
    record(general,343,0,static_cast<unsigned>(body.size()),158,100,12800,2);
    const auto pier_offset=static_cast<unsigned>(body.size());auto pier=color(3200,0x03ff);
    const Bytes post{255,20,1,0xe0,0x7f};pier.insert(pier.end(),post.begin(),post.end());
    body.insert(body.end(),pier.begin(),pier.end());
    record(general,344,pier_offset,static_cast<unsigned>(pier.size()),78,60,3200,1);
    write(f.root/"DATA/China_General.sg3",general);write(f.root/"DATA/China_General.555",body);
    write(f.root/"Shore.map",compressed(map_bytes()));
}
std::array<std::uint8_t,4> pixel(SDL_Renderer* renderer,int x,int y) {
    auto* surface=SDL_RenderReadPixels(renderer,nullptr);check(surface!=nullptr,"read authored pixel");
    std::array<std::uint8_t,4> rgba{};
    const auto okay=SDL_ReadSurfacePixel(surface,x,y,&rgba[0],&rgba[1],&rgba[2],&rgba[3]);
    SDL_DestroySurface(surface);check(okay,"read literal pixel coordinate");return rgba;
}
void positive() {
    Fixture fixture;sources(fixture);
    auto session=maps::load_stored_map_session(fixture.root,"Shore.map",maps::FootprintPolicy::EdgeByte4x4Preview);
    auto* window=SDL_CreateWindow("owned shoreline test",420,400,SDL_WINDOW_HIDDEN|SDL_WINDOW_RESIZABLE);
    auto* renderer=SDL_CreateRenderer(window,"software");check(renderer!=nullptr,"software renderer");
    openemperor::StoredGraphicsRenderer view(std::move(session.plan));view.initialize(renderer);
    view.set_debug_diagnostics(false);view.set_landscape_mode(openemperor::LandscapeDebugMode::Regenerated);
    openemperor::scene::Camera2D camera;camera.viewport_width=420;camera.viewport_height=400;camera.offset={200,-1380};
    SDL_SetRenderDrawColor(renderer,0,0,0,255);SDL_RenderClear(renderer);
    check(view.render(camera,{}),"production shore render");
    const auto observed=pixel(renderer,200,240);
    std::cout<<"literal shore ground pixel "<<unsigned(observed[0])<<','<<unsigned(observed[1])<<','<<unsigned(observed[2])<<'\n';
    check(observed==std::array<std::uint8_t,4>{0,255,0,255},
          "saved shore body must replace the actual brown 2x2 fallback");
    check(pixel(renderer,151,200)==std::array<std::uint8_t,4>{255,0,0,255} &&
          pixel(renderer,214,200)==std::array<std::uint8_t,4>{255,0,255,255},
          "asymmetric land body uses literal side-two anchor and signed height exactly once");
    check(pixel(renderer,141,260)==std::array<std::uint8_t,4>{255,255,0,255},
          "independent existing pier post remains at its original position");
    const auto& p=view.plan();const auto* first=p.at({114,114});const auto* pier=p.at({115,116});
    check(first && first->footprint_index && pier && pier->footprint_index &&
          first->footprint_index!=pier->footprint_index,"land body and independent pier own distinct instances");
    const auto& fp=p.footprints[*first->footprint_index];
    check(fp.width_cells==2 && fp.height_cells==2 && fp.cell_indices.size()==4 && fp.presentation_only &&
          fp.draw_cell_candidate==maps::GridCell{114,115},"canonical complete historical square and marker");
    check(view.hit_test({151.5,200.5},camera)==maps::GridCell{114,115} &&
          !view.hit_test({150.5,200.5},camera),"visible asymmetric alpha and transparent neighbor");
    const auto mask=maps::make_sandbox_buildable_mask(p,maps::MapGeometry{84});
    check(mask[114U*228U+114U]==0 && mask[115U*228U+115U]==0,"late 2x2 art has no legacy buildability");
    std::vector<std::uint8_t> replacement(228U*228U,2);
    std::size_t body_index=0,body_draws=0;
    for(std::size_t i=0;i<view.draw_items().size();++i) {
        const auto& item=view.draw_items()[i];
        if(item.footprint && item.plan_index==fp.id) {
            body_index=i;++body_draws;
            check(view.ground_replacement_role(i,replacement,228)==0,
                  "multi-cell historical index never aliases a road replacement");
        }
    }
    check(body_draws==1,"four saved parts produce one draw descriptor");
    const auto uploads=view.upload_count(),order_builds=view.stored_order_builds();
    openemperor::performance::set_enabled(true);openemperor::performance::reset();
    for(double zoom:{1.0,1.5,2.0,4.0,1.0}) {
        camera.zoom=zoom;camera.offset={151.125+zoom*49.0,200.375-zoom*1580.0};
        SDL_RenderClear(renderer);check(view.render(camera,{}),"fractional pan/intermediate zoom render");
        check(pixel(renderer,151,200)==std::array<std::uint8_t,4>{255,0,0,255} &&
              view.hit_test({151.375,200.625},camera)==maps::GridCell{114,115},
              "literal elevated body coordinate survives zoom and fractional pan");
    }
    check(SDL_SetWindowSize(window,317,289),"resize only the owned dummy window");
    int width=0,height=0;check(SDL_GetCurrentRenderOutputSize(renderer,&width,&height) && width==317 && height==289,
                             "actual software output follows owned window resize");
    camera.viewport_width=width;camera.viewport_height=height;
    SDL_RenderClear(renderer);check(view.render(camera,{}),"resized output keeps the same eager image");
    check(pixel(renderer,151,200)==std::array<std::uint8_t,4>{255,0,0,255},"resize retains literal body pixel");
    view.set_landscape_mode(openemperor::LandscapeDebugMode::Ground);
    SDL_RenderClear(renderer);check(view.render(camera,{}),"F8 Ground component mode");
    check(pixel(renderer,151,200)!=std::array<std::uint8_t,4>{255,0,0,255} &&
          !view.hit_test({151.375,200.625},camera),"hidden structural Overlay cannot retain an alpha hit");
    view.set_landscape_mode(openemperor::LandscapeDebugMode::Snapshot);
    camera.offset={200,-1380};SDL_RenderClear(renderer);check(view.render(camera,{}),"flat historical Snapshot");
    check(pixel(renderer,151,280)==std::array<std::uint8_t,4>{255,0,0,255} &&
          view.hit_test({151.5,280.5},camera)==maps::GridCell{114,115},"Snapshot keeps complete art without saved elevation");
    view.set_landscape_mode(openemperor::LandscapeDebugMode::Regenerated);
    check(SDL_SetWindowSize(window,317,20),"small owned viewport for upper-image culling");
    camera.viewport_height=20;camera.offset={200,-1570};
    SDL_RenderClear(renderer);check(view.render(camera,{}),"cull by entire elevated image");
    check(pixel(renderer,151,10)==std::array<std::uint8_t,4>{255,0,0,255},
          "upper body remains visible when its footprint is below the viewport");
    check(view.upload_count()==uploads && view.stored_order_builds()==order_builds,
          "zoom, resize and modes neither upload nor rebuild stored order");
    for(std::size_t i=0;i<12;++i)check(openemperor::performance::counter(
        static_cast<openemperor::performance::Counter>(i))==0,"render/picking perform no tracked asset or World work");
    openemperor::performance::set_enabled(false);
    // Submit the actual body through its ordinary spatial draw entry. Opaque
    // authored foreground/background rectangles prove that it is not Ferry-last.
    check(SDL_SetWindowSize(window,420,400),"restore owned output");camera.viewport_width=420;camera.viewport_height=400;
    camera.offset={200,-1380};SDL_RenderClear(renderer);view.begin_frame();
    for(std::size_t i=0;i<view.draw_items().size();++i)check(view.draw_ground_item(i,camera),"ordinary ground pass");
    SDL_SetRenderDrawColor(renderer,0,0,255,255);const SDL_FRect overlap{149,198,6,6};
    check(SDL_RenderFillRect(renderer,&overlap) && view.draw_item(body_index,camera),"ordinary spatial body entry");
    check(pixel(renderer,151,200)==std::array<std::uint8_t,4>{255,0,0,255},"body covers an earlier scene item");
    SDL_SetRenderDrawColor(renderer,255,255,0,255);check(SDL_RenderFillRect(renderer,&overlap),"later scene item");
    check(pixel(renderer,151,200)==std::array<std::uint8_t,4>{255,255,0,255},"later scene item covers body normally");
    view.shutdown();SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);
}
void opposite_saved_variants() {
    // Independently authored saved1/3 squares: literal pixels and physical IDs,
    // no source-object, terrain, map-name or production selector oracle.
    for(unsigned variant:{1U,3U}) {
        Fixture f;sources(f);auto raw=map_bytes();square(raw,114,114,139+variant);
        square(raw,121,114,139+variant);write(f.root/"Shore.map",compressed(raw));
        auto session=maps::load_stored_map_session(f.root,"Shore.map",maps::FootprintPolicy::EdgeByte4x4Preview);
        auto* surface=SDL_CreateSurface(420,400,SDL_PIXELFORMAT_RGBA32);
        auto* renderer=SDL_CreateSoftwareRenderer(surface);
        openemperor::StoredGraphicsRenderer view(std::move(session.plan));view.initialize(renderer);
        view.set_debug_diagnostics(false);view.set_landscape_mode(openemperor::LandscapeDebugMode::Regenerated);
        openemperor::scene::Camera2D camera;camera.viewport_width=420;camera.viewport_height=400;camera.offset={200,-1380};
        SDL_SetRenderDrawColor(renderer,0,0,0,255);SDL_RenderClear(renderer);
        check(view.render(camera,{}),"saved1/3 production render");
        check(pixel(renderer,200,240)==std::array<std::uint8_t,4>{0,255,0,255},
              "saved opposite landing variant replaces brown diagnostic with its own authored base");
        check(pixel(renderer,151,200)==std::array<std::uint8_t,4>{255,0,0,255} &&
              pixel(renderer,214,200)==std::array<std::uint8_t,4>{255,0,255,255},
              "saved1/3 keeps full asymmetric upper image and existing side-two anchor");
        check(pixel(renderer,141,260)==std::array<std::uint8_t,4>{255,255,0,255},
              "saved1/3 never moves or removes the independent pier");
        const auto& plan=view.plan();const auto* first=plan.at({114,114});const auto* duplicate=plan.at({121,114});
        check(first->physical_record==340+variant && first->footprint_index && duplicate->footprint_index &&
              first->asset_index==duplicate->asset_index && first->footprint_index!=duplicate->footprint_index,
              "saved1/3 resolves exact original choice and deduplicates assets without merging sites");
        const auto& fp=plan.footprints[*first->footprint_index];
        check(fp.presentation_only && fp.width_cells==2 && fp.height_cells==2 && fp.cell_indices.size()==4,
              "saved1/3 retains complete presentation-only square");
        check(view.hit_test({151.5,200.5},camera)==maps::GridCell{114,115},
              "saved1/3 upper pixels and alpha hit share the same marker");
        view.shutdown();SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);
    }
}
void shared_instances() {
    Fixture f;sources(f);auto raw=map_bytes();square(raw,121,114,139);square(raw,114,123,141);
    write(f.root/"Shore.map",compressed(raw));
    auto session=maps::load_stored_map_session(f.root,"Shore.map",maps::FootprintPolicy::EdgeByte4x4Preview);
    auto* surface=SDL_CreateSurface(420,400,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);
    openemperor::StoredGraphicsRenderer view(std::move(session.plan));view.initialize(renderer);
    const auto& p=view.plan();const auto* a=p.at({114,114});const auto* b=p.at({121,114});const auto* c=p.at({114,123});
    check(a->footprint_index && b->footprint_index && c->footprint_index && a->asset_index==b->asset_index &&
          a->asset_index!=c->asset_index && a->footprint_index!=b->footprint_index,
          "separate sites share physical art without merging instances or variants");
    check(a->physical_record==340 && c->physical_record==342 && a->system_record_skip==200,
          "actual variant resolves through the shared runtime/dummy mapping once");
    check(p.assets[*c->asset_index].record.height==96 &&
          p.footprints[*c->footprint_index].image_origin.y==1844,
          "second evidenced variant retains its own height and literal side-two origin");
    check(p.assets.size()==4 && view.upload_count()==12 && p.logical_texture_bytes==465216,
          "two body variants and independent pier deduplicate complete RGBA triples exactly");
    view.shutdown();SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);
}
void signed_marker_height() {
    Fixture f;sources(f);auto raw=map_bytes();
    raw[std::size_t(maps::landscape_height_offset)+115U*228U+114U]=255;
    raw[std::size_t(maps::landscape_height_offset)+114U*228U+114U]=3;
    write(f.root/"Shore.map",compressed(raw));
    auto session=maps::load_stored_map_session(f.root,"Shore.map",maps::FootprintPolicy::EdgeByte4x4Preview);
    auto* surface=SDL_CreateSurface(420,400,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);
    openemperor::StoredGraphicsRenderer view(std::move(session.plan));view.initialize(renderer);
    view.set_landscape_mode(openemperor::LandscapeDebugMode::Regenerated);
    openemperor::scene::Camera2D camera;camera.viewport_width=420;camera.viewport_height=400;camera.offset={200,-1380};
    SDL_RenderClear(renderer);check(view.render(camera,{}),"negative saved marker height render");
    check(pixel(renderer,151,320)==std::array<std::uint8_t,4>{255,0,0,255} &&
          view.hit_test({151.5,320.5},camera)==maps::GridCell{114,115},
          "signed negative marker height is applied once, independently of the origin height");
    view.shutdown();SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);
}
void refused(const Bytes& metadata,const Bytes& raw,const Bytes* payload,const char* reason,
             bool pier_expected=true,std::uint64_t charged=0) {
    Fixture f;sources(f);write(f.root/"DATA/China_General.sg3",metadata);write(f.root/"Shore.map",compressed(raw));
    if(payload)write(f.root/"DATA/China_General.555",*payload);else fs::remove(f.root/"DATA/China_General.555");
    auto session=maps::load_stored_map_session(f.root,"Shore.map",maps::FootprintPolicy::EdgeByte4x4Preview);
    session.plan.logical_texture_bytes=charged;
    auto* surface=SDL_CreateSurface(420,400,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);
    openemperor::StoredGraphicsRenderer view(std::move(session.plan));view.initialize(renderer);
    view.set_debug_diagnostics(false);view.set_landscape_mode(openemperor::LandscapeDebugMode::Regenerated);
    openemperor::scene::Camera2D camera;camera.viewport_width=420;camera.viewport_height=400;camera.offset={200,-1380};
    SDL_SetRenderDrawColor(renderer,0,0,0,255);SDL_RenderClear(renderer);check(view.render(camera,{}),"full failed-shore render");
    check(pixel(renderer,200,240)==std::array<std::uint8_t,4>{103,94,78,255},reason);
    check(!view.hit_test({151.5,200.5},camera),"refused or damaged body cannot leave an invisible alpha hit");
    if(pier_expected)check(pixel(renderer,141,260)==std::array<std::uint8_t,4>{255,255,0,255},
                          "independent valid pier survives a refused/damaged/budget-exhausted body");
    check(view.plan().assets.front().status==maps::StoredStatus::Rendered,"foreign required Terrain is retained");
    view.shutdown();SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);
}
void negatives() {
    Fixture f;sources(f);const auto original=read(f.root/"DATA/China_General.sg3");
    const auto payload=read(f.root/"DATA/China_General.555");const auto raw=map_bytes();
    const auto body=40680U+340U*64U;
    auto metadata=original;u16(metadata,80U+139U*2U,0);
    refused(metadata,raw,&payload,"wrong containing group cannot select shore art");
    metadata=original;u16(metadata,80U+140U*2U,347);
    refused(metadata,raw,&payload,"incomplete runtime family retains fallback");
    for(unsigned variant:{1U,3U}) {
        auto different=raw;square(different,114,114,139+variant);
        different[std::size_t(maps::candidate_byte_logical_offset)+115U*228U+115U]=0x89;
        refused(original,different,&payload,"saved1/3 still rejects an unknown part state");
    }
    for(unsigned variant=4;variant<8;++variant) {
        auto different=raw;square(different,114,114,139+variant);metadata=original;
        // Even forged valid side-two metadata cannot turn a pier variant into
        // a land body. Variant4 also changes the independent authored pier.
        std::copy_n(original.begin()+static_cast<std::ptrdiff_t>(body),64,
                    metadata.begin()+static_cast<std::ptrdiff_t>(40680U+(340U+variant)*64U));
        refused(metadata,different,&payload,"pier-family variants cannot select a land body",variant!=4);
    }
    metadata=original;u32(metadata,body+16,1);refused(metadata,raw,&payload,"mirror reference remains unsupported");
    metadata=original;u16(metadata,body+30,1);refused(metadata,raw,&payload,"animation cannot become a static shore image");
    metadata=original;u16(metadata,body+50,1);refused(metadata,raw,&payload,"wrong format cannot bypass the decoder contract");
    metadata=original;u16(metadata,body+20,160);refused(metadata,raw,&payload,"wrong square width remains unsupported");
    metadata=original;u16(metadata,body+22,79);refused(metadata,raw,&payload,"insufficient square height remains unsupported");
    metadata=original;metadata[body+55]=1;refused(metadata,raw,&payload,"wrong size flag cannot use the side-two anchor");
    metadata=original;u32(metadata,body+8,3200);refused(metadata,raw,&payload,"wrong base length cannot complete a square image");
    metadata=original;u32(metadata,body,100000);refused(metadata,raw,&payload,"out-of-range body payload remains a complete fallback");
    metadata=original;u32(metadata,body+4,12801);auto damaged=payload;damaged[12800]=1;
    refused(metadata,raw,&damaged,"damaged Overlay rolls back the entire Base/Overlay/Combined group");
    auto missing=raw;u32(missing,std::size_t(maps::candidate_word_logical_offset)+(115U*228U+115U)*4U,0xc000);
    refused(original,missing,&payload,"incomplete map part group cannot draw a whole body");
    auto unknown=raw;unknown[std::size_t(maps::candidate_byte_logical_offset)+115U*228U+115U]=0x89;
    refused(original,unknown,&payload,"unknown part state cannot complete a shoreline footprint");
    auto offmap=raw;u32(offmap,std::size_t(maps::terrain_logical_offset)+(115U*228U+115U)*4U,0x8000c);
    refused(original,offmap,&payload,"an off-map member cannot complete a new historical footprint");
    auto highbit=raw;
    for(unsigned dy=0;dy<2;++dy)for(unsigned dx=0;dx<2;++dx)
        u32(highbit,std::size_t(maps::candidate_word_logical_offset)+((114U+dy)*228U+114U+dx)*4U,0x8000808b);
    refused(original,highbit,&payload,"unsupported packed high bit is not masked away");
    metadata=original;u32(metadata,16,339);
    refused(metadata,raw,&payload,"runtime limit cannot admit an out-of-range physical image",false);
    refused(Bytes{'b','a','d'},raw,&payload,"malformed optional archive preserves the loadable session",false);
    refused(original,raw,nullptr,"missing optional source retains complete diagnostics",false);
    // Exactly the existing Terrain and independent pier fit the remaining
    // authored quota. The new optional body cannot evict either of them.
    refused(original,raw,&payload,"RGBA budget preserves complete fallback",true,
            maps::stored_max_texture_bytes-93600U);
}
void asset_limit_and_legacy() {
    Fixture f;sources(f);
    auto raw=map_bytes();
    for(unsigned dy=0;dy<2;++dy)for(unsigned dx=0;dx<2;++dx)
        u32(raw,std::size_t(maps::terrain_logical_offset)+((114U+dy)*228U+114U+dx)*4U,0x80);
    u32(raw,std::size_t(maps::terrain_logical_offset)+(114U*228U+120U)*4U,0x80);
    write(f.root/"Shore.map",compressed(raw));
    const auto container=maps::EmperorContainer::open(f.root/"Shore.map");
    const auto map=maps::read_emperor_map(container,0);
    const auto candidates=maps::read_map_graphic_candidates(container,0);const maps::MapGeometry geometry{84};
    const auto base_registrations=maps::load_stored_archive_registrations(f.root,candidates,geometry,maps::StoredGraphicsProfile::Base);
    const auto before_shore=[&] {
        auto registrations=base_registrations;
        auto plan=maps::make_stored_graphics_plan(map,candidates,geometry,registrations,
            maps::FootprintPolicy::EdgeByte4x4Preview,maps::StoredGraphicsProfile::Base);
        maps::read_landscape_layers(plan,container,0);maps::add_regenerated_wall_registration(plan,registrations);
        maps::build_regenerated_map_render_plan(plan,registrations);
        maps::resolve_optional_stored_singletons(plan,registrations);
        return std::pair{std::move(plan),std::move(registrations)};
    };
    auto* surface=SDL_CreateSurface(420,400,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);
    auto [historical,historical_registrations]=before_shore();
    (void)historical_registrations;
    openemperor::StoredGraphicsRenderer old(std::move(historical));old.initialize(renderer);
    const auto baseline=maps::make_sandbox_buildable_mask(old.plan(),geometry);old.shutdown();
    check(baseline[114U*228U+114U]==0 && baseline[114U*228U+120U]==1,
          "literal old mask distinguishes an unresolved raw-grass image from ordinary rendered grass");
    auto [plan,registrations]=before_shore();
    const auto old_assets=plan.assets.size();
    const auto before_footprints=plan.footprints.size();
    maps::resolve_optional_stored_shore_footprints(plan,registrations);
    check(plan.footprints.size()==before_footprints+1 && plan.assets.size()==old_assets+1,
          "exact historical resolver adds one complete square and one physical asset");
    const auto selected_assets=plan.assets.size(),selected_footprints=plan.footprints.size();
    maps::resolve_optional_stored_shore_footprints(plan,registrations);
    check(plan.assets.size()==selected_assets && plan.footprints.size()==selected_footprints,
          "repeated load-time resolution is idempotent");
    openemperor::StoredGraphicsRenderer view(std::move(plan));view.initialize(renderer);
    check(maps::make_sandbox_buildable_mask(view.plan(),geometry)==baseline,
          "complete historical and current masks remain equal after actual decode");
    view.shutdown();SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);
    auto [limited,limited_registrations]=before_shore();
    const auto piers=limited.footprints.size();
    while(limited.assets.size()<maps::stored_max_assets)
        limited.assets.push_back({{},maps::StoredStatus::EmptyRecord,false,false,{},false});
    maps::resolve_optional_stored_shore_footprints(limited,limited_registrations);
    check(limited.assets.size()==maps::stored_max_assets && limited.footprints.size()==piers &&
          !limited.at({114,114})->footprint_index,"asset-count cap retains the original full diagnostic group");
    const auto diagnostic=limited.optional_saved_graphics.at(*limited.cell_by_storage[114U*228U+114U]);
    check(diagnostic.fallback.find("asset budget")!=std::string::npos,"asset budget refusal has a precise reason");
    auto disabled=maps::load_stored_map_session(f.root,"Shore.map",maps::FootprintPolicy::Disabled);
    check(!disabled.plan.at({114,114})->footprint_index && disabled.plan.at({115,116})->footprint_index,
          "disabled multi-cell convention still admits only the independent older singleton pier");
}
} // namespace
int main(){try {
    check(SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy") && SDL_Init(SDL_INIT_VIDEO),"SDL dummy init");
    positive();opposite_saved_variants();shared_instances();signed_marker_height();negatives();asset_limit_and_legacy();
    check(openemperor::StoredGraphicsRenderer::live_texture_count()==0,"shore texture shutdown");
    SDL_Quit();std::cout<<"Stored shoreline presentation tests passed\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';SDL_Quit();return 1;}}
