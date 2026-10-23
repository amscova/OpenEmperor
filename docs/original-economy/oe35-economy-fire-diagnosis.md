# OE-35: paid economy and fire diagnosis

This is a diagnosis of the existing authored **City-v16 Rule10 / Policy3 /
Schema26 / development revision27**, using real `Cities/Xia.map`, the pinned
original model and Normal difficulty. Original values, simulation, permissions,
historical rules and saves are unchanged. Technical reproducibility, reaching the
housing goal, maintaining it and financing operations are separate results.

The starting Git audit fetched `origin`: local `main` and `origin/main` both
pointed to `9265ebc32c09`, the merge of PR #24. Local
`feat/oe34-paid-progression` additionally contained the unmerged `13fab40`
Longplay extension. That branch/commit and `feature/original-economy-v1` remain
intact. `feat/oe35-economy-fire-diagnosis` starts at `13fab40`; no merge, push,
tag or release is part of this work.

## Reproduce the observations

With the prerequisites in [testing](../testing.md) and your own installation:

```sh
cmake -S . -B .local/original-economy/oe35/build -DCMAKE_BUILD_TYPE=Release -DOPENEMPEROR_USE_SYSTEM_SDL3=ON -DOPENEMPEROR_ORIGINAL_MODEL_TEST_ROOT=/absolute/path/to/Emperor
cmake --build .local/original-economy/oe35/build --parallel 2 --target openemperor-economy-fire-diagnosis
python3 -B tools/test_economy_fire_diagnosis.py --binary "$PWD/.local/original-economy/oe35/build/openemperor-economy-fire-diagnosis" --data /absolute/path/to/Emperor --output "$PWD/.local/original-economy/oe35/diagnosis" --variant all --ticks 60000
```

Output must be a new directory under ignored `.local/original-economy/oe35/`,
disjoint from the installation and build. Existing outputs, overlapping paths
and symlinks are refused. The runner checks all original files' SHA256, mode and
mtime, executable identity and the supplied seed bytes. It never searches for
personal saves. The original model is
`7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6`;
Xia is `871266c415d7500ff3cf3fe70fc6b36caecc4d4cbf09d4d737849e4060906433`.

Each variant uses three actual processes. The seed process buys its city and
ticks to **816**; a second fresh creator repeats those paid commands/ticks without
restoring its live World and compares the entire seed document literally. A
third process restores that seed. Both continuations advance exactly the requested
additional ticks and compare complete midpoint/final saves, raw snapshots and
observation reports: six literal pairs per variant. Every checkpoint also passes
the production SaveDocument parser and restore. Success witnesses come only from
`World::original_goal_status().reached`; there is no second goal formula.

The shared production starter supplies the legal origin **(102,120)**. A paid
reference World obtains that origin only; each measured variant starts with its
own fresh World and pays its own recipe. All construction, fields, operations and
priorities use normal validate/execute commands. Original data and placement
rules grant no extra construction or resources. Houses/providers cannot be
demolished under Rule10, so a smaller initial portfolio requires a fresh city.

The new observer reports actual payments, invoices attributed to their due
buildings and reconciled with the global ledger, actual pre-tax House inputs,
fire transitions, every phase39 evaluation's aggregate deficits, physical
deliveries/returns and applied visits. Its observation interval starts at816:
baseline Food misses are **five here**, versus **eleven including warmup**.
Seed-only `warmup.*.json` diagnostics reuse the existing paid-progression observer
to record real early route/arrival and supply evidence. They do not modify World.

## Exact costs and income

Upkeep is **2 Funds per installed required worker every400 placement-relative
ticks**, including paused, burning and unstaffed buildings. It is not a wage for
actually assigned workers. Priority and pause change allocation/production, not
this liability. Field/House/road bodies require zero workers.

The complete OE-34 supplied portfolio has:

| Building | Count | Required workers each | Combined invoice per400 |
|---|---:|---:|---:|
| Water Well72 | 2 | 4 | 16 |
| Ancestor Shrine | 2 | 4 | 16 |
| Market | 1 | 4 | 8 |
| Food Farm | 2 | 22 | 88 |
| Clay Source | 1 | 4 | 8 |
| Pottery | 1 | 6 | 12 |
| Warehouse | 1 | 2 | 4 |
| Hemp Farm | 2 | 18 | 72 |
| Acupuncture | 1 | 8 | 16 |
| Herbalist | 1 | 7 | 14 |
| Music School | 1 | 8 | 16 |
| **Total** | | **135** | **270** |

Well, Farm and named service requirements use the prepared Normal model.
Clay/Pottery/Warehouse/Market retain the authored requirements4/6/2/4; substituting
their raw original model employee values would change Rule10.

Tax is collected every **816 global ticks**, from the **pre-tick** occupied,
non-burning House, at actual occupants × prepared current-row TaxRate. It is not
a flat House fee. The Normal Common rows are:

| Current type | Model row capacity | Tax multiplier | Tax at that row capacity |
|---|---:|---:|---:|
| Shelter3 | 7 | 0 | 0 |
| Hut4 | 14 | 0 | 0 |
| Plain Cottage5 | 22 | 1 | 22 |
| Attractive Cottage6 | 31 | 1 | 31 |
| Spacious Dwelling7 | 41 | 1 | 41 |
| Elegant Dwelling8 | 52 | 1 | 52 |
| Ornate Apartment9 | 63 | 1 | 63 |
| Luxurious Apartment10 | 74 | 2 | 148 |

These capacities are not clamps on current population: Rule10 retains residents
after devolution. Diagnosis uses actual occupants and paid tax records, never
an invented population reduction.

The400/816 clocks align every **20400 ticks**:51 upkeep invoices and25 tax months.
For135 workers that is13770 upkeep, an average **550.8 per816 ticks**. The
observed mature six-House population220 at multiplier1 would yield only220 per
assessment even when unburning. The **frozen-state, fire-free arithmetic deficit
is330.8 per tax month**. This calculation does not simulate a no-fire city or
claim impossibility for all other stages/portfolios. Six hypothetical full
Luxurious Houses could yield888/month, but their required infrastructure and
actual progression are not demonstrated by that multiplication.

Month rows report actual tax/payment periods. The final partial tax month may
contain further upkeep, so sum the entire `invoices` collection or use
`summary.maintenance_delta` for a complete arbitrary interval. Later construction
is a separate cash outflow in `controls`, not part of the operating tax-minus-
upkeep balance.

## OE-34 reproduced and extended

The unchanged complete OE-34 workflow reproduced three separate processes, six
complete-state/report pairs and eight additional detailed diagnostic pairs in
`.local/original-economy/oe34/oe35-reproduction-01/`. All fourteen output files
also match the earlier canonical Release result byte for byte. The installation's
1464 files /819711091 bytes retain hashes, modes and mtimes.

| Actual checkpoint | Tick | Population | Funds | Cumulative tax | Cumulative upkeep |
|---|---:|---:|---:|---:|---:|
| First productive World goal | 816 | 108 | 9870 | 80 | 540 |
| Goal +20000 | 20816 | 220 | 290 | 4000 | 14040 |
| First negative Funds | 21600 | 220 | −30 | 4220 | 14580 |
| Goal +60000 | 60816 | 220 | −18130 | 12580 | 41040 |

Construction is1670. Over816→20816, actual tax3920 pays only29.04% of upkeep13500:
**−9580 operating balance**. Continued actual ticks verify21200 Funds20,
21216 Funds240, then21600 Funds−30. No cost or demand was reduced to obtain this.
Negative Funds are observed; the experiment continues rather than stopping or
granting money. Full negative-Funds save/restore continuation is also tested.

## Legal player variants

All positions below are offsets from the production starter origin. The first
four share the existing four-House starter plus two top-row Houses at(12,2) and
(15,2). The short connector is four paid roads at x2/y5–8. Every field explicitly
selects its purchased Farm.

| Variant | Ordinary recipe |
|---|---|
| `supplied` | Exact OE-34: two added Houses, connector, second Food Farm/Millet and second Hemp Farm/Field. |
| `direct` | Only the two extra Houses; original x20 connector remains. |
| `connected` | `direct` plus the four shortcut roads. |
| `food` | `connected` plus second Food Farm(9,10)/Millet(12,10). |
| `supplied12` | `supplied` plus six paid Houses at x0,3,6,9,12,15/y7, all at tick0. |
| `managed` | `supplied`; High priorities for Water/Ancestor/Food/Market; other producers/providers paused until1200, then resumed. Installed upkeep is unchanged. |
| `staged` | Starts `connected`; buys the second Food chain after a real recorded Food miss and the second Hemp chain only after an actual current-row Hemp shortage. |
| `staggered12` | `supplied`; three extra bottom-row Houses at816 and three at1632. Their natural risk clocks start on their actual purchase ticks. |
| `lean6` | Fresh72 roads, six top Houses, one Well(4,5), Ancestor(6,5), Market(9,5), Food Farm(0,10)/Millet(3,10). No Hemp, pottery, medicine or culture portfolio. |
| `lean12` | `lean6` plus six bottom-row Houses. |
| `compact6` | Same purchases/cost as `lean6`, with Well(9,0), Ancestor(12,0) and Market(4,10); Houses, Farm, field and roads are unchanged. |

This is a bounded player-strategy comparison, not an exhaustive optimizer.
More Houses increase both potential tax and actual competing Food/service demand.
Farm caps remain two Food and two Hemp Farms, one adjacent field each; no extra
production slots or free service coverage are introduced.

## Measured results

**All eleven variants reach the productive World goal at816 and remain
financially nonnegative for the following20000 ticks. None demonstrates a
self-financing economy.** All first-goal-plus60000 experiments,33 actual child
processes, complete in `release-all-03/` under the private OE35 root. Each of the
eleven has six byte-identical continuation pairs and an identical fresh-creator
reference. Later purchases are included in the final construction total.
The canonical `release-all-03/report.json` SHA256 is
`77fc6525762bd9df61ba9a18149bd2ab861b15f7db74a1e4b0ae882fcc64c6db`.

| Variant | Total construction | Final workers | Funds20816 | First negative tick | Funds60816 | Operating balance last20400 |
|---|---:|---:|---:|---:|---:|---:|
| supplied | 1670 | 135 | 290 | 21600 | −18130 | −9370 |
| direct | 1564 | 95 | 4176 | 36400 | −6627 | −5466 |
| connected | 1572 | 95 | 4352 | 37200 | −6328 | −5431 |
| food | 1626 | 117 | 2206 | 26800 | −12614 | −7534 |
| supplied12 | 2150 | 135 | 1980 | 26000 | −11801 | −6993 |
| managed | 1670 | 135 | 246 | 21200 | −18174 | −9370 |
| staged | 1670 | 135 | 486 | 22000 | −17934 | −9370 |
| staggered12 | 2150 | 135 | 1052 | 23600 | −13956 | −7611 |
| lean6 | 883 | 34 | 9751 | 170400* | 7329 | −1268 |
| lean12 | 1363 | 34 | 9669 | 245600* | 8193 | −784 |
| compact6 | 883 | 34 | 9711 | 173600* | 7333 | −1224 |

The trailing interval is actual ticks **(40416,60816]**, spanning20400 ticks and
therefore including both complete clock cycles. Its income/upkeep are reconciled
cash ledger events, not a nominal monthly projection. All eleven balances are
negative. `lean12` has the best observed trailing coverage,2684 tax/3468 upkeep
=77.39%, while still missing Food demands.

For the three starred rows, the cities remain positive at60816. Their first negative ticks were
then **actually simulated**, not extrapolated, in separate400000-additional-tick
three-process runs through400816:

```sh
python3 -B tools/test_economy_fire_diagnosis.py --binary "$PWD/.local/original-economy/oe35/build/openemperor-economy-fire-diagnosis" --data /absolute/path/to/Emperor --output "$PWD/.local/original-economy/oe35/lean12-runway" --variant lean12 --ticks 400000
```

Repeat with `lean6`/`compact6` and fresh output paths. These three long workflows
also pass six full continuation pairs each. At400816 Funds are respectively
−14435 (`lean6`),−6993 (`lean12`) and−14189 (`compact6`). The CLI's500000-tick
diagnostic ceiling is a private measurement bound; no productive World budget
or rule is increased. **First negative Funds is not permanent bankruptcy:** some
variants recover briefly at subsequent tax payments. Simulations continue and
record those later states rather than artificially ending at the first debt.

The experiments isolate several different effects:

- **Initial portfolio dominates capital reach.** Omitting initially unnecessary
  goods/medicine/culture buildings reduces installed workers135→34. These cities
  live much longer, but also remain lower-tier and under-supplied. Their remaining
  losses are actual losses, not a claim of sustained profit from lifetime tax.
- **Routes help without changing throughput rules.** The first real Farm→Market
  transfer is179 ticks in `direct`,69 in `connected`. The four roads cost8, and
  Food misses over816→60816 fall333→307. The cost-identical `compact6` relocation
  reduces the same first leg69→19 ticks, yet Food misses are310 versus307 in
  `lean6`; a faster leg alone does not prove better end-to-end supply or profit.
  These durations are observed loading/booking-to-positive-credit intervals in
  `seed/warmup.transport.json`, not path estimates.
- **Supply has a physical ceiling.** A staffed, operating Farm harvests one unit
  per80 productive ticks: maximum sustained average10.2 per816 ticks before
  fires, travel and full buffers. Six occupied Houses demand12 per816; twelve
  demand24. The two-Farm limit yields at most20.4 before those losses, so twelve
  occupied Houses cannot all receive their continuous Food demand from this
  fixed pair even with ideal travel. Burning Houses still consume. No third Farm
  or resource grant is invented to hide that constraint.
- **More residents do not translate linearly to tax.** `supplied12` ends with379
  residents and delays first debt to26000, but has633 Food misses versus five in
  `supplied`. Lost quality can devolve occupied Houses to zero-tax rows. Residents
  persist after devolution, so population is not proof of a stable tax base.
- **Priorities are operational tools.** `managed` eliminates the five post-seed
  Food misses, yet collects66 less tax over the first20000 ticks and ends with246
  Funds. Pausing delays some production and progression while preserving the
  installed bill. The regression separately compares real running/paused Farm
  Worlds through800 ticks: equal nonzero upkeep, positive versus zero harvest.
- **Buying on measured need delays liability.** `staged` purchases its second
  Food chain at816 after six actual warmup misses, and second Hemp chain at907
  after House3's current-row need1 observes stock0. Normal purchases spend54+44.
  Its first20000 additional ticks collect3920 tax against13464 upkeep; the
  initial cheaper portfolio and delayed invoice clocks leave486 Funds rather
  than290. Once installed, it has the same135-worker structural bill and the
  same−9370 trailing balance as `supplied`.
- **Cohorts can preserve the live goal.** `staggered12` has **zero goal-loss ticks
  in the first20000**, versus4800 for the all-at-once `supplied12`. Before its
  first negative Funds at23600 it never loses the live goal. Existing paid
  placement clocks distribute natural House fires without suppressing them;
  enough other occupied Houses remain eligible. It still has637 Food misses and
  a−7611 trailing balance. Missing protection therefore does not prove that
  every legal layout must lose the goal to fire, or that cohort scheduling solves
  finance and supply.

All supply counts in the preceding comparisons cover816→60816. A failed next-row
need blocks an upgrade; it is not automatically a missed current demand. Counts
of current unmet rows also depend on the reached stage and can include several
missing fields simultaneously. For example, `connected` improves Food deliveries
but reaches different rows and has more current-row failures than `direct`.
Transport `delivered_units` sums actual handoffs, including Farm→Market and
Market→House of the same unit. It is not unique production or House intake:
baseline1851 Food handoff units correspond to925 lifetime House receipts and883
consumed. Use the conserved inventories/counters for mass accounting.

## What fire actually does

`WorldFire.cpp` increments eligible, unprotected buildings' risk once every20
placement-relative ticks. At100 risk they burn for600 ticks. Pause does not stop
risk. Tick0 cohorts ignite at2000 and expire2600; after that their natural starts
repeat every2580 ticks, because risk can increment on the exact expiry boundary.
These are the existing deterministic rules, not suppression or injected incidents.

Fire has no destruction fee or direct inventory-loss debit. Its economic effects
are nevertheless substantial:

- Burning Houses do not pay assessed tax, admit immigrants or count toward the
  live goal. Their retained residents still supply the authored workforce.
- Burning producers/providers are unstaffed and stop production/new dispatch.
  Goods/Food reaching a burning destination cannot credit it and return normally.
  Service/Water arrivals grant visits only when the actual arrival is eligible;
  already dispatched visitors may finish after their source starts burning.
- Occupied Rule10 Houses **continue Food/goods consumption and evolution while
  burning**. Devolution can therefore follow indirect supply interruption. The
  older legacy statement about skipping burning-House demand does not describe
  Rule10.
- Installed upkeep continues. Through the baseline's first20000 extra ticks,
  **eleven actual invoices totaling2970** fall while its productive portfolio is
  burning:2400,4800,7200,7600,10000,12400,12800,15200,17600,18000,20400.
  An initial test expectation of twelve invoices was corrected against this actual
  trace; no gameplay value changed.

Tax is assessed before ignition and before expiry publication: ignition-tick tax
can pay, while an assessment at the exact expiry still sees the pre-tick burning
House. Actual per-House inputs and payment deltas are recorded at those boundaries.

The first20000-tick baseline has4800 fire-related lost-goal ticks. Its entire
60000-tick observation has44017 lost-goal ticks: this also includes39217 ticks
from the first negative treasury at21600 onward. Do not call that full count
fire-only loss. `goal_changes` records actual World witnesses; `fire_events`
separately records each real incident/expiry.

The observer also computes a clearly labeled **shadow**: occupants × rate at
actual burning-House tax inputs, which the fire predicate excluded. This is not
income and is never credited. Baseline shadow1266 during the first20000 ticks
would leave **−8314** after upkeep even if merely added arithmetically to the
actual3920 tax. It is not a no-fire counterfactual: removing fire would also change
production, delivery, stages and future tax inputs. No fire-free simulation is run.

## Why a FireWatch cannot simply be enabled

Rule10's explicit construction allowlist in `World.cpp` excludes `PlaceFireWatch`,
before ground/funds validation. This is a deliberate unsupported authority boundary.
The existing funded, legal-ground refusal regression preserves the complete
snapshot, IDs, Funds and command sequence.

Older profiles already have an80-Funds/2-worker/limit2 Watch and cargo-free
Inspector. Its cached shared routes prioritize reachable burning bodies, rotate
by stable ID, and grant extinguishing/risk reset/2400 protection only on actual
arrival. That machinery could inform a new contract, but Rule10 restore also
excludes Watch paid costs, its owner/courier bijection and the FireInspector role.
Original finance history currently assumes only natural fire deadlines. Inspector
dynamic-route limits also retain older40/38-body bounds, versus82 Original bodies.
Adding one allowlist entry would leave incompatible accounting/restore authority.

The original model's Inspector's Tower124 name/art is not proof of functional
mapping; its raw16/5 cost/workers differ from the authored legacy80/2. Existing
[model provenance](model-provenance.md) marks functional FireWatch mapping unsupported.
Neither number set is silently chosen here.

## OE-36 decisions supported by this diagnosis

1. Specify a separately selected, versioned paid fire-protection contract: functional
   source identity and evidence, geometry/admission, cost/workers, owner/courier IDs,
   actual-arrival effects, operation controls, bounded route/cache limits, fire/tax
   history and atomic restore. Preserve Rule10 and all old profiles/saves.
2. Decide explicitly whether a future contract retains consumption/evolution while
   burning. OE-35 documents that behavior rather than silently fixing it.
3. Re-measure legitimate protection with its real price and staff. Fire protection
   alone does not resolve the demonstrated135-worker/220-rate1-resident deficit.
4. Treat any requested maintenance, tax, recipe, demand, labor or Farm-cap changes
   as a separate balance contract with regression evidence. No such value is changed
   or approved by these experiments.
5. Distinguish finite solvency and staggered-cohort goal retention from a steady
   self-financing city. A tested losing portfolio does not prove that every legal
   higher-tier or more complex strategy must lose.

## Technical acceptance and limits

**Technical diagnosis: PASS. Profitable/sustainable city found: NO in the measured
strategies.** The causes are reproducible without changing production code or
economic rules: installed staffing liability exceeds the tested tax base; Food
capacity/distribution limits higher demand; synchronous fires temporarily remove
the OE-34 Houses from eligibility, interrupt supply and tax while upkeep continues.
Funded FireWatch construction is deliberately unsupported, not a placement-price
or navigator defect. No newly demonstrated production-code bug is asserted here.

| Executed checks | Final result |
|---|---|
| Complete Release CTests | **250/250 PASS**, zero skips |
| Relevant Debug CTests | **46/46 distinct tests PASS**, zero skips |
| Same ASan/UBSan selection, RelWithDebInfo | **46/46 PASS**, zero skips/findings |
| New runner guards | **34/34 PASS**, including false/missing/mistyped first-goal witnesses, fake restores, corrupt ledgers, timeout and byte mutation |
| All eleven60000-tick variants, Release | 33 real processes,66 identical full continuation pairs |
| Three400000-tick capital runs, Release | Nine real processes,18 identical continuation pairs |
| Staged/staggered12/compact6,20000 ticks in Release/Debug/ASan | Nine workflows,27 real processes,54 identical continuation pairs |
| Same representative cases across configurations | **54/54** seed/midpoint/final save/snapshot/report pairs byte-identical |

`original-economy-fire-diagnosis-real` reproduces equal nonzero upkeep for paid
running/paused Farms while their actual harvests differ; the live816 goal,
atomic FireWatch refusal, natural-fire losses, real21600 debt and complete
negative-Funds restore continuation. The other44 selected tests cover existing
paid starters, rule gates, scheduler, housing, finance, production, Food/Hemp,
services/religion, adversarial saves and full-world Longplay replay. The new
first-goal guard consumes the true World descriptor bound to its tick; it does
not recompute housing thresholds.

Run the new regressions after building the target above:

```sh
ctest --test-dir .local/original-economy/oe35/build --output-on-failure -R '^original-economy-fire-diagnosis'
```

The existing private OE34 build trees were reused for execution, preserving
earlier output directories. Release evidence is
`.local/original-economy/oe34/oe35-release-final-ctest.{log,xml}` and the final
runner-only JUnit `oe35-release-final-runner.xml`. Focused46-test logs and final
corrected-regression/34-guard JUnits live under `.local/original-economy/oe35/`;
`focused-validation-summary.json` records which final result supersedes the
initial expectation-only failure. `representative-process-workflows.json` and
`cross-config-comparison.json` retain executable identities and pair hashes.
Sanitizers use `OPENEMPEROR_ENABLE_SANITIZERS=ON`, `-O2 -g`, address+undefined,
`ASAN_OPTIONS=detect_leaks=0:halt_on_error=1` and
`UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`, matching the maintained alpha
pipeline. No sanitizer issue was observed.

NOT_RUN: all eleven long variants and the400000-tick runs under Debug/sanitizers;
other maps/difficulties/upper-tier strategies; an exhaustive profitability search;
native human play, new Sandbox rendering captures, packaging/Gatekeeper and
original-game parity. The private historical OE00 reference-save corpus was not
recreated/rerun; its locks remain unchanged, with the executed historical CTests
providing the scoped compatibility evidence. None of these unexecuted checks is
silently counted as technical, visual or human acceptance. No original bytes,
generated saves or diagnostic assets are committed.
