"""Hostile diagnostic metadata controls, never gameplay success fixtures."""
import copy
from pathlib import Path
import sys
import struct
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import test_high_tier_progression as runner


def diagnostic():
    return {"format": "openemperor-high-tier-matrix-v1", "rule": 12, "policy": 3,
            "model_sha256": runner.proof.MODEL_SHA, "difficulty": 2,
            "independent_house_rules_match": True, "query_world_unchanged": True,
            "snapshot_injection": False, "high_tier_success_claim": False,
            "types16": [{"type": value} for value in range(2, 18)],
            "houses": [{"elite_family": family, "matrix15": [{} for _ in range(15)]}
                       for family in (False, True)]}


class DiagnosticEvidenceGuards(unittest.TestCase):
    def refuses(self, mutate):
        value = copy.deepcopy(diagnostic())
        mutate(value)
        with self.assertRaises(Exception):
            runner.review_matrix(value)

    def test_unclaimed_diagnostic_metadata(self):
        runner.review_matrix(diagnostic())

    def test_table_is_not_a_gameplay_success(self):
        self.refuses(lambda d: d.update(high_tier_success_claim=True))

    def test_no_injected_authority(self):
        self.refuses(lambda d: d.update(snapshot_injection=True))

    def test_world_query_must_be_read_only(self):
        self.refuses(lambda d: d.update(query_world_unchanged=False))

    def test_independent_rules_are_mandatory(self):
        self.refuses(lambda d: d.update(independent_house_rules_match=False))

    def test_model_and_difficulty_bound(self):
        self.refuses(lambda d: d.update(model_sha256="0" * 64))
        self.refuses(lambda d: d.update(difficulty=3))

    def test_generation_bound(self):
        self.refuses(lambda d: d.update(rule=11))
        self.refuses(lambda d: d.update(policy=2))
        self.refuses(lambda d: d.update(rule=12.0))

    def test_complete_distinct_type_domain(self):
        self.refuses(lambda d: d["types16"].pop())
        self.refuses(lambda d: d["types16"].reverse())
        self.refuses(lambda d: d["types16"][0].update(type=3))

    def test_both_families_and_every_model_row_required(self):
        self.refuses(lambda d: d["houses"][1].update(elite_family=False))
        self.refuses(lambda d: d["houses"][0]["matrix15"].pop())


def bound_observers():
    matrix = diagnostic()
    matrix.update(tick=120, funds=11840, construction=160)
    vacant, common, elite = {"type": 2, "occupants": 0}, {"type": 3, "occupants": 2}, {"type": 11, "occupants": 0}
    entry = {"admitted": 2, "kind": 0, "evaluation": {"tick": 120, "input": vacant}}
    histories = [{"building_id": n, "first_admission_tick": 120 if n == 1 else None,
                  "first_transitions": [entry if n == 1 and r == 0 else None for r in range(15)]} for n in (1, 2)]
    world = {"ticks": 120, "treasury": 11840, "construction_spent_total": 160,
             "buildings": [{"id": 1, "cell": [0, 0]}, {"id": 2, "cell": [6, 6]}],
             "original_economy": {"houses": [{"building_id": 1, "house": common}, {"building_id": 2, "house": elite}]},
             "original_production": {"housing": {"houses": histories}}}
    for n, observed in enumerate(matrix["houses"], 1):
        observed.update(id=n, cell=[0, 0] if n == 1 else [6, 6], actual_footprint_side=2,
                        actual_post_tick_state=common.copy() if n == 1 else elite.copy(),
                        actual_first_transitions=[{"type": 3, "tick": 120, "kind": 0, "admitted": 2,
                                                   "evaluation_input": vacant.copy()}] if n == 1 else [])
    for row in matrix["types16"]:
        row.update(actual_witnesses=[], actual_occupied_tier_witnessed=False)
        if row["type"] == 3:
            row.update(actual_witnesses=[{"house": 1, "tick": 120, "kind": 0}], actual_occupied_tier_witnessed=True)
        if row["type"] == 11:
            row.update(actual_witnesses=[{"house": 2, "kind": "paid empty plot, no occupied-tier proof"}])
    return matrix, world


class RawWorldBindingGuards(unittest.TestCase):
    def refuses(self, mutate):
        matrix, world = bound_observers()
        mutate(matrix)
        with self.assertRaises(Exception):
            runner.bind_matrix_world(matrix, world)

    def test_real_empty_and_admitted_states_are_distinct(self):
        matrix, world = bound_observers()
        runner.bind_matrix_world(matrix, world)

    def test_fake_high_tier_witness_refused(self):
        self.refuses(lambda d: d["types16"][8].update(actual_occupied_tier_witnessed=True))
        self.refuses(lambda d: d["types16"][15].update(actual_witnesses=[{"house": 2, "tick": 120, "kind": 1}]))

    def test_changed_live_type_or_stock_refused(self):
        self.refuses(lambda d: d["houses"][0]["actual_post_tick_state"].update(type=10))
        self.refuses(lambda d: d["houses"][0]["actual_post_tick_state"].update(tea=8))

    def test_family_footprint_and_observer_identity_bound(self):
        self.refuses(lambda d: d["houses"][1].update(actual_footprint_side=4))
        self.refuses(lambda d: d["houses"][1].update(elite_family=False))
        self.refuses(lambda d: d["houses"][1].update(id=3))

    def test_transition_provenance_bound(self):
        self.refuses(lambda d: d["houses"][0]["actual_first_transitions"][0].update(tick=119))
        self.refuses(lambda d: d["houses"][0]["actual_first_transitions"].clear())

    def test_finance_and_clock_bound(self):
        self.refuses(lambda d: d.update(funds=12000))
        self.refuses(lambda d: d.update(tick=121))


# Deliberately authored metadata controls only. They cannot pass the guarded
# gameplay workflow, which also requires untouched original bytes/full saves.
def receipt_control():
    def row(label, cell, price, funds, seq, tick, owner=None):
        value = {"command": label, "cell": cell, "cost": price, "quoted_cost": price,
                 "funds_before": funds, "funds_after": funds-price, "tick": tick, "type": 0,
                 "sequence_before": seq, "sequence_after": seq+1, "accepted": True, "changed": True}
        if owner is not None:value["building_id"] = owner
        return value
    report = {"initial_commands": [row("Road", [1, 1], 2, 12000, 0, 0),
                                    row("Household", [2, 2], 80, 11998, 1, 0, 1)],
              "expansion_commands": [row("WaterWell", [5, 5], 25, 12020, 2, 20000, 2)]}
    world = {"treasury": 11995, "construction_spent_total": 107, "command_sequence": 3, "roads": [[1, 1]],
             "buildings": [{"id": 1, "cell": [2, 2], "placed_tick": 0},
                           {"id": 2, "cell": [5, 5], "placed_tick": 20000}]}
    return report, world, {"final": {"funds": 12020, "construction": 82}}, {72: [25]}


class PaidCommandGuards(unittest.TestCase):
    def test_receipts_bind_complete_raw_paid_endpoint(self):
        runner.review_paid_commands(*receipt_control())

    def rejects(self, mutate):
        report, world, warmup, model = receipt_control();mutate(report, world)
        with self.assertRaises(Exception):runner.review_paid_commands(report, world, warmup, model)

    def test_free_price_and_extra_funds_refused(self):
        self.rejects(lambda r, w: r["expansion_commands"][0].update(cost=0, quoted_cost=0))
        self.rejects(lambda r, w: r["expansion_commands"][0].update(funds_before=12021, funds_after=11996))

    def test_bool_and_float_receipts_refused(self):
        self.rejects(lambda r, w: r["initial_commands"][0].update(cost=True))
        self.rejects(lambda r, w: r["initial_commands"][0].update(sequence_after=1.0))

    def test_unreceipted_body_road_and_construction_refused(self):
        self.rejects(lambda r, w: w["buildings"].append({"id": 3, "cell": [8, 8], "placed_tick": 0}))
        self.rejects(lambda r, w: w["roads"].append([9, 9]))
        self.rejects(lambda r, w: w.update(construction_spent_total=108))

    def test_wrong_purchase_owner_time_or_command_refused(self):
        self.rejects(lambda r, w: r["expansion_commands"][0].update(building_id=1))
        self.rejects(lambda r, w: w["buildings"][1].update(placed_tick=19999))
        self.rejects(lambda r, w: r["expansion_commands"][0].update(command="Unreviewed grant"))


def requirement_control(bronze=0, lacquer=0):
    model = [[0] * 24 for _ in range(15)];model[1][17] = 37;model[1][9] = model[1][13] = 1
    state = dict.fromkeys(("herbalist", "acupuncture", "music", "acrobat", "drama", "ancestor", "confucian",
                           "daoist", "buddhist", "food_quality", "hemp", "ceramics", "tea", "silk"), 0)
    state.update(water=False, bronzeware=bronze, lacquerware=lacquer)
    needs = [{"field": field, "required": model[1][column], "actual": max(bronze, lacquer) if column == 13 else 0,
              "presence": n < 9, "met": model[1][column] == 0 or column == 13 and max(bronze, lacquer) >= 1}
             for n, (field, column) in enumerate(zip(runner.FIELDS, runner.COLUMNS))]
    return {"model_row": 1, "desirability_low": 0, "desirability_high": 0, "capacity": 37,
            "needs_met": False, "requirements": needs}, state, model


class ActualRequirementGuards(unittest.TestCase):
    def test_actual_or_sides_preserve_other_missing_good(self):
        for bronze, lacquer in ((0, 0), (1, 0), (0, 1), (1, 1)):
            runner.review_needs(*requirement_control(bronze, lacquer))

    def test_inventory_and_source_threshold_cannot_be_spoofed(self):
        needs, state, model = requirement_control(1, 0)
        for edit in (lambda d: d["requirements"][10].update(met=True),
                     lambda d: d["requirements"][14].update(actual=0),
                     lambda d: d["requirements"][10].update(required=0),
                     lambda d: d.update(needs_met=True), lambda d: d.update(capacity=38)):
            bad=copy.deepcopy(needs);edit(bad)
            with self.assertRaises(Exception):runner.review_needs(bad, state, model)


def first_control():
    entry = {"kind": 1, "admitted": 0, "evaluation": {"tick": 27733, "input": {"type": 9}}}
    first = {"ticks": 27733, "treasury": 11431, "construction_spent_total": 2588,
             "buildings": [{"id": 3, "fire_until_tick": 0}],
             "original_economy": {"houses": [{"building_id": 3, "house": {"type": 10, "occupants": 47}}]},
             "original_production": {"housing": {"houses": [{"building_id": 3,
                 "first_transitions": [entry if n == 7 else None for n in range(15)]}]}}}
    reached = {"house": 3, "type": 10, "tick": 27733, "funds": 11431, "construction": 2588,
               "exact_live_save": "first-success.save.json"}
    return reached, first


class FirstLiveWitnessGuards(unittest.TestCase):
    def test_only_exact_evolution_input_and_first_tick_qualify(self):
        runner.review_first_common(*first_control(), 3)

    def test_admission_synthetic_history_and_later_save_refused(self):
        for edit in (lambda e: e.update(kind=0), lambda e: e.update(admitted=2),
                     lambda e: e["evaluation"].update(tick=27732), lambda e: e["evaluation"]["input"].update(type=10)):
            reached, first = first_control();edit(first["original_production"]["housing"]["houses"][0]["first_transitions"][7])
            with self.assertRaises(Exception):runner.review_first_common(reached, first, 3)

    def test_uninhabited_burning_and_wrong_ledger_headline_refused(self):
        reached, first = first_control();first["original_economy"]["houses"][0]["house"]["occupants"] = 0
        with self.assertRaises(Exception):runner.review_first_common(reached, first, 3)
        reached, first = first_control();first["buildings"][0]["fire_until_tick"] = 27734
        with self.assertRaises(Exception):runner.review_first_common(reached, first, 3)
        reached, first = first_control();reached["funds"] += 1
        with self.assertRaises(Exception):runner.review_first_common(reached, first, 3)


class RetentionDurationGuards(unittest.TestCase):
    def control(self):
        return {"begin_tick": 100, "end_tick": 110, "initial_state": {"type": 10},
                "transition_events": [{"tick": 106, "from": 10, "to": 9, "actual_state": {"type": 9}}],
                "summary": {"type_tick_counts": [{"type": 9, "post_tick_count": 5}, {"type": 10, "post_tick_count": 5}],
                            "maximum_observed_type": 10}}

    def test_transient_success_is_not_full_retention(self):
        self.assertEqual(runner.review_type_durations(self.control()), 5)

    def test_hidden_devolution_or_changed_highest_tier_refused(self):
        for edit in (lambda m: m["summary"]["type_tick_counts"][0].update(post_tick_count=0),
                     lambda m: m["transition_events"].clear(), lambda m: m["summary"].update(maximum_observed_type=17)):
            value = self.control();edit(value)
            with self.assertRaises(Exception):runner.review_type_durations(value)


class RetentionEligibilityUnionGuards(unittest.TestCase):
    def control(self, overlap=False):
        metrics=RetentionDurationGuards().control()
        metrics["summary"].update(minimum_target_occupants=47,burning_ticks=2)
        metrics["target_fire_events"] = ([{"tick":108,"burning":True},{"tick":110,"burning":False}] if overlap
                                          else [{"tick":103,"burning":False}])
        seed={"ticks":100,"buildings":[{"id":3,"fire_until_tick":0 if overlap else 103}],
              "original_economy":{"houses":[{"building_id":3,"house":{"type":10,"occupants":47}}]}}
        final={"ticks":110,"buildings":[{"id":3,"fire_until_tick":110 if overlap else 103}],
               "original_economy":{"houses":[{"building_id":3,"house":{"type":9,"occupants":47}}]}}
        return metrics,seed,final

    def test_initial_burn_and_type_loss_count_as_union(self):
        self.assertEqual(runner.review_retention_losses(*self.control(),3),7)

    def test_overlapping_fire_does_not_double_count_type_loss(self):
        self.assertEqual(runner.review_retention_losses(*self.control(True),3),5)

    def test_hidden_fire_or_fake_zero_occupants_cannot_support_pass(self):
        for edit in (lambda m:m["target_fire_events"].clear(),lambda m:m["summary"].update(burning_ticks=0),
                     lambda m:m["summary"].update(minimum_target_occupants=0),
                     lambda m:m["target_fire_events"][0].update(burning=True)):
            metrics,seed,final=self.control();edit(metrics)
            with self.assertRaises(Exception):runner.review_retention_losses(metrics,seed,final,3)

    def test_full_type_retention_still_loses_burning_ticks(self):
        metrics,seed,final=self.control()
        metrics["transition_events"]=[]
        metrics["summary"]["type_tick_counts"]=[{"type":10,"post_tick_count":10}]
        final["original_economy"]["houses"][0]["house"]["type"]=10
        self.assertEqual(runner.review_retention_losses(metrics,seed,final,3),2)


class ActualFinanceMinimumGuards(unittest.TestCase):
    def control(self):
        return {"begin_tick":100,"end_tick":110,"summary":{"minimum_funds":-10,"maximum_funds":20},
                "city":{"summary":{"minimum_funds":-10},"finance":[
                    {"tick":103,"tax":0,"upkeep":20,"funds":-10},
                    {"tick":108,"tax":30,"upkeep":0,"funds":20}]}},{"treasury":10}

    def test_profitable_endpoint_does_not_hide_intermediate_negative_funds(self):
        self.assertEqual(runner.review_finance_extrema(*self.control()),-10)

    def test_coordinated_fake_minima_and_hidden_grant_are_refused(self):
        metrics,seed=self.control();metrics["summary"]["minimum_funds"]=0;metrics["city"]["summary"]["minimum_funds"]=0
        with self.assertRaises(Exception):runner.review_finance_extrema(metrics,seed)
        metrics,seed=self.control();metrics["city"]["finance"][0]["funds"]=-9
        with self.assertRaises(Exception):runner.review_finance_extrema(metrics,seed)

    def test_initial_funds_and_typed_event_chronology_are_included(self):
        metrics,seed=self.control();metrics["city"]["finance"].reverse()
        with self.assertRaises(Exception):runner.review_finance_extrema(metrics,seed)
        metrics,seed=self.control();metrics["city"]["finance"][0]["upkeep"]=20.0
        with self.assertRaises(Exception):runner.review_finance_extrema(metrics,seed)


def authored_certificate_control():
    # Zero-DES authored model metadata, not original fixture bytes or a paid
    # progression claim. Closed-form placement counts avoid reusing the checker.
    identities = [(8,8,208,2,2,0), (15,12,207,2,2,0), (20,13,72,2,4,1),
                  (26,19,214,2,2,5), (27,20,72,2,2,1), (28,21,215,2,2,5),
                  (29,22,217,2,2,5), (30,23,219,3,2,3), (31,24,211,2,2,1),
                  (32,25,212,3,2,1), (33,26,213,3,2,1)]
    cases = []
    for cy in range(4):
        for cx in range(4):
            lattice = [{"cell": [x, y], "charge": 3 if -2 <= x <= 2 and -2 <= y <= 2 else 1}
                       for y in (-4, -2, 0, 2, 4) for x in (-4, -2, 0, 2, 4)
                       if not (-cx <= x < 4-cx and -cy <= y < 4-cy)]
            cases.append({"house_cell": [cx, cy], "translated_house_origin": [-cx, -cy], "lattice": lattice,
                          "cell_charge": 31, "source_cap_charge": 48, "upper_bound": 79,
                          "positive_placements_checked": 0, "minimum_integer_slack": None})
    model = [[0] * 24 for _ in range(15)];model[13][1] = 80
    buildings = {value[2]: [0] * 13 for value in identities}
    bound = {"format": "openemperor-high-tier-des-bound-v1", "verified": True, "model_sha256": runner.proof.MODEL_SHA,
             "snapshot_injection": False, "actual_house_or_goal_witness": False, "bonus": 0,
             "non_overlapping_complete_footprints": True, "house_side": 4, "source_cap_charge": 48,
             "source_identities": [dict(zip(("command", "object", "model_id", "side", "cap", "source_charge"), row)) for row in identities],
             "all16cells": cases, "placements_checked": 16*(8*(22*22-5*5)+3*(23*23-6*6)),
             "positive_placements_checked": 0, "upper_bound": 79, "entry_threshold": 80, "blocks_entry": True}
    return bound, model, buildings


class EliteCertificateGuards(unittest.TestCase):
    def test_authored_zero_des_metadata_is_only_a_bound(self):
        self.assertEqual(runner.review_des_bound(*authored_certificate_control()), "BLOCKED")

    def test_unverified_success_and_unknown_source_refused(self):
        for edit in (lambda b: b.update(verified=False), lambda b: b.update(actual_house_or_goal_witness=True),
                     lambda b: b["source_identities"][0].update(model_id=999),
                     lambda b: b["source_identities"][0].update(cap=3)):
            bound, model, buildings = authored_certificate_control();edit(bound)
            with self.assertRaises(Exception):runner.review_des_bound(bound, model, buildings)

    def test_lattice_threshold_and_case_counts_are_recomputed(self):
        for edit in (lambda b: b["all16cells"].pop(), lambda b: b["all16cells"][0]["lattice"][0].update(charge=9),
                     lambda b: b.update(upper_bound=78), lambda b: b.update(entry_threshold=79),
                     lambda b: b.update(positive_placements_checked=1), lambda b: b.update(blocks_entry=False)):
            bound, model, buildings = authored_certificate_control();edit(bound)
            with self.assertRaises(Exception):runner.review_des_bound(bound, model, buildings)


def resume_control():
    seed = {"ticks": 27733, "command_sequence": 116, "construction_spent_total": 2588}
    final = dict(seed, ticks=48133)
    report = {"format": "openemperor-high-tier-resume-v1", "technical_status": "PASS", "target": 3,
              "begin_tick": 27733, "end_tick": 48133, "retention_ticks": 20400,
              "rule": 12, "schema": 28, "policy": 3, "revision": 27,
              "input_preserved": True, "complete_recovery_equality": True, "command_sequence_unchanged": True,
              "snapshot_injection": False, "native_human": "NOT_RUN"}
    return report, seed, final


class IndependentContinuationGuards(unittest.TestCase):
    def test_exact_generation_interval_and_immutable_commands(self):
        runner.review_resume(*resume_control(), 3)

    def test_changed_identity_tick_injection_and_recovery_refused(self):
        for key, value in (("rule", 11), ("schema", 27), ("policy", 2), ("revision", 28), ("target", 4),
                           ("end_tick", 48132), ("retention_ticks", 20400.0), ("input_preserved", False),
                           ("complete_recovery_equality", False), ("command_sequence_unchanged", False),
                           ("snapshot_injection", True), ("native_human", "PASS")):
            report, seed, final = resume_control();report[key] = value
            with self.assertRaises(Exception):runner.review_resume(report, seed, final, 3)

    def test_even_paid_continuation_commands_break_literal_replay_contract(self):
        report, seed, final = resume_control();final["command_sequence"] += 1
        with self.assertRaises(Exception):runner.review_resume(report, seed, final, 3)
        report, seed, final = resume_control();final["construction_spent_total"] += 2
        with self.assertRaises(Exception):runner.review_resume(report, seed, final, 3)

    def test_complete_artifact_bytes_not_only_json_values(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory).resolve();first = root/"creator";second = root/"resume"
            first.mkdir();second.mkdir()
            for name in runner.LITERAL_PAIRS:
                (first/name).write_text('{"all_world_fields":1}\n');(second/name).write_text('{"all_world_fields":1}\n')
            self.assertEqual(len(runner.compare_literals(first, second)), 5)
            (second/"retention.json").write_text('{ "all_world_fields": 1 }\n')
            with self.assertRaises(Exception):runner.compare_literals(first, second)

    def test_snapshot_is_complete_saved_world(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory).resolve()/"snapshot.json";path.write_text('{"ticks":10,"goods":[1,2]}')
            runner.review_snapshot(path, {"ticks": 10, "goods": [1, 2]})
            with self.assertRaises(Exception):runner.review_snapshot(path, {"ticks": 10, "goods": [1, 3]})


class RecoveryPublicationGuards(unittest.TestCase):
    def control(self, root):
        saved = [root/"seed.save.json", root/"result.save.json"]
        for n, path in enumerate(saved):
            path.write_text(runner.json.dumps({"world": {"ticks": 100+n*20400, "treasury": 200+n,
                             "original_production": {"residents_admitted": 50}}}))
        history = root/"recovery-app/recovery/history-authored-control";history.mkdir(parents=True)
        entries = []
        for n, path in enumerate(saved):
            name = "start.json" if n == 0 else "checkpoint-1-authored.json"
            (history/name).write_bytes(path.read_bytes())
            entries.append({"filename": name, "sequence": n, "kind": "start" if n == 0 else "autosave",
                            "profile": "sandbox-city-v16", "rule_version": 12, "schema": 28, "map": "Cities/Xia.map",
                            "tick": 100+n*20400, "treasury": 200+n, "population": 50})
        meta = {"version": 1, "history_id": history.name, "parent": None, "next_sequence": 2, "checkpoints": entries}
        (history/"history.json").write_text(runner.json.dumps(meta))
        return saved, history, meta

    def test_only_advertised_complete_start_and_result_qualify(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory).resolve();saved, history, meta = self.control(root)
            self.assertEqual(len(runner.review_recovery(root, *saved)), 2)
            (history/meta["checkpoints"][1]["filename"]).write_text('{}')
            with self.assertRaises(Exception):runner.review_recovery(root, *saved)

    def test_wrong_schema_advertised_tick_and_traversal_refused(self):
        for key, value in (("schema", 27), ("tick", 20499), ("treasury", 999), ("filename", "../escape.json")):
            with tempfile.TemporaryDirectory() as directory:
                root = Path(directory).resolve();saved, history, meta = self.control(root)
                meta["checkpoints"][1][key] = value;(history/"history.json").write_text(runner.json.dumps(meta))
                with self.assertRaises(Exception):runner.review_recovery(root, *saved)


class FocusedRenderGuards(unittest.TestCase):
    def control(self, root):
        world = {"ticks": 27733, "original_economy": {"houses": [{"building_id": 3, "house": {"type": 10}}]}}
        sections = {key: ["Authored metadata test; no gameplay/capture claim"] for key in ("State", "Needs", "Goods", "Visits")}
        report = {"format": "openemperor-high-tier-view-v1", "status": "PASS", "target_house": 3, "actual_type": 10,
                  "rule": 12, "schema": 28, "policy": 3, "revision": 27, "tick": 27733,
                  "complete_f5_f9_equality": True, "world_unchanged": True, "input_preserved": True,
                  "selected_via_sdl_list": True, "sections_via_sdl_input": True, "actual_state_only": True,
                  "actual_sandbox_renderer": True, "backend": "authored metadata control",
                  "native_human": "NOT_RUN", "inspection": sections["State"], "sections": sections,
                  "original_house_art": {"authored_metadata_control": True}, "captures": []}
        for name, zoom, section in (("sandbox-1x.png", 1, "State"), ("sandbox-2x.png", 2, "State"),
                                    ("sandbox-4x.png", 4, "State"), ("sandbox-needs-2x.png", 2, "Needs"),
                                    ("sandbox-goods-2x.png", 2, "Goods"), ("sandbox-visits-2x.png", 2, "Visits")):
            # PNG header metadata only, intentionally not an actual image. The
            # workflow separately requires real CLI/source and art provenance.
            (root/name).write_bytes(b'\x89PNG\r\n\x1a\n'+struct.pack('>I',13)+b'IHDR'+struct.pack('>II',1,1))
            report["captures"].append({"file": name, "zoom": zoom, "section": section, "tick": 27733,
                                       "target_house": 3, "actual_type": 10, "width": 1, "height": 1})
        return report, world

    def test_actual_checkpoint_identity_and_six_sections_required(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory).resolve();report, world = self.control(root)
            self.assertEqual(len(runner.review_render(report, root, world, 3, True)), 6)
            report["captures"].pop()
            with self.assertRaises(Exception):runner.review_render(report, root, world, 3, True)

    def test_future_type_fake_dimensions_and_unscripted_selection_refused(self):
        for edit in (lambda r: r.update(actual_type=17), lambda r: r.update(selected_via_sdl_list=False),
                     lambda r: r.update(actual_state_only=False), lambda r: r.update(complete_f5_f9_equality=False),
                     lambda r: r["captures"][0].update(width=2), lambda r: r["captures"][0].update(actual_type=9)):
            with tempfile.TemporaryDirectory() as directory:
                root = Path(directory).resolve();report, world = self.control(root);edit(report)
                with self.assertRaises(Exception):runner.review_render(report, root, world, 3, True)


class OriginalArtProvenanceGuards(unittest.TestCase):
    def control(self, root):
        # Authored metadata and non-image bytes only; no original test asset.
        data=root/"data";resources=root/"resources";data.mkdir();resources.mkdir()
        archive=data/"authored-source.bin";archive.write_bytes(b"authored metadata control")
        profile=resources/"compatibility/gog-derived-2.0.0.2-en-assetset-1/original-houses.json"
        profile.parent.mkdir(parents=True)
        stage={"house_type":10,"original_side":2,"archive":archive.name,"image_index":7,
               "width":20,"height":30,"ground_anchor":[10,25],"evidence":"Authored unit provenance"}
        profile.write_text(runner.json.dumps({"schema_version":1,"mode":"original_house_stages",
            "selection":"canonical_static_variant_0","anchor_policy":"type30_front_cell",
            "files":[{"path":archive.name,"sha256":runner.proof.digest(archive)}],"stages":[stage]}))
        world={"original_economy":{"houses":[{"building_id":3,"house":{"type":10}}]}}
        art={"configured":True,"active":True,"house_type":10,"actual_side":2,"source":"builtin",
             "archive":archive.name,"image_index":7,"image_width":20,"image_height":30,"anchor":[10,25],
             "selection":"canonical static representative","evidence":"Authored unit provenance",
             "original_draws":1,"authored_fallbacks":0,"ground":[100,200],"image_origin":[90,175]}
        return art,world,data,resources

    def test_actual_type_native_anchor_and_source_hash_bind(self):
        with tempfile.TemporaryDirectory() as directory:
            art,world,data,resources=self.control(Path(directory).resolve())
            runner.review_art(art,world,3,data,resources)
            (data/"authored-source.bin").write_bytes(b"changed original-source stand-in")
            with self.assertRaises(Exception):runner.review_art(art,world,3,data,resources)

    def test_future_stage_custom_fallback_and_shifted_anchor_refused(self):
        for edit in (lambda a:a.update(house_type=17),lambda a:a.update(source="custom"),
                     lambda a:a.update(authored_fallbacks=1),lambda a:a.update(original_draws=0),
                     lambda a:a.update(image_index=8),lambda a:a.update(anchor=[10,24]),
                     lambda a:a.update(image_origin=[90,176]),lambda a:a.update(ground=[float("nan"),200])):
            with tempfile.TemporaryDirectory() as directory:
                art,world,data,resources=self.control(Path(directory).resolve());edit(art)
                with self.assertRaises(Exception):runner.review_art(art,world,3,data,resources)


if __name__ == "__main__":
    unittest.main()
