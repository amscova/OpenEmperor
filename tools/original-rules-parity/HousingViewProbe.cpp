// Independent observations of the pinned fork's read-only World adapter.
// Compile with fork includes before current src, and link the unchanged current
// simulation library. The supplied model file is synthetic; the two rows below
// are explicit authored controls, never copied original balance values.
#include "simulation/original/OriginalHousingView.h"

#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace original = openemperor::original;
namespace simulation = openemperor::simulation;

namespace {

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

simulation::BuildingId place(simulation::World& world, simulation::CommandType command,
                             simulation::Cell cell) {
    const auto result = world.execute({command, cell});
    require(result.accepted && result.changed, "HousingView probe placement refused");
    const auto owner = world.building_owner_at(cell);
    require(owner.has_value(), "HousingView probe placement has no owner");
    return *owner;
}

original::OriginalHouseReport evaluate_one(const simulation::World& world,
                                          const original::ModelData& model,
                                          int steps, int food_quality = 0) {
    original::OriginalHousingOptions options;
    options.evolution_steps = steps;
    options.food_quality_when_fed = food_quality;
    const auto reports = original::evaluate_original_housing(world, model, options);
    require(reports.size() == 1, "HousingView probe expected exactly one report");
    return reports.front();
}

} // namespace

int main(int argc, char** argv) {
    try {
        require(argc == 2, "usage: HousingViewProbe SYNTHETIC_GAME_ROOT");
        auto model = original::load_model_data(argv[1]);
        // The adapter's score is zero in an otherwise empty scene. The control
        // model asks Shelter to evolve and Hut to Stay, without any needs.
        for (auto& row : model.house_mods) row.fill(0);
        model.houses[0].fill(0);
        model.houses[0][original::HF_DesLow] = -20;
        model.houses[0][original::HF_DesHigh] = -1;
        model.houses[1].fill(0);
        model.houses[1][original::HF_DesLow] = -20;
        model.houses[1][original::HF_DesHigh] = 1;

        simulation::World empty(16, 16, std::vector<std::uint8_t>(16 * 16, 1),
                                simulation::RulesProfile::HouseholdV3);
        const auto house = place(empty, simulation::CommandType::PlaceHousehold, {6, 2});
        require(empty.building(house).population == 0, "control house is not empty");
        const auto before = empty.snapshot();
        const auto initial = evaluate_one(empty, model, 0);
        const auto developed = evaluate_one(empty, model, 1);
        require(empty.snapshot() == before, "fork read-only adapter mutated the World");

        // Food reaches this independent control through unchanged production,
        // roads and courier travel, with no inventory or snapshot injection.
        simulation::World fed(16, 16, std::vector<std::uint8_t>(16 * 16, 1),
                              simulation::RulesProfile::CityV7);
        const auto fed_house = place(fed, simulation::CommandType::PlaceHousehold, {6, 2});
        static_cast<void>(place(fed, simulation::CommandType::PlaceFarm, {2, 2}));
        for (int x = 2; x <= 6; ++x) {
            const auto road = fed.execute({simulation::CommandType::PlaceRoad, {x, 3}});
            require(road.accepted && road.changed, "HousingView food control road refused");
        }
        for (int tick = 0; tick < 300; ++tick) fed.tick();
        require(fed.building(fed_house).food_stock > 0, "HousingView food control has no delivery");
        // Suppress the diagnostic Farm's desirability and add one independent
        // synthetic food-score requirement. Changing a report option must not
        // be mistaken for deriving quality from the actually delivered food.
        for (auto& row : model.buildings) row.fill(0);
        model.houses[0][original::HF_FoodQuality] = 1;
        model.houses[1][original::HF_FoodQuality] = 1;
        const auto fed_before = fed.snapshot();
        const auto without_food_assumption = evaluate_one(fed, model, 1, 0);
        const auto with_food_assumption = evaluate_one(fed, model, 1, 100);
        require(fed.snapshot() == fed_before, "fork food report mutated the World");

        constexpr std::array objects{
            simulation::Object::ClaySource, simulation::Object::Pottery,
            simulation::Object::Warehouse, simulation::Object::Market,
            simulation::Object::Farm, simulation::Object::Well,
            simulation::Object::HealthPost};
        std::cout << '[';
        for (const auto object : objects)
            std::cout << original::original_model_type(object) << ',';
        // JSON vector: seven functional row candidates; report count; actual
        // zero World population; peak/settled with zero steps; peak/settled
        // after one artificial step; complete WorldSnapshot equality; a real
        // food stock is present; settled type for assumed food quality 0/100;
        // complete fed WorldSnapshot equality.
        std::cout << 1 << ',' << empty.building(house).population << ','
                  << initial.peak_type << ',' << initial.settled_type << ','
                  << developed.peak_type << ',' << developed.settled_type << ','
                  << (empty.snapshot() == before ? 1 : 0) << ','
                  << (fed.building(fed_house).food_stock > 0 ? 1 : 0) << ','
                  << without_food_assumption.settled_type << ','
                  << with_food_assumption.settled_type << ','
                  << (fed.snapshot() == fed_before ? 1 : 0) << "]\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
