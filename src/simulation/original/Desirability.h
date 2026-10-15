#pragma once
// Original-style desirability map: per-tile signed values built from sources
// that spread outward in square rings. Pure data structure, no World coupling.
#include <cstdint>
#include <vector>

namespace openemperor::original {

inline constexpr int desirability_max_range = 10;
inline constexpr int desirability_limit = 100;

struct SpreadSpec {
    int value = 0;      // initial value (ring 1)
    int step = 1;       // rings between value changes; <= 0 means never change
    int step_size = 0;  // added to the value after each step
    int range = 0;      // number of rings, capped at desirability_max_range
};

// How residential walls affect negative sources.
enum class WallBlocking : std::uint8_t {
    None,      // walls are ignored
    Intended,  // 16 direction sectors measured from the true footprint centre
    // Exact (reproduce the original's unit mix-up) is [OPEN]; see the spec.
};

class DesirabilityMap {
public:
    DesirabilityMap(int width, int height);
    int width() const { return width_; }
    int height() const { return height_; }

    void clear();
    // Marks a tile as carrying a residential wall/barrier/gate.
    void set_wall(int x, int y, bool wall = true);
    bool wall(int x, int y) const;

    // Spread a source from an n x n footprint whose top-left tile is (x, y).
    void apply_source(int x, int y, int size, const SpreadSpec& spec,
                      WallBlocking blocking = WallBlocking::Intended);

    int at(int x, int y) const;  // 0 outside the map

    // Maximum over an n x n footprint, plus an optional flat bonus (no clamp).
    int building_value(int x, int y, int size, int bonus = 0) const;

private:
    bool inside(int x, int y) const { return x >= 0 && y >= 0 && x < width_ && y < height_; }
    void add(int x, int y, int value);

    int width_, height_;
    std::vector<std::int8_t> values_;
    std::vector<std::uint8_t> walls_;
};

}  // namespace openemperor::original
