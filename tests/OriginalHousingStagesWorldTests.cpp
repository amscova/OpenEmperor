#include "OriginalProductionFixture.h"
#include "simulation/OriginalHousingDiagnosis.h"
#include "core/PerformanceDiagnostics.h"
#include "simulation/original/OriginalScheduler.h"
#include "simulation/original/Desirability.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <string_view>
#include <tuple>

namespace {
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
namespace perf=openemperor::performance;
using f::check;

struct Fixture {
    f::Root root;
    std::shared_ptr<const sim::MapPermissions> policy=f::permissions();
    std::shared_ptr<const orig::OriginalRulesModel> model;
    explicit Fixture(const f::fs::path& real={},orig::Difficulty difficulty=orig::Difficulty::Normal) {
        auto bytes=real.empty() ? oe_production_fixture::authored_model_text(7):
            f::read_text(real/"Model/EmperorBuildingModels.txt");
        if (real.empty()) oe_production_fixture::set_house_field(bytes,2,1,0);
        f::write_text(root.data/"Model/EmperorBuildingModels.txt",bytes);
        model=f::save::prepare_original_production_rules_model(root.data,difficulty);
    }
    sim::World world() const {return sim::World::create_original_production_development(policy,model);}
};
const sim::OriginalHouseState& house(const sim::World& world,sim::BuildingId id) {
    for (const auto& value:world.original_economy_state()->houses) if (value.building_id==id) return value;
    throw std::runtime_error("Stages lost paid House authority");
}
const sim::OriginalHousingHouse& history(const sim::World& world,sim::BuildingId id) {
    for (const auto& value:world.original_production_state()->housing.houses) if (value.building_id==id) return value;
    throw std::runtime_error("Stages lost bounded transition history");
}
void authority(const sim::World& world) {
    check(world.production_balance_valid()&&world.population_valid()&&world.city_economy_valid()&&world.navigation_valid(),
          "Stages real production/population/navigation invariant failed");
    for (const auto& value:world.original_economy_state()->houses) {
        const auto elite=world.building(value.building_id).kind==sim::Object::OriginalElitePlot;
        check(orig::is_elite(value.house.type)==elite,"Paid House changed family through evolution");
        check(history(world,value.building_id).consumption_rows.size()==15&&
              history(world,value.building_id).first_transitions.size()==15,"Stages history lost a model row");
        if (!elite) check(!value.house.confucian,"Elite-only Academy served a Common House");
    }
    for (unsigned good=0;good<6;++good) check(world.original_goods_balance(static_cast<orig::Goods>(good)).conserved,
                                             "Stages actual goods lost mass");
}
template<class Predicate> void until(sim::World& world,Predicate ready,const char* message,int limit=30000) {
    for (int i=0;i<limit&&!ready();++i) {world.tick();authority(world);}
    check(ready(),message);
}
void compare_matrix(const sim::World& world,sim::BuildingId id) {
    const auto diagnosis=world.original_housing_diagnosis(id);
    const auto& actual=house(world,id).house;
    check(diagnosis.actual==actual&&diagnosis.current_row==orig::house_row(actual.type)&&
          diagnosis.next_row==orig::next_row(actual.type),"Diagnosis replaced actual input/type selectors");
    check(diagnosis.elite_family==orig::is_elite(actual.type)&&diagnosis.devolve_limit==orig::devolve_limit(actual.type),
          "Diagnosis lost actual family/counter boundary");
    check(diagnosis.admission_capacity==world.original_rules_model()->house_rules().row_capacity(orig::admission_capacity_row(actual.type))&&
          diagnosis.census_capacity==world.original_rules_model()->house_rules().row_capacity(orig::census_capacity_row(actual.type)),
          "Diagnosis collapsed separate admission/census selectors");
    auto current=actual;current.desirability=static_cast<std::int8_t>(diagnosis.cached_desirability);
    const auto direct=orig::evolve_step(world.original_rules_model()->house_rules(),current);
    check(diagnosis.pure_result_type==direct.type&&diagnosis.pure_result_counter==direct.devolve_counter,
          "Current World diagnosis differs from immutable pure decision");
    const auto matrix=world.original_housing_decision_matrix(id);
    for (std::size_t row=0;row<matrix.size();++row) {
        const auto& result=matrix[row];auto hypothetical=current;
        hypothetical.type=static_cast<orig::HouseType>(row+3);
        const auto model_row=static_cast<orig::ModelHouse>(row);
        const auto& rules=world.original_rules_model()->house_rules();
        const auto pure=orig::evolve_step(rules,hypothetical);
        check(result.row==model_row&&result.hypothetical_type==hypothetical.type&&
              result.needs_met==orig::needs_met(rules,model_row,hypothetical)&&
              result.verdict==orig::evolution_verdict(rules,hypothetical)&&result.pure_result_type==pure.type&&
              result.pure_result_counter==pure.devolve_counter,"Fifteen-row World matrix differs from pure input query");
        check(result.row_capacity==rules.row_capacity(model_row)&&
              result.low==rules.value(model_row,orig::HouseField::DesirabilityLow)&&
              result.high==rules.value(model_row,orig::HouseField::DesirabilityHigh)&&
              result.same_family==(orig::is_elite(hypothetical.type)==orig::is_elite(actual.type)),
              "Hypothetical matrix changed capacity/threshold/family fact");
        std::array<bool,24> seen{};bool met=true;
        for (const auto& requirement:result.requirements) {
            const auto field=static_cast<std::size_t>(requirement.field);
            check(field<seen.size()&&!seen[field],"Diagnosis duplicated or invented a requirement");seen[field]=true;
            check(requirement.required==rules.value(model_row,requirement.field),"Diagnosis changed prepared requirement");
            const bool either=requirement.field==orig::HouseField::DaoistOrBuddhist||
                              requirement.field==orig::HouseField::BronzewareOrLacquerware;
            const bool accepted=requirement.presence ? requirement.required==0||requirement.actual!=0||
                (either&&requirement.alternative!=0):requirement.actual>=requirement.required||
                (either&&requirement.alternative>=requirement.required);
            check(requirement.met==accepted,"Requirement diagnosis changed nonzero/threshold/OR semantics");met&=accepted;
        }
        check(met==result.needs_met,"Requirement tree and row Boolean disagree");
    }
}
void all_rows_all_difficulties(const f::fs::path& real) {
    for (unsigned d=0;d<5;++d) {
        Fixture fixture(real,static_cast<orig::Difficulty>(d));auto world=fixture.world();
        for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,4}});
        f::buy(world,{sim::CommandType::PlaceHousehold,{0,2}});
        f::buy(world,{sim::CommandType::PlaceOriginalElitePlot,{3,2}});
        const auto common=*world.building_owner_at({0,2}),elite=*world.building_owner_at({3,2});
        compare_matrix(world,common);compare_matrix(world,elite);
        check(world.original_housing_decision_matrix(elite)[8].row_capacity==0,
              "Original EliteUnoccupied zero capacity was silently repaired");
        for (int t=0;t<400;++t) world.tick();
        compare_matrix(world,common);compare_matrix(world,elite);authority(world);
        const auto before=world.snapshot();perf::set_enabled(true);perf::reset();
        for (int i=0;i<100;++i) {compare_matrix(world,common);compare_matrix(world,elite);}
        for (const auto counter:{perf::Counter::WorldCopies,perf::Counter::WorldRestores,perf::Counter::WorldExecutes,
             perf::Counter::BfsCalls,perf::Counter::RouteRefreshes,perf::Counter::SimulationTicks,
             perf::Counter::FileReads,perf::Counter::FileWrites,perf::Counter::AssetDecodes,perf::Counter::TextureUploads})
            check(perf::counter(counter)==0,"Read-only housing diagnosis performed gameplay/preparation/IO");
        perf::set_enabled(false);check(world.snapshot()==before,"Housing matrix mutated actual World authority");
    }
}
void pure_negative_needs_and_counter_boundaries() {
    constexpr std::array fields{orig::HouseField::Water,orig::HouseField::Herbalist,orig::HouseField::Acupuncture,
        orig::HouseField::Music,orig::HouseField::Acrobat,orig::HouseField::Drama,orig::HouseField::Ancestor,
        orig::HouseField::Confucian,orig::HouseField::DaoistOrBuddhist,orig::HouseField::FoodQuality,
        orig::HouseField::Hemp,orig::HouseField::Ceramics,orig::HouseField::Tea,orig::HouseField::Silk,
        orig::HouseField::BronzewareOrLacquerware};
    auto bytes=oe_production_fixture::authored_model_text(7);
    for (std::size_t row=0;row<15;++row) for (const auto field:fields)
        oe_production_fixture::set_house_field(bytes,row,static_cast<unsigned>(field),1);
    const auto parsed=orig::parse_model_data(bytes);check(bool(parsed),"Explicit pure fifteen-row fixture failed parsing");
    for (unsigned difficulty=0;difficulty<5;++difficulty) {
        const orig::HouseEvolutionRules rules(*parsed.model,static_cast<orig::Difficulty>(difficulty));
        for (unsigned type=3;type<=17;++type) {
            orig::HouseState positive;positive.type=static_cast<orig::HouseType>(type);positive.occupants=1;
            positive.water=true;positive.herbalist=positive.acupuncture=positive.music=positive.acrobat=positive.drama=1;
            positive.ancestor=positive.confucian=positive.daoist=positive.buddhist=1;
            positive.food_quality=1;positive.hemp=positive.ceramics=positive.tea=positive.silk=positive.bronzeware=positive.lacquerware=1;
            const auto row=orig::house_row(positive.type);
            check(orig::needs_met(rules,row,positive),"Explicit supplied pure row did not meet its exact needs");
            for (const auto field:fields) {
                auto negative=positive;
                switch (field) {
                case orig::HouseField::Water:negative.water=false;break;
                case orig::HouseField::Herbalist:negative.herbalist=0;break;
                case orig::HouseField::Acupuncture:negative.acupuncture=0;break;
                case orig::HouseField::Music:negative.music=0;break;
                case orig::HouseField::Acrobat:negative.acrobat=0;break;
                case orig::HouseField::Drama:negative.drama=0;break;
                case orig::HouseField::Ancestor:negative.ancestor=0;break;
                case orig::HouseField::Confucian:negative.confucian=0;break;
                case orig::HouseField::DaoistOrBuddhist:negative.daoist=negative.buddhist=0;break;
                case orig::HouseField::FoodQuality:negative.food_quality=0;break;
                case orig::HouseField::Hemp:negative.hemp=0;break;
                case orig::HouseField::Ceramics:negative.ceramics=0;break;
                case orig::HouseField::Tea:negative.tea=0;break;
                case orig::HouseField::Silk:negative.silk=0;break;
                case orig::HouseField::BronzewareOrLacquerware:negative.bronzeware=negative.lacquerware=0;break;
                default:throw std::runtime_error("Unexpected authored pure requirement");
                }
                check(!orig::needs_met(rules,row,negative),"One missing pure need was hidden by another supplied category");
            }
            auto failed=positive;failed.water=false;failed.devolve_counter=0;
            if (type==11) {check(orig::evolve_step(rules,failed)==failed,"Elite11 pure evaluation was not inert");continue;}
            const unsigned limit=type<=10 ? 9:type==12 ? 11:type==13 ? 49:17;
            for (unsigned i=0;i<limit;++i) {const auto next=orig::evolve_step(rules,failed);check(next.type==failed.type&&next.devolve_counter==i+1,"Pure counter demoted before literal boundary");failed=next;}
            const auto after=orig::evolve_step(rules,failed);
            const auto expected=type==3||type==12 ? type:type-1;
            check(static_cast<unsigned>(after.type)==expected&&after.devolve_counter==0,"Pure10/12/50/18 event boundary changed");
        }
        auto terminal=orig::HouseState{};terminal.type=orig::HouseType::LuxuriousApartment;
        check(!orig::evolve_target(terminal.type)&&!orig::evolve_target(orig::HouseType::EliteUnoccupied),
              "Common terminal or Elite11 gained an implicit family edge");
    }
}
void historical_elite_price_and_refusal() {
    constexpr sim::Command elite{sim::CommandType::PlaceOriginalElitePlot,{6,2}};
    const auto refused=[&](sim::World& world) {
        const auto before=world.snapshot();
        check(world.construction_cost(elite.type)==0&&world.snapshot()==before,
              "Historical Elite quote gained a price or mutated authority");
        const auto validation=world.validate(elite);
        check(!validation.accepted&&!validation.changed&&world.snapshot()==before,
              "Historical Elite validation admitted a new body or changed authority");
        const auto result=world.execute(elite);
        const auto after=world.snapshot();
        if (result.accepted||result.changed||after!=before) {
            auto sequence_only=after;sequence_only.command_sequence=before.command_sequence;
            throw std::runtime_error("Historical Elite refusal changed authority: profile="+
                std::to_string(static_cast<unsigned>(world.profile()))+" rule="+std::to_string(world.rule_version())+
                " accepted="+std::to_string(result.accepted)+" changed="+std::to_string(result.changed)+
                " sequence="+std::to_string(before.command_sequence)+"->"+std::to_string(after.command_sequence)+
                " result_sequence="+std::to_string(result.sequence)+
                " only_command_sequence_changed="+std::to_string(sequence_only==before)+" reason="+result.reason);
        }
    };
    const auto ordinary_house=[&](sim::World& world) {
        const auto before=world.snapshot();
        check(world.construction_cost(sim::CommandType::PlaceHousehold)==sim::Rules::household_cost&&
              world.snapshot()==before,"Historical ordinary House quote changed");
        f::buy(world,{sim::CommandType::PlaceHousehold,{0,2}});
        const auto owner=world.building_owner_at({0,2});
        check(owner&&world.building(*owner).kind==sim::Object::Household&&
              world.treasury()==before.treasury-sim::Rules::household_cost&&
              world.construction_spent_total()==before.construction_spent_total+
                  static_cast<std::uint64_t>(sim::Rules::household_cost),
              "Closing Elite accidentally changed the existing paid ordinary House command");
        refused(world);
    };
    sim::World implicit(f::width,f::height,std::vector<std::uint8_t>(f::width*f::height,1));
    check(implicit.profile()==sim::RulesProfile::LogisticsV1&&implicit.rule_version()==1,
          "Historical refusal test did not use the actual default profile");
    refused(implicit); // Logistics has no ordinary House command; do not invent one.
    const auto legacy_policy=[](std::uint32_t version) {
        return std::make_shared<const sim::MapPermissions>(f::width,f::height,version,
            std::vector<sim::MapCellPermission>(f::width*f::height,
                {true,true,false,0,sim::BuildBlocker::None,sim::BuildBlocker::None}),
            std::vector<sim::FixedGatePassage>{});
    };
    for (std::uint32_t version=1;version<=6;++version) {
        auto world=version<=3 ? sim::World(f::width,f::height,
            std::vector<std::uint8_t>(f::width*f::height,1),sim::RulesProfile::CityV16,version):
            sim::World(legacy_policy(version==4 ? 2U:3U),sim::RulesProfile::CityV16,version);
        check(world.profile()==sim::RulesProfile::CityV16&&world.rule_version()==version,
              "Historical Elite refusal used an unsupported substituted City rule");
        refused(world);ordinary_house(world);
    }
    f::Root historical;
    const auto policy=f::permissions();
    const auto model=f::save::prepare_original_rules_model(historical.data);
    for (const std::uint32_t version:{7U,8U,9U}) {
        auto world=version==7 ? sim::World::create_original_economy_development(policy,model):
            version==8 ? sim::World::create_original_influence_development(policy,model):
                         sim::World::create_original_goods_development(policy,model);
        check(world.rule_version()==version&&!world.original_production_enabled(),
              "Historical development refusal gained active production authority");
        refused(world);ordinary_house(world);
    }
    Fixture current;
    auto active=current.world();const auto before=active.snapshot();
    check(active.construction_cost(elite.type)==80&&active.snapshot()==before,
          "Active Elite quote is not the explicit read-only authored80 price");
    f::buy(active,elite);const auto owner=active.building_owner_at(elite.cell);
    check(owner&&active.building(*owner).kind==sim::Object::OriginalElitePlot&&
          active.treasury()==before.treasury-80&&active.construction_spent_total()==before.construction_spent_total+80&&
          active.buildings().size()==before.buildings.size()+1&&active.couriers().size()==before.couriers.size(),
          "Active Elite purchase lost its exact paid body or granted an actor");
    for (const sim::Cell cell:{elite.cell,sim::Cell{7,2},sim::Cell{6,3},sim::Cell{7,3}})
        check(active.building_owner_at(cell)==owner,"Active authored2x2 Elite cells lost one owner");
    const auto& vacant=house(active,*owner).house;
    check(vacant.type==orig::HouseType::EliteUnoccupied&&!vacant.occupants&&!vacant.water&&!vacant.food_quality&&
          !vacant.hemp&&!vacant.ceramics&&!vacant.tea&&!vacant.silk&&!vacant.bronzeware&&!vacant.lacquerware&&
          !vacant.herbalist&&!vacant.acupuncture&&!vacant.music&&!vacant.acrobat&&!vacant.drama&&
          !vacant.ancestor&&!vacant.confucian&&!vacant.daoist&&!vacant.buddhist,
          "Active paid Elite plot granted population, water, goods, quality or service timers");
    check(active.original_production_state()->housing.houses.size()==1&&
          active.original_production_state()->water.houses.size()==1&&
          active.original_production_state()->food.stocks.size()==1,
          "Active paid Elite body lacks its existing sole housing/water/food authorities");
}
void paid_common_and_elite_lifecycle(const f::fs::path& real) {
    Fixture fixture(real);auto world=fixture.world();const auto paid=oe_production_fixture::add_housing_stages_scene(world);
    check(world.treasury()==12000-static_cast<std::int64_t>(paid.spent)&&
        !world.total_population()&&house(world,paid.elite).house.type==orig::HouseType::EliteUnoccupied,
        "Paid Elite plot created free population/type/funds");
    check(sim::building_footprint(world.profile(),world.rule_version(),sim::Object::OriginalElitePlot)==sim::BuildingFootprint{2,2},
          "OE25 temporary Elite geometry silently claimed later original4x4");
    for (int t=0;t<119;++t) world.tick();
    check(!world.total_population(),"Elite or Common residents arrived before paid age120");
    world.tick();authority(world);const auto& admitted=history(world,paid.elite).first_transitions[10];
    check(admitted&&admitted->kind==sim::OriginalHousingTransitionKind::Admission&&admitted->evaluation.tick==120&&
          admitted->evaluation.input.type==orig::HouseType::EliteUnoccupied&&!admitted->evaluation.input.occupants&&
          admitted->admitted==2&&house(world,paid.elite).house.type==orig::HouseType::ModestSiheyuan&&
          house(world,paid.elite).house.occupants==2,"Actual first Elite event did not bind11->13 with actual2 admissions");
    until(world,[&]{return house(world,paid.elite).house.type==orig::HouseType::EliteAbandoned;},
          "Actually unstocked Modest did not devolve to Abandoned on its50th failure");
    const auto abandoned_population=house(world,paid.elite).house.occupants;
    const auto& down=history(world,paid.elite).first_transitions[9];
    check(down&&down->kind==sim::OriginalHousingTransitionKind::Evolution&&down->evaluation.input.type==orig::HouseType::ModestSiheyuan&&
          down->evaluation.input.devolve_counter==49&&abandoned_population>0,
          "Abandoned was injected or lost actual previously admitted residents");
    for (const auto id:paid.material_sources) f::buy(world,sim::set_building_operation(id,true));
    bool recovered=false;
    const auto complete=[&] {return recovered&&house(world,paid.elite).house.confucian>0&&
        std::any_of(paid.common_houses.begin(),paid.common_houses.end(),[&](auto id) {
            return house(world,id).house.type>orig::HouseType::PlainCottage;
        });};
    for (int t=0;t<40000&&!complete();++t) {
        const auto previous=house(world,paid.elite).house.type;
        world.tick();authority(world);
        if (previous==orig::HouseType::EliteAbandoned&&house(world,paid.elite).house.type==orig::HouseType::ModestSiheyuan) {
            const auto& last=history(world,paid.elite).last_evaluation;
            check(last&&last->tick==world.ticks()&&last->input.type==orig::HouseType::EliteAbandoned&&
                  last->input.hemp>0&&last->input.ceramics>0&&house(world,paid.elite).house.occupants>=abandoned_population,
                  "Elite recovery lacks genuine current and next-row material input or lost residents");
            recovered=true;
        }
    }
    check(complete(),"Paid Millet/Hemp/Ceramics/Paper did not yield Common>5, Elite recovery and actual Confucian arrival");
    check(history(world,paid.elite).first_transitions[10]->kind==sim::OriginalHousingTransitionKind::Admission,
          "Later recovery replaced the first actual13 admission proof");
    const auto& academy=world.original_production_state()->industry.auxiliary;
    const auto paper=std::find_if(academy.begin(),academy.end(),[&](const auto& row){return row.building_id==paid.academy;});
    check(paper!=academy.end()&&paper->received[4]>0&&paper->consumed[4]>0,
          "Elite Confucian visit skipped genuine Paper receipt/debit");
    compare_matrix(world,paid.elite);
    auto replay=sim::World::restore_original_production_development(world.snapshot(),fixture.policy,fixture.model);
    for (int t=0;t<20000;++t) {
        world.tick();replay.tick();
        check(world.snapshot()==replay.snapshot(),"Paid higher-stage full20k Snapshot replay diverged");
    }
    authority(world);
}
void combined_house_bound_and_paid_provider_provenance() {
    Fixture limit_fixture;auto bounded=limit_fixture.world();
    for (int i=0;i<20;++i) {
        const sim::Cell cell{(i%8)*3,1+(i/8)*4};
        f::buy(bounded,{i%2 ? sim::CommandType::PlaceOriginalElitePlot:sim::CommandType::PlaceHousehold,cell});
    }
    check(bounded.original_economy_state()->houses.size()==20&&!bounded.total_population(),
          "Combined twenty-house paid limit granted residents or omitted a family");
    for (const auto command:{sim::Command{sim::CommandType::PlaceHousehold,{12,9}},
                             sim::Command{sim::CommandType::PlaceOriginalElitePlot,{15,9}},
                             sim::Command{sim::CommandType::PlaceOriginalElitePlot,{0,1}}}) {
        const auto before=bounded.snapshot();
        check(!bounded.validate(command).accepted&&!bounded.execute(command).accepted&&bounded.snapshot()==before,
              "Combined21st family purchase or Common replacement was not an atomic refusal");
    }
    for (const auto& row:bounded.original_economy_state()->houses) {
        check(!row.house.occupants&&!row.house.hemp&&!row.house.ceramics&&!row.house.food_quality&&
              !row.house.confucian&&!row.house.ancestor&&!row.house.water,
              "Empty paid plot granted goods/services/quality/water/people");
    }
    Fixture fixture;
    constexpr std::array providers{
        std::tuple{sim::CommandType::PlaceHealthPost,orig::ModelBuilding::HerbalistsStall,sim::Cell{6,5}},
        std::tuple{sim::CommandType::PlaceServicePost,orig::ModelBuilding::AcupuncturistsClinic,sim::Cell{10,5}},
        std::tuple{sim::CommandType::PlaceOriginalMusicSchool,orig::ModelBuilding::MusicSchool,sim::Cell{14,5}},
        std::tuple{sim::CommandType::PlaceOriginalAcrobatSchool,orig::ModelBuilding::AcrobatSchool,sim::Cell{6,9}},
        std::tuple{sim::CommandType::PlaceOriginalDramaSchool,orig::ModelBuilding::DramaSchool,sim::Cell{14,9}},
        std::tuple{sim::CommandType::PlaceOriginalAncestorShrine,orig::ModelBuilding::AncestralShrine,sim::Cell{6,13}},
        std::tuple{sim::CommandType::PlaceOriginalDaoistShrine,orig::ModelBuilding::DaoistShrine,sim::Cell{10,13}},
        std::tuple{sim::CommandType::PlaceOriginalBuddhistShrine,orig::ModelBuilding::BuddhistShrine,sim::Cell{14,13}},
        std::tuple{sim::CommandType::PlaceOriginalConfucianAcademy,orig::ModelBuilding::ConfucianAcademy,sim::Cell{18,9}}};
    auto bytes=oe_production_fixture::authored_model_text(7);
    for (const auto& [command,model,cell]:providers) {
        (void)command;(void)cell;
        for (const auto& [field,value]:std::array{std::pair{1U,100},std::pair{2U,1},std::pair{3U,0},std::pair{4U,10}}) {
            const auto marker=std::to_string(orig::model_id(model).value)+','+std::string(orig::model_label(model))+",{,";
            auto at=bytes.find(marker);check(at!=std::string::npos,"Authored own-provider DES row missing");at+=marker.size();
            for (unsigned i=0;i<field;++i) at=bytes.find(',',at)+1;
            const auto end=bytes.find(',',at);bytes.replace(at,end-at,std::to_string(value));
        }
    }
    f::write_text(fixture.root.data/"Model/EmperorBuildingModels.txt",bytes);
    fixture.model=f::save::prepare_original_production_rules_model(fixture.root.data);
    auto world=fixture.world();
    for (int x=0;x<24;++x) f::buy(world,{sim::CommandType::PlaceRoad,{x,7}});
    f::buy(world,{sim::CommandType::PlaceHousehold,{10,8}});const auto home=*world.building_owner_at({10,8});
    for (int t=0;t<142;++t) world.tick();
    const auto historical=history(world,home).last_evaluation;
    check(historical&&historical->input.desirability==0,"Paid provider historical baseline was not actually evaluated");
    orig::DesirabilityMap independent(24,20);std::vector<sim::BuildingId> ids;
    for (const auto& [command,model,cell]:providers) {
        f::buy(world,{command,cell});const auto id=*world.building_owner_at(cell);ids.push_back(id);
        f::buy(world,sim::set_building_operation(id,false));
        const auto size=sim::building_footprint(world.profile(),world.rule_version(),world.building(id).kind);
        const orig::SpreadSpec spread{fixture.model->building_value(model,orig::BuildingField::Desirability),
            fixture.model->building_value(model,orig::BuildingField::DesirabilityStep),
            fixture.model->building_value(model,orig::BuildingField::DesirabilityStepSize),
            fixture.model->building_value(model,orig::BuildingField::DesirabilityRange)};
        independent.apply_source({{cell.x,cell.y,size.width},spread});
        const auto provenance=world.original_influence_provenance(home);
        check(provenance.sources.size()==ids.size()&&provenance.value==independent.building_value({10,8,2}),
              "Paid providers differ from ordered/saturated original map using their own footprints");
        const auto& own=provenance.sources.back();
        check(own.building_id==id&&own.origin==cell&&own.footprint==size&&
              static_cast<unsigned>(own.type)==orig::model_id(model).value&&own.spread.value==100&&
              own.spread.range==10&&own.after<=100&&
              std::is_sorted(provenance.sources.begin(),provenance.sources.end(),[](const auto& a,const auto& b) {
                  return a.building_id<b.building_id;
              }),"Provider provenance aliased Well metadata, lost stable ordering or ignored saturation");
    }
    check(world.original_influence_state()->sources.empty()&&world.original_production_state()->services.providers.size()==9&&
          world.original_influence_provenance(home).value==100,
          "Provider binding invented a second persisted source authority or missed actual saturation");
    check(history(world,home).last_evaluation==historical,
          "Buying later providers rewrote the actual earlier historical desirability input");
}
} // namespace

int main(int argc,char** argv) {
    try {
        check(argc==1||(argc==3&&std::string_view(argv[1])=="--real"),"Usage: original-housing-stages-world [--real DATA]");
        const auto real=argc==3 ? f::fs::canonical(argv[2]):f::fs::path{};
        all_rows_all_difficulties(real);pure_negative_needs_and_counter_boundaries();historical_elite_price_and_refusal();
        combined_house_bound_and_paid_provider_provenance();paid_common_and_elite_lifecycle(real);
        std::cout<<"Fifteen-row/five-difficulty diagnostics, pure negative/counter matrix and paid Common/Elite lifecycle PASS\n";
        return 0;
    } catch (const std::exception& error) {
        perf::set_enabled(false);std::cerr<<"Original Housing Stages World FAIL: "<<error.what()<<'\n';return 1;
    }
}
