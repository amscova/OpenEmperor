#pragma once

// Authored inputs for the active Rule10 working generation only. Historical
// seven-row fixtures and their source/golden bytes remain unchanged.
#include "OriginalInfluenceFixture.h"

#include <array>
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace oe_production_fixture {
// Explicit authored model editing for independent scenario requirements. The
// caller changes inputs before preparation; this never edits a live World.
inline void set_house_field(std::string& text,std::size_t row,unsigned field,int value) {
    namespace original=openemperor::original;
    oe_influence_fixture::check(row<original::house_row_count && field<original::house_field_count,
                                "Authored housing field outside typed model domain");
    const std::string marker=std::string(original::model_label(static_cast<original::ModelHouse>(row)))+",{,";
    auto at=text.find(marker);
    oe_influence_fixture::check(at!=std::string::npos,"Authored housing model row missing");
    at+=marker.size();
    for (unsigned i=0;i<field;++i) {
        at=text.find(',',at);
        oe_influence_fixture::check(at!=std::string::npos,"Authored housing field missing");++at;
    }
    const auto end=text.find(',',at);
    oe_influence_fixture::check(end!=std::string::npos,"Authored housing field value missing");
    text.replace(at,end-at,std::to_string(value));
}
inline std::string authored_model_text(int shelter_capacity=37,int hemp_need=0,int ceramics_need=0,int food_need=0,
                                       int herbalist_need=0,int acupuncture_need=0,int ancestor_need=0,
                                       int hut_capacity=14,int plain_capacity=22,int plain_hemp_need=0,int plain_ceramics_need=0,
                                       int plain_confucian_need=0,int plain_dao_or_buddhist_need=0,
                                       int confucian_need=0,int dao_or_buddhist_need=0,
                                       int plain_music_need=0,int plain_acrobat_need=0,int plain_drama_need=0,
                                       int music_need=0,int acrobat_need=0,int drama_need=0,
                                       int tea_need=0,int silk_need=0,int bronze_or_lacquer_need=0,
                                       int plain_tea_need=0,int plain_silk_need=0,int plain_bronze_or_lacquer_need=0) {
    auto text=oe_influence_fixture::authored_model_text();
    std::string before="1: Shelter,{,",after=before;
    for (int i=0;i<17;++i) {
        before+="0,";
        after+=std::to_string(i==0 ? -99:i==2 ? herbalist_need:i==4 ? acupuncture_need:
                             i==5 ? music_need:i==6 ? acrobat_need:i==7 ? drama_need:
                             i==8 ? food_need:i==9 ? hemp_need:i==10 ? ceramics_need:
                             i==11 ? tea_need:i==12 ? silk_need:i==13 ? bronze_or_lacquer_need:0)+',';
    }
    before+="0,";after+=std::to_string(shelter_capacity)+',';
    for (int i=18;i<24;++i) {
        before+="0,";after+=std::to_string(i==21 ? ancestor_need:i==22 ? confucian_need:i==23 ? dao_or_buddhist_need:0)+',';
    }
    const auto capacity=text.find(before);
    oe_influence_fixture::check(capacity!=std::string::npos,"Authored production Shelter row missing");
    text.replace(capacity,before.size(),after);
    for (const auto& [label,cap]:std::array{std::pair{"2: Hut",hut_capacity},std::pair{"3: Plain Cottage",plain_capacity}}) {
        const bool plain=std::string_view(label)=="3: Plain Cottage";
        std::string empty=std::string(label)+",{,",prepared=empty;
        for (int i=0;i<24;++i) {
            empty+="0,";
            prepared+=std::to_string(i==0 ? -99:i==3 ? 1:i==17 ? cap:
                plain && i==8 ? 20:plain && i==9 ? plain_hemp_need:
                plain && i==10 ? plain_ceramics_need:plain && i==11 ? plain_tea_need:
                plain && i==12 ? plain_silk_need:plain && i==13 ? plain_bronze_or_lacquer_need:plain && i==5 ? plain_music_need:
                plain && i==6 ? plain_acrobat_need:plain && i==7 ? plain_drama_need:plain && i==21 ? 1:
                plain && i==22 ? plain_confucian_need:plain && i==23 ? plain_dao_or_buddhist_need:0)+',';
        }
        const auto at=text.find(empty);
        oe_influence_fixture::check(at!=std::string::npos,"Authored production housing row missing");
        text.replace(at,empty.size(),prepared);
    }
    // The active generation prepares all fifteen rows. These deliberately
    // authored defaults retain the early focused scenarios while making every
    // later capacity readable; EliteUnoccupied's literal row remains zero.
    set_house_field(text,2,1,100);
    constexpr std::array<int,12> later_capacities{31,41,52,63,74,0,1,5,10,15,20,25};
    for (std::size_t i=0;i<later_capacities.size();++i) {
        set_house_field(text,i+3,0,-99);
        set_house_field(text,i+3,1,100);
        set_house_field(text,i+3,17,later_capacities[i]);
    }
    set_house_field(text,10,9,1);
    set_house_field(text,10,10,1);
    const std::string old_well="72,Well,{,0,12,2,-2,5,0,0,0,0,0,0,0,0,},,,\n";
    const auto well=text.find(old_well);
    oe_influence_fixture::check(well!=std::string::npos,"Authored production Well row missing");
    text.replace(well,old_well.size(),"72,Well,{,25,12,2,-2,5,4,0,0,0,0,0,0,0,},,,\n");
    const std::string old_farm="193,Farmhouse,{,0,0,0,0,0,0,0,0,0,0,0,0,0,},,,\n";
    const auto farm=text.find(old_farm);
    oe_influence_fixture::check(farm!=std::string::npos,"Authored Food Farmhouse row missing");
    text.replace(farm,old_farm.size(),"193,Farmhouse,{,80,0,0,0,0,12,0,0,0,0,0,0,0,},,,\n");
    const std::string old_herbalist="207,Herbalist's Stall,{,0,0,0,0,0,0,0,0,0,0,0,0,0,},,,\n";
    const auto herbalist=text.find(old_herbalist);
    oe_influence_fixture::check(herbalist!=std::string::npos,"Authored Herbalist row missing");
    text.replace(herbalist,old_herbalist.size(),"207,Herbalist's Stall,{,30,0,0,0,0,7,0,0,0,0,0,0,0,},,,\n");
    const auto section=text.find("HOUSE MODS\n");
    oe_influence_fixture::check(section!=std::string::npos,"Authored production model section missing");
    text.insert(section,
        "192,Hemp Farm,{,40,0,0,0,0,18,0,0,0,0,0,0,0,},,,\n"
        "194,Hemp Field,{,4,0,0,0,0,0,0,0,0,0,0,0,0,},,,\n"
        "195,Wheat Field,{,4,0,0,0,0,0,0,0,0,0,0,0,0,},,,\n"
        "196,Millet Field,{,4,0,0,0,0,0,0,0,0,0,0,0,0,},,,\n"
        "208,Acupuncturist's Clinic,{,50,0,0,0,0,8,0,0,0,0,0,0,0,},,,\n"
        "214,Ancestral Shrine,{,40,0,0,0,0,4,0,0,0,0,0,0,0,},,,\n"
        "215,Daoist Shrine,{,45,0,0,0,0,4,0,0,0,0,0,0,0,},,,\n"
        "217,Buddhist Shrine,{,45,0,0,0,0,4,0,0,0,0,0,0,0,},,,\n"
        "219,Confucian Academny,{,80,0,0,0,0,12,0,0,0,0,0,0,0,},,,\n"
        "211,Music  School,{,50,0,0,0,0,8,0,0,0,0,0,0,0,},,,\n"
        "212,Acrobat School,{,60,0,0,0,0,9,0,0,0,0,0,0,0,},,,\n"
        "213,Drama School,{,70,0,0,0,0,10,0,0,0,0,0,0,0,},,,\n"
        "38,Logging Shed,{,70,0,0,0,0,14,0,0,0,0,0,0,0,},,,\n"
        "39,Bronze Smelter,{,80,0,0,0,0,19,0,0,0,0,0,0,0,},,,\n"
        "42,Bronzeware Maker,{,60,0,0,0,0,12,0,0,0,0,0,0,0,},,,\n"
        "44,Lacquerware Maker,{,65,0,0,0,0,11,0,0,0,0,0,0,0,},,,\n"
        "45,Paper Maker,{,70,0,0,0,0,10,0,0,0,0,0,0,0,},,,\n"
        "47,Weaver,{,60,0,0,0,0,11,0,0,0,0,0,0,0,},,,\n"
        "237,Tea Curing Shed,{,40,0,0,0,0,12,0,0,0,0,0,0,0,},,,\n"
        "238,Lacquer Refinery,{,40,0,0,0,0,12,0,0,0,0,0,0,0,},,,\n"
        "239,Silkworm Shed,{,40,0,0,0,0,12,0,0,0,0,0,0,0,},,,\n"
        "26,Tea Bush,{,6,0,0,0,0,0,0,0,0,0,0,0,0,},,,\n"
        "27,Lacquer Tree,{,5,0,0,0,0,0,0,0,0,0,0,0,0,},,,\n"
        "28,Mulberry Tree,{,5,0,0,0,0,0,0,0,0,0,0,0,0,},,,\n");
    return text;
}
inline void write_model(const std::filesystem::path& root,int shelter_capacity=37,int hemp_need=0,int ceramics_need=0,int food_need=0,
                        int herbalist_need=0,int acupuncture_need=0,int ancestor_need=0,
                        int hut_capacity=14,int plain_capacity=22,int plain_hemp_need=0,int plain_ceramics_need=0,
                        int plain_confucian_need=0,int plain_dao_or_buddhist_need=0,
                        int confucian_need=0,int dao_or_buddhist_need=0,
                        int plain_music_need=0,int plain_acrobat_need=0,int plain_drama_need=0,
                        int music_need=0,int acrobat_need=0,int drama_need=0,
                        int tea_need=0,int silk_need=0,int bronze_or_lacquer_need=0,
                        int plain_tea_need=0,int plain_silk_need=0,int plain_bronze_or_lacquer_need=0) {
    std::filesystem::create_directories(root/"Model");
    oe_influence_fixture::write_text(root/"Model/EmperorBuildingModels.txt",
        authored_model_text(shelter_capacity,hemp_need,ceramics_need,food_need,herbalist_need,acupuncture_need,ancestor_need,
                            hut_capacity,plain_capacity,plain_hemp_need,plain_ceramics_need,
                            plain_confucian_need,plain_dao_or_buddhist_need,confucian_need,dao_or_buddhist_need,
                            plain_music_need,plain_acrobat_need,plain_drama_need,music_need,acrobat_need,drama_need,
                            tea_need,silk_need,bronze_or_lacquer_need,plain_tea_need,plain_silk_need,plain_bronze_or_lacquer_need));
}
// An explicit paid layout helper, never an inventory/population grant or clock.
struct PaperSupplyLayout {
    openemperor::simulation::Cell farm{},field{},paper{},warehouse{};
    std::vector<openemperor::simulation::Cell> roads,extra_homes;
    std::optional<openemperor::simulation::BuildingId> existing_farm,existing_warehouse;
};
struct PaperSupply {
    openemperor::simulation::BuildingId farm{},field{},paper{},warehouse{};
    std::uint64_t spent=0;
};
inline PaperSupply add_paper_supply(openemperor::simulation::World& world,const PaperSupplyLayout& layout) {
    namespace sim=openemperor::simulation;
    const auto before=world.construction_spent_total();
    for (const auto cell:layout.roads)
        if (world.object_at(cell)!=sim::Object::Road) oe_influence_fixture::buy(world,{sim::CommandType::PlaceRoad,cell});
    for (const auto cell:layout.extra_homes) oe_influence_fixture::buy(world,{sim::CommandType::PlaceHousehold,cell});
    PaperSupply result;
    const auto purchase=[&](sim::CommandType type,sim::Cell cell) {
        oe_influence_fixture::buy(world,{type,cell});
        const auto owner=world.building_owner_at(cell);
        oe_influence_fixture::check(owner.has_value(),"Paid Paper supply purchase lost its body");return *owner;
    };
    if (layout.existing_farm) {
        result.farm=*layout.existing_farm;
        oe_influence_fixture::check(world.building(result.farm).kind==sim::Object::OriginalHempFarm,
                                    "Paper supply reused a non-Hemp Farm");
        const auto& farms=world.original_production_state()->hemp_farms;
        const auto row=std::find_if(farms.begin(),farms.end(),[&](const auto& value){return value.farm_id==result.farm;});
        oe_influence_fixture::check(row!=farms.end() && row->field_id.has_value(),"Reused Paper Hemp Farm lacks its paid field");
        result.field=*row->field_id;
    } else {
        result.farm=purchase(sim::CommandType::PlaceOriginalHempFarm,layout.farm);
        sim::Command field{sim::CommandType::PlaceOriginalHempField,layout.field};field.building_id=result.farm;
        oe_influence_fixture::buy(world,field);result.field=*world.building_owner_at(layout.field);
    }
    result.paper=purchase(sim::CommandType::PlaceOriginalPaperMaker,layout.paper);
    if (layout.existing_warehouse) {
        result.warehouse=*layout.existing_warehouse;
        oe_influence_fixture::check(world.building(result.warehouse).kind==sim::Object::Warehouse,
                                    "Paper supply reused a non-Warehouse body");
    } else result.warehouse=purchase(sim::CommandType::PlaceWarehouse,layout.warehouse);
    result.spent=world.construction_spent_total()-before;return result;
}

// Shared paid OE25 layout. All actors, stock and residents originate in normal
// commands/ticks. Keeping material producers paused initially permits a real
// Modest13 shortage/devolution before the ordinary supply chain is resumed.
struct HousingStagesScene {
    std::vector<openemperor::simulation::BuildingId> common_houses;
    openemperor::simulation::BuildingId elite{},water{},ancestor{},market{},clay{},kiln{},warehouse{},
        academy{},food{},food_field{},hemp{},hemp_field{},paper{};
    std::vector<openemperor::simulation::BuildingId> material_sources;
    std::uint64_t spent=0;
};
inline HousingStagesScene add_housing_stages_scene(openemperor::simulation::World& world,bool pause_material=true) {
    namespace sim=openemperor::simulation;
    HousingStagesScene result;
    const auto before=world.construction_spent_total();
    const auto purchase=[&](sim::CommandType type,sim::Cell cell) {
        oe_influence_fixture::buy(world,{type,cell});
        return *world.building_owner_at(cell);
    };
    for (const int y:{4,9}) for (int x=0;x<24;++x)
        oe_influence_fixture::buy(world,{sim::CommandType::PlaceRoad,{x,y}});
    for (int y=5;y<9;++y) oe_influence_fixture::buy(world,{sim::CommandType::PlaceRoad,{20,y}});
    for (int x=0;x<24;x+=3) result.common_houses.push_back(purchase(sim::CommandType::PlaceHousehold,{x,2}));
    result.elite=purchase(sim::CommandType::PlaceOriginalElitePlot,{0,5});
    result.water=purchase(sim::CommandType::PlaceOriginalWaterWell,{3,5});
    result.ancestor=purchase(sim::CommandType::PlaceOriginalAncestorShrine,{6,5});
    result.market=purchase(sim::CommandType::PlaceMarket,{9,5});
    result.clay=purchase(sim::CommandType::PlaceClaySource,{12,5});
    result.kiln=purchase(sim::CommandType::PlacePottery,{15,5});
    result.warehouse=purchase(sim::CommandType::PlaceWarehouse,{18,5});
    result.academy=purchase(sim::CommandType::PlaceOriginalConfucianAcademy,{21,5});
    result.food=purchase(sim::CommandType::PlaceOriginalFoodFarm,{0,10});
    sim::Command field{sim::CommandType::PlaceOriginalMilletField,{3,10}};field.building_id=result.food;
    oe_influence_fixture::buy(world,field);result.food_field=*world.building_owner_at(field.cell);
    result.hemp=purchase(sim::CommandType::PlaceOriginalHempFarm,{5,10});
    field={sim::CommandType::PlaceOriginalHempField,{7,10}};field.building_id=result.hemp;
    oe_influence_fixture::buy(world,field);result.hemp_field=*world.building_owner_at(field.cell);
    result.paper=purchase(sim::CommandType::PlaceOriginalPaperMaker,{9,10});
    result.material_sources={result.clay,result.kiln,result.hemp,result.paper,result.food,result.academy};
    if (pause_material) for (const auto id:result.material_sources)
        oe_influence_fixture::buy(world,sim::set_building_operation(id,false));
    result.spent=world.construction_spent_total()-before;
    return result;
}
} // namespace oe_production_fixture
