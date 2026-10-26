"""Adversarial evidence controls only; no fixture is a paid gameplay witness."""
import copy
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import test_market_distribution as runner


def transport_control():
    """One physical arrival from already-loaded cargo; no evaluation in100..110."""
    model = [[0]*24 for _ in range(15)]
    state = dict.fromkeys(("water", "herbalist", "acupuncture", "music", "acrobat", "drama", "ancestor",
                          "confucian", "daoist", "buddhist", "food_quality", *runner.GOODS), 0)
    state.update(type=3, occupants=2)
    needs = {"model_row": 0, "desirability_low": 0, "desirability_high": 0, "capacity": 0, "needs_met": True,
             "requirements": [{"field": field, "required": 0, "actual": 0, "presence": n < 9, "met": True}
                              for n, field in enumerate(runner.high.FIELDS)]}

    def endpoint(tick, arrived):
        house = dict(state, hemp=int(arrived))
        totals = [{"good": good, "produced": int(good == 0), "consumed": 0, "imported": 0, "demolished": 0} for good in range(6)]
        trip = {"sequence": 1, "source": 5, "target": 3, "good": 0, "phase": 3 if arrived else 1,
                "loaded": 1, "delivered": int(arrived), "returned": 0}
        world = {"ticks": tick, "treasury": 10000, "taxes_collected_total": 100, "maintenance_spent_total": 20,
                 "buildings": [{"id": ident, "kind": kind, "cell": cell} for ident, kind, cell in ((3, 6, [0, 0]), (5, 9, [3, 0]))],
                 "couriers": [{"id": 1, "owner": 5, "role": 8}],
                 "original_economy": {"houses": [{"building_id": 3, "house": house}]},
                 "original_goods": {"stocks": [{"building_id": 5, "amounts": [0]*6}], "totals": totals},
                 "original_production": {"house_consumption": [{"building_id": 3, "consumed": [0]*6}]}}
        owners = []
        for ident, kind, cell in ((3, 6, [0, 0]), (5, 9, [3, 0])):
            goods = []
            for good in range(6):
                stock = int(ident == 3 and good == 0 and arrived)
                incoming = int(ident == 3 and good == 0 and not arrived)
                returning = int(ident == 5 and good == 0 and not arrived)
                goods.append({"good": good, "stock": stock, "capacity": 8, "source_reserved": 0,
                              "incoming_reserved": incoming, "outgoing_cargo_return_room": returning,
                              "incoming_cargo": incoming, "native_pottery_incoming": 0,
                              "headroom": 8-stock-incoming-returning})
            owners.append({"id": ident, "object": kind, "cell": cell, "operating": True, "burning": False,
                           "workers_required": 4 if ident == 5 else 0, "workers_assigned": 4 if ident == 5 else 0, "goods": goods})
        actual_needs = copy.deepcopy(needs)
        actual_needs["requirements"][10]["actual"] = int(arrived)
        actor = {"id": 1, "owner": 5, "role": 8, "target": 3, "route_pending": False,
                 "path": [[3, 0], [2, 0]], "path_vertex": 0, "edge_progress": 0,
                 "typed_cargo": int(not arrived), "latest_goods_trip": trip}
        sample = {"tick": tick, "funds": 10000, "taxes": 100, "upkeep": 20, "owners": owners, "couriers": [actor],
                  "houses": [{"house": 3, "state": house, "post_tick_current_needs": actual_needs,
                              "post_tick_next_needs": copy.deepcopy(actual_needs), "actual_phase39_evaluation": None}],
                  "goods_balances": [dict(row, stock=int(arrived and good == 0), cargo=int(not arrived and good == 0),
                                          source_reserved=0, incoming_reserved=int(not arrived and good == 0), conserved=True)
                                     for good, row in enumerate(totals)]}
        return world, sample

    seed, initial = endpoint(100, False); final, last = endpoint(110, True)
    event = {"family": "goods", "event": "delivery", "tick": 105, "courier": 1, "sequence": 1,
             "source": 5, "target": 3, "good": 0, "phase": 3, "loaded": 1, "delivered": 1, "returned": 0,
             "new_delivered_units": 1, "new_returned_units": 0, "route_edges": 1, "minimum_one_way_ticks": 5,
             "minimum_round_trip_ticks": 10, "departure_tick": None, "arrival_tick": 105,
             "actor": copy.deepcopy(last["couriers"][0]), "source_post_tick": copy.deepcopy(last["owners"][1]),
             "target_post_tick": copy.deepcopy(last["owners"][0])}
    trace = {"format": "openemperor-market-distribution-trace-v1", "rule": 12, "target": 3,
             "model_sha256": runner.proof.MODEL_SHA, "world_mutation_by_observer": False, "begin_tick": 100, "end_tick": 110,
             "initial": initial, "final": last, "transfer_events": [event], "evaluation_samples": [],
             "detailed_window": [], "consumption_events": []}
    return trace, {"evaluations": []}, seed, final, 3, model


class ActualTransportEvidenceGuards(unittest.TestCase):
    def rejects(self, mutate):
        args = transport_control(); mutate(args[0])
        with self.assertRaises(Exception): runner.review_trace(*args)

    def test_loaded_goods_count_only_at_actual_arrival(self):
        checked = runner.review_trace(*transport_control())
        self.assertEqual(checked["house_arrived_units"], {3: [1, 0, 0, 0, 0, 0]})

    def test_dispatch_is_not_delivery(self):
        self.rejects(lambda t: t["transfer_events"][0].update(event="dispatch_load"))

    def test_missing_or_duplicated_arrival_fails_raw_house_balance(self):
        self.rejects(lambda t: t["transfer_events"].clear())
        self.rejects(lambda t: t["transfer_events"].append(copy.deepcopy(t["transfer_events"][0])))

    def test_wrong_owner_and_house_cannot_get_receipt(self):
        self.rejects(lambda t: t["transfer_events"][0].update(source=3))
        self.rejects(lambda t: t["transfer_events"][0].update(target=5))

    def test_transfer_cannot_exceed_loaded_mass(self):
        self.rejects(lambda t: t["transfer_events"][0].update(delivered=2, new_delivered_units=2))

    def test_known_loading_cannot_arrive_before_physical_path(self):
        self.rejects(lambda t: t["transfer_events"][0].update(arrival_tick=104))
        self.rejects(lambda t: t["transfer_events"][0].update(departure_tick=105, route_edges=2,
                                                            minimum_one_way_ticks=10, minimum_round_trip_ticks=20))

    def test_unknown_inflight_departure_cannot_be_forged(self):
        self.rejects(lambda t: t["transfer_events"][0].update(departure_tick=99))

    def test_return_trip_is_part_of_throughput_bound(self):
        self.rejects(lambda t: t["transfer_events"][0].update(minimum_round_trip_ticks=5))

    def test_sequence_and_integer_types_are_authority(self):
        self.rejects(lambda t: t["transfer_events"][0].update(sequence=True))
        self.rejects(lambda t: t["transfer_events"][0].update(new_delivered_units=1.0))

    def test_warehouse_overflow_cannot_hide_reservations(self):
        self.rejects(lambda t: t["initial"]["owners"][0]["goods"][0].update(headroom=8))
        self.rejects(lambda t: t["initial"]["owners"][1]["goods"][0].update(outgoing_cargo_return_room=0))

    def test_aggregate_stock_cannot_replace_physical_owner_stock(self):
        self.rejects(lambda t: t["final"]["owners"][0]["goods"][0].update(stock=2, headroom=6))

    def test_fake_cargo_or_source_sink_totals_fail_conservation(self):
        self.rejects(lambda t: t["initial"]["couriers"][0].update(typed_cargo=0))
        self.rejects(lambda t: t["final"]["goods_balances"][0].update(produced=2, stock=2))

    def test_another_house_cannot_be_omitted_from_fairness(self):
        self.rejects(lambda t: t["final"]["houses"].clear())

    def test_invented_phase39_event_and_missing_required_input_fail(self):
        self.rejects(lambda t: t["evaluation_samples"].append({"tick": 105}))

    def test_wrong_generation_source_or_mutation_fail(self):
        self.rejects(lambda t: t.update(rule=13))
        self.rejects(lambda t: t.update(model_sha256="0"*64))
        self.rejects(lambda t: t.update(world_mutation_by_observer=True))


class CompleteReplayEvidenceGuards(unittest.TestCase):
    def test_equal_json_with_different_bytes_is_refused(self):
        with tempfile.TemporaryDirectory() as tmp:
            left, right = Path(tmp).resolve()/"creator", Path(tmp).resolve()/"resume"
            left.mkdir(); right.mkdir()
            (left/"result.save.json").write_text('{"world":{}}\n')
            (right/"result.save.json").write_text('{ "world": {} }\n')
            with self.assertRaises(Exception): runner.compare_files(left, right, ["result.save.json"])
            (right/"result.save.json").write_bytes((left/"result.save.json").read_bytes())
            self.assertEqual(len(runner.compare_files(left, right, ["result.save.json"])), 1)

    def test_unsafe_checkpoint_path_and_incomplete_manifest_fail(self):
        for labels in (["../result"], ["first-success", "result"], ["first-success", "first-success"]):
            with self.assertRaises(Exception): runner.review_checkpoint_list(labels, {}, "common-reserved")

    def test_extension_hook_does_not_widen_historical_variant_default(self):
        # Default rejection precedes any supplied checker. Existing OE38 callers
        # still admit only their exact three historical recipes.
        report = {"format": "openemperor-paid-high-tier-common-v1", "technical_status": "PASS", "snapshot_injection": False,
                  "rule": 12, "schema": 28, "policy": 3, "revision": 27, "variant": "common-short-route",
                  "common10": "PASS", "elite17": "PARTIAL", "original_parity": "PARTIAL"}
        with self.assertRaisesRegex(RuntimeError, "Invalid Common scenario"):
            runner.high.review_common(report, {}, {}, {}, {}, [], {})


def paid_control(variant="common-reserved"):
    report = {"variant": variant, "initial_commands": [], "expansion_commands": []}
    world = {"buildings": [], "roads": [], "construction_spent_total": 0, "command_sequence": 0, "treasury": 12000}
    def command(label, typ, kind, cell, cost, tick, ident=None):
        funds, seq = world["treasury"], world["command_sequence"]
        row = {"command": label, "type": typ, "cell": cell, "relative_cell": cell, "tick": tick,
               "cost": cost, "quoted_cost": cost, "funds_before": funds, "funds_after": funds-cost,
               "sequence_before": seq, "sequence_after": seq+1, "accepted": True, "changed": True}
        world.update(treasury=funds-cost, command_sequence=seq+1,
                     construction_spent_total=world["construction_spent_total"]+cost)
        if kind is not None:
            row["building_id"] = ident
            world["buildings"].append({"id": ident, "kind": kind, "cell": cell, "placed_tick": tick, "workforce_priority": 1})
        elif typ == 0: world["roads"].append(cell)
        report["initial_commands" if tick == 0 else "expansion_commands"].append(row)
        return row
    command("Household", 6, 6, [6, 2], 80, 0, 1)
    command("Warehouse", 2, 3, [18, 5], 150, 0, 2)
    command("Market", 9, 9, [9, 5], 140, 0, 3)
    warmup = {"final": {"funds": 12020, "construction": world["construction_spent_total"]}}
    world["treasury"] = 12020  # Authored metadata only, never injected into World.
    near = variant == "common-near-chain"
    command("Clay", 3, 4, [18, 0] if near else [0, 0], 120, 20000, 4)
    command("Pottery", 4, 5, [18, 2] if near else [22, 2], 180, 20000, 5)
    if variant == "common-priority":
        for ident in (2, 3):
            body = world["buildings"][ident-1]
            row = command("Logistics priority", 11, None, body["cell"], 0, 20000)
            row.update(building_id=ident, selected_building_id=ident, workforce_priority=0)
            body["workforce_priority"] = 0
    if near or variant == "common-short-route":
        for y in range(4): command("Near chain road" if near else "Market shortcut road", 0, None,
                                   [17, y] if near else [8, y+5], 2, 20000)
    return report, world, warmup, {}


class PaidVariantEvidenceGuards(unittest.TestCase):
    def test_ordinary_priority_and_route_commands_remain_actual_paid_receipts(self):
        for variant in ("common-reserved", "common-priority", "common-near-chain", "common-short-route"):
            runner.review_paid_commands(*paid_control(variant))

    def rejects(self, variant, mutate):
        args = paid_control(variant); mutate(args[0], args[1])
        with self.assertRaises(Exception): runner.review_paid_commands(*args)

    def test_unpaid_shortcut_and_wrong_physical_cell_are_refused(self):
        self.rejects("common-short-route", lambda r, w: r["expansion_commands"][-1].update(cost=0))
        self.rejects("common-short-route", lambda r, w: r["expansion_commands"][-1].update(relative_cell=[8, 9]))

    def test_priority_cannot_add_funds_skip_sequence_or_choose_another_body(self):
        self.rejects("common-priority", lambda r, w: r["expansion_commands"][-1].update(funds_after=12000))
        self.rejects("common-priority", lambda r, w: r["expansion_commands"][-1].update(sequence_after=99))
        self.rejects("common-priority", lambda r, w: r["expansion_commands"][-1].update(selected_building_id=4))

    def test_commodity_label_cannot_spoof_body_kind_or_command(self):
        self.rejects("common-reserved", lambda r, w: r["expansion_commands"][0].update(type=4))
        self.rejects("common-reserved", lambda r, w: w["buildings"][3].update(kind=5))

    def test_new_strategy_receipts_do_not_widen_legacy_checker(self):
        args = paid_control("common-short-route")
        with self.assertRaises(Exception): runner.high.review_paid_commands(*args)


def supply_control():
    needs = {"needs_met": True, "requirements": [{"field": "Hemp", "met": True}, {"field": "Ceramics", "met": True}]}
    retention = {"begin_tick": 27733, "end_tick": 48133,
                 "evaluations": [{"actual_evaluation": {"current_needs": copy.deepcopy(needs)}} for _ in range(400)],
                 "summary": {"burning_ticks": 0, "devolutions": 0}}
    report = {"retention_ticks": 20400, "target_lost_ticks": 0, "type_lost_ticks": 0, "sustained_supply": "PASS",
              "supply_evaluations": {"actual": 400, "current_unmet": 0, "hemp_unmet": 0, "ceramics_unmet": 0}}
    return report, retention


class ContinuousSupplyEvidenceGuards(unittest.TestCase):
    def test_exact_full_window_without_misses_is_required(self):
        self.assertEqual(runner.review_supply(*supply_control()), "PASS")

    def test_temporary_shortage_without_type_loss_is_not_pass(self):
        report, metrics = supply_control()
        for event in metrics["evaluations"][:9]:
            event["actual_evaluation"]["current_needs"]["needs_met"] = False
            event["actual_evaluation"]["current_needs"]["requirements"][0]["met"] = False
        report["supply_evaluations"].update(current_unmet=9, hemp_unmet=9)
        with self.assertRaisesRegex(RuntimeError, "Continuous-supply verdict"):
            runner.review_supply(report, metrics)
        report["sustained_supply"] = "PARTIAL"
        self.assertEqual(runner.review_supply(report, metrics), "PARTIAL")

    def test_ceramics_service_and_fire_are_each_independent_failures(self):
        for mutate in (lambda r, m: m["summary"].update(burning_ticks=1),
                       lambda r, m: r.update(type_lost_ticks=1),
                       lambda r, m: r.update(target_lost_ticks=1),
                       lambda r, m: m["summary"].update(devolutions=1)):
            report, metrics = supply_control(); mutate(report, metrics)
            with self.assertRaises(Exception): runner.review_supply(report, metrics)

    def test_forged_counts_or_short_window_fail(self):
        report, metrics = supply_control(); report["supply_evaluations"]["hemp_unmet"] = True
        with self.assertRaises(Exception): runner.review_supply(report, metrics)
        report, metrics = supply_control(); metrics["end_tick"] -= 1
        with self.assertRaises(Exception): runner.review_supply(report, metrics)


if __name__ == "__main__":
    unittest.main()
