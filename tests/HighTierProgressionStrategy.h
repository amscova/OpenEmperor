#pragma once

#include "SustainableEconomyStrategy.h"
#include <tuple>

namespace oe_high_tier_strategy {
namespace sim=openemperor::simulation;
namespace base=oe_sustainable_strategy;
namespace paid=oe_economy_fire_strategy;
using Json=nlohmann::json;
using oe_starter_fixture::check;

inline bool common_variant(std::string_view name) {
    return name=="common-reserved"||name=="common-near-pottery"||name=="common-two-market";
}
struct Receipt {
    sim::BuildingId target{};
    Json commands=Json::array();
};

inline Receipt build_common(sim::World& w,sim::Cell origin,std::string_view variant,sim::Cell clay_origin={0,0}) {
    check(common_variant(variant)&&w.original_sustainable_economy_enabled()&&w.ticks()==20000,
          "Common expansion requires the actual earned Rule12 tick20000 city");
    Receipt result;const auto owner=w.building_owner_at(paid::at(origin,6,2));
    check(owner&&w.building(*owner).kind==sim::Object::Household,"Common target must be an existing paid family body");
    result.target=*owner;
    const auto buy=[&](sim::Command command,const char* name) {
        auto absolute=command;absolute.cell=paid::at(origin,command.cell.x,command.cell.y);
        const auto planned=w.validate(absolute);
        check(planned.accepted,"Paid Common plan refused "+std::string(name)+": "+planned.reason);
        auto record=paid::buy(w,command,name,origin);
        check(record.at("accepted").get<bool>(),"Paid Common purchase unexpectedly refused");
        result.commands.push_back(std::move(record));
        return w.building_owner_at(absolute.cell);
    };
    for(int y=0;y<4;++y)buy({sim::CommandType::PlaceRoad,{2,y}},"Road");
    for(const auto& [type,cell,name]:{
        std::tuple{sim::CommandType::PlaceOriginalAncestorShrine,sim::Cell{6,0},"Ancestor"},
        std::tuple{sim::CommandType::PlaceOriginalDaoistShrine,sim::Cell{3,0},"Daoist"},
        std::tuple{sim::CommandType::PlaceOriginalDaoistShrine,sim::Cell{9,0},"Daoist"},
        std::tuple{sim::CommandType::PlaceOriginalBuddhistShrine,sim::Cell{10,5},"Buddhist"},
        std::tuple{sim::CommandType::PlaceHealthPost,sim::Cell{12,0},"Herbalist"},
        std::tuple{sim::CommandType::PlaceOriginalMusicSchool,sim::Cell{15,0},"Music"},
        std::tuple{sim::CommandType::PlaceServicePost,sim::Cell{12,5},"Acupuncture"},
        std::tuple{sim::CommandType::PlaceOriginalAcrobatSchool,sim::Cell{21,5},"Acrobat"},
        std::tuple{sim::CommandType::PlaceClaySource,clay_origin,"Clay"}})buy({type,cell},name);
    buy({sim::CommandType::PlacePottery,variant=="common-near-pottery"?sim::Cell{18,2}:sim::Cell{22,2}},"Pottery");
    const auto tea=buy({sim::CommandType::PlaceOriginalTeaCuringShed,{15,5}},"Tea curing shed");
    check(tea.has_value(),"Paid Tea producer has no typed owner");
    sim::Command plant{sim::CommandType::PlaceOriginalTeaBush,{17,5}};plant.building_id=*tea;
    buy(plant,"Owned Tea bush");
    if(variant=="common-two-market") {
        buy({sim::CommandType::PlaceMarket,{17,7}},"Second Market");
        buy({sim::CommandType::PlaceRoad,{17,8}},"Second Market connection");
    }
    oe_starter_fixture::authority(w);
    check(w.original_influence_provenance(result.target).value>=40,
          "Actually paid source provenance does not reach the real Common entry threshold");
    return result;
}
}
