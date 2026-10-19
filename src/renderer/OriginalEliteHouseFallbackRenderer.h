#pragma once
#include "scene/IsoProjection.h"
struct SDL_Renderer;
namespace openemperor {
// Authored low courtyard on the actual 4x4 footprint. Coordinates derive from
// OpenEmperor's front cell, never from an inferred Original image anchor.
bool draw_original_elite_house_fallback(SDL_Renderer*,scene::Point ground,double zoom);
bool hit_original_elite_house_fallback(scene::Point screen,scene::Point ground,double zoom);
}
