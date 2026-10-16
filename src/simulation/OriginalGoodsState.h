#pragma once

#include "simulation/original/GoodsRules.h"

#include <array>
#include <cstdint>
#include <vector>

namespace openemperor::simulation {

enum class BuildingId : std::uint32_t;
enum class CourierId : std::uint32_t;

struct OriginalGoodsTotals {
    std::uint64_t produced=0,imported=0,consumed=0,demolished=0;
    bool operator==(const OriginalGoodsTotals&) const = default;
};

// Non-house accounts only. HouseState's six signed words remain the sole
// household stock authority. Capacities are caller context, never save grants.
struct OriginalGoodsStock {
    BuildingId building_id=static_cast<BuildingId>(0);
    std::array<std::int32_t,original::goods_count> amounts{};
    bool operator==(const OriginalGoodsStock&) const = default;
};

struct OriginalGoodsTransferKey {
    CourierId courier_id=static_cast<CourierId>(0);
    std::uint64_t sequence=0;
    bool operator==(const OriginalGoodsTransferKey&) const = default;
};

enum class OriginalGoodsTransferPhase : std::uint8_t {
    Reserved=0, Loaded=1, Returning=2, Completed=3, Cancelled=4, Returned=5
};

struct OriginalGoodsCargo {
    OriginalGoodsTransferKey transfer;
    original::Goods good=original::Goods::Hemp;
    BuildingId source=static_cast<BuildingId>(0),target=static_cast<BuildingId>(0);
    std::int32_t amount=0;
    bool operator==(const OriginalGoodsCargo&) const = default;
};

// Terminal rows are retained as transaction history. The caller supplies a
// monotonically increasing per-courier sequence, so stale arrivals/cancels
// cannot book against a later trip. No BuildingId/CourierId is allocated here.
struct OriginalGoodsReservation {
    OriginalGoodsTransferKey transfer;
    original::Goods good=original::Goods::Hemp;
    BuildingId source=static_cast<BuildingId>(0),target=static_cast<BuildingId>(0);
    OriginalGoodsTransferPhase phase=OriginalGoodsTransferPhase::Reserved;
    std::int32_t reserved=0,loaded=0,delivered=0,returned=0;
    bool operator==(const OriginalGoodsReservation&) const = default;
};

struct OriginalGoodsState {
    std::array<OriginalGoodsTotals,original::goods_count> totals{};
    std::vector<OriginalGoodsStock> stocks;
    std::vector<OriginalGoodsCargo> cargo;
    std::vector<OriginalGoodsReservation> reservations;
    bool operator==(const OriginalGoodsState&) const = default;
};

struct OriginalGoodsBalance {
    original::Goods good=original::Goods::Hemp;
    bool supported=false; // Runtime capability only; never serialized.
    OriginalGoodsTotals totals;
    std::uint64_t stock=0,cargo=0,source_reserved=0,incoming_reserved=0;
    bool conserved=true;
    bool operator==(const OriginalGoodsBalance&) const = default;
};

} // namespace openemperor::simulation
