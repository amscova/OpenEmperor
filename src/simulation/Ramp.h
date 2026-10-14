#pragma once
#include "simulation/MapPermissions.h"
namespace openemperor::simulation {
struct RampId {
    std::uint32_t value=0;
    auto operator<=>(const RampId&) const = default;
};
inline constexpr std::size_t ramp_max_links=64;
inline constexpr std::int64_t ramp_cost=40;
// An owned edge, never a new terrain cell or original slope identity.
struct RampLink {
    RampId id{};
    Cell endpoint_a{},endpoint_b{}; // Increasing X or Y on its single axis.
    std::int8_t height_a=0,height_b=0;
    std::int64_t paid_cost=0;
    std::uint64_t placed_tick=0;
    bool operator==(const RampLink&) const = default;
};
struct RampValidation {
    bool geometry_valid=false,accepted=false;
    std::string reason;
    BuildDiagnostic diagnostic{};
    RampLink link{};
};
} // namespace openemperor::simulation
