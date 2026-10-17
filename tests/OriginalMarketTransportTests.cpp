#include "OriginalInfluenceFixture.h"
#include "OriginalProductionFixture.h"
#include "simulation/OriginalGoodsLedger.h"
#include "simulation/original/OriginalRulesModel.h"

#include <algorithm>
#include <iostream>

namespace {
namespace f=oe_influence_fixture;namespace sim=openemperor::simulation;namespace orig=openemperor::original;
using f::check;
template<class F> void refuses(F action) {bool denied=false;try {action();} catch(const std::exception&){denied=true;}check(denied,"Invalid typed trip accepted");}
struct Fixture {
    f::Root root;int width=24,height=20;
    std::shared_ptr<const sim::MapPermissions> policy;
    std::shared_ptr<const orig::OriginalRulesModel> model;
    explicit Fixture(bool large=false) {
        if (large) {width=48;height=24;}
        policy=std::make_shared<const sim::MapPermissions>(width,height,3,
            std::vector<sim::MapCellPermission>(static_cast<std::size_t>(width*height),
                {true,true,false,0,sim::BuildBlocker::None,sim::BuildBlocker::None}),std::vector<sim::FixedGatePassage>{});
        oe_production_fixture::write_model(root.data,6);model=f::save::prepare_original_production_rules_model(root.data);
    }
    sim::World world() const {return sim::World::create_original_production_development(policy,model);}
};
bool typed(sim::CourierRole role){return role==sim::CourierRole::MarketPotteryInbound || role==sim::CourierRole::MarketPotteryDistribution;}
int stock(const sim::World& world,sim::BuildingId id) {
    const auto& b=world.building(id);
    if (b.kind==sim::Object::Household) for(const auto& h:world.original_economy_state()->houses) if(h.building_id==id)return h.house.ceramics;
    for(const auto& row:world.original_goods_state()->stocks)if(row.building_id==id)return row.amounts[1];
    throw std::runtime_error("Missing typed stock");
}
const sim::OriginalGoodsReservation& trip(const sim::World& world,sim::CourierId id) {
    for(const auto& r:world.original_goods_state()->reservations)if(r.transfer.courier_id==id)return r;
    throw std::runtime_error("Missing actual trip");
}
void conserved(const sim::World& world) {
    check(world.production_balance_valid() && world.population_valid() && world.navigation_valid() && world.city_economy_valid(),"Typed World invariants failed");
    for(const auto& b:world.original_goods_balances())check(b.conserved,"Typed conservation failed");
    for(const auto& b:world.buildings())check(b.pottery_stock==0 && b.population==0,"Legacy mass/population mirror appeared");
    check(world.original_goods_state()->reservations.size()<=6 && world.original_goods_state()->cargo.size()<=6,"Latest-row bound grew with trips");
    for(const auto& c:world.couriers())if(typed(c.role))check(c.cargo==0 && c.reserved==0,"Typed Courier cargo was duplicated");
}
void ticks(sim::World& world,int count){for(int i=0;i<count;++i){world.tick();conserved(world);}}
sim::BuildingId buy(sim::World& world,sim::CommandType type,sim::Cell cell){f::buy(world,{type,cell});return *world.building_owner_at(cell);}
struct Scene {sim::BuildingId clay{},pottery{},warehouse{},market{};};
Scene small(sim::World& world,bool connected=true) {
    for(int x=0;x<24;++x)f::buy(world,{sim::CommandType::PlaceRoad,{x,8}});
    for(int x=0;x<12;x+=3)(void)buy(world,sim::CommandType::PlaceHousehold,{x,6});
    return {buy(world,sim::CommandType::PlaceClaySource,{0,9}),buy(world,sim::CommandType::PlacePottery,{8,9}),
        buy(world,sim::CommandType::PlaceWarehouse,{16,9}),buy(world,sim::CommandType::PlaceMarket,connected?sim::Cell{20,9}:sim::Cell{20,10})};
}
sim::CourierId wait_loaded(sim::World& world,sim::CourierRole role,int limit=5000) {
    for(int i=0;i<limit;++i){
        for(const auto& c:world.couriers())if(c.role==role && c.phase==sim::CourierPhase::ToWarehouse &&
            c.edge_progress>0 && world.original_courier_cargo(c.id)>0)return c.id;
        world.tick();conserved(world);
    }
    throw std::runtime_error("Actual typed Courier never loaded");
}
void pause_loaded(sim::World& world,sim::CourierId id,Fixture& fixture) {
    const auto before=world.courier(id);const auto previous=trip(world,id);
    const auto amount=world.original_courier_cargo(id);check(amount>0 && amount<=sim::Rules::courier_capacity,"Invalid actual partial load");
    f::buy(world,sim::set_building_operation(before.owner,false));
    const auto& changed=world.courier(id);
    check(changed.phase==sim::CourierPhase::Returning && changed.route_pending && changed.edge_progress==before.edge_progress &&
        changed.path.front()==before.path[before.path_vertex] && changed.path.back()==before.path[before.path_vertex+1] &&
        world.original_courier_cargo(id)==amount && trip(world,id).phase==sim::OriginalGoodsTransferPhase::Returning &&
        trip(world,id).transfer==previous.transfer,"Pause cancelled cargo or moved the begun edge");
    auto replay=sim::World::restore_original_production_development(world.snapshot(),fixture.policy,fixture.model);
    for(int i=0;i<1000 && world.courier(id).phase!=sim::CourierPhase::IdleAtWorkshop;++i){
        world.tick();replay.tick();conserved(world);check(world.snapshot()==replay.snapshot(),"Cancelled mid-edge save/replay diverged");
    }
    check(world.courier(id).phase==sim::CourierPhase::IdleAtWorkshop && !world.original_courier_cargo(id) &&
        trip(world,id).phase==sim::OriginalGoodsTransferPhase::Returned && trip(world,id).returned==amount &&
        trip(world,id).delivered==0,"Cancelled typed cargo did not physically return exactly once");
}
void paid_and_cancelled() {
    Fixture fixture;auto world=fixture.world();const auto ids=small(world);
    check(world.construction_spent_total()==958 && world.treasury()==11042 && world.couriers().size()==4 &&
        world.couriers().back().role==sim::CourierRole::MarketPotteryDistribution,"Market price/real owned actor mismatch");
    const auto inbound=wait_loaded(world,sim::CourierRole::MarketPotteryInbound);
    check(world.original_courier_cargo(inbound)<sim::Rules::courier_capacity,"Fixture did not prove actual partial loading");
    pause_loaded(world,inbound,fixture);f::buy(world,sim::set_building_operation(ids.warehouse,true));
    const auto distributor=wait_loaded(world,sim::CourierRole::MarketPotteryDistribution);
    pause_loaded(world,distributor,fixture);f::buy(world,sim::set_building_operation(ids.market,true));
    const auto loaded=wait_loaded(world,sim::CourierRole::MarketPotteryDistribution);
    auto replay=sim::World::restore_original_production_development(world.snapshot(),fixture.policy,fixture.model);
    const auto target=world.courier(loaded).target;const auto prior=stock(world,target);
    ticks(world,1200);for(int i=0;i<1200;++i)replay.tick();check(replay.snapshot()==world.snapshot(),"Real delivery/return replay diverged");
    check(stock(world,target)>prior,"Actual Market arrival did not reach sole HouseState stock");
    for(const auto& home:world.original_economy_state()->houses)
        check(orig::goods_demand(fixture.model->house_rules(),home.house,2,orig::Goods::Ceramics).need==0,
            "Authored provision invented an original Ceramics requirement");
    const auto before=world.snapshot();
    for(const auto id:{ids.market,ids.warehouse})check(!world.execute(sim::demolish_building(id)).accepted && world.snapshot()==before,
        "Typed source/actor demolition bypassed strict history contract");
}
void disconnected_and_repaired() {
    Fixture fixture;auto world=fixture.world();const auto ids=small(world,false);ticks(world,1500);
    check(stock(world,ids.warehouse)>0 && stock(world,ids.market)==0,"No-road Market received teleport goods");
    for(const auto& h:world.original_economy_state()->houses)check(h.house.ceramics==0,"No-road House received teleport goods");
    f::buy(world,{sim::CommandType::PlaceRoad,{20,9}});
    const auto id=wait_loaded(world,sim::CourierRole::MarketPotteryDistribution);
    const auto saved=world.snapshot();
    // Remove an unoccupied future road, preserving the current begun edge.
    bool removed=false;
    for(const auto cell:world.courier(id).path)if(world.object_at(cell)==sim::Object::Road &&
        world.validate({sim::CommandType::RemoveRoad,cell}).accepted){f::buy(world,{sim::CommandType::RemoveRoad,cell});removed=true;break;}
    check(removed,"No legal future road interruption available");
    auto replay=sim::World::restore_original_production_development(world.snapshot(),fixture.policy,fixture.model);
    for(int i=0;i<100;++i){world.tick();replay.tick();check(world.snapshot()==replay.snapshot(),"Disconnected loaded replay diverged");}
    check(world.original_goods_balance(orig::Goods::Ceramics).conserved,"Interrupted actual cargo was lost");
    for(const auto cell:saved.roads)if(world.object_at(cell)==sim::Object::Empty)f::buy(world,{sim::CommandType::PlaceRoad,cell});
    ticks(world,1200);check(std::any_of(world.original_economy_state()->houses.begin(),world.original_economy_state()->houses.end(),
        [](const auto& h){return h.house.ceramics>0;}),"Paid repair did not resume actual House delivery");
}
void longplay_and_stale() {
    Fixture fixture(true);auto world=fixture.world();
    for(int x=0;x<48;++x)for(const int y:{8,15})f::buy(world,{sim::CommandType::PlaceRoad,{x,y}});
    for(int y=9;y<15;++y)f::buy(world,{sim::CommandType::PlaceRoad,{47,y}});
    for(const int y:{6,16})for(int x=0;x<30;x+=3)(void)buy(world,sim::CommandType::PlaceHousehold,{x,y});
    for(const int x:{0,3})(void)buy(world,sim::CommandType::PlaceClaySource,{x,9});
    for(const int x:{8,11})(void)buy(world,sim::CommandType::PlacePottery,{x,9});
    for(const int x:{16,19})(void)buy(world,sim::CommandType::PlaceWarehouse,{x,9});
    for(const int x:{25,28,31,34})(void)buy(world,sim::CommandType::PlaceMarket,{x,9});
    const auto first=wait_loaded(world,sim::CourierRole::MarketPotteryInbound);const auto stale=trip(world,first).transfer;
    auto replay=sim::World::restore_original_production_development(world.snapshot(),fixture.policy,fixture.model);
    for(int i=0;i<20000;++i){world.tick();replay.tick();conserved(world);check(world.snapshot()==replay.snapshot(),"20k real typed transport replay diverged");}
    std::uint64_t highest=0;for(const auto& r:world.original_goods_state()->reservations)highest=std::max(highest,r.transfer.sequence);
    check(highest>26 && world.original_goods_state()->reservations.size()<=6,"Real repeated trips did not exercise bounded history compaction");
    for(const auto& h:world.original_economy_state()->houses)check(h.house.ceramics==sim::Rules::household_capacity,"Finite20-House rotation left an authored buffer unserved");
    std::vector<sim::OriginalGoodsOwner> owners;std::vector<sim::CourierId> couriers;
    for(const auto& b:world.buildings())if(b.kind==sim::Object::Household || b.kind==sim::Object::Warehouse || b.kind==sim::Object::Market){
        sim::OriginalGoodsOwner owner;owner.building_id=b.id;owner.household=b.kind==sim::Object::Household;
        owner.capacity[1]=owner.household?sim::Rules::household_capacity:b.kind==sim::Object::Market?sim::Rules::market_pottery_capacity:sim::Rules::warehouse_capacity;
        owner.capacity[0]=owner.capacity[1];owners.push_back(owner);
    }
    for(const auto& c:world.couriers())if(typed(c.role))couriers.push_back(c.id);
    const sim::OriginalGoodsLedgerView view{*world.original_goods_state(),world.original_economy_state()->houses,owners,couriers};
    const auto saved=world.snapshot();
    // Authored diagnostic-history variation only, not a claimed additional
    // gameplay trip: full typed authority must contribute to its fingerprint.
    auto other_history=saved;
    const auto terminal=std::find_if(other_history.original_goods->reservations.begin(),other_history.original_goods->reservations.end(),
        [](const auto& r){return r.phase==sim::OriginalGoodsTransferPhase::Completed || r.phase==sim::OriginalGoodsTransferPhase::Returned;});
    check(terminal!=other_history.original_goods->reservations.end(),"Missing completed real-trip diagnostic control");
    ++terminal->transfer.sequence;
    auto other=sim::World::restore_original_production_development(other_history,fixture.policy,fixture.model);
    check(other.snapshot()!=saved && other.canonical_state()!=world.canonical_state(),
        "Distinct typed transfer histories collided in canonical diagnostic state");
    refuses([&]{(void)sim::original_goods_deliver(view,stale,1);});
    for(const auto& r:world.original_goods_state()->reservations)if(r.phase==sim::OriginalGoodsTransferPhase::Completed)
        refuses([&]{(void)sim::original_goods_deliver(view,r.transfer,1);});
    check(world.snapshot()==saved,"Stale/double pure arrival changed actual World");
    auto bad=saved;++bad.original_goods->reservations.front().transfer.sequence;
    if(!bad.original_goods->cargo.empty())++bad.original_goods->cargo.front().transfer.sequence;
    // An independent reservation phase mismatch cannot become a legal actor.
    bad.original_goods->reservations.front().phase=sim::OriginalGoodsTransferPhase::Loaded;
    refuses([&]{(void)sim::World::restore_original_production_development(bad,fixture.policy,fixture.model);});
}
}
int main(){try{paid_and_cancelled();disconnected_and_repaired();longplay_and_stale();
    std::cout<<"Rule10 revision15 paid typed Market/House arrival, partial loads/returns, interruptions, bounded stale-safe20k replay PASS\n";return 0;
}catch(const std::exception& e){std::cerr<<"Original Market transport FAIL: "<<e.what()<<'\n';return 1;}}
