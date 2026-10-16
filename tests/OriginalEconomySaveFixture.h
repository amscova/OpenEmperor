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
inline std::string authored_model_text() {
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
inline void write_model(const std::filesystem::path& root) {
    std::filesystem::create_directories(root/"Model");
    const auto text=authored_model_text();
    std::ofstream out(root/"Model/EmperorBuildingModels.txt",std::ios::binary);
    out.write(text.data(),static_cast<std::streamsize>(text.size()));
    if (!out.good()) throw std::runtime_error("Cannot write authored schema23 model fixture");
}
} // namespace oe_save_fixture
