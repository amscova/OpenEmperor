#!/usr/bin/env python3
"""OE38 diagnostic and unchanged paid-baseline gates; no high-tier grants."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import sys
import time

import test_sustainable_economy as baseline
import test_paid_progression as proof

PRIVATE = proof.REPO / ".local/original-economy/oe38"
COMMON_VARIANTS = ("common-reserved", "common-near-pottery", "common-two-market")
FIELDS = ("Water", "Herbalist", "Acupuncture", "Music", "Acrobat", "Drama", "Ancestor",
          "Confucian", "DaoistOrBuddhist", "FoodQuality", "Hemp", "Ceramics", "Tea", "Silk",
          "BronzewareOrLacquerware")
COLUMNS = (3, 2, 4, 5, 6, 7, 21, 22, 23, 8, 9, 10, 11, 12, 13)


def review_des_bound(bound, model, buildings):
    """Recheck the certificate's lattice inequalities using independent integers."""
    proof.require(bound["format"] == "openemperor-high-tier-des-bound-v1" and bound["verified"] is True and
                  bound["model_sha256"] == proof.MODEL_SHA and bound["snapshot_injection"] is False and
                  bound["actual_house_or_goal_witness"] is False and bound["bonus"] == 0 and
                  bound["non_overlapping_complete_footprints"] is True and bound["house_side"] == 4,
                  "Elite blocker lacks its exact source-bound conservative certificate")
    # (command, Object enum, original model, side, existing cap, source charge).
    expected = [(8,8,208,2,2,0), (15,12,207,2,2,0), (20,13,72,2,4,1),
                (26,19,214,2,2,5), (27,20,72,2,2,1), (28,21,215,2,2,5),
                (29,22,217,2,2,5), (30,23,219,3,2,3), (31,24,211,2,2,1),
                (32,25,212,3,2,1), (33,26,213,3,2,1)]
    actual = [tuple(row[key] for key in ("command", "object", "model_id", "side", "cap", "source_charge"))
              for row in bound["source_identities"]]
    proof.require(actual == expected and all(type(v) is int for row in actual for v in row),
                  "Elite certificate omitted/changed an admitted source, footprint or cap")
    cap_charge = sum(cap * charge for _, _, _, _, cap, charge in expected)
    proof.require(cap_charge == bound["source_cap_charge"] == 48 and len(bound["all16cells"]) == 16,
                  "Elite bound changed the complete source-cap/lattice domain")
    checked = positive = maximum = 0
    for n, case in enumerate(bound["all16cells"]):
        cx, cy = n % 4, n // 4; hx, hy = -cx, -cy
        def charge(x, y):
            if hx <= x < hx+4 and hy <= y < hy+4 or x % 2 or y % 2 or not (-4 <= x <= 4 and -4 <= y <= 4):return 0
            return 3 if -2 <= x <= 2 and -2 <= y <= 2 else 1
        lattice = [{"cell": [x, y], "charge": charge(x, y)} for y in range(-4, 5) for x in range(-4, 5) if charge(x, y)]
        lattice_sum = sum(row["charge"] for row in lattice);count = 0;slack = None
        for _, _, ident, side, _, source_charge in expected:
            values = buildings[ident];initial, period, step, reach = values[1:5];period = max(1, period);reach = max(0, min(10, reach))
            for y in range(-10-side+1, 11):
                for x in range(-10-side+1, 11):
                    if x < hx+4 and hx < x+side and y < hy+4 and hy < y+side:continue
                    checked += 1
                    distance = max(max(x, -x-side+1, 0), max(y, -y-side+1, 0))
                    value = initial if 1 <= distance <= reach else 0
                    if value:
                        for _ in range((distance-1)//period):value = min(value+step, 0) if initial < 0 else max(value+step, 0)
                    value = max(-100, min(100, value))
                    if value <= 0:continue
                    price = source_charge + sum(charge(x+dx, y+dy) for dy in range(side) for dx in range(side))
                    proof.require(price >= value, "Independent integer source/lattice inequality failed")
                    count += 1;positive += 1;slack = price-value if slack is None else min(slack, price-value)
        upper = cap_charge+lattice_sum;maximum = max(maximum, upper)
        proof.require(case["house_cell"] == [cx, cy] and case["translated_house_origin"] == [hx, hy] and
                      case["lattice"] == lattice and case["cell_charge"] == lattice_sum == 31 and
                      case["source_cap_charge"] == cap_charge and case["upper_bound"] == upper and
                      case["positive_placements_checked"] == count and case["minimum_integer_slack"] == slack,
                      "Elite certificate lattice/case count differs from independent exhaustive integer checks")
    threshold = model[13][1]
    proof.require(bound["placements_checked"] == checked and bound["positive_placements_checked"] == positive and
                  bound["upper_bound"] == maximum == 79 and bound["entry_threshold"] == threshold == 80 and
                  bound["blocks_entry"] is (maximum < threshold), "Elite blocker changed its complete inequality/entry threshold")
    return "BLOCKED" if maximum < threshold else "PARTIAL"


def owned(world, target, collection="houses"):
    rows = world["original_economy"]["houses"] if collection == "houses" else world["original_production"]["housing"]["houses"]
    matches = [row for row in rows if row["building_id"] == target]
    proof.require(len(matches) == 1, "Missing/duplicate declared target owner")
    return matches[0]


def retained(world, target):
    return [{"type": n + 3, "kind": row["kind"], "admitted": row["admitted"],
             "tick": row["evaluation"]["tick"], "input": row["evaluation"]["input"]}
            for n, row in enumerate(owned(world, target, "history")["first_transitions"]) if row is not None]


def observed_retained(rows):
    return [{"type": row["type"], "kind": row["kind"], "admitted": row["admitted"],
             "tick": row["evaluation"]["tick"], "input": row["evaluation"]["input"]} for row in rows]


def review_paid_commands(report, world, warmup, building_models):
    """Prices use the independent original model or the existing fixed contract."""
    fixed = {"Road": 2, "Household": 80, "Clay": 120, "Pottery": 180,
             "Warehouse": 150, "Market": 140, "FireWatch": 80,
             "Second Market": 140, "Second Market connection": 2}
    models = {"WaterWell": 72, "AncestorShrine": 214, "FoodFarm": 193, "MilletField": 196,
              "HempFarm": 192, "HempField": 194, "Ancestor": 214, "Daoist": 215, "Buddhist": 217,
              "Herbalist": 207, "Music": 211, "Acupuncture": 208, "Acrobat": 212,
              "Tea curing shed": 237, "Owned Tea bush": 26}
    bodies = {b["id"]: b for b in world["buildings"]}
    body_ids, roads, sequence, total = set(), set(), 0, 0
    for name, tick, start in (("initial_commands", 0, 12000), ("expansion_commands", 20000, warmup["final"]["funds"])):
        rows = report[name]
        proof.require(isinstance(rows, list) and rows, "Missing actual paid command group")
        funds = start
        for row in rows:
            label = row["command"]
            proof.require(label in fixed or label in models, "Unreviewed purchase in bounded Common recipe")
            price = fixed[label] if label in fixed else building_models[models[label]][0]
            for key in ("cost", "quoted_cost", "funds_before", "funds_after", "sequence_before", "sequence_after", "tick", "type"):
                baseline.integer(row[key], "Invalid typed purchase receipt: " + key)
            proof.require(row.get("accepted") is True and row.get("changed") is True and row["tick"] == tick and
                          row["cost"] == row["quoted_cost"] == price and funds >= price and
                          row["funds_before"] == funds and row["funds_after"] == funds - price and
                          row["sequence_before"] == sequence and row["sequence_after"] == sequence + 1,
                          "Purchase bypassed original price, actual available Funds or monotone commands")
            funds -= price; total += price; sequence += 1
            cell = tuple(row["cell"])
            if label in ("Road", "Second Market connection"):
                proof.require(cell not in roads and "building_id" not in row, "Duplicate or body-owning road purchase")
                roads.add(cell)
            else:
                ident = baseline.integer(row["building_id"], "Missing paid body ID", 1)
                proof.require(ident in bodies and ident not in body_ids and tuple(bodies[ident]["cell"]) == cell and
                              bodies[ident]["placed_tick"] == tick, "Paid body/cell/time lacks its actual raw purchase")
                body_ids.add(ident)
        if tick == 0:
            proof.require(total == warmup["final"]["construction"], "Warmup hid unreceipted construction")
        else:
            proof.require(funds == world["treasury"], "Expansion endpoint Funds differ from actual receipts")
    proof.require(total == world["construction_spent_total"] and sequence == world["command_sequence"] and
                  body_ids == set(bodies) and roads == {tuple(cell) for cell in world["roads"]},
                  "Raw complete expansion has missing/extra paid bodies, roads, costs or commands")


def review_needs(descriptor, state, model):
    row = baseline.integer(descriptor["model_row"], "Invalid observed model row", 0)
    proof.require(row < 15 and descriptor["desirability_low"] == model[row][0] and
                  descriptor["desirability_high"] == model[row][1] and descriptor["capacity"] == model[row][17],
                  "Observed capacity/DES thresholds differ from the independently prepared Normal model")
    actuals = [state["water"], state["herbalist"], state["acupuncture"], state["music"], state["acrobat"],
               state["drama"], state["ancestor"], state["confucian"], max(state["daoist"], state["buddhist"]),
               state["food_quality"], state["hemp"], state["ceramics"], state["tea"], state["silk"],
               max(state["bronzeware"], state["lacquerware"])]
    proof.require(len(descriptor["requirements"]) == 15, "Missing requirement/conjunction member")
    met = []
    for n, (name, column, actual, item) in enumerate(zip(FIELDS, COLUMNS, actuals, descriptor["requirements"])):
        need = model[row][column]; expected = need == 0 or actual != 0 if n < 9 else actual >= need
        proof.require(item["field"] == name and type(item["required"]) is int and item["required"] == need and
                      type(item["actual"]) is int and item["actual"] == actual and item["presence"] is (n < 9) and
                      item["met"] is expected, "Observed requirement/OR value differs from source and real evaluation input")
        met.append(expected)
    proof.require(descriptor["needs_met"] is all(met), "Observed needs conjunction differs from its actual source predicates")


def review_type_durations(metrics):
    begin, end = metrics["begin_tick"], metrics["end_tick"]
    current, cursor, counts = metrics["initial_state"]["type"], begin + 1, {}
    previous = begin
    for event in metrics["transition_events"]:
        tick = baseline.integer(event["tick"], "Invalid type-event tick", previous + 1)
        proof.require(tick <= end and event["from"] == current and 3 <= event["to"] <= 10 and
                      event["to"] != current and event["actual_state"]["type"] == event["to"],
                      "Type-event stream crossed family, skipped an origin or invented a change")
        counts[current] = counts.get(current, 0) + tick - cursor
        current, cursor, previous = event["to"], tick, tick
    counts[current] = counts.get(current, 0) + end + 1 - cursor
    expected = [{"type": kind, "post_tick_count": count} for kind, count in sorted(counts.items()) if count]
    proof.require(metrics["summary"]["type_tick_counts"] == expected and
                  metrics["summary"]["maximum_observed_type"] == max([metrics["initial_state"]["type"], *counts]),
                  "Type-duration/highest-tier summary differs from the complete transition event stream")
    return sum(count for kind, count in counts.items() if kind != 10)


def review_observation(metrics, seed, final, target, model):
    begin, end = seed["ticks"], final["ticks"]
    proof.require(metrics["format"] == "openemperor-high-tier-observation-v1" and
                  metrics["model_sha256"] == proof.MODEL_SHA and type(metrics["difficulty"]) is int and metrics["difficulty"] == 2 and
                  metrics["target"] == target and metrics["begin_tick"] == begin and metrics["end_tick"] == end and
                  metrics["observed_ticks"] == end - begin and metrics["independent_evolution_checks"] is True and
                  metrics["world_mutation_by_observer"] is False, "Observation lost its real source/target/tick/read-only contract")
    proof.require(metrics["initial_state"] == owned(seed, target)["house"] and
                  metrics["final_state"] == owned(final, target)["house"] and
                  observed_retained(metrics["initial_retained_transitions"]) == retained(seed, target) and
                  observed_retained(metrics["final_retained_transitions"]) == retained(final, target),
                  "Observation changed real endpoint states or retained transition input witnesses")
    city = metrics["city"]; summary = city["summary"]
    proof.require(city["begin_tick"] == begin and city["end_tick"] == end and city["upkeep_interval"] == 816 and
                  summary["taxes_delta"] == final["taxes_collected_total"] - seed["taxes_collected_total"] and
                  summary["upkeep_delta"] == final["maintenance_spent_total"] - seed["maintenance_spent_total"] and
                  summary["construction_delta"] == 0 and summary["operating_cashflow"] == summary["taxes_delta"] - summary["upkeep_delta"],
                  "Observed interval cashflow differs from complete real counters")
    baseline.operating_windows(summary["windows20400"], begin, end); baseline.finance_events(city, seed, final)
    ledger = metrics["final_target_ledger"]
    for name, field in (("funds", "treasury"), ("taxes", "taxes_collected_total"),
                        ("upkeep", "maintenance_spent_total"), ("construction", "construction_spent_total")):
        proof.require(ledger[name] == final[field] and city["final"][name if name != "upkeep" else "maintenance"] == final[field],
                      "Observation endpoint ledger differs from complete raw World")
    bodies = {b["id"]: b for b in final["buildings"]}
    proof.require(ledger["target_tax_paid"] == bodies[target]["taxes_paid_total"], "Target tax counter was invented")
    previous = begin
    evaluations = metrics["evaluations"]
    for event in evaluations:
        tick = baseline.integer(event["tick"], "Invalid evaluation tick", previous + 1)
        proof.require(tick <= end and (tick - 40) % 51 == 0 and event["independent_evolution_and_consumption_match"] is True,
                      "Repeated/out-of-phase/incomplete actual evaluation")
        actual = event["actual_evaluation"]
        proof.require(actual["tick"] == tick and actual["phase39_evaluation"] is True and
                      3 <= actual["input"]["type"] <= 10, "Common evaluation crossed its family or clock")
        review_needs(actual["current_needs"], actual["input"], model); review_needs(actual["next_needs"], actual["input"], model)
        account = event["tick_ledger"]
        proof.require(account["construction_delta"] == 0 and account["funds_after"] - account["funds_before"] ==
                      account["tax_delta"] - account["upkeep_delta"], "Evaluation tick ledger hid an injection/cost")
        prefix = 0
        for source in event["actual_des_source_prefixes"]:
            proof.require(source["id"] in bodies and source["origin"] == bodies[source["id"]]["cell"] and
                          source["before"] == prefix and source["after"] == max(-100, min(100, prefix + source["contribution"])),
                          "DES explanation has no actual paid source/prefix")
            prefix = source["after"]
        proof.require(prefix == actual["input"]["desirability"], "Evaluation DES differs from the paid source prefix")
        previous = tick
    proof.require(metrics["summary"]["evaluations"] == len(evaluations), "Evaluation count differs from full events")
    for name, predicate in (("current_unmet_evaluations", "current_needs"), ("next_unmet_evaluations", "next_needs")):
        proof.require(metrics["summary"][name] == sum(not e["actual_evaluation"][predicate]["needs_met"] for e in evaluations),
                      "Deficit count differs from actual evaluation events")
    initial = {row["type"] for row in retained(seed, target)}
    expected_new = [row for row in retained(final, target) if row["type"] not in initial]
    actual_new = [{"type": e["type"], "kind": e["kind"], "admitted": e["admitted"],
                   "tick": e["tick"], "input": e["evaluation"]["input"]} for e in metrics["new_first_transition_events"]]
    proof.require(sorted(actual_new, key=lambda e: e["type"]) == expected_new,
                  "New first-transition events differ from the actual newly retained witnesses")
    transitions = metrics["transition_events"]
    proof.require(metrics["summary"]["devolutions"] == sum(e["to"] < e["from"] for e in transitions) and
                  sum(row["post_tick_count"] for row in metrics["summary"]["type_tick_counts"]) == end - begin,
                  "Retention/type duration counters differ from actual event stream")
    review_type_durations(metrics)
    received_goods = [0] * 6; received_food = 0
    for event in metrics["target_delivery_events"]:
        units = event["physical_units"]["delivered"]
        delivered = sum(units) if isinstance(units, list) else units
        proof.require(begin < event["tick"] <= end and event["target"] == target and
                      event["cumulative_trip_delivered"] == delivered and
                      0 <= event["new_delivered_units"] <= delivered and
                      event["actual_arrival"] is (event["new_delivered_units"] > 0), "Dispatch was mistaken for an actual physical receipt")
        if event["family"] == "goods":
            good = baseline.integer(event["physical_units"]["good"], "Invalid actual delivered good", 0)
            proof.require(good < 6, "Unsupported delivered good")
            received_goods[good] += event["new_delivered_units"]
        else:
            proof.require(event["family"] == "food", "Unknown target delivery family")
            received_food += event["new_delivered_units"]
    goods_fields = ("hemp", "ceramics", "tea", "silk", "bronzeware", "lacquerware")
    before_used = next(row["consumed"] for row in seed["original_production"]["house_consumption"] if row["building_id"] == target)
    after_used = next(row["consumed"] for row in final["original_production"]["house_consumption"] if row["building_id"] == target)
    for good, field in enumerate(goods_fields):
        proof.require(received_goods[good] == owned(final, target)["house"][field] - owned(seed, target)["house"][field] +
                      after_used[good] - before_used[good], "Observed deliveries differ from actual target stock/sink conservation")
    food_before = next(row for row in seed["original_production"]["food"]["stocks"] if row["building_id"] == target)
    food_after = next(row for row in final["original_production"]["food"]["stocks"] if row["building_id"] == target)
    proof.require(received_food == sum(food_after["received"]) - sum(food_before["received"]),
                  "Observed Food arrival units differ from the actual target receipt counters")
    for event in metrics["target_visit_events"]:
        proof.require(begin < event["tick"] <= end and event["provider"] in bodies and
                      event["travel_ticks"] == event["tick"] - event["dispatched_tick"] > 0 and
                      type(event["visit_applied"]) is bool, "Visitor arrival lacks a paid provider and real travel")


def review_first_common(reached, first, target):
    home = owned(first, target)["house"]
    witness = owned(first, target, "history")["first_transitions"][7]
    body = next(b for b in first["buildings"] if b["id"] == target)
    proof.require(home["type"] == 10 and home["occupants"] > 0 and body["fire_until_tick"] <= first["ticks"] and
                  witness is not None and witness["kind"] == 1 and witness["admitted"] == 0 and
                  witness["evaluation"]["input"]["type"] == 9 and witness["evaluation"]["tick"] == first["ticks"] and
                  reached["house"] == target and reached["type"] == 10 and reached["tick"] == first["ticks"] and
                  reached["funds"] == first["treasury"] and reached["construction"] == first["construction_spent_total"] and
                  reached["exact_live_save"] == "first-success.save.json", "Common10 PASS lacks its genuine first9→10 evolution witness")
    return witness


def review_common(report, warmup, search, expansion, final, model, buildings, first=None, retention=None):
    proof.require(report["format"] == "openemperor-paid-high-tier-common-v1" and
                  report["technical_status"] == "PASS" and report["snapshot_injection"] is False and
                  all(type(report[key]) is int and report[key] == value for key, value in
                      (("rule", 12), ("schema", 28), ("policy", 3), ("revision", 27))) and
                  report["variant"] in COMMON_VARIANTS and report["common10"] in ("PASS", "PARTIAL") and
                  report["elite17"] in ("PARTIAL", "BLOCKED") and report["original_parity"] == "PARTIAL",
                  "Invalid Common scenario contract or invented full-parity claim")
    if "elite_entry_bound" in report:
        proof.require(report["elite17"] == review_des_bound(report["elite_entry_bound"], model, buildings),
                      "Elite headline differs from its independently checked bound")
    else:proof.require(report["elite17"] == "PARTIAL", "Elite BLOCKED claim has no certificate")
    target = baseline.integer(report["target_house"], "Invalid actual Common target", 1)
    proof.require(expansion["ticks"] == 20000 and owned(expansion, target)["house"]["type"] < 10 and
                  all(owned(w, target)["house"]["type"] < 11 for w in (expansion, final)),
                  "Common target was pre-granted or crossed its separate family")
    review_paid_commands(report, expansion, warmup, buildings)
    proof.require(report["final_tick"] == final["ticks"] and report["final_funds"] == final["treasury"] and
                  report["final_type"] == owned(final, target)["house"]["type"] and
                  final["construction_spent_total"] == expansion["construction_spent_total"] and
                  final["command_sequence"] == expansion["command_sequence"], "Scenario endpoint/commands differ from full saves")
    if report["common10"] == "PASS":
        proof.require(first is not None and retention is not None and report["first_reached"] is not None,
                      "Common10 PASS requires an exact first live save and retention")
        witness = review_first_common(report["first_reached"], first, target)
        proof.require(final["ticks"] - first["ticks"] == report["retention_ticks"] == 20400 and
                      report["search_ticks"] == first["ticks"] - 20000, "Incomplete first-reach/retention interval")
        review_observation(search, expansion, first, target, model); review_observation(retention, first, final, target, model)
        reached_event = [e for e in search["evaluations"] if e["tick"] == first["ticks"]]
        proof.require(len(reached_event) == 1 and reached_event[0]["actual_evaluation"]["input"] == witness["evaluation"]["input"] and
                      reached_event[0]["actual_evaluation"]["pure_result"]["type"] == 10 and
                      reached_event[0]["actual_evaluation"]["current_needs"]["model_row"] == 6 and
                      reached_event[0]["actual_evaluation"]["next_needs"]["model_row"] == 7 and
                      reached_event[0]["actual_evaluation"]["current_needs"]["needs_met"] is True and
                      reached_event[0]["actual_evaluation"]["next_needs"]["needs_met"] is True and
                      witness["evaluation"]["input"]["desirability"] >= model[6][1],
                      "First10 save lacks its actual source-bound successful evaluation")
        for key, value in (("operating_cashflow", retention["city"]["summary"]["operating_cashflow"]),
                           ("minimum_retention_funds", retention["summary"]["minimum_funds"]),
                           ("goal_loss_ticks", retention["city"]["summary"]["goal_loss_ticks"])):
            proof.require(report[key] == value, "Retention headline differs from full observed events: " + key)
        counts = retention["summary"]["type_tick_counts"]
        loss_floor = sum(row["post_tick_count"] for row in counts if row["type"] != 10)
        proof.require(report["type_lost_ticks"] == loss_floor, "Type loss headline differs from actual event durations")
        proof.require(type(report["target_lost_ticks"]) is int and loss_floor <= report["target_lost_ticks"] <= 20400 and
                      report["retention"] == ("PASS" if report["target_lost_ticks"] == 0 else "PARTIAL") and
                      report["financial_stability"] == ("PASS" if report["operating_cashflow"] >= 0 and report["minimum_retention_funds"] >= 0 else "FAIL") and
                      report["complete_recovery_equality"] is True, "Retention/profit/Recovery claim lacks actual evidence")
        endpoint = owned(final, target)["house"]; end_body = next(b for b in final["buildings"] if b["id"] == target)
        proof.require(report["retained_at_end"] is (endpoint["type"] == 10 and endpoint["occupants"] > 0) and
                      report["eligible_nonburning_at_end"] is (endpoint["type"] == 10 and endpoint["occupants"] > 0 and end_body["fire_until_tick"] <= final["ticks"]) and
                      report["final_goal"] is retention["city"]["final_goal"]["reached"],
                      "End retention/eligibility/World goal headline differs from actual endpoint")
    else:
        proof.require(first is None and retention is None and report["first_reached"] is None and
                      report["retention"] == "NOT_RUN" and report["retention_ticks"] == 0 and
                      report["financial_stability"] == "PARTIAL" and report["complete_recovery_equality"] is False and
                      report["search_ticks"] == final["ticks"] - 20000 and
                      owned(final, target, "history")["first_transitions"][7] is None,
                      "Failed search acquired a fake first success or retention claim")
        review_observation(search, expansion, final, target, model)
    proof.require(report["highest_actual_type"] == search["summary"]["maximum_observed_type"],
                  "Highest actual target type differs from observed events")


def common_workflow(binary, data, output, variant="common-reserved", search_ticks=30000, timeout=1800):
    proof.require(variant in COMMON_VARIANTS and type(search_ticks) is int and 6000 <= search_ticks <= 60000,
                  "Unknown Common variant or unsupported search bound")
    proof.require(type(timeout) is int and 1 <= timeout <= 7200, "Invalid process timeout")
    for path in (binary, data, output, PRIVATE):proof.safe(path)
    proof.require(binary.is_file() and os.access(binary, os.X_OK) and data.is_dir(), "Executable/original installation missing")
    proof.require(PRIVATE in output.parents and not output.exists() and
                  not any(proof.overlaps(output, p) for p in (data, binary.parent)), "New isolated private OE38 output required")
    proof.require(proof.digest(data / "Model/EmperorBuildingModels.txt") == proof.MODEL_SHA, "Unchanged original Normal model required")
    original, sources, executable = proof.inventory(data), proof.sources(), proof.record(binary)
    output.mkdir(parents=True)
    command = [str(binary), "--mode", "common", "--variant", variant, "--data", str(data),
               "--output", str(output / "common"), "--search-ticks", str(search_ticks)]
    result = {"format": "openemperor-high-tier-common-workflow-v1", "stage": "B-common", "status": "FAIL",
              "technical_status": "FAIL", "common10": "PARTIAL", "elite17": "PARTIAL", "retention": "NOT_RUN",
              "separate_process_continuation": "NOT_RUN", "rendering": "NOT_RUN", "native_human": "NOT_RUN",
              "command": command, "executable": executable, "sources": sources}
    started = time.monotonic()
    try:
        with (output / "process.log").open("w") as log:
            child = subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, timeout=timeout, check=False)
        proof.require(child.returncode == 0, "Common process failed; inspect process.log")
        folder = output / "common"; report = proof.load(folder / "report.json")
        expansion = baseline.review_save(folder / "expansion.save.json", 12, data)
        reached = report["common10"] == "PASS"
        first = baseline.review_save(folder / "first-success.save.json", 12, data) if reached else None
        final = baseline.review_save(folder / ("result.save.json" if reached else "search-end.save.json"), 12, data)
        retention = proof.load(folder / "retention.json") if reached else None
        if not reached:proof.require(not (folder / "first-success.save.json").exists() and not (folder / "retention.json").exists(), "Failed search published fake success artifacts")
        model, buildings = baseline.prepared(data / "Model/EmperorBuildingModels.txt")
        review_common(report, proof.load(folder / "warmup.json"), proof.load(folder / "search.json"), expansion, final,
                      model, buildings, first, retention)
        proof.require(report["variant"] == variant and (reached or report["search_ticks"] == search_ticks), "Process changed requested recipe/bound")
        if reached:
            recovery = list((folder / "recovery-app").rglob("*.json"))
            proof.require(any(p.read_bytes() == (folder / "result.save.json").read_bytes() for p in recovery), "Recovery headline lacks literal complete result-save bytes")
        result.update(status=report["common10"], technical_status="PASS", common10=report["common10"], elite17=report["elite17"],
                      retention=report["retention"], financial_stability=report["financial_stability"],
                      complete_saved_world_validated=True, scenario=report)
    except Exception as error:
        result.update(error=str(error));raise
    finally:
        result.update(elapsed_seconds=time.monotonic()-started, original_bytes_modes_mtimes_unchanged=original == proof.inventory(data),
                      sources_unchanged=sources == proof.sources(), executable_unchanged=executable == proof.record(binary))
        if not all(result[k] for k in ("original_bytes_modes_mtimes_unchanged", "sources_unchanged", "executable_unchanged")):
            result.update(status="FAIL", technical_status="FAIL", error="Protected original/source/executable identity changed")
        (output / "common-acceptance.json").write_text(json.dumps(result, indent=2)+"\n")
    proof.require(result["technical_status"] == "PASS", result.get("error", "Common technical gate failed"))
    return result


def review_matrix(matrix):
    proof.require(matrix.get("format") == "openemperor-high-tier-matrix-v1" and
                  matrix.get("rule") == 12 and matrix.get("policy") == 3 and
                  matrix.get("model_sha256") == proof.MODEL_SHA and matrix.get("difficulty") == 2,
                  "Matrix lost its exact unchanged Normal Rule12 source identity")
    proof.require(all(type(matrix[k]) is int for k in ("rule", "policy", "difficulty")),
                  "Matrix identity must use exact integer fields")
    proof.require(matrix.get("independent_house_rules_match") is True and
                  matrix.get("query_world_unchanged") is True and
                  matrix.get("snapshot_injection") is False and
                  matrix.get("high_tier_success_claim") is False,
                  "A diagnostic table cannot grant or claim an actual high-tier event")
    rows = matrix.get("types16")
    proof.require(isinstance(rows, list) and len(rows) == 16 and
                  [r.get("type") for r in rows] == list(range(2, 18)),
                  "Missing, duplicate or reordered House type in source matrix")
    homes = matrix.get("houses")
    proof.require(isinstance(homes, list) and len(homes) >= 2, "Paid Common/Elite observers missing")
    proof.require(all(type(h.get("elite_family")) is bool for h in homes) and
                  {h["elite_family"] for h in homes} == {False, True},
                  "The separate paid Common and Elite families must both be observed")
    proof.require(all(len(h.get("matrix15", [])) == 15 for h in homes), "Incomplete live World decision matrix")


def bind_matrix_world(matrix, world):
    """Only retained raw paid-state events can support an actual tier claim."""
    proof.require(matrix["tick"] == world["ticks"] and matrix["funds"] == world["treasury"] and
                  matrix["construction"] == world["construction_spent_total"],
                  "Matrix observation clock or ledger differs from complete saved World")
    homes = world["original_economy"]["houses"]
    bodies = {b["id"]: b for b in world["buildings"]}
    histories = {h["building_id"]: h for h in world["original_production"]["housing"]["houses"]}
    proof.require([h["id"] for h in matrix["houses"]] == [h["building_id"] for h in homes],
                  "Matrix paid observer identities differ from complete saved World")
    for observed, home in zip(matrix["houses"], homes):
        ident, actual = home["building_id"], home["house"]
        history = histories[ident]
        elite = actual["type"] >= 11
        side = 4 if elite and history["first_admission_tick"] is not None else 2
        proof.require(observed["actual_post_tick_state"] == actual and observed["cell"] == bodies[ident]["cell"] and
                      observed["elite_family"] is elite and observed["actual_footprint_side"] == side,
                      "Matrix actual House state/family/geometry differs from saved authority")
        expected = []
        for index, entry in enumerate(history["first_transitions"]):
            if entry is not None:
                expected.append({"type": index + 3, "tick": entry["evaluation"]["tick"], "kind": entry["kind"],
                                 "admitted": entry["admitted"], "evaluation_input": entry["evaluation"]["input"]})
        proof.require(observed["actual_first_transitions"] == expected,
                      "Matrix invented, omitted or changed an actual House transition")
    for row in matrix["types16"]:
        kind, expected = row["type"], []
        for home in homes:
            ident = home["building_id"]
            if kind in (2, 11):
                if home["house"]["type"] == kind:
                    expected.append({"house": ident, "kind": "paid empty plot, no occupied-tier proof"})
            else:
                entry = histories[ident]["first_transitions"][kind - 3]
                if entry is not None:
                    expected.append({"house": ident, "tick": entry["evaluation"]["tick"], "kind": entry["kind"]})
        proof.require(row["actual_witnesses"] == expected and
                      row["actual_occupied_tier_witnessed"] is (bool(expected) and kind not in (2, 11)),
                      "Matrix occupied-tier claim lacks exact saved admission/evolution witnesses")


def matrix_workflow(binary, data, output, input_path=None, timeout=180):
    for p in (binary, data, output, PRIVATE):
        proof.safe(p)
    proof.require(binary.is_file() and data.is_dir(), "Executable/original installation missing")
    proof.require(output != PRIVATE and PRIVATE in output.parents and not output.exists(),
                  "A new private OE38 output is required")
    proof.require(not any(proof.overlaps(output, p) for p in (data, binary.parent)),
                  "Output overlaps protected original/build input")
    if input_path is not None:
        proof.safe(input_path)
        proof.require(PRIVATE in input_path.parents and input_path.is_file() and
                      not proof.overlaps(input_path, output), "Explicit earned OE38 input required")
    before, sources, executable = proof.inventory(data), proof.sources(), proof.record(binary)
    seed = proof.record(input_path) if input_path is not None else None
    output.mkdir(parents=True)
    command = [str(binary), "--mode", "matrix", "--data", str(data), "--output", str(output / "matrix")]
    if input_path is not None:
        command += ["--input", str(input_path)]
    report = {"stage": "A-matrix", "status": "FAIL", "high_tier_success_claim": False,
              "command": command, "sources": sources, "executable": executable}
    started = time.monotonic()
    try:
        with (output / "process.log").open("w") as log:
            result = subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, timeout=timeout, check=False)
        proof.require(result.returncode == 0, "Matrix process failed; inspect process.log")
        matrix = proof.load(output / "matrix/matrix.json")
        review_matrix(matrix)
        world = baseline.review_save(output / "matrix/matrix-world.save.json", 12, data)
        bind_matrix_world(matrix, world)
        report.update(status="PASS", matrix=matrix, complete_saved_world_validated=True)
    except Exception as error:
        report.update(error=str(error))
        raise
    finally:
        report.update(elapsed_seconds=time.monotonic() - started,
                      original_bytes_modes_mtimes_unchanged=before == proof.inventory(data),
                      sources_unchanged=sources == proof.sources(), executable_unchanged=executable == proof.record(binary),
                      earned_input_unchanged=input_path is None or seed == proof.record(input_path))
        if not all(report[k] for k in ("original_bytes_modes_mtimes_unchanged", "sources_unchanged",
                                      "executable_unchanged", "earned_input_unchanged")):
            report.update(status="FAIL", error="Protected input/source identity changed")
        (output / "matrix-acceptance.json").write_text(json.dumps(report, indent=2) + "\n")
    proof.require(report["status"] == "PASS", report.get("error", "Matrix gate failed"))
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--stage", choices=("matrix", "baseline", "common"), required=True)
    for key in ("binary", "data", "output"):
        parser.add_argument("--" + key, type=Path, required=True)
    parser.add_argument("--resources", type=Path)
    parser.add_argument("--input", type=Path)
    parser.add_argument("--timeout", type=int, default=1800)
    parser.add_argument("--variant", choices=COMMON_VARIANTS, default="common-reserved")
    parser.add_argument("--search-ticks", type=int, default=30000)
    args = parser.parse_args()
    try:
        binary, data, output = (getattr(args, k).absolute() for k in ("binary", "data", "output"))
        if args.stage == "baseline":
            proof.require(args.resources is not None and args.input is None, "Baseline requires resources and no imported state")
            baseline.PRIVATE = PRIVATE
            result = baseline.workflow(binary, data, output, args.resources.absolute(), 12, "balanced-three", 100000, args.timeout, True)
            proof.require(all(result[k] == "PASS" for k in ("status", "sustainability", "goods_supply", "fire_protection")),
                          "OE37 baseline regression did not meet every existing gate")
        elif args.stage == "common":
            proof.require(args.input is None and args.resources is None, "Common creator requires fresh real state and no imported/resource overrides")
            result = common_workflow(binary, data, output, args.variant, args.search_ticks, args.timeout)
            print("OE38 Common10 " + result["common10"] + "; technical " + result["technical_status"] +
                  "; retention " + result["retention"] + "; separate-process/render NOT_RUN")
            return 0
        else:
            result = matrix_workflow(binary, data, output, args.input.absolute() if args.input else None, args.timeout)
        print("OE38 Stage A " + args.stage + " " + result["status"] + "; high-tier gameplay remains unproven")
        return 0
    except Exception as error:
        print("OE38 FAIL: " + str(error), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
