#pragma once

// Finite, reviewable player recipes. Every mutation uses the production World.
#include "EconomyFireDiagnosisStrategy.h"

namespace oe_sustainable_strategy {
namespace q=oe_starter_fixture;
namespace sim=openemperor::simulation;
namespace prior=oe_economy_fire_strategy;
using Json=nlohmann::json;
using q::check;
inline constexpr std::array<std::string_view,7> variants{
    "supplied12","lean12","compact-two","three-food","compact-three","balanced-three","compact-connected-three"};
inline bool known(std::string_view name){return std::find(variants.begin(),variants.end(),name)!=variants.end();}
inline sim::Cell origin(const q::Context& c){auto w=c.world();return sim::place_original_economy_starter(w).origin;}

inline Json build(sim::World& world,std::string_view variant,sim::Cell at) {
    check(known(variant),"Unknown sustainable player recipe");
    auto commands=prior::build(world,variant=="supplied12"?"supplied12":"lean12",at).initial_commands;
    const auto buy=[&](sim::Command command,const char* label){
        auto row=prior::buy(world,command,label,at);check(row.at("accepted").get<bool>(),"Fresh sustainable recipe exhausted Funds");
        commands.push_back(row);return world.building_owner_at(prior::at(at,command.cell.x,command.cell.y));
    };
    if(variant!="supplied12") {
        buy({sim::CommandType::PlaceWarehouse,{18,5}},"Warehouse");
        if(variant!="lean12") {
            const auto farm=buy({sim::CommandType::PlaceOriginalFoodFarm,{9,10}},"FoodFarm");
            sim::Command field{sim::CommandType::PlaceOriginalMilletField,{12,10}};field.building_id=*farm;buy(field,"MilletField");
        }
        const auto hemp=buy({sim::CommandType::PlaceOriginalHempFarm,{5,10}},"HempFarm");
        sim::Command field{sim::CommandType::PlaceOriginalHempField,{7,10}};field.building_id=*hemp;buy(field,"HempField");
        if(variant=="three-food"||variant=="compact-three"||variant=="balanced-three"||variant=="compact-connected-three") {
            const auto farm=buy({sim::CommandType::PlaceOriginalFoodFarm,{18,10}},"FoodFarm");
            field={sim::CommandType::PlaceOriginalMilletField,{21,10}};field.building_id=*farm;buy(field,"MilletField");
        }
        if(variant=="balanced-three") {
            const auto farm=buy({sim::CommandType::PlaceOriginalHempFarm,{14,10}},"HempFarm");
            field={sim::CommandType::PlaceOriginalHempField,{16,10}};field.building_id=*farm;buy(field,"HempField");
        }
        if(variant=="compact-two"||variant=="compact-three"||variant=="compact-connected-three") {
            buy({sim::CommandType::PlaceMarket,{17,7}},"Market");
            buy({sim::CommandType::PlaceOriginalWaterWell,{9,0}},"WaterWell");
            buy({sim::CommandType::PlaceOriginalAncestorShrine,{6,0}},"AncestorShrine");
            if(variant=="compact-connected-three")buy({sim::CommandType::PlaceRoad,{17,8}},"Road");
        }
    }
    for(const auto relative:{sim::Cell{0,5},sim::Cell{19,7}})
        buy({sim::CommandType::PlaceFireWatch,relative},"FireWatch");
    q::authority(world);return commands;
}
inline Json maturity(const sim::World& w) {
    const auto food=q::food_facts(w);const auto& p=*w.original_production_state();
    const auto goal=oe_paid_progression_strategy::goal(w);std::uint64_t visits=0,water=0,goods=0;
    visits=p.services.witnesses.size();
    for(const auto& h:p.water.houses)if(h.current_witness)++water;
    for(unsigned g=0;g<openemperor::original::goods_count;++g)goods+=w.original_goods_balance(static_cast<openemperor::original::Goods>(g)).totals.consumed;
    const auto fire=p.fire_protection?p.fire_protection->arrivals.size():0;
    const bool ready=goal.at("reached").get<bool>()&&goal.at("inhabited_houses").get<unsigned>()>=12&&
        food.house_received>0&&goods>0&&visits>0&&water>0&&fire>0;
    return {{"ready",ready},{"tick",w.ticks()},{"goal",goal},{"inhabited_houses",goal.at("inhabited_houses")},
        {"food_house_received",food.house_received},{"goods_consumed",goods},{"actual_service_visits",visits},
        {"actual_water_visits",water},{"actual_inspector_arrivals",fire},
        {"definition","Fixed tick20000; actual World goal,12 occupied non-burning Houses, paid Food/goods/service/water/Inspector arrival witnesses. Shortages remain measured."}};
}
}
