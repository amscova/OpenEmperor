#include "HighTierProgressionStrategy.h"
#include "simulation/OriginalHousingDiagnosis.h"

#include <chrono>
#include <iostream>
#include <optional>
#include <string_view>

namespace {
namespace q=oe_starter_fixture;
namespace strategy=oe_high_tier_strategy;
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
namespace save=openemperor::persistence;
namespace fs=std::filesystem;
using q::check;

struct Input {
    fs::path path;std::string bytes;fs::perms permissions;fs::file_time_type modified;
    explicit Input(fs::path file):path(std::move(file)),bytes(q::f::read_text(path)),
        permissions(fs::status(path).permissions()),modified(fs::last_write_time(path)){}
    void unchanged()const {
        check(q::f::read_text(path)==bytes&&fs::status(path).permissions()==permissions&&fs::last_write_time(path)==modified,
            "Common supply regression changed original input bytes/mode/mtime");
    }
};
const sim::OriginalHousingHouse& history(const sim::World& w,sim::BuildingId target) {
    for(const auto& h:w.original_production_state()->housing.houses)if(h.building_id==target)return h;
    throw std::runtime_error("Common supply regression lost target transition history");
}
int hemp_stock(const sim::World& w,sim::BuildingId id) {
    for(const auto& stock:w.original_goods_state()->stocks)if(stock.building_id==id)return stock.amounts[0];
    throw std::runtime_error("Common supply regression lost actual typed Hemp owner");
}
void tick(sim::World& w) {
    const auto before=w.ticks();w.tick();check(w.ticks()==before+1,"Real paid World refused the next regression tick");
    if(w.ticks()%816==0)q::authority(w);
}
void complete_roundtrip(const q::Context& c,const sim::World& w) {
    const auto document=c.document(w);
    auto direct=save::restore_original_sustainable_economy(document,c.data,c.mask,c.policy,c.model);
    q::f::same_document(document,c.document(direct));check(direct.snapshot()==w.snapshot(),"Complete in-memory paid World restore differs");
    const auto parent=fs::canonical(fs::path(__FILE__).parent_path().parent_path())/".local/original-economy/oe38/tests";
    const auto directory=parent/("common-supply-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    check(!fs::exists(directory),"Private regression scratch already exists");fs::create_directories(directory);
    struct Cleanup {fs::path path;~Cleanup(){std::error_code e;fs::remove_all(path,e);}} cleanup{directory};
    const auto first=directory/"first.save.json",second=directory/"restored.save.json";
    save::write_save(first,document,c.data,c.mask);const auto parsed=save::read_save(first);
    check(parsed.source_schema_version==28&&parsed.world.rule_version==12,"Common supply save lost its exact Rule12/Schema28 identity");
    auto restored=save::restore_original_sustainable_economy(parsed,c.data,c.mask,c.policy,c.model);
    q::f::same_document(document,c.document(restored));check(restored.snapshot()==w.snapshot(),"Strict complete parsed Rule12 restore differs");
    save::write_save(second,c.document(restored),c.data,c.mask);
    check(q::f::read_text(first)==q::f::read_text(second),"Complete canonical Rule12 SaveDocument changed after strict restore");
}
void reproduce(const fs::path& data) {
    const Input model_input(data/"Model/EmperorBuildingModels.txt"),map_input(data/"Cities/Xia.map");
    const auto c=q::Context::real(data);const auto parsed=orig::load_model_data(data);
    check(bool(parsed)&&c.model->provenance().sha256=="7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6",
        "Supply regression requires unchanged original Normal data");
    const orig::HouseEvolutionRules independent(*parsed.model,orig::Difficulty::Normal);
    auto w=sim::World::create_original_sustainable_economy(c.policy,c.model);
    const auto origin=oe_sustainable_strategy::origin(c);
    (void)oe_sustainable_strategy::build(w,"balanced-three",origin);
    while(w.ticks()<20000)tick(w);
    check(oe_sustainable_strategy::maturity(w).at("ready").get<bool>(),"Supply regression failed to earn the real OE37 baseline");
    const auto spent=w.construction_spent_total();const auto funds=w.treasury();
    const auto expansion=strategy::build_common(w,origin,"common-reserved");const auto target=expansion.target;
    check(static_cast<unsigned>(target)==3&&w.construction_spent_total()-spent==719&&w.treasury()==funds-719,
        "Supply regression did not pay the actual719 Common expansion");
    const auto warehouse=q::body(w,sim::Object::Warehouse);
    std::optional<std::uint64_t> first_reached,first_missing;
    unsigned consecutive=0;bool observed_devolution=false;
    while(w.ticks()<29467) {
        tick(w);const auto& actual=q::house(w,target);const auto& h=history(w,target);
        if(actual.type==orig::HouseType::LuxuriousApartment&&!first_reached) {
            first_reached=w.ticks();
            const auto& entry=h.first_transitions[orig::model_row(orig::house_row(actual.type)).value];
            check(entry&&entry->kind==sim::OriginalHousingTransitionKind::Evolution&&entry->evaluation.tick==w.ticks()&&
                entry->evaluation.input.type==orig::HouseType::OrnateApartment,
                "Real Common10 lacks an actual9->10 first-transition witness");
        }
        if(!h.last_evaluation||h.last_evaluation->tick!=w.ticks())continue;
        const auto& input=h.last_evaluation->input;const auto pure=orig::evolve_step(independent,input);
        check(pure.type==actual.type&&pure.devolve_counter==actual.devolve_counter,
            "Actual paid target decision differs from independently prepared original HouseEvolutionRules");
        if(!first_reached)continue;
        const bool unmet=!orig::needs_met(independent,orig::house_row(input.type),input);
        if(unmet)++consecutive;else consecutive=0;
        if(unmet&&!first_missing) {
            first_missing=w.ticks();
            check(input.type==orig::HouseType::LuxuriousApartment&&input.hemp==0&&input.devolve_counter==0&&
                hemp_stock(w,warehouse)>=28,"First actual target shortage lost its observed local-stock/global-stock distinction");
            unsigned backed_up=0;
            for(const auto& farm:w.original_production_state()->hemp_farms)for(const auto& courier:w.couriers())
                if(courier.owner==farm.farm_id&&courier.role==sim::CourierRole::OriginalHempToWarehouse&&
                    w.courier_dispatch_status(courier.id).status==sim::CourierDispatchStatus::TargetFull)++backed_up;
            check(backed_up>0,"Empty target Hemp was incorrectly reproduced without actual downstream Farm backpressure");
            check(w.original_goods_balance(orig::Goods::Hemp).conserved,"Backpressure case lost actual Hemp conservation");
        }
        if(input.type==orig::HouseType::LuxuriousApartment&&actual.type==orig::HouseType::OrnateApartment) {
            check(consecutive==unsigned(orig::devolve_limit(input.type))+1&&input.hemp==0&&
                input.devolve_counter==orig::devolve_limit(input.type),
                "Observed10->9 did not follow ten real consecutive unmet evaluations and the original counter boundary");
            observed_devolution=true;
        }
    }
    check(first_reached==27733&&first_missing==29008&&observed_devolution&&
        q::house(w,target).type==orig::HouseType::OrnateApartment,
        "Bounded real Common shortage no longer reproduces27733 reach/29008 shortage/29467 devolution; investigate actual causes");
    q::authority(w);complete_roundtrip(c,w);model_input.unchanged();map_input.unchanged();
    std::cout<<"PASS: actual paid Xia Rule12 Common#3 reaches10 at27733; Hemp0 at29008 despite Warehouse>=28 and Farm backpressure; "
        <<"ten real unmet evaluations yield10->9 at29467; conserved goods and complete strict schema28 restore equal.\n";
}
}
int main(int argc,char** argv) {
    try {
        check(argc==3&&std::string_view(argv[1])=="--real","Usage: high-tier-common-supply-tests --real DATA");
        const fs::path data=argv[2];
        if(data.empty()||!fs::is_regular_file(data/"Model/EmperorBuildingModels.txt")||!fs::is_regular_file(data/"Cities/Xia.map")) {
            std::cout<<"SKIP: supplied original Model/Xia installation is unavailable.\n";return 77;
        }
        reproduce(fs::canonical(data));return 0;
    }catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}
}
