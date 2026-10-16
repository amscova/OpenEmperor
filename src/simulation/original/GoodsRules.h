#pragma once

#include "simulation/original/HouseEvolution.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace openemperor::original {

// API identities, separate from original commodity IDs and World resources.
// Food quality remains HouseState::food_quality, not a seventh goods stock.
enum class Goods : std::uint8_t {
    Hemp = 0, Ceramics = 1, Tea = 2, Silk = 3, Bronzeware = 4, Lacquerware = 5
};
inline constexpr std::size_t goods_count = 6;

struct GoodsDemand {
    std::int32_t need = 0;
    std::int32_t target = 10;
    bool operator==(const GoodsDemand&) const = default;
};

[[nodiscard]] HouseField goods_field(Goods goods);
[[nodiscard]] std::int16_t goods_stock(const HouseState& house, Goods goods);
// Canonical stored row, then original next-row selector; stored row 7 stays 7.
[[nodiscard]] ModelHouse goods_delivery_row(HouseType type);
// footprint_side is the observed unsigned byte, not a geometry transaction:
// 0/1 use x1, 2..255 x4; elite then x2. Exactly zero occupants gives Hemp/Tea
// need 10 before either multiplier. This query does not grant admission.
// Signed model needs stay observable; safe delivery disables nonpositive need.
[[nodiscard]] GoodsDemand goods_demand(const HouseEvolutionRules& rules,
                                        const HouseState& house,
                                        std::uint8_t footprint_side, Goods goods);
[[nodiscard]] bool accepts_goods_delivery(const HouseState& house);

struct GoodsDelivery {
    HouseState house;
    std::int32_t source_stock = 0;
    std::int32_t transferred = 0;
    bool operator==(const GoodsDelivery&) const = default;
};

// A resolved source pool and a House observation, not a market/actor/route.
// All six house stocks and source must be nonnegative, even for no-op calls.
// The original request fallback compares against available source, not need.
// Transfer is capped to that source and remaining signed-word storage; no wrap.
[[nodiscard]] GoodsDelivery deliver_goods(const HouseEvolutionRules& rules,
                                           const HouseState& house,
                                           std::uint8_t footprint_side,
                                           Goods goods, std::int32_t source_stock);

struct GoodsConsumption {
    HouseState house;
    std::array<std::int32_t, goods_count> consumed{};
    bool operator==(const GoodsConsumption&) const = default;
};

// One explicit consumption operation, not a phase/month/World tick. Current
// canonical model row; no size multiplier. Exactly zero occupants skips.
// Bronze and Lacquer independently consume the shared field 13: the needs
// predicate's OR is not a stock-priority consumption rule. Nonpositive edited
// model amounts consume nothing. Negative inventory is rejected, not repaired.
[[nodiscard]] GoodsConsumption consume_goods(const HouseEvolutionRules& rules,
                                              const HouseState& house);

// One already resolved food-delivery observation. Denominator is the stock
// BEFORE delivery, never a FoodQuality threshold. A nearest-even binary32 ratio
// is modeled exactly; the original global FPU environment is not established.
// Strict thresholds and integer weighted averages follow the observed path.
// Byte qualities are independent of food quantity. Nonnegative quantities are
// required; zero delivery preserves quality (explicit safe API boundary).
[[nodiscard]] std::uint8_t blend_food_quality(std::uint8_t house_quality,
                                               std::uint8_t source_quality,
                                               std::int32_t delivered,
                                               std::int32_t previous_food_stock);

} // namespace openemperor::original
