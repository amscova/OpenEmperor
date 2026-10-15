#include "simulation/original/ModelData.h"

#include <algorithm>
#include <charconv>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace openemperor::original {
namespace {

enum class Section { None, BuildingMods, Buildings, HouseMods, Houses };

std::string_view trim(std::string_view s) {
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t' || s.front() == '\r')) s.remove_prefix(1);
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r')) s.remove_suffix(1);
    return s;
}

bool parse_int(std::string_view token, int& out) {
    token = trim(token);
    if (token.empty()) return false;
    if (token.front() == '+') token.remove_prefix(1);
    const auto* end = token.data() + token.size();
    const auto result = std::from_chars(token.data(), end, out);
    return result.ec == std::errc() && result.ptr == end;
}

// Numbers following the first '{' up to '}' (or end of line). Empty cells are
// skipped, so trailing ",,,," padding is harmless.
std::vector<int> numbers_in_braces(std::string_view line, int line_no) {
    const auto open = line.find('{');
    auto body = line.substr(open + 1);
    const auto close = body.find('}');
    if (close != std::string_view::npos) body = body.substr(0, close);
    std::vector<int> values;
    while (!body.empty()) {
        const auto comma = body.find(',');
        const auto token = trim(body.substr(0, comma));
        if (!token.empty()) {
            int v = 0;
            if (!parse_int(token, v))
                throw std::runtime_error("model data line " + std::to_string(line_no) +
                                         ": bad number '" + std::string(token) + "'");
            values.push_back(v);
        }
        if (comma == std::string_view::npos) break;
        body.remove_prefix(comma + 1);
    }
    return values;
}

Section section_for(std::string_view line) {
    line = trim(line);
    if (line == "BUILDING MODS") return Section::BuildingMods;
    if (line == "ALL BUILDINGS") return Section::Buildings;
    if (line == "HOUSE MODS") return Section::HouseMods;
    if (line == "ALL HOUSES") return Section::Houses;
    return Section::None;
}

}  // namespace

int ModelData::building_value(int type, int field, int difficulty) const {
    if (!has_building(type) || field < 0 || field >= building_field_count)
        throw std::out_of_range("building_value: bad type or field");
    const int d = std::clamp(difficulty, 0, static_cast<int>(building_mods.size()) - 1);
    const long long pct = building_mods.empty() ? 100 : building_mods[static_cast<std::size_t>(d)][static_cast<std::size_t>(field)];
    return static_cast<int>(buildings[static_cast<std::size_t>(type)][static_cast<std::size_t>(field)] * pct / 100);
}

int ModelData::house_value(int row, int field, int difficulty) const {
    if (row < 0 || row >= static_cast<int>(houses.size()) || field < 0 || field >= house_field_count)
        throw std::out_of_range("house_value: bad row or field");
    const int d = std::clamp(difficulty, 0, static_cast<int>(house_mods.size()) - 1);
    const int add = house_mods.empty() ? 0 : house_mods[static_cast<std::size_t>(d)][static_cast<std::size_t>(field)];
    return std::clamp(houses[static_cast<std::size_t>(row)][static_cast<std::size_t>(field)] + add, -99, 100);
}

ModelData parse_model_data(std::string_view text) {
    ModelData data;
    Section section = Section::None;
    int line_no = 0;
    std::istringstream stream{std::string(text)};
    std::string raw;
    while (std::getline(stream, raw)) {
        ++line_no;
        const std::string_view line = raw;
        if (const auto next = section_for(line); next != Section::None) {
            section = next;
            continue;
        }
        if (trim(line).rfind("End of model data", 0) == 0) section = Section::None;
        if (section == Section::None || line.find('{') == std::string_view::npos) continue;

        auto values = numbers_in_braces(line, line_no);
        switch (section) {
        case Section::BuildingMods:
        case Section::Buildings: {
            if (values.size() != building_field_count)
                throw std::runtime_error("model data line " + std::to_string(line_no) +
                                         ": expected 13 building values, got " + std::to_string(values.size()));
            BuildingRow row{};
            std::copy(values.begin(), values.end(), row.begin());
            if (section == Section::BuildingMods) {
                data.building_mods.push_back(row);
            } else {
                int type = 0;
                const auto comma = line.find(',');
                if (comma == std::string_view::npos || !parse_int(line.substr(0, comma), type) || type < 0 || type > 4096)
                    throw std::runtime_error("model data line " + std::to_string(line_no) + ": bad building type");
                if (data.buildings.size() <= static_cast<std::size_t>(type)) {
                    data.buildings.resize(static_cast<std::size_t>(type) + 1);
                    data.building_present.resize(static_cast<std::size_t>(type) + 1, false);
                }
                data.buildings[static_cast<std::size_t>(type)] = row;
                data.building_present[static_cast<std::size_t>(type)] = true;
            }
            break;
        }
        case Section::HouseMods:
        case Section::Houses: {
            if (values.size() < house_field_count)
                throw std::runtime_error("model data line " + std::to_string(line_no) +
                                         ": expected 24 house values, got " + std::to_string(values.size()));
            HouseRow row{};
            std::copy_n(values.begin(), static_cast<std::size_t>(house_field_count), row.begin());
            (section == Section::HouseMods ? data.house_mods : data.houses).push_back(row);
            break;
        }
        case Section::None:
            break;
        }
    }
    if (data.houses.size() != house_row_count)
        throw std::runtime_error("model data: expected 15 house rows, found " + std::to_string(data.houses.size()));
    if (data.house_mods.size() != difficulty_count)
        throw std::runtime_error("model data: expected 5 house modifier rows, found " + std::to_string(data.house_mods.size()));
    if (data.building_mods.size() != difficulty_count)
        throw std::runtime_error("model data: expected 5 building modifier rows, found " + std::to_string(data.building_mods.size()));
    if (data.buildings.empty())
        throw std::runtime_error("model data: no building rows found");
    return data;
}

ModelData load_model_data(const std::filesystem::path& game_directory) {
    const auto path = game_directory / "Model" / "EmperorBuildingModels.txt";
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("cannot open " + path.string());
    std::ostringstream buffer;
    buffer << in.rdbuf();
    return parse_model_data(buffer.str());
}

}  // namespace openemperor::original
