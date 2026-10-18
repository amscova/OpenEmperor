#pragma once

#include "simulation/OriginalGoodsState.h"

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace openemperor::simulation {

// Authored crop/score adapter. These are not original resource IDs or a
// seventh entry in the frozen six-goods ledger.
enum class OriginalFoodCrop : std::uint8_t { Wheat=0, Millet=1 };
inline constexpr std::size_t original_food_crop_count=2;
using OriginalFoodAmounts=std::array<std::int32_t,original_food_crop_count>;
using OriginalFoodCounters=std::array<std::uint64_t,original_food_crop_count>;
struct FoodBatchQuality {
    std::uint8_t value=0;
    bool operator==(const FoodBatchQuality&) const = default;
};
[[nodiscard]] FoodBatchQuality original_food_batch_quality(const OriginalFoodAmounts& amounts);

struct OriginalFoodFarmState {
    BuildingId farm_id=static_cast<BuildingId>(0);
    std::optional<BuildingId> field_id;
    OriginalFoodCrop crop=OriginalFoodCrop::Wheat;
    std::uint64_t harvested=0;
    bool operator==(const OriginalFoodFarmState&) const = default;
};
struct OriginalFoodQualityWitness {
    OriginalGoodsTransferKey transfer;
    std::uint64_t tick=0;
    OriginalFoodAmounts accepted{};
    OriginalFoodCounters consumed_before{};
    std::uint8_t previous_quality=0;
    bool operator==(const OriginalFoodQualityWitness&) const = default;
};
struct OriginalFoodStock {
    BuildingId building_id=static_cast<BuildingId>(0);
    OriginalFoodAmounts amounts{};
    // Audit counters, never alternative stocks. Exact per-owner equation:
    // harvest + received + returned = stock + departed + consumed.
    OriginalFoodCounters received{},departed{},returned{},consumed{};
    std::uint64_t missed_demands=0;
    std::optional<OriginalFoodQualityWitness> quality_witness=std::nullopt;
    bool operator==(const OriginalFoodStock&) const = default;
};
// The outstanding part (loaded-delivered-returned) is the sole cargo authority.
// One latest trip per actual courier also retains its sequence highwater.
struct OriginalFoodTrip {
    OriginalGoodsTransferKey transfer;
    BuildingId source=static_cast<BuildingId>(0),target=static_cast<BuildingId>(0);
    OriginalGoodsTransferPhase phase=OriginalGoodsTransferPhase::Loaded;
    OriginalFoodAmounts loaded{},delivered{},returned{};
    bool operator==(const OriginalFoodTrip&) const = default;
};
struct OriginalFoodState {
    std::vector<OriginalFoodFarmState> farms;
    std::vector<OriginalFoodStock> stocks;
    std::vector<OriginalFoodTrip> trips;
    bool operator==(const OriginalFoodState&) const = default;
};

} // namespace openemperor::simulation
