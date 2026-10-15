#include "simulation/original/Desirability.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace openemperor::original {
namespace {

constexpr int sector_count = 16;
constexpr double pi = 3.14159265358979323846;

// Advance the spread value after a step. Crossing zero clamps it to zero for
// good, so a fading source never turns into its opposite.
int advance(int value, int step_size) {
    const int next = value + step_size;
    if ((value > 0 && next <= 0) || (value <= 0 && next >= 0)) return 0;
    return next;
}

int sector_of(double dx, double dy) {
    double angle = std::atan2(dy, dx);  // -pi..pi
    if (angle < 0) angle += 2 * pi;
    const int s = static_cast<int>(angle / (2 * pi) * sector_count);
    return std::min(s, sector_count - 1);
}

std::size_t cell_index(int x, int y, int width) {
    return static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x);
}

}  // namespace

DesirabilityMap::DesirabilityMap(int width, int height)
    : width_(width), height_(height),
      values_(static_cast<std::size_t>(width) * static_cast<std::size_t>(height > 0 ? height : 0), 0),
      walls_(static_cast<std::size_t>(width) * static_cast<std::size_t>(height > 0 ? height : 0), 0) {
    if (width <= 0 || height <= 0) throw std::invalid_argument("desirability map needs positive size");
}

void DesirabilityMap::clear() { std::fill(values_.begin(), values_.end(), 0); }

void DesirabilityMap::set_wall(int x, int y, bool wall) {
    if (inside(x, y)) walls_[cell_index(x, y, width_)] = wall ? 1 : 0;
}

bool DesirabilityMap::wall(int x, int y) const {
    return inside(x, y) && walls_[cell_index(x, y, width_)] != 0;
}

int DesirabilityMap::at(int x, int y) const {
    return inside(x, y) ? values_[cell_index(x, y, width_)] : 0;
}

void DesirabilityMap::add(int x, int y, int value) {
    auto& cell = values_[cell_index(x, y, width_)];
    cell = static_cast<std::int8_t>(std::clamp(int(cell) + value, -desirability_limit, desirability_limit));
}

void DesirabilityMap::apply_source(int x, int y, int size, const SpreadSpec& spec, WallBlocking blocking) {
    if (size < 1) throw std::invalid_argument("source footprint must be at least 1");
    int value = spec.value;
    if (value == 0) return;
    const int rings = std::min(spec.range, desirability_max_range);

    const bool shield = blocking == WallBlocking::Intended && value < 0;
    std::array<int, sector_count> blocked_at;
    blocked_at.fill(std::numeric_limits<int>::max());
    const double cx = x + size / 2.0, cy = y + size / 2.0;

    for (int r = 1; r <= rings; ++r) {
        const int x0 = x - r, y0 = y - r, x1 = x + size - 1 + r, y1 = y + size - 1 + r;
        // Pass 1: find walls on this ring so every tile of the ring sees the
        // same blocked distances regardless of scan order.
        std::array<int, sector_count> ring_blocked = blocked_at;
        if (shield) {
            for (int ty = y0; ty <= y1; ++ty)
                for (int tx = x0; tx <= x1; ++tx) {
                    if (ty != y0 && ty != y1 && tx != x0 && tx != x1) continue;
                    if (!inside(tx, ty) || !wall(tx, ty)) continue;
                    const int s = sector_of(tx + 0.5 - cx, ty + 0.5 - cy);
                    ring_blocked[static_cast<std::size_t>(s)] = std::min(ring_blocked[static_cast<std::size_t>(s)], r);
                }
        }
        // Pass 2: apply.
        for (int ty = y0; ty <= y1; ++ty)
            for (int tx = x0; tx <= x1; ++tx) {
                if (ty != y0 && ty != y1 && tx != x0 && tx != x1) continue;
                if (!inside(tx, ty)) continue;
                if (shield) {
                    if (wall(tx, ty)) continue;  // the wall tile itself gets nothing
                    const int s = sector_of(tx + 0.5 - cx, ty + 0.5 - cy);
                    if (ring_blocked[static_cast<std::size_t>(s)] < r) continue;
                }
                add(tx, ty, value);
            }
        blocked_at = ring_blocked;
        if (spec.step > 0 && r % spec.step == 0) value = advance(value, spec.step_size);
        if (value == 0) break;
    }
}

int DesirabilityMap::building_value(int x, int y, int size, int bonus) const {
    int best = std::numeric_limits<int>::min();
    for (int ty = y; ty < y + size; ++ty)
        for (int tx = x; tx < x + size; ++tx) best = std::max(best, at(tx, ty));
    return best + bonus;
}

}  // namespace openemperor::original
