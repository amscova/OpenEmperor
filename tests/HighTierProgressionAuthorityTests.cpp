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
#ifdef OE_MARKET_LOGISTICS_AUTHORITY_TEST
constexpr unsigned authority_rule=13,authority_schema=29;
#else
constexpr unsigned authority_rule=12,authority_schema=28;
#endif

// These are explicitly authored model/geography controls. All positive state
// still comes from normal paid commands and real ticks. Passing this boundary
// test is not a paid Xia Common10/Elite17 progression or retention result.
struct Fixture {
    f::Root root;
    std::shared_ptr<const sim::MapPermissions> policy=f::permissions();
    std::shared_ptr<const orig::OriginalRulesModel> model;
    std::string model_bytes;
    explicit Fixture(std::string bytes=oe_production_fixture::authored_model_text(7)) {
        model_bytes=std::move(bytes);
        f::write_text(root.data/"Model/EmperorBuildingModels.txt",model_bytes);
        model=save::prepare_original_production_rules_model(root.data,orig::Difficulty::Normal);
    }
    sim::World world() const {
#ifdef OE_MARKET_LOGISTICS_AUTHORITY_TEST
        return sim::World::create_original_market_logistics(policy,model);
#else
        return sim::World::create_original_sustainable_economy(policy,model);
#endif
    }
    save::SaveDocument document(const sim::World& w) const {
        return save::make_document(root.data,f::map_relative,policy->building_mask(),w);
    }
    sim::World restore(const save::SaveDocument& d) const {
#ifdef OE_MARKET_LOGISTICS_AUTHORITY_TEST
        return save::restore_original_market_logistics(d,root.data,policy->building_mask(),policy,model);
#else
        return save::restore_original_sustainable_economy(d,root.data,policy->building_mask(),policy,model);
#endif
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
    check(w.rule_version()==authority_rule && w.original_sustainable_economy_enabled() &&
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
    check(parsed.source_schema_version==authority_schema && parsed.world.rule_version==authority_rule &&
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

// StageB deliberately uses small authored input models/geography to isolate
// negative authority controls. It never claims real-model Common10/Elite17.
std::string negative_model(int capacity=7) {
    auto text=oe_production_fixture::authored_model_text(capacity);
    for(std::size_t row=0;row<orig::house_row_count;++row)
        oe_production_fixture::set_house_field(text,row,18,3);
    return text;
}
template<class Predicate> void until(sim::World& w,Predicate done,const char* reason,int limit=6000) {
    for(int i=0;i<limit && !done();++i)w.tick();
    check(done(),reason);authority(w);
}
const sim::OriginalIndustrySource& industry(const sim::World& w,sim::BuildingId id) {
    for(const auto& row:w.original_production_state()->industry.sources)if(row.building_id==id)return row;
    throw std::runtime_error("Rule12 negative control lost its paid industry source");
}
const sim::OriginalAuxStock& auxiliary(const sim::World& w,sim::BuildingId id) {
    for(const auto& row:w.original_production_state()->industry.auxiliary)if(row.building_id==id)return row;
    throw std::runtime_error("Rule12 negative control lost its sole auxiliary account");
}
sim::CourierId actor(const sim::World& w,sim::BuildingId id) {
    for(const auto& c:w.couriers())if(c.owner==id)return c.id;
    throw std::runtime_error("Rule12 negative control lost its actual actor");
}
sim::OriginalHousingRequirement need(const sim::World& w,sim::BuildingId id,std::size_t row,orig::HouseField field) {
    const auto matrix=w.original_housing_decision_matrix(id);
    for(const auto& value:matrix.at(row).requirements)if(value.field==field)return value;
    throw std::runtime_error("Rule12 diagnostic omitted its required field");
}
std::size_t json_owner(const Json& rows,sim::BuildingId id,std::string_view key="building_id") {
    for(std::size_t i=0;i<rows.size();++i)if(rows[i].at(std::string(key))==static_cast<unsigned>(id))return i;
    throw std::runtime_error("Rule12 hostile control lacks its actual owner");
}
void hostile_pointers(const Fixture& fixture,const sim::World& w,const char* label,
                      const std::vector<std::pair<std::string,Json>>& edits) {
    const auto target=fixture.root.path/(std::string(label)+"-good.json");
    (void)disk_roundtrip(fixture,w,target);
    const auto bytes=f::read_text(target);const auto live=w.snapshot();const auto good=Json::parse(bytes);
    for(const auto& [pointer,value]:edits) {
        auto raw=good;raw.at(Json::json_pointer(pointer))=value;
        const auto bad=fixture.root.path/(std::string(label)+"-hostile.json");f::write_text(bad,raw.dump());
        try {refuses([&]{(void)fixture.restore(save::read_save(bad));});}
        catch(const std::exception& error){throw std::runtime_error(std::string(label)+" "+pointer+": "+error.what());}
        check(w.snapshot()==live && f::read_text(target)==bytes,"Rejected StageB input changed protected live/disk authority");
    }
    fixture.unchanged_source();
}

void paid_single_goods_or(bool lacquer) {
    auto text=negative_model(37);
    oe_production_fixture::set_house_field(text,1,3,0);
    for(const unsigned field:{9U,10U,11U,12U,13U})oe_production_fixture::set_house_field(text,1,field,1);
    Fixture fixture(std::move(text));auto w=fixture.world();
    for(int x=0;x<24;++x)buy(w,sim::CommandType::PlaceRoad,{x,4});
    sim::BuildingId target{};
    for(int x=0;x<24;x+=3){const auto id=buy(w,sim::CommandType::PlaceHousehold,{x,2});if(!x)target=id;}
    (void)buy(w,sim::CommandType::PlaceOriginalLoggingShed,{0,5});
    sim::BuildingId producer{};
    if(lacquer) {
        const auto refinery=buy(w,sim::CommandType::PlaceOriginalLacquerRefinery,{3,5});
        sim::Command plant{sim::CommandType::PlaceOriginalLacquerTree,{5,5}};plant.building_id=refinery;f::buy(w,plant);
        producer=buy(w,sim::CommandType::PlaceOriginalLacquerwareMaker,{9,5});
    } else {
        (void)buy(w,sim::CommandType::PlaceOriginalBronzeSmelter,{3,5});
        (void)buy(w,sim::CommandType::PlaceClaySource,{6,5});
        producer=buy(w,sim::CommandType::PlaceOriginalBronzewareMaker,{9,5});
    }
    (void)buy(w,sim::CommandType::PlaceWarehouse,{12,5});(void)buy(w,sim::CommandType::PlaceMarket,{16,5});
    check(!need(w,target,1,orig::HouseField::BronzewareOrLacquerware).met,
          "Neither physical OR good unexpectedly satisfied the authored next-row need");
    until(w,[&]{return lacquer ? home(w,target).house.lacquerware>0:home(w,target).house.bronzeware>0;},
          "Paid single-side OR production/warehouse/market never physically delivered");
    const auto h=home(w,target).house;const auto either=need(w,target,1,orig::HouseField::BronzewareOrLacquerware);
    check(industry(w,producer).completed>0 && either.required==1 && either.met &&
          (lacquer ? h.bronzeware==0 && either.actual==0 && either.alternative>0:
                     h.lacquerware==0 && either.actual>0 && either.alternative==0),
          "Actual single-side OR receipt invented the absent good or required both sides");
    for(const auto field:{orig::HouseField::Hemp,orig::HouseField::Ceramics,orig::HouseField::Tea,orig::HouseField::Silk}) {
        const auto missing=need(w,target,1,field);
        check(missing.required==1 && missing.actual==0 && !missing.met,
              "Missing physical good was substituted by another delivered good");
    }
    check(!w.original_housing_diagnosis(target).next.needs_met &&
          home(w,target).house.type==orig::HouseType::Shelter && !history(w,target).first_transitions[1],
          "Single OR delivery bypassed the remaining conjunction into a higher House");
    through(w,std::max<std::uint64_t>(816,w.ticks()));
    check(w.taxes_collected_total()>0,"Authored paid negative control never earned genuine tax");
    const auto path=fixture.root.path/"single-or.json";auto copy=disk_roundtrip(fixture,w,path);
    for(int i=0;i<80;++i){w.tick();copy.tick();check(w.snapshot()==copy.snapshot(),"Single OR full-save continuation diverged");}
    const auto raw=Json::parse(f::read_text(path));const auto& reservations=raw["world"]["original_goods"]["reservations"];
    std::size_t ri=0;const unsigned good=lacquer?5U:4U;
    while(ri<reservations.size() && reservations[ri]["good"]!=good)++ri;
    check(ri<reservations.size(),"Actual OR good has no physical transfer history");
    const auto reservation="/world/original_goods/reservations/"+std::to_string(ri);
    hostile_pointers(fixture,w,lacquer?"lacquer-only":"bronze-only",{
        {reservation+"/good",3},{reservation+"/delivered",999},
        {"/world/original_production/finance/houses/0/last_assessment/type",17},
        {"/world/original_production/finance/houses/0/last_assessment/occupants",0},
        {"/world/original_production/housing/houses/0/first_transitions/1",Json::object()},
        {"/world/original_economy/houses/0/house/type",17}});
}

void paid_paper_confucian_and_expiry() {
    auto text=negative_model();oe_production_fixture::set_house_field(text,10,22,1);
    Fixture fixture(std::move(text));auto w=fixture.world();
    for(int x=0;x<24;++x)buy(w,sim::CommandType::PlaceRoad,{x,4});
    const auto elite=buy(w,sim::CommandType::PlaceOriginalElitePlot,{0,0});
    for(int x=4;x<24;x+=3)(void)buy(w,sim::CommandType::PlaceHousehold,{x,2});
    const auto farm=buy(w,sim::CommandType::PlaceOriginalHempFarm,{0,5});
    sim::Command field{sim::CommandType::PlaceOriginalHempField,{2,5}};field.building_id=farm;f::buy(w,field);
    const auto paper=buy(w,sim::CommandType::PlaceOriginalPaperMaker,{4,5});
    const auto warehouse=buy(w,sim::CommandType::PlaceWarehouse,{12,5});
    const auto academy=buy(w,sim::CommandType::PlaceOriginalConfucianAcademy,{18,5});const auto visitor=actor(w,academy);
    for(const auto id:{farm,paper})f::buy(w,sim::set_building_operation(id,false));
    through(w,600);const auto absent=w.original_service_availability(elite,orig::ServiceRole::Confucian);
    check(w.workers_assigned(academy)==w.workforce_required(academy) && w.building(academy).operating_enabled &&
          !auxiliary(w,academy).amounts[4] && absent.eligible_target && !absent.available_source_count &&
          !absent.delivered_timer && !absent.travelling_to_target_count &&
          w.original_production_state()->services.trips.empty() && w.original_production_state()->services.witnesses.empty() &&
          w.courier_dispatch_status(visitor).status==sim::CourierDispatchStatus::NoStock,
          "Fully staffed Rule12 Academy dispatched/arrived/granted a timer without actual Paper");
    (void)disk_roundtrip(fixture,w,fixture.root.path/"no-paper.json");
    for(const auto id:{farm,paper})f::buy(w,sim::set_building_operation(id,true));
    until(w,[&]{return home(w,elite).house.confucian>0;},"Genuine Rule12 Hemp→Paper→Warehouse→Academy visit never arrived");
    f::buy(w,sim::set_building_operation(academy,false));
    until(w,[&]{return w.courier(visitor).phase==sim::CourierPhase::IdleAtWorkshop &&
        history(w,elite).last_evaluation && history(w,elite).last_evaluation->input.confucian>0;},
        "Actual completed Confucian visit lacks its subsequent real evaluation witness");
    check(industry(w,paper).completed>0 && industry(w,paper).goods_inputs_used[0]==industry(w,paper).completed &&
          auxiliary(w,warehouse).received[4]>0 && auxiliary(w,academy).received[4]>0 && auxiliary(w,academy).consumed[4]>0,
          "Rule12 Paper visit skipped actual Hemp debit, warehouse receipt or Academy Paper consumption");
    for(const auto& h:w.original_economy_state()->houses)if(h.building_id!=elite)
        check(!h.house.confucian && std::none_of(w.original_production_state()->services.witnesses.begin(),
            w.original_production_state()->services.witnesses.end(),[&](const auto& v){return v.building_id==h.building_id && v.role==orig::ServiceRole::Confucian;}),
            "Elite-only Rule12 Academy granted a Common timer/witness");
    const auto target=fixture.root.path/"paper-positive.json";(void)disk_roundtrip(fixture,w,target);
    const auto raw=Json::parse(f::read_text(target));const auto hi=json_owner(raw["world"]["original_economy"]["houses"],elite);
    const auto hbase="/world/original_economy/houses/"+std::to_string(hi)+"/house/";
    const auto& services=w.original_production_state()->services;std::size_t wi=0,ti=0;
    while(wi<services.witnesses.size() && services.witnesses[wi].role!=orig::ServiceRole::Confucian)++wi;
    while(ti<services.trips.size() && services.trips[ti].transfer.courier_id!=visitor)++ti;
    check(wi<services.witnesses.size() && ti<services.trips.size(),"Rule12 positive visit lacks actual trip/arrival witness");
    const auto witness="/world/original_production/services/witnesses/"+std::to_string(wi);
    const auto trip="/world/original_production/services/trips/"+std::to_string(ti);
    const auto evaluation="/world/original_production/housing/houses/"+std::to_string(hi)+"/last_evaluation";
    hostile_pointers(fixture,w,"paper-visitor",{
        {hbase+"confucian",96},{witness+"/provider_id",static_cast<unsigned>(farm)},
        {witness+"/arrival_tick",w.ticks()+1},{witness+"/role",2},
        {trip+"/arrival_tick",nullptr},{trip+"/visit_applied",false},
        {evaluation+"/service_witnesses/3",nullptr},{evaluation+"/input/confucian",0},
        {"/world/original_production/industry/aux_trips/0/material",0}});
    const auto arrival=services.witnesses[wi].arrival_tick;
    const auto decays=[](std::uint64_t tick){return tick<36?0U:1+(tick-36)/51;};
    const auto expiry=36+(decays(arrival-1)+39)*51;
    through(w,expiry-1);check(home(w,elite).house.confucian==1,"Rule12 Confucian timer expired before its40th actual countdown");
    auto copy=disk_roundtrip(fixture,w,fixture.root.path/"paper-expiry.json");w.tick();copy.tick();
    check(w.snapshot()==copy.snapshot() && w.ticks()==expiry && !home(w,elite).house.confucian &&
          !need(w,elite,10,orig::HouseField::Confucian).met,
          "Paused/idle Rule12 visitor renewed a service or expiry continuation diverged");
    for(int i=0;i<80;++i){w.tick();copy.tick();check(w.snapshot()==copy.snapshot(),"Rule12 expired full-save continuation diverged");}
    authority(w);fixture.unchanged_source();
}

void paid_native_cargo_road_repair() {
    Fixture fixture(negative_model());auto w=fixture.world();
    for(int x=0;x<24;++x)buy(w,sim::CommandType::PlaceRoad,{x,4});
    for(int x=0;x<24;x+=3)(void)buy(w,sim::CommandType::PlaceHousehold,{x,2});
    const auto source=buy(w,sim::CommandType::PlaceClaySource,{0,5});
    const auto target=buy(w,sim::CommandType::PlacePottery,{20,5});const auto id=actor(w,source);
    until(w,[&]{const auto& c=w.courier(id);return c.phase==sim::CourierPhase::ToWarehouse &&
        c.cargo>0 && c.edge_progress==1 && c.path_vertex==0;},"Rule12 paid Clay never began a real reserved edge");
    const auto courier=w.courier(id);const auto position=w.courier_position(id);const auto funds=w.treasury();
    f::buy(w,{sim::CommandType::RemoveRoad,{10,4}});
    check(w.courier_position(id)==position && w.courier(id).cargo==courier.cargo &&
          w.courier(id).reserved==courier.reserved && w.treasury()==funds,
          "Rule12 road cut teleported a begun edge or lost native cargo/reservation");
    // Marking a begun edge pending invalidates its route check. The next
    // tick attempts the new road revision once before ordinary stable waiting.
    until(w,[&]{return w.courier(id).route_pending && w.courier(id).edge_progress==0 &&
        w.courier(id).route_checked_revision==w.road_revision();},
          "Rule12 disconnected Clay did not wait at its real vertex",30);
    auto copy=disk_roundtrip(fixture,w,fixture.root.path/"clay-waiting.json");const auto attempts=w.courier(id).reroute_attempts;
    for(int i=0;i<20;++i){w.tick();copy.tick();check(w.snapshot()==copy.snapshot(),"Rule12 disconnected full-save continuation diverged");}
    check(w.courier(id).cargo==courier.cargo && w.courier(id).reserved==courier.reserved &&
          w.building(target).reserved_incoming==courier.reserved && w.courier(id).reroute_attempts==attempts,
          "Unchanged Rule12 road revision retried routing or discarded retained shipment");
    const auto before=w.treasury();const auto spent=w.construction_spent_total();
    buy(w,sim::CommandType::PlaceRoad,{10,4});buy(copy,sim::CommandType::PlaceRoad,{10,4});
    check(w.treasury()==before-2 && w.construction_spent_total()==spent+2,"Rule12 cargo route repair was not actually paid");
    bool arrived=false;
    for(int i=0;i<300;++i){const auto phase=w.courier(id).phase;w.tick();copy.tick();
        check(w.snapshot()==copy.snapshot(),"Paid Rule12 road-repair disk continuation diverged");
        arrived|=phase==sim::CourierPhase::ToWarehouse && w.courier(id).phase==sim::CourierPhase::Returning;
        if(arrived && w.courier(id).phase==sim::CourierPhase::IdleAtWorkshop)break;
    }
    const auto& trips=w.original_production_state()->industry.clay_trips;
    const auto receipt=std::find_if(trips.begin(),trips.end(),[&](const auto& row){return row.transfer.courier_id==id;});
    check(arrived && !w.building(target).reserved_incoming && receipt!=trips.end() &&
          receipt->arrival_tick && receipt->delivered==courier.cargo,
          "Actual Rule12 repaired shipment never physically arrived/released reservation");
    (void)disk_roundtrip(fixture,w,fixture.root.path/"clay-repaired.json");authority(w);fixture.unchanged_source();
}

void paid_single_religion_and_natural_source_fire(bool buddhist) {
    auto text=negative_model();oe_production_fixture::set_house_field(text,1,3,0);
    oe_production_fixture::set_house_field(text,1,23,1);
    Fixture fixture(std::move(text));auto w=fixture.world();
    for(int x=0;x<24;++x)buy(w,sim::CommandType::PlaceRoad,{x,4});
    const auto source=buy(w,buddhist?sim::CommandType::PlaceOriginalBuddhistShrine:sim::CommandType::PlaceOriginalDaoistShrine,{20,5});
    const auto id=actor(w,source);f::buy(w,sim::set_building_operation(source,false));
    if(!buddhist)through(w,600);
    const auto target=buy(w,sim::CommandType::PlaceHousehold,{0,2});(void)buy(w,sim::CommandType::PlaceHousehold,{3,2});
    through(w,buddhist?600:1990);
    check(!need(w,target,1,orig::HouseField::DaoistOrBuddhist).met,"Absent religions satisfied the Rule12 authored OR gate");
    f::buy(w,sim::set_building_operation(source,true));
    until(w,[&]{return w.courier(id).phase==sim::CourierPhase::ToWarehouse && w.courier(id).edge_progress>0;},
          "Paid single religion did not begin an actual outbound edge",100);
    if(!buddhist) {
        until(w,[&]{return w.building_on_fire(source);},"Naturally aged Rule12 provider did not catch fire",30);
        check(!home(w,target).house.daoist,"Natural provider-fire control arrived before its intended incident");
    }
    until(w,[&]{return buddhist ? home(w,target).house.buddhist>0:home(w,target).house.daoist>0;},
          "Actual single religion/source-fire begun visit never reached the House",500);
    const auto h=home(w,target).house;const auto either=need(w,target,1,orig::HouseField::DaoistOrBuddhist);
    check(either.met && (buddhist ? !h.daoist && h.buddhist>0:h.daoist>0 && !h.buddhist) && !h.confucian,
          "Single actual religion failed OR or invented its absent alternative/Confucian");
    if(!buddhist) {
        check(w.building_on_fire(source) && !w.original_service_availability(target,orig::ServiceRole::Daoist).available_source_count,
              "Burning Rule12 source appeared available or erased its begun delivered visit");
        until(w,[&]{return w.courier(id).phase==sim::CourierPhase::IdleAtWorkshop;},"Burning source visitor never completed its real return",200);
        check(w.building_on_fire(source) && w.courier_dispatch_status(id).status==sim::CourierDispatchStatus::OnFire,
              "Burning Rule12 provider began a fresh dispatch");
    }
    f::buy(w,sim::set_building_operation(source,false));
    auto copy=disk_roundtrip(fixture,w,fixture.root.path/(buddhist?"buddhist-only.json":"natural-provider-fire.json"));
    for(int i=0;i<80;++i){w.tick();copy.tick();check(w.snapshot()==copy.snapshot(),"Rule12 single-religion/fire disk continuation diverged");}
    authority(w);fixture.unchanged_source();
}
}

int main(int argc,char** argv) {
    try {
        check(argc==2 && (std::string_view(argv[1])=="--stage-a" || std::string_view(argv[1])=="--stage-b"),
              "Usage: high-tier-progression-authority-tests --stage-a|--stage-b");
        const bool stage_a=std::string_view(argv[1])=="--stage-a";
        if(stage_a){readonly_matrix_and_identity();blocked_materialization_and_schema28();}
        else {paid_single_goods_or(false);paid_single_goods_or(true);paid_paper_confucian_and_expiry();
            paid_native_cargo_road_repair();paid_single_religion_and_natural_source_fire(true);
            paid_single_religion_and_natural_source_fire(false);}
        std::cout<<"PASS OE38 "<<(stage_a?"StageA":"StageB")<<" Rule12 diagnostic/paid admission/schema28 boundaries; "
                    "authored controls only, no Common10/Elite17 gameplay acceptance\n";
        return 0;
    }catch(const std::exception& error){
        perf::set_enabled(false);std::cerr<<"OE38 authority FAIL: "<<error.what()<<'\n';return 1;
    }
}
