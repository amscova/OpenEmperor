#include "../tests/OriginalInfluenceFixture.h"
#include "simulation/original/GoodsRules.h"
#include "simulation/original/OriginalScheduler.h"
#include "simulation/original/ServiceRules.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <nlohmann/json.hpp>
#include <utility>

namespace {
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace save=openemperor::persistence;
namespace original=openemperor::original;
using json=nlohmann::json;
using f::check;

bool below(const f::fs::path& path,const f::fs::path& root) {
    auto a=path.begin();
    for (auto b=root.begin();b!=root.end();++a,++b)
        if (a==path.end() || *a!=*b) return false;
    return true;
}
void validate_output(const f::fs::path& output,const f::fs::path& data) {
    const auto repo=f::fs::canonical(f::fs::path(__FILE__).parent_path().parent_path());
    const auto root=repo/".local/original-economy/oe20/production-fixtures";
    check(output!=root && below(output,root),"Fixture output must be a new private OE20 subdirectory");
    check(!below(output,data) && !below(data,output),"Fixture output overlaps source data");
    f::fs::path current=output.root_path();
    for (const auto& part:output.relative_path()) {
        check(part!=".." && part!=".","Fixture output must have a canonical path");current/=part;
        std::error_code error;
        check(!f::fs::is_symlink(f::fs::symlink_status(current,error)),"Fixture output traverses a symlink");
    }
    check(!f::fs::exists(output),"Fixture output already exists");
    save::validate_save_target(output/"fixture.json",data);
}
void ticks(sim::World& world,int count) {for (int i=0;i<count;++i) world.tick();}
json inspect_food(const sim::World& world) {
    const auto& food=world.original_production_state()->food;
    json farms=json::array(),stocks=json::array(),trips=json::array();
    sim::OriginalFoodCounters produced{},held{},carried{},consumed{};
    for(const auto& farm:food.farms) {
        produced[static_cast<unsigned>(farm.crop)]+=farm.harvested;
        farms.push_back({{"farm_id",static_cast<unsigned>(farm.farm_id)},
            {"field_id",farm.field_id ? json(static_cast<unsigned>(*farm.field_id)):json(nullptr)},
            {"crop",static_cast<unsigned>(farm.crop)},{"harvested",farm.harvested}});
    }
    for(const auto& stock:food.stocks) {
        json witness=nullptr;unsigned quality=0;
        for(const auto& house:world.original_economy_state()->houses)
            if(house.building_id==stock.building_id)quality=house.house.food_quality;
        if(stock.quality_witness) {
            const auto& w=*stock.quality_witness;
            witness={{"courier_id",static_cast<unsigned>(w.transfer.courier_id)},{"sequence",w.transfer.sequence},
                {"tick",w.tick},{"accepted",w.accepted},{"consumed_before",w.consumed_before},{"previous_quality",w.previous_quality}};
        }
        for(std::size_t g=0;g<2;++g){held[g]+=static_cast<std::uint64_t>(stock.amounts[g]);consumed[g]+=stock.consumed[g];}
        stocks.push_back({{"id",static_cast<unsigned>(stock.building_id)},
            {"kind",static_cast<unsigned>(world.building(stock.building_id).kind)},
            {"amounts",stock.amounts},{"received",stock.received},{"departed",stock.departed},
            {"returned",stock.returned},{"consumed",stock.consumed},{"missed_demands",stock.missed_demands},
            {"quality",quality},{"quality_witness",witness}});
    }
    for(const auto& trip:food.trips) {
        sim::OriginalFoodAmounts cargo{};
        for(std::size_t g=0;g<2;++g) {
            cargo[g]=trip.loaded[g]-trip.delivered[g]-trip.returned[g];
            check(cargo[g]>=0,"Food trip has negative remaining Cargo");carried[g]+=static_cast<std::uint64_t>(cargo[g]);
        }
        trips.push_back({{"courier_id",static_cast<unsigned>(trip.transfer.courier_id)},{"sequence",trip.transfer.sequence},
            {"source",static_cast<unsigned>(trip.source)},{"target",static_cast<unsigned>(trip.target)},
            {"phase",static_cast<unsigned>(trip.phase)},{"loaded",trip.loaded},{"delivered",trip.delivered},
            {"returned",trip.returned},{"cargo",cargo},{"batch_quality",sim::original_food_batch_quality(cargo).value}});
    }
    for(std::size_t g=0;g<2;++g)check(produced[g]==held[g]+carried[g]+consumed[g],"Actual Food Crop mass is not conserved");
    return {{"farms",farms},{"stocks",stocks},{"trips",trips},{"produced",produced},
        {"stock",held},{"cargo",carried},{"consumed",consumed},{"conserved",true}};
}
std::uint64_t countdowns(std::uint64_t tick) {return tick<36 ? 0:1+(tick-36)/51;}
json inspect_services(const sim::World& world) {
    const auto& state=world.original_production_state()->services;
    json providers=json::array(),trips=json::array(),witnesses=json::array();
    for (const auto& p:state.providers) {
        original::ModelBuilding model_type;
        switch (p.type) {
        case sim::OriginalServiceProviderType::Herbalist207: model_type=original::ModelBuilding::HerbalistsStall;break;
        case sim::OriginalServiceProviderType::Acupuncture208: model_type=original::ModelBuilding::AcupuncturistsClinic;break;
        case sim::OriginalServiceProviderType::Ancestor214: model_type=original::ModelBuilding::AncestralShrine;break;
        default: throw std::runtime_error("Unknown paid original service provider");
        }
        providers.push_back({{"id",static_cast<unsigned>(p.building_id)},{"type",static_cast<unsigned>(p.type)},
            {"model_type",static_cast<unsigned>(model_type)},
            {"cost",world.original_rules_model()->building_value(model_type,original::BuildingField::Cost)},
            {"workers_required",world.workforce_required(p.building_id)}});
    }
    for (const auto& t:state.trips)
        trips.push_back({{"courier_id",static_cast<unsigned>(t.transfer.courier_id)},{"sequence",t.transfer.sequence},
            {"target",static_cast<unsigned>(t.target)},{"dispatched_tick",t.dispatched_tick},
            {"arrival_tick",t.arrival_tick ? json(*t.arrival_tick):json(nullptr)},{"visit_applied",t.visit_applied}});
    for (const auto& w:state.witnesses)
        witnesses.push_back({{"house_id",static_cast<unsigned>(w.building_id)},{"role",static_cast<unsigned>(w.role)},
            {"provider_id",static_cast<unsigned>(w.provider_id)},{"courier_id",static_cast<unsigned>(w.transfer.courier_id)},
            {"sequence",w.transfer.sequence},{"arrival_tick",w.arrival_tick}});
    return {{"providers",providers},{"trips",trips},{"witnesses",witnesses},{"countdowns",countdowns(world.ticks())}};
}
json inspect(const sim::World& world) {
    check(world.original_production_enabled() && world.original_production_state() &&
          world.production_balance_valid() && world.population_valid() && world.city_economy_valid() &&
          world.navigation_valid(),"Paid production invariant failed");
    json balances=json::array(),houses=json::array(),buildings=json::array(),couriers=json::array(),stocks=json::array();
    const auto all=world.original_goods_balances();
    for (std::size_t i=0;i<all.size();++i) {
        const auto& b=all[i];
        check(b==world.original_goods_balance(static_cast<original::Goods>(i)) && b.conserved &&
              b.supported==(i<2) && !b.totals.imported && !b.totals.demolished,
              "Unexpected typed goods capability or mass");
        if (i>=2) check(!b.stock && !b.totals.produced && !b.totals.consumed && !b.cargo && !b.source_reserved && !b.incoming_reserved,
                       "Revision20 generated an unsupported luxury good");
        check(b.stock+b.cargo+b.totals.consumed==b.totals.produced,"Physical stock, actual cargo and consumption disagree with produced mass");
        balances.push_back({{"good",i},{"supported",b.supported},{"conserved",b.conserved},
            {"produced",b.totals.produced},{"imported",b.totals.imported},{"consumed",b.totals.consumed},
            {"demolished",b.totals.demolished},{"stock",b.stock},{"cargo",b.cargo},
            {"source_reserved",b.source_reserved},{"incoming_reserved",b.incoming_reserved}});
    }
    const auto& history=world.original_production_state()->house_consumption;
    const auto& homes=world.original_economy_state()->houses;
    check(history.size()==homes.size(),"Consumption history lost a House");
    std::array<std::uint64_t,original::goods_count> consumed{};
    for (std::size_t index=0;index<homes.size();++index) {
        const auto& h=homes[index];check(history[index].building_id==h.building_id,"Consumption House identity mismatch");
        for (std::size_t g=0;g<consumed.size();++g) consumed[g]+=history[index].consumed[g];
        for (std::size_t i=0;i<original::goods_count;++i) if (i>=2)
            check(original::goods_stock(h.house,static_cast<original::Goods>(i))==0,"House acquired a premature delivery");
        check(h.house.hemp>=0 && h.house.hemp<=sim::Rules::household_capacity &&
              h.house.ceramics>=0 && h.house.ceramics<=sim::Rules::household_capacity,
              "House exceeded its actual delivery buffer");
        houses.push_back({{"id",static_cast<unsigned>(h.building_id)},{"type",static_cast<unsigned>(h.house.type)},
            {"occupants",h.house.occupants},{"hemp",h.house.hemp},{"ceramics",h.house.ceramics},
            {"consumed",history[index].consumed},{"food_quality",h.house.food_quality},
            {"service_timers",json::array({h.house.herbalist,h.house.acupuncture,h.house.music,h.house.acrobat,
                h.house.drama,h.house.ancestor,h.house.confucian,h.house.daoist,h.house.buddhist})},
            {"status",h.original_status},{"access_enabled",h.access_enabled},
            {"desirability",world.original_influence_provenance(h.building_id).value}});
    }
    for (std::size_t g=0;g<consumed.size();++g)
        check(consumed[g]==all[g].totals.consumed,"House consumption totals disagree with ledger");
    for (const auto& b:world.buildings()) {
        check(b.population==0 && b.pottery_stock==0 && b.food_stock==0 && b.food_produced==0 &&
              b.food_consumed_total==0 && b.reserved_food_incoming==0 && b.service_until_tick==0 &&
              b.health_protection_until_tick==0 && b.health_risk==0 && b.sick_until_tick==0,"New authority was mirrored into legacy stock or population");
        buildings.push_back({{"id",static_cast<unsigned>(b.id)},{"kind",static_cast<unsigned>(b.kind)},
            {"operating",b.operating_enabled},{"on_fire",world.building_on_fire(b.id)},
            {"placed_tick",b.placed_tick},{"cell",json::array({b.cell.x,b.cell.y})},
            {"workers",world.workers_assigned(b.id)},{"input_clay",b.input_clay},{"output",b.output},
            {"progress",b.progress},{"recipes_completed",b.recipes_completed},{"clay_extracted",b.clay_extracted}});
    }
    for (const auto& c:world.couriers()) {
        json cargo_good=nullptr;int typed_cargo=0;
        for (const auto& row:world.original_goods_state()->cargo)
            if (row.transfer.courier_id==c.id) {cargo_good=static_cast<unsigned>(row.good);typed_cargo=row.amount;}
        const int food_cargo=world.original_food_cargo(c.id);
        check(world.original_courier_cargo(c.id)==typed_cargo+food_cargo,"Public cargo query disagrees with separate actual ledgers");
        couriers.push_back({{"id",static_cast<unsigned>(c.id)},{"role",static_cast<unsigned>(c.role)},
            {"owner",static_cast<unsigned>(c.owner)},{"target",static_cast<unsigned>(c.target)},
            {"phase",static_cast<unsigned>(c.phase)},{"cargo",c.cargo},{"reserved",c.reserved},
            {"typed_cargo",typed_cargo},{"typed_cargo_good",cargo_good},{"food_cargo",food_cargo},
            {"path_vertex",c.path_vertex},{"edge_progress",c.edge_progress},
            {"last_dispatched_target",c.last_dispatched_target ? json(static_cast<unsigned>(*c.last_dispatched_target)):json(nullptr)}});
    }
    for (const auto& row:world.original_goods_state()->stocks)
        stocks.push_back({{"id",static_cast<unsigned>(row.building_id)},
            {"kind",static_cast<unsigned>(world.building(row.building_id).kind)},{"amounts",row.amounts}});
    const auto snapshot=world.snapshot();
    json farms=json::array();std::uint64_t harvested=0;
    for (const auto& farm:snapshot.original_production->hemp_farms) {
        harvested+=farm.harvested;
        farms.push_back({{"farm_id",static_cast<unsigned>(farm.farm_id)},
            {"field_id",farm.field_id ? json(static_cast<unsigned>(*farm.field_id)):json(nullptr)},
            {"harvested",farm.harvested}});
    }
    check(all[0].totals.produced==harvested,"Hemp mass lacks actual harvest history");
    return {{"tick",world.ticks()},{"funds",world.treasury()},{"construction_spent",world.construction_spent_total()},
        {"command_sequence",world.command_sequence()},{"road_revision",snapshot.road_revision},
        {"road_count",snapshot.roads.size()},{"house_count",houses.size()},{"building_count",buildings.size()},
        {"courier_count",couriers.size()},{"population",world.total_population()},
        {"capacity",world.total_population_capacity()},{"workforce_supply",world.workforce_supply()},
        {"workforce_used",world.workforce_used()},{"residents_admitted",snapshot.original_production->residents_admitted},
        {"clay_extracted",world.clay_extracted_total()},{"pottery_completed",world.pottery_completed_total()},
        {"pottery_handed_off",snapshot.original_production->pottery_handed_off},
        {"hemp_harvested",harvested},{"hemp_farms",farms},
        {"rule_version",world.rule_version()},{"development_revision",snapshot.original_production->development_revision},
        {"model_sha256",world.original_rules_model()->provenance().sha256},
        {"difficulty",static_cast<unsigned>(world.original_rules_model()->difficulty())},
        {"shelter_needs",json::array({world.original_rules_model()->house_rules().value(original::ModelHouse::Shelter,original::HouseField::Hemp),
            world.original_rules_model()->house_rules().value(original::ModelHouse::Shelter,original::HouseField::Ceramics)})},
        {"food",inspect_food(world)},{"services",inspect_services(world)},
        {"balances",std::move(balances)},{"houses",std::move(houses)},{"stocks",std::move(stocks)},
        {"buildings",std::move(buildings)},{"couriers",std::move(couriers)}};
}
void save_world(const f::fs::path& data,const f::fs::path& target,const sim::World& world) {
    const auto policy=world.map_permissions();
    save::write_save(target,save::make_document(data,f::map_relative,policy->building_mask(),world),data,policy->building_mask());
}
void continuation(const f::fs::path& data,const f::fs::path& output,const std::string& prefix,sim::World& world) {
    const auto initial=world.ticks();ticks(world,10000);check(world.ticks()==initial+10000,"First replay period drifted");
    save_world(data,output/(prefix+"midpoint.json"),world);
    f::write_text(output/(prefix+"midpoint-report.json"),inspect(world).dump(2)+"\n");
    const auto before=world.snapshot();f::buy(world,{sim::CommandType::PlaceRoad,{23,19}});
    check(world.treasury()==before.treasury-2 && world.construction_spent_total()==before.construction_spent_total+2 &&
          world.command_sequence()==before.command_sequence+1,"Actual paid continuation command changed its contract");
    ticks(world,10000);check(world.ticks()==initial+20000,"Second replay period drifted");
    save_world(data,output/(prefix+"result.json"),world);
    f::write_text(output/(prefix+"result-report.json"),inspect(world).dump(2)+"\n");
}
json checkpoint(const f::fs::path& data,const f::fs::path& output,const char* tag,const sim::World& world) {
    const auto path=output/(std::string(tag)+".json");save_world(data,path,world);
    const auto document=save::read_save(path);const auto policy=world.map_permissions();
    check(document.source_schema_version==26 && document.world.rule_version==10,"Fixture did not write schema26/rule10");
    auto restored=save::restore_original_production_development(document,data,policy->building_mask(),policy);
    f::same_document(save::make_document(data,f::map_relative,policy->building_mask(),world),document);
    check(restored.snapshot()==world.snapshot() && inspect(restored)==inspect(world),"Full checkpoint authority changed");
    const auto bytes=f::read_text(path);save_world(data,path,restored);
    check(f::read_text(path)==bytes,"Canonical schema26 rewrite changed bytes");
    f::write_text(output/(std::string(tag)+"-report.json"),inspect(world).dump(2)+"\n");
    continuation(data,output,std::string(tag)+"-",restored);
    return {{"tag",tag},{"save",path.string()},{"report",(output/(std::string(tag)+"-report.json")).string()},
        {"expected_midpoint",(output/(std::string(tag)+"-midpoint.json")).string()},
        {"expected_midpoint_report",(output/(std::string(tag)+"-midpoint-report.json")).string()},
        {"expected_result",(output/(std::string(tag)+"-result.json")).string()},
        {"expected_report",(output/(std::string(tag)+"-result-report.json")).string()}};
}
void create(const f::fs::path& data,const f::fs::path& output) {
    const auto policy=f::permissions();
    auto world=sim::World::create_original_production_development(policy,save::prepare_original_production_rules_model(data));
    check(world.original_rules_model()->house_rules().value(original::ModelHouse::Shelter,original::HouseField::Hemp)==0 &&
          world.original_rules_model()->house_rules().value(original::ModelHouse::Shelter,original::HouseField::Ceramics)==0,
          "Real-model fixture requires its unchanged Shelter null needs");
    check(world.total_population()==0 && world.workforce_supply()==0 && world.treasury()==12000,
          "Production fixture did not start without residents, workers or inventory");
    for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,8}});
    for (int x=0;x<18;x+=3) f::buy(world,{sim::CommandType::PlaceHousehold,{x,6}});
    f::buy(world,{sim::CommandType::PlaceClaySource,{0,9}});
    f::buy(world,{sim::CommandType::PlacePottery,{8,9}});
    f::buy(world,{sim::CommandType::PlaceWarehouse,{16,9}});
    f::buy(world,{sim::CommandType::PlaceMarket,{21,9}});
    f::buy(world,{sim::CommandType::PlaceOriginalHempFarm,{4,9}});
    const auto clay=*world.building_owner_at({0,9}),pottery=*world.building_owner_at({8,9});
    const auto farm=*world.building_owner_at({4,9});
    sim::Command field{sim::CommandType::PlaceOriginalHempField,{6,9}};field.building_id=farm;f::buy(world,field);
    check(world.construction_spent_total()==1162 && world.treasury()==10838 && world.couriers().size()==6,
          "All actual scene purchases, explicit Field and typed actors were not charged");
    const auto initial=inspect(world);
    f::buy(world,sim::set_building_operation(clay,false));f::buy(world,sim::set_building_operation(farm,false));ticks(world,400);
    check(world.total_population()>0 && world.workforce_supply()>=34 && world.clay_extracted_total()==0 &&
          world.pottery_completed_total()==0 && world.original_goods_balance(original::Goods::Ceramics).stock==0 &&
          world.original_goods_balance(original::Goods::Hemp).totals.produced==0,
          "Stopped producer fabricated output or residents failed genuine migration");
    const auto stopped=inspect(world);f::buy(world,sim::set_building_operation(clay,true));
    f::buy(world,sim::set_building_operation(farm,true));
    bool midroute=false;
    for (int i=0;i<10000 && !midroute;++i) {
        world.tick();for (const auto& c:world.couriers())
            if (c.role==sim::CourierRole::OriginalHempToWarehouse && c.phase==sim::CourierPhase::ToWarehouse &&
                    world.original_courier_cargo(c.id)>0 && c.edge_progress>0) {midroute=true;break;}
    }
    check(midroute,"No actual typed Hemp cargo reached the mid-route checkpoint");
    json checkpoints=json::array();checkpoints.push_back(checkpoint(data,output,"mid-hemp-courier",world));
    auto returning=sim::World::restore_original_production_development(world.snapshot(),policy,world.original_rules_model());
    f::buy(returning,sim::set_building_operation(farm,false));
    const auto has_return=[&] {
        for (const auto& row:returning.original_goods_state()->reservations)
            if (row.source==farm && row.phase==sim::OriginalGoodsTransferPhase::Returning) return true;
        return false;
    };
    for (int i=0;i<1000 && !has_return();++i) returning.tick();
    check(has_return(),"Paused Hemp Farm did not retain its loaded trip as a real return");
    const auto return_save=output/"paused-hemp-return.json";save_world(data,return_save,returning);
    auto return_replay=save::restore_original_production_development(save::read_save(return_save),data,policy->building_mask(),policy);
    for (int i=0;i<1000;++i) {returning.tick();return_replay.tick();}
    check(returning.snapshot()==return_replay.snapshot(),"Paused typed return changed across persistence");
    bool returned=false;
    for (const auto& row:returning.original_goods_state()->reservations)
        returned=returned || (row.source==farm && row.phase==sim::OriginalGoodsTransferPhase::Returned && row.returned>0);
    check(returned,"Paused typed cargo failed to return to protected source space");
    const auto delivered=[&] {
        for (const auto& house:world.original_economy_state()->houses)
            if (house.house.hemp>0 && house.house.ceramics>0) return true;
        return false;
    };
    for (int i=0;i<10000 && !delivered();++i) world.tick();
    check(delivered(),"Actual Market arrivals did not reach two separate goods in one HouseState");
    checkpoints.push_back(checkpoint(data,output,"house-two-goods",world));
    // Test the ordinary incident clock on a separately restored real checkpoint;
    // no event, state, worker or material authority is injected.
    auto fire=sim::World::restore_original_production_development(world.snapshot(),policy,world.original_rules_model());
    f::buy(fire,sim::set_building_operation(clay,false));f::buy(fire,sim::set_building_operation(pottery,false));
    f::buy(fire,sim::set_building_operation(farm,false));
    for (int i=0;i<20000 && !(fire.building_on_fire(clay) && fire.building_on_fire(pottery));++i) fire.tick();
    check(fire.building_on_fire(clay) && fire.building_on_fire(pottery),"Natural placement-relative Fire did not start");
    f::buy(fire,sim::set_building_operation(clay,true));f::buy(fire,sim::set_building_operation(pottery,true));
    const auto extracted=fire.clay_extracted_total(),completed=fire.pottery_completed_total();ticks(fire,200);
    check(fire.clay_extracted_total()==extracted && fire.pottery_completed_total()==completed,
          "Burning sources ignored real Fire gates");
    f::write_text(output/"fixture.json",json{{"status","PASS"},{"authored_geography",true},
        {"snapshot_injection",false},{"positive_runtime_goods",true},{"paid_house_count",6},
        {"paid_road_count",24},{"paid_clay_count",1},{"paid_pottery_count",1},{"paid_warehouse_count",1},{"paid_market_count",1},
        {"paid_hemp_farm_count",1},{"paid_hemp_field_count",1},
        {"initial",initial},{"stopped",stopped},{"final",inspect(world)},
        {"stop_controls_verified",true},{"natural_fire_verified",true},{"fire",inspect(fire)},
        {"paused_hemp_return_verified",true},{"paused_return",inspect(returning)},{"actual_two_good_house_arrival",true},
        {"checkpoints",checkpoints}}.dump(2)+"\n");
}
void create_consumption(const f::fs::path& data,const f::fs::path& output) {
    const auto policy=f::permissions();
    auto world=sim::World::create_original_production_development(policy,save::prepare_original_production_rules_model(data));
    const auto& rules=world.original_rules_model()->house_rules();
    check(rules.value(original::ModelHouse::Shelter,original::HouseField::Hemp)==1 &&
          rules.value(original::ModelHouse::Shelter,original::HouseField::Ceramics)==1 &&
          rules.row_capacity(original::ModelHouse::Shelter)==7,"Positive fixture requires explicit authored 1/1 needs and capacity7");
    for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,8}});
    for (int x=0;x<18;x+=3) f::buy(world,{sim::CommandType::PlaceHousehold,{x,6}});
    f::buy(world,{sim::CommandType::PlaceClaySource,{0,9}});
    f::buy(world,{sim::CommandType::PlacePottery,{8,9}});
    f::buy(world,{sim::CommandType::PlaceWarehouse,{16,9}});
    f::buy(world,{sim::CommandType::PlaceMarket,{21,9}});
    f::buy(world,{sim::CommandType::PlaceOriginalHempFarm,{4,9}});
    sim::Command field{sim::CommandType::PlaceOriginalHempField,{6,9}};
    field.building_id=*world.building_owner_at({4,9});f::buy(world,field);
    check(world.construction_spent_total()==1162 && world.total_population()==0 && world.couriers().size()==6,
          "Authored-need scene was not constructed through regular paid commands");
    const auto initial=inspect(world);const auto market=*world.building_owner_at({21,9});
    json boundaries=json::array(),checkpoints=json::array();
    for (const auto step:{0,7}) {
        bool ready=false;
        for (int i=0;i<30000 && !ready;++i) {
            const auto dispatch=original::next_scheduler_dispatch(world.ticks());
            int hemp=0,ceramics=0;
            for (const auto& h:world.original_economy_state()->houses) {hemp+=h.house.hemp;ceramics+=h.house.ceramics;}
            ready=dispatch && dispatch->consumption && dispatch->position.step==step && hemp>0 && ceramics>0;
            if (!ready) world.tick();
        }
        check(ready,"Paid deliveries did not reach a positive 0/7 consumption boundary");
        f::buy(world,sim::set_building_operation(market,false));
        const auto before=world.snapshot();const auto before_report=inspect(world);
        const std::string tag="step-"+std::to_string(step)+"-before";
        checkpoints.push_back(checkpoint(data,output,tag.c_str(),world));
        world.tick();const auto at_report=inspect(world);json deltas=json::array();
        std::array<std::uint64_t,2> totals{};
        for (std::size_t h=0;h<before.original_economy->houses.size();++h) {
            const auto& previous=before.original_economy->houses[h];
            const auto expected=original::consume_goods(rules,previous.house);
            check(world.original_economy_state()->houses[h].house==expected.house,
                  "Consumption boundary had an unexpected House arrival or wrong debit");
            for (std::size_t g=0;g<2;++g) {
                totals[g]+=static_cast<std::uint64_t>(expected.consumed[g]);
                check(world.original_production_state()->house_consumption[h].consumed[g]==
                      before.original_production->house_consumption[h].consumed[g]+static_cast<std::uint64_t>(expected.consumed[g]),
                      "Boundary charged the House audit incorrectly");
            }
            deltas.push_back({{"building_id",static_cast<unsigned>(previous.building_id)},{"consumed",expected.consumed}});
        }
        check(totals[0]>0 && totals[1]>0,"Both actual goods must be consumed at each proved boundary");
        save_world(data,output/("step-"+std::to_string(step)+"-at.json"),world);
        const auto charged=world.original_production_state()->house_consumption;
        world.tick();const auto after_report=inspect(world);
        check(charged==world.original_production_state()->house_consumption,"Following scheduler phase repeated consumption");
        save_world(data,output/("step-"+std::to_string(step)+"-after.json"),world);
        boundaries.push_back({{"step",step},{"before",before_report},{"at",at_report},{"after",after_report},
            {"expected_house_deltas",deltas},{"house_arrivals_at_boundary",false}});
        f::buy(world,sim::set_building_operation(market,true));
    }
    f::write_text(output/"fixture.json",json{{"status","PASS"},{"authored_geography",true},
        {"authored_model_needs",true},{"original_model_parity",false},{"snapshot_injection",false},
        {"positive_runtime_consumption",true},{"paid_house_count",6},{"paid_road_count",24},
        {"initial",initial},{"final",inspect(world)},{"boundaries",boundaries},{"checkpoints",checkpoints}}.dump(2)+"\n");
}
void create_food(const f::fs::path& data,const f::fs::path& output) {
    const auto policy=f::permissions();
    auto world=sim::World::create_original_production_development(policy,save::prepare_original_production_rules_model(data));
    for(const int y:{5,8})for(int x=0;x<24;++x)f::buy(world,{sim::CommandType::PlaceRoad,{x,y}});
    for(const int y:{6,7})f::buy(world,{sim::CommandType::PlaceRoad,{23,y}});
    for(int x=0;x<24;++x)f::buy(world,{sim::CommandType::PlaceRoad,{x,12}});
    for(const int y:{9,10,11})f::buy(world,{sim::CommandType::PlaceRoad,{23,y}});
    for(const int y:{3,6})for(int x=0;x<18;x+=3)f::buy(world,{sim::CommandType::PlaceHousehold,{x,y}});
    f::buy(world,{sim::CommandType::PlaceOriginalFoodFarm,{0,13}});
    f::buy(world,{sim::CommandType::PlaceOriginalFoodFarm,{5,13}});
    const auto wheat=*world.building_owner_at({0,13}),millet=*world.building_owner_at({5,13});
    sim::Command field{sim::CommandType::PlaceOriginalWheatField,{3,13}};field.building_id=wheat;f::buy(world,field);
    field={sim::CommandType::PlaceOriginalMilletField,{8,13}};field.building_id=millet;f::buy(world,field);
    f::buy(world,{sim::CommandType::PlaceMarket,{12,13}});const auto market=*world.building_owner_at({12,13});
    const auto initial=inspect(world);
    check(world.ticks()==0&&world.total_population()==0&&world.workforce_supply()==0&&world.couriers().size()==4&&
          world.construction_spent_total()==1363&&world.treasury()==10637,
          "Original-model Food scene must pay12Houses/77Roads/Farm193x2/Field195+196/Market without workers or stocks");
    f::buy(world,sim::set_building_operation(wheat,false));f::buy(world,sim::set_building_operation(millet,false));ticks(world,400);
    check(world.total_population()==84&&world.workforce_required(wheat)==22&&world.workforce_required(millet)==22,
          "Actual original-model Food producers lack12paidHouse immigration or model22workers");
    for(const auto& farm:world.original_production_state()->food.farms)check(!farm.harvested,"Stopped Food producer harvested");
    const auto stopped=inspect(world);f::buy(world,sim::set_building_operation(wheat,true));
    const auto wait=[&](auto done,const char* failure) {
        for(int i=0;i<30000&&!done();++i)world.tick();check(done(),failure);
    };
    sim::CourierId loaded=static_cast<sim::CourierId>(0);
    wait([&] {
        for(const auto& c:world.couriers())
            if(c.owner==wheat&&c.role==sim::CourierRole::OriginalFoodToMarket&&
               c.phase==sim::CourierPhase::ToWarehouse&&c.edge_progress>0&&world.original_food_cargo(c.id)>0) {
                loaded=c.id;return true;
            }
        return false;
    },
        "Real Food Farm courier never reached its loaded begun edge");
    json checkpoints=json::array();checkpoints.push_back(checkpoint(data,output,"mid-food-courier",world));
    auto returning=sim::World::restore_original_production_development(world.snapshot(),policy,world.original_rules_model());
    const auto moving=returning.courier(loaded);const auto retained=returning.original_food_cargo(loaded);
    f::buy(returning,sim::set_building_operation(wheat,false));
    check(returning.courier(loaded).phase==sim::CourierPhase::Returning&&returning.courier(loaded).edge_progress==moving.edge_progress&&
          returning.original_food_cargo(loaded)==retained,"Food return did not preserve its actual begun edge/cargo");
    checkpoints.push_back(checkpoint(data,output,"food-return",returning));
    auto returned=sim::World::restore_original_production_development(returning.snapshot(),policy,world.original_rules_model());
    for(int i=0;i<2000&&returned.courier(loaded).phase!=sim::CourierPhase::IdleAtWorkshop;++i)returned.tick();
    check(!returned.original_food_cargo(loaded),"Actual Food did not complete its return to source");
    const auto any_quality=[&](unsigned minimum) {
        for(const auto& h:world.original_economy_state()->houses)
            if(h.house.food_quality>=minimum)return true;
        return false;
    };
    wait([&]{return any_quality(20);},"Actual Wheat arrival did not supply low Food quality");
    const auto low=inspect(world);
    // Keep the destination Market running. A paid road interruption separates
    // only its residential branch while both Farms can genuinely deliver.
    const sim::Command cut{sim::CommandType::RemoveRoad,{23,10}};
    wait([&]{return world.validate(cut).accepted;},"Residential Food branch could not be safely disconnected");
    f::buy(world,cut);f::buy(world,sim::set_building_operation(millet,true));
    wait([&] {
        for(const auto& s:world.original_production_state()->food.stocks)
            if(s.building_id==market)return s.amounts[0]>0&&s.amounts[1]>0;
        return false;
    },
        "Both actual Farm crops never reached the same paid Market");
    const auto mixed_market=inspect(world);f::buy(world,{sim::CommandType::PlaceRoad,{23,10}});
    wait([&] {
        for(const auto& trip:world.original_production_state()->food.trips)
            if(world.courier(trip.transfer.courier_id).role==sim::CourierRole::OriginalFoodToHouse&&
               trip.loaded[0]>0&&trip.loaded[1]>0&&world.original_food_cargo(trip.transfer.courier_id)>0&&
               world.courier(trip.transfer.courier_id).edge_progress>0)return true;
        return false;
    },
        "Actual mixed Market cargo never reached a begun road edge");
    checkpoints.push_back(checkpoint(data,output,"mixed-food-courier",world));
    wait([&] {
        for(const auto& s:world.original_production_state()->food.stocks)
            if(s.quality_witness&&s.quality_witness->accepted[0]>0&&s.quality_witness->accepted[1]>0)return true;
        return false;
    },
        "Actual mixed Food never reached its House quality witness");
    const auto mixed_arrival=inspect(world);f::buy(world,sim::set_building_operation(wheat,false));
    wait([&]{return any_quality(80);},"Pure actual Millet arrival never reached its authored upper quality class");
    checkpoints.push_back(checkpoint(data,output,"house-food-quality",world));
    f::write_text(output/"fixture.json",json{{"status","PASS"},{"authored_geography",true},{"snapshot_injection",false},
        {"real_model_food",true},{"positive_runtime_food",true},{"paid_house_count",12},{"paid_road_count",77},
        {"paid_food_farm_count",2},{"paid_food_field_count",2},{"paid_market_count",1},
        {"initial",initial},{"stopped",stopped},{"low_quality",low},{"mixed_market",mixed_market},
        {"mixed_arrival",mixed_arrival},{"paused_return",inspect(returned)},{"final",inspect(world)},
        {"actual_mixed_cargo",true},{"actual_food_return",true},{"actual_quality_classes",json::array({20,80})},
        {"checkpoints",checkpoints}}.dump(2)+"\n");
}
void create_services(const f::fs::path& data,const f::fs::path& output) {
    const auto policy=f::permissions();
    auto world=sim::World::create_original_production_development(policy,save::prepare_original_production_rules_model(data));
    for (const int y:{5,8,12}) for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,y}});
    for (const int y:{6,7,9,10,11}) f::buy(world,{sim::CommandType::PlaceRoad,{23,y}});
    for (const int y:{3,6}) for (int x=0;x<18;x+=3) f::buy(world,{sim::CommandType::PlaceHousehold,{x,y}});
    f::buy(world,{sim::CommandType::PlaceOriginalFoodFarm,{0,13}});
    f::buy(world,{sim::CommandType::PlaceOriginalFoodFarm,{5,13}});
    const auto wheat=*world.building_owner_at({0,13}),millet=*world.building_owner_at({5,13});
    sim::Command field{sim::CommandType::PlaceOriginalWheatField,{3,13}};field.building_id=wheat;f::buy(world,field);
    field={sim::CommandType::PlaceOriginalMilletField,{8,13}};field.building_id=millet;f::buy(world,field);
    f::buy(world,{sim::CommandType::PlaceMarket,{12,13}});
    f::buy(world,{sim::CommandType::PlaceHealthPost,{16,13}});
    f::buy(world,{sim::CommandType::PlaceServicePost,{19,13}});
    const auto herbalist=*world.building_owner_at({16,13}),acupuncture=*world.building_owner_at({19,13});
    const auto initial=inspect(world);
    check(world.total_population()==0&&world.couriers().size()==6&&world.construction_spent_total()==1443&&world.treasury()==10557,
          "Real-model medical/Food scene must pay12Houses/77Roads/twoFarms/twoFields/Market/207/208");
    f::buy(world,sim::set_building_operation(herbalist,false));
    f::buy(world,sim::set_building_operation(acupuncture,false));ticks(world,400);
    check(world.total_population()==84&&world.workforce_supply()==84&&world.workforce_required(herbalist)==7&&
          world.workforce_required(acupuncture)==8&&world.workforce_required(wheat)==22&&world.workforce_required(millet)==22,
          "Medical scene lacks actual paid-house immigration or exact prepared staffing");
    check(world.original_production_state()->services.trips.empty()&&world.original_production_state()->services.witnesses.empty(),
          "Paused medical sources created dispatch or visit authority");
    const auto stopped=inspect(world);
    const auto wait=[&](auto done,const char* failure) {
        for (int i=0;i<30000&&!done();++i) world.tick();
        check(done(),failure);
    };
    f::buy(world,sim::set_building_operation(herbalist,true));
    f::buy(world,sim::set_building_operation(acupuncture,true));
    wait([&] {
        std::size_t moving=0;
        for (const auto& c:world.couriers()) if (c.owner==herbalist||c.owner==acupuncture)
            moving+=c.phase==sim::CourierPhase::ToWarehouse&&c.edge_progress>0;
        return moving==2;
    },"Both real medical visitors did not reach outbound begun edges");
    check(world.original_production_state()->services.witnesses.empty(),"Medical dispatch or begun edge granted a timer");
    json checkpoints=json::array();checkpoints.push_back(checkpoint(data,output,"mid-services-couriers",world));
    f::buy(world,sim::set_building_operation(herbalist,false));
    f::buy(world,sim::set_building_operation(acupuncture,false));
    wait([&] {
        std::size_t idle=0;
        for (const auto& c:world.couriers()) if (c.owner==herbalist||c.owner==acupuncture)
            idle+=c.phase==sim::CourierPhase::IdleAtWorkshop;
        return idle==2&&world.original_production_state()->services.witnesses.size()==2;
    },"Paused outgoing207/208 visitors did not finish their actual visits and return");
    const auto visited=inspect(world);const auto witnesses=world.original_production_state()->services.witnesses;
    check(witnesses[0].building_id==witnesses[1].building_id&&witnesses[0].role!=witnesses[1].role,
          "Two medical visitors did not reach their fixed first paid House");
    checkpoints.push_back(checkpoint(data,output,"house-medical-visits",world));
    const auto first=std::find_if(witnesses.begin(),witnesses.end(),[](const auto& w){return w.role==original::ServiceRole::Herbalist;});
    check(first!=witnesses.end(),"Actual Herbalist witness missing");
    const auto expiry=36+(countdowns(first->arrival_tick-1)+95)*51;
    check(expiry>world.ticks(),"Medical process fixture missed its future96th countdown");
    while (world.ticks()+1<expiry) world.tick();
    const auto before_expiry=inspect(world);
    checkpoints.push_back(checkpoint(data,output,"service-before-expiry",world));
    world.tick();const auto expired=inspect(world);
    for (const auto& h:world.original_economy_state()->houses) if (h.building_id==first->building_id)
        check(!h.house.herbalist,"Herbalist survived the actual witnessed96th countdown");
    check(world.original_production_state()->services.witnesses==witnesses,"Return/idle/expiry created a new medical visit");
    checkpoints.push_back(checkpoint(data,output,"service-expired",world));
    f::write_text(output/"fixture.json",json{{"status","PASS"},{"snapshot_injection",false},{"authored_geography",true},
        {"real_model_services",true},{"actual_two_provider_arrivals",true},{"paused_outbound_tours_finished",true},
        {"paid_house_count",12},{"paid_road_count",77},{"paid_food_farm_count",2},{"paid_food_field_count",2},
        {"paid_market_count",1},{"paid_herbalist_count",1},{"paid_acupuncture_count",1},
        {"initial",initial},{"stopped",stopped},{"visited",visited},{"before_expiry",before_expiry},{"expired",expired},
        {"expiry_tick",expiry},{"checkpoints",checkpoints}}.dump(2)+"\n");
}
void create_ancestor(const f::fs::path& data,const f::fs::path& output) {
    const auto policy=f::permissions();
    auto world=sim::World::create_original_production_development(policy,save::prepare_original_production_rules_model(data));
    for (const int y:{5,8,12}) for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,y}});
    for (const int y:{6,7,9,10,11}) f::buy(world,{sim::CommandType::PlaceRoad,{23,y}});
    for (const int y:{3,6}) for (int x=0;x<18;x+=3) f::buy(world,{sim::CommandType::PlaceHousehold,{x,y}});
    f::buy(world,{sim::CommandType::PlaceOriginalFoodFarm,{0,13}});
    f::buy(world,{sim::CommandType::PlaceOriginalFoodFarm,{5,13}});
    const auto wheat=*world.building_owner_at({0,13}),millet=*world.building_owner_at({5,13});
    sim::Command field{sim::CommandType::PlaceOriginalWheatField,{3,13}};field.building_id=wheat;f::buy(world,field);
    field={sim::CommandType::PlaceOriginalMilletField,{8,13}};field.building_id=millet;f::buy(world,field);
    f::buy(world,{sim::CommandType::PlaceMarket,{12,13}});
    const std::array<std::pair<sim::CommandType,sim::Cell>,3> sources{{
        {sim::CommandType::PlaceHealthPost,{16,13}},
        {sim::CommandType::PlaceServicePost,{19,13}},
        {sim::CommandType::PlaceOriginalAncestorShrine,{22,13}}}};
    std::array<sim::BuildingId,3> providers{};
    for (std::size_t i=0;i<sources.size();++i) {
        f::buy(world,{sources[i].first,sources[i].second});providers[i]=*world.building_owner_at(sources[i].second);
    }
    const auto shrine=providers[2];const auto home=*world.building_owner_at({0,3});
    const auto found=std::find_if(world.couriers().begin(),world.couriers().end(),[&](const auto& c){return c.owner==shrine;});
    check(found!=world.couriers().end()&&found->role==sim::CourierRole::OriginalAncestor,"Paid214 has no own actual Ancestor Courier");
    const auto visitor=found->id;
    const auto initial=inspect(world);
    check(world.total_population()==0&&world.couriers().size()==7&&world.construction_spent_total()==1483&&world.treasury()==10517,
          "Real Ancestor scene must pay12Houses/77Roads/twoFarms/twoFields/Market/207/208/214");
    for (const auto source:providers) f::buy(world,sim::set_building_operation(source,false));
    ticks(world,400);const auto stopped=inspect(world);
    check(world.total_population()==84&&world.workforce_supply()==84&&world.workforce_required(shrine)==4&&
          world.original_production_state()->services.trips.empty()&&world.original_production_state()->services.witnesses.empty(),
          "Paid Ancestor scene lacks real immigration or paused sources granted visits");
    const auto wait=[&](auto done,const char* failure,int limit=30000) {
        for (int i=0;i<limit&&!done();++i) world.tick();
        check(done(),failure);
    };
    const auto latest=[&]() -> const sim::OriginalServiceTrip& {
        for (const auto& row:world.original_production_state()->services.trips)
            if (row.transfer.courier_id==visitor) return row;
        throw std::runtime_error("Ancestor scene lost its real latest trip");
    };
    const auto witness=[&]() -> const sim::OriginalServiceWitness& {
        for (const auto& row:world.original_production_state()->services.witnesses)
            if (row.building_id==home&&row.role==original::ServiceRole::Ancestor) return row;
        throw std::runtime_error("Ancestor scene lost the actual first-House visit");
    };
    const auto timer=[&]() {
        for (const auto& row:world.original_economy_state()->houses) if (row.building_id==home) return row.house.ancestor;
        throw std::runtime_error("Ancestor scene lost its actual first House");
    };
    for (const auto source:providers) f::buy(world,sim::set_building_operation(source,true));
    wait([&]{return world.courier(visitor).phase==sim::CourierPhase::ToWarehouse&&world.courier(visitor).edge_progress>0;},
         "Real Ancestor visitor never reached its outbound begun edge");
    check(!latest().arrival_tick&&!timer()&&world.workforce_used()==67,
          "Ancestor dispatch granted religion or combined paid scene lacks67 actual workers");
    const auto midroute=inspect(world);json checkpoints=json::array();
    checkpoints.push_back(checkpoint(data,output,"mid-ancestor-courier",world));
    for (const auto source:providers) f::buy(world,sim::set_building_operation(source,false));
    const sim::Command cut{sim::CommandType::RemoveRoad,{23,10}};
    wait([&]{return world.validate(cut).accepted;},"Ancestor route cut never became safe");f::buy(world,cut);
    ticks(world,250);
    check(!latest().arrival_tick&&!timer(),"Ancestor visitor crossed a disconnected trunk or supplied on Return");
    const auto disconnected=inspect(world);
    checkpoints.push_back(checkpoint(data,output,"ancestor-road-disconnected",world));
    const auto before_repair=world.snapshot();f::buy(world,{sim::CommandType::PlaceRoad,{23,10}});
    check(world.treasury()==before_repair.treasury-2&&world.construction_spent_total()==before_repair.construction_spent_total+2,
          "Ancestor road repair did not pay the actual road cost");
    const auto repaired=inspect(world);
    wait([&]{return world.courier(visitor).phase==sim::CourierPhase::IdleAtWorkshop&&latest().visit_applied;},
         "Paused disconnected Ancestor tour did not finish after actual paid road repair");
    check(witness().transfer==latest().transfer&&latest().target==home&&timer()>0,
          "Ancestor arrival lost its exact first House/provider/actor witness");
    const auto visited=inspect(world);const auto first=witness();
    checkpoints.push_back(checkpoint(data,output,"house-ancestor-visit",world));
    const auto expiry=36+(countdowns(first.arrival_tick-1)+39)*51;
    check(expiry>world.ticks(),"Ancestor fixture missed its40th actual countdown");
    while (world.ticks()+1<expiry) world.tick();
    check(timer()==1,"Ancestor timer did not reach exactly1 before expiry");
    const auto before_expiry=inspect(world);
    checkpoints.push_back(checkpoint(data,output,"ancestor-before-expiry",world));
    world.tick();check(!timer()&&witness()==first,"Ancestor40th countdown changed witness or retained religion");
    const auto expired=inspect(world);
    checkpoints.push_back(checkpoint(data,output,"ancestor-expired",world));
    f::buy(world,sim::set_building_operation(shrine,true));
    wait([&]{return timer()>0&&witness().transfer.sequence>first.transfer.sequence;},
         "Real Ancestor target rotation never revisited the expired first House");
    f::buy(world,sim::set_building_operation(shrine,false));
    const auto revisited=inspect(world);
    checkpoints.push_back(checkpoint(data,output,"ancestor-revisited",world));
    f::write_text(output/"fixture.json",json{{"status","PASS"},{"snapshot_injection",false},{"authored_geography",true},
        {"real_model_ancestor",true},{"actual_ancestor_arrival",true},{"actual_ancestor_revisit",true},
        {"paid_road_repair",true},{"paused_outbound_tour_finished",true},
        {"paid_house_count",12},{"paid_road_count",77},{"paid_food_farm_count",2},{"paid_food_field_count",2},
        {"paid_market_count",1},{"paid_herbalist_count",1},{"paid_acupuncture_count",1},{"paid_ancestor_count",1},
        {"initial",initial},{"stopped",stopped},{"midroute",midroute},{"disconnected",disconnected},{"repaired",repaired},
        {"visited",visited},{"before_expiry",before_expiry},{"expired",expired},{"revisited",revisited},
        {"expiry_tick",expiry},{"checkpoints",checkpoints}}.dump(2)+"\n");
}
void resume(const f::fs::path& data,const f::fs::path& output,const f::fs::path& input) {
    const auto before=f::read_text(input);const auto document=save::read_save(input);const auto policy=f::permissions();
    auto world=save::restore_original_production_development(document,data,policy->building_mask(),policy);
    check(world.snapshot()==document.world,"Separate OS process changed full production restore");
    continuation(data,output,"",world);check(f::read_text(input)==before,"Replay modified input save");
}
}
int main(int argc,char** argv) {
    try {
        const std::string mode=argc>1 ? argv[1]:"";
        check((argc==4 && (mode=="--create" || mode=="--create-consumption" || mode=="--create-food" || mode=="--create-services" || mode=="--create-ancestor")) || (argc==5 && mode=="--resume"),
              "Usage: production-fixture --create DATA OUT | --create-consumption AUTHORED_DATA OUT | --create-food DATA OUT | --create-services DATA OUT | --create-ancestor DATA OUT | --resume DATA OUT SAVE");
        const auto data=f::fs::canonical(argv[2]);const auto output=f::fs::absolute(argv[3]);
        validate_output(output,data);f::fs::create_directories(output);
        if (mode=="--create") create(data,output);
        else if (mode=="--create-consumption") create_consumption(data,output);
        else if (mode=="--create-food") create_food(data,output);
        else if (mode=="--create-services") create_services(data,output);
        else if (mode=="--create-ancestor") create_ancestor(data,output);
        else resume(data,output,f::fs::canonical(argv[4]));
        std::cout<<"Original paid production fixture PASS\n";return 0;
    } catch (const std::exception& error) {std::cerr<<"Original production fixture FAIL: "<<error.what()<<'\n';return 1;}
}
