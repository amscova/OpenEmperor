#include "OriginalProductionFixture.h"
#include "simulation/original/HouseEvolution.h"
#include "simulation/original/OriginalScheduler.h"
#include "simulation/original/ServiceRules.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <map>
#include <set>
#include <string_view>

namespace {
namespace f=oe_influence_fixture;namespace sim=openemperor::simulation;namespace orig=openemperor::original;
using f::check;
constexpr std::array<orig::ServiceRole,3> faiths{orig::ServiceRole::Daoist,orig::ServiceRole::Buddhist,orig::ServiceRole::Confucian};
constexpr std::array<sim::CommandType,3> commands{sim::CommandType::PlaceOriginalDaoistShrine,sim::CommandType::PlaceOriginalBuddhistShrine,sim::CommandType::PlaceOriginalConfucianAcademy};
constexpr std::array<sim::CourierRole,3> roles{sim::CourierRole::OriginalDaoist,sim::CourierRole::OriginalBuddhist,sim::CourierRole::OriginalConfucian};
std::uint64_t countdowns(std::uint64_t t){return t<36?0:1+(t-36)/51;}
const orig::HouseState& house(const sim::World& w,sim::BuildingId id){for(const auto& h:w.original_economy_state()->houses)if(h.building_id==id)return h.house;throw std::runtime_error("Religion lost paid House");}
const sim::OriginalServiceTrip* trip(const sim::World& w,sim::CourierId id){for(const auto& t:w.original_production_state()->services.trips)if(t.transfer.courier_id==id)return &t;return nullptr;}
const sim::OriginalServiceWitness& witness(const sim::World& w,sim::BuildingId id,orig::ServiceRole role){for(const auto& x:w.original_production_state()->services.witnesses)if(x.building_id==id&&x.role==role)return x;throw std::runtime_error("Religion lost genuine arrival witness");}
sim::CourierId actor(const sim::World& w,sim::BuildingId owner){for(const auto& c:w.couriers())if(c.owner==owner)return c.id;throw std::runtime_error("Religion source has no real actor");}
sim::BuildingId buy(sim::World& w,sim::CommandType c,sim::Cell p){f::buy(w,{c,p});return *w.building_owner_at(p);}
oe_production_fixture::PaperSupply paper_kit(sim::World& w,const std::vector<sim::Cell>& homes={},bool far=false,bool elite_layout=false){
    oe_production_fixture::PaperSupplyLayout layout;layout.farm={0,17};layout.field={2,17};layout.paper={4,17};layout.warehouse={8,17};layout.extra_homes=homes;
    for(int x=0;x<=12;++x)layout.roads.push_back({x,16});for(int y=5;y<16;++y)layout.roads.push_back({12,y});
    if(far){for(int x=0;x<18;++x)layout.roads.push_back({x,4});for(int x=12;x<18;++x)layout.roads.push_back({x,8});}
    else for(const int x:(elite_layout ? std::array{6,9}:std::array{2,5}))for(int y=0;y<4;++y)layout.roads.push_back({x,y});
    return oe_production_fixture::add_paper_supply(w,layout);
}
void edit(std::string& text,unsigned field,int value){const std::string marker="3: Plain Cottage,{,";auto at=text.find(marker);check(at!=std::string::npos,"Authored Plain model row missing");at+=marker.size();for(unsigned i=0;i<field;++i){at=text.find(',',at);check(at!=std::string::npos,"Authored Plain field missing");++at;}const auto end=text.find(',',at);check(end!=std::string::npos,"Authored Plain value missing");text.replace(at,end-at,std::to_string(value));}
struct Fixture{
    f::Root root;std::shared_ptr<const sim::MapPermissions> policy=f::permissions();std::shared_ptr<const orig::OriginalRulesModel> model;
    explicit Fixture(const f::fs::path& real={},int authored_needs=-1){
        auto bytes=real.empty()?oe_production_fixture::authored_model_text(7):f::read_text(real/"Model/EmperorBuildingModels.txt");
        if(authored_needs>=0){edit(bytes,23,1);if(authored_needs>=3)edit(bytes,22,1);}
        f::write_text(root.data/"Model/EmperorBuildingModels.txt",bytes);model=f::save::prepare_original_production_rules_model(root.data);
    }
    sim::World world()const{return sim::World::create_original_production_development(policy,model);}
    sim::World restore(const sim::World& w)const{return sim::World::restore_original_production_development(w.snapshot(),policy,model);}
};
void authority(const sim::World& w){
    check(w.production_balance_valid()&&w.population_valid()&&w.navigation_valid()&&w.city_economy_valid(),"Religion World invariant failed");
    const auto& s=w.original_production_state()->services;check(s.providers.size()<=18&&s.trips.size()<=18&&s.witnesses.size()<=180,"Religion bounded history grew");
    for(const auto& h:w.original_economy_state()->houses)for(unsigned r=0;r<9;++r){
        std::uint8_t expected=0;
        for(const auto& x:s.witnesses)if(x.building_id==h.building_id&&static_cast<unsigned>(x.role)==r){
            const auto duration=r>=5?40U:96U;const auto age=countdowns(w.ticks())-countdowns(x.arrival_tick-1);expected=age>=duration?0:static_cast<std::uint8_t>(duration-age);
        }
        check(orig::service_timer(h.house,static_cast<orig::ServiceRole>(r))==expected,"Religion visit overwrote another category or lost phase35 origin");
        if(r>=2&&r<=4)check(!expected,"Unsupported culture role was granted by religion");
    }
    for(const auto& c:w.couriers())if(std::find(roles.begin(),roles.end(),c.role)!=roles.end())
        check(!c.cargo&&!c.reserved&&!w.original_courier_cargo(c.id),"Religion actor acquired physical cargo");
    for(const auto& b:w.buildings())check(!b.population&&!b.service_until_tick&&!b.health_protection_until_tick&&!b.input_clay,
        "Religion-only scene invented legacy population/coverage/resource stock");
}
template<class P>void until(sim::World& w,P done,const char* reason,int limit=20000){for(int i=0;i<limit&&!done();++i){w.tick();authority(w);}check(done(),reason);}
void ticks(sim::World& w,int n){for(int i=0;i<n;++i){w.tick();authority(w);}}
struct Scene{sim::BuildingId home{};std::array<sim::BuildingId,3> providers{};};
Scene scene(sim::World& w,bool prior=true){
    for(int x=0;x<24;++x)f::buy(w,{sim::CommandType::PlaceRoad,{x,4}});
    Scene s;for(int x=0;x<24;x+=3){const auto id=buy(w,x ? sim::CommandType::PlaceHousehold:sim::CommandType::PlaceOriginalElitePlot,oe_production_fixture::elite_row_cell(x/3));if(!x)s.home=id;}
    const auto before=w.construction_spent_total();
    for(unsigned i=0;i<3;++i)s.providers[i]=buy(w,commands[i],{10+4*static_cast<int>(i),5});
    check(w.construction_spent_total()-before==170,"Three paid religion sources must independently cost45+45+80=170");
    if(prior){(void)buy(w,sim::CommandType::PlaceHealthPost,{0,5});(void)buy(w,sim::CommandType::PlaceServicePost,{3,5});(void)buy(w,sim::CommandType::PlaceOriginalAncestorShrine,{6,5});(void)buy(w,sim::CommandType::PlaceOriginalWaterWell,{22,5});}
    const auto kit=paper_kit(w,{{4,0},{7,0},{10,0}},false,true);check(kit.spent==568,"Paid religion Paper kit/home/roads cost differs");
    check(w.construction_spent_total()==static_cast<std::uint64_t>(prior?1571:1426)&&!w.total_population(),"Religion scene bypassed normal cost/population origin");return s;
}
std::optional<sim::BuildingId> expected_target(const sim::World& w,const sim::CourierState& c){
    std::vector<sim::BuildingId> ids;
    for(const auto& h:w.original_economy_state()->houses)if(h.house.occupants>0&&h.access_enabled&&(h.original_status==1||h.original_status==3)&&w.building(h.building_id).fire_until_tick<=w.ticks()+1&&(c.role!=sim::CourierRole::OriginalConfucian||orig::is_elite(h.house.type)))ids.push_back(h.building_id);
    if(ids.empty())return{};std::size_t first=0;
    for(std::size_t i=0;i<ids.size();++i)if(c.last_dispatched_target==ids[i])first=(i+1)%ids.size();
    for(std::size_t i=0;i<ids.size();++i){const auto id=ids[(first+i)%ids.size()];for(const auto& route:c.dynamic_target_routes)if(route.first==id&&route.second)return id;}
    return{};
}
void paid_independent_visits_expiry_revisit_and_private_fairness(const f::fs::path& real){
    Fixture f(real);auto w=f.world();const auto s=scene(w);std::array<sim::CourierId,3> ids{};
    for(unsigned i=0;i<3;++i){ids[i]=actor(w,s.providers[i]);check(w.courier(ids[i]).role==roles[i],"Religion source actor has wrong role");}
    check(w.workforce_required(s.providers[0])==4&&w.workforce_required(s.providers[1])==4&&w.workforce_required(s.providers[2])==12,
          "Religion real model workforce changed");
    check(sim::building_footprint(w.profile(),w.rule_version(),sim::Object::OriginalConfucianAcademy219)==sim::BuildingFootprint{3,3},"Confucian Academy is not its actual3x3 body");
    ticks(w,119);check(w.original_production_state()->services.trips.empty(),"Unstaffed religious source dispatched");
    until(w,[&]{return std::all_of(ids.begin(),ids.end(),[&](auto id){return w.courier(id).phase==sim::CourierPhase::ToWarehouse&&w.courier(id).edge_progress>0;});},"Three genuine religion actors never moved together");
    auto replay=f.restore(w);
    for(int i=0;i<10000;++i){
        const auto& h=house(w,s.home);if(h.daoist&&h.buddhist&&h.confucian&&h.ancestor&&h.herbalist&&h.acupuncture&&h.water)break;
        w.tick();replay.tick();authority(w);check(w.snapshot()==replay.snapshot(),"Mid-religion F5/F9 changed real simultaneous visits");
    }
    const auto& h=house(w,s.home);check(h.daoist&&h.buddhist&&h.confucian&&h.ancestor&&h.herbalist&&h.acupuncture&&h.water,
        "Three religion visits lost independent Ancestor/Medical/Water");
    check(w.workforce_used()==73&&w.workforce_supply()>=73,"Religion staffing lacks genuine admitted residents");
    const auto before=w.snapshot().couriers;
    for(const auto p:s.providers)f::buy(w,sim::set_building_operation(p,false));
    check(w.snapshot().couriers==before,"Source pause cancelled a begun religious tour or changed another cursor");
    until(w,[&]{return std::all_of(ids.begin(),ids.end(),[&](auto id){return w.courier(id).phase==sim::CourierPhase::IdleAtWorkshop;});},"Paused outbound religious tours did not finish");
    std::array<sim::OriginalServiceWitness,3> visits{};for(unsigned i=0;i<3;++i)visits[i]=witness(w,s.home,faiths[i]);
    replay=f.restore(w);std::array<bool,16> steps{};std::array<bool,3> expired{};
    for(int i=0;i<20000;++i){const auto dispatch=orig::next_scheduler_dispatch(w.ticks());std::array<unsigned,3> prev{};for(unsigned r=0;r<3;++r)prev[r]=orig::service_timer(house(w,s.home),faiths[r]);
        w.tick();replay.tick();authority(w);check(w.snapshot()==replay.snapshot(),"Religion expiry full20k replay diverged");
        if(dispatch&&dispatch->service_countdown){steps[dispatch->position.step]=true;for(unsigned r=0;r<3;++r)expired[r]|=prev[r]==1&&!orig::service_timer(house(w,s.home),faiths[r]);}
    }
    check(std::all_of(steps.begin(),steps.end(),[](bool x){return x;})&&std::all_of(expired.begin(),expired.end(),[](bool x){return x;}),"Religion expiry missed an actual1->0 or scheduler step");
    f::buy(w,sim::set_building_operation(s.providers[0],true));
    until(w,[&]{return house(w,s.home).daoist>0&&witness(w,s.home,faiths[0]).transfer.sequence>visits[0].transfer.sequence;},"Actual Daoist rotation did not revisit after expiry");
    check(!house(w,s.home).buddhist&&!house(w,s.home).confucian,"Daoist revisit granted another religion");
    for(unsigned i=1;i<3;++i)f::buy(w,sim::set_building_operation(s.providers[i],true));
    for(int i=0;i<12000;++i){std::array<std::uint64_t,3> sequence{};std::array<std::optional<sim::BuildingId>,3> targets{};
        for(unsigned r=0;r<3;++r){const auto* t=trip(w,ids[r]);sequence[r]=t?t->transfer.sequence:0;targets[r]=expected_target(w,w.courier(ids[r]));}
        w.tick();authority(w);
        for(unsigned r=0;r<3;++r){const auto* t=trip(w,ids[r]);if(t&&t->transfer.sequence!=sequence[r])check(targets[r]&&t->target==*targets[r],"Religion dispatch lost its private cyclic reachable-target order");}
    }
    for(const auto& home:w.original_economy_state()->houses) for(const auto role:faiths) {
        if (role==orig::ServiceRole::Confucian&&!orig::is_elite(home.house.type)) {
            check(!home.house.confucian&&std::none_of(w.original_production_state()->services.witnesses.begin(),
                w.original_production_state()->services.witnesses.end(),[&](const auto& visit) {
                    return visit.building_id==home.building_id&&visit.role==role;
                }),"Original Elite-only source left a forged Common visit/history");
        } else (void)witness(w,home.building_id,role);
    }
}
void actual_religion_source_and_target_fire(const f::fs::path& real){
    for(unsigned index=0;index<3;++index){
        Fixture fixture(real);auto w=fixture.world();
        for(int x=0;x<24;++x)f::buy(w,{sim::CommandType::PlaceRoad,{x,4}});
        const auto source=buy(w,commands[index],{20,5});
        check(sim::fire_eligible(w.building(source).kind),"Active religion source is not fire eligible");
        f::buy(w,sim::set_building_operation(source,false));ticks(w,600);
        const auto home=buy(w,index==2 ? sim::CommandType::PlaceOriginalElitePlot:sim::CommandType::PlaceHousehold,index==2 ? sim::Cell{0,0}:sim::Cell{0,2});
        (void)buy(w,sim::CommandType::PlaceHousehold,index==2 ? sim::Cell{4,2}:sim::Cell{3,2});
        if(index==2)(void)paper_kit(w,{{7,2},{10,2},{13,2},{16,2},{19,2}},false,true);
        while(w.ticks()<1990)w.tick();
        f::buy(w,sim::set_building_operation(source,true));const auto id=actor(w,source);
        until(w,[&]{return w.courier(id).phase==sim::CourierPhase::ToWarehouse&&w.courier(id).edge_progress>0;},
            "Religion did not begin actual trip before source fire");
        until(w,[&]{return w.building_on_fire(source);},"Actual religion source incident did not occur",30);
        check(trip(w,id)&&!trip(w,id)->arrival_tick,"Religion source-fire scene arrived before intended incident");
        until(w,[&]{return trip(w,id)->arrival_tick.has_value();},"Later source fire cancelled existing religion visit",500);
        check(trip(w,id)->visit_applied&&w.building_on_fire(source)&&orig::service_timer(house(w,home),faiths[index])>0,
            "Burning religious source erased genuine inflight arrival");
        const auto available=w.original_service_availability(home,faiths[index]);
        check(!available.available_source_count&&available.delivered_timer>0,
            "Religion source fire confused available staff with delivered access");
        auto replay=fixture.restore(w);
        for(int i=0;i<1000;++i){w.tick();replay.tick();authority(w);check(w.snapshot()==replay.snapshot(),
            "Burning religion source full Snapshot restore diverged");}
        auto target=fixture.world();
        for(int x=0;x<24;++x)f::buy(target,{sim::CommandType::PlaceRoad,{x,4}});
        const auto first=buy(target,index==2 ? sim::CommandType::PlaceOriginalElitePlot:sim::CommandType::PlaceHousehold,index==2 ? sim::Cell{0,0}:sim::Cell{0,2});
        (void)buy(target,sim::CommandType::PlaceHousehold,index==2 ? sim::Cell{4,2}:sim::Cell{3,2});
        if(index==2)(void)paper_kit(target,{{7,2},{10,2},{13,2},{16,2},{19,2}},false,true);
        ticks(target,1000);
        const auto delayed=buy(target,commands[index],{20,5});f::buy(target,sim::set_building_operation(delayed,false));
        while(target.ticks()<1990)target.tick();
        f::buy(target,sim::set_building_operation(delayed,true));const auto visitor=actor(target,delayed);
        until(target,[&]{const auto* t=trip(target,visitor);return t&&t->arrival_tick;},
            "Religion target-fire attempted arrival missing",500);
        check(target.building_on_fire(first)&&!trip(target,visitor)->visit_applied&&
            !orig::service_timer(house(target,first),faiths[index])&&target.original_production_state()->services.witnesses.empty(),
            "Burning target acquired actual religion");
    }
}
void confucian_far_body_edge_and_restore(const f::fs::path& real){
    Fixture f(real);auto w=f.world();
    for(const int y:{4,8})for(int x=18;x<24;++x)f::buy(w,{sim::CommandType::PlaceRoad,{x,y}});
    for(int y=5;y<8;++y)f::buy(w,{sim::CommandType::PlaceRoad,{23,y}});
    (void)buy(w,sim::CommandType::PlaceOriginalElitePlot,{20,0});(void)buy(w,sim::CommandType::PlaceHousehold,{21,9});
    (void)paper_kit(w,{{0,2},{3,2},{6,2},{9,2},{12,2}},true);
    const auto academy=buy(w,commands[2],{18,5});const auto id=actor(w,academy);
    until(w,[&]{return w.courier(id).phase==sim::CourierPhase::ToWarehouse&&w.courier(id).edge_progress>0;},"Paid3x3 Confucian source never dispatched");
    check(w.courier(id).path.front().x==20||w.courier(id).path.front().y==7,"Confucian test did not exercise actual source+2 body edge");
    auto replay=f.restore(w);for(int i=0;i<20000;++i){w.tick();replay.tick();authority(w);check(w.snapshot()==replay.snapshot(),"3x3 far-edge Confucian replay diverged");}
}
void river_is_real_immutable_policy_blocker(const f::fs::path& real){
    Fixture f(real);auto cells=f.policy->cells();
    for(int y=0;y<20;++y)cells[static_cast<std::size_t>(y*24+11)]={false,false,false,0,sim::BuildBlocker::UnsupportedBridgeWater,sim::BuildBlocker::UnsupportedTerrain,false,sim::OriginalRoadClassification::WaterOrBridge,true};
    f.policy=std::make_shared<const sim::MapPermissions>(24,20,3,std::move(cells),std::vector<sim::FixedGatePassage>{});
    auto w=f.world();for(int x=0;x<24;++x)if(x!=11)f::buy(w,{sim::CommandType::PlaceRoad,{x,4}});
    for(int x=0;x<9;x+=3)(void)buy(w,sim::CommandType::PlaceHousehold,{x,2});
    for(unsigned i=0;i<3;++i)(void)buy(w,commands[i],{12+4*static_cast<int>(i),5});
    oe_production_fixture::PaperSupplyLayout kit;kit.farm={12,17};kit.field={14,17};kit.paper={16,17};kit.warehouse={20,17};
    kit.extra_homes={{0,0},{3,0},{0,6},{3,6},{6,6}};
    for(int x=0;x<10;++x)kit.roads.push_back({x,8});for(int y=5;y<8;++y)kit.roads.push_back({9,y});
    for(int y=0;y<4;++y)kit.roads.push_back({2,y});for(int x=12;x<24;++x)kit.roads.push_back({x,16});
    for(int y=5;y<16;++y)kit.roads.push_back({23,y});(void)oe_production_fixture::add_paper_supply(w,kit);
    const auto before=w.snapshot();check(!w.execute({sim::CommandType::PlaceRoad,{11,4}}).accepted&&w.snapshot()==before,"Immutable authored normal water accepted a crossing road");
    ticks(w,4000);check(w.total_population()==56&&w.workforce_used()==50&&w.original_production_state()->services.trips.empty()&&w.original_production_state()->services.witnesses.empty(),"Religion crossed the actual policy water barrier or lacked real workforce");
    for(const auto& h:w.original_economy_state()->houses)check(!h.house.daoist&&!h.house.buddhist&&!h.house.confucian,"Across-river House acquired religion");
}
struct NeedScene{sim::BuildingId home{};};
NeedScene need_scene(sim::World& w,int mode){
    for(int x=0;x<24;++x)f::buy(w,{sim::CommandType::PlaceRoad,{x,4}});
    NeedScene s;for(int x=0;x<24;x+=3){const auto id=buy(w,sim::CommandType::PlaceHousehold,{x,2});if(!x)s.home=id;}
    const auto farm=buy(w,sim::CommandType::PlaceOriginalFoodFarm,{0,5});sim::Command crop{sim::CommandType::PlaceOriginalWheatField,{3,5}};crop.building_id=farm;f::buy(w,crop);
    (void)buy(w,sim::CommandType::PlaceMarket,{5,5});(void)buy(w,sim::CommandType::PlaceOriginalWaterWell,{9,5});(void)buy(w,sim::CommandType::PlaceOriginalAncestorShrine,{13,5});
    if(mode!=2)(void)buy(w,commands[mode==1?1:0],{16,5});
    if(mode!=3){(void)buy(w,commands[2],{19,5});(void)paper_kit(w,{{0,0},{3,0},{6,0},{9,0},{12,0}});
        for(const int x:{8,11})for(int y=0;y<4;++y)if(w.object_at({x,y})!=sim::Object::Road)f::buy(w,{sim::CommandType::PlaceRoad,{x,y}});}
    return s;
}
void authored_or_and_confucian_predicate_from_actual_arrivals(const f::fs::path& real){
    for(int mode=0;mode<5;++mode){Fixture f(real,mode);auto w=f.world();const auto s=need_scene(w,mode);
        until(w,[&]{const auto& h=house(w,s.home);return h.water&&h.food_quality>=20&&h.ancestor&&(mode==2 ? true:mode==1 ? h.buddhist>0:h.daoist>0);},"Authored OR/Common-Conf scene did not receive genuine other prerequisites",10000);
        if(mode<2){until(w,[&]{return house(w,s.home).type==orig::HouseType::PlainCottage;},"Actual single-faith OR did not permit authored Plain",10000);
            const auto& h=house(w,s.home);check((mode==0?h.daoist:h.buddhist)>0&&!(mode==0?h.buddhist:h.daoist),"Single-faith OR test silently received both categories");
        }else{
            if(mode==4) {
                until(w,[&]{for(const auto& row:w.original_production_state()->industry.auxiliary)
                    if(w.building(row.building_id).kind==sim::Object::OriginalConfucianAcademy219) return row.amounts[4]>0;
                    return false;},"Common-Conf negative lacked genuine Academy Paper");
                const auto unavailable=w.original_service_availability(s.home,orig::ServiceRole::Confucian);
                check(!unavailable.eligible_target&&!house(w,s.home).confucian,
                      "Fully paid/Paper-supplied Academy bypassed original Elite-only target restriction");
            }
            ticks(w,150);check(house(w,s.home).type==orig::HouseType::Hut,"Neither/noConfucian evolved through authored Need1");
            check(!orig::needs_met(f.model->house_rules(),orig::ModelHouse::PlainCottage,house(w,s.home)),"Actual missing faith met authored Plain predicate");
        }
    }
}
}
int main(int argc,char** argv){try{
    check(argc==1||(argc==3&&std::string_view(argv[1])=="--real"),"Usage: original-religion-world [--real DATA]");const auto real=argc==3?f::fs::canonical(argv[2]):f::fs::path{};
    paid_independent_visits_expiry_revisit_and_private_fairness(real);actual_religion_source_and_target_fire(real);confucian_far_body_edge_and_restore(real);river_is_real_immutable_policy_blocker(real);authored_or_and_confucian_predicate_from_actual_arrivals(real);
    std::cout<<"Paid215/217/219 visits, independent40 timers, private rotation, authored water barrier/OR needs and20k replays PASS; Confucian visits consume genuinely produced delivered Paper, no resource grant\n";return 0;
}catch(const std::exception& e){std::cerr<<"Original Religion World FAIL: "<<e.what()<<'\n';return 1;}}
