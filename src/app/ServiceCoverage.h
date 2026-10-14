#pragma once

#include "simulation/World.h"
#include <limits>
#include <optional>

namespace openemperor::sandbox_ui {

enum class ServiceCoverageState { Covered, ExpiringByDemand, Uncovered };
struct ServiceCoverage {
    ServiceCoverageState state=ServiceCoverageState::Uncovered;
    bool active=false;
    std::uint64_t remaining=0, demand_remaining=0;
    std::optional<std::uint64_t> next_demand;
    bool valid_at_next_demand=false;
};

// Current coverage only: a future visit is unknown, never predicted here.
inline ServiceCoverage service_coverage(std::uint64_t tick,std::uint64_t until,int progress) {
    ServiceCoverage result;
    result.active=until>tick;
    result.remaining=result.active ? until-tick:0;
    if (progress>=0 && progress<simulation::Rules::household_demand_ticks) {
        result.demand_remaining=static_cast<std::uint64_t>(simulation::Rules::household_demand_ticks-progress);
        if (result.demand_remaining<=std::numeric_limits<std::uint64_t>::max()-tick)
            result.next_demand=tick+result.demand_remaining;
    }
    result.valid_at_next_demand=result.next_demand && until>*result.next_demand;
    result.state=!result.active ? ServiceCoverageState::Uncovered:
        result.valid_at_next_demand ? ServiceCoverageState::Covered:ServiceCoverageState::ExpiringByDemand;
    return result;
}

} // namespace openemperor::sandbox_ui
