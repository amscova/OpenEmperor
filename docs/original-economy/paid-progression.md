# OE-34: paid city progression

**Housing acceptance: PASS, from the actual
`World::original_goal_status().reached`. Long-term stability: PARTIAL.** The
measured plan loses the goal during fires and spends more upkeep than it collects
in taxes. A finite supply window does not establish a sustainable economy.
Native/human acceptance is NOT_RUN.

Baseline is current main `9265ebc32c09`; work is isolated on
`feat/oe34-paid-progression`. The experiment uses the real
`Cities/Xia.map`, the installation's unchanged original model, Difficulty Normal,
and the existing paid four-house starter. City-v16 Rule10 / Policy3 / Schema26 /
development revision27 and every historical contract remain unchanged. No costs,
taxes, recipes, speeds, requirements, budgets or simulation authority are altered.
See [architecture](architecture-map.md) and [the play guide](player-handoff.md).

## Reproduce

With the project's CMake, SDL3, OpenSSL and JSON prerequisites installed:

```sh
cmake -S . -B .local/original-economy/oe34/build -DCMAKE_BUILD_TYPE=Release -DOPENEMPEROR_USE_SYSTEM_SDL3=ON
cmake --build .local/original-economy/oe34/build --parallel 2 --target openemperor-original-longplay-acceptance
python3 -B tools/test_original_longplay.py --binary "$PWD/.local/original-economy/oe34/build/openemperor-original-longplay-acceptance" --data "$PWD/.local/gog-extracted/app" --output "$PWD/.local/original-economy/oe34/paid-longplay" --scenario progression --ticks 20000
```

Use your own explicit installation path. Output must be a **new** directory under
`.local/original-economy/oe34/`, disjoint from original data, resources and the build.
Existing output and symlink paths are refused. The new `progression` scenario uses
the existing Longplay executable/runner; historical `starter` and `districts`
retain their behavior. The paid strategy is shared with the existing standalone
View/capture fixture, outside production simulation. The first-goal search is
bounded to 20000 ticks; `--ticks` requires at least 20000 **additional** ticks from
the first true production goal. Neither C++ nor Python reconstructs that goal
from its constituent thresholds. No original saves or personal save directories
are searched.

## Paid construction recipe

The unchanged starter buys 87 commands, 68 roads and 19 bodies for 1404 Funds out of
12000. All residents, stock and elapsed time begin at zero. Its real Xia origin is
**(102,120)**. Add these purchases while still paused at tick 0; field commands
explicitly select their newly purchased farm. These are ordinary World commands,
with normal footprint, source protection, road, price and available-Funds checks.

| Purchase | Offset from starter | Absolute cells | Cost |
|---|---|---|---:|
| Two Common Houses | (12,2), (15,2) | (114,122), (117,122) | 160 |
| Four connecting roads | x2, y5–8 | x104, y125–128 | 8 |
| Second Food Farm, 3×3 | (9,10) | (111,130) | 50 |
| Its Millet Field | (12,10) | (114,130) | 4 |
| Second Hemp Farm, 2×2 | (14,10) | (116,130) | 40 |
| Its Hemp Field | (16,10) | (118,130) | 4 |
| **Additional / complete construction** | | | **266 / 1670** |

The complete city has 25 bodies, 72 paid roads and 97 construction commands.
After purchase, Funds are 10330. No operation/priority changes are required.
Unpause normally; immigration, staffing, harvests and walker arrivals earn all
population, goods, Water and service access. No restored snapshot creates the
positive creator city, and no goods, timers, residents or treasury are assigned.

## Actual progression and supply

The first live goal occurs at **tick 816**: six occupied non-burning Houses,
**108 residents, 9870 Funds and 80 collected taxes**, after 540 upkeep. The House
states are Plain Cottage 5, Plain Cottage 5, Attractive Cottage 6, Plain Cottage 5,
Hut 4 and Hut 4. Population and the six-house count do not imply that every House
has already received Food or every later-stage good.

The 20000-tick continuation ends at **tick 20816** with 220 residents, 290 Funds,
4000 collected taxes and 14040 upkeep. Houses 1,2,3,4,20,21 are respectively
**Plain Cottage 5, Spacious Dwelling 7, Elegant Dwelling 8, Elegant Dwelling 8,
Attractive Cottage 6 and Plain Cottage 5**, with 22,41,52,52,31,22 residents.
All 135 installed workers can be staffed when their bodies are not burning.

| Checkpoint | Tick | Residents | Funds | Collected tax | Upkeep |
|---|---:|---:|---:|---:|---:|
| All purchases, paused | 0 | 0 | 10330 | 0 | 0 |
| First actual World goal | 816 | 108 | 9870 | 80 | 540 |
| Goal +10000 ticks | 10816 | 220 | 4840 | 1800 | 7290 |
| Goal +20000 ticks | 20816 | 220 | 290 | 4000 | 14040 |

In the additional interval alone, actual tax is **3920** against **13500** upkeep:
**−9580 Funds**, with tax covering **29.04%** of upkeep. The positive final Funds
come from the initial treasury, not an operating surplus. This is a failure of
self-financing for this fixed plan, not proof that every legal strategy fails.

At every actual phase39 House evaluation, compare the recorded pre-evolution
input with its current and next model rows independently. Here “current needs
met” means the pure `needs_met` predicate over 15 Water/service/FoodQuality/goods
fields. DES thresholds, fire, evolution blocking and operational availability are
separate gates; this predicate alone does not prove a functioning non-burning city.
Record Food quantities,
FoodQuality, six goods, service timers/witnesses, staffing before the tick, physical
production, upkeep, movement/arrival times and actual upgrades/devolutions.
Future visits, stock elsewhere and a planned route never satisfy a House need.

The supplied plan has 11 actual Food misses: six at 397, two at 856, one at 1213,
one at 2029 and one at 2488. There are **no later Food misses through 20816**.
After tick 5000 there are 11 current-needs failures across 1860 House evaluations:
two FoodQuality and nine Hemp. From **6313 through 20797, all six current rows
are satisfied at every assessment: 285 sets / 1710 individual House evaluations**.
This is a finite needs-predicate observation, including stored supplies across
fire interruptions; burning bodies are not thereby operational.

After 5000, 631 evaluations fail next-row needs, including 620 Acrobat requirements.
Those are upgrade blockers, separate from current supply failures. No Acrobat
service is manufactured to make them pass. Total progression records 28 upgrades
and seven devolutions; native/human and highest-tier progression remain open.

The observer splits warmup (0→816) from the requested continuation (816→20816).
Warmup contains 84 individual evaluations and 11 upgrades; the continuation
contains **2352 evaluations, 17 upgrades, seven devolutions, 126 current-row
failures and 911 next-row failures**. Five Food misses occur after the goal; the
other six occur during warmup. The continuation records **1151 successful
transfer-arrival observations, 971 applied service/religion visits and 481 Water
visits**, zero transfer cancellations and 23 returned-transfer observations.
An already moving leg at the boundary is retained; it is not counted as a newly
dispatched complete trip. The detailed transport records preserve dispatch,
arrival, return and elapsed-time evidence; status counts distinguish unstaffed,
burning, empty-source and full-target delays from actual failed delivery.

## Measured bottlenecks and counterfactual plans

Each plan starts afresh from the same real starter and runs through 20816.
The comparison uses actual arrivals and House evaluation inputs, not HUD shortages.

| Expansion | Paid total | Food misses | Current failures after 5000 /1860 | End Funds |
|---|---:|---:|---:|---:|
| `direct`: two Houses | 1564 | 125 | 669 | 4176 |
| `connected`: Houses + four roads | 1572 | 112 | 895 | 4352 |
| `food`: connected + Food Farm/Field | 1626 | 11 | 158 | 2206 |
| `supplied`: food + Hemp Farm/Field | 1670 | 11 | 11 | 290 |

The original connector lies at x20. Four paid roads at x2 shorten the first
Food Farm→Market actual one-way arrival from **179 to 69 ticks**. They change
travel, not harvest capacity: one fully staffed Farm produces one unit per 80
productive ticks, a maximum sustained average of 10.2 units per 816 ticks.
Six occupied Houses consume 12 per 816 ticks. A second legal Farm/Field addresses that measured capacity gap;
a Farm accepts only one separately purchased adjacent field.

Remaining current failures in the `food` plan are predominantly Hemp: 156 after
5000, alongside two FoodQuality failures. The second paid Hemp chain reduces
these to nine, all before 6313. Global stock alone does not prove local coverage:
the one-Hemp-Farm run ends with 195 produced, 162 consumed, 30 in stock and three
in cargo while an individual House still has zero. Other Houses hold stock,
including Plain Cottages whose current row needs none; ordinary transport rotates
among Houses. Production, distribution, buffers and fire interruptions must be
observed together. This evidence does not establish a navigator defect.

## Unchanged blockers

At **tick 2000**, all 21 eligible bodies of the supplied city ignite. They remain
burning until 2600, so all six Houses temporarily stop counting toward the live
goal. The next incident starts 4580. Ordinary Rule10 `PlaceFireWatch` is rejected
with “Original production permits only its explicitly implemented paid chains”.
The refusal preserves the complete snapshot, IDs, Funds and command sequence.
The regression must retain both natural ignition and this atomic refusal; allowing
fire protection requires a separately authorized versioned contract. Full current
needs during later fires do not make burning Houses count toward the goal.

The fixed city also spends more on recurring upkeep than it earns in taxes.
The actual ledger is **12000 −1670 +4000 −14040 =290** at 20816. This bounded
positive balance is not profit or evidence of indefinitely sustainable finance.
No price, tax, staffing requirement or demand is changed to hide that limit.

**OE-35 proposal, not implemented or authorized here:** specify a separately
selected versioned contract for paid fire protection, including source evidence,
construction/restore authority, staffing, visits, effectiveness and historical
compatibility. The first retention blocker is missing purchasable infrastructure,
not evidence of a routing defect. Re-measure operating income and upkeep with that
infrastructure before proposing any separately approved balance changes. No Rule10
value, risk, price, demand, recipe, tax or schema changes are part of OE-34.

## Validation evidence

The current canonical Release workflow is
`.local/original-economy/oe34/release-longplay-progression-02/`:

- Three distinct actual processes create the goal seed, buy and grow a second
  fresh unrestored city, and restore/continue the seed. Both continuations simulate
  20000 ticks after 816. The complete freshly earned goal document equals the seed
  byte for byte. Six midpoint/final SaveDocument, snapshot and report pairs match;
  every checkpoint also passes production read/restore equality.
- Eight additional midpoint/final evaluation, evaluation-frame, transport and
  detailed-metric files match byte for byte. The runner refuses missing or
  divergent diagnostics even if all children exit successfully. It independently
  checks raw model, paid accounting, population/workforce, inventories, cargo and
  reservations, and compares observed arrival diagnostics between processes,
  without a second housing-goal formula.
- The report separates `goal_status: PASS` from `stability_status: PARTIAL`.
  Its SHA256 is
  `0da4ea23cc5b347781be1ae7a6e721cd4eb5f28aca41c376df0fd5adc16878c4`.
- All 1464 original files / 819711091 bytes retain SHA256, mode and mtime. The
  executable and supplied goal-save bytes remain unchanged. Original Xia SHA256 is
  `871266c415d7500ff3cf3fe70fc6b36caecc4d4cbf09d4d737849e4060906433`;
  original model SHA256 is
  `7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6`.
- **Release: 248/248 CTests pass, zero skips**, including
  `original-paid-progression-real`: actual productive goal, strict funded
  FireWatch refusal, natural fire and temporary live-goal loss. The updated runner
  guard suites contain 25 Longplay and 19 standalone tests. Existing starter,
  goods, housing, finance, recovery, map, legacy and rendering tests pass.

| Configuration | Executed CTests | Result |
|---|---:|---|
| Release | Complete registered suite: 248 | PASS, zero skips |
| Debug | 40 economy/save/rule invariants + two Longplay replay/budget tests + two runner suites | 44/44 PASS, zero skips |
| RelWithDebInfo + ASan/UBSan | Same relevant selection | 44/44 PASS, zero skips; no sanitizer finding |

Sanitizers use the maintained alpha-pipeline configuration: `-O2 -g`,
`OPENEMPEROR_ENABLE_SANITIZERS=ON`, `-fsanitize=address,undefined`,
`ASAN_OPTIONS=detect_leaks=0:halt_on_error=1` and
`UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`. Plain Debug remains a separate
configuration. Logs/JUnit are under the private OE34 root:
`release-ctest.log` / `release-ctest.xml`,
`debug-ctest-invariants.log` / `debug-invariants.junit.xml`,
`debug-ctest-longplay.log` / `debug-longplay.junit.xml`, and
`sanitize-ctest-invariants.log` / `sanitize-invariants.junit.xml`.
`selected-44-test-names.txt` records the exact focused selection: paid starter,
rules1–10, scheduler, goods/production, Food/Hemp, services/religion, housing,
finance, adversarial saves, authored frozen references and real replay/budgets.
The unchanged historical `starter` scenario also passes a 200-tick continuation
after its existing 2000-tick warmup, with six literal process pairs.

The full goal-plus-20000-tick, three-process workflow also completes in
`debug-longplay-progression-02/` and `sanitize-longplay-progression-01/`, with the
same six complete-state/report pairs and eight detailed-diagnostic pairs each.
`cross-config-comparison.json` verifies all three completed workflows and all
**28 cross-configuration file pairs** against Release: complete saves, snapshots,
reports, evaluation inputs/frames and transport diagnostics are byte-identical.
All configurations reproduce the same productive goal at 816, final state at
20816, 4800 goal-loss ticks and −9580 operating balance. The observer is not a
performance benchmark. Superseded interrupted diagnostic runs are retained
privately and are not counted as completed acceptance runs.

The current Longplay's goal and final saves also passed actual SandboxView F9/F5
literal-save equality. All six 1×/2×/4× software-renderer captures were opened and
inspected under `longplay-goal-captures-01/` and `longplay-final-captures-01/`:
the 1× views show six Houses, actual walkers, roads and the mission-met HUD at
816/20816, including the later Spacious/Elegant Dwellings and 290 final Funds.
Existing non-House/field fallbacks remain visible; the closer zooms are framing
and detail checks, not full-scene or original visual parity. The render audit
`longplay-render-audit.json` confirms literal F5/F9 bytes and unchanged original
inventory. These are scripted SDL/software checks, not native/human acceptance.
Reproduce with `openemperor-paid-progression-acceptance --mode render --input
SAVE --data DATA --output NEW_OE34_DIRECTORY --resources BUILD/resources` after
building that existing fixture.

Earlier standalone evidence remains under ignored `acceptance-01/` and `probe/`;
the independent counterfactual plans above are from that experiment. Previously
opened goal/fire captures under `goal-captures-01/` and `fire-captures-01/` show
the HUD change at 816/2000 and actual fire sprites.

The current continuation independently reproduces eight fire interruptions,
**4800 lost-goal ticks** and a longest continuous goal run of **1980 ticks**.
Native delivery, human acceptance and highest-tier paid progression remain open.
The deleted private OE00 historical reference-save corpus was not recreated or
claimed rerun; immutable locks remain unchanged, and existing legacy/rule-gate
CTests provide the executed compatibility evidence. No push, merge, tag or release.
