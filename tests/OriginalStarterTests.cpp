#include "OriginalStarterFixture.h"
#include "core/PerformanceDiagnostics.h"
#include <iostream>

namespace {
namespace q=oe_starter_fixture;
namespace sim=q::sim;
namespace perf=openemperor::performance;
void refused(sim::World& world,const std::string& reason) {
    const auto before=world.snapshot();bool rejected=false;perf::set_enabled(true);perf::reset();
    try {(void)sim::place_original_economy_starter(world);}
    catch(const std::exception& error){rejected=std::string(error.what()).find(reason)!=std::string::npos;}
    for(const auto counter:{perf::Counter::WorldExecutes,perf::Counter::SimulationTicks,perf::Counter::WorldRestores,
        perf::Counter::FileReads,perf::Counter::FileWrites,perf::Counter::AssetDecodes,perf::Counter::TextureUploads})
        q::check(perf::counter(counter)==0,"Rejected starter performed commands/ticks/restore/asset/file work");
    perf::set_enabled(false);q::check(rejected&&world.snapshot()==before,"Rejected starter changed candidate World or omitted its precise reason");
}
void negatives() {
    const auto context=q::Context::authored();
    sim::World legacy(24,20,std::vector<std::uint8_t>(480,1));refused(legacy,"fresh Rule10");
    auto world=context.world();world.tick();refused(world,"fresh Rule10");
    world=context.world();q::execute(world,{sim::CommandType::PlaceRoad,{0,0}});refused(world,"fresh Rule10");
    world=context.world();const auto receipt=sim::place_original_economy_starter(world);q::paid_empty(context,world,receipt);refused(world,"fresh Rule10");
    for(unsigned variant=0;variant<4;++variant) {
        const int width=variant==0?23:24,height=13;
        std::vector<sim::MapCellPermission> cells(static_cast<std::size_t>(width*height),
            {true,true,false,0,sim::BuildBlocker::None,sim::BuildBlocker::None});
        if(variant==1)cells[0]={false,false,true,0,sim::BuildBlocker::OriginalStructure,sim::BuildBlocker::OriginalStructure};
        if(variant==2)for(int y=0;y<height;++y)for(int x=0;x<width;++x)cells[static_cast<std::size_t>(y*width+x)].height=static_cast<std::int8_t>((x+y)%2);
        if(variant==3)for(auto& cell:cells)cell.building_allowed=false;
        const auto policy=std::make_shared<const sim::MapPermissions>(width,height,3,std::move(cells),std::vector<sim::FixedGatePassage>{});
        auto blocked=sim::World::create_original_production_development(policy,context.model);refused(blocked,"No legal empty");
    }
    std::cout<<"Original starter authored negative profiles/freshness/area/protection/signed heights PASS\n";
}
void real(const q::fs::path& installation) {
    const auto context=q::Context::real(installation);auto world=context.world();
    const auto receipt=sim::place_original_economy_starter(world);q::paid_empty(context,world,receipt);
    q::f::Root temporary;const auto output=temporary.path;
    q::equal_roundtrip(context,world,output/"paid.save.json");q::admission_and_priorities(world,receipt);
    q::equal_roundtrip(context,world,output/"scarce-workers.save.json");
    q::upgrade(world,receipt);q::equal_roundtrip(context,world,output/"supplied.save.json");
    const auto proof=q::operations(world,receipt,[&](const char* name,const sim::World& state){q::equal_roundtrip(context,state,output/(std::string(name)+".save.json"));});
    auto replay=context.restore(context.document(world));
    for(unsigned i=0;i<256;++i) {q::tick(world);q::tick(replay);q::worker_oracle(world);q::worker_oracle(replay);
        q::check(world.snapshot()==replay.snapshot(),"Actual original starter full Snapshot/worker replay diverged");}
    q::equal_roundtrip(context,world,output/"final.save.json");
    std::cout<<"Original starter real Xia/Normal1404, four paid admissions/up/down/recovery, both goods consumed, actual visits, road safety/repair, pause/upkeep/priority/demolition and exact offline saves PASS "<<proof.dump()<<'\n';
}
}
int main(int argc,char** argv) {
    try {
        q::check(argc>=2,"Expected --synthetic or --real DATA");
        if(argc==2&&std::string(argv[1])=="--synthetic")negatives();
        else {q::check(argc==3&&std::string(argv[1])=="--real","Expected --real DATA");real(argv[2]);}
        return 0;
    }catch(const std::exception& error){perf::set_enabled(false);std::cerr<<error.what()<<'\n';return 1;}
}
