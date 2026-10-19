# Original house tax: evidenced intermediate and authored currency boundary

This OE27 source contract records static research. It does not establish a complete original tax collector, original currency conversion, original collection cadence or runtime parity. Addresses are static virtual addresses at image base `0x400000`, not file offsets or RVAs.

The unchanged local sources are Emperor.exe, SHA256 `6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e`, and Model/EmperorBuildingModels.txt, SHA256 `7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6`. The ignored checker `.local/original-economy/oe27/research/check-primary-tax-evidence.py` and `primary-tax-evidence.json` bind source hashes, byte counts, modification times, modes, checker identity and repository HEAD. The actual run verified six bounded disassembly windows, 329 instructions, 1,094 instruction bytes and 19 binding assertions; every decoded instruction was compared with mapped PE bytes. Both source identities and metadata remained unchanged. No original executable was run. These results are research evidence, separate from implementation tests, paid gameplay, native launch and human acceptance.

## What the original calculation establishes

Within the bounded traversal `0x5180e0–0x51848f`, instructions `0x518176–0x51817d` fetch prepared House field18 using the stored signed model row at House+`0x16` through getter `0x44cc80`. Instructions `0x51819f–0x5181a3` multiply this value by the actual signed occupants at House+`0x20`. This establishes a population-dependent tax intermediate, without a footprint multiplier.

The original traversal then performs additional work that must not be silently omitted while claiming its complete formula:

- `0x5181ae–0x51822c` uses a virtual signed desirability value, current-row field0, a separate global setting and bracket-dependent factors. `0x518232–0x518235` adds a factor of ten.
- House byte+`0x52` gates the accumulation branch at `0x518237–0x51823d`. Its complete producer/lifecycle and all prerequisites are not established by this bounded research.
- `0x518243–0x5182be` checks a linked source, including concrete building type209, and applies square-root/floating arithmetic and conversion helper `0x7650fc`.
- `0x5182c9–0x518339` accesses House own-info+`0x40`, accumulates a converted quantity and compares the converted value before/after the addition. The own-info getter `0x416b50–0x416b56` returns House+`0xc8`.
- `0x5183b1–0x5183f4` applies positive report conversions using an added5,000 and division by10,000. The separate downstream manager call at `0x518456` is not evidence that a simpler expression is the complete original treasury booking.

The meaning and producers of every global setting, source relationship, byte predicate, rounding boundary and collection-calendar trigger remain unresolved. The model's label “Tax-rate multiplier” alone does not establish a division by100 or a Funds denomination. The fork adds a field name but no independently evidenced tax collector.

## Difficulty and capacity remain model facts

The existing [model provenance contract](../original-economy/model-provenance.md) independently establishes Building difficulty arithmetic `base * modifier / 100`, truncating toward zero, and House arithmetic `clamp(base + modifier, -99, 100)`. The new checker rechecks those pinned instruction windows and independently prepares all15 House rows for all five typed difficulties.

In this pinned model every prepared TaxRate is nonnegative, with range0..24; capacities range0..74. Capacity and TaxRate modifier entries are zero at all five difficulties. Shelter, Hut, Elite Unoccupied and Elite Abandoned have zero TaxRate. Therefore difficulty must not acquire an invented tax or capacity multiplier. Existing prepared thresholds, building costs and risks may differ according to their actual columns. Other valid user-supplied models retain their own hash and modifiers; the pinned ranges are evidence about this source, not universal parser restrictions. Active economy preparation must explicitly validate its consumed signed fields instead of narrowing a negative value to unsigned money.

The [house lifecycle contract](original-house-lifecycle-contract.md) retains separate census/admission row selectors, actual inhabitants and family boundaries. Tax uses the current concrete House row; an empty Elite plot contributes no occupants or revenue. A retained population after devolution remains actual population and must not be replaced by current capacity or multiplied by footprint area.

## Proposed OE27 adapter boundary

The bounded implementation contract may normalize the evidenced population/rate product as **one Funds per prepared TaxRate per actual inhabited, nonburning occupant per authored816-tick month**. This normalization, eligibility and World cadence are authored OpenEmperor rules; they are not the complete original collector above. Zero model rates remain zero. No invented percentage divisor, legacy25/40/60 payment, collector visit or free mission reward follows from the source evidence.

The separately proposed upkeep is authored **two Funds per installed required worker per400 placement-relative ticks**, including paused, unstaffed and burning installed buildings. Model Employees supplies a count, not an original wage or maintenance denomination. Roads, houses and zero-worker objects do not acquire a worker fee.

Any adopted runtime must version and save its actual phase/history, preserve signed debt, check prospective income, counters and expenses before the tick's first mutation, and reconcile `starting Funds + collected tax = construction + upkeep + current Funds` without narrowing uint64 lifetime counters. Debt is an expense result, not a grant or a prohibition on ordinary production/transport. Construction still requires available Funds. Strict restore must bind per-House payments to actual tax history and reject forged totals even when the global accounting equation is preserved. The authoritative runtime contract and real acceptance evidence belong in the OE27 economy documentation; this research does not claim their implementation or validation.
