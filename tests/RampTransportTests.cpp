#include "simulation/World.h"
#include "simulation/RoadConnectivity.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace openemperor::simulation;
namespace {
void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
std::shared_ptr<const MapPermissions> policy(std::uint32_t version=3) {
    std::vector<MapCellPermission> cells(16U*12U,{true,true,false,0,BuildBlocker::None,BuildBlocker::None});
    for(int y=0;y<12;++y)for(int x=8;x<16;++x)cells[std::size_t(y*16+x)].height=1;
    return std::make_shared<const MapPermissions>(16,12,version,cells,std::vector<FixedGatePassage>{});
}
}
int main(){try {
    for(const auto version:{4U,5U}) {
        auto p=policy(version==4?2:3);World old(p,RulesProfile::CityV16,version);
        check(old.execute({CommandType::PlaceRoad,{7,5}}).accepted&&old.execute({CommandType::PlaceRoad,{8,5}}).accepted,"old roads failed");
        check(old.transport_edge_blocker({7,5},{8,5})==BuildBlocker::UnsupportedHeightTransition,"old height negative weakened");
    }
    auto p=policy();World ramp(p,RulesProfile::CityV16,6);
    check(ramp.execute({CommandType::PlaceRoad,{7,5}}).accepted&&ramp.execute({CommandType::PlaceRoad,{8,5}}).accepted,"rule6 roads failed");
    check(ramp.transport_edge_blocker({7,5},{8,5})==BuildBlocker::UnsupportedHeightTransition,"unbuilt height edge opened");
    const auto before=ramp.snapshot();const auto refresh=ramp.route_refresh_count();
    check(ramp.execute(place_ramp({8,5},{7,5})).accepted,"paid ramp rejected");
    check(ramp.ramps().size()==1&&ramp.ramps()[0].endpoint_a==Cell{7,5},"canonical edge lost");
    check(ramp.treasury()==before.treasury-40&&ramp.construction_spent_total()==before.construction_spent_total+40&&
          ramp.command_sequence()==before.command_sequence+1&&ramp.road_revision()==before.road_revision+1&&
          ramp.route_refresh_count()==refresh+1,"ramp paid/revised/refreshed more than once");
    check(p->transport_edge_blocker({7,5},{8,5})==BuildBlocker::UnsupportedHeightTransition,"immutable policy changed");
    check(ramp.transport_edge_allowed({7,5},{8,5})&&ramp.transport_edge_allowed({8,5},{7,5}),"ramp edge not symmetric");
    check(ramp.transport_edge_blocker({7,4},{8,4})==BuildBlocker::UnsupportedHeightTransition,"other height edge freed");
    for(int step=0;step<=5;++step)check(std::abs(*ramp.ramp_surface_height({7+step/5.0,5})-step/5.0)<1e-9,"ramp surface not continuous");
    const auto stable=ramp.snapshot();
    check(!ramp.execute(place_ramp({7,5},{8,5})).accepted&&ramp.snapshot()==stable,"duplicate mutated world");
    check(!ramp.execute({CommandType::RemoveRamp,{7,5}}).accepted&&ramp.snapshot()==stable,"unsafe ramp demolition enabled");
    check(World::restore(stable,p).snapshot()==stable,"ramp restore changed world");
    check(ramp.execute({CommandType::RemoveRoad,{7,5}}).accepted&&!ramp.transport_edge_allowed({7,5},{8,5}),"deleted endpoint still traversable");
    check(World::restore(ramp.snapshot(),p).snapshot()==ramp.snapshot(),"legal disconnected ramp not saveable");
    check(!ramp.execute({CommandType::PlaceHousehold,{6,4}}).accepted,"building occupied reserved endpoint");
    check(ramp.execute({CommandType::PlaceRoad,{7,5}}).accepted&&ramp.transport_edge_allowed({7,5},{8,5}),"repair failed");
    for(int variant=0;variant<13;++variant) {
        auto bad=stable;auto& r=bad.ramps[0];
        switch(variant) {
        case 0:r.id.value=0;break;case 1:r.id.value=2;break;case 2:bad.next_ramp_id=1;break;
        case 3:std::swap(r.endpoint_a,r.endpoint_b);break;case 4:r.endpoint_b={8,6};break;
        case 5:r.height_b=2;break;case 6:r.height_a=-128;r.height_b=127;break;
        case 7:r.paid_cost=0;break;case 8:r.placed_tick=bad.ticks+1;break;
        case 9:bad.ramps.push_back(r);bad.next_ramp_id=3;break;
        case 10:r.endpoint_a={-1,5};break;case 11:bad.construction_spent_total--;break;
        case 12:bad.rule_version=5;break;
        }
        bool failed=false;try{(void)World::restore(bad,p);}catch(const std::exception&){failed=true;}
        check(failed,"malformed ramp authority accepted");
    }
    const std::vector<std::pair<Cell,Cell>> invalid={{{7,5},{7,5}},{{7,5},{8,6}},{{7,5},{9,5}},
        {{-1,5},{0,5}},{{7,5},{INT32_MAX,5}},{{6,5},{7,5}},{{7,5},{7,6}}};
    for(const auto& ends:invalid) {
        const auto snapshot=ramp.snapshot();check(!ramp.execute(place_ramp(ends.first,ends.second)).accepted&&ramp.snapshot()==snapshot,"invalid ramp partially published");
    }
    for(const int gradient:{-1,1})for(const bool axis_x:{false,true})for(const int base:{-128,-1,0,126}) {
        const int high=base+gradient;if(high<-128||high>127)continue;
        auto cells=p->cells();const Cell a{3,3},b=axis_x?Cell{4,3}:Cell{3,4};
        cells[std::size_t(a.y*16+a.x)].height=std::int8_t(base);cells[std::size_t(b.y*16+b.x)].height=std::int8_t(high);
        auto q=std::make_shared<const MapPermissions>(16,12,3,cells,std::vector<FixedGatePassage>{});World v(q,RulesProfile::CityV16,6);
        check(v.execute({CommandType::PlaceRoad,a}).accepted&&v.execute({CommandType::PlaceRoad,b}).accepted&&v.execute(place_ramp(a,b)).accepted,"signed X/Y gradient rejected");
        check(World::restore(v.snapshot(),q).snapshot()==v.snapshot(),"signed extreme not preserved");
    }
    // Conservative immutable endpoint exclusions and failed-command atomicity.
    for(int negative=0;negative<8;++negative) {
        auto cells=p->cells();auto& end=cells[5U*16U+8U];
        switch(negative) {
        case 0:end.height=0;break;case 1:end.height=2;break;
        case 2:end.road_allowed=end.building_allowed=false;end.bridge_water=true;break;
        case 3:end.road_allowed=end.building_allowed=false;end.protected_original=true;end.road_blocker=end.building_blocker=BuildBlocker::OriginalStructure;break;
        case 4:end.road_allowed=end.building_allowed=false;end.road_blocker=end.building_blocker=BuildBlocker::OutsideMap;break;
        case 5:end.road_allowed=false;end.road_blocker=BuildBlocker::UnsupportedTerrain;break;
        case 6:end.road_allowed=false;end.original_road_classification=OriginalRoadClassification::WaterOrBridge;end.road_blocker=BuildBlocker::UnsupportedOriginalRoad;break;
        case 7:end.height=-128;cells[5U*16U+7U].height=127;break;
        }
        auto q=std::make_shared<const MapPermissions>(16,12,3,cells,std::vector<FixedGatePassage>{});World v(q,RulesProfile::CityV16,6);
        (void)v.execute({CommandType::PlaceRoad,{7,5}});(void)v.execute({CommandType::PlaceRoad,{8,5}});
        const auto snapshot=v.snapshot();check(!v.execute(place_ramp({7,5},{8,5})).accepted&&v.snapshot()==snapshot,"unsafe terrain/height ramp accepted");
    }
    {
        World poor(p,RulesProfile::CityV16,6);check(poor.execute({CommandType::PlaceRoad,{7,5}}).accepted&&poor.execute({CommandType::PlaceRoad,{8,5}}).accepted,"poor endpoint setup");
        for(int y=0;y<12&&poor.treasury()>=40;++y)for(int x=0;x<16&&poor.treasury()>=40;++x)
            if(!poor.road_cell({x,y}))(void)poor.execute({CommandType::PlaceRoad,{x,y}});
        // The bounded 192-cell fixture cannot spend all 1300 on roads; paid
        // buildings drain the rest through real commands, without field injection.
        for(int y=0;y<4&&poor.treasury()>=40;y+=2)for(int x=0;x<16&&poor.treasury()>=40;x+=2) {
            const Cell c{x,y};
            for(int dy=0;dy<2;++dy)for(int dx=0;dx<2;++dx)(void)poor.execute({CommandType::RemoveRoad,{c.x+dx,c.y+dy}});
            (void)poor.execute({CommandType::PlaceHousehold,c});
        }
        check(poor.treasury()<40,"paid budget drain did not reach insufficient funds");
        {const auto snapshot=poor.snapshot();const auto v=poor.validate_ramp({7,5},{8,5});
            check(v.geometry_valid&&!v.accepted&&v.diagnostic.blocker==BuildBlocker::InsufficientFunds,"funds-only preview lost valid geometry");
            check(!poor.execute(place_ramp({7,5},{8,5})).accepted&&poor.snapshot()==snapshot,"poor ramp cost or partially mutated");}
    }
    // Real paid production and a combined ramp/bridge in the shared graph.
    // Authored permissions are separate from original-map evidence.
    for(const bool bridge:{false,true}) {
        constexpr int width=24,height=16;std::vector<MapCellPermission> cells(width*height,{true,true,false,0,BuildBlocker::None,BuildBlocker::None});
        for(int y=0;y<height;++y)for(int x=0;x<8;++x)cells[std::size_t(y*width+x)].height=1;
        if(bridge)for(int y=0;y<height;++y){auto& c=cells[std::size_t(y*width+11)];c.road_allowed=c.building_allowed=false;c.bridge_water=true;}
        auto q=std::make_shared<const MapPermissions>(width,height,3,cells,std::vector<FixedGatePassage>{});World v(q,RulesProfile::CityV16,6);
        for(const Command c:std::array<Command,4>{{{CommandType::PlaceHousehold,{1,1}},{CommandType::PlaceHousehold,{1,3}},
            {CommandType::PlaceClaySource,{4,2}},{CommandType::PlacePottery,{14,2}}}})check(v.execute(c).accepted,"paid production setup failed");
        for(int x=4;x<=15;++x)if(!bridge||x!=11)check(v.execute({CommandType::PlaceRoad,{x,5}}).accepted,"paid production road failed");
        for(const Cell end:{Cell{5,4},Cell{14,4}})check(v.execute({CommandType::PlaceRoad,end}).accepted,"entrance failed");
        if(bridge)check(v.execute(place_bridge({10,5},{12,5})).accepted,"rule6 failed inherited bridge");
        const auto source=static_cast<BuildingId>(3),target=static_cast<BuildingId>(4);
        check(!v.find_building_route(source,target),"height connected before ramp");
        check(v.execute(place_ramp({7,5},{8,5})).accepted,"paid transport ramp failed");
        const auto diagnosis=diagnose_road_connectivity(v,source,target);
        check(diagnosis.category==RoadConnectivityCategory::Connected&&diagnosis.ramps_on_route.size()==1&&
            diagnosis.bridges_on_route.size()==std::size_t(bridge),"shared diagnostics failed");
        bool out=false,back=false,delivery=false;std::size_t crossings=0;
        for(int tick=0;tick<12000;++tick) {
            v.tick();check(v.navigation_valid()&&v.production_balance_valid()&&v.city_economy_valid(),"ramp transport conservation failed");
            const auto& c=v.couriers()[0];
            if(c.path_vertex+1<c.path.size()&&v.ramp_between(c.path[c.path_vertex],c.path[c.path_vertex+1])) {
                ++crossings;out|=c.phase==CourierPhase::ToWarehouse;back|=c.phase==CourierPhase::Returning;
                check(World::restore(v.snapshot(),q).snapshot()==v.snapshot(),"mid-ramp progress/cargo restore changed");
                auto control=World::restore(v.snapshot(),q);for(int i=0;i<10;++i)control.tick();
                auto reload=World::restore(v.snapshot(),q);for(int i=0;i<10;++i)reload.tick();
                check(control.snapshot()==reload.snapshot(),"ramp continuation nondeterministic");
            }
            delivery|=v.building(target).active_recipe_clay>0 || v.pottery_completed_total()>0;
        }
        if(!(out&&back&&delivery&&crossings>0&&v.pottery_completed_total()>0))std::cerr<<"out="<<out<<" back="<<back<<" delivered="<<delivery<<" crossing ticks="<<crossings<<" recipes="<<v.pottery_completed_total()<<" bridge="<<bridge<<"\n";
        check(out&&back&&delivery&&crossings>0&&v.pottery_completed_total()>0,"actual climb/delivery/return/endurance missing");
    }
    return 0;
}catch(const std::exception& e){std::cerr<<"RAMP REGRESSION: "<<e.what()<<'\n';return 1;}}
