# OE43 native macOS gameplay acceptance

**October 29, 2026 — overall PARTIAL; HUMAN NOT_RUN.** The current application
starts and renders natively, but native pointer acceptance remains blocked by
observed automation delivery. Desktop control stopped immediately at user
takeover. No gameplay, balance, historical rule or save contract was changed.

| Layer | Result | Observed scope |
|---|---|---|
| TECHNICAL | **PASS** | Current Release 278/278 registered tests, no failures/skips; paid Rules12–14 and Bridge5/Ramp6 continuations, exact process restores and protected inputs |
| SCRIPTED_APP | **PARTIAL** | Current MenuSession/SandboxView journeys, inspector, operations, save/recovery and sampled rendering pass; this is not every native product interaction |
| NATIVE_GUI | **PARTIAL** | Cocoa/Metal startup, keyboard menu/map selection, safe starter refusal, paid Xia startup, F5, held-arrow movement, zoom1/2/4 and Help observed; pointer commit, resize/focus, operations and restart remain open |
| HUMAN | **NOT_RUN** | Prepared working copies and launch scripts; no user observations supplied and no checklist items marked PASS |
| RELEASE_READINESS | **PARTIAL** | Local CMake QA app only; native/human gaps, distribution/signing and project-license blockers remain |

## Identity and isolation

All cases below use production baseline **B =
`151fd0a6b14d3054f2b82c8cd28d7fd769c54539`**, verified against fetched
`origin/main`. The initial worktree was clean. Work is on
`test/oe43-native-gameplay`; the eventual QA/documentation commit is not a new
gameplay baseline. Native code, CMake, simulation, persistence and resources
remain byte-identical to B. Changes to existing QA tools only admit fresh,
disjoint `.local/oe43/` output directories and the earned OE43 restart input;
their source, original-data and complete-save validation remain active.

| Identity | Actual value |
|---|---|
| Host / build | macOS27.0.1 build26A434, arm64; Release, deployment target26.0 |
| Build | `.local/oe43/release/`; generated revision `151fd0a6b14d`, dirty=false at configuration |
| Native QA app | `.local/oe43/OpenEmperor-OE43.app`, bundle `org.openemperor.qa.oe43` |
| Tested executable SHA256 | `ad73ecf018c7ad0ac649be64e626d163a5a2bda10589d0b696fccd8ec9e4dbca` |
| Original CMake executable SHA256 | `8cf44eb97734457d28410cac06cc868baff8f5744b221462be25fae3309300ce` |
| Identical executable `__TEXT,__text` SHA256 | `76e219c9300fa92613a347cced175de300f3b46d33b54a82c111c36d1fcf9505` |
| QA-only bundle changes | Name/identifier and ad-hoc signature; signature data explain different whole-file hashes |
| Native backend | Closed calibration observer: `cocoa` / `metal`, SDL3.4.14, 1100×700 client, 2200×1400 pixels, density/display scale2 |
| Dependencies | System SDL3.4.14 and Homebrew OpenSSL4.0.3; exact `otool -L` output in `binary.json` |
| Protected inputs | 1464 original files plus10 explicitly documented QA saves; SHA256, size, mode and mtime pinned before work |

All evidence paths in this report are below **E = `.local/oe43/`**, except the
explicit read-only source saves named in `human-handoff.json`. Personal saves
were not searched or modified. Original data are `.local/gog-extracted/app`.
The Ramp case has its separate authored data root and is not original-map
ramp acceptance. The human roots and native automation roots are disjoint.
This app has system dependency paths and is not a relocated distribution bundle.

## Reproduction and current technical evidence

The current Release was configured and built with:

```sh
cmake -S . -B .local/oe43/release -DCMAKE_BUILD_TYPE=Release \
  -DOPENEMPEROR_USE_SYSTEM_SDL3=ON -DOPENEMPEROR_BUILD_MACOS_APP=ON \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=26.0 -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DOPENEMPEROR_ORIGINAL_MODEL_TEST_ROOT="$PWD/.local/gog-extracted/app"
cmake --build .local/oe43/release --parallel 6
ctest --test-dir .local/oe43/release --output-on-failure -j 2
```

The actual complete invocation has JUnit `E/release-junit.xml`, logs
`configure.log`, `app-build.log`, `tests-build.log`, `qa-build.log` and
`release-tests.log`. All278 cases passed,0 skipped,240.08s wall time. The
registered coverage includes real-model goods, production, needs, religion,
culture, medical/water, fire/Inspector, house development, transport,
operations/demolition, input/modal boundaries and save/recovery contracts.
Synthetic/direct World tests do not become native gameplay acceptance.
After the QA path changes,15 focused runner/CLI/input/layout contracts also
pass; six output-root/symlink negative controls reject without creating output.
These focused cases overlap the registered suite and are not additional product
features. Evidence: `E/qa-final-junit.xml` and `E/guard-controls/results.json`.
Individual test identities and exact commands are retained in JUnit/CTest;
tests that do not emit map/tick metadata do not acquire invented values here.

For the following cases, severity **—** means no new product fault observed.
P1/P2/P3 labels identify the stated acceptance gap or observed limitation;
an automation limitation is not assigned to application coordinate handling.
All rows have Git identity B.

| ID / function | Level / result | Expected and observed | Map, rule/schema, ticks; exact reproduction | Evidence / severity |
|---|---|---|---|---|---|
| T01 registered contracts | TECHNICAL **PASS** | Every registered case completes;278/278,0 failures/skips | Synthetic fixtures and supplied original maps; multiple historical generations; commands above | `release-junit.xml`; **—** |
| T02 Food/Hemp/Ceramics, production/storage, markets, inhabitants/work/taxes, water/medical/culture/religion/fire | TECHNICAL **PASS**, finite Rule12 window | Resume real paid state; all current House needs remain met,603 Food delivered/600 consumed,284 Inspector arrivals; Funds11785→13735, tax7950/upkeep6000, workers120/120 | Xia,12/28,20000→40400; sustainable acceptance `--mode resume --rule 12 --variant balanced-three --ticks 20400`, explicit protected seed | `rule12-resume-20400/`, `gameplay-summary.json`; **—**; no unlimited-sustainability claim |
| T03 Common development and needs-aware distribution | TECHNICAL **PASS**, finite Rule13 window | Fresh purchases116/cost2588, actual9→10 at22837/House3;400 fulfilled evaluations,52 actual delivery arrivals; Food600/600, workers190/190, cashflow+4360 | Xia,13/29,0→43237; market acceptance `--mode diagnose --rule 13 --variant common-near-pottery --search-ticks 30000`; independent resume22837→43237 | `rule13-common10-create/`, `rule13-common10-resume/`, `fresh-paid-development-execution.json`; **P2** open indefinite Hemp supply,94→67 stock+cargo |
| T04 Elite development and continuation | TECHNICAL entry/replay **PASS**; retention **PARTIAL**, finance **FAIL** | Fresh161 normal commands (146 purchases,15 zero-cost controls)/cost4343, actual16→17 at7333/House43. Following window loses target for15488 ticks,161 current-unmet evaluations,4200 burning ticks;57 new Food misses, cashflow−3241; workers283/345 at end | Xia,14/30,0→27733; elite acceptance `--mode create --variant lean-med`; separate `--mode resume --variant lean-med --input first-success.save.json --expected result.save.json` | `rule14-elite17-create/`, `rule14-elite17-resume/`; **P2**, known measured limitation reproduced, not fixed |
| T05 full documents / Recovery | TECHNICAL **PASS** | Rule12/13 literal saves/metrics equal; Rule14 six full checkpoint snapshots and canonical roundtrips equal, endpoint Recovery exact. Four fresh Common/Elite first-success/end documents also byte-equal to protected creators | Xia,12/28,13/29,14/30; intervals above; run existing acceptance binaries against explicit protected inputs, fresh E outputs | `paid-replay-comparisons.json`, `gameplay-summary.json`, `fresh-paid-development-summary.json`; **—**; comparison categories overlap and are not additive |
| T06 paid bridges and real crossings | TECHNICAL/SCRIPTED_APP **PASS** | Paid$60 span, loaded outward/return crossings, delivery, disconnection and charged repair;36 residents/Funds2295 at12000;10 new processes retain complete immediate/+100t documents | Chang-an Sui,5/21,0→12000; `python3 -B tools/run_city_v16_bridges.py "$PWD" "$PWD/.local/gog-extracted/app" "$PWD/.local/oe43/release" "$PWD/.local/oe43/bridge-gameplay" --gameplay`, initially empty root | `bridge-gameplay/summary.json`, `runner.log`, opened1/2/4× images; **—**, native bridge mouse construction NOT_RUN |
| T07 paid ramps and real crossings | TECHNICAL/SCRIPTED_APP **PASS** | Authored level transition, loaded trips, role crossings, disconnection/repair;30 residents/Funds1082 at12000;11 new process pairs exact | Authored Ramps,6/22,0→12000; corresponding `tools/run_city_v16_ramps.py` with E/ramp-gameplay, initially empty root | `ramp-gameplay/summary.json`, `runner.log`, opened images; **—**; original-map ramp parity UNSUPPORTED |

The Bridge/Ramp comparison is21 new-process checkpoint pairs,42 complete
immediate/+100t documents. Their source digests and1464 original input pins
remain exact; the167-map corpus remains165 prepared/two conservative conflicts.
See `E/bridge-ramp-validation.json`. These checks establish actual deliveries
and paths within their bounded contracts, not proximity-based coverage.

## Current scripted application checks

These use the production MenuSession/SandboxView with SDL dummy/software.
The executable is `E/release/openemperor-system-playtest-acceptance`.
Every invocation uses DATA, RESOURCES, a fresh app root and capture root.
The full command identities/results are in each `report.json`.

| ID / function | Level / result | Expected / observed, identity and exact steps | Evidence / severity |
|---|---|---|---|---|
| S01 inspector, lists/details/hints, operation/priority controls | SCRIPTED_APP **PASS** | Xia14/30,tick901/Market7; `--inspector-layout` with protected OE41 journey save; actual1280×800/1600×900/1920×1080 resizes and zoom1/2/4, complete row reachability, clipped hits refused, commands match reference World, F5/Step/F9/F5 canonical equality | `scripted-inspector/report.json`, `inspector-layout.json`; opened samples in `visual-review.json`; **—** |
| S02 buildings, House tabs, overlays, walkers/rendering | SCRIPTED_APP **PARTIAL** | Xia14/30,tick7333; `--inspect-paid` on a working copy of the earned Elite city. Per-owner/actor outcomes retained; unavailable idle/nonselectable actors do not become universal picking PASS | `scripted-rich/inspection.json`, `report.json`, three opened earned-city zoom images; **P3** remaining visual/picking coverage |
| S03 road purchase/refusal, Market operations, Help, clocks | SCRIPTED_APP **PASS**, scoped | Xia14/30,tick0→901; `--create` into E/scripted-create-app and E/scripted-create. Actual paid Road/House actions, protected-cell refusal, pause/priorities and normal TickDriver advance | `scripted-create/report.json`, private captures; **—** |
| S04 F5/F9, new process, Autosave/Recovery and another generation | SCRIPTED_APP **PASS**, scoped | Creator's full901 document, actual Save/Step/Load/Save and Recovery; separate `--resume` uses creator working copy, restores complete bytes and advances160 ticks to1061. Same-process second session switches to ordinary City11/rule3 Handan | `scripted-create/report.json`, `scripted-resume/report.json`; registered original-menu/session/recovery cases; **—** |
| S05 Escape/right-click/resize/focus/UI-release cancellation, legacy demolition/modal behavior | TECHNICAL/SCRIPTED_APP **PASS** | Existing input, road transaction, operation/demolition and modal tests pass using their actual synthetic/legacy identities. Rules12–14 do not gain productive-building demolition | JUnit `sandbox-input-reliability`, `sandbox-software-view`, `menu-input-boundaries`, `original-menu-session-flow`; **—**; native counterparts remain open |

Sixteen current software PNGs were actually opened:13 inspector samples and all
three earned-city views. The exact files/hashes are in `E/visual-review.json`;
the other16 generated images in those two directories remain NOT_REVIEWED.
No sampled text/control/hint overlap was observed at the three required sizes.
The additional536px-height sample has bounded remaining controls, not complete
inspector usability. The original/fallback marker and whole-scene parity limits
remain. Bridge/Ramp have their own eight opened captures and review manifests.

## Native observations and delivery boundary

The own QA app was launched with explicit original DATA and isolated roots,
`SDL_VIDEO_DRIVER=cocoa SDL_RENDER_DRIVER=metal
OPENEMPEROR_INPUT_DIAGNOSTICS=1`. Stdout and stderr are separate. The observer
exports only on shutdown; an automation request alone is never delivery proof.

| ID / function | Level / result | Expected / observed and reproduction | Identity / evidence / severity |
|---|---|---|---|---|
| N01 app startup/backend | NATIVE_GUI **PASS**, scoped | Launch QA executable with E/native-calibration-app; current menu appears. Own process closed via native Cmd-Q, exit0; observer16/16 records,0 overwritten, Cocoa/Metal confirmed | B; no World loaded. `native-calibration/{01-menu.png,stdout.log,analysis.json}`; **—** |
| N02 native mouse click delivery | NATIVE_GUI **PARTIAL — BLOCKED_AUTOMATION_DELIVERY** | Fresh2200×1464 screenshot, request New Sandbox at[435,410]. Down raw[217,173], render[434,346], correctly recognized menu action2/NewGame. Up raw/render[0,0] cannot complete the same button; menu unchanged. No application coordinate workaround added | B; menu/no World. `native-calibration/requests.jsonl`, `02-after-menu-click.png`, observer sequences6/7; **P1 acceptance gap**, not a demonstrated app transform defect |
| N03 menu/rule/map keyboard selection | NATIVE_GUI **PASS**, scoped | Fresh own native-01 app root; S selects Rule12; Down changes ASSR-Baoji→ASSR-Jiayuguan,1→2/167, actual pictures observed | B; policy3, no save/tick yet. `native-01/{01-rule12-choice.png,02-next-map.png,stdout.log}`; **—** |
| N04 failed prepared starter | NATIVE_GUI **PARTIAL** for that map; safe refusal **PASS** | Return on Rule12/Normal/ASSR-Jiayuguan keeps setup open and shows `No legal empty 24x13 level…`; no paid city published. The message is an in-menu error, not an exercised native OS file/error dialog. Own process closed exit0 | B; no World/schema/tick published. `native-01/03-paid-start.png`; **P3** map/starter usability limit; unsupported placement is not silently widened |
| N05 paid native startup and manual save | NATIVE_GUI **PASS**, scoped | Own native-02 preferences preset last_map=Xia, then real S/Return starts paid Rule12/Normal. Actual F5 creates full OpenEmperor document: tick0,19 bodies, construction1404,Funds10596,0 residents, empty starting stocks | B; Xia12/28,tick0. `native-02/{01-xia-choice.png,02-paid-xia.png,before-camera.save.json}`; **—** |
| N06 camera, zoom and Help | NATIVE_GUI **PASS** for visible effects; World-purity completion **NOT_RUN** | Native held Right700ms visibly pans; local keyboard's physical y activates Z zoom, observed1→2→4→1. H opens readable Help. Next requested Help/scroll/save sequence was interrupted at takeover; no post-camera F5 comparison is claimed | B; last verified Xia12/28,tick0. `native-02/{03-camera-zoom2.png,04-zoom4.png,05-help.png}`; **—** for observed effects |
| N07 building selection, road click/drag, Escape/right-click during drag | NATIVE_GUI **NOT_RUN** | Paired pointer delivery not established; no paid mouse construction or cancellation success inferred from failed requests. Prepared for user observation | B; human scenes below; **P1 acceptance gap** |
| N08 inspector wheel, operation/priority/demolition, native modals | NATIVE_GUI **PARTIAL** | Scroll sequence interrupted; no completed scroll capture observed. Operation/priority/demolition are NOT_RUN. Scripted coverage is S01/S05; original productive demolition remains unsupported | B; last verified Xia12/28,tick0. No invented screenshot/result; **P1 acceptance gap** |
| N09 actual window resize, focus transfer, native OS chooser, native F5/F9/new-process/Recovery | NATIVE_GUI **NOT_RUN** in this run | Prepared for the human checklist. Earlier OE41 observations are historical, not repeated OE43 PASS results | B; identities supplied by chosen scene below; **P1 acceptance gap** |

**Takeover boundary:** during the requested Help-close/inspector-scroll sequence,
Computer Use returned “The user changed … OpenEmperor-OE43.app”. No further
desktop read, action, restart, resize or shutdown was sent. Native-02 remains
under user control; its post-takeover state and observer export were not inspected.
The last confirmed pre-takeover screenshot is `native-02/05-help.png`.
The pending sequence does not establish which individual actions completed.
The calibration request ledger originally transcribed the screenshot height as1466;
the preserved PNG is2200×1464 and remains the authoritative image. This QA
transcription error changes neither the requested point nor the observed events.
User takeover itself supplies no human acceptance result.

## Isolated human handoff

**HUMAN NOT_RUN for every item.** If the native-02 app is still running, finish and close it
manually. Then, from the repository root, run exactly one prepared launcher:

```sh
.local/oe43/start-human.zsh
# Separate authored Ramp data/session, after closing the other QA app:
.local/oe43/start-human-ramps.zsh
```

These executable scripts passed `zsh -n` and were not launched by automation.
They invoke the tested QA app with `--data` and separate `--app-root` values;
they do not use `--load-sandbox` or a personal save root. Use **L Load Save**,
arrows and Return to load a numbered working copy. F5 will target that copy.
`E/human-handoff.json` records source/target SHA256 and identities.

| Working copy | Map / rule / schema / tick | Concrete human checks, all NOT_RUN |
|---|---|---|
| `human/saves/01-Rule12-native-Xia-tick7952.json` | Xia /12/28/7952 | Active typed Hemp cargo/reservations, actual arrivals, complete F5/F9/restart/Recovery |
| `02-Rule12-earned-Xia-tick20000.json` | Xia /12/28/20000 | Twelve-House production/supply, workforce, tax/upkeep boundary, water/medical/culture/religion and Inspector visits |
| `03-Rule13-Common10-Xia-tick22837.json` | Xia /13/29/22837 | House3 Common10, actual Food/Hemp/Ceramics arrivals, Needs/Goods/Visits, buffer accounting |
| `04-Rule14-Elite17-Xia-tick7333.json` | Xia /14/30/7333 | House43 Elite17, DES70+10, FireWatch/Inspector, large lists/overlays/walkers, measured supply/finance limits |
| `05-Rule14-Market7-Xia-tick901.json` | Xia /14/30/901 | Market7 scrolling, details/hints/operations/priorities;1280×800/1600×900/1920×1080 and zoom1/2/4 |
| `06-Rule5-Bridge-Chang-an-Sui-tick56.json` | Chang-an Sui /5/21/56 | Loaded outward/return trips, selection, safe interruption/repair, feet and layering |
| `human-ramps/saves/01-Rule6-Ramp-Authored-Ramps-tick58.json` | Authored Ramps /6/22/58 | Loaded ascent/descent, shared edge legality, feet/layering, save/restart; no original-map ramp claim |

Use the existing [OE41 human checklist](../../OpenEmperor_OE39_OE40_Playtest/OE41-manual-checklist.md)
and [input checklist](../testing-input-acceptance.md). They remain unchecked.
For actual demolition success create a disposable ordinary City-v16/rule4 city
and follow the existing [safe demolition procedure](../testing.md#manual-play-and-persistence);
Rules12–14 do not offer arbitrary productive-building demolition.
Record expected/observed targets, rule/schema/map, tick, IDs, Funds, duration,
display scale and screenshots under E. Keep normal pause/upkeep costs and real
arrivals separate from selector clicks, proximity and planned visits.

## Five highest-priority open problems

1. **P1 — Native pointer and human acceptance are incomplete.** Paired OS pointer
   delivery failed at release; road drag, selection, focus/resize/modal boundaries,
   OS chooser and native persistence need correctly delivered input and actual user
   observations. No speculative global coordinate patch is justified.
2. **P2 — Rule14 earned Elite17 is not retained by the tested recipe.** The fresh
   current run reproduces supply/medical/fire losses and−3241 operating cashflow.
   Entry, saved Funds and technical replay success do not imply sustainable play.
3. **P2 — Higher-tier supply has finite demonstrated bounds.** Rule12's known
   Common retention limitation remains; Rule13 succeeds in its measured window
   while Hemp stock+cargo falls94→67. Indefinite replenishment and arbitrary-layout
   capacity remain open.
4. **P3 — Original visual/terrain parity is incomplete.** Actual images retain
   authored/fallback markers; anchors, interpenetration, shadows, heights/cliffs
   and original ramp semantics are not universally verified. OE42's sampled
   inspector overlap is fixed and is not reintroduced as an open defect here.
5. **P1 — Distribution readiness is unapproved.** This is an ad-hoc signed,
   dependency-linked local QA app. No new relocated-package, Finder/Gatekeeper,
   Developer ID/notarization or clean-Mac acceptance was performed; the checkout
   still lacks a selected project LICENSE. No publication is authorized.

Original Emperor savegames/campaigns, NPC economy, shipping/Ferry, venue/performer
and original illness/terrain lifecycle parity remain **UNSUPPORTED**, not newly
introduced bugs. Authored bounded services, Bridge/Ramp and fire protection keep
their distinct existing contracts. No new P0/data-loss finding is established.

Private final checks retain exact commands, exit codes, JUnit, source/input
pins and image reviews. No push, merge, tag or release is part of OE43.
