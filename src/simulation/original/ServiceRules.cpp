#include "simulation/original/ServiceRules.h"

#include <array>
#include <stdexcept>

namespace openemperor::original {
namespace {

constexpr std::array<std::uint8_t HouseState::*, 9> timers{
    &HouseState::herbalist, &HouseState::acupuncture, &HouseState::music,
    &HouseState::acrobat, &HouseState::drama, &HouseState::ancestor,
    &HouseState::confucian, &HouseState::daoist, &HouseState::buddhist
};

void validate_house(const HouseState& state) {
    if (!is_house_type(state.type)) throw std::out_of_range("invalid original service House type");
}

std::size_t role_index(ServiceRole role) {
    const auto index = static_cast<std::size_t>(role);
    if (index >= timers.size()) throw std::out_of_range("invalid original service role");
    return index;
}

bool religious(VisitAction action) noexcept {
    return action >= VisitAction::Ancestor && action <= VisitAction::Buddhist;
}

void validate_visit(ServiceVisit visit) {
    if (visit.path > VisitPath::SharedWalker || visit.action > VisitAction::AcrobatAndDrama)
        throw std::out_of_range("invalid original service visit identity");
    if ((visit.path == VisitPath::DirectService && visit.action > VisitAction::Drama) ||
        (visit.path == VisitPath::DirectReligion && !religious(visit.action)) ||
        (visit.path == VisitPath::SharedWalker &&
         (visit.action == VisitAction::Acrobat || visit.action == VisitAction::Drama))) {
        throw std::invalid_argument("service action is unavailable in this original visit path");
    }
    if (visit.elite_only && !religious(visit.action))
        throw std::invalid_argument("elite-only source condition applies to religion effects");
}

bool active_status(ServiceHouseContext house) noexcept {
    return (house.original_status == 1 || house.original_status == 3) && house.access_enabled;
}

using Schedule = std::array<std::uint8_t, 6>;
const Schedule& schedule(SpawnBuilding building) {
    constexpr static Schedule herbalist{1, 3, 5, 10, 15, 64};
    constexpr static Schedule acupuncture{1, 3, 7, 15, 29, 64};
    constexpr static Schedule school_or_shrine{3, 6, 12, 24, 32, 64};
    constexpr static Schedule drama{6, 12, 24, 32, 48, 96};
    constexpr static Schedule advanced_religion{6, 12, 24, 32, 48, 64};
    switch (building) {
    case SpawnBuilding::Herbalist: return herbalist;
    case SpawnBuilding::Acupuncture: return acupuncture;
    case SpawnBuilding::MusicSchool:
    case SpawnBuilding::AcrobatSchool:
    case SpawnBuilding::AncestralShrine:
    case SpawnBuilding::DaoistShrine:
    case SpawnBuilding::BuddhistShrine: return school_or_shrine;
    case SpawnBuilding::DramaSchool: return drama;
    case SpawnBuilding::DaoistTemple:
    case SpawnBuilding::BuddhistPagoda:
    case SpawnBuilding::ConfucianAcademy: return advanced_religion;
    }
    throw std::out_of_range("invalid original service spawn building");
}

} // namespace

std::uint8_t service_duration(ServiceRole role) {
    return role_index(role) < 5 ? std::uint8_t{96} : std::uint8_t{40};
}

std::uint8_t service_timer(const HouseState& state, ServiceRole role) {
    validate_house(state);
    return state.*timers[role_index(role)];
}

bool accepts_service_visit(const HouseState& state, ServiceHouseContext house,
                            ServiceVisit visit) {
    validate_house(state);
    validate_visit(visit);
    if (!house.access_enabled) return false;
    if (visit.path != VisitPath::SharedWalker && !active_status(house)) return false;
    if (visit.path == VisitPath::DirectService) return state.occupants > 0;
    const bool elite = is_elite(state.type);
    return (state.occupants > 0 || elite) && (!visit.elite_only || elite);
}

HouseState apply_service_visit(const HouseState& state, ServiceHouseContext house,
                               ServiceVisit visit) {
    if (!accepts_service_visit(state, house, visit)) return state;
    auto next = state;
    if (visit.action == VisitAction::AcrobatAndDrama) {
        next.acrobat = 96;
        next.drama = 96;
    } else {
        const auto role = static_cast<ServiceRole>(visit.action);
        next.*timers[role_index(role)] = service_duration(role);
    }
    return next;
}

HouseState decay_service_timers(const HouseState& state) {
    validate_house(state);
    auto next = state;
    for (const auto timer : timers) {
        if (next.*timer != 0) --(next.*timer);
    }
    return next;
}

HouseState decay_house_services(const HouseState& state, ServiceHouseContext house) {
    validate_house(state);
    return active_status(house) ? decay_service_timers(state) : state;
}

std::uint8_t service_spawn_interval(SpawnBuilding building, std::int32_t staffing) {
    const auto& values = schedule(building);
    const std::size_t tier = staffing >= 100 ? 0U : staffing >= 75 ? 1U :
                             staffing >= 50 ? 2U : staffing >= 25 ? 3U :
                             staffing >= 1 ? 4U : 5U;
    return values[tier];
}

ServiceSpawnStep service_spawn_step(SpawnBuilding building, std::int32_t staffing,
                                    std::uint8_t counter, bool evaluation_allowed) {
    const auto interval = service_spawn_interval(building, staffing);
    if (!evaluation_allowed || staffing <= 0) return {counter, false};
    // Defined narrowing reproduces the observed incb wrap before comparison.
    const auto next = static_cast<std::uint8_t>(static_cast<unsigned>(counter) + 1U);
    return next > interval ? ServiceSpawnStep{0, true} : ServiceSpawnStep{next, false};
}

} // namespace openemperor::original
