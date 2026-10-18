#pragma once

#include "simulation/World.h"
#include "simulation/OriginalGoodsLedger.h"

#include <array>
#include <optional>
#include <stdexcept>

namespace openemperor::simulation::original_industry_goods {

inline std::optional<original::Goods> output(Object kind) {
    switch (kind) {
    case Object::OriginalBronzewareMaker42:return original::Goods::Bronzeware;
    case Object::OriginalLacquerwareMaker44:return original::Goods::Lacquerware;
    case Object::OriginalWeaver47:return original::Goods::Silk;
    case Object::OriginalTeaCuringShed237:return original::Goods::Tea;
    default:return std::nullopt;
    }
}
inline bool owner(Object kind) {
    return original_house_kind(kind) || kind==Object::Warehouse || kind==Object::Market ||
        kind==Object::OriginalHempFarm || kind==Object::OriginalPaperMaker45 || output(kind).has_value();
}
inline int capacity(Object kind,original::Goods good) {
    if (original_house_kind(kind)) return Rules::household_capacity;
    if (kind==Object::Warehouse) return Rules::warehouse_capacity;
    if (kind==Object::Market) return Rules::market_pottery_capacity;
    if (kind==Object::OriginalHempFarm || kind==Object::OriginalPaperMaker45)
        return good==original::Goods::Hemp ? 8:0;
    return output(kind)==good ? 8:0;
}
inline bool typed(CourierRole role) {
    return role==CourierRole::MarketPotteryInbound || role==CourierRole::MarketPotteryDistribution ||
        role==CourierRole::OriginalHempToWarehouse || role==CourierRole::OriginalLuxuryToWarehouse;
}
class Context {
public:
    template<class Buildings,class Couriers> Context(const Buildings& buildings,const Couriers& moving) {
        for (const auto& b:buildings) if (owner(b.kind)) {
            if (owner_count_==owners_.size()) throw std::invalid_argument("original goods owner bound exceeded");
            auto& entry=owners_[owner_count_++];entry.building_id=b.id;entry.household=original_house_kind(b.kind);
            for (std::size_t g=0;g<original::goods_count;++g)
                entry.capacity[g]=capacity(b.kind,static_cast<original::Goods>(g));
        }
        for (const auto& c:moving) if (typed(c.role)) {
            if (actor_count_==actors_.size()) throw std::invalid_argument("original goods actor bound exceeded");
            actors_[actor_count_++]=c.id;
        }
    }
    std::span<const OriginalGoodsOwner> owners() const {return {owners_.data(),owner_count_};}
    std::span<const CourierId> actors() const {return {actors_.data(),actor_count_};}
    OriginalGoodsLedgerView view(const OriginalGoodsState& goods,std::span<const OriginalHouseState> houses) const {
        return {goods,houses,owners(),actors()};
    }
private:
    std::array<OriginalGoodsOwner,original_goods_owner_limit> owners_{};
    std::array<CourierId,16> actors_{};
    std::size_t owner_count_=0,actor_count_=0;
};

} // namespace openemperor::simulation::original_industry_goods
