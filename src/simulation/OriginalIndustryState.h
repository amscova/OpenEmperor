#pragma once

#include "simulation/OriginalGoodsState.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace openemperor::simulation {

// Separate production inputs, never extra entries in the six-house-goods enum.
// Clay and Hemp retain their existing sole physical inventory authorities.
enum class OriginalAuxMaterial : std::uint8_t { Wood=0, Bronze=1, RawSilk=2, Lacquer=3, Paper=4 };
inline constexpr std::size_t original_aux_material_count=5;
inline constexpr std::size_t original_industry_source_limit=18;
inline constexpr std::size_t original_aux_owner_limit=20;
inline constexpr std::size_t original_aux_courier_limit=12;
inline constexpr std::size_t original_managed_ore_limit=2;
inline constexpr std::size_t original_clay_trip_limit=4;
using OriginalAuxAmounts=std::array<std::int32_t,original_aux_material_count>;
using OriginalAuxCounters=std::array<std::uint64_t,original_aux_material_count>;
// Derived audit only; never a second stock, clock or serialized authority.
struct OriginalAuxBalance {
    std::uint64_t produced=0,stock=0,cargo=0,consumed=0,source_reserved=0,incoming_reserved=0;
    bool conserved=false;
    bool operator==(const OriginalAuxBalance&) const = default;
};

struct OriginalClayReceipt {
    OriginalGoodsTransferKey transfer;
    BuildingId source=static_cast<BuildingId>(0);
    std::int32_t amount=0;
    std::uint64_t arrival_tick=0;
    bool operator==(const OriginalClayReceipt&) const = default;
};
struct OriginalIndustrySource {
    BuildingId building_id=static_cast<BuildingId>(0);
    std::optional<BuildingId> plant_id;
    std::uint64_t completed=0;
    std::int32_t progress=0;
    // Recipe history only. Inputs are debited together at successful completion.
    std::array<std::uint64_t,original::goods_count> goods_inputs_used{};
    OriginalAuxCounters aux_inputs_used{};
    std::uint64_t clay_received=0,clay_used=0;
    std::optional<OriginalClayReceipt> last_clay_arrival;
    bool operator==(const OriginalIndustrySource&) const = default;
};
struct OriginalManagedOre {
    BuildingId building_id=static_cast<BuildingId>(0);
    std::int32_t stock=0,progress=0;
    std::uint64_t extracted=0,used=0;
    bool operator==(const OriginalManagedOre&) const = default;
};
struct OriginalAuxStock {
    BuildingId building_id=static_cast<BuildingId>(0);
    OriginalAuxAmounts amounts{};
    // Per-owner audit: own production+received+returned=stock+departed+consumed.
    OriginalAuxCounters received{},departed{},returned{},consumed{};
    bool operator==(const OriginalAuxStock&) const = default;
};
struct OriginalAuxTrip {
    OriginalGoodsTransferKey transfer;
    BuildingId source=static_cast<BuildingId>(0),target=static_cast<BuildingId>(0);
    OriginalAuxMaterial material=OriginalAuxMaterial::Wood;
    OriginalGoodsTransferPhase phase=OriginalGoodsTransferPhase::Loaded;
    // loaded-delivered-returned is the sole material cargo authority.
    std::int32_t loaded=0,delivered=0,returned=0;
    std::uint64_t dispatched_tick=0;
    std::optional<std::uint64_t> arrival_tick;
    bool operator==(const OriginalAuxTrip&) const = default;
};
// Latest metadata for every native Clay trip, including ordinary Pottery trips.
// Native Courier.cargo remains the sole Clay cargo, never mirrored here.
struct OriginalClayTrip {
    OriginalGoodsTransferKey transfer;
    BuildingId source=static_cast<BuildingId>(0),target=static_cast<BuildingId>(0);
    std::int32_t loaded=0,delivered=0;
    std::uint64_t dispatched_tick=0;
    std::optional<std::uint64_t> arrival_tick;
    bool operator==(const OriginalClayTrip&) const = default;
};
struct OriginalIndustryState {
    std::vector<OriginalIndustrySource> sources;
    std::vector<OriginalManagedOre> managed_ore;
    std::vector<OriginalAuxStock> auxiliary;
    std::vector<OriginalAuxTrip> aux_trips;
    std::vector<OriginalClayTrip> clay_trips;
    bool operator==(const OriginalIndustryState&) const = default;
};

} // namespace openemperor::simulation
