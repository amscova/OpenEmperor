#include "simulation/original/GoodsRules.h"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace openemperor::original {
namespace {

std::int16_t HouseState::* stock_member(Goods goods) {
    switch (goods) {
    case Goods::Hemp: return &HouseState::hemp;
    case Goods::Ceramics: return &HouseState::ceramics;
    case Goods::Tea: return &HouseState::tea;
    case Goods::Silk: return &HouseState::silk;
    case Goods::Bronzeware: return &HouseState::bronzeware;
    case Goods::Lacquerware: return &HouseState::lacquerware;
    }
    throw std::out_of_range("invalid original goods identity");
}

void validate_inventory(const HouseState& house) {
    // Validate even a gated/no-op operation; never hide a negative input stock.
    (void)house_row(house.type);
    for (std::size_t i = 0; i < goods_count; ++i)
        if (goods_stock(house, static_cast<Goods>(i)) < 0)
            throw std::invalid_argument("negative original house goods stock");
}

// Compare the nearest-even binary32 quotient against the observed thresholds.
// Integer cross-products implement their rounding midpoints exactly, including
// widened int32 observations that could double-round via double -> float.
// All products are below 2^58. No caller floating-point rounding mode is read.
bool rounded_ratio_above(std::int32_t delivered, std::int32_t previous,
                         std::int64_t scale, std::int64_t midpoint,
                         bool midpoint_rounds_up = false) {
    const auto numerator = static_cast<std::int64_t>(delivered) * scale;
    const auto denominator = static_cast<std::int64_t>(previous) * midpoint;
    return midpoint_rounds_up ? numerator >= denominator : numerator > denominator;
}

} // namespace

HouseField goods_field(Goods goods) {
    switch (goods) {
    case Goods::Hemp: return HouseField::Hemp;
    case Goods::Ceramics: return HouseField::Ceramics;
    case Goods::Tea: return HouseField::Tea;
    case Goods::Silk: return HouseField::Silk;
    case Goods::Bronzeware:
    case Goods::Lacquerware: return HouseField::BronzewareOrLacquerware;
    }
    throw std::out_of_range("invalid original goods identity");
}

std::int16_t goods_stock(const HouseState& house, Goods goods) {
    return house.*stock_member(goods);
}

ModelHouse goods_delivery_row(HouseType type) {
    const auto current = house_row(type);
    return current == ModelHouse::LuxuriousApartment ? current : next_row(type);
}

GoodsDemand goods_demand(const HouseEvolutionRules& rules, const HouseState& house,
                          std::uint8_t footprint_side, Goods goods) {
    const auto field = goods_field(goods);
    const auto row = goods_delivery_row(house.type);
    if (house.occupants == 0 && (goods == Goods::Hemp || goods == Goods::Tea))
        return {10, 10};
    auto need = rules.value(row, field);
    if (footprint_side > 1) need *= 4;
    if (is_elite(house.type)) need *= 2;
    // Prepared model values are in [-99,100], so x8 stays within int32.
    return {need, std::max(need, 10)};
}

bool accepts_goods_delivery(const HouseState& house) {
    (void)house_row(house.type);
    return house.occupants > 0 || is_elite(house.type);
}

GoodsDelivery deliver_goods(const HouseEvolutionRules& rules, const HouseState& house,
                             std::uint8_t footprint_side, Goods goods,
                             std::int32_t source_stock) {
    const auto member = stock_member(goods);
    validate_inventory(house);
    if (source_stock < 0) throw std::invalid_argument("negative goods source stock");
    const auto demand = goods_demand(rules, house, footprint_side, goods);
    GoodsDelivery result{house, source_stock, 0};
    if (!accepts_goods_delivery(house) || demand.need <= 0 || source_stock == 0)
        return result;

    const std::int32_t stock = house.*member;
    auto amount = demand.target - stock;
    if (amount <= 0 || amount > source_stock) amount = demand.need - stock;
    amount = std::clamp(amount, 0, demand.target);
    amount = std::min({amount, source_stock,
                       std::numeric_limits<std::int16_t>::max() - stock});
    result.house.*member = static_cast<std::int16_t>(stock + amount);
    result.source_stock -= amount;
    result.transferred = amount;
    return result;
}

GoodsConsumption consume_goods(const HouseEvolutionRules& rules, const HouseState& house) {
    validate_inventory(house);
    GoodsConsumption result{house, {}};
    if (house.occupants == 0) return result;
    const auto row = house_row(house.type);
    for (std::size_t i = 0; i < goods_count; ++i) {
        const auto goods = static_cast<Goods>(i);
        const auto member = stock_member(goods);
        const auto amount = std::clamp(rules.value(row, goods_field(goods)),
                                        0, static_cast<std::int32_t>(house.*member));
        result.house.*member = static_cast<std::int16_t>(house.*member - amount);
        result.consumed[i] = amount;
    }
    return result;
}

std::uint8_t blend_food_quality(std::uint8_t house_quality, std::uint8_t source_quality,
                                 std::int32_t delivered, std::int32_t previous_food_stock) {
    if (delivered < 0 || previous_food_stock < 0)
        throw std::invalid_argument("negative food delivery observation");
    if (delivered == 0) return house_quality; // Explicit delivery boundary.
    if (source_quality > house_quality) return source_quality;
    const std::int32_t q = house_quality;
    const std::int32_t m = source_quality;
    if (previous_food_stock == 0 ||
        rounded_ratio_above(delivered, previous_food_stock, 8388608, 25165825))
        return static_cast<std::uint8_t>((q + 3 * m) / 4);
    if (rounded_ratio_above(delivered, previous_food_stock, 8388608, 16777217))
        return static_cast<std::uint8_t>((q + 2 * m) / 3);
    if (rounded_ratio_above(delivered, previous_food_stock, 33554432, 16777217))
        return static_cast<std::uint8_t>((q + m) / 2);
    if (rounded_ratio_above(delivered, previous_food_stock, 67108864, 22145927, true))
        return static_cast<std::uint8_t>((m + 2 * q) / 3);
    return static_cast<std::uint8_t>((m + 3 * q) / 4);
}

} // namespace openemperor::original
