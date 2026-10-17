#include "simulation/World.h"
#include "simulation/original/OriginalRulesModel.h"

#include <algorithm>
#include <array>
#include <stdexcept>
#include <utility>

namespace openemperor::simulation {
namespace {
[[noreturn]] void invalid(const char* reason) {throw std::invalid_argument(reason);}
template<class Rows> decltype(auto) body(Rows& rows,BuildingId id) {
    const auto at=std::lower_bound(rows.begin(),rows.end(),id,[](const auto& row,BuildingId key){return row.id<key;});
    if (at==rows.end() || at->id!=id) invalid("original water refers to a missing paid body");
    return *at;
}
template<class Rows> decltype(auto) home(Rows& rows,BuildingId id) {
    const auto at=std::lower_bound(rows.begin(),rows.end(),id,[](const auto& row,BuildingId key){return row.building_id<key;});
    if (at==rows.end() || at->building_id!=id) invalid("original water refers to a missing paid House");
    return *at;
}
const OriginalWaterProvider& provider(const OriginalWaterState& state,BuildingId id) {
    return home(state.providers,id);
}
template<class Rows> auto latest_trip(Rows& rows,CourierId id) {
    const auto at=std::lower_bound(rows.begin(),rows.end(),id,[](const auto& row,CourierId key){return row.transfer.courier_id<key;});
    return at==rows.end() || at->transfer.courier_id!=id ? nullptr:&*at;
}
bool accepts(const OriginalHouseState& house) {
    return (house.original_status==1 || house.original_status==3) && house.access_enabled && house.house.occupants>0;
}
bool evaluates(const OriginalHouseState& house) {
    return (house.original_status==1 || house.original_status==3) && house.access_enabled;
}
auto order(const OriginalWaterWitness& witness) {return std::pair{witness.arrival_tick,witness.transfer.courier_id};}
template<class Building> std::uint64_t minimum_travel(const Building& source,const Building& target) {
    const auto gap=[](int a,int b) {
        const auto delta=static_cast<std::int64_t>(a)-static_cast<std::int64_t>(b);
        return std::max<std::int64_t>(0,(delta<0 ? -delta:delta)-1);
    };
    const auto edges=static_cast<std::uint64_t>(std::max<std::int64_t>(2,
        gap(source.cell.x,target.cell.x)+gap(source.cell.y,target.cell.y)));
    return edges*static_cast<std::uint64_t>(profile_rules(RulesProfile::CityV16,kOriginalProductionRuleVersion).courier_edge_ticks);
}
template<class Building,class Courier> std::uint64_t minimum_movement(const Building& start,const Courier& actor) {
    const auto edge_ticks=profile_rules(RulesProfile::CityV16,kOriginalProductionRuleVersion).courier_edge_ticks;
    if (actor.path_vertex>=actor.path.size() || actor.edge_progress<0 || actor.edge_progress>=edge_ticks)
        invalid("original water movement has no physical vertex/progress");
    const auto current=actor.path[actor.path_vertex];
    const auto gap=[](int origin,int coordinate) {
        const auto delta=static_cast<std::int64_t>(coordinate)-static_cast<std::int64_t>(origin);
        return delta<0 ? -delta:std::max<std::int64_t>(0,delta-1);
    };
    const auto edges=static_cast<std::uint64_t>(gap(start.cell.x,current.x)+gap(start.cell.y,current.y));
    return edges*static_cast<std::uint64_t>(edge_ticks)+static_cast<std::uint64_t>(actor.edge_progress);
}
template<class Buildings,class Couriers,class Houses>
void validate_witness(const OriginalWaterWitness& witness,const OriginalWaterState& state,
                      const Buildings& buildings,const Couriers& couriers,const Houses& houses,const OriginalHousingState& housing,std::uint64_t tick) {
    const auto& p=provider(state,witness.provider_id);const auto& source=body(buildings,p.building_id);
    const auto& target=body(buildings,witness.building_id);const auto& h=home(houses,witness.building_id);
    const auto& admitted=home(housing.houses,witness.building_id).first_admission_tick;
    const auto actor=std::find_if(couriers.begin(),couriers.end(),[&](const auto& c){return c.id==witness.transfer.courier_id;});
    const auto* trip=latest_trip(state.trips,witness.transfer.courier_id);
    if (source.kind!=Object::OriginalWaterWell72 || target.kind!=Object::Household || !accepts(h) ||
        actor==couriers.end() || actor->owner!=p.building_id || actor->role!=CourierRole::OriginalWater ||
        !trip || !witness.transfer.sequence || witness.transfer.sequence>trip->transfer.sequence ||
        !witness.arrival_tick || witness.arrival_tick>tick || witness.transfer.sequence>witness.arrival_tick ||
        witness.arrival_tick<=source.placed_tick || witness.arrival_tick<target.placed_tick ||
        !admitted || witness.arrival_tick<*admitted)
        invalid("original water Witness has no possible paid occupied-House arrival");
    const auto travel=minimum_travel(source,target);
    if (witness.arrival_tick-source.placed_tick<travel || witness.arrival_tick-target.placed_tick<travel+120 || witness.arrival_tick-*admitted<travel)
        invalid("original water Witness predates possible travel after purchase and immigration");
    if (trip->transfer==witness.transfer) {
        if (!trip->visit_applied || trip->target!=witness.building_id || trip->arrival_tick!=witness.arrival_tick)
            invalid("original water Witness disagrees with its retained actual arrival");
    } else if (witness.arrival_tick>=trip->dispatched_tick)
        invalid("compacted water Witness cannot postdate its Courier's later dispatch");
}
template<class Buildings,class Couriers,class Houses>
void validate_evaluation(const OriginalWaterState& state,const Buildings& buildings,const Couriers& couriers,
                         const Houses& houses,const OriginalHousingState& housing,std::uint64_t tick,BuildingId id,std::uint64_t observed,
                         const std::optional<OriginalWaterWitness>& captured,bool supplied) {
    if (observed>tick) invalid("water evaluation cannot observe a future clock");
    const auto& b=body(buildings,id);const auto& row=home(state.houses,id);
    if (b.kind!=Object::Household || observed<b.placed_tick) invalid("water evaluation predates its paid House");
    const auto evaluated=original_water_evaluation_tick(observed);
    if (captured) {
        if (captured->building_id!=id || !evaluated || captured->arrival_tick>evaluated)
            invalid("water evaluation Witness was unavailable at its Phase28");
        validate_witness(*captured,state,buildings,couriers,houses,housing,tick);
        if (!row.current_witness || order(*row.current_witness)<order(*captured) ||
            (order(*row.current_witness)==order(*captured) && row.current_witness!=captured))
            invalid("historical water evaluation is newer than its actual latest arrival");
    }
    const auto known=[&](const OriginalWaterWitness& witness) {
        if (witness.building_id!=id || witness.arrival_tick>evaluated) return;
        if (!captured || order(*captured)<order(witness) || (order(*captured)==order(witness) && *captured!=witness))
            invalid("water evaluation discarded a known later available arrival");
    };
    if (row.current_witness) known(*row.current_witness);
    if (row.evaluation_witness) known(*row.evaluation_witness);
    for (const auto& trip:state.trips) if (trip.visit_applied && trip.target==id && trip.arrival_tick) {
        const auto actor=std::find_if(couriers.begin(),couriers.end(),[&](const auto& c){return c.id==trip.transfer.courier_id;});
        if (actor==couriers.end()) invalid("water evaluation lost its retained actual Courier");
        known({id,actor->owner,trip.transfer,*trip.arrival_tick});
    }
    const bool expected=captured && original_water_timer_at(*captured,evaluated)>0;
    if (supplied!=expected) invalid("materialized water disagrees with its actual Phase28 input");
}
template<class Buildings,class Couriers,class Houses>
void validate_water(const OriginalWaterState& state,const Buildings& buildings,const Couriers& couriers,const Houses& houses,
                    const OriginalHousingState& housing,std::uint64_t tick,std::uint64_t decayed_through,std::uint64_t evaluated_through,bool bind_movement=true) {
    if (state.providers.size()>original_water_provider_limit || state.trips.size()>original_water_trip_limit ||
        state.houses.size()>original_water_house_limit || state.houses.size()!=houses.size() ||
        decayed_through>tick || evaluated_through>tick)
        invalid("original water bounded authority exceeded");
    const auto sources=std::count_if(buildings.begin(),buildings.end(),[](const auto& b){return b.kind==Object::OriginalWaterWell72;});
    const auto actors=std::count_if(couriers.begin(),couriers.end(),[](const auto& c){return c.role==CourierRole::OriginalWater;});
    if (static_cast<std::size_t>(sources)!=state.providers.size() || actors!=sources)
        invalid("original water paid Provider/Courier bindings are incomplete");
    for (std::size_t i=0;i<state.providers.size();++i) {
        const auto& p=state.providers[i];const auto& b=body(buildings,p.building_id);
        if ((i && !(state.providers[i-1].building_id<p.building_id)) || b.kind!=Object::OriginalWaterWell72 ||
            !b.placed || b.placed_tick>tick || std::count_if(couriers.begin(),couriers.end(),[&](const auto& c){
                return c.owner==b.id && c.role==CourierRole::OriginalWater;})!=1)
            invalid("original water Providers require unique matching paid bodies and actors");
    }
    for (std::size_t i=0;i<state.trips.size();++i) {
        const auto& trip=state.trips[i];
        if ((i && !(state.trips[i-1].transfer.courier_id<trip.transfer.courier_id)) || !trip.transfer.sequence ||
            !trip.dispatched_tick || trip.transfer.sequence>trip.dispatched_tick || trip.dispatched_tick>tick)
            invalid("original water trips require sorted bounded actual dispatch sequences");
        const auto actor=std::find_if(couriers.begin(),couriers.end(),[&](const auto& c){return c.id==trip.transfer.courier_id;});
        if (actor==couriers.end() || actor->role!=CourierRole::OriginalWater) invalid("water trip has no actual Water Courier");
        const auto& source=body(buildings,provider(state,actor->owner).building_id);
        const auto& target=body(buildings,trip.target);const auto& h=home(houses,trip.target);
        const auto& admitted=home(housing.houses,trip.target).first_admission_tick;
        if (target.kind!=Object::Household || !accepts(h) || trip.dispatched_tick<=source.placed_tick ||
            trip.dispatched_tick<target.placed_tick || trip.dispatched_tick-target.placed_tick<121 ||
            !admitted || trip.dispatched_tick<=*admitted)
            invalid("water dispatch predates or mismatches its paid occupied target");
        if (bind_movement && (actor->target!=trip.target || actor->last_dispatched_target!=trip.target))
            invalid("water trip lost its fixed actual target");
        if (trip.arrival_tick) {
            if (*trip.arrival_tick<=trip.dispatched_tick || *trip.arrival_tick>tick ||
                *trip.arrival_tick-trip.dispatched_tick<minimum_travel(source,target)-1)
                invalid("water arrival predates its possible actual trip");
            if (bind_movement) {
                if (actor->phase!=CourierPhase::Returning && actor->phase!=CourierPhase::IdleAtWorkshop)
                    invalid("arrived Water Courier must return or be home");
                const auto required=actor->phase==CourierPhase::IdleAtWorkshop ? minimum_travel(source,target):minimum_movement(target,*actor);
                if (tick-*trip.arrival_tick<required) invalid("water return movement predates its actual arrival");
            }
        } else if (trip.visit_applied || (bind_movement && actor->phase!=CourierPhase::ToWarehouse))
            invalid("water visit exists without an actual outbound arrival");
        else if (bind_movement && tick-trip.dispatched_tick+1<minimum_movement(source,*actor))
            invalid("water outbound movement predates its actual dispatch");
    }
    for (const auto& c:couriers) if (c.role==CourierRole::OriginalWater) {
        (void)provider(state,c.owner);
        if (c.good!=Good::Goods || c.cargo || c.reserved || !c.enabled) invalid("Water Courier cannot carry material cargo");
        if (!latest_trip(state.trips,c.id) && (c.phase!=CourierPhase::IdleAtWorkshop || c.last_dispatched_target || c.target!=c.owner))
            invalid("used Water Courier lost its latest trip and sequence highwater");
    }
    for (std::size_t i=0;i<state.houses.size();++i) {
        const auto& row=state.houses[i];
        if (row.building_id!=houses[i].building_id || (i && !(state.houses[i-1].building_id<row.building_id)) ||
            row.normal_timer>96 || row.special_timer)
            invalid("water requires one sorted bounded normal-only row per paid House");
        std::uint8_t expected=0;
        if (row.current_witness) {
            if (row.current_witness->building_id!=row.building_id) invalid("water current Witness changed House");
            validate_witness(*row.current_witness,state,buildings,couriers,houses,housing,tick);
            const auto before=original_water_countdowns(row.current_witness->arrival_tick-1);
            const auto after=original_water_countdowns(decayed_through);
            if (after<before) invalid("water countdown predates its actual arrival");
            const auto elapsed=after-before;expected=elapsed>=96 ? 0:static_cast<std::uint8_t>(96-elapsed);
        }
        if (row.normal_timer!=expected) invalid("water raw timer lost its actual arrival and Phase35 origin");
        // A just-created House after the last evaluation has no observed
        // Phase28 yet; its empty row and false materialization are mandatory.
        const auto& b=body(buildings,row.building_id);
        if (evaluated_through<b.placed_tick) {
            if (row.evaluation_witness || houses[i].house.water) invalid("new House acquired retroactive evaluated water");
        } else validate_evaluation(state,buildings,couriers,houses,housing,tick,row.building_id,evaluated_through,
                                   row.evaluation_witness,houses[i].house.water);
    }
    for (const auto& trip:state.trips) if (trip.visit_applied) {
        if (!trip.arrival_tick) invalid("applied water visit has no actual arrival clock");
        const auto& row=home(state.houses,trip.target);
        if (!row.current_witness || order(*row.current_witness)<std::pair{*trip.arrival_tick,trip.transfer.courier_id})
            invalid("water current Witness discarded a newer retained successful visit");
    }
}
} // namespace

void World::validate_original_water_model() const {
    if (!original_rules_model_ || !original_rules_model_->supports_building(original::ModelBuilding::Well) ||
        original_rules_model_->building_value(original::ModelBuilding::Well,original::BuildingField::Cost)<=0)
        invalid("paid original water requires positive prepared Well model capability");
    const auto workers=original_rules_model_->building_value(original::ModelBuilding::Well,original::BuildingField::Employees);
    if (workers<=0 || workers>INT16_MAX) invalid("paid original water requires bounded positive model staffing");
}
void World::validate_original_water_snapshot(const WorldSnapshot& snapshot) {
    if (!snapshot.original_production || !snapshot.original_economy) invalid("water requires explicit original production authority");
    validate_water(snapshot.original_production->water,snapshot.buildings,snapshot.couriers,
                   snapshot.original_economy->houses,snapshot.original_production->housing,snapshot.ticks,snapshot.ticks,snapshot.ticks);
}
void World::validate_original_water_state() const {
    validate_water(original_production_->water,buildings_,couriers_,original_economy_->houses,original_production_->housing,ticks_,ticks_,ticks_);
}
void World::validate_original_water_evaluation_provenance(const WorldSnapshot& snapshot,BuildingId id,std::uint64_t tick,
    const std::optional<OriginalWaterWitness>& witness,bool supplied) {
    if (!snapshot.original_production || !snapshot.original_economy) invalid("historical water requires production authority");
    validate_evaluation(snapshot.original_production->water,snapshot.buildings,snapshot.couriers,
        snapshot.original_economy->houses,snapshot.original_production->housing,snapshot.ticks,id,tick,witness,supplied);
}
void World::validate_original_water_evaluation_provenance(BuildingId id,std::uint64_t tick,
    const std::optional<OriginalWaterWitness>& witness,bool supplied) const {
    validate_evaluation(original_production_->water,buildings_,couriers_,original_economy_->houses,
        original_production_->housing,ticks_,id,tick,witness,supplied);
}
bool World::original_water_courier(CourierRole role) const {
    return original_production_enabled() && role==CourierRole::OriginalWater;
}
CourierDispatchDecision World::original_water_dispatch_status(const CourierState& courier) const {
    (void)provider(original_production_->water,courier.owner);
    std::array<const BuildingState*,original_water_house_limit> targets{};std::size_t count=0;
    for (const auto& b:buildings_) if (b.kind==Object::Household && !building_on_fire(b.id) &&
        accepts(home(original_economy_->houses,b.id))) targets[count++]=&b;
    if (!count) return {CourierDispatchStatus::NoTarget,{}};
    std::size_t start=0;
    for (std::size_t i=0;i<count;++i) if (courier.last_dispatched_target==targets[i]->id) start=(i+1)%count;
    for (std::size_t i=0;i<count;++i) {
        const auto target=targets[(start+i)%count]->id;
        const auto route=std::lower_bound(courier.dynamic_target_routes.begin(),courier.dynamic_target_routes.end(),target,
            [](const auto& row,BuildingId key){return row.first<key;});
        if (route!=courier.dynamic_target_routes.end() && route->first==target && route->second)
            return {CourierDispatchStatus::Ready,target};
    }
    return {CourierDispatchStatus::NoRoad,{}};
}
void World::dispatch_original_water(CourierState& courier) {
    const auto decision=courier_dispatch_status(courier.id);
    if (decision.status!=CourierDispatchStatus::Ready || !decision.selected_target) return;
    const auto target=*decision.selected_target;
    const auto route=std::lower_bound(courier.dynamic_target_routes.begin(),courier.dynamic_target_routes.end(),target,
        [](const auto& row,BuildingId key){return row.first<key;});
    if (route==courier.dynamic_target_routes.end() || !route->second) throw std::logic_error("Water dispatch lost its shared cached route");
    auto candidate=original_production_->water;const auto* prior=latest_trip(candidate.trips,courier.id);
    if (prior && prior->transfer.sequence==UINT64_MAX) return;
    OriginalWaterTrip next{{courier.id,prior ? prior->transfer.sequence+1:1},target,ticks_};
    if (auto* old=latest_trip(candidate.trips,courier.id)) *old=next;
    else candidate.trips.insert(std::lower_bound(candidate.trips.begin(),candidate.trips.end(),courier.id,
        [](const auto& row,CourierId key){return row.transfer.courier_id<key;}),next);
    auto path=*route->second;
    if (!ticks_) throw std::logic_error("Water dispatch has no advanced World tick");
    validate_water(candidate,buildings_,couriers_,original_economy_->houses,original_production_->housing,ticks_,ticks_-1,ticks_-1,false);
    original_production_->water=std::move(candidate);courier.path=std::move(path);courier.target=target;
    courier.last_dispatched_target=target;courier.path_vertex=0;courier.edge_progress=0;courier.route_pending=false;
    courier.route_checked_revision.reset();courier.phase=CourierPhase::ToWarehouse;
}
void World::arrive_original_water(CourierState& courier) {
    if (courier.phase!=CourierPhase::ToWarehouse) {
        courier.path.clear();courier.path_vertex=0;courier.phase=CourierPhase::IdleAtWorkshop;
        courier.route_pending=false;courier.route_checked_revision.reset();return;
    }
    auto candidate=original_production_->water;auto* trip=latest_trip(candidate.trips,courier.id);
    if (!trip || trip->arrival_tick || trip->target!=courier.target) throw std::logic_error("Water arrival lost its real outbound trip");
    const auto& target=building(courier.target);const auto& h=home(original_economy_->houses,target.id);
    trip->arrival_tick=ticks_;
    if (!building_on_fire(target.id) && accepts(h)) {
        auto& row=home(candidate.houses,target.id);row.normal_timer=96;trip->visit_applied=true;
        row.current_witness=OriginalWaterWitness{target.id,courier.owner,trip->transfer,ticks_};
    }
    validate_water(candidate,buildings_,couriers_,original_economy_->houses,original_production_->housing,ticks_,ticks_-1,ticks_-1,false);
    original_production_->water=std::move(candidate);courier.phase=CourierPhase::Returning;
    if (valid_return_path(courier)) {std::reverse(courier.path.begin(),courier.path.end());courier.path_vertex=0;}
    else {courier.path={courier.path.back()};courier.path_vertex=0;courier.route_pending=true;courier.route_checked_revision.reset();reroute_v2(courier);}
}
void World::evaluate_original_water() {
    if (!ticks_ || original_water_evaluation_tick(ticks_)!=ticks_) throw std::logic_error("water evaluation invoked outside Phase28");
    auto candidate=original_production_->water;auto houses=original_economy_->houses;
    for (auto& row:candidate.houses) {
        auto& h=home(houses,row.building_id);
        if (evaluates(h)) {row.evaluation_witness=row.current_witness;h.house.water=row.normal_timer!=0;}
    }
    validate_water(candidate,buildings_,couriers_,houses,original_production_->housing,ticks_,ticks_-1,ticks_);
    original_production_->water=std::move(candidate);original_economy_->houses=std::move(houses);
}
void World::decay_original_water() {
    if (!ticks_ || original_water_countdowns(ticks_)==original_water_countdowns(ticks_-1))
        throw std::logic_error("water countdown invoked outside Phase35");
    auto candidate=original_production_->water;
    for (auto& row:candidate.houses) if (evaluates(home(original_economy_->houses,row.building_id)) && row.normal_timer) --row.normal_timer;
    validate_water(candidate,buildings_,couriers_,original_economy_->houses,original_production_->housing,ticks_,ticks_,ticks_);
    original_production_->water=std::move(candidate);
}

} // namespace openemperor::simulation
