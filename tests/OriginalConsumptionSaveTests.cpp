#include "OriginalProductionFixture.h"
#include "simulation/original/GoodsRules.h"
#include "simulation/original/OriginalScheduler.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <functional>
#include <iostream>

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
    throw std::runtime_error("Malformed consumption authority was admitted");
}
struct Fixture {
    f::Root root;
    std::shared_ptr<const sim::MapPermissions> policy=f::permissions();
    std::shared_ptr<const orig::OriginalRulesModel> model;
    sim::World world;
    f::fs::path target=root.path/"consumption.json";
    sim::BuildingId market{};
    Fixture(int hemp=3,int ceramics=4):model([&] {
        oe_production_fixture::write_model(root.data,7,hemp,ceramics);
        return save::prepare_original_production_rules_model(root.data);
    }()),world(sim::World::create_original_production_development(policy,model)) {
        for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,8}});
        for (int x=0;x<18;x+=3) f::buy(world,{sim::CommandType::PlaceHousehold,{x,6}});
        for (const auto& [kind,cell]:std::array{
                std::pair{sim::CommandType::PlaceClaySource,sim::Cell{0,9}},
                std::pair{sim::CommandType::PlacePottery,sim::Cell{8,9}},
                std::pair{sim::CommandType::PlaceWarehouse,sim::Cell{16,9}},
                std::pair{sim::CommandType::PlaceMarket,sim::Cell{21,9}},
                std::pair{sim::CommandType::PlaceOriginalHempFarm,sim::Cell{4,9}}}) f::buy(world,{kind,cell});
        sim::Command field{sim::CommandType::PlaceOriginalHempField,{6,9}};
        field.building_id=*world.building_owner_at({4,9});f::buy(world,field);
        market=*world.building_owner_at({21,9});
        check(world.construction_spent_total()==1162 && world.total_population()==0,
              "Consumption fixture did not pay for its initially empty complete chain");
    }
    save::SaveDocument document(const sim::World& value) const {
        return save::make_document(root.data,f::map_relative,policy->building_mask(),value);
    }
    sim::World restore(const save::SaveDocument& value) const {
        return save::restore_original_production_development(value,root.data,policy->building_mask(),policy);
    }
    std::string roundtrip(const sim::World& value) const {
        const auto before=value.snapshot();const auto d=document(value);
        save::write_save(target,d,root.data,policy->building_mask());const auto bytes=f::read_text(target);
        const auto parsed=save::read_save(target);f::same_document(d,parsed);
        check(parsed.source_schema_version==26 && parsed.world.original_production->development_revision==25,
              "Consumption save lost its active generation");
        auto copy=restore(parsed);check(copy.snapshot()==before,"F5/F9 changed consumption authority");
        save::write_save(target,document(copy),root.data,policy->building_mask());
        check(f::read_text(target)==bytes && value.snapshot()==before,"Canonical rewrite mutated save or World");
        return bytes;
    }
};
void conserved(const sim::World& world) {
    std::array<std::uint64_t,orig::goods_count> consumed{};
    const auto& audit=world.original_production_state()->house_consumption;
    const auto& houses=world.original_economy_state()->houses;
    check(audit.size()==houses.size(),"Consumption audit lost a paid House");
    for (std::size_t i=0;i<audit.size();++i) {
        check(audit[i].building_id==houses[i].building_id,"Consumption audit identity mismatch");
        for (std::size_t g=0;g<consumed.size();++g) consumed[g]+=audit[i].consumed[g];
    }
    for (std::size_t g=0;g<consumed.size();++g) {
        const auto b=world.original_goods_balance(static_cast<orig::Goods>(g));
        check(b.conserved && b.totals.consumed==consumed[g] &&
              b.totals.produced==b.stock+b.cargo+b.totals.consumed &&
              !b.totals.imported && !b.totals.demolished,"Consumption mass or House audit mismatch");
    }
}
std::array<int,2> house_stock(const sim::World& world) {
    std::array<int,2> totals{};
    for (const auto& h:world.original_economy_state()->houses) {totals[0]+=h.house.hemp;totals[1]+=h.house.ceramics;}
    return totals;
}
void empty_boundaries(Fixture& f) {
    std::vector<sim::BuildingId> paused;
    for (const auto& b:f.world.buildings()) if (b.kind!=sim::Object::Household && b.kind!=sim::Object::OriginalHempField)
        paused.push_back(b.id);
    // Commands atomically replace the World; retain IDs, never references into
    // its old Building vector across a command.
    for (const auto id:paused) f::buy(f.world,sim::set_building_operation(id,false));
    for (const auto tick:{39U,40U,41U,396U,397U,398U}) {
        while (f.world.ticks()<tick) f.world.tick();f.roundtrip(f.world);conserved(f.world);
        check(f.world.original_goods_balance(orig::Goods::Hemp).totals.consumed==0 &&
              f.world.original_goods_balance(orig::Goods::Ceramics).totals.consumed==0,
              "Empty or unserved House consumed a phantom unit");
    }
    for (const auto id:paused) f::buy(f.world,sim::set_building_operation(id,true));
}
sim::WorldSnapshot positive_boundary(Fixture& f,int step) {
    bool found=false;
    for (int i=0;i<30000 && !found;++i) {
        const auto next=orig::next_scheduler_dispatch(f.world.ticks());const auto stock=house_stock(f.world);
        found=next && next->consumption && next->position.step==step && stock[0]>0 && stock[1]>0;
        if (!found) f.world.tick();
    }
    check(found,"Actual deliveries did not reach a positive consumption boundary");
    // Ordinary pause returns the loaded distributor safely; no House arrival
    // can obscure the exact before/after subtraction at this boundary.
    f::buy(f.world,sim::set_building_operation(f.market,false));
    const auto before=f.world.snapshot();const auto bytes=f.roundtrip(f.world);const auto document=save::read_save(f.target);
    std::vector<orig::HouseState> evolved;
    for (const auto& home:before.original_economy->houses) {
        auto input=home.house;
        if (input.occupants>0) {
            input.desirability=static_cast<std::int8_t>(f.world.original_influence_provenance(home.building_id).value);
            input=orig::evolve_step(f.model->house_rules(),input);
        }
        evolved.push_back(input);
    }
    auto restored=f.restore(document);f.world.tick();restored.tick();
    check(restored.snapshot()==f.world.snapshot(),"F9 immediately before consumption duplicated or skipped a debit");
    bool under=false;std::array<int,2> delta{};
    for (std::size_t i=0;i<before.original_economy->houses.size();++i) {
        const auto expected=orig::consume_goods(f.model->house_rules(),evolved[i]);
        check(f.world.original_economy_state()->houses[i].house==expected.house,"World consumption differs from pure phase39 evolution followed by current-row consumption");
        for (std::size_t g=0;g<2;++g) {
            delta[g]+=expected.consumed[g];
            const auto old=orig::goods_stock(evolved[i],static_cast<orig::Goods>(g));
            under=under || (old>0 && old<f.model->house_rules().value(orig::house_row(evolved[i].type),orig::goods_field(static_cast<orig::Goods>(g))));
            check(f.world.original_production_state()->house_consumption[i].consumed[g]==
                  before.original_production->house_consumption[i].consumed[g]+static_cast<std::uint64_t>(expected.consumed[g]),
                  "Actual House consumption was not booked exactly once");
        }
    }
    check(delta[0]>0 && delta[1]>0 && under,"Boundary lacked actual positive Hemp/Ceramics under-supply");
    conserved(f.world);f.roundtrip(f.world);
    const auto accounted=f.world.original_production_state()->house_consumption;
    f.world.tick();restored.tick();check(restored.snapshot()==f.world.snapshot() &&
        accounted==f.world.original_production_state()->house_consumption,"Following phase charged consumption again");
    f.roundtrip(f.world);
    auto expected=sim::World::restore_original_production_development(before,f.policy,f.model),replay=f.restore(document);
    for (int i=0;i<20000;++i) {expected.tick();replay.tick();}
    check(expected.snapshot()==replay.snapshot(),"Save-to-next-cycle 20k deterministic replay diverged");conserved(replay);
    check(f::read_text(f.target)!=bytes,"Boundary did not change canonical save authority");
    f::buy(f.world,sim::set_building_operation(f.market,true));
    return before;
}
void adversarial(Fixture& f,const sim::WorldSnapshot& actual) {
    auto world=sim::World::restore_original_production_development(actual,f.policy,f.model);
    const auto pristine=f.roundtrip(world);const auto good=json::parse(pristine);const auto hostile=f.root.path/"bad-consumption.json";
    const auto reject=[&](const json& j) {
        f::write_text(hostile,j.dump());
        try {refuses([&]{(void)f.restore(save::read_save(hostile));});}
        catch (const std::exception& e) {throw std::runtime_error(std::string(e.what())+": "+json::diff(good,j).dump());}
        check(f::read_text(f.target)==pristine,"Invalid consumption input replaced the good save");
    };
    const std::string base="/world/original_production/house_consumption";
    for (const auto& [path,value]:std::vector<std::pair<std::string,json>>{
            {base,nullptr},{base,json::array()},{base+"/0/building_id",0},{base+"/0/building_id",UINT32_MAX},
            {base+"/0/consumed",json::array({0,0})},{base+"/0/consumed/0",-1},
            {base+"/0/consumed/0",UINT64_MAX},{base+"/0/consumed/0",0.0},
            {base+"/0/consumed/0",true},{base+"/0/consumed/0","0"},
            {base+"/0/consumed/0",nullptr},{base+"/0/consumed/5",1},
            {"/world/original_production/development_revision",16},{"/world/original_production/development_revision",18},{"/world/original_production/development_revision",19},{"/world/original_production/development_revision",20},{"/world/original_production/development_revision",21},{"/world/original_production/development_revision",22},{"/world/original_production/development_revision",23},{"/world/original_production/development_revision",24},{"/world/original_production/development_revision",26}}) {
        auto j=good;j.at(json::json_pointer(path))=value;reject(j);
    }
    {auto j=good;auto& rows=j.at(json::json_pointer(base));std::swap(rows[0],rows[1]);reject(j);}
    {auto j=good;auto& rows=j.at(json::json_pointer(base));rows.push_back(rows[0]);reject(j);}
    {auto j=good;j.at(json::json_pointer(base))[0]["clock"]=39;reject(j);}
    {auto j=good;j.at(json::json_pointer(base))[0].erase("consumed");reject(j);}
    {auto j=good;j.at(json::json_pointer(base))[0].erase("building_id");reject(j);}
    {auto j=good;j["world"]["original_goods"]["totals"][0]["consumed"]=UINT64_MAX;
        for (auto& row:j.at(json::json_pointer(base))) row["consumed"][0]=UINT64_MAX;reject(j);}
    // A balanced sink cannot consume more than one full buffer per eligible
    // House event, even when all global and per-House totals agree.
    {auto j=good;const auto n=UINT64_C(1000000);
        auto& amount=j.at(json::json_pointer(base))[0]["consumed"][0];amount=amount.get<std::uint64_t>()+n;
        for (const auto key:{"consumed","produced"}) {
            auto& total=j["world"]["original_goods"]["totals"][0][key];total=total.get<std::uint64_t>()+n;
        }
        auto& harvested=j["world"]["original_production"]["hemp_farms"][0]["harvested"];
        harvested=harvested.get<std::uint64_t>()+n;reject(j);}
    const auto doc=f.document(world);
    for (int fault=0;fault<4;++fault) {
        auto bad=doc;
        if (fault==0) bad.world.original_production->house_consumption.clear();
        if (fault==1) ++bad.world.original_production->house_consumption[0].consumed[0];
        if (fault==2) bad.world.original_production->house_consumption[0].building_id=sim::BuildingId{};
        if (fault==3) bad.world.original_production->development_revision=16;
        refuses([&]{save::write_save(f.target,bad,f.root.data,f.policy->building_mask());});
        check(f::read_text(f.target)==pristine,"Bad DTO replaced the protected consumption save");
    }
    auto duplicate=pristine;const auto marker=duplicate.find("\"house_consumption\"");const auto brace=duplicate.find('{',marker);
    check(brace!=std::string::npos,"Consumption duplicate-key case absent");duplicate.insert(brace+1,"\"consumed\":[0,0,0,0,0,0],");
    f::write_text(hostile,duplicate);refuses([&]{(void)save::read_save(hostile);});
}
void zero_model_refuses_sink() {
    Fixture f(0,0);bool delivered=false;
    for (int i=0;i<20000 && !delivered;++i) {f.world.tick();const auto stock=house_stock(f.world);delivered=stock[0]>0 && stock[1]>0;}
    check(delivered,"Zero-need control lacked actual arrivals");f.roundtrip(f.world);
    auto bad=f.document(f.world);std::size_t index=0;
    while (!bad.world.original_economy->houses[index].house.hemp) ++index;
    --bad.world.original_economy->houses[index].house.hemp;
    ++bad.world.original_production->house_consumption[index].consumed[0];++bad.world.original_goods->totals[0].consumed;
    refuses([&]{(void)f.restore(bad);});
    refuses([&]{save::write_save(f.target,bad,f.root.data,f.policy->building_mask());});
    conserved(f.world);
}
void no_arrival_refuses_sink() {
    Fixture f;f::buy(f.world,sim::set_building_operation(f.market,false));
    const auto farm=*f.world.building_owner_at({4,9});bool ready=false;
    for (int i=0;i<10000 && !ready;++i) {
        f.world.tick();
        for (const auto& stock:f.world.original_goods_state()->stocks)
            ready=ready || (f.world.ticks()>=397 && stock.building_id==farm && stock.amounts[0]>0);
    }
    check(ready,"Arrival-origin forgery lacked genuine paid Farm stock");
    const auto pristine=f.roundtrip(f.world);auto bad=f.document(f.world);
    for (auto& stock:bad.world.original_goods->stocks) if (stock.building_id==farm) --stock.amounts[0];
    ++bad.world.original_goods->totals[0].consumed;
    ++bad.world.original_production->house_consumption[0].consumed[0];
    // All mass and age/model bounds still agree. A completely consumed gift
    // must nevertheless have a possible actual Distributor arrival.
    refuses([&]{(void)f.restore(bad);});
    refuses([&]{save::write_save(f.target,bad,f.root.data,f.policy->building_mask());});
    auto encoded=json::parse(pristine);
    for (auto& stock:encoded["world"]["original_goods"]["stocks"])
        if (stock["building_id"]==static_cast<unsigned>(farm)) stock["amounts"][0]=stock["amounts"][0].get<int>()-1;
    encoded["world"]["original_goods"]["totals"][0]["consumed"]=1;
    encoded["world"]["original_production"]["house_consumption"][0]["consumed"][0]=1;
    const auto hostile=f.root.path/"consumed-without-arrival.json";f::write_text(hostile,encoded.dump());
    refuses([&]{(void)f.restore(save::read_save(hostile));});
    check(f::read_text(f.target)==pristine,"Balanced origin forgery replaced the valid paid save");
}
void positive_model_bound_refuses_sink() {
    Fixture f(1,1);std::uint64_t opportunities=0;std::size_t selected=0,good=0;bool ready=false;
    for (int tick=0;tick<30000 && !ready;++tick) {
        const auto dispatch=orig::next_scheduler_dispatch(f.world.ticks());
        // All six Houses were genuinely paid at tick0; a boundary before the
        // first legal immigration at120 is not an eligible House event.
        if (dispatch && dispatch->consumption && f.world.ticks()+1>=120) ++opportunities;
        f.world.tick();
        const auto& homes=f.world.original_economy_state()->houses;
        const auto& history=f.world.original_production_state()->house_consumption;
        for (std::size_t i=0;i<homes.size() && !ready;++i) for (std::size_t g=0;g<2 && !ready;++g) {
            const auto stock=orig::goods_stock(homes[i].house,static_cast<orig::Goods>(g));
            if (opportunities && history[i].consumed[g]+static_cast<std::uint64_t>(stock)>opportunities) {
                selected=i;good=g;ready=true;
            }
        }
    }
    check(ready,"Actual House receipts did not exceed its independently counted Need1 opportunities");
    const auto pristine=f.roundtrip(f.world);auto bad=f.document(f.world);
    auto& house=bad.world.original_economy->houses[selected].house;
    auto& counter=bad.world.original_production->house_consumption[selected].consumed[good];
    const auto debit=opportunities+1-counter;
    const auto old=orig::goods_stock(house,static_cast<orig::Goods>(good));
    check(debit>0 && debit<=static_cast<std::uint64_t>(old) && opportunities+1<=8*opportunities,
          "Model-only counter forgery did not fit the actual House inventory and static event cap");
    const auto remaining=static_cast<std::int16_t>(old-static_cast<int>(debit));
    if (good==0) house.hemp=remaining;else house.ceramics=remaining;
    counter+=debit;bad.world.original_goods->totals[good].consumed+=debit;
    auto& housing=bad.world.original_production->housing.houses[selected];
    check(house.type==orig::HouseType::Shelter && !housing.first_transitions[1] && !housing.first_transitions[2] &&
          housing.consumption_rows[0].events==opportunities,
          "Need1 model-only control unexpectedly left its actual Shelter consumption row");
    housing.consumption_rows[0].consumed[good]+=debit;
    // Origin, cumulative reception, six-column mass, age, and the structural
    // eight-unit event bound all remain valid. Only prepared Need1 rejects it.
    sim::World::validate_original_production_snapshot(bad.world);
    refuses([&]{(void)f.restore(bad);});
    refuses([&]{save::write_save(f.target,bad,f.root.data,f.policy->building_mask());});
    auto encoded=json::parse(pristine);
    encoded["world"]["original_economy"]["houses"][selected]["house"][good==0 ? "hemp":"ceramics"]=remaining;
    encoded["world"]["original_production"]["house_consumption"][selected]["consumed"][good]=counter;
    encoded["world"]["original_production"]["housing"]["houses"][selected]["consumption_rows"][0]["consumed"][good]=housing.consumption_rows[0].consumed[good];
    encoded["world"]["original_goods"]["totals"][good]["consumed"]=bad.world.original_goods->totals[good].consumed;
    const auto hostile=f.root.path/"positive-need-bound.json";f::write_text(hostile,encoded.dump());
    const auto structurally_valid=save::read_save(hostile);
    check(structurally_valid.world==bad.world,"Static parse altered the model-only forged DTO");
    refuses([&]{(void)f.restore(structurally_valid);});
    check(f::read_text(f.target)==pristine,"Model-bound refusal changed the valid save");
    std::cout<<"Isolated Need1 guard: actual received stock, "<<opportunities<<" eligible events, good "<<good<<'\n';
}
}
int main() {
    try {Fixture f;empty_boundaries(f);const auto before=positive_boundary(f,0);positive_boundary(f,7);
        adversarial(f,before);zero_model_refuses_sink();no_arrival_refuses_sink();positive_model_bound_refuses_sink();
        std::cout<<"Consumption save PASS: paid delivered Hemp/Ceramics, 0/7 boundary F5/F9, min0, 20k replay and "
                 <<rejected<<" adversarial refusals\n";return 0;}
    catch (const std::exception& e) {std::cerr<<"Original consumption save FAIL: "<<e.what()<<'\n';return 1;}
}
