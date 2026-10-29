#!/usr/bin/env python3
"""Real paid Xia economy, separate-process replay and finite operating windows.

Technical completion and sustainable gameplay have separate results. Original
data is read-only; only explicit newly earned private saves are ever restored.
"""
import argparse
import json
import os
from pathlib import Path
import subprocess
import sys
import time

import test_paid_progression as proof
from test_original_longplay import prepared

REPO = proof.REPO
PRIVATE = REPO / ".local/original-economy/oe37"
VARIANTS = ("supplied12", "lean12", "compact-two", "three-food", "compact-three",
            "balanced-three", "compact-connected-three")
PAIRS = ("seed.save.json", "midpoint.save.json", "result.save.json", "metrics.json")


def integer(value, message, minimum=None):
    proof.require(type(value) is int and (minimum is None or value >= minimum), message)
    return value


def operating_windows(windows, begin, end):
    """Only complete adjacent actual-ledger windows qualify, never lifetime tax."""
    proof.require(isinstance(windows, list), "Missing operating windows")
    previous, previous_funds, run, longest = begin, None, 0, 0
    for row in windows:
        start = integer(row["begin_tick"], "Invalid window begin", begin)
        stop = integer(row["end_tick"], "Invalid window end", start)
        proof.require(start == previous and stop - start == 20400 and stop <= end,
                      "Operating windows are incomplete, overlapping or nonconsecutive")
        tax = integer(row["taxes"], "Invalid window tax", 0)
        upkeep = integer(row["upkeep"], "Invalid window upkeep", 0)
        purchases = integer(row["construction"], "Invalid window construction", 0)
        before = integer(row["funds_begin"], "Invalid window initial Funds")
        after = integer(row["funds_end"], "Invalid window final Funds")
        proof.require(previous_funds is None or previous_funds == before,
                      "Adjacent operating windows changed Funds between boundaries")
        proof.require(type(row["operating_cashflow"]) is int and
                      row["operating_cashflow"] == tax - upkeep and
                      after - before == tax - upkeep - purchases,
                      "Operating cashflow excludes a recurring cost or disagrees with actual Funds")
        run = run + 1 if tax >= upkeep else 0
        longest = max(longest, run)
        previous = stop
        previous_funds = after
    proof.require(len(windows) == (end - begin) // 20400,
                  "Missing complete operating window")
    return longest


def finance_events(metrics, seed, final):
    """Bind claimed subwindows to the actual observed invoice/payment stream."""
    begin, end = metrics["begin_tick"], metrics["end_tick"]
    events = metrics["finance"]
    proof.require(isinstance(events, list), "Missing actual finance events")
    previous, taxes, upkeep = begin, 0, 0
    for event in events:
        tick = integer(event["tick"], "Invalid finance event tick", previous + 1)
        proof.require(tick <= end, "Finance event outside the observed interval")
        taxes += integer(event["tax"], "Invalid actual payment", 0)
        upkeep += integer(event["upkeep"], "Invalid actual invoice", 0)
        previous = tick
    proof.require(taxes == final["taxes_collected_total"] - seed["taxes_collected_total"] and
                  upkeep == final["maintenance_spent_total"] - seed["maintenance_spent_total"],
                  "Actual finance events do not reconcile raw tax/upkeep counters")
    windows = metrics["summary"]["windows20400"]
    if windows:
        proof.require(windows[0]["funds_begin"] == seed["treasury"], "First operating window Funds differ from earned seed")
    for row in windows:
        selected = [e for e in events if row["begin_tick"] < e["tick"] <= row["end_tick"]]
        proof.require(sum(e["tax"] for e in selected) == row["taxes"] and
                      sum(e["upkeep"] for e in selected) == row["upkeep"],
                      "Profitable subwindow differs from actual invoices and payments")


def review_generation(d, rule):
    proof.require(type(rule) is int and rule in (11, 12, 13), "Unsupported explicit economy generation")
    descriptor = {"id": "sandbox-city-v16", "version": rule}
    if rule == 13: descriptor["market_logistics"] = "authored-needs-aware-market-v1"
    w = d["world"]
    proof.require(d["format"] == "openemperor-sandbox-save" and
                  all(type(value) is int for value in (d["schema_version"], d["rules"]["version"],
                      d["profiles"]["map_permissions"]["version"], w["original_production"]["development_revision"])) and
                  d["schema_version"] == (29 if rule == 13 else 28 if rule == 12 else 27) and
                  d["rules"] == descriptor and
                  d["profiles"]["map_permissions"]["version"] == 3 and
                  w["original_production"]["development_revision"] == 27,
                  "Saved rule/schema/policy/revision changed")
    return w


def review_save(path, rule, data):
    proof.safe(path)
    d = proof.load(path)
    w = review_generation(d, rule)
    proof.require(d["map"]["relative_path"] == "Cities/Xia.map" and
                  d["map"]["sha256"] == proof.digest(data / "Cities/Xia.map"),
                  "Saved original map identity changed")
    source = w["original_economy"]["model_source"]
    proof.require(source["difficulty"] == 2 and source["sha256"] == proof.MODEL_SHA,
                  "Saved unchanged Normal model identity changed")
    for field in ("ticks", "construction_spent_total", "taxes_collected_total", "maintenance_spent_total"):
        integer(w[field], "Invalid actual ledger: " + field, 0)
    integer(w["treasury"], "Invalid actual Funds")
    proof.require(w["treasury"] + w["construction_spent_total"] + w["maintenance_spent_total"] ==
                  12000 + w["taxes_collected_total"], "Paid Funds conservation failed")
    buildings, couriers = w["buildings"], w["couriers"]
    houses = w["original_economy"]["houses"]
    bodies = {b["id"]: b for b in buildings}
    homes = {h["building_id"]: h["house"] for h in houses}
    proof.require(0 < len(bodies) == len(buildings) <= 82 and len(couriers) <= 58 and
                  0 < len(homes) == len(houses) <= 20, "Paid identity or collection bounds changed")
    for rows, key in ((buildings, "id"), (couriers, "id"), (houses, "building_id")):
        ids = [integer(row[key], "Invalid paid ID", 1) for row in rows]
        proof.require(ids == sorted(set(ids)), "Paid IDs duplicated or unsorted")
    proof.require(all(c["owner"] in bodies for c in couriers) and all(i in bodies for i in homes),
                  "Paid entity owner missing")
    proof.require(sum(integer(h["occupants"], "Invalid residents", 0) for h in homes.values()) ==
                  w["original_production"]["residents_admitted"], "Population has a second authority")
    model_rows, _ = prepared(data / "Model/EmperorBuildingModels.txt")
    tax = 0
    records = w["original_production"]["finance"]["houses"]
    proof.require([r["building_id"] for r in records] == list(homes), "Tax owner bijection differs")
    for owner in records:
        proof.require(len(owner["rows"]) == 15, "Tax row count changed")
        paid = sum(integer(bucket["resident_units"], "Invalid tax units", 0) * model_rows[i][18]
                   for i, bucket in enumerate(owner["rows"]))
        proof.require(paid == bodies[owner["building_id"]]["taxes_paid_total"],
                      "Model-weighted House tax differs")
        tax += paid
    proof.require(tax == w["taxes_collected_total"], "Actual global tax differs")
    goods = w["original_goods"]
    fields = ("hemp", "ceramics", "tea", "silk", "bronzeware", "lacquerware")
    proof.require(len(goods["totals"]) == 6, "Goods domain changed")
    for g, total in enumerate(goods["totals"]):
        proof.require(total["good"] == g and total["imported"] == total["demolished"] == 0,
                      "Artificial goods grant or sink")
        held = sum(integer(h[fields[g]], "Invalid House stock", 0) for h in homes.values())
        held += sum(integer(s["amounts"][g], "Invalid owner stock", 0) for s in goods["stocks"])
        held += sum(integer(c["amount"], "Invalid cargo", 0) for c in goods["cargo"] if c["good"] == g)
        proof.require(total["produced"] == held + total["consumed"], "Goods conservation failed")
    food = w["original_production"]["food"]
    cap = 3 if rule in (12, 13) else 2
    proof.require(len(food["farms"]) <= cap and len(food["stocks"]) <= 24 + cap and
                  len(food["trips"]) <= 4 + cap, "Food authority exceeds its versioned bound")
    for crop in range(2):
        produced = sum(f["harvested"] for f in food["farms"] if f["crop"] == crop)
        stock = sum(integer(s["amounts"][crop], "Invalid Food stock", 0) for s in food["stocks"])
        consumed = sum(integer(s["consumed"][crop], "Invalid Food sink", 0) for s in food["stocks"])
        cargo = sum(t["loaded"][crop] - t["delivered"][crop] - t["returned"][crop] for t in food["trips"])
        proof.require(cargo >= 0 and produced == stock + consumed + cargo, "Food crop conservation failed")
    fire = w["original_production"]["fire_protection"]
    proof.require(len(fire["arrivals"]) <= 8192 and len(fire["taxes"]) <= 8192 and
                  fire["travelled_cells"] <= 65536, "Fire provenance bounds changed")
    return w


def goal_witness(goal, tick, world=None):
    proof.require(isinstance(goal, dict) and type(goal.get("reached")) is bool and
                  type(goal.get("tick")) is int and goal["tick"] == tick,
                  "Missing tick-bound production World goal descriptor")
    if world is not None:
        values = {"funds": world["treasury"], "taxes": world["taxes_collected_total"],
                  "construction": world["construction_spent_total"], "maintenance": world["maintenance_spent_total"],
                  "population": world["original_production"]["residents_admitted"]}
        proof.require(all(type(goal.get(k)) is int and goal[k] == v for k, v in values.items()),
                      "World goal descriptor differs from the complete saved ledger")


def validate_process(report, metrics, rule, variant, ticks, mode):
    proof.require(report.get("status") == "PASS" and report.get("mode") == mode and
                  report.get("rule") == rule and report.get("schema") == (28 if rule == 12 else 27) and
                  report.get("policy") == 3 and report.get("revision") == 27 and
                  report.get("snapshot_injection") is False and report.get("variant") == variant and
                  report.get("creator_mode") is (mode == "create") and
                  report.get("restored_live_world") is (mode == "resume") and
                  report.get("input_preserved") is True and report.get("complete_recovery_equality") is True,
                  "Invalid process authority/creator/restore contract")
    proof.require(report.get("begin_tick") == 20000 and report.get("end_tick") == 20000 + ticks and
                  report.get("upkeep_interval") == (816 if rule == 12 else 400) and
                  metrics.get("begin_tick") == 20000 and metrics.get("end_tick") == 20000 + ticks and
                  metrics.get("upkeep_interval") == report["upkeep_interval"],
                  "Incomplete post-maturity interval or changed invoice contract")
    proof.require(metrics.get("format") == "openemperor-sustainable-economy-observation-v1" and
                  metrics.get("summary") == report.get("summary"), "Missing complete observation summary")
    if mode == "create":
        first = report.get("first_goal")
        if first is not None:
            tick = integer(first.get("tick"), "Invalid first World goal tick", 1)
            goal_witness(first, tick)
            proof.require(first["reached"] is True and tick <= 20000,
                          "First-goal claim lacks a true productive World witness")
        commands = report.get("initial_commands")
        proof.require(isinstance(commands, list) and commands, "Missing actual paid construction commands")
        for command in commands:
            cost = integer(command["cost"], "Invalid paid command cost", 0)
            proof.require(command.get("accepted", True) is True and
                          integer(command["funds_before"], "Invalid command initial Funds") >= cost and
                          integer(command["funds_after"], "Invalid command final Funds", 0) == command["funds_before"] - cost,
                          "Construction bypassed available Funds")
    else:
        proof.require(report.get("initial_commands") == [], "Restored continuation claimed fresh construction")
    maturity = report.get("maturity")
    proof.require(isinstance(maturity, dict) and type(maturity.get("ready")) is bool and
                  maturity.get("tick") == 20000, "Missing actual maturity observation")
    goal_witness(maturity.get("goal"), 20000)
    if mode == "create" and maturity["goal"]["reached"]:
        proof.require(report.get("first_goal") is not None, "Reached World goal lacks its first observed witness")
    if maturity["ready"]:
        proof.require(maturity["goal"]["reached"] is True and
                      integer(maturity["inhabited_houses"], "Invalid maturity House count", 12) >= 12 and
                      all(integer(maturity[k], "Missing actual maturity supply: " + k, 1) > 0 for k in
                          ("food_house_received", "goods_consumed", "actual_service_visits",
                           "actual_water_visits", "actual_inspector_arrivals")),
                      "Maturity lacks actual supply and production goal witnesses")
    goal_witness(metrics.get("final_goal"), 20000 + ticks)
    summary = metrics["summary"]
    taxes = integer(summary["taxes_delta"], "Invalid interval tax", 0)
    upkeep = integer(summary["upkeep_delta"], "Invalid interval upkeep", 0)
    proof.require(integer(summary["operating_cashflow"], "Invalid operating summary") == taxes - upkeep,
                  "Operating summary differs from actual tax/upkeep deltas")
    longest = operating_windows(summary["windows20400"], 20000, 20000 + ticks)
    proof.require(summary.get("two_consecutive_nonnegative_windows") is (longest >= 2) and
                  report.get("sustainable_operating_windows") is (longest >= 2) and
                  report.get("minimum_100000_post_maturity_ticks") is (ticks >= 100000),
                  "Sustainability label overstates observed windows or duration")
    return longest


def workflow(binary, data, output, resources, rule=12, variant="balanced-three", ticks=100000,
             timeout=1800, render=True):
    proof.require(type(rule) is int and rule in (11, 12) and variant in VARIANTS, "Unknown rule or strategy")
    proof.require(type(ticks) is int and 6000 <= ticks <= 150000, "Ticks must be in6000..150000")
    proof.require(type(timeout) is int and 1 <= timeout <= 7200, "Invalid process timeout")
    for path in (binary, data, output, resources, PRIVATE):
        proof.safe(path)
    proof.require(binary.is_file() and os.access(binary, os.X_OK) and data.is_dir() and resources.is_dir(),
                  "BLOCKED: executable, original installation or resources missing")
    proof.require(PRIVATE in output.parents and not output.exists(), "Output must be a new private OE37 directory")
    proof.require(not any(proof.overlaps(output, p) for p in (data, binary.parent, resources)),
                  "Output overlaps protected input")
    proof.require(proof.digest(data / "Model/EmperorBuildingModels.txt") == proof.MODEL_SHA,
                  "BLOCKED: unchanged Normal model required")
    before, executable, sources = proof.inventory(data), proof.record(binary), proof.sources()
    output.mkdir(parents=True)
    report = {"format": "openemperor-sustainable-economy-workflow-v1", "status": "FAIL", "sustainability": "FAIL",
              "rule": rule, "variant": variant, "ticks": ticks, "binary": executable,
              "sources": sources, "processes": [], "literal_pairs": [], "native_human": "NOT_RUN"}
    failure = None

    def child(mode, target, input_path=None):
        argv = [str(binary), "--mode", mode, "--rule", str(rule), "--variant", variant, "--data", str(data),
                "--output", str(target), "--ticks", str(ticks)]
        if input_path:
            argv += ["--input", str(input_path)]
        if mode == "render":
            argv += ["--resources", str(resources)]
        log = output / (mode + ".log")
        started, timed_out = time.monotonic(), False
        with log.open("x") as stream:
            process = subprocess.Popen(argv, stdout=stream, stderr=subprocess.STDOUT)
            try:
                code = process.wait(timeout=timeout)
            except subprocess.TimeoutExpired:
                timed_out = True
                process.kill()
                code = process.wait()
        report["processes"].append({"argv": argv, "pid": process.pid, "exit_code": code, "timed_out": timed_out,
                                    "seconds": time.monotonic() - started, "console": proof.record(log)})
        (output / "processes.json").write_text(json.dumps(report["processes"], indent=2) + "\n")
        proof.require(not timed_out and code == 0, "Actual child failed or timed out: " + mode)
        result = proof.load(target / "report.json")
        if mode != "render":
            metrics = proof.load(target / "metrics.json")
            validate_process(result, metrics, rule, variant, ticks, mode)
        return result

    try:
        created, resumed = output / "create", output / "resume"
        creator = child("create", created)
        seed = created / "seed.save.json"
        seed_record = proof.record(seed)
        restored = child("resume", resumed, seed)
        proof.require(creator["maturity"] == restored["maturity"], "Restored maturity differs")
        for name in PAIRS:
            a, b = created / name, resumed / name
            proof.safe(a)
            proof.safe(b)
            proof.require(a.read_bytes() == b.read_bytes(), "Separate-process complete bytes differ: " + name)
            report["literal_pairs"].append({"creator": proof.record(a), "resumed": proof.record(b)})
            if name.endswith("save.json"):
                review_save(a, rule, data)
        proof.require(proof.record(seed) == seed_record, "Protected earned seed changed")
        seed_world = review_save(seed, rule, data)
        final_world = review_save(created / "result.save.json", rule, data)
        proof.require(sum(c["cost"] for c in creator["initial_commands"]) == seed_world["construction_spent_total"],
                      "Paid recipe cost differs from actual construction ledger")
        metrics = proof.load(created / "metrics.json")
        goal_witness(creator["maturity"]["goal"], 20000, seed_world)
        goal_witness(metrics["final_goal"], 20000 + ticks, final_world)
        for metric, saved in (("taxes_delta", "taxes_collected_total"), ("upkeep_delta", "maintenance_spent_total"),
                              ("construction_delta", "construction_spent_total")):
            proof.require(metrics["summary"][metric] == final_world[saved] - seed_world[saved],
                          "Interval summary differs from complete raw ledger: " + metric)
        finance_events(metrics, seed_world, final_world)
        seed_fire = seed_world["original_production"]["fire_protection"]["arrivals"]
        final_fire = final_world["original_production"]["fire_protection"]["arrivals"]
        actual_visits = len(final_fire) - len(seed_fire)
        proof.require(metrics["summary"]["inspector_arrivals"] == actual_visits and
                      len(metrics["fire_visits"]) == actual_visits and
                      metrics["summary"]["actual_extinguishments"] ==
                      sum(v[1] for v in final_fire) - sum(v[1] for v in seed_fire),
                      "Fire telemetry differs from complete actual arrival provenance")
        last = metrics["final"]
        proof.require(all(last[key] == final_world[field] for key, field in
                          (("tick", "ticks"), ("funds", "treasury"), ("taxes", "taxes_collected_total"),
                           ("maintenance", "maintenance_spent_total"), ("construction", "construction_spent_total"))),
                      "Final supply observation differs from complete saved World")
        longest = operating_windows(metrics["summary"]["windows20400"], 20000, 20000 + ticks)
        transport = metrics["transport"]
        def house_food_received(world):
            owners = {h["building_id"] for h in world["original_economy"]["houses"]}
            return sum(sum(s["received"]) for s in world["original_production"]["food"]["stocks"] if s["building_id"] in owners)
        food_receipts = house_food_received(final_world) - house_food_received(seed_world)
        goods_consumed = sum(t["consumed"] for t in final_world["original_goods"]["totals"]) - \
            sum(t["consumed"] for t in seed_world["original_goods"]["totals"])
        delivered = (food_receipts > 0 and goods_consumed > 0 and
                     all(transport.get(family, {}).get("arrivals", 0) > 0 for family in ("goods", "food")))
        visits = all(transport.get(family, {}).get("applied", 0) > 0 for family in ("services", "water"))
        protection = metrics["summary"]["inspector_arrivals"] > 0
        sustainable = (ticks >= 100000 and creator["maturity"]["ready"] is True and longest >= 2 and
                       metrics["final_goal"]["reached"] is True and
                       integer(metrics["final_goal"]["inhabited_houses"], "Invalid final World House count", 0) >= 12 and
                       integer(metrics["summary"]["minimum_inhabited_houses"], "Missing minimum actual House count", 0) >= 12 and
                       delivered and visits and protection)
        supplied = (delivered and visits and metrics["summary"]["food_missed_demands_delta"] == 0 and
                    metrics["summary"]["current_unmet_evaluations"] == 0)
        report.update(status="PASS", sustainability="PASS" if sustainable else "PARTIAL",
                      goods_supply="PASS" if supplied else "PARTIAL",
                      fire_protection="PASS" if protection else "PARTIAL", original_game_parity="PARTIAL",
                      consecutive_nonnegative_windows=longest, creator=creator, restored=restored,
                      actual_food_house_receipts_delta=food_receipts, actual_goods_consumed_delta=goods_consumed,
                      measured_operating_windows=metrics["summary"]["windows20400"])
        if render:
            target = output / "render"
            result = child("render", target, created / "result.save.json")
            proof.require(result.get("actual_sandbox_renderer") is True and result.get("complete_f5_f9_equality") is True,
                          "Production F5/F9/render evidence missing")
            proof.require(result.get("maintenance_caption_verified") is True and
                          result.get("maintenance_interval") == (816 if rule == 12 else 400),
                          "Production inspector displayed a wrong versioned upkeep interval")
            proof.require([c.get("zoom") for c in result.get("captures", [])] == [1, 2, 4], "Missing1x/2x/4x captures")
            for capture in result["captures"]:
                path = target / capture["file"]
                proof.safe(path)
                proof.require(target in path.parents and path.read_bytes().startswith(b"\x89PNG\r\n\x1a\n"),
                              "Invalid production capture")
            report["render"] = result
    except Exception as error:
        failure = error
        report.update(status="FAIL", error=str(error), sustainability="FAIL")
    finally:
        try:
            after = proof.inventory(data)
            report.update(original_inventory_before=before, original_inventory_after=after,
                          original_bytes_modes_mtimes_unchanged=before == after,
                          executable_unchanged=executable == proof.record(binary), sources_unchanged=sources == proof.sources())
            proof.require(report["original_bytes_modes_mtimes_unchanged"] and report["executable_unchanged"] and
                          report["sources_unchanged"], "Protected original bytes, executable or sources changed")
        except Exception as error:
            failure = error
            report.update(status="FAIL", error=str(error), sustainability="FAIL")
        (output / "acceptance.json").write_text(json.dumps(report, indent=2) + "\n")
    if failure:
        raise failure
    return report


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for key in ("binary", "data", "output", "resources"):
        p.add_argument("--" + key, required=True, type=Path)
    p.add_argument("--rule", type=int, choices=(11, 12), default=12)
    p.add_argument("--variant", choices=VARIANTS, default="balanced-three")
    p.add_argument("--ticks", type=int, default=100000)
    p.add_argument("--timeout", type=int, default=1800)
    p.add_argument("--no-render", action="store_true")
    a = p.parse_args()
    try:
        result = workflow(a.binary.absolute(), a.data.absolute(), a.output.absolute(), a.resources.absolute(),
                          a.rule, a.variant, a.ticks, a.timeout, not a.no_render)
        print("OE37 technical " + result["status"] + "; sustainable economy " + result["sustainability"] + "; human NOT_RUN")
        return 0
    except Exception as error:
        print("OE37 FAIL: " + str(error), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
