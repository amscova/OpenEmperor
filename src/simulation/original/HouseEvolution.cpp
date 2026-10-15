#include "simulation/original/HouseEvolution.h"

namespace openemperor::original {

void grant_service(int& timer) { timer = service_timer_start; }

void grant_religion(int& timer) { timer = religion_timer_start; }

void decay_services(HouseState& h) {
    for (int* t : {&h.herbalist, &h.acupuncture, &h.music, &h.acrobat, &h.drama,
                   &h.ancestor, &h.confucian, &h.daoist, &h.buddhist})
        *t = *t < 2 ? 0 : *t - 1;
}

int service_spawn_interval(int staffing) {
    if (staffing >= 100) return 1;
    if (staffing >= 75) return 3;
    if (staffing >= 50) return 7;
    if (staffing >= 25) return 15;
    return staffing >= 1 ? 29 : 64;
}

int religion_spawn_interval(int staffing) {
    if (staffing >= 100) return 3;
    if (staffing >= 75) return 6;
    if (staffing >= 50) return 12;
    if (staffing >= 25) return 24;
    return staffing >= 1 ? 32 : 64;
}

bool service_spawn_tick(int& counter, int staffing) {
    ++counter;
    if (counter > service_spawn_interval(staffing)) { counter = 0; return true; }
    return false;
}

bool is_consumption_step(int step_in_month) { return step_in_month == 0 || step_in_month == 7; }

void consume_goods(const ModelData& m, int difficulty, HouseState& h) {
    if (h.occupants <= 0 || !is_house_type(h.type)) return;
    const int row = house_row(h.type);
    auto use = [&](int& stock, int field) {
        const int amount = m.house_value(row, field, difficulty);
        if (amount > 0) stock = stock < amount ? 0 : stock - amount;
    };
    use(h.hemp, HF_Hemp);
    use(h.ceramics, HF_Ceramics);
    use(h.tea, HF_Tea);
    use(h.silk, HF_Silk);
    use(h.bronzeware, HF_Bronzeware);
    use(h.lacquerware, HF_Bronzeware);
}

int& goods_stock(HouseState& h, Goods g) {
    switch (g) {
    case Goods::Hemp: return h.hemp;
    case Goods::Tea: return h.tea;
    case Goods::Ceramics: return h.ceramics;
    case Goods::Bronzeware: return h.bronzeware;
    case Goods::Lacquerware: return h.lacquerware;
    default: return h.silk;
    }
}

int goods_need(const ModelData& m, int difficulty, const HouseState& h, Goods g) {
    if (h.occupants <= 0 && (g == Goods::Hemp || g == Goods::Tea)) return 10;
    int field = HF_Silk;
    switch (g) {
    case Goods::Hemp: field = HF_Hemp; break;
    case Goods::Tea: field = HF_Tea; break;
    case Goods::Ceramics: field = HF_Ceramics; break;
    case Goods::Bronzeware:
    case Goods::Lacquerware: field = HF_Bronzeware; break;
    default: break;
    }
    const int need = m.house_value(house_row(h.type), field, difficulty);
    return h.size >= 2 ? need * 4 : need;
}

int deliver_goods(const ModelData& m, int difficulty, HouseState& h, Goods g, int& market_stock) {
    const int need = goods_need(m, difficulty, h, g);
    if (need <= 0 || market_stock <= 0) return 0;
    int& stock = goods_stock(h, g);
    const int target = need > 10 ? need : 10;
    int amount = target - stock;
    if (amount < 1 || amount > need) amount = need - stock;
    if (amount < 0) amount = 0;
    if (amount > target) amount = target;
    if (amount > need) amount = need;
    if (amount > market_stock) amount = market_stock;  // port: never move more than the market holds
    if (amount <= 0) return 0;
    stock += amount;
    market_stock -= amount;
    return amount;
}

int blend_food_quality(int q, int m, int delivered, int food_need) {
    if (m >= 1 && m > q) return m;
    if (delivered < 1) return q;
    const double r = food_need < 1 ? 10.0 : static_cast<double>(delivered) / food_need;
    if (r > 3.0) return (q + 3 * m) / 4;
    if (r > 2.0) return (q + 2 * m) / 3;
    if (r > 0.5) return (q + m) / 2;
    if (r > 0.33) return (m + 2 * q) / 3;
    return (m + 3 * q) / 4;
}

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
    if (v(HF_Herbalist) && h.herbalist<=0) return false;
    if (v(HF_Acupuncture) && h.acupuncture<=0) return false;
    if (v(HF_Music) && h.music<=0) return false;
    if (v(HF_Acrobat) && h.acrobat<=0) return false;
    if (v(HF_Drama) && h.drama<=0) return false;
    if (v(HF_Ancestor) && h.ancestor<=0) return false;
    if (v(HF_Confucian) && h.confucian<=0) return false;
    if (v(HF_Buddhist) && h.daoist<=0 && h.buddhist<=0) return false;
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

bool accepts_service_visit(const HouseState& h) {
    return is_house_type(h.type) && (h.occupants > 0 || is_elite(h.type));
}

bool apply_service_visit(HouseState& h, ServiceKind kind) {
    if (!accepts_service_visit(h)) return false;
    switch (kind) {
    case ServiceKind::Herbalist: grant_service(h.herbalist); break;
    case ServiceKind::Acupuncture: grant_service(h.acupuncture); break;
    case ServiceKind::Music: grant_service(h.music); break;
    case ServiceKind::Acrobat: grant_service(h.acrobat); break;
    case ServiceKind::Drama: grant_service(h.drama); break;
    case ServiceKind::Ancestor: grant_religion(h.ancestor); break;
    case ServiceKind::Confucian: grant_religion(h.confucian); break;
    case ServiceKind::Daoist: grant_religion(h.daoist); break;
    case ServiceKind::Buddhist: grant_religion(h.buddhist); break;
    }
    return true;
}

bool step_house(const ModelData& m, int difficulty, HouseState& h, int step_in_month) {
    // Order as in the original: timers decay, then the evolution check, then
    // goods consumption on its own steps.
    decay_services(h);
    const bool changed = evolve_step(m, difficulty, h);
    if (is_consumption_step(step_in_month)) consume_goods(m, difficulty, h);
    return changed;
}

}  // namespace openemperor::original
