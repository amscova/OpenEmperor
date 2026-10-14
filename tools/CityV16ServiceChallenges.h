// Bounded authored transport cases complement (never replace) original-map
// endurance. Starts come only from paid commands; snapshots are never edited.
J service_challenges(const fs::path& root) {
    J results=J::array();
    for(const std::string strategy:{"current","expiry","risk","travel"}) {
        oe::service_experiment::configure(strategy);
        for(bool uneven:{false,true}) {
            constexpr int size=24;
            std::vector<sim::MapCellPermission> cells(size*size,{true,true,false,0,sim::BuildBlocker::None,sim::BuildBlocker::None});
            const auto ix=[](sim::Cell p){return static_cast<std::size_t>(p.y*size+p.x);};
            sim::FixedGatePassage gate;gate.id={101};
            for(int y=7;y<=9;++y)for(int x=5;x<=9;++x)gate.protected_footprint.push_back({x,y});
            for(int y=7;y<=9;++y)gate.corridor.push_back({7,y});gate.openings={sim::Cell{7,6},sim::Cell{7,10}};
            for(const auto p:gate.protected_footprint)cells[ix(p)]={false,false,true,0,sim::BuildBlocker::GateSolidPart,sim::BuildBlocker::OriginalStructure};
            for(const auto p:gate.corridor)cells[ix(p)].road_allowed=true;
            for(int x=0;x<size;++x)if(x<5||x>9)cells[ix({x,8})]={false,false,true,0,sim::BuildBlocker::OriginalStructure,sim::BuildBlocker::OriginalStructure};
            for(int y:{5,6})cells[ix({7,y})]={true,false,false,0,sim::BuildBlocker::None,sim::BuildBlocker::ProtectedOriginalRoad,true,sim::OriginalRoadClassification::GroundRoad};
            if(uneven)cells[ix({7,10})].height=1;
            auto policy=std::make_shared<const sim::MapPermissions>(size,size,2,std::move(cells),std::vector<sim::FixedGatePassage>{gate});
            sim::World w(policy,sim::RulesProfile::CityV16,4);
            const auto command=[&](sim::Command c){const auto r=w.execute(c);check(r.accepted,"challenge paid command: "+r.reason);};
            const auto put=[&](sim::CommandType type,sim::Cell p){command({type,p});return *w.building_owner_at(p);};
            const auto a=put(sim::CommandType::PlaceServicePost,{6,3});
            const auto b=put(sim::CommandType::PlaceServicePost,{8,3});
            const auto near=put(sim::CommandType::PlaceHousehold,{8,12});
            const auto disconnected=put(sim::CommandType::PlaceHousehold,{18,18});
            for(int y=4;y<=14;++y)command({sim::CommandType::PlaceRoad,{7,y}});
            command({sim::CommandType::PlaceRoad,{8,4}});command({sim::CommandType::PlaceRoad,{6,4}});
            check(w.service_route_available(disconnected)==false,"unreachable House admitted");
            check(w.service_route_available(near)==!uneven,"height/gate route contract");
            check(!w.validate({sim::CommandType::RemoveRoad,{7,5}}).accepted,"protected original road removable");
            bool gate_seen=false,original_seen=false,arrival=false,interrupted=false,fire=false;
            unsigned no_route=0,paused_ticks=0,fire_ticks=0;
            auto resumed=sim::World::restore(w.snapshot(),policy);
            const auto tick=[&](){
                w.tick();resumed.tick();invariants(w);invariants(resumed);check(w.snapshot()==resumed.snapshot(),"challenge restore continuation differs");
                for(const auto& c:w.couriers())if(c.role==sim::CourierRole::Service) {
                    check(!c.cargo&&!c.reserved,"challenge Service owns goods");
                    for(const auto p:c.path){gate_seen|=policy->fixed_passage(p);original_seen|=policy->original_road(p);}
                    arrival|=c.phase==sim::CourierPhase::Returning;interrupted|=c.route_pending;
                    const auto status=w.courier_dispatch_status(c.id).status;
                    no_route+=status==sim::CourierDispatchStatus::NoRoad;
                    paused_ticks+=!w.building(c.owner).operating_enabled;
                    fire_ticks+=w.building_on_fire(c.owner);fire|=w.building_on_fire(c.owner);
                }
            };
            const auto both=[&](sim::Command c){command(c);check(resumed.execute(c).accepted,"restored challenge command rejected");};
            for(unsigned n=0;n<20;++n)tick();
            if(!uneven) {
                // Interrupt an unoccupied future player-road edge of an actual
                // outbound trip; the existing state machine must wait/reroute.
                const sim::Command remove{sim::CommandType::RemoveRoad,{7,11}};
                check(w.validate(remove).accepted,"future road deletion not safe");both(remove);
                for(unsigned n=0;n<160;++n)tick();check(interrupted,"road break did not interrupt real Service trip");
                both({sim::CommandType::PlaceRoad,{7,11}});
            }
            both(sim::set_building_operation(a,false));both(sim::set_building_operation(b,false));
            for(unsigned n=0;n<300;++n)tick();
            for(const auto& c:w.couriers())if(c.role==sim::CourierRole::Service)
                check(c.phase==sim::CourierPhase::IdleAtWorkshop,"paused Post did not finish trip");
            both(sim::demolish_building(disconnected));
            both(sim::set_building_workforce_priority(a,sim::WorkforcePriority::High));
            both(sim::set_building_operation(a,true));both(sim::set_building_operation(b,true));
            const sim::Command new_house{sim::CommandType::PlaceHousehold,{10,12}};both(new_house);
            both({sim::CommandType::PlaceRoad,{10,14}});both({sim::CommandType::PlaceRoad,{9,14}});both({sim::CommandType::PlaceRoad,{8,14}});
            // Save/restore equality is checked every tick with the same explicit
            // experimental adapter. No saved fields are normalized or injected.
            resumed=sim::World::restore(w.snapshot(),policy);
            for(unsigned n=0;n<2100;++n)tick();
            check(uneven || (gate_seen&&original_seen&&arrival),"no actual gate/original-road arrival observed");
            check(uneven || fire,"unprotected Post fire was not observed");
            results.push_back({{"strategy",strategy},{"evidence","AUTHORED policy2 transport; real paid commands / real World.tick"},
                {"mixed_height",uneven},{"gate_path_seen",gate_seen},{"original_road_path_seen",original_seen},
                {"actual_arrival",arrival},{"road_interruption_seen",interrupted},{"no_road_status_samples",no_route},
                {"paused_post_tick_samples",paused_ticks},{"fire_post_tick_samples",fire_ticks},
                {"new_house_and_safe_demolition",true},{"unchanged_snapshot_restore_deterministic",true},{"result","PASS"}});
        }
        // Real incomplete workforce allocation, then ordinary priority/pause.
        auto cells=std::vector<sim::MapCellPermission>(24*24,{true,true,false,0,sim::BuildBlocker::None,sim::BuildBlocker::None});
        auto policy=std::make_shared<const sim::MapPermissions>(24,24,2,std::move(cells),std::vector<sim::FixedGatePassage>{});
        sim::World w(policy,sim::RulesProfile::CityV16,4);
        const auto put=[&](sim::CommandType t,sim::Cell p){check(w.execute({t,p}).accepted,"partial staffing paid command");return *w.building_owner_at(p);};
        const auto clay=put(sim::CommandType::PlaceClaySource,{2,2});put(sim::CommandType::PlaceWarehouse,{5,2});
        put(sim::CommandType::PlaceHousehold,{8,2});const auto post=put(sim::CommandType::PlaceServicePost,{11,3});
        check(!w.building_staffed(post),"partial staffing not exercised");
        const auto& c=*std::find_if(w.couriers().begin(),w.couriers().end(),[&](const auto& c){return c.owner==post;});
        check(w.courier_dispatch_status(c.id).status==sim::CourierDispatchStatus::Unstaffed,"unstaffed status");
        check(w.execute(sim::set_building_workforce_priority(post,sim::WorkforcePriority::High)).accepted&&w.building_staffed(post),"priority did not grant existing workers");
        check(w.execute(sim::set_building_operation(clay,false)).accepted,"partial staffing pause");
        for(int x=2;x<=11;++x)check(w.execute({sim::CommandType::PlaceRoad,{x,4}}).accepted,"partial staffing road");
        for(unsigned n=0;n<200;++n){w.tick();invariants(w);}
        check(w.household_service_active(*w.building_owner_at({8,2})),"prioritized Post did not actually arrive");
        results.push_back({{"strategy",strategy},{"case","partial staffing / priority / paused producer"},{"result","PASS"}});
    }
    write(root/"service-challenges.json",results);return results;
}
