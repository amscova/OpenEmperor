#include "simulation/World.h"
#include "simulation/OriginalGoodsLedger.h"

#include <algorithm>
#include <array>
#include <stdexcept>
#include <utility>

namespace openemperor::simulation {
namespace {
constexpr auto ceramics=static_cast<std::size_t>(original::Goods::Ceramics);
bool typed(CourierRole role) {return role==CourierRole::MarketPotteryInbound || role==CourierRole::MarketPotteryDistribution;}
[[noreturn]] void invalid(const char* message) {throw std::invalid_argument(message);}
int capacity(Object kind) {
    return kind==Object::Warehouse ? Rules::warehouse_capacity:
        kind==Object::Market ? Rules::market_pottery_capacity:Rules::household_capacity;
}
struct Context {
    std::array<OriginalGoodsOwner,original_goods_owner_limit> owners{};
    std::array<CourierId,6> couriers{};
    std::size_t count=0,actors=0;
    template<class Buildings,class Couriers> Context(const Buildings& buildings,const Couriers& moving) {
        for (const auto& b:buildings) if (b.kind==Object::Household || b.kind==Object::Warehouse || b.kind==Object::Market) {
            if (count==owners.size()) invalid("typed market owner bound exceeded");
            auto& owner=owners[count++];owner.building_id=b.id;owner.household=b.kind==Object::Household;
            owner.capacity[ceramics]=capacity(b.kind);
        }
        for (const auto& c:moving) if (typed(c.role)) {
            if (actors==couriers.size()) invalid("typed market courier bound exceeded");
            couriers[actors++]=c.id;
        }
    }
    OriginalGoodsLedgerView view(const OriginalGoodsState& goods,std::span<const OriginalHouseState> homes) const {
        return {goods,homes,{owners.data(),count},{couriers.data(),actors}};
    }
};
const OriginalGoodsReservation* row(const OriginalGoodsState& state,CourierId id) {
    const auto found=std::lower_bound(state.reservations.begin(),state.reservations.end(),id,
        [](const auto& r,CourierId key){return r.transfer.courier_id<key;});
    return found==state.reservations.end() || found->transfer.courier_id!=id ? nullptr:&*found;
}
int cargo(const OriginalGoodsState& state,CourierId id) {
    const auto found=std::lower_bound(state.cargo.begin(),state.cargo.end(),id,
        [](const auto& c,CourierId key){return c.transfer.courier_id<key;});
    return found==state.cargo.end() || found->transfer.courier_id!=id ? 0:found->amount;
}
int incoming(const OriginalGoodsState& goods,BuildingId target) {
    std::int64_t total=0;
    for (const auto& r:goods.reservations) if (r.target==target) {
        if (r.phase==OriginalGoodsTransferPhase::Reserved) total+=r.reserved;
        else if (r.phase==OriginalGoodsTransferPhase::Loaded) total+=r.loaded-r.delivered-r.returned;
    }
    if (total>INT32_MAX) invalid("typed incoming reservations overflow");return static_cast<int>(total);
}
int returning_room(const OriginalGoodsState& goods,BuildingId source) {
    std::int64_t total=0;for (const auto& c:goods.cargo) if (c.source==source) total+=c.amount;
    if (total>INT32_MAX) invalid("typed source return-room overflow");return static_cast<int>(total);
}
template<class Buildings,class Couriers> void validate_market_bindings(const Buildings& buildings,const Couriers& couriers,
    const OriginalGoodsState& goods,std::span<const OriginalHouseState> homes) {
    const Context context(buildings,couriers);validate_original_goods_ledger(context.view(goods,homes));
    for (std::size_t i=0;i<goods.reservations.size();++i) {
        const auto& r=goods.reservations[i];
        if ((i && goods.reservations[i-1].transfer.courier_id==r.transfer.courier_id) ||
            r.good!=original::Goods::Ceramics || r.phase==OriginalGoodsTransferPhase::Reserved ||
            r.phase==OriginalGoodsTransferPhase::Cancelled || r.reserved>Rules::courier_capacity ||
            r.reserved!=r.loaded ||
            (r.phase!=OriginalGoodsTransferPhase::Completed && r.delivered!=0))
            invalid("typed market must retain exactly the latest real loaded trip per courier");
        const auto c=std::find_if(couriers.begin(),couriers.end(),[&](const auto& value){return value.id==r.transfer.courier_id;});
        if (c==couriers.end() || !typed(c->role) || c->owner!=r.source || c->target!=r.target ||
            c->last_dispatched_target!=r.target)
            invalid("typed transfer is not owned by its actual courier and destination");
    }
    for (const auto& c:couriers) if (typed(c.role)) {
        if (c.cargo || c.reserved || c.good!=Good::Pottery) invalid("typed cargo is mirrored into legacy fields");
        const auto* r=row(goods,c.id);const auto amount=cargo(goods,c.id);
        if (!r) {
            if (c.phase!=CourierPhase::IdleAtWorkshop || c.last_dispatched_target || c.target!=c.owner)
                invalid("used typed courier lost its latest trip and sequence highwater");
            continue;
        }
        if (c.phase==CourierPhase::ToWarehouse) {
            if (r->phase!=OriginalGoodsTransferPhase::Loaded || amount<=0) invalid("typed outgoing movement lost its cargo");
        } else if (c.phase==CourierPhase::Returning) {
            if (!((r->phase==OriginalGoodsTransferPhase::Completed && !amount) ||
                (r->phase==OriginalGoodsTransferPhase::Returning && amount>0))) invalid("typed return phase and cargo disagree");
        } else if (amount || (r->phase!=OriginalGoodsTransferPhase::Completed && r->phase!=OriginalGoodsTransferPhase::Returned))
            invalid("idle typed courier has unfinished goods");
    }
    for (const auto& b:buildings) if (b.kind==Object::Warehouse || b.kind==Object::Market) {
        const auto stock=std::find_if(goods.stocks.begin(),goods.stocks.end(),[&](const auto& s){return s.building_id==b.id;});
        if (stock==goods.stocks.end() || std::int64_t(stock->amounts[ceramics])+returning_room(goods,b.id)+
            incoming(goods,b.id)+b.reserved_incoming>capacity(b.kind))
            invalid("typed source stock, return-room and incoming commitments exceed physical capacity");
    }
}
}

bool World::original_market_courier(CourierRole role) const {return original_production_enabled() && typed(role);}
int World::original_courier_cargo(CourierId id) const {
    if (!original_production_enabled()) return 0;
    (void)courier(id);return cargo(*original_goods_,id);
}
int World::original_market_headroom(BuildingId owner) const {
    const auto& b=building(owner);
    return capacity(b.kind)-original_ceramics_stock(owner)-returning_room(*original_goods_,owner)-
        incoming(*original_goods_,owner)-b.reserved_incoming;
}
void World::validate_original_market_snapshot(const WorldSnapshot& s) {
    for (const auto& r:s.original_goods->reservations)
        if (r.transfer.sequence>s.ticks) invalid("typed courier history exceeds elapsed dispatch opportunities");
    validate_market_bindings(s.buildings,s.couriers,*s.original_goods,s.original_economy->houses);
}
void World::validate_original_market_state() const {
    for (const auto& r:original_goods_->reservations)
        if (r.transfer.sequence>ticks_) invalid("typed courier history exceeds elapsed dispatch opportunities");
    validate_market_bindings(buildings_,couriers_,*original_goods_,original_economy_->houses);
}

CourierDispatchDecision World::original_market_dispatch_status(const CourierState& c) const {
    if (original_ceramics_stock(c.owner)<=0) return {CourierDispatchStatus::NoStock,{}};
    std::array<const BuildingState*,Rules::city_v10_household_limit> targets{};std::size_t count=0;
    for (const auto& b:buildings_) if (courier_can_target(c.role,b)) {
        if (b.kind==Object::Household) {
            const auto& homes=original_economy_->houses;
            const auto h=std::lower_bound(homes.begin(),homes.end(),b.id,[](const auto& value,BuildingId id){return value.building_id<id;});
            if (h==homes.end() || h->building_id!=b.id || !original::accepts_goods_delivery(h->house)) continue;
        }
        if (!b.operating_enabled || building_on_fire(b.id)) continue;
        targets[count++]=&b;
    }
    if (!count) return {CourierDispatchStatus::NoTarget,{}};
    std::size_t start=0;
    for (std::size_t i=0;i<count;++i) if (c.last_dispatched_target==targets[i]->id) start=(i+1)%count;
    bool room=false;
    for (std::size_t step=0;step<count;++step) {
        const auto& b=*targets[(start+step)%count];if (original_market_headroom(b.id)<=0) continue;room=true;
        const auto found=std::lower_bound(c.dynamic_target_routes.begin(),c.dynamic_target_routes.end(),b.id,
            [](const auto& entry,BuildingId id){return entry.first<id;});
        if (found!=c.dynamic_target_routes.end() && found->first==b.id && found->second)
            return {CourierDispatchStatus::Ready,b.id};
    }
    return {room ? CourierDispatchStatus::NoRoad:CourierDispatchStatus::TargetFull,{}};
}

void World::dispatch_original_market(CourierState& c) {
    const auto decision=courier_dispatch_status(c.id);
    if (decision.status!=CourierDispatchStatus::Ready || !decision.selected_target) return;
    const auto target=*decision.selected_target;
    const auto route=std::lower_bound(c.dynamic_target_routes.begin(),c.dynamic_target_routes.end(),target,
        [](const auto& entry,BuildingId id){return entry.first<id;});
    if (route==c.dynamic_target_routes.end() || !route->second) throw std::logic_error("typed dispatch lost cached route");
    const auto amount=std::min({Rules::courier_capacity,original_ceramics_stock(c.owner),original_market_headroom(target)});
    const auto* previous=row(*original_goods_,c.id);
    if (previous && previous->transfer.sequence==UINT64_MAX) return;
    const OriginalGoodsTransferKey key{c.id,previous ? previous->transfer.sequence+1:1};
    const Context context(buildings_,couriers_);
    // Six real typed actors retain six latest rows; reserve's temporary seventh
    // row fits the unchanged pure ledger. The old highwater is replaced only
    // after its successor exists, and stale handles can never name that trip.
    auto candidate=original_goods_reserve(context.view(*original_goods_,original_economy_->houses),key,
        c.owner,target,original::Goods::Ceramics,amount);
    std::erase_if(candidate.goods.reservations,[&](const auto& r){return r.transfer.courier_id==c.id && r.transfer!=key;});
    candidate=original_goods_load(context.view(candidate.goods,candidate.houses),key,amount);
    auto path=*route->second; // Allocate before authoritative publication.
    original_goods_=std::move(candidate.goods);
    c.path=std::move(path);c.target=target;c.last_dispatched_target=target;
    c.path_vertex=0;c.edge_progress=0;c.route_pending=false;c.route_checked_revision.reset();
    c.phase=CourierPhase::ToWarehouse;
}

void World::cancel_original_market(BuildingId owner) {
    for (auto& c:couriers_) if (c.owner==owner && original_market_courier(c.role) && c.phase==CourierPhase::ToWarehouse) {
        const auto* trip=row(*original_goods_,c.id);if (!trip) throw std::logic_error("loaded typed actor has no trip");
        const auto key=trip->transfer;const Context context(buildings_,couriers_);
        auto candidate=original_goods_cancel(context.view(*original_goods_,original_economy_->houses),key);
        const auto here=c.path.at(c.path_vertex);
        const auto& source=building(owner);
        if (!c.edge_progress && building_footprint_contains(profile_,rule_version_,source.kind,source.cell,here)) {
            candidate=original_goods_return(context.view(candidate.goods,candidate.houses),key,cargo(candidate.goods,c.id));
            original_goods_=std::move(candidate.goods);c.path.clear();c.path_vertex=0;c.edge_progress=0;
            c.route_pending=false;c.route_checked_revision.reset();c.phase=CourierPhase::IdleAtWorkshop;
        } else {
            original_goods_=std::move(candidate.goods);c.phase=CourierPhase::Returning;
            mark_route_pending(c); // A begun edge remains exact until its next vertex.
            if (!c.edge_progress) reroute_v2(c);
        }
    }
}

void World::arrive_original_market(CourierState& c) {
    const auto* trip=row(*original_goods_,c.id);if (!trip) throw std::logic_error("typed arrival has no latest trip");
    const auto key=trip->transfer;const Context context(buildings_,couriers_);
    if (c.phase==CourierPhase::ToWarehouse) {
        const auto& target=building(c.target);
        const auto amount=cargo(*original_goods_,c.id);
        OriginalGoodsLedgerSnapshot candidate;
        if (!target.operating_enabled || building_on_fire(target.id))
            candidate=original_goods_cancel(context.view(*original_goods_,original_economy_->houses),key);
        else candidate=original_goods_deliver(context.view(*original_goods_,original_economy_->houses),key,amount);
        original_goods_=std::move(candidate.goods);original_economy_->houses=std::move(candidate.houses);
        c.phase=CourierPhase::Returning;
        if (valid_return_path(c)) {std::reverse(c.path.begin(),c.path.end());c.path_vertex=0;}
        else {c.path={c.path.back()};c.path_vertex=0;c.route_pending=true;c.route_checked_revision.reset();reroute_v2(c);}
    } else {
        const auto amount=cargo(*original_goods_,c.id);
        if (amount) {
            auto candidate=original_goods_return(context.view(*original_goods_,original_economy_->houses),key,amount);
            original_goods_=std::move(candidate.goods);
        }
        c.path.clear();c.path_vertex=0;c.phase=CourierPhase::IdleAtWorkshop;c.route_pending=false;c.route_checked_revision.reset();
    }
}

} // namespace openemperor::simulation
