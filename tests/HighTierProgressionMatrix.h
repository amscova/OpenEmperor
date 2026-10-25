#pragma once

// On-demand QA observations only. No assignments to World, source parsing,
// clocks, routes or a second reachability/goal authority belong in this file.
#include "simulation/World.h"
#include "simulation/OriginalHousingDiagnosis.h"
#include "simulation/original/OriginalRulesModel.h"

#include <nlohmann/json.hpp>
#include <algorithm>
#include <array>
#include <set>
#include <stdexcept>

namespace oe_high_tier_matrix {
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
using Json=nlohmann::json;

inline void require(bool condition,const char* why) {
    if(!condition)throw std::runtime_error(why);
}
inline Json cell(sim::Cell value){return Json::array({value.x,value.y});}
inline Json target(std::optional<orig::HouseType> value) {
    return value?Json(static_cast<unsigned>(*value)):Json(nullptr);
}
inline Json state(const orig::HouseState& value) {
    return {{"type",static_cast<unsigned>(value.type)},{"desirability",value.desirability},
        {"occupants",value.occupants},{"devolve_counter",value.devolve_counter},
        {"water",value.water},{"herbalist",value.herbalist},{"acupuncture",value.acupuncture},
        {"music",value.music},{"acrobat",value.acrobat},{"drama",value.drama},
        {"ancestor",value.ancestor},{"confucian",value.confucian},{"daoist",value.daoist},
        {"buddhist",value.buddhist},{"food_quality",value.food_quality},{"hemp",value.hemp},
        {"ceramics",value.ceramics},{"tea",value.tea},{"silk",value.silk},
        {"bronzeware",value.bronzeware},{"lacquerware",value.lacquerware}};
}
inline Json requirements(const sim::OriginalHousingRowDiagnosis& value,
    const orig::HouseEvolutionRules& independent) {
    Json result=Json::array();
    for(const auto& need:value.requirements) {
        require(need.required==independent.value(value.row,need.field),"World matrix changed an independently prepared model requirement");
        result.push_back({{"field",sim::original_housing_requirement_name(need.field)},
            {"column",static_cast<unsigned>(need.field)},{"required",need.required},
            {"actual",need.actual},{"alternative",need.alternative},{"presence",need.presence},{"met",need.met}});
    }
    return result;
}
inline Json row(const sim::OriginalHousingRowDiagnosis& value,const orig::HouseState& actual,
    const orig::HouseEvolutionRules& independent) {
    auto hypothetical=actual;hypothetical.type=value.hypothetical_type;
    const auto next=orig::evolve_step(independent,hypothetical);
    require(value.needs_met==orig::needs_met(independent,value.row,hypothetical)&&
        value.verdict==orig::evolution_verdict(independent,hypothetical)&&
        value.pure_result_type==next.type&&value.pure_result_counter==next.devolve_counter,
        "World decision matrix differs from independent immutable HouseEvolutionRules");
    require(value.low==independent.value(value.row,orig::HouseField::DesirabilityLow)&&
        value.high==independent.value(value.row,orig::HouseField::DesirabilityHigh)&&
        value.row_capacity==independent.row_capacity(value.row),"World matrix changed a prepared DES/capacity value");
    return {{"row",static_cast<unsigned>(value.row)},{"hypothetical_type",static_cast<unsigned>(value.hypothetical_type)},
        {"same_family",value.same_family},{"low_inclusive_devolve",value.low},{"high_inclusive_evolve",value.high},
        {"capacity",value.row_capacity},{"requirements",requirements(value,independent)},
        {"needs_met",value.needs_met},{"pure_verdict",static_cast<int>(value.verdict)},
        {"pure_result_type",static_cast<unsigned>(next.type)},{"pure_result_counter",next.devolve_counter},
        {"hypothetical_only",true}};
}

// Enumerate shared construction identities; this is descriptive QA metadata,
// never an admission list. All placement still goes through World::execute.
inline Json source_catalog(const sim::World& world) {
    Json result=Json::array();
    for(unsigned c=0;c<=static_cast<unsigned>(sim::CommandType::PlaceOriginalElitePlot);++c) {
        const auto command=static_cast<sim::CommandType>(c);const auto kind=sim::placed_object(command);
        if(!kind)continue;
        auto model=sim::original_influence_model(*kind);
        if(!model)model=sim::original_industry_model(*kind);
        if(!model) {
            if(*kind==sim::Object::OriginalFoodFarm)model=orig::ModelBuilding::Farmhouse;
            else if(*kind==sim::Object::OriginalWheatField)model=orig::ModelBuilding::WheatField;
            else if(*kind==sim::Object::OriginalMilletField)model=orig::ModelBuilding::MilletField;
            else if(*kind==sim::Object::OriginalHempFarm)model=orig::ModelBuilding::HempFarm;
            else if(*kind==sim::Object::OriginalHempField)model=orig::ModelBuilding::HempField;
        }
        if(!model)continue;
        const auto footprint=sim::building_footprint(world.profile(),world.rule_version(),*kind);
        const auto& prepared=*world.original_rules_model();
        const auto workers=*kind==sim::Object::OriginalInfluenceSource?0:
            prepared.building_value(*model,orig::BuildingField::Employees);
        Json paid=Json::array();for(const auto& body:world.buildings())if(body.kind==*kind)
            paid.push_back({{"id",static_cast<unsigned>(body.id)},{"cell",cell(body.cell)},
                {"workers_required",world.workforce_required(body.id)},{"workers_assigned",world.workers_assigned(body.id)},
                {"operating",body.operating_enabled},{"burning",world.building_on_fire(body.id)}});
        result.push_back({{"command",c},{"object",static_cast<unsigned>(*kind)},
            {"model_id",orig::model_id(*model).value},{"model_label",orig::model_label(*model)},
            {"footprint",Json::array({footprint.width,footprint.height})},{"cost",world.construction_cost(command)},
            {"installed_workers",workers},{"upkeep_per_invoice",std::int64_t(workers)*sim::original_worker_upkeep_funds},
            {"upkeep_interval",world.original_worker_upkeep_interval()},
            {"des_source",sim::original_influence_model(*kind).has_value()},
            {"des",{{"base",prepared.building_value(*model,orig::BuildingField::Desirability)},
                {"step",prepared.building_value(*model,orig::BuildingField::DesirabilityStep)},
                {"delta",prepared.building_value(*model,orig::BuildingField::DesirabilityStepSize)},
                {"range",prepared.building_value(*model,orig::BuildingField::DesirabilityRange)}}},
            {"paid_bodies",paid},{"evidence","Shared placed_object, original_influence_model/original_industry_model, building_footprint and World construction/model authority; mapping alone proves no arrival or paid high-tier progression."}});
    }
    return result;
}

inline Json infrastructure_catalog(const sim::World& world) {
    Json result=Json::array();
    for(const auto command:{sim::CommandType::PlaceClaySource,sim::CommandType::PlacePottery,
        sim::CommandType::PlaceWarehouse,sim::CommandType::PlaceMarket,sim::CommandType::PlaceFireWatch,
        sim::CommandType::PlaceHousehold,sim::CommandType::PlaceOriginalElitePlot,sim::CommandType::PlaceRoad}) {
        const auto kind=*sim::placed_object(command);const auto side=sim::building_footprint(world.profile(),world.rule_version(),kind);
        int workers=0;
        switch(kind) {
        case sim::Object::ClaySource:workers=sim::Rules::clay_source_workers;break;
        case sim::Object::Pottery:workers=sim::Rules::pottery_workers;break;
        case sim::Object::Warehouse:workers=sim::Rules::warehouse_workers;break;
        case sim::Object::Market:workers=sim::Rules::market_workers;break;
        case sim::Object::FireWatch:workers=sim::Rules::fire_watch_workers;break;
        default:break;
        }
        for(const auto& paid:world.buildings())if(paid.kind==kind)
            require(workers==world.workforce_required(paid.id),"Infrastructure description differs from actual installed workforce");
        result.push_back({{"command",static_cast<unsigned>(command)},{"object",static_cast<unsigned>(kind)},
            {"cost",world.construction_cost(command)},{"footprint",Json::array({side.width,side.height})},
            {"workers",workers},{"upkeep",std::int64_t(workers)*sim::original_worker_upkeep_funds},
            {"evidence","World construction_cost and shared footprint/Rules worker constants; not a model-derived original source mapping."}});
    }
    return result;
}

inline Json supply_paths() {
    return Json::array({
        {{"need","Water"},{"classification","already supported"},{"path","Paid Well72 -> actual Water courier -> occupied House; purchase/proximity is not supply"},{"evidence","src/simulation/WorldOriginalWater.cpp"}},
        {{"need","Food quality"},{"classification","already supported"},{"path","Paid Farm193 + uniquely owned adjacent Field195/196 -> actual Market -> actual House; physical Wheat/Millet blend, empty stock resets quality"},{"evidence","src/simulation/WorldOriginalFood.cpp"}},
        {{"need","Hemp"},{"classification","already supported"},{"path","Paid Hemp192 + uniquely owned adjacent Field194 -> Warehouse -> Market -> House"},{"evidence","src/simulation/WorldOriginalHemp.cpp"}},
        {{"need","Ceramics"},{"classification","already supported"},{"path","Paid Clay source -> Pottery -> Warehouse (one Ceramics conversion) -> Market -> House"},{"evidence","src/simulation/WorldOriginalMarket.cpp"}},
        {{"need","Tea"},{"classification","already supported"},{"path","Paid Tea237 + separately paid owned adjacent Bush26 -> Warehouse -> Market -> House"},{"evidence","src/simulation/WorldOriginalIndustry.cpp"}},
        {{"need","Silk"},{"classification","already supported"},{"path","Paid Silkworm239 + separately paid owned adjacent Mulberry28 -> RawSilk -> Warehouse -> Weaver47 -> Warehouse -> Market -> House"},{"evidence","src/simulation/WorldOriginalIndustry.cpp"}},
        {{"need","Bronze OR Lacquer"},{"classification","already supported"},{"path","Logging38 Wood + managed internal Ore -> Smelter39 Bronze -> Warehouse -> Maker42 AND actual Clay; OR Refinery238+Tree27 Lacquer AND Logging38 Wood -> Maker44; final good -> Warehouse -> Market -> House"},{"evidence","src/simulation/WorldOriginalIndustry.cpp; needs accepts either but consumption independently debits both existing stocks"}},
        {{"need","Herbalist/Acupuncture/Music/Acrobat/Drama/Ancestor/Daoist/Buddhist"},{"classification","already supported"},{"path","Corresponding paid, operating, staffed, non-burning provider -> shared legal road route -> actual eligible House arrival -> own timer; no static radius coverage"},{"evidence","src/simulation/WorldOriginalServices.cpp"}},
        {{"need","Confucian"},{"classification","already supported"},{"path","Paid Hemp chain -> Warehouse -> PaperMaker45 consumes Hemp -> actual Paper Warehouse receipt -> actual Academy219 receipt -> debit1 Paper on successful dispatch -> actual occupied Elite arrival; Common ineligible"},{"evidence","src/simulation/WorldOriginalServices.cpp and WorldOriginalIndustry.cpp; Paper-only/debit1 are authored adapters"}},
        {{"need","DES"},{"classification","already supported"},{"path","Only actually admitted paid influence/service/Water bodies enter cached original prefix maps in ID order; max over actual House footprint. Entry packing feasibility remains unproven by this matrix."},{"evidence","src/simulation/WorldOriginalInfluence.cpp; full original source-manager lifecycle remains source mapping unverified"}}
    });
}

inline Json missing_requirements(const sim::OriginalHousingRowDiagnosis& row) {
    Json result=Json::array();for(const auto& need:row.requirements)if(!need.met)
        result.push_back({{"field",sim::original_housing_requirement_name(need.field)},
            {"required",need.required},{"actual",need.actual},{"alternative",need.alternative},
            {"presence",need.presence}});
    return result;
}

inline Json incremental_supply_estimate(const sim::World& world,orig::ModelHouse row,
    const orig::HouseEvolutionRules& independent,const Json& catalog,const Json& infrastructure) {
    using C=sim::CommandType;using F=orig::HouseField;
    std::set<C> purchases;
    const auto exists=[&](C command) {
        const auto kind=sim::placed_object(command);
        return kind&&std::any_of(world.buildings().begin(),world.buildings().end(),[&](const auto& body){return body.kind==*kind;});
    };
    const auto add=[&](std::initializer_list<C> commands){for(const auto command:commands)if(!exists(command))purchases.insert(command);};
    const auto wanted=[&](F field){return independent.value(row,field)!=0;};
    if(wanted(F::Water))add({C::PlaceOriginalWaterWell});
    if(wanted(F::Herbalist))add({C::PlaceHealthPost});
    if(wanted(F::Acupuncture))add({C::PlaceServicePost});
    if(wanted(F::Music))add({C::PlaceOriginalMusicSchool});
    if(wanted(F::Acrobat))add({C::PlaceOriginalAcrobatSchool});
    if(wanted(F::Drama))add({C::PlaceOriginalDramaSchool});
    if(wanted(F::Ancestor))add({C::PlaceOriginalAncestorShrine});
    if(wanted(F::DaoistOrBuddhist)&&!exists(C::PlaceOriginalDaoistShrine)&&!exists(C::PlaceOriginalBuddhistShrine))
        add({world.construction_cost(C::PlaceOriginalDaoistShrine)<=world.construction_cost(C::PlaceOriginalBuddhistShrine)?
            C::PlaceOriginalDaoistShrine:C::PlaceOriginalBuddhistShrine});
    if(wanted(F::FoodQuality))add({C::PlaceOriginalFoodFarm,C::PlaceOriginalMilletField,C::PlaceMarket});
    if(wanted(F::Hemp))add({C::PlaceOriginalHempFarm,C::PlaceOriginalHempField,C::PlaceWarehouse,C::PlaceMarket});
    if(wanted(F::Ceramics))add({C::PlaceClaySource,C::PlacePottery,C::PlaceWarehouse,C::PlaceMarket});
    if(wanted(F::Tea))add({C::PlaceOriginalTeaCuringShed,C::PlaceOriginalTeaBush,C::PlaceWarehouse,C::PlaceMarket});
    if(wanted(F::Silk))add({C::PlaceOriginalSilkwormShed,C::PlaceOriginalMulberryTree,C::PlaceOriginalWeaver,C::PlaceWarehouse,C::PlaceMarket});
    if(wanted(F::Confucian))add({C::PlaceOriginalConfucianAcademy,C::PlaceOriginalPaperMaker,
        C::PlaceOriginalHempFarm,C::PlaceOriginalHempField,C::PlaceWarehouse});
    std::string alternative="not required";
    if(wanted(F::BronzewareOrLacquerware)) {
        if(exists(C::PlaceOriginalBronzewareMaker)) {
            alternative="existing Bronzeware chain";
            add({C::PlaceOriginalLoggingShed,C::PlaceOriginalBronzeSmelter,C::PlaceClaySource,C::PlaceWarehouse,C::PlaceMarket});
        }else {
            alternative="Lacquerware chain (one legal OR option; route/capacity feasibility unproven)";
            add({C::PlaceOriginalLoggingShed,C::PlaceOriginalLacquerRefinery,C::PlaceOriginalLacquerTree,
                C::PlaceOriginalLacquerwareMaker,C::PlaceWarehouse,C::PlaceMarket});
        }
    }
    Json rows=Json::array();std::int64_t cost=0,workers=0;
    for(const auto command:purchases) {
        const auto index=static_cast<unsigned>(command);const Json* facts=nullptr;
        for(const auto& entry:catalog)if(entry.at("command")==index){facts=&entry;break;}
        if(!facts)for(const auto& entry:infrastructure)if(entry.at("command")==index){facts=&entry;break;}
        require(facts,"Incremental supply estimate lacks shared production command metadata");
        const auto needed=facts->contains("installed_workers")?facts->at("installed_workers").get<int>():facts->at("workers").get<int>();
        cost+=world.construction_cost(command);workers+=needed;
        rows.push_back({{"command",index},{"object",static_cast<unsigned>(*sim::placed_object(command))},
            {"cost",world.construction_cost(command)},{"installed_workers",needed},
            {"upkeep_per_invoice",std::int64_t(needed)*sim::original_worker_upkeep_funds},
            {"footprint",facts->at("footprint")}});
    }
    return {{"purchases",rows},{"purchase_cost",cost},{"incremental_installed_workers",workers},
        {"incremental_upkeep_per_invoice",workers*sim::original_worker_upkeep_funds},
        {"invoice_interval",world.original_worker_upkeep_interval()},{"bronze_lacquer_option",alternative},
        {"hypothetical",true},{"method","At most one absent paid source per required chain/type; shared commands counted once, existing bodies reused. No resource, staffing, transport, adjacency, cap or legal-site success is granted."},
        {"unestimated","Roads, additional throughput/duplicate providers, DES packing sources, FireWatch coverage and workforce availability require actual paid layout experiments; zero additional source workers does not imply sufficient delivery."}};
}

inline Json report(const sim::World& world,const orig::HouseEvolutionRules& independent) {
    require(world.original_sustainable_economy_enabled()&&world.original_rules_model(),"High-tier matrix requires an actual Rule12 World");
    require(independent.model_sha256()==world.original_rules_model()->provenance().sha256&&
        independent.difficulty()==world.original_rules_model()->difficulty(),"Independent matrix model identity or Difficulty differs");
    Json houses=Json::array(),types=Json::array();
    const auto catalog=source_catalog(world),infrastructure=infrastructure_catalog(world);
    for(const auto& home:world.original_economy_state()->houses) {
        const auto diagnosis=world.original_housing_diagnosis(home.building_id);
        const auto matrix=world.original_housing_decision_matrix(home.building_id);
        auto input=diagnosis.actual;input.desirability=static_cast<std::int8_t>(diagnosis.cached_desirability);
        const auto pure=orig::evolve_step(independent,input);
        require(diagnosis.actual==home.house&&diagnosis.current_row==orig::house_row(input.type)&&
            diagnosis.next_row==orig::next_row(input.type)&&diagnosis.pure_result_type==pure.type&&
            diagnosis.pure_result_counter==pure.devolve_counter,"Actual World diagnosis differs from independent type/row/evolution query");
        Json rows=Json::array();for(const auto& item:matrix)rows.push_back(row(item,input,independent));
        const auto& history=*std::find_if(world.original_production_state()->housing.houses.begin(),
            world.original_production_state()->housing.houses.end(),[&](const auto& value){return value.building_id==home.building_id;});
        Json transitions=Json::array();for(std::size_t r=0;r<history.first_transitions.size();++r)
            if(const auto& transition=history.first_transitions[r])transitions.push_back({{"type",r+3},
                {"tick",transition->evaluation.tick},{"kind",static_cast<unsigned>(transition->kind)},
                {"admitted",transition->admitted},{"evaluation_input",state(transition->evaluation.input)}});
        Json previous=nullptr;if(history.last_evaluation)previous={{"tick",history.last_evaluation->tick},
            {"input",state(history.last_evaluation->input)},
            {"meaning","Actual PRE-evolution/pre-consumption input retained by World; distinct from current post-tick state."}};
        Json services=Json::array();for(unsigned r=0;r<=static_cast<unsigned>(orig::ServiceRole::Buddhist);++r) {
            const auto role=static_cast<orig::ServiceRole>(r);const auto a=world.original_service_availability(home.building_id,role);
            services.push_back({{"role",r},{"eligible",a.eligible_target},{"timer",a.delivered_timer},
                {"providers",a.provider_count},{"operating_staffed_supplied",a.available_source_count},
                {"reachable",a.reachable_source_count},{"travelling_to_house",a.travelling_to_target_count},
                {"last_arrival",a.last_arrival_tick?Json(*a.last_arrival_tick):Json(nullptr)}});
        }
        const auto provenance=world.original_influence_provenance(home.building_id);Json sources=Json::array();
        for(const auto& source:provenance.sources)sources.push_back({{"id",static_cast<unsigned>(source.building_id)},
            {"type",static_cast<unsigned>(source.type)},{"origin",cell(source.origin)},
            {"footprint",Json::array({source.footprint.width,source.footprint.height})},
            {"before",source.before},{"contribution",source.contribution},{"after",source.after}});
        houses.push_back({{"id",static_cast<unsigned>(home.building_id)},{"cell",cell(world.building(home.building_id).cell)},
            {"actual_post_tick_state",state(home.house)},{"hypothetical_input_uses_cached_des",diagnosis.cached_desirability},
            {"elite_family",diagnosis.elite_family},{"actual_footprint_side",diagnosis.actual_footprint_side},
            {"original_required_side",diagnosis.original_required_side},{"awaiting_admission",diagnosis.awaiting_admission},
            {"materialization_pending",diagnosis.materialization_pending},{"materialization_ready",diagnosis.materialization_ready},
            {"materialization_blocker",static_cast<unsigned>(diagnosis.materialization_diagnostic.blocker)},
            {"materialization_blocked_cell",diagnosis.materialization_diagnostic.cell?cell(*diagnosis.materialization_diagnostic.cell):Json(nullptr)},
            {"admission_capacity",diagnosis.admission_capacity},{"census_capacity",diagnosis.census_capacity},
            {"current",row(diagnosis.current,input,independent)},{"next",row(diagnosis.next,input,independent)},
            {"matrix15",rows},{"last_actual_evaluation",previous},{"actual_first_transitions",transitions},
            {"actual_service_availability",services},{"des_selected_cell",cell(provenance.selected_cell)},
            {"actual_des_sources_in_id_order",sources},
            {"classification",diagnosis.materialization_pending&&!diagnosis.materialization_ready?"geometrically blocked":
                diagnosis.current.needs_met?"already supported":"supported but insufficient supply"},
            {"classification_scope","Current paid House only. Hypothetical rows are not actual progression witnesses."}});
    }
    for(unsigned t=2;t<=17;++t) {
        const auto type=static_cast<orig::HouseType>(t);const auto current=orig::house_row(type),next=orig::next_row(type);
        Json needs=Json::array(),next_needs=Json::array();
        // Reuse the real World matrix requirement identities without inventing
        // a second field/parser registry. A paid House is mandatory below.
        require(!world.original_economy_state()->houses.empty(),"Matrix requires at least one actually purchased House");
        const auto probe=world.original_housing_decision_matrix(world.original_economy_state()->houses.front().building_id);
        const auto& current_probe=probe[static_cast<unsigned>(current)];
        bool impossible=false;
        for(const auto& need:current_probe.requirements) {
            const auto required=independent.value(current,need.field),wanted=independent.value(next,need.field);
            needs.push_back({{"field",sim::original_housing_requirement_name(need.field)},{"column",static_cast<unsigned>(need.field)},
                {"required",required},{"presence",need.presence}});
            next_needs.push_back({{"field",sim::original_housing_requirement_name(need.field)},{"column",static_cast<unsigned>(need.field)},
                {"required",wanted},{"presence",need.presence}});
            if(!need.presence)impossible|=required>(need.field==orig::HouseField::FoodQuality?
                sim::original_food_batch_quality({0,1}).value:sim::Rules::household_capacity);
        }
        Json actual=Json::array();for(const auto& home:world.original_economy_state()->houses) {
            const auto& history=*std::find_if(world.original_production_state()->housing.houses.begin(),
                world.original_production_state()->housing.houses.end(),[&](const auto& h){return h.building_id==home.building_id;});
            if(t==2||t==11) {if(home.house.type==type)actual.push_back({{"house",static_cast<unsigned>(home.building_id)},
                {"kind","paid empty plot, no occupied-tier proof"}});}
            else if(const auto& witness=history.first_transitions[t-3])actual.push_back({{"house",static_cast<unsigned>(home.building_id)},
                {"tick",witness->evaluation.tick},{"kind",static_cast<unsigned>(witness->kind)}});
        }
        Json entry=nullptr;if((t>=4&&t<=10)||(t>=14&&t<=17))entry={
            {"from_type",t-1},{"minimum_des_inclusive",independent.value(orig::house_row(static_cast<orig::HouseType>(t-1)),orig::HouseField::DesirabilityHigh)},
            {"requires_current_and_next_needs",true}};
        Json observations=Json::array(),representative=nullptr,reasons=Json::array();
        int best_score=INT32_MAX;bool any_retaining_supplied=false,any_materialized_or_ready=false;
        for(const auto& home:world.original_economy_state()->houses) {
            if(sim::original_elite_house_kind(world.building(home.building_id).kind)!=orig::is_elite(type))continue;
            const auto diagnosis=world.original_housing_diagnosis(home.building_id);
            const auto matrix=world.original_housing_decision_matrix(home.building_id);
            const auto missing=missing_requirements(matrix[static_cast<unsigned>(current)]);
            const auto next_missing=missing_requirements(matrix[static_cast<unsigned>(next)]);
            const auto low=independent.value(current,orig::HouseField::DesirabilityLow);
            const bool occupied_stage=t!=2&&t!=11;
            const int retention_deficit=occupied_stage&&t!=12?std::max(0,low+1-diagnosis.cached_desirability):0;
            const int entry_deficit=entry.is_null()?0:std::max(0,
                entry.at("minimum_des_inclusive").get<int>()-diagnosis.cached_desirability);
            const bool blocked=t>=13&&diagnosis.materialization_pending&&!diagnosis.materialization_ready;
            any_materialized_or_ready|=!diagnosis.materialization_pending||diagnosis.materialization_ready;
            any_retaining_supplied|=missing.empty()&&!retention_deficit&&!blocked;
            Json observed={{"house",static_cast<unsigned>(home.building_id)},{"actual_type",static_cast<unsigned>(home.house.type)},
                {"occupants",home.house.occupants},{"cached_des",diagnosis.cached_desirability},
                {"projected_current_missing",missing},{"projected_next_missing",next_missing},
                {"entry_des_deficit",entry_deficit},{"retention_des_deficit",retention_deficit},
                {"materialization_blocked",blocked},{"hypothetical_projection",home.house.type!=type}};
            observations.push_back(observed);
            const auto score=static_cast<int>(missing.size())*1000+retention_deficit+entry_deficit+(blocked?100000:0);
            if(score<best_score){best_score=score;representative=std::move(observed);}
        }
        const bool occupied_witness=t!=2&&t!=11&&!actual.empty();
        if(actual.empty())reasons.push_back("No actual transition/admission or paid-plot witness for this type exists in this World.");
        if(observations.empty())reasons.push_back("No paid House of the required family exists in this World; no supply or geometry claim can be made.");
        else {
            if(!representative.at("projected_current_missing").empty())
                reasons.push_back("Representative same-family House lacks the listed current-stage needs; these are actual observed stocks/timers projected onto the target row.");
            if(representative.at("entry_des_deficit").get<int>()>0)
                reasons.push_back("Representative actual cached DES is below the previous row's entry threshold.");
            if(representative.at("retention_des_deficit").get<int>()>0)
                reasons.push_back("Representative actual cached DES does not exceed the target row's low retention boundary.");
            if(representative.at("materialization_blocked").get<bool>())
                reasons.push_back("Representative actual Elite plot cannot currently materialize its legal4x4 body.");
            if(!representative.at("projected_next_missing").empty())
                reasons.push_back("The separately listed next-stage needs are also missing; this is not itself a failure to retain the current stage.");
        }
        std::string classification="supported but insufficient supply";
        if(impossible)classification="model impossible under current caps";
        else if(t>=13&&!observations.empty()&&!any_materialized_or_ready)classification="geometrically blocked";
        else if((occupied_witness&&any_retaining_supplied)||((t==2||t==11)&&!actual.empty()))classification="already supported";
        const auto devolution=orig::devolve_target(type);const auto evolution=orig::evolve_target(type);
        types.push_back({{"type",t},{"name",sim::original_housing_type_name(type)},
            {"family",orig::is_elite(type)?"Elite":"Common"},{"model_row",static_cast<unsigned>(current)},
            {"next_row",static_cast<unsigned>(next)},{"vacant_aliases_shelter_row",t==2},
            {"low_inclusive_devolve",independent.value(current,orig::HouseField::DesirabilityLow)},
            {"high_inclusive_evolve",independent.value(current,orig::HouseField::DesirabilityHigh)},
            {"entry_from_previous",entry},{"requirements",needs},{"next_requirements",next_needs},
            {"row_capacity",independent.row_capacity(current)},
            {"admission_capacity",independent.row_capacity(orig::admission_capacity_row(type))},
            {"census_capacity",independent.row_capacity(orig::census_capacity_row(type))},
            {"tax_rate_per_actual_occupant",independent.value(current,orig::HouseField::TaxRate)},
            {"evolve_target",target(evolution)},{"devolve_target",target(devolution)},
            {"devolve_counter_limit",orig::devolve_limit(type)},
            {"consecutive_devolve_evaluations_to_change",devolution?Json(unsigned(orig::devolve_limit(type))+1):Json(nullptr)},
            {"actual_occupied_tier_witnessed",occupied_witness},{"actual_witnesses",actual},
            {"api_supported",true},{"classification",classification},{"observed_reasons",reasons},
            {"same_family_observations",observations},{"representative_observation",representative},
            {"incremental_supply_estimate",incremental_supply_estimate(world,current,independent,catalog,infrastructure)},
            {"next_stage_supply_estimate",evolution||t==11?incremental_supply_estimate(world,next,independent,catalog,infrastructure):Json(nullptr)},
            {"classification_scope","Only this paid World's actual transition witnesses and current same-family stock/timer/DES/geometry observations. Missing evidence is not global unreachability; hypothetical projections are not arrivals, admissions or evolved tiers."},
            {"physical_side",t==11?2:orig::is_elite(type)?4:2},
            {"lifecycle",t==11?"Purchased empty2x2; actual first admission must legally materialize4x4 and enter13":
                t==12?"Only actual13 devolution; retains4x4; recovery to13 checks its next needs, not DES":
                t==13?"Actual first Elite admission11->13 or actual12->13 recovery; never Common conversion":
                t==2?"Purchased empty2x2; first actual admission enters3":
                t==10||t==17?"Terminal within its separate family":"Ordinary same-family evolution/devolution"}});
    }
    return {{"format","openemperor-high-tier-matrix-v1"},{"tick",world.ticks()},
        {"rule",world.rule_version()},{"policy",world.map_permissions()->policy_version()},
        {"model_sha256",independent.model_sha256()},{"difficulty",static_cast<unsigned>(independent.difficulty())},
        {"independent_house_rules_match",true},{"original_parity","PARTIAL"},
        {"proof_scope","Diagnostic/model reachability matrix, not a successful paid high-tier run. No hypothetical row grants World state."},
        {"funds",world.treasury()},{"construction",world.construction_spent_total()},
        {"limits",{{"bodies",sim::Rules::original_production_building_limit},{"couriers",sim::Rules::original_production_courier_limit},
            {"houses",sim::Rules::city_v10_household_limit},{"food_farms",sim::original_food_farm_limit(world.profile(),world.rule_version())},
            {"house_goods_each",sim::Rules::household_capacity},{"max_physical_food_quality",sim::original_food_batch_quality({0,1}).value},
            {"upkeep_interval",world.original_worker_upkeep_interval()}}},
        {"source_catalog",catalog},{"infrastructure_catalog",infrastructure},
        {"supply_paths",supply_paths()},{"types16",types},{"houses",houses}};
}
} // namespace oe_high_tier_matrix
