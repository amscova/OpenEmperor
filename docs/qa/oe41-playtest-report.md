# OE41 system playtest

**Technical validation completed October 28, 2026; human acceptance remains open.**
Registered Release, Debug and ASan/UBSan coverage is closed at 277/277 each,
and the resumed headless alpha stages and local package checks pass with the
explicit artifact reuse below. Overall acceptance is **PARTIAL**: several product
input subcases remain unexecuted, an inspector layout defect is confirmed, and
native pointer and human acceptance remain open.
Three native app processes add actual keyboard play and exact save restoration;
desktop automation stopped at user takeover. A successful finite
city does not establish indefinite supply, arbitrary-layout fairness or original
Emperor parity. Human acceptance is **NOT_RUN**.

## Identity and evidence boundaries

| Item | Recorded identity |
|---|---|
| Branch / clean resume point | `test/oe41-system-playtest`; `6837ef0f2c3dd880cdab1759850cb2fc40d05c3e` was clean at resumed audit start |
| Production baseline | OE40 `2f433d2af63b8c545e78ab7b3703570f1a3a397c`; current `src/` and `resources/` hashes still match |
| Production/resource manifest | SHA-256 `8fe50cc9fd290769e5328af905a0748cac1293228b8ac1e496ce938dbf405478`;276 files, path-NUL-filehash definition in `resume-01/final-audit/root-source-docs.json` |
| Earlier QA revision | `e752aa994c5fd803d13151db73379946e0bb2096`; its QA changes and the runtime diagnostic are retained by the resume commit |
| Resumed QA revisions | `cc3b033489fd388bd6bc33c20537ad1881247a0f` and `f10d31dabd64e482392138c72d1497d18b974aeb`; alpha optional-data/aggregate-budget controls and 27 focused Python cases, with no production/resource changes |
| OE39 / OE40 status | OE39 finite paid Common10 retention passed; OE40 paid Elite17 entry passed, retention/finance remained partial/failed in the measured recipes |
| Host | macOS27.0.1, build26A434, arm64; Apple clang21.0.0, C++20 |
| Scripted runtime | SDL3.4.14, `dummy` video / `software` renderer; native and human results are separate |
| Native runtime | Actual CMake `OpenEmperor.app`, SDL3.4.14, Cocoa video / Metal renderer, system dependencies; separate from relocated-package acceptance |
| Original root | Explicit supplied `.local/gog-extracted/app`; no personal saves searched |
| Original corpus |1464 files /819711091 bytes; all SHA-256, size, mode and mtime pins match the frozen baseline at resumed audit |
| EXE | `6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e`; read-only static evidence, never executed |
| Model | `7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6`; paid economy runs use Normal |
| Runtime diagnostic binary | `3c60e4dc78f82097954cec073071da4309fcf8797694176f3912787f46fc6f7f`; exact flags, linked dependencies and commands are in `run-01/runtime-qa/` |
| Private evidence | `.local/oe41/run-01/`; resumed audit under `.local/oe41/resume-01/`; feature runners retain their required ignored OE37/OE39/OE40/Bridge/Ramp roots |

Current map SHA-256 values are Xia
`871266c415d7500ff3cf3fe70fc6b36caecc4d4cbf09d4d737849e4060906433`,
Handan`6bf6bcf54c19a680985ce1eaf3e5846259fcaf7652bbf443fbe716845acade13`,
Badaling`c266c0e2a711045f91eb9bfd410445503782e0274884edd36880cf20f554bb46`
and Kaifeng`c25855e4db083128d9344b9177dbbc12c2f0fb3f28c25b63d01c93349912e7b3`.
The bridge transport uses a separately admitted Chang-an Sui site. The ramp
transport uses an explicitly authored map; it is not an original-map ramp claim.

The [capability matrix](oe41-capability-matrix.md) describes registration and
execution separately. The original272-record inventory was preserved and
extended to329 source/context records:201 user-exposed,118 debug-only,2 isolated
prototypes and 8 unsupported systems. These are not329 independently executed
functions. Actual runtime traversal observed67 menu action IDs, including dynamic
rows, and 57 toolbar action IDs across16 ordinary profiles and five original
generations;399 construction-selector events retained exact World snapshots.
The expanded catalog links319 records to related registered contracts, without
converting source-only actions into app passes. The machine catalog's scripted
app-action snapshot is60 PASS /261 NOT_RUN /8 UNSUPPORTED; later native evidence
below is a separate layer. Browser and delegated CLI metadata is now present;
live coverage remains explicitly incomplete.

## Automated baseline and integrity

| Configuration / check | Observed result | Scope and evidence |
|---|---|---|
| Retained Release | **277/277 PASS**,0 failures,0 skips;280.525s | October 26 run, `run-01/release/ctest-current.json`; exact registered list and source-scope exception retained |
| Resumed Debug | **277/277 PASS**,0 failures,0 skips;3021.701s summed wall time | Two invocations: menu alone96.606s with unchanged120s limit, then remaining276 at 2925.095s. Exact-name union in `resume-01/baseline/debug-aggregate-277.json`; not one uninterrupted full invocation |
| Resumed ASan/UBSan | **277/277 PASS**,0 failures,0 skips;966.032s | `resume-01/baseline/sanitize-full-01/result.json`; rebuild183.023s; original-input and tested-binary guards pass |
| Fresh alpha Release tests | **277/277 PASS**,0 failures,0 skips;176.59s CTest wall time | `resume-01/alpha/stages/test-release-01/state.json`; public alpha functions called by the resumed private controller, not an unmodified full CLI run |
| Final QA focus | **27/27 inner cases PASS in each configuration** | Registered `macos-acceptance-harness` rechecked in Release/Debug/San,0.283/0.287/0.283s. `resume-01/baseline/summary.json`; these cases are inside one registered CTest, not27 additional CTests |
| Resumed alpha / local package | **PASS within explicit headless scope** | `resume-01/alpha/alpha-continued-report.json`, pending[]: exact Debug/San baseline reuse; fresh Release277,100k ticks in each configuration,3000 Debug/San frames,100 menu sessions, original-data smoke and package checks. The interrupted original CLI is not reclassified as a successful unmodified invocation. |
| Original map census | **PASS within initialization scope**:167 complete managers,165 prepared,2 known conflicts,1485 policy/mask results | `run-01/map-census-scope-analysis.json`; zero Worlds/ticks/render frames |
| Authored historical bytes | **62/62 equal** to retained references; actual files span schemas1–26 | `run-01/historical-release-comparison.json`; checked again read-only on resume |
| Current originals / protected earned inputs | **PASS**:1464 originals and 7 protected earned inputs match exact guard pins | `resume-01/evidence-audit.json`; no new game/test execution by this audit |

The Release and census broad source guards originally reported a difference in
the excluded, on-demand `SystemPlaytestAcceptance.cpp` while that QA tool was
being completed. That exception is preserved in the scope analyses; it is not
presented as a wholly unchanged broad guard. Tested binary sources, production,
resources and original bytes/modes/mtimes remained unchanged. The final seven
runtime journeys closed their own source/binary/input guards successfully.

The interrupted October 26 Debug attempt remains preserved:102 passes, one menu
timeout at 120.04s, and two endurance processes stopped by the explicit pause.
The menu subsequently passed alone under the same120s limit. The closed split
schedule covers all 277 names; the original timeout's cause, including any
parallel-load contribution, remains unproved. Generated BuildInfo revisions are
retained truthfully in the summary; a QA-only HEAD change was not disguised by
reconfiguring binaries solely to change their displayed revision.

Compatibility evidence spans save schemas1–30 through different checks; it is
not an exhaustive test of every possible historical save. The62 authored files
cover every schema1–26 and selected frozen profile/rule pairs with rule numbers
1–10. Separate paid Bridge21
and Ramp22 continuations exercise active transport; Rule11/schema27 paid fire
persistence is covered by the actual Release tests. Current Rule12/28,
Rule13/29 and Rule14/30 earned continuations compare complete documents and
snapshots in new processes. Exact markers, cross-generation refusals and hostile
saves are covered by registered core/menu tests. Rule versions are profile-bound
and currently reach14, not30. No OE41 migration was introduced; the documented
historical schema1 production adapter retains its pre-existing contract.

## Scenarios B01–B20

`PASS` below is scoped to the stated execution layer. Paid World fixtures use
ordinary charged production commands and actual ticks; they are distinct from
the real Menu/SandboxView journey, native OS input and human play. Partial rows
retain the listed unexecuted or failed subcases.

| ID | Status / actual layer | Observed result and remaining scope | Evidence |
|---|---|---|---|
| B01 Data/start | **PARTIAL**, scripted Menu | Invalid directory refused; supplied original root accepted unchanged. Native folder chooser and missing-required-Model/SG3 product journeys remain NOT_RUN. | `journey-create-02/report.json`, `data-refused.png`; Release model/parser negatives are separate contracts |
| B02 Menu/rules | **PASS**, scripted Menu | All16 ordinary profile toolbars, explicit10–14 entries, five difficulties, paid/empty choices, Back and leave/upgrade/delete confirmations visited. General default stays City11/rule3; ordinary City16 stays rule4. | `inventory-runtime-04/report.json` |
| B03 Navigation | **PARTIAL**, scripted SDL and native keyboard/window | Actual wheel 1×/2×/4×, Step, speeds, Help, focus loss/gain and pure hover/render. Native held-arrow pan, keyboard 1×/2×/4×/1× and real window resize preserve full save bytes; OS fullscreen observed separately. Road-drag/outside/modal negatives remain open. | `runtime-qa/summary.json`; `resume-01/native-01/camera-purity.json`; registered input contracts |
| B04 Paid placement | **PARTIAL**, scripted Menu/View plus paid fixtures | Road $2 and House $80 bought with exact funds/footprint/ID changes; duplicate refusal atomic. Four maps buy Road $2 and refuse actual protected structures. Native starter construction is paid, but native pointer Road/House purchase was blocked by automation delivery; no insufficient-Funds or successful native drag subcase claimed. | `journey-create-02/report.json`; `map-*-02/report.json`; `resume-01/native-summary.json` |
| B05 Clay chain | **PASS**, finite paid World / scripted purchase corroboration | Four original-map technical cities produce12 Clay and 5 Pottery by400; supply scenes produce50 Clay/23 Pottery by1600. Actual source/target trips complete. Current paid starter tests also verify House goods consumption and repair. No dispatch-only delivery claim. | `map-paid-summary.json`; Release `original-paid-starter-real`; `paid-evidence-summary.json` |
| B06 Food/Hemp | **PASS**, finite paid World | Paid farms and separately owned fields feed real arrivals. Rule12 post-maturity100k observes5890 Food units/2041 arrivals and 4412 goods units/2500 arrivals, with no interval Food misses. Rule-specific farm/attachment refusals are registered contracts. | `paid-evidence-summary.json`; Release `original-sustainable-economy-world`, food/hemp tests |
| B07 Market logistics | **PASS**, finite paid Rule13 | Actual loaded/delivered/returned transfers, stock conservation and Common10 needs are recorded. Target has 400 supplied evaluations in 20400 ticks; no indefinite fairness claim. | Rule13 creator/replay reports and `trace-retention.json`; `rule13-separate-process-comparison.json` |
| B08 Operation/workers | **PARTIAL**, scripted inspector plus paid core | Market7 pause/resume and High/Low/Normal execute normally; installed 4-worker requirement remains. New journey does not hold the Market paused across an 816-tick invoice. Registered paid finance/starter tests separately exercise pause/upkeep. | `journey-create-02/report.json`; Release `original-paid-starter-real`, finance tests |
| B09 Water/medical/religion | **PASS**, finite paid World | Actual arrival witnesses/timers are retained. Rule12 interval records 991 Water and 923 service applications; Rule13/14 traces retain medical/religious arrivals and expiry/devolution. Paid service/Ancestor pause/repair negatives pass in Release. | `paid-evidence-summary.json`; Release services/Ancestor/starter tests; OE40 causal evidence |
| B10 Culture | **PASS for authored school→House contract**; original venues **UNSUPPORTED** | Actual Music/Acrobat/Drama visits and separate96 clocks/expiry are checked with current real model; earned Rule14 input retains their arrival witnesses. This does not establish original venue/performer simulation. | Release `original-culture-world-real`, `original-culture-save`; OE40 first-success input |
| B11 Common housing | **PASS**, finite paid Rule13 | Real9→10 at 22837; target stays occupied, non-burning type10 for20400 ticks,400 evaluations and no current unmet needs/devolutions. Other buildings still burn; later buffer exhaustion remains unproved. | `paid-evidence-summary.json`; Rule13 `retention.json`; full new-process comparisons |
| B12 Elite housing | **PARTIAL**: entry PASS, sustained supply/finance FAIL in measured recipe | Rule14 paid2×2 plot materializes4×4; actual16→17 at 7333 with raw70+10=80 and both needs rows met. The following20400 ticks lose type/supply/fire retention and 3241 operating Funds. Historical Rule12/13 DES≤79 remains unchanged. | [OE40 investigation](../original-economy/oe40-desirability-investigation.md); current Rule14 replay / six full comparisons |
| B13 Fire/Watch | **PASS for bounded contract**, finite paid World | Rule11 paid Watch/staffing/pause/arrival/prevention/extinguishing/self-fire/repair tests pass. Rule10 still refuses Watch. Rule12 longplay records 1395 inspector arrivals; Rule14 actual fires remain visible limitations, not omitted outcomes. | Release fire-protection tests; `paid-evidence-summary.json` |
| B14 Road delete/repair | **PASS for paid transport contract**; new Menu drag journey NOT_RUN | Paid loaded trips retain cargo/begun-edge safety through interruption and charged repair. Real starter evidence records occupied-edge refusal, repair cost8 and repaired arrival4319. Four maps complete dispatch/arrival/home. | Release `original-paid-starter-real`; `map-paid-summary.json`; Bridge/Ramp disruption checkpoints |
| B15 Bridge/Ramp | **PASS for separate bounded profiles** | Original-map Rule5/schema21 bridge and authored-map Rule6/schema22 ramp each run12000 paid ticks with real crossings/returns, disconnection/repair and exact restarts. No admitted original-map height slope was found; original ramp233–244 remains a separate prototype. | `geometry-restarts/summary.json`; `bridge-paid-fresh/command.json`, `ramp-paid-fresh/command.json`; geometry captures |
| B16 Maps/rendering | **PASS within sampled scope** | Xia/Handan/Badaling/Kaifeng actual Menu→City16/rule4 Empty→1×/2×/4×; charged road, protected-source refusal, pure frames and full F5/F9. Four separate paid technical scenes demonstrate transport. Local crops do not prove whole-map geometry parity. | `map-*-02/report.json`; `map-paid-summary.json`; `runtime-qa/visual-review.json` |
| B17 Inspector/help | **PARTIAL; P2 visual subcase FAIL** |65/65 paid bodies selected,52 House tab visits and 35/44 couriers selected without World changes. Six idle and three sampled-unselectable live actors remain NOT_RUN. Full F2/F3 clip controls/browser native paths remain open. Market7 list/hints overlap controls. | `inspection-paid-02/inspection.json`; `runtime-qa/observed-ui-issue.json`; opened1×/2×/4× images |
| B18 Save/restart/recovery | **PARTIAL**, scripted Menu, native and complete paid core | Scripted901 city has loaded Clay/typed Hemp, exact F5/Step/F9/F5 and Recovery; new process continues160 ticks. Native Rule12 at 7952/167 residents has exact F5/Step/F9/F5 and a third-process full-document restore including both Hemp cargo/reservations. Periodic-autosave and corrupt/cross-version new product journeys remain open; registered hostile-save/menu contracts pass separately. | `journey-create-02`, `journey-resume-03`; `resume-01/native-02/native-f5-step-f9.json`; `native-03/active-cargo-process-restore.json` |
| B19 Longplay/stress | **PASS for finite paid longplay and named headless stress** | Rule12 runs100000 post-maturity ticks; Rule13 Common10 runs20400; Rule14 reproduces failed retention. Fresh alpha endurance runs100000 ticks/12 JSON roundtrips in each configuration; Debug/San each render3000 frames and 100 Debug menu sessions close. No native FPS or indefinite-supply claim. | Financial table; paid summaries; `resume-01/alpha/alpha-continued-report.json` |
| B20 App cycles | **PASS for executed scripted and native cycles** | Scripted Rule14/Xia→City11/rule3 Handan resets to exact distinct authority. Third native process restores active Rule12/Xia, then same-process menu loads Rule5/schema21 Chang-an Sui, tick0/Funds80, with full save equality. Native03 remains live under user control after takeover; no completed-process exit/guard PASS inferred. | `journey-create-02`, `journey-resume-03`; `resume-01/native-03/{active-cargo-process-restore,different-rule-save-restore}.json` |

## Paid state and finance

These are actual taxes minus all installed-worker upkeep in the named interval.
They exclude separately recorded construction. Positive remaining Funds alone
are not operating profit. Warm-up and lifetime misses are retained in raw metrics.

| Contract / interval | Population / workers | Funds begin→end | Taxes / upkeep / operating cashflow | Needs / continuity |
|---|---|---:|---:|---|
| Rule12,20000→40400 |318 residents;120/120 assigned/required |11785→13735 |7950 /6000 / **+1950** | One full20400-tick current-needs window |
| Rule12,20000→120000 |318 at end;120/120 |11785→21379 |39114 /29520 / **+9594** |0 current-unmet evaluations,0 devolutions,0 interval Food misses;42 lifetime warm-up misses remain recorded; higher/next needs are not universally met |
| Rule13,22837→43237 |429→489 residents;190/190 at end |10995→15355 |13860 /9500 / **+4360** | Target Common10:400 evaluations,0 lost ticks/current unmet;42 fires elsewhere in city |
| Rule14,7333→27733 |340→354 residents |4455→1214 |14009 /17250 / **−3241** |400 evaluations;15488 lost-target ticks,161 current-unmet,324 next-unmet,4200 burning ticks |

Rule13 uses an earned buffer: physical Hemp stock plus cargo falls94→67.
Actual lifetime production/consumption is410/316 at entry and903/836 at the
endpoint; both ledgers conserve stock. The20400-tick retention therefore does
not prove indefinite Hemp replenishment or self-sufficiency.

Rule12 new-process equality covers three full save/snapshot pairs, including the
100k endpoint, with equal metrics. Rule13 covers both endpoints plus equal
metrics/transport trace. Rule14 compares all six complete checkpoints at 7333,
8160,16320,17533,24480 and 27733 plus Recovery. The OE40 accepted continuations
also cover Release, Debug and sanitizer configurations; those feature-specific
continuations are separate from the closed OE41 registered matrix above.

The actual Rule14 entry uses the explicit immutable stored-raw-terrain adapter.
Its successful site's bit4 neighbors include stored markers outside the shared
Diamond. It does not establish a visible shoreline, original post-load terrain
processing or original Emperor Elite17 progression. The
[source investigation](../original-economy/oe40-desirability-investigation.md)
records that boundary.

Bridge Rule5 ends12000 ticks with 36 residents,2295 Funds,4075 taxes and 1580
upkeep; its one-cell span costs60. Ramp Rule6 ends12000 ticks with 30 residents,
1082 Funds,2950 taxes and 1528 upkeep on **authored** terrain. Twenty-one separate
processes compare42 complete immediate/+100-tick documents. Six actual software
captures were opened. Original ramp-prototype coverage has 432 cases/36 stamps
and three opened representative images;996 fresh PNGs are byte-identical to
the reviewed run (`original-ramp-prototype-fresh/complete-png-comparison.json`).
It has no productive World authority.

## Visual, native and human acceptance

| Layer | Current status | Actual scope |
|---|---|---|
| Scripted software app | **PARTIAL** | Real MenuSession/SandboxView events and exact paid states; missing/failed subcases remain above |
| Opened software images | **PASS within selected views** |21 runtime images, six Bridge/Ramp images and three prototype representatives opened; no original-pixel or whole-scene parity claim |
| Native visible runtime | **PARTIAL** | Three actual CMake.app processes: paid starter, held-arrow/zoom/resize, OS fullscreen, keyboard play to7952, exact F5/F9 and independent active-goods restore, then distinct Rule5/map load. Native pointer construction blocked by observed delivery mismatch; third process left under user control |
| Local package / relocation / ZIP | **PASS, headless** | Fresh package277/277 with 0 skips;21 negative controls; moved and unzipped bundles retain valid ad-hoc signatures. Finder/Gatekeeper and independent clean-Mac acceptance remain NOT_RUN. |
| Human playtest | **NOT_RUN** | [Supplied human checklist](../../OpenEmperor_OE39_OE40_Playtest/OE41-manual-checklist.md) remains unfilled |

Software scene images contain documented authored markers/fallbacks. The paid901
captures intentionally retain exercised F4/F6/F10 comparison state. Earned7333
and four-map captures use their ordinary selected visual profiles. These images
do not establish native delivery, Finder/Gatekeeper, notarization or human
acceptance. The following native observations have their own evidence.

### Native app observations

`resume-01/native-summary.json` records the three Cocoa/Metal sessions and image
hashes. The first two processes exited0 and closed their input guards unchanged.
The third has no completed-process result: it remains live under user control.
The CMake.app was built from the clean resume point plus QA/docs changes;
production and resources retain the OE40 baseline. It uses system dependencies
and is not evidence for a relocated distributable package.

Native01 created the real Normal Rule12/Xia starter:19 paid bodies, construction
1404, tick0/Funds10596, command sequence87. A700ms held Right key, keyboard
zoom1×/2×/4×/1× and mouse resize from1100×733 to1292×850 outer-window size
left the complete F5 document byte-identical. The SDL event dimensions after
resize were1292×818. The German QWERTZ automation's physical `y` delivered SDL
`Z` for zoom; this observation caused no hotkey patch. See
`native-01/camera-purity.json` and `native-input-events-analysis.json`.

The requested native pointer point `(368,591)` arrived in SDL as
`(0,289.47265625)` before the application's conversion; converted and raw
coordinates were equal. Later pointer/tool attempts also failed delivery.
Native Road/House mouse construction is **BLOCKED_AUTOMATION_DELIVERY**, with no
successful purchase claimed. This establishes an automation-delivery mismatch,
not an app DPI/camera defect or permission to add a coordinate offset.

Native02 used the OS accessibility fullscreen control, capturing5120×1440;
there is no in-game fullscreen menu action. Actual `L`/Return loaded the first
process's complete tick0 save exactly. Keyboard speed changes and Space advanced
the paid city to7952:19 bodies,15 couriers,167 residents, Funds9604, tax718,
upkeep1710 and operating delta−992. Construction1404 and command sequence87
remained unchanged. This warm-up interval is shorter than20400 ticks and is not
a successful full financial-retention window. Population comes from
`original_economy` households; legacy population/cargo fields are zero under
this contract and cannot describe this city. Two actual `original_goods` Hemp
cargo rows contain2 units each, with matching active reservations.
`native-02/native-f5-step-f9.json` verifies exact full bytes after F5/Step/F9/F5.

Native03 loaded a private working copy of that active save in a new process and
F5 reproduced all bytes, including the 167 occupants and typed cargo/reservations
(`native-03/active-cargo-process-restore.json`). In the same process,
Escape/`L`/Down/Return then selected Rule5/schema21 Chang-an Sui at tick0/Funds80;
F5 and the captured HUD agree with the complete expected document
(`different-rule-save-restore.json`). CUA then reported **“The user changed
app”** before the F1/F2/F3 attempt. All further desktop control stopped. The
post-takeover live state was not inspected, no quit/termination was sent, and
takeover is not human acceptance. Native F1/F2/F3 and browser journeys therefore
remain **NOT_RUN**.

## Local package result

The fresh package pipeline and an independent report/JUnit audit pass:
`resume-01/alpha-dist/package-report.json` and
`resume-01/alpha/package-audit-02/result.json`. Its own current Release suite is
277/277 with 0 failures/skips; all 21 malformed-bundle controls reject. Relocation
outside the repository, a restricted runtime environment, ZIP extraction,
synthetic and real-data supply/tax/process-restart checks pass. The bundle
contains the executable, two actual non-system dylibs, matching notices and ten
bounded compatibility JSONs; no proprietary payloads or saved cities are staged.

The local arm64 candidate targets macOS26.0 and uses SDL3.4.14,
OpenSSL4.0.3 and nlohmann/json3.12.0. Its exact generated revision is
`f10d31dabd64`, dirty due to QA/docs at configuration time. The output is
`.local/oe41/resume-01/alpha-dist/OpenEmperor-0.1.0-alpha.2-f10d31dabd64-macos-arm64.zip`,
5004865 bytes, SHA-256
`97fde4cf39f94c7f4ebff18e6232d5cbd57e27d232d958c22babc2ae48b7203d`.
All final output hashes are retained in the private index.

The signatures are **ad hoc**; there is no Developer ID/notarization result.
Finder/Gatekeeper and an independent clean Mac remain **NOT_RUN**. The project
still lacks its own selected LICENSE, recorded by the package as a publication
blocker. This is a local development candidate, with no push, release or
publication. These package observations do not turn the separately tested native
CMake.app into a packaged-desktop acceptance result.

## Observed issues and preserved QA incidents

| Finding | Classification | Reproduction / effect | Next step |
|---|---|---|---|
| OE41-UI-001 | **P2, confirmed; regression not established** | Rule14/Xia, tick901, Market7,1280×800: building list/demolition hints overlap operation/priority controls/footer. Controls execute, text is not cleanly readable. `journey-create-02/paid-city-1x.png`, also2×/4×. | Reserve/clip one shared list/detail/control region in `SandboxView.cpp` around6320–6385; add a focused UI regression. No production fix made here. |
| Debug menu timeout | **Preserved failure; cause unresolved; product severity not assigned** | `original-menu-session-flow` exceeded120s before pause, then passed alone in 96.606s with the same binary and limit; resumed split coverage is277/277. Two pause interruptions are distinct. | Retain both invocation identities and the unchanged120s contract; parallel-load causation is not established. |
| Native pointer delivery | **Automation limitation; app defect not established** | Requested screenshot point and SDL raw point differ before application conversion; raw and converted agree. No native paid drag succeeded. | A future user-authorized native pointer test must first establish delivery; no speculative DPI/camera workaround. |
| Rule14 supply/finance | **Known measured gameplay limitation** | Paid entry reaches17 but subsequent Food/medical/fire losses and negative operating cashflow remain. Two additional Markets still miss Tea at the House. | Investigate actual arrival/allocation/fire/service evidence; no balance/source expansion in OE41. |
| Two original occupancy conflicts | **Known conservative refusals** | Luoyang WJin and MPcanal4 fail complete-occupancy consistency;165 prepared maps are not165 proven playable cities. | Preserve refusal; independent source evidence required before changing admission. |
| Original shipping/NPC/venues/illness and terrain lifecycle | **UNSUPPORTED / partial original parity** | No complete productive integration; original ramp prototype is isolated. | Keep explicit scope; do not count missing systems as new implemented-game failures. |

OE42 follow-up (October 29): **OE41-UI-001 is fixed in the scripted/visually
reviewed scope.** The shared inspector layout reserves fixed controls and hint
bounds, clips drawing and row hits together, and scrolls all list/detail text.
The actual paid901 Market7 reproduction and paid7333 city cover all three
1280×800/1600×900/1920×1080 sizes, camera 1×/2×/4×, Warehouse, FireWatch and all
House sections. The901 city has no FireWatch; that coverage comes from7333.
Complete World snapshots and canonical F5/Step/F9/F5 documents remain exact;
intentional operation clicks are compared with the same reference-World commands.
Release, Debug and ASan/UBSan each pass the 34 selected registered tests with
zero failures/skips. Actual top/detail/end captures were opened and checked;
private evidence is under `.local/oe42/`. Rules1–14, save schemas, economy and
original bytes are unchanged. This closes the specific inspector defect, while
OE41 native/human/release gaps remain separate. See the maintained
[layout/input contract](../input-reliability.md#inspector-layout-and-scrolling)
and [regression procedure](../testing.md#inspector-layout-regression).

No confirmed production P0/data-loss finding is established by this report.
Initial QA failures are preserved: legacy-pause and deferred-confirmation
assumptions, compile diagnostics, and external-load/F5 isolation. The first
resume wrote to its selected **private OpenEmperor** creator path. Its changed
result was retained,901 creator bytes restored from the intact manual save, and
the OE40 source save's exact metadata restored against its pre-run pin. The
corrected harness loads a working copy; successful guarded reruns and this
resumed audit verify protected input pins exactly. No original game file was
written. This was a repaired harness isolation error, not a World save defect.

## Performance and final scope

The initialization-only map census took138.963s with measured peak RSS242728960
bytes. It created no Worlds and rendered no frames, so it is not an FPS result.
Paid100k/restart runs and registered logical resource budgets are separate
simulation evidence. Resumed headless alpha stages call the public alpha
functions through a private stage controller; they are not a successful
unmodified invocation of the entire alpha CLI. Exact leases revalidate the
closed Debug1+276 and sanitizer277 results, without counting them twice. Fresh
Release277, Release100k endurance, sanitizer100k endurance with 12 JSON roundtrips,
and 3000 sanitizer SandboxView frames pass. Frame stress has zero simulation
ticks and zero decode, texture-upload and stored-order-build deltas after source
files become unavailable; it is not a native FPS benchmark. Actual supplied-data
Industry/City11 CLI smoke also passes and renders frames. These generic alpha
fixtures do not extend the measured Rule12/13/14 financial windows.

The first resumed Debug alpha attempt was interrupted for CPU coordination;
the next was stopped at a known whole-command1800s runner-budget boundary.
Neither is counted as another complete suite or a gameplay assertion failure.
The QA repair in `f10d31d` derives an aggregate49765s allowance from registered
case limits and an untimed-group allowance. Individual case limits, including
the menu's120s, were not changed. Commit `cc3b033` and the final27-case focus
retain optional-data, refusal and aggregate-budget controls. See
`resume-01/alpha/aggregate-budget-control.json`, preserved stage stop records
and `baseline/summary.json`. Fresh Debug100k endurance (460.25s),3000-frame stress (92.07s) and 100
dummy-driver menu sessions (385.74s) also pass. Each session returns its tracked
textures to zero. The final index records 645 source/dependency entries,1464
original files and the selected200 Debug plus 200 sanitizer binaries unchanged,
with no agent-owned headless process groups left. It retains every interrupted
attempt and the actual generated revisions: Debug2f433-dirty, Release/San
e752aa9-dirty and packagef10d31d-dirty. No memory-leak or native FPS promise
follows from this finite, scoped evidence.

### Human handoff using a working copy

The current native03 app belongs to the user. Do not automate, quit or terminate
it to perform this handoff. After the user finishes and closes that app, the
following commands may be run manually from the repository root. They create a
new app root and copy the earned native02 save; F5 will then write only the
working copy. Never load the protected earned source directly for further play.

```sh
oe41_human_root=$(mktemp -d "$PWD/.local/oe41/human-XXXXXXXX")
mkdir "$oe41_human_root/saves"
cp "$PWD/.local/oe41/resume-01/native-02/native-after-play.save.json" \
  "$oe41_human_root/saves/human-working-copy.json"
"$PWD/.local/oe41/resume-01/native-app-build/OpenEmperor.app/Contents/MacOS/OpenEmperor" \
  --data "$PWD/.local/gog-extracted/app" --app-root "$oe41_human_root"
```

Use `L` and Return to load `human-working-copy.json`, then follow the
[human checklist](../../OpenEmperor_OE39_OE40_Playtest/OE41-manual-checklist.md).
The starting state is paused Rule12/schema28, tick7952/Funds9604,167 residents
and two active Hemp cargo rows. Inspect actual arrivals, needs and cashflow;
record the interval and screenshots rather than declaring acceptance from the
starting Funds. This is a local CMake.app handoff, not a packaged/Finder claim.

All 14 persisted native PNGs were opened. Reading the two saved fullscreen
frames after takeover performed no desktop control.

Current assessment: **REGISTERED_CORE PASS**, **TECHNICAL PASS within the registered/headless scope**, **SCRIPTED_APP PARTIAL**,
**NATIVE_GUI PARTIAL**, **HUMAN NOT_RUN**, **RELEASE_READINESS PARTIAL**.
These are an audit snapshot, not release approval or publication.

The next three tasks follow the evidence:

1. Fix OE41-UI-001 using one shared clipped list/detail/control region and the
   actual paid901 Market7 reproduction.
2. Establish correct native pointer delivery, then finish the open construction,
   focus/gesture, persistence and human checklist cases. This is an acceptance
   gap; no speculative DPI/camera change is justified.
3. Investigate the measured high-tier Food/Hemp/Tea and medical/fire delivery
   deadlines and operating deficits. Preserve finite buffers and actual arrivals;
   any proposed balance change needs a separate versioned contract.

These are follow-up tasks, not three newly confirmed product defects. Public
release readiness also remains blocked by the explicit licensing/signing/human
limits above. No push, merge, tag, release or publication is part of this result.
