// Separate QA executable; the production simulation and rendering are reused.
#define OE_HIGH_TIER_PROOF_DIRECTORY "oe39"
#define OE_HIGH_TIER_REUSE_ONLY
#include "HighTierProgressionAcceptance.cpp"
#undef OE_HIGH_TIER_REUSE_ONLY
#undef OE_HIGH_TIER_PROOF_DIRECTORY

#include "../tests/MarketDistributionMetrics.h"
#include "../tests/MarketDistributionStrategy.h"

namespace oe39_cli {
namespace trace=oe_market_distribution_metrics;
namespace high=oe_high_tier_metrics;
namespace strategy=oe_market_distribution_strategy;
using oe38_cli::high_checkpoint;
constexpr const char* help="market-distribution-acceptance --mode diagnose|resume|render --data ABSOLUTE --output NEW_PRIVATE_OE39_DIRECTORY [--variant COMMON_VARIANT] [--search-ticks 30000] [--input EARNED_SAVE --target 3 --end-tick N] [--resources ABSOLUTE]";
struct Options {std::string mode,variant="common-reserved";fs::path data,output,input,resources;unsigned target=3,search=30000,end=0;};
Options options(int argc,char** argv) {
    Options a;std::map<std::string,std::string> v;
    for(int i=1;i<argc;i+=2) {
        check(i+1<argc,help);const std::string key=argv[i];
        check(key=="--mode"||key=="--variant"||key=="--data"||key=="--output"||key=="--input"||
            key=="--resources"||key=="--target"||key=="--search-ticks"||key=="--end-tick",help);
        check(v.emplace(key,argv[i+1]).second,"Duplicate market proof argument");
    }
    for(const auto* key:{"--mode","--data","--output"})check(v.contains(key),help);
    a.mode=v.at("--mode");check(a.mode=="diagnose"||a.mode=="resume"||a.mode=="render",help);
    if(v.contains("--variant"))a.variant=v.at("--variant");check(strategy::common_variant(a.variant),help);
    for(const auto& [key,path]:{std::pair{"--data",&a.data},{"--output",&a.output},{"--input",&a.input},{"--resources",&a.resources}})
        if(v.contains(key))*path=fs::absolute(v.at(key));
    for(const auto& [key,n]:{std::pair{"--target",&a.target},{"--search-ticks",&a.search},{"--end-tick",&a.end}})if(v.contains(key)) {
        const auto& text=v.at(key);const auto parsed=std::from_chars(text.data(),text.data()+text.size(),*n);
        check(parsed.ec==std::errc{}&&parsed.ptr==text.data()+text.size(),help);
    }
    check(a.target>0&&a.search>=6000&&a.search<=60000,help);
    safe(a.data);safe(a.output);check(fs::is_directory(a.data),"Original installation missing");
    check(a.output!=private_root()&&below(a.output,private_root())&&!fs::exists(a.output),"Market proof needs a new private OE39 output");
    check(!below(a.output,a.data)&&!below(a.data,a.output),"Market output overlaps original installation");
    if(a.mode!="diagnose") {safe(a.input);check(below(a.input,private_root())&&fs::is_regular_file(a.input)&&!below(a.input,a.output),"Explicit earned OE39 input required");}
    else check(a.input.empty()&&a.end==0,help);
    if(a.mode=="render"){safe(a.resources);check(fs::is_directory(a.resources),"Rendering requires actual compatibility resources");}
    else check(a.resources.empty(),help);
    q::save::validate_save_target(a.output/"report.json",a.data);return a;
}
bool entered(const sim::World& w,sim::BuildingId target) {
    const auto& h=q::house(w,target);const auto& e=high::history(w,target).first_transitions[
        oe::original::model_row(oe::original::house_row(oe::original::HouseType::LuxuriousApartment)).value];
    return h.type==oe::original::HouseType::LuxuriousApartment&&h.occupants>0&&e&&
        e->kind==sim::OriginalHousingTransitionKind::Evolution&&e->evaluation.input.type==oe::original::HouseType::OrnateApartment;
}
struct Retained {
    Json metrics,transport,checkpoints=Json::array(),first_failure=nullptr;
    std::uint64_t target_lost=0,type_lost=0,goal_lost=0;
    std::uint64_t actual_evaluations=0,current_unmet=0,hemp_unmet=0,ceramics_unmet=0;
    const char* sustained_supply="NOT_RUN";
    std::int64_t minimum=0,cashflow=0;bool recovered=false;
};
Retained retain(const q::Context& c,const fs::path& output,sim::World& w,sim::BuildingId target,
                const oe::original::HouseEvolutionRules& rules,std::uint64_t end,bool diagnostic_checkpoints) {
    Retained out;const auto begin=w.ticks();out.minimum=w.treasury();high::Observation observed(w,target,rules);trace::Observation traced(w,target,rules);
    q::save::RecoveryStore recovery(output/"recovery-app",c.data,q::save::recovery_storage_budget_bytes,q::save::RecoveryAccess::OriginalProduction);
    const auto history=recovery.create_history(c.document(w),c.mask);
    bool cargo_saved=false,shortage_saved=false,devolution_saved=false;
    const auto save=[&](const char* label){high_checkpoint(c,output,label,w);out.checkpoints.push_back(label);};
    while(w.ticks()<end) {
        const auto before_type=q::house(w,target).type;
        observed.before(w);traced.before(w);const auto tick=w.ticks();w.tick();check(w.ticks()==tick+1,"Market proof World refused next tick");
        observed.after(w);traced.after(w);out.minimum=std::min(out.minimum,w.treasury());
        if(!high::actual_first_target_success(w,target,oe::original::HouseType::LuxuriousApartment))++out.target_lost;
        if(q::house(w,target).type!=oe::original::HouseType::LuxuriousApartment)++out.type_lost;
        if(!w.original_goal_status().reached)++out.goal_lost;
        const auto& e=high::history(w,target).last_evaluation;
        const bool actual_unmet=e&&e->tick==w.ticks()&&!oe::original::needs_met(rules,oe::original::house_row(e->input.type),e->input);
        if(e&&e->tick==w.ticks()) {
            ++out.actual_evaluations;if(actual_unmet)++out.current_unmet;
            const auto needs=high::prior::requirements(rules,oe::original::house_row(e->input.type),e->input);
            for(const auto& need:needs.at("requirements"))if(!need.at("met").get<bool>()) {
                if(need.at("field")=="Hemp")++out.hemp_unmet;
                if(need.at("field")=="Ceramics")++out.ceramics_unmet;
            }
        }
        if(actual_unmet&&out.first_failure.is_null())out.first_failure={{"tick",w.ticks()},{"actual_evaluation",high::evaluation(*e,rules)}};
        if(diagnostic_checkpoints) {
            if(w.ticks()==28800)save("before-shortage");
            if(!cargo_saved&&std::any_of(w.couriers().begin(),w.couriers().end(),[&](const auto& actor){
                return sim::original_industry_goods::typed(actor.role)&&actor.phase==sim::CourierPhase::ToWarehouse&&
                    actor.edge_progress>0&&w.original_courier_cargo(actor.id)>0;})) {save("active-cargo");cargo_saved=true;}
            if(!shortage_saved&&actual_unmet){save("first-shortage");shortage_saved=true;}
            if(!devolution_saved&&q::house(w,target).type<before_type){save("first-devolution");devolution_saved=true;}
        }
    }
    out.metrics=observed.report(w);out.transport=traced.report(w);high_checkpoint(c,output,"result",w);
    const auto entry=recovery.write_checkpoint(history,c.document(w),c.mask);
    check(entry.loadable()&&entry.schema==28&&entry.rule_version==12,"Market proof Recovery changed historical generation");
    auto restored=q::save::restore_original_sustainable_economy(q::save::read_save(entry.path),c.data,c.mask,c.policy,c.model);
    check(restored.snapshot()==w.snapshot()&&q::f::read_text(entry.path)==q::f::read_text(output/"result.save.json"),"Market complete Recovery differs");
    out.recovered=true;out.cashflow=out.metrics.at("city").at("summary").at("operating_cashflow").get<std::int64_t>();
    check(out.current_unmet==out.metrics.at("summary").at("current_unmet_evaluations").get<std::uint64_t>(),
          "Strict market supply verdict differs from actual independent target phase39 metrics");
    if(w.ticks()-begin==20400)out.sustained_supply=out.actual_evaluations==400&&out.target_lost==0&&out.type_lost==0&&
        out.current_unmet==0&&out.hemp_unmet==0&&out.ceramics_unmet==0?"PASS":"PARTIAL";
    write(output/"retention.json",out.metrics);write(output/"trace-retention.json",out.transport);return out;
}
int run(int argc,char** argv) {
    const auto a=options(argc,argv);const auto target_requested=static_cast<sim::BuildingId>(a.target);
    if(a.mode=="render") {Args render_args;render_args.mode="render";render_args.rule=12;render_args.variant="balanced-three";
        render_args.data=a.data;render_args.output=a.output;render_args.input=a.input;render_args.resources=a.resources;render(render_args,target_requested);return 0;}
    const auto c=q::Context::real(a.data);check(c.model->provenance().sha256=="7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6","Unchanged pinned Normal model required");
    const auto parsed=oe::original::load_model_data(a.data);check(bool(parsed),"Independent original model preparation failed");
    const oe::original::HouseEvolutionRules rules(*parsed.model,oe::original::Difficulty::Normal);
    fs::create_directories(a.output);
    if(a.mode=="resume") {
        const auto bytes=q::f::read_text(a.input);auto w=q::save::restore_original_sustainable_economy(q::save::read_save(a.input),c.data,c.mask,c.policy,c.model);
        const auto begin=w.ticks(),sequence=w.command_sequence();check(w.rule_version()==12&&a.end>begin&&a.end-begin<=20400,"Resume needs exact Rule12 seed and bounded real endpoint");
        high_checkpoint(c,a.output,"resume-start",w);check(q::f::read_text(a.output/"resume-start.save.json")==bytes,"Resume changed literal complete input SaveDocument");
        if(entered(w,target_requested)&&high::history(w,target_requested).first_transitions[7]->evaluation.tick==w.ticks())high_checkpoint(c,a.output,"first-success",w);
        const auto retained=retain(c,a.output,w,target_requested,rules,a.end,false);q::authority(w);
        check(w.command_sequence()==sequence&&q::f::read_text(a.input)==bytes,"Resume issued commands or changed protected input");
        write(a.output/"report.json",{{"format","openemperor-high-tier-resume-v1"},{"technical_status","PASS"},
            {"target",a.target},{"begin_tick",begin},{"end_tick",w.ticks()},{"retention_ticks",w.ticks()-begin},
            {"rule",12},{"schema",28},{"policy",3},{"revision",27},{"snapshot_injection",false},{"input_preserved",true},
            {"command_sequence_unchanged",true},{"complete_recovery_equality",retained.recovered},{"target_lost_ticks",retained.target_lost},
            {"type_lost_ticks",retained.type_lost},{"goal_loss_ticks",retained.goal_lost},{"minimum_retention_funds",retained.minimum},
            {"operating_cashflow",retained.cashflow},{"sustained_supply",retained.sustained_supply},
            {"supply_evaluations",{{"actual",retained.actual_evaluations},{"current_unmet",retained.current_unmet},
                {"hemp_unmet",retained.hemp_unmet},{"ceramics_unmet",retained.ceramics_unmet}}},
            {"native_human","NOT_RUN"}});return 0;
    }
    auto w=sim::World::create_original_sustainable_economy(c.policy,c.model);const auto origin=player::origin(c);
    const auto initial_commands=player::build(w,"balanced-three",origin);metrics::Observation warmup(w);
    while(w.ticks()<20000){warmup.before(w);w.tick();warmup.after(w);}check(player::maturity(w).at("ready").get<bool>(),"Market proof failed to earn OE37 maturity");
    write(a.output/"warmup.json",warmup.report(w));const auto expansion=strategy::build_common(w,origin,a.variant);const auto target=expansion.target;
    high_checkpoint(c,a.output,"expansion",w);high::Observation searched(w,target,rules);trace::Observation traced(w,target,rules);
    const auto end=w.ticks()+a.search;unsigned highest=static_cast<unsigned>(q::house(w,target).type);
    while(w.ticks()<end&&!entered(w,target)) {
        searched.before(w);traced.before(w);w.tick();searched.after(w);traced.after(w);
        highest=std::max(highest,static_cast<unsigned>(q::house(w,target).type));
    }
    write(a.output/"search.json",searched.report(w));write(a.output/"trace-search.json",traced.report(w));
    const bool reached=entered(w,target);const auto first_tick=w.ticks();Json first=nullptr;Retained retained;Json checkpoints=Json::array({"expansion"});
    if(reached) {
        check(high::history(w,target).first_transitions[7]->evaluation.tick==w.ticks(),"Market proof first success lacks exact actual9->10 tick");
        high_checkpoint(c,a.output,"first-success",w);checkpoints.push_back("first-success");
        first={{"house",static_cast<unsigned>(target)},{"type",10},{"tick",w.ticks()},{"funds",w.treasury()},
            {"construction",w.construction_spent_total()},{"actual_world_goal",w.original_goal_status().reached},{"exact_live_save","first-success.save.json"}};
        retained=retain(c,a.output,w,target,rules,w.ticks()+20400,true);
        for(const auto& label:retained.checkpoints)checkpoints.push_back(label);checkpoints.push_back("result");
    }else {high_checkpoint(c,a.output,"search-end",w);checkpoints.push_back("search-end");}
    q::authority(w);const auto bound=oe_high_tier_des_bound::report(w);
    write(a.output/"report.json",{{"format","openemperor-paid-high-tier-common-v1"},{"technical_status","PASS"},
        {"common10",reached?"PASS":"PARTIAL"},{"elite17","BLOCKED"},{"elite_entry_bound",bound},
        {"retention",reached?(retained.target_lost?"PARTIAL":"PASS"):"NOT_RUN"},
        {"financial_stability",reached?(retained.cashflow>=0&&retained.minimum>=0?"PASS":"FAIL"):"PARTIAL"},
        {"original_parity","PARTIAL"},{"rule",12},{"schema",28},{"policy",3},{"revision",27},{"variant",a.variant},
        {"snapshot_injection",false},{"initial_commands",initial_commands},{"expansion_commands",expansion.commands},
        {"target_house",static_cast<unsigned>(target)},{"first_reached",first},{"highest_actual_type",highest},{"search_ticks",first_tick-20000},
        {"retention_ticks",reached?20400:0},{"target_lost_ticks",retained.target_lost},{"type_lost_ticks",retained.type_lost},
        {"goal_loss_ticks",retained.goal_lost},{"operating_cashflow",retained.cashflow},{"minimum_retention_funds",retained.minimum},
        {"final_tick",w.ticks()},{"final_funds",w.treasury()},{"final_type",static_cast<unsigned>(q::house(w,target).type)},
        {"retained_at_end",reached&&entered(w,target)},{"eligible_nonburning_at_end",reached&&high::actual_first_target_success(w,target,oe::original::HouseType::LuxuriousApartment)},
        {"complete_recovery_equality",retained.recovered},{"final_goal",w.original_goal_status().reached},
        {"trace_format","openemperor-market-distribution-trace-v1"},{"checkpoints",checkpoints},{"first_failure",retained.first_failure},
        {"sustained_supply",retained.sustained_supply},{"supply_evaluations",{{"actual",retained.actual_evaluations},
            {"current_unmet",retained.current_unmet},{"hemp_unmet",retained.hemp_unmet},{"ceramics_unmet",retained.ceramics_unmet}}}});
    std::cout<<"OE39 actual paid "<<a.variant<<" Common10 "<<(reached?"reached":"not reached")<<"; retention "
        <<retained.sustained_supply<<"; tick "<<w.ticks()<<'\n';return 0;
}
}
int main(int argc,char** argv) {
    try {if(argc==2&&std::string_view(argv[1])=="--help"){std::cout<<oe39_cli::help<<'\n';return 0;}return oe39_cli::run(argc,argv);}
    catch(const std::exception& e){std::cerr<<"Market distribution proof FAIL: "<<e.what()<<'\n';return 1;}
}
