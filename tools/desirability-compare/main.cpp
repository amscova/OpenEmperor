#include "simulation/World.h"
#include "simulation/original/Desirability.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#ifndef OPENEMPEROR_SOURCE_DIR
#error OPENEMPEROR_SOURCE_DIR is required for the private-output boundary
#endif

namespace {
namespace fs = std::filesystem;
namespace original = openemperor::original;
namespace sim = openemperor::simulation;
using Json = nlohmann::json;
constexpr int width = 27;
constexpr int height = 25;
using Grid = std::array<int, static_cast<std::size_t>(width * height)>;

struct Source {
    original::ModelBuilding model;
    sim::Object object;
    std::string_view object_name;
    original::DesirabilitySource source;
};
struct Probe { std::string_view name; original::Footprint footprint; };
constexpr std::array<Probe, 5> probes{{
    {"H1", {9, 6, 2}}, {"H2", {13, 10, 2}}, {"H3", {1, 17, 2}},
    {"H4", {10, 10, 2}}, {"H5", {22, 20, 2}}
}};

std::size_t index(int x, int y) {
    return static_cast<std::size_t>(y * width + x);
}

bool within(const fs::path& path, const fs::path& root) {
    auto a = path.begin();
    for (auto b = root.begin(); b != root.end(); ++a, ++b)
        if (a == path.end() || *a != *b) return false;
    return true;
}

fs::path output_path(const fs::path& requested, const fs::path& game_root) {
    const auto source = fs::canonical(OPENEMPEROR_SOURCE_DIR);
    const auto local = source / ".local";
    const auto output = fs::absolute(requested).lexically_normal();
    if (!within(output, local) || output == local || within(output, game_root))
        throw std::invalid_argument("output must be a new directory below repository .local, outside the original installation");
    // Refuse aliases even when they would resolve back inside the allowed tree.
    fs::path cursor = source;
    for (const auto& part : output.lexically_relative(source)) {
        cursor /= part;
        if (fs::is_symlink(cursor))
            throw std::invalid_argument("output path may not contain symlinks");
    }
    if (fs::exists(output))
        throw std::invalid_argument("output directory already exists; choose a fresh directory");
    return output;
}

std::string xml(std::string_view value) {
    std::string result;
    for (const char c : value) {
        switch (c) {
        case '&': result += "&amp;"; break;
        case '<': result += "&lt;"; break;
        case '>': result += "&gt;"; break;
        case '"': result += "&quot;"; break;
        default: result += c; break;
        }
    }
    return result;
}

std::array<int, original::desirability_max_range + 1> ring_values(
    const original::SpreadSpec& spec) {
    std::array<int, original::desirability_max_range + 1> values{};
    const int rings = std::clamp(spec.range, 0, original::desirability_max_range);
    std::int32_t value = spec.value;
    int period = 0;
    for (int ring = 1; ring <= rings && value != 0; ++ring) {
        values[static_cast<std::size_t>(ring)] = value;
        if (++period >= spec.step && ring < rings) {
            const auto next = static_cast<std::int64_t>(value) + spec.step_size;
            if (next < std::numeric_limits<std::int32_t>::min() ||
                next > std::numeric_limits<std::int32_t>::max())
                throw std::overflow_error("Manhattan geometry control spread step overflows int32");
            value = static_cast<std::int32_t>(next);
            if ((spec.value > 0 && value < 0) || (spec.value < 0 && value > 0)) value = 0;
            period = 0;
        }
    }
    return values;
}

int legacy_score(original::Footprint query, const std::vector<Source>& sources) {
    int sum = 0;
    for (const auto& source : sources) {
        const auto f = source.source.footprint;
        const int distance = sim::footprint_distance({query.x, query.y}, {query.side, query.side},
                                                     {f.x, f.y}, {f.side, f.side});
        sum += sim::desirability_contribution(source.object, distance);
    }
    return sim::clamp_desirability(sum);
}

Grid manhattan_control(const std::vector<Source>& sources) {
    Grid grid{};
    for (const auto& source : sources) {
        const auto values = ring_values(source.source.spread);
        const auto f = source.source.footprint;
        for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x) {
            const auto distance = sim::footprint_distance({x, y}, {1, 1},
                                                           {f.x, f.y}, {f.side, f.side});
            if (distance < 1 || distance > original::desirability_max_range) continue;
            auto& cell = grid[index(x, y)];
            const auto sum = static_cast<std::int64_t>(cell) + values[static_cast<std::size_t>(distance)];
            cell = static_cast<int>(std::clamp<std::int64_t>(sum, -100, 100));
        }
    }
    return grid;
}

int maximum(const Grid& grid, original::Footprint query) {
    int best = -100;
    for (int y = query.y; y < query.y + query.side; ++y)
        for (int x = query.x; x < query.x + query.side; ++x)
            best = std::max(best, grid[index(x, y)]);
    return best;
}

Json delta_summary(const Grid& a, const Grid& b) {
    int changed = 0, largest = 0, absolute_sum = 0;
    for (std::size_t i = 0; i < a.size(); ++i) {
        const int delta = std::abs(a[i] - b[i]);
        if (delta != 0) ++changed;
        largest = std::max(largest, delta);
        absolute_sum += delta;
    }
    return {{"changed_cells", changed}, {"maximum_absolute_difference", largest},
            {"sum_absolute_difference", absolute_sum}};
}

std::string colour(int value) {
    const int amount = std::min(std::abs(value), 100);
    // Common fixed scale: red negative, neutral zero, teal positive.
    const std::array<int, 3> end = value < 0 ? std::array<int, 3>{164, 35, 46}
                                          : std::array<int, 3>{0, 106, 102};
    std::ostringstream out;
    out << "rgb(";
    for (std::size_t i = 0; i < end.size(); ++i) {
        if (i) out << ',';
        out << 255 + (end[i] - 255) * amount / 100;
    }
    out << ')';
    return out.str();
}

std::string heatmaps(const std::array<Grid, 3>& grids, const std::vector<Source>& sources,
                     std::string_view sha) {
    constexpr int cell = 23, panel = width * cell + 44;
    constexpr int svg_width = panel * 3 + 20, svg_height = height * cell + 157;
    constexpr std::array<std::string_view, 3> titles{{
        "Original-data square rings", "Same-spec Manhattan geometry", "Actual City-v13 authored weights"
    }};
    std::ostringstream out;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << svg_width
        << "\" height=\"" << svg_height << "\" viewBox=\"0 0 " << svg_width << ' ' << svg_height
        << "\" role=\"img\" aria-labelledby=\"title desc\">\n"
        << "<title id=\"title\">OE-03: identical authored sources, three stated calculations</title>\n"
        << "<desc id=\"desc\">Each cell displays its signed score. Source footprint outlines are solid; "
        << "House probes are dashed. The middle map changes only distance geometry. "
        << "The right map uses actual unchanged City-v13 weights and final saturation. "
        << "These are isolated calculations, not a played city or original footprint restoration.</desc>\n"
        << "<rect width=\"100%\" height=\"100%\" fill=\"#f7f7fa\"/>\n"
        << "<g font-family=\"system-ui, sans-serif\" fill=\"#18212c\">"
        << "<text x=\"20\" y=\"25\" font-size=\"19\">OE-03 / identical sources and footprints / walls OFF / Normal difficulty</text>\n"
        << "<text x=\"20\" y=\"47\" font-size=\"12\">Model SHA-256: " << xml(sha) << "</text>\n";
    for (std::size_t g = 0; g < grids.size(); ++g) {
        const int left = 29 + static_cast<int>(g) * panel;
        constexpr int top = 105;
        out << "<text x=\"" << left << "\" y=\"76\" font-size=\"17\">" << titles[g] << "</text>\n";
        for (int x = 0; x < width; ++x)
            out << "<text x=\"" << left + x * cell + cell / 2 << "\" y=\"98\" text-anchor=\"middle\" font-size=\"10\">" << x << "</text>\n";
        for (int y = 0; y < height; ++y) {
            out << "<text x=\"" << left - 6 << "\" y=\"" << top + y * cell + 15
                << "\" text-anchor=\"end\" font-size=\"10\">" << y << "</text>\n";
            for (int x = 0; x < width; ++x) {
                const int value = grids[g][index(x, y)];
                const int px = left + x * cell, py = top + y * cell;
                out << "<rect x=\"" << px << "\" y=\"" << py << "\" width=\"" << cell << "\" height=\"" << cell
                    << "\" fill=\"" << colour(value) << "\" stroke=\"#c7cbd0\" stroke-width=\"0.4\"><title>"
                    << x << ',' << y << ": " << value << "</title></rect>\n"
                    << "<text x=\"" << px + cell / 2 << "\" y=\"" << py + 15
                    << "\" text-anchor=\"middle\" font-size=\"9\" fill=\"" << (std::abs(value) >= 65 ? "white" : "#18212c")
                    << "\">" << value << "</text>\n";
            }
        }
        for (std::size_t i = 0; i < sources.size(); ++i) {
            const auto f = sources[i].source.footprint;
            out << "<rect x=\"" << left + f.x * cell << "\" y=\"" << top + f.y * cell << "\" width=\""
                << f.side * cell << "\" height=\"" << f.side * cell << "\" fill=\"none\" stroke=\"#101010\" stroke-width=\"2.5\"><title>S"
                << i + 1 << ' ' << sources[i].object_name << "</title></rect>\n";
        }
        for (const auto& probe : probes) {
            const auto f = probe.footprint;
            out << "<rect x=\"" << left + f.x * cell + 2 << "\" y=\"" << top + f.y * cell + 2 << "\" width=\""
                << f.side * cell - 4 << "\" height=\"" << f.side * cell - 4
                << "\" fill=\"none\" stroke=\"#0050a4\" stroke-width=\"2\" stroke-dasharray=\"4 2\"><title>"
                << probe.name << " House probe</title></rect>\n";
        }
    }
    const int legend_y = svg_height - 29;
    for (int value : {-100, -50, 0, 50, 100}) {
        const int left = 20 + (value + 100) * 2;
        out << "<rect x=\"" << left << "\" y=\"" << legend_y - 13 << "\" width=\"28\" height=\"20\" fill=\"" << colour(value)
            << "\" stroke=\"#888\"/><text x=\"" << left + 33 << "\" y=\"" << legend_y + 2
            << "\" font-size=\"11\">" << value << "</text>\n";
    }
    out << "<text x=\"540\" y=\"" << legend_y + 2
        << "\" font-size=\"12\">Common fixed scale -100..100. Solid outlines: sources. Dashed blue: 2x2 House probes. Full numbers: cells.csv / houses.csv.</text>\n"
        << "</g></svg>\n";
    return out.str();
}

std::string html(const Json& report) {
    std::ostringstream out;
    out << "<!doctype html><html lang=\"en\"><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
        << "<title>OE-03 desirability comparison</title><style>body{font:16px/1.5 system-ui,sans-serif;max-width:1500px;margin:2rem auto;padding:0 1rem;color:#18212c;background:#f7f7fa}"
        << "h1{font-size:1.8rem}table{border-collapse:collapse;background:white}td,th{padding:.45rem .65rem;text-align:left;border:1px solid #c7cbd0}"
        << ".scroll{overflow:auto;margin:1rem 0}img{display:block;min-width:1500px;width:100%}code{overflow-wrap:anywhere}</style>"
        << "<main><h1>OE-03: same sources, three explicit calculations</h1>"
        << "<p>Read-only original model, Normal difficulty, walls OFF. Source positions and square footprints are authored. "
        << "Object-to-model relationships are functional analogies. This is a detached calculation, not a played city or original-map parity claim.</p>"
        << "<p><strong>Model SHA-256:</strong> <code>" << xml(report.at("model").at("sha256").get<std::string>()) << "</code></p>"
        << "<p>The square-ring kernel uses the original-data spread fields. The Manhattan control uses exactly those same fields, "
        << "source order, saturation and House maximum, changing only distance geometry. The third column uses the actual unchanged "
        << "City-v13 authored weights and final clamp; its model numbers are therefore intentionally different.</p>"
        << "<p><a href=\"report.json\">Provenance and result JSON</a> · <a href=\"cells.csv\">All cell scores CSV</a> · "
        << "<a href=\"houses.csv\">House footprint scores CSV</a> · <a href=\"heatmaps.svg\">Standalone SVG</a></p>"
        << "<div class=\"scroll\"><img src=\"heatmaps.svg\" alt=\"Three numeric heatmaps of square-ring, same-spec Manhattan, and actual authored City-v13 scores; full numbers available in the linked CSV tables.\"></div>"
        << "<h2>Identical ordered sources</h2><div class=\"scroll\"><table><thead><tr><th>Order</th><th>Model record</th><th>City-v13 analogy</th><th>Origin / side</th>"
        << "<th>Original V / STP / SZE / RNG</th><th>Authored City-v13 impact</th></tr></thead><tbody>";
    for (const auto& source : report.at("sources")) {
        const auto& spread = source.at("spread");
        const auto& f = source.at("footprint");
        out << "<tr><td>" << source.at("order") << "</td><td>" << source.at("model_id") << " / "
            << xml(source.at("model_label").get<std::string>()) << "</td><td>" << xml(source.at("city_v13_object").get<std::string>())
            << "</td><td>" << f.at("x") << ',' << f.at("y") << " / " << f.at("side") << "</td><td>"
            << spread.at("value") << " / " << spread.at("step") << " / " << spread.at("step_size") << " / " << spread.at("range")
            << "</td><td>" << source.at("city_v13_authored_impact") << "</td></tr>";
    }
    out << "</tbody></table></div><h2>2x2 House probes</h2>"
        << "<p>Kernel and geometry-control columns take the maximum of summed cell values over each House footprint. "
        << "Actual City-v13 first takes each source's minimum Manhattan distance to the whole House footprint, applies its authored weight, "
        << "then sums and clamps. No House bonus is assumed. Probes are read-only queries and do not claim paid placement or constructibility.</p>"
        << "<div class=\"scroll\"><table><thead><tr><th>Probe</th><th>Origin</th><th>Square-ring maximum</th><th>Same-spec Manhattan maximum</th>"
        << "<th>Actual City-v13 footprint score</th></tr></thead><tbody>";
    for (const auto& probe : report.at("house_probes")) {
        out << "<tr><td>" << xml(probe.at("name").get<std::string>()) << "</td><td>" << probe.at("x") << ',' << probe.at("y")
            << "</td><td>" << probe.at("original_square_max") << "</td><td>" << probe.at("manhattan_same_spec_max")
            << "</td><td>" << probe.at("actual_city_v13_footprint_score") << "</td></tr>";
    }
    out << "</tbody></table></div><p>Wall-sector effects remain an explicitly experimental hypothesis and are disabled here. "
        << "The existing World, transport, snapshots and save schemas are untouched.</p></main></html>\n";
    return out.str();
}

void write(const fs::path& path, std::string_view bytes) {
    std::ofstream stream(path, std::ios::binary);
    stream.exceptions(std::ios::failbit | std::ios::badbit);
    stream.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    stream.close();
}
} // namespace

int main(int argc, char** argv) {
    try {
        if (argc != 3) {
            std::cerr << "Usage: openemperor-desirability-compare GAME_DIRECTORY NEW_OUTPUT_DIRECTORY_BELOW_REPOSITORY_.local\n";
            return 2;
        }
        const auto game = fs::canonical(argv[1]);
        const auto output = output_path(argv[2], game);
        const auto loaded = original::load_model_data(game);
        if (!loaded) throw std::runtime_error("model load failed: " + loaded.error->reason);
        const auto& model = *loaded.model;
        constexpr auto difficulty = original::Difficulty::Normal;
        std::vector<Source> sources;
        const auto add = [&](original::ModelBuilding id, sim::Object kind, std::string_view name, int x, int y) {
            const auto footprint = sim::building_footprint(sim::RulesProfile::CityV13, 1, kind);
            if (footprint.width != footprint.height) throw std::logic_error("comparison source must be square");
            sources.push_back({id, kind, name, {{x, y, footprint.width}, original::spread_spec(model, id, difficulty)}});
        };
        add(original::ModelBuilding::Kiln, sim::Object::Pottery, "Pottery", 6, 6);
        add(original::ModelBuilding::CommonMarketSquare, sim::Object::Market, "Market", 14, 10);
        add(original::ModelBuilding::ClayPit, sim::Object::ClaySource, "ClaySource", 0, 17);

        original::DesirabilityMap map(width, height);
        for (const auto& source : sources) map.apply_source(source.source);
        std::array<Grid, 3> grids{};
        grids[1] = manhattan_control(sources);
        for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x) {
            grids[0][index(x, y)] = map.at(x, y);
            grids[2][index(x, y)] = legacy_score({x, y, 1}, sources);
        }
        Json source_rows = Json::array(), house_rows = Json::array();
        for (std::size_t i = 0; i < sources.size(); ++i) {
            const auto& source = sources[i];
            const auto& f = source.source.footprint;
            const auto& s = source.source.spread;
            source_rows.push_back({{"order", i + 1}, {"model_id", original::model_id(source.model).value},
                {"model_label", model.building_label(original::model_id(source.model))},
                {"functional_relationship", "inferred analogy; not original identity"},
                {"footprint_provenance", "unchanged authored City-v13 dimensions, not original geometry"},
                {"footprint", {{"x", f.x}, {"y", f.y}, {"side", f.side}}},
                {"spread", {{"value", s.value}, {"step", s.step}, {"step_size", s.step_size}, {"range", s.range}}},
                {"city_v13_object", source.object_name}, {"city_v13_authored_impact", sim::desirability_impact(source.object)}});
        }
        std::ostringstream houses, cells;
        houses << "probe,x,y,side,original_square_max,manhattan_same_spec_max,actual_city_v13_footprint_score\n";
        for (const auto& probe : probes) {
            const auto f = probe.footprint;
            const int square = map.building_value(f);
            const int manhattan = maximum(grids[1], f);
            const int legacy = legacy_score(f, sources);
            if (square != maximum(grids[0], f)) throw std::logic_error("kernel footprint maximum disagrees with cell table");
            house_rows.push_back({{"name", probe.name}, {"x", f.x}, {"y", f.y}, {"side", f.side},
                {"original_square_max", square}, {"manhattan_same_spec_max", manhattan},
                {"actual_city_v13_footprint_score", legacy}});
            houses << probe.name << ',' << f.x << ',' << f.y << ',' << f.side << ',' << square << ',' << manhattan << ',' << legacy << '\n';
        }
        cells << "x,y,original_square,manhattan_same_spec,actual_city_v13_cell_score,square_minus_manhattan,square_minus_city_v13\n";
        for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x) {
            const auto i = index(x, y);
            cells << x << ',' << y << ',' << grids[0][i] << ',' << grids[1][i] << ',' << grids[2][i] << ','
                << grids[0][i] - grids[1][i] << ',' << grids[0][i] - grids[2][i] << '\n';
        }
        const auto& provenance = model.provenance();
        const Json report{{"format_version", 1}, {"task", "OE-03"}, {"status", "computed"},
            {"evidence_limit", "isolated static-contract comparison; not original runtime or human playtest parity"},
            {"model", {{"sha256", provenance.sha256}, {"byte_count", provenance.byte_count}, {"source_path", provenance.source_path}}},
            {"difficulty", "Normal"}, {"width", width}, {"height", height}, {"wall_mode", "None"},
            {"house_bonus", 0}, {"layout", "authored comparison fixture; never a production World or save"},
            {"column_semantics", {
                {"original_square", "bounded reverse-engineered rings, source-ordered per-add saturation; supplied footprints authored"},
                {"manhattan_same_spec", "geometry control: same spread fields, source order, per-add saturation and House maximum; Manhattan distance only"},
                {"actual_city_v13", "unchanged authored World helpers, existing impacts/radius/quarter weights, sum then final clamp; House uses per-source footprint distance"}}},
            {"sources", source_rows}, {"house_probes", house_rows},
            {"cell_differences", {{"square_vs_same_spec_manhattan", delta_summary(grids[0], grids[1])},
                                   {"square_vs_actual_city_v13", delta_summary(grids[0], grids[2])}}}};
        const auto json_bytes = report.dump(2) + '\n';
        const auto svg_bytes = heatmaps(grids, sources, provenance.sha256);
        const auto html_bytes = html(report);
        fs::create_directories(output.parent_path());
        if (!fs::create_directory(output)) throw std::runtime_error("cannot create fresh output directory");
        write(output / "report.json", json_bytes);
        write(output / "cells.csv", cells.str());
        write(output / "houses.csv", houses.str());
        write(output / "heatmaps.svg", svg_bytes);
        write(output / "index.html", html_bytes);
        std::cout << Json{{"status", "computed"}, {"output_directory", output.string()},
            {"sha256", provenance.sha256}, {"cell_differences", report.at("cell_differences")},
            {"house_probes", house_rows}}.dump(2) << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "desirability comparison: " << error.what() << '\n';
        return 1;
    }
}
