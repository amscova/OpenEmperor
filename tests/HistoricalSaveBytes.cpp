// Cross-revision exporter of authored historical SaveDocuments. No game data.
// Run the same source against baseline and candidate libraries; compare files
// literally. Generated documents are private evidence, never tracked fixtures.
#include "OriginalProductionFixture.h"
#include <iostream>
#include <nlohmann/json.hpp>

int main(int argc,char** argv){
    namespace f=oe_influence_fixture;
    namespace sim=openemperor::simulation;
    namespace save=openemperor::persistence;
    namespace fs=std::filesystem;
    using Json=nlohmann::json;
    try{
        f::check(argc==2,"Pass a new private output directory");const auto out=fs::absolute(argv[1]);
        f::check(!fs::exists(out),"Preserve historical proof output");fs::create_directories(out);f::Root root;const std::vector<std::uint8_t> mask(f::width*f::height,1);
        const auto emit=[&](const sim::World& w,const std::string& label){auto d=save::make_document(root.data,f::map_relative,mask,w);save::write_save(out/(label+".json"),d,root.data,mask);};
        std::size_t count=0;
        for(int p=0;p<=static_cast<int>(sim::RulesProfile::CityV16);++p)for(unsigned v=1;v<=10;++v){
            const auto profile=static_cast<sim::RulesProfile>(p);if(!sim::rule_version_supported(profile,v))continue;
            const auto policy_version=profile==sim::RulesProfile::CityV16?(v>=5?3U:v==4?2U:1U):3U;
            auto permissions=std::make_shared<const sim::MapPermissions>(f::width,f::height,policy_version,
                std::vector<sim::MapCellPermission>(f::width*f::height,{true,true,false,0,sim::BuildBlocker::None,sim::BuildBlocker::None}),std::vector<sim::FixedGatePassage>{});
            auto w=[&]{
                if(profile==sim::RulesProfile::CityV16&&v>=7){
                    if(v==10){oe_production_fixture::write_model(root.data);auto model=save::prepare_original_production_rules_model(root.data);return sim::World::create_original_production_development(permissions,model);}
                    f::write_model(root.data);auto model=save::prepare_original_rules_model(root.data);
                    if(v==7)return sim::World::create_original_economy_development(permissions,model);
                    if(v==8)return sim::World::create_original_influence_development(permissions,model);
                    return sim::World::create_original_goods_development(permissions,model);
                }
                if(profile==sim::RulesProfile::CityV16&&v>=3)return sim::World(permissions,profile,v);
                return sim::World(f::width,f::height,permissions->building_mask(),profile,v);
            }();
            const auto name="profile-"+std::to_string(p)+"-rule-"+std::to_string(v);
            emit(w,name+"-tick0");for(unsigned i=0;i<400;++i)w.tick();emit(w,name+"-tick400");count+=2;
        }
        // Schemas1..3 are read-only historical syntax; export their exact
        // canonical reserialization with the same explicit legacy conversions
        // covered by SandboxSaveTests, without changing a source generation.
        for(unsigned schema=1;schema<=3;++schema){
            const auto profile=schema==1?sim::RulesProfile::LogisticsV1:schema==2?sim::RulesProfile::ProductionV2:sim::RulesProfile::HouseholdV3;
            sim::World w(f::width,f::height,std::vector<std::uint8_t>(f::width*f::height,1),profile);
            const auto temporary=out/("legacy-source-"+std::to_string(schema)+".json");emit(w,"legacy-source-"+std::to_string(schema));
            auto j=Json::parse(f::read_text(temporary));j["schema_version"]=schema;
            j["world"].erase("next_household_id");j["world"].erase("last_dispatched_household");
            if(schema==3){while(j["world"]["buildings"].size()>4)j["world"]["buildings"].erase(4);while(j["world"]["couriers"].size()>3)j["world"]["couriers"].erase(3);}
            if(schema<3){while(j["world"]["buildings"].size()>3)j["world"]["buildings"].erase(3);if(j["world"]["couriers"].size()>2)j["world"]["couriers"].erase(2);
                for(auto& b:j["world"]["buildings"])for(const char* key:{"placed_tick","demand_progress","fulfilled_demand","missed_demand","consumed_total","last_demand_status"})b.erase(key);}
            if(schema==1){j["rules"]["version"]=1;j["world"].erase("roads_placed_total");j["world"].erase("roads_removed_total");for(auto& c:j["world"]["couriers"])for(const char* key:{"route_pending","route_checked_revision","reroute_attempts"})c.erase(key);}
            f::write_text(temporary,j.dump(2)+"\n");const auto document=save::read_save(temporary);
            save::write_save(out/("legacy-reserialized-"+std::to_string(schema)+".json"),document,root.data,mask);count+=2;
        }
        std::cout<<count<<" authored historical SaveDocuments exported\n";return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
