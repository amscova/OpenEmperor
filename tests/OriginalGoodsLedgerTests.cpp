#include "simulation/OriginalGoodsLedger.h"

#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
using Goods=orig::Goods;
constexpr auto source=static_cast<sim::BuildingId>(1),store=static_cast<sim::BuildingId>(2),home=static_cast<sim::BuildingId>(3);
constexpr auto courier=static_cast<sim::CourierId>(1),other_courier=static_cast<sim::CourierId>(2);
constexpr sim::OriginalGoodsTransferKey trip{courier,1},next_trip{courier,2},other_trip{other_courier,1};
void check(bool ok,const char* message) {if (!ok) throw std::runtime_error(message);}
struct Fixture {
    std::array<sim::OriginalGoodsOwner,3> owners{};
    std::array<sim::CourierId,2> couriers{courier,other_courier};
    sim::OriginalGoodsLedgerSnapshot state;
    Fixture() {
        owners[0]={source,false,{}};owners[1]={store,false,{}};owners[2]={home,true,{}};
        owners[0].capacity.fill(INT32_MAX);owners[1].capacity.fill(INT32_MAX);owners[2].capacity.fill(INT16_MAX);
        state.goods.stocks={{source,{}},{store,{}}};state.houses={{home}};
        state.houses[0].house.type=orig::HouseType::Hut;state.houses[0].house.occupants=4;
        state.houses[0].house.food_quality=17; // Unrelated fields must survive.
    }
    auto view() const {return state.view(owners,couriers);}
    void valid() const {sim::validate_original_goods_ledger(view());}
};
template<class F> void refuses(Fixture& f,F action,const char* reason) {
    const auto before=f.state;
    bool caught=false;
    try {(void)action();} catch (const std::exception&) {caught=true;}
    check(caught,reason);check(f.state==before,"Failed pure transaction mutated the entire caller input");
}
auto balance(const Fixture& f,Goods good) {return sim::inspect_original_goods_ledger(f.view())[static_cast<std::size_t>(good)];}
void all_six_lifecycles() {
    for (std::size_t i=0;i<orig::goods_count;++i) {
        const auto good=static_cast<Goods>(i);Fixture f;f.valid();
        const auto untouched=f.state;
        const auto produced=sim::original_goods_produce(f.view(),source,good,20);
        check(f.state==untouched,"Successful pure candidate mutated its caller");f.state=produced;
        f.state=sim::original_goods_import(f.view(),store,good,7);
        const auto physical=balance(f,good).stock;
        f.state=sim::original_goods_reserve(f.view(),other_trip,source,store,good,4);
        check(balance(f,good).stock==physical && balance(f,good).source_reserved==4 &&
            balance(f,good).incoming_reserved==4,"Reservation created or removed physical stock");
        f.state=sim::original_goods_cancel(f.view(),other_trip);
        refuses(f,[&]{return sim::original_goods_cancel(f.view(),other_trip);},"Duplicate pre-load cancel accepted");
        f.state=sim::original_goods_reserve(f.view(),trip,source,home,good,8);
        refuses(f,[&]{return sim::original_goods_reserve(f.view(),trip,source,home,good,8);},"Duplicate reservation accepted");
        f.state=sim::original_goods_load(f.view(),trip,5);
        check(balance(f,good).stock==physical-5 && balance(f,good).cargo==5 &&
            balance(f,good).source_reserved==0 && balance(f,good).incoming_reserved==5,
            "Partial loading failed to release source/unused target reservation exactly");
        refuses(f,[&]{return sim::original_goods_load(f.view(),trip,1);},"Transfer loaded twice");
        refuses(f,[&]{return sim::original_goods_return(f.view(),trip,1);},"Cargo returned before cancellation");
        f.state=sim::original_goods_deliver(f.view(),trip,2);
        check(orig::goods_stock(f.state.houses[0].house,good)==2 && balance(f,good).cargo==3 &&
            balance(f,good).incoming_reserved==3,"Partial arrival lost remaining cargo/reservation");
        f.state=sim::original_goods_cancel(f.view(),trip);
        check(balance(f,good).cargo==3 && balance(f,good).incoming_reserved==0,
            "Post-load cancel teleported/destroyed cargo or retained destination commitment");
        refuses(f,[&]{return sim::original_goods_cancel(f.view(),trip);},"Post-load cancel booked twice");
        refuses(f,[&]{return sim::original_goods_deliver(f.view(),trip,1);},"Cancelled transfer delivered");
        f.state=sim::original_goods_return(f.view(),trip,1);
        f.state=sim::original_goods_return(f.view(),trip,2);
        check(f.state.goods.cargo.empty() && balance(f,good).stock==physical,
            "Confirmed returns did not retain the complete physical balance");
        refuses(f,[&]{return sim::original_goods_return(f.view(),trip,1);},"Duplicate final return accepted");
        f.state=sim::original_goods_reserve(f.view(),next_trip,source,home,good,6);
        refuses(f,[&]{return sim::original_goods_deliver(f.view(),trip,1);},"Stale arrival booked against new courier trip");
        f.state=sim::original_goods_load(f.view(),next_trip,6);
        f.state=sim::original_goods_deliver(f.view(),next_trip,6);
        refuses(f,[&]{return sim::original_goods_deliver(f.view(),next_trip,1);},"Completed arrival booked twice");
        f.state=sim::original_goods_consume(f.view(),home,good,3);
        f.state=sim::original_goods_demolish(f.view(),store,good,2);
        const auto b=balance(f,good);
        check(b.totals.produced==20 && b.totals.imported==7 && b.totals.consumed==3 &&
            b.totals.demolished==2 && b.stock==22 && b.cargo==0 && b.conserved,
            "Independent six-goods input/output lifecycle is not conserved");
        check(f.state.houses[0].house.occupants==4 && f.state.houses[0].house.food_quality==17,
            "Goods transaction modified unrelated House authority");
        for (std::size_t j=0;j<orig::goods_count;++j) if (i!=j) {
            const auto other=balance(f,static_cast<Goods>(j));
            check(other.totals==sim::OriginalGoodsTotals{} && !other.stock && !other.cargo &&
                !other.source_reserved && !other.incoming_reserved,"Transaction changed unrelated good");
        }
        f.valid();
    }
}
void stock_reservation_errors() {
    Fixture f;
    refuses(f,[&]{return sim::original_goods_reserve(f.view(),trip,source,home,Goods::Ceramics,1);},"Missing stock reserved");
    refuses(f,[&]{return sim::original_goods_produce(f.view(),source,Goods::Ceramics,0);},"Zero amount accepted");
    refuses(f,[&]{return sim::original_goods_import(f.view(),source,Goods::Ceramics,-1);},"Negative amount accepted");
    refuses(f,[&]{return sim::original_goods_produce(f.view(),source,static_cast<Goods>(6),1);},"Invalid good accepted");
    refuses(f,[&]{return sim::original_goods_produce(f.view(),static_cast<sim::BuildingId>(99),Goods::Hemp,1);},"Unknown stock owner accepted");
    f.state=sim::original_goods_produce(f.view(),source,Goods::Ceramics,12);
    refuses(f,[&]{return sim::original_goods_reserve(f.view(),{static_cast<sim::CourierId>(99),1},source,home,Goods::Ceramics,1);},"Unknown courier reserved");
    refuses(f,[&]{return sim::original_goods_reserve(f.view(),trip,source,source,Goods::Ceramics,1);},"Self transfer accepted");
    f.owners[2].capacity[1]=4;
    refuses(f,[&]{return sim::original_goods_reserve(f.view(),trip,source,home,Goods::Ceramics,5);},"Full target overreserved");
    f.state=sim::original_goods_reserve(f.view(),trip,source,home,Goods::Ceramics,4);
    refuses(f,[&]{return sim::original_goods_reserve(f.view(),other_trip,source,home,Goods::Ceramics,1);},"Incoming capacity reserved twice");
    refuses(f,[&]{return sim::original_goods_consume(f.view(),source,Goods::Ceramics,9);},"Reserved source stock consumed");
    refuses(f,[&]{return sim::original_goods_import(f.view(),home,Goods::Ceramics,1);},"Credit stole already reserved target capacity");
    refuses(f,[&]{return sim::original_goods_load(f.view(),trip,5);},"Loaded more than reserved");
    f.state=sim::original_goods_load(f.view(),trip,4);
    refuses(f,[&]{return sim::original_goods_deliver(f.view(),trip,5);},"Delivered more than cargo");
    auto malformed=f;malformed.state.goods.cargo[0].good=Goods::Tea;
    refuses(malformed,[&]{return sim::original_goods_cancel(malformed.view(),trip);},"Wrong goods cargo survived validation");
    malformed=f;malformed.state.goods.stocks[0].amounts[5]=-1;
    refuses(malformed,[&]{return sim::original_goods_cancel(malformed.view(),trip);},"Unrelated negative stock hidden by cancellation");
    malformed=f;malformed.state.goods.stocks.push_back({home,{}});
    refuses(malformed,[&]{malformed.valid();},"House stock accepted a second authority");
    malformed=f;malformed.state.goods.cargo[0].transfer.sequence=2;
    refuses(malformed,[&]{malformed.valid();},"Orphan cargo accepted");
    malformed=f;malformed.state.goods.reservations.push_back(malformed.state.goods.reservations[0]);
    refuses(malformed,[&]{malformed.valid();},"Duplicate reservation reference accepted");
    malformed=f;malformed.state.goods.totals[0].produced=1;
    refuses(malformed,[&]{return sim::original_goods_cancel(malformed.view(),trip);},"Unrelated unbalanced good hidden by transaction");
}
void numeric_limits_and_return_capacity() {
    Fixture f;
    f.state=sim::original_goods_produce(f.view(),home,Goods::Hemp,INT16_MAX);
    refuses(f,[&]{return sim::original_goods_produce(f.view(),home,Goods::Hemp,1);},"Signed-word House overflow accepted");
    f.state=sim::original_goods_produce(f.view(),source,Goods::Ceramics,INT32_MAX);
    refuses(f,[&]{return sim::original_goods_produce(f.view(),source,Goods::Ceramics,1);},"Non-house signed-int overflow accepted");
    f.state=sim::original_goods_reserve(f.view(),trip,source,store,Goods::Ceramics,INT32_MAX);
    f.state=sim::original_goods_load(f.view(),trip,INT32_MAX);
    check(balance(f,Goods::Ceramics).cargo==static_cast<std::uint64_t>(INT32_MAX),"Maximum cargo narrowed");
    f.state=sim::original_goods_deliver(f.view(),trip,INT32_MAX);f.valid();
    Fixture maximum;maximum.state.goods.totals[0]={UINT64_MAX,0,UINT64_MAX,0};maximum.valid();
    refuses(maximum,[&]{return sim::original_goods_produce(maximum.view(),source,Goods::Hemp,1);},"Historical counter overflow accepted");
    auto bad=maximum;bad.state.goods.totals[0].imported=1;
    refuses(bad,[&]{bad.valid();},"Input aggregation wrapped UINT64_MAX");
    bad=maximum;bad.state.goods.totals[0].demolished=1;
    refuses(bad,[&]{bad.valid();},"Output aggregation wrapped UINT64_MAX");
    Fixture sequence;sequence.state.goods.reservations.push_back({{courier,UINT64_MAX},Goods::Hemp,source,home,
        sim::OriginalGoodsTransferPhase::Cancelled,1,0,0,0});sequence.valid();
    refuses(sequence,[&]{return sim::original_goods_reserve(sequence.view(),{courier,0},source,home,Goods::Hemp,1);},"Transfer sequence wrapped");
    Fixture returns;returns.owners[0].capacity[2]=20;
    returns.state=sim::original_goods_produce(returns.view(),source,Goods::Tea,20);
    returns.state=sim::original_goods_reserve(returns.view(),trip,source,home,Goods::Tea,10);
    returns.state=sim::original_goods_load(returns.view(),trip,10);
    returns.state=sim::original_goods_cancel(returns.view(),trip);
    returns.state=sim::original_goods_produce(returns.view(),source,Goods::Tea,10);
    refuses(returns,[&]{return sim::original_goods_return(returns.view(),trip,10);},"Return overflowed full source or erased cargo");
    returns.state=sim::original_goods_consume(returns.view(),source,Goods::Tea,10);
    returns.state=sim::original_goods_return(returns.view(),trip,10);returns.valid();
}
}
int main() {
    try {all_six_lifecycles();stock_reservation_errors();numeric_limits_and_return_capacity();
        std::cout<<"Original goods ledger: six authored lifecycles, commitments/history, atomic refusals and numeric bounds PASS\n";return 0;
    } catch (const std::exception& error) {std::cerr<<"Original goods ledger FAIL: "<<error.what()<<'\n';return 1;}
}
