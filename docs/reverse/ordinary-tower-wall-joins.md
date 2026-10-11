# Ordinary Tower wall joins (2026-10-11)

This bounded read-only presentation pass adds the evidenced static camera-zero
external components to the existing `cTower` body. The body remains General
`0x44c`, variant 0, one unchanged side-2 Type-30 `SpatialCombined` image. These
are additive attachments, with no new ground claims or gameplay entities.
Other states/views, isolated cap-only decoration, `cResWall`, general Type-256
wall components and complete original scene parity remain open.

## Baseline and first missing case

Actual starting HEAD: `8ca16f950b5a80df28ddbbf917cd989a95e64846`, clean worktree.
The committed Original Road visual fix is retained. All tracked input hashes
were frozen before changes. Original inputs and captures remain ignored.

Kaifeng original ID/manager 3 is a complete active status-3/type-131/side-2
Tower, base schema 3, origin `(86,120)`, serialized local `(57,91)`, signed
height 0. Its prior body is complete but omits six external callback components.
An authored asymmetric production-renderer pixel test first failed on the
required `0x4ca` variant-0 green pixel. After tracing the complete body caller,
the corrected expected position was also run against the frozen baseline
renderer and failed for the same missing component. The integrated test passes.
No proprietary image or instruction bytes are test fixtures.

Map SHA-256:

- Kaifeng: `c25855e4db083128d9344b9177dbbc12c2f0fb3f28c25b63d01c93349912e7b3`.
- Zhengzhou: `d7a212431f25134f7ffc22c888ebbba4a7d82053263ce8338a3990c891427c4c`.

Both complete managers contain 4,000 records, ending at decompressed logical
1,821,368 / 1,827,264 respectively. Typed IDs and field provenance are retained;
raw Objects 0/4 are never interpreted as IDs.

## Verified original callback and caller evidence

The pinned EXE was read only after checking SHA-256
`6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e`.
Fresh bounded disassembly checks 3,542 instruction lines in 12 windows against
the actual file, with zero mismatches. The original EXE was not executed.
Addresses below are VAs; PE image base is `0x400000`, so RVA = VA minus that
base. The ignored audit records corresponding physical EXE file offsets.
Runtime fields and vtable offsets are not map-file offsets.

The normal static draw loop calls body dispatcher `0x4715c0` at `0x4708b2`,
then Tower vtable `+0x108` at `0x4709b9`, then cell actors, and `+0x10c` at
`0x470e67`. Tower vtable `0x7be544` selects `0x5da8c0` / `0x5da910`.
The first callback invokes `0x5daab0` then `0x5dac10`; the second invokes
`0x5da950`. The similarly shaped `+0x10c` call at `0x470491` is guarded by
type `0x53` (decimal 83), not Tower type 131. Common `+0x104` building animation
`0x51e0a0` does not establish the static Tower body.

Both Tower callbacks use runtime origin/reference `this+0x10`. The measured
status-3 body admission stays unchanged; active state alone does not admit
additional body states. Static ordinary callers pass the helper preview flag
false. Placement-preview flag true and other camera orientations remain excluded.

Each neighbor is present when terrain `0x4000` is set, or when the runtime
owner table identifies exact type 131 or 130. This is **not** the ordinary
WallTopology union of `0x4000/0x8000`. Original ownership is read from runtime
short grid `0xfc3750`, resolved through `0x47f1b0`, then exact type tests
`0x42b720` / `0x42b730` at object field `+0x14`. Serialized Objects at
`0xf37da0` are a different grid. OpenEmperor reconstructs only these needed
owner facts from complete, validated typed Tower/GateHouse claims. Ambiguous,
incomplete or unsupported typed ownership fails closed. A gate bit alone does
not prove a GateHouse owner or connection.

Camera-zero neighbor offsets are decoded from `0x7be4e0` / `0x7be500`, using
the observed storage stride 228:

| Input | Offset from Tower origin | Required condition |
|---|---|---|
| n0 | (+1,-1) | `0x4c9` v0 if absent |
| n1 | (-1,+1) | `0x4c9` v1 if absent |
| n2 | (0,+2) | `0x4c9` v2 if absent |
| n3 | (+2,0) | `0x4c9` v3 if absent |
| n4 | (+2,+1) | `0x4c9` v4 if absent |
| n5 | (+1,+2) | `0x4c9` v5 if absent |

`0x5dac10` additionally selects `0x4ca` v0 if n2 **or** n5 is present,
and v1 if n3 **or** n4 is present. These are ordered pairs, not a fixed two-piece
rule. For this milestone at least one relevant neighbor is required before
adding the callback assembly. The original helpers also draw isolated
absence-branch caps; that cap-only decoration is deliberately outside this pass
and receives `NO_JOIN_REQUIRED`, rather than invented connections.

| Resource / variant | Local caller offset | Callback phase | Actual format / dimensions |
|---|---|---|---|
| `0x4c9` / 0 | (127,-60) | after body, `+0x108` | Type 0, 30×35 |
| `0x4c9` / 1 | (1,-57) | after body, `+0x108` | Type 0, 30×35 |
| `0x4ca` / 0 | (28,-71) | after body, `+0x108` | Type 0, 53×71 |
| `0x4ca` / 1 | (79,-70) | after body, `+0x108` | Type 0, 53×70 |
| `0x4c9` / 2 | (1,-46) | after cell actors, `+0x10c` | Type 0, 43×36 |
| `0x4c9` / 3 | (114,-48) | after cell actors, `+0x10c` | Type 0, 44×40 |
| `0x4c9` / 4 | (75,-28) | after cell actors, `+0x10c` | Type 0, 44×40 |
| `0x4c9` / 5 | (39,-24) | after cell actors, `+0x10c` | Type 256, 43×36 |

Offset tables are `0x7be4a0`, `0x7be4b0`, `0x7be4d0`. All draws resolve the
Resource-Key at `0x408170`, add only an in-group variant and call
`0x413960 → 0x5cd8a0` with the plain/Omega image's top-left. There is no
additive blend or replacement of a neighboring wall in this ordinary context.

### Complete body-to-callback position relation

Ignoring the side-two rectangle property is incorrect. Placement
`0x5da420 → 0x4b72b0` writes property 1 at `0x4b73ea` and camera-zero marker
origin+(0,1). Normal body dispatcher takes `0x471a91 → 0x46b620`; Tower
vtable `+0x1f0` returns false at `0x413a00`. Property 1 adds `(40,-20)` before
`0x4169d0 → 0x5ce800`; the 158-pixel branch `0x5ce9a9` adds
`(-40,80-image_height)`. Normal projection setup `0x53ca80 → 0x469820` supplies
78×40, whose half operands are 39/20, hence the first X adjustment is 39+1.
The complete body relation is therefore `(0,60-image_height)` from the original
marker caller, not merely the Overlay primitive's relation.

Both callbacks receive the unadjusted marker caller, with its signed height
already subtracted once. Relative to the **unchanged existing full body image**,
each join begins at `(local_x, image_height-60+local_y)`. This carries the same
marker height exactly once, with no map-specific shift or change to the Tower
anchor. The full-image front anchor/depth remain the existing OpenEmperor
presentation convention; this relative caller proof does not establish complete
original anchor or scene-composition parity.

## Resources, readiness and attachments

Selection uses `ResourceGroupLookup → RuntimeArchiveLayout → AssetId`, never
a Resource-Key as a decoder index. On this measured archive:

- General body `0x44c`: runtime `[678,679)`, physical 879.
- General `0x4c9`: runtime `[679,685)`, physical 880–885.
- General `0x4ca`: runtime `[685,687)`, physical 886–887.

These physical IDs are evidence, not production selection constants. All eight
join records are static, unmirrored and decode through the existing strict
Plain/Omega decoders. Type 256 support is restricted to the evidenced v5 role;
no general Type-256 wall producer or decoder relaxation was introduced. Plain
payloads must still be exactly width×height×2. Transparent `0xf81f` semantics
come from the existing decoder; there is no new color key.

The pure SDL-free selector returns `NO_JOIN_REQUIRED`, `SUPPORTED_JOIN`,
`UNSUPPORTED_CONTEXT` or `INVALID_INPUT`, plus exact six-cell facts, stable
owner/manager, key/variant, height/anchor cells, offsets, phase and evidence.
Historical Graphics IDs and candidate bytes have no selection authority.

Optional join preparation occurs after every body and foreign resource, through
the existing physical asset pool and unchanged limits: 16 MiB payload/image,
64 MiB logical RGBA and the existing asset count cap. All necessary metadata
and reserved pool bytes are checked before publishing the assembly. Each join
uses one eager Combined texture; it is not forced through Type-30 ground
geometry and does not receive a fictitious footprint. The runtime-bounded
TextureCompatibility policy remains unchanged.

Activation requires every selected attachment and the independently ready body.
One missing/bad image disables the whole optional assembly while retaining the
complete current Tower body, walls and foreign resources. Shared successful
resources are retained in the ordinary pool; optional failure never removes a
foreign/shared texture. No extra occupancy, permissions, replacement claims,
Objects, IDs, roads or save records are produced.

The existing Sandbox road-ground replacement query is shared with the stored
renderer and explicitly excludes attachment indices. Those indices belong to
the join array, not the historical cell array. An authored collision first
failed with the old query: replacement of historical cell index 0 suppressed
unrelated attachment index 0. The production pixel regression now preserves
that attachment. Historical singleton and one-cell replacement behavior stays
exact; no Road selector, profile, topology or permission code changes.

At the existing body front ground key, the first group follows the body; a
small appended `StoredTowerAfterActors` phase lets front pieces follow actors
at that same ground key. The comparator remains the transitive depth/X/layer/ID
order. Later spatial objects still draw/pick in front; this is not Tower-last.
Whole-image interpenetration remains unresolved. Culling uses each component's
actual bounds; cached alpha and the same effective order return the original
Tower marker/owner. Transparent holes admit underlying hits. F8 Wall/Monument
visibility controls the whole assembly; Historical Snapshot omits new joins.
F1 uses cached six-cell/source/resource/activation facts, without reparsing a
manager or scanning neighbors in a frame.

## Real source cases and bounded acceptance

| Map / Tower ID = manager | Origin | Marker height | Selected components |
|---|---|---|---|
| Kaifeng 3 | (86,120) | 0 | c9: 0,1,2,3; ca: 0,1 |
| Kaifeng 4 | (86,141) | 0 | c9: 1,2,4,5; ca: 1 |
| Kaifeng 5 | (111,120) | 0 | c9: 0,3,4,5; ca: 0 |
| Kaifeng 8,10 | (142,165), (111,141) | 0 | no join required |
| Zhengzhou 2 | (125,117) | 1 | no join required |
| Zhengzhou 3 | (93,117) | 1 | c9: 1,2,4,5; ca: 1 |
| Zhengzhou 4 | (93,82) | 1 | c9: 0,1,2,3; ca: 0,1 |
| Zhengzhou 6 | (93,105) | 1 | c9: 1,2,3,4; ca: 0 |
| Zhengzhou 7 | (93,98) | 1 | c9: 1,3,4; ca: 0 |
| Zhengzhou 8 | (107,117) | 1 | c9: 0,1,2,4,5; ca: 1 |
| Zhengzhou 9 | (125,77) | 1 | c9: 0,3,4,5; ca: 0 |
| Zhengzhou 10 | (102,77) | 1 | c9: 0,1,2,3; ca: 0,1 |

The first positive cases have n0/n1/n2/n3 absent and n4/n5 ordinary walls.
Kaifeng ID 3 has raw absent terrain `0x4`, walls `0x4080`, raw Objects 0/4,
with no original neighbor owner. Zhengzhou ID 4 has absent terrain `0x80`,
wall `0x4080`, and height 1. Zhengzhou IDs 6/7 exercise independently validated
GateHouse ID/manager 1 at the required neighboring cells, not its terrain bit
alone. Complete facts/provenance are in the ignored local report.

Kaifeng selects/prepares/activates 16 parts on three Towers; Zhengzhou 37 on
seven. Five/eight original bodies remain active, sharing the same body image.
All eight possible variants occur on the real maps and are actually submitted
once in the normal Application's 1× frames. There is no missing real variant
coverage in this measured corpus. These are ten supported assemblies, not 53
new Towers or a corpus-wide original-renderer PASS.

The eight shared join images add 64,756 logical RGBA bytes and eight one-time
uploads per map. Body accounting remains 392,472 bytes / three textures.
Actual per-Tower extra draws are the selected counts above, subject to normal
viewport clipping/culling. The normal production frame/Inspector/picking
observer records zero new values across all 12 IO/decode/upload/World/navigation
counters. Session shutdown releases all owned textures.

Authored production tests cover all six presence sides, typed Tower/Gate
predicates, wrong/missing/ambiguous owner, inactive source, unknown view/context,
map edges, missing inputs, historical-ID independence, format/group/payload/budget
bounds, complete fallback, physical reuse across Towers, signed heights
-128/-2/1/127, asymmetric pixels, alpha holes, owner picking, actual exterior
culling, 1×/1.15×/1.5×/2×/4×/back, F8/Snapshot and House/Walker ordering on both
sides and equal keys. Existing Tower/Gate, road-ground occlusion and complete
scene-composition tests retain their independent oracles.

Normal Application evidence is **SCRIPTED**, using ordinary menu/map loading,
SDL events, held state and a fixed QA frame clock. No World/camera state is
injected. Software and Metal are distinguished from native input or human
acceptance. Before/after use identical original files and compatibility metadata,
camera, zoom, mode and World tick. Direct production captures are separate
supporting evidence. Failed captures with missing relocated compatibility
metadata are excluded and were repeated with identical resources. Human
playtest is not claimed.

The identical-camera software/Metal comparison contains 104 before/after
capture pairs over all 13 Towers at 1×, 1.15×, 2× and 4×. There are zero changed
map pixels outside the union of the actual attachment image bounds (with one
raster-rounding pixel), and all eight variants have measured actual draws.
Full existing body component/resource/geometry/height/claim descriptions are
identical. All five maps' normal-App F5/F9/F5 full SaveDocuments remain equal
before/after and across view changes. Xia, Handan and Badaling are independent
unaffected controls.

The original-road/gate transport fixture reproduces 26 complete saved/checkpoint
documents byte-for-byte on Handan, Badaling, Kaifeng and Zhengzhou; all four full
reports match, including WorldSnapshots, paths, reservations, arrival ticks and
funds. Xia's complete CLI demo/resume reports also match. Fresh policy-1/policy-2
audits retain 102 prepared and 65 explicitly refused maps out of 167, and all
10,370 policy-2 original roads. The 3,366 policy-1 and 1,224 policy-2 complete
component files are byte-identical to the preceding Road baseline; complete
policy-2 corpus report matches apart from elapsed time. No new class is admitted.

Ignored evidence roots: `.local/ordinary-tower-wall-joins/`, with
`app-comparison.json`, `body-comparison.json`, `controls-comparison.json`,
`transport-regression/report.json`, `xia-control/report.json` and
`corpus-comparison.json` and `integrity-report.json`. All 1,464 original files
(819,711,091 bytes) retain the preceding corpus inventory and exact SHA-256s.
Every other tracked source stays byte-identical except the listed bounded
presentation/test/starter/documentation changes and the Sandbox replacement
query seam described above.

### Final validation

These complete runs include the final attachment-index boundary fix. Each
configuration executes all 145 registered tests, with zero failures, skips,
compiler warnings or production/test/resource/starter source changes during
the run:

| Configuration | Complete CTest result | CTest wall time |
|---|---|---|
| Debug | 145/145 PASS | 1,026.59 s |
| Release | 145/145 PASS | 110.75 s |
| RelWithDebInfo, ASan + UBSan | 145/145 PASS | 409.29 s |
| Standard macOS package Release build | 145/145 PASS | 58.01 s |

The sanitizer uses `halt_on_error=1`, UBSan stack traces and
`detect_leaks=0`; system SDL 3.4.14 itself is not instrumented. Owned-renderer
texture counts reach zero after the authored and normal-App sessions.
Tower/Gate/Landscape/Picking, Original Road visuals, ground occlusion, scene
composition, budgets, routing, save/recovery, determinism and the registered
City endurance suites are included, without selective omission.

`sh tools/package_macos.sh` and the final
`python3 tools/verify_macos_bundle.py dist/OpenEmperor.app` pass. The local
arm64/macOS-26 `0.1.0-alpha.2` candidate records revision `8ca16f950b5a` and
`dirty=true`; it is not a release. The normal-menu, relocated/unzipped,
synthetic/original Xia industry/City, F5/F9/process-restart, signature,
dependency-closure and negative bundle checks pass. There are nine bounded
compatibility metadata JSON files and zero bundled proprietary assets.

All three exact starter commands, including the original-data path containing
spaces, pass normal-menu startup in fresh isolated roots. This SDL-only smoke
check quits after ten menu frames; map-loaded scripted acceptance is the
separate normal-App evidence above.

Additional existing alpha checks also pass on the final builds:

- 100,000-tick AlphaEndurance: Release 67.869 s, Debug 718.095 s,
  ASan/UBSan 102.311 s; full deterministic saves, navigation and economy checks.
- 3,000-frame production render stress in Release and ASan/UBSan: 2.979 /
  27.903 s; zero simulation ticks, source files unavailable after load, and zero
  decode/upload/stored-order-build deltas.
- 100 normal-menu session lifecycle runs: 53.977 s; owned stored textures
  reach zero after every session. Walker/building/road/fire lifetimes retain
  their separate production regressions.

The ignored final reports are `debug-tests-report.json`,
`release-tests-report.json`, `sanitizer-tests-report.json`,
`extra-checks/report.json`, `launcher-smoke/report.json`,
`bundle-verification.json` and `dist/package-report.json`. The superseded
pre-boundary partial runs are archived separately and are not final PASS claims.

## Test start

The existing starter builds the current normal application and creates a new
isolated settings/save/recovery root for each run:

```sh
sh tools/test_ordinary_towers.sh .local/gog-extracted/app Kaifeng
sh tools/test_ordinary_towers.sh .local/gog-extracted/app Zhengzhou
```

Choose New Sandbox, wait for map rules, then Start Sandbox. The original Towers
cost nothing and need no Sandbox construction. For the checked map hashes,
inspect Kaifeng storage `(86,120)` or Zhengzhou `(93,82)`. Both positive cases
show the two `0x4ca` transitions plus four `0x4c9` pieces around the unchanged
body. F1 reports owner, key/variant, phase and active/fallback state. F8 Snapshot
shows the comparison without the new attachments. Arrows and zoom use ordinary
controls. Normal activation is independent of this starter and any private
manifest/debug switch.

No personal session, setting or save was used or modified. No original assets
are tracked or bundled. No commit, push, tag, release or publication was made.
