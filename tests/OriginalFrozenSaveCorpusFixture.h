#pragma once
// Representative historical syntax and its existing read-normalize-write contract.
// All model/map inputs below are authored; no original or user saves are edited.
#include "OriginalProductionFixture.h"
#include <nlohmann/json.hpp>
#include <openssl/evp.h>
#include <iostream>

namespace oe_frozen_save_corpus {
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace save=openemperor::persistence;
using json=nlohmann::json;
inline std::string sha256(const std::string& bytes) {
    std::array<unsigned char,EVP_MAX_MD_SIZE> digest{};unsigned length=0;
    f::check(EVP_Digest(bytes.data(),bytes.size(),digest.data(),&length,EVP_sha256(),nullptr)==1 && length==32,"Frozen corpus SHA failure");
    constexpr char hex[]="0123456789abcdef";std::string result;
    for(unsigned i=0;i<length;++i){result+=hex[digest[i]>>4];result+=hex[digest[i]&15];}return result;
}
inline json file(const f::fs::path& path) {const auto bytes=f::read_text(path);return {{"file",path.filename().generic_string()},{"bytes",bytes.size()},{"sha256",sha256(bytes)}};}
inline std::shared_ptr<const sim::MapPermissions> permissions(std::uint32_t policy) {
    return std::make_shared<const sim::MapPermissions>(f::width,f::height,policy,
        std::vector<sim::MapCellPermission>(f::width*f::height,{true,true,false,0,sim::BuildBlocker::None,sim::BuildBlocker::None}),
        std::vector<sim::FixedGatePassage>{});
}
inline sim::World world_for(unsigned schema,const std::shared_ptr<const sim::MapPermissions>& policy,
                             const std::shared_ptr<const openemperor::original::OriginalRulesModel>& model) {
    using P=sim::RulesProfile;
    if(schema==23)return sim::World::create_original_economy_development(policy,model);
    if(schema==24)return sim::World::create_original_influence_development(policy,model);
    if(schema==25)return sim::World::create_original_goods_development(policy,model);
    if(schema>=19)return sim::World(policy,P::CityV16,schema==19 ? 3:schema==20 ? 4:schema==21 ? 5:6);
    const std::array<P,19> profiles{P::LogisticsV1,P::LogisticsV1,P::ProductionV2,P::HouseholdV3,P::LogisticsV1,
        P::IndustryV5,P::CityV6,P::CityV7,P::CityV8,P::CityV9,P::CityV10,P::CityV11,P::CityV11,P::CityV11,
        P::CityV12,P::CityV13,P::CityV14,P::CityV15,P::CityV16};
    const auto version=schema==11 ? 2U:schema==12 ? 3U:schema==13 ? 4U:schema==18 ? 2U:0U;
    return sim::World(f::width,f::height,policy->building_mask(),profiles.at(schema),version);
}
inline json historical_input(json value,unsigned schema) {
    if(schema>3)return value;
    value["schema_version"]=schema;
    auto& world=value["world"];
    world.erase("next_household_id");world.erase("last_dispatched_household");
    const std::size_t buildings=schema==3 ? 4:3,couriers=schema==3 ? 3:2;
    while(world["buildings"].size()>buildings)world["buildings"].erase(buildings);
    while(world["couriers"].size()>couriers)world["couriers"].erase(couriers);
    if(schema<3)for(auto& b:world["buildings"])for(const char* key:{"placed_tick","demand_progress","fulfilled_demand","missed_demand","consumed_total","last_demand_status"})b.erase(key);
    if(schema==1){
        value["rules"]["version"]=1;world.erase("roads_placed_total");world.erase("roads_removed_total");
        for(auto& c:world["couriers"])for(const char* key:{"route_pending","route_checked_revision","reroute_attempts"})c.erase(key);
    }
    return value;
}
inline sim::World restore(const save::SaveDocument& d,const f::fs::path& data,const std::vector<std::uint8_t>& mask,
                          const std::shared_ptr<const sim::MapPermissions>& policy) {
    if(d.world.rule_version==7 && d.world.profile==sim::RulesProfile::CityV16)return save::restore_original_economy_development(d,data,mask,policy);
    if(d.world.rule_version==8 && d.world.profile==sim::RulesProfile::CityV16)return save::restore_original_influence_development(d,data,mask,policy);
    if(d.world.rule_version==9 && d.world.profile==sim::RulesProfile::CityV16)return save::restore_original_goods_development(d,data,mask,policy);
    return d.map_permissions_policy_version ? save::restore_save(d,data,mask,policy):save::restore_save(d,data,mask);
}
inline void export_corpus(const f::fs::path& output) {
    f::check(!f::fs::exists(output),"Frozen corpus output already exists");f::fs::create_directories(output);
    f::Root root;oe_production_fixture::write_model(root.data,37);
    const auto model=save::prepare_original_rules_model(root.data);
    json cases=json::array();
    for(unsigned schema=1;schema<=25;++schema){
        const auto policy=permissions(schema==19 ? 1:schema==20 ? 2:3);const auto mask=policy->building_mask();
        auto world=world_for(schema,policy,model);f::buy(world,{sim::CommandType::PlaceRoad,{0,4}});
        if(schema<23)for(unsigned tick=0;tick<17;++tick)world.tick();
        const auto input=output/("schema"+std::to_string(schema)+".input.json");
        const auto generated=output/("schema"+std::to_string(schema)+".generated.json");
        const auto normalized=output/("schema"+std::to_string(schema)+".normalized.json");
        const auto document=save::make_document(root.data,f::map_relative,mask,world);
        save::write_save(generated,document,root.data,mask);
        const auto raw=historical_input(json::parse(f::read_text(generated)),schema);
        f::write_text(input,raw.dump(2));const auto input_before=f::read_text(input);
        auto parsed=save::read_save(input);f::check(parsed.source_schema_version==schema,"Missing exact historical source schema");
        auto loaded=restore(parsed,root.data,mask,policy);f::check(loaded.snapshot()==parsed.world,"Historical restore differs from parsed authority");
        if(schema>=19)parsed.prepared_permissions=policy;
        if(schema>=23)parsed.prepared_original_rules_model=model;
        save::write_save(normalized,parsed,root.data,mask);
        f::check(f::read_text(input)==input_before,"Historical input was rewritten in place");
        const auto reread=save::read_save(normalized);f::same_document(parsed,reread);
        auto repeated=restore(reread,root.data,mask,policy);f::check(repeated.snapshot()==loaded.snapshot(),"Historical normalized restore differs");
        const auto dto=output/("schema"+std::to_string(schema)+".dto.json");
        f::write_text(dto,json{{"source_schema_version",parsed.source_schema_version},{"migrated_from_schema1",parsed.migrated_from_schema1},
            {"profile",sim::rules_profile_name(parsed.world.profile)},{"rule_version",parsed.world.rule_version},
            {"normalized_schema_version",reread.source_schema_version},{"normalized_document",json::parse(f::read_text(normalized))},
            {"full_snapshot_equal_after_restore",loaded.snapshot()==parsed.world},{"input_unchanged",true}}.dump(2)+"\n");
        cases.push_back({{"input_schema",schema},{"input",file(input)},{"generated",file(generated)},
            {"normalized",file(normalized)},{"dto",file(dto)},{"source_schema_version",parsed.source_schema_version},
            {"normalized_schema_version",reread.source_schema_version},{"input_unchanged",true}});
    }
    f::write_text(output/"corpus.json",json{{"status","PASS"},{"scope","25 representative authored historical inputs; compare old reader normalization/writer output, not in-place migration"},
        {"input_schemas",25},{"cases",cases}}.dump(2)+"\n");
    std::cout<<"Frozen representative schemas1-25 actual reader/normalization/export PASS\n";
}
} // namespace oe_frozen_save_corpus
