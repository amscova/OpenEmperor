#pragma once

#include "simulation/original/ModelSchema.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace openemperor::original {

inline constexpr int desirability_max_dimension = 228;
inline constexpr int desirability_max_footprint = 6;
inline constexpr int desirability_max_range = 10;
inline constexpr std::size_t desirability_max_sources = 4096;

struct Footprint {
    int x = 0;
    int y = 0;
    int side = 1;
};

struct SpreadSpec {
    std::int32_t value = 0;
    std::int32_t step = 1;
    std::int32_t step_size = 0;
    std::int32_t range = 0;
};

struct DesirabilitySource {
    Footprint footprint;
    SpreadSpec spread;
};

// Footprints are supplied independently: model SZE is a value delta, not size.
[[nodiscard]] SpreadSpec spread_spec(const ModelData& model, ModelBuilding building,
                                     Difficulty difficulty);

enum class WallBlocking : std::uint8_t {
    None = 0,
    // Authored, exact integer classification of uniform angular sectors.
    // Original wall coordinate units, sectors and edge path remain unresolved.
    ExperimentalUniform16 = 1
};

// Pure, bounded preparation tool. It has no World, SDL, save or frame hooks.
// Sources and house queries must be entirely on-map squares of side 1..6.
// Rings are clipped to the map. Supplied source order is authoritative for
// per-add saturation. A batch is validated in full before any cell changes.
class DesirabilityMap {
public:
    DesirabilityMap(int width, int height);

    [[nodiscard]] int width() const noexcept { return width_; }
    [[nodiscard]] int height() const noexcept { return height_; }
    void clear() noexcept; // Values only; explicitly prepared walls remain.
    void clear_walls() noexcept;
    void set_wall(int x, int y, bool wall = true);
    [[nodiscard]] bool wall(int x, int y) const;
    [[nodiscard]] int at(int x, int y) const;

    void apply_source(const DesirabilitySource& source,
                      WallBlocking blocking = WallBlocking::None);
    void apply_sources(std::span<const DesirabilitySource> sources,
                       WallBlocking blocking = WallBlocking::None);

    // Maximum over the whole footprint, then explicit bonus without cell clamp.
    // The original +10 trigger is unknown; this API does not infer it.
    [[nodiscard]] std::int32_t building_value(Footprint footprint,
                                              std::int32_t bonus = 0) const;

private:
    [[nodiscard]] std::size_t index(int x, int y) const;
    void validate_footprint(Footprint footprint) const;
    int width_;
    int height_;
    std::vector<std::int8_t> values_;
    std::vector<std::uint8_t> walls_;
};

} // namespace openemperor::original
