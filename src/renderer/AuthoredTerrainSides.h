#pragma once
#include "maps/StoredGraphicsPlan.h"
#include "simulation/MapPermissions.h"
struct SDL_Renderer;
namespace openemperor {
// Explicit authored preview only. These faces never grant a cell/edge right,
// select original cliff artwork, or enter World/SaveDocument authority.
class AuthoredTerrainSides {
public:
    void prepare(const maps::StoredGraphicsPlan&,const maps::MapGeometry&,
                 const simulation::MapPermissions&);
    void clear() { faces_.clear(); }
    std::size_t size() const { return faces_.size(); }
    bool draw(SDL_Renderer*,const scene::Camera2D&) const;
private:
    struct Instance { scene::Point ground; unsigned direction=0; };
    std::vector<Instance> faces_;
};
} // namespace openemperor
