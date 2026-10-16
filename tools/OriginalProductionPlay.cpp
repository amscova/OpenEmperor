#include "app/SandboxView.h"
#include <SDL3/SDL.h>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string_view>

int main(int argc,char** argv) {
    SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;
    try {
        if(argc!=4 && !(argc==5 && std::string_view(argv[4])=="--load"))
            throw std::invalid_argument("Usage: openemperor-original-production-play DATA MAP SAVE [--load]");
        const auto data=std::filesystem::canonical(argv[1]);
        auto session=openemperor::maps::load_stored_map_session(data,argv[2],
            openemperor::maps::FootprintPolicy::EdgeByte4x4Preview,openemperor::maps::StoredGraphicsProfile::Slot8);
        if(!SDL_Init(SDL_INIT_VIDEO))throw std::runtime_error(SDL_GetError());
        if(!SDL_CreateWindowAndRenderer("OpenEmperor original economy development",1280,800,
            SDL_WINDOW_RESIZABLE,&window,&renderer))throw std::runtime_error(SDL_GetError());
        auto view=openemperor::SandboxView::create_original_production_save_fixture(std::move(session));
        std::optional<openemperor::persistence::SaveDocument> initial;
        if(argc==5)initial=openemperor::persistence::read_save(argv[3]);
        view->configure_save(data,argv[2],argv[3],std::move(initial));
        view->initialize(window,renderer);
        std::cout<<"Explicit Rule10/Schema26 development session; starts paused. Build paid roads/houses, "
            "Clay, Pottery and Warehouse; connected houses admit residents. Space: run/pause; F5/F9: save/load.\n";
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
