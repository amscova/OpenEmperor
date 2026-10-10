# Original Road surfaces and mixed connections

2026-10-10, baseline `65aa7f94add4ae81d1e832fe357417cf74a37c57`, initially clean.
This is bounded OpenEmperor presentation for City-v16 rule 4 / map policy 2 /
save schema 20. Oliver reports that the Towers look good; his exact Road
screenshot/save was not supplied. The scenes below are independent reconstructions.

## Findings and limits

**A, authored production failure:** `draw_original_road_ground()` called
`render_asset_index()` and `image_origin()`. In regenerated modes both could
select a generic Ground asset instead of the actual historical Road. The new
test supplies distinct Road and green Ground images with different heights and
anchors. The baseline draws the wrong pixels. The bounded historical Road path
now keeps its historical asset, dimensions, origin and culling together, including
signed saved height once. Other landscape selection is unchanged.

**B, authored production failure:** a supported original straight receives a
paid side Road. `RoadTopology` correctly reports a T, but the original surface
continues to show the straight. Distinct independently authored variant colors
prove the actual wrong SDL pixels before the fix, rather than only a wrong mask.

**Additional real-map finding:** Kaifeng `(78,68)` and Zhengzhou `(131,87)` have
exact raw `0xc0`, objects zero, signed height zero and ordinary original-road
authority. Their saved physical Terrain image **750** is grass, not a usable Road
surface. The reviewed cells have **no regenerated footprint-asset override**;
they do not prove cause A in these particular original scenes. Keeping image 750
alone would therefore leave the real surface defect unresolved.

The existing v213 `ResourceGroupLookup` / `RuntimeArchiveLayout` identifies
Ground key **0x61b**, physical Terrain **736–781**, separately from basic Roads
key **0x61e**, **782–799**. Read-only local decoding confirms grass at 750/751.
The pinned standalone load clears saved graphics at `0x53561e`; the ordinary Road
refresh branch at `0x4b70c6` / `0x4b7113` uses the already documented basic table.
Saved Ground is not Road-style authority. These facts support the bounded
OpenEmperor surface choice; they do not reproduce original restoration, its
external style context or the original compositor. See
[the original presentation audit](../reverse/original-presentation-correction.md)
and [first-draw limits](../reverse/map-first-draw.md).

**C, negative control:** a Road two cells away, with the intermediate cell empty,
does not change the original mask or paint a connection across that cell. A
separate authored height-1 original Road beside a height-0 paid Road retains its
height-filtered mask and visible surface; neither side gains a connecting edge.

## Single effective surface

At load/profile replacement, a bounded sorted list indexes only already imported
`World::original_road()` singleton draw items. It does not classify transport.

- A historical Road physically matching one unique prepared replacement-profile
  entry keeps its correct historical image while its mask matches. A changed
  connection uses that same profile's prepared variant.
- Under the validated built-in profile, a historical image in the resolved 0x61b
  Ground family may use the prepared basic Road surface for an already imported
  ordinary road. The historical source must have ready static unmirrored Type-30,
  side-1, 78-wide/3,200-byte geometry. Snapshot retains the stored Ground image.
- Unknown/ambiguous styles, other groups, unsupported geometry, missing masks,
  additive/disabled/unavailable profiles and F7 comparison retain named historical
  fallback. In particular, upgraded 0x61f images such as 817–820 are not replaced.
  Bridges, ramps, wall-top and unknown mixed raw features gain no support.

There is no second variant table, atlas, crop, stretch, pixel synthesis or color
classification. The existing `RoadVisualProfile` and `RoadSpriteSet` provide the
images, native `[39,20]` anchors, nearest sampling and integer-zoom alignment.

Replacement values retain their existing meanings: **0** ordinary/additive,
**1** a ready opaque replacement, **2** a new alpha-128 preview over old ground.
Only an actually prepared replacement suppresses its old singleton image and
overlay. Existing original roads stay opaque during a side-road preview; only
new planned cells are alpha 128. One effective Road surface draws before the
unchanged building/Walker/historical spatial merge. Original roads acquire only
render descriptors, never World Road objects, IDs, purchases or save records.
Tower, Gate, Cliff and Great Wall bodies retain their existing spatial contracts.

`RoadNeighborMask` and preview masks still come solely from `RoadTopology`.
`EntranceMask` stays separate: adding a building does not extend a Road through
its footprint. Permissions, signed heights and legal gate openings filter the
same edges as before. Construction picking remains logical; an original Road's
hidden historical image cannot win spatial alpha inspection.

## Invalidation, diagnostics and resources

Successful commands refresh only affected original cells and their orthogonal
neighbors; buildings use their authoritative footprint rims, including safe
demolition. A successful road batch invalidates once after its atomic commit.
Failed commands do not publish a visual change. Load/session/profile replacement
reconstructs the list from the current World and immutable plan. A revision
mismatch catches initialization/demo changes using the bounded original-road list.

Preview neighbors are cached by the existing coalesced plan-build counter and
validity. Escape, right click, focus loss or an invalid plan drops the proposed
connection and restores current surfaces. Stable hover rebuilds nothing. Frame
work adds no files, decodes, uploads, texture creation, World copies/commands,
ticks, BFS, route refresh or full-map search. The additional list is bounded by
existing singleton draw items, not another map-sized raster.

All variants share the already eager, deduplicated Road textures. Budgets and
`TextureCompatibility` are unchanged. Stored texture draws decrease where an
opaque replacement suppresses a source image; Road sprite draws increase by the
number of actually submitted original surfaces. This is a presentation counter,
not an imported/purchased Road count or first-draw fidelity PASS.

F1 on an original Road adds mask, separate entrance, draw mode/actual AssetId and
fallback reason. `road_visual_diagnostic()` is a bounded read-only cell query.
The isolated fixture/observer records raw cells, allowed edges/heights, historical
and regenerated assets, origins, selected mode and actual draw multiplicity only
at named checkpoints. There is no production frame log.

## Real paid scenes and acceptance

| Scene | Original center | Paid side Road | Paid Well | Before / side mask | Prepared Road assets |
| --- | --- | --- | --- | --- | --- |
| Kaifeng, y straight | `(78,68)` | `(79,68)` | `(79,69)` | `0x5 → 0x7` | `782 → 795` |
| Zhengzhou, independent x straight | `(130,102)` | `(130,103)` | `(131,103)` | `0xa → 0xe` | `783 → 796` |

The technical fixture writes a genuinely empty map checkpoint, then two Houses,
Clay and Pottery through four normal paid commands: **460 spent / 840 funds**,
no mutable Roads. The side Road costs **2**; Well costs **60**. Removal costs zero
and refunds nothing: **522 lifetime spending / 778 funds**, seven commands, tick
zero. Imported original roads remain free immutable facts throughout.

The ordinary Application/main was separately driven through normal Load Sandbox,
paid construction, branch removal, preview/cancel, F1/F8 Snapshot, zoom
1/intermediate/2/4/back, held-arrow panning, actual SDL window resize and F5/F9/F5.
Owned private roots and processes were used. The observer stops immediately on
native takeover. It observes real production Road/Combined submissions and captures
actual SDL output; it does not inject World or camera state. Full World snapshots
match an independent paid-command control, save JSON matches after F9, and all
12 production render counters stay zero with diagnostics enabled. Software and
Cocoa/Metal evidence is **scripted Application evidence**, not human/native-input
acceptance. Proprietary inputs, decoded pixels, runners and captures remain ignored
under `.local/original-road-visuals/`.

Four authored tests cover A, distinct mixed T/cross/corner/removal pixels,
bounded snapshot Ground, both axes and the height negative. Existing independent
Walker-foot/foreground-hole/alpha-pick/one-ground oracles remain active. New checks
include 1×1 and 2×2 entrances/demolition, missing cells, zoom, preview cancellation,
optional failure/F6, immutable-command rejection, load cache reconstruction and
full World/save purity. Both initial A/B production pixel tests failed before
the presentation fix; all four new tests now pass in each complete configuration.

| Current-source configuration | Complete CTest result | Skipped / warnings | CTest seconds |
| --- | --- | --- | --- |
| Debug | 144 / 144 PASS | 0 / 0 | 824.02 |
| Release | 144 / 144 PASS | 0 / 0 | 98.33 |
| RelWithDebInfo, ASAN + UBSAN | 144 / 144 PASS | 0 / 0 | 345.04 |

All **432 test executions** passed. The sanitizer build uses the existing
`OPENEMPEROR_ENABLE_SANITIZERS=ON` address/undefined instrumentation,
`ASAN_OPTIONS=detect_leaks=0:halt_on_error=1` and
`UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`. Owned SDL texture counts return
to zero independently. Production/test/resource/starter hashes are identical
before and after each suite; no test source changed during execution. These
are the existing full suites, including topology/Original Roads, composition,
Tower/Gate, Walker/picking, budgets, saves/recovery, responsiveness and determinism.

The separate transport regression reproduces Handan, Badaling, Kaifeng and
Zhengzhou: **26 complete save documents are byte-identical** to the baseline,
and all four complete reports match, including permissions, actual gate trips,
routes, arrival ticks, commands, costs and economy. **203 frozen source/resource
files** and **1,464 original-data files** are unchanged. No policy, rule or save
format was changed.

## Test the current version

From the repository root:

```sh
sh tools/test_original_road_visuals.sh .local/gog-extracted/app Kaifeng
```

This exact command builds current Release code, creates a fresh isolated app root,
prepares the paid scenes and launches the ordinary application. Choose **Load
Sandbox → C-mixed-road-well**; it starts paused. A/B/D compare unchanged, side-Road
and removed-branch states; `0-original-empty` contains no player buildings. Use
arrows/zoom to the printed Storage cells, **6** to remove the side Road, **1** to
drag a Road, **Escape** to cancel, and **F1/F8** for actual surface/Snapshot diagnosis.
Personal settings/saves/recovery and original files are untouched.

Human Road acceptance, Oliver's exact screenshot/save identity, unknown styles and
complete Emperor presentation parity remain open. No commit, push, tag, package
publication or release was made.
