#include "../tests/OriginalFinanceFixture.h"
#include <nlohmann/json.hpp>
#include <charconv>
#include <iostream>

namespace {
namespace fx=oe_finance_fixture;
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace save=openemperor::persistence;
namespace orig=openemperor::original;
using json=nlohmann::json;
void output_target(const f::fs::path& output,const f::fs::path& data) {
    const auto repo=f::fs::canonical(f::fs::path(__FILE__).parent_path().parent_path());
    const auto allowed=repo/".local/original-economy/oe27/production-fixtures";
    const auto below=[](const auto& child,const auto& parent){auto c=child.begin();for(auto p=parent.begin();p!=parent.end();++p,++c)if(c==child.end()||*c!=*p)return false;return true;};
    f::check(output!=allowed && below(output,allowed),"Fixture output must be a new private OE27 subdirectory");
    f::check(!below(output,data) && !below(data,output),"Fixture output overlaps source data");
    f::fs::path current=output.root_path();
    for(const auto& part:output.relative_path()) {
        f::check(part!=".."&&part!=".","Fixture output is not canonical");current/=part;
        std::error_code error;f::check(!f::fs::is_symlink(f::fs::symlink_status(current,error)),"Fixture output traverses symlink");
    }
    f::check(!f::fs::exists(output),"Fixture output already exists");
    save::validate_save_target(output/"fixture.json",data);
}
json observe(const sim::World& world) {
    fx::accounting(world);
    json houses=json::array(),buildings=json::array(),rates=json::array();
    for(unsigned r=0;r<orig::house_row_count;++r)rates.push_back(world.original_rules_model()->house_rules().value(static_cast<orig::ModelHouse>(r),orig::HouseField::TaxRate));
    for(const auto& b:world.buildings()) {
        buildings.push_back({{"id",static_cast<unsigned>(b.id)},{"kind",static_cast<unsigned>(b.kind)},
            {"placed_tick",b.placed_tick},{"workers_required",world.workforce_required(b.id)},
            {"workers_assigned",world.workers_assigned(b.id)},{"taxes_paid_total",b.taxes_paid_total}});
        if(!sim::original_house_kind(b.kind))continue;
        const auto& h=fx::house(world,b.id);const auto& finance=fx::finance(world,b.id);const auto tax=world.original_house_tax_status(b.id);
        json rows=json::array(),last=nullptr;
        for(const auto& row:finance.rows)rows.push_back({{"events",row.events},{"resident_units",row.resident_units}});
        if(finance.last_assessment)last={{"tick",finance.last_assessment->tick},{"type",static_cast<unsigned>(finance.last_assessment->type)},
            {"occupants",finance.last_assessment->occupants}};
        houses.push_back({{"id",static_cast<unsigned>(b.id)},{"type",static_cast<unsigned>(h.type)},
            {"occupants",h.occupants},{"burning",tax.burning},{"rate",tax.rate},{"eligible",tax.eligible},
            {"paid_total",b.taxes_paid_total},{"rows",rows},{"last_assessment",last}});
    }
    const auto goal=world.original_goal_status();
    return {{"tick",world.ticks()},{"revision",world.original_production_state()->development_revision},
        {"difficulty",static_cast<unsigned>(world.original_rules_model()->difficulty())},
        {"model_sha256",world.original_rules_model()->provenance().sha256},{"funds",world.treasury()},
        {"taxes",world.taxes_collected_total()},{"maintenance",world.maintenance_spent_total()},
        {"construction",world.construction_spent_total()},{"population",world.total_population()},
        {"workforce_supply",world.workforce_supply()},{"workforce_used",world.workforce_used()},
        {"rates",rates},{"houses",houses},{"buildings",buildings},
        {"goal",{{"inhabited_houses",goal.inhabited_houses},{"common_houses",goal.common_houses},
            {"population",goal.population},{"positive_tax",goal.positive_tax},{"nonnegative_funds",goal.nonnegative_funds},{"reached",goal.reached}}}};
}
json checkpoint(const f::fs::path& data,const f::fs::path& output,const std::string& name,const sim::World& world) {
    const auto saved=output/(name+".json"),report=output/(name+".report.json");
    save::write_save(saved,save::make_document(data,f::map_relative,world.map_permissions()->building_mask(),world),data,world.map_permissions()->building_mask());
    const auto d=save::read_save(saved);const auto restored=save::restore_original_production_development(d,data,world.map_permissions()->building_mask(),world.map_permissions());
    f::check(restored.snapshot()==world.snapshot(),"Whole finance SaveDocument changed actual restore");
    f::write_text(report,observe(world).dump(2)+"\n");
    return {{"name",name},{"save",saved.filename().string()},{"report",report.filename().string()},{"tick",world.ticks()}};
}
void continuation(const f::fs::path& data,const f::fs::path& output,sim::World& world) {
    const auto begin=world.ticks();fx::through(world,begin+10000);const auto mid=checkpoint(data,output,"midpoint",world);
    fx::through(world,begin+20000);const auto end=checkpoint(data,output,"result",world);
    f::write_text(output/"replay.json",json{{"status","PASS"},{"begin",begin},{"ticks",20000},{"midpoint",mid},{"result",end}}.dump(2)+"\n");
}
void create(const f::fs::path& data,const f::fs::path& output,orig::Difficulty difficulty) {
    const auto policy=f::permissions();auto world=sim::World::create_original_production_development(policy,fx::original_model_real(data,difficulty));
    const auto scene=fx::build(world,true);json checkpoints=json::array();
    checkpoints.push_back(checkpoint(data,output,"paid-empty",world));
    fx::through(world,815);const auto before=observe(world);checkpoints.push_back(checkpoint(data,output,"pre-first-tax",world));
    fx::tick(world);const auto first=observe(world);checkpoints.push_back(checkpoint(data,output,"post-first-tax",world));
    fx::through(world,1224);checkpoints.push_back(checkpoint(data,output,"mid-tax-cycle",world));
    fx::advance_to_tax_or_goal(world);const auto reached=observe(world);checkpoints.push_back(checkpoint(data,output,"paid-live-goal",world));
    f::check(world.taxes_collected_total()>0 && world.original_goal_status().reached,"Actual original-model paid city did not earn positive tax and current goal");
    const auto exact=world.snapshot();for(int i=0;i<100;++i){(void)world.original_goal_status();for(const auto id:scene.commons)(void)world.original_house_tax_status(id);}
    f::check(world.snapshot()==exact,"Finance queries mutate actual World");
    auto replay=save::restore_original_production_development(save::make_document(data,f::map_relative,policy->building_mask(),world),data,policy->building_mask(),policy);
    continuation(data,output/"expected",replay);
    f::write_text(output/"fixture.json",json{{"status","PASS"},{"original_model",true},{"authored_geography",true},
        {"snapshot_injection",false},{"difficulty",static_cast<unsigned>(difficulty)},{"scene_spent",scene.spent},
        {"before_first",before},{"after_first",first},{"reached",reached},{"checkpoints",checkpoints}}.dump(2)+"\n");
}
}
int main(int argc,char** argv) {
    try {
        f::check(argc==5,"Usage: original-finance-fixture --create DATA OUT DIFFICULTY | --resume DATA OUT SAVE");
        const std::string mode=argv[1];f::check(mode=="--create"||mode=="--resume","Unknown finance fixture mode");
        unsigned selected=0;if(mode=="--create") {const std::string_view token=argv[4];const auto [end,error]=std::from_chars(token.data(),token.data()+token.size(),selected);f::check(error==std::errc{}&&end==token.data()+token.size()&&selected<5,"Difficulty must be a typed index0..4");}
        const auto data=f::fs::canonical(argv[2]),output=f::fs::absolute(argv[3]);output_target(output,data);f::fs::create_directories(output);
        if(mode=="--create") {f::fs::create_directories(output/"expected");create(data,output,static_cast<orig::Difficulty>(selected));}
        else {const auto input=f::fs::canonical(argv[4]);const auto bytes=f::read_text(input);const auto d=save::read_save(input);const auto p=f::permissions();auto world=save::restore_original_production_development(d,data,p->building_mask(),p);f::check(world.snapshot()==d.world,"Actual separate-process finance restore changed Snapshot");continuation(data,output,world);f::check(f::read_text(input)==bytes,"Actual finance replay changed supplied input");}
        std::cout<<"Original paid finance process PASS\n";return 0;
    } catch(const std::exception& error){std::cerr<<"Original finance fixture FAIL: "<<error.what()<<'\n';return 1;}
}
