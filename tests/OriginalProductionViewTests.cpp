#include "EconomyStartViewFixture.h"
#include "OriginalInfluenceFixture.h"
#include "OriginalProductionFixture.h"
#include "core/PerformanceDiagnostics.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <iostream>
#include <tuple>

namespace {
namespace oe=openemperor;
namespace sim=oe::simulation;
namespace orig=oe::original;
namespace f=oe_influence_fixture;
namespace fs=std::filesystem;
using oe::testing::economy::check;
void key(oe::SandboxView& view,SDL_Keycode code) {
    SDL_Event event{};event.type=SDL_EVENT_KEY_DOWN;event.key.key=code;
    bool running=true;view.handle_event(event,running);
}
void action_click(oe::SandboxView& view,oe::sandbox_ui::Action action) {
    const auto& buttons=view.layout().buttons;
    const auto button=std::find_if(buttons.begin(),buttons.end(),[&](const auto& value){return value.action==action;});
    check(button!=buttons.end(),"Requested production toolbar action is missing");
    const auto rect=button->rect;bool running=true;SDL_Event event{};
    event.type=SDL_EVENT_MOUSE_BUTTON_DOWN;event.button.button=SDL_BUTTON_LEFT;
    event.button.x=static_cast<float>(rect.x+rect.w/2);event.button.y=static_cast<float>(rect.y+rect.h/2);
    view.handle_event(event,running);event.type=SDL_EVENT_MOUSE_BUTTON_UP;view.handle_event(event,running);
}
void click(oe::SandboxView& view,sim::Cell cell) {
    double sx=0,sy=0;unsigned count=0;
    for(int y=80;y<660;y+=4)for(int x=8;x<1090;x+=4)
        if(view.pick({double(x),double(y)})==cell){sx+=x;sy+=y;++count;}
    check(count>0,"Production cell not visible");
    SDL_Event event{};event.type=SDL_EVENT_MOUSE_MOTION;
    event.motion.x=static_cast<float>(sx/count);event.motion.y=static_cast<float>(sy/count);
    bool running=true;view.handle_event(event,running);view.update(0);
    event={};event.type=SDL_EVENT_MOUSE_BUTTON_DOWN;event.button.button=SDL_BUTTON_LEFT;
    event.button.x=static_cast<float>(sx/count);event.button.y=static_cast<float>(sy/count);
    view.handle_event(event,running);event.type=SDL_EVENT_MOUSE_BUTTON_UP;view.handle_event(event,running);
}
fs::path authored_walker_profile(const fs::path& data) {
    using namespace oe::testing::economy;
    Bytes archive(40680U+2U*72U,0);
    u32(archive,0,static_cast<std::uint32_t>(archive.size()));u32(archive,4,214);
    u32(archive,12,2);u32(archive,16,2);u32(archive,20,1);
    const std::string name="authored-clip.bmp";
    std::copy(name.begin(),name.end(),archive.begin()+680);u32(archive,680+124,2);
    const auto at=40680U+72U;u32(archive,at,4);u32(archive,at+4,10);
    u16(archive,at+20,2);u16(archive,at+22,2);u16(archive,at+50,256);
    write(data/"DATA/culture-input.sg3",archive);
    write(data/"DATA/culture-input.555",{0,0,0,0,2,0,0x7c,0,0x7c,2,0,0x7c,0,0x7c});
    const auto path=data/"culture-input.json";
    f::write_text(path,R"({"schema_version":1,"mode":"curated_walker_preview","role":"clay",
"ticks_per_frame":1,"evidence":"Authored2x2clip solely to exercise real F3 input gating",
"frames":[{"alias":"dot","archive":"DATA/culture-input.sg3","image_index":1,"foot_anchor":[1,1]}],
"clips":{"pos_x":["dot"],"neg_x":["dot"],"pos_y":["dot"],"neg_y":["dot"]},"idle":"dot"})");
    return path;
}
}
int main(int argc,char** argv) {
    SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;
    try {
        check(argc<=2,"Expected at most capture output path");
        check(SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy") && SDL_SetHint(SDL_HINT_RENDER_DRIVER,"software") &&
            SDL_Init(SDL_INIT_VIDEO),"Production SDL init");
        check(SDL_CreateWindowAndRenderer("Original production",1100,700,SDL_WINDOW_HIDDEN,
            &window,&renderer),"Production SDL window");
        oe::testing::economy::Temp temp;const auto root=fs::canonical(temp.root),data=root/"data";
        const auto model=oe_production_fixture::authored_model_text(37);
        fs::create_directories(data/"Model");f::write_text(data/"Model/EmperorBuildingModels.txt",model);
        const auto session=[&]{return oe::maps::load_stored_map_session(data,"Cities/A.map",
            oe::maps::FootprintPolicy::EdgeByte4x4Preview,oe::maps::StoredGraphicsProfile::Slot8);};
        const auto target=root/"saves/production.json";
        auto view=oe::SandboxView::create_original_production_save_fixture(session());
        view->configure_save(data,"Cities/A.map",target);view->initialize(window,renderer);
        check(view->world().rule_version()==10 && view->world().treasury()==12000 && view->paused(),"Production identity");
        check(!view->industry_page(),"Industry page was not view-local defaultOFF");
        const auto& buttons=view->layout().buttons;
        check(view->world().bridges_supported() && view->world().ramps_supported() && buttons.size()==39 &&
              std::count_if(buttons.begin(),buttons.end(),[](const auto& b){return b.action==oe::sandbox_ui::Action::Bridge;})==1 &&
              std::count_if(buttons.begin(),buttons.end(),[](const auto& b){return b.action==oe::sandbox_ui::Action::Ramp;})==1,
              "Initial production toolbar disagrees with the actual inherited Bridge/Ramp capabilities");
        for(const auto action:{oe::sandbox_ui::Action::OriginalHempFarm,oe::sandbox_ui::Action::OriginalHempField,
                oe::sandbox_ui::Action::OriginalFoodFarm,oe::sandbox_ui::Action::OriginalWheatField,oe::sandbox_ui::Action::OriginalMilletField,
                oe::sandbox_ui::Action::HealthPost,oe::sandbox_ui::Action::ServicePost,
                oe::sandbox_ui::Action::OriginalAncestorShrine,oe::sandbox_ui::Action::OriginalWaterWell,
                oe::sandbox_ui::Action::OriginalDaoistShrine,oe::sandbox_ui::Action::OriginalBuddhistShrine,
                oe::sandbox_ui::Action::OriginalConfucianAcademy,oe::sandbox_ui::Action::OriginalMusicSchool,
                oe::sandbox_ui::Action::OriginalAcrobatSchool,oe::sandbox_ui::Action::OriginalDramaSchool}) {
            const auto button=std::find_if(buttons.begin(),buttons.end(),[&](const auto& value){return value.action==action;});
            check(button!=buttons.end() && view->layout().button_at(button->rect.x+2,button->rect.y+2)==action &&
                !view->layout().map.contains(button->rect.x+2,button->rect.y+2),"Hemp action was hidden or overlapped playable map input");
        }
        check(std::string(SDL_GetWindowTitle(window)).find("save schema 26")!=std::string::npos,"Production title");
        check(!view->recovery_safe_point(),"Development production exposed recovery");
        const auto buy=[&](sim::CommandType type,sim::Cell cell) {
            const auto result=view->execute({type,cell});check(result.accepted && result.changed,result.reason.c_str());
        };
        for(int x=104;x<=120;++x)buy(sim::CommandType::PlaceRoad,{x,103});
        for(int x:{106,109,112,115})buy(sim::CommandType::PlaceHousehold,{x,101});
        buy(sim::CommandType::PlaceClaySource,{106,104});
        buy(sim::CommandType::PlacePottery,{110,104});
        buy(sim::CommandType::PlaceWarehouse,{114,104});
        check(view->world().total_population()==0 && view->world().original_goods_balance(orig::Goods::Ceramics).stock==0,
            "Construction injected population or goods");
        bool travelling=false;
        for(int tick=0;tick<2000 && !travelling;++tick) {
            view->tick_once();
            travelling=std::any_of(view->world().couriers().begin(),view->world().couriers().end(),[](const auto& c){
                return c.role==sim::CourierRole::Pottery && c.phase==sim::CourierPhase::ToWarehouse && c.cargo>0;});
        }
        check(travelling && view->world().total_population()>0,"Real staffed Pottery courier never departed");
        const auto saved=view->world().snapshot();key(*view,SDLK_F5);
        check(oe::persistence::read_save(target).source_schema_version==26,"Actual production F5 schema");
        for(int i=0;i<100;++i)view->tick_once();
        key(*view,SDLK_F9);check(view->world().snapshot()==saved && view->paused(),"Actual production F9 midcourier replay");
        for(int i=0;i<800;++i)view->tick_once();
        const auto balance=view->world().original_goods_balance(orig::Goods::Ceramics);
        check(balance.supported && balance.conserved && balance.totals.produced>0 && balance.stock>0,
            "Actual warehouse arrival did not produce typed Ceramics");
        key(*view,SDLK_0);check(view->tool()==0,"Market keyboard selection unavailable");
        click(*view,{118,104});
        check(view->world().object_at({118,104})==sim::Object::Market,"Paid Market mouse placement failed");
        bool typed_trip=false;
        for(int i=0;i<5000 && !typed_trip;++i) {
            view->tick_once();
            for(const auto& c:view->world().couriers()) if(c.role==sim::CourierRole::MarketPotteryDistribution &&
                view->world().original_courier_cargo(c.id)>0 && c.edge_progress>0) {
                check(c.cargo==0 && c.reserved==0,"Typed delivery mirrored cargo in legacy movement fields");
                const auto rows=view->walker_diagnostics();
                const auto row=std::find_if(rows.begin(),rows.end(),[&](const auto& value){return value.id==c.id;});
                check(row!=rows.end() && row->cargo==view->world().original_courier_cargo(c.id),
                    "Rendered walker diagnostics hid actual typed cargo");
                typed_trip=true;break;
            }
        }
        check(typed_trip,"No actual typed Market-to-House courier departed");
        const auto typed_saved=view->world().snapshot();key(*view,SDLK_F5);
        for(int i=0;i<80;++i)view->tick_once();
        key(*view,SDLK_F9);check(view->world().snapshot()==typed_saved,"Actual typed midcourier F5/F9 diverged");
        bool delivered=false;
        for(int i=0;i<2500 && !delivered;++i) {
            view->tick_once();
            for(const auto& house:view->world().original_economy_state()->houses) delivered|=house.house.ceramics>0;
        }
        check(delivered,"Actual Market arrival never updated the authoritative House stock");
        const auto funds_before=view->world().treasury();
        for(const sim::Cell road:std::array{sim::Cell{120,104},sim::Cell{120,105},sim::Cell{120,106},
                sim::Cell{119,106},sim::Cell{118,106}}) buy(sim::CommandType::PlaceRoad,road);
        key(*view,SDLK_N);check(view->tool()==16,"Hemp Farm keyboard tool unavailable");
        click(*view,{118,107});
        check(view->world().object_at({118,107})==sim::Object::OriginalHempFarm,"Paid Hemp Farm mouse placement failed");
        const auto farm=view->world().building_owner_at({118,107});
        check(farm && view->selected_building()==farm,"Farm purchase did not retain explicit selection for paid Field");
        for(int i=0;i<100;++i)view->tick_once();
        check(view->world().original_goods_balance(orig::Goods::Hemp).totals.produced==0,
            "Farm without a separately paid Field produced free Hemp");
        key(*view,SDLK_M);check(view->tool()==17,"Hemp Field keyboard tool unavailable");
        click(*view,{120,107});
        check(view->world().object_at({120,107})==sim::Object::OriginalHempField &&
            view->world().treasury()==funds_before-54,"Field and Farm were not separately paid through normal UI commands");
        bool hemp_trip=false;
        for(int i=0;i<5000 && !hemp_trip;++i) {
            view->tick_once();
            for(const auto& c:view->world().couriers()) if(c.role==sim::CourierRole::OriginalHempToWarehouse &&
                view->world().original_courier_cargo(c.id)>0 && c.edge_progress>0) {
                check(view->world().original_courier_good(c.id)==orig::Goods::Hemp,"Actual Farm cargo lost its Hemp identity");
                hemp_trip=true;break;
            }
        }
        check(hemp_trip,"Paid staffed Farm never dispatched physical Hemp");
        const auto hemp_saved=view->world().snapshot();key(*view,SDLK_F5);
        for(int i=0;i<100;++i)view->tick_once();
        key(*view,SDLK_F9);check(view->world().snapshot()==hemp_saved,"Typed Hemp midcourier F5/F9 diverged");
        bool both_goods=false;
        for(int i=0;i<8000 && !both_goods;++i) {
            view->tick_once();
            for(const auto& house:view->world().original_economy_state()->houses)
                both_goods|=house.house.hemp>0 && house.house.ceramics>0;
        }
        check(both_goods,"No authoritative House received two independently transported original goods");
        const auto food_funds=view->world().treasury();
        for(int x=104;x<=117;++x) buy(sim::CommandType::PlaceRoad,{x,106});
        for(int y:{104,105})buy(sim::CommandType::PlaceRoad,{104,y});
        key(*view,SDLK_O);check(view->tool()==18,"Food Farm keyboard tool unavailable");
        click(*view,{104,107});
        check(view->world().object_at({106,109})==sim::Object::OriginalFoodFarm &&
            view->selected_building()==view->world().building_owner_at({104,107}),"Paid Food Farm lost3x3 occupancy or explicit selection");
        key(*view,SDLK_W);check(view->tool()==19,"Wheat Field keyboard tool unavailable");
        click(*view,{107,107});check(view->world().object_at({107,107})==sim::Object::OriginalWheatField,"Paid Wheat Field mouse command failed");
        key(*view,SDLK_O);click(*view,{110,107});
        check(view->world().object_at({112,109})==sim::Object::OriginalFoodFarm,"Second paid Food Farm footprint failed");
        key(*view,SDLK_L);check(view->tool()==20,"Millet Field keyboard tool unavailable");
        click(*view,{113,107});check(view->world().object_at({113,107})==sim::Object::OriginalMilletField,"Paid Millet Field mouse command failed");
        check(food_funds-view->world().treasury()==200,"Food Farms, Fields and access roads were not separately paid");
        bool food_trip=false;
        for(int i=0;i<10000 && !food_trip;++i) {
            view->tick_once();
            for(const auto& c:view->world().couriers()) if(c.role==sim::CourierRole::OriginalFoodToHouse &&
                c.phase==sim::CourierPhase::ToWarehouse && c.edge_progress>0 && view->world().original_food_cargo(c.id)>0) {
                const auto rows=view->walker_diagnostics();
                const auto row=std::find_if(rows.begin(),rows.end(),[&](const auto& value){return value.id==c.id;});
                check(c.cargo==0 && c.reserved==0 && row!=rows.end() && row->cargo==view->world().original_food_cargo(c.id),
                    "Actual Food cargo hidden by movement or visual diagnostics");
                food_trip=true;break;
            }
        }
        check(food_trip,"Paid Food chain never dispatched an actual House courier");
        const auto food_saved=view->world().snapshot();key(*view,SDLK_F5);
        for(int i=0;i<80;++i)view->tick_once();
        key(*view,SDLK_F9);check(view->world().snapshot()==food_saved,"Food midcourier F5/F9 changed sole crop or quality authority");
        bool real_quality=false;
        for(int i=0;i<10000 && !real_quality;++i) {
            view->tick_once();
            for(const auto& house:view->world().original_economy_state()->houses)
                real_quality|=house.house.food_quality>0 && house.house.hemp>0 && house.house.ceramics>0;
        }
        check(real_quality,"Actual Food House arrival never applied quality");
        const auto medical_funds=view->world().treasury();
        key(*view,SDLK_J);check(view->tool()==13,"Herbalist keyboard tool unavailable");
        click(*view,{108,104});
        key(*view,SDLK_9);check(view->tool()==9,"Acupuncture keyboard tool unavailable");
        click(*view,{112,104});
        check(view->world().object_at({109,105})==sim::Object::HealthPost &&
            view->world().object_at({113,105})==sim::Object::ServicePost &&
            view->world().treasury()==medical_funds-80,"Medical UI purchases lost2x2 ownership or prepared30/50 prices");
        check(view->world().original_production_state()->services.witnesses.empty(),"Provider purchase granted visits");
        bool medical_trip=false;
        for(int i=0;i<1000 && !medical_trip;++i) {
            view->tick_once();
            for(const auto& c:view->world().couriers())
                medical_trip|=(c.role==sim::CourierRole::HealthWorker || c.role==sim::CourierRole::Service) &&
                    c.phase==sim::CourierPhase::ToWarehouse && c.edge_progress>0;
        }
        check(medical_trip,"Paid medical sources never dispatched through ordinary movement");
        const auto medical_saved=view->world().snapshot();key(*view,SDLK_F5);
        for(int i=0;i<80;++i)view->tick_once();
        key(*view,SDLK_F9);check(view->world().snapshot()==medical_saved,"Medical midtrip F5/F9 changed sole visit authority");
        bool medical_visits=false;
        for(int i=0;i<4000 && !medical_visits;++i) {
            view->tick_once();
            for(const auto& house:view->world().original_economy_state()->houses)
                medical_visits|=house.house.herbalist>0 && house.house.acupuncture>0 &&
                    house.house.hemp>0 && house.house.ceramics>0 && house.house.food_quality>0;
        }
        check(medical_visits,"Actual same House never received both paid medical visits and physical goods/Food");
        const auto ancestor_funds=view->world().treasury();
        const auto previous_tool=view->tool();key(*view,SDLK_H);key(*view,SDLK_Y);
        check(view->tool()==previous_tool,"Ancestor keyboard tool escaped the help modal");
        key(*view,SDLK_H);
        key(*view,SDLK_Y);check(view->tool()==21,"Ancestor keyboard tool unavailable");
        click(*view,{116,104});
        check(view->world().object_at({117,105})==sim::Object::OriginalAncestorShrine214 &&
            view->world().treasury()==ancestor_funds-40,"Ancestor UI purchase lost2x2 ownership or prepared price");
        const auto shrine=*view->world().building_owner_at({116,104});
        check(view->world().workforce_required(shrine)==4,"Ancestor UI source lost its prepared worker requirement");
        for(const auto& h:view->world().original_economy_state()->houses)
            check(h.house.ancestor==0,"Ancestor purchase granted free access");
        bool ancestor_trip=false;
        for(int i=0;i<1000 && !ancestor_trip;++i) {
            view->tick_once();
            for(const auto& c:view->world().couriers()) if(c.role==sim::CourierRole::OriginalAncestor &&
                c.phase==sim::CourierPhase::ToWarehouse && c.edge_progress>0) ancestor_trip=true;
        }
        check(ancestor_trip,"Paid Ancestor source never dispatched an actual visitor");
        const auto ancestor_saved=view->world().snapshot();key(*view,SDLK_F5);
        for(int i=0;i<80;++i)view->tick_once();
        key(*view,SDLK_F9);check(view->world().snapshot()==ancestor_saved,"Ancestor midtrip F5/F9 changed its sole visit authority");
        bool ancestor_visit=false;
        for(int i=0;i<4000 && !ancestor_visit;++i) {
            view->tick_once();
            for(const auto& h:view->world().original_economy_state()->houses)
                ancestor_visit|=h.house.ancestor>0 && h.house.herbalist>0 && h.house.acupuncture>0 &&
                    h.house.hemp>0 && h.house.ceramics>0 && h.house.food_quality>0;
        }
        check(ancestor_visit,"No actual House received Ancestor plus medical visits and physical goods/Food");
        const auto water_funds=view->world().treasury();
        const auto old_tool=view->tool();key(*view,SDLK_H);key(*view,SDLK_Q);
        check(view->tool()==old_tool,"Water keyboard escaped Help modal");key(*view,SDLK_H);
        key(*view,SDLK_Q);check(view->tool()==22,"Water keyboard tool unavailable");
        click(*view,{116,107});
        check(view->world().object_at({117,108})==sim::Object::OriginalWaterWell72 &&
            view->world().treasury()==water_funds-25,"Water Well UI did not pay its2x2model source");
        bool water_trip=false;
        for(int i=0;i<1000 && !water_trip;++i) {
            view->tick_once();
            for(const auto& c:view->world().couriers()) water_trip|=c.role==sim::CourierRole::OriginalWater &&
                c.phase==sim::CourierPhase::ToWarehouse && c.edge_progress>0;
        }
        check(water_trip,"Paid Water Well did not start real movement");
        const auto water_saved=view->world().snapshot();key(*view,SDLK_F5);
        for(int i=0;i<80;++i)view->tick_once();key(*view,SDLK_F9);
        check(view->world().snapshot()==water_saved,"Water midtrip F5/F9 changed authority");
        bool plain=false;
        for(int i=0;i<4000 && !plain;++i) {
            view->tick_once();
            for(const auto& h:view->world().original_economy_state()->houses)
                plain|=h.house.type==orig::HouseType::PlainCottage && h.house.water && h.house.ancestor>0 &&
                    h.house.food_quality>=20 && h.house.hemp>0 && h.house.ceramics>0 && h.house.herbalist>0 && h.house.acupuncture>0;
        }
        check(plain,"Actual UI city never reached paid Water/Food/Ancestor-backed Plain Cottage");
        const auto religion_funds=view->world().treasury();
        for(int x=104;x<=120;++x)buy(sim::CommandType::PlaceRoad,{x,100});
        for(int y:{101,102})buy(sim::CommandType::PlaceRoad,{104,y});
        for(const auto [code,tool,origin,kind,price]:std::array{
                std::tuple{SDLK_U,23,sim::Cell{106,98},sim::Object::OriginalDaoistShrine215,45},
                std::tuple{SDLK_K,24,sim::Cell{110,98},sim::Object::OriginalBuddhistShrine217,45},
                std::tuple{SDLK_V,25,sim::Cell{114,97},sim::Object::OriginalConfucianAcademy219,80}}) {
            const auto old=view->tool();key(*view,SDLK_H);key(*view,code);
            check(view->tool()==old,"Religion key escaped Help modal");key(*view,SDLK_H);
            key(*view,code);check(view->tool()==tool,"Paid religion keyboard tool unavailable");
            const auto funds=view->world().treasury();click(*view,origin);
            const auto side=kind==sim::Object::OriginalConfucianAcademy219 ? 3:2;
            check(view->world().object_at({origin.x+side-1,origin.y+side-1})==kind &&
                view->world().treasury()==funds-price,"Religion UI purchase lost actual footprint or model price");
        }
        check(view->world().treasury()==religion_funds-208,"Religion sources and access roads were not separately paid");
        using A=oe::sandbox_ui::Action;
        const auto page_before=view->world().snapshot();
        const auto main_layout_before=view->layout();
        key(*view,SDLK_H);action_click(*view,A::ToggleIndustryPage);
        check(!view->industry_page() && view->world().snapshot()==page_before,"Industry page escaped Help modal or changed World");key(*view,SDLK_H);
        action_click(*view,A::ToggleIndustryPage);
        check(view->industry_page() && view->world().snapshot()==page_before && view->layout().buttons.size()<=31,
              "Industry page changed authority or exceeded20 construction tools");
        for (const auto action:{A::OriginalLoggingShed,A::OriginalBronzeSmelter,A::OriginalBronzewareMaker,A::OriginalLacquerwareMaker,
                A::OriginalPaperMaker,A::OriginalWeaver,A::OriginalTeaCuringShed,A::OriginalLacquerRefinery,A::OriginalSilkwormShed,
                A::OriginalTeaBush,A::OriginalLacquerTree,A::OriginalMulberryTree})
            check(std::count_if(view->layout().buttons.begin(),view->layout().buttons.end(),[&](const auto& b){return b.action==action;})==1,
                  "Industry page lost one of its12 actual paid construction tools");
        const auto industry_funds=view->world().treasury();
        for (int x=104;x<=120;++x) buy(sim::CommandType::PlaceRoad,{x,91});
        for (int y=92;y<100;++y) buy(sim::CommandType::PlaceRoad,{104,y});
        action_click(*view,A::OriginalPaperMaker);check(view->tool()==33,"Paper Maker industry tool is unavailable");
        click(*view,{110,89});
        check(view->world().object_at({111,90})==sim::Object::OriginalPaperMaker45,"Paper mouse purchase lost actual2x2 body");
        const auto paper_id=*view->world().building_owner_at({110,89});
        action_click(*view,A::OriginalTeaCuringShed);check(view->tool()==35,"Tea Shed industry tool is unavailable");click(*view,{118,89});
        const auto tea_id=*view->world().building_owner_at({118,89});
        action_click(*view,A::OriginalTeaBush);check(view->tool()==38,"Paid Tea Bush tool did not find its free purchased source");click(*view,{120,89});
        const auto plant=*view->world().building_owner_at({120,89});
        const auto& sources=view->world().original_production_state()->industry.sources;
        const auto tea=std::find_if(sources.begin(),sources.end(),[&](const auto& row){return row.building_id==tea_id;});
        check(tea!=sources.end() && tea->plant_id==plant && view->world().object_at({120,89})==sim::Object::OriginalTeaBush26 &&
              view->world().treasury()==industry_funds-166,"Plant mouse placement lost unique adjacent ownership or separate50/70/40/6 costs");
        const auto pause_tea=view->execute(sim::set_building_operation(tea_id,false));check(pause_tea.accepted && pause_tea.changed,"Tea did not pause through ordinary operation control");
        const auto elite_funds=view->world().treasury();
        const auto elite_old_tool=view->tool();key(*view,SDLK_H);action_click(*view,A::OriginalElitePlot);
        check(view->tool()==elite_old_tool,"Elite tool escaped the Help modal");key(*view,SDLK_H);
        action_click(*view,A::OriginalElitePlot);check(view->tool()==41,"Paid Elite plot tool unavailable");
        click(*view,{118,92});
        const auto elite_id=*view->world().building_owner_at({118,92});
        check(view->world().object_at({119,93})==sim::Object::OriginalElitePlot &&
              view->world().treasury()==elite_funds-80 && view->world().household_population(elite_id)==0,
              "Elite mouse purchase lost authored2x2 ownership, exact80 price or granted residents");
        const auto empty_diagnosis=view->world().original_housing_diagnosis(elite_id);
        check(empty_diagnosis.actual.type==orig::HouseType::EliteUnoccupied && empty_diagnosis.awaiting_admission &&
              empty_diagnosis.elite_family && empty_diagnosis.geometry_authored_2x2 && empty_diagnosis.geometry_deferred &&
              empty_diagnosis.original_required_side==4 && empty_diagnosis.pure_result_type==orig::HouseType::EliteUnoccupied,
              "Empty Elite diagnosis invented an Original footprint or pure first occupancy");
        const auto empty_matrix=view->world().original_housing_decision_matrix(elite_id);
        check(empty_matrix.size()==15 && empty_matrix[8].row_capacity==0 && empty_matrix[8].same_family &&
              !empty_matrix[0].same_family && !empty_matrix[10].needs_met,
              "Hypothetical15-row decision matrix granted an unlocked type or missing actual goods");
        const auto elite_saved=view->world().snapshot();key(*view,SDLK_F5);key(*view,SDLK_F9);
        check(view->world().snapshot()==elite_saved,"Empty paid Elite plot F5/F9 changed family or bootstrap state");
        view->set_tool(5);click(*view,{110,89});
        const auto paper_details=view->inspection_lines();
        check(view->industry_inspector_selected() && paper_details.size()<=10,
              "Actual sidebar did not select the compact visible Industry inspector");
        for (const std::string_view expected:{"Authored managed recipes","Hemp -> Paper","80 productive ticks","Hemp input","Marker art; original pending"})
            check(std::any_of(paper_details.begin(),paper_details.end(),[&](const auto& line){return line.find(expected)!=std::string::npos;}),
                  "Paper inspector hid actual input, authored recipe or pending original art");
        const auto page_saved=view->world().snapshot();key(*view,SDLK_F5);
        if (argc==2) {
            check(view->industry_page() && view->selected_building()==paper_id,"Industry capture lost its actual Paper inspector selection");
            view->update(0);check(view->render(),"Industry page actual capture frame");
            auto output=fs::path(argv[1]).parent_path()/"production.industry.png";
            auto* frame=SDL_RenderReadPixels(renderer,nullptr);check(frame!=nullptr,"Industry page capture pixels");
            const bool okay=SDL_SavePNG(frame,output.string().c_str());SDL_DestroySurface(frame);check(okay,"Industry page capture output");
            nlohmann::json tools=nlohmann::json::array();
            for (const auto& button:view->layout().buttons)
                tools.push_back({{"action",static_cast<unsigned>(button.action)},
                    {"rect",nlohmann::json::array({button.rect.x,button.rect.y,button.rect.w,button.rect.h})}});
            nlohmann::json observed{{"capture","paid-industry-page-paper-inspector"},
                {"development_revision",view->world().original_production_state()->development_revision},
                {"ticks",view->world().ticks()},{"funds",view->world().treasury()},
                {"population",view->world().total_population()},{"industry_page",view->industry_page()},
                {"industry_inspector",view->industry_inspector_selected()},
                {"selected_building",static_cast<unsigned>(*view->selected_building())},
                {"paper_id",static_cast<unsigned>(paper_id)},{"tea_id",static_cast<unsigned>(tea_id)},
                {"plant_id",static_cast<unsigned>(plant)},{"tools",tools},{"inspection_lines",paper_details},
                {"original_art_restored",false},{"human_acceptance_claimed",false}};
            auto observed_output=output;observed_output.replace_extension(".observed.json");f::write_text(observed_output,observed.dump(2)+"\n");
            auto saved_output=output;saved_output.replace_extension(".save.json");f::write_text(saved_output,f::read_text(target));
            check(view->world().snapshot()==page_saved,"Industry screenshot preparation mutated paid World authority");
        }
        for (int i=0;i<3;++i) view->tick_once();key(*view,SDLK_F9);
        check(view->industry_page() && view->world().snapshot()==page_saved,"Industry F5/F9 changed new body/plant binding or saved page authority");
        action_click(*view,A::ToggleIndustryPage);check(!view->industry_page(),"Industry Back action did not restore city tools");
        const auto& main_layout=view->layout();
        check(main_layout.buttons.size()==39 && main_layout.buttons.size()==main_layout_before.buttons.size() &&
              std::equal(main_layout.buttons.begin(),main_layout.buttons.end(),main_layout_before.buttons.begin(),[](const auto& a,const auto& b){
                  return a.action==b.action && a.rect.x==b.rect.x && a.rect.y==b.rect.y && a.rect.w==b.rect.w && a.rect.h==b.rect.h;
              }),"Industry Back changed the exact27 historical tools, page toggle or11 existing controls");
        check(main_layout.buttons[28].action==A::Pause &&
              main_layout.buttons[28].rect.y==main_layout.toolbar.y+6*36*main_layout.scale &&
              main_layout.toolbar.h==7*36*main_layout.scale,
              "Main page exceeded its actual six construction rows plus one control row");
        bool real_paper_trip=false;
        for (int i=0;i<3000 && !real_paper_trip;++i) {
            view->tick_once();
            for (const auto& trip:view->world().original_production_state()->industry.aux_trips)
                real_paper_trip|=trip.material==sim::OriginalAuxMaterial::Paper && trip.phase==sim::OriginalGoodsTransferPhase::Loaded &&
                    view->world().courier(trip.transfer.courier_id).role==sim::CourierRole::OriginalAuxSupplier;
        }
        check(real_paper_trip && view->world().workers_assigned(paper_id)==10,"Actual paid Paper chain did not supply a real Academy material courier");
        const auto walkers=view->walker_diagnostics();
        check(std::any_of(walkers.begin(),walkers.end(),[](const auto& row){return row.role==sim::CourierRole::OriginalAuxSupplier && row.cargo>0 && row.live_visible;}),
              "Actual material cargo was hidden by marker or diagnostic presentation");
        const auto paper_trip_saved=view->world().snapshot();key(*view,SDLK_F5);for (int i=0;i<40;++i) view->tick_once();key(*view,SDLK_F9);
        check(view->world().snapshot()==paper_trip_saved,"Material midcourier F5/F9 changed sole cargo or recipe sinks");
        for (int i=0;i<800 && view->world().household_population(elite_id)==0;++i) view->tick_once();
        const auto moved=view->world().original_housing_diagnosis(elite_id);
        const auto& histories=view->world().original_production_state()->housing.houses;
        const auto admitted=std::find_if(histories.begin(),histories.end(),[&](const auto& row){return row.building_id==elite_id;});
        check(moved.actual.type==orig::HouseType::ModestSiheyuan && moved.actual.occupants>0 &&
              admitted!=histories.end() && admitted->first_transitions[10] &&
              admitted->first_transitions[10]->kind==sim::OriginalHousingTransitionKind::Admission &&
              admitted->first_transitions[10]->evaluation.input.type==orig::HouseType::EliteUnoccupied &&
              admitted->first_transitions[10]->admitted>0 && admitted->first_transitions[10]->admitted<=2,
              "Elite first occupancy did not record the actual11→13 paid immigration event");
        bool religion_trip=false;
        for(int i=0;i<1200 && !religion_trip;++i) {
            view->tick_once();
            for(const auto& c:view->world().couriers()) religion_trip|=c.role==sim::CourierRole::OriginalConfucian &&
                c.phase==sim::CourierPhase::ToWarehouse && c.edge_progress>0;
        }
        check(religion_trip,"Paid Confucian3x3 source never dispatched through shared movement");
        const auto religion_saved=view->world().snapshot();key(*view,SDLK_F5);
        for(int i=0;i<80;++i)view->tick_once();key(*view,SDLK_F9);
        check(view->world().snapshot()==religion_saved,"Religion midtrip F5/F9 changed actual trip/clock authority");
        bool all_religions=false;
        for(int i=0;i<6000 && !all_religions;++i) {
            view->tick_once();
            for(const auto& h:view->world().original_economy_state()->houses)
                all_religions|=h.building_id==elite_id && orig::is_elite(h.house.type) && h.house.confucian>0 &&
                    h.house.daoist>0 && h.house.buddhist>0 && h.house.ancestor>0 && h.house.water &&
                    h.house.herbalist>0 && h.house.acupuncture>0 && h.house.hemp>0 && h.house.ceramics>0 && h.house.food_quality>0;
        }
        check(all_religions,"No actual occupied Elite received its Paper-backed Confucian visit and five other service categories");
        for (const auto& h:view->world().original_economy_state()->houses)
            if (!orig::is_elite(h.house.type)) check(h.house.confucian==0,"Common House received forbidden Elite-only Confucian effect");
        const auto culture_funds=view->world().treasury();
        for(int x=104;x<=120;++x)if(!view->world().road_cell({x,96}))buy(sim::CommandType::PlaceRoad,{x,96});
        for(int y:{97,98,99})if(!view->world().road_cell({104,y}))buy(sim::CommandType::PlaceRoad,{104,y});
        view->set_walker_visuals(authored_walker_profile(data));
        check(view->walker_display_stats().schema_version==1,"Authored profile did not enable actual F3 input gate");
        const auto diagnostic_world=view->world().snapshot();key(*view,SDLK_F3);action_click(*view,A::ToggleIndustryPage);
        check(!view->industry_page() && view->world().snapshot()==diagnostic_world,"Industry page escaped F3 diagnostics or changed authority");key(*view,SDLK_F3);
        for(const auto [code,tool,origin,kind,command,side]:std::array{
                std::tuple{SDLK_E,26,sim::Cell{106,94},sim::Object::OriginalMusicSchool211,sim::CommandType::PlaceOriginalMusicSchool,2},
                std::tuple{SDLK_C,27,sim::Cell{110,93},sim::Object::OriginalAcrobatSchool212,sim::CommandType::PlaceOriginalAcrobatSchool,3},
                std::tuple{SDLK_A,28,sim::Cell{114,93},sim::Object::OriginalDramaSchool213,sim::CommandType::PlaceOriginalDramaSchool,3}}) {
            const auto old=view->tool();key(*view,SDLK_H);key(*view,code);
            check(view->tool()==old,"Culture key escaped Help modal");key(*view,SDLK_H);
            key(*view,SDLK_F3);key(*view,code);
            check(view->tool()==old,"Culture key changed the tool during walker diagnostics");key(*view,SDLK_F3);
            key(*view,code);check(view->tool()==tool,"Paid school keyboard tool unavailable");
            const auto funds=view->world().treasury(),price=view->world().construction_cost(command);
            click(*view,origin);
            check(view->world().object_at({origin.x+side-1,origin.y+side-1})==kind &&
                view->world().treasury()==funds-price,"School UI purchase lost its evidenced footprint or prepared model price");
            check(view->world().workers_assigned(*view->world().building_owner_at(origin))==view->world().workforce_required(*view->world().building_owner_at(origin)),
                "Paid culture view scene did not supply actual full workers");
            for(const auto& h:view->world().original_economy_state()->houses)
                check(h.house.music==0 && h.house.acrobat==0 && h.house.drama==0,"School purchase granted a free culture visit");
        }
        view->set_walker_visuals({});
        check(view->world().treasury()==culture_funds-212,"Schools and16 additional physical access roads were not separately paid");
        bool culture_trip=false;
        for(int i=0;i<2000 && !culture_trip;++i) {
            view->tick_once();
            for(const auto& c:view->world().couriers()) culture_trip|=c.role==sim::CourierRole::OriginalDrama &&
                c.phase==sim::CourierPhase::ToWarehouse && c.edge_progress>0;
        }
        check(culture_trip,"Paid Drama3x3 source never dispatched through shared movement");
        view->set_tool(5);click(*view,{114,93});
        const auto source_details=view->inspection_lines();
        for(const std::string_view expected:{"Experimental school-to-House","Target timer:","Available ","No future visit promised"})
            check(std::any_of(source_details.begin(),source_details.end(),[&](const auto& line){return line.find(expected)!=std::string::npos;}),
                "School inspector hid its experimental adapter or derived actual-target delivery status");
        const auto culture_saved=view->world().snapshot();key(*view,SDLK_F5);
        for(int i=0;i<80;++i)view->tick_once();key(*view,SDLK_F9);
        check(view->world().snapshot()==culture_saved,"Culture midtrip F5/F9 changed real trip/clock authority");
        bool all_services=false;
        for(int i=0;i<6000 && !all_services;++i) {
            view->tick_once();
            for(const auto& h:view->world().original_economy_state()->houses)
                all_services|=h.building_id==elite_id && orig::is_elite(h.house.type) && h.house.music>0 && h.house.acrobat>0 && h.house.drama>0 &&
                    h.house.confucian>0 && h.house.daoist>0 && h.house.buddhist>0 && h.house.ancestor>0 && h.house.water &&
                    h.house.herbalist>0 && h.house.acupuncture>0 && h.house.hemp>0 && h.house.ceramics>0 && h.house.food_quality>0;
        }
        check(all_services,"No actual House simultaneously received nine separate services and paid water/goods/Food");
        bool common_services=false;
        for (const auto& h:view->world().original_economy_state()->houses) if (!orig::is_elite(h.house.type)) {
            check(h.house.confucian==0,"New Elite-only religion leaked to a Common House");
            common_services|=h.house.music>0 && h.house.acrobat>0 && h.house.drama>0 && h.house.daoist>0 &&
                h.house.buddhist>0 && h.house.ancestor>0 && h.house.herbalist>0 && h.house.acupuncture>0;
        }
        check(common_services,"No actual Common House retained its eight independently delivered permitted services");
        key(*view,SDLK_F5);const auto current=f::read_text(target);
        const auto before=view->world().snapshot();
        const auto model_before=view->world().original_rules_model();
        auto bad=nlohmann::json::parse(current);
        bad["world"]["original_production"]["development_revision"]=sim::kOriginalProductionDevelopmentRevision-1;
        f::write_text(target,bad.dump());key(*view,SDLK_F9);
        check(view->world().snapshot()==before && view->world().original_rules_model()==model_before,
            "Bad development revision published through F9");
        f::write_text(target,current);key(*view,SDLK_F9);
        std::optional<sim::Cell> supplied;
        for(const auto& house:view->world().original_economy_state()->houses)
            if(house.building_id==elite_id && orig::is_elite(house.house.type) && house.house.water && house.house.hemp>0 && house.house.ceramics>0 && house.house.food_quality>0 &&
                house.house.herbalist>0 && house.house.acupuncture>0 && house.house.ancestor>0 &&
                house.house.confucian>0 && house.house.daoist>0 && house.house.buddhist>0 &&
                house.house.music>0 && house.house.acrobat>0 && house.house.drama>0) {supplied=view->world().building(house.building_id).cell;break;}
        check(supplied.has_value(),"No actual two-goods House available for inspection");
        view->set_tool(5);click(*view,*supplied);
        const auto details=view->inspection_lines();
        for (const std::string_view expected:{"temp2x2","Authored2x2; Original4x4:26","Current row","Next row",
                "Bronze OR Lacquer","Daoist OR Buddhist","need","Original housing art pending"})
            check(std::any_of(details.begin(),details.end(),[&](const auto& line){return line.find(expected)!=std::string::npos;}),
                  "House inspector hid a real condition, alternative or temporary geometry boundary");
        check(std::count_if(details.begin(),details.end(),[](const auto& line) {
            return line.find("Herbalist")!=std::string::npos && line.find("Acupuncture")!=std::string::npos;
        })==1,"Medical values wrapped and displaced the default House inspector");
        check(std::count_if(details.begin(),details.end(),[](const auto& line) {
            return line.find("Music")!=std::string::npos && line.find("Acrobat")!=std::string::npos && line.find("Drama")!=std::string::npos;
        })==1,"Three culture timers did not fit on their own visible inspector line");
        check(std::count_if(details.begin(),details.end(),[](const auto& line) {
            return line.find("Food:")!=std::string::npos && line.find("Quality:")!=std::string::npos &&
                line.find(" W")!=std::string::npos && line.find(" M")!=std::string::npos;
        })==1,"Food stock, quality and actual crop amounts did not fit on one inspector line");
        const auto misses=std::find_if(details.begin(),details.end(),[](const auto& line) {
            return line.find("Food demand misses:")!=std::string::npos;
        });
        check(misses!=details.end() && std::distance(details.begin(),misses)<=10,
              "Ancestor display displaced Food demand misses from the visible default inspector");
        oe::performance::set_enabled(true);oe::performance::reset();
        for(int i=0;i<100;++i){view->update(0);check(view->render(),"Production idle frame");}
        check(view->world().snapshot()==before,"Production frame changed World");
        for(const auto counter:{oe::performance::Counter::WorldCopies,oe::performance::Counter::WorldExecutes,
            oe::performance::Counter::BfsCalls,oe::performance::Counter::AssetDecodes,oe::performance::Counter::TextureUploads,
            oe::performance::Counter::FileReads,oe::performance::Counter::FileWrites})
            check(oe::performance::counter(counter)==0,"Production frame prepared or mutated state");
        oe::performance::set_enabled(false);
        if(argc==2){auto* frame=SDL_RenderReadPixels(renderer,nullptr);check(frame!=nullptr,"Capture pixels");
            const bool okay=SDL_SavePNG(frame,argv[1]);SDL_DestroySurface(frame);check(okay,"Capture output");
            const auto id=*view->selected_building();
            const auto& houses=view->world().original_economy_state()->houses;
            const auto h=std::find_if(houses.begin(),houses.end(),[&](const auto& value){return value.building_id==id;});
            check(h!=houses.end() && h->house.food_quality>0,"Captured House lost its actual Food quality");
            nlohmann::json observed{{"development_revision",view->world().original_production_state()->development_revision},
                {"ticks",view->world().ticks()},{"funds",view->world().treasury()},
                {"population",view->world().total_population()},
                {"selected_house",static_cast<unsigned>(id)},
                {"selected_house_food_quality",h->house.food_quality},
                {"selected_house_hemp",h->house.hemp},{"selected_house_ceramics",h->house.ceramics},
                {"selected_house_herbalist",h->house.herbalist},{"selected_house_acupuncture",h->house.acupuncture},
                {"selected_house_ancestor",h->house.ancestor},{"selected_house_water",h->house.water},
                {"selected_house_confucian",h->house.confucian},{"selected_house_daoist",h->house.daoist},
                {"selected_house_buddhist",h->house.buddhist},
                {"selected_house_music",h->house.music},{"selected_house_acrobat",h->house.acrobat},
                {"selected_house_drama",h->house.drama},
                {"selected_house_type",static_cast<unsigned>(h->house.type)},
                {"selected_house_devolve_counter",h->house.devolve_counter},
                {"model_sha256",view->world().original_rules_model()->provenance().sha256},
                {"synthetic_model",true},{"inspection_lines",details},{"idle_frames",100},{"idle_world_unchanged",true}};
            for(const auto& row:view->world().original_production_state()->food.stocks) if(row.building_id==id)
                observed["selected_house_food"]=row.amounts;
            auto output=fs::path(argv[1]);output.replace_extension(".observed.json");f::write_text(output,observed.dump(2));
            output=fs::path(argv[1]);output.replace_extension(".save.json");f::write_text(output,current);
            SDL_Event scroll{};scroll.type=SDL_EVENT_MOUSE_WHEEL;scroll.wheel.y=-8.0F;
            scroll.wheel.mouse_x=static_cast<float>(view->layout().panel.x+12);
            scroll.wheel.mouse_y=static_cast<float>(view->layout().panel.y+50);
            bool running=true;view->handle_event(scroll,running);view->update(0);
            check(view->render(),"Actual housing diagnosis scroll frame");
            output=fs::path(argv[1]).parent_path()/"production.housing.png";
            auto* diagnosis_frame=SDL_RenderReadPixels(renderer,nullptr);check(diagnosis_frame!=nullptr,"Housing diagnosis capture pixels");
            const bool diagnosis_okay=SDL_SavePNG(diagnosis_frame,output.string().c_str());SDL_DestroySurface(diagnosis_frame);
            check(diagnosis_okay,"Actual housing diagnosis capture output");
            const auto diagnosis=view->world().original_housing_diagnosis(id);
            const auto row_json=[](const sim::OriginalHousingRowDiagnosis& row) {
                nlohmann::json needs=nlohmann::json::array();
                for (const auto& need:row.requirements) needs.push_back({{"field",static_cast<unsigned>(need.field)},
                    {"required",need.required},{"actual",need.actual},{"alternative",need.alternative},
                    {"presence",need.presence},{"met",need.met}});
                return nlohmann::json{{"row",static_cast<unsigned>(row.row)},{"low",row.low},{"high",row.high},
                    {"row_capacity",row.row_capacity},{"needs_met",row.needs_met},{"requirements",needs}};
            };
            nlohmann::json housing_observed{{"capture","actual-paid-elite-housing-diagnosis"},
                {"development_revision",view->world().original_production_state()->development_revision},
                {"ticks",view->world().ticks()},{"selected_house",static_cast<unsigned>(id)},
                {"selected_house_type",static_cast<unsigned>(diagnosis.actual.type)},
                {"occupants",diagnosis.actual.occupants},{"elite_family",diagnosis.elite_family},
                {"actual_cached_desirability",diagnosis.cached_desirability},{"counter",diagnosis.actual.devolve_counter},
                {"counter_limit",diagnosis.devolve_limit},{"admission_capacity",diagnosis.admission_capacity},
                {"census_capacity",diagnosis.census_capacity},{"current",row_json(diagnosis.current)},{"next",row_json(diagnosis.next)},
                {"authored_footprint_side",2},{"original_required_side",diagnosis.original_required_side},
                {"geometry_deferred",diagnosis.geometry_deferred},{"hypothetical_matrix_rows",15},
                {"inspection_lines",view->inspection_lines()},{"actual_panel_wheel_notches",-8},
                {"world_unchanged",view->world().snapshot()==before},{"original_art_restored",false}};
            auto housing_observed_output=output;housing_observed_output.replace_extension(".observed.json");
            f::write_text(housing_observed_output,housing_observed.dump(2)+"\n");
            auto housing_saved_output=output;housing_saved_output.replace_extension(".save.json");f::write_text(housing_saved_output,current);
            check(view->selected_building()==id && view->world().snapshot()==before,
                  "Housing scroll or diagnostic screenshot changed paid World authority");
            // Move through the real preceding service rows to the actual
            // food/goods requirements. Wrapped OR labels also occupy rows.
            scroll.wheel.y=-13.0F;view->handle_event(scroll,running);view->update(0);
            check(view->render(),"Actual housing requirements scroll frame");
            output=fs::path(argv[1]).parent_path()/"production.requirements.png";
            auto* requirements_frame=SDL_RenderReadPixels(renderer,nullptr);check(requirements_frame!=nullptr,"Housing requirements capture pixels");
            const bool requirements_okay=SDL_SavePNG(requirements_frame,output.string().c_str());SDL_DestroySurface(requirements_frame);
            check(requirements_okay,"Actual housing requirements capture output");
            auto requirements_observed=housing_observed;
            requirements_observed["capture"]="actual-paid-elite-housing-requirements";
            requirements_observed["actual_panel_wheel_notches"]=-21;
            requirements_observed["additional_panel_wheel_notches"]=-13;
            requirements_observed["inspection_lines"]=view->inspection_lines();
            requirements_observed["world_unchanged"]=view->world().snapshot()==before;
            auto requirements_observed_output=output;requirements_observed_output.replace_extension(".observed.json");
            f::write_text(requirements_observed_output,requirements_observed.dump(2)+"\n");
            auto requirements_saved_output=output;requirements_saved_output.replace_extension(".save.json");f::write_text(requirements_saved_output,current);
            check(view->selected_building()==id && view->world().snapshot()==before,
                  "Requirements scroll or screenshot changed paid World authority");
        }
        view->shutdown();view.reset();
        SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();
        std::cout<<"Original production SDL paid goods/Food/nine services, actual House timers/quality, midcourier F5/F9/100 idle frames PASS\n";return 0;
    }catch(const std::exception& error){
        std::cerr<<error.what()<<'\n';SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();return 1;
    }
}
