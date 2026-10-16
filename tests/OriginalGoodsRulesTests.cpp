#include "simulation/original/GoodsRules.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace {
namespace original = openemperor::original;
namespace fs = std::filesystem;
using original::Difficulty;
using original::Goods;
using original::HouseEvolutionRules;
using original::HouseField;
using original::HouseState;
using original::HouseType;

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

struct Commodity {
    Goods goods;
    HouseField field;
    std::int16_t HouseState::*stock;
};
constexpr std::array<Commodity, 6> commodities{{
    {Goods::Hemp, HouseField::Hemp, &HouseState::hemp},
    {Goods::Ceramics, HouseField::Ceramics, &HouseState::ceramics},
    {Goods::Tea, HouseField::Tea, &HouseState::tea},
    {Goods::Silk, HouseField::Silk, &HouseState::silk},
    {Goods::Bronzeware, HouseField::BronzewareOrLacquerware, &HouseState::bronzeware},
    {Goods::Lacquerware, HouseField::BronzewareOrLacquerware, &HouseState::lacquerware}
}};
static_assert(original::goods_count == commodities.size());

// Independently listed canonical row selectors, including the goods-only
// row-7 override and the unoccupied-elite skip. No balance table is copied.
struct Stage { HouseType type; std::uint8_t current, delivery; bool elite; };
constexpr std::array<Stage, 16> stages{{
    {HouseType::Vacant, 0, 1, false}, {HouseType::Shelter, 0, 1, false},
    {HouseType::Hut, 1, 2, false}, {HouseType::PlainCottage, 2, 3, false},
    {HouseType::AttractiveCottage, 3, 4, false}, {HouseType::SpaciousDwelling, 4, 5, false},
    {HouseType::ElegantDwelling, 5, 6, false}, {HouseType::OrnateApartment, 6, 7, false},
    {HouseType::LuxuriousApartment, 7, 7, false}, {HouseType::EliteUnoccupied, 8, 10, true},
    {HouseType::EliteAbandoned, 9, 10, true}, {HouseType::ModestSiheyuan, 10, 11, true},
    {HouseType::LavishSiheyuan, 11, 12, true}, {HouseType::HumbleCompound, 12, 13, true},
    {HouseType::ImpressiveCompound, 13, 14, true}, {HouseType::HeavenlyCompound, 14, 14, true}
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

    std::int32_t& at(std::size_t house, HouseField field) {
        return values.at(house).at(static_cast<std::size_t>(field));
    }
    void goods(std::int32_t amount) {
        for (std::size_t h = 0; h < values.size(); ++h)
            for (const auto& commodity : commodities) at(h, commodity.field) = amount;
    }
    std::string text() const {
        std::string result = "Authored goods rules fixture\nBUILDING MODS\n";
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
        for (std::size_t i = 0; i < values.size(); ++i) result += row(labels[i], values[i]);
        return result + "End of model data.\n-------------------\n";
    }
};

original::ModelData parse(const Fixture& fixture) {
    auto result = original::parse_model_data(fixture.text());
    check(static_cast<bool>(result), result.error ? result.error->reason : "authored fixture parsed");
    return std::move(*result.model);
}

HouseState observed(HouseType type = HouseType::Hut) {
    HouseState state;
    state.type = type;
    state.desirability = -21;
    state.occupants = 7;
    state.devolve_counter = 39;
    state.water = true;
    state.herbalist = 17;
    state.acupuncture = 31;
    state.music = 47;
    state.acrobat = 59;
    state.drama = 71;
    state.ancestor = 83;
    state.confucian = 97;
    state.daoist = 109;
    state.buddhist = 127;
    state.food_quality = 193;
    state.hemp = 19;
    state.ceramics = 23;
    state.tea = 29;
    state.silk = 31;
    state.bronzeware = 37;
    state.lacquerware = 41;
    return state;
}

std::int32_t modified(const original::ModelData& model, std::uint8_t house,
                      HouseField field, Difficulty difficulty) {
    const auto sum = static_cast<std::int64_t>(model.house({house}, field)) +
                     model.house_modifier(difficulty, field);
    return static_cast<std::int32_t>(std::clamp(sum, std::int64_t{-99}, std::int64_t{100}));
}

original::GoodsDemand reference_demand(const original::ModelData& model, Difficulty difficulty,
                                      const Stage& stage, const HouseState& house,
                                      std::uint8_t side, const Commodity& commodity) {
    std::int32_t amount = 10;
    if (house.occupants != 0 || (commodity.goods != Goods::Hemp && commodity.goods != Goods::Tea)) {
        amount = modified(model, stage.delivery, commodity.field, difficulty);
        if (side > 1) amount *= 4;
        if (stage.elite) amount *= 2;
    }
    return {amount, std::max(amount, 10)};
}

void identities_and_demands() {
    Fixture fixture;
    for (std::size_t h = 0; h < fixture.values.size(); ++h)
        for (std::size_t f = 0; f < original::house_field_count; ++f)
            fixture.values[h][f] = static_cast<std::int32_t>((h * 13 + f * 7) % 39) - 11;
    for (std::size_t d = 0; d < difficulties.size(); ++d)
        for (std::size_t f = 0; f < original::house_field_count; ++f)
            fixture.modifiers[d][f] = static_cast<std::int32_t>(d * 5 + f % 7) - 9;
    const auto model = parse(fixture);
    for (const auto& commodity : commodities) {
        check(original::goods_field(commodity.goods) == commodity.field, "six typed goods select the exact observed model field");
        const auto state = observed();
        check(original::goods_stock(state, commodity.goods) == state.*(commodity.stock), "each commodity reads its independent stock word");
    }
    for (const auto difficulty : difficulties) {
        const HouseEvolutionRules rules(model, difficulty);
        for (const auto& stage : stages) {
            check(static_cast<std::uint8_t>(original::goods_delivery_row(stage.type)) == stage.delivery,
                  "literal goods delivery row selector including row-seven override");
            for (const auto occupants : std::array<std::int16_t, 5>{-32768, -1, 0, 1, 32767}) {
                auto state = observed(stage.type);
                state.occupants = occupants;
                const auto before = state;
                check(original::accepts_goods_delivery(state) == (occupants > 0 || stage.elite),
                      "goods visit admits positive population or any elite type");
                for (const auto side : std::array<std::uint8_t, 6>{0, 1, 2, 3, 127, 255})
                    for (const auto& commodity : commodities)
                        check(original::goods_demand(rules, state, side, commodity.goods) ==
                              reference_demand(model, difficulty, stage, state, side, commodity),
                              "independent modified next-row demand, exact-zero exceptions and distinct area/elite multipliers");
                check(state == before, "need and eligibility queries preserve the input House observation");
            }
        }
    }
    for (const auto amount : {-500, -99, -1, 0, 1, 100, 500}) {
        Fixture limits;
        limits.goods(amount);
        const auto bounded = parse(limits);
        const HouseEvolutionRules rules(bounded, Difficulty::Normal);
        auto state = observed(HouseType::HeavenlyCompound);
        for (const auto& commodity : commodities) {
            const auto demand = original::goods_demand(rules, state, 255, commodity.goods);
            const auto expected = std::clamp(amount, -99, 100) * 8;
            check(demand.need == expected && demand.target == std::max(expected, 10),
                  "signed model extrema remain bounded after both multipliers with a minimum target of ten");
        }
    }
}

std::int32_t reference_transfer(original::GoodsDemand demand, bool admitted,
                                std::int16_t stock, std::int32_t source) {
    if (!admitted || demand.need <= 0 || source == 0) return 0;
    std::int64_t requested = static_cast<std::int64_t>(demand.target) - stock;
    if (requested <= 0 || requested > source) requested = static_cast<std::int64_t>(demand.need) - stock;
    return static_cast<std::int32_t>(std::max(std::int64_t{0},
        std::min({requested, static_cast<std::int64_t>(demand.target), static_cast<std::int64_t>(source),
                  static_cast<std::int64_t>(std::numeric_limits<std::int16_t>::max()) - stock})));
}

void delivery_conservation() {
    Fixture fixture;
    for (std::size_t h = 0; h < fixture.values.size(); ++h)
        for (std::size_t f = 0; f < original::house_field_count; ++f)
            fixture.values[h][f] = static_cast<std::int32_t>((h * 11 + f * 3) % 27) - 5;
    const auto model = parse(fixture);
    const HouseEvolutionRules rules(model, Difficulty::Normal);
    constexpr std::array<std::int32_t, 9> sources{0, 1, 4, 9, 10, 31, 800, 32767, std::numeric_limits<std::int32_t>::max()};
    constexpr std::array<std::int16_t, 12> stocks{0, 1, 4, 5, 9, 10, 17, 63, 799, 800, 32766, 32767};
    for (const auto& stage : stages) {
        for (const auto occupants : std::array<std::int16_t, 3>{-1, 0, 1}) {
            for (const auto side : std::array<std::uint8_t, 4>{0, 1, 2, 255}) {
                for (const auto& commodity : commodities) {
                    for (const auto stock : stocks) {
                        auto state = observed(stage.type);
                        state.occupants = occupants;
                        state.*(commodity.stock) = stock;
                        const auto before = state;
                        const auto demand = reference_demand(model, Difficulty::Normal, stage, state, side, commodity);
                        for (const auto source : sources) {
                            const auto moved = reference_transfer(demand, occupants > 0 || stage.elite, stock, source);
                            auto expected = state;
                            expected.*(commodity.stock) = static_cast<std::int16_t>(stock + moved);
                            const auto result = original::deliver_goods(rules, state, side, commodity.goods, source);
                            check(result.house == expected && result.source_stock == source - moved && result.transferred == moved,
                                  "delivery preserves unrelated state and uses source-dependent original request plus safe caps");
                            check(static_cast<std::int64_t>(source) + stock ==
                                  static_cast<std::int64_t>(result.source_stock) + result.house.*(commodity.stock),
                                  "partial delivery conserves source plus destination before/after even at maximum int32 source");
                            check(result.source_stock >= 0 && result.house.*(commodity.stock) >= 0 && result.transferred >= 0,
                                  "delivery never creates negative inventories or negative transfers");
                            check(state == before, "delivery leaves the caller House observation unchanged");
                        }
                    }
                }
            }
        }
    }

    // Literal requests distinguish the EXE source-stock fallback from the
    // fork's need-based fallback and final need cap.
    Fixture five;
    five.goods(5);
    const HouseEvolutionRules five_rules(parse(five), Difficulty::Normal);
    struct Vector { std::int16_t stock; std::int32_t source, moved; };
    constexpr std::array<Vector, 10> vectors{{
        {0, 100, 10}, {0, 10, 10}, {0, 5, 5}, {0, 4, 4}, {4, 5, 1},
        {4, 6, 6}, {5, 100, 5}, {9, 1, 1}, {10, 100, 0}, {32767, 2147483647, 0}
    }};
    for (const auto& vector : vectors) {
        auto state = observed();
        state.hemp = vector.stock;
        const auto result = original::deliver_goods(five_rules, state, 1, Goods::Hemp, vector.source);
        check(result.transferred == vector.moved && result.house.hemp == vector.stock + vector.moved &&
              result.source_stock == vector.source - vector.moved,
              "literal source-dependent fallback and ten-unit target boundary");
    }
    for (const auto amount : {-99, -1, 0}) {
        Fixture disabled;
        disabled.goods(amount);
        const HouseEvolutionRules disabled_rules(parse(disabled), Difficulty::Normal);
        const auto state = observed();
        for (const auto& commodity : commodities)
            check(original::deliver_goods(disabled_rules, state, 255, commodity.goods, std::numeric_limits<std::int32_t>::max()) ==
                  original::GoodsDelivery{state, std::numeric_limits<std::int32_t>::max(), 0},
                  "zero and negative model need cannot mint stock through the minimum target");
    }
}

void consumption_conservation() {
    Fixture fixture;
    for (std::size_t h = 0; h < fixture.values.size(); ++h) {
        for (std::size_t f = 0; f < original::house_field_count; ++f)
            fixture.values[h][f] = static_cast<std::int32_t>((h * 7 + f * 11) % 31) - 7;
    }
    const auto model = parse(fixture);
    for (const auto difficulty : difficulties) {
        const HouseEvolutionRules rules(model, difficulty);
        for (const auto& stage : stages) {
            for (const auto occupants : std::array<std::int16_t, 5>{-32768, -1, 0, 1, 32767}) {
                for (const auto base : std::array<std::int16_t, 8>{0, 1, 4, 9, 17, 99, 32766, 32767}) {
                    auto state = observed(stage.type);
                    state.occupants = occupants;
                    for (const auto& commodity : commodities) state.*(commodity.stock) = base;
                    const auto before = state;
                    auto expected = state;
                    std::array<std::int32_t, 6> amounts{};
                    std::int64_t total_before = 0;
                    for (std::size_t g = 0; g < commodities.size(); ++g) {
                        const auto& commodity = commodities[g];
                        const auto raw = modified(model, stage.current, commodity.field, difficulty);
                        amounts[g] = occupants == 0 ? 0 : std::min<std::int32_t>(base, std::max(raw, 0));
                        expected.*(commodity.stock) = static_cast<std::int16_t>(base - amounts[g]);
                        total_before += base;
                    }
                    const auto result = original::consume_goods(rules, state);
                    check(result.house == expected && result.consumed == amounts,
                          "explicit consumption uses each current-row positive amount without delivery/size/elite scaling");
                    std::int64_t accounted = 0;
                    for (std::size_t g = 0; g < commodities.size(); ++g) {
                        const auto& commodity = commodities[g];
                        check(state.*(commodity.stock) == result.house.*(commodity.stock) + result.consumed[g],
                              "each goods stock plus its explicitly reported consumption conserves the initial amount");
                        check(result.house.*(commodity.stock) >= 0 && result.consumed[g] >= 0,
                              "consumption cannot create goods or negative stock");
                        accounted += result.house.*(commodity.stock) + result.consumed[g];
                    }
                    check(accounted == total_before, "whole six-goods inventory accounts for every consumed unit");
                    check(state == before, "consumption preserves caller state and unrelated food/services");
                }
            }
        }
    }

    Fixture alternatives;
    alternatives.goods(0);
    alternatives.at(1, HouseField::BronzewareOrLacquerware) = 7;
    alternatives.at(1, HouseField::Silk) = 23;
    const HouseEvolutionRules rules(parse(alternatives), Difficulty::Normal);
    auto state = observed();
    state.bronzeware = 8;
    state.lacquerware = 9;
    const auto result = original::consume_goods(rules, state);
    check(result.house.bronzeware == 1 && result.house.lacquerware == 2 &&
          result.consumed[4] == 7 && result.consumed[5] == 7,
          "both independent bronze/lacquer stocks consume field thirteen despite the evolution OR predicate");
    check(result.house.silk == 8 && result.consumed[3] == 23,
          "silk remains independent from the shared bronze/lacquer consumption column");
    state.occupants = -1;
    auto negative_expected = result.house;
    negative_expected.occupants = -1;
    check(original::consume_goods(rules, state).house == negative_expected,
          "observed consumption gate is exactly zero, not a positive-population eligibility rule");
    state.occupants = 0;
    check(original::consume_goods(rules, state) == original::GoodsConsumption{state, {}},
          "an exactly empty House consumes none of its six existing stocks");
}

void food_quality_boundaries() {
    // Each vector lists independently expected (house weight, source weight).
    // Ratios are delivery / PRE-delivery stock, not a model quality threshold.
    struct Blend { std::int32_t delivered, previous; unsigned house, source; };
    constexpr std::array<Blend, 20> blends{{
        {1, 0, 1, 3}, {330, 1000, 3, 1}, {331, 1000, 2, 1},
        {1, 3, 2, 1}, {1, 2, 2, 1}, {501, 1000, 1, 1},
        {2, 1, 1, 1}, {2001, 1000, 1, 2}, {3, 1, 1, 2},
        {3001, 1000, 1, 3}, {2147483647, 2147483647, 1, 1},
        {2147483647, 1, 1, 3}, {2147483647, 715827882, 1, 2},
        {2088763472, 696254463, 1, 3}, // Above 3's binary32 midpoint; binary64 division loses that distinction.
        {1635778622, 545259519, 1, 3}, {1090519103, 545259519, 1, 2},
        {1073741887, 2147483646, 1, 1}, // Above .5's midpoint despite a double-rounded ratio equaling .5F.
        {1, 2147483647, 3, 1}, {1, 1, 1, 1}, {100, 10, 1, 3}
    }};
    for (unsigned house = 0; house <= 255; ++house) {
        for (unsigned source = 0; source <= 255; ++source) {
            const auto q = static_cast<std::uint8_t>(house);
            const auto m = static_cast<std::uint8_t>(source);
            check(original::blend_food_quality(q, m, 0, 0) == q &&
                  original::blend_food_quality(q, m, 0, 2147483647) == q,
                  "safe zero-delivery boundary preserves every quality pair, including higher source quality");
            for (const auto& blend : blends) {
                const auto expected = source > house ? source :
                    (house * blend.house + source * blend.source) / (blend.house + blend.source);
                check(original::blend_food_quality(q, m, blend.delivered, blend.previous) == expected,
                      "all byte-quality pairs follow the independently classified strict-ratio band and floor average");
            }
        }
    }

    // Exact binary32 midpoints are rational integers. 3, 2 and .5 have an
    // even significand, so a tie stays in the lower band. .33F is odd, so its
    // midpoint tie rounds upward. These expectations do not use host floats.
    struct Midpoint { std::int32_t numerator, denominator; bool tie_above; std::uint8_t below, above; };
    constexpr std::array<Midpoint, 8> midpoints{{
        {25165825, 8388608, false, 85, 63},
        {16777217, 8388608, false, 127, 85},
        {16777217, 33554432, false, 170, 127},
        {22145927, 67108864, true, 191, 170},
        {2139095125, 713031680, false, 85, 63},
        {1426063445, 713031680, false, 127, 85},
        {1056964671, 2113929216, false, 170, 127},
        {686523737, 2080374784, true, 191, 170}
    }};
    for (const auto& midpoint : midpoints) {
        check(original::blend_food_quality(255, 0, midpoint.numerator - 1, midpoint.denominator) == midpoint.below,
              "one integer below an exact binary32 midpoint remains in the lower weight band");
        check(original::blend_food_quality(255, 0, midpoint.numerator, midpoint.denominator) ==
              (midpoint.tie_above ? midpoint.above : midpoint.below),
              "exact midpoint uses binary32 nearest ties-to-even, including odd .33F threshold");
        check(original::blend_food_quality(255, 0, midpoint.numerator + 1, midpoint.denominator) == midpoint.above,
              "one integer above a binary32 midpoint enters the next weight band at high int32 magnitudes");
    }
}

void rejected_domains() {
    Fixture fixture;
    fixture.goods(5);
    const HouseEvolutionRules rules(parse(fixture), Difficulty::Normal);
    auto state = observed();
    const auto before = state;
    for (const auto value : std::array<std::uint16_t, 4>{0, 1, 18, 65535}) {
        auto invalid = state;
        invalid.type = static_cast<HouseType>(value);
        expect_exception<std::out_of_range>([&] { (void)original::goods_delivery_row(invalid.type); }, "nonhouse row rejected");
        expect_exception<std::out_of_range>([&] { (void)original::accepts_goods_delivery(invalid); }, "nonhouse admission rejected");
        expect_exception<std::out_of_range>([&] { (void)original::goods_demand(rules, invalid, 1, Goods::Hemp); }, "nonhouse demand rejected");
        expect_exception<std::out_of_range>([&] { (void)original::deliver_goods(rules, invalid, 1, Goods::Hemp, 0); }, "nonhouse delivery rejected even without source");
        expect_exception<std::out_of_range>([&] { (void)original::consume_goods(rules, invalid); }, "nonhouse consumption rejected");
    }
    for (const auto value : std::array<std::uint8_t, 3>{6, 7, 255}) {
        const auto invalid = static_cast<Goods>(value);
        expect_exception<std::out_of_range>([&] { (void)original::goods_field(invalid); }, "invalid commodity field rejected");
        expect_exception<std::out_of_range>([&] { (void)original::goods_stock(state, invalid); }, "invalid commodity never aliases silk");
        expect_exception<std::out_of_range>([&] { (void)original::goods_demand(rules, state, 1, invalid); }, "invalid commodity need rejected");
        expect_exception<std::out_of_range>([&] { (void)original::deliver_goods(rules, state, 1, invalid, 0); }, "invalid commodity delivery rejected even without source");
    }
    for (const auto& commodity : commodities) {
        for (const auto amount : std::array<std::int16_t, 2>{-1, -32768}) {
            auto invalid = state;
            invalid.occupants = 0;
            invalid.*(commodity.stock) = amount;
            expect_exception<std::invalid_argument>([&] { (void)original::deliver_goods(rules, invalid, 1, Goods::Hemp, 0); },
                                                   "any negative House inventory rejected before gated empty/no-source delivery");
            expect_exception<std::invalid_argument>([&] { (void)original::consume_goods(rules, invalid); },
                                                   "any negative House inventory rejected even for zero-occupant consumption");
        }
    }
    state.occupants = 0;
    for (const auto amount : {-1, std::numeric_limits<std::int32_t>::min()}) {
        expect_exception<std::invalid_argument>([&] { (void)original::deliver_goods(rules, state, 1, Goods::Hemp, amount); },
                                               "negative source rejected even when an ordinary empty House cannot receive");
        expect_exception<std::invalid_argument>([&] { (void)original::blend_food_quality(0, 255, amount, 0); },
                                               "negative delivered food rejected before an immediate quality upgrade");
        expect_exception<std::invalid_argument>([&] { (void)original::blend_food_quality(0, 255, 0, amount); },
                                               "negative previous food stock rejected even on zero delivery");
    }
    state.occupants = before.occupants;
    check(state == before, "all rejected operations leave supplied state unchanged");
}

std::string read_bytes(const fs::path& path) {
    const auto length = fs::file_size(path);
    check(length <= original::max_model_bytes, "original integration read stays bounded");
    std::ifstream file(path, std::ios::binary);
    check(static_cast<bool>(file), "original model readable");
    std::string bytes(static_cast<std::size_t>(length), '\0');
    file.read(bytes.data(), static_cast<std::streamsize>(length));
    check(file.gcount() == static_cast<std::streamsize>(length) && !file.bad(), "complete bounded original integration read");
    char extra = 0;
    check(!file.get(extra) && file.eof() && !file.bad(), "original source does not grow during read");
    return bytes;
}

int real_model(const fs::path& root) {
    if (root.empty()) { std::cout << "SKIP: original root not configured\n"; return 77; }
    std::error_code error;
    const auto source = root / "Model/EmperorBuildingModels.txt";
    for (const auto& path : std::array<fs::path, 3>{root, root / "Model", source}) {
        const auto status = fs::symlink_status(path, error);
        if (status.type() == fs::file_type::not_found) { std::cout << "SKIP: original model path missing\n"; return 77; }
        check(!error, "existing original path metadata must be readable");
        const auto followed = fs::status(path, error);
        check(!error, "existing original path must resolve");
        check(path == source ? fs::is_regular_file(followed) : fs::is_directory(followed), "existing source path has expected type");
    }
    const auto initial_size = fs::file_size(source);
    const auto initial_time = fs::last_write_time(source);
    const auto before = read_bytes(source);
    auto loaded = original::load_model_data(root);
    check(static_cast<bool>(loaded), loaded.error ? loaded.error->reason : "original model loaded");
    const auto& model = *loaded.model;
    check(model.provenance().sha256 == "7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6",
          "integration uses the documented read-only model identity");
    std::size_t queries = 0;
    std::size_t positive = 0;
    for (const auto difficulty : difficulties) {
        const HouseEvolutionRules rules(model, difficulty);
        for (const auto& stage : stages) {
            for (const auto occupants : std::array<std::int16_t, 3>{-1, 0, 1}) {
                for (const auto side : std::array<std::uint8_t, 3>{0, 1, 2}) {
                    auto state = observed(stage.type);
                    state.occupants = occupants;
                    for (const auto& commodity : commodities) state.*(commodity.stock) = 0;
                    for (const auto& commodity : commodities) {
                        const auto demand = reference_demand(model, difficulty, stage, state, side, commodity);
                        check(original::goods_demand(rules, state, side, commodity.goods) == demand,
                              "real next-row goods needs use independent raw-plus-difficulty clamping and explicit type/size gates");
                        positive += demand.need > 0 ? 1U : 0U;
                        const auto moved = reference_transfer(demand, occupants > 0 || stage.elite, 0, 3);
                        const auto delivered = original::deliver_goods(rules, state, side, commodity.goods, 3);
                        auto expected = state;
                        expected.*(commodity.stock) = static_cast<std::int16_t>(moved);
                        check(delivered == original::GoodsDelivery{expected, 3 - moved, moved},
                              "real-source bounded partial delivery and admission conserve all three supplied units");
                        ++queries;
                    }
                    for (const auto& commodity : commodities) state.*(commodity.stock) = 32767;
                    const auto consumed = original::consume_goods(rules, state);
                    auto expected = state;
                    std::array<std::int32_t, 6> amounts{};
                    for (std::size_t g = 0; g < commodities.size(); ++g) {
                        const auto& commodity = commodities[g];
                        const auto amount = modified(model, stage.current, commodity.field, difficulty);
                        amounts[g] = occupants == 0 ? 0 : std::max(amount, 0);
                        expected.*(commodity.stock) = static_cast<std::int16_t>(32767 - amounts[g]);
                    }
                    check(consumed == original::GoodsConsumption{expected, amounts},
                          "real-source consumption selects current row and independently accounts for both shared-column goods");
                }
            }
        }
    }
    check(positive > 0, "real integration exercises actual positive goods needs");
    check(read_bytes(source) == before, "original model bytes remain unchanged");
    check(fs::file_size(source) == initial_size && fs::last_write_time(source) == initial_time,
          "original model size and modification time remain unchanged");
    std::cout << "real goods queries=" << queries << "; positive needs=" << positive
              << "; SHA-256=" << model.provenance().sha256 << '\n';
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    try {
        if (argc == 3 && std::string_view(argv[1]) == "--real") {
            const auto result = real_model(argv[2]);
            if (result == 77) return result;
        } else if (argc == 1) {
            identities_and_demands();
            delivery_conservation();
            consumption_conservation();
            food_quality_boundaries();
            rejected_domains();
        } else {
            std::cerr << "usage: original-goods-rules-tests [--real ROOT]\n";
            return 2;
        }
        std::cout << "original goods rules checks passed: " << checks << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "original goods rules test failed: " << error.what() << '\n';
        return 1;
    }
}
