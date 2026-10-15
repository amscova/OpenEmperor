#pragma once
// Original-style house evolution: pure rules over a ModelData table.
// Services the host game does not model yet are simply reported as absent.
#include "simulation/original/ModelData.h"

#include <cstdint>

namespace openemperor::original {

inline constexpr int house_type_first = 2;   // Vacant
inline constexpr int house_type_last = 17;   // Heavenly Compound
enum HouseType : int {
    HT_Vacant = 2, HT_Shelter, HT_Hut, HT_PlainCottage, HT_AttractiveCottage,
    HT_SpaciousDwelling, HT_ElegantDwelling, HT_OrnateApartment, HT_LuxuriousApartment,
    HT_EliteUnoccupied, HT_EliteAbandoned, HT_ModestSiheyuan, HT_LavishSiheyuan,
    HT_HumbleCompound, HT_ImpressiveCompound, HT_HeavenlyCompound
};

// What a house currently has. Anything the host does not simulate stays false/0.
struct HouseState {
    int type = HT_Shelter;
    int desirability = 0;
    int occupants = 0;
    int devolve_counter = 0;

    bool water = false;
    // Service access is a countdown: a visit sets it to service_timer_start and
    // decay_services() lowers it once per cycle. Nonzero means access.
    int herbalist = 0, acupuncture = 0, music = 0, acrobat = 0, drama = 0;
    int ancestor = 0, confucian = 0, daoist = 0, buddhist = 0;
    int food_quality = 0;
    int hemp = 0, ceramics = 0, tea = 0, silk = 0, bronzeware = 0, lacquerware = 0;
};

inline constexpr int service_timer_start = 96;
inline constexpr int religion_timer_start = 40;
void grant_service(int& timer);        // a visit: timer = 96
void grant_religion(int& timer);       // a religious visit: timer = 40
void decay_services(HouseState& h);    // once per cycle (phase 35 in the original)

// Updates a service building waits between walkers, from its staffing level
// (a percentage-like value). No workers at all means 64.
int service_spawn_interval(int staffing);
// Religious buildings use their own, slower tier table.
int religion_spawn_interval(int staffing);
// Counter step: returns true when a walker should be sent this update.
bool service_spawn_tick(int& counter, int staffing);

// Time: one step = one 51-phase pass; 16 steps = a month, 12 months = a year.
inline constexpr int steps_per_month = 16;
inline constexpr int months_per_year = 12;
// Goods are consumed on steps 0 and 7 of each month.
bool is_consumption_step(int step_in_month);
// Subtracts each goods requirement of the house's row from its stocks (floor 0).
// Does nothing for a house with no occupants.
void consume_goods(const ModelData& m, int difficulty, HouseState& h);

enum class Verdict : int { Devolve = -1, Stay = 0, Evolve = 1 };

bool is_house_type(int type);
int house_row(int type);   // max(type - 3, 0)
int next_row(int type);    // row whose needs must be met to evolve
bool is_elite(int type);

// Every requirement of `row` is satisfied by the house.
bool needs_met(const ModelData& m, int difficulty, int row, const HouseState& h);
// Desirability-only verdict for the current row.
Verdict desirability_verdict(const ModelData& m, int difficulty, const HouseState& h);
// Combined verdict as the original computes it (desirability + needs).
Verdict combined_verdict(const ModelData& m, int difficulty, const HouseState& h);

// Consecutive devolve verdicts tolerated before the next one demotes (limit).
int devolve_limit(int type);
// Target type for a given step, or -1 when the step has no effect.
int evolve_target(int type);
int devolve_target(int type);

// One evaluation, run once per cycle. Updates h.type / h.devolve_counter and
// returns true when the type changed.
bool evolve_step(const ModelData& m, int difficulty, HouseState& h);

}  // namespace openemperor::original
