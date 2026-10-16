# Original desirability: bounded static contract (OE-03)

This document separates observations from the supplied original executable,
the inspected fork, and explicit OpenEmperor hypotheses. It specifies a pure
score calculation; it does not establish original-game runtime parity or change
the existing authored City-v13 desirability implementation.

## Source binding and repeatability

| Source | SHA-256 |
|---|---|
| `Emperor.exe` | `6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e` |
| `Model/EmperorBuildingModels.txt` | `7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6` |

The model-field identities and difficulty transforms come from the preceding
[model contract](original-model-contract.md) and
[model provenance register](../original-economy/model-provenance.md). The fork
study is pinned to `amscova/OpenEmperor` commit
`d861d2912337c09751cb68196f85d5a30752726e`,
`src/simulation/original/Desirability.h` and `.cpp`. Its `Intended` wall mode is
a corrected-centre hypothesis, not evidence of the original wall calculation.
No fork code, original tables, original bytes or full disassemblies are copied
into this document.

All addresses below are static virtual addresses with PE image base
`0x400000`, neither RVAs nor physical file offsets. Read-only PE-section mapping
checks 15 bounded instruction windows: 1,233 instructions and 3,853 instruction
bytes, with zero mismatches. Two bounded data checks separately validate square
query offsets and multi-cell perimeter counts. The executable is unchanged
before and after these checks; it was not executed.

The private repeat command is:

```sh
python3 .local/original-economy/oe03/check-desirability-evidence.py
```

The private report is
`.local/original-economy/oe03/exe-desirability-evidence.json`. These ignored
research artifacts are not dependencies of the distributable core or tests.

## Spread, bounds and arithmetic

The rules in this section describe the checked **square** path. The later
[market rectangle study](original-market-desirability-shape.md) establishes a
separate wrapper with different period phase, zero-input and sign-crossing
behavior. Those findings neither widen this square API nor turn its historical
OE-03 acceptance into a rectangular-source or complete runtime-parity claim.

| Static window | Observed mechanism |
|---|---|
| `0x44ecd0–0x44ed84` | Reject nonpositive footprint side; an initial zero value returns; cap range at ten rings; start at ring one with the supplied initial value. |
| `0x44ed41–0x44ed70` | After a ring, increment a period counter; when it reaches the supplied step, add the signed step-size delta and reset the counter. Preserve the initial sign by clamping a crossing to zero. |
| `0x44ed90–0x44f17a` | Traverse a square ring around the footprint, clip boundary candidates on the slow path, read each cell as signed, and clamp each source addition separately to −100 through 100. |
| `0x44f180–0x44f1ca` | Byte cell storage/get/set/raw addition helpers. Saturation belongs to the spread caller, not the raw byte-adder helper itself. |

Ring one is the immediately surrounding perimeter, including diagonal corner
cells. For footprint origin `(x,y)` and side `n`, ring `r` is the outer square
from `(x−r,y−r)` through `(x+n−1+r,y+n−1+r)`, excluding its interior. A source
does not add its own value inside its footprint. Each perimeter cell appears
once. The resulting score is square-distance based, not a Manhattan diamond.

The ring receives its current value before the period update. A positive step
therefore keeps the initial value for that many rings. A nonpositive step
updates after every ring in the inspected executable; the fork instead treats
that input as never advancing. Nonpositive range performs no additions, and
range above ten does not extend the original spread. Initial zero remains a
no-op even if the delta is nonzero. For a fading source, crossing zero never
turns it into a source of the opposite sign.

The signed cell limit is applied after each contribution. Consequently the
order of overlapping sources matters when saturation occurs: summing all
contributions first and clamping once is a different calculation. The original
uses 32-bit arithmetic. Checked OpenEmperor arithmetic outside the representable
intermediate domain is a deliberate input boundary, not emulation of arbitrary
edited-model integer overflow.

## Footprints and deterministic traversal

`0x4b0710–0x4b0760` generates singleton ring coordinates in ascending Y, then
ascending X, retaining only the perimeter. `0x4bb810–0x4bb994` prepares their
linear offsets using the original storage stride of 228. Multi-cell square
perimeters are generated independently by `0x44cde0–0x44cec8`, walking clockwise
from the top edge immediately above the footprint's leftmost column and
finishing the remaining top-edge cells. `0x44cf60–0x44cfc2` converts these
coordinate pairs to offsets. Neither generator duplicates corners.

`0x44d090–0x44d0d1` selects the multi-cell perimeter and caps its table side at
six. The inspected original table domain is thus singleton plus sides two
through six. A generalized larger-footprint calculation must not be presented
as demonstrated original behavior. No model field supplies the footprint:
Building field 3, abbreviated `SZE`, is the signed score delta.

`0x44f1d0–0x44f322` clears the score grid, visits eligible building pointers in
manager traversal order, and then calls the terrain-source pass.
`0x44eb20–0x44ecce` visits that pass with outer ascending Y and inner ascending
X. A detached core must retain the caller's ordered source sequence; sorting by
score, grouping positive and negative sources, or relying on unordered-container
iteration would alter saturated sums. The inspected routine also has source
eligibility and special-shape branches. OE-03 does not reconstruct the complete
original source manager, terrain admission or arbitrary shaped buildings.

## Building query

`0x4acd10–0x4ace13` computes the maximum signed cell score over a positive
multi-cell square footprint. The bounded table at `0x81ff18` has six columns
and six rows; the first word of each eight-byte record is exactly
`X + 228×Y`. The adjacent word is independent auxiliary indexing, not an
additional desirability score. A singleton dispatches a virtual query; the
base query at `0x4273d0–0x4273e0` reads its origin cell.

After the footprint result is stored, a nonzero building byte at offset `+0x60`
causes an additional ten points. There is no subsequent −100/100 clamp in this
routine. The result is stored in a signed-byte field, so this observation does
not establish arbitrary unbounded bonuses or safe behavior beyond that field's
domain. The meaning and lifecycle of the bonus flag are outside this bounded
study. A pure query can accept an explicitly supplied bonus while documenting
its own checked result domain; it must not silently clamp the query back to the
cell range or infer that every building receives the original ten-point bonus.

## Wall evidence and the experimental boundary

The negative-source path in `0x44ed90–0x44f17a` tests Objects-grid bit `0x4`
at runtime base `0xf37da0`; this is distinct from the terrain and saved owner
grids. It records per-source sector obstruction and skips the obstacle cell's
negative contribution. A farther ring is blocked when its distance is strictly
greater than the recorded obstruction distance. Positive contributions bypass
this wall path. The original also contains neighboring-sector cases and a
separate clipped-boundary path that bypasses this sector logic.

`0x44e550–0x44e76e` prepares sixteen angular intervals from integer direction
endpoints. `0x44e770–0x44e7e0` returns the first matching interval, or a sentinel
when no direction matches. These are not proven equivalent to sixteen equal
`atan2` wedges. More importantly, the negative spread computes target tile
coordinates but subtracts source-coordinate intermediates scaled by four; for
larger footprints it also adds twice the side to those intermediates. Replacing
this with a true footprint centre changes the observed unit handling.

Thus the following remain **unknown**, rather than silently repaired original
rules: the complete original angle-boundary behavior, the intended meaning of
the mixed coordinate units, every obstacle producer's identity/lifecycle,
neighbor-sector interactions, and clipped-edge wall parity. The existing object
bit alone does not license adding an original wall-building command or changing
map permissions.

An explicitly selected, corrected-centre wall hypothesis is valid for isolated
experiments if its sector definition, scan policy and expected synthetic cases
are stated. It is authored behavior. Its output must remain distinguishable
from the wall-free, statically evidenced arithmetic core, and it must never be
advertised as an exact original-wall mode. No wall hypothesis is enabled by
loading a model, constructing the independent map, or running the existing game.

## Acceptance limits

Static evidence supports bounded ring geometry, model-field use, sign handling,
per-add saturation and square-footprint maximum queries. Synthetic tests and
same-input heatmaps establish the independent implementation's stated contract
and its measured difference from City-v13. They do not prove original execution,
world-level source selection, wall parity, original timing, complete housing
evolution or human gameplay acceptance. Existing World, navigation, rendering,
map policies, authored City-v13 snapshots and save schemas remain governed by
their existing implementations.


## OE-12 follow-up: source admission and the standard footprint path

The later [bounded source-admission study](original-desirability-source-admission.md)
checks the complete status helper, the manager's additional zero-word predicate,
the actual type-to-model reads, and constructor-linked base/Industrial virtual
callbacks. It establishes the standard square-source path more precisely than
the original OE-03 pass; it does not reconstruct every source factory, special
shape, terrain source or wall branch. The original OE-03 counts above remain
historical evidence rather than being retroactively expanded.

The current paid Vacant container's explicit original status is zero. Applying
the observed status1/3 predicate would reject it before its spread fields are
read. Neither its House-type binding nor the separate Vacant/Shelter evolution
alias grants influence-source authority. Authored World-object identities,
construction, restore and financing remain governed by the unchanged
[OE-12 blocker decision](../original-economy/desirability-integration-blockers.md).
