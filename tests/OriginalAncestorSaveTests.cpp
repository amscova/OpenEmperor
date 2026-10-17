#include "OriginalProductionFixture.h"
#include "simulation/original/ServiceRules.h"

#include <nlohmann/json.hpp>
#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <utility>
#include <vector>

namespace {
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace save=openemperor::persistence;
namespace orig=openemperor::original;
using json=nlohmann::json;
using f::check;
std::size_t rejected=0;
template<class F> void refuses(F action) {
    try {action();} catch (const std::exception&) {++rejected;return;}
    throw std::runtime_error("Malformed Ancestor save authority was admitted");
}
template<class P> void until(sim::World& world,P condition,int limit=30000) {
    for (int i=0;i<limit && !condition();++i) world.tick();
    check(condition(),"Paid Ancestor scene did not reach its required actual state");
}
std::uint64_t decays(std::uint64_t tick) {return tick<36 ? 0:1+(tick-36)/51;}
std::uint8_t timer(std::uint64_t tick,std::uint64_t arrival,unsigned duration=40) {
    check(arrival>0 && arrival<=tick,"Independent Ancestor timer received an invalid actual arrival");
    const auto elapsed=decays(tick)-decays(arrival-1);
    return static_cast<std::uint8_t>(elapsed>=duration ? 0:duration-elapsed);
}
const orig::HouseState& house(const sim::World& world,sim::BuildingId id) {
    const auto& rows=world.original_economy_state()->houses;
    const auto it=std::find_if(rows.begin(),rows.end(),[&](const auto& row){return row.building_id==id;});
    check(it!=rows.end(),"Actual Ancestor target House disappeared");return it->house;
}
void witnesses_match(const sim::World& world) {
    for (const auto& w:world.original_production_state()->services.witnesses) {
        check(w.role==orig::ServiceRole::Herbalist || w.role==orig::ServiceRole::Acupuncture || w.role==orig::ServiceRole::Ancestor,
              "A visit fabricated an unsupported category");
        check(orig::service_timer(house(world,w.building_id),w.role)==timer(world.ticks(),w.arrival_tick,w.role==orig::ServiceRole::Ancestor ? 40:96),
              "Saved House timer differs from its own actual arrival and category duration");
    }
    for (const auto& c:world.couriers()) check(!c.cargo && !c.reserved,"Ancestor visitor acquired physical cargo");
    for (const auto& b:world.buildings())
        check(!b.service_until_tick && !b.health_protection_until_tick,"Original visit acquired legacy coverage");
}
struct Fixture {
    f::Root root;
    std::shared_ptr<const sim::MapPermissions> policy=f::permissions();
    std::shared_ptr<const orig::OriginalRulesModel> model=[&] {
        oe_production_fixture::write_model(root.data,7,0,0,0,0,0,1);
        return save::prepare_original_production_rules_model(root.data);
    }();
    sim::World world=sim::World::create_original_production_development(policy,model);
    f::fs::path target=root.path/"ancestor-schema26.json";
    sim::BuildingId herbalist{},acupuncture{},ancestor{};
    explicit Fixture(bool populate=true) {
        if (!populate) return;
        for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,4}});
        for (int x=0;x<18;x+=3) f::buy(world,{sim::CommandType::PlaceHousehold,{x,2}});
        f::buy(world,{sim::CommandType::PlaceHealthPost,{0,5}});
        f::buy(world,{sim::CommandType::PlaceServicePost,{21,5}});
        f::buy(world,{sim::CommandType::PlaceOriginalAncestorShrine,{10,5}});
        herbalist=*world.building_owner_at({0,5});acupuncture=*world.building_owner_at({21,5});ancestor=*world.building_owner_at({10,5});
        for (const auto id:{herbalist,acupuncture,ancestor}) f::buy(world,sim::set_building_operation(id,false));
        check(world.construction_spent_total()==648 && world.treasury()==11352 && world.total_population()==0,
              "Scene did not pay for24 roads, six empty Houses and all three separate providers");
        check(world.original_production_state()->services.providers.size()==3 && world.couriers().size()==3 &&
              world.original_production_state()->services.trips.empty() && world.original_production_state()->services.witnesses.empty(),
              "Provider purchase granted visits or actors without their own paid bodies");
        for (const auto& h:world.original_economy_state()->houses)
            check(!h.house.occupants && !h.house.ancestor,"Purchase granted free residents or Ancestor access");
    }
    save::SaveDocument document(const sim::World& value) const {
        return save::make_document(root.data,f::map_relative,policy->building_mask(),value);
    }
    sim::World restore(const save::SaveDocument& d) const {
        return save::restore_original_production_development(d,root.data,policy->building_mask(),policy);
    }
    std::string roundtrip(const sim::World& value) const {
        witnesses_match(value);const auto before=value.snapshot();const auto doc=document(value);
        save::write_save(target,doc,root.data,policy->building_mask());const auto bytes=f::read_text(target);
        const auto parsed=save::read_save(target);f::same_document(doc,parsed);
        check(parsed.source_schema_version==26 && parsed.world.original_production->development_revision==23,
              "Ancestor save did not require its explicit active revision23");
        auto copy=restore(parsed);check(copy.snapshot()==before,"Ancestor F5/F9 changed the entire snapshot");
        save::write_save(target,document(copy),root.data,policy->building_mask());
        check(f::read_text(target)==bytes && value.snapshot()==before,"Ancestor canonical rewrite mutated bytes or live state");
        return bytes;
    }
    void replay(const sim::World& value) const {
        roundtrip(value);auto expected=sim::World::restore_original_production_development(value.snapshot(),policy,model);
        auto copy=restore(save::read_save(target));
        for (int i=0;i<20000;++i) {
            if (i==10000) {
                f::buy(expected,{sim::CommandType::PlaceRoad,{23,1}});
                f::buy(copy,{sim::CommandType::PlaceRoad,{23,1}});
            }
            expected.tick();copy.tick();
        }
        check(expected.snapshot()==copy.snapshot(),"Actual Ancestor20k continuation with paid road diverged");
        witnesses_match(expected);witnesses_match(copy);
    }
};
struct States {sim::WorldSnapshot midroute,positive,expired;};
States lifecycle(Fixture& f) {
    f.roundtrip(f.world);until(f.world,[&]{return f.world.total_population()==42;});
    check(f.world.original_production_state()->services.trips.empty(),"Paused sources dispatched while residents entered");
    for (const auto id:{f.herbalist,f.acupuncture,f.ancestor}) f::buy(f.world,sim::set_building_operation(id,true));
    until(f.world,[&] {
        for (const auto& c:f.world.couriers())
            if (c.role==sim::CourierRole::OriginalAncestor && c.phase==sim::CourierPhase::ToWarehouse && c.edge_progress>0) return true;
        return false;
    });
    check(f.world.original_production_state()->services.witnesses.empty(),"Ancestor disposition or a begun edge granted a visit");
    const auto midroute=f.world.snapshot();f.replay(f.world);
    for (const auto id:{f.herbalist,f.acupuncture,f.ancestor}) f::buy(f.world,sim::set_building_operation(id,false));
    until(f.world,[&] {
        return std::any_of(f.world.original_production_state()->services.witnesses.begin(),f.world.original_production_state()->services.witnesses.end(),
            [](const auto& w){return w.role==orig::ServiceRole::Herbalist;});
    });
    const auto first=f.world.original_production_state()->services.witnesses.front().building_id;
    check(house(f.world,first).herbalist>0 && !house(f.world,first).ancestor &&
          !orig::needs_met(f.model->house_rules(),orig::ModelHouse::Shelter,house(f.world,first)),
          "Medical arrival substituted for an independently required Ancestor visit");
    until(f.world,[&]{return house(f.world,first).ancestor==40;});
    check(orig::needs_met(f.model->house_rules(),orig::ModelHouse::Shelter,house(f.world,first)),
          "Only an actual40-unit Ancestor arrival must satisfy its named House need");
    f.roundtrip(f.world);
    until(f.world,[&] {
        return std::all_of(f.world.couriers().begin(),f.world.couriers().end(),[](const auto& c){return c.phase==sim::CourierPhase::IdleAtWorkshop;});
    });
    const auto& rows=f.world.original_production_state()->services.witnesses;
    const auto found=std::find_if(rows.begin(),rows.end(),[](const auto& w){return w.role==orig::ServiceRole::Ancestor;});
    check(found!=rows.end() && rows.size()==3,"Paused outgoing sources did not complete all three actual first visits");
    const auto witness=*found;const auto positive=f.world.snapshot();f.roundtrip(f.world);
    for (int phase=0;phase<16;++phase) {
        const auto next=36+decays(f.world.ticks())*51;
        while (f.world.ticks()+1<next) f.world.tick();
        const auto before=house(f.world,witness.building_id).ancestor;
        if (phase==0 || phase==15) f.roundtrip(f.world);
        f.world.tick();
        check(before>0 && house(f.world,witness.building_id).ancestor+1==before,
              "Ancestor countdown did not execute exactly once at one of16 phase35 steps");
    }
    const auto expiry=36+(decays(witness.arrival_tick-1)+39)*51;
    check(expiry>f.world.ticks(),"Actual Ancestor test passed its future40th countdown");
    while (f.world.ticks()+1<expiry) f.world.tick();
    check(house(f.world,witness.building_id).ancestor==1,"Ancestor expired before its actual40th countdown");
    f.roundtrip(f.world);f.replay(f.world);f.world.tick();
    check(f.world.ticks()==expiry && !house(f.world,witness.building_id).ancestor &&
          house(f.world,witness.building_id).herbalist>0 &&
          !orig::needs_met(f.model->house_rules(),orig::ModelHouse::Shelter,house(f.world,witness.building_id)),
          "Ancestor expiry was renewed by idle, return or the longer medical duration");
    const auto expired=f.world.snapshot();f.roundtrip(f.world);
    f::buy(f.world,sim::set_building_operation(f.ancestor,true));
    until(f.world,[&] {
        for (const auto& h:f.world.original_economy_state()->houses) if (h.house.ancestor==40) return true;
        return false;
    });
    check(std::any_of(f.world.original_production_state()->services.trips.begin(),f.world.original_production_state()->services.trips.end(),
        [&](const auto& t){return f.world.courier(t.transfer.courier_id).owner==f.ancestor && t.transfer.sequence>1;}),
        "Actual Ancestor revisit did not retain its strictly increasing trip highwater");
    f.roundtrip(f.world);return {midroute,positive,expired};
}
void movement_clocks(Fixture& f,const sim::WorldSnapshot& genuine) {
    auto actual=sim::World::restore_original_production_development(genuine,f.policy,f.model);
    const auto found=std::find_if(actual.couriers().begin(),actual.couriers().end(),[](const auto& c){return c.role==sim::CourierRole::OriginalAncestor;});
    check(found!=actual.couriers().end(),"Movement-clock test lost its own paid Ancestor actor");
    const auto actor=found->id;
    const auto reject_clock=[&](bool outbound) {
        const auto pristine=f.roundtrip(actual);auto bad=f.document(actual);auto altered=json::parse(pristine);
        auto& state=bad.world.original_production->services;
        const auto trip=std::find_if(state.trips.begin(),state.trips.end(),[&](const auto& t){return t.transfer.courier_id==actor;});
        check(trip!=state.trips.end(),"Actual Ancestor movement lacks its latest trip");
        const auto ti=static_cast<std::size_t>(std::distance(state.trips.begin(),trip));
        auto& json_state=altered["world"]["original_production"]["services"];
        if (outbound) {
            trip->dispatched_tick=actual.ticks();json_state["trips"][ti]["dispatched_tick"]=actual.ticks();
        } else {
            check(trip->arrival_tick && trip->visit_applied,"Return-clock test lacks an actual successful Ancestor arrival");
            trip->arrival_tick=actual.ticks();json_state["trips"][ti]["arrival_tick"]=actual.ticks();
            const auto witness=std::find_if(state.witnesses.begin(),state.witnesses.end(),[&](const auto& w){return w.transfer==trip->transfer;});
            check(witness!=state.witnesses.end() && witness->role==orig::ServiceRole::Ancestor,"Movement-clock test lost its matching40-unit witness");
            witness->arrival_tick=actual.ticks();
            const auto wi=static_cast<std::size_t>(std::distance(state.witnesses.begin(),witness));
            json_state["witnesses"][wi]["arrival_tick"]=actual.ticks();
            auto& houses=bad.world.original_economy->houses;
            const auto home=std::find_if(houses.begin(),houses.end(),[&](const auto& h){return h.building_id==witness->building_id;});
            check(home!=houses.end(),"Movement-clock test lost its actual Ancestor House");
            home->house.ancestor=timer(actual.ticks(),actual.ticks());
            const auto hi=static_cast<std::size_t>(std::distance(houses.begin(),home));
            altered["world"]["original_economy"]["houses"][hi]["house"]["ancestor"]=home->house.ancestor;
        }
        refuses([&]{(void)sim::World::restore_original_production_development(bad.world,f.policy,f.model);});
        refuses([&]{(void)f.restore(bad);});
        refuses([&]{save::write_save(f.target,bad,f.root.data,f.policy->building_mask());});
        const auto path=f.root.path/"bad-ancestor-movement-clock.json";f::write_text(path,altered.dump());
        refuses([&]{(void)f.restore(save::read_save(path));});
        check(f::read_text(f.target)==pristine,"Impossible Ancestor movement clock replaced protected data");
    };
    const auto legitimate_next_tick=[&] {
        f.roundtrip(actual);auto restored=f.restore(save::read_save(f.target));actual.tick();restored.tick();
        check(actual.snapshot()==restored.snapshot(),"Legitimate Ancestor movement changed on its first restored tick");
    };
    until(actual,[&] {
        const auto& c=actual.courier(actor);
        return c.phase==sim::CourierPhase::ToWarehouse && c.path_vertex+2==c.path.size() && c.edge_progress==4;
    });
    reject_clock(true);legitimate_next_tick();
    check(actual.courier(actor).phase==sim::CourierPhase::Returning,"Legitimate final Ancestor edge did not actually arrive");
    until(actual,[&] {
        const auto& c=actual.courier(actor);return c.phase==sim::CourierPhase::Returning && c.path_vertex>0 && c.edge_progress>0;
    });
    reject_clock(false);legitimate_next_tick();
    until(actual,[&]{return actual.courier(actor).phase==sim::CourierPhase::IdleAtWorkshop;});
    reject_clock(false);legitimate_next_tick();
}
void hostile(Fixture& f,const States& states) {
    auto world=sim::World::restore_original_production_development(states.positive,f.policy,f.model);
    const auto pristine=f.roundtrip(world);const auto good=json::parse(pristine);const auto badpath=f.root.path/"bad-ancestor.json";
    const std::string base="/world/original_production/services";
    const auto reject=[&](const json& value) {
        f::write_text(badpath,value.dump());
        try {refuses([&]{(void)f.restore(save::read_save(badpath));});}
        catch (const std::exception& e) {throw std::runtime_error(std::string(e.what())+": "+json::diff(good,value).dump());}
        check(f::read_text(f.target)==pristine,"Malformed Ancestor input replaced the valid protected save");
    };
    const auto& services=good.at(json::json_pointer(base));
    std::size_t pi=0,ti=0,wi=0,hi=0,ci=0;
    while (services["providers"][pi]["type"]!=2) ++pi;
    while (services["witnesses"][wi]["role"]!=5) ++wi;
    const auto actor=services["witnesses"][wi]["transfer"]["courier_id"];
    while (services["trips"][ti]["transfer"]["courier_id"]!=actor) ++ti;
    while (good["world"]["couriers"][ci]["id"]!=actor) ++ci;
    const auto target=services["witnesses"][wi]["building_id"];
    while (good["world"]["original_economy"]["houses"][hi]["building_id"]!=target) ++hi;
    const auto provider=base+"/providers/"+std::to_string(pi);
    const auto trip=base+"/trips/"+std::to_string(ti);
    const auto witness=base+"/witnesses/"+std::to_string(wi);
    const auto home="/world/original_economy/houses/"+std::to_string(hi)+"/house/";
    std::vector<std::string> objects;
    const std::function<void(const json&,std::string)> visit=[&](const json& value,std::string path) {
        if (value.is_object()) {
            objects.push_back(path);
            for (const auto& [key,child]:value.items()) visit(child,path+"/"+key);
        } else if (value.is_array()) {
            for (std::size_t i=0;i<value.size();++i) visit(value[i],path+"/"+std::to_string(i));
        }
    };
    visit(services,base);
    for (const auto& path:objects) {
        auto value=good;value.at(json::json_pointer(path))["unsupported_ancestor_authority"]=true;reject(value);
        for (const auto& [key,child]:good.at(json::json_pointer(path)).items()) {
            (void)child;value=good;value.at(json::json_pointer(path)).erase(key);reject(value);
        }
    }
    for (const auto& [path,replacement]:std::vector<std::pair<std::string,json>>{
            {base,nullptr},{provider+"/type",0},{provider+"/type",1},{provider+"/type",3},
            {witness+"/role",0},{witness+"/role",1},{witness+"/role",2},{witness+"/role",3},
            {witness+"/role",4},{witness+"/role",6},{witness+"/role",7},{witness+"/role",8},
            {witness+"/provider_id",services["providers"][0]["building_id"]},
            {trip+"/target",services["providers"][pi]["building_id"]},
            {trip+"/arrival_tick",nullptr},{trip+"/visit_applied",false},{trip+"/visit_applied",1},
            {home+"ancestor",0},{home+"ancestor",41},{home+"ancestor",96},{home+"ancestor",255},
            {home+"daoist",40},{home+"confucian",40},{home+"buddhist",40},
            {"/world/couriers/"+std::to_string(ci)+"/role",5},
            {"/world/original_production/development_revision",19},{"/world/original_production/development_revision",20},{"/world/original_production/development_revision",21},{"/world/original_production/development_revision",22},{"/world/original_production/development_revision",24}}) {
        auto value=good;value.at(json::json_pointer(path))=replacement;reject(value);
    }
    for (const auto& path:{provider,base}) {
        auto value=good;value.at(json::json_pointer(path))["elite_only"]=false;reject(value);
        value.at(json::json_pointer(path))["elite_only"]=true;reject(value);
    }
    for (const auto& path:{provider+"/building_id",trip+"/target",trip+"/transfer/courier_id",witness+"/building_id",witness+"/provider_id"})
        for (const json& invalid:{json(0),json(-1),json(true),json(0.0),json("1"),json(nullptr),json(UINT32_MAX),json(UINT64_MAX)}) {
            auto value=good;value.at(json::json_pointer(path))=invalid;reject(value);
        }
    for (const auto& path:{trip+"/dispatched_tick",trip+"/arrival_tick",trip+"/transfer/sequence",witness+"/arrival_tick",witness+"/transfer/sequence"})
        for (const json& invalid:{json(0),json(-1),json(true),json(0.0),json("1"),json(UINT64_MAX)}) {
            auto value=good;value.at(json::json_pointer(path))=invalid;reject(value);
        }
    for (int fault=0;fault<3;++fault) {
        auto value=good;auto& t=value.at(json::json_pointer(trip));auto& w=value.at(json::json_pointer(witness));
        const auto now=value["world"]["ticks"].get<std::uint64_t>();
        if (fault==0) {t["arrival_tick"]=now+1;w["arrival_tick"]=now+1;}
        if (fault==1) {
            const auto early=t["dispatched_tick"].get<std::uint64_t>()+1;
            t["arrival_tick"]=early;w["arrival_tick"]=early;
            value.at(json::json_pointer(home+"ancestor"))=timer(now,early);
        }
        if (fault==2) {w["transfer"]["sequence"]=t["transfer"]["sequence"].get<std::uint64_t>()+1;}
        reject(value);
    }
    for (const char* key:{"providers","trips","witnesses"}) {
        auto value=good;auto& rows=value.at(json::json_pointer(base))[key];rows.push_back(rows[0]);reject(value);
        value=good;auto& reverse=value.at(json::json_pointer(base))[key];std::swap(reverse[0],reverse[1]);reject(value);
        for (const json& invalid:{json(nullptr),json(true),json(0),json::object()}) {
            value=good;value.at(json::json_pointer(base))[key]=invalid;reject(value);
        }
    }
    auto absent=sim::World::restore_original_production_development(states.midroute,f.policy,f.model);
    f::write_text(badpath,f.roundtrip(absent));auto no_visit=json::parse(f::read_text(badpath));
    no_visit["world"]["original_economy"]["houses"][hi]["house"]["ancestor"]=1;
    f::write_text(badpath,no_visit.dump());refuses([&]{(void)f.restore(save::read_save(badpath));});
    auto expired=sim::World::restore_original_production_development(states.expired,f.policy,f.model);
    auto stale=json::parse(f.roundtrip(expired));stale["world"]["original_economy"]["houses"][hi]["house"]["ancestor"]=1;
    f::write_text(badpath,stale.dump());refuses([&]{(void)f.restore(save::read_save(badpath));});
    f.roundtrip(world);
    save::write_save(badpath,f.document(f.world),f.root.data,f.policy->building_mask());
    auto compacted=json::parse(f::read_text(badpath));auto& history=compacted["world"]["original_production"]["services"];
    bool compacted_probe=false;
    for (auto& old:history["witnesses"]) {
        if (old["role"]!=5) continue;
        const auto latest=std::find_if(history["trips"].begin(),history["trips"].end(),[&](const auto& t) {
            return t["transfer"]["courier_id"]==old["transfer"]["courier_id"];
        });
        if (latest==history["trips"].end() || old["transfer"]["sequence"]>=(*latest)["transfer"]["sequence"]) continue;
        old["arrival_tick"]=122;
        for (auto& h:compacted["world"]["original_economy"]["houses"])
            if (h["building_id"]==old["building_id"])
                h["house"]["ancestor"]=timer(compacted["world"]["ticks"].get<std::uint64_t>(),122);
        compacted_probe=true;break;
    }
    check(compacted_probe,"A genuine Ancestor revisit did not leave compacted first-House history");
    f::write_text(badpath,compacted.dump());refuses([&]{(void)f.restore(save::read_save(badpath));});
    check(f::read_text(f.target)==pristine,"Compacted Ancestor forgery replaced the protected save");
    const auto doc=f.document(world);
    for (int fault=0;fault<7;++fault) {
        auto bad=doc;auto& state=bad.world.original_production->services;
        if (fault==0) state.providers.erase(state.providers.begin()+static_cast<std::ptrdiff_t>(pi));
        if (fault==1) state.trips.erase(state.trips.begin()+static_cast<std::ptrdiff_t>(ti));
        if (fault==2) state.witnesses.erase(state.witnesses.begin()+static_cast<std::ptrdiff_t>(wi));
        if (fault==3) state.witnesses[wi].arrival_tick=bad.world.ticks+1;
        if (fault==4) bad.world.original_economy->houses[hi].house.ancestor=96;
        if (fault==5) state.trips[ti].transfer.sequence=UINT64_MAX;
        if (fault==6) bad.world.original_production->development_revision=19;
        refuses([&]{save::write_save(f.target,bad,f.root.data,f.policy->building_mask());});
        check(f::read_text(f.target)==pristine,"Bad Ancestor writer replaced the valid protected save");
    }
    auto duplicate=pristine;const auto marker=duplicate.find("\"services\": {");
    check(marker!=std::string::npos,"Actual services object missing for duplicate-key test");
    duplicate.insert(duplicate.find('{',marker)+1,"\"witnesses\":[],");f::write_text(badpath,duplicate);
    refuses([&]{(void)save::read_save(badpath);});
    auto bad=doc;++bad.world.construction_spent_total;--bad.world.treasury;
    sim::World::validate_original_production_snapshot(bad.world);
    refuses([&]{(void)f.restore(bad);});
    refuses([&]{save::write_save(f.target,bad,f.root.data,f.policy->building_mask());});
    check(f::read_text(f.target)==pristine,"Modelbound Ancestor cost refusal replaced valid data");
}
void maximum_context() {
    Fixture f(false);
    for (const int y:{4,7,10,13,14}) for (int x=0;x<24;++x) f::buy(f.world,{sim::CommandType::PlaceRoad,{x,y}});
    for (const int y:{5,6,8,9,11,12}) f::buy(f.world,{sim::CommandType::PlaceRoad,{20,y}});
    for (int row=0;row<4;++row) for (int col=0;col<5;++col)
        f::buy(f.world,{sim::CommandType::PlaceHousehold,{2+3*col,2+3*row}});
    std::vector<sim::BuildingId> providers;
    for (const auto& [type,x]:std::array{
            std::pair{sim::CommandType::PlaceHealthPost,0},std::pair{sim::CommandType::PlaceHealthPost,3},
            std::pair{sim::CommandType::PlaceServicePost,6},std::pair{sim::CommandType::PlaceServicePost,9},
            std::pair{sim::CommandType::PlaceOriginalAncestorShrine,12},std::pair{sim::CommandType::PlaceOriginalAncestorShrine,15}}) {
        f::buy(f.world,{type,{x,15}});const auto id=*f.world.building_owner_at({x,15});providers.push_back(id);
        f::buy(f.world,sim::set_building_operation(id,false));
    }
    check(f.world.construction_spent_total()==2092 && f.world.total_population()==0 &&
          f.world.original_production_state()->services.providers.size()==6 && f.world.couriers().size()==6,
          "Full service context was not bought through ordinary commands and model prices");
    until(f.world,[&]{return f.world.total_population()==140;});
    for (const auto id:providers) f::buy(f.world,sim::set_building_operation(id,true));
    until(f.world,[&]{return f.world.original_production_state()->services.witnesses.size()==60;});
    check(f.world.original_production_state()->services.trips.size()==6,"Six actual provider tours did not retain six latest trip rows");
    const auto pristine=f.roundtrip(f.world);const auto good=json::parse(pristine);const auto badpath=f.root.path/"oversized-ancestor-context.json";
    for (const char* key:{"providers","trips","witnesses"}) {
        auto value=good;auto& rows=value["world"]["original_production"]["services"][key];rows.push_back(rows[0]);
        f::write_text(badpath,value.dump());refuses([&]{(void)save::read_save(badpath);});
        check(f::read_text(f.target)==pristine,"Oversized service collection replaced the valid maximum context");
    }
    const auto before=f.world.snapshot();const auto third=f.world.execute({sim::CommandType::PlaceOriginalAncestorShrine,{18,15}});
    check(!third.accepted && !third.changed && f.world.snapshot()==before,"Third Ancestor source bypassed paid source bounds or mutated the World");
}
}
int main() {
    try {
        Fixture fixture;const auto states=lifecycle(fixture);
        movement_clocks(fixture,states.midroute);hostile(fixture,states);maximum_context();
        std::cout<<"Ancestor save PASS: paid214 actual40 visit/expiry/revisit, independent96 medical timers,20k paid replay, real6/6/60 context and "
                 <<rejected<<" hostile refusals\n";return 0;
    } catch (const std::exception& error) {
        std::cerr<<"Original Ancestor save FAIL: "<<error.what()<<'\n';return 1;
    }
}
