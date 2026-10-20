#pragma once

#include "simulation/World.h"
#include <array>

namespace openemperor::simulation {

// An authored arrangement of normal paid commands, never original starting-city
// authority. Call only on an unpublished, fresh Rule10 candidate.
struct OriginalEconomyStarterReceipt {
    Cell origin{};
    Position focus{};
    std::array<BuildingId,4> house_ids{};
    std::uint64_t construction_spent=0;
    std::uint64_t commands_executed=0;
};

OriginalEconomyStarterReceipt place_original_economy_starter(World& world);

} // namespace openemperor::simulation
