#include "simulation/original/ServiceRules.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {
namespace original = openemperor::original;
namespace fs = std::filesystem;
using original::HouseField;
using original::HouseState;
using original::HouseType;
using original::ServiceHouseContext;
using original::ServiceRole;
using original::ServiceVisit;
using original::SpawnBuilding;
using original::VisitAction;
using original::VisitPath;

std::size_t checks = 0;
void check(bool condition, std::string_view message) {
    ++checks;
    if (!condition) throw std::runtime_error(std::string(message));
}

template<typename Error, typename Operation>
void expect_exception(Operation operation, std::string_view message) {
    try { operation(); }
    catch (const Error&) { ++checks; return; }
    throw std::runtime_error(std::string(message));
}

struct Role {
    ServiceRole role;
    VisitAction action;
    std::uint8_t HouseState::*timer;
    std::uint8_t duration;
    HouseField need;
};
// Independently listed access fields and observed grant constants. These API
// roles are not the original actor IDs, which may combine several effects.
constexpr std::array<Role, 9> roles{{
    {ServiceRole::Herbalist, VisitAction::Herbalist, &HouseState::herbalist, 96, HouseField::Herbalist},
    {ServiceRole::Acupuncture, VisitAction::Acupuncture, &HouseState::acupuncture, 96, HouseField::Acupuncture},
    {ServiceRole::Music, VisitAction::Music, &HouseState::music, 96, HouseField::Music},
    {ServiceRole::Acrobat, VisitAction::Acrobat, &HouseState::acrobat, 96, HouseField::Acrobat},
    {ServiceRole::Drama, VisitAction::Drama, &HouseState::drama, 96, HouseField::Drama},
    {ServiceRole::Ancestor, VisitAction::Ancestor, &HouseState::ancestor, 40, HouseField::Ancestor},
    {ServiceRole::Confucian, VisitAction::Confucian, &HouseState::confucian, 40, HouseField::Confucian},
    {ServiceRole::Daoist, VisitAction::Daoist, &HouseState::daoist, 40, HouseField::DaoistOrBuddhist},
    {ServiceRole::Buddhist, VisitAction::Buddhist, &HouseState::buddhist, 40, HouseField::DaoistOrBuddhist}
}};

HouseState distinct_state() {
    HouseState state;
    state.type = HouseType::Hut;
    state.desirability = -27;
    state.occupants = 13;
    state.devolve_counter = 19;
    state.water = true;
    for (std::size_t i = 0; i < roles.size(); ++i)
        state.*(roles[i].timer) = static_cast<std::uint8_t>(i * 23 + 7);
    state.food_quality = 179;
    state.hemp = 18;
    state.ceramics = 27;
    state.tea = -2;
    state.silk = 101;
    state.bronzeware = 8;
    state.lacquerware = 91;
    return state;
}

HouseState supplied(HouseType type = HouseType::Hut) {
    auto state = distinct_state();
    state.type = type;
    state.occupants = 1;
    for (const auto& role : roles) state.*(role.timer) = 255;
    state.food_quality = 255;
    state.hemp = state.ceramics = state.tea = state.silk = 32767;
    state.bronzeware = state.lacquerware = 32767;
    return state;
}

bool religion(VisitAction action) {
    return action == VisitAction::Ancestor || action == VisitAction::Confucian ||
           action == VisitAction::Daoist || action == VisitAction::Buddhist;
}

ServiceVisit direct_visit(const Role& role) {
    return {role.duration == 40 ? VisitPath::DirectReligion : VisitPath::DirectService,
            role.action, false};
}

HouseState reference_grant(HouseState state, VisitAction action) {
    if (action == VisitAction::AcrobatAndDrama) {
        state.acrobat = 96;
        state.drama = 96;
    } else {
        for (const auto& role : roles)
            if (role.action == action) state.*(role.timer) = role.duration;
    }
    return state;
}

bool reference_accepts(const HouseState& state, ServiceHouseContext house,
                       ServiceVisit visit) {
    if (!house.access_enabled) return false;
    const bool elite = state.type >= HouseType::EliteUnoccupied;
    if (visit.path != VisitPath::SharedWalker &&
        house.original_status != 1 && house.original_status != 3) return false;
    if (visit.path == VisitPath::DirectService) return state.occupants > 0;
    return (state.occupants > 0 || elite) && (!visit.elite_only || elite);
}

std::vector<ServiceVisit> visits() {
    std::vector<ServiceVisit> result;
    for (const auto& role : roles) {
        result.push_back(direct_visit(role));
        if (role.duration == 40) {
            result.push_back({VisitPath::DirectReligion, role.action, true});
            result.push_back({VisitPath::SharedWalker, role.action, false});
            result.push_back({VisitPath::SharedWalker, role.action, true});
        } else if (role.action != VisitAction::Acrobat && role.action != VisitAction::Drama) {
            result.push_back({VisitPath::SharedWalker, role.action, false});
        }
    }
    result.push_back({VisitPath::SharedWalker, VisitAction::AcrobatAndDrama, false});
    return result;
}

void visits_and_admission() {
    const auto cases = visits();
    check(cases.size() == 25, "all independently resolved legal visit/action combinations covered");
    for (std::uint16_t type = 2; type <= 17; ++type) {
        for (const auto occupants : std::array<std::int16_t, 5>{-32768, -1, 0, 1, 32767}) {
            for (const auto status : std::array<std::uint8_t, 6>{0, 1, 2, 3, 4, 255}) {
                for (const auto access : {false, true}) {
                    auto state = distinct_state();
                    state.type = static_cast<HouseType>(type);
                    state.occupants = occupants;
                    const auto before = state;
                    const ServiceHouseContext house{status, access};
                    for (const auto visit : cases) {
                        const bool accepted = reference_accepts(state, house, visit);
                        check(original::accepts_service_visit(state, house, visit) == accepted,
                              "path-specific occupancy, elite, status and access predicates");
                        const auto expected = accepted ? reference_grant(state, visit.action) : state;
                        check(original::apply_service_visit(state, house, visit) == expected,
                              "resolved visit changes exactly its selected access bytes or none");
                        check(state == before, "visit query and application preserve caller snapshot");
                    }
                }
            }
        }
    }

    for (const auto& role : roles) {
        check(original::service_duration(role.role) == role.duration, "observed role grant duration");
        auto state = distinct_state();
        check(original::service_timer(state, role.role) == state.*(role.timer), "role reads matching byte only");
        for (const auto previous : std::array<std::uint8_t, 8>{0, 1, 39, 40, 95, 96, 254, 255}) {
            state.*(role.timer) = previous;
            const auto granted = original::apply_service_visit(state, {3, true}, direct_visit(role));
            check(granted == reference_grant(state, role.action), "visit overwrites, including a greater previous byte");
            check(original::apply_service_visit(granted, {3, true}, direct_visit(role)) == granted,
                  "multiple visits renew without accumulating or doubling duration");
        }
    }

    auto state = distinct_state();
    state.occupants = 0;
    state.type = HouseType::HeavenlyCompound;
    check(original::apply_service_visit(state, {3, true}, direct_visit(roles[0])) == state,
          "direct ordinary service does not inherit the shared-walker empty-elite exception");
    check(original::apply_service_visit(state, {0, true},
                                       {VisitPath::SharedWalker, VisitAction::Herbalist, false}).herbalist == 96,
          "shared walker accepts empty elite and does not invent a local status gate");
    state.type = HouseType::Hut;
    state.occupants = 1;
    check(original::apply_service_visit(state, {3, true},
                                       {VisitPath::DirectReligion, VisitAction::Ancestor, true}) == state,
          "elite-only religious source rejects even an occupied ordinary house");

    // Several observed callback paths emit several resolved effects. The API
    // deliberately receives each effect rather than treating an actor ID as
    // one universal action or letting one service stand in for every role.
    const ServiceHouseContext shared_context{0, true};
    const auto before = state;
    auto combined = original::apply_service_visit(state, shared_context,
                                                  {VisitPath::SharedWalker, VisitAction::AcrobatAndDrama, false});
    combined = original::apply_service_visit(combined, shared_context,
                                              {VisitPath::SharedWalker, VisitAction::Buddhist, false});
    combined = original::apply_service_visit(combined, shared_context,
                                              {VisitPath::SharedWalker, VisitAction::Daoist, false});
    auto expected = before;
    expected.acrobat = expected.drama = 96;
    expected.daoist = expected.buddhist = 40;
    check(combined == expected, "multiple resolved paired-entertainment and religion effects remain independent");
    combined = original::apply_service_visit(state, shared_context,
                                              {VisitPath::SharedWalker, VisitAction::Music, false});
    combined = original::apply_service_visit(combined, shared_context,
                                              {VisitPath::SharedWalker, VisitAction::Confucian, false});
    combined = original::apply_service_visit(combined, shared_context,
                                              {VisitPath::SharedWalker, VisitAction::Ancestor, false});
    expected = before;
    expected.music = 96;
    expected.confucian = expected.ancestor = 40;
    check(combined == expected, "multiple resolved music and religion effects never alias another timer");
}

void countdown_and_renewal() {
    for (const auto& selected : roles) {
        for (unsigned value = 0; value <= 255; ++value) {
            auto state = distinct_state();
            state.*(selected.timer) = static_cast<std::uint8_t>(value);
            auto expected = state;
            for (const auto& role : roles) {
                const auto old = state.*(role.timer);
                expected.*(role.timer) = old == 0 ? 0 : static_cast<std::uint8_t>(old - 1);
            }
            const auto before = state;
            check(original::decay_service_timers(state) == expected,
                  "exhaustive byte countdown has no wrap, signed interpretation or cross-field mutation");
            check(state == before, "countdown returns a new snapshot");
        }
    }

    for (std::uint16_t type = 2; type <= 17; ++type) {
        for (const auto occupants : std::array<std::int16_t, 3>{-1, 0, 1}) {
            for (const auto status : std::array<std::uint8_t, 6>{0, 1, 2, 3, 4, 255}) {
                for (const auto access : {false, true}) {
                    auto state = distinct_state();
                    state.type = static_cast<HouseType>(type);
                    state.occupants = occupants;
                    const auto expected = access && (status == 1 || status == 3)
                                        ? original::decay_service_timers(state) : state;
                    check(original::decay_house_services(state, {status, access}) == expected,
                          "house-dispatch countdown respects status/access without an occupancy gate");
                }
            }
        }
    }

    for (const auto& role : roles) {
        HouseState absent;
        absent.occupants = 1;
        for (unsigned cycle = 0; cycle < 300; ++cycle)
            absent = original::decay_house_services(absent, {3, true});
        for (const auto& other : roles)
            check(original::service_timer(absent, other.role) == 0,
                  "no observed visit grants no coverage through any countdown or elapsed cycles");

        auto state = original::apply_service_visit(absent, {3, true}, direct_visit(role));
        for (unsigned cycle = 1; cycle <= role.duration; ++cycle) {
            state = original::decay_house_services(state, {3, true});
            check(original::service_timer(state, role.role) == role.duration - cycle,
                  "exact service/religion grant expires after its explicit countdown count");
        }
        state = original::apply_service_visit(state, {3, true}, direct_visit(role));
        for (unsigned cycle = 0; cycle < role.duration / 2U; ++cycle)
            state = original::decay_house_services(state, {3, true});
        state = original::apply_service_visit(state, {3, true}, direct_visit(role));
        check(original::service_timer(state, role.role) == role.duration,
              "interleaved arrival renews a partly elapsed timer to full duration");
        for (unsigned cycle = 1; cycle <= role.duration; ++cycle) {
            state = original::decay_house_services(state, {3, true});
            check((original::service_timer(state, role.role) != 0) == (cycle < role.duration),
                  "renewal extends access exactly until its own final countdown boundary");
        }
    }
}

constexpr std::array<std::string_view, 15> house_labels{
    "1: Shelter", "2: Hut", "3: Plain Cottage", "4: Attractive Cottage",
    "5: Spacious Dwelling", "6: Elegant Dwelling", "7: Ornate Apartment",
    "8: Lux. Apartment", "Elite: Unoccupied", "Elite 0: Abandoned",
    "Elite 1: Modest Siheyuan", "Elite 2: Lavish Siheyuan",
    "Elite 3: Humble Compound", "Elite 4: Impressive Compound", "Elite 5: Heavenly Compound"
};
constexpr std::array<std::string_view, 5> difficulty_labels{
    "Very Easy Modifiers", "Easy Modifiers", "Normal Modifier",
    "Hard Modifiers", "Very Hard Modifiers"
};

template<std::size_t N>
std::string row(std::string_view label, const std::array<std::int32_t, N>& values) {
    std::string result(label);
    result += ",{,";
    for (const auto value : values) result += std::to_string(value) + ',';
    return result + "},,,\n";
}

original::ModelData authored_model(HouseField need) {
    std::string text = "Authored timer/evolution fixture\nBUILDING MODS\n";
    original::BuildingRow percentages{};
    percentages.fill(100);
    for (std::size_t i = 0; i < difficulty_labels.size(); ++i)
        text += row(std::to_string(i) + ',' + std::string(difficulty_labels[i]), percentages);
    text += "ALL BUILDINGS\n";
    text += row("72,Well", original::BuildingRow{});
    text += "HOUSE MODS\n";
    for (const auto label : difficulty_labels) text += row(label, original::HouseRow{});
    text += "ALL HOUSES\n";
    for (const auto label : house_labels) {
        original::HouseRow values{};
        values[static_cast<std::size_t>(HouseField::DesirabilityLow)] = -10;
        values[static_cast<std::size_t>(HouseField::DesirabilityHigh)] = 10;
        values[static_cast<std::size_t>(need)] = 1;
        text += row(label, values);
    }
    text += "End of model data.\n-------------------\n";
    auto parsed = original::parse_model_data(text);
    check(static_cast<bool>(parsed), parsed.error ? parsed.error->reason : "authored model parsed");
    return std::move(*parsed.model);
}

void decay_before_evolution() {
    for (const auto& role : roles) {
        const original::HouseEvolutionRules rules(authored_model(role.need), original::Difficulty::Normal);
        auto state = supplied();
        state.desirability = 0;
        state.devolve_counter = 9;
        state.*(role.timer) = 1;
        if (role.role == ServiceRole::Daoist) state.buddhist = 0;
        if (role.role == ServiceRole::Buddhist) state.daoist = 0;
        check(original::needs_met(rules, original::ModelHouse::Hut, state),
              "last positive timer still supplies its actual evolution predicate");
        const auto decayed = original::decay_house_services(state, {3, true});
        check(!original::needs_met(rules, original::ModelHouse::Hut, decayed),
              "phase-35 final countdown removes access before the later evolution query");
        const auto correct = original::evolve_step(rules, decayed);
        check(correct.type == HouseType::Shelter && correct.devolve_counter == 0,
              "explicit decay then evolution sees the expired need and reaches its existing devolution boundary");
        const auto reversed = original::decay_house_services(original::evolve_step(rules, state), {3, true});
        check(reversed.type == HouseType::Hut && reversed.devolve_counter == 0 && reversed != correct,
              "reversing independent dispatches is observably different at timer one");
        const auto renewed = original::apply_service_visit(decayed, {3, true}, direct_visit(role));
        check(original::evolve_step(rules, renewed).type == HouseType::Hut,
              "an explicitly interleaved arrival can renew access between separate dispatches");
    }
}

struct SpawnProfile { SpawnBuilding building; std::array<std::uint8_t, 6> intervals; };
// Each table is listed against its independently observed provider identity;
// the two Daoist/Buddhist variants intentionally do not collapse into a role.
constexpr std::array<SpawnProfile, 11> spawn_profiles{{
    {SpawnBuilding::Herbalist, {1, 3, 5, 10, 15, 64}},
    {SpawnBuilding::Acupuncture, {1, 3, 7, 15, 29, 64}},
    {SpawnBuilding::MusicSchool, {3, 6, 12, 24, 32, 64}},
    {SpawnBuilding::AcrobatSchool, {3, 6, 12, 24, 32, 64}},
    {SpawnBuilding::DramaSchool, {6, 12, 24, 32, 48, 96}},
    {SpawnBuilding::AncestralShrine, {3, 6, 12, 24, 32, 64}},
    {SpawnBuilding::DaoistShrine, {3, 6, 12, 24, 32, 64}},
    {SpawnBuilding::DaoistTemple, {6, 12, 24, 32, 48, 64}},
    {SpawnBuilding::BuddhistShrine, {3, 6, 12, 24, 32, 64}},
    {SpawnBuilding::BuddhistPagoda, {6, 12, 24, 32, 48, 64}},
    {SpawnBuilding::ConfucianAcademy, {6, 12, 24, 32, 48, 64}}
}};

std::size_t staffing_tier(std::int32_t staffing) {
    if (staffing >= 100) return 0;
    if (staffing >= 75) return 1;
    if (staffing >= 50) return 2;
    if (staffing >= 25) return 3;
    return staffing >= 1 ? 4 : 5;
}

void spawn_schedules() {
    constexpr std::array<std::int32_t, 17> staffing_values{
        std::numeric_limits<std::int32_t>::min(), -100, -1, 0, 1, 24, 25, 49,
        50, 74, 75, 99, 100, 101, 255, 10000, std::numeric_limits<std::int32_t>::max()
    };
    for (const auto& profile : spawn_profiles) {
        for (const auto staffing : staffing_values) {
            const auto interval = profile.intervals[staffing_tier(staffing)];
            check(original::service_spawn_interval(profile.building, staffing) == interval,
                  "provider-specific staffing staircase including signed domain and every threshold");
            for (unsigned counter = 0; counter <= 255; ++counter) {
                for (const auto allowed : {false, true}) {
                    const auto byte = static_cast<std::uint8_t>(counter);
                    const auto incremented = static_cast<std::uint8_t>((counter + 1U) % 256U);
                    const bool evaluated = allowed && staffing > 0;
                    const bool due = evaluated && incremented > interval;
                    const original::ServiceSpawnStep expected{
                        evaluated ? (due ? std::uint8_t{0} : incremented) : byte, due
                    };
                    check(original::service_spawn_step(profile.building, staffing, byte, allowed) == expected,
                          "exhaustive byte scheduler: prerequisite freeze, modulo increment, strict comparison, reset");
                }
            }
            if (staffing > 0) {
                auto counter = std::uint8_t{0};
                for (unsigned update = 1; update <= static_cast<unsigned>(interval) + 1U; ++update) {
                    const auto step = original::service_spawn_step(profile.building, staffing, counter, true);
                    check(step.attempt_due == (update == static_cast<unsigned>(interval) + 1U),
                          "attempt first becomes due after interval plus one allowed evaluations");
                    counter = step.counter;
                }
                check(counter == 0, "due attempt resets before any unmodelled actor factory result");
            }
        }
        check(original::service_spawn_step(profile.building, 100, 255, true) == original::ServiceSpawnStep{0, false},
              "stored counter 255 wraps to zero and does not produce a due attempt");
    }
    auto house = HouseState{};
    house.occupants = 1;
    const auto before = house;
    std::size_t attempts = 0;
    auto counter = std::uint8_t{0};
    for (unsigned update = 0; update < 1000; ++update) {
        const auto step = original::service_spawn_step(SpawnBuilding::Herbalist, 100, counter, true);
        counter = step.counter;
        attempts += step.attempt_due ? 1U : 0U;
        house = original::decay_house_services(house, {3, true});
    }
    check(attempts == 500 && house == before,
          "due spawn attempts alone create no actor, arrival, proximity coverage or timer grant");
}

void rejected_domains() {
    const auto state = distinct_state();
    for (const auto raw : std::array<std::uint16_t, 4>{0, 1, 18, 65535}) {
        auto invalid = state;
        invalid.type = static_cast<HouseType>(raw);
        const auto before = invalid;
        const ServiceVisit visit{VisitPath::DirectService, VisitAction::Herbalist, false};
        expect_exception<std::out_of_range>([&] { (void)original::service_timer(invalid, ServiceRole::Herbalist); },
                                           "narrow timer getter rejects a nonhouse snapshot");
        expect_exception<std::out_of_range>([&] { (void)original::accepts_service_visit(invalid, {}, visit); },
                                           "invalid target type cannot hide behind admission gates");
        expect_exception<std::out_of_range>([&] { (void)original::apply_service_visit(invalid, {3, true}, visit); },
                                           "visit application rejects a nonhouse snapshot");
        expect_exception<std::out_of_range>([&] { (void)original::decay_service_timers(invalid); },
                                           "narrow lowlevel countdown rejects a nonhouse snapshot");
        expect_exception<std::out_of_range>([&] { (void)original::decay_house_services(invalid, {}); },
                                           "invalid target cannot hide behind disabled countdown dispatch");
        check(invalid == before, "invalid-house rejection leaves the input untouched");
    }
    for (const auto raw : std::array<std::uint8_t, 3>{9, 128, 255}) {
        const auto role = static_cast<ServiceRole>(raw);
        expect_exception<std::out_of_range>([&] { (void)original::service_duration(role); }, "invalid role duration rejected");
        expect_exception<std::out_of_range>([&] { (void)original::service_timer(state, role); }, "invalid role read rejected");
    }
    for (const auto raw : std::array<std::uint8_t, 3>{11, 128, 255}) {
        const auto building = static_cast<SpawnBuilding>(raw);
        expect_exception<std::out_of_range>([&] { (void)original::service_spawn_interval(building, 100); }, "invalid provider interval rejected");
        expect_exception<std::out_of_range>([&] { (void)original::service_spawn_step(building, 0, 255, false); },
                                           "invalid provider cannot hide behind disabled prerequisites");
    }
    for (const auto raw : std::array<std::uint8_t, 3>{3, 128, 255}) {
        const ServiceVisit visit{static_cast<VisitPath>(raw), VisitAction::Herbalist, false};
        expect_exception<std::out_of_range>([&] { (void)original::accepts_service_visit(state, {}, visit); }, "invalid visit path rejected before admission");
        expect_exception<std::out_of_range>([&] { (void)original::apply_service_visit(state, {}, visit); }, "invalid visit path application rejected");
    }
    for (const auto raw : std::array<std::uint8_t, 3>{10, 128, 255}) {
        const ServiceVisit visit{VisitPath::DirectService, static_cast<VisitAction>(raw), false};
        expect_exception<std::out_of_range>([&] { (void)original::accepts_service_visit(state, {}, visit); }, "invalid visit action rejected before admission");
        expect_exception<std::out_of_range>([&] { (void)original::apply_service_visit(state, {}, visit); }, "invalid visit action application rejected");
    }
    for (const auto path : {VisitPath::DirectService, VisitPath::DirectReligion, VisitPath::SharedWalker}) {
        for (unsigned action = 0; action <= 9; ++action) {
            for (const auto elite_only : {false, true}) {
                const auto selected = static_cast<VisitAction>(action);
                const bool religious = religion(selected);
                const bool valid = path == VisitPath::DirectService ? action <= 4 && !elite_only
                                 : path == VisitPath::DirectReligion ? religious
                                 : (religious || action <= 2 || action == 9) && (!elite_only || religious);
                if (valid) continue;
                const ServiceVisit visit{path, selected, elite_only};
                expect_exception<std::invalid_argument>([&] { (void)original::accepts_service_visit(state, {}, visit); },
                                                       "unobserved path/action combination rejected even for an ineligible target");
                expect_exception<std::invalid_argument>([&] { (void)original::apply_service_visit(state, {3, true}, visit); },
                                                       "unsupported resolved action never silently reports successful delivery");
            }
        }
    }
    check(state == distinct_state(), "all rejected operations preserve supplied state");
}

std::string read_bytes(const fs::path& path) {
    const auto length = fs::file_size(path);
    check(length <= original::max_model_bytes, "original integration read stays bounded");
    std::ifstream file(path, std::ios::binary);
    check(static_cast<bool>(file), "original model readable");
    std::string bytes(static_cast<std::size_t>(length), '\0');
    file.read(bytes.data(), static_cast<std::streamsize>(length));
    check(file.gcount() == static_cast<std::streamsize>(length) && !file.bad(),
          "complete bounded original integration read");
    char extra = 0;
    check(!file.get(extra) && file.eof() && !file.bad(), "original source does not grow during read");
    return bytes;
}

int real_model(const fs::path& root) {
    if (root.empty()) { std::cout << "SKIP: original root not configured\n"; return 77; }
    std::error_code error;
    const auto source = root / "Model/EmperorBuildingModels.txt";
    for (const auto& path : std::array<fs::path, 3>{root, root / "Model", source}) {
        const auto status = fs::symlink_status(path, error);
        if (status.type() == fs::file_type::not_found) { std::cout << "SKIP: original model path missing\n"; return 77; }
        check(!error, "existing original path metadata must be readable");
        const auto followed = fs::status(path, error);
        check(!error, "existing original path must resolve");
        check(path == source ? fs::is_regular_file(followed) : fs::is_directory(followed), "existing source path has expected type");
    }
    const auto initial_size = fs::file_size(source);
    const auto initial_time = fs::last_write_time(source);
    const auto before = read_bytes(source);
    auto loaded = original::load_model_data(root);
    check(static_cast<bool>(loaded), loaded.error ? loaded.error->reason : "original model loaded");
    const auto& model = *loaded.model;
    check(model.provenance().sha256 == "7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6",
          "integration uses the documented read-only model identity");
    std::size_t queries = 0;
    std::size_t required_accesses = 0;
    for (std::size_t difficulty = 0; difficulty < original::difficulty_count; ++difficulty) {
        const original::HouseEvolutionRules rules(model, static_cast<original::Difficulty>(difficulty));
        for (std::size_t row_index = 0; row_index < original::house_row_count; ++row_index) {
            const auto model_house = static_cast<original::ModelHouse>(row_index);
            const auto type = static_cast<HouseType>(row_index + 3);
            for (const auto& role : roles) {
                auto state = supplied(type);
                state.*(role.timer) = 0;
                if (role.role == ServiceRole::Daoist) state.buddhist = 0;
                if (role.role == ServiceRole::Buddhist) state.daoist = 0;
                const bool required = rules.value(model_house, role.need) != 0;
                required_accesses += required ? 1U : 0U;
                check(original::needs_met(rules, model_house, state) == !required,
                      "missing access agrees with the actual modified nonzero model requirement");
                const auto input = state;
                state = original::apply_service_visit(state, {3, true}, direct_visit(role));
                check(original::service_timer(state, role.role) == role.duration &&
                      original::needs_met(rules, model_house, state),
                      "explicit eligible visit supplies the actual model predicate for every row and difficulty");
                for (unsigned cycle = 1; cycle < role.duration; ++cycle) {
                    state = original::decay_house_services(state, {3, true});
                    check(original::needs_met(rules, model_house, state), "every positive countdown byte keeps actual access");
                }
                state = original::decay_house_services(state, {3, true});
                check(original::needs_met(rules, model_house, state) == !required,
                      "final explicit countdown removes exactly the actual required access");
                check(input.*(role.timer) == 0, "real-source event application leaves its input unchanged");
                ++queries;
            }
        }
    }
    check(required_accesses > 0, "real integration exercises actual nonzero access requirements");
    check(read_bytes(source) == before, "original model bytes remain unchanged");
    check(fs::file_size(source) == initial_size && fs::last_write_time(source) == initial_time,
          "original model size and modification time remain unchanged");
    std::cout << "real visit/expiry queries=" << queries << "; nonzero needs=" << required_accesses
              << "; SHA-256=" << model.provenance().sha256 << '\n';
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    try {
        if (argc == 3 && std::string_view(argv[1]) == "--real") {
            const auto result = real_model(argv[2]);
            if (result == 77) return result;
        } else if (argc == 1) {
            visits_and_admission();
            countdown_and_renewal();
            decay_before_evolution();
            spawn_schedules();
            rejected_domains();
        } else {
            std::cerr << "usage: original-service-rules-tests [--real ROOT]\n";
            return 2;
        }
        std::cout << "original service rules checks passed: " << checks << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "original service rules test failed: " << error.what() << '\n';
        return 1;
    }
}
