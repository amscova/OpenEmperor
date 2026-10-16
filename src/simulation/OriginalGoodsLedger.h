#pragma once

#include "simulation/OriginalEconomyState.h"
#include "simulation/OriginalGoodsState.h"

#include <span>

namespace openemperor::simulation {

inline constexpr std::size_t original_goods_owner_limit=46;
inline constexpr std::size_t original_goods_courier_limit=26;
inline constexpr std::size_t original_goods_reservation_limit=26;

// Explicit bounded numerical context, not a claim about an original warehouse
// or a serialized capacity. World capabilities remain responsible for admission.
struct OriginalGoodsOwner {
    BuildingId building_id=static_cast<BuildingId>(0);
    bool household=false;
    std::array<std::int32_t,original::goods_count> capacity{};
};
struct OriginalGoodsLedgerView {
    const OriginalGoodsState& goods;
    std::span<const OriginalHouseState> houses;
    std::span<const OriginalGoodsOwner> owners;
    std::span<const CourierId> couriers;
};
struct OriginalGoodsLedgerSnapshot {
    OriginalGoodsState goods;
    std::vector<OriginalHouseState> houses;
    bool operator==(const OriginalGoodsLedgerSnapshot&) const = default;
    OriginalGoodsLedgerView view(std::span<const OriginalGoodsOwner> owners,
        std::span<const CourierId> couriers) const { return {goods,houses,owners,couriers}; }
};

// Validates all domains/references/commitments without copies or mutation.
// A mathematical balance never declares a supported World capability.
std::array<OriginalGoodsBalance,original::goods_count>
    inspect_original_goods_ledger(OriginalGoodsLedgerView view);
void validate_original_goods_ledger(OriginalGoodsLedgerView view);

// Pure candidate results. Inputs remain untouched on both success and failure.
// These are numerical operations, never World stock grants or Arrival events.
OriginalGoodsLedgerSnapshot original_goods_produce(OriginalGoodsLedgerView,BuildingId,original::Goods,std::int32_t);
OriginalGoodsLedgerSnapshot original_goods_import(OriginalGoodsLedgerView,BuildingId,original::Goods,std::int32_t);
OriginalGoodsLedgerSnapshot original_goods_consume(OriginalGoodsLedgerView,BuildingId,original::Goods,std::int32_t);
OriginalGoodsLedgerSnapshot original_goods_demolish(OriginalGoodsLedgerView,BuildingId,original::Goods,std::int32_t);
OriginalGoodsLedgerSnapshot original_goods_reserve(OriginalGoodsLedgerView,OriginalGoodsTransferKey,
    BuildingId source,BuildingId target,original::Goods,std::int32_t);
OriginalGoodsLedgerSnapshot original_goods_load(OriginalGoodsLedgerView,OriginalGoodsTransferKey,std::int32_t);
OriginalGoodsLedgerSnapshot original_goods_deliver(OriginalGoodsLedgerView,OriginalGoodsTransferKey,std::int32_t);
OriginalGoodsLedgerSnapshot original_goods_cancel(OriginalGoodsLedgerView,OriginalGoodsTransferKey);
OriginalGoodsLedgerSnapshot original_goods_return(OriginalGoodsLedgerView,OriginalGoodsTransferKey,std::int32_t);

} // namespace openemperor::simulation
