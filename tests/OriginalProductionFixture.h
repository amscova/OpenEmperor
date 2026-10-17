#pragma once

// Authored inputs for the active Rule10 working generation only. Historical
// seven-row fixtures and their source/golden bytes remain unchanged.
#include "OriginalInfluenceFixture.h"

namespace oe_production_fixture {
inline std::string authored_model_text(int shelter_capacity=37) {
    auto text=oe_influence_fixture::authored_model_text();
    std::string before="1: Shelter,{,",after=before;
    for (int i=0;i<17;++i) {before+="0,";after+="0,";}
    before+="0,";after+=std::to_string(shelter_capacity)+',';
    const auto capacity=text.find(before);
    oe_influence_fixture::check(capacity!=std::string::npos,"Authored production Shelter row missing");
    text.replace(capacity,before.size(),after);
    const auto section=text.find("HOUSE MODS\n");
    oe_influence_fixture::check(section!=std::string::npos,"Authored production model section missing");
    text.insert(section,
        "192,Hemp Farm,{,40,0,0,0,0,18,0,0,0,0,0,0,0,},,,\n"
        "194,Hemp Field,{,4,0,0,0,0,0,0,0,0,0,0,0,0,},,,\n");
    return text;
}
inline void write_model(const std::filesystem::path& root,int shelter_capacity=37) {
    std::filesystem::create_directories(root/"Model");
    oe_influence_fixture::write_text(root/"Model/EmperorBuildingModels.txt",authored_model_text(shelter_capacity));
}
} // namespace oe_production_fixture
