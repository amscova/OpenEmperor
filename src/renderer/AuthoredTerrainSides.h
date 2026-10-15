#pragma once
#include "maps/StoredGraphicsPlan.h"
#include "simulation/MapPermissions.h"
#include "simulation/Ramp.h"
#include <span>
struct SDL_Renderer;
namespace openemperor {
// Explicit authored preview only. These faces never grant a cell/edge right,
// select original cliff artwork, or enter World/SaveDocument authority.
class AuthoredTerrainSides {
public:
    void prepare(const maps::StoredGraphicsPlan&,const maps::MapGeometry&,
                 const simulation::MapPermissions&);
    void clear();
    std::size_t size() const { return faces_.size(); }
    void set_ramps(std::span<const simulation::RampLink>);
    void set_preview(const std::optional<simulation::RampLink>&);
    std::size_t open_edges() const { return committed_.size(); }
    std::size_t logical_geometry_bytes() const;
    bool draw(SDL_Renderer*,const scene::Camera2D&,bool openings=true) const;
private:
    struct Run { std::int8_t x=0,y=0;std::uint8_t width=0,r=0,g=0,b=0,a=0; };
    struct Instance {
        scene::Point ground;simulation::Cell high,low;std::int8_t height=0;
        unsigned direction=0,start_degree=0,end_degree=0;
        std::vector<Run> closed,opened;bool cut=false;
    };
    std::optional<std::size_t> edge_for(const simulation::RampLink&) const;
    std::vector<Instance> faces_;
    std::vector<std::pair<std::uint32_t,std::size_t>> edge_index_;
    std::vector<std::size_t> committed_;
    std::optional<std::size_t> preview_;
};
} // namespace openemperor
