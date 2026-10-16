#include "simulation/original/Desirability.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <numbers>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {
namespace original = openemperor::original;
namespace fs = std::filesystem;
using original::DesirabilityMap;
using original::DesirabilitySource;
using original::Difficulty;
using original::Footprint;
using original::ModelBuilding;
using original::SpreadSpec;
using original::WallBlocking;

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

std::vector<int> snapshot(const DesirabilityMap& map) {
    std::vector<int> values;
    for (int y = 0; y < map.height(); ++y)
        for (int x = 0; x < map.width(); ++x) values.push_back(map.at(x, y));
    return values;
}

// Independent oracle: distance to a square and a closed-form value for that
// distance. It neither constructs nor traverses the implementation's rings.
int reference_contribution(const DesirabilitySource& source, int x, int y) {
    const auto& f = source.footprint;
    const auto& s = source.spread;
    const auto dx = std::max({0, f.x - x, x - (f.x + f.side - 1)});
    const auto dy = std::max({0, f.y - y, y - (f.y + f.side - 1)});
    const auto distance = std::max(dx, dy);
    const auto range = std::clamp<std::int32_t>(s.range, 0, 10);
    if (s.value == 0 || distance == 0 || distance > range) return 0;
    const auto period = std::max<std::int32_t>(s.step, 1);
    const auto updates = (distance - 1) / period;
    const auto candidate = static_cast<std::int64_t>(s.value) +
                           static_cast<std::int64_t>(updates) * s.step_size;
    return static_cast<int>(s.value > 0 ? std::max(candidate, std::int64_t{0})
                                       : std::min(candidate, std::int64_t{0}));
}

std::vector<int> reference(int width, int height,
                           std::span<const DesirabilitySource> sources) {
    std::vector<int> values(static_cast<std::size_t>(width * height), 0);
    for (const auto& source : sources) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                auto& cell = values[static_cast<std::size_t>(y * width + x)];
                const auto sum = static_cast<std::int64_t>(cell) +
                                 reference_contribution(source, x, y);
                cell = static_cast<int>(std::clamp(sum, std::int64_t{-100}, std::int64_t{100}));
            }
        }
    }
    return values;
}

void check_reference(int width, int height,
                     std::span<const DesirabilitySource> sources,
                     std::string_view message) {
    DesirabilityMap map(width, height);
    map.apply_sources(sources);
    check(snapshot(map) == reference(width, height, sources), message);
}

void single_sources_and_steps() {
    DesirabilityMap map(27, 27);
    map.apply_source({{13, 13, 1}, {17, 1, 0, 1}});
    check(map.at(13, 13) == 0, "source does not affect its own footprint");
    check(map.at(12, 12) == 17 && map.at(14, 14) == 17 &&
          map.at(12, 13) == 17 && map.at(13, 14) == 17,
          "square ring includes equal-valued diagonals");
    check(map.at(11, 13) == 0, "single ring ends at distance one");
    check(map.width() == 27 && map.height() == 27, "map dimensions preserved");

    const std::array<SpreadSpec, 15> specs{{
        {21, 2, -4, 10}, {-21, 2, 4, 10},
        {5, 1, -3, 10}, {-5, 1, 3, 10},
        {5, 0, -1, 10}, {5, -3, -1, 10},
        {-5, 0, 1, 10}, {-5, std::numeric_limits<std::int32_t>::min(), 1, 10},
        {0, 1, 29, 10}, {15, 1, 2, 0}, {15, 1, 2, -1},
        {15, 1, 2, 11}, {15, 1, 2, std::numeric_limits<std::int32_t>::max()},
        {11, std::numeric_limits<std::int32_t>::max(), -3, 10},
        {11, 1, 3, 10}
    }};
    for (const auto& spec : specs) {
        const std::array sources{DesirabilitySource{{13, 13, 1}, spec}};
        check_reference(27, 27, sources, "step, sign, range and zero reference");
    }
    map.clear();
    map.apply_source({{13, 13, 1}, {21, 2, -4, 10}});
    check(map.at(14, 13) == 21 && map.at(15, 13) == 21 &&
          map.at(16, 13) == 17 && map.at(17, 13) == 17,
          "initial value is applied before each period change");
    map.clear();
    map.apply_source({{13, 13, 1}, {5, 0, -1, 10}});
    check(map.at(14, 13) == 5 && map.at(15, 13) == 4,
          "nonpositive period advances each ring, unlike inspected fork");
    map.clear();
    map.apply_source({{13, 13, 1}, {5, 1, -3, 10}});
    check(map.at(14, 13) == 5 && map.at(15, 13) == 2 &&
          map.at(16, 13) == 0 && map.at(23, 13) == 0,
          "positive decay is absorbed at zero");
    map.clear();
    map.apply_source({{13, 13, 1}, {-5, 1, 3, 10}});
    check(map.at(15, 13) == -2 && map.at(16, 13) == 0 && map.at(23, 13) == 0,
          "negative decay is absorbed at zero");
    map.clear();
    map.apply_source({{13, 13, 1}, {9, 1, 0, 100}});
    check(map.at(23, 13) == 9 && map.at(24, 13) == 0,
          "range above ten stays bounded at ten");
}

void footprints_edges_and_order() {
    for (int side = 1; side <= 6; ++side) {
        const std::array<Footprint, 5> origins{{
            {0, 0, side}, {20 - side, 0, side}, {0, 20 - side, side},
            {20 - side, 20 - side, side}, {8, 8, side}
        }};
        for (const auto footprint : origins) {
            const std::array sources{DesirabilitySource{footprint, {-37, 3, 5, 10}}};
            check_reference(20, 20, sources, "all evidenced sides, corners and clipped rings");
        }
    }
    DesirabilityMap multi(12, 12);
    multi.apply_source({{4, 4, 3}, {17, 1, 0, 1}});
    check(multi.at(4, 4) == 0 && multi.at(6, 6) == 0 && multi.at(5, 5) == 0,
          "whole explicit multi-cell footprint is excluded");
    check(multi.at(3, 3) == 17 && multi.at(7, 7) == 17,
          "multi-cell corners receive one contribution, not duplicated corners");

    const DesirabilitySource positive{{3, 3, 1}, {90, 1, 0, 3}};
    const DesirabilitySource negative{{3, 3, 1}, {-90, 1, 0, 3}};
    const std::array first{positive, positive, negative};
    const std::array second{positive, negative, positive};
    DesirabilityMap a(8, 8), b(8, 8);
    a.apply_sources(first);
    b.apply_sources(second);
    check(a.at(4, 3) == 10 && b.at(4, 3) == 90,
          "each addition saturates and caller order is preserved");
    check_reference(8, 8, first, "positive overlap reference");
    check_reference(8, 8, second, "ordered overlap reference");
    const std::array negative_first{negative, negative, positive};
    check_reference(8, 8, negative_first, "negative saturation reference");

    DesirabilityMap serial(8, 8);
    for (const auto& source : first) serial.apply_source(source);
    check(snapshot(serial) == snapshot(a), "batch equals the same ordered individual sources");
    a.apply_sources({});
    check(snapshot(serial) == snapshot(a), "empty source sequence is a no-op");
}

void house_queries() {
    DesirabilityMap map(10, 10);
    map.apply_source({{0, 0, 1}, {-40, 1, 7, 10}});
    check(map.building_value({1, 1, 1}) == -40, "signed negative singleton query");
    check(map.building_value({1, 1, 3}) == -26,
          "house footprint returns its maximum signed score");
    check(map.building_value({0, 0, 2}) == 0,
          "query includes a valid neutral footprint cell");
    check(map.building_value({1, 1, 3}, 10) == -16,
          "explicit bonus follows the footprint maximum");
    DesirabilityMap saturated(10, 10);
    saturated.apply_source({{0, 0, 1}, {101, 1, 0, 10}});
    check(saturated.building_value({1, 1, 6}, 10) == 110,
          "query bonus is not silently clamped to the cell range");
    DesirabilityMap neutral(1, 1);
    check(neutral.building_value({0, 0, 1}, std::numeric_limits<std::int32_t>::min()) ==
          std::numeric_limits<std::int32_t>::min(), "explicit bonus int32 lower boundary");
    check(neutral.building_value({0, 0, 1}, std::numeric_limits<std::int32_t>::max()) ==
          std::numeric_limits<std::int32_t>::max(), "explicit bonus int32 upper boundary");
    expect_exception<std::overflow_error>([&] {
        (void)saturated.building_value({1, 1, 1}, std::numeric_limits<std::int32_t>::max());
    }, "positive bonus overflow must be rejected");
    expect_exception<std::overflow_error>([&] {
        (void)map.building_value({1, 1, 1}, std::numeric_limits<std::int32_t>::min());
    }, "negative bonus overflow must be rejected");
}

void walls_are_explicit_experiments() {
    const DesirabilitySource negative{{8, 8, 1}, {-11, 1, 0, 7}};
    DesirabilityMap default_mode(17, 17), plain(17, 17), experimental(17, 17);
    default_mode.set_wall(9, 8);
    experimental.set_wall(9, 8);
    default_mode.apply_source(negative);
    plain.apply_source(negative);
    experimental.apply_source(negative, WallBlocking::ExperimentalUniform16);
    check(snapshot(default_mode) == snapshot(plain), "prepared walls are ignored by default");
    check(default_mode.at(9, 8) == -11 && default_mode.at(10, 8) == -11,
          "default mode has no implicit wall effect");
    check(experimental.at(9, 8) == 0 && experimental.at(10, 8) == 0,
          "explicit experimental mode skips wall and farther shadow");
    check(experimental.at(7, 8) == -11 && experimental.at(9, 9) == -11,
          "experimental wall does not block other sectors");

    DesirabilityMap same_ring(17, 17);
    same_ring.set_wall(12, 9);
    same_ring.apply_source(negative, WallBlocking::ExperimentalUniform16);
    check(same_ring.at(12, 9) == 0 && same_ring.at(12, 8) == -11 && same_ring.at(13, 8) == 0,
          "wall shadow starts strictly after its own ring with a complete ring prepass");

    DesirabilityMap positive(17, 17), positive_plain(17, 17);
    positive.set_wall(9, 8);
    const DesirabilitySource good{{8, 8, 1}, {11, 1, 0, 7}};
    positive.apply_source(good, WallBlocking::ExperimentalUniform16);
    positive_plain.apply_source(good);
    check(snapshot(positive) == snapshot(positive_plain), "positive sources bypass experimental walls");

    experimental.clear();
    const auto cleared = snapshot(experimental);
    check(std::all_of(cleared.begin(), cleared.end(),
                      [](int value) { return value == 0; }), "clear resets values");
    check(experimental.wall(9, 8), "clear retains explicitly prepared walls");
    experimental.clear_walls();
    check(!experimental.wall(9, 8), "clear_walls explicitly resets obstruction state");
    experimental.set_wall(9, 8);
    experimental.set_wall(9, 8, false);
    check(!experimental.wall(9, 8), "individual wall can be removed");

    for (const auto direction : std::array<std::array<int, 2>, 8>{{
             {{1, 0}}, {{-1, 0}}, {{0, 1}}, {{0, -1}},
             {{1, 1}}, {{-1, 1}}, {{1, -1}}, {{-1, -1}}
         }}) {
        DesirabilityMap directional(21, 21);
        directional.set_wall(10 + 2 * direction[0], 10 + 2 * direction[1]);
        directional.apply_source({{10, 10, 1}, {-11, 1, 0, 7}},
                                 WallBlocking::ExperimentalUniform16);
        check(directional.at(10 + 2 * direction[0], 10 + 2 * direction[1]) == 0,
              "wall cell is skipped in every cardinal and diagonal direction");
        check(directional.at(10 + 4 * direction[0], 10 + 4 * direction[1]) == 0,
              "cardinal and diagonal experimental shadows extend outward");
        check(directional.at(10 - 4 * direction[0], 10 - 4 * direction[1]) == -11,
              "opposite direction remains unaffected by a wall");
    }

    DesirabilityMap even(24, 24);
    even.set_wall(11, 8);
    even.apply_source({{8, 8, 2}, {-11, 1, 0, 10}}, WallBlocking::ExperimentalUniform16);
    check(even.at(11, 8) == 0 && even.at(16, 7) == 0 && even.at(1, 10) == -11,
          "even-side experimental sectors use the true footprint centre");

    // Independent angular oracle for this authored hypothesis: atan2 in the
    // test versus exact integer comparisons in the kernel. No nonzero integer
    // direction lies on an irrational tan(22.5 degree) boundary.
    const Footprint footprint{8, 8, 2};
    const std::array<std::array<int, 2>, 8> walls{{
        {{12, 9}}, {{4, 8}}, {{11, 13}}, {{6, 5}},
        {{8, 14}}, {{13, 6}}, {{3, 12}}, {{9, 3}}
    }};
    const auto sector = [&](int x, int y) {
        const double dx = static_cast<double>(x) + 0.5 -
                          (static_cast<double>(footprint.x) + footprint.side / 2.0);
        const double dy = static_cast<double>(y) + 0.5 -
                          (static_cast<double>(footprint.y) + footprint.side / 2.0);
        double angle = std::atan2(dy, dx);
        if (angle < 0) angle += 2.0 * std::numbers::pi;
        return static_cast<int>(angle * 16.0 / (2.0 * std::numbers::pi));
    };
    const auto distance = [&](int x, int y) {
        return std::max({0, footprint.x - x, x - (footprint.x + footprint.side - 1),
                        footprint.y - y, y - (footprint.y + footprint.side - 1)});
    };
    DesirabilityMap angular(24, 24);
    for (const auto wall : walls) angular.set_wall(wall[0], wall[1]);
    angular.apply_source({footprint, {-11, 1, 0, 10}}, WallBlocking::ExperimentalUniform16);
    for (int y = 0; y < 24; ++y) {
        for (int x = 0; x < 24; ++x) {
            const auto ring = distance(x, y);
            int expected = ring >= 1 && ring <= 10 ? -11 : 0;
            for (const auto wall : walls) {
                if ((wall[0] == x && wall[1] == y) ||
                    (sector(wall[0], wall[1]) == sector(x, y) && distance(wall[0], wall[1]) < ring))
                    expected = 0;
            }
            check(angular.at(x, y) == expected,
                  "experimental integer sectors agree with independent angular oracle");
        }
    }
}

void invalid_inputs_and_atomicity() {
    for (const auto dimensions : std::array<std::array<int, 2>, 9>{{
             {{0, 1}}, {{1, 0}}, {{-1, 4}}, {{4, -1}}, {{229, 1}}, {{1, 229}},
             {{std::numeric_limits<int>::min(), 1}}, {{1, std::numeric_limits<int>::max()}},
             {{std::numeric_limits<int>::max(), std::numeric_limits<int>::max()}}
         }}) {
        expect_exception<std::invalid_argument>([&] { DesirabilityMap bad(dimensions[0], dimensions[1]); },
                                                "dimension rejected before allocation");
    }
    DesirabilityMap maximum(original::desirability_max_dimension, original::desirability_max_dimension);
    check(maximum.at(227, 227) == 0, "maximum bounded grid is valid");
    DesirabilityMap map(10, 10);
    map.set_wall(9, 9);
    const DesirabilitySource good{{4, 4, 1}, {19, 1, 0, 5}};
    map.apply_source(good);
    const auto before = snapshot(map);
    for (const auto side : {0, -1, 7, std::numeric_limits<int>::min(), std::numeric_limits<int>::max()}) {
        expect_exception<std::invalid_argument>([&] { map.apply_source({{0, 0, side}, {1, 1, 0, 1}}); },
                                                "invalid source side rejected");
        expect_exception<std::invalid_argument>([&] { (void)map.building_value({0, 0, side}); },
                                                "invalid query side rejected");
    }
    for (const auto f : std::array<Footprint, 8>{{
             {-1, 0, 1}, {0, -1, 1}, {10, 0, 1}, {0, 10, 1}, {9, 9, 2},
             {std::numeric_limits<int>::max(), 0, 6},
             {0, std::numeric_limits<int>::min(), 6}, {-1, -1, 2}
         }}) {
        expect_exception<std::out_of_range>([&] { map.apply_source({f, {1, 1, 0, 1}}); },
                                            "source must fit wholly on map");
        expect_exception<std::out_of_range>([&] { (void)map.building_value(f); },
                                            "query must fit wholly on map, no off-map zero");
    }
    for (const auto p : std::array<std::array<int, 2>, 5>{{
             {{-1, 0}}, {{0, -1}}, {{10, 0}}, {{0, 10}},
             {{std::numeric_limits<int>::max(), std::numeric_limits<int>::min()}}
         }}) {
        expect_exception<std::out_of_range>([&] { (void)map.at(p[0], p[1]); }, "invalid cell query");
        expect_exception<std::out_of_range>([&] { (void)map.wall(p[0], p[1]); }, "invalid wall query");
        expect_exception<std::out_of_range>([&] { map.set_wall(p[0], p[1]); }, "invalid wall write");
    }
    expect_exception<std::invalid_argument>([&] {
        map.apply_source(good, static_cast<WallBlocking>(255));
    }, "invalid wall mode rejected");
    expect_exception<std::invalid_argument>([&] {
        map.apply_sources({}, static_cast<WallBlocking>(255));
    }, "invalid wall mode rejected even for empty batch");
    const std::array bad_batch{good, DesirabilitySource{{10, 0, 1}, {1, 1, 0, 1}}};
    expect_exception<std::out_of_range>([&] { map.apply_sources(bad_batch); },
                                        "late invalid source rejects whole batch");
    std::vector<DesirabilitySource> oversized(original::desirability_max_sources + 1, good);
    expect_exception<std::length_error>([&] { map.apply_sources(oversized); }, "source count budget enforced");
    check(snapshot(map) == before && map.wall(9, 9), "all rejected input preserves cells and walls");

    constexpr auto low = std::numeric_limits<std::int32_t>::min();
    constexpr auto high = std::numeric_limits<std::int32_t>::max();
    for (const auto spec : std::array<SpreadSpec, 6>{{
             {high - 99, 1, 0, 1}, {low + 99, 1, 0, 1},
             {1, 1, high, 2}, {-1, 1, low, 2},
             {high - 100, 1, 1, 2}, {low + 100, 1, -1, 2}
         }}) {
        const std::array sources{good, DesirabilitySource{{4, 4, 1}, spec}};
        expect_exception<std::overflow_error>([&] { map.apply_sources(sources); },
                                              "unsafe ring value or advance rejects whole batch");
        check(snapshot(map) == before && map.wall(9, 9), "arithmetic rejection is atomic");
    }
    for (const auto value : {low + 100, high - 100}) {
        DesirabilityMap bounded(3, 3);
        bounded.apply_source({{1, 1, 1}, {value, 1, 0, 1}});
        check(bounded.at(0, 0) == (value < 0 ? -100 : 100), "safe int32 contribution boundaries saturate");
    }
    DesirabilityMap no_unused_advance(3, 3);
    no_unused_advance.apply_source({{1, 1, 1}, {1, 1, high, 1}});
    check(no_unused_advance.at(0, 0) == 1, "unused update after last ring is not evaluated");
    no_unused_advance.apply_source({{1, 1, 1}, {0, 1, high, 10}});
    check(no_unused_advance.at(0, 0) == 1, "zero initial value skips otherwise unsafe advances");
    no_unused_advance.apply_source({{1, 1, 1}, {high, 1, high, 0}});
    check(no_unused_advance.at(0, 0) == 1, "no active ring does not evaluate its unused value");

    std::vector<DesirabilitySource> maximum_sources(original::desirability_max_sources,
                                                   {{1, 1, 1}, {1, 1, 0, 0}});
    no_unused_advance.apply_sources(maximum_sources);
    check(no_unused_advance.at(0, 0) == 1, "maximum source budget is accepted");
}

void deterministic_reference_cases() {
    std::uint32_t state = 0x51a7b923U;
    const auto next = [&]() { state = state * 1664525U + 1013904223U; return state; };
    for (int trial = 0; trial < 180; ++trial) {
        const int width = 7 + static_cast<int>(next() % 13U);
        const int height = 7 + static_cast<int>(next() % 13U);
        std::vector<DesirabilitySource> sources;
        const auto count = next() % 13U;
        for (std::uint32_t n = 0; n < count; ++n) {
            const int side = 1 + static_cast<int>(next() % 6U);
            const int x = static_cast<int>(next() % static_cast<std::uint32_t>(width - side + 1));
            const int y = static_cast<int>(next() % static_cast<std::uint32_t>(height - side + 1));
            const auto value = static_cast<std::int32_t>(next() % 243U) - 121;
            const auto step = static_cast<std::int32_t>(next() % 7U) - 2;
            const auto delta = static_cast<std::int32_t>(next() % 35U) - 17;
            const auto range = static_cast<std::int32_t>(next() % 16U) - 2;
            sources.push_back({{x, y, side}, {value, step, delta, range}});
        }
        check_reference(width, height, sources, "fixed-seed mixed-source mathematical reference");
        DesirabilityMap repeated(width, height);
        repeated.apply_sources(sources);
        repeated.clear();
        repeated.apply_sources(sources);
        check(snapshot(repeated) == reference(width, height, sources), "clear and rebuild is deterministic");
        auto mirrored_sources = sources;
        for (auto& source : mirrored_sources)
            source.footprint.x = width - source.footprint.x - source.footprint.side;
        DesirabilityMap mirrored(width, height);
        mirrored.apply_sources(mirrored_sources);
        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x)
                check(repeated.at(x, y) == mirrored.at(width - 1 - x, y), "wall-free reflection symmetry");
        auto translated_sources = sources;
        for (auto& source : translated_sources) {
            source.footprint.x += 3;
            source.footprint.y += 3;
        }
        DesirabilityMap translated(width + 6, height + 6);
        translated.apply_sources(translated_sources);
        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x)
                check(repeated.at(x, y) == translated.at(x + 3, y + 3),
                      "wall-free translation preserves original cells even when rings gain outside space");
    }
}

constexpr std::array difficulties{Difficulty::VeryEasy, Difficulty::Easy, Difficulty::Normal,
                                   Difficulty::Hard, Difficulty::VeryHard};
constexpr std::array<std::string_view, 5> difficulty_labels{
    "Very Easy Modifiers", "Easy Modifiers", "Normal Modifier", "Hard Modifiers", "Very Hard Modifiers"
};

template<std::size_t N>
std::string row(std::string_view label, const std::array<std::int32_t, N>& values) {
    std::string result(label);
    result += ",{,";
    for (const auto value : values) result += std::to_string(value) + ',';
    return result + "},,,\n";
}

original::ModelData model_fixture(bool include_well = true, std::int32_t modifier = 125,
                                  bool correct_label = true) {
    original::BuildingRow values{};
    values[1] = 12;
    values[2] = 2;
    values[3] = -3;
    values[4] = 4;
    original::BuildingRow modifiers{};
    modifiers.fill(modifier);
    std::string text = "Authored desirability fixture\nBUILDING MODS\n";
    for (std::size_t i = 0; i < difficulties.size(); ++i)
        text += row(std::to_string(i) + ',' + std::string(difficulty_labels[i]), modifiers);
    text += "ALL BUILDINGS\n";
    text += row(include_well ? (correct_label ? "72,Well" : "72,Authored mismatched label")
                             : "35,Clay Pit", values);
    text += "HOUSE MODS\n";
    for (const auto label : difficulty_labels) text += row(label, original::HouseRow{});
    text += "ALL HOUSES\n";
    for (std::size_t i = 0; i < original::house_row_count; ++i)
        text += row("Authored house " + std::to_string(i), original::HouseRow{});
    text += "End of model data.\n-------------------\n";
    auto parsed = original::parse_model_data(text);
    check(static_cast<bool>(parsed), parsed.error ? parsed.error->reason : "synthetic model parsed");
    return std::move(*parsed.model);
}

void model_adapter() {
    const auto model = model_fixture();
    for (const auto difficulty : difficulties) {
        const auto spread = original::spread_spec(model, ModelBuilding::Well, difficulty);
        check(spread.value == 15 && spread.step == 2 && spread.step_size == -3 && spread.range == 5,
              "four model fields receive signed difficulty preparation");
        const std::array sources{DesirabilitySource{{6, 6, 2}, spread}};
        check_reference(18, 18, sources, "model-derived source with independently supplied footprint");
    }
    const auto missing = model_fixture(false);
    expect_exception<std::out_of_range>([&] {
        (void)original::spread_spec(missing, ModelBuilding::Well, Difficulty::Normal);
    }, "missing original source does not acquire a spread default");
    expect_exception<std::out_of_range>([&] {
        (void)original::spread_spec(model, static_cast<ModelBuilding>(0), Difficulty::Normal);
    }, "invalid named model identity rejected");
    expect_exception<std::out_of_range>([&] {
        (void)original::spread_spec(model, ModelBuilding::Well, static_cast<Difficulty>(5));
    }, "invalid difficulty rejected");
    const auto mislabeled = model_fixture(true, 125, false);
    expect_exception<std::invalid_argument>([&] {
        (void)original::spread_spec(mislabeled, ModelBuilding::Well, Difficulty::Normal);
    }, "incorrect source label cannot acquire named original spread semantics");
    const auto overflowing = model_fixture(true, std::numeric_limits<std::int32_t>::max());
    expect_exception<std::overflow_error>([&] {
        (void)original::spread_spec(overflowing, ModelBuilding::Well, Difficulty::Normal);
    }, "model preparation rejects intermediate overflow");
}

std::string read_bytes(const fs::path& source) {
    const auto size = fs::file_size(source);
    check(size <= original::max_model_bytes, "bounded original source read");
    std::ifstream input(source, std::ios::binary);
    check(static_cast<bool>(input), "original source can be read");
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
        if (status.type() == fs::file_type::not_found) {
            std::cout << "SKIP: original model path missing\n";
            return 77;
        }
        check(!error, "existing original path metadata must be readable");
        const auto followed = fs::status(path, error);
        check(!error, "existing original path must resolve");
        check(path == root / "Model/EmperorBuildingModels.txt" ? fs::is_regular_file(followed)
                                                             : fs::is_directory(followed),
              "existing original path has expected type");
    }
    const auto source = root / "Model/EmperorBuildingModels.txt";
    const auto initial_size = fs::file_size(source);
    const auto initial_time = fs::last_write_time(source);
    auto loaded = original::load_model_data(root);
    check(static_cast<bool>(loaded), loaded.error ? loaded.error->reason : "real model loaded");
    const auto before = read_bytes(source);
    const auto& model = *loaded.model;
    check(model.provenance().sha256 == "7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6",
          "read-only integration uses the documented original model hash");
    constexpr std::array buildings{
        ModelBuilding::ClayPit, ModelBuilding::Kiln, ModelBuilding::Warehouse,
        ModelBuilding::CommonMarketSquare, ModelBuilding::Well, ModelBuilding::Farmhouse,
        ModelBuilding::HerbalistsStall
    };
    constexpr std::array fields{
        original::BuildingField::Desirability, original::BuildingField::DesirabilityStep,
        original::BuildingField::DesirabilityStepSize, original::BuildingField::DesirabilityRange
    };
    std::size_t sources_checked = 0;
    for (const auto difficulty : difficulties) {
        std::vector<DesirabilitySource> sources;
        for (std::size_t b = 0; b < buildings.size(); ++b) {
            const auto spread = original::spread_spec(model, buildings[b], difficulty);
            const std::array actual{spread.value, spread.step, spread.step_size, spread.range};
            for (std::size_t f = 0; f < fields.size(); ++f) {
                const auto raw = model.building(original::model_id(buildings[b]), fields[f]);
                const auto product = static_cast<std::int64_t>(raw) * model.building_modifier(difficulty, fields[f]);
                check(product >= std::numeric_limits<std::int32_t>::min() &&
                      product <= std::numeric_limits<std::int32_t>::max(), "real field transform fits original arithmetic domain");
                check(actual[f] == product / 100, "model spread field agrees with independent signed percentage arithmetic");
            }
            // Authored placements and side lengths, not inferred original geometry.
            sources.push_back({{4 + static_cast<int>(b * 3), 11, 1 + static_cast<int>(b % 6)}, spread});
            ++sources_checked;
        }
        check_reference(40, 30, sources, "real-model ordered sources agree with mathematical reference");
    }
    check(read_bytes(source) == before, "real model bytes remain unchanged");
    check(fs::file_size(source) == initial_size && fs::last_write_time(source) == initial_time,
          "real model size and modification time remain unchanged");
    std::cout << "real model sources=" << sources_checked << "; SHA-256=" << model.provenance().sha256 << '\n';
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    try {
        if (argc == 3 && std::string_view(argv[1]) == "--real") {
            const auto result = real_model(argv[2]);
            if (result == 77) return result;
        } else if (argc == 1) {
            single_sources_and_steps();
            footprints_edges_and_order();
            house_queries();
            walls_are_explicit_experiments();
            invalid_inputs_and_atomicity();
            deterministic_reference_cases();
            model_adapter();
        } else {
            std::cerr << "usage: original-desirability-tests [--real ROOT]\n";
            return 2;
        }
        std::cout << "original desirability checks passed: " << checks << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "original desirability test failed: " << error.what() << '\n';
        return 1;
    }
}
