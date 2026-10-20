// Excluded OE32 process fixture. All positive states are earned by ordinary
// commands and ticks; only explicit private OE32 saves can be supplied.
#include "OriginalLongplayMetrics.h"

#include <charconv>
#include <iostream>
#include <map>
#include <optional>
#include <string_view>

namespace {
namespace f=oe_longplay_fixture;
namespace q=oe_starter_fixture;
namespace sim=openemperor::simulation;
namespace fs=std::filesystem;
using Json=nlohmann::json;
using q::check;

const char* usage="Usage: original-longplay-acceptance --mode seed|run|resume --scenario starter|districts --data DATA --output NEW_OE32_DIRECTORY [--ticks 1..100000] [--input PRIVATE_OE32_SAVE]";
struct Arguments {
    std::string mode,scenario;
    fs::path data,output,input;
    unsigned ticks=20000;
};
bool below(const fs::path& child,const fs::path& parent) {
    auto c=child.begin();for(auto p=parent.begin();p!=parent.end();++p,++c)if(c==child.end()||*c!=*p)return false;
    return true;
}
void safe_components(const fs::path& path) {
    fs::path prefix;
    for(const auto& component:fs::absolute(path)) {
        check(component!="."&&component!="..","Longplay paths must be canonical");prefix/=component;
        std::error_code error;const auto status=fs::symlink_status(prefix,error);
        check(!error||error==std::errc::no_such_file_or_directory,"Cannot inspect longplay proof path");
        check(!fs::is_symlink(status),"Longplay proof path traverses a symlink");
    }
}
Arguments arguments(int argc,char** argv) {
    std::map<std::string,std::string> values;
    for(int i=1;i<argc;i+=2) {
        check(i+1<argc,usage);const std::string key=argv[i];
        check(key=="--mode"||key=="--scenario"||key=="--data"||key=="--output"||key=="--ticks"||key=="--input",usage);
        check(values.emplace(key,argv[i+1]).second,"Duplicate longplay argument");
    }
    for(const auto* name:{"--mode","--scenario","--data","--output"})check(values.contains(name),usage);
    Arguments result;result.mode=values.at("--mode");result.scenario=values.at("--scenario");
    check(result.mode=="seed"||result.mode=="run"||result.mode=="resume",usage);
    check(result.scenario=="starter"||result.scenario=="districts",usage);
    if(values.contains("--ticks")) {
        const auto& raw=values.at("--ticks");const auto converted=std::from_chars(raw.data(),raw.data()+raw.size(),result.ticks);
        check(converted.ec==std::errc{}&&converted.ptr==raw.data()+raw.size()&&result.ticks>0&&result.ticks<=100000,
            "Longplay ticks must be in1..100000");
    }
    result.data=fs::absolute(values.at("--data"));result.output=fs::absolute(values.at("--output"));
    if(values.contains("--input"))result.input=fs::absolute(values.at("--input"));
    check(result.mode!="seed"||result.input.empty(),"Seed mode cannot read a supplied save");
    check(result.mode!="resume"||!result.input.empty(),"Resume mode requires an explicit private input");
    return result;
}
fs::path private_root() {return fs::canonical(fs::path(__FILE__).parent_path().parent_path())/".local/original-economy/oe32";}
void preflight(const Arguments& args) {
    const auto allowed=private_root();safe_components(args.output);safe_components(args.data);
    check(fs::is_directory(args.data),"BLOCKED: original installation is missing");
    const auto data=fs::canonical(args.data),out=fs::weakly_canonical(args.output);
    check(out!=allowed&&below(out,allowed)&&!below(out,data)&&!below(data,out),
        "Longplay output must be a private OE32 directory disjoint from original data");
    check(!fs::exists(out),"Preserve earlier evidence: longplay output already exists");
    check(fs::is_regular_file(data/"Model/EmperorBuildingModels.txt")&&fs::is_regular_file(data/"Cities/Xia.map"),
        "BLOCKED: mandatory original Model/Xia input is missing");
    q::save::validate_save_target(out/"manifest.json",data);
    if(!args.input.empty()) {
        safe_components(args.input);const auto input=fs::canonical(args.input);
        check(below(input,allowed)&&fs::is_regular_file(input)&&!below(input,out),
            "Longplay input must be an existing private OE32 save");
        check(!below(input,data),"Original data cannot be a longplay save input");
    }
}
void write(const fs::path& path,const Json& value) {
    safe_components(path);check(!fs::exists(path),"Preserve existing longplay artifact");q::f::write_text(path,value.dump(2)+"\n");
}
void controls(sim::World& world,oe_longplay_metrics::Metrics& metrics) {
    const auto phase=world.ticks()%5000;
    if(phase!=1000&&phase!=1120&&phase!=3000&&phase!=3180)return;
    const auto target=std::find_if(world.buildings().begin(),world.buildings().end(),[](const auto& b){return b.kind==sim::Object::Market;});
    check(target!=world.buildings().end(),"Paid longplay scene has no actual Market for ordinary staffing controls");
    sim::Command command;
    if(phase==1000||phase==1120) {
        const auto priority=phase==1000?sim::WorkforcePriority::High:sim::WorkforcePriority::Normal;
        if(target->workforce_priority==priority)return;
        command=sim::set_building_workforce_priority(target->id,priority);
    } else {
        const bool enabled=phase==3180;if(target->operating_enabled==enabled)return;
        command=sim::set_building_operation(target->id,enabled);
    }
    const auto funds=world.treasury();const auto before=world.command_sequence();
    const auto result=world.execute(command);check(result.accepted&&result.changed,"Ordinary longplay staffing control refused: "+result.reason);
    check(world.treasury()==funds&&world.command_sequence()==before+1,"Ordinary staffing control changed funds or command accounting");metrics.command();
}
void tick(sim::World& world,oe_longplay_metrics::Metrics& metrics,const f::Scene* scene) {
    controls(world,metrics);metrics.before(world);q::finance::tick(world);f::check_authority(world);metrics.after(world);
    if(scene)f::assert_independent(world,*scene);
}
void checkpoint(const q::Context& context,const fs::path& output,const char* name,const sim::World& world,
                const oe_longplay_metrics::Metrics& metrics) {
    const auto path=output/(std::string(name)+".save.json");check(!fs::exists(path),"Longplay save output already exists");
    const auto document=context.document(world);q::save::write_save(path,document,context.data,context.mask);
    const auto read=q::save::read_save(path);auto restored=context.restore(read);
    check(read.world==world.snapshot()&&restored.snapshot()==world.snapshot(),"Longplay full SaveDocument/snapshot roundtrip differs");
    const auto raw=Json::parse(q::f::read_text(path));write(output/(std::string(name)+".snapshot.json"),raw.at("world"));
    write(output/(std::string(name)+".report.json"),metrics.report(world));
    write(output/(std::string(name)+".resources.json"),metrics.resources(world));
}
void run(const Arguments& args) {
    preflight(args);
    const auto context=args.scenario=="starter"?q::Context::real(args.data):f::real_district_context(args.data);
    const auto input_bytes=args.input.empty()?std::string{}:q::f::read_text(args.input);
    const bool restored_live_world=args.mode=="resume";
    auto world=restored_live_world?context.restore(q::save::read_save(args.input)):context.world();
    if(restored_live_world)check(world.snapshot()==q::save::read_save(args.input).world,"Separate process changed supplied complete snapshot");
    std::optional<f::Scene> scene;
    if(!restored_live_world) {
        if(args.scenario=="starter") {
            const auto receipt=sim::place_original_economy_starter(world);q::paid_empty(context,world,receipt);
        }else scene=f::build(context,world);
    }else if(args.scenario=="districts")scene=f::classify(world);
    f::check_authority(world);if(scene)f::assert_independent(world,*scene);
    fs::create_directories(args.output);
    if(!context.original_model)q::f::write_text(args.output/"authored-model.txt",q::f::read_text(context.data/"Model/EmperorBuildingModels.txt"));
    const auto warmup_begin=world.ticks();
    if(!restored_live_world) {
        oe_longplay_metrics::Metrics warmup(world);
        for(unsigned i=0;i<2000;++i)tick(world,warmup,scene?&*scene:nullptr);
        check(world.total_population()>0,"Paid longplay seed never admitted real residents");
        if(args.mode=="seed")checkpoint(context,args.output,"seed",world,warmup);
    }
    auto begin=world.ticks();
    if(args.mode=="run"&&!args.input.empty()) {
        // The supplied save is a protected reference only. This live World
        // was freshly bought and ticked; it is never replaced by a restore.
        check(world.snapshot()==q::save::read_save(args.input).world,"Fresh earned creator snapshot differs from protected seed");
        const auto reference=args.output/"creator-reference.save.json";
        q::save::write_save(reference,context.document(world),context.data,context.mask);
        check(q::f::read_text(reference)==input_bytes,"Fresh earned creator complete SaveDocument differs from protected seed bytes");
    }
    if(args.mode!="seed") {
        oe_longplay_metrics::Metrics metrics(world);
        for(unsigned elapsed=1;elapsed<=args.ticks;++elapsed) {
            tick(world,metrics,scene?&*scene:nullptr);
            if(elapsed==std::max(1U,args.ticks/2))checkpoint(context,args.output,"midpoint",world,metrics);
        }
        checkpoint(context,args.output,"result",world,metrics);
    }else begin=warmup_begin;
    if(!args.input.empty())check(q::f::read_text(args.input)==input_bytes,"Separate longplay process rewrote supplied save bytes");
    Json manifest{{"format","openemperor-original-longplay-process-v1"},{"status","PASS"},{"mode",args.mode},{"scenario",args.scenario},
        {"begin_tick",begin},{"end_tick",world.ticks()},{"ticks",world.ticks()-begin},{"original_map",context.original_map},
        {"original_model",context.original_model},{"model_sha256",context.model->provenance().sha256},
        {"map",context.map_relative.generic_string()},{"authored_geography",!context.original_map},
        {"snapshot_injection",false},{"health_authority","neutral; Original illness unsupported"},
        {"creator_mode",!restored_live_world},{"restored_live_world",restored_live_world},
        {"protected_reference_compared",args.mode=="run"&&!args.input.empty()},
        {"warmup_ticks",restored_live_world?0:2000},
        {"rule",10},{"schema",26},{"policy",3},{"revision",27},{"budget_format",1},
        {"input_preserved",true},{"native_human","NOT_RUN"},{"observer_is_performance_benchmark",false}};
    if(scene)manifest["districts"]=f::describe(context,world,*scene);
    write(args.output/"manifest.json",manifest);
    std::cout<<"Original longplay "<<args.scenario<<" "<<args.mode<<" PASS; ticks="<<world.ticks()-begin<<"; native/human NOT_RUN\n";
}
}
int main(int argc,char** argv) {
    if(argc==2&&std::string_view(argv[1])=="--help") {std::cout<<usage<<'\n';return 0;}
    try {run(arguments(argc,argv));return 0;}
    catch(const std::exception& error) {std::cerr<<"Original longplay FAIL: "<<error.what()<<'\n';return 1;}
}
