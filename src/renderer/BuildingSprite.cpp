#include "renderer/BuildingSprite.h"
#include "renderer/TextureCompatibility.h"
#include "core/PerformanceDiagnostics.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <stdexcept>

namespace openemperor {
namespace { std::size_t live_textures=0; }
std::size_t BuildingSprite::live_texture_count() { return live_textures; }
BuildingSprite::~BuildingSprite() { shutdown(); }
void BuildingSprite::initialize(SDL_Renderer* renderer,
                                const assets::BuildingVisualProfile& profile) {
    shutdown(); renderer_=renderer;
    try { append(profile); }
    catch (...) { shutdown();throw; }
}
void BuildingSprite::append(const assets::BuildingVisualProfile& profile,std::uint64_t max_final_rgba) {
    if (!renderer_ || profile.unique_images.size()<textures_.size())
        throw std::runtime_error("building texture append requires its existing image prefix");
    const auto limit=std::min(max_final_rgba,assets::building_max_rgba_bytes);
    std::uint64_t bytes=0;
    for (const auto& image:profile.unique_images) {
        if (image.pixels.size()>limit-bytes) throw std::runtime_error("building texture RGBA budget exceeded");
        bytes+=image.pixels.size();
    }
    // Reserve before creating resources. Publication then copies only pointers
    // into existing capacity; every earlier texture remains owned throughout.
    std::vector<SDL_Texture*> additional;
    additional.reserve(profile.unique_images.size()-textures_.size());
    textures_.reserve(profile.unique_images.size());
    try {
        for (std::size_t index=textures_.size();index<profile.unique_images.size();++index) {
            const auto& image=profile.unique_images[index];
            if (!image.width || !image.height || image.pixels.size()!=std::uint64_t(image.width)*image.height*4U)
                throw std::runtime_error("building texture dimensions differ from its RGBA payload");
            auto* texture=SDL_CreateTexture(renderer_,SDL_PIXELFORMAT_RGBA32,
                texture_compatibility::eager_rgba_access(renderer_),image.width,image.height);
            if (!texture) throw std::runtime_error(SDL_GetError());
            additional.push_back(texture);++live_textures;
            if (!SDL_UpdateTexture(texture,nullptr,image.pixels.data(),image.width*4) ||
                !SDL_SetTextureBlendMode(texture,SDL_BLENDMODE_BLEND) ||
                !SDL_SetTextureScaleMode(texture,SDL_SCALEMODE_NEAREST))
                throw std::runtime_error(SDL_GetError());
            performance::increment(performance::Counter::TextureUploads);
        }
    } catch (...) {
        for (auto* texture:additional) { SDL_DestroyTexture(texture);--live_textures; }
        throw;
    }
    textures_.insert(textures_.end(),additional.begin(),additional.end());
}
void BuildingSprite::shutdown() {
    for (auto* texture:textures_) { SDL_DestroyTexture(texture);--live_textures; }
    textures_.clear();
    renderer_=nullptr;
}
bool BuildingSprite::draw(scene::Point ground,double zoom,
                          const assets::BuildingVisualProfile& profile,
                          const assets::BuildingVisualEntry& entry,bool preview) const {
    const auto& image=profile.unique_images.at(entry.image_index);
    const SDL_FRect destination{
        static_cast<float>(ground.x-entry.ground_x*zoom),
        static_cast<float>(ground.y-entry.ground_y*zoom),
        static_cast<float>(image.width*zoom),static_cast<float>(image.height*zoom)};
    auto* texture=textures_.at(entry.image_index);
    if (preview && !SDL_SetTextureAlphaMod(texture,128)) return false;
    const bool drawn=SDL_RenderTexture(renderer_,texture,nullptr,&destination);
    const bool restored=!preview || SDL_SetTextureAlphaMod(texture,255);
    return drawn && restored;
}
}
