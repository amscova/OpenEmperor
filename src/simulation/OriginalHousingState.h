#pragma once

#include "simulation/MapPermissions.h"
#include "simulation/OriginalFoodState.h"
#include "simulation/OriginalServicesState.h"
#include "simulation/OriginalWaterState.h"
#include "simulation/original/HouseEvolution.h"

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace openemperor::simulation {

// Compact historical inputs; HouseState remains the sole current authority.
struct OriginalHousingEvaluation {
    std::uint64_t tick=0;
    original::HouseState input;
    // Fixed order: Herbalist, Acupuncture, Ancestor, Confucian, Daoist, Buddhist, Music, Acrobat, Drama.
    std::array<std::optional<OriginalServiceWitness>,9> service_witnesses{};
    std::optional<OriginalWaterWitness> water_evaluation_witness;
    std::optional<OriginalFoodQualityWitness> food_quality_witness;
    OriginalFoodCounters food_received{},food_consumed{};
    std::array<std::uint64_t,original::goods_count> goods_consumed_before{};
    bool operator==(const OriginalHousingEvaluation&) const = default;
};
struct OriginalHousingConsumptionRow {
    std::uint64_t events=0;
    std::array<std::uint64_t,original::goods_count> consumed{};
    bool operator==(const OriginalHousingConsumptionRow&) const = default;
};
enum class OriginalHousingTransitionKind : std::uint8_t { Admission=0, Evolution=1 };
struct OriginalHousingTransition {
    OriginalHousingTransitionKind kind=OriginalHousingTransitionKind::Admission;
    OriginalHousingEvaluation evaluation;
    std::int16_t admitted=0;
    bool operator==(const OriginalHousingTransition&) const = default;
};
struct OriginalHousingHouse {
    BuildingId building_id=static_cast<BuildingId>(0);
    std::optional<std::uint64_t> first_admission_tick;
    // ModelHouse row order (type3..17); the paid family's initial empty type
    // is established by its body kind, not by a duplicated family flag.
    std::array<OriginalHousingConsumptionRow,original::house_row_count> consumption_rows{};
    std::optional<OriginalHousingEvaluation> last_evaluation;
    std::array<std::optional<OriginalHousingTransition>,original::house_row_count> first_transitions{};
    bool operator==(const OriginalHousingHouse&) const = default;
};
struct OriginalHousingSourceOrigin {
    BuildingId building_id=static_cast<BuildingId>(0);
    Cell origin;
    bool operator==(const OriginalHousingSourceOrigin&) const = default;
};
struct OriginalHousingState {
    std::vector<OriginalHousingSourceOrigin> neutral_source_origins;
    std::vector<OriginalHousingHouse> houses;
    bool operator==(const OriginalHousingState&) const = default;
};
constexpr std::uint64_t original_housing_evaluation_tick(std::uint64_t ticks) noexcept {
    return ticks<40 ? 0:ticks-((ticks-40)%51);
}
constexpr std::uint64_t original_housing_consumption_events(std::uint64_t ticks) noexcept {
    return (ticks>=40 ? 1+(ticks-40)/816:0)+(ticks>=397 ? 1+(ticks-397)/816:0);
}

} // namespace openemperor::simulation
