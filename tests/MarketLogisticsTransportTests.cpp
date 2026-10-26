// Reuse the same paid Paper/OR/service/navigation controls under the explicit
// successor, preserving the default Rule12 test executable unchanged.
#define OE_MARKET_LOGISTICS_AUTHORITY_TEST
#define main oe38_authority_reused_main
#include "HighTierProgressionAuthorityTests.cpp"
#undef main
#undef OE_MARKET_LOGISTICS_AUTHORITY_TEST

#include "simulation/OriginalIndustryGoodsContext.h"

namespace {
std::string market_model() {
    auto text=negative_model(37);
    oe_production_fixture::set_house_field(text,0,9,1);
    oe_production_fixture::set_house_field(text,0,10,1);
    oe_production_fixture::set_house_field(text,1,8,80);
    return text;
}
struct MarketScene {sim::BuildingId warehouse{},market{},farm{};};
MarketScene market_scene(sim::World& w,bool houses=true) {
    for(int x=0;x<24;++x)buy(w,sim::CommandType::PlaceRoad,{x,8});
    if(houses)for(int x=0;x<18;x+=3)(void)buy(w,sim::CommandType::PlaceHousehold,{x,6});
    (void)buy(w,sim::CommandType::PlaceClaySource,{0,9});
    (void)buy(w,sim::CommandType::PlacePottery,{3,9});
    const auto warehouse=buy(w,sim::CommandType::PlaceWarehouse,{6,9});
    const auto market=buy(w,sim::CommandType::PlaceMarket,{9,9});
    const auto farm=buy(w,sim::CommandType::PlaceOriginalHempFarm,{12,9});
    sim::Command field{sim::CommandType::PlaceOriginalHempField,{14,9}};field.building_id=farm;f::buy(w,field);
    return {warehouse,market,farm};
}
const sim::OriginalGoodsReservation& actual_trip(const sim::World& w,sim::CourierId id) {
    for(const auto& r:w.original_goods_state()->reservations)if(r.transfer.courier_id==id)return r;
    throw std::runtime_error("Rule13 actual typed actor lost its retained trip");
}
sim::CourierId typed_actor(const sim::World& w,sim::BuildingId owner,sim::CourierRole role) {
    for(const auto& c:w.couriers())if(c.owner==owner&&c.role==role)return c.id;
    throw std::runtime_error("Rule13 paid owner has no required typed actor");
}
void await_loaded(sim::World& w,sim::CourierId id) {
    until(w,[&]{const auto& c=w.courier(id);return c.phase==sim::CourierPhase::ToWarehouse&&c.edge_progress>0&&
        w.original_courier_cargo(id)>0;},"Rule13 paid production never loaded the requested physical typed courier");
}
void typed_pause_and_stale_cursor() {
    Fixture fixture(market_model());auto w=fixture.world();const auto scene=market_scene(w);
    const auto id=typed_actor(w,scene.market,sim::CourierRole::MarketPotteryDistribution);await_loaded(w,id);
    const auto begun=w.courier(id);const auto load=w.original_courier_cargo(id);const auto original=actual_trip(w,id);
    auto copy=disk_roundtrip(fixture,w,fixture.root.path/"rule13-loaded.json");
    check(copy.snapshot()==w.snapshot(),"Rule13 loaded complete Schema29 restore differs");
    const auto before=w.snapshot();perf::set_enabled(true);perf::reset();
    for(int i=0;i<32;++i)(void)w.courier_dispatch_status(id);
    for(const auto counter:{perf::Counter::WorldCopies,perf::Counter::WorldRestores,perf::Counter::WorldExecutes,
        perf::Counter::SimulationTicks,perf::Counter::BfsCalls,perf::Counter::RouteRefreshes,perf::Counter::FileReads,
        perf::Counter::FileWrites})check(perf::counter(counter)==0,"Rule13 cached selection query acquired hidden gameplay or navigation authority");
    perf::set_enabled(false);check(w.snapshot()==before,"Rule13 dispatch observation changed its saved cursor");
    f::buy(w,sim::set_building_operation(scene.market,false));
    check(w.courier(id).phase==sim::CourierPhase::Returning&&w.courier(id).edge_progress==begun.edge_progress&&
        w.original_courier_cargo(id)==load&&actual_trip(w,id).transfer==original.transfer&&
        actual_trip(w,id).phase==sim::OriginalGoodsTransferPhase::Returning,
        "Rule13 pause teleported a begun edge, changed its cursor or destroyed actual cargo");
    copy=disk_roundtrip(fixture,w,fixture.root.path/"rule13-returning.json");
    for(int i=0;i<1000&&w.courier(id).phase!=sim::CourierPhase::IdleAtWorkshop;++i) {
        w.tick();copy.tick();check(w.snapshot()==copy.snapshot(),"Rule13 loaded cancellation/return replay diverged");
    }
    check(w.courier(id).phase==sim::CourierPhase::IdleAtWorkshop&&!w.original_courier_cargo(id)&&
        actual_trip(w,id).returned==load&&actual_trip(w,id).delivered==0,
        "Rule13 actual cancelled load did not return to its reserved source room");
    f::buy(w,sim::set_building_operation(scene.market,true));
    until(w,[&]{return actual_trip(w,id).transfer.sequence>original.transfer.sequence;},
          "Rule13 paid resumed courier never advanced its real monotone cursor");
    const auto preserved=w.snapshot();const sim::original_industry_goods::Context context(w.buildings(),w.couriers());
    refuses([&]{(void)sim::original_goods_deliver(context.view(*w.original_goods_state(),w.original_economy_state()->houses),original.transfer,1);});
    auto hostile=w.snapshot();std::erase_if(hostile.original_goods->reservations,[&](const auto& r){return r.transfer.courier_id==id;});
    refuses([&]{(void)sim::World::restore_original_market_logistics(hostile,fixture.policy,fixture.model);});
    check(w.snapshot()==preserved,"Rule13 stale arrival/history rejection changed the actual World");
    authority(w);fixture.unchanged_source();
}
void typed_road_cut_and_paid_repair() {
    Fixture fixture(market_model());auto w=fixture.world();const auto scene=market_scene(w);
    const auto id=typed_actor(w,scene.warehouse,sim::CourierRole::MarketPotteryInbound);await_loaded(w,id);
    const auto begun=w.courier(id);const auto trip=actual_trip(w,id);const auto cargo=w.original_courier_cargo(id);
    const auto position=w.courier_position(id);std::optional<sim::Cell> cut;
    for(std::size_t i=begun.path_vertex+2;i<begun.path.size();++i) {
        const auto cell=begun.path[i];
        if(w.object_at(cell)==sim::Object::Road&&w.validate({sim::CommandType::RemoveRoad,cell}).accepted){cut=cell;break;}
    }
    check(cut.has_value(),"Rule13 paid route has no legal future-edge interruption");
    f::buy(w,{sim::CommandType::RemoveRoad,*cut});
    check(w.courier_position(id)==position&&w.courier(id).edge_progress==begun.edge_progress&&
        w.original_courier_cargo(id)==cargo&&actual_trip(w,id).transfer==trip.transfer,
        "Rule13 road cut moved a begun edge or changed the physical load/cursor");
    until(w,[&]{const auto& c=w.courier(id);return c.route_pending&&c.edge_progress==0&&
        c.route_checked_revision==w.road_revision();},"Rule13 disconnected loaded actor failed to wait after its begun edge",30);
    const auto attempts=w.courier(id).reroute_attempts;auto copy=disk_roundtrip(fixture,w,fixture.root.path/"rule13-road-wait.json");
    for(int i=0;i<20;++i){w.tick();copy.tick();check(w.snapshot()==copy.snapshot(),"Rule13 waiting cargo full-save replay diverged");}
    check(w.original_courier_cargo(id)==cargo&&w.courier(id).reroute_attempts==attempts&&actual_trip(w,id).delivered==0,
        "Rule13 unchanged road revision retried navigation or credited cargo without arrival");
    const auto funds=w.treasury();const auto spent=w.construction_spent_total();
    f::buy(w,{sim::CommandType::PlaceRoad,*cut});f::buy(copy,{sim::CommandType::PlaceRoad,*cut});
    check(w.treasury()==funds-2&&w.construction_spent_total()==spent+2,"Rule13 actual road repair was not paid");
    bool arrived=false;
    for(int i=0;i<300&&!arrived;++i) {
        w.tick();copy.tick();check(w.snapshot()==copy.snapshot(),"Rule13 actual paid road repair replay diverged");
        arrived=actual_trip(w,id).transfer==trip.transfer&&actual_trip(w,id).delivered==cargo;
    }
    check(arrived,"Rule13 repaired reserved shipment never arrived at its real Market");authority(w);fixture.unchanged_source();
}
void no_workers_and_natural_fire() {
    Fixture fixture(market_model());auto w=fixture.world();const auto scene=market_scene(w,false);
    const auto id=typed_actor(w,scene.market,sim::CourierRole::MarketPotteryDistribution);
    through(w,240);check(w.total_population()==0&&w.workers_assigned(scene.market)==0&&
        w.courier_dispatch_status(id).status==sim::CourierDispatchStatus::Unstaffed&&w.original_goods_state()->cargo.empty(),
        "Rule13 needs policy invented workers, production or cargo before actual immigration");
    for(int x=0;x<18;x+=3)(void)buy(w,sim::CommandType::PlaceHousehold,{x,6});
    await_loaded(w,id);through(w,2000);
    check(w.building_on_fire(scene.market),"Rule13 natural placement-relative Market fire was suppressed");
    until(w,[&]{return w.courier(id).phase==sim::CourierPhase::IdleAtWorkshop;},
          "Rule13 naturally burning Market did not finish its already begun trip",300);
    check(w.building_on_fire(scene.market)&&w.courier_dispatch_status(id).status==sim::CourierDispatchStatus::OnFire,
        "Rule13 naturally burning Market began a fresh goods dispatch");
    auto copy=disk_roundtrip(fixture,w,fixture.root.path/"rule13-market-fire.json");
    for(int i=0;i<30;++i){w.tick();copy.tick();check(w.snapshot()==copy.snapshot(),"Rule13 natural-fire full-save continuation diverged");}
    authority(w);fixture.unchanged_source();
}
}
int main() {
    try {
        typed_pause_and_stale_cursor();typed_road_cut_and_paid_repair();no_workers_and_natural_fire();
        paid_single_goods_or(false);paid_single_goods_or(true);paid_paper_confucian_and_expiry();
        paid_native_cargo_road_repair();paid_single_religion_and_natural_source_fire(true);
        paid_single_religion_and_natural_source_fire(false);
        std::cout<<"PASS Rule13/Schema29 actual typed load/return/repair, staffing/fire, stale cursor, Paper and both physical OR alternatives; authored controls only\n";
        return 0;
    }catch(const std::exception& e){perf::set_enabled(false);std::cerr<<"Rule13 transport FAIL: "<<e.what()<<'\n';return 1;}
}
