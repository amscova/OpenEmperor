# Original goods needs and delivery: bounded static evidence

OE-06 isolates a deterministic goods query and a conservative transfer between
two supplied inventories. The original delivery callback supplies the numerical
branch contract below. It does not establish World goods units, actor routes,
market availability, producer behavior or a transport clock. No original
executable is run and no callback is installed in the current World.

## Sources and repeatable checks

| Read-only source | SHA-256 |
|---|---|
| `Emperor.exe` | `6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e` |
| `Model/EmperorBuildingModels.txt` | `7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6` |

Addresses are static virtual addresses in the PE32 image based at `0x400000`,
not file offsets or RVA. The private read-only checker
`python3 .local/original-economy/oe06/delivery-check-evidence.py` maps each
instruction back through its PE section to the original bytes. It checks 11
bounded windows, 428 instructions and 1,240 instruction bytes, with 15 specific
semantic assertions, six pointer bindings and six goods-table bindings. The
report `.local/original-economy/oe06/delivery-evidence.json` records zero byte
mismatches and unchanged sources. Original disassembly and complete tables
remain ignored.

The checker also evaluates 64,821 bounded authored arithmetic cases for source
conservation. These are numerical checks of the transcription, not an original
runtime replay or a copied balance table. The model preparation, labels and
difficulty clamp are established separately by the
[model contract](original-model-contract.md).

## Callback, observations and eligibility

The concrete Market constructor publishes vtable `0x7b6f3c` at `0x543491`.
Its slot `+0x2c` binds the delivery callback at `0x5437b0`. The callback first
requires House type 2 through 17. It then admits positive signed-short occupants,
or, for a nonpositive population, an elite type 11 through 17. Therefore an
ordinary empty House receives nothing from this callback even though a narrow
goods-need calculation may have an empty-House fallback.

The callback itself has no status or House-byte-`+0x09` check. Those must not be
invented as numerical goods predicates, nor does their absence prove that all
upstream actor paths admit every such House. Its actor construction, proximity,
source eligibility, food and side effects remain separate context.

| Observation | Original representation |
|---|---|
| House type | Signed short at House `+0x14`, bounded to 2 through 17 by this callback |
| Stored model row | Signed short at House `+0x16`; canonical initializer is `max(type − 3, 0)` |
| Occupants | Signed short at House `+0x20` |
| Size | Unsigned byte at House `+0x07`; the helper tests only whether it exceeds one |
| Goods stocks | Independent signed-short quantities in House own-info |
| Source availability | Signed 32-bit aggregate quantity returned through Market virtual `+0x264` |

The House own-info virtual binds `0x416b50`, which returns House `+0xc8`.
The Market quantity getter binds `0x5d4a60`; it sums matching nonempty storage
slots. The quantity getter at `0x4f8200` returns a slot's raw 32-bit value at
`+0x08`. The transfer callback passes the chosen quantity to source-removal
virtual `+0x298`, bound to `0x5d50c0`, which subtracts from actual matching
slots. No unit conversion is visible in these windows.

## Six distinct goods and model selection

The callback traverses six records in a fixed table; each independently binds a
commodity, House-stock index and model column. FoodQuality is absent from this
table: it is a separate byte attribute associated with the food path.

| Good | Model field | House own-info stock |
|---|---:|---|
| Hemp | 9 | Signed short `+0x14` |
| Tea | 11 | Signed short `+0x1e` |
| Ceramics | 10 | Signed short `+0x16` |
| Bronzeware | 13 | Signed short `+0x1a` |
| Lacquerware | 13 | Signed short `+0x1c` |
| Silk | 12 | Signed short `+0x18` |

Bronzeware and Lacquerware have distinct inventories but share the same prepared
model field. This delivery mapping alone does not choose one as a substitute
for the other; the separate needs predicate and consumption path have their own
contracts.

The initial delivery row comes from House virtual `+0x22c`, bound to
`0x51a370`, the [next-row selector](original-house-evolution-contract.md).
It selects the next canonical model row, keeps the highest row, and skips
EliteAbandoned when the current type is EliteUnoccupied. The goods loop then
retains current row 7 for canonical LuxuriousApartment, instead of moving into
the unoccupied elite row. In particular, an ordinary evolving House generally
stocks for its next row; the fork's unqualified current-row lookup differs.

For a model-backed need, helper `0x546b90` obtains the prepared model value and
multiplies it by four when the unsigned size byte is greater than one. The goods
callback separately doubles that result for the House elite predicate, bound to
`0x518d90`. Thus this is not a general footprint-area formula: sizes 2 through
255 share one factor, and elite status supplies an additional independent
factor. The pure observation is a byte, not a geometric width inferred from
World cells.

There is one local exception. Exactly zero occupants selects need 10 for Hemp
and Tea, bypassing the model lookup and both size and elite multiplication.
The original test is equality to zero, not `<= 0`. An elite House with a negative
signed population therefore takes the model-backed branch; ordinary nonpositive
Houses have already been rejected by callback admission. Whether a real game
state can produce a negative population is not established here.

## Target, fallback and partial delivery

For the original branch on a nonnegative stock domain, let `n` be the resolved
need, `s` the destination stock and `a` the available source quantity:

1. A zero need or zero source availability skips the transfer.
2. Set target `t = max(n, 10)` and tentative quantity `x = t − s`.
3. If `x <= 0` or `x > a`, replace it with `n − s`.
4. Floor `x` at zero and cap it at `t`.
5. If positive, transfer `min(x, a)` to the House and remove that same quantity
   from the source.

The comparison in step 3 uses source availability, not need. There is no extra
`min(x, n)` after it. This is a material difference from the fork. For authored
need 1 and stock zero, source availability 10 delivers 10, whereas availability
9 delivers 1. With destination stock 1, availability 9 delivers 9, but
availability 8 delivers zero. These examples describe branch behavior, not
original model balance values. Replacing the routine by a single
`min(shortfall, availability)` would erase this discontinuity.

The target is a refill threshold. It is not evidence of a physical House
storage capacity, and a preexisting stock above it is not automatically
discarded. Source inventory and destination capacity are distinct constraints.

## Explicit safety boundary of the pure kernel

The original writes destination quantities with word-width addition and sums
source slots in an unchecked 32-bit accumulator. Static evidence does not turn
those operations into a safe arbitrary-integer contract. Negative prepared
requirements also pass the original callback's `need != 0` branch; interpreting
them as positive deliveries would require further behavior outside the ordinary
stock domain.

OE-06 therefore treats nonnegative inventory observations and sufficient
destination headroom as explicit pure-operation preconditions or bounds, uses
widened intermediate arithmetic, and disables nonpositive prepared needs.
These arithmetic guards are authored safety behavior, stated separately from
the original instruction transcription. They cannot create goods, subtract
more than a supplied source holds, overflow a destination or grant a producer
stock. They do not infer the original Market slot capacity or install an
aggregate inventory in the World.

The result remains an SDL-free kernel over explicit values. Original runtime
parity, market dispatch, actor visits, source-slot ordering, caller-side goods
availability, transport persistence and human gameplay acceptance remain open.
