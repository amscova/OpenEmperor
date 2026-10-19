#include "OriginalProductionFixture.h"
#include "simulation/original/OriginalScheduler.h"

#include <nlohmann/json.hpp>
#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <vector>

namespace {
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace save=openemperor::persistence;
namespace orig=openemperor::original;
using json=nlohmann::json;
using f::check;
std::size_t rejected=0;
template<class Action> void refuses(Action action) {
    try {action();} catch (const std::exception&) {++rejected;return;}
    throw std::runtime_error("Malformed housing authority was admitted");
}
template<class Predicate> void until(sim::World& world,Predicate ready,int limit=40000) {
    for (int i=0;i<limit && !ready();++i) world.tick();
    check(ready(),"Actual paid housing scene did not reach its required event");
}
const orig::HouseState& house(const sim::World& world,sim::BuildingId id) {
    for (const auto& h:world.original_economy_state()->houses) if (h.building_id==id) return h.house;
    throw std::runtime_error("Paid housing target is absent");
}
const sim::OriginalHousingHouse& history(const sim::World& world,sim::BuildingId id) {
    for (const auto& h:world.original_production_state()->housing.houses) if (h.building_id==id) return h;
    throw std::runtime_error("Paid House lost its bounded housing history");
}
const sim::OriginalWaterHouseState& water(const sim::World& world,sim::BuildingId id) {
    for (const auto& h:world.original_production_state()->water.houses) if (h.building_id==id) return h;
    throw std::runtime_error("Paid House lost its separate water history");
}
struct Fixture {
    f::Root root;
    std::shared_ptr<const sim::MapPermissions> policy=f::permissions();
    std::shared_ptr<const orig::OriginalRulesModel> model=[&] {
        // Explicitly authored Plain Need1/1; the real original Plain row
        // requires no Hemp/Ceramics. Its separate real-model proof is elsewhere.
        oe_production_fixture::write_model(root.data,7,0,0,0,0,0,0,14,22,1,1);
        return save::prepare_original_production_rules_model(root.data);
    }();
    sim::World world=sim::World::create_original_production_development(policy,model);
    f::fs::path target=root.path/"housing-schema26.json";
    sim::BuildingId home{},well{},ancestor{},neutral{};
    std::vector<sim::BuildingId> sources;
    Fixture() {
        for (const int y:{4,12}) for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,y}});
        for (int y=5;y<12;++y) f::buy(world,{sim::CommandType::PlaceRoad,{23,y}});
        for (int x=0;x<18;x+=3) f::buy(world,{sim::CommandType::PlaceHousehold,{x,2}});
        home=*world.building_owner_at({0,2});
        const auto buy=[&](sim::CommandType type,sim::Cell cell) {
            f::buy(world,{type,cell});const auto id=*world.building_owner_at(cell);sources.push_back(id);
            f::buy(world,sim::set_building_operation(id,false));return id;
        };
        const auto food=buy(sim::CommandType::PlaceOriginalFoodFarm,{0,13});
        sim::Command field{sim::CommandType::PlaceOriginalWheatField,{3,13}};field.building_id=food;f::buy(world,field);
        (void)buy(sim::CommandType::PlaceMarket,{5,13});
        ancestor=buy(sim::CommandType::PlaceOriginalAncestorShrine,{8,13});
        well=buy(sim::CommandType::PlaceOriginalWaterWell,{11,13});
        (void)buy(sim::CommandType::PlaceClaySource,{0,10});
        (void)buy(sim::CommandType::PlacePottery,{4,10});
        const auto warehouse=buy(sim::CommandType::PlaceWarehouse,{7,10});
        const auto hemp=buy(sim::CommandType::PlaceOriginalHempFarm,{14,13});
        field={sim::CommandType::PlaceOriginalHempField,{16,13}};field.building_id=hemp;f::buy(world,field);
        f::buy(world,{sim::CommandType::PlaceOriginalInfluenceSource,{0,0}});neutral=*world.building_owner_at({0,0});
        check(world.total_population()==0 && world.original_production_state()->water.providers.size()==1 &&
              world.couriers().size()==10 && world.original_production_state()->water.trips.empty() &&
              world.original_production_state()->industry.aux_trips.empty(),
              "Paid source purchase granted free residents, extra actors or water visits");
        check(std::count_if(world.couriers().begin(),world.couriers().end(),[](const auto& actor){return actor.role==sim::CourierRole::OriginalAuxSupplier;})==1 &&
              std::count_if(world.couriers().begin(),world.couriers().end(),[&](const auto& actor){return actor.owner==warehouse;})==2,
              "The one paid Warehouse did not own exactly one Goods and one material supplier");
        int goods_suppliers=0,material_suppliers=0;
        for (const auto& actor:world.couriers()) if (actor.owner==warehouse) {
            if (actor.role==sim::CourierRole::MarketPotteryInbound && actor.good==sim::Good::Pottery) ++goods_suppliers;
            else if (actor.role==sim::CourierRole::OriginalAuxSupplier && actor.good==sim::Good::Goods) ++material_suppliers;
            else check(false,"Warehouse purchase attached an unrelated actor or incorrect relation label");
            check(actor.enabled && !actor.cargo && !actor.reserved && actor.phase==sim::CourierPhase::IdleAtWorkshop && actor.target==warehouse,
                  "Warehouse purchase granted a loaded, moving or foreign-target supplier");
        }
        check(goods_suppliers==1 && material_suppliers==1,"Warehouse supply roles were duplicated or missing");
        check(world.construction_spent_total()==1433 && world.treasury()==10567,
              "Housing scene failed regular costs including distinct60 influence and25 water sources");
        for (const auto& h:world.original_economy_state()->houses)
            check(h.house.type==orig::HouseType::Vacant && !h.house.water && !water(world,h.building_id).normal_timer,
                  "Purchase granted housing stage or water");
    }
    save::SaveDocument document(const sim::World& value) const {return save::make_document(root.data,f::map_relative,policy->building_mask(),value);}
    sim::World restore(const save::SaveDocument& d) const {return save::restore_original_production_development(d,root.data,policy->building_mask(),policy);}
    std::string roundtrip(const sim::World& value) const {
        const auto before=value.snapshot();const auto original=document(value);
        save::write_save(target,original,root.data,policy->building_mask());const auto bytes=f::read_text(target);
        const auto parsed=save::read_save(target);f::same_document(original,parsed);
        check(parsed.source_schema_version==26 && parsed.world.original_production->development_revision==27,
              "Housing save did not identify its explicit revision27");
        auto restored=restore(parsed);check(restored.snapshot()==before,"Housing F5/F9 changed full World state");
        save::write_save(target,document(restored),root.data,policy->building_mask());
        check(f::read_text(target)==bytes && value.snapshot()==before,"Housing canonical rewrite mutated bytes/live state");return bytes;
    }
    void replay(const sim::World& value) const {
        roundtrip(value);auto expected=sim::World::restore_original_production_development(value.snapshot(),policy,model);
        auto actual=restore(save::read_save(target));
        for (int i=0;i<20000;++i) {
            if (i==10000) {
                f::buy(expected,{sim::CommandType::PlaceRoad,{22,0}});f::buy(actual,{sim::CommandType::PlaceRoad,{22,0}});
            }
            expected.tick();actual.tick();
        }
        check(expected.snapshot()==actual.snapshot(),"Housing20k save continuation with real paid road diverged");
    }
};
struct States {sim::WorldSnapshot midwater,plain,ninth_failure,expired_water;};
States paid_lifecycle(Fixture& f) {
    f.roundtrip(f.world);until(f.world,[&]{return f.world.total_population()==42;});
    check(!house(f.world,f.home).water && house(f.world,f.home).type==orig::HouseType::Shelter,
          "Neutral60 influence source supplied water or caused a free upgrade");
    for (const auto id:{f.well,f.ancestor}) f::buy(f.world,sim::set_building_operation(id,true));
    until(f.world,[&] {
        for (const auto& c:f.world.couriers())
            if (c.role==sim::CourierRole::OriginalWater && c.phase==sim::CourierPhase::ToWarehouse && c.edge_progress>0) return true;
        return false;
    });
    const auto midwater=f.world.snapshot();f.roundtrip(f.world);
    check(!water(f.world,f.home).current_witness && !house(f.world,f.home).water,"Water dispatch supplied its future target");
    until(f.world,[&]{return house(f.world,f.home).type==orig::HouseType::Hut;});
    check(history(f.world,f.home).first_transitions[1] && !history(f.world,f.home).first_transitions[2],
          "Actual water-backed Hut lacks its permanent first upgrade proof");
    for (const auto id:f.sources) if (!f.world.building(id).operating_enabled)
        f::buy(f.world,sim::set_building_operation(id,true));
    until(f.world,[&]{return house(f.world,f.home).type==orig::HouseType::PlainCottage;});
    {
        const auto pristine=f.roundtrip(f.world);auto bad=f.document(f.world);
        auto& row=bad.world.original_production->housing.houses[0];
        check(row.first_transitions[2] && row.last_evaluation &&
              row.first_transitions[2]->evaluation.tick==row.last_evaluation->tick && row.first_transitions[2]->evaluation.input.occupants>1,
              "First Plain test did not capture one exact actual phase39 input");
        --row.first_transitions[2]->evaluation.input.occupants;
        refuses([&]{(void)f.restore(bad);});
        refuses([&]{save::write_save(f.target,bad,f.root.data,f.policy->building_mask());});
        auto altered=json::parse(pristine);
        altered["world"]["original_production"]["housing"]["houses"][0]["first_transitions"][2]["evaluation"]["input"]["occupants"]=
            row.first_transitions[2]->evaluation.input.occupants;
        const auto path=f.root.path/"contradictory-same-tick-housing.json";f::write_text(path,altered.dump());
        refuses([&]{(void)f.restore(save::read_save(path));});
        check(f::read_text(f.target)==pristine,"Contradictory same-tick proof replaced a valid actual upgrade save");
    }
    until(f.world,[&] {
        const auto& h=history(f.world,f.home);
        return house(f.world,f.home).type==orig::HouseType::PlainCottage && h.consumption_rows[2].consumed[0]>0 && h.consumption_rows[2].consumed[1]>0;
    });
    check(house(f.world,f.home).water && house(f.world,f.home).food_quality>=20 && house(f.world,f.home).ancestor>0 &&
          history(f.world,f.home).first_transitions[1] && history(f.world,f.home).first_transitions[2],
          "Paid full chain failed to prove its authored Need1/1 Plain upgrade");
    const auto plain=f.world.snapshot();f.roundtrip(f.world);
    // Removal is a normal paid-source lifecycle command. Every old origin
    // remains globally available for the already recorded first evaluations.
    sim::Command remove{sim::CommandType::DemolishBuilding};remove.building_id=f.neutral;f::buy(f.world,remove);
    check(f.world.original_production_state()->housing.neutral_source_origins.size()==1 &&
          f.world.original_influence_state()->removed_sources.size()==1,"Removed paid influence origin was discarded");
    f.roundtrip(f.world);
    for (const auto id:f.sources) f::buy(f.world,sim::set_building_operation(id,false));
    until(f.world,[&] {
        return std::all_of(f.world.couriers().begin(),f.world.couriers().end(),[](const auto& c){return c.phase==sim::CourierPhase::IdleAtWorkshop;});
    });
    until(f.world,[&]{return house(f.world,f.home).type==orig::HouseType::PlainCottage && house(f.world,f.home).devolve_counter==9;});
    const auto ninth=f.world.snapshot();const auto occupants=house(f.world,f.home).occupants;f.roundtrip(f.world);
    const auto next=sim::original_housing_evaluation_tick(f.world.ticks())+51;
    while (f.world.ticks()+1<next) f.world.tick();
    check(house(f.world,f.home).type==orig::HouseType::PlainCottage && house(f.world,f.home).devolve_counter==9,
          "Ninth failure downgraded before its next actual phase39");
    f.world.tick();
    check(house(f.world,f.home).type==orig::HouseType::Hut && !house(f.world,f.home).devolve_counter &&
          house(f.world,f.home).occupants==occupants,"Tenth actual failure did not devolve or invented eviction");
    f.roundtrip(f.world);
    until(f.world,[&]{return water(f.world,f.home).current_witness && !water(f.world,f.home).normal_timer && house(f.world,f.home).water;});
    const auto expired=f.world.snapshot();f.roundtrip(f.world);
    const auto evaluation=sim::original_water_evaluation_tick(f.world.ticks())+51;
    while (f.world.ticks()<evaluation) f.world.tick();
    check(!house(f.world,f.home).water,"Expired raw Water was not removed at its actual next phase28");
    until(f.world,[&]{return house(f.world,f.home).type==orig::HouseType::Shelter;});
    check(history(f.world,f.home).consumption_rows[2].consumed[0]>0 && history(f.world,f.home).consumption_rows[2].consumed[1]>0 &&
          house(f.world,f.home).occupants>=occupants,"Devolution lost historical Need1 consumption or admitted residents");
    f.roundtrip(f.world);
    auto replay=sim::World::restore_original_production_development(ninth,f.policy,f.model);f.replay(replay);
    return {midwater,plain,ninth,expired};
}
void hostile(Fixture& f,const States& states) {
    auto actual=sim::World::restore_original_production_development(states.plain,f.policy,f.model);
    const auto pristine=f.roundtrip(actual);const auto good=json::parse(pristine);const auto badpath=f.root.path/"bad-housing.json";
    const auto reject=[&](const json& bad) {
        f::write_text(badpath,bad.dump());
        try {refuses([&]{(void)f.restore(save::read_save(badpath));});}
        catch (const std::exception& error) {throw std::runtime_error(std::string(error.what())+": "+json::diff(good,bad).dump());}
        check(f::read_text(f.target)==pristine,"Rejected housing save replaced valid protected data");
    };
    const std::string base="/world/original_production/",h=base+"housing/houses/0/",w=base+"water/houses/0/";
    std::vector<std::string> objects;
    const std::function<void(const json&,std::string)> visit=[&](const json& value,std::string path) {
        if (value.is_object()) {
            objects.push_back(path);for (const auto& [key,child]:value.items()) visit(child,path+"/"+key);
        } else if (value.is_array()) for (std::size_t i=0;i<value.size();++i) visit(value[i],path+"/"+std::to_string(i));
    };
    visit(good.at(json::json_pointer(base+"water")),base+"water");
    visit(good.at(json::json_pointer(base+"housing")),base+"housing");
    for (const auto& path:objects) {
        auto bad=good;bad.at(json::json_pointer(path))["unsupported_housing_authority"]=true;reject(bad);
        for (const auto& [key,child]:good.at(json::json_pointer(path)).items()) {
            (void)child;bad=good;bad.at(json::json_pointer(path)).erase(key);reject(bad);
        }
    }
    for (const auto& [path,replacement]:std::vector<std::pair<std::string,json>>{
            {base+"development_revision",20},{base+"development_revision",21},{base+"development_revision",22},{base+"development_revision",23},{base+"development_revision",24},{base+"development_revision",25},{base+"development_revision",26},{base+"development_revision",28},
            {h+"first_admission_tick",nullptr},{h+"first_admission_tick",0},{h+"first_admission_tick",121},
            {h+"last_evaluation",nullptr},{h+"first_transitions/1",nullptr},{h+"first_transitions/2",nullptr},
            {h+"last_evaluation/tick",1},{h+"last_evaluation/input/type",17},
            {h+"last_evaluation/input/desirability",100},{h+"last_evaluation/input/water",false},
            {h+"first_transitions/2/evaluation/input/ancestor",0},{h+"first_transitions/2/evaluation/input/food_quality",0},
            {h+"first_transitions/2/evaluation/input/hemp",0},{h+"first_transitions/2/evaluation/input/ceramics",0},
            {h+"first_transitions/1/evaluation/water_evaluation_witness",nullptr},
            {h+"first_transitions/1/evaluation/input/occupants",8},
            {h+"first_transitions/2/evaluation/service_witnesses/2",nullptr},
            {h+"first_transitions/2/evaluation/food_quality_witness",nullptr},
            {w+"current_witness",nullptr},{w+"evaluation_witness",nullptr},{w+"normal_timer",97},{w+"special_timer",1},
            {base+"water/providers/0/building_id",static_cast<unsigned>(f.neutral)},
            {base+"housing/neutral_source_origins/0/origin",json::array({INT32_MAX,INT32_MIN})}}) {
        auto bad=good;bad.at(json::json_pointer(path))=replacement;if (bad!=good) reject(bad);
    }
    for (const auto& path:{h+"first_admission_tick",h+"last_evaluation/tick",h+"consumption_rows/2/events",w+"normal_timer"})
        for (const json& invalid:{json(-1),json(true),json(1.0),json("1"),json(UINT64_MAX)}) {
            auto bad=good;bad.at(json::json_pointer(path))=invalid;reject(bad);
        }
    for (const auto& path:{h+"consumption_rows",h+"last_evaluation/service_witnesses",h+"last_evaluation/food_received",
                          h+"last_evaluation/food_consumed",h+"last_evaluation/goods_consumed_before",h+"consumption_rows/2/consumed"}) {
        auto bad=good;auto& array=bad.at(json::json_pointer(path));array.push_back(array[0]);reject(bad);
        bad=good;bad.at(json::json_pointer(path)).erase(0);reject(bad);
    }
    for (const auto& [path,limit]:std::vector<std::pair<std::string,std::size_t>>{
            {base+"water/providers",2},{base+"water/trips",2},{base+"water/houses",20},
            {base+"housing/houses",20},{base+"housing/neutral_source_origins",37}}) {
        auto bad=good;auto& array=bad.at(json::json_pointer(path));check(!array.empty(),"Positive context missing bounded rows");
        while (array.size()<=limit) array.push_back(array[0]);
        reject(bad);
    }
    auto never=sim::World::restore_original_production_development(states.midwater,f.policy,f.model);
    while (never.ticks()<397) never.tick();
    check(!history(never,f.home).first_transitions[1] && history(never,f.home).consumption_rows[0].events>0,
          "No-upgrade malicious-row fixture must own a real Shelter consumption opportunity");
    auto fake=json::parse(f.roundtrip(never));
    auto impossible_row=fake;
    auto& wrong_rows=impossible_row["world"]["original_production"]["housing"]["houses"][0]["consumption_rows"];
    std::swap(wrong_rows[0],wrong_rows[2]);
    f::write_text(badpath,impossible_row.dump());refuses([&]{(void)f.restore(save::read_save(badpath));});
    auto premature_counter=fake;
    premature_counter["world"]["original_production"]["housing"]["houses"][0]["last_evaluation"]["input"]["devolve_counter"]=9;
    f::write_text(badpath,premature_counter.dump());refuses([&]{(void)f.restore(save::read_save(badpath));});
    fake["world"]["original_economy"]["houses"][0]["house"]["type"]=5;
    fake["world"]["original_economy"]["houses"][0]["house"]["devolve_counter"]=1;
    auto& proof=fake["world"]["original_production"]["housing"]["houses"][0]["last_evaluation"];
    proof["input"]["type"]=5;proof["input"]["devolve_counter"]=0;
    f::write_text(badpath,fake.dump());refuses([&]{(void)f.restore(save::read_save(badpath));});
    f.roundtrip(actual);
    auto raw_expired=sim::World::restore_original_production_development(states.expired_water,f.policy,f.model);
    auto lost=json::parse(f.roundtrip(raw_expired));lost["world"]["original_production"]["water"]["houses"][0]["evaluation_witness"]=nullptr;
    f::write_text(badpath,lost.dump());refuses([&]{(void)f.restore(save::read_save(badpath));});f.roundtrip(actual);
    const auto document=f.document(actual);
    for (int fault=0;fault<9;++fault) {
        auto bad=document;auto& state=*bad.world.original_production;
        if (fault==0) state.housing.houses[0].first_transitions[2].reset();
        if (fault==1) state.housing.houses[0].last_evaluation->tick=bad.world.ticks+1;
        if (fault==2) state.housing.houses[0].consumption_rows[2].consumed[0]=UINT64_MAX;
        if (fault==3) state.water.houses[0].special_timer=1;
        if (fault==4) state.water.houses[0].current_witness->arrival_tick=bad.world.ticks+1;
        if (fault==5) state.housing.neutral_source_origins[0].origin={INT32_MAX,INT32_MIN};
        if (fault==6) state.development_revision=20;
        if (fault==7) {++bad.world.construction_spent_total;--bad.world.treasury;}
        if (fault==8) state.housing.houses[0].first_transitions[1]->evaluation.input.occupants=8;
        refuses([&]{(void)f.restore(bad);});
        refuses([&]{save::write_save(f.target,bad,f.root.data,f.policy->building_mask());});
        check(f::read_text(f.target)==pristine,"Bad housing writer replaced protected canonical save");
    }
    for (const auto marker:{"\"housing\": {","\"water\": {"}) {
        auto duplicate=pristine;const auto at=duplicate.find(marker);check(at!=std::string::npos,"Active context absent");
        duplicate.insert(duplicate.find('{',at)+1,"\"houses\":[],");f::write_text(badpath,duplicate);
        refuses([&]{(void)save::read_save(badpath);});
    }
}
}
int main() {
    try {
        Fixture fixture;const auto states=paid_lifecycle(fixture);hostile(fixture,states);
        std::cout<<"Housing save PASS: paid Water/Food/Ancestor/Hemp/Ceramics, authoredNeed1 Plain, historical devolution/consumption,20k replay and "
                 <<rejected<<" hostile refusals\n";return 0;
    } catch (const std::exception& error) {std::cerr<<"Original housing save FAIL: "<<error.what()<<'\n';return 1;}
}
