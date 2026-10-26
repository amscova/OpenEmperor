#pragma once

// Explicit test telemetry. No observer state is gameplay/save authority; all
// events below are differences across one genuine production World::tick().
#include "HighTierProgressionMetrics.h"
#include "simulation/OriginalIndustryGoodsContext.h"

namespace oe_market_distribution_metrics {
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
namespace high=oe_high_tier_metrics;
namespace matrix=oe_high_tier_matrix;
using Json=nlohmann::json;
using oe_starter_fixture::check;

inline Json cells(std::span<const sim::Cell> path) {
    auto out=Json::array();for(const auto cell:path)out.push_back(matrix::cell(cell));return out;
}
inline Json goods_row(const sim::OriginalGoodsReservation& r) {
    return {{"sequence",r.transfer.sequence},{"source",static_cast<unsigned>(r.source)},
        {"target",static_cast<unsigned>(r.target)},{"good",static_cast<unsigned>(r.good)},
        {"phase",static_cast<unsigned>(r.phase)},{"loaded",r.loaded},{"delivered",r.delivered},{"returned",r.returned}};
}
inline bool traced(sim::CourierRole role) {
    return sim::original_industry_goods::typed(role)||role==sim::CourierRole::Clay||role==sim::CourierRole::Pottery;
}
inline Json owner(const sim::World& w,sim::BuildingId id) {
    const auto& b=w.building(id);Json goods=Json::array();
    if(sim::original_industry_goods::owner(b.kind))for(unsigned g=0;g<orig::goods_count;++g) {
        const auto good=static_cast<orig::Goods>(g);int stock=0,incoming=0,reserved=0,return_room=0,cargo=0,addressed=0;
        if(sim::original_house_kind(b.kind))stock=orig::goods_stock(oe_starter_fixture::house(w,id),good);
        else for(const auto& s:w.original_goods_state()->stocks)if(s.building_id==id)stock=s.amounts[g];
        for(const auto& r:w.original_goods_state()->reservations)if(r.good==good) {
            if(r.source==id&&r.phase==sim::OriginalGoodsTransferPhase::Reserved)reserved+=r.reserved;
            if(r.target==id) {
                if(r.phase==sim::OriginalGoodsTransferPhase::Reserved)incoming+=r.reserved;
                if(r.phase==sim::OriginalGoodsTransferPhase::Loaded)incoming+=r.loaded-r.delivered-r.returned;
            }
        }
        for(const auto& c:w.original_goods_state()->cargo)if(c.good==good) {
            if(c.source==id)return_room+=c.amount;
            if(c.target==id) {
                addressed+=c.amount;
                for(const auto& r:w.original_goods_state()->reservations)if(r.transfer==c.transfer&&
                    r.phase==sim::OriginalGoodsTransferPhase::Loaded)cargo+=c.amount;
            }
        }
        const auto native=b.kind==sim::Object::Warehouse&&good==orig::Goods::Ceramics?b.reserved_incoming:0;
        const auto capacity=sim::original_industry_goods::capacity(b.kind,good);
        goods.push_back({{"good",g},{"stock",stock},{"source_reserved",reserved},{"incoming_reserved",incoming},
            {"outgoing_cargo_return_room",return_room},{"incoming_cargo",cargo},{"addressed_cargo_including_returns",addressed},
            {"native_pottery_incoming",native},
            {"capacity",capacity},{"headroom",capacity-stock-incoming-return_room-native}});
    }
    Json consumed=nullptr,harvested=nullptr;
    for(const auto& s:w.original_production_state()->house_consumption)if(s.building_id==id)consumed=s.consumed;
    for(const auto& f:w.original_production_state()->hemp_farms)if(f.farm_id==id)harvested=f.harvested;
    return {{"id",static_cast<unsigned>(id)},{"object",static_cast<unsigned>(b.kind)},{"cell",matrix::cell(b.cell)},
        {"operating",b.operating_enabled},{"burning",w.building_on_fire(id)},{"workers_required",w.workforce_required(id)},
        {"workers_assigned",w.workers_assigned(id)},{"priority",static_cast<unsigned>(b.workforce_priority)},
        {"goods",goods},{"consumed",consumed},{"harvested",harvested},{"native_input_clay",b.input_clay},
        {"native_output",b.output},{"native_reserved_incoming",b.reserved_incoming},{"native_progress",b.progress},
        {"recipes_completed",b.recipes_completed},{"clay_extracted",b.clay_extracted}};
}
inline Json actor(const sim::World& w,const sim::CourierState& c,bool routes=true) {
    const auto decision=w.courier_dispatch_status(c.id);Json latest=nullptr,cached=Json::array();
    for(const auto& r:w.original_goods_state()->reservations)if(r.transfer.courier_id==c.id)latest=goods_row(r);
    if(routes)for(const auto& r:c.dynamic_target_routes)cached.push_back({{"target",static_cast<unsigned>(r.first)},
        {"path",r.second?cells(*r.second):Json(nullptr)}});
    return {{"id",static_cast<unsigned>(c.id)},{"owner",static_cast<unsigned>(c.owner)},{"role",static_cast<unsigned>(c.role)},
        {"phase",sim::courier_phase_name(c.phase)},{"target",static_cast<unsigned>(c.target)},
        {"last_target",c.last_dispatched_target?Json(static_cast<unsigned>(*c.last_dispatched_target)):Json(nullptr)},
        {"latest_goods_trip",latest},{"dispatch_status",sim::courier_dispatch_status_name(decision.status)},
        {"dispatch_selected_target",decision.selected_target?Json(static_cast<unsigned>(*decision.selected_target)):Json(nullptr)},
        {"path",cells(c.path)},{"path_vertex",c.path_vertex},{"edge_progress",c.edge_progress},
        {"route_pending",c.route_pending},{"cached_revision",c.cached_revision},
        {"native_cargo",c.cargo},{"native_reserved",c.reserved},{"typed_cargo",w.original_courier_good(c.id)?w.original_courier_cargo(c.id):0},
        {"cached_target_routes",cached}};
}
inline Json balances(const sim::World& w) {
    Json out=Json::array();for(const auto& b:w.original_goods_balances())out.push_back({{"good",static_cast<unsigned>(b.good)},
        {"produced",b.totals.produced},{"consumed",b.totals.consumed},{"imported",b.totals.imported},
        {"demolished",b.totals.demolished},{"stock",b.stock},{"cargo",b.cargo},{"source_reserved",b.source_reserved},
        {"incoming_reserved",b.incoming_reserved},{"conserved",b.conserved}});
    return out;
}
class Observation {
    struct Prior {
        sim::CourierPhase phase{};sim::CourierDispatchStatus status{};sim::BuildingId target{};
        std::uint64_t sequence=0;int loaded=0,delivered=0,returned=0,cargo=0;
        std::optional<std::uint64_t> departure,arrival;std::size_t edges=0;
    };
public:
    Observation(const sim::World& w,sim::BuildingId target,const orig::HouseEvolutionRules& rules):
        begin_(w.ticks()),target_(target),rules_(rules) {
        for(const auto& c:w.couriers())if(traced(c.role)) {
            auto& p=prior_[c.id];p.phase=c.phase;p.status=w.courier_dispatch_status(c.id).status;p.target=c.target;
            p.cargo=c.cargo;p.loaded=c.cargo;
            for(const auto& r:w.original_goods_state()->reservations)if(r.transfer.courier_id==c.id) {
                p.sequence=r.transfer.sequence;p.loaded=r.loaded;p.delivered=r.delivered;p.returned=r.returned;
            }
            for(const auto& r:w.original_production_state()->industry.clay_trips)if(r.transfer.courier_id==c.id) {
                p.sequence=r.transfer.sequence;p.delivered=r.delivered;p.departure=r.dispatched_tick;p.arrival=r.arrival_tick;
            }
            if(c.path.size()>1)p.edges=c.path.size()-1;
        }
        for(const auto& h:w.original_economy_state()->houses)consumed_[h.building_id]=high::consumed(w,h.building_id);
        initial_=sample(w,true);before(w);
    }
    void before(const sim::World& w) {
        before_tick_=w.ticks();
        for(const auto& h:w.original_economy_state()->houses)house_before_[h.building_id]=h.house;
    }
    void after(const sim::World& w) {
        check(w.ticks()==before_tick_+1,"Market trace must observe exactly one real tick");
        for(const auto& c:w.couriers())if(traced(c.role))observe_actor(w,c);
        const auto dispatch=orig::next_scheduler_dispatch(w.ticks()-1);check(dispatch.has_value(),"Trace lost actual scheduler dispatch");
        if(dispatch->consumption)for(const auto& h:w.original_economy_state()->houses) {
            const auto& now=high::consumed(w,h.building_id);auto delta=now;
            for(unsigned g=0;g<orig::goods_count;++g){check(now[g]>=consumed_[h.building_id][g],"House goods sink regressed");delta[g]-=consumed_[h.building_id][g];}
            consumption_.push_back({{"tick",w.ticks()},{"house",static_cast<unsigned>(h.building_id)},
                {"before_tick",matrix::state(house_before_.at(h.building_id))},{"after_tick",matrix::state(h.house)},
                {"actual_consumed",delta},{"cumulative_consumed",now}});consumed_[h.building_id]=now;
        }
        if(dispatch->evolution) {
            const auto& e=high::history(w,target_).last_evaluation;
            check(e&&e->tick==w.ticks(),"Market trace missed the real target phase39 input");
            auto value=sample(w,true);value["actual_evaluation"]=high::evaluation(*e,rules_);evaluations_.push_back(std::move(value));
        }
        if(w.ticks()>=27733&&w.ticks()<=29467)window_.push_back(sample(w,false));
    }
    Json report(const sim::World& w)const {
        return {{"format","openemperor-market-distribution-trace-v1"},{"begin_tick",begin_},{"end_tick",w.ticks()},
            {"target",static_cast<unsigned>(target_)},{"model_sha256",rules_.model_sha256()},{"rule",w.rule_version()},
            {"world_mutation_by_observer",false},{"initial",initial_},{"final",sample(w,true)},
            {"transfer_events",events_},{"dispatch_status_events",statuses_},{"evaluation_samples",evaluations_},
            {"consumption_events",consumption_},{"detailed_window",window_},
            {"timing_contract","Events are observed post tick. A new typed sequence is a dispatch/load in that exact tick; source production precedes dispatch and House consumption follows arrival. Post-tick stocks are not claimed as stocks immediately before loading. Native Pottery has no saved trip sequence. Trips already in flight at observer start have unknown departure unless World retains it."}};
    }
private:
    Json sample(const sim::World& w,bool complete)const {
        Json owners=Json::array(),actors=Json::array(),houses=Json::array();
        for(const auto& b:w.buildings())if(sim::original_industry_goods::owner(b.kind)||b.kind==sim::Object::ClaySource||b.kind==sim::Object::Pottery) {
            if(complete||!sim::original_house_kind(b.kind)||b.id==target_)owners.push_back(owner(w,b.id));
        }
        for(const auto& c:w.couriers())if(complete||traced(c.role))actors.push_back(actor(w,c,complete));
        if(complete)for(const auto& h:w.original_economy_state()->houses) {
            const auto& retained=high::history(w,h.building_id).last_evaluation;
            houses.push_back({{"house",static_cast<unsigned>(h.building_id)},
                {"state",matrix::state(h.house)},
                {"post_tick_current_needs",high::prior::requirements(rules_,orig::house_row(h.house.type),h.house)},
                {"post_tick_next_needs",high::prior::requirements(rules_,orig::next_row(h.house.type),h.house)},
                {"actual_phase39_evaluation",retained&&retained->tick==w.ticks()?high::evaluation(*retained,rules_):Json(nullptr)}});
        }
        return {{"tick",w.ticks()},{"funds",w.treasury()},{"taxes",w.taxes_collected_total()},
            {"upkeep",w.maintenance_spent_total()},{"owners",owners},{"couriers",actors},{"houses",houses},
            {"goods_balances",balances(w)}};
    }
    void observe_actor(const sim::World& w,const sim::CourierState& c) {
        auto& old=prior_.at(c.id);const auto status=w.courier_dispatch_status(c.id).status;
        if(status!=old.status)statuses_.push_back({{"tick",w.ticks()},{"courier",static_cast<unsigned>(c.id)},
            {"previous",sim::courier_dispatch_status_name(old.status)},{"actor",actor(w,c,false)}});
        old.status=status;Json row=nullptr;const char* family=nullptr;std::uint64_t seq=0;int delivered=0,returned=0;
        if(sim::original_industry_goods::typed(c.role)) {
            family="goods";for(const auto& r:w.original_goods_state()->reservations)if(r.transfer.courier_id==c.id) {
                row=goods_row(r);seq=r.transfer.sequence;delivered=r.delivered;returned=r.returned;break;
            }
        } else if(c.role==sim::CourierRole::Clay) {
            family="native_clay";for(const auto& r:w.original_production_state()->industry.clay_trips)if(r.transfer.courier_id==c.id) {
                seq=r.transfer.sequence;delivered=r.delivered;row={{"sequence",seq},{"source",static_cast<unsigned>(r.source)},
                    {"target",static_cast<unsigned>(r.target)},{"good",nullptr},{"phase",sim::courier_phase_name(c.phase)},
                    {"loaded",r.loaded},{"delivered",r.delivered},{"returned",0}};break;
            }
        } else {
            family="native_pottery";delivered=old.delivered;
            if(old.phase==sim::CourierPhase::IdleAtWorkshop&&c.phase==sim::CourierPhase::ToWarehouse) {
                old.loaded=c.cargo;delivered=0;
            }
            if(old.phase==sim::CourierPhase::ToWarehouse&&c.phase==sim::CourierPhase::Returning)delivered=old.cargo;
            row={{"sequence",nullptr},{"source",static_cast<unsigned>(c.owner)},
                {"target",static_cast<unsigned>(c.target)},{"good",static_cast<unsigned>(orig::Goods::Ceramics)},
                {"phase",sim::courier_phase_name(c.phase)},{"loaded",old.loaded},
                {"delivered",delivered},{"returned",0}};
        }
        const bool fresh=!row.is_null()&&(family==std::string_view("native_pottery")?
            old.phase==sim::CourierPhase::IdleAtWorkshop&&c.phase==sim::CourierPhase::ToWarehouse:seq!=old.sequence);
        if(fresh){old.departure=w.ticks();old.arrival.reset();old.edges=c.path.size()-1;old.delivered=0;old.returned=0;}
        const auto new_delivery=delivered-old.delivered,new_return=returned-old.returned;
        if(!row.is_null()&&(fresh||new_delivery>0||new_return>0||old.phase!=c.phase)) {
            const char* event=fresh?"dispatch_load":new_delivery>0?"delivery":new_return>0?"source_return":
                c.phase==sim::CourierPhase::IdleAtWorkshop?"courier_home":"cancel_return";
            if(new_delivery>0)old.arrival=w.ticks();
            row["family"]=family;row["event"]=event;row["tick"]=w.ticks();row["courier"]=static_cast<unsigned>(c.id);
            row["new_delivered_units"]=std::max(0,new_delivery);row["new_returned_units"]=std::max(0,new_return);
            row["departure_tick"]=old.departure?Json(*old.departure):Json(nullptr);
            row["arrival_tick"]=old.arrival?Json(*old.arrival):Json(nullptr);row["route_edges"]=old.edges;
            const auto edge=sim::profile_rules(w.profile(),w.rule_version()).courier_edge_ticks;
            row["minimum_one_way_ticks"]=old.edges*static_cast<unsigned>(edge);
            row["minimum_round_trip_ticks"]=old.edges*static_cast<unsigned>(edge)*2;
            row["actor"]=actor(w,c);row["source_post_tick"]=owner(w,c.owner);row["target_post_tick"]=owner(w,c.target);
            events_.push_back(std::move(row));
        }
        old.sequence=seq;old.delivered=delivered;old.returned=returned;old.phase=c.phase;old.target=c.target;old.cargo=c.cargo;
    }
    std::uint64_t begin_=0,before_tick_=0;sim::BuildingId target_;const orig::HouseEvolutionRules& rules_;
    std::map<sim::CourierId,Prior> prior_;
    std::map<sim::BuildingId,std::array<std::uint64_t,orig::goods_count>> consumed_;
    std::map<sim::BuildingId,orig::HouseState> house_before_;
    Json initial_,events_=Json::array(),statuses_=Json::array(),evaluations_=Json::array(),consumption_=Json::array(),window_=Json::array();
};
} // namespace oe_market_distribution_metrics
