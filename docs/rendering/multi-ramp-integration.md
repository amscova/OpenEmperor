# Joined ramp surfaces and road material

The [original asset audit](original-ramp-asset-audit.md) records the complete
Elevation/Terrain inspection, newly verified original ramp sprites, measured
native-fit blocker and current before/after image review. Original ramp identity
is now bounded evidence; original traversal/whole-raster placement remains open.

This presentation correction reproduces the rejected four-ramp scene from
`ac61dcfeae58be41b7f04c2a4125f78357296d94`. One paid ramp crosses
`(115,114) -> (116,114)`; three adjacent paid ramps cross the same height1 -> 0
boundary at storageY117,118,119. The middle edge is the player's reported
`(115,118) -> (116,118)`. The fixture loads the existing authored map and paid
prebuild SaveDocument, buys the connecting roads and ramps with production
commands, and renders the actual SandboxView painter with its detected road pack.
It does not modify Snapshot fields or add free infrastructure.

## Confirmed causes

The previous material had nearly uniform beige tessellation and long grey
shoulders. Its independent per-link banks stopped at lateral coordinate0.46;
each cliff cut stopped at0.30. Immediately adjacent links therefore retained
rock between their cuts and repeated banks beside every narrow tread. The old
half-height and half-raster continuity fixes were necessary but did not address
those composited defects.

The production capture makes this independently measurable: twenty samples along
each internal boundary in the three-ramp group are exactly equal to the same
pixels in the closed zero-ramp cliff. The regression rejects even one retained
closed-cliff witness, rather than accepting a permissive dark-pixel percentage.

## Derived visual topology

`RampTopology` accepts at most64 existing links and one accepted preview. A
pair merges visually only when both canonical axes, signed endpoint levels and
gradient match and both endpoints are displaced by exactly one lateral cell.
X ramps use lateralY; Y ramps use lateralX. Diagonal, longitudinal, reversed,
different-height and source-ineligible edges remain separate.

SandboxView qualifies existing normal endpoint roads against the unchanged
immutable policy3 road, protection, gate, water and signed-height facts. Its
event-only bounded preparation rebuilds after load, paid ramp/road commands,
road batches, repair or a coalesced preview change. Broken endpoint roads stop
visual grouping without deleting a saved RampId. Committed and effective preview
masks are separate; cancellation restores both existing neighbors. Fixed65-entry
mask arrays provide O(1) frame lookup. No grouping is saved or used for transport.

The internal boundary is exactly
`S_r(t,+0.5) == S_neighbor(t,-0.5)`. Each raster pixel has one half-open lateral
owner, as well as one existing half/back/front owner. Internal sides have no
separate bank/front strip, curb or highlight. The exterior retains an irregular
earth/stone edge within0.46. No overlapping bank meshes hide the old gap.

Each eligible cliff face prepares five states: closed and four lateral masks.
The corridor extends to0.5 only on a joined side, keeps the exterior0.30 cut and
depth0..20, and intersects the union of the link's and connected neighbors'
prepared opaque coverage. Neighbor coverage matters because projection shears
the wall's lateral coordinate relative to the slope. Exterior rock and lower
support remain closed. Identical closed/open raster runs share a visibility
bitset; selecting a state performs no new rasterization.

## Road material and composition

The normal RoadVisualProfile already owns deduplicated decoded CPU RGBA.
Compatible replacement profiles supply static78x40 images anchored at39,20.
Preparation uses the existing profile mask lookup: X starts with0xa and adds
its lateral bits; Y starts with0x5 and adds its lateral bits. No physical-record
mapping, image classifier, original selector or texture is added.

Native ramp pixels inverse-map onto the endpoint road material, retaining fine
earth grain and irregular external stones. A whole road rectangle is never
stretched onto the ramp. Repeating entire junction images was also inspected
and rejected: their corner stones produced conspicuous interior white rows.
The initial reflected UV interpolation produced visible stretched beige streaks
and identical neighboring interior strips. Current interiors select native
six-by-four pieces inside the same image's bounded12x8 earth rectangle. Eight
eagerly cached lateral phases and deterministic pixel coverage replace coordinate
interpolation. Both endpoint joins and external source curbs retain normal
sampling. Selection depends on topology and coordinates, never RGB classification.
Missing or incompatible profile entries use the prepared deterministic authored
earth/stone material. F6 switches between the two prepared styles.

Signed height x40, fixed2:1 projection, authoritative actor feet, picking and
the existing local half/back/front merge remain shared. Surface pixels precede
their actors; only an exterior near shoulder uses the later front slot. Valid
graded previews retain an opaque tinted base so an opened cliff cannot expose
the clear background. Partial F8 layers hide preview and committed ramps alike
and use closed cliffs. Historical Snapshot preserves the old flat timber/rail
comparison raster and its tint alpha. No global painter or Bridge path changes.

Canonical ascending ramps remain exactly edge-on under the fixed projection;
their existing shallow crest stays on the authoritative foot line. This work
does not infer original Emperor slope identity or original cliff composition.

## Bounds and regression

Compact seven-byte ramp runs replace the former fixed Face arrays. Both runtime
and authored styles, all four adjacency variants, flat comparison and opaque
bitsets use approximately427KB with the pinned road pack, below the prior
approximately3.5MiB mesh cache. That427KB figure describes the initial single
runtime material phase; current eight-phase preparation remains within the same
3MiB run-cache limit. Preparation enforces4096 runs per part.
Cliff states retain512 faces,4096 runs per selected face
state and an8MiB combined capacity limit; shared visibility avoids storing five
complete material copies. All preparation precedes frames and hover repetition.

`RampRenderingTests` retains the previous signed-height, shoulder continuity,
foot/picking, half-seam and tint checks. `RampMaterialRenderingTests` uses authored
synthetic road profiles to check exact mask selection, native grain, shared-curb
suppression, unchanged endpoints/exterior, absent internal front geometry,
background independence, F6/F8 and cache bounds. `AuthoredTerrainSideTests`
covers both axes/gradients, negative heights, source qualification, joined cuts,
retained exterior/lower support, preview cancellation and exact closed F8 states.

`MultiRampVisualFixture` adds the full production composite: zero, isolated,
three-adjacent, all-four and actual mid-Ramp courier paid states at1x/2x/4x/fractional zoom, F6 and F8;
literal boundary/exterior RGB diagnostics; actual preview/release/cancel;
100 pure frames per state and preview; full World/SaveDocument and normalized
map-pixel equality through F5/advance/F9 and separate process reloads. Frame
counters must remain zero for files, decode/upload, World copies/commands/ticks,
BFS and routes. Image inspection remains necessary; these checks do not confer
human acceptance.

The added144-case direct production renderer atlas covers both axes/gradients,
negative/zero/positive starting heights and1–4 adjacent ramps at1x/2x/4x. Two
backgrounds check opaque tread/half seams, and complete neighboring interior RGB
strips must differ. This is authored renderer evidence, separate from paid
Sandbox captures. `OE_MULTI_RAMP_BEFORE=path/to/extended/captures` enables exact
before/after saves, cameras, F6/Snapshot/no-Ramp controls and Bridge/bank pixels.

## Run the current version

```sh
# Builds current Release and opens the actual game with all four paid ramps.
sh tools/test_multi_ramp_visuals.sh .local/gog-extracted/app
# Software before/after-compatible matrix and separate-process save/pixel checks.
sh tools/test_multi_ramp_visuals.sh .local/gog-extracted/app --capture
# Also repeats the existing full Ramp/Bridge/city/original-corpus regression.
sh tools/test_multi_ramp_visuals.sh .local/gog-extracted/app --regression
```

The generated map and saves live in a fresh ignored `.local/city-v16-ramps`
directory. Original inputs and existing player saves/settings stay unchanged.
Z cycles zoom, F6 compares road materials, and F1 then F8 cycles landscape layers.
The old two-ramp starter remains available for its historical comparison matrix.
Rule6/policy3/schema22, pricing, navigation and all older save contracts are
unchanged. Original-map, scripted SDL, native input and human visual acceptance
remain separate evidence.
