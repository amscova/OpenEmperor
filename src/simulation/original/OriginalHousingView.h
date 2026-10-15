#pragma once
// Read-only diagnosis: what the original-style desirability and house
// evolution rules would make of an existing World. Nothing here mutates the
// World, its saves or its rules.
#include "simulation/World.h"
#include "simulation/original/HouseEvolution.h"
#include "simulation/original/ModelData.h"

#include <vector>

namespace openemperor::original {

struct OriginalHousingOptions {
    int difficulty = 2;  // 0 Very Easy .. 4 Very Hard
    // Food quality is not modelled by the fork. A house that holds food counts
    // as having this quality; 0 means "never meets a food requirement".
    int food_quality_when_fed = 0;
    int evolution_steps = 500;  // cycles simulated per house
};

struct OriginalHouseReport {
    simulation::BuildingId id{};
    int desirability = 0;       // original-style, signed, 0 at the clamp ends of the map
    int peak_type = HT_Shelter;     // highest type reached while simulating
    int settled_type = HT_Shelter;  // type after the last simulated cycle
};

// Which original model row a fork building kind stands in for (-1 = none).
// This is an approximation by function, not an identification.
int original_model_type(simulation::Object kind);

std::vector<OriginalHouseReport> evaluate_original_housing(
    const simulation::World& world, const ModelData& model,
    const OriginalHousingOptions& options = {});

}  // namespace openemperor::original
