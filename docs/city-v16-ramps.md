# City-v16 Ramps V1

Explicit **rule 6 / unchanged MapPermissions policy 3 / schema 22**, starting from
clean `bd0a8e856b21f4229acea011d3805c1d95a67206`. New City-v16 defaults remain rule4;
Bridge saves remain rule5/schema21, without migration. Rule6 inherits every existing
economy, upkeep, population, service, safety and Bridge parameter.

## Owned land edge

`RampId` is independent of building/courier/Bridge/original IDs. A canonical
`RampLink` stores increasing orthogonal adjacent endpoints, their exact signed
height facts, paid cost and placement tick. There are at most64 links, with no
shared endpoint claims. Price **40**, upkeep0, workers/goods/production0. Both ends
must already be normal owned or immutable source ground roads; height difference
is exactly1, calculated after promotion to int. Negative heights and valid signed
extremes are supported. Diagonal/nonadjacent/equal/multilevel edges, water/decks,
off-map, protected originals/markers/gates and unsupported terrain refuse.

The normal producer only admits exact raw0x80/0xc0, objects0, inside the unchanged
Diamond and outside complete original occupancy. These immutable policy3 facts
exclude cliffs/mountains/unknown mixtures; the ramp does not identify an original
slope. The explicit authored contract joins two independently permitted land
roads, never two arbitrary height samples.

Only World overrides `UnsupportedHeightTransition` on that exact owned edge.
Policy3 continues reporting its original height blocker. Outside/invalid/structure/
gate/Bridge-side restrictions retain precedence. Other height edges stay blocked.
Normal endpoint roads retain ordinary land attachments; the ramp has no inner
cell permitting a T/crossing/curved branch. Derived bounded endpoint indexes are
rebuilt at restore and never saved. Existing BFS, all courier roles, topology,
dispatch/return/reroute and explicit G diagnosis use this same World graph.

Failed construction leaves the full Snapshot untouched. Success buys once and
increments command sequence, spending, road revision and route refresh once.
**Ramp demolition is unavailable in V1.** Ordinary endpoint removal retains
occupied/begun-edge safety and the paid link identity; traversal stops without
both roads. Legal repair restores traversal with the same RampId and real cargo/
reservations. A removed endpoint cannot be replaced by a building. The geometric
permission for an old path prefix survives disconnection, while live traversal
still requires both road cells, as in the existing Bridge waiting contract.

Bridge V1 retains its complete separate canonical straight1..8-water/equal-bank
contract, price40+20/cell, deck heights and IDs. Rule6 explicitly includes Bridge
support. A ramp can join a normal Bridge bank without changing its height or
admitting deck/side entry. No original structure, terrain or height is repaired.

## Presentation and controls

Select **Enable Ramps + Bridges (rule6)** in New Sandbox, or explicitly use CLI
`--sandbox-rules sandbox-city-v16 --sandbox-ramps`. The choice is session-only.
**P Ramp** appears only in rule6; B Bridge/Road and every older control remain.
Drag between neighboring existing roads and release. Green means affordable,
amber geometry-valid without funds, red a named blocker. Status reports both
heights, delta and $40. Real MouseUp revalidates. Escape/right-click/focus/resize/
UI release cancel; previews coalesce endpoints/revision/sequence/funds and never
execute/copy/restore/tick World or run navigation/file/asset work. P respects the
existing F3 diagnostic boundary.

The surface is **OpenEmperor-authored**, with weathered cross-planks, tapering
stone infill, narrow side beams and quiet rails; no original pixels are copied.
Fixed axis/gradient/half/back/front templates, including flat Snapshot comparison,
prepare their material detail and bounded coverage raster before the frame loop.
Drawing/picking reuse those components without textures. Actual endpoint projection
uses signed height×40. Each half deck precedes its local actors; its near rail
follows them within the existing painter, with no global ordering change. Original
foreground keeps its effective order. F8 hides the whole body in nonstructural
comparisons; cached drawn geometric hits and endpoint Inspector retain provenance.

Authoritative courier positions already interpolate saved path vertices and
edge progress/duration. The pure ramp surface projection interpolates the matching
saved endpoint heights, including ascent/descent and negative levels. No additional
floating height is saved. Marker drawing and picking share that foot position.
Bridge graphics are unchanged. The independent terrace starter now explicitly
enables **`--authored-terrain-sides`**: an OpenEmperor soil/stone face closes the
40px gap between neighboring single-level land cells, including before a Ramp is
built. This is a session-only, default-OFF presentation option for explicit rule6,
not original cliff artwork or a new transport permission.

Preparation reads complete source/policy3 geometry once. Both cells must have
exact raw0x80/0xc0, objects0, matching signed source/policy heights, permitted normal
land and no original protection, gate or water; the signed difference must be1.
The bounded cache holds at most512 faces and16 eagerly prepared direction/material
templates, each with at most4,096 color runs. Faces draw before the existing Ground
pass; unchanged terrain, roads, foreground, Ramp and actor composition cover them.
Flat Snapshot hides the faces. Picking, all gameplay/save authority, Bridge art and
the global painter remain unchanged. Frames perform no material generation, map
scan, file/asset/texture work, World copies, commands, ticks or navigation.
General original elevation, slope identity and cliff composition remain unresolved.

## Evidence and persistence

The complete pinned167-map audit prepares165, refuses the same two conflicts and
finds **zero eligible nonzero-height land pairs**. Its618 raw-road height edges all
encounter existing terrain restrictions. A real original-map ramp gameplay PASS
is therefore **unavailable**, not substituted by permission relaxation. See
[raw census and pre-implementation version decision](reverse/original-height-transitions.md).

The independently authored test map binds its actual source SHA and policy3 through
normal production loading. All infrastructure, roads and links are normally paid.
Ramp endpoints are **(115,114) height1 → (116,114) height0**; normal roads lead to
Bridge banks **(118,114) → (120,114)**, water(119,114), deck0. Clay starts on the low
side, crosses Bridge then climbs Ramp to Pottery and descends on return. A no-ramp
control has the actual height blocker and no complete alternative route.

The12,000-tick main run uses real cargo, reservations, recipes, staffing, operations,
House demand, Service, Food/Pottery, Fire/Health, taxes and upkeep. It ends with
**30 residents, Funds1082, taxes2950, upkeep1528**; first recipe305. Actual outgoing/
returning ramp occupancy is recorded separately from trip counts: Clay635/630,
Pottery610/610, Service510/510, MarketFoodInbound505/500, MarketPotteryDistributor490/486,
MarketFoodDistributor505/505, FireInspector310/310, HealthWorker245/240 ticks.
No goods, funds, residents, workers or courier progress is injected. Finite layout
success is not a universal capacity/balance or original-game claim.

Schema22 alone adds exactly seven canonical fields per link and next RampId, while
retaining all existing Bridge authority. Complete source/policy/mask hashes, IDs,
canonical endpoints, signed heights, paid cost/tick, reserved land, accounting and
active edges validate atomically. Old schema19/20/21 documents receive no new keys.
Manipulated identities, heights, claims, costs, rules/schemas, hashes, counters and
truncated documents reject. F5→advance→F9 checks the actual View handler, including
mid-ramp and disconnected states. Separate OS processes compare full immediate and
100-tick SaveDocuments at eleven checkpoints, rather than selected counters alone.

## Reproducible local test

```sh
sh tools/test_city_v16_ramps.sh .local/gog-extracted/app --all
sh tools/test_city_v16_ramps.sh .local/gog-extracted/app --ui
```

Default `--all` builds current Release, verifies the pinned original inventory,
runs the complete elevation and permission census, paid authored gameplay, malformed
saves, process restart and old city/Bridge controls. `--audit` omits gameplay;
`--gameplay` omits the repeated old long-play controls. `--ui` performs diagnosis
and opens the normal Release app with authored terrain sides, paused **before ramp construction**. Press P or
click P Ramp, drag the visible neighboring roads at the step, release the green
$40 preview, then Continue. Watch Clay climb, deposit and return; F5/F9 preserves
the city. The existing bridge further down the road remains independently built.
Escape exits the direct CLI view; L belongs to the main-menu save list.

Private reports, playable saves and independently authored map are under ignored
`.local/city-v16-ramps/run-*`. Private DATA/Model/Res are read-only hard links to
the user's originals; only the new map and outside-data saves are written. No
personal settings/recovery or original data are modified. This authored map is
not a repaired/imported original map. Original slope identity/import, arbitrary
cliffs, multilevel/curved/intersecting ramps, water ramps and safe ramp demolition
remain unsupported. Native/human appearance and pointer acceptance are a manual
check, distinct from software pixels and scripted SDL success.

## Full validation

All153 registered tests pass without skips in Debug (673.65s), Release (90.20s)
and ASan+UBSan RelWithDebInfo (342.44s), with no sanitizer findings or compiler
warnings. The two former unknown-version assertions now explicitly admit rule6
and reject rule7; historical supported-version/default/height negatives remain.
The complete old control retains50 literal byte-identical SaveDocuments:14 historical
Xia rule3,8 current Xia rule4,8 Handan rule4 and20 rule5 Bridge checkpoints/continuations.
All1,485 policy1/2/3 actual/zero/one fingerprint/canonical/mask summaries match the
pre-production baseline; complete census stays167/165/2. Chang-an Sui12k paid Bridge
transport, ten old process restarts and current Xia/Handan goal7200/6800 plus10k
stability are repeated. Eleven new process pairs retain the full schema22 document
immediately and after100 ticks. Twenty-three malformed full documents plus a
truncated save reject. Actual View F5/advance/F9 covers nine construction/movement/
wait/repair states. Their100 pure frames and the100 normal P-drag preview frames
keep every prohibited counter zero; texture count returns to0 after shutdown.

Evidence roots and final source/original inventories are printed by the starter.
Registered tests, external paid authored gameplay, original-source audit, software
capture/launch and native/human acceptance remain separate. No commit or publication.

Final complete starter: `.local/city-v16-ramps/run-E3TeNf`; source SHA-256
`254b92d86190568cec679447de2ff59b504e6fb70f9b24b5cda469d2dbe4258a`. All original1464files/819711091bytes retain manifest
`dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`.
`--audit`, `--gameplay`, `--all` and `--ui` were executed. The normal Release
`--ui` load was additionally smoke-tested under dummy/software and its owned
process exited0; this does not infer Cocoa/Metal or human input acceptance.
The private normal-player prebuild save is in
`.local/city-v16-ramps/run-5p6rom/player-app/saves/01-build-ramp.json`.

### Authored terrain-side graphics follow-up

The new pixel check covers all four edge directions, signed endpoint heights,
1x/2x/4x/fractional zoom, material detail, gap coverage and pure cached frames.
Scripted production View checks also exercise zoom/pan, actual window resize,
clipping, F8, normal paid P drag and exact F5/F9 state. These checks pass in
Release, Debug and ASan+UBSan with no sanitizer findings. Before/after-construction
and mid-Ramp software captures are inspected separately from native/human acceptance.
The complete Release suite passes154/154; the four relevant pixel/View/input checks
also pass in Debug and ASan+UBSan. The current `--gameplay` starter
`.local/city-v16-ramps/run-7f7RQM` passes12,000 paid ticks and11 full-document
process restart pairs. Its23 authoritative Ramp saves remain byte-identical to
`run-E3TeNf`; all1,485 original-policy summaries and the pinned original inventory
remain unchanged. The normal Release app with the explicit side option also loads
the prebuild save under dummy/software; this does not establish native/human acceptance.
