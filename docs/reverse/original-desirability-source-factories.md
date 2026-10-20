# Original desirability: seven concrete factories and geometry limits (OE-12)

This extends the [source-admission](original-desirability-source-admission.md)
and [source-link](original-desirability-source-state.md) studies with the actual
constructor dispatch for seven original model types. It also compares their
initial geometry with the current World candidates. **OE-12 and Gate B remain
BLOCKED.** Original factory identity is evidence about an original type, not
proof that an authored World Object is that original influence source.

## Sources and reproduction

**EXE-OBSERVED:** Read-only `Emperor.exe`, SHA-256
`6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e`,
PE image base `0x400000`. Addresses are static virtual addresses. The original
executable was not run. The [model register](../original-economy/model-provenance.md)
and [typed IDs](../../src/simulation/original/ModelSchema.h#L19) separately bind
the model-row names; no original model-value tables are copied here.

Private checkers, disassemblies, probe and JSON reports are under
`.local/original-economy/oe12/source-factory-research/`, outside Git. From the
repository root:

```sh
python3 .local/original-economy/oe12/source-factory-research/industry-audit/check_industry_factories.py
python3 .local/original-economy/oe12/source-factory-research/market-farm-audit/check_market_farm_factory.py
python3 .local/original-economy/oe12/source-factory-research/well-health-audit/check_well_health_factories.py
python3 .local/original-economy/oe12/source-factory-research/root/run_footprint_probe.py
python3 .local/original-economy/oe12/source-factory-research/root/check_footprint_comparison.py
```

The production probe is compiled against the unchanged, fingerprinted Release
libraries and calls the existing `building_footprint(CityV16,7,Object)` seven
times. It creates no World and places no source. These geometry queries do not
grant construction permission. The callback comparison independently checks
ten bounded code windows/54 instructions/169 bytes, 15 explicit assertions,
28 vtable slots and seven initial-side bytes. Factory ownership is checked by
the three separate dispatch audits. Overlapping windows are not unique combined
coverage; these are static research and read-only query results, not feature
or gameplay acceptance.

All three factory audits pass with zero byte mismatches:

| Private factory audit | Code windows | Instructions / bytes | Explicit code assertions |
| --- | --- | --- | --- |
| Industry35/43/54 | 57 | 1276 /4036 | 161 |
| Market59/Farm193 | 42 | 1021 /3211 | 82 |
| Well72/Herbalist207 | 26 | 459 /1459 | 55 |

## Concrete dispatch and callbacks

All seven checked cases pass the actual original type through `0x42d360` into
`0x51c660`. Each audit checks the predicates and earlier dispatch branches for
its own exact types, then follows the selected constructor to the installed
table. Intermediate constructor wrappers are retained in the private reports.
The table below summarizes the selected paths, not an exhaustive call trace.

| Original model type | Selected path after `0x51c660` | Final constructor / vtable | Desirability path |
| --- | --- | --- | --- |
| ClayPit35 | `0x5f0e20 → 0x5ac260` | `0x5ac3a0 / 0x7bca9c` | Square; runtime side byte. |
| Kiln43 | `0x5f0e20`, exact predicate `0x5f50f0` | `0x5f50c0 / 0x7bfc2c` | Square; runtime side byte. |
| Warehouse54 | `0x5d3580` | `0x5d61e0 / 0x7be1bc` | Square; type-specific side callback. |
| CommonMarketSquare59 | `0x5d3580`, predicate `0x543d90` | `0x543450 / 0x7b6f3c` | Special shape, not the square branch. |
| Well72 | `0x51bef0`, selector0 | `0x51c090 / 0x7b5eb4` | Square; runtime side byte. |
| Farmhouse193 | `0x4c2960` | `0x4c5b80 / 0x7af1e8` | Square; runtime side byte. |
| HerbalistsStall207 | `0x51bef0`, selector1 | `0x51c0b0 / 0x7b6114` | Square; runtime side byte. |

All seven tables select shared initializer `+0x94 → 0x428aa0`: supplied type
to runtime `+0x14`, status3, side byte `+0x07` from `0x823598 + 24*type`.
For the five ordinary side-byte cases, `+0x168 → 0x413a00` returns false and
`+0x170 → 0x416ad0` returns that unsigned runtime side byte. This describes the
direct initialization assignment and callbacks. The shared initializer also
invokes virtual slot `+0x90`; a complete transitive audit of every initialization
helper and later lifecycle is not supplied here. Byte-based square dimensions
below therefore describe the checked lookup defaults, not a universal final
initializer or restored-state invariant.

Warehouse54 also has the false square predicate, but its `+0x170` is
`0x5d3720`. This passes the signed runtime type to `0x5d3f50`; exact type54
selects `0x5d3f63 → 0x4fa3a0`, which returns3. The separate initial-side table
byte also equals3, but is not the evidence for that callback's result.

Market59 instead has `+0x168 → 0x4e1c40`, returning true. The influence manager
therefore invokes geometry slot `+0x16c → 0x544700 → 0x5446d0` and the separate
spread path `0x44ea70`. Initial side byte1 does not establish a 1×1 influence
footprint. Its `+0x170` callback returns0 for type59; that unused square-branch
result is not a zero-sized influence footprint. This pass does not reconstruct
its complete special-shape contract.

The subsequent [market descriptor/spread study](original-market-desirability-shape.md)
establishes conditional4×7/7×4 influence-bounding rectangles for its two checked
selector blocks and a distinct rectangle-spread arithmetic path. Physical
occupancy, full selector/lifecycle validity and World identity remain separate.

## Comparison with current World geometry

**READ-ONLY INTEGRATION:** The query uses the current
[production footprint function](../../src/simulation/World.cpp#L142) and
[functional candidate register](../../src/simulation/original/OriginalModelMapping.h).
Rule7 still refuses the actual source-building commands. The World column is
the function's geometry result for a candidate, not a paid or admitted source.

| World candidate / original type | Original callback / lookup default | Current World geometry query | Result |
| --- | --- | --- | --- |
| ClaySource /35 | Square; side-byte default2 | 2×2 | Matching default dimensions only. |
| Pottery /43 | Square; side-byte default2 | 2×2 | Matching default dimensions only. |
| Warehouse /54 | Square; type54 callback returns3 | 2×2 | Different geometry. |
| Market /59 | Separate special-shape path | 1×1 | A simple square equivalence is unproved. |
| Well /72 | Square; side-byte default2 | 2×2 | Matching default dimensions only. |
| Farm /193 | Square; side-byte default3 | 1×1 | Different default dimensions. |
| HealthPost /207 | Square; side-byte default2 | 2×2 | Matching default dimensions only. |

The two mismatches and the market's distinct path prevent a blanket geometry
substitution. Matching lookup defaults in the other four cases do not establish source
identity, placement costs, goods/services, status/link lifecycle or original
ownership. All seven candidate mappings remain `functional=Inferred` and
`mechanics_identity=Unknown`; none is upgraded by this research.

This research changes no World footprint, source status, saved state or
build/restore/funding gate. Admitted runtime behavior requires the separate
[versioned contract](../original-economy/architecture-map.md).
Static source linkage does not establish original execution or human acceptance.
