#include "OriginalLongplayFixture.h"

#include <iostream>
#include <string_view>

namespace {
namespace f=oe_longplay_fixture;
namespace sim=f::sim;

void run(const f::Context& context) {
    auto world=context.world();const auto scene=f::build(context,world);
    f::check_authority(world);f::check_workforce(world,true);
    const auto empty=world.snapshot();
    const auto budget=f::budgets(world);
    f::check(budget.at("buildings")==82 && budget.at("couriers")==58,
             "Ordinary paid purchases did not reach exact Rule10 capacity");
    f::check(!world.total_population() && !f::delivered(world,scene)[0] && !f::delivered(world,scene)[1],
             "Full-capacity tick0 fixture already has residents or delivered supply");
    // Use otherwise legal spare ground and a kind still below its own four-body
    // limit so the global capacity guard, rather than geometry/per-kind guard,
    // is the observed rejection. A neutral influence source has a separate path.
    const auto origin=scene.origins[0];
    for (const auto type:{sim::CommandType::PlaceClaySource,sim::CommandType::PlaceOriginalInfluenceSource}) {
        const auto before=world.snapshot();const auto result=world.execute({type,{origin.x+12,origin.y+18}});
        f::check(!result.accepted && !result.changed && world.snapshot()==before,
                 "Over-capacity ordinary purchase was not an atomic refusal");
        const std::string expected=type==sim::CommandType::PlaceClaySource ?
            "Original production building capacity reached":"Original production building collection limit reached";
        f::check(result.reason==expected,
                 "Additional body was refused for a reason other than actual global capacity: "+result.reason);
    }
    f::check(world.snapshot()==empty,"Capacity diagnosis mutated the paid tick0 city");
    auto restored=context.restore(context.document(world));
    f::check(restored.snapshot()==world.snapshot(),"Paid full-capacity initial restore differs");
    const auto refreshes=world.route_refresh_count(),influence=world.original_influence_cache_build_count();
    bool moving=false,scarce=false;
    for (unsigned tick=0;tick<1200;++tick) {
        world.tick();restored.tick();
        f::check(world.snapshot()==restored.snapshot(),"Paid full-capacity deterministic short replay differs");
        f::check_authority(world);
        scarce=scarce || (world.total_population()>0 && world.workforce_used()<world.active_workforce_required());
        for (const auto& actor:world.couriers())
            moving=moving || (actor.phase!=sim::CourierPhase::IdleAtWorkshop && actor.edge_progress>0);
        if ((tick+1)%200==0) {f::assert_independent(world,scene);(void)f::budgets(world);f::check_workforce(world,true);}
    }
    f::check(moving && scarce,"Short scale run did not exercise actual movement and scarce staffing");
    const auto arrivals=f::delivered(world,scene);
    f::check(arrivals[0]>0 && arrivals[1]>0,"Both independent districts did not receive any actual Food/Aux delivery");
    f::check(world.route_refresh_count()==refreshes && world.original_influence_cache_build_count()==influence,
             "Fixed common-house city rebuilt future route/influence caches during normal ticks");
    f::assert_independent(world,scene);const auto final=f::budgets(world);
    f::check(final.at("cache_slots")==budget.at("cache_slots"),"Fixed city grew its role-cache slot collection");
    const auto document=context.document(world);auto final_restore=context.restore(document);
    f::check(final_restore.snapshot()==world.snapshot(),"Earned full-capacity SaveDocument restore differs");
    std::cout<<f::describe(context,world,scene).dump(2)<<'\n';
}
}

int main(int argc,char** argv) {
    try {
        f::check((argc==2 && std::string_view(argv[1])=="--synthetic") ||
                 (argc==3 && std::string_view(argv[1])=="--real"),
                 "Usage: original-longplay-tests --synthetic | --real DATA");
        const auto context=argc==3?f::real_district_context(argv[2]):f::authored_context();
        run(context);std::cout<<"Original paid82/58 two-district1200t replay, real arrivals, budgets and conservation PASS\n";return 0;
    } catch (const std::exception& error) {
        std::cerr<<"Original Longplay FAIL: "<<error.what()<<'\n';return 1;
    }
}
