// Authored paid integration and strict own-save checks; no synthetic state is
// an original-data Elite progression or human-playtest witness.
#include "OriginalFinanceFixture.h"
#include "persistence/RecoveryStore.h"
#include "maps/EmperorMap.h"
#include "maps/MapCatalog.h"
#include <nlohmann/json.hpp>
#include <zlib.h>
#include <iostream>

namespace {
namespace f=oe_influence_fixture;
namespace q=oe_finance_fixture;
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
namespace save=openemperor::persistence;
namespace maps=openemperor::maps;
using Json=nlohmann::json;
using Bytes=std::vector<std::uint8_t>;
using f::check;
template<class F>void refuses(F action,const char* why) {
    try{action();}catch(const std::exception&){return;}throw std::runtime_error(why);
}
void u32(Bytes& bytes,std::size_t at,std::uint32_t value) {
    for(unsigned i=0;i<4;++i)bytes.at(at+i)=static_cast<std::uint8_t>(value>>(8*i));
}
Bytes encode(const Bytes& raw) {
    Bytes out(4);u32(out,0,0xfedcbaaaU);
    for(std::size_t at=0;at<raw.size();at+=32768) {
        const auto size=std::min<std::size_t>(32768,raw.size()-at);uLongf length=compressBound(static_cast<uLong>(size));
        Bytes block(length);check(compress2(block.data(),&length,raw.data()+at,static_cast<uLong>(size),6)==Z_OK,"Authored compression failed");
        const auto offset=out.size();out.resize(offset+12);u32(out,offset,0x12345678U);
        u32(out,offset+4,static_cast<std::uint32_t>(length));u32(out,offset+8,static_cast<std::uint32_t>(size));
        out.insert(out.end(),block.begin(),block.begin()+static_cast<std::ptrdiff_t>(length));
    }
    return out;
}
struct Fixture {
    f::Root root;
    std::shared_ptr<const sim::MapPermissions> policy;
    std::shared_ptr<const orig::OriginalRulesModel> model;
    std::shared_ptr<const sim::OriginalTerrainFacts> terrain;
    std::vector<std::uint32_t> raw=std::vector<std::uint32_t>(sim::OriginalTerrainFacts::cell_count);
    Fixture() {
        // Extra bits deliberately prevent any equation of raw bit4 with bridge
        // eligibility. Policy independently refuses construction on this cell.
        raw[20*228+19]=0x80004;raw[43*228+44]=4;
        std::vector<sim::MapCellPermission> cells(raw.size(),{true,true,false,0,sim::BuildBlocker::None,sim::BuildBlocker::None});
        cells[20*228+19]={false,false,false,0,sim::BuildBlocker::LegacyBuildabilityRestriction,sim::BuildBlocker::LegacyBuildabilityRestriction};
        cells[43*228+44]=cells[20*228+19];
        policy=std::make_shared<const sim::MapPermissions>(228,228,3,std::move(cells),std::vector<sim::FixedGatePassage>{});
        auto text=q::authored_model();
        q::set_building_field(text,orig::ModelBuilding::Well,orig::BuildingField::Desirability,100);
        q::set_building_field(text,orig::ModelBuilding::Well,orig::BuildingField::DesirabilityStep,1);
        q::set_building_field(text,orig::ModelBuilding::Well,orig::BuildingField::DesirabilityStepSize,0);
        q::set_building_field(text,orig::ModelBuilding::Well,orig::BuildingField::DesirabilityRange,10);
        f::write_text(root.data/"Model/EmperorBuildingModels.txt",text);
        model=save::prepare_original_production_rules_model(root.data);
        Bytes bytes(static_cast<std::size_t>(maps::objects_logical_offset+maps::grid_byte_length));
        const std::array<std::uint8_t,8> signature{5,0,0xfe,0xca,0,0,2,0};std::copy(signature.begin(),signature.end(),bytes.begin());
        u32(bytes,84,226);for(std::size_t i=0;i<raw.size();++i)u32(bytes,static_cast<std::size_t>(maps::terrain_logical_offset)+4*i,raw[i]);
        const auto encoded=encode(bytes);f::write_text(root.data/f::map_relative,std::string(encoded.begin(),encoded.end()));
        terrain=save::prepare_original_terrain_facts(root.data,f::map_relative);
    }
    sim::World world() const{return sim::World::create_original_cardinal_water(policy,model,terrain);}
    save::SaveDocument document(const sim::World& w)const{return save::make_document(root.data,f::map_relative,policy->building_mask(),w);}
};
sim::BuildingId house(sim::World& w,sim::Cell at) {
    f::buy(w,{sim::CommandType::PlaceHousehold,at});return *w.building_owner_at(at);
}
void identities_and_recipient_authority() {
    Fixture fxt;auto w=fxt.world();const auto start=w.snapshot();
    check(w.rule_version()==14&&w.active_rules().version==14&&w.original_cardinal_water_enabled()&&
        w.original_market_selection_enabled()&&!w.original_market_logistics_enabled()&&w.original_sustainable_economy_enabled()&&
        w.original_fire_protection_enabled()&&w.original_worker_upkeep_interval()==816&&w.treasury()==12000&&
        w.original_production_state()->development_revision==27,"Rule14 did not retain explicit inherited authority");
    refuses([&]{(void)sim::World::create_original_cardinal_water(fxt.policy,fxt.model,nullptr);},"Missing terrain context accepted");
    refuses([&]{sim::World ordinary(fxt.policy,sim::RulesProfile::CityV16,14);},"Ordinary constructor silently enabled Rule14");
    refuses([&]{(void)sim::World::create_original_cardinal_water(f::permissions(),fxt.model,fxt.terrain);},"Partial source grid accepted");
    const auto edge=w.execute({sim::CommandType::PlaceHousehold,{0,0}});
    check(!edge.accepted&&!edge.changed&&w.snapshot()==start,"Outside cardinal perimeter mutated state/IDs/Funds");
    const auto home=house(w,{20,20});f::source(w,{22,20});
    const auto p=w.original_influence_provenance(home);
    check(p.raw_value==100&&w.original_house_cardinal_water_bonus(home)==10&&
        w.original_house_influence_value(home)==110,"Recipient bonus was omitted, clamped or emitted as a source");
    check(p.raw_value==100&&p.cardinal_water_bonus==10&&p.value==110&&p.source_map_sha256==fxt.terrain->source_map_sha256(),
        "Inspection conflated raw cells and recipient query provenance");
    const auto no_water=house(w,{30,20});check(w.original_house_cardinal_water_bonus(no_water)==0,"Proximity invented a recipient water flag");
    const auto source_before=w.original_influence_provenance(home).raw_value;const auto before=w.snapshot();
    refuses([&]{(void)w.original_house_cardinal_water_bonus(*w.building_owner_at({22,20}));},"Non-House received a recipient bonus");
    check(w.snapshot()==before&&w.original_influence_provenance(home).raw_value==source_before,"Query mutated source/grid authority");
    const auto denied=w.execute({sim::CommandType::PlaceHousehold,{18,20}});
    check(!denied.accepted&&!denied.changed&&w.snapshot()==before,"Raw water fact granted paid placement permission");
    const auto old=sim::World::create_original_market_logistics(fxt.policy,fxt.model);
    refuses([&]{(void)sim::World::restore_original_cardinal_water(old.snapshot(),fxt.policy,fxt.model,fxt.terrain);},"Rule13 silently migrated");
    refuses([&]{(void)sim::World::restore_original_market_logistics(w.snapshot(),fxt.policy,fxt.model);},"Rule14 restored through historical Rule13");
}
void paid_lifecycle_and_persistence() {
    Fixture fxt;auto w=fxt.world();
    for(int x=20;x<36;++x)f::buy(w,{sim::CommandType::PlaceRoad,{x,22}});
    const auto common=house(w,{20,20});f::source(w,{22,20});
    const auto elite_cell=sim::Cell{40,40};f::buy(w,{sim::CommandType::PlaceOriginalElitePlot,elite_cell});
    const auto elite=*w.building_owner_at(elite_cell);
    for(int x=40;x<44;++x)f::buy(w,{sim::CommandType::PlaceRoad,{x,44}});
    check(!w.total_population()&&w.original_house_cardinal_water_bonus(elite)==0&&w.building(elite).kind==sim::Object::OriginalElitePlot,
        "Buying a pending2x2 plot granted residents or a4x4 bonus");
    for(unsigned i=0;i<200;++i)w.tick();
    check(w.building(elite).kind==sim::Object::OriginalEliteHouse&&w.original_house_cardinal_water_bonus(elite)==10&&
        q::house(w,elite).occupants>0&&q::house(w,common).occupants>0&&q::house(w,common).desirability==110,
        "Actual admission/materialization/evaluation did not retain the paid recipient body/query");
    const auto path=fxt.root.path/"cardinal-water.json";const auto mask=fxt.policy->building_mask();
    auto document=fxt.document(w);save::write_save(path,document,fxt.root.data,mask);const auto bytes=f::read_text(path);const auto snapshot=w.snapshot();
    const auto parsed=save::read_save(path);check(parsed.source_schema_version==30&&parsed.world.rule_version==14&&parsed.world==snapshot&&
        !parsed.prepared_original_terrain_facts,"Read JSON supplied a terrain fact bitset instead of source identity");
    const auto json=Json::parse(bytes);check(json.at("rules").at("cardinal_water")==sim::original_cardinal_water_contract&&
        json.at("rules").at("market_logistics")==sim::original_market_logistics_contract,
        "Schema30 omitted exact query or inherited logistics contract");
    auto restored=save::restore_original_cardinal_water(parsed,fxt.root.data,mask,fxt.policy,fxt.model);
    check(restored.snapshot()==snapshot&&*restored.original_terrain_facts()==*fxt.terrain,"Independent restore lost source facts or complete snapshot");
    save::write_save(path,fxt.document(restored),fxt.root.data,mask);check(f::read_text(path)==bytes,"Complete canonical Schema30 SaveDocument changed on rewrite");
    const auto reject=[&](auto edit){auto d=parsed;edit(d);refuses([&]{(void)save::restore_original_cardinal_water(d,fxt.root.data,mask,fxt.policy,fxt.model);},"Hostile Rule14 restore accepted");
        check(w.snapshot()==snapshot&&f::read_text(path)==bytes,"Rejected restore changed live World or previous save");};
    reject([](auto& d){d.source_schema_version=29;});reject([](auto& d){d.world.rule_version=13;});
    reject([](auto& d){d.world.original_production->development_revision=28;});reject([](auto& d){d.map_sha256=std::string(64,'0');});
    reject([](auto& d){++d.world.treasury;});reject([](auto& d){d.map_permissions_policy_version=2;});
    reject([&](auto& d){d.world.buildings.front().cell.x++;});
    reject([&](auto& d){d.world.original_economy->houses.front().house.desirability=100;});
    auto forged_raw=fxt.raw;forged_raw[20*228+19]=0;
    const auto forged=std::make_shared<const sim::OriginalTerrainFacts>(forged_raw,fxt.terrain->source_map_sha256());
    check(*forged!=*fxt.terrain,"Complete fact equality ignored a forged same-SHA context");
    refuses([&]{(void)save::restore_original_cardinal_water(parsed,fxt.root.data,mask,fxt.policy,fxt.model,forged);},"Prepared same-SHA forged facts bypassed live source rederivation");
    auto hostile_doc=document;hostile_doc.prepared_original_terrain_facts=forged;
    refuses([&]{save::write_save(path,hostile_doc,fxt.root.data,mask);},"Forged document facts reached atomic publication");
    check(f::read_text(path)==bytes&&w.snapshot()==snapshot,"Failed publication overwrote previous save/World");
    const auto badpath=fxt.root.path/"hostile.json";
    const auto raw_reject=[&](auto edit){auto j=json;edit(j);f::write_text(badpath,j.dump());
        refuses([&]{(void)save::read_save(badpath);},"Hostile Rule14 JSON accepted");check(f::read_text(path)==bytes,"Hostile JSON changed prior save");};
    raw_reject([](auto& j){j["schema_version"]=29;});raw_reject([](auto& j){j["rules"]["version"]=13;});
    raw_reject([](auto& j){j["rules"].erase("cardinal_water");});raw_reject([](auto& j){j["rules"]["cardinal_water"]="unverified";});
    raw_reject([](auto& j){j["rules"].erase("market_logistics");});raw_reject([](auto& j){j["world"]["original_production"]["cardinal_water_bonus"]=10;});
    raw_reject([](auto& j){j["profiles"]["terrain_flags"]=Json::array({4});});
    auto duplicate=bytes;const auto at=duplicate.find("\"cardinal_water\":");check(at!=std::string::npos,"Missing query marker");
    duplicate.insert(at,"\"cardinal_water\":\"verified-cardinal-water-desirability-v1\",");f::write_text(badpath,duplicate);
    refuses([&]{(void)save::read_save(badpath);},"Duplicate schema30 key accepted");
    save::RecoveryStore recovery(fxt.root.path/"recovery",fxt.root.data,save::recovery_storage_budget_bytes,save::RecoveryAccess::OriginalProduction);
    const auto history=recovery.create_history(document,mask);
    for(unsigned i=0;i<200;++i){w.tick();restored.tick();check(w.snapshot()==restored.snapshot(),"Complete Rule14 continuation diverged");}
    const auto entry=recovery.write_checkpoint(history,fxt.document(w),mask);check(entry.loadable()&&entry.schema==30&&entry.rule_version==14,"Recovery lost exact Schema30 generation");
    check(save::restore_original_cardinal_water(save::read_save(entry.path),fxt.root.data,mask,fxt.policy,fxt.model).snapshot()==w.snapshot(),"Recovery lost full recipient history");
    refuses([&]{(void)save::restore_original_market_logistics(parsed,fxt.root.data,mask,fxt.policy,fxt.model);},"Historical restore admitted Schema30");
    const auto old=sim::World::create_original_market_logistics(fxt.policy,fxt.model);const auto olddoc=fxt.document(old);
    const auto oldpath=fxt.root.path/"rule13.json";save::write_save(oldpath,olddoc,fxt.root.data,mask);const auto oldbytes=f::read_text(oldpath);
    const auto oldparsed=save::read_save(oldpath);refuses([&]{(void)save::restore_original_cardinal_water(oldparsed,fxt.root.data,mask,fxt.policy,fxt.model);},"Historical save silently migrated");
    auto relabel=Json::parse(oldbytes);relabel["schema_version"]=30;relabel["rules"]["version"]=14;f::write_text(badpath,relabel.dump());
    refuses([&]{(void)save::read_save(badpath);},"Relabeled Rule13 omitted marker but passed");
    const auto oldrestored=save::restore_original_market_logistics(oldparsed,fxt.root.data,mask,fxt.policy,fxt.model);
    save::write_save(oldpath,fxt.document(oldrestored),fxt.root.data,mask);check(f::read_text(oldpath)==oldbytes,"Historical Schema29 canonical bytes changed");
}
void real_source_only(const std::filesystem::path& data) {
    const auto a=save::prepare_original_terrain_facts(data,"Cities/Xia.map"),b=save::prepare_original_terrain_facts(data,"Cities/Xia.map");
    check(*a==*b,"Repeated real original source preparation differed");
    const auto source=maps::read_emperor_map(maps::EmperorContainer::open(maps::resolve_map_path(data,"Cities/Xia.map")),0);
    const auto word=[&](int x,int y){return source.terrain_at(static_cast<std::uint32_t>(x),static_cast<std::uint32_t>(y));};
    for(int side:{2,4})for(int y=1;y<228-side;++y)for(int x=1;x<228-side;++x) {
        bool edge=false;for(int i=0;i<side;++i)edge|=((word(x-1,y+i)|word(x+side,y+i)|word(x+i,y-1)|word(x+i,y+side))&4)!=0;
        check(a->cardinal_water_bonus({x,y},side)==(edge?10:0),"Real raw-word cardinal oracle differs from prepared immutable facts");
    }
}
}
int main(int argc,char** argv) {
    try{check(argc==1||(argc==3&&std::string_view(argv[1])=="--real"),"Use no args or --real DATA");
        identities_and_recipient_authority();paid_lifecycle_and_persistence();if(argc==3)real_source_only(argv[2]);
        std::cout<<"PASS: authored paid Rule14 recipient query/lifecycle, strict Schema30/recovery and complete continuation; no paid Elite17 claim\n";return 0;
    }catch(const std::exception& e){std::cerr<<"Cardinal water FAIL: "<<e.what()<<'\n';return 1;}
}
