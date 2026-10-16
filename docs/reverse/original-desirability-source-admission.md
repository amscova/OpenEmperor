# Original desirability: bounded source-admission evidence (OE-12)

This adds static source-selection evidence to the
[OE-03 numerical contract](original-desirability-contract.md). It does not
implement the blocked World integration. **OE-12 and Gate B remain BLOCKED**;
the [integration blockers](../original-economy/desirability-integration-blockers.md)
and [progress record](../codex-original-economy/PROGRESS.md#oe-12--2026-10-16-basisdf29393--blocked)
retain their scope. No construction, budget, source gate, saved status or restore
permission changes follow from this research.

## Source and reproduction

**EXE-OBSERVED:** The independently run private checker binds `Emperor.exe` to
SHA-256 `6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e`,
with PE image base `0x400000`. Addresses below are static virtual addresses,
not file offsets. The [model-provenance register](../original-economy/model-provenance.md)
separately governs model-field identities and their interpretation.

From the repository root, the exact private reproduction command is:

```sh
python3 .local/original-economy/oe12/source-admission-research/root/check_source_admission.py
```

Its report is
`.local/original-economy/oe12/source-admission-research/root/source-admission-evidence.json`.
It checks eight bounded windows, 251 instructions and 766 instruction bytes,
with 38 predicate assertions and eight virtual-table slot checks. Result:
**PASS for these static checks**, zero byte mismatches; the executable's bytes,
modification time and file mode remain unchanged. The executable was never run.
Private disassemblies and checker inputs remain outside Git; no executable
bytes or model-value tables are reproduced here. These checks are distinct from
the older OE-03 ring-arithmetic evidence and from any gameplay acceptance.

## Manager predicates and model-field selection

The checked manager window is `0x44f1d0–0x44f323` (exclusive upper endpoint).
Within its source-processing loop, the observed path is:

| Check or dispatch | Bounded observation |
| --- | --- |
| `0x44f218–0x44f22b` | Call status helper `0x426d10` with literal argument0; a false result skips this source. The complete helper reads runtime status byte `+0x04` and accepts exactly1 or3. Its supplied argument does not change that predicate. |
| `0x44f22b` | Runtime word `+0x3c` must equal0; nonzero skips the source. The later [source-state study](original-desirability-source-state.md) establishes its separate archive location and concrete manager-link use. |
| `0x44f236–0x44f26c` | Read the signed runtime type word `+0x14` and obtain Building fields1,2,3,4 through four calls to `0x44cc50`. These are Desirability, DesirabilityStep, DesirabilityStepSize and DesirabilityRange. |
| `0x44f277–0x44f281` | Invoke virtual slot `+0x168`; false selects the square-footprint path. |
| `0x44f299`, `0x44f2bf` | The separate special-shape path invokes geometry slot `+0x16c`, then spread routine `0x44ea70`. That path is not reconstructed by this evidence. |
| `0x44f2d2–0x44f2ec` | The square path gets its side from virtual slot `+0x170`, reads signed origin Y/X words at runtime `+0x0c`/`+0x0a`, and invokes ring-spread routine `0x44ecd0`. |

The checked getter `0x44cc50–0x44cc79` selects the Building model address
`0xa5b398 + 4 * (13 * type + field)`. Nonzero fields bypass its separate
field0-only price hook at `0x44c380`. Thus the four influence values belong to
the Building model table; the House evolution-row selector is not this source
selector. This does not establish arbitrary signed type indices as safe inputs
or identify every caller's concrete building class.

Runtime word `+0x3c` is distinct from the parsed saved `Subindex`. The current
[archive reader](../../src/maps/OriginalMapEntities.cpp#L245) reads that signed
Subindex from raw Base byte offset18 (decimal). The first pass did not locate
the additional word in the archive; the subsequent
[archive and link audit](original-desirability-source-state.md) places it at
raw Base offset54, independently checked for schemas3/4/5. A saved Subindex0
therefore cannot be used as proof that the additional predicate passes.

## Two checked constructor/vtable cases

The checks establish the following concrete cases, rather than assuming that
every original building shares these callbacks:

| Constructor | Observed installed table and relationship |
| --- | --- |
| Base Building `0x426c90–0x426cd9` | Installs vtable `0x7ab59c` at `0x426cbc`. |
| Industrial `0x51c9a0–0x51c9e9` | Calls the Base Building constructor at `0x51c9bd`, then installs vtable `0x7b65e4` at `0x51c9d9`. |

For **each of these two tables**, four slots are checked directly:

| Slot | Target | Bounded meaning |
| --- | --- | --- |
| `+0x94` | `0x428aa0` | Shared initializer. |
| `+0x168` | `0x413a00` | Returns false, selecting the square path. |
| `+0x16c` | `0x416ac0` | Base rectangle stub; it does not supply a reconstructed special-shape contract. |
| `+0x170` | `0x416ad0` | Returns the unsigned side byte at runtime `+0x07`. |

The checked shared initializer `0x428aa0–0x428b5f` writes its supplied type to
runtime word `+0x14`, sets runtime status3, and writes side byte `+0x07` from
the lookup at `0x823598 + 24 * type`, using the signed supplied type. This
connects these initializer fields to these two concrete constructor tables.
It does not establish the full creation, ownership, post-load or lifecycle
paths of all seven World/model functional candidates. No initializer from the
original executable is replayed into OpenEmperor.

## Consequence for the current Vacant container

The existing [OriginalHouse state](../../src/simulation/OriginalEconomyState.h#L26)
explicitly associates a World BuildingId with original HouseType Vacant2.
Its defaults are original status0 and Access=false; the current
[World validator](../../src/simulation/World.cpp#L444) requires that complete
default state for the paid container, apart from its permitted placement time.

**INFERENCE FROM THE CHECKED PREDICATE:** If that saved status0 is offered as
the original source status, it fails the observed status1/3 predicate before
the manager reads the model fields. Access=false is another current state
fact; this window does not establish Access as a source-admission predicate.
The container comparison itself establishes neither runtime word `+0x3c` nor
the concrete House vtable; the separate link audit does not import original
state into that World container.

This is neither a blanket prohibition of original type2 nor proof that every
Vacant original house can emit influence. The checked manager path does not
reject type2 by a type-specific rule. In particular, the Vacant→Shelter alias
used for House evolution needs cannot identify a Building-table influence row.
The original initializer's status3 does not authorize rewriting the World's
saved status0, introducing a new source type or widening its restore contract.

## Remaining boundary

RE has narrowed original source admission and the square callback chain. It
has not verified all seven type-specific source factories, their complete
status/link-word/lifecycle conditions, special-shape sources, terrain or
wall admission, or an identity between an authored World Object and an original
influence source. Model-file identity remains separate from instance identity.

The strict [OE-12 decision and blockers](../original-economy/desirability-integration-blockers.md)
therefore continue to apply. There is no new production cache, source purchase,
removal path, extra funding, save migration, original-game execution or human
acceptance result. A successful static byte check is not an OE-12 feature PASS
or a Gate-B release.
