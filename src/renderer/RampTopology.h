#pragma once
#include "simulation/Ramp.h"
#include <array>
#include <cmath>
#include <optional>
#include <span>
#include <stdexcept>

namespace openemperor {
// Presentation only: lateral neighbors never create a transport edge, change
// a RampLink, or grant a terrain permission. Positive is the renderer's near
// side: +Y for a canonical X link and +X for a canonical Y link.
inline constexpr unsigned ramp_negative_neighbor=1U;
inline constexpr unsigned ramp_positive_neighbor=2U;
inline bool ramp_canonical_slope(const simulation::RampLink& r) {
    const auto dx=std::int64_t(r.endpoint_b.x)-r.endpoint_a.x,dy=std::int64_t(r.endpoint_b.y)-r.endpoint_a.y;
    return ((dx==1&&dy==0)||(dy==1&&dx==0))&&
           std::abs(int(r.height_b)-int(r.height_a))==1;
}
inline simulation::Cell ramp_lateral_step(const simulation::RampLink& r) {
    return r.endpoint_a.x!=r.endpoint_b.x?simulation::Cell{0,1}:simulation::Cell{1,0};
}
inline bool ramp_lateral_neighbor(const simulation::RampLink& r,
                                  const simulation::RampLink& other,int side) {
    if((side!=-1&&side!=1)||!ramp_canonical_slope(r)||!ramp_canonical_slope(other)||
       r.height_a!=other.height_a||r.height_b!=other.height_b)return false;
    const auto step=ramp_lateral_step(r);
    return std::int64_t(other.endpoint_a.x)-r.endpoint_a.x==step.x*side&&
           std::int64_t(other.endpoint_a.y)-r.endpoint_a.y==step.y*side&&
           std::int64_t(other.endpoint_b.x)-r.endpoint_b.x==step.x*side&&
           std::int64_t(other.endpoint_b.y)-r.endpoint_b.y==step.y*side;
}

// Rebuilt only on load, paid commands, or a coalesced preview change. The input
// predicate is where the owner supplies complete source/permission eligibility;
// the default overload is for links already validated by production World.
class RampTopology {
public:
    template<class Eligible>
    void prepare(std::span<const simulation::RampLink> links,
                 const std::optional<simulation::RampLink>& preview,Eligible eligible) {
        if(links.size()>simulation::ramp_max_links)
            throw std::invalid_argument("ramp presentation topology bound exceeded");
        count_=0;preview_.reset();committed_masks_.fill(0);effective_masks_.fill(0);
        for(const auto& r:links)if(ramp_canonical_slope(r)&&eligible(r)) {
            if(r.id.value==0||r.id.value>simulation::ramp_max_links)
                throw std::invalid_argument("ramp presentation identity outside V1 bound");
            bool duplicate=false;
            for(std::size_t i=0;i<count_;++i)
                if(entries_[i].link.id==r.id||same_edge(entries_[i].link,r))duplicate=true;
            if(!duplicate)entries_[count_++]={r,0,0};
        }
        if(preview&&preview->id.value>0&&preview->id.value<=simulation::ramp_max_links&&
           ramp_canonical_slope(*preview)&&eligible(*preview)) {
            bool shares_endpoint=false;
            for(std::size_t i=0;i<count_;++i) {
                const auto& r=entries_[i].link;
                if(r.id==preview->id||r.endpoint_a==preview->endpoint_a||r.endpoint_a==preview->endpoint_b||
                   r.endpoint_b==preview->endpoint_a||r.endpoint_b==preview->endpoint_b)shares_endpoint=true;
            }
            if(!shares_endpoint)preview_=Entry{*preview,0,0};
        }
        for(std::size_t i=0;i<count_;++i) {
            auto& entry=entries_[i];
            for(std::size_t j=0;j<count_;++j)if(i!=j)
                entry.committed|=neighbor_mask(entry.link,entries_[j].link);
            entry.effective=entry.committed;
            if(preview_)entry.effective|=neighbor_mask(entry.link,preview_->link);
            committed_masks_[entry.link.id.value]=std::uint8_t(entry.committed);
            effective_masks_[entry.link.id.value]=std::uint8_t(entry.effective);
        }
        if(preview_)for(std::size_t i=0;i<count_;++i)
            preview_->effective|=neighbor_mask(preview_->link,entries_[i].link);
    }
    void prepare(std::span<const simulation::RampLink> links,
                 const std::optional<simulation::RampLink>& preview=std::nullopt) {
        prepare(links,preview,[](const auto&){return true;});
    }
    unsigned committed_mask(simulation::RampId id) const {
        return id.value<committed_masks_.size()?committed_masks_[id.value]:0;
    }
    unsigned effective_mask(simulation::RampId id) const {
        return id.value<effective_masks_.size()?effective_masks_[id.value]:0;
    }
    bool has_preview() const { return preview_.has_value(); }
    unsigned preview_mask() const { return preview_?preview_->effective:0; }
private:
    struct Entry { simulation::RampLink link{};unsigned committed=0,effective=0; };
    static bool same_edge(const simulation::RampLink& a,const simulation::RampLink& b) {
        return a.endpoint_a==b.endpoint_a&&a.endpoint_b==b.endpoint_b;
    }
    static unsigned neighbor_mask(const simulation::RampLink& a,const simulation::RampLink& b) {
        return (ramp_lateral_neighbor(a,b,-1)?ramp_negative_neighbor:0U)|
               (ramp_lateral_neighbor(a,b,1)?ramp_positive_neighbor:0U);
    }
    std::array<Entry,simulation::ramp_max_links> entries_{};
    std::array<std::uint8_t,simulation::ramp_max_links+1> committed_masks_{},effective_masks_{};
    std::size_t count_=0;
    std::optional<Entry> preview_;
};
} // namespace openemperor
