#include "OriginalProductionFixture.h"
#include "core/PerformanceDiagnostics.h"
#include "simulation/OriginalHousingDiagnosis.h"
#include "persistence/RecoveryStore.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <string_view>

namespace {
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
namespace save=openemperor::persistence;
namespace perf=openemperor::performance;
using Json=nlohmann::json;
using f::check;

// These are explicitly authored model/geography controls. All positive state
// still comes from normal paid commands and real ticks. Passing this boundary
// test is not a paid Xia Common10/Elite17 progression or retention result.
struct Fixture {
    f::Root root;
    std::shared_ptr<const sim::MapPermissions> policy=f::permissions();
    std::shared_ptr<const orig::OriginalRulesModel> model;
    std::string model_bytes;
    Fixture() {
        model_bytes=oe_production_fixture::authored_model_text(7);
        f::write_text(root.data/"Model/EmperorBuildingModels.txt",model_bytes);
        model=save::prepare_original_production_rules_model(root.data,orig::Difficulty::Normal);
    }
    sim::World world() const {return sim::World::create_original_sustainable_economy(policy,model);}
    save::SaveDocument document(const sim::World& w) const {
        return save::make_document(root.data,f::map_relative,policy->building_mask(),w);
    }
    sim::World restore(const save::SaveDocument& d) const {
        return save::restore_original_sustainable_economy(d,root.data,policy->building_mask(),policy,model);
    }
    void unchanged_source() const {
        check(f::read_text(root.data/"Model/EmperorBuildingModels.txt")==model_bytes,
              "Rule12 authority test changed its prepared model source");
    }
};

const sim::OriginalHouseState& home(const sim::World& w,sim::BuildingId id) {
    const auto& rows=w.original_economy_state()->houses;
    const auto at=std::find_if(rows.begin(),rows.end(),[&](const auto& row){return row.building_id==id;});
    check(at!=rows.end(),"Paid House lost its sole original House authority");return *at;
}
const sim::OriginalHousingHouse& history(const sim::World& w,sim::BuildingId id) {
    const auto& rows=w.original_production_state()->housing.houses;
    const auto at=std::find_if(rows.begin(),rows.end(),[&](const auto& row){return row.building_id==id;});
    check(at!=rows.end(),"Paid House lost its bounded transition history");return *at;
}
sim::BuildingId buy(sim::World& w,sim::CommandType kind,sim::Cell cell) {
    const auto before=w.treasury();const auto spent=w.construction_spent_total();
    const auto price=w.construction_cost(kind);const auto valid=w.validate({kind,cell});
    check(valid.accepted && valid.changed,"Authored Rule12 authority layout has an illegal paid purchase");
    f::buy(w,{kind,cell});
    check(w.treasury()==before-price && w.construction_spent_total()==spent+static_cast<std::uint64_t>(price),
          "Rule12 authority purchase bypassed actual Funds");
    if(kind==sim::CommandType::PlaceRoad)return static_cast<sim::BuildingId>(0);
    const auto owner=w.building_owner_at(cell);check(owner.has_value(),"Paid body has no owner");return *owner;
}
void through(sim::World& w,std::uint64_t tick) {while(w.ticks()<tick)w.tick();}
void authority(const sim::World& w) {
    check(w.rule_version()==12 && w.original_sustainable_economy_enabled() &&
          w.navigation_valid() && w.population_valid() && w.production_balance_valid() &&
          w.city_economy_valid() && w.fire_state_valid(),"Rule12 paid authority invariant failed");
    for(const auto& balance:w.original_goods_balances())check(balance.conserved,"House authority test lost physical goods mass");
}
template<class Action> void refuses(Action action) {
    bool rejected=false;
    try {action();}catch(const std::exception&){rejected=true;}
    check(rejected,"Wrong-generation or forged Rule12 House authority was accepted");
}

void compare_matrix(const sim::World& w,sim::BuildingId id) {
    const auto d=w.original_housing_diagnosis(id);const auto actual=home(w,id).house;
    const auto& rules=w.original_rules_model()->house_rules();
    check(d.actual==actual && d.current_row==orig::house_row(actual.type) &&
          d.next_row==orig::next_row(actual.type) && d.elite_family==orig::is_elite(actual.type),
          "Rule12 diagnostic replaced actual type/family/selectors");
    auto live=actual;live.desirability=static_cast<std::int8_t>(d.cached_desirability);
    const auto direct=orig::evolve_step(rules,live);
    check(d.pure_result_type==direct.type && d.pure_result_counter==direct.devolve_counter,
          "Live Rule12 diagnosis differs from immutable HouseEvolutionRules");
    const auto matrix=w.original_housing_decision_matrix(id);
    for(std::size_t r=0;r<matrix.size();++r) {
        const auto& row=matrix[r];const auto model=static_cast<orig::ModelHouse>(r);
        auto input=live;input.type=static_cast<orig::HouseType>(r+3);
        const auto step=orig::evolve_step(rules,input);
        check(row.row==model && row.hypothetical_type==input.type &&
              row.same_family==(orig::is_elite(input.type)==d.elite_family) &&
              row.row_capacity==rules.row_capacity(model) &&
              row.low==rules.value(model,orig::HouseField::DesirabilityLow) &&
              row.high==rules.value(model,orig::HouseField::DesirabilityHigh) &&
              row.needs_met==orig::needs_met(rules,model,input) &&
              row.verdict==orig::evolution_verdict(rules,input) &&
              row.pure_result_type==step.type && row.pure_result_counter==step.devolve_counter,
              "Rule12 fifteen-row diagnostic differs from unchanged pure model rules");
        std::array<bool,24> seen{};bool all=true;
        for(const auto& requirement:row.requirements) {
            const auto field=static_cast<std::size_t>(requirement.field);
            check(field<seen.size() && !seen[field] && requirement.required==rules.value(model,requirement.field),
                  "Rule12 diagnostic duplicated a requirement or changed its prepared value");
            seen[field]=true;all&=requirement.met;
        }
        check(all==row.needs_met,"Rule12 diagnostic requirement tree contradicts the pure needs verdict");
    }
    check(!orig::evolve_target(orig::HouseType::LuxuriousApartment) &&
          !orig::evolve_target(orig::HouseType::EliteUnoccupied),
          "Common terminal or empty Elite acquired an ordinary evolution target");
}

void readonly_matrix_and_identity() {
    Fixture fixture;auto w=fixture.world();
    for(int x=0;x<10;++x)buy(w,sim::CommandType::PlaceRoad,{x,4});
    const auto elite=buy(w,sim::CommandType::PlaceOriginalElitePlot,{0,0});
    const auto common=buy(w,sim::CommandType::PlaceHousehold,{6,2});
    const auto initial=w.snapshot();
    check(w.total_population()==0 && home(w,elite).house.type==orig::HouseType::EliteUnoccupied &&
          home(w,common).house.type==orig::HouseType::Vacant &&
          w.treasury()==12000-static_cast<std::int64_t>(w.construction_spent_total()),
          "Authored paid Rule12 diagnostic scene granted residents/type/resources");
    perf::set_enabled(true);perf::reset();
    for(int i=0;i<30;++i){compare_matrix(w,common);compare_matrix(w,elite);}
    for(const auto counter:{perf::Counter::WorldCopies,perf::Counter::WorldRestores,perf::Counter::WorldExecutes,
        perf::Counter::SimulationTicks,perf::Counter::BfsCalls,perf::Counter::RouteRefreshes,perf::Counter::FileReads,
        perf::Counter::FileWrites,perf::Counter::AssetDecodes,perf::Counter::TextureUploads})
        check(perf::counter(counter)==0,"Read-only Rule12 matrix performed simulation/preparation/IO");
    perf::set_enabled(false);
    check(w.snapshot()==initial,"Rule12 hypothetical matrix mutated real paid authority");
    through(w,120);compare_matrix(w,common);compare_matrix(w,elite);authority(w);
    check(w.building(elite).kind==sim::Object::OriginalEliteHouse &&
          home(w,elite).house.type==orig::HouseType::ModestSiheyuan &&
          history(w,elite).first_admission_tick==120,
          "Actual Rule12 first admission did not materialize paid11 into13");
    fixture.unchanged_source();
}

sim::World disk_roundtrip(const Fixture& fixture,const sim::World& w,const f::fs::path& path) {
    const auto before=w.snapshot();const auto original=fixture.document(w);
    save::write_save(path,original,fixture.root.data,fixture.policy->building_mask());
    const auto bytes=f::read_text(path);const auto parsed=save::read_save(path);
    f::same_document(original,parsed);
    check(parsed.source_schema_version==28 && parsed.world.rule_version==12 &&
          parsed.world.original_production->development_revision==27 &&
          parsed.map_permissions_policy_version==3,"Rule12 House disk boundary changed its exact generation");
    auto copy=fixture.restore(parsed);check(copy.snapshot()==before && w.snapshot()==before,
          "Rule12 House complete authority changed on save/restore");
    save::write_save(path,fixture.document(copy),fixture.root.data,fixture.policy->building_mask());
    check(f::read_text(path)==bytes,"Rule12 House canonical SaveDocument bytes changed");
    return copy;
}

void blocked_materialization_and_schema28() {
    Fixture fixture;auto w=fixture.world();
    for(int x=0;x<10;++x)buy(w,sim::CommandType::PlaceRoad,{x,4});
    const auto elite=buy(w,sim::CommandType::PlaceOriginalElitePlot,{0,0});
    const auto common=buy(w,sim::CommandType::PlaceHousehold,{6,2});
    buy(w,sim::CommandType::PlaceRoad,{2,2});
    const auto target=fixture.root.path/"stage-a-schema28.json";
    const auto pending=w.original_elite_materialization_status(elite);
    check(pending.pending && !pending.ready && pending.diagnostic.blocker==sim::BuildBlocker::SandboxOccupied &&
          pending.diagnostic.cell==sim::Cell{2,2},"Rule12 prospective4x4 ignored a genuinely paid road");
    through(w,119);auto copy=disk_roundtrip(fixture,w,target);
    const auto empty=home(w,elite);const auto proof=history(w,elite);
    w.tick();copy.tick();
    check(w.snapshot()==copy.snapshot() && home(w,elite)==empty && history(w,elite)==proof,
          "Blocked Rule12 admission or restore published a partial type/resident/history");
    const auto funds=w.treasury();const auto spent=w.construction_spent_total();
    f::buy(w,{sim::CommandType::RemoveRoad,{2,2}});
    check(w.treasury()==funds && w.construction_spent_total()==spent,
          "Legal removal of a paid blocking road invented a refund");
    through(w,159);copy=disk_roundtrip(fixture,w,target);
    const auto before=w.snapshot();w.tick();copy.tick();
    check(w.snapshot()==copy.snapshot() && w.building(elite).kind==sim::Object::OriginalEliteHouse &&
          home(w,elite).house.type==orig::HouseType::ModestSiheyuan && home(w,elite).house.occupants==2 &&
          history(w,elite).first_admission_tick==160 && w.road_revision()==before.road_revision+1 &&
          w.command_sequence()==before.command_sequence && w.treasury()==before.treasury &&
          w.construction_spent_total()==before.construction_spent_total &&
          w.snapshot().next_building_id==before.next_building_id,
          "Actual Rule12 repaired11→13 admission changed identity, commands or finance");
    const auto& admitted=history(w,elite).first_transitions[10];
    check(admitted && admitted->kind==sim::OriginalHousingTransitionKind::Admission &&
          admitted->evaluation.input.type==orig::HouseType::EliteUnoccupied &&
          !admitted->evaluation.input.occupants && admitted->admitted==2,
          "Rule12 admission lacks the actual empty-Elite pre-event witness");
    for(int y=0;y<4;++y)for(int x=0;x<4;++x)
        check(w.building_owner_at({x,y})==elite,"Rule12 materialization omitted one of16 owner claims");
    check(sim::building_footprint(w.profile(),w.rule_version(),w.building(common).kind)==sim::BuildingFootprint{2,2},
          "Elite materialization enlarged the unrelated Common family");
    copy=disk_roundtrip(fixture,w,target);
    for(int i=0;i<101;++i){w.tick();copy.tick();check(w.snapshot()==copy.snapshot(),"Rule12 post-admission disk continuation diverged");}
    authority(w);fixture.unchanged_source();

    const auto good=disk_roundtrip(fixture,w,target);const auto protected_bytes=f::read_text(target);
    const auto document=save::read_save(target);const auto live=w.snapshot();
    refuses([&]{(void)save::restore_original_fire_protection(document,fixture.root.data,fixture.policy->building_mask(),fixture.policy,fixture.model);});
    refuses([&]{(void)save::restore_original_production_development(document,fixture.root.data,fixture.policy->building_mask(),fixture.policy,fixture.model);});
    const auto reject=[&](auto edit) {
        auto raw=Json::parse(protected_bytes);edit(raw);const auto bad=fixture.root.path/"stage-a-hostile.json";
        f::write_text(bad,raw.dump());refuses([&]{(void)fixture.restore(save::read_save(bad));});
        check(f::read_text(target)==protected_bytes && w.snapshot()==live,
              "Rejected Rule12 House save changed the protected city/document");
    };
    reject([](auto& j){j["schema_version"]=27;});
    reject([](auto& j){j["rules"]["version"]=11;});
    reject([](auto& j){j["world"]["original_production"]["development_revision"]=28;});
    reject([](auto& j){j["world"]["buildings"][0]["kind"]=static_cast<unsigned>(sim::Object::Household);});
    reject([](auto& j){j["world"]["buildings"][0]["kind"]=static_cast<unsigned>(sim::Object::OriginalElitePlot);});
    reject([](auto& j){j["world"]["buildings"][1]["kind"]=static_cast<unsigned>(sim::Object::OriginalEliteHouse);});
    reject([](auto& j){j["world"]["original_economy"]["houses"][0]["house"]["type"]=17;});
    reject([](auto& j){j["world"]["original_production"]["housing"]["houses"][0]["first_transitions"][10]=nullptr;});
    reject([](auto& j){j["world"]["original_production"]["housing"]["houses"][0]["first_admission_tick"]=120;});
    reject([](auto& j){j["world"]["buildings"][0]["cell"]=Json::array({22,0});});
    reject([](auto& j){j["world"]["buildings"][1]["cell"]=Json::array({2,2});});
    reject([](auto& j){j["world"]["original_production"]["housing"]["houses"][0]["invented_high_tier_gate"]=true;});
    check(good.snapshot()==live,"Hostile negative controls changed the genuine restored control");
    fixture.unchanged_source();
}
}

int main(int argc,char** argv) {
    try {
        check(argc==2 && std::string_view(argv[1])=="--stage-a",
              "Usage: high-tier-progression-authority-tests --stage-a");
        readonly_matrix_and_identity();blocked_materialization_and_schema28();
        std::cout<<"PASS OE38 StageA Rule12 diagnostic/paid admission/schema28 boundaries; "
                    "authored controls only, no Common10/Elite17 gameplay acceptance\n";
        return 0;
    }catch(const std::exception& error){
        perf::set_enabled(false);std::cerr<<"OE38 StageA authority FAIL: "<<error.what()<<'\n';return 1;
    }
}
