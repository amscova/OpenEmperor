#pragma once
// Paid authored layout shared by the frozen rule-3 replay and rule-4 long-play.
// Coordinates are translated once; legality still comes from actual World/map
// permissions. No goods, funds, population or coverage are supplied by this tool.
#include "simulation/World.h"
#include <vector>
namespace openemperor::city_playthrough {
namespace sim = simulation;
struct Action {
    std::uint64_t tick;
    sim::CommandType type;
    sim::Cell cell;
    const char *name;
};
inline std::vector<Action> expansion_recipe(sim::Cell origin) {
    std::vector<Action> a;
    const auto add = [&](std::uint64_t t, sim::CommandType type, int x, int y, const char *name) {
        a.push_back({t, type, {origin.x + x - 103, origin.y + y - 113}, name});
    };
    const auto road = [&](std::uint64_t t, int x, int y) {
        add(t, sim::CommandType::PlaceRoad, x, y, "road");
    };
    add(800, sim::CommandType::PlaceWell, 114, 116, "well");
    add(1200, sim::CommandType::PlaceHousehold, 116, 113, "house");
    road(1200, 118, 115);
    road(1200, 119, 115);
    add(1600, sim::CommandType::PlaceHousehold, 119, 113, "house");
    add(2000, sim::CommandType::PlaceMarket, 118, 116, "market");
    road(2000, 120, 115);
    for (int y = 115; y <= 119; ++y)
        road(2000, 121, y);
    for (int x = 112; x <= 124; ++x)
        if (x != 121)
            road(2000, x, 119);
    road(2000, 117, 120);
    road(2000, 117, 121);
    add(2400, sim::CommandType::PlaceServicePost, 117, 122, "service");
    add(2400, sim::CommandType::PlaceFireWatch, 111, 114, "fire_watch");
    for (int y = 116; y <= 122; ++y)
        road(2400, 105, y);
    for (int x = 103; x <= 104; ++x) {
        road(2400, x, 119);
        road(2400, x, 122);
    }
    add(2800, sim::CommandType::PlaceClaySource, 103, 120, "clay");
    add(3200, sim::CommandType::PlacePottery, 103, 123, "pottery");
    add(3200, sim::CommandType::PlaceWell, 112, 120, "well");
    add(3600, sim::CommandType::PlaceWell, 122, 113, "well");
    add(3600, sim::CommandType::PlaceHousehold, 119, 116, "house");
    add(4000, sim::CommandType::PlaceHousehold, 122, 116, "house");
    add(4000, sim::CommandType::PlaceHousehold, 115, 120, "house");
    add(4400, sim::CommandType::PlaceHousehold, 118, 120, "house");
    for (int y = 120; y <= 122; ++y)
        road(4400, 114, y);
    add(4400, sim::CommandType::PlaceHealthPost, 114, 123, "health");
    add(4800, sim::CommandType::PlaceWell, 114, 113, "well");
    return a;
}
inline std::vector<sim::Command> paid_starter(sim::Cell o) {
    std::vector<sim::Command> c{{sim::CommandType::PlaceClaySource, o},
                                {sim::CommandType::PlacePottery, {o.x, o.y + 3}},
                                {sim::CommandType::PlaceWarehouse, {o.x + 3, o.y}},
                                {sim::CommandType::PlaceFarm, {o.x + 4, o.y + 3}},
                                {sim::CommandType::PlaceMarket, {o.x + 3, o.y + 3}},
                                {sim::CommandType::PlaceServicePost, {o.x + 5, o.y + 3}}};
    for (int x = 0; x <= 14; ++x)
        c.push_back({sim::CommandType::PlaceRoad, {o.x + x, o.y + 2}});
    for (auto p : std::vector<sim::Cell>{{6, 0}, {6, 3}, {9, 0}, {9, 3}})
        c.push_back({sim::CommandType::PlaceHousehold, {o.x + p.x, o.y + p.y}});
    c.push_back({sim::CommandType::PlaceFireWatch, {o.x + 14, o.y + 3}});
    return c;
}
} // namespace openemperor::city_playthrough
