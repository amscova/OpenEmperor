#pragma once

// Authored inputs for the active Rule10 working generation only. Historical
// seven-row fixtures and their source/golden bytes remain unchanged.
#include "OriginalInfluenceFixture.h"

namespace oe_production_fixture {
inline std::string authored_model_text(int shelter_capacity=37,int hemp_need=0,int ceramics_need=0,int food_need=0,
                                       int herbalist_need=0,int acupuncture_need=0) {
    auto text=oe_influence_fixture::authored_model_text();
    std::string before="1: Shelter,{,",after=before;
    for (int i=0;i<17;++i) {
        before+="0,";
        after+=std::to_string(i==2 ? herbalist_need:i==4 ? acupuncture_need:
                             i==8 ? food_need:i==9 ? hemp_need:i==10 ? ceramics_need:0)+',';
    }
    before+="0,";after+=std::to_string(shelter_capacity)+',';
    const auto capacity=text.find(before);
    oe_influence_fixture::check(capacity!=std::string::npos,"Authored production Shelter row missing");
    text.replace(capacity,before.size(),after);
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
        "208,Acupuncturist's Clinic,{,50,0,0,0,0,8,0,0,0,0,0,0,0,},,,\n");
    return text;
}
inline void write_model(const std::filesystem::path& root,int shelter_capacity=37,int hemp_need=0,int ceramics_need=0,int food_need=0,
                        int herbalist_need=0,int acupuncture_need=0) {
    std::filesystem::create_directories(root/"Model");
    oe_influence_fixture::write_text(root/"Model/EmperorBuildingModels.txt",
        authored_model_text(shelter_capacity,hemp_need,ceramics_need,food_need,herbalist_need,acupuncture_need));
}
} // namespace oe_production_fixture
