#pragma once
#include "simulation/MapPermissions.h"
#include <span>

namespace openemperor::simulation {
struct BridgeId {
    std::uint32_t value=0;
    auto operator<=>(const BridgeId&) const = default;
};
enum class BridgeAxis : std::uint8_t { StorageX, StorageY };
inline constexpr std::size_t bridge_max_water_cells=8,bridge_max_spans=64;
inline constexpr std::int64_t bridge_base_cost=40,bridge_water_cell_cost=20;
// Authored infrastructure, independent of Building/Courier/original IDs.
struct BridgeSpan {
    BridgeId id{};
    BridgeAxis axis=BridgeAxis::StorageX;
    Cell bank_a{},bank_b{}; // Increasing coordinate on the chosen axis.
    std::vector<Cell> water_cells; // Complete canonical ordered interior.
    std::int8_t deck_height=0;
    std::int64_t paid_cost=0;
    std::uint64_t placed_tick=0;
    bool operator==(const BridgeSpan&) const = default;
};
struct BridgeValidation {
    bool geometry_valid=false,accepted=false;
    std::string reason;
    BuildDiagnostic diagnostic{};
    BridgeSpan span{};
};
} // namespace openemperor::simulation
