#pragma once

#include "simulation/original/HouseEvolution.h"

#include <array>
#include <cstdint>
#include <memory>

namespace openemperor::original {

// One immutable, eagerly checked interpretation of a successfully loaded file.
// No World, path lookup, source parsing or mutable global cache is retained.
class OriginalRulesModel final {
public:
    [[nodiscard]] static std::shared_ptr<const OriginalRulesModel> prepare(
        ModelData model,Difficulty difficulty);
    // Explicit active-generation capability. The historical seven-row factory
    // does not require, read or validate the additional production records.
    [[nodiscard]] static std::shared_ptr<const OriginalRulesModel> prepare_production(
        ModelData model,Difficulty difficulty);
    OriginalRulesModel(const OriginalRulesModel&)=delete;
    OriginalRulesModel(OriginalRulesModel&&)=delete;
    OriginalRulesModel& operator=(const OriginalRulesModel&)=delete;
    OriginalRulesModel& operator=(OriginalRulesModel&&)=delete;
    ~OriginalRulesModel()=default;

    [[nodiscard]] const ModelProvenance& provenance() const noexcept { return provenance_; }
    [[nodiscard]] Difficulty difficulty() const noexcept { return difficulty_; }
    [[nodiscard]] const HouseEvolutionRules& house_rules() const noexcept { return house_rules_; }
    [[nodiscard]] std::int32_t building_value(ModelBuilding building,BuildingField field) const;
    [[nodiscard]] bool supports_building(ModelBuilding building) const noexcept;

private:
    OriginalRulesModel(const ModelData& model,Difficulty difficulty,bool production);
    const ModelProvenance provenance_;
    const Difficulty difficulty_;
    const HouseEvolutionRules house_rules_;
    const std::array<BuildingRow,7> building_rows_;
    const bool production_prepared_;
    const std::array<BuildingRow,6> production_rows_;
};

} // namespace openemperor::original
