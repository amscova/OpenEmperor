#pragma once
// Reader for the game's own balance file (Model/EmperorBuildingModels.txt).
// The numbers are read from the user's installation at runtime and are never
// embedded in this repository.
#include <array>
#include <filesystem>
#include <string_view>
#include <vector>

namespace openemperor::original {

inline constexpr int building_field_count = 13;
inline constexpr int house_field_count = 24;
inline constexpr int house_row_count = 15;
inline constexpr int difficulty_count = 5;

// Building model field indices (the file's legend letters a..m).
enum BuildingField : int {
    BF_Cost, BF_DesValue, BF_DesStep, BF_DesStepSize, BF_DesRange, BF_Employees,
    BF_FireRisk, BF_DamageRisk, BF_Resource, BF_RiskReducer, BF_EvolveDes,
    BF_Integrity, BF_FengShui
};

// House model field indices (legend letters a..x).
enum HouseField : int {
    HF_DesLow, HF_DesHigh, HF_Herbalist, HF_Water, HF_Acupuncture, HF_Music,
    HF_Acrobat, HF_Drama, HF_FoodQuality, HF_Hemp, HF_Ceramics, HF_Tea, HF_Silk,
    HF_Bronzeware, HF_CrimeInc, HF_CrimeBase, HF_Unused16, HF_Capacity, HF_TaxRate,
    HF_Unused19, HF_DiseaseInc, HF_Ancestor, HF_Confucian, HF_Buddhist
};

using BuildingRow = std::array<int, building_field_count>;
using HouseRow = std::array<int, house_field_count>;

struct ModelData {
    // Indexed by building type; types absent from the file stay all-zero.
    std::vector<BuildingRow> buildings;
    std::vector<bool> building_present;
    std::vector<BuildingRow> building_mods;   // percent, per difficulty
    std::vector<HouseRow> house_mods;         // additive, per difficulty
    std::vector<HouseRow> houses;             // base values, 15 rows

    // Building value after the difficulty percentage (base * pct / 100).
    int building_value(int type, int field, int difficulty) const;
    // House value after the additive modifier, clamped to [-99, 100].
    int house_value(int row, int field, int difficulty) const;
    bool has_building(int type) const {
        return type >= 0 && type < static_cast<int>(building_present.size()) && building_present[static_cast<std::size_t>(type)];
    }
};

// Throws std::runtime_error with a readable message on malformed input.
ModelData parse_model_data(std::string_view text);
ModelData load_model_data(const std::filesystem::path& game_directory);

}  // namespace openemperor::original
