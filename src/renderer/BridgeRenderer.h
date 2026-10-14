#pragma once
#include "simulation/Bridge.h"
#include "scene/IsoProjection.h"
struct SDL_Renderer;
namespace openemperor {
enum class BridgePart { DeckAndBackRail, FrontRail };
enum class BridgeTint { Normal, Valid, Unaffordable, Blocked };
// Authored timber construction. All geometry is bounded code, no textures.
bool draw_bridge_cell(SDL_Renderer*,const scene::Camera2D&,scene::Point ground,
    const simulation::BridgeSpan&,simulation::Cell,BridgePart,BridgeTint=BridgeTint::Normal);
bool hit_bridge_cell(scene::Point world,scene::Point ground,
    const simulation::BridgeSpan&,simulation::Cell);
bool hit_bridge_part(scene::Point world,scene::Point ground,const simulation::BridgeSpan&,
    simulation::Cell,BridgePart,double line_tolerance);
}
