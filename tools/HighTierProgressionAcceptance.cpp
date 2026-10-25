// The OE37 baseline protocol is deliberately reused without another simulator.
// Only this executable's explicit private evidence directory differs.
#define OE_SUSTAINABLE_PROOF_DIRECTORY "oe38"
#define main oe37_baseline_main
#include "SustainableEconomyAcceptance.cpp"
#undef main
#undef OE_SUSTAINABLE_PROOF_DIRECTORY

#include "../tests/HighTierProgressionMatrix.h"

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
    fs::create_directories(output);write(output/"matrix.json",report);
    q::save::write_save(output/"matrix-world.save.json",c.document(w),c.data,c.mask);
    if(input)check(q::f::read_text(*input)==input_bytes,"Matrix changed earned source save");
    std::cout<<"OE38 matrix technical PASS; high-tier reachability unproven\n";return 0;
}
}

int main(int argc,char** argv) {
    try {
        for(int i=1;i+1<argc;++i)if(std::string_view(argv[i])=="--mode"&&std::string_view(argv[i+1])=="matrix")
            return oe38_cli::matrix(argc,argv);
        if(argc==2&&std::string_view(argv[1])=="--help") {
            std::cout<<oe38_cli::matrix_usage<<"\nBaseline modes also accept the sustainable-economy arguments; all evidence stays under .local/original-economy/oe38/.\n";return 0;
        }
        return oe37_baseline_main(argc,argv);
    }catch(const std::exception& e){std::cerr<<"High-tier progression FAIL: "<<e.what()<<'\n';return 1;}
}
