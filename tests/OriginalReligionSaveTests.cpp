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
    throw std::runtime_error("Malformed religion save authority was admitted");
}
template<class P> void until(sim::World& world,P condition,int limit=30000) {
    for (int i=0;i<limit && !condition();++i) world.tick();
    check(condition(),"Paid religion scene did not reach its required actual state");
}
std::uint64_t decays(std::uint64_t tick) {return tick<36 ? 0:1+(tick-36)/51;}
std::uint8_t timer(std::uint64_t tick,std::uint64_t arrival,orig::ServiceRole role) {
    check(arrival && arrival<=tick,"Independent religion timer requires an actual prior arrival");
    const auto n=decays(tick)-decays(arrival-1);
    const unsigned duration=role==orig::ServiceRole::Herbalist || role==orig::ServiceRole::Acupuncture ? 96U:40U;
    return static_cast<std::uint8_t>(n>=duration ? 0:duration-n);
}
const orig::HouseState& house(const sim::World& world,sim::BuildingId id) {
    const auto& rows=world.original_economy_state()->houses;
    const auto found=std::find_if(rows.begin(),rows.end(),[&](const auto& h){return h.building_id==id;});
    check(found!=rows.end(),"Paid religion target House disappeared");return found->house;
}
void witnesses_match(const sim::World& world) {
    for (const auto& witness:world.original_production_state()->services.witnesses)
        check(orig::service_timer(house(world,witness.building_id),witness.role)==timer(world.ticks(),witness.arrival_tick,witness.role),
              "Saved service timer differs from its independent actual arrival/category countdown");
    for (const auto& h:world.original_economy_state()->houses)
        check(!h.house.music && !h.house.acrobat && !h.house.drama,"Religion granted an unsupported culture category");
    for (const auto& actor:world.couriers()) check(!actor.cargo && !actor.reserved,"Cargo-free religion scene acquired cargo/reservations");
}
struct Fixture {
    f::Root root;
    std::shared_ptr<const sim::MapPermissions> policy;
    std::shared_ptr<const orig::OriginalRulesModel> model;
    sim::World world;
    f::fs::path target=root.path/"religion-schema26.json";
    std::vector<sim::BuildingId> sources;
    sim::BuildingId dao{},buddhist{},confucian{};
    std::optional<oe_production_fixture::PaperSupply> paper;
    int expected_population=56;
    explicit Fixture(bool populate=true,int confucian_need=1,int or_need=1,int width=24,int height=20):
        policy(std::make_shared<const sim::MapPermissions>(width,height,3,
            std::vector<sim::MapCellPermission>(static_cast<std::size_t>(width*height),
                {true,true,false,0,sim::BuildBlocker::None,sim::BuildBlocker::None}),std::vector<sim::FixedGatePassage>{})),
        model([&] {
            oe_production_fixture::write_model(root.data,7,0,0,0,0,0,0,14,22,0,0,0,0,confucian_need,or_need);
            return save::prepare_original_production_rules_model(root.data);
        }()),world(sim::World::create_original_production_development(policy,model)) {
        if (!populate) return;
        for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,4}});
        for (int x=0;x<24;x+=3)
            (void)buy(x==0 && confucian_need>0 ? sim::CommandType::PlaceOriginalElitePlot:
                sim::CommandType::PlaceHousehold,
                confucian_need>0 ? (x==0 ? sim::Cell{0,0}:sim::Cell{x+1,2}):sim::Cell{x,2},false);
        if (confucian_need>0) expected_population=54; // Seven Common7 + genuine Elite5.
        for (const auto& [command,x]:std::array{
                std::pair{sim::CommandType::PlaceHealthPost,0},std::pair{sim::CommandType::PlaceServicePost,3},
                std::pair{sim::CommandType::PlaceOriginalAncestorShrine,6},std::pair{sim::CommandType::PlaceOriginalWaterWell,9},
                std::pair{sim::CommandType::PlaceOriginalDaoistShrine,12},std::pair{sim::CommandType::PlaceOriginalBuddhistShrine,15},
                std::pair{sim::CommandType::PlaceOriginalConfucianAcademy,18}}) {
            const auto id=buy(command,{x,5});
            if (command==sim::CommandType::PlaceOriginalDaoistShrine) dao=id;
            if (command==sim::CommandType::PlaceOriginalBuddhistShrine) buddhist=id;
            if (command==sim::CommandType::PlaceOriginalConfucianAcademy) confucian=id;
        }
        check(world.construction_spent_total()==1003 && world.treasury()==10997 && world.total_population()==0 &&
              world.couriers().size()==7 && world.original_production_state()->services.providers.size()==6 &&
              world.original_production_state()->services.trips.empty() && world.original_production_state()->services.witnesses.empty(),
              "Eight Houses and seven real sources did not pay their exact model/road costs without grants");
        for (int y=5;y<8;++y) for (int x=18;x<21;++x)
            check(world.building_owner_at({x,y})==confucian,"Paid Academy did not own all nine actual footprint cells");
        if (confucian_need>0) {
            oe_production_fixture::PaperSupplyLayout layout;layout.farm={0,17};layout.field={2,17};
            layout.paper={4,17};layout.warehouse={8,17};
            for (int x=0;x<24;++x) layout.roads.push_back({x,16});
            for (int y=5;y<16;++y) layout.roads.push_back({23,y});
            for (const int x:{7,10,13}) {layout.extra_homes.push_back({x,0});for (int y=0;y<4;++y) layout.roads.push_back({x-1,y});}
            paper=oe_production_fixture::add_paper_supply(world,layout);expected_population+=21;
        }
    }
    sim::BuildingId buy(sim::CommandType command,sim::Cell cell,bool pause=true) {
        f::buy(world,{command,cell});const auto id=*world.building_owner_at(cell);
        if (pause) {sources.push_back(id);f::buy(world,sim::set_building_operation(id,false));}
        return id;
    }
    void prepare_paper() {
        check(paper.has_value(),"Positive Academy requires actual paid Paper production and delivery");
        until(world,[&]{const auto& stocks=world.original_production_state()->industry.auxiliary;return std::any_of(stocks.begin(),stocks.end(),[&](const auto& row){return row.building_id==confucian && row.amounts[4]>0;});});
        for (const auto id:{paper->farm,paper->paper,paper->warehouse}) f::buy(world,sim::set_building_operation(id,false));
        until(world,[&]{return std::all_of(world.couriers().begin(),world.couriers().end(),[](const auto& c){return c.phase==sim::CourierPhase::IdleAtWorkshop;});});
    }
    save::SaveDocument document(const sim::World& value) const {
        return save::make_document(root.data,f::map_relative,policy->building_mask(),value);
    }
    sim::World restore(const save::SaveDocument& doc) const {
        return save::restore_original_production_development(doc,root.data,policy->building_mask(),policy);
    }
    std::string roundtrip(const sim::World& value) const {
        witnesses_match(value);const auto before=value.snapshot();const auto doc=document(value);
        save::write_save(target,doc,root.data,policy->building_mask());const auto bytes=f::read_text(target);
        const auto parsed=save::read_save(target);f::same_document(doc,parsed);
        check(parsed.source_schema_version==26 && parsed.world.original_production->development_revision==27,
              "Religion save did not use its own strict active revision27");
        auto copy=restore(parsed);check(copy.snapshot()==before,"Religion F5/F9 changed full World authority");
        save::write_save(target,document(copy),root.data,policy->building_mask());
        check(f::read_text(target)==bytes && value.snapshot()==before,"Religion canonical rewrite mutated saved/live authority");
        return bytes;
    }
    void replay(const sim::World& value) const {
        roundtrip(value);auto expected=sim::World::restore_original_production_development(value.snapshot(),policy,model);
        auto copy=restore(save::read_save(target));
        for (int tick=0;tick<20000;++tick) {
            if (tick==10000) {
                f::buy(expected,{sim::CommandType::PlaceRoad,{23,1}});
                f::buy(copy,{sim::CommandType::PlaceRoad,{23,1}});
            }
            expected.tick();copy.tick();
        }
        check(expected.snapshot()==copy.snapshot(),"Real mid-religion20k replay with paid road diverged");
        witnesses_match(expected);witnesses_match(copy);
    }
};
struct States {sim::WorldSnapshot midroute,positive,expired;};
States lifecycle(Fixture& f) {
    f.roundtrip(f.world);until(f.world,[&]{return f.world.total_population()==f.expected_population;});f.prepare_paper();
    check(f.world.workforce_supply()==f.expected_population && f.world.original_production_state()->services.trips.empty(),
          "Paused providers granted visits or lost genuinely admitted workers");
    for (const auto id:f.sources) f::buy(f.world,sim::set_building_operation(id,true));
    until(f.world,[&] {
        for (const auto& c:f.world.couriers()) if (c.role==sim::CourierRole::OriginalConfucian &&
            c.phase==sim::CourierPhase::ToWarehouse && c.edge_progress>0) return true;
        return false;
    });
    check(f.world.original_production_state()->services.witnesses.empty(),"Source purchase/dispatch/mid-edge granted a religion visit");
    const auto midroute=f.world.snapshot();f.replay(f.world);
    for (const auto id:f.sources) f::buy(f.world,sim::set_building_operation(id,false));
    until(f.world,[&] {
        for (const auto& c:f.world.couriers()) if (c.role==sim::CourierRole::OriginalConfucian &&
            c.phase==sim::CourierPhase::Returning && c.edge_progress>0) return true;
        return false;
    });
    f.roundtrip(f.world);
    until(f.world,[&] {
        const auto& h=f.world.original_production_state()->housing.houses.front();
        return h.last_evaluation && h.last_evaluation->input.confucian>0 &&
            std::all_of(f.world.couriers().begin(),f.world.couriers().end(),[](const auto& c){return c.phase==sim::CourierPhase::IdleAtWorkshop;});
    });
    const auto& visits=f.world.original_production_state()->services.witnesses;
    check(visits.size()==6 && f.world.original_production_state()->services.trips.size()==6,
          "Six paused outgoing category visitors did not complete exactly six own actual visits");
    const auto target=f.world.original_economy_state()->houses.front().building_id;
    const auto& supplied=house(f.world,target);
    check(f.world.building(target).kind==sim::Object::OriginalEliteHouse && orig::is_elite(supplied.type) &&
          supplied.herbalist && supplied.acupuncture && supplied.ancestor && supplied.confucian && supplied.daoist && supplied.buddhist &&
          orig::needs_met(f.model->house_rules(),orig::ModelHouse::Shelter,supplied),
          "Real independent religious arrivals failed their authored Confucian/OR requirement");
    const auto& slots=f.world.original_production_state()->housing.houses.front().last_evaluation->service_witnesses;
    constexpr std::array roles{orig::ServiceRole::Herbalist,orig::ServiceRole::Acupuncture,orig::ServiceRole::Ancestor,
        orig::ServiceRole::Confucian,orig::ServiceRole::Daoist,orig::ServiceRole::Buddhist};
    for (std::size_t i=0;i<roles.size();++i)
        check(slots[i] && slots[i]->role==roles[i],"Historical six witness slots changed category order");
    for (const auto& h:f.world.original_economy_state()->houses) if (h.building_id!=target)
        check(!h.house.confucian && !h.house.daoist && !h.house.buddhist,"A local actual visit buffed another House globally");
    const auto positive=f.world.snapshot();f.roundtrip(f.world);
    const auto conf=std::find_if(visits.begin(),visits.end(),[](const auto& w){return w.role==orig::ServiceRole::Confucian;});
    check(conf!=visits.end(),"No actual Academy witness");const auto arrival=conf->arrival_tick;
    const auto expiry=36+(decays(arrival-1)+39)*51;
    while (f.world.ticks()+1<expiry) f.world.tick();
    check(house(f.world,target).confucian==1,"Confucian expired before its actual40th countdown");f.roundtrip(f.world);
    f.world.tick();
    check(f.world.ticks()==expiry && !house(f.world,target).confucian && house(f.world,target).herbalist>0 &&
          !orig::needs_met(f.model->house_rules(),orig::ModelHouse::Shelter,house(f.world,target)),
          "Expired religion was renewed by idle/return or substituted by96-unit medicine");
    f.roundtrip(f.world);return {midroute,positive,f.world.snapshot()};
}
void single_or(bool buddhist) {
    Fixture f(true,0,1);until(f.world,[&]{return f.world.total_population()==56;});
    const auto target=f.world.original_economy_state()->houses.front().building_id;
    check(!orig::needs_met(f.model->house_rules(),orig::ModelHouse::Shelter,house(f.world,target)),"Neither religion satisfied an authored OR need");
    const auto owner=buddhist ? f.buddhist:f.dao;
    f::buy(f.world,sim::set_building_operation(owner,true));
    until(f.world,[&]{return buddhist ? house(f.world,target).buddhist>0:house(f.world,target).daoist>0;});
    f::buy(f.world,sim::set_building_operation(owner,false));
    check((buddhist ? !house(f.world,target).daoist:!house(f.world,target).buddhist) && !house(f.world,target).confucian &&
          orig::needs_met(f.model->house_rules(),orig::ModelHouse::Shelter,house(f.world,target)),
          "Daoist/Buddhist-only actual arrival failed OR or invented the other category");
    f.roundtrip(f.world);
    until(f.world,[&]{return !house(f.world,target).daoist && !house(f.world,target).buddhist;});
    check(!orig::needs_met(f.model->house_rules(),orig::ModelHouse::Shelter,house(f.world,target)),"Expired single religion still fulfilled OR");
    f.roundtrip(f.world);
}
void hostile(Fixture& f,const States& states) {
    auto actual=sim::World::restore_original_production_development(states.positive,f.policy,f.model);
    const auto pristine=f.roundtrip(actual);const auto good=json::parse(pristine);const auto path=f.root.path/"bad-religion.json";
    const auto reject=[&](const json& value) {
        f::write_text(path,value.dump());
        try {refuses([&]{(void)f.restore(save::read_save(path));});}
        catch (const std::exception& error) {throw std::runtime_error(std::string(error.what())+": "+json::diff(good,value).dump());}
        check(f::read_text(f.target)==pristine,"Hostile religion input replaced the protected good save");
    };
    const std::string base="/world/original_production/services",evaluation="/world/original_production/housing/houses/0/last_evaluation";
    std::size_t pi=0,wi=0,ti=0,ci=0;
    const auto& services=good.at(json::json_pointer(base));
    while (services["providers"][pi]["type"]!=5) ++pi;
    while (services["witnesses"][wi]["role"]!=6) ++wi;
    const auto actor=services["witnesses"][wi]["transfer"]["courier_id"];
    while (services["trips"][ti]["transfer"]["courier_id"]!=actor) ++ti;
    while (good["world"]["couriers"][ci]["id"]!=actor) ++ci;
    const auto provider=base+"/providers/"+std::to_string(pi),witness=base+"/witnesses/"+std::to_string(wi),trip=base+"/trips/"+std::to_string(ti);
    const std::string home="/world/original_economy/houses/0/house/";
    std::vector<std::string> objects;
    const std::function<void(const json&,std::string)> visit=[&](const json& value,std::string prefix) {
        if (value.is_object()) {objects.push_back(prefix);for (const auto& [key,child]:value.items()) visit(child,prefix+"/"+key);}
        else if (value.is_array()) for (std::size_t i=0;i<value.size();++i) visit(value[i],prefix+"/"+std::to_string(i));
    };
    visit(services,base);
    for (const auto& prefix:objects) {
        auto value=good;value.at(json::json_pointer(prefix))["unsupported_religion_authority"]=true;reject(value);
        for (const auto& [key,unused]:good.at(json::json_pointer(prefix)).items()) {
            (void)unused;value=good;value.at(json::json_pointer(prefix)).erase(key);reject(value);
        }
    }
    for (const auto& [pointer,replacement]:std::vector<std::pair<std::string,json>>{
            {provider+"/type",6},{provider+"/type",0},{witness+"/role",2},{witness+"/role",3},{witness+"/role",4},
            {witness+"/role",7},{witness+"/role",8},{witness+"/provider_id",static_cast<unsigned>(f.dao)},
            {trip+"/target",static_cast<unsigned>(f.confucian)},{trip+"/arrival_tick",nullptr},{trip+"/visit_applied",false},
            {home+"confucian",0},{home+"confucian",41},{home+"confucian",96},{home+"music",40},
            {evaluation+"/service_witnesses/3",nullptr},{evaluation+"/input/confucian",0},
            {"/world/couriers/"+std::to_string(ci)+"/role",17},{"/world/couriers/"+std::to_string(ci)+"/role",20},
            {"/world/original_production/development_revision",21},{"/world/original_production/development_revision",22},{"/world/original_production/development_revision",23},{"/world/original_production/development_revision",24},{"/world/original_production/development_revision",25},{"/world/original_production/development_revision",26},{"/world/original_production/development_revision",28}}) {
        auto value=good;value.at(json::json_pointer(pointer))=replacement;reject(value);
    }
    for (const auto& pointer:{provider+"/building_id",witness+"/provider_id",trip+"/target",trip+"/transfer/courier_id",trip+"/transfer/sequence",trip+"/dispatched_tick",trip+"/arrival_tick"})
        for (const json& invalid:{json(0),json(-1),json(true),json(1.0),json("1"),json(UINT64_MAX)}) {
            auto value=good;value.at(json::json_pointer(pointer))=invalid;reject(value);
        }
    for (const auto& [key,limit]:std::array{std::pair{"providers",12U},std::pair{"trips",12U},std::pair{"witnesses",120U}}) {
        auto value=good;auto& rows=value["world"]["original_production"]["services"][key];
        while (rows.size()<=limit) rows.push_back(rows[0]);
        reject(value);
    }
    {auto value=good;value.at(json::json_pointer(evaluation+"/service_witnesses")).erase(0);reject(value);}
    {auto value=good;auto& slots=value.at(json::json_pointer(evaluation+"/service_witnesses"));std::swap(slots[3],slots[4]);reject(value);}
    {auto value=good;value.at(json::json_pointer(evaluation+"/service_witnesses")).push_back(nullptr);reject(value);}
    {auto value=good;auto& t=value.at(json::json_pointer(trip));auto& w=value.at(json::json_pointer(witness));
        const auto early=t["dispatched_tick"].get<std::uint64_t>()+1;t["arrival_tick"]=early;w["arrival_tick"]=early;
        value.at(json::json_pointer(home+"confucian"))=timer(actual.ticks(),early,orig::ServiceRole::Confucian);reject(value);}
    {auto value=good;value.at(json::json_pointer(trip+"/dispatched_tick"))=actual.ticks();reject(value);}
    const auto document=f.document(actual);
    for (int fault=0;fault<7;++fault) {
        auto bad=document;
        if (fault==0) bad.world.original_production->services.providers[pi].type=sim::OriginalServiceProviderType::Daoist215;
        if (fault==1) bad.world.original_production->services.trips[ti].transfer.sequence=UINT64_MAX;
        if (fault==2) bad.world.original_production->services.witnesses[wi].arrival_tick=bad.world.ticks+1;
        if (fault==3) bad.world.original_production->housing.houses[0].last_evaluation->service_witnesses[3].reset();
        if (fault==4) bad.world.original_economy->houses[0].house.confucian=96;
        if (fault==5) bad.world.original_production->development_revision=21;
        if (fault==6) {
            auto& body=*std::find_if(bad.world.buildings.begin(),bad.world.buildings.end(),[&](const auto& b){return b.id==f.confucian;});
            body.cell={INT32_MAX,INT32_MIN};
        }
        refuses([&]{(void)f.restore(bad);});
        refuses([&]{save::write_save(f.target,bad,f.root.data,f.policy->building_mask());});
        check(f::read_text(f.target)==pristine,"Invalid religion DTO replaced valid saved authority");
    }
    auto absent=sim::World::restore_original_production_development(states.midroute,f.policy,f.model);
    auto fabricated=json::parse(f.roundtrip(absent));fabricated["world"]["original_economy"]["houses"][0]["house"]["confucian"]=1;
    f::write_text(path,fabricated.dump());refuses([&]{(void)f.restore(save::read_save(path));});
    f.roundtrip(actual);
    auto bad=document;++bad.world.construction_spent_total;--bad.world.treasury;
    sim::World::validate_original_production_snapshot(bad.world);
    refuses([&]{(void)f.restore(bad);});refuses([&]{save::write_save(f.target,bad,f.root.data,f.policy->building_mask());});
    auto duplicate=pristine;const auto marker=duplicate.find("\"services\": {");check(marker!=std::string::npos,"Services object absent");
    duplicate.insert(duplicate.find('{',marker)+1,"\"witnesses\":[],");f::write_text(path,duplicate);
    refuses([&]{(void)save::read_save(path);});
    check(f::read_text(f.target)==pristine,"Religion modelbound/duplicate refusal replaced good saved data");
}
void maximum_context_and_legacy_limits() {
    Fixture f(false,0,0,44,26);
    for (int i=0;i<33;++i) {const auto id=f::source(f.world,{0,0});f::buy(f.world,sim::demolish_building(id));}
    for (const int y:{1,5}) for (int x=1;x<41;x+=4) (void)f.buy(sim::CommandType::PlaceHousehold,{x,y},false);
    for (const int x:{1,5,9,13}) (void)f.buy(sim::CommandType::PlacePottery,{x,9});
    for (const int x:{17,21}) (void)f.buy(sim::CommandType::PlaceWarehouse,{x,9});
    for (const int x:{25,29,33,37}) (void)f.buy(sim::CommandType::PlaceMarket,{x,9});
    for (const int x:{1,9}) {
        const auto id=f.buy(sim::CommandType::PlaceOriginalHempFarm,{x,13});
        sim::Command field{sim::CommandType::PlaceOriginalHempField,{x+2,13}};field.building_id=id;f::buy(f.world,field);
    }
    for (const int x:{17,25}) {
        const auto id=f.buy(sim::CommandType::PlaceOriginalFoodFarm,{x,13});
        sim::Command field{x==17 ? sim::CommandType::PlaceOriginalWheatField:sim::CommandType::PlaceOriginalMilletField,{x+3,13}};
        field.building_id=id;f::buy(f.world,field);
    }
    const std::array commands{sim::CommandType::PlaceHealthPost,sim::CommandType::PlaceServicePost,sim::CommandType::PlaceOriginalAncestorShrine,
        sim::CommandType::PlaceOriginalDaoistShrine,sim::CommandType::PlaceOriginalBuddhistShrine,sim::CommandType::PlaceOriginalConfucianAcademy};
    for (std::size_t i=0;i<commands.size();++i) for (int j=0;j<2;++j)
        (void)f.buy(commands[i],{1+4*(2*static_cast<int>(i%3)+j),i<3 ? 17:21});
    for (const int x:{29,33}) (void)f.buy(sim::CommandType::PlaceOriginalWaterWell,{x,17});
    check(f.world.buildings().size()==52 && f.world.couriers().size()==34 && f.world.next_building_id()==86 &&
          f.world.next_courier_id()==35 && f.world.original_production_state()->services.providers.size()==12 &&
          f.world.original_influence_state()->removed_sources.size()==33 && f.world.total_population()==0,
          "Actual52/34 context or contiguous paid85-purchase history did not match its explicit limits");
    const auto pristine=f.roundtrip(f.world);const auto good=json::parse(pristine);const auto path=f.root.path/"bounds-religion.json";
    for (const char* key:{"buildings","couriers"}) {
        auto bad=good;bad["world"][key].push_back(bad["world"][key][0]);f::write_text(path,bad.dump());
        refuses([&]{(void)save::read_save(path);});
    }
    auto legacy=good;legacy["schema_version"]=22;legacy["rules"]["version"]=6;
    for (const char* key:{"original_production","original_economy","original_influence","original_goods"}) legacy["world"].erase(key);
    const auto legacy_reject=[&](json bad) {
        f::write_text(path,bad.dump());bool exact=false;
        try {(void)save::read_save(path);} catch (const std::exception& error) {exact=std::string(error.what())=="invalid scalable City entity array length";}
        check(exact,"Frozen schema22 inherited active52/34 entity syntax bounds");++rejected;
    };
    auto too_many_bodies=legacy;too_many_bodies["world"]["couriers"].erase(too_many_bodies["world"]["couriers"].begin()+26,too_many_bodies["world"]["couriers"].end());
    legacy_reject(too_many_bodies);
    auto too_many_actors=legacy;too_many_actors["world"]["buildings"].erase(too_many_actors["world"]["buildings"].begin()+46,too_many_actors["world"]["buildings"].end());
    legacy_reject(too_many_actors);
    auto old=f.world.snapshot();old.rule_version=6;
    old.original_production.reset();old.original_goods.reset();old.original_influence.reset();old.original_economy.reset();
    const auto old_restore_reject=[&](const sim::WorldSnapshot& state) {
        bool exact=false;
        try {(void)sim::World::restore(state,f.policy);} catch (const std::exception& error) {exact=std::string(error.what())=="invalid City-v10 entity counts or next IDs";}
        check(exact,"Frozen rule6 restore inherited active52/34 entity limits");++rejected;
    };
    auto old_bodies=old;old_bodies.couriers.resize(26);old_restore_reject(old_bodies);
    auto old_actors=old;old_actors.buildings.resize(46);old_restore_reject(old_actors);
    check(f::read_text(f.target)==pristine,"Active/frozen bound refusals changed protected good data");
}
void academy_shortest_body_route() {
    Fixture f(false,0,0);
    for (const sim::Cell cell:std::array{sim::Cell{3,5},sim::Cell{4,5},sim::Cell{3,6},sim::Cell{3,7},sim::Cell{3,8},sim::Cell{4,8}})
        f::buy(f.world,{sim::CommandType::PlaceRoad,cell});
    const auto target=f.buy(sim::CommandType::PlaceOriginalElitePlot,{5,5},false);
    (void)f.buy(sim::CommandType::PlaceHousehold,{5,9},false);
    const auto academy=f.buy(sim::CommandType::PlaceOriginalConfucianAcademy,{0,5});
    f.confucian=academy;
    oe_production_fixture::PaperSupplyLayout layout;layout.farm={10,12};layout.field={12,12};layout.paper={15,12};layout.warehouse={18,12};
    for (int x=3;x<23;++x) layout.roads.push_back({x,11});
    for (int y=9;y<11;++y) layout.roads.push_back({3,y});
    for (int x=0;x<23;++x) layout.roads.push_back({x,2});
    for (int y=3;y<11;++y) layout.roads.push_back({22,y});
    for (const int x:{0,3,6,9,12}) layout.extra_homes.push_back({x,0});
    f.paper=oe_production_fixture::add_paper_supply(f.world,layout);
    until(f.world,[&]{return f.world.total_population()==47;});f.prepare_paper();
    f::buy(f.world,sim::set_building_operation(academy,true));f.world.tick();
    const auto& actor=f.world.couriers().front();
    check(actor.role==sim::CourierRole::OriginalConfucian && actor.phase==sim::CourierPhase::ToWarehouse && actor.target==target &&
          actor.path.size()==4 && actor.path.front()==sim::Cell{2,5},
          "Academy shared route did not depart its actual third-column body through two real road cells");
    f.roundtrip(f.world);f::buy(f.world,sim::set_building_operation(academy,false));
    until(f.world,[&]{return house(f.world,target).confucian>0;});
    const auto trip=f.world.original_production_state()->services.trips.front();
    check(trip.arrival_tick && *trip.arrival_tick-trip.dispatched_tick==14,
          "Actual3x3 Academy arrival did not take exactly three five-tick edges including dispatch progress");
    f.roundtrip(f.world);
    until(f.world,[&]{return f.world.couriers().front().phase==sim::CourierPhase::Returning && f.world.couriers().front().edge_progress>0;});
    f.roundtrip(f.world);
    until(f.world,[&]{return f.world.couriers().front().phase==sim::CourierPhase::IdleAtWorkshop;});
    f.roundtrip(f.world);
}
void academy_edges() {
    Fixture f(false,0,0);
    const auto academy=f.buy(sim::CommandType::PlaceOriginalConfucianAcademy,{21,17});
    for (int y=17;y<20;++y) for (int x=21;x<24;++x)
        check(f.world.building_owner_at({x,y})==academy,"Legal corner Academy lost a3x3 body cell");
    f.roundtrip(f.world);const auto before=f.world.snapshot();
    const auto outside=f.world.execute({sim::CommandType::PlaceOriginalConfucianAcademy,{0,18}});
    check(!outside.accepted && !outside.changed && f.world.snapshot()==before,"Off-map3x3 Academy mutated authority");
}
}
int main() {
    try {Fixture f;const auto states=lifecycle(f);hostile(f,states);single_or(false);single_or(true);
        maximum_context_and_legacy_limits();academy_shortest_body_route();academy_edges();
        std::cout<<"Religion save PASS: paid215/217/219 actual arrivals,3x3 Academy, separate40/96 timers, OR/expiry, F5F9/20k, active52/34 versus frozen46/26 and "
                 <<rejected<<" hostile refusals\n";return 0;
    } catch (const std::exception& error) {std::cerr<<"Original religion save FAIL: "<<error.what()<<'\n';return 1;}
}
