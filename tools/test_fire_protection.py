#!/usr/bin/env python3
"""OE36 paid Xia comparison, complete process continuations and production captures."""
import argparse
import json
from pathlib import Path
import subprocess
import sys
import time

import test_paid_progression as proof

REPO = proof.REPO
PRIVATE = REPO / ".local/original-economy/oe36"


def validate_process(report, rule, ticks):
    proof.require(report.get("status") == "PASS" and report.get("rule") == rule and
                  report.get("schema") == (27 if rule == 11 else 26) and
                  report.get("policy") == 3 and report.get("revision") == 27 and
                  report.get("snapshot_injection") is False,
                  "Invalid process authority/status descriptor")
    goal = report.get("first_goal")
    proof.require(isinstance(goal, dict) and goal.get("reached") is True and
                  type(goal.get("tick")) is int and goal["tick"] == 816 and
                  report.get("begin_tick") == 816 and report.get("end_tick") == 816 + ticks,
                  "Missing actual World goal bound to the earned seed tick")
    fire = report.get("fire", {})
    if rule == 11:
        proof.require(type(fire.get("actual_arrivals")) is int and fire["actual_arrivals"] > 0 and
                      type(fire.get("protected_risk_steps")) is int and fire["protected_risk_steps"] > 0,
                      "Missing actual Inspector arrival/protection measurement")
        visits = fire.get("visits")
        proof.require(isinstance(visits, list) and len(visits) == fire["actual_arrivals"] and
                      all(row.get("couriers") and row.get("risk_after") == 0 and
                          row.get("deadline") == row.get("tick", -1) + 2400 for row in visits),
                      "Protection lacks tick-bound courier arrival witnesses")
    else:
        proof.require(fire.get("actual_arrivals") == 0 and fire.get("actual_fire_starts", 0) > 0,
                      "Rule10 fire baseline changed")


def validate_save(path, rule, data):
    d = proof.load(path)
    w = d["world"]
    proof.require(d["schema_version"] == (27 if rule == 11 else 26) and d["rules"]["version"] == rule and
                  d["profiles"]["map_permissions"]["version"] == 3 and
                  w["original_production"]["development_revision"] == 27, "Saved version changed")
    proof.require(d["map"]["relative_path"] == "Cities/Xia.map" and
                  d["map"]["sha256"] == proof.digest(data / "Cities/Xia.map"), "Saved original map identity changed")
    model = w["original_economy"]["model_source"]
    proof.require(model["difficulty"] == 2 and model["sha256"] == proof.MODEL_SHA, "Saved model/difficulty changed")
    proof.require(w["treasury"] + w["construction_spent_total"] + w["maintenance_spent_total"] ==
                  12000 + w["taxes_collected_total"], "Paid funds conservation failed")
    proof.require(len(w["buildings"]) <= 82 and len(w["couriers"]) <= 58, "Collection bounds changed")
    return proof.record(path)


def workflow(binary, data, output, resources, ticks=60000, timeout=1800, render=True):
    for p in (binary, data, output, resources, PRIVATE):
        proof.safe(p)
    proof.require(binary.is_file() and data.is_dir() and resources.is_dir(), "BLOCKED: executable/data/resources missing")
    proof.require(PRIVATE in output.parents and not output.exists(), "Output must be a new private OE36 directory")
    proof.require(not any(proof.overlaps(output, p) for p in (data, binary.parent, resources)), "Output overlaps protected input")
    proof.require(20000 <= ticks <= 60000 and 1 <= timeout <= 7200, "Invalid continuation or timeout bound")
    proof.require(proof.digest(data / "Model/EmperorBuildingModels.txt") == proof.MODEL_SHA, "BLOCKED: unchanged Normal model required")
    before = proof.inventory(data)
    executable = proof.record(binary)
    sources = proof.sources()
    output.mkdir(parents=True)
    report = {"format": "openemperor-fire-protection-workflow-v1", "status": "FAIL", "ticks": ticks,
              "processes": [], "literal_pairs": [], "rules": {}, "native_human": "NOT_RUN"}
    failure = None

    def child(rule, mode, target, input_path=None):
        argv = [str(binary), "--mode", mode, "--rule", str(rule), "--data", str(data),
                "--output", str(target), "--ticks", str(ticks)]
        if input_path:
            argv += ["--input", str(input_path)]
        if mode == "render":
            argv += ["--resources", str(resources)]
        log = output / (str(rule) + "-" + mode + ".log")
        started = time.monotonic()
        with log.open("x") as stream:
            p = subprocess.Popen(argv, stdout=stream, stderr=subprocess.STDOUT)
            timed_out = False
            try:
                code = p.wait(timeout=timeout)
            except subprocess.TimeoutExpired:
                timed_out = True
                p.kill()
                code = p.wait()
        report["processes"].append({"argv": argv, "pid": p.pid, "exit_code": code, "timed_out": timed_out,
                                    "seconds": time.monotonic() - started, "console": proof.record(log)})
        proof.require(not timed_out and code == 0, "Child failed or timed out: " + str(rule) + "/" + mode)
        result = proof.load(target / "report.json")
        if mode != "render":
            validate_process(result, rule, ticks)
        return result

    try:
        for rule in (10, 11):
            created, resumed = output / (str(rule) + "-create"), output / (str(rule) + "-resume")
            result = child(rule, "create", created)
            seed = created / "seed.save.json"
            seed_record = proof.record(seed)
            child(rule, "resume", resumed, seed)
            for name in ("seed.save.json", "plus20000.save.json", "midpoint.save.json", "result.save.json", "metrics.json", "fire.json"):
                a, b = created / name, resumed / name
                proof.require(a.read_bytes() == b.read_bytes(), "Separate process complete bytes differ: " + name)
                report["literal_pairs"].append({"creator": proof.record(a), "resumed": proof.record(b)})
                if name.endswith("save.json"):
                    validate_save(a, rule, data)
            proof.require(proof.record(seed) == seed_record, "Protected seed changed")
            report["rules"][str(rule)] = result
        ten, eleven = report["rules"]["10"], report["rules"]["11"]
        report["comparison"] = {
            "baseline_fire_starts": ten["fire"]["actual_fire_starts"],
            "rule11_fire_starts": eleven["fire"]["actual_fire_starts"],
            "actual_inspector_arrivals": eleven["fire"]["actual_arrivals"],
            "actual_extinguishments": eleven["fire"]["actual_extinguishments"],
            "fire_start_difference_is_paired_observation_not_counted_prevented_events": True,
            "fire_protection_proves_economy_profit": False,
        }
        if render:
            rendered = output / "render"
            input_path = output / "11-create/midflight.save.json"
            preserved = proof.record(input_path)
            result = child(11, "render", rendered, input_path)
            proof.require(result.get("actual_sandbox_renderer") is True and result.get("complete_f5_f9_equality") is True,
                          "Production View/persistence evidence missing")
            proof.require([r.get("zoom") for r in result.get("captures", [])] == [1, 2, 4], "Missing1x/2x/4x captures")
            for row in result["captures"]:
                p = rendered / row["file"]
                proof.require(p.read_bytes().startswith(b"\x89PNG\r\n\x1a\n"), "Invalid capture")
            proof.require(proof.record(input_path) == preserved, "Rendering mutated paid midflight input")
            report["render"] = result
        report["status"] = "PASS"
    except Exception as error:
        failure = error
        report.update(status="BLOCKED" if "BLOCKED:" in str(error) else "FAIL", error=str(error))
    finally:
        try:
            after = proof.inventory(data)
            report.update(original_inventory_before=before, original_inventory_after=after,
                          original_bytes_modes_mtimes_unchanged=before == after,
                          executable_unchanged=executable == proof.record(binary), sources_unchanged=sources == proof.sources())
            proof.require(report["original_bytes_modes_mtimes_unchanged"] and report["executable_unchanged"] and report["sources_unchanged"],
                          "Protected original bytes, executable or source files changed")
        except Exception as error:
            failure = error
            report.update(status="FAIL", error=str(error))
        (output / "acceptance.json").write_text(json.dumps(report, indent=2) + "\n")
    if failure:
        raise failure
    return report


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for key in ("binary", "data", "output", "resources"):
        p.add_argument("--" + key, required=True, type=Path)
    p.add_argument("--ticks", type=int, default=60000)
    p.add_argument("--timeout", type=int, default=1800)
    p.add_argument("--no-render", action="store_true", help="Cross-configuration checks only; visual acceptance remains separate")
    a = p.parse_args()
    try:
        result = workflow(a.binary.absolute(), a.data.absolute(), a.output.absolute(), a.resources.absolute(), a.ticks, a.timeout, not a.no_render)
        print("OE36 technical " + result["status"] + "; economic sustainability evaluated separately; human NOT_RUN")
        return 0
    except Exception as e:
        print("OE36 FAIL: " + str(e), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
