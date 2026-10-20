#include "OriginalFinanceFixture.h"

#include <nlohmann/json.hpp>
#include <functional>
#include <iostream>
#include <string_view>

namespace {
namespace q=oe_finance_fixture;
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
namespace save=openemperor::persistence;
using json=nlohmann::json;
using f::check;
std::size_t refusals=0,roundtrips=0;
template<class Action> void refuses(Action action) {
    try {action();} catch (const std::exception&) {++refusals;return;}
    throw std::runtime_error("Forged finance provenance, month, residents or payment was accepted");
}
struct Fixture:q::Fixture {
    using q::Fixture::Fixture;
    f::fs::path target=root.path/"finance-good.json";
    sim::World roundtrip(const sim::World& world) {
        const auto before=world.snapshot();const auto original=document(world);
        save::write_save(target,original,root.data,policy->building_mask());
        const auto bytes=f::read_text(target);const auto parsed=save::read_save(target);
        f::same_document(original,parsed);
        check(parsed.source_schema_version==26 && parsed.world.original_production->development_revision==27,
              "Finance canonical save lost Rule10/schema26/revision27");
        auto copy=restore(parsed);
        check(copy.snapshot()==before && world.snapshot()==before,"Finance F5/F9 changed complete authority");
        check(copy.original_production_state()->finance==world.original_production_state()->finance &&
              copy.original_goal_status().reached==world.original_goal_status().reached,
              "Finance F5/F9 changed historical monthly inputs or the current mission");
        save::write_save(target,document(copy),root.data,policy->building_mask());
        check(bytes==f::read_text(target),"Finance canonical rewrite changed complete bytes");
        ++roundtrips;return copy;
    }
};
void five_paid_difficulty_roundtrips(const f::fs::path& original) {
    for (unsigned selected=0;selected<5;++selected) {
        Fixture fixture(original,static_cast<orig::Difficulty>(selected));auto world=fixture.world();
        const auto scene=q::build(world);
        (void)fixture.roundtrip(world);q::through(world,815);auto before=fixture.roundtrip(world);
        q::tick(world);q::tick(before);check(before.snapshot()==world.snapshot(),"Pre-month F9 changed the next actual tax tick");
        (void)fixture.roundtrip(world);q::advance_to_tax_or_goal(world);
        bool paid_plain=false;
        for (unsigned ticks=0;ticks<20000 && !paid_plain;++ticks) {
            for (const auto id:scene.commons) {
                const auto& input=q::finance(world,id).last_assessment;
                paid_plain=paid_plain || (input && input->type>=orig::HouseType::PlainCottage && world.building(id).taxes_paid_total>0);
            }
            if (!paid_plain) q::tick(world);
        }
        check(paid_plain && world.original_economy_state()->model_source->difficulty==static_cast<orig::Difficulty>(selected),
              "Selected difficulty save never contained a genuinely paid model-bound Plain taxpayer");
        auto copy=fixture.roundtrip(world);
        q::through(world,world.ticks()+83);q::through(copy,copy.ticks()+83);
        check(copy.snapshot()==world.snapshot(),"Mid-cycle restored difficulty changed actual recurring finance");
        (void)fixture.roundtrip(copy);
        const auto document=fixture.document(copy);
        for (unsigned other=0;other<5;++other) if (other!=selected) {
            const auto wrong=q::original_model_real(fixture.root.data,static_cast<orig::Difficulty>(other));
            refuses([&]{(void)save::restore_original_production_development(document,fixture.root.data,
                fixture.policy->building_mask(),fixture.policy,wrong);});
        }
    }
}
void lifecycle_and_twenty_thousand_ticks() {
    Fixture fixture;auto world=fixture.world();const auto scene=q::build(world,true);
    for (const auto checkpoint:std::array<std::uint64_t,13>{0,119,120,399,400,815,816,1201,1631,1632,2447,2448,3264}) {
        q::through(world,checkpoint);auto copy=fixture.roundtrip(world);
        q::tick(copy);auto expected=fixture.restore(world);q::tick(expected);
        check(copy.snapshot()==expected.snapshot(),"Tax/upkeep/admission/fire F9 boundary changed next-tick authority");
    }
    check(scene.elite && world.building(*scene.elite).kind==sim::Object::OriginalEliteHouse &&
          q::finance(world,*scene.elite).last_assessment &&
          q::finance(world,*scene.elite).rows[10].events>0 && q::finance(world,*scene.elite).rows[9].events>0,
          "Lifecycle save lacks genuine4x4 Elite income before and after retained-population abandonment");
    auto copy=fixture.roundtrip(world);
    for (unsigned tick=0;tick<20000;++tick) {
        if (tick==10000) {
            check(world.treasury()>=2,"Paid finance long-play could not fund its ordinary2Funds road command");
            const auto funds=world.treasury();const auto spent=world.construction_spent_total();
            f::buy(world,{sim::CommandType::PlaceRoad,{21,13}});f::buy(copy,{sim::CommandType::PlaceRoad,{21,13}});
            check(world.treasury()==funds-2 && world.construction_spent_total()==spent+2,
                  "Long-play road command bypassed exact ordinary purchase costs");
            check(copy.snapshot()==world.snapshot(),"Midpoint paid command changed restored finance authority");
            const auto a=fixture.document(world),b=fixture.document(copy);f::same_document(a,b);
            (void)fixture.roundtrip(copy);
        }
        q::tick(world);q::tick(copy);
        if (tick%816==0) check(copy.snapshot()==world.snapshot(),"Twenty-thousand-tick finance continuation diverged");
    }
    check(copy.snapshot()==world.snapshot(),"Complete20k finance save continuation differs");
    const auto a=fixture.document(world),b=fixture.document(copy);f::same_document(a,b);
    (void)fixture.roundtrip(copy);
}
void latest_natural_fire_assessment_files() {
    for (const auto delay:std::array<std::uint64_t,3>{0,448,664}) {
        Fixture fixture;auto world=fixture.world();q::through(world,delay);const auto scene=q::build(world,true);
        check(scene.elite.has_value(),"Natural-fire save scene lacks a genuinely paid Elite");
        const auto elite=*scene.elite;
        const auto boundary=delay==664 ? std::uint64_t{3264}:std::uint64_t{2448};
        q::through(world,boundary);(void)fixture.roundtrip(world);
        check(world.building(elite).fire_until_tick==delay+2600 && q::finance(world,elite).last_assessment,
              "Natural-fire file probe lacks actual incident/tax history");
        if (delay==448) {
            check(world.building_on_fire(elite) && q::finance(world,elite).last_assessment->tick==boundary,
                  "Genuine ignition-tick assessment failed canonical save/restore");
            continue;
        }
        const auto reject=[&] {
            const auto before=world.snapshot();const auto pristine=f::read_text(fixture.target);
            auto document=fixture.document(world);auto& rows=document.world.original_production->finance.houses;
            const auto found=std::find_if(rows.begin(),rows.end(),[&](const auto& row){return row.building_id==elite;});
            check(found!=rows.end() && found->last_assessment && found->last_assessment->tick<boundary,
                  "Burning/expiry probe lost its genuine preceding assessment");
            const auto index=static_cast<std::size_t>(found-rows.begin());found->last_assessment->tick=boundary;
            refuses([&]{(void)fixture.restore(document);});
            refuses([&]{save::write_save(fixture.target,document,fixture.root.data,fixture.policy->building_mask());});
            auto raw=json::parse(pristine);
            raw[json::json_pointer("/world/original_production/finance/houses/"+std::to_string(index)+"/last_assessment/tick")]=boundary;
            const auto badpath=fixture.root.path/"finance-latest-fire-forged.json";f::write_text(badpath,raw.dump());
            refuses([&]{(void)fixture.restore(save::read_save(badpath));});
            check(world.snapshot()==before && f::read_text(fixture.target)==pristine,
                  "Rejected latest-fire tax changed complete good World or canonical save bytes");
        };
        reject();
        if (delay==0) {
            q::through(world,2600);check(!world.building_on_fire(elite),"Natural-fire file probe did not reach actual expiry");
            (void)fixture.roundtrip(world);reject();
        }
    }
}
void hostile_history_and_provenance() {
    Fixture fixture;auto world=fixture.world();const auto scene=q::build(world,true);q::through(world,1632);
    (void)fixture.roundtrip(world);const auto pristine=f::read_text(fixture.target);const auto good=json::parse(pristine);
    const auto live=world.snapshot();const auto badpath=fixture.root.path/"finance-hostile.json";
    const auto reject=[&](const json& value) {
        f::write_text(badpath,value.dump());
        refuses([&]{(void)fixture.restore(save::read_save(badpath));});
        check(f::read_text(fixture.target)==pristine && world.snapshot()==live,
              "Rejected finance JSON changed the complete good save or live World");
    };
    const std::string base="/world/original_production/finance";
    const std::string home=base+"/houses/0/";
    const auto& actual=q::finance(world,scene.commons.front());check(actual.last_assessment.has_value(),"Paid Common lacks actual tax input");
    const auto row=static_cast<unsigned>(actual.last_assessment->type)-3;
    const auto bucket=home+"rows/"+std::to_string(row)+"/";
    for (const auto& [path,value]:std::vector<std::pair<std::string,json>>{
            {base,nullptr},{base+"/houses",nullptr},{base+"/houses",json::object()},
            {home+"building_id",0},{home+"building_id",UINT32_MAX},{home+"building_id",true},{home+"building_id",1.0},
            {bucket+"events",0},{bucket+"events",UINT64_MAX},{bucket+"events",-1},{bucket+"events",true},{bucket+"events",2.0},
            {bucket+"resident_units",0},{bucket+"resident_units",UINT64_MAX},{bucket+"resident_units",-1},
            {bucket+"resident_units",true},{bucket+"resident_units",1.5},
            {home+"last_assessment",nullptr},{home+"last_assessment/tick",0},{home+"last_assessment/tick",815},
            {home+"last_assessment/tick",2448},{home+"last_assessment/tick",UINT64_MAX},
            {home+"last_assessment/tick",true},{home+"last_assessment/tick",1632.0},
            {home+"last_assessment/type",0},{home+"last_assessment/type",2},{home+"last_assessment/type",11},
            {home+"last_assessment/type",13},{home+"last_assessment/type",18},{home+"last_assessment/type",true},
            {home+"last_assessment/type",5.0},{home+"last_assessment/occupants",0},{home+"last_assessment/occupants",32768},
            {home+"last_assessment/occupants",-1},{home+"last_assessment/occupants",true},{home+"last_assessment/occupants",2.0},
            {"/world/taxes_collected_total",world.taxes_collected_total()+1},
            {"/world/maintenance_spent_total",world.maintenance_spent_total()+1},
            {"/world/treasury",world.treasury()+1},{"/world/buildings/0/taxes_paid_total",world.building(scene.commons.front()).taxes_paid_total+1},
            {"/world/construction_spent_total",world.construction_spent_total()+1},
            {"/world/original_production/development_revision",26},{"/world/original_production/development_revision",28},
            {"/world/original_economy/model_source/difficulty",5},
            {"/world/original_economy/model_source/difficulty",true},{"/world/original_economy/model_source/difficulty",2.0},
            {"/world/original_economy/model_source/sha256",std::string(64,'0')}}) {
        auto bad=good;bad.at(json::json_pointer(path))=value;reject(bad);
    }
    for (const auto& path:std::vector<std::string>{base,base+"/houses",home+"rows",home+"building_id",
            home+"last_assessment",bucket+"events",bucket+"resident_units",home+"last_assessment/type"}) {
        auto bad=good;const json::json_pointer pointer(path);bad.at(pointer.parent_pointer()).erase(pointer.back());reject(bad);
    }
    {auto bad=good;bad["world"]["original_production"]["finance"]["houses"].erase(0);reject(bad);}
    {auto bad=good;auto& rows=bad["world"]["original_production"]["finance"]["houses"][0]["rows"];rows.erase(0);reject(bad);}
    {auto bad=good;auto& rows=bad["world"]["original_production"]["finance"]["houses"][0]["rows"];rows.push_back(rows[0]);reject(bad);}
    {auto bad=good;auto& houses=bad["world"]["original_production"]["finance"]["houses"];houses[1]=houses[0];reject(bad);}
    {auto bad=good;auto& houses=bad["world"]["original_production"]["finance"]["houses"];std::swap(houses[0],houses[1]);reject(bad);}
    for (const auto& [path,key]:std::vector<std::pair<std::string,std::string>>{
            {base,"difficulty"},{base,"tax_rate"},{home.substr(0,home.size()-1),"current_population"},
            {home+"rows/0","amount"},{home+"last_assessment","phase"}}) {
        auto bad=good;bad.at(json::json_pointer(path))[key]=1;reject(bad);
    }
    const auto reject_document=[&](const save::SaveDocument& bad) {
        refuses([&]{(void)fixture.restore(bad);});
        refuses([&]{save::write_save(fixture.target,bad,fixture.root.data,fixture.policy->building_mask());});
        check(f::read_text(fixture.target)==pristine && world.snapshot()==live,
              "Rejected direct finance DTO replaced the protected save or live World");
    };
    const auto document=fixture.document(world);
    for (unsigned fault=0;fault<13;++fault) {
        auto bad=document;auto& finance=bad.world.original_production->finance;
        auto& house=finance.houses.front();
        if (fault==0) finance.houses.pop_back();
        if (fault==1) finance.houses.push_back(house);
        if (fault==2) house.building_id=scene.farm;
        if (fault==3) house.last_assessment.reset();
        if (fault==4) house.rows[row].events=UINT64_MAX;
        if (fault==5) house.rows[row].resident_units=UINT64_MAX;
        if (fault==6) house.last_assessment->occupants=1;
        if (fault==7) {house.rows[row]={};house.last_assessment=sim::OriginalTaxAssessment{1632,orig::HouseType::LuxuriousApartment,2};}
        if (fault==8) bad.world.original_production->housing.houses.front().first_transitions[row].reset();
        if (fault==9) {++bad.world.maintenance_spent_total;--bad.world.treasury;}
        if (fault==10) {++bad.world.buildings.front().taxes_paid_total;++bad.world.taxes_collected_total;++bad.world.treasury;}
        if (fault==11) ++bad.world.original_economy->model_source->byte_count;
        if (fault==12) bad.world.original_production->development_revision=26;
        reject_document(bad);
    }
    // Even a consistently adjusted treasury/global/body payment cannot turn
    // one claimed resident into a real pre-tick occupied type/month.
    {
        auto bad=document;auto& house=bad.world.original_production->finance.houses.front();
        const auto before=bad.world.buildings.front().taxes_paid_total;
        std::uint64_t payment=0;
        for (std::size_t r=0;r<house.rows.size();++r) if (house.rows[r].events) {
            house.rows[r].resident_units=house.rows[r].events;
            const auto rate=fixture.model->house_rules().value(static_cast<orig::ModelHouse>(r),orig::HouseField::TaxRate);
            payment+=house.rows[r].resident_units*static_cast<std::uint64_t>(rate);
        }
        bad.world.buildings.front().taxes_paid_total=payment;
        bad.world.taxes_collected_total=bad.world.taxes_collected_total-before+payment;
        bad.world.treasury-=static_cast<std::int64_t>(before-payment);
        reject_document(bad);
    }
    const auto modelpath=fixture.root.data/"Model/EmperorBuildingModels.txt";
    const auto modelbytes=f::read_text(modelpath);f::write_text(modelpath,modelbytes+"\n");
    refuses([&]{(void)fixture.restore(document);});
    refuses([&]{save::write_save(fixture.target,document,fixture.root.data,fixture.policy->building_mask());});
    check(f::read_text(fixture.target)==pristine && world.snapshot()==live,"Changed model provenance overwrote valid finance");
    f::write_text(modelpath,modelbytes);
    (void)fixture.roundtrip(world);
}
void genuine_negative_funds() {
    Fixture fixture({},orig::Difficulty::Normal,0);
    auto text=f::read_text(fixture.root.data/"Model/EmperorBuildingModels.txt");
    q::set_building_field(text,orig::ModelBuilding::HempFarm,orig::BuildingField::Employees,32767);
    f::write_text(fixture.root.data/"Model/EmperorBuildingModels.txt",text);
    fixture.model=q::original_model_real(fixture.root.data,orig::Difficulty::Normal);auto world=fixture.world();
    f::buy(world,{sim::CommandType::PlaceOriginalHempFarm,{0,5}});const auto farm=*world.building_owner_at({0,5});
    f::buy(world,sim::set_building_operation(farm,false));q::through(world,400);
    check(world.treasury()<0 && world.maintenance_spent_total()==65534 && !world.taxes_collected_total(),
          "Negative-funds save example was not real installed paid maintenance");
    auto copy=fixture.roundtrip(world);const auto pristine=f::read_text(fixture.target);const auto live=world.snapshot();
    auto bad=json::parse(pristine);const auto badpath=fixture.root.path/"finance-debt-hostile.json";
    for (const json& value:{json(std::uint64_t{1}<<63),json(UINT64_MAX),json(true),json(-1.0)}) {
        bad["world"]["treasury"]=value;f::write_text(badpath,bad.dump());
        refuses([&]{(void)fixture.restore(save::read_save(badpath));});
    }
    bad=json::parse(pristine);bad["world"]["maintenance_spent_total"]=-1;f::write_text(badpath,bad.dump());
    refuses([&]{(void)fixture.restore(save::read_save(badpath));});
    const auto before=copy.snapshot();check(!copy.execute({sim::CommandType::PlaceRoad,{0,4}}).accepted && copy.snapshot()==before,
          "Debt F9 permitted an unpaid repair/new purchase");
    q::through(world,1201);q::through(copy,1201);
    check(world.snapshot()==copy.snapshot() && f::read_text(fixture.target)==pristine && live.treasury<0,
          "Debt continuation waived upkeep, changed the good save or diverged");
    (void)fixture.roundtrip(copy);
}
}
int main(int argc,char** argv) {
    try {
        f::fs::path original;
        if (argc==3 && std::string_view(argv[1])=="--real") original=f::fs::canonical(argv[2]);
        else check(argc==1,"Usage: finance-save [--real DATA]");
        five_paid_difficulty_roundtrips(original);lifecycle_and_twenty_thousand_ticks();latest_natural_fire_assessment_files();
        hostile_history_and_provenance();genuine_negative_funds();
        std::cout<<"Original finance Save PASS: five paid difficulties, exact canonical pre/post/month/fire/debt F5/F9, "
                    "both families, complete20k continuation, model-bound ledgers, "<<roundtrips<<" roundtrips and "<<refusals<<" hostile refusals\n";
        return 0;
    } catch (const std::exception& error) {std::cerr<<"Original finance Save FAIL: "<<error.what()<<'\n';return 1;}
}
