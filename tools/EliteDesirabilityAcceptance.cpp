// OE40 Gate C: actual paid commands and observations, never a projected tier.
// Reuse only the existing generic path/SDL helpers, not its Rule11/12 factories.
#define OE_SUSTAINABLE_PROOF_DIRECTORY "oe40"
#define main oe40_reused_sustainable_main
#include "SustainableEconomyAcceptance.cpp"
#undef main
#undef OE_SUSTAINABLE_PROOF_DIRECTORY
#include "../tests/EliteDesirabilityStrategy.h"
#include "../tests/MarketDistributionMetrics.h"
#include "../tests/EliteStabilityDiagnosis.h"
#include <openssl/evp.h>
#include <fstream>
#include <iomanip>
#include <set>
#include <sstream>

namespace oe40_cli {
namespace strategy=oe_elite_desirability_strategy;
namespace high=oe_high_tier_metrics;
namespace trace=oe_market_distribution_metrics;
namespace orig=oe::original;
constexpr const char* help="elite-desirability-acceptance --mode create|resume|render --data ABS --output NEW_IGNORED_OE40_OR_OE43_DIR [--variant staged|all-paid|lean-med|lean-med-extra-markets] [--search-ticks 40000|60000] [--input EARNED_SCHEMA30_SAVE] [--expected CREATOR_RESULT_SAVE (resume)] [--resources ABS (render)] [--diagnosis oe44 (create/resume; fresh OE44 output only)]";
struct Options {std::string mode,variant="staged",diagnosis;fs::path data,output,input,expected,resources;unsigned search=40000;};
Options options(int argc,char** argv){
    Options a;std::map<std::string,std::string> values;
    for(int i=1;i<argc;i+=2){check(i+1<argc,help);const std::string key=argv[i];
        check(key=="--mode"||key=="--data"||key=="--output"||key=="--variant"||key=="--search-ticks"||key=="--input"||key=="--expected"||key=="--resources"||key=="--diagnosis",help);
        check(values.emplace(key,argv[i+1]).second,"Duplicate Elite proof argument");}
    for(const auto* key:{"--mode","--data","--output"})check(values.contains(key),help);
    a.mode=values.at("--mode");check(a.mode=="create"||a.mode=="resume"||a.mode=="render",help);
    if(values.contains("--variant"))a.variant=values.at("--variant");check(strategy::known(a.variant),help);
    if(values.contains("--diagnosis")){a.diagnosis=values.at("--diagnosis");check(a.diagnosis=="oe44"&&a.mode!="render",help);}
    for(const auto& [key,path]:{std::pair{"--data",&a.data},{"--output",&a.output},{"--input",&a.input},{"--expected",&a.expected},{"--resources",&a.resources}})
        if(values.contains(key))*path=fs::absolute(values.at(key));
    if(values.contains("--search-ticks")){const auto& text=values.at("--search-ticks");const auto parsed=std::from_chars(text.data(),text.data()+text.size(),a.search);
        check(parsed.ec==std::errc{}&&parsed.ptr==text.data()+text.size()&&(a.search==40000||a.search==60000),help);}
    safe(a.data);safe(a.output);check(fs::is_directory(a.data),"Original installation missing");
    const auto diagnosis_root=fs::canonical(fs::path(__FILE__).parent_path().parent_path())/".local/original-economy/oe44";
    const bool allowed_output=a.diagnosis.empty()?private_output(a.output):
        a.output!=diagnosis_root&&below(a.output,diagnosis_root);
    check(allowed_output&&!fs::exists(a.output),"Use a fresh ignored output in the selected OE40/OE43 or explicit OE44 diagnosis root");
    check(!below(a.output,a.data)&&!below(a.data,a.output),"Output overlaps original data");
    for(const auto& path:{a.input,a.expected})if(!path.empty()){
        safe(path);const bool allowed_input=below(path,private_root())||
            (!a.diagnosis.empty()&&path!=diagnosis_root&&below(path,diagnosis_root));
        check(allowed_input&&fs::is_regular_file(path)&&fs::file_size(path)<=8U*1024U*1024U&&!below(path,a.output),
            "Use bounded explicit protected OE40 or selected OE44 diagnosis saves");
    }
    if(a.mode=="create")check(a.input.empty()&&a.expected.empty()&&a.resources.empty(),help);
    else check(!a.input.empty(),help);
    if(a.mode=="resume")check(!a.expected.empty()&&a.resources.empty(),"Resume requires the independent creator's expected result");
    if(a.mode=="render"){safe(a.resources);check(fs::is_directory(a.resources)&&a.expected.empty(),help);}
    q::save::validate_save_target(a.output/"report.json",a.data);return a;
}
std::string hash_file(const fs::path& path){
    check(fs::is_regular_file(path)&&!fs::is_symlink(path)&&fs::file_size(path)<=64U*1024U*1024U,"Bounded regular source file required");
    std::ifstream in(path,std::ios::binary);check(bool(in),"Cannot read source identity");
    std::unique_ptr<EVP_MD_CTX,decltype(&EVP_MD_CTX_free)> ctx(EVP_MD_CTX_new(),EVP_MD_CTX_free);
    check(ctx&&EVP_DigestInit_ex(ctx.get(),EVP_sha256(),nullptr)==1,"SHA256 setup failed");std::array<char,65536> buffer{};
    while(in){in.read(buffer.data(),static_cast<std::streamsize>(buffer.size()));const auto n=in.gcount();if(n>0)check(EVP_DigestUpdate(ctx.get(),buffer.data(),static_cast<std::size_t>(n))==1,"SHA256 update failed");}
    check(in.eof(),"Source read failed");std::array<unsigned char,EVP_MAX_MD_SIZE> digest{};unsigned length=0;
    check(EVP_DigestFinal_ex(ctx.get(),digest.data(),&length)==1&&length==32,"SHA256 finish failed");std::ostringstream text;
    for(unsigned i=0;i<length;++i)text<<std::hex<<std::setw(2)<<std::setfill('0')<<static_cast<unsigned>(digest[i]);return text.str();
}
Json source_pins(const fs::path& data){
    Json out=Json::object();for(const auto* name:{"Cities/Xia.map","Model/EmperorBuildingModels.txt","Emperor.exe"}){
        const auto file=data/name;safe(file);out[name]={{"sha256",hash_file(file)},{"bytes",fs::file_size(file)},
            {"mode",static_cast<unsigned>(fs::status(file).permissions())},{"mtime",fs::last_write_time(file).time_since_epoch().count()}};}
    check(out["Cities/Xia.map"]["sha256"]==strategy::map_sha&&out["Model/EmperorBuildingModels.txt"]["sha256"]==strategy::model_sha&&
        out["Emperor.exe"]["sha256"]=="6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e","Unchanged pinned Xia/Normal/EXE inputs required");return out;
}
sim::World restore14(const q::Context& c,const q::save::SaveDocument& d){
    check(d.source_schema_version==30&&d.world.rule_version==14,"Exact schema30/Rule14 save required");
    return q::save::restore_original_cardinal_water(d,c.data,c.mask,c.policy,c.model);
}
void checkpoint14(const q::Context& c,const fs::path& output,const std::string& label,const sim::World& w,Json& manifest){
    check(!label.empty()&&label.size()<=80&&std::all_of(label.begin(),label.end(),[](unsigned char v){return (v>='a'&&v<='z')||(v>='0'&&v<='9')||v=='-';}),"Invalid bounded checkpoint label");
    q::authority(w);const auto path=output/(label+".save.json");check(below(path,output)&&!fs::exists(path),"Preserve existing contained checkpoint");
    q::save::write_save(path,c.document(w),c.data,c.mask);const auto bytes=q::f::read_text(path);const auto document=q::save::read_save(path);
    auto loaded=restore14(c,document);check(document.world==w.snapshot()&&loaded.snapshot()==w.snapshot(),"Complete schema30 snapshot restore differs");
    const auto canonical=output/(label+".roundtrip.json");q::save::write_save(canonical,c.document(loaded),c.data,c.mask);
    check(q::f::read_text(canonical)==bytes,"Complete canonical schema30 SaveDocument bytes differ");
    write(output/(label+".snapshot.json"),Json::parse(bytes).at("world"));
    manifest.push_back({{"label",label},{"tick",w.ticks()},{"file",path.filename().string()}});
}
const sim::OriginalHousingTransition* entry17(const sim::World& w,sim::BuildingId target){
    const auto& proof=high::history(w,target).first_transitions[14];
    const bool valid=proof&&proof->evaluation.tick<=w.ticks()&&proof->kind==sim::OriginalHousingTransitionKind::Evolution&&proof->evaluation.input.type==orig::HouseType::ImpressiveCompound&&
        proof->evaluation.input.desirability>=80&&orig::needs_met(w.original_rules_model()->house_rules(),orig::house_row(proof->evaluation.input.type),proof->evaluation.input)&&
        orig::needs_met(w.original_rules_model()->house_rules(),orig::next_row(proof->evaluation.input.type),proof->evaluation.input)&&
        orig::evolve_step(w.original_rules_model()->house_rules(),proof->evaluation.input).type==orig::HouseType::HeavenlyCompound;
    return valid?&*proof:nullptr;
}
bool entered17(const sim::World& w,sim::BuildingId target){
    return entry17(w,target)&&high::actual_first_target_success(w,target,orig::HouseType::HeavenlyCompound);
}
Json provenance(const sim::World& w,sim::BuildingId target){const auto value=w.original_influence_provenance(target);
    return {{"raw",value.raw_value},{"cardinal_bonus",value.cardinal_water_bonus},{"query",value.value},{"source_map_sha256",value.source_map_sha256},
        {"model_sha256",value.model_sha256},{"selected_cell",{value.selected_cell.x,value.selected_cell.y}}};}
void simulate(const Options& a){
    const auto pins=source_pins(a.data);const auto c=q::Context::real(a.data);const bool resumed=a.mode=="resume";
    const auto input=resumed?q::f::read_text(a.input):std::string{},expected_bytes=resumed?q::f::read_text(a.expected):std::string{};
    auto w=resumed?restore14(c,q::save::read_save(a.input)):sim::World::create_original_cardinal_water(c.policy,c.model,q::save::prepare_original_terrain_facts(c.data,c.map_relative));
    check(w.original_cardinal_water_enabled()&&w.original_rules_model()->provenance().sha256==strategy::model_sha,"Explicit unchanged Rule14 required");
    fs::create_directories(a.output);Json commands=Json::array(),blocked=nullptr,manifest=Json::array(),comparisons=Json::array();
    const auto independent=orig::load_model_data(c.data);check(bool(independent),"Independent original model preparation failed");
    const orig::HouseEvolutionRules rules(*independent.model,orig::Difficulty::Normal);
    std::string stop_reason;
    if(!resumed&&!strategy::apply(w,a.variant,commands,blocked))stop_reason="Actual initial paid command refused";
    const auto begin=w.ticks();checkpoint14(c,a.output,"seed",w,manifest);
    if(resumed)check(q::f::read_text(a.output/"seed.save.json")==input,"Separate process changed complete input SaveDocument bytes");
    Json expected_manifest=Json::array();std::set<std::string> compared;
    std::uint64_t requested_end=resumed?q::save::read_save(a.expected).world.ticks:a.search;
    if(resumed){check(requested_end>begin&&requested_end-begin<=80400,"Expected creator endpoint is outside bounded replay");
        const auto path=a.expected.parent_path()/"checkpoints.json";safe(path);check(fs::is_regular_file(path)&&fs::file_size(path)<=65536,"Creator checkpoint manifest required");
        expected_manifest=Json::parse(q::f::read_text(path));check(expected_manifest.is_array()&&expected_manifest.size()<=128,"Invalid bounded checkpoint manifest");
        std::set<std::string> names;for(const auto& entry:expected_manifest){
            const auto name=entry.at("file").get<std::string>();const auto label=entry.at("label").get<std::string>();
            check(!name.empty()&&name.size()<=128&&fs::path(name).filename()==fs::path(name)&&name==label+".save.json"&&
                !label.empty()&&label.size()<=70&&std::all_of(label.begin(),label.end(),[](unsigned char v){return (v>='a'&&v<='z')||(v>='0'&&v<='9')||v=='-';})&&
                names.insert(name).second,"Invalid or duplicate creator checkpoint label");
            check(entry.at("tick").is_number_unsigned(),"Invalid creator checkpoint tick");}
        const auto creator_report=a.expected.parent_path()/"report.json";safe(creator_report);
        check(fs::is_regular_file(creator_report)&&fs::file_size(creator_report)<=1024U*1024U,"Bounded creator recipe report required");
        const auto creator=Json::parse(q::f::read_text(creator_report));check(creator.at("variant")==a.variant&&creator.at("rule")==14&&creator.at("schema")==30,"Replay variant or generation differs from creator");}
    const auto compare_expected=[&]{if(!resumed)return;for(const auto& entry:expected_manifest)if(entry.at("tick").get<std::uint64_t>()==w.ticks()){
        const auto name=entry.at("file").get<std::string>();if(compared.contains(name))continue;
        const auto path=a.expected.parent_path()/name;safe(path);check(fs::is_regular_file(path)&&fs::file_size(path)<=8U*1024U*1024U,"Bounded creator checkpoint required");
        const auto original=q::f::read_text(path);const auto d=q::save::read_save(path);
        check(d.world==w.snapshot(),"Separate-process full checkpoint snapshot differs");Json ignored=Json::array();
        const auto label="replay-"+entry.at("label").get<std::string>();checkpoint14(c,a.output,label,w,ignored);
        check(q::f::read_text(a.output/(label+".save.json"))==original,"Separate-process full checkpoint SaveDocument bytes differ");
        compared.insert(name);comparisons.push_back({{"file",name},{"tick",w.ticks()},{"full_snapshot_equal",true},{"full_document_bytes_equal",true}});}};
    compare_expected();metrics::Observation city(w);std::unique_ptr<high::Observation> observed,retained;
    std::unique_ptr<oe_elite_stability_diagnosis::Observation> diagnosis;
    if(!a.diagnosis.empty())diagnosis=std::make_unique<oe_elite_stability_diagnosis::Observation>(w,a.output);
    std::unique_ptr<trace::Observation> transported;std::optional<sim::BuildingId> target=strategy::elite(w);
    const auto attach=[&]{if(target&&!observed){observed=std::make_unique<high::Observation>(w,*target,rules);transported=std::make_unique<trace::Observation>(w,*target,rules);}};attach();
    std::optional<std::uint64_t> first;std::uint64_t lost=0,unmet=0,next_unmet=0,evaluations=0;std::int64_t retain_min=w.treasury();Json first_entry=nullptr,evaluation_provenance=Json::array();
    if(target&&entry17(w,*target)){const auto& proof=*entry17(w,*target);first=proof.evaluation.tick;
        first_entry={{"tick",*first},{"input",high::evaluation(proof.evaluation,rules)},{"entry_observed_in_this_process",false}};
        retained=std::make_unique<high::Observation>(w,*target,rules);}
    const auto tick_once=[&]{city.before(w);if(observed)observed->before(w);if(transported)transported->before(w);if(retained)retained->before(w);
        if(diagnosis)diagnosis->before(w);
        const auto tick=w.ticks();w.tick();check(w.ticks()==tick+1,"World did not advance one actual tick");city.after(w);
        if(observed)observed->after(w);if(transported)transported->after(w);if(retained)retained->after(w);
        if(diagnosis)diagnosis->after(w);};
    try{while(stop_reason.empty()&&w.ticks()<requested_end){
        if(!first){if(!strategy::apply(w,a.variant,commands,blocked)){stop_reason="Actual paid command refused";break;}target=strategy::elite(w);attach();}
        const bool was_retaining=retained!=nullptr;tick_once();
        if(target){const auto& e=high::history(w,*target).last_evaluation;if(e&&e->tick==w.ticks())evaluation_provenance.push_back({{"tick",w.ticks()},{"provenance",provenance(w,*target)}});}
        if(target&&!first&&entered17(w,*target)){
            const auto& proof=*high::history(w,*target).first_transitions[14];check(proof.evaluation.tick==w.ticks(),"First success was not captured at exact actual16to17 tick");
            first=w.ticks();checkpoint14(c,a.output,"first-success",w,manifest);
            first_entry={{"tick",w.ticks()},{"input",high::evaluation(proof.evaluation,rules)},{"provenance",provenance(w,*target)},{"entry_observed_in_this_process",true}};
            retained=std::make_unique<high::Observation>(w,*target,rules);retain_min=w.treasury();
            if(!resumed)requested_end=w.ticks()+20400;
        }
        if(was_retaining&&target){retain_min=std::min(retain_min,w.treasury());if(!entered17(w,*target))++lost;
            const auto& e=high::history(w,*target).last_evaluation;if(e&&e->tick==w.ticks()){++evaluations;if(!orig::needs_met(rules,orig::house_row(e->input.type),e->input))++unmet;
                if(!orig::needs_met(rules,orig::next_row(e->input.type),e->input))++next_unmet;}}
        if(first&&w.ticks()==*first+10200)checkpoint14(c,a.output,"midpoint",w,manifest);
        else if(w.ticks()%8160==0)checkpoint14(c,a.output,"checkpoint-"+std::to_string(w.ticks()),w,manifest);
        compare_expected();
    }}catch(const std::exception& error){stop_reason=error.what();}
    try{q::authority(w);q::worker_oracle(w);checkpoint14(c,a.output,"result",w,manifest);compare_expected();}
    catch(const std::exception& error){write(a.output/"failed-run.json",{{"technical_status","FAIL"},{"tick",w.ticks()},
        {"tick_or_observer_error",stop_reason.empty()?Json(nullptr):Json(stop_reason)},{"authority_save_or_comparison_error",error.what()},
        {"previous_checked_checkpoints",manifest},{"commands",commands},{"last_refused_command",blocked},
        {"paid_elite17_certified",false},{"retention_certified",false},{"native_human","NOT_RUN"}});throw;}
    if(resumed){check(w.ticks()==requested_end,"Replay stopped before creator endpoint");check(q::f::read_text(a.output/"result.save.json")==expected_bytes,"Separate-process final complete SaveDocument bytes differ");
        for(const auto& entry:expected_manifest){const auto tick=entry.at("tick").get<std::uint64_t>();if(tick>=begin&&tick<=requested_end)check(compared.contains(entry.at("file").get<std::string>()),"Replay omitted an in-range complete checkpoint comparison");}}
    q::save::RecoveryStore recovery(a.output/"recovery-app",c.data,q::save::recovery_storage_budget_bytes,q::save::RecoveryAccess::OriginalProduction);
    const auto history=recovery.create_history(c.document(w),c.mask);const auto entry=recovery.write_checkpoint(history,c.document(w),c.mask);
    check(entry.loadable()&&entry.schema==30&&entry.rule_version==14,"Recovery lost exact Rule14/schema30");auto recovered=restore14(c,q::save::read_save(entry.path));
    check(recovered.snapshot()==w.snapshot()&&q::f::read_text(entry.path)==q::f::read_text(a.output/"result.save.json"),"Complete Recovery bytes or snapshot differ");
    const bool retained_pass=first&&w.ticks()==*first+20400&&lost==0&&unmet==0&&evaluations==400;
    Json retention=retained?retained->report(w):Json(nullptr);std::int64_t cashflow=0;if(retained)cashflow=retention.at("city").at("summary").at("operating_cashflow");
    const auto financial_ticks=retained?w.ticks()-std::max(begin,*first):0;
    write(a.output/"city-metrics.json",city.report(w));if(observed)write(a.output/"elite-metrics.json",observed->report(w));
    if(transported)write(a.output/"transport-metrics.json",transported->report(w));if(retained)write(a.output/"retention-metrics.json",retention);
    write(a.output/"commands.json",commands);write(a.output/"checkpoints.json",manifest);write(a.output/"evaluation-provenance.json",evaluation_provenance);
    if(diagnosis)write(a.output/"oe44-diagnosis.json",diagnosis->finish(w));
    check(source_pins(a.data)==pins,"Original input bytes/modes/mtimes changed");if(resumed)check(q::f::read_text(a.input)==input&&q::f::read_text(a.expected)==expected_bytes,"Replay changed protected creator files");
    const Json outcome= !stop_reason.empty()?Json(stop_reason):!first?Json("Bounded search ended without actual Elite16-to-17; final paid population, phase gate, pending purchases and unmet needs are retained"):
        !retained_pass?Json("Actual Elite17 entry retained, but this observed interval does not certify 20400 uninterrupted ticks with 400 needs-met evaluations"):Json(nullptr);
    write(a.output/"report.json",{{"format","openemperor-paid-elite-desirability-v1"},{"technical_status",stop_reason.empty()?"PASS":"PARTIAL"},
        {"mode",a.mode},{"variant",a.variant},{"rule",14},{"schema",30},{"policy",3},{"revision",27},{"snapshot_injection",false},
        {"begin_tick",begin},{"end_tick",w.ticks()},{"paid_elite17",first?"PASS":"PARTIAL"},{"retention",first?(retained_pass?"PASS":"PARTIAL"):"NOT_RUN"},
        {"first_entry",first_entry},{"retention_observation_begin",retained?Json(std::max(begin,*first)):Json(nullptr)},
        {"retention_lost_ticks",lost},{"retention_current_unmet",unmet},{"retention_next_unmet",next_unmet},{"retention_evaluations",evaluations},
        {"operating_cashflow",cashflow},{"minimum_retention_funds",retain_min},{"financial_observed_ticks",financial_ticks},
        {"financial_stability",retained?(financial_ticks<20400?"PARTIAL":cashflow>=0&&retain_min>=0?"PASS":"FAIL"):"NOT_RUN"},
        {"actual_blocker",outcome},{"last_refused_command",blocked},{"final_plan_and_actual_needs",strategy::status(w,a.variant)},
        {"final_provenance",target?provenance(w,*target):Json(nullptr)},{"complete_recovery_equality",true},
        {"separate_process_comparison",resumed?"PASS":"NOT_RUN"},{"checkpoint_comparisons",comparisons},{"source_pins",pins},
        {"input_bytes_modes_mtimes_unchanged",true},{"original_executable_ran",false},{"native_human","NOT_RUN"}});
    std::cout<<"OE40 actual Elite17="<<(first?"reached":"not reached")<<" retention="<<retained_pass<<" tick="<<w.ticks()<<'\n';
}
void render14(const Options& a){
    const auto pins=source_pins(a.data);const auto c=q::Context::real(a.data);const auto input=q::f::read_text(a.input);const auto d=q::save::read_save(a.input);
    auto restored=restore14(c,d);q::authority(restored);const auto target=strategy::elite(restored);check(target.has_value(),"Render save has no actual paid Elite family");
    fs::create_directories(a.output);const auto copy=a.output/"view.save.json";q::f::write_text(copy,input);Sdl sdl;
    auto session=oe::maps::load_stored_map_session(c.data,c.map_relative,oe::maps::FootprintPolicy::EdgeByte4x4Preview,oe::maps::StoredGraphicsProfile::Slot8,
        oe::maps::GreatWallPresentationMode::Automatic,{},true);
    auto view=oe::SandboxView::create_original_cardinal_water(std::move(session),orig::Difficulty::Normal,false);view->configure_save(c.data,c.map_relative,copy);
    const auto art=oe::detect_and_select_visual_profiles(c.data,a.resources);
    if(!art.walker.empty())view->set_walker_visuals(art.walker,art.walker_source);if(!art.building.empty())view->set_building_visuals(art.building,art.building_source);
    if(!art.road.empty())view->set_road_visuals(art.road,art.road_source);view->set_fire_visuals(art.fire,art.fire_source,art.fire_fallback_reason);
    view->set_fire_inspector_visuals(art.fire_inspector,art.fire_inspector_source,art.fire_inspector_fallback_reason);
    view->set_market_walker_visuals(art.market_walker,art.market_walker_source,art.market_walker_fallback_reason);
    view->set_service_walker_visuals(art.service_walker,art.service_walker_source,art.service_walker_fallback_reason);
    view->set_health_walker_visuals(art.health_walker,art.health_walker_source,art.health_walker_fallback_reason);
    view->set_original_house_visuals(art.original_house,art.original_house_source,art.original_house_fallback_reason);
    view->initialize(sdl.window,sdl.renderer);key(*view,SDLK_F9);check(view->paused()&&view->world().snapshot()==d.world,"Actual Rule14 View F9 differs");
    key(*view,SDLK_F5);check(q::f::read_text(copy)==input,"Actual Rule14 View F5 canonical bytes differ");
    SDL_Event focus{};focus.type=SDL_EVENT_WINDOW_FOCUS_GAINED;bool running=true;view->handle_event(focus,running);
    const auto visual=view->original_house_visual_diagnostic(*target);check(visual.applicable&&std::isfinite(visual.ground.x)&&std::isfinite(visual.ground.y),"Actual Elite renderer lacks a finite production anchor");
    const auto at=visual.ground;Json captures=Json::array();
    for(unsigned zoom:{1U,2U,4U}){for(int n=0;n<3&&view->camera().zoom!=zoom;++n)key(*view,SDLK_Z);check(view->camera().zoom==zoom,"Actual View zoom failed");
        center(*view,at);view->update(0);check(view->render(),"Actual Rule14 SandboxView render failed");
        const auto drawn=view->original_house_display_stats();const auto slot=static_cast<unsigned>(visual.house_type)-2U;
        const auto original_draws=slot<drawn.drawn_instances.size()?drawn.drawn_instances[slot]:0;
        const auto fallback_draws=slot<drawn.authored_fallbacks.size()?drawn.authored_fallbacks[slot]:0;
        const char* draw_status=original_draws>0&&fallback_draws==0?"ORIGINAL_DRAW_OBSERVED":fallback_draws>0?"AUTHORED_FALLBACK_DRAW_OBSERVED":"TARGET_TYPE_DRAW_NOT_OBSERVED";
        std::unique_ptr<SDL_Surface,decltype(&SDL_DestroySurface)> image(SDL_RenderReadPixels(sdl.renderer,nullptr),SDL_DestroySurface);
        const auto file=a.output/("sandbox-"+std::to_string(zoom)+"x.png");check(image&&SDL_SavePNG(image.get(),file.string().c_str()),"Cannot capture actual Rule14 View");
        check(view->world().snapshot()==d.world&&view->paused(),"Capture changed World authority");
        key(*view,SDLK_F5);key(*view,SDLK_F9);check(view->world().snapshot()==d.world&&view->paused()&&q::f::read_text(copy)==input,"Per-zoom actual F5/F9 changed complete schema30 bytes or World");
        captures.push_back({{"file",file.filename().string()},{"zoom",zoom},{"tick",view->world().ticks()},{"complete_f5_f9_equality",true},
            {"target_type_original_draws",original_draws},{"target_type_authored_fallbacks",fallback_draws},{"actual_draw_status",draw_status},
            {"frame_original_house_art_configured",drawn.configured},{"frame_original_house_art_active",drawn.active}});}
    key(*view,SDLK_F9);key(*view,SDLK_F5);check(view->world().snapshot()==d.world&&q::f::read_text(copy)==input&&q::f::read_text(a.input)==input,"Post-capture F5/F9 differs");view->shutdown();
    check(source_pins(a.data)==pins,"Rendering changed original inputs");write(a.output/"report.json",{{"format","openemperor-elite-desirability-view-v1"},{"status","PASS"},
        {"rule",14},{"schema",30},{"actual_sandbox_renderer",true},{"backend",SDL_GetRendererName(sdl.renderer)},{"complete_f5_f9_equality",true},
        {"target",static_cast<unsigned>(*target)},{"actual_type",visual.house_type},{"actual_side",visual.actual_side},
        {"original_house_art_configured",visual.configured},{"original_house_art_active",visual.active},{"original_house_asset_selected",visual.asset.has_value()},
        {"original_house_fallback_reason",visual.fallback_reason},{"captures",captures},{"captures_visually_inspected",false},
        {"world_unchanged",true},{"source_inputs_unchanged",true},{"native_human","NOT_RUN"}});
}
}
int main(int argc,char** argv){try{if(argc==2&&std::string_view(argv[1])=="--help"){std::cout<<oe40_cli::help<<'\n';return 0;}
    const auto args=oe40_cli::options(argc,argv);if(args.mode=="render")oe40_cli::render14(args);else oe40_cli::simulate(args);return 0;
}catch(const std::exception& error){std::cerr<<"Elite desirability proof FAIL: "<<error.what()<<'\n';return 1;}}
