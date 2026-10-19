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
                       self.repo / ".local/original-economy/oe21/production-fixtures/run",
                       self.repo / ".local/original-economy/oe22/production-fixtures/run",
                       self.repo / ".local/original-economy/oe23/production-fixtures/run",
                       self.repo / ".local/original-economy/oe24/production-fixtures/run",
                       self.repo / ".local/original-economy/oe25/production-fixtures/run",
                       self.repo / ".local/original-economy/oe26/production-fixtures/run",
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
        return {"balances": [{"good": index, "supported": True, "conserved": True,
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

    @staticmethod
    def auxiliary_scene():
        # Parser-only reference: two produced Wood units, one held and one
        # physically loaded. This never supplies a positive World snapshot.
        scene = ProductionRunnerTests.scene()
        scene.update(tick=200, buildings=[{"id": 1, "kind": 27, "cell": [0, 0], "placed_tick": 0},
            {"id": 2, "kind": 3, "cell": [8, 0], "placed_tick": 0}],
            couriers=[{"id": 1, "owner": 1, "role": 24, "aux_cargo": 1, "cargo": 0, "reserved": 0}])
        source = {"id": 1, "plant_id": None, "completed": 2, "progress": 0,
                  "goods_inputs_used": [0]*6, "aux_inputs_used": [0]*5, "clay_received": 0, "clay_used": 0}
        stock = {"id": 1, "amounts": [1, 0, 0, 0, 0], "received": [0]*5,
                 "departed": [1, 0, 0, 0, 0], "returned": [0]*5, "consumed": [0]*5}
        empty = {"id": 2, "amounts": [0]*5, "received": [0]*5, "departed": [0]*5,
                 "returned": [0]*5, "consumed": [0]*5}
        trip = {"courier_id": 1, "sequence": 1, "source": 1, "target": 2, "material": 0, "phase": 1,
                "loaded": 1, "delivered": 0, "returned": 0, "dispatched_tick": 180, "arrival_tick": None}
        balances = [{"material": a, "produced": 2 if a == 0 else 0, "stock": 1 if a == 0 else 0,
                     "cargo": 1 if a == 0 else 0, "consumed": 0, "conserved": True} for a in range(5)]
        scene["industry"] = {"sources": [source], "managed_ore": [], "stocks": [stock, empty],
                             "trips": [trip], "clay_trips": [], "balances": balances}
        return scene

    def test_auxiliary_physical_cargo_and_owner_equations_reject_mirrors(self):
        scene = self.auxiliary_scene()
        runner.check_industry(scene)
        for mutate in (
            lambda s: s["industry"]["stocks"][0]["amounts"].__setitem__(0, 2),
            lambda s: s["couriers"][0].__setitem__("cargo", 1),
            lambda s: s["industry"]["trips"][0].__setitem__("delivered", 1),
            lambda s: s["industry"]["sources"][0].__setitem__("progress", 80),
            lambda s: s["industry"]["trips"][0].__setitem__("sequence", 201),
        ):
            bad = copy.deepcopy(scene)
            mutate(bad)
            with self.assertRaises(AssertionError):
                runner.check_industry(bad)

    def test_recipe_uses_native_clay_and_actual_hemp_instead_of_auxiliary_mirrors(self):
        scene = self.auxiliary_scene()
        bad = copy.deepcopy(scene)
        bad["industry"]["sources"][0]["goods_inputs_used"][0] = 2
        with self.assertRaises(AssertionError):
            runner.check_industry(bad)
        bad = copy.deepcopy(scene)
        bad["industry"]["sources"][0]["aux_inputs_used"][0] = 2
        with self.assertRaises(AssertionError):
            runner.check_industry(bad)
        bad = copy.deepcopy(scene)
        bad["industry"]["sources"][0]["clay_used"] = 1
        with self.assertRaises(AssertionError):
            runner.check_industry(bad)

    def test_managed_ore_and_paid_crop_age_cannot_be_backdated(self):
        scene = self.auxiliary_scene()
        bad = copy.deepcopy(scene)
        bad["tick"] = 159
        with self.assertRaises(AssertionError):
            runner.check_industry(bad)
        bad = copy.deepcopy(scene)
        bad["industry"]["managed_ore"] = [{"id": 1, "stock": 1, "progress": 0, "extracted": 1, "used": 0}]
        with self.assertRaises(AssertionError):
            runner.check_industry(bad)

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
        rows = [[0] * 24 for _ in range(15)]
        for row in rows:
            row[0], row[1], row[17] = -99, -10, 7
        rows[8][17] = 0
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
                    "service_witnesses": [None] * 9, "water_evaluation_witness": None, "food_quality_witness": None,
                    "food_received": [0, 0], "food_consumed": [0, 0], "goods_consumed_before": [0] * 6}
                histories.append({"id": i + 1, "first_admission_tick": 120, "last_evaluation": evaluation,
                    "first_hut_evaluation": None, "first_plain_evaluation": None,
                    "consumption_rows": [{"events": events, "consumed": consumed.copy()},
                        {"events": 0, "consumed": [0] * 6}, {"events": 0, "consumed": [0] * 6}]})
            for audit in histories:
                admission = {"tick": 120, "input": {"type": 2, "occupants": 0, "desirability": 0,
                    "devolve_counter": 0, "water": False, "service_timers": [0] * 9, "food_quality": 0,
                    "goods": [0] * 6}, "service_witnesses": [None] * 9, "water_evaluation_witness": None,
                    "food_quality_witness": None, "food_received": [0, 0], "food_consumed": [0, 0],
                    "goods_consumed_before": [0] * 6}
                audit["first_transitions"] = [None] * 15
                audit["first_transitions"][0] = {"kind": 0, "admitted": 2, "evaluation": admission}
                audit["consumption_rows"] += [{"events": 0, "consumed": [0] * 6} for _ in range(12)]
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
        rows = [[0] * 24 for _ in range(15)]
        for i, row in enumerate(rows):
            row[0], row[1], row[17] = -99, -5 if i < 2 else 99, 7 + 7 * i
        rows[8][17] = 0
        rows[1][3] = rows[2][3] = rows[2][21] = 1
        rows[2][8] = 20
        i = {"type": 3, "occupants": 4, "devolve_counter": 0, "water": True, "desirability": 0,
             "service_timers": [0] * 9, "food_quality": 0, "goods": [0] * 6}
        e = {"tick": 193, "input": i, "service_witnesses": [None] * 9,
             "water_evaluation_witness": {"house_id": 1, "provider_id": 2, "courier_id": 1,
                                          "sequence": 1, "arrival_tick": 150},
             "food_quality_witness": None, "food_received": [0, 0], "food_consumed": [0, 0],
             "goods_consumed_before": [0] * 6}
        scene = {"tick": 193, "capacity": 14,
                "houses": [{"id": 1, "type": 4, "occupants": 4, "devolve_counter": 0, "consumed": [0] * 6}],
                "buildings": [{"id": 1, "kind": 6, "cell": [0, 2], "placed_tick": 0}],
                "housing": {"model_rows": rows, "evaluation_tick": 193,
                    "consumption_events": runner.housing_consumption_events(193), "neutral_source_origins": [],
                    "houses": [{"id": 1, "first_admission_tick": 120,
                        "consumption_rows": [{"events": 0, "consumed": [0] * 6} for _ in range(15)],
                        "last_evaluation": e, "first_hut_evaluation": copy.deepcopy(e), "first_plain_evaluation": None}]}}

        audit = scene["housing"]["houses"][0]
        admission = copy.deepcopy(e)
        admission.update(tick=120, water_evaluation_witness=None)
        admission["input"].update(type=2, occupants=0, water=False)
        audit["first_transitions"] = [None] * 15
        audit["first_transitions"][0] = {"kind": 0, "admitted": 2, "evaluation": admission}
        audit["first_transitions"][1] = {"kind": 1, "admitted": 0, "evaluation": copy.deepcopy(e)}
        return scene

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


    @staticmethod
    def religion_scene():
        # Synthetic report-parser data, not evidence of a running paid World.
        scene = ProductionRunnerTests.ancestor_scene()
        next(b for b in scene["buildings"] if b["id"] == 1)["kind"] = 40
        scene["houses"][0]["type"] = 13
        for index, (kind, model, cost, workers, actor_role, timer_role) in enumerate((
                (21, 215, 45, 4, 17, 7), (22, 217, 45, 4, 18, 8), (23, 219, 80, 12, 19, 6))):
            owner, actor = 5 + index, 4 + index
            scene["buildings"].append({"id": owner, "kind": kind, "placed_tick": 0, "operating": False})
            scene["couriers"].append({"id": actor, "role": actor_role, "owner": owner, "target": 1,
                "last_dispatched_target": 1, "phase": 2, "cargo": 0, "reserved": 0,
                "typed_cargo": 0, "typed_cargo_good": None, "food_cargo": 0})
            scene["services"]["providers"].append({"id": owner, "type": 3 + index,
                "model_type": model, "cost": cost, "workers_required": workers})
            scene["services"]["trips"].append({"courier_id": actor, "sequence": 1, "target": 1,
                "dispatched_tick": 200, "arrival_tick": 270 + index, "visit_applied": True})
            scene["services"]["witnesses"].append({"house_id": 1, "role": timer_role, "provider_id": owner,
                "courier_id": actor, "sequence": 1, "arrival_tick": 270 + index})
            scene["houses"][0]["service_timers"][timer_role] = 39
        return scene

    def test_religion_distinct_models_workers_actors_and_sole_timer_provenance(self):
        scene = self.religion_scene()
        runner.check_services(scene, religions=True)
        mutations = (
            lambda row: row["services"]["providers"][3].update(type=4),
            lambda row: row["services"]["providers"][4].update(model_type=215),
            lambda row: row["services"]["providers"][5].update(workers_required=4),
            lambda row: row["services"]["providers"][5].update(cost=45),
            lambda row: row["couriers"][3].update(role=18),
            lambda row: row["couriers"][4].update(cargo=1),
            lambda row: row["couriers"][5].update(food_cargo=1),
            lambda row: row["services"]["witnesses"][3].update(role=8),
            lambda row: row["services"]["witnesses"][5].update(provider_id=5),
            lambda row: row["services"]["witnesses"][4].update(sequence=2),
            lambda row: row["services"]["trips"][3].update(visit_applied=False),
            lambda row: row["houses"][0]["service_timers"].__setitem__(7, 95),
            lambda row: row["houses"][0]["service_timers"].__setitem__(6, 40),
        )
        for index, mutation in enumerate(mutations):
            wrong = copy.deepcopy(scene)
            mutation(wrong)
            with self.subTest(mutation=index), self.assertRaises(AssertionError):
                runner.check_services(wrong, religions=True)

    def test_all_three_religions_use40_countdowns_and_preserve_medical96(self):
        for step in range(16):
            for offset in (0, 1):
                scene = self.religion_scene()
                at = 816 + step * 51 + 36 + offset
                scene["tick"] = at
                scene["services"]["countdowns"] = runner.service_countdowns(at)
                for w in scene["services"]["witnesses"]:
                    if w["role"] in runner.RELIGION_ROLES:
                        w["arrival_tick"] = at
                        next(t for t in scene["services"]["trips"] if t["courier_id"] == w["courier_id"]).update(
                            dispatched_tick=at - 50, arrival_tick=at)
                    duration = 40 if w["role"] >= 5 else 96
                    scene["houses"][0]["service_timers"][w["role"]] = max(duration - (
                        runner.service_countdowns(at) - runner.service_countdowns(w["arrival_tick"] - 1)), 0)
                runner.check_services(scene, religions=True)
                for role in runner.RELIGION_ROLES:
                    self.assertEqual(scene["houses"][0]["service_timers"][role], 39 if not offset else 40)

    def test_authored_religion_need_uses_independent_confucian_and_dao_or_buddhist(self):
        scene = self.housing_scene()
        row = scene["housing"]["model_rows"][2]
        state = copy.deepcopy(scene["housing"]["houses"][0]["last_evaluation"]["input"])
        state.update(food_quality=20)
        state["service_timers"][5] = 40
        row[22] = row[23] = 1
        self.assertFalse(runner.housing_needs(row, state))
        state["service_timers"][6] = 40
        self.assertFalse(runner.housing_needs(row, state))
        for single, absent in ((7, 8), (8, 7)):
            state["service_timers"][single], state["service_timers"][absent] = 40, 0
            self.assertTrue(runner.housing_needs(row, state))
            state["service_timers"][6] = 0
            self.assertFalse(runner.housing_needs(row, state))
            state["service_timers"][6] = 40
        state["service_timers"][7] = state["service_timers"][8] = 0
        self.assertFalse(runner.housing_needs(row, state))

    @staticmethod
    def culture_scene():
        # Synthetic parser records only; actual paid sources are exercised by C++.
        scene = ProductionRunnerTests.religion_scene()
        for index, role in enumerate(runner.CULTURE_ROLES):
            owner, actor = 8 + index, 7 + index
            scene["buildings"].append({"id": owner, "kind": 24 + index, "placed_tick": 0, "operating": False})
            scene["couriers"].append({"id": actor, "role": 20 + index, "owner": owner, "target": 1,
                "last_dispatched_target": 1, "phase": 2, "cargo": 0, "reserved": 0,
                "typed_cargo": 0, "typed_cargo_good": None, "food_cargo": 0})
            scene["services"]["providers"].append({"id": owner, "type": 6 + index,
                "model_type": 211 + index, "cost": 50 + 10 * index, "workers_required": 8 + index})
            scene["services"]["trips"].append({"courier_id": actor, "sequence": 1, "target": 1,
                "dispatched_tick": 200, "arrival_tick": 280 + index, "visit_applied": True})
            scene["services"]["witnesses"].append({"house_id": 1, "role": role, "provider_id": owner,
                "courier_id": actor, "sequence": 1, "arrival_tick": 280 + index})
            scene["houses"][0]["service_timers"][role] = 95
        return scene

    def test_culture_separate96_source_category_actor_and_paid_model_bindings(self):
        scene = self.culture_scene()
        runner.check_services(scene, culture=True)
        for mutation in (
                lambda row: row["services"]["providers"][6].update(model_type=212),
                lambda row: row["services"]["providers"][7].update(workers_required=8),
                lambda row: row["services"]["providers"][8].update(cost=0),
                lambda row: row["couriers"][6].update(role=21),
                lambda row: row["couriers"][8].update(cargo=1),
                lambda row: row["services"]["witnesses"][7].update(role=4),
                lambda row: row["services"]["trips"][6].update(visit_applied=False),
                lambda row: row["houses"][0]["service_timers"].__setitem__(2, 39),
                lambda row: row["houses"][0]["service_timers"].__setitem__(3, 96)):
            bad = copy.deepcopy(scene)
            mutation(bad)
            with self.subTest(mutation=mutation), self.assertRaises(AssertionError):
                runner.check_services(bad, culture=True)
        for role in runner.CULTURE_ROLES:
            for step in range(16):
                for offset, expected in ((0, 95), (1, 96)):
                    aligned = self.culture_scene()
                    at = 816 + 51 * step + 36 + offset
                    aligned["tick"] = at
                    aligned["services"]["countdowns"] = runner.service_countdowns(at)
                    for w in aligned["services"]["witnesses"]:
                        if w["role"] == role:
                            w["arrival_tick"] = at
                            next(t for t in aligned["services"]["trips"] if t["courier_id"] == w["courier_id"]).update(
                                dispatched_tick=at - 50, arrival_tick=at)
                        duration = 96 if w["role"] < 5 else 40
                        aligned["houses"][0]["service_timers"][w["role"]] = max(duration - (
                            runner.service_countdowns(at) - runner.service_countdowns(w["arrival_tick"] - 1)), 0)
                    runner.check_services(aligned)
                    self.assertEqual(aligned["houses"][0]["service_timers"][role], expected)

    def test_historical_culture_slots_append_without_reindexing_six_prior_categories(self):
        scene = self.housing_scene()
        row = scene["housing"]["houses"][0]
        for key in ("last_evaluation", "first_hut_evaluation"):
            e = row[key]
            for index, role in enumerate((0, 1, 5, 6, 7, 8, 2, 3, 4)):
                if role == 6:
                    continue
                e["service_witnesses"][index] = {"house_id": 1, "role": role, "provider_id": role + 10,
                    "courier_id": role + 10, "sequence": 1, "arrival_tick": 180}
                e["input"]["service_timers"][role] = 95 if role < 5 else 39
        row["first_transitions"][1]["evaluation"] = copy.deepcopy(row["first_hut_evaluation"])
        runner.check_housing(scene)
        for left, right in ((6, 7), (7, 8), (2, 6)):
            bad = copy.deepcopy(scene)
            slots = bad["housing"]["houses"][0]["last_evaluation"]["service_witnesses"]
            slots[left], slots[right] = slots[right], slots[left]
            with self.subTest(swapped=(left, right)), self.assertRaises(AssertionError):
                runner.check_housing(bad)

    def test_availability_capacity_travel_and_received_access_are_independent(self):
        scene = self.culture_scene()
        for b in scene["buildings"]:
            b.update(on_fire=False, workers=0)
        visits = {w["role"]: w for w in scene["services"]["witnesses"]}
        scene["service_availability"] = [{"house_id": 1, "role": role, "eligible_target": True,
            "delivered_timer": scene["houses"][0]["service_timers"][role], "provider_count": 1,
            "available_source_count": 0, "reachable_source_count": 0, "travelling_to_target_count": 0,
            "last_arrival_tick": visits[role]["arrival_tick"]} for role in range(9)]
        runner.check_availability(scene)  # paused sources retain actual prior access
        active = scene["buildings"][7]
        self.assertEqual(active["id"], 8)
        active.update(operating=True, workers=8)
        scene["services"]["witnesses"] = [w for w in scene["services"]["witnesses"] if w["role"] != 2]
        scene["services"]["trips"][6].update(arrival_tick=None, visit_applied=False)
        scene["couriers"][6].update(phase=1)
        scene["houses"][0]["service_timers"][2] = 0
        diagnosis = scene["service_availability"][2]
        diagnosis.update(delivered_timer=0, last_arrival_tick=None, available_source_count=1,
                         reachable_source_count=1, travelling_to_target_count=1)
        runner.check_services(scene)
        runner.check_availability(scene)  # available+reachable+inflight still is not received
        active.update(operating=False, workers=0)
        diagnosis.update(available_source_count=0, reachable_source_count=0)
        runner.check_availability(scene)  # begun tour survives source pause
        for mutation in (
                lambda row: row["service_availability"][2].update(delivered_timer=96),
                lambda row: row["service_availability"][2].update(available_source_count=1),
                lambda row: row["service_availability"][2].update(reachable_source_count=1),
                lambda row: row["service_availability"][2].update(travelling_to_target_count=0),
                lambda row: row["service_availability"].pop()):
            bad = copy.deepcopy(scene)
            mutation(bad)
            with self.subTest(mutation=mutation), self.assertRaises(AssertionError):
                runner.check_availability(bad)



    def test_pending_elite_attempt_cannot_hide_admission_resources_or_preparation(self):
        scene = {"buildings": [{"id": 1, "kind": 39, "footprint": [2, 2]}],
                 "houses": [{"id": 1, "type": 11, "occupants": 0, "status": 0, "access_enabled": False,
                    "water": False, "food_quality": 0, "service_timers": [0]*9, "consumed": [0]*6,
                    **{key: 0 for key in ("hemp", "ceramics", "tea", "silk", "bronzeware", "lacquerware")}}],
                 "housing": {"houses": [{"id": 1, "first_admission_tick": None, "last_evaluation": None,
                    "first_transitions": [None]*15,
                    "consumption_rows": [{"events": 0, "consumed": [0]*6} for _ in range(15)]}]}}
        runner.check_pending_elite(scene, 1)
        for mutation in (
                lambda value: value["buildings"][0].update(kind=40),
                lambda value: value["buildings"][0].update(footprint=[4, 4]),
                lambda value: value["houses"][0].update(type=13),
                lambda value: value["houses"][0].update(occupants=2),
                lambda value: value["houses"][0].update(access_enabled=True),
                lambda value: value["houses"][0].update(water=True),
                lambda value: value["houses"][0].update(hemp=1),
                lambda value: value["houses"][0]["service_timers"].__setitem__(6, 40),
                lambda value: value["housing"]["houses"][0].update(first_admission_tick=120),
                lambda value: value["housing"]["houses"][0].update(last_evaluation={}),
                lambda value: value["housing"]["houses"][0]["first_transitions"].__setitem__(10, {"kind": 0}),
                lambda value: value["housing"]["houses"][0]["consumption_rows"][0].update(events=1)):
            wrong = copy.deepcopy(scene)
            mutation(wrong)
            with self.subTest(mutation=mutation), self.assertRaises(AssertionError):
                runner.check_pending_elite(wrong, 1)

    def test_actual_footprints_rebuild_all16_claims_without_overlap_or_shrink(self):
        scene = {"geometry": {"width": 12, "height": 10, "owners": [], "roads": [[4, 0]]},
                 "road_count": 1, "buildings": [{"id": 1, "kind": 40, "cell": [0, 0], "footprint": [4, 4]}],
                 "houses": [{"id": 1, "type": 13}]}
        scene["geometry"]["owners"] = [[x, y, 1, 40] for y in range(4) for x in range(4)]
        runner.check_geometry(scene)
        abandoned = copy.deepcopy(scene)
        abandoned["houses"][0]["type"] = 12
        runner.check_geometry(abandoned)
        for mutation in (
                lambda value: value["geometry"]["owners"].pop(),
                lambda value: value["geometry"]["owners"].append([0, 0, 1, 40]),
                lambda value: value["geometry"]["owners"][0].__setitem__(2, 2),
                lambda value: value["buildings"][0].update(footprint=[2, 2]),
                lambda value: value["buildings"][0].update(kind=39),
                lambda value: value["houses"][0].update(type=11),
                lambda value: value["geometry"]["roads"].__setitem__(0, [3, 3]),
                lambda value: value["buildings"][0].update(cell=[9, 0])):
            wrong = copy.deepcopy(scene)
            mutation(wrong)
            with self.subTest(mutation=mutation), self.assertRaises(AssertionError):
                runner.check_geometry(wrong)

    def test_original_15_row_domains_and_admission_records_are_strict(self):
        scene = self.housing_scene()
        self.assertEqual(len(scene["housing"]["model_rows"]), 15)
        runner.check_housing(scene)
        mutations = (
            lambda s: s["housing"]["model_rows"][8].__setitem__(17, 1),
            lambda s: s["housing"]["houses"][0]["first_transitions"].pop(),
            lambda s: s["housing"]["houses"][0]["first_transitions"][0].update(kind=1),
            lambda s: s["housing"]["houses"][0]["first_transitions"][0].update(kind=False),
            lambda s: s["housing"]["houses"][0]["first_transitions"][0].update(admitted=1),
            lambda s: s["housing"]["houses"][0]["first_transitions"][0].update(admitted=3),
            lambda s: s["housing"]["houses"][0]["first_transitions"][1].update(admitted=1),
            lambda s: s["buildings"][0].update(kind=39),
        )
        for mutation in mutations:
            wrong = copy.deepcopy(scene)
            mutation(wrong)
            with self.subTest(mutation=mutation), self.assertRaises(AssertionError):
                runner.check_housing(wrong)

    def test_confucian_actual_visit_requires_elite_family(self):
        scene = self.religion_scene()
        runner.check_services(scene, religions=True)
        wrong = copy.deepcopy(scene)
        next(b for b in wrong["buildings"] if b["id"] == 1)["kind"] = 6
        wrong["houses"][0]["type"] = 3
        with self.assertRaises(AssertionError):
            runner.check_services(wrong, religions=True)

    def test_pure_reference_elite_inert_terminal_and_literal_counter_limits(self):
        scene = self.housing_scene()
        rows = scene["housing"]["model_rows"]
        for row in rows:
            row[3] = 1
        state = copy.deepcopy(scene["housing"]["houses"][0]["last_evaluation"]["input"])
        state.update(type=11, water=False, devolve_counter=0)
        self.assertEqual(runner.housing_step(rows, state), (11, 0))
        for kind, limit in ((3, 9), (5, 9), (10, 9), (12, 11), (13, 49), (14, 17), (17, 17)):
            state.update(type=kind, water=False, devolve_counter=limit - 1)
            self.assertEqual(runner.housing_step(rows, state), (kind, limit))
            state["devolve_counter"] = limit
            self.assertEqual(runner.housing_step(rows, state), (kind if kind in (3, 12) else kind - 1, 0))


if __name__ == "__main__":
    unittest.main()
