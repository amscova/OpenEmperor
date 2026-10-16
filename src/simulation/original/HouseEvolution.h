#pragma once

#include "simulation/original/ModelSchema.h"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace openemperor::original {

// Original building types, distinct from ModelHouse rows and World levels.
enum class HouseType : std::uint16_t {
    Vacant = 2, Shelter = 3, Hut = 4, PlainCottage = 5,
    AttractiveCottage = 6, SpaciousDwelling = 7, ElegantDwelling = 8,
    OrnateApartment = 9, LuxuriousApartment = 10, EliteUnoccupied = 11,
    EliteAbandoned = 12, ModestSiheyuan = 13, LavishSiheyuan = 14,
    HumbleCompound = 15, ImpressiveCompound = 16, HeavenlyCompound = 17
};

// Bounded original storage domains. Access bytes are nonzero predicates;
// stocks and occupants are signed words. No grants, decay or delivery here.
struct HouseState {
    HouseType type = HouseType::Shelter;
    std::int8_t desirability = 0;
    std::int16_t occupants = 0;
    std::uint8_t devolve_counter = 0;
    bool water = false;
    std::uint8_t herbalist = 0, acupuncture = 0, music = 0, acrobat = 0, drama = 0;
    std::uint8_t ancestor = 0, confucian = 0, daoist = 0, buddhist = 0;
    std::uint8_t food_quality = 0;
    std::int16_t hemp = 0, ceramics = 0, tea = 0, silk = 0;
    std::int16_t bronzeware = 0, lacquerware = 0;

    bool operator==(const HouseState&) const = default;
};

// Explicit values of two independently observed original global gates.
// Their game triggers are unknown; the kernel never reads global state.
struct EvolutionContext {
    bool evaluation_blocked = false;
    bool force_desirability_evolve = false;
};

// Eager, immutable preparation of all 15x24 named house values. Construction
// validates every label, field, difficulty and checked modifier before use.
// This is numerical evidence only, with no World, save, SDL or clock hooks.
class HouseEvolutionRules {
public:
    HouseEvolutionRules(const ModelData& model, Difficulty difficulty);
    [[nodiscard]] std::int32_t value(ModelHouse row, HouseField field) const;
    [[nodiscard]] std::int32_t row_capacity(ModelHouse row) const;
    [[nodiscard]] Difficulty difficulty() const noexcept { return difficulty_; }
    [[nodiscard]] std::string_view model_sha256() const noexcept { return model_sha256_; }

private:
    std::array<HouseRow, house_row_count> rows_{};
    Difficulty difficulty_;
    std::string model_sha256_;
};

enum class Verdict : std::int8_t { Devolve = -1, Stay = 0, Evolve = 1 };

[[nodiscard]] bool is_house_type(HouseType type) noexcept;
[[nodiscard]] bool is_elite(HouseType type);
[[nodiscard]] ModelHouse house_row(HouseType type); // Vacant explicitly aliases Shelter.
[[nodiscard]] ModelHouse next_row(HouseType type);

// Narrow model-row selectors observed in distinct original consumers. These
// do not include census eligibility/occupant overrides or actor admission.
// EliteUnoccupied selects different rows in the two paths; no area scaling.
// The stored row is assumed canonical for the supplied type.
[[nodiscard]] ModelHouse census_capacity_row(HouseType type);
[[nodiscard]] ModelHouse admission_capacity_row(HouseType type);

[[nodiscard]] bool needs_met(const HouseEvolutionRules& rules, ModelHouse row,
                              const HouseState& state);
[[nodiscard]] Verdict desirability_verdict(const HouseEvolutionRules& rules,
                                           const HouseState& state,
                                           EvolutionContext context = {});
// Composition used by per-type updates, not the separate original diagnostic
// routine. EliteAbandoned ignores desirability; current needs always win.
// evaluation_blocked is applied only by evolve_step, not these table queries.
[[nodiscard]] Verdict evolution_verdict(const HouseEvolutionRules& rules,
                                         const HouseState& state,
                                         EvolutionContext context = {});
[[nodiscard]] std::uint8_t devolve_limit(HouseType type);
[[nodiscard]] std::optional<HouseType> evolve_target(HouseType type);
[[nodiscard]] std::optional<HouseType> devolve_target(HouseType type);

// One evaluation, returning a new value and leaving the input untouched.
// No population, service, goods, geometry or time mutation is implied.
[[nodiscard]] HouseState evolve_step(const HouseEvolutionRules& rules,
                                     const HouseState& state,
                                     EvolutionContext context = {});

} // namespace openemperor::original
