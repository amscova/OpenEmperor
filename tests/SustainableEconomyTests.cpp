#include "OriginalFinanceFixture.h"

#include <array>
#include <iostream>
#include <stdexcept>

namespace {
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
using f::check;

struct Fixture {
    f::Root root;
    std::shared_ptr<const sim::MapPermissions> policy=f::permissions();
    std::shared_ptr<const orig::OriginalRulesModel> model;
    explicit Fixture(std::string text=oe_production_fixture::authored_model_text(37)) {
        f::write_text(root.data/"Model/EmperorBuildingModels.txt",text);
        model=f::save::prepare_original_production_rules_model(root.data);
    }
    sim::World world() const {return sim::World::create_original_sustainable_economy(policy,model);}
    sim::World restore(const sim::WorldSnapshot& snapshot) const {
        return sim::World::restore_original_sustainable_economy(snapshot,policy,model);
    }
    sim::World roundtrip(const sim::World& world,const char* name) const {
        const auto mask=policy->building_mask();const auto path=root.path/name;
        const auto document=f::save::make_document(root.data,f::map_relative,mask,world);
        f::save::write_save(path,document,root.data,mask);const auto bytes=f::read_text(path);
        const auto parsed=f::save::read_save(path);
        auto restored=f::save::restore_original_sustainable_economy(parsed,root.data,mask,policy);
        check(parsed.world==world.snapshot() && restored.snapshot()==world.snapshot(),
            "Rule12 complete SaveDocument changed after parse/restore");
        f::save::write_save(path,f::save::make_document(root.data,f::map_relative,mask,restored),root.data,mask);
        check(f::read_text(path)==bytes,"Rule12 complete canonical save bytes changed on rewrite");
        return restored;
    }
};
sim::BuildingId buy(sim::World& world,sim::CommandType kind,sim::Cell cell) {
    f::buy(world,{kind,cell});return *world.building_owner_at(cell);
}
void denies(sim::World& world,sim::Command command) {
    const auto before=world.snapshot();const auto result=world.execute(command);
    check(!result.accepted && !result.changed && world.snapshot()==before,
        "Rule12 rejected command partially changed paid World");
}
template<class F> void refuses(F action) {
    bool rejected=false;try {action();} catch(const std::exception&) {rejected=true;}
    check(rejected,"Wrong-generation or hostile Rule12 restore was accepted");
}
void through(sim::World& world,std::uint64_t end) {
    while(world.ticks()<end) world.tick();
}
void authority(const sim::World& world) {
    check(world.navigation_valid() && world.population_valid() && world.production_balance_valid() &&
        world.fire_state_valid() && world.city_economy_valid(),"Rule12 authority/conservation failed");
    // Independent contract arithmetic: the old installed liability is retained;
    // only Rule12 aligns its placement-relative invoice with the 816t month.
    std::uint64_t maintenance=0;
    const std::uint64_t interval=world.rule_version()==12 ? 816:400;
    for(const auto& b:world.buildings())
        maintenance+=(world.ticks()-b.placed_tick)/interval*static_cast<std::uint64_t>(world.workforce_required(b.id))*2;
    check(maintenance==world.maintenance_spent_total(),"Rule12 actual installed invoice differs from contract");
    check(sim::original_finance_accounting_valid(world.treasury(),world.taxes_collected_total(),
        world.construction_spent_total(),world.maintenance_spent_total()),"Rule12 Funds conservation failed");
    const auto& food=world.original_production_state()->food;
    std::array<std::uint64_t,2> harvested{},held{};
    for(const auto& farm:food.farms) harvested[static_cast<unsigned>(farm.crop)]+=farm.harvested;
    for(const auto& stock:food.stocks) for(std::size_t g=0;g<2;++g)
        held[g]+=static_cast<std::uint64_t>(stock.amounts[g])+stock.consumed[g];
    for(const auto& trip:food.trips) for(std::size_t g=0;g<2;++g)
        held[g]+=static_cast<std::uint64_t>(trip.loaded[g]-trip.delivered[g]-trip.returned[g]);
    check(harvested==held,"Rule12 actual crop mass differs from harvest/stock/cargo/sink");
    for(const auto& balance:world.original_goods_balances()) check(balance.conserved,"Rule12 goods conservation failed");
}
void identity_and_strict_generation() {
    Fixture fixture;auto world=fixture.world();
    check(world.rule_version()==12 && world.active_rules().version==12 && world.original_sustainable_economy_enabled() &&
        world.original_production_enabled() && world.original_fire_protection_enabled() && world.treasury()==12000,
        "Rule12 requires an explicit original economy/fire identity");
    check(sim::current_rule_version(sim::RulesProfile::CityV16)==4 &&
        sim::current_rule_version(sim::RulesProfile::CityV11)==3,"Rule12 changed legacy defaults");
    check(sim::Rules::original_production_building_limit==82 && sim::Rules::original_production_courier_limit==58 &&
        sim::Rules::city_v10_household_limit==20,"Rule12 widened global resource bounds");
    refuses([&]{sim::World ordinary(fixture.policy,sim::RulesProfile::CityV16,12);});
    refuses([&]{(void)sim::World::restore_original_fire_protection(world.snapshot(),fixture.policy,fixture.model);});
    refuses([&]{(void)sim::World::restore_original_production_development(world.snapshot(),fixture.policy,fixture.model);});
    auto old=sim::World::create_original_fire_protection(fixture.policy,fixture.model);
    refuses([&]{(void)fixture.restore(old.snapshot());});
    check(fixture.restore(world.snapshot()).snapshot()==world.snapshot(),"Rule12 empty restore changed authority");
    auto oversized=world.snapshot();oversized.couriers.resize(59);
    refuses([&]{(void)fixture.restore(oversized);});
    oversized=world.snapshot();oversized.buildings.resize(83);
    refuses([&]{(void)fixture.restore(oversized);});
}
void paid_food_capacity_and_fields() {
    Fixture fixture;
    for(const unsigned rule:{10U,11U,12U}) {
        auto world=rule==10 ? sim::World::create_original_production_development(fixture.policy,fixture.model):
            rule==11 ? sim::World::create_original_fire_protection(fixture.policy,fixture.model):fixture.world();
        const auto a=buy(world,sim::CommandType::PlaceOriginalFoodFarm,{0,5});
        const auto b=buy(world,sim::CommandType::PlaceOriginalFoodFarm,{7,5});
        sim::Command field{sim::CommandType::PlaceOriginalMilletField,{3,5}};field.building_id=a;
        f::buy(world,field);
        field.cell={10,5};field.building_id=b;f::buy(world,field);
        const auto before=world.construction_spent_total();
        if(rule!=12) {
            denies(world,{sim::CommandType::PlaceOriginalFoodFarm,{14,5}});
            check(world.original_production_state()->food.farms.size()==2,"Historical two-Farm cap changed");
        } else {
            const auto c=buy(world,sim::CommandType::PlaceOriginalFoodFarm,{14,5});
            check(world.construction_spent_total()-before==static_cast<std::uint64_t>(
                fixture.model->building_value(orig::ModelBuilding::Farmhouse,orig::BuildingField::Cost)),
                "Third Farm bypassed its prepared positive cost");
            field.cell={17,5};field.building_id=static_cast<sim::BuildingId>(0);denies(world,field);
            field.building_id=a;denies(world,field);
            field.building_id=c;field.cell={23,19};denies(world,field);
            field.cell={17,5};f::buy(world,field);
            denies(world,{sim::CommandType::PlaceOriginalFoodFarm,{0,10}});
            check(world.original_production_state()->food.farms.size()==3 && world.couriers().size()==3 &&
                world.original_production_state()->food.stocks.size()==3,"Third paid chain lacks sole actor/stock/field authority");
            const auto snapshot=world.snapshot();
            check(fixture.restore(snapshot).snapshot()==snapshot,"Three paid Food chains did not restore exactly");
            auto hostile=snapshot;
            hostile.original_production->food.farms[2].field_id=hostile.original_production->food.farms[0].field_id;
            refuses([&]{(void)fixture.restore(hostile);});
            hostile=snapshot;hostile.original_production->food.farms.push_back(hostile.original_production->food.farms.back());
            refuses([&]{(void)fixture.restore(hostile);});
        }
        authority(world);
    }
    // Original physical/height/source predicates remain mandatory for all cells
    // of the third 3x3 Farm body, independently from its increased count limit.
    std::vector<sim::MapCellPermission> cells(f::width*f::height,
        {true,true,false,0,sim::BuildBlocker::None,sim::BuildBlocker::None});
    cells[7*f::width+16]={false,false,true,0,sim::BuildBlocker::OriginalStructure,sim::BuildBlocker::OriginalStructure};
    const auto restricted=std::make_shared<const sim::MapPermissions>(f::width,f::height,3,std::move(cells),
        std::vector<sim::FixedGatePassage>{});
    auto world=sim::World::create_original_sustainable_economy(restricted,fixture.model);
    buy(world,sim::CommandType::PlaceOriginalFoodFarm,{0,5});buy(world,sim::CommandType::PlaceOriginalFoodFarm,{7,5});
    denies(world,{sim::CommandType::PlaceOriginalFoodFarm,{14,5}});
}
void installed_upkeep_pause_fire_and_restore() {
    Fixture fixture;
    auto old=sim::World::create_original_fire_protection(fixture.policy,fixture.model);
    auto world=fixture.world();
    const auto farm=buy(world,sim::CommandType::PlaceOriginalFoodFarm,{0,5});
    buy(old,sim::CommandType::PlaceOriginalFoodFarm,{0,5});
    f::buy(world,sim::set_building_operation(farm,false));
    check(world.workforce_required(farm)==12 && world.workers_assigned(farm)==0,"Authored paused fixture unexpectedly staffed");
    through(world,400);through(old,400);
    check(world.maintenance_spent_total()==0 && old.maintenance_spent_total()==24 &&
        world.original_worker_upkeep_interval()==816 && old.original_worker_upkeep_interval()==400,
        "Rule12 invoice timing changed Rule11 or billed before its month");
    check(world.maintenance_projection(416).interval_cost==24,"Rule12 installed projection uses a different clock");
    through(world,816);check(world.maintenance_spent_total()==24,"Paused/unstaffed Farm escaped monthly liability");
    through(world,2447);check(world.building_on_fire(farm),"Real natural fire did not occur in authored invoice fixture");
    auto resumed=fixture.restore(world.snapshot());through(world,2448);through(resumed,2448);
    check(world.maintenance_spent_total()==72 && world.snapshot()==resumed.snapshot(),
        "Burning/paused installation escaped real upkeep or changed after restore");
    authority(world);authority(old);
}
void isolated_third_chain_paid_repair_and_begun_edge() {
    Fixture fixture;auto world=fixture.world();
    for(int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,4}});
    for(int x=0;x<13;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,9}});
    for(int x=14;x<19;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,9}});
    for(int y=5;y<9;++y) f::buy(world,{sim::CommandType::PlaceRoad,{12,y}});
    for(int x:{0,3,6,9,12,15,18,21}) buy(world,sim::CommandType::PlaceHousehold,{x,2});
    for(int x:{0,3,6,9}) buy(world,sim::CommandType::PlaceHousehold,{x,7});
    std::array<sim::BuildingId,3> farms{};
    for(std::size_t i=0;i<farms.size();++i) {
        const auto x=static_cast<int>(i)*7;
        farms[i]=buy(world,sim::CommandType::PlaceOriginalFoodFarm,{x,10});
        sim::Command field{sim::CommandType::PlaceOriginalMilletField,{x+3,10}};field.building_id=farms[i];f::buy(world,field);
    }
    const auto market=buy(world,sim::CommandType::PlaceMarket,{0,5});
    through(world,1000);authority(world);
    const auto& third=world.original_production_state()->food.farms[2];
    check(third.harvested>0,"Isolated third chain never earned a real staffed harvest");
    const auto actor=std::find_if(world.couriers().begin(),world.couriers().end(),[&](const auto& c){return c.owner==farms[2];});
    check(actor!=world.couriers().end(),"Third chain lost its paid CourierId");const auto id=actor->id;
    check(actor->phase==sim::CourierPhase::IdleAtWorkshop && world.original_food_cargo(id)==0 &&
        std::none_of(world.original_production_state()->food.trips.begin(),world.original_production_state()->food.trips.end(),
            [&](const auto& trip){return trip.source==farms[2];}),
        "Disconnected third chain granted Market cargo, reservation or arrival across a road gap");
    // Isolate the route proof from competition for the old Market's real room:
    // pause the other paid sources and buy an empty second Market normally.
    for(std::size_t i=0;i<2;++i) f::buy(world,sim::set_building_operation(farms[i],false));
    const auto second_market=buy(world,sim::CommandType::PlaceMarket,{9,5});
    const auto before=world.treasury();f::buy(world,{sim::CommandType::PlaceRoad,{13,9}});
    check(world.treasury()==before-2,"Disconnected chain repair did not spend its normal paid road cost");
    std::uint64_t bounded=0;
    while(bounded++<400 && !(world.courier(id).phase==sim::CourierPhase::ToWarehouse && world.courier(id).edge_progress>0)) world.tick();
    const auto moving=world.courier(id);
    check(moving.phase==sim::CourierPhase::ToWarehouse && moving.edge_progress>0 &&
        (moving.target==market || moving.target==second_market) &&
        world.original_food_cargo(id)>0 && world.couriers().size()<=58,"Paid repair never dispatched the actual third-chain cargo");
    const auto next=moving.path.at(moving.path_vertex+1);
    check(world.object_at(next)==sim::Object::Road,"Third chain fixture did not begin its first road edge");
    denies(world,{sim::CommandType::RemoveRoad,next});
    // The next gap is a future edge, so its ordinary removal may safely leave
    // the current begun edge intact and preserve the loaded cargo while waiting.
    f::buy(world,{sim::CommandType::RemoveRoad,{13,9}});
    check(world.courier(id).edge_progress==moving.edge_progress && world.original_food_cargo(id)>0,
        "Future road disconnection erased the third chain's begun edge/cargo");
    through(world,world.ticks()+20);const auto cargo=world.original_food_cargo(id);
    check(world.courier(id).route_pending && cargo>0,"Disconnected third courier failed to wait with its actual cargo");
    auto resumed=fixture.roundtrip(world,"isolated-third-chain.json");
    f::buy(world,{sim::CommandType::PlaceRoad,{13,9}});f::buy(resumed,{sim::CommandType::PlaceRoad,{13,9}});
    const auto arrived=[&](const sim::World& value) {
        return std::any_of(value.original_production_state()->food.trips.begin(),value.original_production_state()->food.trips.end(),
            [&](const auto& trip){return trip.source==farms[2] && trip.delivered[1]>0;});
    };
    bounded=0;
    while(bounded++<500 && !arrived(world)) {world.tick();resumed.tick();}
    check(arrived(world) && world.snapshot()==resumed.snapshot(),
        "Real repaired third-chain Market arrival changed across complete save/load continuation");
    authority(world);
}
void debt_then_actual_tax_recovery() {
    // Explicit authored input, prepared before World creation. High installed
    // liability creates genuine debt before late Houses admit their residents;
    // their later positive payments recover it without a live grant or edit.
    auto text=oe_production_fixture::authored_model_text(37);
    for(std::size_t row=0;row<orig::house_row_count;++row)
        oe_production_fixture::set_house_field(text,row,18,100);
    oe_finance_fixture::set_building_field(text,orig::ModelBuilding::HerbalistsStall,orig::BuildingField::Employees,7000);
    Fixture fixture(text);auto world=fixture.world();
    const auto herbalist=buy(world,sim::CommandType::PlaceHealthPost,{0,12});
    f::buy(world,sim::set_building_operation(herbalist,false));
    through(world,700);
    for(int y:{4,9}) for(int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,y}});
    for(int y:{2,7}) for(int x:{0,3,6,9,12,15}) buy(world,sim::CommandType::PlaceHousehold,{x,y});
    through(world,816);
    check(world.treasury()<0 && world.taxes_collected_total()==0 && world.maintenance_spent_total()==14000,
        "Paid installed liability failed to create genuine negative Funds before admission");
    denies(world,{sim::CommandType::PlaceFireWatch,{20,12}});
    auto continuation=fixture.roundtrip(world,"negative-funds.json");through(world,1632);through(continuation,1632);
    check(world.treasury()>=0 && world.taxes_collected_total()>28000 && world.maintenance_spent_total()==28000 &&
        world.total_population()>0 && world.snapshot()==continuation.snapshot(),
        "Real monthly taxes did not recover debt identically after restore");
    authority(world);
    (void)fixture.roundtrip(world,"recovered-funds.json");
}
}
int main() {
    try {
        identity_and_strict_generation();paid_food_capacity_and_fields();
        installed_upkeep_pause_fire_and_restore();isolated_third_chain_paid_repair_and_begun_edge();
        debt_then_actual_tax_recovery();
        std::cout<<"Rule12 paid capacity, strict generations, installed monthly upkeep, fire and debt recovery PASS\n";
        return 0;
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
