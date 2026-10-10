#include "simulation/MapPermissions.h"

#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <locale>
#include <stdexcept>

namespace openemperor::simulation {
namespace {
bool storage_less(Cell a,Cell b) { return a.y!=b.y ? a.y<b.y:a.x<b.x; }
bool adjacent(Cell a,Cell b) { return std::abs(a.x-b.x)+std::abs(a.y-b.y)==1; }
}
const char* build_blocker_name(BuildBlocker b) {
    switch (b) {
    case BuildBlocker::None:return "none";
    case BuildBlocker::OutsideMap:return "outside_map";
    case BuildBlocker::UnsupportedTerrain:return "unsupported_terrain";
    case BuildBlocker::OriginalStructure:return "original_structure";
    case BuildBlocker::GateSolidPart:return "gate_solid_part";
    case BuildBlocker::GateSideEntry:return "gate_side_entry";
    case BuildBlocker::SandboxOccupied:return "sandbox_occupied";
    case BuildBlocker::InsufficientFunds:return "insufficient_funds";
    case BuildBlocker::LegacyBuildabilityRestriction:return "legacy_buildability_restriction";
    case BuildBlocker::UnsupportedHeightTransition:return "unsupported_height_transition";
    case BuildBlocker::ProtectedGatePassage:return "protected_gate_passage";
    case BuildBlocker::InvalidEdge:return "invalid_edge";
    case BuildBlocker::CounterExhausted:return "counter_exhausted";
    case BuildBlocker::ProtectedOriginalRoad:return "protected_original_road";
    case BuildBlocker::UnsupportedOriginalRoad:return "unsupported_original_road";
    }
    return "unknown";
}
const char* build_blocker_reason(BuildBlocker b) {
    switch (b) {
    case BuildBlocker::None:return "Road ready";
    case BuildBlocker::OutsideMap:return "Outside sandbox grid";
    case BuildBlocker::UnsupportedTerrain:return "Unsupported terrain";
    case BuildBlocker::OriginalStructure:return "Protected original structure";
    case BuildBlocker::GateSolidPart:return "Solid part of original gate";
    case BuildBlocker::GateSideEntry:return "Original gate permits entry only at its openings";
    case BuildBlocker::SandboxOccupied:return "Cell already occupied";
    case BuildBlocker::InsufficientFunds:return "Not enough money";
    case BuildBlocker::LegacyBuildabilityRestriction:return "Not sandbox-buildable";
    case BuildBlocker::UnsupportedHeightTransition:return "Unsupported height transition";
    case BuildBlocker::ProtectedGatePassage:return "Original gate passage cannot be removed.";
    case BuildBlocker::InvalidEdge:return "Road drag has a nonorthogonal edge";
    case BuildBlocker::CounterExhausted:return "Sandbox counter exhausted";
    case BuildBlocker::ProtectedOriginalRoad:return "Original map road cannot be removed.";
    case BuildBlocker::UnsupportedOriginalRoad:return "Unsupported original road form";
    }
    return "Unknown road blocker";
}
const char* original_road_classification_name(OriginalRoadClassification c) {
    switch (c) {
    case OriginalRoadClassification::None:return "none";
    case OriginalRoadClassification::GroundRoad:return "ground_road";
    case OriginalRoadClassification::UnsupportedTerrain:return "unsupported_road_terrain";
    case OriginalRoadClassification::WaterOrBridge:return "water_or_bridge_candidate";
    case OriginalRoadClassification::OriginalStructure:return "original_structure";
    case OriginalRoadClassification::NonzeroObjects:return "nonzero_original_objects";
    case OriginalRoadClassification::OutsideMap:return "outside_map";
    case OriginalRoadClassification::FixedGateFootprint:return "fixed_gate_footprint";
    }
    return "unknown";
}
const char* original_road_classification_reason(OriginalRoadClassification c) {
    switch (c) {
    case OriginalRoadClassification::None:return "No original road marking";
    case OriginalRoadClassification::GroundRoad:return "Validated normal ground road";
    case OriginalRoadClassification::UnsupportedTerrain:return "Original road has unsupported terrain features";
    case OriginalRoadClassification::WaterOrBridge:return "Original water/bridge road is unsupported";
    case OriginalRoadClassification::OriginalStructure:return "Original road marking is covered by protected original occupancy";
    case OriginalRoadClassification::NonzeroObjects:return "Original road has nonzero raw object features";
    case OriginalRoadClassification::OutsideMap:return "Original road marking is outside the supported map";
    case OriginalRoadClassification::FixedGateFootprint:return "Original road marking belongs to the fixed gate contract";
    }
    return "Unknown original road classification";
}
MapPermissions::MapPermissions(int width,int height,std::uint32_t version,
    std::vector<MapCellPermission> cells,std::vector<FixedGatePassage> gates)
    : width_(width),height_(height),policy_version_(version),cells_(std::move(cells)),gates_(std::move(gates)) {
    if (width<=0 || height<=0 || width>512 || height>512 ||
        cells_.size()!=static_cast<std::size_t>(width)*static_cast<std::size_t>(height))
        throw std::invalid_argument("invalid bounded map permissions grid");
    if (!supported_map_permissions_policy(version))
        throw std::invalid_argument("unsupported map permissions policy version");
    gate_index_.assign(cells_.size(),-1);corridor_index_.assign(cells_.size(),-1);
    std::sort(gates_.begin(),gates_.end(),[](const auto& a,const auto& b){return a.id<b.id;});
    for (std::size_t gi=0;gi<gates_.size();++gi) {
        auto& g=gates_[gi];
        if (!g.id.value || (gi && gates_[gi-1].id==g.id) || g.protected_footprint.size()!=15 || g.corridor.size()!=3)
            throw std::invalid_argument("invalid fixed gate identity or footprint/corridor size");
        // Bound all coordinates before subtracting extents or indexing. Even
        // malformed authored inputs must fail without signed overflow.
        if (!std::all_of(g.protected_footprint.begin(),g.protected_footprint.end(),
            [&](Cell p){return in_bounds(p);}))
            throw std::invalid_argument("fixed gate footprint outside map permissions grid");
        std::sort(g.protected_footprint.begin(),g.protected_footprint.end(),storage_less);
        const auto first=g.protected_footprint.front(),last=g.protected_footprint.back();
        const int fw=last.x-first.x+1,fh=last.y-first.y+1;
        if (!((fw==5 && fh==3) || (fw==3 && fh==5)))
            throw std::invalid_argument("unsupported fixed gate footprint");
        std::size_t pi=0;
        for (int y=first.y;y<first.y+fh;++y) for (int x=first.x;x<first.x+fw;++x) {
            const Cell p{x,y};
            if (g.protected_footprint[pi++]!=p || !in_bounds(p) || gate_index_[index(p)]!=-1 ||
                !cells_[index(p)].protected_original || cells_[index(p)].building_allowed)
                throw std::invalid_argument("incomplete or conflicting fixed gate footprint");
            gate_index_[index(p)]=static_cast<int>(gi);
        }
        if (storage_less(g.corridor.back(),g.corridor.front())) {
            std::reverse(g.corridor.begin(),g.corridor.end());
            std::swap(g.openings[0],g.openings[1]);
        }
        for (std::size_t ci=0;ci<g.corridor.size();++ci) {
            const Cell expected=fw==5 ? Cell{first.x+2,first.y+static_cast<int>(ci)}:
                Cell{first.x+static_cast<int>(ci),first.y+2};
            const auto p=g.corridor[ci];
            if (p!=expected || gate_index_[index(p)]!=static_cast<int>(gi) || !cells_[index(p)].road_allowed)
                throw std::invalid_argument("invalid fixed gate corridor");
            corridor_index_[index(p)]=static_cast<int>(ci);
        }
        const Cell d{g.corridor[1].x-g.corridor[0].x,g.corridor[1].y-g.corridor[0].y};
        if (g.openings[0]!=Cell{g.corridor.front().x-d.x,g.corridor.front().y-d.y} ||
            g.openings[1]!=Cell{g.corridor.back().x+d.x,g.corridor.back().y+d.y} ||
            !in_bounds(g.openings[0]) || !in_bounds(g.openings[1]))
            throw std::invalid_argument("invalid fixed gate openings");
        for (const auto p:g.protected_footprint)
            if (corridor_index_[index(p)]<0 && cells_[index(p)].road_allowed)
                throw std::invalid_argument("solid gate footprint permits a road");
    }
    for (const auto& g:gates_) for (const auto p:g.openings)
        if (gate_index_[index(p)]>=0)
            throw std::invalid_argument("fixed gate opening overlaps another gate");
    road_mask_.reserve(cells_.size());building_mask_.reserve(cells_.size());
    const auto maximum_blocker=version==kMapPermissionsPolicyVersion ?
        BuildBlocker::CounterExhausted:BuildBlocker::UnsupportedOriginalRoad;
    for (std::size_t i=0;i<cells_.size();++i) {
        const auto& c=cells_[i];
        if (static_cast<unsigned>(c.road_blocker)>static_cast<unsigned>(maximum_blocker) ||
            static_cast<unsigned>(c.building_blocker)>static_cast<unsigned>(maximum_blocker) ||
            static_cast<unsigned>(c.original_road_classification)>static_cast<unsigned>(OriginalRoadClassification::FixedGateFootprint) ||
            (c.protected_original && (c.building_allowed || (c.road_allowed && corridor_index_[i]<0))))
            throw std::invalid_argument("inconsistent protected map permission");
        if ((version==kMapPermissionsPolicyVersion && c.original_road) ||
            (version==kOriginalRoadsMapPermissionsPolicyVersion &&
                c.original_road!=(c.original_road_classification==OriginalRoadClassification::GroundRoad)) ||
            (c.original_road && (!c.road_allowed || c.building_allowed || c.protected_original ||
                gate_index_[i]>=0 || c.road_blocker!=BuildBlocker::None ||
                c.building_blocker!=BuildBlocker::ProtectedOriginalRoad)))
            throw std::invalid_argument("inconsistent original map road permission");
        road_mask_.push_back(static_cast<std::uint8_t>(c.road_allowed));
        building_mask_.push_back(static_cast<std::uint8_t>(c.building_allowed));
    }
}
bool MapPermissions::in_bounds(Cell p) const {return p.x>=0 && p.y>=0 && p.x<width_ && p.y<height_;}
std::size_t MapPermissions::index(Cell p) const {return static_cast<std::size_t>(p.y)*static_cast<std::size_t>(width_)+static_cast<std::size_t>(p.x);}
const MapCellPermission* MapPermissions::cell_permission(Cell p) const {return in_bounds(p) ? &cells_[index(p)]:nullptr;}
bool MapPermissions::road_allowed(Cell p) const {const auto* c=cell_permission(p);return c && c->road_allowed;}
bool MapPermissions::building_allowed(Cell p) const {const auto* c=cell_permission(p);return c && c->building_allowed;}
bool MapPermissions::protected_original(Cell p) const {const auto* c=cell_permission(p);return c && c->protected_original;}
bool MapPermissions::original_road(Cell p) const {const auto* c=cell_permission(p);return c && c->original_road;}
OriginalRoadClassification MapPermissions::original_road_classification(Cell p) const {
    const auto* c=cell_permission(p);return c ? c->original_road_classification:OriginalRoadClassification::OutsideMap;
}
std::int8_t MapPermissions::cell_height(Cell p) const {const auto* c=cell_permission(p);return c ? c->height:0;}
bool MapPermissions::fixed_passage(Cell p) const {return in_bounds(p) && corridor_index_[index(p)]>=0;}
const FixedGatePassage* MapPermissions::fixed_gate(Cell p) const {
    if (!in_bounds(p) || gate_index_[index(p)]<0)return nullptr;
    return &gates_[static_cast<std::size_t>(gate_index_[index(p)])];
}
std::optional<FixedGateId> MapPermissions::fixed_gate_at(Cell p) const {
    const auto* gate=fixed_gate(p);
    return gate ? std::optional<FixedGateId>{gate->id}:std::nullopt;
}
BuildBlocker MapPermissions::road_blocker(Cell p) const {
    const auto* c=cell_permission(p);
    if (!c)return BuildBlocker::OutsideMap;
    if (c->road_allowed)return BuildBlocker::None;
    return c->road_blocker==BuildBlocker::None ? BuildBlocker::UnsupportedTerrain:c->road_blocker;
}
BuildBlocker MapPermissions::building_blocker(Cell p) const {
    const auto* c=cell_permission(p);
    if (!c)return BuildBlocker::OutsideMap;
    if (c->building_allowed)return BuildBlocker::None;
    return c->building_blocker==BuildBlocker::None ? BuildBlocker::LegacyBuildabilityRestriction:c->building_blocker;
}
BuildBlocker MapPermissions::transport_edge_blocker(Cell from,Cell to) const {
    if (!in_bounds(from) || !in_bounds(to))return BuildBlocker::OutsideMap;
    if (!adjacent(from,to))return BuildBlocker::InvalidEdge;
    const bool fp=fixed_passage(from),tp=fixed_passage(to);
    if (fp || tp) {
        const auto passage=fp ? from:to,other=fp ? to:from;
        const auto gi=gate_index_[index(passage)];
        const auto ci=corridor_index_[index(passage)];
        const auto& g=gates_[static_cast<std::size_t>(gi)];
        const bool internal=fixed_passage(other) && gate_index_[index(other)]==gi &&
            std::abs(corridor_index_[index(other)]-ci)==1;
        const bool opening=(!fixed_gate_at(other)) &&
            ((ci==0 && other==g.openings[0]) || (ci==2 && other==g.openings[1]));
        if (!internal && !opening)return BuildBlocker::GateSideEntry;
    } else if (protected_original(from) || protected_original(to)) {
        return fixed_gate_at(from) || fixed_gate_at(to) ? BuildBlocker::GateSolidPart:BuildBlocker::OriginalStructure;
    }
    if (cell_height(from)!=cell_height(to))return BuildBlocker::UnsupportedHeightTransition;
    return BuildBlocker::None;
}
std::string MapPermissions::canonical_state() const {
    std::ostringstream s;s.imbue(std::locale::classic());s<<"map-permissions:"<<policy_version_<<':'<<width_<<':'<<height_<<'\n';
    if (policy_version_==kOriginalRoadsMapPermissionsPolicyVersion)s<<"original-roads-format:1\n";
    for (const auto& c:cells_) {
        s<<c.road_allowed<<','<<c.building_allowed<<','<<c.protected_original<<','
        <<static_cast<int>(c.height)<<','<<static_cast<unsigned>(c.road_blocker)<<','
        <<static_cast<unsigned>(c.building_blocker);
        if (policy_version_==kOriginalRoadsMapPermissionsPolicyVersion)
            s<<','<<c.original_road<<','<<static_cast<unsigned>(c.original_road_classification);
        s<<';';
    }
    for (const auto& g:gates_) {
        s<<"\ngate:"<<g.id.value<<':';
        for (const auto p:g.protected_footprint)s<<p.x<<','<<p.y<<';';
        s<<"corridor:";for (const auto p:g.corridor)s<<p.x<<','<<p.y<<';';
        s<<"openings:";for (const auto p:g.openings)s<<p.x<<','<<p.y<<';';
    }
    return s.str();
}
std::shared_ptr<const MapPermissions> authored_simple_permissions(int width,int height,const std::vector<std::uint8_t>& mask,
    std::uint32_t policy_version) {
    std::vector<MapCellPermission> cells;cells.reserve(mask.size());
    for (const auto m:mask)cells.push_back({m!=0,m!=0,false,0,
        m ? BuildBlocker::None:BuildBlocker::LegacyBuildabilityRestriction,
        m ? BuildBlocker::None:BuildBlocker::LegacyBuildabilityRestriction});
    return std::make_shared<const MapPermissions>(width,height,policy_version,std::move(cells),std::vector<FixedGatePassage>{});
}
} // namespace openemperor::simulation
