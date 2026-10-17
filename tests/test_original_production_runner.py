#!/usr/bin/env python3
"""Production-runner private-output and real-origin guards without licensed data."""
import copy
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

SPEC = importlib.util.spec_from_file_location(
    "original_production_runner", Path(__file__).resolve().parents[1] / "tools/test_original_production.py")
runner = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(runner)


class ProductionRunnerTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="production-output-guards-")
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name).resolve(strict=True)
        self.repo, self.original, self.build, self.outside = (self.root / name for name in (
            "repo", "original", "build", "outside"))
        for path in (self.repo, self.original, self.build, self.outside):
            path.mkdir()
        self.private = self.repo / runner.PRIVATE_OUTPUT

    def entries(self):
        return sorted(str(path.relative_to(self.root)) for path in self.root.rglob("*"))

    def create(self, output):
        return runner.create_output(self.repo, output, self.original, self.build)

    def test_valid_preflight_does_not_write(self):
        before = self.entries()
        output = self.private / "new" / "run"
        self.assertEqual(runner.preflight_output(self.repo, output, self.original, self.build), (self.private, output))
        self.assertEqual(runner.preflight_output(self.repo, None, self.original, self.build), (self.private, None))
        self.assertEqual(self.entries(), before)

    def test_outside_and_older_generation_roots_are_rejected_before_write(self):
        before = self.entries()
        for output in (self.outside / "run", self.repo / "report", self.original / "run", self.build / "run",
                       self.private, self.private / ".." / "escape",
                       self.repo / ".local/original-economy/oe14/production-fixtures/run",
                       self.repo / ".local/original-economy/oe15/production-fixtures/run",
                       self.repo / ".local/original-economy/oe16/production-fixtures/run",
                       self.repo / ".local/original-economy/oe17/production-fixtures/run",
                       self.repo / ".local/original-economy/oe18/production-fixtures/run",
                       self.repo / ".local/original-economy/oe19/production-fixtures/run",
                       self.repo / ".local/original-economy/oe20/production-fixtures/run",
                       self.repo / ".local/original-economy/oe13/goods-fixtures/run"):
            with self.subTest(output=output), self.assertRaises(RuntimeError):
                self.create(output)
            self.assertEqual(self.entries(), before)

    def test_existing_output_and_marker_are_preserved(self):
        output = self.private / "existing"
        output.mkdir(parents=True)
        marker = output / "report.json"
        marker.write_bytes(b"retain earlier proof\n")
        before = self.entries()
        for path in (output, marker):
            with self.subTest(path=path), self.assertRaises(RuntimeError):
                self.create(path)
        self.assertEqual(marker.read_bytes(), b"retain earlier proof\n")
        self.assertEqual(self.entries(), before)

    def test_symlinked_private_ancestor_is_rejected_for_default_and_explicit(self):
        (self.repo / ".local").symlink_to(self.outside, target_is_directory=True)
        before = self.entries()
        for output in (None, self.private / "run"):
            with self.subTest(output=output), self.assertRaises(RuntimeError):
                self.create(output)
        self.assertEqual(self.entries(), before)
        self.assertEqual(list(self.outside.iterdir()), [])

    def test_external_internal_and_dangling_symlinks_are_rejected(self):
        self.private.mkdir(parents=True)
        inside = self.private / "real"
        inside.mkdir()
        for index, target in enumerate((self.outside, inside, self.outside / "missing")):
            link = self.private / f"link-{index}"
            link.symlink_to(target, target_is_directory=True)
            before = self.entries()
            for output in (link, link / "run"):
                with self.subTest(output=output), self.assertRaises(RuntimeError):
                    self.create(output)
            self.assertEqual(self.entries(), before)
        self.assertEqual(list(inside.iterdir()), [])
        self.assertEqual(list(self.outside.iterdir()), [])

    def test_original_and_build_cannot_overlap_output_in_either_direction(self):
        nested = self.private / "source"
        nested.mkdir(parents=True)
        for original, build in ((nested, self.build), (self.original, nested),
                                (self.repo, self.build), (self.original, self.repo)):
            before = self.entries()
            with self.subTest(original=original, build=build), self.assertRaises(RuntimeError):
                runner.preflight_output(self.repo, self.private / "run", original, build)
            self.assertEqual(self.entries(), before)

    def test_non_directory_parent_is_preserved(self):
        self.private.mkdir(parents=True)
        marker = self.private / "file"
        marker.write_bytes(b"keep\n")
        before = self.entries()
        with self.assertRaises(RuntimeError):
            self.create(marker / "run")
        self.assertEqual(self.entries(), before)
        self.assertEqual(marker.read_bytes(), b"keep\n")

    def test_new_default_and_explicit_output_are_private_and_empty(self):
        default = self.create(None)
        explicit = self.create(self.private / "new" / "explicit")
        self.assertTrue(default.name.startswith("run-"))
        for output in (default, explicit):
            self.assertTrue(output.resolve().is_relative_to(self.private))
            self.assertEqual(list(output.iterdir()), [])
        self.assertEqual(list(self.original.iterdir()), [])
        self.assertEqual(list(self.build.iterdir()), [])

    def test_record_paths_cannot_escape_or_follow_links(self):
        output = self.create(None)
        good = output / "save.json"
        good.write_text("{}")
        self.assertEqual(runner.checked_path(self.repo, output, good), good)
        link = output / "alias.json"
        link.symlink_to(good)
        for path in (Path("save.json"), self.outside / "save.json", output / ".." / "escape", link):
            with self.subTest(path=path), self.assertRaises(RuntimeError):
                runner.checked_path(self.repo, output, path)

    @staticmethod
    def scene():
        return {"balances": [{"good": index, "supported": index < 2, "conserved": True,
                    "produced": 0, "imported": 0, "consumed": 0, "demolished": 0,
                    "stock": 0, "cargo": 0, "source_reserved": 0, "incoming_reserved": 0}
                for index in range(6)], "pottery_handed_off": 0, "pottery_completed": 0,
                "hemp_harvested": 0, "hemp_farms": [],
                "residents_admitted": 0, "population": 0, "workforce_supply": 0,
                "houses": [], "stocks": [], "couriers": []}

    def test_balance_guards_distinguish_supported_origin_and_actual_population(self):
        scene = self.scene()
        runner.check_balances(scene)
        with self.assertRaises(AssertionError):
            runner.check_balances(scene, positive=True)
        scene["balances"][1]["produced"] = scene["balances"][1]["stock"] = 2
        scene["balances"][0]["produced"] = scene["balances"][0]["stock"] = 2
        scene.update(pottery_handed_off=2, pottery_completed=2, hemp_harvested=2, hemp_farms=[{"harvested": 2}],
                     residents_admitted=42, population=42, workforce_supply=42,
                     houses=[{"occupants": 7, "hemp": 2 if index == 0 else 0,
                              "ceramics": 2 if index == 0 else 0, "consumed": [0] * 6} for index in range(6)])
        runner.check_balances(scene, positive=True)
        for field in ("imported", "consumed", "demolished", "cargo"):
            bad = copy.deepcopy(scene)
            bad["balances"][1][field] = 1
            with self.subTest(field=field), self.assertRaises(AssertionError):
                runner.check_balances(bad)
        carrying = copy.deepcopy(scene)
        carrying["balances"][1].update(stock=0, cargo=2, source_reserved=0, incoming_reserved=2)
        carrying["houses"][0]["ceramics"] = 0
        carrying["couriers"] = [{"role": 8, "cargo": 0, "reserved": 0, "typed_cargo": 2, "typed_cargo_good": 1}]
        runner.check_balances(carrying, positive=False)
        mirrored = copy.deepcopy(carrying)
        mirrored["couriers"][0]["cargo"] = 2
        with self.assertRaises(AssertionError):
            runner.check_balances(mirrored)
        unsupported_stock = copy.deepcopy(scene)
        unsupported_stock["stocks"] = [{"kind": 3, "amounts": [0, 0, 1, 0, 0, 0]}]
        with self.assertRaises(AssertionError):
            runner.check_balances(unsupported_stock)
        for key in ("pottery_completed", "pottery_handed_off", "hemp_harvested", "residents_admitted", "population"):
            bad = copy.deepcopy(scene)
            bad[key] = 0
            with self.subTest(key=key), self.assertRaises(AssertionError):
                runner.check_balances(bad)

    def test_balanced_unsupported_forgery_wrong_identity_flags_and_domains_fail(self):
        for good in (2, 3, 4, 5):
            bad = self.scene()
            bad["balances"][good]["produced"] = bad["balances"][good]["stock"] = 1
            with self.subTest(good=good), self.assertRaises(AssertionError):
                runner.check_balances(bad)
        for field, value in (("good", 0), ("supported", False), ("conserved", False),
                             ("stock", -1), ("produced", True), ("cargo", 0.0)):
            bad = self.scene()
            bad["balances"][1][field] = value
            with self.subTest(field=field), self.assertRaises(AssertionError):
                runner.check_balances(bad)

    def test_inventory_and_save_identity_detect_bytes_and_metadata_changes(self):
        source = self.original / "file"
        source.write_bytes(b"original\n")
        before = runner.inventory(self.original)
        identity = runner.file_identity(source)
        mode = source.stat().st_mode
        self.assertEqual(runner.inventory(self.original), before)
        self.assertEqual(runner.file_identity(source), identity)
        source.chmod(mode ^ 0o200)
        self.assertNotEqual(runner.inventory(self.original), before)
        self.assertNotEqual(runner.file_identity(source), identity)
        source.chmod(mode)
        source.write_bytes(b"changed\n")
        self.assertNotEqual(runner.file_identity(source)["sha256"], identity["sha256"])

    def test_consumption_requires_exact_house_sinks_and_conservation(self):
        scene = self.scene()
        scene.update(pottery_handed_off=3, pottery_completed=3, hemp_harvested=2, hemp_farms=[{"harvested": 2}],
                     residents_admitted=7, population=7, workforce_supply=7,
                     houses=[{"occupants": 7, "hemp": 1, "ceramics": 1, "consumed": [1, 2, 0, 0, 0, 0]}])
        scene["balances"][0].update(produced=2, stock=1, consumed=1)
        scene["balances"][1].update(produced=3, stock=1, consumed=2)
        runner.check_balances(scene)
        for field, value in (("consumed", [2, 2, 0, 0, 0, 0]), ("consumed", [1, 2, 0, 0, 0, 1]),
                             ("consumed", [True, 2, 0, 0, 0, 0]), ("consumed", [1, 2]), ("occupants", 0)):
            bad = copy.deepcopy(scene)
            bad["houses"][0][field] = value
            with self.subTest(field=field, value=value), self.assertRaises(AssertionError):
                runner.check_balances(bad)

    def test_authored_model_copy_changes_only_two_explicit_need_bytes(self):
        fields = [b"0"] * 24
        fields[17] = b"7"
        source = b"other original rows\r\n1: Shelter,{," + b",".join(fields) + b",,,,,\r\nunchanged footer\r\n"
        expected = fields.copy()
        expected[9] = expected[10] = b"1"
        changed = runner.authored_consumption_model(source)
        self.assertEqual(changed, b"other original rows\r\n1: Shelter,{," + b",".join(expected) + b",,,,,\r\nunchanged footer\r\n")
        for bad in (source + source, source.replace(b"1: Shelter", b"1: Unknown"), changed,
                    source.replace(b"7", b"8"), b"1: Shelter,{,0"):
            with self.subTest(source=bad), self.assertRaises(RuntimeError):
                runner.authored_consumption_model(bad)

    def test_boundary_guard_checks_scheduler_delta_and_no_second_booking(self):
        before = self.scene()
        rows = [[0] * 24 for _ in range(3)]
        for row in rows:
            row[0], row[1], row[17] = -99, -10, 7
        rows[0][9] = rows[0][10] = rows[1][3] = 1
        before.update(tick=855, shelter_needs=[1, 1], pottery_handed_off=12, pottery_completed=12,
                      hemp_harvested=6, hemp_farms=[{"harvested": 6}], residents_admitted=42,
                      population=42, workforce_supply=42,
                      houses=[{"id": i + 1, "occupants": 7, "hemp": 1, "ceramics": 2,
                               "type": 3, "devolve_counter": 4, "water": False,
                               "desirability": 0, "service_timers": [0] * 9, "food_quality": 0,
                               "consumed": [0] * 6} for i in range(6)])
        before["balances"][0].update(produced=6, stock=6)
        before["balances"][1].update(produced=12, stock=12)
        def attach_housing(scene, evaluation_tick, before_counter, goods, events, consumed):
            scene["capacity"] = 42
            scene["buildings"] = [{"id": i + 1, "kind": 6, "cell": [3 * i, 2], "placed_tick": 0} for i in range(6)]
            histories = []
            for i in range(6):
                evaluation = {"tick": evaluation_tick,
                    "input": {"type": 3, "occupants": 7, "desirability": 0, "devolve_counter": before_counter,
                        "water": False, "service_timers": [0] * 9, "food_quality": 0, "goods": goods.copy()},
                    "service_witnesses": [None] * 3, "water_evaluation_witness": None, "food_quality_witness": None,
                    "food_received": [0, 0], "food_consumed": [0, 0], "goods_consumed_before": [0] * 6}
                histories.append({"id": i + 1, "first_admission_tick": 120, "last_evaluation": evaluation,
                    "first_hut_evaluation": None, "first_plain_evaluation": None,
                    "consumption_rows": [{"events": events, "consumed": consumed.copy()},
                        {"events": 0, "consumed": [0] * 6}, {"events": 0, "consumed": [0] * 6}]})
            scene["housing"] = {"model_rows": rows, "evaluation_tick": runner.housing_evaluation_tick(scene["tick"]),
                "consumption_events": runner.housing_consumption_events(scene["tick"]),
                "neutral_source_origins": [], "houses": histories}
        attach_housing(before, 805, 3, [0] * 6, 1, [0] * 6)
        at = copy.deepcopy(before)
        at["tick"] = 856
        at["balances"][0].update(stock=0, consumed=6)
        at["balances"][1].update(stock=6, consumed=6)
        for house in at["houses"]:
            house.update(hemp=0, ceramics=1, devolve_counter=0, consumed=[1, 1, 0, 0, 0, 0])
        attach_housing(at, 856, 4, [1, 2, 0, 0, 0, 0], 2, [1, 1, 0, 0, 0, 0])
        after = copy.deepcopy(at)
        after["tick"] = 857
        boundary = {"step": 0, "house_arrivals_at_boundary": False, "before": before, "at": at, "after": after,
                    "expected_house_deltas": [{"building_id": i + 1, "evolved_type": 3, "evolved_counter": 0,
                        "evolved_desirability": 0, "consumed": [1, 1, 0, 0, 0, 0]} for i in range(6)]}
        runner.check_consumption_boundary(boundary)
        for field, value in (("step", 7), ("house_arrivals_at_boundary", True)):
            bad = copy.deepcopy(boundary)
            bad[field] = value
            with self.subTest(field=field), self.assertRaises(AssertionError):
                runner.check_consumption_boundary(bad)
        bad = copy.deepcopy(boundary)
        bad["after"]["houses"][0]["consumed"][0] += 1
        with self.assertRaises(AssertionError):
            runner.check_consumption_boundary(bad)
        bad = copy.deepcopy(boundary)
        bad["at"]["houses"][1]["devolve_counter"] = 4
        with self.assertRaises(AssertionError):
            runner.check_consumption_boundary(bad)

    def test_frozen_rule9_requires_pinned_completed_review(self):
        root = self.repo / runner.RULE9_REFERENCE
        root.mkdir(parents=True)
        (root / "report.json").write_text(json.dumps({"status": "PASS"}))
        (root / "review-validation.json").write_text(json.dumps({"status": "PASS", "all_files": []}))
        before = self.entries()
        with self.assertRaises(RuntimeError):
            runner.frozen_rule9(self.repo)
        self.assertEqual(self.entries(), before)

    @staticmethod
    def food_scene():
        def stock(identity, kind, amounts, received, departed, consumed):
            return {"id": identity, "kind": kind, "amounts": amounts, "received": received,
                    "departed": departed, "returned": [0, 0], "consumed": consumed,
                    "missed_demands": 0, "quality": 0, "quality_witness": None}
        rows = [stock(1, 6, [1, 1], [2, 2], [0, 0], [1, 1]),
                stock(2, 16, [0, 0], [0, 0], [4, 0], [0, 0]),
                stock(3, 9, [2, 2], [4, 4], [2, 2], [0, 0]),
                stock(4, 16, [0, 0], [0, 0], [0, 4], [0, 0])]
        rows[0].update(quality=50, quality_witness={"courier_id": 2, "sequence": 1, "tick": 10,
            "accepted": [2, 2], "consumed_before": [0, 0], "previous_quality": 0})
        trips = [{"courier_id": courier, "sequence": 1, "source": source, "target": target, "phase": 3,
                  "loaded": amounts, "delivered": amounts.copy(), "returned": [0, 0], "cargo": [0, 0], "batch_quality": 0}
                 for courier, source, target, amounts in ((1, 2, 3, [4, 0]), (2, 3, 1, [2, 2]), (3, 4, 3, [0, 4]))]
        return {"tick": 11, "houses": [{"id": 1, "food_quality": 50}],
            "couriers": [{"id": courier, "role": role, "food_cargo": 0} for courier, role in ((1, 13), (2, 14), (3, 13))],
            "food": {"conserved": True, "farms": [{"farm_id": 2, "field_id": 5, "crop": 0, "harvested": 4},
                        {"farm_id": 4, "field_id": 6, "crop": 1, "harvested": 4}],
                     "stocks": rows, "trips": trips, "produced": [4, 4], "stock": [3, 3], "cargo": [0, 0], "consumed": [1, 1]}}

    def test_food_arrival_witness_and_per_crop_mass_are_checked_independently(self):
        scene = self.food_scene()
        runner.check_food(scene, positive=True)
        for mutation in (lambda row: row["food"]["stocks"][0].update(quality=51),
                         lambda row: row["food"]["stocks"][0]["quality_witness"].update(previous_quality=20),
                         lambda row: row["food"]["stocks"][0]["quality_witness"].update(accepted=[3, 1]),
                         lambda row: row["food"]["stocks"][0]["quality_witness"].update(courier_id=1),
                         lambda row: row["food"]["stocks"][0]["quality_witness"].update(sequence=2),
                         lambda row: row["food"]["stocks"][0]["quality_witness"].update(tick=12),
                         lambda row: row["food"]["stocks"][0].update(amounts=[True, 1]),
                         lambda row: row["food"]["stocks"][1].update(departed=[3, 0]),
                         lambda row: row["food"]["trips"][1].update(batch_quality=20),
                         lambda row: row["food"]["produced"].__setitem__(1, 5)):
            bad = copy.deepcopy(scene)
            mutation(bad)
            with self.subTest(scene=bad), self.assertRaises(AssertionError):
                runner.check_food(bad)

    def test_food_quality_is_zero_when_consumed_empty_and_cannot_be_credited(self):
        scene = self.food_scene()
        home = scene["food"]["stocks"][0]
        home.update(amounts=[0, 0], consumed=[2, 2], quality=0)
        scene["houses"][0]["food_quality"] = 0
        scene["food"].update(stock=[2, 2], consumed=[2, 2])
        runner.check_food(scene, positive=True)
        home["quality"] = scene["houses"][0]["food_quality"] = 50
        with self.assertRaises(AssertionError):
            runner.check_food(scene)
        empty = self.food_scene()
        empty["food"]["stocks"][0]["quality_witness"] = None
        with self.assertRaises(AssertionError):
            runner.check_food(empty)

    def test_food_weighted_score_and_original_blend_vectors(self):
        for amounts, expected in (([0, 0], 0), ([4, 0], 20), ([0, 4], 80), ([3, 1], 35), ([1, 1], 50), ([1, 4], 68)):
            self.assertEqual(runner.food_batch_quality(amounts), expected)
        for amounts in ([-1, 1], [True, 1], [0.0, 1], [2**31 - 1, 1]):
            with self.subTest(amounts=amounts), self.assertRaises(AssertionError):
                runner.food_batch_quality(amounts)
        for delivered, previous, expected in ((1, 4, 65), (1, 2, 60), (2, 2, 50), (3, 1, 40), (4, 1, 35)):
            self.assertEqual(runner.food_blend(80, 20, delivered, previous), expected)
        self.assertEqual(runner.food_blend(20, 80, 0, 4), 20)
        self.assertEqual(runner.food_blend(20, 80, 1, 4), 80)

    @staticmethod
    def water_scene(tick=500, evaluated=True):
        old = {"house_id": 1, "provider_id": 2, "courier_id": 1, "sequence": 1, "arrival_tick": 450}
        current = {**old, "sequence": 2, "arrival_tick": 490}
        return {"tick": tick,
            "houses": [{"id": 1, "water": bool(evaluated)}],
            "buildings": [{"id": 1, "kind": 6, "placed_tick": 0}, {"id": 2, "kind": 20, "placed_tick": 0}],
            "couriers": [{"id": 1, "role": 16, "owner": 2, "target": 1, "last_dispatched_target": 1,
                "phase": 2, "cargo": 0, "reserved": 0, "typed_cargo": 0, "food_cargo": 0, "typed_cargo_good": None}],
            "water": {"providers": [{"id": 2, "cost": 25, "workers_required": 4}],
                "trips": [{"courier_id": 1, "sequence": 2, "target": 1, "dispatched_tick": 470,
                           "arrival_tick": 490, "visit_applied": True}],
                "houses": [{"id": 1, "normal_timer": runner.water_timer(current, tick), "special_timer": 0,
                            "current_witness": current, "evaluation_witness": old if evaluated else None}],
                "countdowns": runner.service_countdowns(tick), "evaluation_tick": runner.water_evaluation_tick(tick)}}

    def test_water_current_arrival_and_cached_phase28_witness_are_distinct(self):
        for evaluated in (False, True):
            scene = self.water_scene(evaluated=evaluated)
            runner.check_water(scene)
            wrong = copy.deepcopy(scene)
            wrong["houses"][0]["water"] = not evaluated
            with self.subTest(evaluated=evaluated), self.assertRaises(AssertionError):
                runner.check_water(wrong)
        scene = self.water_scene()
        for mutation in (
                lambda row: row["water"]["houses"][0].update(normal_timer=96),
                lambda row: row["water"]["houses"][0].update(special_timer=1),
                lambda row: row["water"]["houses"][0]["evaluation_witness"].update(arrival_tick=490),
                lambda row: row["water"]["houses"][0]["current_witness"].update(provider_id=1),
                lambda row: row["water"]["trips"][0].update(visit_applied=False),
                lambda row: row["water"]["trips"][0].update(dispatched_tick=120),
                lambda row: row["water"]["providers"][0].update(workers_required=0),
                lambda row: row["couriers"][0].update(role=15),
                lambda row: row["couriers"][0].update(cargo=1),
                lambda row: row["buildings"][1].update(kind=13)):
            wrong = copy.deepcopy(scene)
            mutation(wrong)
            with self.subTest(scene=wrong), self.assertRaises(AssertionError):
                runner.check_water(wrong)

    def test_water_zero_raw_timer_can_remain_materialized_until_next_phase28(self):
        scene = self.water_scene(tick=5340)
        current = scene["water"]["houses"][0]["current_witness"]
        scene["water"]["houses"][0]["evaluation_witness"] = current.copy()
        self.assertEqual(scene["water"]["houses"][0]["normal_timer"], 0)
        self.assertGreater(runner.water_timer(current, runner.water_evaluation_tick(scene["tick"])), 0)
        runner.check_water(scene)
        scene["houses"][0]["water"] = False
        with self.assertRaises(AssertionError):
            runner.check_water(scene)

    @staticmethod
    def housing_scene():
        rows = [[0] * 24 for _ in range(3)]
        for i, row in enumerate(rows):
            row[0], row[1], row[17] = -99, -5 if i < 2 else 99, 7 + 7 * i
        rows[1][3] = rows[2][3] = rows[2][21] = 1
        rows[2][8] = 20
        i = {"type": 3, "occupants": 4, "devolve_counter": 0, "water": True, "desirability": 0,
             "service_timers": [0] * 9, "food_quality": 0, "goods": [0] * 6}
        e = {"tick": 193, "input": i, "service_witnesses": [None] * 3,
             "water_evaluation_witness": {"house_id": 1, "provider_id": 2, "courier_id": 1,
                                          "sequence": 1, "arrival_tick": 150},
             "food_quality_witness": None, "food_received": [0, 0], "food_consumed": [0, 0],
             "goods_consumed_before": [0] * 6}
        return {"tick": 193, "capacity": 14,
                "houses": [{"id": 1, "type": 4, "occupants": 4, "devolve_counter": 0, "consumed": [0] * 6}],
                "buildings": [{"id": 1, "kind": 6, "cell": [0, 2], "placed_tick": 0}],
                "housing": {"model_rows": rows, "evaluation_tick": 193,
                    "consumption_events": runner.housing_consumption_events(193), "neutral_source_origins": [],
                    "houses": [{"id": 1, "first_admission_tick": 120,
                        "consumption_rows": [{"events": 0, "consumed": [0] * 6} for _ in range(3)],
                        "last_evaluation": e, "first_hut_evaluation": copy.deepcopy(e), "first_plain_evaluation": None}]}}

    def test_housing_upgrade_requires_real_input_and_per_type_historical_consumption(self):
        scene = self.housing_scene()
        runner.check_housing(scene)
        for mutation in (
                lambda row: row["houses"][0].update(type=5),
                lambda row: row["houses"][0].update(occupants=15),
                lambda row: row["houses"][0].update(devolve_counter=1),
                lambda row: row["housing"]["houses"][0].update(first_admission_tick=121),
                lambda row: row["housing"]["houses"][0].update(first_hut_evaluation=None),
                lambda row: row["housing"]["houses"][0]["first_hut_evaluation"]["input"].update(water=False),
                lambda row: row["housing"]["houses"][0]["last_evaluation"].update(tick=192),
                lambda row: row["housing"]["houses"][0]["last_evaluation"]["water_evaluation_witness"].update(arrival_tick=190),
                lambda row: row["housing"]["houses"][0]["consumption_rows"][0].update(consumed=[1, 0, 0, 0, 0, 0]),
                lambda row: row["housing"]["houses"][0]["consumption_rows"][1].update(events=2),
                lambda row: row.update(capacity=7)):
            wrong = copy.deepcopy(scene)
            mutation(wrong)
            with self.subTest(scene=wrong), self.assertRaises(AssertionError):
                runner.check_housing(wrong)

    def test_housing_reference_tenth_failure_and_original_plain_zero_goods_requirements(self):
        scene = self.housing_scene()
        rows = scene["housing"]["model_rows"]
        state = copy.deepcopy(scene["housing"]["houses"][0]["last_evaluation"]["input"])
        state.update(type=5, food_quality=0, devolve_counter=8)
        self.assertEqual(runner.housing_step(rows, state), (5, 9))
        state["devolve_counter"] = 9
        self.assertEqual(runner.housing_step(rows, state), (4, 0))
        state.update(type=4, devolve_counter=0, food_quality=20)
        state["service_timers"][5] = 40
        self.assertEqual(runner.housing_step(rows, state), (5, 0))
        rows[2][9] = 1  # Explicitly authored missing-Hemp requirement.
        self.assertEqual(runner.housing_step(rows, state), (4, 0))


    @staticmethod
    def services_scene():
        tick = 300
        witnesses = [{"house_id": 1, "role": role, "provider_id": role + 2,
                      "courier_id": role + 1, "sequence": 1, "arrival_tick": 250 + 10 * role}
                     for role in (0, 1)]
        providers = [{"id": role + 2, "type": role, "model_type": 207 + role,
                      "cost": 30 + 20 * role, "workers_required": 7 + role} for role in (0, 1)]
        trips = [{"courier_id": w["courier_id"], "sequence": 1, "target": 1,
                  "dispatched_tick": 200, "arrival_tick": w["arrival_tick"], "visit_applied": True}
                 for w in witnesses]
        couriers = [{"id": role + 1, "role": 11 if role == 0 else 5, "owner": role + 2,
                     "target": 1, "last_dispatched_target": 1, "phase": 2,
                     "cargo": 0, "reserved": 0, "typed_cargo": 0, "typed_cargo_good": None,
                     "food_cargo": 0} for role in (0, 1)]
        buildings = [{"id": identity, "kind": kind, "placed_tick": 0, "operating": False}
                     for identity, kind in ((1, 6), (2, 12), (3, 8))]
        return {"tick": tick, "houses": [{"id": 1, "occupants": 7, "status": 1, "access_enabled": True,
                                            "service_timers": [95, 95] + [0] * 7}],
                "buildings": buildings, "couriers": couriers,
                "services": {"providers": providers, "trips": trips, "witnesses": witnesses,
                             "countdowns": runner.service_countdowns(tick)}}

    def test_medical_timers_require_actual_matching_source_trip_and_phase35_origin(self):
        scene = self.services_scene()
        runner.check_services(scene, positive=True)
        mutations = (
            lambda row: row["houses"][0]["service_timers"].__setitem__(0, 96),
            lambda row: row["houses"][0]["service_timers"].__setitem__(2, 1),
            lambda row: row["houses"][0]["service_timers"].__setitem__(0, True),
            lambda row: row["houses"][0].update(access_enabled=False),
            lambda row: row["houses"][0].update(occupants=0),
            lambda row: row["services"]["providers"][0].update(type=1),
            lambda row: row["services"]["providers"][0].update(model_type=208),
            lambda row: row["services"]["providers"][0].update(workers_required=6),
            lambda row: row["services"]["trips"][0].update(visit_applied=False),
            lambda row: row["services"]["trips"][0].update(arrival_tick=None),
            lambda row: row["services"]["trips"][0].update(dispatched_tick=120),
            lambda row: row["services"]["witnesses"][0].update(arrival_tick=301),
            lambda row: row["services"]["witnesses"][0].update(provider_id=3),
            lambda row: row["services"]["witnesses"][0].update(sequence=2),
            lambda row: row["services"]["witnesses"].pop(),
            lambda row: row["couriers"][0].update(cargo=1),
            lambda row: row["couriers"][0].update(food_cargo=1),
            lambda row: row["couriers"][0].update(role=5),
            lambda row: row["couriers"][0].update(last_dispatched_target=2),
            lambda row: row["services"].update(countdowns=7),
        )
        for index, mutation in enumerate(mutations):
            bad = copy.deepcopy(scene)
            mutation(bad)
            with self.subTest(mutation=index), self.assertRaises(AssertionError):
                runner.check_services(bad)

    def test_medical_compacted_previous_visit_remains_bound_to_later_actual_dispatch(self):
        scene = self.services_scene()
        scene["services"]["trips"][0].update(sequence=2, dispatched_tick=280, arrival_tick=None, visit_applied=False)
        scene["couriers"][0]["phase"] = 1
        runner.check_services(scene, positive=True)
        for arrival in (280, 290):
            bad = copy.deepcopy(scene)
            bad["services"]["witnesses"][0]["arrival_tick"] = arrival
            with self.subTest(arrival=arrival), self.assertRaises(AssertionError):
                runner.check_services(bad)
        bad = copy.deepcopy(scene)
        bad["services"]["trips"].pop(0)
        with self.assertRaises(AssertionError):
            runner.check_services(bad)

    def test_medical_exact96th_countdown_expiry_retains_arrival_witness(self):
        visited = self.services_scene()
        before, after = copy.deepcopy(visited), copy.deepcopy(visited)
        expiry = 36 + (runner.service_countdowns(249) + 95) * 51
        before["tick"], after["tick"] = expiry - 1, expiry
        for scene, timers in ((before, [1, 1]), (after, [0, 0])):
            scene["services"]["countdowns"] = runner.service_countdowns(scene["tick"])
            scene["houses"][0]["service_timers"] = timers + [0] * 7
        fixture = {"visited": visited, "before_expiry": before, "expired": after, "expiry_tick": expiry}
        runner.check_service_expiry(fixture)
        for mutation in (lambda row: row.update(expiry_tick=expiry + 1),
                         lambda row: row["expired"]["houses"][0]["service_timers"].__setitem__(0, 1),
                         lambda row: row["before_expiry"]["buildings"][1].update(operating=True)):
            bad = copy.deepcopy(fixture)
            mutation(bad)
            with self.subTest(fixture=bad), self.assertRaises(AssertionError):
                runner.check_service_expiry(bad)

    def test_medical_countdown_boundary_is51_ticks_in_every_scheduler_step(self):
        for step in range(16):
            boundary = step * 51 + 36
            self.assertEqual(runner.service_countdowns(boundary) - runner.service_countdowns(boundary - 1), 1)
            self.assertEqual(runner.service_countdowns(boundary + 1), runner.service_countdowns(boundary))
        for tick in (-1, True, 1.0, 2**64):
            with self.subTest(tick=tick), self.assertRaises(AssertionError):
                runner.service_countdowns(tick)

    @staticmethod
    def ancestor_scene():
        scene = ProductionRunnerTests.services_scene()
        scene["buildings"].append({"id": 4, "kind": 19, "placed_tick": 0, "operating": False})
        scene["couriers"].append({"id": 3, "role": 15, "owner": 4, "target": 1,
            "last_dispatched_target": 1, "phase": 2, "cargo": 0, "reserved": 0,
            "typed_cargo": 0, "typed_cargo_good": None, "food_cargo": 0})
        scene["services"]["providers"].append({"id": 4, "type": 2, "model_type": 214, "cost": 40, "workers_required": 4})
        scene["services"]["trips"].append({"courier_id": 3, "sequence": 1, "target": 1,
            "dispatched_tick": 200, "arrival_tick": 270, "visit_applied": True})
        scene["services"]["witnesses"].append({"house_id": 1, "role": 5, "provider_id": 4,
            "courier_id": 3, "sequence": 1, "arrival_tick": 270})
        scene["houses"][0]["service_timers"][5] = 39
        return scene

    def test_ancestor_requires214_four_workers_own_actor_and_exact40_origin(self):
        scene = self.ancestor_scene()
        runner.check_services(scene, ancestor=True)
        mutations = (
            lambda row: row["houses"][0]["service_timers"].__setitem__(5, 40),
            lambda row: row["houses"][0]["service_timers"].__setitem__(5, 95),
            lambda row: row["houses"][0]["service_timers"].__setitem__(6, 1),
            lambda row: row["services"]["providers"][2].update(type=1),
            lambda row: row["services"]["providers"][2].update(model_type=207),
            lambda row: row["services"]["providers"][2].update(cost=0),
            lambda row: row["services"]["providers"][2].update(workers_required=3),
            lambda row: row["services"]["witnesses"][2].update(role=2),
            lambda row: row["services"]["witnesses"][2].update(provider_id=2),
            lambda row: row["services"]["trips"][2].update(visit_applied=False),
            lambda row: row["services"]["trips"][2].update(arrival_tick=None),
            lambda row: row["couriers"][2].update(role=5),
            lambda row: row["couriers"][2].update(cargo=1),
            lambda row: row["couriers"][2].update(food_cargo=1),
            lambda row: row["services"]["witnesses"].pop(),
        )
        for index, mutation in enumerate(mutations):
            bad = copy.deepcopy(scene)
            mutation(bad)
            with self.subTest(mutation=index), self.assertRaises(AssertionError):
                runner.check_services(bad)

    def test_ancestor_same_tick40_to39_and_noncountdown40_in_all16_steps(self):
        for step in range(16):
            for offset, expected in ((0, 39), (1, 40)):
                scene = self.ancestor_scene()
                at = 816 + step * 51 + 36 + offset
                scene["tick"] = at
                scene["services"]["countdowns"] = runner.service_countdowns(at)
                # This is a synthetic report-parser test. Real paid boundary
                # arrivals are separately exercised by OriginalAncestorWorldTests.
                scene["services"]["trips"][2].update(dispatched_tick=at - 50, arrival_tick=at)
                scene["services"]["witnesses"][2]["arrival_tick"] = at
                for w in scene["services"]["witnesses"]:
                    duration = 40 if w["role"] == 5 else 96
                    scene["houses"][0]["service_timers"][w["role"]] = max(
                        duration - (runner.service_countdowns(at) - runner.service_countdowns(w["arrival_tick"] - 1)), 0)
                with self.subTest(step=step, offset=offset):
                    self.assertEqual(scene["houses"][0]["service_timers"][5], expected)
                    runner.check_services(scene, ancestor=True)

    def test_ancestor_exact40th_countdown_expiry_keeps_real_visit(self):
        visited = self.ancestor_scene()
        before, after = copy.deepcopy(visited), copy.deepcopy(visited)
        expiry = 36 + (runner.service_countdowns(269) + 39) * 51
        before["tick"], after["tick"] = expiry - 1, expiry
        for scene in (before, after):
            scene["services"]["countdowns"] = runner.service_countdowns(scene["tick"])
            for w in scene["services"]["witnesses"]:
                duration = 40 if w["role"] == 5 else 96
                scene["houses"][0]["service_timers"][w["role"]] = max(
                    duration - (runner.service_countdowns(scene["tick"]) - runner.service_countdowns(w["arrival_tick"] - 1)), 0)
        fixture = {"visited": visited, "before_expiry": before, "expired": after, "expiry_tick": expiry}
        runner.check_ancestor_expiry(fixture)
        for mutation in (lambda row: row.update(expiry_tick=expiry + 1),
                         lambda row: row["expired"]["houses"][0]["service_timers"].__setitem__(5, 1),
                         lambda row: row["before_expiry"]["buildings"][3].update(operating=True)):
            bad = copy.deepcopy(fixture)
            mutation(bad)
            with self.subTest(mutation=mutation), self.assertRaises(AssertionError):
                runner.check_ancestor_expiry(bad)


if __name__ == "__main__":
    unittest.main()
