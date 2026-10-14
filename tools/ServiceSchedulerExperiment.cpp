#include "ServiceSchedulerExperiment.h"
#include "app/ServiceCoverage.h"
#include <algorithm>
#include <stdexcept>
#include <tuple>

namespace openemperor::service_experiment {
namespace {
enum class Strategy { Current, Expiry, Risk, Travel };
Strategy strategy=Strategy::Current;
}
void configure(std::string_view name) {
    if (name=="current") strategy=Strategy::Current;
    else if (name=="expiry") strategy=Strategy::Expiry;
    else if (name=="risk") strategy=Strategy::Risk;
    else if (name=="travel") strategy=Strategy::Travel;
    else throw std::runtime_error("unknown experimental Service scheduler");
}
bool enabled() { return strategy!=Strategy::Current; }
std::optional<simulation::BuildingId> choose(const simulation::World& w,
    const simulation::CourierState& courier,std::span<const Candidate> candidates) {
    if (candidates.empty()) return std::nullopt;
    if (strategy==Strategy::Current) throw std::logic_error("CURRENT must use the production selector");
    const Candidate* best=nullptr;
    auto rank=[&](const Candidate& c) {
        const auto& h=w.building(c.id);
        const auto coverage=sandbox_ui::service_coverage(w.ticks(),h.service_until_tick,h.demand_progress);
        const auto deadline=coverage.next_demand.value_or(UINT64_MAX);
        const auto age=w.ticks()>h.service_until_tick ? w.ticks()-h.service_until_tick:0;
        const auto travel=static_cast<std::uint64_t>(c.edges)*static_cast<unsigned>(w.active_rules().courier_edge_ticks);
        // Exact cached edge cost, not a binding arrival forecast. The travel
        // experiment avoids simultaneous duplicate outbound visits and gives
        // >=1200-tick uncovered age precedence over shorter journeys.
        const std::int64_t cost=static_cast<std::int64_t>(4*travel)-
            static_cast<std::int64_t>(std::min<std::uint64_t>(age,1200));
        if (strategy==Strategy::Expiry)
            return std::tuple{0,std::uint64_t{0},std::uint64_t{0},std::int64_t{0},h.service_until_tick,c.id};
        if (strategy==Strategy::Risk)
            return std::tuple{coverage.valid_at_next_demand ? 1:0,deadline,std::uint64_t{0},std::int64_t{0},h.service_until_tick,c.id};
        return std::tuple{age>=1200 ? 0:coverage.valid_at_next_demand ? 2:1,
            age>=1200 ? h.service_until_tick:std::uint64_t{0},std::uint64_t{0},cost,h.service_until_tick,c.id};
    };
    for (const auto& candidate:candidates) {
        if (strategy==Strategy::Travel && std::any_of(w.couriers().begin(),w.couriers().end(),[&](const auto& c) {
            return c.id!=courier.id && c.enabled && c.role==simulation::CourierRole::Service &&
                c.phase==simulation::CourierPhase::ToWarehouse && c.target==candidate.id;
        })) continue;
        if (!best || rank(candidate)<rank(*best)) best=&candidate;
    }
    return best ? std::optional{best->id}:std::nullopt;
}
} // namespace openemperor::service_experiment
