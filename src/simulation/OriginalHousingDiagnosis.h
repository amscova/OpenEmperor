#pragma once

#include "simulation/MapPermissions.h"
#include "simulation/original/HouseEvolution.h"

#include <array>
#include <cstdint>
#include <string_view>

namespace openemperor::simulation {

// Read-only observations, never saved authority or an instruction to evolve.
struct OriginalHousingRequirement {
    original::HouseField field=original::HouseField::Water;
    std::int32_t required=0,actual=0,alternative=0;
    bool presence=false,met=false;
    bool operator==(const OriginalHousingRequirement&) const = default;
};
struct OriginalHousingRowDiagnosis {
    original::ModelHouse row=original::ModelHouse::Shelter;
    original::HouseType hypothetical_type=original::HouseType::Shelter;
    std::int32_t low=0,high=0,row_capacity=0;
    std::array<OriginalHousingRequirement,15> requirements{};
    bool needs_met=false,same_family=false;
    original::Verdict verdict=original::Verdict::Stay;
    original::HouseType pure_result_type=original::HouseType::Shelter;
    std::uint8_t pure_result_counter=0;
    bool operator==(const OriginalHousingRowDiagnosis&) const = default;
};
struct OriginalHousingDiagnosis {
    BuildingId building_id=static_cast<BuildingId>(0);
    original::HouseState actual;
    bool elite_family=false;
    original::ModelHouse current_row=original::ModelHouse::Shelter,next_row=original::ModelHouse::Hut;
    std::int32_t cached_desirability=0,admission_capacity=0,census_capacity=0;
    OriginalHousingRowDiagnosis current,next;
    original::HouseType pure_result_type=original::HouseType::Shelter;
    std::uint8_t pure_result_counter=0,devolve_limit=0;
    bool awaiting_admission=false,common_terminal=false,elite_terminal=false;
    bool geometry_authored_2x2=true,geometry_deferred=false;
    std::uint8_t actual_footprint_side=2,original_required_side=2;
    bool materialization_pending=false,materialization_ready=false;
    BuildDiagnostic materialization_diagnostic;
    std::int32_t raw_desirability=0,cardinal_water_bonus=0;
    bool operator==(const OriginalHousingDiagnosis&) const = default;
};

// Labels describe the checked enum, not a selected Original sprite.
[[nodiscard]] std::string_view original_housing_type_name(original::HouseType type);
[[nodiscard]] std::string_view original_housing_requirement_name(original::HouseField field);

} // namespace openemperor::simulation
