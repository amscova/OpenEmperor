#include "OriginalProductionFixture.h"
#include "simulation/original/OriginalRulesModel.h"
#include "simulation/original/OriginalScheduler.h"

#include <algorithm>
#include <iostream>
#include <limits>

namespace {
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
using f::check;

struct Fixture {
    f::Root root;int width=24,height=20;
    std::shared_ptr<const sim::MapPermissions> policy;
    std::shared_ptr<const orig::OriginalRulesModel> model;
    explicit Fixture(int hemp=1,int ceramics=1,bool large=false) {
        if (large) width=36;
        policy=std::make_shared<const sim::MapPermissions>(width,height,3,
            std::vector<sim::MapCellPermission>(static_cast<std::size_t>(width*height),
                {true,true,false,0,sim::BuildBlocker::None,sim::BuildBlocker::None}),std::vector<sim::FixedGatePassage>{});
        oe_production_fixture::write_model(root.data,7,hemp,ceramics);
        model=f::save::prepare_original_production_rules_model(root.data);
    }
    sim::World world() const {return sim::World::create_original_production_development(policy,model);}
};
sim::BuildingId buy(sim::World& w,sim::CommandType type,sim::Cell cell) {
    f::buy(w,{type,cell});return *w.building_owner_at(cell);
}
void add_farm(sim::World& w,sim::Cell cell,std::vector<sim::BuildingId>& farms) {
    const auto farm=buy(w,sim::CommandType::PlaceOriginalHempFarm,cell);farms.push_back(farm);
    sim::Command command{sim::CommandType::PlaceOriginalHempField,{cell.x+2,cell.y}};
    command.building_id=farm;f::buy(w,command);
}
struct Scene {std::vector<sim::BuildingId> farms,markets;};
Scene scene(sim::World& w,bool large=false,bool second_market=false) {
    Scene result;
    for (int x=0;x<w.width();++x) f::buy(w,{sim::CommandType::PlaceRoad,{x,8}});
    if (large) {
        for (int x=0;x<w.width();++x) f::buy(w,{sim::CommandType::PlaceRoad,{x,15}});
        for (int y=9;y<15;++y) f::buy(w,{sim::CommandType::PlaceRoad,{14,y}});
    }
    for (const int y:large ? std::vector<int>{6,16}:std::vector<int>{6})
        for (int x=0;x<(large ? 30:18);x+=3) (void)buy(w,sim::CommandType::PlaceHousehold,{x,y});
    (void)buy(w,sim::CommandType::PlaceClaySource,{0,9});
    add_farm(w,{4,9},result.farms);
    (void)buy(w,sim::CommandType::PlacePottery,{8,9});
    (void)buy(w,sim::CommandType::PlaceWarehouse,{16,9});
    result.markets.push_back(buy(w,sim::CommandType::PlaceMarket,{21,9}));
    if (large) {
        add_farm(w,{11,9},result.farms);
        (void)buy(w,sim::CommandType::PlaceClaySource,{30,9});
        (void)buy(w,sim::CommandType::PlacePottery,{27,9});
        (void)buy(w,sim::CommandType::PlaceWarehouse,{18,9});
        result.markets.push_back(buy(w,sim::CommandType::PlaceMarket,{24,9}));
    } else if (second_market) result.markets.push_back(buy(w,sim::CommandType::PlaceMarket,{22,9}));
    return result;
}
const sim::OriginalHouseConsumptionState& history(const sim::World& w,sim::BuildingId owner) {
    for (const auto& row:w.original_production_state()->house_consumption) if (row.building_id==owner) return row;
    throw std::runtime_error("Missing actual House consumption history");
}
int stock(const sim::World& w,sim::BuildingId owner,orig::Goods good) {
    if (w.building(owner).kind==sim::Object::Household) {
        for (const auto& row:w.original_economy_state()->houses) if (row.building_id==owner) return orig::goods_stock(row.house,good);
    } else for (const auto& row:w.original_goods_state()->stocks) if (row.building_id==owner) return row.amounts[static_cast<std::size_t>(good)];
    throw std::runtime_error("Missing actual consumption-scene stock");
}
bool due(std::uint64_t old_tick) {return orig::next_scheduler_dispatch(old_tick)->consumption;}
std::uint64_t next_boundary(std::uint64_t tick,int step=-1) {
    do {++tick;} while (!due(tick-1) || (step>=0 && orig::scheduler_position(tick-1).step!=step));
    return tick;
}
void conserved(const sim::World& w) {
    check(w.production_balance_valid() && w.navigation_valid() && w.population_valid() && w.city_economy_valid(),
          "Consumption World invariant failed");
    std::array<std::uint64_t,orig::goods_count> sums{};
    check(w.original_production_state()->house_consumption.size()==w.original_economy_state()->houses.size(),
          "Purchased House lost its sole consumption-history row");
    for (const auto& row:w.original_production_state()->house_consumption)
        for (std::size_t g=0;g<orig::goods_count;++g) sums[g]+=row.consumed[g];
    const auto balances=w.original_goods_balances();
    for (std::size_t g=0;g<orig::goods_count;++g) {
        const auto& balance=balances[g];
        check(balance.conserved && balance.totals.consumed==sums[g] && !balance.totals.imported && !balance.totals.demolished,
              "House consumption and actual ledger sink disagree");
        check(balance.supported,"Active consumption omitted a supported commodity");
        if(g>=2)check(!balance.totals.produced&&!balance.stock&&!balance.cargo&&!balance.totals.consumed,"Hemp/Ceramics consumption scene fabricated luxury goods");
    }
    for (const auto& b:w.buildings()) check(!b.population && !b.pottery_stock && !b.food_stock && !b.consumed_total && !b.food_consumed_total,
        "Original consumption was mirrored into legacy population, stock or400t-demand authority");
}
void tick(sim::World& w) {
    const auto before=w.original_production_state()->house_consumption;const bool consume=due(w.ticks());
    w.tick();conserved(w);
    for (std::size_t i=0;i<before.size();++i) for (std::size_t g=0;g<orig::goods_count;++g) {
        const auto now=w.original_production_state()->house_consumption[i].consumed[g];
        check(now>=before[i].consumed[g] && (consume || now==before[i].consumed[g]),
              "House consumed outside the exact versioned step0/7 phase or history went backwards");
    }
}
void until_tick(sim::World& w,std::uint64_t target) {while (w.ticks()<target) tick(w);}
template<class P> void until(sim::World& w,P condition,int limit=20000) {
    for (int i=0;i<limit && !condition();++i) tick(w);
    check(condition(),"Paid consumption scene did not reach the required real state");
}
void pause_markets(sim::World& w,const Scene& ids) {
    for (const auto owner:ids.markets) f::buy(w,sim::set_building_operation(owner,false));
}
void isolated_boundary(sim::World& w,Fixture& f,std::uint64_t at,sim::BuildingId positive) {
    until_tick(w,at-1);
    auto replay=sim::World::restore_original_production_development(w.snapshot(),f.policy,f.model);
    const auto before=w.original_economy_state()->houses;
    const auto counters=w.original_production_state()->house_consumption;
    std::vector<orig::HouseState> evolved;
    for (const auto& home:before) {
        auto input=home.house;
        if (input.occupants>0) {
            input.desirability=static_cast<std::int8_t>(w.original_influence_provenance(home.building_id).value);
            input=orig::evolve_step(f.model->house_rules(),input);
            if (input.type>orig::HouseType::PlainCottage) input.type=orig::HouseType::PlainCottage;
        }
        evolved.push_back(input);
    }
    check(stock(w,positive,orig::Goods::Hemp)>0 && stock(w,positive,orig::Goods::Ceramics)>0,
          "Positive boundary control has no physically delivered goods");
    tick(w);tick(replay);check(w.snapshot()==replay.snapshot(),"F5/F9 immediately before consumption changed next-cycle authority");
    for (std::size_t i=0;i<before.size();++i) {
        const auto expected=orig::consume_goods(f.model->house_rules(),evolved[i]);
        check(w.original_economy_state()->houses[i].house==expected.house,"Boundary did not apply pure phase39 evolution followed by consumption exactly once");
        for (std::size_t g=0;g<orig::goods_count;++g)
            check(history(w,before[i].building_id).consumed[g]-counters[i].consumed[g]==static_cast<std::uint64_t>(expected.consumed[g]),
                  "Boundary sink differs from physically consumed units");
    }
    const auto at_boundary=w.original_production_state()->house_consumption;
    tick(w);tick(replay);check(w.snapshot()==replay.snapshot() && w.original_production_state()->house_consumption==at_boundary,
          "The tick after a consumption boundary repeated its sink");
}
void exact_boundaries_and_replay() {
    Fixture f;auto w=f.world();const auto ids=scene(w);
    until_tick(w,39);check(w.total_population()==0,"Boundary40 empty-House control unexpectedly has residents");
    tick(w);check(w.original_goods_balance(orig::Goods::Hemp).totals.consumed==0 && w.original_goods_balance(orig::Goods::Ceramics).totals.consumed==0,
                  "Exactly empty Houses consumed before any immigration or delivery");
    tick(w);until_tick(w,396);tick(w);tick(w);
    sim::BuildingId positive{};
    until(w,[&] {
        for (const auto& h:w.original_economy_state()->houses) if (h.house.hemp>=2 && h.house.ceramics>=2) {positive=h.building_id;return true;}
        return false;
    });
    pause_markets(w,ids);
    const auto first=next_boundary(w.ticks());
    const auto first_step=orig::scheduler_position(first-1).step;
    isolated_boundary(w,f,first,positive);
    const auto second=next_boundary(w.ticks());
    check(orig::scheduler_position(second-1).step!=first_step,"Consecutive positive controls did not span both step0 and7");
    isolated_boundary(w,f,second,positive);
    auto replay=sim::World::restore_original_production_development(w.snapshot(),f.policy,f.model);
    for (int i=0;i<20000;++i) {tick(w);tick(replay);check(w.snapshot()==replay.snapshot(),"20k consumption/empty-buffer replay diverged");}
    for (const auto& h:w.original_economy_state()->houses) check(!h.house.hemp && !h.house.ceramics,"Paused delivery did not reach min0 without negative stock");
}
void same_tick_arrival_and_minimum() {
    Fixture f(100,100);auto w=f.world();const auto ids=scene(w);
    sim::CourierId actor{};
    until(w,[&] {
        for (const auto& c:w.couriers()) if (c.role==sim::CourierRole::MarketPotteryDistribution && c.phase==sim::CourierPhase::IdleAtWorkshop &&
            (stock(w,c.owner,orig::Goods::Hemp)>0 || stock(w,c.owner,orig::Goods::Ceramics)>0)) {actor=c.id;return true;}
        return false;
    });
    pause_markets(w,ids);f::buy(w,sim::set_building_operation(ids.markets[0],true));
    const auto ready=w.courier_dispatch_status(actor);check(ready.status==sim::CourierDispatchStatus::Ready && ready.selected_target,"Preloaded real Market has no reachable House");
    const auto target=*ready.selected_target;const auto& c=w.courier(actor);
    const auto route=std::find_if(c.dynamic_target_routes.begin(),c.dynamic_target_routes.end(),[&](const auto& entry){return entry.first==target;});
    check(route!=c.dynamic_target_routes.end() && route->second,"Missing shared real House route");
    const auto travel=static_cast<std::uint64_t>(route->second->size()-1)*static_cast<std::uint64_t>(w.active_rules().courier_edge_ticks);
    pause_markets(w,ids);const auto at=next_boundary(w.ticks()+travel-1);
    until_tick(w,at-travel);f::buy(w,sim::set_building_operation(ids.markets[0],true));
    until_tick(w,at-1);
    const auto& arriving=w.courier(actor);
    check(arriving.phase==sim::CourierPhase::ToWarehouse && arriving.target==target && arriving.path_vertex+2==arriving.path.size() &&
          arriving.edge_progress+1==w.active_rules().courier_edge_ticks,"Real paid dispatch did not align its actual final edge with the consumption tick");
    const auto good=*w.original_courier_good(actor);const auto g=static_cast<std::size_t>(good);
    const auto amount=w.original_courier_cargo(actor);const auto previous=history(w,target).consumed[g];
    check(amount>0 && amount<=sim::Rules::courier_capacity && stock(w,target,good)==0,"Same-tick under-supply control already had stock");
    auto replay=sim::World::restore_original_production_development(w.snapshot(),f.policy,f.model);
    tick(w);tick(replay);
    check(w.snapshot()==replay.snapshot() && !stock(w,target,good) && history(w,target).consumed[g]-previous==static_cast<std::uint64_t>(amount),
          "Actual arrival was not consumed on its same tick, or need100 invented missing stock");
    pause_markets(w,ids);const auto no_more=w.original_production_state()->house_consumption;
    until_tick(w,next_boundary(w.ticks())+1);
    check(w.original_production_state()->house_consumption==no_more,"Empty supplied House consumed phantom goods on its next boundary");
}
std::uint64_t verify_pairing(const sim::World& w) {
    std::uint64_t checked=0;
    for (const auto& c:w.couriers()) if (c.role==sim::CourierRole::MarketPotteryDistribution && c.phase==sim::CourierPhase::IdleAtWorkshop) {
        const auto found=std::find_if(w.original_goods_state()->reservations.begin(),w.original_goods_state()->reservations.end(),
            [&](const auto& r){return r.transfer.courier_id==c.id;});
        if (found==w.original_goods_state()->reservations.end() || found->good!=orig::Goods::Hemp || stock(w,c.owner,orig::Goods::Ceramics)<=0 ||
            w.building_on_fire(found->target)) continue;
        int incoming=0;
        for (const auto& r:w.original_goods_state()->reservations) if (r.target==found->target && r.good==orig::Goods::Ceramics && r.phase==sim::OriginalGoodsTransferPhase::Loaded)
            incoming+=r.loaded-r.delivered-r.returned;
        if (stock(w,found->target,orig::Goods::Ceramics)+incoming>=sim::Rules::household_capacity) continue;
        const auto decision=w.courier_dispatch_status(c.id);
        if (decision.status==sim::CourierDispatchStatus::Ready) {
            check(decision.selected_target==found->target,"Hemp→Ceramics advanced the House cursor and recreated even-target goods phase-lock");++checked;
        }
    }
    return checked;
}
void ongoing_rotation(bool large) {
    Fixture f(1,1,large);auto w=f.world();const auto ids=scene(w,large);
    for (const auto farm:ids.farms) f::buy(w,sim::set_building_operation(farm,false));
    until_tick(w,4000);
    check(w.original_goods_balance(orig::Goods::Hemp).totals.produced==0 && w.original_goods_balance(orig::Goods::Ceramics).totals.consumed>0,
          "Temporary single-good availability blocked real Ceramics consumption or granted Hemp");
    std::size_t ceramics_houses=0;
    for (const auto& h:w.original_production_state()->house_consumption) if (h.consumed[1]) ++ceramics_houses;
    check(ceramics_houses>1,"Repeated same-good trips did not advance the real House round-robin");
    for (const auto farm:ids.farms) f::buy(w,sim::set_building_operation(farm,true));
    auto replay=sim::World::restore_original_production_development(w.snapshot(),f.policy,f.model);
    std::uint64_t paired=0;
    for (int i=0;i<20000;++i) {
        paired+=verify_pairing(w);tick(w);tick(replay);
        check(w.snapshot()==replay.snapshot(),"Ongoing two-good consumption replay diverged");
    }
    check(paired>0,"Ongoing actual supply never exercised the corrected Hemp→Ceramics House pairing");
    for (const auto& h:w.original_production_state()->house_consumption)
        check(h.consumed[0]>0 && h.consumed[1]>0,"Finite ongoing rotation left a House permanently assigned only one good");
    check(w.original_production_state()->house_consumption.size()==(large ? 20U:6U) && ids.markets.size()==(large ? 2U:1U),
          "Ongoing finite scenario did not exercise its declared House/Market count");
    std::cout<<(large ? "20 Houses/two Markets":"six Houses/one Market")<<": "<<paired<<" actual paired dispatch observations; H/C consumed "
             <<w.original_goods_balance(orig::Goods::Hemp).totals.consumed<<'/'<<w.original_goods_balance(orig::Goods::Ceramics).totals.consumed<<'\n';
}
void zero_and_negative_need() {
    Fixture f(0,-1);auto w=f.world();const auto ids=scene(w);(void)ids;
    until(w,[&] {return std::any_of(w.original_economy_state()->houses.begin(),w.original_economy_state()->houses.end(),
        [](const auto& h){return h.house.hemp>0 && h.house.ceramics>0;});});
    until_tick(w,next_boundary(w.ticks())+1);
    check(!w.original_goods_balance(orig::Goods::Hemp).totals.consumed && !w.original_goods_balance(orig::Goods::Ceramics).totals.consumed,
          "Zero/negative bound model need consumed physically delivered goods");
}
}
int main() {
    try {exact_boundaries_and_replay();same_tick_arrival_and_minimum();ongoing_rotation(false);ongoing_rotation(true);zero_and_negative_need();
        std::cout<<"Original consumption World PASS: steps0/7±1, empty/min0/same-tick real arrival, paid ongoing two-good rotation and20k replay\n";return 0;
    } catch (const std::exception& error) {std::cerr<<"Original consumption World FAIL: "<<error.what()<<'\n';return 1;}
}
