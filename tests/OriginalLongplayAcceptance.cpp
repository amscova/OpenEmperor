// Excluded OE32/OE34 process fixture. All positive states are earned by ordinary
// commands and ticks; only explicit private OE32 saves can be supplied.
#include "OriginalLongplayMetrics.h"
#include "PaidProgressionMetrics.h"
#include "PaidProgressionStrategy.h"

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

const char* usage="Usage: original-longplay-acceptance --mode seed|run|resume --scenario starter|districts|progression --data DATA --output NEW_PRIVATE_DIRECTORY [--ticks 1..100000] [--input PRIVATE_OE32_SAVE]";
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
    check(result.scenario=="starter"||result.scenario=="districts"||result.scenario=="progression",usage);
    if(values.contains("--ticks")) {
        const auto& raw=values.at("--ticks");const auto converted=std::from_chars(raw.data(),raw.data()+raw.size(),result.ticks);
        check(converted.ec==std::errc{}&&converted.ptr==raw.data()+raw.size()&&result.ticks>0&&result.ticks<=100000,
            "Longplay ticks must be in1..100000");
    }
    check(result.scenario!="progression"||result.ticks>=20000,"Paid progression requires at least20000 ticks after the actual goal");
    result.data=fs::absolute(values.at("--data"));result.output=fs::absolute(values.at("--output"));
    if(values.contains("--input"))result.input=fs::absolute(values.at("--input"));
    check(result.mode!="seed"||result.input.empty(),"Seed mode cannot read a supplied save");
    check(result.mode!="resume"||!result.input.empty(),"Resume mode requires an explicit private input");
    return result;
}
fs::path private_root(std::string_view scenario) {
    return fs::canonical(fs::path(__FILE__).parent_path().parent_path())/
        (scenario=="progression"?".local/original-economy/oe34":".local/original-economy/oe32");
}
void preflight(const Arguments& args) {
    const auto allowed=private_root(args.scenario);safe_components(args.output);safe_components(args.data);
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
                const oe_longplay_metrics::Metrics& metrics,const Json& progression=nullptr) {
    const auto path=output/(std::string(name)+".save.json");check(!fs::exists(path),"Longplay save output already exists");
    const auto document=context.document(world);q::save::write_save(path,document,context.data,context.mask);
    const auto read=q::save::read_save(path);auto restored=context.restore(read);
    check(read.world==world.snapshot()&&restored.snapshot()==world.snapshot(),"Longplay full SaveDocument/snapshot roundtrip differs");
    const auto raw=Json::parse(q::f::read_text(path));write(output/(std::string(name)+".snapshot.json"),raw.at("world"));
    auto report=metrics.report(world);if(!progression.is_null())report["progression"]=progression;
    write(output/(std::string(name)+".report.json"),report);
    write(output/(std::string(name)+".resources.json"),metrics.resources(world));
}
// The strategy and goal query are shared with the existing OE34 View fixture.
// Goal success is never reconstructed from individual thresholds in this test.
namespace player=oe_paid_progression_strategy;
namespace detail=oe_paid_progression_metrics;
struct GoalObservation {
    Json first_goal,changes=Json::array(),samples=Json::array();
    std::uint64_t lost=0,streak=0,longest=0,previous_taxes=0,previous_upkeep=0;
    bool previous=true;
    explicit GoalObservation(const sim::World& world):first_goal(player::goal(world)),
        previous_taxes(world.taxes_collected_total()),previous_upkeep(world.maintenance_spent_total()) {
        check(world.original_goal_status().reached,"Paid progression requires the productive World goal");
        sample_frame(world);
    }
    void sample_frame(const sim::World& world) {
        auto row=player::goal(world);Json houses=Json::array(),burning=Json::array();
        for(const auto& home:world.original_economy_state()->houses)houses.push_back({
            {"id",static_cast<unsigned>(home.building_id)},{"type",static_cast<unsigned>(home.house.type)},
            {"occupants",home.house.occupants},{"burning",world.building_on_fire(home.building_id)}});
        for(const auto& body:world.buildings())if(world.building_on_fire(body.id))burning.push_back(static_cast<unsigned>(body.id));
        row["houses"]=houses;row["burning_ids"]=burning;samples.push_back(std::move(row));
    }
    void after(const sim::World& world) {
        const auto current=player::goal(world);const bool reached=world.original_goal_status().reached;
        if(reached){++streak;longest=std::max(longest,streak);}else{++lost;streak=0;}
        const bool changed=reached!=previous;
        if(changed)changes.push_back(current);
        if(changed||world.taxes_collected_total()!=previous_taxes||world.maintenance_spent_total()!=previous_upkeep)sample_frame(world);
        previous=reached;previous_taxes=world.taxes_collected_total();previous_upkeep=world.maintenance_spent_total();
    }
    Json report(const sim::World& world) const {
        const auto taxes=world.taxes_collected_total()-first_goal.at("taxes").get<std::uint64_t>();
        const auto upkeep=world.maintenance_spent_total()-first_goal.at("maintenance").get<std::uint64_t>();
        check(taxes<=static_cast<std::uint64_t>(INT64_MAX)&&upkeep<=static_cast<std::uint64_t>(INT64_MAX),"Observed finance delta exceeds signed reporting domain");
        const auto balance=static_cast<std::int64_t>(taxes)-static_cast<std::int64_t>(upkeep);
        return {{"goal_reached",first_goal.at("reached")},{"goal_status","PASS"},
            {"first_goal_tick",first_goal.at("tick")},{"first_goal",first_goal},{"final_goal",player::goal(world)},
            {"stability_status",lost||balance<0?"PARTIAL":"PASS"},{"goal_loss_ticks",lost},
            {"longest_goal_streak",longest},{"goal_changes",changes},{"financial_samples",samples},
            {"operating_tax_delta",taxes},{"operating_maintenance_delta",upkeep},{"operating_balance",balance},
            {"taxes_cover_upkeep",taxes>=upkeep},{"goal_predicate","World::original_goal_status().reached"}};
    }
};
void progression_tick(sim::World& world,oe_longplay_metrics::Metrics& measured,detail::Metrics& detailed) {
    measured.before(world);detailed.before(world);world.tick();f::check_authority(world);
    measured.after(world);detailed.after(world);
}
void detailed_checkpoint(const fs::path& output,const char* name,const sim::World& world,const detail::Metrics& detailed) {
    const auto stem=std::string(name);
    write(output/(stem+".evaluations.json"),detailed.events());
    write(output/(stem+".evaluation-frames.json"),detailed.evaluation_frames());
    write(output/(stem+".transport.json"),detailed.transport_events());
    write(output/(stem+".detailed.json"),detailed.report(world));
}
void progression_run(const Arguments& args) {
    const auto context=q::Context::real(args.data);
    check(context.model->provenance().sha256=="7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6",
        "BLOCKED: progression requires the unchanged original Normal model");
    const bool resumed=args.mode=="resume";const auto input=args.input.empty()?std::string{}:q::f::read_text(args.input);
    auto world=resumed?context.restore(q::save::read_save(args.input)):context.world();
    Json commands=Json::array(),refusal=nullptr;
    std::uint64_t warmup=0;
    if(resumed)check(world.snapshot()==q::save::read_save(args.input).world,"Progression restore changed the complete supplied snapshot");
    else {
        const auto receipt=sim::place_original_economy_starter(world);q::paid_empty(context,world,receipt);
        commands=player::expand(world,receipt,"supplied");refusal=player::fire_refusal(world,receipt.origin);
        check(world.construction_spent_total()==1670&&world.command_sequence()==97,"Ordinary paid progression receipt changed");
    }
    f::check_authority(world);fs::create_directories(args.output);
    if(!resumed) {
        oe_longplay_metrics::Metrics measured(world);detail::Metrics detailed(world);
        while(!world.original_goal_status().reached&&world.ticks()<20000)progression_tick(world,measured,detailed);
        warmup=world.ticks();
        if(!world.original_goal_status().reached) {
            write(args.output/"first-blocker.json",{{"status","FAIL"},{"reason","Productive World goal not reached within20000 normal ticks"},
                {"actual_goal",player::goal(world)},{"metrics",measured.report(world)}});
            throw std::runtime_error("Productive World goal not reached; inspect first-blocker.json");
        }
        if(args.mode=="seed") {
            GoalObservation goal(world);checkpoint(context,args.output,"seed",world,measured,goal.report(world));
            detailed_checkpoint(args.output,"seed",world,detailed);
        }
    }
    check(world.original_goal_status().reached,"Goal success requires the production World predicate");
    GoalObservation observation(world);const auto begin=world.ticks();
    if(args.mode=="run"&&!args.input.empty()) {
        check(world.snapshot()==q::save::read_save(args.input).world,"Fresh paid creator differs from protected goal seed");
        q::save::write_save(args.output/"creator-reference.save.json",context.document(world),context.data,context.mask);
        check(q::f::read_text(args.output/"creator-reference.save.json")==input,"Unrestored creator goal document differs from protected seed bytes");
    }
    if(args.mode!="seed") {
        oe_longplay_metrics::Metrics measured(world);detail::Metrics detailed(world);
        for(unsigned elapsed=1;elapsed<=args.ticks;++elapsed) {
            progression_tick(world,measured,detailed);observation.after(world);
            if(elapsed==args.ticks/2) {
                checkpoint(context,args.output,"midpoint",world,measured,observation.report(world));
                detailed_checkpoint(args.output,"midpoint",world,detailed);
            }
        }
        checkpoint(context,args.output,"result",world,measured,observation.report(world));
        detailed_checkpoint(args.output,"result",world,detailed);
    }
    if(!args.input.empty())check(q::f::read_text(args.input)==input,"Progression process rewrote its supplied goal save");
    Json manifest{{"format","openemperor-original-longplay-process-v1"},{"status","PASS"},{"mode",args.mode},{"scenario",args.scenario},
        {"begin_tick",args.mode=="seed"?0:begin},{"end_tick",world.ticks()},{"ticks",args.mode=="seed"?warmup:world.ticks()-begin},
        {"original_map",true},{"original_model",true},{"model_sha256",context.model->provenance().sha256},
        {"map",context.map_relative.generic_string()},{"authored_geography",false},{"snapshot_injection",false},
        {"health_authority","neutral; Original illness unsupported"},{"creator_mode",!resumed},{"restored_live_world",resumed},
        {"protected_reference_compared",args.mode=="run"&&!args.input.empty()},{"warmup_ticks",warmup},
        {"rule",10},{"schema",26},{"policy",3},{"revision",27},{"budget_format",1},{"input_preserved",true},
        {"native_human","NOT_RUN"},{"observer_is_performance_benchmark",false},{"progression",observation.report(world)},
        {"strategy","supplied paid player commands; no legacy pause/priorities"},{"expansion_commands",commands},{"fire_watch_control",refusal}};
    write(args.output/"manifest.json",manifest);
    std::cout<<"Original longplay progression goal PASS; first_tick="<<begin<<"; additional_ticks="<<world.ticks()-begin
        <<"; stability="<<manifest["progression"]["stability_status"]<<"; native/human NOT_RUN\n";
}

void run(const Arguments& args) {
    preflight(args);
    if(args.scenario=="progression"){progression_run(args);return;}
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
