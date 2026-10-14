# City-v16 Service coverage, walker audit and endurance

## Baseline

Reference and actual starting HEAD: `26005cdfa7e66d59b448634a752dff2afff0858b`, initially clean. This is a local, uncommitted candidate. Production retains City-v16 **rule4 / MapPermissions policy2 / save schema20**, all older contracts, coverage1200t, demand400t, courier edge timing, costs, workers, two-Post limit, goods, fire/health, taxes and maintenance. No new gameplay version or migration is installed.

The main case is the previous **RECONSTRUCTED, normally paid original Xia** city, not Oliver's missing60-resident player save. The same paid command recipe earns ten Houses, expands to18, deliberately pauses Post16, and reaches tick27200 with62 residents. Its separate ordinary Resume command and4000t recovery reach tick31200 with113 residents,44 staffed workers and both Posts active. The measurement window is **(31200,41200]**, with no commands or purchases.

The pre-change recovery reports are frozen under `.local/city-v16-service-coverage/baseline-before/`. The new start is byte-identical to `prepare-development/service-start.json`; the complete end SaveDocument is byte-identical to the frozen recovery end. All450 demand-event lines in the fixed window are byte-identical. The old recovery JSONL also contains180 earlier demands from the separate4000t recovery phase; those are not counted in endurance. Neither frozen file was rewritten.

CURRENT results:

| Quantity | Observed |
| --- | ---: |
| Actual Service-valid deadlines / all deadlines | 351 /450 (78%) |
| Fully supplied demands / Service-only misses | 351 /99 |
| Covered Household ticks | 77.8222% |
| Longest uncovered run within window | 1150t |
| Revisit mean /p95 /maximum | 1361.35 /2220 /2350t |
| Houses never visited /known unreachable at end | 0 /0 |
| Population start /minimum /maximum /end | 113 /113 /245 /235 |
| Effective L2 Houses /capacity at end | 12 /252 |
| Live goal lost | 0 ticks |
| Funds start /end /net | 21318 /37698 /+16380 |
| Actual tax /upkeep | 18780 /2400 |

The exact House8 example is preserved: tick31600 Food8, Pottery7, expiry30915, previous arrival29715, no Service at demand, no tax, population6→5. Actual renewal arrives at31625, **25t after the failed deadline**. An arrival later in that tick window cannot undo the missed demand.

## Walker analysis

The request's shared-cursor hypothesis does not describe scalable City-v16. Each Service Courier stores its **private `last_dispatched_target`**. Reachable placed Houses are ordered by stable Building ID; selection starts after that courier's cursor and wraps. The legacy non-scalable Service path retains `last_dispatched_service_household_`. The Post inspector now selects the correct cursor and labels coverage **city-wide**, not as a Post's exclusive service territory.

Dispatch requires an enabled, idle courier, a running, staffed, non-burning owner and an existing current-revision cached route. A House need not lack coverage to be eligible. Multiple Posts rotate independently over the same Houses; there is no coverage reservation or partition. The saved cursor updates on dispatch. Ordinary outbound movement, actual arrival,1200t renewal, returning movement and idle transition remain authoritative. Service never carries goods or reservations. New roads/Houses and safe demolition refresh existing caches through ordinary commands; no overlay action does so.

CURRENT Post6 completes59 visits and Post16 completes71. Combined pre-tick phase exposure is19870 travelling /20000 available Post ticks (**99.35% utilization**); no paused, unstaffed, burning, route-pending or reroute exposure occurs in this window. Dispatch routes average15.446 actual edges, p95/max27; completed tours average152.798t, p95/max269. No simultaneous duplicate outbound dispatch or active-target oscillation was observed. Initial and final partial trips are explicitly censored; saved expiry minus1200 identifies a prior arrival, not an invented dispatch time. Revisit statistics include that first saved-arrival interval; uncovered-run statistics are clipped to the measurement window.

Both Posts are almost continuously occupied by round trips. Visits to already covered Houses and return cost make the rotation longer than1200t for some homes. These measured layout/capacity limitations explain lost coverage without proving a route implementation bug. Every House was reached in these finite10k windows; this is **not a universal starvation-freedom theorem**.

The four strategies also pass bounded **AUTHORED** transport challenges: an actual gate corridor and protected original-road path, disconnected House, signed-height barrier, real future-road interruption/repair, finishing an active trip after pause, new Household, safe demolition, priority changes, incomplete staffing and actual unprotected Post fire. Paired Worlds restore identical, unedited snapshots and then execute the same paid commands/ticks. These synthetic transport cases complement the original-map evidence; they do not establish original gate visuals, arbitrary map acceptance or original Emperor gameplay.

## Scheduler comparison

Only the excluded long-play executable compiles `OPENEMPEROR_SERVICE_SCHEDULER_EXPERIMENT`. The normal app and registered production tests contain no alternate scheduler symbols. Experiments use the same complete start SaveDocument, rules, cached transport paths, movement, arrivals, demand ordering and goods; no Snapshot fields are edited. Explicit strategy arguments are reused after process restart.

The tested choices are:

- **CURRENT:** unchanged private-cursor cyclic selection.
- **Expiry:** earliest coverage expiry, then stable ID. It permits concurrently outbound Posts to choose the same House.
- **Risk:** missing guarantee at the next actual demand first, then next deadline, expiry and stable ID. These scenario deadlines are aligned, so the measured expiry/risk results coincide; the ranking contracts remain different.
- **Travel:** skip another Post's current outbound target; give uncovered age≥1200t earliest-expiry precedence; otherwise prioritize risk and rank `4 × cached travel ticks − min(uncovered age,1200)`, then expiry/ID. No active-trip retargeting, guessed route or binding arrival ETA is introduced.

Same main18-House start and10k window:

| Strategy | Service-valid /450 | Full demand success | Service-only misses | Covered ticks | Max gap | Duplicate outbound dispatches | Revisit mean /p95 /max | Tax /net | End population |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | --- | --- | ---: |
| Current | 351 | 351 | 99 | 77.82% | 1150 | 0 | 1361 /2220 /2350 | 18780 /16380 | 235 |
| Expiry | 283 | 283 | 167 | 63.68% | 770 | 52 | 1240 /1960 /1970 | 15040 /12640 | 203 |
| Risk | 283 | 283 | 167 | 63.68% | 770 | 52 | 1240 /1960 /1970 | 15040 /12640 | 203 |
| Travel | 446 | 435 | 4 | 97.52% | 360 | 0 | 647 /1320 /1710 | 23160 /20760 | 248 |

Travel dispatch paths average6.927 edges versus Current15.446; completed tours average67.912t versus152.798t. Expiry/risk reduce the longest individual gap but their52 simultaneous duplicate dispatches waste visits and reduce aggregate coverage. The Travel experiment has11 additional demands failing for goods despite valid Service: **446 Service-valid does not mean446 fully supplied**. Its changed residents/consumption make goods outcomes part of the comparison, not fixed injected inventory.

Recommendation: **KEEP CURRENT for rule4**, classify this as a **LAYOUT/CAPACITY LIMITATION**, and consider a **separately versioned targeting upgrade**. No implementation error against the current rule was confirmed. Travel is promising across larger tested layouts, but at ten Houses it worsens continuous coverage: Xia100%→99.50%, max gap0→70t, worst revisit1100→1270t; Handan100%→99.92%, gap0→80t, worst revisit1150→1280t. All ten-House demands still succeed. Finite aging precedence is not a proof against every paused, unreachable or changing-road scenario.

A future upgrade needs an independently approved rule contract, deterministic tie breaking and multi-Post coordination, explicit old-save versus upgraded-copy choice, and complete SaveDocument/replay tests. Old rules must keep their current route/arrival/population/tax sequence. No rule5/schema21, migration or production strategy switch is proposed as an implicit part of this milestone.

## UI

`sandbox_ui::Action::ServiceOverlay` adds **V Service / V Service ON** to supported Service profiles. Default is OFF, stored only in the view. Normal V toggles it; **F3 walker diagnostics retain V role cycling**. Existing Help, budget, demolition, save/load and focus boundaries consume input before the toggle. Unsupported Industry/older profiles have no Service-overlay button. The adaptive toolbar computes an extra row where necessary rather than overlapping controls.

| Symbol /color | Exact current fact |
| --- | --- |
| C /muted green | Active now and `service_until_tick > next_actual_demand_tick` |
| ! /amber | Active now, expiry≤next demand; another visit could renew it |
| X /muted red | Expired or never supplied now |

Equality at the deadline is amber, never green. The classifier uses actual demand progress, checks addition overflow, and never predicts arrivals. Green guarantees **current Service coverage only**, not Food, Pottery, health, fire or a fully supplied future demand. The starting18-House city has8 green,4 amber and6 red Houses.

Four faint filled diamonds/colored outlines follow each actual2×2 footprint through the existing signed-height ground projection. C/!/X distinguish categories without color. The overlay follows map clipping/culling, preserves texture/blend state and leaves neighboring road/building authority intact. It uses no file work, parse/decode/upload, World copy/command/tick, BFS or route refresh. House inspection shows current Service/remaining, next demand, last actual demand, existing coverage guarantee, future arrival **unknown**, and geometric route **known reachable /no known route /unknown** from current cached routes. A paused/unstaffed Post may still have a known geometric route; that does not promise dispatch. There is no route calculation in the footprint loop.

Scripted software tests cover strict boundaries, actual arrival before deadline, real green→amber→red after pausing a Post, never-visited/disconnected Houses, keyboard/button/diagnostic precedence, modal gating, inspector cursor, exact OFF map pixels, F5/F9, combined Water/Health/Desirability,1×/2×/4×, panel/resize, and100 pure toggle frames. Original Xia tests repeat with18 and20 normally paid Houses and the exact original visual pack. CPU pixel capture is test-only.

Private captures are under the run's `scripted-ui18/` and `scripted-ui20/` (`overlay-on`, `overlay-off`, restored OFF, zoom1/2/4, combined, panel-closed and resized BMPs). macOS captures under `.local/city-v16-service-coverage/native-*.png` show the **normal Release app**, loaded via the ordinary menu with an isolated app root. OS automation demonstrates V, button Down/Up, wheel, House inspection, Help gating, F3 role cycling and F5→Step→F9. The CUA native pipe failed; AppleScript/CoreGraphics delivery was used with foreground-PID checks. This is **native OS automation delivery**, not human play acceptance or hardware-input parity.

## Endurance and capacity

Each original-map capacity family begins at its own earned ten-House goal, pays for the same common20-House road template, purchases only the selected House count, and warms up4000 ordinary ticks. Road-cell vectors match exactly within each family. The different House counts necessarily have different resident/coverage/stocks/history after warmup; these are comparable capacity scenarios, **not the identical tick31200 reconstruction**. Both real maps admit20 paid Houses within existing limits. Every window then runs10000t without commands, with two Posts/two Service Couriers, normal goods production and no injection.

CURRENT capacity results:

| Map /Houses | Service-valid /deadlines | Full demands | Service-only misses | Covered ticks | Longest gap | Revisit mean /p95 /max | Population min /max /end | Tax /upkeep /net |
| --- | ---: | ---: | ---: | ---: | ---: | --- | --- | --- |
| Xia10 | 250 /250 | 250 | 0 | 100.00% | 0 | 613 /1100 /1100 | 148 /148 /148 | 14000 /2400 /11600 |
| Xia14 | 293 /350 | 292 | 56 | 82.99% | 670 | 931 /1835 /1870 | 184 /210 /208 | 16800 /2400 /14400 |
| Xia18 | 309 /450 | 309 | 139 | 68.27% | 1350 | 1437 /2550 /2550 | 165 /250 /250 | 16160 /2400 /13760 |
| Xia20 | 329 /500 | 324 | 167 | 65.35% | 1700 | 1651 /2900 /2900 | 153 /255 /254 | 16580 /2400 /14180 |
| Handan10 | 250 /250 | 250 | 0 | 100.00% | 0 | 637 /1150 /1150 | 148 /148 /148 | 14000 /2400 /11600 |
| Handan14 | 303 /350 | 300 | 47 | 84.70% | 570 | 934 /1770 /1770 | 180 /200 /200 | 16120 /2400 /13720 |
| Handan18 | 336 /450 | 318 | 113 | 75.34% | 1120 | 1284 /2320 /2320 | 173 /236 /222 | 16260 /2400 /13860 |
| Handan20 | 309 /500 | 301 | 185 | 61.64% | 1650 | 1543 /2850 /2850 | 162 /232 /231 | 14680 /2400 /12280 |

All32 capacity/strategy combinations are recorded. Service-only misses for **Current /Expiry /Risk /Travel** are Xia10 `0/0/0/0`, Xia14 `56/62/62/0`, Xia18 `139/164/164/9`, Xia20 `167/230/230/15`; Handan10 `0/0/0/0`, Handan14 `47/2/2/0`, Handan18 `113/92/92/4`, Handan20 `185/196/196/10`. Thus expiry/risk can help Handan14/18 while harming other layouts. No House is never reached in these finite windows. Complete per-House worst coverage, Post route/tour distributions, phase exposures, actual levels, workers, goal and finance data remain in each `endurance-summary.json`; do not infer one global balance PASS from the table.

The current goal remains held even when coverage reliability drops and demand misses occur; all eight current capacity windows are profitable. **Goal stability, Service reliability, all-demand success and financial profit are separate measured properties.** Technical PASS means deterministic invariants/assertions hold, not100% supply.

## Persistence

The baseline captures actual outbound and returning edge progress, expiry−1, demand−1, after House8's actual missed demand and the end. The existing real SandboxView F5/F9 handlers save, advance, restore the full state and pause; overlay UI is excluded. Each checkpoint also compares100 ordinary continuation ticks after unedited restoration. Experimental end checkpoints reuse their explicit strategy.

The final reproducible comparison includes **71 new OS-process restarts**, each structurally equal over the **complete SaveDocument** to its saved expected continuation, including complete WorldSnapshot, private cursors, owner/target/phase/path/edge progress, coverage/demand clocks, stocks/reservations, population/workers, controls, treasury/tax/maintenance, original map identity and permission fingerprint. The main before/end saves and historical compatibility additionally compare bytes. Native OS-automated F5 also writes a complete document equal to the tick31200 input; Step shows31201 and F9 returns31200 paused.

## Regression and performance

Six initial focused UI/input tests pass; the dedicated `sandbox-service-coverage-view` joins the existing suite, making147 registered tests. **Debug147/147 (1218.15s), Release147/147 (80.82s), ASan/UBSan147/147 (296.61s)** pass without skipped tests. Exact invocation logs live under `.local/city-v16-service-coverage/matrix-*.log`; `final-validation.json` records completion and source hashes. Successful-test output in every `Testing/Temporary/LastTest.log` is also checked for sanitizer diagnostics: none. Existing timeouts and assertions are unchanged. The sanitizer suite uses the repository's existing **RelWithDebInfo** configuration; an initial Debug sanitizer configuration was stopped and preserved separately before restoring that established configuration, not credited as a completed suite. Its two completed optimized runs (319.72s and296.61s) both pass all147 tests; only the final run is used in the matrix above.

The167-map census remains **165 prepared /2 known conflicts**. Its complete frozen report is equal, preserving every policy1/2 actual/zero/one fingerprint and legacy contract. The original1464-file/819711091-byte manifest remains `dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`. All14 historical Xia rule3/policy1/schema19 saves remain byte-identical, with goal6800/end9200. Fresh production Xia and Handan retain goal7200/6800, another10k with148 residents,250/250 full demands and+11600 net.

The excluded diagnostic adapter is explicitly built in all three configurations. Debug's full18-House CURRENT run and ASan/UBSan's four complete10000t strategy runs produce structurally identical **complete end SaveDocuments** to Release. Both Debug and sanitizer adapters pass the authored transport challenges. Those sanitizer processes set `ASAN_OPTIONS=halt_on_error=1:abort_on_error=1` and `UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`; their complete logs contain no sanitizer diagnostics. Adapter work is distinct from the147 registered production tests.

Paused original-map software measurements separate renderer cost from audit overhead. The final shell run measured18-House mean frame1.573ms OFF /1.890ms ON, p95 1.629/1.946ms. **All twelve expensive counters are zero** in100 OFF,100 ON and100 toggle frames: World copies/restores/executes, road plans, BFS/visited cells, route refresh, decode/upload, file reads/writes and simulation ticks. Live texture count is1346 before/after. The complete WorldSnapshot stays equal. These are local software-rendered direct frames, not Metal GPU or human frame-rate claims; full measurements are in `ui-overlay-acceptance.json`.

The observed baseline's10000 `World.tick` calls took0.686s excluding all report/Save/Snapshot work. A separate plain-World continuation process took1.525s including load/save startup and peaked at95.37MB RSS. Each scenario also records its own diagnostic peak RSS and wall time. Concurrent regression load can affect timings; no universal speed or memory promise follows. The observer/statistics/alternative ranking exist only in the on-demand tool.

## Player test and reproducible reports

From the repository:

```sh
sh tools/test_city_v16_service_coverage.sh .local/gog-extracted/app
```

The default validates the pinned read-only original revision, builds current Release code in the existing build root, reconstructs the same paid case, measures all four main strategies, both maps'10/14/18/20 families, the transport challenges,18/20-House scripted UI, full process restarts, historical Xia and original census. Output is a fresh ignored `.local/city-v16-service-coverage/run-XXXXXX/`; no personal save search/settings change or extra persistent build tree.

```sh
# Same full diagnostics plus separate plain-World benchmark
sh tools/test_city_v16_service_coverage.sh .local/gog-extracted/app --benchmark
# Prepare main case and diagnostics, then launch the normal Release menu
sh tools/test_city_v16_service_coverage.sh .local/gog-extracted/app --ui
# Full diagnostics + benchmark, then normal Release menu
sh tools/test_city_v16_service_coverage.sh .local/gog-extracted/app --all
```

In the isolated menu choose **L Load Save**, `01-service-baseline.json`, **Open selected /Enter**. It loads paused. Wheel zoom over the central paid district and pan with arrows; press **V** or **V Service**. The baseline contains all three categories. Select Houses/Post list entries; wheel-scroll the inspector if needed. Test V under Help and under F3; close F3 to return to normal V behavior. F5 saves only the test copy, Step advances, F9 restores paused. Run and inspect actual deadline outcomes separately from current colors. Personal saves/settings and original assets remain untouched; autosave is disabled in this test root.

The executed final shell benchmark run is `.local/city-v16-service-coverage/run-MH3eDc/`; the independently repeated full run is `run-repro-final/`. The actual `--ui` shell launch uses `run-OSv179/player-app/`; it loads the normal Release menu and the same18-House city, with complete native-written F5 document equality. Final native screenshots are `native-final-overlay.png` and `native-final-house.png`; earlier screenshots also show Help/F3 and Step/F9. Each baseline/scenario report has source Save hash, map, rule/policy/schema, start/end ticks, commit, dirty-workspace and CURRENT/OBSERVED/RECONSTRUCTED/EXPERIMENTAL provenance. Top-level named reports are `baseline.json`, `service-visits.jsonl`, `service-couriers.jsonl`, `house-coverage.jsonl`, `demand-results.jsonl`, `service-failures.jsonl`, `scheduler-comparison.json`, `finance-400t.jsonl`, `endurance-summary.json`, `save-restart-comparison.json` and `ui-overlay-acceptance.json`. Raw pre-change comparison events remain separate and unchanged. Original pixels/saves/reports are ignored and excluded from Git/bundles.

**Human acceptance remains OPEN:** manual readability/color-independent symbols and occlusion at all zooms, real held arrows/wheel/resize/focus/external-display behavior, long manual construction play and Oliver's exact reported player state. No native OS automation, synthetic pixels or profitable city is promoted to human PASS or original Emperor parity.
