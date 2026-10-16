#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <utility>
#include <vector>

// Isolated diagnostic model. These cells are not World cells, MapPermissions,
// RampLinks, saved state, or a claim about recovered original actor behavior.
namespace openemperor::original_ramp {

struct Cell {
    int x = 0;
    int y = 0;
    bool operator==(const Cell&) const = default;
};
struct Vec2 { double x = 0; double y = 0; };
struct Vec3 { double x = 0; double y = 0; double height = 0; };

enum class Direction { North, East, South, West };
enum class SurfaceStyle { Earth, Stairs, Cobbled };

Cell direction_delta(Direction direction);
Direction opposite(Direction direction);

struct ResourceInfo {
    std::uint32_t resource_key = 0;
    std::uint16_t physical_id = 0;
};
// Camera 0, independently pinned selector rows 67..70. Style is a diagnostic
// choice; it does not infer the original road helper/global's unknown state.
ResourceInfo resource_info(SurfaceStyle style, Direction higher);

struct CellState {
    int low_height = 0;
    std::optional<Direction> higher;
    SurfaceStyle style = SurfaceStyle::Earth;
    bool road = false;
    bool operator==(const CellState&) const = default;
};

// Coordinates name cell centers. The square of cell (x,y) is half-open
// [x-.5,x+.5) x [y-.5,y+.5). A slope owns one complete source cell, with its
// low edge at low_height and its opposite high edge at low_height+1. This
// continuous planar surface is a selectable prototype hypothesis, independent
// of the unchanged, unstretched original bitmap drawn for the source cell.
class CellTerrain {
public:
    static constexpr int max_dimension = 64;
    CellTerrain(int width, int height, int default_height = 0);

    int width() const { return width_; }
    int height() const { return height_; }
    bool contains(Cell cell) const;
    const CellState& at(Cell cell) const;
    std::span<const CellState> cells() const { return cells_; }

    // All failed edits are atomic. Heights retain the original signed-byte
    // range. A plateau edit cannot invalidate any existing selector pattern.
    // Rows 67..70 require high-facing diagonals within abs(delta)<=1; the
    // opposite cardinal is a wildcard there. This diagnostic model deliberately
    // requires that opposite cardinal equal low_height for a closed low join.
    bool set_plateau(Cell cell, int height);
    bool set_road(Cell cell, bool road = true);
    bool can_place_ramp(Cell cell, Direction higher) const;
    bool place_ramp(Cell cell, Direction higher, SurfaceStyle style);
    bool clear_ramp(Cell cell);

    std::optional<Cell> cell_at(Vec2 position) const;
    std::optional<double> surface_height(Vec2 position) const;
    // Explicit ownership permits either side's exact shared-edge evaluation.
    std::optional<double> surface_height_in_cell(Cell cell, Vec2 position) const;
    double center_height(Cell cell) const;
    // Original sprites retain their lateral banks. Traversal through a ramp
    // is conservatively limited to its central low/high axis; equal lateral
    // surface heights are diagnostic geometry, never a lateral permission.
    bool edge_connected(Cell from, Cell to) const;
    // Bounded prototype-only BFS. The production search belongs to World and
    // is deliberately not adapted to this incompatible diagnostic cell model.
    std::optional<std::vector<Cell>> route(Cell from, Cell to) const;

private:
    int width_;
    int height_;
    std::vector<CellState> cells_;
    std::size_t index(Cell cell) const;
    bool ramp_pattern(Cell cell, Direction higher,
                      std::optional<std::pair<Cell, int>> changed_height = {}) const;
};

Vec2 project(Vec3 position, double height_step = 40.0);
Vec2 unproject(Vec2 world, double known_height, double height_step = 40.0);

struct SurfacePick { Cell cell; Vec3 position; };
// Intersects source-cell planes and chooses the frontmost visible surface.
// With 40/20 ground projection and height_step=40, higher East/South planes
// collapse to lines: they have no unique geometric inverse and are excluded.
// A renderer may separately expose bitmap-alpha picking for those sprites.
std::optional<SurfacePick> pick_surface(const CellTerrain& terrain, Vec2 world,
                                      double height_step = 40.0);

class Walker {
public:
    bool reset(const CellTerrain& terrain, Cell cell);
    bool target(const CellTerrain& terrain, Cell destination);
    void update(const CellTerrain& terrain, double seconds);
    Vec3 position(const CellTerrain& terrain) const;
    bool moving() const { return path_vertex_ + 1 < path_.size(); }
    Cell current_cell() const;
    double edge_progress() const { return edge_progress_; }
    std::span<const Cell> path() const { return path_; }
    void set_speed(double cells_per_second);
    double speed() const { return speed_; }

private:
    std::vector<Cell> path_;
    std::size_t path_vertex_ = 0;
    double edge_progress_ = 0;
    double speed_ = 1.0;
};

} // namespace openemperor::original_ramp
