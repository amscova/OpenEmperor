#pragma once

#include "simulation/MapPermissions.h"
#include "simulation/OriginalFinanceState.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace openemperor::simulation {
enum class CourierId : std::uint32_t;
// Schema27-only authored provenance. These bounded journals are authority for
// replaying visited fire clocks and the precise pre-movement tax inputs.
inline constexpr std::size_t original_fire_arrival_limit=8192;
inline constexpr std::size_t original_fire_tax_limit=8192;
inline constexpr std::size_t original_fire_path_cell_limit=65536;
struct OriginalFireTrip {
    CourierId courier_id=static_cast<CourierId>(0);
    BuildingId owner=static_cast<BuildingId>(0),target=static_cast<BuildingId>(0);
    std::uint64_t dispatched_tick=0,road_revision=0;
    int assigned_workers=0,population=0;
    bool operating=false;
    std::vector<Cell> travelled;
    bool operator==(const OriginalFireTrip&) const = default;
};
struct OriginalFireArrival {
    OriginalFireTrip trip;
    std::uint64_t tick=0;
    bool extinguished=false;
    bool operator==(const OriginalFireArrival&) const = default;
};
struct OriginalFireTaxWitness {
    BuildingId building_id=static_cast<BuildingId>(0);
    OriginalTaxAssessment assessment;
    bool operator==(const OriginalFireTaxWitness&) const = default;
};
struct OriginalFireProtectionState {
    std::uint64_t travelled_cells=0;
    std::vector<OriginalFireTrip> trips;
    std::vector<OriginalFireArrival> arrivals;
    std::vector<OriginalFireTaxWitness> taxes;
    bool operator==(const OriginalFireProtectionState&) const = default;
};
} // namespace openemperor::simulation
