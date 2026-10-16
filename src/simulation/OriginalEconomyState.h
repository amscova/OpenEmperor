#pragma once

#include "simulation/original/HouseEvolution.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace openemperor::simulation {

enum class BuildingId : std::uint32_t;

// Source identity only. No proprietary model rows, prepared rules or runtime
// readiness are stored here; the later session binding owns those checks.
struct OriginalModelSource {
    std::uint32_t format_version=1;
    std::string sha256;
    std::uint64_t byte_count=0;
    original::Difficulty difficulty=original::Difficulty::Normal;
    bool operator==(const OriginalModelSource&) const = default;
};

// Independent from the legacy Household level, population and demand fields.
// A newly paid development container is vacant and grants no needs or goods.
struct OriginalHouseState {
    BuildingId building_id=static_cast<BuildingId>(0);
    original::HouseState house{.type=original::HouseType::Vacant};
    std::uint8_t cycle_phase=0;
    std::uint8_t original_status=0;
    bool access_enabled=false;
    bool operator==(const OriginalHouseState&) const = default;
};

struct OriginalEconomyState {
    std::optional<OriginalModelSource> model_source;
    std::vector<OriginalHouseState> houses;
    bool operator==(const OriginalEconomyState&) const = default;
};

} // namespace openemperor::simulation
