#include "simulation/World.h"
#include "simulation/RoadConnectivity.h"
#include <iostream>
#include <stdexcept>
using namespace openemperor::simulation;
namespace {
void check(bool ok,const char* reason){if(!ok)throw std::runtime_error(reason);}
std::shared_ptr<const MapPermissions> permissions(int width,int height,int river_x,std::uint32_t version=3) {
    std::vector<MapCellPermission> cells(std::size_t(width*height),{true,true,false,0,BuildBlocker::None,BuildBlocker::None});
    for(int y=0;y<height;++y){auto& p=cells[std::size_t(y*width+river_x)];p.road_allowed=p.building_allowed=false;p.bridge_water=version==3;}
    return std::make_shared<const MapPermissions>(width,height,version,std::move(cells),std::vector<FixedGatePassage>{});
}
void rejected(World& w,Cell a,Cell b) {
    const auto before=w.snapshot();check(!w.execute(place_bridge(a,b)).accepted,"invalid span accepted");
    check(w.snapshot()==before,"failed span partially mutated authoritative state");
}
}
int main(){try {
    auto p=permissions(7,5,3);World w(p,RulesProfile::CityV16,5);
    check(w.execute({CommandType::PlaceRoad,{2,2}}).accepted,"bank A road rejected");
    check(w.execute({CommandType::PlaceRoad,{4,2}}).accepted,"bank B road rejected");
    check(!w.transport_cell({3,2}),"unbuilt water became traversable");
    rejected(w,{2,2},{4,3});rejected(w,{2,2},{2,2});rejected(w,{2,2},{3,2});
    rejected(w,{-1,2},{4,2});rejected(w,{2,2},{INT32_MAX,2});
    rejected(w,{1,2},{4,2});rejected(w,{2,2},{5,2});
    const auto before=w.snapshot();const auto refresh=w.route_refresh_count();
    const auto r=w.execute(place_bridge({4,2},{2,2}));
    check(r.accepted,"REGRESSION: complete own bridge command rejected; water crossing unavailable");
    check(w.transport_cell({3,2})&&w.transport_edge_allowed({2,2},{3,2})&&w.transport_edge_allowed({3,2},{4,2}),"bridge longitudinal edges missing");
    check(!w.transport_edge_allowed({3,2},{3,1}),"bridge permitted a lateral edge");
    check(w.treasury()==before.treasury-60&&w.construction_spent_total()==before.construction_spent_total+60,
        "bridge price not exactly once");
    check(w.road_revision()==before.road_revision+1&&w.route_refresh_count()==refresh+1,"bridge did not refresh topology exactly once");
    check(w.bridges().size()==1&&w.bridges()[0].id.value==1&&w.bridges()[0].bank_a==Cell{2,2},"span not canonical or independently typed");
    check(w.transport_source({3,2})==TransportCellSource::SandboxBridge&&!w.road_cell({3,2}),"deck confused with an ordinary land road");
    check(w.city_economy_valid(),"bridge spending not reconciled");
    rejected(w,{2,2},{4,2});
    const auto built=w.snapshot();check(!w.execute({CommandType::RemoveRoad,{3,2}}).accepted&&w.snapshot()==built,"individual deck cell removed");
    check(!w.execute({CommandType::RemoveBridge,{3,2}}).accepted&&w.snapshot()==built,"V1 unsafe bridge removal accepted");
    check(World::restore(built,p).snapshot()==built,"bridge restore lost state");
    for(int variant=0;variant<7;++variant) {
        auto bad=built;
        switch(variant) {
        case 0:bad.bridges[0].water_cells.clear();break;
        case 1:bad.bridges[0].deck_height=1;break;
        case 2:bad.bridges[0].paid_cost=0;break;
        case 3:bad.bridges[0].id.value=2;break;
        case 4:bad.bridges.push_back(bad.bridges[0]);bad.next_bridge_id=3;break;
        case 5:bad.bridges[0].axis=BridgeAxis::StorageY;break;
        case 6:bad.bridges[0].water_cells[0]={3,3};break;
        }
        bool failed=false;try{(void)World::restore(bad,p);}catch(const std::exception&){failed=true;}
        check(failed,"manipulated bridge snapshot accepted");
    }
    World old(permissions(7,5,3,2),RulesProfile::CityV16,4);
    check(old.execute({CommandType::PlaceRoad,{2,2}}).accepted&&old.execute({CommandType::PlaceRoad,{4,2}}).accepted,"legacy banks failed");
    rejected(old,{2,2},{4,2});check(!old.transport_cell({3,2}),"rule 4 water negative weakened");
    {
        auto cells=p->cells();cells[2U*7+4].height=1;
        auto uneven=std::make_shared<const MapPermissions>(7,5,3,cells,std::vector<FixedGatePassage>{});
        World height(uneven,RulesProfile::CityV16,5);check(height.execute({CommandType::PlaceRoad,{2,2}}).accepted&&height.execute({CommandType::PlaceRoad,{4,2}}).accepted,"unequal bank roads failed");
        rejected(height,{2,2},{4,2});
    }
    {
        auto policy=permissions(50,50,25);World poor(policy,RulesProfile::CityV16,5);
        check(poor.execute({CommandType::PlaceRoad,{24,10}}).accepted&&poor.execute({CommandType::PlaceRoad,{26,10}}).accepted,"poor bank roads failed");
        for(int y=0;y<50&&poor.treasury()>=60;++y)for(int x=0;x<50&&poor.treasury()>=60;++x)
            if(x!=25&&!poor.road_cell({x,y}))check(poor.execute({CommandType::PlaceRoad,{x,y}}).accepted,"ordinary paid budget drain failed");
        const auto preview=poor.validate_bridge({24,10},{26,10});check(preview.geometry_valid&&!preview.accepted&&preview.span.paid_cost==60&&preview.diagnostic.blocker==BuildBlocker::InsufficientFunds,"amber affordable geometry lost exact price");
        rejected(poor,{24,10},{26,10});
    }
    for(const auto axis:{BridgeAxis::StorageX,BridgeAxis::StorageY}) {
        constexpr int size=15;std::vector<MapCellPermission> cells(size*size,{true,true,false,-1,BuildBlocker::None,BuildBlocker::None});
        const Cell a=axis==BridgeAxis::StorageX?Cell{2,2}:Cell{2,2},b=axis==BridgeAxis::StorageX?Cell{11,2}:Cell{2,11};
        const Cell delta=axis==BridgeAxis::StorageX?Cell{1,0}:Cell{0,1};
        for(int i=1;i<=8;++i){auto& c=cells[std::size_t((a.y+i*delta.y)*size+a.x+i*delta.x)];c.road_allowed=c.building_allowed=false;c.height=std::int8_t(i%2?127:-128);c.bridge_water=true;}
        auto& bank=cells[std::size_t(a.y*size+a.x)];bank.original_road=true;bank.original_road_classification=OriginalRoadClassification::GroundRoad;bank.building_allowed=false;bank.building_blocker=BuildBlocker::ProtectedOriginalRoad;
        auto policy=std::make_shared<const MapPermissions>(size,size,3,cells,std::vector<FixedGatePassage>{});World maxspan(policy,RulesProfile::CityV16,5);
        check(maxspan.execute({CommandType::PlaceRoad,b}).accepted&&maxspan.execute(place_bridge(b,a)).accepted,"eight-cell/reversed/original-bank span rejected");
        check(maxspan.bridges()[0].water_cells.size()==8&&maxspan.bridges()[0].paid_cost==200&&maxspan.bridges()[0].deck_height==-1,"eight-cell deck/cost failed");
        Cell previous=a;for(const auto cell:maxspan.bridges()[0].water_cells){check(maxspan.transport_edge_allowed(previous,cell)&&maxspan.transport_height(cell)==-1,"water signed height replaced bridge deck height or blocked edge");previous=cell;}
        check(maxspan.transport_edge_allowed(previous,b)&&World::restore(maxspan.snapshot(),policy).snapshot()==maxspan.snapshot(),"long-span signed deck restore failed");
        rejected(maxspan,{2,2},axis==BridgeAxis::StorageX?Cell{12,2}:Cell{2,12});
    }
    {
        World limit(p,RulesProfile::CityV16,5);check(limit.execute({CommandType::PlaceRoad,{2,2}}).accepted&&limit.execute({CommandType::PlaceRoad,{4,2}}).accepted,"counter fixture banks failed");
        auto snapshot=limit.snapshot();snapshot.command_sequence=UINT64_MAX-1;limit=World::restore(snapshot,p);rejected(limit,{2,2},{4,2});
    }

    // Real paid Clay -> Pottery trip and return, through the same BFS and
    // movement state machine; no snapshot goods/workers/cargo injection.
    auto traffic_policy=permissions(16,12,7);World traffic(traffic_policy,RulesProfile::CityV16,5);
    for(const auto command:std::array<Command,4>{{{CommandType::PlaceHousehold,{1,1}},
        {CommandType::PlaceHousehold,{1,3}},{CommandType::PlaceClaySource,{4,2}},
        {CommandType::PlacePottery,{9,2}}}})check(traffic.execute(command).accepted,"paid traffic building failed");
    for(int x=3;x<=10;++x)if(x!=7)check(traffic.execute({CommandType::PlaceRoad,{x,5}}).accepted,"traffic road failed");
    for(const Cell cell:{Cell{5,4},Cell{9,4}})check(traffic.execute({CommandType::PlaceRoad,cell}).accepted,"traffic entrance road failed");
    check(!traffic.find_building_route(static_cast<BuildingId>(3),static_cast<BuildingId>(4)),"pre-bridge alternate route existed");
    check(traffic.execute(place_bridge({6,5},{8,5})).accepted,"paid traffic bridge failed");
    const auto diagnosis=diagnose_road_connectivity(traffic,static_cast<BuildingId>(3),static_cast<BuildingId>(4));
    check(diagnosis.category==RoadConnectivityCategory::Connected&&diagnosis.bridges_on_route.size()==1&&diagnosis.bridges_on_route[0].id.value==1,
        "explicit shared RoadConnectivity lost bridge source/provenance");
    bool outbound=false,returning=false,delivery=false,recipe=false;
    for(int tick=0;tick<400;++tick) {
        traffic.tick();check(traffic.navigation_valid()&&traffic.production_balance_valid()&&traffic.city_economy_valid(),"traffic invariant failed");
        const auto& courier=traffic.couriers()[0];
        if(courier.phase!=CourierPhase::IdleAtWorkshop&&traffic.bridge_at(courier.path.at(courier.path_vertex))) {
            outbound|=courier.phase==CourierPhase::ToWarehouse;returning|=courier.phase==CourierPhase::Returning;
            check(World::restore(traffic.snapshot(),traffic_policy).snapshot()==traffic.snapshot(),"on-deck restore failed");
        }
        const auto& pot=traffic.building(static_cast<BuildingId>(4));delivery|=pot.input_clay>0;recipe|=pot.recipes_completed>0;
    }
    check(outbound&&returning&&delivery&&recipe,"actual bridge cargo/delivery/recipe/return missing");
    std::cout<<"Bridge transport PASS\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
