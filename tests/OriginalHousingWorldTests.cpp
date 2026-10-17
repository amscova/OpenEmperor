#include "OriginalProductionFixture.h"
#include "simulation/original/HouseEvolution.h"
#include "simulation/original/OriginalScheduler.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <string_view>

namespace {
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
using f::check;

// Small explicitly authored requirement changes. Real-model tests otherwise
// preserve every original model byte, including prices and all house rows.
void field(std::string& text,int row,orig::HouseField name,int value) {
    constexpr std::array<std::string_view,3> labels{"1: Shelter,{,","2: Hut,{,","3: Plain Cottage,{,"};
    const std::string marker(labels.at(static_cast<std::size_t>(row)));
    const auto at=text.find(marker);check(at!=std::string::npos,"Housing fixture model row missing");
    auto start=at+marker.size();
    for (std::size_t i=0;i<static_cast<std::size_t>(name);++i) {
        start=text.find(',',start);check(start!=std::string::npos,"Housing fixture field missing");++start;
    }
    const auto end=text.find(',',start);check(end!=std::string::npos,"Housing fixture value missing");
    text.replace(start,end-start,std::to_string(value));
}
struct Fixture {
    f::Root root;
    std::shared_ptr<const sim::MapPermissions> policy=f::permissions();
    std::shared_ptr<const orig::OriginalRulesModel> model;
    explicit Fixture(const f::fs::path& original={},int variant=0) {
        std::string bytes;
        if (!original.empty()) bytes=f::read_text(original/"Model/EmperorBuildingModels.txt");
        else {
            bytes=oe_production_fixture::authored_model_text(7);
            // Authored early housing scenario; these are deliberate inputs,
            // not a second implementation of the full original model table.
            for (int row=0;row<3;++row) {
                field(bytes,row,orig::HouseField::DesirabilityLow,-99);
                field(bytes,row,orig::HouseField::DesirabilityHigh,row==2 ? 99:-5);
                field(bytes,row,orig::HouseField::Capacity,7+7*row);
                field(bytes,row,orig::HouseField::Water,row ? 1:0);
                field(bytes,row,orig::HouseField::FoodQuality,row==2 ? 20:0);
                field(bytes,row,orig::HouseField::Ancestor,row==2 ? 1:0);
            }
        }
        if (variant==1) field(bytes,2,orig::HouseField::Hemp,1);
        if (variant==2) field(bytes,1,orig::HouseField::DesirabilityHigh,99);
        f::write_text(root.data/"Model/EmperorBuildingModels.txt",bytes);
        model=f::save::prepare_original_production_rules_model(root.data);
    }
    sim::World world() const {return sim::World::create_original_production_development(policy,model);}
    sim::World restore(const sim::World& world) const {
        return sim::World::restore_original_production_development(world.snapshot(),policy,model);
    }
};
const orig::HouseState& house(const sim::World& world,sim::BuildingId id) {
    for (const auto& h:world.original_economy_state()->houses) if (h.building_id==id) return h.house;
    throw std::runtime_error("Housing test lost paid House");
}
const sim::OriginalHousingHouse& history(const sim::World& world,sim::BuildingId id) {
    for (const auto& h:world.original_production_state()->housing.houses) if (h.building_id==id) return h;
    throw std::runtime_error("Housing test lost compact House history");
}
sim::BuildingId buy(sim::World& world,sim::CommandType command,sim::Cell cell) {
    f::buy(world,{command,cell});return *world.building_owner_at(cell);
}
void authority(const sim::World& world) {
    check(world.production_balance_valid()&&world.population_valid()&&world.navigation_valid()&&world.city_economy_valid(),
          "Housing actual World invariant failed");
    const auto& production=*world.original_production_state();
    check(production.housing.houses.size()==world.original_economy_state()->houses.size(),"Housing history identity count mismatch");
    std::uint64_t people=0;
    for (const auto& row:world.original_economy_state()->houses) {
        const auto& h=row.house;const auto& audit=history(world,row.building_id);people+=static_cast<std::uint64_t>(h.occupants);
        check(h.type>=orig::HouseType::Vacant&&h.type<=orig::HouseType::PlainCottage&&h.devolve_counter<=9,
              "Early housing escaped its authored type/counter boundary");
        check(bool(audit.first_admission_tick)==bool(h.occupants),"Housing lifetime admission did not originate in real residents");
        if (h.type>=orig::HouseType::Hut) check(audit.first_hut_evaluation.has_value(),"Hut has no first actual upgrade input");
        if (h.type>=orig::HouseType::PlainCottage) check(audit.first_plain_evaluation.has_value(),"Plain has no first actual upgrade input");
        if (audit.last_evaluation) check(audit.last_evaluation->tick==sim::original_housing_evaluation_tick(world.ticks())&&
            audit.last_evaluation->tick%51==40,"Housing evaluated outside actual phase39");
    }
    check(people==production.residents_admitted&&people==static_cast<std::uint64_t>(world.workforce_supply()),
          "Housing population/workforce duplicated or granted residents");
    for (const auto& b:world.buildings()) check(!b.population&&!b.service_until_tick&&!b.health_protection_until_tick,
          "Housing changed legacy population or service authorities");
}
template<class Predicate> void until(sim::World& world,Predicate done,const char* failure,int limit=20000) {
    for (int i=0;i<limit&&!done();++i) {world.tick();authority(world);}
    check(done(),failure);
}
void ticks(sim::World& world,int count) {for (int i=0;i<count;++i) {world.tick();authority(world);}}
struct Scene {sim::BuildingId home{},farm{},market{},well{},shrine{};};
Scene minimal(sim::World& world,int missing=0) {
    for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,4}});
    Scene s;
    for (int x=0;x<15;x+=3) {const auto id=buy(world,sim::CommandType::PlaceHousehold,{x,2});if (!x)s.home=id;}
    s.farm=buy(world,sim::CommandType::PlaceOriginalFoodFarm,{0,5});
    if (missing!=3) {sim::Command crop{sim::CommandType::PlaceOriginalWheatField,{3,5}};crop.building_id=s.farm;f::buy(world,crop);}
    s.market=buy(world,sim::CommandType::PlaceMarket,{5,missing==4 ? 9:5});
    if (missing!=1) s.well=buy(world,sim::CommandType::PlaceOriginalWaterWell,{9,5});
    else (void)buy(world,sim::CommandType::PlaceOriginalInfluenceSource,{9,5});
    if (missing!=2) s.shrine=buy(world,sim::CommandType::PlaceOriginalAncestorShrine,{13,5});
    return s;
}
void verify_first_upgrade(const sim::World& world,sim::BuildingId id) {
    const auto& h=house(world,id);const auto& audit=history(world,id);
    check(h.type==orig::HouseType::PlainCottage&&audit.first_hut_evaluation&&audit.first_plain_evaluation,
          "Paid House did not actually evolve Shelter->Hut->Plain");
    const auto& hut=*audit.first_hut_evaluation;const auto& plain=*audit.first_plain_evaluation;
    check(hut.tick<plain.tick&&hut.input.type==orig::HouseType::Shelter&&plain.input.type==orig::HouseType::Hut&&
          hut.input.water&&plain.input.water&&plain.input.food_quality>=20&&plain.input.ancestor>0&&
          hut.water_evaluation_witness&&plain.water_evaluation_witness&&plain.food_quality_witness&&plain.service_witnesses[2],
          "First upgrade lacks genuine water/Food/Ancestor historical input");
    auto evolved=orig::evolve_step(world.original_rules_model()->house_rules(),plain.input);
    check(evolved.type==orig::HouseType::PlainCottage&&evolved.devolve_counter==0,
          "First Plain input disagrees with unchanged pure HouseEvolution");
}
void paid_first_upgrade_and_devolution(const f::fs::path& original) {
    Fixture fixture(original);auto world=fixture.world();const auto scene=minimal(world);
    const auto expected=5*sim::Rules::household_cost+24*sim::Rules::road_cost+sim::Rules::market_cost+
        fixture.model->building_value(orig::ModelBuilding::Farmhouse,orig::BuildingField::Cost)+
        fixture.model->building_value(orig::ModelBuilding::WheatField,orig::BuildingField::Cost)+
        fixture.model->building_value(orig::ModelBuilding::Well,orig::BuildingField::Cost)+
        fixture.model->building_value(orig::ModelBuilding::AncestralShrine,orig::BuildingField::Cost);
    check(!world.total_population()&&world.construction_spent_total()==static_cast<std::uint64_t>(expected)&&
          world.treasury()==12000-expected,"First-Upgrade scene did not pay its normal construction commands");
    if (!original.empty()) check(expected==708,"Actual-model first-upgrade must cost708: Houses400 + Roads48 + Market140 + Farm"+
        std::to_string(fixture.model->building_value(orig::ModelBuilding::Farmhouse,orig::BuildingField::Cost))+" + Field"+
        std::to_string(fixture.model->building_value(orig::ModelBuilding::WheatField,orig::BuildingField::Cost))+" + Well"+
        std::to_string(fixture.model->building_value(orig::ModelBuilding::Well,orig::BuildingField::Cost))+" + Shrine"+
        std::to_string(fixture.model->building_value(orig::ModelBuilding::AncestralShrine,orig::BuildingField::Cost))+" = "+std::to_string(expected));
    ticks(world,119);check(!world.total_population()&&!history(world,scene.home).first_admission_tick,
          "Housing manufacture preceded first regular immigration");
    until(world,[&]{return std::any_of(world.couriers().begin(),world.couriers().end(),[&](const auto& c){
        return c.owner==scene.well&&c.phase==sim::CourierPhase::ToWarehouse&&c.edge_progress>0;});},"Paid water actor never moved");
    auto replay=fixture.restore(world);
    for (int i=0;i<20000&&house(world,scene.home).type!=orig::HouseType::PlainCottage;++i) {
        world.tick();replay.tick();authority(world);check(world.snapshot()==replay.snapshot(),"Mid-water F5/F9 changed actual first upgrade");
    }
    verify_first_upgrade(world,scene.home);
    check(world.workforce_supply()>0&&world.workforce_used()<=world.workforce_supply(),"Upgrade lacks genuine resident workforce");
    // Ordinary source pause drains actual stocks and prevents new visits. An
    // already travelling direct service completes normally before expiry.
    for (const auto source:{scene.farm,scene.market,scene.well,scene.shrine}) f::buy(world,sim::set_building_operation(source,false));
    bool counter9=false,devolved=false;std::optional<int> retained_people;
    for (int i=0;i<20000&&!devolved;++i) {
        world.tick();authority(world);const auto& audit=history(world,scene.home);
        if (!audit.last_evaluation||audit.last_evaluation->tick!=world.ticks()) continue;
        auto expected_house=orig::evolve_step(fixture.model->house_rules(),audit.last_evaluation->input);
        if (expected_house.type>orig::HouseType::PlainCottage) expected_house.type=orig::HouseType::PlainCottage;
        check(house(world,scene.home).type==expected_house.type&&house(world,scene.home).devolve_counter==expected_house.devolve_counter,
              "Productive type/counter differs from actual pure phase39 evaluation");
        counter9|=audit.last_evaluation->input.type==orig::HouseType::PlainCottage&&
            audit.last_evaluation->input.devolve_counter==8&&expected_house.devolve_counter==9;
        if (audit.last_evaluation->input.type==orig::HouseType::PlainCottage&&expected_house.type==orig::HouseType::Hut) {
            check(counter9&&audit.last_evaluation->input.devolve_counter==9&&expected_house.devolve_counter==0,
                  "Devolution did not occur at the actual tenth failed evaluation");
            check(house(world,scene.home).occupants==audit.last_evaluation->input.occupants,
                  "Devolution invented unsupported resident eviction");
            retained_people=house(world,scene.home).occupants;devolved=true;
            check(*retained_people>fixture.model->house_rules().row_capacity(orig::ModelHouse::Hut),
                  "Devolution test did not actually retain residents above the current Hut capacity");
        }
    }
    check(counter9&&devolved&&retained_people,"Paused actual supply did not cause bounded real devolution");
    replay=fixture.restore(world);
    for (int i=0;i<20000;++i) {
        world.tick();replay.tick();authority(world);check(world.snapshot()==replay.snapshot(),"20k actual devolution F5/F9 replay diverged");
        check(house(world,scene.home).occupants==*retained_people,
              "Later devolution evicted residents or admitted more people above the current capacity");
    }
}
void missing_requirements_and_authored_counterexamples(const f::fs::path& original) {
    for (int missing=1;missing<=4;++missing) {
        Fixture fixture(original);auto world=fixture.world();const auto s=minimal(world,missing);
        ticks(world,1700);
        for (const auto& h:world.original_economy_state()->houses) {
            check(h.house.type!=orig::HouseType::PlainCottage&&!history(world,h.building_id).first_plain_evaluation,
                  "Missing Water/Ancestor/Food/unreachableMarket permitted Plain");
            if (missing==1) check(h.house.type==orig::HouseType::Shelter&&!h.house.water,
                  "Historical influence-only source granted water or Hut");
            if (missing==2) check(!h.house.ancestor,"Absent religion source manufactured Ancestor");
            if (missing>=3) check(!h.house.food_quality,"Absent Field/unreachable Market manufactured Food quality");
        }
        check(world.total_population()>0&&world.construction_spent_total()>0,"Negative case bypassed paid populated World");
        if (missing!=1) check(house(world,s.home).water,"Negative prerequisite scene never obtained genuine Water");
    }
    for (const int authored_variant:{1,2}) {
        Fixture fixture(original,authored_variant);auto world=fixture.world();const auto s=minimal(world);
        until(world,[&]{return house(world,s.home).water&&house(world,s.home).food_quality>=20&&house(world,s.home).ancestor>0;},
              "Authored counterexample did not obtain real other prerequisites",1700);
        ticks(world,150);
        check(house(world,s.home).type==orig::HouseType::Hut&&!history(world,s.home).first_plain_evaluation,
              "Authored missingHempNeed1/insufficientDes counterexample evolved");
        if (authored_variant==1) check(!house(world,s.home).hemp&&
            fixture.model->house_rules().value(orig::ModelHouse::PlainCottage,orig::HouseField::Hemp)==1,
            "Authored missing raw-material test did not require exactly one real Hemp");
        else check(world.original_influence_provenance(s.home).value<
            fixture.model->house_rules().value(orig::ModelHouse::Hut,orig::HouseField::DesirabilityHigh),
            "Authored Desirability counterexample was not actually below threshold");
    }
}
void complete_paid_goods_food_and_service_integration(const f::fs::path& original) {
    Fixture fixture(original);auto world=fixture.world();
    for (const int y:{5,8,12}) for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,y}});
    for (const int y:{6,7,9,10,11}) f::buy(world,{sim::CommandType::PlaceRoad,{23,y}});
    for (const int y:{3,6}) for (int x=0;x<18;x+=3) (void)buy(world,sim::CommandType::PlaceHousehold,{x,y});
    (void)buy(world,sim::CommandType::PlaceClaySource,{0,9});
    const auto hemp=buy(world,sim::CommandType::PlaceOriginalHempFarm,{4,9});
    sim::Command crop{sim::CommandType::PlaceOriginalHempField,{6,9}};crop.building_id=hemp;f::buy(world,crop);
    (void)buy(world,sim::CommandType::PlacePottery,{8,9});(void)buy(world,sim::CommandType::PlaceWarehouse,{16,9});
    (void)buy(world,sim::CommandType::PlaceMarket,{21,9});
    const auto food=buy(world,sim::CommandType::PlaceOriginalFoodFarm,{0,13});
    crop={sim::CommandType::PlaceOriginalWheatField,{3,13}};crop.building_id=food;f::buy(world,crop);
    (void)buy(world,sim::CommandType::PlaceHealthPost,{5,13});(void)buy(world,sim::CommandType::PlaceServicePost,{8,13});
    (void)buy(world,sim::CommandType::PlaceOriginalAncestorShrine,{11,13});(void)buy(world,sim::CommandType::PlaceOriginalWaterWell,{14,13});
    check(!world.total_population()&&world.treasury()<12000,"Grand integration began with free people or unpaid bodies");
    if (!original.empty()) check(world.construction_spent_total()==1948,"Real GrandIntegration construction must cost1948; actual="+
        std::to_string(world.construction_spent_total()));
    until(world,[&]{return std::any_of(world.original_economy_state()->houses.begin(),world.original_economy_state()->houses.end(),
        [&](const auto& h){return h.house.type==orig::HouseType::PlainCottage&&h.house.hemp>0&&h.house.ceramics>0&&
            h.house.food_quality>=20&&h.house.herbalist>0&&h.house.acupuncture>0&&h.house.ancestor>0&&h.house.water;});},
        "Paid full chain never combined actual goods/Food/medical/Ancestor/Water arrivals with real Plain",10000);
    check(world.original_goods_balance(orig::Goods::Hemp).totals.produced>0&&
          world.original_goods_balance(orig::Goods::Ceramics).totals.produced>0,
          "GrandIntegration satisfied goods without real paid harvest and recipe completion");
    authority(world);
}
}
int main(int argc,char** argv) {
    try {
        check(argc==1||(argc==3&&std::string_view(argv[1])=="--real"),"Usage: original-housing-world [--real DATA]");
        const auto original=argc==3 ? f::fs::canonical(argv[2]):f::fs::path{};
        paid_first_upgrade_and_devolution(original);
        missing_requirements_and_authored_counterexamples(original);
        complete_paid_goods_food_and_service_integration(original);
        std::cout<<"Paid early housing, actual Water/Food/Ancestor upgrades, authored counterexamples, full goods integration and20k devolution replay PASS ("
                 <<(original.empty()?"authored model":"original model plus explicitly authored negative copies")<<")\n";
        return 0;
    } catch (const std::exception& e) {std::cerr<<"Original Housing World FAIL: "<<e.what()<<'\n';return 1;}
}
