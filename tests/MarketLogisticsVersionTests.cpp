// Authored integration inputs only; no fixture is an original-data progression proof.
#include "OriginalFinanceFixture.h"
#include "persistence/RecoveryStore.h"
#include <nlohmann/json.hpp>
#include <iostream>

namespace {
namespace q=oe_finance_fixture;
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace save=openemperor::persistence;
using Json=nlohmann::json;
using f::check;
template<class F> void refuses(F action) {
    bool denied=false;try {action();}catch(const std::exception&){denied=true;}
    check(denied,"Wrong-generation or hostile logistics authority was accepted");
}
void identities() {
    q::Fixture fixture;auto w=sim::World::create_original_market_logistics(fixture.policy,fixture.model);
    check(w.rule_version()==13 && w.active_rules().version==13 && w.original_market_logistics_enabled() &&
        w.original_sustainable_economy_enabled() && w.original_fire_protection_enabled() &&
        w.original_production_enabled() && w.original_goods_enabled() && w.original_influence_enabled() &&
        w.bridges_supported() && w.ramps_supported() && w.original_worker_upkeep_interval()==816 &&
        w.treasury()==12000 && w.original_production_state()->development_revision==27,
        "Rule13 did not inherit its explicit sustainable production/fire contract");
    check(sim::current_rule_version(sim::RulesProfile::CityV16)==4 &&
        sim::current_rule_version(sim::RulesProfile::CityV11)==3 &&
        sim::Rules::original_production_building_limit==82 && sim::Rules::original_production_courier_limit==58 &&
        sim::original_food_farm_limit(w.profile(),13)==3 && sim::original_food_farm_limit(w.profile(),11)==2,
        "Rule13 changed defaults, actor bounds or historical Food capacity");
    refuses([&]{sim::World ordinary(fixture.policy,sim::RulesProfile::CityV16,13);});
    check(sim::World::restore_original_market_logistics(w.snapshot(),fixture.policy,fixture.model).snapshot()==w.snapshot(),
        "Explicit empty Rule13 restore changed authority");
    refuses([&]{(void)sim::World::restore_original_sustainable_economy(w.snapshot(),fixture.policy,fixture.model);});
    refuses([&]{(void)sim::World::restore_original_fire_protection(w.snapshot(),fixture.policy,fixture.model);});
    refuses([&]{(void)sim::World::restore_original_production_development(w.snapshot(),fixture.policy,fixture.model);});
    const auto old=sim::World::create_original_sustainable_economy(fixture.policy,fixture.model);
    refuses([&]{(void)sim::World::restore_original_market_logistics(old.snapshot(),fixture.policy,fixture.model);});
    check(!old.original_market_logistics_enabled() && old.rule_version()==12 && old.active_rules().version==12 &&
        sim::World::restore_original_sustainable_economy(old.snapshot(),fixture.policy,fixture.model).snapshot()==old.snapshot() &&
        old.canonical_state()!=w.canonical_state(),"Rule12 became a logistics opt-in or lost its exact restore");
}
void paid_save_recovery_and_hostile_inputs() {
    q::Fixture fixture;auto w=sim::World::create_original_market_logistics(fixture.policy,fixture.model);
    for(int x=0;x<24;++x)f::buy(w,{sim::CommandType::PlaceRoad,{x,4}});
    for(int x=0;x<12;x+=3)f::buy(w,{sim::CommandType::PlaceHousehold,{x,2}});
    f::buy(w,{sim::CommandType::PlaceMarket,{5,5}});
    f::buy(w,{sim::CommandType::PlaceOriginalWaterWell,{9,5}});
    f::buy(w,{sim::CommandType::PlaceOriginalAncestorShrine,{13,5}});
    for(const auto& [farm_x,field_x]:std::array{std::pair{0,3},std::pair{15,18},std::pair{19,22}}) {
        f::buy(w,{sim::CommandType::PlaceOriginalFoodFarm,{farm_x,5}});
        sim::Command field{sim::CommandType::PlaceOriginalMilletField,{field_x,5}};
        field.building_id=*w.building_owner_at({farm_x,5});f::buy(w,field);
    }
    f::buy(w,{sim::CommandType::PlaceFireWatch,{23,5}});
    const auto unpaid=w.snapshot();const auto fourth=w.execute({sim::CommandType::PlaceOriginalFoodFarm,{10,5}});
    check(!fourth.accepted && !fourth.changed && w.snapshot()==unpaid && !w.total_population(),
        "Fourth Farm refusal mutated authority or construction granted residents");
    for(unsigned i=0;i<1700;++i)w.tick();
    check(w.taxes_collected_total()>0 && !w.original_production_state()->fire_protection->arrivals.empty(),
        "Paid Rule13 integration did not earn tax/Inspector arrivals");
    std::uint64_t installed=0;
    for(const auto& b:w.buildings())installed+=(w.ticks()-b.placed_tick)/816*static_cast<unsigned>(w.workforce_required(b.id))*2;
    check(installed==w.maintenance_spent_total(),"Rule13 changed Rule12 installed monthly liability");
    const auto mask=fixture.policy->building_mask();const auto path=fixture.root.path/"good-rule13.json";
    auto document=fixture.document(w);save::write_save(path,document,fixture.root.data,mask);
    const auto bytes=f::read_text(path);const auto before=w.snapshot();const auto parsed=save::read_save(path);
    check(parsed.source_schema_version==29 && parsed.world.rule_version==13 && parsed.world==before &&
        parsed.map_permissions_policy_version==3,"Rule13 did not preserve exact Schema29/policy3 identity");
    auto restored=save::restore_original_market_logistics(parsed,fixture.root.data,mask,fixture.policy,fixture.model);
    save::write_save(path,fixture.document(restored),fixture.root.data,mask);
    check(f::read_text(path)==bytes,"Rule13 full canonical SaveDocument changed after rewrite");
    refuses([&]{(void)save::restore_original_sustainable_economy(parsed,fixture.root.data,mask,fixture.policy,fixture.model);});
    refuses([&]{(void)save::restore_original_fire_protection(parsed,fixture.root.data,mask,fixture.policy,fixture.model);});
    refuses([&]{(void)save::restore_save(parsed,fixture.root.data,mask,fixture.policy);});
    const auto reject=[&](auto edit) {
        auto bad=parsed;edit(bad);
        refuses([&]{(void)save::restore_original_market_logistics(bad,fixture.root.data,mask,fixture.policy,fixture.model);});
        check(w.snapshot()==before && f::read_text(path)==bytes,"Rejected logistics restore changed city or prior save");
    };
    reject([](auto& d){d.source_schema_version=28;});
    reject([](auto& d){d.world.rule_version=12;});
    reject([](auto& d){d.world.original_production->development_revision=28;});
    reject([](auto& d){d.map_permissions_policy_version=2;});
    reject([](auto& d){d.map_sha256=std::string(64,'0');});
    reject([](auto& d){d.world.original_economy->model_source->sha256=std::string(64,'0');});
    reject([](auto& d){d.world.original_economy->model_source->difficulty=openemperor::original::Difficulty::Hard;});
    reject([](auto& d){++d.world.treasury;});
    reject([](auto& d){d.world.couriers.front().owner=static_cast<sim::BuildingId>(999);});
    reject([](auto& d){d.world.couriers.front().edge_progress=5;});
    reject([](auto& d){d.world.original_production->fire_protection.reset();});
    reject([](auto& d){d.world.original_production->fire_protection->arrivals.clear();});
    reject([](auto& d){d.world.original_production->food.farms[2].field_id=d.world.original_production->food.farms[0].field_id;});
    const auto raw_reject=[&](auto edit) {
        auto j=Json::parse(bytes);edit(j);const auto bad=fixture.root.path/"hostile-rule13.json";f::write_text(bad,j.dump());
        refuses([&]{(void)save::read_save(bad);});
        check(f::read_text(path)==bytes && w.snapshot()==before,"Hostile JSON changed paid city or prior save");
    };
    raw_reject([](auto& j){j["schema_version"]=28;});
    raw_reject([](auto& j){j["rules"]["version"]=12;});
    raw_reject([](auto& j){j["rules"].erase("market_logistics");});
    raw_reject([](auto& j){j["rules"]["market_logistics"]="unknown";});
    raw_reject([](auto& j){j["world"]["original_production"]["unknown"]=1;});
    raw_reject([](auto& j){j["world"]["original_goods"]["totals"][0]["produced"]=1;});
    raw_reject([](auto& j){j["world"]["couriers"][0]["unknown_cursor"]=1;});
    auto duplicate=bytes;const auto at=duplicate.find("\"travelled_cells\":");check(at!=std::string::npos,"Missing Inspector history");
    duplicate.insert(at,"\"travelled_cells\":0,");const auto dup=fixture.root.path/"duplicate-rule13.json";
    f::write_text(dup,duplicate);refuses([&]{(void)save::read_save(dup);});
    save::RecoveryStore legacy(fixture.root.path/"legacy",fixture.root.data);
    refuses([&]{(void)legacy.create_history(document,mask);});
    save::RecoveryStore recovery(fixture.root.path/"logistics",fixture.root.data,save::recovery_storage_budget_bytes,
        save::RecoveryAccess::OriginalProduction);
    const auto history=recovery.create_history(document,mask);
    for(unsigned i=0;i<400;++i){w.tick();restored.tick();check(w.snapshot()==restored.snapshot(),"Rule13 full continuation diverged");}
    const auto entry=recovery.write_checkpoint(history,fixture.document(w),mask);
    check(entry.loadable() && entry.schema==29 && entry.rule_version==13,"Recovery changed Rule13/Schema29");
    check(save::restore_original_market_logistics(save::read_save(entry.path),fixture.root.data,mask,fixture.policy,fixture.model).snapshot()==w.snapshot(),
        "Recovery lost complete logistics authority");
    const auto old=sim::World::create_original_sustainable_economy(fixture.policy,fixture.model);
    const auto old_doc=fixture.document(old);refuses([&]{(void)recovery.write_checkpoint(history,old_doc,mask);});
    const auto old_path=fixture.root.path/"unchanged-rule12.json";save::write_save(old_path,old_doc,fixture.root.data,mask);
    const auto old_bytes=f::read_text(old_path);const auto old_parsed=save::read_save(old_path);
    auto relabel=Json::parse(old_bytes);relabel["schema_version"]=29;relabel["rules"]["version"]=13;
    const auto relabel_path=fixture.root.path/"relabelled-rule12.json";f::write_text(relabel_path,relabel.dump());
    refuses([&]{(void)save::read_save(relabel_path);});
    refuses([&]{(void)save::restore_original_market_logistics(old_parsed,fixture.root.data,mask,fixture.policy,fixture.model);});
    const auto old_restored=save::restore_original_sustainable_economy(old_parsed,fixture.root.data,mask,fixture.policy,fixture.model);
    save::write_save(old_path,fixture.document(old_restored),fixture.root.data,mask);
    check(old_parsed.source_schema_version==28 && old_parsed.world.rule_version==12 && f::read_text(old_path)==old_bytes,
        "Historical Rule12 save was relabelled or changed by Rule13 I/O");
}
}
int main() {try {identities();paid_save_recovery_and_hostile_inputs();
    std::cout<<"Rule13 exact identity, paid inherited authority, Schema29 hostile inputs and Recovery PASS\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
