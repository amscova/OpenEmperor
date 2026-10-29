#pragma once

#include "simulation/original/GoodsRules.h"
#include <span>

namespace openemperor::simulation {

// Authored Rule13 selection, inherited by Rule14. No stock, arrival, clock or saved authority.
// Rows are canonical (target, good) pairs supplied by World from cached routes.
struct MarketNeed {
    unsigned tier=3; // 0: missing current; 1: current buffer; 2: usable next row.
    int stock=0, demand=1;
};
struct MarketChoice {
    std::size_t pair=0;
    MarketNeed need;
};
std::optional<MarketNeed> original_market_need(const original::HouseEvolutionRules& rules,
    const original::HouseState& house,original::Goods good,
    const std::array<int,original::goods_count>& incoming);
std::optional<std::size_t> choose_original_market_pair(std::span<const MarketChoice> choices,
    std::size_t start,std::size_t pair_count);

} // namespace openemperor::simulation
