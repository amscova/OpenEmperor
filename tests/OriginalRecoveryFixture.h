#pragma once

// Shared earned-state corpus for strict Rule10 persistence and recovery tests.
// Original inputs are read-only; no live/saved authority is injected.
#include "OriginalStarterFixture.h"
#include "simulation/original/OriginalScheduler.h"
#include <functional>
#include <string>
#include <vector>

namespace oe_recovery_fixture {
namespace starter=oe_starter_fixture;
namespace oe=openemperor;
namespace sim=oe::simulation;
namespace orig=oe::original;
namespace save=oe::persistence;
namespace fs=std::filesystem;
using Context=starter::Context;
using Json=nlohmann::json;
using starter::check;

struct Checkpoint {
    std::string label;
    save::SaveDocument document;
};
using Observer=std::function<void(const Checkpoint&)>;

inline Context authored_context() {
    auto context=Context::authored();
    auto text=oe_finance_fixture::authored_model(3);
    oe_production_fixture::set_house_field(text,0,17,37);
    oe_production_fixture::set_house_field(text,2,9,1);
    oe_production_fixture::set_house_field(text,2,10,1);
    starter::f::write_text(context.data/"Model/EmperorBuildingModels.txt",text);
    context.model=save::prepare_original_production_rules_model(context.data,orig::Difficulty::Normal);
    return context;
}
inline void same_document(const save::SaveDocument& a,const save::SaveDocument& b) {
    check(a.world==b.world && a.map_relative==b.map_relative && a.map_sha256==b.map_sha256 &&
        a.buildable_sha256==b.buildable_sha256 &&
        a.map_permissions_policy_version==b.map_permissions_policy_version &&
        a.map_permissions_sha256==b.map_permissions_sha256,
        "Complete original SaveDocument authority/source/policy differs");
}
inline sim::World restore(const Context& context,const save::SaveDocument& document) {
    // A fresh source/model validation is deliberate. The prepared model in a
    // SaveDocument never substitutes for an unavailable or changed input.
    return save::restore_original_production_development(document,context.data,context.mask,context.policy);
}
inline Json observe(const sim::World& world) {
    auto result=starter::observe(world);
    const auto position=orig::scheduler_position(world.ticks());
    result["next_scheduler"]={{"phase",position.phase},{"step",position.step}};
    Json cargo=Json::array(),reservations=Json::array(),edges=Json::array();
    for (const auto& c:world.original_goods_state()->cargo)
        cargo.push_back({{"courier",static_cast<unsigned>(c.transfer.courier_id)},
            {"sequence",c.transfer.sequence},{"amount",c.amount},{"good",static_cast<unsigned>(c.good)}});
    for (const auto& r:world.original_goods_state()->reservations)
        reservations.push_back({{"courier",static_cast<unsigned>(r.transfer.courier_id)},
            {"sequence",r.transfer.sequence},{"phase",static_cast<unsigned>(r.phase)},
            {"reserved",r.reserved},{"loaded",r.loaded},{"delivered",r.delivered},{"returned",r.returned}});
    for (const auto& c:world.couriers()) if (c.phase!=sim::CourierPhase::IdleAtWorkshop && c.edge_progress>0)
        edges.push_back({{"courier",static_cast<unsigned>(c.id)},{"path_vertex",c.path_vertex},
            {"edge_progress",c.edge_progress},{"phase",static_cast<unsigned>(c.phase)}});
    result["typed_cargo"]=std::move(cargo);result["typed_reservations"]=std::move(reservations);
    result["begun_edges"]=std::move(edges);return result;
}
inline bool reserved_begun_edge(const sim::World& world) {
    for (const auto& cargo:world.original_goods_state()->cargo) if (cargo.amount>0) {
        const auto trip=std::find_if(world.original_goods_state()->reservations.begin(),
            world.original_goods_state()->reservations.end(),[&](const auto& row){return row.transfer==cargo.transfer;});
        if (trip==world.original_goods_state()->reservations.end() || trip->reserved<=0 ||
            trip->phase!=sim::OriginalGoodsTransferPhase::Loaded) continue;
        for (const auto& actor:world.couriers()) if (actor.id==cargo.transfer.courier_id &&
            actor.phase!=sim::CourierPhase::IdleAtWorkshop && actor.edge_progress>0 && actor.path_vertex+1<actor.path.size())
            return true;
    }
    return false;
}
inline std::uint64_t consumed(const sim::World& world) {
    return world.original_goods_balance(orig::Goods::Hemp).totals.consumed+
        world.original_goods_balance(orig::Goods::Ceramics).totals.consumed;
}
inline void through(sim::World& world,std::uint64_t tick) {
    while (world.ticks()<tick) world.tick();
    starter::authority(world);
}
inline std::vector<Checkpoint> build_corpus(const Context& context,bool include_debt=true,
                                           const Observer& observer={}) {
    std::vector<Checkpoint> corpus;
    auto world=context.world();const auto receipt=sim::place_original_economy_starter(world);
    starter::paid_empty(context,world,receipt);
    const auto publish_document=[&](std::string label,save::SaveDocument document) {
        Checkpoint point{std::move(label),std::move(document)};
        if (observer) observer(point);
        corpus.push_back(std::move(point));
    };
    const auto publish=[&](const char* label) {
        starter::authority(world);publish_document(label,context.document(world));
    };
    publish("paid-empty");
    for (unsigned n=0;n<14000 && !reserved_begun_edge(world);++n) world.tick();
    check(reserved_begun_edge(world),"Paid recovery corpus never reached actual typed cargo/reservation on a begun edge");
    publish("moving-reserved");
    for (unsigned n=0;n<14000 && !starter::supplied(world,receipt);++n) world.tick();
    check(starter::supplied(world,receipt),"Paid recovery corpus never supplied four Plain-or-higher houses and both consumed goods");
    const auto food=starter::food_facts(world);
    check(food.house_received>0 && food.house_consumed>0 &&
        !world.original_production_state()->services.witnesses.empty() && world.taxes_collected_total()>0 &&
        world.maintenance_spent_total()>0,"Rich recovery state lacks real Food/Service/finance events");
    publish("rich-supplied");

    bool consumption=false;
    for (unsigned n=0;n<1700 && !consumption;++n) {
        const auto dispatch=orig::next_scheduler_dispatch(world.ticks());
        if (dispatch && dispatch->consumption && dispatch->position.step==7) {
            const auto before=context.document(world);const auto amount=consumed(world);world.tick();
            if (consumed(world)>amount) {
                publish_document("before-consumption",before);publish("after-consumption");consumption=true;
            }
        } else world.tick();
    }
    check(consumption,"Recovery corpus never reached a physical step7/phase39 consumption boundary");

    std::vector<sim::BuildingId> ancestors;
    for (const auto& body:world.buildings()) if (body.kind==sim::Object::OriginalAncestorShrine214) ancestors.push_back(body.id);
    for (const auto id:ancestors) starter::execute(world,sim::set_building_operation(id,false));
    bool devolution=false;
    for (unsigned n=0;n<9000 && !devolution;++n) {
        const auto dispatch=orig::next_scheduler_dispatch(world.ticks());
        if (!dispatch || !dispatch->evolution) {world.tick();continue;}
        const auto before=context.document(world);world.tick();
        for (const auto id:receipt.house_ids) {
            const auto prior=std::find_if(before.world.original_economy->houses.begin(),before.world.original_economy->houses.end(),
                [&](const auto& h){return h.building_id==id;});
            const auto& current=starter::house(world,id);
            if (prior->house.type==orig::HouseType::PlainCottage && current.type==orig::HouseType::Hut &&
                current.occupants>0 && current.ancestor==0) {
                publish_document("before-devolution",before);publish("after-devolution");devolution=true;break;
            }
        }
    }
    check(devolution,"Recovery corpus never reached a real occupied Plain5-to-Hut4 phase39 devolution after Ancestor pause");

    const auto cell=starter::offset(receipt,0,0);const auto funds=world.treasury();
    const auto required=world.workforce_required();
    starter::execute(world,{sim::CommandType::PlaceOriginalInfluenceSource,cell});
    const auto neutral=*world.building_owner_at(cell);
    check(world.treasury()==funds-60 && world.workforce_required()==required,
        "Neutral Well72 purchase did not retain its ordinary paid/zero-worker contract");
    publish("before-neutral-demolition");const auto paid=world.treasury();
    starter::execute(world,sim::demolish_building(neutral));
    check(world.treasury()==paid && !world.building_owner_at(cell),"Legal neutral-owner demolition refunded money or retained ownership");
    publish("after-neutral-demolition");
    if (include_debt) {
        std::vector<sim::BuildingId> pause;
        for (const auto& b:world.buildings()) if (sim::World::operation_controllable(b.kind) && b.operating_enabled) pause.push_back(b.id);
        for (const auto id:pause) starter::execute(world,sim::set_building_operation(id,false));
        const auto maintenance=world.maintenance_spent_total();
        for (unsigned n=0;n<60000 && world.treasury()>=0;++n) world.tick();
        check(world.treasury()<0 && world.maintenance_spent_total()>maintenance,
            "Normally paused paid owners did not reach signed debt through actual upkeep");
        publish("debt");
    }
    return corpus;
}
} // namespace oe_recovery_fixture
