// Included by the existing long-play adapter after Context/Trace/paid planning.
// All observation and statistics are external to production frame/tick paths.
struct ServiceAudit {
    struct House {
        unsigned visits=0,covered=0,uncovered_run=0,longest_gap=0,demands=0,service_valid=0,
                 success=0,service_only_misses=0,service_missing_exposures=0;
        std::optional<std::uint64_t> last_arrival,last_service_miss;
        std::vector<std::uint64_t> intervals;
    };
    struct Post {
        unsigned dispatches=0,visits=0,returns=0,idle=0,outbound=0,returning=0,waiting=0,
                 paused=0,unstaffed=0,on_fire=0,reroutes=0,duplicate_dispatches=0,target_changes_while_active=0;
        std::vector<std::uint64_t> lengths,tours;
        J trip;
    };
    std::map<unsigned,House> houses;
    std::map<unsigned,Post> posts;
    std::ofstream visits_log,couriers_log,coverage_log,demand_log,failures_log;
    std::uint64_t ticks=0,start_tick=0,goal_lost=0;
    int min_population=INT_MAX,max_population=0;
    std::optional<std::uint64_t> house8_next_arrival;
    J house8_miss;
    ServiceAudit(const fs::path& root,const sim::World& w):start_tick(w.ticks()) {
        min_population=max_population=w.total_population();
        visits_log.open(root/"service-visits.jsonl");couriers_log.open(root/"service-couriers.jsonl");
        coverage_log.open(root/"house-coverage.jsonl");demand_log.open(root/"demand-results.jsonl");
        failures_log.open(root/"service-failures.jsonl");
        for (const auto& h:w.buildings()) if(h.placed&&h.kind==sim::Object::Household) {
            auto& stats=houses[id(h.id)];
            if(h.service_until_tick>=sim::Rules::service_coverage_ticks)
                stats.last_arrival=h.service_until_tick-sim::Rules::service_coverage_ticks;
        }
        for(const auto& c:w.couriers())if(c.enabled&&c.role==sim::CourierRole::Service&&c.phase!=sim::CourierPhase::IdleAtWorkshop) {
            auto& trip=posts[id(c.owner)].trip;
            trip={{"post",id(c.owner)},{"courier",static_cast<unsigned>(c.id)},{"target",id(c.target)},
                  {"dispatch_tick",nullptr},{"initial_partial_trip",true}};
            if(c.phase==sim::CourierPhase::Returning) {
                const auto until=w.building(c.target).service_until_tick;
                if(until>=1200){trip["last_house_arrival_from_saved_expiry"]=until-1200;
                    trip["arrival_provenance"]="last House visit; initial returning Courier arrival is censored";}
            }
        }
    }
    void observe(const sim::WorldSnapshot& before,const sim::World& w,const J& pre_workers) {
        ++ticks;min_population=std::min(min_population,w.total_population());
        max_population=std::max(max_population,w.total_population());if(!w.settlement_goal_reached())++goal_lost;
        for(const auto& c:w.couriers()) if(c.enabled&&c.role==sim::CourierRole::Service) {
            const auto old=std::find_if(before.couriers.begin(),before.couriers.end(),[&](const auto& a){return a.id==c.id;});
            check(old!=before.couriers.end(),"Service Courier appeared without a command");
            check(c.cargo==0&&c.reserved==0,"Service acquired goods/reservations");
            auto& p=posts[id(c.owner)];
            if(old->phase!=sim::CourierPhase::IdleAtWorkshop&&c.phase!=sim::CourierPhase::IdleAtWorkshop&&old->target!=c.target)
                ++p.target_changes_while_active;
            if(old->phase==sim::CourierPhase::IdleAtWorkshop)++p.idle;
            else if(old->phase==sim::CourierPhase::ToWarehouse)++p.outbound;
            else ++p.returning;
            if(old->route_pending)++p.waiting;
            p.reroutes+=static_cast<unsigned>(c.reroute_attempts-old->reroute_attempts);
            for(const auto& worker:pre_workers)if(worker["id"]==id(c.owner)) {
                if(!worker["running"].get<bool>())++p.paused;
                else if(worker["burning"].get<bool>())++p.on_fire;
                else if(worker["assigned"].get<int>()<worker["required"].get<int>())++p.unstaffed;
            }
            if(old->phase==sim::CourierPhase::IdleAtWorkshop&&c.phase==sim::CourierPhase::ToWarehouse) {
                ++p.dispatches;p.lengths.push_back(c.path.size()-1);
                const bool duplicate=std::any_of(w.couriers().begin(),w.couriers().end(),[&](const auto& other){
                    return other.id!=c.id&&other.role==sim::CourierRole::Service&&other.phase==sim::CourierPhase::ToWarehouse&&other.target==c.target;
                });if(duplicate)++p.duplicate_dispatches;
                p.trip={{"post",id(c.owner)},{"courier",static_cast<unsigned>(c.id)},{"target",id(c.target)},
                    {"dispatch_tick",w.ticks()},{"dispatch_edges",c.path.size()-1},{"duplicate_outbound_target",duplicate},
                    {"cursor_before",old->last_dispatched_target?J(id(*old->last_dispatched_target)):J(nullptr)},
                    {"cursor_after",c.last_dispatched_target?J(id(*c.last_dispatched_target)):J(nullptr)}};
            }
            if(old->phase==sim::CourierPhase::ToWarehouse&&c.phase==sim::CourierPhase::Returning) {
                ++p.visits;auto& h=houses.at(id(old->target));const auto& target=w.building(old->target);
                check(target.service_until_tick==w.ticks()+1200,"arrival coverage contract");
                if(h.last_arrival)h.intervals.push_back(w.ticks()-*h.last_arrival);
                ++h.visits;h.last_arrival=w.ticks();
                if(p.trip.is_null())p.trip={{"post",id(c.owner)},{"courier",static_cast<unsigned>(c.id)},
                    {"target",id(old->target)},{"dispatch_tick",nullptr},{"initial_partial_trip",true}};
                p.trip["arrival_tick"]=w.ticks();p.trip["coverage_until"]=target.service_until_tick;
                p.trip["outbound_ticks"]=p.trip["dispatch_tick"].is_null()?J(nullptr):J(w.ticks()-p.trip["dispatch_tick"].get<std::uint64_t>());
                p.trip["after_actual_service_miss_ticks"]=h.last_service_miss?J(w.ticks()-*h.last_service_miss):J(nullptr);
                line(visits_log,{{"event","arrival"},{"trip",p.trip}});
                if(id(old->target)==8&&house8_miss.is_object()&&!house8_next_arrival)house8_next_arrival=w.ticks();
            }
            if(old->phase==sim::CourierPhase::Returning&&c.phase==sim::CourierPhase::IdleAtWorkshop) {
                ++p.returns;
                if(!p.trip.is_null()) {
                    p.trip["return_tick"]=w.ticks();
                    p.trip["returning_ticks"]=p.trip.contains("arrival_tick")&&!p.trip["arrival_tick"].is_null()?J(w.ticks()-p.trip["arrival_tick"].get<std::uint64_t>()):J(nullptr);
                    if(!p.trip["dispatch_tick"].is_null()) {
                        p.trip["tour_ticks"]=w.ticks()-p.trip["dispatch_tick"].get<std::uint64_t>();
                        p.tours.push_back(p.trip["tour_ticks"].get<std::uint64_t>());
                    }
                    line(visits_log,{{"event","return"},{"trip",p.trip}});p.trip=J();
                }
            }
            if(old->phase!=c.phase||old->route_pending!=c.route_pending||old->reroute_attempts!=c.reroute_attempts||w.ticks()%400==0)
                line(couriers_log,{{"tick",w.ticks()},{"post",id(c.owner)},{"courier",static_cast<unsigned>(c.id)},
                    {"target",id(c.target)},{"phase",sim::courier_phase_name(c.phase)},{"path_vertices",c.path.size()},
                    {"path_vertex",c.path_vertex},{"edge_progress",c.edge_progress},{"route_pending",c.route_pending},
                    {"route_revision",w.road_revision()},{"reroute_attempts",c.reroute_attempts},
                    {"last_dispatched_target",c.last_dispatched_target?J(id(*c.last_dispatched_target)):J(nullptr)}});
        }
        for(const auto& b:w.buildings())if(b.placed&&b.kind==sim::Object::Household) {
            const auto old=std::find_if(before.buildings.begin(),before.buildings.end(),[&](const auto& h){return h.id==b.id;});
            auto& h=houses.at(id(b.id));const auto coverage=oe::sandbox_ui::service_coverage(w.ticks(),b.service_until_tick,b.demand_progress);
            if(coverage.active){++h.covered;h.longest_gap=std::max(h.longest_gap,h.uncovered_run);h.uncovered_run=0;}
            else {++h.uncovered_run;h.longest_gap=std::max(h.longest_gap,h.uncovered_run);}
            if(b.fulfilled_demand!=old->fulfilled_demand||b.missed_demand!=old->missed_demand) {
                ++h.demands;const bool service=old->service_until_tick>w.ticks();if(service)++h.service_valid;
                const bool success=b.fulfilled_demand>old->fulfilled_demand;if(success)++h.success;
                const bool only=!success&&!service&&old->food_stock>0&&old->pottery_stock>0&&
                    old->sick_until_tick<=w.ticks()&&old->fire_until_tick<=w.ticks();
                if(!service){++h.service_missing_exposures;h.last_service_miss=w.ticks();}
                if(only)++h.service_only_misses;
                J event={{"tick",w.ticks()},{"house",id(b.id)},{"input_service_until",old->service_until_tick},
                    {"service_valid_at_actual_deadline",service},{"food",old->food_stock},{"pottery",old->pottery_stock},
                    {"sick",old->sick_until_tick>w.ticks()},{"burning",old->fire_until_tick>w.ticks()},
                    {"actual_success",success},{"service_only_miss",only},{"population_before",old->population},
                    {"population_after",b.population},{"tax_delta",b.taxes_paid_total-old->taxes_paid_total}};
                line(demand_log,event);if(!service)line(failures_log,event);
                if(id(b.id)==8&&w.ticks()==31600)house8_miss=event;
            }
            if(w.ticks()%100==0)line(coverage_log,{{"tick",w.ticks()},{"phase","after arrivals/current coverage"},
                {"house",id(b.id)},{"origin",{b.cell.x,b.cell.y}},{"placement_tick",b.placed_tick},
                {"population",b.population},{"effective_level",w.household_level(b.id)},
                {"service_until",b.service_until_tick},{"remaining",coverage.remaining},
                {"next_demand",coverage.next_demand?J(*coverage.next_demand):J(nullptr)},
                {"coverage_guarantees_next_demand_without_revisit",coverage.valid_at_next_demand},
                {"visits_in_window",h.visits},{"last_arrival",h.last_arrival?J(*h.last_arrival):J(nullptr)}});
        }
    }
    static J distribution(std::vector<std::uint64_t> values) {
        if(values.empty())return {{"count",0},{"mean",nullptr},{"p95",nullptr},{"max",nullptr}};
        std::sort(values.begin(),values.end());const auto sum=std::accumulate(values.begin(),values.end(),std::uint64_t{0});
        return {{"count",values.size()},{"mean",static_cast<double>(sum)/values.size()},
            {"p95",values[(95*values.size()+99)/100-1]},{"max",values.back()}};
    }
    J finish(const sim::World& w) {
        J hs=J::array(),ps=J::array();std::vector<std::uint64_t> intervals,distances,tours;
        unsigned deadlines=0,service_valid=0,success=0,misses=0,covered=0,never=0,longest=0,travel=0,duplicate=0,no_route=0,unknown_route=0;
        for(const auto& [bid,h]:houses) {
            const auto id_value=static_cast<sim::BuildingId>(bid);const auto route=w.service_route_available(id_value);
            J row=house(w,w.building(id_value),false);row["visits_in_window"]=h.visits;
            row["last_arrival"]=h.last_arrival?J(*h.last_arrival):J(nullptr);
            row["intervisit_intervals"]=distribution(h.intervals);row["covered_ticks"]=h.covered;
            row["uncovered_ticks"]=ticks-h.covered;row["actual_demand_deadlines"]=h.demands;
            row["actual_demand_misses"]=h.demands-h.success;
            row["previous_intervisit_interval"]=h.intervals.empty()?J(nullptr):J(h.intervals.back());
            const auto coverage=oe::sandbox_ui::service_coverage(w.ticks(),w.building(id_value).service_until_tick,w.building(id_value).demand_progress);
            row["next_actual_demand_tick"]=coverage.next_demand?J(*coverage.next_demand):J(nullptr);
            row["coverage_guarantees_next_deadline_without_revisit"]=coverage.valid_at_next_demand;
            row["longest_uncovered_in_window"]=h.longest_gap;row["service_only_misses"]=h.service_only_misses;
            row["service_missing_deadline_exposures"]=h.service_missing_exposures;
            row["known_service_route"]=route?J(*route):J(nullptr);row["actual_demand_successes"]=h.success;
            row["never_visited_in_window"]=h.visits==0;
            no_route+=route&&!*route;unknown_route+=!route;
            if(!h.visits) {
                // This fixed-window run has no commands/topology changes.
                // State limitations are reported separately from rotation,
                // never inferred from a guessed arrival time.
                const bool paused=std::all_of(posts.begin(),posts.end(),[&](const auto& p){return p.second.paused==ticks;});
                const bool unstaffed=std::all_of(posts.begin(),posts.end(),[&](const auto& p){return p.second.unstaffed==ticks;});
                row["never_visited_diagnosis"]=!route?"UNKNOWN_ROUTE_CACHE":!*route?"NO_ROUTE":
                    paused?"POST_PAUSED":unstaffed?"NO_STAFF":"ROTATION_FAIRNESS_WINDOW / inspect phase trace";
            }
            hs.push_back(row);intervals.insert(intervals.end(),h.intervals.begin(),h.intervals.end());
            deadlines+=h.demands;service_valid+=h.service_valid;success+=h.success;misses+=h.service_only_misses;
            covered+=h.covered;never+=h.visits==0;longest=std::max(longest,h.longest_gap);
        }
        for(const auto& [bid,p]:posts) {
            ps.push_back({{"post",bid},{"dispatches",p.dispatches},{"completed_visits",p.visits},{"returns",p.returns},
                {"phase_exposure_pre_tick",{{"idle",p.idle},{"outbound",p.outbound},{"returning",p.returning}}},
                {"route_pending_ticks",p.waiting},{"paused_ticks",p.paused},{"running_unstaffed_ticks",p.unstaffed},
                {"on_fire_ticks",p.on_fire},{"reroutes",p.reroutes},{"duplicate_outbound_dispatches",p.duplicate_dispatches},
                {"target_changes_while_active",p.target_changes_while_active},
                {"dispatch_edges",distribution(p.lengths)},{"complete_tour_ticks",distribution(p.tours)},
                {"utilization",static_cast<double>(p.outbound+p.returning)/ticks},{"unfinished_trip",p.trip},
                {"workers_assigned_final",w.workers_assigned(static_cast<sim::BuildingId>(bid))},
                {"operation_enabled_final",w.building(static_cast<sim::BuildingId>(bid)).operating_enabled},
                {"priority_final",sim::workforce_priority_name(w.building(static_cast<sim::BuildingId>(bid)).workforce_priority)}});
            distances.insert(distances.end(),p.lengths.begin(),p.lengths.end());tours.insert(tours.end(),p.tours.begin(),p.tours.end());
            travel+=p.outbound+p.returning;duplicate+=p.duplicate_dispatches;
        }
        return {{"households",hs},{"posts",ps},{"ticks",ticks},{"actual_demand_deadlines",deadlines},
            {"service_valid_at_deadline_count",service_valid},{"service_deadline_valid_ratio",static_cast<double>(service_valid)/deadlines},
            {"all_demand_successes",success},{"all_demand_success_ratio",static_cast<double>(success)/deadlines},
            {"service_only_misses",misses},{"covered_house_tick_ratio",static_cast<double>(covered)/(ticks*houses.size())},
            {"longest_uncovered_in_window",longest},{"houses_never_visited_in_window",never},
            {"unreachable_households_final",no_route},{"unknown_service_route_households_final",unknown_route},
            {"revisit_intervals_including_first_saved_expiry_interval",distribution(intervals)},
            {"dispatch_edges",distribution(distances)},{"complete_tour_ticks",distribution(tours)},
            {"travel_phase_exposure_ticks",travel},{"duplicate_outbound_dispatches",duplicate},
            {"population_min",min_population},{"population_max",max_population},{"goal_lost_ticks",goal_lost},
            {"house8_tick31600",house8_miss},{"house8_next_actual_arrival",house8_next_arrival?J(*house8_next_arrival):J(nullptr)}};
    }
};

J service_audit(Context& context,const fs::path& source,const std::string& strategy) {
    auto w=context.restore(source);oe::service_experiment::configure(strategy);
    Trace trace(context,context.root);trace.begin(w);ServiceAudit audit(context.root,w);
    trace.observer=[&](const auto& before,const auto& now,const auto& workers){audit.observe(before,now,workers);};
    const auto start=finances(w);
    write(context.root/"baseline.json",{{"source_save",source.string()},{"map",context.map.generic_string()},
        {"rule",w.rule_version()},{"policy",w.map_permissions()->policy_version()},{"schema",20},
        {"strategy",strategy},{"experimental",strategy!="current"},{"start",start},
        {"global_legacy_cursor",w.last_dispatched_service_household()?J(id(*w.last_dispatched_service_household())):J(nullptr)},
        {"service_cursors_are_per_courier_for_scalable_profiles",true},{"households",households(w)},
        {"couriers",couriers(w)}});
    std::set<std::string> checkpoints;
    for(unsigned i=0;i<10000;++i) {
        trace.tick(w);
        const auto capture=[&](const std::string& tag){if(checkpoints.insert(tag).second)trace.checkpoint(w,tag);};
        if(strategy=="current") {
            for(const auto& c:w.couriers())if(c.role==sim::CourierRole::Service&&c.edge_progress>0) {
                if(c.phase==sim::CourierPhase::ToWarehouse)capture("service-outbound");
                if(c.phase==sim::CourierPhase::Returning)capture("service-returning");
            }
            for(const auto& h:w.buildings())if(h.kind==sim::Object::Household) {
                if(h.service_until_tick>w.ticks()&&h.service_until_tick-w.ticks()==1)capture("coverage-before-expiry");
                if(h.demand_progress==399)capture("before-demand");
            }
            if(w.ticks()==31600&&audit.house8_miss.is_object()&&audit.house8_miss["service_only_miss"].get<bool>())capture("after-house8-miss");
        }
    }
    check(w.command_sequence()==start["command_sequence"].get<std::uint64_t>()&&
        w.construction_spent_total()==start["construction_total"].get<std::uint64_t>(),"Service endurance received commands");
    trace.checkpoint(w,"service-end");trace.finish(w);
    auto result=audit.finish(w);result["technical_result"]="PASS";result["strategy"]=strategy;
    result["experimental"]=strategy!="current";result["start"]=start;result["final"]=finances(w);
    result["delta_tax"]=w.taxes_collected_total()-start["taxes_total"].get<std::uint64_t>();
    result["delta_upkeep"]=w.maintenance_spent_total()-start["maintenance_total"].get<std::uint64_t>();
    result["net_funds"]=w.treasury()-start["funds"].get<std::int64_t>();
    result["world_tick_seconds_excluding_reports"]=trace.tick_seconds;
    write(context.root/"endurance-summary.json",result);write(context.root/"summary.json",result);
    return result;
}

J service_capacity(Context& context,const fs::path& source) {
    oe::service_experiment::configure("current");
    const auto base=context.restore(source);auto planned=base;
    check(std::count_if(base.buildings().begin(),base.buildings().end(),[](const auto& h){return h.kind==sim::Object::Household;})==10,
          "capacity branch requires the earned10-House goal save");
    std::vector<sim::Command> plan;
    for(unsigned n=0;n<10;++n)for(const auto command:connected_purchase(planned,sim::CommandType::PlaceHousehold,true)) {
        check(planned.execute(command).accepted,"full20-House paid layout plan rejected");plan.push_back(command);
    }
    J scenarios=J::array();std::optional<std::vector<sim::Cell>> common_roads;
    for(unsigned count:{10U,14U,18U,20U}) {
        auto w=base;const auto dir=context.root/("houses-"+std::to_string(count));Trace trace(context,dir);trace.begin(w);
        unsigned homes=10;
        for(const auto command:plan) {
            if(command.type==sim::CommandType::PlaceHousehold) {
                if(homes>=count)continue;
                ++homes;
            }
            trace.command(w,command,"paid common20-House road template / selected household count");
        }
        const auto roads=w.snapshot().roads;
        if(common_roads)check(roads==*common_roads,"capacity road-cell topology differs");else common_roads=roads;
        for(unsigned n=0;n<4000;++n)trace.tick(w);
        trace.checkpoint(w,"capacity-start");trace.finish(w);
        scenarios.push_back({{"houses",count},{"start",finances(w)},{"save",(dir/"capacity-start.json").string()},
            {"road_cells_identical_between_capacity_branches",true},
            {"layout_history_differs_from_the31200_recovery_baseline",true}});
    }
    write(context.root/"summary.json",scenarios);return scenarios;
}

// Scripted SDL events through the real view, separately labelled from native
// desktop delivery and human playtesting. Pixel reads are test-only.
void service_ui(Context& context,const fs::path& source) {
    namespace perf=oe::performance;
    auto& view=*context.view;const auto doc=oe::persistence::read_save(source);
    oe::persistence::write_save(view.save_path(),doc,context.data,view.buildable_mask());
    bool running=true;
    const auto key=[&](SDL_Keycode k){SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=k;view.handle_event(e,running);};
    key(SDLK_F9);check(view.world().snapshot()==doc.world&&view.paused(),"UI F9 restore changed World");
    const auto snapshot=view.world().snapshot();const auto textures=view.building_texture_count();
    const auto live=[](){return oe::StoredGraphicsRenderer::live_texture_count()+oe::WalkerSpriteSet::live_texture_count()+
        oe::BuildingSprite::live_texture_count()+oe::RoadSpriteSet::live_texture_count()+oe::FireSpriteSet::live_texture_count();};
    const auto live_textures=live();
    const auto focus=[&](double zoom) {
        auto map=view.layout().map;const double cx=map.x+map.w*.5,cy=map.y+map.h*.5;
        SDL_Event wheel{};wheel.type=SDL_EVENT_MOUSE_WHEEL;wheel.wheel.mouse_x=static_cast<float>(cx);wheel.wheel.mouse_y=static_cast<float>(cy);
        wheel.wheel.y=static_cast<float>(std::log(zoom/view.camera().zoom)/std::log(1.15));view.handle_event(wheel,running);
        // Actual input update seam, the same camera pan as held arrow state.
        // Centre the paid district; no production camera offsets are altered.
        for(unsigned n=0;n<300;++n) {
            const auto p=view.camera().world_to_screen(oe::maps::terrain_ground({113,117},context.border));
            const double dx=cx-p.x,dy=cy-p.y;if(std::abs(dx)+std::abs(dy)<1)break;
            oe::SandboxView::PanKeyState held;held.left=dx>1;held.right=dx<-1;held.up=dy>1;held.down=dy<-1;
            view.update(std::min(.02,std::max(std::abs(dx),std::abs(dy))/400),held);
        }
    };
    focus(1);
    const auto capture=[&](const char* name){check(view.render(),"Service UI render failed");
        auto* pixels=SDL_RenderReadPixels(context.renderer,nullptr);check(pixels,SDL_GetError());
        check(SDL_SaveBMP(pixels,(context.root/(std::string(name)+".bmp")).c_str()),SDL_GetError());
        SDL_DestroySurface(pixels);};
    capture("overlay-off");key(SDLK_V);check(view.service_overlay(),"V did not toggle Service");capture("overlay-on");
    J states=J::array();for(const auto& h:view.world().buildings())if(h.kind==sim::Object::Household) {
        const auto c=oe::sandbox_ui::service_coverage(view.world().ticks(),h.service_until_tick,h.demand_progress);
        states.push_back({{"house",id(h.id)},{"state",static_cast<unsigned>(c.state)},{"remaining",c.remaining}});
    }
    key(SDLK_H);key(SDLK_V);check(view.service_overlay(),"Help leaked V");key(SDLK_H);
    key(SDLK_F3);key(SDLK_V);check(view.service_overlay(),"Walker diagnostic leaked V");key(SDLK_F3);
    key(SDLK_V);check(!view.service_overlay(),"V did not switch OFF");capture("overlay-off-restored");
    key(SDLK_F5);check(oe::persistence::read_save(view.save_path()).world==snapshot,"overlay changed F5 state");
    // Warm renderer, then compare paused frame work without capture/file work.
    J timings=J::object();
    for(bool overlay:{false,true}) {
        if(view.service_overlay()!=overlay)key(SDLK_V);
        perf::set_enabled(true);perf::reset();std::vector<std::uint64_t> durations;
        for(unsigned frame=0;frame<100;++frame) {
            const auto start=Clock::now();check(view.render(),"paused Service frame failed");
            durations.push_back(static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now()-start).count()));
        }
        J counters=J::object();for(std::size_t i=0;i<perf::counter_names.size();++i) {
            const auto n=perf::counter(static_cast<perf::Counter>(i));counters[perf::counter_names[i]]=n;
            check(n==0,"Service renderer performed expensive work");
        }
        timings[overlay?"on":"off"]={{"nanoseconds",ServiceAudit::distribution(durations)},{"counters",counters}};
        perf::set_enabled(false);
    }
    perf::set_enabled(true);perf::reset();
    for(unsigned n=0;n<100;++n){key(SDLK_V);check(view.render(),"100 Service toggle frame failed");}
    J toggle=J::object();for(std::size_t i=0;i<perf::counter_names.size();++i) {
        auto n=perf::counter(static_cast<perf::Counter>(i));toggle[perf::counter_names[i]]=n;check(n==0,"Service toggle expensive work");
    }
    perf::set_enabled(false);check(view.world().snapshot()==snapshot&&view.building_texture_count()==textures&&live()==live_textures,"Service stress changed World/textures");
    for(const double zoom:{1.,2.,4.}){focus(zoom);capture(("zoom-"+std::to_string(static_cast<int>(zoom))).c_str());}
    key(SDLK_U);key(SDLK_K);key(SDLK_D);capture("combined-overlays");key(SDLK_TAB);capture("panel-closed");
    check(SDL_SetWindowSize(context.window,1450,900),SDL_GetError());SDL_PumpEvents();
    SDL_Event resized{};resized.type=SDL_EVENT_WINDOW_RESIZED;view.handle_event(resized,running);capture("resized");
    check(view.world().snapshot()==snapshot,"Service UI camera/modal combination changed World");
    write(context.root/"ui-overlay-acceptance.json",{{"technical_result","PASS"},
        {"evidence","SCRIPTED SDL / SOFTWARE DIRECT PIXELS; not native input or human acceptance"},
        {"source_save",source.string()},{"map",context.map.string()},{"rule",4},{"policy",2},{"schema",20},
        {"tick",view.world().ticks()},{"households",states},{"building_textures",textures},{"live_textures_before",live_textures},{"live_textures_after",live()},
        {"frame_timings",timings},
        {"100_toggle_counters",toggle},{"full_world_unchanged",true},{"F5_F9_world_equal",true},
        {"human_acceptance","OPEN"}});
}
