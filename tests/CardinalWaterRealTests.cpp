// Small real-Xia paid adapter acceptance. This is not a high-tier progression,
// sustainable-economy, original-runtime or native-human acceptance claim.
#include "OriginalStarterFixture.h"
#include <openssl/evp.h>
#include <array>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>

namespace {
namespace f=oe_starter_fixture;
namespace sim=f::sim;
namespace save=f::save;
namespace fs=std::filesystem;
using Json=nlohmann::json;
using f::check;
constexpr const char* model_pin="7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6";
constexpr const char* exe_pin="6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e";
std::string hash(const fs::path& path) {
    check(fs::is_regular_file(path)&&!fs::is_symlink(fs::symlink_status(path)),"Source pin needs a regular non-symlink file");
    std::ifstream in(path,std::ios::binary);check(in.good(),"Cannot read pinned source");
    std::unique_ptr<EVP_MD_CTX,decltype(&EVP_MD_CTX_free)> digest(EVP_MD_CTX_new(),EVP_MD_CTX_free);
    check(digest&&EVP_DigestInit_ex(digest.get(),EVP_sha256(),nullptr)==1,"Cannot initialize source digest");
    std::array<char,65536> bytes{};
    while(in.read(bytes.data(),static_cast<std::streamsize>(bytes.size()))||in.gcount())
        check(EVP_DigestUpdate(digest.get(),bytes.data(),static_cast<std::size_t>(in.gcount()))==1,"Source digest failed");
    check(in.eof()&&!in.bad(),"Source digest read failed");
    std::array<unsigned char,32> output{};unsigned size=0;
    check(EVP_DigestFinal_ex(digest.get(),output.data(),&size)==1&&size==output.size(),"Source digest finalization failed");
    std::ostringstream out;out<<std::hex<<std::setfill('0');for(auto byte:output)out<<std::setw(2)<<unsigned(byte);return out.str();
}
Json pins(const fs::path& data) {
    Json result=Json::object();
    for(const auto* relative:{"Model/EmperorBuildingModels.txt","Cities/Xia.map","Emperor.exe"}) {
        const auto path=data/relative;
        result[relative]={{"sha256",hash(path)},{"bytes",fs::file_size(path)},
            {"mode",static_cast<unsigned>(fs::status(path).permissions())},
            {"mtime",fs::last_write_time(path).time_since_epoch().count()}};
    }
    check(result.at("Model/EmperorBuildingModels.txt").at("sha256")==model_pin&&result.at("Emperor.exe").at("sha256")==exe_pin,
        "Real adapter smoke requires the pinned unmodified original model and EXE");return result;
}
bool below(const fs::path& child,const fs::path& parent) {
    auto a=child.begin();for(auto b=parent.begin();b!=parent.end();++a,++b)if(a==child.end()||*a!=*b)return false;return true;
}
void safe_absolute(const fs::path& path) {
    check(path.is_absolute(),"Use absolute smoke-test paths");fs::path at;
    for(const auto& part:path){check(part!="."&&part!="..","Smoke-test path must be canonical");at/=part;
        std::error_code ec;const auto status=fs::symlink_status(at,ec);
        check(!ec||ec==std::errc::no_such_file_or_directory,"Cannot inspect smoke-test path");
        check(!fs::is_symlink(status),"Smoke-test paths must not traverse symlinks");}
}
void accounting(const sim::World& w) {
    check(w.navigation_valid()&&w.population_valid()&&w.production_balance_valid()&&w.city_economy_valid()&&w.fire_state_valid(),
        "Real paid smoke broke authoritative navigation/population/production/finance/fire");
    std::uint64_t maintenance=0,tax=0;
    for(const auto& body:w.buildings()) {
        check(body.placed_tick<=w.ticks()&&w.workforce_required(body.id)>=0,"Invalid installed paid owner");
        maintenance+=(w.ticks()-body.placed_tick)/w.original_worker_upkeep_interval()*
            2*static_cast<std::uint64_t>(w.workforce_required(body.id));tax+=body.taxes_paid_total;
    }
    check(maintenance==w.maintenance_spent_total()&&tax==w.taxes_collected_total()&&
        sim::original_finance_accounting_valid(w.treasury(),tax,w.construction_spent_total(),maintenance),
        "Actual Funds differ from paid construction, collected tax or installed upkeep");
    for(const auto& balance:w.original_goods_balances())check(balance.conserved&&!balance.totals.imported,
        "Real paid smoke granted or lost physical goods");
    (void)f::food_facts(w);
}
struct Edge {sim::Cell outside,inside;};
std::vector<Edge> edges(sim::Cell at,int side) {
    std::vector<Edge> result;for(int d=0;d<side;++d) {
        result.push_back({{at.x+d,at.y-1},{at.x+d,at.y}});
        result.push_back({{at.x+side,at.y+d},{at.x+side-1,at.y+d}});
        result.push_back({{at.x+d,at.y+side},{at.x+d,at.y+side-1}});
        result.push_back({{at.x-1,at.y+d},{at.x,at.y+d}});
    }return result;
}
sim::CommandResult purchase(sim::World& w,sim::Command command,Json& log) {
    const auto before=w.treasury();const auto spent=w.construction_spent_total();const auto cost=w.construction_cost(command.type);
    check(cost>0,"Real adapter smoke purchase has no positive authoritative cost");const auto result=w.execute(command);
    check(result.accepted&&result.changed,"Real paid smoke command refused: "+result.reason);
    check(w.treasury()==before-cost&&w.construction_spent_total()==spent+static_cast<std::uint64_t>(cost),
        "Real paid smoke did not debit the normal price exactly once");
    log.push_back({{"type",static_cast<unsigned>(command.type)},{"cell",{command.cell.x,command.cell.y}},
        {"cost",cost},{"tick",w.ticks()},{"sequence",w.command_sequence()},{"funds_before",before},{"funds_after",w.treasury()}});
    accounting(w);return result;
}
std::pair<sim::Cell,sim::Cell> common_site(const sim::World& w) {
    const auto facts=w.original_terrain_facts();const auto policy=w.map_permissions();
    for(int y=1;y<225;++y)for(int x=1;x<225;++x) {
        const sim::Cell at{x,y};
        // Scenario reservation only: keep the later explicit Elite probe clear.
        // This is not a map permission, game admission list or coordinate grant.
        if(x>=122&&x<=140&&y>=125&&y<=143)continue;
        if(facts->cardinal_water_bonus(at,2)!=10||!w.validate({sim::CommandType::PlaceHousehold,at}).accepted)continue;
        for(const auto& edge:edges(at,2)) {
            const auto result=w.validate({sim::CommandType::PlaceRoad,edge.outside});
            if(result.accepted&&result.changed&&policy->transport_edge_allowed(edge.inside,edge.outside))return {at,edge.outside};
        }
    }
    throw std::runtime_error("No actual-Xia paid Common shoreline body with a legal new shared-edge Road");
}
Json state(const sim::World& w,sim::BuildingId common,sim::BuildingId elite) {
    Json homes=Json::array();for(auto id:{common,elite}) {
        const auto& b=w.building(id);const auto& h=f::house(w,id);const auto provenance=w.original_influence_provenance(id);
        homes.push_back({{"id",static_cast<unsigned>(id)},{"object",static_cast<unsigned>(b.kind)},
            {"cell",{b.cell.x,b.cell.y}},{"side",sim::building_footprint(w.profile(),w.rule_version(),b.kind).width},
            {"type",static_cast<unsigned>(h.type)},{"occupants",h.occupants},{"evaluated_des",int(h.desirability)},
            {"raw_des",provenance.raw_value},{"water_bonus",provenance.cardinal_water_bonus},{"query_des",provenance.value},
            {"burning",w.building_on_fire(id)}});
    }
    return {{"tick",w.ticks()},{"funds",w.treasury()},{"construction",w.construction_spent_total()},
        {"taxes",w.taxes_collected_total()},{"upkeep",w.maintenance_spent_total()},{"population",w.total_population()},
        {"rule",w.rule_version()},{"revision",w.original_production_state()->development_revision},{"houses",homes}};
}
Json run(const fs::path& data,const fs::path& output) {
    const auto before=pins(data);auto context=f::Context::real(data);
    const auto terrain=save::prepare_original_terrain_facts(context.data,context.map_relative);
    check(terrain->source_map_sha256()==before.at("Cities/Xia.map").at("sha256").get<std::string>(),"Prepared real terrain differs from the pinned source map");
    auto w=sim::World::create_original_cardinal_water(context.policy,context.model,terrain);
    check(w.rule_version()==14&&w.ticks()==0&&w.total_population()==0&&w.buildings().empty()&&w.couriers().empty()&&
        w.treasury()==sim::original_production_starting_treasury&&w.original_worker_upkeep_interval()==816,
        "Real adapter smoke did not start a fresh Rule14/Normal city");
    check(context.model->provenance().sha256==model_pin&&context.model->difficulty()==f::orig::Difficulty::Normal,
        "Real adapter smoke silently used an authored or different-difficulty model");
    Json purchases=Json::array();const auto [at,road]=common_site(w);
    purchase(w,{sim::CommandType::PlaceHousehold,at},purchases);const auto common=*w.building_owner_at(at);
    purchase(w,{sim::CommandType::PlaceRoad,road},purchases);
    check(w.original_house_cardinal_water_bonus(common)==10&&!w.building_entrances(common).empty(),
        "Actual paid Common lacks its raw-water bonus or legal adjacent entrance");
    const sim::Cell elite_at{130,133};purchase(w,{sim::CommandType::PlaceOriginalElitePlot,elite_at},purchases);
    const auto elite=*w.building_owner_at(elite_at);
    check(w.original_house_cardinal_water_bonus(elite)==0&&w.building(elite).kind==sim::Object::OriginalElitePlot,
        "Pending real2x2 Elite plot received a4x4 bonus or free materialization");
    bool road_bought=false;for(const auto& edge:edges(elite_at,4)) {
        const auto possible=w.validate({sim::CommandType::PlaceRoad,edge.outside});
        if(possible.accepted&&possible.changed&&context.policy->transport_edge_allowed(edge.inside,edge.outside)) {
            purchase(w,{sim::CommandType::PlaceRoad,edge.outside},purchases);road_bought=true;break;
        }
    }
    check(road_bought&&w.original_elite_materialization_status(elite).ready,
        "Actual paid Elite plot lacks a fully legal4x4 materialization/road");
    const auto paid=state(w,common,elite);
    while(w.ticks()<119){w.tick();accounting(w);}
    check(!w.total_population()&&w.original_house_cardinal_water_bonus(elite)==0,"Real immigration/materialization occurred before age120");
    w.tick();accounting(w);
    check(w.building(elite).kind==sim::Object::OriginalEliteHouse&&w.original_house_cardinal_water_bonus(elite)==10&&
        f::house(w,elite).occupants>0&&f::house(w,common).occupants>0,
        "Actual120t admissions did not materialize a paid water-adjacent4x4 Elite");
    const auto admitted=state(w,common,elite);while(w.ticks()<200){w.tick();accounting(w);}
    const auto document=context.document(w);const auto checkpoint=output/"tick200.json";
    save::write_save(checkpoint,document,context.data,context.mask);const auto checkpoint_bytes=f::f::read_text(checkpoint);
    const auto parsed=save::read_save(checkpoint);check(parsed.source_schema_version==30&&parsed.world==w.snapshot(),"Real Schema30 read-back lost complete World");
    auto resumed=save::restore_original_cardinal_water(parsed,context.data,context.mask,context.policy,context.model);
    check(resumed.snapshot()==w.snapshot()&&resumed.canonical_state()==w.canonical_state()&&*resumed.original_terrain_facts()==*terrain,
        "Real source-rederived restore differs from full original World/context");
    save::write_save(output/"tick200-rewrite.json",context.document(resumed),context.data,context.mask);
    check(f::f::read_text(output/"tick200-rewrite.json")==checkpoint_bytes,"Real full Schema30 SaveDocument rewrite differs");
    const auto evaluated=state(w,common,elite);
    while(w.ticks()<400) {
        w.tick();resumed.tick();accounting(w);accounting(resumed);
        check(w.snapshot()==resumed.snapshot()&&w.canonical_state()==resumed.canonical_state(),"Real complete200-tick resumed continuation diverged");
    }
    save::write_save(output/"tick400.json",context.document(w),context.data,context.mask);
    save::write_save(output/"tick400-resumed.json",context.document(resumed),context.data,context.mask);
    check(f::f::read_text(output/"tick400.json")==f::f::read_text(output/"tick400-resumed.json"),"Real full final SaveDocuments differ");
    f::f::write_text(output/"tick400.snapshot.txt",w.canonical_state());
    const auto after=pins(data);check(before==after,"Original Model/Xia/EXE bytes, sizes, modes or mtimes changed");
    return {{"format","openemperor-oe40-real-paid-adapter-smoke-v1"},{"status","PASS"},
        {"scope","Actual paid Xia Rule14/Schema30/Normal recipient-adapter smoke only"},{"original_map",true},{"original_model",true},
        {"original_executable_run",false},{"direct_grants",false},{"terrain_mutated",false},{"pure_packing_used",false},
        {"source_pins_before",before},{"source_pins_after",after},{"source_files_unchanged",true},
        {"policy",3},{"schema",30},{"contract",sim::original_cardinal_water_contract},{"purchases",purchases},
        {"paid_tick0",paid},{"actual_admission_tick120",admitted},{"evaluated_tick200",evaluated},{"continued_tick400",state(w,common,elite)},
        {"full_checkpoint_rewrite_equal",true},{"full_continuation_equal_each_tick",true},{"full_final_savedocuments_equal",true},
        {"continuation_ticks",200},{"same_process_restore",true},{"separate_process_restore","NOT_RUN"},
        {"paid_elite17","NOT_RUN"},{"longterm_retention","NOT_RUN"},{"native_human","NOT_RUN"}};
}
}
int main(int argc,char** argv) {
    fs::path output;bool disposable=false;
    try {
        const auto repo=fs::canonical(fs::path(__FILE__).parent_path().parent_path()),private_root=repo/".local/original-economy/oe40";
        fs::path data;
        if(argc==3&&std::string_view(argv[1])=="--real") {
            data=argv[2];disposable=true;output=private_root/("real-adapter-smoke-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        } else {
            check(argc==5,"Use --data ABS --output NEW_OE40_DIRECTORY, or --real ABS_DATA");std::map<std::string,fs::path> args;
            for(int i=1;i<argc;i+=2){const std::string key=argv[i];check(key=="--data"||key=="--output","Unknown real smoke argument");
                check(args.emplace(key,argv[i+1]).second,"Duplicate real smoke argument");}
            check(args.contains("--data")&&args.contains("--output"),"Missing real smoke data/output");data=args.at("--data");output=args.at("--output");
        }
        safe_absolute(data);safe_absolute(output);check(output!=private_root&&below(output,private_root)&&!fs::exists(output),"Preserve old evidence; choose a new ignored OE40 output");
        check(!below(output,data)&&!below(data,output),"Real smoke output overlaps original source");
        if(!fs::is_regular_file(data/"Model/EmperorBuildingModels.txt")||!fs::is_regular_file(data/"Cities/Xia.map")||!fs::is_regular_file(data/"Emperor.exe")) {
            std::cout<<"BLOCKED: exact supplied original Model/Xia/EXE unavailable\n";return 77;
        }
        fs::create_directories(output.parent_path());check(fs::create_directory(output),"Refuse concurrently created evidence directory");
        const auto report=run(data,output);f::f::write_text(output/"report.json",report.dump(2)+"\n");
        std::cout<<"PASS: actual paid Xia Common+Elite water adapter,120t materialization,400 real ticks and full Schema30 save continuation; no Elite17 claim\n";
        if(disposable)fs::remove_all(output);return 0;
    }catch(const std::exception& error){std::cerr<<"Real cardinal-water smoke FAIL: "<<error.what()<<'\n';return 1;}
}
