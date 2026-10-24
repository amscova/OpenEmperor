"""Negative acceptance evidence controls; no simulation mocks count as gameplay."""
import copy
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import test_fire_protection as runner


class EvidenceControls(unittest.TestCase):
    def witness(self):
        return {"status": "PASS", "rule": 11, "schema": 27, "policy": 3, "revision": 27,
                "snapshot_injection": False, "first_goal": {"reached": True, "tick": 816},
                "begin_tick": 816, "end_tick": 60816, "fire": {"actual_arrivals": 1,
                    "protected_risk_steps": 1, "visits": [{"couriers": [18], "tick": 900,
                        "risk_after": 0, "deadline": 3300}]}}

    def test_accepts_bound_actual_descriptor(self):
        runner.validate_process(self.witness(), 11, 60000)

    def test_rejects_missing_false_or_mistyped_world_goal(self):
        for value in (None, {}, {"reached": False, "tick": 816}, {"reached": 1, "tick": 816},
                      {"reached": True, "tick": "816"}, {"reached": True, "tick": 815}):
            with self.subTest(value=value):
                witness = self.witness()
                witness["first_goal"] = value
                with self.assertRaises(RuntimeError):
                    runner.validate_process(witness, 11, 60000)

    def test_rejects_changed_authority_or_interval(self):
        for key, value in (("rule", 10), ("schema", 26), ("policy", 2), ("revision", 28),
                           ("snapshot_injection", True), ("end_tick", 60000), ("status", "FAIL")):
            witness = self.witness()
            witness[key] = value
            with self.subTest(key=key), self.assertRaises(RuntimeError):
                runner.validate_process(witness, 11, 60000)

    def test_rejects_coverage_without_actual_arrival(self):
        for key, value in (("actual_arrivals", 0), ("actual_arrivals", True),
                           ("protected_risk_steps", 0), ("visits", [])):
            witness = self.witness()
            witness["fire"][key] = value
            with self.subTest(key=key), self.assertRaises(RuntimeError):
                runner.validate_process(witness, 11, 60000)

    def test_rejects_impossible_arrival_effect(self):
        for key, value in (("couriers", []), ("risk_after", 1), ("deadline", 3301)):
            witness = copy.deepcopy(self.witness())
            witness["fire"]["visits"][0][key] = value
            with self.subTest(key=key), self.assertRaises(RuntimeError):
                runner.validate_process(witness, 11, 60000)


if __name__ == "__main__":
    unittest.main()
