#include "maps/RegeneratedMapRenderPlan.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace openemperor::maps;
namespace {
void check(bool value,const char* message) {if(!value) throw std::runtime_error(message);}
WaterNeighborhood wet(unsigned mask) {return {bool(mask&1),bool(mask&2),bool(mask&4),bool(mask&8),
    bool(mask&16),bool(mask&32),bool(mask&64),bool(mask&128)};}
constexpr std::size_t count=stored_grid_width*stored_grid_height;
constexpr GridCell center{114,114};
constexpr auto cell_index=std::size_t(114)*stored_grid_width+114;
}
int main() {try {
    const auto isolated=match_water(wet(0),0,255);
    check(isolated && isolated->row==1 && isolated->orientation_offset==71 && isolated->variant==0,
        "isolated water matches land cardinal requirements, diagonals ignored");
    const auto channel=match_water(wet(0x44),0,255);
    check(channel && channel->row==6 && channel->orientation_offset==32 && channel->variant_count==2 && channel->variant==1,
        "one-cell east-west channel uses straight bank and modulo two");
    check(match_water(wet(0x11),0,2)->row==7,"north-south channel rotates the bank");
    check(match_water(wet(0x40),0,4)->orientation_offset==39 &&
        match_water(wet(0x40),1,4)->orientation_offset==38,"four orientation offsets retain original order");
    check(!match_water(wet(0xff),0,0) && !match_water(wet(0),4,0),"interior bypass and invalid orientation");
    check(match_water(wet(0x80),0,0)->row==1,"cardinal rule has priority over diagonal-only water");
    for(unsigned mask=0;mask<256;++mask) for(unsigned orientation=0;orientation<4;++orientation) {
        const auto a=match_water(wet(mask),orientation,193),b=match_water(wet(mask),orientation,193);
        check(a==b,"all neighbourhoods and orientations are deterministic");
        if(a) check(a->row>=1 && a->row<=46 && a->variant<a->variant_count &&
            unsigned(a->orientation_offset)+a->variant<72,"bounded shore family");
    }
    std::vector<std::uint32_t> terrain(count,0),objects(count,0);
    std::vector<std::uint8_t> variation(count,190),fertility(count,100);
    LandscapeSelectorInput input{terrain,objects,variation,fertility,false,0};
    terrain[cell_index]=4;
    auto selection=select_landscape(input,center);
    check(selection.group.value==0x605 && selection.variant==71,"isolated selector preserves table offset");
    for(int y=-1;y<=1;++y) for(int x=-1;x<=1;++x)
        terrain[std::size_t(114+y)*stored_grid_width+std::size_t(114+x)]=4;
    selection=select_landscape(input,center);
    check(selection.group.value==0x61c && selection.variant==22,"lake interior uses modulo24 plus parity");
    variation[cell_index]=191;
    check(select_landscape(input,center).variant==24,"odd modulo24 includes parity increment");
    terrain[cell_index]=0x4000004;
    check(select_landscape(input,center).variant==47,"special water boundary modulo48");
    for(int y=-1;y<=1;++y) for(int x=-1;x<=1;++x)
        terrain[std::size_t(114+y)*stored_grid_width+std::size_t(114+x)]=0x4000004;
    check(select_landscape(input,center).variant==71,"special interior offset48 plus modulo24");
    input.flood_context=true;
    check(select_landscape(input,center).variant==71,"active context has the same special interior phase");
    terrain.assign(count,0);terrain[cell_index]=0x100;
    check(select_landscape(input,center).family==LandscapeFamily::Ground &&
        select_landscape(input,center).evidence==SelectorEvidence::Unresolved,"flood never enters water selector");
    terrain[cell_index]=0x40000;
    check(select_landscape(input,center).family!=LandscapeFamily::Water,"marsh is separate from water");
    input.flood_context=false;terrain[cell_index]=0x80;variation[cell_index]=57;
    selection=select_landscape(input,center);
    check(selection.group.value==0x602 && selection.variant==39,"fertility100 uses bank four and modulo9");
    fertility[cell_index]=40;
    check(select_landscape(input,center).variant==12,"even fertility boundary and odd variation select lower bank");
    variation[cell_index]=58;
    check(select_landscape(input,center).variant==22,"even variation selects upper boundary bank");
    variation[cell_index]=6;
    check(select_landscape(input,center).evidence==SelectorEvidence::Unresolved,"low variation packing remains explicit");
    terrain[cell_index]=0x82;variation[cell_index]=57;
    check(select_landscape(input,center).group.value==0x606 && select_landscape(input,center).variant==1,
        "raw rock chooses rock resource group independent of fertile flag");
    terrain[cell_index+1]=2;terrain[cell_index+stored_grid_width]=2;terrain[cell_index+stored_grid_width+1]=2;
    check(select_landscape(input,center).evidence==SelectorEvidence::Unresolved,"possible large rock placement is not invented");
    terrain[cell_index]=0x81;objects[cell_index]=2;
    check(select_landscape(input,center).group.value==0x625,"object bit two only selects vegetation family inside vegetation branch");
    terrain[cell_index]=0x80;variation[cell_index]=200;
    check(select_landscape(input,center).group.value==0x602,"object bit two never becomes global bamboo meaning");
    terrain.assign(count,0);terrain[228]=4;
    check(!terrain_neighborhood(terrain,{227,0},4).e,"neighbour probe cannot wrap into another storage row");
    check(select_landscape(input,{228,0}).evidence==SelectorEvidence::Unresolved,"outside storage fails closed");
    // Independent boundary truth set from the pinned 32-row quarry semantics,
    // after first-row priority. It excludes sparse and unmatched neighborhoods.
    constexpr std::array<unsigned,84> quarry_boundaries{
        0x07,0x0f,0x17,0x1c,0x1d,0x1e,0x1f,0x27,0x2f,0x37,0x3c,0x3d,0x3e,0x3f,
        0x47,0x4f,0x5c,0x5e,0x67,0x6f,0x70,0x71,0x72,0x73,0x74,0x76,0x78,0x79,
        0x7a,0x7b,0x7c,0x7e,0x7f,0x87,0x8f,0x97,0x9c,0x9d,0x9e,0x9f,0xa7,0xaf,
        0xb7,0xbc,0xbd,0xbe,0xbf,0xc1,0xc3,0xc5,0xc7,0xc9,0xcb,0xcd,0xcf,0xd1,0xd3,
        0xd9,0xdb,0xdc,0xde,0xdf,0xe1,0xe3,0xe5,0xe7,0xe9,0xeb,0xed,0xef,0xf0,0xf1,
        0xf2,0xf3,0xf4,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe};
    constexpr std::array<int,8> nx{0,1,1,1,0,-1,-1,-1},ny{-1,-1,0,1,1,1,0,-1};
    constexpr std::array<unsigned,59> quarry_unmatched{
        0x05,0x0d,0x14,0x15,0x16,0x25,0x2d,0x34,0x35,0x36,0x41,0x43,0x45,0x49,0x4b,
        0x4d,0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5d,0x5f,
        0x61,0x63,0x65,0x69,0x6b,0x6d,0x75,0x77,0x7d,0x85,0x8d,0x94,0x95,0x96,0xa5,
        0xad,0xb4,0xb5,0xb6,0xd0,0xd2,0xd4,0xd5,0xd6,0xd7,0xd8,0xda,0xdd,0xf5};
    terrain.assign(count,0);objects.assign(count,0);
    terrain[cell_index]=0x20000;
    for(unsigned mask=0;mask<256;++mask) {
        for(unsigned d=0;d<8;++d)
            terrain[std::size_t(114+ny[d])*stored_grid_width+std::size_t(114+nx[d])]=
                mask&(1U<<d) ? 0x20080U:0;
        const bool boundary=std::find(quarry_boundaries.begin(),quarry_boundaries.end(),mask)!=quarry_boundaries.end();
        for(unsigned orientation=0;orientation<4;++orientation) for(unsigned v=0;v<256;++v) {
            input.orientation=orientation;variation[cell_index]=static_cast<std::uint8_t>(v);
            selection=select_landscape(input,center);
            check((selection.evidence==SelectorEvidence::Verified)==boundary,
                "only the observed quarry boundary truth set has verified identity");
            if(boundary) check(selection.group.value==0x603 && selection.variant==(v&7U),
                "quarry boundary normalization uses dry ground and original variation, not row modulo");
            else if(std::find(quarry_unmatched.begin(),quarry_unmatched.end(),mask)!=quarry_unmatched.end())
                check(selection.evidence==SelectorEvidence::Unresolved && selection.group.value==0,
                    "unmatched quarry topology cannot inherit another cell's selector state");
            else check(selection.evidence==SelectorEvidence::Preview &&
                selection.group.value==0x628 && selection.variant==v%6,
                "bounded quarry reset-depth preview uses six variants and never claims restored context");
        }
    }
    input.orientation=0;variation[cell_index]=155;
    check(select_landscape(input,center).variant==5,"full quarry interior retains modulo six");
    terrain[cell_index]=0x20080;objects[cell_index]=0xffffffff;fertility[cell_index]=255;
    input.flood_context.reset();
    check(select_landscape(input,center).variant==5 && select_landscape(input,center).group.value==0x628,
        "quarry preview does not infer state from objects, fertility or unknown transient context");
    terrain[cell_index]|=0x80000;
    check(select_landscape(input,center).evidence==SelectorEvidence::Unresolved,
        "quarry cannot bypass original early exclusions");
    input.flood_context=false;
    RuntimeArchiveLayout layout;layout.slot=3;layout.verified_registration=true;layout.sg3_version=213;
    layout.runtime_image_count=300;layout.groups={{0,80,20,20},{1,82,92,92}};
    std::map<std::uint32_t,GroupRegistration> groups{{3,{&layout}}};
    check(resolve_landscape_variant({0x601},71,groups)->value==3*0x4000+91,
        "large variation resolved through bounded resource group, not physical constant");
    check(!resolve_landscape_variant({0x601},72,groups) && !resolve_landscape_variant({0x80000601},0,groups) &&
        !resolve_landscape_variant({0x600},0,groups),"group boundary, high bit and missing position fail closed");
    // No original bytes: fake metadata is enough to test load-plan isolation,
    // physical dedupe and independence from the discarded saved identity.
    StoredArchiveRegistration registration;registration.layout=layout;
    auto& l=*registration.layout;l.system_record_skip=0;l.first_physical_record=1;
    l.image_capacity=101;l.runtime_image_count=100;l.groups={{0,80,1,1},{1,82,1,1},{2,84,1,1},{3,86,1,1},{4,88,1,1},{5,90,73,73}};
    registration.catalog.emplace();
    for(unsigned n=0;n<101;++n) {
        openemperor::assets::AssetRecord r;r.id={"DATA/synthetic.sg3",n};r.image_type=30;r.width=78;r.height=40;
        r.uncompressed_length=3200;r.data_length=3200;r.isometric_size_flag=1;
        r.payload_in_bounds=true;r.decoder_supported=true;r.color_decoder_supported=true;
        r.color_bounds=openemperor::assets::AssetRangeStatus::InBounds;
        registration.catalog->records.push_back(r);
    }
    StoredArchiveRegistrations registrations{{3,registration}};
    StoredGraphicsPlan plan;plan.landscape_layers_available=true;
    plan.raw_terrain.assign(count,0);plan.raw_terrain[cell_index]=4;plan.raw_objects.assign(count,0);
    plan.variation_bytes.assign(count,190);plan.fertility_bytes.assign(count,100);plan.raw_saved_ids.assign(count,123);
    plan.raw_candidate_bytes.assign(count,64);plan.height_bytes.assign(count,3);
    StoredCell c;c.storage=center;c.cell_index=cell_index;c.slot=3;c.stored_id=123;c.footprint_index=0;
    c.asset_index=0;c.status=StoredStatus::Rendered;plan.cells.push_back(c);
    plan.assets.push_back({registration.catalog->records[1],StoredStatus::Rendered,true,true,{}});
    PlacedFootprint f;f.id=0;f.origin=center;f.cell_indices={0};plan.footprints.push_back(f);
    const auto original_ids=plan.raw_saved_ids;
    build_regenerated_map_render_plan(plan,registrations);
    check(plan.regenerated && plan.regenerated->cells[0].asset_index && plan.assets.size()==2,
        "load plan resolves and eagerly declares one distinct new physical asset");
    check(plan.raw_saved_ids==original_ids && plan.cells[0].stored_id==123 && plan.cells[0].status==StoredStatus::Rendered &&
        plan.footprints[0].asset_index==0,"historical geometry/status/buildability inputs remain exact");
    auto second=plan;second.regenerated.reset();second.assets.resize(1);
    second.raw_saved_ids.assign(count,999999);second.cells[0].stored_id=999999;
    second.raw_candidate_bytes.assign(count,7);second.height_bytes.assign(count,255);
    build_regenerated_map_render_plan(second,registrations);
    check(second.regenerated->cells[0].graphic && plan.regenerated->cells[0].graphic &&
        second.regenerated->cells[0].graphic->value==plan.regenerated->cells[0].graphic->value &&
        second.regenerated->cells[0].selection==plan.regenerated->cells[0].selection,
        "identity independent of saved IDs, candidate byte and height; elevation remains separate");
    const auto published=plan.regenerated;build_regenerated_map_render_plan(plan,registrations);
    check(plan.regenerated==published && plan.assets.size()==2,"published plan never rebuilt or duplicated");
    auto quarry_registration=registration;
    auto& quarry_layout=*quarry_registration.layout;
    quarry_layout.groups.resize(41);
    for(unsigned i=6;i<39;++i) quarry_layout.groups[i]={i,0,1,1};
    quarry_layout.groups[39]={39,0,80,80}; // 628 has exactly six variants.
    quarry_layout.groups[40]={40,0,86,86};
    const StoredArchiveRegistrations quarry_registrations{{3,quarry_registration}};
    auto quarry_plan=plan;quarry_plan.regenerated.reset();quarry_plan.assets.resize(1);
    quarry_plan.raw_terrain.assign(count,0);
    for(int y=-1;y<=1;++y) for(int x=-1;x<=2;++x)
        quarry_plan.raw_terrain[std::size_t(114+y)*stored_grid_width+std::size_t(114+x)]=0x20000;
    quarry_plan.variation_bytes.assign(count,155);
    auto c2=c;c2.storage.x++;c2.cell_index++;c2.footprint_index=1;c2.stored_id=999999;
    quarry_plan.cells.push_back(c2);auto f2=f;f2.id=1;f2.origin=c2.storage;f2.cell_indices={1};
    quarry_plan.footprints.push_back(f2);
    const auto quarry_before=quarry_plan;
    build_regenerated_map_render_plan(quarry_plan,quarry_registrations);
    const auto& quarry_cells=quarry_plan.regenerated->cells;
    check(quarry_cells[0].asset_index && quarry_cells[0].asset_index==quarry_cells[1].asset_index &&
        quarry_plan.assets.size()==2 && quarry_plan.assets[*quarry_cells[0].asset_index].record.id.image_index==86,
        "quarry group resolves bounded physical identity and deduplicates across different saved IDs");
    check(quarry_plan.raw_terrain==quarry_before.raw_terrain && quarry_plan.raw_objects==quarry_before.raw_objects &&
        quarry_plan.raw_saved_ids==quarry_before.raw_saved_ids && quarry_plan.raw_candidate_bytes==quarry_before.raw_candidate_bytes &&
        quarry_plan.height_bytes==quarry_before.height_bytes && quarry_plan.footprints[0].asset_index==0 &&
        quarry_plan.footprints[1].asset_index==0 && quarry_plan.cells[1].stored_id==999999,
        "quarry presentation preserves raw map, historical ownership and all buildability inputs");
    auto missing_quarry=quarry_before;
    build_regenerated_map_render_plan(missing_quarry,registrations);
    check(!missing_quarry.regenerated->cells[0].asset_index && missing_quarry.assets.size()==1,
        "missing quarry group preserves historical preview without a substitute sprite");
    auto narrow=quarry_registrations;narrow.at(3).layout->groups[40].local_base=85;
    auto rejected_quarry=quarry_before;build_regenerated_map_render_plan(rejected_quarry,narrow);
    check(!rejected_quarry.regenerated->cells[0].asset_index && rejected_quarry.assets.size()==1,
        "quarry variant cannot cross the exact resource-group boundary");
    auto mirrored=quarry_registrations;mirrored.at(3).catalog->records[86].horizontal_mirror_offset=1;
    rejected_quarry=quarry_before;build_regenerated_map_render_plan(rejected_quarry,mirrored);
    check(!rejected_quarry.regenerated->cells[0].asset_index && rejected_quarry.assets.size()==1,
        "unsupported quarry geometry retains historical graphics atomically");
    std::cout<<"landscape selectors passed\n";return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}}
