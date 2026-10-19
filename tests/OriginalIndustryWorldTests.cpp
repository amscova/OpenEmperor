#include "OriginalProductionFixture.h"
#include "simulation/original/HouseEvolution.h"
#include "simulation/original/OriginalScheduler.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <string_view>
#include <tuple>

namespace {
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
using f::check;
const orig::HouseState& home(const sim::World& w,sim::BuildingId id){for(const auto& h:w.original_economy_state()->houses)if(h.building_id==id)return h.house;throw std::runtime_error("Missing paid industry House");}
std::uint64_t aux_received(const sim::World& w,sim::BuildingId id,unsigned material){for(const auto& row:w.original_production_state()->industry.auxiliary)if(row.building_id==id)return row.received[material];return 0;}
int aux(const sim::World& w,sim::BuildingId id,unsigned material){for(const auto& row:w.original_production_state()->industry.auxiliary)if(row.building_id==id)return row.amounts[material];return 0;}
const sim::OriginalIndustrySource& source(const sim::World& w,sim::BuildingId id){for(const auto& row:w.original_production_state()->industry.sources)if(row.building_id==id)return row;throw std::runtime_error("Missing paid industry source");}
sim::BuildingId buy(sim::World& w,sim::CommandType type,sim::Cell cell){f::buy(w,{type,cell});return *w.building_owner_at(cell);}
void valid(const sim::World& w){check(w.production_balance_valid()&&w.population_valid()&&w.city_economy_valid()&&w.navigation_valid(),"Industry runtime conservation/reachability failed");for(unsigned g=0;g<6;++g){const auto b=w.original_goods_balance(static_cast<orig::Goods>(g));check(b.supported&&b.conserved&&!b.totals.imported&&!b.totals.demolished,"Industry six-good mass failed");}for(unsigned a=0;a<5;++a)check(w.original_auxiliary_balance(static_cast<sim::OriginalAuxMaterial>(a)).conserved,"Auxiliary material mass failed");}
template<class P>void until(sim::World& w,P done,const char* why,int limit=30000){for(int t=0;t<limit&&!done();++t){w.tick();valid(w);}check(done(),why);}
struct Fixture{f::Root root;std::shared_ptr<const sim::MapPermissions> policy=f::permissions();std::shared_ptr<const orig::OriginalRulesModel> model;
    explicit Fixture(const f::fs::path& real={},bool authored_needs=false){auto text=real.empty()?oe_production_fixture::authored_model_text(7):f::read_text(real/"Model/EmperorBuildingModels.txt");
        if(authored_needs)for(const unsigned field:{11U,12U,13U}){const std::string marker="3: Plain Cottage,{,";auto at=text.find(marker);check(at!=std::string::npos,"Copied authored Plain row missing");at+=marker.size();for(unsigned i=0;i<field;++i){at=text.find(',',at)+1;}const auto end=text.find(',',at);check(text.substr(at,end-at)=="0","Original Plain luxury requirement unexpectedly nonzero");text.replace(at,end-at,"1");}
        f::write_text(root.data/"Model/EmperorBuildingModels.txt",text);model=f::save::prepare_original_production_rules_model(root.data);}
    sim::World world()const{return sim::World::create_original_production_development(policy,model);}
};
struct Scene{sim::BuildingId first{},tea{},lacquer{},silkworm{},logging{},smelter{},clay{},bronze{},lacquerware{},paper{},weaver{},farm{},field{},warehouse{},other_warehouse{},food{},market{},academy{},well{},ancestor{};std::vector<sim::BuildingId> plants;};
Scene scene(sim::World& w){
    for(const int y:{4,7,11,16})for(int x=0;x<24;++x)f::buy(w,{sim::CommandType::PlaceRoad,{x,y}});
    for(const int y:{5,6,8,9,10,12,13,14,15})f::buy(w,{sim::CommandType::PlaceRoad,{23,y}});
    f::buy(w,{sim::CommandType::PlaceRoad,{14,12}});
    Scene s;for(const int y:{1,5})for(int x=0;x<24;x+=3){const auto id=buy(w,x==0&&y==1 ? sim::CommandType::PlaceOriginalElitePlot:sim::CommandType::PlaceHousehold,y==1 ? oe_production_fixture::elite_row_cell(x/3):sim::Cell{x,y});if(x==0&&y==1)s.first=id;}
    for(int x=0;x<12;x+=3)(void)buy(w,sim::CommandType::PlaceHousehold,{x,9});
    s.tea=buy(w,sim::CommandType::PlaceOriginalTeaCuringShed,{0,12});s.lacquer=buy(w,sim::CommandType::PlaceOriginalLacquerRefinery,{4,12});s.silkworm=buy(w,sim::CommandType::PlaceOriginalSilkwormShed,{8,12});
    for(const auto& [command,id,cell]:std::array{std::tuple{sim::CommandType::PlaceOriginalTeaBush,s.tea,sim::Cell{2,12}},std::tuple{sim::CommandType::PlaceOriginalLacquerTree,s.lacquer,sim::Cell{6,12}},std::tuple{sim::CommandType::PlaceOriginalMulberryTree,s.silkworm,sim::Cell{10,12}}}){sim::Command plant{command,cell};plant.building_id=id;f::buy(w,plant);s.plants.push_back(*w.building_owner_at(cell));}
    s.logging=buy(w,sim::CommandType::PlaceOriginalLoggingShed,{12,12});s.smelter=buy(w,sim::CommandType::PlaceOriginalBronzeSmelter,{16,12});s.clay=buy(w,sim::CommandType::PlaceClaySource,{20,12});s.other_warehouse=buy(w,sim::CommandType::PlaceWarehouse,{14,13});
    s.bronze=buy(w,sim::CommandType::PlaceOriginalBronzewareMaker,{0,17});s.ancestor=buy(w,sim::CommandType::PlaceOriginalAncestorShrine,{2,17});s.lacquerware=buy(w,sim::CommandType::PlaceOriginalLacquerwareMaker,{4,17});s.paper=buy(w,sim::CommandType::PlaceOriginalPaperMaker,{8,17});s.weaver=buy(w,sim::CommandType::PlaceOriginalWeaver,{12,17});s.farm=buy(w,sim::CommandType::PlaceOriginalHempFarm,{16,17});
    sim::Command field{sim::CommandType::PlaceOriginalHempField,{18,17}};field.building_id=s.farm;f::buy(w,field);s.field=*w.building_owner_at({18,17});s.warehouse=buy(w,sim::CommandType::PlaceWarehouse,{20,17});
    s.food=buy(w,sim::CommandType::PlaceOriginalFoodFarm,{12,8});field={sim::CommandType::PlaceOriginalWheatField,{15,8}};field.building_id=s.food;f::buy(w,field);s.market=buy(w,sim::CommandType::PlaceMarket,{15,10});s.academy=buy(w,sim::CommandType::PlaceOriginalConfucianAcademy,{18,8});s.well=buy(w,sim::CommandType::PlaceOriginalWaterWell,{21,9});
    const bool real=w.original_rules_model()->house_rules().value(orig::ModelHouse::Shelter,orig::HouseField::Capacity)==7&&w.workforce_required(s.food)==22;
    check(w.buildings().size()==43&&w.couriers().size()==21&&w.snapshot().roads.size()==106&&!w.total_population(),"Paid industry scene geometry/actors/zero population differs");
    check(w.construction_spent_total()==static_cast<std::uint64_t>(real?3157:3186)&&w.treasury()==12000-static_cast<std::int64_t>(w.construction_spent_total()),"Real/authored industry construction bill differs");
    return s;
}
void bootstrap(sim::World& w,const Scene& s){int full_workers=0;for(const auto& b:w.buildings())full_workers+=w.workforce_required(b.id);for(const auto id:{s.paper,s.academy,s.food,s.ancestor})f::buy(w,sim::set_building_operation(id,false));
    until(w,[&]{return w.workforce_supply()>=full_workers;},"Paid Water/Hut bootstrap did not yield genuine workers");
    for(const auto id:{s.paper,s.academy,s.food,s.ancestor})f::buy(w,sim::set_building_operation(id,true));
}
void paid_chain_and_paper(const f::fs::path& real){Fixture f(real);auto w=f.world();const auto s=scene(w);
    const auto unavailable=w.original_service_availability(s.first,orig::ServiceRole::Confucian);check(!unavailable.available_source_count&&!aux(w,s.academy,4),"Empty Academy dispatched without actual Paper");bootstrap(w,s);
    const auto empty_academy=w.original_service_availability(s.first,orig::ServiceRole::Confucian);
    check(w.building(s.academy).operating_enabled&&w.workers_assigned(s.academy)==w.workforce_required(s.academy)&&
          !aux(w,s.academy,4)&&empty_academy.eligible_target&&!empty_academy.available_source_count&&
          !empty_academy.delivered_timer&&!empty_academy.travelling_to_target_count&&
          std::none_of(w.original_production_state()->services.trips.begin(),w.original_production_state()->services.trips.end(),
              [&](const auto& trip){return w.courier(trip.transfer.courier_id).owner==s.academy;}),
          "Fully staffed operating Academy bypassed actual zero-Paper dispatch gate");
    until(w,[&]{return source(w,s.bronze).progress==79&&w.building(s.bronze).input_clay>0&&aux(w,s.bronze,1)>0;},"Actual native Clay+Bronze recipe never reached progress79");
    const auto bronze_received=aux_received(w,s.bronze,1);const auto before=source(w,s.bronze);const auto clay=w.building(s.bronze).input_clay,bronze=aux(w,s.bronze,1);w.tick();valid(w);const auto after=source(w,s.bronze);
    check(after.completed==before.completed+1&&after.clay_used==before.clay_used+1&&after.aux_inputs_used[1]==before.aux_inputs_used[1]+1&&static_cast<std::uint64_t>(w.building(s.bronze).input_clay)+1==static_cast<std::uint64_t>(clay)+after.clay_received-before.clay_received&&
          static_cast<std::uint64_t>(aux(w,s.bronze,1))+1==static_cast<std::uint64_t>(bronze)+aux_received(w,s.bronze,1)-bronze_received,"Recipe inputs were not atomically debited exactly at successful completion");
    until(w,[&]{for(const auto& h:w.original_economy_state()->houses)if(h.house.tea&&h.house.silk&&h.house.bronzeware&&h.house.lacquerware)return true;return false;},"Four genuinely produced luxury goods never arrived at one House");
    until(w,[&]{return source(w,s.paper).completed>0&&home(w,s.first).confucian>0;},"Genuine Hemp→Paper→Academy did not enable paid Confucian visit");
    const auto& paper=source(w,s.paper);check(paper.goods_inputs_used[0]==paper.completed&&!paper.aux_inputs_used[0],"Paper consumed Wood or an unsupported Hemp mirror");
    const auto& smelter=source(w,s.smelter);check(smelter.aux_inputs_used[0]==smelter.completed,"Smelter skipped actual Wood");
    const auto& bw=source(w,s.bronze);check(bw.clay_used==bw.completed&&bw.aux_inputs_used[1]==bw.completed,"Bronzeware skipped native Clay/Bronze");
    const auto& lw=source(w,s.lacquerware);check(lw.aux_inputs_used[0]==lw.completed&&lw.aux_inputs_used[3]==lw.completed,"Lacquerware skipped real Wood/Lacquer");
    check(source(w,s.weaver).aux_inputs_used[2]==source(w,s.weaver).completed,"Weaver skipped real RawSilk");
    auto replay=sim::World::restore_original_production_development(w.snapshot(),f.policy,f.model);for(int i=0;i<20000;++i){w.tick();replay.tick();check(w.snapshot()==replay.snapshot(),"Industry full20k Snapshot replay diverged");}valid(w);
    for(const auto id:{s.tea,s.plants[0],s.warehouse,s.paper,s.farm,s.clay}){const auto before_demolition=w.snapshot();const auto r=w.execute(sim::demolish_building(id));check(!r.accepted&&w.snapshot()==before_demolition,"Industry/plant/material owner demolition was not atomic refusal");}
    const auto neutral=buy(w,sim::CommandType::PlaceOriginalInfluenceSource,{20,14});const auto before_neutral_demolition=w.construction_spent_total();f::buy(w,sim::demolish_building(neutral));check(w.construction_spent_total()==before_neutral_demolition,"Neutral Well removal during production invented refund or spending");valid(w);
}
void authored_plain_needs_and_original_both_stock_consumption(const f::fs::path& real){
    Fixture fixture(real,true);auto w=fixture.world();const auto s=scene(w);f::buy(w,sim::set_building_operation(s.tea,false));bootstrap(w,s);
    sim::BuildingId selected{};
    until(w,[&]{for(const auto& row:w.original_economy_state()->houses){const auto& h=row.house;if(w.building(row.building_id).kind==sim::Object::Household&&h.water&&h.food_quality>=20&&h.ancestor&&h.silk&&(h.bronzeware||h.lacquerware)){selected=row.building_id;return true;}}return false;},"Authored Common Plain negative never received actual other prerequisites");
    check(w.building(selected).kind==sim::Object::Household&&!home(w,selected).tea&&home(w,selected).type==orig::HouseType::Hut&&!orig::needs_met(fixture.model->house_rules(),orig::ModelHouse::PlainCottage,home(w,selected)),"Missing actual Tea passed explicitly authored PlainNeed1");
    f::buy(w,sim::set_building_operation(s.tea,true));
    until(w,[&]{for(const auto& row:w.original_production_state()->housing.houses)if(row.first_transitions[2]){
        const auto& input=row.first_transitions[2]->evaluation.input;
        if(input.tea>0&&input.silk>0&&(input.bronzeware>0||input.lacquerware>0)&&
           orig::needs_met(fixture.model->house_rules(),orig::ModelHouse::PlainCottage,input)){selected=row.building_id;return true;}}
        return false;},"Actual luxury delivery never admitted authored PlainNeed1");
    const auto& rows=w.original_production_state()->housing.houses;const auto proof=std::find_if(rows.begin(),rows.end(),[&](const auto& row){return row.building_id==selected;});
    check(proof!=rows.end()&&proof->first_transitions[2]&&proof->first_transitions[2]->evaluation.input.tea>0&&proof->first_transitions[2]->evaluation.input.silk>0&&
          (proof->first_transitions[2]->evaluation.input.bronzeware>0||proof->first_transitions[2]->evaluation.input.lacquerware>0)&&
          orig::needs_met(fixture.model->house_rules(),orig::ModelHouse::PlainCottage,proof->first_transitions[2]->evaluation.input),
          "First Plain proof lacks actual historical luxury goods before same-tick sink");
    const auto due=[&]{const auto d=orig::next_scheduler_dispatch(w.ticks());return d&&d->consumption;};
    until(w,[&]{if(!due())return false;
        for(const auto& candidate:w.original_economy_state()->houses){const auto& h=candidate.house;
            if(h.bronzeware<=0||h.lacquerware<=0||h.tea<=0||h.silk<=0)continue;
            const auto& histories=w.original_production_state()->housing.houses;
            const auto history_row=std::find_if(histories.begin(),histories.end(),[&](const auto& row){return row.building_id==candidate.building_id;});
            if(history_row==histories.end()||!history_row->first_transitions[2])continue;
            auto input=h;input.desirability=static_cast<std::int8_t>(w.original_influence_provenance(candidate.building_id).value);
            auto evolved=orig::evolve_step(fixture.model->house_rules(),input);
            if(evolved.type>orig::HouseType::PlainCottage)evolved.type=orig::HouseType::PlainCottage;
            if(evolved.type==orig::HouseType::PlainCottage){selected=candidate.building_id;return true;}
        }return false;
    },"Actual both OR stocks never coincided with a due consumption boundary after pure evolution");
    const auto& sink_rows=w.original_production_state()->housing.houses;
    const auto sink_row=std::find_if(sink_rows.begin(),sink_rows.end(),[&](const auto& row){return row.building_id==selected;});
    check(sink_row!=sink_rows.end()&&sink_row->first_transitions[2]&&
          orig::needs_met(fixture.model->house_rules(),orig::ModelHouse::PlainCottage,sink_row->first_transitions[2]->evaluation.input),
          "Actual consumption target lacks its own genuine strict first-Plain proof");
    const auto first_plain_input=sink_row->first_transitions[2]->evaluation.input;
    const auto history=[&]{for(const auto& row:w.original_production_state()->house_consumption)if(row.building_id==selected)return row.consumed;throw std::runtime_error("Missing actual House sink history");};
    auto bronze_only=first_plain_input,lacquer_only=first_plain_input,neither=first_plain_input;
    bronze_only.bronzeware=home(w,selected).bronzeware;bronze_only.lacquerware=0;
    lacquer_only.lacquerware=home(w,selected).lacquerware;lacquer_only.bronzeware=0;
    neither.bronzeware=neither.lacquerware=0;
    check(orig::needs_met(fixture.model->house_rules(),orig::ModelHouse::PlainCottage,bronze_only)&&
          orig::needs_met(fixture.model->house_rules(),orig::ModelHouse::PlainCottage,lacquer_only)&&
          !orig::needs_met(fixture.model->house_rules(),orig::ModelHouse::PlainCottage,neither),
          "Pure authored Need1 counterexamples derived from actual House do not preserve original OR predicate");
    const auto before=history();w.tick();const auto after=history();check(after[4]==before[4]+1&&after[5]==before[5]+1&&after[2]==before[2]+1&&after[3]==before[3]+1,
        "Original Bronze OR Lacquer admission was confused with independent both-positive-stock consumption");valid(w);
}
void plant_worker_route_and_pause(){Fixture f;auto w=f.world();const auto tea=buy(w,sim::CommandType::PlaceOriginalTeaCuringShed,{4,9});
    for(const auto command:std::array{sim::Command{sim::CommandType::PlaceOriginalTeaBush,{6,9}},sim::Command{sim::CommandType::PlaceOriginalLacquerTree,{6,9}}}){auto c=command;c.building_id=command.type==sim::CommandType::PlaceOriginalTeaBush?static_cast<sim::BuildingId>(0):tea;const auto old=w.snapshot();check(!w.execute(c).accepted&&old==w.snapshot(),"Unbound/wrong plant was not refused atomically");}
    sim::Command plant{sim::CommandType::PlaceOriginalTeaBush,{6,9}};plant.building_id=tea;f::buy(w,plant);const auto before=w.snapshot();plant.cell={6,10};check(!w.execute(plant).accepted&&w.snapshot()==before,"Second paid plant bypassed one-per-source bound");
    for(int t=0;t<400;++t)w.tick();check(!source(w,tea).completed&&!w.total_population(),"Unstaffed/noRoad orchard generated goods");
    for(int x=0;x<24;++x)f::buy(w,{sim::CommandType::PlaceRoad,{x,8}});for(int x=0;x<6;x+=3)(void)buy(w,sim::CommandType::PlaceHousehold,{x,6});const auto wh=buy(w,sim::CommandType::PlaceWarehouse,{12,9});(void)wh;
    until(w,[&]{for(const auto& c:w.couriers())if(c.owner==tea&&w.original_courier_cargo(c.id)>0&&c.edge_progress>0)return true;return false;},"Paid orchard never loaded actual moving Tea");
    sim::CourierId courier{};for(const auto& c:w.couriers())if(c.owner==tea)courier=c.id;const auto edge=w.courier(courier).edge_progress;f::buy(w,sim::set_building_operation(tea,false));check(w.courier(courier).edge_progress==edge,"Pause rewound a begun industry edge");
    until(w,[&]{for(const auto& r:w.original_goods_state()->reservations)if(r.source==tea&&r.phase==sim::OriginalGoodsTransferPhase::Returned)return r.returned>0;return false;},"Actual loaded Tea did not return to retained source capacity");
    const auto completed=source(w,tea).completed;for(int t=0;t<160;++t)w.tick();check(source(w,tea).completed==completed,"Paused source produced despite operation gate");valid(w);
    f::buy(w,sim::set_building_operation(tea,true));until(w,[&]{return w.building_on_fire(tea);},"Actual paid Tea source never became fire eligible");
    const auto burned=source(w,tea).completed;for(int t=0;t<80;++t)w.tick();check(source(w,tea).completed==burned,"Burning orchard completed production");valid(w);
    for(const bool missing_plant:{false,true}){auto denied=f.world();for(int x=0;x<24;++x)f::buy(denied,{sim::CommandType::PlaceRoad,{x,8}});
        for(int x=0;x<6;x+=3)(void)buy(denied,sim::CommandType::PlaceHousehold,{x,6});
        const sim::Cell origin{4,missing_plant?9:13};const auto owner=buy(denied,sim::CommandType::PlaceOriginalTeaCuringShed,origin);
        if(!missing_plant){sim::Command plant_command{sim::CommandType::PlaceOriginalTeaBush,{6,13}};plant_command.building_id=owner;f::buy(denied,plant_command);}
        (void)buy(denied,sim::CommandType::PlaceWarehouse,{12,9});for(int t=0;t<1400;++t)denied.tick();
        check(denied.workers_assigned(owner)==12&&!source(denied,owner).completed&&!source(denied,owner).progress,
              "Fully staffed orchard bypassed actual road/paid plant gate");valid(denied);
    }
}
}
int main(int argc,char** argv){try{check(argc==1||(argc==3&&std::string_view(argv[1])=="--real"),"Usage: original-industry-world [--real DATA]");const auto real=argc==3?f::fs::canonical(argv[2]):f::fs::path{};plant_worker_route_and_pause();paid_chain_and_paper(real);authored_plain_needs_and_original_both_stock_consumption(real);std::cout<<"Paid luxury/Aux/native Clay/Hemp→Paper chains and actual20k replay PASS; managed resource timing is authored\n";return 0;}catch(const std::exception& e){std::cerr<<"Original Industry World FAIL: "<<e.what()<<'\n';return 1;}}
