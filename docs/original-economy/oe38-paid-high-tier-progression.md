# Paid high-tier progression

This investigation starts from `main`/`origin/main`
`7ca37d1c661adf88fe6042eb1093b9e720892448`, on
`feat/oe38-paid-high-tier-progression`. The supplied root task document is
preserved separately; it is not a runtime contract or a committed fixture.

Rule12 / Schema28 / Policy3 / development revision27 and all historical
generations remain unchanged. Diagnostics and player strategies use production
World commands, immutable prepared model data and the shared navigator. Original
files, complete model tables, saves, reports and captures stay outside Git.

## Baseline and model gate

The fresh `balanced-three` reproduction uses real `Cities/Xia.map`, the pinned
unchanged original Normal model and the existing paid twelve-House recipe.
The source model SHA256 remains
`7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6`.

At maturity tick20000 the city has318 residents and11785 Funds. After100000
additional ticks it has21379 Funds and still has at least twelve occupied
non-burning Houses and318 residents. Four consecutive complete20400-tick
windows each collect7950 tax, pay6000 upkeep and earn1950 Funds. Construction
costs1869 and is booked separately; no capital or supplies are granted.
Post-maturity current unmet evaluations, Food misses, goal losses, devolutions
and fires are zero. Startup deficiencies remain in the unchanged warmup.

Three fresh processes perform creation, earned-state restoration and actual
SandboxView rendering. Complete seed/midpoint/final saves, all continuation
metrics and warmup are byte-identical to OE37's accepted Release case. New
1×/2×/4× captures are likewise byte-identical and were opened and inspected.
The existing OE37 evidence tree, original inputs and supplied task retain their
bytes/modes/mtimes. A private output-path-only QA adaptation initially permits
new evidence under `.local/original-economy/oe38/`; it links the accepted
unchanged Release World/persistence archives. This is not a rule change.

The maintained OE38 executable reuses that same baseline protocol with an
explicit compile-time evidence directory. The original OE37 executable retains
its exact default directory and checks. No second simulation or parser is added.

The machine-readable model report uses `World::original_housing_diagnosis()`
and `original_housing_decision_matrix()` on paid Common and Elite observers.
It independently loads the same source through the existing bounded ModelData
reader and cross-checks against separately prepared `HouseEvolutionRules`.
Queries must leave the complete World unchanged. It includes every current/next
requirement, nine individual timers, alternative goods/religion predicates,
capacity, entry/retention thresholds, legal source chains and cost/workforce
estimates. Full numerical source tables are generated privately, not committed.

| Gate | Entry condition from the actual predecessor | Retention at target | Required physical additions beyond the baseline |
|---|---|---|---|
| Common10 | Ornate9 DES≥40 and current/next needs met | Luxurious10 DES>35 and current needs | Herbalist, Music, Acupuncture, Acrobat, Ceramics and Tea chains, Daoist OR Buddhist; actual DES packing |
| Elite17 | Impressive16 DES≥80 and current/next needs met | Heavenly17 DES>75 and current needs | Separate paid Elite plot/materialization, Silk, Bronze OR Lacquer, Paper-fed Confucian, Drama and the other required supplies; actual DES packing |

Food thresholds50/70 fit the existing physical Millet score80. Required goods
thresholds1 fit the existing per-good8 capacity. This rules out those simple
numeric-cap objections; it does not prove transport, staffing, finance or legal
DES packing. Common2–10 and Elite11–17 remain separate. Paid Elite11 enters13
only on real first admission with atomic validated4×4 materialization. Type12
is an actual devolution state, never an injected starting residence.

Actual evaluation witnesses precede evolution and consumption. Post-tick World
diagnosis describes the resulting live state and is labeled separately. A pure
or hypothetical row never counts as an actual high-tier event.

## Reproduction and evidence

Build the registered `openemperor-high-tier-progression-acceptance` target with
the setup in [testing](../testing.md), then use new private output directories:

```sh
python3 -B tools/test_high_tier_progression.py --stage baseline --binary "$PWD/build/openemperor-high-tier-progression-acceptance" --data /absolute/path/to/Emperor --resources "$PWD/build/resources" --output "$PWD/.local/original-economy/oe38/baseline-new"
python3 -B tools/test_high_tier_progression.py --stage matrix --binary "$PWD/build/openemperor-high-tier-progression-acceptance" --data /absolute/path/to/Emperor --input "$PWD/.local/original-economy/oe38/baseline-new/create/seed.save.json" --output "$PWD/.local/original-economy/oe38/matrix-new"
```

Initial immutable evidence is `stage0-baseline-result.json`,
`baseline-release-01/acceptance.json`, `baseline-frozen/compile-manifest.json`
and the inspected `baseline-release-01/render/` captures, all under ignored
`.local/original-economy/oe38/`. The matrix has an independent complete
schema28 saved World; its additional empty Elite plot is ordinarily purchased
for80 Funds at the saved tick and receives no admission or resources.

The upper-tier gameplay gates and their retention remain unproven until the
subsequent paid scenarios produce actual first-success events and continuations.
Numerical/technical checks and the successful OE37 baseline do not substitute
for those gates. Original parity remains PARTIAL because immigration, workforce,
recipes, targeted School visits, Paper dispatch units and taxation are authored
adapters; complete original lifecycle and venue/performer mechanics remain open.

Stage A technical acceptance passes19 relevant Release CTests, including the
registered100k regression, and10 each in Debug and ASan/UBSan (eight targeted
authority/housing/Elite/save cases plus two matrix cases). All run with the real
Normal model where registered; authored-geography controls are labeled. The
runner has15 hostile metadata controls, including binding actual House states,
families, geometry and every claimed first transition to the complete saved
World. A diagnostic table alone cannot claim a reached tier. The final baseline
and matrix workflows retain complete original/source/executable/input identity.

Current Stage A artifacts: `stage-a-baseline-release-03/acceptance.json`,
`matrix-release-03/matrix-acceptance.json`, `stage-a-release.xml`,
`stage-a-release-runner-final.xml`, `authority-stage-a-{debug,sanitize}.junit.xml`
and `stage-a-{debug,sanitize}-matrix.xml`, under the private OE38 directory.
The100k workflow is rerun in Release; an additional OE38 Debug/sanitizer100k
workflow, native delivery and human play are NOT_RUN at this stage. Existing
OE37 Debug/sanitizer evidence is historical evidence, not a new run.
