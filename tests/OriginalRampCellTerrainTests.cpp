#include "original-ramp/CellTerrain.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

using namespace openemperor::original_ramp;
namespace {
void check(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
void near(double a, double b, const char* message) {
    if (std::abs(a - b) > 1e-8) throw std::runtime_error(message);
}
Cell add(Cell a, Cell b) { return {a.x + b.x, a.y + b.y}; }
Cell lateral(Direction direction) {
    const auto d = direction_delta(direction);
    return {-d.y, d.x};
}
std::vector<CellState> snapshot(const CellTerrain& terrain) {
    return {terrain.cells().begin(), terrain.cells().end()};
}

CellTerrain plateau_scene(Direction higher, int base) {
    CellTerrain terrain(14, 14, base);
    const auto d = direction_delta(higher);
    const Cell source{6, 6};
    for (int y = 0; y < terrain.height(); ++y)
        for (int x = 0; x < terrain.width(); ++x)
            if ((x - source.x) * d.x + (y - source.y) * d.y > 0)
                check(terrain.set_plateau({x, y}, base + 1), "high plateau setup failed");
    return terrain;
}
} // namespace

int main() {
    try {
        constexpr std::array directions{Direction::North, Direction::East,
                                        Direction::South, Direction::West};
        constexpr std::array ids{235, 236, 233, 234};
        constexpr std::array keys{0x2050, 0x2051, 0x204e, 0x204f};
        for (std::size_t i = 0; i < directions.size(); ++i)
            for (int style = 0; style < 3; ++style) {
                const auto info = resource_info(static_cast<SurfaceStyle>(style), directions[i]);
                check(info.physical_id == ids[i] + style * 4 &&
                      info.resource_key == static_cast<unsigned>(keys[i] + style * 4),
                      "original selector key/physical index domains diverged");
            }

        int round_trips = 0;
        // One through four laterally joined native source cells, both storage
        // axes, all signed gradients and signed-byte extremes. This measures
        // the diagnostic surface hypothesis, not original actor restoration.
        for (const auto direction : directions)
            for (const int base : {-128, -2, 0, 126})
                for (int count = 1; count <= 4; ++count) {
                    auto terrain = plateau_scene(direction, base);
                    const auto d = direction_delta(direction);
                    const auto across = lateral(direction);
                    for (int n = 0; n < count; ++n) {
                        const Cell source{6 + across.x * n, 6 + across.y * n};
                        const Cell low = add(source, direction_delta(opposite(direction)));
                        const Cell high = add(source, d);
                        check(terrain.set_road(low) && terrain.set_road(high), "endpoint road failed");
                        check(!terrain.edge_connected(low, source), "unbuilt ramp traversable");
                        check(terrain.place_ramp(source, direction, SurfaceStyle::Stairs),
                              "legal original source-cell pattern refused");
                        check(terrain.edge_connected(low, source) && terrain.edge_connected(source, high),
                              "source-cell low/high boundary disconnected");
                        for (double sign : {-1.0, 1.0}) {
                            const Vec2 boundary{source.x + d.x * sign * .5,
                                                source.y + d.y * sign * .5};
                            const Cell neighbor = sign < 0 ? low : high;
                            near(*terrain.surface_height_in_cell(source, boundary),
                                 *terrain.surface_height_in_cell(neighbor, boundary),
                                 "slope/plateau has a height discontinuity");
                        }
                        if (n > 0) {
                            const Cell previous{source.x - across.x, source.y - across.y};
                            check(!terrain.edge_connected(previous, source),
                                  "native bank opened an unverified lateral ramp traversal");
                            const auto around_bank = terrain.route(previous, source);
                            check(around_bank && around_bank->size() == 4 &&
                                  !terrain.at(around_bank->at(1)).higher &&
                                  !terrain.at(around_bank->at(2)).higher,
                                  "adjacent ramp route did not go around native banks via plateau");
                            for (double along : {-.5, -.2, 0.0, .3, .5}) {
                                const Vec2 seam{source.x - across.x * .5 + d.x * along,
                                                source.y - across.y * .5 + d.y * along};
                                near(*terrain.surface_height_in_cell(previous, seam),
                                     *terrain.surface_height_in_cell(source, seam),
                                     "joined source cells disagree on shared surface");
                            }
                        }
                        const auto path = terrain.route(low, high);
                        check(path && path->size() == 3 && path->at(1) == source,
                              "prototype route bypasses source-cell slope");
                        Walker walker;
                        check(walker.reset(terrain, low) && walker.target(terrain, high),
                              "walker dispatch failed");
                        for (int step = 0; step <= 200; ++step) {
                            const auto foot = walker.position(terrain);
                            const double distance = step * .01;
                            near(foot.height, base + std::clamp(distance - .5, 0.0, 1.0),
                                 "outbound walker interpolated center heights instead of source surface");
                            near(*terrain.surface_height({foot.x, foot.y}), foot.height,
                                 "outbound feet differ from picked terrain surface");
                            if (step < 200) walker.update(terrain, .01);
                        }
                        check(!walker.moving() && walker.current_cell() == high,
                              "walker did not arrive at high plateau");
                        check(walker.target(terrain, low), "return dispatch failed");
                        for (int step = 0; step <= 200; ++step) {
                            const auto foot = walker.position(terrain);
                            const double distance = step * .01;
                            near(foot.height, base + 1 - std::clamp(distance - .5, 0.0, 1.0),
                                 "return walker surface is asymmetric");
                            if (step < 200) walker.update(terrain, .01);
                        }
                        check(!walker.moving() && walker.current_cell() == low,
                              "walker did not return to low plateau");
                        ++round_trips;
                    }
                }

        {
            auto terrain = plateau_scene(Direction::East, 0);
            check(terrain.place_ramp({6, 6}, Direction::East, SurfaceStyle::Earth), "setup failed");
            check(terrain.set_road({6, 5}), "lateral setup failed");
            check(!terrain.edge_connected({6, 6}, {6, 5}),
                  "one shared-edge point opened a lateral cliff");
            const auto before = snapshot(terrain);
            check(!terrain.place_ramp({6, 6}, Direction::West, SurfaceStyle::Earth) &&
                  !terrain.place_ramp({0, 0}, Direction::East, SurfaceStyle::Earth) &&
                  !terrain.set_plateau({7, 6}, 0) && !terrain.set_plateau({6, 5}, 1) &&
                  !terrain.set_plateau({2, 2}, 128) && !terrain.set_plateau({2, 2}, -129),
                  "invalid signed/neighborhood edit accepted");
            check(snapshot(terrain) == before, "failed edit partially changed terrain");
            check(!terrain.cell_at({-.50001, 0}) && !terrain.cell_at({13.5, 0}) &&
                  !terrain.cell_at({std::numeric_limits<double>::infinity(), 0}),
                  "half-open or nonfinite bounds accepted");
            check(terrain.cell_at({-.5, -.5}) == Cell{0, 0} &&
                  terrain.cell_at({.5, .5}) == Cell{1, 1}, "half-open cell owner changed");
            check(terrain.clear_ramp({6, 6}) && !terrain.route({6, 6}, {7, 6}),
                  "removing diagnostic slope left height transition traversable");
        }

        for (const auto direction : directions) {
            const Cell source{6, 6};
            const auto d = direction_delta(direction);
            const auto across = lateral(direction);
            const Cell diagonal{source.x + d.x + across.x, source.y + d.y + across.y};
            auto terrain = plateau_scene(direction, 0);
            check(terrain.set_plateau(diagonal, 2), "invalid diagonal setup failed");
            const auto unbuilt = snapshot(terrain);
            check(!terrain.place_ramp(source, direction, SurfaceStyle::Earth) &&
                  snapshot(terrain) == unbuilt,
                  "high-facing diagonal outside original abs(delta)<=1 accepted");
            check(terrain.set_plateau(diagonal, -1) &&
                  terrain.place_ramp(source, direction, SurfaceStyle::Earth),
                  "signed delta -1 original high-facing diagonal refused");
            const auto built = snapshot(terrain);
            check(!terrain.set_plateau(diagonal, -2) && snapshot(terrain) == built,
                  "diagonal terrain edit invalidated an existing ramp or mutated on refusal");

            auto conservative = plateau_scene(direction, 0);
            const auto low_neighbor = add(source, direction_delta(opposite(direction)));
            check(conservative.set_plateau(low_neighbor, -1), "opposite wildcard setup failed");
            const auto before = snapshot(conservative);
            check(!conservative.place_ramp(source, direction, SurfaceStyle::Earth) &&
                  snapshot(conservative) == before,
                  "prototype conservative opposite-low join restriction weakened");
        }

        {
            // Signed height is applied exactly once. The known-height inverse
            // remains valid even where an entire planar slope is edge-on.
            for (double height_step : {20.0, 40.0})
                for (double h : {-128.0, -1.5, 0.0, 126.5, 127.0})
                    for (const Vec2 position : {Vec2{0, 0}, Vec2{3.27, 8.91}, Vec2{12.5, 1.5}}) {
                        const auto screen = project({position.x, position.y, h}, height_step);
                        const auto inverse = unproject(screen, h, height_step);
                        near(inverse.x, position.x, "signed projection X inverse failed");
                        near(inverse.y, position.y, "signed projection Y inverse failed");
                    }
            CellTerrain flat(6, 6, -2);
            for (const Cell cell : {Cell{0, 0}, Cell{2, 3}, Cell{5, 5}}) {
                const auto pick = pick_surface(flat, project({double(cell.x), double(cell.y), -2}));
                check(pick && pick->cell == cell, "flat signed surface picker missed center");
                near(pick->position.height, -2, "surface picker ignored signed plateau");
            }
            check(!pick_surface(flat, {std::numeric_limits<double>::quiet_NaN(), 0}),
                  "nonfinite picker input accepted");
            // An East-rising one-cell plane collapses under height40. Record
            // this actual geometric limitation instead of inventing an inverse.
            const Vec2 a = project({5.5, 5.5, 0});
            const Vec2 b = project({6.5, 5.5, 1});
            const Vec2 c = project({5.5, 6.5, 0});
            near((b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x), 0,
                 "expected edge-on original-height hypothesis changed");
        }

        {
            CellTerrain terrain(6, 6);
            for (int x = 1; x <= 4; ++x) check(terrain.set_road({x, 2}), "road setup failed");
            Walker a, b;
            check(a.reset(terrain, {1, 2}) && a.target(terrain, {4, 2}) &&
                  b.reset(terrain, {1, 2}) && b.target(terrain, {4, 2}), "dispatch failed");
            a.update(terrain, 2.25);
            for (int i = 0; i < 225; ++i) b.update(terrain, .01);
            near(a.position(terrain).x, b.position(terrain).x, "walker depends on frame partition");
            check(terrain.set_road({4, 2}, false), "road removal failed");
            a.update(terrain, .1);
            check(!a.moving() && a.current_cell() == Cell{3, 2},
                  "diagnostic walker entered an edited blocked edge");
        }
        std::cout << "OriginalRampCellTerrainTests PASS: " << round_trips
                  << " signed source-cell round trips; resource keys, joins, atomic edits, projection and picking\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "OriginalRampCellTerrainTests FAIL: " << error.what() << '\n';
        return 1;
    }
}
