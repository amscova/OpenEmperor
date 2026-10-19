#include "simulation/World.h"
#include "simulation/original/OriginalRulesModel.h"
#include "simulation/original/OriginalScheduler.h"

#include <algorithm>
#include <array>
#include <stdexcept>
#include <utility>

namespace openemperor::simulation {
namespace {
[[noreturn]] void invalid(const char* reason) {throw std::invalid_argument(reason);}
std::uint64_t magnitude(std::int64_t negative) noexcept {
    return static_cast<std::uint64_t>(-(negative+1))+1;
}
std::int64_t negative_value(std::uint64_t value) noexcept {
    return value ? -static_cast<std::int64_t>(value-1)-1:0;
}
std::uint64_t add(std::uint64_t a,std::uint64_t b) {
    if (b>UINT64_MAX-a) throw std::overflow_error("original finance counter exhausted");
    return a+b;
}
std::uint64_t multiply(std::uint64_t a,std::uint64_t b) {
    if (b && a>UINT64_MAX/b) throw std::overflow_error("original finance product exhausted");
    return a*b;
}
template<class Rows> const auto& by_id(const Rows& rows,BuildingId id) {
    const auto it=std::lower_bound(rows.begin(),rows.end(),id,[](const auto& row,BuildingId key){
        if constexpr (requires {row.building_id;}) return row.building_id<key;
        else return row.id<key;
    });
    if (it==rows.end()) invalid("original finance has missing paid identity");
    if constexpr (requires {it->building_id;}) {if (it->building_id!=id) invalid("original finance has orphan House");}
    else if (it->id!=id) invalid("original finance has orphan body");
    return *it;
}
bool occupied_type(Object kind,original::HouseType type) {
    return original_elite_house_kind(kind) ? type>=original::HouseType::EliteAbandoned && type<=original::HouseType::HeavenlyCompound:
        type>=original::HouseType::Shelter && type<=original::HouseType::LuxuriousApartment;
}
std::size_t row_index(original::HouseType type) {return original::model_row(original::house_row(type)).value;}
std::uint64_t opportunities(std::uint64_t first,std::uint64_t through) {
    return through<first ? 0:1+(through-first)/40;
}
// The existing admission kernel can only add residents. Earlier unlocked
// capacities bound every later admission, including residents retained after
// devolution. No current-capacity clamp or footprint multiplier is introduced.
std::uint64_t population_bound(const OriginalHousingHouse& history,Object kind,
    const original::HouseEvolutionRules& rules,std::uint64_t through) {
    if (!history.first_admission_tick || through<*history.first_admission_tick) return 0;
    const auto first=*history.first_admission_tick;
    const auto initial=original_elite_house_kind(kind) ? original::HouseType::ModestSiheyuan:original::HouseType::Shelter;
    const auto& admission=history.first_transitions[row_index(initial)];
    if (!admission || admission->admitted<=0) invalid("original tax requires actual first admission");
    std::array<std::pair<original::HouseType,const OriginalHousingTransition*>,original::house_row_count> entries{};
    std::size_t count=0;
    for (std::size_t r=0;r<history.first_transitions.size();++r) if (const auto& entry=history.first_transitions[r])
        entries[count++]={static_cast<original::HouseType>(r+3),&*entry};
    std::sort(entries.begin(),entries.begin()+count,[](const auto& a,const auto& b){return a.second->evaluation.tick<b.second->evaluation.tick;});
    auto population=static_cast<std::uint64_t>(admission->admitted),previous=first+1;
    auto capacity=rules.row_capacity(original::admission_capacity_row(initial));
    const auto accumulate=[&](std::uint64_t until) {
        if (until<previous) return;
        const auto n=opportunities(first,until)-opportunities(first,previous-1);
        if (population<static_cast<std::uint64_t>(capacity))
            population=std::min(static_cast<std::uint64_t>(capacity),population+2*n);
        previous=until+1;
    };
    for (std::size_t i=0;i<count;++i) {
        const auto& [type,entry]=entries[i];
        if (entry->kind==OriginalHousingTransitionKind::Evolution && entry->evaluation.tick<through) {
            accumulate(entry->evaluation.tick);
            capacity=std::max(capacity,rules.row_capacity(original::admission_capacity_row(type)));
        }
    }
    accumulate(through);return population;
}
bool odd_population_possible(const OriginalHousingHouse& history,Object kind,
    const original::HouseEvolutionRules& rules,int population,std::uint64_t through) {
    if (population%2==0) return true;
    const auto initial=original_elite_house_kind(kind) ? original::HouseType::ModestSiheyuan:original::HouseType::Shelter;
    const auto& admission=history.first_transitions[row_index(initial)];
    if (admission && admission->admitted%2 && admission->admitted<=population) return true;
    for (std::size_t r=0;r<history.first_transitions.size();++r) if (const auto& entry=history.first_transitions[r]) {
        const auto capacity=rules.row_capacity(original::admission_capacity_row(static_cast<original::HouseType>(r+3)));
        if (entry->evaluation.tick<through && capacity%2 && capacity>entry->evaluation.input.occupants && capacity<=population) return true;
    }
    return false;
}
// Maximum population sum for a bounded count of the latest possible monthly
// inputs. Split at the at-most15 unlocks; within one interval a population of
// at most100 reaches its bound after at most100 sampled months. Long saves
// therefore never cause a loop proportional to elapsed World time.
std::uint64_t resident_units_bound(std::uint64_t events,std::uint64_t last,
    const OriginalHousingHouse& history,Object kind,const original::HouseEvolutionRules& rules,int current_population) {
    if (!events) return 0;
    const auto last_month=last/original_tax_interval_ticks;
    if (events>last_month) invalid("original tax events exceed monthly clock");
    auto month=last_month-events+1;
    std::uint64_t total=0;
    while (month<=last_month) {
        auto end=last_month;
        for (const auto& entry:history.first_transitions) if (entry) {
            const auto unlock=entry->evaluation.tick/original_tax_interval_ticks+1;
            if (unlock>month && unlock<=end) end=unlock-1;
        }
        const auto bound=[&](std::uint64_t m) {
            return std::min<std::uint64_t>(static_cast<std::uint64_t>(current_population),
                population_bound(history,kind,rules,m*original_tax_interval_ticks-1));
        };
        const auto plateau=bound(end);
        std::size_t sampled=0;
        while (month<=end) {
            const auto population=bound(month);
            if (population==plateau) {total=add(total,multiply(end-month+1,plateau));month=end+1;break;}
            if (++sampled>100) invalid("original tax population bound did not saturate");
            total=add(total,population);++month;
        }
    }
    return total;
}
template<class Homes,class Bodies>
std::uint64_t validate_records(const OriginalFinanceState& finance,const Homes& homes,
    const OriginalHousingState& housing,const Bodies& bodies,std::uint64_t ticks,
    const original::HouseEvolutionRules* rules,bool detailed) {
    if (finance.houses.size()!=homes.size() || housing.houses.size()!=homes.size() || homes.size()>20)
        invalid("original finance requires an exact bounded House bijection");
    std::uint64_t global=0;
    for (std::size_t i=0;i<homes.size();++i) {
        const auto& home=homes[i];const auto& record=finance.houses[i];const auto& history=housing.houses[i];
        const auto& b=by_id(bodies,home.building_id);
        if (record.building_id!=home.building_id || history.building_id!=home.building_id || !original_house_kind(b.kind))
            invalid("original finance House identity/order differs from paid body");
        if (record.last_assessment) {
            const auto& last=*record.last_assessment;
            if (!last.tick || last.tick%original_tax_interval_ticks || last.tick>ticks ||
                !occupied_type(b.kind,last.type) || last.occupants<=0 || last.occupants>home.house.occupants ||
                !history.first_admission_tick || last.tick<=*history.first_admission_tick)
                invalid("original last tax assessment has impossible time/family/residents");
            // Rule10 has no FireInspector; its retained deadline describes
            // the latest natural incident, including after expiry. Finance
            // samples pre-tick fire, so ignition itself can pay, whereas the
            // following600 ticks through the exact expiry cannot.
            if (last.tick<=b.fire_until_tick &&
                b.fire_until_tick-last.tick<Rules::fire_incident_ticks)
                invalid("original last tax assessment occurred during the latest natural fire");
        }
        std::uint64_t events=0,units=0,paid=0;
        for (std::size_t r=0;r<record.rows.size();++r) {
            const auto& bucket=record.rows[r];
            const auto type=static_cast<original::HouseType>(r+3);
            if ((!bucket.events)!=(!bucket.resident_units) || bucket.resident_units<bucket.events)
                invalid("original tax bucket has inconsistent events/residents");
            if (bucket.events) {
                if (!record.last_assessment || !history.first_admission_tick || !occupied_type(b.kind,type) ||
                    !history.first_transitions[r]) invalid("original tax bucket precedes its actual occupied type");
                const auto& last=*record.last_assessment;
                const auto through=last.type==type ? last.tick:last.tick>=original_tax_interval_ticks ? last.tick-original_tax_interval_ticks:0;
                const auto& transition=*history.first_transitions[r];
                const auto unlocked=transition.evaluation.tick;
                const auto minimum=transition.kind==OriginalHousingTransitionKind::Admission ? transition.admitted:transition.evaluation.input.occupants;
                if (through<=unlocked || bucket.events>through/original_tax_interval_ticks-unlocked/original_tax_interval_ticks ||
                    minimum<=0 || bucket.resident_units<multiply(bucket.events,static_cast<std::uint64_t>(minimum)) ||
                    bucket.resident_units>multiply(bucket.events,static_cast<std::uint64_t>(home.house.occupants)))
                    invalid("original tax history exceeds unlocked months/current actual residents");
                if (rules && detailed) {
                    const auto maximum=last.type==type ?
                        add(resident_units_bound(bucket.events-1,through-original_tax_interval_ticks,history,b.kind,*rules,home.house.occupants),
                            static_cast<std::uint64_t>(last.occupants)):
                        resident_units_bound(bucket.events,through,history,b.kind,*rules,home.house.occupants);
                    if (bucket.resident_units>maximum) invalid("original tax residents precede their admission/capacity opportunities");
                }
            }
            events=add(events,bucket.events);units=add(units,bucket.resident_units);
            if (rules) {
                const auto rate=rules->value(static_cast<original::ModelHouse>(r),original::HouseField::TaxRate);
                if (rate<0) invalid("original finance requires nonnegative prepared TaxRate");
                paid=add(paid,multiply(bucket.resident_units,static_cast<std::uint64_t>(rate)));
            }
        }
        if (record.last_assessment) {
            const auto& last=*record.last_assessment;
            if (events>last.tick/original_tax_interval_ticks-*history.first_admission_tick/original_tax_interval_ticks)
                invalid("original last tax assessment has impossible time/family/residents");
            // Different type buckets cannot both occupy the same monthly
            // slots after their respective first actual unlocks.
            for (std::size_t r=0;r<record.rows.size();++r) if (record.rows[r].events) {
                const auto unlocked=history.first_transitions[r]->evaluation.tick;
                std::uint64_t later=0;
                for (std::size_t q=0;q<record.rows.size();++q) if (record.rows[q].events &&
                    history.first_transitions[q]->evaluation.tick>=unlocked) later=add(later,record.rows[q].events);
                if (later>last.tick/original_tax_interval_ticks-unlocked/original_tax_interval_ticks)
                    invalid("original tax type buckets overlap unavailable monthly slots");
            }
            const auto& bucket=record.rows[row_index(last.type)];
            const auto& proof=history.first_transitions[row_index(last.type)];
            if (!bucket.events || !proof) invalid("original last tax input lacks a matching unlocked bucket");
            const auto& transition=*proof;
            const auto minimum=transition.kind==OriginalHousingTransitionKind::Admission ? transition.admitted:transition.evaluation.input.occupants;
            if (last.occupants<minimum || bucket.resident_units<
                add(multiply(bucket.events-1,static_cast<std::uint64_t>(minimum)),static_cast<std::uint64_t>(last.occupants)))
                invalid("original last tax input is absent from its bucket");
            if (rules && detailed && (static_cast<std::uint64_t>(last.occupants)>population_bound(history,b.kind,*rules,last.tick-1) ||
                !odd_population_possible(history,b.kind,*rules,last.occupants,last.tick-1)))
                invalid("original last tax population was not yet possible before its tick");
        } else if (events || units || b.taxes_paid_total) invalid("original House payments lack tax history");
        // An unbound maximum is only a conservative rejection bound. If its
        // mathematical value exceeds uint64, every representable payment is
        // already below it; only the later actual model product may overflow.
        if ((rules && paid!=b.taxes_paid_total) ||
            (!rules && units<=UINT64_MAX/100 && b.taxes_paid_total>units*100))
            invalid("original House taxes differ from model-bound historical residents");
        global=add(global,b.taxes_paid_total);
    }
    return global;
}
}

OriginalFinanceBooking original_finance_book(std::int64_t funds,std::uint64_t taxes,
    std::uint64_t maintenance,std::uint64_t income,std::uint64_t bill) {
    const auto next_taxes=add(taxes,income),next_maintenance=add(maintenance,bill);
    if (funds<0) {
        const auto debt=magnitude(funds);
        if (income>debt+static_cast<std::uint64_t>(INT64_MAX)) throw std::overflow_error("original tax Funds exhausted");
        funds=income<debt ? negative_value(debt-income):static_cast<std::int64_t>(income-debt);
    } else {
        if (income>static_cast<std::uint64_t>(INT64_MAX-funds)) throw std::overflow_error("original tax Funds exhausted");
        funds+=static_cast<std::int64_t>(income);
    }
    if (funds<0) {
        const auto debt=magnitude(funds),limit=std::uint64_t{1}<<63;
        if (bill>limit-debt) throw std::overflow_error("original upkeep Funds exhausted");
        funds=negative_value(debt+bill);
    } else {
        const auto available=static_cast<std::uint64_t>(funds),limit=std::uint64_t{1}<<63;
        if (bill>available+limit) throw std::overflow_error("original upkeep Funds exhausted");
        funds=bill>available ? negative_value(bill-available):static_cast<std::int64_t>(available-bill);
    }
    return {funds,next_taxes,next_maintenance};
}

bool original_finance_accounting_valid(std::int64_t funds,std::uint64_t taxes,
    std::uint64_t construction,std::uint64_t maintenance) noexcept {
    std::array<std::uint64_t,3> credits{static_cast<std::uint64_t>(original_production_starting_treasury),
        taxes,funds<0 ? magnitude(funds):0};
    std::array<std::uint64_t,3> debits{construction,maintenance,funds>=0 ? static_cast<std::uint64_t>(funds):0};
    for (auto& credit:credits) for (auto& debit:debits) {
        const auto n=std::min(credit,debit);credit-=n;debit-=n;
    }
    return std::ranges::all_of(credits,[](auto n){return !n;}) && std::ranges::all_of(debits,[](auto n){return !n;});
}

void World::validate_original_finance_snapshot(const WorldSnapshot& s) {
    try {
    const auto taxes=validate_records(s.original_production->finance,s.original_economy->houses,
        s.original_production->housing,s.buildings,s.ticks,nullptr,false);
    if (taxes!=s.taxes_collected_total || !original_finance_accounting_valid(s.treasury,
        s.taxes_collected_total,s.construction_spent_total,s.maintenance_spent_total))
        invalid("original finance global payments or Funds do not reconcile");
    } catch (const std::overflow_error&) {invalid("original finance snapshot arithmetic exceeds supported counters");}
}

std::int64_t World::original_worker_upkeep(BuildingId id) const {
    const auto workers=workforce_required(id);
    if (workers<0) invalid("original upkeep requires nonnegative installed workers");
    return static_cast<std::int64_t>(workers)*original_worker_upkeep_funds;
}

void World::validate_original_finance_model() const {
    try {
    const auto& rules=original_rules_model_->house_rules();
    for (std::size_t r=0;r<original::house_row_count;++r)
        if (rules.value(static_cast<original::ModelHouse>(r),original::HouseField::TaxRate)<0)
            invalid("original finance requires nonnegative prepared TaxRate in every row");
    const auto taxes=validate_records(original_production_->finance,original_economy_->houses,
        original_production_->housing,buildings_,ticks_,&rules,true);
    if (taxes!=taxes_collected_total_ || !original_finance_history_valid())
        invalid("original finance does not match the bound model and actual installation ages");
    } catch (const std::overflow_error&) {invalid("original model-bound finance arithmetic exceeds supported counters");}
}

bool World::original_finance_history_valid() const {
    try {
        const auto taxes=validate_records(original_production_->finance,original_economy_->houses,
            original_production_->housing,buildings_,ticks_,&original_rules_model_->house_rules(),false);
        std::uint64_t maintenance=0;
        for (const auto& b:buildings_) if (b.placed) {
            if (b.placed_tick>ticks_) return false;
            maintenance=add(maintenance,multiply((ticks_-b.placed_tick)/original_worker_upkeep_interval_ticks,
                static_cast<std::uint64_t>(original_worker_upkeep(b.id))));
        }
        return taxes==taxes_collected_total_ && maintenance==maintenance_spent_total_ &&
            original_finance_accounting_valid(treasury_,taxes_collected_total_,construction_spent_total_,maintenance_spent_total_);
    } catch (const std::invalid_argument&) {return false;}
      catch (const std::overflow_error&) {return false;}
}

void World::book_original_finance_tick(std::uint64_t next) {
    const bool assess=next%original_tax_interval_ticks==0;
    OriginalFinanceState candidate;
    std::vector<std::pair<BuildingId,std::uint64_t>> payments;
    std::uint64_t income=0,bill=0;
    if (assess) {
        candidate=original_production_->finance;
        payments.reserve(original_economy_->houses.size());
        for (std::size_t i=0;i<original_economy_->houses.size();++i) {
            const auto& home=original_economy_->houses[i];
            if (home.house.occupants<=0 || building_on_fire(home.building_id)) continue;
            const auto rate=original_rules_model_->house_rules().value(original::house_row(home.house.type),original::HouseField::TaxRate);
            if (rate<0) invalid("negative original TaxRate");
            const auto occupants=static_cast<std::uint64_t>(home.house.occupants);
            const auto tax=multiply(occupants,static_cast<std::uint64_t>(rate));
            const auto paid=add(building(home.building_id).taxes_paid_total,tax);
            auto& record=candidate.houses[i];auto& bucket=record.rows[row_index(home.house.type)];
            bucket.events=add(bucket.events,1);bucket.resident_units=add(bucket.resident_units,occupants);
            record.last_assessment=OriginalTaxAssessment{next,home.house.type,home.house.occupants};
            income=add(income,tax);payments.emplace_back(home.building_id,paid);
        }
    }
    for (const auto& b:buildings_) if (b.placed && next>b.placed_tick &&
        (next-b.placed_tick)%original_worker_upkeep_interval_ticks==0)
        bill=add(bill,static_cast<std::uint64_t>(original_worker_upkeep(b.id)));
    const auto booked=original_finance_book(treasury_,taxes_collected_total_,maintenance_spent_total_,income,bill);
    // Every allocation, signed/counter product and invoice has passed. Commit
    // before any tick, producer, movement, incident, admission or evolution.
    if (assess) original_production_->finance=std::move(candidate);
    for (const auto& [id,paid]:payments) mutable_building(id).taxes_paid_total=paid;
    treasury_=booked.funds;taxes_collected_total_=booked.taxes;maintenance_spent_total_=booked.maintenance;
}

OriginalHouseTaxStatus World::original_house_tax_status(BuildingId id) const {
    OriginalHouseTaxStatus result;
    if (!original_production_enabled()) return result;
    const auto found=std::lower_bound(original_economy_->houses.begin(),original_economy_->houses.end(),id,
        [](const auto& row,BuildingId key){return row.building_id<key;});
    if (found==original_economy_->houses.end() || found->building_id!=id) return result;
    result.applicable=true;result.occupants=found->house.occupants;result.inhabited=result.occupants>0;
    result.burning=building_on_fire(id);result.eligible=result.inhabited && !result.burning;
    result.rate=original_rules_model_->house_rules().value(original::house_row(found->house.type),original::HouseField::TaxRate);
    result.amount_if_assessed_now=result.eligible ? multiply(static_cast<std::uint64_t>(result.occupants),static_cast<std::uint64_t>(result.rate)):0;
    result.paid_total=building(id).taxes_paid_total;
    const auto distance=original_tax_interval_ticks-ticks_%original_tax_interval_ticks;
    if (distance<=original::scheduler_max_tick-ticks_) result.next_assessment_tick=ticks_+distance;
    result.last_assessment=by_id(original_production_->finance.houses,id).last_assessment;
    return result;
}

OriginalGoalStatus World::original_goal_status() const {
    OriginalGoalStatus result;
    if (!original_production_enabled()) return result;
    result.applicable=true;result.population=total_population();
    result.positive_tax=taxes_collected_total_>0;result.nonnegative_funds=treasury_>=0;
    for (const auto& home:original_economy_->houses) if (home.house.occupants>0 && !building_on_fire(home.building_id)) {
        ++result.inhabited_houses;
        if (!original_elite_house_kind(building(home.building_id).kind) && home.house.type>=original::HouseType::PlainCottage)
            ++result.common_houses;
    }
    result.reached=result.inhabited_houses>=result.required_inhabited_houses &&
        result.common_houses>=result.required_common_houses && result.population>=result.required_population &&
        result.positive_tax && result.nonnegative_funds;
    return result;
}

} // namespace openemperor::simulation
