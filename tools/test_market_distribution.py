#!/usr/bin/env python3
"""OE39 actual paid transport diagnosis; immutable input and complete replay guards."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import sys
import time

import test_high_tier_progression as high

proof, baseline = high.proof, high.baseline
PRIVATE = proof.REPO / ".local/original-economy/oe39"
VARIANTS = (*high.COMMON_VARIANTS, "common-near-chain", "common-priority", "common-short-route")
CHECKPOINTS = ("expansion", "first-success", "before-shortage", "first-shortage",
               "first-devolution", "active-cargo", "result")


def integer(value, minimum=0):
    return baseline.integer(value, "Invalid actual transport integer", minimum)


def compare_files(left, right, names):
    pairs = []
    for name in names:
        a, b = left / name, right / name
        proof.safe(a); proof.safe(b)
        proof.require(a.is_file() and b.is_file() and a.read_bytes() == b.read_bytes(),
                      "Complete immutable artifact bytes differ: " + name)
        pairs.append({"reference": proof.record(a), "replayed": proof.record(b)})
    return pairs


def review_paid_commands(report, world, warmup, building_models):
    """Bind every construction and operation to actual IDs, prices, time and command sequence."""
    variant = report["variant"]
    proof.require(variant in VARIANTS, "Unreviewed paid player strategy")
    rows = report["expansion_commands"]
    priority = [row for row in rows if row["command"] == "Logistics priority"]
    roads = [row for row in rows if row["command"] == "Near chain road"]
    shortcuts = [row for row in rows if row["command"] == "Market shortcut road"]
    proof.require(len(priority) == (2 if variant == "common-priority" else 0) and
                  len(roads) == (4 if variant == "common-near-chain" else 0) and
                  len(shortcuts) == (4 if variant == "common-short-route" else 0),
                  "Paid variant omitted or added strategy-specific World commands")
    bodies = {row["id"]: row for row in world["buildings"]}
    expected = sorted(ident for ident, body in bodies.items() if body["kind"] in (3, 9))
    fixed = {"Road": 2, "Household": 80, "Clay": 120, "Pottery": 180, "Warehouse": 150,
             "Market": 140, "FireWatch": 80, "Second Market": 140, "Second Market connection": 2,
             "Near chain road": 2, "Market shortcut road": 2, "Logistics priority": 0}
    models = {"WaterWell": 72, "AncestorShrine": 214, "FoodFarm": 193, "MilletField": 196,
              "HempFarm": 192, "HempField": 194, "Ancestor": 214, "Daoist": 215, "Buddhist": 217,
              "Herbalist": 207, "Music": 211, "Acupuncture": 208, "Acrobat": 212,
              "Tea curing shed": 237, "Owned Tea bush": 26}
    identities = {"Household": (6, 6), "Clay": (3, 4), "Pottery": (4, 5), "Warehouse": (2, 3),
                  "Market": (9, 9), "Second Market": (9, 9), "FireWatch": (13, 10), "WaterWell": (27, 20),
                  "AncestorShrine": (26, 19), "FoodFarm": (23, 16), "MilletField": (25, 18),
                  "HempFarm": (21, 14), "HempField": (22, 15), "Ancestor": (26, 19), "Daoist": (28, 21),
                  "Buddhist": (29, 22), "Herbalist": (15, 12), "Music": (31, 24), "Acupuncture": (8, 8),
                  "Acrobat": (32, 25), "Tea curing shed": (40, 33), "Owned Tea bush": (43, 36)}
    owners, paid_roads, selected, sequence, total = set(), set(), [], 0, 0
    for name, tick, funds in (("initial_commands", 0, 12000), ("expansion_commands", 20000, warmup["final"]["funds"])):
        proof.require(isinstance(report[name], list) and report[name], "Missing actual paid command group")
        for row in report[name]:
            label = row["command"]
            proof.require(label in fixed or label in models, "Unreviewed paid command in distribution strategy")
            price = fixed[label] if label in fixed else building_models[models[label]][0]
            for field in ("type", "tick", "cost", "quoted_cost", "sequence_before", "sequence_after"):
                integer(row[field])
            for field in ("funds_before", "funds_after"):
                baseline.integer(row[field], "Invalid actual command Funds")
            proof.require(row["accepted"] is True and row["changed"] is True and row["tick"] == tick and
                          row["cost"] == row["quoted_cost"] == price and funds >= price and
                          row["funds_before"] == funds and row["funds_after"] == funds-price and
                          row["sequence_before"] == sequence and row["sequence_after"] == sequence+1,
                          "Paid command bypassed actual available Funds, source price or monotone sequence")
            funds -= price; total += price; sequence += 1
            cell = tuple(row["cell"])
            if label == "Logistics priority":
                owner = integer(row["selected_building_id"], 1)
                proof.require(tick == 20000 and row["type"] == 11 and owner in expected and
                              row["building_id"] == owner and row["workforce_priority"] == bodies[owner]["workforce_priority"] == 0 and
                              row["cell"] == bodies[owner]["cell"], "Priority command lacks its actual paid logistics owner")
                selected.append(owner)
            elif label in ("Road", "Second Market connection", "Near chain road", "Market shortcut road"):
                proof.require(row["type"] == 0 and cell not in paid_roads and "building_id" not in row,
                              "Duplicate, body-owning or non-PlaceRoad connection")
                paid_roads.add(cell)
            else:
                owner = integer(row["building_id"], 1)
                proof.require(owner in bodies and owner not in owners and tuple(bodies[owner]["cell"]) == cell and
                              bodies[owner]["placed_tick"] == tick and
                              (row["type"], bodies[owner]["kind"]) == identities[label],
                              "Purchase lacks its actual unique paid command/body/time/cell")
                owners.add(owner)
        if tick == 0:
            proof.require(total == warmup["final"]["construction"], "Warmup hid unreceipted construction")
        else: proof.require(funds == world["treasury"], "Expansion actual Funds differ from all construction/operation receipts")
    proof.require(total == world["construction_spent_total"] and sequence == world["command_sequence"] and
                  owners == set(bodies) and paid_roads == {tuple(cell) for cell in world["roads"]} and
                  (not priority or selected == expected), "Actual commands omit/duplicate paid authority or expenses")
    if roads:
        proof.require([row["relative_cell"] for row in roads] == [[17, y] for y in range(4)],
                      "Near-chain road plan changed its actual paid connection")
    if shortcuts:
        proof.require([row["relative_cell"] for row in shortcuts] == [[8, y] for y in range(5, 9)],
                      "Market shortcut changed its actual paid band connection")
    expected_cells = {"Clay": [18, 0] if variant == "common-near-chain" else [0, 0],
                      "Pottery": [18, 2] if variant in ("common-near-pottery", "common-near-chain") else [22, 2]}
    for name, cell in expected_cells.items():
        actual = [row for row in rows if row["command"] == name]
        proof.require(len(actual) == 1 and actual[0]["relative_cell"] == cell,
                      "Paid industrial comparison changed its declared source/route geometry")


def review_checkpoint_list(labels, worlds, variant):
    proof.require(isinstance(labels, list) and len(labels) == len(set(labels)) and
                  all(type(label) is str and label in CHECKPOINTS for label in labels) and
                  {"expansion", "first-success", "active-cargo", "result"} <= set(labels),
                  "Checkpoint labels omitted earned entry/cargo/end or introduced an unreviewed path")
    proof.require(set(worlds) == set(labels), "Checkpoint manifest differs from its complete saved Worlds")
    start, end = worlds["first-success"]["ticks"], worlds["result"]["ticks"]
    proof.require(end-start == 20400 and worlds["expansion"]["ticks"] == 20000,
                  "Incomplete actual paid progression/retention interval")
    for label, world in worlds.items():
        proof.require((label == "expansion" and world["ticks"] == 20000 or start <= world["ticks"] <= end) and
                      world["command_sequence"] == worlds["expansion"]["command_sequence"] and
                      world["construction_spent_total"] == worlds["expansion"]["construction_spent_total"],
                      "Checkpoint introduced post-expansion commands or escaped its actual retention interval")
    proof.require(worlds["active-cargo"]["original_goods"]["cargo"],
                  "Active-cargo checkpoint contains no physically loaded six-goods transport")
    if variant == "common-reserved":
        proof.require({"before-shortage", "first-shortage", "first-devolution"} <= set(labels) and
                      start == 27733 and worlds["first-shortage"]["ticks"] == 29008 and
                      worlds["first-devolution"]["ticks"] == 29467 and
                      start < worlds["before-shortage"]["ticks"] < 29008,
                      "Unchanged Rule12 lost its frozen real entry/shortage/devolution witnesses")


def review_resume(report, seed, final, target):
    proof.require(report["technical_status"] == "PASS" and report["snapshot_injection"] is False and
                  report["input_preserved"] is True and report["complete_recovery_equality"] is True and
                  report["command_sequence_unchanged"] is True and
                  all(type(report[key]) is int and report[key] == expected for key, expected in
                      (("rule", 12), ("schema", 28), ("policy", 3), ("revision", 27),
                       ("target", target), ("begin_tick", seed["ticks"]), ("end_tick", final["ticks"]))) and
                  seed["command_sequence"] == final["command_sequence"] and
                  seed["construction_spent_total"] == final["construction_spent_total"],
                  "Separate process altered generation, restored input, commands or complete Recovery")


GOODS = ("hemp", "ceramics", "tea", "silk", "bronzeware", "lacquerware")


def review_sample(sample, world, model, complete=True):
    """Check physical per-owner commitments separately from global conserved mass."""
    bodies = {row["id"]: row for row in world["buildings"]}
    owners, actors = sample["owners"], sample["couriers"]
    proof.require(len({row["id"] for row in owners}) == len(owners) and
                  len({row["id"] for row in actors}) == len(actors), "Duplicate trace owner/courier identity")
    stock, cargo = [0]*6, [0]*6
    for owner in owners:
        ident = integer(owner["id"], 1)
        proof.require(ident in bodies and owner["object"] == bodies[ident]["kind"] and
                      owner["cell"] == bodies[ident]["cell"] and
                      type(owner["operating"]) is bool and type(owner["burning"]) is bool and
                      0 <= integer(owner["workers_assigned"]) <= integer(owner["workers_required"]),
                      "Trace owner/staffing lacks its actual paid body")
        rows = owner["goods"]
        proof.require(not rows or [row["good"] for row in rows] == list(range(6)), "Incomplete owner goods domain")
        for row in rows:
            for key in ("stock", "source_reserved", "incoming_reserved", "outgoing_cargo_return_room",
                        "incoming_cargo", "native_pottery_incoming", "capacity", "headroom"):
                integer(row[key])
            proof.require(row["source_reserved"] <= row["stock"] and row["headroom"] == row["capacity"]-row["stock"]-
                          row["incoming_reserved"]-row["outgoing_cargo_return_room"]-row["native_pottery_incoming"],
                          "Per-owner headroom omitted actual stock, incoming commitment or cargo return room")
            stock[row["good"]] += row["stock"]
            if sample["tick"] == world["ticks"]:
                if owner["object"] == 6:
                    actual = high.owned(world, ident)["house"][GOODS[row["good"]]]
                else:
                    account = next(account for account in world["original_goods"]["stocks"] if account["building_id"] == ident)
                    actual = account["amounts"][row["good"]]
                proof.require(row["stock"] == actual, "Trace endpoint owner stock differs from its sole saved inventory")
    raw_actors = {row["id"]: row for row in world["couriers"]}
    for actor in actors:
        ident = integer(actor["id"], 1)
        proof.require(ident in raw_actors and actor["owner"] == raw_actors[ident]["owner"] and
                      actor["role"] == raw_actors[ident]["role"] and actor["target"] in bodies and
                      type(actor["route_pending"]) is bool, "Trace courier role/owner/target differs from paid actor authority")
        path = actor["path"]
        proof.require(isinstance(path, list) and all(len(cell) == 2 and all(type(v) is int for v in cell) for cell in path) and
                      integer(actor["path_vertex"]) < max(1, len(path)) and 0 <= integer(actor["edge_progress"]) < 5,
                      "Trace path/edge state is not a bounded actual Rule12 route")
        amount = integer(actor["typed_cargo"])
        trip = actor["latest_goods_trip"]
        if amount:
            proof.require(trip is not None and trip["source"] == actor["owner"] and trip["target"] == actor["target"] and
                          0 <= integer(trip["good"]) < 6 and amount == trip["loaded"]-trip["delivered"]-trip["returned"],
                          "Actual typed cargo does not match its saved transfer")
            cargo[trip["good"]] += amount
    balances = sample["goods_balances"]
    proof.require([row["good"] for row in balances] == list(range(6)), "Trace omitted a conserved goods domain")
    for good, row in enumerate(balances):
        for key in ("produced", "consumed", "stock", "cargo", "source_reserved", "incoming_reserved"):
            integer(row[key])
        proof.require(row["conserved"] is True and row["imported"] == row["demolished"] == 0 and
                      row["produced"] == row["consumed"]+row["stock"]+row["cargo"] and
                      (not complete or row["stock"] == stock[good] and row["cargo"] == cargo[good]),
                      "Trace stock/cargo/sinks do not reconcile actual conserved physical mass")
        if sample["tick"] == world["ticks"]:
            total = world["original_goods"]["totals"][good]
            proof.require(all(row[key] == total[key] for key in ("produced", "consumed", "imported", "demolished")),
                          "Trace endpoint invented its physical source/sink counters")
    if complete:
        proof.require([h["house"] for h in sample["houses"]] == [h["building_id"] for h in world["original_economy"]["houses"]],
                      "Trace omitted another paid household from fairness observation")
        for house in sample["houses"]:
            high.review_needs(house["post_tick_current_needs"], house["state"], model)
            high.review_needs(house["post_tick_next_needs"], house["state"], model)
            actual = house["actual_phase39_evaluation"]
            if actual is not None:
                proof.require(actual["tick"] == sample["tick"] and actual["phase39_evaluation"] is True and
                              (sample["tick"]-40) % 51 == 0, "Another House's actual evaluation escaped phase39")
                high.review_needs(actual["current_needs"], actual["input"], model)
                high.review_needs(actual["next_needs"], actual["input"], model)


def review_supply(report, retention):
    """A temporarily buffered type10 is not a continuously supplied Common10."""
    evaluations = retention["evaluations"]
    counts = {"actual": len(evaluations),
              "current_unmet": sum(not e["actual_evaluation"]["current_needs"]["needs_met"] for e in evaluations)}
    for field, key in (("Hemp", "hemp_unmet"), ("Ceramics", "ceramics_unmet")):
        counts[key] = sum(not next(row["met"] for row in e["actual_evaluation"]["current_needs"]["requirements"]
                                   if row["field"] == field) for e in evaluations)
    proof.require(report["supply_evaluations"] == counts and
                  all(type(value) is int for value in report["supply_evaluations"].values()),
                  "Sustained-supply counts differ from actual pre-consumption need evaluations")
    passed = (report["retention_ticks"] == retention["end_tick"]-retention["begin_tick"] == 20400 and
              counts["actual"] == 400 and counts["current_unmet"] == counts["hemp_unmet"] == counts["ceramics_unmet"] == 0 and
              report["target_lost_ticks"] == report["type_lost_ticks"] == 0 and
              retention["summary"]["burning_ticks"] == retention["summary"]["devolutions"] == 0)
    proof.require(report["sustained_supply"] == ("PASS" if passed else "PARTIAL"),
                  "Continuous-supply verdict accepted unmet evaluations, type loss, fire or an incomplete window")
    return report["sustained_supply"]


def review_trace(trace, observation, seed, final, target, model):
    begin, end = seed["ticks"], final["ticks"]
    proof.require(trace["format"] == "openemperor-market-distribution-trace-v1" and
                  trace["world_mutation_by_observer"] is False and trace["model_sha256"] == proof.MODEL_SHA and
                  all(type(trace[key]) is int and trace[key] == value for key, value in
                      (("rule", 12), ("target", target), ("begin_tick", begin), ("end_tick", end))),
                  "Actual transport trace changed source, rule, owner, clock or read-only contract")
    for label, world in (("initial", seed), ("final", final)):
        sample = trace[label]
        proof.require(sample["tick"] == world["ticks"] and sample["funds"] == world["treasury"] and
                      sample["taxes"] == world["taxes_collected_total"] and sample["upkeep"] == world["maintenance_spent_total"],
                      "Transport endpoint sample differs from complete raw World")
        review_sample(sample, world, model)
        for house in sample["houses"]:
            proof.require(house["state"] == high.owned(world, house["house"])["house"], "Trace endpoint invented a House inventory/type")
    samples = trace["evaluation_samples"]
    proof.require([sample["tick"] for sample in samples] == [event["tick"] for event in observation["evaluations"]] ==
                  [tick for tick in range(begin+1, end+1) if (tick-40) % 51 == 0],
                  "Trace omitted, invented or reordered a real phase39 evaluation")
    for sample, event in zip(samples, observation["evaluations"]):
        proof.require(sample["actual_evaluation"] == event["actual_evaluation"],
                      "Physical chain sample differs from the actual pre-evolution/pre-consumption World input")
        review_sample(sample, final, model)
    expected_window = list(range(max(begin+1, 27733), min(end, 29467)+1))
    proof.require([sample["tick"] for sample in trace["detailed_window"]] == expected_window,
                  "Detailed first-failure transport window omitted actual ticks")
    for sample in trace["detailed_window"]: review_sample(sample, final, model, False)
    bodies = {row["id"]: row for row in final["buildings"]}
    actors = {row["id"]: row for row in final["couriers"]}
    received = {row["building_id"]: [0]*6 for row in final["original_economy"]["houses"]}
    previous_tick = begin
    for event in trace["transfer_events"]:
        tick = integer(event["tick"], previous_tick)
        proof.require(begin < tick <= end and event["family"] in ("goods", "native_clay", "native_pottery") and
                      event["event"] in ("dispatch_load", "delivery", "cancel_return", "source_return", "courier_home") and
                      event["courier"] in actors and event["source"] == actors[event["courier"]]["owner"] and
                      event["target"] in bodies and event["source_post_tick"]["id"] == event["source"] and
                      event["target_post_tick"]["id"] == event["target"], "Transfer event lacks its real paid courier/source/destination")
        previous_tick = tick
        loaded, delivered, returned = (integer(event[key]) for key in ("loaded", "delivered", "returned"))
        new_delivered, new_returned = (integer(event[key]) for key in ("new_delivered_units", "new_returned_units"))
        proof.require(0 <= new_delivered <= delivered <= loaded <= 4 and 0 <= new_returned <= returned and
                      delivered+returned <= loaded and
                      (event["event"] == "delivery") is (new_delivered > 0) and
                      (event["event"] == "source_return") is (new_returned > 0),
                      "Dispatch, arrival and source return were conflated or created physical mass")
        edges = integer(event["route_edges"])
        proof.require(event["minimum_one_way_ticks"] == edges*5 and event["minimum_round_trip_ticks"] == edges*10,
                      "Throughput bound changed real Rule12 edge time or omitted return travel")
        departure = event["departure_tick"]
        proof.require(departure is None or integer(departure) <= tick,
                      "Observed loading has an invented/future departure tick")
        if departure is not None and departure <= begin:
            trips = seed["original_production"]["industry"]["clay_trips"] if event["family"] == "native_clay" else []
            proof.require(any(row["transfer"]["courier_id"] == event["courier"] and
                              row["transfer"]["sequence"] == event["sequence"] and row["dispatched_tick"] == departure
                              for row in trips), "Pre-window departure is not the seed's actual retained Clay transfer/time")
        if new_delivered:
            proof.require(event["arrival_tick"] == tick and (departure is None or tick-departure+1 >= edges*5),
                          "Claimed actual arrival predates physical route travel")
        if event["family"] == "goods":
            good = integer(event["good"])
            proof.require(good < 6 and integer(event["sequence"], 1) <= tick,
                          "Typed goods event lost its saved commodity/monotone transfer handle")
            if event["target"] in received: received[event["target"]][good] += new_delivered
    before_used = {row["building_id"]: row["consumed"] for row in seed["original_production"]["house_consumption"]}
    after_used = {row["building_id"]: row["consumed"] for row in final["original_production"]["house_consumption"]}
    for owner, amounts in received.items():
        for good, field in enumerate(GOODS):
            proof.require(amounts[good] == high.owned(final, owner)["house"][field]-high.owned(seed, owner)["house"][field]+
                          after_used[owner][good]-before_used[owner][good],
                          "Actual arrival stream does not reconcile another House's stock/consumption")
    consumption = {owner: [0]*6 for owner in received}
    seen = set()
    for event in trace["consumption_events"]:
        tick, owner = integer(event["tick"], begin+1), integer(event["house"], 1)
        proof.require(tick <= end and owner in consumption and (tick, owner) not in seen and
                      (tick-40) % 51 == 0 and (tick-40)//51 % 16 in (0, 7),
                      "Consumption event was duplicated or escaped its actual scheduler boundary")
        seen.add((tick, owner))
        proof.require(len(event["actual_consumed"]) == 6, "Missing actual goods sink dimension")
        for good, amount in enumerate(event["actual_consumed"]): consumption[owner][good] += integer(amount)
    for owner, amounts in consumption.items():
        proof.require(amounts == [b-a for a, b in zip(before_used[owner], after_used[owner])],
                      "Observed physical consumption differs from complete per-House sinks")
    boundaries = [tick for tick in range(begin+1, end+1) if (tick-40) % 51 == 0 and (tick-40)//51 % 16 in (0, 7)]
    proof.require(seen == {(tick, owner) for tick in boundaries for owner in consumption},
                  "Trace omitted an actual per-House scheduled consumption event")
    return {"evaluation_samples": len(samples), "detailed_ticks": len(expected_window),
            "transfer_events": len(trace["transfer_events"]), "house_arrived_units": received}


def workflow(binary, data, output, variant="common-reserved", reference=None, timeout=1800,
             replay_checkpoints=True):
    for path in (binary, data, output, PRIVATE): proof.safe(path)
    proof.require(binary.is_file() and os.access(binary, os.X_OK) and data.is_dir(),
                  "Executable or supplied original installation is missing")
    proof.require(variant in VARIANTS and type(timeout) is int and 1 <= timeout <= 7200 and
                  PRIVATE in output.parents and not output.exists() and
                  not any(proof.overlaps(output, p) for p in (data, binary.parent)),
                  "Fresh isolated OE39 output, reviewed variant and bounded timeout required")
    proof.require(proof.digest(data / "Model/EmperorBuildingModels.txt") == proof.MODEL_SHA,
                  "Pinned unchanged original Normal model required")
    if reference is not None:
        proof.safe(reference)
        proof.require(reference.is_dir() and proof.REPO / ".local/original-economy" in reference.parents and
                      not proof.overlaps(reference, output), "Explicit immutable historical creator required")
    original, sources, executable = proof.inventory(data), proof.sources(), proof.record(binary)
    historical = proof.inventory(reference) if reference is not None else None
    output.mkdir(parents=True)
    result = {"format": "openemperor-market-distribution-workflow-v1", "stage": "A", "status": "FAIL",
              "technical_status": "FAIL", "retention": "NOT_RUN", "original_parity": "PARTIAL",
              "variant": variant, "native_human": "NOT_RUN", "rendering": "NOT_RUN",
              "sources": sources, "executable": executable, "processes": []}
    started = time.monotonic()

    def child(mode, folder, extra):
        command = [str(binary), "--mode", mode, "--data", str(data), "--output", str(folder), *extra]
        entry = {"command": command}; result["processes"].append(entry)
        now = time.monotonic()
        try:
            with (output / (folder.name+".log")).open("w") as log:
                ran = subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, timeout=timeout, check=False)
            entry["returncode"] = ran.returncode
            proof.require(ran.returncode == 0, "Actual transport process failed; inspect " + folder.name + ".log")
            return proof.load(folder / "report.json")
        finally: entry["elapsed_seconds"] = time.monotonic()-now

    try:
        creator = output / "creator"
        report = child("diagnose", creator, ["--variant", variant])
        model, buildings = baseline.prepared(data / "Model/EmperorBuildingModels.txt")
        labels = report["checkpoints"]
        proof.require(isinstance(labels, list) and all(type(label) is str and label in CHECKPOINTS for label in labels),
                      "Unreviewed checkpoint path in diagnosis report")
        worlds = {}
        for label in labels:
            worlds[label] = baseline.review_save(creator / (label+".save.json"), 12, data)
            high.review_snapshot(creator / (label+".snapshot.json"), worlds[label])
        review_checkpoint_list(labels, worlds, variant)
        target = integer(report["target_house"], 1)
        expansion, first, final = (worlds[key] for key in ("expansion", "first-success", "result"))
        retention = proof.load(creator / "retention.json")
        high.review_common(report, proof.load(creator / "warmup.json"), proof.load(creator / "search.json"),
                           expansion, final, model, buildings, first, retention, review_paid_commands, VARIANTS)
        supply = review_supply(report, retention)
        result["transport_checks"] = {
            "search": review_trace(proof.load(creator / "trace-search.json"), proof.load(creator / "search.json"), expansion, first, target, model),
            "retention": review_trace(proof.load(creator / "trace-retention.json"), retention, first, final, target, model)}
        failed = [event["actual_evaluation"] for event in retention["evaluations"]
                  if not event["actual_evaluation"]["current_needs"]["needs_met"]]
        proof.require(report["first_failure"] == ({"tick": failed[0]["tick"], "actual_evaluation": failed[0]} if failed else None),
                      "First-failure claim differs from the earliest actual unmet phase39 input")
        result["creator_recovery"] = high.review_recovery(creator, creator / "first-success.save.json", creator / "result.save.json")
        if reference is not None:
            result["historical_literal_pairs"] = compare_files(reference, creator,
                ("expansion.save.json", "first-success.save.json", "result.save.json",
                 "first-success.snapshot.json", "result.snapshot.json", "warmup.json", "search.json", "retention.json"))
        result["replays"] = []
        if replay_checkpoints:
            for label in labels:
                if label in ("expansion", "result"): continue
                folder = output / ("resume-"+label); seed = worlds[label]
                input_path = creator / (label+".save.json"); identity = proof.record(input_path)
                resumed_report = child("resume", folder, ["--input", str(input_path), "--target", str(target), "--end-tick", str(final["ticks"])])
                proof.require(proof.record(input_path) == identity, "Resume changed its exact earned input")
                resumed_seed = baseline.review_save(folder / "resume-start.save.json", 12, data)
                proof.require((folder / "resume-start.save.json").read_bytes() == input_path.read_bytes(),
                              "Canonical complete restore differs from its earned input")
                high.review_snapshot(folder / "resume-start.snapshot.json", resumed_seed)
                replayed = baseline.review_save(folder / "result.save.json", 12, data)
                high.review_snapshot(folder / "result.snapshot.json", replayed)
                review_resume(resumed_report, seed, replayed, target)
                observed = proof.load(folder / "retention.json")
                high.review_observation(observed, seed, replayed, target, model)
                review_trace(proof.load(folder / "trace-retention.json"), observed, seed, replayed, target, model)
                if label == "first-success": review_supply(resumed_report, observed)
                names = ["result.save.json", "result.snapshot.json"]
                if label == "first-success": names += ["retention.json", "trace-retention.json"]
                result["replays"].append({"checkpoint": label, "literal_pairs": compare_files(creator, folder, names),
                    "recovery": high.review_recovery(folder, folder / "resume-start.save.json", folder / "result.save.json")})
        result.update(status="PASS", technical_status="PASS", scenario=report, retention=report["retention"], sustained_supply=supply,
                      separate_process_continuation="PASS" if replay_checkpoints else "NOT_RUN")
    except Exception as error:
        result["error"] = str(error)
        raise
    finally:
        result.update(elapsed_seconds=time.monotonic()-started,
                      original_bytes_modes_mtimes_unchanged=original == proof.inventory(data),
                      sources_unchanged=sources == proof.sources(), executable_unchanged=executable == proof.record(binary),
                      historical_creator_unchanged=reference is None or historical == proof.inventory(reference))
        if not all(result[key] for key in ("original_bytes_modes_mtimes_unchanged", "sources_unchanged", "executable_unchanged", "historical_creator_unchanged")):
            result.update(status="FAIL", technical_status="FAIL", error="Protected original/source/executable/historical input changed")
        (output / "market-acceptance.json").write_text(json.dumps(result, indent=2)+"\n")
    proof.require(result["technical_status"] == "PASS", result.get("error", "Transport diagnosis technical gate failed"))
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("binary", "data", "output"): parser.add_argument("--"+name, type=Path, required=True)
    parser.add_argument("--variant", choices=VARIANTS, default="common-reserved")
    parser.add_argument("--reference", type=Path, help="Explicit immutable OE38 common creator directory")
    parser.add_argument("--timeout", type=int, default=1800)
    parser.add_argument("--no-replay", action="store_true", help="Diagnostic only; separate-process proof NOT_RUN")
    args = parser.parse_args()
    try:
        result = workflow(*(getattr(args, key).absolute() for key in ("binary", "data", "output")),
                          args.variant, args.reference.absolute() if args.reference else None, args.timeout, not args.no_replay)
        print("OE39 Gate A technical " + result["technical_status"] + "; retention " + result["retention"] + "; native/human NOT_RUN")
        return 0
    except Exception as error:
        print("OE39 FAIL: " + str(error), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
