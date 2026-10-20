#include "app/OriginalHouseInspection.h"
#include "OriginalFinanceFixture.h"
#include "core/PerformanceDiagnostics.h"

#include <algorithm>
#include <array>
#include <climits>
#include <iostream>
#include <string_view>

namespace {
namespace ui=openemperor::original_house_ui;
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
namespace perf=openemperor::performance;
namespace q=oe_finance_fixture;
namespace f=oe_influence_fixture;
using f::check;

bool contains(const std::vector<std::string>& lines,std::string_view text) {
    return std::any_of(lines.begin(),lines.end(),[&](const auto& line){return line.find(text)!=std::string::npos;});
}
void schedule_and_display_boundaries() {
    const auto first=ui::upcoming_checks(0);
    check(first.water==29 && first.countdown==36 && first.housing==40 && first.consumption==40,
          "Inspector changed the existing next-dispatch phase interpretation");
    const auto processed=ui::upcoming_checks(40);
    check(processed.housing==91 && processed.consumption==397 && processed.countdown==87 && processed.water==80,
          "Inspector repeated an already completed check or invented monthly consumption");
    for (const auto tick:{orig::scheduler_max_tick,UINT64_MAX}) {
        const auto end=ui::upcoming_checks(tick);
        check(!end.housing && !end.countdown && !end.water && !end.consumption,
              "Terminal scheduler observation wrapped into a future check");
    }
    check(ui::relative_need_percentage(1,4)==25 && ui::relative_need_percentage(0,4)==0 &&
          ui::relative_need_percentage(INT32_MAX,INT32_MAX)==100 && ui::relative_need_percentage(INT32_MAX,1)==100 &&
          !ui::relative_need_percentage(0,0) && !ui::relative_need_percentage(0,-1),
          "Relative display percentage overflowed or labeled a nonpositive requirement");
    sim::World legacy(4,4,std::vector<std::uint8_t>(16,1));
    check(!ui::inspect_house(legacy,static_cast<sim::BuildingId>(1)).applicable &&
          !ui::overlay_observation(legacy,static_cast<sim::BuildingId>(1),ui::Overlay::Supply).applicable,
          "Original inspector acquired a legacy World capability");
}
void paid_geometry_visits_and_purity(const f::fs::path& original) {
    q::Fixture fixture(original);auto world=fixture.world();const auto scene=q::build(world,true);
    const auto common=scene.commons.front();check(scene.elite.has_value(),"Inspector scene lost paid Elite");
    const auto empty=ui::inspect_house(world,*scene.elite);
    check(empty.applicable && empty.type==orig::HouseType::EliteUnoccupied && empty.footprint==sim::BuildingFootprint{2,2} &&
          empty.housing.awaiting_admission && empty.overlays[0].status==ui::Status::Unknown,
          "Inspector pretended an empty paid Plot was an inhabited4x4 House");
    const auto common_empty=ui::inspect_house(world,common);
    check(common_empty.visits[2].provider_count==1 && common_empty.visits[2].delivered_timer==0 &&
          !common_empty.visits[2].last_arrival_tick &&
          contains(common_empty.sections[3],"Ancestor actual timer 0") && contains(common_empty.sections[3],"Source existence never grants"),
          "An existing but never-visiting provider acquired a served indication");
    q::through(world,120);
    const auto materialized=ui::inspect_house(world,*scene.elite);
    check(materialized.footprint==sim::BuildingFootprint{4,4} && materialized.type==orig::HouseType::ModestSiheyuan &&
          materialized.origin==world.building(*scene.elite).cell,
          "Inspector or overlay lost actual paid materialized4x4 geometry");
    for (const auto id:{common,*scene.elite}) {
        const auto observation=ui::inspect_house(world,id);const auto diagnosis=world.original_housing_diagnosis(id);
        check(observation.housing==diagnosis && observation.sections[0].size()>2 &&
              observation.sections[0][2].starts_with("Blocker:"),"Inspector hides or changes the authoritative housing blocker");
        check(contains(observation.sections[0],"Tax model rate ") && contains(observation.sections[0],"House tax paid total "+
              std::to_string(world.original_house_tax_status(id).paid_total)) && contains(observation.sections[0],"Next tax slot "),
              "New housing State hid the existing actual per-House finance diagnosis");
        for (unsigned section=0;section<4;++section) {
            const auto key=static_cast<ui::Section>(section);const auto scoped=ui::inspect_house(world,id,key);
            check(scoped.sections[section]==observation.sections[section] && ui::section_lines(world,id,key)==observation.sections[section],
                  "Selected inspector section differs from the bounded complete observation");
            for (unsigned other=0;other<4;++other) if (other!=section)
                check(scoped.sections[other].empty(),"A selected section formatted all other sections");
            check(!observation.sections[section].empty() && observation.sections[section].size()<=ui::section_line_limit,
                  "Inspector section omitted data or exceeded its explicit line bound");
        }
        check(contains(observation.sections[1],"Presence uses actual YES/NO") && contains(observation.sections[1],"relative need"),
              "Inspector conflated binary visits and numeric relative requirement percentages");
        for (unsigned mode=1;mode<=3;++mode) {
            const auto overlay=ui::overlay_observation(world,id,static_cast<ui::Overlay>(mode));
            check(overlay==observation.overlays[mode-1] && overlay.footprint==observation.footprint && overlay.origin==observation.origin,
                  "Lightweight overlay and selected House inspector disagree");
        }
    }
    const auto before=world.snapshot();const auto routes=world.route_refresh_count();
    const auto cache=world.original_influence_cache_build_count();perf::set_enabled(true);perf::reset();
    for (unsigned frame=0;frame<100;++frame) {
        for (const auto section:{ui::Section::State,ui::Section::Needs,ui::Section::Goods,ui::Section::Visits})
            (void)ui::section_lines(world,common,section);
        for (const auto mode:{ui::Overlay::Off,ui::Overlay::Supply,ui::Overlay::Desirability,ui::Overlay::Risk})
            for (const auto id:{common,*scene.elite}) (void)ui::overlay_observation(world,id,mode);
    }
    for (std::size_t counter=0;counter<static_cast<std::size_t>(perf::Counter::Count);++counter)
        check(perf::counter(static_cast<perf::Counter>(counter))==0,"Pure inspector frames performed World/navigation/file/asset work");
    perf::set_enabled(false);
    check(world.snapshot()==before && world.route_refresh_count()==routes && world.original_influence_cache_build_count()==cache,
          "Pure inspection changed complete authority or rebuilt routes/influence");
    bool visited=false;
    for (unsigned tick=0;tick<3000 && !visited;++tick) {
        world.tick();const auto observation=ui::inspect_house(world,common,ui::Section::Visits);
        visited=observation.visits[2].delivered_timer>0 && observation.visits[2].last_arrival_tick.has_value();
    }
    check(visited,"Paid Ancestor source never produced a real visited-house inspection case");
}
void actual_goods_and_food_conservation() {
    q::Fixture fixture;auto world=fixture.world();const auto scene=oe_production_fixture::add_housing_stages_scene(world,false);
    bool transit=false,delivered=false,consumed=false,food=false,pruned=false;
    for (unsigned tick=0;tick<6000 && !(transit && delivered && consumed && food && pruned);++tick) {
        world.tick();
        for (const auto& home:world.original_economy_state()->houses) {
            const auto observation=ui::inspect_house(world,home.building_id,ui::Section::Goods);
            for (std::size_t g=0;g<observation.goods.size();++g) {
                const auto& value=observation.goods[g];
                check(value.delivered==static_cast<std::uint64_t>(value.stock)+value.consumed && value.city.conserved,
                      "Inspector invented or conflated House delivery, consumption and city production");
                check(value.in_transit<=value.incoming_reserved,"Actual inbound cargo escaped its House space commitment");
                transit|=value.in_transit>0;delivered|=value.delivered>0;consumed|=value.consumed>0;
                std::uint64_t retained=0;
                for (const auto& trip:world.original_goods_state()->reservations) if (trip.target==home.building_id && trip.good==value.good) {
                    check(trip.delivered>=0,"Retained delivery evidence has a negative amount");
                    retained+=static_cast<std::uint64_t>(trip.delivered);
                }
                pruned|=value.delivered>retained;
            }
            for (std::size_t crop=0;crop<2;++crop) {
                const auto& value=observation.food;
                check(value.received[crop]+value.returned[crop]==static_cast<std::uint64_t>(value.stock[crop])+value.departed[crop]+value.consumed[crop],
                      "Food inspection broke the actual per-owner stock/receipt/departure/consumption equation");
                food|=value.received[crop]>0 && value.consumed[crop]>0 && value.quality_witness.has_value();
            }
        }
    }
    check(transit && delivered && consumed && food && pruned,
          "Paid House inspection lacks real cargo, delivery, consumption, Food witness or pruned-history evidence");
    check(world.building(scene.elite).kind==sim::Object::OriginalEliteHouse,
          "Goods observation changed actual materialized Elite ownership");
}
}
int main(int argc,char** argv) {
    try {
        f::fs::path original;
        if (argc==3 && std::string_view(argv[1])=="--real") original=f::fs::canonical(argv[2]);
        else check(argc==1,"Usage: original-house-inspection [--real DATA]");
        schedule_and_display_boundaries();paid_geometry_visits_and_purity(original);actual_goods_and_food_conservation();
        std::cout<<"Original House inspection PASS: real paid empty/4x4 geometry, visit absence/arrival, scoped conserved flows, "
                    "pruned delivery history, exact scheduler slots and100 pure frames\n";
        return 0;
    } catch (const std::exception& error) {std::cerr<<"Original House inspection FAIL: "<<error.what()<<'\n';return 1;}
}
