#pragma once
// Entirely authored model text; no copied original table values or assets.
#include <array>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace oe_save_fixture {
inline std::string historical_syntactic_model_text() {
    constexpr std::array<std::string_view,5> difficulties{
        "Very Easy Modifiers","Easy Modifiers","Normal Modifier","Hard Modifiers","Very Hard Modifiers"};
    const auto row=[](std::string prefix,std::size_t fields,bool closed) {
        prefix+="{,";
        for (std::size_t i=0;i<fields;++i) { if (i) prefix+=',';prefix+='0'; }
        prefix+=closed?",},,,\n":",,,,,,,\n";
        return prefix;
    };
    std::string text="Authored schema23 source identity fixture\nModel data starts here:-\nBUILDING MODS\n";
    for (std::size_t i=0;i<difficulties.size();++i)
        text+=row(std::to_string(i)+','+std::string(difficulties[i])+',',13,true);
    text+="ALL BUILDINGS\n";
    text+=row("35,Authored test building,",13,true);
    text+="HOUSE MODS\n";
    for (const auto name:difficulties) text+=row(std::string(name)+',',24,false);
    text+="ALL HOUSES\n";
    for (std::size_t i=0;i<15;++i) text+=row("Authored house "+std::to_string(i)+',',24,false);
    text+="End of model data.\n";
    return text;
}
inline std::string authored_model_text(int variant=0) {
    // Independently authored zero-valued rows with the public typed-schema
    // names. The old fixture remains separately available as a syntax-only
    // historical save control; its labels cannot prepare runtime house rules.
    constexpr std::array<std::string_view,15> houses{
        "1: Shelter","2: Hut","3: Plain Cottage","4: Attractive Cottage",
        "5: Spacious Dwelling","6: Elegant Dwelling","7: Ornate Apartment",
        "8: Lux. Apartment","Elite: Unoccupied","Elite 0: Abandoned",
        "Elite 1: Modest Siheyuan","Elite 2: Lavish Siheyuan",
        "Elite 3: Humble Compound","Elite 4: Impressive Compound","Elite 5: Heavenly Compound"};
    constexpr std::array<std::string_view,7> buildings{
        "35,Clay Pit,","43,Kiln,","54,Warehouse,","59,Common Market Square,",
        "72,Well,","193,Farmhouse,","207,Herbalist's Stall,"};
    auto text=historical_syntactic_model_text();
    const std::string old_building="35,Authored test building,{,0,0,0,0,0,0,0,0,0,0,0,0,0,},,,\n";
    std::string typed_buildings;
    for (const auto name:buildings) typed_buildings+=std::string(name)+"{,0,0,0,0,0,0,0,0,0,0,0,0,0,},,,\n";
    text.replace(text.find(old_building),old_building.size(),typed_buildings);
    for (std::size_t i=0;i<houses.size();++i) {
        const auto old="Authored house "+std::to_string(i)+',';
        text.replace(text.find(old),old.size(),std::string(houses[i])+',');
    }
    if (variant==1) {
        const std::string old="35,Clay Pit,{,0,";
        text.replace(text.find(old),old.size(),"35,Clay Pit,{,37,");
        std::string empty="1: Shelter,{,",capacity=empty;
        for (int field=0;field<17;++field) { empty+="0,";capacity+="0,"; }
        empty+="0,";capacity+="37,";
        text.replace(text.find(empty),empty.size(),capacity);
    } else if (variant!=0) throw std::invalid_argument("Unknown authored schema23 model variant");
    return text;
}
inline void write_model(const std::filesystem::path& root,int variant=0) {
    std::filesystem::create_directories(root/"Model");
    const auto text=authored_model_text(variant);
    std::ofstream out(root/"Model/EmperorBuildingModels.txt",std::ios::binary);
    out.write(text.data(),static_cast<std::streamsize>(text.size()));
    if (!out.good()) throw std::runtime_error("Cannot write authored schema23 model fixture");
}
} // namespace oe_save_fixture
