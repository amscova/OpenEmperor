#pragma once

#include "simulation/original/HouseEvolution.h"

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace openemperor::simulation {

enum class BuildingId : std::uint32_t;
inline constexpr std::uint64_t original_tax_interval_ticks=816;
inline constexpr std::uint64_t original_worker_upkeep_interval_ticks=400;
inline constexpr std::int64_t original_worker_upkeep_funds=2;
struct OriginalTaxBucket {
    std::uint64_t events=0,resident_units=0;
    bool operator==(const OriginalTaxBucket&) const = default;
};
struct OriginalTaxAssessment {
    std::uint64_t tick=0;
    original::HouseType type=original::HouseType::Vacant;
    std::int16_t occupants=0;
    bool operator==(const OriginalTaxAssessment&) const = default;
};
struct OriginalFinanceHouse {
    BuildingId building_id=static_cast<BuildingId>(0);
    std::array<OriginalTaxBucket,original::house_row_count> rows{};
    std::optional<OriginalTaxAssessment> last_assessment;
    bool operator==(const OriginalFinanceHouse&) const = default;
};
struct OriginalFinanceState {
    std::vector<OriginalFinanceHouse> houses;
    bool operator==(const OriginalFinanceState&) const = default;
};
// Pure transaction arithmetic; this value is never separate saved authority.
struct OriginalFinanceBooking {
    std::int64_t funds=0;
    std::uint64_t taxes=0,maintenance=0;
    bool operator==(const OriginalFinanceBooking&) const = default;
};
[[nodiscard]] OriginalFinanceBooking original_finance_book(std::int64_t funds,
    std::uint64_t taxes,std::uint64_t maintenance,std::uint64_t income,std::uint64_t bill);
[[nodiscard]] bool original_finance_accounting_valid(std::int64_t funds,
    std::uint64_t taxes,std::uint64_t construction,std::uint64_t maintenance) noexcept;

struct OriginalHouseTaxStatus {
    bool applicable=false,inhabited=false,burning=false,eligible=false;
    int occupants=0,rate=0;
    std::uint64_t amount_if_assessed_now=0,paid_total=0;
    std::optional<std::uint64_t> next_assessment_tick;
    std::optional<OriginalTaxAssessment> last_assessment;
};
struct OriginalGoalStatus {
    static constexpr int required_inhabited_houses=6,required_common_houses=1,required_population=50;
    bool applicable=false,reached=false,positive_tax=false,nonnegative_funds=false;
    int inhabited_houses=0,common_houses=0,population=0;
};

} // namespace openemperor::simulation
