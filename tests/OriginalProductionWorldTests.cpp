#include "OriginalInfluenceFixture.h"
#include "simulation/original/OriginalRulesModel.h"

#include <algorithm>
#include <iostream>

namespace {
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
using f::check;
template<class F> void refuses(F action) {
    bool denied=false;try {action();} catch (const std::exception&) {denied=true;}
    check(denied,"Hostile original production authority accepted");
}
struct Fixture {
    f::Root root;
    std::shared_ptr<const sim::MapPermissions> policy=f::permissions();
    std::shared_ptr<const orig::OriginalRulesModel> model;
    explicit Fixture(int capacity=6) {
        auto text=f::authored_model_text();std::string old="1: Shelter,{,",replacement=old;
        for (int i=0;i<17;++i) {old+="0,";replacement+="0,";}
        old+="0,";replacement+=std::to_string(capacity)+",";
        const auto at=text.find(old);check(at!=std::string::npos,"Authored Shelter capacity row missing");
        text.replace(at,old.size(),replacement);
        f::write_text(root.data/"Model/EmperorBuildingModels.txt",text);
        model=f::save::prepare_original_rules_model(root.data);
    }
    sim::World world() const {return sim::World::create_original_production_development(policy,model);}
};
void ticks(sim::World& world,int count) {for (int i=0;i<count;++i) world.tick();}
struct Scene {sim::BuildingId clay{},pottery{},warehouse{};};
Scene scene(sim::World& world,bool houses=true,bool roads=true) {
    if (roads) for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,8}});
    if (houses) for (int x=0;x<12;x+=3) f::buy(world,{sim::CommandType::PlaceHousehold,{x,6}});
    f::buy(world,{sim::CommandType::PlaceClaySource,{0,9}});
    f::buy(world,{sim::CommandType::PlacePottery,{8,9}});
    f::buy(world,{sim::CommandType::PlaceWarehouse,{16,9}});
    return {*world.building_owner_at({0,9}),*world.building_owner_at({8,9}),*world.building_owner_at({16,9})};
}
void conserved(const sim::World& world) {
    check(world.production_balance_valid() && world.city_economy_valid() && world.population_valid() && world.navigation_valid(),
        "Real paid production authority invariant failed");
    const auto balances=world.original_goods_balances();
    for (std::size_t i=0;i<balances.size();++i) {
        const auto& b=balances[i];check(b.conserved,"Original goods conservation failed");
        check(b.supported==(i==static_cast<std::size_t>(orig::Goods::Ceramics)),"Revision14 granted an unsupported good");
        check(b.totals.imported==0 && b.totals.consumed==0 && b.totals.demolished==0 && b.cargo==0,
            "Revision14 granted import, consumption or typed Courier authority");
    }
    const auto& original=*world.original_production_state();
    check(balances[1].totals.produced==original.pottery_handed_off && balances[1].stock==original.pottery_handed_off,
        "Warehouse conversion was duplicated, consumed or lost");
    for (const auto& b:world.buildings()) check(b.pottery_stock==0 && b.population==0,
        "Ceramics or residents were mirrored into legacy Household/Warehouse fields");
}
void immigration_and_paid_origin() {
    Fixture fixture;auto world=fixture.world();const auto ids=scene(world);
    check(world.rule_version()==10 && world.original_production_enabled() && world.treasury()==11182 &&
        world.construction_spent_total()==818 && world.total_population()==0 && world.workforce_supply()==0,
        "Paid Rule10 scene, starting funds or vacant bootstrap mismatch");
    ticks(world,119);check(world.total_population()==0 && world.clay_extracted_total()==0,"Production appeared before any immigration");
    world.tick();check(world.total_population()==8 && world.original_production_state()->residents_admitted==8 &&
        world.clay_extracted_total()==0,"First due road-connected immigration or pre-tick staffing mismatch");
    for (const auto& h:world.original_economy_state()->houses)
        check(h.house.type==orig::HouseType::Shelter && h.house.occupants==2 && h.original_status==3 &&
            h.access_enabled && !h.house.water && !h.house.ceramics,"Authored move-in fabricated needs or goods");
    ticks(world,40);
    auto unreachable=world.snapshot();
    --unreachable.original_economy->houses.front().house.occupants;
    --unreachable.original_production->residents_admitted;
    refuses([&]{(void)sim::World::restore_original_production_development(unreachable,fixture.policy,fixture.model);});
    ticks(world,40);check(world.total_population()==24 && world.workforce_supply()==24 && world.workforce_used()==12,
        "Resident-derived workforce did not cap at the prepared Shelter capacity");
    Fixture odd_capacity(7);auto odd=odd_capacity.world();(void)scene(odd);ticks(odd,240);
    check(odd.total_population()==28,"Odd model capacity lost its lawful final one-resident admission");
    check(sim::World::restore_original_production_development(odd.snapshot(),odd_capacity.policy,odd_capacity.model).snapshot()==odd.snapshot(),
        "Lawful odd final capacity failed full World restore");
    bool captured=false;std::optional<sim::World> restored;
    for (int i=0;i<1100;++i) {
        world.tick();conserved(world);
        for (const auto& c:world.couriers()) if (!captured && c.role==sim::CourierRole::Pottery && c.cargo>0 && c.edge_progress>0) {
            const auto snapshot=world.snapshot();restored.emplace(sim::World::restore_original_production_development(snapshot,fixture.policy,fixture.model));
            check(restored->snapshot()==snapshot,"World mid-courier restore changed full authority");captured=true;
        }
        if (restored && restored->ticks()<world.ticks()) {
            restored->tick();check(restored->snapshot()==world.snapshot(),"Real production/mid-courier replay diverged");
        }
    }
    check(captured && world.original_goods_balance(orig::Goods::Ceramics).stock>0 && world.pottery_completed_total()>0 &&
        world.clay_extracted_total()>0,"No complete paid Clay→Pottery→actual Warehouse arrival→Ceramics origin");
    check(world.treasury()==11182 && world.taxes_collected_total()==0 && world.maintenance_spent_total()==0 &&
        world.building(ids.warehouse).pottery_stock==0,"Revision14 finance changed or Warehouse double-booked Ceramics");
    const auto saved=world.snapshot();
    for (const auto type:{sim::CommandType::PlaceMarket,sim::CommandType::PlaceFarm,sim::CommandType::PlaceWell,
            sim::CommandType::PlaceHealthPost,sim::CommandType::PlaceServicePost,sim::CommandType::PlaceFireWatch}) {
        check(!world.execute({type,{20,12}}).accepted && world.snapshot()==saved,"Future producer/service gained premature authority");
    }
    check(!world.execute(sim::demolish_building(ids.pottery)).accepted && world.snapshot()==saved,
        "Producer demolition bypassed accepted history boundary");
    auto bad=saved;bad.original_production->development_revision=15;
    refuses([&]{(void)sim::World::restore_original_production_development(bad,fixture.policy,fixture.model);});
    bad=saved;++bad.original_goods->totals[1].produced;
    refuses([&]{(void)sim::World::restore_original_production_development(bad,fixture.policy,fixture.model);});
    bad=saved;bad.original_economy->houses.front().house.water=true;
    refuses([&]{(void)sim::World::restore_original_production_development(bad,fixture.policy,fixture.model);});
    bad=saved;bad.original_production->residents_admitted=0;
    refuses([&]{(void)sim::World::restore_original_production_development(bad,fixture.policy,fixture.model);});
    refuses([&]{(void)sim::World::restore(saved,fixture.policy);});
    refuses([&]{(void)sim::World::restore_original_goods_development(saved,fixture.policy,fixture.model);});
    refuses([&]{sim::World normal(fixture.policy,sim::RulesProfile::CityV16,10);});
}
void missing_staff_and_road() {
    Fixture fixture;
    for (const auto flags:{std::pair{false,true},std::pair{true,false}}) {
        auto world=fixture.world();(void)scene(world,flags.first,flags.second);ticks(world,1000);conserved(world);
        check(world.total_population()==0 && world.workforce_supply()==0 && world.clay_extracted_total()==0 &&
            world.pottery_completed_total()==0 && world.original_goods_balance(orig::Goods::Ceramics).stock==0,
            "Disconnected or absent House granted residents/workforce/Ceramics");
    }
    auto world=fixture.world();const auto ids=scene(world);
    f::buy(world,{sim::CommandType::RemoveRoad,{12,8}});ticks(world,1100);conserved(world);
    check(world.pottery_completed_total()>0 && world.original_goods_balance(orig::Goods::Ceramics).stock==0,
        "Disconnected Pottery courier fabricated a Warehouse arrival");
    f::buy(world,{sim::CommandType::PlaceRoad,{12,8}});ticks(world,500);conserved(world);
    check(world.original_goods_balance(orig::Goods::Ceramics).stock>0 && world.building(ids.warehouse).pottery_stock==0,
        "Paid road repair failed to recover actual Warehouse conversion");
}
void paused_and_fire() {
    Fixture fixture;auto world=fixture.world();const auto ids=scene(world);
    f::buy(world,sim::set_building_operation(ids.clay,false));ticks(world,400);conserved(world);
    check(world.total_population()==24 && world.clay_extracted_total()==0 && world.pottery_completed_total()==0,
        "Stopped staffed Clay source produced resources");
    f::buy(world,sim::set_building_operation(ids.clay,true));ticks(world,600);conserved(world);
    check(world.pottery_completed_total()>0,"Regular paid chain did not resume after operation control");
    // Start the incident through the ordinary placement-relative risk clock.
    // Keep all Couriers idle by pausing both producers before it begins.
    f::buy(world,sim::set_building_operation(ids.clay,false));
    f::buy(world,sim::set_building_operation(ids.pottery,false));
    while (world.ticks()<2000) world.tick();
    check(world.building_on_fire(ids.clay) && world.building_on_fire(ids.pottery),"Natural Fire incident did not occur");
    f::buy(world,sim::set_building_operation(ids.clay,true));
    f::buy(world,sim::set_building_operation(ids.pottery,true));
    const auto clay=world.clay_extracted_total(),pottery=world.pottery_completed_total();
    ticks(world,200);conserved(world);
    check(world.clay_extracted_total()==clay && world.pottery_completed_total()==pottery &&
        world.courier_dispatch_status(world.couriers().front().id).status==sim::CourierDispatchStatus::OnFire,
        "Burning source ignored actual staffing/dispatch Fire gates");
    auto restored=sim::World::restore_original_production_development(world.snapshot(),fixture.policy,fixture.model);
    for (int i=0;i<1000;++i) {world.tick();restored.tick();check(world.snapshot()==restored.snapshot(),"Fire save/recovery replay diverged");}
    conserved(world);check(!world.building_on_fire(ids.clay),"Natural Fire deadline did not expire");
}
}
int main() {
    try {immigration_and_paid_origin();missing_staff_and_road();paused_and_fire();
        std::cout<<"Rule10 revision14 real paid Clay/Pottery/Courier/Ceramics origin, immigration, stops, Fire and replay PASS\n";return 0;}
    catch (const std::exception& error) {std::cerr<<"Original production World FAIL: "<<error.what()<<'\n';return 1;}
}
