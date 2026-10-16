#pragma once

#include "simulation/original/HouseEvolution.h"

#include <cstdint>

namespace openemperor::original {

// Authored API identities, not original actor IDs or World courier roles.
enum class ServiceRole : std::uint8_t {
    Herbalist = 0, Acupuncture = 1, Music = 2, Acrobat = 3, Drama = 4,
    Ancestor = 5, Confucian = 6, Daoist = 7, Buddhist = 8
};

enum class VisitAction : std::uint8_t {
    Herbalist = 0, Acupuncture = 1, Music = 2, Acrobat = 3, Drama = 4,
    Ancestor = 5, Confucian = 6, Daoist = 7, Buddhist = 8,
    AcrobatAndDrama = 9
};

enum class VisitPath : std::uint8_t {
    DirectService = 0, DirectReligion = 1, SharedWalker = 2
};

// Explicit original House observations absent from the pure OE-04 snapshot.
// access_enabled normalizes House byte +0x09 != 0; its producer is not inferred.
struct ServiceHouseContext {
    std::uint8_t original_status = 0;
    bool access_enabled = false;
};

// One already-resolved visit effect, not an entire original actor callback
// (which may combine service and several religion effects). No proximity,
// route, purchase or actor creation is inferred. elite_only is the resolved
// original religion-source condition, not a building-type assumption.
struct ServiceVisit {
    VisitPath path = VisitPath::DirectService;
    VisitAction action = VisitAction::Herbalist;
    bool elite_only = false;
};

[[nodiscard]] std::uint8_t service_duration(ServiceRole role);
[[nodiscard]] std::uint8_t service_timer(const HouseState& state, ServiceRole role);
[[nodiscard]] bool accepts_service_visit(const HouseState& state,
                                          ServiceHouseContext house,
                                          ServiceVisit visit);
// DirectService permits five single service actions; DirectReligion four
// religion actions. SharedWalker permits Music/Herbalist/Acupuncture, four
// religions and the paired AcrobatAndDrama action. Invalid combinations throw.
[[nodiscard]] HouseState apply_service_visit(const HouseState& state,
                                              ServiceHouseContext house,
                                              ServiceVisit visit);

// One explicit countdown operation: each nonzero byte decreases once.
// Neither function advances a clock or means one World::tick().
[[nodiscard]] HouseState decay_service_timers(const HouseState& state);
// Observed phase-35 caller gate: status 1 or 3, access byte nonzero; no
// occupancy gate. Evolution is separately dispatched later, at phase 39.
[[nodiscard]] HouseState decay_house_services(const HouseState& state,
                                               ServiceHouseContext house);

// Provider identity is separate from the nine access roles. Shrine and
// Temple/Pagoda variants have different schedules for the same religion.
enum class SpawnBuilding : std::uint8_t {
    Herbalist = 0, Acupuncture = 1, MusicSchool = 2, AcrobatSchool = 3,
    DramaSchool = 4, AncestralShrine = 5, DaoistShrine = 6,
    DaoistTemple = 7, BuddhistShrine = 8, BuddhistPagoda = 9,
    ConfucianAcademy = 10
};

struct ServiceSpawnStep {
    std::uint8_t counter = 0;
    bool attempt_due = false;
    bool operator==(const ServiceSpawnStep&) const = default;
};

// Original comparison threshold: from zero, due on eligible call threshold+1.
[[nodiscard]] std::uint8_t service_spawn_interval(SpawnBuilding building,
                                                  std::int32_t staffing);
// Staffing is the resolved virtual staffing value; the external gate includes
// all pre-counter prerequisites. Nonpositive staffing or a closed gate
// preserves the counter. Otherwise increment modulo 256, then compare strictly
// greater than the interval; a due attempt resets before any actor factory.
// attempt_due never establishes actor creation, arrival or House coverage.
[[nodiscard]] ServiceSpawnStep service_spawn_step(SpawnBuilding building,
                                                   std::int32_t staffing,
                                                   std::uint8_t counter,
                                                   bool evaluation_allowed);

} // namespace openemperor::original
