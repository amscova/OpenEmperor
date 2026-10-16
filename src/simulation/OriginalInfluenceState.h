#pragma once

#include <cstdint>
#include <vector>

namespace openemperor::simulation {

enum class BuildingId : std::uint32_t;

// Authored runtime binding to a verified model row, not a reconstructed full
// original building or a functional_model_candidate identity upgrade.
enum class OriginalInfluenceSourceType : std::uint16_t { Well72=72 };

struct OriginalInfluenceSourceState {
    BuildingId building_id=static_cast<BuildingId>(0);
    OriginalInfluenceSourceType type=OriginalInfluenceSourceType::Well72;
    bool operator==(const OriginalInfluenceSourceState&) const = default;
};

struct OriginalInfluenceRemovedSource {
    BuildingId building_id=static_cast<BuildingId>(0);
    OriginalInfluenceSourceType type=OriginalInfluenceSourceType::Well72;
    std::uint64_t placed_tick=0;
    std::uint64_t removed_tick=0;
    bool operator==(const OriginalInfluenceRemovedSource&) const = default;
};

struct OriginalInfluenceState {
    std::vector<OriginalInfluenceSourceState> sources;
    std::vector<OriginalInfluenceRemovedSource> removed_sources;
    bool operator==(const OriginalInfluenceState&) const = default;
};

} // namespace openemperor::simulation
