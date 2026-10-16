#pragma once

#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>

namespace openemperor::original {

// Immutable Rule7/Schema23 time interpretation. One World tick per phase and
// the elapsed-calendar epoch are authored, not observed original game speed.
// Changing this mapping requires a new saved rule identity, never reinterpretation.
inline constexpr std::uint32_t scheduler_mapping_version=1;
inline constexpr std::uint8_t scheduler_phase_count=51;
inline constexpr std::uint8_t scheduler_steps_per_month=16;
inline constexpr std::uint8_t scheduler_months_per_year=12;
inline constexpr std::uint32_t scheduler_world_ticks_per_phase=1;
// Keep the existing strict save integer domain; the terminal state is savable.
inline constexpr std::uint64_t scheduler_max_tick=std::numeric_limits<std::uint64_t>::max()-1;

// Position of the NEXT dispatch after this many completed ticks. Month/year
// count elapsed authored periods from zero, not Emperor's historical date.
struct SchedulerPosition {
    std::uint8_t phase=0;
    std::uint8_t step=0;
    std::uint8_t month=0;
    std::uint64_t year=0;
    bool operator==(const SchedulerPosition&) const = default;
};

struct SchedulerDispatch {
    SchedulerPosition position;
    bool service_countdown=false;
    bool evolution=false;
    bool consumption=false; // After evolution, only selectors 0 and 7.
    bool operator==(const SchedulerDispatch&) const = default;
};

constexpr SchedulerPosition scheduler_position(std::uint64_t completed_ticks) noexcept {
    const auto passes=completed_ticks/scheduler_phase_count;
    const auto months=passes/scheduler_steps_per_month;
    return {static_cast<std::uint8_t>(completed_ticks%scheduler_phase_count),
            static_cast<std::uint8_t>(passes%scheduler_steps_per_month),
            static_cast<std::uint8_t>(months%scheduler_months_per_year),
            months/scheduler_months_per_year};
}

// Dispatch describes eligibility in the pipeline; it grants no visit, stock,
// evolution or consumption. Those operations retain their later World gates.
constexpr std::optional<SchedulerDispatch> next_scheduler_dispatch(
        std::uint64_t completed_ticks) noexcept {
    if (completed_ticks>=scheduler_max_tick) return std::nullopt;
    const auto position=scheduler_position(completed_ticks);
    const bool evolution=position.phase==39;
    return SchedulerDispatch{position,position.phase==35,evolution,
        evolution && (position.step==0 || position.step==7)};
}

inline std::uint64_t advance_scheduler_tick(std::uint64_t completed_ticks) {
    if (!next_scheduler_dispatch(completed_ticks))
        throw std::overflow_error("original scheduler tick counter exhausted");
    return completed_ticks+1;
}

} // namespace openemperor::original
