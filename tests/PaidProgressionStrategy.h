#pragma once

// Shared QA player strategy: ordinary paid commands and the production goal
// query only. This header is never linked into World or the normal application.
#include "OriginalStarterFixture.h"

namespace oe_paid_progression_strategy {
namespace q=oe_starter_fixture;
namespace sim=openemperor::simulation;
using Json=nlohmann::json;
using q::check;

inline Json goal(const sim::World& w) {
    const auto g=w.original_goal_status();
    return {{"reached",g.reached},{"inhabited_houses",g.inhabited_houses},{"common_plain_or_higher",g.common_houses},
        {"population",g.population},{"positive_tax",g.positive_tax},{"nonnegative_funds",g.nonnegative_funds},
        {"tick",w.ticks()},{"funds",w.treasury()},{"taxes",w.taxes_collected_total()},
        {"maintenance",w.maintenance_spent_total()},{"construction",w.construction_spent_total()}};
}
inline Json fire_refusal(sim::World& w,sim::Cell origin) {
    const sim::Cell cell{origin.x,origin.y+5};
    check(w.buildable(cell)&&w.object_at(cell)==sim::Object::Empty&&w.treasury()>=80,"FireWatch control lacks legal funded spare ground");
    const auto before=w.snapshot();const sim::Command command{sim::CommandType::PlaceFireWatch,cell};
    const auto valid=w.validate(command),actual=w.execute(command);
    const std::string reason="Original production permits only its explicitly implemented paid chains";
    check(!valid.accepted&&!actual.accepted&&!actual.changed&&valid.reason==reason&&actual.reason==reason&&w.snapshot()==before,
        "Rule10 FireWatch refusal or atomicity changed; review its versioned contract");
    return {{"tick",w.ticks()},{"cell",{cell.x,cell.y}},{"reason",actual.reason},{"complete_snapshot_unchanged",true}};
}
inline Json expand(sim::World& w,const sim::OriginalEconomyStarterReceipt& receipt,const std::string& plan) {
    Json commands=Json::array();
    const auto buy=[&](sim::Command command,const char* name) {
        const auto relative=command.cell;command.cell=q::offset(receipt,relative.x,relative.y);
        const auto before=w.treasury();const auto sequence=w.command_sequence();const auto paid=w.construction_spent_total();
        const auto validated=w.validate(command);check(validated.accepted,"Paid plan validation refused: "+validated.reason);
        const auto result=w.execute(command);check(result.accepted&&result.changed,"Paid plan command refused: "+result.reason);
        check(w.command_sequence()==sequence+1&&w.treasury()>=0&&w.treasury()<=before&&
            w.construction_spent_total()-paid==static_cast<std::uint64_t>(before-w.treasury()),"Paid plan bypassed actual funds/accounting");
        q::authority(w);Json row{{"command",name},{"type",static_cast<unsigned>(command.type)},{"tick",w.ticks()},
            {"relative_cell",{relative.x,relative.y}},{"cell",{command.cell.x,command.cell.y}},
            {"funds_before",before},{"funds_after",w.treasury()},{"cost",before-w.treasury()}};
        if(const auto id=w.building_owner_at(command.cell))row["building_id"]=static_cast<unsigned>(*id);
        commands.push_back(std::move(row));
        return w.building_owner_at(command.cell);
    };
    buy({sim::CommandType::PlaceHousehold,{12,2}},"Household");
    buy({sim::CommandType::PlaceHousehold,{15,2}},"Household");
    if(plan!="direct")for(int y=5;y<=8;++y)buy({sim::CommandType::PlaceRoad,{2,y}},"Road");
    if(plan=="food"||plan=="supplied") {
        const auto farm=buy({sim::CommandType::PlaceOriginalFoodFarm,{9,10}},"FoodFarm");check(farm.has_value(),"Paid Farm owner missing");
        sim::Command field{sim::CommandType::PlaceOriginalMilletField,{12,10}};field.building_id=*farm;buy(field,"MilletField");
    }
    if(plan=="supplied") {
        const auto farm=buy({sim::CommandType::PlaceOriginalHempFarm,{14,10}},"HempFarm");check(farm.has_value(),"Paid Hemp Farm owner missing");
        sim::Command field{sim::CommandType::PlaceOriginalHempField,{16,10}};field.building_id=*farm;buy(field,"HempField");
    }
    return commands;
}

} // namespace oe_paid_progression_strategy
