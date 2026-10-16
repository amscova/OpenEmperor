// Authored numerical adapter. Compiled twice against separate implementations;
// no fork implementation or original model data is included in this file.
#include "simulation/original/ModelData.h"
#include "simulation/original/Desirability.h"
#include "simulation/original/HouseEvolution.h"
#ifndef OE_PARITY_FORK
#include "simulation/original/GoodsRules.h"
#include "simulation/original/ServiceRules.h"
#endif

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace o = openemperor::original;
namespace {
using Numbers = std::vector<std::int64_t>;
void emit(const Numbers& values) {
    std::cout << '[';
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i != 0) std::cout << ',';
        std::cout << values[i];
    }
    std::cout << "]\n";
}
int bounded(int value, int low, int high) {
    if (value < low || value > high) throw std::out_of_range("adapter input domain");
    return value;
}
o::HouseState house(const std::vector<int>& p, std::size_t start = 0) {
    if (p.size() < start + 21) throw std::invalid_argument("incomplete house observation");
    o::HouseState h;
    h.type = static_cast<decltype(h.type)>(bounded(p[start], 2, 17));
    h.desirability = static_cast<decltype(h.desirability)>(bounded(p[start+1], -128, 127));
    h.occupants = static_cast<decltype(h.occupants)>(bounded(p[start+2], -32768, 32767));
    h.devolve_counter = static_cast<decltype(h.devolve_counter)>(bounded(p[start+3], 0, 255));
    h.water = bounded(p[start+4], 0, 1) != 0;
    const auto byte = [&](std::size_t i) { return static_cast<std::uint8_t>(bounded(p[start+i], 0, 255)); };
    h.herbalist = byte(5); h.acupuncture = byte(6); h.music = byte(7);
    h.acrobat = byte(8); h.drama = byte(9); h.ancestor = byte(10);
    h.confucian = byte(11); h.daoist = byte(12); h.buddhist = byte(13);
    h.food_quality = byte(14);
    const auto stock = [&](std::size_t i) { return static_cast<std::int16_t>(bounded(p[start+i], 0, 32767)); };
    h.hemp = stock(15); h.ceramics = stock(16); h.tea = stock(17);
    h.silk = stock(18); h.bronzeware = stock(19); h.lacquerware = stock(20);
    return h;
}
Numbers stocks(const o::HouseState& h) { return {h.hemp,h.ceramics,h.tea,h.silk,h.bronzeware,h.lacquerware}; }
#ifdef OE_PARITY_FORK
o::Goods goods(int i) {
    constexpr std::array values{o::Goods::Hemp,o::Goods::Ceramics,o::Goods::Tea,
                                o::Goods::Silk,o::Goods::Bronzeware,o::Goods::Lacquerware};
    return values.at(static_cast<std::size_t>(bounded(i,0,5)));
}
#else
o::Goods goods(int i) { return static_cast<o::Goods>(bounded(i,0,5)); }
#endif
}

int main(int argc, char** argv) {
    if (argc != 2) { std::cerr << "expected model fixture path\n"; return 2; }
    try {
        const auto path = std::filesystem::path(argv[1]);
        const auto size = std::filesystem::file_size(path);
        if (size > 256 * 1024) throw std::runtime_error("fixture exceeds bounded reader");
        std::ifstream in(path, std::ios::binary);
        std::string text(static_cast<std::size_t>(size), '\0');
        in.read(text.data(), static_cast<std::streamsize>(text.size()));
        if (!in || in.peek() != std::char_traits<char>::eof()) throw std::runtime_error("fixture read changed/failed");
        std::string first;
        if (!std::getline(std::cin, first)) throw std::invalid_argument("missing operation");
        const bool parse_only = first == "parse" || first == "parse ";
#ifdef OE_PARITY_FORK
        o::ModelData model;
        try { model = o::parse_model_data(text); }
        catch (...) { if (parse_only) { emit({0}); return 0; } throw; }
#else
        auto parsed = o::parse_model_data(text);
        if (!parsed) { if (parse_only) { emit({0}); return 0; } throw std::runtime_error("current fixture parser rejected input"); }
        const auto& model = *parsed.model;
#endif
        if (parse_only) { emit({1}); return 0; }
        auto line = first;
        do {
            std::istringstream command(line);
            std::string op;
            command >> op;
            std::vector<int> p;
            for (int value = 0; command >> value;) p.push_back(value);
            if (!command.eof()) throw std::invalid_argument("invalid adapter number");
#ifndef OE_PARITY_FORK
            const o::HouseEvolutionRules rules(model, o::Difficulty::Normal);
#endif
            if (op == "house_value") {
                if (p.size()!=3) throw std::invalid_argument("house_value arity");
                const int row=bounded(p[0],0,14), field=bounded(p[1],0,23), difficulty=bounded(p[2],0,4);
#ifdef OE_PARITY_FORK
                emit({model.houses.at(static_cast<std::size_t>(row)).at(static_cast<std::size_t>(field)), model.house_value(row,field,difficulty)});
#else
                emit({model.house(o::HouseRowId{static_cast<std::uint8_t>(row)},static_cast<o::HouseField>(field)),o::house_value(model,static_cast<o::ModelHouse>(row),static_cast<o::HouseField>(field),static_cast<o::Difficulty>(difficulty))});
#endif
            } else if (op == "building_value") {
                if (p.size()!=3 || p[0]!=72) throw std::invalid_argument("building_value arity/identity");
                const auto field=bounded(p[1],0,12), difficulty=bounded(p[2],0,4);
#ifdef OE_PARITY_FORK
                emit({model.buildings.at(72).at(static_cast<std::size_t>(field)),model.building_value(72,field,difficulty)});
#else
                emit({model.building(o::ModelBuildingId{72},static_cast<o::BuildingField>(field)),o::building_value(model,o::ModelBuilding::Well,static_cast<o::BuildingField>(field),static_cast<o::Difficulty>(difficulty))});
#endif
            } else if (op == "needs" || op == "evolve" || op == "consume" || op == "timers") {
                if (p.size() != (op=="needs" ? 22U : 21U)) throw std::invalid_argument("house command arity");
                auto h=house(p,op=="needs"?1U:0U);
                if (op == "needs") {
#ifdef OE_PARITY_FORK
                    emit({o::needs_met(model,2,bounded(p[0],0,14),h)});
#else
                    emit({o::needs_met(rules,static_cast<o::ModelHouse>(bounded(p[0],0,14)),h)});
#endif
                } else if (op == "evolve") {
#ifdef OE_PARITY_FORK
                    o::evolve_step(model,2,h);
#else
                    h=o::evolve_step(rules,h);
#endif
                    emit({static_cast<int>(h.type),h.devolve_counter});
                } else if (op == "consume") {
#ifdef OE_PARITY_FORK
                    o::consume_goods(model,2,h);
#else
                    h=o::consume_goods(rules,h).house;
#endif
                    emit(stocks(h));
                } else {
#ifdef OE_PARITY_FORK
                    o::decay_services(h);
#else
                    h=o::decay_service_timers(h);
#endif
                    emit({h.herbalist,h.acupuncture,h.music,h.acrobat,h.drama,h.ancestor,h.confucian,h.daoist,h.buddhist});
                }
            } else if (op == "composition") {
                if (p.size()!=22) throw std::invalid_argument("composition arity");
                const int phase=bounded(p[0],0,15); auto h=house(p,1);
#ifdef OE_PARITY_FORK
                o::step_house(model,2,h,phase);
#else
                // Authored explicit invocation sequence for comparison only.
                // This is not a reconstructed original or World clock.
                h=o::decay_service_timers(h);
                h=o::evolve_step(rules,h);
                if (phase==0 || phase==7) h=o::consume_goods(rules,h).house;
#endif
                emit({static_cast<int>(h.type),h.devolve_counter,h.herbalist,h.hemp});
            } else if (op == "visit") {
                if (p.size()!=22) throw std::invalid_argument("visit arity");
                const int role=bounded(p[0],0,8); auto h=house(p,1);
#ifdef OE_PARITY_FORK
                const bool accepted=o::apply_service_visit(h,static_cast<o::ServiceKind>(role));
                const std::array<int,9> timers{h.herbalist,h.acupuncture,h.music,h.acrobat,h.drama,h.ancestor,h.confucian,h.daoist,h.buddhist};
                emit({accepted,timers.at(static_cast<std::size_t>(role))});
#else
                const o::ServiceVisit visit{role<5?o::VisitPath::DirectService:o::VisitPath::DirectReligion,static_cast<o::VisitAction>(role),false};
                const o::ServiceHouseContext context{3,true};
                const bool accepted=o::accepts_service_visit(h,context,visit);
                h=o::apply_service_visit(h,context,visit);
                emit({accepted,o::service_timer(h,static_cast<o::ServiceRole>(role))});
#endif
            } else if (op == "need" || op == "delivery") {
                if (p.size() != (op=="need"?23U:24U)) throw std::invalid_argument("goods command arity");
                auto h=house(p,2);
                const auto g=goods(p[0]); const auto side=static_cast<std::uint8_t>(bounded(p[1],0,255));
#ifdef OE_PARITY_FORK
                h.size=side;
                if (op=="need") emit({o::goods_need(model,2,h,g)});
                else { int source=p[23]; const int moved=o::deliver_goods(model,2,h,g,source); emit({moved,source,o::goods_stock(h,g)}); }
#else
                if (op=="need") emit({o::goods_demand(rules,h,side,g).need});
                else { auto result=o::deliver_goods(rules,h,side,g,p[23]); emit({result.transferred,result.source_stock,o::goods_stock(result.house,g)}); }
#endif
            } else if (op == "food") {
                if (p.size()!=4) throw std::invalid_argument("food arity");
                bounded(p[0],0,255); bounded(p[1],0,255); bounded(p[2],0,std::numeric_limits<int>::max()); bounded(p[3],0,std::numeric_limits<int>::max());
#ifdef OE_PARITY_FORK
                emit({o::blend_food_quality(p[0],p[1],p[2],p[3])});
#else
                emit({o::blend_food_quality(static_cast<std::uint8_t>(p[0]),static_cast<std::uint8_t>(p[1]),p[2],p[3])});
#endif
            } else if (op == "spawn" || op == "spawn_tick") {
                if (p.size()!=3) throw std::invalid_argument("spawn arity");
                const auto provider=bounded(p[0],0,10), staffing=p[1], counter=bounded(p[2],0,255);
#ifdef OE_PARITY_FORK
                const int interval=provider>=5?o::religion_spawn_interval(staffing):o::service_spawn_interval(staffing);
                if (op=="spawn") emit({interval});
                else {
                    if (provider!=1) throw std::invalid_argument("only generic Acupuncture counter has a direct fork tick");
                    int next=counter; const bool due=o::service_spawn_tick(next,staffing);
                    emit({next,due});
                }
#else
                const auto building=static_cast<o::SpawnBuilding>(provider);
                const auto result=o::service_spawn_step(building,staffing,static_cast<std::uint8_t>(counter),true);
                if (op=="spawn") emit({o::service_spawn_interval(building,staffing)});
                else emit({result.counter,result.attempt_due});
#endif
            } else if (op == "spread") {
                if (p.size()!=7) throw std::invalid_argument("spread arity");
                o::DesirabilityMap map(11,11);
                const o::SpreadSpec spec{p[3],p[4],p[5],p[6]};
#ifdef OE_PARITY_FORK
                map.apply_source(p[0],p[1],p[2],spec,o::WallBlocking::None);
#else
                map.apply_source({{p[0],p[1],p[2]},spec});
#endif
                emit({map.at(5,5),map.at(6,5),map.at(7,5),map.at(8,5),map.at(9,5),map.at(10,5)});
            } else if (op == "order") {
                if (p.size()!=3) throw std::invalid_argument("order arity");
                o::DesirabilityMap map(11,11);
                for (const auto value:p) {
#ifdef OE_PARITY_FORK
                    map.apply_source(5,5,1,{value,1,0,1},o::WallBlocking::None);
#else
                    map.apply_source({{5,5,1},{value,1,0,1}});
#endif
                }
                emit({map.at(6,5)});
            } else if (op == "real_digest") {
                if (!p.empty()) throw std::invalid_argument("real_digest arity");
                std::uint64_t digest=14695981039346656037ULL; std::int64_t count=0;
                const auto add=[&](std::int32_t value) { for (int b=0;b<4;++b) { digest^=(static_cast<std::uint32_t>(value)>>(b*8))&255U; digest*=1099511628211ULL; } ++count; };
                for (int row=0;row<15;++row) for (int field=0;field<24;++field) for (int difficulty=0;difficulty<5;++difficulty) {
#ifdef OE_PARITY_FORK
                    add(model.house_value(row,field,difficulty));
#else
                    add(o::house_value(model,static_cast<o::ModelHouse>(row),static_cast<o::HouseField>(field),static_cast<o::Difficulty>(difficulty)));
#endif
                }
                // Digest only: never print or persist the original numeric table.
                emit({count,static_cast<std::int64_t>(digest>>32U),static_cast<std::int64_t>(digest&0xffffffffULL)});
            } else throw std::invalid_argument("unknown adapter operation");
        } while (std::getline(std::cin,line));
        return 0;
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 2; }
}
