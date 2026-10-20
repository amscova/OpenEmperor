#include "OriginalRecoveryFixture.h"
#include "OriginalFrozenSaveCorpusFixture.h"
#include <cctype>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
#include <set>

namespace {
namespace q=oe_recovery_fixture;
namespace f=oe_influence_fixture;
namespace sim=openemperor::simulation;
namespace save=openemperor::persistence;
namespace orig=openemperor::original;
namespace fs=std::filesystem;
using Json=nlohmann::json;
using Pointer=Json::json_pointer;
using f::check;
using oe_frozen_save_corpus::sha256;
std::string normalized(const std::string& path) {
    std::string result;std::size_t begin=0;
    while(begin<path.size()) {
        const auto end=path.find('/',begin+1);const auto part=path.substr(begin+1,end==std::string::npos ? end:end-begin-1);
        const bool index=!part.empty() && std::all_of(part.begin(),part.end(),[](unsigned char c){return std::isdigit(c)!=0;});
        result+='/';result+=index ? "*":part;
        if(end==std::string::npos)break;
        begin=end;
    }
    return result;
}
struct CorpusDocument {
    std::string label,bytes;Json raw;save::SaveDocument document;
    std::shared_ptr<const sim::World> protected_world;
};
struct Location {std::size_t corpus=0;std::string path;};
struct Suite {
    const q::Context& context;
    fs::path output;
    bool persistent=false;
    std::vector<CorpusDocument> corpus;
    Json cases=Json::array(),positives=Json::array();
    std::size_t parsed_refusals=0,restore_refusals=0;
    explicit Suite(const q::Context& c,fs::path directory,bool keep):context(c),output(std::move(directory)),persistent(keep) {
        check(!fs::exists(output),"Adversarial output must be new");
        save::validate_save_target(output/"report.json",context.data);fs::create_directories(output);
    }
    void add(const q::Checkpoint& point) {
        const auto path=output/(point.label+".good.save.json");
        save::write_save(path,point.document,context.data,context.mask);
        const auto bytes=f::read_text(path);const auto parsed=save::read_save(path);q::same_document(point.document,parsed);
        const auto restored=std::make_shared<const sim::World>(q::restore(context,parsed));
        check(restored->snapshot()==point.document.world,"Positive checkpoint did not preserve all authority");
        auto rewrite=parsed;rewrite.prepared_permissions=context.policy;rewrite.prepared_original_rules_model=context.model;
        const auto canonical=output/(point.label+".rewritten.save.json");save::write_save(canonical,rewrite,context.data,context.mask);
        check(f::read_text(canonical)==bytes,"Canonical active save rewrite changed bytes");
        check(f::read_text(path)==bytes,"Positive restore rewrote source save");
        positives.push_back({{"label",point.label},{"tick",point.document.world.ticks},{"bytes",bytes.size()},{"sha256",sha256(bytes)},
            {"full_snapshot_equal",true},{"canonical_bytes_equal",true}});
        corpus.push_back({point.label,bytes,Json::parse(bytes),parsed,restored});
    }
    const CorpusDocument& named(const char* label) const {
        const auto at=std::find_if(corpus.begin(),corpus.end(),[&](const auto& d){return d.label==label;});
        check(at!=corpus.end(),std::string("Missing genuine checkpoint ")+label);return *at;
    }
    void reject_bytes(const CorpusDocument& good,const std::string& bytes,const std::string& pointer,const std::string& mutation) {
        check(bytes!=good.bytes,"Adversarial mutation did not alter input bytes");
        const auto id=cases.size();const auto path=output/(persistent ? "hostile-"+std::to_string(id)+".json":"hostile.json");
        f::write_text(path,bytes);const auto protected_path=output/(good.label+".good.save.json");
        const auto before=good.protected_world->snapshot();
        std::string reason,stage="parse";
        try {const auto parsed=save::read_save(path);stage="restore";(void)q::restore(context,parsed);}
        catch(const std::exception& error){reason=error.what();}
        check(!reason.empty(),"Malformed save admitted: "+good.label+pointer+" "+mutation);
        check(f::read_text(path)==bytes && f::read_text(protected_path)==good.bytes,"Rejected input modified itself or protected good save");
        check(good.protected_world->snapshot()==before && before==good.document.world,
            "Rejected input changed complete protected live World authority");
        if(stage=="parse")++parsed_refusals;else ++restore_refusals;
        cases.push_back({{"id",id},{"checkpoint",good.label},{"pointer",pointer},{"mutation",mutation},{"bytes",bytes.size()},
            {"sha256",sha256(bytes)},{"refused_at",stage},{"reason",reason},{"input_unchanged",true},{"good_save_unchanged",true}});
    }
    void reject_json(const CorpusDocument& good,const std::string& pointer,const std::string& mutation,const std::function<void(Json&)>& change) {
        auto raw=good.raw;change(raw);reject_bytes(good,raw.dump(2),pointer,mutation);
    }
    void fields_and_types() {
        std::map<std::string,Location> objects,scalars,arrays;
        for(std::size_t index=0;index<corpus.size();++index) {
            const std::function<void(const Json&,const std::string&)> visit=[&](const Json& value,const std::string& path) {
                const auto key=normalized(path);
                if(value.is_object()) {
                    objects.try_emplace(key,Location{index,path});
                    for(auto it=value.begin();it!=value.end();++it)visit(it.value(),path+'/'+it.key());
                } else if(value.is_array()) {
                    arrays.try_emplace(key,Location{index,path});
                    for(std::size_t i=0;i<value.size();++i)visit(value[i],path+'/'+std::to_string(i));
                } else if(!value.is_null())scalars.try_emplace(key+'|'+value.type_name(),Location{index,path});
            };
            visit(corpus[index].raw,"");
        }
        for(const auto& [shape,location]:objects) {
            (void)shape;const auto& good=corpus[location.corpus];const Pointer p(location.path);const auto object=good.raw.at(p);
            reject_json(good,location.path,"extra-field",[&](auto& j){j.at(p)["unsupported_OE31_authority"]=true;});
            for(const auto& [key,value]:object.items()) {
                (void)value;reject_json(good,location.path+'/'+key,"missing-field",[&](auto& j){j.at(p).erase(key);});
            }
            for(const auto& value:std::array<Json,2>{Json(nullptr),Json::array()})
                reject_json(good,location.path,"object-type:"+std::string(value.type_name()),[&](auto& j){j.at(p)=value;});
        }
        for(const auto& [shape,location]:scalars) {
            (void)shape;const auto& good=corpus[location.corpus];const Pointer p(location.path);const auto& old=good.raw.at(p);
            const std::vector<Json> bad=old.is_boolean() ? std::vector<Json>{0,"false",nullptr,Json::array()}:
                old.is_string() ? std::vector<Json>{0,true,nullptr,Json::array()}:
                std::vector<Json>{true,0.0,"0",nullptr,Json::array()};
            for(const auto& value:bad)reject_json(good,location.path,"scalar-type:"+std::string(value.type_name()),[&](auto& j){j.at(p)=value;});
        }
        for(const auto& [shape,location]:arrays) {
            (void)shape;const auto& good=corpus[location.corpus];const Pointer p(location.path);
            for(const auto& value:std::array<Json,2>{Json(nullptr),Json::object()})
                reject_json(good,location.path,"array-type:"+std::string(value.type_name()),[&](auto& j){j.at(p)=value;});
        }
    }
    void set(const CorpusDocument& good,const std::string& pointer,Json value,const char* mutation="hostile-value") {
        const Pointer p(pointer);check(good.raw.contains(p),"Required exercised pointer absent: "+pointer);
        reject_json(good,pointer,mutation,[&](auto& j){j.at(p)=value;});
    }
    void semantic_catalog() {
        const auto& rich=named("rich-supplied");const auto& moving=named("moving-reserved");
        for(const auto& [pointer,value]:std::vector<std::pair<std::string,Json>>{
            {"/schema_version",27},{"/rules/version",9},{"/rules/version",11},{"/world/original_production/development_revision",26},
            {"/world/original_production/development_revision",28},{"/world/ticks",UINT64_MAX},{"/world/command_sequence",UINT64_MAX},
            {"/world/width",513},{"/world/height",0},{"/world/next_building_id",0},{"/world/next_courier_id",UINT32_MAX},
            {"/world/original_economy/model_source/sha256",std::string(64,'0')},{"/world/original_economy/model_source/byte_count",UINT64_MAX},
            {"/world/original_economy/model_source/difficulty",5},{"/profiles/map_permissions/version",2},
            {"/profiles/map_permissions/sha256",std::string(64,'0')},{"/map/sha256",std::string(64,'0')},
            {"/profiles/buildable_sha256",std::string(64,'0')},{"/map/relative_path","../outside.map"},
            {"/world/treasury",INT64_MIN},{"/world/taxes_collected_total",UINT64_MAX},{"/world/maintenance_spent_total",UINT64_MAX},
            {"/world/original_economy/houses/0/building_id",0},{"/world/original_economy/houses/0/building_id",UINT32_MAX},
            {"/world/original_economy/houses/0/house/occupants",32768},{"/world/original_economy/houses/0/house/desirability",-129},
            {"/world/original_economy/houses/0/house/ancestor",256},{"/world/original_economy/houses/0/house/food_quality",256},
            {"/world/original_economy/houses/0/house/ceramics",-1},{"/world/original_goods/stocks/0/amounts/0",-1},
            {"/world/original_goods/stocks/0/amounts/1",INT64_MAX},{"/world/original_goods/totals/0/consumed",UINT64_MAX},
            {"/world/original_production/food/stocks/0/amounts/0",-1},{"/world/original_production/food/stocks/0/received/0",UINT64_MAX},
            {"/world/original_production/services/providers/0/building_id",0},{"/world/original_production/services/providers/0/type",9},
            {"/world/original_production/services/trips/0/target",UINT32_MAX},{"/world/original_production/services/trips/0/dispatched_tick",UINT64_MAX},
            {"/world/original_production/services/witnesses/0/provider_id",UINT32_MAX},{"/world/original_production/services/witnesses/0/role",9},
            {"/world/original_production/services/witnesses/0/arrival_tick",UINT64_MAX},
            {"/world/original_production/water/houses/0/building_id",UINT32_MAX},{"/world/original_production/water/providers/0/building_id",0},
            {"/world/original_production/finance/houses/0/last_assessment/tick",UINT64_MAX},
            {"/world/original_production/finance/houses/0/rows/0/resident_units",UINT64_MAX},
            {"/world/original_production/residents_admitted",UINT64_MAX},{"/world/original_production/pottery_handed_off",UINT64_MAX}})set(rich,pointer,value);
        for(const std::string pointer:{"/world/buildings","/world/couriers","/world/original_economy/houses","/world/original_goods/stocks",
                "/world/original_production/finance/houses","/world/original_production/housing/houses","/world/original_production/services/providers"}) {
            const Pointer p(pointer);check(!rich.raw.at(p).empty(),"Empty required ID collection");
            reject_json(rich,pointer,"duplicate-id-record",[&](auto& j){j.at(p).push_back(j.at(p).front());});
            reject_json(rich,pointer,"missing-required-owner",[&](auto& j){j.at(p).erase(0);});
        }
        for(const std::string pointer:{"/world/original_goods/totals","/world/original_goods/stocks/0/amounts",
                "/world/original_production/house_consumption/0/consumed","/world/original_production/housing/houses/0/first_transitions",
                "/world/original_production/housing/houses/0/consumption_rows","/world/original_production/finance/houses/0/rows"}) {
            const Pointer p(pointer);reject_json(rich,pointer,"fixed-count-minus-one",[&](auto& j){j.at(p).erase(0);});
            reject_json(rich,pointer,"fixed-count-plus-one",[&](auto& j){j.at(p).push_back(j.at(p).front());});
        }
        const auto& cargo=moving.raw.at(Pointer("/world/original_goods/cargo/0"));
        const auto courier=cargo.at("transfer").at("courier_id");const auto& actors=moving.raw.at(Pointer("/world/couriers"));
        const auto at=std::find_if(actors.begin(),actors.end(),[&](const auto& actor){return actor.at("id")==courier;});
        check(at!=actors.end() && at->at("edge_progress").template get<int>()>0,"Typed cargo lacks actual begun actor edge");
        const auto base="/world/couriers/"+std::to_string(at-actors.begin());
        for(const auto& [pointer,value]:std::vector<std::pair<std::string,Json>>{
                {"/world/original_goods/cargo/0/transfer/courier_id",0},{"/world/original_goods/cargo/0/amount",-1},
                {"/world/original_goods/cargo/0/amount",cargo.at("amount").get<int>()+1},{"/world/original_goods/cargo/0/source",UINT32_MAX},
                {"/world/original_goods/reservations/0/loaded",INT32_MAX},{"/world/original_goods/reservations/0/delivered",INT32_MAX},
                {base+"/owner",UINT32_MAX},{base+"/target",UINT32_MAX},{base+"/phase",0},
                {base+"/path_vertex",UINT32_MAX},{base+"/edge_progress",INT32_MAX},{base+"/path/0",Json::array({511,511})}})set(moving,pointer,value,"route-cargo-contradiction");
        const auto house=rich.raw.at(Pointer("/world/buildings/0/cell"));
        set(rich,"/world/buildings/1/cell",house,"physical-owner-overlap");
        set(rich,"/world/buildings/0/cell",Json::array({511,511}),"footprint-outside-grid");
        set(rich,"/world/roads/0",house,"road-through-house-footprint");
    }
    void json_limits() {
        const auto& good=named("rich-supplied");
        reject_bytes(good,"{\"schema_version\":26,"+good.bytes.substr(1),"/schema_version","duplicate-root-key");
        auto escaped=good.bytes;const auto position=escaped.find("\"development_revision\"");check(position!=std::string::npos,"Missing revision key");
        escaped.insert(position,"\"development_\\u0072evision\":27,");
        reject_bytes(good,escaped,"/world/original_production/development_revision","duplicate-escaped-nested-key");
        reject_bytes(good,good.bytes+" {}","","trailing-json-document");
        reject_bytes(good,good.bytes.substr(0,good.bytes.size()/2),"","truncated-document");
        auto deep=good.raw;deep["deep"]=Json::array();auto* p=&deep["deep"];
        for(unsigned n=0;n<34;++n){p->push_back(Json::array());p=&p->at(0);}
        reject_bytes(good,deep.dump(),"/deep","depth-over32");
        constexpr std::size_t limit=8U*1024U*1024U;
        const auto padded=std::string(limit-good.bytes.size(),' ')+good.bytes;
        const auto exact=output/"exact-8mib.json";f::write_text(exact,padded);q::same_document(save::read_save(exact),good.document);
        reject_bytes(good,' '+padded,"","over8MiB");
        auto overflow=good.bytes;const auto key=overflow.find("\"ticks\"");const auto begin=overflow.find(':',key)+1;const auto end=overflow.find(',',begin);
        check(key!=std::string::npos && end!=std::string::npos,"Missing ticks numeric overflow fixture");
        overflow.replace(begin,end-begin,"18446744073709551616");reject_bytes(good,overflow,"/world/ticks","integer-aboveUINT64");
    }
    template<class F>void refused_action(const std::string& pointer,const std::string& mutation,F action) {
        std::string reason;try{action();}catch(const std::exception& error){reason=error.what();}
        check(!reason.empty(),"Direct hostile state admitted: "+mutation);++restore_refusals;
        cases.push_back({{"id",cases.size()},{"pointer",pointer},{"mutation",mutation},{"refused_at","direct-restore-or-write"},{"reason",reason}});
    }
    void provenance_and_atomicity() {
        const auto& good=named("rich-supplied");const auto target=output/(good.label+".good.save.json");
        auto valid=good.document;valid.prepared_permissions=context.policy;valid.prepared_original_rules_model=context.model;
        for(const auto fault:{save::WriteFault::BeforeWrite,save::WriteFault::AfterPartialWrite,save::WriteFault::BeforeRename}) {
            refused_action("","atomic-write-fault",[&]{save::write_save(target,valid,context.data,context.mask,fault);});
            check(f::read_text(target)==good.bytes,"Injected atomic writer fault replaced protected good bytes");
        }
        for(unsigned kind=0;kind<5;++kind) {
            auto bad=valid;
            if(kind==0)bad.source_schema_version=25;
            if(kind==1)bad.map_sha256=std::string(64,'0');
            if(kind==2)bad.world.original_economy->model_source->sha256=std::string(64,'0');
            if(kind==3)++bad.world.original_production->development_revision;
            if(kind==4)++bad.world.treasury;
            refused_action("/world","invalid-DTO-write:"+std::to_string(kind),[&]{save::write_save(target,bad,context.data,context.mask);});
            refused_action("/world","invalid-DTO-restore:"+std::to_string(kind),[&]{(void)q::restore(context,bad);});
            check(f::read_text(target)==good.bytes,"Rejected DTO replaced valid save");
        }
        auto mask=context.mask;check(!mask.empty(),"Missing real mask");mask.front()^=1;
        refused_action("/profiles/buildable_sha256","wrong-prepared-mask",[&]{(void)save::restore_original_production_development(valid,context.data,mask,context.policy);});
        const auto different=save::prepare_original_production_rules_model(context.data,orig::Difficulty::Hard);
        refused_action("/world/original_economy/model_source/difficulty","wrong-prepared-difficulty",[&]{(void)save::restore_original_production_development(valid,context.data,context.mask,context.policy,different);});
        for(unsigned fault=0;fault<2;++fault) {
            std::vector<sim::MapCellPermission> cells;const auto& policy=*context.policy;
            for(int y=0;y<policy.height();++y)for(int x=0;x<policy.width();++x)cells.push_back(*policy.cell_permission({x,y}));
            const auto cell=valid.world.buildings.front().cell;auto& edited=cells[static_cast<std::size_t>(cell.y*policy.width()+cell.x)];
            if(fault==0){edited.protected_original=true;edited.building_allowed=edited.road_allowed=false;edited.building_blocker=edited.road_blocker=sim::BuildBlocker::OriginalStructure;}
            else edited.height=static_cast<std::int8_t>(edited.height==127 ? 126:edited.height+1);
            auto hostile=std::make_shared<const sim::MapPermissions>(policy.width(),policy.height(),3,std::move(cells),policy.gates());
            auto bad=valid;
            if(fault==0)bad.map_permissions_sha256=save::map_permissions_fingerprint(*hostile,bad.map_sha256);
            refused_action("/profiles/map_permissions",fault==0 ? "matching-policy-protected-owner-overlap":"different-prepared-height-policy-fingerprint",[&]{(void)save::restore_original_production_development(bad,context.data,context.mask,hostile);});
        }
        check(f::read_text(target)==good.bytes,"Provenance/geometry refusal changed protected bytes");
    }
    void finish(const char* label) {
        for(const auto& good:corpus)check(good.protected_world->snapshot()==good.document.world &&
            f::read_text(output/(good.label+".good.save.json"))==good.bytes,"Final protected live World/save bytes changed");
        f::write_text(output/"report.json",Json{{"status","PASS"},{"scope",label},{"original_model",context.original_model},
            {"original_map",context.original_map},{"snapshot_injection",false},{"positive_checkpoints",positives},
            {"cases",cases},{"case_count",cases.size()},{"parse_refusals",parsed_refusals},{"restore_refusals",restore_refusals},
            {"human_acceptance","NOT_RUN"},{"native_actions",false}}.dump(2)+"\n");
        std::cout<<label<<" PASS: "<<positives.size()<<" earned checkpoints, "<<cases.size()<<" exact adversarial cases (parse "<<parsed_refusals<<", restore "<<restore_refusals<<")\n";
    }
};
void elite_geometry(const fs::path& output,bool persistent) {
    auto context=q::authored_context();auto world=context.world();const auto scene=oe_finance_fixture::build(world,true);
    q::through(world,120);check(scene.elite && world.building(*scene.elite).kind==sim::Object::OriginalEliteHouse,"No paid timed4x4 Elite materialization");
    Suite suite(context,output,persistent);suite.add({"paid-elite4x4",context.document(world)});const auto& doc=suite.corpus.front();
    const auto& buildings=doc.raw["world"]["buildings"];const auto found=std::find_if(buildings.begin(),buildings.end(),[&](const auto& b){return b["id"].template get<unsigned>()==static_cast<unsigned>(*scene.elite);});
    check(found!=buildings.end(),"Missing actual4x4 save owner");const auto index=std::to_string(found-buildings.begin());
    suite.set(doc,"/world/buildings/"+index+"/cell",Json::array({22,18}),"elite4x4-outside-grid");
    suite.set(doc,"/world/buildings/"+index+"/cell",buildings.front()["cell"],"elite4x4-owner-overlap");
    suite.set(doc,"/world/roads/0",(*found)["cell"],"road-through-owned-elite4x4");suite.finish("Authored paid Elite4x4 hostile geometry");
}
}
int main(int argc,char** argv) {
    try {
        fs::path original,output;
        if(argc==3 && std::string_view(argv[1])=="--export-frozen-corpus") {oe_frozen_save_corpus::export_corpus(argv[2]);return 0;}
        for(int i=1;i<argc;++i) {
            const std::string_view arg=argv[i];check((arg=="--real" || arg=="--output") && i+1<argc,"Usage: original-save-adversarial [--real DATA] [--output NEW_DIR] | --export-frozen-corpus NEW_DIR");
            if(arg=="--real")original=fs::canonical(argv[++i]);else output=fs::absolute(argv[++i]);
        }
        f::Root temporary;const bool persistent=!output.empty();if(output.empty())output=temporary.path/"adversarial";
        auto context=original.empty() ? q::authored_context():q::Context::real(original);
        Suite suite(context,output,persistent);const auto corpus=q::build_corpus(context,false);
        for(const auto& point:corpus)suite.add(point);
        suite.fields_and_types();suite.semantic_catalog();suite.json_limits();suite.provenance_and_atomicity();
        elite_geometry(output/"elite4x4",persistent);suite.finish(original.empty() ? "Authored paid active save corpus":"Real Xia/Normal paid active save corpus");
        return 0;
    }catch(const std::exception& error){std::cerr<<"Original save adversarial FAIL: "<<error.what()<<'\n';return 1;}
}
