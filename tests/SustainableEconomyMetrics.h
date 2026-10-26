#pragma once

// QA observation only: no clocks, stocks, money or service state are assigned.
#include "SustainableEconomyStrategy.h"
#include "PaidProgressionMetrics.h"

namespace oe_sustainable_metrics {
namespace q=oe_starter_fixture;
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
namespace house=oe_paid_progression_metrics;
using Json=nlohmann::json;
using q::check;
inline std::uint64_t interval(const sim::World& w){
#ifdef OE37_BASELINE
    (void)w;return 400;
#else
    return w.original_worker_upkeep_interval();
#endif
}
class Observation {
    struct Transfer {std::uint64_t sequence=0,delivered=0;std::optional<std::uint64_t> started;sim::OriginalGoodsTransferPhase phase{};};
public:
    explicit Observation(const sim::World& w):begin_(w.ticks()),start_tax_(w.taxes_collected_total()),start_upkeep_(w.maintenance_spent_total()),start_construction_(w.construction_spent_total()),
        window_tax_(start_tax_),window_upkeep_(start_upkeep_),window_construction_(start_construction_),
        minimum_(w.treasury()),window_minimum_(w.treasury()),window_funds_(w.treasury()) {
        const auto status=w.original_goal_status();
        minimum_inhabited_houses_=status.inhabited_houses;minimum_population_=status.population;
        if(status.reached)first_goal_=oe_paid_progression_strategy::goal(w);
        previous_goal_=status.reached;
        for(const auto& b:w.buildings())burning_[b.id]=w.building_on_fire(b.id);
        for(const auto& stock:w.original_production_state()->food.stocks)misses_begin_[stock.building_id]=stock.missed_demands;
        observe_transfers(w,false);
    }
    void before(const sim::World& w){
        before_tick_=w.ticks();before_funds_=w.treasury();before_tax_=w.taxes_collected_total();before_upkeep_=w.maintenance_spent_total();
        expected_bill_=0;const auto next=w.ticks()+1;
        for(const auto& b:w.buildings())if(b.placed&&next>b.placed_tick&&(next-b.placed_tick)%interval(w)==0)
            expected_bill_+=static_cast<std::uint64_t>(w.workforce_required(b.id))*sim::original_worker_upkeep_funds;
    }
    void after(const sim::World& w){
        check(w.ticks()==before_tick_+1,"Sustainable observer must bracket one real tick");
        const auto tax=w.taxes_collected_total()-before_tax_,bill=w.maintenance_spent_total()-before_upkeep_;
        check(bill==expected_bill_,"Installed-worker invoice differs from actual selected-rule ledger");
        check(w.treasury()==before_funds_+static_cast<std::int64_t>(tax)-static_cast<std::int64_t>(bill),"Tick Funds were not conserved");
        minimum_=std::min(minimum_,w.treasury());window_minimum_=std::min(window_minimum_,w.treasury());
        if(w.treasury()<0&&first_negative_.is_null())first_negative_=w.ticks();
        if(before_funds_<0&&w.treasury()>=0)recoveries_.push_back(w.ticks());
        const auto status=w.original_goal_status();const auto reached=status.reached;
        minimum_inhabited_houses_=std::min(minimum_inhabited_houses_,status.inhabited_houses);
        minimum_population_=std::min(minimum_population_,status.population);
        if(reached&&first_goal_.is_null())first_goal_=oe_paid_progression_strategy::goal(w);
        if(!reached)++goal_loss_ticks_;
        if(reached!=previous_goal_){goal_changes_.push_back(oe_paid_progression_strategy::goal(w));previous_goal_=reached;}
        for(const auto& b:w.buildings()) {
            const auto now=w.building_on_fire(b.id);if(now&&!burning_[b.id])++fire_starts_;burning_[b.id]=now;
        }
        if(w.original_fire_protection_enabled())for(auto it=w.original_production_state()->fire_protection->arrivals.rbegin();
            it!=w.original_production_state()->fire_protection->arrivals.rend()&&it->tick==w.ticks();++it) {
            ++inspector_arrivals_;if(it->extinguished)++extinguishments_;
            fire_visits_.push_back({{"tick",it->tick},{"owner",static_cast<unsigned>(it->trip.owner)},
                {"target",static_cast<unsigned>(it->trip.target)},{"courier",static_cast<unsigned>(it->trip.courier_id)},
                {"extinguished",it->extinguished}});
        }
        observe_transfers(w,true);
        for(const auto& history:w.original_production_state()->housing.houses)if(history.last_evaluation&&history.last_evaluation->tick==w.ticks()) {
            const auto& e=*history.last_evaluation;const auto& h=q::house(w,history.building_id);const auto& rules=w.original_rules_model()->house_rules();
            if(h.type<e.input.type)++devolutions_;if(h.type>e.input.type)++upgrades_;
            Json current=Json::array(),next=Json::array();const auto values=oe_longplay_metrics::values(e.input);
            for(std::size_t i=0;i<oe_longplay_metrics::fields.size();++i) {
                const auto need=rules.value(orig::house_row(e.input.type),oe_longplay_metrics::fields[i]);
                const auto wanted=rules.value(orig::next_row(e.input.type),oe_longplay_metrics::fields[i]);
                if(i<9 ? need!=0&&values[i]==0:values[i]<need){current.push_back(oe_longplay_metrics::field_names[i]);++current_unmet_[history.building_id][oe_longplay_metrics::field_names[i]];}
                if(i<9 ? wanted!=0&&values[i]==0:values[i]<wanted){next.push_back(oe_longplay_metrics::field_names[i]);++next_unmet_[history.building_id][oe_longplay_metrics::field_names[i]];}
            }
            if(!current.empty())++current_unmet_evaluations_;
            if(!next.empty())++next_unmet_evaluations_;
            Json arrivals=Json::array();for(const auto& witness:e.service_witnesses)arrivals.push_back(witness?Json(witness->arrival_tick):Json(nullptr));
            evaluations_.push_back({{"tick",w.ticks()},{"house",static_cast<unsigned>(history.building_id)},
                {"input",house::state(e.input)},{"after_type",static_cast<unsigned>(h.type)},
                {"current_unmet_fields",current},{"next_unmet_fields",next},{"food",house::food_stock(w,history.building_id)},
                {"service_arrival_ticks",arrivals},{"water_arrival_tick",e.water_evaluation_witness?Json(e.water_evaluation_witness->arrival_tick):Json(nullptr)}});
        }
        if(tax||bill||w.ticks()%816==0)finance_.push_back({{"tick",w.ticks()},{"tax",tax},{"upkeep",bill},{"funds",w.treasury()},
            {"population",w.total_population()},{"installed_workers",w.workforce_required()},{"active_workers",w.workforce_used()}});
        if((w.ticks()-begin_)%20400==0) {
            const auto taxes=w.taxes_collected_total()-window_tax_,upkeep=w.maintenance_spent_total()-window_upkeep_,construction=w.construction_spent_total()-window_construction_;
            const auto balance=static_cast<std::int64_t>(taxes)-static_cast<std::int64_t>(upkeep);
            check(w.treasury()-window_funds_==balance-static_cast<std::int64_t>(construction),"Complete operating window hid a running or construction cost");
            windows_.push_back({{"begin_tick",w.ticks()-20400},{"end_tick",w.ticks()},{"ticks",20400},
                {"taxes",taxes},{"upkeep",upkeep},{"construction",construction},{"operating_cashflow",balance},
                {"funds_delta",w.treasury()-window_funds_},{"minimum_funds",window_minimum_},{"funds_begin",window_funds_},{"funds_end",w.treasury()}});
            window_tax_=w.taxes_collected_total();window_upkeep_=w.maintenance_spent_total();window_construction_=w.construction_spent_total();
            window_funds_=w.treasury();window_minimum_=w.treasury();
        }
        if(w.ticks()%816==0){q::authority(w);q::worker_oracle(w);}
    }
    Json report(const sim::World& w)const {
        std::uint64_t misses=0,interval_misses=0;Json homes=Json::array(),stocks=Json::array(),farms=Json::array(),goods_stocks=Json::array(),needs=Json::array();
        for(const auto& stock:w.original_production_state()->food.stocks){misses+=stock.missed_demands;
            const auto found=misses_begin_.find(stock.building_id);const auto difference=stock.missed_demands-(found==misses_begin_.end()?0:found->second);interval_misses+=difference;
            stocks.push_back({{"id",static_cast<unsigned>(stock.building_id)},{"amounts",stock.amounts},{"received",stock.received},{"consumed",stock.consumed},{"missed_interval",difference}});}
        for(const auto& farm:w.original_production_state()->food.farms)farms.push_back({{"id",static_cast<unsigned>(farm.farm_id)},
            {"field",farm.field_id?Json(static_cast<unsigned>(*farm.field_id)):Json(nullptr)},{"crop",static_cast<unsigned>(farm.crop)},{"harvested",farm.harvested}});
        for(const auto& stock:w.original_goods_state()->stocks)goods_stocks.push_back({{"id",static_cast<unsigned>(stock.building_id)},{"amounts",stock.amounts}});
        for(const auto& h:w.original_economy_state()->houses){
            const auto current=current_unmet_.find(h.building_id),next=next_unmet_.find(h.building_id);
            homes.push_back({{"id",static_cast<unsigned>(h.building_id)},{"final",house::state(h.house)},
                {"current_unmet",current==current_unmet_.end()?Json::object():Json(current->second)},
                {"next_unmet",next==next_unmet_.end()?Json::object():Json(next->second)},
                {"taxes",w.building(h.building_id).taxes_paid_total}});}
        for(unsigned row=0;row<orig::house_row_count;++row){Json values=Json::array();
            for(const auto field:oe_longplay_metrics::fields)values.push_back(w.original_rules_model()->house_rules().value(static_cast<orig::ModelHouse>(row),field));
            needs.push_back({{"row",row},{"requirements",values}});}
        bool consecutive=false,prior=false;for(const auto& window:windows_){const auto yes=window.at("operating_cashflow").get<std::int64_t>()>=0;if(prior&&yes)consecutive=true;prior=yes;}
        const auto taxes=w.taxes_collected_total()-start_tax_,upkeep=w.maintenance_spent_total()-start_upkeep_;
        return {{"format","openemperor-sustainable-economy-observation-v1"},{"begin_tick",begin_},{"end_tick",w.ticks()},
            {"first_goal",first_goal_},{"final_goal",oe_paid_progression_strategy::goal(w)},{"upkeep_interval",interval(w)},
            {"summary",{{"windows20400",windows_},{"two_consecutive_nonnegative_windows",consecutive},
                {"taxes_delta",taxes},{"upkeep_delta",upkeep},{"construction_delta",w.construction_spent_total()-start_construction_},
                {"operating_cashflow",static_cast<std::int64_t>(taxes)-static_cast<std::int64_t>(upkeep)},
                {"minimum_funds",minimum_},{"first_negative_tick",first_negative_},{"recovery_ticks",recoveries_},
                {"minimum_inhabited_houses",minimum_inhabited_houses_},{"minimum_population",minimum_population_},
                {"goal_loss_ticks",goal_loss_ticks_},{"upgrades",upgrades_},{"devolutions",devolutions_},
                {"current_unmet_evaluations",current_unmet_evaluations_},{"next_unmet_evaluations",next_unmet_evaluations_},
                {"food_missed_demands_lifetime",misses},{"food_missed_demands",interval_misses},{"food_missed_demands_delta",interval_misses},{"fire_starts",fire_starts_},{"inspector_arrivals",inspector_arrivals_},
                {"actual_extinguishments",extinguishments_}}},
            {"finance",finance_},{"goal_changes",goal_changes_},{"transport",transport_},{"fire_visits",fire_visits_},
            {"evaluations",evaluations_},{"houses",homes},{"food_stocks",stocks},{"food_farms",farms},{"goods_stocks",goods_stocks},
            {"model_needs",needs},{"model_need_fields",oe_longplay_metrics::field_names},{"final",q::observe(w)}};
    }
private:
    void observe_transfers(const sim::World& w,bool count){
        const auto visit=[&](const char* family,const auto& trip){if(!count||trip.arrival_tick!=w.ticks())return;
            auto& total=transport_[family];if(total.is_null())total={{"applied",0},{"refused",0},{"travel_ticks_sum",0}};
            const auto key=trip.visit_applied?"applied":"refused";total[key]=total[key].template get<std::uint64_t>()+1;
            total["travel_ticks_sum"]=total["travel_ticks_sum"].template get<std::uint64_t>()+w.ticks()-trip.dispatched_tick;};
        for(const auto& trip:w.original_production_state()->services.trips)visit("services",trip);
        for(const auto& trip:w.original_production_state()->water.trips)visit("water",trip);
        const auto transfer=[&](const char* family,const auto& trip,std::uint64_t delivered){
            auto& p=transfers_[{family,trip.transfer.courier_id}];const auto fresh=p.sequence!=trip.transfer.sequence;
            if(fresh){p.sequence=trip.transfer.sequence;p.delivered=0;p.started=count?std::optional{w.ticks()}:std::nullopt;}
            auto& total=transport_[family];if(total.is_null())total={{"arrivals",0},{"units",0},{"returns",0},{"cancellations",0},{"travel_ticks_sum",0}};
            if(count&&delivered>p.delivered){total["arrivals"]=total["arrivals"].template get<std::uint64_t>()+1;
                total["units"]=total["units"].template get<std::uint64_t>()+delivered-p.delivered;
                if(p.started)total["travel_ticks_sum"]=total["travel_ticks_sum"].template get<std::uint64_t>()+w.ticks()-*p.started;}
            if(count&&(fresh||trip.phase!=p.phase)){const auto key=trip.phase==sim::OriginalGoodsTransferPhase::Returned?"returns":trip.phase==sim::OriginalGoodsTransferPhase::Cancelled?"cancellations":nullptr;
                if(key)total[key]=total[key].template get<std::uint64_t>()+1;}
            p.delivered=delivered;p.phase=trip.phase;};
        for(const auto& trip:w.original_goods_state()->reservations)transfer("goods",trip,static_cast<std::uint64_t>(trip.delivered));
        for(const auto& trip:w.original_production_state()->food.trips)transfer("food",trip,static_cast<std::uint64_t>(trip.delivered[0]+trip.delivered[1]));
    }
    std::uint64_t begin_,start_tax_,start_upkeep_,start_construction_,window_tax_,window_upkeep_,window_construction_;
    std::int64_t minimum_,window_minimum_,window_funds_,before_funds_=0;
    std::uint64_t before_tick_=0,before_tax_=0,before_upkeep_=0,expected_bill_=0,goal_loss_ticks_=0,upgrades_=0,devolutions_=0,fire_starts_=0,inspector_arrivals_=0,extinguishments_=0;
    std::uint64_t current_unmet_evaluations_=0,next_unmet_evaluations_=0;
    int minimum_inhabited_houses_=0,minimum_population_=0;
    bool previous_goal_=false;
    Json first_goal_=nullptr,first_negative_=nullptr,recoveries_=Json::array(),windows_=Json::array(),finance_=Json::array(),goal_changes_=Json::array(),evaluations_=Json::array(),fire_visits_=Json::array(),transport_=Json::object();
    std::map<sim::BuildingId,bool> burning_;
    std::map<sim::BuildingId,std::uint64_t> misses_begin_;
    std::map<sim::BuildingId,std::map<std::string,std::uint64_t>> current_unmet_,next_unmet_;
    std::map<std::pair<std::string,sim::CourierId>,Transfer> transfers_;
};
}
