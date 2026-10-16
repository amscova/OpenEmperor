#include "../tests/OriginalInfluenceFixture.h"
#include "simulation/original/GoodsRules.h"

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
    const auto root=repo/".local/original-economy/oe14/production-fixtures";
    check(output!=root && below(output,root),"Fixture output must be a new private OE14 subdirectory");
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
    json balances=json::array(),houses=json::array(),buildings=json::array(),couriers=json::array();
    const auto all=world.original_goods_balances();
    for (std::size_t i=0;i<all.size();++i) {
        const auto& b=all[i];
        check(b==world.original_goods_balance(static_cast<original::Goods>(i)) && b.conserved &&
              b.supported==(i==1) && !b.totals.imported && !b.totals.consumed && !b.totals.demolished &&
              !b.cargo && !b.source_reserved && !b.incoming_reserved,"Unexpected typed goods capability or mass");
        if (i!=1) check(!b.stock && !b.totals.produced,"Revision14 generated an unsupported good");
        balances.push_back({{"good",i},{"supported",b.supported},{"conserved",b.conserved},
            {"produced",b.totals.produced},{"imported",b.totals.imported},{"consumed",b.totals.consumed},
            {"demolished",b.totals.demolished},{"stock",b.stock},{"cargo",b.cargo},
            {"source_reserved",b.source_reserved},{"incoming_reserved",b.incoming_reserved}});
    }
    for (const auto& h:world.original_economy_state()->houses) {
        for (std::size_t i=0;i<original::goods_count;++i)
            check(original::goods_stock(h.house,static_cast<original::Goods>(i))==0,"House acquired a premature delivery");
        houses.push_back({{"id",static_cast<unsigned>(h.building_id)},{"type",static_cast<unsigned>(h.house.type)},
            {"occupants",h.house.occupants},{"status",h.original_status},{"access_enabled",h.access_enabled},
            {"desirability",world.original_influence_provenance(h.building_id).value}});
    }
    for (const auto& b:world.buildings()) {
        check(b.population==0 && b.pottery_stock==0,"New authority was mirrored into legacy stock or population");
        buildings.push_back({{"id",static_cast<unsigned>(b.id)},{"kind",static_cast<unsigned>(b.kind)},
            {"operating",b.operating_enabled},{"on_fire",world.building_on_fire(b.id)},
            {"workers",world.workers_assigned(b.id)},{"input_clay",b.input_clay},{"output",b.output},
            {"progress",b.progress},{"recipes_completed",b.recipes_completed},{"clay_extracted",b.clay_extracted}});
    }
    for (const auto& c:world.couriers())
        couriers.push_back({{"id",static_cast<unsigned>(c.id)},{"role",static_cast<unsigned>(c.role)},
            {"owner",static_cast<unsigned>(c.owner)},{"target",static_cast<unsigned>(c.target)},
            {"phase",static_cast<unsigned>(c.phase)},{"cargo",c.cargo},{"reserved",c.reserved},
            {"path_vertex",c.path_vertex},{"edge_progress",c.edge_progress}});
    const auto snapshot=world.snapshot();
    return {{"tick",world.ticks()},{"funds",world.treasury()},{"construction_spent",world.construction_spent_total()},
        {"command_sequence",world.command_sequence()},{"road_revision",snapshot.road_revision},
        {"road_count",snapshot.roads.size()},{"house_count",houses.size()},{"building_count",buildings.size()},
        {"courier_count",couriers.size()},{"population",world.total_population()},
        {"capacity",world.total_population_capacity()},{"workforce_supply",world.workforce_supply()},
        {"workforce_used",world.workforce_used()},{"residents_admitted",snapshot.original_production->residents_admitted},
        {"clay_extracted",world.clay_extracted_total()},{"pottery_completed",world.pottery_completed_total()},
        {"pottery_handed_off",snapshot.original_production->pottery_handed_off},
        {"rule_version",world.rule_version()},{"development_revision",snapshot.original_production->development_revision},
        {"model_sha256",world.original_rules_model()->provenance().sha256},
        {"difficulty",static_cast<unsigned>(world.original_rules_model()->difficulty())},
        {"balances",std::move(balances)},{"houses",std::move(houses)},
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
    auto world=sim::World::create_original_production_development(policy,save::prepare_original_rules_model(data));
    check(world.total_population()==0 && world.workforce_supply()==0 && world.treasury()==12000,
          "Production fixture did not start without residents, workers or inventory");
    for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,8}});
    for (int x=0;x<12;x+=3) f::buy(world,{sim::CommandType::PlaceHousehold,{x,6}});
    f::buy(world,{sim::CommandType::PlaceClaySource,{0,9}});
    f::buy(world,{sim::CommandType::PlacePottery,{8,9}});
    f::buy(world,{sim::CommandType::PlaceWarehouse,{16,9}});
    const auto clay=*world.building_owner_at({0,9}),pottery=*world.building_owner_at({8,9});
    check(world.construction_spent_total()==818 && world.treasury()==11182,"All actual scene purchases were not charged");
    const auto initial=inspect(world);
    f::buy(world,sim::set_building_operation(clay,false));ticks(world,400);
    check(world.total_population()>0 && world.workforce_supply()>=12 && world.clay_extracted_total()==0 &&
          world.pottery_completed_total()==0 && world.original_goods_balance(original::Goods::Ceramics).stock==0,
          "Stopped producer fabricated output or residents failed genuine migration");
    const auto stopped=inspect(world);f::buy(world,sim::set_building_operation(clay,true));
    bool midroute=false;
    for (int i=0;i<10000 && !midroute;++i) {
        world.tick();for (const auto& c:world.couriers())
            if (c.role==sim::CourierRole::Pottery && c.phase==sim::CourierPhase::ToWarehouse &&
                    c.cargo>0 && c.edge_progress>0) {midroute=true;break;}
    }
    check(midroute,"No actual moving Pottery cargo reached the mid-route checkpoint");
    json checkpoints=json::array();checkpoints.push_back(checkpoint(data,output,"mid-pottery-courier",world));
    for (int i=0;i<10000 && world.original_goods_balance(original::Goods::Ceramics).stock==0;++i) world.tick();
    check(world.original_goods_balance(original::Goods::Ceramics).stock>0,"No actual Warehouse arrival created Ceramics");
    checkpoints.push_back(checkpoint(data,output,"warehouse-ceramics",world));
    // Test the ordinary incident clock on a separately restored real checkpoint;
    // no event, state, worker or material authority is injected.
    auto fire=sim::World::restore_original_production_development(world.snapshot(),policy,world.original_rules_model());
    f::buy(fire,sim::set_building_operation(clay,false));f::buy(fire,sim::set_building_operation(pottery,false));
    while (fire.ticks()<2000) fire.tick();
    check(fire.building_on_fire(clay) && fire.building_on_fire(pottery),"Natural placement-relative Fire did not start");
    f::buy(fire,sim::set_building_operation(clay,true));f::buy(fire,sim::set_building_operation(pottery,true));
    const auto extracted=fire.clay_extracted_total(),completed=fire.pottery_completed_total();ticks(fire,200);
    check(fire.clay_extracted_total()==extracted && fire.pottery_completed_total()==completed,
          "Burning sources ignored real Fire gates");
    f::write_text(output/"fixture.json",json{{"status","PASS"},{"authored_geography",true},
        {"snapshot_injection",false},{"positive_runtime_goods",true},{"paid_house_count",4},
        {"paid_road_count",24},{"paid_clay_count",1},{"paid_pottery_count",1},{"paid_warehouse_count",1},
        {"initial",initial},{"stopped",stopped},{"final",inspect(world)},
        {"stop_controls_verified",true},{"natural_fire_verified",true},{"fire",inspect(fire)},
        {"checkpoints",checkpoints}}.dump(2)+"\n");
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
        check((argc==4 && mode=="--create") || (argc==5 && mode=="--resume"),
              "Usage: production-fixture --create DATA OUT | --resume DATA OUT SAVE");
        const auto data=f::fs::canonical(argv[2]);const auto output=f::fs::absolute(argv[3]);
        validate_output(output,data);f::fs::create_directories(output);
        if (mode=="--create") create(data,output);else resume(data,output,f::fs::canonical(argv[4]));
        std::cout<<"Original paid production fixture PASS\n";return 0;
    } catch (const std::exception& error) {std::cerr<<"Original production fixture FAIL: "<<error.what()<<'\n';return 1;}
}
