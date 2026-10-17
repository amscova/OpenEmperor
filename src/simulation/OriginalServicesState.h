#pragma once

#include "simulation/OriginalGoodsState.h"
#include "simulation/original/ServiceRules.h"

#include <cstdint>
#include <optional>
#include <vector>

namespace openemperor::simulation {

// Explicit Rule10 purchase bindings; old generic Posts acquire no identity.
enum class OriginalServiceProviderType : std::uint8_t { Herbalist207=0, Acupuncture208=1 };
struct OriginalServiceProvider {
    BuildingId building_id=static_cast<BuildingId>(0);
    OriginalServiceProviderType type=OriginalServiceProviderType::Herbalist207;
    bool operator==(const OriginalServiceProvider&) const = default;
};
// Latest real trip per owned cargo-free courier, including sequence highwater.
struct OriginalServiceTrip {
    OriginalGoodsTransferKey transfer;
    BuildingId target=static_cast<BuildingId>(0);
    std::uint64_t dispatched_tick=0;
    std::optional<std::uint64_t> arrival_tick=std::nullopt;
    bool visit_applied=false;
    bool operator==(const OriginalServiceTrip&) const = default;
};
// Last successful visit per House/category. HouseState owns the only timer.
struct OriginalServiceWitness {
    BuildingId building_id=static_cast<BuildingId>(0);
    original::ServiceRole role=original::ServiceRole::Herbalist;
    BuildingId provider_id=static_cast<BuildingId>(0);
    OriginalGoodsTransferKey transfer;
    std::uint64_t arrival_tick=0;
    bool operator==(const OriginalServiceWitness&) const = default;
};
struct OriginalServicesState {
    std::vector<OriginalServiceProvider> providers;
    std::vector<OriginalServiceTrip> trips;
    std::vector<OriginalServiceWitness> witnesses;
    bool operator==(const OriginalServicesState&) const = default;
};

} // namespace openemperor::simulation
