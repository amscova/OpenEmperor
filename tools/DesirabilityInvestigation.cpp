#include "../tests/DesirabilityInvestigation.h"
#include <iostream>
#include <map>

namespace q=oe_desirability_investigation;
namespace fs=std::filesystem;
void safe(const fs::path& path) {
    q::check(path.is_absolute(),"Use absolute diagnostic paths");fs::path at;
    for(const auto& part:path){q::check(part!="."&&part!="..","Diagnostic path must be canonical");at/=part;
        std::error_code ec;const auto status=fs::symlink_status(at,ec);q::check(!ec||ec==std::errc::no_such_file_or_directory,"Cannot inspect diagnostic path");
        q::check(!fs::is_symlink(status),"Diagnostic paths must not traverse symlinks");}
}
int main(int argc,char** argv) {
    try {
        std::map<std::string,fs::path> args;
        for(int i=1;i<argc;i+=2){q::check(i+1<argc,"Use --data ABS --output NEW_OE40_DIRECTORY [--packing ABS_PRIVATE_JSON]");const std::string key=argv[i];
            q::check(key=="--data"||key=="--output"||key=="--packing","Unknown desirability diagnostic argument");q::check(args.emplace(key,argv[i+1]).second,"Duplicate diagnostic argument");}
        q::check(args.contains("--data")&&args.contains("--output"),"Required --data and --output missing");
        for(const auto& [key,path]:args){(void)key;safe(path);}
        const auto repo=fs::canonical(fs::path(__FILE__).parent_path().parent_path()),private_root=repo/".local/original-economy/oe40";
        const auto output=args.at("--output"),data=args.at("--data");
        const auto contained=[](const fs::path& child,const fs::path& parent){auto a=child.begin();for(auto b=parent.begin();b!=parent.end();++a,++b)if(a==child.end()||*a!=*b)return false;return true;};
        q::check(output!=private_root&&contained(output,private_root)&&!fs::exists(output),"Preserve previous evidence; use a new ignored OE40 directory");
        q::check(!contained(output,data)&&!contained(data,output),"Research output must not overlap original inputs");
        const auto model_file=data/"Model/EmperorBuildingModels.txt",exe_file=data/"Emperor.exe";
        safe(model_file);safe(exe_file);
        const auto model_hash=q::file_hash(model_file,openemperor::original::max_model_bytes);
        const auto exe_hash=fs::is_regular_file(exe_file)?q::file_hash(exe_file):std::string{};
        const auto model_mode=fs::status(model_file).permissions();const auto model_time=fs::last_write_time(model_file);
        const auto exe_mode=exe_hash.empty()?fs::perms::unknown:fs::status(exe_file).permissions();
        const auto exe_time=exe_hash.empty()?fs::file_time_type{}:fs::last_write_time(exe_file);
        q::check(model_hash==q::model_sha,"This investigation requires the pinned unchanged original Normal model");
        const auto raw=q::orig::load_model_data(data);q::check(bool(raw),"Shared ModelData reader refused source");
        const auto model=q::orig::OriginalRulesModel::prepare_production(*raw.model,q::orig::Difficulty::Normal);
        auto rule12=q::sim::World::create_original_sustainable_economy(q::authored_plane(),model);
        auto rule13=q::sim::World::create_original_market_logistics(q::authored_plane(),model);
        const auto before12=rule12.snapshot(),before13=rule13.snapshot();
        const auto proof12=q::certificate(rule12),proof13=q::certificate(rule13);
        q::check(proof12.at("historical_checker_raw_report").at("source_identities")==proof13.at("historical_checker_raw_report").at("source_identities"),"OE39 changed the DES source inventory");
        q::Json report{{"format","openemperor-oe40-desirability-investigation-v1"},{"status",exe_hash==q::exe_sha?"PASS":"BLOCKED"},
            {"mode","READ_ONLY_RESEARCH_NO_GAMEPLAY_AUTHORITY"},{"model_sha256",model_hash},{"exe_sha256",exe_hash.empty()?q::Json(nullptr):q::Json(exe_hash)},
            {"exe_pin_matches",exe_hash==q::exe_sha},{"source_executable_ran",false},{"rule12_certificate",proof12},{"rule13_certificate",proof13},
            {"source_domains_equal",true},{"candidate_register",q::model_register(*raw.model,rule13)},{"pure_controls",q::pure_controls()},
            {"raw_model_reading","Shared ModelData API over complete bounded model ID domain0..4096; no parallel parser."},
            {"original_runtime_parity","PARTIAL"},{"paid_elite17","NOT_RUN"},{"new_generation_created",false},{"new_source_admitted",false},
            {"original_source_manager_unknowns",{"full placement/status/link lifecycle","rectangle selector and restoration admission","complete negative wall sectors and clipped edges","World map identity for original terrain-water flag"}}};
        if(args.contains("--packing")) {
            const auto path=args.at("--packing");q::check(contained(path,private_root)&&fs::is_regular_file(path)&&fs::file_size(path)<=65536,"Use an explicit bounded private packing JSON");
            std::ifstream in(path);q::Json packing;in>>packing;report["constructive_pure_packing"]=q::packing_report(rule13,packing);
        }
        q::check(rule12.snapshot()==before12&&rule13.snapshot()==before13&&rule13.buildings().empty()&&rule13.couriers().empty()&&rule13.ticks()==0&&rule13.total_population()==0,
            "Research query assigned gameplay or changed a fresh World");report["worlds_unchanged"]=true;
        q::check(q::file_hash(model_file,openemperor::original::max_model_bytes)==model_hash&&fs::status(model_file).permissions()==model_mode&&fs::last_write_time(model_file)==model_time,"Original model changed during research");
        if(!exe_hash.empty())q::check(q::file_hash(exe_file)==exe_hash&&fs::status(exe_file).permissions()==exe_mode&&fs::last_write_time(exe_file)==exe_time,"Original EXE changed during research");
        report["input_bytes_modes_mtimes_unchanged"]=true;fs::create_directories(output.parent_path());q::check(fs::create_directory(output),"Preserve any concurrently created output directory");
        std::ofstream out(output/"investigation.json",std::ios::binary);out<<report.dump(2)<<'\n';q::check(out.good(),"Cannot write private research report");
        std::cout<<"OE40 read-only "<<report.at("status").get<std::string>()<<"; "<<raw.model->building_count()<<" model candidates; Rule12 and13 source core upper79<80; no paid Elite claim\n";
        return exe_hash==q::exe_sha?0:77;
    }catch(const std::exception& e){std::cerr<<"OE40 investigation FAIL: "<<e.what()<<'\n';return 1;}
}
