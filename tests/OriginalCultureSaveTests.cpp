#include "OriginalProductionFixture.h"
#include "simulation/original/ServiceRules.h"

#include <nlohmann/json.hpp>
#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
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
    throw std::runtime_error("Malformed culture save authority was admitted");
}
template<class P> void until(sim::World& world,P condition,int limit=30000) {
    for (int i=0;i<limit && !condition();++i) world.tick();
    check(condition(),"Paid culture scene did not reach the required actual state");
}
std::uint64_t decays(std::uint64_t tick) {return tick<36 ? 0:1+(tick-36)/51;}
std::uint8_t timer(std::uint64_t tick,std::uint64_t arrival,orig::ServiceRole role) {
    check(arrival && arrival<=tick,"Culture timer requires an actual prior arrival");
    const auto duration=static_cast<unsigned>(role)>=5 ? 40U:96U;
    const auto elapsed=decays(tick)-decays(arrival-1);
    return static_cast<std::uint8_t>(elapsed>=duration ? 0:duration-elapsed);
}
const orig::HouseState& house(const sim::World& world,sim::BuildingId id) {
    const auto& rows=world.original_economy_state()->houses;
    const auto found=std::find_if(rows.begin(),rows.end(),[&](const auto& h){return h.building_id==id;});
    check(found!=rows.end(),"Paid culture House disappeared");return found->house;
}
void witnesses_match(const sim::World& world) {
    for (const auto& w:world.original_production_state()->services.witnesses)
        check(orig::service_timer(house(world,w.building_id),w.role)==timer(world.ticks(),w.arrival_tick,w.role),
              "Saved category timer differs from its own actual visit/countdown");
    for (const auto& c:world.couriers()) check(!c.cargo && !c.reserved,"School visitor acquired physical cargo");
    for (const auto& b:world.buildings()) check(!b.service_until_tick && !b.health_protection_until_tick,"School visitor granted legacy coverage");
}
constexpr std::array school_commands{sim::CommandType::PlaceOriginalMusicSchool,sim::CommandType::PlaceOriginalAcrobatSchool,sim::CommandType::PlaceOriginalDramaSchool};
constexpr std::array school_roles{orig::ServiceRole::Music,orig::ServiceRole::Acrobat,orig::ServiceRole::Drama};
struct Fixture {
    f::Root root;
    std::shared_ptr<const sim::MapPermissions> policy;
    std::shared_ptr<const orig::OriginalRulesModel> model;
    sim::World world;
    f::fs::path target=root.path/"culture-schema26.json";
    std::vector<sim::BuildingId> sources;
    std::array<sim::BuildingId,3> schools{};
    explicit Fixture(bool populate=true,std::array<int,3> needs={1,1,1},int width=32,int height=24):
        policy(std::make_shared<const sim::MapPermissions>(width,height,3,
            std::vector<sim::MapCellPermission>(static_cast<std::size_t>(width*height),
                {true,true,false,0,sim::BuildBlocker::None,sim::BuildBlocker::None}),std::vector<sim::FixedGatePassage>{})),
        model([&] {
            oe_production_fixture::write_model(root.data,7,0,0,0,0,0,0,14,22,0,0,0,0,0,0,0,0,0,needs[0],needs[1],needs[2]);
            return save::prepare_original_production_rules_model(root.data);
        }()),world(sim::World::create_original_production_development(policy,model)) {
        if (!populate) return;
        for (int x=0;x<32;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,4}});
        for (int y=5;y<=16;++y) f::buy(world,{sim::CommandType::PlaceRoad,{31,y}});
        for (int x=0;x<30;x+=3) (void)buy(sim::CommandType::PlaceHousehold,{x,2},false);
        for (const auto& [command,x]:std::array{
                std::pair{sim::CommandType::PlaceHealthPost,0},std::pair{sim::CommandType::PlaceServicePost,3},
                std::pair{sim::CommandType::PlaceOriginalAncestorShrine,6},std::pair{sim::CommandType::PlaceOriginalDaoistShrine,9},
                std::pair{sim::CommandType::PlaceOriginalBuddhistShrine,12},std::pair{sim::CommandType::PlaceOriginalConfucianAcademy,15},
                std::pair{sim::CommandType::PlaceOriginalWaterWell,19}}) (void)buy(command,{x,5});
        for (std::size_t i=0;i<school_commands.size();++i) schools[i]=buy(school_commands[i],{22+3*static_cast<int>(i),5});
        check(world.construction_spent_total()==1383 && world.treasury()==10617 && world.total_population()==0 &&
              world.couriers().size()==10 && world.original_production_state()->services.providers.size()==9 &&
              world.original_production_state()->services.trips.empty() && world.original_production_state()->services.witnesses.empty(),
              "Ten paid Houses/44 roads/nine services/Water did not pay1383 without free population or visits");
        for (std::size_t i=0;i<schools.size();++i) {
            const auto origin=world.building(schools[i]).cell;const int side=i==0 ? 2:3;
            for (int y=origin.y;y<origin.y+side;++y) for (int x=origin.x;x<origin.x+side;++x)
                check(world.building_owner_at({x,y})==schools[i],"School lost an actual2x2/3x3 body cell");
        }
    }
    sim::BuildingId buy(sim::CommandType command,sim::Cell cell,bool pause=true) {
        f::buy(world,{command,cell});const auto id=*world.building_owner_at(cell);
        if (pause) {sources.push_back(id);f::buy(world,sim::set_building_operation(id,false));}
        return id;
    }
    save::SaveDocument document(const sim::World& value) const {
        return save::make_document(root.data,f::map_relative,policy->building_mask(),value);
    }
    sim::World restore(const save::SaveDocument& value) const {
        return save::restore_original_production_development(value,root.data,policy->building_mask(),policy);
    }
    std::string roundtrip(const sim::World& value) const {
        witnesses_match(value);const auto before=value.snapshot();const auto doc=document(value);
        save::write_save(target,doc,root.data,policy->building_mask());const auto bytes=f::read_text(target);
        const auto parsed=save::read_save(target);f::same_document(doc,parsed);
        check(parsed.source_schema_version==26 && parsed.world.original_production->development_revision==23,"Culture save lost its strict revision23 identity");
        auto restored=restore(parsed);check(restored.snapshot()==before,"Culture F5/F9 changed full authority");
        save::write_save(target,document(restored),root.data,policy->building_mask());
        check(f::read_text(target)==bytes && value.snapshot()==before,"Culture canonical rewrite mutated authority");return bytes;
    }
    void replay(const sim::World& value) const {
        roundtrip(value);auto expected=sim::World::restore_original_production_development(value.snapshot(),policy,model);
        auto restored=restore(save::read_save(target));
        for (int tick=0;tick<20000;++tick) {
            if (tick==10000) {f::buy(expected,{sim::CommandType::PlaceRoad,{31,1}});f::buy(restored,{sim::CommandType::PlaceRoad,{31,1}});}
            expected.tick();restored.tick();
        }
        check(expected.snapshot()==restored.snapshot(),"Mid-school20k replay with genuine paid road diverged");witnesses_match(restored);
    }
};
struct States {sim::WorldSnapshot midroute,positive,expired;};
States lifecycle(Fixture& f) {
    f.roundtrip(f.world);until(f.world,[&]{return f.world.total_population()==70;});
    check(f.world.workforce_supply()==70 && f.world.original_production_state()->services.trips.empty(),"Paused schools granted free visits or lost actual worker origin");
    for (const auto source:f.sources) f::buy(f.world,sim::set_building_operation(source,true));
    until(f.world,[&]{return std::any_of(f.world.couriers().begin(),f.world.couriers().end(),[](const auto& c){return c.role==sim::CourierRole::OriginalDrama && c.phase==sim::CourierPhase::ToWarehouse && c.edge_progress>0;});});
    check(f.world.workforce_used()==70 && f.world.original_production_state()->services.witnesses.empty(),"School dispatch granted a category before actual arrival or lacked70 workers");
    const auto midroute=f.world.snapshot();f.replay(f.world);
    for (const auto source:f.sources) f::buy(f.world,sim::set_building_operation(source,false));
    until(f.world,[&]{return std::any_of(f.world.couriers().begin(),f.world.couriers().end(),[](const auto& c){return c.role==sim::CourierRole::OriginalDrama && c.phase==sim::CourierPhase::Returning && c.edge_progress>0;});});
    f.roundtrip(f.world);
    until(f.world,[&] {
        const auto& row=f.world.original_production_state()->housing.houses.front();
        return row.last_evaluation && row.last_evaluation->input.drama>0 &&
            std::all_of(f.world.couriers().begin(),f.world.couriers().end(),[](const auto& c){return c.phase==sim::CourierPhase::IdleAtWorkshop;});
    });
    const auto target=f.world.original_economy_state()->houses.front().building_id;const auto& h=house(f.world,target);
    check(f.world.original_production_state()->services.witnesses.size()==9 && f.world.original_production_state()->services.trips.size()==9 &&
          h.music && h.acrobat && h.drama && h.herbalist && h.acupuncture && h.ancestor && h.confucian && h.daoist && h.buddhist && h.water &&
          orig::needs_met(f.model->house_rules(),orig::ModelHouse::Shelter,h),"Independent school arrivals overwrote another actual service or failed authored culture needs");
    constexpr std::array roles{orig::ServiceRole::Herbalist,orig::ServiceRole::Acupuncture,orig::ServiceRole::Ancestor,
        orig::ServiceRole::Confucian,orig::ServiceRole::Daoist,orig::ServiceRole::Buddhist,orig::ServiceRole::Music,orig::ServiceRole::Acrobat,orig::ServiceRole::Drama};
    const auto& slots=f.world.original_production_state()->housing.houses.front().last_evaluation->service_witnesses;
    for (std::size_t i=0;i<roles.size();++i) check(slots[i] && slots[i]->role==roles[i],"Nine historical witness slots changed their explicit order");
    for (const auto& other:f.world.original_economy_state()->houses) if (other.building_id!=target)
        check(!other.house.music && !other.house.acrobat && !other.house.drama,"Actual school arrival globally buffed another House");
    const auto positive=f.world.snapshot();f.roundtrip(f.world);
    std::uint64_t expiry=0;
    for (const auto& w:f.world.original_production_state()->services.witnesses) if (w.role>=orig::ServiceRole::Music && w.role<=orig::ServiceRole::Drama)
        expiry=std::max(expiry,36+(decays(w.arrival_tick-1)+95)*51);
    check(expiry>f.world.ticks(),"School fixture passed its actual96th countdown");
    while (f.world.ticks()+1<expiry) f.world.tick();
    const auto& before=house(f.world,target);
    check(before.music==1 || before.acrobat==1 || before.drama==1,"Last school timer expired before its96th countdown");f.roundtrip(f.world);f.world.tick();
    const auto& after=house(f.world,target);
    check(f.world.ticks()==expiry && !after.music && !after.acrobat && !after.drama &&
          !orig::needs_met(f.model->house_rules(),orig::ModelHouse::Shelter,after),"Idle/return granted culture after actual96-countdown expiry");
    f.roundtrip(f.world);return {midroute,positive,f.world.snapshot()};
}
void individual(std::size_t category) {
    std::array<int,3> needs{};needs[category]=1;Fixture f(true,needs);
    until(f.world,[&]{return f.world.total_population()==70;});const auto target=f.world.original_economy_state()->houses.front().building_id;
    check(!orig::needs_met(f.model->house_rules(),orig::ModelHouse::Shelter,house(f.world,target)),"Unserved authored culture need was already fulfilled");
    f::buy(f.world,sim::set_building_operation(f.schools[category],true));
    until(f.world,[&]{return orig::service_timer(house(f.world,target),school_roles[category])>0;});
    f::buy(f.world,sim::set_building_operation(f.schools[category],false));
    check(orig::service_timer(house(f.world,target),school_roles[category])==96,"Actual individual school arrival did not set its own96 timer");
    for (std::size_t other=0;other<3;++other) if (other!=category)
        check(!orig::service_timer(house(f.world,target),school_roles[other]),"Individual school granted the paired or another category");
    check(orig::needs_met(f.model->house_rules(),orig::ModelHouse::Shelter,house(f.world,target)),"Actual individual school failed its named authored need");
    f.roundtrip(f.world);const auto next=36+decays(f.world.ticks())*51;
    while (f.world.ticks()+1<next) f.world.tick();f.roundtrip(f.world);f.world.tick();
    check(orig::service_timer(house(f.world,target),school_roles[category])==95,"Actual next phase35 did not decrement96 to95 once");f.roundtrip(f.world);
    until(f.world,[&]{return !orig::service_timer(house(f.world,target),school_roles[category]);});
    check(!orig::needs_met(f.model->house_rules(),orig::ModelHouse::Shelter,house(f.world,target)),"Expired named culture category retained its authored need");f.roundtrip(f.world);
}
void hostile(Fixture& f,const States& states) {
    auto actual=sim::World::restore_original_production_development(states.positive,f.policy,f.model);
    const auto pristine=f.roundtrip(actual);const auto good=json::parse(pristine);const auto path=f.root.path/"bad-culture.json";
    const auto reject=[&](const json& value) {
        f::write_text(path,value.dump());
        try {refuses([&]{(void)f.restore(save::read_save(path));});}
        catch (const std::exception& error) {throw std::runtime_error(std::string(error.what())+": "+json::diff(good,value).dump());}
        check(f::read_text(f.target)==pristine,"Malformed culture input replaced protected good data");
    };
    const std::string base="/world/original_production/services",evaluation="/world/original_production/housing/houses/0/last_evaluation";
    const auto& services=good.at(json::json_pointer(base));std::size_t pi=0,wi=0,ti=0,ci=0;
    while (services["providers"][pi]["type"]!=8) ++pi;
    while (services["witnesses"][wi]["role"]!=4) ++wi;
    const auto actor=services["witnesses"][wi]["transfer"]["courier_id"];
    while (services["trips"][ti]["transfer"]["courier_id"]!=actor) ++ti;
    while (good["world"]["couriers"][ci]["id"]!=actor) ++ci;
    const auto provider=base+"/providers/"+std::to_string(pi),trip=base+"/trips/"+std::to_string(ti),witness=base+"/witnesses/"+std::to_string(wi);
    const std::string home="/world/original_economy/houses/0/house/";
    std::vector<std::string> objects;
    const std::function<void(const json&,std::string)> walk=[&](const json& value,std::string prefix) {
        if (value.is_object()) {objects.push_back(prefix);for (const auto& [key,child]:value.items()) walk(child,prefix+"/"+key);}
        else if (value.is_array()) for (std::size_t i=0;i<value.size();++i) walk(value[i],prefix+"/"+std::to_string(i));
    };
    walk(services,base);
    for (const auto& prefix:objects) {
        auto value=good;value.at(json::json_pointer(prefix))["unsupported_culture_authority"]=true;reject(value);
        for (const auto& [key,unused]:good.at(json::json_pointer(prefix)).items()) {(void)unused;value=good;value.at(json::json_pointer(prefix)).erase(key);reject(value);}
    }
    for (const auto& [pointer,value]:std::vector<std::pair<std::string,json>>{
            {provider+"/type",9},{provider+"/type",6},{witness+"/role",9},{witness+"/role",2},{witness+"/role",3},
            {trip+"/arrival_tick",nullptr},{trip+"/visit_applied",false},{trip+"/target",static_cast<unsigned>(f.schools[2])},
            {home+"drama",0},{home+"drama",97},{home+"acrobat",96},{home+"music",255},
            {evaluation+"/service_witnesses/8",nullptr},{evaluation+"/input/drama",0},
            {"/world/couriers/"+std::to_string(ci)+"/role",20},{"/world/couriers/"+std::to_string(ci)+"/role",23},
            {"/world/original_production/development_revision",22},{"/world/original_production/development_revision",24}}) {
        auto bad=good;bad.at(json::json_pointer(pointer))=value;reject(bad);
    }
    for (const auto& pointer:{provider+"/building_id",trip+"/target",trip+"/transfer/courier_id",trip+"/transfer/sequence",trip+"/dispatched_tick",trip+"/arrival_tick",witness+"/provider_id"})
        for (const json& value:{json(0),json(-1),json(true),json(1.0),json("1"),json(UINT64_MAX)}) {auto bad=good;bad.at(json::json_pointer(pointer))=value;reject(bad);}
    for (const auto& [key,limit]:std::array{std::pair{"providers",18U},std::pair{"trips",18U},std::pair{"witnesses",180U}}) {
        auto bad=good;auto& rows=bad["world"]["original_production"]["services"][key];while (rows.size()<=limit) rows.push_back(rows[0]);reject(bad);
    }
    {auto bad=good;auto& slots=bad.at(json::json_pointer(evaluation+"/service_witnesses"));std::swap(slots[6],slots[8]);reject(bad);}
    for (const std::size_t length:{6U,8U,10U}) {auto bad=good;auto& slots=bad.at(json::json_pointer(evaluation+"/service_witnesses"));while (slots.size()>length) slots.erase(slots.size()-1);while (slots.size()<length) slots.push_back(nullptr);reject(bad);}
    {auto bad=good;const auto early=bad.at(json::json_pointer(trip+"/dispatched_tick")).get<std::uint64_t>()+1;
        bad.at(json::json_pointer(trip+"/arrival_tick"))=early;bad.at(json::json_pointer(witness+"/arrival_tick"))=early;
        bad.at(json::json_pointer(home+"drama"))=timer(actual.ticks(),early,orig::ServiceRole::Drama);reject(bad);}
    const auto doc=f.document(actual);
    for (int fault=0;fault<6;++fault) {
        auto bad=doc;
        if (fault==0) bad.world.original_production->services.providers[pi].type=sim::OriginalServiceProviderType::Acrobat212;
        if (fault==1) bad.world.original_production->services.trips[ti].transfer.sequence=UINT64_MAX;
        if (fault==2) bad.world.original_production->services.witnesses[wi].arrival_tick=bad.world.ticks+1;
        if (fault==3) bad.world.original_production->housing.houses[0].last_evaluation->service_witnesses[8].reset();
        if (fault==4) bad.world.original_production->development_revision=22;
        if (fault==5) {auto& body=*std::find_if(bad.world.buildings.begin(),bad.world.buildings.end(),[&](const auto& b){return b.id==f.schools[2];});body.cell={INT32_MAX,INT32_MIN};}
        refuses([&]{(void)f.restore(bad);});refuses([&]{save::write_save(f.target,bad,f.root.data,f.policy->building_mask());});
        check(f::read_text(f.target)==pristine,"Bad culture writer replaced protected authority");
    }
    auto mid=sim::World::restore_original_production_development(states.midroute,f.policy,f.model);
    auto phantom=json::parse(f.roundtrip(mid));phantom["world"]["original_economy"]["houses"][0]["house"]["drama"]=1;
    f::write_text(path,phantom.dump());refuses([&]{(void)f.restore(save::read_save(path));});f.roundtrip(actual);
    auto bad=doc;++bad.world.construction_spent_total;--bad.world.treasury;sim::World::validate_original_production_snapshot(bad.world);
    refuses([&]{(void)f.restore(bad);});refuses([&]{save::write_save(f.target,bad,f.root.data,f.policy->building_mask());});
    auto duplicate=pristine;const auto marker=duplicate.find("\"services\": {");check(marker!=std::string::npos,"Culture services object absent");
    duplicate.insert(duplicate.find('{',marker)+1,"\"witnesses\":[],");f::write_text(path,duplicate);refuses([&]{(void)save::read_save(path);});
    check(f::read_text(f.target)==pristine,"Modelbound or duplicate culture input replaced protected data");
}
void maximum_and_frozen_bounds() {
    Fixture f(false,{0,0,0},44,30);
    for (int i=0;i<33;++i) {const auto id=f::source(f.world,{0,0});f::buy(f.world,sim::demolish_building(id));}
    for (const int y:{1,5}) for (int x=1;x<41;x+=4) (void)f.buy(sim::CommandType::PlaceHousehold,{x,y},false);
    for (const int x:{1,5,9,13}) (void)f.buy(sim::CommandType::PlacePottery,{x,9});
    for (const int x:{17,21}) (void)f.buy(sim::CommandType::PlaceWarehouse,{x,9});
    for (const int x:{25,29,33,37}) (void)f.buy(sim::CommandType::PlaceMarket,{x,9});
    for (const int x:{1,9}) {const auto id=f.buy(sim::CommandType::PlaceOriginalHempFarm,{x,13});sim::Command field{sim::CommandType::PlaceOriginalHempField,{x+2,13}};field.building_id=id;f::buy(f.world,field);}
    for (const int x:{17,25}) {const auto id=f.buy(sim::CommandType::PlaceOriginalFoodFarm,{x,13});sim::Command field{x==17 ? sim::CommandType::PlaceOriginalWheatField:sim::CommandType::PlaceOriginalMilletField,{x+3,13}};field.building_id=id;f::buy(f.world,field);}
    constexpr std::array commands{sim::CommandType::PlaceHealthPost,sim::CommandType::PlaceServicePost,sim::CommandType::PlaceOriginalAncestorShrine,
        sim::CommandType::PlaceOriginalDaoistShrine,sim::CommandType::PlaceOriginalBuddhistShrine,sim::CommandType::PlaceOriginalConfucianAcademy,
        sim::CommandType::PlaceOriginalMusicSchool,sim::CommandType::PlaceOriginalAcrobatSchool,sim::CommandType::PlaceOriginalDramaSchool};
    for (std::size_t i=0;i<commands.size();++i) for (int j=0;j<2;++j) (void)f.buy(commands[i],{1+4*(2*static_cast<int>(i%3)+j),17+4*static_cast<int>(i/3)});
    for (const int x:{29,33}) (void)f.buy(sim::CommandType::PlaceOriginalWaterWell,{x,17});
    check(f.world.buildings().size()==58 && f.world.couriers().size()==38 && f.world.next_building_id()==92 && f.world.next_courier_id()==39 &&
          f.world.original_production_state()->services.providers.size()==18 && f.world.original_influence_state()->removed_sources.size()==33 && !f.world.total_population(),
          "Genuine paid58/38 context or contiguous91-purchase history failed");
    const auto pristine=f.roundtrip(f.world);const auto good=json::parse(pristine);const auto path=f.root.path/"culture-bounds.json";
    for (const char* key:{"buildings","couriers"}) {auto bad=good;bad["world"][key].push_back(bad["world"][key][0]);f::write_text(path,bad.dump());refuses([&]{(void)save::read_save(path);});}
    auto legacy=good;legacy["schema_version"]=22;legacy["rules"]["version"]=6;
    for (const char* key:{"original_production","original_economy","original_influence","original_goods"}) legacy["world"].erase(key);
    const auto old_parse=[&](const json& value) {f::write_text(path,value.dump());bool exact=false;try {(void)save::read_save(path);} catch (const std::exception& e) {exact=std::string(e.what())=="invalid scalable City entity array length";}check(exact,"Frozen schema22 inherited active58/38 bounds");++rejected;};
    auto bodies=legacy;bodies["world"]["couriers"].erase(bodies["world"]["couriers"].begin()+26,bodies["world"]["couriers"].end());old_parse(bodies);
    auto actors=legacy;actors["world"]["buildings"].erase(actors["world"]["buildings"].begin()+46,actors["world"]["buildings"].end());old_parse(actors);
    auto old=f.world.snapshot();old.rule_version=6;old.original_production.reset();old.original_goods.reset();old.original_influence.reset();old.original_economy.reset();
    const auto old_restore=[&](const sim::WorldSnapshot& value) {bool exact=false;try {(void)sim::World::restore(value,f.policy);} catch (const std::exception& e) {exact=std::string(e.what())=="invalid City-v10 entity counts or next IDs";}check(exact,"Frozen rule6 restore inherited active58/38 bounds");++rejected;};
    auto old_bodies=old;old_bodies.couriers.resize(26);old_restore(old_bodies);auto old_actors=old;old_actors.buildings.resize(46);old_restore(old_actors);
    check(f::read_text(f.target)==pristine,"Entity bound refusals replaced protected good data");
}
void school_third_column_routes() {
    for (const auto command:{sim::CommandType::PlaceOriginalAcrobatSchool,sim::CommandType::PlaceOriginalDramaSchool}) {
        Fixture f(false,{0,0,0});
        for (const sim::Cell cell:std::array{sim::Cell{3,5},sim::Cell{4,5},sim::Cell{3,6},sim::Cell{3,7},sim::Cell{3,8},sim::Cell{4,8}}) f::buy(f.world,{sim::CommandType::PlaceRoad,cell});
        const auto target=f.buy(sim::CommandType::PlaceHousehold,{5,5},false);(void)f.buy(sim::CommandType::PlaceHousehold,{5,8},false);
        const auto school=f.buy(command,{0,5});until(f.world,[&]{return f.world.total_population()==14;});f::buy(f.world,sim::set_building_operation(school,true));f.world.tick();
        const auto& actor=f.world.couriers().front();const auto role=command==sim::CommandType::PlaceOriginalAcrobatSchool ? orig::ServiceRole::Acrobat:orig::ServiceRole::Drama;
        check(actor.phase==sim::CourierPhase::ToWarehouse && actor.target==target && actor.path.size()==4 && actor.path.front()==sim::Cell{2,5},"3x3 school did not use its actual third body column");
        f.roundtrip(f.world);f::buy(f.world,sim::set_building_operation(school,false));until(f.world,[&]{return orig::service_timer(house(f.world,target),role)>0;});
        const auto trip=f.world.original_production_state()->services.trips.front();check(trip.arrival_tick && *trip.arrival_tick-trip.dispatched_tick==14,"3x3 school arrival did not use actual three five-tick edges");f.roundtrip(f.world);
        until(f.world,[&]{return f.world.couriers().front().phase==sim::CourierPhase::Returning && f.world.couriers().front().edge_progress>0;});f.roundtrip(f.world);
        until(f.world,[&]{return f.world.couriers().front().phase==sim::CourierPhase::IdleAtWorkshop;});f.roundtrip(f.world);
    }
}
}
int main() {
    try {Fixture fixture;const auto states=lifecycle(fixture);hostile(fixture,states);for (std::size_t i=0;i<3;++i) individual(i);
        maximum_and_frozen_bounds();school_third_column_routes();
        std::cout<<"Culture save PASS: paid211/212/213 actual separate96 visits/expiry, nine historical categories, F5F9/20k,3x3 source travel,58/38 paid context and "<<rejected<<" hostile refusals\n";return 0;
    } catch (const std::exception& error) {std::cerr<<"Original culture save FAIL: "<<error.what()<<'\n';return 1;}
}
