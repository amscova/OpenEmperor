#include "OriginalFinanceFixture.h"
#include "persistence/RecoveryStore.h"
#include <nlohmann/json.hpp>
#include <iostream>

namespace {
namespace q=oe_finance_fixture;
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace save=openemperor::persistence;
using f::check;
using Json=nlohmann::json;
template<class F> void refuses(F action) {
    try {action();} catch (const std::exception&) {return;}
    throw std::runtime_error("Contradictory Rule11 fire/finance save was accepted");
}
void rejects_unvisited_return_target() {
    q::Fixture fixture;auto world=sim::World::create_original_fire_protection(fixture.policy,fixture.model);
    q::build(world);f::buy(world,{sim::CommandType::PlaceFireWatch,{20,5}});
    while(world.original_production_state()->fire_protection->arrivals.empty())world.tick();
    const auto pristine=world.snapshot();
    bool exercised=false;
    for(const auto& actor:world.couriers())if(actor.role==sim::CourierRole::FireInspector) {
        check(actor.phase==sim::CourierPhase::Returning,"First arrival did not begin a real physical return");
        check(actor.path.size()>=3 && actor.path_vertex==0 && actor.edge_progress==0,
              "First arrival lacks a genuine untouched reverse path");
        bool accepted_teleport=false,accepted_early_idle=false;
        {
            auto forged=pristine;auto& c=forged.couriers[static_cast<std::uint32_t>(actor.id)-1];
            c.path_vertex=c.path.size()-2;c.edge_progress=4;
            try {(void)sim::World::restore_original_fire_protection(forged,fixture.policy,fixture.model);accepted_teleport=true;}
            catch(const std::exception&) {}
        }
        {
            auto forged=pristine;auto& c=forged.couriers[static_cast<std::uint32_t>(actor.id)-1];
            c.phase=sim::CourierPhase::IdleAtWorkshop;c.path.clear();c.path_vertex=0;c.edge_progress=0;
            c.route_pending=false;c.route_checked_revision.reset();
            try {(void)sim::World::restore_original_fire_protection(forged,fixture.policy,fixture.model);accepted_early_idle=true;}
            catch(const std::exception&) {}
        }
        check(!accepted_teleport && !accepted_early_idle,
              std::string("Rule11 accepted impossible same-tick Inspector return: progressed=")+
              (accepted_teleport ? "yes":"no")+", idle="+(accepted_early_idle ? "yes":"no"));
        {
            auto forged=pristine;auto& c=forged.couriers[static_cast<std::uint32_t>(actor.id)-1];
            c.path.front()={std::numeric_limits<int>::min(),std::numeric_limits<int>::max()};
            refuses([&]{(void)sim::World::restore_original_fire_protection(forged,fixture.policy,fixture.model);});
        }
        for(const auto& route:actor.dynamic_target_routes)if(route.first!=actor.target && route.second) {
            auto forged=pristine;auto& c=forged.couriers[static_cast<std::uint32_t>(actor.id)-1];
            // The alternate path is a real, prepared legal route, and all fire
            // clocks remain genuine. Only the invented return origin differs
            // from the paid Inspector's recorded actual arrival.
            c.target=route.first;c.last_dispatched_target=route.first;c.path=*route.second;
            std::reverse(c.path.begin(),c.path.end());c.path_vertex=0;c.edge_progress=0;
            c.route_pending=false;c.route_checked_revision.reset();
            refuses([&]{(void)sim::World::restore_original_fire_protection(forged,fixture.policy,fixture.model);});
            check(world.snapshot()==pristine,"Rejected invented return changed the genuine city");
            exercised=true;break;
        }
    }
    check(exercised,"Returning-target hostile probe lacks a genuinely prepared alternate route");
    auto valid=sim::World::restore_original_fire_protection(pristine,fixture.policy,fixture.model);
    while(valid.couriers().back().phase!=sim::CourierPhase::IdleAtWorkshop)valid.tick();
    auto idle=valid.snapshot();const auto actual=idle.couriers.back().target;
    idle.couriers.back().last_dispatched_target=idle.couriers.back().owner;
    refuses([&]{(void)sim::World::restore_original_fire_protection(idle,fixture.policy,fixture.model);});
    check(actual!=idle.couriers.back().owner,"Idle target probe never completed a real visit");
}

void legitimate_repaired_return_progress() {
    q::Fixture fixture;auto world=sim::World::create_original_fire_protection(fixture.policy,fixture.model);
    for(int x=0;x<=6;++x)f::buy(world,{sim::CommandType::PlaceRoad,{x,8}});
    f::buy(world,{sim::CommandType::PlaceHousehold,{0,6}});
    f::buy(world,{sim::CommandType::PlaceFireWatch,{4,9}});
    while(world.original_production_state()->fire_protection->arrivals.empty() && world.ticks()<1000)world.tick();
    check(!world.original_production_state()->fire_protection->arrivals.empty(),"Return repair fixture lacks an earned arrival");
    const auto id=world.couriers().back().id;const auto watch=world.couriers().back().owner;
    f::buy(world,sim::set_building_operation(watch,false));world.tick();
    const auto& actor=world.courier(id);
    check(actor.phase==sim::CourierPhase::Returning && actor.edge_progress==1 && actor.path.size()>3,
          "Return repair fixture lacks a genuinely begun return edge");
    const auto cut=actor.path[2];f::buy(world,{sim::CommandType::RemoveRoad,cut});
    const auto restore=[&] {
        const auto snapshot=world.snapshot();
        auto restored=sim::World::restore_original_fire_protection(snapshot,fixture.policy,fixture.model);
        check(restored.snapshot()==snapshot,"Legitimate pending/repaired return progress failed exact restore");
        return restored;
    };
    check(world.courier(id).route_pending && world.courier(id).edge_progress==1,"Return cut lost begun-edge safety");
    (void)restore();
    for(int i=0;i<6;++i)world.tick();
    check(world.courier(id).route_pending && !world.courier(id).edge_progress,"Disconnected return did not wait at its actual cell");
    (void)restore();
    f::buy(world,{sim::CommandType::PlaceRoad,cut});world.tick();
    check(!world.courier(id).route_pending && world.courier(id).edge_progress==1 &&
          world.courier(id).path.front()!=world.original_production_state()->fire_protection->arrivals.back().trip.travelled.back(),
          "Repaired return did not retain an ordinary resumed transport-cell prefix");
    auto copy=restore();
    for(int i=0;i<50;++i){world.tick();copy.tick();check(world.snapshot()==copy.snapshot(),"Repaired return continuation diverged");}
    check(world.courier(id).phase==sim::CourierPhase::IdleAtWorkshop,"Paid paused Inspector failed to finish its legitimate return");
    (void)restore();
}

void paid_fire_and_recovery() {
    q::Fixture fixture;
    auto world=sim::World::create_original_fire_protection(fixture.policy,fixture.model);
    const auto scene=q::build(world);
    f::buy(world,{sim::CommandType::PlaceFireWatch,{20,5}});
    const auto watch=*world.building_owner_at({20,5});
    const auto restore=[&](const save::SaveDocument& d) {
        return save::restore_original_fire_protection(d,fixture.root.data,fixture.policy->building_mask(),fixture.policy,fixture.model);
    };
    const auto target=fixture.root.path/"rule11-good.json";
    const auto roundtrip=[&] {
        const auto before=world.snapshot();const auto document=fixture.document(world);
        save::write_save(target,document,fixture.root.data,fixture.policy->building_mask());
        const auto bytes=f::read_text(target);const auto parsed=save::read_save(target);
        check(parsed.source_schema_version==27 && parsed.world.rule_version==11 &&
              parsed.world.original_production->development_revision==27,"Rule11 save tuple changed");
        auto copy=restore(parsed);check(copy.snapshot()==before,"Rule11 F5/F9 changed complete authority");
        save::write_save(target,fixture.document(copy),fixture.root.data,fixture.policy->building_mask());
        check(bytes==f::read_text(target),"Rule11 canonical full-document bytes changed");
        return copy;
    };
    (void)roundtrip();
    for (unsigned i=0;i<10000 && world.original_production_state()->fire_protection->trips.empty();++i) world.tick();
    check(!world.original_production_state()->fire_protection->trips.empty(),"Paid staffed Watch never dispatched a real Inspector");
    auto inflight=roundtrip();
    for (unsigned i=0;i<5000;++i) {world.tick();inflight.tick();}
    check(world.snapshot()==inflight.snapshot(),"Restored real Inspector trip diverged after5k ticks");
    check(!world.original_production_state()->fire_protection->arrivals.empty(),"No actual paid Inspector arrived");
    check(world.building(watch).kind==sim::Object::FireWatch && world.maintenance_spent_total()>0,
          "Saved Watch lost paid identity or real upkeep");
    (void)roundtrip();
    const auto pristine=f::read_text(target);const auto before=world.snapshot();
    const auto reject=[&](auto mutate) {
        auto bad=fixture.document(world);mutate(bad);
        refuses([&]{(void)restore(bad);});
        refuses([&]{save::write_save(target,bad,fixture.root.data,fixture.policy->building_mask());});
        check(world.snapshot()==before && f::read_text(target)==pristine,"Rejected fire save changed source World or good save bytes");
    };
    reject([](auto& d){d.world.original_production->fire_protection.reset();});
    reject([](auto& d){++d.world.original_production->fire_protection->travelled_cells;});
    reject([](auto& d){d.world.original_production->fire_protection->arrivals.front().trip.owner=static_cast<sim::BuildingId>(999);});
    reject([](auto& d){d.world.original_production->fire_protection->arrivals.front().trip.assigned_workers=0;});
    reject([](auto& d){d.world.original_production->fire_protection->arrivals.front().trip.operating=false;});
    reject([](auto& d){auto& e=d.world.original_production->fire_protection->arrivals.front();e.tick=e.trip.dispatched_tick;});
    reject([](auto& d){auto& e=d.world.original_production->fire_protection->arrivals.front();e.extinguished=!e.extinguished;});
    reject([](auto& d){d.world.original_production->fire_protection->arrivals.front().trip.travelled.back()={-1,-1};});
    reject([](auto& d){d.world.original_production->fire_protection->arrivals.clear();});
    reject([&](auto& d){for(auto& b:d.world.buildings)if(b.id==watch)++b.fire_risk;});
    reject([](auto& d){d.world.original_production->fire_protection->taxes.clear();});
    reject([](auto& d){--d.world.treasury;});
    reject([](auto& d){d.world.construction_spent_total-=80;d.world.treasury+=80;});
    reject([](auto& d){--d.world.maintenance_spent_total;++d.world.treasury;});
    reject([](auto& d){d.source_schema_version=26;});
    refuses([&]{(void)save::restore_original_production_development(fixture.document(world),fixture.root.data,fixture.policy->building_mask(),fixture.policy,fixture.model);});
    auto legacy=fixture.world();q::build(legacy);q::through(legacy,816);
    save::write_save(target,fixture.document(legacy),fixture.root.data,fixture.policy->building_mask());
    const auto legacy_bytes=f::read_text(target);auto legacy_copy=fixture.restore(save::read_save(target));
    save::write_save(target,fixture.document(legacy_copy),fixture.root.data,fixture.policy->building_mask());
    check(legacy_bytes==f::read_text(target),"Historical Rule10/schema26 bytes changed on canonical rewrite");
    refuses([&]{(void)restore(fixture.document(legacy));});
    save::RecoveryStore store(fixture.root.path/"recovery-app",fixture.root.data,
        save::recovery_storage_budget_bytes,save::RecoveryAccess::OriginalProduction);
    const auto history=store.create_history(fixture.document(world),fixture.policy->building_mask());
    for(unsigned i=0;i<31;++i)world.tick();
    const auto entry=store.write_checkpoint(history,fixture.document(world),fixture.policy->building_mask());
    check(entry.schema==27 && entry.rule_version==11 && entry.loadable(),"Rule11 autosave/recovery tuple invalid");
    auto recovered=restore(save::read_save(entry.path));
    for(unsigned i=0;i<1000;++i){world.tick();recovered.tick();}
    check(recovered.snapshot()==world.snapshot(),"Rule11 full recovery continuation diverged");
    const auto catalog=store.catalog();
    check(catalog.histories.size()==1 && catalog.histories.front().error.empty(),"Rule11 recovery history failed canonical validation");
    const auto raw_reject=[&](auto mutate) {
        auto hostile=Json::parse(pristine);mutate(hostile);
        const auto path=fixture.root.path/"hostile-rule11.json";f::write_text(path,hostile.dump());
        refuses([&]{(void)restore(save::read_save(path));});
        check(world.navigation_valid() && f::read_text(target)==legacy_bytes,"Hostile raw save changed good authority or bytes");
    };
    raw_reject([](auto& j){j["world"]["original_production"]["fire_protection"]["unknown"]=1;});
    raw_reject([](auto& j){j["world"]["original_production"].erase("fire_protection");});
    raw_reject([](auto& j){j["world"]["original_production"]["fire_protection"]["arrivals"][0][1]=0;});
    raw_reject([](auto& j){j["world"]["original_production"]["fire_protection"]["arrivals"][0][2][0]=999;});
    raw_reject([](auto& j){j["world"]["original_production"]["fire_protection"]["arrivals"][0][2][2]=999;});
    raw_reject([](auto& j){j["world"]["original_production"]["fire_protection"]["arrivals"][0][2][8][0]=51984;});
    raw_reject([](auto& j){j["world"]["original_production"]["fire_protection"]["arrivals"][0][2][8][1]=575;});
    raw_reject([](auto& j){j["world"]["original_production"]["fire_protection"]["taxes"][0][3]=32767;});
    raw_reject([](auto& j){j["world"]["original_production"]["fire_protection"]["arrivals"]=Json::array();
        for(std::size_t i=0;i<=sim::original_fire_arrival_limit;++i)j["world"]["original_production"]["fire_protection"]["arrivals"].push_back(nullptr);});
    raw_reject([](auto& j){j["world"]["original_production"]["fire_protection"]["trips"]=Json::array({nullptr,nullptr,nullptr});});
    raw_reject([](auto& j){auto& taxes=j["world"]["original_production"]["fire_protection"]["taxes"];
        taxes=Json::array();for(std::size_t i=0;i<=sim::original_fire_tax_limit;++i)taxes.push_back(nullptr);});
    raw_reject([](auto& j){auto& cells=j["world"]["original_production"]["fire_protection"]["arrivals"][0][2][8];
        cells=Json::array();for(std::size_t i=0;i<=sim::original_fire_path_cell_limit;++i)cells.push_back(1);});
    raw_reject([](auto& j){j["world"]["buildings"][0]["fire_protection_until_tick"]=999999;});
    // Source fingerprints are validated against supplied immutable files, not
    // only against duplicated metadata inside the hostile JSON.
    raw_reject([](auto& j){j["map"]["sha256"]=std::string(64,'0');});
    raw_reject([](auto& j){j["world"]["original_economy"]["model_source"]["sha256"]=std::string(64,'0');});
    raw_reject([](auto& j){j["world"]["original_economy"]["model_source"]["difficulty"]=4;});
    auto duplicated=pristine;
    const auto key=duplicated.find("\"travelled_cells\":");
    check(key!=std::string::npos,"Canonical schema27 omitted path history counter");
    duplicated.insert(key,"\"travelled_cells\": 0,");
    const auto duplicatepath=fixture.root.path/"duplicate-rule11.json";f::write_text(duplicatepath,duplicated);
    refuses([&]{(void)save::read_save(duplicatepath);});
    auto raw=Json::parse(pristine);raw["schema_version"]=26;
    const auto badpath=fixture.root.path/"cross-schema.json";f::write_text(badpath,raw.dump());
    refuses([&]{(void)save::read_save(badpath);});
    auto burning=sim::World::create_original_fire_protection(fixture.policy,fixture.model);
    const auto burning_scene=q::build(burning);q::through(burning,2448);
    auto burned=fixture.document(burning);const auto id=burning_scene.commons.front();
    check(burning.building_on_fire(id),"Natural-fire tax probe lacks an actual burning paid House");
    auto& taxhistory=burned.world.original_production->fire_protection->taxes;
    auto tax=std::find_if(taxhistory.rbegin(),taxhistory.rend(),[&](const auto& t){return t.building_id==id;});
    check(tax!=taxhistory.rend(),"Natural-fire tax probe lacks genuine preceding assessed input");
    tax->assessment.tick=2448;
    std::sort(taxhistory.begin(),taxhistory.end(),[](const auto& a,const auto& b){return a.assessment.tick<b.assessment.tick ||
        (a.assessment.tick==b.assessment.tick && a.building_id<b.building_id);});
    for(auto& h:burned.world.original_production->finance.houses)if(h.building_id==id)h.last_assessment->tick=2448;
    refuses([&]{(void)restore(burned);});

}
void sustainable_paid_save_and_recovery() {
    // Independently authored geography/model, using only ordinary paid actions.
    // This verifies the new persistence boundary, not original-data gameplay.
    q::Fixture fixture;auto world=sim::World::create_original_sustainable_economy(fixture.policy,fixture.model);
    for(int x=0;x<24;++x)f::buy(world,{sim::CommandType::PlaceRoad,{x,4}});
    for(int x=0;x<12;x+=3)f::buy(world,{sim::CommandType::PlaceHousehold,{x,2}});
    f::buy(world,{sim::CommandType::PlaceMarket,{5,5}});
    f::buy(world,{sim::CommandType::PlaceOriginalWaterWell,{9,5}});
    f::buy(world,{sim::CommandType::PlaceOriginalAncestorShrine,{13,5}});
    for(const auto& [farm_x,field_x]:std::array{std::pair{0,3},std::pair{15,18},std::pair{19,22}}) {
        f::buy(world,{sim::CommandType::PlaceOriginalFoodFarm,{farm_x,5}});
        sim::Command field{sim::CommandType::PlaceOriginalMilletField,{field_x,5}};
        field.building_id=*world.building_owner_at({farm_x,5});f::buy(world,field);
    }
    f::buy(world,{sim::CommandType::PlaceFireWatch,{23,5}});
    check(world.original_production_state()->food.farms.size()==3 && world.treasury()>=0 && !world.total_population(),
          "Rule12 persistence fixture did not pay for three distinct Food chains before admission");
    const auto unmutated=world.snapshot();
    const auto fourth=world.execute({sim::CommandType::PlaceOriginalFoodFarm,{10,5}});
    check(!fourth.accepted && std::string_view(fourth.reason)=="Food Farmhouse limit reached" && world.snapshot()==unmutated,
          "Fourth Food farm was not refused atomically");
    for(int i=0;i<1700;++i)world.tick();
    check(world.taxes_collected_total()>0 && !world.original_production_state()->fire_protection->arrivals.empty(),
          "Rule12 persistence fixture did not earn real taxes and Inspector arrivals");
    const auto target=fixture.root.path/"rule12-good.json";
    const auto document=fixture.document(world);
    save::write_save(target,document,fixture.root.data,fixture.policy->building_mask());
    const auto bytes=f::read_text(target);const auto parsed=save::read_save(target);
    check(parsed.source_schema_version==28 && parsed.world.rule_version==12 && parsed.world==world.snapshot(),
          "Rule12 schema28 did not preserve complete paid authority");
    auto copy=save::restore_original_sustainable_economy(parsed,fixture.root.data,fixture.policy->building_mask(),fixture.policy,fixture.model);
    save::write_save(target,fixture.document(copy),fixture.root.data,fixture.policy->building_mask());
    check(f::read_text(target)==bytes,"Rule12 complete canonical SaveDocument changed after restore");
    refuses([&]{(void)save::restore_original_fire_protection(parsed,fixture.root.data,fixture.policy->building_mask(),fixture.policy,fixture.model);});
    refuses([&]{(void)save::restore_original_production_development(parsed,fixture.root.data,fixture.policy->building_mask(),fixture.policy,fixture.model);});
    for(const auto schema:{26U,27U}) {
        auto forged=parsed;forged.source_schema_version=schema;
        refuses([&]{(void)save::restore_original_sustainable_economy(forged,fixture.root.data,fixture.policy->building_mask(),fixture.policy,fixture.model);});
    }
    const auto raw_reject=[&](auto edit) {
        auto j=Json::parse(bytes);edit(j);const auto path=fixture.root.path/"hostile-rule12.json";f::write_text(path,j.dump());
        refuses([&]{(void)save::read_save(path);});
        check(f::read_text(target)==bytes && world.snapshot()==parsed.world,"Rejected schema28 input changed live World or prior save");
    };
    raw_reject([](auto& j){j["rules"]["version"]=11;});
    raw_reject([](auto& j){j["schema_version"]=27;});
    raw_reject([](auto& j){j["world"]["original_production"]["food"]["farms"].push_back(j["world"]["original_production"]["food"]["farms"].back());});
    raw_reject([](auto& j){j["world"]["original_production"]["finance"]["unknown"]=1;});
    raw_reject([](auto& j){j["world"]["original_production"].erase("fire_protection");});
    auto duplicate=bytes;const auto at=duplicate.find("\"travelled_cells\":");
    check(at!=std::string::npos,"Rule12 omitted its mandatory fire provenance");
    duplicate.insert(at,"\"travelled_cells\":0,");
    const auto duplicate_path=fixture.root.path/"duplicate-rule12.json";f::write_text(duplicate_path,duplicate);
    refuses([&]{(void)save::read_save(duplicate_path);});
    save::RecoveryStore legacy(fixture.root.path/"legacy-app",fixture.root.data);
    refuses([&]{(void)legacy.create_history(document,fixture.policy->building_mask());});
    save::RecoveryStore recovery(fixture.root.path/"rule12-app",fixture.root.data,save::recovery_storage_budget_bytes,
                                 save::RecoveryAccess::OriginalProduction);
    const auto history=recovery.create_history(document,fixture.policy->building_mask());
    for(int i=0;i<400;++i){world.tick();copy.tick();check(world.snapshot()==copy.snapshot(),"Rule12 complete continuation diverged");}
    const auto checkpoint=recovery.write_checkpoint(history,fixture.document(world),fixture.policy->building_mask());
    check(checkpoint.schema==28 && checkpoint.rule_version==12 && checkpoint.loadable(),"Recovery changed the explicit schema28/rule12 tuple");
    const auto resumed=save::restore_original_sustainable_economy(save::read_save(checkpoint.path),fixture.root.data,
        fixture.policy->building_mask(),fixture.policy,fixture.model);
    check(resumed.snapshot()==world.snapshot(),"Rule12 recovery changed complete current authority");
}
}
int main() {
    try {rejects_unvisited_return_target();legitimate_repaired_return_progress();paid_fire_and_recovery();sustainable_paid_save_and_recovery();std::cout<<"PASS Rule11/12 paid fire persistence, exact taxes, hostile saves and recovery\n";return 0;}
    catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
