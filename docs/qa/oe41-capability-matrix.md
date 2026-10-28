# OE41 capability and execution matrix

The baseline is `2f433d2af63b8c545e78ab7b3703570f1a3a397c` on
`test/oe41-system-playtest`. This catalog describes the product that exists at
that baseline. It does not add gameplay, migrate a save, or treat source reading
as a completed playtest. The machine catalog is ignored
`.local/oe41/inventory.json`; raw reports, complete saves and captures remain
under `.local/oe41/run-01/`.

`available`, `visible`, `enabled`, `executed` and the observed result are separate
fields. A successful construction-selector event establishes tool selection;
it does not establish a paid purchase, route, delivery or sustainable city.
`UNSUPPORTED` identifies an absent system. An existing but unexecuted action is
`NOT_RUN`.

## How the inventory is obtained

`tools/SystemPlaytestAcceptance.cpp` drives the real `MenuSession` and
`SandboxView` with queued SDL events. It enumerates actual
`SandboxView::layout().buttons` for all16 ordinary profiles, plus explicit
Original Rules10–14 on both tool pages. Selector events retain the complete
World snapshot. Actual enabled state is observed from activation; the layout's
default enabled flag is retained separately and is not an affordability claim.

Menu candidate labels and identities come from `MenuSession::rebuild_buttons`
and its action registration. Public diagnostics confirm visible rectangles,
action identities and enabled states. Bounded hit sampling discovers dynamic
map/save/recovery rows. There is no public complete label enumerator: observed
action IDs and context rows are not advertised as a complete independent count
of all menu functions. Conditional controls retain their own execution status.
Hotkeys and CLI branches are cataloged from their real handlers; source-only
entries remain `NOT_RUN` until a corresponding current invocation is recorded.

The October28 metadata audit preserves the original272-entry catalog at
`.local/oe41/resume-01/inventory-before-extension.json` and extends the working
catalog to329 records. The57 added records cover AssetBrowser, MapBrowser,
MapDebugView and SceneView keys/window/pointer handlers and the four delegated
`--menu-check` modes. All48 top-level main-parser flags remain represented.
Every entry now has an explicit numeric or context-dependent profile/rule scope;
319 records link related registered CTests and their actual Release result.
Those links describe supporting contract coverage, not execution of the same
native/app action. The app-action totals remain60 `PASS`,261 `NOT_RUN` and8
`UNSUPPORTED`; none of the new browser/child-mode rows was marked executed by
source inspection. Prototype records remain separate from productive gameplay.
The bounded menu/toolbar traversal passes, while complete browser/native
interaction coverage remains partial. Exact checks are in
`.local/oe41/resume-01/evidence-audit.json`.

The final runtime traversal is
`run-01/inventory-runtime-04/report.json`:569 menu context rows and708 toolbar
context rows, with67 observed menu action IDs (including dynamic rows),57 toolbar
action IDs and399 actual construction-selector events. These are contextual
observations, not1277 distinct product features. Later reports extend this
inventory rather than replacing unexecuted entries with blanket PASS results.

## Product surfaces

| Surface / entry point | Identity and classification | Current execution evidence |
|---|---|---|
| Data setup, main menu, map/rule choice, Back/Cancel, manual and recovery lists | `user_exposed`; actual `MenuSession` actions | Scripted runtime traversal; individual conditional actions in machine catalog |
| Ordinary profiles LogisticsV1 through CityV16 |16 `user_exposed` choices; general default CityV11/rule3 | All16 choices and actual runtime toolbars visited |
| Ordinary CityV16 | Rule4/policy2/schema20, `user_exposed` | Toolbar visited; four-map journeys recorded separately |
| Bridges and Ramps | Explicit Rule5/schema21 and Rule6/schema22, policy3; `user_exposed` bounded authored transport | Runtime availability is distinct from separate paid transport fixtures |
| Original economy entries O/F/S/M/W | Explicit Rules10/11/12/13/14, schemas26/27/28/29/30, policy3/revision27; `user_exposed` | All five setups, five difficulty choices, paid/empty choices and both actual toolbar pages visited |
| Historical original Rules7/8/9 | Exact schemas23/24/25; `debug_only` fixtures/restore | No normal new-game menu entry; compatibility validation separate |
| Main construction toolbar | Roads/removal, Clay/Pottery/Warehouse/Market/Common, farms/owned fields, Water, religion, health, culture, influence and gated FireWatch | Actual per-profile selector events; purchases and supply require their own B04–B13 evidence |
| Industry page | Logging, metal/lacquer/paper/silk/tea chains, owned plants and ElitePlot | All current selectors visited; no stock or resident authority inferred |
| Clock, operation and staffing | Pause, Step, speeds1/2/4, permitted operation and workforce priorities; `user_exposed` | Selector inventory does not itself certify operation or upkeep behavior |
| Inspector, HUD, overlays and Help | Building/actor selection; original House State/Needs/Goods/Visits; actual funds/tax/upkeep and profile overlays | B17 paid inspection and pure frame/hover checks recorded separately |
| Save, Load and Recovery | Actual F5/F9, manual files, history and exact-generation restore; `user_exposed` | Complete documents and independent-process journey evidence required |
| F1/G, F2/F3/F4/F6/F7/F8/F10 and clip-inspector controls | `debug_only`; context-sensitive event branches | Each unexecuted branch remains `NOT_RUN`; no construction authority |
| Asset/map/image/scene browsers and diagnostic CLI modes | `debug_only`; accepted branches in `src/app/main.cpp` | Source-registered CLI catalog; current invocations reported individually |
| Authored terrain sides and joined ramps | Explicit presentation option / isolated QA fixture | Session presentation is distinct from paid transport and source parity |
| Original ramps233–244 | `isolated_prototype` executable | Separate prototype result; never productive Rule6 acceptance |
| Great Wall, quarry and shore landing | Bounded presentation/material scope | No wall/quarry/landing construction or shipping authority |
| Original savegames/campaigns, original NPC economy, shipping/Ferry, venue/performer and illness lifecycle | `unsupported` complete systems | Not reclassified as newly failing implemented gameplay |
| Fullscreen menu action | `unsupported` in current registered UI | Real window resize is a separate supported input case |

## Execution layers and preserved failures

The runtime diagnostic uses SDL's dummy video driver and software renderer.
Its events are scripted application evidence. Captures become visual-review
evidence only after opening and inspecting them. Native pointer/window delivery
and a human playtest remain independent results; neither is inferred from this
catalog or from a CTest count.

The October28 native continuation is **PARTIAL**, not wholly unexecuted:
three actual Cocoa/Metal CMake.app processes exercised paid Rule12 starter
creation, held-arrow/zoom/resize, OS fullscreen, keyboard play to7952/167
residents, complete F5/F9 and new-process active-Hemp restoration, then a
same-process Rule5/Chang-an Sui load. Pointer construction was blocked by an
observed automation-delivery mismatch before app conversion. CUA reported
“The user changed app”; all later desktop automation stopped and the third app
remains live under user control. Human acceptance is **NOT_RUN**. These later
observations do not blanket-change the source-only browser/catalog rows to
PASS. Exact scope and gaps are in [the playtest report](oe41-playtest-report.md)
and `.local/oe41/resume-01/native-summary.json`.

The current guarded binary is SHA256
`3c60e4dc78f82097954cec073071da4309fcf8797694176f3912787f46fc6f7f`.
It was built from the unchanged production baseline plus QA commit
`e752aa994c5fd803d13151db73379946e0bb2096` and the new diagnostic/CMake changes.
Exact commands, dependency paths, source/binary hashes and protected-input pins
are in `run-01/runtime-qa/*.execution.json`. All1464 original files match the
frozen baseline in bytes, size, mode and mtime before and after these runs.

| Scripted scope | Actual evidence and limits |
|---|---|
| B01/B02 data and setup | Invalid directory refused, supplied read-only root accepted; all16 ordinary profiles and explicit Rules10–14 visited. The native OS chooser and missing-Model/SG3 product subcases were not exercised here. |
| B03 navigation and input | Real wheel1×/2×/4×, Step, speed-button events, Help, focus loss/gain and pure hover/render checks. Held-arrow delivery, real resize, drag cancellation and native input remain separate/unexecuted subcases. |
| B04 purchases/refusals | Actual Road2 and Common House80 purchases, complete footprint/ID/funds accounting and duplicate-placement refusal. Each of four maps also bought one Road2 and refused an actual protected source cell without snapshot mutation. Insufficient-Funds and drag cases remain unexecuted here. |
| B08 operation/staffing | Paid Market7 paused/resumed through inspector, High/Low/Normal selected; installed four-worker requirement retained. The full city advances900 real TickDriver ticks. This does not certify a paid upkeep boundary held continuously while paused. |
| B17 rich-city inspection | Actual earned Rule14 city at7333:65/65 paid bodies selected,52 State/Needs/Goods/Visits tab visits, original House overlays and35/44 courier selections. Six idle couriers have no live frame; three live couriers had no selectable sampled pixel. These nine rows remain `NOT_RUN`, not universal picking success. |
| B18 manual/new-process/recovery | At901 the paid city has20 bodies,15 couriers,71 residents and10346 Funds, including a loaded Clay courier and two Hemp units in typed cargo. Actual F5/Step/F9/F5 and menu Recovery retain complete snapshots and canonical SaveDocument bytes. Independent process loads the901 working copy, saves byte-identically, advances160 ordinary ticks to1061/79 residents, and repeats F5/F9. |
| B20 repeated sessions | In one process Rule14/Xia is replaced through the real menu with City11/rule3 Handan, distinct source SHA, tick0,10 bodies and100 Funds. The independent restart is recorded separately. |
| B16 four original maps | Xia, Handan, Badaling and Kaifeng each start through actual Menu→City16/rule4/policy2 Empty, render1×/2×/4×, preserve World during hover/render, charge1300→1298 for one Road, reject a protected original structure, and pass complete F5/F9 equality. Original-road ground counters are56/194/227/288 respectively. Empty-city rendering is not a transport or whole-map parity proof. |

Current proof directories are `journey-create-02/`, `journey-resume-03/`,
`inspection-paid-02/` and `map-{Xia,Handan,Badaling,Kaifeng}-02/`. All12 four-map
zoom images, the three earned-city images and six current create-journey images
were opened. The paid901 captures intentionally retain the exercised F4/F6/F10
debug comparison state; they are not normal-art parity evidence. The earned7333
and four-map captures retain their ordinary selected visual profiles. No image
review constitutes native or human acceptance.

An observed **P2 inspector layout defect** remains: at1280×800, selecting Market7
in the paid20-body city lets the long building list and demolition-hint text
overlap the operation/priority controls and footer. Repro: the current create
journey, tick901, select Market7; see `journey-create-02/paid-city-1x.png` (also
visible at2×/4×). The controls still execute correctly, but the text is not cleanly
readable. Relevant production code is `SandboxView.cpp`'s building-list/detail
and operation/demolition drawing around lines6320–6385. No production fix was
made during this playtest, and a regression relative to an earlier build was
not established.

Initial `inventory-runtime-01` and`-02` failures are preserved. The first harness
incorrectly required historical profiles to begin paused; legacy views retain
their actual default and the harness now uses normal Space before inspection.
The second omitted the normal dirty-world replacement confirmation after
deferred loading. Its correction visits and resolves that real menu modal.
These are QA assumptions, with no production rules or expectations weakened.

The first separate-process attempt also exposed a QA isolation mistake:
F5 normally writes to the externally selected save path. The harness had loaded
the protected creator directly rather than a working copy. Its tick1061
result is preserved; the901 creator bytes were restored from its intact actual
manual save. The inspection attempt wrote identical bytes to the supplied OE40
save; its exact mode and mtime were restored and checked against the existing
`gate-c-lean-rechecks-03/guard-before.json` pin. The current harness copies input
before menu loading. Guarded corrected runs verify original and protected-save
SHA/size/mode/mtime unchanged. This repaired harness error is documented in
`run-01/runtime-qa/isolation-incident.json`; it is not a World save bug.
