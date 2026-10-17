#include "../tests/OriginalInfluenceFixture.h"
#include "simulation/original/GoodsRules.h"
#include "simulation/original/OriginalScheduler.h"

#include <algorithm>
#include <iostream>
#include <nlohmann/json.hpp>

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
    const auto root=repo/".local/original-economy/oe17/production-fixtures";
    check(output!=root && below(output,root),"Fixture output must be a new private OE17 subdirectory");
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
                       "Revision17 generated an unsupported good");
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
            {"consumed",history[index].consumed},
            {"status",h.original_status},{"access_enabled",h.access_enabled},
            {"desirability",world.original_influence_provenance(h.building_id).value}});
    }
    for (std::size_t g=0;g<consumed.size();++g)
        check(consumed[g]==all[g].totals.consumed,"House consumption totals disagree with ledger");
    for (const auto& b:world.buildings()) {
        check(b.population==0 && b.pottery_stock==0,"New authority was mirrored into legacy stock or population");
        buildings.push_back({{"id",static_cast<unsigned>(b.id)},{"kind",static_cast<unsigned>(b.kind)},
            {"operating",b.operating_enabled},{"on_fire",world.building_on_fire(b.id)},
            {"workers",world.workers_assigned(b.id)},{"input_clay",b.input_clay},{"output",b.output},
            {"progress",b.progress},{"recipes_completed",b.recipes_completed},{"clay_extracted",b.clay_extracted}});
    }
    for (const auto& c:world.couriers()) {
        json cargo_good=nullptr;
        for (const auto& row:world.original_goods_state()->cargo)
            if (row.transfer.courier_id==c.id) cargo_good=static_cast<unsigned>(row.good);
        couriers.push_back({{"id",static_cast<unsigned>(c.id)},{"role",static_cast<unsigned>(c.role)},
            {"owner",static_cast<unsigned>(c.owner)},{"target",static_cast<unsigned>(c.target)},
            {"phase",static_cast<unsigned>(c.phase)},{"cargo",c.cargo},{"reserved",c.reserved},
            {"typed_cargo",world.original_courier_cargo(c.id)},{"typed_cargo_good",cargo_good},
            {"path_vertex",c.path_vertex},{"edge_progress",c.edge_progress}});
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
    check(world.construction_spent_total()==1162 && world.treasury()==10838 && world.couriers().size()==5,
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
    check(world.construction_spent_total()==1162 && world.total_population()==0 && world.couriers().size()==5,
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
        check((argc==4 && (mode=="--create" || mode=="--create-consumption")) || (argc==5 && mode=="--resume"),
              "Usage: production-fixture --create DATA OUT | --create-consumption AUTHORED_DATA OUT | --resume DATA OUT SAVE");
        const auto data=f::fs::canonical(argv[2]);const auto output=f::fs::absolute(argv[3]);
        validate_output(output,data);f::fs::create_directories(output);
        if (mode=="--create") create(data,output);
        else if (mode=="--create-consumption") create_consumption(data,output);
        else resume(data,output,f::fs::canonical(argv[4]));
        std::cout<<"Original paid production fixture PASS\n";return 0;
    } catch (const std::exception& error) {std::cerr<<"Original production fixture FAIL: "<<error.what()<<'\n';return 1;}
}
