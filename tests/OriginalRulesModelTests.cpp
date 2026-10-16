#include "simulation/original/OriginalRulesModel.h"
#include "persistence/SandboxSave.h"

#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace {
namespace original=openemperor::original;
namespace sim=openemperor::simulation;
namespace save=openemperor::persistence;
namespace fs=std::filesystem;
using original::Difficulty;
constexpr std::array buildings{original::ModelBuilding::ClayPit,original::ModelBuilding::Kiln,
    original::ModelBuilding::Warehouse,original::ModelBuilding::CommonMarketSquare,
    original::ModelBuilding::Well,original::ModelBuilding::Farmhouse,original::ModelBuilding::HerbalistsStall};
constexpr std::array<std::string_view,5> difficulty_names{
    "Very Easy Modifiers","Easy Modifiers","Normal Modifier","Hard Modifiers","Very Hard Modifiers"};
std::size_t checks=0;
void check(bool condition,const char* message) { ++checks;if (!condition) throw std::runtime_error(message); }
template<class F> void refuses(F operation) {
    try { operation(); } catch (const std::exception&) { ++checks;return; }
    throw std::runtime_error("Invalid original model binding was admitted");
}
void write(const fs::path& path,const std::string& text) {
    fs::create_directories(path.parent_path());std::ofstream out(path,std::ios::binary);
    out.write(text.data(),static_cast<std::streamsize>(text.size()));check(out.good(),"Authored model write failed");
}
std::string bytes(const fs::path& path) {
    std::ifstream in(path,std::ios::binary);check(in.good(),"Missing authored test bytes");
    return {std::istreambuf_iterator<char>(in),{}};
}
struct Temp {
    fs::path path=fs::canonical(fs::temp_directory_path())/("oe-rules-binding-"+
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    ~Temp() { std::error_code ignored;fs::remove_all(path,ignored); }
};
template<class Value> std::string row(std::string name,std::size_t fields,Value value) {
    name+="{,";
    for (std::size_t field=0;field<fields;++field) name+=std::to_string(value(field))+',';
    return name+"},,,\n";
}
// Independent authored arithmetic, never original model table values.
std::string model_text(int offset=0,bool building_overflow=false,bool house_overflow=false) {
    std::string text="Authored immutable runtime model\nBUILDING MODS\n";
    for (std::size_t d=0;d<5;++d) text+=row(std::to_string(d)+','+std::string(difficulty_names[d])+',',13,
        [d](std::size_t) { return 100+static_cast<int>(d)*25; });
    text+="ALL BUILDINGS\n";
    for (std::size_t b=0;b<buildings.size();++b)
        text+=row(std::to_string(original::model_id(buildings[b]).value)+','+
            std::string(original::model_label(buildings[b]))+',',13,[=](std::size_t field) {
                return building_overflow && b==0 && field==0 ? std::numeric_limits<int>::max():
                    offset+10+static_cast<int>(b)+static_cast<int>(field);
            });
    text+="HOUSE MODS\n";
    for (std::size_t d=0;d<5;++d) text+=row(std::string(difficulty_names[d])+',',24,
        [d](std::size_t) { return static_cast<int>(d)-2; });
    text+="ALL HOUSES\n";
    for (unsigned h=0;h<15;++h) text+=row(std::string(original::model_label(static_cast<original::ModelHouse>(h)))+',',24,
        [=](std::size_t field) {
            return house_overflow && h==0 && field==0 ? std::numeric_limits<int>::max():
                offset+static_cast<int>(h)+static_cast<int>(field);
        });
    return text+"End of model data.\n";
}
std::shared_ptr<const sim::MapPermissions> policy() {
    return std::make_shared<const sim::MapPermissions>(6,6,3,
        std::vector<sim::MapCellPermission>(36,
            {true,true,false,0,sim::BuildBlocker::None,sim::BuildBlocker::None}),
        std::vector<sim::FixedGatePassage>{});
}
void synthetic() {
    Temp temp;const auto data=temp.path/"data";const auto source=data/"Model/EmperorBuildingModels.txt";
    const auto text=model_text();write(source,text);
    auto a=save::prepare_original_rules_model(data);
    auto b=save::prepare_original_rules_model(data);
    auto hard=save::prepare_original_rules_model(data,Difficulty::VeryHard);
    check(a && b && hard && a!=b && a!=hard,"Preparations unexpectedly share mutable global state");
    check(a->provenance().sha256==b->provenance().sha256 && a->difficulty()==Difficulty::Normal,
        "Repeated preparation lost source or difficulty identity");
    check(a->building_value(buildings[0],original::BuildingField::Cost)==15 &&
        hard->building_value(buildings[0],original::BuildingField::Cost)==20 &&
        a->house_rules().value(original::ModelHouse::Hut,original::HouseField::Capacity)==18 &&
        hard->house_rules().value(original::ModelHouse::Hut,original::HouseField::Capacity)==20,
        "Prepared authored arithmetic or difficulty changed");
    const auto fingerprint=original::inspect_model_source(data);
    check(fingerprint && fingerprint.fingerprint->sha256==a->provenance().sha256 &&
        fingerprint.fingerprint->byte_count==text.size(),"Bounded fingerprint differs from parsed model");
    const auto permissions=policy();
    auto world=sim::World::create_original_economy_development(permissions,a);
    check(world.original_rules_model()==a && world.original_economy_state()->model_source->sha256==
        a->provenance().sha256,"New World did not retain its explicit immutable dependency");
    auto copy=world;check(copy.original_rules_model()==a,"World copy duplicated the prepared model");
    const auto saved=world.snapshot();
    check(sim::World::restore_original_economy_development(saved,permissions,a).original_rules_model()==a,
        "Explicit restore did not retain the supplied prepared object");
    refuses([&] { (void)sim::World::restore_original_economy_development(saved,permissions); });
    refuses([&] { (void)sim::World::restore_original_economy_development(saved,permissions,hard); });
    auto invalid=saved;invalid.original_economy->model_source->sha256=std::string(64,'a');
    refuses([&] { (void)sim::World::restore_original_economy_development(invalid,permissions,a); });
    const auto unbound=sim::World::create_original_economy_development(permissions).snapshot();
    refuses([&] { (void)sim::World::restore_original_economy_development(unbound,permissions,a); });
    auto memory=original::parse_model_data(text);check(bool(memory),"Authored memory model failed grammar");
    refuses([&] { (void)original::OriginalRulesModel::prepare(*memory.model,Difficulty::Normal); });
    for (const auto bad:{std::size_t{5},std::numeric_limits<std::size_t>::max(),
            std::numeric_limits<std::size_t>::max()/2+1})
        refuses([&] { (void)save::prepare_original_rules_model(data,static_cast<Difficulty>(bad)); });
    refuses([&] { (void)a->building_value(static_cast<original::ModelBuilding>(4096),original::BuildingField::Cost); });
    refuses([&] { (void)a->building_value(buildings[0],static_cast<original::BuildingField>(13)); });
    write(source,model_text(0,true));
    refuses([&] { (void)save::prepare_original_rules_model(data,Difficulty::VeryHard); });
    write(source,model_text(0,false,true));
    refuses([&] { (void)save::prepare_original_rules_model(data,Difficulty::VeryHard); });
    auto malformed=text;const auto label=malformed.find("Clay Pit");check(label!=std::string::npos,"Missing authored row");
    malformed.replace(label,8,"Wrong ID");write(source,malformed);
    refuses([&] { (void)save::prepare_original_rules_model(data); });
    write(source,"malformed runtime source");check(bool(original::inspect_model_source(data)),
        "Fingerprint API incorrectly parses or grants semantic validity");
    refuses([&] { (void)save::prepare_original_rules_model(data); });
    write(source,model_text(7));auto other=save::prepare_original_rules_model(data);
    check(other->provenance().sha256!=a->provenance().sha256 &&
        other->building_value(buildings[0],original::BuildingField::Cost)==25 &&
        a->building_value(buildings[0],original::BuildingField::Cost)==15,
        "Failed or alternate preparations mutated an existing session's values");
    auto second=sim::World::create_original_economy_development(permissions,other);
    refuses([&] { (void)sim::World::restore_original_economy_development(saved,permissions,other); });
    fs::remove(source);
    for (unsigned tick=0;tick<20000;++tick) {
        world.tick();
        check(world.original_rules_model()==a &&
            a->house_rules().value(original::ModelHouse::Hut,original::HouseField::Capacity)==18,
            "Ticks or prepared queries reloaded, reparsed or replaced the missing source");
    }
    check(second.ticks()==0 && second.original_rules_model()==other && copy.ticks()==0,
        "Separate Worlds shared mutable simulation state");
    auto normalized=world.snapshot();normalized.ticks=0;check(normalized==saved,"Model binding activated economy state");
    refuses([&] { (void)save::prepare_original_rules_model(data); });
    sim::World legacy(6,6,std::vector<std::uint8_t>(36,1),sim::RulesProfile::HouseholdV3);
    check(!legacy.original_rules_model(),"Legacy World acquired a runtime model");
    write(data/"Cities/Authored.map","Independent legacy source without original model\n");
    const auto document=save::make_document(data,"Cities/Authored.map",std::vector<std::uint8_t>(36,1),legacy);
    const auto target=temp.path/"legacy.json";save::write_save(target,document,data,std::vector<std::uint8_t>(36,1));
    const auto before=bytes(target);write(source,"invalid model ignored by legacy");
    save::write_save(target,document,data,std::vector<std::uint8_t>(36,1));
    check(bytes(target)==before && save::restore_save(save::read_save(target),data,
        std::vector<std::uint8_t>(36,1)).snapshot()==legacy.snapshot(),
        "Legacy persistence introduced a model dependency or changed bytes");
}
int real(const fs::path& data) {
    if (!fs::exists(data/"Model/EmperorBuildingModels.txt") &&
        !fs::is_symlink(data/"Model/EmperorBuildingModels.txt")) {
        std::cout<<"Original model absent; explicit real preparation check SKIP\n";return 77;
    }
    const auto source=data/"Model/EmperorBuildingModels.txt";const auto before=bytes(source);
    const auto loaded=original::load_model_data(data);check(bool(loaded),"Present real model failed to parse");
    for (std::size_t d=0;d<5;++d) {
        const auto difficulty=static_cast<Difficulty>(d);const auto model=save::prepare_original_rules_model(data,difficulty);
        check(model->provenance().sha256=="7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6" &&
            model->difficulty()==difficulty,"Real runtime model lost its pinned source/difficulty");
        for (const auto building:buildings) for (std::size_t f=0;f<13;++f)
            check(model->building_value(building,static_cast<original::BuildingField>(f))==
                original::building_value(*loaded.model,building,static_cast<original::BuildingField>(f),difficulty),
                "Prepared real building value differs from checked reader");
        for (unsigned h=0;h<15;++h) for (std::size_t f=0;f<24;++f)
            check(model->house_rules().value(static_cast<original::ModelHouse>(h),static_cast<original::HouseField>(f))==
                original::house_value(*loaded.model,static_cast<original::ModelHouse>(h),static_cast<original::HouseField>(f),difficulty),
                "Prepared real house value differs from checked reader");
    }
    check(bytes(source)==before,"Read-only real source changed during preparation");return 0;
}
}
int main(int argc,char** argv) {
    try {
        static_assert(!std::is_default_constructible_v<original::OriginalRulesModel>);
        if (argc==3 && std::string_view(argv[1])=="--real") {
            const auto result=real(argv[2]);if (result) return result;
        } else { check(argc==1,"Unknown rules model test arguments");synthetic(); }
        std::cout<<"Immutable OriginalRulesModel PASS: "<<checks<<" checks\n";return 0;
    } catch (const std::exception& error) {
        std::cerr<<"OriginalRulesModel FAIL: "<<error.what()<<'\n';return 1;
    }
}
