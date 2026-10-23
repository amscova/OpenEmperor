// OE35: legal player experiments and read-only fire/economy diagnosis.
#include "../tests/EconomyFireDiagnosisMetrics.h"
#include "../tests/EconomyFireDiagnosisStrategy.h"

#include <charconv>
#include <iostream>
#include <map>

namespace {
namespace q=oe_starter_fixture;
namespace sim=openemperor::simulation;
namespace player=oe_economy_fire_strategy;
namespace metrics=oe_economy_fire_metrics;
namespace fs=std::filesystem;
using Json=nlohmann::json;
using q::check;
constexpr const char* usage="economy-fire-diagnosis --mode seed|run|resume|check --variant supplied|direct|connected|food|lean6|lean12|compact6|supplied12|managed|staged|staggered12 --data DATA [--output NEW_OE35_DIRECTORY] [--ticks 20000..500000] [--input PRIVATE_OE35_SEED]";
struct Args {std::string mode,variant;fs::path data,output,input;unsigned ticks=60000;};
void safe(const fs::path& path) {
    check(path.is_absolute(),"Diagnosis paths must be absolute");fs::path prefix;
    for(const auto& part:path) {
        check(part!=".."&&part!=".","Diagnosis paths must be canonical");prefix/=part;
        std::error_code error;const auto status=fs::symlink_status(prefix,error);
        check(!error||error==std::errc::no_such_file_or_directory,"Cannot inspect diagnosis path");
        check(!fs::is_symlink(status),"Diagnosis path traverses symlink");
    }
}
bool below(const fs::path& child,const fs::path& parent) {
    auto c=child.begin();for(auto p=parent.begin();p!=parent.end();++p,++c)if(c==child.end()||*c!=*p)return false;return true;
}
Args args(int argc,char** argv) {
    std::map<std::string,std::string> values;
    for(int i=1;i<argc;i+=2) {
        check(i+1<argc,usage);const std::string key=argv[i];
        check(key=="--mode"||key=="--variant"||key=="--data"||key=="--output"||key=="--ticks"||key=="--input",usage);
        check(values.emplace(key,argv[i+1]).second,"Duplicate diagnosis argument");
    }
    for(const auto* name:{"--mode","--variant","--data"})check(values.contains(name),usage);
    Args a;a.mode=values.at("--mode");a.variant=values.at("--variant");
    check(a.mode=="seed"||a.mode=="run"||a.mode=="resume"||a.mode=="check",usage);
    check(std::find(player::variants.begin(),player::variants.end(),a.variant)!=player::variants.end(),usage);
    const auto path=[&](const char* name){return values.contains(name)?fs::absolute(values.at(name)):fs::path{};};
    a.data=path("--data");a.output=path("--output");a.input=path("--input");
    if(values.contains("--ticks")) {
        const auto& text=values.at("--ticks");const auto parsed=std::from_chars(text.data(),text.data()+text.size(),a.ticks);
        check(parsed.ec==std::errc{}&&parsed.ptr==text.data()+text.size()&&a.ticks>=20000&&a.ticks<=500000,usage);
    }
    check(a.mode!="resume"||!a.input.empty(),"Resume needs explicit private seed");
    check((a.mode!="seed"&&a.mode!="check")||a.input.empty(),"Seed/check cannot import a save");
    return a;
}
void preflight(const Args& a) {
    safe(a.data);check(fs::is_directory(a.data),"BLOCKED: original installation missing");
    const auto root=fs::canonical(fs::path(__FILE__).parent_path().parent_path())/".local/original-economy/oe35";
    if(a.mode!="check") {
        safe(a.output);check(a.output!=root&&below(a.output,root)&&!fs::exists(a.output),"Output must be new private OE35 directory");
        check(!below(a.output,a.data)&&!below(a.data,a.output),"Diagnosis output overlaps original data");
        q::save::validate_save_target(a.output/"manifest.json",a.data);
    }
    if(!a.input.empty()) {
        safe(a.input);check(below(a.input,root)&&fs::is_regular_file(a.input)&&!below(a.input,a.output),"Input must be explicit private OE35 seed");
    }
}
void write(const fs::path& p,const Json& data) {check(!fs::exists(p),"Preserve existing diagnosis artifact");q::f::write_text(p,data.dump(2)+"\n");}
void checkpoint(const q::Context& c,const fs::path& output,const char* name,const sim::World& w,const metrics::Observation& observed) {
    const auto path=output/(std::string(name)+".save.json");
    q::save::write_save(path,c.document(w),c.data,c.mask);
    const auto read=q::save::read_save(path);auto restored=c.restore(read);
    check(read.world==w.snapshot()&&restored.snapshot()==w.snapshot(),"Diagnosis full SaveDocument restore differs");
    write(output/(std::string(name)+".snapshot.json"),Json::parse(q::f::read_text(path)).at("world"));
    write(output/(std::string(name)+".report.json"),observed.report(w));
}
void regression(const q::Context& c,sim::Cell origin) {
    auto active=c.world(),paused=c.world();player::build(active,"lean6",origin);player::build(paused,"lean6",origin);
    const auto farm=q::body(paused,sim::Object::OriginalFoodFarm);
    q::execute(paused,sim::set_building_operation(farm,false));
    for(unsigned i=0;i<800;++i){active.tick();paused.tick();}
    q::authority(active);q::authority(paused);
    check(active.maintenance_spent_total()==paused.maintenance_spent_total()&&active.maintenance_spent_total()>0,
        "Pause changed installed-worker upkeep");
    check(q::food_facts(active).produced>0&&q::food_facts(paused).produced==0,"Pause probe did not isolate real Food production");
    auto world=c.world();player::build(world,"supplied",origin);
    oe_paid_progression_strategy::fire_refusal(world,origin);
    while(world.ticks()<816)world.tick();
    check(world.original_goal_status().reached,"OE34 actual goal no longer reached at816");
    metrics::Observation observed(world);
    while(world.ticks()<21600){observed.before(world);world.tick();observed.after(world);}
    const auto report=observed.report(world);const auto& summary=report.at("summary");
    check(summary.at("funds_at_goal_plus_20000")==290&&summary.at("first_negative_tick")==21600&&world.treasury()==-30,
        "Real unchanged OE34 capital exhaustion moved; inspect rule/model contract");
    check(summary.at("goal_loss_ticks").get<unsigned>()>=4800&&summary.at("upkeep_while_burning")==2970,
        "Diagnosis hid real fire retention/upkeep effects");
    const auto document=c.document(world);auto restored=c.restore(document);
    for(unsigned i=0;i<400;++i){world.tick();restored.tick();}
    check(world.snapshot()==restored.snapshot(),"Negative-Funds complete save continuation differs");
    std::cout<<"Actual paid goal, fire refusal, installed paused upkeep,21600t debt and full negative-Funds continuation PASS\n";
}
void run(const Args& a) {
    preflight(a);const auto c=q::Context::real(a.data);
    check(c.model->provenance().sha256=="7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6",
        "BLOCKED: diagnosis requires pinned unchanged Normal model");
    // Only obtain the production starter's legally searched origin; each measured
    // variant begins in its own fresh World and pays its own complete recipe.
    auto reference=c.world();const auto receipt=sim::place_original_economy_starter(reference);
    q::paid_empty(c,reference,receipt);const auto origin=receipt.origin;
    if(a.mode=="check"){regression(c,origin);return;}
    const bool resume=a.mode=="resume";const auto input=a.input.empty()?std::string{}:q::f::read_text(a.input);
    auto world=resume?c.restore(q::save::read_save(a.input)):c.world();Json initial=Json::array();
    std::unique_ptr<oe_paid_progression_metrics::Metrics> warmup;
    if(resume)check(world.ticks()==816&&world.snapshot()==q::save::read_save(a.input).world,"Diagnosis seed restore differs");
    else {
        initial=player::build(world,a.variant,origin).initial_commands;
        if(a.mode=="seed")warmup=std::make_unique<oe_paid_progression_metrics::Metrics>(world);
        while(world.ticks()<816) {
            const auto actions=player::controls(world,a.variant,origin);for(const auto& row:actions)initial.push_back(row);
            if(warmup)warmup->before(world);world.tick();if(warmup)warmup->after(world);
        }
    }
    q::authority(world);q::worker_oracle(world);fs::create_directories(a.output);
    metrics::Observation observed(world);
    if(a.mode=="seed") {
        checkpoint(c,a.output,"seed",world,observed);
        write(a.output/"warmup.report.json",warmup->report(world));
        write(a.output/"warmup.transport.json",warmup->transport_events());
        write(a.output/"warmup.evaluations.json",warmup->events());
    }
    if(a.mode=="run"&&!a.input.empty()) {
        check(world.snapshot()==q::save::read_save(a.input).world,"Fresh paid variant differs from protected seed");
        q::save::write_save(a.output/"creator-reference.save.json",c.document(world),c.data,c.mask);
        check(q::f::read_text(a.output/"creator-reference.save.json")==input,"Fresh paid variant literal seed differs");
    }
    const auto begin=world.ticks();
    if(a.mode!="seed")for(unsigned elapsed=1;elapsed<=a.ticks;++elapsed) {
        observed.commands(player::controls(world,a.variant,origin),world);
        observed.before(world);world.tick();observed.after(world);
        if(elapsed==a.ticks/2)checkpoint(c,a.output,"midpoint",world,observed);
    }
    if(a.mode!="seed")checkpoint(c,a.output,"result",world,observed);
    if(!a.input.empty())check(q::f::read_text(a.input)==input,"Diagnosis rewrote supplied seed");
    write(a.output/"manifest.json",{{"format","openemperor-economy-fire-process-v1"},{"status","PASS"},
        {"mode",a.mode},{"variant",a.variant},{"begin_tick",a.mode=="seed"?0:begin},{"end_tick",world.ticks()},
        {"ticks",a.mode=="seed"?816:world.ticks()-begin},{"creator_mode",!resume},{"restored_live_world",resume},
        {"protected_reference_compared",a.mode=="run"&&!a.input.empty()},{"input_preserved",true},
        {"rule",10},{"schema",26},{"policy",3},{"revision",27},{"original_map",true},{"original_model",true},
        {"snapshot_injection",false},{"initial_commands",initial},{"origin",{origin.x,origin.y}},
        {"summary",observed.report(world).at("summary")},{"native_human","NOT_RUN"}});
    std::cout<<"Economy/fire diagnosis "<<a.variant<<" "<<a.mode<<" technical PASS "<<observed.report(world).at("summary").dump()<<'\n';
}
}
int main(int argc,char** argv) {
    if(argc==2&&std::string_view(argv[1])=="--help"){std::cout<<usage<<'\n';return 0;}
    try{run(args(argc,argv));return 0;}catch(const std::exception& e){std::cerr<<"Economy/fire diagnosis FAIL: "<<e.what()<<'\n';return 1;}
}
