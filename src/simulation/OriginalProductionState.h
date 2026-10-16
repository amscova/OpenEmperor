#pragma once

#include <cstdint>

namespace openemperor::simulation {

// Explicit authored working-generation revision. A later stage must opt into
// its own revision; a saved earlier development state is never reinterpreted.
inline constexpr std::uint32_t kOriginalProductionDevelopmentRevision=15;
struct OriginalProductionState {
    std::uint32_t development_revision=kOriginalProductionDevelopmentRevision;
    std::uint64_t residents_admitted=0;
    std::uint64_t pottery_handed_off=0;
    bool operator==(const OriginalProductionState&) const = default;
};

} // namespace openemperor::simulation
