// The OE37 baseline protocol is deliberately reused without another simulator.
// Only this executable's explicit private evidence directory differs.
#define OE_SUSTAINABLE_PROOF_DIRECTORY "oe38"
#define main oe37_baseline_main
#include "SustainableEconomyAcceptance.cpp"
#undef main
#undef OE_SUSTAINABLE_PROOF_DIRECTORY

#include "../tests/HighTierProgressionMatrix.h"
#include "../tests/HighTierProgressionStrategy.h"
#include "../tests/HighTierProgressionMetrics.h"
#include "../tests/HighTierDesirabilityBound.h"

namespace oe38_cli {
constexpr const char* matrix_usage="high-tier-progression-acceptance --mode matrix --data ABSOLUTE --output NEW_PRIVATE_OE38_DIRECTORY [--input EARNED_OE38_SAVE]";

int matrix(int argc,char** argv) {
    std::map<std::string,std::string> args;
    for(int i=1;i<argc;i+=2) {
        check(i+1<argc,matrix_usage);const std::string key=argv[i];
        check(key=="--mode"||key=="--data"||key=="--output"||key=="--input",matrix_usage);
        check(args.emplace(key,argv[i+1]).second,"Duplicate matrix argument");
    }
    check(args.contains("--mode")&&args.at("--mode")=="matrix"&&args.contains("--data")&&args.contains("--output"),matrix_usage);
    const auto data=fs::absolute(args.at("--data")),output=fs::absolute(args.at("--output"));
    safe(data);safe(output);check(fs::is_directory(data),"Original installation missing");
    check(output!=private_root()&&below(output,private_root())&&!fs::exists(output),"Matrix requires a new private OE38 directory");
    check(!below(output,data)&&!below(data,output),"Matrix output overlaps original installation");
    q::save::validate_save_target(output/"matrix.json",data);
    const auto c=q::Context::real(data);
    check(c.model->provenance().sha256=="7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6","Unchanged Normal model required");
    const auto model=oe::original::load_model_data(data);check(bool(model),"Independent original ModelData preparation failed");
    const oe::original::HouseEvolutionRules independent(*model.model,oe::original::Difficulty::Normal);
    std::optional<fs::path> input;std::string input_bytes;
    if(args.contains("--input")) {
        input=fs::absolute(args.at("--input"));safe(*input);
        check(below(*input,private_root())&&fs::is_regular_file(*input)&&!below(*input,output),"Explicit earned OE38 input required");
        input_bytes=q::f::read_text(*input);
    }
    auto w=input?q::save::restore_original_sustainable_economy(q::save::read_save(*input),c.data,c.mask,c.policy,c.model):
        sim::World::create_original_sustainable_economy(c.policy,c.model);
    const auto origin=player::origin(c);Json commands=Json::array();
    if(!input)commands=player::build(w,"balanced-three",origin);
    // A paid empty plot provides a real separate-family observer. It does not
    // receive an admission, a type promotion or any House resources here.
    const sim::Command plot{sim::CommandType::PlaceOriginalElitePlot,{origin.x+18,origin.y}};
    const auto funds=w.treasury();const auto spent=w.construction_spent_total();
    const auto planned=w.validate(plot);check(planned.accepted,"Matrix Elite plot refused: "+planned.reason);
    const auto bought=w.execute(plot);check(bought.accepted&&bought.changed,"Matrix paid Elite purchase failed");
    commands.push_back({{"label","ElitePlot"},{"cell",{plot.cell.x,plot.cell.y}},{"tick",w.ticks()},
        {"cost",w.construction_spent_total()-spent},{"funds_before",funds},{"funds_after",w.treasury()}});
    const auto before=w.snapshot();auto report=oe_high_tier_matrix::report(w,independent);
    check(w.snapshot()==before,"Read-only matrix changed World authority");q::authority(w);
    report["preparation_commands"]=commands;
    report["snapshot_injection"]=false;report["query_world_unchanged"]=true;
    report["observations_are_post_tick"]=true;report["high_tier_success_claim"]=false;
    report["elite_entry_bound"]=oe_high_tier_des_bound::report(w);
    if(report.at("elite_entry_bound").at("blocks_entry").get<bool>())for(auto& row:report.at("types16"))
        if(row.at("type")==17) {
            row["classification"]="geometrically blocked";
            row["classification_scope"]="Exact checked integer source/footprint relaxation for current Normal Rule12: DES<=79<80 even without terrain, roads, funds, staffing or existing-city restrictions. Not a statement about original Emperor runtime.";
        }
    fs::create_directories(output);write(output/"matrix.json",report);
    q::save::write_save(output/"matrix-world.save.json",c.document(w),c.data,c.mask);
    if(input)check(q::f::read_text(*input)==input_bytes,"Matrix changed earned source save");
    std::cout<<"OE38 matrix technical PASS; high-tier reachability unproven\n";return 0;
}

// The raw JSON is the serialized complete snapshot already checked by checkpoint.
void high_checkpoint(const q::Context& c,const fs::path& output,const char* label,const sim::World& w) {
    checkpoint(c,output,label,w);
    write(output/(std::string(label)+".snapshot.json"),Json::parse(q::f::read_text(output/(std::string(label)+".save.json"))).at("world"));
}
struct Continuation {
    Json metrics=nullptr;std::uint64_t target_lost=0,type_lost=0,goal_lost=0;
    std::int64_t minimum=0,cashflow=0;bool recovery_equal=false;
};
Continuation retain(const q::Context& c,const fs::path& output,sim::World& w,sim::BuildingId target,
                    const oe::original::HouseEvolutionRules& independent) {
    namespace observation=oe_high_tier_metrics;
    constexpr auto wanted=oe::original::HouseType::LuxuriousApartment;
    const auto begin=w.ticks();Continuation result;result.minimum=w.treasury();
    q::save::RecoveryStore recovery(output/"recovery-app",c.data,q::save::recovery_storage_budget_bytes,q::save::RecoveryAccess::OriginalProduction);
    const auto history=recovery.create_history(c.document(w),c.mask);
    observation::Observation retained(w,target,independent);
    while(w.ticks()<begin+20400) {
        retained.before(w);const auto tick=w.ticks();w.tick();check(w.ticks()==tick+1,"Common retention World refused the next tick");retained.after(w);
        result.minimum=std::min(result.minimum,w.treasury());
        if(!observation::actual_first_target_success(w,target,wanted))++result.target_lost;
        if(q::house(w,target).type!=wanted)++result.type_lost;
        if(!w.original_goal_status().reached)++result.goal_lost;
    }
    result.metrics=retained.report(w);high_checkpoint(c,output,"result",w);
    const auto entry=recovery.write_checkpoint(history,c.document(w),c.mask);
    check(entry.loadable()&&entry.schema==28&&entry.rule_version==12,"Common recovery lost exact generation");
    auto restored=q::save::restore_original_sustainable_economy(q::save::read_save(entry.path),c.data,c.mask,c.policy,c.model);
    check(restored.snapshot()==w.snapshot()&&q::f::read_text(entry.path)==q::f::read_text(output/"result.save.json"),"Common complete Recovery readback differs");
    result.recovery_equal=true;
    result.cashflow=result.metrics.at("city").at("summary").at("operating_cashflow").get<std::int64_t>();
    write(output/"retention.json",result.metrics);return result;
}

constexpr const char* common_usage="high-tier-progression-acceptance --mode common --variant common-reserved|common-near-pottery|common-two-market --data ABSOLUTE --output NEW_PRIVATE_OE38_DIRECTORY [--search-ticks 6000..60000]";
int common(int argc,char** argv) {
    namespace strategy=oe_high_tier_strategy;
    namespace observation=oe_high_tier_metrics;
    std::map<std::string,std::string> args;
    for(int i=1;i<argc;i+=2) {
        check(i+1<argc,common_usage);const std::string key=argv[i];
        check(key=="--mode"||key=="--variant"||key=="--data"||key=="--output"||key=="--search-ticks",common_usage);
        check(args.emplace(key,argv[i+1]).second,"Duplicate Common scenario argument");
    }
    for(const char* key:{"--mode","--variant","--data","--output"})check(args.contains(key),common_usage);
    check(args.at("--mode")=="common"&&strategy::common_variant(args.at("--variant")),common_usage);
    unsigned search_ticks=30000;
    if(args.contains("--search-ticks")) {
        const auto& text=args.at("--search-ticks");const auto parsed=std::from_chars(text.data(),text.data()+text.size(),search_ticks);
        check(parsed.ec==std::errc{}&&parsed.ptr==text.data()+text.size(),common_usage);
    }
    check(search_ticks>=6000&&search_ticks<=60000,common_usage);
    const auto data=fs::absolute(args.at("--data")),output=fs::absolute(args.at("--output"));
    safe(data);safe(output);check(output!=private_root()&&below(output,private_root())&&!fs::exists(output),"Common requires a new private OE38 directory");
    check(!below(output,data)&&!below(data,output),"Common output overlaps original installation");
    q::save::validate_save_target(output/"report.json",data);
    const auto c=q::Context::real(data);
    check(c.model->provenance().sha256=="7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6","Unchanged original Normal model required");
    const auto model=oe::original::load_model_data(data);check(bool(model),"Independent model preparation failed");
    const oe::original::HouseEvolutionRules independent(*model.model,oe::original::Difficulty::Normal);
    auto w=sim::World::create_original_sustainable_economy(c.policy,c.model);
    const auto origin=player::origin(c);const auto initial_commands=player::build(w,"balanced-three",origin);
    metrics::Observation warmup(w);
    while(w.ticks()<20000){warmup.before(w);const auto tick=w.ticks();w.tick();check(w.ticks()==tick+1,"Baseline World refused the next tick");warmup.after(w);}
    check(player::maturity(w).at("ready").get<bool>(),"Common expansion did not earn the actual OE37 maturity");
    const auto warmup_report=warmup.report(w);
    const auto expansion=strategy::build_common(w,origin,args.at("--variant"));
    const auto target=expansion.target;constexpr auto wanted=oe::original::HouseType::LuxuriousApartment;
    fs::create_directories(output);high_checkpoint(c,output,"expansion",w);
    observation::Observation search(w,target,independent);unsigned highest=static_cast<unsigned>(q::house(w,target).type);
    const auto actual_target_entry=[&] {
        const auto& home=q::house(w,target);
        const auto& histories=w.original_production_state()->housing.houses;
        const auto found=std::find_if(histories.begin(),histories.end(),[&](const auto& h){return h.building_id==target;});
        check(found!=histories.end(),"Paid Common target lost its authoritative housing history");
        const auto& entry=found->first_transitions[oe::original::model_row(oe::original::house_row(wanted)).value];
        return home.type==wanted&&home.occupants>0&&entry&&entry->kind==sim::OriginalHousingTransitionKind::Evolution;
    };
    const auto end=w.ticks()+search_ticks;
    while(w.ticks()<end&&!actual_target_entry()) {
        search.before(w);const auto tick=w.ticks();w.tick();check(w.ticks()==tick+1,"Common World refused the next tick");search.after(w);
        highest=std::max(highest,static_cast<unsigned>(q::house(w,target).type));
    }
    const bool reached=actual_target_entry();const auto search_report=search.report(w);
    const auto reached_tick=w.ticks();
    std::uint64_t target_lost_ticks=0,type_lost_ticks=0,goal_lost_ticks=0;
    std::int64_t minimum=w.treasury();std::int64_t cashflow=0;bool recovery_equal=false;
    Json first=nullptr;
    if(reached) {
        check(observation::history(w,target).first_transitions[oe::original::model_row(oe::original::house_row(wanted)).value]->evaluation.tick==w.ticks(),
              "First success must be captured at its exact actual transition tick");
        high_checkpoint(c,output,"first-success",w);
        first={{"house",static_cast<unsigned>(target)},{"type",static_cast<unsigned>(wanted)},
            {"tick",w.ticks()},{"funds",w.treasury()},{"construction",w.construction_spent_total()},
            {"actual_world_goal",w.original_goal_status().reached},{"exact_live_save","first-success.save.json"}};
        const auto continued=retain(c,output,w,target,independent);
        target_lost_ticks=continued.target_lost;type_lost_ticks=continued.type_lost;
        goal_lost_ticks=continued.goal_lost;minimum=continued.minimum;cashflow=continued.cashflow;recovery_equal=continued.recovery_equal;
    }else high_checkpoint(c,output,"search-end",w);
    q::authority(w);
    write(output/"search.json",search_report);write(output/"warmup.json",warmup_report);
    const auto elite_bound=oe_high_tier_des_bound::report(w);
    write(output/"report.json",{{"format","openemperor-paid-high-tier-common-v1"},
        {"technical_status","PASS"},{"common10",reached?"PASS":"PARTIAL"},
        {"elite17",elite_bound.at("blocks_entry").get<bool>()?"BLOCKED":"PARTIAL"},{"elite_entry_bound",elite_bound},
        {"retention",reached?(target_lost_ticks==0?"PASS":"PARTIAL"):"NOT_RUN"},
        {"financial_stability",reached?(cashflow>=0&&minimum>=0?"PASS":"FAIL"):"PARTIAL"},
        {"original_parity","PARTIAL"},{"rule",12},{"schema",28},{"policy",3},{"revision",27},
        {"variant",args.at("--variant")},{"snapshot_injection",false},{"initial_commands",initial_commands},
        {"expansion_commands",expansion.commands},{"target_house",static_cast<unsigned>(target)},
        {"first_reached",first},{"highest_actual_type",highest},{"search_ticks",reached_tick-20000},
        {"retention_ticks",reached?20400:0},{"target_lost_ticks",target_lost_ticks},{"type_lost_ticks",type_lost_ticks},{"goal_loss_ticks",goal_lost_ticks},
        {"operating_cashflow",cashflow},{"minimum_retention_funds",minimum},{"final_tick",w.ticks()},
        {"final_funds",w.treasury()},{"final_type",static_cast<unsigned>(q::house(w,target).type)},
        {"retained_at_end",reached&&actual_target_entry()},{"eligible_nonburning_at_end",reached&&observation::actual_first_target_success(w,target,wanted)},
        {"complete_recovery_equality",recovery_equal},{"final_goal",w.original_goal_status().reached}});
    std::cout<<"Common10 "<<(reached?"PASS":"PARTIAL")<<"; retention "<<(reached?(target_lost_ticks?"PARTIAL":"PASS"):"NOT_RUN")<<"; real tick "<<w.ticks()<<'\n';
    return 0;
}

constexpr const char* continuation_usage="high-tier-progression-acceptance --mode high-resume|high-render --target ID --data ABSOLUTE --output NEW_PRIVATE_OE38_DIRECTORY --input EXACT_EARNED_SAVE [--resources ABSOLUTE (render only)]";
int continuation(int argc,char** argv) {
    std::map<std::string,std::string> args;
    for(int i=1;i<argc;i+=2) {
        check(i+1<argc,continuation_usage);const std::string key=argv[i];
        check(key=="--mode"||key=="--target"||key=="--data"||key=="--output"||key=="--input"||key=="--resources",continuation_usage);
        check(args.emplace(key,argv[i+1]).second,"Duplicate continuation argument");
    }
    for(const char* key:{"--mode","--target","--data","--output","--input"})check(args.contains(key),continuation_usage);
    const bool visual=args.at("--mode")=="high-render";
    check(visual||args.at("--mode")=="high-resume",continuation_usage);
    check(args.contains("--resources")==visual,continuation_usage);
    unsigned raw_target=0;const auto& text=args.at("--target");
    const auto parsed=std::from_chars(text.data(),text.data()+text.size(),raw_target);
    check(parsed.ec==std::errc{}&&parsed.ptr==text.data()+text.size()&&raw_target>0,continuation_usage);
    const auto target=static_cast<sim::BuildingId>(raw_target);
    Args a;a.mode="render";a.rule=12;a.variant="balanced-three";
    a.data=fs::absolute(args.at("--data"));a.output=fs::absolute(args.at("--output"));a.input=fs::absolute(args.at("--input"));
    for(const auto& path:{a.data,a.output,a.input})safe(path);
    check(a.output!=private_root()&&below(a.output,private_root())&&!fs::exists(a.output),"Continuation requires a new private OE38 directory");
    check(below(a.input,private_root())&&fs::is_regular_file(a.input)&&!below(a.input,a.output),"Explicit protected earned OE38 input required");
    check(!below(a.output,a.data)&&!below(a.data,a.output),"Continuation output overlaps original installation");
    if(visual) {
        a.resources=fs::absolute(args.at("--resources"));safe(a.resources);render(a,target);return 0;
    }
    q::save::validate_save_target(a.output/"report.json",a.data);
    const auto c=q::Context::real(a.data);const auto input=q::f::read_text(a.input);
    check(c.model->provenance().sha256=="7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6","Unchanged original Normal model required");
    auto w=q::save::restore_original_sustainable_economy(q::save::read_save(a.input),c.data,c.mask,c.policy,c.model);
    const auto& house=q::house(w,target);const auto& history=oe_high_tier_metrics::history(w,target);
    const auto& entry=history.first_transitions[oe::original::model_row(oe::original::house_row(oe::original::HouseType::LuxuriousApartment)).value];
    check(house.type==oe::original::HouseType::LuxuriousApartment&&house.occupants>0&&entry&&
          entry->kind==sim::OriginalHousingTransitionKind::Evolution&&entry->evaluation.tick==w.ticks()&&
          entry->evaluation.input.type==oe::original::HouseType::OrnateApartment,
          "Continuation requires the exact actual first9->10 live checkpoint");
    const auto model=oe::original::load_model_data(a.data);check(bool(model),"Independent original model preparation failed");
    const oe::original::HouseEvolutionRules independent(*model.model,oe::original::Difficulty::Normal);
    q::authority(w);const auto begin=w.ticks(),sequence=w.command_sequence();
    fs::create_directories(a.output);high_checkpoint(c,a.output,"first-success",w);
    check(q::f::read_text(a.output/"first-success.save.json")==input,"Separate process changed literal first-success bytes");
    const auto continued=retain(c,a.output,w,target,independent);q::authority(w);
    check(w.command_sequence()==sequence&&q::f::read_text(a.input)==input,"Continuation issued a command or changed its protected seed");
    write(a.output/"report.json",{{"format","openemperor-high-tier-resume-v1"},{"technical_status","PASS"},
        {"target",raw_target},{"begin_tick",begin},{"end_tick",w.ticks()},{"retention_ticks",20400},
        {"rule",12},{"schema",28},{"policy",3},{"revision",27},{"snapshot_injection",false},
        {"input_preserved",true},{"command_sequence_unchanged",true},{"complete_recovery_equality",continued.recovery_equal},
        {"target_lost_ticks",continued.target_lost},{"type_lost_ticks",continued.type_lost},{"goal_loss_ticks",continued.goal_lost},
        {"minimum_retention_funds",continued.minimum},{"operating_cashflow",continued.cashflow},{"native_human","NOT_RUN"}});
    std::cout<<"Actual Common first-success separate-process continuation technical PASS;20400 ticks\n";return 0;
}
}

int main(int argc,char** argv) {
    try {
        for(int i=1;i+1<argc;++i)if(std::string_view(argv[i])=="--mode"&&std::string_view(argv[i+1])=="matrix")
            return oe38_cli::matrix(argc,argv);
        for(int i=1;i+1<argc;++i)if(std::string_view(argv[i])=="--mode"&&std::string_view(argv[i+1])=="common")
            return oe38_cli::common(argc,argv);
        for(int i=1;i+1<argc;++i)if(std::string_view(argv[i])=="--mode"&&
            (std::string_view(argv[i+1])=="high-resume"||std::string_view(argv[i+1])=="high-render"))
            return oe38_cli::continuation(argc,argv);
        if(argc==2&&std::string_view(argv[1])=="--help") {
            std::cout<<oe38_cli::matrix_usage<<'\n'<<oe38_cli::common_usage<<'\n'<<oe38_cli::continuation_usage
                <<"\nBaseline modes also accept the sustainable-economy arguments; all evidence stays under .local/original-economy/oe38/.\n";return 0;
        }
        return oe37_baseline_main(argc,argv);
    }catch(const std::exception& e){std::cerr<<"High-tier progression FAIL: "<<e.what()<<'\n';return 1;}
}
