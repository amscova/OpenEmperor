#include "OriginalProductionFixture.h"
#include "simulation/original/GoodsRules.h"
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
template<class F> void refuses(F action) {
    bool denied=false;try {action();} catch(const std::exception&) {denied=true;}
    check(denied,"Invalid Food authority accepted");
}
struct Fixture {
    f::Root root;
    std::shared_ptr<const sim::MapPermissions> policy=f::permissions();
    std::shared_ptr<const orig::OriginalRulesModel> model;
    explicit Fixture(int food_need=0,int capacity=7) {oe_production_fixture::write_model(root.data,capacity,0,0,food_need);model=f::save::prepare_original_production_rules_model(root.data);}
    sim::World world() const {return sim::World::create_original_production_development(policy,model);}
};
sim::BuildingId buy(sim::World& world,sim::CommandType type,sim::Cell cell) {
    f::buy(world,{type,cell});return *world.building_owner_at(cell);
}
sim::Command field(sim::BuildingId farm,sim::Cell cell,bool millet=false) {
    sim::Command command{millet ? sim::CommandType::PlaceOriginalMilletField:sim::CommandType::PlaceOriginalWheatField,cell};
    command.building_id=farm;return command;
}
const sim::OriginalFoodStock& food(const sim::World& world,sim::BuildingId id) {
    for(const auto& stock:world.original_production_state()->food.stocks)if(stock.building_id==id)return stock;
    throw std::runtime_error("Missing paid Food stock");
}
int volume(const sim::OriginalFoodAmounts& amounts) {return amounts[0]+amounts[1];}
unsigned quality(const sim::World& world,sim::BuildingId id) {
    for(const auto& home:world.original_economy_state()->houses)if(home.building_id==id)return home.house.food_quality;
    throw std::runtime_error("Missing Food House quality");
}
void conserved(const sim::World& world) {
    check(world.production_balance_valid()&&world.city_economy_valid()&&world.population_valid()&&world.navigation_valid(),
          "Actual Food World invariant failed");
    const auto& state=world.original_production_state()->food;
    check(state.stocks.size()<=26&&state.trips.size()<=6,"Food ledger exceeded bounded owners/latest trips");
    sim::OriginalFoodCounters produced{},stock{},cargo{},consumed{};
    for(const auto& farm:state.farms)produced[static_cast<unsigned>(farm.crop)]+=farm.harvested;
    for(const auto& row:state.stocks) {
        const auto kind=world.building(row.building_id).kind;
        const int cap=kind==sim::Object::Market ? 16:8;
        check(row.amounts[0]>=0&&row.amounts[1]>=0&&volume(row.amounts)<=cap,"Food exceeded shared Crop capacity");
        std::uint64_t harvest=0;int crop=-1;
        for(const auto& farm:state.farms)if(farm.farm_id==row.building_id) {harvest=farm.harvested;crop=static_cast<int>(farm.crop);}
        for(std::size_t g=0;g<2;++g) {
            stock[g]+=static_cast<std::uint64_t>(row.amounts[g]);consumed[g]+=row.consumed[g];
            check((crop==static_cast<int>(g) ? harvest:0)+row.received[g]+row.returned[g]==
                  static_cast<std::uint64_t>(row.amounts[g])+row.departed[g]+row.consumed[g],"Food per-owner origin equation failed");
        }
        if(kind==sim::Object::Household) {
            check(volume(row.amounts)>0 || quality(world,row.building_id)==0,"Empty House retained stale Food quality");
            if(volume(row.amounts)>0)check(row.quality_witness.has_value(),"Positive House Food lacks actual-arrival witness");
        } else check(!row.quality_witness&&row.missed_demands==0,"Non-House owns Food quality/demand authority");
    }
    for(const auto& trip:state.trips)for(std::size_t g=0;g<2;++g) {
        const int remaining=trip.loaded[g]-trip.delivered[g]-trip.returned[g];
        check(remaining>=0,"Food trip creates negative cargo");cargo[g]+=static_cast<std::uint64_t>(remaining);
    }
    for(std::size_t g=0;g<2;++g)check(produced[g]==stock[g]+cargo[g]+consumed[g],"Global Crop volume is not conserved");
    for(const auto& building:world.buildings())
        check(!building.food_stock&&!building.food_produced&&!building.food_consumed_total&&!building.reserved_food_incoming&&!building.population,
              "Food authority mirrored into frozen legacy fields");
    for(const auto& courier:world.couriers())if(courier.role==sim::CourierRole::OriginalFoodToMarket||courier.role==sim::CourierRole::OriginalFoodToHouse)
        check(!courier.cargo&&!courier.reserved&&world.original_courier_cargo(courier.id)==world.original_food_cargo(courier.id),
              "Food cargo mirrored into legacy or six-Goods cargo");
}
void ticks(sim::World& world,int count) {for(int i=0;i<count;++i){world.tick();conserved(world);}}
template<class F> void until(sim::World& world,F done,int limit=20000) {
    for(int i=0;i<limit&&!done();++i){world.tick();conserved(world);}check(done(),"Paid Food event did not occur within finite observation");
}
struct Scene {sim::BuildingId wheat{},millet{},market{},wheat_field{},millet_field{};};
Scene scene(sim::World& world,bool fields=true,bool connected=true,bool houses=true) {
    for(int x=0;x<24;++x)f::buy(world,{sim::CommandType::PlaceRoad,{x,8}});
    if(houses)for(int x=0;x<18;x+=3)(void)buy(world,sim::CommandType::PlaceHousehold,{x,6});
    Scene s;const int y=connected ? 9:11;
    s.wheat=buy(world,sim::CommandType::PlaceOriginalFoodFarm,{0,y});
    s.millet=buy(world,sim::CommandType::PlaceOriginalFoodFarm,{5,y});
    if(fields) {
        f::buy(world,field(s.wheat,{3,y}));s.wheat_field=*world.building_owner_at({3,y});
        f::buy(world,field(s.millet,{8,y},true));s.millet_field=*world.building_owner_at({8,y});
    }
    s.market=buy(world,sim::CommandType::PlaceMarket,{12,9});return s;
}
void independent_quality_vectors() {
    check(sim::original_food_batch_quality({0,0}).value==0&&sim::original_food_batch_quality({4,0}).value==20&&
          sim::original_food_batch_quality({0,4}).value==80&&sim::original_food_batch_quality({3,1}).value==35&&
          sim::original_food_batch_quality({1,2}).value==60&&sim::original_food_batch_quality({1,4}).value==68,
          "Authored weighted Crop-score vectors failed");
    refuses([]{(void)sim::original_food_batch_quality({-1,1});});
    struct Vector {int delivered,previous,expected;};
    for(const auto v:{Vector{1,4,65},Vector{1,2,60},Vector{2,2,50},Vector{3,1,40},Vector{4,1,35}})
        check(orig::blend_food_quality(80,20,v.delivered,v.previous)==v.expected,"Independent arrival-volume original blend vector failed");
    check(orig::blend_food_quality(20,80,0,4)==20&&orig::blend_food_quality(20,80,1,4)==80,
          "Zero Food arrival improved quality or real superior batch failed");
}
void paid_two_crop_arrivals_and_replay() {
    Fixture f;auto world=f.world();const auto s=scene(world);
    check(world.original_production_state()->development_revision==18&&world.total_population()==0&&
          world.workforce_required(s.wheat)==12&&world.couriers().size()==4,
          "Food purchases did not retain exact model workforce/actual actor contract");
    check(sim::building_footprint(world.profile(),world.rule_version(),sim::Object::OriginalFoodFarm)==sim::BuildingFootprint{3,3}&&
          sim::building_footprint(world.profile(),world.rule_version(),sim::Object::OriginalWheatField)==sim::BuildingFootprint{1,1},
          "Food source physical identity mismatch");
    f::buy(world,sim::set_building_operation(s.millet,false));
    for(const auto& house:world.original_economy_state()->houses)check(quality(world,house.building_id)==0,"New House was granted Food quality");
    ticks(world,119);for(const auto& farm:world.original_production_state()->food.farms)check(!farm.harvested,"Food preceded genuine workers");
    until(world,[&]{for(const auto& h:world.original_economy_state()->houses)if(quality(world,h.building_id)==20)return true;return false;});
    const auto first=world.snapshot();
    for(const auto& row:first.original_production->food.stocks)if(row.quality_witness) {
        check(row.quality_witness->previous_quality==0&&row.quality_witness->accepted[0]>0&&!row.quality_witness->accepted[1],
              "First actual Wheat arrival has a fabricated previous quality or Crop");
    }
    f::buy(world,sim::set_building_operation(s.millet,true));
    until(world,[&]{for(const auto& h:world.original_economy_state()->houses)if(quality(world,h.building_id)>20)return true;return false;});
    bool multiple=false;for(const auto& row:world.original_production_state()->food.stocks)multiple|=row.quality_witness&&row.quality_witness->accepted[1]>0;
    check(multiple,"High Food class lacks a real Millet arrival");
    auto replay=sim::World::restore_original_production_development(world.snapshot(),f.policy,f.model);
    for(int i=0;i<20000;++i){world.tick();replay.tick();conserved(world);check(world.snapshot()==replay.snapshot(),"20k Food replay diverged");}
    std::uint64_t consumed=0;for(const auto& row:world.original_production_state()->food.stocks)consumed+=row.consumed[0]+row.consumed[1];
    check(consumed>0&&world.total_population()==42,"Real Food arrivals never produced a House sink or changed authored population");
    auto bad=world.snapshot();bad.original_production->development_revision=17;
    refuses([&]{(void)sim::World::restore_original_production_development(bad,f.policy,f.model);});
    bad=world.snapshot();++bad.original_economy->houses.front().house.food_quality;
    refuses([&]{(void)sim::World::restore_original_production_development(bad,f.policy,f.model);});
}
void no_origin_invalid_commands_and_missed_demand() {
    Fixture f;
    for(const bool houses:{false,true}) {
        auto world=f.world();(void)scene(world,!houses,true,houses);ticks(world,1200);
        for(const auto& farm:world.original_production_state()->food.farms)check(!farm.harvested,"Missing Field/workforce fabricated Food");
        for(const auto& row:world.original_production_state()->food.stocks)if(world.building(row.building_id).kind==sim::Object::Household)
            check(row.missed_demands>0&&!row.quality_witness&&!volume(row.amounts),"Food miss did not remain empty/unqualified");
    }
    auto world=f.world();const auto s=scene(world,false);const auto before=world.snapshot();
    for(const auto command:{field(s.wheat,{INT32_MIN,9}),field(s.wheat,{INT32_MAX,9}),field(s.wheat,{3,INT32_MIN}),
            field(s.wheat,{3,INT32_MAX}),field(s.market,{3,9}),field(s.wheat,{20,18}),field(s.wheat,{0,9})})
        check(!world.execute(command).accepted&&world.snapshot()==before,"Invalid Food Field command changed paid authority");
    check(!world.execute({sim::CommandType::PlaceOriginalFoodFarm,{10,12}}).accepted&&world.snapshot()==before,"Third Food Farm bypassed bound");
    f::buy(world,field(s.wheat,{3,9}));const auto planted=world.snapshot();
    check(!world.execute(field(s.wheat,{4,9},true)).accepted&&world.snapshot()==planted,"Second Field changed existing Farm/Crop binding");
    for(const auto id:{s.wheat,*world.building_owner_at({3,9})})
        check(!world.execute(sim::demolish_building(id)).accepted&&world.snapshot()==planted,"Food demolition discarded paid origin");
    auto old=sim::World::create_original_goods_development(f.policy,f::save::prepare_original_rules_model(f.root.data));
    const auto saved=old.snapshot();check(!old.execute({sim::CommandType::PlaceOriginalFoodFarm,{0,9}}).accepted&&old.snapshot()==saved,
          "Frozen Rule9 gained Food construction");
}
void full_no_road_begun_edge_cancel_and_empty_quality() {
    Fixture f;auto world=f.world();const auto s=scene(world,true,false);ticks(world,1500);
    for(const auto& farm:world.original_production_state()->food.farms)
        check(farm.harvested==8&&volume(food(world,farm.farm_id).amounts)==8&&world.building(farm.farm_id).progress==0,
              "Disconnected Food source exceeded shared capacity");
    check(!volume(food(world,s.market).amounts),"Food crossed absent road");
    f::buy(world,sim::set_building_operation(s.millet,false));
    f::buy(world,{sim::CommandType::PlaceRoad,{0,9}});f::buy(world,{sim::CommandType::PlaceRoad,{0,10}});
    sim::CourierId id=static_cast<sim::CourierId>(0);
    until(world,[&] {
        for(const auto& c:world.couriers())
            if(c.owner==s.wheat&&c.role==sim::CourierRole::OriginalFoodToMarket&&
               c.phase==sim::CourierPhase::ToWarehouse&&c.edge_progress>0&&world.original_food_cargo(c.id)>0) {
                id=c.id;return true;
            }
        return false;
    });
    const auto moving=world.courier(id);const auto cargo=world.original_food_cargo(id);
    f::buy(world,sim::set_building_operation(s.wheat,false));
    check(world.courier(id).phase==sim::CourierPhase::Returning&&world.courier(id).edge_progress==moving.edge_progress&&
          world.original_food_cargo(id)==cargo,"Food cancellation changed begun edge or physical cargo");
    auto replay=sim::World::restore_original_production_development(world.snapshot(),f.policy,f.model);
    for(int i=0;i<1500&&world.courier(id).phase!=sim::CourierPhase::IdleAtWorkshop;++i) {
        world.tick();replay.tick();conserved(world);check(world.snapshot()==replay.snapshot(),"Food return replay diverged");
    }
    check(!world.original_food_cargo(id)&&volume(food(world,s.wheat).amounts)==8&&!volume(food(world,s.market).amounts),
          "Full source lost its reserved Food return headroom");
    f::buy(world,sim::set_building_operation(s.wheat,true));
    until(world,[&]{for(const auto& h:world.original_economy_state()->houses)if(quality(world,h.building_id)>0)return true;return false;});
    f::buy(world,sim::set_building_operation(s.wheat,false));f::buy(world,sim::set_building_operation(s.market,false));
    ticks(world,10000);
    for(const auto& h:world.original_economy_state()->houses)
        check(!volume(food(world,h.building_id).amounts)&&quality(world,h.building_id)==0&&food(world,h.building_id).missed_demands>0,
              "Missed demand retained stale quality after actual Food depletion");
}
void actual_quality_drives_original_needs_without_evolution() {
    Fixture f(80);auto world=f.world();const auto s=scene(world);
    const auto& rules=f.model->house_rules();
    check(rules.value(orig::ModelHouse::Shelter,orig::HouseField::FoodQuality)==80,
          "Explicit authored quality-demand model was not prepared");
    for(const auto& h:world.original_economy_state()->houses)
        check(!orig::needs_met(rules,orig::ModelHouse::Shelter,h.house),"Empty House automatically met positive Food requirement");
    f::buy(world,sim::set_building_operation(s.millet,false));
    sim::BuildingId target=static_cast<sim::BuildingId>(0);
    until(world,[&]{for(const auto& h:world.original_economy_state()->houses)if(h.house.food_quality==20){target=h.building_id;return true;}return false;});
    const auto meets=[&] {
        for(const auto& h:world.original_economy_state()->houses)if(h.building_id==target) {
            check(h.house.type==orig::HouseType::Shelter,"Read-only Food predicate evolved a World House");
            return orig::needs_met(rules,orig::ModelHouse::Shelter,h.house);
        }
        throw std::runtime_error("Food predicate lost actual House");
    };
    check(!meets(),"Actual Wheat20 silently satisfied authored Food80 demand");
    f::buy(world,sim::set_building_operation(s.wheat,false));f::buy(world,sim::set_building_operation(s.millet,true));
    until(world,[&]{return quality(world,target)==80;},30000);
    check(meets()&&food(world,target).quality_witness&&food(world,target).quality_witness->accepted[1]>0,
          "Original needs predicate failed to read actual positive Millet80 arrival");
    f::buy(world,sim::set_building_operation(s.millet,false));f::buy(world,sim::set_building_operation(s.market,false));
    until(world,[&]{return volume(food(world,target).amounts)==0;},10000);
    check(quality(world,target)==0&&!meets(),"Consumed-empty House kept passing Food demand from stale quality");
}
void actual_mixed_market_full_house_and_partial_arrival() {
    // Explicit authored capacity37 supplies28 real workers from one paid House;
    // no positive stock, population or quality is installed through a snapshot.
    Fixture f(0,37);auto world=f.world();
    for(const int y:{2,8})for(int x=0;x<24;++x)f::buy(world,{sim::CommandType::PlaceRoad,{x,y}});
    for(int y=3;y<8;++y)f::buy(world,{sim::CommandType::PlaceRoad,{23,y}});
    const auto house=buy(world,sim::CommandType::PlaceHousehold,{0,0});
    const auto wheat=buy(world,sim::CommandType::PlaceOriginalFoodFarm,{0,9});f::buy(world,field(wheat,{3,9}));
    const auto millet=buy(world,sim::CommandType::PlaceOriginalFoodFarm,{5,9});f::buy(world,field(millet,{8,9},true));
    const auto market=buy(world,sim::CommandType::PlaceMarket,{12,9});
    f::buy(world,{sim::CommandType::RemoveRoad,{23,5}});
    until(world,[&]{return volume(food(world,market).amounts)==16;});
    check(food(world,market).amounts[0]>0&&food(world,market).amounts[1]>0&&!volume(food(world,house).amounts)&&quality(world,house)==0,
          "Disconnected House gained Food quality or full Market lost a genuine Crop class");
    f::buy(world,sim::set_building_operation(wheat,false));f::buy(world,sim::set_building_operation(millet,false));
    f::buy(world,{sim::CommandType::PlaceRoad,{23,5}});
    until(world,[&] {
        for(const auto& trip:world.original_production_state()->food.trips)
            if(trip.source==market&&trip.loaded[0]>0&&trip.loaded[1]>0)return true;
        return false;
    });
    bool partial=false,full=false,mixed=false;std::uint64_t previous_sequence=0;
    for(int i=0;i<15000&&!(partial&&full&&mixed);++i) {
        world.tick();conserved(world);const auto& row=food(world,house);
        full|=volume(row.amounts)==8;
        if(row.quality_witness&&row.quality_witness->transfer.sequence!=previous_sequence) {
            previous_sequence=row.quality_witness->transfer.sequence;
            partial|=volume(row.quality_witness->accepted)<4;
            mixed|=row.quality_witness->accepted[0]>0&&row.quality_witness->accepted[1]>0;
        }
    }
    check(partial&&full&&mixed,"Actual mixed/full-target/partial Food arrivals were not observed");
}
}
int main() {try {independent_quality_vectors();paid_two_crop_arrivals_and_replay();no_origin_invalid_commands_and_missed_demand();
    full_no_road_begun_edge_cancel_and_empty_quality();actual_quality_drives_original_needs_without_evolution();
    actual_mixed_market_full_house_and_partial_arrival();
    std::cout<<"Rule10 revision18 paid two-Crop Food, actual quality arrivals, sink/conservation and20k replay PASS\n";return 0;
}catch(const std::exception& error){std::cerr<<"Original Food World FAIL: "<<error.what()<<'\n';return 1;}}
