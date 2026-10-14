// Read-only, pre-implementation elevation census. No ramp/slope semantics inferred.
#include "maps/MapCatalog.h"
#include "maps/MapRulesCheck.h"
#include "maps/SandboxPlacement.h"
#include "maps/MapGraphicCandidates.h"
#include "maps/LandscapeProvenance.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <map>
namespace fs=std::filesystem;
namespace m=openemperor::maps;
namespace s=openemperor::simulation;
using J=nlohmann::json;
int main(int argc,char** argv) {try {
    if(argc!=3)throw std::invalid_argument("usage: original-height-audit DATA FRESH-.local-ROOT");
    const auto data=fs::canonical(argv[1]),out=fs::weakly_canonical(argv[2]);
    const auto local=fs::canonical(".local");
    const auto relative=out.lexically_relative(local);
    if(relative.empty()||relative=="."||*relative.begin()==".."||
       out.string().starts_with(data.string()+"/")||(fs::exists(out)&&!fs::is_empty(out)))
        throw std::invalid_argument("fresh isolated ignored output root required");
    fs::create_directories(out);
    constexpr int w=int(m::stored_grid_width),h=int(m::stored_grid_height);
    auto at=[](s::Cell p){return std::size_t(p.y*w+p.x);};
    auto cell=[](s::Cell p){return J::array({p.x,p.y});};
    J report={{"format","openemperor-original-height-audit-v1"},{"maps",J::array()},
        {"evidence","ORIGINAL HEIGHT FACT / UNRESOLVED ORIGINAL SLOPE"},
        {"verified_original_ramps",0},{"height_offset",m::landscape_height_offset},
        {"encoding","signed int8; projection minus height * 40; no normalization"}};
    std::map<std::string,std::size_t> total;std::size_t prepared=0;
    for(const auto& entry:m::discover_standalone_maps(data).entries)if(entry.map_profile) {
        const auto c=m::EmperorContainer::open(m::resolve_map_path(data,entry.relative_path));
        const auto map=m::read_emperor_map(c,0);const m::MapGeometry g{map.declared_map_size};
        const auto entities=m::read_original_map_entities(c,0);
        const auto heights=c.read_range(0,m::landscape_height_offset,w*h);
        const auto graphics=m::read_map_graphic_candidates(c,0);
        J row={{"map",entry.relative_path.generic_string()},{"sha256",m::map_input_sha256(c)},
            {"counts",J::object()},{"land_delta1",J::array()},{"road_height_examples",J::array()}};
        std::shared_ptr<const s::MapPermissions> p;
        try {p=m::prepare_sandbox_map_permissions(map,g,std::vector<std::uint8_t>(w*h,1),entities,heights,3);++prepared;}
        catch(const std::exception& error){row["policy_error"]=error.what();}
        auto inside=[&](s::Cell q){return q.x>=0&&q.y>=0&&q.x<w&&q.y<h&&g.contains({unsigned(q.x),unsigned(q.y)})&&!(map.terrain_raw.values[at(q)]&0x80000U);};
        auto facts=[&](s::Cell q) {
            const auto i=at(q);J f={{"cell",cell(q)},{"terrain",map.terrain_raw.values[i]},
                {"objects",map.objects_raw.values[i]},{"height",int(std::int8_t(heights[i]))},
                {"diamond",g.contains({unsigned(q.x),unsigned(q.y)})},{"offmap",bool(map.terrain_raw.values[i]&0x80000U)},
                {"saved_graphic",graphics.candidate_word_layer[i]},
                {"slope_evidence","saved height/terrain/image only; original placement/restore/traversal unverified"}};
            if(p) {f["protected"]=p->protected_original(q);f["road_allowed"]=p->road_allowed(q);
                f["building_allowed_probe_all_ones"]=p->building_allowed(q);f["original_road"]=p->original_road(q);
                f["road_classification"]=s::original_road_classification_name(p->original_road_classification(q));
                f["gate"]=p->fixed_gate_at(q)?J(p->fixed_gate_at(q)->value):J(nullptr);
            }return f;
        };
        auto count=[&](const std::string& name){++total[name];row["counts"][name]=row["counts"].value(name,std::size_t(0))+1;};
        for(int y=0;y<h;++y)for(int x=0;x<w;++x) {
            const s::Cell a{x,y};if(!inside(a))continue;
            for(const auto d:std::array<s::Cell,2>{s::Cell{1,0},s::Cell{0,1}}) {
                const s::Cell b{x+d.x,y+d.y};if(!inside(b))continue;
                const auto ta=map.terrain_raw.values[at(a)],tb=map.terrain_raw.values[at(b)];
                const int delta=std::abs(int(std::int8_t(heights[at(a)]))-int(std::int8_t(heights[at(b)])));
                const bool roads=p&&p->original_road(a)&&p->original_road(b);
                if(roads)count(delta==0?"normal_road_equal":delta==1?"normal_road_delta1":"normal_road_delta_gt1");
                if((ta&0x40U)||(tb&0x40U)) {
                    if(delta)count("road_at_height_edge");
                    if((ta&0x200U)||(tb&0x200U))count("road_near_raw_elevation_hint_unverified");
                    if((ta!=0x80U&&ta!=0xc0U)||(tb!=0x80U&&tb!=0xc0U))count("road_near_special_terrain");
                }
                const bool land=p&&p->road_allowed(a)&&p->road_allowed(b)&&!p->protected_original(a)&&!p->protected_original(b)&&!p->fixed_gate_at(a)&&!p->fixed_gate_at(b);
                if(land)count(delta==0?"legal_land_equal":delta==1?"legal_land_delta1":"legal_land_delta_gt1");
                if((land&&delta==1)||(((ta|tb)&0x40U)&&delta&&row["road_height_examples"].size()<64)) {
                    J nearby=J::array();bool nearwater=false;
                    for(int dy=-1;dy<=1;++dy)for(int dx=-1;dx<=1;++dx) {
                        const s::Cell q{x+dx,y+dy};if(!inside(q))continue;
                        if(p&&p->protected_original(q))nearby.push_back(facts(q));
                        nearwater|=p&&p->bridge_water(q);
                    }
                    J e={{"a",facts(a)},{"b",facts(b)},{"delta",delta},{"nearby_structures",nearby},
                        {"near_normal_bridge_water",nearwater},{"edge_blocker",p?s::build_blocker_name(p->transport_edge_blocker(a,b)):"policy unavailable"}};
                    if(land&&delta==1) {row["land_delta1"].push_back(e);if(nearwater)count("land_delta1_near_bridge_water");}
                    else row["road_height_examples"].push_back(e);
                }
            }
        }
        report["maps"].push_back(std::move(row));
    }
    report["totals"]=total;report["prepared"]=prepared;report["complete"]=report["maps"].size();
    std::ofstream f(out/"height-corpus.json");if(!(f<<report.dump(2)<<'\n'))throw std::runtime_error("audit write failed");
    std::cout<<report["complete"]<<" complete / "<<prepared<<" prepared; "<<J(total).dump()<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
