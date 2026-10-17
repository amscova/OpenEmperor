#include "OriginalProductionFixture.h"
#include "simulation/original/ServiceRules.h"

#include <nlohmann/json.hpp>
#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <limits>
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
    throw std::runtime_error("Malformed medical service save authority was admitted");
}
template<class P> void until(sim::World& world,P condition,int limit=30000) {
    for (int i=0;i<limit && !condition();++i) world.tick();
    check(condition(),"Paid medical route did not reach its required actual state");
}
std::uint64_t decay_count(std::uint64_t tick) {
    return tick<36 ? 0:1+(tick-36)/51;
}
std::uint8_t expected_timer(std::uint64_t tick,std::uint64_t arrival) {
    check(arrival>0 && arrival<=tick,"Invalid actual visit clock in independent timer check");
    const auto elapsed=decay_count(tick)-decay_count(arrival-1);
    return static_cast<std::uint8_t>(elapsed>=96 ? 0:96-elapsed);
}
const orig::HouseState& house(const sim::World& world,sim::BuildingId id) {
    const auto& houses=world.original_economy_state()->houses;
    const auto row=std::find_if(houses.begin(),houses.end(),[&](const auto& value){return value.building_id==id;});
    check(row!=houses.end(),"Actual visited House disappeared");return row->house;
}
void actual_timers(const sim::World& world) {
    for (const auto& value:world.original_production_state()->services.witnesses)
        check(orig::service_timer(house(world,value.building_id),value.role)==expected_timer(world.ticks(),value.arrival_tick),
              "Medical House timer differs from actual arrival and phase35 countdown");
    for (const auto& building:world.buildings())
        check(!building.service_until_tick && !building.health_protection_until_tick,
              "Original medical visit granted legacy coverage or protection");
    for (const auto& courier:world.couriers())
        check(!courier.cargo && !courier.reserved,"Cargo-free medical visit created material authority");
}
struct Fixture {
    f::Root root;
    std::shared_ptr<const sim::MapPermissions> policy=f::permissions();
    std::shared_ptr<const orig::OriginalRulesModel> model=[&] {
        oe_production_fixture::write_model(root.data,7,0,0,0,1,1);
        return save::prepare_original_production_rules_model(root.data);
    }();
    sim::World world=sim::World::create_original_production_development(policy,model);
    f::fs::path target=root.path/"medical-schema26.json";
    sim::BuildingId herbalist{},acupuncture{};
    Fixture() {
        for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,4}});
        for (int x=0;x<18;x+=3) f::buy(world,{sim::CommandType::PlaceHousehold,{x,2}});
        f::buy(world,{sim::CommandType::PlaceHealthPost,{0,5}});
        f::buy(world,{sim::CommandType::PlaceServicePost,{21,5}});
        herbalist=*world.building_owner_at({0,5});acupuncture=*world.building_owner_at({21,5});
        f::buy(world,sim::set_building_operation(herbalist,false));
        f::buy(world,sim::set_building_operation(acupuncture,false));
        check(world.construction_spent_total()==608 && world.treasury()==11392 && world.total_population()==0,
              "Medical scene did not pay for24 roads, six empty Houses and prepared30/50 providers");
        const auto& services=world.original_production_state()->services;
        check(services.providers.size()==2 && services.trips.empty() && services.witnesses.empty() && world.couriers().size()==2,
              "Provider purchase granted a trip, visit or extra unpurchased actor");
        for (const auto& h:world.original_economy_state()->houses)
            check(!h.house.herbalist && !h.house.acupuncture && !h.house.occupants,
                  "Provider or House purchase granted a medical timer or residents");
    }
    save::SaveDocument document(const sim::World& value) const {
        return save::make_document(root.data,f::map_relative,policy->building_mask(),value);
    }
    sim::World restore(const save::SaveDocument& d) const {
        return save::restore_original_production_development(d,root.data,policy->building_mask(),policy);
    }
    std::string roundtrip(const sim::World& value) const {
        actual_timers(value);const auto before=value.snapshot();const auto d=document(value);
        save::write_save(target,d,root.data,policy->building_mask());const auto bytes=f::read_text(target);
        const auto parsed=save::read_save(target);f::same_document(d,parsed);
        check(parsed.source_schema_version==26 && parsed.world.original_production->development_revision==19,
              "Medical save lost its explicit revision19 identity");
        auto copy=restore(parsed);check(copy.snapshot()==before,"Medical F5/F9 changed the complete snapshot");
        save::write_save(target,document(copy),root.data,policy->building_mask());
        check(f::read_text(target)==bytes && value.snapshot()==before,"Medical rewrite changed bytes or active authority");
        return bytes;
    }
    void replay(const sim::World& value) const {
        roundtrip(value);auto expected=sim::World::restore_original_production_development(value.snapshot(),policy,model);
        auto copy=restore(save::read_save(target));
        for (int i=0;i<20000;++i) {expected.tick();copy.tick();}
        check(expected.snapshot()==copy.snapshot(),"Actual medical midtrip/expiry20k replay diverged");
        actual_timers(expected);actual_timers(copy);
    }
};
struct States {sim::WorldSnapshot midtrip,positive,expired;};
States lifecycle(Fixture& f) {
    f.roundtrip(f.world);
    until(f.world,[&]{return f.world.total_population()==42;});
    check(f.world.original_production_state()->services.trips.empty() &&
          f.world.original_production_state()->services.witnesses.empty(),
          "Paused medical sources dispatched or supplied before ordinary staffing");
    f::buy(f.world,sim::set_building_operation(f.herbalist,true));
    f::buy(f.world,sim::set_building_operation(f.acupuncture,true));
    until(f.world,[&] {
        for (const auto& c:f.world.couriers())
            if (c.phase==sim::CourierPhase::ToWarehouse && c.edge_progress>0 &&
                    !f.world.original_production_state()->services.trips.empty()) return true;
        return false;
    });
    check(f.world.total_population()>0,"Medical dispatch lacked genuinely immigrated workers");
    check(f.world.original_production_state()->services.trips.size()==2,
          "Both fully staffed paid providers did not actually dispatch their first trips");
    check(f.world.original_production_state()->services.witnesses.empty(),"Mid-outbound disposition granted a visit");
    const auto midtrip=f.world.snapshot();f.replay(f.world);
    f::buy(f.world,sim::set_building_operation(f.herbalist,false));
    f::buy(f.world,sim::set_building_operation(f.acupuncture,false));
    f.roundtrip(f.world);
    until(f.world,[&] {
        const auto& services=f.world.original_production_state()->services;
        return services.witnesses.size()==2 &&
            std::all_of(f.world.couriers().begin(),f.world.couriers().end(),
                [](const auto& c){return c.phase==sim::CourierPhase::IdleAtWorkshop;});
    });
    const auto& visits=f.world.original_production_state()->services.witnesses;
    check(visits[0].building_id==visits[1].building_id && visits[0].role!=visits[1].role,
          "Paused outgoing medical sources did not complete their two fixed first targets");
    const auto served=visits[0].building_id;
    check(house(f.world,served).herbalist>0 && house(f.world,served).acupuncture>0,
          "Real207/208 arrivals did not set their separate original timers");
    const orig::HouseEvolutionRules needs(f.model->house_rules());
    check(orig::needs_met(needs,orig::ModelHouse::Shelter,house(f.world,served)),
          "Actual medical timers did not satisfy their matching authored House requirements");
    const auto positive=f.world.snapshot();f.roundtrip(f.world);f.replay(f.world);
    const auto witness=*std::find_if(visits.begin(),visits.end(),[](const auto& value){return value.role==orig::ServiceRole::Herbalist;});
    const auto next_decay=36+(decay_count(f.world.ticks()))*51;
    while (f.world.ticks()+1<next_decay) f.world.tick();
    const auto before_decay=house(f.world,served).herbalist;f.roundtrip(f.world);f.world.tick();
    check(f.world.ticks()==next_decay && house(f.world,served).herbalist+1==before_decay,
          "Medical phase35 countdown did not execute once after save/restore");
    f.roundtrip(f.world);
    const auto expiry=36+(decay_count(witness.arrival_tick-1)+95)*51;
    check(expiry>f.world.ticks(),"Actual service test missed its future expiry boundary");
    while (f.world.ticks()+1<expiry) f.world.tick();
    check(house(f.world,served).herbalist==1,"Medical timer expired before the witnessed96th countdown");
    f.roundtrip(f.world);f.world.tick();
    check(f.world.ticks()==expiry && !house(f.world,served).herbalist,
          "Medical timer survived its witnessed96th countdown");
    check(!orig::needs_met(needs,orig::ModelHouse::Shelter,house(f.world,served)),
          "Expired Herbalist was hidden by Acupuncture or an unrelated service");
    const auto expired=f.world.snapshot();f.roundtrip(f.world);
    f::buy(f.world,sim::set_building_operation(f.herbalist,true));
    until(f.world,[&] {
        for (const auto& h:f.world.original_economy_state()->houses) if (h.house.herbalist==96) return true;
        return false;
    });
    f.roundtrip(f.world);
    check(f.world.original_production_state()->services.trips.front().transfer.sequence>1,
          "Actual repeat visit did not retain a strictly increasing trip highwater");
    return {midtrip,positive,expired};
}
void refuse_impossible_movement_clock(Fixture& f,const sim::World& actual,sim::CourierId actor,bool outbound) {
    const auto pristine=f.roundtrip(actual);
    auto bad=f.document(actual);
    auto& services=bad.world.original_production->services;
    const auto trip=std::find_if(services.trips.begin(),services.trips.end(),
        [&](const auto& value){return value.transfer.courier_id==actor;});
    check(trip!=services.trips.end(),"Actual movement-clock probe lost its dispatched trip");
    auto altered=json::parse(pristine);
    const auto actor_id=static_cast<std::uint32_t>(actor);
    auto& json_services=altered["world"]["original_production"]["services"];
    if (outbound) {
        // Keep the real pose just before arrival; change only the dispatch tick.
        trip->dispatched_tick=actual.ticks();
        for (auto& value:json_services["trips"])
            if (value["transfer"]["courier_id"]==actor_id) value["dispatched_tick"]=actual.ticks();
    } else {
        check(trip->arrival_tick && trip->visit_applied,"Actual return-clock probe lacks its successful arrival");
        trip->arrival_tick=actual.ticks();
        const auto witness=std::find_if(services.witnesses.begin(),services.witnesses.end(),
            [&](const auto& value){return value.transfer==trip->transfer;});
        check(witness!=services.witnesses.end() && witness->role==orig::ServiceRole::Herbalist,
              "Actual return-clock probe lost its matching Herbalist witness");
        witness->arrival_tick=actual.ticks();
        auto& homes=bad.world.original_economy->houses;
        const auto home=std::find_if(homes.begin(),homes.end(),
            [&](const auto& value){return value.building_id==witness->building_id;});
        check(home!=homes.end(),"Actual return-clock probe lost its visited House");
        home->house.herbalist=expected_timer(actual.ticks(),actual.ticks());
        for (auto& value:json_services["trips"])
            if (value["transfer"]["courier_id"]==actor_id) value["arrival_tick"]=actual.ticks();
        for (auto& value:json_services["witnesses"])
            if (value["transfer"]["courier_id"]==actor_id && value["transfer"]["sequence"]==trip->transfer.sequence)
                value["arrival_tick"]=actual.ticks();
        for (auto& value:altered["world"]["original_economy"]["houses"])
            if (value["building_id"]==static_cast<std::uint32_t>(home->building_id))
                value["house"]["herbalist"]=home->house.herbalist;
    }
    refuses([&]{(void)sim::World::restore_original_production_development(bad.world,f.policy,f.model);});
    refuses([&]{(void)f.restore(bad);});
    refuses([&]{save::write_save(f.target,bad,f.root.data,f.policy->building_mask());});
    check(f::read_text(f.target)==pristine,"Impossible movement clock replaced the valid protected file");
    const auto path=f.root.path/"bad-medical-movement-clock.json";
    f::write_text(path,altered.dump());
    refuses([&]{(void)f.restore(save::read_save(path));});
    check(f::read_text(f.target)==pristine,"Malformed serialized movement clock replaced the valid file");
}
void legitimate_next_tick(Fixture& f,sim::World& actual) {
    f.roundtrip(actual);
    auto restored=f.restore(save::read_save(f.target));
    actual.tick();restored.tick();
    check(actual.snapshot()==restored.snapshot(),"Legitimate medical movement changed on the first tick after restore");
}
void movement_clock_refusals(Fixture& f,const sim::WorldSnapshot& genuine_midtrip) {
    auto actual=sim::World::restore_original_production_development(genuine_midtrip,f.policy,f.model);
    const auto actor=std::find_if(actual.couriers().begin(),actual.couriers().end(),
        [&](const auto& value){return value.owner==f.herbalist && value.role==sim::CourierRole::HealthWorker;});
    check(actor!=actual.couriers().end(),"Paid Herbalist movement probe lacks its own actual courier");
    const auto actor_id=actor->id;
    check(actual.active_rules().courier_edge_ticks==5,"Medical movement boundary fixture requires actual five-tick edges");
    until(actual,[&] {
        const auto& c=actual.courier(actor_id);
        return c.phase==sim::CourierPhase::ToWarehouse && c.path_vertex+2==c.path.size() && c.edge_progress==4;
    });
    refuse_impossible_movement_clock(f,actual,actor_id,true);
    legitimate_next_tick(f,actual);
    check(actual.courier(actor_id).phase==sim::CourierPhase::Returning,
          "Legitimate final outbound edge did not complete its actual arrival after restore");
    until(actual,[&] {
        const auto& c=actual.courier(actor_id);
        return c.phase==sim::CourierPhase::Returning && c.path_vertex>0 && c.edge_progress>0;
    });
    refuse_impossible_movement_clock(f,actual,actor_id,false);
    legitimate_next_tick(f,actual);
    until(actual,[&]{return actual.courier(actor_id).phase==sim::CourierPhase::IdleAtWorkshop;});
    refuse_impossible_movement_clock(f,actual,actor_id,false);
    legitimate_next_tick(f,actual);
}
void hostile(Fixture& f,const States& states) {
    auto world=sim::World::restore_original_production_development(states.positive,f.policy,f.model);
    const auto pristine=f.roundtrip(world);const auto good=json::parse(pristine);const auto path=f.root.path/"bad-medical.json";
    const auto reject=[&](const json& value) {
        f::write_text(path,value.dump());
        try {refuses([&]{(void)f.restore(save::read_save(path));});}
        catch (const std::exception& e) {throw std::runtime_error(std::string(e.what())+": "+json::diff(good,value).dump());}
        check(f::read_text(f.target)==pristine,"Bad medical input replaced the protected save");
    };
    const std::string base="/world/original_production/services";
    std::vector<std::string> objects;
    const std::function<void(const json&,std::string)> visit=[&](const json& value,std::string prefix) {
        if (value.is_object()) {
            objects.push_back(prefix);
            for (const auto& [key,child]:value.items()) visit(child,prefix+"/"+key);
        } else if (value.is_array())
            for (std::size_t i=0;i<value.size();++i) visit(value[i],prefix+"/"+std::to_string(i));
    };
    visit(good.at(json::json_pointer(base)),base);
    for (const auto& object:objects) {
        auto value=good;value.at(json::json_pointer(object))["unsupported_medical_authority"]=true;reject(value);
        for (const auto& [key,child]:good.at(json::json_pointer(object)).items()) {
            (void)child;value=good;value.at(json::json_pointer(object)).erase(key);reject(value);
        }
    }
    for (const auto& [suffix,replacement]:std::vector<std::pair<std::string,json>>{
            {"",nullptr},{"/providers",json::array()},{"/trips",json::array()},{"/witnesses",json::array()},
            {"/providers/0/type",1},{"/providers/0/type",2},
            {"/trips/0/target",good.at(json::json_pointer(base))["providers"][0]["building_id"]},
            {"/trips/0/dispatched_tick",0},{"/trips/0/dispatched_tick",UINT64_MAX},
            {"/trips/0/arrival_tick",nullptr},{"/trips/0/arrival_tick",0},{"/trips/0/arrival_tick",UINT64_MAX},
            {"/trips/0/visit_applied",false},{"/trips/0/visit_applied",1},
            {"/witnesses/0/role",1},{"/witnesses/0/role",2},
            {"/witnesses/0/provider_id",good.at(json::json_pointer(base))["providers"][1]["building_id"]},
            {"/witnesses/0/arrival_tick",0},{"/witnesses/0/arrival_tick",UINT64_MAX}}) {
        auto value=good;value.at(json::json_pointer(base+suffix))=replacement;reject(value);
    }
    for (const auto& suffix:{"/providers/0/building_id","/trips/0/target","/trips/0/transfer/courier_id",
            "/witnesses/0/building_id","/witnesses/0/provider_id","/witnesses/0/transfer/courier_id"})
        for (const json& invalid:{json(0),json(-1),json(true),json(0.0),json("1"),json(nullptr),json(UINT32_MAX),json(UINT64_MAX)}) {
            auto value=good;value.at(json::json_pointer(base+suffix))=invalid;reject(value);
        }
    for (const auto& suffix:{"/trips/0/transfer/sequence","/trips/0/dispatched_tick","/trips/0/arrival_tick",
            "/witnesses/0/transfer/sequence","/witnesses/0/arrival_tick"})
        for (const json& invalid:{json(-1),json(true),json(0.0),json("1"),json(UINT64_MAX)}) {
            auto value=good;value.at(json::json_pointer(base+suffix))=invalid;reject(value);
        }
    for (const char* key:{"providers","trips","witnesses"}) {
        auto value=good;auto& duplicate=value.at(json::json_pointer(base))[key];duplicate.push_back(duplicate[0]);reject(value);
        value=good;auto& reverse=value.at(json::json_pointer(base))[key];std::swap(reverse[0],reverse[1]);reject(value);
        value=good;auto& oversized=value.at(json::json_pointer(base))[key];
        const std::size_t maximum=std::string(key)=="witnesses" ? 40:4;
        while (oversized.size()<=maximum) oversized.push_back(oversized[0]);
        reject(value);
        for (const json& invalid:{json(nullptr),json(false),json(0),json::object()}) {
            value=good;value.at(json::json_pointer(base))[key]=invalid;reject(value);
        }
    }
    const auto served=good.at(json::json_pointer(base))["witnesses"][0]["building_id"];
    std::size_t house_index=0;
    while (good["world"]["original_economy"]["houses"][house_index]["building_id"]!=served) ++house_index;
    const std::string house_base="/world/original_economy/houses/"+std::to_string(house_index)+"/house/";
    for (const auto& [suffix,replacement]:std::vector<std::pair<std::string,json>>{
            {"herbalist",0},{"herbalist",96},{"herbalist",255},{"acupuncture",0},{"music",96},
            {"ancestor",96},{"buddhist",96}}) {
        auto value=good;value.at(json::json_pointer(house_base+suffix))=replacement;reject(value);
    }
    for (const auto revision:{18,20}) {
        auto value=good;value["world"]["original_production"]["development_revision"]=revision;reject(value);
    }
    for (int fault=0;fault<6;++fault) {
        auto value=good;auto& trip=value.at(json::json_pointer(base))["trips"][0];
        auto& witness=value.at(json::json_pointer(base))["witnesses"][0];
        const auto now=value["world"]["ticks"].get<std::uint64_t>();
        if (fault==0) trip["dispatched_tick"]=trip["arrival_tick"];
        if (fault==1) trip["arrival_tick"]=now+1;
        if (fault==2) witness["arrival_tick"]=now+1;
        if (fault==3) witness["transfer"]["sequence"]=trip["transfer"]["sequence"].get<std::uint64_t>()+1;
        if (fault==4) {
            const auto early=trip["dispatched_tick"].get<std::uint64_t>();
            trip["arrival_tick"]=early;witness["arrival_tick"]=early;
            value.at(json::json_pointer(house_base+"herbalist"))=expected_timer(now,early);
        }
        if (fault==5) {
            // Preserve references, trip/witness agreement and the exact timer,
            // but physically teleport a real multi-edge visit to dispatch+1.
            const auto early=trip["dispatched_tick"].get<std::uint64_t>()+1;
            trip["arrival_tick"]=early;witness["arrival_tick"]=early;
            value.at(json::json_pointer(house_base+"herbalist"))=expected_timer(now,early);
        }
        reject(value);
    }
    auto unvisited=sim::World::restore_original_production_development(states.midtrip,f.policy,f.model);
    auto absent=json::parse(f.roundtrip(unvisited));
    absent["world"]["original_economy"]["houses"][0]["house"]["herbalist"]=1;
    f::write_text(path,absent.dump());refuses([&]{(void)f.restore(save::read_save(path));});
    auto expired=sim::World::restore_original_production_development(states.expired,f.policy,f.model);
    auto stale=json::parse(f.roundtrip(expired));
    stale["world"]["original_economy"]["houses"][house_index]["house"]["herbalist"]=1;
    f::write_text(path,stale.dump());refuses([&]{(void)f.restore(save::read_save(path));});
    f.roundtrip(world);
    // A genuine second trip leaves the first House's successful visit compacted.
    // Its expired timer still cannot justify a geometrically impossible first
    // arrival at Houseage122 (two ordinary courier edges require a later tick).
    save::write_save(path,f.document(f.world),f.root.data,f.policy->building_mask());
    auto compacted=json::parse(f::read_text(path));bool compacted_probe=false;
    auto& history=compacted["world"]["original_production"]["services"];
    for (auto& old:history["witnesses"]) {
        if (old["role"]!=0) continue;
        const auto latest=std::find_if(history["trips"].begin(),history["trips"].end(),[&](const auto& trip) {
            return trip["transfer"]["courier_id"]==old["transfer"]["courier_id"];
        });
        if (latest==history["trips"].end() || old["transfer"]["sequence"]>=(*latest)["transfer"]["sequence"]) continue;
        const auto body=std::find_if(compacted["world"]["buildings"].begin(),compacted["world"]["buildings"].end(),[&](const auto& b) {
            return b["id"]==old["building_id"];
        });
        check(body!=compacted["world"]["buildings"].end() && (*body)["placed_tick"]==0,
              "Compacted medical probe lost its genuinely purchased initial House");
        old["arrival_tick"]=122;
        for (auto& h:compacted["world"]["original_economy"]["houses"])
            if (h["building_id"]==old["building_id"])
                h["house"]["herbalist"]=expected_timer(compacted["world"]["ticks"].get<std::uint64_t>(),122);
        compacted_probe=true;break;
    }
    check(compacted_probe,"Real repeat visit did not provide compacted medical history");
    f::write_text(path,compacted.dump());refuses([&]{(void)f.restore(save::read_save(path));});
    check(f::read_text(f.target)==pristine,"Compacted medical forgery replaced the protected save");
    const auto doc=f.document(world);
    for (int fault=0;fault<8;++fault) {
        auto bad=doc;auto& services=bad.world.original_production->services;
        if (fault==0) services.providers.clear();
        if (fault==1) services.trips.clear();
        if (fault==2) services.witnesses.clear();
        if (fault==3) services.trips[0].transfer.sequence=UINT64_MAX;
        if (fault==4) services.witnesses[0].arrival_tick=bad.world.ticks+1;
        if (fault==5) bad.world.original_production->development_revision=18;
        if (fault==6) bad.world.couriers[0].role=sim::CourierRole::Service;
        if (fault==7) bad.world.couriers[0].phase=sim::CourierPhase::ToWarehouse;
        refuses([&]{save::write_save(f.target,bad,f.root.data,f.policy->building_mask());});
        check(f::read_text(f.target)==pristine,"Bad medical writer replaced the valid protected file");
    }
    auto duplicate=pristine;const auto marker=duplicate.find("\"services\": {");
    check(marker!=std::string::npos,"Medical duplicate-key test object absent");
    duplicate.insert(duplicate.find('{',marker)+1,"\"trips\":[],");f::write_text(path,duplicate);
    refuses([&]{(void)save::read_save(path);});
    auto bad=doc;++bad.world.construction_spent_total;--bad.world.treasury;
    sim::World::validate_original_production_snapshot(bad.world);
    refuses([&]{(void)f.restore(bad);});
    refuses([&]{save::write_save(f.target,bad,f.root.data,f.policy->building_mask());});
    check(f::read_text(f.target)==pristine,"Modelbound medical finance refusal replaced valid data");
}
}
int main() {
    try {
        Fixture fixture;const auto states=lifecycle(fixture);
        movement_clock_refusals(fixture,states.midtrip);hostile(fixture,states);
        std::cout<<"Medical save PASS: paid207/208 actual arrivals, separate96 timers/decay/expiry, midtrip20k replay and "
                 <<rejected<<" hostile refusals\n";return 0;
    } catch (const std::exception& error) {
        std::cerr<<"Original medical save FAIL: "<<error.what()<<'\n';return 1;
    }
}
