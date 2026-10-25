#!/usr/bin/env python3
"""OE38 diagnostic and unchanged paid-baseline gates; no high-tier grants."""
import argparse
import json
from pathlib import Path
import subprocess
import sys
import time

import test_sustainable_economy as baseline
import test_paid_progression as proof

PRIVATE = proof.REPO / ".local/original-economy/oe38"


def review_matrix(matrix):
    proof.require(matrix.get("format") == "openemperor-high-tier-matrix-v1" and
                  matrix.get("rule") == 12 and matrix.get("policy") == 3 and
                  matrix.get("model_sha256") == proof.MODEL_SHA and matrix.get("difficulty") == 2,
                  "Matrix lost its exact unchanged Normal Rule12 source identity")
    proof.require(all(type(matrix[k]) is int for k in ("rule", "policy", "difficulty")),
                  "Matrix identity must use exact integer fields")
    proof.require(matrix.get("independent_house_rules_match") is True and
                  matrix.get("query_world_unchanged") is True and
                  matrix.get("snapshot_injection") is False and
                  matrix.get("high_tier_success_claim") is False,
                  "A diagnostic table cannot grant or claim an actual high-tier event")
    rows = matrix.get("types16")
    proof.require(isinstance(rows, list) and len(rows) == 16 and
                  [r.get("type") for r in rows] == list(range(2, 18)),
                  "Missing, duplicate or reordered House type in source matrix")
    homes = matrix.get("houses")
    proof.require(isinstance(homes, list) and len(homes) >= 2, "Paid Common/Elite observers missing")
    proof.require(all(type(h.get("elite_family")) is bool for h in homes) and
                  {h["elite_family"] for h in homes} == {False, True},
                  "The separate paid Common and Elite families must both be observed")
    proof.require(all(len(h.get("matrix15", [])) == 15 for h in homes), "Incomplete live World decision matrix")


def bind_matrix_world(matrix, world):
    """Only retained raw paid-state events can support an actual tier claim."""
    proof.require(matrix["tick"] == world["ticks"] and matrix["funds"] == world["treasury"] and
                  matrix["construction"] == world["construction_spent_total"],
                  "Matrix observation clock or ledger differs from complete saved World")
    homes = world["original_economy"]["houses"]
    bodies = {b["id"]: b for b in world["buildings"]}
    histories = {h["building_id"]: h for h in world["original_production"]["housing"]["houses"]}
    proof.require([h["id"] for h in matrix["houses"]] == [h["building_id"] for h in homes],
                  "Matrix paid observer identities differ from complete saved World")
    for observed, home in zip(matrix["houses"], homes):
        ident, actual = home["building_id"], home["house"]
        history = histories[ident]
        elite = actual["type"] >= 11
        side = 4 if elite and history["first_admission_tick"] is not None else 2
        proof.require(observed["actual_post_tick_state"] == actual and observed["cell"] == bodies[ident]["cell"] and
                      observed["elite_family"] is elite and observed["actual_footprint_side"] == side,
                      "Matrix actual House state/family/geometry differs from saved authority")
        expected = []
        for index, entry in enumerate(history["first_transitions"]):
            if entry is not None:
                expected.append({"type": index + 3, "tick": entry["evaluation"]["tick"], "kind": entry["kind"],
                                 "admitted": entry["admitted"], "evaluation_input": entry["evaluation"]["input"]})
        proof.require(observed["actual_first_transitions"] == expected,
                      "Matrix invented, omitted or changed an actual House transition")
    for row in matrix["types16"]:
        kind, expected = row["type"], []
        for home in homes:
            ident = home["building_id"]
            if kind in (2, 11):
                if home["house"]["type"] == kind:
                    expected.append({"house": ident, "kind": "paid empty plot, no occupied-tier proof"})
            else:
                entry = histories[ident]["first_transitions"][kind - 3]
                if entry is not None:
                    expected.append({"house": ident, "tick": entry["evaluation"]["tick"], "kind": entry["kind"]})
        proof.require(row["actual_witnesses"] == expected and
                      row["actual_occupied_tier_witnessed"] is (bool(expected) and kind not in (2, 11)),
                      "Matrix occupied-tier claim lacks exact saved admission/evolution witnesses")


def matrix_workflow(binary, data, output, input_path=None, timeout=180):
    for p in (binary, data, output, PRIVATE):
        proof.safe(p)
    proof.require(binary.is_file() and data.is_dir(), "Executable/original installation missing")
    proof.require(output != PRIVATE and PRIVATE in output.parents and not output.exists(),
                  "A new private OE38 output is required")
    proof.require(not any(proof.overlaps(output, p) for p in (data, binary.parent)),
                  "Output overlaps protected original/build input")
    if input_path is not None:
        proof.safe(input_path)
        proof.require(PRIVATE in input_path.parents and input_path.is_file() and
                      not proof.overlaps(input_path, output), "Explicit earned OE38 input required")
    before, sources, executable = proof.inventory(data), proof.sources(), proof.record(binary)
    seed = proof.record(input_path) if input_path is not None else None
    output.mkdir(parents=True)
    command = [str(binary), "--mode", "matrix", "--data", str(data), "--output", str(output / "matrix")]
    if input_path is not None:
        command += ["--input", str(input_path)]
    report = {"stage": "A-matrix", "status": "FAIL", "high_tier_success_claim": False,
              "command": command, "sources": sources, "executable": executable}
    started = time.monotonic()
    try:
        with (output / "process.log").open("w") as log:
            result = subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, timeout=timeout, check=False)
        proof.require(result.returncode == 0, "Matrix process failed; inspect process.log")
        matrix = proof.load(output / "matrix/matrix.json")
        review_matrix(matrix)
        world = baseline.review_save(output / "matrix/matrix-world.save.json", 12, data)
        bind_matrix_world(matrix, world)
        report.update(status="PASS", matrix=matrix, complete_saved_world_validated=True)
    except Exception as error:
        report.update(error=str(error))
        raise
    finally:
        report.update(elapsed_seconds=time.monotonic() - started,
                      original_bytes_modes_mtimes_unchanged=before == proof.inventory(data),
                      sources_unchanged=sources == proof.sources(), executable_unchanged=executable == proof.record(binary),
                      earned_input_unchanged=input_path is None or seed == proof.record(input_path))
        if not all(report[k] for k in ("original_bytes_modes_mtimes_unchanged", "sources_unchanged",
                                      "executable_unchanged", "earned_input_unchanged")):
            report.update(status="FAIL", error="Protected input/source identity changed")
        (output / "matrix-acceptance.json").write_text(json.dumps(report, indent=2) + "\n")
    proof.require(report["status"] == "PASS", report.get("error", "Matrix gate failed"))
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--stage", choices=("matrix", "baseline"), required=True)
    for key in ("binary", "data", "output"):
        parser.add_argument("--" + key, type=Path, required=True)
    parser.add_argument("--resources", type=Path)
    parser.add_argument("--input", type=Path)
    parser.add_argument("--timeout", type=int, default=1800)
    args = parser.parse_args()
    try:
        binary, data, output = (getattr(args, k).absolute() for k in ("binary", "data", "output"))
        if args.stage == "baseline":
            proof.require(args.resources is not None and args.input is None, "Baseline requires resources and no imported state")
            baseline.PRIVATE = PRIVATE
            result = baseline.workflow(binary, data, output, args.resources.absolute(), 12, "balanced-three", 100000, args.timeout, True)
            proof.require(all(result[k] == "PASS" for k in ("status", "sustainability", "goods_supply", "fire_protection")),
                          "OE37 baseline regression did not meet every existing gate")
        else:
            result = matrix_workflow(binary, data, output, args.input.absolute() if args.input else None, args.timeout)
        print("OE38 Stage A " + args.stage + " " + result["status"] + "; high-tier gameplay remains unproven")
        return 0
    except Exception as error:
        print("OE38 FAIL: " + str(error), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
