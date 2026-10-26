#pragma once

// Bounded, test-only observations. Nothing here is saved simulation authority,
// a new deadline, or a timing benchmark. JSON is produced only at checkpoints.
#include "OriginalLongplayFixture.h"
#include "simulation/original/OriginalScheduler.h"

#include <array>
#include <algorithm>
#include <cstdint>
#include <map>
#include <vector>

namespace oe_longplay_metrics {
namespace f=oe_longplay_fixture;
namespace starter=oe_starter_fixture;
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
using Json=nlohmann::json;
using starter::check;

inline constexpr std::array<orig::HouseField,15> fields{
    orig::HouseField::Water,orig::HouseField::Herbalist,orig::HouseField::Acupuncture,
    orig::HouseField::Music,orig::HouseField::Acrobat,orig::HouseField::Drama,
    orig::HouseField::Ancestor,orig::HouseField::Confucian,orig::HouseField::DaoistOrBuddhist,
    orig::HouseField::FoodQuality,orig::HouseField::Hemp,orig::HouseField::Ceramics,
    orig::HouseField::Tea,orig::HouseField::Silk,orig::HouseField::BronzewareOrLacquerware};
inline constexpr std::array<const char*,15> field_names{
    "Water","Herbalist","Acupuncture","Music","Acrobat","Drama","Ancestor","Confucian",
    "DaoistOrBuddhist","FoodQuality","Hemp","Ceramics","Tea","Silk","BronzewareOrLacquerware"};
inline std::array<int,15> values(const orig::HouseState& h) {
    return {h.water,h.herbalist,h.acupuncture,h.music,h.acrobat,h.drama,h.ancestor,h.confucian,
        std::max(h.daoist,h.buddhist),h.food_quality,h.hemp,h.ceramics,h.tea,h.silk,
        std::max(h.bronzeware,h.lacquerware)};
}
struct House {
    sim::BuildingId id{};
    std::uint64_t evaluations=0,current_unmet=0,next_unmet=0,upgrades=0,devolutions=0,admissions=0;
    std::uint64_t consumption_events=0,food_missed=0,food_consumed=0,occupied_ticks=0;
    std::uint64_t gap_begin=0,longest_gap=0;
    std::array<std::uint64_t,15> current_fields{},next_fields{};
    std::array<std::uint64_t,orig::goods_count> requested{},consumed{},shortfall{},shortfall_events{};
    orig::HouseState before;
    std::array<std::uint64_t,orig::goods_count> previous_consumed{};
    std::uint64_t previous_food_missed=0,previous_food_consumed=0;
    unsigned maximum_type=0;
};
struct Transfer {
    std::uint64_t sequence=0,delivered=0;
};
class Metrics {
public:
    explicit Metrics(const sim::World& world):begin_(world.ticks()),minimum_funds_(world.treasury()),maximum_funds_(world.treasury()) {
        for (const auto& home:world.original_economy_state()->houses) {
            House row;row.id=home.building_id;row.maximum_type=static_cast<unsigned>(home.house.type);houses_.push_back(row);
        }
        observe_transfers(world,false);before(world,false);capacities(world);
    }
    void before(const sim::World& world,bool count_staffing=true) {
        for (auto& row:houses_) {
            row.before=starter::house(world,row.id);
            const auto& production=*world.original_production_state();
            for (const auto& sink:production.house_consumption)if(sink.building_id==row.id)row.previous_consumed=sink.consumed;
            for (const auto& stock:production.food.stocks)if(stock.building_id==row.id) {
                row.previous_food_missed=stock.missed_demands;
                row.previous_food_consumed=stock.consumed[0]+stock.consumed[1];
            }
        }
        for (const auto& body:world.buildings()) {
            const bool burning=world.building_on_fire(body.id);previous_fire_[body.id]=burning;
            if(count_staffing&&world.workforce_required(body.id)>0) {
                if(burning)++burning_staffing_owner_ticks_;
                if(!body.operating_enabled)++paused_owner_ticks_;
                else if(!burning&&world.workers_assigned(body.id)<world.workforce_required(body.id))++understaffed_owner_ticks_;
            }
        }
    }
    void after(const sim::World& world) {
        ++ticks_;minimum_funds_=std::min(minimum_funds_,world.treasury());maximum_funds_=std::max(maximum_funds_,world.treasury());
        minimum_population_=std::min(minimum_population_,world.total_population());maximum_population_=std::max(maximum_population_,world.total_population());
        const auto& rules=world.original_rules_model()->house_rules();
        const auto& p=*world.original_production_state();
        for (const auto& body:world.buildings()) {
            check(!body.health_risk&&!body.health_protection_until_tick&&!body.sick_until_tick,
                "Original longplay acquired unsupported Legacy illness authority");++neutral_health_checks_;
            const bool burning=world.building_on_fire(body.id);
            if (burning) {++burning_owner_ticks_;if(!previous_fire_.at(body.id))++ignitions_;}
        }
        for(auto& row:houses_) {
            const auto& current=starter::house(world,row.id);
            row.maximum_type=std::max(row.maximum_type,static_cast<unsigned>(current.type));
            if(current.occupants>0)++row.occupied_ticks;
            if(!row.before.occupants&&current.occupants>0)++row.admissions;
            const auto history=std::find_if(p.housing.houses.begin(),p.housing.houses.end(),[&](const auto& h){return h.building_id==row.id;});
            check(history!=p.housing.houses.end(),"Longplay lost a paid House history");
            if(history->last_evaluation&&history->last_evaluation->tick==world.ticks()) {
                const auto& input=history->last_evaluation->input;
                ++row.evaluations;
                const bool unmet=missing(rules,orig::house_row(input.type),input,row.current_fields);
                if(unmet) {++row.current_unmet;if(!row.gap_begin)row.gap_begin=world.ticks();}
                else if(row.gap_begin) {row.longest_gap=std::max(row.longest_gap,world.ticks()-row.gap_begin);row.gap_begin=0;}
                if(missing(rules,orig::next_row(input.type),input,row.next_fields))++row.next_unmet;
                if(current.type>input.type)++row.upgrades;
                if(current.type<input.type)++row.devolutions;
                const auto evolved=orig::evolve_step(rules,input);
                check(evolved.type==current.type&&evolved.devolve_counter==current.devolve_counter,
                    "Longplay actual evaluation differs from original pure kernel");
                const auto dispatch=orig::next_scheduler_dispatch(world.ticks()-1);
                if(dispatch&&dispatch->consumption&&input.occupants>0) {
                    ++row.consumption_events;
                    const auto consumed=orig::consume_goods(rules,evolved);
                    const auto sink=std::find_if(p.house_consumption.begin(),p.house_consumption.end(),[&](const auto& r){return r.building_id==row.id;});
                    check(sink!=p.house_consumption.end(),"Longplay lost actual consumption sink");
                    for(std::size_t g=0;g<orig::goods_count;++g) {
                        const auto need=static_cast<std::uint64_t>(std::max(0,rules.value(orig::house_row(evolved.type),orig::goods_field(static_cast<orig::Goods>(g)))));
                        const auto amount=static_cast<std::uint64_t>(consumed.consumed[g]);
                        check(sink->consumed[g]>=row.previous_consumed[g]&&sink->consumed[g]-row.previous_consumed[g]==amount,
                            "Longplay physical goods debit differs from actual scheduled kernel");
                        row.requested[g]+=need;row.consumed[g]+=amount;row.shortfall[g]+=need-amount;
                        if(amount<need)++row.shortfall_events[g];
                    }
                }
            }
            for(const auto& stock:p.food.stocks)if(stock.building_id==row.id) {
                check(stock.missed_demands>=row.previous_food_missed,"Food missed-demand counter regressed");
                row.food_missed+=stock.missed_demands-row.previous_food_missed;
                const auto consumed=stock.consumed[0]+stock.consumed[1];check(consumed>=row.previous_food_consumed,"Food consumption counter regressed");
                row.food_consumed+=consumed-row.previous_food_consumed;
            }
        }
        for(const auto& trip:p.services.trips)if(trip.arrival_tick==world.ticks()&&trip.visit_applied)++service_arrivals_;
        for(const auto& trip:p.water.trips)if(trip.arrival_tick==world.ticks()&&trip.visit_applied)++water_arrivals_;
        observe_transfers(world,true);capacities(world);
    }
    void command() {++control_commands_;}
    Json report(const sim::World& world) const {
        Json homes=Json::array();std::uint64_t evaluations=0,up=0,down=0,unmet=0,next=0,food_missed=0;
        for(const auto& row:houses_) {
            const auto gap=row.gap_begin?std::max(row.longest_gap,world.ticks()-row.gap_begin):row.longest_gap;
            homes.push_back({{"id",static_cast<unsigned>(row.id)},{"evaluations",row.evaluations},
                {"evaluations_with_unmet_current_needs",row.current_unmet},{"evaluations_with_unmet_next_row_needs",row.next_unmet},
                {"unmet_current_by_requirement",row.current_fields},{"unmet_next_by_requirement",row.next_fields},
                {"upgrades",row.upgrades},{"devolutions",row.devolutions},{"admissions",row.admissions},
                {"maximum_type",row.maximum_type},{"occupied_ticks",row.occupied_ticks},
                {"longest_sampled_current_need_gap_ticks",gap},{"goods_consumption_events",row.consumption_events},
                {"goods_requested_units",row.requested},{"goods_consumed_units",row.consumed},
                {"goods_shortfall_units",row.shortfall},{"goods_shortfall_events",row.shortfall_events},
                {"food_consumed",row.food_consumed},{"actual_food_missed_demands",row.food_missed}});
            evaluations+=row.evaluations;up+=row.upgrades;down+=row.devolutions;unmet+=row.current_unmet;next+=row.next_unmet;food_missed+=row.food_missed;
        }
        Json materials=Json::array();
        for(unsigned m=0;m<sim::original_aux_material_count;++m) {
            const auto b=world.original_auxiliary_balance(static_cast<sim::OriginalAuxMaterial>(m));
            materials.push_back({{"material",m},{"produced",b.produced},{"stock",b.stock},{"cargo",b.cargo},{"consumed",b.consumed},{"conserved",b.conserved}});
        }
        return {{"format","openemperor-original-longplay-metrics-v1"},{"status","PASS"},{"budget_format",1},
            {"begin_tick",begin_},{"end_tick",world.ticks()},{"observed_ticks",ticks_},
            {"requirement_order",field_names},{"houses",homes},{"evaluations",evaluations},{"upgrades",up},{"devolutions",down},
            {"evaluations_with_unmet_current_needs",unmet},{"evaluations_with_unmet_next_row_needs",next},
            {"actual_food_missed_demands",food_missed},{"minimum_funds",minimum_funds_},{"maximum_funds",maximum_funds_},
            {"minimum_population",minimum_population_},{"maximum_population",maximum_population_},
            {"natural_fire_ignitions",ignitions_},{"burning_owner_ticks",burning_owner_ticks_},
            {"understaffed_operating_owner_ticks",understaffed_owner_ticks_},{"paused_owner_ticks",paused_owner_ticks_},
            {"burning_pre_tick_staffing_owner_ticks",burning_staffing_owner_ticks_},
            {"staffing_observation","After ordinary controls, before the actual tick captures workforce and before immigration/fire changes."},
            {"ordinary_control_commands",control_commands_},{"service_applied_arrivals",service_arrivals_},{"water_applied_arrivals",water_arrivals_},
            {"goods_delivered_units",goods_delivered_},{"food_delivered_units",food_delivered_},{"aux_delivered_units",aux_delivered_},
            {"original_health_neutral_checks",neutral_health_checks_},{"original_illness","unsupported; neutral fields verified"},
            {"materials",materials},{"final",starter::observe(world)},
            {"limitations",{"Finite current-needs evaluation observations are not universal starvation freedom.",
                "Next-row requirements are upgrade blockers, not current-demand misses.","This observer is not a performance benchmark.",
                "Native/human and complete original visual parity remain NOT_RUN."}}};
    }
    Json resources(const sim::World& world) const {
        return {{"format","openemperor-original-longplay-resources-v1"},{"current",f::budgets(world)},
            {"highwater_route_slots",maximum_route_slots_},{"highwater_route_cell_capacity",maximum_route_cells_},
            {"highwater_path_cell_capacity",maximum_path_cells_},{"highwater_dynamic_slot_capacity",maximum_dynamic_slots_},
            {"limitations","Visible vector capacities only; not a complete heap or private World building-vector inventory."}};
    }
private:
    static bool missing(const orig::HouseEvolutionRules& rules,orig::ModelHouse model,const orig::HouseState& input,
                        std::array<std::uint64_t,15>& counts) {
        const auto actual=values(input);bool any=false;
        for(std::size_t i=0;i<fields.size();++i) {
            const auto need=rules.value(model,fields[i]);const bool met=i<9?(need==0||actual[i]!=0):actual[i]>=need;
            if(!met){++counts[i];any=true;}
        }
        check(any==!orig::needs_met(rules,model,input),"Observed actual requirement predicates differ from original kernel");return any;
    }
    static void transfer(std::map<sim::CourierId,Transfer>& seen,sim::CourierId id,std::uint64_t sequence,
                         std::uint64_t delivered,std::uint64_t& total,bool count) {
        auto& old=seen[id];if(count) {
            check(sequence>=old.sequence,"Actual transfer sequence regressed");
            if(sequence==old.sequence) {check(delivered>=old.delivered,"Actual same-trip delivery regressed");total+=delivered-old.delivered;}
            else total+=delivered;
        }
        old={sequence,delivered};
    }
    void observe_transfers(const sim::World& world,bool count) {
        for(const auto& trip:world.original_goods_state()->reservations)
            transfer(goods_transfers_,trip.transfer.courier_id,trip.transfer.sequence,static_cast<std::uint64_t>(trip.delivered),goods_delivered_,count);
        for(const auto& trip:world.original_production_state()->food.trips)
            transfer(food_transfers_,trip.transfer.courier_id,trip.transfer.sequence,static_cast<std::uint64_t>(trip.delivered[0]+trip.delivered[1]),food_delivered_,count);
        for(const auto& trip:world.original_production_state()->industry.aux_trips)
            transfer(aux_transfers_,trip.transfer.courier_id,trip.transfer.sequence,static_cast<std::uint64_t>(trip.delivered),aux_delivered_,count);
    }
    void capacities(const sim::World& world) {
        std::size_t paths=0,routes=0,slots=0;
        for(const auto& c:world.couriers()) {
            paths+=c.path.capacity();slots+=c.dynamic_target_routes.capacity();
            if(c.cached_route)routes+=c.cached_route->capacity();
            for(const auto& r:c.dynamic_target_routes)if(r.second)routes+=r.second->capacity();
            for(const auto& r:c.target_routes)if(r)routes+=r->capacity();
        }
        maximum_path_cells_=std::max(maximum_path_cells_,paths);maximum_route_cells_=std::max(maximum_route_cells_,routes);
        maximum_dynamic_slots_=std::max(maximum_dynamic_slots_,slots);maximum_route_slots_=std::max(maximum_route_slots_,world.route_cache_entries());
    }
    std::vector<House> houses_;
    std::map<sim::BuildingId,bool> previous_fire_;
    std::map<sim::CourierId,Transfer> goods_transfers_,food_transfers_,aux_transfers_;
    std::uint64_t begin_=0,ticks_=0,ignitions_=0,burning_owner_ticks_=0,neutral_health_checks_=0;
    std::uint64_t understaffed_owner_ticks_=0,paused_owner_ticks_=0,burning_staffing_owner_ticks_=0,control_commands_=0,service_arrivals_=0,water_arrivals_=0;
    std::uint64_t goods_delivered_=0,food_delivered_=0,aux_delivered_=0;
    std::int64_t minimum_funds_=0,maximum_funds_=0;
    int minimum_population_=INT32_MAX,maximum_population_=0;
    std::size_t maximum_path_cells_=0,maximum_route_cells_=0,maximum_dynamic_slots_=0,maximum_route_slots_=0;
};
} // namespace oe_longplay_metrics
