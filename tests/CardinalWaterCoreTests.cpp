#include "simulation/OriginalTerrainFacts.h"
#include "simulation/original/Desirability.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
void check(bool value,const char* why) { if (!value) throw std::runtime_error(why); }
template<class F> void refuses(F action) {
    try {action();}catch(const std::invalid_argument&) {return;}
    throw std::runtime_error("unsupported cardinal-water input accepted");
}
using Terrain=std::vector<std::uint32_t>;
const std::string source_hash(64,'a');
std::size_t at(sim::Cell cell) {return std::size_t(cell.y)*std::size_t(sim::OriginalTerrainFacts::width)+std::size_t(cell.x);}
sim::OriginalTerrainFacts one_water(sim::Cell cell,std::uint32_t word=4) {
    Terrain terrain(sim::OriginalTerrainFacts::cell_count,0x80U);
    terrain[at(cell)]=word;
    return {terrain,source_hash};
}
void geometry_and_immutability() {
    const sim::Cell origin{20,20};
    // Independent boundary cases distinguish side4 cardinal adjacency from
    // diagonals, interior, side2 geometry, distance-only or repeated bonuses.
    for (const auto cell:std::array<sim::Cell,8>{{{20,19},{23,19},{24,20},{24,23},
            {20,24},{23,24},{19,20},{19,23}}}) {
        const auto facts=one_water(cell);
        check(facts.cardinal_water_bonus(origin,4)==10,"cardinal edge did not receive its single bonus");
    }
    for (const auto cell:std::array<sim::Cell,9>{{{19,19},{24,19},{19,24},{24,24},
            {20,20},{21,21},{23,23},{25,20},{20,18}}})
        check(one_water(cell).cardinal_water_bonus(origin,4)==0,"diagonal, interior or distant water granted bonus");
    const auto larger=one_water({24,20});
    check(larger.cardinal_water_bonus(origin,4)==10 && larger.cardinal_water_bonus(origin,2)==0,
        "prepared side2/4 queries were conflated");
    const auto smaller=one_water({22,20});
    check(smaller.cardinal_water_bonus(origin,2)==10 && smaller.cardinal_water_bonus(origin,4)==0,
        "interior water became a side4 edge");
    check(one_water({20,19},0x8000104U).cardinal_water_bonus(origin,4)==10,
        "raw bit4 was narrowed to exact normal-water words");
    check(one_water({20,19},0x4000U).cardinal_water_bonus(origin,4)==0,
        "wall bit was confused with terrain bit4");
    Terrain terrain(sim::OriginalTerrainFacts::cell_count,4);
    const sim::OriginalTerrainFacts before(terrain,source_hash);
    check(before.cardinal_water_bonus(origin,2)==10 && before.cardinal_water_bonus(origin,4)==10,
        "multiple edge cells stacked the bonus");
    std::fill(terrain.begin(),terrain.end(),0);
    check(before.cardinal_water_bonus(origin,4)==10 && before.source_map_sha256()==source_hash,
        "prepared facts retained mutable input storage");
    const sim::OriginalTerrainFacts after(terrain,source_hash);
    check(before!=after,"same claimed hash concealed different prepared terrain facts");
    check(after==sim::OriginalTerrainFacts(terrain,source_hash),"identical prepared source facts compare unequal");
}
void boundaries_and_query_domain() {
    const Terrain terrain(sim::OriginalTerrainFacts::cell_count);
    const sim::OriginalTerrainFacts facts(terrain,source_hash);
    for (int side:{2,4}) {
        check(facts.supports_cardinal_footprint({1,1},side) &&
            facts.supports_cardinal_footprint({227-side,227-side},side),"last valid perimeter refused");
        for (auto cell:std::array<sim::Cell,5>{{{0,1},{1,0},{228-side,1},{1,228-side},
                {std::numeric_limits<int>::max(),std::numeric_limits<int>::min()}}}) {
            check(!facts.supports_cardinal_footprint(cell,side),"out-of-storage perimeter admitted");
            refuses([&]{(void)facts.cardinal_water_bonus(cell,side);});
        }
    }
    for (int side:{-1,0,1,3,5,6,std::numeric_limits<int>::max()})
        refuses([&]{(void)facts.cardinal_water_bonus({20,20},side);});
    refuses([&]{(void)sim::OriginalTerrainFacts(std::span(terrain).first(terrain.size()-1),source_hash);});
    refuses([&]{(void)sim::OriginalTerrainFacts(terrain,std::string(64,'A'));});
    refuses([&]{(void)sim::OriginalTerrainFacts(terrain,std::string(63,'a'));});
    orig::DesirabilityMap map(32,32);
    map.apply_source({{14,14,2},{100,1,0,10}});
    check(map.building_value({10,10,4})==100 && map.building_value({10,10,4},10)==110,
        "recipient bonus was clamped to the source-cell domain");
}
}
int main() {
    try {
        geometry_and_immutability();boundaries_and_query_domain();
        std::cout<<"cardinal-water core checks passed\n";
    } catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
}
