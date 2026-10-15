#include "simulation/original/HouseEvolution.h"

namespace openemperor::original {

bool is_house_type(int type) { return type >= house_type_first && type <= house_type_last; }
bool is_elite(int type) { return type >= HT_EliteUnoccupied && type <= house_type_last; }

int house_row(int type) { return type - 3 > 0 ? type - 3 : 0; }

int next_row(int type) {
    const int row = house_row(type);
    if (type == HT_EliteUnoccupied) return row + 2;           // skips Abandoned
    if (row == 14 || row == 8) return row;                    // top elite / unoccupied map to themselves
    return row + 1;
}

bool needs_met(const ModelData& m, int difficulty, int row, const HouseState& h) {
    auto v = [&](int f) { return m.house_value(row, f, difficulty); };
    if (v(HF_Water) && !h.water) return false;
    if (v(HF_Herbalist) && !h.herbalist) return false;
    if (v(HF_Acupuncture) && !h.acupuncture) return false;
    if (v(HF_Music) && !h.music) return false;
    if (v(HF_Acrobat) && !h.acrobat) return false;
    if (v(HF_Drama) && !h.drama) return false;
    if (v(HF_Ancestor) && !h.ancestor) return false;
    if (v(HF_Confucian) && !h.confucian) return false;
    if (v(HF_Buddhist) && !(h.daoist || h.buddhist)) return false;
    if (h.food_quality < v(HF_FoodQuality)) return false;
    if (h.hemp < v(HF_Hemp)) return false;
    if (h.ceramics < v(HF_Ceramics)) return false;
    if (h.tea < v(HF_Tea)) return false;
    if (h.silk < v(HF_Silk)) return false;
    if ((h.bronzeware > h.lacquerware ? h.bronzeware : h.lacquerware) < v(HF_Bronzeware)) return false;
    return true;
}

Verdict desirability_verdict(const ModelData& m, int difficulty, const HouseState& h) {
    const int row = house_row(h.type);
    if (h.desirability <= m.house_value(row, HF_DesLow, difficulty)) return Verdict::Devolve;
    if (h.desirability >= m.house_value(row, HF_DesHigh, difficulty)) return Verdict::Evolve;
    return Verdict::Stay;
}

Verdict combined_verdict(const ModelData& m, int difficulty, const HouseState& h) {
    Verdict v = h.type == HT_EliteAbandoned ? Verdict::Evolve  // desirability is ignored
                                            : desirability_verdict(m, difficulty, h);
    if (!needs_met(m, difficulty, house_row(h.type), h)) return Verdict::Devolve;
    if (v == Verdict::Evolve && !needs_met(m, difficulty, next_row(h.type), h)) return Verdict::Stay;
    return v;
}

int devolve_limit(int type) {
    if (type >= HT_Shelter && type <= HT_LuxuriousApartment) return 9;
    if (type == HT_EliteAbandoned) return 11;
    if (type == HT_ModestSiheyuan) return 49;
    if (type >= HT_LavishSiheyuan && type <= HT_HeavenlyCompound) return 17;
    return 0;
}

int evolve_target(int type) {
    if (type >= HT_Vacant && type <= HT_OrnateApartment) return type + 1;
    if (type >= HT_EliteAbandoned && type <= HT_ImpressiveCompound) return type + 1;
    return -1;  // top common, Elite Unoccupied, top elite
}

int devolve_target(int type) {
    if (type >= HT_Hut && type <= HT_LuxuriousApartment) return type - 1;
    if (type >= HT_ModestSiheyuan && type <= HT_HeavenlyCompound) return type - 1;
    return -1;  // Vacant, Shelter, Unoccupied, Abandoned
}

bool evolve_step(const ModelData& m, int difficulty, HouseState& h) {
    if (!is_house_type(h.type)) return false;
    if (h.type == HT_Vacant) {
        if (h.occupants > 0) { h.type = HT_Shelter; return true; }
        return false;
    }
    if (h.type == HT_EliteUnoccupied) return false;

    const Verdict v = combined_verdict(m, difficulty, h);
    if (v == Verdict::Evolve) {
        h.devolve_counter = 0;
        const int target = evolve_target(h.type);
        if (target < 0) return false;
        h.type = target;
        return true;
    }
    if (v == Verdict::Stay) { h.devolve_counter = 0; return false; }

    if (h.devolve_counter < devolve_limit(h.type)) { ++h.devolve_counter; return false; }
    h.devolve_counter = 0;
    const int target = devolve_target(h.type);
    if (target < 0) return false;
    h.type = target;
    return true;
}

}  // namespace openemperor::original
