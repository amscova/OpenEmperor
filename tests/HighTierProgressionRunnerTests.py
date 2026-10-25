"""Hostile diagnostic metadata controls, never gameplay success fixtures."""
import copy
from pathlib import Path
import sys
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


if __name__ == "__main__":
    unittest.main()
