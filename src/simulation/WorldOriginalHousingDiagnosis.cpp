#include "simulation/World.h"
#include "simulation/OriginalHousingDiagnosis.h"
#include "simulation/original/OriginalRulesModel.h"

#include <algorithm>
#include <stdexcept>

namespace openemperor::simulation {
namespace {
using Field=original::HouseField;
constexpr std::array<Field,15> fields{
    Field::Water,Field::Herbalist,Field::Acupuncture,Field::Music,Field::Acrobat,
    Field::Drama,Field::Ancestor,Field::Confucian,Field::DaoistOrBuddhist,
    Field::FoodQuality,Field::Hemp,Field::Ceramics,Field::Tea,Field::Silk,
    Field::BronzewareOrLacquerware};

OriginalHousingRequirement requirement(const original::HouseEvolutionRules& rules,
    original::ModelHouse row,const original::HouseState& state,Field field) {
    OriginalHousingRequirement result;result.field=field;result.required=rules.value(row,field);
    switch (field) {
    case Field::Water:result.actual=state.water;break;
    case Field::Herbalist:result.actual=state.herbalist;break;
    case Field::Acupuncture:result.actual=state.acupuncture;break;
    case Field::Music:result.actual=state.music;break;
    case Field::Acrobat:result.actual=state.acrobat;break;
    case Field::Drama:result.actual=state.drama;break;
    case Field::Ancestor:result.actual=state.ancestor;break;
    case Field::Confucian:result.actual=state.confucian;break;
    case Field::DaoistOrBuddhist:result.actual=state.daoist;result.alternative=state.buddhist;break;
    case Field::FoodQuality:result.actual=state.food_quality;break;
    case Field::Hemp:result.actual=state.hemp;break;
    case Field::Ceramics:result.actual=state.ceramics;break;
    case Field::Tea:result.actual=state.tea;break;
    case Field::Silk:result.actual=state.silk;break;
    case Field::BronzewareOrLacquerware:result.actual=state.bronzeware;result.alternative=state.lacquerware;break;
    default:throw std::logic_error("unsupported Housing diagnostic requirement");
    }
    result.presence=field!=Field::FoodQuality && field!=Field::Hemp && field!=Field::Ceramics &&
        field!=Field::Tea && field!=Field::Silk && field!=Field::BronzewareOrLacquerware;
    const auto actual=std::max(result.actual,result.alternative);
    result.met=result.presence ? result.required==0 || actual!=0:actual>=result.required;
    return result;
}

OriginalHousingRowDiagnosis row_diagnosis(const original::HouseEvolutionRules& rules,
    original::ModelHouse row,const original::HouseState& actual,bool elite_family) {
    OriginalHousingRowDiagnosis result;result.row=row;
    result.hypothetical_type=static_cast<original::HouseType>(static_cast<unsigned>(row)+3U);
    result.low=rules.value(row,Field::DesirabilityLow);result.high=rules.value(row,Field::DesirabilityHigh);
    result.row_capacity=rules.row_capacity(row);result.same_family=original::is_elite(result.hypothetical_type)==elite_family;
    for (std::size_t i=0;i<fields.size();++i) result.requirements[i]=requirement(rules,row,actual,fields[i]);
    result.needs_met=std::all_of(result.requirements.begin(),result.requirements.end(),[](const auto& value){return value.met;});
    if (result.needs_met!=original::needs_met(rules,row,actual)) throw std::logic_error("Housing diagnostic differs from pure needs");
    auto hypothetical=actual;hypothetical.type=result.hypothetical_type;
    result.verdict=original::evolution_verdict(rules,hypothetical);
    const auto evolved=original::evolve_step(rules,hypothetical);
    result.pure_result_type=evolved.type;result.pure_result_counter=evolved.devolve_counter;
    return result;
}
}

OriginalHousingDiagnosis World::original_housing_diagnosis(BuildingId id) const {
    if (!original_production_enabled() || !original_rules_model_ || !original_economy_)
        throw std::invalid_argument("Housing diagnosis requires active original production");
    const auto found=std::find_if(original_economy_->houses.begin(),original_economy_->houses.end(),
        [&](const auto& home){return home.building_id==id;});
    if (found==original_economy_->houses.end() || !original_house_kind(building(id).kind))
        throw std::invalid_argument("Housing diagnosis requires an actual paid House");
    const auto& rules=original_rules_model_->house_rules();OriginalHousingDiagnosis result;
    result.building_id=id;result.actual=found->house;result.elite_family=original_elite_house_kind(building(id).kind);
    result.actual_footprint_side=static_cast<std::uint8_t>(building_footprint(profile_,rule_version_,building(id).kind).width);
    result.cached_desirability=original_influence_value_at(building(id).cell,result.actual_footprint_side);
    result.current_row=original::house_row(result.actual.type);result.next_row=original::next_row(result.actual.type);
    auto input=result.actual;input.desirability=static_cast<std::int8_t>(result.cached_desirability);
    result.current=row_diagnosis(rules,result.current_row,input,result.elite_family);
    result.next=row_diagnosis(rules,result.next_row,input,result.elite_family);
    result.admission_capacity=rules.row_capacity(original::admission_capacity_row(result.actual.type));
    result.census_capacity=rules.row_capacity(original::census_capacity_row(result.actual.type));
    const auto evolved=original::evolve_step(rules,input);
    result.pure_result_type=evolved.type;result.pure_result_counter=evolved.devolve_counter;
    result.devolve_limit=original::devolve_limit(result.actual.type);
    result.awaiting_admission=result.actual.occupants==0;
    result.common_terminal=result.actual.type==original::HouseType::LuxuriousApartment;
    result.elite_terminal=result.actual.type==original::HouseType::HeavenlyCompound;
    result.original_required_side=result.elite_family ? std::uint8_t{4}:std::uint8_t{2};
    result.geometry_authored_2x2=result.actual_footprint_side==2;
    result.geometry_deferred=result.elite_family && result.actual_footprint_side!=4;
    if (result.elite_family) {
        const auto status=original_elite_materialization_status(id);
        result.materialization_pending=status.pending;result.materialization_ready=status.ready;
        result.materialization_diagnostic=status.diagnostic;
    }
    return result;
}

std::array<OriginalHousingRowDiagnosis,15> World::original_housing_decision_matrix(BuildingId id) const {
    const auto diagnosis=original_housing_diagnosis(id);auto input=diagnosis.actual;
    input.desirability=static_cast<std::int8_t>(diagnosis.cached_desirability);
    std::array<OriginalHousingRowDiagnosis,15> result{};
    for (std::size_t i=0;i<result.size();++i)
        result[i]=row_diagnosis(original_rules_model_->house_rules(),static_cast<original::ModelHouse>(i),input,diagnosis.elite_family);
    return result;
}

std::string_view original_housing_type_name(original::HouseType type) {
    constexpr std::array<std::string_view,16> names{"Vacant","Shelter","Hut","Plain Cottage","Attractive Cottage",
        "Spacious Dwelling","Elegant Dwelling","Ornate Apartment","Luxurious Apartment","Elite Unoccupied",
        "Elite Abandoned","Modest Siheyuan","Lavish Siheyuan","Humble Compound","Impressive Compound","Heavenly Compound"};
    if (!original::is_house_type(type)) throw std::out_of_range("invalid Housing diagnostic type");
    return names[static_cast<std::size_t>(type)-2U];
}

std::string_view original_housing_requirement_name(Field field) {
    switch (field) {
    case Field::Water:return "Water";case Field::Herbalist:return "Herbalist";
    case Field::Acupuncture:return "Acupuncture";case Field::Music:return "Music";
    case Field::Acrobat:return "Acrobat";case Field::Drama:return "Drama";
    case Field::Ancestor:return "Ancestor";case Field::Confucian:return "Confucian";
    case Field::DaoistOrBuddhist:return "Daoist OR Buddhist";case Field::FoodQuality:return "Food quality";
    case Field::Hemp:return "Hemp";case Field::Ceramics:return "Ceramics";case Field::Tea:return "Tea";
    case Field::Silk:return "Silk";case Field::BronzewareOrLacquerware:return "Bronze OR Lacquer";
    default:throw std::out_of_range("invalid Housing diagnostic requirement");
    }
}
} // namespace openemperor::simulation
