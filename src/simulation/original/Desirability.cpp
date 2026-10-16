#include "simulation/original/Desirability.h"

#include <algorithm>
#include <array>
#include <limits>
#include <stdexcept>

namespace openemperor::original {
namespace {

[[nodiscard]] std::size_t cell_count(int width, int height) {
    if (width < 1 || height < 1 || width > desirability_max_dimension ||
        height > desirability_max_dimension) {
        throw std::invalid_argument("desirability dimensions must be in 1..228");
    }
    return static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
}

struct PreparedSource {
    Footprint footprint;
    std::array<std::int32_t, desirability_max_range> rings{};
    int range = 0;
    bool negative = false;
};

[[nodiscard]] PreparedSource prepare(const DesirabilitySource& source) {
    PreparedSource result;
    result.footprint = source.footprint;
    result.negative = source.spread.value < 0;
    if (source.spread.value == 0) return result;
    result.range = std::clamp(source.spread.range, 0, desirability_max_range);
    const auto period = std::max(source.spread.step, std::int32_t{1});
    std::int64_t value = source.spread.value;
    for (int ring = 0; ring < result.range; ++ring) {
        // Authored safety domain: adding any stored [-100,100] cell must fit
        // the original signed-int intermediate. Never emulate edited overflow.
        if (value < static_cast<std::int64_t>(std::numeric_limits<std::int32_t>::min()) + 100 ||
            value > static_cast<std::int64_t>(std::numeric_limits<std::int32_t>::max()) - 100) {
            throw std::overflow_error("desirability ring addition outside int32 domain");
        }
        result.rings[static_cast<std::size_t>(ring)] = static_cast<std::int32_t>(value);
        if (ring + 1 == result.range || value == 0 || (ring + 1) % period != 0) continue;
        const auto next = value + source.spread.step_size;
        if (next < std::numeric_limits<std::int32_t>::min() ||
            next > std::numeric_limits<std::int32_t>::max()) {
            throw std::overflow_error("desirability step outside int32 domain");
        }
        value = result.negative ? std::min(next, std::int64_t{0})
                                : std::max(next, std::int64_t{0});
    }
    return result;
}

// Authored uniform 22.5-degree sectors, beginning at +x toward +y. Squaring
// positive integers compares against tan(22.5) = sqrt(2)-1 without floats.
// This intentionally does not claim the original wall classifier or units.
[[nodiscard]] std::size_t experimental_sector(Footprint source, int x, int y) noexcept {
    const std::int64_t dx = 2 * x + 1 - (2 * source.x + source.side);
    const std::int64_t dy = 2 * y + 1 - (2 * source.y + source.side);
    std::int64_t u = 0;
    std::int64_t v = 0;
    std::size_t quadrant = 0;
    if (dx > 0 && dy >= 0) {
        u = dx; v = dy;
    } else if (dy > 0 && dx <= 0) {
        quadrant = 1; u = dy; v = -dx;
    } else if (dx < 0 && dy <= 0) {
        quadrant = 2; u = -dx; v = -dy;
    } else {
        quadrant = 3; u = -dy; v = dx;
    }
    const auto sum_squared = (u + v) * (u + v);
    std::size_t sector = 0;
    if (v < u) sector = sum_squared < 2 * u * u ? 0U : 1U;
    else if (v == u) sector = 2;
    else sector = sum_squared > 2 * v * v ? 2U : 3U;
    return quadrant * 4 + sector;
}

template <class Visit>
void visit_ring(Footprint footprint, int radius, int width, int height, Visit visit) {
    const int left = footprint.x - radius;
    const int right = footprint.x + footprint.side - 1 + radius;
    const int top = footprint.y - radius;
    const int bottom = footprint.y + footprint.side - 1 + radius;
    for (int y = std::max(top, 0); y <= std::min(bottom, height - 1); ++y) {
        if (y == top || y == bottom) {
            for (int x = std::max(left, 0); x <= std::min(right, width - 1); ++x) visit(x, y);
        } else {
            if (left >= 0 && left < width) visit(left, y);
            if (right >= 0 && right < width) visit(right, y);
        }
    }
}

} // namespace

SpreadSpec spread_spec(const ModelData& model, ModelBuilding building, Difficulty difficulty) {
    return {building_value(model, building, BuildingField::Desirability, difficulty),
            building_value(model, building, BuildingField::DesirabilityStep, difficulty),
            building_value(model, building, BuildingField::DesirabilityStepSize, difficulty),
            building_value(model, building, BuildingField::DesirabilityRange, difficulty)};
}

DesirabilityMap::DesirabilityMap(int width, int height)
    : width_(width), height_(height), values_(cell_count(width, height), 0),
      walls_(values_.size(), 0) {}

std::size_t DesirabilityMap::index(int x, int y) const {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) {
        throw std::out_of_range("desirability cell outside map");
    }
    return static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) +
           static_cast<std::size_t>(x);
}

void DesirabilityMap::validate_footprint(Footprint footprint) const {
    if (footprint.side < 1 || footprint.side > desirability_max_footprint) {
        throw std::invalid_argument("desirability footprint side must be in 1..6");
    }
    // Subtraction avoids overflow even for caller-supplied INT_MIN/MAX origins.
    if (footprint.x < 0 || footprint.y < 0 || footprint.side > width_ ||
        footprint.side > height_ || footprint.x > width_ - footprint.side ||
        footprint.y > height_ - footprint.side) {
        throw std::out_of_range("desirability footprint must be fully on-map");
    }
}

void DesirabilityMap::clear() noexcept { std::fill(values_.begin(), values_.end(), 0); }
void DesirabilityMap::clear_walls() noexcept { std::fill(walls_.begin(), walls_.end(), 0); }
void DesirabilityMap::set_wall(int x, int y, bool value) { walls_[index(x, y)] = value ? 1 : 0; }
bool DesirabilityMap::wall(int x, int y) const { return walls_[index(x, y)] != 0; }
int DesirabilityMap::at(int x, int y) const { return values_[index(x, y)]; }

void DesirabilityMap::apply_source(const DesirabilitySource& source, WallBlocking blocking) {
    apply_sources(std::span<const DesirabilitySource>(&source, 1), blocking);
}

void DesirabilityMap::apply_sources(std::span<const DesirabilitySource> sources,
                                  WallBlocking blocking) {
    if (blocking != WallBlocking::None && blocking != WallBlocking::ExperimentalUniform16) {
        throw std::invalid_argument("unknown desirability wall mode");
    }
    if (sources.size() > desirability_max_sources) {
        throw std::length_error("desirability source batch exceeds 4096");
    }
    std::vector<PreparedSource> prepared;
    prepared.reserve(sources.size());
    for (const auto& source : sources) {
        validate_footprint(source.footprint);
        prepared.push_back(prepare(source));
    }
    // All allocations, input checks and arithmetic preflight precede mutation.
    for (const auto& source : prepared) {
        const bool use_walls = blocking == WallBlocking::ExperimentalUniform16 && source.negative;
        std::array<int, 16> blocked_at;
        blocked_at.fill(desirability_max_range + 1);
        for (int radius = 1; radius <= source.range; ++radius) {
            if (use_walls) {
                visit_ring(source.footprint, radius, width_, height_, [&](int x, int y) {
                    if (walls_[index(x, y)] != 0) {
                        auto& distance = blocked_at[experimental_sector(source.footprint, x, y)];
                        distance = std::min(distance, radius);
                    }
                });
            }
            const auto amount = source.rings[static_cast<std::size_t>(radius - 1)];
            visit_ring(source.footprint, radius, width_, height_, [&](int x, int y) {
                const auto cell = index(x, y);
                if (use_walls && (walls_[cell] != 0 ||
                    radius > blocked_at[experimental_sector(source.footprint, x, y)])) return;
                const auto sum = static_cast<std::int64_t>(values_[cell]) + amount;
                values_[cell] = static_cast<std::int8_t>(std::clamp(sum, std::int64_t{-100},
                                                                      std::int64_t{100}));
            });
        }
    }
}

std::int32_t DesirabilityMap::building_value(Footprint footprint, std::int32_t bonus) const {
    validate_footprint(footprint);
    int value = -100;
    for (int y = footprint.y; y < footprint.y + footprint.side; ++y) {
        for (int x = footprint.x; x < footprint.x + footprint.side; ++x) {
            value = std::max(value, at(x, y));
        }
    }
    const auto result = static_cast<std::int64_t>(value) + bonus;
    if (result < std::numeric_limits<std::int32_t>::min() ||
        result > std::numeric_limits<std::int32_t>::max()) {
        throw std::overflow_error("desirability footprint bonus outside int32 domain");
    }
    return static_cast<std::int32_t>(result);
}

} // namespace openemperor::original
