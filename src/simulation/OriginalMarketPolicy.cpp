#include "simulation/OriginalMarketPolicy.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <tuple>

namespace openemperor::simulation {
namespace {
bool non_goods_met(const original::HouseEvolutionRules& rules,original::ModelHouse row,
                   const original::HouseState& actual) {
    // Pure query of the shared needs kernel with goods masked out. This local
    // value never becomes World state, a transfer, a resource or a witness.
    auto query=actual;
    query.hemp=query.ceramics=query.tea=query.silk=query.bronzeware=query.lacquerware=
        std::numeric_limits<std::int16_t>::max();
    return original::needs_met(rules,row,query);
}
}
std::optional<MarketNeed> original_market_need(const original::HouseEvolutionRules& rules,
    const original::HouseState& house,original::Goods good,
    const std::array<int,original::goods_count>& incoming) {
    (void)original::house_row(house.type);
    for(unsigned g=0;g<original::goods_count;++g)
        if(original::goods_stock(house,static_cast<original::Goods>(g))<0)throw std::invalid_argument("negative Market House stock");
    if(house.occupants<=0)return std::nullopt;
    for(unsigned i=0;i<original::goods_count;++i)
        if(incoming[i]<0||incoming[i]>8||original::goods_stock(house,static_cast<original::Goods>(i))+incoming[i]>8)
            throw std::invalid_argument("incoming Market commitment exceeds physical House capacity");
    const auto g=static_cast<std::size_t>(good);
    if(g>=original::goods_count)throw std::invalid_argument("invalid Market commodity");
    const int stock=original::goods_stock(house,good)+incoming[g];
    // Keep enough physical buffer for two full unchanged four-unit carts.
    // A half-full buffer triggers refill; incoming stock counts once. Models
    // requiring more than the unchanged eight-unit House capacity fail closed.
    const auto need_for=[&](original::ModelHouse row)->int {
        const auto need=rules.value(row,original::goods_field(good));
        if(need<=0||need>8)return 0;
        if(good==original::Goods::Bronzeware||good==original::Goods::Lacquerware) {
            const auto alternative=good==original::Goods::Bronzeware?original::Goods::Lacquerware:original::Goods::Bronzeware;
            if(original::goods_stock(house,alternative)+incoming[static_cast<std::size_t>(alternative)]>=need)return 0;
        }
        return need;
    };
    if(const int need=need_for(original::house_row(house.type));need>0) {
        if(stock<need)return MarketNeed{0,stock,need};
        if(stock<=4)return MarketNeed{1,stock,need};
        return std::nullopt;
    }
    if(stock>4)return std::nullopt;
    const auto next=original::evolve_target(house.type);
    if(!next||original::is_elite(*next)!=original::is_elite(house.type)||
       (house.type!=original::HouseType::EliteAbandoned&&original::desirability_verdict(rules,house)!=original::Verdict::Evolve)||
       !non_goods_met(rules,original::house_row(house.type),house)||
       !non_goods_met(rules,original::house_row(*next),house))return std::nullopt;
    if(const int need=need_for(original::house_row(*next));need>0)return MarketNeed{2,stock,need};
    return std::nullopt;
}
std::optional<std::size_t> choose_original_market_pair(std::span<const MarketChoice> choices,
    std::size_t start,std::size_t pair_count) {
    if(!pair_count||start>=pair_count)throw std::invalid_argument("invalid Market cursor domain");
    const MarketChoice* best=nullptr;
    const auto distance=[&](std::size_t pair){return (pair+pair_count-start)%pair_count;};
    for(const auto& choice:choices) {
        if(choice.pair>=pair_count||choice.need.tier>2||choice.need.stock<0||choice.need.demand<=0)
            throw std::invalid_argument("invalid Market need candidate");
        if(!best||choice.need.tier<best->need.tier||
           (choice.need.tier==best->need.tier&&
            (std::int64_t(choice.need.stock)*best->need.demand<std::int64_t(best->need.stock)*choice.need.demand||
             (std::int64_t(choice.need.stock)*best->need.demand==std::int64_t(best->need.stock)*choice.need.demand&&
              distance(choice.pair)<distance(best->pair)))))best=&choice;
    }
    return best?std::optional{best->pair}:std::nullopt;
}
} // namespace openemperor::simulation
