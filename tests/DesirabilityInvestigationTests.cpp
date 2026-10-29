// Pure/source-query falsifiers only. No test state is a paid-city witness.
#include "DesirabilityInvestigation.h"
#include "OriginalProductionFixture.h"
#include <iostream>

namespace q=oe_desirability_investigation;
namespace f=oe_influence_fixture;
using q::check;
template<class F>void refuses(F action,const char* why) {bool refused=false;try{action();}catch(const std::exception&){refused=true;}check(refused,why);}

q::Json simple_packing() {
    return {{"house",{20,20}},{"bonus",10},{"water_cell",{24,23}},
        {"sources",q::Json::array({{{"object",static_cast<unsigned>(q::sim::Object::OriginalInfluenceSource)},
            {"cell",{18,18}},{"side",2},{"model_id",72}}})}};
}
void metadata_falsifiers(const q::sim::World& world) {
    const auto before=world.snapshot();const auto input=simple_packing();
    const auto good=q::packing_report(world,input);
    check(good.at("source_caps_checked")==true&&good.at("paid_city_witness")==false&&good.at("world_mutated")==false,
        "Pure metadata was claimed as gameplay authority");
    auto bad=input;bad["sources"][0]["object"]=static_cast<unsigned>(q::sim::Object::Road);
    refuses([&]{(void)q::packing_report(world,bad);},"Unadmitted source object accepted");
    bad=input;bad["sources"][0]["object"]=std::uint64_t{4294967309ULL};
    refuses([&]{(void)q::packing_report(world,bad);},"Large source number wrapped into a valid object");
    bad=input;bad["sources"][0]["model_id"]=73;
    refuses([&]{(void)q::packing_report(world,bad);},"Wrong source model identity accepted");
    bad=input;bad["sources"][0]["model_id"]=std::uint64_t{4294967368ULL};
    refuses([&]{(void)q::packing_report(world,bad);},"Large model number wrapped into72");
    bad=input;bad["sources"][0]["side"]=3;
    refuses([&]{(void)q::packing_report(world,bad);},"Forged source footprint accepted");
    bad=input;bad["sources"][0]["side"]=std::uint64_t{4294967298ULL};
    refuses([&]{(void)q::packing_report(world,bad);},"Large footprint wrapped into side2");
    bad=input;bad["sources"][0]["cell"]={INT32_MAX,0};
    refuses([&]{(void)q::packing_report(world,bad);},"Unbounded source coordinates accepted");
    bad=input;bad["sources"].push_back(bad["sources"][0]);
    refuses([&]{(void)q::packing_report(world,bad);},"Overlapping complete source bodies accepted");
    bad=input;bad["sources"]=q::Json::array();for(unsigned i=0;i<5;++i)bad["sources"].push_back({{"object",13},{"cell",{2+3*i,2}},{"side",2},{"model_id",72}});
    refuses([&]{(void)q::packing_report(world,bad);},"A fifth neutral source bypassed the actual cap");
    bad=input;bad["sources"][0]["cell"]={20,20};
    refuses([&]{(void)q::packing_report(world,bad);},"A source overlapped the complete4x4 House");
    bad=input;bad["water_cell"]={24,24};
    refuses([&]{(void)q::packing_report(world,bad);},"Diagonal water was treated as an exact cardinal edge");
    bad=input;bad["water_cell"]={21,21};
    refuses([&]{(void)q::packing_report(world,bad);},"Interior water was treated as an outside edge");
    bad=input;bad["sources"][0]["cell"]={24,23};
    refuses([&]{(void)q::packing_report(world,bad);},"Water was silently occupied by a source body");
    bad=input;bad["bonus"]=1;
    refuses([&]{(void)q::packing_report(world,bad);},"An arbitrary+1 bonus was admitted");
    bad=input;bad["bonus"]=true;
    refuses([&]{(void)q::packing_report(world,bad);},"A boolean became numerical authority");
    bad=input;bad["world_grant"]=true;
    refuses([&]{(void)q::packing_report(world,bad);},"Unknown packing authority field ignored");
    check(world.snapshot()==before,"Rejected or successful pure report changed World state");
}
void core_falsifiers() {
    const auto report=q::pure_controls();check(report.at("classification")=="SYNTHETIC_QUERY_CONTROL"&&
        report.at("conditional_upper_bound_status")=="BOUND_ONLY_NOT_REACHABILITY"&&report.at("existing_source_packing_witness")==false,
        "Upper-bound arithmetic was mislabeled as a constructive packing");
    q::orig::DesirabilityMap a(32,32),b(32,32);
    const q::orig::DesirabilitySource positive{{7,7,1},{200,1,0,3}},negative{{7,8,1},{-60,1,0,3}};
    a.apply_source(positive);a.apply_source(negative);b.apply_source(negative);b.apply_source(positive);
    check(a.at(8,8)==40&&b.at(8,8)==100,"Source-order per-add saturation was replaced by a final summed clamp");
    refuses([&]{(void)a.building_value({31,31,4},10);},"Partial off-map House query was accepted");
    refuses([&]{(void)a.building_value({7,7,7},10);},"Unbounded source geometry query was accepted");
}
void synthetic() {
    f::Root root;oe_production_fixture::write_model(root.data);
    const auto parsed=q::orig::load_model_data(root.data);check(bool(parsed),"Shared synthetic model reader failed");
    const auto model=q::orig::OriginalRulesModel::prepare_production(*parsed.model,q::orig::Difficulty::Normal);
    auto world=q::sim::World::create_original_market_logistics(q::authored_plane(),model);
    metadata_falsifiers(world);core_falsifiers();
    const auto before=world.snapshot();const auto registry=q::model_register(*parsed.model,world);
    check(registry.size()==parsed.model->building_count()&&world.snapshot()==before&&world.buildings().empty()&&world.total_population()==0,
        "Shared model registry missed rows or created a gameplay object");
    refuses([&]{(void)q::certificate(world);},"Pinned real-model certificate accepted an authored replacement");
    const auto text=oe_production_fixture::authored_model_text();
    auto overflow=text;const std::string marker="72,Well,{,25,";const auto pos=overflow.find(marker);check(pos!=std::string::npos,"Authored model fixture changed");
    overflow.replace(pos,marker.size(),"72,Well,{,2147483647,");const auto huge=q::orig::parse_model_data(overflow);check(bool(huge),"Raw parser incorrectly applied arithmetic");
    refuses([&]{(void)q::normal_value(*huge.model,{72},q::orig::BuildingField::Cost);},"Generic Normal transform hid original int32 intermediate overflow");
}
void real(const std::filesystem::path& data) {
    const auto parsed=q::orig::load_model_data(data);check(bool(parsed)&&parsed.model->provenance().sha256==q::model_sha,"Exact supplied original model required");
    const auto model=q::orig::OriginalRulesModel::prepare_production(*parsed.model,q::orig::Difficulty::Normal);
    auto old=q::sim::World::create_original_sustainable_economy(q::authored_plane(),model);
    auto current=q::sim::World::create_original_market_logistics(q::authored_plane(),model);
    const auto a=q::certificate(old),b=q::certificate(current);
    check(a.at("actual_rule")==12&&b.at("actual_rule")==13&&a.at("historical_checker_raw_report").at("source_identities")==b.at("historical_checker_raw_report").at("source_identities"),
        "Historical certificate was silently relabeled instead of separately checked");
    const auto rows=q::model_register(*parsed.model,current);check(rows.size()==parsed.model->building_count(),"Complete real model inventory lost a row");
    const auto catalogue=q::world_catalog(current);unsigned admitted=0;
    for(const auto& item:catalogue) {
        admitted+=item.at("des_admitted").get<bool>()?1U:0U;
        const auto id=static_cast<q::orig::ModelBuilding>(item.at("model_id").get<unsigned>());
        for(const auto field:{q::orig::BuildingField::Desirability,q::orig::BuildingField::Cost,q::orig::BuildingField::Employees})
            check(q::normal_value(*parsed.model,q::orig::model_id(id),field)==q::orig::building_value(*parsed.model,id,field,q::orig::Difficulty::Normal),
                "Generic shared-reader arithmetic differs from the existing typed schema");
    }
    check(admitted==11,"Current source catalogue changed and needs recertification");metadata_falsifiers(current);
}
int main(int argc,char** argv) {
    try {
        check(argc==1||(argc==3&&std::string_view(argv[1])=="--real"),"Use no arguments or --real DATA");
        synthetic();if(argc==3) {
            const std::filesystem::path data=argv[2];if(!std::filesystem::is_regular_file(data/"Model/EmperorBuildingModels.txt")){std::cout<<"BLOCKED: original model unavailable\n";return 77;}
            real(data);
        }
        std::cout<<"PASS: read-only full registry, exact Rule12/Rule13 bound, source/shape/cap/pin falsifiers; no paid Elite claim\n";return 0;
    }catch(const std::exception& e){std::cerr<<"OE40 QA FAIL: "<<e.what()<<'\n';return 1;}
}
