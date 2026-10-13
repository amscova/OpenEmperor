# City-v16 population, workforce and long-play stability

The paid rule-4 original-map runs reach the complete live goal on **Xia at tick 7,200** and **Handan at tick 6,800**. Both hold it at every tick for another **10,000 ticks**, end with **148 residents / eight effective Level-2 Houses**, and earn **14,000 actual tax − 2,400 actual maintenance = 11,600 net Funds** in that fixed-construction window. All 250 actual House demands in each window succeed.

The player's reported **60/100** state remains **UNRESOLVED**: no explicitly authorized player SaveDocument was supplied. Its reported capacities and upkeep nevertheless exclude a simple housing-space or current workforce-count shortage, conditional on those values being accurate. A separate normally paid 18-House reconstruction demonstrates declining population with growing Funds, and recovers with one ordinary Service Post resume command. This does not establish the cause of the player's state.

No confirmed engine bug was found. Production rules, costs, capacities, timing, priorities, maintenance, save versions, map permissions, rendering and input remain unchanged. This is OpenEmperor's authored economy, with no original Emperor balance/parity claim.

## Baseline and evidence

Starting HEAD: `a852fb443c17ca0342901cbaadc9baab16cf3579`, initially clean. The requested reference matches the actual HEAD. All changes in this pass are on-demand tools, their shared recipe/CMake target and documentation. No commit, publication or package was created.

Pinned original-data revision: 1,464 files / 819,711,091 bytes, manifest SHA-256 `dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`. EXE SHA-256 `6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e`; Model SHA-256 `7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6`. Complete input inventories are unchanged before/after the runs.

| Evidence | Status and scope |
| --- | --- |
| PLAYER SCREENSHOT / OBSERVATION | Values quoted in the milestone: rule4/policy2/schema20; tick13,067, Funds2,259, lifetime taxes6,925, installed upkeep98/400t; 18 Houses, eight effective L2, population60; current missing Service8/Pottery11. These are unverified reported observations, not a parsed baseline |
| ACTUAL PLAYER SAVE / scenario A | **NOT RUN**. No authorized file; no personal save collection searched |
| RECONSTRUCTED SCENARIO / B | Actual paid Xia city, eight extra paid Houses/roads, deliberately paused existing Post, actual simulation; explicitly different from the player's exact tick/state/layout |
| SCRIPTED APP EVIDENCE | Production MapSession, SandboxView, World, save reader/writer/restore and direct synthetic F5/F9 handlers; separate OS fixture processes. No normal Application event-pump or native keyboard assertion |
| Fresh original-map scenario C | Actual Xia and Handan with real prepared original-map authority, rule4/policy2/schema20 and ordinary paid commands |
| NATIVE HUMAN ACCEPTANCE | **NOT RUN** in this milestone. Complete manual construction/input/full-city play remains open |

## Population and the reported 60 residents

The authoritative `World` capacities are effective Level0/1/2 **6/10/16**. Historical level comes from cumulative successful demands: two successes establish L1, five L2. Water and desirability cap the effective level; a later miss does not erase successful-demand history. An effective L2 House can consequently have few residents after repeated supply misses.

At each placement-relative 400-tick deadline, the existing code evaluates actual stocked Food/Pottery, Service expiry and sickness/fire. A success consumes one of each required good and pays actual tax. For these profiles, over-capacity population declines by one; otherwise a miss outside the 800-tick move-in grace declines by one above the minimum two residents, and a success below capacity grows by one. No population update is inferred from frames or a current Inspector warning. Paid House placement's ordinary six starting residents are retained; no residents or goods are injected by the harness.

For 18 Houses and eight **currently effective** L2 Houses, the minimum capacity is `8×16 + 10×6 = 188`, leaving at least 128 vacant places at population60. Thus the reported state is not housing-capacity limited. Exact per-House populations, water, caps, demand history and the reasons for Service8/Pottery11 cannot be recovered from these aggregate values.

Current `household_demand_status()` describes readiness **now**, rather than the next demand's result. Pottery0 immediately after successful consumption can be normal while another delivery is underway. Neither “missing11” nor “missing8” is a worker count or proof of eleven/eight failed deadlines. The actual event trace uses pre-demand stocks/expiry and the resulting fulfilled/missed, consumed, tax and population counter deltas; arrivals occur later in the same tick and are kept separate.

## Workforce and feedback

Residents supply actual workers. Allocation considers enabled, nonburning buildings in **High → Normal → Low → stable Building ID** order, grants complete requirements only, and skips a request that does not fit. Paused/burning owners cannot start new production/dispatch; existing trips and inbound deliveries retain ordinary semantics. Sickness does not remove House residents from workforce supply. The private allocation implementation is observed through existing public `workers_assigned()` queries; no new engine API or replacement allocator was added.

Each upkeep-bearing staffed kind has maintenance exactly twice its workers: Clay8/4, Pottery12/6, Warehouse4/2, Farm8/4, Service4/2, Market8/4, Watch4/2 and Health4/2. Wells add2 upkeep and zero workers. Therefore reported upkeep98 implies installed workers `49 − WellCount`, at most49. With reported population60 there is no current numerical staffing shortage, if those observations correspond to one valid rule4 state. Pause, fire, timing or routes can still prevent supply; their exact occurrence in the player's state is unknown.

All 44 installed/active workers are assigned in the fresh fixed windows. In the reconstruction one paused Service Post lowers active need to42; all42 remain assigned, even as population falls to62 and later a minimum51. **Zero observed staffing-shortfall ticks** occur. The population→unstaffed-production→more misses feedback loop is possible under the existing rules below active demand, but is **not observed** in these runs and does not explain the conditional screenshot staffing bound.

Five legal High-priority commands on Farm/Markets/Posts leave the paused Post paused. Through the first10,000 ticks their actual demands, population62 and Funds23,898 match the no-intervention control. Priorities cannot reactivate a paused building and have no allocation benefit here because all active jobs already fit.

## Reconstructed under-supply and minimal recovery

The earned Xia first-goal SaveDocument is restored. Eight Houses and their legal connections are normally purchased: **640 House cost + 60 new-road cost = 700 Funds**, total lifetime construction3,480. The layout planner only uses real buildability/validation/routing; all selected roads/buildings then execute through normal paid commands. New House starting residents are the ordinary rule, not restored/invented population. Existing Service Post **ID16** is deliberately paused through a regular operation command. The saved state has18 Houses/population177 and capacity196; historical development later raises capacity252/effectiveL2 count12 independently of falling residents.

| Branch / interval | Population | Actual tax | Actual upkeep | New construction | Net Funds |
| --- | --- | ---: | ---: | ---: | ---: |
| Reconstruction, tick7,200→17,200 | 177→104 | 9,380 | 2,400 | 0 | +6,980 |
| Same city, tick17,200→27,200 | 104→62 | 9,500 | 2,400 | 0 | +7,100 |
| B control, tick27,200→41,200 | 62→52, min51/max70 | 13,720 | 3,360 | 0 | +10,360 |
| B priority-only, tick27,200→37,200 | 62→62 | 10,140 | 2,400 | 0 | +7,740 |
| B minimal recovery, tick27,200→31,200 | 62→113 | 6,120 | 960 | 0 | +5,160 |
| Same recovered city, fixed tick31,200→41,200 | 113→235 | 18,780 | 2,400 | 0 | +16,380 |

The reconstruction's second10,000 ticks have450 actual deadlines,181 successes and269 misses, all solely **MISSING_SERVICE**. Food/Pottery are stocked, all active workers fit, and all Houses retain valid Market/Service routes. Positive money therefore coexists with a demographic crisis; it is not a whole-city health PASS. The longer14,000-tick no-intervention control has630 demands,255 successes/375 Service-only misses.

Recovery accepts exactly **one** `SetBuildingOperation(ID16, true)` at tick27,200, cost0. Assigned/active workers rise42→44 with no changed installed upkeep96/400t. No priority change is necessary in this branch, no producer is purchased, and no goods/population/funds are granted. After4,000 ticks population113 already satisfies the full goal; a further10,000 ticks without commands retains the goal every tick and ends at235, capacity252, effectiveL2 count12, Funds37,698. This window still has351/450 successful demands and99 Service-only misses; the goal is stable, not universal perfect supply.

One concrete remaining miss is House8 at tick31,600: input Food8/Pottery7, valid road routes, no active fire/sickness, Service expiry30,915, population6→5, no goods consumed or tax. Its preceding actual Service arrival is29,715 and the next is31,625, twenty-five ticks after demand. The gap exceeds the existing1,200-tick coverage. This is **SERVICE COVERAGE / DEMAND TIMING** in this18-House route layout, not a missing stock, failed reservation or numerical staffing shortage. No claimed optimization of all18-House service paths was required to pass the unchanged live goal.

Correct bounded recovery: inspect operations first, resume the existing paused Post and observe actual deadlines. Wrong intervention here: buy extra Farm/Clay/Pottery merely because population is low or the theoretical18-House consumption ceiling is high. An earlier exploratory capacity purchase was superseded: the controls-only observation already restored growth and actual goods misses were zero. The final starter performs no automatic producer purchase; another supplied save with persistent goods failures retains an explicit open capacity/backlog/routing diagnosis.

## Fresh Xia and Handan, current authority

Both runs begin with the ordinary paid1,280/1,300 starter at tick0, four Houses/population24,24 workers, upkeep48/400t, Funds20, no free Well/coverage/goods/tax/progress. The shared historical expansion recipe is translated to a legal original-map district: storage origin **Xia(103,113)**, **Handan(103,139)**. Purchases are the recipe in [the historical playthrough](city-v16-playthrough.md#paid-expansion-recipe), preserving order/ticks/relative origins and full active rule4 footprints. If Funds are temporarily insufficient, a purchase waits for actual revenue; nonfinancial blockers fail explicitly.

The last purchase is the fourth Well at tick4,800. Each completed city has two Clay/two Pottery, one Warehouse/Farm, two Markets/Service Posts/Fire Watches, four Wells, one HealthPost and ten Houses:27 buildings/15 Couriers. Handan total construction is2,812. Xia total2,780 is32 lower because16 scheduled expansion Road commands are legal original-road no-ops; its starter still costs1,280. No cost or map authority was altered.

| Current rule4/policy2/schema20 state | Xia | Handan |
| --- | ---: | ---: |
| First full goal tick | 7,200 | 6,800 |
| Population at first goal | 129 | 126 |
| Fixed stability interval | 7,200…17,200 | 6,800…16,800 |
| Population min/max in window | 129/148 | 126/148 |
| Effective L2 minimum | 8 | 8 |
| Goal-lost ticks / interruptions | 0 / none | 0 / none |
| Actual successful / missed demands in window | 250 / 0 | 250 / 0 |
| New fire/sickness incidents in window | 0 | 0 |
| Installed / active / assigned workers | 44 / 44 / 44 | 44 / 44 / 44 |
| Actual tax / upkeep per400t | 560 / 96 | 560 / 96 |
| Net Funds per400t | +464 | +464 |
| Window tax / upkeep / construction | 14,000 / 2,400 / 0 | 14,000 / 2,400 / 0 |
| Funds at goal → final | 2,778→14,378 | 2,282→13,882 |
| Final population / capacity / Houses / effectiveL2 | 148 / 148 / 10 / 8 | 148 / 148 / 10 / 8 |

Every House has Water and valid Market/Service reachability. The two Level1 Houses have capacity10, the eight L2 Houses16. Every House fulfils exactly25 further demands without misses:

| House ID | Desirability | Historical/effective level | Final residents/capacity | Successes/misses in fixed window |
| ---: | ---: | --- | --- | --- |
| 7 | −6 | 2/1 | 10/10 | 25/0 |
| 8 | 0 | 2/1 | 10/10 | 25/0 |
| 9 | 13 | 2/2 | 16/16 | 25/0 |
| 10 | 22 | 2/2 | 16/16 | 25/0 |
| 13 | 23 | 2/2 | 16/16 | 25/0 |
| 14 | 17 | 2/2 | 16/16 | 25/0 |
| 22 | 22 | 2/2 | 16/16 | 25/0 |
| 23 | 13 | 2/2 | 16/16 | 25/0 |
| 24 | 20 | 2/2 | 16/16 | 25/0 |
| 25 | 23 | 2/2 | 16/16 | 25/0 |

Pre-goal supply misses and illnesses remain in the event reports; they were not skipped. All main ticks verify actual goods/economy/navigation/population/Service/Health/fire invariants. Goal, effectiveL2 and population minima are checked every tick; construction and command sequence remain fixed after the first complete goal. Successful startup and uninterrupted goal retention are separate measured outcomes.

## Pipelines and accounting

The live chains are Clay→Pottery→Warehouse→Market→House, Farm→Market→House, and Service Post→House. Reports retain actual production deltas, production progress, recipe Clay, stocks, target-specific incoming reservations, courier cargo/good/owner/target/phase/path/edge progress, dispatch blockers and actual departure/arrival/return transitions. On-demand entrance/reachability queries accompany the400t snapshots. Outgoing cargo is identified by its courier owner; it is not double-counted as owner stock. Clay incoming at Pottery is not relabelled incoming Pottery.

Classification separates current full storage/normal reserved goods/active backlog from actual unavailable supply. Neither fresh fixed window has a goods miss, unstaffed owner, pending-road stall or invariant violation. The B control's stocked goods plus expired Service establish its measured blocker; the reconstruction's early newly placed House stock misses are logged separately, not retrospectively called production shortages. The player's Pottery11 remains **UNRESOLVED** without stocks, reservations, route/courier state and actual deadlines.

Each finance row measures actual tax, upkeep and construction deltas over the **same** interval and asserts `Funds delta = tax delta − upkeep delta − construction delta`. Cumulative screenshot taxes6,925 and installed rate98 cannot establish historical profit without actual maintenance/construction. Debt is separately reported; no interest, refund or bankruptcy mechanic was invented. Paid opening reserves may reach zero, while both fresh fixed windows stay positive and grow. Their margin is bounded evidence for these layouts, not every possible city.

## Persistence and compatibility

Each fresh map saves at tick399 before its known initial supply miss, with an actually moving Courier after4,406, at first goal, and after10,000 stable ticks. For each checkpoint the real writer/read/restore preserves the complete Snapshot; two real Worlds then continue100 ticks and match after every tick. Production F5/F9 handlers are exercised by direct synthetic SDL keys, with F9 restoring the exact state and pausing. These are scripted view checks, not native input acceptance.

After the writer process has exited, separate fixture OS processes reopen the same real checkpoint through production original-map/View/restore paths, continue100 ticks and compare the **entire SaveDocument JSON**, not just a digest or selected counters. Map SHA, permission fingerprint, profile/rule/schema, demand progress, population, stocks/reservations, courier routes/progress/cargo, operations/priorities, protections and lifetime finances are retained. B has additional outage/control/recovery checkpoints. Native manual save/restart remains a separate open acceptance item.

The full167-map census is exactly equal to the pre-change census, including all165 prepared policies under real/zero/one legacy masks, both known refusals and original inventories. All328 protected `src/`, `tests/`, `resources/` files have their pre-change SHA-256. The historical Xia rule3/policy1/schema19 helper still reaches goal6,800, ends9,200, and **all14 complete historical SaveDocuments** equal the pre-refactor baseline. The shared recipe does not migrate that historical run to rule4.

## Reproduce and inspect

From the repository root:

```sh
sh tools/test_city_v16_longplay.sh .local/gog-extracted/app --fresh
```

An explicitly provided OpenEmperor JSON save, only when its owner authorizes this replay:

```sh
sh tools/test_city_v16_longplay.sh .local/gog-extracted/app --save /explicit/path/save.json
```

The starter builds current on-demand helpers, verifies the pinned original revision, uses a new `.local/city-v16-longplay/run-*` root, scrubs app/SDL overrides, runs scenarios/restarts/history/census and rechecks all original inputs. The one supplied bounded file is read/copied and SHA-256 recorded; no personal directory is scanned, no input file is changed. Reader/restore reject wrong profile/rule/schema/map/policy. `--fresh` reports A **NOT RUN**. `--save` runs a14,000-tick no-intervention replay and separate controls-only experiments on copies; the file's human origin is caller-provided provenance, never inferred from schema20.

Output includes `summary.json`, `field-provenance.json`, per-scenario `households.json`, `demand-events.jsonl`, `workforce-events.jsonl`, `production-events.jsonl`, `finance-400t.jsonl`, `households-400t.jsonl`, `goal-stability.json`, `save-restart-comparison.json`, `commands.jsonl`, production/workforce/courier final reports, real saves and performance reports. Persisted facts, derived current queries, cumulative counters, instantaneous rates and interval deltas have separate provenance. Current diagnostic flags are not actual failure reasons; simultaneous reasons are overlapping exposures, not additional failed demands.

PASS means the stated technical assertions executed successfully. OBSERVED/RECONSTRUCTED/RECOVERY_INCOMPLETE and NOT RUN retain their actual meanings; a positively financed but shrinking control is not relabelled a stable city. Invalid source/mode/preparation stops dependent work with a named local log. All original bytes and generated saves/reports remain ignored and unbundled.

## Validation and runtime cost

Final primary reports: `.local/city-v16-longplay/run-J2TFkR/summary.json` and its scenario directories. The public `--fresh` starter exits0 in88.78s, with A NOT RUN, both C goals/stability passed, all **15 separate-process complete SaveDocument comparisons** equal and the full corpus unchanged. The public `--save` interface check uses the **owned authored reconstruction**, not a player save: `.local/city-v16-longplay/run-V3EMGt/`, exit0 in77.72s,14 separate-process comparisons, source/copy SHA-256 and bytes unchanged. That interface rehearsal does not close scenario A for the reported player's city.

| Check | Executed result |
| --- | --- |
| Release full CTest | **146/146 PASS**,71.08s |
| Debug full CTest | **146/146 PASS**,1,150.24s |
| ASan/UBSan full CTest, RelWithDebInfo | **146/146 PASS**,312.43s |
| Standalone AlphaEndurance,100k per existing scenario | **PASS** in Release17.90s / ASan-UBSan84.27s; standalone Debug NOT RUN, full Debug CTest endurance cases did run |
| Original Xia rule4 goal +10k under sanitizers | **PASS**,57.35s process; all eight complete checkpoint/continuation SaveDocuments equal final Release |
| Negative adapter checks | **8/8 expected rejections**: extra/missing arguments, malformed tick count, schema19, wrong map hash, permission hash, rule and save format |
| Valid explicitly readonly authored save | **PASS**,10k replay; bytes/SHA-256 unchanged |
| Frozen legacy and map authority | **PASS**,all14 historical SaveDocuments and the complete167-map census byte-identical;328 protected source/test/resource hashes unchanged |
| Exact player save / native human play | **NOT RUN** |

The actual configure/build/test commands use the existing build roots, with no new duplicate build tree:

```sh
cmake --build .local/reswall/release-build --parallel 2
ctest --test-dir .local/reswall/release-build --output-on-failure -j 2
cmake -S . -B .local/market-walkers-finish/debug-build \
  -DCMAKE_BUILD_TYPE=Debug -DOPENEMPEROR_USE_SYSTEM_SDL3=ON \
  -DOPENEMPEROR_ENABLE_SANITIZERS=OFF
cmake --build .local/market-walkers-finish/debug-build --parallel 2
ctest --test-dir .local/market-walkers-finish/debug-build --output-on-failure -j 2
cmake -S . -B .local/market-walkers-finish/sanitizer-build \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo -DOPENEMPEROR_USE_SYSTEM_SDL3=ON \
  -DOPENEMPEROR_ENABLE_SANITIZERS=ON
cmake --build .local/market-walkers-finish/sanitizer-build --parallel 2
ctest --test-dir .local/market-walkers-finish/sanitizer-build --output-on-failure -j 2
.local/reswall/release-build/openemperor-alpha-endurance
.local/market-walkers-finish/sanitizer-build/openemperor-alpha-endurance
```

The new excluded helper was also explicitly built in all three configurations. Sanitizer runs set `ASAN_OPTIONS=halt_on_error=1:abort_on_error=1` and `UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`. Logs and exact arguments/timing are in `.local/city-v16-longplay/regression-summary.json`, `ctest-{release,debug,sanitize}.log`, `interface-checks/summary.json` and `final-run-invocations.json`. No timeout or production assertion was weakened, and no registered test was skipped. Initial new-harness compilation errors using nonexistent/private query names were corrected to actual public APIs; these were adapter build errors, not failing engine regressions. Final builds have no reported compiler warnings.

Report I/O, Snapshot copies, diagnostic queries/BFS and save serialization are explicit harness work outside the measured `World.tick` span and outside normal frame/tick paths. No production instrumentation was installed.

The first complete starter run measured Xia17,200 production tick calls in0.604s and Handan16,800 in0.583s (about35µs/tick on this host), excluding diagnostics/I/O. End-to-end fresh fixture processes took3.23s/3.17s and peaked at108.0MB/153.6MB RSS. The18-House20k fixture took4.91s, with1.17s in production ticks and107.1MB peakRSS. Final Release processes measured3.25s/100.3MB for Xia,3.19s/153.7MB for Handan and4.98s/105.7MB for the20k reconstruction. The sanitizer-instrumented Xia diagnostic process measured20.98s inside `World.tick`,56.72s wall time and857.9MB peakRSS; these include sanitizer overhead and a diagnostic harness, and are not normal Release-app memory/frame claims. The independently measured initial167-map census took62.79s/221.2MB peakRSS, initializing presentation without rendered frames/Worlds. These are local diagnostic measurements, not native renderer frame-time or universal performance promises.

## Open findings

- **UNRESOLVED:** exact player60/100, Service8 and Pottery11 cause, actual routes/operations/deadlines, and historical net profitability, pending an explicitly authorized SaveDocument.
- **EXPECTED RULE / RECONSTRUCTED OPERATION ISSUE:** deliberate paused Post, missed deadlines lowering residents without removing historicalL2, and normal coverage gaps in the larger18-House route layout. Both control and recovery keep all active jobs staffed.
- **CONFIRMED ENGINE BUG:** none found; no failing production regression, engine fix or rebalancing introduced.
- **Technical scope closed:** two paid rule4 original-map full goals, uninterrupted further10k stability, actual finance/goods/worker diagnostics, scripted F5/F9 and complete process-restart equality, with frozen original/legacy contracts.
- **Human scope open:** manual input/construction/full-city play, exact player replay and general balance/service reliability for arbitrary larger layouts. ScenarioB's235 residents do not mean every demand succeeds.
