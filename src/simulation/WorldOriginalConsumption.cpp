#include "simulation/World.h"
#include "simulation/OriginalIndustryGoodsContext.h"
#include "simulation/original/OriginalRulesModel.h"

#include <algorithm>
#include <array>
#include <stdexcept>
#include <utility>

namespace openemperor::simulation {
namespace {
[[noreturn]] void invalid(const char* message) {throw std::invalid_argument(message);}
std::uint64_t consumption_events(std::uint64_t tick) {
    return (tick>=40 ? 1+(tick-40)/816:0)+(tick>=397 ? 1+(tick-397)/816:0);
}
template<class Buildings>
std::array<std::uint64_t,original::goods_count> house_totals(
    std::span<const OriginalHouseConsumptionState> history,std::span<const OriginalHouseState> houses,
    const Buildings& buildings,std::uint64_t tick,const original::HouseEvolutionRules* rules=nullptr) {
    if (history.size()!=houses.size() || history.size()>Rules::city_v10_household_limit)
        invalid("original consumption requires exactly one history per paid House");
    std::array<std::uint64_t,original::goods_count> sums{};
    for (std::size_t i=0;i<history.size();++i) {
        const auto& entry=history[i];const auto& house=houses[i];
        if (entry.building_id!=house.building_id || (i && !(history[i-1].building_id<entry.building_id)))
            invalid("original consumption House references must be exact and sorted");
        const auto b=std::lower_bound(buildings.begin(),buildings.end(),entry.building_id,
            [](const auto& value,BuildingId id){return value.id<id;});
        if (b==buildings.end() || b->id!=entry.building_id || b->kind!=Object::Household || b->placed_tick>tick)
            invalid("original consumption history has no paid House");
        // paid age proves placed+119 is in-domain before that addition.
        const auto opportunities=tick-b->placed_tick<120 ? 0:
            consumption_events(tick)-consumption_events(b->placed_tick+119);
        for (std::size_t g=0;g<original::goods_count;++g) {
            int cap=Rules::household_capacity;
            if (rules) {
                int need=0;
                for (const auto row:{original::ModelHouse::Shelter,original::ModelHouse::Hut,original::ModelHouse::PlainCottage})
                    need=std::max(need,rules->value(row,original::goods_field(static_cast<original::Goods>(g))));
                cap=std::clamp(need,0,cap);
            }
            const auto maximum=static_cast<std::uint64_t>(cap);const auto n=entry.consumed[g];
            if ((maximum && opportunities>UINT64_MAX/maximum) || n>opportunities*maximum ||
                (!house.house.occupants && n) || n>UINT64_MAX-sums[g])
                invalid("original consumption exceeds House age, model need or counter domain");
            sums[g]+=n;
        }
    }
    return sums;
}
void add_paper_inputs(std::array<std::uint64_t,original::goods_count>& totals,
    const OriginalIndustryState& industry) {
    for (const auto& source:industry.sources) for (std::size_t g=0;g<original::goods_count;++g) {
        if (source.goods_inputs_used[g]>UINT64_MAX-totals[g]) invalid("House plus industry goods sink overflow");
        totals[g]+=source.goods_inputs_used[g];
    }
}
void compare(const std::array<std::uint64_t,original::goods_count>& sums,const OriginalGoodsState& goods) {
    for (std::size_t g=0;g<original::goods_count;++g)
        if (sums[g]!=goods.totals[g].consumed) invalid("goods ledger consumption differs from actual House plus Paper recipe sinks");
}
}

void World::validate_original_consumption_snapshot(const WorldSnapshot& s) {
    auto sums=house_totals(s.original_production->house_consumption,s.original_economy->houses,s.buildings,s.ticks);
    add_paper_inputs(sums,s.original_production->industry);compare(sums,*s.original_goods);
}
void World::validate_original_consumption_model() const {
    auto sums=house_totals(original_production_->house_consumption,original_economy_->houses,buildings_,ticks_,&original_rules_model_->house_rules());
    add_paper_inputs(sums,original_production_->industry);compare(sums,*original_goods_);
}
void World::consume_original_house_goods() {
    const original_industry_goods::Context context(buildings_,couriers_);
    OriginalGoodsLedgerSnapshot candidate{*original_goods_,original_economy_->houses};
    auto history=original_production_->house_consumption;auto rows=original_production_->housing.houses;
    if (history.size()!=candidate.houses.size() || rows.size()!=history.size()) throw std::logic_error("missing House consumption history");
    for (std::size_t i=0;i<candidate.houses.size();++i) {
        if (!candidate.houses[i].house.occupants) continue;
        auto& row=rows[i].consumption_rows[static_cast<std::size_t>(candidate.houses[i].house.type)-3];
        if (row.events==UINT64_MAX) throw std::overflow_error("Housing consumption events exhausted");
        ++row.events;
        // Keep the original OR-needs and separate per-commodity debit kernel.
        const auto expected=original::consume_goods(original_rules_model_->house_rules(),candidate.houses[i].house);
        for (std::size_t g=0;g<original::goods_count;++g) {
            const auto amount=expected.consumed[g];if (!amount) continue;
            if (amount<0 || history[i].consumed[g]>UINT64_MAX-static_cast<std::uint64_t>(amount) ||
                row.consumed[g]>UINT64_MAX-static_cast<std::uint64_t>(amount))
                throw std::overflow_error("original House consumption history exhausted");
            candidate=original_goods_consume(context.view(candidate.goods,candidate.houses),
                candidate.houses[i].building_id,static_cast<original::Goods>(g),amount);
            history[i].consumed[g]+=static_cast<std::uint64_t>(amount);row.consumed[g]+=static_cast<std::uint64_t>(amount);
        }
        if (candidate.houses[i].house!=expected.house) throw std::logic_error("original consumption ledger diverged from pure model kernel");
    }
    auto sums=house_totals(history,candidate.houses,buildings_,ticks_,&original_rules_model_->house_rules());
    add_paper_inputs(sums,original_production_->industry);compare(sums,candidate.goods);
    validate_original_goods_ledger(context.view(candidate.goods,candidate.houses));
    original_goods_=std::move(candidate.goods);original_economy_->houses=std::move(candidate.houses);
    original_production_->house_consumption=std::move(history);original_production_->housing.houses=std::move(rows);
}

} // namespace openemperor::simulation
