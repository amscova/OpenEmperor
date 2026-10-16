# Original house evolution: bounded static contract (OE-04)

This document records the independently checked type and transition rules used
by the isolated SDL-free kernel. It does not connect these rules to the current
World, add original population migration, or establish playable original-house
behavior. The separate [needs and capacity contract](original-house-needs-contract.md)
describes the requirement predicates and the limits of population-capacity
interpretation.

## Sources and reproduction

**EXE-OBSERVED:** The read-only local `Emperor.exe` has SHA-256
`6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e`.
Addresses below are static virtual addresses, with image base `0x400000`;
they are neither RVAs nor physical file offsets. No executable code was run.

**FORK-STUDY:** `HouseEvolution.{h,cpp}` from fork commit
`d861d2912337c09751cb68196f85d5a30752726e` was compared with these instructions.
Its implementation and tests are study material, not the evidence authority.
The original model table is loaded through OE-01/OE-02; original balance arrays,
instruction bytes and full disassembly are not copied into Git.

The private read-only check can be reproduced when the same local installation
and the private research artifacts are available:

```sh
python3 .local/original-economy/oe04/check-evolution-evidence.py
```

It verifies **25 bounded windows, 1,105 instructions and 3,254 instruction
bytes**, five virtual pointers, 16 type-dispatch pointers and 14 counter/target
patterns. The current report has **zero byte mismatches** and confirms that
the executable is unchanged. The ignored report is
`.local/original-economy/oe04/exe-evolution-evidence.json`. This is static
instruction evidence; it is not an independent original-game playthrough.

## Concrete call chain and state widths

Constructor publication at `0x42d4b9` installs virtual table `0x7aba38`.
The checked entries connect the transition handlers to the following methods:

| Virtual slot | Target | Observation |
|---|---|---|
| `+0x1e4` | `0x416b50` | Own-state pointer is `this + 0xc8`. |
| `+0x20c` | `0x51a2d0` | Desirability-only verdict. |
| `+0x210` | `0x51a3f0` | Requirement check; zero selects current row, nonzero selects next row. |
| `+0x22c` | `0x51a370` | Next requirement row. |
| `+0x230` | `0x518de0` | Type setter, followed by original geometry and related work outside this task. |

`0x519034` dispatches through `0x8541bc[type]`. The bounded handlers for types
2–17 occupy `0x519180–0x519d38`; the pointer checks establish their exact type
assignment rather than inferring it from code proximity.

The input desirability is a **signed byte** at House `+0x5e`; occupancy is a
**signed 16-bit word** at House `+0x20`. The devolution counter is an
**unsigned byte** at own-state `+0x39`, equivalently House `+0x101`.
Counter comparisons use unsigned `ja`, not a signed-byte interpretation.
These are bounded source-storage facts; the pure state is not an original
serialized-memory layout or an OpenEmperor SaveDocument.

## Type and row identity

`0x5188b0–0x518903` separately checks House 2–17, elite 11–17 and common
2–10. Initialization `0x518b70–0x518b91`, and the type setter through
`0x518e21`, derive the row as `max(type - 3, 0)`. Vacant type 2 and Shelter
type 3 share row 0. They remain distinct types and do not create another model
row. The named House schema retains its 15 rows from OE-02.

`0x51a370–0x51a397` adds two to the current row for type 11, thereby skipping
the Abandoned row. Otherwise it retains rows 8 and 14, and adds one to any
other row. In the valid House domain, type 11 takes its special branch first;
the retained-row-8 branch therefore does not change that type's next row.

| Type | Name | Current row | Next requirement row | Tolerated Devolve evaluations | Devolve target | Evolve target |
|---|---|---:|---:|---:|---:|---:|
| 2 | Vacant | 0 | 1 | — | — | 3 |
| 3 | Shelter | 0 | 1 | 9 | — | 4 |
| 4 | Hut | 1 | 2 | 9 | 3 | 5 |
| 5 | Plain Cottage | 2 | 3 | 9 | 4 | 6 |
| 6 | Attractive Cottage | 3 | 4 | 9 | 5 | 7 |
| 7 | Spacious Dwelling | 4 | 5 | 9 | 6 | 8 |
| 8 | Elegant Dwelling | 5 | 6 | 9 | 7 | 9 |
| 9 | Ornate Apartment | 6 | 7 | 9 | 8 | 10 |
| 10 | Luxurious Apartment | 7 | 8 | 9 | 9 | — |
| 11 | Elite Unoccupied | 8 | 10 | — | — | — |
| 12 | Elite Abandoned | 9 | 10 | 11 | — | 13 |
| 13 | Modest Siheyuan | 10 | 11 | 49 | 12 | 14 |
| 14 | Lavish Siheyuan | 11 | 12 | 17 | 13 | 15 |
| 15 | Humble Compound | 12 | 13 | 17 | 14 | 16 |
| 16 | Impressive Compound | 13 | 14 | 17 | 15 | 17 |
| 17 | Heavenly Compound | 14 | 14 | 17 | 16 | — |

The table contains derived control-flow rules, not a copy of model balance
data. A target is a transition intent; it does not grant the space or perform
the original geometry transaction.

## Verdict order and explicit opaque flags

With the observed force flag clear, `0x51a2d0–0x51a321` reads the current
row's prepared low and high thresholds. It tests **score <= low first** and
returns Devolve. Only scores above low reach **score >= high**, which returns
Evolve; other scores return Stay. Thus low wins even if a modified model has
equal or reversed thresholds. The score is sign-extended before comparison.

The method has an early flag at byte `0x10bff78`: when nonzero it returns
Evolve before these comparisons. A second independent gate,
`0x4acd00–0x4acd0d`, returns whether global 32-bit value `0xd62408` is nonzero.
Ordinary transition handlers return immediately when that gate is set.
The meanings and runtime producers of these globals are **unidentified**.
The isolated kernel receives explicit snapshot flags for these two observed
conditions and never reads globals or assigns them a guessed gameplay cause.
They default to false; a flag name documents its control-flow effect only.

For ordinary evaluated types, the handlers compose the verdict as follows:

1. Get the desirability verdict; type 12 starts with Evolve instead.
2. Missing any requirement of the current row forces Devolve.
3. If the resulting verdict is Evolve, missing a next-row requirement changes
   it to Stay. Existing Devolve or Stay does not become Evolve from needs alone.

The original `0x51a660–0x51aeb5` is a separate diagnostic function with reason
byte writes, extra global branches and terminal-house shortcuts. It is not the
function called by the checked type-dispatch handlers. The pure transition
verdict follows the actual handlers and does not claim complete diagnostic
function equivalence.

## Per-type evaluation and counter behavior

**Vacant 2:** Handler `0x519180–0x5191f1` first requires signed occupancy
greater than zero, then the ordinary gate to be clear. It checks current row 0
and, on Evolve, next requirement row 1. Only an Evolve result changes type to
Shelter 3. The counter remains unchanged, including on that successful change.
Consequently occupancy alone does not promote a vacant house. The fork's
unconditional occupied-Vacant promotion omits original prerequisites.

**Shelter 3:** `0x519200–0x5192c3` likewise requires positive occupancy
before the gate or verdict. An empty Shelter retains type and counter. An
occupied Shelter applies the ordinary counter rules even though it has no
Devolve target. The fork evaluates empty Shelters and therefore differs.

**Elite Unoccupied 11:** `0x519870–0x51987e` calls the requirement method
with argument 2, which selects the next row. It changes neither type nor
counter and does not read the ordinary gate. Its requirement result is discarded;
the pure state transition models no additional observer. This does not
establish how an original unoccupied elite residence is later inhabited.

**Elite Abandoned 12:** `0x519880–0x519932` ignores desirability and starts
with Evolve, then checks current and next requirements. It can become Modest
Siheyuan 13 while empty. Failed current needs can advance its counter, but no
Devolve target exists. The ordinary gate still suppresses evaluation.

**Other types:** There is no positive-occupancy gate in the checked handlers
for types 4–10 and 12–17. Empty occupancy alone does not stop their evaluation.
The terminal types 10 and 17 still check next-row needs when the initial
verdict is Evolve: row 8 for type 10 and row 14 for type 17. They have no
Evolve target, but the ordinary counter reset still occurs.

For each counter-bearing type, an evaluated Stay or Evolve resets the counter
to zero. On Devolve, a counter below the tolerated count increments once and
returns without a type change. At or above the count it resets to zero and
applies the listed Devolve target, if any. Starting at zero, a type with limit
9 therefore demotes on the **tenth** consecutive Devolve evaluation; limits
11, 49 and 17 act on evaluations 12, 50 and 18 respectively. Shelter and
Abandoned still reset at their limits despite having no lower target.
An incoming counter up to 255 is handled by the unsigned comparison; it never
wraps through 255 because only the bounded below-limit branch increments.

The kernel's checked enum/model preparation, value-returning transaction and
exception behavior are authored API boundaries. Timer decay, goods consumption,
scheduler phase/month mapping, service arrivals, capacity-driven migration,
elite expansion space, diagnostics, renderer selection and persistence remain
separate tasks. No static instruction check is a human acceptance result.
