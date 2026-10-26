# Original market influence: descriptor bounds and rectangle spread (OE-12)

This follows the [seven-type factory study](original-desirability-source-factories.md).
For CommonMarketSquare59, the two checked descriptor blocks yield inclusive
influence-bounding rectangles of **4×7 or7×4**, selected by additional runtime
state. Its rectangle-spread wrapper differs from the audited square kernel.
These are bounded static observations, not physical occupancy or World identity.
**OE-12 and Gate B remain BLOCKED.**

## Sources and reproduction

**EXE-OBSERVED:** Static virtual addresses in the read-only local `Emperor.exe`,
PE image base `0x400000`, SHA-256
`6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e`.
Original bytes, descriptor tables, disassemblies and checkers remain private
under `.local/original-economy/oe12/market-shape-research/`. The executable was
not run, including for the synthetic comparison below. From the repository root:

```sh
python3 .local/original-economy/oe12/market-shape-research/geometry-audit/check_market_geometry.py
python3 .local/original-economy/oe12/market-shape-research/descriptor-audit/check_market_descriptor.py
python3 .local/original-economy/oe12/market-shape-research/spread-audit/check_rectangle_spread.py
python3 .local/original-economy/oe12/market-shape-research/root/check_market_call_bridge.py
python3 .local/original-economy/oe12/market-shape-research/root/run_square_contrast.py
```

| Static check | Windows | Instructions / bytes | Explicit code assertions |
| --- | --- | --- | --- |
| Descriptor geometry | 13 | 228 /666 | 43 |
| Descriptor creation/state/archive | 27 | 653 /2047 | 56 |
| Rectangle spread | 8 | 689 /2161 | 40 |
| Manager call bridge | 6 | 260 /674 | 34 |

All four pass with zero byte mismatches; windows overlap, so these are not
unique combined counts. The geometry audit also checks two table blocks and
pointer forwarding; the archive audit independently counts the serialized
field prefix. Source bytes, modes and modification times remain unchanged.
The spread check's eight traces and360 perimeter cases are authored algebraic
checks of the bounded interpretation, not original-game execution.

## The descriptor supplies position and shape state

The actual type59 constructor `0x543450` selects descriptor constructor
`0x546c80` and stores its pointer at Building `+0x158`. The descriptor's final
vtable is `0x7ab800`.
Its base constructor copies global DWORD `0x8c7628` to descriptor `+0x0c`;
type59 alone does not fix one orientation or position.

The checked setup/apply path `0x5451a0 → 0x544220 → 0x5428b0` has two modes:
one uses that global selector with explicitly supplied coordinates; the other
uses unsigned byte `NonHouseInfo+0x60` and signed Building origin words
`+0x0a/+0x0c`. Market slot `+0xc0 → 0x544d80` also rebuilds this descriptor from
the latter state. `+0x1e8 → 0x416b50` identifies the embedded NonHouseInfo at
Building `+0xc8`, rather than an unrelated object with the same member offset.

The NonHouseInfo schema1 serializer `0x51c3a0` writes/reads that selector as
one byte at raw Info offset94, including the schema word. This is an archive
location, not a new OpenEmperor parser field. A setup branch writes the selected
descriptor state back to that byte. These local paths do not prove a complete
load/lifecycle call order or that every supplied/saved selector lies in0/1.

## Exact four-output geometry

Market slot `+0x16c → 0x544700 → 0x5446d0 → 0x542d10` preserves four output
pointers. For descriptor fields `X=D[+0x04]`, `Y=D[+0x08]`, `s=D[+0x0c]`, its
concrete methods return28 records per selector, two declared selectors and a
coordinate-record table. Each record is16 bytes. The geometry function reads
the final record of block `s`, at index `(s+1)*28−1`, obtains signed `(dx,dy)`
and outputs:

```text
(min(X,X+dx), min(Y,Y+dy), max(X,X+dx), max(Y,Y+dy))
```

| Checked selector | Opposite-corner offset | Inclusive influence-bounding dimensions |
| --- | --- | --- |
| 0 | `(3,6)` | 4×7 |
| 1 | `(6,3)` | 7×4 |

The complete28-record coordinate bounding box of each checked block agrees
with that final-record result. This does not import those records as physical
claims or establish every interior cell as traversable/owned. `0x542d10` itself
does not locally validate the selector; no additional blocks or malformed-state
behavior are admitted by this research. Runtime descriptor coordinates are not
silently translated into the authored World's coordinate or ownership domain.

## A separate rectangle-spread contract

The manager's checked call bridge forwards exactly
`(x1,y1,x2,y2,DES,STP,DELTA,RGE)` to `0x44ea70`. Its worker `0x44e7f0` receives
the current four bounds, contribution and ring number. For valid inclusive
bounds, ring `r` visits the perimeter of
`[x1−r,y1−r]..[x2+r,y2+r]`, outer ascending Y then ascending X, with each corner
visited once. Range is capped at10; after each contribution, the resulting
cell value is saturated to−100…100.

This wrapper starts its period counter at1. It contributes first, increments
the counter, then adds DELTA/reset0 when the counter reaches or exceeds STP. Unlike the
separate square wrapper `0x44ecd0`, it has no initial-DES-zero return and does
not clamp a sign crossing to zero. STP2 therefore first updates after ring1;
the square path first updates after ring2. Neither caller behavior nor the
original model field labels permit treating these wrappers as interchangeable.

The audited rectangle routines also contain no local map/diamond clipping or
side6 guard. Their worker uses direct stride228 addressing and the original
sector logic, including a global-flag branch for nonnegative contributions.
The flag's complete runtime meaning and original wall parity remain unresolved.
These observations grant no safe out-of-map or arbitrary-overflow input domain.

## Synthetic comparison with the existing square core

**AUTHORED SYNTHETIC CHECK:** A private C++ probe freshly compiles the unchanged
`Desirability.cpp`, `ModelSchema.cpp` and `ModelData.cpp`, applies synthetic
on-map singleton square sources without walls and reads four separate rings.
No World, original model values, market instance or original code is executed.
The rectangle column is a separate small arithmetic trace derived from the
bytechecked wrapper; it is not a gameplay or whole-worker parity test.

| Synthetic `(DES,STP,DELTA,RGE)` | Current square-core rings | Derived rectangle-wrapper trace |
| --- | --- | --- |
| `(2,2,−3,4)` | `2,2,0,0` | `2,−1,−1,−4` |
| `(0,1,3,4)` | `0,0,0,0` | `0,3,6,9` |
| `(−2,2,3,4)` | `−2,−2,0,0` | `−2,1,1,4` |
| `(6,2,−1,4)` | `6,6,5,5` | `6,5,5,4` |

All four contrasts pass without source changes. The existing
[square-core contract](original-desirability-contract.md) and its historical
OE-03 PASS retain their bounded scope; no rectangle implementation is added.
The current API supplies one square side, not independent rectangle endpoints.

## Remaining integration boundary

The conditional market bounds, selector provenance and distinct arithmetic are
now evidenced. Full saved-state admission, selector/lifecycle validity, complete
wall/edge behavior and a paid World→original source contract remain unresolved.
Construction, restore and funding remain subject to the separate
[runtime contracts](../original-economy/architecture-map.md).
These checks change no production core, footprint, parser, World, save, budget
or source gate and establish no release acceptance.
