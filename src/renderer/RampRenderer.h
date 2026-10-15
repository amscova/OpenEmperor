#pragma once
#include "simulation/Ramp.h"
#include "renderer/BridgeRenderer.h"
#include <cmath>
#include <memory>
#include <cstddef>
struct SDL_Renderer;
namespace openemperor {
namespace assets { struct RoadVisualProfile; }
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
// The shoulder is graded with the entire ramp, not either half's undisturbed
// plateau. Only the surrounding cliff keeps the original vertical terrain step.
inline scene::Point ramp_bank_point(scene::Point a,const simulation::RampLink& r,
                                   double t,double side) {
    return ramp_surface_point(a,r,t,side<0?-.46:.46);
}
inline bool ramp_terrain_opening(double side,double depth,unsigned adjacency=0) {
    return side>=((adjacency&1U)?-.5:-ramp_surface_half_width)&&
        side<((adjacency&2U)?.5:ramp_surface_half_width)&&depth>=0&&depth<20.;
}
// Session-local material preparation. Adjacency bits 1/2 mean a matching
// negative/positive perpendicular RampLink (X uses Y; Y uses X). These bits
// select presentation only; they never introduce transport connectivity.
class RampPresentation {
public:
    RampPresentation();
    ~RampPresentation();
    RampPresentation(RampPresentation&&) noexcept;
    RampPresentation& operator=(RampPresentation&&) noexcept;
    RampPresentation(const RampPresentation&)=delete;
    RampPresentation& operator=(const RampPresentation&)=delete;
    void prepare(const assets::RoadVisualProfile* profile=nullptr);
    void clear();
    std::size_t geometry_bytes() const;
    bool draw_part(SDL_Renderer*,const scene::Camera2D&,scene::Point endpoint_a,
        const simulation::RampLink&,int half,bool front,bool elevated,
        BridgeTint tint=BridgeTint::Normal,unsigned adjacency=0,bool road_material=true) const;
    bool hit_part(scene::Point world,scene::Point endpoint_a,const simulation::RampLink&,
        int half,bool front,bool elevated,unsigned adjacency=0) const;
private:
    struct Data;
    std::unique_ptr<Data> data_;
};
// Bounded independently authored slope components; no textures or original pixels.
bool draw_ramp_part(SDL_Renderer*,const scene::Camera2D&,scene::Point endpoint_a,
    const simulation::RampLink&,int half,bool front,bool elevated,BridgeTint=BridgeTint::Normal);
bool hit_ramp_part(scene::Point world,scene::Point endpoint_a,const simulation::RampLink&,
    int half,bool front,bool elevated);
// Prepared authored geometry coverage, used only while preparing cliff cuts.
bool ramp_opaque_at(scene::Point world,scene::Point endpoint_a,const simulation::RampLink&,
    unsigned adjacency=0);
}
