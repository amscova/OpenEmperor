# OE-34: paid city progression

**Overall: PARTIAL. The requested paid housing goal is reached (PASS), and a
bounded supply window is demonstrated. Continuous fire-safe goal retention and
financial self-sufficiency remain unproven.** Native/human acceptance is NOT_RUN.

Baseline is main `d81c5015c61e`; work is isolated on
`feature/oe34-paid-city-progression`. The experiment uses the real
`Cities/Xia.map`, the installation's unchanged original model, Difficulty Normal,
and the existing paid four-house starter. City-v16 Rule10 / Policy3 / Schema26 /
development revision27 and every historical contract remain unchanged. No costs,
taxes, recipes, speeds, requirements, budgets or simulation authority are altered.
See [architecture](architecture-map.md) and [the play guide](player-handoff.md).

## Reproduce

With the project's CMake, SDL3, OpenSSL and JSON prerequisites installed:

```sh
cmake -S . -B .local/original-economy/oe34/build -DCMAKE_BUILD_TYPE=Release -DOPENEMPEROR_USE_SYSTEM_SDL3=ON
cmake --build .local/original-economy/oe34/build --parallel 2 --target openemperor-paid-progression-acceptance
python3 -B tools/test_paid_progression.py --binary "$PWD/.local/original-economy/oe34/build/openemperor-paid-progression-acceptance" --data "$PWD/.local/gog-extracted/app" --output "$PWD/.local/original-economy/oe34/paid-acceptance" --plan supplied --hold 20000
```

Use your own explicit installation path. Output must be a **new** directory under
`.local/original-economy/oe34/`, disjoint from original data, resources and the build.
Existing output and symlink paths are refused. Resources default to the executable's
sibling `resources` directory; `--resources` selects an explicit prepared directory.
The first-goal bound defaults to 20000 ticks; the hold starts at the actual first
successful goal. No original saves or personal save directories are searched.

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
productive ticks, at most 10.2 units per 816 ticks. Six occupied Houses consume
12 per 816 ticks. A second legal Farm/Field addresses that measured capacity gap;
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

## Validation evidence

The independent private plan probes above completed against production World/map
and model preparation. Their diagnostics live under ignored
`.local/original-economy/oe34/probe/`; original bytes are read-only.

The maintained runner completed in `.local/original-economy/oe34/acceptance-01/`:

- Fresh creator reaches the goal at 816 and runs through 20816; a separate process
  restores the goal save and independently continues for the same 20000 ticks.
  Both the entire goal/loaded documents and entire final documents are byte-identical.
  Every checkpoint also passes production SaveDocument read/restore equality.
- A third process loads the final state through actual SandboxView F9 and saves
  through F5, preserving the complete literal document. Its actual software-renderer
  captures at 1×/2×/4× were opened and inspected: six developed Houses, live walkers,
  roads and the mission-met HUD are visible. Existing non-House/field fallbacks remain;
  the close zooms are framing/detail checks, not whole-scene or native visual parity.
- The observer records **2436 actual evaluations**, 28 upgrades/seven devolutions,
 1184 actual transfer-arrival observations, 1495 applied Water/service visits,
  zero transfer cancellations and 23 returned-transfer observations. Returned cargo
  remains conserved; a blocked dispatch is not mislabeled a failed delivery.
- All 1464 original files / 819711091 bytes retain SHA256, mode and mtime.
  Executable and recorded source inventories are unchanged across the workflow.
  `acceptance.json` SHA256 is
  `979920119a5cbb218e87eea75bb24c9eb58f9893bb6ac6fcc499d97138517b2f`.
- Fresh Release build and **248/248 CTests pass, zero skips**, including
  `original-paid-progression-real` (actual paid goal, strict funded FireWatch refusal,
  natural fire and temporary live-goal loss) and the 19 runner guards. Relevant existing
  starter, goods, housing, finance, save/recovery, map and visual tests remain green.

Additional real Sandbox renders of the goal and first-fire saves were opened at 1×
under `.local/original-economy/oe34/goal-captures-01/` and `fire-captures-01/`:
the HUD changes from mission met at 816 to zero eligible Houses at 2000, with actual
fire sprites. Reproduce either with the same binary's `--mode render --input SAVE
--data DATA --output NEW_OE34_DIRECTORY --resources BUILD/resources`.
The fixed continuation contains 4800 fire-related lost-goal ticks; its longest
continuous goal run is 1980 ticks. Final Funds 290 are positive but the treasury
continues to decline. These gameplay limits retain overall PARTIAL even though
all technical verification passed. Native delivery and human acceptance remain
NOT_RUN. The deleted private OE00 historical reference-save corpus was not recreated
or claimed rerun; immutable locks remain unchanged, and existing legacy/rule-gate
CTests are the compatibility evidence executed here. No push, merge, tag or release.
