// Read-only test-starter preflight. Reuses production prerequisites, decoder,
// actual legacy mask and complete policy; never runs a paid demo or saves.
#include "maps/MapRulesCheck.h"
#include "maps/StoredMapSession.h"
#include "maps/SandboxPlacement.h"
#include "renderer/StoredGraphicsRenderer.h"
#include "persistence/SandboxSave.h"
#include <SDL3/SDL.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <iostream>
#include <stdexcept>
using namespace openemperor;
int main(int argc,char** argv) {
    SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;
    try {
        if(argc!=3)throw std::invalid_argument("usage: empty-map-preflight DATA Cities/exact.map");
        const auto root=std::filesystem::canonical(argv[1]);
        const maps::MapRulesRequest request{root,argv[2],simulation::RulesProfile::CityV16,4,2};
        const auto checked=maps::check_map_rules(request);
        if(checked.status!=maps::MapRulesStatus::MapRulesChecked)
            throw std::runtime_error(checked.detail.empty()?checked.reason:checked.detail);
        if(!SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy") || !SDL_Init(SDL_INIT_VIDEO))
            throw std::runtime_error(SDL_GetError());
        window=SDL_CreateWindow("Owned empty-map preflight",1280,720,SDL_WINDOW_HIDDEN);
        if(!window)throw std::runtime_error(SDL_GetError());
        renderer=SDL_CreateRenderer(window,"software");
        if(!renderer)throw std::runtime_error(SDL_GetError());
        nlohmann::json report;
        {
            auto session=maps::load_stored_map_session(root,argv[2],maps::FootprintPolicy::EdgeByte4x4Preview,
                maps::StoredGraphicsProfile::Slot8,maps::GreatWallPresentationMode::Automatic,checked.input_sha256);
            const maps::MapGeometry geometry{session.map.declared_map_size};
            StoredGraphicsRenderer background{std::move(session.plan)};background.initialize(renderer);
            const auto mask=maps::make_sandbox_buildable_mask(background.plan(),geometry);
            const auto permissions=maps::load_sandbox_map_permissions(session.map,background.plan(),geometry,mask,2,checked.input_sha256);
            simulation::World world{permissions,simulation::RulesProfile::CityV16,4};
            report={{"schema","openemperor-empty-original-map-preflight-v1"},
                {"rules","sandbox-city-v16"},{"rule_version",4},{"policy_version",2},
                {"map",argv[2]},{"map_sha256",checked.input_sha256},
                {"map_permissions_sha256",persistence::map_permissions_fingerprint(*permissions,checked.input_sha256)},
                {"complete_policy",true},{"empty_city",true},{"tick",world.ticks()},
                {"funds",world.treasury()},{"building_cells",std::count(permissions->building_mask().begin(),permissions->building_mask().end(),std::uint8_t(1))},
                {"road_cells",std::count(permissions->road_mask().begin(),permissions->road_mask().end(),std::uint8_t(1))},
                {"decoded_assets",background.plan().decoded_assets},{"logical_texture_bytes",background.plan().logical_texture_bytes}};
        }
        if(StoredGraphicsRenderer::live_texture_count()!=0)throw std::runtime_error("preflight texture ownership leak");
        SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();
        std::cout<<report.dump(2)<<'\n';return 0;
    } catch(const std::exception& error) {
        if(renderer)SDL_DestroyRenderer(renderer);
        if(window)SDL_DestroyWindow(window);
        SDL_Quit();std::cerr<<"Empty-map preflight failed: "<<error.what()<<'\n';return 1;
    }
}
