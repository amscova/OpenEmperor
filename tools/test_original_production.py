#!/usr/bin/env python3
"""Verify paid Rule10 production and actual OS-process replay without changing original data."""
import argparse
import importlib.util
import json
from pathlib import Path
import shutil
import tempfile

_SPEC = importlib.util.spec_from_file_location(
    "original_goods_runner", Path(__file__).with_name("test_original_goods.py"))
legacy = importlib.util.module_from_spec(_SPEC)
_SPEC.loader.exec_module(legacy)
MODEL_SHA = legacy.MODEL_SHA
PRIVATE_OUTPUT = Path(".local/original-economy/oe15/production-fixtures")
RULE9_REFERENCE = Path(".local/original-economy/oe13/goods-fixtures/run-d4mm76cf")
RULE9_REPORT_SHA = "1b5f95d6b1de09558be7f4a5859b1fa953aaa79da089f831afece692760678f2"
RULE9_REVIEW_SHA = "c5534befba97a9ad1469bec05131d21f17fe509db101c0f4bd1e0351e742a017"
digest = legacy.digest
inventory = legacy.inventory


def preflight_output(repo, requested, original, build):
    """Reject existing/linked/escaping targets before any output or model copy."""
    repo = repo.resolve(strict=True)
    private = repo / PRIVATE_OUTPUT
    legacy.check_output_path(repo, private)
    if private.exists() and not private.is_dir():
        raise RuntimeError("Private fixture root is not a directory")
    for source in (original.resolve(strict=True), build.resolve(strict=True)):
        if legacy.overlaps(private, source):
            raise RuntimeError("Private fixtures must not overlap original data or build directory")
    if requested is None:
        return private, None
    output = requested.absolute()
    if ".." in output.parts or output == private or not output.is_relative_to(private):
        raise RuntimeError(f"Output must be a new subdirectory of {private}")
    legacy.check_output_path(repo, output)
    if output.exists():
        raise RuntimeError(f"Output already exists: {output}")
    return private, output


def create_output(repo, requested, original, build):
    private, output = preflight_output(repo, requested, original, build)
    if output is None:
        private.mkdir(parents=True, exist_ok=True)
        preflight_output(repo, None, original, build)
        output = Path(tempfile.mkdtemp(prefix="run-", dir=private))
    else:
        output.mkdir(parents=True, exist_ok=False)
    legacy.check_output_path(repo.resolve(strict=True), output)
    return output


def file_identity(path):
    status = path.stat()
    return {"bytes": status.st_size, "mode": status.st_mode,
            "mtime_ns": status.st_mtime_ns, "sha256": digest(path)}


def check_balances(scene, positive=False):
    balances = scene["balances"]
    assert len(balances) == 6
    for index, balance in enumerate(balances):
        assert balance["good"] == index and balance["conserved"] is True
        assert balance["supported"] is (index == 1)
        assert all(type(balance[field]) is int and balance[field] >= 0 for field in (
            "produced", "imported", "consumed", "demolished", "stock", "cargo",
            "source_reserved", "incoming_reserved"))
        assert all(balance[field] == 0 for field in ("imported", "consumed", "demolished"))
        assert balance["stock"] + balance["cargo"] == balance["produced"]
        if index != 1:
            assert all(balance[field] == 0 for field in ("stock", "cargo", "source_reserved", "incoming_reserved"))
    assert balances[1]["produced"] == scene["pottery_handed_off"] <= scene["pottery_completed"]
    assert scene["residents_admitted"] == scene["population"]
    assert sum(house["occupants"] for house in scene["houses"]) == scene["population"]
    assert all(type(house["ceramics"]) is int and 0 <= house["ceramics"] <= 8 for house in scene["houses"])
    assert all(stock["kind"] in (3, 9) and len(stock["amounts"]) == 6 for stock in scene["stocks"])
    for stock in scene["stocks"]:
        assert all(type(amount) is int and amount >= 0 for amount in stock["amounts"])
        assert all(amount == 0 for good, amount in enumerate(stock["amounts"]) if good != 1)
    assert balances[1]["stock"] == sum(house["ceramics"] for house in scene["houses"]) + sum(
        stock["amounts"][1] for stock in scene["stocks"])
    assert balances[1]["cargo"] == sum(courier["typed_cargo"] for courier in scene["couriers"])
    for courier in scene["couriers"]:
        if courier["role"] in (6, 8):
            assert courier["cargo"] == courier["reserved"] == 0
        else:
            assert courier["typed_cargo"] == 0
    if positive:
        assert balances[1]["stock"] > 0 and scene["population"] > 0
        assert scene["workforce_supply"] >= 12


def checked_path(repo, root, path):
    path = Path(path)
    if not path.is_absolute() or not path.is_relative_to(root) or ".." in path.parts:
        raise RuntimeError("Fixture record escaped its private output root")
    legacy.check_output_path(repo, path)
    if not path.is_file():
        raise RuntimeError(f"Fixture output is missing: {path}")
    return path


def frozen_rule9(repo):
    root = repo / RULE9_REFERENCE
    legacy.check_output_path(repo, root)
    report = root / "report.json"
    review = root / "review-validation.json"
    if digest(report) != RULE9_REPORT_SHA or digest(review) != RULE9_REVIEW_SHA:
        raise RuntimeError("Independently reviewed pre-OE14 Rule9 reference changed")
    records = json.loads(review.read_text())
    if records["status"] != "PASS" or records["report_sha256"] != RULE9_REPORT_SHA:
        raise RuntimeError("Rule9 reference has no matching completed review")
    for record in records["all_files"]:
        relative = Path(record["path"])
        if relative.is_absolute() or ".." in relative.parts:
            raise RuntimeError("Rule9 reference manifest escaped its root")
        path = checked_path(repo, root, root / relative)
        if digest(path) != record["sha256"] or path.stat().st_size != record["bytes"]:
            raise RuntimeError(f"Frozen Rule9 reference bytes changed: {path}")
    return root


def compare_rule9(repo, reference, binary, stage, output):
    current = output / "legacy-rule9"
    legacy.run(binary, ["--create", stage, current], output / "legacy-rule9.log")
    compared = []
    for tag in ("removed", "rebought"):
        for suffix in ("", "-midpoint", "-result"):
            name = tag + suffix + ".json"
            assert (current / name).read_bytes() == (reference / "create" / name).read_bytes(), name
            report = tag + suffix + "-report.json"
            assert json.loads((current / report).read_text()) == json.loads(
                (reference / "create" / report).read_text()), report
            compared.append(name)
        resumed = output / ("legacy-rule9-resume-" + tag)
        source = current / (tag + ".json")
        before = file_identity(source)
        legacy.run(binary, ["--resume", stage, resumed, source], output / ("legacy-rule9-resume-" + tag + ".log"))
        for name in ("midpoint.json", "result.json"):
            assert (resumed / name).read_bytes() == (reference / ("resume-" + tag) / name).read_bytes()
            report = name.replace(".json", "-report.json")
            assert json.loads((resumed / report).read_text()) == json.loads(
                (reference / ("resume-" + tag) / report).read_text())
            compared.append("resume-" + tag + "/" + name)
        assert before == file_identity(source)
    frozen_rule9(repo)
    return {"rule9_schema25_byte_equal": True, "rule9_provenance_equal": True,
            "rule9_save_files": compared, "rule9_reference_report_sha256": RULE9_REPORT_SHA,
            "rule9_reference_review_sha256": RULE9_REVIEW_SHA,
            "current_rule9_binary_sha256": digest(binary)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("data", type=Path, help="Read-only original installation")
    parser.add_argument("--build-dir", type=Path)
    parser.add_argument("--output", type=Path, help=f"New subdirectory of repository/{PRIVATE_OUTPUT} only")
    args = parser.parse_args()
    repo = Path(__file__).resolve().parent.parent
    original = args.data.resolve(strict=True)
    build = (args.build_dir or repo / ".local/reswall/release-build").resolve(strict=True)
    # No preparation/output may precede validation of the caller's target.
    preflight_output(repo, args.output, original, build)
    source_model = original / "Model/EmperorBuildingModels.txt"
    if digest(source_model) != MODEL_SHA:
        raise RuntimeError("Expected pinned original Model SHA; authored models are unit tests only")
    binaries = {name: build / name for name in (
        "openemperor-original-production-fixture", "openemperor-original-goods-fixture",
        "openemperor-original-influence-fixture")}
    for path in binaries.values():
        if not path.is_file():
            raise RuntimeError(f"Build the excluded fixture target first: {path.name}")
    baseline, capture = legacy.frozen_baseline(repo)
    reference9 = frozen_rule9(repo)
    before = inventory(original)
    output = create_output(repo, args.output, original, build)
    stage = output / "data"
    (stage / "Model").mkdir(parents=True)
    (stage / "Cities").mkdir()
    shutil.copy2(source_model, stage / "Model/EmperorBuildingModels.txt")
    (stage / "Cities/Influence.map").write_text(
        "Authored24x20 policy3 influence acceptance geography; not an original map.\n")
    assert digest(stage / "Model/EmperorBuildingModels.txt") == MODEL_SHA
    private_before = inventory(stage)
    binary = binaries["openemperor-original-production-fixture"]
    legacy.run(binary, ["--create", stage, output / "create"], output / "create.log")
    fixture = json.loads((output / "create/fixture.json").read_text())
    assert fixture["status"] == "PASS" and fixture["snapshot_injection"] is False
    assert fixture["positive_runtime_goods"] is True and fixture["authored_geography"] is True
    assert fixture["paid_house_count"] == 4 and fixture["paid_road_count"] == 24
    assert all(fixture[key] == 1 for key in ("paid_clay_count", "paid_pottery_count", "paid_warehouse_count", "paid_market_count"))
    assert fixture["stop_controls_verified"] is True and fixture["natural_fire_verified"] is True
    assert fixture["paused_market_return_verified"] is True and fixture["actual_house_arrival"] is True
    assert fixture["initial"]["population"] == fixture["initial"]["workforce_supply"] == 0
    assert fixture["initial"]["funds"] == 11042 and fixture["initial"]["construction_spent"] == 958
    assert fixture["stopped"]["population"] > 0 and fixture["stopped"]["clay_extracted"] == 0
    for key in ("initial", "stopped", "final", "fire", "paused_return"):
        check_balances(fixture[key], positive=key == "final")
        assert fixture[key]["model_sha256"] == MODEL_SHA
    assert sum(house["ceramics"] for house in fixture["final"]["houses"]) > 0
    restarts = []
    assert [entry["tag"] for entry in fixture["checkpoints"]] == ["mid-market-courier", "house-ceramics"]
    for checkpoint in fixture["checkpoints"]:
        source = checked_path(repo, output, checkpoint["save"])
        source_identity = file_identity(source)
        source_report = json.loads(checked_path(repo, output, checkpoint["report"]).read_text())
        check_balances(source_report)
        if checkpoint["tag"] == "mid-market-courier":
            assert any(c["role"] == 8 and c["typed_cargo"] > 0 and c["cargo"] == 0 and c["edge_progress"] > 0
                       for c in source_report["couriers"])
        else:
            check_balances(source_report, positive=True)
            assert sum(house["ceramics"] for house in source_report["houses"]) > 0
        folder = output / ("resume-" + checkpoint["tag"])
        legacy.run(binary, ["--resume", stage, folder, source], output / (checkpoint["tag"] + "-resume.log"))
        actual_reports = []
        for name, expected_key, report_key in (
                ("midpoint", "expected_midpoint", "expected_midpoint_report"),
                ("result", "expected_result", "expected_report")):
            actual = folder / (name + ".json")
            expected = checked_path(repo, output, checkpoint[expected_key])
            assert actual.read_bytes() == expected.read_bytes(), "Canonical new-process replay bytes differ"
            assert json.loads(actual.read_text()) == json.loads(expected.read_text()), "Full SaveDocument differs"
            report = json.loads((folder / (name + "-report.json")).read_text())
            assert report == json.loads(checked_path(repo, output, checkpoint[report_key]).read_text())
            check_balances(report, positive=True)
            saved = json.loads(actual.read_text())
            assert saved["schema_version"] == 26 and saved["rules"]["version"] == 10
            assert saved["world"]["original_production"]["development_revision"] == 15
            actual_reports.append(report)
        midpoint, final = actual_reports
        assert midpoint["tick"] == source_report["tick"] + 10000
        assert final["tick"] == source_report["tick"] + 20000
        assert final["funds"] == midpoint["funds"] - 2
        assert final["construction_spent"] == midpoint["construction_spent"] + 2
        assert final["command_sequence"] == midpoint["command_sequence"] + 1
        assert final["road_count"] == midpoint["road_count"] + 1
        assert file_identity(source) == source_identity, "Process changed its source save or metadata"
        restarts.append({"checkpoint": checkpoint["tag"], "separate_os_process": True,
            "ticks_advanced": 20000, "periods": [10000, 10000], "actual_paid_road_continuation": True,
            "full_savedocument_equal": True, "canonical_bytes_equal": True, "provenance_equal": True,
            "balances_equal": True, "midpoint_equal": True, "source_save_and_metadata_unchanged": True})
    old = legacy.compare_legacy(baseline, binaries["openemperor-original-goods-fixture"],
        binaries["openemperor-original-influence-fixture"], stage, output)
    old.update(compare_rule9(repo, reference9, binaries["openemperor-original-goods-fixture"], stage, output))
    after = inventory(original)
    assert before == after, "Original installation bytes or metadata changed"
    assert private_before == inventory(stage), "Private hash-identical model copy or map changed"
    legacy.frozen_baseline(repo)
    frozen_rule9(repo)
    report = {"status": "PASS", "evidence": "Paid production; real source model; authored geography; actual arrivals",
        "original_data": str(original), "original_inventory_before": before, "original_inventory_after": after,
        "original_unchanged": True, "private_source_inventory_unchanged": True,
        "original_model_sha256": MODEL_SHA, "model_copy_byte_identical": True,
        "fixture_binary_sha256": digest(binary), "snapshot_injection": False, "positive_runtime_goods": True,
        "fixture": fixture, "restarts": restarts, "legacy": old,
        "frozen_baseline_capture_sha256": digest(baseline / "capture.json"),
        "frozen_baseline_head": capture["head_before_implementation"],
        "limits": ["Actual paid Ceramics Market delivery; no consumption, other goods or house evolution yet",
            "Migration, funding and recipe timing are authored; no original production parity claim",
            "Authored geography; separate SDL/native/human acceptance"]}
    (output / "report.json").write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps({"status": "PASS", "report": str(output / "report.json"),
        "separate_process_replays": len(restarts), "positive_runtime_goods": True, "original_unchanged": True}))


if __name__ == "__main__":
    main()
