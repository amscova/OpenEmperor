#pragma once

// Private, on-demand QA telemetry. World and its retained actual histories are
// the only authority; this observer never issues commands or assigns gameplay.
#include "SustainableEconomyMetrics.h"
#include "HighTierProgressionMatrix.h"

#include <map>

namespace oe_high_tier_metrics {
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
namespace q=oe_starter_fixture;
namespace prior=oe_paid_progression_metrics;
namespace matrix=oe_high_tier_matrix;
using Json=nlohmann::json;
using q::check;

inline const sim::OriginalHousingHouse& history(const sim::World& w,sim::BuildingId id) {
    for(const auto& h:w.original_production_state()->housing.houses)if(h.building_id==id)return h;
    throw std::runtime_error("High-tier observer lost its actual paid House history");
}
inline const sim::OriginalFoodStock& food(const sim::World& w,sim::BuildingId id) {
    for(const auto& h:w.original_production_state()->food.stocks)if(h.building_id==id)return h;
    throw std::runtime_error("High-tier observer lost its actual House Food account");
}
inline const std::array<std::uint64_t,orig::goods_count>& consumed(const sim::World& w,sim::BuildingId id) {
    for(const auto& h:w.original_production_state()->house_consumption)if(h.building_id==id)return h.consumed;
    throw std::runtime_error("High-tier observer lost its actual House goods sink");
}
inline Json service_witness(const sim::OriginalServiceWitness& witness) {
    auto value=prior::service_witness(witness);value["house"]=static_cast<unsigned>(witness.building_id);return value;
}
inline Json water_witness(const std::optional<sim::OriginalWaterWitness>& witness) {
    if(!witness)return nullptr;
    return {{"house",static_cast<unsigned>(witness->building_id)},{"provider",static_cast<unsigned>(witness->provider_id)},
        {"courier",static_cast<unsigned>(witness->transfer.courier_id)},{"sequence",witness->transfer.sequence},
        {"arrival_tick",witness->arrival_tick}};
}
inline Json quality_witness(const std::optional<sim::OriginalFoodQualityWitness>& witness) {
    if(!witness)return nullptr;
    return {{"courier",static_cast<unsigned>(witness->transfer.courier_id)},{"sequence",witness->transfer.sequence},
        {"tick",witness->tick},{"accepted",witness->accepted},{"consumed_before",witness->consumed_before},
        {"previous_quality",witness->previous_quality}};
}
inline Json evaluation(const sim::OriginalHousingEvaluation& value,const orig::HouseEvolutionRules& rules,bool admission=false) {
    Json witnesses=Json::array();for(const auto& witness:value.service_witnesses)
        witnesses.push_back(witness?service_witness(*witness):Json(nullptr));
    const auto pure=orig::evolve_step(rules,value.input);
    return {{"tick",value.tick},{"input",matrix::state(value.input)},
        {"input_phase",admission?"Actual World immigration origin; not phase39. The pure kernel does not grant admission or materialization.":
            "Actual World phase39 PRE-evolution and PRE-consumption input; not a hypothetical populated row."},
        {"phase39_evaluation",!admission},
        {"current_needs",prior::requirements(rules,orig::house_row(value.input.type),value.input)},
        {"next_needs",prior::requirements(rules,orig::next_row(value.input.type),value.input)},
        {"pure_result",matrix::state(pure)},{"service_witnesses",witnesses},
        {"service_witness_order",{"Herbalist","Acupuncture","Ancestor","Confucian","Daoist","Buddhist","Music","Acrobat","Drama"}},
        {"water_evaluation_witness",water_witness(value.water_evaluation_witness)},
        {"food_quality_witness",quality_witness(value.food_quality_witness)},
        {"actual_food_received",value.food_received},{"actual_food_consumed_before",value.food_consumed},
        {"actual_goods_consumed_before",value.goods_consumed_before}};
}
inline Json transitions(const sim::World& w,sim::BuildingId id,const orig::HouseEvolutionRules& rules) {
    Json result=Json::array();const auto& h=history(w,id);
    for(std::size_t i=0;i<h.first_transitions.size();++i)if(const auto& first=h.first_transitions[i])
        result.push_back({{"type",i+3},{"kind",static_cast<unsigned>(first->kind)},
            {"admitted",first->admitted},{"evaluation",evaluation(first->evaluation,rules,first->kind==sim::OriginalHousingTransitionKind::Admission)}});
    return result;
}

// A bounded type/admission witness only. Root's acceptance must separately
// assess paid command provenance, Funds, 20400-tick retention and stability.
inline bool actual_first_target_success(const sim::World& w,sim::BuildingId id,orig::HouseType wanted) {
    if(!orig::is_house_type(wanted)||wanted<orig::HouseType::Shelter||wanted==orig::HouseType::EliteUnoccupied)return false;
    const auto& actual=q::house(w,id);const auto& proof=history(w,id).first_transitions[static_cast<unsigned>(wanted)-3];
    if(actual.type!=wanted||actual.occupants<=0||w.building_on_fire(id)||!proof||proof->evaluation.tick>w.ticks())return false;
    if(proof->kind==sim::OriginalHousingTransitionKind::Admission)
        return (wanted==orig::HouseType::Shelter||wanted==orig::HouseType::ModestSiheyuan)&&proof->admitted>0;
    const auto evolved=orig::evolve_step(w.original_rules_model()->house_rules(),proof->evaluation.input);
    return evolved.type==wanted&&proof->evaluation.input.type!=wanted;
}

class Observation {
public:
    Observation(const sim::World& w,sim::BuildingId target,const orig::HouseEvolutionRules& independent):
        city_(w),target_(target),rules_(independent),begin_(w.ticks()),before_tick_(w.ticks()),
        previous_(q::house(w,target)),minimum_funds_(w.treasury()),maximum_funds_(w.treasury()),
        minimum_population_(previous_.occupants),maximum_population_(previous_.occupants),
        maximum_type_(static_cast<unsigned>(previous_.type)),minimum_type_(maximum_type_) {
        check(w.original_sustainable_economy_enabled()&&independent.model_sha256()==w.original_rules_model()->provenance().sha256&&
            independent.difficulty()==w.original_rules_model()->difficulty(),"High-tier observer independent source/rule identity differs");
        initial_state_=matrix::state(previous_);initial_transitions_=transitions(w,target,rules_);
        const auto& h=history(w,target);for(std::size_t r=0;r<seen_first_.size();++r)seen_first_[r]=h.first_transitions[r].has_value();
        observe_transfers(w,false);before(w);
    }
    // Ordinary player commands may precede this call. Exactly one genuine
    // World tick must then occur before after(). No World snapshot is copied.
    void before(const sim::World& w) {
        city_.before(w);before_tick_=w.ticks();previous_=q::house(w,target_);
        food_before_=food(w,target_);goods_consumed_before_=consumed(w,target_);
        funds_before_=w.treasury();taxes_before_=w.taxes_collected_total();upkeep_before_=w.maintenance_spent_total();
        construction_before_=w.construction_spent_total();burning_before_=w.building_on_fire(target_);
        minimum_funds_=std::min(minimum_funds_,w.treasury());maximum_funds_=std::max(maximum_funds_,w.treasury());
        const auto dispatch=orig::next_scheduler_dispatch(w.ticks());
        if(dispatch&&dispatch->evolution)providers_before_=providers(w);else providers_before_=nullptr;
    }
    void after(const sim::World& w) {
        check(w.ticks()==before_tick_+1,"High-tier observer must bracket exactly one real World tick");
        city_.after(w);++ticks_;const auto& current=q::house(w,target_);const auto& h=history(w,target_);
        check(orig::is_elite(previous_.type)==orig::is_elite(current.type),"Observed paid House crossed families");
        minimum_funds_=std::min(minimum_funds_,w.treasury());maximum_funds_=std::max(maximum_funds_,w.treasury());
        minimum_population_=std::min<int>(minimum_population_,current.occupants);maximum_population_=std::max<int>(maximum_population_,current.occupants);
        maximum_type_=std::max(maximum_type_,static_cast<unsigned>(current.type));minimum_type_=std::min(minimum_type_,static_cast<unsigned>(current.type));
        ++type_ticks_[current.type];if(w.building_on_fire(target_))++burning_ticks_;
        if(burning_before_!=w.building_on_fire(target_))fire_events_.push_back({{"tick",w.ticks()},{"burning",w.building_on_fire(target_)}});
        observe_transfers(w,true);observe_visits(w);
        if(h.last_evaluation&&h.last_evaluation->tick==w.ticks())observe_evaluation(w,*h.last_evaluation);
        if(current.type!=previous_.type) {
            const auto& first=h.first_transitions[static_cast<unsigned>(current.type)-3];
            check(first.has_value(),"Actual type change lacks World transition evidence");
            const bool admission=first->kind==sim::OriginalHousingTransitionKind::Admission&&first->evaluation.tick==w.ticks()&&!previous_.occupants;
            if(admission)++admissions_;else if(current.type<previous_.type)++devolutions_;else ++upgrades_;
            transition_events_.push_back({{"tick",w.ticks()},{"from",static_cast<unsigned>(previous_.type)},
                {"to",static_cast<unsigned>(current.type)},{"first_for_type",first->evaluation.tick==w.ticks()},
                {"event_kind",admission?"actual_admission":current.type<previous_.type?"phase39_devolution":"phase39_evolution"},
                {"actual_state",matrix::state(current)},{"burning",w.building_on_fire(target_)},
                {"first_transition",{{"kind",static_cast<unsigned>(first->kind)},{"admitted",first->admitted},
                    {"evaluation",evaluation(first->evaluation,rules_,first->kind==sim::OriginalHousingTransitionKind::Admission)}}}});
        }
        for(std::size_t r=0;r<seen_first_.size();++r)if(!seen_first_[r]&&h.first_transitions[r]) {
            const auto& first=*h.first_transitions[r];check(first.evaluation.tick==w.ticks(),"New retained first-transition witness was not produced by this tick");
            seen_first_[r]=true;first_events_.push_back({{"tick",w.ticks()},{"type",r+3},
                {"kind",static_cast<unsigned>(first.kind)},{"admitted",first.admitted},
                {"evaluation",evaluation(first.evaluation,rules_,first.kind==sim::OriginalHousingTransitionKind::Admission)}});
        }
    }
    Json report(const sim::World& w)const {
        const auto diagnosis=w.original_housing_diagnosis(target_);auto input=diagnosis.actual;
        input.desirability=static_cast<std::int8_t>(diagnosis.cached_desirability);
        Json durations=Json::array();for(const auto& [type,count]:type_ticks_)
            durations.push_back({{"type",static_cast<unsigned>(type)},{"post_tick_count",count}});
        return {{"format","openemperor-high-tier-observation-v1"},{"target",static_cast<unsigned>(target_)},
            {"begin_tick",begin_},{"end_tick",w.ticks()},{"observed_ticks",ticks_},
            {"model_sha256",rules_.model_sha256()},{"difficulty",static_cast<unsigned>(rules_.difficulty())},
            {"independent_evolution_checks",true},{"world_mutation_by_observer",false},
            {"initial_state",initial_state_},{"final_state",matrix::state(diagnosis.actual)},
            {"initial_retained_transitions",initial_transitions_},{"final_retained_transitions",transitions(w,target_,rules_)},
            {"summary",{{"minimum_funds",minimum_funds_},{"maximum_funds",maximum_funds_},
                {"minimum_target_occupants",minimum_population_},{"maximum_target_occupants",maximum_population_},
                {"minimum_observed_type",minimum_type_},{"maximum_observed_type",maximum_type_},
                {"evaluations",evaluations_.size()},{"current_unmet_evaluations",current_unmet_},
                {"next_unmet_evaluations",next_unmet_},{"admissions",admissions_},{"upgrades",upgrades_},{"devolutions",devolutions_},
                {"burning_ticks",burning_ticks_},{"type_tick_counts",durations}}},
            {"evaluations",evaluations_},{"transition_events",transition_events_},{"new_first_transition_events",first_events_},
            {"target_delivery_events",deliveries_},{"target_visit_events",visits_},{"target_fire_events",fire_events_},
            {"upstream_material_events",upstream_},
            {"upstream_material_scope","Actual Hemp receipts at PaperMakers and Paper handoffs at Warehouses/Academies. Handoff units can include more than one transport leg of the same mass; they are not unique production. Auxiliary departure ticks come from World trips; goods departure time is only first observed loading and is unknown for a trip already in flight at observation start."},
            {"post_tick_current",matrix::row(diagnosis.current,input,rules_)},
            {"post_tick_next",matrix::row(diagnosis.next,input,rules_)},
            {"final_target_ledger",ledger(w)},{"city",city_.report(w)},
            {"claim_scope","Actual observed target transitions/arrivals and independent evaluation checks only. First reach, retention, nonnegative Funds and long-term stability are distinct acceptance decisions owned by the runner."}};
    }
private:
    Json providers(const sim::World& w)const {
        Json result=Json::array();
        for(const auto& b:w.buildings())if(w.workforce_required(b.id)>0||sim::original_influence_model(b.kind)) {
            Json material=nullptr,household_goods=nullptr;
            for(const auto& stock:w.original_production_state()->industry.auxiliary)if(stock.building_id==b.id)
                material={{"amounts",stock.amounts},{"received",stock.received},{"departed",stock.departed},
                    {"returned",stock.returned},{"consumed",stock.consumed}};
            for(const auto& stock:w.original_goods_state()->stocks)if(stock.building_id==b.id)household_goods=stock.amounts;
            Json actors=Json::array();for(const auto& c:w.couriers())if(c.owner==b.id) {
                const auto route=std::find_if(c.dynamic_target_routes.begin(),c.dynamic_target_routes.end(),[&](const auto& r){return r.first==target_;});
                actors.push_back({{"id",static_cast<unsigned>(c.id)},{"role",static_cast<unsigned>(c.role)},
                    {"phase",sim::courier_phase_name(c.phase)},{"active_target",static_cast<unsigned>(c.target)},
                    {"dispatch_status",sim::courier_dispatch_status_name(w.courier_dispatch_status(c.id).status)},
                    {"cached_route_to_target",route!=c.dynamic_target_routes.end()&&route->second.has_value()},
                    {"path_vertices",c.path.size()},{"path_vertex",c.path_vertex},{"edge_progress",c.edge_progress},
                    {"current_vertex",c.path_vertex<c.path.size()?matrix::cell(c.path[c.path_vertex]):Json(nullptr)},
                    {"begun_edge_to",c.edge_progress>0&&c.path_vertex+1<c.path.size()?matrix::cell(c.path[c.path_vertex+1]):Json(nullptr)},
                    {"route_pending",c.route_pending},{"native_cargo",c.cargo},
                    {"goods_cargo",w.original_courier_good(c.id)?w.original_courier_cargo(c.id):0},{"food_cargo",w.original_food_cargo(c.id)},
                    {"aux_cargo",w.original_auxiliary_cargo(c.id)}});
            }
            result.push_back({{"id",static_cast<unsigned>(b.id)},{"object",static_cast<unsigned>(b.kind)},
                {"cell",matrix::cell(b.cell)},{"operating",b.operating_enabled},{"burning",w.building_on_fire(b.id)},
                {"workers_required",w.workforce_required(b.id)},{"workers_assigned",w.workers_assigned(b.id)},
                {"priority",static_cast<unsigned>(b.workforce_priority)},
                {"installed_upkeep",std::int64_t(w.workforce_required(b.id))*sim::original_worker_upkeep_funds},
                {"upkeep_interval",w.original_worker_upkeep_interval()},
                {"native_progress",b.progress},{"native_output",b.output},{"native_input_clay",b.input_clay},
                {"actual_auxiliary_stock",material},{"actual_six_goods_stock",household_goods},{"couriers",actors}});
        }
        return result;
    }
    Json ledger(const sim::World& w)const {
        Json goods=Json::array(),aux=Json::array(),industry=Json::array(),food_farms=Json::array(),hemp_farms=Json::array();
        for(const auto& balance:w.original_goods_balances())goods.push_back({{"good",static_cast<unsigned>(balance.good)},
            {"produced",balance.totals.produced},{"imported",balance.totals.imported},{"consumed",balance.totals.consumed},
            {"demolished",balance.totals.demolished},{"stock",balance.stock},
            {"cargo",balance.cargo},{"source_reserved",balance.source_reserved},{"incoming_reserved",balance.incoming_reserved},
            {"conserved",balance.conserved}});
        for(unsigned a=0;a<sim::original_aux_material_count;++a) {
            const auto balance=w.original_auxiliary_balance(static_cast<sim::OriginalAuxMaterial>(a));
            aux.push_back({{"material",a},{"produced",balance.produced},{"consumed",balance.consumed},
                {"stock",balance.stock},{"cargo",balance.cargo},{"conserved",balance.conserved}});
        }
        for(const auto& source:w.original_production_state()->industry.sources)industry.push_back({{"owner",static_cast<unsigned>(source.building_id)},
            {"plant",source.plant_id?Json(static_cast<unsigned>(*source.plant_id)):Json(nullptr)},
            {"completed",source.completed},{"progress",source.progress},{"goods_inputs_used",source.goods_inputs_used},
            {"aux_inputs_used",source.aux_inputs_used},{"clay_received",source.clay_received},{"clay_used",source.clay_used}});
        for(const auto& farm:w.original_production_state()->food.farms)food_farms.push_back({{"owner",static_cast<unsigned>(farm.farm_id)},
            {"crop",static_cast<unsigned>(farm.crop)},{"harvested",farm.harvested}});
        for(const auto& farm:w.original_production_state()->hemp_farms)hemp_farms.push_back({{"owner",static_cast<unsigned>(farm.farm_id)},{"harvested",farm.harvested}});
        return {{"funds",w.treasury()},{"taxes",w.taxes_collected_total()},{"upkeep",w.maintenance_spent_total()},
            {"construction",w.construction_spent_total()},{"installed_workers",w.workforce_required()},
            {"active_workers",w.workforce_used()},{"target_tax_paid",w.building(target_).taxes_paid_total},
            {"target_food",prior::food_stock(w,target_)},{"goods_balances",goods},{"aux_balances",aux},{"industry",industry},
            {"food_farms",food_farms},{"hemp_farms",hemp_farms}};
    }
    void observe_evaluation(const sim::World& w,const sim::OriginalHousingEvaluation& actual) {
        const auto& current=q::house(w,target_);const auto pure=orig::evolve_step(rules_,actual.input);
        check(pure.type==current.type&&pure.devolve_counter==current.devolve_counter,
            "Actual target evaluation differs from independent unchanged HouseEvolutionRules");
        const auto dispatch=orig::next_scheduler_dispatch(w.ticks()-1);
        check(dispatch&&dispatch->evolution,"Retained actual evaluation appeared outside phase39");
        auto expected=pure;std::array<std::int32_t,orig::goods_count> expected_goods{};
        if(dispatch->consumption) {const auto debit=orig::consume_goods(rules_,pure);expected=debit.house;expected_goods=debit.consumed;}
        const auto& stock=food(w,target_);if(stock.amounts[0]+stock.amounts[1]==0)expected.food_quality=0;
        check(expected==current,"Post-tick target does not equal actual independent evolution/consumption and physical Food state");
        const auto diagnosis=w.original_housing_diagnosis(target_);auto post=current;
        post.desirability=static_cast<std::int8_t>(diagnosis.cached_desirability);
        const auto provenance=w.original_influence_provenance(target_);Json sources=Json::array();
        check(provenance.value==actual.input.desirability,"Target evaluation DES differs from actual paid cached source prefixes");
        for(const auto& source:provenance.sources)sources.push_back({{"id",static_cast<unsigned>(source.building_id)},
            {"type",static_cast<unsigned>(source.type)},{"origin",matrix::cell(source.origin)},
            {"footprint",Json::array({source.footprint.width,source.footprint.height})},
            {"before",source.before},{"contribution",source.contribution},{"after",source.after}});
        std::array<std::uint64_t,orig::goods_count> goods_delta{};const auto& used=consumed(w,target_);
        for(unsigned g=0;g<orig::goods_count;++g) {
            check(used[g]>=goods_consumed_before_[g],"Target goods consumption counter regressed");goods_delta[g]=used[g]-goods_consumed_before_[g];
            check(goods_delta[g]==static_cast<std::uint64_t>(expected_goods[g]),"Target scheduled goods debit differs from independent original consumption");
        }
        const auto current_needs=prior::requirements(rules_,orig::house_row(actual.input.type),actual.input);
        const auto next_needs=prior::requirements(rules_,orig::next_row(actual.input.type),actual.input);
        if(!current_needs.at("needs_met").get<bool>())++current_unmet_;
        if(!next_needs.at("needs_met").get<bool>())++next_unmet_;
        evaluations_.push_back({{"tick",w.ticks()},{"before_tick",matrix::state(previous_)},
            {"actual_evaluation",evaluation(actual,rules_)},{"after_evolution_and_consumption",matrix::state(current)},
            {"independent_evolution_and_consumption_match",true},{"goods_consumption_boundary",dispatch->consumption},
            {"actual_goods_consumed_this_tick",goods_delta},{"actual_food_amounts_before_tick",food_before_.amounts},
            {"actual_food_amounts_after_tick",stock.amounts},{"actual_food_misses_this_tick",stock.missed_demands-food_before_.missed_demands},
            {"post_tick_current",matrix::row(diagnosis.current,post,rules_)},{"post_tick_next",matrix::row(diagnosis.next,post,rules_)},
            {"actual_visits_and_availability_post_tick",prior::visits(w,target_,actual)},
            {"actual_des_selected_cell",matrix::cell(provenance.selected_cell)},{"actual_des_source_prefixes",sources},
            {"providers_before_tick",providers_before_},{"providers_after_tick",providers(w)},
            {"tick_ledger",{{"funds_before",funds_before_},{"funds_after",w.treasury()},
                {"tax_delta",w.taxes_collected_total()-taxes_before_},{"upkeep_delta",w.maintenance_spent_total()-upkeep_before_},
                {"construction_delta",w.construction_spent_total()-construction_before_}}},{"ledger_post_tick",ledger(w)}});
    }
    void observe_visits(const sim::World& w) {
        const auto visit=[&](const char* kind,const auto& value){if(value.target==target_&&value.arrival_tick==w.ticks())
            visits_.push_back({{"kind",kind},{"tick",w.ticks()},{"courier",static_cast<unsigned>(value.transfer.courier_id)},
                {"sequence",value.transfer.sequence},{"provider",static_cast<unsigned>(w.courier(value.transfer.courier_id).owner)},
                {"courier_role",static_cast<unsigned>(w.courier(value.transfer.courier_id).role)},{"dispatched_tick",value.dispatched_tick},
                {"travel_ticks",w.ticks()-value.dispatched_tick},{"visit_applied",value.visit_applied}});};
        for(const auto& trip:w.original_production_state()->services.trips)visit("service_or_religion",trip);
        for(const auto& trip:w.original_production_state()->water.trips)visit("water",trip);
    }
    struct Transfer {std::uint64_t sequence=0,delivered=0;sim::OriginalGoodsTransferPhase phase{};std::optional<std::uint64_t> departure;};
    void observe_transfers(const sim::World& w,bool count) {
        const auto record=[&](const char* family,const auto& trip,std::uint64_t delivered,const Json& units){
            if(trip.target!=target_)return;auto& previous_trip=transfers_[{family,trip.transfer.courier_id}];
            const bool fresh=previous_trip.sequence!=trip.transfer.sequence;
            if(fresh){previous_trip.sequence=trip.transfer.sequence;previous_trip.delivered=0;previous_trip.departure=count?std::optional{w.ticks()}:std::nullopt;}
            check(delivered>=previous_trip.delivered,"Target real transfer receipt regressed");
            if(count&&(delivered>previous_trip.delivered||fresh||trip.phase!=previous_trip.phase))deliveries_.push_back({{"family",family},
                {"tick",w.ticks()},{"courier",static_cast<unsigned>(trip.transfer.courier_id)},{"sequence",trip.transfer.sequence},
                {"source",static_cast<unsigned>(trip.source)},{"target",static_cast<unsigned>(trip.target)},
                {"phase",static_cast<unsigned>(trip.phase)},{"new_delivered_units",delivered-previous_trip.delivered},
                {"cumulative_trip_delivered",delivered},{"physical_units",units},
                {"departure_first_observed",previous_trip.departure?Json(*previous_trip.departure):Json(nullptr)},
                {"observed_travel_ticks",previous_trip.departure?Json(w.ticks()-*previous_trip.departure):Json(nullptr)},
                {"actual_arrival",delivered>previous_trip.delivered}});
            previous_trip.delivered=delivered;previous_trip.phase=trip.phase;
        };
        for(const auto& trip:w.original_goods_state()->reservations)record("goods",trip,static_cast<std::uint64_t>(trip.delivered),
            {{"good",static_cast<unsigned>(trip.good)},{"loaded",trip.loaded},{"delivered",trip.delivered},{"returned",trip.returned}});
        for(const auto& trip:w.original_production_state()->food.trips)record("food",trip,
            static_cast<std::uint64_t>(trip.delivered[0]+trip.delivered[1]),
            {{"loaded",trip.loaded},{"delivered",trip.delivered},{"returned",trip.returned}});
        const auto upstream=[&](const char* family,const auto& trip,std::uint64_t delivered,
            const Json& physical,const Json& receipt,std::optional<std::uint64_t> dispatched,
            std::optional<std::uint64_t> arrived) {
            auto& previous_trip=transfers_[{family,trip.transfer.courier_id}];
            const bool fresh=previous_trip.sequence!=trip.transfer.sequence;
            if(fresh) {
                previous_trip.sequence=trip.transfer.sequence;previous_trip.delivered=0;
                previous_trip.departure=dispatched?dispatched:count?std::optional{w.ticks()}:std::nullopt;
            }
            check(delivered>=previous_trip.delivered,"Actual upstream material receipt regressed");
            const auto delta=delivered-previous_trip.delivered;
            if(count&&delta&&arrived)check(*arrived==w.ticks(),"New upstream arrival contradicts actual retained World arrival tick");
            if(count&&(delta||fresh||trip.phase!=previous_trip.phase))upstream_.push_back({{"family",family},
                {"tick",w.ticks()},{"courier",static_cast<unsigned>(trip.transfer.courier_id)},
                {"sequence",trip.transfer.sequence},{"source",static_cast<unsigned>(trip.source)},
                {"target",static_cast<unsigned>(trip.target)},{"destination_object",static_cast<unsigned>(w.building(trip.target).kind)},
                {"phase",static_cast<unsigned>(trip.phase)},{"new_delivered_units",delta},
                {"cumulative_trip_delivered",delivered},{"physical_units",physical},
                {"actual_arrival",delta>0},{"actual_dispatched_tick",dispatched?Json(*dispatched):Json(nullptr)},
                {"actual_retained_arrival_tick",arrived?Json(*arrived):Json(nullptr)},
                {"departure_first_observed",previous_trip.departure?Json(*previous_trip.departure):Json(nullptr)},
                {"travel_ticks",delta&&previous_trip.departure?Json(w.ticks()-*previous_trip.departure):Json(nullptr)},
                {"elapsed_since_departure",previous_trip.departure?Json(w.ticks()-*previous_trip.departure):Json(nullptr)},
                {"travel_time_kind",dispatched?"actual retained World departure":previous_trip.departure?
                    "first observed loading; not an independently saved departure":"unknown: already in flight at observer start"},
                {"destination_receipt_post_tick",receipt}});
            previous_trip.delivered=delivered;previous_trip.phase=trip.phase;
        };
        for(const auto& trip:w.original_goods_state()->reservations) {
            if(trip.good!=orig::Goods::Hemp||w.building(trip.target).kind!=sim::Object::OriginalPaperMaker45)continue;
            Json receipt={{"authority","Actual OriginalGoodsReservation delivered quantity; six-goods ledger owns physical Hemp input."}};
            for(const auto& stock:w.original_goods_state()->stocks)if(stock.building_id==trip.target)receipt["six_goods_stock"]=stock.amounts;
            for(const auto& recipe:w.original_production_state()->industry.sources)if(recipe.building_id==trip.target) {
                receipt["actual_hemp_used_by_recipe"]=recipe.goods_inputs_used[static_cast<unsigned>(orig::Goods::Hemp)];
                receipt["completed_paper_recipes"]=recipe.completed;
            }
            upstream("hemp_to_paper",trip,static_cast<std::uint64_t>(trip.delivered),
                {{"good",static_cast<unsigned>(trip.good)},{"loaded",trip.loaded},{"delivered",trip.delivered},{"returned",trip.returned}},
                receipt,std::nullopt,std::nullopt);
        }
        for(const auto& trip:w.original_production_state()->industry.aux_trips) {
            if(trip.material!=sim::OriginalAuxMaterial::Paper)continue;
            Json receipt={{"authority","Actual OriginalAuxStock amounts/received/consumed; receipt is not inferred from dispatch or proximity."}};
            bool found=false;
            for(const auto& stock:w.original_production_state()->industry.auxiliary)if(stock.building_id==trip.target) {
                receipt["amounts"]=stock.amounts;receipt["received"]=stock.received;receipt["consumed"]=stock.consumed;
                receipt["departed"]=stock.departed;receipt["returned"]=stock.returned;found=true;break;
            }
            check(found,"Actual Paper transport lost its destination material authority");
            upstream("paper",trip,static_cast<std::uint64_t>(trip.delivered),
                {{"material",static_cast<unsigned>(trip.material)},{"loaded",trip.loaded},{"delivered",trip.delivered},{"returned",trip.returned}},
                receipt,trip.dispatched_tick,trip.arrival_tick);
        }
    }
    oe_sustainable_metrics::Observation city_;
    sim::BuildingId target_;const orig::HouseEvolutionRules& rules_;
    std::uint64_t begin_=0,before_tick_=0,ticks_=0,taxes_before_=0,upkeep_before_=0,construction_before_=0;
    std::uint64_t current_unmet_=0,next_unmet_=0,admissions_=0,upgrades_=0,devolutions_=0,burning_ticks_=0;
    orig::HouseState previous_;sim::OriginalFoodStock food_before_;
    std::array<std::uint64_t,orig::goods_count> goods_consumed_before_{};
    std::array<bool,orig::house_row_count> seen_first_{};
    std::int64_t funds_before_=0,minimum_funds_=0,maximum_funds_=0;
    int minimum_population_=0,maximum_population_=0;unsigned maximum_type_=0,minimum_type_=0;
    bool burning_before_=false;
    std::map<orig::HouseType,std::uint64_t> type_ticks_;
    std::map<std::pair<std::string,sim::CourierId>,Transfer> transfers_;
    Json initial_state_,initial_transitions_,providers_before_,evaluations_=Json::array(),transition_events_=Json::array(),
        first_events_=Json::array(),deliveries_=Json::array(),visits_=Json::array(),fire_events_=Json::array(),upstream_=Json::array();
};
} // namespace oe_high_tier_metrics
