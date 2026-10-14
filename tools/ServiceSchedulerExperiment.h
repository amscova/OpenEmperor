#pragma once
// Only the explicitly excluded diagnostic executable defines the matching macro.
// No scheduler state, hook or selection change exists in the production app.
#include "simulation/World.h"
#include <span>
#include <string_view>

namespace openemperor::service_experiment {
struct Candidate { simulation::BuildingId id; std::size_t edges; };
void configure(std::string_view name);
bool enabled();
std::optional<simulation::BuildingId> choose(const simulation::World& world,
    const simulation::CourierState& courier,std::span<const Candidate> reachable);
} // namespace openemperor::service_experiment
