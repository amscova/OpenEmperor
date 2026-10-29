#pragma once

// Authored finite player recipe on pinned Xia; coordinates are our plan,
// never an original fixture, source-manager grant or automatic game default.
#include "HighTierProgressionMetrics.h"
#include <array>
#include <map>
#include <string_view>

namespace oe_elite_desirability_strategy {
namespace q=oe_starter_fixture;
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
using Json=nlohmann::json;
using C=sim::CommandType;
inline constexpr const char* model_sha="7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6";
inline constexpr const char* map_sha="871266c415d7500ff3cf3fe70fc6b36caecc4d4cbf09d4d737849e4060906433";
inline constexpr sim::Cell elite_cell{130,133};
struct Planned {unsigned plan_id;C command;sim::Cell cell;unsigned owner;unsigned phase;bool duplicate;bool lean_omit;bool extra_market=false;};
inline constexpr std::array plan{
    Planned{1,C::PlaceOriginalInfluenceSource,{129,128},0,3,false,false},
    Planned{2,C::PlaceOriginalInfluenceSource,{127,128},0,3,false,false},
    Planned{3,C::PlaceOriginalInfluenceSource,{127,135},0,3,false,false},
    Planned{4,C::PlaceOriginalInfluenceSource,{125,133},0,3,false,false},
    Planned{5,C::PlaceOriginalWaterWell,{110,113},0,0,true,false},
    Planned{6,C::PlaceOriginalWaterWell,{125,135},0,0,false,false},
    Planned{7,C::PlaceHealthPost,{110,115},0,2,false,false},
    Planned{8,C::PlaceHealthPost,{110,117},0,3,true,true},
    Planned{9,C::PlaceServicePost,{110,120},0,2,false,false},
    Planned{10,C::PlaceServicePost,{110,122},0,3,true,true},
    Planned{11,C::PlaceOriginalMusicSchool,{130,137},0,3,true,false},
    Planned{12,C::PlaceOriginalMusicSchool,{129,130},0,2,false,false},
    Planned{13,C::PlaceOriginalAcrobatSchool,{110,130},0,3,true,false},
    Planned{14,C::PlaceOriginalAcrobatSchool,{110,124},0,2,false,false},
    Planned{15,C::PlaceOriginalDramaSchool,{110,127},0,3,true,false},
    Planned{16,C::PlaceOriginalDramaSchool,{124,130},0,2,false,false},
    Planned{17,C::PlaceOriginalAncestorShrine,{134,131},0,3,true,false},
    Planned{18,C::PlaceOriginalAncestorShrine,{125,137},0,0,false,false},
    Planned{19,C::PlaceOriginalDaoistShrine,{127,130},0,2,false,false},
    Planned{20,C::PlaceOriginalDaoistShrine,{131,129},0,3,true,false},
    Planned{21,C::PlaceOriginalBuddhistShrine,{127,133},0,2,false,false},
    Planned{22,C::PlaceOriginalBuddhistShrine,{132,131},0,3,true,false},
    Planned{23,C::PlaceOriginalConfucianAcademy,{124,127},0,3,false,false},
    Planned{24,C::PlaceOriginalConfucianAcademy,{133,128},0,3,true,false},
    Planned{101,C::PlaceOriginalFoodFarm,{114,128},0,0,false,false},
    Planned{102,C::PlaceOriginalMilletField,{117,128},101,0,false,false},
    Planned{103,C::PlaceOriginalFoodFarm,{114,125},0,1,false,false},
    Planned{104,C::PlaceOriginalMilletField,{117,125},103,1,false,false},
    Planned{105,C::PlaceOriginalFoodFarm,{114,131},0,2,false,false},
    Planned{106,C::PlaceOriginalMilletField,{117,131},105,2,false,false},
    Planned{107,C::PlaceOriginalHempFarm,{114,123},0,1,false,false},
    Planned{108,C::PlaceOriginalHempField,{116,123},107,1,false,false},
    Planned{109,C::PlaceOriginalHempFarm,{113,121},0,1,false,false},
    Planned{110,C::PlaceOriginalHempField,{115,121},109,1,false,false},
    Planned{111,C::PlaceHousehold,{111,133},0,0,false,false},
    Planned{112,C::PlaceHousehold,{114,134},0,0,false,false},
    Planned{113,C::PlaceHousehold,{121,128},0,0,false,false},
    Planned{114,C::PlaceHousehold,{111,135},0,0,false,false},
    Planned{115,C::PlaceHousehold,{121,126},0,1,false,false},
    Planned{116,C::PlaceHousehold,{121,130},0,1,false,false},
    Planned{117,C::PlaceHousehold,{116,135},0,1,false,false},
    Planned{118,C::PlaceHousehold,{113,138},0,1,false,false},
    Planned{119,C::PlaceHousehold,{113,117},0,1,false,false},
    Planned{120,C::PlaceHousehold,{111,137},0,1,false,false},
    Planned{121,C::PlaceHousehold,{121,132},0,1,false,false},
    Planned{122,C::PlaceHousehold,{113,115},0,1,false,false},
    Planned{123,C::PlaceWarehouse,{122,124},0,1,false,false},
    Planned{124,C::PlaceWarehouse,{118,136},0,1,false,false},
    Planned{125,C::PlaceMarket,{113,120},0,1,false,false},
    Planned{126,C::PlaceMarket,{114,136},0,0,false,false},
    Planned{127,C::PlaceClaySource,{121,134},0,1,false,false},
    Planned{128,C::PlaceClaySource,{113,113},0,2,false,false},
    Planned{129,C::PlacePottery,{124,124},0,2,false,false},
    Planned{130,C::PlacePottery,{121,136},0,2,false,false},
    Planned{131,C::PlaceFireWatch,{115,136},0,0,false,false},
    Planned{132,C::PlaceFireWatch,{127,127},0,2,false,false},
    Planned{133,C::PlaceOriginalLoggingShed,{117,140},0,3,false,false},
    Planned{134,C::PlaceOriginalSilkwormShed,{126,124},0,3,false,false},
    Planned{135,C::PlaceOriginalMulberryTree,{128,124},134,3,false,false},
    Planned{136,C::PlaceOriginalWeaver,{131,127},0,3,false,false},
    Planned{137,C::PlaceOriginalPaperMaker,{129,124},0,3,false,false},
    Planned{138,C::PlaceOriginalLacquerRefinery,{131,124},0,3,false,false},
    Planned{139,C::PlaceOriginalLacquerTree,{133,124},138,3,false,false},
    Planned{140,C::PlaceOriginalLacquerwareMaker,{134,126},0,3,false,false},
    Planned{141,C::PlaceOriginalTeaCuringShed,{128,138},0,4,false,false},
    Planned{142,C::PlaceOriginalTeaBush,{128,140},141,4,false,false},
    Planned{143,C::PlaceMarket,{127,140},0,3,false,false,true},
    Planned{144,C::PlaceMarket,{126,140},0,3,false,false,true},
};
inline constexpr std::array<sim::Cell,82> roads{{
{112,114}, {112,115}, {112,116}, {112,117}, {112,118}, {112,119}, {112,120}, {112,121}, {112,122}, {112,123}, {113,123}, {113,124}, {113,125}, {113,126}, {113,127}, {113,128}, {113,129}, {113,130}, {113,131}, {113,132}, {113,133}, {113,134}, {113,135}, {113,136}, {113,137}, {114,137}, {115,137}, {116,137}, {117,137}, {117,138}, {118,138}, {118,139}, {119,139}, {120,139}, {121,139}, {122,139}, {123,126}, {123,127}, {123,128}, {123,129}, {123,130}, {123,131}, {123,132}, {123,133}, {123,134}, {123,135}, {123,136}, {123,139}, {124,126}, {124,136}, {124,137}, {124,138}, {124,139}, {125,126}, {125,139}, {126,126}, {126,139}, {127,126}, {127,132}, {127,137}, {127,138}, {127,139}, {128,126}, {128,132}, {128,137}, {129,126}, {129,132}, {129,133}, {129,134}, {129,135}, {129,136}, {129,137}, {130,126}, {130,132}, {131,126}, {131,131}, {131,132}, {132,126}, {132,137}, {133,126}, {133,127}, {134,133}
}};
inline bool known(std::string_view variant){return variant=="staged"||variant=="all-paid"||variant=="lean-med"||variant=="lean-med-extra-markets";}
inline bool lean(std::string_view variant){return variant=="lean-med"||variant=="lean-med-extra-markets";}
inline bool included(const Planned& node,std::string_view variant){return (!node.extra_market||variant=="lean-med-extra-markets")&&!(lean(variant)&&node.lean_omit);}
inline std::optional<sim::BuildingId> owner(const sim::World& w,sim::Cell cell){return w.building_owner_at(cell);}
inline std::optional<sim::BuildingId> elite(const sim::World& w){
    const auto id=owner(w,elite_cell);if(!id)return {};
    q::check(sim::original_elite_house_kind(w.building(*id).kind),"Reserved Elite site has another paid owner");return id;
}
inline unsigned phase(const sim::World& w){
    unsigned value=0;if(w.total_population()>=20)value=1;if(w.total_population()>=120)value=2;
    if(w.total_population()>=200)value=3;
    if(w.total_population()>=260||(elite(w)&&q::house(w,*elite(w)).type>=orig::HouseType::ImpressiveCompound))value=4;
    return value;
}
inline Json ordinary(sim::World& w,sim::Command command,const char* reason){
    const auto before=w.snapshot();const auto quote=w.construction_cost(command.type);const auto planned=w.validate(command);
    Json row{{"tick",w.ticks()},{"command",static_cast<unsigned>(command.type)},{"cell",{command.cell.x,command.cell.y}},
        {"owner_argument",static_cast<unsigned>(command.building_id)},{"reason",reason},{"quoted_cost",quote},
        {"funds_before",w.treasury()},{"sequence_before",w.command_sequence()}};
    const auto result=w.execute(command);
    q::check(result.accepted==planned.accepted,"Validation/command decision changed without a tick");
    row["accepted"]=result.accepted;row["changed"]=result.changed;row["result"]=result.reason;
    row["funds_after"]=w.treasury();row["sequence_after"]=w.command_sequence();
    if(!result.accepted){q::check(w.snapshot()==before,"Refused paid recipe command was not atomic");return row;}
    q::check(result.changed&&w.ticks()==before.ticks&&w.treasury()==before.treasury-quote&&
        w.construction_spent_total()==before.construction_spent_total+static_cast<std::uint64_t>(quote),"Recipe purchase bypassed Funds");
    if(const auto id=owner(w,command.cell))row["building_id"]=static_cast<unsigned>(*id);
    return row;
}
inline const Planned& planned(unsigned id){for(const auto& node:plan)if(node.plan_id==id)return node;throw std::runtime_error("Unknown recipe owner plan ID");}
inline void attachment(const sim::World& w,const Planned& node){
    if(!node.owner)return;const auto parent=owner(w,planned(node.owner).cell);const auto child=owner(w,node.cell);
    q::check(parent&&child,"Paid recipe attachment lost its intended owner or child body");bool exact=false;
    if(node.command==C::PlaceOriginalMilletField||node.command==C::PlaceOriginalWheatField){
        for(const auto& f:w.original_production_state()->food.farms)if(f.farm_id==*parent)exact=f.field_id==child;
    }else if(node.command==C::PlaceOriginalHempField){
        for(const auto& f:w.original_production_state()->hemp_farms)if(f.farm_id==*parent)exact=f.field_id==child;
    }else{
        for(const auto& f:w.original_production_state()->industry.sources)if(f.building_id==*parent)exact=f.plant_id==child;
    }
    q::check(exact,"Paid recipe attachment is not bound to its exact explicitly selected owner");
}
inline bool initial_road(const sim::World& w,sim::Cell cell){return w.original_road(cell)||w.object_at(cell)==sim::Object::Road;}
inline bool apply(sim::World& w,std::string_view variant,Json& commands,Json& blocked){
    q::check(known(variant)&&w.original_cardinal_water_enabled(),"Elite recipe requires explicit Rule14");
    const auto append=[&](sim::Command command,const char* reason){auto row=ordinary(w,command,reason);const bool ok=row.at("accepted");commands.push_back(row);if(!ok)blocked=row;return ok;};
    for(const auto cell:roads)if(!initial_road(w,cell))if(!append({C::PlaceRoad,cell},"Authored reserved road recipe"))return false;
    const auto unlocked=variant=="all-paid"?4U:phase(w);
    for(const auto& node:plan){
        if(node.phase>unlocked||!included(node,variant))continue;
        const auto kind=sim::placed_object(node.command);q::check(kind.has_value(),"Unmapped player recipe command");
        if(const auto id=owner(w,node.cell)){q::check(w.building(*id).kind==*kind,"Player recipe site has another body");attachment(w,node);continue;}
        sim::Command command{node.command,node.cell};
        if(node.owner){const auto parent=owner(w,planned(node.owner).cell);if(!parent)continue;command.building_id=*parent;}
        // This is an explicit player reserve, not a new World admission rule.
        // A waiting reserve is reported separately from an actual refusal.
        if(variant!="all-paid"&&node.phase>=3&&w.treasury()<w.construction_cost(node.command)+
            std::int64_t(w.workforce_required())*sim::original_worker_upkeep_funds)continue;
        if(!append(command,"Actual population/needs stage unlocked"))return false;
        attachment(w,node);if(node.owner)commands.back()["exact_owner_binding_checked"]=true;
    }
    if((unlocked>=2||variant=="all-paid")&&!elite(w))if(!append({C::PlaceOriginalElitePlot,elite_cell},"Separate paid Elite family plot"))return false;
    // Redundant providers remain paid physical DES sources. Ordinary pause
    // changes future staffing/dispatch only; installed upkeep is not waived.
    if(variant!="all-paid")for(const auto& node:plan)if(node.duplicate){
        if(node.command==C::PlaceOriginalWaterWell||node.command==C::PlaceOriginalAncestorShrine)continue;
        if(const auto id=owner(w,node.cell);id&&w.building(*id).operating_enabled)
            if(!append(sim::set_building_operation(*id,false),"Pause redundant provider; installed upkeep retained"))return false;
    }
    for(const auto& b:w.buildings()){
        const bool critical=b.kind==sim::Object::OriginalWaterWell72||b.kind==sim::Object::OriginalFoodFarm||b.kind==sim::Object::Market||b.kind==sim::Object::FireWatch;
        if(critical&&b.workforce_priority!=sim::WorkforcePriority::High){
            auto command=sim::set_building_workforce_priority(b.id,sim::WorkforcePriority::High);command.cell=b.cell;
            if(!append(command,"Prioritize actual Water/Food/Market/Fire staffing"))return false;
        }
    }
    return true;
}
inline Json status(const sim::World& w,std::string_view variant){
    q::check(known(variant),"Unknown paid recipe variant in final status");
    Json pending=Json::array();for(const auto& node:plan)if(included(node,variant)&&!owner(w,node.cell))pending.push_back({{"plan_id",node.plan_id},{"phase",node.phase},{"command",static_cast<unsigned>(node.command)}});
    Json homes=Json::array();for(const auto& h:w.original_economy_state()->houses){const auto d=w.original_housing_diagnosis(h.building_id);auto query=d.actual;
        query.desirability=static_cast<std::int8_t>(d.cached_desirability);
        homes.push_back({{"id",static_cast<unsigned>(h.building_id)},{"cell",{w.building(h.building_id).cell.x,w.building(h.building_id).cell.y}},
            {"actual_type",static_cast<unsigned>(d.actual.type)},{"actual_occupants",d.actual.occupants},
            {"raw_des",d.raw_desirability},{"cardinal_bonus",d.cardinal_water_bonus},{"query_des",d.cached_desirability},
            {"current",oe_high_tier_matrix::row(d.current,query,w.original_rules_model()->house_rules())},
            {"next",oe_high_tier_matrix::row(d.next,query,w.original_rules_model()->house_rules())}});}
    Json result{{"unlocked_phase",phase(w)},{"actual_population",w.total_population()},{"installed_workers",w.workforce_required()},
        {"active_workers",w.active_workforce_required()},{"used_workers",w.workforce_used()},{"funds",w.treasury()},{"pending_plan_nodes",pending},{"actual_houses_and_needs",homes}};
    if(const auto target=elite(w)){const auto d=w.original_housing_diagnosis(*target);auto query=d.actual;
        query.desirability=static_cast<std::int8_t>(d.cached_desirability);
        result["target"]=static_cast<unsigned>(*target);result["type"]=static_cast<unsigned>(d.actual.type);
        result["raw_des"]=d.raw_desirability;result["cardinal_bonus"]=d.cardinal_water_bonus;result["query_des"]=d.cached_desirability;
        result["current"]=oe_high_tier_matrix::row(d.current,query,w.original_rules_model()->house_rules());
        result["next"]=oe_high_tier_matrix::row(d.next,query,w.original_rules_model()->house_rules());}
    return result;
}
} // namespace oe_elite_desirability_strategy
