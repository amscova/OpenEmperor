#pragma once

#include "maps/LandscapeInstances.h"
#include "maps/OriginalMapEntities.h"
#include "maps/StoredGraphicsPlan.h"

#include <map>

namespace openemperor::maps {

inline constexpr std::uint32_t ordinary_tower_resource_key = 0x44cU;

struct OrdinaryTowerPresentation {
    OriginalEntityRecord source;
    GridCell origin{};
    unsigned width = 0, height = 0;
    std::vector<LandscapeInstanceSpec> components;
    std::string fallback;
};

// One complete static cTower body, prepared from a fully read original manager.
// Exterior wall-connection fragments and original painter parity remain open.
// These claims are renderer ownership, never simulation occupancy or roads.
struct OrdinaryTowerMapPresentation {
    std::uint64_t manager_offset = 0, manager_bytes = 0;
    std::size_t manager_records = 0;
    std::vector<OrdinaryTowerPresentation> towers;
    std::vector<std::optional<std::size_t>> tower_by_storage;
    // Bounded source diagnostics only, including refused static-body states.
    // Null means multiple original sources at this origin, never ownership.
    std::map<std::size_t,std::optional<std::size_t>> tower_source_at_origin;
    std::string error;
};

OrdinaryTowerMapPresentation prepare_ordinary_tower_presentation(
    const OriginalMapEntities& entities,
    std::span<const std::uint32_t> terrain,
    std::span<const std::uint32_t> objects,
    std::span<const std::uint8_t> heights);

void read_ordinary_tower_presentation(StoredGraphicsPlan& plan,
    const EmperorContainer& container, std::size_t part);

} // namespace openemperor::maps
