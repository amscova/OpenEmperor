#include "simulation/original/ModelData.h"

#include <openssl/evp.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace {
namespace fs = std::filesystem;
namespace original = openemperor::original;
using Error = original::ModelErrorCode;
static_assert(!std::is_default_constructible_v<original::ModelData>, "no incomplete default model");

std::size_t checks = 0;

void check(bool condition, const std::string& message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}

std::string sha256(std::string_view bytes) {
    std::array<unsigned char, EVP_MAX_MD_SIZE> digest{};
    unsigned int length = 0;
    if (EVP_Digest(bytes.data(), bytes.size(), digest.data(), &length, EVP_sha256(), nullptr) != 1 || length != 32)
        throw std::runtime_error("test SHA-256 failed");
    constexpr char digits[] = "0123456789abcdef";
    std::string result;
    for (unsigned int index = 0; index < length; ++index) {
        result += digits[digest[index] >> 4];
        result += digits[digest[index] & 15U];
    }
    return result;
}

constexpr std::array<std::string_view, original::difficulty_count> difficulty_names{
    "Very Easy Modifiers", "Easy Modifiers", "Normal Modifier", "Hard Modifiers", "Very Hard Modifiers"
};

// Every label and numeric value is independent of the original model tables.
constexpr std::array<std::string_view, original::house_row_count> house_names{
    "Synthetic A", "Synthetic B", "Synthetic C", "Synthetic D", "Synthetic E",
    "Synthetic F", "Synthetic G", "Synthetic H", "Synthetic I", "Synthetic J",
    "Synthetic K", "Synthetic L", "Synthetic M", "Synthetic N", "Synthetic O"
};

std::int32_t synthetic_value(std::size_t table, std::size_t row, std::size_t field) {
    const auto magnitude = static_cast<std::int32_t>((table + 1) * 997 + row * 53 + field * 11);
    return (field % 3 == 0) ? -magnitude : magnitude;
}

std::string row(std::string prefix, std::size_t count, std::size_t table, std::size_t index, bool closed) {
    prefix += "{,";
    for (std::size_t field = 0; field < count; ++field) {
        if (field != 0) prefix += ',';
        prefix += std::to_string(synthetic_value(table, index, field));
    }
    prefix += closed ? ",},,," : ",,,,,,,";
    return prefix;
}

struct Fixture {
    std::vector<std::string> lines;
    std::array<std::size_t, original::difficulty_count> building_modifiers{};
    std::array<std::size_t, 3> buildings{};
    std::array<std::size_t, original::difficulty_count> house_modifiers{};
    std::array<std::size_t, original::house_row_count> houses{};
    std::array<std::size_t, 4> sections{};
    std::size_t end = 0;

    std::string text(std::string_view ending = "\n", bool terminal_newline = true) const {
        std::string result;
        for (std::size_t index = 0; index < lines.size(); ++index) {
            result += lines[index];
            if (terminal_newline || index + 1 < lines.size()) result += ending;
        }
        return result;
    }
};

Fixture fixture() {
    Fixture result;
    result.lines = {"Synthetic fixture: numbers in braces ( { } ) are data.", "", "Model data starts here:-", "-------------------------"};
    result.sections[0] = result.lines.size();
    result.lines.emplace_back("BUILDING MODS");
    for (std::size_t index = 0; index < original::difficulty_count; ++index) {
        result.building_modifiers[index] = result.lines.size();
        result.lines.push_back(row(std::to_string(index) + ',' + std::string(difficulty_names[index]) + ',',
                                   original::building_field_count, 0, index, true));
    }
    result.sections[1] = result.lines.size();
    result.lines.emplace_back("ALL BUILDINGS");
    constexpr std::array<std::uint16_t, 3> ids{0, 17, original::max_building_id};
    for (std::size_t index = 0; index < ids.size(); ++index) {
        result.buildings[index] = result.lines.size();
        result.lines.push_back(row(std::to_string(ids[index]) + ",Synthetic building,", original::building_field_count, 1, index, true));
    }
    result.sections[2] = result.lines.size();
    result.lines.emplace_back("HOUSE MODS");
    for (std::size_t index = 0; index < original::difficulty_count; ++index) {
        result.house_modifiers[index] = result.lines.size();
        result.lines.push_back(row(std::string(difficulty_names[index]) + ',', original::house_field_count, 2, index, false));
    }
    result.sections[3] = result.lines.size();
    result.lines.emplace_back("ALL HOUSES");
    for (std::size_t index = 0; index < original::house_row_count; ++index) {
        result.houses[index] = result.lines.size();
        result.lines.push_back(row(std::string(house_names[index]) + ',', original::house_field_count, 3, index, false));
    }
    result.end = result.lines.size();
    result.lines.emplace_back("End of model data.");
    result.lines.emplace_back("-------------------");
    return result;
}

void replace_value(std::string& line, std::size_t field, std::string_view replacement) {
    const auto open = line.find("{,");
    if (open == std::string::npos) throw std::runtime_error("invalid synthetic fixture row");
    auto begin = open + 2;
    for (std::size_t index = 0; index < field; ++index) begin = line.find(',', begin) + 1;
    const auto end = line.find(',', begin);
    line.replace(begin, end - begin, replacement);
}

void expect_error(const original::ModelResult& result, const std::string& name,
                  std::optional<Error> code = {}, std::optional<std::size_t> line = {}) {
    check(!static_cast<bool>(result) && !result.model.has_value() && result.error.has_value(), name + ": atomic error result");
    const auto& error = *result.error;
    check(!error.reason.empty(), name + ": readable error");
    if (code) check(error.code == *code, name + ": wrong error code " + std::to_string(static_cast<int>(error.code)));
    if (line) check(error.line == *line, name + ": wrong error line " + std::to_string(error.line));
}

const original::ModelData& expect_success(const original::ModelResult& result, std::string_view bytes,
                                         original::ModelSourceKind kind = original::ModelSourceKind::Memory) {
    check(static_cast<bool>(result) && result.model.has_value() && !result.error.has_value(),
          "expected valid fixture" + (result.error ? ": " + result.error->reason : ""));
    const auto& model = *result.model;
    const auto& provenance = model.provenance();
    check(provenance.format_version == 1, "provenance format version");
    check(provenance.source_kind == kind, "provenance source kind");
    check(provenance.sha256 == sha256(bytes), "SHA-256 binds exact input bytes");
    check(provenance.byte_count == bytes.size(), "provenance byte count");
    check(provenance.building_count == model.building_count() && model.building_count() > 0, "building provenance count");
    check(provenance.house_count == original::house_row_count, "house provenance count");
    check(provenance.building_modifier_count == original::difficulty_count &&
          provenance.house_modifier_count == original::difficulty_count, "modifier provenance counts");
    return model;
}

template <typename Operation> void expect_out_of_range(Operation operation, const std::string& name) {
    try { operation(); }
    catch (const std::out_of_range&) { ++checks; return; }
    throw std::runtime_error(name + ": did not throw std::out_of_range");
}

void positive_and_accessors() {
    check(sha256("abc") == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad", "independent SHA test vector");
    const auto source = fixture();
    const auto bytes = source.text();
    const auto parsed = original::parse_model_data(bytes);
    const auto& model = expect_success(parsed, bytes);
    check(model.provenance().source_path.empty(), "memory provenance has no file path");
    check(model.building_count() == source.buildings.size(), "sparse building rows retained without defaults");
    constexpr std::array<std::uint16_t, 3> ids{0, 17, original::max_building_id};
    for (std::size_t index = 0; index < ids.size(); ++index) {
        check(model.has_building({ids[index]}), "sparse ID present");
        for (std::size_t field = 0; field < original::building_field_count; ++field)
            check(model.building({ids[index]}, static_cast<original::BuildingField>(field)) == synthetic_value(1, index, field), "raw building field");
    }
    for (std::size_t index = 0; index < original::house_row_count; ++index)
        for (std::size_t field = 0; field < original::house_field_count; ++field)
            check(model.house({static_cast<std::uint8_t>(index)}, static_cast<original::HouseField>(field)) == synthetic_value(3, index, field), "raw house field");
    for (std::size_t index = 0; index < original::difficulty_count; ++index) {
        const auto difficulty = static_cast<original::Difficulty>(index);
        for (std::size_t field = 0; field < original::building_field_count; ++field)
            check(model.building_modifier(difficulty, static_cast<original::BuildingField>(field)) == synthetic_value(0, index, field), "raw building modifier");
        for (std::size_t field = 0; field < original::house_field_count; ++field)
            check(model.house_modifier(difficulty, static_cast<original::HouseField>(field)) == synthetic_value(2, index, field), "raw house modifier");
    }
    check(!model.has_building({1}) && !model.has_building({std::numeric_limits<std::uint16_t>::max()}), "absent ID stays absent");
    expect_out_of_range([&] { (void)model.building({1}, original::BuildingField::Cost); }, "absent building");
    expect_out_of_range([&] { (void)model.building({0}, static_cast<original::BuildingField>(original::building_field_count)); }, "building column bound");
    expect_out_of_range([&] { (void)model.house({static_cast<std::uint8_t>(original::house_row_count)}, original::HouseField::Capacity); }, "house row bound");
    expect_out_of_range([&] { (void)model.house({0}, static_cast<original::HouseField>(original::house_field_count)); }, "house column bound");
    expect_out_of_range([&] { (void)model.building_modifier(static_cast<original::Difficulty>(original::difficulty_count), original::BuildingField::Cost); }, "building difficulty not clamped");
    expect_out_of_range([&] { (void)model.house_modifier(static_cast<original::Difficulty>(std::numeric_limits<std::size_t>::max()), original::HouseField::Capacity); }, "house difficulty not clamped");
    expect_out_of_range([&] { (void)model.building_modifier(original::Difficulty::Normal, static_cast<original::BuildingField>(999)); }, "building modifier column");
    expect_out_of_range([&] { (void)model.house_modifier(original::Difficulty::Normal, static_cast<original::HouseField>(999)); }, "house modifier column");

    const auto crlf = source.text("\r\n");
    const auto crlf_parsed = original::parse_model_data(crlf);
    const auto& crlf_model = expect_success(crlf_parsed, crlf);
    check(crlf_model.house({14}, original::HouseField::Capacity) == model.house({14}, original::HouseField::Capacity), "CRLF preserves values");
    check(crlf_model.provenance().sha256 != model.provenance().sha256, "CRLF byte provenance distinct");
    const auto no_final = source.text("\n", false);
    expect_success(original::parse_model_data(no_final), no_final);

    auto minimal = source;
    std::vector<std::size_t> data_lines;
    data_lines.insert(data_lines.end(), source.building_modifiers.begin(), source.building_modifiers.end());
    data_lines.insert(data_lines.end(), source.buildings.begin(), source.buildings.end());
    data_lines.insert(data_lines.end(), source.house_modifiers.begin(), source.house_modifiers.end());
    data_lines.insert(data_lines.end(), source.houses.begin(), source.houses.end());
    for (const auto index : data_lines) {
        auto& line = minimal.lines[index];
        line.erase(line.find("{,") + 1, 1);
        const auto last_value = line.find_last_not_of(",}");
        line.erase(last_value + 1);
        line += '}';
    }
    const auto minimal_bytes = minimal.text();
    const auto minimal_parsed = original::parse_model_data(minimal_bytes);
    const auto& minimal_model = expect_success(minimal_parsed, minimal_bytes);
    check(minimal_model.house({14}, original::HouseField::Capacity) == model.house({14}, original::HouseField::Capacity),
          "minimal rows need no leading or trailing padding; house close is optional");

    auto with_legends = source;
    with_legends.lines.insert(with_legends.lines.begin() + static_cast<std::ptrdiff_t>(source.buildings[0]),
                             {"\t a   b   c   d   e   f   g   h   i   j", " CST DES STP SZE RGE EMP FRI DRI RES RRD"});
    const auto legend_bytes = with_legends.text();
    expect_success(original::parse_model_data(legend_bytes), legend_bytes);

    auto extremes = source;
    replace_value(extremes.lines[extremes.buildings[0]], 0, "-2147483648");
    replace_value(extremes.lines[extremes.buildings[0]], 1, "+2147483647");
    replace_value(extremes.lines[extremes.houses[0]], 0, "+0");
    replace_value(extremes.lines[extremes.houses[0]], 1, "-0");
    replace_value(extremes.lines[extremes.house_modifiers[0]], 0, "-2147483648");
    const auto extreme_bytes = extremes.text();
    const auto extreme_parsed = original::parse_model_data(extreme_bytes);
    const auto& extreme_model = expect_success(extreme_parsed, extreme_bytes);
    check(extreme_model.building({0}, original::BuildingField::Cost) == std::numeric_limits<std::int32_t>::min(), "int32 minimum retained");
    check(extreme_model.building({0}, original::BuildingField::Desirability) == std::numeric_limits<std::int32_t>::max(), "int32 maximum retained");
    check(extreme_model.house({0}, original::HouseField::DesirabilityLow) == 0 &&
          extreme_model.house({0}, original::HouseField::DesirabilityHigh) == 0, "signed zeros");
    check(extreme_model.house_modifier(original::Difficulty::VeryEasy, original::HouseField::DesirabilityLow) == std::numeric_limits<std::int32_t>::min(), "raw modifier has no arithmetic or clamping");
}

void negative_tables() {
    const auto source = fixture();
    const std::array<std::string_view, 15> invalid_numbers{
        "", " ", "+", "-", "--1", "++1", "+-1", "1x", "1.5", "0x10", "1e2", "1 2", "2147483648", "-2147483649", "999999999999999999999999999999999"
    };
    std::size_t numeric_cases = 0;
    for (const auto token : invalid_numbers) {
        for (const auto line_index : {source.buildings[0], source.houses[0], source.building_modifiers[0], source.house_modifiers[0]}) {
            auto changed = source;
            replace_value(changed.lines[line_index], 4, token);
            expect_error(original::parse_model_data(changed.text()), "invalid number " + std::to_string(numeric_cases),
                         token.empty() || token == " " ? std::optional<Error>{Error::FieldCount} : std::optional<Error>{Error::InvalidNumber}, line_index + 1);
            ++numeric_cases;
        }
    }
    struct Mutation { std::string name; std::size_t line; std::string replacement; std::optional<Error> code; };
    const std::vector<Mutation> mutations{
        {"duplicate building", source.buildings[1], row("0,Duplicate,", 13, 1, 1, true), Error::DuplicateBuilding},
        {"duplicate normalized building", source.buildings[1], row("+000,Duplicate,", 13, 1, 1, true), Error::DuplicateBuilding},
        {"negative building ID", source.buildings[0], row("-1,Invalid,", 13, 1, 0, true), Error::InvalidRow},
        {"building ID exceeds bound", source.buildings[0], row(std::to_string(static_cast<unsigned int>(original::max_building_id) + 1U) + ",Invalid,", 13, 1, 0, true), Error::InvalidRow},
        {"missing building ID", source.buildings[0], row(",Invalid,", 13, 1, 0, true), Error::InvalidRow},
        {"ID number junk", source.buildings[0], row("0x1,Invalid,", 13, 1, 0, true), Error::InvalidRow},
        {"missing building label", source.buildings[0], row("0,,", 13, 1, 0, true), Error::InvalidRow},
        {"short building", source.buildings[0], row("0,Synthetic,", 12, 1, 0, true), Error::FieldCount},
        {"extra building", source.buildings[0], row("0,Synthetic,", 14, 1, 0, true), Error::FieldCount},
        {"short house", source.houses[0], row(std::string(house_names[0]) + ',', 23, 3, 0, false), Error::FieldCount},
        {"extra house", source.houses[0], row(std::string(house_names[0]) + ',', 25, 3, 0, false), Error::FieldCount},
        {"duplicate house", source.houses[1], row(std::string(house_names[0]) + ',', 24, 3, 1, false), Error::DuplicateHouse},
        {"empty house label", source.houses[0], row(",", 24, 3, 0, false), Error::InvalidRow},
        {"wrong building difficulty ID", source.building_modifiers[0], row("1,Very Easy Modifiers,", 13, 0, 0, true), Error::InvalidRow},
        {"wrong building difficulty label", source.building_modifiers[0], row("0,Unrecognized difficulty,", 13, 0, 0, true), Error::InvalidRow},
        {"repeated building difficulty", source.building_modifiers[1], source.lines[source.building_modifiers[0]], Error::InvalidRow},
        {"wrong house difficulty label", source.house_modifiers[0], row("Unrecognized difficulty,", 24, 2, 0, false), Error::InvalidRow},
        {"repeated house difficulty", source.house_modifiers[1], source.lines[source.house_modifiers[0]], Error::InvalidRow},
        {"repeated building section", source.sections[1], "BUILDING MODS", Error::InvalidSection},
        {"wrong section order", source.sections[1], "ALL HOUSES", Error::InvalidSection},
        {"unknown section", source.sections[1], "OTHER BUILDINGS", Error::UnexpectedText},
        {"missing brace row", source.buildings[0], "0,Synthetic,", Error::UnexpectedText},
        {"missing building close", source.buildings[0], row("0,Synthetic,", 13, 1, 0, false), Error::InvalidRow},
        {"closing suffix junk", source.buildings[0], row("0,Synthetic,", 13, 1, 0, true) + "junk", Error::InvalidRow},
        {"second end marker", source.end + 1, "End of model data.", Error::InvalidSection}
    };
    for (const auto& mutation : mutations) {
        auto changed = source;
        changed.lines[mutation.line] = mutation.replacement;
        expect_error(original::parse_model_data(changed.text()), mutation.name, mutation.code, mutation.line + 1);
    }
    for (const auto table : {source.building_modifiers.back(), source.house_modifiers.back(), source.houses.back()}) {
        auto changed = source;
        changed.lines.insert(changed.lines.begin() + static_cast<std::ptrdiff_t>(table + 1), source.lines[table]);
        expect_error(original::parse_model_data(changed.text()), "extra fixed row", Error::RowCount, table + 2);
    }
    for (const auto table : {source.building_modifiers, source.house_modifiers}) {
        auto changed = source;
        std::swap(changed.lines[table[0]], changed.lines[table[1]]);
        expect_error(original::parse_model_data(changed.text()), "swapped difficulty rows", Error::InvalidRow, table[0] + 1);
    }
    auto no_buildings = source;
    no_buildings.lines.erase(no_buildings.lines.begin() + static_cast<std::ptrdiff_t>(source.buildings.front()),
                            no_buildings.lines.begin() + static_cast<std::ptrdiff_t>(source.buildings.back() + 1));
    expect_error(original::parse_model_data(no_buildings.text()), "no building rows", Error::RowCount);
    for (const auto index : {source.buildings[0], source.houses[0]}) {
        auto leading_empty = source;
        auto& line = leading_empty.lines[index];
        line.insert(line.find("{,") + 1, 1, ',');
        expect_error(original::parse_model_data(leading_empty.text()), "double leading empty field", Error::FieldCount, index + 1);
        auto nested = source;
        nested.lines[index].insert(nested.lines[index].find('{'), 1, '{');
        expect_error(original::parse_model_data(nested.text()), "nested opening brace", Error::InvalidRow, index + 1);
        auto extra_close = source;
        extra_close.lines[index] += "}}";
        expect_error(original::parse_model_data(extra_close.text()), "extra closing braces", Error::InvalidRow, index + 1);
    }
    for (const std::size_t insertion : {source.buildings.front(), source.buildings.back() + 1}) {
        auto legend = source;
        legend.lines.insert(legend.lines.begin() + static_cast<std::ptrdiff_t>(insertion),
                            "a   b   c   d   e   f   g   h   i   j");
        if (insertion == source.buildings.front())
            legend.lines.insert(legend.lines.begin() + static_cast<std::ptrdiff_t>(insertion),
                                "a   b   c   d   e   f   g   h   i   j");
        expect_error(original::parse_model_data(legend.text()), "duplicate or misplaced legend", Error::UnexpectedText);
    }
    for (const auto section : source.sections) {
        auto changed = source;
        changed.lines.erase(changed.lines.begin() + static_cast<std::ptrdiff_t>(section));
        expect_error(original::parse_model_data(changed.text()), "missing section");
    }
    for (const auto line : {source.buildings[0], source.building_modifiers[0], source.house_modifiers[0], source.houses[0]}) {
        auto changed = source;
        changed.lines.erase(changed.lines.begin() + static_cast<std::ptrdiff_t>(line));
        // Removing an optional sparse Building row is legal; all fixed tables must stay complete.
        if (line == source.buildings[0]) expect_success(original::parse_model_data(changed.text()), changed.text());
        else expect_error(original::parse_model_data(changed.text()), "missing fixed row");
    }
    auto no_end = source;
    no_end.lines.resize(no_end.end);
    expect_error(original::parse_model_data(no_end.text()), "missing end marker", Error::MissingEndMarker);
    auto after_end = source;
    after_end.lines.push_back(source.lines[source.buildings[0]]);
    expect_error(original::parse_model_data(after_end.text()), "row after end marker", Error::UnexpectedText);
    expect_error(original::parse_model_data(""), "empty input", Error::EmptyInput, 0);
    for (const unsigned int byte : {0U, 1U, 127U}) {
        auto bytes = source.text();
        bytes.insert(bytes.find("Synthetic building") + 3, 1, static_cast<char>(byte));
        expect_error(original::parse_model_data(bytes), "invalid character", Error::InvalidCharacter);
    }
    auto bare_cr = source.text();
    bare_cr.insert(bare_cr.find("Synthetic building"), 1, '\r');
    expect_error(original::parse_model_data(bare_cr), "embedded CR", Error::InvalidCharacter);
    std::cout << "negative numeric cases=" << numeric_cases << ", row/section cases=" << mutations.size() << '\n';
}

void bounds_and_fuzz() {
    auto line_bound = fixture();
    auto& line = line_bound.lines[line_bound.buildings[0]];
    check(line.size() < original::max_model_line_bytes, "synthetic row below line bound");
    line.append(original::max_model_line_bytes - line.size(), ',');
    const auto at_line_bound = line_bound.text();
    expect_success(original::parse_model_data(at_line_bound), at_line_bound);
    line += ',';
    expect_error(original::parse_model_data(line_bound.text()), "line bound plus one", Error::LineTooLong, line_bound.buildings[0] + 1);

    auto bytes = fixture().text();
    check(bytes.size() < original::max_model_bytes, "fixture below input bound");
    bytes.append(original::max_model_bytes - bytes.size(), '\n');
    expect_success(original::parse_model_data(bytes), bytes);
    bytes += '\n';
    expect_error(original::parse_model_data(bytes), "input bound plus one", Error::InputTooLarge, 0);

    const auto valid = fixture().text();
    std::uint32_t random = 0x6d2b79f5U;
    auto next = [&] {
        random ^= random << 13;
        random ^= random >> 17;
        random ^= random << 5;
        return random;
    };
    std::size_t accepted = 0, rejected = 0;
    constexpr std::size_t mutation_count = 768;
    for (std::size_t index = 0; index < mutation_count; ++index) {
        auto changed = valid;
        const auto position = static_cast<std::size_t>(next()) % changed.size();
        switch (index % 4) {
        case 0: changed[position] = static_cast<char>(next() & 255U); break;
        case 1: changed.erase(position, 1); break;
        case 2: changed.insert(position, 1, static_cast<char>(next() & 255U)); break;
        default: changed.resize(position); break;
        }
        const auto parsed = original::parse_model_data(changed);
        const auto repeated = original::parse_model_data(changed);
        check(static_cast<bool>(parsed) == static_cast<bool>(repeated), "deterministic fuzz status");
        if (parsed) {
            expect_success(parsed, changed);
            check(parsed.model->provenance().sha256 == repeated.model->provenance().sha256, "deterministic accepted fuzz provenance");
            ++accepted;
        } else {
            expect_error(parsed, "fuzz rejection");
            check(repeated.error.has_value() && parsed.error->code == repeated.error->code &&
                  parsed.error->line == repeated.error->line && parsed.error->section == repeated.error->section &&
                  parsed.error->reason == repeated.error->reason, "deterministic fuzz diagnostics");
            ++rejected;
        }
    }
    check(accepted > 0 && rejected > 0, "fuzz exercises accepted and rejected mutations");
    std::cout << "deterministic mutations=" << mutation_count << ", accepted=" << accepted << ", rejected=" << rejected << '\n';
}

std::string read_bytes(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error("cannot read test file " + path.string());
    std::string bytes{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    if (input.bad()) throw std::runtime_error("test file read failed " + path.string());
    return bytes;
}

void write_bytes(const fs::path& path, std::string_view bytes) {
    fs::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary);
    output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    check(static_cast<bool>(output), "write synthetic file");
}

void file_checks() {
    const auto unique = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto root = fs::temp_directory_path() / ("openemperor-model-data-" + std::to_string(unique));
    check(fs::create_directory(root), "exclusive test directory");
    struct Cleanup { fs::path path; ~Cleanup() { std::error_code ignored; fs::remove_all(path, ignored); } } cleanup{root};
    const auto game = root / "game";
    const auto path = game / "Model" / "EmperorBuildingModels.txt";
    expect_error(original::load_model_data(game), "missing game root", Error::InvalidPath, 0);
    fs::create_directory(game);
    expect_error(original::load_model_data(game), "missing Model directory", Error::MissingFile, 0);
    fs::create_directory(path.parent_path());
    expect_error(original::load_model_data(game), "missing model file", Error::MissingFile, 0);
    const auto bytes = fixture().text("\r\n");
    write_bytes(path, bytes);
    const auto before_time = fs::last_write_time(path);
    const auto before_permissions = fs::status(path).permissions();
    const auto loaded = original::load_model_data(game);
    const auto& model = expect_success(loaded, bytes, original::ModelSourceKind::File);
    check(!model.provenance().source_path.empty() && fs::equivalent(model.provenance().source_path, path), "file provenance exact source path");
    check(read_bytes(path) == bytes && fs::last_write_time(path) == before_time &&
          fs::status(path).permissions() == before_permissions, "successful load preserves source bytes, mtime, permissions");
    const auto parsed = original::parse_model_data(bytes);
    check(parsed.model->provenance().sha256 == model.provenance().sha256, "memory and file provenance agree");

    fs::permissions(path, fs::perms::owner_read, fs::perm_options::replace);
    const auto read_only_permissions = fs::status(path).permissions();
    const auto read_only = original::load_model_data(game);
    expect_success(read_only, bytes, original::ModelSourceKind::File);
    check(read_bytes(path) == bytes && fs::last_write_time(path) == before_time &&
          fs::status(path).permissions() == read_only_permissions, "read-only source permissions and data remain exact");
    fs::permissions(path, before_permissions, fs::perm_options::replace);

    fs::remove(path);
    fs::create_directory(path);
    expect_error(original::load_model_data(game), "directory model file", Error::NotRegularFile, 0);
    fs::remove(path);
    write_bytes(path, std::string(original::max_model_bytes + 1, 'x'));
    expect_error(original::load_model_data(game), "oversized regular file", Error::FileTooLarge, 0);
    write_bytes(path, "damaged model");
    const auto damaged_time = fs::last_write_time(path);
    expect_error(original::load_model_data(game), "existing corrupted model");
    check(read_bytes(path) == "damaged model" && fs::last_write_time(path) == damaged_time, "failed load preserves corrupt source");
    fs::remove(path);
    const auto outside = root / "outside.txt";
    write_bytes(outside, bytes);
    fs::create_symlink(outside, path);
    expect_error(original::load_model_data(game), "model symlink escapes game root", Error::InvalidPath, 0);
    check(read_bytes(outside) == bytes, "escape source remains unchanged");
    fs::remove(path);
    const auto inside = path.parent_path() / "synthetic-inside.txt";
    write_bytes(inside, bytes);
    const auto inside_time = fs::last_write_time(inside);
    fs::create_symlink(inside.filename(), path);
    const auto linked = original::load_model_data(game);
    const auto& linked_model = expect_success(linked, bytes, original::ModelSourceKind::File);
    check(fs::equivalent(linked_model.provenance().source_path, inside), "inside symlink retains canonical source provenance");
    check(read_bytes(inside) == bytes && fs::last_write_time(inside) == inside_time, "inside symlink source stays read-only");
    fs::remove(path);
    fs::create_symlink("missing-synthetic-model.txt", path);
    expect_error(original::load_model_data(game), "dangling model symlink");
    fs::remove(path);
    fs::remove(inside);
    fs::remove(path.parent_path());
    write_bytes(root / "EmperorBuildingModels.txt", bytes);
    fs::create_directory_symlink(root, path.parent_path());
    expect_error(original::load_model_data(game), "Model symlink escapes game root", Error::InvalidPath, 0);
    fs::remove(path.parent_path());
    fs::create_directory_symlink(root / "missing-directory", path.parent_path());
    expect_error(original::load_model_data(game), "dangling Model directory symlink");
    expect_error(original::load_model_data(fs::path{}), "empty game root", Error::InvalidPath, 0);
}

int real_checks(const fs::path& root) {
    check(!root.empty(), "supplied original root cannot be empty");
    std::error_code error;
    const auto root_status = fs::symlink_status(root, error);
    if (root_status.type() == fs::file_type::not_found) {
        std::cout << "SKIP: supplied original game root is missing: " << root << '\n';
        return 77;
    }
    check(!error && fs::is_directory(root), "existing original root must be a readable directory");
    const auto model_directory = root / "Model";
    const auto directory_status = fs::symlink_status(model_directory, error);
    if (directory_status.type() == fs::file_type::not_found) {
        std::cout << "SKIP: original Model directory is missing: " << model_directory << '\n';
        return 77;
    }
    check(!error && fs::is_directory(model_directory), "existing original Model path must be a directory");
    const auto path = model_directory / "EmperorBuildingModels.txt";
    const auto file_status = fs::symlink_status(path, error);
    if (file_status.type() == fs::file_type::not_found) {
        std::cout << "SKIP: original model file is missing: " << path << '\n';
        return 77;
    }
    check(!error && fs::is_regular_file(file_status), "existing original model must be a regular non-symlink file");
    check(fs::file_size(path) <= original::max_model_bytes, "original model file exceeds parser bound");
    const auto modified = fs::last_write_time(path);
    const auto permissions = fs::status(path).permissions();
    // The loader validates canonical containment before this test reads bytes.
    const auto loaded = original::load_model_data(root);
    check(static_cast<bool>(loaded), "existing original model must pass the bounded loader" +
          (loaded.error ? ": " + loaded.error->reason : ""));
    const auto bytes = read_bytes(path);
    const auto& model = expect_success(loaded, bytes, original::ModelSourceKind::File);
    const auto memory = original::parse_model_data(bytes);
    expect_success(memory, bytes);
    for (std::size_t index = 0; index < original::house_row_count; ++index)
        for (std::size_t field = 0; field < original::house_field_count; ++field)
            check(model.house({static_cast<std::uint8_t>(index)}, static_cast<original::HouseField>(field)) ==
                  memory.model->house({static_cast<std::uint8_t>(index)}, static_cast<original::HouseField>(field)), "real file/memory house equality");
    for (std::size_t difficulty = 0; difficulty < original::difficulty_count; ++difficulty) {
        for (std::size_t field = 0; field < original::building_field_count; ++field)
            check(model.building_modifier(static_cast<original::Difficulty>(difficulty), static_cast<original::BuildingField>(field)) ==
                  memory.model->building_modifier(static_cast<original::Difficulty>(difficulty), static_cast<original::BuildingField>(field)), "real building modifiers");
        for (std::size_t field = 0; field < original::house_field_count; ++field)
            check(model.house_modifier(static_cast<original::Difficulty>(difficulty), static_cast<original::HouseField>(field)) ==
                  memory.model->house_modifier(static_cast<original::Difficulty>(difficulty), static_cast<original::HouseField>(field)), "real house modifiers");
    }
    std::size_t building_count = 0;
    for (std::uint32_t id = 0; id <= original::max_building_id; ++id) {
        const original::ModelBuildingId building_id{static_cast<std::uint16_t>(id)};
        check(model.has_building(building_id) == memory.model->has_building(building_id), "real file/memory sparse identity equality");
        if (!model.has_building(building_id)) continue;
        ++building_count;
        for (std::size_t field = 0; field < original::building_field_count; ++field)
            check(model.building(building_id, static_cast<original::BuildingField>(field)) ==
                  memory.model->building(building_id, static_cast<original::BuildingField>(field)), "real file/memory building equality");
    }
    check(building_count == model.building_count(), "real sparse building count agrees with accessible rows");
    check(read_bytes(path) == bytes && fs::last_write_time(path) == modified && fs::status(path).permissions() == permissions,
          "original model remains byte/mtime/permission exact");
    std::cout << "original model: bytes=" << bytes.size() << ", buildings=" << model.building_count()
              << ", SHA-256=" << model.provenance().sha256 << ", checks=" << checks << '\n';
    return 0;
}
} // namespace

int main(int argc, char** argv) {
    try {
        if (argc == 3 && std::string_view(argv[1]) == "--real") return real_checks(argv[2]);
        if (argc != 1) throw std::runtime_error("usage: original-model-data-tests [--real GAME_ROOT]");
        positive_and_accessors();
        negative_tables();
        bounds_and_fuzz();
        file_checks();
        std::cout << "original ModelData synthetic checks PASS: " << checks << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "original ModelData checks FAIL: " << error.what() << '\n';
        return 1;
    }
}
