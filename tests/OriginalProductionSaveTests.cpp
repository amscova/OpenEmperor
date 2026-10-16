#include "OriginalInfluenceFixture.h"
#include "persistence/RecoveryStore.h"
#include <nlohmann/json.hpp>
#include <array>
#include <functional>
#include <iostream>

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
    throw std::runtime_error("Schema26 admitted malformed production authority");
}
struct Fixture {
    fixture::Root root;
    std::shared_ptr<const sim::MapPermissions> policy=fixture::permissions();
    std::shared_ptr<const original::OriginalRulesModel> model=[&] {
        // Independently authored capacity37 supports genuine timed migration;
        // original model data and existing historical fixtures remain untouched.
        write_text(root.data/"Model/EmperorBuildingModels.txt",oe_save_fixture::authored_model_text(1));
        return save::prepare_original_rules_model(root.data);
    }();
    sim::World world=sim::World::create_original_production_development(policy,model);
    fs::path target=root.path/"protected-schema26.json";
    save::SaveDocument document(const sim::World& value) const {
        return save::make_document(root.data,fixture::map_relative,policy->building_mask(),value);
    }
    save::SaveDocument document() const { return document(world); }
    sim::World restore(const save::SaveDocument& d,
            std::shared_ptr<const original::OriginalRulesModel> prepared=nullptr) const {
        return save::restore_original_production_development(d,root.data,policy->building_mask(),policy,std::move(prepared));
    }
    void write(const save::SaveDocument& d,save::WriteFault fault=save::WriteFault::None) const {
        save::write_save(target,d,root.data,policy->building_mask(),fault);
    }
    void scene() {
        check(world.treasury()==12000 && world.ticks()==0,"Wrong paid-production starting contract");
        for (int x=0;x<24;++x) fixture::buy(world,{sim::CommandType::PlaceRoad,{x,4}});
        for (int i=0;i<4;++i) fixture::buy(world,{sim::CommandType::PlaceHousehold,{2+3*i,2}});
        fixture::buy(world,{sim::CommandType::PlaceClaySource,{2,5}});
        fixture::buy(world,{sim::CommandType::PlacePottery,{7,5}});
        fixture::buy(world,{sim::CommandType::PlaceWarehouse,{12,5}});
        check(world.construction_spent_total()==818 && world.treasury()==11182,
              "Scene did not pay for roads, four homes and all producers");
        for (const auto& h:world.original_economy_state()->houses)
            check(h.house.occupants==0,"Purchasing a house granted immediate residents");
        check(world.original_goods_balance(original::Goods::Ceramics).stock==0,
              "Purchasing a warehouse granted Ceramics");
    }
    std::string roundtrip() {
        const auto before=world.snapshot();const auto d=document();write(d);
        const auto bytes=read_text(target);const auto j=json::parse(bytes);
        check(j.at("schema_version")==26 && j.at("rules").at("version")==10 &&
              j.at("world").at("original_production").at("development_revision")==14,
              "Rule10 did not use its own strict working generation");
        const auto parsed=save::read_save(target);fixture::same_document(d,parsed);
        check(parsed.source_schema_version==26 && !parsed.prepared_permissions && !parsed.prepared_original_rules_model,
              "JSON acquired prepared runtime dependencies");
        auto restored=restore(parsed);check(restored.snapshot()==before,"Full schema26 roundtrip changed authority");
        auto retained=restore(parsed,model);
        check(restored.original_rules_model()!=model && retained.original_rules_model()==model,
              "Independent restore or explicitly retained session model changed binding");
        auto rewritten=parsed;rewritten.prepared_permissions=policy;rewritten.prepared_original_rules_model=model;
        write(rewritten);check(read_text(target)==bytes,"Canonical schema26 rewrite changed bytes");
        check(world.snapshot()==before,"Save validation mutated the active World");
        refuses([&] { (void)save::restore_save(parsed,root.data,policy->building_mask(),policy); });
        refuses([&] { (void)save::restore_original_goods_development(parsed,root.data,policy->building_mask(),policy,model); });
        refuses([&] { (void)save::restore_original_influence_development(parsed,root.data,policy->building_mask(),policy,model); });
        refuses([&] { (void)save::restore_original_economy_development(parsed,root.data,policy->building_mask(),policy,model); });
        return bytes;
    }
};
void actual_production_and_midroute(Fixture& f) {
    f.scene();f.roundtrip();
    bool midroute=false;
    for (int tick=0;tick<20000 && !midroute;++tick) {
        f.world.tick();
        for (const auto& c:f.world.couriers())
            if (c.role==sim::CourierRole::Pottery && c.phase==sim::CourierPhase::ToWarehouse &&
                    c.cargo>0 && c.edge_progress>0) { midroute=true;break; }
    }
    check(midroute,"Paid staffed recipe never produced a moving Pottery courier");
    check(f.world.original_production_state()->residents_admitted>0 && f.world.total_population()>0,
          "Production ran without genuine admitted residents");
    f.roundtrip();auto restored=f.restore(save::read_save(f.target));
    for (int tick=0;tick<2000;++tick) { f.world.tick();restored.tick(); }
    check(restored.snapshot()==f.world.snapshot(),"Mid-courier restore diverged during later actual arrivals");
    const auto balance=f.world.original_goods_balance(original::Goods::Ceramics);
    check(balance.supported && balance.conserved && balance.stock>0 && balance.cargo==0 &&
          balance.totals.produced==f.world.original_production_state()->pottery_handed_off &&
          balance.totals.produced==balance.stock,"Arrival did not produce exactly one typed Ceramics account");
    for (const auto& building:f.world.buildings())
        if (building.kind==sim::Object::Warehouse) check(building.pottery_stock==0,"Warehouse retained duplicate Pottery mass");
    const auto canonical=f.roundtrip();const auto other=f.root.path/"restored-schema26.json";
    save::write_save(other,f.document(restored),f.root.data,f.policy->building_mask());
    check(read_text(other)==canonical,"Restored process state produced different canonical save bytes");
}
void strict_json(Fixture& f) {
    const auto pristine=read_text(f.target);const auto good=json::parse(pristine);
    const auto hostile=f.root.path/"hostile-schema26.json";
    const auto reject=[&](const json& value) {
        write_text(hostile,value.dump());refuses([&] { (void)f.restore(save::read_save(hostile),f.model); });
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
        for (const auto& [key,value]:good.at(json::json_pointer(path)).items()) {
            (void)value;j=good;j.at(json::json_pointer(path)).erase(key);reject(j);
        }
    }
    for (const auto& [path,value]:std::vector<std::pair<std::string,json>>{
            {"/schema_version",25},{"/schema_version",24},{"/schema_version",27},
            {"/rules/version",9},{"/rules/version",11},{"/rules/id","city-v13"},
            {"/profiles/map_permissions/version",2},{"/world/ticks",UINT64_MAX},
            {"/world/treasury",12000},{"/world/construction_spent_total",0},
            {"/world/next_building_id",1},{"/world/next_courier_id",1},
            {"/world/taxes_collected_total",1},{"/world/maintenance_spent_total",1},
            {"/world/original_production/development_revision",13},
            {"/world/original_production/development_revision",15},
            {"/world/original_production/residents_admitted",UINT64_MAX},
            {"/world/original_production/pottery_handed_off",UINT64_MAX},
            {"/world/original_economy/model_source/difficulty",5},
            {"/world/original_economy/model_source/format_version",2},
            {"/world/original_economy/model_source/byte_count",0},
            {"/world/original_economy/houses/0/house/occupants",32768},
            {"/world/original_economy/houses/0/house/ceramics",1},
            {"/world/original_goods/stocks/0/building_id",1},
            {"/world/original_goods/stocks/0/amounts/1",-1},
            {"/world/original_goods/stocks/0/amounts/1",INT64_MAX}}) {
        auto j=good;j.at(json::json_pointer(path))=value;reject(j);
    }
    for (std::size_t g=0;g<original::goods_count;++g) {
        for (const char* counter:{"produced","imported","consumed","demolished"}) {
            for (const json& value:{json(-1),json(0.0),json(true),json("0"),json(nullptr),json(UINT64_MAX)}) {
                auto j=good;j["world"]["original_goods"]["totals"][g][counter]=value;reject(j);
            }
        }
        if (g!=static_cast<std::size_t>(original::Goods::Ceramics)) {
            auto j=good;j["world"]["original_goods"]["totals"][g]["produced"]=1;
            j["world"]["original_goods"]["stocks"][0]["amounts"][g]=1;reject(j);
        }
    }
    // A mathematically balanced fake Ceramics origin still has to agree with
    // paid recipe and confirmed-arrival history, not just the six sums.
    { auto j=good;auto& produced=j["world"]["original_goods"]["totals"][1]["produced"];
        produced=produced.get<std::uint64_t>()+1;
        auto& amount=j["world"]["original_goods"]["stocks"][0]["amounts"][1];amount=amount.get<int>()+1;
        reject(j); }
    for (const int delta:{-1,1}) {
        auto j=good;auto& produced=j["world"]["original_goods"]["totals"][1]["produced"];
        auto& handed_off=j["world"]["original_production"]["pottery_handed_off"];
        auto& amount=j["world"]["original_goods"]["stocks"][0]["amounts"][1];
        check(produced.get<std::uint64_t>()>0 && handed_off.get<std::uint64_t>()>0 && amount.get<int>()>0,
              "Coordinated handoff forgery requires an actually stocked scene");
        produced=static_cast<std::uint64_t>(produced.get<std::int64_t>()+delta);
        handed_off=static_cast<std::uint64_t>(handed_off.get<std::int64_t>()+delta);
        amount=amount.get<int>()+delta;reject(j);
    }
    for (const char* key:{"cargo","reservations"}) {
        auto j=good;j["world"]["original_goods"][key]=json::array({json::object()});reject(j);
    }
    { auto j=good;auto& rows=j["world"]["original_goods"]["stocks"];rows.push_back(rows[0]);reject(j); }
    { auto j=good;j["world"]["original_goods"]["stocks"]=json::array();reject(j); }
    { auto j=good;j["world"]["couriers"][0]["owner"]=UINT32_MAX;reject(j); }
    { auto j=good;j["world"]["couriers"][0]["target"]=UINT32_MAX;reject(j); }
    { auto j=good;j["world"]["couriers"][0]["cargo"]=INT32_MAX;reject(j); }
    { auto j=good;j["world"]["couriers"][0]["reserved"]=INT32_MAX;reject(j); }
    { auto j=good;j["world"]["couriers"][0]["path_vertex"]=UINT32_MAX;reject(j); }
    { auto j=good;j["world"]["couriers"][0]["edge_progress"]=INT32_MAX;reject(j); }
    { auto j=good;auto& rows=j["world"]["couriers"];rows.push_back(rows[0]);reject(j); }
    { auto j=good;auto& rows=j["world"]["buildings"];rows.push_back(rows[0]);reject(j); }
    { auto j=good;j["world"]["buildings"][0]["population"]=2;reject(j); }
    for (const char* key:{"supported","capacity","house_stocks"}) {
        auto j=good;j["world"]["original_goods"][key]=0;reject(j);
    }
    for (const auto& marker:{std::string("\"totals\""),std::string("\"original_production\"")}) {
        auto bytes=pristine;const auto at=bytes.find('{',bytes.find(marker));
        check(at!=std::string::npos,"Duplicate-key test object missing");
        bytes.insert(at+1,marker=="\"totals\"" ? "\"good\":0,":"\"development_revision\":14,");
        write_text(hostile,bytes);refuses([&] { (void)save::read_save(hostile); },"duplicate");
    }
    write_text(hostile,"{\"schema_version\":26,"+pristine.substr(1));
    refuses([&] { (void)save::read_save(hostile); },"duplicate");
    { auto j=good;j["deep"]=json::array();auto* at=&j["deep"];
        for (int i=0;i<34;++i) { at->push_back(json::array());at=&(*at)[0]; }
        write_text(hostile,j.dump());refuses([&] { (void)save::read_save(hostile); },"nesting exceeds 32"); }
    constexpr std::size_t limit=8U*1024U*1024U;
    write_text(hostile,std::string(limit-pristine.size(),' ')+pristine);
    fixture::same_document(f.document(),save::read_save(hostile));
    write_text(hostile,std::string(limit-pristine.size()+1,' ')+pristine);
    refuses([&] { (void)save::read_save(hostile); },"exceeds 8 MiB");
    check(read_text(f.target)==pristine,"Malformed reader changed protected target");
}
void atomicity_and_provenance(Fixture& f) {
    const auto d=f.document();const auto pristine=read_text(f.target);const auto state=f.world.snapshot();
    for (const auto fault:{save::WriteFault::BeforeWrite,save::WriteFault::AfterPartialWrite,save::WriteFault::BeforeRename}) {
        refuses([&] { f.write(d,fault); });check(read_text(f.target)==pristine,"Atomic fault changed target");
    }
    for (int kind=0;kind<15;++kind) {
        auto bad=d;
        if (kind==0) bad.map_sha256=std::string(64,'0');
        if (kind==1) bad.buildable_sha256=std::string(64,'0');
        if (kind==2) bad.map_permissions_sha256=std::string(64,'0');
        if (kind==3) bad.world.original_economy->model_source->sha256=std::string(64,'0');
        if (kind==4) ++bad.world.original_economy->model_source->byte_count;
        if (kind==5) bad.source_schema_version=25;
        if (kind==6) bad.world.original_economy.reset();
        if (kind==7) bad.world.original_influence.reset();
        if (kind==8) bad.world.original_goods.reset();
        if (kind==9) bad.world.original_production.reset();
        if (kind==10) ++bad.world.original_production->development_revision;
        if (kind==11) ++bad.world.original_production->pottery_handed_off;
        if (kind==12) ++bad.world.original_production->residents_admitted;
        if (kind==13) ++bad.world.original_goods->totals[1].produced;
        if (kind==14) ++bad.world.treasury;
        refuses([&] { f.write(bad); });refuses([&] { (void)f.restore(bad,f.model); });
        check(read_text(f.target)==pristine,"Rejected authority overwrote protected target");
    }
    const auto hard=save::prepare_original_rules_model(f.root.data,original::Difficulty::Hard);
    refuses([&] { (void)f.restore(d,hard); });
    auto mask=f.policy->building_mask();mask.back()=0;
    refuses([&] { (void)save::restore_original_production_development(d,f.root.data,mask,f.policy,f.model); });
    auto cells=f.policy->cells();cells.back().height=1;
    const auto changed=std::make_shared<const sim::MapPermissions>(fixture::width,fixture::height,3,
        std::move(cells),std::vector<sim::FixedGatePassage>{});
    refuses([&] { (void)save::restore_original_production_development(d,f.root.data,f.policy->building_mask(),changed,f.model); });
    const auto model_file=f.root.data/"Model/EmperorBuildingModels.txt";const auto model_bytes=read_text(model_file);
    write_text(model_file,"Changed source\n"+model_bytes);
    refuses([&] { (void)f.document(); });refuses([&] { (void)f.restore(d,f.model); });refuses([&] { f.write(d); });
    write_text(model_file,model_bytes);
    check(read_text(f.target)==pristine && f.world.snapshot()==state,"Provenance refusal mutated live authority");
    refuses([&] { save::write_save(f.root.data/"forbidden.json",d,f.root.data,f.policy->building_mask()); });
    const auto alias=f.root.path/"save-alias.json";fs::create_symlink(f.target,alias);
    refuses([&] { save::write_save(alias,d,f.root.data,f.policy->building_mask()); });
}
void older_generations(Fixture& f) {
    const auto active=f.document();
    const auto old_world=sim::World::create_original_goods_development(f.policy,f.model);
    const auto old=f.document(old_world);f.write(old);const auto before=read_text(f.target);
    const auto parsed=save::read_save(f.target);
    check(parsed.source_schema_version==25 && parsed.world.rule_version==9 && !parsed.world.original_production,
          "Frozen rule9 schema25 writer acquired production authority");
    refuses([&] { (void)f.restore(parsed,f.model); });
    const auto hostile=f.root.path/"old-production-injection.json";
    auto j=json::parse(before);j["world"]["original_production"]={{"development_revision",14},
        {"residents_admitted",0},{"pottery_handed_off",0}};
    write_text(hostile,j.dump());refuses([&] { (void)save::read_save(hostile); });
    f.write(old);check(read_text(f.target)==before,"Frozen schema25 canonical rewrite changed");
    const auto recovery_path=f.root.path/"recovery";fs::create_directory(recovery_path);
    save::RecoveryStore recovery(recovery_path,f.root.data);
    refuses([&] { (void)recovery.create_history(active,f.policy->building_mask()); });
    f.write(active);
}
} // namespace
int main() {
    try {
        Fixture f;actual_production_and_midroute(f);strict_json(f);atomicity_and_provenance(f);older_generations(f);
        std::cout<<"Schema26 PASS: paid production, actual mid-courier replay, canonical stocks, provenance, atomicity and "
                 <<rejected<<" adversarial refusals\n";return 0;
    } catch (const std::exception& error) { std::cerr<<"Schema26: "<<error.what()<<'\n';return 1; }
}
