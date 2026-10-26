#pragma once

// QA-only observations. World remains the only gameplay and save authority.
#include "PaidProgressionMetrics.h"
#include "PaidProgressionStrategy.h"

namespace oe_economy_fire_metrics {
namespace q=oe_starter_fixture;
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
namespace detail=oe_paid_progression_metrics;
using Json=nlohmann::json;
using q::check;

class Observation {
    struct Cost {
        std::uint64_t charged=0,burning_charge=0,paused_charge=0,unstaffed_ticks=0,burning_ticks=0,paused_ticks=0;
    };
    struct Home {
        std::uint64_t evaluations=0,current_unmet=0,next_unmet=0,up=0,down=0;
        std::map<std::string,std::uint64_t> current_fields,next_fields;
        Json last=nullptr;
    };
    struct Transfer {std::uint64_t sequence=0,delivered=0;sim::OriginalGoodsTransferPhase phase{};std::optional<std::uint64_t> first_seen;};
public:
    explicit Observation(const sim::World& w):begin_(w.ticks()),taxes_(w.taxes_collected_total()),upkeep_(w.maintenance_spent_total()),
        month_tax_(taxes_),month_upkeep_(upkeep_),minimum_(w.treasury()) {
        first_goal_=nullptr;first_negative_=nullptr;funds_20k_=nullptr;funds_goal_20k_=nullptr;
        if(w.original_goal_status().reached)first_goal_=oe_paid_progression_strategy::goal(w);
        previous_goal_=w.original_goal_status().reached;goal_minimum_=w.treasury();
        observe_transfers(w,false);
        for(const auto& stock:w.original_production_state()->food.stocks)food_misses_begin_[stock.building_id]=stock.missed_demands;
        for(const auto& b:w.buildings())burning_[b.id]=w.building_on_fire(b.id);
    }
    void commands(const Json& commands,const sim::World& w) {
        for(const auto& row:commands)controls_.push_back(row);
        if(!commands.empty())observe_transfers(w,true,false);
    }
    void before(const sim::World& w) {
        before_tick_=w.ticks();before_tax_=w.taxes_collected_total();before_upkeep_=w.maintenance_spent_total();
        due_=Json::array();assessed_=Json::array();expected_bill_=0;
        const auto next=w.ticks()+1;
        for(const auto& b:w.buildings()) {
            auto& cost=costs_[b.id];const auto workers=w.workforce_required(b.id);
            if(workers>0) {
                if(w.building_on_fire(b.id))++cost.burning_ticks;
                if(!b.operating_enabled)++cost.paused_ticks;
                if(b.operating_enabled&&!w.building_on_fire(b.id)&&w.workers_assigned(b.id)<workers)++cost.unstaffed_ticks;
            }
            if(b.placed&&next>b.placed_tick&&(next-b.placed_tick)%sim::original_worker_upkeep_interval_ticks==0) {
                const auto bill=static_cast<std::uint64_t>(workers*sim::original_worker_upkeep_funds);
                expected_bill_+=bill;cost.charged+=bill;
                if(w.building_on_fire(b.id))cost.burning_charge+=bill;
                if(!b.operating_enabled)cost.paused_charge+=bill;
                due_.push_back({{"id",static_cast<unsigned>(b.id)},{"kind",static_cast<unsigned>(b.kind)},
                    {"workers",workers},{"charge",bill},{"burning_before_tick",w.building_on_fire(b.id)},
                    {"paused_before_tick",!b.operating_enabled}});
            }
        }
        if(next%sim::original_tax_interval_ticks==0)for(const auto& h:w.original_economy_state()->houses) {
            const auto status=w.original_house_tax_status(h.building_id);
            const auto shadow=status.inhabited&&status.burning?static_cast<std::uint64_t>(status.occupants)*static_cast<unsigned>(status.rate):0;
            shadow_tax_+=shadow;
            assessed_.push_back({{"id",static_cast<unsigned>(h.building_id)},{"type",static_cast<unsigned>(h.house.type)},
                {"occupants",status.occupants},{"rate",status.rate},{"burning_before_tick",status.burning},
                {"paid_before",status.paid_total},{"shadow_excluded_at_observed_inputs",shadow}});
        }
    }
    void after(const sim::World& w) {
        check(w.ticks()==before_tick_+1,"Diagnosis observer must bracket one productive tick");
        const auto bill=w.maintenance_spent_total()-before_upkeep_,tax=w.taxes_collected_total()-before_tax_;
        check(bill==expected_bill_,"Actual invoice differs from installed-worker attribution");
        check(sim::original_finance_accounting_valid(w.treasury(),w.taxes_collected_total(),w.construction_spent_total(),w.maintenance_spent_total()),
            "Diagnosis observed unreconciled actual Funds");
        if(bill)invoices_.push_back({{"tick",w.ticks()},{"actual_total",bill},{"buildings",due_}});
        if(w.ticks()%sim::original_tax_interval_ticks==0) {
            std::uint64_t sum=0;
            for(auto& h:assessed_) {
                const auto id=static_cast<sim::BuildingId>(h.at("id").get<unsigned>());
                const auto paid=w.building(id).taxes_paid_total-h.at("paid_before").get<std::uint64_t>();
                sum+=paid;h["actual_tax"]=paid;
            }
            check(sum==tax,"Actual per-House tax payments differ from global ledger");
            months_.push_back({{"tick",w.ticks()},{"funds",w.treasury()},{"population",w.total_population()},
                {"actual_tax",w.taxes_collected_total()-month_tax_},{"actual_upkeep",w.maintenance_spent_total()-month_upkeep_},
                {"houses",assessed_},{"installed_bill_per_400",w.current_maintenance_rate()}});
            month_tax_=w.taxes_collected_total();month_upkeep_=w.maintenance_spent_total();
            q::authority(w);q::worker_oracle(w);
        }
        minimum_=std::min(minimum_,w.treasury());
        if(first_negative_.is_null()&&w.treasury()<0)first_negative_=w.ticks();
        const auto reached=w.original_goal_status().reached;
        if(first_goal_.is_null()&&reached){first_goal_=oe_paid_progression_strategy::goal(w);goal_minimum_=w.treasury();}
        if(!first_goal_.is_null()) {
            goal_minimum_=std::min(goal_minimum_,w.treasury());
            if(!reached)++goal_loss_ticks_;
            if(w.ticks()==first_goal_.at("tick").get<std::uint64_t>()+20000) {
                funds_goal_20k_=w.treasury();goal_20k_financeable_=goal_minimum_>=0;
                goal_window_={{"end",oe_paid_progression_strategy::goal(w)},
                    {"taxes_delta",w.taxes_collected_total()-first_goal_.at("taxes").get<std::uint64_t>()},
                    {"maintenance_delta",w.maintenance_spent_total()-first_goal_.at("maintenance").get<std::uint64_t>()},
                    {"minimum_funds",goal_minimum_}};
            }
        }
        if(reached!=previous_goal_){goal_changes_.push_back(oe_paid_progression_strategy::goal(w));previous_goal_=reached;}
        if(w.ticks()==begin_+20000)funds_20k_=w.treasury();
        for(const auto& b:w.buildings()) {
            const bool burning=w.building_on_fire(b.id);const bool was=burning_[b.id];
            if(burning!=was)fire_events_.push_back({{"tick",w.ticks()},{"id",static_cast<unsigned>(b.id)},
                {"burning",burning},{"funds",w.treasury()},{"eligible_goal",reached}});
            burning_[b.id]=burning;
        }
        const auto& rules=w.original_rules_model()->house_rules();
        for(const auto& history:w.original_production_state()->housing.houses) {
            if(!history.last_evaluation||history.last_evaluation->tick!=w.ticks())continue;
            const auto& input=history.last_evaluation->input;const auto& actual=q::house(w,history.building_id);
            auto& row=homes_[history.building_id];++row.evaluations;
            const auto current=detail::requirements(rules,orig::house_row(input.type),input);
            const auto next=detail::requirements(rules,orig::next_row(input.type),input);
            if(!current.at("needs_met").get<bool>())++row.current_unmet;
            if(!next.at("needs_met").get<bool>())++row.next_unmet;
            for(const auto& field:current.at("requirements"))if(!field.at("met").get<bool>())++row.current_fields[field.at("field").get<std::string>()];
            for(const auto& field:next.at("requirements"))if(!field.at("met").get<bool>())++row.next_fields[field.at("field").get<std::string>()];
            if(actual.type>input.type)++row.up;if(actual.type<input.type)++row.down;
            row.last={{"tick",w.ticks()},{"input",detail::state(input)},{"current",current},{"next",next},
                {"food",detail::food_stock(w,history.building_id)}};
        }
        observe_transfers(w,true);
    }
    Json report(const sim::World& w) const {
        Json costs=Json::array(),homes=Json::array();std::uint64_t burning_bill=0,paused_bill=0,misses=0;
        for(const auto& b:w.buildings()) {
            const auto found=costs_.find(b.id);const auto c=found==costs_.end()?Cost{}:found->second;
            burning_bill+=c.burning_charge;paused_bill+=c.paused_charge;
            costs.push_back({{"id",static_cast<unsigned>(b.id)},{"kind",static_cast<unsigned>(b.kind)},
                {"cell",{b.cell.x,b.cell.y}},{"placed_tick",b.placed_tick},{"installed_workers",w.workforce_required(b.id)},
                {"bill_per_400",w.workforce_required(b.id)*sim::original_worker_upkeep_funds},{"actual_charges",c.charged},
                {"charges_while_burning",c.burning_charge},{"charges_while_paused",c.paused_charge},
                {"burning_worker_owner_ticks",c.burning_ticks},{"paused_owner_ticks",c.paused_ticks},{"unstaffed_operating_owner_ticks",c.unstaffed_ticks}});
        }
        for(const auto& h:w.original_economy_state()->houses) {
            const auto found=homes_.find(h.building_id);const auto row=found==homes_.end()?Home{}:found->second;
            std::uint64_t food=0;
            for(const auto& s:w.original_production_state()->food.stocks)if(s.building_id==h.building_id) {
                const auto start=food_misses_begin_.find(h.building_id);food=s.missed_demands-(start==food_misses_begin_.end()?0:start->second);
            }
            misses+=food;
            homes.push_back({{"id",static_cast<unsigned>(h.building_id)},{"evaluations",row.evaluations},
                {"current_unmet",row.current_unmet},{"next_unmet",row.next_unmet},{"upgrades",row.up},{"devolutions",row.down},
                {"current_fields",row.current_fields},{"next_fields",row.next_fields},{"food_missed_demands",food},
                {"last_evaluation",row.last},{"final",detail::state(h.house)},
                {"taxes_paid_total",w.building(h.building_id).taxes_paid_total}});
        }
        const auto taxes=w.taxes_collected_total()-taxes_,maintenance=w.maintenance_spent_total()-upkeep_;
        check(taxes<=INT64_MAX&&maintenance<=INT64_MAX,"Diagnosis finance interval exceeds signed report domain");
        Json trailing=nullptr;
        if(w.ticks()-begin_>=20400) {
            const auto cutoff=w.ticks()-20400;std::uint64_t income=0,bill=0;
            for(const auto& row:months_)if(row.at("tick").get<std::uint64_t>()>cutoff)income+=row.at("actual_tax").get<std::uint64_t>();
            for(const auto& row:invoices_)if(row.at("tick").get<std::uint64_t>()>cutoff)bill+=row.at("actual_total").get<std::uint64_t>();
            check(income<=INT64_MAX&&bill<=INT64_MAX,"Trailing operating interval exceeds reporting domain");
            trailing={{"begin_exclusive",cutoff},{"end_inclusive",w.ticks()},{"ticks",20400},
                {"taxes",income},{"upkeep",bill},{"operating_balance",static_cast<std::int64_t>(income)-static_cast<std::int64_t>(bill)}};
        }
        const Json summary{{"first_goal_tick",first_goal_.is_null()?Json(nullptr):first_goal_.at("tick")},
            {"first_goal",first_goal_},{"goal_at_end",oe_paid_progression_strategy::goal(w)},
            {"first_negative_tick",first_negative_},{"funds_at_seed_plus_20000",funds_20k_},
            {"funds_at_goal_plus_20000",funds_goal_20k_},{"goal_20k_financeable",goal_20k_financeable_},
            {"funds_20k_nonnegative",funds_20k_.is_null()?Json(nullptr):Json(!first_negative_.is_number()||first_negative_.get<std::uint64_t>()>begin_+20000)},
            {"minimum_funds",minimum_},{"taxes_delta",taxes},{"maintenance_delta",maintenance},
            {"operating_balance",static_cast<std::int64_t>(taxes)-static_cast<std::int64_t>(maintenance)},
            {"goal_loss_ticks",goal_loss_ticks_},{"upkeep_while_burning",burning_bill},{"upkeep_while_paused",paused_bill},
            {"food_missed_demands",misses},{"goal_20000_window",goal_window_},{"trailing_20400",trailing}};
        return {{"format","openemperor-economy-fire-observation-v1"},{"begin_tick",begin_},{"end_tick",w.ticks()},
            {"observed_ticks",w.ticks()-begin_},{"summary",summary},{"months",months_},{"invoices",invoices_},
            {"building_costs",costs},{"supply_by_house",homes},{"goal_changes",goal_changes_},{"fire_events",fire_events_},
            {"controls",controls_},{"transport",transport_},
            {"shadow_tax_excluded_by_fire_at_observed_inputs",shadow_tax_},
            {"shadow_tax_is_actual_income",false},{"final",q::observe(w)},
            {"limitations",{"World::original_goal_status().reached is the sole goal predicate.",
                "Shadow tax uses observed burning-house inputs; it is hypothetical arithmetic, not a no-fire simulation.",
                "Needs aggregates exclude separate desirability/fire/operational gates.",
                "Transport duration is first observed booking/loading to credited arrival; trips already in flight have unknown start.",
                "Nonnegative treasury over a finite interval does not establish indefinitely sustainable finance."}}};
    }
private:
    void observe_transfers(const sim::World& w,bool count,bool visits=true) {
        const auto transfer=[&](const char* family,const auto& r,std::uint64_t delivered) {
            auto& prior=transfers_[{family,r.transfer.courier_id}];const bool fresh=prior.sequence!=r.transfer.sequence;
            if(fresh){prior.sequence=r.transfer.sequence;prior.delivered=0;prior.first_seen=count?std::optional{w.ticks()}:std::nullopt;}
            auto& totals=transport_[family];if(totals.is_null())totals={{"arrival_observations",0},{"delivered_units",0},{"returns",0},{"cancellations",0},
                {"timed_arrivals",0},{"travel_ticks_sum",0},{"maximum_travel_ticks",0}};
            if(count) {
                check(delivered>=prior.delivered,"Diagnosis actual delivery counter regressed");
                if(delivered>prior.delivered) {
                    totals["arrival_observations"]=totals["arrival_observations"].get<std::uint64_t>()+1;
                    totals["delivered_units"]=totals["delivered_units"].get<std::uint64_t>()+delivered-prior.delivered;
                    if(prior.first_seen) {
                        const auto elapsed=w.ticks()-*prior.first_seen;
                        totals["timed_arrivals"]=totals["timed_arrivals"].get<std::uint64_t>()+1;
                        totals["travel_ticks_sum"]=totals["travel_ticks_sum"].get<std::uint64_t>()+elapsed;
                        totals["maximum_travel_ticks"]=std::max(totals["maximum_travel_ticks"].get<std::uint64_t>(),elapsed);
                    }
                }
                if(fresh||r.phase!=prior.phase) {
                    if(r.phase==sim::OriginalGoodsTransferPhase::Returned)totals["returns"]=totals["returns"].get<unsigned>()+1;
                    if(r.phase==sim::OriginalGoodsTransferPhase::Cancelled)totals["cancellations"]=totals["cancellations"].get<unsigned>()+1;
                }
            }
            prior.delivered=delivered;prior.phase=r.phase;
        };
        for(const auto& r:w.original_goods_state()->reservations)transfer("goods",r,static_cast<std::uint64_t>(r.delivered));
        for(const auto& r:w.original_production_state()->food.trips)transfer("food",r,static_cast<std::uint64_t>(r.delivered[0]+r.delivered[1]));
        const auto visit=[&](const char* family,const auto& row) {
            if(!count||!visits||row.arrival_tick!=w.ticks())return;
            auto& totals=transport_[family];if(totals.is_null())totals={{"applied",0},{"not_applied",0},{"travel_ticks_sum",0}};
            const auto key=row.visit_applied?"applied":"not_applied";
            totals[key]=totals[key].template get<unsigned>()+1;
            totals["travel_ticks_sum"]=totals["travel_ticks_sum"].get<std::uint64_t>()+w.ticks()-row.dispatched_tick;
        };
        for(const auto& row:w.original_production_state()->services.trips)visit("services",row);
        for(const auto& row:w.original_production_state()->water.trips)visit("water",row);
    }
    std::uint64_t begin_,taxes_,upkeep_,month_tax_,month_upkeep_,before_tick_=0,before_tax_=0,before_upkeep_=0,expected_bill_=0;
    std::uint64_t shadow_tax_=0,goal_loss_ticks_=0;
    std::int64_t minimum_,goal_minimum_=0;
    bool previous_goal_=false;
    Json first_goal_,first_negative_,funds_20k_,funds_goal_20k_,goal_20k_financeable_=nullptr,goal_window_=nullptr;
    Json months_=Json::array(),invoices_=Json::array(),due_=Json::array(),assessed_=Json::array();
    Json controls_=Json::array(),goal_changes_=Json::array(),fire_events_=Json::array(),transport_=Json::object();
    std::map<sim::BuildingId,Cost> costs_;
    std::map<sim::BuildingId,Home> homes_;
    std::map<sim::BuildingId,bool> burning_;
    std::map<sim::BuildingId,std::uint64_t> food_misses_begin_;
    std::map<std::pair<std::string,sim::CourierId>,Transfer> transfers_;
};
} // namespace oe_economy_fire_metrics
