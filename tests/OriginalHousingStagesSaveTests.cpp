#include "OriginalProductionFixture.h"

#include <nlohmann/json.hpp>
#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>

namespace {
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
namespace save=openemperor::persistence;
using json=nlohmann::json;
using f::check;
std::size_t refusals=0;
template<class Action> void refuses(Action action) {
    try {action();} catch (const std::exception&) {++refusals;return;}
    throw std::runtime_error("Malformed fifteen-stage Housing authority was admitted");
}
template<class Predicate> void until(sim::World& world,Predicate ready,int limit=10000) {
    for (int i=0;i<limit && !ready();++i) world.tick();
    check(ready(),"Paid Elite admission/devolution did not reach its actual event");
}
struct Fixture {
    f::Root root;
    bool abandoned_hemp_need=false;
    std::shared_ptr<const sim::MapPermissions> policy=f::permissions();
    std::shared_ptr<const orig::OriginalRulesModel> model=[&] {
        // Authored fixture: Modest requires actual Hemp and Ceramics, neither
        // of which exists in this deliberately unserved paid scene.
        oe_production_fixture::write_model(root.data,7);
        if (abandoned_hemp_need) {
            auto text=f::read_text(root.data/"Model/EmperorBuildingModels.txt");
            oe_production_fixture::set_house_field(text,9,9,1);
            f::write_text(root.data/"Model/EmperorBuildingModels.txt",text);
        }
        return save::prepare_original_production_rules_model(root.data);
    }();
    sim::World world=sim::World::create_original_production_development(policy,model);
    f::fs::path target=root.path/"housing-stages-schema26.json";
    sim::BuildingId elite{},common{};
    explicit Fixture(bool hemp_need=false):abandoned_hemp_need(hemp_need) {
        for (int x=0;x<8;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,4}});
        f::buy(world,{sim::CommandType::PlaceOriginalElitePlot,{0,2}});
        elite=*world.building_owner_at({0,2});
        f::buy(world,{sim::CommandType::PlaceHousehold,{3,2}});
        common=*world.building_owner_at({3,2});
        check(world.construction_spent_total()==176 && world.treasury()==11824 &&
              world.total_population()==0 && world.couriers().empty(),
              "Separate Elite/Common purchase failed real costs or granted residents/actors");
        check(home(world).type==orig::HouseType::EliteUnoccupied && !home(world).occupants &&
              history(world).first_transitions.size()==15 && history(world).consumption_rows.size()==15,
              "Paid empty Elite plot lost its distinct family or fifteen bounded rows");
    }
    const orig::HouseState& home(const sim::World& value) const {
        for (const auto& h:value.original_economy_state()->houses)
            if (h.building_id==elite) return h.house;
        throw std::runtime_error("Paid Elite owner is absent");
    }
    const sim::OriginalHousingHouse& history(const sim::World& value) const {
        for (const auto& h:value.original_production_state()->housing.houses)
            if (h.building_id==elite) return h;
        throw std::runtime_error("Paid Elite history is absent");
    }
    save::SaveDocument document(const sim::World& value) const {
        return save::make_document(root.data,f::map_relative,policy->building_mask(),value);
    }
    sim::World restore(const save::SaveDocument& value) const {
        return save::restore_original_production_development(value,root.data,policy->building_mask(),policy);
    }
    std::string roundtrip(const sim::World& value) const {
        const auto before=value.snapshot();const auto original=document(value);
        save::write_save(target,original,root.data,policy->building_mask());
        const auto bytes=f::read_text(target);const auto parsed=save::read_save(target);
        f::same_document(original,parsed);
        check(parsed.source_schema_version==26 && parsed.world.original_production->development_revision==25,
              "Fifteen-stage save lost strict revision25 identity");
        const auto copy=restore(parsed);check(copy.snapshot()==before,"Elite F5/F9 changed complete World authority");
        save::write_save(target,document(copy),root.data,policy->building_mask());
        check(f::read_text(target)==bytes && value.snapshot()==before,
              "Elite canonical rewrite changed bytes or the live World");return bytes;
    }
};
void authored_abandoned_counter() {
    Fixture f(true);
    until(f.world,[&]{return f.home(f.world).type==orig::HouseType::EliteAbandoned;});
    until(f.world,[&]{return f.home(f.world).devolve_counter==1;});
    f.roundtrip(f.world);
    until(f.world,[&]{return f.home(f.world).devolve_counter==11;});
    f.roundtrip(f.world);
    const auto next=sim::original_housing_evaluation_tick(f.world.ticks())+51;
    while (f.world.ticks()<next) f.world.tick();
    check(f.home(f.world).type==orig::HouseType::EliteAbandoned && !f.home(f.world).devolve_counter,
          "Authored Need1 abandoned row did not reset its actual twelfth missing-input event");
    f.roundtrip(f.world);
}
void paid_lifecycle(Fixture& f) {
    f.roundtrip(f.world);
    while (f.world.ticks()<119) f.world.tick();
    check(f.home(f.world).type==orig::HouseType::EliteUnoccupied && !f.home(f.world).occupants,
          "Empty Elite materialized before its real authored admission event");
    f.roundtrip(f.world);f.world.tick();
    const auto& admitted=f.history(f.world).first_transitions[10];
    check(f.home(f.world).type==orig::HouseType::ModestSiheyuan && f.home(f.world).occupants==2 &&
          admitted && admitted->kind==sim::OriginalHousingTransitionKind::Admission &&
          admitted->evaluation.tick==120 && admitted->evaluation.input.type==orig::HouseType::EliteUnoccupied &&
          !admitted->evaluation.input.occupants && admitted->admitted==2 &&
          f.history(f.world).first_admission_tick==120,
          "Actual first Elite admission did not bind empty11 ->13 before adding two residents");
    f.roundtrip(f.world);
    until(f.world,[&]{return f.home(f.world).devolve_counter==49;});
    check(f.home(f.world).type==orig::HouseType::ModestSiheyuan && f.home(f.world).occupants==5,
          "Forty-nine missing-input evaluations devolved early or lost genuine residents");
    f.roundtrip(f.world);const auto occupants=f.home(f.world).occupants;
    const auto next=sim::original_housing_evaluation_tick(f.world.ticks())+51;
    while (f.world.ticks()+1<next) f.world.tick();
    check(f.home(f.world).type==orig::HouseType::ModestSiheyuan && f.home(f.world).devolve_counter==49,
          "Elite devolution occurred outside its actual phase39");
    f.world.tick();
    const auto& abandoned=f.history(f.world).first_transitions[9];
    check(f.home(f.world).type==orig::HouseType::EliteAbandoned && !f.home(f.world).devolve_counter &&
          f.home(f.world).occupants==occupants && abandoned &&
          abandoned->kind==sim::OriginalHousingTransitionKind::Evolution && !abandoned->admitted &&
          abandoned->evaluation.input.type==orig::HouseType::ModestSiheyuan &&
          abandoned->evaluation.input.devolve_counter==49,
          "Fiftieth actual missing-input event did not prove13 ->12 with retained population");
    f.roundtrip(f.world);
    auto expected=f.restore(f.document(f.world));auto actual=f.restore(save::read_save(f.target));
    for (int tick=0;tick<20000;++tick) {
        if (tick==10000) {
            f::buy(expected,{sim::CommandType::PlaceRoad,{7,5}});
            f::buy(actual,{sim::CommandType::PlaceRoad,{7,5}});
        }
        expected.tick();actual.tick();
    }
    check(expected.snapshot()==actual.snapshot(),"Actual Elite12 twenty-thousand-tick save continuation diverged");
    f.roundtrip(actual);
}
void late_admission_dispatch(Fixture& f) {
    // Existing genuine residents staff this newly paid provider before a
    // second Elite plot is purchased. Route preparation must not dispatch
    // to that empty plot in its eventual first-admission tick.
    f::buy(f.world,{sim::CommandType::PlaceHealthPost,{3,5}});
    const auto provider=*f.world.building_owner_at({3,5});
    until(f.world,[&]{return f.world.workers_assigned(provider)==7;});
    f::buy(f.world,{sim::CommandType::PlaceOriginalElitePlot,{6,2}});
    const auto late=*f.world.building_owner_at({6,2});
    const auto first=f.world.building(late).placed_tick+120;
    f.roundtrip(f.world);
    while (f.world.ticks()<first) f.world.tick();
    const auto& histories=f.world.original_production_state()->housing.houses;
    const auto row=std::find_if(histories.begin(),histories.end(),[&](const auto& h){return h.building_id==late;});
    check(row!=histories.end() && row->first_admission_tick==first,
          "Late Elite did not use its actual paid placement/admission clock");
    for (const auto& trip:f.world.original_production_state()->services.trips)
        check(trip.target!=late,"Already staffed provider dispatched in the first Elite admission tick");
    f.roundtrip(f.world);
    until(f.world,[&] {
        for (const auto& witness:f.world.original_production_state()->services.witnesses)
            if (witness.building_id==late && witness.role==orig::ServiceRole::Herbalist) return true;
        return false;
    });
    bool observed=false;
    for (const auto& trip:f.world.original_production_state()->services.trips) if (trip.target==late) {
        check(trip.dispatched_tick>first && trip.visit_applied && trip.arrival_tick && *trip.arrival_tick>trip.dispatched_tick,
              "Late Elite actual visit lost dispatch/admission/arrival ordering");
        observed=true;
    }
    check(observed,"Late Elite witness has no actual retained provider trip");
    f.roundtrip(f.world);
}
void hostile(Fixture& f) {
    const auto pristine=f.roundtrip(f.world);const auto good=json::parse(pristine);
    const auto badpath=f.root.path/"hostile-housing-stages.json";
    const std::string house="/world/original_production/housing/houses/0/";
    const auto reject=[&](const json& value) {
        f::write_text(badpath,value.dump());
        refuses([&]{(void)f.restore(save::read_save(badpath));});
        check(f::read_text(f.target)==pristine,"Hostile stage save replaced a protected valid file");
    };
    for (const auto& [path,value]:std::vector<std::pair<std::string,json>>{
            {"/world/original_production/development_revision",24},
            {"/world/original_production/development_revision",26},
            {"/world/buildings/0/kind",static_cast<unsigned>(sim::Object::Household)},
            {"/world/buildings/1/kind",static_cast<unsigned>(sim::Object::OriginalElitePlot)},
            {house+"first_transitions/10/kind",1},{house+"first_transitions/10/kind",2},
            {house+"first_transitions/10/admitted",0},{house+"first_transitions/10/admitted",1},
            {house+"first_transitions/10/admitted",3},{house+"first_transitions/10/admitted",-1},
            {house+"first_transitions/10/admitted",true},{house+"first_transitions/10/admitted",2.0},
            {house+"first_transitions/10/evaluation/input/type",2},
            {house+"first_transitions/10/evaluation/input/occupants",1},
            {house+"first_transitions/10/evaluation/input/hemp",1},
            {house+"first_transitions/10/evaluation/input/confucian",40},
            {house+"first_transitions/10/evaluation/tick",121},
            {house+"first_transitions/10",nullptr},{house+"first_transitions/9",nullptr},
            {house+"first_transitions/9/kind",0},{house+"first_transitions/9/admitted",1},
            {house+"first_transitions/9/evaluation/input/devolve_counter",48},
            {house+"first_transitions/9/evaluation/input/type",11},
            {house+"first_admission_tick",121},
            {"/world/original_economy/houses/0/house/type",17}}) {
        auto bad=good;bad.at(json::json_pointer(path))=value;reject(bad);
    }
    for (const char* key:{"first_transitions","consumption_rows"}) {
        auto bad=good;auto& array=bad.at(json::json_pointer(house+key));array.push_back(nullptr);reject(bad);
        bad=good;bad.at(json::json_pointer(house+key)).erase(14);reject(bad);
    }
    {auto bad=good;std::swap(bad.at(json::json_pointer(house+"first_transitions/9")),
        bad.at(json::json_pointer(house+"first_transitions/10")));reject(bad);}
    {auto bad=good;bad[json::json_pointer(house+"first_hut_evaluation")]=nullptr;reject(bad);}
    for (int fault=0;fault<7;++fault) {
        auto bad=f.document(f.world);auto& history=bad.world.original_production->housing.houses[0];
        if (fault==0) history.first_transitions[10].reset();
        if (fault==1) history.first_transitions[10]->admitted=1;
        if (fault==2) history.first_transitions[9]->evaluation.input.devolve_counter=48;
        if (fault==3) bad.world.buildings[0].kind=sim::Object::Household;
        if (fault==4) bad.world.original_economy->houses[0].house.type=orig::HouseType::HeavenlyCompound;
        if (fault==5) history.first_transitions[8]=history.first_transitions[10];
        if (fault==6) {
            check(history.last_evaluation && history.last_evaluation->input.type==orig::HouseType::EliteAbandoned,
                  "Counter forgery control requires an actual stable abandoned Elite input");
            history.last_evaluation->input.devolve_counter=1;
        }
        refuses([&]{(void)f.restore(bad);});
        refuses([&]{save::write_save(f.target,bad,f.root.data,f.policy->building_mask());});
        check(f::read_text(f.target)==pristine,"Malformed stage writer replaced a valid save");
    }
    f.roundtrip(f.world);
}
}
int main() {
    try {
        Fixture fixture;paid_lifecycle(fixture);late_admission_dispatch(fixture);hostile(fixture);
        authored_abandoned_counter();
        std::cout<<"Fifteen-stage Save PASS: real paid empty Elite11 ->13, exact50-event13 ->12, "
            <<"retained population, canonical15 histories,20k replay and "<<refusals<<" strict refusals\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr<<"Fifteen-stage Save FAIL: "<<error.what()<<'\n';return 1;
    }
}
