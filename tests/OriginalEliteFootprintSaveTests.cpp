#include "OriginalProductionFixture.h"

#include <nlohmann/json.hpp>
#include <algorithm>
#include <iostream>

namespace {
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
namespace save=openemperor::persistence;
using json=nlohmann::json;
using f::check;
std::size_t refusals=0;
template<class F> void refuses(F action) {
    try {action();} catch (const std::exception&) {++refusals;return;}
    throw std::runtime_error("Forged Elite geometry/history was accepted");
}
struct Fixture {
    f::Root root;
    std::shared_ptr<const sim::MapPermissions> policy=f::permissions();
    std::shared_ptr<const orig::OriginalRulesModel> model=[&] {
        oe_production_fixture::write_model(root.data,7);
        return save::prepare_original_production_rules_model(root.data);
    }();
    sim::World world=sim::World::create_original_production_development(policy,model);
    sim::BuildingId elite{},common{};
    f::fs::path target=root.path/"elite-footprint-good.json";
    Fixture() {
        for (int x=0;x<8;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,4}});
        f::buy(world,{sim::CommandType::PlaceOriginalElitePlot,{0,0}});
        elite=*world.building_owner_at({0,0});
        f::buy(world,{sim::CommandType::PlaceHousehold,{4,2}});
        common=*world.building_owner_at({4,2});
    }
    const sim::OriginalHouseState& home(const sim::World& w) const {
        for (const auto& h:w.original_economy_state()->houses) if (h.building_id==elite) return h;
        throw std::runtime_error("Missing original Elite authority");
    }
    const sim::OriginalHousingHouse& proof(const sim::World& w) const {
        for (const auto& h:w.original_production_state()->housing.houses) if (h.building_id==elite) return h;
        throw std::runtime_error("Missing actual Elite history");
    }
    save::SaveDocument document(const sim::World& w) const {
        return save::make_document(root.data,f::map_relative,policy->building_mask(),w);
    }
    sim::World restore(const save::SaveDocument& d) const {
        return save::restore_original_production_development(d,root.data,policy->building_mask(),policy);
    }
    sim::World roundtrip(const sim::World& w) const {
        const auto before=w.snapshot();const auto d=document(w);
        save::write_save(target,d,root.data,policy->building_mask());
        const auto bytes=f::read_text(target);const auto read=save::read_save(target);
        f::same_document(d,read);auto copy=restore(read);
        check(copy.snapshot()==before && w.snapshot()==before,"Geometry F5/F9 changed complete authority");
        save::write_save(target,document(copy),root.data,policy->building_mask());
        check(bytes==f::read_text(target),"Geometry canonical save rewrite differs");
        return copy;
    }
    void claims(const sim::World& w,int side,sim::Object kind) const {
        check(w.building(elite).kind==kind,"Restored Elite has the wrong geometric body");
        check(sim::building_footprint(w.profile(),w.rule_version(),kind)==sim::BuildingFootprint{side,side},
              "Restored body footprint differs from its authoritative kind");
        for (int y=0;y<4;++y) for (int x=0;x<4;++x) {
            const bool own=x<side && y<side;
            check((w.building_owner_at({x,y})==elite)==own,"Elite gained or lost a partial cell claim");
            if (own) check(w.object_at({x,y})==kind,"Object/owner geometry differs");
        }
    }
};
void blocked_restart_repair() {
    Fixture f;
    f::buy(f.world,{sim::CommandType::PlaceRoad,{2,2}});
    while (f.world.ticks()<119) f.world.tick();
    const auto empty=f.home(f.world);const auto history=f.proof(f.world);
    f.claims(f.world,2,sim::Object::OriginalElitePlot);
    const auto status=f.world.original_elite_materialization_status(f.elite);
    check(status.pending && !status.ready && status.diagnostic.cell==sim::Cell{2,2},
          "Pending growth lost its real road collision");
    auto copy=f.roundtrip(f.world);
    const auto before=f.world.snapshot();
    const auto demolition=f.world.execute(sim::demolish_building(f.elite));
    check(!demolition.accepted && !demolition.changed && f.world.snapshot()==before,
          "Demolition during pure preparation left partial state");
    f.world.tick();copy.tick();
    check(copy.snapshot()==f.world.snapshot() && f.home(f.world)==empty && f.proof(f.world)==history,
          "Blocked admission or its restart changed empty House authority");
    f.claims(f.world,2,sim::Object::OriginalElitePlot);
    f::buy(f.world,{sim::CommandType::RemoveRoad,{2,2}});
    copy=f.roundtrip(f.world);
    while (f.world.ticks()<159) {f.world.tick();copy.tick();}
    const auto body=f.world.building(f.elite);
    const auto sequence=f.world.command_sequence(),revision=f.world.road_revision();
    const auto spent=f.world.construction_spent_total();const auto funds=f.world.treasury();
    f.world.tick();copy.tick();
    check(copy.snapshot()==f.world.snapshot(),"Restart changed actual repaired materialization");
    f.claims(f.world,4,sim::Object::OriginalEliteHouse);
    check(f.world.building(f.elite).id==body.id && f.world.building(f.elite).cell==body.cell &&
          f.world.building(f.elite).placed_tick==body.placed_tick &&
          f.world.command_sequence()==sequence && f.world.road_revision()==revision+1 &&
          f.world.construction_spent_total()==spent && f.world.treasury()==funds &&
          f.home(f.world).house.type==orig::HouseType::ModestSiheyuan &&
          f.home(f.world).house.occupants==2 && f.proof(f.world).first_admission_tick==160,
          "Materialization invented a command, replacement body, funds or residents");
    copy=f.roundtrip(f.world);
    for (int tick=0;tick<20000;++tick) {f.world.tick();copy.tick();}
    check(copy.snapshot()==f.world.snapshot(),"4x4 in-process20k save continuation differs");
    check(f.home(f.world).house.type==orig::HouseType::EliteAbandoned,
          "Unserved actual Elite did not reach real50-event abandonment");
    f.claims(f.world,4,sim::Object::OriginalEliteHouse);
    (void)f.roundtrip(f.world);
}
void hostile_geometry() {
    Fixture f;
    while (f.world.ticks()<119) f.world.tick();
    (void)f.roundtrip(f.world);
    auto pending=json::parse(f::read_text(f.target));
    const auto badpath=f.root.path/"elite-footprint-hostile.json";
    const auto reject=[&](const json& j) {
        const auto protected_bytes=f::read_text(f.target);const auto live=f.world.snapshot();
        f::write_text(badpath,j.dump());
        refuses([&]{(void)f.restore(save::read_save(badpath));});
        check(f::read_text(f.target)==protected_bytes && f.world.snapshot()==live,
              "Hostile geometry changed the good save or live world");
    };
    pending["world"]["buildings"][0]["kind"]=static_cast<unsigned>(sim::Object::OriginalEliteHouse);
    reject(pending);
    f.world.tick();(void)f.roundtrip(f.world);
    f.claims(f.world,4,sim::Object::OriginalEliteHouse);
    const auto good=json::parse(f::read_text(f.target));
    for (const auto& [path,value]:std::vector<std::pair<std::string,json>>{
            {"/world/buildings/0/kind",static_cast<unsigned>(sim::Object::OriginalElitePlot)},
            {"/world/buildings/0/kind",static_cast<unsigned>(sim::Object::Household)},
            {"/world/buildings/0/kind",41},{"/world/buildings/0/kind",true},{"/world/buildings/0/kind",40.0},
            {"/world/buildings/0/cell",json::array({22,0})},
            {"/world/buildings/1/cell",json::array({2,2})},
            {"/world/original_production/housing/houses/0/first_admission_tick",nullptr},
            {"/world/original_production/housing/houses/0/first_transitions/10",nullptr},
            {"/world/original_production/development_revision",25},
            {"/world/original_production/development_revision",26},
            {"/world/original_production/development_revision",28}}) {
        auto bad=good;bad.at(json::json_pointer(path))=value;reject(bad);
    }
    for (const int delta:{-1,1}) {
        auto bad=good;bad["world"]["road_revision"]=static_cast<std::int64_t>(f.world.road_revision())+delta;
        reject(bad);
    }
    for (const char* key:{"footprint_side","materialized_tick","expansion_pending"}) {
        auto bad=good;bad["world"]["buildings"][0][key]=4;reject(bad);
    }
    {auto bad=good;bad["world"]["original_production"]["materialized_count"]=1;reject(bad);}
    for (const bool erase_proof:{false,true}) {
        auto bad=f.document(f.world);
        if (erase_proof) bad.world.original_production->housing.houses[0].first_admission_tick.reset();
        else bad.world.buildings[0].kind=sim::Object::OriginalElitePlot;
        refuses([&]{(void)f.restore(bad);});
        refuses([&]{save::write_save(f.target,bad,f.root.data,f.policy->building_mask());});
        check(json::parse(f::read_text(f.target))==good,"Rejected direct snapshot overwrote the good document");
    }
    check(sim::building_footprint(f.world.profile(),f.world.rule_version(),f.world.building(f.common).kind)==sim::BuildingFootprint{2,2},
          "Materialized Elite changed Common geometry");
}
}
int main() {
    try {
        blocked_restart_repair();hostile_geometry();
        std::cout<<"Elite footprint Save PASS: real blocked/repaired admission,16claims, unchanged IDs/finance, "
                 <<"canonical restart and20k continuation, no abandonment shrink, "<<refusals<<" hostile refusals\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr<<"Elite footprint Save FAIL: "<<e.what()<<'\n';return 1;
    }
}
