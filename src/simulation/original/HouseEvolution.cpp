#include "simulation/original/HouseEvolution.h"

#include <algorithm>
#include <stdexcept>

namespace openemperor::original {
namespace {

void validate_type(HouseType type) {
    if (!is_house_type(type)) throw std::out_of_range("invalid original house type");
}

} // namespace

HouseEvolutionRules::HouseEvolutionRules(const ModelData& model, Difficulty difficulty)
    : difficulty_(difficulty), model_sha256_(model.provenance().sha256) {
    for (std::size_t row = 0; row < house_row_count; ++row) {
        for (std::size_t field = 0; field < house_field_count; ++field) {
            rows_[row][field] = house_value(model, static_cast<ModelHouse>(row),
                                            static_cast<HouseField>(field), difficulty);
        }
    }
}

std::int32_t HouseEvolutionRules::value(ModelHouse row, HouseField field) const {
    const auto column = static_cast<std::size_t>(field);
    if (column >= house_field_count) throw std::out_of_range("invalid original house field");
    return rows_.at(model_row(row).value).at(column);
}

std::int32_t HouseEvolutionRules::row_capacity(ModelHouse row) const {
    return value(row, HouseField::Capacity);
}

bool is_house_type(HouseType type) noexcept {
    return type >= HouseType::Vacant && type <= HouseType::HeavenlyCompound;
}

bool is_elite(HouseType type) {
    validate_type(type);
    return type >= HouseType::EliteUnoccupied;
}

ModelHouse house_row(HouseType type) {
    validate_type(type);
    const auto id = static_cast<std::uint16_t>(type);
    return static_cast<ModelHouse>(id <= 3 ? 0 : id - 3);
}

ModelHouse next_row(HouseType type) {
    const auto row = house_row(type);
    if (type == HouseType::EliteUnoccupied) return ModelHouse::ModestSiheyuan;
    if (type == HouseType::HeavenlyCompound) return row;
    return static_cast<ModelHouse>(model_row(row).value + 1U);
}

ModelHouse census_capacity_row(HouseType type) {
    const auto row = house_row(type);
    return type == HouseType::EliteUnoccupied ? ModelHouse::EliteAbandoned : row;
}

ModelHouse admission_capacity_row(HouseType type) {
    const auto row = house_row(type);
    return type == HouseType::EliteUnoccupied ? ModelHouse::LavishSiheyuan : row;
}

bool needs_met(const HouseEvolutionRules& rules, ModelHouse row, const HouseState& state) {
    validate_type(state.type);
    const auto v = [&](HouseField field) { return rules.value(row, field); };
    // A negative prepared requirement is still nonzero. Access itself is a
    // byte predicate, not a signed timer comparison or a grant of service.
    if (v(HouseField::Water) != 0 && !state.water) return false;
    if (v(HouseField::Herbalist) != 0 && state.herbalist == 0) return false;
    if (v(HouseField::Acupuncture) != 0 && state.acupuncture == 0) return false;
    if (v(HouseField::Music) != 0 && state.music == 0) return false;
    if (v(HouseField::Acrobat) != 0 && state.acrobat == 0) return false;
    if (v(HouseField::Drama) != 0 && state.drama == 0) return false;
    if (v(HouseField::Ancestor) != 0 && state.ancestor == 0) return false;
    if (v(HouseField::Confucian) != 0 && state.confucian == 0) return false;
    if (v(HouseField::DaoistOrBuddhist) != 0 && state.daoist == 0 && state.buddhist == 0)
        return false;
    if (state.food_quality < v(HouseField::FoodQuality)) return false;
    if (state.hemp < v(HouseField::Hemp)) return false;
    if (state.ceramics < v(HouseField::Ceramics)) return false;
    if (state.tea < v(HouseField::Tea)) return false;
    if (state.silk < v(HouseField::Silk)) return false;
    return std::max(state.bronzeware, state.lacquerware) >= v(HouseField::BronzewareOrLacquerware);
}

Verdict desirability_verdict(const HouseEvolutionRules& rules, const HouseState& state,
                             EvolutionContext context) {
    const auto row = house_row(state.type);
    if (context.force_desirability_evolve) return Verdict::Evolve;
    if (state.desirability <= rules.value(row, HouseField::DesirabilityLow)) return Verdict::Devolve;
    if (state.desirability >= rules.value(row, HouseField::DesirabilityHigh)) return Verdict::Evolve;
    return Verdict::Stay;
}

Verdict evolution_verdict(const HouseEvolutionRules& rules, const HouseState& state,
                           EvolutionContext context) {
    const auto row = house_row(state.type);
    const auto verdict = state.type == HouseType::EliteAbandoned ? Verdict::Evolve
                      : desirability_verdict(rules, state, context);
    if (!needs_met(rules, row, state)) return Verdict::Devolve;
    if (verdict == Verdict::Evolve && !needs_met(rules, next_row(state.type), state))
        return Verdict::Stay;
    return verdict;
}

std::uint8_t devolve_limit(HouseType type) {
    validate_type(type);
    if (type >= HouseType::Shelter && type <= HouseType::LuxuriousApartment) return 9;
    if (type == HouseType::EliteAbandoned) return 11;
    if (type == HouseType::ModestSiheyuan) return 49;
    if (type >= HouseType::LavishSiheyuan) return 17;
    return 0;
}

std::optional<HouseType> evolve_target(HouseType type) {
    validate_type(type);
    if ((type >= HouseType::Vacant && type <= HouseType::OrnateApartment) ||
        (type >= HouseType::EliteAbandoned && type <= HouseType::ImpressiveCompound)) {
        return static_cast<HouseType>(static_cast<std::uint16_t>(type) + 1U);
    }
    return std::nullopt;
}

std::optional<HouseType> devolve_target(HouseType type) {
    validate_type(type);
    if ((type >= HouseType::Hut && type <= HouseType::LuxuriousApartment) ||
        (type >= HouseType::ModestSiheyuan && type <= HouseType::HeavenlyCompound)) {
        return static_cast<HouseType>(static_cast<std::uint16_t>(type) - 1U);
    }
    return std::nullopt;
}

HouseState evolve_step(const HouseEvolutionRules& rules, const HouseState& state,
                       EvolutionContext context) {
    validate_type(state.type);
    // Unoccupied calls next-row needs and discards its Boolean result. Its
    // type/counter do not change; this kernel models no additional observer.
    if (state.type == HouseType::EliteUnoccupied) return state;
    if ((state.type == HouseType::Vacant || state.type == HouseType::Shelter) &&
        state.occupants <= 0) return state;
    if (context.evaluation_blocked) return state;

    const auto verdict = evolution_verdict(rules, state, context);
    auto next = state;
    if (state.type == HouseType::Vacant) {
        if (verdict == Verdict::Evolve) next.type = HouseType::Shelter;
        return next; // Vacant never changes the devolution counter.
    }
    if (verdict == Verdict::Evolve) {
        next.devolve_counter = 0;
        if (const auto target = evolve_target(state.type)) next.type = *target;
    } else if (verdict == Verdict::Stay) {
        next.devolve_counter = 0;
    } else if (state.devolve_counter < devolve_limit(state.type)) {
        ++next.devolve_counter;
    } else {
        next.devolve_counter = 0;
        if (const auto target = devolve_target(state.type)) next.type = *target;
    }
    return next;
}

} // namespace openemperor::original
