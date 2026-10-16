#include "CellTerrain.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <deque>
#include <limits>
#include <stdexcept>
#include <utility>

namespace openemperor::original_ramp {
namespace {
constexpr std::array directions{Direction::North, Direction::East,
                                Direction::South, Direction::West};
constexpr double epsilon = 1e-9;
Cell add(Cell a, Cell b) { return {a.x + b.x, a.y + b.y}; }
bool valid_height(int height) { return height >= -128 && height <= 127; }

double height_in(const CellState& state, Cell cell, Vec2 position) {
    if (!state.higher) return state.low_height;
    const auto delta = direction_delta(*state.higher);
    return state.low_height + 0.5 + (position.x - cell.x) * delta.x +
           (position.y - cell.y) * delta.y;
}
} // namespace

Cell direction_delta(Direction direction) {
    switch (direction) {
    case Direction::North: return {0, -1};
    case Direction::East: return {1, 0};
    case Direction::South: return {0, 1};
    case Direction::West: return {-1, 0};
    }
    throw std::invalid_argument("invalid prototype direction");
}

Direction opposite(Direction direction) {
    switch (direction) {
    case Direction::North: return Direction::South;
    case Direction::East: return Direction::West;
    case Direction::South: return Direction::North;
    case Direction::West: return Direction::East;
    }
    throw std::invalid_argument("invalid prototype direction");
}

ResourceInfo resource_info(SurfaceStyle style, Direction higher) {
    int orientation = 0;
    switch (higher) {
    case Direction::South: orientation = 0; break;
    case Direction::West: orientation = 1; break;
    case Direction::North: orientation = 2; break;
    case Direction::East: orientation = 3; break;
    default: throw std::invalid_argument("invalid prototype direction");
    }
    int variant = 0;
    switch (style) {
    case SurfaceStyle::Earth: variant = 0; break;
    case SurfaceStyle::Stairs: variant = 4; break;
    case SurfaceStyle::Cobbled: variant = 8; break;
    default: throw std::invalid_argument("invalid prototype style");
    }
    return {static_cast<std::uint32_t>(0x204e + orientation + variant),
            static_cast<std::uint16_t>(233 + orientation + variant)};
}

CellTerrain::CellTerrain(int width, int height, int default_height)
    : width_(width), height_(height) {
    if (width <= 0 || height <= 0 || width > max_dimension || height > max_dimension ||
        !valid_height(default_height))
        throw std::invalid_argument("prototype terrain dimensions or height out of range");
    cells_.resize(static_cast<std::size_t>(width * height));
    for (auto& cell : cells_) cell.low_height = default_height;
}

bool CellTerrain::contains(Cell cell) const {
    return cell.x >= 0 && cell.y >= 0 && cell.x < width_ && cell.y < height_;
}

std::size_t CellTerrain::index(Cell cell) const {
    return static_cast<std::size_t>(cell.y * width_ + cell.x);
}

const CellState& CellTerrain::at(Cell cell) const {
    if (!contains(cell)) throw std::out_of_range("prototype cell outside terrain");
    return cells_[index(cell)];
}

bool CellTerrain::ramp_pattern(Cell cell, Direction higher,
                               std::optional<std::pair<Cell, int>> changed_height) const {
    if (!contains(cell)) return false;
    const auto height_at = [&](Cell c) {
        return changed_height && changed_height->first == c ?
            changed_height->second : at(c).low_height;
    };
    const int low = height_at(cell);
    if (low >= 127) return false;
    for (const auto direction : directions) {
        const auto neighbor = add(cell, direction_delta(direction));
        if (!contains(neighbor) || height_at(neighbor) != low + (direction == higher ? 1 : 0))
            return false;
    }
    const auto high_delta = direction_delta(higher);
    const Cell lateral{-high_delta.y, high_delta.x};
    for (const int sign : {-1, 1}) {
        const Cell diagonal{cell.x + high_delta.x + lateral.x * sign,
                            cell.y + high_delta.y + lateral.y * sign};
        if (!contains(diagonal) || std::abs(height_at(diagonal) - low) > 1) return false;
    }
    return true;
}

bool CellTerrain::set_plateau(Cell cell, int height) {
    if (!contains(cell) || !valid_height(height)) return false;
    const auto change = std::pair{cell, height};
    for (int y = 0; y < height_; ++y)
        for (int x = 0; x < width_; ++x) {
            const Cell candidate{x, y};
            if (candidate != cell && at(candidate).higher &&
                !ramp_pattern(candidate, *at(candidate).higher, change)) return false;
        }
    auto& state = cells_[index(cell)];
    state.low_height = height;
    state.higher.reset();
    return true;
}

bool CellTerrain::set_road(Cell cell, bool road) {
    if (!contains(cell)) return false;
    cells_[index(cell)].road = road;
    return true;
}

bool CellTerrain::can_place_ramp(Cell cell, Direction higher) const {
    // Validate the enum before using it in selector comparisons.
    (void)direction_delta(higher);
    return ramp_pattern(cell, higher);
}

bool CellTerrain::place_ramp(Cell cell, Direction higher, SurfaceStyle style) {
    (void)resource_info(style, higher);
    if (!can_place_ramp(cell, higher)) return false;
    auto& state = cells_[index(cell)];
    state.higher = higher;
    state.style = style;
    state.road = true;
    return true;
}

bool CellTerrain::clear_ramp(Cell cell) {
    if (!contains(cell) || !at(cell).higher) return false;
    cells_[index(cell)].higher.reset();
    return true;
}

std::optional<Cell> CellTerrain::cell_at(Vec2 position) const {
    if (!std::isfinite(position.x) || !std::isfinite(position.y) ||
        position.x < -0.5 || position.y < -0.5 ||
        position.x >= width_ - 0.5 || position.y >= height_ - 0.5) return {};
    return Cell{static_cast<int>(std::floor(position.x + 0.5)),
                static_cast<int>(std::floor(position.y + 0.5))};
}

std::optional<double> CellTerrain::surface_height(Vec2 position) const {
    const auto owner = cell_at(position);
    if (!owner) return {};
    return height_in(at(*owner), *owner, position);
}

std::optional<double> CellTerrain::surface_height_in_cell(Cell cell, Vec2 position) const {
    if (!contains(cell) || !std::isfinite(position.x) || !std::isfinite(position.y) ||
        std::abs(position.x - cell.x) > 0.5 + epsilon ||
        std::abs(position.y - cell.y) > 0.5 + epsilon) return {};
    return height_in(at(cell), cell, position);
}

double CellTerrain::center_height(Cell cell) const {
    return at(cell).low_height + (at(cell).higher ? 0.5 : 0.0);
}

bool CellTerrain::edge_connected(Cell from, Cell to) const {
    if (!contains(from) || !contains(to) || !at(from).road || !at(to).road) return false;
    const int dx = to.x - from.x, dy = to.y - from.y;
    if (std::abs(dx) + std::abs(dy) != 1) return false;
    for (const auto cell : {from, to}) {
        if (!at(cell).higher) continue;
        const auto axis = direction_delta(*at(cell).higher);
        if (std::abs(dx * axis.x + dy * axis.y) != 1) return false;
    }
    // Check both ends of the shared edge: equality only at its midpoint would
    // permit a lateral cliff or crossed slopes to masquerade as a road join.
    const Vec2 middle{(from.x + to.x) * 0.5, (from.y + to.y) * 0.5};
    for (const double side : {-0.5, 0.5}) {
        const Vec2 boundary{middle.x + dy * side, middle.y + dx * side};
        if (std::abs(height_in(at(from), from, boundary) -
                     height_in(at(to), to, boundary)) > epsilon) return false;
    }
    return true;
}

std::optional<std::vector<Cell>> CellTerrain::route(Cell from, Cell to) const {
    if (!contains(from) || !contains(to) || !at(from).road || !at(to).road) return {};
    const std::size_t unseen = cells_.size();
    std::vector<std::size_t> previous(cells_.size(), unseen);
    std::deque<Cell> queue{from};
    previous[index(from)] = index(from);
    while (!queue.empty()) {
        const Cell current = queue.front();
        queue.pop_front();
        if (current == to) {
            std::vector<Cell> result;
            auto cursor = index(to);
            for (;;) {
                result.push_back({static_cast<int>(cursor % static_cast<std::size_t>(width_)),
                                  static_cast<int>(cursor / static_cast<std::size_t>(width_))});
                if (cursor == index(from)) break;
                cursor = previous[cursor];
            }
            std::reverse(result.begin(), result.end());
            return result;
        }
        for (const auto direction : directions) {
            const auto next = add(current, direction_delta(direction));
            if (!edge_connected(current, next) || previous[index(next)] != unseen) continue;
            previous[index(next)] = index(current);
            queue.push_back(next);
        }
    }
    return {};
}

Vec2 project(Vec3 position, double height_step) {
    return {(position.x - position.y) * 40.0,
            (position.x + position.y) * 20.0 - position.height * height_step};
}

Vec2 unproject(Vec2 world, double known_height, double height_step) {
    const double ground_y = world.y + known_height * height_step;
    return {world.x / 80.0 + ground_y / 40.0,
            ground_y / 40.0 - world.x / 80.0};
}

std::optional<SurfacePick> pick_surface(const CellTerrain& terrain, Vec2 world,
                                       double height_step) {
    if (!std::isfinite(world.x) || !std::isfinite(world.y) ||
        !std::isfinite(height_step) || height_step <= 0) return {};
    std::optional<SurfacePick> result;
    double front = -std::numeric_limits<double>::infinity();
    for (int y = 0; y < terrain.height(); ++y)
        for (int x = 0; x < terrain.width(); ++x) {
            const Cell cell{x, y};
            const auto& state = terrain.at(cell);
            const auto delta = state.higher ? direction_delta(*state.higher) : Cell{};
            const double constant = state.low_height + (state.higher ? 0.5 : 0.0) -
                                    delta.x * x - delta.y * y;
            const double a = 20.0 - height_step * delta.x;
            const double b = 20.0 - height_step * delta.y;
            if (std::abs(a + b) < epsilon) continue;
            const double px = (world.y + height_step * constant + b * world.x / 40.0) / (a + b);
            const double py = px - world.x / 40.0;
            if (px < x - 0.5 || px >= x + 0.5 || py < y - 0.5 || py >= y + 0.5) continue;
            // The ray's frontmost intersection has the greatest x+y. Stable
            // storage-order ties make exact half-open boundary behavior explicit.
            if (px + py <= front) continue;
            front = px + py;
            result = SurfacePick{cell, {px, py, height_in(state, cell, {px, py})}};
        }
    return result;
}

bool Walker::reset(const CellTerrain& terrain, Cell cell) {
    if (!terrain.contains(cell) || !terrain.at(cell).road) return false;
    path_ = {cell};
    path_vertex_ = 0;
    edge_progress_ = 0;
    return true;
}

bool Walker::target(const CellTerrain& terrain, Cell destination) {
    if (path_.empty() || moving()) return false;
    auto next = terrain.route(current_cell(), destination);
    if (!next) return false;
    path_ = std::move(*next);
    path_vertex_ = 0;
    edge_progress_ = 0;
    return true;
}

void Walker::set_speed(double cells_per_second) {
    if (!std::isfinite(cells_per_second) || cells_per_second <= 0 || cells_per_second > 20)
        throw std::invalid_argument("prototype walker speed out of range");
    speed_ = cells_per_second;
}

Cell Walker::current_cell() const {
    if (path_.empty()) throw std::logic_error("prototype walker has not been reset");
    return path_[path_vertex_];
}

void Walker::update(const CellTerrain& terrain, double seconds) {
    if (!std::isfinite(seconds) || seconds <= 0 || !moving()) return;
    double remaining = seconds * speed_;
    while (remaining > 0 && moving()) {
        if (!terrain.edge_connected(path_[path_vertex_], path_[path_vertex_ + 1])) {
            // Diagnostic edits can close a route. Finish no invalid edge and
            // return to its last source center; this is not production safety.
            path_.resize(path_vertex_ + 1);
            edge_progress_ = 0;
            break;
        }
        const double step = std::min(remaining, 1.0 - edge_progress_);
        edge_progress_ += step;
        remaining -= step;
        if (edge_progress_ >= 1.0 - epsilon) {
            ++path_vertex_;
            edge_progress_ = 0;
        }
    }
}

Vec3 Walker::position(const CellTerrain& terrain) const {
    const auto from = current_cell();
    if (!moving()) return {double(from.x), double(from.y), terrain.center_height(from)};
    const auto to = path_[path_vertex_ + 1];
    const Vec2 xy{from.x + (to.x - from.x) * edge_progress_,
                  from.y + (to.y - from.y) * edge_progress_};
    const Cell owner = edge_progress_ < 0.5 ? from : to;
    return {xy.x, xy.y, *terrain.surface_height_in_cell(owner, xy)};
}

} // namespace openemperor::original_ramp
