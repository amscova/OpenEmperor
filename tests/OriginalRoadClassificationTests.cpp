#include "maps/SandboxPlacement.h"
#include <array>
#include <iostream>
#include <stdexcept>
using namespace openemperor;
namespace {
constexpr std::size_t count=228U*228U;
constexpr maps::GridCell site{116,110};
std::size_t at(maps::GridCell c){return std::size_t(c.y)*228+c.x;}
void check(bool ok,const char* reason){if(!ok)throw std::runtime_error(reason);}
template<class F>void rejects(F f,const char* reason){try{f();}catch(const std::invalid_argument&){return;}throw std::runtime_error(reason);}
struct Fixture {
 maps::ParsedEmperorMap map;maps::MapGeometry geometry{170};maps::OriginalMapEntities entities;
 maps::StoredGraphicsPlan plan;std::vector<std::uint8_t> mask=std::vector<std::uint8_t>(count,1);
 Fixture(){map.declared_map_size=170;map.terrain_raw.values.assign(count,0x80);map.objects_raw.values.assign(count,0);entities.manager_schema=1;entities.declared_map_size=170;plan.border=29;plan.height_bytes.assign(count,0);}
 auto make(unsigned policy=simulation::kOriginalRoadsMapPermissionsPolicyVersion)const{return maps::make_sandbox_map_permissions(map,plan,geometry,mask,entities,policy);}
 maps::OriginalEntityRecord source(maps::GridCell c)const{maps::OriginalEntityRecord e;e.manager_index=0;e.original_id=maps::OriginalEntityId{7};e.serialized_original_id=7;e.status=3;e.local_x=std::int16_t(c.x-29);e.local_y=std::int16_t(c.y-29);e.serialized_cell_reference=int(at(c));e.footprint_side=2;e.provenance.base_schema=4;return e;}
};
void exact_ground_road(){
 Fixture f;f.map.terrain_raw.values[at(site)]=0xc0;
 auto p=f.make();check(p->policy_version()==2&&p->original_road({116,110})&&p->road_allowed({116,110})&&!p->building_allowed({116,110})&&!p->protected_original({116,110}),"literal ground road is available fixed transport and separately protected from buildings");
 check(p->original_road_classification({116,110})==simulation::OriginalRoadClassification::GroundRoad,"explicit ground-road evidence");
 check(p->building_blocker({116,110})==simulation::BuildBlocker::ProtectedOriginalRoad,"original road building blocker");
 check(!p->original_road({117,110})&&p->road_allowed({117,110})&&p->building_allowed({117,110}),"grass permits purchase but does not become transport");
 check(p->transport_edge_allowed({116,110},{117,110}),"same signed height ordinary edge remains legal");
 const auto state=p->canonical_state();f.mask.assign(count,0);f.plan.raw_saved_ids.assign(count,0xffffffff);f.plan.assets.resize(1);f.plan.assets[0].status=maps::StoredStatus::DecodeFailed;
 const auto unavailable=f.make();check(unavailable->original_road({116,110})&&unavailable->road_mask()==p->road_mask(),"legacy/render/profile/graphics readiness cannot establish original road authority");
 f.mask.assign(count,1);check(f.make()->canonical_state()==state,"render diagnostic changes leave full policy identical");
 f.plan.height_bytes[at({117,110})]=255;check(f.make()->transport_edge_blocker({116,110},{117,110})==simulation::BuildBlocker::UnsupportedHeightTransition,"original road is not a ramp");
}
void rejected_road_forms(){
 for(const auto raw:std::array<unsigned,13>{0x40,0x44,0x4000044,0x1c0,0xc1,0xc2,0x640,0x800640,0x20400240,0x80c8,0x4080,0x20000c0,0xe0}){
  Fixture f;f.map.terrain_raw.values[at(site)]=raw;const auto p=f.make();check(!p->original_road({116,110}),"unknown/special/water/flood/elevation/structure mixes cannot become original roads");
  if(raw&0x40)check(p->original_road_classification({116,110})!=simulation::OriginalRoadClassification::GroundRoad,"known special road diagnostic never labels a normal road");
 }
 Fixture objects;objects.map.terrain_raw.values[at(site)]=0xc0;objects.map.objects_raw.values[at(site)]=1;const auto obj=objects.make();check(!obj->original_road({116,110})&&obj->original_road_classification({116,110})==simulation::OriginalRoadClassification::NonzeroObjects,"object bits reject normal road import");
 Fixture off;off.map.terrain_raw.values[at(site)]=0x800c0;check(!off.make()->original_road({116,110})&&off.make()->original_road_classification({116,110})==simulation::OriginalRoadClassification::OutsideMap,"off-map wins over road bit");
 Fixture outside;outside.map.terrain_raw.values[0]=0xc0;check(!outside.make()->original_road({0,0})&&outside.make()->original_road_classification({0,0})==simulation::OriginalRoadClassification::OutsideMap,"candidate geometry required");
}
void complete_claims_first(){
 Fixture f;f.map.terrain_raw.values[at(site)]=0xc0;f.entities.records.push_back(f.source(site));auto p=f.make();check(!p->original_road({116,110})&&p->protected_original({116,110})&&p->original_road_classification({116,110})==simulation::OriginalRoadClassification::OriginalStructure,"complete original square beats ground road");
 auto marker=f.source(site);marker.entity_class=maps::OriginalEntityClass::Industrial;marker.type=163;marker.footprint_side=0;f.entities.records[0]=marker;p=f.make();check(!p->original_road({116,110})&&p->protected_original({116,110}),"reserved marker beats ground road");
 auto gate=f.source({100,110});gate.entity_class=maps::OriginalEntityClass::GateHouse;gate.type=130;gate.footprint_side=1;gate.gate_house=maps::OriginalGateHouseState{0};gate.provenance.wrapper_schema=gate.provenance.extended_schema=1;gate.provenance.class_wrapper_schema=2;f.entities.records[0]=gate;
 for(unsigned dy=0;dy<3;++dy)for(unsigned dx=0;dx<5;++dx)f.map.terrain_raw.values[at({100+dx,110+dy})]=0x80c8;
 f.map.terrain_raw.values[at({102,109})]=f.map.terrain_raw.values[at({102,113})]=0xc0;p=f.make();
 check(p->original_road({102,109})&&p->original_road({102,113})&&p->fixed_passage({102,110})&&!p->original_road({102,110})&&!p->original_road({100,110}),"gate openings can use ground roads; fixed corridor and solids remain distinct");
 check(p->transport_edge_allowed({102,109},{102,110})&&p->transport_edge_blocker({101,111},{102,111})==simulation::BuildBlocker::GateSideEntry,"gate openings and forbidden side contract unchanged");
 f.entities.records[0].type=131;rejects([&]{f.make();},"road import admits no unknown gate class/type");
}
void version_and_canonical_contract(){
 Fixture f;f.map.terrain_raw.values[at(site)]=0xc0;const auto old=f.make(1);check(!old->original_road({116,110})&&old->road_allowed({116,110})&&!old->building_allowed({116,110})&&old->original_road_classification({116,110})==simulation::OriginalRoadClassification::GroundRoad,"old saved policy sees original road as visual-only, retaining paid placement");
 std::string expected="map-permissions:1:228:228\n";
 for(unsigned y=0;y<228;++y)for(unsigned x=0;x<228;++x){const bool outside=!f.geometry.contains({x,y});expected+=outside?"0,0,0,0,1,1;":(x==116&&y==110?"1,0,0,0,0,8;":"1,1,0,0,0,0;");}
 check(old->canonical_state()==expected,"policy1 canonical bytes retain literal former representation");
 const auto current=f.make();check(current->canonical_state().find("original-roads-format:1")!=std::string::npos&&current->canonical_state()!=old->canonical_state(),"policy2 canonically binds explicit import and classifier format");
 f.map.terrain_raw.values[at(site)]=0x80;check(f.make()->canonical_state()!=current->canonical_state(),"original road deletion changes policy2 fingerprint input");
 rejects([&]{f.make(3);},"unknown policy rejected");
 auto cells=current->cells();cells[at(site)].original_road=true;cells[at(site)].original_road_classification=simulation::OriginalRoadClassification::GroundRoad;cells[at(site)].building_allowed=false;
 rejects([&]{simulation::MapPermissions(228,228,1,cells,{});},"policy1 constructor cannot smuggle imported road authority");
 auto old_cells=old->cells();old_cells[at(site)].road_blocker=simulation::BuildBlocker::UnsupportedOriginalRoad;
 rejects([&]{simulation::MapPermissions(228,228,1,old_cells,{});},"policy1 retains its original blocker enum ceiling");
 cells[at(site)].protected_original=true;rejects([&]{simulation::MapPermissions(228,228,2,cells,{});},"original structure cannot simultaneously be an imported road");
 check(simulation::authored_simple_permissions(2,2,{1,1,1,1},2)->policy_version()==2,"explicit authored empty policy2 retains no invented roads");
}
}
int main(){try{exact_ground_road();rejected_road_forms();complete_claims_first();version_and_canonical_contract();std::cout<<"Original road classification tests passed\n";return 0;}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
