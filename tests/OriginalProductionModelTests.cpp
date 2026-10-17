#include "OriginalProductionFixture.h"
#include "simulation/original/OriginalRulesModel.h"
#include "simulation/original/OriginalModelMapping.h"

#include <array>
#include <iostream>
#include <limits>

namespace {
namespace f=oe_influence_fixture;
namespace orig=openemperor::original;
using f::check;
constexpr std::array legacy{orig::ModelBuilding::ClayPit,orig::ModelBuilding::Kiln,
    orig::ModelBuilding::Warehouse,orig::ModelBuilding::CommonMarketSquare,
    orig::ModelBuilding::Well,orig::ModelBuilding::Farmhouse,orig::ModelBuilding::HerbalistsStall};
constexpr std::array extra{orig::ModelBuilding::HempFarm,orig::ModelBuilding::HempField};
template<class F> void refuses(F action) {
    bool denied=false;try {action();} catch (const std::exception&) {denied=true;}
    check(denied,"Active model admitted an absent/invalid explicit production capability");
}
std::shared_ptr<const orig::OriginalRulesModel> prepared(const std::filesystem::path& root,
        bool active,orig::Difficulty difficulty=orig::Difficulty::Normal) {
    auto input=orig::load_model_data(root);check(bool(input),"Model source did not parse");
    return active ? orig::OriginalRulesModel::prepare_production(std::move(*input.model),difficulty):
        orig::OriginalRulesModel::prepare(std::move(*input.model),difficulty);
}
void equivalent_legacy(const orig::OriginalRulesModel& old,const orig::OriginalRulesModel& active) {
    check(old.difficulty()==active.difficulty(),"Active preparation changed difficulty identity");
    for (const auto building:legacy) for (std::size_t field=0;field<orig::building_field_count;++field) {
        const auto column=static_cast<orig::BuildingField>(field);
        check(old.supports_building(building) && active.supports_building(building) &&
            old.building_value(building,column)==active.building_value(building,column),
            "Historical seven prepared building rows changed");
    }
    for (std::size_t row=0;row<orig::house_row_count;++row)
        for (std::size_t field=0;field<orig::house_field_count;++field)
            check(old.house_rules().value(static_cast<orig::ModelHouse>(row),static_cast<orig::HouseField>(field))==
                active.house_rules().value(static_cast<orig::ModelHouse>(row),static_cast<orig::HouseField>(field)),
                "Active production preparation changed House rules");
}
void synthetic() {
    for(const auto [object,model]:std::array{
            std::pair{f::sim::Object::OriginalHempFarm,orig::ModelBuilding::HempFarm},
            std::pair{f::sim::Object::OriginalHempField,orig::ModelBuilding::HempField}}) {
        const auto mapped=orig::functional_model_candidate(object);
        check(mapped.candidate==model && mapped.functional==orig::EvidenceStatus::AuthoredFallback &&
            mapped.mechanics_identity==orig::EvidenceStatus::AuthoredFallback &&
            mapped.presentation_identity==orig::EvidenceStatus::Unknown,
            "Typed Hemp identity was omitted or falsely advertised as original mechanics/art parity");
    }
    f::Root root;const auto path=root.data/"Model/EmperorBuildingModels.txt";
    f::write_model(root.data);auto old=prepared(root.data,false);
    for (const auto building:extra) {
        check(!old->supports_building(building),"Historical factory acquired active capabilities");
        refuses([&]{(void)old->building_value(building,orig::BuildingField::Cost);});
    }
    refuses([&]{(void)prepared(root.data,true);});
    oe_production_fixture::write_model(root.data);
    auto active=prepared(root.data,true);old=prepared(root.data,false);equivalent_legacy(*old,*active);
    check(old->provenance().sha256==active->provenance().sha256,"Explicit active factory changed source identity");
    check(active->building_value(extra[0],orig::BuildingField::Cost)==40 &&
        active->building_value(extra[0],orig::BuildingField::Employees)==18 &&
        active->building_value(extra[1],orig::BuildingField::Cost)==4 &&
        active->building_value(extra[1],orig::BuildingField::Employees)==0,
        "Authored named production values were not prepared");
    check(!active->supports_building(static_cast<orig::ModelBuilding>(4096)),"Undefined production model ID admitted");
    refuses([&]{(void)active->building_value(extra[0],static_cast<orig::BuildingField>(13));});
    const auto pristine=f::read_text(path);
    for (const auto building:extra) {
        auto malformed=pristine;const auto label=std::string(orig::model_label(building));
        const auto at=malformed.find(label);check(at!=std::string::npos,"Named authored production row missing");
        malformed.replace(at,label.size(),"Wrong producer");f::write_text(path,malformed);
        // Additional edited rows must not add dependencies to old sessions.
        equivalent_legacy(*old,*prepared(root.data,false));
        refuses([&]{(void)prepared(root.data,true);});
    }
    auto overflow=pristine;const std::string from="192,Hemp Farm,{,40,";
    const auto at=overflow.find(from);check(at!=std::string::npos,"Overflow control missing");
    overflow.replace(at,from.size(),"192,Hemp Farm,{,"+std::to_string(INT32_MAX)+',');
    f::write_text(path,overflow);check(bool(prepared(root.data,false)),"Old factory inspected new overflow row");
    refuses([&]{(void)prepared(root.data,true);});
    f::write_text(path,pristine);const auto identity=active->provenance().sha256;
    std::filesystem::remove(path);
    for (int i=0;i<1000;++i) check(active->supports_building(extra[0]) &&
        active->building_value(extra[0],orig::BuildingField::Employees)==18 && active->provenance().sha256==identity,
        "Prepared active queries re-read source or lost immutable values");
    refuses([&]{(void)prepared(root.data,true);});
}
int real(const std::filesystem::path& data) {
    const auto path=data/"Model/EmperorBuildingModels.txt";
    if (!std::filesystem::exists(path)) return 77;
    const auto before=f::read_text(path);const auto loaded=orig::load_model_data(data);check(bool(loaded),"Real model parse");
    for (std::size_t d=0;d<orig::difficulty_count;++d) {
        const auto difficulty=static_cast<orig::Difficulty>(d);
        const auto old=prepared(data,false,difficulty),active=prepared(data,true,difficulty);
        equivalent_legacy(*old,*active);
        check(old->provenance().sha256==active->provenance().sha256,"Real factory source identities differ");
        check(active->provenance().sha256=="7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6",
            "Real active model source fingerprint changed");
        for (const auto building:extra) for (std::size_t field=0;field<orig::building_field_count;++field) {
            const auto column=static_cast<orig::BuildingField>(field);
            check(active->supports_building(building) &&
                active->building_value(building,column)==orig::building_value(*loaded.model,building,column,difficulty),
                "Active prepared value differs from checked raw model/difficulty arithmetic");
        }
        const auto permissions=f::permissions();
        auto world=f::sim::World::create_original_production_development(permissions,active);
        const auto farm_cost=orig::building_value(*loaded.model,extra[0],orig::BuildingField::Cost,difficulty);
        const auto field_cost=orig::building_value(*loaded.model,extra[1],orig::BuildingField::Cost,difficulty);
        check(world.construction_cost(f::sim::CommandType::PlaceOriginalHempFarm)==farm_cost &&
            world.construction_cost(f::sim::CommandType::PlaceOriginalHempField)==field_cost,
            "Real difficulty-adjusted model prices did not reach paid World commands");
        f::buy(world,{f::sim::CommandType::PlaceOriginalHempFarm,{0,9}});
        f::sim::Command field{f::sim::CommandType::PlaceOriginalHempField,{2,9}};
        field.building_id=*world.building_owner_at({0,9});f::buy(world,field);
        check(world.construction_spent_total()==static_cast<std::uint64_t>(farm_cost+field_cost) &&
            world.treasury()==12000-farm_cost-field_cost,"Paid Farm/Field did not charge the selected difficulty");
        const auto saved=world.snapshot();
        check(f::sim::World::restore_original_production_development(saved,permissions,active).snapshot()==saved,
            "Real difficulty-adjusted Farm/Field finance did not restore exactly");
        auto bad=saved;++bad.construction_spent_total;--bad.treasury;
        refuses([&]{(void)f::sim::World::restore_original_production_development(bad,permissions,active);});
    }
    check(f::read_text(path)==before,"Real production model was modified");return 0;
}
}
int main(int argc,char** argv) {
    try {
        if (argc==3 && std::string_view(argv[1])=="--real") {const auto result=real(argv[2]);if (result)return result;}
        else {check(argc==1,"Unknown production model test arguments");synthetic();}
        std::cout<<"Explicit active model production preparation/old seven-row compatibility PASS\n";return 0;
    } catch (const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
}
