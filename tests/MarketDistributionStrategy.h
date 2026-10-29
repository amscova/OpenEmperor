#pragma once

// Paid player-side controls. These select ordinary construction/operation
// commands; no transport, demand or historical rule is changed.
#include "HighTierProgressionStrategy.h"

namespace oe_market_distribution_strategy {
namespace sim=openemperor::simulation;
namespace prior=oe_high_tier_strategy;
namespace paid=oe_economy_fire_strategy;
using oe_starter_fixture::check;

inline bool common_variant(std::string_view name) {
    return prior::common_variant(name)||name=="common-near-chain"||name=="common-priority"||name=="common-short-route"||name=="common-compact-chain";
}
inline prior::Receipt build_common(sim::World& w,sim::Cell origin,std::string_view variant) {
    check(common_variant(variant),"Unknown paid market-distribution variant");
    const bool compact=variant=="common-compact-chain";
    const bool near=variant=="common-near-chain"||compact,priority=variant=="common-priority",shortcut=variant=="common-short-route"||compact;
    auto result=prior::build_common(w,origin,near?"common-near-pottery":priority||shortcut?"common-reserved":variant,
                                   near?sim::Cell{18,0}:sim::Cell{0,0});
    if(near)for(int y=0;y<4;++y) {
        auto row=paid::buy(w,{sim::CommandType::PlaceRoad,{17,y}},"Near chain road",origin);
        check(row.at("accepted").get<bool>(),"Near chain road was not paid");result.commands.push_back(std::move(row));
    }
    if(shortcut)for(int y=5;y<9;++y) {
        auto row=paid::buy(w,{sim::CommandType::PlaceRoad,{8,y}},"Market shortcut road",origin);
        check(row.at("accepted").get<bool>(),"Market shortcut road was not paid");result.commands.push_back(std::move(row));
    }
    if(compact)for(const auto& [type,cell,label]:{
        std::tuple{sim::CommandType::PlaceMarket,sim::Cell{17,7},"Second Market"},
        std::tuple{sim::CommandType::PlaceRoad,sim::Cell{17,8},"Second Market connection"}}) {
        auto row=paid::buy(w,{type,cell},label,origin);
        check(row.at("accepted").get<bool>(),"Compact chain connection was not paid");result.commands.push_back(std::move(row));
    }
    if(priority)for(const auto& body:w.buildings())if(body.kind==sim::Object::Warehouse||body.kind==sim::Object::Market) {
        auto command=sim::set_building_workforce_priority(body.id,sim::WorkforcePriority::High);
        command.cell=body.cell;
        auto row=paid::ordinary(w,command,"Logistics priority",origin);
        check(row.at("accepted").get<bool>(),"Logistics priority command refused");result.commands.push_back(std::move(row));
    }
    oe_starter_fixture::authority(w);return result;
}
}
