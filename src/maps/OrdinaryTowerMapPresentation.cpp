#include "maps/OrdinaryTowerMapPresentation.h"

#include <memory>
#include <set>

namespace openemperor::maps {
namespace {
constexpr std::size_t cell_count = stored_grid_width * stored_grid_height;
std::size_t storage(GridCell cell) {
    return std::size_t(cell.y) * stored_grid_width + cell.x;
}

// Compare source reservations only. A foreign zero-side record contributes its
// exact origin conservatively; this does not admit an occupancy class, create a
// policy exception or reproduce its original geometry. Gameplay still uses the
// unchanged, separately validated complete MapPermissions producer.
bool foreign_reservation_contains(const OriginalEntityRecord& source,
    GridCell cell, const MapGeometry& map) {
    if (!source.active() || source.local_x < 0 || source.local_y < 0) return false;
    const auto x = unsigned(source.local_x) + map.border;
    const auto y = unsigned(source.local_y) + map.border;
    unsigned width = source.footprint_side, height = source.footprint_side;
    if (source.entity_class == OriginalEntityClass::GateHouse && source.type == 130 &&
        source.gate_house && (source.gate_house->layout == 0 || source.gate_house->layout == 1)) {
        width = source.gate_house->layout == 0 ? 5U : 3U;
        height = source.gate_house->layout == 0 ? 3U : 5U;
    } else if (!width) width = height = 1;
    return cell.x >= x && cell.y >= y && cell.x - x < width && cell.y - y < height;
}

bool tower_schemas_supported(const OriginalEntityRecord& source) {
    const auto& p = source.provenance;
    return (p.base_schema == 3 || p.base_schema == 4 || p.base_schema == 5) &&
        p.wrapper_schema == 1 && p.extended_schema == 1 &&
        p.logical_extended_offset && p.logical_class_wrapper_offset &&
        p.class_wrapper_schema == 0;
}
} // namespace

OrdinaryTowerMapPresentation prepare_ordinary_tower_presentation(
    const OriginalMapEntities& entities, std::span<const std::uint32_t> terrain,
    std::span<const std::uint32_t> objects, std::span<const std::uint8_t> heights) {
    OrdinaryTowerMapPresentation out;
    out.manager_offset = entities.logical_offset;
    out.manager_bytes = entities.byte_length;
    out.manager_records = entities.records.size();
    out.tower_by_storage.resize(cell_count);
    const MapGeometry map{entities.declared_map_size};
    if (!map.supported || entities.manager_schema != 1 ||
        entities.records.size() > maximum_original_entity_records ||
        terrain.size() != cell_count || objects.size() != cell_count || heights.size() != cell_count) {
        out.error = "ordinary cTower requires a complete supported original manager and terrain/objects/heights";
        return out;
    }
    // The parser already checks these identities. Keep the same atomic boundary
    // for callers supplying an authored/in-memory validated-manager candidate.
    std::set<std::uint32_t> ids, indices;
    for (const auto& source : entities.records) {
        if (!source.active()) continue;
        if (!source.original_id || source.original_id->value == 0 ||
            source.original_id->value > maximum_original_entity_records ||
            source.serialized_original_id != std::int32_t(source.original_id->value) ||
            source.manager_index >= maximum_original_entity_records ||
            !ids.insert(source.original_id->value).second || !indices.insert(source.manager_index).second) {
            out.error = "ordinary cTower complete manager has invalid or duplicate active original identity";
            return out;
        }
    }
    for (const auto& source : entities.records) {
        if (!source.active() || source.entity_class != OriginalEntityClass::Tower) continue;
        OrdinaryTowerPresentation tower;
        tower.source = source;
        if (source.local_x >= 0 && source.local_y >= 0 &&
            unsigned(source.local_x) < map.declared_size && unsigned(source.local_y) < map.declared_size) {
            const GridCell origin{unsigned(source.local_x) + map.border, unsigned(source.local_y) + map.border};
            if (map.contains(origin)) {
                tower.origin = origin;
                const auto [entry,unique] = out.tower_source_at_origin.emplace(storage(origin),out.towers.size());
                if (!unique) entry->second.reset();
            }
        }
        if (!tower_schemas_supported(source))
            tower.fallback = "unsupported original cTower base/parent/non-house/class schema; historical preview retained";
        else if (source.type != 131 || source.status != 3 || source.subindex != 0 || source.footprint_side != 2 ||
            source.gate_house || source.monument)
            tower.fallback = "unsupported original cTower type/status/subindex/footprint state; historical preview retained";
        else if (source.local_x < 0 || source.local_y < 0 ||
            unsigned(source.local_x) >= map.declared_size || unsigned(source.local_y) >= map.declared_size)
            tower.fallback = "invalid original cTower local origin; historical preview retained";
        else {
            tower.origin = {unsigned(source.local_x) + map.border, unsigned(source.local_y) + map.border};
            tower.width = tower.height = 2;
            if (source.serialized_cell_reference != std::int32_t(storage(tower.origin)))
                tower.fallback = "original cTower origin/serialized reference mismatch; historical preview retained";
            std::vector<GridCell> members;
            for (unsigned dy = 0; dy < 2 && tower.fallback.empty(); ++dy)
                for (unsigned dx = 0; dx < 2 && tower.fallback.empty(); ++dx) {
                    const GridCell cell{tower.origin.x + dx, tower.origin.y + dy};
                    if (!map.contains(cell)) {
                        tower.fallback = "original cTower complete footprint outside map; historical preview retained";
                        break;
                    }
                    const auto at = storage(cell);
                    // Conservative raw admission measured independently on the
                    // supported Kaifeng/Zhengzhou states. Object word 4 remains
                    // a raw whitelist value, without an invented flag meaning.
                    if (terrain[at] != 0x8088U || (objects[at] != 0 && objects[at] != 4)) {
                        tower.fallback = "original cTower complete raw terrain/object footprint unsupported; historical preview retained";
                        break;
                    }
                    for (const auto& other : entities.records) {
                        if (other.manager_index == source.manager_index) continue;
                        if (foreign_reservation_contains(other, cell, map)) {
                            tower.fallback = "original cTower footprint conflicts with another original source reservation; historical preview retained";
                            break;
                        }
                    }
                    if (tower.fallback.empty()) members.push_back(cell);
                }
            if (tower.fallback.empty() && members.size() == 4) {
                LandscapeInstanceSpec body;
                body.selection = {LandscapeFamily::OrdinaryTower, SelectorEvidence::Verified,
                    "52f030 / 4b11f0 / 5da420 / 4b72b0",
                    "EXE-observed cTower static body key; bounded measured saved state, without saved graphic authority",
                    {ordinary_tower_resource_key}, 0, {}};
                body.origin = tower.origin;
                body.side = 2;
                body.draw_cell = {tower.origin.x, tower.origin.y + 1};
                body.owned_cells = std::move(members);
                body.explicit_height = LandscapeInstanceHeight{
                    LandscapeInstanceHeightSource::SerializedCellHeight, body.draw_cell};
                const GridCell front{tower.origin.x + 1, tower.origin.y + 1};
                body.explicit_anchor = LandscapeInstanceAnchor{front, 79, 20};
                body.depth_cell = front;
                body.original_entity_index = out.towers.size();
                body.placement_evidence = "EXE-OBSERVED: 4b72b0 camera-zero 2x2 marker and signed cell height x40; OPENEMPEROR PRESENTATION CONVENTION: existing Type30 front anchor";
                body.composition_evidence = "complete static cTower body; OPENEMPEROR PRESENTATION CONVENTION: one Combined spatial body; external 4c9/4ca wall composition unresolved";
                tower.components.push_back(std::move(body));
                for (const auto cell : tower.components.front().owned_cells)
                    out.tower_by_storage[storage(cell)] = out.towers.size();
            }
        }
        out.towers.push_back(std::move(tower));
    }
    return out;
}

void read_ordinary_tower_presentation(StoredGraphicsPlan& plan,
    const EmperorContainer& container, std::size_t part) {
    if (!plan.landscape_layers_available || plan.original_ordinary_towers) return;
    auto result = std::make_shared<OrdinaryTowerMapPresentation>();
    try {
        const auto entities = read_original_map_entities(container, part);
        *result = prepare_ordinary_tower_presentation(
            entities, plan.raw_terrain, plan.raw_objects, plan.height_bytes);
    } catch (const std::exception& error) {
        // The complete parser returns nothing on failure. Never publish any
        // tower parsed before the first unsupported or malformed source.
        result->error = error.what();
    }
    plan.original_ordinary_towers = std::move(result);
}

} // namespace openemperor::maps
