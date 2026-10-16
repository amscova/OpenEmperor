# OE-03: isolated desirability preparation

The SDL-free `openemperor-original-desirability` library implements the bounded
spread and house-query contract described in
[the static evidence register](../reverse/original-desirability-contract.md).
It links only the existing read-only model module. Neither the application nor
`World`, rendering, navigation or persistence links this new kernel. There is no
new rule, save field, source-selection pass, frame scan or automatic activation.

## Inputs and arithmetic

`spread_spec(model, building, difficulty)` obtains all four fields through the
checked OE-02 model adapter. Footprints are supplied separately; `SZE` is the
score delta, never a building size. Missing or incorrectly labelled model rows,
invalid enums and difficulty arithmetic errors propagate rather than selecting
a fallback. Sources are applied in supplied order, without sorting.

The first surrounding square ring receives the initial value. A period of two
retains it for two rings; a nonpositive period advances after every ring, as
observed in the EXE. Each following step adds the delta, with an absorbing zero
at a sign crossing. Initial zero and nonpositive range do nothing. Range is
capped at ten. Footprint interiors receive no contribution from their own
source. Every cell addition saturates separately to −100…100.

House queries return the maximum signed cell value over the complete square,
then add an explicitly supplied bonus without the cell clamp. The original
conditional +10 is observed, but its trigger is unknown. The default bonus is
zero; arbitrary caller bonuses are an authored checked API, not original flag
semantics.

These are explicit OpenEmperor safety boundaries:

- Map dimensions are 1…228, at most 51,984 cells.
- Source and query squares have sides 1…6 and must be entirely on-map.
  Surrounding rings are clipped; out-of-map queries never fabricate zero cells.
- One batch accepts at most 4,096 sources. All sources, ring values and
  allocations are prepared before any cell mutates, so rejection is atomic.
- Used ring values must lie within `INT32_MIN+100…INT32_MAX−100`, ensuring
  addition to any stored cell fits the original signed intermediate. Used step
  updates and the explicit query bonus are checked against int32 before clamp
  or return. Overflow throws; arbitrary edited-model wraparound is not emulated.
  An unused update after the final ring is not evaluated.
- `clear()` clears values while retaining prepared walls; `clear_walls()` is
  separate. Construction validates dimensions before allocating either grid.

These limits are not permissions to place a building or proof of original
behavior outside the inspected footprint domain.

## Explicit wall hypothesis

`WallBlocking::None` is the default. Merely preparing wall cells has no effect.
`ExperimentalUniform16` is an authored experiment for negative sources only:
sixteen equal 22.5° sectors around the true square centre, starting at +X and
turning toward +Y. Integer squared comparisons against `sqrt(2)−1` select the
sectors without floating-point boundary dependence. A per-ring prepass records
the nearest wall in each sector. Wall cells receive no negative contribution;
only strictly farther rings are shadowed. Positive sources ignore walls.

This two-pass experiment has deterministic behavior independent of traversal
within a ring. It deliberately does not reconstruct the original mixed units,
integer direction intervals, adjacent-sector cases or clipped-edge bypass.
Singleton and multi-cell perimeter traversal differ in the original; that does
not change wall-free scores because each perimeter cell is visited once.

## Direct comparison artifacts

Build and run the isolated diagnostic tool:

```sh
cmake --build .local/reswall/release-build --parallel 2 \
  --target openemperor-desirability-compare
.local/reswall/release-build/openemperor-desirability-compare \
  .local/gog-extracted/app .local/original-economy/oe03/comparison
```

The output directory must be fresh and contained in this checkout's `.local`,
disjoint from the original installation, with no symlink path components. The
tool emits JSON, cell/house CSV tables and SVG/HTML heatmaps. Model values and
generated artifacts stay local; no proprietary table is embedded in Git.

All comparison layers use the same ordered Pottery/Market/Clay sources and
explicit footprints. Mapping original model types to these World functions is
an inferred comparison bridge, not recovered original identity or geometry.

| Layer | Values and geometry | House aggregation |
|---|---|---|
| Bounded original-data kernel | Prepared model fields, square rings, per-add saturation | Maximum after cell accumulation |
| Geometry control | Same model fields/order/saturation, Manhattan distance | Same maximum after cell accumulation |
| Authored City-v13 | Existing production weights, radius and Manhattan helpers | Per-source minimum footprint distance, sum then final clamp |

The geometry control isolates ring shape; it is not the literal City-v13 rule.
The third layer calls unchanged production helpers and retains City-v13's
distinct house aggregation. These authored scene layouts and synthetic probes
are a numerical comparison, not a played original city or runtime parity test.

## Verification boundary

The synthetic CTest uses an independent distance-based oracle rather than
duplicating perimeter traversal. A separate real-source CTest reads the actual
model and verifies its hash/read-only status. Missing optional data alone skips
with code 77; existing malformed data fails. Existing City-v13 tests and the
frozen OE-00 save/policy replay remain separate evidence. Exact commands and
executed results belong to [PROGRESS](../codex-original-economy/PROGRESS.md).

Original full source admission, terrain contributions, wall parity, simulation
timing and human gameplay acceptance remain open. Product integration belongs
to later tasks; this task establishes a pure preparation kernel only.
