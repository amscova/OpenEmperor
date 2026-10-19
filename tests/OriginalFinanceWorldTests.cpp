#include "OriginalFinanceFixture.h"
#include "simulation/original/OriginalScheduler.h"

#include <array>
#include <climits>
#include <iostream>
#include <limits>
#include <string_view>

namespace {
namespace q=oe_finance_fixture;
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
using f::check;
template<class Action> void refuses(Action action) {
    bool refused=false;try {action();} catch (const std::exception&) {refused=true;}
    check(refused,"Unsupported finance input/counter was accepted");
}
template<class Action> void overflows(Action action) {
    bool refused=false;try {action();} catch (const std::overflow_error&) {refused=true;}
    check(refused,"Full-domain finance overflow was not refused");
}
void arithmetic_boundaries() {
    constexpr auto high=std::uint64_t{1}<<63;
    const auto a=sim::original_finance_book(INT64_MIN,0,0,UINT64_MAX,UINT64_MAX);
    check(a.funds==INT64_MIN && a.taxes==UINT64_MAX && a.maintenance==UINT64_MAX,
          "Full unsigned tax/bill cancellation lost the signed minimum");
    const auto b=sim::original_finance_book(INT64_MIN,0,0,high,0);
    check(b.funds==0 && b.taxes==high && !b.maintenance,"Unsigned income crossing INT64_MIN was narrowed");
    const auto c=sim::original_finance_book(INT64_MAX,0,0,0,UINT64_MAX);
    check(c.funds==INT64_MIN && c.maintenance==UINT64_MAX,"Full unsigned bill did not reach exact signed minimum");
    const auto d=sim::original_finance_book(-1,0,0,high,0);
    check(d.funds==INT64_MAX && d.taxes==high,"Unsigned income did not reach exact signed maximum");
    const auto e=sim::original_finance_book(INT64_MAX,UINT64_MAX,UINT64_MAX,0,0);
    check(e.funds==INT64_MAX && e.taxes==UINT64_MAX && e.maintenance==UINT64_MAX,
          "Zero booking altered terminal counters");
    const auto full=sim::original_finance_book(INT64_MIN,0,0,UINT64_MAX,0);
    check(full.funds==INT64_MAX && full.taxes==UINT64_MAX,"Full unsigned income lost the signed positive endpoint");
    const auto debt=sim::original_finance_book(0,0,0,0,high);
    check(debt.funds==INT64_MIN && debt.maintenance==high,"Unsigned bill crossing zero lost exact signed minimum");
    overflows([]{(void)sim::original_finance_book(INT64_MAX,0,0,1,0);});
    overflows([]{(void)sim::original_finance_book(INT64_MIN,0,0,0,1);});
    overflows([]{(void)sim::original_finance_book(0,0,0,0,(std::uint64_t{1}<<63)+1);});
    overflows([]{(void)sim::original_finance_book(0,UINT64_MAX,0,1,0);});
    overflows([]{(void)sim::original_finance_book(0,0,UINT64_MAX,0,1);});
    // Tax is booked first. A later bill cannot legitimize an intermediate
    // positive-funds overflow, even if the final mathematical net would fit.
    overflows([]{(void)sim::original_finance_book(INT64_MAX,0,0,1,1);});
    check(sim::original_finance_accounting_valid(12000,0,0,0) &&
          sim::original_finance_accounting_valid(-1,0,12000,1) &&
          sim::original_finance_accounting_valid(INT64_MIN,0,12000,high) &&
          sim::original_finance_accounting_valid(12000,UINT64_MAX,UINT64_MAX,0) &&
          sim::original_finance_accounting_valid(INT64_MAX,UINT64_MAX,12000,high) &&
          sim::original_finance_accounting_valid(INT64_MIN,UINT64_MAX,UINT64_MAX,high+12000),
          "Exact full-domain credits/debits were rejected");
    check(!sim::original_finance_accounting_valid(11999,0,0,0) &&
          !sim::original_finance_accounting_valid(-1,0,12000,0) &&
          !sim::original_finance_accounting_valid(INT64_MIN,0,12000,high-1) &&
          !sim::original_finance_accounting_valid(12000,UINT64_MAX,UINT64_MAX,1),
          "One-unit finance mismatch disappeared at a signed/unsigned boundary");
}
void five_paid_difficulties(const f::fs::path& original) {
    std::optional<std::int32_t> plain_rate,plain_capacity;
    for (unsigned selected=0;selected<5;++selected) {
        const auto difficulty=static_cast<orig::Difficulty>(selected);
        q::Fixture fixture(original,difficulty);auto world=fixture.world();const auto scene=q::build(world);
        check(world.original_rules_model()->difficulty()==difficulty &&
              world.original_economy_state()->model_source->difficulty==difficulty &&
              world.original_production_state()->development_revision==27,
              "Paid finance session lost its sole typed saved difficulty/revision");
        check(world.construction_spent_total()==scene.spent && world.treasury()==12000-static_cast<std::int64_t>(scene.spent),
              "Difficulty scenario bypassed the real prepared purchase bill");
        q::through(world,815);check(!world.taxes_collected_total(),"House tax was collected before authored816t boundary");
        q::tick(world);q::advance_to_tax_or_goal(world);
        bool paid_plain=false;
        for (unsigned tries=0;tries<20000 && !paid_plain;++tries) {
            for (const auto id:scene.commons) {
                const auto& audit=q::finance(world,id);
                if (!audit.last_assessment || audit.last_assessment->type<orig::HouseType::PlainCottage ||
                    !world.building(id).taxes_paid_total) continue;
                const auto& history=world.original_production_state()->housing.houses;
                const auto proof=std::find_if(history.begin(),history.end(),[&](const auto& row){return row.building_id==id;});
                check(proof!=history.end() && proof->first_admission_tick && proof->first_transitions[2],
                      "Paid Plain taxpayer lacks actual admission and original evolution history");
                const auto& input=proof->first_transitions[2]->evaluation;
                check(input.input.type==orig::HouseType::Hut && input.input.water && input.input.food_quality>=20 &&
                      input.input.ancestor>0 && input.water_evaluation_witness && input.food_quality_witness && input.service_witnesses[2],
                      "Original Plain taxpayer lacks genuine paid Water/Food/Ancestor arrival inputs");
                const auto rate=fixture.model->house_rules().value(orig::house_row(audit.last_assessment->type),orig::HouseField::TaxRate);
                check(rate>0 && audit.last_assessment->tick%816==0 && audit.last_assessment->occupants>0,
                      "Positive tax example did not originate in prepared model rate and actual occupied month");
                paid_plain=true;break;
            }
            if (!paid_plain) q::tick(world);
        }
        check(paid_plain,"Selected original difficulty never produced an actually paid Plain taxpayer");
        const auto goal=world.original_goal_status();
        check(goal.applicable && goal.required_inhabited_houses==6 && goal.required_common_houses==1 &&
              goal.required_population==50 && goal.positive_tax && world.total_population()>=50,
              "Current authored housing mission hid or replaced a required threshold");
        q::accounting(world);
        check(world.workforce_supply()==world.total_population() && world.workforce_used()<=world.workforce_supply() &&
              world.population_valid() && world.navigation_valid() && world.production_balance_valid(),
              "Taxing houses duplicated residents/workers, cargo or paths");
        const auto before=world.snapshot();
        for (unsigned query=0;query<100;++query) {
            (void)world.original_goal_status();for (const auto id:scene.commons) (void)world.original_house_tax_status(id);
        }
        check(world.snapshot()==before,"Finance/mission inspection booked money, latched a win or changed World");
        const auto restored=fixture.restore(world);check(restored.snapshot()==before,"Paid selected difficulty did not restore exact finance/provenance");
        const auto rate=fixture.model->house_rules().value(orig::ModelHouse::PlainCottage,orig::HouseField::TaxRate);
        const auto capacity=fixture.model->house_rules().row_capacity(orig::ModelHouse::PlainCottage);
        check((!plain_rate || *plain_rate==rate) && (!plain_capacity || *plain_capacity==capacity),
              "Zero TaxRate/Capacity difficulty modifiers acquired invented extra difficulty factors");
        plain_rate=rate;plain_capacity=capacity;
    }
}
void both_families_empty_zero_and_fire() {
    q::Fixture fixture({},orig::Difficulty::Normal,0);auto zero=fixture.world();const auto zero_scene=q::build(zero,true);
    f::buy(zero,{sim::CommandType::PlaceOriginalElitePlot,{10,14}});const auto empty=*zero.building_owner_at({10,14});
    f::buy(zero,{sim::CommandType::PlaceRoad,{12,16}});
    q::through(zero,1632);
    check(zero.taxes_collected_total()==0 && zero.maintenance_spent_total()>0 && !zero.original_goal_status().positive_tax &&
          !zero.original_goal_status().reached,"Zero-rate or empty houses granted taxes or mission success");
    const auto& unoccupied=q::finance(zero,empty);
    check(q::house(zero,empty).type==orig::HouseType::EliteUnoccupied && !unoccupied.last_assessment &&
          std::all_of(unoccupied.rows.begin(),unoccupied.rows.end(),[](const auto& row){return !row.events && !row.resident_units;}),
          "Unmaterialized empty Plot acquired tax events or resident units");
    check(zero_scene.elite && zero.building(*zero_scene.elite).kind==sim::Object::OriginalEliteHouse &&
          zero.original_house_tax_status(*zero_scene.elite).rate==0 &&
          zero.original_house_tax_status(*zero_scene.elite).amount_if_assessed_now==0 &&
          q::finance(zero,*zero_scene.elite).rows[10].events==2 &&
          q::finance(zero,*zero_scene.elite).last_assessment &&
          zero.building(*zero_scene.elite).taxes_paid_total==0,
          "Zero-rate materialized Elite invented income or retained2x2 geometry");

    f::Root root;auto text=q::authored_model(3);
    oe_production_fixture::set_house_field(text,10,18,7);oe_production_fixture::set_house_field(text,9,18,5);
    f::write_text(root.data/"Model/EmperorBuildingModels.txt",text);
    const auto model=q::original_model_real(root.data,orig::Difficulty::Normal);
    auto world=sim::World::create_original_production_development(f::permissions(),model);const auto scene=q::build(world,true);
    check(scene.elite.has_value(),"Both-family finance scene has no paid Elite plot");const auto elite=*scene.elite;
    q::through(world,1632);
    check(q::finance(world,elite).rows[10].events==2 && q::finance(world,elite).rows[10].resident_units==10 &&
          world.building(elite).taxes_paid_total==70,"Two actual Modest tax events did not preserve5-resident rate7 origin");
    q::through(world,2447);check(world.building_on_fire(elite),"Actual Elite natural fire did not cover the next tax boundary");
    const auto tax=world.taxes_collected_total();const auto proof=world.original_production_state()->finance;
    q::tick(world);
    check(world.taxes_collected_total()==tax && world.original_production_state()->finance==proof &&
          !world.original_goal_status().reached,"Burning houses paid taxes, changed proof or retained a hidden mission latch");
    q::through(world,3264);
    check(q::house(world,elite).type==orig::HouseType::EliteAbandoned &&
          world.building(elite).kind==sim::Object::OriginalEliteHouse && q::finance(world,elite).last_assessment &&
          q::finance(world,elite).last_assessment->tick==3264 && q::finance(world,elite).last_assessment->type==orig::HouseType::EliteAbandoned &&
          q::finance(world,elite).rows[9].events==1 && q::finance(world,elite).rows[9].resident_units==5 &&
          world.building(elite).taxes_paid_total==95,"Actual13→12 changed4x4/population or reinterpreted earlier rate7 taxes at rate5");
    const auto funds=world.treasury();const auto total=world.taxes_collected_total();
    for (unsigned i=0;i<100;++i) (void)world.original_goal_status();
    check(world.treasury()==funds && world.taxes_collected_total()==total,"Mission requery awarded an automatic bonus");
}
void latest_natural_fire_assessment_restore() {
    q::Fixture fixture;
    // Actual placement-relative natural incidents, never injected fire state.
    // Delayed paid construction also puts ignition and expiry exactly on a
    // monthly boundary to distinguish their opposite pre-tick semantics.
    for (const auto delay:std::array<std::uint64_t,3>{0,448,664}) {
        auto world=fixture.world();q::through(world,delay);const auto scene=q::build(world,true);
        check(scene.elite.has_value(),"Natural-fire finance restore scene lost its paid Elite");
        const auto elite=*scene.elite;
        const auto boundary=delay==664 ? std::uint64_t{3264}:std::uint64_t{2448};
        q::through(world,boundary);
        const auto& body=world.building(elite);const auto& actual=q::finance(world,elite);
        check(body.fire_until_tick==delay+2600 && actual.last_assessment,
              "Paid natural incident or preceding real tax assessment was not reached");
        const auto before=world.snapshot();
        check(sim::World::restore_original_production_development(before,fixture.policy,fixture.model).snapshot()==before,
              "Genuine tax/fire boundary snapshot was refused");
        if (delay==448) {
            check(world.building_on_fire(elite) && actual.last_assessment->tick==boundary &&
                  body.fire_until_tick-boundary==sim::Rules::fire_incident_ticks,
                  "Actual ignition-tick tax was skipped or lost its pre-tick assessment");
            continue;
        }
        check(actual.last_assessment->tick<boundary &&
              world.building_on_fire(elite)==(delay==0) && boundary<=body.fire_until_tick &&
              body.fire_until_tick-boundary<sim::Rules::fire_incident_ticks,
              "Burning/expiry boundary unexpectedly collected a tax assessment");
        auto forged=before;
        auto& records=forged.original_production->finance.houses;
        const auto found=std::find_if(records.begin(),records.end(),[&](const auto& row){return row.building_id==elite;});
        check(found!=records.end() && found->last_assessment,"Forged tax probe lost the actual finance row");
        found->last_assessment->tick=boundary;
        refuses([&]{(void)sim::World::restore_original_production_development(forged,fixture.policy,fixture.model);});
        check(world.snapshot()==before,"Rejected burning/expiry assessment changed the genuine paid World");
        if (delay==0) {
            q::through(world,2600);const auto expired=world.snapshot();
            check(!world.building_on_fire(elite) && world.building(elite).fire_until_tick==2600,
                  "Natural fire expiry did not retain its authoritative latest deadline");
            check(sim::World::restore_original_production_development(expired,fixture.policy,fixture.model).snapshot()==expired,
                  "Genuine expired-fire history failed complete restore");
            forged=expired;
            for (auto& row:forged.original_production->finance.houses) if (row.building_id==elite)
                row.last_assessment->tick=boundary;
            refuses([&]{(void)sim::World::restore_original_production_development(forged,fixture.policy,fixture.model);});
            check(world.snapshot()==expired,"Rejected expired-fire assessment changed genuine history");
        }
    }
}
void current_mission_is_reversible() {
    q::Fixture fixture;auto world=fixture.world();(void)q::build(world);q::advance_to_tax_or_goal(world);
    const auto success=world.snapshot();check(world.settlement_goal_reached(),"Public settlement goal omitted the actual Original mission success");
    for (unsigned i=0;i<100;++i) check(world.original_goal_status().reached,"Pure mission query discarded a stable actual success");
    check(world.snapshot()==success,"Mission query awarded a bonus or latched additional authority");
    const auto deadline=world.ticks()+6000;
    while (world.original_goal_status().reached && world.ticks()<deadline) q::tick(world);
    const auto loss=world.original_goal_status();
    check(!loss.reached && !world.settlement_goal_reached() &&
          (loss.inhabited_houses<6 || loss.common_houses<1 || loss.population<50 ||
           !loss.positive_tax || !loss.nonnegative_funds),
          "Current mission retained a hidden success latch after real fire, devolution or debt");
    const auto failed=world.snapshot();for (unsigned i=0;i<100;++i) (void)world.original_goal_status();
    check(world.snapshot()==failed,"Lost mission mutated finance or paused the live simulation");
    auto copy=fixture.restore(world);q::tick(world);q::tick(copy);
    check(world.snapshot()==copy.snapshot(),"Mission loss changed live continuation or introduced a hidden pause");
}
void installed_debt_and_paid_repair() {
    f::Root root;auto text=q::authored_model(0);
    q::set_building_field(text,orig::ModelBuilding::HempFarm,orig::BuildingField::Employees,32767);
    f::write_text(root.data/"Model/EmperorBuildingModels.txt",text);
    const auto model=q::original_model_real(root.data,orig::Difficulty::Normal);const auto policy=f::permissions();
    auto world=sim::World::create_original_production_development(policy,model);
    f::buy(world,{sim::CommandType::PlaceOriginalHempFarm,{0,5}});const auto farm=*world.building_owner_at({0,5});
    f::buy(world,{sim::CommandType::PlaceRoad,{0,4}});f::buy(world,sim::set_building_operation(farm,false));
    q::through(world,399);const auto before=world.snapshot();q::tick(world);
    check(world.workforce_supply()==0 && world.workforce_used()==0 && !world.building(farm).operating_enabled &&
          world.maintenance_spent_total()==65534 && world.treasury()==before.treasury-65534 && world.treasury()<0,
          "Installed paused/unstaffed model Employees escaped actual2-per-worker maintenance debt");
    const auto negative=world.snapshot();
    const sim::Command purchase{sim::CommandType::PlaceRoad,{1,4}};
    check(!world.validate(purchase).accepted && !world.execute(purchase).accepted && world.snapshot()==negative,
          "Debt allowed an unpaid purchase or changed command/finance authority on refusal");
    f::buy(world,{sim::CommandType::RemoveRoad,{0,4}});const auto removed=world.snapshot();
    check(!world.execute({sim::CommandType::PlaceRoad,{0,4}}).accepted && world.snapshot()==removed,
          "Negative funds silently paid for road repair");
    auto restored=sim::World::restore_original_production_development(world.snapshot(),policy,model);
    check(restored.snapshot()==world.snapshot(),"Actual negative Treasury did not survive strict complete restore");
    q::through(world,800);q::through(restored,800);check(world.snapshot()==restored.snapshot(),"Debt continuation diverged or waived installed upkeep");
}
void later_income_can_fund_lifetime_construction() {
    f::Root root;auto text=q::authored_model(100);
    oe_production_fixture::set_house_field(text,0,17,100);oe_production_fixture::set_house_field(text,0,1,100);
    q::set_building_field(text,orig::ModelBuilding::HempFarm,orig::BuildingField::Cost,7000);
    f::write_text(root.data/"Model/EmperorBuildingModels.txt",text);
    const auto model=q::original_model_real(root.data,orig::Difficulty::Normal);const auto policy=f::permissions();
    auto world=sim::World::create_original_production_development(policy,model);
    for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,4}});
    for (int x=0;x<24;x+=3) f::buy(world,{sim::CommandType::PlaceHousehold,{x,2}});
    q::through(world,816);check(world.taxes_collected_total()>14000,"Genuine residents/rate100 did not earn later construction funds");
    const auto funds=world.treasury();
    f::buy(world,{sim::CommandType::PlaceOriginalHempFarm,{0,6}});f::buy(world,{sim::CommandType::PlaceOriginalHempFarm,{6,6}});
    check(world.construction_spent_total()>12000 && world.treasury()==funds-14000 && world.treasury()>=0,
          "Real later tax income could not fund exact paid cumulative construction beyond starting12000");
    q::accounting(world);
    check(sim::World::restore_original_production_development(world.snapshot(),policy,model).snapshot()==world.snapshot(),
          "Legitimate tax-funded lifetime spending above12000 could not restore");
}
void synthetic_long_zero_rate_restore_boundary() {
    // This is an explicitly synthetic integer-boundary DTO, not a claim to
    // have played trillions of months. Its paid body, first actual admission
    // and saturated population originate in ordinary commands/ticks below.
    q::Fixture fixture({},orig::Difficulty::Normal,0);
    auto text=f::read_text(fixture.root.data/"Model/EmperorBuildingModels.txt");
    oe_production_fixture::set_house_field(text,0,17,100);
    oe_production_fixture::set_house_field(text,0,1,100);
    f::write_text(fixture.root.data/"Model/EmperorBuildingModels.txt",text);
    fixture.model=q::original_model_real(fixture.root.data,orig::Difficulty::Normal);
    auto world=fixture.world();f::buy(world,{sim::CommandType::PlaceRoad,{0,4}});
    f::buy(world,{sim::CommandType::PlaceHousehold,{0,2}});const auto id=*world.building_owner_at({0,2});
    q::through(world,6000);
    check(q::house(world,id).type==orig::HouseType::Shelter && q::house(world,id).occupants==100 &&
          world.taxes_collected_total()==0 && world.maintenance_spent_total()==0,
          "Synthetic boundary seed lacks genuinely paid rate0 capacity100 residents");
    auto boundary=world.snapshot();constexpr auto months=std::uint64_t{4'000'000'000'000'000};
    boundary.ticks=months*816;
    // Unprotected risk100*20 starts fire at2000. At the600t expiry the first
    // next20t risk step is already eligible: subsequent starts are2580 apart.
    // The816t tax input samples the PRE-tick fire, giving a215-month pattern.
    constexpr auto eligible=[](std::uint64_t month){return (month*816+579)%2580>=600;};
    std::uint64_t period=0,remainder=0;
    for (std::uint64_t month=1;month<=215;++month) {
        if (eligible(month)) {++period;if (month<=months%215) ++remainder;}
    }
    const auto events=(months/215)*period+remainder;
    auto last=months;while (!eligible(last%215 ? last%215:215)) --last;
    auto& record=boundary.original_production->finance.houses.front();
    record.rows={};record.rows[0]={events,events*100-88};
    // Actual early tax inputs are36 and76; every later eligible input has100.
    record.last_assessment=sim::OriginalTaxAssessment{last*816,orig::HouseType::Shelter,100};
    auto& housing=boundary.original_production->housing.houses.front();
    check(housing.first_admission_tick==120 && housing.last_evaluation.has_value(),
          "Synthetic boundary discarded its genuine initial occupied origin");
    housing.consumption_rows={};housing.consumption_rows[0].events=
        sim::original_housing_consumption_events(boundary.ticks)-sim::original_housing_consumption_events(119);
    housing.last_evaluation->tick=sim::original_housing_evaluation_tick(boundary.ticks);
    housing.last_evaluation->input=boundary.original_economy->houses.front().house;
    auto& body=boundary.buildings.front();
    const auto start=std::uint64_t{2000}+((boundary.ticks-2000)/2580)*2580;
    body.fire_until_tick=start+600;body.fire_protection_until_tick=0;
    body.fire_risk=boundary.ticks<body.fire_until_tick ? 0:
        static_cast<int>((boundary.ticks-body.fire_until_tick)/20+1);
    check(record.rows[0].resident_units>UINT64_MAX/100 && !body.taxes_paid_total &&
          sim::original_finance_accounting_valid(boundary.treasury,0,boundary.construction_spent_total,0),
          "Synthetic boundary failed to exceed only the conservative100x bound");
    sim::World::validate_original_production_snapshot(boundary);
    auto copy=sim::World::restore_original_production_development(boundary,fixture.policy,fixture.model);
    check(copy.snapshot()==boundary,"Representable zero-rate large resident history did not restore complete authority");
    const auto document=fixture.document(copy);const auto target=fixture.root.path/"synthetic-long-zero-rate.json";
    f::save::write_save(target,document,fixture.root.data,fixture.policy->building_mask());
    const auto parsed=f::save::read_save(target);f::same_document(document,parsed);
    check(fixture.restore(parsed).snapshot()==boundary,"Synthetic large zero-rate history failed strict file roundtrip");
    auto wrong=boundary;++wrong.buildings.front().taxes_paid_total;++wrong.taxes_collected_total;++wrong.treasury;
    sim::World::validate_original_production_snapshot(wrong);
    refuses([&]{(void)sim::World::restore_original_production_development(wrong,fixture.policy,fixture.model);});
    check(world.taxes_collected_total()==0 && world.ticks()==6000 && copy.snapshot()==boundary,
          "Rejected model-inconsistent actual payment changed its real seed or restored boundary");
}
void negative_models_and_counter_refusals() {
    q::Fixture fixture;auto world=fixture.world();(void)q::build(world);q::through(world,816);const auto snapshot=world.snapshot();
    const auto pristine=f::read_text(fixture.root.data/"Model/EmperorBuildingModels.txt");
    for (std::size_t row=0;row<orig::house_row_count;++row) {
        auto text=pristine;oe_production_fixture::set_house_field(text,row,18,-1);
        f::write_text(fixture.root.data/"Model/EmperorBuildingModels.txt",text);
        const auto malformed=q::original_model_real(fixture.root.data,orig::Difficulty::Normal);
        refuses([&]{(void)sim::World::create_original_production_development(fixture.policy,malformed);});
        refuses([&]{(void)sim::World::restore_original_production_development(snapshot,fixture.policy,malformed);});
        check(world.snapshot()==snapshot,"Negative tax model candidate changed actual paid World");
    }
    f::write_text(fixture.root.data/"Model/EmperorBuildingModels.txt",pristine);
    for (unsigned fault=0;fault<5;++fault) {
        auto bad=snapshot;
        if (fault==0) bad.ticks=UINT64_MAX;
        if (fault==1) bad.command_sequence=UINT64_MAX;
        if (fault==2) bad.taxes_collected_total=UINT64_MAX;
        if (fault==3) bad.maintenance_spent_total=UINT64_MAX;
        if (fault==4) bad.original_production->finance.houses.front().rows[0].resident_units=UINT64_MAX;
        refuses([&]{(void)sim::World::restore_original_production_development(bad,fixture.policy,fixture.model);});
        check(world.snapshot()==snapshot,"Counter/finance refusal changed real World authority");
    }
}
}
int main(int argc,char** argv) {
    try {
        f::fs::path original;
        if (argc==3 && std::string_view(argv[1])=="--real") original=f::fs::canonical(argv[2]);
        else check(argc==1,"Usage: finance-world [--real DATA]");
        arithmetic_boundaries();five_paid_difficulties(original);both_families_empty_zero_and_fire();latest_natural_fire_assessment_restore();current_mission_is_reversible();
        installed_debt_and_paid_repair();later_income_can_fund_lifetime_construction();
        synthetic_long_zero_rate_restore_boundary();negative_models_and_counter_refusals();
        std::cout<<"Original finance World PASS: five genuinely paid difficulties, Plain tax/mission, both families, zero/empty/fire, "
                    "exact installed upkeep/debt, paid repair refusal, earned construction, full integer arithmetic/synthetic restore boundaries and negative models\n";
        return 0;
    } catch (const std::exception& error) {std::cerr<<"Original finance World FAIL: "<<error.what()<<'\n';return 1;}
}
