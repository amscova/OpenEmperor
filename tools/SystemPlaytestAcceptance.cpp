// OE41 QA only. Reuse the shipped event/dialog harness; all gameplay enters
// MenuSession and SandboxView through ordinary SDL events.
#define main oe41_reused_original_menu_acceptance_main
#include "OriginalStarterMenuAcceptance.cpp"
#undef main
#include <map>
#include <regex>
#include <set>

namespace oe41 {
using A=oe::sandbox_ui::Action;
constexpr const char* help="openemperor-system-playtest-acceptance --inventory|--create|--resume|--map|--inspect-paid DATA RESOURCES FRESH_APP_ROOT FRESH_OUTPUT [SAVE_OR_MAP]";
const fs::path repo=fs::absolute(__FILE__).parent_path().parent_path();
const fs::path private_root=repo/".local/oe41";
Json menu_rows=Json::array(),toolbar_rows=Json::array(),events=Json::array(),scenario_results=Json::array();
std::map<int,std::string> action_names;

void paths(const fs::path& data,const fs::path& resources,const fs::path& app,const fs::path& out) {
    for(const auto& target:{app,out}) {
        no_symlinks(target);const auto p=fs::weakly_canonical(target);
        require(contains(private_root,p)&&p!=private_root&&!fs::exists(p),"Use fresh ignored OE41 app/output directories");
        for(const auto& source:{fs::canonical(data),fs::canonical(resources)})
            require(!contains(p,source)&&!contains(source,p),"OE41 outputs overlap read-only inputs");
    }
    require(!contains(app,out)&&!contains(out,app),"App root and evidence output must be disjoint");
}
void names() {
    const auto source=bytes(repo/"src/app/SandboxUiLayout.h");
    const auto begin=source.find("enum class Action {");const auto end=source.find("};",begin);
    require(begin!=std::string::npos&&end!=std::string::npos,"Actual toolbar registration missing");
    const auto body=source.substr(begin+19,end-begin-19);const std::regex word("[A-Za-z][A-Za-z0-9_]*");
    int value=0;for(auto i=std::sregex_iterator(body.begin(),body.end(),word);i!=std::sregex_iterator();++i)action_names.emplace(value++,i->str());
}
std::vector<std::string> menu_labels() {
    // Candidate labels come from current registration, then actual diagnostics
    // confirm visibility, enabled state and action identity. Dynamic rows below
    // are independently discovered by bounded hit sampling.
    const auto source=bytes(repo/"src/app/MenuSession.cpp");const std::regex literal("\"([^\"\\n]*)\"");
    std::set<std::string> labels;for(auto i=std::sregex_iterator(source.begin(),source.end(),literal);i!=std::sregex_iterator();++i)labels.insert((*i)[1]);
    for(const auto* d:{"Very Easy","Easy","Normal","Hard","Very Hard"})labels.insert(std::string("D Difficulty: ")+d);
    for(const auto* s:{"Automatic","Off","Earthen","Stone","Brick"})labels.insert(std::string("G Great Wall: ")+s);
    return {labels.begin(),labels.end()};
}
struct Journey:Harness {
    using Harness::Harness;
    void reset_menu(const fs::path& explicit_data,const fs::path& root) {
        menu->shutdown();menu.reset();auto adapter=std::make_unique<FileDialog>();dialog=adapter.get();
        menu=std::make_unique<Menu>(explicit_data,root,std::move(adapter),resources);menu->initialize(window,renderer);
    }
    Json state() {
        if(menu->state()!=Menu::State::Playing)return {{"menu_state",static_cast<unsigned>(menu->state())},{"message",menu->message()}};
        const auto& w=view().world();const auto doc=view().capture_save_document();
        return {{"profile",sim::rules_profile_name(w.profile())},{"rule",w.rule_version()},
            {"schema_read",doc.source_schema_version?Json(doc.source_schema_version):Json(nullptr)},
            {"policy",doc.map_permissions_policy_version},{"tick",w.ticks()},{"funds",w.treasury()},
            {"construction",w.construction_spent_total()},{"tax",w.taxes_collected_total()},{"upkeep",w.maintenance_spent_total()},
            {"population",w.total_population()},{"buildings",w.buildings().size()},{"couriers",w.couriers().size()},
            {"command_sequence",w.command_sequence()},{"paused",view().paused()},{"tool",view().tool()},
            {"map",doc.map_relative.generic_string()},{"map_sha256",doc.map_sha256},{"hud",view().hud_status_line()}};
    }
    void note(const std::string& id,const std::string& observation) {
        events.push_back({{"id",id},{"observation",observation},{"state",state()},{"status","PASS"}});
    }
    void menu_click(const std::string& label) {
        const auto before=menu->state();Harness::menu_click(label);
        events.push_back({{"id","menu."+label},{"label",label},{"before_state",static_cast<unsigned>(before)},
            {"after_state",static_cast<unsigned>(menu->state())},{"status","PASS"},{"scope","actual mouse activation"}});
        for(auto& row:menu_rows)if(row["label"]==label&&row["menu_state"]==static_cast<unsigned>(before)) {
            row["executed"]=true;row["result"]="PASS";
        }
    }
    void capture(const std::string& name) {
        require(menu->render(),"Actual Menu/SandboxView render failed");
        std::unique_ptr<SDL_Surface,decltype(&SDL_DestroySurface)> image(SDL_RenderReadPixels(renderer,nullptr),SDL_DestroySurface);
        const auto file=out/(name+".png");require(image&&!fs::exists(file)&&SDL_SavePNG(image.get(),file.string().c_str()),"Cannot capture actual frame");
        captures.push_back({{"file",file.filename().string()},{"width",image->w},{"height",image->h},{"state",state()},
            {"visually_inspected",false},{"runtime","scripted SDL dummy/software"}});
        if(menu->state()==Menu::State::Playing){const auto road=view().road_display_stats();const auto buildings=view().building_display_stats();const auto houses=view().original_house_display_stats();
            captures.back()["render_counters"]={{"original_road_ground_items",view().original_road_ground_items()},
                {"road_draws",road.draws},{"road_fallback_draws",road.fallback_draws},{"road_texture_uploads",road.texture_uploads},
                {"building_draws",buildings.drawn_instances},{"building_fallbacks",buildings.placeholder_fallbacks},
                {"house_draws",houses.drawn_instances},{"house_fallbacks",houses.authored_fallbacks},
                {"walker_draws",view().last_courier_draws()},{"building_textures",view().building_texture_count()},{"walker_textures",view().walker_texture_count()}};}
    }
    void menu_inventory(const std::string& context) {
        require(menu->render(),"Menu registration render failed");
        std::map<int,Json> found;
        for(const auto& label:menu_labels())if(const auto r=menu->diagnostic_button_rect(label)) {
            const double scale=1.5*std::max(1.0F,SDL_GetWindowPixelDensity(window));
            const auto p=window_point({(r->x+r->w*.5)*scale,(r->y+r->h*.5)*scale});
            if(const auto hit=menu->diagnostic_button_hit(float(p.x),float(p.y)))found[hit->action]=
                {{"action",hit->action},{"label",label},{"enabled",hit->enabled},{"label_provenance","registration candidate confirmed by actual diagnostic"}};
        }
        int width=0,height=0;require(SDL_GetWindowSize(window,&width,&height),SDL_GetError());
        for(int y=4;y<height;y+=8)for(int x=4;x<width;x+=8)if(const auto hit=menu->diagnostic_button_hit(float(x),float(y)))
            if(!found.contains(hit->action))found[hit->action]={{"action",hit->action},{"label","dynamic row or computed label"},
                {"enabled",hit->enabled},{"label_provenance","actual bounded hit sampling; label unavailable through public API"}};
        for(auto& [id,row]:found){(void)id;row["context"]=context;row["menu_state"]=static_cast<unsigned>(menu->state());
            row["available"]=true;row["visible"]=true;row["executed"]=false;row["result"]="NOT_RUN";menu_rows.push_back(row);}
        std::ofstream progress(out/"inventory-progress.json");progress<<Json{{"menu_controls",menu_rows},{"toolbar_controls",toolbar_rows},{"events",events}}.dump(2)<<'\n';
    }
    void wait_map() {
        const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(60);
        while(menu->map_rules_result().status==oe::maps::MapRulesStatus::Checking&&std::chrono::steady_clock::now()<deadline){menu->advance();std::this_thread::yield();}
        require(menu->map_rules_result().status!=oe::maps::MapRulesStatus::Checking,"Actual map preflight deadline");
    }
    void choose_map(const std::string& name) {
        const auto catalog=oe::menu::validate_data_root(data);const fs::path target="Cities/"+name+".map";
        const auto wanted=std::find_if(catalog.entries.begin(),catalog.entries.end(),[&](const auto& e){return e.relative_path==target;});
        require(wanted!=catalog.entries.end(),"Requested original map absent: "+target.string());
        for(std::size_t i=0;i<catalog.entries.size();++i)key(SDLK_UP);
        for(auto i=catalog.entries.begin();i!=wanted;++i)key(SDLK_DOWN);
        wait_map();require(menu->map_rules_result().request.map_relative==target,"Actual menu selected another map");
        const oe::maps::MapGeometry geometry(*wanted->declared_size);require(geometry.supported,"Map geometry unsupported");border=geometry.border;
    }
    bool start() {
        wait_map();if(!menu->map_rules_can_start()){note("map.preflight","Actual selected map refused: "+menu->map_rules_result().reason);return false;}
        menu_click("Start sandbox");
        if(menu->state()==Menu::State::ConfirmLeave){menu_inventory("confirm-replace");menu_click("Discard Changes");}
        require(menu->state()==Menu::State::Loading,"Start did not defer actual load");
        require(menu->render(),"Actual Loading render failed");menu->advance();
        if(menu->state()==Menu::State::ConfirmLeave){menu_inventory("confirm-prepared-replace");menu_click("Discard Changes");}
        (void)view();focus();
        if(view().world().original_production_enabled())require(view().paused(),"New original production view must be paused");
        else paused(true);
        return true;
    }
    void back() {if(menu->state()==Menu::State::Playing)key(SDLK_ESCAPE);if(menu->state()!=Menu::State::MainMenu)key(SDLK_ESCAPE);require(menu->state()==Menu::State::MainMenu,"Cannot return to actual MainMenu");}
    void toolbar(const std::string& context,bool exercise) {
        require(menu->render(),"Toolbar render failed");const auto buttons=view().layout().buttons;
        for(const auto& button:buttons){const int id=static_cast<int>(button.action);const auto name=action_names.at(id);
            Json row{{"action",id},{"name",name},{"context",context},{"identity",state()},{"available",true},{"visible",true},
                {"layout_enabled",button.enabled},{"enabled",nullptr},{"executed",false},{"result","NOT_RUN"},
                {"source","src/app/SandboxUiLayout.cpp; src/app/SandboxView.cpp::action_enabled"}};
            // Exercise every construction selector without granting a command.
            const bool selector=button.action<=A::Market||button.action==A::FireWatch||button.action==A::Well||button.action==A::HealthPost||button.action==A::Bridge||button.action==A::Ramp||
                (button.action>=A::OriginalHempFarm&&button.action!=A::ToggleIndustryPage);
            if(exercise&&selector){ui(A::Select);const auto before=view().world().snapshot();const auto old=view().tool();
                ui(button.action);require(view().world().snapshot()==before,"Tool selection mutated World");
                row["executed"]=true;row["tool_before"]=old;row["tool_after"]=view().tool();row["message"]=view().last_message();
                row["enabled"]=view().tool()!=old||button.action==A::Select;row["result"]="PASS";row["scope"]="selector event only; purchase separately reported";}
            toolbar_rows.push_back(row);
        }ui(A::Select);
    }
    void zoom_images(const std::string& prefix) {
        const auto map=view().layout().map;Point anchor{map.x+map.w*.5,map.y+map.h*.5};
        for(const auto& b:view().world().buildings())if(b.placed&&sim::original_house_kind(b.kind))if(const auto p=ground(b.cell)){anchor=*p;break;}
        for(int z:{1,2,4}){zoom(z,anchor);const auto before=view().world().snapshot();
            for(int i=0;i<3;++i){SDL_Event e{};e.type=SDL_EVENT_MOUSE_MOTION;const auto raw=window_point(anchor);e.motion.x=float(raw.x);e.motion.y=float(raw.y);event(e);menu->update(0);require(menu->render(),"Purity frame failed");}
            require(view().world().snapshot()==before,"Hover/render changed World");capture(prefix+"-"+std::to_string(z)+"x");}
        zoom(1,anchor);note("B03.B17.purity","Actual hover/update(0)/render at 1/2/4x retained complete World snapshot");
    }
    void center_world(Point world) {
        const auto map=view().layout().map;const Point middle{map.x+map.w*.5,map.y+map.h*.5};zoom(1,middle);
        // Two normal wheel gestures at different anchors translate the camera.
        // No camera setter or injected held-device state is used.
        for(int n=0;n<48;++n){const auto p=view().camera().world_to_screen(world);const double dx=middle.x-p.x,dy=middle.y-p.y;
            if(std::abs(dx)<.01&&std::abs(dy)<.01)return;
            const double x=std::clamp(dx,-map.w*.3,map.w*.3),y=std::clamp(dy,-map.h*.3,map.h*.3);
            zoom(2,{middle.x-x,middle.y-y});zoom(1,{middle.x+x,middle.y+y});}
        throw std::runtime_error("Bounded actual wheel centering did not converge");
    }
    void center_cell(sim::Cell cell) {
        const auto& w=view().world();const double h=w.map_permissions()?w.map_permissions()->cell_height(cell):0;
        center_world({(cell.x-cell.y)*40.0,(cell.x+cell.y-2.0*border)*20+40-h*40});
    }
    bool select_body(sim::BuildingId id) {
        ui(A::Select);if(view().debug_diagnostics())key(SDLK_F1);const auto& b=view().world().building(id);center_cell(b.cell);
        for(const auto cell:sim::building_footprint_cells(view().world().profile(),view().world().rule_version(),b.kind,b.cell))
            if(const auto p=ground(cell)){click(*p);if(view().selected_building()==id)return true;}
        return false;
    }
    void operations() {
        for(const auto& body:view().world().buildings())if(body.placed&&body.kind==sim::Object::Market&&select_body(body.id)) {
            const auto rect=view().diagnostic_operation_toggle_rect();const auto before=view().world().snapshot();
            click({rect.x+rect.w*.5,rect.y+rect.h*.5});require(!view().world().building(body.id).operating_enabled,"Actual operation pause failed");
            require(view().world().maintenance_spent_total()==before.maintenance_spent_total&&view().world().workforce_required(body.id)==sim::Rules::market_workers,"Pause waived installed workforce/upkeep authority");
            click({rect.x+rect.w*.5,rect.y+rect.h*.5});require(view().world().building(body.id).operating_enabled,"Actual operation resume failed");
            const auto panel=view().layout().panel;const int scale=view().layout().scale;const int width=(rect.w-8*scale)/3;
            for(int index:{0,2,1})click({panel.x+10*scale+index*(width+4*scale)+width*.5,double(rect.y+44*scale+14*scale)});
            require(view().world().building(body.id).workforce_priority==sim::WorkforcePriority::Normal,"Actual priority controls failed");
            events.push_back({{"id","B08.operation"},{"status","PASS"},{"owner",static_cast<unsigned>(body.id)},{"scope","actual inspector pause/resume and High/Low/Normal; installed worker requirement retained"},{"state",state()}});return;
        }
        events.push_back({{"id","B08.operation"},{"status","NOT_RUN"},{"reason","No paid Market selectable through current UI"}});
    }
    void protected_refusal() {
        const auto& w=view().world();if(!w.map_permissions())return;
        for(int y=0;y<w.height();++y)for(int x=0;x<w.width();++x){const sim::Cell cell{x,y};
            if(!w.map_permissions()->protected_original(cell))continue;const auto decision=w.validate({sim::CommandType::PlaceRoad,cell});
            if(decision.accepted)continue;center_cell(cell);ui(A::Road);const auto p=ground(cell);if(!p)continue;
            const auto before=w.snapshot();click(*p);require(w.snapshot()==before,"Protected original-cell UI placement changed World");
            events.push_back({{"id","B04.protected-source"},{"status","PASS"},{"cell",{x,y}},{"validator_reason",decision.reason},
                {"actual_ui_message",view().last_message()},{"state",state()}});ui(A::Select);return;}
        events.push_back({{"id","B04.protected-source"},{"status","NOT_RUN"},{"reason","No protected original cell with a selectable current ground point"}});
    }
    bool buy(A action,sim::CommandType command,const std::string& id) {
        ui(action);const auto& w=view().world();
        for(int y=0;y<w.height();++y)for(int x=0;x<w.width();++x){const sim::Cell c{x,y};const auto valid=w.validate({command,c});
            if(!valid.accepted||!valid.changed)continue;const auto point=ground(c);if(!point)continue;
            const auto before=w.snapshot();const auto cost=w.construction_cost(command);click(*point);
            if(view().budget_warning_pending()){key(SDLK_ESCAPE);continue;}
            if(w.snapshot()==before)continue;
            require(w.command_sequence()==before.command_sequence+1&&w.treasury()==before.treasury-cost&&w.construction_spent_total()==before.construction_spent_total+static_cast<std::uint64_t>(cost),"UI purchase bypassed normal paid ledger");
            events.push_back({{"id",id},{"status","PASS"},{"cell",{x,y}},{"quote",cost},{"owner",w.building_owner_at(c)?Json(static_cast<unsigned>(*w.building_owner_at(c))):Json(nullptr)},{"state",state()}});
            const auto after=w.snapshot();click(*point);if(view().budget_warning_pending())key(SDLK_ESCAPE);
            require(w.snapshot()==after,"Duplicate footprint refusal changed authority");note(id+".duplicate","Repeated occupied-cell placement left exact World unchanged");ui(A::Select);return true;
        }
        ui(A::Select);events.push_back({{"id",id},{"status","NOT_RUN"},{"reason","No visible affordable legal cell; no placement bypass"},{"state",state()}});return false;
    }
    void controls() {
        paused(true);const auto before=view().world().ticks();ui(A::Step);require(view().world().ticks()==before+1,"Actual Step did not advance once");
        ui(A::Speed2);ui(A::Speed4);ui(A::Speed1);note("B03.clock","Clicked Step, Speed2, Speed4, Speed1 through the real toolbar");
        const auto state_before=view().world().snapshot();
        for(const auto keycode:{SDLK_F1,SDLK_G,SDLK_F4,SDLK_F6,SDLK_F7,SDLK_F8,SDLK_F10}){key(keycode);require(menu->render(),"Debug/overlay render failed");
            require(view().world().snapshot()==state_before,"Debug/overlay key changed World");note("B17.key."+std::to_string(keycode),"Actual diagnostic/overlay key routed; authority unchanged");}
        key(SDLK_F1);key(SDLK_H);capture("help");key(SDLK_ESCAPE);ui(A::ToggleIncome);require(menu->render(),"Income render failed");ui(A::ToggleIncome);
        ui(A::TogglePanel);ui(A::TogglePanel);require(view().world().snapshot()==state_before,"Help/income/panel changed World");
        SDL_Event e{};e.type=SDL_EVENT_WINDOW_FOCUS_LOST;event(e);e.type=SDL_EVENT_WINDOW_FOCUS_GAINED;event(e);
        require(view().world().snapshot()==state_before,"Focus boundary changed World");note("B03.focus","Actual focus loss/gain retained full authority");
    }
    void save_checkpoint(const std::string& label) {
        key(SDLK_F5);require(!menu->settings().last_save.empty(),"Actual F5 did not publish manual save");
        const auto path=menu->settings().last_save;const auto input=bytes(path);const auto saved=oe::persistence::read_save(path);
        require(saved.world==view().world().snapshot(),"Actual F5 snapshot differs");
        fs::copy_file(path,out/(label+".save.json"));key(SDLK_PERIOD);key(SDLK_F9);
        require(view().paused()&&view().world().snapshot()==saved.world&&bytes(path)==input,"Actual F9 changed complete saved authority/bytes");
        key(SDLK_F5);require(bytes(path)==input,"Actual F5 canonical complete document differs after F9");note("B18.manual","Actual F5/Step/F9/F5 complete snapshot and canonical document bytes equal");
    }
};

void inventory(Journey& h) {
    h.reset_menu({},h.app/"inventory-selection");h.menu_inventory("data-setup");h.menu_click("Choose Game Data Folder");h.dialog->select(h.data);h.menu->advance();
    h.menu_inventory("main");h.menu_click("N New Sandbox");h.menu_inventory("ordinary-new-default");
    require(h.menu->settings().profile==sim::RulesProfile::CityV11,"General default changed");
    for(int i=0;i<15;++i)h.menu_click("Previous rules");
    h.choose_map("Xia");
    for(int p=0;p<16;++p){h.menu_inventory("profile-"+std::to_string(p));
        if(h.start()){h.toolbar("profile-"+std::to_string(p),true);if(p==10){h.key(SDLK_F5);fs::copy_file(h.menu->settings().last_save,h.out/"city11.save.json");}h.back();h.menu_click("N New Sandbox");}
        if(p<15)h.menu_click("Next rules");}
    h.menu_click("Advanced...");h.menu_inventory("advanced-visuals");h.key(SDLK_ESCAPE);
    const std::array<const char*,5> entries{"O Original economy","F Original fire protection","S Sustainable economy","M Needs-aware logistics","W Cardinal-water economy"};
    for(const auto* label:entries){h.menu_click(label);h.choose_map("Xia");
        for(int d=0;d<5;++d){h.menu_inventory(std::string(label)+"-difficulty-"+std::to_string(d));h.key(SDLK_D);}
        h.key(SDLK_P);h.menu_inventory(std::string(label)+"-empty");h.key(SDLK_P);
        if(h.start()){h.toolbar(label,true);h.ui(A::ToggleIndustryPage);h.toolbar(std::string(label)+"-industry",true);h.ui(A::ToggleIndustryPage);h.capture("rule-"+std::to_string(h.view().world().rule_version()));h.key(SDLK_F5);}
        h.back();}
    h.menu_click("L Load Save");h.menu_inventory("manual-load");h.menu_click("Show recovery history");h.menu_inventory("recovery-load");
    if(h.menu->diagnostic_button_rect("Delete history...")){h.menu_click("Delete history...");h.menu_inventory("confirm-delete");h.menu_click("Cancel");}
    h.key(SDLK_ESCAPE);
    const auto upgrade=h.menu->app_root()/"saves/OE41-City11.json";fs::copy_file(h.out/"city11.save.json",upgrade);
    h.menu_click("L Load Save");if(h.menu->diagnostic_button_rect("Show manual saves"))h.menu_click("Show manual saves");
    const auto saves=oe::menu::list_saves(h.menu->app_root());bool found=false;
    for(const auto& entry:saves.entries){if(entry.path==upgrade){found=true;break;}h.menu_click("Next");}
    require(found,"Actual paid City11 save missing from manual list");h.menu_inventory("upgrade-eligible");
    h.menu_click("Enable demolition in a copy");h.menu_inventory("confirm-upgrade");h.menu_click("Cancel");h.key(SDLK_ESCAPE);
    h.note("B02.inventory","All16 ordinary profile choices, five original generations/difficulties, both original tool pages, manual/recovery and confirmation controls visited");
    const auto bad=h.app/"invalid-preferences";fs::create_directories(bad);{std::ofstream file(bad/"settings.json");file<<"{invalid";}
    h.reset_menu({},bad);h.menu_inventory("invalid-preferences-data-setup");h.menu_click("Reset preferences");
}
void create(Journey& h) {
    h.reset_menu({},h.app/"data-selection");require(h.menu->state()==Menu::State::DataSetup,"Fresh data-selection entry missing");h.menu_inventory("data-setup");
    h.menu_click("Choose Game Data Folder");h.dialog->select(h.out/"invalid-data");h.menu->advance();
    require(h.menu->state()==Menu::State::DataSetup,"Invalid data root accepted");h.capture("data-refused");
    h.menu_click("Choose Game Data Folder");h.dialog->select(h.data);h.menu->advance();require(h.menu->state()==Menu::State::MainMenu,"Supplied original data rejected");h.note("B01","Invalid root refused then explicit read-only supplied root selected");
    h.menu_click("W Cardinal-water economy");h.choose_map("Xia");require(h.start(),"Rule14 paid start refused");
    require(h.view().world().ticks()==0&&h.view().world().construction_spent_total()>0,"Paid menu starter did not begin normally");
    h.toolbar("rule14-main",true);h.ui(A::ToggleIndustryPage);h.toolbar("rule14-industry",true);h.ui(A::ToggleIndustryPage);
    h.controls();h.buy(A::Road,sim::CommandType::PlaceRoad,"B04.road");h.buy(A::Household,sim::CommandType::PlaceHousehold,"B04.house");h.operations();
    h.ticks(900);h.note("B08.staffing","900 real TickDriver ticks after paid starter and extra charged UI placements");
    h.zoom_images("paid-city");h.save_checkpoint("journey");
    const auto saved=h.view().world().snapshot();h.back();h.menu_inventory("main-with-session");h.menu_click("Autosave: ON");h.menu_click("Autosave: OFF");
    h.menu_click("L Load Save");h.menu_click("Show recovery history");h.menu_inventory("paid-recovery");h.menu_click("Open selected");
    if(h.menu->state()==Menu::State::ConfirmLeave)h.menu_click("Discard Changes");
    require(h.menu->state()==Menu::State::Loading,"Recovery did not enter Loading");require(h.menu->render(),"Recovery loading render failed");h.menu->advance();h.focus();
    require(h.view().world().snapshot()==saved,"Freshly published actual Recovery differs from manual state");h.note("B18.recovery","Actual menu recovery entry restored complete paid snapshot");
    const auto previous_map=h.view().capture_save_document().map_sha256;
    h.back();h.menu_click("N New Sandbox");h.choose_map("Handan");if(h.start()){
        const auto document=h.view().capture_save_document();require(h.view().world().profile()==sim::RulesProfile::CityV11&&h.view().world().rule_version()==3&&
            document.map_relative==fs::path("Cities/Handan.map")&&document.map_sha256!=previous_map,"Second menu session leaked old profile/map authority");
        h.note("B20.second-session","Second actual ordinary City11/rule3 Handan replaced Rule14 Xia through menu, with a distinct map SHA");h.capture("second-session");
    }else scenario_results.push_back({{"id","B20.second-session"},{"status","BLOCKED"},{"reason",h.menu->map_rules_result().reason}});
}
void resume(Journey& h,const fs::path& input) {
    no_symlinks(input);require(contains(private_root,fs::weakly_canonical(input)),"Only explicit OE41 earned save accepted");const auto original=bytes(input);const auto document=oe::persistence::read_save(input);
    const auto working=h.out/"working-input.save.json";fs::copy_file(input,working);
    h.menu_click("L Load Save");h.menu_click("Open save file...");h.dialog->select(working);h.menu->advance();
    require(h.menu->state()==Menu::State::Loading,"Independent actual menu load not deferred");require(h.menu->render(),"Independent loading render failed");h.menu->advance();h.focus();
    require(h.view().paused()&&h.view().world().snapshot()==document.world,"Independent actual menu load differs from complete saved World");
    h.key(SDLK_F5);require(bytes(h.menu->settings().last_save)==original,"Independent full canonical document bytes differ");h.note("B18.new-process","Independent process actual Menu load and F5 retain complete original document bytes");
    h.ticks(160);h.save_checkpoint("continued");require(bytes(input)==original,"Independent continuation altered protected input");h.zoom_images("resumed-city");
}
void map_journey(Journey& h,const std::string& map) {
    h.menu_click("N New Sandbox");while(h.menu->settings().profile!=sim::RulesProfile::CityV16)h.menu_click("Next rules");
    h.menu_click("Prepared starter settlement");h.choose_map(map);h.menu_inventory("map-"+map);h.capture("map-choice");
    if(!h.start()){scenario_results.push_back({{"id","B16"},{"map",map},{"status","BLOCKED"},{"reason",h.menu->map_rules_result().reason}});return;}
    require(h.view().world().rule_version()==4&&h.view().world().buildings().empty(),"Map view was not ordinary Rule4 Empty");
    h.zoom_images("map-"+map);h.buy(A::Road,sim::CommandType::PlaceRoad,"B04.map-road");h.protected_refusal();
    h.note("B16","Actual original map Menu→Rule4 Empty→SandboxView, 1/2/4x captures and paid Road attempt");h.save_checkpoint("map");
    scenario_results.push_back({{"id","B16"},{"map",map},{"status","PASS"},{"scope","scripted Menu/SandboxView and captures; visual inspection recorded separately"}});
}
void inspect_paid(Journey& h,const fs::path& input) {
    no_symlinks(input);const auto resolved=fs::canonical(input);
    require(contains(repo/".local/original-economy/oe40",resolved)||contains(repo/".local/original-economy/oe39",resolved)||contains(private_root,resolved),"Only explicit earned task save accepted");
    const auto original=bytes(input);const auto document=oe::persistence::read_save(input);
    const auto working=h.out/"working-input.save.json";fs::copy_file(input,working);
    h.menu_click("L Load Save");h.menu_click("Open save file...");h.dialog->select(working);h.menu->advance();
    require(h.menu->state()==Menu::State::Loading,"Paid inspection load did not defer");require(h.menu->render(),"Paid Loading render failed");h.menu->advance();h.focus();
    require(h.view().paused()&&h.view().world().snapshot()==document.world,"Paid inspection load altered actual World");const auto snapshot=h.view().world().snapshot();
    Json bodies=Json::array(),actors=Json::array(),tabs=Json::array();
    for(const auto& body:h.view().world().buildings())if(body.placed){const bool selected=h.select_body(body.id);
        bodies.push_back({{"id",static_cast<unsigned>(body.id)},{"object",static_cast<unsigned>(body.kind)},{"selected",selected},{"status",selected?"PASS":"NOT_RUN"},{"lines",selected?h.view().inspection_lines():std::vector<std::string>{}}});
        if(selected&&sim::original_house_kind(body.kind)){require(h.view().original_house_inspector_active(),"Actual House selection lacks inspector");
            for(unsigned n=0;n<4;++n){const auto section=static_cast<oe::SandboxView::OriginalHouseSection>(n);const auto r=h.view().original_house_section_rect(section);h.click({r.x+r.w*.5,r.y+r.h*.5});
                require(h.view().original_house_section()==section,"Actual House tab did not select");tabs.push_back({{"house",static_cast<unsigned>(body.id)},{"section",n},{"lines",h.view().original_house_section_lines()},{"status","PASS"}});}
            const auto r=h.view().original_house_overlay_rect();for(int n=0;n<4;++n)h.click({r.x+r.w*.5,r.y+r.h*.5});}
        require(h.view().world().snapshot()==snapshot,"Building/inspector selection changed World");}
    h.ui(A::Select);if(!h.view().debug_diagnostics())h.key(SDLK_F1);require(h.menu->render(),"Actor frame render failed");
    for(const auto& courier:h.view().world().couriers()){
        auto diagnostics=h.view().walker_diagnostics();const auto first=std::find_if(diagnostics.begin(),diagnostics.end(),[&](const auto& d){return d.id==courier.id;});
        require(first!=diagnostics.end(),"Actual courier has no bounded diagnostic row");const bool live=first->live_visible;
        bool selected=false;if(live){const auto world=h.view().camera().screen_to_world(first->screen_ground);h.center_world(world);require(h.menu->render(),"Centered actor render failed");
            diagnostics=h.view().walker_diagnostics();const auto d=std::find_if(diagnostics.begin(),diagnostics.end(),[&](const auto& row){return row.id==courier.id;});
            if(d!=diagnostics.end()&&d->submitted)for(int y=0;y<int(d->image_height)&&!selected;y+=4)for(int x=0;x<int(d->image_width)&&!selected;x+=4){
                const Point point{d->image_origin.x+x,d->image_origin.y+y};if(!h.view().layout().map.contains(point.x,point.y))continue;
                h.click(point);selected=h.view().input_diagnostic_state().selected_walker==static_cast<std::uint32_t>(courier.id);}}
        actors.push_back({{"id",static_cast<unsigned>(courier.id)},{"owner",static_cast<unsigned>(courier.owner)},{"role",static_cast<unsigned>(courier.role)},
            {"phase",static_cast<unsigned>(courier.phase)},{"live_visible",live},{"selected",selected},{"status",selected?"PASS":"NOT_RUN"},{"reason",selected?"actual alpha-hit selection":live?"No selectable visible pixel found":"idle/non-live actor has no pickable frame"}});
        require(h.view().world().snapshot()==snapshot,"Actor picking changed World");}
    h.key(SDLK_F1);h.zoom_images("earned-city");h.key(SDLK_F5);require(bytes(h.menu->settings().last_save)==original,"Paid inspection F5 full bytes differ");h.key(SDLK_F9);
    require(h.view().world().snapshot()==snapshot&&bytes(input)==original,"Paid inspection F9 or protected source changed");
    json_file(h.out/"inspection.json",{{"bodies",bodies},{"actors",actors},{"house_tabs",tabs},{"complete_world_unchanged",true},{"complete_f5_f9_bytes_equal",true}});
    h.note("B17.paid-inspection","Actual earned-city building, actor, House tab/overlay selection; individual inaccessible rows remain NOT_RUN");
}
}
int main(int argc,char** argv) {
    try {
        if(argc==2&&std::string_view(argv[1])=="--help"){std::cout<<oe41::help<<'\n';return 0;}
        require(argc==6||argc==7,oe41::help);const std::string mode=argv[1];
        require(mode=="--inventory"||mode=="--create"||mode=="--resume"||mode=="--map"||mode=="--inspect-paid",oe41::help);
        require((mode=="--resume"||mode=="--map"||mode=="--inspect-paid")==bool(argc==7),oe41::help);
        const fs::path data=fs::canonical(argv[2]),resources=fs::canonical(argv[3]);
        const auto app=fs::absolute(argv[4]).lexically_normal(),out=fs::absolute(argv[5]).lexically_normal();
        oe41::paths(data,resources,app,out);oe41::names();oe41::Journey h(data,resources,app,out);
        if(mode=="--inventory")oe41::inventory(h);else if(mode=="--create")oe41::create(h);
        else if(mode=="--resume")oe41::resume(h,fs::absolute(argv[6]).lexically_normal());else if(mode=="--inspect-paid")oe41::inspect_paid(h,fs::absolute(argv[6]).lexically_normal());else oe41::map_journey(h,argv[6]);
        json_file(out/"report.json",{{"format","openemperor-system-playtest-v1"},{"mode",mode},{"technical_status","PASS"},{"scenario_results",oe41::scenario_results},
            {"renderer",SDL_GetRendererName(h.renderer)},{"video_driver",SDL_GetCurrentVideoDriver()},{"sdl_runtime",SDL_GetVersion()},
            {"menu_controls",oe41::menu_rows},{"toolbar_controls",oe41::toolbar_rows},{"events",oe41::events},{"captures",h.captures},
            {"final_state",h.state()},{"native_gui","NOT_RUN"},{"human","NOT_RUN"},
            {"scope","Actual scripted MenuSession/SandboxView SDL events; toolbar selector PASS is distinct from purchase/supply; unexecuted menu rows remain NOT_RUN"}});
        std::cout<<"System playtest PASS: "<<out<<'\n';return 0;
    }catch(const std::exception& e){std::cerr<<"System playtest FAIL: "<<e.what()<<'\n';return 1;}
}
