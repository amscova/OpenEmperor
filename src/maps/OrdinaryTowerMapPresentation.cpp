#include "maps/OrdinaryTowerMapPresentation.h"
#include "maps/OrdinaryGateMapPresentation.h"

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

const char* tower_join_status_name(TowerJoinStatus s) {
    switch(s) {
    case TowerJoinStatus::NoJoinRequired:return "NO_JOIN_REQUIRED";
    case TowerJoinStatus::SupportedJoin:return "SUPPORTED_JOIN";
    case TowerJoinStatus::UnsupportedContext:return "UNSUPPORTED_CONTEXT";
    case TowerJoinStatus::InvalidInput:return "INVALID_INPUT";
    }
    return "INVALID_INPUT";
}
const char* tower_join_phase_name(TowerJoinPhase p) {
    return p==TowerJoinPhase::AfterBody ? "after_body_108":"after_cell_actors_10c";
}
TowerJoinSelection select_ordinary_tower_joins(const OrdinaryTowerPresentation& tower,
    const std::array<std::optional<TowerJoinNeighbor>,6>& neighbors,unsigned view,bool static_draw) {
    TowerJoinSelection out;
    const auto& source=tower.source;
    if (view!=0 || !static_draw || !tower.fallback.empty() || source.status!=3 || source.type!=131 ||
        source.entity_class!=OriginalEntityClass::Tower || source.subindex!=0 || source.footprint_side!=2 ||
        !tower_schemas_supported(source) || tower.components.size()!=1) {
        out.reason="only validated static cTower body / ordinary camera-zero draw is supported";return out;
    }
    const auto fail=[&](const char* why) {out.status=TowerJoinStatus::InvalidInput;out.reason=why;return out;};
    if (!source.original_id || !source.original_id->value ||
        source.original_id->value>maximum_original_entity_records ||
        source.serialized_original_id!=std::int32_t(source.original_id->value) ||
        source.manager_index>=maximum_original_entity_records ||
        source.serialized_cell_reference!=std::int32_t(storage(tower.origin)))
        return fail("invalid stable Tower source identity/reference");
    constexpr std::array<int,6> dx{1,-1,0,2,2,1},dy{-1,1,2,0,1,2};
    std::uint8_t present=0;
    for(unsigned n=0;n<6;++n) {
        const auto x=std::int64_t(tower.origin.x)+dx[n],y=std::int64_t(tower.origin.y)+dy[n];
        if(x<0 || y<0 || x>=stored_grid_width || y>=stored_grid_height || !neighbors[n] ||
            neighbors[n]->cell!=GridCell{unsigned(x),unsigned(y)})
            return fail("complete exact Tower neighbor cells required; no edge wrapping");
        out.neighbors[n]=*neighbors[n];const auto& cell=out.neighbors[n];
        const auto kind=cell.owner.kind;
        if(kind<TowerJoinOwnerKind::None || kind>TowerJoinOwnerKind::Unresolved ||
            (kind==TowerJoinOwnerKind::None && cell.owner.id))
            return fail("invalid typed neighbor ownership fact");
        if(kind==TowerJoinOwnerKind::Unresolved ||
            (kind!=TowerJoinOwnerKind::None &&
             (!cell.owner.id || !cell.owner.id->value || cell.owner.id->value>maximum_original_entity_records ||
              cell.owner.manager_index>=maximum_original_entity_records)) ||
            ((cell.terrain&0x8000U) && kind==TowerJoinOwnerKind::None)) {
            out.reason="Tower/Gate neighbor ownership unsupported or unproved";return out;
        }
        // 5daab0 / 5da950 / 5dac10 test 0x4000, then exact type 131/130.
        if((cell.terrain&0x4000U) || kind==TowerJoinOwnerKind::Tower || kind==TowerJoinOwnerKind::GateHouse)
            present|=std::uint8_t(1U<<n);
    }
    if(!present) {
        // Scope admission: do not add an isolated tower's cap-only decoration.
        // The original helpers also have absence branches; their caps accompany
        // a supported connection here, never establish one themselves.
        out.status=TowerJoinStatus::NoJoinRequired;out.reason="no relevant wall or validated Tower/Gate neighbor";return out;
    }
    const GridCell marker{tower.origin.x,tower.origin.y+1};
    const auto append=[&](ResourceGroupKey key,unsigned variant,int x,int y,TowerJoinPhase phase,std::uint8_t cells) {
        out.components.push_back({*source.original_id,source.manager_index,key,variant,tower.origin,marker,marker,x,y,phase,cells,
            key.value==0x4ca ? "EXE-OBSERVED 5dac10 / ordered presence pairs / 7be4d0":
            phase==TowerJoinPhase::AfterBody ? "EXE-OBSERVED 5daab0 / absent rear neighbors / 7be4a0":
            "EXE-OBSERVED 5da950 / absent forward neighbors / 7be4b0"});
    };
    constexpr std::array<int,6> px{127,1,1,114,75,39},py{-60,-57,-46,-48,-28,-24};
    for(unsigned n=0;n<2;++n) if(!(present&(1U<<n)))
        append({0x4c9},n,px[n],py[n],TowerJoinPhase::AfterBody,std::uint8_t(1U<<n));
    // Pair 0: forward indices 0,3. Pair 1: 1,2. Additive, not replacement.
    if(present&0x24U) append({0x4ca},0,28,-71,TowerJoinPhase::AfterBody,0x24);
    if(present&0x18U) append({0x4ca},1,79,-70,TowerJoinPhase::AfterBody,0x18);
    for(unsigned n=2;n<6;++n) if(!(present&(1U<<n)))
        append({0x4c9},n,px[n],py[n],TowerJoinPhase::AfterCellActors,std::uint8_t(1U<<n));
    out.status=TowerJoinStatus::SupportedJoin;
    out.reason="EXE-OBSERVED 4709b9/470e67 -> 5da8c0/5da910; complete typed neighbors, static view zero";
    return out;
}

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
    // Reconstruct only the needed runtime owner facts from complete typed
    // source claims. The serialized Objects grid is not fc3750's owner table.
    const auto gates=prepare_ordinary_gate_presentation(entities,terrain);
    constexpr std::array<int,6> dx{1,-1,0,2,2,1},dy{-1,1,2,0,1,2};
    for(auto& tower:out.towers) {
        if(!tower.fallback.empty()) continue;
        std::array<std::optional<TowerJoinNeighbor>,6> neighbors;
        for(unsigned n=0;n<6;++n) {
            const int x=int(tower.origin.x)+dx[n],y=int(tower.origin.y)+dy[n];
            if(x<0 || y<0 || x>=int(stored_grid_width) || y>=int(stored_grid_height)) continue;
            const GridCell cell{unsigned(x),unsigned(y)};
            if(!map.contains(cell)) continue;
            const auto at=storage(cell);TowerJoinNeighbor fact{cell,terrain[at],objects[at],{}};
            const OriginalEntityRecord* owner=nullptr;bool ambiguous=false;
            for(const auto& e:entities.records) if(foreign_reservation_contains(e,cell,map)) {
                if(owner) ambiguous=true;else owner=&e;
            }
            if(ambiguous) fact.owner.kind=TowerJoinOwnerKind::Unresolved;
            else if(owner) {
                fact.owner={TowerJoinOwnerKind::Other,owner->original_id,owner->manager_index};
                if(owner->entity_class==OriginalEntityClass::Tower) {
                    const auto index=out.tower_by_storage[at];
                    fact.owner.kind=index && out.towers[*index].source.manager_index==owner->manager_index ?
                        TowerJoinOwnerKind::Tower:TowerJoinOwnerKind::Unresolved;
                } else if(owner->entity_class==OriginalEntityClass::GateHouse) {
                    const auto index=gates.gate_by_storage[at];
                    fact.owner.kind=index && gates.gates[*index].source.manager_index==owner->manager_index ?
                        TowerJoinOwnerKind::GateHouse:TowerJoinOwnerKind::Unresolved;
                }
            }
            neighbors[n]=fact;
        }
        tower.joins=select_ordinary_tower_joins(tower,neighbors);
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
