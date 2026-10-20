#include "../tests/OriginalProductionFixture.h"
#include "simulation/original/GoodsRules.h"
#include "simulation/original/OriginalScheduler.h"
#include "simulation/original/ServiceRules.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <nlohmann/json.hpp>
#include <utility>

namespace {
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace save=openemperor::persistence;
namespace original=openemperor::original;
using json=nlohmann::json;
using f::check;

bool below(const f::fs::path& path,const f::fs::path& root) {
    auto a=path.begin();
    for (auto b=root.begin();b!=root.end();++a,++b)
        if (a==path.end() || *a!=*b) return false;
    return true;
}
void validate_output(const f::fs::path& output,const f::fs::path& data) {
    const auto repo=f::fs::canonical(f::fs::path(__FILE__).parent_path().parent_path());
    const auto root=repo/".local/original-economy/oe27/production-fixtures";
    check(output!=root && below(output,root),"Fixture output must be a new private OE27 subdirectory");
    check(!below(output,data) && !below(data,output),"Fixture output overlaps source data");
    f::fs::path current=output.root_path();
    for (const auto& part:output.relative_path()) {
        check(part!=".." && part!=".","Fixture output must have a canonical path");current/=part;
        std::error_code error;
        check(!f::fs::is_symlink(f::fs::symlink_status(current,error)),"Fixture output traverses a symlink");
    }
    check(!f::fs::exists(output),"Fixture output already exists");
    save::validate_save_target(output/"fixture.json",data);
}
void ticks(sim::World& world,int count) {for (int i=0;i<count;++i) world.tick();}
json inspect_food(const sim::World& world) {
    const auto& food=world.original_production_state()->food;
    json farms=json::array(),stocks=json::array(),trips=json::array();
    sim::OriginalFoodCounters produced{},held{},carried{},consumed{};
    for(const auto& farm:food.farms) {
        produced[static_cast<unsigned>(farm.crop)]+=farm.harvested;
        farms.push_back({{"farm_id",static_cast<unsigned>(farm.farm_id)},
            {"field_id",farm.field_id ? json(static_cast<unsigned>(*farm.field_id)):json(nullptr)},
            {"crop",static_cast<unsigned>(farm.crop)},{"harvested",farm.harvested}});
    }
    for(const auto& stock:food.stocks) {
        json witness=nullptr;unsigned quality=0;
        for(const auto& house:world.original_economy_state()->houses)
            if(house.building_id==stock.building_id)quality=house.house.food_quality;
        if(stock.quality_witness) {
            const auto& w=*stock.quality_witness;
            witness={{"courier_id",static_cast<unsigned>(w.transfer.courier_id)},{"sequence",w.transfer.sequence},
                {"tick",w.tick},{"accepted",w.accepted},{"consumed_before",w.consumed_before},{"previous_quality",w.previous_quality}};
        }
        for(std::size_t g=0;g<2;++g){held[g]+=static_cast<std::uint64_t>(stock.amounts[g]);consumed[g]+=stock.consumed[g];}
        stocks.push_back({{"id",static_cast<unsigned>(stock.building_id)},
            {"kind",static_cast<unsigned>(world.building(stock.building_id).kind)},
            {"amounts",stock.amounts},{"received",stock.received},{"departed",stock.departed},
            {"returned",stock.returned},{"consumed",stock.consumed},{"missed_demands",stock.missed_demands},
            {"quality",quality},{"quality_witness",witness}});
    }
    for(const auto& trip:food.trips) {
        sim::OriginalFoodAmounts cargo{};
        for(std::size_t g=0;g<2;++g) {
            cargo[g]=trip.loaded[g]-trip.delivered[g]-trip.returned[g];
            check(cargo[g]>=0,"Food trip has negative remaining Cargo");carried[g]+=static_cast<std::uint64_t>(cargo[g]);
        }
        trips.push_back({{"courier_id",static_cast<unsigned>(trip.transfer.courier_id)},{"sequence",trip.transfer.sequence},
            {"source",static_cast<unsigned>(trip.source)},{"target",static_cast<unsigned>(trip.target)},
            {"phase",static_cast<unsigned>(trip.phase)},{"loaded",trip.loaded},{"delivered",trip.delivered},
            {"returned",trip.returned},{"cargo",cargo},{"batch_quality",sim::original_food_batch_quality(cargo).value}});
    }
    for(std::size_t g=0;g<2;++g)check(produced[g]==held[g]+carried[g]+consumed[g],"Actual Food Crop mass is not conserved");
    return {{"farms",farms},{"stocks",stocks},{"trips",trips},{"produced",produced},
        {"stock",held},{"cargo",carried},{"consumed",consumed},{"conserved",true}};
}
std::uint64_t countdowns(std::uint64_t tick) {return tick<36 ? 0:1+(tick-36)/51;}
json inspect_services(const sim::World& world) {
    const auto& state=world.original_production_state()->services;
    json providers=json::array(),trips=json::array(),witnesses=json::array();
    for (const auto& p:state.providers) {
        original::ModelBuilding model_type;
        switch (p.type) {
        case sim::OriginalServiceProviderType::Herbalist207: model_type=original::ModelBuilding::HerbalistsStall;break;
        case sim::OriginalServiceProviderType::Acupuncture208: model_type=original::ModelBuilding::AcupuncturistsClinic;break;
        case sim::OriginalServiceProviderType::Ancestor214: model_type=original::ModelBuilding::AncestralShrine;break;
        case sim::OriginalServiceProviderType::Daoist215: model_type=original::ModelBuilding::DaoistShrine;break;
        case sim::OriginalServiceProviderType::Buddhist217: model_type=original::ModelBuilding::BuddhistShrine;break;
        case sim::OriginalServiceProviderType::Confucian219: model_type=original::ModelBuilding::ConfucianAcademy;break;
        case sim::OriginalServiceProviderType::Music211: model_type=original::ModelBuilding::MusicSchool;break;
        case sim::OriginalServiceProviderType::Acrobat212: model_type=original::ModelBuilding::AcrobatSchool;break;
        case sim::OriginalServiceProviderType::Drama213: model_type=original::ModelBuilding::DramaSchool;break;
        default: throw std::runtime_error("Unknown paid original service provider");
        }
        providers.push_back({{"id",static_cast<unsigned>(p.building_id)},{"type",static_cast<unsigned>(p.type)},
            {"model_type",static_cast<unsigned>(model_type)},
            {"cost",world.original_rules_model()->building_value(model_type,original::BuildingField::Cost)},
            {"workers_required",world.workforce_required(p.building_id)}});
    }
    for (const auto& t:state.trips)
        trips.push_back({{"courier_id",static_cast<unsigned>(t.transfer.courier_id)},{"sequence",t.transfer.sequence},
            {"target",static_cast<unsigned>(t.target)},{"dispatched_tick",t.dispatched_tick},
            {"arrival_tick",t.arrival_tick ? json(*t.arrival_tick):json(nullptr)},{"visit_applied",t.visit_applied}});
    for (const auto& w:state.witnesses)
        witnesses.push_back({{"house_id",static_cast<unsigned>(w.building_id)},{"role",static_cast<unsigned>(w.role)},
            {"provider_id",static_cast<unsigned>(w.provider_id)},{"courier_id",static_cast<unsigned>(w.transfer.courier_id)},
            {"sequence",w.transfer.sequence},{"arrival_tick",w.arrival_tick}});
    return {{"providers",providers},{"trips",trips},{"witnesses",witnesses},{"countdowns",countdowns(world.ticks())}};
}
json inspect_availability(const sim::World& world) {
    json rows=json::array();
    for(const auto& h:world.original_economy_state()->houses)for(unsigned r=0;r<9;++r) {
        const auto d=world.original_service_availability(h.building_id,static_cast<original::ServiceRole>(r));
        rows.push_back({{"house_id",static_cast<unsigned>(h.building_id)},{"role",r},{"eligible_target",d.eligible_target},
            {"delivered_timer",d.delivered_timer},{"provider_count",d.provider_count},{"available_source_count",d.available_source_count},
            {"reachable_source_count",d.reachable_source_count},{"travelling_to_target_count",d.travelling_to_target_count},
            {"last_arrival_tick",d.last_arrival_tick ? json(*d.last_arrival_tick):json(nullptr)}});
    }
    return rows;
}
json water_witness(const std::optional<sim::OriginalWaterWitness>& value) {
    if (!value) return nullptr;
    const auto& w=*value;
    return {{"house_id",static_cast<unsigned>(w.building_id)},{"provider_id",static_cast<unsigned>(w.provider_id)},
        {"courier_id",static_cast<unsigned>(w.transfer.courier_id)},{"sequence",w.transfer.sequence},{"arrival_tick",w.arrival_tick}};
}
json service_witness(const std::optional<sim::OriginalServiceWitness>& value) {
    if (!value) return nullptr;
    const auto& w=*value;
    return {{"house_id",static_cast<unsigned>(w.building_id)},{"role",static_cast<unsigned>(w.role)},
        {"provider_id",static_cast<unsigned>(w.provider_id)},{"courier_id",static_cast<unsigned>(w.transfer.courier_id)},
        {"sequence",w.transfer.sequence},{"arrival_tick",w.arrival_tick}};
}
json food_witness(const std::optional<sim::OriginalFoodQualityWitness>& value) {
    if (!value) return nullptr;
    const auto& w=*value;
    return {{"courier_id",static_cast<unsigned>(w.transfer.courier_id)},{"sequence",w.transfer.sequence},
        {"tick",w.tick},{"accepted",w.accepted},{"consumed_before",w.consumed_before},{"previous_quality",w.previous_quality}};
}
json inspect_water(const sim::World& world) {
    const auto& state=world.original_production_state()->water;
    json providers=json::array(),trips=json::array(),houses=json::array();
    for (const auto& p:state.providers) providers.push_back({{"id",static_cast<unsigned>(p.building_id)},
        {"cost",world.original_rules_model()->building_value(original::ModelBuilding::Well,original::BuildingField::Cost)},
        {"workers_required",world.workforce_required(p.building_id)}});
    for (const auto& t:state.trips) trips.push_back({{"courier_id",static_cast<unsigned>(t.transfer.courier_id)},
        {"sequence",t.transfer.sequence},{"target",static_cast<unsigned>(t.target)},{"dispatched_tick",t.dispatched_tick},
        {"arrival_tick",t.arrival_tick ? json(*t.arrival_tick):json(nullptr)},{"visit_applied",t.visit_applied}});
    for (const auto& h:state.houses) houses.push_back({{"id",static_cast<unsigned>(h.building_id)},
        {"normal_timer",h.normal_timer},{"special_timer",h.special_timer},
        {"current_witness",water_witness(h.current_witness)},{"evaluation_witness",water_witness(h.evaluation_witness)}});
    return {{"providers",providers},{"trips",trips},{"houses",houses},
        {"countdowns",countdowns(world.ticks())},{"evaluation_tick",sim::original_water_evaluation_tick(world.ticks())}};
}
json evaluation(const std::optional<sim::OriginalHousingEvaluation>& value) {
    if (!value) return nullptr;
    const auto& e=*value;const auto& h=e.input;
    json services=json::array();for (const auto& w:e.service_witnesses) services.push_back(service_witness(w));
    return {{"tick",e.tick},{"input",{{"type",static_cast<unsigned>(h.type)},{"desirability",h.desirability},
        {"occupants",h.occupants},{"devolve_counter",h.devolve_counter},{"water",h.water},
        {"service_timers",json::array({h.herbalist,h.acupuncture,h.music,h.acrobat,h.drama,h.ancestor,h.confucian,h.daoist,h.buddhist})},
        {"food_quality",h.food_quality},{"goods",json::array({h.hemp,h.ceramics,h.tea,h.silk,h.bronzeware,h.lacquerware})}}},
        {"service_witnesses",services},{"water_evaluation_witness",water_witness(e.water_evaluation_witness)},
        {"food_quality_witness",food_witness(e.food_quality_witness)},
        {"food_received",e.food_received},{"food_consumed",e.food_consumed},{"goods_consumed_before",e.goods_consumed_before}};
}
json evaluation(const std::optional<sim::OriginalHousingTransition>& value) {
    return value ? evaluation(std::optional<sim::OriginalHousingEvaluation>{value->evaluation}):json(nullptr);
}
json inspect_housing(const sim::World& world) {
    const auto& state=world.original_production_state()->housing;
    json houses=json::array(),origins=json::array(),model_rows=json::array();
    for (const auto& h:state.houses) {
        json rows=json::array();for (const auto& r:h.consumption_rows) rows.push_back({{"events",r.events},{"consumed",r.consumed}});
        json transitions=json::array();
        for (const auto& transition:h.first_transitions)
            transitions.push_back(transition ? json{{"kind",static_cast<unsigned>(transition->kind)},
                {"admitted",transition->admitted},{"evaluation",evaluation(transition)}}:json(nullptr));
        houses.push_back({{"id",static_cast<unsigned>(h.building_id)},
            {"first_admission_tick",h.first_admission_tick ? json(*h.first_admission_tick):json(nullptr)},
            {"consumption_rows",rows},{"last_evaluation",evaluation(h.last_evaluation)},
            {"first_transitions",transitions},
            {"first_hut_evaluation",evaluation(h.first_transitions[1])},{"first_plain_evaluation",evaluation(h.first_transitions[2])}});
    }
    for (const auto& o:state.neutral_source_origins) origins.push_back({{"id",static_cast<unsigned>(o.building_id)},
        {"cell",json::array({o.origin.x,o.origin.y})}});
    for (unsigned r=0;r<original::house_row_count;++r) {
        json row=json::array();for (unsigned c=0;c<original::house_field_count;++c)
            row.push_back(world.original_rules_model()->house_rules().value(static_cast<original::ModelHouse>(r),static_cast<original::HouseField>(c)));
        model_rows.push_back(row);
    }
    return {{"houses",houses},{"neutral_source_origins",origins},{"model_rows",model_rows},
        {"evaluation_tick",sim::original_housing_evaluation_tick(world.ticks())},
        {"consumption_events",sim::original_housing_consumption_events(world.ticks())}};
}
json inspect_industry(const sim::World& world) {
    const auto& state=world.original_production_state()->industry;
    json sources=json::array(),ore=json::array(),stocks=json::array(),trips=json::array(),clay=json::array(),balances=json::array();
    for(const auto& row:state.sources)sources.push_back({{"id",static_cast<unsigned>(row.building_id)},
        {"plant_id",row.plant_id?json(static_cast<unsigned>(*row.plant_id)):json(nullptr)},
        {"completed",row.completed},{"progress",row.progress},{"goods_inputs_used",row.goods_inputs_used},
        {"aux_inputs_used",row.aux_inputs_used},{"clay_received",row.clay_received},{"clay_used",row.clay_used}});
    for(const auto& row:state.managed_ore)ore.push_back({{"id",static_cast<unsigned>(row.building_id)},
        {"stock",row.stock},{"progress",row.progress},{"extracted",row.extracted},{"used",row.used}});
    for(const auto& row:state.auxiliary)stocks.push_back({{"id",static_cast<unsigned>(row.building_id)},
        {"amounts",row.amounts},{"received",row.received},{"departed",row.departed},{"returned",row.returned},{"consumed",row.consumed}});
    for(const auto& row:state.aux_trips)trips.push_back({{"courier_id",static_cast<unsigned>(row.transfer.courier_id)},
        {"sequence",row.transfer.sequence},{"source",static_cast<unsigned>(row.source)},{"target",static_cast<unsigned>(row.target)},
        {"material",static_cast<unsigned>(row.material)},{"phase",static_cast<unsigned>(row.phase)},
        {"loaded",row.loaded},{"delivered",row.delivered},{"returned",row.returned},
        {"dispatched_tick",row.dispatched_tick},{"arrival_tick",row.arrival_tick?json(*row.arrival_tick):json(nullptr)}});
    for(const auto& row:state.clay_trips)clay.push_back({{"courier_id",static_cast<unsigned>(row.transfer.courier_id)},
        {"sequence",row.transfer.sequence},{"source",static_cast<unsigned>(row.source)},{"target",static_cast<unsigned>(row.target)},
        {"loaded",row.loaded},{"delivered",row.delivered},{"dispatched_tick",row.dispatched_tick},
        {"arrival_tick",row.arrival_tick?json(*row.arrival_tick):json(nullptr)}});
    for(unsigned a=0;a<5;++a){const auto b=world.original_auxiliary_balance(static_cast<sim::OriginalAuxMaterial>(a));
        check(b.conserved,"Auxiliary production/cargo/recipe mass differs");
        balances.push_back({{"material",a},{"produced",b.produced},{"stock",b.stock},{"cargo",b.cargo},{"consumed",b.consumed},
            {"source_reserved",b.source_reserved},{"incoming_reserved",b.incoming_reserved},{"conserved",b.conserved}});}
    return {{"sources",sources},{"managed_ore",ore},{"stocks",stocks},{"trips",trips},{"clay_trips",clay},{"balances",balances}};
}
json inspect_finance(const sim::World& world) {
    json houses=json::array(),rates=json::array();
    for (unsigned i=0;i<original::house_row_count;++i)
        rates.push_back(world.original_rules_model()->house_rules().value(
            static_cast<original::ModelHouse>(i),original::HouseField::TaxRate));
    for (const auto& h:world.original_production_state()->finance.houses) {
        json rows=json::array(),last=nullptr;
        for (const auto& row:h.rows) rows.push_back({{"events",row.events},{"resident_units",row.resident_units}});
        if (h.last_assessment) last={{"tick",h.last_assessment->tick},
            {"type",static_cast<unsigned>(h.last_assessment->type)},{"occupants",h.last_assessment->occupants}};
        const auto status=world.original_house_tax_status(h.building_id);
        houses.push_back({{"building_id",static_cast<unsigned>(h.building_id)},
            {"rows",rows},{"last_assessment",last},{"paid_total",world.building(h.building_id).taxes_paid_total},
            {"current_rate",status.rate},{"eligible",status.eligible},
            {"amount_if_assessed_now",status.amount_if_assessed_now}});
    }
    const auto goal=world.original_goal_status();
    return {{"houses",houses},{"tax_rate_by_row",rates},{"tax_interval",816},{"upkeep_interval",400},
        {"upkeep_per_worker",2},{"installed_upkeep",world.current_maintenance_rate()},
        {"goal",{{"inhabited_houses",goal.inhabited_houses},{"common_houses",goal.common_houses},
            {"population",goal.population},{"positive_tax",goal.positive_tax},
            {"nonnegative_funds",goal.nonnegative_funds},{"reached",goal.reached}}}};
}
json inspect(const sim::World& world) {
    check(world.original_production_enabled() && world.original_production_state() &&
          world.production_balance_valid() && world.population_valid() && world.city_economy_valid() &&
          world.navigation_valid(),"Paid production invariant failed");
    json balances=json::array(),houses=json::array(),buildings=json::array(),couriers=json::array(),stocks=json::array();
    const auto all=world.original_goods_balances();
    for (std::size_t i=0;i<all.size();++i) {
        const auto& b=all[i];
        check(b==world.original_goods_balance(static_cast<original::Goods>(i)) && b.conserved &&
              b.supported && !b.totals.imported && !b.totals.demolished,
              "Unexpected typed goods capability or mass");
        check(b.stock+b.cargo+b.totals.consumed==b.totals.produced,"Physical stock, actual cargo and consumption disagree with produced mass");
        balances.push_back({{"good",i},{"supported",b.supported},{"conserved",b.conserved},
            {"produced",b.totals.produced},{"imported",b.totals.imported},{"consumed",b.totals.consumed},
            {"demolished",b.totals.demolished},{"stock",b.stock},{"cargo",b.cargo},
            {"source_reserved",b.source_reserved},{"incoming_reserved",b.incoming_reserved}});
    }
    const auto& history=world.original_production_state()->house_consumption;
    const auto& homes=world.original_economy_state()->houses;
    check(history.size()==homes.size(),"Consumption history lost a House");
    std::array<std::uint64_t,original::goods_count> consumed{};
    for (std::size_t index=0;index<homes.size();++index) {
        const auto& h=homes[index];check(history[index].building_id==h.building_id,"Consumption House identity mismatch");
        for (std::size_t g=0;g<consumed.size();++g) consumed[g]+=history[index].consumed[g];
        check(h.house.hemp>=0 && h.house.hemp<=sim::Rules::household_capacity &&
              h.house.ceramics>=0 && h.house.ceramics<=sim::Rules::household_capacity,
              "House exceeded its actual delivery buffer");
        houses.push_back({{"id",static_cast<unsigned>(h.building_id)},{"type",static_cast<unsigned>(h.house.type)},
            {"occupants",h.house.occupants},{"devolve_counter",h.house.devolve_counter},{"water",h.house.water},
            {"hemp",h.house.hemp},{"ceramics",h.house.ceramics},{"tea",h.house.tea},{"silk",h.house.silk},{"bronzeware",h.house.bronzeware},{"lacquerware",h.house.lacquerware},
            {"consumed",history[index].consumed},{"food_quality",h.house.food_quality},
            {"service_timers",json::array({h.house.herbalist,h.house.acupuncture,h.house.music,h.house.acrobat,
                h.house.drama,h.house.ancestor,h.house.confucian,h.house.daoist,h.house.buddhist})},
            {"status",h.original_status},{"access_enabled",h.access_enabled},
            {"desirability",world.original_influence_provenance(h.building_id).value}});
    }
    for (std::size_t g=0;g<consumed.size();++g)
        {std::uint64_t recipe=0;for(const auto& row:world.original_production_state()->industry.sources)recipe+=row.goods_inputs_used[g];
        check(consumed[g]+recipe==all[g].totals.consumed,"House-only histories plus separate actual recipe sinks disagree with ledger");}
    int installed_workers=0;
    for (const auto& b:world.buildings()) {
        installed_workers+=world.workforce_required(b.id);
        check(b.population==0 && b.pottery_stock==0 && b.food_stock==0 && b.food_produced==0 &&
              b.food_consumed_total==0 && b.reserved_food_incoming==0 && b.service_until_tick==0 &&
              b.health_protection_until_tick==0 && b.health_risk==0 && b.sick_until_tick==0,"New authority was mirrored into legacy stock or population");
        const auto footprint=sim::building_footprint(world.profile(),world.rule_version(),b.kind);
        buildings.push_back({{"id",static_cast<unsigned>(b.id)},{"kind",static_cast<unsigned>(b.kind)},
            {"footprint",json::array({footprint.width,footprint.height})},
            {"operating",b.operating_enabled},{"on_fire",world.building_on_fire(b.id)},
            {"placed_tick",b.placed_tick},{"cell",json::array({b.cell.x,b.cell.y})},
            {"workers",world.workers_assigned(b.id)},{"workers_required",world.workforce_required(b.id)},
            {"taxes_paid_total",b.taxes_paid_total},{"input_clay",b.input_clay},{"output",b.output},
            {"progress",b.progress},{"recipes_completed",b.recipes_completed},{"clay_extracted",b.clay_extracted}});
    }
    for (const auto& c:world.couriers()) {
        json cargo_good=nullptr;int typed_cargo=0;
        for (const auto& row:world.original_goods_state()->cargo)
            if (row.transfer.courier_id==c.id) {cargo_good=static_cast<unsigned>(row.good);typed_cargo=row.amount;}
        const int food_cargo=world.original_food_cargo(c.id);
        const int aux_cargo=world.original_auxiliary_cargo(c.id);
        check(world.original_courier_cargo(c.id)==typed_cargo+food_cargo+aux_cargo,"Public cargo query disagrees with separate actual ledgers");
        couriers.push_back({{"id",static_cast<unsigned>(c.id)},{"role",static_cast<unsigned>(c.role)},
            {"owner",static_cast<unsigned>(c.owner)},{"target",static_cast<unsigned>(c.target)},
            {"phase",static_cast<unsigned>(c.phase)},{"cargo",c.cargo},{"reserved",c.reserved},
            {"typed_cargo",typed_cargo},{"typed_cargo_good",cargo_good},{"food_cargo",food_cargo},{"aux_cargo",aux_cargo},
            {"path_vertex",c.path_vertex},{"edge_progress",c.edge_progress},
            {"last_dispatched_target",c.last_dispatched_target ? json(static_cast<unsigned>(*c.last_dispatched_target)):json(nullptr)}});
    }
    for (const auto& row:world.original_goods_state()->stocks)
        stocks.push_back({{"id",static_cast<unsigned>(row.building_id)},
            {"kind",static_cast<unsigned>(world.building(row.building_id).kind)},{"amounts",row.amounts}});
    const auto snapshot=world.snapshot();
    json farms=json::array();std::uint64_t harvested=0;
    for (const auto& farm:snapshot.original_production->hemp_farms) {
        harvested+=farm.harvested;
        farms.push_back({{"farm_id",static_cast<unsigned>(farm.farm_id)},
            {"field_id",farm.field_id ? json(static_cast<unsigned>(*farm.field_id)):json(nullptr)},
            {"harvested",farm.harvested}});
    }
    check(all[0].totals.produced==harvested,"Hemp mass lacks actual harvest history");
    json owners=json::array(),roads=json::array();
    for (int y=0;y<world.height();++y) for (int x=0;x<world.width();++x) {
        const sim::Cell cell{x,y};
        if (const auto owner=world.building_owner_at(cell)) owners.push_back(json::array({x,y,static_cast<unsigned>(*owner),static_cast<unsigned>(world.object_at(cell))}));
        if (world.road_cell(cell)) roads.push_back(json::array({x,y}));
    }
    return {{"geometry",{{"width",world.width()},{"height",world.height()},{"owners",owners},{"roads",roads}}},
        {"tick",world.ticks()},{"funds",world.treasury()},{"construction_spent",world.construction_spent_total()},
        {"taxes_collected",world.taxes_collected_total()},{"maintenance_spent",world.maintenance_spent_total()},
        {"difficulty",static_cast<unsigned>(world.original_rules_model()->difficulty())},{"finance",inspect_finance(world)},
        {"command_sequence",world.command_sequence()},{"road_revision",snapshot.road_revision},
        {"road_count",snapshot.roads.size()},{"house_count",houses.size()},{"building_count",buildings.size()},
        {"courier_count",couriers.size()},{"population",world.total_population()},
        {"capacity",world.total_population_capacity()},{"workforce_supply",world.workforce_supply()},
        {"workforce_used",world.workforce_used()},{"workforce_required",world.workforce_required()},{"installed_workforce_required",installed_workers},{"residents_admitted",snapshot.original_production->residents_admitted},
        {"clay_extracted",world.clay_extracted_total()},{"pottery_completed",world.pottery_completed_total()},
        {"pottery_handed_off",snapshot.original_production->pottery_handed_off},
        {"hemp_harvested",harvested},{"hemp_farms",farms},
        {"rule_version",world.rule_version()},{"development_revision",snapshot.original_production->development_revision},
        {"model_sha256",world.original_rules_model()->provenance().sha256},
        {"difficulty",static_cast<unsigned>(world.original_rules_model()->difficulty())},
        {"shelter_needs",json::array({world.original_rules_model()->house_rules().value(original::ModelHouse::Shelter,original::HouseField::Hemp),
            world.original_rules_model()->house_rules().value(original::ModelHouse::Shelter,original::HouseField::Ceramics)})},
        {"food",inspect_food(world)},{"services",inspect_services(world)},{"service_availability",inspect_availability(world)},
        {"water",inspect_water(world)},{"housing",inspect_housing(world)},{"industry",inspect_industry(world)},
        {"balances",std::move(balances)},{"houses",std::move(houses)},{"stocks",std::move(stocks)},
        {"buildings",std::move(buildings)},{"couriers",std::move(couriers)}};
}
void save_world(const f::fs::path& data,const f::fs::path& target,const sim::World& world) {
    const auto policy=world.map_permissions();
    save::write_save(target,save::make_document(data,f::map_relative,policy->building_mask(),world),data,policy->building_mask());
}
void continuation(const f::fs::path& data,const f::fs::path& output,const std::string& prefix,sim::World& world) {
    const auto initial=world.ticks();ticks(world,10000);check(world.ticks()==initial+10000,"First replay period drifted");
    save_world(data,output/(prefix+"midpoint.json"),world);
    f::write_text(output/(prefix+"midpoint-report.json"),inspect(world).dump(2)+"\n");
    const auto before=world.snapshot();const sim::Command road{sim::CommandType::PlaceRoad,{23,19}};
    if(before.treasury>=sim::Rules::road_cost) {
        f::buy(world,road);
        check(world.treasury()==before.treasury-2 && world.construction_spent_total()==before.construction_spent_total+2 &&
              world.command_sequence()==before.command_sequence+1,"Actual paid continuation command changed its contract");
    } else {
        const auto refused=world.execute(road);
        check(!refused.accepted && refused.reason=="Not enough money" && world.snapshot()==before,
              "Debt continuation granted a road, rescue funds or mutated a refused command");
    }
    ticks(world,10000);check(world.ticks()==initial+20000,"Second replay period drifted");
    save_world(data,output/(prefix+"result.json"),world);
    f::write_text(output/(prefix+"result-report.json"),inspect(world).dump(2)+"\n");
}
json checkpoint(const f::fs::path& data,const f::fs::path& output,const char* tag,const sim::World& world) {
    const auto path=output/(std::string(tag)+".json");save_world(data,path,world);
    const auto document=save::read_save(path);const auto policy=world.map_permissions();
    check(document.source_schema_version==26 && document.world.rule_version==10,"Fixture did not write schema26/rule10");
    auto restored=save::restore_original_production_development(document,data,policy->building_mask(),policy);
    f::same_document(save::make_document(data,f::map_relative,policy->building_mask(),world),document);
    check(restored.snapshot()==world.snapshot() && inspect(restored)==inspect(world),"Full checkpoint authority changed");
    const auto bytes=f::read_text(path);save_world(data,path,restored);
    check(f::read_text(path)==bytes,"Canonical schema26 rewrite changed bytes");
    f::write_text(output/(std::string(tag)+"-report.json"),inspect(world).dump(2)+"\n");
    continuation(data,output,std::string(tag)+"-",restored);
    return {{"tag",tag},{"save",path.string()},{"report",(output/(std::string(tag)+"-report.json")).string()},
        {"expected_midpoint",(output/(std::string(tag)+"-midpoint.json")).string()},
        {"expected_midpoint_report",(output/(std::string(tag)+"-midpoint-report.json")).string()},
        {"expected_result",(output/(std::string(tag)+"-result.json")).string()},
        {"expected_report",(output/(std::string(tag)+"-result-report.json")).string()}};
}
void create(const f::fs::path& data,const f::fs::path& output) {
    const auto policy=f::permissions();
    auto world=sim::World::create_original_production_development(policy,save::prepare_original_production_rules_model(data));
    check(world.original_rules_model()->house_rules().value(original::ModelHouse::Shelter,original::HouseField::Hemp)==0 &&
          world.original_rules_model()->house_rules().value(original::ModelHouse::Shelter,original::HouseField::Ceramics)==0,
          "Real-model fixture requires its unchanged Shelter null needs");
    check(world.total_population()==0 && world.workforce_supply()==0 && world.treasury()==12000,
          "Production fixture did not start without residents, workers or inventory");
    for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,8}});
    for (int x=0;x<18;x+=3) f::buy(world,{sim::CommandType::PlaceHousehold,{x,6}});
    f::buy(world,{sim::CommandType::PlaceClaySource,{0,9}});
    f::buy(world,{sim::CommandType::PlacePottery,{8,9}});
    f::buy(world,{sim::CommandType::PlaceWarehouse,{16,9}});
    f::buy(world,{sim::CommandType::PlaceMarket,{21,9}});
    f::buy(world,{sim::CommandType::PlaceOriginalHempFarm,{4,9}});
    const auto clay=*world.building_owner_at({0,9}),pottery=*world.building_owner_at({8,9});
    const auto farm=*world.building_owner_at({4,9});
    sim::Command field{sim::CommandType::PlaceOriginalHempField,{6,9}};field.building_id=farm;f::buy(world,field);
    check(world.construction_spent_total()==1162 && world.treasury()==10838 && world.couriers().size()==7,
          "All actual scene purchases, explicit Field and typed actors were not charged");
    const auto initial=inspect(world);
    f::buy(world,sim::set_building_operation(clay,false));f::buy(world,sim::set_building_operation(farm,false));ticks(world,400);
    check(world.total_population()>0 && world.workforce_supply()>=34 && world.clay_extracted_total()==0 &&
          world.pottery_completed_total()==0 && world.original_goods_balance(original::Goods::Ceramics).stock==0 &&
          world.original_goods_balance(original::Goods::Hemp).totals.produced==0,
          "Stopped producer fabricated output or residents failed genuine migration");
    const auto stopped=inspect(world);f::buy(world,sim::set_building_operation(clay,true));
    f::buy(world,sim::set_building_operation(farm,true));
    bool midroute=false;
    for (int i=0;i<10000 && !midroute;++i) {
        world.tick();for (const auto& c:world.couriers())
            if (c.role==sim::CourierRole::OriginalHempToWarehouse && c.phase==sim::CourierPhase::ToWarehouse &&
                    world.original_courier_cargo(c.id)>0 && c.edge_progress>0) {midroute=true;break;}
    }
    check(midroute,"No actual typed Hemp cargo reached the mid-route checkpoint");
    json checkpoints=json::array();checkpoints.push_back(checkpoint(data,output,"mid-hemp-courier",world));
    auto returning=sim::World::restore_original_production_development(world.snapshot(),policy,world.original_rules_model());
    f::buy(returning,sim::set_building_operation(farm,false));
    const auto has_return=[&] {
        for (const auto& row:returning.original_goods_state()->reservations)
            if (row.source==farm && row.phase==sim::OriginalGoodsTransferPhase::Returning) return true;
        return false;
    };
    for (int i=0;i<1000 && !has_return();++i) returning.tick();
    check(has_return(),"Paused Hemp Farm did not retain its loaded trip as a real return");
    const auto return_save=output/"paused-hemp-return.json";save_world(data,return_save,returning);
    auto return_replay=save::restore_original_production_development(save::read_save(return_save),data,policy->building_mask(),policy);
    for (int i=0;i<1000;++i) {returning.tick();return_replay.tick();}
    check(returning.snapshot()==return_replay.snapshot(),"Paused typed return changed across persistence");
    bool returned=false;
    for (const auto& row:returning.original_goods_state()->reservations)
        returned=returned || (row.source==farm && row.phase==sim::OriginalGoodsTransferPhase::Returned && row.returned>0);
    check(returned,"Paused typed cargo failed to return to protected source space");
    const auto delivered=[&] {
        for (const auto& house:world.original_economy_state()->houses)
            if (house.house.hemp>0 && house.house.ceramics>0) return true;
        return false;
    };
    for (int i=0;i<10000 && !delivered();++i) world.tick();
    check(delivered(),"Actual Market arrivals did not reach two separate goods in one HouseState");
    checkpoints.push_back(checkpoint(data,output,"house-two-goods",world));
    // Test the ordinary incident clock on a separately restored real checkpoint;
    // no event, state, worker or material authority is injected.
    auto fire=sim::World::restore_original_production_development(world.snapshot(),policy,world.original_rules_model());
    f::buy(fire,sim::set_building_operation(clay,false));f::buy(fire,sim::set_building_operation(pottery,false));
    f::buy(fire,sim::set_building_operation(farm,false));
    for (int i=0;i<20000 && !(fire.building_on_fire(clay) && fire.building_on_fire(pottery));++i) fire.tick();
    check(fire.building_on_fire(clay) && fire.building_on_fire(pottery),"Natural placement-relative Fire did not start");
    f::buy(fire,sim::set_building_operation(clay,true));f::buy(fire,sim::set_building_operation(pottery,true));
    const auto extracted=fire.clay_extracted_total(),completed=fire.pottery_completed_total();ticks(fire,200);
    check(fire.clay_extracted_total()==extracted && fire.pottery_completed_total()==completed,
          "Burning sources ignored real Fire gates");
    f::write_text(output/"fixture.json",json{{"status","PASS"},{"authored_geography",true},
        {"snapshot_injection",false},{"positive_runtime_goods",true},{"paid_house_count",6},
        {"paid_road_count",24},{"paid_clay_count",1},{"paid_pottery_count",1},{"paid_warehouse_count",1},{"paid_market_count",1},
        {"paid_hemp_farm_count",1},{"paid_hemp_field_count",1},
        {"initial",initial},{"stopped",stopped},{"final",inspect(world)},
        {"stop_controls_verified",true},{"natural_fire_verified",true},{"fire",inspect(fire)},
        {"paused_hemp_return_verified",true},{"paused_return",inspect(returning)},{"actual_two_good_house_arrival",true},
        {"checkpoints",checkpoints}}.dump(2)+"\n");
}
void create_consumption(const f::fs::path& data,const f::fs::path& output) {
    const auto policy=f::permissions();
    auto world=sim::World::create_original_production_development(policy,save::prepare_original_production_rules_model(data));
    const auto& rules=world.original_rules_model()->house_rules();
    check(rules.value(original::ModelHouse::Shelter,original::HouseField::Hemp)==1 &&
          rules.value(original::ModelHouse::Shelter,original::HouseField::Ceramics)==1 &&
          rules.row_capacity(original::ModelHouse::Shelter)==7,"Positive fixture requires explicit authored 1/1 needs and capacity7");
    for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,8}});
    for (int x=0;x<18;x+=3) f::buy(world,{sim::CommandType::PlaceHousehold,{x,6}});
    f::buy(world,{sim::CommandType::PlaceClaySource,{0,9}});
    f::buy(world,{sim::CommandType::PlacePottery,{8,9}});
    f::buy(world,{sim::CommandType::PlaceWarehouse,{16,9}});
    f::buy(world,{sim::CommandType::PlaceMarket,{21,9}});
    f::buy(world,{sim::CommandType::PlaceOriginalHempFarm,{4,9}});
    sim::Command field{sim::CommandType::PlaceOriginalHempField,{6,9}};
    field.building_id=*world.building_owner_at({4,9});f::buy(world,field);
    check(world.construction_spent_total()==1162 && world.total_population()==0 && world.couriers().size()==7,
          "Authored-need scene was not constructed through regular paid commands");
    const auto initial=inspect(world);const auto market=*world.building_owner_at({21,9});
    json boundaries=json::array(),checkpoints=json::array();
    for (const auto step:{0,7}) {
        bool ready=false;
        for (int i=0;i<30000 && !ready;++i) {
            const auto dispatch=original::next_scheduler_dispatch(world.ticks());
            int hemp=0,ceramics=0;
            for (const auto& h:world.original_economy_state()->houses) {hemp+=h.house.hemp;ceramics+=h.house.ceramics;}
            ready=dispatch && dispatch->consumption && dispatch->position.step==step && hemp>0 && ceramics>0;
            if (!ready) world.tick();
        }
        check(ready,"Paid deliveries did not reach a positive 0/7 consumption boundary");
        f::buy(world,sim::set_building_operation(market,false));
        const auto before=world.snapshot();const auto before_report=inspect(world);
        const std::string tag="step-"+std::to_string(step)+"-before";
        checkpoints.push_back(checkpoint(data,output,tag.c_str(),world));
        world.tick();const auto at_report=inspect(world);json deltas=json::array();
        std::array<std::uint64_t,2> totals{};
        for (std::size_t h=0;h<before.original_economy->houses.size();++h) {
            const auto& previous=before.original_economy->houses[h];
            // Revision21 runs genuine phase39 evolution before the same0/7
            // consumption. Reconstruct that operation from the actual saved
            // input, retaining complete House equality and exact stock debits.
            auto input=previous.house;
            input.desirability=static_cast<std::int8_t>(world.original_influence_provenance(previous.building_id).value);
            auto evolved=original::evolve_step(rules,input);
            if (evolved.type>original::HouseType::PlainCottage) evolved.type=original::HouseType::PlainCottage;
            const auto& evaluation=world.original_production_state()->housing.houses[h].last_evaluation;
            check(evaluation&&evaluation->tick==world.ticks()&&evaluation->input==input,
                  "Consumption boundary had an unexpected arrival or different actual evolution input");
            const auto expected=original::consume_goods(rules,evolved);
            check(world.original_economy_state()->houses[h].house==expected.house,
                  "Consumption boundary had an unexpected House arrival or wrong debit");
            for (std::size_t g=0;g<2;++g) {
                totals[g]+=static_cast<std::uint64_t>(expected.consumed[g]);
                check(world.original_production_state()->house_consumption[h].consumed[g]==
                      before.original_production->house_consumption[h].consumed[g]+static_cast<std::uint64_t>(expected.consumed[g]),
                      "Boundary charged the House audit incorrectly");
            }
            deltas.push_back({{"building_id",static_cast<unsigned>(previous.building_id)},
                {"evolved_type",static_cast<unsigned>(evolved.type)},{"evolved_counter",evolved.devolve_counter},
                {"evolved_desirability",evolved.desirability},{"consumed",expected.consumed}});
        }
        check(totals[0]>0 && totals[1]>0,"Both actual goods must be consumed at each proved boundary");
        save_world(data,output/("step-"+std::to_string(step)+"-at.json"),world);
        const auto charged=world.original_production_state()->house_consumption;
        world.tick();const auto after_report=inspect(world);
        check(charged==world.original_production_state()->house_consumption,"Following scheduler phase repeated consumption");
        save_world(data,output/("step-"+std::to_string(step)+"-after.json"),world);
        boundaries.push_back({{"step",step},{"before",before_report},{"at",at_report},{"after",after_report},
            {"expected_house_deltas",deltas},{"house_arrivals_at_boundary",false}});
        f::buy(world,sim::set_building_operation(market,true));
    }
    f::write_text(output/"fixture.json",json{{"status","PASS"},{"authored_geography",true},
        {"authored_model_needs",true},{"original_model_parity",false},{"snapshot_injection",false},
        {"positive_runtime_consumption",true},{"paid_house_count",6},{"paid_road_count",24},
        {"initial",initial},{"final",inspect(world)},{"boundaries",boundaries},{"checkpoints",checkpoints}}.dump(2)+"\n");
}
void create_food(const f::fs::path& data,const f::fs::path& output) {
    const auto policy=f::permissions();
    auto world=sim::World::create_original_production_development(policy,save::prepare_original_production_rules_model(data));
    for(const int y:{5,8})for(int x=0;x<24;++x)f::buy(world,{sim::CommandType::PlaceRoad,{x,y}});
    for(const int y:{6,7})f::buy(world,{sim::CommandType::PlaceRoad,{23,y}});
    for(int x=0;x<24;++x)f::buy(world,{sim::CommandType::PlaceRoad,{x,12}});
    for(const int y:{9,10,11})f::buy(world,{sim::CommandType::PlaceRoad,{23,y}});
    for(const int y:{3,6})for(int x=0;x<18;x+=3)f::buy(world,{sim::CommandType::PlaceHousehold,y==1 ? oe_production_fixture::elite_row_cell(x/3):sim::Cell{x,y}});
    f::buy(world,{sim::CommandType::PlaceOriginalFoodFarm,{0,13}});
    f::buy(world,{sim::CommandType::PlaceOriginalFoodFarm,{5,13}});
    const auto wheat=*world.building_owner_at({0,13}),millet=*world.building_owner_at({5,13});
    sim::Command field{sim::CommandType::PlaceOriginalWheatField,{3,13}};field.building_id=wheat;f::buy(world,field);
    field={sim::CommandType::PlaceOriginalMilletField,{8,13}};field.building_id=millet;f::buy(world,field);
    f::buy(world,{sim::CommandType::PlaceMarket,{12,13}});const auto market=*world.building_owner_at({12,13});
    const auto initial=inspect(world);
    check(world.ticks()==0&&world.total_population()==0&&world.workforce_supply()==0&&world.couriers().size()==4&&
          world.construction_spent_total()==1363&&world.treasury()==10637,
          "Original-model Food scene must pay12Houses/77Roads/Farm193x2/Field195+196/Market without workers or stocks");
    f::buy(world,sim::set_building_operation(wheat,false));f::buy(world,sim::set_building_operation(millet,false));ticks(world,400);
    check(world.total_population()==84&&world.workforce_required(wheat)==22&&world.workforce_required(millet)==22,
          "Actual original-model Food producers lack12paidHouse immigration or model22workers");
    for(const auto& farm:world.original_production_state()->food.farms)check(!farm.harvested,"Stopped Food producer harvested");
    const auto stopped=inspect(world);f::buy(world,sim::set_building_operation(wheat,true));
    const auto wait=[&](auto done,const char* failure) {
        for(int i=0;i<30000&&!done();++i)world.tick();check(done(),failure);
    };
    sim::CourierId loaded=static_cast<sim::CourierId>(0);
    wait([&] {
        for(const auto& c:world.couriers())
            if(c.owner==wheat&&c.role==sim::CourierRole::OriginalFoodToMarket&&
               c.phase==sim::CourierPhase::ToWarehouse&&c.edge_progress>0&&world.original_food_cargo(c.id)>0) {
                loaded=c.id;return true;
            }
        return false;
    },
        "Real Food Farm courier never reached its loaded begun edge");
    json checkpoints=json::array();checkpoints.push_back(checkpoint(data,output,"mid-food-courier",world));
    auto returning=sim::World::restore_original_production_development(world.snapshot(),policy,world.original_rules_model());
    const auto moving=returning.courier(loaded);const auto retained=returning.original_food_cargo(loaded);
    f::buy(returning,sim::set_building_operation(wheat,false));
    check(returning.courier(loaded).phase==sim::CourierPhase::Returning&&returning.courier(loaded).edge_progress==moving.edge_progress&&
          returning.original_food_cargo(loaded)==retained,"Food return did not preserve its actual begun edge/cargo");
    checkpoints.push_back(checkpoint(data,output,"food-return",returning));
    auto returned=sim::World::restore_original_production_development(returning.snapshot(),policy,world.original_rules_model());
    for(int i=0;i<2000&&returned.courier(loaded).phase!=sim::CourierPhase::IdleAtWorkshop;++i)returned.tick();
    check(!returned.original_food_cargo(loaded),"Actual Food did not complete its return to source");
    const auto any_quality=[&](unsigned minimum) {
        for(const auto& h:world.original_economy_state()->houses)
            if(h.house.food_quality>=minimum)return true;
        return false;
    };
    wait([&]{return any_quality(20);},"Actual Wheat arrival did not supply low Food quality");
    const auto low=inspect(world);
    // Keep the destination Market running. A paid road interruption separates
    // only its residential branch while both Farms can genuinely deliver.
    const sim::Command cut{sim::CommandType::RemoveRoad,{23,10}};
    wait([&]{return world.validate(cut).accepted;},"Residential Food branch could not be safely disconnected");
    f::buy(world,cut);f::buy(world,sim::set_building_operation(millet,true));
    wait([&] {
        for(const auto& s:world.original_production_state()->food.stocks)
            if(s.building_id==market)return s.amounts[0]>0&&s.amounts[1]>0;
        return false;
    },
        "Both actual Farm crops never reached the same paid Market");
    const auto mixed_market=inspect(world);f::buy(world,{sim::CommandType::PlaceRoad,{23,10}});
    wait([&] {
        for(const auto& trip:world.original_production_state()->food.trips)
            if(world.courier(trip.transfer.courier_id).role==sim::CourierRole::OriginalFoodToHouse&&
               trip.loaded[0]>0&&trip.loaded[1]>0&&world.original_food_cargo(trip.transfer.courier_id)>0&&
               world.courier(trip.transfer.courier_id).edge_progress>0)return true;
        return false;
    },
        "Actual mixed Market cargo never reached a begun road edge");
    checkpoints.push_back(checkpoint(data,output,"mixed-food-courier",world));
    wait([&] {
        for(const auto& s:world.original_production_state()->food.stocks)
            if(s.quality_witness&&s.quality_witness->accepted[0]>0&&s.quality_witness->accepted[1]>0)return true;
        return false;
    },
        "Actual mixed Food never reached its House quality witness");
    const auto mixed_arrival=inspect(world);f::buy(world,sim::set_building_operation(wheat,false));
    wait([&]{return any_quality(80);},"Pure actual Millet arrival never reached its authored upper quality class");
    checkpoints.push_back(checkpoint(data,output,"house-food-quality",world));
    f::write_text(output/"fixture.json",json{{"status","PASS"},{"authored_geography",true},{"snapshot_injection",false},
        {"real_model_food",true},{"positive_runtime_food",true},{"paid_house_count",12},{"paid_road_count",77},
        {"paid_food_farm_count",2},{"paid_food_field_count",2},{"paid_market_count",1},
        {"initial",initial},{"stopped",stopped},{"low_quality",low},{"mixed_market",mixed_market},
        {"mixed_arrival",mixed_arrival},{"paused_return",inspect(returned)},{"final",inspect(world)},
        {"actual_mixed_cargo",true},{"actual_food_return",true},{"actual_quality_classes",json::array({20,80})},
        {"checkpoints",checkpoints}}.dump(2)+"\n");
}
void create_services(const f::fs::path& data,const f::fs::path& output) {
    const auto policy=f::permissions();
    auto world=sim::World::create_original_production_development(policy,save::prepare_original_production_rules_model(data));
    for (const int y:{5,8,12}) for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,y}});
    for (const int y:{6,7,9,10,11}) f::buy(world,{sim::CommandType::PlaceRoad,{23,y}});
    for (const int y:{3,6}) for (int x=0;x<18;x+=3) f::buy(world,{sim::CommandType::PlaceHousehold,y==1 ? oe_production_fixture::elite_row_cell(x/3):sim::Cell{x,y}});
    f::buy(world,{sim::CommandType::PlaceOriginalFoodFarm,{0,13}});
    f::buy(world,{sim::CommandType::PlaceOriginalFoodFarm,{5,13}});
    const auto wheat=*world.building_owner_at({0,13}),millet=*world.building_owner_at({5,13});
    sim::Command field{sim::CommandType::PlaceOriginalWheatField,{3,13}};field.building_id=wheat;f::buy(world,field);
    field={sim::CommandType::PlaceOriginalMilletField,{8,13}};field.building_id=millet;f::buy(world,field);
    f::buy(world,{sim::CommandType::PlaceMarket,{12,13}});
    f::buy(world,{sim::CommandType::PlaceHealthPost,{16,13}});
    f::buy(world,{sim::CommandType::PlaceServicePost,{19,13}});
    const auto herbalist=*world.building_owner_at({16,13}),acupuncture=*world.building_owner_at({19,13});
    const auto initial=inspect(world);
    check(world.total_population()==0&&world.couriers().size()==6&&world.construction_spent_total()==1443&&world.treasury()==10557,
          "Real-model medical/Food scene must pay12Houses/77Roads/twoFarms/twoFields/Market/207/208");
    const auto market=*world.building_owner_at({12,13});
    for(const auto& [owner,role]:std::array{
            std::pair{wheat,sim::CourierRole::OriginalFoodToMarket},
            std::pair{millet,sim::CourierRole::OriginalFoodToMarket},
            std::pair{market,sim::CourierRole::MarketPotteryDistribution},
            std::pair{market,sim::CourierRole::OriginalFoodToHouse},
            std::pair{herbalist,sim::CourierRole::HealthWorker},
            std::pair{acupuncture,sim::CourierRole::Service}})
        check(std::count_if(world.couriers().begin(),world.couriers().end(),[&](const auto& c){return c.owner==owner&&c.role==role;})==1,
              "Medical scene must own exactly its six paid Farm/Market/provider role pairs");
    f::buy(world,sim::set_building_operation(herbalist,false));
    f::buy(world,sim::set_building_operation(acupuncture,false));ticks(world,400);
    check(world.total_population()==84&&world.workforce_supply()==84&&world.workforce_required(herbalist)==7&&
          world.workforce_required(acupuncture)==8&&world.workforce_required(wheat)==22&&world.workforce_required(millet)==22,
          "Medical scene lacks actual paid-house immigration or exact prepared staffing");
    check(world.original_production_state()->services.trips.empty()&&world.original_production_state()->services.witnesses.empty(),
          "Paused medical sources created dispatch or visit authority");
    const auto stopped=inspect(world);
    const auto wait=[&](auto done,const char* failure) {
        for (int i=0;i<30000&&!done();++i) world.tick();
        check(done(),failure);
    };
    f::buy(world,sim::set_building_operation(herbalist,true));
    f::buy(world,sim::set_building_operation(acupuncture,true));
    wait([&] {
        std::size_t moving=0;
        for (const auto& c:world.couriers()) if (c.owner==herbalist||c.owner==acupuncture)
            moving+=c.phase==sim::CourierPhase::ToWarehouse&&c.edge_progress>0;
        return moving==2;
    },"Both real medical visitors did not reach outbound begun edges");
    check(world.original_production_state()->services.witnesses.empty(),"Medical dispatch or begun edge granted a timer");
    json checkpoints=json::array();checkpoints.push_back(checkpoint(data,output,"mid-services-couriers",world));
    f::buy(world,sim::set_building_operation(herbalist,false));
    f::buy(world,sim::set_building_operation(acupuncture,false));
    wait([&] {
        std::size_t idle=0;
        for (const auto& c:world.couriers()) if (c.owner==herbalist||c.owner==acupuncture)
            idle+=c.phase==sim::CourierPhase::IdleAtWorkshop;
        return idle==2&&world.original_production_state()->services.witnesses.size()==2;
    },"Paused outgoing207/208 visitors did not finish their actual visits and return");
    const auto visited=inspect(world);const auto witnesses=world.original_production_state()->services.witnesses;
    check(witnesses[0].building_id==witnesses[1].building_id&&witnesses[0].role!=witnesses[1].role,
          "Two medical visitors did not reach their fixed first paid House");
    checkpoints.push_back(checkpoint(data,output,"house-medical-visits",world));
    const auto first=std::find_if(witnesses.begin(),witnesses.end(),[](const auto& w){return w.role==original::ServiceRole::Herbalist;});
    check(first!=witnesses.end(),"Actual Herbalist witness missing");
    const auto expiry=36+(countdowns(first->arrival_tick-1)+95)*51;
    check(expiry>world.ticks(),"Medical process fixture missed its future96th countdown");
    while (world.ticks()+1<expiry) world.tick();
    const auto before_expiry=inspect(world);
    checkpoints.push_back(checkpoint(data,output,"service-before-expiry",world));
    world.tick();const auto expired=inspect(world);
    for (const auto& h:world.original_economy_state()->houses) if (h.building_id==first->building_id)
        check(!h.house.herbalist,"Herbalist survived the actual witnessed96th countdown");
    check(world.original_production_state()->services.witnesses==witnesses,"Return/idle/expiry created a new medical visit");
    checkpoints.push_back(checkpoint(data,output,"service-expired",world));
    f::write_text(output/"fixture.json",json{{"status","PASS"},{"snapshot_injection",false},{"authored_geography",true},
        {"real_model_services",true},{"actual_two_provider_arrivals",true},{"paused_outbound_tours_finished",true},
        {"paid_house_count",12},{"paid_road_count",77},{"paid_food_farm_count",2},{"paid_food_field_count",2},
        {"paid_market_count",1},{"paid_herbalist_count",1},{"paid_acupuncture_count",1},
        {"initial",initial},{"stopped",stopped},{"visited",visited},{"before_expiry",before_expiry},{"expired",expired},
        {"expiry_tick",expiry},{"checkpoints",checkpoints}}.dump(2)+"\n");
}
void create_ancestor(const f::fs::path& data,const f::fs::path& output) {
    const auto policy=f::permissions();
    auto world=sim::World::create_original_production_development(policy,save::prepare_original_production_rules_model(data));
    for (const int y:{5,8,12}) for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,y}});
    for (const int y:{6,7,9,10,11}) f::buy(world,{sim::CommandType::PlaceRoad,{23,y}});
    for (const int y:{3,6}) for (int x=0;x<18;x+=3) f::buy(world,{sim::CommandType::PlaceHousehold,y==1 ? oe_production_fixture::elite_row_cell(x/3):sim::Cell{x,y}});
    f::buy(world,{sim::CommandType::PlaceOriginalFoodFarm,{0,13}});
    f::buy(world,{sim::CommandType::PlaceOriginalFoodFarm,{5,13}});
    const auto wheat=*world.building_owner_at({0,13}),millet=*world.building_owner_at({5,13});
    sim::Command field{sim::CommandType::PlaceOriginalWheatField,{3,13}};field.building_id=wheat;f::buy(world,field);
    field={sim::CommandType::PlaceOriginalMilletField,{8,13}};field.building_id=millet;f::buy(world,field);
    f::buy(world,{sim::CommandType::PlaceMarket,{12,13}});
    const std::array<std::pair<sim::CommandType,sim::Cell>,3> sources{{
        {sim::CommandType::PlaceHealthPost,{16,13}},
        {sim::CommandType::PlaceServicePost,{19,13}},
        {sim::CommandType::PlaceOriginalAncestorShrine,{22,13}}}};
    std::array<sim::BuildingId,3> providers{};
    for (std::size_t i=0;i<sources.size();++i) {
        f::buy(world,{sources[i].first,sources[i].second});providers[i]=*world.building_owner_at(sources[i].second);
    }
    const auto shrine=providers[2];const auto home=*world.building_owner_at({0,3});
    const auto found=std::find_if(world.couriers().begin(),world.couriers().end(),[&](const auto& c){return c.owner==shrine;});
    check(found!=world.couriers().end()&&found->role==sim::CourierRole::OriginalAncestor,"Paid214 has no own actual Ancestor Courier");
    const auto visitor=found->id;
    const auto initial=inspect(world);
    check(world.total_population()==0&&world.couriers().size()==7&&world.construction_spent_total()==1483&&world.treasury()==10517,
          "Real Ancestor scene must pay12Houses/77Roads/twoFarms/twoFields/Market/207/208/214");
    for (const auto source:providers) f::buy(world,sim::set_building_operation(source,false));
    ticks(world,400);const auto stopped=inspect(world);
    check(world.total_population()==84&&world.workforce_supply()==84&&world.workforce_required(shrine)==4&&
          world.original_production_state()->services.trips.empty()&&world.original_production_state()->services.witnesses.empty(),
          "Paid Ancestor scene lacks real immigration or paused sources granted visits");
    const auto wait=[&](auto done,const char* failure,int limit=30000) {
        for (int i=0;i<limit&&!done();++i) world.tick();
        check(done(),failure);
    };
    const auto latest=[&]() -> const sim::OriginalServiceTrip& {
        for (const auto& row:world.original_production_state()->services.trips)
            if (row.transfer.courier_id==visitor) return row;
        throw std::runtime_error("Ancestor scene lost its real latest trip");
    };
    const auto witness=[&]() -> const sim::OriginalServiceWitness& {
        for (const auto& row:world.original_production_state()->services.witnesses)
            if (row.building_id==home&&row.role==original::ServiceRole::Ancestor) return row;
        throw std::runtime_error("Ancestor scene lost the actual first-House visit");
    };
    const auto timer=[&]() {
        for (const auto& row:world.original_economy_state()->houses) if (row.building_id==home) return row.house.ancestor;
        throw std::runtime_error("Ancestor scene lost its actual first House");
    };
    for (const auto source:providers) f::buy(world,sim::set_building_operation(source,true));
    wait([&]{return world.courier(visitor).phase==sim::CourierPhase::ToWarehouse&&world.courier(visitor).edge_progress>0;},
         "Real Ancestor visitor never reached its outbound begun edge");
    check(!latest().arrival_tick&&!timer()&&world.workforce_used()==67,
          "Ancestor dispatch granted religion or combined paid scene lacks67 actual workers");
    const auto midroute=inspect(world);json checkpoints=json::array();
    checkpoints.push_back(checkpoint(data,output,"mid-ancestor-courier",world));
    for (const auto source:providers) f::buy(world,sim::set_building_operation(source,false));
    const sim::Command cut{sim::CommandType::RemoveRoad,{23,10}};
    wait([&]{return world.validate(cut).accepted;},"Ancestor route cut never became safe");f::buy(world,cut);
    ticks(world,250);
    check(!latest().arrival_tick&&!timer(),"Ancestor visitor crossed a disconnected trunk or supplied on Return");
    const auto disconnected=inspect(world);
    checkpoints.push_back(checkpoint(data,output,"ancestor-road-disconnected",world));
    const auto before_repair=world.snapshot();f::buy(world,{sim::CommandType::PlaceRoad,{23,10}});
    check(world.treasury()==before_repair.treasury-2&&world.construction_spent_total()==before_repair.construction_spent_total+2,
          "Ancestor road repair did not pay the actual road cost");
    const auto repaired=inspect(world);
    wait([&]{return world.courier(visitor).phase==sim::CourierPhase::IdleAtWorkshop&&latest().visit_applied;},
         "Paused disconnected Ancestor tour did not finish after actual paid road repair");
    check(witness().transfer==latest().transfer&&latest().target==home&&timer()>0,
          "Ancestor arrival lost its exact first House/provider/actor witness");
    const auto visited=inspect(world);const auto first=witness();
    checkpoints.push_back(checkpoint(data,output,"house-ancestor-visit",world));
    const auto expiry=36+(countdowns(first.arrival_tick-1)+39)*51;
    check(expiry>world.ticks(),"Ancestor fixture missed its40th actual countdown");
    while (world.ticks()+1<expiry) world.tick();
    check(timer()==1,"Ancestor timer did not reach exactly1 before expiry");
    const auto before_expiry=inspect(world);
    checkpoints.push_back(checkpoint(data,output,"ancestor-before-expiry",world));
    world.tick();check(!timer()&&witness()==first,"Ancestor40th countdown changed witness or retained religion");
    const auto expired=inspect(world);
    checkpoints.push_back(checkpoint(data,output,"ancestor-expired",world));
    f::buy(world,sim::set_building_operation(shrine,true));
    wait([&]{return timer()>0&&witness().transfer.sequence>first.transfer.sequence;},
         "Real Ancestor target rotation never revisited the expired first House");
    f::buy(world,sim::set_building_operation(shrine,false));
    const auto revisited=inspect(world);
    checkpoints.push_back(checkpoint(data,output,"ancestor-revisited",world));
    f::write_text(output/"fixture.json",json{{"status","PASS"},{"snapshot_injection",false},{"authored_geography",true},
        {"real_model_ancestor",true},{"actual_ancestor_arrival",true},{"actual_ancestor_revisit",true},
        {"paid_road_repair",true},{"paused_outbound_tour_finished",true},
        {"paid_house_count",12},{"paid_road_count",77},{"paid_food_farm_count",2},{"paid_food_field_count",2},
        {"paid_market_count",1},{"paid_herbalist_count",1},{"paid_acupuncture_count",1},{"paid_ancestor_count",1},
        {"initial",initial},{"stopped",stopped},{"midroute",midroute},{"disconnected",disconnected},{"repaired",repaired},
        {"visited",visited},{"before_expiry",before_expiry},{"expired",expired},{"revisited",revisited},
        {"expiry_tick",expiry},{"checkpoints",checkpoints}}.dump(2)+"\n");
}
const original::HouseState& housing_house(const sim::World& world,sim::BuildingId id) {
    for (const auto& h:world.original_economy_state()->houses) if (h.building_id==id) return h.house;
    throw std::runtime_error("Housing fixture lost its paid House");
}
const sim::OriginalHousingHouse& housing_history(const sim::World& world,sim::BuildingId id) {
    for (const auto& h:world.original_production_state()->housing.houses) if (h.building_id==id) return h;
    throw std::runtime_error("Housing fixture lost its historical House row");
}
struct HousingScene {sim::BuildingId home{},farm{},market{},well{},shrine{};};
HousingScene housing_scene(sim::World& world,int missing=0) {
    const auto buy=[&](sim::CommandType type,sim::Cell cell) {
        f::buy(world,{type,cell});return *world.building_owner_at(cell);
    };
    for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,4}});
    HousingScene s;
    for (int x=0;x<15;x+=3) {const auto id=buy(sim::CommandType::PlaceHousehold,{x,2});if (!x)s.home=id;}
    s.farm=buy(sim::CommandType::PlaceOriginalFoodFarm,{0,5});
    if (missing!=3) {sim::Command crop{sim::CommandType::PlaceOriginalWheatField,{3,5}};crop.building_id=s.farm;f::buy(world,crop);}
    s.market=buy(sim::CommandType::PlaceMarket,{5,missing==4 ? 9:5});
    if (missing==1) (void)buy(sim::CommandType::PlaceOriginalInfluenceSource,{9,5});
    else s.well=buy(sim::CommandType::PlaceOriginalWaterWell,{9,5});
    if (missing!=2) s.shrine=buy(sim::CommandType::PlaceOriginalAncestorShrine,{13,5});
    return s;
}
std::string edited_housing_model(std::string bytes,int row,unsigned field,int value) {
    const std::string marker=row==1 ? "2: Hut,{,":"3: Plain Cottage,{,";
    const auto at=bytes.find(marker);check(at!=std::string::npos,"Housing authored negative row missing");
    auto start=at+marker.size();
    for (unsigned i=0;i<field;++i) {start=bytes.find(',',start);check(start!=std::string::npos,"Housing negative field missing");++start;}
    const auto end=bytes.find(',',start);check(end!=std::string::npos,"Housing negative value missing");
    check(std::stoi(bytes.substr(start,end-start))==(row==1 ? -2:0),"Housing negative source value changed");
    bytes.replace(start,end-start,std::to_string(value));return bytes;
}
void create_housing(const f::fs::path& data,const f::fs::path& output) {
    const auto policy=f::permissions();const auto model=save::prepare_original_production_rules_model(data);
    const auto fresh=[&]{return sim::World::create_original_production_development(policy,model);};
    auto world=fresh();const auto s=housing_scene(world);
    check(world.construction_spent_total()==708&&world.treasury()==11292&&!world.total_population()&&world.couriers().size()==5,
          "Real housing scene must pay708 for5Houses400/24Roads48/Farm50/Field5/Market140/Well25/Shrine40; actual="+
          std::to_string(world.construction_spent_total()));
    const auto initial=inspect(world);json checkpoints=json::array();
    const auto wait=[&](auto done,const char* failure,int limit=20000) {
        for (int i=0;i<limit&&!done();++i) world.tick();
        check(done(),failure);
    };
    wait([&]{return std::any_of(world.couriers().begin(),world.couriers().end(),[&](const auto& c){
        return c.owner==s.well&&c.role==sim::CourierRole::OriginalWater&&c.phase==sim::CourierPhase::ToWarehouse&&c.edge_progress>0;});},
        "Actual paid Water visitor never moved");
    const auto midroute=inspect(world);checkpoints.push_back(checkpoint(data,output,"mid-water-courier",world));
    wait([&]{
        for (const auto& h:world.original_production_state()->water.houses) {
            if (h.building_id==s.home) return h.current_witness.has_value();
        }
        return false;
    },"Actual Water visit never arrived");
    check(!housing_house(world,s.home).water,"Water fixture arrival must precede actual phase28 materialization");
    const auto arrived=inspect(world);checkpoints.push_back(checkpoint(data,output,"water-arrived-pre-evaluation",world));
    wait([&]{return housing_house(world,s.home).type==original::HouseType::Hut;},"Paid House never became actual Hut");
    const auto hut=inspect(world);checkpoints.push_back(checkpoint(data,output,"house-hut-upgrade",world));
    wait([&]{return housing_house(world,s.home).type==original::HouseType::PlainCottage;},"Real-model Hut never became actual Plain");
    const auto plain=inspect(world);const auto& proof=housing_history(world,s.home);
    check(proof.first_transitions[1]&&proof.first_transitions[2]&&proof.first_transitions[2]->evaluation.input.water&&
          proof.first_transitions[2]->evaluation.input.food_quality>=20&&proof.first_transitions[2]->evaluation.input.ancestor>0&&
          proof.first_transitions[2]->evaluation.service_witnesses[2]&&proof.first_transitions[2]->evaluation.food_quality_witness&&
          proof.first_transitions[2]->evaluation.water_evaluation_witness,"Actual first Plain lacks genuine historical inputs");
    checkpoints.push_back(checkpoint(data,output,"house-plain-upgrade",world));
    for (const auto source:{s.farm,s.market,s.well,s.shrine}) f::buy(world,sim::set_building_operation(source,false));
    wait([&]{const auto& h=housing_house(world,s.home);return h.type==original::HouseType::PlainCottage&&h.devolve_counter==9;},
         "Real failed-needs counter never reached9");
    const auto counter9=inspect(world);checkpoints.push_back(checkpoint(data,output,"house-devolution-counter9",world));
    wait([&]{return housing_house(world,s.home).type==original::HouseType::Hut;},"Actual tenth bad evaluation never devolved Plain");
    const auto& last=*housing_history(world,s.home).last_evaluation;
    check(last.input.type==original::HouseType::PlainCottage&&last.input.devolve_counter==9&&
          !housing_house(world,s.home).devolve_counter&&housing_house(world,s.home).occupants>=last.input.occupants,
          "Actual devolution failed original counter or invented resident eviction");
    const auto devolved=inspect(world);checkpoints.push_back(checkpoint(data,output,"house-devolved",world));
    json negatives=json::array();
    for (int missing=1;missing<=4;++missing) {
        auto denied=fresh();const auto n=housing_scene(denied,missing);ticks(denied,1700);
        for (const auto& h:denied.original_economy_state()->houses) {
            check(h.house.type!=original::HouseType::PlainCottage&&!housing_history(denied,h.building_id).first_transitions[2],
                  "Absent Water/Ancestor/Food/unreachableMarket permitted unearned Plain");
            if (missing==1) check(h.house.type==original::HouseType::Shelter&&!h.house.water,"Influence60 granted actual Water");
            if (missing==2) check(!h.house.ancestor,"NoAncestor negative acquired religion");
            if (missing>=3) check(!h.house.food_quality,"NoFood/unreachableMarket acquired quality");
        }
        if (missing!=1)check(housing_house(denied,n.home).water,"Other-prerequisite negative did not acquire genuine Water");
        negatives.push_back({{"case",std::array<const char*,4>{"no-water-influence-only","no-ancestor","no-food-field","unreachable-market"}[static_cast<std::size_t>(missing-1)]},
            {"authored_model",false},{"original_model_parity",true},{"final",inspect(denied)}});
    }
    const auto original_bytes=f::read_text(data/"Model/EmperorBuildingModels.txt");
    for (int variant=1;variant<=2;++variant) {
        const auto private_data=output/(variant==1 ? "authored-need1-data":"authored-desirability-data");
        f::fs::create_directories(private_data/"Model");f::fs::create_directories(private_data/"Cities");
        const auto changed=edited_housing_model(original_bytes,variant==1 ? 2:1,variant==1 ? 9U:1U,variant==1 ? 1:99);
        f::write_text(private_data/"Model/EmperorBuildingModels.txt",changed);
        f::write_text(private_data/f::map_relative,f::read_text(data/f::map_relative));
        auto denied=sim::World::create_original_production_development(policy,save::prepare_original_production_rules_model(private_data));
        const auto n=housing_scene(denied);bool supplied=false;
        for (int i=0;i<1700&&!supplied;++i) {
            denied.tick();const auto& h=housing_house(denied,n.home);supplied=h.water&&h.food_quality>=20&&h.ancestor>0;
        }
        check(supplied,"Authored negative did not obtain genuine remaining prerequisites");ticks(denied,150);
        check(housing_house(denied,n.home).type==original::HouseType::Hut&&!housing_history(denied,n.home).first_transitions[2],
              "Authored Need1/Des counterexample unexpectedly evolved");
        negatives.push_back({{"case",variant==1 ? "authored-missing-hemp-need1":"authored-insufficient-desirability"},
            {"authored_model",true},{"original_model_parity",false},{"model_path",(private_data/"Model/EmperorBuildingModels.txt").string()},
            {"changed_row",variant==1 ? 2:1},{"changed_field",variant==1 ? 9:1},{"before",variant==1 ? 0:-2},{"after",variant==1 ? 1:99},
            {"final",inspect(denied)}});
    }
    auto grand=fresh();
    const auto buy=[&](sim::CommandType type,sim::Cell cell) {f::buy(grand,{type,cell});return *grand.building_owner_at(cell);};
    for (const int y:{5,8,12}) for (int x=0;x<24;++x) f::buy(grand,{sim::CommandType::PlaceRoad,{x,y}});
    for (const int y:{6,7,9,10,11}) f::buy(grand,{sim::CommandType::PlaceRoad,{23,y}});
    for (const int y:{3,6}) for (int x=0;x<18;x+=3)(void)buy(sim::CommandType::PlaceHousehold,y==1 ? oe_production_fixture::elite_row_cell(x/3):sim::Cell{x,y});
    (void)buy(sim::CommandType::PlaceClaySource,{0,9});const auto hemp=buy(sim::CommandType::PlaceOriginalHempFarm,{4,9});
    sim::Command crop{sim::CommandType::PlaceOriginalHempField,{6,9}};crop.building_id=hemp;f::buy(grand,crop);
    (void)buy(sim::CommandType::PlacePottery,{8,9});(void)buy(sim::CommandType::PlaceWarehouse,{16,9});
    (void)buy(sim::CommandType::PlaceMarket,{21,9});const auto food=buy(sim::CommandType::PlaceOriginalFoodFarm,{0,13});
    crop={sim::CommandType::PlaceOriginalWheatField,{3,13}};crop.building_id=food;f::buy(grand,crop);
    (void)buy(sim::CommandType::PlaceHealthPost,{5,13});(void)buy(sim::CommandType::PlaceServicePost,{8,13});
    (void)buy(sim::CommandType::PlaceOriginalAncestorShrine,{11,13});(void)buy(sim::CommandType::PlaceOriginalWaterWell,{14,13});
    const auto grand_initial=inspect(grand);check(grand.construction_spent_total()==1948&&!grand.total_population(),"GrandIntegration must pay1948; actual="+
        std::to_string(grand.construction_spent_total()));
    bool complete=false;
    for (int i=0;i<10000&&!complete;++i) {
        grand.tick();complete=std::any_of(grand.original_economy_state()->houses.begin(),grand.original_economy_state()->houses.end(),[](const auto& h){
            return h.house.type==original::HouseType::PlainCottage&&h.house.hemp>0&&h.house.ceramics>0&&h.house.food_quality>=20&&
                h.house.herbalist>0&&h.house.acupuncture>0&&h.house.ancestor>0&&h.house.water;});
    }
    check(complete,"GrandIntegration never combined actual material/Food/Water/medical/Ancestor arrivals with Plain");
    const auto grand_final=inspect(grand);
    f::write_text(output/"fixture.json",json{{"status","PASS"},{"snapshot_injection",false},{"authored_geography",true},
        {"real_model_housing",true},{"actual_first_hut_and_plain",true},{"actual_tenth_failure_devolution",true},
        {"grand_actual_all_arrivals",true},{"home_id",static_cast<unsigned>(s.home)},
        {"initial",initial},{"midroute",midroute},{"arrived",arrived},{"hut",hut},{"plain",plain},
        {"counter9",counter9},{"devolved",devolved},{"negatives",negatives},
        {"grand_initial",grand_initial},{"grand_final",grand_final},{"checkpoints",checkpoints}}.dump(2)+"\n");
}
int academy_paper(const sim::World& w,sim::BuildingId academy){for(const auto& row:w.original_production_state()->industry.auxiliary)if(row.building_id==academy)return row.amounts[4];return 0;}
oe_production_fixture::PaperSupply process_paper_kit(sim::World& w,bool culture=false,const std::vector<sim::Cell>& homes={},bool elite_layout=false){
    oe_production_fixture::PaperSupplyLayout kit;kit.farm={0,17};kit.field={2,17};kit.paper={4,17};kit.warehouse={8,17};kit.extra_homes=homes;
    for(int x=0;x<=12;++x)kit.roads.push_back({x,16});for(int y=culture?9:5;y<16;++y)kit.roads.push_back({12,y});
    const std::vector<int> columns=culture?std::vector<int>{9,12,15,18,21}:elite_layout?std::vector<int>{6,9}:std::vector<int>{2,5};
    for(const int x:columns)for(int y=0;y<4;++y)kit.roads.push_back({x,y});return oe_production_fixture::add_paper_supply(w,kit);
}
void create_religion(const f::fs::path& data,const f::fs::path& output) {
    const auto policy=f::permissions();const auto model=save::prepare_original_production_rules_model(data);
    constexpr std::array<sim::CommandType,3> types{sim::CommandType::PlaceOriginalDaoistShrine,
        sim::CommandType::PlaceOriginalBuddhistShrine,sim::CommandType::PlaceOriginalConfucianAcademy};
    constexpr std::array<original::ServiceRole,3> roles{original::ServiceRole::Daoist,original::ServiceRole::Buddhist,original::ServiceRole::Confucian};
    const auto buy=[](sim::World& w,sim::CommandType type,sim::Cell cell) {f::buy(w,{type,cell});return *w.building_owner_at(cell);};
    auto world=sim::World::create_original_production_development(policy,model);
    for(int x=0;x<24;++x)f::buy(world,{sim::CommandType::PlaceRoad,{x,4}});
    sim::BuildingId home{};for(int x=0;x<24;x+=3){const auto id=buy(world,x ? sim::CommandType::PlaceHousehold:sim::CommandType::PlaceOriginalElitePlot,oe_production_fixture::elite_row_cell(x/3));if(!x)home=id;}
    std::array<sim::BuildingId,3> providers{};std::array<sim::CourierId,3> actors{};
    const auto spent_before=world.construction_spent_total();
    for(unsigned i=0;i<3;++i){providers[i]=buy(world,types[i],{10+4*static_cast<int>(i),5});for(const auto& c:world.couriers())if(c.owner==providers[i])actors[i]=c.id;}
    check(world.construction_spent_total()-spent_before==170,"Actual three religion models must independently cost45+45+80=170");
    std::vector<sim::BuildingId> all_sources(providers.begin(),providers.end());
    all_sources.push_back(buy(world,sim::CommandType::PlaceHealthPost,{0,5}));
    all_sources.push_back(buy(world,sim::CommandType::PlaceServicePost,{3,5}));
    all_sources.push_back(buy(world,sim::CommandType::PlaceOriginalAncestorShrine,{6,5}));
    all_sources.push_back(buy(world,sim::CommandType::PlaceOriginalWaterWell,{22,5}));
    const auto supply=process_paper_kit(world,false,{{4,0},{7,0},{10,0}},true);check(supply.spent==568,"Religion normal Paper chain bill differs");
    check(world.construction_spent_total()==1571&&world.treasury()==10429&&!world.total_population(),"Religion integration must pay1571 before genuine immigration");
    const auto initial=inspect(world);for(const auto source:all_sources)f::buy(world,sim::set_building_operation(source,false));ticks(world,400);
    const auto stopped=inspect(world);check(world.total_population()==75&&world.workforce_supply()==75&&world.original_production_state()->services.trips.empty(),"Paused religious sources granted free workers or visits");
    const auto wait=[&](auto done,const char* reason,int limit=20000){for(int i=0;i<limit&&!done();++i)world.tick();check(done(),reason);};
    const auto latest=[&](unsigned i)->const sim::OriginalServiceWitness& {for(const auto& row:world.original_production_state()->services.witnesses)if(row.building_id==home&&row.role==roles[i])return row;throw std::runtime_error("Religion fixture lost genuine witness");};
    wait([&]{return academy_paper(world,providers[2])>0;},"Actual paid Paper never reached paused Academy");
    for(const auto source:all_sources)f::buy(world,sim::set_building_operation(source,true));
    wait([&]{return std::all_of(actors.begin(),actors.end(),[&](auto id){const auto& c=world.courier(id);return c.phase==sim::CourierPhase::ToWarehouse&&c.edge_progress>0;});},"Three real religion couriers never shared an outbound begun edge");
    check(world.workforce_used()==73&&!housing_house(world,home).daoist&&!housing_house(world,home).buddhist&&!housing_house(world,home).confucian,"Religion dispatch granted a timer before arrival or lacks43 actual workers");
    const auto midroute=inspect(world);json checkpoints=json::array();checkpoints.push_back(checkpoint(data,output,"mid-religion-couriers",world));
    for(const auto source:providers)f::buy(world,sim::set_building_operation(source,false));
    const sim::Command cut{sim::CommandType::RemoveRoad,{8,4}};wait([&]{return world.validate(cut).accepted;},"Religion trunk road never became safe to remove");f::buy(world,cut);ticks(world,250);
    check(!housing_house(world,home).daoist&&!housing_house(world,home).buddhist&&!housing_house(world,home).confucian,"Religion crossed a removed shared-navigation trunk");
    const auto disconnected=inspect(world);checkpoints.push_back(checkpoint(data,output,"religion-road-disconnected",world));
    const auto before_repair=world.snapshot();f::buy(world,{sim::CommandType::PlaceRoad,{8,4}});
    check(world.treasury()==before_repair.treasury-2&&world.construction_spent_total()==before_repair.construction_spent_total+2,"Religion trunk repair did not pay2");
    wait([&]{const auto& h=housing_house(world,home);return h.daoist&&h.buddhist&&h.confucian&&h.ancestor&&h.herbalist&&h.acupuncture&&h.water&&std::all_of(actors.begin(),actors.end(),[&](auto id){return world.courier(id).phase==sim::CourierPhase::IdleAtWorkshop;});},"Paused religion tours failed real repaired arrivals beside existing medicine/Ancestor/Water");
    const auto visited=inspect(world);std::array<sim::OriginalServiceWitness,3> first{};std::array<std::uint64_t,3> expiries{};
    for(unsigned i=0;i<3;++i){first[i]=latest(i);expiries[i]=36+(countdowns(first[i].arrival_tick-1)+39)*51;}
    checkpoints.push_back(checkpoint(data,output,"house-three-religions",world));
    const auto expiry=*std::max_element(expiries.begin(),expiries.end());check(expiry>world.ticks(),"Religion fixture missed40-step expiry");
    while(world.ticks()+1<expiry){world.tick();}
    check(std::any_of(roles.begin(),roles.end(),[&](auto role){return original::service_timer(housing_house(world,home),role)==1;}),"Last religion timer did not reach1");
    const auto before_expiry=inspect(world);checkpoints.push_back(checkpoint(data,output,"religion-before-expiry",world));world.tick();
    for(unsigned i=0;i<3;++i)check(!original::service_timer(housing_house(world,home),roles[i])&&latest(i)==first[i],"Religion40th countdown retained a timer or rewrote its witness");
    const auto expired=inspect(world);checkpoints.push_back(checkpoint(data,output,"religion-expired",world));
    for(const auto source:providers)f::buy(world,sim::set_building_operation(source,true));
    wait([&]{for(unsigned i=0;i<3;++i)if(!original::service_timer(housing_house(world,home),roles[i])||latest(i).transfer.sequence<=first[i].transfer.sequence)return false;return true;},"Independent religious rotations never genuinely revisited the expired House");
    for(const auto source:providers)f::buy(world,sim::set_building_operation(source,false));
    const auto revisited=inspect(world);checkpoints.push_back(checkpoint(data,output,"religion-revisited",world));
    json predicates=json::array();const auto bytes=f::read_text(data/"Model/EmperorBuildingModels.txt");
    for(int mode=0;mode<5;++mode){
        const auto private_data=output/("authored-or-"+std::to_string(mode));f::fs::create_directories(private_data/"Model");f::fs::create_directories(private_data/"Cities");
        auto changed=edited_housing_model(bytes,2,23,1);if(mode>=3)changed=edited_housing_model(std::move(changed),2,22,1);
        f::write_text(private_data/"Model/EmperorBuildingModels.txt",changed);f::write_text(private_data/f::map_relative,f::read_text(data/f::map_relative));
        auto candidate=sim::World::create_original_production_development(policy,save::prepare_original_production_rules_model(private_data));
        for(int x=0;x<24;++x)f::buy(candidate,{sim::CommandType::PlaceRoad,{x,4}});
        sim::BuildingId first_home{};for(int x=0;x<24;x+=3){const auto id=buy(candidate,sim::CommandType::PlaceHousehold,{x,2});if(!x)first_home=id;}
        const auto farm=buy(candidate,sim::CommandType::PlaceOriginalFoodFarm,{0,5});sim::Command field{sim::CommandType::PlaceOriginalWheatField,{3,5}};field.building_id=farm;f::buy(candidate,field);
        (void)buy(candidate,sim::CommandType::PlaceMarket,{5,5});(void)buy(candidate,sim::CommandType::PlaceOriginalWaterWell,{9,5});(void)buy(candidate,sim::CommandType::PlaceOriginalAncestorShrine,{13,5});
        std::optional<sim::BuildingId> single,control_academy;
        if(mode!=2) single=buy(candidate,types[mode==1?1:0],{16,5});
        if(mode!=3) control_academy=buy(candidate,types[2],{19,5});
        if(mode!=3){(void)process_paper_kit(candidate,false,{{0,0},{3,0},{6,0},{9,0},{12,0}});for(const int x:{8,11})for(int y=0;y<4;++y)if(candidate.object_at({x,y})!=sim::Object::Road)f::buy(candidate,{sim::CommandType::PlaceRoad,{x,y}});}
        const auto populated=[&]{const auto& h=housing_house(candidate,first_home);return h.water&&h.food_quality>=20&&h.ancestor&&(mode==2 ? true:mode==1 ? h.buddhist>0:h.daoist>0);};
        for(int i=0;i<10000&&!populated();++i){candidate.tick();}
        check(populated(),"Authored religion controls lacked actual other prerequisites");
        check(!housing_house(candidate,first_home).confucian,"Common control bypassed actual Elite-only Academy");
        if(mode==4) {
            check(control_academy.has_value(),"Common refusal control lacks its actual paid Academy ID");
            const auto simultaneously_ready=[&] {
                const auto availability=candidate.original_service_availability(first_home,original::ServiceRole::Confucian);
                const auto& academy_body=candidate.building(*control_academy);
                return populated()&&academy_paper(candidate,*control_academy)>0&&
                    academy_body.operating_enabled&&!candidate.building_on_fire(*control_academy)&&
                    candidate.workers_assigned(*control_academy)==candidate.workforce_required(*control_academy)&&
                    availability.provider_count==1&&availability.available_source_count==1;
            };
            for(int n=0;n<10000&&!simultaneously_ready();++n) candidate.tick();
            check(simultaneously_ready(),"Actual Academy Paper/full staffing and remaining Common prerequisites never coincided");
            check(!candidate.original_service_availability(first_home,original::ServiceRole::Confucian).eligible_target,
                  "Paid Paper-supplied fully staffed Academy accepted a Common target");
        }
        const auto prerequisites=inspect(candidate);
        if(mode>=2)check(!original::needs_met(candidate.original_rules_model()->house_rules(),original::ModelHouse::PlainCottage,housing_house(candidate,first_home)),"Missing actual religion category met authored Plain with remaining prerequisites supplied");
        if(mode<2){for(int i=0;i<10000&&housing_house(candidate,first_home).type!=original::HouseType::PlainCottage;++i)candidate.tick();check(housing_house(candidate,first_home).type==original::HouseType::PlainCottage,"Actual single-faith OR did not upgrade authored Plain");}
        else{ticks(candidate,150);check(housing_house(candidate,first_home).type==original::HouseType::Hut&&!housing_history(candidate,first_home).first_transitions[2],"Missing OR/Confucian category passed authored Plain Need1");}
        const auto final=inspect(candidate);json after_expired=nullptr;
        if(mode<2){f::buy(candidate,sim::set_building_operation(*single,false));const auto role=mode==1?original::ServiceRole::Buddhist:original::ServiceRole::Daoist;
            for(int i=0;i<10000&&original::service_timer(housing_house(candidate,first_home),role);++i)candidate.tick();
            check(!original::service_timer(housing_house(candidate,first_home),role)&&!original::needs_met(candidate.original_rules_model()->house_rules(),original::ModelHouse::PlainCottage,housing_house(candidate,first_home)),"Expired actual single-faith visit retained authored OR need");after_expired=inspect(candidate);
        }
        predicates.push_back({{"case",std::array<const char*,5>{"dao-only","buddhist-only","neither-or","no-confucian","common-confucian-refused"}[static_cast<unsigned>(mode)]},{"authored_model",true},{"original_model_or_requirement",false},{"home_id",static_cast<unsigned>(first_home)},{"model_path",(private_data/"Model/EmperorBuildingModels.txt").string()},{"prerequisites",prerequisites},{"final",final},{"expired",after_expired}});
    }
    auto water_cells=policy->cells();for(int y=0;y<20;++y)water_cells[static_cast<std::size_t>(y*24+11)]={false,false,false,0,sim::BuildBlocker::UnsupportedBridgeWater,sim::BuildBlocker::UnsupportedTerrain,false,sim::OriginalRoadClassification::WaterOrBridge,true};
    const auto barrier_policy=std::make_shared<const sim::MapPermissions>(24,20,3,std::move(water_cells),std::vector<sim::FixedGatePassage>{});
    auto barrier=sim::World::create_original_production_development(barrier_policy,model);
    for(int x=0;x<24;++x)if(x!=11)f::buy(barrier,{sim::CommandType::PlaceRoad,{x,4}});
    for(int x=0;x<9;x+=3){(void)buy(barrier,sim::CommandType::PlaceHousehold,{x,2});}
    for(unsigned i=0;i<3;++i)(void)buy(barrier,types[i],{12+4*static_cast<int>(i),5});
    oe_production_fixture::PaperSupplyLayout barrier_kit;barrier_kit.farm={12,17};barrier_kit.field={14,17};barrier_kit.paper={16,17};barrier_kit.warehouse={20,17};barrier_kit.extra_homes={{0,0},{3,0},{0,6},{3,6},{6,6}};
    for(int x=0;x<10;++x)barrier_kit.roads.push_back({x,8});for(int y=5;y<8;++y)barrier_kit.roads.push_back({9,y});for(int y=0;y<4;++y)barrier_kit.roads.push_back({2,y});for(int x=12;x<24;++x)barrier_kit.roads.push_back({x,16});for(int y=5;y<16;++y)barrier_kit.roads.push_back({23,y});(void)oe_production_fixture::add_paper_supply(barrier,barrier_kit);
    const auto barrier_before=barrier.snapshot();check(!barrier.execute({sim::CommandType::PlaceRoad,{11,4}}).accepted&&barrier.snapshot()==barrier_before,"Immutable authored normal-water barrier accepted an atomic crossing");ticks(barrier,4000);
    check(barrier.workforce_used()==50&&barrier.original_production_state()->services.trips.empty()&&barrier.original_production_state()->services.witnesses.empty(),"Paid religions crossed actual policy water or lacked full workforce");
    f::write_text(output/"fixture.json",json{{"status","PASS"},{"snapshot_injection",false},{"authored_geography",true},{"real_model_prices_and_workers",true},
        {"original_confucian_resource_gate",true},{"confucian_adapter","actual paid Hemp→Paper→Warehouse→Academy, authored recipe/route timing"},{"resource_grant",false},
        {"home_id",static_cast<unsigned>(home)},{"religion_source_cost",170},{"religion_source_workers",20},{"combined_workers",73},
        {"initial",initial},{"stopped",stopped},{"midroute",midroute},{"disconnected",disconnected},{"visited",visited},{"before_expiry",before_expiry},{"expired",expired},{"revisited",revisited},
        {"expiry_ticks",expiries},{"expiry_tick",expiry},{"predicate_controls",predicates},{"barrier",inspect(barrier)},{"barrier_authored_topology",true},{"barrier_shared_policy_navigation",true},{"checkpoints",checkpoints}}.dump(2)+"\n");
}
void create_culture(const f::fs::path& data,const f::fs::path& output) {
    const auto policy=f::permissions();const auto model=save::prepare_original_production_rules_model(data);
    constexpr std::array<sim::CommandType,3> commands{sim::CommandType::PlaceOriginalMusicSchool,sim::CommandType::PlaceOriginalAcrobatSchool,sim::CommandType::PlaceOriginalDramaSchool};
    constexpr std::array<original::ServiceRole,3> roles{original::ServiceRole::Music,original::ServiceRole::Acrobat,original::ServiceRole::Drama};
    const auto buy=[](sim::World& w,sim::CommandType type,sim::Cell cell){f::buy(w,{type,cell});return *w.building_owner_at(cell);};
    auto world=sim::World::create_original_production_development(policy,model);
    for(int x=0;x<24;++x)f::buy(world,{sim::CommandType::PlaceRoad,{x,4}});
    sim::BuildingId home{};for(int x=0;x<24;x+=3){const auto id=buy(world,x ? sim::CommandType::PlaceHousehold:sim::CommandType::PlaceOriginalElitePlot,oe_production_fixture::elite_row_cell(x/3));if(!x)home=id;}
    (void)buy(world,sim::CommandType::PlaceHousehold,{4,0});(void)buy(world,sim::CommandType::PlaceHousehold,{7,0});
    for(int y=0;y<4;++y)f::buy(world,{sim::CommandType::PlaceRoad,{6,y}});
    for(int x=0;x<=12;++x)f::buy(world,{sim::CommandType::PlaceRoad,{x,8}});
    for(int y=5;y<8;++y)f::buy(world,{sim::CommandType::PlaceRoad,{12,y}});
    std::vector<sim::BuildingId> all_sources;
    for(const auto& [type,cell]:std::array{std::pair{sim::CommandType::PlaceOriginalDaoistShrine,sim::Cell{10,5}},std::pair{sim::CommandType::PlaceOriginalBuddhistShrine,sim::Cell{14,5}},std::pair{sim::CommandType::PlaceOriginalConfucianAcademy,sim::Cell{18,5}},std::pair{sim::CommandType::PlaceHealthPost,sim::Cell{0,5}},std::pair{sim::CommandType::PlaceServicePost,sim::Cell{3,5}},std::pair{sim::CommandType::PlaceOriginalAncestorShrine,sim::Cell{6,5}},std::pair{sim::CommandType::PlaceOriginalWaterWell,sim::Cell{22,5}}})all_sources.push_back(buy(world,type,cell));
    std::array<sim::BuildingId,3> providers{};std::array<sim::CourierId,3> actors{};const auto spent=world.construction_spent_total();
    for(unsigned i=0;i<3;++i){providers[i]=buy(world,commands[i],{4*static_cast<int>(i),9});all_sources.push_back(providers[i]);for(const auto& c:world.couriers())if(c.owner==providers[i])actors[i]=c.id;}
    check(world.construction_spent_total()-spent==180,"Actual three cultural sources did not cost180");
    const auto supply=process_paper_kit(world,true,{{10,0},{13,0},{16,0},{19,0},{22,0}},true);check(supply.spent==744,"Culture normal Paper kit bill differs");
    check(world.construction_spent_total()==2127&&world.treasury()==9873&&!world.total_population()&&world.couriers().size()==14,"Culture scene must pay2127 including real Paper supply/15Houses/84Roads");
    const auto initial=inspect(world);for(const auto id:all_sources)f::buy(world,sim::set_building_operation(id,false));ticks(world,400);
    check(world.total_population()==103&&world.workforce_supply()==103&&world.original_production_state()->services.trips.empty(),"Culture scene lacks70 actual admitted Shelter workers or paused source granted visit");const auto stopped=inspect(world);
    const auto wait=[&](auto done,const char* reason,int limit=20000){for(int i=0;i<limit&&!done();++i)world.tick();check(done(),reason);};
    const auto latest=[&](unsigned i)->const sim::OriginalServiceWitness&{for(const auto& row:world.original_production_state()->services.witnesses)if(row.building_id==home&&row.role==roles[i])return row;throw std::runtime_error("Culture lost actual target visit witness");};
    wait([&]{return academy_paper(world,all_sources[2])>0;},"Actual Paper never reached paused Culture-scene Academy");
    for(const auto id:all_sources)f::buy(world,sim::set_building_operation(id,true));
    wait([&]{return std::all_of(actors.begin(),actors.end(),[&](auto id){const auto& c=world.courier(id);return c.phase==sim::CourierPhase::ToWarehouse&&c.edge_progress>0;});},"Three real school actors never shared genuine outbound edges");
    check(world.workforce_used()==100,"Culture scene did not actually staff all70 workers at dispatch");for(const auto role:roles){const auto d=world.original_service_availability(home,role);check(d.available_source_count==1&&d.reachable_source_count==1&&d.travelling_to_target_count==1&&!d.delivered_timer,"Available/inflight culture was confused with delivered access");}
    const auto midroute=inspect(world);json checkpoints=json::array();checkpoints.push_back(checkpoint(data,output,"mid-culture-couriers",world));
    for(const auto id:providers)f::buy(world,sim::set_building_operation(id,false));const sim::Command cut{sim::CommandType::RemoveRoad,{12,6}};
    wait([&]{return world.validate(cut).accepted;},"Culture trunk never became safe to remove");f::buy(world,cut);ticks(world,250);
    for(const auto role:roles)check(!original::service_timer(housing_house(world,home),role),"Culture crossed actual disconnected trunk before arrival");
    const auto disconnected=inspect(world);checkpoints.push_back(checkpoint(data,output,"culture-road-disconnected",world));const auto before_repair=world.snapshot();f::buy(world,{sim::CommandType::PlaceRoad,{12,6}});
    check(world.treasury()==before_repair.treasury-2&&world.construction_spent_total()==before_repair.construction_spent_total+2,"Culture road repair did not pay2");
    wait([&]{const auto& h=housing_house(world,home);for(unsigned r=0;r<9;++r)if(!original::service_timer(h,static_cast<original::ServiceRole>(r)))return false;return h.water&&std::all_of(actors.begin(),actors.end(),[&](auto id){return world.courier(id).phase==sim::CourierPhase::IdleAtWorkshop;});},"Paused school tours did not finish actual nine-category/Water arrivals");
    check(world.workforce_used()==73,"Paused27 culture workers did not leave exactly43 prior active workers");const auto visited=inspect(world);checkpoints.push_back(checkpoint(data,output,"house-three-cultures",world));
    std::array<sim::OriginalServiceWitness,3> first{};std::array<std::uint64_t,3> expiries{};for(unsigned i=0;i<3;++i){first[i]=latest(i);expiries[i]=36+(countdowns(first[i].arrival_tick-1)+95)*51;}
    const auto expiry=*std::max_element(expiries.begin(),expiries.end());check(expiry>world.ticks(),"Culture missed its actual96th countdown");while(world.ticks()+1<expiry){world.tick();}
    check(std::any_of(roles.begin(),roles.end(),[&](auto role){return original::service_timer(housing_house(world,home),role)==1;}),"Culture timer did not reach1 before last expiry");const auto before_expiry=inspect(world);checkpoints.push_back(checkpoint(data,output,"culture-before-expiry",world));world.tick();
    for(unsigned i=0;i<3;++i)check(!original::service_timer(housing_house(world,home),roles[i])&&latest(i)==first[i],"Culture96th countdown retained access or rewrote origin");
    const auto expired=inspect(world);checkpoints.push_back(checkpoint(data,output,"culture-expired",world));for(const auto id:providers)f::buy(world,sim::set_building_operation(id,true));
    wait([&]{for(unsigned i=0;i<3;++i)if(!original::service_timer(housing_house(world,home),roles[i])||latest(i).transfer.sequence<=first[i].transfer.sequence)return false;return true;},"Private school rotations did not genuinely revisit expired target");
    for(const auto id:providers)f::buy(world,sim::set_building_operation(id,false));const auto revisited=inspect(world);checkpoints.push_back(checkpoint(data,output,"culture-revisited",world));
    json controls=json::array();const auto bytes=f::read_text(data/"Model/EmperorBuildingModels.txt");
    for(unsigned index=0;index<3;++index){const auto private_data=output/("authored-culture-"+std::to_string(index));f::fs::create_directories(private_data/"Model");f::fs::create_directories(private_data/"Cities");f::write_text(private_data/"Model/EmperorBuildingModels.txt",edited_housing_model(bytes,2,5+index,1));f::write_text(private_data/f::map_relative,f::read_text(data/f::map_relative));auto candidate=sim::World::create_original_production_development(policy,save::prepare_original_production_rules_model(private_data));for(int x=0;x<24;++x)f::buy(candidate,{sim::CommandType::PlaceRoad,{x,4}});sim::BuildingId target{};for(int x=0;x<18;x+=3){const auto id=buy(candidate,sim::CommandType::PlaceHousehold,{x,2});if(!x)target=id;}const auto farm=buy(candidate,sim::CommandType::PlaceOriginalFoodFarm,{0,5});sim::Command field{sim::CommandType::PlaceOriginalWheatField,{3,5}};field.building_id=farm;f::buy(candidate,field);(void)buy(candidate,sim::CommandType::PlaceMarket,{5,5});(void)buy(candidate,sim::CommandType::PlaceOriginalWaterWell,{9,5});(void)buy(candidate,sim::CommandType::PlaceOriginalAncestorShrine,{13,5});const auto school=buy(candidate,commands[index],{17,5});f::buy(candidate,sim::set_building_operation(school,false));const auto supplied=[&]{const auto& h=housing_house(candidate,target);return h.water&&h.food_quality>=20&&h.ancestor;};for(int i=0;i<20000&&!supplied();++i){candidate.tick();}check(supplied()&&!original::service_timer(housing_house(candidate,target),roles[index])&&!original::needs_met(candidate.original_rules_model()->house_rules(),original::ModelHouse::PlainCottage,housing_house(candidate,target)),"Authored culture negative lacked true remaining prerequisites or passed missing category");const auto prerequisites=inspect(candidate);f::buy(candidate,sim::set_building_operation(school,true));for(int i=0;i<20000&&housing_house(candidate,target).type!=original::HouseType::PlainCottage;++i){candidate.tick();}check(housing_house(candidate,target).type==original::HouseType::PlainCottage&&housing_history(candidate,target).first_transitions[2]&&housing_history(candidate,target).first_transitions[2]->evaluation.service_witnesses[6+index],"Actual culture did not upgrade authored Need1 through appended historical slot");const auto delivered=inspect(candidate);f::buy(candidate,sim::set_building_operation(school,false));for(int i=0;i<20000&&(original::service_timer(housing_house(candidate,target),roles[index])||!supplied());++i){candidate.tick();}check(supplied()&&!original::service_timer(housing_house(candidate,target),roles[index])&&!original::needs_met(candidate.original_rules_model()->house_rules(),original::ModelHouse::PlainCottage,housing_house(candidate,target)),"Expired culture lacked actual other prerequisites or retained authored Need1");controls.push_back({{"role",static_cast<unsigned>(roles[index])},{"home_id",static_cast<unsigned>(target)},{"authored_model",true},{"original_plain_requirement",false},{"model_path",(private_data/"Model/EmperorBuildingModels.txt").string()},{"changed_field",5+index},{"prerequisites",prerequisites},{"delivered",delivered},{"expired",inspect(candidate)}});}
    json no_road=json::array();for(unsigned index=0;index<3;++index){auto denied=sim::World::create_original_production_development(policy,model);for(int x=0;x<24;++x)f::buy(denied,{sim::CommandType::PlaceRoad,{x,4}});sim::BuildingId target{};for(int x=0;x<6;x+=3){const auto id=buy(denied,sim::CommandType::PlaceHousehold,{x,2});if(!x)target=id;}(void)buy(denied,commands[index],{20,9});ticks(denied,600);check(denied.original_production_state()->services.trips.empty()&&!original::service_timer(housing_house(denied,target),roles[index]),"NoRoad cultural source granted actual access");no_road.push_back({{"role",static_cast<unsigned>(roles[index])},{"home_id",static_cast<unsigned>(target)},{"final",inspect(denied)}});}
    f::write_text(output/"fixture.json",json{{"status","PASS"},{"snapshot_injection",false},{"authored_geography",true},{"real_model_prices_and_workers",true},{"experimental_authored_targeted_school_visits",true},{"original_venue_performer_loop",false},{"free_venue_or_performer",false},{"home_id",static_cast<unsigned>(home)},{"culture_source_cost",180},{"culture_source_workers",27},{"combined_workers",100},{"paused_workers",73},{"initial",initial},{"stopped",stopped},{"midroute",midroute},{"disconnected",disconnected},{"visited",visited},{"before_expiry",before_expiry},{"expired",expired},{"revisited",revisited},{"expiry_ticks",expiries},{"expiry_tick",expiry},{"predicate_controls",controls},{"no_road_controls",no_road},{"checkpoints",checkpoints}}.dump(2)+"\n");
}
sim::BuildingId industry_buy(sim::World& w,sim::CommandType type,sim::Cell cell){f::buy(w,{type,cell});return *w.building_owner_at(cell);}
template<class P>void industry_wait(sim::World& w,P done,const char* why,int limit=30000){for(int i=0;i<limit&&!done();++i)w.tick();check(done(),why);}
const sim::OriginalIndustrySource& industry_source(const sim::World& w,sim::BuildingId id){for(const auto& row:w.original_production_state()->industry.sources)if(row.building_id==id)return row;throw std::runtime_error("Missing actual industry source");}
int industry_aux(const sim::World& w,sim::BuildingId id,unsigned material){for(const auto& row:w.original_production_state()->industry.auxiliary)if(row.building_id==id)return row.amounts[material];return 0;}
struct IndustryScene{sim::BuildingId first{},tea{},lacquer{},silkworm{},logging{},smelter{},clay{},bronze{},lacquerware{},paper{},weaver{},farm{},field{},warehouse{},other_warehouse{},food{},market{},academy{},well{},ancestor{};std::vector<sim::BuildingId> plants;};
IndustryScene industry_scene(sim::World& w){
    for(const int y:{4,7,11,16})for(int x=0;x<24;++x)f::buy(w,{sim::CommandType::PlaceRoad,{x,y}});
    for(const int y:{5,6,8,9,10,12,13,14,15})f::buy(w,{sim::CommandType::PlaceRoad,{23,y}});
    f::buy(w,{sim::CommandType::PlaceRoad,{14,12}});
    IndustryScene s;for(const int y:{1,5})for(int x=0;x<24;x+=3){const auto id=industry_buy(w,x==0&&y==1 ? sim::CommandType::PlaceOriginalElitePlot:sim::CommandType::PlaceHousehold,y==1 ? oe_production_fixture::elite_row_cell(x/3):sim::Cell{x,y});if(x==0&&y==1)s.first=id;}
    for(int x=0;x<12;x+=3)(void)industry_buy(w,sim::CommandType::PlaceHousehold,{x,9});
    s.tea=industry_buy(w,sim::CommandType::PlaceOriginalTeaCuringShed,{0,12});s.lacquer=industry_buy(w,sim::CommandType::PlaceOriginalLacquerRefinery,{4,12});s.silkworm=industry_buy(w,sim::CommandType::PlaceOriginalSilkwormShed,{8,12});
    for(const auto& [command,id,cell]:std::array{std::tuple{sim::CommandType::PlaceOriginalTeaBush,s.tea,sim::Cell{2,12}},std::tuple{sim::CommandType::PlaceOriginalLacquerTree,s.lacquer,sim::Cell{6,12}},std::tuple{sim::CommandType::PlaceOriginalMulberryTree,s.silkworm,sim::Cell{10,12}}}){sim::Command plant{command,cell};plant.building_id=id;f::buy(w,plant);s.plants.push_back(*w.building_owner_at(cell));}
    s.logging=industry_buy(w,sim::CommandType::PlaceOriginalLoggingShed,{12,12});s.smelter=industry_buy(w,sim::CommandType::PlaceOriginalBronzeSmelter,{16,12});s.clay=industry_buy(w,sim::CommandType::PlaceClaySource,{20,12});s.other_warehouse=industry_buy(w,sim::CommandType::PlaceWarehouse,{14,13});
    s.bronze=industry_buy(w,sim::CommandType::PlaceOriginalBronzewareMaker,{0,17});s.ancestor=industry_buy(w,sim::CommandType::PlaceOriginalAncestorShrine,{2,17});s.lacquerware=industry_buy(w,sim::CommandType::PlaceOriginalLacquerwareMaker,{4,17});s.paper=industry_buy(w,sim::CommandType::PlaceOriginalPaperMaker,{8,17});s.weaver=industry_buy(w,sim::CommandType::PlaceOriginalWeaver,{12,17});s.farm=industry_buy(w,sim::CommandType::PlaceOriginalHempFarm,{16,17});
    sim::Command field{sim::CommandType::PlaceOriginalHempField,{18,17}};field.building_id=s.farm;f::buy(w,field);s.field=*w.building_owner_at({18,17});s.warehouse=industry_buy(w,sim::CommandType::PlaceWarehouse,{20,17});
    s.food=industry_buy(w,sim::CommandType::PlaceOriginalFoodFarm,{12,8});field={sim::CommandType::PlaceOriginalWheatField,{15,8}};field.building_id=s.food;f::buy(w,field);s.market=industry_buy(w,sim::CommandType::PlaceMarket,{15,10});s.academy=industry_buy(w,sim::CommandType::PlaceOriginalConfucianAcademy,{18,8});s.well=industry_buy(w,sim::CommandType::PlaceOriginalWaterWell,{21,9});
    const bool real=w.original_rules_model()->house_rules().value(original::ModelHouse::Shelter,original::HouseField::Capacity)==7&&w.workforce_required(s.food)==22;
    check(w.buildings().size()==43&&w.couriers().size()==21&&w.snapshot().roads.size()==106&&!w.total_population(),"Paid industry scene geometry/actors/zero population differs");
    check(w.construction_spent_total()==static_cast<std::uint64_t>(real?3157:3186)&&w.treasury()==12000-static_cast<std::int64_t>(w.construction_spent_total()),"Real/authored industry construction bill differs");
    return s;
}
void industry_bootstrap(sim::World& w,const IndustryScene& s){int full_workers=0;for(const auto& b:w.buildings())full_workers+=w.workforce_required(b.id);for(const auto id:{s.paper,s.academy,s.food,s.ancestor})f::buy(w,sim::set_building_operation(id,false));
    industry_wait(w,[&]{return w.workforce_supply()>=full_workers;},"Paid Water/Hut bootstrap did not yield genuine workers");
    for(const auto id:{s.paper,s.academy,s.food,s.ancestor})f::buy(w,sim::set_building_operation(id,true));
}
void create_industry(const f::fs::path& data,const f::fs::path& output){
    const auto policy=f::permissions();const auto model=save::prepare_original_production_rules_model(data);
    auto world=sim::World::create_original_production_development(policy,model);const auto owners=industry_scene(world);const auto initial=inspect(world);
    industry_bootstrap(world,owners);f::buy(world,sim::set_building_operation(owners.academy,false));const auto staffed=inspect(world);
    check(world.workforce_supply()>=world.workforce_required(),"Actual luxury scene failed real Water/Hut workforce bootstrap");
    const auto moving_aux=[&]{for(const auto& row:world.original_production_state()->industry.aux_trips){const auto& c=world.courier(row.transfer.courier_id);if(row.loaded-row.delivered-row.returned>0&&c.edge_progress>0&&c.phase==sim::CourierPhase::ToWarehouse)return true;}return false;};
    industry_wait(world,moving_aux,"No actual auxiliary cart entered a begun edge");json checkpoints=json::array();const auto mid_aux=inspect(world);checkpoints.push_back(checkpoint(data,output,"mid-aux-courier",world));
    industry_wait(world,[&]{return academy_paper(world,owners.academy)>0;},"Real Hemp→Paper→Warehouse→paused Academy failed");const auto paper_arrived=inspect(world);checkpoints.push_back(checkpoint(data,output,"paper-academy-arrival",world));f::buy(world,sim::set_building_operation(owners.academy,true));
    industry_wait(world,[&]{for(const auto& c:world.couriers())if(c.role==sim::CourierRole::OriginalLuxuryToWarehouse&&world.original_courier_cargo(c.id)>0&&c.edge_progress>0)return true;return false;},"No four-good producer ever carried genuine luxury output");const auto mid_luxury=inspect(world);checkpoints.push_back(checkpoint(data,output,"mid-luxury-courier",world));
    sim::BuildingId served{};industry_wait(world,[&]{for(const auto& h:world.original_economy_state()->houses)if(h.house.tea&&h.house.silk&&h.house.bronzeware&&h.house.lacquerware){served=h.building_id;return true;}return false;},"Actual four luxury chains never supplied one House");const auto delivered=inspect(world);checkpoints.push_back(checkpoint(data,output,"house-four-luxuries",world));
    auto returning=sim::World::restore_original_production_development(world.snapshot(),policy,model);
    std::optional<sim::BuildingId> return_owner;
    industry_wait(returning,[&]{for(const auto& row:returning.original_production_state()->industry.aux_trips){const auto& c=returning.courier(row.transfer.courier_id);if(row.loaded-row.delivered-row.returned>0&&c.edge_progress>0&&c.phase==sim::CourierPhase::ToWarehouse){return_owner=row.source;return true;}}return false;},"No loaded auxiliary trip for genuine cancel/return");
    f::buy(returning,sim::set_building_operation(*return_owner,false));industry_wait(returning,[&]{for(const auto& row:returning.original_production_state()->industry.aux_trips)if(row.source==*return_owner&&row.phase==sim::OriginalGoodsTransferPhase::Returning&&row.loaded>row.returned)return true;return false;},"Paused material owner did not physically return loaded cargo");const auto return_state=inspect(returning);checkpoints.push_back(checkpoint(data,output,"industry-return",returning));
    auto disconnected=sim::World::restore_original_production_development(world.snapshot(),policy,model);const sim::Command cut{sim::CommandType::RemoveRoad,{23,14}};industry_wait(disconnected,[&]{return disconnected.validate(cut).accepted;},"Industry trunk did not become safe for ordinary removal");f::buy(disconnected,cut);ticks(disconnected,250);const auto disconnect_state=inspect(disconnected);checkpoints.push_back(checkpoint(data,output,"industry-road-disconnected",disconnected));const auto repair_before=disconnected.snapshot();f::buy(disconnected,{sim::CommandType::PlaceRoad,{23,14}});check(disconnected.treasury()==repair_before.treasury-2,"Industry trunk repair did not pay2");
    industry_wait(world,[&]{return industry_source(world,owners.bronze).progress==79&&world.building(owners.bronze).input_clay>0&&industry_aux(world,owners.bronze,1)>0;},"Actual recipe never reached both-input progress79");const auto recipe_ready=inspect(world);checkpoints.push_back(checkpoint(data,output,"industry-recipe-ready",world));
    const auto before=industry_source(world,owners.bronze);world.tick();const auto after=industry_source(world,owners.bronze);check(after.completed==before.completed+1&&after.clay_used==before.clay_used+1&&after.aux_inputs_used[1]==before.aux_inputs_used[1]+1,"Actual ready recipe did not atomically consume both inputs");
    json refusals=json::array();for(const auto id:{owners.tea,owners.plants.front(),owners.warehouse,owners.paper,owners.farm,owners.clay}){const auto before_demolition=world.snapshot();const auto result=world.execute(sim::demolish_building(id));check(!result.accepted&&world.snapshot()==before_demolition,"Production source/plant/receiver demolition changed authority");refusals.push_back(static_cast<unsigned>(id));}
    const auto neutral=industry_buy(world,sim::CommandType::PlaceOriginalInfluenceSource,{20,14});f::buy(world,sim::demolish_building(neutral));const auto final=inspect(world);
    f::write_text(output/"fixture.json",json{{"status","PASS"},{"snapshot_injection",false},{"authored_geography",true},{"real_model_prices_and_workers",true},
        {"original_resource_calendar_parity",false},{"managed_ore_authored",true},{"home_id",static_cast<unsigned>(served)},
        {"academy_id",static_cast<unsigned>(owners.academy)},{"paper_id",static_cast<unsigned>(owners.paper)},{"bronze_id",static_cast<unsigned>(owners.bronze)},
        {"initial",initial},{"staffed",staffed},{"mid_aux",mid_aux},{"paper_arrived",paper_arrived},{"mid_luxury",mid_luxury},{"delivered",delivered},
        {"return_state",return_state},{"disconnected",disconnect_state},{"recipe_ready",recipe_ready},{"final",final},{"demolition_refusals",refusals},
        {"neutral_well_demolished",true},{"checkpoints",checkpoints}}.dump(2)+"\n");
}
void create_stages(const f::fs::path& data,const f::fs::path& output) {
    const auto policy=f::permissions();const auto model=save::prepare_original_production_rules_model(data);
    auto world=sim::World::create_original_production_development(policy,model);
    const auto paid=oe_production_fixture::add_housing_stages_scene(world);
    // A genuine paid extension-cell road creates an actual failed120-tick
    // materialization opportunity, separately replayed by a new OS process.
    const sim::Cell growth_blocker{3,8};
    f::buy(world,{sim::CommandType::PlaceRoad,growth_blocker});
    const auto scene_spent=paid.spent+sim::Rules::road_cost;
    const auto initial=inspect(world);
    check(!world.total_population()&&housing_house(world,paid.elite).type==original::HouseType::EliteUnoccupied,
          "Paid Elite plot did not begin empty11");
    json checkpoints=json::array();checkpoints.push_back(checkpoint(data,output,"elite-empty",world));
    ticks(world,119);check(!world.total_population(),"Stages initial residents arrived before paid age120");
    const auto before_attempt=world.snapshot();world.tick();
    const auto& blocked_history=housing_history(world,paid.elite);
    const auto blocked_status=world.original_elite_materialization_status(paid.elite);
    const auto old_elite=std::find_if(before_attempt.original_economy->houses.begin(),before_attempt.original_economy->houses.end(),
        [&](const auto& value){return value.building_id==paid.elite;});
    check(old_elite!=before_attempt.original_economy->houses.end()&&world.ticks()==120&&
          world.building(paid.elite).kind==sim::Object::OriginalElitePlot&&housing_house(world,paid.elite)==old_elite->house,
          "Actual blocked attempt changed the existing Elite body or input");
    check(housing_house(world,paid.elite).type==original::HouseType::EliteUnoccupied&&
          !housing_house(world,paid.elite).occupants&&!blocked_history.first_admission_tick&&!blocked_history.last_evaluation&&
          std::none_of(blocked_history.first_transitions.begin(),blocked_history.first_transitions.end(),[](const auto& value){return bool(value);})&&
          blocked_status.pending&&!blocked_status.ready&&blocked_status.diagnostic.blocker==sim::BuildBlocker::SandboxOccupied&&
          world.command_sequence()==before_attempt.command_sequence&&world.road_revision()==before_attempt.road_revision&&
          world.treasury()==before_attempt.treasury&&world.construction_spent_total()==before_attempt.construction_spent_total&&
          world.snapshot().next_building_id==before_attempt.next_building_id,
          "Actual120-tick extension conflict published residents/history/geometry/finance/IDs");
    const auto blocked=inspect(world);checkpoints.push_back(checkpoint(data,output,"elite-growth-blocked",world));
    const auto before_repair=world.snapshot();f::buy(world,{sim::CommandType::RemoveRoad,growth_blocker});
    check(world.ticks()==before_repair.ticks&&world.treasury()==before_repair.treasury&&
          world.construction_spent_total()==before_repair.construction_spent_total&&
          world.command_sequence()==before_repair.command_sequence+1&&world.road_revision()==before_repair.road_revision+1&&
          world.original_elite_materialization_status(paid.elite).ready&&
          housing_house(world,paid.elite).type==original::HouseType::EliteUnoccupied,
          "Removing the genuinely paid blocker granted a refund/admission or failed to clear growth");
    const auto repaired=inspect(world);ticks(world,39);
    check(world.ticks()==159&&!housing_history(world,paid.elite).first_admission_tick&&
          !housing_house(world,paid.elite).occupants,"Repair bypassed the next ordinary admission opportunity");
    world.tick();const auto& admission=housing_history(world,paid.elite).first_transitions[10];
    check(admission&&admission->kind==sim::OriginalHousingTransitionKind::Admission&&admission->admitted==2&&
          admission->evaluation.tick==160&&admission->evaluation.input.type==original::HouseType::EliteUnoccupied&&
          !admission->evaluation.input.occupants&&housing_house(world,paid.elite).type==original::HouseType::ModestSiheyuan&&
          world.building(paid.elite).kind==sim::Object::OriginalEliteHouse,
          "Genuine next160-tick Elite event did not materialize4x4 and admit exactly2");
    const auto admitted=inspect(world);checkpoints.push_back(checkpoint(data,output,"elite-first-admission",world));
    const auto wait=[&](auto ready,const char* why,int limit=40000) {
        for (int i=0;i<limit&&!ready();++i) world.tick();
        check(ready(),why);
    };
    wait([&]{const auto& h=housing_house(world,paid.elite);return h.type==original::HouseType::ModestSiheyuan&&h.devolve_counter==49;},
         "Genuinely unstocked Modest never reached49 failed evaluations");
    const auto counter49=inspect(world);checkpoints.push_back(checkpoint(data,output,"elite-counter49",world));
    wait([&]{return housing_house(world,paid.elite).type==original::HouseType::EliteAbandoned;},
         "Actual50th Modest failure did not become Abandoned");
    const auto& devolution=housing_history(world,paid.elite).first_transitions[9];
    check(devolution&&devolution->kind==sim::OriginalHousingTransitionKind::Evolution&&
          devolution->evaluation.input.type==original::HouseType::ModestSiheyuan&&
          devolution->evaluation.input.devolve_counter==49,"Abandoned lacks actual50-event source input");
    const auto abandoned=inspect(world);checkpoints.push_back(checkpoint(data,output,"elite-abandoned",world));
    for (const auto id:paid.material_sources) f::buy(world,sim::set_building_operation(id,true));
    bool recovered=false;
    for (int i=0;i<40000&&!recovered;++i) {
        const auto previous=housing_house(world,paid.elite).type;world.tick();
        recovered=previous==original::HouseType::EliteAbandoned&&
                  housing_house(world,paid.elite).type==original::HouseType::ModestSiheyuan;
    }
    check(recovered,"Actual Hemp/Ceramics did not recover12->13");
    const auto& recovery=housing_history(world,paid.elite).last_evaluation;
    check(recovery&&recovery->tick==world.ticks()&&recovery->input.type==original::HouseType::EliteAbandoned&&
          recovery->input.hemp>0&&recovery->input.ceramics>0,"Elite recovery invented current/next-row goods");
    const auto recovered_state=inspect(world);checkpoints.push_back(checkpoint(data,output,"elite-recovered",world));
    sim::BuildingId advanced{};
    wait([&] {
        for (const auto id:paid.common_houses) {
            if (housing_house(world,id).type>original::HouseType::PlainCottage) {advanced=id;return true;}
        }
        return false;
    },"Actual paid Millet/Hemp/Water/Ancestor did not develop Common abovePlain");
    const auto advanced_state=inspect(world);checkpoints.push_back(checkpoint(data,output,"common-attractive",world));
    wait([&]{return housing_house(world,paid.elite).confucian>0;},"Actual paid Paper-backed Academy never visited Elite");
    for (const auto id:paid.common_houses) check(!housing_house(world,id).confucian,
        "Academy incorrectly visited Common during actual Elite scene");
    const auto visited=inspect(world);checkpoints.push_back(checkpoint(data,output,"elite-confucian-visit",world));
    const auto diagnosis=world.original_housing_diagnosis(advanced);
    const auto current=housing_house(world,advanced).type;
    const auto elite_diagnosis=world.original_housing_diagnosis(paid.elite);
    json blockers=json::array();
    blockers.push_back({{"house_id",static_cast<unsigned>(advanced)},{"actual_type",static_cast<unsigned>(current)},
        {"actual_desirability",diagnosis.cached_desirability},{"current_low",diagnosis.current.low},
        {"current_high",diagnosis.current.high},{"pure_next_type",static_cast<unsigned>(diagnosis.pure_result_type)},
        {"all_higher_common_stages_claimed_reached",false}});
    blockers.push_back({{"house_id",static_cast<unsigned>(paid.elite)},
        {"actual_type",static_cast<unsigned>(housing_house(world,paid.elite).type)},
        {"actual_desirability",elite_diagnosis.cached_desirability},{"current_high",elite_diagnosis.current.high},
        {"all_higher_elite_stages_claimed_reached",false},{"original_geometry_materialized",true}});
    f::write_text(output/"fixture.json",json{{"status","PASS"},{"snapshot_injection",false},
        {"real_model_housing_stages",true},{"authored_geography",true},{"empty_elite_plot_geometry",json::array({2,2})},{"materialized_elite_geometry",json::array({4,4})},
        {"original_elite_required_side",4},{"authored_elite_plot_price",80},
        {"home_id",static_cast<unsigned>(advanced)},{"elite_id",static_cast<unsigned>(paid.elite)},
        {"academy_id",static_cast<unsigned>(paid.academy)},{"scene_spent",scene_spent},
        {"initial",initial},{"blocked",blocked},{"repaired",repaired},{"admitted",admitted},{"counter49",counter49},{"abandoned",abandoned},
        {"recovered",recovered_state},{"advanced",advanced_state},{"visited",visited},
        {"higher_stage_observations",blockers},{"paid_city_heavenly_reached",false},
        {"checkpoints",checkpoints}}.dump(2)+"\n");
}
void resume(const f::fs::path& data,const f::fs::path& output,const f::fs::path& input) {
    const auto before=f::read_text(input);const auto document=save::read_save(input);const auto policy=f::permissions();
    auto world=save::restore_original_production_development(document,data,policy->building_mask(),policy);
    check(world.snapshot()==document.world,"Separate OS process changed full production restore");
    continuation(data,output,"",world);check(f::read_text(input)==before,"Replay modified input save");
}
}
int main(int argc,char** argv) {
    try {
        const std::string mode=argc>1 ? argv[1]:"";
        check((argc==4 && (mode=="--create" || mode=="--create-consumption" || mode=="--create-food" || mode=="--create-services" || mode=="--create-ancestor" || mode=="--create-housing" || mode=="--create-religion" || mode=="--create-culture" || mode=="--create-industry" || mode=="--create-stages")) || (argc==5 && mode=="--resume"),
              "Usage: production-fixture --create DATA OUT | --create-consumption AUTHORED_DATA OUT | --create-food DATA OUT | --create-services DATA OUT | --create-ancestor DATA OUT | --create-housing DATA OUT | --create-religion DATA OUT | --create-culture DATA OUT | --create-industry DATA OUT | --resume DATA OUT SAVE");
        const auto data=f::fs::canonical(argv[2]);const auto output=f::fs::absolute(argv[3]);
        validate_output(output,data);f::fs::create_directories(output);
        if (mode=="--create") create(data,output);
        else if (mode=="--create-consumption") create_consumption(data,output);
        else if (mode=="--create-food") create_food(data,output);
        else if (mode=="--create-services") create_services(data,output);
        else if (mode=="--create-ancestor") create_ancestor(data,output);
        else if (mode=="--create-housing") create_housing(data,output);
        else if (mode=="--create-religion") create_religion(data,output);
        else if (mode=="--create-culture") create_culture(data,output);
        else if (mode=="--create-industry") create_industry(data,output);
        else if (mode=="--create-stages") create_stages(data,output);
        else resume(data,output,f::fs::canonical(argv[4]));
        std::cout<<"Original paid production fixture PASS\n";return 0;
    } catch (const std::exception& error) {std::cerr<<"Original production fixture FAIL: "<<error.what()<<'\n';return 1;}
}
