#pragma once

// Read-only, test-only observations of the paid Rule10 World. Every evaluation
// uses the production history's actual input; missing upgrade requirements are
// kept separate from actual consumption misses and failed transport bookings.
#include "OriginalLongplayMetrics.h"

#include <limits>
#include <map>
#include <optional>
#include <string>
#include <tuple>

namespace oe_paid_progression_metrics {
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
namespace prior=oe_longplay_metrics;
namespace starter=oe_starter_fixture;
using Json=nlohmann::json;

inline Json state(const orig::HouseState& h) {
    return {{"type",static_cast<unsigned>(h.type)},{"name",sim::original_housing_type_name(h.type)},
        {"occupants",h.occupants},{"desirability",int(h.desirability)},{"devolve_counter",h.devolve_counter},
        {"water",h.water},{"food_quality",h.food_quality},
        {"goods",{h.hemp,h.ceramics,h.tea,h.silk,h.bronzeware,h.lacquerware}},
        {"timers",{{"Herbalist",h.herbalist},{"Acupuncture",h.acupuncture},{"Music",h.music},
            {"Acrobat",h.acrobat},{"Drama",h.drama},{"Ancestor",h.ancestor},{"Confucian",h.confucian},
            {"Daoist",h.daoist},{"Buddhist",h.buddhist}}}};
}
inline Json requirements(const orig::HouseEvolutionRules& rules,orig::ModelHouse row,const orig::HouseState& h) {
    const auto actual=prior::values(h);Json needs=Json::array();
    for(std::size_t i=0;i<prior::fields.size();++i) {
        const auto need=rules.value(row,prior::fields[i]);const bool presence=i<9;
        needs.push_back({{"field",prior::field_names[i]},{"required",need},{"actual",actual[i]},
            {"presence",presence},{"met",presence ? need==0||actual[i]!=0:actual[i]>=need}});
    }
    return {{"model_row",static_cast<unsigned>(row)},{"model_label",orig::model_label(row)},
        {"desirability_low",rules.value(row,orig::HouseField::DesirabilityLow)},
        {"desirability_high",rules.value(row,orig::HouseField::DesirabilityHigh)},
        {"capacity",rules.row_capacity(row)},{"needs_met",orig::needs_met(rules,row,h)},
        {"requirements",std::move(needs)}};
}
inline Json food_stock(const sim::World& world,sim::BuildingId id) {
    for(const auto& row:world.original_production_state()->food.stocks)if(row.building_id==id)
        return {{"amounts",row.amounts},{"received",row.received},{"departed",row.departed},
            {"returned",row.returned},{"consumed",row.consumed},{"missed_demands",row.missed_demands}};
    return nullptr;
}
inline Json service_witness(const sim::OriginalServiceWitness& row) {
    return {{"provider",static_cast<unsigned>(row.provider_id)},{"courier",static_cast<unsigned>(row.transfer.courier_id)},
        {"sequence",row.transfer.sequence},{"arrival_tick",row.arrival_tick},{"role",static_cast<unsigned>(row.role)}};
}
inline Json visits(const sim::World& world,sim::BuildingId id,const sim::OriginalHousingEvaluation& evaluation) {
    Json result=Json::array();const auto& after=starter::house(world,id);
    for(unsigned i=0;i<9;++i) {
        const auto role=static_cast<orig::ServiceRole>(i);const auto available=world.original_service_availability(id,role);
        Json witness=nullptr;
        for(const auto& row:evaluation.service_witnesses)if(row&&row->role==role)witness=service_witness(*row);
        Json last=nullptr;if(available.last_arrival_tick)last=*available.last_arrival_tick;
        result.push_back({{"role",i},{"input_timer",orig::service_timer(evaluation.input,role)},
            {"after_timer",orig::service_timer(after,role)},{"evaluation_witness",std::move(witness)},
            {"last_actual_arrival_tick",std::move(last)},{"eligible_target",available.eligible_target},
            {"providers",available.provider_count},{"available_sources",available.available_source_count},
            {"reachable_sources",available.reachable_source_count},{"travelling_to_house",available.travelling_to_target_count}});
    }
    return result;
}

class Metrics {
public:
    explicit Metrics(const sim::World& world):begin_(world.ticks()),minimum_funds_(world.treasury()),maximum_funds_(world.treasury()) {
        starter::check(world.original_production_enabled(),"Paid progression observer requires an actual Rule10 World");
        before(world,false);observe_transfers(world,false);
    }
    // Call after ordinary purchases/controls and immediately before World::tick.
    void before(const sim::World& world,bool count_status=true) {
        prior_tick_=world.ticks();pre_funds_=world.treasury();pre_taxes_=world.taxes_collected_total();
        pre_maintenance_=world.maintenance_spent_total();staffing_before_=Json::array();
        minimum_funds_=std::min(minimum_funds_,world.treasury());maximum_funds_=std::max(maximum_funds_,world.treasury());
        for(const auto& b:world.buildings()) {
            staffing_before_.push_back({{"id",static_cast<unsigned>(b.id)},{"kind",static_cast<unsigned>(b.kind)},
                {"required",world.workforce_required(b.id)},{"assigned",world.workers_assigned(b.id)},
                {"enabled",b.operating_enabled},{"burning",world.building_on_fire(b.id)},
                {"priority",static_cast<unsigned>(b.workforce_priority)}});
        }
        for(const auto& h:world.original_economy_state()->houses) {
            auto& row=houses_[h.building_id];row.before=h.house;row.food_before=food_stock(world,h.building_id);
            row.maximum_type=std::max(row.maximum_type,static_cast<unsigned>(h.house.type));
            for(const auto& sink:world.original_production_state()->house_consumption)if(sink.building_id==h.building_id)
                row.consumed_before=sink.consumed;
        }
        for(const auto& c:world.couriers()) {
            const auto status=world.courier_dispatch_status(c.id).status;
            auto& row=couriers_[c.id];row.role=c.role;row.owner=c.owner;
            if(count_status)++row.status_ticks[status];
            if(count_status&&row.initialized&&row.status!=status)
                transport_events_.push_back({{"kind","dispatch_status_change"},{"tick",world.ticks()},
                    {"courier",static_cast<unsigned>(c.id)},{"owner",static_cast<unsigned>(c.owner)},
                    {"before",sim::courier_dispatch_status_name(row.status)},
                    {"after",sim::courier_dispatch_status_name(status)}});
            row.initialized=true;row.status=status;row.phase=c.phase;row.target=c.target;
            if(count_status&&c.route_pending)++row.route_pending_ticks;
        }
    }
    // Exactly one production tick must occur between before and after.
    void after(const sim::World& world) {
        starter::check(world.ticks()==prior_tick_+1,"Paid observer did not bracket exactly one actual tick");
        ++observed_ticks_;minimum_funds_=std::min(minimum_funds_,world.treasury());
        maximum_funds_=std::max(maximum_funds_,world.treasury());
        observe_transfers(world,true);observe_couriers(world);
        const auto& p=*world.original_production_state();const auto& rules=world.original_rules_model()->house_rules();
        bool framed=false;
        for(const auto& history:p.housing.houses) {
            auto& totals=houses_.at(history.building_id);const auto& h=starter::house(world,history.building_id);
            totals.maximum_type=std::max(totals.maximum_type,static_cast<unsigned>(h.type));
            const auto food=food_stock(world,history.building_id);
            const auto missed_before=totals.food_before.at("missed_demands").get<std::uint64_t>();
            const auto missed_after=food.at("missed_demands").get<std::uint64_t>();
            starter::check(missed_after>=missed_before,"Paid observer Food misses regressed");
            totals.food_misses+=missed_after-missed_before;
            if(!history.last_evaluation||history.last_evaluation->tick!=world.ticks())continue;
            if(!framed) {evaluation_frames_.push_back(frame(world));framed=true;}
            const auto& evaluation=*history.last_evaluation;const auto& input=evaluation.input;
            const auto current=requirements(rules,orig::house_row(input.type),input);
            const auto next=requirements(rules,orig::next_row(input.type),input);
            ++totals.evaluations;if(!current.at("needs_met").get<bool>())++totals.current_unmet;
            if(!next.at("needs_met").get<bool>())++totals.next_unmet;
            if(h.type>input.type)++totals.upgrades;if(h.type<input.type)++totals.devolutions;
            std::array<std::uint64_t,orig::goods_count> consumed{};
            for(const auto& sink:p.house_consumption)if(sink.building_id==history.building_id)
                for(std::size_t i=0;i<orig::goods_count;++i) {
                    starter::check(sink.consumed[i]>=totals.consumed_before[i],"Paid observer goods debit regressed");
                    consumed[i]=sink.consumed[i]-totals.consumed_before[i];
                }
            Json water=nullptr;
            for(const auto& row:p.water.houses)if(row.building_id==history.building_id) {
                water={{"normal_timer",row.normal_timer},{"special_timer",row.special_timer}};
                if(evaluation.water_evaluation_witness) {
                    const auto& w=*evaluation.water_evaluation_witness;
                    water["evaluation_witness"]={{"provider",static_cast<unsigned>(w.provider_id)},
                        {"courier",static_cast<unsigned>(w.transfer.courier_id)},{"sequence",w.transfer.sequence},
                        {"arrival_tick",w.arrival_tick}};
                }else water["evaluation_witness"]=nullptr;
            }
            Json quality=nullptr;
            if(evaluation.food_quality_witness) {
                const auto& w=*evaluation.food_quality_witness;
                quality={{"courier",static_cast<unsigned>(w.transfer.courier_id)},{"sequence",w.transfer.sequence},
                    {"tick",w.tick},{"accepted",w.accepted},{"consumed_before",w.consumed_before},
                    {"previous_quality",w.previous_quality}};
            }
            const auto dispatch=orig::next_scheduler_dispatch(world.ticks()-1);
            const bool consumption=dispatch&&dispatch->consumption&&input.occupants>0;
            Json shortfall=Json::array();
            if(consumption) {
                for(std::size_t i=0;i<orig::goods_count;++i) {
                    const auto requested=std::max(0,rules.value(orig::house_row(h.type),orig::goods_field(static_cast<orig::Goods>(i))));
                    starter::check(consumed[i]<=static_cast<std::uint64_t>(requested),"Observed goods consumption exceeds the actual row requirement");
                    const auto missing=static_cast<std::uint64_t>(requested)-consumed[i];
                    totals.goods_shortfall[i]+=missing;
                    if(missing)++totals.goods_shortfall_events[i];
                    shortfall.push_back(missing);
                }
            }
            events_.push_back({{"tick",world.ticks()},{"frame_index",evaluation_frames_.size()-1},
                {"house",static_cast<unsigned>(history.building_id)},
                {"before_tick",state(totals.before)},{"actual_evaluation_input",state(input)},
                {"after_evaluation_and_consumption",state(h)},
                {"current_needs_at_evaluation",current},{"next_needs_at_evaluation",next},
                {"current_needs_after_tick",requirements(rules,orig::house_row(h.type),h)},
                {"next_needs_after_tick",requirements(rules,orig::next_row(h.type),h)},
                {"food_before_tick",totals.food_before},{"food_after_tick",food},
                {"actual_food_missed_demand_delta",missed_after-missed_before},
                {"food_quality_witness",std::move(quality)},{"water",std::move(water)},
                {"services",visits(world,history.building_id,evaluation)},
                {"goods_consumption_boundary",consumption},{"goods_consumed_this_tick",consumed},
                {"actual_goods_shortfall_this_boundary",std::move(shortfall)},
                {"upgraded",h.type>input.type},{"devolved",h.type<input.type}});
        }
    }
    const Json& events() const {return events_;}
    const Json& evaluation_frames() const {return evaluation_frames_;}
    const Json& transport_events() const {return transport_events_;}
    Json report(const sim::World& world) const {
        Json houses=Json::array(),couriers=Json::array();std::uint64_t misses=0,current=0,next=0,up=0,down=0;
        for(const auto& [id,row]:houses_) {
            misses+=row.food_misses;current+=row.current_unmet;next+=row.next_unmet;up+=row.upgrades;down+=row.devolutions;
            houses.push_back({{"id",static_cast<unsigned>(id)},{"evaluations",row.evaluations},
                {"unmet_current_evaluations",row.current_unmet},{"unmet_next_evaluations",row.next_unmet},
                {"actual_food_missed_demands",row.food_misses},{"goods_shortfall_units",row.goods_shortfall},
                {"goods_shortfall_events",row.goods_shortfall_events},{"upgrades",row.upgrades},
                {"devolutions",row.devolutions},{"maximum_type",row.maximum_type}});
        }
        for(const auto& [id,row]:couriers_) {
            Json status=Json::object();for(const auto& [s,ticks]:row.status_ticks)status[sim::courier_dispatch_status_name(s)]=ticks;
            couriers.push_back({{"id",static_cast<unsigned>(id)},{"owner",static_cast<unsigned>(row.owner)},
                {"role",static_cast<unsigned>(row.role)},{"pre_tick_status_counts",status},
                {"dispatches",row.dispatches},{"outbound_legs_ended",row.arrivals},
                {"completed_round_trips",row.round_trips},{"route_pending_ticks",row.route_pending_ticks}});
        }
        return {{"format","openemperor-paid-progression-metrics-v1"},{"begin_tick",begin_},{"end_tick",world.ticks()},
            {"observed_ticks",observed_ticks_},{"evaluations",events_.size()},{"evaluation_frames",evaluation_frames_.size()},
            {"goods_order",{"Hemp","Ceramics","Tea","Silk","Bronzeware","Lacquerware"}},
            {"food_order",{"Wheat","Millet"}},{"service_role_order",{"Herbalist","Acupuncture","Music","Acrobat","Drama","Ancestor","Confucian","Daoist","Buddhist"}},
            {"minimum_funds",minimum_funds_},{"maximum_funds",maximum_funds_},{"houses",std::move(houses)},
            {"couriers",std::move(couriers)},{"actual_food_missed_demands",misses},
            {"unmet_current_evaluations",current},{"unmet_next_evaluations",next},{"upgrades",up},{"devolutions",down},
            {"successful_transfer_arrivals",transfer_arrivals_},{"cancelled_transfers",cancelled_transfers_},
            {"returned_transfers",returned_transfers_},{"observed_delivered_units",delivered_units_},
            {"actual_service_or_religion_visits",service_visits_},{"actual_water_visits",water_visits_},
            {"final",starter::observe(world)},
            {"limitations",{"Every actual House evaluation is recorded; no resource or visit state is injected.",
                "NoStock/NoRoad/Unstaffed are blocked dispatch observations, not failed delivered transactions.",
                "Unmet next-row needs are upgrade blockers, not consumption deadline misses.",
                "Trips already in flight when observing begins have unknown departure time.",
                "Finite samples do not establish universal balance or native/human acceptance."}}};
    }
private:
    struct HouseTotals {
        orig::HouseState before;Json food_before;
        std::array<std::uint64_t,orig::goods_count> consumed_before{},goods_shortfall{},goods_shortfall_events{};
        std::uint64_t evaluations=0,current_unmet=0,next_unmet=0,food_misses=0,upgrades=0,devolutions=0;
        unsigned maximum_type=0;
    };
    struct CourierTotals {
        sim::CourierRole role=sim::CourierRole::None;sim::BuildingId owner{};sim::BuildingId target{};
        sim::CourierPhase phase=sim::CourierPhase::IdleAtWorkshop;
        sim::CourierDispatchStatus status=sim::CourierDispatchStatus::Disabled;
        bool initialized=false;std::optional<std::uint64_t> dispatch_tick;
        std::uint64_t dispatches=0,arrivals=0,round_trips=0,route_pending_ticks=0;
        std::map<sim::CourierDispatchStatus,std::uint64_t> status_ticks;
    };
    struct Transfer {
        std::uint64_t sequence=0,delivered=0;
        std::optional<std::uint64_t> dispatched;
        sim::OriginalGoodsTransferPhase phase=sim::OriginalGoodsTransferPhase::Reserved;
    };
    Json frame(const sim::World& world) const {
        Json production=Json::array(),food=Json::array(),hemp=Json::array(),couriers=Json::array();
        Json food_stocks=Json::array(),goods_stocks=Json::array(),goods_balances=Json::array(),industry=Json::array();
        const auto& p=*world.original_production_state();
        for(const auto& b:world.buildings())production.push_back({{"id",static_cast<unsigned>(b.id)},
            {"progress",b.progress},{"input_clay",b.input_clay},{"output",b.output},{"recipes",b.recipes_completed},
            {"clay_extracted",b.clay_extracted},{"food_produced",b.food_produced}});
        for(const auto& farm:p.food.farms)food.push_back({{"farm",static_cast<unsigned>(farm.farm_id)},
            {"crop",static_cast<unsigned>(farm.crop)},{"harvested",farm.harvested}});
        for(const auto& farm:p.hemp_farms)hemp.push_back({{"farm",static_cast<unsigned>(farm.farm_id)},{"harvested",farm.harvested}});
        for(const auto& stock:p.food.stocks) {
            auto row=food_stock(world,stock.building_id);row["owner"]=static_cast<unsigned>(stock.building_id);
            food_stocks.push_back(std::move(row));
        }
        for(const auto& stock:world.original_goods_state()->stocks)
            goods_stocks.push_back({{"owner",static_cast<unsigned>(stock.building_id)},{"amounts",stock.amounts}});
        for(const auto& balance:world.original_goods_balances())goods_balances.push_back({
            {"good",static_cast<unsigned>(balance.good)},{"produced",balance.totals.produced},
            {"consumed",balance.totals.consumed},{"stock",balance.stock},{"cargo",balance.cargo},
            {"source_reserved",balance.source_reserved},{"incoming_reserved",balance.incoming_reserved},
            {"conserved",balance.conserved}});
        for(const auto& source:p.industry.sources)industry.push_back({{"owner",static_cast<unsigned>(source.building_id)},
            {"completed",source.completed},{"progress",source.progress},{"goods_inputs_used",source.goods_inputs_used},
            {"aux_inputs_used",source.aux_inputs_used},{"clay_received",source.clay_received},{"clay_used",source.clay_used}});
        for(const auto& c:world.couriers())couriers.push_back({{"id",static_cast<unsigned>(c.id)},
            {"owner",static_cast<unsigned>(c.owner)},{"target",static_cast<unsigned>(c.target)},
            {"role",static_cast<unsigned>(c.role)},{"phase",sim::courier_phase_name(c.phase)},
            {"dispatch_status",sim::courier_dispatch_status_name(world.courier_dispatch_status(c.id).status)},
            {"path_vertices",c.path.size()},{"path_vertex",c.path_vertex},{"edge_progress",c.edge_progress},
            {"native_cargo",c.cargo},{"goods_cargo",world.original_courier_good(c.id) ? world.original_courier_cargo(c.id):0},
            {"food_cargo",world.original_food_cargo(c.id)},{"aux_cargo",world.original_auxiliary_cargo(c.id)},
            {"route_pending",c.route_pending}});
        return {{"tick",world.ticks()},{"funds_before",pre_funds_},{"funds_after",world.treasury()},
            {"tax_delta_this_tick",world.taxes_collected_total()-pre_taxes_},
            {"maintenance_delta_this_tick",world.maintenance_spent_total()-pre_maintenance_},
            {"taxes_total",world.taxes_collected_total()},{"maintenance_total",world.maintenance_spent_total()},
            {"construction_total",world.construction_spent_total()},{"installed_upkeep_per_400t",world.current_maintenance_rate()},
            {"population",world.total_population()},{"staffing_before_tick",staffing_before_},
            {"production_after_tick",std::move(production)},{"food_harvests",std::move(food)},
            {"hemp_harvests",std::move(hemp)},{"food_stocks",std::move(food_stocks)},
            {"nonhouse_goods_stocks",std::move(goods_stocks)},{"goods_balances",std::move(goods_balances)},
            {"industry_after_tick",std::move(industry)},{"couriers_after_tick",std::move(couriers)}};
    }
    void observe_couriers(const sim::World& world) {
        for(const auto& c:world.couriers()) {
            auto& row=couriers_.at(c.id);
            if(row.phase==sim::CourierPhase::IdleAtWorkshop&&c.phase==sim::CourierPhase::ToWarehouse) {
                row.dispatch_tick=world.ticks();++row.dispatches;
                transport_events_.push_back({{"kind","courier_dispatch"},{"tick",world.ticks()},
                    {"courier",static_cast<unsigned>(c.id)},{"source",static_cast<unsigned>(c.owner)},
                    {"target",static_cast<unsigned>(c.target)},{"path_vertices",c.path.size()}});
            }
            if(row.phase==sim::CourierPhase::ToWarehouse&&c.phase==sim::CourierPhase::Returning) {
                ++row.arrivals;Json duration=nullptr;if(row.dispatch_tick)duration=world.ticks()-*row.dispatch_tick;
                transport_events_.push_back({{"kind","courier_outbound_leg_ended"},{"tick",world.ticks()},
                    {"courier",static_cast<unsigned>(c.id)},{"source",static_cast<unsigned>(c.owner)},
                    {"target",static_cast<unsigned>(row.target)},{"transport_ticks",std::move(duration)},
                    {"actual_delivery_not_inferred",true}});
            }
            if(row.phase==sim::CourierPhase::Returning&&c.phase==sim::CourierPhase::IdleAtWorkshop) {
                ++row.round_trips;Json duration=nullptr;if(row.dispatch_tick)duration=world.ticks()-*row.dispatch_tick;
                transport_events_.push_back({{"kind","courier_round_trip_complete"},{"tick",world.ticks()},
                    {"courier",static_cast<unsigned>(c.id)},{"elapsed_ticks",std::move(duration)}});row.dispatch_tick.reset();
            }
        }
    }
    void transfer(const sim::World& world,const char* family,sim::OriginalGoodsTransferKey key,
                  sim::BuildingId source,sim::BuildingId target,std::uint64_t delivered,
                  sim::OriginalGoodsTransferPhase phase,bool count,std::optional<std::uint64_t> actual_dispatch=std::nullopt) {
        auto& old=transfers_[{family,key.courier_id}];const bool new_trip=old.sequence!=key.sequence;
        if(new_trip) {old.sequence=key.sequence;old.delivered=0;old.dispatched=actual_dispatch;
            if(count&&!old.dispatched)old.dispatched=world.ticks();}
        if(count) {
            starter::check(delivered>=old.delivered,"Paid observer transfer delivery regressed");
            const auto delta=delivered-old.delivered;
            if(delta) {
                ++transfer_arrivals_;delivered_units_+=delta;Json duration=nullptr;
                if(old.dispatched)duration=world.ticks()-*old.dispatched;
                transport_events_.push_back({{"kind","actual_transfer_arrival"},{"family",family},
                    {"tick",world.ticks()},{"courier",static_cast<unsigned>(key.courier_id)},{"sequence",key.sequence},
                    {"source",static_cast<unsigned>(source)},{"target",static_cast<unsigned>(target)},
                    {"delivered_units",delta},{"transport_ticks",std::move(duration)}});
            }
            if((new_trip||old.phase!=phase)&&(phase==sim::OriginalGoodsTransferPhase::Cancelled||phase==sim::OriginalGoodsTransferPhase::Returned)) {
                if(phase==sim::OriginalGoodsTransferPhase::Cancelled)++cancelled_transfers_;else ++returned_transfers_;
                transport_events_.push_back({{"kind",phase==sim::OriginalGoodsTransferPhase::Cancelled ? "transfer_cancelled":"transfer_returned"},
                    {"family",family},{"tick",world.ticks()},{"courier",static_cast<unsigned>(key.courier_id)},
                    {"sequence",key.sequence},{"source",static_cast<unsigned>(source)},
                    {"target",static_cast<unsigned>(target)},{"delivered_units",delivered}});
            }
        }
        old.delivered=delivered;old.phase=phase;
    }
    void observe_transfers(const sim::World& world,bool count) {
        const auto& p=*world.original_production_state();
        for(const auto& row:world.original_goods_state()->reservations)
            transfer(world,"goods",row.transfer,row.source,row.target,static_cast<std::uint64_t>(row.delivered),row.phase,count);
        for(const auto& row:p.food.trips)
            transfer(world,"food",row.transfer,row.source,row.target,static_cast<std::uint64_t>(row.delivered[0]+row.delivered[1]),row.phase,count);
        for(const auto& row:p.industry.aux_trips)
            transfer(world,"aux",row.transfer,row.source,row.target,static_cast<std::uint64_t>(row.delivered),row.phase,count,row.dispatched_tick);
        for(const auto& row:p.industry.clay_trips)
            transfer(world,"clay",row.transfer,row.source,row.target,static_cast<std::uint64_t>(row.delivered),
                row.arrival_tick ? sim::OriginalGoodsTransferPhase::Completed:sim::OriginalGoodsTransferPhase::Loaded,count,row.dispatched_tick);
        const auto visit=[&](const char* family,const auto& row) {
            if(!count||!row.visit_applied||row.arrival_tick!=world.ticks())return;
            if(std::string_view(family)=="water")++water_visits_;else ++service_visits_;
            transport_events_.push_back({{"kind","actual_visit"},{"family",family},{"tick",world.ticks()},
                {"courier",static_cast<unsigned>(row.transfer.courier_id)},{"sequence",row.transfer.sequence},
                {"target",static_cast<unsigned>(row.target)},{"dispatched_tick",row.dispatched_tick},
                {"transport_ticks",world.ticks()-row.dispatched_tick}});
        };
        for(const auto& row:p.services.trips)visit("service_or_religion",row);
        for(const auto& row:p.water.trips)visit("water",row);
    }
    std::uint64_t begin_=0,prior_tick_=0,observed_ticks_=0,pre_taxes_=0,pre_maintenance_=0;
    std::uint64_t transfer_arrivals_=0,cancelled_transfers_=0,returned_transfers_=0,delivered_units_=0,service_visits_=0,water_visits_=0;
    std::int64_t pre_funds_=0,minimum_funds_=0,maximum_funds_=0;
    std::map<sim::BuildingId,HouseTotals> houses_;
    std::map<sim::CourierId,CourierTotals> couriers_;
    std::map<std::pair<std::string,sim::CourierId>,Transfer> transfers_;
    Json staffing_before_=Json::array(),events_=Json::array(),evaluation_frames_=Json::array(),transport_events_=Json::array();
};
} // namespace oe_paid_progression_metrics
