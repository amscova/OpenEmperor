#include "OriginalInfluenceFixture.h"
#include "persistence/RecoveryStore.h"
#include "simulation/original/OriginalScheduler.h"
#include <nlohmann/json.hpp>
#include <array>
#include <functional>
#include <iostream>
#include <limits>

namespace {
namespace fixture=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace save=openemperor::persistence;
namespace original=openemperor::original;
namespace fs=std::filesystem;
using json=nlohmann::json;
using fixture::check;
using fixture::read_text;
using fixture::write_text;
std::size_t rejected=0;
template<class F> void refuses(F action,const std::string& reason={}) {
    try { action(); } catch (const std::exception& error) {
        check(reason.empty() || std::string(error.what()).find(reason)!=std::string::npos,
              "Unexpected refusal: "+std::string(error.what()));
        ++rejected;return;
    }
    throw std::runtime_error("Schema25 admitted malformed or unsupported goods authority");
}
void zero_goods(const sim::World& world) {
    check(world.original_goods_state() && *world.original_goods_state()==sim::OriginalGoodsState{},
          "Runtime goods state acquired stock, cargo, reservation or historical credit");
    for (const auto& balance:world.original_goods_balances())
        check(!balance.supported && balance.conserved && balance.totals==sim::OriginalGoodsTotals{} &&
              balance.stock==0 && balance.cargo==0 && balance.source_reserved==0 && balance.incoming_reserved==0,
              "Unsupported runtime goods balance is nonzero or unconserved");
}
struct Fixture {
    fixture::Root root;
    std::shared_ptr<const sim::MapPermissions> policy=fixture::permissions();
    std::shared_ptr<const original::OriginalRulesModel> model=save::prepare_original_rules_model(root.data);
    sim::World world=sim::World::create_original_goods_development(policy,model);
    fixture::Scene scene=fixture::build_scene(world);
    fs::path target=root.path/"protected-schema25.json";
    save::SaveDocument document(const sim::World& value) const {
        return save::make_document(root.data,fixture::map_relative,policy->building_mask(),value);
    }
    save::SaveDocument document() const { return document(world); }
    sim::World restore(const save::SaveDocument& d,
            std::shared_ptr<const original::OriginalRulesModel> prepared=nullptr) const {
        return save::restore_original_goods_development(d,root.data,policy->building_mask(),policy,std::move(prepared));
    }
    void write(const save::SaveDocument& d,save::WriteFault fault=save::WriteFault::None) const {
        save::write_save(target,d,root.data,policy->building_mask(),fault);
    }
    void roundtrip() {
        zero_goods(world);
        const auto before=world.snapshot();const auto cache=world.original_influence_cache_build_count();
        const auto d=document();write(d);const auto bytes=read_text(target);const auto j=json::parse(bytes);
        check(j.at("schema_version")==25 && j.at("rules").at("version")==9,
              "Rule9 fell through to an older schema writer");
        const auto& goods=j.at("world").at("original_goods");
        check(goods.size()==4 && goods.at("totals").size()==6 && goods.at("stocks").empty() &&
              goods.at("cargo").empty() && goods.at("reservations").empty(),"Wrong inactive goods payload shape");
        for (std::size_t i=0;i<6;++i)
            check(goods.at("totals").at(i)==json{{"good",i},{"produced",0},{"imported",0},
                  {"consumed",0},{"demolished",0}},"Goods identity or zero history changed");
        auto parsed=save::read_save(target);fixture::same_document(d,parsed);
        check(parsed.source_schema_version==25 && !parsed.prepared_original_rules_model && !parsed.prepared_permissions,
              "JSON acquired a prepared session dependency");
        auto restored=restore(parsed);check(restored.snapshot()==before,"Complete Schema25 restore changed state");
        check(restored.original_rules_model()!=model &&
              restored.original_rules_model()->provenance().sha256==model->provenance().sha256,
              "Independent F9 preparation lost immutable source identity");
        auto retained=restore(parsed,model);zero_goods(restored);zero_goods(retained);
        check(retained.original_rules_model()==model && d.prepared_original_rules_model==model &&
              world.original_rules_model()==model,"F5 or explicit restore replaced the bound model instance");
        restored.tick();retained.tick();auto expected=before;++expected.ticks;
        check(restored.snapshot()==expected && retained.snapshot()==expected,"Restored scheduler diverged on next tick");
        parsed.prepared_permissions=policy;parsed.prepared_original_rules_model=model;
        write(parsed);check(read_text(target)==bytes,"Canonical Schema25 rewrite changed bytes");
        check(world.snapshot()==before && world.original_influence_cache_build_count()==cache,
              "Save/restore validation mutated the active World or rebuilt its cache");
        refuses([&] { (void)save::restore_save(parsed,root.data,policy->building_mask(),policy); });
        refuses([&] { (void)save::restore_original_economy_development(parsed,root.data,policy->building_mask(),policy,model); });
        refuses([&] { (void)save::restore_original_influence_development(parsed,root.data,policy->building_mask(),policy,model); });
    }
};
void lifecycle_roundtrips(Fixture& f) {
    f.roundtrip();
    for (int i=0;i<51;++i) f.world.tick();
    fixture::buy(f.world,sim::demolish_building(f.scene.a));
    check(f.world.treasury()==200 && f.world.construction_spent_total()==1800 &&
          f.world.original_influence_state()->removed_sources.size()==1,"Goods contract changed paid source removal");
    f.roundtrip();f.world.tick();const auto id=fixture::source(f.world,{0,0});
    check(static_cast<unsigned>(id)==23 && f.world.snapshot().next_building_id==24 &&
          f.world.treasury()==140 && f.world.construction_spent_total()==1860,"Goods contract changed source replacement");
    const auto goods=*f.world.original_goods_state();
    for (int i=0;i<20000;++i) f.world.tick();
    check(*f.world.original_goods_state()==goods,"Clock granted material or activated consumption");
    f.roundtrip();
}
void strict_json(Fixture& f) {
    const auto pristine=read_text(f.target);const auto good=json::parse(pristine);
    const auto hostile=f.root.path/"hostile-schema25.json";
    const auto reject=[&](const json& value) {
        write_text(hostile,value.dump());refuses([&] { (void)save::read_save(hostile); });
    };
    std::vector<std::string> objects;
    const std::function<void(const json&,const std::string&)> visit=[&](const json& value,const std::string& path) {
        if (value.is_object()) {
            objects.push_back(path);
            for (auto at=value.begin();at!=value.end();++at) visit(at.value(),path+"/"+at.key());
        } else if (value.is_array())
            for (std::size_t i=0;i<value.size();++i) visit(value[i],path+"/"+std::to_string(i));
    };
    visit(good,"");
    for (const auto& path:objects) {
        auto j=good;j.at(json::json_pointer(path))["unsupported_extra_authority"]=true;reject(j);
        j=good;auto& value=j.at(json::json_pointer(path));value.erase(value.begin());reject(j);
    }
    for (const auto& [path,value]:std::vector<std::pair<std::string,json>>{
            {"/schema_version",24},{"/schema_version",23},{"/schema_version",26},
            {"/rules/version",8},{"/rules/version",7},{"/rules/version",10},{"/rules/id","city-v13"},
            {"/profiles/map_permissions/version",2},{"/world/ticks",UINT64_MAX},
            {"/world/treasury",2000},{"/world/construction_spent_total",0},{"/world/road_revision",0},
            {"/world/next_building_id",23},{"/world/taxes_collected_total",1},
            {"/world/maintenance_spent_total",1},{"/world/next_courier_id",2},
            {"/world/original_economy/model_source/difficulty",5},
            {"/world/original_economy/model_source/format_version",2},
            {"/world/original_economy/model_source/byte_count",0},
            {"/world/original_economy/houses/0/original_status",3},
            {"/world/original_economy/houses/0/access_enabled",true}}) {
        auto j=good;j.at(json::json_pointer(path))=value;reject(j);
    }
    constexpr std::array names{"hemp","ceramics","tea","silk","bronzeware","lacquerware"};
    for (std::size_t i=0;i<names.size();++i) {
        for (const char* counter:{"produced","imported","consumed","demolished"}) {
            for (const json& value:{json(1),json(-1),json(0.0),json(true),json("0"),json(nullptr),json(UINT64_MAX)}) {
                auto j=good;j["world"]["original_goods"]["totals"][i][counter]=value;reject(j);
            }
        }
        // Even balanced mass/history cannot grant a missing runtime capability.
        for (const char* input:{"produced","imported"}) {
            auto j=good;j["world"]["original_goods"]["totals"][i][input]=1;
            j["world"]["original_economy"]["houses"][0]["house"][names[i]]=1;reject(j);
            for (const char* output:{"consumed","demolished"}) {
                j=good;j["world"]["original_goods"]["totals"][i][input]=1;
                j["world"]["original_goods"]["totals"][i][output]=1;reject(j);
            }
        }
        auto j=good;j["world"]["original_economy"]["houses"][0]["house"][names[i]]=32768;reject(j);
    }
    for (const json& id:{json(-1),json(6),json(UINT64_MAX),json(0.0),json(false),json("0"),json(nullptr)}) {
        auto j=good;j["world"]["original_goods"]["totals"][0]["good"]=id;reject(j);
    }
    { auto j=good;auto& rows=j["world"]["original_goods"]["totals"];rows.erase(0);reject(j); }
    { auto j=good;auto& rows=j["world"]["original_goods"]["totals"];rows.push_back(rows[0]);reject(j); }
    { auto j=good;auto& rows=j["world"]["original_goods"]["totals"];std::swap(rows[0],rows[1]);reject(j); }
    { auto j=good;j["world"]["original_goods"]["totals"][1]["good"]=0;reject(j); }
    for (const char* key:{"stocks","cargo","reservations"}) {
        for (const json& value:{json::object(),json(nullptr),json(0),json::array({json::object()}),
                json::array({{{"building_id",1},{"amount",0},{"capacity",INT32_MAX}}})}) {
            auto j=good;j["world"]["original_goods"][key]=value;reject(j);
        }
    }
    for (const char* key:{"supported","capacity","stock","incoming_reserved","house_stocks"}) {
        auto j=good;j["world"]["original_goods"][key]=0;reject(j);
    }
    { auto j=good;j["original_goods"]=j["world"]["original_goods"];reject(j); }
    for (const auto& marker:{std::string("\"totals\""),std::string("\"houses\"")}) {
        auto bytes=pristine;const auto object=bytes.find('{',bytes.find(marker));
        check(object!=std::string::npos,"Duplicate-key control object missing");
        bytes.insert(object+1,marker=="\"totals\"" ? "\"good\":0,":"\"building_id\":1,");
        write_text(hostile,bytes);refuses([&] { (void)save::read_save(hostile); },"duplicate");
    }
    write_text(hostile,"{\"schema_version\":25,"+pristine.substr(1));
    refuses([&] { (void)save::read_save(hostile); },"duplicate");
    { auto j=good;j["deep"]=json::array();auto* at=&j["deep"];
        for (int i=0;i<34;++i) { at->push_back(json::array());at=&(*at)[0]; }
        write_text(hostile,j.dump());refuses([&] { (void)save::read_save(hostile); },"nesting exceeds 32"); }
    constexpr std::size_t limit=8U*1024U*1024U;
    write_text(hostile,std::string(limit-pristine.size(),' ')+pristine);
    fixture::same_document(f.document(),save::read_save(hostile));
    write_text(hostile,std::string(limit-pristine.size()+1,' ')+pristine);
    refuses([&] { (void)save::read_save(hostile); },"exceeds 8 MiB");
    check(read_text(f.target)==pristine,"Malformed reader changed protected save");
}
void authority_and_atomicity(Fixture& f) {
    const auto d=f.document();const auto pristine=read_text(f.target);const auto state=f.world.snapshot();
    const auto cache=f.world.original_influence_cache_build_count();
    for (const auto fault:{save::WriteFault::BeforeWrite,save::WriteFault::AfterPartialWrite,save::WriteFault::BeforeRename}) {
        refuses([&] { f.write(d,fault); });check(read_text(f.target)==pristine,"Atomic fault changed target");
    }
    for (std::size_t i=0;i<original::goods_count;++i) {
        auto bad=d;bad.world.original_goods->totals[i].produced=1;
        bad.world.original_goods->totals[i].consumed=1;
        refuses([&] { f.write(bad); });refuses([&] { (void)f.restore(bad,f.model); });
        bad=d;bad.world.original_goods->totals[i].imported=1;
        bad.world.original_goods->totals[i].demolished=1;
        refuses([&] { f.write(bad); });refuses([&] { (void)f.restore(bad,f.model); });
    }
    for (int kind=0;kind<14;++kind) {
        auto bad=d;
        if (kind==0) bad.map_sha256=std::string(64,'0');
        if (kind==1) bad.buildable_sha256=std::string(64,'0');
        if (kind==2) bad.map_permissions_sha256=std::string(64,'0');
        if (kind==3) bad.world.original_economy->model_source->sha256=std::string(64,'0');
        if (kind==4) ++bad.world.original_economy->model_source->byte_count;
        if (kind==5) bad.source_schema_version=24;
        if (kind==6) bad.world.original_economy->model_source.reset();
        if (kind==7) bad.world.original_influence.reset();
        if (kind==8) bad.world.original_goods.reset();
        if (kind==9) bad.world.original_economy->houses[0].house.ceramics=1;
        if (kind==10) bad.world.original_goods->stocks.push_back({f.scene.b,{}});
        if (kind==11) bad.world.original_goods->cargo.push_back({});
        if (kind==12) bad.world.original_goods->reservations.push_back({});
        if (kind==13) bad.world.couriers.push_back({});
        refuses([&] { f.write(bad); });refuses([&] { (void)f.restore(bad,f.model); });
        check(read_text(f.target)==pristine,"Malformed authority overwrote target");
    }
    const auto hard=save::prepare_original_rules_model(f.root.data,original::Difficulty::Hard);
    refuses([&] { (void)f.restore(d,hard); });auto wrong=d;wrong.prepared_original_rules_model=hard;
    refuses([&] { f.write(wrong); });
    auto changed_cells=f.policy->cells();changed_cells.back().height=1;
    const auto changed_policy=std::make_shared<const sim::MapPermissions>(fixture::width,fixture::height,3,
        std::move(changed_cells),std::vector<sim::FixedGatePassage>{});
    refuses([&] { (void)save::restore_original_goods_development(d,f.root.data,fixture::permissions()->building_mask(),changed_policy,f.model); });
    auto mask=f.policy->building_mask();mask.back()=0;
    refuses([&] { (void)save::restore_original_goods_development(d,f.root.data,mask,f.policy,f.model); });
    fixture::Root second;const auto identical=save::prepare_original_rules_model(second.data);
    auto copied=save::restore_original_goods_development(d,second.data,f.policy->building_mask(),f.policy,identical);
    check(copied.snapshot()==state && copied.original_rules_model()==identical && identical!=f.model,
          "Identical source roots failed independent binding");
    auto model_text=fixture::authored_model_text();const std::string row="72,Well,{,0,12,";
    const auto at=model_text.find(row);check(at!=std::string::npos,"Authored model variant missing");
    model_text.replace(at,row.size(),"72,Well,{,0,13,");write_text(second.data/"Model/EmperorBuildingModels.txt",model_text);
    const auto other=save::prepare_original_rules_model(second.data);
    auto isolated=sim::World::create_original_goods_development(f.policy,other);isolated.tick();zero_goods(isolated);
    check(isolated.original_rules_model()->building_value(original::ModelBuilding::Well,original::BuildingField::Desirability)==13 &&
          f.model->building_value(original::ModelBuilding::Well,original::BuildingField::Desirability)==12,
          "Independent session changed the active immutable model");
    refuses([&] { (void)f.restore(d,other); });
    const auto file=f.root.data/"Model/EmperorBuildingModels.txt";const auto text=read_text(file);
    write_text(file,"Changed provenance\n"+text);
    refuses([&] { (void)f.restore(d,f.model); });refuses([&] { (void)f.document(); });refuses([&] { f.write(d); });
    write_text(file,"invalid authored model");refuses([&] { (void)f.restore(d); });
    fs::remove(file);refuses([&] { (void)f.restore(d,f.model); });refuses([&] { f.write(d); });
    write_text(file,text);
    const auto backup=file.parent_path()/"authored-model.saved";
    fs::rename(file,backup);fs::create_symlink(backup,file);
    // The established reader permits canonical aliases inside the data root.
    // Preserve that admission; only missing targets and root escapes fail.
    check(f.restore(d,f.model).snapshot()==state,"Contained source alias changed the established restore contract");
    f.write(d);check(read_text(f.target)==pristine,"Contained source alias changed canonical save bytes");
    fs::remove(file);
    const auto outside=f.root.path/"outside-authored-model.txt";write_text(outside,text);
    fs::create_symlink(outside,file);
    refuses([&] { (void)f.restore(d,f.model); });refuses([&] { f.write(d); });
    fs::remove(file);fs::create_symlink(f.root.path/"missing-authored-model.txt",file);
    refuses([&] { (void)f.restore(d,f.model); });refuses([&] { f.write(d); });
    fs::remove(file);fs::rename(backup,file);
    const auto map_file=f.root.data/fixture::map_relative;const auto map_text=read_text(map_file);
    write_text(map_file,map_text+"changed");refuses([&] { (void)f.restore(d,f.model); });refuses([&] { f.write(d); });
    write_text(map_file,map_text);
    const auto forbidden=f.root.data/"saves/schema25.json";
    refuses([&] { save::write_save(forbidden,d,f.root.data,f.policy->building_mask()); });
    check(!fs::exists(forbidden.parent_path()),"Protected original root acquired save directories");
    const auto link=f.root.path/"save-link.json";fs::create_symlink(f.target,link);
    refuses([&] { save::write_save(link,d,f.root.data,f.policy->building_mask()); });
    check(f.world.snapshot()==state && f.world.original_rules_model()==f.model &&
          f.world.original_influence_cache_build_count()==cache && read_text(f.target)==pristine,
          "Failed dependency or write changed active state, model, cache or save");
    f.write(d);check(read_text(f.target)==pristine,"Valid retry changed canonical bytes");
}
void clock_limits(Fixture& f) {
    auto terminal=f.world.snapshot();terminal.ticks=original::scheduler_max_tick;
    terminal.command_sequence=UINT64_MAX-1;
    auto world=sim::World::restore_original_goods_development(terminal,f.policy,f.model);
    f.write(f.document(world));auto restored=f.restore(save::read_save(f.target));zero_goods(restored);
    refuses([&] { restored.tick(); });
    check(restored.snapshot()==terminal,"Exhausted restored clock mutated ledger or source authority");
    auto invalid=terminal;invalid.ticks=UINT64_MAX;
    refuses([&] { (void)sim::World::restore_original_goods_development(invalid,f.policy,f.model); });
    invalid=terminal;invalid.command_sequence=UINT64_MAX;
    refuses([&] { (void)sim::World::restore_original_goods_development(invalid,f.policy,f.model); });
    f.write(f.document());
}
void historical_boundaries(Fixture& f) {
    const auto hostile=f.root.path/"historical-with-goods.json";
    for (const unsigned version:{6U,7U,8U}) {
        auto old=version==6 ? sim::World(f.policy,sim::RulesProfile::CityV16,6):
            version==7 ? sim::World::create_original_economy_development(f.policy,f.model):
                         sim::World::create_original_influence_development(f.policy,f.model);
        fixture::buy(old,{sim::CommandType::PlaceHousehold,{2,2}});
        if (version==8) fixture::source(old,{18,6});
        auto document=f.document(old);f.write(document);const auto bytes=read_text(f.target);
        check(json::parse(bytes).at("schema_version")==version+16 && !old.original_goods_state(),
              "Historical world acquired goods schema or state");
        auto parsed=save::read_save(f.target);fixture::same_document(document,parsed);
        auto bad=document;bad.world.original_goods=sim::OriginalGoodsState{};
        refuses([&] { f.write(bad); });check(read_text(f.target)==bytes,"Goods DTO changed protected old save");
        for (const bool root_key:{false,true}) {
            auto j=json::parse(bytes);
            if (root_key) j["original_goods"]=json::object();else j["world"]["original_goods"]=json::object();
            write_text(hostile,j.dump());refuses([&] { (void)save::read_save(hostile); });
            if (version==6) for (unsigned schema=1;schema<=22;++schema) {
                j["schema_version"]=schema;write_text(hostile,j.dump());
                refuses([&] { (void)save::read_save(hostile); },"original goods authority requires schema25");
            }
        }
        refuses([&] { (void)f.restore(document,f.model); });
        f.write(document);check(read_text(f.target)==bytes,"Historical canonical writer bytes changed");
        if (version==6) {
            auto j=json::parse(bytes);j["historical_extension"]=true;write_text(hostile,j.dump());
            check(save::read_save(hostile).world==old.snapshot(),"Old unknown-key tolerance changed");
        }
    }
    const auto recovery_path=f.root.path/"recovery";fs::create_directory(recovery_path);
    save::RecoveryStore recovery(recovery_path,f.root.data);
    refuses([&] { (void)recovery.create_history(f.document(),f.policy->building_mask()); });
    f.write(f.document());
}
} // namespace
int main() {
    try {
        Fixture f;lifecycle_roundtrips(f);strict_json(f);authority_and_atomicity(f);
        clock_limits(f);historical_boundaries(f);
        std::cout<<"Schema25 PASS: inactive goods, paid inherited lifecycle, strict atomic persistence and "
                 <<rejected<<" rejected adversarial cases\n";return 0;
    } catch (const std::exception& error) { std::cerr<<"Schema25: "<<error.what()<<'\n';return 1; }
}
