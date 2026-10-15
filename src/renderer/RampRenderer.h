#pragma once
#include "simulation/Ramp.h"
#include "renderer/BridgeRenderer.h"
#include <cmath>
struct SDL_Renderer;
namespace openemperor {
// Shared authored road/cutout cross-section, never transport/save authority.
inline constexpr double ramp_surface_half_width=.30;
inline double ramp_surface_width(double t){return .26+.04*std::sin(t*3.141592653589793);}
inline scene::Point ramp_surface_point(scene::Point a,const simulation::RampLink& r,
                                      double t,double side,bool elevated=true) {
    const bool x=r.endpoint_a.x!=r.endpoint_b.x;
    const int delta=elevated?int(r.height_b)-int(r.height_a):0;
    return {a.x+(x?40.:-40.)*t+(x?-40.:40.)*side,
            a.y+(20.-delta*40.)*t+20.*side};
}
inline bool ramp_terrain_opening(double side,double depth) {
    return std::abs(side)<ramp_surface_half_width&&depth>=0&&depth<20.;
}
// Bounded independently authored slope components; no textures or original pixels.
bool draw_ramp_part(SDL_Renderer*,const scene::Camera2D&,scene::Point endpoint_a,
    const simulation::RampLink&,int half,bool front,bool elevated,BridgeTint=BridgeTint::Normal);
bool hit_ramp_part(scene::Point world,scene::Point endpoint_a,const simulation::RampLink&,
    int half,bool front,bool elevated);
// Prepared authored geometry coverage, used only while preparing cliff cuts.
bool ramp_opaque_at(scene::Point world,scene::Point endpoint_a,const simulation::RampLink&);
}
