// On-demand real-map paid-command replay. Generated saves and proprietary
// facts stay in a fresh ignored root; this is separate from normal App evidence.
#include "maps/MapRulesCheck.h"
#include "maps/SandboxPlacement.h"
#include "maps/StoredMapSession.h"
#include "renderer/StoredGraphicsRenderer.h"
#include "persistence/SandboxSave.h"
#include "simulation/RoadConnectivity.h"

#include <SDL3/SDL.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>

namespace oe=openemperor;
namespace sim=oe::simulation;
namespace maps=oe::maps;
namespace save=oe::persistence;
namespace fs=std::filesystem;
using J=nlohmann::json;
namespace {
void require(bool ok,const std::string& reason) { if (!ok) throw std::runtime_error(reason); }
bool beneath(const fs::path& child,const fs::path& parent) {
    auto c=child.begin();
    for (auto p=parent.begin();p!=parent.end();++p,++c)
        if (c==child.end() || *c!=*p) return false;
    return true;
}
J point(sim::Cell cell) {return J::array({cell.x,cell.y});}
J points(const std::vector<sim::Cell>& cells) {
    J result=J::array();for (const auto cell:cells) result.push_back(point(cell));return result;
}
struct Sdl {
    SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;
    Sdl() {
        SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy");
        require(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
        window=SDL_CreateWindow("Private original-road fixture",1280,720,SDL_WINDOW_HIDDEN);
        if (!window) {SDL_Quit();throw std::runtime_error(SDL_GetError());}
        renderer=SDL_CreateRenderer(window,"software");
        if (!renderer) {const std::string error=SDL_GetError();SDL_DestroyWindow(window);SDL_Quit();
            throw std::runtime_error(error);}
    }
    ~Sdl() {SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();}
};
struct Loaded {
    maps::ParsedEmperorMap map;
    maps::MapGeometry geometry;
    std::unique_ptr<oe::StoredGraphicsRenderer> background;
    std::vector<std::uint8_t> legacy;
    std::shared_ptr<const sim::MapPermissions> policy,old_policy;
    Loaded(maps::ParsedEmperorMap value):map(std::move(value)),geometry(map.declared_map_size) {}
};
Loaded load(const fs::path& data,const fs::path& relative,SDL_Renderer* renderer,const std::string& sha) {
    auto session=maps::load_stored_map_session(data,relative,maps::FootprintPolicy::EdgeByte4x4Preview,
        maps::StoredGraphicsProfile::Slot8,maps::GreatWallPresentationMode::Automatic,sha);
    Loaded result(std::move(session.map));
    result.background=std::make_unique<oe::StoredGraphicsRenderer>(std::move(session.plan));
    result.background->initialize(renderer);
    result.legacy=maps::make_sandbox_buildable_mask(result.background->plan(),result.geometry);
    result.policy=maps::load_sandbox_map_permissions(result.map,result.background->plan(),result.geometry,
        result.legacy,2,sha);
    result.old_policy=maps::load_sandbox_map_permissions(result.map,result.background->plan(),result.geometry,
        result.legacy,1,sha);
    return result;
}
J raw(const Loaded& map,const sim::World& world,sim::Cell cell) {
    require(world.in_bounds(cell),"Raw proof cell outside bounded storage grid");
    const auto index=static_cast<std::size_t>(cell.y)*maps::stored_grid_width+static_cast<std::size_t>(cell.x);
    return { {"cell",point(cell)},{"raw_terrain",map.map.terrain_raw.values[index]},
        {"raw_objects",map.map.objects_raw.values[index]},
        {"signed_height",map.policy->cell_height(cell)},
        {"off_map",bool(map.map.terrain_raw.values[index]&0x80000U)},
        {"legacy_buildability",map.legacy[index]},
        {"original_road",world.original_road(cell)},
        {"classification",sim::original_road_classification_name(map.policy->original_road_classification(cell))},
        {"transport_source",sim::transport_cell_source_name(world.transport_source(cell))},
        {"traversable",world.transport_cell(cell)},
        {"protected_original",map.policy->protected_original(cell)},
        {"fixed_passage",world.fixed_passage(cell)},
        {"sandbox_object",static_cast<unsigned>(world.object_at(cell))} };
}
J facts(const sim::World& world) {
    J result={{"tick",world.ticks()},{"rule",world.rule_version()},
        {"policy",world.map_permissions()->policy_version()},
        {"funds",world.treasury()},{"spent",world.construction_spent_total()},
        {"command_sequence",world.command_sequence()},{"road_revision",world.road_revision()},
        {"roads_placed",world.roads_placed_total()},{"roads_removed",world.roads_removed_total()},
        {"mutable_roads",world.snapshot().roads.size()},{"population",world.total_population()},
        {"workers_available",world.workforce_supply()},{"workers_assigned",world.workforce_used()},
        {"clay_extracted",world.clay_extracted_total()},{"pottery_produced",world.pottery_completed_total()},
        {"buildings",J::array()},{"couriers",J::array()}};
    for (const auto& b:world.buildings()) result["buildings"].push_back({
        {"id",static_cast<unsigned>(b.id)},{"kind",static_cast<unsigned>(b.kind)},
        {"origin",point(b.cell)},{"input_clay",b.input_clay},{"output",b.output},
        {"reserved_incoming",b.reserved_incoming},{"workers",world.workers_assigned(b.id)}});
    for (const auto& c:world.couriers()) {
        J courier={{"id",static_cast<unsigned>(c.id)},{"role",static_cast<unsigned>(c.role)},
            {"owner",static_cast<unsigned>(c.owner)},{"target",static_cast<unsigned>(c.target)},
            {"phase",sim::courier_phase_name(c.phase)},{"cargo",c.cargo},{"reservation",c.reserved},
            {"path",points(c.path)},{"vertex",c.path_vertex},{"edge_progress",c.edge_progress},
            {"dispatch",sim::courier_dispatch_status_name(world.courier_dispatch_status(c.id).status)}};
        if (const auto p=world.courier_position(c.id)) courier["position"]={p->x,p->y};
        result["couriers"].push_back(std::move(courier));
    }
    return result;
}
void invariants(const sim::World& world) {
    require(world.navigation_valid() && world.production_balance_valid() && world.food_balance_valid() &&
        world.city_economy_valid() && world.population_valid() && world.service_state_valid() &&
        world.fire_state_valid() && world.health_state_valid(),"Actual original-road replay invariant failed");
}
void write_world(const fs::path& path,const fs::path& data,const fs::path& relative,
                 const Loaded& map,const sim::World& world) {
    const auto document=save::make_document(data,relative,map.legacy,world);
    save::write_save(path,document,data,map.legacy);
    const auto read=save::read_save(path);
    auto restored=save::restore_save(read,data,map.legacy,world.map_permissions());
    const auto expected_schema=world.rule_version()==4 ? 20U:19U;
    require(read.source_schema_version==expected_schema && read.world==document.world &&
        read.map_permissions_policy_version==world.map_permissions()->policy_version() &&
        restored.snapshot()==world.snapshot(),"Complete original-road save/read/restore mismatch");
}
constexpr std::array<sim::Cell,4> neighbors{{{0,-1},{-1,0},{1,0},{0,1}}};
bool overlaps(const std::vector<sim::Cell>& a,const std::vector<sim::Cell>& b) {
    return std::any_of(a.begin(),a.end(),[&](sim::Cell p) {return std::find(b.begin(),b.end(),p)!=b.end();});
}
std::vector<sim::Cell> footprint(sim::Cell origin) {
    return sim::building_footprint_cells(sim::RulesProfile::CityV16,4,sim::Object::Household,origin);
}
bool legal_footprint(const sim::World& empty,sim::Cell origin,const std::vector<sim::Cell>& occupied) {
    const auto cells=footprint(origin);
    return !overlaps(cells,occupied) && std::all_of(cells.begin(),cells.end(),[&](sim::Cell cell) {
        return empty.buildable(cell) && empty.object_at(cell)==sim::Object::Empty;
    });
}
std::vector<sim::Cell> original_entrances(const sim::World& empty,sim::Cell origin) {
    std::vector<sim::Cell> result;
    for (const auto from:footprint(origin)) for (const auto d:neighbors) {
        const sim::Cell to{from.x+d.x,from.y+d.y};
        if (empty.original_road(to) && empty.map_permissions()->transport_edge_allowed(from,to))
            result.push_back(to);
    }
    return result;
}
struct Layout {sim::Cell clay{},pottery{},house1{},house2{};bool user_geometry=false;};
std::optional<Layout> layout_at(const sim::World& empty,sim::Cell clay,sim::Cell pottery,bool user_geometry,
                              bool require_shared_road=true) {
    if (!legal_footprint(empty,clay,{}) || !legal_footprint(empty,pottery,footprint(clay))) return {};
    const auto source=original_entrances(empty,clay),target=original_entrances(empty,pottery);
    if (source.empty() || target.empty() || (require_shared_road && !overlaps(source,target))) return {};
    auto occupied=footprint(clay);const auto pot=footprint(pottery);
    occupied.insert(occupied.end(),pot.begin(),pot.end());
    std::vector<sim::Cell> houses;
    const auto consider=[&](sim::Cell p) {
        if (houses.size()==2 || !legal_footprint(empty,p,occupied)) return;
        houses.push_back(p);const auto cells=footprint(p);occupied.insert(occupied.end(),cells.begin(),cells.end());
    };
    consider({clay.x+6,clay.y});consider({clay.x+9,clay.y});
    for (int y=clay.y-12;y<=clay.y+12 && houses.size()<2;++y)
        for (int x=clay.x-12;x<=clay.x+12 && houses.size()<2;++x) consider({x,y});
    if (houses.size()!=2) return {};
    return Layout{clay,pottery,houses[0],houses[1],user_geometry};
}
Layout find_layout(const sim::World& empty) {
    // A documented test choice only. Classification/permission comes entirely
    // from the loaded immutable policy, with no coordinate exception.
    if (const auto selected=layout_at(empty,{116,111},{116,108},true)) return *selected;
    for (int y=0;y<empty.height();++y) for (int x=0;x<empty.width();++x) {
        if (!empty.original_road({x,y})) continue;
        const std::array<std::pair<sim::Cell,sim::Cell>,4> candidates{{
            {{x,y+1},{x,y-2}},{{x-1,y+1},{x-1,y-2}},
            {{x+1,y},{x-2,y}},{{x+1,y-1},{x-2,y-1}}}};
        for (const auto& [clay,pottery]:candidates)
            if (const auto selected=layout_at(empty,clay,pottery,false)) return *selected;
    }
    throw std::runtime_error("No legal paid shared-original-ground-road mini found; other layouts not exhausted");
}
std::pair<sim::BuildingId,sim::BuildingId> build(sim::World& world,const Layout& layout,J& commands) {
    const std::array plan{
        sim::Command{sim::CommandType::PlaceHousehold,layout.house1},
        sim::Command{sim::CommandType::PlaceHousehold,layout.house2},
        sim::Command{sim::CommandType::PlaceClaySource,layout.clay},
        sim::Command{sim::CommandType::PlacePottery,layout.pottery}};
    for (const auto& command:plan) {
        const auto result=world.execute(command);
        require(result.accepted && result.changed,"Normal paid starter command rejected: "+result.reason);
        commands.push_back({{"type",static_cast<unsigned>(command.type)},{"cell",point(command.cell)},
            {"funds",world.treasury()},{"spent",world.construction_spent_total()},
            {"sequence",world.command_sequence()}});
    }
    return {*world.building_owner_at(layout.clay),*world.building_owner_at(layout.pottery)};
}
sim::CourierId clay_courier(const sim::World& world) {
    for (const auto& courier:world.couriers()) if (courier.role==sim::CourierRole::Clay) return courier.id;
    throw std::runtime_error("Actual Clay courier missing");
}
J connectivity(const Loaded& map,const sim::World& world,sim::BuildingId clay,sim::BuildingId pottery) {
    const auto before=world.snapshot();const auto report=sim::diagnose_road_connectivity(world,clay,pottery);
    require(world.snapshot()==before,"Diagnostic query mutated actual mini");
    J result={{"category",sim::road_connectivity_category_name(report.category)},
        {"source_entrances",J::array()},{"target_entrances",J::array()},{"route",J::array()}};
    const auto entrances=[&](const std::vector<sim::BuildingEntrance>& input) {
        J out=J::array();for (const auto& e:input) out.push_back({{"building",raw(map,world,e.building_cell)},
            {"road",raw(map,world,e.road_cell)},
            {"edge_blocker",sim::build_blocker_name(world.map_permissions()->transport_edge_blocker(e.building_cell,e.road_cell))}});
        return out;
    };
    result["source_entrances"]=entrances(report.source.entrances);
    result["target_entrances"]=entrances(report.target.entrances);
    if (report.route) for (const auto cell:*report.route) result["route"].push_back(raw(map,world,cell));
    return result;
}
J gate_edges(const Loaded& map,const sim::World& world) {
    J result=J::array();
    for (const auto& gate:world.map_permissions()->gates()) {
        const sim::Cell direction{gate.corridor[1].x-gate.corridor[0].x,
            gate.corridor[1].y-gate.corridor[0].y};
        const sim::Cell middle=gate.corridor[1];
        const sim::Cell side{middle.x+direction.y,middle.y-direction.x};
        J entry={{"gate",gate.id.value},{"openings",J::array()},
            {"side_from",raw(map,world,middle)},{"side_to",raw(map,world,side)},
            {"side_edge_blocker",sim::build_blocker_name(world.map_permissions()->transport_edge_blocker(middle,side))},
            {"side_traversable",world.transport_edge_allowed(middle,side)}};
        for (std::size_t i=0;i<2;++i) {
            const auto passage=i==0 ? gate.corridor.front():gate.corridor.back();
            entry["openings"].push_back({{"outside",raw(map,world,gate.openings[i])},
                {"passage",raw(map,world,passage)},
                {"edge_blocker",sim::build_blocker_name(world.map_permissions()->transport_edge_blocker(gate.openings[i],passage))},
                {"traversable",world.transport_edge_allowed(gate.openings[i],passage)}});
        }
        require(!world.transport_edge_allowed(middle,side),"Actual original-road policy permitted gate side entry");
        result.push_back(std::move(entry));
    }
    return result;
}
J actual_gate_trip(const fs::path& output,const fs::path& data,const fs::path& relative,const Loaded& map) {
    sim::World empty(map.policy,sim::RulesProfile::CityV16,4);
    for (const auto& gate:map.policy->gates()) {
        if (!empty.transport_edge_allowed(gate.openings[0],gate.corridor.front()) ||
            !empty.transport_edge_allowed(gate.openings[1],gate.corridor.back())) continue;
        const sim::Cell direction{gate.corridor[1].x-gate.corridor[0].x,
            gate.corridor[1].y-gate.corridor[0].y};
        const auto candidates=[&](std::size_t side) {
            const auto opening=gate.openings[side];std::vector<sim::Cell> result;
            for (int y=opening.y-5;y<=opening.y+5;++y) for (int x=opening.x-5;x<=opening.x+5;++x) {
                const sim::Cell origin{x,y};
                if (!legal_footprint(empty,origin,{}) || original_entrances(empty,origin).empty()) continue;
                const auto cells=footprint(origin);
                const bool outside=std::all_of(cells.begin(),cells.end(),[&](sim::Cell point) {
                    const int projection=(point.x-opening.x)*direction.x+(point.y-opening.y)*direction.y;
                    return side==0 ? projection<=0:projection>=0;
                });
                if (outside) result.push_back(origin);
            }
            const auto distance=[&](sim::Cell point) {
                return std::abs(point.x-opening.x)+std::abs(point.y-opening.y);
            };
            std::sort(result.begin(),result.end(),[&](sim::Cell a,sim::Cell b) {
                if (distance(a)!=distance(b)) return distance(a)<distance(b);
                return a.y!=b.y ? a.y<b.y:a.x<b.x;
            });
            if (result.size()>8) result.resize(8);return result;
        };
        for (const auto source:candidates(0)) for (const auto target:candidates(1)) {
            const auto layout=layout_at(empty,source,target,false,false);if (!layout) continue;
            sim::World world(map.policy,sim::RulesProfile::CityV16,4);J commands=J::array();
            const auto [clay,pottery]=build(world,*layout,commands);
            const auto route=world.find_building_route(clay,pottery);
            if (!route || !std::all_of(gate.corridor.begin(),gate.corridor.end(),[&](sim::Cell cell) {
                return std::count(route->begin(),route->end(),cell)==1;
            })) continue;
            const auto id=clay_courier(world);J result={{"status","actual delivery and return"},
                {"gate",gate.id.value},{"commands",commands},{"clay",point(source)},
                {"pottery",point(target)},{"route",points(*route)},
                {"gate_cells",points(gate.corridor)},{"ready",J::object()}};
            while (world.ticks()<31) {world.tick();invariants(world);}
            result["ready"]=facts(world);
            write_world(output/"checkpoints"/"gate-before-entry.json",data,relative,map,world);
            auto restored=sim::World::restore(world.snapshot(),map.policy);
            bool dispatched=false,inside=false,arrived=false,returned=false;
            while (world.ticks()<400) {
                world.tick();restored.tick();invariants(world);
                require(world.snapshot()==restored.snapshot(),"Actual original gate mini restore diverged");
                const auto& courier=world.courier(id);
                if (!dispatched && courier.phase==sim::CourierPhase::ToWarehouse) {
                    dispatched=true;result["first_dispatch"]=facts(world);
                }
                if (!inside && courier.phase==sim::CourierPhase::ToWarehouse &&
                    world.fixed_passage(courier.path[courier.path_vertex])) {
                    inside=true;result["first_inside_gate"]=facts(world);
                    write_world(output/"checkpoints"/"inside-original-gate.json",data,relative,map,world);
                }
                if (!arrived && dispatched && courier.phase==sim::CourierPhase::Returning) {
                    arrived=true;result["first_arrival"]=facts(world);
                }
                if (!returned && arrived && courier.phase==sim::CourierPhase::IdleAtWorkshop) {
                    returned=true;result["first_return"]=facts(world);
                }
            }
            require(dispatched && inside && arrived && returned && world.pottery_completed_total()>0 &&
                world.roads_placed_total()==0 && world.snapshot().roads.empty(),
                "Actual original-road/fixed-gate trip did not complete naturally");
            result["at400"]=facts(world);
            write_world(output/"checkpoints"/"gate-at400.json",data,relative,map,world);return result;
        }
    }
    return {{"status",map.policy->gates().empty() ? "No fixed gate in this map policy":
        "No complete paid gate mini among the bounded eight origins per side; other layouts not exhausted"}};
}
void write_report(const fs::path& path,const J& report) {
    std::ofstream output(path);output<<report.dump(2)<<'\n';require(bool(output),"Report write failed");
}
}
int main(int argc,char** argv) {
    std::optional<fs::path> failure_output;
    try {
        require(argc==4 || (argc==6 && std::string(argv[4])=="--continue"),
            "usage: openemperor-original-roads-fixture DATA exact-relative-map fresh-.local-root [--continue private-save]");
        const auto data=fs::canonical(argv[1]);const fs::path relative=argv[2];
        const auto output=fs::weakly_canonical(argv[3]);const auto private_root=fs::canonical(fs::current_path()/".local");
        require(beneath(output,private_root) && output!=private_root && !beneath(output,data),
            "Fixture output must be ignored and outside original inputs");
        require(!fs::exists(output) || (fs::is_directory(output) && fs::is_empty(output)),
            "Fixture root must be fresh and empty");
        failure_output=output;
        const auto checked=maps::check_map_rules({data,relative,sim::RulesProfile::CityV16,4,2});
        require(checked.status==maps::MapRulesStatus::MapRulesChecked,"Map rule4/policy2 rejected: "+checked.detail);
        fs::create_directories(output/"saves");fs::create_directories(output/"checkpoints");Sdl sdl;
        auto map=load(data,relative,sdl.renderer,checked.input_sha256);
        J report={{"evidence","Technical real-map normal-paid-command replay; not Oliver's actual save or normal-App/human acceptance"},
            {"map",relative.generic_string()},{"map_sha256",checked.input_sha256},{"rule",4},{"policy",2},
            {"schema",20},{"policy_fingerprint",save::map_permissions_fingerprint(*map.policy,checked.input_sha256)},
            {"old_policy_fingerprint",save::map_permissions_fingerprint(*map.old_policy,checked.input_sha256)},
            {"original_road_cells",0},{"road_classifications",J::object()}};
        for (const auto& cell:map.policy->cells()) {
            if (cell.original_road) report["original_road_cells"]=report["original_road_cells"].get<int>()+1;
            const auto name=sim::original_road_classification_name(cell.original_road_classification);
            report["road_classifications"][name]=report["road_classifications"].value(name,0)+1;
        }
        sim::World world(map.policy,sim::RulesProfile::CityV16,4);
        report["actual_gate_edges"]=gate_edges(map,world);
        if (argc==6) {
            const auto input=fs::canonical(argv[5]);
            require(beneath(input,private_root) && !beneath(input,data),"Continuation must read an isolated private save");
            const auto document=save::read_save(input);
            require(document.source_schema_version==20 && document.map_relative==relative &&
                document.map_sha256==checked.input_sha256,"Continuation source map/schema mismatch");
            world=save::restore_save(document,data,map.legacy,map.policy);
            report["fresh_process_source"]=input.generic_string();report["start"]=facts(world);
            while (world.ticks()<400) {world.tick();invariants(world);}
            write_world(output/"checkpoints"/"at400.json",data,relative,map,world);
            report["at400"]=facts(world);write_report(output/"original-roads-report.json",report);
            std::cout<<"Independent original-road continuation reached tick 400\n";return 0;
        }
        require(world.buildings().empty() && world.couriers().empty() && world.total_population()==0 &&
            world.roads_placed_total()==0 && world.command_sequence()==0,"Original infrastructure imported mutable entities/commands");
        report["empty"]=facts(world);const auto layout=find_layout(world);report["commands"]=J::array();
        report["layout"]={{"clay",point(layout.clay)},{"pottery",point(layout.pottery)},
            {"house1",point(layout.house1)},{"house2",point(layout.house2)},
            {"matches_user_geometry",layout.user_geometry},{"exact_user_save_executed",false}};
        const auto [clay,pottery]=build(world,layout,report["commands"]);
        require(world.roads_placed_total()==0 && world.command_sequence()==4 && world.snapshot().roads.empty() &&
            world.workforce_supply()==12 && world.workforce_used()==10,"Paid no-road mini has artificial authority");
        report["connectivity"]=connectivity(map,world,clay,pottery);
        require(report["connectivity"]["category"]=="CONNECTED","Actual original-road entrances did not connect");
        sim::World old(map.old_policy,sim::RulesProfile::CityV16,3);J old_commands=J::array();
        const auto [old_clay,old_pottery]=build(old,layout,old_commands);
        for (int i=0;i<64;++i) old.tick();
        require(!old.find_building_route(old_clay,old_pottery) &&
            old.courier_dispatch_status(clay_courier(old)).status==sim::CourierDispatchStatus::NoRoad,
            "Old policy unexpectedly acquired fixed original-road authority");
        report["old_policy_control"]=facts(old);
        write_world(output/"checkpoints"/"old-policy-control.json",data,relative,map,old);
        while (world.ticks()<31) {world.tick();invariants(world);}
        report["ready"]=facts(world);
        write_world(output/"saves"/"A-original-ready.json",data,relative,map,world);
        auto restored=sim::World::restore(world.snapshot(),map.policy);
        const auto id=clay_courier(world);bool outbound=false,on_road=false,arrived=false,returned=false;
        report["phase_events"]=J::array();
        while (world.ticks()<400) {
            const auto phase=world.courier(id).phase;world.tick();restored.tick();invariants(world);
            require(world.snapshot()==restored.snapshot(),"Before-entry snapshot continuation diverged");
            const auto& courier=world.courier(id);
            if (phase!=courier.phase) report["phase_events"].push_back({{"tick",world.ticks()},{"facts",facts(world)}});
            if (!outbound && courier.phase==sim::CourierPhase::ToWarehouse) {
                outbound=true;report["first_dispatch"]=facts(world);
            }
            if (!on_road && courier.phase==sim::CourierPhase::ToWarehouse &&
                world.original_road(courier.path[courier.path_vertex])) {
                on_road=true;report["first_on_original_road"]=facts(world);
                write_world(output/"checkpoints"/"on-original-road.json",data,relative,map,world);
            }
            if (!arrived && outbound && courier.phase==sim::CourierPhase::Returning) {
                arrived=true;report["first_arrival"]=facts(world);
                write_world(output/"checkpoints"/"returning.json",data,relative,map,world);
            }
            if (!returned && arrived && courier.phase==sim::CourierPhase::IdleAtWorkshop) {
                returned=true;report["first_return"]=facts(world);
            }
        }
        require(outbound && on_road && arrived && returned && world.pottery_completed_total()>0 &&
            world.roads_placed_total()==0 && world.snapshot().roads.empty(),
            "No actual complete original-road delivery/return/production without road purchase");
        report["actual_complete_clay_trip"]=true;report["at400"]=facts(world);
        write_world(output/"checkpoints"/"at400.json",data,relative,map,world);
        report["actual_through_gate_trip"]=actual_gate_trip(output,data,relative,map);
        write_report(output/"original-roads-report.json",report);
        std::cout<<"Actual paid original-road mini: "<<relative.generic_string()<<", rule 4 / policy 2 / schema 20\n";
        std::cout<<"Ready save: "<<(output/"saves"/"A-original-ready.json").generic_string()<<'\n';return 0;
    } catch (const std::exception& error) {
        if (failure_output) try {
            fs::create_directories(*failure_output);
            write_report(*failure_output/"fixture-failure.json",{{"status","failed"},{"reason",error.what()}});
        } catch (...) {}
        std::cerr<<"Original-road fixture: "<<error.what()<<'\n';return 1;
    }
}
