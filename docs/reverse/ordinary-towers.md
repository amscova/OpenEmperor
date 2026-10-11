# Ordinary city-wall Tower bodies — bounded presentation pass

This pass implements one static original `cTower` body through the existing
original-object, landscape and renderer pipeline. The complete roof, central
tower and stone plinth are one Type-30 image. It adds no playable Tower,
military simulation, NPC, economy, navigation, policy or save version. Original
Tower footprints retain their existing protection.

The body is distinct from the original renderer's small external wall-join
pieces. Their bounded static camera-zero connection assembly is now implemented;
see [Ordinary Tower wall joins](ordinary-tower-wall-joins.md). Other Tower states
and complete original scene parity remain unresolved. A complete visible body does not establish complete original
wall/environment composition.

## Baseline and actual original objects

The checked starting HEAD was `fa0caa5d05ff51e83c11adb5328a767d16e35503`, with
a clean worktree. The fresh production parser reads the complete 4,000-record,
schema-1 managers before any Tower presentation is admitted.

| Map | Exact map SHA-256 | Active Tower records | Measured state |
| --- | --- | --- | --- |
| `Cities/Kaifeng.map` | `c25855e4db083128d9344b9177dbbc12c2f0fb3f28c25b63d01c93349912e7b3` | IDs 3, 4, 5, 8, 10 | Base 3; height 0 |
| `Cities/Zhengzhou.map` | `d7a212431f25134f7ffc22c888ebbba4a7d82053263ce8338a3990c891427c4c` | IDs 2, 3, 4, 6, 7, 8, 9, 10 | Base 4; height 1 |

Every listed record is class `cTower`, type 131, status 3, saved footprint side 2
and subindex 0, with parent wrapper 1, NonHouse state 1 and Tower wrapper 0.
Each complete 2×2 footprint has exact raw terrain `0x8088`, Objects 0 or 4,
and the same signed height on its four cells. The meaning of Objects 4 is not
asserted. These are **RAW MAP FACTS**, not inferred from images.

The first Kaifeng source is original ID 3 / manager index 3: serialized local
`(57,91)`, Storage `(86,120)`, serialized cell reference 27,446. Declared size 170
gives border 29. The independent Zhengzhou source is ID 4 / manager index 4:
local `(35,24)`, Storage `(93,82)`, reference 18,789; size 112 gives border 58.
The historical slot-2 graphics on these cells were unresolved at baseline.
Their values remain comparison evidence and never select the new body.

Parser recognition, supported source, prepared body, renderer activation and
visible body in a particular camera view are separate counts. The 5/8 parser
counts alone establish none of the later stages. Fresh raw reports and baseline
captures are ignored under `.local/ordinary-towers/`.

The report's `prepared_complete_tower_objects` counts complete metadata/claim
specifications with a resolved asset, before eager decode and atomic activation.
It is not a draw-readiness count: a later decode/readiness failure may retain
that preparation count while `active_complete_tower_objects` is zero and the
source's decode/activation status names its fallback. Only active bodies with
actual in-view draw observations establish the visible acceptance below.
Tower totals live under `ordinary_tower_rendering`. Existing wall/gate counters
remain separate, including the Gate pass's historical
`ordinary_gate_rendering.tower_bodies_reproduced=0`; that field does not count
this Tower producer. Selected wall cells, complete GateHouse bodies and Tower
bodies are three different quantities.

## Bounded original evidence

The EXE was read only, after checking SHA-256
`6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e`.
The bounded body trace checks 718 instruction byte lines in 14 windows against
the actual binary, with zero mismatches. The EXE was not executed. No original
instruction bytes, disassembly or decoded pixels are repository fixtures.

Addresses here are **VAs**, with PE image base `0x400000`; subtract the base for
an RVA. The ignored evidence separately records actual EXE file offsets. Map
offsets are decompressed logical offsets; object fields such as `+0x14` and
VTable offsets are runtime offsets, not map-file offsets.

The `cTower` descriptor at `0x871fe8` identifies constructor `0x5da340`, which
reaches `0x5da3d0`, common NonHouse constructor `0x51c9a0`, and VTable `0x7be544`.
Serializer `0x5da880` consumes the common saved NonHouse parent followed by its
own wrapper 0 with no extra payload. The common base reader supports schemas
3/4/5; these do not change for this pass.

Standalone restore `0x52f030` admits type 131 through `0x52f1d0`, reconstructs the
typed object through `0x42d540`, and passes saved coordinates and the type-table
side through `0x4b11f0`. The latter invokes VTable `+0x100`, which is Tower
placement `0x5da420`. Factory `0x51c91b` uses exact type-131 test `0x42b720` and
the Tower constructor. Only type 130 takes the separate saved GateHouse-layout
copy; that contract is not applied to Tower.

The type-131 information row at `0x8241e0` contains
`[2,1,0x44c,0,9,101]`. Tower placement selects the **base, variant 0** of key
`0x44c`, then uses rectangle writer `0x4b72b0` for one complete 2×2 body and marker
`0x8008`. No saved graphic ID, orientation, layout, subindex, mission or current
player goal selects the static body. The original restore's active check is
nonzero; the implementation conservatively admits only the measured status-3,
subindex-0 case.

Resolution remains `ResourceGroupLookup → RuntimeArchiveLayout → AssetId`, using
the already evidenced General registration at `0x475c4b`. On this exact archive,
key `0x44c` means slot 2 / group position 75 / SG3 index position 76, runtime
interval `[678,679)`, physical record 879. It is one static, unmirrored Type-30
side-2 image, 158×207, with a 12,800-byte base. Variant 1 is out of bounds. The
physical record number belongs to this evidence table, not production selection.

Camera-zero rectangle placement marks `(origin.x,origin.y+1)`, as observed at
`0x4b72c8` and `0x4b7434`. The signed saved-height grid begins at logical 989,275;
getter `0x471e20 → 0x471e00 → 0x408cf0` sign-extends its byte, multiplies by five
and shifts by three. Existing drawing subtracts **signed height ×40** once.
Original post-load height normalization remains outside this pass.

## Body preparation, composition and evidence boundary

`OrdinaryTowerMapPresentation` derives source identity and complete body claims
from the validated manager and read-only raw grids. Its bounded admission is
status 3 / type 131 / side 2 / subindex 0, base schema 3/4/5, parent/state 1,
Tower wrapper 0, exact coordinates/reference and complete nonoverlapping raw
`0x8088` cells with Objects 0 or 4. Unknown combinations retain a named fallback.
Presentation claims do not add or change simulation occupancy.

The resource is prepared through the existing load-time physical asset pool.
Several Towers share its image and textures. Complete group bounds, geometry,
claims, eager decode/upload and composition readiness are required before the
historical replacement is suppressed. A failed body retains complete historical
fallback and functioning foreign instances. Optional Tower resources do not
reject a valid saved city. The historical Snapshot view remains a comparison.

The decoded body contains 20,735 nontransparent pixels: Base 6,400, Overlay
14,335. Base's visible region extends through image Y 127–206 and contains stone
plinth/walls. It is not merely flat terrain. This supports a bounded structural
body `SpatialCombined` policy: the existing Combined image draws once at its
existing spatial front key, with zero early Base and zero separate Overlay.
Other landscape, gates, Great Wall context and original-road ground remain
unchanged. No Tower-last or always-on-top ordering is introduced.

The marker-cell height source is **EXE-OBSERVED**. The equivalent front-cell
anchor `(origin.x+1,origin.y+1)`, X anchor 79, Y anchor `image_height−20`, and front-cell depth
remain **OPENEMPEROR PRESENTATION CONVENTIONS**. Their shared full-image bounds,
cached alpha and effective painter order govern draw, culling and read-only
picking. Transparent image space does not block later hits. Construction
ground-picking retains its separate authority. F8 Wall/Monument visibility hides
the whole body. No whole-image sorter establishes general interpenetrating-image
parity.

The one unique body uses three existing 158×207 component canvases, each 130,824
RGBA bytes: 392,472 logical RGBA bytes total. The existing texture/access policy
and all budgets remain fixed. Preparation occurs at load; render, inspect and
pick add no file access, manager parse, decode/upload/readback, World copy,
command, BFS, route refresh or full map scan.

A separate load-time source-origin diagnostic map lets F1 describe a rejected
Tower state at a valid unambiguous origin. It does not create a render claim,
simulation occupancy or per-frame source scan.

## Separate unresolved scope

Tower callbacks `0x5da8c0` / `0x5da910` submit small external ordinary-wall joins
through keys `0x4c9` (variants 0–5) and `0x4ca` (0–1). They test neighboring
ordinary-wall terrain and exact typed Tower/GateHouse ownership. Their graphics
are small outer wall pieces: five Type-0 and one Type-256 image in the first
group, two Type-0 images in the second. They are not missing roof or plinth parts
of the one Type-30 body. The later [bounded wall-join pass](ordinary-tower-wall-joins.md)
traces their complete static caller offsets/order and adds them without new
claims or decoder relaxation. Complete environment composition remains open.

Other Tower states, views/lifecycle, `cResWall`, further Type-256 wall components,
wall joins beyond the [bounded static connection pass](ordinary-tower-wall-joins.md),
original height normalization, complete caller anchoring
and original Emperor scene parity remain open. Better pixels do not establish
military or gameplay behavior.

## Validation and local review

The two new versioned tests use authored data and RGB555 pixels. They check
supported and rejected source states, schemas, identity/reference/bounds,
foreign reservations, independence from saved graphic IDs, exact group/variant
bounds and static geometry, physical deduplication, complete readiness and
historical fallback. Pixel cases independently check the full roof and structural
Base, one spatial draw and zero early Base draws, transparent holes, signed
height −128/−2/+1/+127 applied once, zoom 1/1.5/2/4, all four culling edges,
viewport changes, F8 and Snapshot. Budget tests include a later-selected Rock
at exact capacity and one byte short: the optional Tower cannot displace an
existing landscape asset. Cached frames and shutdown have zero prohibited
work and zero retained textures.

The authored Tower merge test uses independently specified front/rear
descriptors, rather than deriving its expected order from the production
sorter. That test alone is not evidence of a real House or walking Courier.
The existing scene-composition tests separately exercise the shared production
path with actual Sandbox buildings, a ticked Clay Courier, raised cliff imagery
and alpha picking. Original interpenetrating artwork remains subject to the
whole-image composition limit above.

An additional ignored **DIRECT PRODUCTION / AUTHORED PIXEL** runner passes
24 concrete countercases with the supported Tower producer, General resolver
and `StoredGraphicsRenderer`. Twelve cases also use actual `SandboxView` with
normally paid City-v10 Houses and a normally paid Clay/Pottery/House/road
network. Actual simulation ticks move the Clay Courier, carrying cargo 1, to
rear `(113,113)` at tick 119 and front `(118,118)` at tick 219. At zoom 1/2/4,
the rear House/Courier loses to the Tower Base; the front House/Courier wins,
including actual App-view alpha selection. Twelve further direct renderer cases
check a typed GateHouse's complete 15-component body and raised slot-16 landscape
in front of and behind the Tower at the same three zooms. The complete GateHouse
and Tower claims remain disjoint; pixels and cached-alpha owner follow the
effective order. Independently chosen opaque probes and depth values determine
the pixel expectations. Rendering, zoom and picking leave the full WorldSnapshot
unchanged. These authored fixtures use a 158×238 Tower image and marker height 2;
they do not substitute their image for the real 158×207 body. The report is ignored at
`.local/ordinary-towers/composition/composition-report.json`; it uses authored
Tower/source/pixels and is not a real-original-map, normal Application or human
walker acceptance claim.

### Normal Application, actual draw and visible body

The final production Application was loaded through its ordinary New Sandbox
menu, using City-v16 rule 4 / MapPermissions policy 2 / Save schema 20, Empty
City and automatic Great Wall context. No manual sprite profile, Tower debug
switch, World injection or camera injection was used. The first complete
Kaifeng ID-3 body was inspected before extending the same measured rule to the
other sources and the independent Zhengzhou map.

All of these objects were separately recognized, supported, prepared, active,
actually submitted once and visible in their checked views, with no fallback:

| Map | Original ID / manager index | Serialized local | Storage origin | Signed height | Recognized / supported / prepared / active / visible |
| --- | --- | --- | --- | --- | --- |
| Kaifeng | 3 / 3 | (57,91) | (86,120) | 0 | 1 / 1 / 1 / 1 / 1 |
| Kaifeng | 4 / 4 | (57,112) | (86,141) | 0 | 1 / 1 / 1 / 1 / 1 |
| Kaifeng | 5 / 5 | (82,91) | (111,120) | 0 | 1 / 1 / 1 / 1 / 1 |
| Kaifeng | 8 / 8 | (113,136) | (142,165) | 0 | 1 / 1 / 1 / 1 / 1 |
| Kaifeng | 10 / 10 | (82,112) | (111,141) | 0 | 1 / 1 / 1 / 1 / 1 |
| Zhengzhou | 2 / 2 | (67,59) | (125,117) | 1 | 1 / 1 / 1 / 1 / 1 |
| Zhengzhou | 3 / 3 | (35,59) | (93,117) | 1 | 1 / 1 / 1 / 1 / 1 |
| Zhengzhou | 4 / 4 | (35,24) | (93,82) | 1 | 1 / 1 / 1 / 1 / 1 |
| Zhengzhou | 6 / 6 | (35,47) | (93,105) | 1 | 1 / 1 / 1 / 1 / 1 |
| Zhengzhou | 7 / 7 | (35,40) | (93,98) | 1 | 1 / 1 / 1 / 1 / 1 |
| Zhengzhou | 8 / 8 | (49,59) | (107,117) | 1 | 1 / 1 / 1 / 1 / 1 |
| Zhengzhou | 9 / 9 | (67,19) | (125,77) | 1 | 1 / 1 / 1 / 1 / 1 |
| Zhengzhou | 10 / 10 | (44,19) | (102,77) | 1 | 1 / 1 / 1 / 1 / 1 |

These are per-object observations in centered views, not a claim that every
active Tower is drawn in every frame. Four final runs checked each object at
1/1.15/2/4 zoom:

| Normal Application surface | Map | Tower draw captures | Frames | Time |
| --- | --- | --- | --- | --- |
| SDL dummy / software | Kaifeng | 20 | 1,161 | 24.91 s |
| SDL dummy / software | Zhengzhou | 32 | 1,453 | 31.46 s |
| Cocoa window / Metal | Kaifeng | 20 | 1,184 | 30.43 s |
| Cocoa window / Metal | Zhengzhou | 32 | 1,490 | 32.92 s |

All 104 captures measured one complete-body texture submission. At 1× and
1.15× the checked views show the complete roof, shaft and plinth. Large zoom
captures can crop the body at viewport edges: at 4× the 158×207 image is
632×828 pixels, larger than the software run's map viewport. The observer's
`captured_complete_static_body` flag means one prepared Combined-body asset was
actually submitted; it does not assert that every image bound fits inside the
viewport. The separate authored tests check full elevated bounds and four-edge
culling. Each run also checked
actual App selection of an opaque Tower pixel, its F1 source/height/resource
provenance, all eight F8 modes, and F5/F9/F5 with byte-identical complete saved
JSON. Read-only view operations preserved the full WorldSnapshot and policy.
All twelve production render counters were zero before the private observer's
own capture/report work; live texture count after shutdown was zero.

This is **SCRIPTED NORMAL APPLICATION** evidence: a private ignored observer
delivered SDL events and scripted held-device state. The Metal runs establish
a native window/render-backend result, not actual native-device or human input
acceptance. No human PASS is claimed. The observer stops its automation on native
interaction/focus takeover; only fresh isolated task roots/processes were used.
The final App SHA-256 is
`fd869a370498b7e1ee73d281a8e5688aab58f4d6b4c34a39beb15086ca23cdbd`;
the observer and detailed run provenance are in the ignored
`.local/ordinary-towers/app/final-summary.json`.

A separate **DIRECT PRODUCTION SOFTWARE RENDER** comparison used identical
map/data/camera before and after. Across all 13 views, 12,566,349 pixels outside
the complete Tower bounds stayed byte-identical. Its per-object four-zoom draw
and cached-alpha results are not counted as normal Application runs. The added
resource cost per map is one physical asset, one decode, three eager uploads
and 392,472 logical RGBA bytes, shared by every Tower. There is one Combined
body draw per visible Tower and no per-frame resource preparation.

### Save, policy and optional-resource regression

Kaifeng and Zhengzhou retain exact policy-1/policy-2 canonical state, legacy
buildability masks and complete empty-city schema-19/schema-20 SaveDocuments.
Separate private APFS data clones remove exactly `China_General.sg3` or its
`.555` dependency. All four map/dependency combinations still load: Kaifeng
reports 5 recognized / 5 supported / 0 prepared / 0 active / 5 fallback;
Zhengzhou reports 8 / 8 / 0 / 0 / 8. F1 reports
`required Tower General group unavailable; historical preview retained`.
All eight schema-19/schema-20 save/restore pairs remain byte-identical and
shutdown retains no textures. Existing MenuSession failure tests check that
a failed candidate leaves the previous World and save target intact.

Final Debug, Release and optimized ASan/UBSan paid-gameplay checks reproduce all 14
Xia SaveDocuments and main facts, the existing rule-3 Chang-an Zhou / Handan /
Badaling A–E saves, and rule-4 OriginalRoads/GatePassages reports and checkpoints
on Handan / Badaling / Kaifeng / Zhengzhou. Six fresh-process continuations also
match the frozen baseline. Complete permissions, funds, taxes, population,
stocks, roads, gate passage cells, paths and arrival ticks remain unchanged.
The separate 100,000-tick endurance covers all seven existing profiles; these
helper runs remain distinct from CTest and normal Application evidence.

Each configuration passes 16 helper stages and 66 complete byte-identical file
comparisons: 58 SaveDocuments, including the six restarts, and eight facts
reports. The production/helper freeze covers 223 files and remains identical
through all 36 source checks per configuration. The ignored aggregate is
`.local/ordinary-towers/authority/final-summary.json`.

| Final check | Result | Complete CTest time |
| --- | --- | --- |
| Release full suite | 140/140 PASS, no skips/warnings/source changes | 123.94 s |
| RelWithDebInfo `-O2`, ASan + UBSan full suite | 140/140 PASS, no skips/warnings/source changes/diagnoses | 340.53 s |
| Debug full suite | 140/140 PASS, no skips/warnings/source changes | 1,505.06 s |
| Fresh arm64/macOS-26.0 package Release suite | 140/140 PASS | 96.55 s |

ASan uses `detect_leaks=0:halt_on_error=1`; UBSan uses
`halt_on_error=1:print_stacktrace=1`. Own resource shutdown is independently
measured by zero retained SDL textures in authored tests, direct render and
both normal Application backends.

The unchanged official `tools/package_macos.py` pipeline was actually run with
private BUILD/DIST under `.local/ordinary-towers/package/` and a two-job wrapper.
Its fresh Release build, full CTest, dependency fixup, recursive verification,
ad-hoc signature, relocation/fresh menu, synthetic and Original-Xia
Industry/City-v11 save/resume/process restart, ZIP/unzip and all 17 negative
verification cases passed. The complete pipeline took 501.84 seconds. Its
18 allowed bundle files contain no original assets, settings or saves; existing
user `build-macos-app` and `dist` contents remain byte-identical.

The private wrapper's separate broad source audit exited 1 because this milestone
finalized `KNOWN_ISSUES.md` and this document while packaging ran. That is a
doc-only audit delta, not a failed official packaging check: production, tests,
build, resources and pipeline inputs are unchanged, and neither changed document
is copied to the bundle/ZIP. No restaging is needed. Both outcomes are explicitly
recorded in `.local/ordinary-towers/package/acceptance-report.json`.
The resulting package is a dirty local `0.1.0-alpha.2` / `fa0caa5d05ff` test
candidate, ad-hoc signed; Developer ID/notarization, Finder/Gatekeeper and a
clean independent Mac remain unverified. No commit, release or publication was
performed. Final integrity checks confirm all 47 frozen contract sources and
all 1,464 original files (819,711,091 bytes) unchanged, with no added/removed
original files and no new original pixels or binary fixtures visible to Git.
`git diff --check` passes. The final ignored acceptance aggregate is
`.local/ordinary-towers/final-acceptance.json`.

### Tested local starter

From the repository root, the versioned starter builds the current local
Release code and opens the normal menu with fresh isolated settings/save/
recovery roots:

```sh
sh tools/test_ordinary_towers.sh .local/gog-extracted/app Kaifeng
sh tools/test_ordinary_towers.sh .local/gog-extracted/app Zhengzhou
```

Choose **New Sandbox**, wait for **Map rules checked**, then choose **Start
Sandbox**. The preselected Empty City / City-v16 rule-4 / policy-2 session
shows the original Towers without a purchase. On the checked data revision,
Kaifeng ID 3 is Storage `(86,120)`, serialized local `(57,91)`; Zhengzhou ID 4
is Storage `(93,82)`, local `(35,24)`. Use arrows to reach the wall corner and
the wheel to inspect its roof, shaft and plinth. F1 identifies the visible
source; F8 compares whole-body visibility and Historical Snapshot. Map coordinates
are storage coordinates unless explicitly labeled local.

Both exact commands and the Zhengzhou command with a data path containing spaces
have actually started the Release normal menu and exited successfully through
a private SDL-only ten-frame observer. Five invalid names and two escaping
symlinks were rejected. That starter smoke test is menu-start evidence; the
map-loaded visible acceptance is the separate normal Application table above.
No personal settings, saves or original files were changed. Original assets,
pixel captures, private runners and reports remain ignored and are absent
from repository fixtures and the App bundle.
