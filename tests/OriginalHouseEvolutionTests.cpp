#include "simulation/original/HouseEvolution.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace {
namespace original = openemperor::original;
namespace fs = std::filesystem;
using original::Difficulty;
using original::EvolutionContext;
using original::HouseEvolutionRules;
using original::HouseField;
using original::HouseState;
using original::HouseType;
using original::ModelHouse;
using original::Verdict;

std::size_t checks = 0;
void check(bool condition, std::string_view message) {
    ++checks;
    if (!condition) throw std::runtime_error(std::string(message));
}

template<typename Error, typename Operation>
void expect_exception(Operation operation, std::string_view message) {
    try { operation(); }
    catch (const Error&) { ++checks; return; }
    throw std::runtime_error(std::string(message));
}

constexpr std::array difficulties{
    Difficulty::VeryEasy, Difficulty::Easy, Difficulty::Normal,
    Difficulty::Hard, Difficulty::VeryHard
};
constexpr std::array<std::string_view, 5> difficulty_labels{
    "Very Easy Modifiers", "Easy Modifiers", "Normal Modifier",
    "Hard Modifiers", "Very Hard Modifiers"
};
constexpr std::array<std::string_view, 15> labels{
    "1: Shelter", "2: Hut", "3: Plain Cottage", "4: Attractive Cottage",
    "5: Spacious Dwelling", "6: Elegant Dwelling", "7: Ornate Apartment",
    "8: Lux. Apartment", "Elite: Unoccupied", "Elite 0: Abandoned",
    "Elite 1: Modest Siheyuan", "Elite 2: Lavish Siheyuan",
    "Elite 3: Humble Compound", "Elite 4: Impressive Compound",
    "Elite 5: Heavenly Compound"
};

// Literal original type/row and control-flow boundaries. The numbers are
// identities and EXE-observed branch constants, not a copied balance table.
struct Stage {
    HouseType type;
    std::uint16_t id;
    std::uint8_t row, next, census, admission, limit;
    int evolve, devolve;
    bool elite;
};
constexpr std::array<Stage, 16> stages{{
    {HouseType::Vacant, 2, 0, 1, 0, 0, 0, 3, -1, false},
    {HouseType::Shelter, 3, 0, 1, 0, 0, 9, 4, -1, false},
    {HouseType::Hut, 4, 1, 2, 1, 1, 9, 5, 3, false},
    {HouseType::PlainCottage, 5, 2, 3, 2, 2, 9, 6, 4, false},
    {HouseType::AttractiveCottage, 6, 3, 4, 3, 3, 9, 7, 5, false},
    {HouseType::SpaciousDwelling, 7, 4, 5, 4, 4, 9, 8, 6, false},
    {HouseType::ElegantDwelling, 8, 5, 6, 5, 5, 9, 9, 7, false},
    {HouseType::OrnateApartment, 9, 6, 7, 6, 6, 9, 10, 8, false},
    {HouseType::LuxuriousApartment, 10, 7, 8, 7, 7, 9, -1, 9, false},
    {HouseType::EliteUnoccupied, 11, 8, 10, 9, 11, 0, -1, -1, true},
    {HouseType::EliteAbandoned, 12, 9, 10, 9, 9, 11, 13, -1, true},
    {HouseType::ModestSiheyuan, 13, 10, 11, 10, 10, 49, 14, 12, true},
    {HouseType::LavishSiheyuan, 14, 11, 12, 11, 11, 17, 15, 13, true},
    {HouseType::HumbleCompound, 15, 12, 13, 12, 12, 17, 16, 14, true},
    {HouseType::ImpressiveCompound, 16, 13, 14, 13, 13, 17, 17, 15, true},
    {HouseType::HeavenlyCompound, 17, 14, 14, 14, 14, 17, -1, 16, true}
}};

template<std::size_t N>
std::string row(std::string_view label, const std::array<std::int32_t, N>& values) {
    std::string result(label);
    result += ",{,";
    for (const auto value : values) result += std::to_string(value) + ',';
    return result + "},,,\n";
}

struct Fixture {
    std::array<original::HouseRow, 15> values{};
    std::array<original::HouseRow, 5> modifiers{};
    std::array<std::string, 15> names{};

    Fixture() {
        for (std::size_t i = 0; i < values.size(); ++i) {
            names[i] = labels[i];
            values[i][static_cast<std::size_t>(HouseField::DesirabilityLow)] = -10;
            values[i][static_cast<std::size_t>(HouseField::DesirabilityHigh)] = 10;
            values[i][static_cast<std::size_t>(HouseField::Capacity)] = static_cast<std::int32_t>(i + 3);
        }
    }

    std::int32_t& at(std::size_t house, HouseField field) {
        return values.at(house).at(static_cast<std::size_t>(field));
    }

    std::string text() const {
        std::string result = "Authored house evolution fixture\nBUILDING MODS\n";
        original::BuildingRow percentages{};
        percentages.fill(100);
        for (std::size_t i = 0; i < difficulty_labels.size(); ++i)
            result += row(std::to_string(i) + ',' + std::string(difficulty_labels[i]), percentages);
        result += "ALL BUILDINGS\n";
        result += row("72,Well", original::BuildingRow{});
        result += "HOUSE MODS\n";
        for (std::size_t i = 0; i < difficulty_labels.size(); ++i)
            result += row(difficulty_labels[i], modifiers[i]);
        result += "ALL HOUSES\n";
        for (std::size_t i = 0; i < names.size(); ++i) result += row(names[i], values[i]);
        return result + "End of model data.\n-------------------\n";
    }
};

original::ModelData parse(const Fixture& fixture) {
    auto parsed = original::parse_model_data(fixture.text());
    check(static_cast<bool>(parsed), parsed.error ? parsed.error->reason : "authored fixture parsed");
    return std::move(*parsed.model);
}

HouseState supplied(HouseType type = HouseType::Hut) {
    HouseState state;
    state.type = type;
    state.occupants = 1;
    state.water = true;
    state.herbalist = 255;
    state.acupuncture = 255;
    state.music = 255;
    state.acrobat = 255;
    state.drama = 255;
    state.ancestor = 255;
    state.confucian = 255;
    state.daoist = 255;
    state.buddhist = 255;
    state.food_quality = 255;
    state.hemp = 32767;
    state.ceramics = 32767;
    state.tea = 32767;
    state.silk = 32767;
    state.bronzeware = 32767;
    state.lacquerware = 32767;
    return state;
}

void identities_and_preparation() {
    for (const auto& stage : stages) {
        check(static_cast<std::uint16_t>(stage.type) == stage.id, "literal original house type identity");
        check(original::is_house_type(stage.type), "every original type is a house");
        check(original::is_elite(stage.type) == stage.elite, "literal elite boundary");
        check(static_cast<std::uint8_t>(original::house_row(stage.type)) == stage.row, "literal current model row");
        check(static_cast<std::uint8_t>(original::next_row(stage.type)) == stage.next, "literal next model row including elite skips");
        check(static_cast<std::uint8_t>(original::census_capacity_row(stage.type)) == stage.census, "distinct original census row selector");
        check(static_cast<std::uint8_t>(original::admission_capacity_row(stage.type)) == stage.admission, "distinct original admission row selector");
        check(original::devolve_limit(stage.type) == stage.limit, "literal devolution tolerance");
        const auto up = original::evolve_target(stage.type);
        const auto down = original::devolve_target(stage.type);
        check(up ? static_cast<int>(*up) == stage.evolve : stage.evolve == -1, "literal evolution target or no target");
        check(down ? static_cast<int>(*down) == stage.devolve : stage.devolve == -1, "literal devolution target or no target");
    }

    Fixture fixture;
    for (std::size_t h = 0; h < fixture.values.size(); ++h)
        for (std::size_t f = 0; f < original::house_field_count; ++f)
            fixture.values[h][f] = static_cast<std::int32_t>(h * 19 + f * 7) - 121;
    for (std::size_t d = 0; d < difficulties.size(); ++d)
        for (std::size_t f = 0; f < original::house_field_count; ++f)
            fixture.modifiers[d][f] = static_cast<std::int32_t>(d * 13 + f) - 17;
    const auto model = parse(fixture);
    for (std::size_t d = 0; d < difficulties.size(); ++d) {
        const HouseEvolutionRules rules(model, difficulties[d]);
        check(rules.difficulty() == difficulties[d], "prepared difficulty retained");
        check(rules.model_sha256() == model.provenance().sha256, "prepared rules retain model identity");
        for (std::size_t h = 0; h < fixture.values.size(); ++h) {
            const auto house = static_cast<ModelHouse>(h);
            for (std::size_t f = 0; f < original::house_field_count; ++f) {
                const auto expected = std::clamp(static_cast<std::int64_t>(fixture.values[h][f]) + fixture.modifiers[d][f],
                                                 std::int64_t{-99}, std::int64_t{100});
                check(rules.value(house, static_cast<HouseField>(f)) == expected,
                      "every prepared field agrees with independent additive clamping");
            }
            check(rules.row_capacity(house) == rules.value(house, HouseField::Capacity),
                  "row capacity is the prepared count without an invented footprint scale");
        }
    }
    Fixture negative_capacity;
    negative_capacity.at(14, HouseField::Capacity) = -4;
    const HouseEvolutionRules signed_rules(parse(negative_capacity), Difficulty::Normal);
    check(signed_rules.row_capacity(ModelHouse::HeavenlyCompound) == -4,
          "raw modified negative capacity is not silently repaired into world authority");
}

struct AccessNeed { HouseField field; std::uint8_t HouseState::*member; };
constexpr std::array<AccessNeed, 7> access_needs{{
    {HouseField::Herbalist, &HouseState::herbalist},
    {HouseField::Acupuncture, &HouseState::acupuncture},
    {HouseField::Music, &HouseState::music},
    {HouseField::Acrobat, &HouseState::acrobat},
    {HouseField::Drama, &HouseState::drama},
    {HouseField::Ancestor, &HouseState::ancestor},
    {HouseField::Confucian, &HouseState::confucian}
}};
struct StockNeed { HouseField field; std::int16_t HouseState::*member; };
constexpr std::array<StockNeed, 4> stock_needs{{
    {HouseField::Hemp, &HouseState::hemp},
    {HouseField::Ceramics, &HouseState::ceramics},
    {HouseField::Tea, &HouseState::tea},
    {HouseField::Silk, &HouseState::silk}
}};

void independent_need_boundaries() {
    for (std::size_t row_index = 0; row_index < original::house_row_count; ++row_index) {
        const auto house = static_cast<ModelHouse>(row_index);
        for (const auto requirement : {-1, 1, 73}) {
            for (const auto& need : access_needs) {
                Fixture fixture;
                fixture.at(row_index, need.field) = requirement;
                const HouseEvolutionRules rules(parse(fixture), Difficulty::Normal);
                auto state = supplied();
                state.*(need.member) = 0;
                check(!original::needs_met(rules, house, state), "each absent specific access fails even a negative nonzero requirement");
                for (const auto timer : std::array<std::uint8_t, 3>{1, 96, 255}) {
                    state.*(need.member) = timer;
                    check(original::needs_met(rules, house, state), "each access uses byte nonzero, not duration or numeric requirement");
                }
            }
            Fixture water;
            water.at(row_index, HouseField::Water) = requirement;
            const HouseEvolutionRules water_rules(parse(water), Difficulty::Normal);
            auto state = supplied();
            state.water = false;
            check(!original::needs_met(water_rules, house, state), "water requirement is nonzero predicate");
            state.water = true;
            check(original::needs_met(water_rules, house, state), "water presence satisfies requirement");

            Fixture religion;
            religion.at(row_index, HouseField::DaoistOrBuddhist) = requirement;
            const HouseEvolutionRules religion_rules(parse(religion), Difficulty::Normal);
            state.daoist = 0;
            state.buddhist = 0;
            check(!original::needs_met(religion_rules, house, state), "either religion requirement fails when both absent");
            state.daoist = 255;
            check(original::needs_met(religion_rules, house, state), "Daoist alone satisfies shared religion column");
            state.daoist = 0;
            state.buddhist = 1;
            check(original::needs_met(religion_rules, house, state), "Buddhist alone satisfies shared religion column");
        }
        for (const auto& need : stock_needs) {
            Fixture fixture;
            fixture.at(row_index, need.field) = 23;
            const HouseEvolutionRules rules(parse(fixture), Difficulty::Normal);
            auto state = supplied();
            for (const auto stock : std::array<std::int16_t, 4>{-32768, -1, 0, 22}) {
                state.*(need.member) = stock;
                check(!original::needs_met(rules, house, state), "each signed good below threshold fails");
            }
            for (const auto stock : std::array<std::int16_t, 3>{23, 24, 32767}) {
                state.*(need.member) = stock;
                check(original::needs_met(rules, house, state), "each signed good at or above threshold passes");
            }
        }
        Fixture quality;
        quality.at(row_index, HouseField::FoodQuality) = 37;
        const HouseEvolutionRules quality_rules(parse(quality), Difficulty::Normal);
        auto state = supplied();
        for (const auto value : std::array<std::uint8_t, 3>{0, 36, 37}) {
            state.food_quality = value;
            check(original::needs_met(quality_rules, house, state) == (value >= 37),
                  "food quality has an inclusive numeric threshold");
        }
        state.food_quality = 255;
        check(original::needs_met(quality_rules, house, state), "food quality is unsigned, not signed byte");

        Fixture alternatives;
        alternatives.at(row_index, HouseField::BronzewareOrLacquerware) = 23;
        const HouseEvolutionRules either_rules(parse(alternatives), Difficulty::Normal);
        state = supplied();
        for (const auto stocks : std::array<std::array<std::int16_t, 2>, 7>{{
                 {{0, 0}}, {{12, 12}}, {{22, 22}}, {{-32768, -32768}},
                 {{23, -32768}}, {{-32768, 23}}, {{32767, 32767}}
             }}) {
            state.bronzeware = stocks[0];
            state.lacquerware = stocks[1];
            check(original::needs_met(either_rules, house, state) == (stocks[0] >= 23 || stocks[1] >= 23),
                  "luxury alternatives compare independently and never sum stocks");
        }
    }

    Fixture numeric_negative;
    for (auto& values : numeric_negative.values) {
        values[static_cast<std::size_t>(HouseField::FoodQuality)] = -1;
        for (const auto& need : stock_needs) values[static_cast<std::size_t>(need.field)] = -1;
        values[static_cast<std::size_t>(HouseField::BronzewareOrLacquerware)] = -1;
    }
    const HouseEvolutionRules negative_rules(parse(numeric_negative), Difficulty::Normal);
    HouseState empty;
    check(original::needs_met(negative_rules, ModelHouse::HeavenlyCompound, empty),
          "zero satisfies negative numeric requirements while negative access remains a presence check");
    for (const auto& need : stock_needs) {
        auto state = empty;
        state.*(need.member) = -2;
        check(!original::needs_met(negative_rules, ModelHouse::HeavenlyCompound, state),
              "negative numeric goods requirement is still compared against lower signed stock");
        state.*(need.member) = -1;
        check(original::needs_met(negative_rules, ModelHouse::HeavenlyCompound, state),
              "negative goods requirement accepts exact signed equality");
    }
    empty.bronzeware = -2;
    empty.lacquerware = -2;
    check(!original::needs_met(negative_rules, ModelHouse::HeavenlyCompound, empty),
          "negative luxury requirement is not skipped when both stocks are lower");
    empty.bronzeware = -1;
    check(original::needs_met(negative_rules, ModelHouse::HeavenlyCompound, empty),
          "bronze signed equality satisfies negative alternative requirement");
    empty.bronzeware = -2;
    empty.lacquerware = -1;
    check(original::needs_met(negative_rules, ModelHouse::HeavenlyCompound, empty),
          "lacquer signed equality satisfies negative alternative requirement");
    empty = HouseState{};
    const HouseEvolutionRules zero_rules(parse(Fixture{}), Difficulty::Normal);
    check(original::needs_met(zero_rules, ModelHouse::HeavenlyCompound, empty), "zero access requirements accept absent services");
    empty.hemp = -1;
    check(!original::needs_met(zero_rules, ModelHouse::HeavenlyCompound, empty),
          "numeric zero requirement still rejects a negative stored good");
}

void desirability_and_need_precedence() {
    const HouseEvolutionRules rules(parse(Fixture{}), Difficulty::Normal);
    for (const auto& stage : stages) {
        for (const auto point : std::array<std::pair<std::int8_t, Verdict>, 7>{{
                 {-128, Verdict::Devolve}, {-11, Verdict::Devolve}, {-10, Verdict::Devolve},
                 {-9, Verdict::Stay}, {9, Verdict::Stay}, {10, Verdict::Evolve}, {127, Verdict::Evolve}
             }}) {
            auto state = supplied(stage.type);
            state.desirability = point.first;
            const auto before = state;
            check(original::desirability_verdict(rules, state) == point.second,
                  "each type has inclusive low/high and strict interior desirability boundaries");
            check(original::desirability_verdict(rules, state, {true, false}) == point.second,
                  "raw desirability query deliberately ignores the outer evaluation gate");
            check(original::evolution_verdict(rules, state, {true, false}) ==
                      (stage.type == HouseType::EliteAbandoned ? Verdict::Evolve : point.second),
                  "pure combined query retains result under outer gate which only step applies");
            check(state == before, "desirability query never changes state");
        }
    }
    for (const auto thresholds : std::array<std::array<int, 2>, 3>{{{{0, 0}}, {{11, -11}}, {{100, -99}}}}) {
        Fixture fixture;
        fixture.at(1, HouseField::DesirabilityLow) = thresholds[0];
        fixture.at(1, HouseField::DesirabilityHigh) = thresholds[1];
        const HouseEvolutionRules overlap(parse(fixture), Difficulty::Normal);
        auto state = supplied();
        state.desirability = 0;
        check(original::desirability_verdict(overlap, state) == Verdict::Devolve,
              "low/devolve branch takes precedence when thresholds overlap");
        check(original::desirability_verdict(overlap, state, {false, true}) == Verdict::Evolve,
              "explicit force flag bypasses only desirability branches");
    }

    constexpr std::array need_fields{
        HouseField::Water, HouseField::Herbalist, HouseField::Acupuncture,
        HouseField::Music, HouseField::Acrobat, HouseField::Drama,
        HouseField::Ancestor, HouseField::Confucian, HouseField::DaoistOrBuddhist,
        HouseField::FoodQuality, HouseField::Hemp, HouseField::Ceramics,
        HouseField::Tea, HouseField::Silk, HouseField::BronzewareOrLacquerware
    };
    for (const auto field : need_fields) {
        Fixture fixture;
        fixture.at(1, field) = 1;
        const HouseEvolutionRules current_missing(parse(fixture), Difficulty::Normal);
        HouseState state;
        state.type = HouseType::Hut;
        state.occupants = 1;
        state.desirability = 127;
        state.devolve_counter = 9;
        const auto before = state;
        check(original::evolution_verdict(current_missing, state) == Verdict::Devolve,
              "each current need defeats maximum desirability");
        check(original::evolution_verdict(current_missing, state, {false, true}) == Verdict::Devolve,
              "force desirability cannot satisfy any current need");
        auto expected = state;
        expected.type = HouseType::Shelter;
        expected.devolve_counter = 0;
        check(original::evolve_step(current_missing, state) == expected,
              "missing current need at tolerance boundary really demotes");
        check(state == before, "pure update leaves original state unchanged");

        fixture.at(1, field) = 0;
        fixture.at(2, field) = 1;
        const HouseEvolutionRules next_missing(parse(fixture), Difficulty::Normal);
        check(original::evolution_verdict(next_missing, state) == Verdict::Stay,
              "each next-row need blocks high-score evolution without demotion");
        check(original::evolution_verdict(next_missing, state, {false, true}) == Verdict::Stay,
              "force desirability cannot satisfy any next-row need");
        expected = state;
        expected.devolve_counter = 0;
        check(original::evolve_step(next_missing, state) == expected, "next-row shortage stays and clears devolution streak");
    }
}

void step_boundaries_and_special_types() {
    const HouseEvolutionRules rules(parse(Fixture{}), Difficulty::Normal);
    for (const auto& stage : stages) {
        auto state = supplied(stage.type);
        state.devolve_counter = 255;
        state.desirability = 10;
        const auto before = state;
        auto expected = state;
        if (stage.type != HouseType::EliteUnoccupied) {
            if (stage.evolve >= 0) expected.type = static_cast<HouseType>(stage.evolve);
            if (stage.type != HouseType::Vacant) expected.devolve_counter = 0;
        }
        check(original::evolve_step(rules, state) == expected, "every type evolution boundary including no-target and counter exceptions");
        check(state == before, "successful pure step does not mutate caller state");

        state.desirability = 0;
        expected = state;
        if (stage.type == HouseType::EliteAbandoned) {
            expected.type = HouseType::ModestSiheyuan;
            expected.devolve_counter = 0;
        } else if (stage.type != HouseType::Vacant && stage.type != HouseType::EliteUnoccupied) {
            expected.devolve_counter = 0;
        }
        check(original::evolve_step(rules, state) == expected, "every type interior score with explicit abandoned override");

        state.desirability = -10;
        for (const auto counter : std::array<std::uint8_t, 4>{0, static_cast<std::uint8_t>(stage.limit == 0 ? 0 : stage.limit - 1), stage.limit, 255}) {
            state.devolve_counter = counter;
            expected = state;
            if (stage.type == HouseType::EliteAbandoned) {
                expected.type = HouseType::ModestSiheyuan;
                expected.devolve_counter = 0;
            } else if (stage.type != HouseType::Vacant && stage.type != HouseType::EliteUnoccupied) {
                if (counter < stage.limit) expected.devolve_counter = static_cast<std::uint8_t>(counter + 1);
                else {
                    expected.devolve_counter = 0;
                    if (stage.devolve >= 0) expected.type = static_cast<HouseType>(stage.devolve);
                }
            }
            check(original::evolve_step(rules, state) == expected, "every type devolution threshold, maximum counter and absent target");
        }
        for (const auto desire : std::array<std::int8_t, 3>{-128, 0, 127}) {
            state.desirability = desire;
            for (const auto force : {false, true})
                check(original::evolve_step(rules, state, {true, force}) == state,
                      "evaluation-blocked gate preserves every type and counter regardless of force");
        }
    }

    for (const auto& stage : stages) {
        Fixture current_shortage;
        current_shortage.at(stage.row, HouseField::Water) = 1;
        const HouseEvolutionRules current_rules(parse(current_shortage), Difficulty::Normal);
        auto state = supplied(stage.type);
        state.desirability = 127;
        state.devolve_counter = stage.limit;
        state.water = false;
        check(original::evolution_verdict(current_rules, state) == Verdict::Devolve,
              "every current row shortage wins over high desirability including elite and vacant aliases");
        auto expected = state;
        if (stage.type != HouseType::Vacant && stage.type != HouseType::EliteUnoccupied) {
            expected.devolve_counter = 0;
            if (stage.devolve >= 0) expected.type = static_cast<HouseType>(stage.devolve);
        }
        check(original::evolve_step(current_rules, state) == expected,
              "every type obeys current-need devolution boundaries or its explicit no-op exception");

        Fixture next_shortage;
        next_shortage.at(stage.next, HouseField::Water) = 1;
        const HouseEvolutionRules next_rules(parse(next_shortage), Difficulty::Normal);
        const auto same_row = stage.row == stage.next;
        check(original::evolution_verdict(next_rules, state) == (same_row ? Verdict::Devolve : Verdict::Stay),
              "literal next row shortage blocks evolution and top elite retains current-row precedence");
        expected = state;
        if (stage.type != HouseType::Vacant && stage.type != HouseType::EliteUnoccupied) {
            expected.devolve_counter = 0;
            if (same_row && stage.devolve >= 0) expected.type = static_cast<HouseType>(stage.devolve);
        }
        check(original::evolve_step(next_rules, state) == expected,
              "every type applies next-row shortage with its exact counter behavior");

        state = supplied(stage.type);
        state.desirability = -128;
        state.devolve_counter = 255;
        expected = state;
        if (stage.type != HouseType::EliteUnoccupied) {
            if (stage.evolve >= 0) expected.type = static_cast<HouseType>(stage.evolve);
            if (stage.type != HouseType::Vacant) expected.devolve_counter = 0;
        }
        check(original::evolve_step(rules, state, {false, true}) == expected,
              "explicit force permits supplied low-score evolution but preserves type-specific no-target exceptions");
    }

    for (const auto type : {HouseType::Vacant, HouseType::Shelter}) {
        auto state = supplied(type);
        state.desirability = 127;
        state.devolve_counter = 255;
        for (const auto occupants : std::array<std::int16_t, 3>{-32768, -1, 0}) {
            state.occupants = occupants;
            check(original::evolve_step(rules, state) == state, "empty or negative-occupant vacant/shelter update is a no-op");
        }
    }

    // Unlike Vacant/Shelter, these original handlers have no occupancy gate.
    // In particular an empty Abandoned can become Modest when needs are met.
    for (const auto& stage : stages) {
        if (stage.id <= 3 || stage.type == HouseType::EliteUnoccupied) continue;
        for (const auto occupants : std::array<std::int16_t, 2>{-32768, 0}) {
            auto empty = supplied(stage.type);
            empty.occupants = occupants;
            empty.desirability = 10;
            empty.devolve_counter = 255;
            auto result = empty;
            result.devolve_counter = 0;
            if (stage.evolve >= 0) result.type = static_cast<HouseType>(stage.evolve);
            check(original::evolve_step(rules, empty) == result,
                  "empty higher common and elite houses still evaluate evolution");

            empty.desirability = -10;
            empty.devolve_counter = stage.limit;
            result = empty;
            result.devolve_counter = 0;
            if (stage.type == HouseType::EliteAbandoned) result.type = HouseType::ModestSiheyuan;
            else if (stage.devolve >= 0) result.type = static_cast<HouseType>(stage.devolve);
            check(original::evolve_step(rules, empty) == result,
                  "empty higher houses retain devolution and Abandoned ignores low desirability");
        }
    }

    Fixture vacant;
    vacant.at(0, HouseField::Water) = 1;
    auto state = supplied(HouseType::Vacant);
    state.desirability = 127;
    state.devolve_counter = 199;
    state.water = false;
    check(original::evolve_step(HouseEvolutionRules(parse(vacant), Difficulty::Normal), state) == state,
          "occupied Vacant cannot become Shelter with missing current water");
    vacant.at(0, HouseField::Water) = 0;
    vacant.at(1, HouseField::Water) = 1;
    check(original::evolve_step(HouseEvolutionRules(parse(vacant), Difficulty::Normal), state) == state,
          "occupied Vacant tests actual next row rather than upgrading unconditionally");
    state.water = true;
    auto expected = state;
    expected.type = HouseType::Shelter;
    check(original::evolve_step(HouseEvolutionRules(parse(vacant), Difficulty::Normal), state) == expected,
          "successful Vacant upgrade preserves original counter and all resources");

    Fixture abandoned;
    abandoned.at(9, HouseField::Water) = 1;
    const HouseEvolutionRules abandoned_rules(parse(abandoned), Difficulty::Normal);
    state = supplied(HouseType::EliteAbandoned);
    state.water = false;
    state.desirability = 127;
    for (const auto counter : std::array<std::uint8_t, 4>{0, 10, 11, 255}) {
        state.devolve_counter = counter;
        expected = state;
        expected.devolve_counter = counter < 11 ? static_cast<std::uint8_t>(counter + 1) : 0;
        check(original::evolve_step(abandoned_rules, state) == expected,
              "Abandoned missing current needs uses its own counter but never demotes");
    }
    abandoned.at(9, HouseField::Water) = 0;
    abandoned.at(10, HouseField::Water) = 1;
    state.devolve_counter = 10;
    expected = state;
    expected.devolve_counter = 0;
    check(original::evolve_step(HouseEvolutionRules(parse(abandoned), Difficulty::Normal), state) == expected,
          "Abandoned ignores desirability but still requires next Modest row");

    for (const auto type : {HouseType::LuxuriousApartment, HouseType::HeavenlyCompound}) {
        Fixture top;
        const auto next = static_cast<std::size_t>(original::next_row(type));
        top.at(next, HouseField::Water) = 1;
        state = supplied(type);
        state.water = false;
        state.desirability = 127;
        const auto verdict = type == HouseType::HeavenlyCompound ? Verdict::Devolve : Verdict::Stay;
        check(original::evolution_verdict(HouseEvolutionRules(parse(top), Difficulty::Normal), state) == verdict,
              "top stages retain current/next requirement evaluation despite lacking an evolution target");
    }
}

void rejected_domains_and_preparation() {
    const auto model = parse(Fixture{});
    const HouseEvolutionRules rules(model, Difficulty::Normal);
    for (const auto bad : {static_cast<HouseType>(0), static_cast<HouseType>(1),
                           static_cast<HouseType>(18), static_cast<HouseType>(65535)}) {
        check(!original::is_house_type(bad), "outside literal original house type range");
        expect_exception<std::out_of_range>([&] { (void)original::is_elite(bad); }, "invalid elite type rejected");
        expect_exception<std::out_of_range>([&] { (void)original::house_row(bad); }, "invalid current row type rejected");
        expect_exception<std::out_of_range>([&] { (void)original::next_row(bad); }, "invalid next row type rejected");
        expect_exception<std::out_of_range>([&] { (void)original::census_capacity_row(bad); }, "invalid census selector type rejected");
        expect_exception<std::out_of_range>([&] { (void)original::admission_capacity_row(bad); }, "invalid admission selector type rejected");
        expect_exception<std::out_of_range>([&] { (void)original::devolve_limit(bad); }, "invalid counter type rejected");
        expect_exception<std::out_of_range>([&] { (void)original::evolve_target(bad); }, "invalid evolution target type rejected");
        expect_exception<std::out_of_range>([&] { (void)original::devolve_target(bad); }, "invalid devolution target type rejected");
        auto state = supplied(bad);
        const auto before = state;
        expect_exception<std::out_of_range>([&] { (void)original::needs_met(rules, ModelHouse::Shelter, state); }, "invalid state type rejected even with valid explicit needs row");
        expect_exception<std::out_of_range>([&] { (void)original::desirability_verdict(rules, state); }, "invalid desirability type rejected");
        expect_exception<std::out_of_range>([&] { (void)original::evolution_verdict(rules, state); }, "invalid combined type rejected");
        expect_exception<std::out_of_range>([&] { (void)original::evolve_step(rules, state, {true, true}); }, "gate cannot conceal invalid original type");
        check(state == before, "rejected pure update leaves caller state untouched");
    }
    for (const auto bad : {static_cast<ModelHouse>(15), static_cast<ModelHouse>(255)}) {
        expect_exception<std::out_of_range>([&] { (void)rules.value(bad, HouseField::Water); }, "invalid prepared row rejected");
        expect_exception<std::out_of_range>([&] { (void)rules.row_capacity(bad); }, "invalid capacity row rejected");
        expect_exception<std::out_of_range>([&] { (void)original::needs_met(rules, bad, supplied()); }, "invalid needs row rejected");
    }
    for (const auto bad : {static_cast<HouseField>(24), static_cast<HouseField>(std::numeric_limits<std::size_t>::max())})
        expect_exception<std::out_of_range>([&] { (void)rules.value(ModelHouse::Shelter, bad); }, "invalid prepared field rejected");
    for (const auto bad : {static_cast<Difficulty>(5), static_cast<Difficulty>(std::numeric_limits<std::size_t>::max())})
        expect_exception<std::out_of_range>([&] { (void)HouseEvolutionRules(model, bad); }, "invalid preparation difficulty rejected");

    Fixture mislabeled;
    mislabeled.names[14] = "Authored last row without canonical identity";
    expect_exception<std::invalid_argument>([&] { (void)HouseEvolutionRules(parse(mislabeled), Difficulty::Normal); },
                                            "unused last-row label mismatch rejects complete preparation");
    Fixture overflow;
    overflow.at(14, HouseField::Unused19) = std::numeric_limits<std::int32_t>::max();
    overflow.modifiers[2][static_cast<std::size_t>(HouseField::Unused19)] = 1;
    expect_exception<std::overflow_error>([&] { (void)HouseEvolutionRules(parse(overflow), Difficulty::Normal); },
                                         "unused field intermediate overflow cannot hide behind clamping");
    overflow.at(14, HouseField::Unused19) = std::numeric_limits<std::int32_t>::min();
    overflow.modifiers[2][static_cast<std::size_t>(HouseField::Unused19)] = -1;
    expect_exception<std::overflow_error>([&] { (void)HouseEvolutionRules(parse(overflow), Difficulty::Normal); },
                                         "unused negative intermediate overflow rejects preparation");
    check(rules.model_sha256() == model.provenance().sha256 &&
          rules.row_capacity(ModelHouse::HeavenlyCompound) == 17,
          "rejected independent preparations leave existing immutable rules intact");
}

// Independent boolean/numeric predicate over prepared raw values. It does not
// call the kernel's needs or verdict APIs and uses explicit pairs for OR fields.
bool reference_needs(const original::HouseRow& row_values, const HouseState& state) {
    const auto get = [&](HouseField field) { return row_values[static_cast<std::size_t>(field)]; };
    if (get(HouseField::Water) != 0 && !state.water) return false;
    for (const auto& need : access_needs)
        if (get(need.field) != 0 && state.*(need.member) == 0) return false;
    if (get(HouseField::DaoistOrBuddhist) != 0 && state.daoist == 0 && state.buddhist == 0) return false;
    if (state.food_quality < get(HouseField::FoodQuality)) return false;
    for (const auto& need : stock_needs)
        if (state.*(need.member) < get(need.field)) return false;
    return state.bronzeware >= get(HouseField::BronzewareOrLacquerware) ||
           state.lacquerware >= get(HouseField::BronzewareOrLacquerware);
}

std::string read_bytes(const fs::path& source) {
    const auto size = fs::file_size(source);
    check(size <= original::max_model_bytes, "bounded original source read");
    std::ifstream input(source, std::ios::binary);
    check(static_cast<bool>(input), "original model can be read");
    std::string bytes(static_cast<std::size_t>(size), '\0');
    input.read(bytes.data(), static_cast<std::streamsize>(size));
    check(input.gcount() == static_cast<std::streamsize>(size) && !input.bad(), "complete bounded original read");
    char extra = 0;
    check(!input.get(extra) && input.eof() && !input.bad(), "original source does not grow during read");
    return bytes;
}

int real_model(const fs::path& root) {
    if (root.empty()) { std::cout << "SKIP: original root not configured\n"; return 77; }
    std::error_code error;
    for (const auto& path : std::array<fs::path, 3>{root, root / "Model", root / "Model/EmperorBuildingModels.txt"}) {
        const auto status = fs::symlink_status(path, error);
        if (status.type() == fs::file_type::not_found) { std::cout << "SKIP: original model path missing\n"; return 77; }
        check(!error, "existing original path metadata must be readable");
        const auto followed = fs::status(path, error);
        check(!error, "existing original path must resolve");
        check(path == root / "Model/EmperorBuildingModels.txt" ? fs::is_regular_file(followed) : fs::is_directory(followed),
              "existing original path has expected type");
    }
    const auto source = root / "Model/EmperorBuildingModels.txt";
    const auto initial_size = fs::file_size(source);
    const auto initial_time = fs::last_write_time(source);
    const auto before = read_bytes(source);
    auto loaded = original::load_model_data(root);
    check(static_cast<bool>(loaded), loaded.error ? loaded.error->reason : "original model loaded");
    const auto& model = *loaded.model;
    check(model.provenance().sha256 == "7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6",
          "read-only integration uses documented original model hash");
    std::size_t type_queries = 0;
    for (const auto difficulty : difficulties) {
        const HouseEvolutionRules rules(model, difficulty);
        std::array<original::HouseRow, 15> rows{};
        for (std::size_t h = 0; h < rows.size(); ++h) {
            check(model.house_label({static_cast<std::uint8_t>(h)}) == labels[h], "real row retains independent canonical identity");
            for (std::size_t f = 0; f < original::house_field_count; ++f) {
                const auto field = static_cast<HouseField>(f);
                const auto raw = model.house({static_cast<std::uint8_t>(h)}, field);
                const auto sum = static_cast<std::int64_t>(raw) + model.house_modifier(difficulty, field);
                check(sum >= std::numeric_limits<std::int32_t>::min() && sum <= std::numeric_limits<std::int32_t>::max(),
                      "real house intermediate fits original arithmetic domain");
                rows[h][f] = static_cast<std::int32_t>(std::clamp(sum, std::int64_t{-99}, std::int64_t{100}));
                check(rules.value(static_cast<ModelHouse>(h), field) == rows[h][f], "real prepared value agrees with independent arithmetic");
            }
            check(rules.row_capacity(static_cast<ModelHouse>(h)) == rows[h][static_cast<std::size_t>(HouseField::Capacity)],
                  "real row capacity is unscaled model count");
        }
        for (const auto& stage : stages) {
            auto state = supplied(stage.type);
            const auto& current = rows[stage.row];
            const auto& next = rows[stage.next];
            const auto low = current[static_cast<std::size_t>(HouseField::DesirabilityLow)];
            const auto high = current[static_cast<std::size_t>(HouseField::DesirabilityHigh)];
            for (const auto desire : std::array<int, 7>{-128, low - 1, low, (low + high) / 2, high, high + 1, 127}) {
                state.desirability = static_cast<std::int8_t>(desire);
                const auto des = desire <= low ? Verdict::Devolve : desire >= high ? Verdict::Evolve : Verdict::Stay;
                check(original::desirability_verdict(rules, state) == des, "real per-type desirability threshold precedence");
                const auto initial = stage.type == HouseType::EliteAbandoned ? Verdict::Evolve : des;
                const auto expected = !reference_needs(current, state) ? Verdict::Devolve
                                    : initial == Verdict::Evolve && !reference_needs(next, state) ? Verdict::Stay : initial;
                check(original::evolution_verdict(rules, state) == expected, "real per-type current and next needs compose with desirability");
                const auto input = state;
                const auto first = original::evolve_step(rules, state);
                check(original::evolve_step(rules, state) == first && state == input,
                      "real-source repeated pure updates are deterministic and leave caller untouched");
                ++type_queries;
            }
            for (const auto& need : access_needs) {
                auto missing = supplied(stage.type);
                missing.*(need.member) = 0;
                check(original::needs_met(rules, static_cast<ModelHouse>(stage.row), missing) == reference_needs(current, missing),
                      "real individual missing service/religion agrees with independent predicate");
            }
            for (const auto& need : stock_needs) {
                auto missing = supplied(stage.type);
                const auto required = current[static_cast<std::size_t>(need.field)];
                missing.*(need.member) = static_cast<std::int16_t>(required - 1);
                check(!original::needs_met(rules, static_cast<ModelHouse>(stage.row), missing),
                      "real individual good below actual threshold fails");
                missing.*(need.member) = static_cast<std::int16_t>(required);
                check(original::needs_met(rules, static_cast<ModelHouse>(stage.row), missing),
                      "real individual good at actual threshold passes");
            }
            check(rules.row_capacity(original::census_capacity_row(stage.type)) == rows[stage.census][17],
                  "real census path uses distinct literal row");
            check(rules.row_capacity(original::admission_capacity_row(stage.type)) == rows[stage.admission][17],
                  "real admission path uses distinct literal row");
        }
    }
    check(read_bytes(source) == before, "original model bytes remain unchanged");
    check(fs::file_size(source) == initial_size && fs::last_write_time(source) == initial_time,
          "original model size and modification time remain unchanged");
    std::cout << "real type boundary queries=" << type_queries << "; SHA-256=" << model.provenance().sha256 << '\n';
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    try {
        if (argc == 3 && std::string_view(argv[1]) == "--real") {
            const auto result = real_model(argv[2]);
            if (result == 77) return result;
        } else if (argc == 1) {
            identities_and_preparation();
            independent_need_boundaries();
            desirability_and_need_precedence();
            step_boundaries_and_special_types();
            rejected_domains_and_preparation();
        } else {
            std::cerr << "usage: original-house-evolution-tests [--real ROOT]\n";
            return 2;
        }
        std::cout << "original house evolution checks passed: " << checks << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "original house evolution test failed: " << error.what() << '\n';
        return 1;
    }
}
