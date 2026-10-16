# OE-04: independent house evolution

`openemperor-original-house-evolution` is an SDL-free numerical library with no
application, World, renderer, navigation, clock or persistence connection. Its
inputs are checked model values and an explicit House observation. Its output
is a transition intent represented by a new `HouseState`; it does not perform
the original type setter's geometry transaction or supply any service or goods.

The bounded original evidence is documented separately for
[evolution](../reverse/original-house-evolution-contract.md) and
[needs/capacity](../reverse/original-house-needs-contract.md). Fork commit
`d861d2912337c09751cb68196f85d5a30752726e` is study material. The implementation
follows independently checked original control flow where the fork differs.

## Preparation and identity

`HouseEvolutionRules(model, difficulty)` eagerly prepares every field of all
15 named House rows using OE-02's checked signed addition and −99…100 clamp.
Every source label, field and difficulty is validated before the object is
available. The immutable result retains its source SHA-256 and difficulty;
queries perform no file reads or reparsing. Failure in any row, including an
otherwise unused field, rejects preparation rather than publishing a partial
ruleset or substituting defaults.

`HouseType` identifies original building types 2…17. `ModelHouse` identifies
15 model rows 0…14. They are separate types and neither is a World House level.
Vacant 2 explicitly aliases Shelter's model row 0. `house_row()` and
`next_row()` enforce the valid type domain. EliteUnoccupied selects next row
10, skipping Abandoned row 9; terminal common type 10 still selects next row 8,
while terminal elite type 17 selects its current row 14. There is no sixteenth
model row and no automatic transition from common housing into elite housing.

The snapshot uses the observed input storage domains: signed-byte desirability,
signed-word residents and goods, unsigned-byte access/food quality/counter.
Water is an explicit normalization of its byte's nonzero predicate. These
fields are not an original memory layout or a serialized save schema. In
particular a service byte supplies only a current observation; the library has
no visit grant, timer decay, dispatcher, goods arrival or consumption operation.
Signed negative stock observations remain comparable to signed model thresholds.

## Separate queries and one evaluation

- `needs_met(rules, row, state)` checks a selected row. Nonzero prepared presence
  requirements activate a check even when negative; access bytes 1…255 are
  present. Food quality and each stock use inclusive thresholds. Either bronze
  or lacquer must independently reach the shared threshold; adding two short
  stocks does not satisfy it. Either Daoist or Buddhist access satisfies its
  shared field. No footprint/population multiplier is applied.
- `desirability_verdict()` tests `score <= low` before `score >= high`. Low wins
  for equal or reversed edited thresholds. The explicit force flag returns
  Evolve before score comparison.
- `evolution_verdict()` follows the actual per-type update composition:
  Abandoned starts at Evolve; other types use the desirability verdict. Missing
  current needs forces Devolve. Missing next needs converts Evolve to Stay.
  This query does not implement the separate original diagnostic routine.
- `evolve_target()`, `devolve_target()` and `devolve_limit()` expose transition
  bounds separately. Targets are optional, not sentinel integers or placement
  permissions.
- `evolve_step()` applies occupancy and evaluation gates and returns a new
  snapshot. Input and rules remain untouched. Only type/counter can differ.

`EvolutionContext` names two independent observed conditions by their technical
effects. `evaluation_blocked` suppresses a step; it deliberately does not alter
the separate table queries. `force_desirability_evolve` affects the score
verdict, but never bypasses current or next requirements. The original global
flag producers and gameplay meanings are unknown. Both inputs default false;
the kernel does not read, infer or set their original globals.

Vacant needs positive occupancy, an unblocked evaluation and the full Evolve
decision before becoming Shelter. Its counter remains unchanged on success or
failure. Empty or negative-occupancy Shelter stays unchanged. EliteUnoccupied
always retains type/counter; its original discarded requirement query is not
an automatic occupation or development mechanism. Other types are evaluated
even when empty, as in the inspected handlers.

Evaluated Stay/Evolve resets ordinary counters. Devolve increments a counter
strictly below its limit; at or above the limit it resets and applies a lower
target if one exists. Common types tolerate nine failures and act on the tenth;
Abandoned uses 11/12, Modest 49/50, higher elite 17/18. Shelter and Abandoned
still reset despite lacking a lower target. A raw counter of 255 cannot wrap,
because only the bounded below-limit branch increments. Terminal types can
produce Evolve and reset the counter while retaining their type.

All enum/model checks and the value-returning API are explicit OpenEmperor
safety boundaries. The original needs predicate has no type guard; this API
rejects invalid House types instead of accepting a non-House observation.

## Capacity queries have a bounded meaning

`rules.row_capacity(row)` returns the prepared signed field 17 without area
scaling or an invented nonnegative clamp. It is a model count, not the complete
effective capacity of a World building. Negative edited values remain readable
model facts and do not grant population or allocate storage.

`census_capacity_row(type)` and `admission_capacity_row(type)` are narrowly named
model-row selectors, assuming the canonical stored row for the supplied type.
They preserve the observed difference for EliteUnoccupied: census selects row 9,
the two inspected admission substitutions select row 11. They do not model a
stale/inconsistent original type-row pair, census eligibility, the original
occupant-count override, arrival guards or local availability floors. Other
inspected consumers directly use the stored row. A universal `capacity(type)`
or full migration claim would erase these distinct paths and is not supplied.

## Reproduction and acceptance boundary

```sh
cmake --build .local/reswall/release-build --parallel 2 \
  --target openemperor-original-house-evolution-tests
ctest --test-dir .local/reswall/release-build --output-on-failure \
  -R '^(original-house-evolution-(synthetic|real)|sandbox-city-v13-(base|persistence|districts|determinism|endurance))$'
```

Synthetic tests use independently specified type/row/target matrices and
isolated model requirements, including every type's Evolve/Stay/Devolve input
boundary and deliberate no-target/no-evaluation cases. A separate real-source
test reads all 15 rows at five difficulties, retaining the actual source hash.
Missing optional data alone skips with code 77; existing malformed data fails.
Executed configuration results and the frozen legacy replay are recorded in
[PROGRESS](../codex-original-economy/PROGRESS.md).

This establishes a pure numerical decision contract. Timer lifecycles,
religion/food production, consumption, original phase timing, population
admission, elite footprint expansion, save integration and human gameplay
acceptance remain separate work. The existing authored City-v13 calculations
and every previous World/save contract retain their current implementations.
