#include "ProcessCommand.h"
#include "app/AutosaveController.h"
#include "maps/EmperorMap.h"
#include "maps/MapGeometry.h"
#include "maps/OriginalMapEntities.h"
#include "maps/SandboxPlacement.h"
#include "persistence/SandboxSave.h"

#include <nlohmann/json.hpp>
#include <zlib.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string_view>

namespace {
namespace fs=std::filesystem;
namespace sim=openemperor::simulation;
namespace maps=openemperor::maps;
namespace save=openemperor::persistence;
using Bytes=std::vector<std::uint8_t>;
constexpr int width=228;
const fs::path relative="Cities/AuthoredRoad.map";
void check(bool ok,const char* message) {if(!ok)throw std::runtime_error(message);}
template<class F> void rejects(F action,std::string_view expected) {
    try {action();} catch(const std::exception& error) {
        if(std::string_view(error.what()).find(expected)==std::string_view::npos)
            throw std::runtime_error(std::string("Wrong persistence rejection: ")+error.what());
        return;
    }
    throw std::runtime_error("Invalid original-road save accepted");
}
void put(Bytes& bytes,std::size_t at,std::uint32_t value,unsigned count=4) {
    for(unsigned i=0;i<count;++i)bytes.at(at+i)=static_cast<std::uint8_t>(value>>(8*i));
}
void append(Bytes& bytes,std::uint32_t value) {
    const auto at=bytes.size();bytes.resize(at+4);put(bytes,at,value);
}
std::size_t index(sim::Cell cell) {return static_cast<std::size_t>(cell.y)*width+static_cast<unsigned>(cell.x);}
Bytes legacy_mask() {
    const maps::MapGeometry geometry{84};Bytes mask(width*width);
    for(unsigned y=0;y<width;++y)for(unsigned x=0;x<width;++x)
        mask[y*width+x]=static_cast<std::uint8_t>(geometry.contains({x,y}));
    return mask;
}
// Independent authored original-map bytes: complete empty manager and ordinary
// 0xc0/objects-zero, on-map, signed-height-zero roads. No saved transit links.
void write_map(const fs::path& data,bool shared_cell) {
    Bytes raw(maps::original_entities_logical_offset+6);
    const Bytes signature{5,0,0xfe,0xca,0,0,2,0};std::copy(signature.begin(),signature.end(),raw.begin());
    put(raw,84,84);
    const auto mask=legacy_mask();
    for(std::size_t i=0;i<mask.size();++i)
        put(raw,maps::terrain_logical_offset+4*i,mask[i] ? 0x80:0x80000);
    const int first=shared_cell ? 116:114,last=shared_cell ? 117:122;
    for(int x=first;x<=last;++x)put(raw,maps::terrain_logical_offset+4*index({x,110}),0xc0);
    put(raw,maps::original_entities_logical_offset,1,2); // wrapper 1, count zero
    Bytes container;append(container,0xfedcbaaa);
    for(std::size_t at=0;at<raw.size();at+=16384) {
        const auto count=std::min<std::size_t>(16384,raw.size()-at);
        uLongf length=compressBound(static_cast<uLong>(count));Bytes compressed(length);
        check(compress2(compressed.data(),&length,raw.data()+at,static_cast<uLong>(count),6)==Z_OK,
              "Authored road map compression failed");
        append(container,0);append(container,static_cast<std::uint32_t>(length));append(container,static_cast<std::uint32_t>(count));
        container.insert(container.end(),compressed.begin(),compressed.begin()+static_cast<std::ptrdiff_t>(length));
    }
    fs::create_directories(data/"Cities");std::ofstream output(data/relative,std::ios::binary);
    output.write(reinterpret_cast<const char*>(container.data()),static_cast<std::streamsize>(container.size()));
    check(output.good(),"Authored road map write failed");
}
void place(sim::World& world,sim::CommandType type,sim::Cell cell) {
    const auto result=world.execute({type,cell});
    if(!result.accepted || !result.changed)throw std::runtime_error(std::string("Paid fixture command: ")+result.reason);
}
sim::World playing(const std::shared_ptr<const sim::MapPermissions>& permissions,bool shared_cell) {
    sim::World world(permissions,sim::RulesProfile::CityV16,4);
    place(world,sim::CommandType::PlaceHousehold,{108,104});place(world,sim::CommandType::PlaceHousehold,{111,104});
    place(world,sim::CommandType::PlaceClaySource,shared_cell ? sim::Cell{116,111}:sim::Cell{112,109});
    place(world,sim::CommandType::PlacePottery,shared_cell ? sim::Cell{116,108}:sim::Cell{124,109});
    if(!shared_cell)place(world,sim::CommandType::PlaceRoad,{123,110});
    return world;
}
const sim::CourierState& clay(const sim::World& world) {
    const auto found=std::find_if(world.couriers().begin(),world.couriers().end(),[](const auto& courier){
        return courier.role==sim::CourierRole::Clay;
    });
    check(found!=world.couriers().end(),"Clay Courier absent");return *found;
}
template<class P> void until(sim::World& world,P ready) {
    for(int i=0;i<1200 && !ready();++i)world.tick();check(ready(),"Original-road trip boundary not reached");
}
void equal_continuation(sim::World reference,sim::World restored,int ticks=250) {
    check(reference.snapshot()==restored.snapshot(),"Original-road paused save/restore differs");
    for(int i=0;i<ticks;++i) {
        reference.tick();restored.tick();
        check(reference.snapshot()==restored.snapshot(),"Original-road save continuation diverged");
    }
}
void overwrite(const fs::path& path,const nlohmann::json& json) {
    std::ofstream output(path);output<<json.dump();check(output.good(),"Tamper fixture write failed");
}
nlohmann::json json_file(const fs::path& path) {std::ifstream input(path);return nlohmann::json::parse(input);}
std::string file_bytes(const fs::path& path) {
    std::ifstream input(path,std::ios::binary);check(input.good(),"Saved byte comparison input missing");
    return {std::istreambuf_iterator<char>(input),std::istreambuf_iterator<char>()};
}
using openemperor::test::shell_quote;
void boundary(const fs::path& root,const Bytes& mask,const sim::World& world,const fs::path& executable,
              std::string_view name,bool repair_on_resume=false) {
    const auto before=world.snapshot();const auto path=root/(std::string(name)+".json");
    const auto document=save::make_document(root/"data",relative,mask,world);
    save::write_save(path,document,root/"data",mask);
    const auto parsed=save::read_save(path);
    check(parsed.source_schema_version==20 && parsed.map_permissions_policy_version==2 &&
          !parsed.prepared_permissions && parsed.map_permissions_sha256==document.map_permissions_sha256,
          "Schema20 did not retain rule4/policy2 map identity");
    auto restored=save::restore_save(parsed,root/"data",mask);
    check(restored.map_permissions()->canonical_state()==world.map_permissions()->canonical_state(),
          "Original-road load did not reconstruct complete immutable permissions");
    auto reference=sim::World::restore(before,world.map_permissions());
    check(restored.snapshot()==before,"Loading original-road save changed current edge or cargo");
    if(repair_on_resume) {
        place(reference,sim::CommandType::PlaceRoad,{123,110});place(restored,sim::CommandType::PlaceRoad,{123,110});
    }
    equal_continuation(reference,restored);
    for(int i=0;i<500;++i)reference.tick();
    save::write_save(root/(std::string(name)+"-expected.json"),
        save::make_document(root/"data",relative,mask,reference),root/"data",mask);
    const auto command=shell_quote(executable.string())+" --process-save "+shell_quote(root.string())+" "+
        shell_quote(name)+" "+(repair_on_resume ? "repair":"continue");
    check(openemperor::test::run_command(command)==0,"Fresh process original-road restore failed");
    check(json_file(root/(std::string(name)+"-expected.json"))==json_file(root/(std::string(name)+"-process.json")),
          "Fresh process full save document differs from uninterrupted continuation");
    check(world.snapshot()==before,"Save/restore mutated source World");
}
void run_case(const fs::path& root,bool shared_cell,const fs::path& executable) {
    fs::create_directories(root);write_map(root/"data",shared_cell);const auto mask=legacy_mask();
    const auto permissions=maps::read_sandbox_map_permissions(root/"data",relative,mask,2);
    const auto historic=maps::read_sandbox_map_permissions(root/"data",relative,mask,1);
    const sim::Cell original=shared_cell ? sim::Cell{116,110}:sim::Cell{118,110};
    check(permissions->original_road(original) && !historic->original_road(original),
          "Road authority was not independently versioned");
    auto world=playing(permissions,shared_cell);
    check(world.object_at(original)==sim::Object::Empty && world.transport_cell(original),
          "Fixed original road was stored as a mutable Sandbox road");
    const auto source=world.buildings()[2].id,target=world.buildings()[3].id;
    const auto route=world.find_building_route(source,target);
    check(route && route->size()==(shared_cell ? 3U:12U),
          "Real building entrances did not share original-road route");
    check(world.snapshot().construction_spent_total==static_cast<std::uint64_t>(shared_cell ? 460:462),
          "Imported original roads were charged or fabricated commands");
    while(world.ticks()<31)world.tick();boundary(root,mask,world,executable,"before-entry");
    until(world,[&]{const auto& courier=clay(world);return courier.phase==sim::CourierPhase::ToWarehouse &&
        courier.path_vertex<courier.path.size() && permissions->original_road(courier.path[courier.path_vertex]) &&
        courier.edge_progress>0;
    });
    check(clay(world).cargo>0 && clay(world).reserved>0,"Actual road trip did not own cargo/reservation");
    boundary(root,mask,world,executable,"on-original-road");
    openemperor::AutosaveController autosave(root/"app",root/"data",true);
    const auto document=save::make_document(root/"data",relative,mask,world);
    check(autosave.begin(document,mask).kind==openemperor::AutosaveResult::Kind::Saved,
          "Schema20 original-road recovery start failed");
    const auto catalog=autosave.store().catalog();
    check(catalog.histories.size()==1 && catalog.histories[0].entries.size()==1 &&
          catalog.histories[0].entries[0].loadable() && catalog.histories[0].entries[0].schema==20,
          "Schema20 recovery metadata did not retain original-road state");
    equal_continuation(sim::World::restore(world.snapshot(),permissions),
        save::restore_save(save::read_save(catalog.histories[0].entries[0].path),root/"data",mask));
    const auto recovery_command=shell_quote(executable.string())+" --process-recovery "+shell_quote(root.string());
    check(openemperor::test::run_command(recovery_command)==0,"Fresh process schema20 recovery reconstruction failed");
    check(json_file(root/"recovery-process.json")==json_file(root/"on-original-road-expected.json"),
          "Fresh process recovery continuation differs from earned manual save");
    if(!shared_cell) {
        const auto removed=world.execute({sim::CommandType::RemoveRoad,{123,110}});
        check(removed.accepted && removed.changed,"Mutable target attachment could not be removed");
        until(world,[&]{return clay(world).route_pending && clay(world).edge_progress==0;});
        check(permissions->original_road(clay(world).path[clay(world).path_vertex]) &&
              clay(world).cargo>0 && clay(world).reserved>0,"Interruption lost original-road wait/cargo/reservation");
        boundary(root,mask,world,executable,"interrupted",true);
        place(world,sim::CommandType::PlaceRoad,{123,110});
    }
    until(world,[&]{return clay(world).phase==sim::CourierPhase::Returning;});
    check(clay(world).cargo==0 && clay(world).reserved==0,"Actual original-road arrival did not release cargo/reservation");
    until(world,[&]{const auto& courier=clay(world);return courier.phase==sim::CourierPhase::Returning &&
        courier.path_vertex<courier.path.size() && permissions->original_road(courier.path[courier.path_vertex]) &&
        courier.edge_progress>0;
    });
    boundary(root,mask,world,executable,"returning");
    until(world,[&]{return clay(world).phase==sim::CourierPhase::IdleAtWorkshop;});
    check(world.buildings()[3].input_clay>0 || world.buildings()[3].active_recipe_clay>0 || world.buildings()[3].recipes_completed>0,
          "Pottery never received actual Clay");
    const auto path=root/"on-original-road.json";
    const auto valid=json_file(path);
    auto changed=valid;changed["profiles"]["map_permissions"]["version"]=1;overwrite(path,changed);
    rejects([&]{save::read_save(path);},"policy version");
    changed=valid;changed["rules"]["version"]=3;overwrite(path,changed);
    rejects([&]{save::read_save(path);},"schema 20 requires");
    changed=valid;changed["schema_version"]=19;overwrite(path,changed);
    rejects([&]{save::read_save(path);},"policy version");
    changed=valid;changed["profiles"]["map_permissions"]["links"]={{1,2}};overwrite(path,changed);
    rejects([&]{save::read_save(path);},"not transit links");
    changed=valid;changed["profiles"]["map_permissions"]["original_roads"]={{116,110}};overwrite(path,changed);
    rejects([&]{save::read_save(path);},"not transit links");
    changed=valid;changed["map"]["sha256"]=std::string(64,'0');overwrite(path,changed);
    rejects([&]{save::restore_save(save::read_save(path),root/"data",mask);},"original map SHA-256");
    changed=valid;changed["profiles"]["map_permissions"]["sha256"]=std::string(64,'0');overwrite(path,changed);
    rejects([&]{save::restore_save(save::read_save(path),root/"data",mask);},"map-permissions SHA-256");
    changed=valid;
    for(auto& courier:changed["world"]["couriers"])if(courier["role"]==static_cast<unsigned>(sim::CourierRole::Clay)) {
        const auto vertex=courier["path_vertex"].get<std::size_t>();courier["path"][vertex]={118,111};
    }
    overwrite(path,changed);rejects([&]{save::restore_save(save::read_save(path),root/"data",mask);},"");
    overwrite(path,valid);const auto parsed=save::read_save(path);
    auto cells=permissions->cells();cells[index(original)].original_road=false;
    rejects([&]{(void)sim::MapPermissions(width,width,2,cells,permissions->gates());},"");
    cells[index(original)].original_road_classification=sim::OriginalRoadClassification::None;
    auto missing_road=std::make_shared<const sim::MapPermissions>(width,width,2,cells,permissions->gates());
    check(save::map_permissions_fingerprint(*missing_road,parsed.map_sha256)!=parsed.map_permissions_sha256,
          "Policy2 fingerprint omitted original-road transport authority");
    rejects([&]{save::restore_save(parsed,root/"data",mask,missing_road);},"map-permissions SHA-256");
    rejects([&]{save::restore_save(parsed,root/"data",mask,historic);},"policy differs");
    auto wrong_document=parsed;wrong_document.map_permissions_sha256=std::string(64,'0');
    rejects([&]{save::write_save(path,wrong_document,root/"data",mask);},"map-permissions SHA-256");
    check(json_file(path)==valid,"Rejected schema20 write replaced valid save");
    const auto metadata=autosave.store().root()/autosave.history_id()/"history.json";
    const auto metadata_before=json_file(metadata);
    rejects([&]{autosave.store().write_checkpoint(autosave.history_id(),wrong_document,mask);},"map-permissions SHA-256");
    check(json_file(metadata)==metadata_before,"Rejected original-road checkpoint changed recovery metadata");
    rejects([&]{autosave.store().write_checkpoint(autosave.history_id(),document,mask,
        {save::WriteFault::BeforeRename,{}});},"");
    check(json_file(metadata)==metadata_before,"Interrupted schema20 checkpoint changed recovery metadata");
    rejects([&]{save::write_save(path,parsed,root/"data",mask,save::WriteFault::BeforeRename);},"");
    check(json_file(path)==valid,"Interrupted schema20 write replaced valid save");
    // Original markings remain visual-only under the historic pair. Rewriting
    // the same old save preserves the entire schema19 document byte for byte.
    sim::World old(historic,sim::RulesProfile::CityV16,3);
    check(!old.transport_cell(original),"Historic rule3 gained original-road transit");
    const auto old_path=root/"historic.json";
    save::write_save(old_path,save::make_document(root/"data",relative,mask,old),root/"data",mask);
    const auto old_json=json_file(old_path);
    const auto old_document=save::read_save(old_path);
    check(old_document.source_schema_version==19 && old_document.map_permissions_policy_version==1,
          "Historic schema19 version identity changed");
    auto old_restored=save::restore_save(old_document,root/"data",mask);
    equal_continuation(old,old_restored);
    const auto old_copy=root/"historic-copy.json";
    save::write_save(old_copy,save::make_document(root/"data",relative,mask,old_restored),root/"data",mask);
    check(json_file(old_copy)==old_json && file_bytes(old_copy)==file_bytes(old_path),
          "Historic schema19 full document bytes changed");
    auto migrated=old_document;migrated.world.rule_version=4;migrated.map_permissions_policy_version=2;
    migrated.map_permissions_sha256=parsed.map_permissions_sha256;
    rejects([&]{save::restore_save(migrated,root/"data",mask,permissions);},"legacy saves are not migrated");
}
} // namespace
int main(int argc,char** argv) {
    try {
        if(argc==3 && std::string_view(argv[1])=="--process-recovery") {
            const fs::path root=argv[2];const auto mask=legacy_mask();
            save::RecoveryStore store(root/"app",root/"data");const auto catalog=store.catalog();
            check(catalog.histories.size()==1 && catalog.histories[0].entries.size()==1,
                  "Fresh process could not read schema20 recovery start");
            const auto entry=catalog.histories[0].entries[0];const auto document=save::read_save(entry.path);
            auto restored=save::restore_save(document,root/"data",mask);
            check(restored.snapshot()==document.world && restored.map_permissions()->policy_version()==2,
                  "Fresh process recovery changed original-road/cargo authority");
            openemperor::AutosaveController child(root/"app",root/"data",true);
            check(child.begin(save::make_document(root/"data",relative,mask,restored),mask,
                save::RecoveryParent{entry.history_id,entry.sequence}).kind==openemperor::AutosaveResult::Kind::Saved,
                  "Fresh process schema20 child history failed");
            for(int i=0;i<500;++i)restored.tick();
            save::write_save(root/"recovery-process.json",save::make_document(root/"data",relative,mask,restored),root/"data",mask);
            return 0;
        }
        if(argc==5 && std::string_view(argv[1])=="--process-save") {
            const fs::path root=argv[2];const std::string name=argv[3];const auto mask=legacy_mask();
            const auto document=save::read_save(root/(name+".json"));auto restored=save::restore_save(document,root/"data",mask);
            check(restored.snapshot()==document.world,"Fresh process changed original-road paused state");
            if(std::string_view(argv[4])=="repair")place(restored,sim::CommandType::PlaceRoad,{123,110});
            for(int i=0;i<500;++i)restored.tick();
            save::write_save(root/(name+"-process.json"),save::make_document(root/"data",relative,mask,restored),root/"data",mask);
            return 0;
        }
        const auto root=fs::canonical(fs::temp_directory_path())/("openemperor-original-road-save-"+
            std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        struct Cleanup {fs::path root;~Cleanup(){std::error_code error;fs::remove_all(root,error);}} cleanup{root};
        run_case(root/"shared",true,fs::absolute(argv[0]));run_case(root/"mixed",false,fs::absolute(argv[0]));
        std::cout<<"Original-road schema20 save/recovery: earned trips, shared cell, partial edges, interruption, restart, tamper and legacy passed\n";
        return 0;
    } catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
}
