#pragma once
#include "assets/BuildingVisualProfile.h"
#include "scene/IsoProjection.h"
#include <cstddef>
#include <vector>
struct SDL_Renderer;
struct SDL_Texture;

namespace openemperor {
class BuildingSprite {
public:
    BuildingSprite()=default;
    BuildingSprite(const BuildingSprite&)=delete;
    BuildingSprite& operator=(const BuildingSprite&)=delete;
    ~BuildingSprite();
    void initialize(SDL_Renderer* renderer,const assets::BuildingVisualProfile& profile);
    // Load-time extension only. The caller retains an identical already
    // uploaded image prefix; failure leaves all existing textures intact.
    void append(const assets::BuildingVisualProfile& profile,
                std::uint64_t max_final_rgba=assets::building_max_rgba_bytes);
    void shutdown();
    bool draw(scene::Point ground,double zoom,const assets::BuildingVisualProfile& profile,
              const assets::BuildingVisualEntry& entry,bool preview=false) const;
    std::size_t texture_count() const { return textures_.size(); }
    static std::size_t live_texture_count();
private:
    SDL_Renderer* renderer_=nullptr;
    std::vector<SDL_Texture*> textures_;
};
}
