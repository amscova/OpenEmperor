#pragma once

#include "simulation/OriginalGoodsState.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace openemperor::simulation {

inline constexpr std::size_t original_water_provider_limit=2;
inline constexpr std::size_t original_water_trip_limit=2;
inline constexpr std::size_t original_water_house_limit=20;

// These paid model72 bodies are independent of the historical authored60
// influence-only objects. Each owns exactly one cargo-free Water courier.
struct OriginalWaterProvider {
    BuildingId building_id=static_cast<BuildingId>(0);
    bool operator==(const OriginalWaterProvider&) const = default;
};
struct OriginalWaterTrip {
    OriginalGoodsTransferKey transfer;
    BuildingId target=static_cast<BuildingId>(0);
    std::uint64_t dispatched_tick=0;
    std::optional<std::uint64_t> arrival_tick=std::nullopt;
    bool visit_applied=false;
    bool operator==(const OriginalWaterTrip&) const = default;
};
struct OriginalWaterWitness {
    BuildingId building_id=static_cast<BuildingId>(0);
    BuildingId provider_id=static_cast<BuildingId>(0);
    OriginalGoodsTransferKey transfer;
    std::uint64_t arrival_tick=0;
    bool operator==(const OriginalWaterWitness&) const = default;
};
struct OriginalWaterHouseState {
    BuildingId building_id=static_cast<BuildingId>(0);
    std::uint8_t normal_timer=0;
    std::uint8_t special_timer=0; // The unbound original alternate branch is unsupported.
    std::optional<OriginalWaterWitness> current_witness=std::nullopt;
    // Retain the arrival actually observed at the last Phase28. A newer
    // arrival may replace current_witness before another evaluation occurs.
    std::optional<OriginalWaterWitness> evaluation_witness=std::nullopt;
    bool operator==(const OriginalWaterHouseState&) const = default;
};
struct OriginalWaterState {
    std::vector<OriginalWaterProvider> providers;
    std::vector<OriginalWaterTrip> trips;
    std::vector<OriginalWaterHouseState> houses;
    bool operator==(const OriginalWaterState&) const = default;
};

inline constexpr std::uint64_t original_water_countdowns(std::uint64_t tick) noexcept {
    return tick<36 ? 0:1+(tick-36)/51;
}
inline constexpr std::uint64_t original_water_evaluation_tick(std::uint64_t tick) noexcept {
    return tick<29 ? 0:tick-((tick-29)%51);
}
// Observation helper only: invalid/future arrivals yield no coverage, and
// authority validation separately rejects them. No addition can overflow.
inline constexpr std::uint8_t original_water_timer_at(const OriginalWaterWitness& witness,
                                                     std::uint64_t tick) noexcept {
    if (!witness.arrival_tick || witness.arrival_tick>tick) return 0;
    const auto elapsed=original_water_countdowns(tick)-original_water_countdowns(witness.arrival_tick-1);
    return elapsed>=96 ? 0:static_cast<std::uint8_t>(96-elapsed);
}

} // namespace openemperor::simulation
