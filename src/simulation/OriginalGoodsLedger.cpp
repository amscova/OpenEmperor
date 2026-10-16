#include "simulation/OriginalGoodsLedger.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

namespace openemperor::simulation {
namespace {
[[noreturn]] void invalid(const char* message) { throw std::invalid_argument(message); }
std::size_t column(original::Goods good) {
    const auto i=static_cast<std::size_t>(good);
    if (i>=original::goods_count) invalid("invalid original goods identity");
    return i;
}
void add(std::uint64_t& target,std::uint64_t amount) {
    if (amount>UINT64_MAX-target) throw std::overflow_error("original goods total overflow");
    target+=amount;
}
bool key_less(OriginalGoodsTransferKey a,OriginalGoodsTransferKey b) {
    return a.courier_id<b.courier_id || (a.courier_id==b.courier_id && a.sequence<b.sequence);
}
bool active(OriginalGoodsTransferPhase phase) {
    return phase==OriginalGoodsTransferPhase::Reserved || phase==OriginalGoodsTransferPhase::Loaded ||
        phase==OriginalGoodsTransferPhase::Returning;
}
std::size_t owner_index(OriginalGoodsLedgerView v,BuildingId id) {
    const auto found=std::lower_bound(v.owners.begin(),v.owners.end(),id,
        [](const OriginalGoodsOwner& a,BuildingId b) { return a.building_id<b; });
    if (found==v.owners.end() || found->building_id!=id) invalid("unknown original goods owner");
    return static_cast<std::size_t>(found-v.owners.begin());
}
std::int16_t& house_stock(original::HouseState& house,original::Goods good) {
    switch (good) {
    case original::Goods::Hemp:return house.hemp;
    case original::Goods::Ceramics:return house.ceramics;
    case original::Goods::Tea:return house.tea;
    case original::Goods::Silk:return house.silk;
    case original::Goods::Bronzeware:return house.bronzeware;
    case original::Goods::Lacquerware:return house.lacquerware;
    }
    invalid("invalid original goods identity");
}
std::int32_t stock(OriginalGoodsLedgerView v,BuildingId id,original::Goods good) {
    const auto& owner=v.owners[owner_index(v,id)];
    if (owner.household) {
        const auto h=std::lower_bound(v.houses.begin(),v.houses.end(),id,
            [](const OriginalHouseState& a,BuildingId b) { return a.building_id<b; });
        if (h==v.houses.end() || h->building_id!=id) invalid("missing original house stock authority");
        return original::goods_stock(h->house,good);
    }
    const auto s=std::lower_bound(v.goods.stocks.begin(),v.goods.stocks.end(),id,
        [](const OriginalGoodsStock& a,BuildingId b) { return a.building_id<b; });
    if (s==v.goods.stocks.end() || s->building_id!=id) invalid("missing original non-house stock authority");
    return s->amounts[column(good)];
}
void set_stock(OriginalGoodsLedgerSnapshot& s,OriginalGoodsLedgerView v,BuildingId id,
               original::Goods good,std::int64_t amount) {
    const auto& owner=v.owners[owner_index(v,id)];
    if (amount<0 || amount>owner.capacity[column(good)]) invalid("original goods stock outside capacity");
    if (owner.household) {
        const auto h=std::lower_bound(s.houses.begin(),s.houses.end(),id,
            [](const OriginalHouseState& a,BuildingId b) { return a.building_id<b; });
        house_stock(h->house,good)=static_cast<std::int16_t>(amount);
    } else {
        const auto row=std::lower_bound(s.goods.stocks.begin(),s.goods.stocks.end(),id,
            [](const OriginalGoodsStock& a,BuildingId b) { return a.building_id<b; });
        row->amounts[column(good)]=static_cast<std::int32_t>(amount);
    }
}
OriginalGoodsReservation& reservation(OriginalGoodsLedgerSnapshot& s,OriginalGoodsTransferKey key) {
    const auto r=std::lower_bound(s.goods.reservations.begin(),s.goods.reservations.end(),key,
        [](const OriginalGoodsReservation& a,OriginalGoodsTransferKey b) { return key_less(a.transfer,b); });
    if (r==s.goods.reservations.end() || r->transfer!=key) invalid("unknown original goods transfer");
    return *r;
}
OriginalGoodsCargo& cargo(OriginalGoodsLedgerSnapshot& s,OriginalGoodsTransferKey key) {
    const auto c=std::find_if(s.goods.cargo.begin(),s.goods.cargo.end(),
        [&](const OriginalGoodsCargo& value) { return value.transfer==key; });
    if (c==s.goods.cargo.end()) invalid("original goods transfer has no cargo");
    return *c;
}
void positive(std::int32_t amount) { if (amount<=0) invalid("original goods transaction amount must be positive"); }
template<class F> OriginalGoodsLedgerSnapshot transaction(OriginalGoodsLedgerView v,F operation) {
    validate_original_goods_ledger(v);
    OriginalGoodsLedgerSnapshot candidate{v.goods,{v.houses.begin(),v.houses.end()}};
    operation(candidate);
    validate_original_goods_ledger(candidate.view(v.owners,v.couriers));
    return candidate;
}
std::uint64_t source_reserved(OriginalGoodsLedgerView v,BuildingId id,original::Goods good) {
    std::uint64_t result=0;
    for (const auto& r:v.goods.reservations)
        if (r.source==id && r.good==good && r.phase==OriginalGoodsTransferPhase::Reserved)
            add(result,static_cast<std::uint64_t>(r.reserved));
    return result;
}
OriginalGoodsLedgerSnapshot book(OriginalGoodsLedgerView v,BuildingId id,original::Goods good,
                                 std::int32_t amount,int direction,std::uint64_t OriginalGoodsTotals::* total) {
    return transaction(v,[&](auto& s) {
        positive(amount);const auto i=column(good);const auto prior=stock(v,id,good);
        if (direction<0 && static_cast<std::uint64_t>(amount)>
            static_cast<std::uint64_t>(prior)-source_reserved(v,id,good))
            invalid("original goods debit exceeds unreserved stock");
        set_stock(s,v,id,good,std::int64_t(prior)+std::int64_t(direction)*amount);
        add(s.goods.totals[i].*total,static_cast<std::uint64_t>(amount));
    });
}
}

std::array<OriginalGoodsBalance,original::goods_count> inspect_original_goods_ledger(OriginalGoodsLedgerView v) {
    if (v.owners.size()>original_goods_owner_limit || v.couriers.size()>original_goods_courier_limit ||
        v.houses.size()>20 || v.goods.stocks.size()>original_goods_owner_limit ||
        v.goods.cargo.size()>original_goods_courier_limit ||
        v.goods.reservations.size()>original_goods_reservation_limit)
        invalid("original goods collection exceeds bounded context");
    for (std::size_t i=0;i<v.owners.size();++i) {
        const auto& owner=v.owners[i];
        if (owner.building_id==static_cast<BuildingId>(0) || (i && !(v.owners[i-1].building_id<owner.building_id)))
            invalid("original goods owner IDs must be positive and sorted");
        for (const auto cap:owner.capacity)
            if (cap<0 || (owner.household && cap>INT16_MAX)) invalid("invalid original goods capacity context");
    }
    for (std::size_t i=0;i<v.couriers.size();++i)
        if (v.couriers[i]==static_cast<CourierId>(0) || (i && !(v.couriers[i-1]<v.couriers[i])))
            invalid("original goods courier IDs must be positive and sorted");
    for (std::size_t i=0;i<v.houses.size();++i) {
        const auto id=v.houses[i].building_id;
        if ((i && !(v.houses[i-1].building_id<id)) || !v.owners[owner_index(v,id)].household)
            invalid("duplicate, unsorted or non-house original house account");
    }
    for (std::size_t i=0;i<v.goods.stocks.size();++i) {
        const auto id=v.goods.stocks[i].building_id;
        if ((i && !(v.goods.stocks[i-1].building_id<id)) || v.owners[owner_index(v,id)].household)
            invalid("duplicate, unsorted or mirrored original stock account");
    }
    std::array<OriginalGoodsBalance,original::goods_count> result{};
    std::array<std::array<std::uint64_t,original::goods_count>,original_goods_owner_limit> outgoing{},incoming{};
    for (std::size_t g=0;g<original::goods_count;++g) {
        const auto good=static_cast<original::Goods>(g);result[g].good=good;result[g].totals=v.goods.totals[g];
        for (const auto& owner:v.owners) {
            const auto amount=stock(v,owner.building_id,good);
            if (amount<0 || amount>owner.capacity[g]) invalid("original goods stock outside declared capacity");
            add(result[g].stock,static_cast<std::uint64_t>(amount));
        }
    }
    for (std::size_t i=0;i<v.goods.reservations.size();++i) {
        const auto& r=v.goods.reservations[i];const auto g=column(r.good);
        if (!r.transfer.sequence || !std::binary_search(v.couriers.begin(),v.couriers.end(),r.transfer.courier_id) ||
            (i && !key_less(v.goods.reservations[i-1].transfer,r.transfer)) || r.source==r.target ||
            r.reserved<=0 || r.loaded<0 || r.loaded>r.reserved || r.delivered<0 || r.returned<0 ||
            std::int64_t(r.delivered)+r.returned>r.loaded)
            invalid("invalid original goods reservation identity or amounts");
        const auto from=owner_index(v,r.source),to=owner_index(v,r.target);
        const auto remaining=r.loaded-r.delivered-r.returned;
        const bool has_cargo=r.phase==OriginalGoodsTransferPhase::Loaded || r.phase==OriginalGoodsTransferPhase::Returning;
        switch (r.phase) {
        case OriginalGoodsTransferPhase::Reserved:
            if (r.loaded || r.delivered || r.returned) invalid("reserved goods already moved");
            add(outgoing[from][g],static_cast<std::uint64_t>(r.reserved));
            add(incoming[to][g],static_cast<std::uint64_t>(r.reserved));break;
        case OriginalGoodsTransferPhase::Loaded:
            if (remaining<=0 || r.returned) invalid("invalid loaded original goods transfer");
            add(incoming[to][g],static_cast<std::uint64_t>(remaining));break;
        case OriginalGoodsTransferPhase::Returning:
            if (remaining<=0) invalid("returning original goods lost cargo");break;
        case OriginalGoodsTransferPhase::Completed:
            if (!r.loaded || r.delivered!=r.loaded || r.returned) invalid("invalid completed goods history");break;
        case OriginalGoodsTransferPhase::Cancelled:
            if (r.loaded || r.delivered || r.returned) invalid("cancelled reservation moved goods");break;
        case OriginalGoodsTransferPhase::Returned:
            if (!r.loaded || !r.returned || remaining) invalid("invalid returned goods history");break;
        default:invalid("invalid original goods transfer phase");
        }
        if (active(r.phase) && i+1<v.goods.reservations.size() &&
            v.goods.reservations[i+1].transfer.courier_id==r.transfer.courier_id)
            invalid("courier has a later transaction before finishing active goods transfer");
        const auto c=std::find_if(v.goods.cargo.begin(),v.goods.cargo.end(),
            [&](const OriginalGoodsCargo& value) { return value.transfer==r.transfer; });
        if (has_cargo) {
            if (c==v.goods.cargo.end() || c->good!=r.good || c->source!=r.source || c->target!=r.target ||
                c->amount!=remaining) invalid("original cargo disagrees with reservation");
        } else if (c!=v.goods.cargo.end()) invalid("inactive reservation retains original cargo");
    }
    for (std::size_t i=0;i<v.goods.cargo.size();++i) {
        const auto& c=v.goods.cargo[i];const auto g=column(c.good);
        if (c.amount<=0 || (i && !(v.goods.cargo[i-1].transfer.courier_id<c.transfer.courier_id)))
            invalid("original cargo must have unique ordered courier and positive amount");
        const auto r=std::find_if(v.goods.reservations.begin(),v.goods.reservations.end(),
            [&](const OriginalGoodsReservation& value) { return value.transfer==c.transfer; });
        if (r==v.goods.reservations.end()) invalid("orphan original goods cargo");
        add(result[g].cargo,static_cast<std::uint64_t>(c.amount));
    }
    for (std::size_t i=0;i<v.owners.size();++i) for (std::size_t g=0;g<original::goods_count;++g) {
        const auto amount=static_cast<std::uint64_t>(stock(v,v.owners[i].building_id,static_cast<original::Goods>(g)));
        if (outgoing[i][g]>amount || incoming[i][g]>static_cast<std::uint64_t>(v.owners[i].capacity[g])-amount)
            invalid("original goods reservation exceeds source stock or target capacity");
        add(result[g].source_reserved,outgoing[i][g]);add(result[g].incoming_reserved,incoming[i][g]);
    }
    for (auto& b:result) {
        auto inputs=b.totals.produced;add(inputs,b.totals.imported);
        auto outputs=b.totals.consumed;add(outputs,b.totals.demolished);add(outputs,b.stock);add(outputs,b.cargo);
        b.conserved=inputs==outputs;
    }
    return result;
}

void validate_original_goods_ledger(OriginalGoodsLedgerView v) {
    for (const auto& balance:inspect_original_goods_ledger(v))
        if (!balance.conserved) invalid("original goods conservation mismatch");
}
OriginalGoodsLedgerSnapshot original_goods_produce(OriginalGoodsLedgerView v,BuildingId id,original::Goods good,std::int32_t n) {
    return book(v,id,good,n,1,&OriginalGoodsTotals::produced);
}
OriginalGoodsLedgerSnapshot original_goods_import(OriginalGoodsLedgerView v,BuildingId id,original::Goods good,std::int32_t n) {
    return book(v,id,good,n,1,&OriginalGoodsTotals::imported);
}
OriginalGoodsLedgerSnapshot original_goods_consume(OriginalGoodsLedgerView v,BuildingId id,original::Goods good,std::int32_t n) {
    return book(v,id,good,n,-1,&OriginalGoodsTotals::consumed);
}
OriginalGoodsLedgerSnapshot original_goods_demolish(OriginalGoodsLedgerView v,BuildingId id,original::Goods good,std::int32_t n) {
    return book(v,id,good,n,-1,&OriginalGoodsTotals::demolished);
}
OriginalGoodsLedgerSnapshot original_goods_reserve(OriginalGoodsLedgerView v,OriginalGoodsTransferKey key,
    BuildingId source,BuildingId target,original::Goods good,std::int32_t n) {
    return transaction(v,[&](auto& s) {
        positive(n);(void)column(good);(void)owner_index(v,source);(void)owner_index(v,target);
        std::uint64_t previous=0;
        for (const auto& r:v.goods.reservations) if (r.transfer.courier_id==key.courier_id) {
            if (active(r.phase)) invalid("courier already has an active original goods transfer");
            previous=r.transfer.sequence;
        }
        if (previous==UINT64_MAX || key.sequence!=previous+1) invalid("original goods transfer sequence is stale or exhausted");
        OriginalGoodsReservation row{key,good,source,target,OriginalGoodsTransferPhase::Reserved,n,0,0,0};
        const auto at=std::lower_bound(s.goods.reservations.begin(),s.goods.reservations.end(),key,
            [](const OriginalGoodsReservation& a,OriginalGoodsTransferKey b) { return key_less(a.transfer,b); });
        s.goods.reservations.insert(at,row);
    });
}
OriginalGoodsLedgerSnapshot original_goods_load(OriginalGoodsLedgerView v,OriginalGoodsTransferKey key,std::int32_t n) {
    return transaction(v,[&](auto& s) {
        positive(n);auto& r=reservation(s,key);
        if (r.phase!=OriginalGoodsTransferPhase::Reserved || n>r.reserved) invalid("invalid original goods load");
        set_stock(s,v,r.source,r.good,std::int64_t(stock(v,r.source,r.good))-n);
        r.loaded=n;r.phase=OriginalGoodsTransferPhase::Loaded;
        const auto at=std::lower_bound(s.goods.cargo.begin(),s.goods.cargo.end(),key.courier_id,
            [](const OriginalGoodsCargo& a,CourierId id) { return a.transfer.courier_id<id; });
        s.goods.cargo.insert(at,{key,r.good,r.source,r.target,n});
    });
}
OriginalGoodsLedgerSnapshot original_goods_deliver(OriginalGoodsLedgerView v,OriginalGoodsTransferKey key,std::int32_t n) {
    return transaction(v,[&](auto& s) {
        positive(n);auto& r=reservation(s,key);auto& c=cargo(s,key);
        if (r.phase!=OriginalGoodsTransferPhase::Loaded || n>c.amount) invalid("invalid original goods arrival");
        set_stock(s,v,r.target,r.good,std::int64_t(stock(v,r.target,r.good))+n);
        c.amount-=n;r.delivered+=n;
        if (!c.amount) {r.phase=OriginalGoodsTransferPhase::Completed;std::erase_if(s.goods.cargo,
            [&](const auto& value) {return value.transfer==key;});}
    });
}
OriginalGoodsLedgerSnapshot original_goods_cancel(OriginalGoodsLedgerView v,OriginalGoodsTransferKey key) {
    return transaction(v,[&](auto& s) {
        auto& r=reservation(s,key);
        if (r.phase==OriginalGoodsTransferPhase::Reserved) r.phase=OriginalGoodsTransferPhase::Cancelled;
        else if (r.phase==OriginalGoodsTransferPhase::Loaded) r.phase=OriginalGoodsTransferPhase::Returning;
        else invalid("original goods transfer cannot be cancelled twice or after completion");
    });
}
OriginalGoodsLedgerSnapshot original_goods_return(OriginalGoodsLedgerView v,OriginalGoodsTransferKey key,std::int32_t n) {
    return transaction(v,[&](auto& s) {
        positive(n);auto& r=reservation(s,key);auto& c=cargo(s,key);
        if (r.phase!=OriginalGoodsTransferPhase::Returning || n>c.amount) invalid("invalid original goods return confirmation");
        set_stock(s,v,r.source,r.good,std::int64_t(stock(v,r.source,r.good))+n);
        c.amount-=n;r.returned+=n;
        if (!c.amount) {r.phase=OriginalGoodsTransferPhase::Returned;std::erase_if(s.goods.cargo,
            [&](const auto& value) {return value.transfer==key;});}
    });
}
} // namespace openemperor::simulation
