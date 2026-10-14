#pragma once
#include "simulation/Ramp.h"
#include "renderer/BridgeRenderer.h"
struct SDL_Renderer;
namespace openemperor {
// Bounded independently authored slope components; no textures or original pixels.
bool draw_ramp_part(SDL_Renderer*,const scene::Camera2D&,scene::Point endpoint_a,
    const simulation::RampLink&,int half,bool front,bool elevated,BridgeTint=BridgeTint::Normal);
bool hit_ramp_part(scene::Point world,scene::Point endpoint_a,const simulation::RampLink&,
    int half,bool front,bool elevated);
}
