#include "maps/SandboxPlacement.h"
#include "simulation/World.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <zlib.h>
#include <iostream>
#include <stdexcept>
#include <utility>
using namespace openemperor;
namespace {
constexpr std::size_t count=228U*228U;
constexpr maps::GridCell start{100,110};
std::size_t raw(maps::GridCell c){return std::size_t(c.y)*228U+c.x;}
void check(bool b,const char*m){if(!b)throw std::runtime_error(m);}
template<class F>void rejects(F f,const char*m){try{f();}catch(const std::invalid_argument&){return;}throw std::runtime_error(m);}
struct Fixture {
 maps::ParsedEmperorMap map;maps::MapGeometry geometry{170};maps::OriginalMapEntities entities;
 maps::StoredGraphicsPlan plan;std::vector<std::uint8_t> legacy=std::vector<std::uint8_t>(count,1);
 Fixture(){map.declared_map_size=170;map.terrain_raw.values.assign(count,0x80);map.objects_raw.values.assign(count,0);entities.manager_schema=1;entities.declared_map_size=170;plan.border=29;plan.height_bytes.assign(count,255);}
 maps::OriginalEntityRecord source(maps::GridCell p,unsigned id=7,unsigned index=0) const {
  maps::OriginalEntityRecord e;e.manager_index=index;e.original_id=maps::OriginalEntityId{id};e.serialized_original_id=int(id);e.status=3;e.local_x=std::int16_t(p.x-29);e.local_y=std::int16_t(p.y-29);e.serialized_cell_reference=int(raw(p));e.footprint_side=2;e.provenance.base_schema=4;return e;
 }
 void gate(int layout){auto e=source(start);e.entity_class=maps::OriginalEntityClass::GateHouse;e.type=130;e.footprint_side=1;e.gate_house=maps::OriginalGateHouseState{layout};e.provenance.wrapper_schema=1;e.provenance.extended_schema=1;e.provenance.class_wrapper_schema=2;entities.records.push_back(e);const unsigned w=layout?3:5,h=layout?5:3;for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x)map.terrain_raw.values[raw({start.x+x,start.y+y})]=0x8088;}
 auto make() const {return maps::make_sandbox_map_permissions(map,plan,geometry,legacy,entities);}
};
void free_and_protected(){Fixture f;f.legacy[raw(start)]=0;auto p=f.make();check(p->road_allowed({100,110})&&!p->building_allowed({100,110}),"road raw fact independent of legacy/render footprint");auto before=p->canonical_state();f.plan.assets.resize(2);f.plan.assets[0].status=maps::StoredStatus::DecodeFailed;f.plan.landscape_layers_available=false;f.plan.raw_saved_ids.assign(count,0xffffffff);f.plan.raw_candidate_bytes.assign(count,255);check(f.make()->canonical_state()==before,"decode/profile/saved graphic state cannot decide roads or passages");f.legacy[raw(start)]=1;auto e=f.source(start);f.entities.records.push_back(e);p=f.make();check(!p->road_allowed({100,110})&&!p->building_allowed({100,110})&&p->protected_original({100,110})&&p->road_blocker({101,111})==simulation::BuildBlocker::OriginalStructure,"complete original saved square reserved despite grass");check(p->road_allowed({102,112}),"original reservation does not expand beyond source square");Fixture rawbits;for(auto terrain:{4U,0x4080U,0x8088U,0x82U,0x2000080U,0x80080U}){rawbits.map.terrain_raw.values[raw(start)]=terrain;check(!rawbits.make()->road_allowed({100,110})&&!rawbits.make()->building_allowed({100,110}),"water/wall/gate/rock/pinnacle/offmap denied");}rawbits.map.terrain_raw.values[raw(start)]=0x80;rawbits.map.objects_raw.values[raw(start)]=1;check(!rawbits.make()->road_allowed({100,110}),"objects0 required");check(!f.make()->road_allowed({0,0}),"candidate-map border is not road terrain");}
void road_markings(){Fixture f;f.map.terrain_raw.values[raw(start)]=0xc0;auto p=f.make();check(p->road_allowed({100,110})&&!p->building_allowed({100,110})&&!p->fixed_passage({100,110}),"explicit ground-road raw marking permits paid roads only, not building or fixed road");f.map.objects_raw.values[raw(start)]=1;check(!f.make()->road_allowed({100,110}),"road-marked original objects stay blocked");for(auto value:{0x40U,0xc1U,0x1c0U,0x800c0U}){f.map.objects_raw.values[raw(start)]=0;f.map.terrain_raw.values[raw(start)]=value;check(!f.make()->road_allowed({100,110}),"ground-road whitelist admits no extra bits");}}
void reswall_occupancy() {
 for (unsigned schema:{3U,4U}) for (int type:{90,91,231}) for (unsigned policy:{1U,2U}) {
  Fixture f;auto wall=f.source(start);
  wall.entity_class=maps::OriginalEntityClass::ResWall;wall.type=std::int16_t(type);
  wall.footprint_side=1;wall.subindex=0;wall.provenance.base_schema=std::uint16_t(schema);
  wall.provenance.wrapper_schema=wall.provenance.extended_schema=wall.provenance.class_wrapper_schema=1;
  f.entities.records.push_back(wall);
  auto prepare=[&](const Fixture& input) {
   return maps::make_sandbox_map_permissions(input.map,input.plan,input.geometry,input.legacy,input.entities,policy);
  };
  // A typed physical singleton must remain protected even on otherwise
  // importable raw ground road. Objects is not an ownership-ID oracle.
  f.map.terrain_raw.values[raw(start)]=0xc0;
  const auto p=prepare(f);
  check(p->protected_original({100,110}) && !p->building_allowed({100,110}) &&
        !p->road_allowed({100,110}) && !p->original_road({100,110}) &&
        p->road_blocker({100,110})==simulation::BuildBlocker::OriginalStructure &&
        p->original_road_classification({100,110})==simulation::OriginalRoadClassification::OriginalStructure,
        "verified ResWall singleton excludes Road-bit import and building permission");
  check(!p->protected_original({101,110}) && p->road_allowed({101,110}) &&
        p->building_allowed({101,110}) && p->cell_height({100,110})==-1 && p->gates().empty(),
        "ResWall protection is exactly one cell with unchanged neighbors, heights and no passages");
  auto actual=f;actual.map.terrain_raw.values[raw(start)]=0x88;actual.map.objects_raw.values[raw(start)]=4;
  check(prepare(actual)->protected_original({100,110}),"observed raw structure/object words stay protected independently");
  auto mutate=[&](auto edit,const char* reason) {auto bad=f;edit(bad.entities.records[0]);rejects([&]{prepare(bad);},reason);};
  mutate([](auto& e){e.type=232;},"unevidenced ResWall type with positive footprint rejected");
  mutate([](auto& e){e.status=2;},"unevidenced ResWall active status rejected");
  mutate([](auto& e){e.status=255;},"unknown status cannot become inactive");
  mutate([](auto& e){e.subindex=1;},"unevidenced ResWall subindex rejected");
  mutate([](auto& e){e.footprint_side=0;},"zero-side ResWall is neither free nor implicit one-cell");
  mutate([](auto& e){e.footprint_side=2;},"generic positive square cannot admit unsupported ResWall geometry");
  mutate([](auto& e){e.provenance.base_schema=5;},"structurally parsed Base5 does not prove this occupancy state");
  mutate([](auto& e){e.provenance.wrapper_schema=2;},"unknown ResWall parent wrapper rejected");
  mutate([](auto& e){e.provenance.extended_schema=2;},"unknown ResWall NonHouse state rejected");
  mutate([](auto& e){e.provenance.class_wrapper_schema=0;},"Tower wrapper cannot select ResWall occupancy");
  mutate([](auto& e){++e.serialized_cell_reference;},"ResWall requires matching cell reference");
  mutate([](auto& e){e.local_x=-1;},"negative ResWall origin rejected");
  mutate([](auto& e){e.local_y=170;},"ResWall beyond declared extent rejected");
  mutate([](auto& e){e.original_id.reset();},"ResWall requires original identity");
  mutate([](auto& e){e.gate_house=maps::OriginalGateHouseState{0};},"ResWall cannot carry typed gate state");
  auto offmap=f;offmap.map.terrain_raw.values[raw(start)]|=0x80000;
  rejects([&]{prepare(offmap);},"ResWall on off-map terrain rejected");
  auto outside=f;outside.entities.records[0].local_x=outside.entities.records[0].local_y=0;
  outside.entities.records[0].serialized_cell_reference=int(raw({29,29}));
  rejects([&]{prepare(outside);},"ResWall outside candidate diamond rejected");
  for (bool first:{false,true}) {
   auto conflict=f;conflict.entities.records.push_back(f.source(start,8,1));
   if(first)std::reverse(conflict.entities.records.begin(),conflict.entities.records.end());
   rejects([&]{prepare(conflict);},"ResWall / foreign physical claim conflicts in either order");
   for (maps::GridCell claim:{start,maps::GridCell{102,110},maps::GridCell{102,109}}) {
    Fixture g;g.gate(0);auto e=wall;e.manager_index=1;e.original_id=maps::OriginalEntityId{8};e.serialized_original_id=8;
    e.local_x=std::int16_t(claim.x-29);e.local_y=std::int16_t(claim.y-29);e.serialized_cell_reference=int(raw(claim));
    g.entities.records.push_back(e);if(first)std::reverse(g.entities.records.begin(),g.entities.records.end());
    rejects([&]{prepare(g);},"ResWall conflicts with gate solid, corridor and opening in either order");
   }
  }
  Fixture gate;gate.gate(0);const auto before=prepare(gate)->canonical_state();
  auto inactive=wall;inactive.status=0;inactive.original_id.reset();inactive.serialized_original_id=-1;
  inactive.local_x=inactive.local_y=-1;gate.entities.records.push_back(inactive);
  check(prepare(gate)->canonical_state()==before,"complete inactive ResWall leaves gate geometry exact");
 }
}
void resgate_occupancy() {
 for (unsigned schema:{3U,4U}) for (int type:{105,106}) for (unsigned policy:{1U,2U}) {
  Fixture f;auto wall=f.source(start);
  wall.entity_class=maps::OriginalEntityClass::ResGate;wall.type=std::int16_t(type);
  wall.footprint_side=1;wall.subindex=0;wall.provenance.base_schema=std::uint16_t(schema);
  wall.provenance.wrapper_schema=wall.provenance.extended_schema=wall.provenance.class_wrapper_schema=1;
  f.entities.records.push_back(wall);
  auto prepare=[&](const Fixture& input) {
   return maps::make_sandbox_map_permissions(input.map,input.plan,input.geometry,input.legacy,input.entities,policy);
  };
  // A typed physical singleton must remain protected even on otherwise
  // importable raw ground road. Objects is not an ownership-ID oracle.
  f.map.terrain_raw.values[raw(start)]=0xc0;
  const auto p=prepare(f);
  check(p->protected_original({100,110}) && !p->building_allowed({100,110}) &&
        !p->road_allowed({100,110}) && !p->original_road({100,110}) &&
        p->road_blocker({100,110})==simulation::BuildBlocker::OriginalStructure &&
        p->original_road_classification({100,110})==simulation::OriginalRoadClassification::OriginalStructure,
        "verified ResGate singleton excludes Road-bit import and building permission");
  check(!p->protected_original({101,110}) && p->road_allowed({101,110}) &&
        p->building_allowed({101,110}) && p->cell_height({100,110})==-1 && p->gates().empty(),
        "ResGate protection is exactly one cell with unchanged neighbors, heights and no passages");
  auto actual=f;actual.map.terrain_raw.values[raw(start)]=0xc8;actual.map.objects_raw.values[raw(start)]=4;
  check(prepare(actual)->protected_original({100,110}),"observed raw structure/object words stay protected independently");
  auto mutate=[&](auto edit,const char* reason) {auto bad=f;edit(bad.entities.records[0]);rejects([&]{prepare(bad);},reason);};
  mutate([](auto& e){e.type=107;},"unevidenced ResGate type with positive footprint rejected");
  mutate([](auto& e){e.status=2;},"unevidenced ResGate active status rejected");
  mutate([](auto& e){e.status=255;},"unknown status cannot become inactive");
  mutate([](auto& e){e.subindex=1;},"unevidenced ResGate subindex rejected");
  mutate([](auto& e){e.footprint_side=0;},"zero-side ResGate is neither free nor implicit one-cell");
  mutate([](auto& e){e.footprint_side=2;},"generic positive square cannot admit unsupported ResGate geometry");
  mutate([](auto& e){e.provenance.base_schema=5;},"structurally parsed Base5 does not prove this occupancy state");
  mutate([](auto& e){e.provenance.wrapper_schema=2;},"unknown ResGate parent wrapper rejected");
  mutate([](auto& e){e.provenance.extended_schema=2;},"unknown ResGate NonHouse state rejected");
  mutate([](auto& e){e.provenance.class_wrapper_schema=0;},"Tower wrapper cannot select ResGate occupancy");
  mutate([](auto& e){++e.serialized_cell_reference;},"ResGate requires matching cell reference");
  mutate([](auto& e){e.local_x=-1;},"negative ResGate origin rejected");
  mutate([](auto& e){e.local_y=170;},"ResGate beyond declared extent rejected");
  mutate([](auto& e){e.original_id.reset();},"ResGate requires original identity");
  mutate([](auto& e){e.gate_house=maps::OriginalGateHouseState{0};},"ResGate cannot carry typed gate state");
  auto offmap=f;offmap.map.terrain_raw.values[raw(start)]|=0x80000;
  rejects([&]{prepare(offmap);},"ResGate on off-map terrain rejected");
  auto outside=f;outside.entities.records[0].local_x=outside.entities.records[0].local_y=0;
  outside.entities.records[0].serialized_cell_reference=int(raw({29,29}));
  rejects([&]{prepare(outside);},"ResGate outside candidate diamond rejected");
  for (bool first:{false,true}) {
   auto conflict=f;conflict.entities.records.push_back(f.source(start,8,1));
   if(first)std::reverse(conflict.entities.records.begin(),conflict.entities.records.end());
   rejects([&]{prepare(conflict);},"ResGate / foreign physical claim conflicts in either order");
   for (maps::GridCell claim:{start,maps::GridCell{102,110},maps::GridCell{102,109}}) {
    Fixture g;g.gate(0);auto e=wall;e.manager_index=1;e.original_id=maps::OriginalEntityId{8};e.serialized_original_id=8;
    e.local_x=std::int16_t(claim.x-29);e.local_y=std::int16_t(claim.y-29);e.serialized_cell_reference=int(raw(claim));
    g.entities.records.push_back(e);if(first)std::reverse(g.entities.records.begin(),g.entities.records.end());
    rejects([&]{prepare(g);},"ResGate conflicts with gate solid, corridor and opening in either order");
   }
  }
  Fixture gate;gate.gate(0);const auto before=prepare(gate)->canonical_state();
  auto inactive=wall;inactive.status=0;inactive.original_id.reset();inactive.serialized_original_id=-1;
  inactive.local_x=inactive.local_y=-1;gate.entities.records.push_back(inactive);
  check(prepare(gate)->canonical_state()==before,"complete inactive ResGate leaves gate geometry exact");
 }
}
void passage_layouts(){for(int layout:{0,1}){Fixture f;f.gate(layout);auto p=f.make();check(p->gates().size()==1&&p->gates()[0].id==simulation::FixedGateId{7},"stable original gate ID is distinct and zero manager index valid");const std::array<simulation::Cell,3> corridor=layout?std::array<simulation::Cell,3>{{{100,112},{101,112},{102,112}}}:std::array<simulation::Cell,3>{{{102,110},{102,111},{102,112}}};const std::array<simulation::Cell,2> openings=layout?std::array<simulation::Cell,2>{{{99,112},{103,112}}}:std::array<simulation::Cell,2>{{{102,109},{102,113}}};check(p->gates()[0].corridor==std::vector(corridor.begin(),corridor.end())&&p->gates()[0].openings==openings,"literal independently authored short-axis corridor and two openings");for(auto c:p->gates()[0].protected_footprint)check(p->protected_original(c)&&!p->building_allowed(c),"all15 protected from buildings");for(unsigned k=0;k<3;++k)check(p->fixed_passage(corridor[k])&&p->road_allowed(corridor[k])&&p->cell_height(corridor[k])==-1,"exactthree corridor cells and signed height retained");check(p->transport_edge_allowed(openings[0],corridor[0])&&p->transport_edge_allowed(corridor[0],openings[0])&&p->transport_edge_allowed(openings[1],corridor[2])&&p->transport_edge_allowed(corridor[2],openings[1]),"both opening directions");check(p->transport_edge_allowed(corridor[0],corridor[1])&&p->transport_edge_allowed(corridor[1],corridor[2]),"ordinary internal neighbor edges");const simulation::Cell side=layout?simulation::Cell{101,111}:simulation::Cell{101,111};check(p->transport_edge_blocker(side,corridor[1])==simulation::BuildBlocker::GateSideEntry,"solid side entry denied");check(!p->road_allowed({100,110})&&p->road_blocker({100,110})==simulation::BuildBlocker::GateSolidPart,"solid gate corner denied independently of corridor");Fixture no_render=f;no_render.plan.height_bytes=f.plan.height_bytes;no_render.legacy.assign(count,0);auto missing=no_render.make();check(missing->gates()[0].corridor==p->gates()[0].corridor&&missing->road_mask()==p->road_mask(),"no render-derived legacy permissions prerequisite for gate/roads");}}
void markers(){for(int type:{162,163,174,180,185}){Fixture f;auto e=f.source(start);e.entity_class=maps::OriginalEntityClass::Industrial;e.type=std::int16_t(type);e.footprint_side=0;f.entities.records.push_back(e);auto second=e;second.manager_index=1;second.original_id=maps::OriginalEntityId{8};second.serialized_original_id=8;f.entities.records.push_back(second);auto p=f.make();check(p->protected_original({100,110})&&!p->road_allowed({100,110})&&p->road_allowed({101,110}),"bounded marker reserves only exact saved origin; duplicate markers are not duplicate physical claims");}Fixture bad;auto e=bad.source(start);e.entity_class=maps::OriginalEntityClass::Industrial;e.footprint_side=0;e.type=35;bad.entities.records.push_back(e);rejects([&]{bad.make();},"unknown zero-side marker must not free occupancy");Fixture other;other.gate(0);auto marker=other.source(start,8,1);marker.entity_class=maps::OriginalEntityClass::Industrial;marker.footprint_side=0;marker.type=163;other.entities.records.push_back(marker);rejects([&]{other.make();},"marker/gate conflict fail closed");}
void saved_editor_marker_origins() {
 const std::array<std::pair<int,unsigned>,7> sources{{
  {175,3},{175,4},{165,3},{165,4},{165,5},{183,3},{183,4}}};
 for (const auto [type,schema]:sources) {
  Fixture f;
  auto marker=f.source(start);
  marker.entity_class=maps::OriginalEntityClass::Industrial;
  marker.type=std::int16_t(type);marker.footprint_side=0;marker.subindex=0;
  marker.provenance.base_schema=std::uint16_t(schema);
  marker.provenance.wrapper_schema=marker.provenance.extended_schema=1;
  f.entities.records.push_back(marker);
  f.map.terrain_raw.values[raw(start)]=0xc0;
  const auto p=f.make();
  check(p->protected_original({100,110}) && !p->road_allowed({100,110}) &&
        !p->building_allowed({100,110}) &&
        p->road_blocker({100,110})==simulation::BuildBlocker::OriginalStructure &&
        p->building_blocker({100,110})==simulation::BuildBlocker::OriginalStructure,
        "saved marker origin protected even on independently road-permitted raw ground");
  check(p->road_allowed({101,110}) && p->building_allowed({101,110}) &&
        !p->protected_original({101,110}) && p->gates().empty(),
        "marker reservation is one exact origin, not an invented square or transport link");
  auto mutate=[&](auto edit,const char* message) {
   auto bad=f;edit(bad.entities.records[0]);
   rejects([&]{bad.make();},message);
  };
  mutate([](auto&e){e.provenance.base_schema=6;},"unknown marker base schema rejected");
  if (type==175 || type==183)
   mutate([](auto&e){e.provenance.base_schema=5;},"unevidenced point-marker schema combination rejected");
  mutate([](auto&e){e.provenance.wrapper_schema=2;},"unknown marker wrapper rejected");
  mutate([](auto&e){e.provenance.extended_schema=2;},"unknown marker state schema rejected");
  mutate([](auto&e){e.provenance.class_wrapper_schema=1;},"unknown marker class wrapper rejected");
  mutate([](auto&e){e.type=176;},"same zero side does not admit unknown type");
  mutate([](auto&e){e.entity_class=maps::OriginalEntityClass::Building;},"same marker type in different class rejected");
  mutate([](auto&e){e.status=2;},"unevidenced active marker status rejected");
  mutate([](auto&e){e.subindex=1;},"unevidenced marker subindex rejected");
  mutate([](auto&e){e.original_id.reset();},"marker requires valid original identity");
  mutate([](auto&e){e.serialized_original_id=8;},"marker original identity mismatch rejected");
  mutate([](auto&e){++e.serialized_cell_reference;},"marker requires exact serialized reference");
  mutate([](auto&e){e.local_x=-1;},"negative marker origin rejected");
  mutate([](auto&e){e.local_y=170;},"outside marker origin rejected");
  auto outside=f;
  outside.entities.records[0].local_x=outside.entities.records[0].local_y=0;
  outside.entities.records[0].serialized_cell_reference=int(raw({29,29}));
  rejects([&]{outside.make();},"marker origin outside candidate diamond rejected");
  auto offmap=f;offmap.map.terrain_raw.values[raw(start)]|=0x80000;
  rejects([&]{offmap.make();},"marker origin on raw off-map terrain rejected");
  auto duplicate=f;auto second=marker;
  second.manager_index=1;second.original_id=maps::OriginalEntityId{8};second.serialized_original_id=8;
  duplicate.entities.records.push_back(second);
  check(duplicate.make()->canonical_state()==p->canonical_state(),
        "distinct validated markers sharing an origin follow existing marker duplicate contract");
  duplicate.entities.records[1].type=163;
  check(duplicate.make()->canonical_state()==p->canonical_state(),
        "new and existing validated marker origins share the same reservation");
  duplicate.entities.records[1]=marker;
  rejects([&]{duplicate.make();},"duplicate marker identity still rejected");
  for (bool marker_first:{false,true}) {
   auto physical=f;auto building=f.source(start,8,1);
   physical.entities.records.push_back(building);
   if (!marker_first) std::reverse(physical.entities.records.begin(),physical.entities.records.end());
   rejects([&]{physical.make();},"physical/marker overlap rejected in either record order");
   for (const maps::GridCell claim:{start,maps::GridCell{102,110},maps::GridCell{102,109}}) {
    Fixture gate;gate.gate(0);auto gate_marker=marker;
    gate_marker.manager_index=1;gate_marker.original_id=maps::OriginalEntityId{8};gate_marker.serialized_original_id=8;
    gate_marker.local_x=std::int16_t(claim.x-29);gate_marker.local_y=std::int16_t(claim.y-29);
    gate_marker.serialized_cell_reference=int(raw(claim));
    gate.entities.records.push_back(gate_marker);
    if (marker_first) std::reverse(gate.entities.records.begin(),gate.entities.records.end());
    rejects([&]{gate.make();},"marker cannot occupy a gate solid cell, corridor or opening in either order");
   }
  }
  auto terrain=f;terrain.map.terrain_raw.values[raw({101,110})]=0x82;
  check(!terrain.make()->road_allowed({101,110}) && !terrain.make()->building_allowed({101,110}),
        "marker support does not expand the terrain whitelist");
  if (type==165) {
   auto water=f;water.map.terrain_raw.values[raw(start)]=4;
   water.map.terrain_raw.values[raw({101,110})]=4;
   const auto reserved=water.make();
   check(reserved->protected_original({100,110}) && !reserved->road_allowed({100,110}) &&
         !reserved->building_allowed({100,110}) && !reserved->road_allowed({101,110}) &&
         !reserved->building_allowed({101,110}),
         "literal water cells stay blocked; editor marker does not clear water or create roads");
  }
  if (type==183) {
   // Literal authored expectations, independent of the classifier: a saved
   // point is protected even when current terrain/objects no longer describe
   // an empty grass cell. No surrounding physical square is inferred.
   for (const auto [terrain_word,objects_word]:
        std::array<std::pair<unsigned,unsigned>,3>{{{0,0},{0x80,0},{0x80,1}}}) {
    auto point=f;
    point.map.terrain_raw.values[raw(start)]=terrain_word;
    point.map.objects_raw.values[raw(start)]=objects_word;
    const auto reserved=point.make();
    check(reserved->protected_original({100,110}) && !reserved->road_allowed({100,110}) &&
          !reserved->building_allowed({100,110}) &&
          reserved->road_allowed({101,110}) && reserved->building_allowed({101,110}) &&
          !reserved->protected_original({101,110}) && reserved->cell_height({100,110})==-1,
          "saved type183 point protects exact origin without changing neighbors or signed height");
   }
   for (std::uint8_t status:std::array<std::uint8_t,3>{1,4,255})
    mutate([status](auto&e){e.status=status;},"other active type183 statuses remain unsupported");
   mutate([](auto&e){e.entity_class=static_cast<maps::OriginalEntityClass>(255);},
          "invalid source class remains unsupported");
  }
  auto inactive=f;inactive.entities.records[0].status=0;
  check(inactive.make()->road_allowed({100,110}) && !inactive.make()->protected_original({100,110}),
        "inactive records retain the previous non-occupying contract");
 }
}
void river_marker_origins() {
 for (int type:{177,178}) for (unsigned schema:{4U,3U}) for (unsigned policy:{1U,2U}) {
  Fixture f;auto marker=f.source(start);
  marker.entity_class=maps::OriginalEntityClass::Industrial;marker.type=std::int16_t(type);
  marker.footprint_side=0;marker.provenance.base_schema=std::uint16_t(schema);
  marker.provenance.wrapper_schema=marker.provenance.extended_schema=1;
  f.entities.records.push_back(marker);f.map.terrain_raw.values[raw(start)]=0x04000004;
  auto prepare=[&](const Fixture& input) {
   return maps::make_sandbox_map_permissions(input.map,input.plan,input.geometry,input.legacy,input.entities,policy);
  };
  const auto p=prepare(f);
  check(p->protected_original({100,110}) && !p->road_allowed({100,110}) &&
        !p->building_allowed({100,110}) && !p->original_road({100,110}) &&
        p->road_blocker({100,110})==simulation::BuildBlocker::OriginalStructure &&
        p->building_blocker({100,110})==simulation::BuildBlocker::OriginalStructure &&
        p->cell_height({100,110})==-1 && p->gates().empty(),
        "literal river marker origin protected without water transit or physical square");
  check(p->road_allowed({101,110}) && p->building_allowed({101,110}) &&
        !p->protected_original({101,110}) && !p->protected_original({100,111}),
        "river point does not reserve a neighboring region");
  auto road=f;road.map.terrain_raw.values[raw(start)]=0xc0;
  check(!prepare(road)->original_road({100,110}) && !prepare(road)->road_allowed({100,110}),
        "Road bit cannot override river origin protection");
  auto water=f;water.map.terrain_raw.values[raw({101,110})]=0x04000004;
  check(!prepare(water)->road_allowed({101,110}) && !prepare(water)->building_allowed({101,110}) &&
        !prepare(water)->protected_original({101,110}),"neighboring water keeps independent terrain restrictions");
  // Original coordinates remain meaningful independently of current raw bits.
  auto changed=f;changed.map.terrain_raw.values[raw(start)]=0x80;changed.map.objects_raw.values[raw(start)]=1;
  check(prepare(changed)->protected_original({100,110}),"raw Objects is not river marker ownership authority");
  auto mutate=[&](auto edit,const char* reason) {auto bad=f;edit(bad.entities.records[0]);rejects([&]{prepare(bad);},reason);};
  for (auto entity_class:{maps::OriginalEntityClass::Building,maps::OriginalEntityClass::Ferry})
   mutate([entity_class](auto& e){e.entity_class=entity_class;},"river type requires exact Industrial class");
  for (int unknown:{176,188})
   mutate([unknown](auto& e){e.type=std::int16_t(unknown);if(unknown==138||unknown==167||unknown==181||unknown==187||unknown==188||unknown==189)e.provenance.extended_schema=2;},"neighboring unknown types remain unsupported");
  for (int formerly_unknown:{179,164})
   mutate([formerly_unknown](auto& e){e.type=std::int16_t(formerly_unknown);e.provenance.extended_schema=2;},
          "invalid variants of formerly unknown MP34 types remain unsupported");
  for (std::uint8_t status:std::array<std::uint8_t,4>{1,2,4,255})
   mutate([status](auto& e){e.status=status;},"unevidenced active river status remains unsupported");
  mutate([](auto& e){e.subindex=1;},"unevidenced river subindex rejected");
  mutate([](auto& e){e.provenance.base_schema=5;},"unevidenced zero-side river Base5 rejected");
  mutate([](auto& e){e.provenance.wrapper_schema=2;},"unevidenced river parent wrapper rejected");
  mutate([](auto& e){e.provenance.extended_schema=2;},"unevidenced river non-house state rejected");
  mutate([](auto& e){e.provenance.class_wrapper_schema=1;},"river has no class-specific wrapper");
  mutate([](auto& e){e.gate_house=maps::OriginalGateHouseState{0};},"river cannot carry GateHouse geometry");
  mutate([](auto& e){e.original_id.reset();},"river requires original identity");
  mutate([](auto& e){++e.serialized_cell_reference;},"river requires exact cell reference");
  mutate([](auto& e){e.local_x=-1;},"negative river origin rejected");
  mutate([](auto& e){e.local_y=170;},"unbounded river origin rejected");
  auto outside=f;outside.entities.records[0].local_x=outside.entities.records[0].local_y=0;
  outside.entities.records[0].serialized_cell_reference=int(raw({29,29}));
  rejects([&]{prepare(outside);},"river origin outside actual diamond rejected");
  auto offmap=f;offmap.map.terrain_raw.values[raw(start)]|=0x80000;
  rejects([&]{prepare(offmap);},"river origin on raw off-map terrain rejected");
  auto duplicate=f;auto second=marker;second.manager_index=1;
  second.original_id=maps::OriginalEntityId{8};second.serialized_original_id=8;
  duplicate.entities.records.push_back(second);
  check(prepare(duplicate)->canonical_state()==p->canonical_state(),"distinct river markers at same point retain duplicate-marker contract");
  duplicate.entities.records[1].type=164;duplicate.entities.records[1].provenance.extended_schema=2;
  rejects([&]{prepare(duplicate);},"valid river prefix cannot hide another unknown active source");
  for (bool reverse:{false,true}) {
   auto physical=f;physical.entities.records.push_back(f.source(start,8,1));
   if(reverse)std::reverse(physical.entities.records.begin(),physical.entities.records.end());
   rejects([&]{prepare(physical);},"river/physical conflict rejected in both orders");
   for (maps::GridCell claim:{start,maps::GridCell{102,110},maps::GridCell{102,109}}) {
    Fixture g;g.gate(0);auto e=marker;e.manager_index=1;e.original_id=maps::OriginalEntityId{8};e.serialized_original_id=8;
    e.local_x=std::int16_t(claim.x-29);e.local_y=std::int16_t(claim.y-29);e.serialized_cell_reference=int(raw(claim));
    g.entities.records.push_back(e);if(reverse)std::reverse(g.entities.records.begin(),g.entities.records.end());
    rejects([&]{prepare(g);},"river cannot claim gate solids, corridor or opening");
   }
  }
  // Preserve the existing positive SavedSquare branch independently of the
  // new narrow zero-side admission, even with otherwise unevidenced states.
  auto square=f;square.entities.records[0].footprint_side=2;square.entities.records[0].status=2;
  square.entities.records[0].provenance.base_schema=5;
  const auto reserved=prepare(square);
  check(reserved->protected_original({101,111}) && !reserved->protected_original({102,111}),
        "existing positive-square contract is unchanged for river types");
 }
}
void mp34_origin_contract(const Fixture& f,std::int8_t expected_height) {
 for (unsigned policy:{1U,2U}) {
  auto prepare=[&](const Fixture& input) {
   return maps::make_sandbox_map_permissions(input.map,input.plan,input.geometry,input.legacy,input.entities,policy);
  };
  const auto p=prepare(f);
  // Literal expected permission mask: one protected point, four independent
  // free neighbors. Neither a physical square nor editor terrain is imported.
  check(p->protected_original({100,110}) && !p->road_allowed({100,110}) &&
        !p->building_allowed({100,110}) && !p->original_road({100,110}) &&
        p->road_blocker({100,110})==simulation::BuildBlocker::OriginalStructure &&
        p->building_blocker({100,110})==simulation::BuildBlocker::OriginalStructure &&
        p->cell_height({100,110})==expected_height && p->gates().empty(),
        "MP34 exact saved origin must remain protected");
  for (simulation::Cell c:std::array<simulation::Cell,4>{{{99,110},{101,110},{100,109},{100,111}}})
   check(p->road_allowed(c) && p->building_allowed(c) && !p->protected_original(c),
         "MP34 point does not claim free neighboring cells");
  auto road=f;road.map.terrain_raw.values[raw(start)]=0xc0;
  check(prepare(road)->protected_original({100,110}) && !prepare(road)->road_allowed({100,110}) &&
        !prepare(road)->original_road({100,110}),"Road bit cannot free MP34 protected origin");
  for (const auto [terrain,objects]:std::array<std::pair<unsigned,unsigned>,3>{{{0x04000004,0},{0x80,1},{0x80,0}}}) {
   auto neighbor=f;neighbor.map.terrain_raw.values[raw({101,110})]=terrain;
   neighbor.map.objects_raw.values[raw({101,110})]=objects;
   const auto n=prepare(neighbor);const bool free=terrain==0x80 && objects==0;
   check(n->road_allowed({101,110})==free && n->building_allowed({101,110})==free &&
         !n->protected_original({101,110}),"neighbor terrain/object authority is independent");
  }
  auto mutate=[&](auto edit,const char* reason){auto bad=f;edit(bad.entities.records[0]);rejects([&]{prepare(bad);},reason);};
  for (auto cls:{maps::OriginalEntityClass::Building,maps::OriginalEntityClass::Ferry})
   mutate([cls](auto& e){e.entity_class=cls;},"MP34 same type in wrong class rejected");
  for (std::uint8_t status:std::array<std::uint8_t,4>{1,2,4,255})
   mutate([status](auto& e){e.status=status;},"MP34 other active status rejected");
  mutate([](auto& e){e.subindex=1;},"MP34 unevidenced subindex rejected");
  mutate([](auto& e){e.provenance.base_schema=2;},"MP34 unevidenced base schema rejected");
  mutate([](auto& e){e.provenance.base_schema=5;},"MP34 zero-side Base5 remains unsupported");
  if(f.entities.records[0].type==179)
   mutate([](auto& e){e.provenance.base_schema=3;},"type179 zero-side Base3 is not evidenced");
  mutate([](auto& e){e.provenance.wrapper_schema=2;},"MP34 wrong parent wrapper rejected");
  mutate([](auto& e){e.provenance.extended_schema=2;},"MP34 wrong NonHouseInfo state rejected");
  mutate([](auto& e){e.provenance.class_wrapper_schema=1;},"MP34 unexpected class wrapper rejected");
  mutate([](auto& e){e.monument=maps::OriginalMonumentState{};},"MP34 cannot carry monument state");
  mutate([](auto& e){e.gate_house=maps::OriginalGateHouseState{0};},"MP34 cannot carry GateHouse state");
  mutate([](auto& e){e.original_id.reset();},"MP34 original identity required");
  mutate([](auto& e){e.serialized_original_id=8;},"MP34 mismatched identity rejected");
  mutate([](auto& e){e.manager_index=4000;},"MP34 unbounded manager index rejected");
  mutate([](auto& e){++e.serialized_cell_reference;},"MP34 exact cell reference required");
  mutate([](auto& e){e.local_x=-1;},"MP34 negative origin rejected");
  mutate([](auto& e){e.local_y=170;},"MP34 unbounded origin rejected");
  auto outside=f;outside.entities.records[0].local_x=outside.entities.records[0].local_y=0;
  outside.entities.records[0].serialized_cell_reference=int(raw({29,29}));
  rejects([&]{prepare(outside);},"MP34 origin outside actual map diamond rejected");
  auto offmap=f;offmap.map.terrain_raw.values[raw(start)]|=0x80000;
  rejects([&]{prepare(offmap);},"MP34 raw off-map origin rejected");
  auto duplicate=f;auto second=f.entities.records[0];second.manager_index=1;
  second.original_id=maps::OriginalEntityId{8};second.serialized_original_id=8;
  duplicate.entities.records.push_back(second);
  check(prepare(duplicate)->canonical_state()==p->canonical_state(),"distinct MP34 point records retain allowed marker overlap");
  duplicate.entities.records[1].type=188;duplicate.entities.records[1].provenance.extended_schema=2;
  rejects([&]{prepare(duplicate);},"valid MP34 prefix cannot publish policy with later unknown source");
  for (bool reverse:{false,true}) {
   auto physical=f;physical.entities.records.push_back(f.source(start,8,1));
   if(reverse)std::reverse(physical.entities.records.begin(),physical.entities.records.end());
   rejects([&]{prepare(physical);},"MP34 foreign physical overlap rejected in either order");
   for (maps::GridCell claim:{start,maps::GridCell{102,110},maps::GridCell{102,109}}) {
    Fixture g;g.gate(0);auto e=second;e.type=f.entities.records[0].type;
    e.local_x=std::int16_t(claim.x-29);e.local_y=std::int16_t(claim.y-29);e.serialized_cell_reference=int(raw(claim));
    g.entities.records.push_back(e);if(reverse)std::reverse(g.entities.records.begin(),g.entities.records.end());
    rejects([&]{prepare(g);},"MP34 point cannot overlap gate solids/corridor/opening");
   }
  }
  auto square=f;square.entities.records[0].footprint_side=2;square.entities.records[0].status=2;
  square.entities.records[0].provenance.base_schema=5;
  check(prepare(square)->protected_original({101,111}) && !prepare(square)->protected_original({102,111}),
        "MP34 types retain prior positive SavedSquare contract");
  auto inactive=f;inactive.entities.records[0].status=0;
  check(!prepare(inactive)->protected_original({100,110}),"inactive MP34 source remains non-occupying");
 }
}
void industrial179_origin() {
 Fixture f;auto e=f.source(start);e.entity_class=maps::OriginalEntityClass::Industrial;
 e.type=179;e.footprint_side=0;e.subindex=0;e.provenance.base_schema=4;
 e.provenance.wrapper_schema=e.provenance.extended_schema=1;f.entities.records.push_back(e);
 f.map.terrain_raw.values[raw(start)]=0x04000004;f.plan.height_bytes[raw(start)]=0;
 mp34_origin_contract(f,0);
}
void industrial164_origin() {
 Fixture f;auto e=f.source(start);e.entity_class=maps::OriginalEntityClass::Industrial;
 e.type=164;e.footprint_side=0;e.subindex=0;e.provenance.base_schema=4;
 e.provenance.wrapper_schema=e.provenance.extended_schema=1;f.entities.records.push_back(e);
 f.map.terrain_raw.values[raw(start)]=0x81;f.map.objects_raw.values[raw(start)]=2;
 f.plan.height_bytes[raw(start)]=2;mp34_origin_contract(f,2);
 f.map.terrain_raw.values[raw(start)]=0x200;f.map.objects_raw.values[raw(start)]=0;
 f.plan.height_bytes[raw(start)]=0;mp34_origin_contract(f,0);
 f.entities.records[0].provenance.base_schema=3;mp34_origin_contract(f,0);
}
void industrial186_origin() {
 // Authored MP48-like state. No original bytes, coordinates or admission
 // oracle: the literal contract is one protected source, not a copper brush.
 Fixture f;auto e=f.source(start);e.entity_class=maps::OriginalEntityClass::Industrial;
 e.type=186;e.footprint_side=0;e.subindex=0;e.provenance.base_schema=4;
 e.provenance.wrapper_schema=e.provenance.extended_schema=1;f.entities.records.push_back(e);
 f.map.terrain_raw.values[raw(start)]=0x100082;f.plan.height_bytes[raw(start)]=2;
 mp34_origin_contract(f,2); // Same established origin/bounds/conflict contract.
 for(unsigned policy:{1U,2U}) {
  auto prepare=[&](const Fixture& input) {
   return maps::make_sandbox_map_permissions(input.map,input.plan,input.geometry,input.legacy,input.entities,policy);
  };
  auto mutate=[&](auto edit,const char* message){auto bad=f;edit(bad.entities.records[0]);rejects([&]{prepare(bad);},message);};
  mutate([](auto& source){source.provenance.base_schema=3;},"type186 zero-side Base3 has no evidence");
  mutate([](auto& source){source.footprint_side=17;},"type186 invalid side must not become a marker");
  mutate([](auto& source){source.type=151;source.provenance.extended_schema=2;},"unevidenced type151 state remains unsupported");
  for(int unknown:{173,176,187,188})
   mutate([unknown](auto& source){source.type=std::int16_t(unknown);if(unknown==138||unknown==167||unknown==181||unknown==187||unknown==188||unknown==189)source.provenance.extended_schema=2;if(unknown==173)source.provenance.extended_schema=2;},"unevidenced original type/state remains unsupported");
  auto duplicate=f;auto second=e;second.manager_index=1;duplicate.entities.records.push_back(second);
  rejects([&]{prepare(duplicate);},"type186 duplicate original ID rejected");
  second.original_id=maps::OriginalEntityId{8};second.serialized_original_id=8;second.manager_index=e.manager_index;
  duplicate.entities.records[1]=second;
  rejects([&]{prepare(duplicate);},"type186 duplicate manager index rejected");
  auto marker=f;auto other=e;other.type=185;other.manager_index=1;other.original_id=maps::OriginalEntityId{8};other.serialized_original_id=8;
  marker.entities.records.push_back(other);
  check(prepare(marker)->canonical_state()==prepare(f)->canonical_state(),"type186 retains existing allowed distinct-marker overlap");
  auto unknown=marker;unknown.entities.records[1].provenance.extended_schema=2;unknown.entities.records[1].type=186;
  rejects([&]{prepare(unknown);},"valid marker prefix cannot hide conflicting unevidenced type186 state");
 }
}
void industrial151_origin() {
 // Authored MPWall1-like state. No original bytes, coordinates or admission
 // oracle: the literal contract is one protected source, not a terrain-flag edit.
 Fixture f;auto e=f.source(start);e.entity_class=maps::OriginalEntityClass::Industrial;
 e.type=151;e.footprint_side=0;e.subindex=0;e.provenance.base_schema=4;
 e.provenance.wrapper_schema=e.provenance.extended_schema=1;f.entities.records.push_back(e);
 f.map.terrain_raw.values[raw(start)]=0x02000080;f.plan.height_bytes[raw(start)]=0;f.map.objects_raw.values[raw(start)]=8;
 mp34_origin_contract(f,0); // Same established origin/bounds/conflict contract.
 for(unsigned policy:{1U,2U}) {
  auto prepare=[&](const Fixture& input) {
   return maps::make_sandbox_map_permissions(input.map,input.plan,input.geometry,input.legacy,input.entities,policy);
  };
  auto mutate=[&](auto edit,const char* message){auto bad=f;edit(bad.entities.records[0]);rejects([&]{prepare(bad);},message);};
  mutate([](auto& source){source.provenance.base_schema=3;},"type151 zero-side Base3 has no evidence");
  mutate([](auto& source){source.footprint_side=17;},"type151 invalid side must not become a marker");
  for(int unknown:{149,173,176,187,188})
   mutate([unknown](auto& source){source.type=std::int16_t(unknown);if(unknown==138||unknown==167||unknown==181||unknown==187||unknown==188||unknown==189)source.provenance.extended_schema=2;if(unknown==173)source.provenance.extended_schema=2;},"unevidenced original type/state remains unsupported");
  auto duplicate=f;auto second=e;second.manager_index=1;duplicate.entities.records.push_back(second);
  rejects([&]{prepare(duplicate);},"type151 duplicate original ID rejected");
  second.original_id=maps::OriginalEntityId{8};second.serialized_original_id=8;second.manager_index=e.manager_index;
  duplicate.entities.records[1]=second;
  rejects([&]{prepare(duplicate);},"type151 duplicate manager index rejected");
  auto marker=f;auto other=e;other.type=185;other.manager_index=1;other.original_id=maps::OriginalEntityId{8};other.serialized_original_id=8;
  marker.entities.records.push_back(other);
  check(prepare(marker)->canonical_state()==prepare(f)->canonical_state(),"type151 retains existing allowed distinct-marker overlap");
  auto unknown=marker;unknown.entities.records[1].provenance.extended_schema=2;unknown.entities.records[1].type=151;
  rejects([&]{prepare(unknown);},"valid marker prefix cannot hide conflicting unevidenced type151 state");
 }
}
void industrial173_origin() {
 // Independently authored invasion-point state; literal expectations remain
 // one protected origin and free neighbors, without an invasion or region.
 Fixture f;auto e=f.source(start);e.entity_class=maps::OriginalEntityClass::Industrial;
 e.type=173;e.footprint_side=0;e.subindex=0;e.provenance.base_schema=4;
 e.provenance.wrapper_schema=e.provenance.extended_schema=1;f.entities.records.push_back(e);
 f.map.terrain_raw.values[raw(start)]=0x80;f.plan.height_bytes[raw(start)]=0;
 for(const auto base:std::array<std::uint16_t,2>{4,3}) {
  f.entities.records[0].provenance.base_schema=base;
  mp34_origin_contract(f,0);
  for(unsigned policy:{1U,2U}) {
   auto prepare=[&](const Fixture& input) {
    return maps::make_sandbox_map_permissions(input.map,input.plan,input.geometry,input.legacy,input.entities,policy);
   };
   auto mutate=[&](auto edit,const char* message){auto bad=f;edit(bad.entities.records[0]);rejects([&]{prepare(bad);},message);};
   mutate([](auto& source){source.footprint_side=17;},"type173 invalid side must not become a marker");
   mutate([](auto& source){source.original_id=maps::OriginalEntityId{0};source.serialized_original_id=0;},"type173 zero original ID rejected");
   mutate([](auto& source){source.original_id=maps::OriginalEntityId{4001};source.serialized_original_id=4001;},"type173 out-of-range original ID rejected");
   for(int unknown:{149,167,176,181,187,188})
    mutate([unknown](auto& source){source.type=std::int16_t(unknown);if(unknown==138||unknown==167||unknown==181||unknown==187||unknown==188||unknown==189)source.provenance.extended_schema=2;},"unrelated unknown type remains unsupported");
   auto duplicate=f;auto second=f.entities.records[0];second.manager_index=1;
   duplicate.entities.records.push_back(second);
   rejects([&]{prepare(duplicate);},"type173 duplicate original ID rejected");
   second.original_id=maps::OriginalEntityId{8};second.serialized_original_id=8;
   second.manager_index=f.entities.records[0].manager_index;duplicate.entities.records[1]=second;
   rejects([&]{prepare(duplicate);},"type173 duplicate manager index rejected");
   auto marked=f;marked.map.terrain_raw.values[raw(start)]=0xc0;marked.map.objects_raw.values[raw(start)]=1;
   check(prepare(marked)->protected_original({100,110}) && !prepare(marked)->original_road({100,110}),
         "saved invasion point is protected independently of Road and Objects bits");
  }
 }
}
void remaining_editor_origin(int type,std::initializer_list<std::uint16_t> bases) {
 // Independently authored source states and literal singleton expectations.
 // No original bytes or terrain-brush replay; test both complete policies.
 Fixture f;auto e=f.source(start);e.entity_class=maps::OriginalEntityClass::Industrial;
 e.type=std::int16_t(type);e.footprint_side=0;e.subindex=0;
 e.provenance.wrapper_schema=e.provenance.extended_schema=1;
 f.entities.records.push_back(e);f.plan.height_bytes[raw(start)]=255;
 for(auto base:bases) {
  f.entities.records[0].provenance.base_schema=base;
  mp34_origin_contract(f,-1);
  for(unsigned policy:{1U,2U}) {
   auto prepare=[&](const Fixture& input){return maps::make_sandbox_map_permissions(
       input.map,input.plan,input.geometry,input.legacy,input.entities,policy);};
   auto mutate=[&](auto edit,const char* reason){auto bad=f;edit(bad.entities.records[0]);rejects([&]{prepare(bad);},reason);};
   if(bases.size()==1)mutate([](auto& source){source.provenance.base_schema=3;},"unevidenced Base3 source refused");
   mutate([](auto& source){source.type=149;},"unrelated zero-side type remains unsupported");
   mutate([](auto& source){source.footprint_side=17;},"invalid source side cannot become marker");
   mutate([](auto& source){source.original_id=maps::OriginalEntityId{0};source.serialized_original_id=0;},"zero original ID refused");
   mutate([](auto& source){source.original_id=maps::OriginalEntityId{4001};source.serialized_original_id=4001;},"unbounded original ID refused");
   auto duplicate=f;auto second=f.entities.records[0];second.manager_index=1;
   duplicate.entities.records.push_back(second);rejects([&]{prepare(duplicate);},"duplicate source identity refused");
   second.original_id=maps::OriginalEntityId{8};second.serialized_original_id=8;second.manager_index=f.entities.records[0].manager_index;
   duplicate.entities.records[1]=second;rejects([&]{prepare(duplicate);},"duplicate manager index refused");
   auto inactive=f;inactive.entities.records[0].status=0;
   inactive.entities.records[0].local_x=inactive.entities.records[0].local_y=-1;
   auto empty=f;empty.entities.records.clear();
   check(prepare(inactive)->canonical_state()==prepare(empty)->canonical_state(),"inactive source changes no permission");
  }
 }
}
Fixture dirt_ferry_fixture(unsigned ferry_schema=1) {
 // Independently authored complete archive records. The real reader enforces
 // Ferry's additional schema1/2008-byte state before any claim decision.
 Fixture f;using Bytes=std::vector<std::uint8_t>;Bytes bytes(6,0);
 auto put16=[](Bytes& b,std::size_t at,unsigned value){b.at(at)=std::uint8_t(value);b.at(at+1)=std::uint8_t(value>>8);};
 auto put32=[&](Bytes& b,std::size_t at,unsigned value){put16(b,at,value);put16(b,at+2,value>>16);};
 put16(bytes,0,1);put32(bytes,2,2);
 auto append=[&](const std::string& name,unsigned type,unsigned side,unsigned id,bool ferry){
  const auto tag=bytes.size();bytes.resize(tag+6+name.size(),0);
  put16(bytes,tag,0xffff);put16(bytes,tag+4,unsigned(name.size()));
  std::copy(name.begin(),name.end(),bytes.begin()+std::ptrdiff_t(tag+6));
  const auto base=bytes.size();bytes.resize(base+311+(ferry?2010:0),0);
  put16(bytes,base,4);bytes[base+2]=3;bytes[base+5]=std::uint8_t(side);
  put16(bytes,base+8,71);put16(bytes,base+10,81);put32(bytes,base+12,unsigned(raw(start)));
  put16(bytes,base+16,type);put32(bytes,base+161,id);put16(bytes,base+181,1);put16(bytes,base+183,1);
  if(ferry)put16(bytes,base+311,ferry_schema);
 };
 append("cIndustrialBldg",162,0,7,false);append("cFerryBldg",210,2,8,true);
 f.entities=maps::parse_original_map_entities(bytes,170);return f;
}
void dirt_ferry_association() {
 const auto f=dirt_ferry_fixture();
 for(unsigned policy:{1U,2U}) {
  auto prepare=[&](const Fixture& input){return maps::make_sandbox_map_permissions(
      input.map,input.plan,input.geometry,input.legacy,input.entities,policy);};
  const auto permissions=prepare(f);
  // Literal union: complete2x2 Ferry reservation plus a coincident non-owning
  // terrain source. Both identities persist; no road/passage is created.
  for(simulation::Cell c:std::array<simulation::Cell,4>{{{100,110},{101,110},{100,111},{101,111}}})
   check(permissions->protected_original(c)&&!permissions->road_allowed(c)&&!permissions->building_allowed(c)&&
         permissions->road_blocker(c)==simulation::BuildBlocker::OriginalStructure&&
         permissions->building_blocker(c)==simulation::BuildBlocker::OriginalStructure&&
         permissions->cell_height(c)==-1,"exact Ferry union remains protected");
  for(simulation::Cell c:std::array<simulation::Cell,4>{{{99,110},{102,110},{100,109},{100,112}}})
   check(permissions->road_allowed(c)&&permissions->building_allowed(c)&&!permissions->protected_original(c),
         "association claims no extra neighbor");
  check(f.entities.records.size()==2&&f.entities.records[0].serialized_original_id==7&&
        f.entities.records[1].serialized_original_id==8,"both original sources retained");
  auto reversed=f;std::reverse(reversed.entities.records.begin(),reversed.entities.records.end());
  check(prepare(reversed)->canonical_state()==permissions->canonical_state(),"association independent of record order");
  auto body=f;body.entities.records.erase(body.entities.records.begin());
  check(prepare(body)->canonical_state()==permissions->canonical_state(),"annotation changes no physical permissions");
  auto marked=f;marked.map.terrain_raw.values[raw(start)]=0xc0;
  auto marked_body=marked;marked_body.entities.records.erase(marked_body.entities.records.begin());
  check(prepare(marked)->canonical_state()==prepare(marked_body)->canonical_state()&&
        !prepare(marked)->original_road({100,110}),"association creates no road under Road bit");
  auto mutate=[&](unsigned index,auto edit,const char* reason){auto bad=f;edit(bad.entities.records[index]);
   for(bool reverse:{false,true}){auto ordered=bad;if(reverse)std::reverse(ordered.entities.records.begin(),ordered.entities.records.end());
    rejects([&]{prepare(ordered);},reason);}};
  mutate(0,[](auto& e){e.type=163;},"same geometry with a different marker relation refused");
  mutate(0,[](auto& e){e.entity_class=maps::OriginalEntityClass::Building;},"wrong marker class refused");
  mutate(1,[](auto& e){e.entity_class=maps::OriginalEntityClass::Building;},"same geometry without typed Ferry refused");
  mutate(1,[](auto& e){e.type=211;},"another physical type refused");
  for(unsigned index:{0U,1U}) {
   mutate(index,[](auto& e){e.status=2;},"unevidenced association status refused");
   mutate(index,[](auto& e){e.subindex=1;},"unevidenced association subindex refused");
   mutate(index,[](auto& e){e.provenance.base_schema=3;},"unevidenced association Base3 refused");
   mutate(index,[](auto& e){e.provenance.base_schema=5;},"unevidenced association Base5 refused");
   mutate(index,[](auto& e){e.provenance.wrapper_schema=2;},"wrong parent wrapper refused");
   mutate(index,[](auto& e){e.provenance.extended_schema=2;},"wrong NonHouse state refused");
   mutate(index,[](auto& e){e.provenance.class_wrapper_schema=1;},"unexpected class wrapper refused");
   mutate(index,[](auto& e){e.monument=maps::OriginalMonumentState{};},"unproved composite parent state refused");
   mutate(index,[](auto& e){e.gate_house=maps::OriginalGateHouseState{0};},"unproved Gate state refused");
   mutate(index,[](auto& e){e.original_id.reset();},"association identity required");
   mutate(index,[](auto& e){e.serialized_original_id+=1;},"wrong source/physical owner identity refused");
   mutate(index,[](auto& e){++e.serialized_cell_reference;},"association reference must be exact");
  }
  mutate(0,[](auto& e){e.footprint_side=1;},"physical source cannot be relabeled annotation");
  mutate(1,[](auto& e){e.footprint_side=1;},"wrong physical area refused");
  mutate(0,[](auto& e){++e.local_x;++e.serialized_cell_reference;},"interior point is not the evidenced Ferry anchor");
  auto other=f;auto second=other.entities.records[0];second.original_id=maps::OriginalEntityId{9};
  second.serialized_original_id=9;second.manager_index=2;other.entities.records.push_back(second);
  check(prepare(other)->canonical_state()==permissions->canonical_state()&&other.entities.records.size()==3,
        "distinct valid Dirt annotations are retained, not merged or duplicate-ID owners");
  for(bool reverse:{false,true}) {
   auto mixed=other;mixed.entities.records.back().type=185;
   auto exclusive=other;exclusive.entities.records.back()=f.source(start,9,2);
   auto late=other;late.entities.records.back().type=138;late.entities.records.back().provenance.extended_schema=2;
   auto duplicate_id=other;duplicate_id.entities.records.back().original_id=maps::OriginalEntityId{7};duplicate_id.entities.records.back().serialized_original_id=7;
   auto duplicate_index=other;duplicate_index.entities.records.back().manager_index=0;
   auto offmap=f;offmap.map.terrain_raw.values[raw({101,111})]|=0x80000U;
   auto outside=f;for(auto& e:outside.entities.records){e.local_x=e.local_y=0;e.serialized_cell_reference=int(raw({29,29}));}
   for(auto* input:{&mixed,&exclusive,&late,&duplicate_id,&duplicate_index,&offmap,&outside}) {
    if(reverse)std::reverse(input->entities.records.begin(),input->entities.records.end());
    rejects([&]{prepare(*input);},"mixed marker/competing owner/late unknown/identity/bounds remain atomic failures");
   }
   Fixture gate;gate.gate(0);gate.entities.records[0].original_id=maps::OriginalEntityId{8};gate.entities.records[0].serialized_original_id=8;
   auto annotation=f.entities.records[0];annotation.manager_index=1;annotation.local_x=73;
   annotation.serialized_cell_reference=int(raw({102,110}));gate.entities.records.push_back(annotation);
   if(reverse)std::reverse(gate.entities.records.begin(),gate.entities.records.end());
   rejects([&]{prepare(gate);},"Dirt marker cannot free a Gate corridor");
  }
  bool invalid_ferry_state=false;
  try {dirt_ferry_fixture(2);}catch(const maps::OriginalEntityError&){invalid_ferry_state=true;}
  check(invalid_ferry_state,"Ferry's own unevidenced archive state rejected by complete reader");
  simulation::World world(permissions,simulation::RulesProfile::CityV16,policy==1?3:4);
  for(simulation::Cell c:std::array<simulation::Cell,4>{{{100,110},{101,110},{100,111},{101,111}}})
   check(!world.validate({simulation::CommandType::PlaceHousehold,c}).accepted&&
         !world.validate({simulation::CommandType::PlaceRoad,c}).accepted,"ordinary House/Road validation retains complete protection");
 }
}
Fixture outside_source_fixture(int type,unsigned schema) {
 Fixture f;using Bytes=std::vector<std::uint8_t>;
 auto put16=[](Bytes& b,std::size_t at,unsigned value){b.at(at)=std::uint8_t(value);b.at(at+1)=std::uint8_t(value>>8);};
 auto put32=[&](Bytes& b,std::size_t at,unsigned value){put16(b,at,value);put16(b,at+2,value>>16);};
 Bytes bytes(6,0);put16(bytes,0,1);put32(bytes,2,1);
 const std::string name="cIndustrialBldg";const auto tag=bytes.size();bytes.resize(tag+6+name.size());
 put16(bytes,tag,0xffff);put16(bytes,tag+4,unsigned(name.size()));
 std::copy(name.begin(),name.end(),bytes.begin()+std::ptrdiff_t(tag+6));
 const auto base=bytes.size();const auto length=schema==3 ? 179U:181U;bytes.resize(base+length+130,0);
 put16(bytes,base,schema);bytes[base+2]=3;bytes[base+5]=0;
 // Independently authored local/storage relation:170 map, border29.
 put16(bytes,base+8,138);put16(bytes,base+10,117);put32(bytes,base+12,33455);
 put16(bytes,base+16,unsigned(type));put32(bytes,base+length-20,7);
 put16(bytes,base+length,1);put16(bytes,base+length+2,1);
 f.entities=maps::parse_original_map_entities(bytes,170);
 f.map.terrain_raw.values[raw({167,146})]=0x80005U;
 bool truncated=false;bytes.pop_back();try {maps::parse_original_map_entities(bytes,170);}catch(const maps::OriginalEntityError&){truncated=true;}
 check(truncated,"external source still requires a complete original archive");return f;
}
void outside_editor_source(int type) {
 const unsigned schema=type==162 ? 3U:4U;const auto source=outside_source_fixture(type,schema);
 const auto& original=source.entities.records.front();
 check(source.entities.records.size()==1 && original.original_id->value==7 && original.manager_index==0 &&
       original.local_x==138 && original.local_y==117 && original.serialized_cell_reference==33455,
       "authored complete source identity/origin/reference retained");
 for(unsigned policy:{1U,2U}) for(bool offmap:{false,true}) {
  auto f=source;if(!offmap)f.map.terrain_raw.values[raw({167,146})]=0x80;
  auto control=f;control.entities.records.clear();
  auto prepare=[&](const Fixture& value){return maps::make_sandbox_map_permissions(value.map,value.plan,value.geometry,value.legacy,value.entities,policy);};
  const auto expected=prepare(control);const auto actual=prepare(f);
  check(actual->canonical_state()==expected->canonical_state(),
        "evidenced outside nonphysical source has no playable claim and leaves the full policy unchanged");
  check(!actual->protected_original({167,146}) && !actual->road_allowed({167,146}) &&
        !actual->building_allowed({167,146}) && actual->road_blocker({167,146})==simulation::BuildBlocker::OutsideMap,
        "storage-only source creates no outside playable cell");
  check(f.entities.records.front().original_id==original.original_id && f.entities.records.front().local_x==138 &&
        f.entities.records.front().local_y==117 && f.entities.records.front().serialized_cell_reference==33455,
        "preparation never deletes, merges or repairs the original source");
  simulation::World world(actual,simulation::RulesProfile::CityV16,policy==1?3:4);
  check(world.buildings().empty() && world.couriers().empty() &&
        !world.validate({simulation::CommandType::PlaceHousehold,{167,146}}).accepted &&
        !world.validate({simulation::CommandType::PlaceRoad,{167,146}}).accepted,
        "outside metadata creates no World owner or construction permission");
  auto in=f;in.entities.records[0].local_x=71;in.entities.records[0].local_y=81;
  in.entities.records[0].serialized_cell_reference=int(raw(start));
  check(prepare(in)->protected_original({100,110}),"same valid in-map source retains its old origin protection");
  in.map.terrain_raw.values[raw(start)]|=0x80000U;
  rejects([&]{prepare(in);},"inside-diamond offmap origin remains a strict failure");
  auto mutate=[&](auto edit,const char* reason){auto bad=f;edit(bad.entities.records[0]);rejects([&]{prepare(bad);},reason);};
  mutate([](auto& e){e.status=2;},"other active source status cannot exempt bounds");
  mutate([](auto& e){e.subindex=1;},"other source subindex cannot exempt bounds");
  mutate([](auto& e){e.footprint_side=1;},"physical side-one source outside remains forbidden");
  mutate([](auto& e){e.footprint_side=17;},"oversized positive claim stays unsupported");
  mutate([](auto& e){e.entity_class=maps::OriginalEntityClass::Building;},"foreign class does not gain a storage-only contract");
  mutate([&](auto& e){e.provenance.base_schema=std::uint16_t(schema==3?4:3);},"unevidenced outside Base family is refused");
  mutate([](auto& e){e.provenance.wrapper_schema=2;},"outside source keeps parent schema checks");
  mutate([](auto& e){e.provenance.extended_schema=2;},"outside source keeps NonHouse schema checks");
  mutate([](auto& e){e.provenance.class_wrapper_schema=1;},"outside source keeps absent own wrapper");
  mutate([](auto& e){e.provenance.logical_class_wrapper_offset=1;},"outside source cannot have an unproved class wrapper");
  mutate([](auto& e){e.monument=maps::OriginalMonumentState{};},"outside source cannot carry Monument state");
  mutate([](auto& e){e.gate_house=maps::OriginalGateHouseState{};},"outside source cannot carry Gate state");
  mutate([](auto& e){e.original_id.reset();},"outside source still needs an original identity");
  mutate([](auto& e){++e.serialized_cell_reference;},"outside source still needs the exact reference");
  mutate([](auto& e){e.local_x=-1;},"negative local origin stays forbidden");
  mutate([](auto& e){e.local_x=32767;},"outside storage must fail before any cell lookup");
  mutate([](auto& e){e.local_x=2;e.serialized_cell_reference=int(raw({31,146}));},"original zero-area placement margin is retained");
  mutate([](auto& e){e.local_x=168;e.serialized_cell_reference=int(raw({197,146}));},"original upper rectangle margin is retained");
  mutate([](auto& e){e.type=151;},"another normal marker has no unproved outside exemption");
  for(bool reverse:{false,true}) {
   auto mixed=f;auto physical=f.source(start,8,1);mixed.entities.records.push_back(physical);
   auto expected_mixed=mixed;expected_mixed.entities.records.erase(expected_mixed.entities.records.begin());
   if(reverse)std::reverse(mixed.entities.records.begin(),mixed.entities.records.end());
   check(prepare(mixed)->canonical_state()==prepare(expected_mixed)->canonical_state(),
         "source order does not alter a valid foreign physical claim or its protection");
   auto duplicate=f;duplicate.entities.records.push_back(duplicate.entities.records.front());
   duplicate.entities.records.back().manager_index=1;
   rejects([&]{prepare(duplicate);},"outside source cannot hide a duplicate original ID");
   duplicate.entities.records.back().original_id=maps::OriginalEntityId{8};duplicate.entities.records.back().serialized_original_id=8;
   duplicate.entities.records.back().manager_index=0;
   rejects([&]{prepare(duplicate);},"outside source cannot hide a duplicate manager identity");
   auto late=f;auto unknown=physical;unknown.type=32767;unknown.entity_class=maps::OriginalEntityClass::Industrial;unknown.footprint_side=0;
   late.entities.records.push_back(unknown);if(reverse)std::reverse(late.entities.records.begin(),late.entities.records.end());
   rejects([&]{prepare(late);},"later unknown record still refuses the entire manager");
   auto solid=f;solid.gate(0);
   auto& gate=solid.entities.records.back();gate.original_id=maps::OriginalEntityId{8};
   gate.serialized_original_id=8;gate.manager_index=1;
   check(prepare(solid)->gates().size()==1,"outside source leaves a distinct valid Gate fully prepared");
   auto annotation=solid.source({102,110},9,2);annotation.entity_class=maps::OriginalEntityClass::Industrial;
   annotation.type=std::int16_t(type);annotation.provenance=f.entities.records.front().provenance;
   solid.entities.records.push_back(annotation);
   if(reverse)std::reverse(solid.entities.records.begin(),solid.entities.records.end());
   bool gate_conflict=false;
   try{prepare(solid);}catch(const std::invalid_argument& error){gate_conflict=std::string(error.what()).find("conflicting complete original occupancy")!=std::string::npos;}
   check(gate_conflict,"outside contract cannot excuse an independent Gate solid conflict");
   auto foreign=f;foreign.entities.records.push_back(foreign.source({167,146},8,1));
   if(reverse)std::reverse(foreign.entities.records.begin(),foreign.entities.records.end());
   rejects([&]{prepare(foreign);},"a physical outside claim cannot borrow a nonphysical source contract");
  }
 }
}
void invalid_sources(){Fixture f;f.gate(0);auto mutate=[&](auto edit,const char* msg){auto bad=f;edit(bad);rejects([&]{bad.make();},msg);};mutate([](auto&b){b.map.terrain_raw.values[raw({104,112})]&=~8U;},"partial gate claims rejected");mutate([](auto&b){++b.entities.records[0].serialized_cell_reference;},"reference mismatch rejected");mutate([](auto&b){b.entities.records[0].gate_house->layout=2;},"unknown layout rejected");mutate([](auto&b){b.entities.records[0].provenance.class_wrapper_schema=3;},"unknown gate wrapper rejected");mutate([](auto&b){b.plan.height_bytes[raw({102,111})]=0;},"unequal corridor heights rejected");mutate([](auto&b){b.plan.height_bytes[raw({102,109})]=0;},"unequal opening height rejected");mutate([](auto&b){auto e=b.source({102,111},8,1);b.entities.records.push_back(e);},"overlapping original structure rejected");mutate([](auto&b){b.entities.manager_schema=0;},"unknown full manager schema rejected");mutate([](auto&b){b.plan.height_bytes.pop_back();},"truncated heights rejected");rejects([&]{maps::make_sandbox_map_permissions(f.map,f.plan,f.geometry,f.legacy,f.entities,4);},"unknown map policy rejected");Fixture h;h.plan.height_bytes[raw(start)]=128;h.plan.height_bytes[raw({101,110})]=127;auto p=h.make();check(p->cell_height({100,110})==-128&&p->cell_height({101,110})==127&&p->transport_edge_blocker({100,110},{101,110})==simulation::BuildBlocker::UnsupportedHeightTransition,"signed byte extremes do not create steep edges");}
void standalone_reader(){
 Fixture f;f.gate(1);
 using Bytes=std::vector<std::uint8_t>;
 const auto put16=[](Bytes&b,std::size_t p,unsigned v){b.at(p)=std::uint8_t(v);b.at(p+1)=std::uint8_t(v>>8);};
 const auto put32=[&](Bytes&b,std::size_t p,unsigned v){put16(b,p,v);put16(b,p+2,v>>16);};
 const auto append32=[&](Bytes&b,unsigned v){auto p=b.size();b.resize(p+4);put32(b,p,v);};
 Bytes record(6,0);put16(record,0,1);put32(record,2,1);
 const std::string name="cGateHouse";auto p=record.size();record.resize(p+6+name.size());put16(record,p,0xffff);put16(record,p+4,unsigned(name.size()));std::copy(name.begin(),name.end(),record.begin()+std::ptrdiff_t(p+6));
 p=record.size();record.resize(p+181+2+128+10);put16(record,p,4);record[p+2]=3;record[p+5]=1;
 put16(record,p+8,71);put16(record,p+10,81);put32(record,p+12,unsigned(raw(start)));put16(record,p+16,130);put32(record,p+112,1);put32(record,p+161,7);put16(record,p+181,1);put16(record,p+183,1);put16(record,p+311,2);
 Bytes bytes(std::size_t(maps::original_entities_logical_offset)+record.size(),0);
 const std::array<std::uint8_t,8> signature{5,0,0xfe,0xca,0,0,2,0};std::copy(signature.begin(),signature.end(),bytes.begin());put32(bytes,84,170);
 for(std::size_t k=0;k<count;++k){put32(bytes,std::size_t(maps::terrain_logical_offset)+k*4,f.map.terrain_raw.values[k]);put32(bytes,std::size_t(maps::objects_logical_offset)+k*4,f.map.objects_raw.values[k]);bytes[989275+k]=f.plan.height_bytes[k];}
 std::copy(record.begin(),record.end(),bytes.begin()+std::ptrdiff_t(maps::original_entities_logical_offset));
 Bytes container;append32(container,0xfedcbaaa);
 for(std::size_t at=0;at<bytes.size();at+=16384){const auto len=std::min<std::size_t>(16384,bytes.size()-at);uLongf length=compressBound(uLong(len));Bytes compressed(length);check(compress2(compressed.data(),&length,bytes.data()+at,uLong(len),6)==Z_OK,"authored map compression");compressed.resize(length);append32(container,0);append32(container,unsigned(length));append32(container,unsigned(len));container.insert(container.end(),compressed.begin(),compressed.end());}
 const auto root=std::filesystem::temp_directory_path()/("openemperor-map-permissions-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
 struct Clean{std::filesystem::path root;~Clean(){std::error_code e;std::filesystem::remove_all(root,e);}}clean{root};std::filesystem::create_directories(root/"Cities");
 {std::ofstream out(root/"Cities/authored.map",std::ios::binary);out.write(reinterpret_cast<const char*>(container.data()),std::streamsize(container.size()));check(bool(out),"authored map file");}
 auto read=maps::read_sandbox_map_permissions(root,"Cities/authored.map",f.legacy);check(read->canonical_state()==f.make()->canonical_state(),"SDL-free contained reader and explicit typed full-manager builder produce identical topology");
 rejects([&]{maps::read_sandbox_map_permissions(root,"Cities/missing.map",f.legacy,4);},"unknown policy rejected before reading a missing file");
}
void legacy_contract(){Fixture f;maps::StoredCell c;c.storage=start;c.cell_index=raw(start);c.terrain_raw=0x80;c.objects_raw=0;c.footprint_index=0;c.status=maps::StoredStatus::Rendered;f.plan.cells.push_back(c);maps::PlacedFootprint fp;fp.origin=start;fp.width_cells=fp.height_cells=1;fp.status=maps::StoredStatus::Rendered;f.plan.footprints.push_back(fp);check(maps::make_sandbox_buildable_mask(f.plan,f.geometry)[raw(start)]==1,"legacy successful1x1 rendered fixture");f.plan.cells[0].status=maps::StoredStatus::DecodeFailed;check(maps::make_sandbox_buildable_mask(f.plan,f.geometry)[raw(start)]==0,"legacy decode dependence remains exact");f.plan.cells[0].status=maps::StoredStatus::Rendered;f.plan.footprints[0].width_cells=2;check(maps::make_sandbox_buildable_mask(f.plan,f.geometry)[raw(start)]==0,"legacy footprint dependence remains exact");}
}
int main(int argc,char** argv){try{if(argc==2){const std::string type=argv[1];if(type.rfind("outside-",0)==0)outside_editor_source(std::stoi(type.substr(8)));else if(type=="179")industrial179_origin();else if(type=="164")industrial164_origin();else if(type=="186")industrial186_origin();else if(type=="151")industrial151_origin();else if(type=="173")industrial173_origin();else if(type=="187")remaining_editor_origin(187,{4});else if(type=="189")remaining_editor_origin(189,{4});else if(type=="138")remaining_editor_origin(138,{3,4});else if(type=="181")remaining_editor_origin(181,{3,4});else if(type=="188")remaining_editor_origin(188,{3,4});else if(type=="167")remaining_editor_origin(167,{4});else if(type=="dirt-ferry")dirt_ferry_association();else throw std::invalid_argument("Unknown focused occupancy test");}else{for(int type:{163,164,165,185,187,162})outside_editor_source(type);dirt_ferry_association();remaining_editor_origin(189,{4});remaining_editor_origin(187,{4});industrial173_origin();industrial151_origin();industrial186_origin();industrial179_origin();industrial164_origin();river_marker_origins();reswall_occupancy();resgate_occupancy();free_and_protected();road_markings();passage_layouts();markers();saved_editor_marker_origins();invalid_sources();standalone_reader();legacy_contract();}std::cout<<"Sandbox map permission tests passed\n";return 0;}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
