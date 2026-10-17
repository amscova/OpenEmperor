#include "simulation/World.h"
#include "simulation/original/OriginalRulesModel.h"

#include <algorithm>
#include <array>
#include <map>
#include <stdexcept>

namespace openemperor::simulation {
namespace {
[[noreturn]] void invalid(const char* reason) {throw std::invalid_argument(reason);}
template<class Rows> const auto& by_id(const Rows& rows,BuildingId id) {
    const auto it=std::lower_bound(rows.begin(),rows.end(),id,[](const auto& row,BuildingId key) {
        if constexpr (requires {row.building_id;}) return row.building_id<key;
        else return row.id<key;
    });
    if (it==rows.end()) invalid("Housing history refers to missing paid identity");
    if constexpr (requires {it->building_id;}) {if (it->building_id!=id) invalid("Housing history has orphan identity");}
    else {if (it->id!=id) invalid("Housing history has orphan body");}
    return *it;
}
constexpr std::array roles{original::ServiceRole::Herbalist,original::ServiceRole::Acupuncture,original::ServiceRole::Ancestor};
bool early(original::HouseType type) {return type>=original::HouseType::Shelter && type<=original::HouseType::PlainCottage;}
original::HouseState evolved(const original::HouseEvolutionRules& rules,const original::HouseState& input) {
    auto result=original::evolve_step(rules,input);
    if (result.type>original::HouseType::PlainCottage) result.type=original::HouseType::PlainCottage;
    return result;
}
void add(std::uint64_t& total,std::uint64_t value) {
    if (value>UINT64_MAX-total) invalid("Housing historical counter overflow");total+=value;
}
std::uint64_t admission_opportunities(std::uint64_t first,std::uint64_t through) {
    return through<first ? 0:1+(through-first)/40;
}
void basic_input(const original::HouseState& input) {
    if (!early(input.type) || input.occupants<=0 || input.devolve_counter>9 ||
        input.desirability<-100 || input.desirability>100 || input.food_quality>80 ||
        input.music || input.acrobat || input.drama || input.confucian || input.daoist || input.buddhist)
        invalid("unsupported early Housing evaluation input");
    for (std::size_t g=0;g<original::goods_count;++g) {
        const auto stock=original::goods_stock(input,static_cast<original::Goods>(g));
        if (stock<0 || stock>(g<2 ? Rules::household_capacity:0)) invalid("Housing input invents unsupported goods stock");
    }
}
void historical_service(const WorldSnapshot& s,BuildingId id,std::uint64_t first,const OriginalHousingEvaluation& proof,std::size_t index) {
    const auto role=roles[index];const auto& value=proof.service_witnesses[index];std::uint8_t expected=0;
    if (value) {
        const auto& w=*value;const auto& source=by_id(s.buildings,w.provider_id);const auto& target=by_id(s.buildings,id);
        const auto actor=std::find_if(s.couriers.begin(),s.couriers.end(),[&](const auto& c){return c.id==w.transfer.courier_id;});
        const auto trip=std::find_if(s.original_production->services.trips.begin(),s.original_production->services.trips.end(),
            [&](const auto& row){return row.transfer.courier_id==w.transfer.courier_id;});
        const auto kind=index==0 ? Object::HealthPost:index==1 ? Object::ServicePost:Object::OriginalAncestorShrine214;
        const auto actor_role=index==0 ? CourierRole::HealthWorker:index==1 ? CourierRole::Service:CourierRole::OriginalAncestor;
        const auto delta=[](int a,int b){const auto d=std::int64_t(a)-b;return std::max<std::int64_t>(0,(d<0 ? -d:d)-1);};
        const auto travel=static_cast<std::uint64_t>(std::max<std::int64_t>(2,delta(source.cell.x,target.cell.x)+delta(source.cell.y,target.cell.y)))*
            static_cast<std::uint64_t>(profile_rules(s.profile,s.rule_version).courier_edge_ticks);
        if (w.building_id!=id || w.role!=role || source.kind!=kind || actor==s.couriers.end() || actor->owner!=w.provider_id ||
            actor->role!=actor_role || trip==s.original_production->services.trips.end() || !w.transfer.sequence ||
            w.transfer.sequence>trip->transfer.sequence || w.transfer.sequence>w.arrival_tick || w.arrival_tick>proof.tick ||
            w.arrival_tick<=source.placed_tick || w.arrival_tick-source.placed_tick<travel ||
            w.arrival_tick<first || w.arrival_tick-first<travel)
            invalid("Housing service input lacks a possible actual paid visit");
        if (trip->transfer==w.transfer) {
            if (!trip->visit_applied || trip->target!=id || trip->arrival_tick!=w.arrival_tick)
                invalid("Housing historical service contradicts retained arrival");
        } else if (w.arrival_tick>=trip->dispatched_tick) invalid("Housing service witness postdates later trip");
        const auto current=std::find_if(s.original_production->services.witnesses.begin(),s.original_production->services.witnesses.end(),
            [&](const auto& row){return row.building_id==id && row.role==role;});
        if (current==s.original_production->services.witnesses.end() || current->arrival_tick<w.arrival_tick ||
            (current->arrival_tick==w.arrival_tick && *current!=w)) invalid("Housing discarded its latest historical service origin");
        const auto n=original_water_countdowns(proof.tick)-original_water_countdowns(w.arrival_tick-1);
        const auto duration=original::service_duration(role);expected=n>=duration ? 0:static_cast<std::uint8_t>(duration-n);
    }
    const auto require_known=[&](const OriginalServiceWitness& known) {
        if (known.arrival_tick>proof.tick) return;
        const auto key=[](const auto& w){return std::pair{w.arrival_tick,w.transfer.courier_id};};
        if (!value || key(*value)<key(known) || (key(*value)==key(known) && *value!=known))
            invalid("Housing service input discarded an arrival already known at evaluation");
    };
    for (const auto& known:s.original_production->services.witnesses)
        if (known.building_id==id && known.role==role) require_known(known);
    for (const auto& trip:s.original_production->services.trips) if (trip.visit_applied && trip.target==id && trip.arrival_tick) {
        const auto actor=std::find_if(s.couriers.begin(),s.couriers.end(),[&](const auto& c){return c.id==trip.transfer.courier_id;});
        const auto actor_role=index==0 ? CourierRole::HealthWorker:index==1 ? CourierRole::Service:CourierRole::OriginalAncestor;
        if (actor!=s.couriers.end() && actor->role==actor_role) require_known({id,role,actor->owner,trip.transfer,*trip.arrival_tick});
    }
    if (original::service_timer(proof.input,role)!=expected) invalid("Housing input service timer differs from actual visit phase");
}
void historical_food(const WorldSnapshot& s,BuildingId id,std::uint64_t first,const OriginalHousingEvaluation& proof) {
    const auto& current=by_id(s.original_production->food.stocks,id);
    std::uint64_t received=0,consumed=0;
    for (std::size_t g=0;g<original_food_crop_count;++g) {
        if (proof.food_received[g]>current.received[g] || proof.food_consumed[g]>current.consumed[g] || proof.food_consumed[g]>proof.food_received[g])
            invalid("Housing Food input exceeds actual physical reception/sinks");
        add(received,proof.food_received[g]);add(consumed,proof.food_consumed[g]);
    }
    if (received-consumed>8 || consumed>original_housing_consumption_events(proof.tick-1)-original_housing_consumption_events(first-1))
        invalid("Housing Food input exceeds House capacity or occupied clock");
    if (current.quality_witness && current.quality_witness->tick<=proof.tick && (proof.food_quality_witness!=current.quality_witness || proof.food_received!=current.received))
        invalid("Housing Food input discarded a known last quality arrival");
    if (!proof.food_quality_witness) {
        if (received || consumed || proof.input.food_quality) invalid("Housing Food quality has no actual delivery witness");return;
    }
    const auto& w=*proof.food_quality_witness;
    const auto actor=std::find_if(s.couriers.begin(),s.couriers.end(),[&](const auto& c){return c.id==w.transfer.courier_id;});
    const auto trip=std::find_if(s.original_production->food.trips.begin(),s.original_production->food.trips.end(),
        [&](const auto& row){return row.transfer.courier_id==w.transfer.courier_id;});
    if (actor==s.couriers.end() || actor->role!=CourierRole::OriginalFoodToHouse || trip==s.original_production->food.trips.end() ||
        !w.transfer.sequence || w.transfer.sequence>trip->transfer.sequence || w.transfer.sequence>w.tick || !w.tick ||
        w.tick<first || w.tick>proof.tick || !current.quality_witness || current.quality_witness->tick<w.tick ||
        (current.quality_witness->tick==w.tick && *current.quality_witness!=w))
        invalid("Housing Food quality witness has no possible paid Distributor arrival");
    const auto& market=by_id(s.buildings,actor->owner);const auto& target=by_id(s.buildings,id);
    const auto gap=[](int a,int b){const auto d=std::int64_t(a)-b;return std::max<std::int64_t>(0,(d<0 ? -d:d)-1);};
    // Market is currently1x1; a conservative2x2 source lower bound remains
    // necessary without pretending to retain the complete road history.
    const auto travel=static_cast<std::uint64_t>(std::max<std::int64_t>(2,gap(market.cell.x,target.cell.x)+gap(market.cell.y,target.cell.y)))*
        static_cast<std::uint64_t>(profile_rules(s.profile,s.rule_version).courier_edge_ticks);
    if (market.kind!=Object::Market || w.tick<=market.placed_tick || w.tick-market.placed_tick<travel || w.tick-first<travel)
        invalid("Housing Food witness predates its actual paid Market/occupied target travel");
    if (trip->transfer==w.transfer && (trip->phase!=OriginalGoodsTransferPhase::Completed || trip->target!=id || trip->delivered!=w.accepted))
        invalid("Housing Food witness contradicts retained delivery");
    std::int32_t previous=0,accepted=0;std::uint64_t used_since=0;
    for (std::size_t g=0;g<original_food_crop_count;++g) {
        if (w.accepted[g]<0 || w.accepted[g]>4 || w.consumed_before[g]>proof.food_consumed[g] ||
            proof.food_received[g]<static_cast<std::uint64_t>(w.accepted[g]) ||
            proof.food_received[g]-static_cast<std::uint64_t>(w.accepted[g])<w.consumed_before[g])
            invalid("Housing Food witness invents portions");
        const auto prior=proof.food_received[g]-static_cast<std::uint64_t>(w.accepted[g])-w.consumed_before[g];
        const auto delta=proof.food_consumed[g]-w.consumed_before[g];
        if (prior>8 || delta>prior+static_cast<std::uint64_t>(w.accepted[g])) invalid("Housing Food witness exceeds historical stock");
        previous+=static_cast<std::int32_t>(prior);accepted+=w.accepted[g];add(used_since,delta);
    }
    if (accepted<=0 || accepted>4 || previous+accepted>8 || (!previous && w.previous_quality) ||
        (previous && (w.previous_quality<20 || w.previous_quality>80)) ||
        (previous && ((proof.food_received[1]==static_cast<std::uint64_t>(w.accepted[1]) && w.previous_quality!=20) ||
         (proof.food_received[0]==static_cast<std::uint64_t>(w.accepted[0]) && w.previous_quality!=80))) ||
        used_since>original_housing_consumption_events(proof.tick-1)-original_housing_consumption_events(w.tick-1))
        invalid("Housing Food witness violates real crop quality/capacity/clock");
    const auto expected=original::blend_food_quality(w.previous_quality,original_food_batch_quality(w.accepted).value,accepted,previous);
    if (proof.input.food_quality!=(received>consumed ? expected:0)) invalid("Housing Food input differs from witnessed blend/empty reset");
}
}

void World::validate_original_housing_snapshot(const WorldSnapshot& s) {
    const auto& state=s.original_production->housing;const auto& homes=s.original_economy->houses;
    if (state.houses.size()!=homes.size() || s.original_production->house_consumption.size()!=homes.size() || state.houses.size()>20 || state.neutral_source_origins.size()>37 ||
        state.neutral_source_origins.size()!=s.original_influence->sources.size()+s.original_influence->removed_sources.size())
        invalid("Housing authority requires exact bounded paid House/source histories");
    for (std::size_t i=0;i<state.neutral_source_origins.size();++i) {
        const auto& origin=state.neutral_source_origins[i];
        if ((i && !(state.neutral_source_origins[i-1].building_id<origin.building_id)) ||
            origin.origin.x<0 || origin.origin.y<0 || origin.origin.x>s.width-2 || origin.origin.y>s.height-2)
            invalid("Housing neutral source origins must be unique sorted on-map footprints");
        const auto live=std::find_if(s.original_influence->sources.begin(),s.original_influence->sources.end(),[&](const auto& p){return p.building_id==origin.building_id;});
        if (live!=s.original_influence->sources.end()) {
            if (by_id(s.buildings,origin.building_id).cell!=origin.origin) invalid("Housing source origin differs from actual paid body");
        } else if (std::none_of(s.original_influence->removed_sources.begin(),s.original_influence->removed_sources.end(),
            [&](const auto& row){return row.building_id==origin.building_id;})) invalid("Housing origin has no ever-paid neutral source");
    }
    for (std::size_t i=0;i<homes.size();++i) {
        const auto& h=homes[i];const auto& row=state.houses[i];const auto& b=by_id(s.buildings,h.building_id);
        if (row.building_id!=h.building_id) invalid("Housing rows must match exact paid House order");
        if (!row.first_admission_tick) {
            if (h.house!=original::HouseState{.type=original::HouseType::Vacant} || h.original_status || h.access_enabled || h.cycle_phase ||
                row.last_evaluation || row.first_hut_evaluation || row.first_plain_evaluation || row.consumption_rows!=std::array<OriginalHousingConsumptionRow,3>{})
                invalid("Vacant House has no admission but owns Housing authority");
            continue;
        }
        const auto first=*row.first_admission_tick;
        if (first>s.ticks || first<b.placed_tick || first-b.placed_tick<120 || (first-b.placed_tick-120)%40 ||
            h.house.occupants<=0 || h.original_status!=3 || !h.access_enabled || h.cycle_phase)
            invalid("Housing admission must be a real paid120+40k opportunity");
        basic_input(h.house);
        const auto latest=original_housing_evaluation_tick(s.ticks);
        if (static_cast<bool>(row.last_evaluation)!=(latest>=first)) invalid("Housing last evaluation is missing or prematurely injected");
        std::uint64_t events=0;std::array<std::uint64_t,original::goods_count> consumed{};
        for (const auto& c:row.consumption_rows) {
            add(events,c.events);
            for (std::size_t g=0;g<original::goods_count;++g) {
                if (c.consumed[g]>c.events*std::uint64_t(g<2 ? 8:0)) invalid("Housing consumption row exceeds physical stock/events");
                add(consumed[g],c.consumed[g]);
            }
        }
        if (events!=original_housing_consumption_events(s.ticks)-original_housing_consumption_events(first-1) ||
            consumed!=s.original_production->house_consumption[i].consumed) invalid("Housing row sinks/events disagree with occupied clock/ledger");
        for (const auto* proof:{&row.last_evaluation,&row.first_hut_evaluation,&row.first_plain_evaluation}) if (*proof) {
            const auto& e=**proof;basic_input(e.input);
            if (e.tick<first || e.tick>s.ticks || original_housing_evaluation_tick(e.tick)!=e.tick ||
                e.input.occupants>h.house.occupants) invalid("Housing evaluation has impossible paid occupied phase");
            for (std::size_t g=0;g<original::goods_count;++g) {
                const auto before=e.goods_consumed_before[g];const auto total=consumed[g];
                const auto input=static_cast<std::uint64_t>(original::goods_stock(e.input,static_cast<original::Goods>(g)));
                const auto current=static_cast<std::uint64_t>(original::goods_stock(h.house,static_cast<original::Goods>(g)));
                if (before>total || before>UINT64_MAX-input || total>UINT64_MAX-current || input+before>current+total)
                    invalid("Housing evaluation invents goods without actual House delivery");
            }
            for (std::size_t r=0;r<roles.size();++r) historical_service(s,h.building_id,first,e,r);
            validate_original_water_evaluation_provenance(s,h.building_id,e.tick,e.water_evaluation_witness,e.input.water);
            historical_food(s,h.building_id,first,e);
        }
        if (row.last_evaluation && row.last_evaluation->tick!=latest) invalid("Housing last evaluation skipped an occupied phase39");
        if (row.first_hut_evaluation && (row.first_hut_evaluation->input.type!=original::HouseType::Shelter ||
            row.first_hut_evaluation->tick>latest)) invalid("first Hut proof is not a Shelter input");
        if (row.first_plain_evaluation && (!row.first_hut_evaluation || row.first_plain_evaluation->input.type!=original::HouseType::Hut ||
            row.first_plain_evaluation->tick<=row.first_hut_evaluation->tick || row.first_plain_evaluation->tick>latest))
            invalid("first Plain proof lacks an earlier real Hut proof");
        for (std::size_t r=1;r<row.consumption_rows.size();++r) {
            const auto& upgrade=r==1 ? row.first_hut_evaluation:row.first_plain_evaluation;
            const auto opportunities=upgrade ? original_housing_consumption_events(s.ticks)-original_housing_consumption_events(upgrade->tick-1):0;
            if (row.consumption_rows[r].events>opportunities) invalid("Housing consumption row predates its first actual stage upgrade");
        }
        const auto shelter_until=row.first_hut_evaluation ? row.first_hut_evaluation->tick-1:s.ticks;
        if (row.consumption_rows[0].events<original_housing_consumption_events(shelter_until)-original_housing_consumption_events(first-1))
            invalid("Housing consumption omitted mandatory initial Shelter events");
        for (const auto* proof:{&row.last_evaluation,&row.first_hut_evaluation,&row.first_plain_evaluation}) if (*proof) {
            const auto& e=**proof;
            const auto first_eval=original_housing_evaluation_tick(first)==first ? first:original_housing_evaluation_tick(first)+51;
            auto previous_evaluations=(e.tick-first_eval)/51;
            const auto* upgrade=e.input.type==original::HouseType::Hut ? &row.first_hut_evaluation:
                e.input.type==original::HouseType::PlainCottage ? &row.first_plain_evaluation:nullptr;
            if (upgrade && *upgrade && (**upgrade).tick<e.tick)
                previous_evaluations=std::min(previous_evaluations,(e.tick-(**upgrade).tick)/51-1);
            if (e.input.devolve_counter>previous_evaluations) invalid("Housing input counter exceeds earlier actual evaluations");
        }
        for (const auto* first_proof:{&row.first_hut_evaluation,&row.first_plain_evaluation})
            if (row.last_evaluation && *first_proof && row.last_evaluation->tick==(**first_proof).tick &&
                *row.last_evaluation!=**first_proof) invalid("same House phase39 has conflicting first/latest inputs");
        const auto require_proofs=[&](original::HouseType type,std::uint64_t tick) {
            if (type>=original::HouseType::Hut && (!row.first_hut_evaluation || row.first_hut_evaluation->tick>=tick))
                invalid("higher Housing input has no earlier first Hut proof");
            if (type>=original::HouseType::PlainCottage && (!row.first_plain_evaluation || row.first_plain_evaluation->tick>=tick))
                invalid("Plain Housing input has no earlier first Plain proof");
        };
        if (row.last_evaluation) require_proofs(row.last_evaluation->input.type,row.last_evaluation->tick);
        if ((h.house.type>=original::HouseType::Hut && !row.first_hut_evaluation) ||
            (h.house.type>=original::HouseType::PlainCottage && !row.first_plain_evaluation)) invalid("higher current House lacks its permanent first upgrade");
    }
}

void World::validate_original_housing_model() {
    const auto& rules=original_rules_model_->house_rules();
    for (const auto type:{original::HouseType::Shelter,original::HouseType::Hut,original::HouseType::PlainCottage}) {
        const auto cap=rules.row_capacity(original::admission_capacity_row(type));
        if (cap<=0 || cap>INT16_MAX) invalid("early Housing requires positive signed-word capacities");
    }
    for (const auto& source:original_production_->housing.neutral_source_origins)
        for (int y=0;y<2;++y) for (int x=0;x<2;++x)
            if (!permissions_->building_allowed({source.origin.x+x,source.origin.y+y})) invalid("historical neutral source origin violates immutable permissions");
    const original::SpreadSpec spread{
        original_rules_model_->building_value(original::ModelBuilding::Well,original::BuildingField::Desirability),
        original_rules_model_->building_value(original::ModelBuilding::Well,original::BuildingField::DesirabilityStep),
        original_rules_model_->building_value(original::ModelBuilding::Well,original::BuildingField::DesirabilityStepSize),
        original_rules_model_->building_value(original::ModelBuilding::Well,original::BuildingField::DesirabilityRange)};
    std::map<std::uint64_t,original::DesirabilityMap> maps;
    const auto desirability=[&](std::uint64_t tick,Cell cell) {
        auto found=maps.find(tick);
        if (found==maps.end()) {
            original::DesirabilityMap map(width_,height_);
            std::vector<std::pair<BuildingId,Cell>> sources;
            for (const auto& origin:original_production_->housing.neutral_source_origins) {
                const auto live=std::find_if(buildings_.begin(),buildings_.end(),[&](const auto& b){return b.id==origin.building_id;});
                if (live!=buildings_.end()) {if (live->placed_tick<tick) sources.emplace_back(origin.building_id,origin.origin);}
                else {
                    const auto& old=by_id(original_influence_->removed_sources,origin.building_id);
                    if (old.placed_tick<tick && old.removed_tick>=tick) sources.emplace_back(origin.building_id,origin.origin);
                }
            }
            for (const auto& p:original_production_->water.providers) {const auto& b=building(p.building_id);if (b.placed_tick<tick) sources.emplace_back(b.id,b.cell);}
            std::sort(sources.begin(),sources.end(),[](const auto& a,const auto& b){return a.first<b.first;});
            for (const auto& source:sources) map.apply_source({{source.second.x,source.second.y,2},spread});
            found=maps.emplace(tick,std::move(map)).first;
        }
        return found->second.building_value({cell.x,cell.y,2});
    };
    for (std::size_t i=0;i<original_economy_->houses.size();++i) {
        const auto& home=original_economy_->houses[i];const auto& row=original_production_->housing.houses[i];
        if (!row.first_admission_tick) continue;
        const auto first=*row.first_admission_tick;const auto& b=building(home.building_id);
        const auto maximum_population=[&](std::uint64_t through) {
            auto cap=rules.row_capacity(original::admission_capacity_row(original::HouseType::Shelter));
            std::uint64_t population=0,previous=first;
            const auto accumulate=[&](std::uint64_t until) {
                const auto count=admission_opportunities(first,until)-(previous==first ? 0:admission_opportunities(first,previous-1));
                population=std::min<std::uint64_t>(static_cast<std::uint64_t>(cap),population+2*count);
                previous=until+1;
            };
            for (const auto* proof:{&row.first_hut_evaluation,&row.first_plain_evaluation}) if (*proof && (**proof).tick<through) {
                accumulate((**proof).tick);
                cap=std::max(cap,rules.row_capacity(original::admission_capacity_row(evolved(rules,(**proof).input).type)));
            }
            accumulate(through);return population;
        };
        const auto parity_reachable=[&](int population,std::uint64_t through) {
            if (population%2==0) return true;
            const auto eligible=[&](original::HouseType type,int population_at_unlock) {
                const auto cap=rules.row_capacity(original::admission_capacity_row(type));
                return cap%2 && cap>population_at_unlock && cap<=population;
            };
            return eligible(original::HouseType::Shelter,0) ||
                (row.first_hut_evaluation && row.first_hut_evaluation->tick<through && eligible(original::HouseType::Hut,row.first_hut_evaluation->input.occupants)) ||
                (row.first_plain_evaluation && row.first_plain_evaluation->tick<through && eligible(original::HouseType::PlainCottage,row.first_plain_evaluation->input.occupants));
        };
        if (!parity_reachable(home.house.occupants,ticks_)) invalid("Housing population has no possible earlier odd-cap saturation");
        for (const auto* proof:{&row.last_evaluation,&row.first_hut_evaluation,&row.first_plain_evaluation}) if (*proof)
            if (!parity_reachable((**proof).input.occupants,(**proof).tick)) invalid("Housing input population has no earlier odd-cap saturation");
        if (static_cast<std::uint64_t>(home.house.occupants)>maximum_population(ticks_))
            invalid("Housing residents exceed proven historical admission capacities/opportunities");
        for (const auto* proof:{&row.last_evaluation,&row.first_hut_evaluation,&row.first_plain_evaluation}) if (*proof)
            if (static_cast<std::uint64_t>((**proof).input.occupants)>maximum_population((**proof).tick))
                invalid("Housing input residents exceed earlier real stage capacities/opportunities");
        for (std::size_t r=0;r<row.consumption_rows.size();++r) for (std::size_t g=0;g<original::goods_count;++g) {
            const auto need=std::clamp(rules.value(original::house_row(static_cast<original::HouseType>(r+3)),original::goods_field(static_cast<original::Goods>(g))),0,g<2 ? 8:0);
            if (row.consumption_rows[r].consumed[g]>row.consumption_rows[r].events*static_cast<std::uint64_t>(need))
                invalid("Housing row consumption exceeds its actual model need");
        }
        for (const auto* proof:{&row.last_evaluation,&row.first_hut_evaluation,&row.first_plain_evaluation}) if (*proof) {
            if ((**proof).input.desirability!=desirability((**proof).tick,b.cell)) invalid("Housing historical desire differs from paid sources at phase39");
        }
        if (row.first_hut_evaluation && evolved(rules,row.first_hut_evaluation->input).type!=original::HouseType::Hut)
            invalid("first Hut proof fails unchanged pure evolution");
        if (row.first_plain_evaluation && evolved(rules,row.first_plain_evaluation->input).type!=original::HouseType::PlainCottage)
            invalid("first Plain proof fails unchanged pure evolution");
    }
    validate_original_housing_state();
}

void World::validate_original_housing_state() const {
    for (std::size_t i=0;i<original_economy_->houses.size();++i) {
        const auto& home=original_economy_->houses[i];const auto& row=original_production_->housing.houses[i];
        if (row.last_evaluation) {
            const auto expected=evolved(original_rules_model_->house_rules(),row.last_evaluation->input);
            if (home.house.type!=expected.type || home.house.devolve_counter!=expected.devolve_counter || home.house.desirability!=expected.desirability)
                invalid("current Housing type/counter differs from last pure evaluation");
        } else if (row.first_admission_tick && (home.house.type!=original::HouseType::Shelter || home.house.devolve_counter || home.house.desirability))
            invalid("newly occupied Shelter changed before its first phase39");
    }
}

void World::evaluate_original_houses() {
    for (std::size_t i=0;i<original_economy_->houses.size();++i) {
        auto& home=original_economy_->houses[i];if (!home.house.occupants) continue;
        auto& row=original_production_->housing.houses[i];OriginalHousingEvaluation proof;
        home.house.desirability=static_cast<std::int8_t>(original_influence_value_at(building(home.building_id).cell));
        proof.tick=ticks_;proof.input=home.house;
        for (std::size_t r=0;r<roles.size();++r) {
            const auto w=std::find_if(original_production_->services.witnesses.begin(),original_production_->services.witnesses.end(),
                [&](const auto& witness){return witness.building_id==home.building_id && witness.role==roles[r];});
            if (w!=original_production_->services.witnesses.end()) proof.service_witnesses[r]=*w;
        }
        proof.water_evaluation_witness=original_production_->water.houses[i].evaluation_witness;
        const auto& food=by_id(original_production_->food.stocks,home.building_id);
        proof.food_quality_witness=food.quality_witness;proof.food_received=food.received;proof.food_consumed=food.consumed;
        proof.goods_consumed_before=original_production_->house_consumption[i].consumed;
        home.house=evolved(original_rules_model_->house_rules(),home.house);
        row.last_evaluation=proof;
        if (!row.first_hut_evaluation && proof.input.type==original::HouseType::Shelter && home.house.type==original::HouseType::Hut) row.first_hut_evaluation=proof;
        if (!row.first_plain_evaluation && proof.input.type==original::HouseType::Hut && home.house.type==original::HouseType::PlainCottage) row.first_plain_evaluation=proof;
    }
}

} // namespace openemperor::simulation
