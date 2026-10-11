#include "maps/OrdinaryTowerMapPresentation.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace {
namespace maps = openemperor::maps;
constexpr std::size_t count = 228U * 228U;
constexpr maps::GridCell origin{100, 110};
constexpr std::size_t raw(maps::GridCell cell) { return std::size_t(cell.y) * 228 + cell.x; }
void check(bool value, const char* message) { if (!value) throw std::runtime_error(message); }

maps::OriginalEntityRecord source(unsigned schema = 5, maps::GridCell start = origin,
    unsigned manager = 11, unsigned id = 7) {
    maps::OriginalEntityRecord e;
    e.manager_index = manager; e.entity_class = maps::OriginalEntityClass::Tower;
    e.status = 3; e.footprint_side = 2; e.type = 131; e.subindex = 0;
    e.local_x = std::int16_t(start.x - 29); e.local_y = std::int16_t(start.y - 29);
    e.serialized_cell_reference = std::int32_t(raw(start));
    e.serialized_original_id = std::int32_t(id); e.original_id = maps::OriginalEntityId{id};
    e.provenance.logical_record_offset = 9984; e.provenance.logical_base_offset = 10000;
    e.provenance.base_schema = std::uint16_t(schema); e.provenance.wrapper_schema = 1;
    e.provenance.logical_extended_offset = 10184; e.provenance.extended_schema = 1;
    e.provenance.logical_class_wrapper_offset = 10312; e.provenance.class_wrapper_schema = 0;
    return e;
}
struct Input {
    maps::OriginalMapEntities entities;
    std::vector<std::uint32_t> terrain = std::vector<std::uint32_t>(count, 0x80);
    std::vector<std::uint32_t> objects = std::vector<std::uint32_t>(count, 0);
    std::vector<std::uint8_t> heights = std::vector<std::uint8_t>(count, 0);
    explicit Input(unsigned schema = 5) {
        entities.manager_schema = 1; entities.declared_map_size = 170;
        entities.records.push_back(source(schema)); mark(origin);
    }
    void mark(maps::GridCell start) {
        for (unsigned dy = 0; dy < 2; ++dy) for (unsigned dx = 0; dx < 2; ++dx)
            terrain[raw({start.x + dx, start.y + dy})] = 0x8088;
    }
    maps::OrdinaryTowerMapPresentation prepare() const {
        return maps::prepare_ordinary_tower_presentation(entities, terrain, objects, heights);
    }
};
bool no_claims(const maps::OrdinaryTowerMapPresentation& p) {
    return std::none_of(p.tower_by_storage.begin(), p.tower_by_storage.end(), [](const auto& c) { return bool(c); });
}
void positive() {
    for (const unsigned schema : {3U, 4U, 5U}) {
        const Input input(schema); const auto prepared = input.prepare();
        check(prepared.error.empty() && prepared.manager_records == 1 && prepared.towers.size() == 1,
            "supported complete manager admits one typed static cTower");
        const auto& t = prepared.towers.front();
        check(t.source.original_id == maps::OriginalEntityId{7} && t.source.type == 131 &&
            t.source.footprint_side == 2 && t.origin == origin && t.width == 2 && t.height == 2 &&
            t.fallback.empty() && t.components.size() == 1, "one complete tower body owns its exact square");
        const auto& s = t.components.front();
        const std::vector<maps::GridCell> cells{{100,110},{101,110},{100,111},{101,111}};
        check(s.selection.family == maps::LandscapeFamily::OrdinaryTower &&
            s.selection.evidence == maps::SelectorEvidence::Verified && s.selection.group.value == 0x44c &&
            s.selection.variant == 0 && s.origin == origin && s.side == 2 && s.draw_cell == maps::GridCell{100,111} &&
            s.owned_cells == cells && s.original_entity_index == 0 && s.explicit_height &&
            s.explicit_height->cell == maps::GridCell{100,111} && s.explicit_anchor &&
            s.explicit_anchor->ground_cell == maps::GridCell{101,111} && s.explicit_anchor->x == 79 &&
            s.explicit_anchor->y_from_image_bottom == 20 && s.depth_cell == maps::GridCell{101,111},
            "literal camera-zero writer/body expectations retain bounded height and presentation anchor");
        check(std::string(s.placement_evidence).find("PRESENTATION CONVENTION") != std::string::npos &&
            std::string(s.composition_evidence).find("unresolved") != std::string::npos,
            "body identity is distinct from anchor and exterior wall-composition evidence");
        for (const auto cell : cells) check(prepared.tower_by_storage[raw(cell)] == 0,
            "all four source cells point to one tower, not four independent images");
        check(prepared.tower_source_at_origin.size() == 1 && prepared.tower_source_at_origin.at(raw(origin)) == 0,
            "one bounded source-origin diagnostic remains separate from four renderer claims");
        auto changed = input; changed.heights.assign(count, 255); changed.objects[raw({101,111})] = 4;
        const auto same = changed.prepare();
        check(same.towers.front().components.front().selection == s.selection &&
            same.towers.front().components.front().owned_cells == s.owned_cells &&
            same.towers.front().components.front().draw_cell == s.draw_cell,
            "height and either supported raw object word cannot select a different body");
    }
}
void refusals() {
    const Input good;
    for (unsigned failure = 0; failure < 22; ++failure) {
        auto input = good; auto& e = input.entities.records.front();
        switch (failure) {
        case 0: e.type = 130; break;
        case 1: e.status = 1; break;
        case 2: e.subindex = 1; break;
        case 3: e.footprint_side = 1; break;
        case 4: e.provenance.base_schema = 2; break;
        case 5: e.provenance.wrapper_schema = 0; break;
        case 6: e.provenance.extended_schema = 2; break;
        case 7: e.provenance.class_wrapper_schema = 1; break;
        case 8: e.provenance.logical_extended_offset.reset(); break;
        case 9: e.provenance.logical_class_wrapper_offset.reset(); break;
        case 10: ++e.serialized_cell_reference; break;
        case 11: e.local_x = -1; break;
        case 12: e.local_y = 170; break;
        case 13: e.local_x = 0; e.local_y = 0; e.serialized_cell_reference = 29*228+29; break;
        case 14: input.terrain[raw({101,111})] = 0x8080; break;
        case 15: input.terrain[raw({101,111})] |= 0x4000; break;
        case 16: input.terrain[raw({101,111})] |= 0x80000; break;
        case 17: input.objects[raw({101,111})] = 1; break;
        case 18: e.gate_house = maps::OriginalGateHouseState{0}; break;
        case 19: e.monument = maps::OriginalMonumentState{}; break;
        case 20: e.serialized_original_id = 8; break;
        case 21: e.original_id.reset(); break;
        }
        const auto p = input.prepare();
        check(no_claims(p) && std::all_of(p.towers.begin(), p.towers.end(), [](const auto& t) {
            return t.components.empty() && !t.fallback.empty(); }),
            "invalid identity/state/schema/reference/raw member never publishes partial body claims");
        check(!p.error.empty() || (!p.towers.empty() && !p.towers.front().fallback.empty()),
            "each refusal provides a named manager or original-source fallback");
        if (failure <= 10 || (failure >= 14 && failure <= 19))
            check(p.tower_source_at_origin.size() == 1 && p.tower_source_at_origin.at(raw(origin)) == 0 &&
                p.towers.front().origin == origin,
                "unsupported state/schema/raw/reference at a valid origin retains read-only F1 source diagnostics without claims");
        else check(p.tower_source_at_origin.empty(),
            "unbounded origin or failed complete-manager identity publishes no source-origin hint");
    }
    auto inactive = good; inactive.entities.records.front().status = 0;
    check(inactive.prepare().towers.empty() && no_claims(inactive.prepare()), "inactive tower claims no image");
    for (const auto c : {maps::OriginalEntityClass::Building, maps::OriginalEntityClass::GateHouse,
        maps::OriginalEntityClass::Monument, maps::OriginalEntityClass::ResWall}) {
        auto wrong = good; wrong.entities.records.front().entity_class = c;
        check(wrong.prepare().towers.empty() && no_claims(wrong.prepare()), "entity class, not numeric type, establishes cTower");
    }
    for (unsigned failure = 0; failure < 6; ++failure) {
        auto input = good;
        if (failure == 0) input.terrain.pop_back();
        if (failure == 1) input.objects.pop_back();
        if (failure == 2) input.heights.pop_back();
        if (failure == 3) input.entities.declared_map_size = 169;
        if (failure == 4) input.entities.manager_schema = 2;
        if (failure == 5) input.entities.records.resize(4001, input.entities.records.front());
        const auto p = input.prepare(); check(!p.error.empty() && p.towers.empty() && no_claims(p),
            "incomplete map or unsupported complete manager fails atomically");
        check(p.tower_source_at_origin.empty(), "incomplete manager/map publishes no diagnostic-origin index");
    }
}
void conflicts() {
    for (unsigned kind = 0; kind < 5; ++kind) {
        Input input; auto e = source(4, {101,111}, 12, 8);
        if (kind == 1) e.entity_class = maps::OriginalEntityClass::Building;
        if (kind == 2) {e.entity_class = maps::OriginalEntityClass::GateHouse; e.type = 130;
            e.footprint_side = 1; e.gate_house = maps::OriginalGateHouseState{1};}
        if (kind == 3) {e.entity_class = maps::OriginalEntityClass::Industrial; e.type = 175; e.footprint_side = 0;}
        if (kind == 4) {e.entity_class = maps::OriginalEntityClass::ResWall; e.type = 231;
            e.footprint_side = 1; e.provenance.class_wrapper_schema = 1;}
        input.entities.records.push_back(e); input.mark({101,111});
        const auto p = input.prepare();
        check(no_claims(p) && p.towers.front().components.empty() &&
            p.towers.front().fallback.find("conflict") != std::string::npos,
            "Tower body never competes with a foreign Tower, saved square, typed GateHouse or exact reserved origin");
    }
    for (bool id : {false, true}) {
        Input input; auto e = source(3, {120,130}, id ? 12 : 11, id ? 7 : 8);
        input.entities.records.push_back(e); input.mark({120,130});
        const auto p = input.prepare(); check(!p.error.empty() && p.towers.empty() && no_claims(p),
            "duplicate manager or original identity cannot circumvent complete-parser atomicity");
        check(p.tower_source_at_origin.empty(), "duplicate manager identities leave no partially published source hints");
    }
    Input input; input.entities.records.push_back(source(3, {120,130}, 12, 8)); input.mark({120,130});
    const auto p = input.prepare(); check(p.towers.size() == 2 && p.towers[0].components.size() == 1 &&
        p.towers[1].components.size() == 1 && p.tower_by_storage[raw({121,131})] == 1,
        "independent complete towers keep their stable source ordinals and own cells");
    input.entities.records.back().status = 1;
    const auto bounded = input.prepare();
    check(bounded.towers[0].components.size() == 1 && bounded.towers[1].components.empty() &&
        !bounded.towers[1].fallback.empty() && bounded.tower_by_storage[raw({100,110})] == 0 &&
        !bounded.tower_by_storage[raw({120,130})],
        "an unsupported independent Tower state preserves a complete supported foreign source");
    input = Input{};input.entities.records.push_back(source(3,origin,12,8));
    const auto ambiguous = input.prepare();
    check(ambiguous.error.empty() && no_claims(ambiguous) && ambiguous.tower_source_at_origin.size() == 1 &&
        !ambiguous.tower_source_at_origin.at(raw(origin)) && ambiguous.towers.size() == 2,
        "different original IDs sharing one origin are explicitly ambiguous diagnostics, never one selected object hint");
    input=Input{};
    auto wall=source(4,{102,111},12,8);wall.entity_class=maps::OriginalEntityClass::ResWall;
    wall.type=231;wall.footprint_side=1;wall.provenance.class_wrapper_schema=1;
    input.entities.records.push_back(wall);
    const auto beside=input.prepare();
    check(beside.towers.size()==1 && beside.towers.front().components.size()==1 &&
          beside.towers.front().joins.status==maps::TowerJoinStatus::NoJoinRequired,
          "ResWall owner beside a Tower is Other and creates no Tower/Gate join or body");
}
} // namespace

int main() {
    try {positive(); refusals(); conflicts(); std::cout << "ordinary cTower selectors and complete claims PASS\n"; return 0;}
    catch (const std::exception& error) {std::cerr << error.what() << '\n'; return 1;}
}
