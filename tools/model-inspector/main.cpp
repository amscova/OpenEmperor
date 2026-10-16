#include "simulation/original/ModelData.h"

#include <nlohmann/json.hpp>

#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: openemperor-model-inspect GAME_DIRECTORY\n";
        return 2;
    }
    const auto result = openemperor::original::load_model_data(argv[1]);
    if (!result) {
        const auto& error = *result.error;
        std::cerr << nlohmann::json{{"status", "error"}, {"line", error.line},
            {"section", error.section}, {"reason", error.reason}}.dump(2) << '\n';
        return 1;
    }
    const auto& p = result.model->provenance();
    std::cout << nlohmann::json{
        {"status", "validated"}, {"evidence", "RAW MODEL FACT"},
        {"reader_format_version", p.format_version}, {"source_kind", "file"},
        {"source_path", p.source_path}, {"sha256", p.sha256},
        {"byte_count", p.byte_count}, {"building_rows", p.building_count},
        {"house_rows", p.house_count},
        {"building_modifier_rows", p.building_modifier_count},
        {"house_modifier_rows", p.house_modifier_count},
        {"building_fields", openemperor::original::building_field_count},
        {"house_fields", openemperor::original::house_field_count}
    }.dump(2) << '\n';
    return 0;
}
