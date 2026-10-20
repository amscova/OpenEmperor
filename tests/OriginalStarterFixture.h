#pragma once

// Technical acceptance of the paid starter. Positive integration reads the
// supplied original Model and Xia; authored permissions are separately labeled.
#include "OriginalFinanceFixture.h"
#include "simulation/OriginalEconomyStarter.h"
#include "maps/StoredMapSession.h"
#include "maps/SandboxPlacement.h"
#include "renderer/StoredGraphicsRenderer.h"
#include <SDL3/SDL.h>
#include <nlohmann/json.hpp>
#include <functional>
#include <memory>

namespace oe_starter_fixture {
namespace oe=openemperor;
namespace sim=oe::simulation;
namespace orig=oe::original;
namespace save=oe::persistence;
namespace fs=std::filesystem;
namespace finance=oe_finance_fixture;
namespace f=oe_influence_fixture;
using Json=nlohmann::json;
using f::check;

struct Context {
    fs::path data,map_relative="Cities/Xia.map";
    std::vector<std::uint8_t> mask;
    std::shared_ptr<const sim::MapPermissions> policy;
    std::shared_ptr<const orig::OriginalRulesModel> model;
    std::shared_ptr<f::Root> private_authored;
    bool original_map=false,original_model=false;

    static Context authored() {
        Context result;result.private_authored=std::make_shared<f::Root>();result.data=result.private_authored->data;
        result.map_relative=f::map_relative;f::write_text(result.data/"Model/EmperorBuildingModels.txt",oe_production_fixture::authored_model_text(7));
        result.model=save::prepare_original_production_rules_model(result.data,orig::Difficulty::Normal);
        result.policy=f::permissions();result.mask=result.policy->building_mask();return result;
    }
    static Context real(const fs::path& installation) {
        Context result;result.data=fs::canonical(installation);result.original_map=result.original_model=true;
        result.model=save::prepare_original_production_rules_model(result.data,orig::Difficulty::Normal);
        check(SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy") && SDL_SetHint(SDL_HINT_RENDER_DRIVER,"software") && SDL_Init(SDL_INIT_VIDEO),
              "Starter real-map software preparation failed");
        SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;
        try {
            check(SDL_CreateWindowAndRenderer("Original starter map preparation",64,64,SDL_WINDOW_HIDDEN,&window,&renderer),
                  "Starter real-map software renderer failed");
            auto session=oe::maps::load_stored_map_session(result.data,result.map_relative,
                oe::maps::FootprintPolicy::EdgeByte4x4Preview,oe::maps::StoredGraphicsProfile::Slot8);
            const oe::maps::MapGeometry geometry(session.map.declared_map_size);
            oe::StoredGraphicsRenderer background(std::move(session.plan));background.initialize(renderer);
            result.mask=oe::maps::make_sandbox_buildable_mask(background.plan(),geometry);
            result.policy=oe::maps::load_sandbox_map_permissions(session.map,background.plan(),geometry,result.mask,3);
            background.shutdown();SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();
        } catch (...) {
            if(renderer)SDL_DestroyRenderer(renderer);
            if(window)SDL_DestroyWindow(window);
            SDL_Quit();throw;
        }
        return result;
    }
    sim::World world() const {return sim::World::create_original_production_development(policy,model);}
    save::SaveDocument document(const sim::World& world) const {return save::make_document(data,map_relative,mask,world);}
    sim::World restore(const save::SaveDocument& document) const {
        return save::restore_original_production_development(document,data,mask,policy);
    }
};

inline const orig::HouseState& house(const sim::World& world,sim::BuildingId id) {return finance::house(world,id);}
inline sim::Cell offset(const sim::OriginalEconomyStarterReceipt& receipt,int x,int y) {return {receipt.origin.x+x,receipt.origin.y+y};}
inline sim::BuildingId body(const sim::World& world,sim::Object kind) {
    for(const auto& value:world.buildings())if(value.kind==kind)return value.id;
    throw std::runtime_error("Starter has no required paid building kind");
}
struct FoodFacts {
    std::uint64_t produced=0,consumed=0,house_received=0,house_consumed=0;
};
inline FoodFacts food_facts(const sim::World& world) {
    const auto& state=world.original_production_state()->food;
    sim::OriginalFoodCounters produced{},stock{},cargo{},consumed{};FoodFacts facts;
    for(const auto& farm:state.farms)produced[static_cast<unsigned>(farm.crop)]+=farm.harvested;
    for(const auto& row:state.stocks) {
        const auto farm=std::find_if(state.farms.begin(),state.farms.end(),[&](const auto& value){return value.farm_id==row.building_id;});
        const bool home=sim::original_house_kind(world.building(row.building_id).kind);
        for(unsigned g=0;g<sim::original_food_crop_count;++g) {
            check(row.amounts[g]>=0,"Original Food stock is negative");
            const auto harvest=farm!=state.farms.end()&&static_cast<unsigned>(farm->crop)==g?farm->harvested:0;
            check(harvest+row.received[g]+row.returned[g]==static_cast<std::uint64_t>(row.amounts[g])+row.departed[g]+row.consumed[g],
                "Original Food owner lost its real harvest/receipt/departure/return/consumption equation");
            stock[g]+=static_cast<std::uint64_t>(row.amounts[g]);consumed[g]+=row.consumed[g];
            if(home){facts.house_received+=row.received[g];facts.house_consumed+=row.consumed[g];}
        }
    }
    for(const auto& trip:state.trips)for(unsigned g=0;g<sim::original_food_crop_count;++g) {
        check(trip.loaded[g]>=0&&trip.delivered[g]>=0&&trip.returned[g]>=0&&trip.loaded[g]>=trip.delivered[g]+trip.returned[g],
            "Original Food trip has impossible physical cargo");
        cargo[g]+=static_cast<std::uint64_t>(trip.loaded[g]-trip.delivered[g]-trip.returned[g]);
    }
    for(unsigned g=0;g<sim::original_food_crop_count;++g) {
        check(produced[g]==stock[g]+cargo[g]+consumed[g],"Original Food crop mass differs from harvest/stock/cargo/sink");
        facts.produced+=produced[g];facts.consumed+=consumed[g];
    }
    return facts;
}
inline void authority(const sim::World& world) {
    check(world.navigation_valid() && world.population_valid() && world.production_balance_valid() && world.city_economy_valid(),
          "Paid starter broke navigation/population/production/finance authority");
    finance::accounting(world);
    for(unsigned i=0;i<orig::goods_count;++i)check(world.original_goods_balance(static_cast<orig::Goods>(i)).conserved,
        "Paid starter goods/cargo/reservation/sink conservation failed");
    (void)food_facts(world);
}
inline void tick(sim::World& world) {world.tick();authority(world);}
inline void through(sim::World& world,std::uint64_t end) {while(world.ticks()<end)tick(world);}
inline void execute(sim::World& world,sim::Command command) {
    const auto result=world.execute(command);check(result.accepted&&result.changed,"Starter ordinary command: "+result.reason);authority(world);
}
inline void until(sim::World& world,const std::function<bool()>& ready,unsigned bound,const char* reason) {
    for(unsigned i=0;i<bound&&!ready();++i)tick(world);
    check(ready(),reason);
}
inline void paid_empty(const Context& context,const sim::World& world,const sim::OriginalEconomyStarterReceipt& receipt) {
    check(world.ticks()==0 && world.total_population()==0 && world.taxes_collected_total()==0 &&
          world.maintenance_spent_total()==0 && world.food_produced_total()==0 && world.clay_extracted_total()==0 && world.pottery_completed_total()==0,
          "Starter granted elapsed time/residents/food/industry/tax/maintenance");
    const auto food=food_facts(world);check(food.produced==0&&food.consumed==0&&food.house_received==0,
        "Starter granted original Food harvest/arrival/consumption");
    check(receipt.commands_executed==87 && world.command_sequence()==87 && world.buildings().size()==19 &&
          world.snapshot().roads.size()==68,"Paid starter command/building/road identity count differs from its recipe");
    check(receipt.construction_spent>0 && receipt.construction_spent<=static_cast<std::uint64_t>(sim::original_production_starting_treasury) &&
          world.construction_spent_total()==receipt.construction_spent &&
          world.treasury()==sim::original_production_starting_treasury-static_cast<std::int64_t>(receipt.construction_spent),
          "Starter did not pay every ordinary construction debit");
    if(context.original_model)check(receipt.construction_spent==1404,"Unmodified Normal-model starter must pay exactly1404");
    for(std::size_t i=0;i<receipt.house_ids.size();++i) {
        const auto id=receipt.house_ids[i];check(static_cast<unsigned>(id)==i+1 && house(world,id).type==orig::HouseType::Vacant &&
            !house(world,id).occupants,"Starter did not buy four empty ordinary House containers");
    }
    for(unsigned i=0;i<orig::goods_count;++i) {
        const auto balance=world.original_goods_balance(static_cast<orig::Goods>(i));
        check(balance.stock==0 && balance.cargo==0 && balance.source_reserved==0 && balance.incoming_reserved==0 &&
              balance.totals==sim::OriginalGoodsTotals{},"Starter granted goods, cargo, reservations or production totals");
    }
    authority(world);
}
inline void worker_oracle(const sim::World& world) {
    std::vector<const sim::BuildingState*> ordered;for(const auto& b:world.buildings())if(world.workforce_required(b.id)>0 &&
        (!sim::World::operation_controllable(b.kind)||(b.operating_enabled&&!world.building_on_fire(b.id))))ordered.push_back(&b);
    std::sort(ordered.begin(),ordered.end(),[](const auto* a,const auto* b){return a->workforce_priority!=b->workforce_priority ?
        a->workforce_priority<b->workforce_priority:a->id<b->id;});
    int available=world.workforce_supply(),assigned=0;std::vector<sim::BuildingId> staffed;
    for(const auto* b:ordered)if(const auto need=world.workforce_required(b->id);need<=available){available-=need;assigned+=need;staffed.push_back(b->id);}
    for(const auto& b:world.buildings())check(world.workers_assigned(b.id)==
        (std::find(staffed.begin(),staffed.end(),b.id)!=staffed.end()?world.workforce_required(b.id):0),
        "Actual worker allocation differs from paid priority/ID/all-or-nothing budget");
    check(world.workforce_used()==assigned,"Actual global workers differ from the independently selected staffed owners");
}
inline void admission_and_priorities(sim::World& world,const sim::OriginalEconomyStarterReceipt& receipt) {
    through(world,119);check(!world.total_population(),"Starter admitted residents before paid age120");tick(world);
    check(world.total_population()==8 && world.workforce_supply()==8,"Four actual first admissions must supply exactly8 residents/workers");
    const auto water=body(world,sim::Object::OriginalWaterWell72),ancestor=body(world,sim::Object::OriginalAncestorShrine214),market=body(world,sim::Object::Market);
    check(world.building_staffed(water)&&world.building_staffed(ancestor)&&!world.building_staffed(market),"Paid purchase order did not bootstrap Water/Ancestor at scarce8 workers");
    const auto funds=world.treasury();execute(world,sim::set_building_workforce_priority(market,sim::WorkforcePriority::High));
    check(world.building_staffed(market)&&world.building_staffed(water)&&!world.building_staffed(ancestor),"Real High-priority Market did not displace later Normal Ancestor");worker_oracle(world);
    execute(world,sim::set_building_workforce_priority(ancestor,sim::WorkforcePriority::High));
    check(world.building_staffed(ancestor)&&world.building_staffed(market)&&!world.building_staffed(water),"Equal High priorities did not retain stable paid BuildingId order");worker_oracle(world);
    execute(world,sim::set_building_workforce_priority(ancestor,sim::WorkforcePriority::Normal));
    execute(world,sim::set_building_workforce_priority(market,sim::WorkforcePriority::Normal));
    check(world.treasury()==funds,"Ordinary priority controls changed funds");
    for(const auto id:receipt.house_ids)check(house(world,id).type==orig::HouseType::Shelter&&house(world,id).occupants==2,
        "Actual120t House admission changed during free worker controls");
    worker_oracle(world);
}
inline bool supplied(const sim::World& world,const sim::OriginalEconomyStarterReceipt& receipt) {
    for(const auto id:receipt.house_ids)if(house(world,id).type<orig::HouseType::PlainCottage)return false;
    return world.original_goods_balance(orig::Goods::Hemp).totals.consumed>0 &&
           world.original_goods_balance(orig::Goods::Ceramics).totals.consumed>0;
}
inline void upgrade(sim::World& world,const sim::OriginalEconomyStarterReceipt& receipt) {
    until(world,[&]{return supplied(world,receipt);},14000,"Paid original four-House starter never supplied both goods and real Plain-or-higher homes");
    for(const auto id:receipt.house_ids) {
        const auto& histories=world.original_production_state()->housing.houses;
        const auto h=std::find_if(histories.begin(),histories.end(),[&](const auto& row){return row.building_id==id;});
        check(h!=histories.end()&&h->first_transitions[2]&&h->first_transitions[2]->evaluation.input.type==orig::HouseType::Hut&&
            h->first_transitions[2]->evaluation.input.water&&h->first_transitions[2]->evaluation.input.ancestor>0&&
            h->first_transitions[2]->evaluation.input.food_quality>=20,"Actual Hut→Plain proof lacks real Water/Ancestor/Food input");
    }
    const auto food=food_facts(world);
    check(world.clay_extracted_total()>0&&world.pottery_completed_total()>0&&food.produced>0&&food.house_received>0&&food.house_consumed>0,
          "House upgrade bypassed real Clay/Pottery/original Food harvest/arrival/consumption");worker_oracle(world);
}
inline Json observe(const sim::World& world) {
    authority(world);const auto food=food_facts(world);Json houses=Json::array(),goods=Json::array(),buildings=Json::array();
    for(const auto& b:world.buildings()) {
        buildings.push_back({{"id",static_cast<unsigned>(b.id)},{"kind",static_cast<unsigned>(b.kind)},{"cell",{b.cell.x,b.cell.y}},
            {"workers",world.workforce_required(b.id)},{"assigned",world.workers_assigned(b.id)},{"operating",b.operating_enabled},
            {"priority",static_cast<unsigned>(b.workforce_priority)},{"burning",world.building_on_fire(b.id)}});
        if(!sim::original_house_kind(b.kind))continue;
        const auto& h=house(world,b.id);
        houses.push_back({{"id",static_cast<unsigned>(b.id)},{"type",static_cast<unsigned>(h.type)},{"occupants",h.occupants},
            {"water",h.water},{"ancestor",h.ancestor},{"herbalist",h.herbalist},{"acupuncture",h.acupuncture},{"music",h.music},
            {"food_quality",h.food_quality},{"hemp",h.hemp},{"ceramics",h.ceramics},{"DES",int(h.desirability)}});
    }
    for(unsigned g=0;g<orig::goods_count;++g) {
        const auto b=world.original_goods_balance(static_cast<orig::Goods>(g));goods.push_back({{"good",g},{"produced",b.totals.produced},
            {"imported",b.totals.imported},{"consumed",b.totals.consumed},{"demolished",b.totals.demolished},{"stock",b.stock},{"cargo",b.cargo},
            {"source_reserved",b.source_reserved},{"incoming_reserved",b.incoming_reserved},{"conserved",b.conserved}});
    }
    return {{"tick",world.ticks()},{"funds",world.treasury()},{"taxes",world.taxes_collected_total()},
        {"construction",world.construction_spent_total()},{"maintenance",world.maintenance_spent_total()},
        {"population",world.total_population()},{"workforce",world.workforce_supply()},{"installed_workers",world.workforce_required()},
        {"assigned_workers",world.workforce_used()},{"houses",houses},{"goods",goods},{"buildings",buildings},
        {"clay_extracted",world.clay_extracted_total()},{"pottery_produced",world.pottery_completed_total()},{"food_produced",food.produced},
        {"food_consumed",food.consumed},{"food_house_received",food.house_received},{"food_house_consumed",food.house_consumed},
        {"legacy_food_produced",world.food_produced_total()},
        {"model_sha256",world.original_rules_model()->provenance().sha256},{"difficulty",static_cast<unsigned>(world.original_rules_model()->difficulty())},
        {"rule",world.rule_version()},{"revision",world.original_production_state()->development_revision}};
}
using Checkpoint=std::function<void(const char*,const sim::World&)>;
inline Json operations(sim::World& world,const sim::OriginalEconomyStarterReceipt& receipt,const Checkpoint& checkpoint={}) {
    Json result=Json::object();const auto save_stage=[&](const char* name){if(checkpoint)checkpoint(name,world);};
    bool occupied=false;
    until(world,[&]{
      for(const auto& c:world.couriers())if(c.phase!=sim::CourierPhase::IdleAtWorkshop&&c.edge_progress>0&&c.path_vertex+1<c.path.size()) {
        for(const auto cell:{c.path[c.path_vertex],c.path[c.path_vertex+1]})if(world.object_at(cell)==sim::Object::Road) {
            const auto before=world.snapshot();const sim::Command remove{sim::CommandType::RemoveRoad,cell};const auto validation=world.validate(remove),execution=world.execute(remove);
            check(!validation.accepted&&!execution.accepted&&validation.reason=="Road occupied by courier"&&world.snapshot()==before,
                "A real occupied path edge was removed or partially mutated");occupied=true;return true;
        }
      }
      return false;
    },1000,"Starter never produced a real occupied edge for atomic removal refusal");
    result["occupied_edge_refusal"]=occupied;
    std::vector<sim::BuildingId> paused;for(const auto& b:world.buildings())if(sim::World::operation_controllable(b.kind)&&b.operating_enabled)paused.push_back(b.id);
    for(const auto id:paused)execute(world,sim::set_building_operation(id,false));
    check(world.workforce_used()==0&&world.active_workforce_required()==0,"Paused actual producers retained assigned workers");
    const auto maintenance=world.maintenance_spent_total();const auto installed=world.workforce_required();
    const auto funds=world.treasury();const auto taxes=world.taxes_collected_total();
    through(world,world.ticks()+400);
    check(world.maintenance_spent_total()-maintenance==2*static_cast<std::uint64_t>(installed)&&
        world.treasury()==funds+static_cast<std::int64_t>(world.taxes_collected_total()-taxes)-2*installed,
        "Pausing stopped the exact installed-worker400t upkeep or changed real taxes");
    until(world,[&]{return std::all_of(world.couriers().begin(),world.couriers().end(),[](const auto& c){return c.phase==sim::CourierPhase::IdleAtWorkshop;});},4000,
        "Normally paused couriers did not finish their actual paid round trips");save_stage("paused-idle");
    const std::array cuts{offset(receipt,3,4),offset(receipt,4,4),offset(receipt,5,2),offset(receipt,5,3)};
    const auto target=receipt.house_ids[1];for(const auto cell:cuts)execute(world,{sim::CommandType::RemoveRoad,cell});
    for(const auto id:paused)execute(world,sim::set_building_operation(id,true));
    const auto missing=world.original_service_availability(target,orig::ServiceRole::Herbalist);
    check(missing.available_source_count>0&&missing.reachable_source_count==0,"Disconnected inner House retained a cached live Herbalist route");
    through(world,world.ticks()+32);check(world.original_service_availability(target,orig::ServiceRole::Herbalist).last_arrival_tick==missing.last_arrival_tick,
        "A disconnected House received a fake service arrival");save_stage("disconnected");
    const auto repair_funds=world.treasury();const auto spending=world.construction_spent_total();const auto revision=world.road_revision();
    for(const auto cell:cuts)execute(world,{sim::CommandType::PlaceRoad,cell});
    check(world.treasury()==repair_funds-8&&world.construction_spent_total()==spending+8&&world.road_revision()==revision+4,
        "Four repaired real roads did not pay8 once and refresh each committed edge");
    const auto repaired=world.ticks();until(world,[&]{const auto live=world.original_service_availability(target,orig::ServiceRole::Herbalist);
        return live.reachable_source_count>0&&live.last_arrival_tick&&*live.last_arrival_tick>repaired&&live.delivered_timer>0;},3000,
        "Paid road repair never yielded a genuine later Herbalist arrival");save_stage("repaired-arrival");
    result["repair_cost"]=8;result["actual_repaired_arrival_tick"]=*world.original_service_availability(target,orig::ServiceRole::Herbalist).last_arrival_tick;
    std::vector<sim::BuildingId> ancestors;for(const auto& b:world.buildings())if(b.kind==sim::Object::OriginalAncestorShrine214)ancestors.push_back(b.id);
    until(world,[&]{return supplied(world,receipt);},6000,"Repaired city did not regain all four real supplied houses");
    const auto pause=world.ticks();for(const auto id:ancestors)execute(world,sim::set_building_operation(id,false));save_stage("ancestor-paused");
    sim::BuildingId down=receipt.house_ids.front();Json earlier_huts=Json::array(),down_proof=Json::object();std::uint64_t observed_hut_tick=0;
    const auto actual_ancestor_down=[&] {
        for(const auto id:receipt.house_ids) {
            if(house(world,id).type!=orig::HouseType::Hut)continue;
            const auto& rows=world.original_production_state()->housing.houses;
            const auto row=std::find_if(rows.begin(),rows.end(),[&](const auto& value){return value.building_id==id;});
            check(row!=rows.end(),"Paid starter lost real housing history during expiry");
            if(!row->last_evaluation||row->last_evaluation->tick!=world.ticks()||row->last_evaluation->input.type!=orig::HouseType::PlainCottage)continue;
            const auto& input=row->last_evaluation->input;const auto visit=world.original_service_availability(id,orig::ServiceRole::Ancestor);
            Json proof{{"tick",world.ticks()},{"house",static_cast<unsigned>(id)},{"input_type",static_cast<unsigned>(input.type)},
                {"output_type",static_cast<unsigned>(house(world,id).type)},{"input_counter",input.devolve_counter},
                {"input_ancestor",input.ancestor},{"input_water",input.water},{"input_food_quality",input.food_quality},
                {"input_DES",int(input.desirability)},{"occupants",input.occupants},
                {"last_real_ancestor_arrival",visit.last_arrival_tick?Json(*visit.last_arrival_tick):Json(nullptr)}};
            // A pure counterfactual proves the missing Ancestor is the sole
            // unmet current-row need. It is never written to a live/saved World.
            auto supplied_ancestor=input;supplied_ancestor.ancestor=1;
            const auto& rules=world.original_rules_model()->house_rules();
            if(!input.ancestor&&input.occupants>0&&input.devolve_counter==orig::devolve_limit(orig::HouseType::PlainCottage)&&
               input.desirability>rules.value(orig::ModelHouse::PlainCottage,orig::HouseField::DesirabilityLow)&&
               !orig::needs_met(rules,orig::ModelHouse::PlainCottage,input)&&orig::needs_met(rules,orig::ModelHouse::PlainCottage,supplied_ancestor)) {
                down=id;down_proof=std::move(proof);return true;
            }
            if(observed_hut_tick!=world.ticks()&&earlier_huts.size()<12){earlier_huts.push_back(std::move(proof));observed_hut_tick=world.ticks();}
        }
        return false;
    };
    bool ancestor_down=actual_ancestor_down();
    for(unsigned i=0;i<6500&&!ancestor_down;++i){tick(world);ancestor_down=actual_ancestor_down();}
    check(ancestor_down,"No sole-Ancestor-expiry Plain5→Hut4 transition; earlier actual Hut inputs="+earlier_huts.dump()+" final="+observe(world).dump());
    check(house(world,down).ancestor==0&&house(world,down).occupants>0,"Down checkpoint lacks actual expiry and retained residents");
    result["ancestor_down_proof"]=down_proof;result["earlier_other_hut_transitions"]=earlier_huts;
    result["ancestor_paused_tick"]=pause;result["actual_down_tick"]=world.ticks();result["actual_down_house"]=static_cast<unsigned>(down);save_stage("actual-down");
    for(const auto id:ancestors)execute(world,sim::set_building_operation(id,true));
    const auto resumed=world.ticks();
    until(world,[&]{const auto visit=world.original_service_availability(down,orig::ServiceRole::Ancestor);return house(world,down).type>=orig::HouseType::PlainCottage&&
        visit.last_arrival_tick&&*visit.last_arrival_tick>resumed&&visit.delivered_timer>0;},6500,
        "Normally resumed Ancestor did not yield a real later visit and paid House recovery");save_stage("recovered");
    result["actual_recovered_tick"]=world.ticks();
    const auto before=world.snapshot();const auto refused=world.execute(sim::demolish_building(receipt.house_ids.front()));
    check(!refused.accepted&&world.snapshot()==before,"Unsupported occupied Rule10 House demolition changed authority");
    const auto neutral_cell=offset(receipt,0,0);const auto debit=world.treasury();const auto workers=world.workforce_required();
    execute(world,{sim::CommandType::PlaceOriginalInfluenceSource,neutral_cell});const auto neutral=*world.building_owner_at(neutral_cell);
    check(world.treasury()==debit-60&&world.workforce_required()==workers,"Neutral Well72 purchase changed paid cost/workers");
    const auto paid=world.treasury();execute(world,sim::demolish_building(neutral));
    check(world.treasury()==paid&&!world.building_owner_at(neutral_cell)&&world.workforce_required()==workers,
        "Legal neutral source demolition refunded money or retained an owner/worker");result["neutral_source_demolition_paid_no_refund"]=true;
    worker_oracle(world);save_stage("operations-complete");return result;
}

inline void equal_roundtrip(const Context& context,const sim::World& world,const fs::path& destination) {
    save::write_save(destination,context.document(world),context.data,context.mask);const auto bytes=f::read_text(destination);
    const auto document=save::read_save(destination);auto restored=context.restore(document);
    check(document.source_schema_version==26&&document.world==world.snapshot()&&restored.snapshot()==world.snapshot(),
        "Full real original starter SaveDocument/restore authority differs");
    check(f::read_text(destination)==bytes,"Offline starter save/restore rewrote supplied bytes");worker_oracle(restored);
}
inline void continuation(const Context& context,const fs::path& output,sim::World& world,unsigned ticks=20000) {
    const auto begin=world.ticks();through(world,begin+ticks/2);
    save::write_save(output/"midpoint.save.json",context.document(world),context.data,context.mask);
    f::write_text(output/"midpoint.report.json",observe(world).dump(2)+"\n");through(world,begin+ticks);
    save::write_save(output/"result.save.json",context.document(world),context.data,context.mask);
    f::write_text(output/"result.report.json",observe(world).dump(2)+"\n");
    f::write_text(output/"replay.json",Json{{"status","PASS"},{"begin",begin},{"ticks",ticks},{"midpoint",begin+ticks/2},{"end",world.ticks()}}.dump(2)+"\n");
}
} // namespace oe_starter_fixture
