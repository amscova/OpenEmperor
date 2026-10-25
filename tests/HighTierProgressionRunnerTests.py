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


if __name__ == "__main__":
    unittest.main()
