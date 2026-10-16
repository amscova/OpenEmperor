#include "simulation/original/ModelData.h"

#include <charconv>
#include <fstream>
#include <memory>
#include <set>
#include <stdexcept>
#include <utility>

#include <openssl/evp.h>

namespace openemperor::original {
namespace {

enum class Section : std::size_t { Prologue, BuildingMods, Buildings, HouseMods, Houses, End };

constexpr std::array<std::string_view, 4> section_names{
    "BUILDING MODS", "ALL BUILDINGS", "HOUSE MODS", "ALL HOUSES"};
constexpr std::array<std::string_view, difficulty_count> difficulty_labels{
    "Very Easy Modifiers", "Easy Modifiers", "Normal Modifier", "Hard Modifiers", "Very Hard Modifiers"};
constexpr std::array<std::string_view, 2> building_legends{
    "a   b   c   d   e   f   g   h   i   j", "CST DES STP SZE RGE EMP FRI DRI RES RRD"};
constexpr std::string_view end_marker = "End of model data.";
constexpr std::string_view end_separator = "-------------------";

std::string_view trim(std::string_view value) {
    const auto first = value.find_first_not_of(" \t");
    if (first == std::string_view::npos) return {};
    const auto last = value.find_last_not_of(" \t");
    return value.substr(first, last - first + 1);
}

std::string_view name_for(Section section) {
    const auto index = static_cast<std::size_t>(section);
    if (index >= 1 && index <= section_names.size()) return section_names[index - 1];
    return section == Section::End ? "END" : "PROLOGUE";
}

ModelResult failure(ModelErrorCode code, std::size_t line, Section section, std::string reason) {
    return {std::nullopt, ModelError{code, line, std::string(name_for(section)), std::move(reason)}};
}

bool signed_number(std::string_view token, std::int32_t& result) {
    token = trim(token);
    if (token.empty()) return false;
    if (token.front() == '+') {
        token.remove_prefix(1);
        if (token.empty() || token.front() < '0' || token.front() > '9') return false;
    }
    const auto parsed = std::from_chars(token.data(), token.data() + token.size(), result, 10);
    return parsed.ec == std::errc{} && parsed.ptr == token.data() + token.size();
}

bool padding(std::string_view value) {
    return value.find_first_not_of(" ,\t") == std::string_view::npos;
}

std::optional<std::string> sha256(std::string_view bytes) {
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> context(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    std::array<unsigned char, EVP_MAX_MD_SIZE> digest{};
    unsigned int length = 0;
    if (!context || EVP_DigestInit_ex(context.get(), EVP_sha256(), nullptr) != 1 ||
        EVP_DigestUpdate(context.get(), bytes.data(), bytes.size()) != 1 ||
        EVP_DigestFinal_ex(context.get(), digest.data(), &length) != 1 || length != 32) return std::nullopt;
    constexpr std::string_view digits = "0123456789abcdef";
    std::string result;
    result.reserve(64);
    for (unsigned int index = 0; index < length; ++index) {
        result.push_back(digits[digest[index] >> 4]);
        result.push_back(digits[digest[index] & 15]);
    }
    return result;
}

bool contained_in(const std::filesystem::path& root, const std::filesystem::path& file) {
    auto root_part = root.begin();
    auto file_part = file.begin();
    for (; root_part != root.end(); ++root_part, ++file_part) {
        if (file_part == file.end() || *root_part != *file_part) return false;
    }
    return file_part != file.end();
}

} // namespace

struct ModelDataParser {
    ModelData data;
    Section section = Section::Prologue;
    std::size_t line_number = 0;
    std::size_t building_mod_count = 0;
    std::size_t house_mod_count = 0;
    std::size_t house_count = 0;
    std::set<std::string> house_labels;
    std::array<bool, building_legends.size()> seen_legends{};
    bool seen_end_separator = false;

    ModelResult error(ModelErrorCode code, std::string reason) const {
        return failure(code, line_number, section, std::move(reason));
    }

    std::optional<ModelResult> complete_section() const {
        switch (section) {
        case Section::BuildingMods:
            if (building_mod_count != difficulty_count) return error(ModelErrorCode::RowCount, "expected exactly 5 building modifier rows");
            break;
        case Section::Buildings:
            if (data.buildings_.empty()) return error(ModelErrorCode::RowCount, "expected at least one building row");
            break;
        case Section::HouseMods:
            if (house_mod_count != difficulty_count) return error(ModelErrorCode::RowCount, "expected exactly 5 house modifier rows");
            break;
        case Section::Houses:
            if (house_count != house_row_count) return error(ModelErrorCode::RowCount, "expected exactly 15 house rows");
            break;
        case Section::Prologue:
        case Section::End:
            break;
        }
        return std::nullopt;
    }

    template<std::size_t Count>
    std::optional<ModelResult> values(std::string_view body, std::array<std::int32_t, Count>& row) const {
        std::size_t count = 0;
        std::size_t token_index = 0;
        bool trailing_empty = false;
        for (;;) {
            const auto comma = body.find(',');
            const auto token = trim(body.substr(0, comma));
            if (token.empty()) {
                // A single empty cell directly after '{' is the original CSV framing.
                if (token_index != 0) trailing_empty = true;
            } else {
                if (trailing_empty) return error(ModelErrorCode::FieldCount, "empty interior model field");
                if (count == Count) return error(ModelErrorCode::FieldCount, "too many model fields");
                if (!signed_number(token, row[count])) return error(ModelErrorCode::InvalidNumber, "model field is not a signed 32-bit decimal integer");
                ++count;
            }
            ++token_index;
            if (comma == std::string_view::npos) break;
            body.remove_prefix(comma + 1);
        }
        if (count != Count) return error(ModelErrorCode::FieldCount, "incorrect model field count");
        return std::nullopt;
    }

    std::optional<ModelResult> row(std::string_view line) {
        const auto open = line.find('{');
        if (open == std::string_view::npos || line.find('{', open + 1) != std::string_view::npos)
            return error(ModelErrorCode::InvalidRow, "expected exactly one opening model brace");
        auto prefix = trim(line.substr(0, open));
        if (prefix.empty() || prefix.back() != ',') return error(ModelErrorCode::InvalidRow, "expected label comma before model brace");
        prefix = trim(prefix.substr(0, prefix.size() - 1));
        auto body = line.substr(open + 1);
        const auto close = body.find('}');
        if (close == std::string_view::npos) {
            if (section == Section::BuildingMods || section == Section::Buildings)
                return error(ModelErrorCode::InvalidRow, "building row requires a closing model brace");
        } else {
            if (!padding(body.substr(close + 1))) return error(ModelErrorCode::InvalidRow, "unexpected text after closing model brace");
            body = body.substr(0, close);
        }
        if (prefix.find_first_of("{}") != std::string_view::npos)
            return error(ModelErrorCode::InvalidRow, "invalid model row prefix");

        if (section == Section::BuildingMods || section == Section::Buildings) {
            const auto comma = prefix.find(',');
            std::int32_t id = 0;
            if (comma == std::string_view::npos || !signed_number(prefix.substr(0, comma), id) || id < 0 || id > max_building_id)
                return error(ModelErrorCode::InvalidRow, "building ID must be in 0..4096");
            const auto label = trim(prefix.substr(comma + 1));
            if (label.empty() || label.find(',') != std::string_view::npos)
                return error(ModelErrorCode::InvalidRow, "expected one nonempty building label");
            BuildingRow parsed{};
            if (auto result = values(body, parsed)) return result;
            if (section == Section::BuildingMods) {
                if (building_mod_count == difficulty_count) return error(ModelErrorCode::RowCount, "too many building modifier rows");
                if (static_cast<std::size_t>(id) != building_mod_count || label != difficulty_labels[building_mod_count])
                    return error(ModelErrorCode::InvalidRow, "building modifier ID and label must match the ordered difficulty");
                data.building_modifiers_[building_mod_count++] = parsed;
            } else {
                if (!data.buildings_.emplace(static_cast<std::uint16_t>(id),
                        ModelData::BuildingRecord{parsed, std::string(label)}).second)
                    return error(ModelErrorCode::DuplicateBuilding, "duplicate building ID");
            }
            return std::nullopt;
        }

        if (prefix.empty() || prefix.find(',') != std::string_view::npos)
            return error(ModelErrorCode::InvalidRow, "expected one nonempty house label");
        HouseRow parsed{};
        if (auto result = values(body, parsed)) return result;
        if (section == Section::HouseMods) {
            if (house_mod_count == difficulty_count) return error(ModelErrorCode::RowCount, "too many house modifier rows");
            if (prefix != difficulty_labels[house_mod_count])
                return error(ModelErrorCode::InvalidRow, "house modifier label must match the ordered difficulty");
            data.house_modifiers_[house_mod_count++] = parsed;
        } else {
            if (house_count == house_row_count) return error(ModelErrorCode::RowCount, "too many house rows");
            if (!house_labels.emplace(prefix).second) return error(ModelErrorCode::DuplicateHouse, "duplicate house label");
            data.house_labels_[house_count] = std::string(prefix);
            data.houses_[house_count++] = parsed;
        }
        return std::nullopt;
    }

    std::optional<ModelResult> line(std::string_view text) {
        text = trim(text);
        if (text.empty()) return std::nullopt;
        for (std::size_t index = 0; index < section_names.size(); ++index) {
            if (text != section_names[index]) continue;
            const auto next = static_cast<Section>(index + 1);
            if (static_cast<std::size_t>(next) != static_cast<std::size_t>(section) + 1)
                return error(ModelErrorCode::InvalidSection, "duplicate or out-of-order model section");
            if (auto result = complete_section()) return result;
            section = next;
            return std::nullopt;
        }
        if (text == end_marker) {
            if (section != Section::Houses) return error(ModelErrorCode::InvalidSection, "end marker before all model sections");
            if (auto result = complete_section()) return result;
            section = Section::End;
            return std::nullopt;
        }
        if (section == Section::End) {
            if (text == end_separator && !seen_end_separator) {
                seen_end_separator = true;
                return std::nullopt;
            }
            return error(ModelErrorCode::UnexpectedText, "unexpected text after end marker");
        }
        // The explanatory prologue itself contains braces; it is not model data.
        if (section == Section::Prologue) return std::nullopt;
        if (section == Section::Buildings) {
            for (std::size_t index = 0; index < building_legends.size(); ++index) {
                if (text != building_legends[index]) continue;
                if (seen_legends[index] || !data.buildings_.empty())
                    return error(ModelErrorCode::UnexpectedText, "duplicate or misplaced building column legend");
                seen_legends[index] = true;
                return std::nullopt;
            }
        }
        if (text.find('{') == std::string_view::npos)
            return error(ModelErrorCode::UnexpectedText, "expected model row or next section");
        return row(text);
    }

    static ModelResult parse(std::string_view bytes) {
        if (bytes.empty()) return failure(ModelErrorCode::EmptyInput, 0, Section::Prologue, "model input is empty");
        if (bytes.size() > max_model_bytes) return failure(ModelErrorCode::InputTooLarge, 0, Section::Prologue, "model input exceeds 256 KiB");
        ModelDataParser parser;
        std::size_t start = 0;
        while (start < bytes.size()) {
            ++parser.line_number;
            const auto newline = bytes.find('\n', start);
            const auto end = newline == std::string_view::npos ? bytes.size() : newline;
            auto text = bytes.substr(start, end - start);
            if (newline != std::string_view::npos && !text.empty() && text.back() == '\r') text.remove_suffix(1);
            if (text.size() > max_model_line_bytes) return parser.error(ModelErrorCode::LineTooLong, "model line exceeds 4096 bytes");
            for (const char raw_character : text) {
                const auto character = static_cast<unsigned char>(raw_character);
                if ((character < 32 && character != '\t') || character == 127)
                    return parser.error(ModelErrorCode::InvalidCharacter, "unexpected control byte or lone carriage return");
            }
            if (auto result = parser.line(text)) return std::move(*result);
            if (newline == std::string_view::npos) break;
            start = newline + 1;
        }
        if (parser.section != Section::End) return parser.error(ModelErrorCode::MissingEndMarker, "missing complete model end marker");
        const auto digest = sha256(bytes);
        if (!digest) return parser.error(ModelErrorCode::HashFailure, "SHA-256 calculation failed");
        parser.data.provenance_.sha256 = *digest;
        parser.data.provenance_.byte_count = bytes.size();
        parser.data.provenance_.building_count = parser.data.buildings_.size();
        parser.data.provenance_.house_count = parser.house_count;
        parser.data.provenance_.building_modifier_count = parser.building_mod_count;
        parser.data.provenance_.house_modifier_count = parser.house_mod_count;
        return {std::move(parser.data), std::nullopt};
    }

    static void file_source(ModelData& model, const std::filesystem::path& path) {
        model.provenance_.source_kind = ModelSourceKind::File;
        model.provenance_.source_path = path.generic_string();
    }
};

bool ModelData::has_building(ModelBuildingId id) const noexcept { return buildings_.contains(id.value); }
std::size_t ModelData::building_count() const noexcept { return buildings_.size(); }

std::int32_t ModelData::building(ModelBuildingId id, BuildingField field) const {
    return buildings_.at(id.value).values.at(static_cast<std::size_t>(field));
}

std::int32_t ModelData::house(HouseRowId row, HouseField field) const {
    return houses_.at(row.value).at(static_cast<std::size_t>(field));
}

std::int32_t ModelData::building_modifier(Difficulty difficulty, BuildingField field) const {
    return building_modifiers_.at(static_cast<std::size_t>(difficulty)).at(static_cast<std::size_t>(field));
}

std::int32_t ModelData::house_modifier(Difficulty difficulty, HouseField field) const {
    return house_modifiers_.at(static_cast<std::size_t>(difficulty)).at(static_cast<std::size_t>(field));
}

const ModelProvenance& ModelData::provenance() const noexcept { return provenance_; }

std::string_view ModelData::building_label(ModelBuildingId id) const { return buildings_.at(id.value).label; }
std::string_view ModelData::house_label(HouseRowId row) const { return house_labels_.at(row.value); }

ModelResult parse_model_data(std::string_view bytes) { return ModelDataParser::parse(bytes); }

ModelResult load_model_data(const std::filesystem::path& game_root) {
    namespace fs = std::filesystem;
    const auto path_error = [](ModelErrorCode code, std::string reason) {
        return failure(code, 0, Section::Prologue, std::move(reason));
    };
    if (game_root.empty() || game_root.native().find('\0') != fs::path::string_type::npos)
        return path_error(ModelErrorCode::InvalidPath, "game root is empty or contains a null byte");
    std::error_code error;
    const auto root = fs::canonical(game_root, error);
    if (error || !fs::is_directory(root, error) || error)
        return path_error(ModelErrorCode::InvalidPath, "game root is not an accessible directory");
    const auto candidate = root / "Model" / "EmperorBuildingModels.txt";
    const auto status = fs::status(candidate, error);
    if (error || !fs::exists(status)) {
        if (!fs::exists(status) && (!error || error == std::errc::no_such_file_or_directory))
            return path_error(ModelErrorCode::MissingFile, "Model/EmperorBuildingModels.txt is missing");
        return path_error(ModelErrorCode::IoFailure, "cannot inspect model file");
    }
    if (!fs::is_regular_file(status)) return path_error(ModelErrorCode::NotRegularFile, "model source must be a regular file");
    const auto path = fs::canonical(candidate, error);
    if (error) return path_error(ModelErrorCode::InvalidPath, "cannot resolve model source path");
    if (!contained_in(root, path)) return path_error(ModelErrorCode::InvalidPath, "model source resolves outside game root");
    const auto size = fs::file_size(path, error);
    if (error) return path_error(ModelErrorCode::IoFailure, "cannot determine model file size");
    if (size > max_model_bytes) return path_error(ModelErrorCode::FileTooLarge, "model file exceeds 256 KiB");
    std::ifstream input(path, std::ios::binary);
    if (!input) return path_error(ModelErrorCode::IoFailure, "cannot open model file read-only");
    std::string bytes(static_cast<std::size_t>(size), '\0');
    input.read(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    if (input.gcount() != static_cast<std::streamsize>(bytes.size()) || input.bad())
        return path_error(ModelErrorCode::IoFailure, "model file changed size or could not be read completely");
    if (input.peek() != std::char_traits<char>::eof() || input.bad())
        return path_error(ModelErrorCode::IoFailure, "model file grew or failed during bounded read");
    auto result = parse_model_data(bytes);
    if (result) ModelDataParser::file_source(*result.model, path);
    return result;
}

} // namespace openemperor::original
