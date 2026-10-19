"""Independent finance reader: hostile scalar/JSON/model-source boundaries."""
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

SPEC = importlib.util.spec_from_file_location("finance_runner", Path(__file__).resolve().parents[1]/"tools/test_original_finance.py")
runner = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(runner)


class FinanceReaderTests(unittest.TestCase):
    def test_unsigned_full_domain_is_exact(self):
        self.assertEqual(runner.number((1 << 64)-1), (1 << 64)-1)
        for value in (-1, 1 << 64, True, False, 1.0, "1", None):
            with self.subTest(value=value), self.assertRaises(AssertionError):
                runner.number(value)

    def test_signed_full_domain_is_exact(self):
        for value in (-(1 << 63), -1, 0, (1 << 63)-1):
            self.assertEqual(runner.number(value, -(1 << 63), (1 << 63)-1), value)
        for value in (-(1 << 63)-1, 1 << 63):
            with self.assertRaises(AssertionError):
                runner.number(value, -(1 << 63), (1 << 63)-1)

    def test_duplicate_json_is_refused_at_any_depth(self):
        for text in ('{"taxes":0,"taxes":1}', '{"finance":{"events":0,"events":1}}'):
            with self.assertRaises(AssertionError):
                json.loads(text, object_pairs_hook=runner.unique)

    @staticmethod
    def model():
        def row(values):
            return "x,{," + ",".join(map(str, values)) + ",},\n"
        houses = [[0]*24 for _ in range(15)]
        houses[0][18] = 99; houses[1][18] = -99
        mods = [[0]*24 for _ in range(5)]
        mods[0][18] = 2; mods[4][18] = -2
        building = [0]*13; building[0] = 13; building[1] = -13
        bmods = [[100]*13 for _ in range(5)]
        bmods[0] = [150]*13
        return ("HOUSE MODS\n"+"".join(row(v) for v in mods)+"ALL HOUSES\n"+"".join(row(v) for v in houses)+
                "BUILDING MODS\n"+"".join(row(v) for v in bmods)+"ALL BUILDINGS\n"+row(building).replace("x,", "193,", 1))

    def read(self, text, difficulty=0):
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary)/"model.txt"
            path.write_text(text, encoding="ascii")
            return runner.prepared_model(path, difficulty)

    def test_source_house_modifiers_clamp_signed_values(self):
        easy, _ = self.read(self.model(), 0)
        hard, _ = self.read(self.model(), 4)
        self.assertEqual((easy[0][18], easy[1][18]), (100, -97))
        self.assertEqual((hard[0][18], hard[1][18]), (97, -99))

    def test_source_building_percentage_truncates_toward_zero(self):
        _, models = self.read(self.model())
        self.assertEqual(models[193][:2], [19, -19])

    def test_missing_rows_and_noninteger_model_are_refused(self):
        for text in (self.model().replace("ALL HOUSES", "MISSING HOUSES"),
                     self.model().replace("193,{,13,", "193,{,13.5,"),
                     self.model().replace("193,{,13,", "193,{,2147483648,")):
            with self.assertRaises(AssertionError):
                self.read(text)

    def test_difficulty_rejects_bool_and_outside_typed_domain(self):
        for selected in (True, -1, 5, "2"):
            with self.assertRaises(AssertionError):
                self.read(self.model(), selected)


if __name__ == "__main__":
    unittest.main()
