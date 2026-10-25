#include "HighTierProgressionMatrix.h"
#include "OriginalProductionFixture.h"
#include "core/PerformanceDiagnostics.h"
#include "simulation/original/GoodsRules.h"

#include <iostream>
#include <memory>
#include <string_view>

namespace {
namespace matrix=oe_high_tier_matrix;
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
namespace perf=openemperor::performance;
using f::check;

struct Fixture {
    f::Root temporary;
    std::shared_ptr<const sim::MapPermissions> policy=f::permissions();
    std::shared_ptr<const orig::OriginalRulesModel> model;
    std::unique_ptr<const orig::HouseEvolutionRules> independent;
    bool original=false;
    explicit Fixture(const f::fs::path& data={}) {
        if(data.empty())f::write_text(temporary.data/"Model/EmperorBuildingModels.txt",oe_production_fixture::authored_model_text(7));
        auto loaded=orig::load_model_data(data.empty()?temporary.data:data);
        check(bool(loaded),"High-tier matrix model loading failed");original=!data.empty();
        model=orig::OriginalRulesModel::prepare_production(*loaded.model,orig::Difficulty::Normal);
        // A distinct object is prepared through the same bounded production
        // ModelData parser; no duplicate table or copied original fixture.
        independent=std::make_unique<const orig::HouseEvolutionRules>(*loaded.model,orig::Difficulty::Normal);
    }
    sim::World world()const{return sim::World::create_original_sustainable_economy(policy,model);}
};
sim::BuildingId buy(sim::World& world,sim::CommandType type,sim::Cell at) {
    f::buy(world,{type,at});return *world.building_owner_at(at);
}
const orig::HouseState& house(const sim::World& w,sim::BuildingId id) {
    for(const auto& value:w.original_economy_state()->houses)if(value.building_id==id)return value.house;
    throw std::runtime_error("Matrix paid House disappeared");
}
void through(sim::World& world,std::uint64_t target) {
    while(world.ticks()<target)world.tick();
    check(world.navigation_valid()&&world.population_valid()&&world.production_balance_valid()&&world.city_economy_valid(),
        "Matrix ordinary paid scenario violated World authority");
}
const matrix::Json& type(const matrix::Json& report,unsigned wanted) {
    for(const auto& row:report.at("types16"))if(row.at("type")==wanted)return row;
    throw std::runtime_error("Matrix dropped a type");
}
const matrix::Json& body(const matrix::Json& report,sim::BuildingId wanted) {
    for(const auto& row:report.at("houses"))if(row.at("id")==static_cast<unsigned>(wanted))return row;
    throw std::runtime_error("Matrix dropped an actually purchased House");
}
void read_only_tables_and_real_family_paths(const f::fs::path& data) {
    Fixture fxt(data);auto world=fxt.world();
    for(int x=0;x<24;++x)f::buy(world,{sim::CommandType::PlaceRoad,{x,4}});
    const auto common=buy(world,sim::CommandType::PlaceHousehold,{0,2});
    buy(world,sim::CommandType::PlaceHousehold,{3,2});
    const auto elite=buy(world,sim::CommandType::PlaceOriginalElitePlot,{3,5});
    const auto academy=buy(world,sim::CommandType::PlaceOriginalConfucianAcademy,{9,5});
    f::buy(world,{sim::CommandType::PlaceRoad,{6,7}}); // Real future4x4 conflict.
    const auto initial=world.snapshot();const auto zero=matrix::report(world,*fxt.independent);
    check(world.snapshot()==initial&&zero.at("types16").size()==16&&zero.at("houses").size()==3,
        "Matrix mutated paid commands/time/Funds/IDs or lost rows");
    check(!type(zero,10).at("actual_occupied_tier_witnessed").get<bool>()&&
        !type(zero,17).at("actual_occupied_tier_witnessed").get<bool>(),"Hypothetical matrix claimed actual Common10/Elite17");
    check(type(zero,10).at("classification")!="already supported"&&
        type(zero,17).at("classification")!="already supported"&&
        !type(zero,10).at("observed_reasons").empty()&&
        !type(zero,17).at("same_family_observations").empty(),
        "Unwitnessed high tiers were classified as already reached without local shortage/witness evidence");
    check(body(zero,elite).at("classification")=="geometrically blocked"&&
        type(zero,11).at("physical_side")==2&&type(zero,13).at("physical_side")==4,
        "Matrix hid actual blocked Elite expansion or conflated initial/final footprint");
    through(world,120);
    check(house(world,common).occupants>0&&house(world,elite).type==orig::HouseType::EliteUnoccupied&&
        house(world,elite).occupants==0,"Blocked prospective4x4 granted Elite residents or blocked independent Common admission");
    f::buy(world,{sim::CommandType::RemoveRoad,{6,7}});
    through(world,160);
    check(house(world,elite).type==orig::HouseType::ModestSiheyuan&&house(world,elite).occupants==2&&
        world.building(elite).kind==sim::Object::OriginalEliteHouse&&world.building_owner_at({6,8})==elite,
        "Real paid repair did not permit atomic11->13 four-square admission");
    const auto admitted=matrix::report(world,*fxt.independent);
    check(type(admitted,13).at("actual_occupied_tier_witnessed").get<bool>()&&
        !type(admitted,12).at("actual_occupied_tier_witnessed").get<bool>()&&
        !type(admitted,14).at("actual_occupied_tier_witnessed").get<bool>(),
        "Actual admission was promoted into unearned Elite devolution/evolution proof");
    check(body(admitted,elite).at("actual_footprint_side")==4&&
        body(admitted,elite).at("current").at("needs_met")==false,
        "Empty Hemp/Ceramics were hidden by legal admission into a needy Elite row");
    through(world,600);
    check(world.workers_assigned(academy)==world.workforce_required(academy),
        "Paper-negative scenario never staffed its actual Academy");
    const auto eligible=world.original_service_availability(elite,orig::ServiceRole::Confucian);
    const auto ineligible=world.original_service_availability(common,orig::ServiceRole::Confucian);
    check(eligible.eligible_target&&!ineligible.eligible_target&&eligible.provider_count==1&&
        eligible.available_source_count==0&&!eligible.last_arrival_tick&&!house(world,elite).confucian,
        "Staffed but Paper-empty Academy created a visit or served the wrong House family");

    const auto before=world.snapshot();perf::set_enabled(true);perf::reset();
    const auto observed=matrix::report(world,*fxt.independent);
    for(const auto count:{perf::Counter::WorldCopies,perf::Counter::WorldRestores,perf::Counter::WorldExecutes,
        perf::Counter::SimulationTicks,perf::Counter::RouteRefreshes,perf::Counter::BfsCalls,
        perf::Counter::FileReads,perf::Counter::FileWrites,perf::Counter::AssetDecodes,perf::Counter::TextureUploads})
        check(perf::counter(count)==0,"On-demand matrix performed gameplay, navigation, source IO or rendering work");
    perf::set_enabled(false);check(world.snapshot()==before,"Live matrix changed actual authority");
    check(body(observed,common).at("actual_service_availability").size()==9&&
        body(observed,elite).at("matrix15").size()==15&&
        !body(observed,elite).at("last_actual_evaluation").is_null(),"Live matrix lost nine services, model rows or actual retained evaluation input");
    for(const auto& row:body(observed,common).at("matrix15"))
        check(row.at("same_family").get<bool>()==(row.at("hypothetical_type").get<unsigned>()<11),
            "Common matrix silently authorized an Elite conversion");
    for(const auto& stage:observed.at("types16")) {
        const auto& estimate=stage.at("incremental_supply_estimate");
        std::set<unsigned> commands;std::int64_t cost=0,workers=0;
        for(const auto& purchase:estimate.at("purchases")) {
            const auto command=purchase.at("command").get<unsigned>();
            check(commands.insert(command).second,"Shared source was charged twice in one stage's hypothetical infrastructure estimate");
            check(command!=static_cast<unsigned>(sim::CommandType::PlaceOriginalConfucianAcademy),
                "Existing paid Academy was billed a second time in incremental stage estimate");
            cost+=purchase.at("cost").get<std::int64_t>();workers+=purchase.at("installed_workers").get<std::int64_t>();
        }
        check(cost==estimate.at("purchase_cost")&&workers==estimate.at("incremental_installed_workers")&&
            workers*sim::original_worker_upkeep_funds==estimate.at("incremental_upkeep_per_invoice")&&
            estimate.at("hypothetical")==true,"Stage estimates hid costs/workers or represented planning arithmetic as an actual paid result");
    }
    const auto alien=orig::parse_model_data(oe_production_fixture::authored_model_text(19));
    check(bool(alien),"Distinct authored negative model failed parsing");
    const orig::HouseEvolutionRules wrong(*alien.model,orig::Difficulty::Normal);
    bool refused=false;try{(void)matrix::report(world,wrong);}catch(const std::exception&){refused=true;}
    check(refused&&world.snapshot()==before,"Matrix accepted another independent model identity or mutated on refusal");
    if(fxt.original) {
        check(type(observed,10).at("entry_from_previous").at("minimum_des_inclusive")==40&&
            type(observed,10).at("low_inclusive_devolve")==35&&
            type(observed,17).at("entry_from_previous").at("minimum_des_inclusive")==80&&
            type(observed,17).at("low_inclusive_devolve")==75,
            "Real Normal matrix confused entry DES with destination high/retention threshold");
        check(type(observed,11).at("row_capacity")==0&&type(observed,11).at("census_capacity")==1&&
            type(observed,11).at("admission_capacity")==10,"Empty Elite contextual capacity selectors were silently repaired");
        check(type(observed,10).at("classification")=="supported but insufficient supply"&&
            type(observed,10).at("representative_observation").at("entry_des_deficit").get<int>()>0&&
            !type(observed,10).at("representative_observation").at("projected_current_missing").empty(),
            "Real Normal target did not expose observed DES and current supply deficits");
    }
}

orig::HouseState satisfied(orig::HouseType type) {
    orig::HouseState value;value.type=type;value.occupants=1;value.water=true;
    value.herbalist=value.acupuncture=value.music=value.acrobat=value.drama=1;
    value.ancestor=value.confucian=value.daoist=value.buddhist=1;
    value.food_quality=sim::original_food_batch_quality({0,1}).value;
    value.hemp=value.ceramics=value.tea=value.silk=value.bronzeware=value.lacquerware=sim::Rules::household_capacity;
    return value;
}
void independent_entry_retention_and_no_conversion(const f::fs::path& data) {
    Fixture fxt(data);const auto& rules=*fxt.independent;
    for(unsigned number=4;number<=17;++number) {
        if(number>=11&&number<=13)continue;
        const auto next=static_cast<orig::HouseType>(number),prior=static_cast<orig::HouseType>(number-1);
        auto input=satisfied(prior);const auto high=rules.value(orig::house_row(prior),orig::HouseField::DesirabilityHigh);
        input.desirability=static_cast<std::int8_t>(high);
        check(orig::evolve_step(rules,input).type==next,"Inclusive source-row high failed to enter its same-family next type");
        input.desirability=static_cast<std::int8_t>(high-1);
        check(orig::evolve_step(rules,input).type!=next,"House entered next type below actual previous-row high");
        auto retained=satisfied(next);const auto low=rules.value(orig::house_row(next),orig::HouseField::DesirabilityLow);
        retained.desirability=static_cast<std::int8_t>(low+1);
        check(orig::evolution_verdict(rules,retained)!=orig::Verdict::Devolve,"Destination low+1 did not retain supplied type");
        retained.desirability=static_cast<std::int8_t>(low);
        retained.devolve_counter=orig::devolve_limit(next);
        check(orig::evolve_step(rules,retained).type==prior,"Inclusive destination low/counter boundary failed real family devolution");
    }
    for(const auto terminal:{orig::HouseType::LuxuriousApartment,orig::HouseType::HeavenlyCompound}) {
        auto input=satisfied(terminal);input.desirability=100;
        check(!orig::evolve_target(terminal)&&orig::evolve_step(rules,input).type==terminal,
            "Terminal row invented an adjacent-family conversion");
    }
    auto plot=satisfied(orig::HouseType::EliteUnoccupied);plot.occupants=0;plot.desirability=100;
    check(orig::evolve_step(rules,plot)==plot,"Pure matrix granted Elite admission/materialization without World");
}

void independently_labeled_alternative_goods() {
    // A minimal PURE negative control, never published into any World or
    // accepted as a paid progression witness. No original model row is copied.
    auto bytes=oe_production_fixture::authored_model_text(7);
    oe_production_fixture::set_house_field(bytes,12,13,1);
    const auto parsed=orig::parse_model_data(bytes);check(bool(parsed),"Alternative-goods pure fixture failed parsing");
    const orig::HouseEvolutionRules rules(*parsed.model,orig::Difficulty::Normal);
    auto value=satisfied(orig::HouseType::HumbleCompound);value.bronzeware=0;value.lacquerware=1;
    check(orig::needs_met(rules,orig::house_row(value.type),value),"Lacquer alternative did not satisfy the shared need");
    value.lacquerware=0;value.bronzeware=1;
    check(orig::needs_met(rules,orig::house_row(value.type),value),"Bronze alternative did not satisfy the shared need");
    value.bronzeware=0;
    check(!orig::needs_met(rules,orig::house_row(value.type),value),"Absent alternatives were combined into a fictional good");
    value.bronzeware=value.lacquerware=1;const auto consumed=orig::consume_goods(rules,value);
    check(consumed.house.bronzeware==0&&consumed.house.lacquerware==0&&consumed.consumed[4]==1&&consumed.consumed[5]==1,
        "OR eligibility was mistaken for first-available/shared-budget consumption");
}
}
int main(int argc,char** argv) {
    try {
        check(argc==1||(argc==3&&std::string_view(argv[1])=="--real"),"Usage: high-tier-matrix-tests [--real DATA]");
        const f::fs::path data=argc==3?f::fs::path(argv[2]):f::fs::path{};
        if(argc==3&&(data.empty()||!f::fs::is_regular_file(data/"Model/EmperorBuildingModels.txt"))) {
            std::cout<<"SKIP: supplied original model installation is unavailable.\n";return 77;
        }
        read_only_tables_and_real_family_paths(data);independent_entry_retention_and_no_conversion(data);
        independently_labeled_alternative_goods();
        std::cout<<"PASS: high-tier read-only matrix, paid admission/blocked repair/Paper negative, independent thresholds and OR semantics ("
            <<(data.empty()?"authored model/geography":"unchanged original Normal model; authored test geography")<<").\n";
        return 0;
    }catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}
}
