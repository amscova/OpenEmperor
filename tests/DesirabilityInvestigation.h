#pragma once

// Read-only OE40 research queries. Numeric source rows are written privately by
// the diagnostic CLI; neither this header nor any report grants World authority.
#include "HighTierDesirabilityBound.h"
#include "simulation/original/OriginalModelMapping.h"

#include <fstream>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <openssl/evp.h>

namespace oe_desirability_investigation {
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
namespace bound=oe_high_tier_des_bound;
namespace fs=std::filesystem;
using Json=nlohmann::json;
inline constexpr const char* model_sha="7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6";
inline constexpr const char* exe_sha="6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e";
inline void check(bool value,const std::string& message){if(!value)throw std::runtime_error(message);}
inline int bounded_integer(const Json& value,int minimum,int maximum) {
    check(value.is_number_integer(),"Research integer must not be floating point or boolean");
    if(value.is_number_unsigned()) {
        const auto n=value.get<std::uint64_t>();check(n<=static_cast<std::uint64_t>(maximum),"Research integer outside bounded domain");
        check(static_cast<std::int64_t>(n)>=minimum,"Research integer below bounded domain");return static_cast<int>(n);
    }
    const auto n=value.get<std::int64_t>();check(n>=minimum&&n<=maximum,"Research integer outside bounded domain");return static_cast<int>(n);
}

inline std::int32_t normal_value(const orig::ModelData& model,orig::ModelBuildingId id,orig::BuildingField field) {
    const auto product=std::int64_t(model.building(id,field))*model.building_modifier(orig::Difficulty::Normal,field);
    if(product<INT32_MIN||product>INT32_MAX)throw std::overflow_error("generic model transformation exceeds original int32 intermediate");
    return static_cast<std::int32_t>(product/100);
}
inline std::string file_hash(const fs::path& path,std::uintmax_t budget=64U*1024U*1024U) {
    check(fs::is_regular_file(path)&&!fs::is_symlink(fs::symlink_status(path)),"Source hash requires a regular non-symlink file");
    check(fs::file_size(path)<=budget,"Research source exceeds bounded hash input");
    const auto size=fs::file_size(path);std::ifstream in(path,std::ios::binary);check(in.good(),"Cannot read research input");
    std::unique_ptr<EVP_MD_CTX,decltype(&EVP_MD_CTX_free)> hash(EVP_MD_CTX_new(),EVP_MD_CTX_free);
    check(hash&&EVP_DigestInit_ex(hash.get(),EVP_sha256(),nullptr)==1,"Cannot initialize source fingerprint");
    std::array<char,16384> buffer{};std::uintmax_t read=0;
    while(in.read(buffer.data(),static_cast<std::streamsize>(buffer.size()))||in.gcount()) {
        const auto bytes=in.gcount();check(bytes>=0,"Invalid hash read length");read+=static_cast<std::uintmax_t>(bytes);
        check(read<=budget&&EVP_DigestUpdate(hash.get(),buffer.data(),static_cast<std::size_t>(bytes))==1,"Source hash update failed");
    }
    check(in.eof()&&read==size,"Research input changed length while hashing");
    std::array<unsigned char,32> digest{};unsigned length=0;
    check(EVP_DigestFinal_ex(hash.get(),digest.data(),&length)==1&&length==digest.size(),"Source hash finalization failed");
    std::ostringstream out;out<<std::hex<<std::setfill('0');for(const auto byte:digest)out<<std::setw(2)<<unsigned(byte);return out.str();
}
inline std::shared_ptr<const sim::MapPermissions> authored_plane() {
    return std::make_shared<const sim::MapPermissions>(100,100,3,
        std::vector<sim::MapCellPermission>(10000,{true,true,false,0,sim::BuildBlocker::None,sim::BuildBlocker::None}),
        std::vector<sim::FixedGatePassage>{});
}
inline std::string world_name(sim::Object kind) {
    static constexpr std::array names{"Empty","Road","Workshop","Warehouse","ClaySource","Pottery","Household","Farm","ServicePost","Market",
        "FireWatch","Well","HealthPost","OriginalInfluenceSource","OriginalHempFarm","OriginalHempField","OriginalFoodFarm","OriginalWheatField","OriginalMilletField",
        "OriginalAncestorShrine214","OriginalWaterWell72","OriginalDaoistShrine215","OriginalBuddhistShrine217","OriginalConfucianAcademy219","OriginalMusicSchool211",
        "OriginalAcrobatSchool212","OriginalDramaSchool213","OriginalLoggingShed38","OriginalBronzeSmelter39","OriginalBronzewareMaker42","OriginalLacquerwareMaker44",
        "OriginalPaperMaker45","OriginalWeaver47","OriginalTeaCuringShed237","OriginalLacquerRefinery238","OriginalSilkwormShed239","OriginalTeaBush26","OriginalLacquerTree27",
        "OriginalMulberryTree28","OriginalElitePlot","OriginalEliteHouse"};
    static_assert(names.size()==static_cast<unsigned>(sim::Object::OriginalEliteHouse)+1);
    return names.at(static_cast<unsigned>(kind));
}
inline std::optional<unsigned> static_cap(sim::Object kind,const sim::World& w) {
    if(const auto source=bound::source_charge(kind))return static_cast<unsigned>(source->cap);
    switch(kind) {
    case sim::Object::ClaySource:return static_cast<unsigned>(sim::Rules::city_v10_clay_source_limit);
    case sim::Object::Pottery:return static_cast<unsigned>(sim::Rules::city_v10_pottery_limit);
    case sim::Object::Warehouse:return static_cast<unsigned>(sim::Rules::city_v10_warehouse_limit);
    case sim::Object::Market:return static_cast<unsigned>(sim::Rules::city_v11_market_limit);
    case sim::Object::OriginalFoodFarm:return static_cast<unsigned>(sim::original_food_farm_limit(w.profile(),w.rule_version()));
    case sim::Object::OriginalHempFarm:return 2;
    default:return std::nullopt;
    }
}
inline Json worker_formula(const sim::World& w,sim::Object kind,std::optional<orig::ModelBuilding> model) {
    switch(kind) {
    case sim::Object::OriginalInfluenceSource:return {{"value",0},{"evidence","Authored neutral influence source, no workers"}};
    case sim::Object::ClaySource:return {{"value",sim::Rules::clay_source_workers},{"evidence","World::workforce_required fixed Rules constant"}};
    case sim::Object::Pottery:return {{"value",sim::Rules::pottery_workers},{"evidence","World::workforce_required fixed Rules constant"}};
    case sim::Object::Warehouse:return {{"value",sim::Rules::warehouse_workers},{"evidence","World::workforce_required fixed Rules constant"}};
    case sim::Object::Market:return {{"value",sim::Rules::market_workers},{"evidence","World::workforce_required fixed Rules constant"}};
    case sim::Object::Farm:return {{"value",sim::Rules::farm_workers},{"evidence","World::workforce_required fixed authored Farm constant; not an original Farmhouse worker grant"}};
    default:if((sim::original_industry_source(kind)||sim::original_influence_model(kind)||kind==sim::Object::OriginalFoodFarm||kind==sim::Object::OriginalHempFarm)&&
               model&&w.original_rules_model()->supports_building(*model))return {{"value",w.original_rules_model()->building_value(*model,orig::BuildingField::Employees)},
        {"evidence","Existing World::workforce_required prepared-model formula only; no worker assigned without actual paid body/residents"}};
    }
    return {{"value",0},{"evidence","World::workforce_required default has no installed workers for this object; original identity remains unknown"}};
}
inline Json world_catalog(const sim::World& world) {
    const auto before=world.snapshot();Json result=Json::array();
    for(unsigned n=0;n<=static_cast<unsigned>(sim::CommandType::PlaceOriginalElitePlot);++n) {
        const auto command=static_cast<sim::CommandType>(n);const auto kind=sim::placed_object(command);if(!kind)continue;
        const auto active=sim::original_influence_model(*kind);const auto candidate=orig::functional_model_candidate(*kind);
        const auto mapping=active?active:candidate.candidate;if(!mapping)continue;
        const auto footprint=sim::building_footprint(world.profile(),world.rule_version(),*kind);
        const auto validation=world.validate({command,{20,20}});const auto cap=static_cap(*kind,world);
        result.push_back({{"command",n},{"world_object",static_cast<unsigned>(*kind)},{"world_enum",world_name(*kind)},
            {"model_id",orig::model_id(*mapping).value},{"mapping",active?"CURRENT_AUTHORED_DES_BINDING":"FUNCTIONAL_CANDIDATE_ONLY"},
            {"des_admitted",active.has_value()},{"square_core",active.has_value()},
            {"physical_footprint",{footprint.width,footprint.height}},{"quoted_world_cost",world.construction_cost(command)},
            {"worker_formula",worker_formula(world,*kind,mapping)},{"per_kind_cap",cap?Json(*cap):Json(nullptr)},
            {"command_on_empty_authored_plane",{{"accepted",validation.accepted},{"changed",validation.changed},{"reason",validation.reason}}},
            {"world_query_unchanged",true},{"original_runtime_identity",active?"AUTHORED_BINDING_NOT_ORIGINAL_OBJECT_IDENTITY":"UNVERIFIED_FUNCTIONAL_ANALOGY"},
            {"placement_scope","Read-only World::validate on an authored all-permitted plane; not a paid Xia placement witness."},
            {"source_lifecycle",active?"Placed paid owner; source cache rebuilt on successful construction/removal/restore; only neutral Well72 removal permitted.":"No DES source admission in this generation."},
            {"des_dependencies",active?"Existing placed owner binding; static DES is not granted by worker allocation, visits, road proximity or rendered art.":"No emitted DES through current World cache."},
            {"transport_service_dependencies","Operation/staff/fire/shared roads and actual arrivals govern production/services separately; this query creates none."},
            {"historical_save_rights",validation.accepted?"Existing paid object only under exact generation and complete geometry/accounting/transfer validation; no original archive import.":"UNKNOWN_OR_REFUSED for this command/context; no permission inferred."}});
    }
    check(world.snapshot()==before,"World catalogue query changed gameplay authority");return result;
}
inline Json factory(unsigned id) {
    const char* path=nullptr;const char* shape=nullptr;
    switch(id) {
    case 35:path="0x51c660 -> 0x5f0e20 -> 0x5ac260 -> 0x5ac3a0 / vtable0x7bca9c";shape="Square default side2; full later source lifecycle unknown";break;
    case 43:path="0x51c660 -> 0x5f0e20 / predicate0x5f50f0 -> 0x5f50c0 / vtable0x7bfc2c";shape="Square default side2; full later source lifecycle unknown";break;
    case 54:path="0x51c660 -> 0x5d3580 -> 0x5d61e0 / vtable0x7be1bc";shape="Square callback0x5d3720 -> 0x5d3f50 -> 0x4fa3a0 returns3; current World Warehouse is2x2";break;
    case 59:path="0x51c660 -> 0x5d3580 / predicate0x543d90 -> 0x543450 / vtable0x7b6f3c";shape="Special rectangle4x7 or7x4 from descriptor selector; not physical occupancy; lifecycle/selector admission unknown";break;
    case 72:path="0x51c660 -> 0x51bef0 selector0 -> 0x51c090 / vtable0x7b5eb4";shape="Square default side2; source link/state identity not supplied by rendered Well";break;
    case 193:path="0x51c660 -> 0x4c2960 -> 0x4c5b80 / vtable0x7af1e8";shape="Square default side3; legacy Farm1x1 is a distinct authored object";break;
    case 207:path="0x51c660 -> 0x51bef0 selector1 -> 0x51c0b0 / vtable0x7b6114";shape="Square default side2; original full service identity unknown";break;
    default:break;
    }
    if(!path)return {{"status","MODEL_ROW_ONLY_FACTORY_NOT_VERIFIED"},{"shape",nullptr},{"source_manager_admission","UNKNOWN; model label/value alone grants nothing"},
        {"runtime_status_links","UNKNOWN for this candidate; no invented lifecycle"}};
    return {{"status","DOCUMENTED_PINNED_EXE_FACTORY_NOT_FULL_GAMEPLAY_ADMISSION"},{"path",path},{"shape",shape},
        {"reference","docs/reverse/original-desirability-source-factories.md"},
        {"source_manager_admission","Observed global gate status1or3 AND separate runtime+0x3c==0; complete candidate placement/destruction/lifecycle remains unverified"},
        {"runtime_status_links","Initializer status3; +0x3c is separate archive byte54, not Subindex18; manager replacement can restore previous links"}};
}
inline Json model_register(const orig::ModelData& data,const sim::World& world) {
    const auto catalogue=world_catalog(world);Json rows=Json::array();
    for(unsigned number=0;number<=orig::max_building_id;++number) {
        const orig::ModelBuildingId id{static_cast<std::uint16_t>(number)};if(!data.has_building(id))continue;
        Json raw,normal;for(const auto& [name,field]:std::array{
            std::pair{"cost",orig::BuildingField::Cost},std::pair{"des",orig::BuildingField::Desirability},
            std::pair{"step",orig::BuildingField::DesirabilityStep},std::pair{"delta",orig::BuildingField::DesirabilityStepSize},
            std::pair{"range",orig::BuildingField::DesirabilityRange},std::pair{"employees",orig::BuildingField::Employees}}) {
            raw[name]=data.building(id,field);normal[name]=normal_value(data,id,field);
        }
        Json candidates=Json::array();bool admitted=false;
        for(const auto& item:catalogue)if(item.at("model_id")==number){candidates.push_back(item);admitted|=item.at("des_admitted").get<bool>();}
        const int raw_des=raw.at("des").get<int>(),normal_des=normal.at("des").get<int>();
        const auto initial_sign=[](int value){return value>0?"POSITIVE":value<0?"NEGATIVE":"ZERO";};
        rows.push_back({{"model_id",number},{"raw_model_label",std::string(data.building_label(id))},{"raw",raw},{"normal",normal},
            {"initial_raw_sign",initial_sign(raw_des)},{"initial_normal_sign",initial_sign(normal_des)},
            {"factory",factory(number)},{"world_candidates",candidates},
            {"current_des_admission",admitted?"EXISTING_AUTHORED_WORLD_BINDING":"NOT_ADMITTED_AS_WORLD_DES_SOURCE"},
            {"pure_eligibility","Initial DES signs classify model fields only, not aggregate source contribution or source, geometry, status/link, placement, budget or save admission. A rectangle with nonzero delta may contribute despite initial DES zero and may cross signs; unknown shapes remain unverified."}});
    }
    check(rows.size()==data.building_count(),"Model registry missed a row outside the shared bounded ID domain");return rows;
}
inline Json certificate(const sim::World& w) {
    check(w.rule_version()==12||w.rule_version()==13,"Certificate scope requires exact Rule12 or Rule13");
    check(w.original_rules_model()->provenance().sha256==model_sha,"Certificate refused a non-pinned original model");
    const auto before=w.snapshot();const auto proof=bound::report(w);check(w.snapshot()==before,"Certificate query changed World authority");
    check(proof.at("verified")==true&&proof.at("blocks_entry")==true&&proof.at("upper_bound")==79&&
        proof.at("entry_threshold")==80&&proof.at("source_cap_charge")==48&&proof.at("all16cells").size()==16,
        "Current original-model/source-geometry certificate no longer verifies79<80");
    return {{"actual_rule",w.rule_version()},{"actual_schema",w.rule_version()==12?28:29},{"world_snapshot_unchanged",true},
        {"source_order","Shared World binding enumeration; the bound removes negative additions conservatively only within the current admitted sign-preserving square-core source contract, excluding unadmitted rectangle shapes."},
        {"scope",w.rule_version()==12?"Exact historical Rule12 recertification":"Separately recomputed actual Rule13; its inherited source identities/geometry must equal Rule12"},
        {"historical_checker_raw_report",proof}};
}
inline Json pure_controls() {
    orig::DesirabilityMap map(32,32);map.apply_source({{7,7,1},{79,1,0,1}});
    const orig::Footprint house{8,7,4};const auto zero=map.building_value(house),bonus=map.building_value(house,10);
    check(zero==79&&bonus==89,"Shared pure explicit-bonus query changed");
    return {{"classification","SYNTHETIC_QUERY_CONTROL"},{"cell_maximum",zero},{"explicit_bonus",10},{"query_value",bonus},
        {"existing_source_packing_witness",false},{"actual_house_or_goal_witness",false},{"world_bonus_granted",false},
        {"conditional_new_upper_bound",89},{"conditional_upper_bound_status","BOUND_ONLY_NOT_REACHABILITY"},
        {"negative_source_hypothesis_scope","CURRENT_ADMITTED_SIGN_PRESERVING_SQUARE_CORE_ONLY"},
        {"negative_source_hypothesis","Within that bounded current-source contract, removing negative contributions is already a conservative relaxation in79. This does not classify unadmitted rectangle factories, sign-crossing profiles or unknown shapes."}};
}
inline Json packing_report(const sim::World& world,const Json& input) {
    check(input.is_object()&&input.contains("house")&&input.contains("sources")&&input.contains("bonus"),"Pure packing requires explicit house/sources/bonus");
    for(const auto& item:input.items())check(item.key()=="house"||item.key()=="sources"||item.key()=="bonus"||item.key()=="water_cell","Unknown pure packing field");
    const auto point=[](const Json& j){check(j.is_array()&&j.size()==2,"Packing cell requires two bounded integers");return sim::Cell{bounded_integer(j[0],0,63),bounded_integer(j[1],0,63)};};
    const auto at=point(input.at("house"));const orig::Footprint house{at.x,at.y,4};
    orig::DesirabilityMap map(64,64);(void)map.building_value(house);
    const auto bonus=bounded_integer(input.at("bonus"),0,10);
    check(bonus==0||bonus==10,"No arbitrary bonus is admitted by this pure counterfactual");
    const auto& sources=input.at("sources");check(sources.is_array()&&sources.size()<=24,"Pure packing exceeds admitted source cap total");
    std::array<unsigned,static_cast<unsigned>(sim::Object::OriginalEliteHouse)+1> counts{};
    std::vector<orig::Footprint> occupied;Json provenance=Json::array();
    for(const auto& source:sources) {
        check(source.is_object()&&source.contains("object")&&source.at("object").is_number_integer(),"Packing source lost typed object");
        for(const auto& item:source.items())check(item.key()=="object"||item.key()=="cell"||item.key()=="side"||item.key()=="model_id","Unknown pure source field");
        const auto n=bounded_integer(source.at("object"),0,static_cast<int>(counts.size())-1);const auto kind=static_cast<sim::Object>(n);
        const auto model=sim::original_influence_model(kind);const auto charge=bound::source_charge(kind);check(model&&charge,"Packing source is not admitted by current World");
        check(++counts[static_cast<unsigned>(kind)]<=charge->cap,"Packing source count exceeds actual current source cap");
        const auto geometry=sim::building_footprint(world.profile(),world.rule_version(),kind);
        check(geometry.width==geometry.height,"Packing requires actual square source core geometry");
        if(source.contains("side"))check(bounded_integer(source.at("side"),1,6)==geometry.width,"Forged source geometry");
        if(source.contains("model_id"))check(bounded_integer(source.at("model_id"),0,orig::max_building_id)==orig::model_id(*model).value,"Forged source/model binding");
        const auto cell=point(source.at("cell"));const orig::Footprint body{cell.x,cell.y,geometry.width};
        check(cell.x<=64-body.side&&cell.y<=64-body.side,"Complete packing source outside bounded map");
        check(!bound::overlaps(house,body)&&std::none_of(occupied.begin(),occupied.end(),[&](const auto& other){return bound::overlaps(body,other);}),"Pure packing overlaps complete footprints");
        const auto before=map.building_value(house);map.apply_source({body,bound::spread(*world.original_rules_model(),*model)});occupied.push_back(body);
        provenance.push_back({{"implicit_ordered_id",occupied.size()},{"object",n},{"model_id",orig::model_id(*model).value},{"cell",{cell.x,cell.y}},
            {"side",body.side},{"before",before},{"after",map.building_value(house)}});
    }
    if(bonus) {
        check(input.contains("water_cell"),"Water bonus counterfactual needs explicit unclaimed cardinal cell");const auto water=point(input.at("water_cell"));
        const bool cardinal=((water.x==house.x-1||water.x==house.x+4)&&water.y>=house.y&&water.y<house.y+4)||
            ((water.y==house.y-1||water.y==house.y+4)&&water.x>=house.x&&water.x<house.x+4);
        check(cardinal&&water.x>=0&&water.y>=0&&water.x<64&&water.y<64,"Water counterfactual is not an exact cardinal edge cell");
        check(std::none_of(occupied.begin(),occupied.end(),[&](const auto& body){return bound::inside(water.x,water.y,body);}),"Water cell is occupied by a source body");
    }
    const auto without=map.building_value(house),with=map.building_value(house,bonus);const auto threshold=world.original_rules_model()->house_rules().value(orig::ModelHouse::ImpressiveCompound,orig::HouseField::DesirabilityHigh);
    return {{"classification",with>=threshold?"PURE_POSSIBLE":"PURE_BELOW_THRESHOLD"},{"cell_maximum",without},{"explicit_conditional_bonus",bonus},
        {"query_value",with},{"entry_threshold",threshold},{"source_provenance",provenance},{"nonoverlapping_complete_footprints",true},
        {"source_caps_checked",true},{"source_order","Provided order corresponds to hypothetical monotone Building IDs; shared core saturates after each addition."},
        {"paid_city_witness",false},{"world_mutated",false},{"original_water_flag_or_map_admission_proof",false},
        {"scope","Constructive pure synthetic-geography packing only, conditional on an independently evidenced/admitted water predicate. No paid World, terrain permission, residents or services are inferred."}};
}
}
