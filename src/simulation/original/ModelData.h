#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <string_view>

namespace openemperor::original {

inline constexpr std::size_t building_field_count = 13;
inline constexpr std::size_t house_field_count = 24;
inline constexpr std::size_t house_row_count = 15;
inline constexpr std::size_t difficulty_count = 5;
inline constexpr std::size_t max_model_bytes = 256 * 1024;
inline constexpr std::size_t max_model_line_bytes = 4096;
inline constexpr std::uint16_t max_building_id = 4096;

// These names identify file columns, not World rules or units.
enum class BuildingField : std::size_t {
    Cost = 0, Desirability = 1, DesirabilityStep = 2, DesirabilityStepSize = 3,
    DesirabilityRange = 4, Employees = 5, FireRisk = 6, DamageRisk = 7,
    Resource = 8, RiskReducer = 9, EvolveDesirability = 10, Integrity = 11,
    FengShui = 12
};

enum class HouseField : std::size_t {
    DesirabilityLow = 0, DesirabilityHigh = 1, Herbalist = 2, Water = 3,
    Acupuncture = 4, Music = 5, Acrobat = 6, Drama = 7, FoodQuality = 8,
    Hemp = 9, Ceramics = 10, Tea = 11, Silk = 12, BronzewareOrLacquerware = 13,
    CrimeIncrement = 14, CrimeBase = 15, Unused16 = 16, Capacity = 17,
    TaxRate = 18, Unused19 = 19, DiseaseIncrement = 20, Ancestor = 21,
    Confucian = 22, DaoistOrBuddhist = 23
};

struct ModelBuildingId { std::uint16_t value; };
struct HouseRowId { std::uint8_t value; };
enum class Difficulty : std::size_t { VeryEasy = 0, Easy = 1, Normal = 2, Hard = 3, VeryHard = 4 };

using BuildingRow = std::array<std::int32_t, building_field_count>;
using HouseRow = std::array<std::int32_t, house_field_count>;

enum class ModelSourceKind { Memory, File };

struct ModelProvenance {
    std::uint32_t format_version = 1;
    ModelSourceKind source_kind = ModelSourceKind::Memory;
    std::string source_path;
    std::string sha256;
    std::size_t byte_count = 0;
    std::size_t building_count = 0;
    std::size_t house_count = 0;
    std::size_t building_modifier_count = 0;
    std::size_t house_modifier_count = 0;
};

enum class ModelErrorCode {
    EmptyInput, InputTooLarge, LineTooLong, InvalidCharacter, InvalidSection,
    UnexpectedText, InvalidRow, InvalidNumber, FieldCount, DuplicateBuilding,
    DuplicateHouse, RowCount, MissingEndMarker, InvalidPath, MissingFile,
    NotRegularFile, FileTooLarge, IoFailure, HashFailure
};

struct ModelError {
    ModelErrorCode code;
    std::size_t line = 0; // Zero identifies an input/path error without a line.
    std::string section;
    std::string reason;
};

struct ModelDataParser;

// Created only by a successful parse. No absent row has a default model value.
class ModelData {
public:
    [[nodiscard]] bool has_building(ModelBuildingId id) const noexcept;
    [[nodiscard]] std::size_t building_count() const noexcept;
    [[nodiscard]] std::int32_t building(ModelBuildingId id, BuildingField field) const;
    [[nodiscard]] std::int32_t house(HouseRowId row, HouseField field) const;
    [[nodiscard]] std::int32_t building_modifier(Difficulty difficulty, BuildingField field) const;
    [[nodiscard]] std::int32_t house_modifier(Difficulty difficulty, HouseField field) const;
    [[nodiscard]] std::string_view building_label(ModelBuildingId id) const;
    [[nodiscard]] std::string_view house_label(HouseRowId row) const;
    [[nodiscard]] const ModelProvenance& provenance() const noexcept;

private:
    ModelData() = default;
    friend struct ModelDataParser;

    struct BuildingRecord { BuildingRow values; std::string label; };
    std::map<std::uint16_t, BuildingRecord> buildings_;
    std::array<HouseRow, house_row_count> houses_{};
    std::array<std::string, house_row_count> house_labels_;
    std::array<BuildingRow, difficulty_count> building_modifiers_{};
    std::array<HouseRow, difficulty_count> house_modifiers_{};
    ModelProvenance provenance_;
};

struct ModelResult {
    std::optional<ModelData> model;
    std::optional<ModelError> error;
    [[nodiscard]] explicit operator bool() const noexcept { return model.has_value(); }
};

// Raw signed values only. Difficulty arithmetic and semantic validation belong
// to the later original-rule contract, not this isolated reader.
[[nodiscard]] ModelResult parse_model_data(std::string_view bytes);
[[nodiscard]] ModelResult load_model_data(const std::filesystem::path& game_root);

} // namespace openemperor::original
