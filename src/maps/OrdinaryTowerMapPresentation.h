#pragma once

#include "maps/LandscapeInstances.h"
#include "maps/OriginalMapEntities.h"
#include "maps/StoredGraphicsPlan.h"

#include <map>

namespace openemperor::maps {

inline constexpr std::uint32_t ordinary_tower_resource_key = 0x44cU;

enum class TowerJoinStatus { NoJoinRequired, SupportedJoin, UnsupportedContext, InvalidInput };
enum class TowerJoinPhase { AfterBody, AfterCellActors };
enum class TowerJoinOwnerKind { None, Other, Tower, GateHouse, Unresolved };
struct TowerJoinOwner {
    TowerJoinOwnerKind kind = TowerJoinOwnerKind::None;
    std::optional<OriginalEntityId> id;
    std::size_t manager_index = 0;
};
struct TowerJoinNeighbor {
    GridCell cell{};
    std::uint32_t terrain = 0, objects = 0; // Objects is a raw diagnostic, never an ID.
    TowerJoinOwner owner;
};
struct TowerJoinComponent {
    OriginalEntityId owner{};
    std::size_t manager_index = 0;
    ResourceGroupKey group{};
    unsigned variant = 0;
    GridCell source_cell{}, height_cell{}, anchor_cell{};
    int offset_x = 0, offset_y = 0; // Relative to the original overlay caller.
    TowerJoinPhase phase = TowerJoinPhase::AfterBody;
    std::uint8_t condition_cells = 0;
    std::string evidence;
};
struct TowerJoinSelection {
    TowerJoinStatus status = TowerJoinStatus::UnsupportedContext;
    std::array<TowerJoinNeighbor,6> neighbors{};
    std::vector<TowerJoinComponent> components;
    std::string reason;
};
const char* tower_join_status_name(TowerJoinStatus status);
const char* tower_join_phase_name(TowerJoinPhase phase);

struct OrdinaryTowerPresentation {
    OriginalEntityRecord source;
    GridCell origin{};
    unsigned width = 0, height = 0;
    std::vector<LandscapeInstanceSpec> components;
    std::string fallback;
    TowerJoinSelection joins;
};

// Neighbor order: (+1,-1),(-1,+1),(0,+2),(+2,0),(+2,+1),(+1,+2).
// Pure static camera-zero, ordinary draw (not placement preview) selection.
TowerJoinSelection select_ordinary_tower_joins(const OrdinaryTowerPresentation& tower,
    const std::array<std::optional<TowerJoinNeighbor>,6>& neighbors,
    unsigned view = 0, bool static_draw = true);

// One complete static cTower body, prepared from a fully read original manager.
// External fragments are optional render attachments, without ground claims.
// These claims are renderer ownership, never simulation occupancy or roads.
struct OrdinaryTowerMapPresentation {
    std::uint64_t manager_offset = 0, manager_bytes = 0;
    std::size_t manager_records = 0;
    std::vector<OrdinaryTowerPresentation> towers;
    std::vector<std::optional<std::size_t>> tower_by_storage;
    // Bounded source diagnostics only, including refused static-body states.
    // Null means multiple original sources at this origin, never ownership.
    std::map<std::size_t,std::optional<std::size_t>> tower_source_at_origin;
    std::string error;
};

OrdinaryTowerMapPresentation prepare_ordinary_tower_presentation(
    const OriginalMapEntities& entities,
    std::span<const std::uint32_t> terrain,
    std::span<const std::uint32_t> objects,
    std::span<const std::uint8_t> heights);

void read_ordinary_tower_presentation(StoredGraphicsPlan& plan,
    const EmperorContainer& container, std::size_t part);

} // namespace openemperor::maps
