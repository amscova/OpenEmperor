#include "OriginalProductionFixture.h"
#include "core/PerformanceDiagnostics.h"
#include "simulation/original/HouseEvolution.h"
#include "simulation/original/OriginalScheduler.h"
#include "simulation/original/ServiceRules.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <string_view>

namespace {
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
namespace perf=openemperor::performance;
using f::check;
constexpr std::array<sim::CommandType,3> commands{sim::CommandType::PlaceOriginalMusicSchool,
    sim::CommandType::PlaceOriginalAcrobatSchool,sim::CommandType::PlaceOriginalDramaSchool};
constexpr std::array<sim::CourierRole,3> actor_roles{sim::CourierRole::OriginalMusic,
    sim::CourierRole::OriginalAcrobat,sim::CourierRole::OriginalDrama};
constexpr std::array<sim::Object,3> objects{sim::Object::OriginalMusicSchool211,
    sim::Object::OriginalAcrobatSchool212,sim::Object::OriginalDramaSchool213};
constexpr std::array<orig::ServiceRole,3> categories{orig::ServiceRole::Music,orig::ServiceRole::Acrobat,orig::ServiceRole::Drama};
constexpr std::array<int,3> prices{50,60,70},employees{8,9,10};
std::uint64_t countdowns(std::uint64_t t){return t<36?0:1+(t-36)/51;}
const orig::HouseState& house(const sim::World& w,sim::BuildingId id){for(const auto& h:w.original_economy_state()->houses)if(h.building_id==id)return h.house;throw std::runtime_error("Culture lost actual House");}
const sim::OriginalServiceTrip* trip(const sim::World& w,sim::CourierId id){for(const auto& row:w.original_production_state()->services.trips)if(row.transfer.courier_id==id)return &row;return nullptr;}
const sim::OriginalServiceWitness& witness(const sim::World& w,sim::BuildingId id,orig::ServiceRole role){for(const auto& row:w.original_production_state()->services.witnesses)if(row.building_id==id&&row.role==role)return row;throw std::runtime_error("Culture lost actual visit witness");}
sim::BuildingId buy(sim::World& w,sim::CommandType type,sim::Cell cell){f::buy(w,{type,cell});return *w.building_owner_at(cell);}
sim::CourierId actor(const sim::World& w,sim::BuildingId owner){for(const auto& c:w.couriers())if(c.owner==owner)return c.id;throw std::runtime_error("Paid culture source has no own actor");}
void edit(std::string& text,unsigned field){const std::string marker="3: Plain Cottage,{,";auto at=text.find(marker);check(at!=std::string::npos,"Culture Plain row missing");at+=marker.size();for(unsigned i=0;i<field;++i){const auto comma=text.find(',',at);check(comma!=std::string::npos,"Culture model field missing");at=comma+1;}const auto end=text.find(',',at);check(text.substr(at,end-at)=="0","Original Plain culture requirement changed");text.replace(at,end-at,"1");}
struct Fixture{
    f::Root root;std::shared_ptr<const sim::MapPermissions> policy=f::permissions();std::shared_ptr<const orig::OriginalRulesModel> model;
    explicit Fixture(const f::fs::path& real={},int need=-1){auto bytes=real.empty()?oe_production_fixture::authored_model_text(7):f::read_text(real/"Model/EmperorBuildingModels.txt");if(need>=0)edit(bytes,5U+static_cast<unsigned>(need));f::write_text(root.data/"Model/EmperorBuildingModels.txt",bytes);model=f::save::prepare_original_production_rules_model(root.data);}
    sim::World world()const{return sim::World::create_original_production_development(policy,model);}
    sim::World restore(const sim::World& w)const{return sim::World::restore_original_production_development(w.snapshot(),policy,model);}
};
void authority(const sim::World& w){
    check(w.production_balance_valid()&&w.population_valid()&&w.navigation_valid()&&w.city_economy_valid(),"Culture World invariant failed");
    const auto& state=w.original_production_state()->services;check(state.providers.size()<=18&&state.trips.size()<=18&&state.witnesses.size()<=180,"Culture bounded authority grew");
    for(const auto& h:w.original_economy_state()->houses)for(unsigned r=0;r<9;++r){
        std::uint8_t expected=0;for(const auto& x:state.witnesses)if(x.building_id==h.building_id&&static_cast<unsigned>(x.role)==r){const auto duration=r<5?96U:40U;const auto elapsed=countdowns(w.ticks())-countdowns(x.arrival_tick-1);expected=elapsed>=duration?0:static_cast<std::uint8_t>(duration-elapsed);}
        check(orig::service_timer(h.house,static_cast<orig::ServiceRole>(r))==expected,"Culture timer lost independent actual-arrival clock");
    }
    for(const auto& c:w.couriers())if(std::find(actor_roles.begin(),actor_roles.end(),c.role)!=actor_roles.end())check(!c.cargo&&!c.reserved&&!w.original_courier_cargo(c.id),"School actor acquired material/performer cargo");
    for(const auto& b:w.buildings())check(!b.population&&!b.service_until_tick&&!b.health_protection_until_tick&&!b.health_risk&&!b.sick_until_tick,"Culture granted legacy coverage/health/population");
}
void ticks(sim::World& w,int n){for(int i=0;i<n;++i){w.tick();authority(w);}}
template<class P>void until(sim::World& w,P predicate,const char* message,int limit=20000){for(int i=0;i<limit&&!predicate();++i){w.tick();authority(w);}check(predicate(),message);}
void roads(sim::World& w){for(int x=0;x<24;++x)f::buy(w,{sim::CommandType::PlaceRoad,{x,4}});}
sim::BuildingId homes(sim::World& w,int n=2){sim::BuildingId first{};for(int i=0;i<n;++i){const auto id=buy(w,sim::CommandType::PlaceHousehold,{3*i,2});if(!i)first=id;}return first;}
void diagnostic_is_read_only(const sim::World& w,sim::BuildingId id){
    const auto before=w.snapshot();perf::set_enabled(true);perf::reset();
    for(int i=0;i<100;++i)for(unsigned r=0;r<9;++r)(void)w.original_service_availability(id,static_cast<orig::ServiceRole>(r));
    for(const auto c:{perf::Counter::WorldCopies,perf::Counter::WorldRestores,perf::Counter::WorldExecutes,perf::Counter::BfsCalls,perf::Counter::RouteRefreshes,perf::Counter::SimulationTicks,perf::Counter::FileReads,perf::Counter::FileWrites,perf::Counter::AssetDecodes,perf::Counter::TextureUploads})check(perf::counter(c)==0,"Availability diagnostic mutated or prepared World/navigation/assets");
    bool refused=false;try{(void)w.original_service_availability(id,static_cast<orig::ServiceRole>(9));}catch(const std::invalid_argument&){refused=true;}
    check(refused,"Availability accepted an invalid service role");
    perf::set_enabled(false);check(w.snapshot()==before,"Availability diagnostic changed actual authority");
}
void paid_single_category_pause_expiry_and_replay(const f::fs::path& real){
    for(unsigned index=0;index<3;++index){Fixture fixture(real);auto w=fixture.world();roads(w);const auto home=homes(w);const auto before=w.treasury();const auto source=buy(w,commands[index],{20,5});const auto id=actor(w,source);
        check(w.treasury()==before-prices[index]&&w.workforce_required(source)==employees[index]&&w.courier(id).role==actor_roles[index],"Culture actual price/workers/own actor differs");
        check(sim::building_footprint(w.profile(),w.rule_version(),objects[index])==sim::BuildingFootprint{index?3:2,index?3:2},"Culture source lost actual2x2/3x3 footprint");
        auto available=w.original_service_availability(home,categories[index]);check(!available.eligible_target&&available.provider_count==1&&!available.available_source_count&&!available.delivered_timer&&!available.last_arrival_tick,"Unoccupied/unfunded staff source supplied culture");
        ticks(w,119);check(w.original_production_state()->services.trips.empty(),"Unstaffed school dispatched before genuine immigration");
        until(w,[&]{return w.courier(id).phase==sim::CourierPhase::ToWarehouse&&w.courier(id).edge_progress>0;},"Paid school did not enter actual movement");
        available=w.original_service_availability(home,categories[index]);check(available.eligible_target&&available.provider_count==1&&available.available_source_count==1&&available.reachable_source_count==1&&available.travelling_to_target_count==1&&!available.delivered_timer&&!available.last_arrival_tick,"Available/reachable/inflight school was confused with delivered access");diagnostic_is_read_only(w,home);
        const auto begun=w.snapshot().couriers;f::buy(w,sim::set_building_operation(source,false));check(w.snapshot().couriers==begun,"School pause cancelled begun edge");
        available=w.original_service_availability(home,categories[index]);check(!available.available_source_count&&!available.reachable_source_count&&available.travelling_to_target_count==1&&!available.delivered_timer,"Paused inflight school lost physical travelling distinction");
        auto replay=fixture.restore(w);until(w,[&]{return w.courier(id).phase==sim::CourierPhase::IdleAtWorkshop;},"Paused school tour did not finish");
        while(replay.ticks()<w.ticks()){replay.tick();}
        check(replay.snapshot()==w.snapshot(),"Actual paused culture trip F5/F9 diverged");
        const auto first=witness(w,home,categories[index]);check(trip(w,id)&&trip(w,id)->visit_applied&&orig::service_timer(house(w,home),categories[index])>0,"School actual arrival did not grant own96 timer");
        available=w.original_service_availability(home,categories[index]);check(!available.available_source_count&&!available.reachable_source_count&&available.delivered_timer>0&&available.last_arrival_tick==first.arrival_tick&&!available.travelling_to_target_count,"Paused source erased real delivered access or retained pending travel");
        for(unsigned r=0;r<9;++r)if(r!=index+2)check(!orig::service_timer(house(w,home),static_cast<orig::ServiceRole>(r)),"One culture arrival granted another category");
        std::array<bool,16> phases{};bool expired=false;for(int i=0;i<20000;++i){const auto dispatch=orig::next_scheduler_dispatch(w.ticks());const auto prior=orig::service_timer(house(w,home),categories[index]);w.tick();replay.tick();authority(w);check(w.snapshot()==replay.snapshot(),"Single culture full20k Snapshot replay changed authority");if(dispatch&&dispatch->service_countdown){phases[dispatch->position.step]=true;expired|=prior==1&&!orig::service_timer(house(w,home),categories[index]);}}
        check(expired&&std::all_of(phases.begin(),phases.end(),[](bool x){return x;})&&witness(w,home,categories[index])==first,"Culture96 expiry lost exact1->0/origin/all16 scheduler phases");
        available=w.original_service_availability(home,categories[index]);check(!available.delivered_timer&&available.last_arrival_tick==first.arrival_tick,"Expired culture lost history or retained access");diagnostic_is_read_only(w,home);
    }
}
void no_road_and_staffed_water_barrier(const f::fs::path& real){
    for(unsigned index=0;index<3;++index){Fixture f(real);auto w=f.world();roads(w);const auto home=homes(w);(void)buy(w,commands[index],{20,9});ticks(w,600);check(w.total_population()==14&&w.original_production_state()->services.trips.empty()&&!orig::service_timer(house(w,home),categories[index]),"NoRoad school dispatched or granted a visit");diagnostic_is_read_only(w,home);
        auto cells=f.policy->cells();for(int y=0;y<20;++y)cells[static_cast<std::size_t>(y*24+11)]={false,false,false,0,sim::BuildBlocker::UnsupportedBridgeWater,sim::BuildBlocker::UnsupportedTerrain,false,sim::OriginalRoadClassification::WaterOrBridge,true};f.policy=std::make_shared<const sim::MapPermissions>(24,20,3,std::move(cells),std::vector<sim::FixedGatePassage>{});auto barrier=f.world();for(int x=0;x<24;++x)if(x!=11)f::buy(barrier,{sim::CommandType::PlaceRoad,{x,4}});const auto target=homes(barrier);const auto source=buy(barrier,commands[index],{20,5});ticks(barrier,600);const auto a=barrier.original_service_availability(target,categories[index]);check(barrier.workers_assigned(source)==employees[index]&&a.eligible_target&&a.available_source_count==1&&!a.reachable_source_count&&!a.travelling_to_target_count&&!a.delivered_timer&&!a.last_arrival_tick,"Staffed school crossed policy water or availability granted delivered access");diagnostic_is_read_only(barrier,target);
    }
}
void actual_source_and_target_fire(const f::fs::path& real){
    for(unsigned index=0;index<3;++index){Fixture f(real);auto source_fire=f.world();roads(source_fire);const auto source=buy(source_fire,commands[index],{20,5});f::buy(source_fire,sim::set_building_operation(source,false));ticks(source_fire,600);const auto home=homes(source_fire);while(source_fire.ticks()<1990)source_fire.tick();f::buy(source_fire,sim::set_building_operation(source,true));const auto id=actor(source_fire,source);until(source_fire,[&]{return source_fire.courier(id).phase==sim::CourierPhase::ToWarehouse&&source_fire.courier(id).edge_progress>0;},"School did not dispatch before source fire");until(source_fire,[&]{return source_fire.building_on_fire(source);},"Actual school source never burned",30);check(trip(source_fire,id)&&!trip(source_fire,id)->arrival_tick,"School fire scenario arrived too early");until(source_fire,[&]{return trip(source_fire,id)->arrival_tick.has_value();},"Source fire cancelled actual culture arrival",500);check(trip(source_fire,id)->visit_applied&&source_fire.building_on_fire(source)&&orig::service_timer(house(source_fire,home),categories[index])>0,"Source fire erased existing school tour");const auto a=source_fire.original_service_availability(home,categories[index]);check(!a.available_source_count&&a.delivered_timer>0,"Burning source diagnostic confused availability with actual prior arrival");
        auto target_fire=f.world();roads(target_fire);const auto target=homes(target_fire);ticks(target_fire,1000);const auto late=buy(target_fire,commands[index],{20,5});f::buy(target_fire,sim::set_building_operation(late,false));while(target_fire.ticks()<1990)target_fire.tick();f::buy(target_fire,sim::set_building_operation(late,true));const auto visitor=actor(target_fire,late);until(target_fire,[&]{const auto* t=trip(target_fire,visitor);return t&&t->arrival_tick;},"Actual culture target-fire arrival absent",500);check(target_fire.building_on_fire(target)&&!trip(target_fire,visitor)->visit_applied&&!orig::service_timer(house(target_fire,target),categories[index])&&target_fire.original_production_state()->services.witnesses.empty(),"Burning House received culture");
    }
}
void actual_countdown_boundary_and_far_edge(const f::fs::path& real){
    for(unsigned index=0;index<3;++index){Fixture f(real);auto base=f.world();roads(base);const auto home=homes(base);ticks(base,900);
        const auto arrive=[&](sim::World& w){const auto source=buy(w,commands[index],{20,5});const auto id=actor(w,source);until(w,[&]{const auto* t=trip(w,id);return t&&t->arrival_tick;},"Aligned culture actual arrival missing");return *trip(w,id)->arrival_tick;};
        auto probe=f.restore(base);const auto first=arrive(probe);for(const auto phase:{36U,37U}){auto w=f.restore(base);ticks(w,static_cast<int>((phase+51-first%51)%51));const auto at=arrive(w);check(at%51==phase&&orig::service_timer(house(w,home),categories[index])==(phase==36?95:96),"Actual culture arrival did not apply own96 and phase35 exactly once");}
        if(index){auto edge=f.world();for(const int y:{4,8})for(int x=18;x<24;++x)f::buy(edge,{sim::CommandType::PlaceRoad,{x,y}});for(int y=5;y<8;++y)f::buy(edge,{sim::CommandType::PlaceRoad,{23,y}});(void)buy(edge,sim::CommandType::PlaceHousehold,{21,2});(void)buy(edge,sim::CommandType::PlaceHousehold,{21,9});const auto source=buy(edge,commands[index],{18,5});const auto id=actor(edge,source);until(edge,[&]{return edge.courier(id).phase==sim::CourierPhase::ToWarehouse&&edge.courier(id).edge_progress>0;},"Actual3x3 culture far-edge trip missing");check(edge.courier(id).path.front().x==20||edge.courier(id).path.front().y==7,"Culture3x3 route did not exercise source+2 edge");auto restored=f.restore(edge);for(int t=0;t<2000;++t){edge.tick();restored.tick();check(edge.snapshot()==restored.snapshot(),"Culture3x3 edge restore changed physical movement");}}
    }
}
std::optional<sim::BuildingId> expected_target(const sim::World& w,const sim::CourierState& c){std::vector<sim::BuildingId> ids;for(const auto& h:w.original_economy_state()->houses)if(h.house.occupants>0&&h.access_enabled&&(h.original_status==1||h.original_status==3)&&w.building(h.building_id).fire_until_tick<=w.ticks()+1)ids.push_back(h.building_id);if(ids.empty())return{};std::size_t first=0;for(std::size_t i=0;i<ids.size();++i)if(c.last_dispatched_target==ids[i])first=(i+1)%ids.size();for(std::size_t i=0;i<ids.size();++i)for(const auto& route:c.dynamic_target_routes)if(route.first==ids[(first+i)%ids.size()]&&route.second)return route.first;return{};}
void simultaneous_nine_and_private_competition(const f::fs::path& real){
    Fixture f(real);auto w=f.world();roads(w);const auto home=homes(w,8);(void)buy(w,sim::CommandType::PlaceHousehold,{0,0});(void)buy(w,sim::CommandType::PlaceHousehold,{3,0});for(int y=0;y<4;++y)f::buy(w,{sim::CommandType::PlaceRoad,{2,y}});for(int x=0;x<=12;++x)f::buy(w,{sim::CommandType::PlaceRoad,{x,8}});for(int y=5;y<8;++y)f::buy(w,{sim::CommandType::PlaceRoad,{12,y}});
    std::vector<sim::BuildingId> sources;for(const auto& [type,cell]:std::array{std::pair{sim::CommandType::PlaceOriginalDaoistShrine,sim::Cell{10,5}},std::pair{sim::CommandType::PlaceOriginalBuddhistShrine,sim::Cell{14,5}},std::pair{sim::CommandType::PlaceOriginalConfucianAcademy,sim::Cell{18,5}},std::pair{sim::CommandType::PlaceHealthPost,sim::Cell{0,5}},std::pair{sim::CommandType::PlaceServicePost,sim::Cell{3,5}},std::pair{sim::CommandType::PlaceOriginalAncestorShrine,sim::Cell{6,5}},std::pair{sim::CommandType::PlaceOriginalWaterWell,sim::Cell{22,5}}})sources.push_back(buy(w,type,cell));std::array<sim::CourierId,3> actors{};for(unsigned i=0;i<3;++i){const auto source=buy(w,commands[i],{4*static_cast<int>(i),9});sources.push_back(source);actors[i]=actor(w,source);}
    oe_production_fixture::PaperSupplyLayout kit;kit.farm={0,17};kit.field={2,17};kit.paper={4,17};kit.warehouse={8,17};
    kit.extra_homes={{6,0},{9,0},{12,0},{15,0},{18,0}};
    for(const int x:{5,8,11,14,17})for(int y=0;y<4;++y)kit.roads.push_back({x,y});
    for(int x=0;x<=12;++x)kit.roads.push_back({x,16});for(int y=9;y<16;++y)kit.roads.push_back({12,y});
    const auto paper=oe_production_fixture::add_paper_supply(w,kit);check(paper.spent==744,"Culture Paper kit cost differs");
    check(w.construction_spent_total()==2127&&!w.total_population()&&w.buildings().size()==29&&w.couriers().size()==14,"Nine-category scene did not pay exact2127 with only real bodies/actors");for(const auto source:sources)f::buy(w,sim::set_building_operation(source,false));ticks(w,400);check(w.total_population()==105&&w.workforce_supply()==105,"Full real-school scene lacks105 actual admitted workers");until(w,[&]{for(const auto& row:w.original_production_state()->industry.auxiliary)if(row.building_id==sources[2])return row.amounts[4]>0;return false;},"Actual paid Paper did not reach paused Academy");for(const auto source:sources)f::buy(w,sim::set_building_operation(source,true));until(w,[&]{return w.workforce_used()==100&&std::all_of(actors.begin(),actors.end(),[&](auto id){return w.courier(id).phase==sim::CourierPhase::ToWarehouse;});},"All schools never dispatched with actual100 workforce");until(w,[&]{const auto& h=house(w,home);for(unsigned r=0;r<9;++r)if(!orig::service_timer(h,static_cast<orig::ServiceRole>(r)))return false;return h.water;},"Actual nine categories plus Water never reached one House");diagnostic_is_read_only(w,home);
    for(int i=0;i<12000;++i){std::array<std::uint64_t,3> sequences{};std::array<std::optional<sim::BuildingId>,3> targets{};for(unsigned r=0;r<3;++r){const auto* t=trip(w,actors[r]);sequences[r]=t?t->transfer.sequence:0;targets[r]=expected_target(w,w.courier(actors[r]));}w.tick();authority(w);for(unsigned r=0;r<3;++r){const auto* t=trip(w,actors[r]);if(t&&t->transfer.sequence!=sequences[r])check(targets[r]&&t->target==*targets[r],"Cultural actors lost private reachable-target fairness");}}
    for(const auto& h:w.original_economy_state()->houses)for(const auto role:categories)(void)witness(w,h.building_id,role);
}
void authored_need1_actual_prerequisites(const f::fs::path& real){
    for(unsigned index=0;index<3;++index){Fixture f(real,static_cast<int>(index));auto w=f.world();roads(w);const auto home=homes(w,6);const auto farm=buy(w,sim::CommandType::PlaceOriginalFoodFarm,{0,5});sim::Command field{sim::CommandType::PlaceOriginalWheatField,{3,5}};field.building_id=farm;f::buy(w,field);(void)buy(w,sim::CommandType::PlaceMarket,{5,5});(void)buy(w,sim::CommandType::PlaceOriginalWaterWell,{9,5});(void)buy(w,sim::CommandType::PlaceOriginalAncestorShrine,{13,5});const auto source=buy(w,commands[index],{17,5});f::buy(w,sim::set_building_operation(source,false));const auto supplied=[&]{const auto& h=house(w,home);return h.water&&h.food_quality>=20&&h.ancestor;};until(w,supplied,"Culture Need1 control lacked actual other prerequisites");check(house(w,home).type==orig::HouseType::Hut&&!orig::needs_met(f.model->house_rules(),orig::ModelHouse::PlainCottage,house(w,home)),"Missing actual culture satisfied authored Plain Need1");f::buy(w,sim::set_building_operation(source,true));until(w,[&]{return house(w,home).type==orig::HouseType::PlainCottage;},"Genuine culture arrival never upgraded authored Plain");const auto& histories=w.original_production_state()->housing.houses;const auto row=std::find_if(histories.begin(),histories.end(),[&](const auto& h){return h.building_id==home;});check(row!=histories.end()&&row->first_plain_evaluation&&row->first_plain_evaluation->service_witnesses[6+index]&&row->first_plain_evaluation->service_witnesses[6+index]->role==categories[index],"Authored culture first Plain reindexed an existing historical slot");f::buy(w,sim::set_building_operation(source,false));until(w,[&]{return !orig::service_timer(house(w,home),categories[index])&&supplied();},"Expired culture control never regained actual remaining prerequisites");check(!orig::needs_met(f.model->house_rules(),orig::ModelHouse::PlainCottage,house(w,home)),"Expired actual culture retained authored Need1 access");
    }
}
}
int main(int argc,char** argv){try{check(argc==1||(argc==3&&std::string_view(argv[1])=="--real"),"Usage: original-culture-world [--real DATA]");const auto real=argc==3?f::fs::canonical(argv[2]):f::fs::path{};paid_single_category_pause_expiry_and_replay(real);no_road_and_staffed_water_barrier(real);actual_source_and_target_fire(real);actual_countdown_boundary_and_far_edge(real);simultaneous_nine_and_private_competition(real);authored_need1_actual_prerequisites(real);std::cout<<"Actual paid211/212/213 visits/independent96 clocks, nine-category competition, read-only availability, fire/routes/20k and authored Need1 PASS; experimental targeted visits, no original venue/performer pipeline\n";return 0;}catch(const std::exception& e){perf::set_enabled(false);std::cerr<<"Original Culture World FAIL: "<<e.what()<<'\n';return 1;}}
