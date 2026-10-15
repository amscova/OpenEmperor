# Authored cliff and ramp integration

The criticized two-Ramp scene now uses a continuous earth/stone slope through an
explicitly opened rock edge. The saved RampLink supplies both the slope and the
cutout. The remaining rock flanks and lower support are retained. This is a
bounded OpenEmperor visual fallback, not verified original Emperor cliff art.

## Baseline and root cause

Work started at clean HEAD `9d6ac7a88bc34c5f013d153af420bf64b0b48e61`.
The pinned original inventory is1,464 files/819,711,091 bytes, manifest SHA-256
`dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`.
The read-only Emperor.exe SHA-256 is
`6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e`.
Rule4/policy2/schema20, rule5/policy3/schema21 and rule6/policy3/schema22 are unchanged.

The exact source is `Cities/Authored Ramps.map`, independently authored by the
existing `author_ramp_terrain.py`, with a height1→0 boundary at storageX116 and
the existing paid Bridge `(118,114)→(120,114)`. Starting from the existing normal
prebuild SaveDocument, ten paid road commands connect the second site. Paid Ramp
commands create `(115,114)→(116,114)` and `(115,117)→(116,117)`, $40 each.
The four captures show0/1/2 Ramps at tick0 and an actual loaded Clay courier
mid-Ramp at tick58. Funds are730/690/650/650; no snapshot fields are injected.

The old side renderer had independent vertical40px faces and sixteen shared
direction/material templates. Its four phases repeated visibly. It had no
RampLink input, so construction could never excavate a side face. Twelve timber
cross-planks, tall parallel rails/posts and regular stone courses made the
separate Ramp look like a ladder leaning against the closed face. Correct signed
Ground projection alone could not solve the missing local topology/composition.
The side pass preceded Ground, and the ordinary half/back/front Ramp/actor merge
followed Ground. Those anchors and pass boundaries remain in place.

The private diagnosis predates production edits:
`.local/cliff-ramp-visuals/research/root-cause-before.md`. The independent
four-cell material-repeat assertion was run against the old renderer and failed
with `cliff material visibly repeats every four cells`; its log is retained as
`research/before-failing-test.log`.

## Original evidence and its limit

**RAW MAP FACT:** real saved slot16 references exist on Xia, Banpo, Anyi and
Badaling. The original source bytes/heights/terrain are retained. A saved slot
local value, a resource key and a physical archive record are separate domains.

**EXE-OBSERVED:** the pinned registration resolves slot16 to
`DATA/China_Elevation.sg3`. Existing generation uses the key`0x601` path through
`0x53eae0/0x53ec5d`; `0x5403c0` also has conditional normalization and calls the
height setter at`0x540511`. See [map first draw](../reverse/map-first-draw.md).
This does not establish complete current selectors, orientation, anchors or
post-normalization height state for a generic cliff/ramp family.

The requested historical physical records decode through the existing decoder:

| Record | Dimensions | Type / side | Group |
|---|---|---|---|
| 201 | 78×102 | 30 / 1 | 1 |
| 229 | 78×83 | 30 / 1 | 1 |
| 211 | 78×120 | 30 / 1 | 1 |
| 344 | 78×84 | 30 / 1 | 2 |
| 312 | 78×106 | 30 / 1 | 1 |
| 349 | 78×68 | 30 / 1 | 2 |
| 231 | 78×70 | 30 / 1 | 1 |
| 353 | 78×93 | 30 / 1 | 2 |

They are bounded, unmirrored records. Inspected201/211/349 show pillars and
irregular caps, including transparent separation between cap and ground. Blindly
stretching them across the authored40px step would not be a verified selector or
closed terrain surface. Metadata, decoded private examples and real-map facts
are in `research/elevation-metadata.json` and
`research/original-elevation-evidence.json`. No original pixels enter Git.

**VERIFIED ORIGINAL SELECTOR:** no newly complete generic cliff/ramp selector was
established. Existing supported stored/regenerated selection is unchanged.
**OPENEMPEROR VISUAL PREVIEW:** the explicit authored sides and slope described
below. **UNRESOLVED:** complete original height normalization, slope identity,
cliff anchors/layer composition and original first-draw parity. Original maps do
not acquire new height-only walls or new Ramp permissions.

## Prepared edge geometry and excavation

The existing strict eligibility remains: complete matching source/policy3 signed
heights, exact raw`0x80/0xc0`, objects0, normal permitted unprotected Diamond land
on both sides, no gate/water, signed difference exactly1. Each physical high/low
edge is unique. A shared doubled-storage corner lattice counts connected joints,
outer/inner corners and terminal edges. It controls end treatment and exact shared
boundaries; internal high/high edges generate no wall. Only this local authored
edge stream is sorted far to near. The global painter is unchanged.

Every edge prepares a closed and opened raster. Across-edge position and depth
below the upper lip define a central corridor of half-width0.30 and depth0..20.
The cut is intersected with the eagerly prepared opaque Ramp footprint. Thus
excavation follows the actual projected sloped surface instead of removing a
whole tile. Side flanks and rock below it remain; pixels lacking opaque supported
slope coverage remain closed. Shared lattice bounds stay exact. Small visual
relief and a≤1.5px toe taper to zero at shared ends; neither changes ground rights.

Saved Ramp endpoints and signed heights look up exactly one edge in a bounded
sorted index. Successful construction updates only cut state. Initialization/F9
derive it from saved RampLinks. A separate optional preview edge never modifies
committed state; invalid/cancel/focus/layout/UI release clears it. There are no
new saved fields, map overrides or transport rules.

The continuous coordinate material replaces four-phase tiling with small irregular
vertical stone facets, interrupted fissures, stepped highlights, soil pockets,
rooted grass lip and an uneven earthy foot. It is prepared once, deterministic,
map-name independent and contains no copied original pixels. The first grey,
large-facet prototype was rejected during visual review and refined before final
capture; recoloring the old wall was insufficient.

## Slope, actors, picking and F8

The elevated Ramp uses continuous compacted earth/stone with irregular gravel,
quiet low edging and cut/embankment sides. No elevated transverse planks, tall
rails or posts remain. Its road width narrows slightly at the endpoint joins.
All points use the same endpoint projection and signed height×40 as the saved
path/edge-progress foot interpolation. Actor feet retain their authoritative
height; picking reuses the actual cached raster geometry.

Under the fixed2:1 projection, canonical ascendingX/Y slopes have collinear along
and across vectors: the tread is exactly edge-on. A shallow2.2px authored stone
crest supplies visible thickness on the unchanged foot line. This is an explicit
presentation limitation, not a shifted foot/height or changed camera projection.
DescendingX/Y expose the full slope. Negative signed heights use the same math.

Sides remain before Ground; each half's back bank/surface precedes its local
actors and its near bank follows them in the existing merge. Original foreground
and Bridge ordering are unchanged. No global sort correction is introduced.
Partial F8 layers use closed sides when hiding the Ramp; Regenerated Full and
Walls/Monuments use cutouts with Ramp bodies. Historical Snapshot hides authored
sides and retains the previous flat-debug Ramp template byte for byte.

## Bounds and pure rendering

Limits remain512 faces and4,096 raster runs per face/state. The new per-edge
closed/open run vectors have an explicit combined8MiB capacity budget; exceeding
either bound refuses preparation. The actual80-edge scene uses2,293,760 bytes
for those run capacities. The static Ramp opaque-coverage lookup adds221,184
bytes, prepared before frames; it never performs renderer readback.

Frames traverse prepared/cullable runs. Preview only selects a cached edge; it
does not regenerate materials/geometry. Pure-frame and repeated P-drag checks
assert zero World copies/executes/ticks, BFS/routes, files, asset decodes and
texture uploads. The visual fixture checks100 frames in each state, exact
F5/advance/F9 World equality and normalized map pixels, then resource release.

## Reproduce and inspect

```sh
# Default opens the normal Release game with both paid ramps and the bridge.
sh tools/test_cliff_ramp_visuals.sh .local/gog-extracted/app
# Full software capture matrix, original controls, saved-state process restarts.
sh tools/test_cliff_ramp_visuals.sh .local/gog-extracted/app --capture
# Also run the existing full Ramp/Bridge/city/corpus acceptance runner.
sh tools/test_cliff_ramp_visuals.sh .local/gog-extracted/app --regression
# Existing before-construction starter remains available.
sh tools/test_city_v16_ramps.sh .local/gog-extracted/app --ui
```

Each run builds current Release, verifies the pinned source inventory, authors
the same map in a fresh ignored root, and uses paid production commands. No
personal save/settings or original bytes are overwritten. `--ui` is the explicit
default; `--capture` stays in dummy/software and `--regression` adds the existing
full acceptance runner. Continue starts real Clay transport through Ramp/Bridge;
F5/F9 preserves it. The generated prebuilt `captures/state-2.json` is a normal
schema22 SaveDocument, not renderer state.

Before/after matrices are private under `.local/cliff-ramp-visuals/before/` and
`after/`: `sides-2-z0-full.png` is the two-Ramp overview,
`sides-2-z1-full.png` is2x, `sides-2-z2-full.png` is4x,
`sides-2-z3-full.png` is fractional, `sides-3-z2-full.png` shows the real courier.
States0/1 show no/one Ramp. `plain-*` is the side opt-out control; `*-f8-*`
records Historical Snapshot/Ground/Elevation comparisons. State JSONs preserve the complete
paid construction/movement authority. A separate process loads each actual
state save and compares its full SaveDocument and normalized map PNG.

Software/scripted pixels, original-source diagnosis, normal executable launch
and native/human visual/input acceptance are separate evidence. No desktop
automation or original-fidelity PASS is inferred from these captures.

## Recorded acceptance

The capture starter completed at
`.local/city-v16-ramps/cliff-qe8ck0x_`, source SHA-256
`7fef177b0cc7c46961ec1e6d058057b73cc163b7c5e72afbb8b803a94abe3c30`.
All four complete state SaveDocuments are byte-identical before/after. Four
separate processes load the actual0/1/2-Ramp and mid-Walker saves, retaining
eight exact SaveDocument/map-PNG comparisons. Every state's100 pure frames and
F5/advance/F9 checks pass; all textures release.

Independent pixel comparisons preserve all eight Historical Snapshot map images,
four opt-out/no-Ramp map images at1/2/4/fractional zoom and the actual Bridge/banks
crop. The four original normal render-check reports retain every non-timing
field; normal original View captures prepare zero authored cliff faces. Banpo
still shows repeated historical elevation, and Anyi/Badaling still show the
existing separated/floating saved-edge composition. These are retained original
limitations, not repaired or relabeled original parity.

The existing complete Ramp acceptance runner finished at
`.local/city-v16-ramps/run-Dsi6l0`:12,000 paid authored ticks, eight actual outbound
and returning Ramp roles, eleven full-document process checkpoint pairs,
23 malformed-document rejections plus truncation, and unchanged167/165/2 census.
All1,485 old policy1/2/3 actual/zero/one summaries and50 historical full save
documents remain exact. The nested controls repeat the old rule5 Chang-an Sui
12,000-tick Bridge ride, its restarts and the Xia/Handan rule3/rule4 long-play
contracts. Original inventory and source digest remain unchanged during the run.

All three complete CTest suites pass154/154 without skips: Debug1189.58s,
Release98.51s and ASan+UBSan313.98s. Sanitizer diagnostics and compiler warnings
are absent. Both the new two-Ramp `--ui` starter and existing before-construction
`--ui` starter launch the normal Release executable under dummy/software and
exit0 through their owned processes. This proves executable startup/rendering,
not native Cocoa/Metal input or human visual acceptance. Build/test logs, pixel
controls and final manifest are in `.local/cliff-ramp-visuals/validation/`.

## Graded shoulder continuity follow-up

This bounded geometry correction starts at clean
`0e4166374918c5512e4c72fcb3af9ea4e2e75b4f`. It preserves all material/color
functions, the road surface, original assets, signed projection, actor heights,
local painter slots, Bridge rendering, rule6/policy3/schema22 and gameplay.

Two independent defects were confirmed before their respective corrections:

1. The tread was continuous, but the outer shoulder used
   `20*t - (half ? delta*40 : 0) + 20*edge`. At the shared midpoint its two
   limits differ by`-40*delta` pixels. That is the undisturbed terrain's actual
   vertical step; using it as the graded shoulder reference created separate
   cut/fill triangles which could paint across the road. A rendered-pixel
   regression against the old renderer fails with
   `triangular bank panel remains in rendered clearance`.
2. After correcting that reference, two independently antialiased halves still
   exposed the background along their diagonal seam. Two alpha-half fragments
   blended successively retain approximately25% background. Rendering against two
   different clear colors confirmed this at storageY, delta−1, height−1,1x,
   t0.5/side−0.15. The independent two-background test failed before the raster fix.

Coordinates relative to endpoint A are
`S(t,s) = ((axisX ? 40 : -40)*t + (axisX ? -40 : 40)*s,
          (20 - 40*delta)*t + 20*s)`.
The tread keeps its existing width`0.26 + 0.04*sin(pi*t)`. The shoulder's
outer edge now uses the same continuous height function at side±0.46, without a
half-dependent plateau. At t0 and t1 it meets the actual endpoint road planes;
at t0.5 both halves share the same cross-section. For a descendingX ramp, the
outer near-edge y values are9.2/39.2/69.2, instead of9.2/(19.2 or59.2)/69.2.
For ascendingX they are9.2/−0.8/−10.8. Y mirrors x; signed starting heights
translate the entire construction without changing these differences.

Only the elevated ramp is rasterized as joined geometry, once at preparation.
Each resulting pixel belongs to exactly one existing half/back/front component,
using the inverse ramp-plane projection. For the edge-on ascending case, ownership
uses projection along the unchanged foot line and existing shallow crest. This
removes overlapping half-alpha seam fragments without a new frame pass, texture,
palette, global sorting rule or floating height. The flat Snapshot path is kept
unchanged. The existing24 cached components,2,048-run per-component limit and
opaque-coverage lookup remain bounded; no renderer budget is increased. Joined
runs stream directly to those components without an oversized intermediate mesh.

`AuthoredTerrainSides` requires no source change: its existing prepared cut
intersects the updated actual opaque ramp coverage. Lateral rock and lower
support remain. Existing pixel tests continue to check all four directions,
negative heights, openings, flanks, lower closure, cancelled preview and pure
frames. New Ramp tests add exact endpoint/midpoint shoulder interpolation, left/
right midpoint limits, forbidden triangular panels and background-independent
interior pixels at1x/2x/4x/1.73x in both axes and height directions. Existing foot
projection/picking and tint assertions are retained.

Reproducible current-HEAD baseline captures are in
`.local/ramp-bank-continuity/before/`; private geometry facts and the two failing
regression logs are alongside them. The same paid two-Ramp/Bridge scene is used
afterward, including the actual mid-Ramp courier, F8, F5/F9 and saved-state process
restarts. These are software/scripted checks; human native visual acceptance and
complete original Emperor cliff parity remain separate and unverified.

Follow-up validation: the full Release suite passes154/154 (102.88s). The15
selected geometry/scene/input/transport/save/recovery tests pass in Debug
(93.61s) and ASan+UBSan (32.55s), with no compiler warnings or sanitizer findings.
Full Debug/Sanitizer suites and native/human testing were not repeated in this
bounded follow-up; the complete three-configuration results above are historical.
The final test object also fails against the exact renderer source from HEAD,
independently of the new renderer implementation.

`sh tools/test_cliff_ramp_visuals.sh .local/gog-extracted/app --capture` passes at
`.local/city-v16-ramps/cliff-mdvy6zjp`; its captures are also linked under
`.local/ramp-bank-continuity/after/`. The unchanged fixture produces1x/2x/4x and
fractional views, including both endpoint joins, the midpoint, two Ramps, Bridge
and actual courier. Four before/after SaveDocuments,17 unaffected map pixel
controls and all20 original-map PNGs remain identical. Four actual saved-state
process restarts retain eight exact save/map-PNG comparisons, and800 pure frames
plus F5/advance/F9 retain their existing zero-work/equality assertions.

The full existing `--all` Ramp runner passes at
`.local/city-v16-ramps/run-6u54mS`, including12,000 paid Ramp ticks,11 restart
checkpoint pairs, the old Bridge ride and Xia/Handan long-play controls. All50
old complete SaveDocuments and1,485 policy summaries remain exact; census stays
167 complete/165 prepared/2 known conflicts. Original data remains unchanged.
The tested source digest is
`f9521e6da82fe1792a35766dad5ce1eff3dd8cdb31490d72dab168c2f6b3145b`.

Visual inspection of the software images finds the large cut/fill triangles and
midpoint stripe removed, with continuous road joins and retained rock flanks.
This is a visual assessment, separate from the assertions above. The ascending
orientations remain edge-on under the fixed projection; historical original
cliff parity and human native appearance/input acceptance remain open. The
existing texture/palette transition remains deliberately unchanged in this pass.
