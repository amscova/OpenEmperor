#pragma once

// Finite QA player recipes. Construction and operation always pass through
// World commands; the controller derives its continuation from the real World.
#include "PaidProgressionStrategy.h"

#include <array>
#include <string_view>

namespace oe_economy_fire_strategy {
namespace q=oe_starter_fixture;
namespace paid=oe_paid_progression_strategy;
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
using Json=nlohmann::json;
using q::check;

inline constexpr std::array<std::string_view,11> variants{
    "supplied","direct","connected","food","supplied12","managed","staged","staggered12","lean6","lean12","compact6"};

struct Plan {
    sim::Cell origin;
    Json initial_commands=Json::array();
};

inline bool known_variant(std::string_view variant) {
    return std::find(variants.begin(),variants.end(),variant)!=variants.end();
}
inline sim::Cell at(sim::Cell origin,int x,int y) {return {origin.x+x,origin.y+y};}

// A refusal is evidence only when World itself rejects insufficient Funds.
// Other refusals expose a recipe/ground blocker and must not be bypassed.
inline Json ordinary(sim::World& world,sim::Command command,const char* name,
                     sim::Cell origin,const Json& trigger=nullptr) {
    const auto funds=world.treasury(),price=world.construction_cost(command.type);
    const auto sequence=world.command_sequence(),construction=world.construction_spent_total();
    const auto taxes=world.taxes_collected_total(),maintenance=world.maintenance_spent_total(),tick=world.ticks();
    const auto validated=world.validate(command);
    Json row{{"command",name},{"type",static_cast<unsigned>(command.type)},{"tick",tick},
        {"cell",{command.cell.x,command.cell.y}},
        {"relative_cell",{command.cell.x-origin.x,command.cell.y-origin.y}},
        {"funds_before",funds},{"quoted_cost",price},{"sequence_before",sequence}};
    if(command.building_id!=static_cast<sim::BuildingId>(0))
        row["selected_building_id"]=static_cast<unsigned>(command.building_id);
    if(!trigger.is_null())row["trigger"]=trigger;
    if(!validated.accepted) {
        check(validated.diagnostic.blocker==sim::BuildBlocker::InsufficientFunds,
            "OE35 ordinary recipe refused at ("+std::to_string(command.cell.x)+","+
            std::to_string(command.cell.y)+"): "+validated.reason);
        const auto before=world.snapshot();const auto actual=world.execute(command);
        check(!actual.accepted&&!actual.changed&&actual.reason==validated.reason&&world.snapshot()==before,
            "OE35 insufficient-Funds refusal partially changed World authority");
        row["accepted"]=false;row["changed"]=false;row["reason"]=actual.reason;
        row["funds_after"]=world.treasury();row["cost"]=0;row["sequence_after"]=world.command_sequence();
        row["complete_snapshot_unchanged"]=true;return row;
    }
    check(validated.changed,"OE35 player recipe attempted a duplicate/no-op command");
    const auto actual=world.execute(command);
    check(actual.accepted&&actual.changed,"OE35 validated ordinary command refused: "+actual.reason);
    check(price>=0&&world.command_sequence()==sequence+1&&world.ticks()==tick&&
        world.treasury()==funds-price&&world.construction_spent_total()==construction+static_cast<std::uint64_t>(price)&&
        world.taxes_collected_total()==taxes&&world.maintenance_spent_total()==maintenance,
        "OE35 ordinary command bypassed actual funds, sequence or accounting");
    q::authority(world);row["accepted"]=true;row["changed"]=true;row["reason"]=actual.reason;
    row["funds_after"]=world.treasury();row["cost"]=price;row["sequence_after"]=world.command_sequence();
    if(const auto owner=world.building_owner_at(command.cell))row["building_id"]=static_cast<unsigned>(*owner);
    if(command.type==sim::CommandType::SetBuildingOperation)row["operating_enabled"]=command.operating_enabled;
    if(command.type==sim::CommandType::SetBuildingWorkforcePriority)
        row["workforce_priority"]=static_cast<unsigned>(command.workforce_priority);
    return row;
}

inline Json buy(sim::World& world,sim::Command command,const char* name,sim::Cell origin,
                const Json& trigger=nullptr) {
    command.cell=at(origin,command.cell.x,command.cell.y);
    return ordinary(world,command,name,origin,trigger);
}
inline void initial_buy(Plan& plan,sim::World& world,sim::Command command,const char* name) {
    auto row=buy(world,command,name,plan.origin);const bool accepted=row.at("accepted").get<bool>();
    plan.initial_commands.push_back(std::move(row));check(accepted,"OE35 fresh initial recipe exhausted actual Funds");
}

inline Plan build(sim::World& world,std::string_view variant,sim::Cell origin) {
    check(known_variant(variant),"Unknown OE35 finite player recipe: "+std::string(variant));
    check(world.original_production_enabled()&&world.ticks()==0&&world.command_sequence()==0&&
        world.buildings().empty()&&world.treasury()==sim::original_production_starting_treasury,
        "OE35 build requires a fresh unpaid Rule10 World");
    Plan plan{origin};
    if(variant=="lean6"||variant=="lean12"||variant=="compact6") {
        for(int y:{4,9})for(int x=0;x<24;++x)initial_buy(plan,world,{sim::CommandType::PlaceRoad,{x,y}},"Road");
        for(int y=5;y<9;++y)initial_buy(plan,world,{sim::CommandType::PlaceRoad,{20,y}},"Road");
        for(int x:{5,8,11,14})for(int y=0;y<4;++y)initial_buy(plan,world,{sim::CommandType::PlaceRoad,{x,y}},"Road");
        for(int y=5;y<9;++y)initial_buy(plan,world,{sim::CommandType::PlaceRoad,{2,y}},"Road");
        for(int x:{0,3,6,9,12,15})initial_buy(plan,world,{sim::CommandType::PlaceHousehold,{x,2}},"Household");
        if(variant=="lean12")for(int x:{0,3,6,9,12,15})
            initial_buy(plan,world,{sim::CommandType::PlaceHousehold,{x,7}},"Household");
        initial_buy(plan,world,{sim::CommandType::PlaceOriginalWaterWell,
            variant=="compact6" ? sim::Cell{9,0}:sim::Cell{4,5}},"WaterWell");
        initial_buy(plan,world,{sim::CommandType::PlaceOriginalAncestorShrine,
            variant=="compact6" ? sim::Cell{12,0}:sim::Cell{6,5}},"AncestorShrine");
        initial_buy(plan,world,{sim::CommandType::PlaceMarket,
            variant=="compact6" ? sim::Cell{4,10}:sim::Cell{9,5}},"Market");
        initial_buy(plan,world,{sim::CommandType::PlaceOriginalFoodFarm,{0,10}},"FoodFarm");
        const auto farm=world.building_owner_at(at(origin,0,10));check(farm.has_value(),"OE35 lean Farm lost its paid owner");
        sim::Command field{sim::CommandType::PlaceOriginalMilletField,{3,10}};field.building_id=*farm;
        initial_buy(plan,world,field,"MilletField");q::authority(world);return plan;
    }
    const auto funds=world.treasury();
    const auto construction=world.construction_spent_total(),sequence=world.command_sequence();
    const auto receipt=sim::place_original_economy_starter(world);
    check(receipt.origin==origin,"OE35 paid starter selected another legal origin");
    check(world.command_sequence()==sequence+receipt.commands_executed&&
        world.construction_spent_total()==construction+receipt.construction_spent&&
        world.treasury()==funds-static_cast<std::int64_t>(receipt.construction_spent),
        "OE35 production starter receipt does not reconcile its ordinary commands");
    plan.initial_commands.push_back({{"command","PaidFourHouseStarter"},{"tick",world.ticks()},
        {"origin",{origin.x,origin.y}},{"accepted",true},{"changed",true},
        {"command_count",receipt.commands_executed},{"funds_before",funds},{"funds_after",world.treasury()},
        {"cost",receipt.construction_spent},{"sequence_before",sequence},{"sequence_after",world.command_sequence()},
        {"source","simulation::place_original_economy_starter; normal World commands"}});
    const std::string expansion=variant=="staged" ? "connected":
        variant=="direct"||variant=="connected"||variant=="food" ? std::string(variant):"supplied";
    for(auto& row:paid::expand(world,receipt,expansion)) {
        row["accepted"]=true;row["changed"]=true;plan.initial_commands.push_back(std::move(row));
    }
    if(variant=="supplied12")for(int x:{0,3,6,9,12,15})
        initial_buy(plan,world,{sim::CommandType::PlaceHousehold,{x,7}},"Household");
    q::authority(world);return plan;
}

inline void complete_farm(sim::World& world,Json& commands,sim::Cell origin,bool food,const Json& trigger) {
    const sim::Cell farm_cell=food ? sim::Cell{9,10}:sim::Cell{14,10};
    const auto kind=food ? sim::Object::OriginalFoodFarm:sim::Object::OriginalHempFarm;
    auto owner=world.building_owner_at(at(origin,farm_cell.x,farm_cell.y));
    if(!owner) {
        auto row=buy(world,{food ? sim::CommandType::PlaceOriginalFoodFarm:sim::CommandType::PlaceOriginalHempFarm,farm_cell},
            food ? "FoodFarm":"HempFarm",origin,trigger);
        const bool accepted=row.at("accepted").get<bool>();commands.push_back(std::move(row));if(!accepted)return;
        owner=world.building_owner_at(at(origin,farm_cell.x,farm_cell.y));
    }
    check(owner&&world.building(*owner).kind==kind,"OE35 staged Farm coordinate has another paid owner");
    const auto has_field=[&] {
        if(food)for(const auto& farm:world.original_production_state()->food.farms)if(farm.farm_id==*owner)return farm.field_id.has_value();
        if(!food)for(const auto& farm:world.original_production_state()->hemp_farms)if(farm.farm_id==*owner)return farm.field_id.has_value();
        throw std::runtime_error("OE35 paid Farm has no productive Farm identity");
    };
    if(has_field())return;
    sim::Command field{food ? sim::CommandType::PlaceOriginalMilletField:sim::CommandType::PlaceOriginalHempField,
        food ? sim::Cell{12,10}:sim::Cell{16,10}};
    field.building_id=*owner;commands.push_back(buy(world,field,food ? "MilletField":"HempField",origin,trigger));
}

inline Json controls(sim::World& world,std::string_view variant,sim::Cell origin) {
    check(known_variant(variant)&&world.original_production_enabled(),"OE35 controls require a known Rule10 recipe");
    Json commands=Json::array();
    if(variant=="managed") {
        for(const auto& b:world.buildings()) {
            const bool essential=b.kind==sim::Object::OriginalWaterWell72||b.kind==sim::Object::OriginalAncestorShrine214||
                b.kind==sim::Object::OriginalFoodFarm||b.kind==sim::Object::Market;
            if(essential&&b.workforce_priority!=sim::WorkforcePriority::High) {
                auto command=sim::set_building_workforce_priority(b.id,sim::WorkforcePriority::High);command.cell=b.cell;
                commands.push_back(ordinary(world,command,"SetBuildingWorkforcePriority",origin,
                    {{"reason","Prioritize paid Water, Ancestor, Food and Market during staffing scarcity"}}));
            }
            const bool delayed=b.kind==sim::Object::ClaySource||b.kind==sim::Object::Pottery||b.kind==sim::Object::Warehouse||
                b.kind==sim::Object::OriginalHempFarm||b.kind==sim::Object::HealthPost||b.kind==sim::Object::ServicePost||
                b.kind==sim::Object::OriginalMusicSchool211;
            const bool enabled=world.ticks()>=1200;
            if(delayed&&b.operating_enabled!=enabled) {
                auto command=sim::set_building_operation(b.id,enabled);command.cell=b.cell;
                commands.push_back(ordinary(world,command,"SetBuildingOperation",origin,
                    {{"reason",enabled ? "Resume paid deferred providers at tick1200":"Defer paid providers until tick1200"},
                     {"installed_upkeep_unchanged",true}}));
            }
        }
    }
    if(variant=="staggered12"&&world.ticks()>=816) {
        for(int x:{0,3,6,9,12,15}) {
            const auto scheduled=x<=6 ? std::uint64_t{816}:std::uint64_t{1632};
            if(world.ticks()<scheduled)continue;
            const auto cell=at(origin,x,7);const auto owner=world.building_owner_at(cell);
            if(owner) {check(sim::original_house_kind(world.building(*owner).kind),"OE35 staggered House plot has another owner");continue;}
            commands.push_back(buy(world,{sim::CommandType::PlaceHousehold,{x,7}},"Household",origin,
                {{"reason","Paid staggered household expansion"},{"scheduled_tick",scheduled}}));
        }
    }
    if(variant=="staged"&&world.ticks()>=816) {
        std::uint64_t misses=0;
        for(const auto& row:world.original_production_state()->food.stocks)
            if(sim::original_house_kind(world.building(row.building_id).kind)) {
                check(row.missed_demands<=UINT64_MAX-misses,"OE35 observed Food-miss count overflow");misses+=row.missed_demands;
            }
        if(misses)complete_farm(world,commands,origin,true,
            {{"reason","Observed actual household Food consumption miss"},{"actual_food_missed_demands",misses}});
        Json hemp_trigger=nullptr;const auto& rules=world.original_rules_model()->house_rules();
        for(const auto& row:world.original_production_state()->housing.houses)if(row.last_evaluation) {
            const auto& evaluation=*row.last_evaluation;const auto& input=evaluation.input;
            const auto required=rules.value(orig::house_row(input.type),orig::HouseField::Hemp);
            if(input.occupants>0&&required>input.hemp) {
                hemp_trigger={{"reason","Observed actual current-row Hemp shortage"},
                    {"house",static_cast<unsigned>(row.building_id)},{"evaluation_tick",evaluation.tick},
                    {"type",static_cast<unsigned>(input.type)},{"required",required},{"actual",input.hemp}};break;
            }
        }
        if(!hemp_trigger.is_null())complete_farm(world,commands,origin,false,hemp_trigger);
    }
    return commands;
}
} // namespace oe_economy_fire_strategy
