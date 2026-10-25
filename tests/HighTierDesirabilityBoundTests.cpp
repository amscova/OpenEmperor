#include "HighTierDesirabilityBound.h"

#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
namespace bound=oe_high_tier_des_bound;
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
namespace fs=std::filesystem;
void check(bool value,const std::string& why){if(!value)throw std::runtime_error(why);}
std::shared_ptr<const sim::MapPermissions> authored_plane() {
    // Independent synthetic geometry only; supplied original model stays read-only.
    return std::make_shared<const sim::MapPermissions>(100,30,3,
        std::vector<sim::MapCellPermission>(3000,{true,true,false,0,sim::BuildBlocker::None,sim::BuildBlocker::None}),
        std::vector<sim::FixedGatePassage>{});
}
void authority_caps(const std::shared_ptr<const orig::OriginalRulesModel>& model,const bound::Json& proof) {
    auto world=sim::World::create_original_sustainable_economy(authored_plane(),model);int position=0;
    for(const auto& source:proof.at("source_identities")) {
        const auto command=static_cast<sim::CommandType>(source.at("command").get<unsigned>());
        const auto cap=source.at("cap").get<unsigned>();
        for(unsigned i=0;i<cap;++i) {
            sim::Command buy{command,{position*4,2}};++position;
            const auto before=world.treasury(),quote=world.construction_cost(command);
            const auto result=world.execute(buy);
            check(result.accepted&&result.changed&&world.treasury()==before-quote,
                "Certificate cap does not match an actual paid World source purchase");
        }
        const auto before=world.snapshot();const sim::Command extra{command,{position*4,10}};
        const auto planned=world.validate(extra);const auto actual=world.execute(extra);
        check(!planned.accepted&&!actual.accepted&&!actual.changed&&world.snapshot()==before,
            "Certificate missed an actual source cap or cap refusal was not atomic");
    }
    check(world.buildings().size()==24&&world.ticks()==0&&world.total_population()==0,
        "Cap fixture introduced ticks, residents or unexpected source bodies");
}
void pure_negative_gate(const orig::HouseEvolutionRules& rules,int maximum,int threshold) {
    // Synthetic pure-kernel negative control only: never inserted into World,
    // persisted or accepted as an actual paid House/service/goods witness.
    orig::HouseState input;input.type=orig::HouseType::ImpressiveCompound;input.occupants=1;
    input.water=true;input.herbalist=input.acupuncture=input.music=input.acrobat=input.drama=1;
    input.ancestor=input.confucian=input.daoist=input.buddhist=1;input.food_quality=80;
    input.hemp=input.ceramics=input.tea=input.silk=input.bronzeware=input.lacquerware=8;
    check(orig::needs_met(rules,orig::house_row(input.type),input)&&
        orig::needs_met(rules,orig::next_row(input.type),input),"Negative control has an unrelated need deficit");
    input.desirability=static_cast<std::int8_t>(maximum);
    check(orig::evolve_step(rules,input).type!=orig::HouseType::HeavenlyCompound,
        "Geometric upper bound still enters Elite17");
    input.desirability=static_cast<std::int8_t>(threshold);
    check(orig::evolve_step(rules,input).type==orig::HouseType::HeavenlyCompound,
        "Pure threshold control failed; the claimed blocker was unrelated to DES");
}
}
int main(int argc,char** argv) {
    try {
        check(argc==3&&std::string_view(argv[1])=="--real","Usage: high-tier-des-bound-tests --real DATA");
        const fs::path data=argv[2];if(!fs::is_regular_file(data/"Model/EmperorBuildingModels.txt")) {
            std::cout<<"SKIP: supplied original model installation is unavailable.\n";return 77;
        }
        const auto parsed=orig::load_model_data(data);check(bool(parsed),"Shared original ModelData loader refused supplied installation");
        const auto model=orig::OriginalRulesModel::prepare_production(*parsed.model,orig::Difficulty::Normal);
        auto world=sim::World::create_original_sustainable_economy(authored_plane(),model);
        const auto before=world.snapshot();const auto proof=bound::report(world);
        check(world.snapshot()==before,"Read-only bound verifier changed World authority");
        check(proof.at("verified")==true&&proof.at("blocks_entry")==true&&proof.at("upper_bound")==79&&
            proof.at("entry_threshold")==80&&proof.at("source_cap_charge")==48&&proof.at("all16cells").size()==16,
            "Current Normal Rule12 geometric79<80 certificate failed; review the explicit rule/model contract");
        for(const auto& cell:proof.at("all16cells"))check(cell.at("cell_charge")==31&&cell.at("minimum_integer_slack").get<int>()>=0,
            "A House cell escaped the integer dual certificate");
        authority_caps(model,proof);pure_negative_gate(model->house_rules(),79,80);
        std::cout<<"PASS: exact16-cell Rule12 geometric DES<=79<Elite17 entry80; "
            <<proof.at("positive_placements_checked")<<" positive relaxed placements; paid source-cap refusals atomic; "
            <<"synthetic pure DES79/80 controls only, no paid high-tier success claim.\n";
        return 0;
    }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
