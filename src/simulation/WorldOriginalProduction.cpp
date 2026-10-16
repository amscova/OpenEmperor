#include "simulation/World.h"
#include "simulation/OriginalGoodsLedger.h"
#include "simulation/original/OriginalRulesModel.h"
#include "simulation/original/OriginalScheduler.h"

#include <algorithm>
#include <array>
#include <limits>
#include <stdexcept>
#include <utility>

namespace openemperor::simulation {
namespace {
constexpr auto ceramics=static_cast<std::size_t>(original::Goods::Ceramics);
[[noreturn]] void invalid(const char* message) {throw std::invalid_argument(message);}
struct GoodsContext {
    std::array<OriginalGoodsOwner,original_goods_owner_limit> owners{};
    std::array<CourierId,6> couriers{};
    std::size_t count=0;
    std::size_t courier_count=0;
    template<class Buildings,class Couriers> GoodsContext(const Buildings& buildings,const Couriers& actors) {
        for (const auto& b:buildings) if (b.kind==Object::Household || b.kind==Object::Warehouse || b.kind==Object::Market) {
            if (count==owners.size()) invalid("original production goods owner limit exceeded");
            auto& owner=owners[count++];owner.building_id=b.id;owner.household=b.kind==Object::Household;
            owner.capacity[ceramics]=owner.household ? Rules::household_capacity:
                b.kind==Object::Market ? Rules::market_pottery_capacity:Rules::warehouse_capacity;
        }
        for (const auto& c:actors) if (c.role==CourierRole::MarketPotteryInbound || c.role==CourierRole::MarketPotteryDistribution) {
            if (courier_count==couriers.size()) invalid("typed market courier limit exceeded");
            couriers[courier_count++]=c.id;
        }
    }
    std::span<const OriginalGoodsOwner> view() const {return {owners.data(),count};}
    std::span<const CourierId> courier_view() const {return {couriers.data(),courier_count};}
};
std::uint64_t paid_cost(Object kind) {
    switch (kind) {
    case Object::Household:return Rules::household_cost;
    case Object::ClaySource:return Rules::clay_source_cost;
    case Object::Pottery:return Rules::pottery_cost;
    case Object::Warehouse:return Rules::warehouse_cost;
    case Object::Market:return Rules::market_cost;
    case Object::OriginalInfluenceSource:return original_influence_source_cost;
    default:invalid("unsupported original production building kind");
    }
}
template<class Buildings,class Bridges> std::uint64_t spending(const Buildings& buildings,
    const Bridges& bridges,std::size_t ramps,std::uint64_t roads,std::size_t removed) {
    const auto budget=static_cast<std::uint64_t>(original_production_starting_treasury);
    std::uint64_t result=0;
    const auto add=[&](std::uint64_t count,std::uint64_t cost) {
        if (count>budget/cost || result>budget-count*cost) invalid("original production spending exceeds initial funds");
        result+=count*cost;
    };
    add(roads,Rules::road_cost);add(ramps,ramp_cost);add(removed,original_influence_source_cost);
    for (const auto& b:buildings) add(1,paid_cost(b.kind));
    for (const auto& bridge:bridges) {
        if (bridge.paid_cost<=0) invalid("invalid original production bridge price");
        add(1,static_cast<std::uint64_t>(bridge.paid_cost));
    }
    return result;
}
template<class Building> void validate_neutral_fields(const Building& b) {
    BuildingSnapshot expected{b.id,b.kind,b.cell,true};expected.placed_tick=b.placed_tick;
    if (b.kind!=Object::OriginalInfluenceSource) {
        expected.fire_risk=b.fire_risk;expected.fire_protection_until_tick=b.fire_protection_until_tick;
        expected.fire_until_tick=b.fire_until_tick;
    }
    if (b.kind!=Object::Household && b.kind!=Object::OriginalInfluenceSource) {
        expected.operating_enabled=b.operating_enabled;expected.workforce_priority=b.workforce_priority;
    }
    if (b.kind==Object::ClaySource) {
        expected.output=b.output;expected.progress=b.progress;expected.clay_extracted=b.clay_extracted;
    } else if (b.kind==Object::Pottery) {
        expected.input_clay=b.input_clay;expected.output=b.output;expected.reserved_incoming=b.reserved_incoming;
        expected.progress=b.progress;expected.active_recipe_clay=b.active_recipe_clay;
        expected.recipes_completed=b.recipes_completed;
    } else if (b.kind==Object::Warehouse) expected.reserved_incoming=b.reserved_incoming;
    if (b!=expected) invalid("unsupported original production building authority");
}
}

World World::create_original_production_development(std::shared_ptr<const MapPermissions> permissions,
    std::shared_ptr<const original::OriginalRulesModel> model) {
    if (!permissions || permissions->policy_version()!=kBridgesMapPermissionsPolicyVersion || !model)
        invalid("original production development requires policy3 and a prepared model");
    const auto capacity=model->house_rules().row_capacity(original::ModelHouse::Shelter);
    if (capacity<=0 || capacity>INT16_MAX) invalid("original production requires a positive signed-word Shelter capacity");
    const auto width=permissions->width(),height=permissions->height();auto mask=permissions->building_mask();
    World world(width,height,std::move(mask),RulesProfile::CityV16,kOriginalProductionRuleVersion,
        std::move(permissions),true,std::move(model));
    world.rebuild_original_influence_cache();return world;
}

World World::restore_original_production_development(const WorldSnapshot& snapshot,
    std::shared_ptr<const MapPermissions> permissions,
    std::shared_ptr<const original::OriginalRulesModel> model) {
    if (!simulation::original_production_enabled(snapshot.profile,snapshot.rule_version) ||
        !permissions || permissions->policy_version()!=kBridgesMapPermissionsPolicyVersion || !model)
        invalid("original production restore requires explicit rule10, policy3 and prepared model");
    const auto capacity=model->house_rules().row_capacity(original::ModelHouse::Shelter);
    if (capacity<=0 || capacity>INT16_MAX) invalid("invalid original production Shelter capacity");
    validate_original_production_snapshot(snapshot);
    return restore_impl(snapshot,{},std::move(permissions),true,std::move(model));
}

void World::validate_original_production_snapshot(const WorldSnapshot& s) {
    if (!simulation::original_production_enabled(s.profile,s.rule_version)) {
        if (s.original_production) invalid("original production requires City-v16 rule10");
        return;
    }
    if (!s.original_production || s.original_production->development_revision!=kOriginalProductionDevelopmentRevision ||
        !s.original_economy || !s.original_economy->model_source || !s.original_influence || !s.original_goods ||
        s.width<1 || s.height<1 || s.width>228 || s.height>228)
        invalid("invalid original production development revision or required authority");
    const auto& source=*s.original_economy->model_source;
    if (source.format_version!=1 || source.sha256.size()!=64 || !source.byte_count ||
        source.byte_count>original::max_model_bytes || static_cast<std::size_t>(source.difficulty)>=original::difficulty_count ||
        !std::all_of(source.sha256.begin(),source.sha256.end(),[](char c){return (c>='0' && c<='9') || (c>='a' && c<='f');}))
        invalid("invalid original production model source identity");
    if (s.ticks>original::scheduler_max_tick || s.workshop || s.warehouse || s.total_produced ||
        s.workshop_stock || s.production_progress || s.courier_cargo || s.warehouse_stock ||
        s.phase!=CourierPhase::IdleAtWorkshop || !s.path.empty() || s.path_vertex || s.edge_progress ||
        s.next_household_id!=first_household_id || s.next_production_id!=8 ||
        s.last_dispatched_household || s.last_dispatched_food_household || s.last_dispatched_service_household ||
        s.food_produced_total || s.taxes_collected_total || s.maintenance_spent_total ||
        s.demolition_history!=DemolitionHistory{} || s.buildings.size()>Rules::city_v15_building_limit ||
        s.original_economy->houses.size()>Rules::city_v10_household_limit ||
        s.original_influence->sources.size()>original_influence_source_limit ||
        s.original_influence->removed_sources.size()>original_influence_purchase_limit)
        invalid("unsupported original production runtime authority");
    std::array<bool,Rules::city_v15_building_limit+original_influence_purchase_limit+1> seen{};
    const auto mark=[&](BuildingId id) {
        const auto value=static_cast<std::uint32_t>(id);
        if (!value || value>=s.next_building_id || value>=seen.size() || seen[value])
            invalid("original production purchase identities must be disjoint and contiguous");
        seen[value]=true;
    };
    std::size_t house_index=0,warehouse_index=0,influence_index=0,courier_index=0;
    std::uint64_t residents=0;
    for (std::size_t i=0;i<s.buildings.size();++i) {
        const auto& b=s.buildings[i];mark(b.id);
        if ((i && !(s.buildings[i-1].id<b.id)) || b.placed_tick>s.ticks)
            invalid("original production buildings must be sorted with valid placement time");
        (void)paid_cost(b.kind);validate_neutral_fields(b);
        if (b.kind==Object::ClaySource || b.kind==Object::Pottery || b.kind==Object::Warehouse || b.kind==Object::Market) {
            if (courier_index>=s.couriers.size()) invalid("missing purchased original production courier");
            const auto& c=s.couriers[courier_index];
            const auto role=b.kind==Object::ClaySource ? CourierRole::Clay:
                b.kind==Object::Pottery ? CourierRole::Pottery:b.kind==Object::Market ?
                CourierRole::MarketPotteryDistribution:CourierRole::MarketPotteryInbound;
            if (static_cast<std::uint32_t>(c.id)!=courier_index+1 || c.owner!=b.id || c.role!=role)
                invalid("original production courier IDs must follow purchased owners");
            ++courier_index;
        }
        const auto age=s.ticks-b.placed_tick;
        if (b.kind==Object::Household) {
            if (house_index>=s.original_economy->houses.size()) invalid("missing original production house");
            const auto& h=s.original_economy->houses[house_index++];
            const auto occupants=h.house.occupants;
            const auto maximum=age<120 ? 0:std::min<std::uint64_t>(INT16_MAX,2*std::min<std::uint64_t>(INT16_MAX,(age-120)/40+1));
            if (h.building_id!=b.id || occupants<0 || static_cast<std::uint64_t>(occupants)>maximum)
                invalid("original residents exceed paid house immigration history");
            OriginalHouseState expected{b.id};
            if (occupants) {
                expected.house.type=original::HouseType::Shelter;expected.house.occupants=occupants;
                expected.house.ceramics=h.house.ceramics;
                expected.original_status=3;expected.access_enabled=true;
            }
            if (h!=expected) invalid("unsupported original production house needs, goods or evolution");
            residents+=static_cast<std::uint64_t>(occupants);
        } else if (b.kind==Object::Warehouse || b.kind==Object::Market) {
            if (warehouse_index>=s.original_goods->stocks.size() ||
                s.original_goods->stocks[warehouse_index++].building_id!=b.id)
                invalid("original production requires exactly one typed stock per Warehouse");
        } else if (b.kind==Object::OriginalInfluenceSource) {
            if (influence_index>=s.original_influence->sources.size()) invalid("missing original influence source");
            const auto& typed=s.original_influence->sources[influence_index++];
            if (typed.building_id!=b.id || typed.type!=OriginalInfluenceSourceType::Well72)
                invalid("invalid original production influence identity");
        } else if (b.kind==Object::ClaySource) {
            if (b.clay_extracted>age/32) invalid("Clay history exceeds actual recipe clock");
        } else if (b.recipes_completed>age/65) invalid("Pottery history exceeds actual recipe clock");
    }
    if (house_index!=s.original_economy->houses.size() || warehouse_index!=s.original_goods->stocks.size() ||
        influence_index!=s.original_influence->sources.size() || residents!=s.original_production->residents_admitted ||
        courier_index!=s.couriers.size() || s.next_courier_id!=courier_index+1)
        invalid("orphan original production house, stock, source or resident history");
    const auto& removed=s.original_influence->removed_sources;
    for (std::size_t i=0;i<removed.size();++i) {
        const auto& entry=removed[i];mark(entry.building_id);
        if ((i && !(removed[i-1].building_id<entry.building_id)) || entry.type!=OriginalInfluenceSourceType::Well72 ||
            entry.placed_tick>entry.removed_tick || entry.removed_tick>s.ticks)
            invalid("invalid original production source removal history");
    }
    if (s.next_building_id!=s.buildings.size()+removed.size()+1)
        invalid("invalid original production next building identity");
    for (std::uint32_t id=1;id<s.next_building_id;++id) if (!seen[id]) invalid("missing paid original production purchase");
    const auto spent=spending(s.buildings,s.bridges,s.ramps.size(),s.roads_placed_total,removed.size());
    if (spent!=s.construction_spent_total || s.treasury!=original_production_starting_treasury-static_cast<std::int64_t>(spent) ||
        s.roads_removed_total>s.roads_placed_total || s.roads_placed_total-s.roads_removed_total!=s.roads.size() ||
        s.road_revision!=s.roads_placed_total+s.roads_removed_total+s.buildings.size()+2*removed.size()+s.bridges.size()+s.ramps.size())
        invalid("original production finances or placement revision mismatch");
    for (std::size_t g=0;g<original::goods_count;++g) {
        const auto& totals=s.original_goods->totals[g];
        if (totals.imported || totals.consumed || totals.demolished ||
            totals.produced!=(g==ceramics ? s.original_production->pottery_handed_off:0))
            invalid("unsupported original production goods origin");
    }
    if (s.original_production->pottery_handed_off>s.pottery_completed_total)
        invalid("Ceramics handoff exceeds actual completed Pottery");
    const GoodsContext owners(s.buildings,s.couriers);
    validate_original_goods_ledger({*s.original_goods,s.original_economy->houses,owners.view(),owners.courier_view()});
    validate_original_market_snapshot(s);
    for (const auto& c:s.couriers) if (c.role!=CourierRole::Clay && c.role!=CourierRole::Pottery &&
        c.role!=CourierRole::MarketPotteryInbound && c.role!=CourierRole::MarketPotteryDistribution)
        invalid("unsupported original production courier role");
}

bool World::original_production_finance_valid() const {
    if (!original_production_ || !original_influence_ || !original_economy_ || !original_goods_ ||
        !original_rules_model_ || taxes_collected_total_ || maintenance_spent_total_ || demolition_history_!=DemolitionHistory{}) return false;
    try {
        const auto spent=spending(buildings_,bridges_,ramps_.size(),roads_placed_total_,original_influence_->removed_sources.size());
        return construction_spent_total_==spent && treasury_==original_production_starting_treasury-static_cast<std::int64_t>(spent);
    } catch (const std::invalid_argument&) {return false;}
}

std::array<OriginalGoodsBalance,original::goods_count> World::original_production_goods_balances() const {
    if (!original_goods_ || !original_economy_) throw std::logic_error("missing original production goods authority");
    const GoodsContext owners(buildings_,couriers_);
    validate_original_market_state();
    auto balances=inspect_original_goods_ledger({*original_goods_,original_economy_->houses,owners.view(),owners.courier_view()});
    balances[ceramics].supported=true;return balances;
}

int World::original_ceramics_stock(BuildingId owner) const {
    if (building(owner).kind==Object::Household) {
        const auto& homes=original_economy_->houses;
        const auto found=std::lower_bound(homes.begin(),homes.end(),owner,
            [](const OriginalHouseState& home,BuildingId id){return home.building_id<id;});
        if (found==homes.end() || found->building_id!=owner) throw std::logic_error("missing original House stock");
        return found->house.ceramics;
    }
    const auto& stocks=original_goods_->stocks;
    const auto found=std::lower_bound(stocks.begin(),stocks.end(),owner,
        [](const OriginalGoodsStock& row,BuildingId id) {return row.building_id<id;});
    if (found==stocks.end() || found->building_id!=owner) throw std::logic_error("missing typed Warehouse stock authority");
    return found->amounts[ceramics];
}

void World::receive_original_ceramics(BuildingId warehouse,int amount) {
    if (amount<=0 || static_cast<std::uint64_t>(amount)>UINT64_MAX-original_production_->pottery_handed_off)
        throw std::overflow_error("original Ceramics handoff exhausted");
    const GoodsContext owners(buildings_,couriers_);
    auto candidate=original_goods_produce({*original_goods_,original_economy_->houses,owners.view(),owners.courier_view()},warehouse,
        original::Goods::Ceramics,amount);
    // All arithmetic, capacity and conservation checks precede the handoff.
    // HouseState is unchanged by the producer; only the typed Warehouse owns it.
    original_goods_=std::move(candidate.goods);
    original_production_->pottery_handed_off+=static_cast<std::uint64_t>(amount);
}

void World::admit_original_residents() {
    const auto capacity=original_rules_model_->house_rules().row_capacity(original::ModelHouse::Shelter);
    for (auto& home:original_economy_->houses) {
        const auto& b=building(home.building_id);
        const auto age=ticks_-b.placed_tick;
        if (age<120 || (age-120)%40 || home.house.occupants>=capacity || building_on_fire(b.id) ||
            building_entrances(b.id).empty()) continue;
        const auto admitted=std::min(2,capacity-home.house.occupants);
        home.house.occupants=static_cast<std::int16_t>(home.house.occupants+admitted);
        home.house.type=original::HouseType::Shelter;home.original_status=3;home.access_enabled=true;
        original_production_->residents_admitted+=static_cast<std::uint64_t>(admitted);
    }
}

void World::tick_original_production() {
    const auto next=original::advance_scheduler_tick(ticks_);
    if (next>UINT64_MAX-Rules::fire_incident_ticks) throw std::overflow_error("original production Fire clock exhausted");
    capture_tick_staffing();ticks_=next;
    // Staffing belongs to the pre-tick population. Immigration cannot staff a
    // producer until the next ordinary tick. No 400t legacy demand is run.
    advance_clay_pottery_production();
    for (auto& c:couriers_) dispatch_v2(c);
    for (auto& c:couriers_) move_v2(c);
    update_fire();admit_original_residents();tick_staffing_active_=false;
    const auto balances=original_production_goods_balances();
    if (!population_valid() || !fire_state_valid() || !production_balance_valid() || !navigation_valid() ||
        !city_economy_valid() || !std::all_of(balances.begin(),balances.end(),[](const auto& b){return b.conserved;}))
        throw std::logic_error("original production, transport, population or finance invariant violated");
}

} // namespace openemperor::simulation
