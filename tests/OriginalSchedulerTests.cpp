#include "simulation/original/OriginalScheduler.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
namespace original=openemperor::original;
using original::SchedulerPosition;

std::size_t checks=0;
void check(bool condition,std::string_view message) {
    ++checks;
    if (!condition) throw std::runtime_error(std::string(message));
}
template<class Operation>
void exhausted(Operation operation) {
    try { operation(); }
    catch (const std::overflow_error&) { ++checks;return; }
    throw std::runtime_error("Exhausted scheduler tick was not rejected before advance");
}

// These bind the saved authored mapping, not the original game's wall clock.
static_assert(original::scheduler_mapping_version==1);
static_assert(original::scheduler_world_ticks_per_phase==1);
static_assert(original::scheduler_phase_count==51);
static_assert(original::scheduler_steps_per_month==16);
static_assert(original::scheduler_months_per_year==12);
static_assert(original::scheduler_position(0)==SchedulerPosition{0,0,0,0});
static_assert(original::scheduler_position(9792)==SchedulerPosition{0,0,0,1});
static_assert(!original::next_scheduler_dispatch(original::scheduler_max_tick));
static_assert(noexcept(original::scheduler_position(std::uint64_t{})));
static_assert(noexcept(original::next_scheduler_dispatch(std::uint64_t{})));

void known_positions() {
    struct Case { std::uint64_t ticks;SchedulerPosition position; };
    // Literal edge observations avoid deriving the expected answer through the
    // same division expression as the production helper.
    constexpr std::array<Case,17> cases{{
        {0,{0,0,0,0}}, {35,{35,0,0,0}}, {39,{39,0,0,0}},
        {50,{50,0,0,0}}, {51,{0,1,0,0}}, {356,{50,6,0,0}},
        {357,{0,7,0,0}}, {396,{39,7,0,0}}, {407,{50,7,0,0}},
        {408,{0,8,0,0}}, {764,{50,14,0,0}}, {765,{0,15,0,0}},
        {804,{39,15,0,0}}, {815,{50,15,0,0}}, {816,{0,0,1,0}},
        {9791,{50,15,11,0}}, {9792,{0,0,0,1}}
    }};
    for (const auto& item:cases) {
        check(original::scheduler_position(item.ticks)==item.position,
            "Scheduler position changed at a saved phase/calendar boundary");
        const auto dispatch=original::next_scheduler_dispatch(item.ticks);
        check(dispatch && dispatch->position==item.position,
            "Next dispatch disagrees with the completed-tick save convention");
    }
}

void separate_service_and_evolution() {
    constexpr std::array<std::uint64_t,16> service_ticks{
        35,86,137,188,239,290,341,392,443,494,545,596,647,698,749,800};
    constexpr std::array<std::uint64_t,16> evolution_ticks{
        39,90,141,192,243,294,345,396,447,498,549,600,651,702,753,804};
    std::size_t services=0,evolutions=0,consumptions=0;
    for (std::uint64_t tick=0;tick<816;++tick) {
        const auto event=original::next_scheduler_dispatch(tick);
        check(event.has_value(),"First scheduler month unexpectedly exhausted");
        if (event->service_countdown) {
            check(services<service_ticks.size() && tick==service_ticks[services],
                "Service countdown is not dispatched at original phase35");
            ++services;
        }
        if (event->evolution) {
            check(evolutions<evolution_ticks.size() && tick==evolution_ticks[evolutions],
                "Evolution is not dispatched at original phase39");
            ++evolutions;
        }
        check(!(event->service_countdown && event->evolution),
            "Service decay and evolution were fused, removing intervening event positions");
        check(event->consumption==(tick==39 || tick==396),
            "Consumption is not restricted to selector0/7 after evolution");
        if (event->consumption) {
            ++consumptions;
            check(event->evolution,"Consumption precedes its evolution dispatch");
        }
    }
    check(services==16 && evolutions==16 && consumptions==2,
        "A complete authored month has the wrong eligible callback counts");
    for (const auto service:service_ticks)
        for (std::uint64_t offset=1;offset<4;++offset) {
            const auto between=original::next_scheduler_dispatch(service+offset);
            check(between && !between->service_countdown && !between->evolution && !between->consumption,
                "An intervening phase unexpectedly dispatches a House update");
        }
    const auto step15=original::next_scheduler_dispatch(804);
    check(step15 && step15->position.step==15 && step15->evolution && !step15->consumption,
        "The last monthly selector accidentally consumes goods");
}

void independent_calendar_replay() {
    // An incrementing odometer is independent of scheduler_position's division
    // arithmetic. Two complete years plus a partial year exercise each rollover
    // and every phase repeatedly without invoking any goods/service/evolution API.
    SchedulerPosition expected;
    std::uint64_t tick=0;
    std::size_t services=0,evolutions=0,consumptions=0;
    for (std::size_t replay=0;replay<20000;++replay) {
        check(original::scheduler_position(tick)==expected,
            "Clock position drifted from the independent calendar odometer");
        const auto dispatch=original::next_scheduler_dispatch(tick);
        check(dispatch && dispatch->position==expected,
            "Replay's next dispatch changed after a completed tick");
        services+=dispatch->service_countdown ? 1U:0U;
        evolutions+=dispatch->evolution ? 1U:0U;
        consumptions+=dispatch->consumption ? 1U:0U;
        tick=original::advance_scheduler_tick(tick);
        ++expected.phase;
        if (expected.phase==51) {
            expected.phase=0;
            ++expected.step;
            if (expected.step==16) {
                expected.step=0;
                ++expected.month;
                if (expected.month==12) {
                    expected.month=0;
                    ++expected.year;
                }
            }
        }
    }
    check(tick==20000 && original::scheduler_position(tick)==SchedulerPosition{8,8,0,2},
        "Twenty-thousand-step replay ended at the wrong exact position");
    check(services==392 && evolutions==392 && consumptions==50,
        "Repeated calendar replay changed eligible operation counts");
}

void authored_epoch_is_not_original_year_field() {
    // Bounded EXE research shows d623fc increments when the original larger
    // counter becomes1 (4ac7d0..4ac7e5, literal-one getter4f8d50). This helper
    // deliberately counts elapsed authored years from epoch0 instead. Its year
    // must not silently adopt that separate historical calendar field.
    check(original::scheduler_position(815).year==0 &&
          original::scheduler_position(816)==SchedulerPosition{0,0,1,0},
        "Elapsed-year epoch was confused with the original counter1 year boundary");
    check(original::scheduler_position(9791).year==0 &&
          original::scheduler_position(9792)==SchedulerPosition{0,0,0,1},
        "Authored elapsed year does not advance after exactly twelve complete months");
}

void integer_limit() {
    constexpr auto maximum=std::numeric_limits<std::uint64_t>::max();
    static_assert(original::scheduler_max_tick==maximum-1);
    check(original::scheduler_position(maximum-2)==SchedulerPosition{49,4,8,1883858667658246ULL},
        "Near-maximum tick position overflowed its division-only calendar");
    check(original::scheduler_position(maximum-1)==SchedulerPosition{50,4,8,1883858667658246ULL},
        "Savable terminal tick has the wrong exact phase");
    check(original::scheduler_position(maximum)==SchedulerPosition{0,5,8,1883858667658246ULL},
        "Position query is not defined over the complete uint64 domain");
    const auto last=original::next_scheduler_dispatch(maximum-2);
    check(last && last->position.phase==49 && !last->service_countdown &&
        !last->evolution && !last->consumption,"Last permitted dispatch changed");
    check(original::advance_scheduler_tick(maximum-2)==original::scheduler_max_tick,
        "Last legal advance did not stop at the savable terminal tick");
    check(!original::next_scheduler_dispatch(maximum-1) &&
        !original::next_scheduler_dispatch(maximum),
        "Terminal or unsavable maximum tick still offers dispatch authority");
    exhausted([&] { (void)original::advance_scheduler_tick(maximum-1); });
    exhausted([&] { (void)original::advance_scheduler_tick(maximum); });
    check(original::advance_scheduler_tick(0)==1,"Initial clock advance failed");
}
} // namespace

int main() {
    try {
        known_positions();separate_service_and_evolution();independent_calendar_replay();
        authored_epoch_is_not_original_year_field();integer_limit();
        std::cout<<"Original scheduler: "<<checks
            <<" checks PASS; authored mapping, observed dispatch order, calendar and uint64 bounds\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr<<"Original scheduler: "<<error.what()<<'\n';return 1;
    }
}
