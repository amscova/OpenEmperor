#!/usr/bin/env python3
"""Verify paid Rule10 production and actual OS-process replay without changing original data."""
import argparse
import importlib.util
import json
from pathlib import Path
import shutil
import struct
import tempfile

_SPEC = importlib.util.spec_from_file_location(
    "original_goods_runner", Path(__file__).with_name("test_original_goods.py"))
legacy = importlib.util.module_from_spec(_SPEC)
_SPEC.loader.exec_module(legacy)
MODEL_SHA = legacy.MODEL_SHA
DEVELOPMENT_REVISION = 25
INDUSTRY_CHECKPOINTS = ["mid-aux-courier", "paper-academy-arrival", "mid-luxury-courier", "house-four-luxuries",
    "industry-return", "industry-road-disconnected", "industry-recipe-ready"]
CULTURE_ROLES = (2, 3, 4)
CULTURE_CHECKPOINTS = ["mid-culture-couriers", "culture-road-disconnected", "house-three-cultures",
    "culture-before-expiry", "culture-expired", "culture-revisited"]
STAGES_CHECKPOINTS = ["elite-empty", "elite-first-admission", "elite-counter49", "elite-abandoned", "elite-recovered", "common-attractive", "elite-confucian-visit"]
HOUSING_WITNESS_ROLES = (0, 1, 5, 6, 7, 8, 2, 3, 4)
RELIGION_ROLES = (7, 8, 6)
RELIGION_CHECKPOINTS = ["mid-religion-couriers", "religion-road-disconnected", "house-three-religions",
    "religion-before-expiry", "religion-expired", "religion-revisited"]
SERVICE_PROVIDERS = ((11, 12, 207, 30, 7), (5, 8, 208, 50, 8), (15, 19, 214, 40, 4),
    (17, 21, 215, 45, 4), (18, 22, 217, 45, 4), (19, 23, 219, 80, 12),
    (20, 24, 211, 50, 8), (21, 25, 212, 60, 9), (22, 26, 213, 70, 10))
SERVICE_ROLES = (0, 1, 5, 7, 8, 6, 2, 3, 4)
PRIVATE_OUTPUT = Path(".local/original-economy/oe25/production-fixtures")
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
        assert balance["supported"] is True
        assert all(type(balance[field]) is int and balance[field] >= 0 for field in (
            "produced", "imported", "consumed", "demolished", "stock", "cargo",
            "source_reserved", "incoming_reserved"))
        assert all(balance[field] == 0 for field in ("imported", "demolished"))
        assert balance["stock"] + balance["cargo"] + balance["consumed"] == balance["produced"]
    assert balances[1]["produced"] == scene["pottery_handed_off"] <= scene["pottery_completed"]
    assert balances[0]["produced"] == scene["hemp_harvested"] == sum(
        farm["harvested"] for farm in scene["hemp_farms"])
    assert scene["residents_admitted"] == scene["population"]
    assert sum(house["occupants"] for house in scene["houses"]) == scene["population"]
    assert all(type(house[field]) is int and 0 <= house[field] <= 8
               for house in scene["houses"] for field in ("hemp", "ceramics"))
    for house in scene["houses"]:
        if "industry" in scene:
            assert all(type(house[field]) is int and 0 <= house[field] <= 8 for field in ("hemp", "ceramics", "tea", "silk", "bronzeware", "lacquerware"))
        assert len(house["consumed"]) == 6
        assert all(type(amount) is int and amount >= 0 for amount in house["consumed"])
        if not house["occupants"]:
            assert not any(house["consumed"]) and house["hemp"] == house["ceramics"] == 0
    for good in range(6):
        recipe = sum(row["goods_inputs_used"][good] for row in scene.get("industry", {}).get("sources", []))
        assert sum(house["consumed"][good] for house in scene["houses"]) + recipe == balances[good]["consumed"]
    assert all(stock["kind"] in (3, 9, 14, 29, 30, 31, 32, 33) and len(stock["amounts"]) == 6 for stock in scene["stocks"])
    for stock in scene["stocks"]:
        assert all(type(amount) is int and amount >= 0 for amount in stock["amounts"])
        if stock["kind"] in (14, 31):
            assert stock["amounts"][0] <= 8 and not any(stock["amounts"][1:])
        elif stock["kind"] in (29, 30, 32, 33):
            selected = {29: 4, 30: 5, 32: 3, 33: 2}[stock["kind"]]
            assert stock["amounts"][selected] <= 8 and not any(n for g, n in enumerate(stock["amounts"]) if g != selected)
    for good, field in enumerate(("hemp", "ceramics", "tea", "silk", "bronzeware", "lacquerware")):
        assert balances[good]["stock"] == sum(house.get(field, 0) for house in scene["houses"]) + sum(
            stock["amounts"][good] for stock in scene["stocks"])
        assert balances[good]["cargo"] == sum(courier["typed_cargo"] for courier in scene["couriers"]
                                              if courier["typed_cargo_good"] == good)
    for courier in scene["couriers"]:
        if courier["role"] in (5, 6, 8, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25):
            assert courier["cargo"] == courier["reserved"] == 0
        else:
            assert courier["typed_cargo"] == 0
        if courier["typed_cargo"]:
            assert courier["typed_cargo_good"] in range(6)
            if courier["role"] == 12:
                assert courier["typed_cargo_good"] == 0
        else:
            assert courier["typed_cargo_good"] is None
    if positive:
        assert balances[0]["stock"] > 0 and balances[1]["stock"] > 0 and scene["population"] > 0
        assert scene["workforce_supply"] >= 34
    if "food" in scene:
        check_food(scene)
    if "services" in scene:
        check_services(scene)
    if "service_availability" in scene:
        check_availability(scene)
    if "water" in scene:
        check_water(scene)
    if "housing" in scene:
        check_housing(scene)
    if "industry" in scene:
        check_industry(scene)


def check_industry(scene):
    state = scene["industry"]
    assert len(state["sources"]) <= 18 and len(state["stocks"]) <= 20 and len(state["trips"]) <= 12
    assert len(state["managed_ore"]) <= 2 and len(state["clay_trips"]) <= 4
    bodies = {b["id"]: b for b in scene["buildings"]}
    owners = {row["id"]: row for row in state["stocks"]}
    sources = {row["id"]: row for row in state["sources"]}
    assert len(owners) == len(state["stocks"]) and len(sources) == len(state["sources"])
    output = {27: 0, 28: 1, 31: 4, 34: 3, 35: 2}
    final_goods = {29: 4, 30: 5, 32: 3, 33: 2}
    used_inputs = {28: (0,), 29: (1,), 30: (0, 3), 32: (2,)}
    produced = [0] * 5
    for identity, row in sources.items():
        body = bodies[identity]
        kind = body["kind"]
        assert kind in range(27, 36)
        assert type(row["completed"]) is int and row["completed"] >= 0 and 0 <= row["progress"] < 80
        assert row["completed"] * 80 + row["progress"] <= scene["tick"] - body["placed_tick"]
        assert len(row["goods_inputs_used"]) == 6 and len(row["aux_inputs_used"]) == 5
        assert row["goods_inputs_used"] == [row["completed"] if kind == 31 and g == 0 else 0 for g in range(6)]
        assert row["aux_inputs_used"] == [row["completed"] if a in used_inputs.get(kind, ()) else 0 for a in range(5)]
        assert row["clay_used"] == (row["completed"] if kind == 29 else 0)
        assert row["clay_received"] >= row["clay_used"]
        if kind in (33, 34, 35):
            if row["plant_id"] is None:
                assert row["completed"] == row["progress"] == 0
            else:
                plant = bodies[row["plant_id"]]
                assert plant["kind"] == kind + 3
                x, y = body["cell"]
                px, py = plant["cell"]
                assert max(x-px, 0, px-(x+1)) + max(y-py, 0, py-(y+1)) == 1
                assert row["completed"] * 80 + row["progress"] <= scene["tick"] - plant["placed_tick"]
        else:
            assert row["plant_id"] is None
        if kind in output:
            produced[output[kind]] += row["completed"]
    for good in range(2, 6):
        assert scene["balances"][good]["produced"] == sum(row["completed"] for i, row in sources.items() if final_goods.get(bodies[i]["kind"]) == good)
    ore = {row["id"]: row for row in state["managed_ore"]}
    assert set(ore) == {i for i in sources if bodies[i]["kind"] == 28}
    for identity, row in ore.items():
        assert row["stock"] + row["used"] == row["extracted"] and row["used"] == sources[identity]["completed"]
        assert 0 <= row["stock"] <= 8 and 0 <= row["progress"] < 80
        assert row["extracted"] * 80 + row["progress"] <= scene["tick"] - bodies[identity]["placed_tick"]
    cargo, incoming, returning = [0] * 5, {}, {}
    ids = set()
    actors = {row["id"]: row for row in scene["couriers"]}
    for row in state["trips"]:
        identity, material = row["courier_id"], row["material"]
        assert identity not in ids and material in range(5)
        ids.add(identity)
        actor = actors[identity]
        assert actor["role"] in (24, 25) and actor["owner"] == row["source"]
        assert 0 < row["sequence"] <= scene["tick"] and 0 < row["dispatched_tick"] <= scene["tick"]
        assert 0 < row["loaded"] <= 4 and row["delivered"] >= 0 and row["returned"] >= 0
        held = row["loaded"] - row["delivered"] - row["returned"]
        assert 0 <= held <= 4 and actor["aux_cargo"] == held and actor["cargo"] == actor["reserved"] == 0
        if row["arrival_tick"] is not None:
            assert row["dispatched_tick"] < row["arrival_tick"] <= scene["tick"]
        if row["phase"] in (1, 2, 5):
            assert row["delivered"] == 0
        if row["phase"] == 3:
            assert row["delivered"] == row["loaded"] and row["returned"] == 0 and row["arrival_tick"] is not None
        if row["phase"] == 5:
            assert row["returned"] == row["loaded"]
        cargo[material] += held
        returning[row["source"], material] = returning.get((row["source"], material), 0) + held
        if row["phase"] == 1:
            incoming[row["target"], material] = incoming.get((row["target"], material), 0) + held
    for identity, row in owners.items():
        kind = bodies[identity]["kind"]
        assert kind in (3, 23, 27, 28, 29, 30, 31, 32, 34, 35)
        for name in ("amounts", "received", "departed", "returned", "consumed"):
            assert len(row[name]) == 5 and all(type(n) is int and n >= 0 for n in row[name])
        for a in range(5):
            own = sources.get(identity, {}).get("completed", 0) if output.get(kind) == a else 0
            assert own + row["received"][a] + row["returned"][a] == row["amounts"][a] + row["departed"][a] + row["consumed"][a]
            expected_sink = sources.get(identity, {}).get("aux_inputs_used", [0]*5)[a]
            if kind == 23 and a == 4:
                actor = next(c for c in scene["couriers"] if c["owner"] == identity and c["role"] == 19)
                expected_sink = next((t["sequence"] for t in scene["services"]["trips"] if t["courier_id"] == actor["id"]), 0)
            assert row["consumed"][a] == expected_sink
            assert row["amounts"][a] + returning.get((identity, a), 0) + incoming.get((identity, a), 0) <= (32 if kind == 3 else 8)
    for a, balance in enumerate(state["balances"]):
        assert balance["material"] == a and balance["conserved"] is True and balance["produced"] == produced[a]
        assert balance["stock"] == sum(row["amounts"][a] for row in owners.values())
        assert balance["cargo"] == cargo[a] and balance["consumed"] == sum(row["consumed"][a] for row in owners.values())
        assert produced[a] == balance["stock"] + balance["cargo"] + balance["consumed"]


def check_industry_fixture(fixture):
    assert fixture["status"] == "PASS" and fixture["snapshot_injection"] is False
    assert fixture["authored_geography"] is fixture["real_model_prices_and_workers"] is True
    assert fixture["managed_ore_authored"] is True and fixture["original_resource_calendar_parity"] is False
    assert [row["tag"] for row in fixture["checkpoints"]] == INDUSTRY_CHECKPOINTS
    for name in ("initial", "staffed", "mid_aux", "paper_arrived", "mid_luxury", "delivered", "return_state", "disconnected", "recipe_ready", "final"):
        scene = fixture[name]
        check_balances(scene)
        assert scene["model_sha256"] == MODEL_SHA and scene["development_revision"] == DEVELOPMENT_REVISION
        assert scene["funds"] + scene["construction_spent"] == 12000
    initial = fixture["initial"]
    assert initial["construction_spent"] == 3159 and initial["funds"] == 8841
    assert initial["building_count"] == 43 and initial["house_count"] == 20 and initial["road_count"] == 107 and initial["courier_count"] == 21
    assert initial["population"] == 0 and fixture["staffed"]["workforce_supply"] >= fixture["staffed"]["installed_workforce_required"]
    assert any(c.get("aux_cargo", 0) > 0 and c["edge_progress"] > 0 for c in fixture["mid_aux"]["couriers"])
    academy = fixture["academy_id"]
    row = next(r for r in fixture["paper_arrived"]["industry"]["stocks"] if r["id"] == academy)
    assert row["amounts"][4] > 0 and row["received"][4] > 0
    assert any(c["role"] == 23 and c["typed_cargo"] > 0 and c["edge_progress"] > 0 for c in fixture["mid_luxury"]["couriers"])
    house = next(h for h in fixture["delivered"]["houses"] if h["id"] == fixture["home_id"])
    assert all(house[field] > 0 for field in ("tea", "silk", "bronzeware", "lacquerware"))
    assert any(t["phase"] == 2 and t["loaded"] > t["returned"] for t in fixture["return_state"]["industry"]["trips"])
    assert fixture["disconnected"]["road_count"] == 106
    ready = next(r for r in fixture["recipe_ready"]["industry"]["sources"] if r["id"] == fixture["bronze_id"])
    assert ready["progress"] == 79
    assert next(b for b in fixture["recipe_ready"]["buildings"] if b["id"] == fixture["bronze_id"])["input_clay"] > 0
    assert next(r for r in fixture["recipe_ready"]["industry"]["stocks"] if r["id"] == fixture["bronze_id"])["amounts"][1] > 0
    assert len(fixture["demolition_refusals"]) == 6 and fixture["neutral_well_demolished"] is True


def service_countdowns(tick):
    assert type(tick) is int and 0 <= tick <= 2**64 - 1
    return 0 if tick < 36 else 1 + (tick - 36) // 51


def water_evaluation_tick(tick):
    assert type(tick) is int and tick >= 0
    return 0 if tick < 29 else tick - (tick - 29) % 51


def water_timer(witness, tick):
    if witness is None:
        return 0
    assert type(witness["arrival_tick"]) is int and 0 < witness["arrival_tick"] <= tick
    return max(96 - (service_countdowns(tick) - service_countdowns(witness["arrival_tick"] - 1)), 0)


def check_water(scene):
    state, tick = scene["water"], scene["tick"]
    assert state["countdowns"] == service_countdowns(tick)
    assert state["evaluation_tick"] == water_evaluation_tick(tick)
    assert len(state["providers"]) <= 2 and len(state["trips"]) <= 2 and len(state["houses"]) <= 20
    houses = {h["id"]: h for h in scene["houses"]}
    buildings = {b["id"]: b for b in scene["buildings"]}
    couriers = {c["id"]: c for c in scene["couriers"]}
    providers = {p["id"]: p for p in state["providers"]}
    assert len(providers) == len(state["providers"])
    assert set(providers) == {b["id"] for b in scene["buildings"] if b["kind"] == 20}
    actors = {c["id"]: c for c in scene["couriers"] if c["role"] == 16}
    assert len(actors) == len(providers)
    for identity, p in providers.items():
        assert p["cost"] == 25 and p["workers_required"] == 4
        matching = [c for c in actors.values() if c["owner"] == identity]
        assert len(matching) == 1
        c = matching[0]
        assert c["cargo"] == c["reserved"] == c["typed_cargo"] == c["food_cargo"] == 0
        assert c["typed_cargo_good"] is None
    trips = {t["courier_id"]: t for t in state["trips"]}
    assert len(trips) == len(state["trips"])
    for identity, t in trips.items():
        assert identity in actors and type(t["sequence"]) is int and 0 < t["sequence"] <= t["dispatched_tick"] <= tick
        c = actors[identity]
        assert c["owner"] in providers and t["target"] in houses
        assert c["last_dispatched_target"] == t["target"]
        assert t["dispatched_tick"] > buildings[c["owner"]]["placed_tick"]
        assert t["dispatched_tick"] >= buildings[t["target"]]["placed_tick"] + 121
        assert type(t["visit_applied"]) is bool
        if t["arrival_tick"] is None:
            assert not t["visit_applied"] and c["phase"] == 1
        else:
            assert type(t["arrival_tick"]) is int and t["dispatched_tick"] < t["arrival_tick"] <= tick
            assert c["phase"] in (0, 2)
    for identity in actors.keys() - trips.keys():
        c = actors[identity]
        assert c["phase"] == 0 and c["last_dispatched_target"] is None and c["target"] == c["owner"]

    def witness(w, home, at):
        if w is None:
            return
        assert w["house_id"] == home and w["provider_id"] in providers and w["courier_id"] in actors
        assert actors[w["courier_id"]]["owner"] == w["provider_id"]
        assert type(w["arrival_tick"]) is int and 0 < w["arrival_tick"] <= at
        assert w["arrival_tick"] >= buildings[home]["placed_tick"] + 122
        t = trips[w["courier_id"]]
        assert type(w["sequence"]) is int and 0 < w["sequence"] <= t["sequence"]
        if w["sequence"] == t["sequence"]:
            assert t["visit_applied"] and t["target"] == home and t["arrival_tick"] == w["arrival_tick"]
        else:
            assert w["arrival_tick"] < t["dispatched_tick"]

    rows = {h["id"]: h for h in state["houses"]}
    assert len(rows) == len(state["houses"]) and rows.keys() == houses.keys()
    for identity, h in rows.items():
        current, evaluated = h["current_witness"], h["evaluation_witness"]
        witness(current, identity, tick)
        witness(evaluated, identity, state["evaluation_tick"])
        assert type(h["normal_timer"]) is int and h["normal_timer"] == water_timer(current, tick)
        assert type(h["special_timer"]) is int and h["special_timer"] == 0
        if evaluated:
            assert current and evaluated["arrival_tick"] <= current["arrival_tick"]
        assert type(houses[identity]["water"]) is bool
        assert houses[identity]["water"] is bool(water_timer(evaluated, state["evaluation_tick"]))


def housing_evaluation_tick(tick):
    return 0 if tick < 40 else tick - (tick - 40) % 51


def housing_consumption_events(tick):
    return (1 + (tick - 40) // 816 if tick >= 40 else 0) + (1 + (tick - 397) // 816 if tick >= 397 else 0)


def housing_needs(row, state):
    service_fields = (2, 4, 5, 6, 7, 21, 22, 23, 23)
    if any(row[field] != 0 and state["service_timers"][role] == 0 for role, field in enumerate(service_fields[:7])):
        return False
    if row[23] and not (state["service_timers"][7] or state["service_timers"][8]):
        return False
    if row[3] and not state["water"]:
        return False
    if state["food_quality"] < row[8]:
        return False
    if any(state["goods"][good] < row[9 + good] for good in range(4)):
        return False
    return not row[13] or state["goods"][4] >= row[13] or state["goods"][5] >= row[13]


def housing_step(rows, state):
    """Independent fifteen-row reference; no World type is assigned here."""
    kind, counter = state["type"], state["devolve_counter"]
    if kind == 11 or (kind in (2, 3) and state["occupants"] <= 0):
        return kind, counter
    row = rows[max(0, kind - 3)]
    next_row = 10 if kind == 11 else 14 if kind == 17 else max(0, kind - 3) + 1
    verdict = 1 if kind == 12 else -1 if state["desirability"] <= row[0] else 1 if state["desirability"] >= row[1] else 0
    if not housing_needs(row, state):
        verdict = -1
    elif verdict == 1 and not housing_needs(rows[next_row], state):
        verdict = 0
    if kind == 2:
        return (3 if verdict == 1 else 2), counter
    if verdict == 1:
        return (kind + 1 if kind in (*range(3, 10), *range(12, 17)) else kind), 0
    if verdict == 0:
        return kind, 0
    limit = 9 if kind <= 10 else 11 if kind == 12 else 49 if kind == 13 else 17
    if counter < limit:
        return kind, counter + 1
    return (kind - 1 if kind in (*range(4, 11), *range(13, 18)) else kind), 0


def check_housing(scene):
    state, tick = scene["housing"], scene["tick"]
    assert state["evaluation_tick"] == housing_evaluation_tick(tick)
    assert state["consumption_events"] == housing_consumption_events(tick)
    rows = state["model_rows"]
    assert len(rows) == 15 and all(len(r) == 24 and all(type(n) is int for n in r) for r in rows)
    assert rows[8][17] == 0 and all(0 < row[17] <= 32767 for index, row in enumerate(rows) if index != 8)
    houses = {h["id"]: h for h in scene["houses"]}
    histories = {h["id"]: h for h in state["houses"]}
    buildings = {b["id"]: b for b in scene["buildings"]}
    assert len(histories) == len(state["houses"]) and histories.keys() == houses.keys()
    assert len(state["neutral_source_origins"]) <= 37
    origins = {o["id"]: o["cell"] for o in state["neutral_source_origins"]}
    assert len(origins) == len(state["neutral_source_origins"])
    for identity, cell in origins.items():
        assert type(identity) is int and identity > 0 and len(cell) == 2 and all(type(n) is int for n in cell)
        if identity in buildings:
            assert buildings[identity]["kind"] == 13 and buildings[identity]["cell"] == cell

    def evaluation(e, identity, admission=False):
        assert type(e["tick"]) is int and 0 < e["tick"] <= tick
        if not admission:
            assert e["tick"] % 51 == 40
        i = e["input"]
        elite = buildings[identity]["kind"] == 39
        assert i["type"] in (range(11, 18) if elite else range(2, 11))
        assert type(i["occupants"]) is int and (i["occupants"] == 0 if admission else 0 < i["occupants"] <= 32767)
        limit = 0 if i["type"] in (2, 11) else 9 if i["type"] <= 10 else 11 if i["type"] == 12 else 49 if i["type"] == 13 else 17
        assert type(i["devolve_counter"]) is int and 0 <= i["devolve_counter"] <= limit
        assert type(i["water"]) is bool and type(i["desirability"]) is int and -128 <= i["desirability"] <= 127
        assert len(i["goods"]) == 6 and all(type(n) is int and 0 <= n <= 8 for n in i["goods"])
        assert len(i["service_timers"]) == 9 and all(type(n) is int and 0 <= n <= (40 if r >= 5 else 96) for r, n in enumerate(i["service_timers"]))
        if not elite:
            assert i["service_timers"][6] == 0
        assert len(e["service_witnesses"]) == 9
        for index, role in enumerate(HOUSING_WITNESS_ROLES):
            w = e["service_witnesses"][index]
            expected = 0
            if w:
                assert w["house_id"] == identity and w["role"] == role and 0 < w["arrival_tick"] <= e["tick"]
                assert role != 6 or elite
                expected = max((40 if role >= 5 else 96) - (service_countdowns(e["tick"]) - service_countdowns(w["arrival_tick"] - 1)), 0)
            assert i["service_timers"][role] == expected
        assert i["water"] is bool(water_timer(e["water_evaluation_witness"], water_evaluation_tick(e["tick"])))
        assert len(e["food_received"]) == len(e["food_consumed"]) == 2
        assert all(type(e[key][g]) is int and 0 <= e["food_consumed"][g] <= e["food_received"][g] for key in ("food_received", "food_consumed") for g in (0, 1))
        amount = sum(e["food_received"][g] - e["food_consumed"][g] for g in (0, 1))
        assert 0 <= amount <= 8
        w = e["food_quality_witness"]
        if w:
            assert 0 < w["tick"] <= e["tick"]
            previous = [e["food_received"][g] - w["accepted"][g] - w["consumed_before"][g] for g in (0, 1)]
            assert all(n >= 0 for n in previous)
            expected = food_blend(w["previous_quality"], food_batch_quality(w["accepted"]), sum(w["accepted"]), sum(previous))
            assert i["food_quality"] == (expected if amount else 0)
        else:
            assert not any(e["food_received"]) and not amount and i["food_quality"] == 0
        assert len(e["goods_consumed_before"]) == 6 and all(type(n) is int and n >= 0 for n in e["goods_consumed_before"])

    capacity = 0
    for identity, h in houses.items():
        audit = histories[identity]
        elite = buildings[identity]["kind"] == 39
        assert buildings[identity]["kind"] in (6, 39)
        assert h["type"] in (range(11, 18) if elite else range(2, 11))
        limit = 0 if h["type"] in (2, 11) else 9 if h["type"] <= 10 else 11 if h["type"] == 12 else 49 if h["type"] == 13 else 17
        assert type(h["devolve_counter"]) is int and 0 <= h["devolve_counter"] <= limit
        assert type(h["occupants"]) is int and 0 <= h["occupants"] <= 32767
        capacity += rows[9 if h["type"] == 11 else max(0, h["type"] - 3)][17]
        admitted = audit["first_admission_tick"]
        if admitted is None:
            assert h["type"] == (11 if elite else 2) and h["occupants"] == 0
        else:
            assert type(admitted) is int and buildings[identity]["placed_tick"] + 120 <= admitted <= tick
            assert (admitted - buildings[identity]["placed_tick"] - 120) % 40 == 0 and h["occupants"] > 0
        assert len(audit["consumption_rows"]) == 15
        for row, model in zip(audit["consumption_rows"], rows, strict=True):
            assert type(row["events"]) is int and 0 <= row["events"] <= state["consumption_events"]
            assert len(row["consumed"]) == 6
            for good, n in enumerate(row["consumed"]):
                need = max(model[9 + good] if good < 4 else model[13], 0)
                assert type(n) is int and 0 <= n <= row["events"] * min(need, 8)
        assert sum(r["events"] for r in audit["consumption_rows"]) <= state["consumption_events"]
        assert [sum(r["consumed"][g] for r in audit["consumption_rows"]) for g in range(6)] == h["consumed"]
        transitions = audit["first_transitions"]
        assert len(transitions) == 15
        for row, transition in enumerate(transitions):
            if transition is None:
                continue
            target = row + 3
            assert (target >= 11) is elite and type(transition["kind"]) is int and transition["kind"] in (0, 1)
            e = transition["evaluation"]
            evaluation(e, identity, transition["kind"] == 0)
            if transition["kind"] == 0:
                assert target == (13 if elite else 3) and e["tick"] == admitted
                assert e["input"]["type"] == (11 if elite else 2)
                assert type(transition["admitted"]) is int
                assert transition["admitted"] == min(2, rows[11 if elite else 0][17])
            else:
                assert transition["admitted"] == 0 and housing_step(rows, e["input"]) == (target, 0)
        assert audit["first_hut_evaluation"] == (transitions[1]["evaluation"] if transitions[1] else None)
        assert audit["first_plain_evaluation"] == (transitions[2]["evaluation"] if transitions[2] else None)
        if admitted is not None:
            assert transitions[10 if elite else 0] and transitions[10 if elite else 0]["kind"] == 0
            visited = [row for row, proof in enumerate(transitions) if proof]
            assert h["occupants"] <= max(rows[row][17] for row in visited)
        e = audit["last_evaluation"]
        if e:
            evaluation(e, identity)
            assert e["tick"] == state["evaluation_tick"]
            assert housing_step(rows, e["input"]) == (h["type"], h["devolve_counter"])
    assert scene["capacity"] == capacity


def check_housing_fixture(fixture):
    assert fixture["status"] == "PASS" and fixture["snapshot_injection"] is False
    assert fixture["authored_geography"] is fixture["real_model_housing"] is fixture["actual_first_hut_and_plain"] is True
    assert fixture["actual_tenth_failure_devolution"] is fixture["grand_actual_all_arrivals"] is True
    identity = fixture["home_id"]
    for key in ("initial", "midroute", "arrived", "hut", "plain", "counter9", "devolved", "grand_initial", "grand_final"):
        check_balances(fixture[key])
        assert fixture[key]["model_sha256"] == MODEL_SHA
    assert fixture["initial"]["funds"] == 11292 and fixture["initial"]["construction_spent"] == 708
    assert fixture["initial"]["population"] == 0 and fixture["initial"]["house_count"] == 5
    home = lambda name: next(h for h in fixture[name]["houses"] if h["id"] == identity)
    assert any(c["role"] == 16 and c["phase"] == 1 and c["edge_progress"] > 0 for c in fixture["midroute"]["couriers"])
    assert home("arrived")["water"] is False
    assert next(h for h in fixture["arrived"]["water"]["houses"] if h["id"] == identity)["normal_timer"] > 0
    assert home("hut")["type"] == 4 and home("hut")["water"] is True
    assert home("plain")["type"] == 5
    plain_proof = next(h for h in fixture["plain"]["housing"]["houses"] if h["id"] == identity)["first_plain_evaluation"]
    assert plain_proof["input"]["water"] and plain_proof["input"]["food_quality"] >= 20 and plain_proof["input"]["service_timers"][5] > 0
    assert fixture["plain"]["housing"]["model_rows"][2][9:11] == [0, 0]
    assert home("counter9")["type"] == 5 and home("counter9")["devolve_counter"] == 9
    assert home("devolved")["type"] == 4 and home("devolved")["devolve_counter"] == 0
    assert home("devolved")["occupants"] >= home("counter9")["occupants"]
    assert fixture["grand_initial"]["construction_spent"] == 1948 and fixture["grand_initial"]["population"] == 0
    assert any(h["type"] == 5 and h["water"] and h["hemp"] > 0 and h["ceramics"] > 0 and h["food_quality"] >= 20 and
               all(h["service_timers"][r] > 0 for r in (0, 1, 5)) for h in fixture["grand_final"]["houses"])
    assert [n["case"] for n in fixture["negatives"]] == ["no-water-influence-only", "no-ancestor", "no-food-field", "unreachable-market",
        "authored-missing-hemp-need1", "authored-insufficient-desirability"]
    for index, negative in enumerate(fixture["negatives"]):
        assert negative["authored_model"] is (index >= 4) and negative["original_model_parity"] is (index < 4)
        check_balances(negative["final"])
        assert all(h["type"] < 5 for h in negative["final"]["houses"])
        assert all(not h["first_plain_evaluation"] for h in negative["final"]["housing"]["houses"])
        if index < 4:
            assert negative["final"]["model_sha256"] == MODEL_SHA
        else:
            assert negative["final"]["model_sha256"] != MODEL_SHA
            if index == 4:
                assert negative["final"]["housing"]["model_rows"][2][9] == 1
                assert all(not h["hemp"] for h in negative["final"]["houses"])
            else:
                assert all(h["desirability"] < negative["final"]["housing"]["model_rows"][1][1] for h in negative["final"]["houses"])


def check_availability(scene):
    """Derived diagnosis only: actual source capacity/travel is not a visit."""
    rows = scene["service_availability"]
    houses = {h["id"]: h for h in scene["houses"]}
    buildings = {b["id"]: b for b in scene["buildings"]}
    providers = scene["services"]["providers"]
    visits = {(w["house_id"], w["role"]): w for w in scene["services"]["witnesses"]}
    trips = {t["courier_id"]: t for t in scene["services"]["trips"]}
    couriers = {c["id"]: c for c in scene["couriers"]}
    keys = [(r["house_id"], r["role"]) for r in rows]
    assert keys == [(identity, role) for identity in houses for role in range(9)]
    for r in rows:
        identity, role = r["house_id"], r["role"]
        h = houses[identity]
        expected_eligible = h["occupants"] > 0 and h["access_enabled"] and h["status"] in (1, 3) and not buildings[identity]["on_fire"] and (role != 6 or buildings[identity]["kind"] == 39)
        assert type(r["eligible_target"]) is bool and r["eligible_target"] is expected_eligible
        assert type(r["delivered_timer"]) is int and r["delivered_timer"] == h["service_timers"][role]
        matching = [p for p in providers if SERVICE_ROLES[p["type"]] == role]
        paper = {row["id"]: row["amounts"][4] for row in scene.get("industry", {}).get("stocks", [])}
        available = [p for p in matching if buildings[p["id"]]["operating"] and not buildings[p["id"]]["on_fire"] and
                     buildings[p["id"]]["workers"] == p["workers_required"] and (p["type"] != 5 or paper.get(p["id"], 0) > 0)]
        assert r["provider_count"] == len(matching) and r["available_source_count"] == len(available)
        for field in ("provider_count", "available_source_count", "reachable_source_count", "travelling_to_target_count"):
            assert type(r[field]) is int and 0 <= r[field] <= 2
        assert r["reachable_source_count"] <= r["available_source_count"]
        if not expected_eligible:
            assert r["reachable_source_count"] == 0
        actual_travelling = 0
        for p in matching:
            c = next(c for c in couriers.values() if c["owner"] == p["id"])
            t = trips.get(c["id"])
            actual_travelling += bool(t and c["phase"] == 1 and t["arrival_tick"] is None and t["target"] == identity)
        assert r["travelling_to_target_count"] == actual_travelling
        visit = visits.get((identity, role))
        assert r["last_arrival_tick"] == (visit["arrival_tick"] if visit else None)


def check_culture_fixture(fixture):
    assert fixture["status"] == "PASS" and fixture["snapshot_injection"] is False
    assert fixture["authored_geography"] is fixture["real_model_prices_and_workers"] is True
    assert fixture["experimental_authored_targeted_school_visits"] is True
    assert fixture["original_venue_performer_loop"] is fixture["free_venue_or_performer"] is False
    assert fixture["culture_source_cost"] == 50 + 60 + 70 == 180
    assert fixture["culture_source_workers"] == 8 + 9 + 10 == 27
    assert fixture["combined_workers"] == 100 and fixture["paused_workers"] == 73
    identity = fixture["home_id"]
    home = lambda name: next(h for h in fixture[name]["houses"] if h["id"] == identity)
    diagnosis = lambda name, role: next(r for r in fixture[name]["service_availability"] if r["house_id"] == identity and r["role"] == role)
    for name in ("initial", "stopped", "midroute", "disconnected", "visited", "before_expiry", "expired", "revisited"):
        scene = fixture[name]
        assert "service_availability" in scene
        check_balances(scene)
        assert scene["development_revision"] == DEVELOPMENT_REVISION and scene["model_sha256"] == MODEL_SHA
        assert scene["construction_spent"] + scene["funds"] == 12000
    initial = fixture["initial"]
    assert initial["construction_spent"] == 2127 and initial["funds"] == 9873
    assert initial["house_count"] == 15 and initial["road_count"] == 84 and initial["building_count"] == 29 and initial["courier_count"] == 14
    assert initial["population"] == 0 and fixture["stopped"]["population"] == fixture["stopped"]["workforce_supply"] == 103
    assert not fixture["stopped"]["services"]["trips"] and not fixture["stopped"]["services"]["witnesses"]
    assert fixture["midroute"]["workforce_used"] == 100 and fixture["visited"]["workforce_used"] == 73
    assert all(not b["operating"] and b["workers"] == 0 for b in fixture["visited"]["buildings"] if b["kind"] in (24, 25, 26))
    for role in CULTURE_ROLES:
        d = diagnosis("midroute", role)
        assert d["available_source_count"] == d["reachable_source_count"] == d["travelling_to_target_count"] == 1
        assert d["delivered_timer"] == 0 and d["last_arrival_tick"] is None
        stopped = diagnosis("disconnected", role)
        assert stopped["available_source_count"] == stopped["reachable_source_count"] == stopped["delivered_timer"] == 0
        assert stopped["travelling_to_target_count"] == 1
        arrived = diagnosis("visited", role)
        assert arrived["available_source_count"] == arrived["reachable_source_count"] == arrived["travelling_to_target_count"] == 0
        assert arrived["delivered_timer"] > 0 and arrived["last_arrival_tick"] is not None
        assert diagnosis("expired", role)["delivered_timer"] == 0
    assert all(home(name)["service_timers"][r] == 0 for name in ("midroute", "disconnected") for r in CULTURE_ROLES)
    assert all(home("visited")["service_timers"]) and home("visited")["water"]
    assert fixture["disconnected"]["road_count"] == 83 and fixture["disconnected"]["construction_spent"] == 2127
    assert fixture["visited"]["road_count"] == 84 and fixture["visited"]["construction_spent"] == 2129
    check_services(fixture["visited"], culture=True)
    expiries = []
    for role in CULTURE_ROLES:
        first = next(w for w in fixture["visited"]["services"]["witnesses"] if w["house_id"] == identity and w["role"] == role)
        expired = next(w for w in fixture["expired"]["services"]["witnesses"] if w["house_id"] == identity and w["role"] == role)
        again = next(w for w in fixture["revisited"]["services"]["witnesses"] if w["house_id"] == identity and w["role"] == role)
        expiries.append(36 + (service_countdowns(first["arrival_tick"] - 1) + 95) * 51)
        assert expired == first and again["sequence"] > first["sequence"] and again["arrival_tick"] > fixture["expiry_tick"]
        assert home("expired")["service_timers"][role] == 0
    assert fixture["expiry_ticks"] == expiries and fixture["expiry_tick"] == max(expiries)
    assert fixture["before_expiry"]["tick"] + 1 == fixture["expired"]["tick"] == fixture["expiry_tick"]
    assert any(home("before_expiry")["service_timers"][r] == 1 for r in CULTURE_ROLES)
    check_services(fixture["revisited"], culture=True)
    assert [c["role"] for c in fixture["predicate_controls"]] == list(CULTURE_ROLES)
    for index, control in enumerate(fixture["predicate_controls"]):
        role = index + 2
        assert control["authored_model"] is True and control["original_plain_requirement"] is False
        assert control["changed_field"] == 5 + index
        for name in ("prerequisites", "delivered", "expired"):
            scene = control[name]
            check_balances(scene)
            assert scene["model_sha256"] != MODEL_SHA and scene["development_revision"] == DEVELOPMENT_REVISION
            h = next(h for h in scene["houses"] if h["id"] == control["home_id"])
            assert h["water"] and h["service_timers"][5] > 0
            if name == "delivered":
                assert h["type"] == 5 and h["service_timers"][role] > 0
                history = next(h for h in scene["housing"]["houses"] if h["id"] == control["home_id"])
                proof = history["first_plain_evaluation"]
                assert proof["tick"] == scene["tick"] and proof["input"]["type"] == 4
                assert proof["input"]["water"] and proof["input"]["food_quality"] >= 20
                assert proof["input"]["service_timers"][5] > 0 and proof["input"]["service_timers"][role] > 0
                assert len(proof["service_witnesses"]) == 9
                assert proof["service_witnesses"][6 + index]["role"] == role
                actual_visit = next(w for w in scene["services"]["witnesses"] if w["house_id"] == control["home_id"] and w["role"] == role)
                assert proof["service_witnesses"][6 + index] == actual_visit
                # Evolution precedes the same-tick0/7 sink. Bind the actual
                # historical Food20 input and exact possible wheat-first debit.
                food = next(row for row in scene["food"]["stocks"] if row["id"] == control["home_id"])
                assert food["received"] == proof["food_received"]
                before = [proof["food_received"][g] - proof["food_consumed"][g] for g in (0, 1)]
                debit = [0, 0]
                if housing_consumption_events(scene["tick"]) > housing_consumption_events(scene["tick"] - 1) and sum(before):
                    debit[0 if before[0] else 1] = 1
                assert food["consumed"] == [proof["food_consumed"][g] + debit[g] for g in (0, 1)]
                assert food["amounts"] == [before[g] - debit[g] for g in (0, 1)]
                assert h["food_quality"] == (proof["input"]["food_quality"] if sum(food["amounts"]) else 0)
            else:
                assert h["food_quality"] >= 20 and h["service_timers"][role] == 0
                assert not housing_needs(scene["housing"]["model_rows"][2], dict(water=h["water"], food_quality=h["food_quality"],
                    service_timers=h["service_timers"], goods=[h["hemp"], h["ceramics"], 0, 0, 0, 0]))
                if name == "prerequisites":
                    assert h["type"] == 4
                    assert next(h for h in scene["housing"]["houses"] if h["id"] == control["home_id"])["first_plain_evaluation"] is None
    assert [c["role"] for c in fixture["no_road_controls"]] == list(CULTURE_ROLES)
    for control in fixture["no_road_controls"]:
        scene = control["final"]
        check_balances(scene)
        assert scene["model_sha256"] == MODEL_SHA and scene["population"] == 14
        assert not scene["services"]["trips"] and not scene["services"]["witnesses"]
        assert all(h["service_timers"][control["role"]] == 0 for h in scene["houses"])


def check_services(scene, positive=False, ancestor=False, religions=False, culture=False):
    """Independently bind sole House timers to paid providers and actual trips."""
    state = scene["services"]
    tick = scene["tick"]
    assert state["countdowns"] == service_countdowns(tick)
    assert len(state["providers"]) <= 18 and len(state["trips"]) <= 18 and len(state["witnesses"]) <= 180
    integer = lambda n, low=0, high=2**64 - 1: type(n) is int and low <= n <= high
    houses = {h["id"]: h for h in scene["houses"]}
    buildings = {b["id"]: b for b in scene["buildings"]}
    couriers = {c["id"]: c for c in scene["couriers"]}
    providers = {p["id"]: p for p in state["providers"]}
    assert len(providers) == len(state["providers"])
    assert set(providers) == {b["id"] for b in scene["buildings"] if b["kind"] in (8, 12, 19, 21, 22, 23, 24, 25, 26)}
    actors = {c["id"] for c in scene["couriers"] if c["role"] in (5, 11, 15, 17, 18, 19, 20, 21, 22)}
    assert len(actors) == len(providers)
    for p in providers.values():
        assert integer(p["id"], 1, 2**32 - 1) and integer(p["type"], 0, 8)
        role, kind, model, cost, workers = SERVICE_PROVIDERS[p["type"]]
        assert p["model_type"] == model and p["cost"] == cost and p["workers_required"] == workers
        assert buildings[p["id"]]["kind"] == kind
        matching = [c for c in couriers.values() if c["owner"] == p["id"] and c["role"] == role]
        assert len(matching) == 1
        c = matching[0]
        assert c["cargo"] == c["reserved"] == c["typed_cargo"] == c["food_cargo"] == 0
        assert c["typed_cargo_good"] is None
    for category in range(9):
        assert sum(p["type"] == category for p in providers.values()) <= 2
    trips = {t["courier_id"]: t for t in state["trips"]}
    assert len(trips) == len(state["trips"])
    for t in trips.values():
        assert t["courier_id"] in actors and integer(t["sequence"], 1)
        assert integer(t["dispatched_tick"], 1, tick) and t["sequence"] <= t["dispatched_tick"]
        c = couriers[t["courier_id"]]
        assert c["target"] == c["last_dispatched_target"] == t["target"] and t["target"] in houses
        target, source = buildings[t["target"]], buildings[c["owner"]]
        assert t["dispatched_tick"] > source["placed_tick"]
        assert t["dispatched_tick"] >= target["placed_tick"] + 121
        assert houses[t["target"]]["occupants"] > 0
        assert c["role"] != 19 or buildings[t["target"]]["kind"] == 39
        assert type(t["visit_applied"]) is bool
        if t["arrival_tick"] is None:
            assert not t["visit_applied"] and c["phase"] == 1
        else:
            assert integer(t["arrival_tick"], t["dispatched_tick"] + 1, tick) and c["phase"] in (0, 2)
    for identity in actors - trips.keys():
        c = couriers[identity]
        assert c["phase"] == 0 and c["last_dispatched_target"] is None and c["target"] == c["owner"]
    witnesses = {(w["house_id"], w["role"]): w for w in state["witnesses"]}
    assert len(witnesses) == len(state["witnesses"])
    for (identity, role), w in witnesses.items():
        assert identity in houses and type(role) is int and role in SERVICE_ROLES and w["provider_id"] in providers
        p = providers[w["provider_id"]]
        assert SERVICE_ROLES[p["type"]] == role and w["courier_id"] in actors
        assert role != 6 or buildings[identity]["kind"] == 39
        c = couriers[w["courier_id"]]
        assert c["owner"] == p["id"] and w["courier_id"] in trips
        t = trips[w["courier_id"]]
        assert integer(w["sequence"], 1, t["sequence"]) and integer(w["arrival_tick"], 1, tick)
        assert w["arrival_tick"] >= buildings[identity]["placed_tick"] + 122
        assert w["arrival_tick"] >= buildings[p["id"]]["placed_tick"] + 2
        assert w["sequence"] <= w["arrival_tick"]
        if w["sequence"] == t["sequence"]:
            assert t["visit_applied"] and t["target"] == identity and t["arrival_tick"] == w["arrival_tick"]
        else:
            assert w["arrival_tick"] < t["dispatched_tick"]
    for t in trips.values():
        if t["visit_applied"]:
            role = SERVICE_ROLES[providers[couriers[t["courier_id"]]["owner"]]["type"]]
            assert (t["target"], role) in witnesses
            w = witnesses[(t["target"], role)]
            assert (w["arrival_tick"], w["courier_id"]) >= (t["arrival_tick"], t["courier_id"])
    for identity, h in houses.items():
        assert len(h["service_timers"]) == 9
        assert all(integer(timer, 0, 96) for timer in h["service_timers"])
        for role, timer in enumerate(h["service_timers"]):
            w = witnesses.get((identity, role))
            duration = 40 if role >= 5 else 96
            expected = max(duration - (service_countdowns(tick) - service_countdowns(w["arrival_tick"] - 1)), 0) if w else 0
            assert timer == expected
        if any(h["service_timers"]):
            assert h["occupants"] > 0 and h["status"] in (1, 3) and h["access_enabled"] is True
    if culture:
        assert all(any(w["role"] == r for w in witnesses.values()) for r in CULTURE_ROLES)
        assert any(all(h["service_timers"][r] > 0 for r in CULTURE_ROLES) for h in houses.values())
    elif religions:
        assert all(any(w["role"] == r for w in witnesses.values()) for r in RELIGION_ROLES)
        assert any(all(h["service_timers"][r] > 0 for r in RELIGION_ROLES) for h in houses.values())
    elif ancestor:
        assert any(w["role"] == 5 for w in witnesses.values())
        assert any(h["service_timers"][5] > 0 for h in houses.values())
    elif positive:
        assert {w["role"] for w in witnesses.values()} == {0, 1}
        assert any(h["service_timers"][0] > 0 and h["service_timers"][1] > 0 for h in houses.values())


def check_service_expiry(fixture):
    before, after = fixture["before_expiry"], fixture["expired"]
    check_services(fixture["visited"], positive=True)
    check_services(before)
    check_services(after)
    w = next(w for w in before["services"]["witnesses"] if w["role"] == 0)
    expiry = 36 + (service_countdowns(w["arrival_tick"] - 1) + 95) * 51
    assert fixture["expiry_tick"] == expiry and before["tick"] == expiry - 1 and after["tick"] == expiry
    assert before["services"]["witnesses"] == after["services"]["witnesses"] == fixture["visited"]["services"]["witnesses"]
    assert before["services"]["trips"] == after["services"]["trips"]
    old = next(h for h in before["houses"] if h["id"] == w["house_id"])
    new = next(h for h in after["houses"] if h["id"] == w["house_id"])
    assert old["service_timers"][0] == 1 and new["service_timers"][0] == 0
    assert all(not b["operating"] for b in before["buildings"] if b["kind"] in (8, 12))


def check_ancestor_expiry(fixture):
    visited, before, after = (fixture[key] for key in ("visited", "before_expiry", "expired"))
    check_services(visited, ancestor=True)
    check_services(before)
    check_services(after)
    witnesses = [w for w in before["services"]["witnesses"] if w["role"] == 5]
    assert len(witnesses) == 1
    w = witnesses[0]
    expiry = 36 + (service_countdowns(w["arrival_tick"] - 1) + 39) * 51
    assert fixture["expiry_tick"] == expiry and before["tick"] == expiry - 1 and after["tick"] == expiry
    assert before["services"]["witnesses"] == after["services"]["witnesses"] == visited["services"]["witnesses"]
    assert before["services"]["trips"] == after["services"]["trips"]
    old = next(h for h in before["houses"] if h["id"] == w["house_id"])
    new = next(h for h in after["houses"] if h["id"] == w["house_id"])
    assert old["service_timers"][5] == 1 and new["service_timers"][5] == 0
    assert all(not b["operating"] for b in before["buildings"] if b["kind"] in (8, 12, 19, 21, 22, 23, 24, 25, 26))


def check_ancestor_fixture(fixture):
    assert fixture["status"] == "PASS" and fixture["snapshot_injection"] is False
    assert fixture["authored_geography"] is fixture["real_model_ancestor"] is True
    assert all(fixture[key] is True for key in ("actual_ancestor_arrival", "actual_ancestor_revisit",
                                               "paid_road_repair", "paused_outbound_tour_finished"))
    assert fixture["paid_house_count"] == 12 and fixture["paid_road_count"] == 77
    assert fixture["paid_food_farm_count"] == fixture["paid_food_field_count"] == 2
    assert all(fixture[key] == 1 for key in ("paid_market_count", "paid_herbalist_count",
                                           "paid_acupuncture_count", "paid_ancestor_count"))
    initial, stopped, moving = (fixture[key] for key in ("initial", "stopped", "midroute"))
    assert initial["funds"] == 10517 and initial["construction_spent"] == 1483
    assert initial["population"] == initial["workforce_supply"] == 0
    assert initial["courier_count"] == 7
    assert not initial["services"]["trips"] and not initial["services"]["witnesses"]
    assert stopped["population"] == stopped["workforce_supply"] == 84 and not stopped["services"]["witnesses"]
    assert moving["workforce_used"] == 67
    for key in ("initial", "stopped", "midroute", "disconnected", "repaired", "visited", "before_expiry", "expired", "revisited"):
        check_balances(fixture[key])
        assert fixture[key]["model_sha256"] == MODEL_SHA and fixture[key]["development_revision"] == DEVELOPMENT_REVISION
    moving_ancestor = next(c for c in moving["couriers"] if c["role"] == 15)
    assert moving_ancestor["phase"] == 1 and moving_ancestor["edge_progress"] > 0
    assert all(h["service_timers"][5] == 0 for h in moving["houses"])
    disconnected, repaired = fixture["disconnected"], fixture["repaired"]
    t = next(t for t in disconnected["services"]["trips"] if t["courier_id"] == moving_ancestor["id"])
    assert t["arrival_tick"] is None and not t["visit_applied"]
    assert all(h["service_timers"][5] == 0 for h in disconnected["houses"])
    assert repaired["funds"] == disconnected["funds"] - 2
    assert repaired["construction_spent"] == disconnected["construction_spent"] + 2
    assert repaired["command_sequence"] == disconnected["command_sequence"] + 1
    assert repaired["road_count"] == disconnected["road_count"] + 1
    check_ancestor_expiry(fixture)
    check_services(fixture["revisited"], ancestor=True)
    first = next(w for w in fixture["visited"]["services"]["witnesses"] if w["role"] == 5)
    again = next(w for w in fixture["revisited"]["services"]["witnesses"] if w["role"] == 5 and w["house_id"] == first["house_id"])
    assert again["sequence"] > first["sequence"] and again["arrival_tick"] > fixture["expiry_tick"]


def check_stages_fixture(fixture):
    assert fixture["status"] == "PASS" and fixture["snapshot_injection"] is False
    assert fixture["real_model_housing_stages"] is fixture["authored_geography"] is True
    assert fixture["temporary_elite_geometry"] == [2, 2] and fixture["original_elite_required_side"] == 4
    assert fixture["authored_elite_plot_price"] == 80 and fixture["paid_city_heavenly_reached"] is False
    assert [entry["tag"] for entry in fixture["checkpoints"]] == STAGES_CHECKPOINTS
    for name in ("initial", "admitted", "counter49", "abandoned", "recovered", "advanced", "visited"):
        scene = fixture[name]
        check_balances(scene)
        assert scene["model_sha256"] == MODEL_SHA and scene["development_revision"] == DEVELOPMENT_REVISION
        assert scene["construction_spent"] == fixture["scene_spent"] and scene["funds"] + scene["construction_spent"] == 12000
    elite = fixture["elite_id"]
    home = lambda name: next(h for h in fixture[name]["houses"] if h["id"] == elite)
    proof = lambda name: next(h for h in fixture[name]["housing"]["houses"] if h["id"] == elite)
    assert home("initial")["type"] == 11 and home("initial")["occupants"] == 0
    admission = proof("admitted")["first_transitions"][10]
    assert fixture["admitted"]["tick"] == 120 and admission["kind"] == 0 and admission["admitted"] == 2
    assert admission["evaluation"]["input"]["type"] == 11 and admission["evaluation"]["input"]["occupants"] == 0
    assert home("admitted")["type"] == 13 and home("admitted")["occupants"] == 2
    assert home("counter49")["type"] == 13 and home("counter49")["devolve_counter"] == 49
    assert home("abandoned")["type"] == 12 and home("abandoned")["devolve_counter"] == 0
    down = proof("abandoned")["first_transitions"][9]
    assert down["kind"] == 1 and down["evaluation"]["input"]["type"] == 13 and down["evaluation"]["input"]["devolve_counter"] == 49
    recovered = proof("recovered")["last_evaluation"]
    assert recovered["input"]["type"] == 12 and recovered["input"]["goods"][0] > 0 and recovered["input"]["goods"][1] > 0
    assert home("recovered")["type"] == 13 and home("recovered")["occupants"] >= home("abandoned")["occupants"]
    common = next(h for h in fixture["advanced"]["houses"] if h["id"] == fixture["home_id"])
    assert 6 <= common["type"] <= 10
    common_history = next(h for h in fixture["advanced"]["housing"]["houses"] if h["id"] == fixture["home_id"])
    actual_first_attractive = common_history["first_transitions"][3]
    assert actual_first_attractive and actual_first_attractive["kind"] == 1
    assert actual_first_attractive["evaluation"]["input"]["food_quality"] >= 30
    assert home("visited")["service_timers"][6] > 0
    assert all(h["service_timers"][6] == 0 for h in fixture["visited"]["houses"] if h["type"] < 11)
    academy = next(s for s in fixture["visited"]["industry"]["stocks"] if s["id"] == fixture["academy_id"])
    assert academy["received"][4] > 0 and academy["consumed"][4] > 0
    assert len(fixture["higher_stage_observations"]) == 2


def check_religion_fixture(fixture):
    assert fixture["status"] == "PASS" and fixture["snapshot_injection"] is False
    assert fixture["authored_geography"] is fixture["real_model_prices_and_workers"] is True
    assert fixture["original_confucian_resource_gate"] is True and fixture["resource_grant"] is False
    assert "Paper" in fixture["confucian_adapter"] and "paid" in fixture["confucian_adapter"]
    assert fixture["religion_source_cost"] == 45 + 45 + 80 == 170
    assert fixture["religion_source_workers"] == 4 + 4 + 12 == 20
    assert fixture["combined_workers"] == 73
    identity = fixture["home_id"]
    names = ("initial", "stopped", "midroute", "disconnected", "visited", "before_expiry", "expired", "revisited", "barrier")
    for key in names:
        scene = fixture[key]
        check_balances(scene)
        assert scene["model_sha256"] == MODEL_SHA and scene["development_revision"] == DEVELOPMENT_REVISION
        assert scene["funds"] + scene["construction_spent"] == 12000
    initial = fixture["initial"]
    assert initial["construction_spent"] == 1571 and initial["funds"] == 10429 and initial["population"] == 0
    assert initial["house_count"] == 11 and initial["road_count"] == 56 and initial["courier_count"] == 11
    assert fixture["stopped"]["population"] == fixture["stopped"]["workforce_supply"] == 75
    assert not fixture["stopped"]["services"]["trips"]
    assert fixture["midroute"]["workforce_used"] == 73 and fixture["visited"]["workforce_used"] == 53
    assert all(not b["operating"] and b["workers"] == 0 for b in fixture["visited"]["buildings"] if b["kind"] in (21, 22, 23))
    for role in (17, 18, 19):
        moving = next(c for c in fixture["midroute"]["couriers"] if c["role"] == role)
        assert moving["phase"] == 1 and moving["edge_progress"] > 0
        assert next(t for t in fixture["midroute"]["services"]["trips"] if t["courier_id"] == moving["id"])["arrival_tick"] is None
    home = lambda key: next(h for h in fixture[key]["houses"] if h["id"] == identity)
    assert all(home(key)["service_timers"][r] == 0 for key in ("midroute", "disconnected") for r in RELIGION_ROLES)
    assert fixture["disconnected"]["road_count"] == 55 and fixture["disconnected"]["construction_spent"] == 1571
    assert fixture["visited"]["road_count"] == 56 and fixture["visited"]["construction_spent"] == 1573
    assert all(home("visited")["service_timers"][r] > 0 for r in (0, 1, 5, 6, 7, 8)) and home("visited")["water"]
    check_services(fixture["visited"], religions=True)
    for i, role in enumerate(RELIGION_ROLES):
        witness = next(w for w in fixture["visited"]["services"]["witnesses"] if w["house_id"] == identity and w["role"] == role)
        assert fixture["expiry_ticks"][i] == 36 + (service_countdowns(witness["arrival_tick"] - 1) + 39) * 51
        repeated = next(w for w in fixture["revisited"]["services"]["witnesses"] if w["house_id"] == identity and w["role"] == role)
        assert repeated["sequence"] > witness["sequence"] and repeated["arrival_tick"] > fixture["expiry_tick"]
        assert home("expired")["service_timers"][role] == 0
    assert fixture["expiry_tick"] == max(fixture["expiry_ticks"])
    assert fixture["before_expiry"]["tick"] + 1 == fixture["expired"]["tick"] == fixture["expiry_tick"]
    assert any(home("before_expiry")["service_timers"][r] == 1 for r in RELIGION_ROLES)
    check_services(fixture["revisited"], religions=True)
    for key in ("before_expiry", "expired"):
        assert all(not b["operating"] for b in fixture[key]["buildings"] if b["kind"] in (21, 22, 23))
    assert [p["case"] for p in fixture["predicate_controls"]] == ["dao-only", "buddhist-only", "neither-or", "no-confucian", "common-confucian-refused"]
    for index, p in enumerate(fixture["predicate_controls"]):
        assert p["authored_model"] is True and p["original_model_or_requirement"] is False
        final = p["final"]
        prerequisites = p["prerequisites"]
        check_balances(prerequisites)
        supplied = next(h for h in prerequisites["houses"] if h["id"] == p["home_id"])
        assert supplied["water"] and supplied["food_quality"] >= 20 and supplied["service_timers"][5] > 0
        assert prerequisites["model_sha256"] == final["model_sha256"]
        check_balances(final)
        assert final["model_sha256"] != MODEL_SHA and final["housing"]["model_rows"][2][22:] == [int(index >= 3), 1]
        h = next(h for h in final["houses"] if h["id"] == p["home_id"])
        assert h["water"] and h["service_timers"][5] > 0
        if index < 2:
            role, other = ((7, 8), (8, 7))[index]
            assert h["type"] == 5 and h["food_quality"] >= 20 and h["service_timers"][role] > 0 and h["service_timers"][other] == 0 and h["service_timers"][6] == 0
            check_balances(p["expired"])
            expired = next(h for h in p["expired"]["houses"] if h["id"] == p["home_id"])
            assert expired["service_timers"][role] == expired["service_timers"][other] == 0
            assert not housing_needs(final["housing"]["model_rows"][2], dict(type=expired["type"], water=expired["water"],
                food_quality=expired["food_quality"], service_timers=expired["service_timers"], goods=[expired["hemp"], expired["ceramics"], 0, 0, 0, 0]))
        else:
            assert p["expired"] is None and h["type"] == 4
            assert all(not a["first_plain_evaluation"] for a in final["housing"]["houses"])
            assert not h["service_timers"][6] if index >= 3 else not any(h["service_timers"][r] for r in (7, 8))
            assert not supplied["service_timers"][6] if index >= 3 else not any(supplied["service_timers"][r] for r in (7, 8))
            assert not housing_needs(final["housing"]["model_rows"][2], dict(type=supplied["type"], water=supplied["water"],
                food_quality=supplied["food_quality"], service_timers=supplied["service_timers"], goods=[supplied["hemp"], supplied["ceramics"], 0, 0, 0, 0]))
    barrier = fixture["barrier"]
    assert fixture["barrier_authored_topology"] is fixture["barrier_shared_policy_navigation"] is True
    assert barrier["population"] == 56 and barrier["workforce_used"] == 50
    assert not barrier["services"]["trips"] and not barrier["services"]["witnesses"]
    assert all(h["service_timers"][r] == 0 for h in barrier["houses"] for r in RELIGION_ROLES)


def food_batch_quality(amounts):
    assert len(amounts) == 2 and all(type(n) is int and 0 <= n <= 2**31 - 1 for n in amounts) and sum(amounts) <= 2**31 - 1
    return (20 * amounts[0] + 80 * amounts[1]) // sum(amounts) if sum(amounts) else 0


def food_blend(previous_quality, source_quality, delivered, previous_stock):
    """Independent bounded arrival vector: physical House quantities never exceed8."""
    assert all(type(n) is int for n in (previous_quality, source_quality, delivered, previous_stock))
    assert 0 <= previous_quality <= 80 and 0 <= source_quality <= 80 and 0 <= delivered <= 4 and 0 <= previous_stock <= 8
    if not delivered:
        return previous_quality
    if source_quality > previous_quality:
        return source_quality
    single = lambda n: struct.unpack("f", struct.pack("f", n))[0]
    ratio = single(delivered / previous_stock) if previous_stock else single(10)
    for threshold, house_weight, source_weight in ((3, 1, 3), (2, 1, 2), (.5, 1, 1), (.33, 2, 1)):
        if ratio > single(threshold):
            return (previous_quality * house_weight + source_quality * source_weight) // (house_weight + source_weight)
    return (3 * previous_quality + source_quality) // 4


def check_food(scene, positive=False):
    food = scene["food"]
    assert food["conserved"] is True and len(food["farms"]) <= 2 and len(food["stocks"]) <= 26 and len(food["trips"]) <= 6
    vector = lambda values, maximum: (len(values) == 2 and all(type(n) is int and 0 <= n <= maximum for n in values))
    for key in ("produced", "stock", "cargo", "consumed"):
        assert vector(food[key], 2**64 - 1)
    farms = {farm["farm_id"]: farm for farm in food["farms"]}
    assert len(farms) == len(food["farms"])
    for farm in food["farms"]:
        assert type(farm["crop"]) is int and farm["crop"] in (0, 1)
        assert type(farm["harvested"]) is int and 0 <= farm["harvested"] <= 2**64 - 1
        if not farm["field_id"]:
            assert farm["harvested"] == 0
    stocks = {row["id"]: row for row in food["stocks"]}
    assert len(stocks) == len(food["stocks"])
    houses = {house["id"]: house for house in scene["houses"]}
    couriers = {courier["id"]: courier for courier in scene["couriers"]}
    trips = {trip["courier_id"]: trip for trip in food["trips"]}
    assert len(trips) == len(food["trips"])
    for row in food["stocks"]:
        assert row["kind"] in (6, 9, 16, 39) and vector(row["amounts"], 16)
        assert sum(row["amounts"]) <= (16 if row["kind"] == 9 else 8)
        for key in ("received", "departed", "returned", "consumed"):
            assert vector(row[key], 2**64 - 1)
        assert type(row["missed_demands"]) is int and 0 <= row["missed_demands"] <= 2**64 - 1
        for good in (0, 1):
            harvest = farms[row["id"]]["harvested"] if row["id"] in farms and farms[row["id"]]["crop"] == good else 0
            assert harvest + row["received"][good] + row["returned"][good] == row["amounts"][good] + row["departed"][good] + row["consumed"][good]
        if row["kind"] not in (6, 39):
            assert row["quality"] == 0 and row["quality_witness"] is None and row["missed_demands"] == 0 and not any(row["consumed"])
            continue
        assert row["id"] in houses and row["quality"] == houses[row["id"]]["food_quality"]
        assert type(row["quality"]) is int and 0 <= row["quality"] <= 80
        assert not any(row["departed"]) and not any(row["returned"])
        witness = row["quality_witness"]
        if witness is None:
            assert not any(row["received"]) and not any(row["amounts"]) and row["quality"] == 0
            continue
        assert vector(witness["accepted"], 4) and 0 < sum(witness["accepted"]) <= 4
        assert vector(witness["consumed_before"], 2**64 - 1)
        assert type(witness["tick"]) is int and 0 < witness["tick"] <= scene["tick"]
        assert type(witness["sequence"]) is int and witness["sequence"] > 0
        assert witness["courier_id"] in couriers and couriers[witness["courier_id"]]["role"] == 14
        assert witness["courier_id"] in trips and witness["sequence"] <= trips[witness["courier_id"]]["sequence"]
        previous = [row["received"][g] - witness["accepted"][g] - witness["consumed_before"][g] for g in (0, 1)]
        assert all(n >= 0 for n in previous) and sum(previous) + sum(witness["accepted"]) <= 8
        assert all(witness["consumed_before"][g] <= row["consumed"][g] for g in (0, 1))
        if row["received"] == witness["accepted"]:
            assert not any(previous) and witness["previous_quality"] == 0
        expected = food_blend(witness["previous_quality"], food_batch_quality(witness["accepted"]), sum(witness["accepted"]), sum(previous))
        assert row["quality"] == (expected if sum(row["amounts"]) else 0)
    for trip in food["trips"]:
        assert type(trip["sequence"]) is int and 0 < trip["sequence"] < 2**64 - 1
        assert trip["source"] in stocks and trip["target"] in stocks
        assert trip["courier_id"] in couriers and couriers[trip["courier_id"]]["role"] in (13, 14)
        for key in ("loaded", "delivered", "returned", "cargo"):
            assert vector(trip[key], 4)
        assert 0 < sum(trip["loaded"]) <= 4
        assert trip["cargo"] == [trip["loaded"][g] - trip["delivered"][g] - trip["returned"][g] for g in (0, 1)]
        assert trip["batch_quality"] == food_batch_quality(trip["cargo"])
        assert couriers[trip["courier_id"]]["food_cargo"] == sum(trip["cargo"])
    for good in (0, 1):
        assert food["produced"][good] == sum(farm["harvested"] for farm in food["farms"] if farm["crop"] == good)
        assert food["stock"][good] == sum(row["amounts"][good] for row in food["stocks"])
        assert food["cargo"][good] == sum(trip["cargo"][good] for trip in food["trips"])
        assert food["consumed"][good] == sum(row["consumed"][good] for row in food["stocks"])
        assert food["produced"][good] == food["stock"][good] + food["cargo"][good] + food["consumed"][good]
    if positive:
        assert all(n > 0 for n in food["produced"]) and any(row["quality_witness"] for row in food["stocks"])
        assert all(n > 0 for n in food["consumed"])


def checked_path(repo, root, path):
    path = Path(path)
    if not path.is_absolute() or not path.is_relative_to(root) or ".." in path.parts:
        raise RuntimeError("Fixture record escaped its private output root")
    legacy.check_output_path(repo, path)
    if not path.is_file():
        raise RuntimeError(f"Fixture output is missing: {path}")
    return path


def authored_consumption_model(source):
    """Change exactly two Shelter requirement bytes in a private model copy."""
    marker = b"1: Shelter,{,"
    if source.count(marker) != 1:
        raise RuntimeError("Pinned Shelter row is not unique")
    start = source.index(marker) + len(marker)
    ends = [position for byte in (b"\r", b"\n") if (position := source.find(byte, start)) >= 0]
    if not ends:
        raise RuntimeError("Pinned Shelter row has no line boundary")
    end = min(ends)
    fields = source[start:end].split(b",")
    if len(fields) < 24 or fields[9] != b"0" or fields[10] != b"0" or int(fields[17]) != 7:
        raise RuntimeError("Pinned zero-need Shelter inputs changed")
    fields[9] = fields[10] = b"1"
    result = source[:start] + b",".join(fields) + source[end:]
    if len(result) != len(source) or sum(a != b for a, b in zip(source, result)) != 2:
        raise RuntimeError("Authored needs changed more than the two explicit model bytes")
    return result


def check_consumption_boundary(boundary):
    assert boundary["step"] in (0, 7) and boundary["house_arrivals_at_boundary"] is False
    before, at, after = (boundary[key] for key in ("before", "at", "after"))
    for scene in (before, at, after):
        check_balances(scene)
        assert scene["shelter_needs"] == [1, 1]
    assert before["tick"] % 816 == 39 + boundary["step"] * 51
    assert at["tick"] == before["tick"] + 1 and after["tick"] == at["tick"] + 1
    assert len(before["houses"]) == len(at["houses"]) == len(after["houses"]) == 6
    totals = [0, 0]
    rows = before["housing"]["model_rows"]
    for previous, charged, following, recorded in zip(
            before["houses"], at["houses"], after["houses"], boundary["expected_house_deltas"], strict=True):
        assert previous["id"] == charged["id"] == following["id"] == recorded["building_id"]
        input_state = {key: previous[key] for key in ("type", "desirability", "occupants", "devolve_counter", "water", "service_timers", "food_quality")}
        input_state["goods"] = [previous["hemp"], previous["ceramics"], 0, 0, 0, 0]
        evolved_type, evolved_counter = housing_step(rows, input_state)
        assert (recorded["evolved_type"], recorded["evolved_counter"]) == (evolved_type, evolved_counter)
        assert recorded["evolved_desirability"] == input_state["desirability"]
        assert (charged["type"], charged["devolve_counter"]) == (evolved_type, evolved_counter)
        assert (following["type"], following["devolve_counter"]) == (evolved_type, evolved_counter)
        expected = [min(previous[field], 1) if previous["occupants"] else 0 for field in ("hemp", "ceramics")]
        assert recorded["consumed"] == expected + [0, 0, 0, 0]
        for good, field in enumerate(("hemp", "ceramics")):
            assert charged[field] == previous[field] - expected[good] == following[field]
            assert charged["consumed"][good] == previous["consumed"][good] + expected[good]
            assert following["consumed"][good] == charged["consumed"][good]
            totals[good] += expected[good]
    assert all(amount > 0 for amount in totals)
    for good in (0, 1):
        assert at["balances"][good]["consumed"] == before["balances"][good]["consumed"] + totals[good]
        assert after["balances"][good]["consumed"] == at["balances"][good]["consumed"]


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
        raise RuntimeError("Expected pinned original Model SHA before explicitly authored copy preparation")
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
    assert fixture["paid_house_count"] == 6 and fixture["paid_road_count"] == 24
    assert all(fixture[key] == 1 for key in ("paid_clay_count", "paid_pottery_count", "paid_warehouse_count", "paid_market_count",
                                           "paid_hemp_farm_count", "paid_hemp_field_count"))
    assert fixture["stop_controls_verified"] is True and fixture["natural_fire_verified"] is True
    assert fixture["paused_hemp_return_verified"] is True and fixture["actual_two_good_house_arrival"] is True
    assert fixture["initial"]["population"] == fixture["initial"]["workforce_supply"] == 0
    assert fixture["initial"]["funds"] == 10838 and fixture["initial"]["construction_spent"] == 1162
    assert fixture["stopped"]["population"] > 0 and fixture["stopped"]["clay_extracted"] == 0
    for key in ("initial", "stopped", "final", "fire", "paused_return"):
        check_balances(fixture[key], positive=key == "final")
        assert fixture[key]["model_sha256"] == MODEL_SHA
        assert fixture[key]["shelter_needs"] == [0, 0]
        assert all(balance["consumed"] == 0 for balance in fixture[key]["balances"])
    assert any(house["hemp"] > 0 and house["ceramics"] > 0 for house in fixture["final"]["houses"])
    authored_stage = output / "authored-consumption-data"
    (authored_stage / "Model").mkdir(parents=True)
    (authored_stage / "Cities").mkdir()
    authored_model = authored_stage / "Model/EmperorBuildingModels.txt"
    authored_model.write_bytes(authored_consumption_model(source_model.read_bytes()))
    shutil.copy2(stage / "Cities/Influence.map", authored_stage / "Cities/Influence.map")
    authored_sha = digest(authored_model)
    assert authored_sha != MODEL_SHA
    authored_before = inventory(authored_stage)
    legacy.run(binary, ["--create-consumption", authored_stage, output / "consumption"], output / "consumption.log")
    consumption = json.loads((output / "consumption/fixture.json").read_text())
    assert consumption["status"] == "PASS" and consumption["snapshot_injection"] is False
    assert consumption["authored_model_needs"] is True and consumption["original_model_parity"] is False
    assert consumption["positive_runtime_consumption"] is True and consumption["authored_geography"] is True
    assert consumption["paid_house_count"] == 6 and consumption["paid_road_count"] == 24
    assert consumption["initial"]["funds"] == 10838 and consumption["initial"]["construction_spent"] == 1162
    assert consumption["initial"]["population"] == 0
    assert [entry["step"] for entry in consumption["boundaries"]] == [0, 7]
    for boundary in consumption["boundaries"]:
        check_consumption_boundary(boundary)
        assert all(boundary[key]["model_sha256"] == authored_sha for key in ("before", "at", "after"))
    check_balances(consumption["final"])
    assert all(consumption["final"]["balances"][good]["consumed"] > 0 for good in (0, 1))
    legacy.run(binary, ["--create-food", stage, output / "food"], output / "food.log")
    food_fixture = json.loads((output / "food/fixture.json").read_text())
    assert food_fixture["status"] == "PASS" and food_fixture["snapshot_injection"] is False
    assert food_fixture["real_model_food"] is food_fixture["positive_runtime_food"] is True
    assert food_fixture["paid_house_count"] == 12 and food_fixture["paid_road_count"] == 77
    assert food_fixture["paid_food_farm_count"] == food_fixture["paid_food_field_count"] == 2
    assert food_fixture["paid_market_count"] == 1
    assert food_fixture["initial"]["funds"] == 10637 and food_fixture["initial"]["construction_spent"] == 1363
    assert food_fixture["initial"]["population"] == food_fixture["initial"]["workforce_supply"] == 0
    assert food_fixture["stopped"]["population"] == food_fixture["stopped"]["workforce_supply"] == 84
    assert food_fixture["actual_mixed_cargo"] is food_fixture["actual_food_return"] is True
    assert food_fixture["actual_quality_classes"] == [20, 80]
    for key in ("initial", "stopped", "low_quality", "mixed_market", "mixed_arrival", "paused_return", "final"):
        check_balances(food_fixture[key])
        assert food_fixture[key]["model_sha256"] == MODEL_SHA
    assert any(house["food_quality"] == 20 for house in food_fixture["low_quality"]["houses"])
    assert any(house["food_quality"] == 80 for house in food_fixture["final"]["houses"])
    assert any(row["kind"] == 9 and all(n > 0 for n in row["amounts"])
               for row in food_fixture["mixed_market"]["food"]["stocks"])
    assert any(row["quality_witness"] and all(n > 0 for n in row["quality_witness"]["accepted"])
               for row in food_fixture["mixed_arrival"]["food"]["stocks"])
    legacy.run(binary, ["--create-services", stage, output / "services"], output / "services.log")
    services_fixture = json.loads((output / "services/fixture.json").read_text())
    assert services_fixture["status"] == "PASS" and services_fixture["snapshot_injection"] is False
    assert services_fixture["real_model_services"] is services_fixture["actual_two_provider_arrivals"] is True
    assert services_fixture["paused_outbound_tours_finished"] is True
    assert services_fixture["paid_house_count"] == 12 and services_fixture["paid_road_count"] == 77
    assert services_fixture["paid_food_farm_count"] == services_fixture["paid_food_field_count"] == 2
    assert all(services_fixture[key] == 1 for key in ("paid_market_count", "paid_herbalist_count", "paid_acupuncture_count"))
    assert services_fixture["initial"]["funds"] == 10557 and services_fixture["initial"]["construction_spent"] == 1443
    assert services_fixture["initial"]["population"] == services_fixture["initial"]["workforce_supply"] == 0
    assert services_fixture["stopped"]["population"] == services_fixture["stopped"]["workforce_supply"] == 84
    for key in ("initial", "stopped", "visited", "before_expiry", "expired"):
        check_balances(services_fixture[key])
        assert services_fixture[key]["model_sha256"] == MODEL_SHA
    assert not services_fixture["initial"]["services"]["trips"] and not services_fixture["stopped"]["services"]["witnesses"]
    check_service_expiry(services_fixture)
    legacy.run(binary, ["--create-ancestor", stage, output / "ancestor"], output / "ancestor.log")
    ancestor_fixture = json.loads((output / "ancestor/fixture.json").read_text())
    check_ancestor_fixture(ancestor_fixture)
    legacy.run(binary, ["--create-housing", stage, output / "housing"], output / "housing.log")
    housing_fixture = json.loads((output / "housing/fixture.json").read_text())
    check_housing_fixture(housing_fixture)
    housing_negative_inventories = []
    for negative in housing_fixture["negatives"]:
        if negative["authored_model"]:
            model_path = checked_path(repo, output, negative["model_path"])
            expected = source_model.read_bytes()
            marker = (b"2: Hut,{," if negative["changed_row"] == 1 else b"3: Plain Cottage,{,")
            assert expected.count(marker) == 1
            start = expected.index(marker) + len(marker)
            for _ in range(negative["changed_field"]):
                start = expected.index(b",", start) + 1
            end = expected.index(b",", start)
            assert int(expected[start:end]) == negative["before"]
            expected = expected[:start] + str(negative["after"]).encode() + expected[end:]
            assert model_path.read_bytes() == expected and digest(model_path) == negative["final"]["model_sha256"]
            housing_negative_inventories.append((model_path.parent.parent, inventory(model_path.parent.parent)))
    legacy.run(binary, ["--create-religion", stage, output / "religion"], output / "religion.log")
    religion_fixture = json.loads((output / "religion/fixture.json").read_text())
    check_religion_fixture(religion_fixture)
    religion_private_inventories = []
    for predicate in religion_fixture["predicate_controls"]:
        model_path = checked_path(repo, output, predicate["model_path"])
        expected = source_model.read_bytes()
        marker = b"3: Plain Cottage,{,"
        assert expected.count(marker) == 1
        for field in ((22, 23) if predicate["case"] in ("no-confucian", "common-confucian-refused") else (23,)):
            start = expected.index(marker) + len(marker)
            for _ in range(field):
                start = expected.index(b",", start) + 1
            end = expected.index(b",", start)
            assert expected[start:end] == b"0"
            expected = expected[:start] + b"1" + expected[end:]
        assert model_path.read_bytes() == expected and digest(model_path) == predicate["final"]["model_sha256"]
        religion_private_inventories.append((model_path.parent.parent, inventory(model_path.parent.parent)))
    legacy.run(binary, ["--create-culture", stage, output / "culture"], output / "culture.log")
    culture_fixture = json.loads((output / "culture/fixture.json").read_text())
    check_culture_fixture(culture_fixture)
    culture_private_inventories = []
    for control in culture_fixture["predicate_controls"]:
        model_path = checked_path(repo, output, control["model_path"])
        expected = source_model.read_bytes()
        marker = b"3: Plain Cottage,{,"
        assert expected.count(marker) == 1
        start = expected.index(marker) + len(marker)
        for _ in range(control["changed_field"]):
            start = expected.index(b",", start) + 1
        end = expected.index(b",", start)
        assert expected[start:end] == b"0"
        expected = expected[:start] + b"1" + expected[end:]
        assert model_path.read_bytes() == expected and digest(model_path) == control["delivered"]["model_sha256"]
        culture_private_inventories.append((model_path.parent.parent, inventory(model_path.parent.parent)))
    restarts = []
    assert [entry["tag"] for entry in fixture["checkpoints"]] == ["mid-hemp-courier", "house-two-goods"]
    assert [entry["tag"] for entry in consumption["checkpoints"]] == ["step-0-before", "step-7-before"]
    assert [entry["tag"] for entry in food_fixture["checkpoints"]] == [
        "mid-food-courier", "food-return", "mixed-food-courier", "house-food-quality"]
    assert [entry["tag"] for entry in services_fixture["checkpoints"]] == [
        "mid-services-couriers", "house-medical-visits", "service-before-expiry", "service-expired"]
    assert [entry["tag"] for entry in ancestor_fixture["checkpoints"]] == [
        "mid-ancestor-courier", "ancestor-road-disconnected", "house-ancestor-visit",
        "ancestor-before-expiry", "ancestor-expired", "ancestor-revisited"]
    assert [entry["tag"] for entry in housing_fixture["checkpoints"]] == [
        "mid-water-courier", "water-arrived-pre-evaluation", "house-hut-upgrade", "house-plain-upgrade",
        "house-devolution-counter9", "house-devolved"]
    checkpoint_inputs = [(entry, stage, MODEL_SHA, False, False, False, False, False) for entry in fixture["checkpoints"]]
    checkpoint_inputs += [(entry, authored_stage, authored_sha, True, False, False, False, False) for entry in consumption["checkpoints"]]
    checkpoint_inputs += [(entry, stage, MODEL_SHA, False, True, False, False, False) for entry in food_fixture["checkpoints"]]
    checkpoint_inputs += [(entry, stage, MODEL_SHA, False, False, True, False, False) for entry in services_fixture["checkpoints"]]
    checkpoint_inputs += [(entry, stage, MODEL_SHA, False, False, True, True, False) for entry in ancestor_fixture["checkpoints"]]
    checkpoint_inputs += [(entry, stage, MODEL_SHA, False, False, False, False, True) for entry in housing_fixture["checkpoints"]]
    assert [entry["tag"] for entry in religion_fixture["checkpoints"]] == RELIGION_CHECKPOINTS
    checkpoint_inputs = [row + (False,) for row in checkpoint_inputs]
    checkpoint_inputs += [(entry, stage, MODEL_SHA, False, False, True, False, False, True) for entry in religion_fixture["checkpoints"]]
    assert [entry["tag"] for entry in culture_fixture["checkpoints"]] == CULTURE_CHECKPOINTS
    checkpoint_inputs = [row + (False,) for row in checkpoint_inputs]
    checkpoint_inputs += [(entry, stage, MODEL_SHA, False, False, True, False, False, False, True) for entry in culture_fixture["checkpoints"]]
    legacy.run(binary, ["--create-industry", stage, output / "industry"], output / "industry.log")
    industry_fixture = json.loads((output / "industry/fixture.json").read_text())
    check_industry_fixture(industry_fixture)
    checkpoint_inputs = [row + (False,) for row in checkpoint_inputs]
    checkpoint_inputs += [(entry, stage, MODEL_SHA, False, False, False, False, False, False, False, True) for entry in industry_fixture["checkpoints"]]
    legacy.run(binary, ["--create-stages", stage, output / "stages"], output / "stages.log")
    stages_fixture = json.loads((output / "stages/fixture.json").read_text())
    check_stages_fixture(stages_fixture)
    checkpoint_inputs = [row + (False,) for row in checkpoint_inputs]
    checkpoint_inputs += [(entry, stage, MODEL_SHA, False, False, False, False, False, False, False, False, True)
                         for entry in stages_fixture["checkpoints"]]
    for checkpoint, replay_data, model_sha, authored_needs, food_scenario, services_scenario, ancestor_scenario, housing_scenario, religion_scenario, culture_scenario, industry_scenario, stages_scenario in checkpoint_inputs:
        source = checked_path(repo, output, checkpoint["save"])
        source_identity = file_identity(source)
        source_report = json.loads(checked_path(repo, output, checkpoint["report"]).read_text())
        check_balances(source_report)
        assert source_report["model_sha256"] == model_sha
        if checkpoint["tag"] == "mid-hemp-courier":
            assert any(c["role"] == 12 and c["typed_cargo_good"] == 0 and c["typed_cargo"] > 0 and
                       c["cargo"] == 0 and c["edge_progress"] > 0
                       for c in source_report["couriers"])
        elif not authored_needs and not food_scenario and not services_scenario and not housing_scenario and not industry_scenario and not stages_scenario:
            check_balances(source_report, positive=True)
            assert any(house["hemp"] > 0 and house["ceramics"] > 0 for house in source_report["houses"])
        if food_scenario:
            if checkpoint["tag"] == "mid-food-courier":
                assert any(c["role"] == 13 and c["food_cargo"] > 0 and c["edge_progress"] > 0 for c in source_report["couriers"])
            elif checkpoint["tag"] == "mixed-food-courier":
                assert any(all(n > 0 for n in trip["cargo"]) for trip in source_report["food"]["trips"])
            elif checkpoint["tag"] == "food-return":
                assert any(trip["phase"] == 2 and sum(trip["cargo"]) > 0 for trip in source_report["food"]["trips"])
            else:
                assert any(row["quality"] == 80 and row["quality_witness"] for row in source_report["food"]["stocks"])
        if services_scenario and not ancestor_scenario and not religion_scenario and not culture_scenario:
            if checkpoint["tag"] == "mid-services-couriers":
                assert len(source_report["services"]["trips"]) == 2 and not source_report["services"]["witnesses"]
                assert sum(c["role"] in (5, 11) and c["phase"] == 1 and c["edge_progress"] > 0
                           for c in source_report["couriers"]) == 2
            elif checkpoint["tag"] == "house-medical-visits":
                check_services(source_report, positive=True)
            elif checkpoint["tag"] == "service-before-expiry":
                assert any(h["service_timers"][0] == 1 for h in source_report["houses"])
            else:
                assert all(h["service_timers"][0] == 0 for h in source_report["houses"])
        if ancestor_scenario:
            if checkpoint["tag"] in ("mid-ancestor-courier", "ancestor-road-disconnected"):
                assert all(h["service_timers"][5] == 0 for h in source_report["houses"])
                assert any(c["role"] == 15 and c["phase"] == 1 for c in source_report["couriers"])
            elif checkpoint["tag"] in ("house-ancestor-visit", "ancestor-revisited"):
                check_services(source_report, ancestor=True)
            elif checkpoint["tag"] == "ancestor-before-expiry":
                assert any(h["service_timers"][5] == 1 for h in source_report["houses"])
            else:
                assert all(h["service_timers"][5] == 0 for h in source_report["houses"])
        if housing_scenario:
            identity = housing_fixture["home_id"]
            home = next(h for h in source_report["houses"] if h["id"] == identity)
            tag = checkpoint["tag"]
            if tag == "mid-water-courier":
                assert any(c["role"] == 16 and c["phase"] == 1 and c["edge_progress"] > 0 for c in source_report["couriers"])
            elif tag == "water-arrived-pre-evaluation":
                assert home["water"] is False and any(h["current_witness"] for h in source_report["water"]["houses"])
            elif tag == "house-hut-upgrade":
                assert home["type"] == 4 and home["water"] is True
            elif tag == "house-plain-upgrade":
                assert home["type"] == 5
            elif tag == "house-devolution-counter9":
                assert home["type"] == 5 and home["devolve_counter"] == 9
            else:
                assert home["type"] == 4 and home["devolve_counter"] == 0
        if religion_scenario:
            identity = religion_fixture["home_id"]
            home = next(h for h in source_report["houses"] if h["id"] == identity)
            tag = checkpoint["tag"]
            if tag in ("mid-religion-couriers", "religion-road-disconnected"):
                assert all(home["service_timers"][r] == 0 for r in RELIGION_ROLES)
                assert sum(c["role"] in (17, 18, 19) and c["phase"] == 1 for c in source_report["couriers"]) == 3
            elif tag in ("house-three-religions", "religion-revisited"):
                check_services(source_report, religions=True)
                assert all(home["service_timers"][r] > 0 for r in RELIGION_ROLES)
            elif tag == "religion-before-expiry":
                assert any(home["service_timers"][r] == 1 for r in RELIGION_ROLES)
            else:
                assert all(home["service_timers"][r] == 0 for r in RELIGION_ROLES)
        if culture_scenario:
            identity = culture_fixture["home_id"]
            home = next(h for h in source_report["houses"] if h["id"] == identity)
            tag = checkpoint["tag"]
            if tag in ("mid-culture-couriers", "culture-road-disconnected"):
                assert all(home["service_timers"][r] == 0 for r in CULTURE_ROLES)
                assert sum(c["role"] in (20, 21, 22) and c["phase"] == 1 for c in source_report["couriers"]) == 3
            elif tag in ("house-three-cultures", "culture-revisited"):
                check_services(source_report, culture=True)
                assert all(home["service_timers"][r] > 0 for r in CULTURE_ROLES)
            elif tag == "culture-before-expiry":
                assert any(home["service_timers"][r] == 1 for r in CULTURE_ROLES)
            else:
                assert all(home["service_timers"][r] == 0 for r in CULTURE_ROLES)
        folder = output / ("resume-" + checkpoint["tag"])
        legacy.run(binary, ["--resume", replay_data, folder, source], output / (checkpoint["tag"] + "-resume.log"))
        actual_reports = []
        for name, expected_key, report_key in (
                ("midpoint", "expected_midpoint", "expected_midpoint_report"),
                ("result", "expected_result", "expected_report")):
            actual = folder / (name + ".json")
            expected = checked_path(repo, output, checkpoint[expected_key])
            assert actual.read_bytes() == expected.read_bytes(), "Canonical new-process replay bytes differ"
            assert json.loads(actual.read_text()) == json.loads(expected.read_text()), "Full SaveDocument differs"
            actual_report = folder / (name + "-report.json")
            expected_report = checked_path(repo, output, checkpoint[report_key])
            assert actual_report.read_bytes() == expected_report.read_bytes(), "Canonical new-process report bytes differ"
            report = json.loads(actual_report.read_text())
            assert report == json.loads(expected_report.read_text())
            check_balances(report, positive=not food_scenario and not services_scenario and not housing_scenario and not industry_scenario and not stages_scenario)
            assert report["model_sha256"] == model_sha
            if authored_needs:
                assert report["shelter_needs"] == [1, 1]
                assert all(report["balances"][good]["consumed"] > 0 for good in (0, 1))
            else:
                assert report["shelter_needs"] == [0, 0]
                assert all(not any(row["consumed"]) for house in report["housing"]["houses"] for row in house["consumption_rows"][:3])
            saved = json.loads(actual.read_text())
            assert saved["schema_version"] == 26 and saved["rules"]["version"] == 10
            assert saved["world"]["original_production"]["development_revision"] == DEVELOPMENT_REVISION
            assert "service_availability" not in saved["world"]["original_production"]
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
            "authored_model_needs": authored_needs, "food_scenario": food_scenario, "services_scenario": services_scenario,
            "ancestor_scenario": ancestor_scenario, "housing_scenario": housing_scenario, "religion_scenario": religion_scenario, "culture_scenario": culture_scenario, "industry_scenario": industry_scenario, "stages_scenario": stages_scenario, "model_sha256": model_sha,
            "ticks_advanced": 20000, "periods": [10000, 10000], "actual_paid_road_continuation": True,
            "full_savedocument_equal": True, "canonical_bytes_equal": True, "provenance_equal": True,
            "balances_equal": True, "midpoint_equal": True, "source_save_and_metadata_unchanged": True})
    old = legacy.compare_legacy(baseline, binaries["openemperor-original-goods-fixture"],
        binaries["openemperor-original-influence-fixture"], stage, output)
    old.update(compare_rule9(repo, reference9, binaries["openemperor-original-goods-fixture"], stage, output))
    after = inventory(original)
    assert before == after, "Original installation bytes or metadata changed"
    assert private_before == inventory(stage), "Private hash-identical model copy or map changed"
    assert authored_before == inventory(authored_stage), "Private explicitly authored model or map changed during simulation"
    assert all(before == inventory(root) for root, before in housing_negative_inventories), "Private authored housing copies changed"
    assert all(before == inventory(root) for root, before in religion_private_inventories), "Private authored religion copies changed"
    assert all(before == inventory(root) for root, before in culture_private_inventories), "Private authored culture copies changed"
    legacy.frozen_baseline(repo)
    frozen_rule9(repo)
    assert sum(row["culture_scenario"] for row in restarts) == 6
    assert sum(row["religion_scenario"] for row in restarts) == 6
    assert sum(row["industry_scenario"] for row in restarts) == 7
    assert len(restarts) == 50 and sum(row["stages_scenario"] for row in restarts) == 7 and sum(row["ancestor_scenario"] for row in restarts) == sum(row["housing_scenario"] for row in restarts) == 6
    report = {"status": "PASS", "evidence": "Actual paid four-luxury and five-Aux chains, native Clay/Hemp inputs, real delivered Paper Academy gate and all43 prior scenario tags plus7new full20k process cases under the explicit active25 contract",
        "original_data": str(original), "original_inventory_before": before, "original_inventory_after": after,
        "original_unchanged": True, "private_source_inventory_unchanged": True,
        "original_model_sha256": MODEL_SHA, "model_copy_byte_identical": True,
        "fixture_binary_sha256": digest(binary), "snapshot_injection": False, "positive_runtime_goods": True,
        "positive_runtime_food": True, "actual_food_quality_classes": [20, 80],
        "positive_authored_runtime_consumption": True, "original_shelter_consumption_zero": True,
        "authored_model_sha256": authored_sha, "authored_model_changes": {"row": "Shelter", "fields": [9, 10], "before": [0, 0], "after": [1, 1], "changed_bytes": 2},
        "authored_private_inventory_unchanged": True,
        "fixture": fixture, "consumption_fixture": consumption, "food_fixture": food_fixture,
        "services_fixture": services_fixture, "positive_runtime_services": True,
        "ancestor_fixture": ancestor_fixture, "positive_runtime_ancestor": True,
        "housing_fixture": housing_fixture, "positive_runtime_water": True, "positive_runtime_housing": True,
        "religion_fixture": religion_fixture, "positive_authored_runtime_religion": True, "original_confucian_resource_gate": True,
        "industry_fixture": industry_fixture, "positive_runtime_four_luxury_goods": True,
        "stages_fixture": stages_fixture, "positive_runtime_housing_stages": True,
        "culture_fixture": culture_fixture, "positive_authored_runtime_culture": True, "original_venue_performer_loop": False,
        "restarts": restarts, "legacy": old,
        "frozen_baseline_capture_sha256": digest(baseline / "capture.json"),
        "frozen_baseline_head": capture["head_before_implementation"],
        "limits": ["Actual paid Food/Hemp/Ceramics delivery; original Shelter six-Goods needs0 consume0; Food sink is separate authored rule",
            "All fifteen house rows are diagnosed; paid Common>5 and Elite11->13->12->13 are actual, high tiers remain separately observed",
            "Need1 and stricter Desirability counterexamples use explicitly authored copies; original Plain Hemp/Ceramics requirements are zero",
            "Actual stage decisions use immutable pure formulas and bounded real transition inputs; Elite geometry is temporarily authored2x2 until26",
            "Experimental targeted School-to-House visitors use actual paid bodies and direct96 effects; Original venue/performer/Actor36 loop is not reconstructed and receives no free state",
            "Actual Plain culture needs0; single-Need1 controls change only copied fields5/6/7",
            "Availability diagnosis reads existing prepared state and cannot grant current or future access",
            "Academy now consumes actual delivered Paper per new trip; production timing and managed ore are authored, no resource grant",
            "Original Plain religion needs0; OR controls change field23 and Common-Conf counterexamples additionallyfield22; actual Conf visits are Elite-only",
            "Ancestor elite_only=false and targeted ordinary tours are explicit authored integration, no original spawn cadence or random-walk parity claim",
            "Migration, funding, Food crop scores20/80 and recipe timing are authored; no original production parity claim",
            "Authored geography; separate SDL/native/human acceptance"]}
    (output / "report.json").write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps({"status": "PASS", "report": str(output / "report.json"),
        "separate_process_replays": len(restarts), "positive_runtime_goods": True, "original_unchanged": True}))


if __name__ == "__main__":
    main()
