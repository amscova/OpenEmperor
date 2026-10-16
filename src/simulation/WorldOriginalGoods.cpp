#include "simulation/World.h"
#include "simulation/OriginalGoodsLedger.h"

#include <array>
#include <stdexcept>
#include <utility>

namespace openemperor::simulation {

World World::create_original_goods_development(std::shared_ptr<const MapPermissions> permissions,
    std::shared_ptr<const original::OriginalRulesModel> model) {
    if (!permissions || permissions->policy_version()!=kBridgesMapPermissionsPolicyVersion || !model)
        throw std::invalid_argument("original goods development requires policy3 and prepared model");
    const auto width=permissions->width(),height=permissions->height();
    auto mask=permissions->building_mask();
    World world(width,height,std::move(mask),RulesProfile::CityV16,kOriginalGoodsRuleVersion,
        std::move(permissions),true,std::move(model));
    world.rebuild_original_influence_cache();
    return world;
}

World World::restore_original_goods_development(const WorldSnapshot& snapshot,
    std::shared_ptr<const MapPermissions> permissions,
    std::shared_ptr<const original::OriginalRulesModel> model) {
    if (!simulation::original_goods_enabled(snapshot.profile,snapshot.rule_version) ||
        !permissions || permissions->policy_version()!=kBridgesMapPermissionsPolicyVersion || !model)
        throw std::invalid_argument("original goods restore requires explicit rule9, policy3 and prepared model");
    validate_original_goods_snapshot(snapshot);
    return restore_impl(snapshot,{},std::move(permissions),true,std::move(model));
}

void World::validate_original_goods_snapshot(const WorldSnapshot& snapshot) {
    if (!simulation::original_goods_enabled(snapshot.profile,snapshot.rule_version)) {
        if (snapshot.original_goods)
            throw std::invalid_argument("original goods authority requires City-v16 rule9");
        return;
    }
    if (!snapshot.original_goods || !snapshot.original_economy || !snapshot.original_economy->model_source ||
        !snapshot.original_influence || *snapshot.original_goods!=OriginalGoodsState{})
        throw std::invalid_argument("rule9 goods capabilities require zero totals and empty stock/cargo/reservation lists");
    for (const auto& home:snapshot.original_economy->houses)
        for (std::size_t i=0;i<original::goods_count;++i)
            if (original::goods_stock(home.house,static_cast<original::Goods>(i))!=0)
                throw std::invalid_argument("rule9 has no permitted positive original goods origin");
}

std::array<OriginalGoodsBalance,original::goods_count> World::original_goods_balances() const {
    if (!original_goods_enabled() || !original_goods_ || !original_economy_)
        throw std::invalid_argument("original goods balances require explicit rule9 authority");
    std::array<OriginalGoodsOwner,Rules::city_v10_household_limit> owners{};
    const auto& houses=original_economy_->houses;
    if (houses.size()>owners.size()) throw std::logic_error("original goods house bound violated");
    for (std::size_t i=0;i<houses.size();++i) owners[i]={houses[i].building_id,true,{}};
    // The spans refer only to existing immutable World authorities. No World
    // copies, stock mirrors, allocation, routes, model preparation or IO.
    return inspect_original_goods_ledger({*original_goods_,houses,
        std::span<const OriginalGoodsOwner>(owners.data(),houses.size()),{}});
}

OriginalGoodsBalance World::original_goods_balance(original::Goods good) const {
    const auto index=static_cast<std::size_t>(good);
    if (index>=original::goods_count) throw std::out_of_range("invalid original goods identity");
    return original_goods_balances()[index];
}

} // namespace openemperor::simulation
