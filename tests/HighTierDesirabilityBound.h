#pragma once

// QA-only dual certificate for the current bounded Rule12 source adapter.
// This proves an upper bound, never a reachable layout or a paid House state.
#include "simulation/World.h"
#include "simulation/original/Desirability.h"
#include "simulation/original/OriginalRulesModel.h"

#include <nlohmann/json.hpp>
#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace oe_high_tier_des_bound {
namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
using Json=nlohmann::json;

struct SourceCharge {std::size_t cap;int charge;};
inline std::optional<SourceCharge> source_charge(sim::Object kind) {
    // These are the existing authority's source limits, not an admission list.
    // The tests independently purchase each cap and check the next refusal.
    const auto service_types=static_cast<std::size_t>(sim::OriginalServiceProviderType::Drama213)+1;
    const auto per_service=sim::original_service_provider_limit/service_types;
    switch(kind) {
    case sim::Object::OriginalInfluenceSource:return SourceCharge{sim::original_influence_source_limit,1};
    case sim::Object::OriginalWaterWell72:return SourceCharge{sim::original_water_provider_limit,1};
    case sim::Object::HealthPost:
    case sim::Object::ServicePost:return SourceCharge{per_service,0};
    case sim::Object::OriginalMusicSchool211:
    case sim::Object::OriginalAcrobatSchool212:
    case sim::Object::OriginalDramaSchool213:return SourceCharge{per_service,1};
    case sim::Object::OriginalAncestorShrine214:
    case sim::Object::OriginalDaoistShrine215:
    case sim::Object::OriginalBuddhistShrine217:return SourceCharge{per_service,5};
    case sim::Object::OriginalConfucianAcademy219:return SourceCharge{per_service,3};
    default:return std::nullopt;
    }
}
inline bool inside(int x,int y,orig::Footprint body) {
    return x>=body.x&&x<body.x+body.side&&y>=body.y&&y<body.y+body.side;
}
inline bool overlaps(orig::Footprint a,orig::Footprint b) {
    return a.x<b.x+b.side&&b.x<a.x+a.side&&a.y<b.y+b.side&&b.y<a.y+a.side;
}
inline int cell_charge(int x,int y,orig::Footprint house) {
    if(inside(x,y,house)||x%2||y%2||x< -4||x>4||y< -4||y>4)return 0;
    return x>=-2&&x<=2&&y>=-2&&y<=2?3:1;
}
inline orig::SpreadSpec spread(const orig::OriginalRulesModel& model,orig::ModelBuilding kind) {
    return {model.building_value(kind,orig::BuildingField::Desirability),
        model.building_value(kind,orig::BuildingField::DesirabilityStep),
        model.building_value(kind,orig::BuildingField::DesirabilityStepSize),
        model.building_value(kind,orig::BuildingField::DesirabilityRange)};
}
struct Source {
    sim::CommandType command;sim::Object kind;orig::ModelBuilding model;
    int side;SourceCharge dual;orig::SpreadSpec spread;
};
inline Json report(const sim::World& world) {
    Json result{{"format","openemperor-high-tier-des-bound-v1"},{"verified",false},{"blocks_entry",false},
        {"actual_house_or_goal_witness",false},{"snapshot_injection",false},
        {"scope","Current Rule12 admitted sources and Normal model only; all caps freely relocatable, no terrain/roads/existing-city/funds/staff restrictions."},
        {"proof","Each source contribution is bounded by its source charge plus covered nonnegative lattice charges. Complete non-overlapping footprints can spend each lattice cell once. World uses zero bonus and the maximum of its16 House-cell values."}};
    if(!world.original_sustainable_economy_enabled()||!world.original_rules_model()||
        world.original_rules_model()->difficulty()!=orig::Difficulty::Normal) {
        result["reason"]="Certificate requires explicit Rule12 and a prepared Normal model";return result;
    }
    const auto& model=*world.original_rules_model();result["model_sha256"]=model.provenance().sha256;
    const auto elite=sim::building_footprint(world.profile(),world.rule_version(),sim::Object::OriginalEliteHouse);
    if(elite.width!=4||elite.height!=4){result["reason"]="The shared Elite footprint no longer matches the certificate";return result;}
    std::vector<Source> sources;Json identities=Json::array();int cap_charge=0;
    // Enumerate every current command through the present final enum member.
    // Added commands/source contracts require a fresh completeness audit.
    // Changed mappings/footprints inside this current domain fail closed.
    for(unsigned value=0;value<=static_cast<unsigned>(sim::CommandType::PlaceOriginalElitePlot);++value) {
        const auto command=static_cast<sim::CommandType>(value);const auto kind=sim::placed_object(command);
        if(!kind)continue;const auto mapped=sim::original_influence_model(*kind);if(!mapped)continue;
        const auto dual=source_charge(*kind);const auto geometry=sim::building_footprint(world.profile(),world.rule_version(),*kind);
        if(!dual||geometry.width!=geometry.height||(geometry.width!=2&&geometry.width!=3)) {
            result["reason"]="Shared DES source/footprint is outside the checked certificate";return result;
        }
        sources.push_back({command,*kind,*mapped,geometry.width,*dual,spread(model,*mapped)});
        cap_charge+=static_cast<int>(dual->cap)*dual->charge;
        identities.push_back({{"command",value},{"object",static_cast<unsigned>(*kind)},
            {"model_id",orig::model_id(*mapped).value},{"model_label",orig::model_label(*mapped)},
            {"side",geometry.width},{"cap",dual->cap},{"source_charge",dual->charge}});
    }
    result["source_identities"]=identities;result["source_cap_charge"]=cap_charge;
    if(sources.size()!=11||sim::original_service_provider_limit!=18) {
        result["reason"]="Source identity/cap contract changed; recertification required";return result;
    }
    constexpr int center=16,map_side=40;
    constexpr int reach=orig::desirability_max_range; // Complete clamped support, including zero tails.
    const orig::DesirabilityMap zero(map_side,map_side);Json cases=Json::array();int maximum=0;
    std::uint64_t checked=0,positive=0;
    for(int cell_y=0;cell_y<4;++cell_y)for(int cell_x=0;cell_x<4;++cell_x) {
        const orig::Footprint house{-cell_x,-cell_y,4};int lattice_charge=0;Json lattice=Json::array();
        for(int y=-4;y<=4;++y)for(int x=-4;x<=4;++x) {
            const int price=cell_charge(x,y,house);lattice_charge+=price;
            if(price)lattice.push_back({{"cell",{x,y}},{"charge",price}});
        }
        std::uint64_t tested=0;int minimum_slack=INT32_MAX;
        for(const auto& source:sources) {
            // A positive source has distance<=10 from the selected cell.
            // For side2/3 its whole square therefore lies within[-12,12].
            // Larger translations cannot contribute and require no placement.
            for(int y=-reach-source.side+1;y<=reach;++y)
                for(int x=-reach-source.side+1;x<=reach;++x) {
                    const orig::Footprint footprint{x,y,source.side};if(overlaps(house,footprint))continue;
                    auto map=zero;map.apply_source({{center+x,center+y,source.side},source.spread},orig::WallBlocking::None);
                    const int contribution=map.at(center,center);++checked;
                    if(contribution<=0)continue; // Removing negative additions is a conservative relaxation.
                    ++positive;++tested;int charged=source.dual.charge;
                    for(int dy=0;dy<source.side;++dy)for(int dx=0;dx<source.side;++dx)
                        charged+=cell_charge(x+dx,y+dy,house);
                    if(charged<contribution) {
                        result["reason"]="Prepared model violates an exact source/cell dual inequality";
                        result["counterexample"]={{"command",static_cast<unsigned>(source.command)},
                            {"house_origin",{house.x,house.y}},{"source_origin",{x,y}},
                            {"contribution",contribution},{"charge",charged}};return result;
                    }
                    minimum_slack=std::min(minimum_slack,charged-contribution);
                }
        }
        const int bound=cap_charge+lattice_charge;maximum=std::max(maximum,bound);
        cases.push_back({{"house_cell",{cell_x,cell_y}},{"translated_house_origin",{house.x,house.y}},
            {"cell_charge",lattice_charge},{"source_cap_charge",cap_charge},{"upper_bound",bound},
            {"positive_placements_checked",tested},{"minimum_integer_slack",minimum_slack},{"lattice",lattice}});
    }
    const auto threshold=model.house_rules().value(orig::house_row(orig::HouseType::ImpressiveCompound),orig::HouseField::DesirabilityHigh);
    result["verified"]=true;result["upper_bound"]=maximum;result["entry_threshold"]=threshold;
    result["blocks_entry"]=maximum<threshold;result["house_side"]=4;
    result["all16cells"]=cases;result["placements_checked"]=checked;result["positive_placements_checked"]=positive;
    result["bonus"]=0;result["non_overlapping_complete_footprints"]=true;
    return result;
}
} // namespace oe_high_tier_des_bound
