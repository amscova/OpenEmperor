#include "persistence/SandboxSave.h"
#include "persistence/RecoveryStore.h"
#include "simulation/World.h"
#include "simulation/original/OriginalScheduler.h"
#include "OriginalEconomySaveFixture.h"
#include <nlohmann/json.hpp>
#include <openssl/evp.h>
#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
namespace fs=std::filesystem;
namespace sim=openemperor::simulation;
namespace save=openemperor::persistence;
namespace original=openemperor::original;
using json=nlohmann::json;
std::size_t rejected=0;
void check(bool value,const char* message) { if (!value) throw std::runtime_error(message); }
template<class F> void refuses(F action) {
    try { action(); } catch (const std::exception&) { ++rejected;return; }
    throw std::runtime_error("Schema23 admitted a malformed or unauthorized state");
}
void write_text(const fs::path& path,const std::string& text) {
    std::ofstream out(path,std::ios::binary);out.write(text.data(),static_cast<std::streamsize>(text.size()));
    check(out.good(),"Cannot write authored schema23 test input");
}
std::string read_text(const fs::path& path) {
    std::ifstream in(path,std::ios::binary);return {std::istreambuf_iterator<char>(in),{}};
}
std::string sha256(const std::string& bytes) {
    std::array<unsigned char,EVP_MAX_MD_SIZE> digest{};unsigned length=0;
    check(EVP_Digest(bytes.data(),bytes.size(),digest.data(),&length,EVP_sha256(),nullptr)==1 && length==32,
        "Cannot hash authored historical save fixture");
    constexpr char hex[]="0123456789abcdef";
    std::string result;
    for (unsigned i=0;i<length;++i) { result+=hex[digest[i]>>4];result+=hex[digest[i]&15]; }
    return result;
}
struct Root {
    fs::path path=fs::canonical(fs::temp_directory_path())/
        ("openemperor-schema23-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    fs::path data=path/"data";
    Root() {
        fs::create_directories(data/"Cities");
        write_text(data/"Cities/Authored.map","Independent schema23 map identity; explicit prepared policy fixture\n");
        oe_save_fixture::write_model(data);
    }
    ~Root() { std::error_code ignored;fs::remove_all(path,ignored); }
};
constexpr int width=24,height=16;
std::shared_ptr<const sim::MapPermissions> permissions() {
    std::vector<sim::MapCellPermission> cells(width*height,
        {true,true,false,0,sim::BuildBlocker::None,sim::BuildBlocker::None});
    for (int y=0;y<height;++y) {
        for (int x=0;x<8;++x) cells[static_cast<std::size_t>(y*width+x)].height=1;
        auto& water=cells[static_cast<std::size_t>(y*width+11)];
        water.building_allowed=water.road_allowed=false;water.bridge_water=true;
    }
    return std::make_shared<const sim::MapPermissions>(width,height,3,std::move(cells),
        std::vector<sim::FixedGatePassage>{});
}
void buy(sim::World& world,sim::Command command) {
    auto result=world.execute(command);
    if (!result.accepted || !result.changed) throw std::runtime_error("Paid schema23 fixture: "+result.reason);
}
void same_document(const save::SaveDocument& a,const save::SaveDocument& b) {
    check(a.map_relative==b.map_relative && a.map_sha256==b.map_sha256 &&
        a.buildable_sha256==b.buildable_sha256 && a.world==b.world &&
        a.map_permissions_policy_version==b.map_permissions_policy_version &&
        a.map_permissions_sha256==b.map_permissions_sha256,
        "Canonical schema23 roundtrip changed complete document authority");
}
struct Fixture {
    Root root;
    std::shared_ptr<const sim::MapPermissions> policy=permissions();
    sim::World world=sim::World::create_original_economy_development(policy,save::original_model_source(root.data));
    fs::path target=root.path/"protected-save.json";
    Fixture() {
        for (int x=4;x<=15;++x) if (x!=11) buy(world,{sim::CommandType::PlaceRoad,{x,5}});
        buy(world,sim::place_bridge({10,5},{12,5}));buy(world,sim::place_ramp({7,5},{8,5}));
        const auto before=world.snapshot();
        buy(world,{sim::CommandType::PlaceHousehold,{3,9}});
        check(world.treasury()==before.treasury-80 && world.total_population()==0 &&
            world.total_population_capacity()==0 && world.couriers().empty() && world.ticks()==0,
            "Paid vacant container granted population, capacity, actors or time");
        buy(world,{sim::CommandType::PlaceHousehold,{14,9}});
    }
    save::SaveDocument document() const {
        return save::make_document(root.data,"Cities/Authored.map",policy->building_mask(),world);
    }
    sim::World restore(const save::SaveDocument& d) const {
        return save::restore_original_economy_development(d,root.data,policy->building_mask(),policy);
    }
    void write(const save::SaveDocument& d,save::WriteFault fault=save::WriteFault::None) const {
        save::write_save(target,d,root.data,policy->building_mask(),fault);
    }
};
void positive_roundtrip(Fixture& f) {
    const auto d=f.document();f.write(d);const auto bytes=read_text(f.target);
    // Independent OE-09 baseline: compile the exact a1a352b test/fixture and
    // headers against retained OE-09 libraries, before the OE-10 rebuild.
    // All 6,778 bytes are authored; the private provenance records source and
    // library hashes. This is a frozen writer result, not a mirrored writer.
    check(bytes.size()==6778 && sha256(bytes)=="9f2392de51302cb7451605eb5c2b7e8aa15cf827801c00cb1b8c21aebf967e6e",
        "OE-09 Tick0 canonical save bytes changed");
    const auto j=json::parse(bytes);
    check(j.at("schema_version")==23 && j.at("rules").at("version")==7 &&
        j.at("world").at("original_economy").at("houses").size()==2,
        "Wrong schema, generation or separate original-house payload");
    const auto parsed=save::read_save(f.target);same_document(d,parsed);
    auto restored=f.restore(parsed);check(restored.snapshot()==f.world.snapshot(),"Complete development restore changed state");
    auto clocked=f.restore(parsed);
    auto expected_clock=clocked.snapshot();++expected_clock.ticks;
    clocked.tick();
    check(clocked.snapshot()==expected_clock,"Rule7 clock granted active economy authority");
    for (const auto& house:parsed.world.original_economy->houses) {
        check(house.house.type==original::HouseType::Vacant && house.house.occupants==0 &&
            house.house.hemp==0 && house.house.ancestor==0 && !house.house.water &&
            house.cycle_phase==0 && !house.access_enabled && house.original_status==0,
            "Vacant persistence granted unavailable original supply");
        refuses([&] { (void)restored.household_level(house.building_id); });
    }
    refuses([&] { (void)save::restore_save(parsed,f.root.data,f.policy->building_mask(),f.policy); });
    refuses([&] { (void)sim::World::restore(parsed.world,f.policy); });
    refuses([&] { restored.import_snapshot(parsed.world); });
    auto repeat=save::make_document(f.root.data,"Cities/Authored.map",f.policy->building_mask(),restored);
    f.write(repeat);check(read_text(f.target)==bytes,"Canonical schema23 rewrite changed bytes");
    for (const auto difficulty:{original::Difficulty::VeryEasy,original::Difficulty::Easy,
            original::Difficulty::Normal,original::Difficulty::Hard,original::Difficulty::VeryHard}) {
        auto w=sim::World::create_original_economy_development(f.policy,save::original_model_source(f.root.data,difficulty));
        auto doc=save::make_document(f.root.data,"Cities/Authored.map",f.policy->building_mask(),w);
        check(f.restore(doc).snapshot()==w.snapshot(),"Valid source difficulty did not roundtrip");
    }
    // Difficulty uses size_t: validate the full underlying domain before I/O,
    // including values whose low 32 bits would otherwise look like VeryEasy.
    for (const auto bad:std::array<std::size_t,3>{5,std::numeric_limits<std::size_t>::max(),
            std::numeric_limits<std::size_t>::max()/2+1})
        refuses([&] { (void)save::original_model_source(f.root.data,static_cast<original::Difficulty>(bad)); });
    check(bytes.find("ALL HOUSES")==std::string::npos && bytes.find("Authored house")==std::string::npos,
        "Model rows or labels leaked into save");
}
void malformed_json(Fixture& f) {
    const auto pristine=read_text(f.target);const auto good=json::parse(pristine);
    const auto hostile=f.root.path/"hostile.json";
    const auto reject=[&](json j) {
        write_text(hostile,j.dump());
        refuses([&] { auto d=save::read_save(hostile);d.prepared_permissions=f.policy;f.write(d); });
        check(read_text(f.target)==pristine,"Malformed save destroyed protected target");
    };
    const std::string base="/world/original_economy";
    const std::string h=base+"/houses/0";
    const std::string state=h+"/house";
    for (const auto& path:std::vector<std::string>{"","/map","/profiles","/profiles/map_permissions","/rules","/world",
            "/world/buildings/0","/world/bridges/0","/world/ramps/0",base,base+"/model_source",h,state}) {
        auto j=good;j.at(json::json_pointer(path))["unknown_authority"]=true;reject(std::move(j));
    }
    for (const auto& path:std::vector<std::string>{base,base+"/model_source",base+"/model_source/sha256",base+"/houses",h+"/building_id",h+"/cycle_phase",state+"/type",state+"/hemp"}) {
        auto j=good;const auto p=json::json_pointer(path);j.at(p.parent_pointer()).erase(p.back());reject(std::move(j));
    }
    for (const auto& field:{"occupants","hemp","ceramics","tea","silk","bronzeware","lacquerware"})
        for (const int value:{-1,1,32768}) { auto j=good;j[json::json_pointer(state+"/"+field)]=value;reject(std::move(j)); }
    for (const auto& field:{"devolve_counter","herbalist","acupuncture","music","acrobat","drama","ancestor","confucian","daoist","buddhist","food_quality"})
        for (const int value:{-1,1,256}) { auto j=good;j[json::json_pointer(state+"/"+field)]=value;reject(std::move(j)); }
    for (const int type:{0,1,3,17,18,65536}) { auto j=good;j[json::json_pointer(state+"/type")]=type;reject(std::move(j)); }
    for (const int value:{-129,1,128}) { auto j=good;j[json::json_pointer(state+"/desirability")]=value;reject(std::move(j)); }
    for (const auto& path:std::vector<std::string>{h+"/cycle_phase",h+"/original_status","/world/buildings/0/population"}) {
        auto j=good;j[json::json_pointer(path)]=1;reject(std::move(j));
    }
    // OE-10 explicitly admits elapsed clock-only time. The old Tick1 refusal
    // becomes the terminal-domain refusal; active House fields remain illegal.
    { auto j=good;j["world"]["ticks"]=std::numeric_limits<std::uint64_t>::max();reject(std::move(j)); }
    for (const auto& path:std::vector<std::string>{"","/world",base})
        for (const char* field:{"clock","scheduler_mapping_version","month","year","phase","step_in_month"}) {
            auto j=good;j.at(json::json_pointer(path))[field]=1;reject(std::move(j));
        }
    for (const auto& path:{h+"/access_enabled",state+"/water"}) { auto j=good;j[json::json_pointer(path)]=true;reject(std::move(j)); }
    for (const auto& path:std::vector<std::string>{state+"/occupants","/world/ticks",base+"/model_source/difficulty"}) {
        for (const json& value:{json(0.0),json(true),json("0"),json(nullptr)}) { auto j=good;j[json::json_pointer(path)]=value;reject(std::move(j)); }
    }
    for (const auto& change:std::vector<std::pair<std::string,json>>{
        {base+"/model_source/difficulty",5},{base+"/model_source/format_version",2},
        {base+"/model_source/byte_count",0},{base+"/model_source/byte_count",262145},
        {base+"/model_source/sha256",std::string(64,'A')},{h+"/building_id",0},
        {h+"/building_id",9999},{"/schema_version",22},{"/rules/version",6},
        {"/profiles/map_permissions/version",2},{"/map/part_index",1}}) {
        auto j=good;j[json::json_pointer(change.first)]=change.second;reject(std::move(j));
    }
    { auto j=good;j[json::json_pointer(base+"/houses")].erase(1);reject(std::move(j)); }
    { auto j=good;auto& a=j[json::json_pointer(base+"/houses")];a.push_back(a[0]);reject(std::move(j)); }
    { auto j=good;auto& a=j[json::json_pointer(base+"/houses")];std::swap(a[0],a[1]);reject(std::move(j)); }
    { auto j=good;j["world"]["buildings"][0]["kind"]="farm";reject(std::move(j)); }
    for (const std::string& duplicate:{std::string("{\"schema_version\":23,")+pristine.substr(1),
            [&] { auto s=pristine;auto at=s.find("\"model_source\"");at=s.find('{',at);s.insert(at+1,"\"format_version\":1,");return s; }()}) {
        write_text(hostile,duplicate);refuses([&] { (void)save::read_save(hostile); });
    }
    check(read_text(f.target)==pristine,"Parser changed protected save");
}
void source_and_atomic_failures(Fixture& f) {
    const auto d=f.document();const auto pristine=read_text(f.target);const auto initial=f.world.snapshot();
    for (const auto fault:{save::WriteFault::BeforeWrite,save::WriteFault::AfterPartialWrite,save::WriteFault::BeforeRename}) {
        refuses([&] { f.write(d,fault); });check(read_text(f.target)==pristine,"Atomic failure changed save");
    }
    for (const auto field:{0,1,2,3,4,5}) {
        auto bad=d;
        if (field==0) bad.map_sha256=std::string(64,'0');
        if (field==1) bad.buildable_sha256=std::string(64,'0');
        if (field==2) bad.map_permissions_sha256=std::string(64,'0');
        if (field==3) bad.world.original_economy->model_source->sha256=std::string(64,'0');
        if (field==4) bad.source_schema_version=22;
        if (field==5) bad.world.original_economy->model_source.reset();
        refuses([&] { f.write(bad); });check(read_text(f.target)==pristine,"Source mismatch changed target");
    }
    auto unbound=sim::World::create_original_economy_development(f.policy);
    refuses([&] { (void)save::make_document(f.root.data,"Cities/Authored.map",f.policy->building_mask(),unbound); });
    const auto model=f.root.data/"Model/EmperorBuildingModels.txt";const auto text=read_text(model);
    write_text(model,"Authored different source\n"+text);
    refuses([&] { (void)f.restore(d); });refuses([&] { f.write(d); });
    write_text(model,"invalid authored model");refuses([&] { (void)f.restore(d); });
    fs::remove(model);refuses([&] { (void)f.restore(d); });write_text(model,text);
    check(f.world.snapshot()==initial && read_text(f.target)==pristine,"Failed source restore mutated live authority or saved bytes");
    f.write(d);check(read_text(f.target)==pristine,"Failed writes changed later valid canonical output");
}
void scheduler_file_roundtrips(Fixture& f) {
    const auto initial=f.document();
    const auto tick0_bytes=read_text(f.target);
    auto running=f.restore(initial);
    constexpr std::array<std::uint64_t,20> boundaries{
        0,34,35,36,38,39,40,50,51,52,7*51+39,15*51+39,
        815,816,817,9791,9792,9793,
        original::scheduler_max_tick-1,original::scheduler_max_tick};
    for (const auto tick:boundaries) {
        if (tick<=9793) {
            // Ordinary time advance reaches every practical boundary. Only the
            // terminal arithmetic controls use explicit adversarial DTO setup.
            while (running.ticks()<tick) running.tick();
        } else {
            auto terminal=initial.world;terminal.ticks=tick;
            running=sim::World::restore_original_economy_development(terminal,f.policy);
        }
        const auto before=running.snapshot();
        auto document=save::make_document(f.root.data,"Cities/Authored.map",f.policy->building_mask(),running);
        f.write(document);
        const auto bytes=read_text(f.target);
        auto parsed=save::read_save(f.target);
        same_document(document,parsed);
        auto restored=f.restore(parsed);
        check(restored.snapshot()==before,"Boundary save changed complete clock authority");
        check(original::next_scheduler_dispatch(restored.ticks())==original::next_scheduler_dispatch(tick),
            "Boundary save changed the next service, evolution or consumption opportunity");
        parsed.prepared_permissions=f.policy;
        f.write(parsed);
        check(read_text(f.target)==bytes,"Boundary canonical rewrite changed bytes");
        if (tick==0) check(bytes==tick0_bytes,"OE-09 Tick0 save acquired scheduler payload or changed bytes");
        auto expected=before;
        if (tick<original::scheduler_max_tick) {
            ++expected.ticks;
            auto reference=sim::World::restore_original_economy_development(before,f.policy);
            reference.tick();restored.tick();
            check(restored.snapshot()==reference.snapshot() && restored.snapshot()==expected,
                "Save/load drifted on the exact next clock-only tick");
        } else {
            refuses([&] { restored.tick(); });
            check(restored.snapshot()==before,"Exhausted clock mutated restored authority");
            f.write(save::make_document(f.root.data,"Cities/Authored.map",f.policy->building_mask(),restored));
            check(read_text(f.target)==bytes,"Exhausted clock could not preserve its saved state");
        }
    }
    auto invalid=initial.world;invalid.ticks=std::numeric_limits<std::uint64_t>::max();
    refuses([&] { (void)sim::World::restore_original_economy_development(invalid,f.policy); });
    // Restore the common Tick0 fixture so its malformed/legacy checks retain
    // their original premise and cannot accidentally accept clock authority.
    f.write(initial);check(read_text(f.target)==tick0_bytes,"Boundary tests changed the historical Tick0 document");
}
void direct_authority_and_recovery(Fixture& f) {
    const auto saved=f.world.snapshot();
    for (int kind=0;kind<9;++kind) {
        auto bad=saved;
        if (kind==0) bad.buildings[0].population=2;
        if (kind==1) bad.buildings[0].fulfilled_demand=1;
        if (kind==2) bad.original_economy->houses[0].building_id=static_cast<sim::BuildingId>(900);
        if (kind==3) bad.next_building_id=1;
        if (kind==4) bad.construction_spent_total=0;
        if (kind==5) bad.treasury=-1;
        if (kind==6) bad.original_economy.reset();
        if (kind==7) bad.original_economy->houses[0].house.ceramics=1;
        if (kind==8) bad.road_revision++;
        refuses([&] { (void)sim::World::restore_original_economy_development(bad,f.policy); });
        check(f.world.snapshot()==saved,"Rejected DTO restore changed live World");
    }
    const auto app=f.root.path/"recovery-app";
    fs::create_directory(app);
    save::RecoveryStore store(app,f.root.data);
    const auto exists=fs::exists(store.root());
    const auto d=f.document();
    refuses([&] { (void)store.create_history(d,f.policy->building_mask()); });
    refuses([&] { (void)store.write_checkpoint("history-123-12345678",d,f.policy->building_mask()); });
    check(fs::exists(store.root())==exists &&
        (!exists || fs::is_empty(store.root())),"Development schema23 entered normal recovery history");
}
void legacy_boundaries(Fixture& f) {
    sim::World old(f.policy,sim::RulesProfile::CityV16,6);
    const auto d=save::make_document(f.root.data,"Cities/Authored.map",f.policy->building_mask(),old);
    const auto path=f.root.path/"legacy.json";
    save::write_save(path,d,f.root.data,f.policy->building_mask());const auto bytes=read_text(path);
    auto j=json::parse(bytes);check(j.at("schema_version")==22 && !j.at("world").contains("original_economy"),"Legacy writer gained new fields");
    const auto payload=f.document().world.original_economy;
    auto bad=d;bad.world.original_economy=payload;
    refuses([&] { save::write_save(path,bad,f.root.data,f.policy->building_mask()); });
    check(read_text(path)==bytes,"Old-new DTO payload overwrote legacy save");
    const auto malicious=f.root.path/"old-new.json";
    for (const bool at_root:{false,true}) {
        auto mixed=j;if (at_root) mixed["original_economy"]=json::object();else mixed["world"]["original_economy"]=json::object();
        write_text(malicious,mixed.dump());refuses([&] { (void)save::read_save(malicious); });
    }
    // Legacy reader permissiveness stays historical; strict unknown/duplicate
    // handling belongs solely to the new generation.
    auto historical=j;historical["historical_extension"]=true;
    write_text(malicious,historical.dump());
    check(save::read_save(malicious).world==old.snapshot(),"Legacy reader lost historical unknown-key tolerance");
    write_text(malicious,std::string("{\"schema_version\":22,")+bytes.substr(1));
    check(save::read_save(malicious).world==old.snapshot(),"Legacy reader lost historical duplicate-key behavior");
    refuses([&] { (void)sim::World::restore(bad.world,f.policy); });
    refuses([&] { (void)f.restore(d); });
    auto reread=save::read_save(path);reread.prepared_permissions=f.policy;
    save::write_save(path,reread,f.root.data,f.policy->building_mask());
    check(read_text(path)==bytes,"Legacy canonical roundtrip changed");
}
} // namespace
int main() {
    try {
        Fixture f;positive_roundtrip(f);malformed_json(f);source_and_atomic_failures(f);scheduler_file_roundtrips(f);direct_authority_and_recovery(f);legacy_boundaries(f);
        std::cout<<"Schema23 PASS: paid vacant houses/infrastructure, canonical persistence and "<<rejected<<" rejected adversarial cases\n";return 0;
    } catch (const std::exception& error) { std::cerr<<"Schema23: "<<error.what()<<'\n';return 1; }
}
