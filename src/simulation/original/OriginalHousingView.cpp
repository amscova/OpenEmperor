#include "simulation/original/OriginalHousingView.h"

#include "simulation/original/Desirability.h"

#include <algorithm>

namespace openemperor::original {
namespace {

int footprint_size(const simulation::World& world, simulation::Object kind) {
    const auto f = simulation::building_footprint(world.profile(), world.rule_version(), kind);
    return std::max(f.width, f.height);
}

}  // namespace

int original_model_type(simulation::Object kind) {
    using simulation::Object;
    switch (kind) {
    case Object::ClaySource: return 35;  // Clay Pit
    case Object::Pottery: return 43;     // Kiln
    case Object::Warehouse: return 54;
    case Object::Market: return 59;      // Common Market Square
    case Object::Farm: return 193;       // Farmhouse
    case Object::Well: return 72;
    case Object::HealthPost: return 207; // Herbalist's Stall
    default: return -1;
    }
}

std::vector<OriginalHouseReport> evaluate_original_housing(
    const simulation::World& world, const ModelData& model, const OriginalHousingOptions& options) {
    DesirabilityMap map(world.width(), world.height());
    for (const auto& b : world.buildings()) {
        if (!b.placed) continue;
        const int type = original_model_type(b.kind);
        if (type < 0 || !model.has_building(type)) continue;
        const SpreadSpec spec{model.building_value(type, BF_DesValue, options.difficulty),
                              model.building_value(type, BF_DesStep, options.difficulty),
                              model.building_value(type, BF_DesStepSize, options.difficulty),
                              model.building_value(type, BF_DesRange, options.difficulty)};
        map.apply_source(b.cell.x, b.cell.y, footprint_size(world, b.kind), spec);
    }

    std::vector<OriginalHouseReport> reports;
    for (const auto& b : world.buildings()) {
        if (!b.placed || b.kind != simulation::Object::Household) continue;
        OriginalHouseReport report;
        report.id = b.id;
        report.desirability = std::clamp(
            map.building_value(b.cell.x, b.cell.y, footprint_size(world, b.kind)), -100, 100);

        HouseState house;
        house.type = HT_Shelter;
        house.occupants = std::max(b.population, 1);
        house.desirability = report.desirability;
        house.water = world.household_has_water(b.id);
        const bool herbalist_visits = world.household_health_protected(b.id);
        if (herbalist_visits) grant_service(house.herbalist);
        house.food_quality = b.food_stock > 0 ? options.food_quality_when_fed : 0;

        report.peak_type = house.type;
        for (int i = 0; i < options.evolution_steps; ++i) {
            decay_services(house);
            if (herbalist_visits) grant_service(house.herbalist);  // steady revisits
            evolve_step(model, options.difficulty, house);
            report.peak_type = std::max(report.peak_type, house.type);
        }
        report.settled_type = house.type;
        reports.push_back(report);
    }
    return reports;
}

}  // namespace openemperor::original
