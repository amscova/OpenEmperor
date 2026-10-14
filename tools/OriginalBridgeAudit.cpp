// Explicit read-only corpus diagnosis. No World, inferred original crossing,
// renderer, source edits, or permission exemptions are created here.
#include "maps/MapCatalog.h"
#include "maps/MapRulesCheck.h"
#include "maps/SandboxPlacement.h"
#include "maps/MapGraphicCandidates.h"
#include "maps/LandscapeProvenance.h"
#include "assets/Sg3ImageLoader.h"
#include "maps/RuntimeArchiveLayout.h"
#include <nlohmann/json.hpp>
#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <set>

namespace fs=std::filesystem;
namespace m=openemperor::maps;
namespace s=openemperor::simulation;
using J=nlohmann::json;
namespace {
constexpr int width=int(m::stored_grid_width),height=int(m::stored_grid_height);
constexpr std::array<s::Cell,4> directions={s::Cell{1,0},{0,1},{-1,0},{0,-1}};
bool inside(s::Cell p) {return p.x>=0 && p.y>=0 && p.x<width && p.y<height;}
std::size_t index(s::Cell p) {return std::size_t(p.y)*width+std::size_t(p.x);}
s::Cell add(s::Cell p,s::Cell d) {return {p.x+d.x,p.y+d.y};}
J cell(s::Cell p) {return J::array({p.x,p.y});}
bool beneath(const fs::path& a,const fs::path& b) {
    auto i=a.begin();for(auto j=b.begin();j!=b.end();++j,++i)if(i==a.end()||*i!=*j)return false;return true;
}
void write(const fs::path& path,const J& value) {
    std::ofstream output(path);if(!output || !(output<<value.dump(2)<<'\n'))throw std::runtime_error("audit write failed");
}
}
int main(int argc,char** argv) {
    try {
        if(argc!=3)throw std::invalid_argument("usage: original-bridge-audit DATA FRESH-.local-ROOT");
        const auto data=fs::canonical(argv[1]),output=fs::weakly_canonical(argv[2]);
        const auto local=fs::canonical(fs::current_path()/".local");
        if(!beneath(output,local)||output==local||beneath(output,data)||
           (fs::exists(output)&&!fs::is_empty(output)))throw std::invalid_argument("fresh ignored output root required");
        fs::create_directories(output);
        J report={{"format","openemperor-original-bridge-audit-v1"},{"maps",J::array()},
            {"evidence","RAW MAP FACT / OPENEMPEROR diagnostic candidate; no original bridge verified"},
            {"verified_original_bridges",0},{"saved_graphics_authority",false},
            {"graphic_provenance","candidate word at logical1535 + storage_index*4; byte at209471 + index"}};
        std::size_t complete=0,prepared=0;std::map<std::uint32_t,std::size_t> water_road_graphics;
        for(const auto& entry:m::discover_standalone_maps(data).entries)if(entry.map_profile) {
            const auto container=m::EmperorContainer::open(m::resolve_map_path(data,entry.relative_path));
            const auto map=m::read_emperor_map(container,0);const m::MapGeometry geometry{map.declared_map_size};
            const auto entities=m::read_original_map_entities(container,0);++complete;
            const auto heights=container.read_range(0,m::landscape_height_offset,width*height);
            const auto graphics=m::read_map_graphic_candidates(container,0);
            J row={{"map",entry.relative_path.generic_string()},{"sha256",m::map_input_sha256(container)},
                {"complete_manager_records",entities.records.size()},{"water_signatures",J::object()},
                {"categories",J::object()},{"water_components",J::array()},{"spans",J::array()},
                {"ferry_sources",J::array()}};
            std::shared_ptr<const s::MapPermissions> policy;
            try {policy=m::prepare_sandbox_map_permissions(map,geometry,std::vector<std::uint8_t>(width*height,1),entities,heights,2);++prepared;}
            catch(const std::exception& error) {row["policy_error"]=error.what();}
            std::set<std::size_t> ferry;
            for(const auto& e:entities.records)if(e.active() && e.entity_class==m::OriginalEntityClass::Ferry) {
                s::Cell p{int(e.local_x)+int(geometry.border),int(e.local_y)+int(geometry.border)};
                if(inside(p))ferry.insert(index(p));
                row["ferry_sources"].push_back({{"id",e.serialized_original_id},{"type",e.type},{"origin",cell(p)},
                    {"side",e.footprint_side},{"reference",e.serialized_cell_reference},{"logical_record",e.provenance.logical_record_offset}});
            }
            auto onmap=[&](s::Cell p){return inside(p)&&geometry.contains({unsigned(p.x),unsigned(p.y)})&&!(map.terrain_raw.values[index(p)]&0x80000U);};
            auto raw_water=[&](s::Cell p){return onmap(p)&&(map.terrain_raw.values[index(p)]&4U);};
            // Candidate-only exact signature, subsequently reviewed against
            // this census. Never accepts mixed road/water, shoreline features,
            // original entities, markers, gates or raw object bits.
            auto water=[&](s::Cell p){return policy && onmap(p) && map.terrain_raw.values[index(p)]==4U &&
                map.objects_raw.values[index(p)]==0 && !policy->protected_original(p) && !ferry.contains(index(p));};
            auto bank=[&](s::Cell p){return policy&&onmap(p)&&policy->road_allowed(p)&&!policy->protected_original(p)&&!policy->fixed_gate_at(p);};
            std::vector<bool> visited(width*height,false);
            for(int y=0;y<height;++y)for(int x=0;x<width;++x) {
                const s::Cell p{x,y};if(!raw_water(p))continue;const auto at=index(p);
                const auto terrain=map.terrain_raw.values[at],objects=map.objects_raw.values[at];
                const auto key=std::to_string(terrain)+":"+std::to_string(objects)+":"+std::to_string(int(std::int8_t(heights[at])));
                auto& sig=row["water_signatures"][key];if(sig.is_null())sig={{"count",0},{"examples",J::array()},{"saved_graphic_words",J::object()}};
                sig["count"]=sig["count"].get<std::size_t>()+1;
                const auto graphic=std::to_string(graphics.candidate_word_layer[at]);auto& count=sig["saved_graphic_words"][graphic];count=count.is_null()?1:count.get<std::size_t>()+1;
                if(terrain&0x40U)++water_road_graphics[graphics.candidate_word_layer[at]];
                if(sig["examples"].size()<8) {
                    J neighbors=J::array();for(const auto d:directions) {const auto q=add(p,d);if(inside(q))neighbors.push_back({{"cell",cell(q)},{"terrain",map.terrain_raw.values[index(q)]},{"objects",map.objects_raw.values[index(q)]},{"height",int(std::int8_t(heights[index(q)]))}});}
                    sig["examples"].push_back({{"cell",cell(p)},{"graphic_word",graphics.candidate_word_layer[at]},{"graphic_byte",graphics.candidate_byte_layer[at]},
                        {"protected",policy?J(policy->protected_original(p)):J(nullptr)},{"neighbors",neighbors}});
                }
                const char* category=ferry.contains(at)?"FERRY_LANDING":(terrain&0x40U)?"WATER_WITH_ROAD_BIT":water(p)?"NORMAL_WATER":"UNSUPPORTED_SPECIAL_TERRAIN";
                auto& count_category=row["categories"][category];count_category=count_category.is_null()?1:count_category.get<std::size_t>()+1;
                if(!visited[at]) {
                    std::queue<s::Cell> pending;pending.push(p);visited[at]=true;std::size_t size=0,land_edges=0;
                    while(!pending.empty()) {const auto q=pending.front();pending.pop();++size;
                        for(const auto d:directions) {const auto n=add(q,d);if(raw_water(n)) {if(!visited[index(n)]){visited[index(n)]=true;pending.push(n);}}else if(bank(n))++land_edges;}}
                    row["water_components"].push_back({{"origin",cell(p)},{"cells",size},{"normal_land_edges",land_edges}});
                }
            }
            row["categories"]["POSSIBLE_BRIDGE"]=row["categories"].value("WATER_WITH_ROAD_BIT",std::size_t(0));
            row["categories"]["VERIFIED_ORIGINAL_BRIDGE"]=0;
            row["possible_bridge_semantics"]="Diagnostic water+road hint only; no original bridge contract or traversal admitted";
            if(policy)for(int y=0;y<height;++y)for(int x=0;x<width;++x) {
                const s::Cell a{x,y};if(!bank(a))continue;
                for(const auto d:std::array<s::Cell,2>{s::Cell{1,0},s::Cell{0,1}}) {
                    auto p=add(a,d);J deck=J::array();while(water(p)&&deck.size()<8){deck.push_back(cell(p));p=add(p,d);}
                    if(deck.empty()||!bank(p)||policy->cell_height(a)!=policy->cell_height(p))continue;
                    row["spans"].push_back({{"bank_a",cell(a)},{"bank_b",cell(p)},{"water_cells",deck},
                        {"axis",d.x?"storage_x":"storage_y"},{"deck_height",policy->cell_height(a)},
                        {"bank_a_original_road",policy->original_road(a)},{"bank_b_original_road",policy->original_road(p)}});
                }
            }
            std::cerr<<entry.relative_path.generic_string()<<": "<<row["spans"].size()<<" conservative spans\n";
            report["maps"].push_back(std::move(row));
        }
        report["complete_managers"]=complete;report["prepared"]=prepared;
        // Inspect the already evidenced slot-3 metadata before deciding that
        // a saved word names bridge artwork. No image bytes are decoded/copied.
        const auto terrain=openemperor::assets::read_sg3_archive(data/"DATA/China_Terrain.sg3");
        const auto layout=m::build_runtime_archive_layout(3,terrain);
        if(!layout)throw std::runtime_error("required terrain runtime layout unavailable");
        J sprites={{"archive","DATA/China_Terrain.sg3"},{"registration",3},{"verified_bridge_identity",false},
            {"method","exact saved word / verified runtime metadata; saved graphics never establish bridge authority"},{"records",J::array()}};
        for(const auto& [word,count]:water_road_graphics) {
            J record={{"saved_word",word},{"occurrences",count},{"slot",word/16384},{"local",word%16384}};
            if(word/16384==3)if(const auto physical=layout->physical_record_for_local(word%16384)) {
                const auto& image=terrain.images.at(*physical);record["physical_record"]=*physical;record["width"]=image.width;record["height"]=image.height;
                record["type"]=image.image_type;record["size_flag"]=image.isometric_size_flag;record["data_length"]=image.data_length;
                record["group_id"]=image.group_id;
            }
            sprites["records"].push_back(std::move(record));
        }
        write(output/"water-road-sprite-metadata.json",sprites);write(output/"water-corpus.json",report);
        std::cout<<complete<<" complete / "<<prepared<<" prepared\n";return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
