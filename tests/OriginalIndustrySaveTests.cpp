#include "OriginalProductionFixture.h"

#include <nlohmann/json.hpp>
#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <string>
#include <tuple>
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
    throw std::runtime_error("Malformed paid industry authority was admitted");
}
template<class P> void until(sim::World& world,P predicate,int limit=40000) {
    for (int i=0;i<limit && !predicate();++i) world.tick();
    check(predicate(),"Paid industry did not reach its required actual state");
}
const sim::OriginalIndustrySource& source(const sim::World& world,sim::BuildingId id) {
    const auto& rows=world.original_production_state()->industry.sources;
    const auto found=std::find_if(rows.begin(),rows.end(),[&](const auto& row){return row.building_id==id;});
    check(found!=rows.end(),"Paid industry source disappeared");return *found;
}
struct Fixture {
    f::Root root;
    std::shared_ptr<const sim::MapPermissions> policy;
    std::shared_ptr<const orig::OriginalRulesModel> model;
    sim::World world;
    f::fs::path target=root.path/"industry-schema26.json";
    std::array<sim::BuildingId,9> producers{};
    sim::BuildingId academy{},elite{};
    explicit Fixture(bool populate=true,int width=64,int height=36):
        policy(std::make_shared<const sim::MapPermissions>(width,height,3,
            std::vector<sim::MapCellPermission>(static_cast<std::size_t>(width*height),
                {true,true,false,0,sim::BuildBlocker::None,sim::BuildBlocker::None}),std::vector<sim::FixedGatePassage>{})),
        model([&] {
            // Explicit authored Shelter37/Need1 inputs; every worker still
            // arrives through ordinary paid Houses, with no snapshot grants.
            oe_production_fixture::write_model(root.data,37,0,0,0,0,0,0,14,22,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1);
            return save::prepare_original_production_rules_model(root.data);
        }()),world(sim::World::create_original_production_development(policy,model)) {
        if (!populate) return;
        for (int x=0;x<64;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,4}});
        for (int x=0;x<60;x+=3) {
            const auto id=buy(x==0 ? sim::CommandType::PlaceOriginalElitePlot:sim::CommandType::PlaceHousehold,
                              x==0 ? sim::Cell{0,0}:sim::Cell{x+1,2});
            if (!x) elite=id;
        }
        (void)buy(sim::CommandType::PlaceClaySource,{0,5});
        (void)buy(sim::CommandType::PlacePottery,{3,5});
        (void)buy(sim::CommandType::PlaceWarehouse,{6,5});
        (void)buy(sim::CommandType::PlaceWarehouse,{9,5});
        (void)buy(sim::CommandType::PlaceMarket,{12,5});
        const auto farm=buy(sim::CommandType::PlaceOriginalHempFarm,{15,5});
        sim::Command field{sim::CommandType::PlaceOriginalHempField,{17,5}};field.building_id=farm;f::buy(world,field);
        constexpr std::array commands{sim::CommandType::PlaceOriginalLoggingShed,sim::CommandType::PlaceOriginalBronzeSmelter,
            sim::CommandType::PlaceOriginalBronzewareMaker,sim::CommandType::PlaceOriginalLacquerwareMaker,
            sim::CommandType::PlaceOriginalPaperMaker,sim::CommandType::PlaceOriginalWeaver,
            sim::CommandType::PlaceOriginalTeaCuringShed,sim::CommandType::PlaceOriginalLacquerRefinery,
            sim::CommandType::PlaceOriginalSilkwormShed};
        constexpr std::array positions{20,23,27,30,33,36,39,43,47};
        for (std::size_t i=0;i<commands.size();++i) producers[i]=buy(commands[i],{positions[i],5});
        for (const auto& [command,index,x]:std::array{
                std::tuple{sim::CommandType::PlaceOriginalTeaBush,6U,41},
                std::tuple{sim::CommandType::PlaceOriginalLacquerTree,7U,45},
                std::tuple{sim::CommandType::PlaceOriginalMulberryTree,8U,49}}) {
            sim::Command plant{command,{x,5}};plant.building_id=producers[index];f::buy(world,plant);
        }
        academy=buy(sim::CommandType::PlaceOriginalConfucianAcademy,{51,5});
        check(!world.total_population() && world.treasury()+static_cast<std::int64_t>(world.construction_spent_total())==12000 &&
              world.original_production_state()->industry.sources.size()==9 && world.original_production_state()->industry.managed_ore.size()==1,
              "Paid industry construction granted population, materials or unsupported producers");
        for (const auto& row:world.original_production_state()->industry.sources)
            check(!row.completed && !row.progress && !row.clay_received && !row.clay_used,"Industry purchase granted recipe history");
        for (const auto& row:world.original_production_state()->industry.auxiliary)
            check(row.amounts==sim::OriginalAuxAmounts{} && row.received==sim::OriginalAuxCounters{},"Industry purchase granted materials");
        const auto before=world.snapshot();
        for (const auto id:producers) {
            const auto result=world.execute(sim::demolish_building(id));
            check(!result.accepted && !result.changed && world.snapshot()==before,"New producer demolition mutated paid history");
        }
    }
    sim::BuildingId buy(sim::CommandType command,sim::Cell cell) {
        f::buy(world,{command,cell});return *world.building_owner_at(cell);
    }
    save::SaveDocument document(const sim::World& value) const {
        return save::make_document(root.data,f::map_relative,policy->building_mask(),value);
    }
    sim::World restore(const save::SaveDocument& value) const {
        return save::restore_original_production_development(value,root.data,policy->building_mask(),policy);
    }
    std::string roundtrip(const sim::World& value) const {
        const auto before=value.snapshot();const auto doc=document(value);
        save::write_save(target,doc,root.data,policy->building_mask());const auto bytes=f::read_text(target);
        const auto parsed=save::read_save(target);f::same_document(doc,parsed);
        check(parsed.source_schema_version==26 && parsed.world.original_production->development_revision==26,
              "Industry save lost explicit revision26 identity");
        const auto restored=restore(parsed);check(restored.snapshot()==before,"Industry F5/F9 changed full authority");
        save::write_save(target,document(restored),root.data,policy->building_mask());
        check(f::read_text(target)==bytes && value.snapshot()==before,"Industry canonical rewrite mutated source authority");return bytes;
    }
    void replay(const sim::World& value) const {
        roundtrip(value);auto expected=restore(document(value));auto actual=restore(save::read_save(target));
        for (int tick=0;tick<20000;++tick) {
            if (tick==10000) {f::buy(expected,{sim::CommandType::PlaceRoad,{63,1}});f::buy(actual,{sim::CommandType::PlaceRoad,{63,1}});}
            expected.tick();actual.tick();
        }
        check(expected.snapshot()==actual.snapshot(),"Industry20k continuation with paid road diverged");
    }
};
struct States {sim::WorldSnapshot clay,auxiliary,recipe,positive;};
States lifecycle(Fixture& f) {
    f.roundtrip(f.world);
    until(f.world,[&]{return std::any_of(f.world.couriers().begin(),f.world.couriers().end(),[](const auto& c){return c.role==sim::CourierRole::Clay && c.cargo>0 && c.edge_progress>0;});});
    const auto clay=f.world.snapshot();f.roundtrip(f.world);
    check(f.world.original_production_state()->industry.clay_trips.size()==1,"Actual native Clay trip lost its all-target sequence metadata");
    until(f.world,[&]{return std::any_of(f.world.original_production_state()->industry.aux_trips.begin(),f.world.original_production_state()->industry.aux_trips.end(),[&](const auto& trip){
        return trip.phase==sim::OriginalGoodsTransferPhase::Loaded && f.world.courier(trip.transfer.courier_id).edge_progress>0;
    });});
    const auto auxiliary=f.world.snapshot();f.replay(f.world);
    until(f.world,[&]{return source(f.world,f.producers[4]).progress==79;});
    const auto recipe=f.world.snapshot();f.roundtrip(f.world);
    auto next=f.restore(save::read_save(f.target));f.world.tick();next.tick();
    check(next.snapshot()==f.world.snapshot() && source(f.world,f.producers[4]).completed==recipe.original_production->industry.sources[4].completed+1,
          "Restored final Paper recipe tick double-booked or lost actual inputs/output");
    until(f.world,[&] {
        const auto& state=*f.world.original_goods_state();
        return std::all_of(state.totals.begin()+2,state.totals.end(),[](const auto& totals){return totals.produced>0 && totals.consumed>0;}) &&
            std::any_of(f.world.original_production_state()->services.witnesses.begin(),f.world.original_production_state()->services.witnesses.end(),[&](const auto& w){return w.role==orig::ServiceRole::Confucian && w.building_id==f.elite;});
    });
    const auto& industry=f.world.original_production_state()->industry;
    check(source(f.world,f.producers[2]).clay_received>0 && source(f.world,f.producers[2]).clay_used>0 && source(f.world,f.producers[2]).last_clay_arrival,
          "Actual Bronzeware lacks native Clay arrival and recipe sink");
    check(source(f.world,f.producers[3]).aux_inputs_used[0]>0 && source(f.world,f.producers[3]).aux_inputs_used[3]>0,
          "Lacquerware did not consume both actual Wood and Lacquer");
    check(source(f.world,f.producers[4]).goods_inputs_used[0]==source(f.world,f.producers[4]).completed &&
          source(f.world,f.producers[1]).aux_inputs_used[0]==source(f.world,f.producers[1]).completed &&
          industry.managed_ore.front().used==source(f.world,f.producers[1]).completed,
          "Hemp/Paper or Wood/Ore/Bronze recipe histories disagree");
    std::uint64_t house_hemp=0;for (const auto& row:f.world.original_production_state()->house_consumption) house_hemp+=row.consumed[0];
    check(f.world.original_goods_state()->totals[0].consumed==house_hemp+source(f.world,f.producers[4]).goods_inputs_used[0],
          "Industrial Hemp sink was lost or assigned to household consumption");
    const auto paper=std::find_if(industry.auxiliary.begin(),industry.auxiliary.end(),[&](const auto& row){return row.building_id==f.academy;});
    const auto trip=std::find_if(f.world.original_production_state()->services.trips.begin(),f.world.original_production_state()->services.trips.end(),[&](const auto& row){return f.world.courier(row.transfer.courier_id).owner==f.academy;});
    check(paper!=industry.auxiliary.end() && trip!=f.world.original_production_state()->services.trips.end() && paper->consumed[4]==trip->transfer.sequence,
          "Academy Paper sink is not exactly bound to its actual dispatch highwater");
    const auto positive=f.world.snapshot();f.roundtrip(f.world);
    // An ordinary paid neutral removal preserves all actual material mass.
    const auto neutral=f::source(f.world,{60,29});const auto goods=*f.world.original_goods_state();
    const auto materials=f.world.original_production_state()->industry;
    f::buy(f.world,sim::demolish_building(neutral));
    check(*f.world.original_goods_state()==goods && f.world.original_production_state()->industry==materials,
          "Allowed neutral demolition silently changed production mass/history");
    for (const auto id:f.producers) {
        const auto before=f.world.snapshot();const auto result=f.world.execute(sim::demolish_building(id));
        check(!result.accepted && !result.changed && f.world.snapshot()==before,"Stocked productive demolition was not atomic refusal");
    }
    f.roundtrip(f.world);return {clay,auxiliary,recipe,positive};
}
void hostile(Fixture& f,const States& states) {
    const auto world=sim::World::restore_original_production_development(states.positive,f.policy,f.model);
    const auto pristine=f.roundtrip(world);const auto good=json::parse(pristine);const auto bad_path=f.root.path/"bad-industry.json";
    const std::string base="/world/original_production/industry";
    const auto reject=[&](const json& bad) {
        f::write_text(bad_path,bad.dump());
        try {refuses([&]{(void)f.restore(save::read_save(bad_path));});}
        catch (const std::exception& error) {throw std::runtime_error(std::string(error.what())+": "+json::diff(good,bad).dump());}
        check(f::read_text(f.target)==pristine,"Malformed industry read replaced protected save");
    };
    std::vector<std::string> objects;
    const std::function<void(const json&,std::string)> walk=[&](const json& value,std::string pointer) {
        if (value.is_object()) {objects.push_back(pointer);for (const auto& [key,child]:value.items()) walk(child,pointer+"/"+key);}
        else if (value.is_array()) for (std::size_t i=0;i<value.size();++i) walk(value[i],pointer+"/"+std::to_string(i));
    };
    walk(good.at(json::json_pointer(base)),base);
    for (const auto& pointer:objects) {
        auto bad=good;bad.at(json::json_pointer(pointer))["unbacked_industry_authority"]=true;reject(bad);
        for (const auto& [key,unused]:good.at(json::json_pointer(pointer)).items()) {
            (void)unused;bad=good;bad.at(json::json_pointer(pointer)).erase(key);reject(bad);
        }
    }
    for (const auto& [key,limit]:std::array{std::pair{"sources",18U},std::pair{"managed_ore",2U},std::pair{"auxiliary",20U},std::pair{"aux_trips",12U},std::pair{"clay_trips",4U}}) {
        auto bad=good;auto& rows=bad.at(json::json_pointer(base+"/"+key));check(!rows.empty(),"Hostile industry collection has no actual row");
        while (rows.size()<=limit) rows.push_back(rows[0]);reject(bad);
        bad=good;bad.at(json::json_pointer(base+"/"+key)).push_back(bad.at(json::json_pointer(base+"/"+key))[0]);reject(bad);
    }
    for (const auto& pointer:{base+"/sources/0/building_id",base+"/sources/0/completed",base+"/sources/0/progress",
            base+"/managed_ore/0/extracted",base+"/auxiliary/0/building_id",base+"/aux_trips/0/transfer/courier_id",
            base+"/aux_trips/0/transfer/sequence",base+"/aux_trips/0/dispatched_tick",base+"/clay_trips/0/transfer/sequence"})
        for (const json& value:{json(-1),json(true),json(1.0),json("1"),json(UINT64_MAX)}) {
            auto bad=good;bad.at(json::json_pointer(pointer))=value;reject(bad);
        }
    for (const auto& [pointer,value]:std::vector<std::pair<std::string,json>>{
            {base+"/sources/0/plant_id",f.producers[0]},
            {base+"/sources/0/progress",80},{base+"/sources/4/goods_inputs_used/1",1},
            {base+"/sources/2/clay_used",0},{base+"/sources/2/last_clay_arrival",nullptr},
            {base+"/managed_ore/0/stock",9},{base+"/managed_ore/0/progress",80},
            {base+"/auxiliary/0/amounts/0",33},{base+"/aux_trips/0/material",5},
            {base+"/aux_trips/0/phase",0},{base+"/aux_trips/0/phase",4},{base+"/aux_trips/0/loaded",0},
            {base+"/clay_trips/0/loaded",0},{base+"/clay_trips/0/dispatched_tick",0},
            {"/world/original_production/development_revision",23},{"/world/original_production/development_revision",24},{"/world/original_production/development_revision",25},{"/world/original_production/development_revision",27}}) {
        auto bad=good;bad.at(json::json_pointer(pointer))=value;reject(bad);
    }
    for (const auto& pointer:{base+"/sources/0/goods_inputs_used",base+"/sources/0/aux_inputs_used",base+"/auxiliary/0/amounts",
            base+"/auxiliary/0/received",base+"/auxiliary/0/departed",base+"/auxiliary/0/returned",base+"/auxiliary/0/consumed"}) {
        for (const bool extend:{false,true}) {auto bad=good;auto& array=bad.at(json::json_pointer(pointer));if (extend) array.push_back(0);else array.erase(array.size()-1);reject(bad);}
    }
    {auto bad=good;bad["world"]["original_production"].erase("industry");reject(bad);}
    {auto bad=good;bad.at(json::json_pointer(base+"/sources")).erase(0);reject(bad);}
    const auto document=f.document(world);
    for (int fault=0;fault<8;++fault) {
        auto bad=document;auto& state=bad.world.original_production->industry;
        if (fault==0) state.sources[0].completed=UINT64_MAX;
        if (fault==1) ++state.sources[2].clay_used;
        if (fault==2) ++state.sources[4].goods_inputs_used[0];
        if (fault==3) ++state.managed_ore[0].used;
        if (fault==4) state.aux_trips[0].transfer.sequence=UINT64_MAX;
        if (fault==5) state.clay_trips[0].dispatched_tick=bad.world.ticks+1;
        if (fault==6) state.sources[0].plant_id=state.sources[1].building_id;
        if (fault==7) bad.world.original_production->development_revision=23;
        refuses([&]{(void)f.restore(bad);});refuses([&]{save::write_save(f.target,bad,f.root.data,f.policy->building_mask());});
        check(f::read_text(f.target)==pristine,"Invalid industry writer replaced protected good save");
    }
    auto wrong_cost=document;++wrong_cost.world.construction_spent_total;--wrong_cost.world.treasury;
    sim::World::validate_original_production_snapshot(wrong_cost.world);
    refuses([&]{(void)f.restore(wrong_cost);});refuses([&]{save::write_save(f.target,wrong_cost,f.root.data,f.policy->building_mask());});
    check(f::read_text(f.target)==pristine,"Wrong industry model price replaced protected save");
    auto duplicate=pristine;const auto at=duplicate.find("\"industry\": {");check(at!=std::string::npos,"Canonical industry object missing");
    duplicate.insert(duplicate.find('{',at)+1,"\"sources\":[],");f::write_text(bad_path,duplicate);refuses([&]{(void)save::read_save(bad_path);});
    auto loaded=sim::World::restore_original_production_development(states.auxiliary,f.policy,f.model);
    const auto original=f.document(loaded);auto bad=original;
    auto& trip=*std::find_if(bad.world.original_production->industry.aux_trips.begin(),bad.world.original_production->industry.aux_trips.end(),[](const auto& row){return row.phase==sim::OriginalGoodsTransferPhase::Loaded;});
    trip.arrival_tick=bad.world.ticks;trip.delivered=trip.loaded;trip.phase=sim::OriginalGoodsTransferPhase::Completed;
    refuses([&]{(void)f.restore(bad);});
}
void maximum_and_frozen_limits() {
    Fixture f(false,64,44);
    for (int i=0;i<33;++i) {const auto id=f::source(f.world,{0,0});f::buy(f.world,sim::demolish_building(id));}
    for (const int y:{1,5}) for (int x=1;x<41;x+=4) (void)f.buy(sim::CommandType::PlaceHousehold,{x,y});
    for (const int x:{1,5,9,13}) (void)f.buy(sim::CommandType::PlacePottery,{x,9});
    for (const int x:{17,21}) (void)f.buy(sim::CommandType::PlaceWarehouse,{x,9});
    for (const int x:{25,29,33,37}) (void)f.buy(sim::CommandType::PlaceMarket,{x,9});
    for (const int x:{1,9}) {const auto id=f.buy(sim::CommandType::PlaceOriginalHempFarm,{x,13});sim::Command field{sim::CommandType::PlaceOriginalHempField,{x+2,13}};field.building_id=id;f::buy(f.world,field);}
    for (const int x:{17,25}) {const auto id=f.buy(sim::CommandType::PlaceOriginalFoodFarm,{x,13});sim::Command field{x==17 ? sim::CommandType::PlaceOriginalWheatField:sim::CommandType::PlaceOriginalMilletField,{x+3,13}};field.building_id=id;f::buy(f.world,field);}
    constexpr std::array service_commands{sim::CommandType::PlaceHealthPost,sim::CommandType::PlaceServicePost,sim::CommandType::PlaceOriginalAncestorShrine,
        sim::CommandType::PlaceOriginalDaoistShrine,sim::CommandType::PlaceOriginalBuddhistShrine,sim::CommandType::PlaceOriginalConfucianAcademy,
        sim::CommandType::PlaceOriginalMusicSchool,sim::CommandType::PlaceOriginalAcrobatSchool,sim::CommandType::PlaceOriginalDramaSchool};
    for (std::size_t i=0;i<service_commands.size();++i) for (int j=0;j<2;++j)
        (void)f.buy(service_commands[i],{1+4*(2*static_cast<int>(i%3)+j),17+4*static_cast<int>(i/3)});
    for (const int x:{29,33}) (void)f.buy(sim::CommandType::PlaceOriginalWaterWell,{x,17});
    constexpr std::array industry_commands{sim::CommandType::PlaceOriginalLoggingShed,sim::CommandType::PlaceOriginalBronzeSmelter,
        sim::CommandType::PlaceOriginalBronzewareMaker,sim::CommandType::PlaceOriginalLacquerwareMaker,
        sim::CommandType::PlaceOriginalPaperMaker,sim::CommandType::PlaceOriginalWeaver,
        sim::CommandType::PlaceOriginalTeaCuringShed,sim::CommandType::PlaceOriginalLacquerRefinery,sim::CommandType::PlaceOriginalSilkwormShed};
    for (int copy=0;copy<2;++copy) for (std::size_t i=0;i<industry_commands.size();++i) {
        const int x=1+4*static_cast<int>(i),y=30+5*copy;
        const auto id=f.buy(industry_commands[i],{x,y});
        if (i>=6) {
            constexpr std::array plants{sim::CommandType::PlaceOriginalTeaBush,sim::CommandType::PlaceOriginalLacquerTree,sim::CommandType::PlaceOriginalMulberryTree};
            sim::Command plant{plants[i-6],{x+2,y}};plant.building_id=id;f::buy(f.world,plant);
        }
    }
    check(f.world.buildings().size()==82 && f.world.couriers().size()==58 && f.world.next_building_id()==116 && f.world.next_courier_id()==59 &&
          f.world.original_production_state()->industry.sources.size()==18 && f.world.original_production_state()->industry.auxiliary.size()==20 &&
          f.world.original_production_state()->services.providers.size()==18 && f.world.original_influence_state()->removed_sources.size()==33 && !f.world.total_population(),
          "Paid82/58 bounded context,20 material owners or contiguous115-purchase history differs");
    const auto pristine=f.roundtrip(f.world);const auto good=json::parse(pristine);const auto path=f.root.path/"industry-bounds.json";
    for (const char* key:{"buildings","couriers"}) {
        auto bad=good;bad["world"][key].push_back(bad["world"][key][0]);f::write_text(path,bad.dump());
        refuses([&]{(void)save::read_save(path);});
    }
    auto legacy=good;legacy["schema_version"]=22;legacy["rules"]["version"]=6;
    for (const char* key:{"original_production","original_economy","original_influence","original_goods"}) legacy["world"].erase(key);
    const auto frozen_parse=[&](const json& value) {
        f::write_text(path,value.dump());bool exact=false;
        try {(void)save::read_save(path);} catch (const std::exception& error) {exact=std::string(error.what())=="invalid scalable City entity array length";}
        check(exact,"Frozen schema22 accepted active82/58 entity syntax");++rejected;
    };
    auto too_many_bodies=legacy;too_many_bodies["world"]["couriers"].erase(too_many_bodies["world"]["couriers"].begin()+26,too_many_bodies["world"]["couriers"].end());frozen_parse(too_many_bodies);
    auto too_many_actors=legacy;too_many_actors["world"]["buildings"].erase(too_many_actors["world"]["buildings"].begin()+46,too_many_actors["world"]["buildings"].end());frozen_parse(too_many_actors);
    auto old=f.world.snapshot();old.rule_version=6;old.original_production.reset();old.original_goods.reset();old.original_influence.reset();old.original_economy.reset();
    const auto frozen_restore=[&](const sim::WorldSnapshot& value) {
        bool exact=false;try {(void)sim::World::restore(value,f.policy);} catch (const std::exception& error) {exact=std::string(error.what())=="invalid City-v10 entity counts or next IDs";}
        check(exact,"Frozen rule6 restore accepted active82/58 entity context");++rejected;
    };
    auto bodies=old;bodies.couriers.resize(26);frozen_restore(bodies);auto actors=old;actors.buildings.resize(46);frozen_restore(actors);
    check(f::read_text(f.target)==pristine,"Bound refusals replaced protected industry save");
}
}
int main() {
    try {
        Fixture f;const auto states=lifecycle(f);hostile(f,states);maximum_and_frozen_limits();
        std::cout<<"Industry save PASS: paid four-Goods recipes and native Clay, five materials, real Hemp/Paper/Academy sink, F5F9/final-recipe/20k and "<<rejected<<" hostile refusals\n";
        return 0;
    } catch (const std::exception& error) {std::cerr<<"Original industry save FAIL: "<<error.what()<<'\n';return 1;}
}
