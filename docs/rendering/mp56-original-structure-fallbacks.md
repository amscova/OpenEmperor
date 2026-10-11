# MP56: missing historical General singleton presentation (2026-10-11)

The pass starts at clean `21959435cc47d9a2d2f17998efb0ecbf922305db`.
It fixes **saved-image resolution**, not an original ResWall/ResGate restore
producer. The normal MP56 Empty City now draws the nine previously brown
singletons in the investigated city strip. City-v16 rule4, permissions policy2,
schema20, original occupancy and the complete historical legacy masks stay
unchanged. Local original pixels, source dumps, binaries and QA stay ignored
under `.local/mp56-visuals/`.

## Verified source and actual failing draw

**RAW MAP FACT:** `Cities/MP56.map` is 102,064 bytes, SHA-256
`96e3bd832b40ae57a6e7526eb864cfa85a5bacd4dc3cad95aa7e4fff8dc17892`.
The freshly inventoried original root remains 1,464 files / 819,711,091 bytes,
manifest SHA-256
`dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`.
Manifest encoding is sorted relative UTF-8 path, TAB size, TAB file SHA-256, LF.
The original EXE retains the pinned
`6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e` hash;
this resolution fix requires no new binary-derived selector.

The user supplied neighboring Sandbox origins, not fallback coordinates.
No user SaveDocument was found in the existing isolated MP56 launcher roots.
Acceptance uses a fresh Empty City and a separately identified, regularly paid
QA city; it is not a reproduction of the user's exact save or negative treasury.
The fallback already exists before construction, mode changes or F5/F9.
The default Sandbox mode is Regenerated with diagnostics off.

The complete manager, validated within the existing 4,000-record bound,
provides these source witnesses:

| Storage cell(s) | Source / manager index and original ID | Type / state | Saved selection |
| --- | --- | --- | --- |
| `(112,96)`, `(110,96)`, `(113,96)`, `(116,96)`, `(120,96)`, `(119,96)`, `(118,96)`, `(123,96)` | `cResWall`, respectively 13–20 | type90, status3, side1, subindex0, Base4 / parent1 / NonHouseInfo1 / own1 | `0x80cb`: slot2, local203 |
| `(111,96)` | `cResGate`, manager21 / ID21 | type105, status3, side1, subindex0, Base4 / parent1 / NonHouseInfo1 / own1 | `0x80d8`: slot2, local216 |

The eight ResWall origins have Terrain `0x88`; the ResGate origin has `0xc8`.
All nine have Objects4, candidate byte `0x40`, signed height1 and existing
OriginalStructure protection. None is an imported original road or a new
passage. ResWall record logical offsets, in manager order, are 1,097,644;
1,097,971; 1,098,286; 1,098,601; 1,098,916; 1,099,231; 1,099,546; 1,099,861.
ResGate starts at 1,100,176; its local origin `(82,67)` plus border29 gives
storage `(111,96)` and reference21,999. These are logical map offsets, not PE
addresses or SG3 record numbers. Full source/provenance dumps are in the local
fresh corpus report.

**OPENEMPEROR OBSERVATION:** each affected draw was an unfootprinted historical
StoredCell. The nine StoredCell indices are respectively 5,693; 5,521; 5,779;
6,037; 6,381; 6,295; 6,209; 6,639; and gate5,607. In the actual baseline
draw order, the gate is renderer item5,723. No regenerated owner suppresses
these nine cells. `draw_component(base=true)` calls `draw_diagnostic` with
the Structure palette `(103,94,78)`, at saved signed height ×40. It never
submits a texture for them. This is neither a debug overlay nor intentional
blank art. A brown color alone was not used to identify the source class.

`make_stored_graphics_plan` resolves historical IDs using Terrain/Elevation
and optional slot8. General slot2 was added **after** this resolution for the
existing regenerated wall/Tower pipeline. Thus these saved IDs remained
`UnregisteredSlot` despite valid General metadata later being available.

## Bounded resolution and activation

Existing exact `China_General` registration now also responds to actual
unresolved slot2 historical candidates. After all existing historical,
regenerated body and attachment resource selection, the late resolver uses
the same `ResourceGroupLookup`, `RuntimeArchiveLayout` and graphics-ID resolver.
It validates the containing group and admits only static, unmirrored Type30
side1, width78, height at least40, base3,200. No Type0/1/256, animated, mirrored,
multi-cell, missing-group, reserved-runtime or invalid-source bypass is added.

| Stored packed ID | Containing key and runtime bounds | Variant within that saved group | Physical asset |
| --- | --- | --- | --- |
| `0x80cb` | `0x428`, locals `[203,220)` | 0 | `DATA/China_General.sg3`, record404, 78×57, base3,200 / Overlay1,499 |
| `0x80d8` | `0x428`, locals `[203,220)` | 13 | same archive, record417, 78×62, base3,200 / Overlay1,818 |

The key is **containing-group provenance**, not a newly inferred type90/105
producer. Physical records are `local + 200 system skip + 1 dummy`, applied
once by the existing runtime layout. Saved IDs are not physical record numbers.
Existing physical assets are reused; only the two new records are appended on
MP56. This also prepares compatible historical singleton fallback for an
already supported generated wall; an active generated owner still suppresses
its historical image. The resolver creates no generated claims, original
entity IDs, World objects, exclusive permissions, gate passages or selectors.

Each accepted singleton has one complete image. Its Base/Overlay preparation
uses the existing atomic decoder/upload path and alpha caches. Any failed
part removes all textures for that image and restores the whole diagnostic
diamond, without retaining an invisible hit. Missing/unsafe General metadata,
bitmap, unsupported geometry and group bounds have a named load-time reason.
New optional asset-count/RGBA/payload budget failures retain that fallback;
previous required-source preflight and required budget failures retain their
old contracts. Existing protected assets precede optional images in preparation.

New footprints explicitly carry `presentation_only`; the old
`make_sandbox_buildable_mask` excludes them. This protects even a newly resolved
General singleton with raw Terrain `0x80` / Objects0. Full old masks and saves
are compared, rather than assuming Structure terrain alone protects this
renderer/gameplay coupling. No source asset previously refused by another
consumer is revived by the late resolver.

StoredCell, Footprint, Regenerated and Tower attachment index domains remain
distinct. The historical cell gets an explicit new Footprint reference.
The existing attachment/road replacement guard is retained. Complete generated
readiness and historical suppression are recomputed through their ordinary
load-time paths; numeric index equality creates no identity.

## Placement, composition and inspection

**OPENEMPEROR PRESENTATION CONVENTION:** reuse the historical singleton anchor
`(world.x−width/2, world.y−(height−40))`. Regenerated mode subtracts the cell's
signed height ×40 once; Snapshot remains flat. At `(110,96)`, record404's
elevated image origin is `(521,2903)`; the gate417 at `(111,96)` uses
`(561,2918)`. Actual chosen image, dimensions, anchor, cached alpha and culling
remain paired. No coordinate/ID exception, global offset, new foreground rule
or alternative painter comparison is introduced.

Regenerated rendering retains the existing historical Base-ground /
Overlay-spatial convention, with the ordinary logical cell painter key. Full
Snapshot uses the saved Combined image once and the existing flat anchor.
F8 Ground omits the structural Overlay and its hit; Full modes restore it.
Base is retained as the historical ground-layer convention. This fix does not
establish original per-pixel composition or justify hiding every Type30 Base
as if its scene role had been independently proved. Original road ground,
walkers, Tower bodies/joins and GateHouse components keep their existing passes.

F1 adds a short saved-General component/selection/fallback note; read-only
provenance includes containing-group bounds, actual physical asset, decode
error and whether a generated owner supersedes it. A decoded but superseded
historical image is not counted as an active replacement. F8, alpha hit testing,
roads and ground picking use the existing shared production path. Selecting an
original visual gives a map cell, never a Sandbox BuildingId or demolition.

**UNRESOLVED:** restored ResWall/ResGate resource choice, lifecycle, rotated
views and original caller/compositor geometry. These are now visible **saved
historical images**, not a verified reproduction of original post-load restore.
Other unsupported formats, components and interpenetrating-image composition
remain separately bounded. The known post-load saved-ID clear is unchanged.

## Visible and neutral acceptance

The authored production-load regression first fails on the actual brown
singleton pixel. The corrected fixture uses a distinguishable green diamond,
red Omega body pixel and independent literal coordinates over blue terrain.
The final authored fixture was also compiled against the frozen baseline
libraries, renderer objects and headers archived from the original HEAD: it
independently fails on the literal red body pixel (exit1), with the baseline
source/ABI kept intact. The first RED ground-pixel failure remains separately
recorded; the later blue-terrain fixture avoids a coincidental red background.
It covers saved slot2 resolution, dummy/skip, physical deduplication,
height once, transparent alpha, F8 and flat Snapshot, pan/resize and
1/1.5/2/4 zoom. Negative cases cover missing bitmap, Type1, mirror, missing
group, reserved runtime record, invalid payload range and damaged Omega.
Each retains complete fallback and unrelated required Terrain. A separate
ignored authored production check exhausts the exact 64 MiB RGBA budget:
required uploads stay valid, the optional image retains its named complete
fallback and shutdown owns zero textures. No original pixels are fixtures.
Another ignored authored check submits the actual selected historical draw
item through the production spatial merge, with actor and neighboring building
rectangles both before and after its ground key. Body pixels follow that order,
Base stays in the ground pass, and all four out-of-viewport cases cull the whole
image. These are synthetic scene witnesses; existing actual Tower/Gate/Road
regressions remain separate.

Unmodified normal Application/main is separately driven through New Sandbox →
City-v16 → MP56 → Empty City in isolated roots. At the same tick5, map,
Regenerated mode, camera and zoom, baseline shows the brown strips and the
candidate shows the stored masonry and small wooden gate. Two camera views
are captured outside production rendering. F5/F9/F5 compares entire typed
documents. This is **scripted SDL App evidence**, not native or human input PASS.
No private asset manifest or debug activation is used by the app.
The map-only pixel comparisons change 8,218 pixels in the first camera and
16,261 in the complete-band camera. Both have **zero changed pixels outside
the union of these nine image/old-diagnostic bounds**. HUD timing text is outside
that map comparison. The captures show all nine replacements in the second
normal-App camera without unrelated map changes.

Independent **Xiangjun** (93,522 bytes, SHA-256
`ed1389f93a238fb39a2e4e835cc6ab1e67c0830de90b4984a04a4639748f746b`)
uses the same path with different saved choices and height0.
Its ResGate at `(137,124)` resolves saved `0x80d0`, local208 / group
`0x428` variant5 → physical409. The measured direct-render view changes from
36 fallback draws to one. The remaining `(140,124)` has local201 in another
group: key`0x429`, locals`[186,203)`, variant15 → physical402, Type256,
68×79, size flag0 / base field2,716. This is not supported side-one Type30
geometry. It is retained, not overpainted or called a missing file.
Normal Empty City / F5/F9 is separately exercised there. This establishes a
countercase, not corpus-wide rendering fidelity.

Fresh before/after corpus checks retain **167 complete managers, 121 prepared
policies, 46 refusals**: 41 unknown-occupancy first failures, three known
conflicts and two other original-rule failures. Source states, masks, blockers,
heights, passages, topology and error strings remain unchanged. Byte comparison
covers 3,993 policy1 files, 1,452 policy2 files and 6,847 legacy contract files.
All 167 maps ×20 old rule variants retain **3,340 complete old-save roundtrips**
per build. Changed presentation metadata is separately counted, never confused
with permission equality.

The small paid MP56 control repeats normal commands through tick605, including
rejected protected placements, free original-road no-op, paid houses/roads/
Clay/Pottery and real courier movement. All three full saves at ticks5/45/605
are byte-identical before/after and to the prior completed occupancy control.
F5/F9 and a separate process restart from in-flight tick45 to605 produce the
same entire document, including routes, stocks, reservations, workforce,
population, funds, maintenance and all fire/health state. No treasury repair or
user-save mutation occurs. Existing normal-menu failure-retention regressions
remain part of the full matrix.

At the frozen direct-render MP56 camera, assets515→517, uploads1,529→1,535,
RGBA28,585,876→28,697,260 bytes, texture draws1,613→1,631 and diagnostics9→0.
The eighteen new Base/Overlay submissions replace nine diagnostic diamonds.
Normal App total uploads1,783→1,789, reflecting its other unchanged sprite sets.
Preparation is eager; all twelve production render counters remain zero during
the normal app checks. Render does no files, decode/upload/readback, World
copies/commands, BFS, routes or map scans; observer capture/readback is outside
the measured production frame. Every instrumented scripted session ends with
zero live textures; passive native launch is a separate, narrower check.
The complete-band App run observes 108 production frames per build, with median
render time1.282ms before /1.217ms after, excluding observer work. These are local
timing observations under concurrent QA load, not a general speedup claim.

The final registered suite is run serially across configurations:

| Configuration | Executed / passed | CTest time |
| --- | --- | --- |
| Debug | 145 / 145 | 1,699.56 s |
| Release | 145 / 145 | 61.01 s |
| ASan + UBSan | 145 / 145 | 221.61 s |

All **435 invocations pass**, with no skipped tests, compiler warnings or
production/test source changes. The same 329 source paths are hash-identical
across the three completed builds. StoredGraphics/assets, landscape/typed
original presentation, scene composition/picking, roads/ground occlusion,
Tower/Gate/Wall joins, permissions/legacy masks, budgets, save/recovery and
determinism remain covered by the full registered suite. Local per-configuration
reports and JUnit output are retained.

The excluded Xia fixture is built and replayed separately, also serially:
Debug 211.774 s, Release 4.720 s, ASan+UBSan 61.550 s. All three pass without
warnings or sanitizer findings. Each compares all 14 complete SaveDocuments
and the entire `main-facts.jsonl` byte-for-byte with the prior completed
Xia baseline, including paid construction, active transport, food-road and fire
branches. The existing control stays City-v16 rule3 / policy1 / schema19;
it is not migrated to the MP56 rule4 session. Its goal is reached at tick6800
(population126 / funds2,282), retained through tick9200 (population148 /
funds5,066), and all **42 compared saves** remain identical. Results are retained
in `xia-controls-report.json` and the three fresh Xia roots.

Development first failures are
retained: the intentional RED pixel test, two fixture integration failures
(early SDL initialization and an extra unremoved catalog map), and ignored
helper compile/path issues. These are distinct from final production results.
The initial optional Python image comparison lacked Pillow; the completed
comparison uses SDL's existing BMP surface reader instead. No dependency or
second image decoder was added to production.

## Test start and remaining scope

```sh
sh tools/test_mp56_occupancy.sh .local/gog-extracted/app
```

The existing starter builds the current Release candidate, verifies the exact
MP56 hash/contained path/full policy, and opens the normal menu using a new
isolated settings/save/recovery root. This exact command was executed after
all matrices and Xia controls passed. Its current Release build and complete
headless MP56 check passed; the normal native app launched in the fresh root
`.local/resgate/player-Nj94H8/` (owned PID97895) and was left open for the user.
This is **native launch evidence only**, without automated native inputs or
a human visual/picking/zoom PASS. The visible before/after comparison above
remains separately labeled scripted SDL/software evidence.

Choose **New Sandbox**, then **Start Sandbox** with the preselected MP56 Empty City,
then use normal wheel/arrows to inspect storage row96 near the user's Clay
origin `(112,101)`. F1 can identify `(111,96)` for the small wooden gate and
the eight listed neighboring masonry cells. The former continuous brown
segments should now show saved original art immediately, without constructing
a city or advancing tax cycles. Existing protected river points remain protected.
Other unsupported original formats and restored ResWall/ResGate semantics stay
open. No commit, push, tag, release or publication is performed.
