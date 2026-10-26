"""Adversarial evidence checks, separate from real paid gameplay acceptance."""
import copy
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import test_sustainable_economy as runner


class OperatingEvidence(unittest.TestCase):
    def rows(self):
        return [{"begin_tick": 20000, "end_tick": 40400, "taxes": 8000, "upkeep": 7500,
                 "construction": 80, "funds_begin": 200, "funds_end": 620,
                 "operating_cashflow": 500},
                {"begin_tick": 40400, "end_tick": 60800, "taxes": 7600, "upkeep": 7600,
                 "construction": 0, "funds_begin": 620, "funds_end": 620,
                 "operating_cashflow": 0}]

    def test_complete_adjacent_windows_and_real_purchases(self):
        self.assertEqual(runner.operating_windows(self.rows(), 20000, 60800), 2)

    def test_lifetime_tax_does_not_hide_current_operating_loss(self):
        rows = self.rows()
        rows[1].update(taxes=7599, funds_end=619, operating_cashflow=-1)
        self.assertEqual(runner.operating_windows(rows, 20000, 60800), 1)

    def test_incomplete_trailing_interval_does_not_count(self):
        self.assertEqual(runner.operating_windows(self.rows(), 20000, 70000), 2)
        rows = self.rows()
        rows[1]["end_tick"] = 60000
        with self.assertRaises(RuntimeError):
            runner.operating_windows(rows, 20000, 60000)

    def test_rejects_omitted_recurring_costs_and_construction(self):
        for key, value in (("upkeep", 7400), ("construction", 0), ("operating_cashflow", 501),
                           ("taxes", 8001), ("funds_end", 621)):
            rows = self.rows()
            rows[0][key] = value
            with self.subTest(key=key), self.assertRaises(RuntimeError):
                runner.operating_windows(rows, 20000, 60800)

    def test_rejects_gaps_overlaps_duplicates_missing_windows(self):
        cases = ([], self.rows()[:1], self.rows()[::-1], self.rows() + [copy.deepcopy(self.rows()[1])])
        for rows in cases:
            with self.subTest(rows=rows), self.assertRaises(RuntimeError):
                runner.operating_windows(rows, 20000, 60800)
        for offset in (-1, 1):
            rows = self.rows()
            rows[1]["begin_tick"] += offset
            rows[1]["end_tick"] += offset
            with self.subTest(offset=offset), self.assertRaises(RuntimeError):
                runner.operating_windows(rows, 20000, 60801)

    def test_rejects_mistyped_values(self):
        for key, value in (("taxes", True), ("upkeep", "7500"), ("funds_begin", False),
                           ("operating_cashflow", 500.0), ("end_tick", 40400.0)):
            rows = self.rows()
            rows[0][key] = value
            with self.subTest(key=key), self.assertRaises(RuntimeError):
                runner.operating_windows(rows, 20000, 60800)

    def test_rejects_money_created_between_adjacent_windows(self):
        rows = self.rows()
        rows[1].update(funds_begin=1000, funds_end=1000)
        with self.assertRaisesRegex(RuntimeError, "between boundaries"):
            runner.operating_windows(rows, 20000, 60800)


class ProcessEvidence(unittest.TestCase):
    def witness(self):
        summary = {"taxes_delta": 15600, "upkeep_delta": 15100, "operating_cashflow": 500,
                   "windows20400": OperatingEvidence().rows(), "two_consecutive_nonnegative_windows": True}
        metrics = {"format": "openemperor-sustainable-economy-observation-v1", "begin_tick": 20000,
                   "end_tick": 60800, "upkeep_interval": 816, "summary": summary,
                   "final_goal": {"reached": True, "tick": 60800}}
        report = {"status": "PASS", "mode": "create", "rule": 12, "schema": 28, "policy": 3, "revision": 27,
                  "snapshot_injection": False, "variant": "compact-three", "creator_mode": True,
                  "restored_live_world": False, "input_preserved": True, "complete_recovery_equality": True,
                  "begin_tick": 20000, "end_tick": 60800,
                  "upkeep_interval": 816, "summary": copy.deepcopy(summary),
                  "first_goal": {"tick": 816, "reached": True},
                  "maturity": {"ready": True, "tick": 20000, "goal": {"reached": True, "tick": 20000},
                               "inhabited_houses": 12, "food_house_received": 12, "goods_consumed": 12,
                               "actual_service_visits": 12, "actual_water_visits": 12, "actual_inspector_arrivals": 12},
                  "initial_commands": [{"cost": 80, "funds_before": 12000, "funds_after": 11920}],
                  "sustainable_operating_windows": True, "minimum_100000_post_maturity_ticks": False}
        return report, metrics

    def validate(self, report, metrics):
        return runner.validate_process(report, metrics, 12, "compact-three", 40800, "create")

    def test_accepts_actual_descriptors_without_alternative_goal_formula(self):
        self.assertEqual(self.validate(*self.witness()), 2)

    def test_rejects_fake_first_goal(self):
        for goal in ({"reached": False, "tick": 816}, {"reached": 1, "tick": 816},
                     {"reached": True, "tick": "816"}, {"reached": True, "tick": 20001}):
            report, metrics = self.witness()
            report["first_goal"] = goal
            with self.subTest(goal=goal), self.assertRaises(RuntimeError):
                self.validate(report, metrics)

    def test_rejects_authority_injection_wrong_schema_or_restore(self):
        for key, value in (("snapshot_injection", True), ("schema", 27), ("rule", 11), ("policy", 2),
                           ("creator_mode", False), ("restored_live_world", True), ("input_preserved", False),
                           ("complete_recovery_equality", False),
                           ("upkeep_interval", 400), ("end_tick", 60799)):
            report, metrics = self.witness()
            report[key] = value
            with self.subTest(key=key), self.assertRaises(RuntimeError):
                self.validate(report, metrics)

    def test_rejects_granted_construction_or_missing_paid_commands(self):
        for commands in ([], [{"cost": 80, "funds_before": 79, "funds_after": -1}],
                         [{"cost": 80, "funds_before": 12000, "funds_after": 12000}]):
            report, metrics = self.witness()
            report["initial_commands"] = commands
            with self.subTest(commands=commands), self.assertRaises(RuntimeError):
                self.validate(report, metrics)

    def test_rejects_unobserved_100000_tick_claim(self):
        report, metrics = self.witness()
        report["minimum_100000_post_maturity_ticks"] = True
        with self.assertRaisesRegex(RuntimeError, "duration"):
            self.validate(report, metrics)

    def test_rejects_mistyped_final_world_goal(self):
        report, metrics = self.witness()
        metrics["final_goal"]["reached"] = 1
        with self.assertRaisesRegex(RuntimeError, "World goal"):
            self.validate(report, metrics)

    def test_maturity_cannot_claim_missing_actual_supplies(self):
        for key in ("food_house_received", "goods_consumed", "actual_service_visits",
                    "actual_water_visits", "actual_inspector_arrivals", "inhabited_houses"):
            report, metrics = self.witness()
            report["maturity"][key] = 0
            with self.subTest(key=key), self.assertRaises(RuntimeError):
                self.validate(report, metrics)

    def test_reached_maturity_requires_first_observed_goal(self):
        report, metrics = self.witness()
        report["first_goal"] = None
        with self.assertRaisesRegex(RuntimeError, "first observed"):
            self.validate(report, metrics)


class FinanceStreamEvidence(unittest.TestCase):
    def witness(self):
        metrics = {"begin_tick": 20000, "end_tick": 60800,
                   "summary": {"windows20400": OperatingEvidence().rows()},
                   "finance": [{"tick": 40400, "tax": 8000, "upkeep": 7500},
                               {"tick": 60800, "tax": 7600, "upkeep": 7600}]}
        seed = {"treasury": 200, "taxes_collected_total": 0, "maintenance_spent_total": 0}
        final = {"taxes_collected_total": 15600, "maintenance_spent_total": 15100}
        return metrics, seed, final

    def test_complete_actual_stream_matches_both_windows(self):
        runner.finance_events(*self.witness())

    def test_rejects_invented_profitable_subwindow_even_when_full_total_is_correct(self):
        metrics, seed, final = self.witness()
        # Moving income between windows would preserve lifetime totals, but
        # must not manufacture consecutive profitable windows.
        metrics["summary"]["windows20400"][0]["taxes"] += 500
        metrics["summary"]["windows20400"][1]["taxes"] -= 500
        with self.assertRaisesRegex(RuntimeError, "subwindow"):
            runner.finance_events(metrics, seed, final)

    def test_rejects_missing_or_duplicate_invoice_events(self):
        for change in ("missing", "duplicate", "outside", "mistyped"):
            metrics, seed, final = self.witness()
            if change == "missing":
                metrics["finance"].pop()
            elif change == "duplicate":
                metrics["finance"].append(copy.deepcopy(metrics["finance"][-1]))
            elif change == "outside":
                metrics["finance"][-1]["tick"] += 1
            else:
                metrics["finance"][-1]["upkeep"] = True
            with self.subTest(change=change), self.assertRaises(RuntimeError):
                runner.finance_events(metrics, seed, final)

    def test_rejects_window_starting_from_unearned_funds(self):
        metrics, seed, final = self.witness()
        seed["treasury"] += 1
        with self.assertRaisesRegex(RuntimeError, "earned seed"):
            runner.finance_events(metrics, seed, final)


if __name__ == "__main__":
    unittest.main()
