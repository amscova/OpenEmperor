// Explicit, on-demand diagnostics. No instrumentation or alternate economy is
// installed in the application's frame/tick paths. Original maps are read only.
#include "CityV16PlaythroughRecipe.h"
#include "ServiceSchedulerExperiment.h"
#include "app/ServiceCoverage.h"
#include "app/SandboxView.h"
#include "app/VisualSelection.h"
#include "core/PerformanceDiagnostics.h"
#include "maps/StoredMapSession.h"
#include "persistence/SandboxSave.h"
#include "renderer/FireSpriteSet.h"
#include "simulation/CityStartGuidance.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <climits>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <numeric>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#if defined(__APPLE__) || defined(__unix__)
#include <sys/resource.h>
#endif

namespace {
namespace oe = openemperor;
namespace sim = oe::simulation;
namespace fs = std::filesystem;
using J = nlohmann::json;
using Clock = std::chrono::steady_clock;
unsigned id(sim::BuildingId v) { return static_cast<unsigned>(v); }
void check(bool v, const std::string &message) {
    if (!v)
        throw std::runtime_error(message);
}
std::uint64_t bounded_ticks(const std::string &text, std::uint64_t minimum, std::uint64_t maximum) {
    std::uint64_t value = 0;
    const auto end = text.data() + text.size();
    const auto parsed = std::from_chars(text.data(), end, value);
    check(parsed.ec == std::errc{} && parsed.ptr == end && value >= minimum && value <= maximum,
          "invalid bounded tick count");
    return value;
}
void write(const fs::path &p, const J &j) {
    std::ofstream f(p);
    f << j.dump(2) << '\n';
    check(bool(f), "report write failed");
}
void line(std::ofstream &f, const J &j) {
    f << j.dump() << '\n';
    check(bool(f), "JSONL write failed");
}
bool beneath(const fs::path &p, const fs::path &base) {
    auto i = p.begin();
    for (auto b = base.begin(); b != base.end(); ++b, ++i)
        if (i == p.end() || *i != *b)
            return false;
    return true;
}
void invariants(const sim::World &w) {
    check(w.production_balance_valid() && w.food_balance_valid() && w.city_economy_valid() &&
              w.navigation_valid() && w.population_valid() && w.service_state_valid() &&
              w.health_state_valid() && w.fire_state_valid(),
          "World invariant failed at tick " + std::to_string(w.ticks()));
}
J workers(const sim::World &w) {
    J a = J::array();
    for (const auto &b : w.buildings())
        if (w.workforce_required(b.id))
            a.push_back({{"id", id(b.id)},
                         {"kind", sim::starter_building_name(b.kind)},
                         {"required", w.workforce_required(b.id)},
                         {"assigned", w.workers_assigned(b.id)},
                         {"running", b.operating_enabled},
                         {"priority", sim::workforce_priority_name(b.workforce_priority)},
                         {"burning", w.building_on_fire(b.id)}});
    return a;
}
J finances(const sim::World &w) {
    return {{"tick", w.ticks()},
            {"command_sequence", w.command_sequence()},
            {"funds", w.treasury()},
            {"debt",
             w.treasury() < 0 ? std::uint64_t(0) - static_cast<std::uint64_t>(w.treasury()) : 0},
            {"taxes_total", w.taxes_collected_total()},
            {"maintenance_total", w.maintenance_spent_total()},
            {"construction_total", w.construction_spent_total()},
            {"installed_upkeep_per400", w.current_maintenance_rate()},
            {"population", w.total_population()},
            {"capacity", w.total_population_capacity()},
            {"workers_supply", w.workforce_supply()},
            {"workers_assigned", w.workforce_used()},
            {"workers_active_required", w.active_workforce_required()},
            {"workers_installed_required", w.workforce_required()},
            {"level2", w.settlement_goal_households_ready()},
            {"goal", w.settlement_goal_reached()}};
}
J couriers(const sim::World &w) {
    J rows = J::array();
    for (const auto &c : w.couriers()) {
        J path = J::array();
        for (auto p : c.path)
            path.push_back({p.x, p.y});
        rows.push_back({{"id", static_cast<unsigned>(c.id)},
                        {"role", static_cast<unsigned>(c.role)},
                        {"owner", id(c.owner)},
                        {"target", id(c.target)},
                        {"good", static_cast<unsigned>(c.good)},
                        {"phase", sim::courier_phase_name(c.phase)},
                        {"cargo", c.cargo},
                        {"reservation", c.reserved},
                        {"route_pending", c.route_pending},
                        {"path_vertex", c.path_vertex},
                        {"edge_progress", c.edge_progress},
                        {"path", path},
                        {"dispatch_status", sim::courier_dispatch_status_name(
                                                w.courier_dispatch_status(c.id).status)}});
    }
    return rows;
}
J house(const sim::World &w, const sim::BuildingState &b, bool routes) {
    const auto d = w.household_demand_status(b.id);
    J flags = J::array();
    if (d.ready)
        flags.push_back("SUPPLIED");
    if (!d.food_available)
        flags.push_back("MISSING_FOOD");
    if (!d.pottery_available)
        flags.push_back("MISSING_POTTERY");
    if (!d.service_available)
        flags.push_back("MISSING_SERVICE");
    if (d.sick)
        flags.push_back("SICK");
    if (d.burning)
        flags.push_back("BURNING");
    if (!w.household_has_water(b.id))
        flags.push_back("DRY");
    if (w.household_desirability(b.id) < 10)
        flags.push_back("LOW_DESIRABILITY");
    if (b.population >= w.household_population_capacity(b.id))
        flags.push_back("CAPACITY_LIMITED");
    if (b.demand_progress != sim::Rules::household_demand_ticks - 1)
        flags.push_back("NOT_YET_DUE");
    J result = {{"id", id(b.id)},
                {"storage", {b.cell.x, b.cell.y}},
                {"placement_tick", b.placed_tick},
                {"population", w.household_population(b.id)},
                {"capacity", w.household_population_capacity(b.id)},
                {"historical_level", w.historical_household_level(b.id)},
                {"effective_level", w.household_level(b.id)},
                {"desirability", w.household_desirability(b.id)},
                {"water", w.household_has_water(b.id)},
                {"food", b.food_stock},
                {"pottery", b.pottery_stock},
                {"food_incoming", b.reserved_food_incoming},
                {"pottery_incoming", b.reserved_incoming},
                {"service_active", w.household_service_active(b.id)},
                {"service_until_tick", b.service_until_tick},
                {"sick", d.sick},
                {"sick_until_tick", b.sick_until_tick},
                {"health_risk", b.health_risk},
                {"health_protection_until", b.health_protection_until_tick},
                {"burning", d.burning},
                {"fire_until_tick", b.fire_until_tick},
                {"fire_risk", b.fire_risk},
                {"fire_protection_until", b.fire_protection_until_tick},
                {"fulfilled_total", b.fulfilled_demand},
                {"missed_total", b.missed_demand},
                {"taxes_total", b.taxes_paid_total},
                {"grace_remaining", w.household_move_in_grace_remaining(b.id)},
                {"demand_progress", b.demand_progress},
                {"next_demand_tick", w.ticks() + sim::Rules::household_demand_ticks -
                                         static_cast<unsigned>(b.demand_progress)},
                {"diagnostic_flags", flags},
                {"actual_demand_blockers_are_separate_from_water_desirability_capacity", true}};
    result["active_incoming_couriers"] = J::array();
    for (const auto &c : w.couriers())
        if (c.target == b.id && c.phase == sim::CourierPhase::ToWarehouse)
            result["active_incoming_couriers"].push_back({{"id", static_cast<unsigned>(c.id)},
                                                          {"role", static_cast<unsigned>(c.role)},
                                                          {"good", static_cast<unsigned>(c.good)},
                                                          {"cargo", c.cargo},
                                                          {"reservation", c.reserved},
                                                          {"route_pending", c.route_pending}});
    if (routes) {
        bool market = false, service = false;
        for (const auto &source : w.buildings()) {
            if (source.kind != sim::Object::Market && source.kind != sim::Object::ServicePost)
                continue;
            const bool reachable = w.find_building_route(source.id, b.id).has_value();
            if (source.kind == sim::Object::Market)
                market |= reachable;
            else
                service |= reachable;
        }
        result["market_reachable"] = market;
        result["service_reachable"] = service;
        result["entrance_count"] = w.building_entrances(b.id).size();
        if (!market || !service)
            result["diagnostic_flags"].push_back("ROUTE_BLOCKED");
    }
    return result;
}
J households(const sim::World &w, bool routes = true) {
    J a = J::array();
    for (const auto &b : w.buildings())
        if (b.kind == sim::Object::Household)
            a.push_back(house(w, b, routes));
    return a;
}
J production(const sim::World &w) {
    J a = J::array();
    for (const auto &b : w.buildings())
        if (b.kind != sim::Object::Household)
            a.push_back({{"id", id(b.id)},
                         {"kind", sim::starter_building_name(b.kind)},
                         {"storage", {b.cell.x, b.cell.y}},
                         {"workers", w.workers_assigned(b.id)},
                         {"required", w.workforce_required(b.id)},
                         {"running", b.operating_enabled},
                         {"priority", sim::workforce_priority_name(b.workforce_priority)},
                         {"burning", w.building_on_fire(b.id)},
                         {"input_clay", b.input_clay},
                         {"output", b.output},
                         {"active_recipe_clay", b.active_recipe_clay},
                         {"progress", b.progress},
                         {"pottery_stock", b.pottery_stock},
                         {"food_stock", b.food_stock},
                         {"incoming", b.reserved_incoming},
                         {"food_incoming", b.reserved_food_incoming},
                         {"entrances", w.building_entrances(b.id).size()}});
    return a;
}

struct Context {
    fs::path data, map, root;
    unsigned border=0;
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    std::unique_ptr<oe::SandboxView> view;
    Context(fs::path d, fs::path m, fs::path r, fs::path resources = {})
        : data(std::move(d)), map(std::move(m)), root(std::move(r)) {
        auto stored = oe::maps::load_stored_map_session(
            data, map, oe::maps::FootprintPolicy::EdgeByte4x4Preview,
            oe::maps::StoredGraphicsProfile::Slot8);
        border=stored.plan.border;
        check(SDL_CreateWindowAndRenderer("Isolated long-play adapter", 1100, 700,
                                          SDL_WINDOW_HIDDEN, &window, &renderer),
              SDL_GetError());
        view = std::make_unique<oe::SandboxView>(std::move(stored), false,
                                                 sim::RulesProfile::CityV16, 4);
        if (!resources.empty()) {
            const auto profiles=oe::detect_and_select_visual_profiles(data,resources);
            check(profiles.compatibility.compatible(),"UI visual pack fingerprint mismatch");
            view->set_walker_visuals(profiles.walker,profiles.walker_source);
            view->set_building_visuals(profiles.building,profiles.building_source);
            view->set_road_visuals(profiles.road,profiles.road_source);
            view->set_fire_visuals(profiles.fire,profiles.fire_source);
            view->set_fire_inspector_visuals(profiles.fire_inspector,profiles.fire_inspector_source);
            view->set_market_walker_visuals(profiles.market_walker,profiles.market_walker_source);
            view->set_service_walker_visuals(profiles.service_walker,profiles.service_walker_source);
            view->set_health_walker_visuals(profiles.health_walker,profiles.health_walker_source);
        }
        fs::create_directories(root / "saves");
        view->configure_save(data, map, root / "saves/current.json");
        view->initialize(window, renderer);
        check(view->world().rule_version() == 4 &&
                  view->world().map_permissions()->policy_version() == 2,
              "wrong live rule/policy");
    }
    ~Context() {
        if (view)
            view->shutdown();
        if (renderer)
            SDL_DestroyRenderer(renderer);
        if (window)
            SDL_DestroyWindow(window);
    }
    oe::persistence::SaveDocument document(const sim::World &w) {
        auto d = view->capture_save_document();
        d.world = w.snapshot();
        return d;
    }
    sim::World restore(const fs::path &file) {
        auto d = oe::persistence::read_save(file);
        check(d.source_schema_version == 20 && d.world.profile == sim::RulesProfile::CityV16 &&
                  d.world.rule_version == 4 && d.map_permissions_policy_version == 2,
              "save must be explicit City-v16 rule4/policy2/schema20");
        return oe::persistence::restore_save(d, data, view->buildable_mask(),
                                             view->world().map_permissions());
    }
};
struct Trace {
    std::function<void(const sim::WorldSnapshot&,const sim::World&,const J&)> observer;
    Context &context;
    fs::path root;
    std::ofstream demand, workforce, events, finance, commands, house_log;
    J last_finance, last_workers;
    unsigned successes = 0, misses = 0;
    J miss_reasons = J::object(), total_miss_reasons = J::object();
    int interval_pop_min = INT_MAX, interval_pop_max = 0;
    std::int64_t minimum_funds = INT64_MAX;
    std::uint64_t interval_staff_shortfall_ticks = 0;
    double tick_seconds = 0;
    std::uint64_t tick_calls = 0;
    J checkpoints = J::array();
    Trace(Context &c, fs::path p) : context(c), root(std::move(p)) {
        fs::create_directories(root);
        demand.open(root / "demand-events.jsonl");
        workforce.open(root / "workforce-events.jsonl");
        events.open(root / "production-events.jsonl");
        finance.open(root / "finance-400t.jsonl");
        commands.open(root / "commands.jsonl");
        house_log.open(root / "households-400t.jsonl");
    }
    void begin(const sim::World &w) {
        last_finance = finances(w);
        last_workers = workers(w);
        interval_pop_min = interval_pop_max = w.total_population();
        minimum_funds = w.treasury();
        line(workforce, {{"tick", w.ticks()},
                         {"phase", "initial_current_allocation"},
                         {"buildings", last_workers}});
        write(root / "households-initial.json", households(w));
    }
    void command(sim::World &w, sim::Command c, const char *purpose) {
        const auto before = w.treasury();
        const auto r = w.execute(c);
        check(r.accepted, "ordinary command rejected: " + r.reason);
        line(commands, {{"tick", w.ticks()},
                        {"type", static_cast<unsigned>(c.type)},
                        {"cell", {c.cell.x, c.cell.y}},
                        {"building_id", id(c.building_id)},
                        {"operation_argument", c.operating_enabled},
                        {"priority_argument", sim::workforce_priority_name(c.workforce_priority)},
                        {"cost", before - w.treasury()},
                        {"changed", r.changed},
                        {"purpose", purpose}});
    }
    void interval(const sim::World &w) {
        const auto current = finances(w);
        J row = current;
        row["delta_taxes"] = current["taxes_total"].get<std::uint64_t>() -
                             last_finance["taxes_total"].get<std::uint64_t>();
        row["delta_maintenance"] = current["maintenance_total"].get<std::uint64_t>() -
                                   last_finance["maintenance_total"].get<std::uint64_t>();
        row["delta_construction"] = current["construction_total"].get<std::uint64_t>() -
                                    last_finance["construction_total"].get<std::uint64_t>();
        row["net_funds"] =
            current["funds"].get<std::int64_t>() - last_finance["funds"].get<std::int64_t>();
        row["from_tick"] = last_finance["tick"];
        row["boundary_phase"] =
            "actual snapshot after tick, before commands scheduled at this tick";
        row["demand_successes"] = successes;
        row["demand_misses"] = misses;
        row["miss_reason_exposures"] = miss_reasons;
        row["reason_counts_can_overlap"] = true;
        row["population_min"] = interval_pop_min;
        row["population_max"] = interval_pop_max;
        row["staff_shortfall_tick_exposures"] = interval_staff_shortfall_ticks;
        check(row["net_funds"].get<std::int64_t>() ==
                  static_cast<std::int64_t>(row["delta_taxes"].get<std::uint64_t>()) -
                      static_cast<std::int64_t>(row["delta_maintenance"].get<std::uint64_t>()) -
                      static_cast<std::int64_t>(row["delta_construction"].get<std::uint64_t>()),
              "interval money identity");
        line(finance, row);
        line(house_log, {{"tick", w.ticks()},
                         {"households", households(w)},
                         {"production", production(w)},
                         {"couriers", couriers(w)}});
        last_finance = current;
        successes = misses = 0;
        miss_reasons = J::object();
        interval_pop_min = interval_pop_max = w.total_population();
        interval_staff_shortfall_ticks = 0;
    }
    void tick(sim::World &w) {
        const auto pre_workers = workers(w);
        if (w.workforce_supply() < w.active_workforce_required())
            ++interval_staff_shortfall_ticks;
        if (pre_workers != last_workers) {
            line(workforce, {{"tick", w.ticks() + 1},
                             {"phase", "pre_tick_actual_allocation"},
                             {"buildings", pre_workers}});
            last_workers = pre_workers;
        }
        const auto before = w.snapshot();
        const auto start = Clock::now();
        w.tick();
        tick_seconds += std::chrono::duration<double>(Clock::now() - start).count();
        ++tick_calls;
        invariants(w);
        interval_pop_min = std::min(interval_pop_min, w.total_population());
        interval_pop_max = std::max(interval_pop_max, w.total_population());
        minimum_funds = std::min(minimum_funds, w.treasury());
        for (const auto &b : w.buildings()) {
            const auto p = std::find_if(before.buildings.begin(), before.buildings.end(),
                                        [&](const auto &a) { return a.id == b.id; });
            if (p == before.buildings.end())
                continue;
            if (b.kind == sim::Object::Household && (b.fulfilled_demand != p->fulfilled_demand ||
                                                     b.missed_demand != p->missed_demand)) {
                const bool ok = b.fulfilled_demand > p->fulfilled_demand;
                J reasons = J::array();
                if (!ok) {
                    if (p->food_stock <= 0)
                        reasons.push_back("MISSING_FOOD");
                    if (p->pottery_stock <= 0)
                        reasons.push_back("MISSING_POTTERY");
                    if (p->service_until_tick <= w.ticks())
                        reasons.push_back("MISSING_SERVICE");
                    if (p->sick_until_tick > w.ticks())
                        reasons.push_back("SICK");
                    if (p->fire_until_tick > w.ticks())
                        reasons.push_back("BURNING");
                    for (const auto &reason : reasons) {
                        auto &n = miss_reasons[reason.get<std::string>()];
                        n = n.is_null() ? 1 : n.get<unsigned>() + 1;
                    }
                }
                for (const auto &reason : reasons) {
                    auto &n = total_miss_reasons[reason.get<std::string>()];
                    n = n.is_null() ? 1 : n.get<unsigned>() + 1;
                }
                ok ? ++successes : ++misses;
                const auto cap = w.household_population_capacity(b.id);
                const bool grace =
                    w.ticks() - p->placed_tick < w.active_rules().household_move_in_grace_ticks;
                const int expected =
                    p->population > cap ? p->population - 1
                    : (!ok && !grace && p->population > sim::Rules::household_min_population)
                        ? p->population - 1
                    : (ok && p->population < cap) ? p->population + 1
                                                  : p->population;
                check(expected == b.population, "independent demand/population contract violation");
                line(demand, {{"tick", w.ticks()},
                              {"id", id(b.id)},
                              {"input_food", p->food_stock},
                              {"input_pottery", p->pottery_stock},
                              {"input_service_until", p->service_until_tick},
                              {"input_sick_until", p->sick_until_tick},
                              {"input_fire_until", p->fire_until_tick},
                              {"input_progress", p->demand_progress},
                              {"grace_at_deadline", grace},
                              {"actual_success", ok},
                              {"actual_reasons", reasons},
                              {"consumed_food", b.food_consumed_total - p->food_consumed_total},
                              {"consumed_pottery", b.consumed_total - p->consumed_total},
                              {"tax", b.taxes_paid_total - p->taxes_paid_total},
                              {"population_before", p->population},
                              {"population_after", b.population},
                              {"capacity_after", cap},
                              {"fulfilled_total", b.fulfilled_demand},
                              {"missed_total", b.missed_demand}});
            }
            if (b.clay_extracted != p->clay_extracted ||
                b.recipes_completed != p->recipes_completed || b.food_produced != p->food_produced)
                line(events, {{"tick", w.ticks()},
                              {"event", "production"},
                              {"building", id(b.id)},
                              {"clay_delta", b.clay_extracted - p->clay_extracted},
                              {"pottery_delta", b.recipes_completed - p->recipes_completed},
                              {"food_delta", b.food_produced - p->food_produced},
                              {"workers_at_tick_start", pre_workers}});
            if (b.sick_until_tick != p->sick_until_tick ||
                b.fire_until_tick != p->fire_until_tick ||
                b.health_protection_until_tick != p->health_protection_until_tick ||
                b.fire_protection_until_tick != p->fire_protection_until_tick)
                line(events, {{"tick", w.ticks()},
                              {"event", "safety_transition"},
                              {"building", id(b.id)},
                              {"sick_until_before", p->sick_until_tick},
                              {"sick_until_after", b.sick_until_tick},
                              {"fire_until_before", p->fire_until_tick},
                              {"fire_until_after", b.fire_until_tick},
                              {"health_protection_before", p->health_protection_until_tick},
                              {"health_protection_after", b.health_protection_until_tick},
                              {"fire_protection_before", p->fire_protection_until_tick},
                              {"fire_protection_after", b.fire_protection_until_tick}});
        }
        for (const auto &c : w.couriers()) {
            const auto p = std::find_if(before.couriers.begin(), before.couriers.end(),
                                        [&](const auto &a) { return a.id == c.id; });
            if (p != before.couriers.end() &&
                (p->phase != c.phase || p->route_pending != c.route_pending))
                line(events, {{"tick", w.ticks()},
                              {"event", "courier_transition"},
                              {"id", static_cast<unsigned>(c.id)},
                              {"role", static_cast<unsigned>(c.role)},
                              {"owner", id(c.owner)},
                              {"target", id(c.target)},
                              {"phase", sim::courier_phase_name(c.phase)},
                              {"cargo_before", p->cargo},
                              {"cargo_after", c.cargo},
                              {"reservation_before", p->reserved},
                              {"reservation_after", c.reserved},
                              {"route_pending", c.route_pending},
                              {"path_vertices", c.path.size()},
                              {"dispatch_status", sim::courier_dispatch_status_name(
                                                      w.courier_dispatch_status(c.id).status)}});
        }
        if (observer) observer(before,w,pre_workers);
        if (w.ticks() % 400 == 0)
            interval(w);
    }
    void checkpoint(const sim::World &w, const std::string &tag) {
        auto doc = context.document(w);
        const auto file = root / (tag + ".json");
        oe::persistence::write_save(file, doc, context.data, context.view->buildable_mask());
        auto parsed = oe::persistence::read_save(file);
        auto restored = context.restore(file);
        check(parsed.world == w.snapshot() && restored.snapshot() == w.snapshot(),
              "full checkpoint mismatch");
        auto expected = w;
        for (unsigned i = 0; i < 100; ++i) {
            expected.tick();
            restored.tick();
            invariants(expected);
            invariants(restored);
            check(expected.snapshot() == restored.snapshot(),
                  "checkpoint deterministic continuation mismatch");
        }
        const auto expected_file = root / (tag + "-plus100.json");
        oe::persistence::write_save(expected_file, context.document(expected), context.data,
                                    context.view->buildable_mask());
        // Real production F5/F9 handlers, explicitly scripted direct SDL delivery.
        context.view->configure_save(context.data, context.map, file);
        context.view->load_now();
        check(context.view->world().snapshot() == w.snapshot(), "view restore mismatch");
        bool running = true;
        SDL_Event e{};
        e.type = SDL_EVENT_KEY_DOWN;
        e.key.key = SDLK_F5;
        context.view->handle_event(e, running);
        check(oe::persistence::read_save(file).world == w.snapshot(), "F5 mismatch");
        context.view->tick_once();
        e.key.key = SDLK_F9;
        context.view->handle_event(e, running);
        check(context.view->world().snapshot() == w.snapshot() && context.view->paused(),
              "F9 must restore the complete snapshot and pause");
        checkpoints.push_back({{"tag", tag},
                               {"save", file.string()},
                               {"expected_plus100", expected_file.string()},
                               {"ticks", 100},
                               {"source_schema", parsed.source_schema_version},
                               {"policy", parsed.map_permissions_policy_version},
                               {"map_sha256", parsed.map_sha256},
                               {"policy_fingerprint", parsed.map_permissions_sha256},
                               {"complete_snapshot_equal", true},
                               {"scripted_view_f5_f9", true},
                               {"native_human_input", false}});
    }
    void finish(const sim::World &w) {
        if (last_finance["tick"] != w.ticks())
            interval(w);
        write(root / "households.json", households(w));
        write(root / "workforce-final.json", workers(w));
        write(root / "production-final.json", production(w));
        write(root / "couriers-final.json", couriers(w));
        write(root / "measured-tick-cost.json",
              {{"tick_calls", tick_calls},
               {"production_tick_seconds_excluding_reports", tick_seconds},
               {"minimum_observed_funds_after_ticks", minimum_funds},
               {"actual_miss_reason_exposures", total_miss_reasons}});
        write(root / "save-restart-comparison.json",
              {{"same_process_checkpoints", checkpoints},
               {"separate_process", "pending wrapper verification"}});
    }
};

sim::Cell choose_origin(const sim::World &w) {
    const auto starter = oe::city_playthrough::paid_starter({0, 0});
    const auto expansion = oe::city_playthrough::expansion_recipe({0, 0});
    std::vector<sim::Command> all = starter;
    for (const auto &a : expansion)
        all.push_back({a.type, a.cell});
    std::vector<sim::Cell> candidates;
    for (int y = 3; y + 14 < w.height(); ++y)
        for (int x = 3; x + 24 < w.width(); ++x)
            candidates.push_back({x, y});
    std::sort(candidates.begin(), candidates.end(), [&](auto a, auto b) {
        const auto distance = [&](auto c) {
            return std::abs(c.x + 11 - w.width() / 2) + std::abs(c.y + 6 - w.height() / 2);
        };
        return distance(a) != distance(b) ? distance(a) < distance(b)
               : a.y != b.y               ? a.y < b.y
                                          : a.x < b.x;
    });
    for (auto origin : candidates) {
        bool valid = true;
        for (const auto &c : all) {
            const sim::Cell cell{origin.x + c.cell.x, origin.y + c.cell.y};
            if (c.type == sim::CommandType::PlaceRoad) {
                if (!w.validate({c.type, cell}).accepted) {
                    valid = false;
                    break;
                }
            } else {
                const auto kind = *sim::placed_object(c.type);
                for (auto p :
                     sim::building_footprint_cells(w.profile(), w.rule_version(), kind, cell))
                    if (!w.buildable(p) || w.object_at(p) != sim::Object::Empty)
                        valid = false;
                if (!valid)
                    break;
            }
        }
        if (valid)
            return origin;
    }
    throw std::runtime_error("no complete legal translated paid district on this original map");
}

// Placement planning belongs only to this explicit harness. Every selected
// road/building is then purchased through the unchanged production commands.
// Trial copies only check real validation/routing, never invent simulated goods.
std::vector<sim::Command> connected_purchase(const sim::World &w, sim::CommandType type,
                                             bool residential) {
    const auto kind = *sim::placed_object(type);
    std::vector<sim::Cell> candidates;
    for (int y = 2; y + 2 < w.height(); ++y)
        for (int x = 2; x + 2 < w.width(); ++x) {
            const sim::Cell p{x, y};
            if (!w.validate({type, p}).accepted)
                continue;
            if (residential &&
                (!w.household_has_water_at(p) || w.household_desirability_at(p) < -20))
                continue;
            if (!residential && sim::desirability_impact(kind) < 0) {
                bool too_close = false;
                for (const auto &home : w.buildings())
                    if (home.kind == sim::Object::Household &&
                        sim::footprint_distance(
                            p, sim::building_footprint(w.profile(), w.rule_version(), kind),
                            home.cell,
                            sim::building_footprint(w.profile(), w.rule_version(), home.kind)) <=
                            sim::desirability_radius)
                        too_close = true;
                if (too_close)
                    continue;
            }
            candidates.push_back(p);
        }
    const auto &anchor = w.buildings().front().cell;
    std::sort(candidates.begin(), candidates.end(), [&](auto a, auto b) {
        if (residential && w.household_desirability_at(a) != w.household_desirability_at(b))
            return w.household_desirability_at(a) > w.household_desirability_at(b);
        const auto dist = [&](auto c) {
            return std::abs(c.x - anchor.x) + std::abs(c.y - anchor.y);
        };
        return dist(a) != dist(b) ? dist(a) < dist(b) : a.y != b.y ? a.y < b.y : a.x < b.x;
    });
    for (auto p : candidates) {
        const auto footprint =
            sim::building_footprint_cells(w.profile(), w.rule_version(), kind, p);
        std::set<std::pair<int, int>> occupied;
        for (auto c : footprint)
            occupied.emplace(c.x, c.y);
        std::vector<sim::Cell> perimeter;
        for (auto c : footprint)
            for (auto d : std::array<sim::Cell, 4>{{{0, -1}, {1, 0}, {0, 1}, {-1, 0}}}) {
                sim::Cell n{c.x + d.x, c.y + d.y};
                if (!occupied.contains({n.x, n.y}))
                    perimeter.push_back(n);
            }
        for (auto entrance : perimeter) {
            std::vector<sim::Cell> queue{entrance};
            std::map<std::pair<int, int>, sim::Cell> prior;
            prior[{entrance.x, entrance.y}] = entrance;
            std::optional<sim::Cell> end;
            for (std::size_t q = 0; q < queue.size() && q < 512; ++q) {
                const auto cell = queue[q];
                if (w.object_at(cell) == sim::Object::Road || w.original_road(cell)) {
                    end = cell;
                    break;
                }
                if (std::abs(cell.x - entrance.x) + std::abs(cell.y - entrance.y) >= 10)
                    continue;
                for (auto d : std::array<sim::Cell, 4>{{{0, -1}, {1, 0}, {0, 1}, {-1, 0}}}) {
                    sim::Cell n{cell.x + d.x, cell.y + d.y};
                    if (occupied.contains({n.x, n.y}) || prior.contains({n.x, n.y}) ||
                        !w.validate({sim::CommandType::PlaceRoad, n}).accepted)
                        continue;
                    prior[{n.x, n.y}] = cell;
                    queue.push_back(n);
                }
            }
            if (!end)
                continue;
            std::vector<sim::Command> purchase;
            auto cell = *end;
            while (cell != entrance) {
                purchase.push_back({sim::CommandType::PlaceRoad, cell});
                cell = prior.at({cell.x, cell.y});
            }
            purchase.push_back({sim::CommandType::PlaceRoad, entrance});
            purchase.push_back({type, p});
            auto trial = w;
            bool valid = true;
            for (auto c : purchase)
                if (!trial.execute(c).accepted) {
                    valid = false;
                    break;
                }
            if (!valid)
                continue;
            const auto added = trial.building_owner_at(p);
            check(added.has_value(), "trial purchase has no owner");
            bool reachable = false;
            for (const auto &source : trial.buildings())
                if (source.id != *added) {
                    const bool relevant =
                        residential                       ? source.kind == sim::Object::Market
                        : kind == sim::Object::ClaySource ? source.kind == sim::Object::Pottery
                        : kind == sim::Object::Pottery    ? source.kind == sim::Object::Warehouse
                        : kind == sim::Object::Warehouse  ? source.kind == sim::Object::Market
                        : kind == sim::Object::Farm       ? source.kind == sim::Object::Market
                                                          : false;
                    if (relevant && trial.find_building_route(*added, source.id))
                        reachable = true;
                }
            if (reachable)
                return purchase;
        }
    }
    throw std::runtime_error("no affordable legally connected purchase for " +
                             std::string(sim::starter_building_name(kind)));
}

#include "CityV16ServiceAudit.h"
#include "CityV16ServiceChallenges.h"

J observe(Context &context, sim::World w, std::uint64_t ticks, const char *evidence) {
    Trace t(context, context.root);
    t.begin(w);
    const auto start = finances(w);
    J at10000;
    int min_pop = w.total_population(), max_pop = min_pop, min_workers = w.workforce_supply();
    for (std::uint64_t i = 0; i < ticks; ++i) {
        t.tick(w);
        min_pop = std::min(min_pop, w.total_population());
        max_pop = std::max(max_pop, w.total_population());
        min_workers = std::min(min_workers, w.workforce_supply());
        if (i + 1 == 10000) {
            at10000 = finances(w);
            if (ticks > 10000)
                t.checkpoint(w, "observed-plus10000");
        }
    }
    J result = {{"status", "OBSERVED"},
                {"evidence", evidence},
                {"start", start},
                {"final", finances(w)},
                {"at10000", at10000},
                {"ticks", ticks},
                {"population_min", min_pop},
                {"population_max", max_pop},
                {"minimum_workforce_supply", min_workers},
                {"production_tick_seconds_excluding_reports", t.tick_seconds},
                {"engine_contract_violations", 0}};
    t.checkpoint(w, "observed-end");
    t.finish(w);
    write(context.root / "summary.json", result);
    write(context.root / "goal-stability.json", result);
    return result;
}

J reconstructed(Context &context, const fs::path &earned_save) {
    auto w = context.restore(earned_save);
    Trace t(context, context.root);
    t.begin(w);
    while (std::count_if(w.buildings().begin(), w.buildings().end(),
                         [](const auto &b) { return b.kind == sim::Object::Household; }) < 18) {
        for (auto c : connected_purchase(w, sim::CommandType::PlaceHousehold, true))
            t.command(w, c, "paid expansion to18 Houses; not an exact player reconstruction");
    }
    bool paused = false;
    for (auto i = w.buildings().rbegin(); i != w.buildings().rend(); ++i)
        if (i->kind == sim::Object::ServicePost) {
            t.command(w, sim::set_building_operation(i->id, false),
                      "explicit reconstructed Service outage; not asserted for the player");
            paused = true;
            break;
        }
    check(paused, "reconstructed scenario has no Service Post");
    t.checkpoint(w, "before-under-supply");
    const auto start = finances(w);
    for (unsigned i = 0; i < 10000; ++i)
        t.tick(w);
    t.checkpoint(w, "under-supplied-10000");
    for (unsigned i = 0; i < 10000; ++i)
        t.tick(w);
    t.checkpoint(w, "under-supplied-baseline");
    t.finish(w);
    J r = {{"status", "RECONSTRUCTED"},
           {"evidence", "RECONSTRUCTED SCENARIO: paid18-House expansion plus a legal Service "
                        "pause; no claim of exact tick13067/60 population"},
           {"start", start},
           {"final", finances(w)},
           {"no_commands_observation_ticks", 20000},
           {"baseline_save", (context.root / "under-supplied-baseline.json").string()},
           {"actual_miss_reason_exposures", t.total_miss_reasons},
           {"engine_contract_violations", 0}};
    write(context.root / "summary.json", r);
    write(context.root / "goal-stability.json", r);
    return r;
}

J recovery(Context &context, const fs::path &save,bool service_prepare=false) {
    auto w = context.restore(save);
    Trace t(context, context.root);
    t.begin(w);
    J stages = J::array();
    const auto start = finances(w);
    // First restore existing supply and use legal priorities. These change no
    // funds or goods and cannot evade the ownership-maintenance bill.
    const bool staffing_shortfall = w.workforce_supply() < w.active_workforce_required();
    for (const auto &b : w.buildings())
        if (b.kind == sim::Object::ServicePost || b.kind == sim::Object::Market ||
            b.kind == sim::Object::Farm) {
            if (!b.operating_enabled)
                t.command(w, sim::set_building_operation(b.id, true),
                          "resume existing essential supply first");
            if (staffing_shortfall)
                t.command(w,
                          sim::set_building_workforce_priority(b.id, sim::WorkforcePriority::High),
                          "prioritize essential supply only when workforce is actually short");
        }
    for (unsigned i = 0; i < 4000; ++i)
        t.tick(w);
    stages.push_back({{"stage", "existing supply / controls only"}, {"after", finances(w)}});
    if (service_prepare) {
        t.checkpoint(w,"service-start");t.finish(w);
        J result={{"evidence","same paid18-House outage source and ordinary controls-only4000t recovery"},
                  {"start",start},{"stages",stages},{"final",finances(w)}};
        write(context.root/"summary.json",result);return result;
    }
    // The measured reconstruction needs no paid expansion. On another supplied
    // save, a goods miss alone cannot distinguish capacity, backlog or routing;
    // report that open diagnosis rather than automatically buying producers.
    const bool goods_diagnosis_needed = t.total_miss_reasons.value("MISSING_FOOD", 0U) ||
                                        t.total_miss_reasons.value("MISSING_POTTERY", 0U);
    const auto fixed_start = finances(w);
    int min_pop = w.total_population();
    std::uint64_t goal_lost = 0;
    for (unsigned i = 0; i < 10000; ++i) {
        t.tick(w);
        min_pop = std::min(min_pop, w.total_population());
        if (!w.settlement_goal_reached())
            ++goal_lost;
    }
    check(w.command_sequence() == fixed_start["command_sequence"].get<std::uint64_t>(),
          "recovery stability received commands");
    t.checkpoint(w, "recovery-end");
    t.finish(w);
    J r = {{"status", w.settlement_goal_reached() ? "GOAL_REACHED" : "RECOVERY_INCOMPLETE"},
           {"evidence", "ordinary operation/priority commands; no paid expansion required in the "
                        "measured reconstruction; other goods failures retain open diagnosis"},
           {"additional_goods_diagnosis_needed_after_controls", goods_diagnosis_needed},
           {"start", start},
           {"stages", stages},
           {"fixed_window_start", fixed_start},
           {"final", finances(w)},
           {"fixed_window_ticks", 10000},
           {"fixed_window_min_population", min_pop},
           {"fixed_window_goal_lost_ticks", goal_lost},
           {"actual_miss_reason_exposures", t.total_miss_reasons},
           {"engine_contract_violations", 0}};
    write(context.root / "summary.json", r);
    write(context.root / "goal-stability.json", r);
    return r;
}

J priority_only(Context &context, const fs::path &save) {
    auto w = context.restore(save);
    Trace t(context, context.root);
    t.begin(w);
    const auto start = finances(w);
    for (const auto &b : w.buildings())
        if (b.kind == sim::Object::ServicePost || b.kind == sim::Object::Market ||
            b.kind == sim::Object::Farm)
            t.command(w, sim::set_building_workforce_priority(b.id, sim::WorkforcePriority::High),
                      "separate priority-only experiment; paused supply remains paused");
    for (unsigned i = 0; i < 10000; ++i)
        t.tick(w);
    t.checkpoint(w, "priority-only-end");
    t.finish(w);
    J r = {{"status", "OBSERVED_PRIORITY_ONLY"},
           {"evidence", "legal priority commands; no reactivation or capacity changes"},
           {"start", start},
           {"final", finances(w)},
           {"actual_miss_reason_exposures", t.total_miss_reasons},
           {"ticks", 10000},
           {"engine_contract_violations", 0}};
    write(context.root / "summary.json", r);
    write(context.root / "goal-stability.json", r);
    return r;
}
J fresh(Context &context) {
    auto w = sim::World::restore(context.view->world().snapshot(),
                                 context.view->world().map_permissions());
    const auto origin = choose_origin(w);
    Trace t(context, context.root);
    t.begin(w);
    for (auto c : oe::city_playthrough::paid_starter(origin))
        t.command(w, c, "normally paid starter");
    check(w.ticks() == 0 && w.total_population() == 24 && w.treasury() >= 20,
          "starter authority changed");
    const auto actions = oe::city_playthrough::expansion_recipe(origin);
    std::size_t next = 0;
    std::optional<std::uint64_t> goal;
    J goal_start;
    bool checkpoint_b = false;
    int min_pop = 100000, max_pop = 0, min_l2 = 100000;
    std::uint64_t goal_lost_ticks = 0;
    J interruptions = J::array();
    std::optional<std::uint64_t> lost_start;
    for (unsigned step = 0; step < 30000; ++step) {
        t.tick(w);
        while (next < actions.size() && actions[next].tick <= w.ticks()) {
            const auto &a = actions[next];
            if (!w.validate({a.type, a.cell}).accepted) {
                check(w.validate({a.type, a.cell}).diagnostic.blocker ==
                          sim::BuildBlocker::InsufficientFunds,
                      "paid expansion has a non-financial blocker");
                break;
            }
            t.command(w, {a.type, a.cell}, a.name);
            ++next;
        }
        if (w.ticks() == 399)
            t.checkpoint(w, "A-before-first-supply-miss");
        if (!checkpoint_b && w.ticks() >= 4406 &&
            std::any_of(w.couriers().begin(), w.couriers().end(), [](const auto &c) {
                return c.phase == sim::CourierPhase::ToWarehouse && c.edge_progress > 0;
            })) {
            t.checkpoint(w, "B-active-courier");
            checkpoint_b = true;
        }
        if (!goal && next == actions.size() && w.settlement_goal_reached()) {
            goal = w.ticks();
            goal_start = finances(w);
            t.checkpoint(w, "first-goal");
        }
        if (goal) {
            min_pop = std::min(min_pop, w.total_population());
            max_pop = std::max(max_pop, w.total_population());
            min_l2 = std::min(min_l2, w.settlement_goal_households_ready());
            if (!w.settlement_goal_reached()) {
                ++goal_lost_ticks;
                if (!lost_start)
                    lost_start = w.ticks();
            } else if (lost_start) {
                interruptions.push_back(
                    {{"from", *lost_start}, {"through", w.ticks() - 1}, {"recovered", true}});
                lost_start.reset();
            }
            if (w.ticks() - *goal >= 10000)
                break;
        }
    }
    if (lost_start)
        interruptions.push_back(
            {{"from", *lost_start}, {"through", w.ticks()}, {"recovered", false}});
    const auto final = finances(w);
    J result = {
        {"evidence",
         "RECONSTRUCTED SCENARIO / actual paid original-map World and scripted save handlers"},
        {"map", context.map.generic_string()},
        {"origin", {origin.x, origin.y}},
        {"rule", 4},
        {"policy", 2},
        {"schema", 20},
        {"first_goal_tick", goal ? J(*goal) : J(nullptr)},
        {"goal_start", goal_start},
        {"final", final},
        {"stability_ticks", goal ? w.ticks() - *goal : 0},
        {"population_min", min_pop},
        {"population_max", max_pop},
        {"level2_min", min_l2},
        {"goal_lost_ticks", goal_lost_ticks},
        {"interruptions", interruptions},
        {"tick_calls", t.tick_calls},
        {"production_tick_seconds_excluding_reports", t.tick_seconds},
        {"accepted_recipe_actions", next},
        {"scheduled_recipe_actions", actions.size()},
        {"engine_contract_violations", 0}};
    if (goal) {
        result["stability_tax_delta"] =
            w.taxes_collected_total() - goal_start["taxes_total"].get<std::uint64_t>();
        result["stability_maintenance_delta"] =
            w.maintenance_spent_total() - goal_start["maintenance_total"].get<std::uint64_t>();
        result["stability_net_funds"] = w.treasury() - goal_start["funds"].get<std::int64_t>();
        check(w.construction_spent_total() ==
                      goal_start["construction_total"].get<std::uint64_t>() &&
                  w.command_sequence() == goal_start["command_sequence"].get<std::uint64_t>(),
              "stability received commands or construction");
    }
    result["status"] =
        goal && w.ticks() - *goal >= 10000 && goal_lost_ticks == 0 && min_pop >= 100 && min_l2 >= 8
            ? "PASS"
            : "FAIL_OR_REGULAR_STABILITY_INTERRUPTION";
    t.checkpoint(w, "C-after-goal-stability");
    t.finish(w);
    write(context.root / "summary.json", result);
    write(context.root / "goal-stability.json", result);
    return result;
}
} // namespace

int main(int argc, char **argv) {
    const auto process_start = Clock::now();
    try {
        check(argc == 5 || argc == 6 || argc == 7 || argc==8,
              "usage: fixture DATA FRESH-ROOT --fresh MAP | --analyse SAVE [TICKS] | "
              "--reconstructed SAVE | --recovery SAVE | --resume SAVE TICKS EXPECTED");
        const std::string mode = argv[3];
        check((mode == "--resume" && (argc == 7||argc==8)) || ((mode=="--service-audit"||mode=="--service-ui")&&argc==6) ||
                  (mode=="--service-challenges"&&argc==5) ||
                  (mode == "--analyse" && (argc == 5 || argc == 6)) ||
                  ((mode == "--fresh" || mode == "--reconstructed" || mode == "--recovery" ||
                    mode == "--priority-only"||mode=="--service-prepare"||mode=="--service-capacity") &&
                   argc == 5),
              "invalid mode or argument count");
        const auto data = fs::canonical(argv[1]), root = fs::canonical(argv[2]);
        check(fs::is_empty(root) && !beneath(root, data),
              "fresh isolated root outside original data required");
        check(SDL_SetHintWithPriority(SDL_HINT_VIDEO_DRIVER, "dummy", SDL_HINT_OVERRIDE) &&
                  SDL_Init(SDL_INIT_VIDEO),
              SDL_GetError());
        int code = 0;
        if (mode=="--service-challenges") {
            check(std::string(argv[4])=="authored","challenge evidence must be explicit");service_challenges(root);
        } else if (std::string(argv[3]) == "--fresh") {
            Context context(data, argv[4], root);
            const auto result = fresh(context);
            std::cout << result.dump(2) << '\n';
            code = result["status"] == "PASS" ? 0 : 1;
        } else if(mode=="--service-ui") {
            const auto doc=oe::persistence::read_save(argv[4]);Context context(data,doc.map_relative,root,argv[5]);
            service_ui(context,argv[4]);
        } else if(mode=="--service-audit") {
            const auto doc=oe::persistence::read_save(argv[4]);Context context(data,doc.map_relative,root);
            const auto result=service_audit(context,argv[4],argv[5]);std::cout<<result.dump(2)<<'\n';
        } else if (std::string(argv[3]) == "--analyse" || mode=="--service-prepare" || mode=="--service-capacity" ||
                   std::string(argv[3]) == "--reconstructed" ||
                   std::string(argv[3]) == "--recovery" ||
                   std::string(argv[3]) == "--priority-only") {
            const auto doc = oe::persistence::read_save(argv[4]);
            Context context(data, doc.map_relative, root);
            J result;
            if (std::string(argv[3]) == "--analyse") {
                const auto ticks = argc == 6 ? bounded_ticks(argv[5], 10000, 30000) : 10000;
                result = observe(context, context.restore(argv[4]), ticks,
                                 "read-only supplied SaveDocument replay; origin/evidence "
                                 "classification supplied by the wrapper");
            } else if (std::string(argv[3]) == "--reconstructed")
                result = reconstructed(context, argv[4]);
            else if (std::string(argv[3]) == "--priority-only")
                result = priority_only(context, argv[4]);
            else if(mode=="--service-prepare") result=recovery(context,argv[4],true);
            else if(mode=="--service-capacity") result=service_capacity(context,argv[4]);
            else
                result = recovery(context, argv[4]);
            std::cout << result.dump(2) << '\n';
        } else if (std::string(argv[3]) == "--resume") {
            oe::service_experiment::configure(argc==8?argv[7]:"current");
            const auto d = oe::persistence::read_save(argv[4]);
            Context context(data, d.map_relative, root);
            auto w = context.restore(argv[4]);
            const auto ticks = bounded_ticks(argv[5], 0, 30000);
            for (std::uint64_t i = 0; i < ticks; ++i) {
                w.tick();
                invariants(w);
            }
            const auto expected = oe::persistence::read_save(argv[6]);
            check(w.snapshot() == expected.world, "new process full snapshot differs");
            const auto result = context.document(w);
            oe::persistence::write_save(root / "result.json", result, data,
                                        context.view->buildable_mask());
            write(root / "comparison.json",
                  {{"status", "PASS"},
                   {"complete_snapshot_equal", true},
                   {"map_identity_equal", result.map_sha256 == expected.map_sha256},
                   {"policy_fingerprint_equal",
                    result.map_permissions_sha256 == expected.map_permissions_sha256},
                   {"rule", w.rule_version()},
                   {"source_schema", expected.source_schema_version},
                   {"ticks", ticks}});
        } else
            throw std::runtime_error("unknown fixture mode");
        J metrics = {{"elapsed_wall_seconds",
                      std::chrono::duration<double>(Clock::now() - process_start).count()},
                     {"peak_rss_bytes", nullptr},
                     {"normal_frame_instrumentation", false}};
#if defined(__APPLE__) || defined(__unix__)
        struct rusage usage{};
        if (getrusage(RUSAGE_SELF, &usage) == 0) {
#if defined(__APPLE__)
            metrics["peak_rss_bytes"] = usage.ru_maxrss;
#else
            metrics["peak_rss_bytes"] = std::uint64_t(usage.ru_maxrss) * 1024;
#endif
        }
#endif
        write(root / "performance.json", metrics);
        SDL_Quit();
        return code;
    } catch (const std::exception &e) {
        SDL_Quit();
        std::cerr << "Long-play: " << e.what() << '\n';
        return 1;
    }
}
