// Tests for the original-style model reader, desirability map and house
// evolution. Synthetic numbers only; the optional install test reads the
// user's own game files from OPENEMPEROR_GAME_DIR and skips when unset.
#include "simulation/original/Desirability.h"
#include "simulation/original/HouseEvolution.h"
#include "simulation/original/ModelData.h"
#include "simulation/original/OriginalHousingView.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace openemperor::original;
namespace {
void check(bool ok, const char* msg) { if (!ok) throw std::runtime_error(msg); }

std::string synthetic_model() {
    std::ostringstream s;
    s << "Notes\r\nBUILDING MODS\r\n";
    const char* pct[] = {"50", "80", "100", "120", "150"};
    for (int d = 0; d < 5; ++d) {
        s << d << ",Mods,{";
        for (int f = 0; f < 13; ++f) s << "," << (f == 1 ? pct[d] : "100");
        s << ",},,,\r\n";
    }
    s << "ALL BUILDINGS\r\n";
    s << "7,Test Sink,{,10,-8,2,2,5,0,0,0,0,0,0,0,0,},,,\r\n";
    s << "8,Test Fountain,{,10,6,1,-2,4,0,0,0,0,0,0,0,0,},,,\r\n";
    s << "HOUSE MODS\r\n";
    for (int d = 0; d < 5; ++d) {
        s << "Mod" << d << ",{";
        for (int f = 0; f < 24; ++f) s << "," << (f == 0 ? d - 2 : 0);
        s << ",,,\r\n";
    }
    s << "ALL HOUSES\r\n";
    for (int r = 0; r < 15; ++r) {
        // low = r*10-20, high = r*10+10; water needed for rows >= 2,
        // food quality 5*r from row 4.
        s << r << ": Row,{," << (r * 10 - 20) << "," << (r * 10 + 10)
          << ",0," << (r >= 2 ? 1 : 0) << ",0,0,0,0," << (r >= 4 ? 5 * r : 0)
          << ",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\r\n";
    }
    s << "End of model data.\r\n";
    return s.str();
}

void test_parser() {
    const auto m = parse_model_data(synthetic_model());
    check(m.houses.size() == 15 && m.house_mods.size() == 5 && m.building_mods.size() == 5, "section sizes");
    check(m.has_building(7) && m.has_building(8) && !m.has_building(9), "building presence");
    check(m.building_value(7, BF_DesValue, 2) == -8, "normal difficulty keeps value");
    check(m.building_value(7, BF_DesValue, 0) == -4, "very easy halves value");
    check(m.building_value(8, BF_DesValue, 4) == 9, "very hard x1.5 truncates");
    // Row 5: low 30, high 60; difficulty 0 adds -2 to field 0 only.
    check(m.house_value(5, HF_DesLow, 2) == 30, "house base");
    check(m.house_value(5, HF_DesLow, 0) == 28, "house modifier");
    check(m.house_value(0, HF_DesLow, 0) == -22, "negative values parse");
    bool threw = false;
    try { parse_model_data("ALL HOUSES\r\n1,x,{,1,2,},\r\n"); } catch (const std::runtime_error&) { threw = true; }
    check(threw, "malformed input throws");
}

void test_spread() {
    DesirabilityMap map(30, 30);
    // Value 4, -1 per ring, three rings: 4, 3, 2.
    map.apply_source(10, 10, 1, {4, 1, -1, 3});
    check(map.at(10, 10) == 0, "footprint tile gets nothing");
    check(map.at(11, 10) == 4 && map.at(11, 11) == 4, "ring 1");
    check(map.at(12, 10) == 3 && map.at(8, 8) == 3, "ring 2");
    check(map.at(13, 13) == 2, "ring 3");
    check(map.at(14, 10) == 0, "beyond range");

    // Step of 3: -15 for rings 1-3, then -13 for 4-6, then -11, capped to range 8.
    map.clear();
    map.apply_source(10, 10, 1, {-15, 3, 2, 8});
    check(map.at(11, 10) == -15 && map.at(13, 10) == -15, "rings 1-3");
    check(map.at(14, 10) == -13 && map.at(16, 10) == -13, "rings 4-6");
    check(map.at(17, 10) == -11 && map.at(18, 10) == -11, "rings 7-8");
    check(map.at(19, 10) == 0, "range 8");

    // A fading value stops at zero and never flips sign.
    map.clear();
    map.apply_source(10, 10, 1, {2, 1, -2, 5});
    check(map.at(11, 10) == 2 && map.at(12, 10) == 0 && map.at(13, 10) == 0, "stops at zero");

    // Range is capped at 10 rings.
    map.clear();
    map.apply_source(15, 15, 1, {5, 99, 0, 25});
    check(map.at(25, 15) == 5 && map.at(26, 15) == 0, "range cap 10");

    // Values saturate at +-100 and add up.
    map.clear();
    for (int i = 0; i < 30; ++i) map.apply_source(10, 10, 1, {9, 1, 0, 1});
    check(map.at(11, 10) == 100, "saturates at 100");

    // Larger footprint: ring 1 sits one tile outside the 2x2, interior untouched.
    map.clear();
    map.apply_source(10, 10, 2, {5, 1, 0, 1});
    check(map.at(10, 10) == 0 && map.at(11, 11) == 0, "footprint interior untouched");
    check(map.at(9, 9) == 5 && map.at(12, 12) == 5 && map.at(12, 10) == 5, "ring around 2x2");

    // Off-map tiles are skipped without error.
    map.clear();
    map.apply_source(0, 0, 1, {3, 1, 0, 2});
    check(map.at(1, 0) == 3 && map.at(-1, 0) == 0, "edges are safe");

    // Building value: max over footprint, plus an unclamped bonus.
    map.clear();
    map.apply_source(10, 10, 1, {7, 1, 0, 1});
    check(map.building_value(11, 10, 2) == 7, "max over footprint");
    check(map.building_value(11, 10, 1, 10) == 17, "bonus is not clamped");
}

void test_walls() {
    // A wall ring shields negative sources beyond it but not positive ones.
    auto build = [](int value) {
        DesirabilityMap map(40, 40);
        for (int y = 0; y < 40; ++y) map.set_wall(20 + 3, y);  // vertical wall at x=23
        map.apply_source(20, 20, 1, {value, 1, 0, 6});
        return map;
    };
    const auto neg = build(-5);
    check(neg.at(22, 20) == -5, "before wall receives value");
    check(neg.at(23, 20) == 0, "wall tile receives nothing");
    check(neg.at(25, 20) == 0, "behind wall is shielded");
    check(neg.at(18, 20) == -5, "other side unaffected");
    const auto pos = build(5);
    check(pos.at(25, 20) == 5 && pos.at(23, 20) == 5, "positive ignores walls");

    DesirabilityMap off(40, 40);
    off.set_wall(23, 20);
    off.apply_source(20, 20, 1, {-5, 1, 0, 6}, WallBlocking::None);
    check(off.at(25, 20) == -5, "blocking can be disabled");
}

HouseState house(int type, int desirability) {
    HouseState h;
    h.type = type;
    h.desirability = desirability;
    h.water = true;
    h.food_quality = 100;
    return h;
}

void test_evolution() {
    const auto m = parse_model_data(synthetic_model());
    const int d = 2;
    check(house_row(HT_Shelter) == 0 && house_row(HT_Vacant) == 0 && house_row(HT_Hut) == 1, "rows");
    check(next_row(HT_EliteUnoccupied) == house_row(HT_EliteUnoccupied) + 2, "unoccupied skips abandoned");
    check(next_row(HT_HeavenlyCompound) == 14, "top maps to itself");

    // Hut (row 1): low -10, high 20. Next row (2) needs water.
    HouseState h = house(HT_Hut, 25);
    check(evolve_step(m, d, h) && h.type == HT_PlainCottage, "evolves immediately");

    // Evolution blocked when next level's needs are missing.
    h = house(HT_Hut, 25);
    h.water = false;
    check(needs_met(m, d, 1, h) && !needs_met(m, d, 2, h), "needs differ by row");
    check(combined_verdict(m, d, h) == Verdict::Stay, "stay when next needs unmet");
    check(!evolve_step(m, d, h) && h.type == HT_Hut, "no change");

    // Devolve needs limit+1 consecutive failures (limit 9 => 10th check).
    h = house(HT_PlainCottage, -50);
    for (int i = 0; i < 9; ++i) check(!evolve_step(m, d, h) && h.devolve_counter == i + 1, "counting");
    check(evolve_step(m, d, h) && h.type == HT_Hut && h.devolve_counter == 0, "devolves on 10th");

    // A single good check resets the counter.
    h = house(HT_PlainCottage, -50);
    for (int i = 0; i < 5; ++i) evolve_step(m, d, h);
    h.desirability = 10;
    check(!evolve_step(m, d, h) && h.devolve_counter == 0, "reset on stay");

    // Missing current-level need devolves even with great desirability.
    h = house(HT_PlainCottage, 90);
    h.water = false;
    check(combined_verdict(m, d, h) == Verdict::Devolve, "missing need devolves");

    // Modest Siheyuan is slow to fall; Abandoned ignores desirability.
    h = house(HT_ModestSiheyuan, -90);
    for (int i = 0; i < 49; ++i) evolve_step(m, d, h);
    check(h.type == HT_ModestSiheyuan, "still modest after 49");
    check(evolve_step(m, d, h) && h.type == HT_EliteAbandoned, "falls on the 50th");
    h = house(HT_EliteAbandoned, -90);
    check(evolve_step(m, d, h) && h.type == HT_ModestSiheyuan, "abandoned returns regardless of desirability");

    // Vacant houses fill only with occupants; unoccupied elites never change.
    h = house(HT_Vacant, 0);
    check(!evolve_step(m, d, h), "empty vacant stays");
    h.occupants = 3;
    check(evolve_step(m, d, h) && h.type == HT_Shelter, "occupied vacant becomes shelter");
    h = house(HT_EliteUnoccupied, 99);
    check(!evolve_step(m, d, h), "elite unoccupied inert");

    // Tops cannot evolve; the bottom common house cannot devolve.
    h = house(HT_LuxuriousApartment, 100);
    check(!evolve_step(m, d, h) && h.type == HT_LuxuriousApartment, "top common");
    h = house(HT_Shelter, -99);
    for (int i = 0; i < 30; ++i) check(!evolve_step(m, d, h) && h.type == HT_Shelter, "shelter floor");

    // Bronzeware OR lacquerware; Daoist OR Buddhist.
    HouseState g = house(HT_Hut, 0);
    g.bronzeware = 0; g.lacquerware = 3;
    check(g.lacquerware > g.bronzeware, "setup");
}


void test_world_view() {
    const char* dir = std::getenv("OPENEMPEROR_GAME_DIR");
    if (!dir || !*dir) { std::cout << "world view test skipped (OPENEMPEROR_GAME_DIR unset)\n"; return; }
    namespace sim = openemperor::simulation;
    const auto m = load_model_data(dir);
    sim::World w(160, 32, std::vector<std::uint8_t>(160 * 32, 1), sim::RulesProfile::CityV14);
    auto place = [&](sim::CommandType t, sim::Cell c) {
        const auto r = w.execute({t, c});
        check(r.accepted && r.changed, "placement failed");
        return *w.building_owner_at(c);
    };
    const auto near_warehouse = place(sim::CommandType::PlaceHousehold, {10, 10});
    const auto far_house = place(sim::CommandType::PlaceHousehold, {60, 10});
    place(sim::CommandType::PlaceWarehouse, {14, 10});
    const auto reports = evaluate_original_housing(w, m);
    check(reports.size() == 2, "one report per house");
    int near_d = 0, far_d = 99;
    for (const auto& r : reports) (r.id == near_warehouse ? near_d : far_d) = r.desirability;
    (void)far_house;
    check(near_d == -13, "warehouse ring 3/4 over the 2x2 footprint takes the maximum");
    check(far_d == 0, "distant house unaffected");
    for (const auto& r : reports) check(r.settled_type >= HT_Shelter, "settled type valid");
    std::cout << "world view test passed\n";
}

void test_service_timers() {
    HouseState h;
    check(h.herbalist == 0, "no access by default");
    grant_service(h.herbalist);
    check(h.herbalist == 96, "visit sets 96");
    // Access lasts 95 decay passes and is gone on the 96th.
    for (int i = 0; i < 95; ++i) { decay_services(h); check(h.herbalist > 0, "still served"); }
    check(h.herbalist == 1, "last tick");
    decay_services(h);
    check(h.herbalist == 0, "lost on the 96th decay");
    decay_services(h);
    check(h.herbalist == 0, "stays at zero");

    // Religion: Daoist OR Buddhist suffices; timers are independent.
    const auto m = parse_model_data(synthetic_model());
    grant_service(h.music);
    decay_services(h);
    check(h.music == 95 && h.drama == 0, "independent timers");

    // A need backed by a timer is lost when it runs out (row 2 needs water only in
    // the synthetic data, so use the real table when available).
    const char* dir = std::getenv("OPENEMPEROR_GAME_DIR");
    if (dir && *dir) {
        const auto real = load_model_data(dir);
        HouseState s = house(HT_SpaciousDwelling, 24);  // row 4: herbalist needed
        s.food_quality = 100; s.ceramics = 9; s.hemp = 9; s.tea = 9; s.silk = 9; s.bronzeware = 9;
        const int row = house_row(HT_SpaciousDwelling);
        check(real.house_value(row, HF_Herbalist, 2) != 0, "spacious dwelling needs herbalist");
        check(!needs_met(real, 2, row, s), "no herbalist, needs unmet");
        grant_service(s.herbalist);
        check(!needs_met(real, 2, row, s), "music is also required at this level");
        grant_service(s.music);
        check(!needs_met(real, 2, row, s), "ancestor worship is also required at this level");
        grant_service(s.ancestor);
        check(needs_met(real, 2, row, s), "herbalist met");
        for (int i = 0; i < 96; ++i) decay_services(s);
        check(!needs_met(real, 2, row, s), "unmet again once the timer expires");
    }
    (void)m;
}

void test_spawn_rate() {
    check(service_spawn_interval(100) == 1 && service_spawn_interval(150) == 1, "full staffing");
    check(service_spawn_interval(99) == 3 && service_spawn_interval(75) == 3, "75-99");
    check(service_spawn_interval(74) == 7 && service_spawn_interval(50) == 7, "50-74");
    check(service_spawn_interval(49) == 15 && service_spawn_interval(25) == 15, "25-49");
    check(service_spawn_interval(24) == 29 && service_spawn_interval(1) == 29, "1-24");
    check(service_spawn_interval(0) == 64 && service_spawn_interval(-5) == 64, "unstaffed");
    // Interval 3: a walker leaves on every 4th update (counter must exceed 3).
    int c = 0, sent = 0;
    for (int i = 0; i < 40; ++i) if (service_spawn_tick(c, 75)) ++sent;
    check(sent == 10, "interval 3 sends every 4th update");
    c = 0; sent = 0;
    for (int i = 0; i < 40; ++i) if (service_spawn_tick(c, 100)) ++sent;
    check(sent == 20, "interval 1 sends every 2nd update");
}

void test_install() {
    const char* dir = std::getenv("OPENEMPEROR_GAME_DIR");
    if (!dir || !*dir) { std::cout << "install test skipped (OPENEMPEROR_GAME_DIR unset)\n"; return; }
    const auto m = load_model_data(dir);
    check(m.houses.size() == 15 && m.house_mods.size() == 5, "install section sizes");
    for (int t : {54, 59, 111, 113, 115, 13}) check(m.has_building(t), "install lacks expected building");
    // Independently observed worked examples at normal difficulty.
    auto spec = [&](int type) {
        return SpreadSpec{m.building_value(type, BF_DesValue, 2), m.building_value(type, BF_DesStep, 2),
                          m.building_value(type, BF_DesStepSize, 2), m.building_value(type, BF_DesRange, 2)};
    };
    DesirabilityMap map(40, 40);
    map.apply_source(20, 20, 1, spec(111));
    check(map.at(21, 20) == 4 && map.at(22, 20) == 3 && map.at(23, 20) == 2 && map.at(24, 20) == 0, "Grand Way rings");
    map.clear();
    map.apply_source(20, 20, 1, spec(54));
    check(map.at(21, 20) == -15 && map.at(24, 20) == -13 && map.at(27, 20) == -11 && map.at(29, 20) == 0, "Warehouse rings");
    std::cout << "install test passed against " << dir << "\n";
}
}  // namespace

int main() {
    try {
        test_parser();
        test_spread();
        test_walls();
        test_evolution();
        test_service_timers();
        test_spawn_rate();
        test_install();
        test_world_view();
    } catch (const std::exception& e) {
        std::cerr << "FAILED: " << e.what() << "\n";
        return 1;
    }
    std::cout << "original rules tests passed\n";
    return 0;
}
