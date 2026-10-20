#include "app/SandboxView.h"
#include "app/ResourceLocator.h"
#include "app/VisualSelection.h"
#include <SDL3/SDL.h>
#include <array>
#include <filesystem>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {
constexpr std::string_view usage="Usage: openemperor-original-production-play DATA MAP SAVE [--load] "
    "[--difficulty very-easy|easy|normal|hard|very-hard]\n";
constexpr std::array<std::string_view,5> difficulty_names{"very-easy","easy","normal","hard","very-hard"};
struct Options {
    std::vector<std::string_view> paths;
    bool load=false,help=false;
    std::optional<openemperor::original::Difficulty> difficulty;
};
Options parse_options(int argc,char** argv) {
    Options result;
    for (int i=1;i<argc;++i) {
        const std::string_view value=argv[i];
        if (value=="--help") {
            if (result.help) throw std::invalid_argument("Duplicate --help option");
            result.help=true;
        } else if (value=="--load") {
            if (result.load) throw std::invalid_argument("Duplicate --load option");
            result.load=true;
        } else if (value=="--difficulty") {
            if (result.difficulty) throw std::invalid_argument("Duplicate --difficulty option");
            if (++i>=argc) throw std::invalid_argument("--difficulty requires a named difficulty");
            const std::string_view name=argv[i];
            for (std::size_t n=0;n<difficulty_names.size();++n) if (name==difficulty_names[n])
                result.difficulty=static_cast<openemperor::original::Difficulty>(n);
            if (!result.difficulty) throw std::invalid_argument("Unknown original difficulty; use very-easy, easy, normal, hard or very-hard");
        } else if (value.starts_with('-')) throw std::invalid_argument("Unknown original production play option");
        else result.paths.push_back(value);
    }
    if (result.paths.size()!=3 && !(result.help && result.paths.empty()))
        throw std::invalid_argument(std::string(usage));
    return result;
}
}

int main(int argc,char** argv) {
    SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;
    try {
        const auto options=parse_options(argc,argv);
        if (options.help) {
            std::cout<<usage<<"Fresh default: normal. Load uses saved difficulty; an explicit selection must match.\n";
            if (options.difficulty) std::cout<<"Selected difficulty: "<<difficulty_names[static_cast<std::size_t>(*options.difficulty)]<<'\n';
            return 0;
        }
        const auto data=std::filesystem::canonical(options.paths[0]);
        std::optional<openemperor::persistence::SaveDocument> initial;
        if (options.load) {
            initial=openemperor::persistence::read_save(options.paths[2]);
            if (options.difficulty && (!initial->world.original_economy ||
                !initial->world.original_economy->model_source ||
                initial->world.original_economy->model_source->difficulty!=*options.difficulty))
                throw std::invalid_argument("Requested original difficulty differs from saved difficulty");
        }
        auto session=openemperor::maps::load_stored_map_session(data,options.paths[1],
            openemperor::maps::FootprintPolicy::EdgeByte4x4Preview,openemperor::maps::StoredGraphicsProfile::Slot8);
        if(!SDL_Init(SDL_INIT_VIDEO))throw std::runtime_error(SDL_GetError());
        if(!SDL_CreateWindowAndRenderer("OpenEmperor original economy development",1280,800,
            SDL_WINDOW_RESIZABLE,&window,&renderer))throw std::runtime_error(SDL_GetError());
        auto view=openemperor::SandboxView::create_original_production_save_fixture(std::move(session),options.difficulty);
        view->configure_save(data,options.paths[1],options.paths[2],std::move(initial));
        const auto* base=SDL_GetBasePath();
        const auto resources=openemperor::locate_resource_root(base ? std::filesystem::path(base):std::filesystem::path{});
        const auto visuals=openemperor::detect_and_select_visual_profiles(data,resources);
        view->set_walker_visuals(visuals.walker,visuals.walker_source);
        view->set_building_visuals(visuals.building,visuals.building_source);
        view->set_road_visuals(visuals.road,visuals.road_source);
        view->set_fire_visuals(visuals.fire,visuals.fire_source,visuals.fire_fallback_reason);
        view->set_fire_inspector_visuals(visuals.fire_inspector,visuals.fire_inspector_source,visuals.fire_inspector_fallback_reason);
        view->set_market_walker_visuals(visuals.market_walker,visuals.market_walker_source,visuals.market_walker_fallback_reason);
        view->set_service_walker_visuals(visuals.service_walker,visuals.service_walker_source,visuals.service_walker_fallback_reason);
        view->set_health_walker_visuals(visuals.health_walker,visuals.health_walker_source,visuals.health_walker_fallback_reason);
        view->set_original_house_visuals(visuals.original_house,visuals.original_house_source,visuals.original_house_fallback_reason);
        view->initialize(window,renderer);
        std::cout<<"Explicit Rule10/Schema26 development session; starts paused. Build paid roads/houses, "
            "Clay, Pottery and Warehouse; connected houses admit residents. Space: run/pause; F5/F9: save/load.\n";
        std::cout<<"Difficulty: "<<difficulty_names[static_cast<std::size_t>(view->world().original_rules_model()->difficulty())]<<
            "; authored direct house taxes every816 ticks; T: actual finances and live housing mission.\n";
        const auto houses=view->original_house_display_stats();
        std::cout<<"House art: "<<(houses.active ? "proven type mapping, canonical static representative":
            houses.fallback_reason)<<"; F4: authored comparison; F6: independent roads.\n";
        bool running=true;auto previous=SDL_GetTicksNS();
        while(running){
            SDL_Event event{};while(SDL_PollEvent(&event))view->handle_event(event,running);
            const auto now=SDL_GetTicksNS();view->update(static_cast<double>(now-previous)/1e9);previous=now;
            if(!view->render())throw std::runtime_error(SDL_GetError());
            SDL_Delay(8);
        }
        view->shutdown();view.reset();
        SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();return 0;
    }catch(const std::exception& error){
        std::cerr<<error.what()<<'\n';SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();return 1;
    }
}
