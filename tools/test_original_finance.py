#!/usr/bin/env python3
"""Paid original-model finance in all five difficulties and real process restarts."""
import argparse
import importlib.util
import json
from pathlib import Path
import re
import shutil
import subprocess
import time

_SPEC = importlib.util.spec_from_file_location("production_runner", Path(__file__).with_name("test_original_production.py"))
production = importlib.util.module_from_spec(_SPEC)
_SPEC.loader.exec_module(production)


def require(value, message):
    if not value:
        raise AssertionError(message)


def unique(pairs):
    result = {}
    for key, value in pairs:
        require(key not in result, "Duplicate JSON key: " + key)
        result[key] = value
    return result


def load(path):
    return json.loads(path.read_text(), object_pairs_hook=unique)


def number(value, lower=0, upper=(1 << 64)-1):
    require(type(value) is int and lower <= value <= upper, "Finance integer outside exact domain")
    return value


def prepared_model(path, difficulty):
    """Read the original text directly; no production model parser or report rates."""
    number(difficulty, 0, 4)
    sections = {key: [] for key in ("ALL HOUSES", "HOUSE MODS", "ALL BUILDINGS", "BUILDING MODS")}
    section = None
    for line in path.read_text(encoding="ascii").splitlines():
        line = line.strip()
        if line in sections:
            section = line
        elif section and "{," in line:
            prefix, body = line.split("{,", 1)
            count = 24 if section in ("ALL HOUSES", "HOUSE MODS") else 13
            tokens = body.split(",")[:count]
            require(len(tokens) == count and all(re.fullmatch(r"[+-]?\d+", token.strip()) for token in tokens), "Malformed model row")
            values = [number(int(token), -(1 << 31), (1 << 31)-1) for token in tokens]
            sections[section].append((prefix, values))
    houses = [v for _, v in sections["ALL HOUSES"]]
    house_mods = [v for _, v in sections["HOUSE MODS"]]
    building_mods = [v for _, v in sections["BUILDING MODS"]]
    require(len(houses) == 15 and len(house_mods) == len(building_mods) == 5, "Incomplete original model domain")
    prepared = [[max(-99, min(100, value + house_mods[difficulty][i])) for i, value in enumerate(row)] for row in houses]
    buildings = {}
    for prefix, row in sections["ALL BUILDINGS"]:
        identity = int(prefix.split(",", 1)[0])
        require(identity not in buildings, "Duplicate model building")
        products = [v * building_mods[difficulty][i] for i, v in enumerate(row)]
        buildings[identity] = [(abs(v)//100) * (-1 if v < 0 else 1) for v in products]
    return prepared, buildings


def review(document, report, model_path, difficulty):
    """Independently bind raw money/history, original rates, workforce and live goal."""
    require(document["schema_version"] == 26 and document["rules"]["version"] == 10, "Wrong finance save contract")
    w = document["world"]; p = w["original_production"]
    require(p["development_revision"] == report["revision"] == 27, "Wrong finance development revision")
    source = w["original_economy"]["model_source"]
    require(source["difficulty"] == report["difficulty"] == difficulty, "Saved selected difficulty changed")
    require(source["sha256"] == report["model_sha256"] == production.MODEL_SHA == production.digest(model_path), "Original model byte identity changed")
    rows, models = prepared_model(model_path, difficulty)
    rates = [row[18] for row in rows]
    require(report["rates"] == rates and all(0 <= n <= 100 for n in rates), "Reported rates differ from actual prepared original source")
    tick = number(w["ticks"]); require(report["tick"] == tick, "Report tick differs")
    bodies = {b["id"]: b for b in w["buildings"]}
    homes = {h["building_id"]: h["house"] for h in w["original_economy"]["houses"]}
    history = {h["building_id"]: h for h in p["housing"]["houses"]}
    require(len(bodies) == len(w["buildings"]) and len(homes) == len(w["original_economy"]["houses"]), "Duplicate paid identity")
    ledger = p["finance"]["houses"]
    require([h["building_id"] for h in ledger] == sorted(homes), "Finance House bijection changed")
    # This fixture buys only these independently identified physical source types.
    ids = {16:193, 20:72, 19:214}
    fixed = {6:0, 9:4, 18:0, 39:0, 40:0}
    required = {identity: models[ids[b["kind"]]][5] if b["kind"] in ids else fixed[b["kind"]] for identity, b in bodies.items()}
    available = sum(h["occupants"] for h in homes.values()); assigned = {identity:0 for identity in bodies}
    for identity, b in sorted(bodies.items(), key=lambda pair:(pair[1]["workforce_priority"], pair[0])):
        need = required[identity]
        if need and b["operating_enabled"] and b["fire_until_tick"] <= tick and available >= need:
            assigned[identity] = need; available -= need
    upkeep = 0; paid = 0; expected_bodies = []; expected_homes = []
    for identity, b in bodies.items():
        kind = b["kind"]
        require(kind in ids or kind in fixed, "Unreviewed finance fixture body")
        workers = models[ids[kind]][5] if kind in ids else fixed[kind]
        number(workers, 0, 32767); placed = number(b["placed_tick"], 0, tick)
        upkeep += 2 * workers * ((tick-placed)//400)
        expected_bodies.append({"id": identity, "kind": kind, "placed_tick": placed,
            "workers_required": workers, "workers_assigned": assigned[identity],
            "taxes_paid_total": b["taxes_paid_total"]})
        if identity not in homes:
            require(b["taxes_paid_total"] == 0, "Non-House acquired taxes")
    require(report["buildings"] == expected_bodies, "Raw body/model worker projection changed")
    for item in ledger:
        identity = item["building_id"]; home = homes[identity]; b = bodies[identity]
        buckets = item["rows"]; require(len(buckets) == 15, "Wrong finance row domain")
        total = events = 0
        occupants = number(home["occupants"], 0, 32767)
        for row, bucket in enumerate(buckets):
            count = number(bucket["events"], 0, tick//816)
            units = number(bucket["resident_units"], count, count*occupants)
            if row == 8 or (b["kind"] == 6 and row >= 8) or (b["kind"] in (39, 40) and row < 9):
                require(count == units == 0, "Tax history crosses paid House family")
            events += count; total += units*rates[row]
        require(events <= tick//816 and (item["last_assessment"] is None) == (events == 0), "Finance event timing differs")
        last = item["last_assessment"]
        if last:
            at = number(last["tick"], 1, tick); kind = number(last["type"], 3, 17)
            number(last["occupants"], 1, occupants)
            require(at % 816 == 0 and kind != 11 and buckets[kind-3]["events"] > 0 and buckets[kind-3]["resident_units"] >= last["occupants"], "Last actual tax input lacks monthly bucket")
            first = history[identity]["first_admission_tick"]
            require(first is not None and first < at, "Tax preceded actual first admission")
        require(total == b["taxes_paid_total"], "Raw sole House payment differs from original rate-weighted ledger")
        paid += total
        burning = b["fire_until_tick"] > tick
        eligible = occupants > 0 and not burning
        expected_homes.append({"id": identity, "type": home["type"], "occupants": occupants,
            "burning": burning, "rate": rates[max(0, home["type"]-3)], "eligible": eligible,
            "paid_total": total, "rows": buckets, "last_assessment": last})
    require(report["houses"] == expected_homes, "Raw House finance projection differs")
    tax = number(w["taxes_collected_total"]); maintenance = number(w["maintenance_spent_total"])
    construction = number(w["construction_spent_total"]); funds = number(w["treasury"], -(1 << 63), (1 << 63)-1)
    require(tax == paid and maintenance == upkeep and funds+construction+maintenance == 12000+tax, "Exact full signed finance ledger differs")
    require([report[k] for k in ("funds", "taxes", "maintenance", "construction")] == [funds, tax, maintenance, construction], "Raw reported money differs")
    population = sum(h["occupants"] for h in homes.values())
    inhabited = sum(h["occupants"] > 0 and not h["burning"] for h in expected_homes)
    common = sum(h["occupants"] > 0 and not h["burning"] and bodies[h["id"]]["kind"] == 6 and 5 <= h["type"] <= 10 for h in expected_homes)
    require(report["population"] == report["workforce_supply"] == population == p["residents_admitted"], "Population gained a second authority")
    require(report["goal"] == {"inhabited_houses": inhabited, "common_houses": common,
        "population": population, "positive_tax": tax > 0, "nonnegative_funds": funds >= 0,
        "reached": inhabited >= 6 and common >= 1 and population >= 50 and tax > 0 and funds >= 0}, "Live goal differs from actual current state")
    return {"tick": tick, "difficulty": difficulty, "taxes": tax, "upkeep": upkeep, "funds": funds}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("data", type=Path)
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    repo = Path(__file__).resolve().parents[1]; original = args.data.resolve(strict=True); build = args.build_dir.resolve(strict=True)
    production.preflight_output(repo, args.output, original, build)
    model = original/"Model/EmperorBuildingModels.txt"
    require(production.digest(model) == production.MODEL_SHA, "Expected pinned original model")
    binary = build/"openemperor-original-finance-fixture"
    require(binary.is_file(), "Build excluded finance fixture first")
    before = production.inventory(original); binary_sha = production.digest(binary)
    output = production.create_output(repo, args.output, original, build)
    stage = output/"data"; (stage/"Model").mkdir(parents=True); (stage/"Cities").mkdir()
    shutil.copy2(model, stage/"Model/EmperorBuildingModels.txt")
    (stage/"Cities/Influence.map").write_text("Authored24x20 finance acceptance geography; not an original map.\n")
    private_before = production.inventory(stage); processes = []; reviewed = []; pairs = 0; summaries = []
    def run(tag, arguments):
        log = output/(tag+".log"); start = time.monotonic(); argv = [str(binary), *map(str, arguments)]
        with log.open("x") as stream:
            child = subprocess.Popen(argv, stdout=stream, stderr=subprocess.STDOUT); code = child.wait()
        processes.append({"argv": argv, "actual_pid": child.pid, "exit_code": code,
            "seconds": round(time.monotonic()-start, 3), "log": str(log.relative_to(output)), "log_sha256": production.digest(log)})
        (output/"processes.json").write_text(json.dumps(processes, indent=2)+"\n")
        require(code == 0, "Actual finance process failed: "+str(log))
    for difficulty in range(5):
        created = output/f"difficulty-{difficulty}"
        run(f"create-{difficulty}", ["--create", stage, created, difficulty])
        fixture = load(created/"fixture.json")
        require(fixture["status"] == "PASS" and fixture["original_model"] is True and fixture["snapshot_injection"] is False, "Missing actual paid fixture")
        require(fixture["reached"]["goal"]["reached"] is True and fixture["reached"]["taxes"] > 0, "Difficulty never earned actual goal and positive tax")
        require([r["name"] for r in fixture["checkpoints"]] == ["paid-empty", "pre-first-tax", "post-first-tax", "mid-tax-cycle", "paid-live-goal"], "Incomplete actual checkpoint set")
        summaries.append(fixture["reached"])
        for item in fixture["checkpoints"]:
            name = item["name"]; source = created/item["save"]; raw_bytes = source.read_bytes()
            reviewed.append(review(load(source), load(created/item["report"]), model, difficulty))
            outputs = []
            for trial in range(2):
                target = output/f"resume-{difficulty}-{name}-{trial}"
                run(target.name, ["--resume", stage, target, source]); outputs.append(target)
                replay = load(target/"replay.json"); require(replay["status"] == "PASS" and replay["ticks"] == 20000, "Incomplete actual20k replay")
                for end in ("midpoint", "result"):
                    reviewed.append(review(load(target/(end+".json")), load(target/(end+".report.json")), model, difficulty))
            require(source.read_bytes() == raw_bytes, "Supplied save bytes changed")
            for end in ("midpoint", "result"):
                for suffix in (".json", ".report.json"):
                    require((outputs[0]/(end+suffix)).read_bytes() == (outputs[1]/(end+suffix)).read_bytes(), "Separate process whole artifact differs")
                    pairs += 1
                    if name == "paid-live-goal":
                        require((created/"expected"/(end+suffix)).read_bytes() == (outputs[0]/(end+suffix)).read_bytes(), "In-process continuation differs from actual separate process")
        print("Finance difficulty", difficulty, "PASS", flush=True)
    require(production.inventory(original) == before and production.inventory(stage) == private_before, "Source bytes/metadata changed")
    require(production.digest(binary) == binary_sha, "Actual executed binary changed")
    result = {"status": "PASS", "revision": 27, "all_five_difficulties": True, "original_model": True,
        "authored_geography": True, "human_playtest": "NOT_RUN", "snapshot_injection": False,
        "original_before_after": before, "private_before_after": private_before, "binary_sha256": binary_sha,
        "processes": processes, "actual_children": len(processes), "full_artifact_pairs": pairs,
        "reviewed_raw_save_documents": len(reviewed), "reviewed": reviewed, "positive_tax_goals": summaries,
        "limits": ["Direct tax timing/normalization, upkeep and mission are explicit authored contracts; original collector parity remains partial.",
            "Bounded tax history is not a complete event journal."]}
    (output/"report.json").write_text(json.dumps(result, indent=2)+"\n")
    print("Original finance PASS:", output)


if __name__ == "__main__":
    main()
