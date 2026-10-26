# Original economy: architecture and contracts

The optional **City-v16 Rule10 / schema26 / policy3 / development revision27**
has paid production, transport, House evolution, taxes and recovery. Full original
parity remains partial; see [the play guide](player-handoff.md) and [Known Issues](../../KNOWN_ISSUES.md).

## Versions and source preparation

| Generation | Saved contract and scope |
|---|---|
| Rule7 / schema23 | Frozen original House state, immutable model binding and scheduler; no active economy. |
| Rule8 / schema24 | Frozen paid Well72 desirability sources; no water, population or production. |
| Rule9 / schema25 | Frozen neutral six-goods ledger; real World goods remain unsupported/zero. |
| Rule10 / schema26 / revision27 | Current explicit paid economy; incompatible development revisions are refused. |
| Rule11 / schema27 / revision27 | Separate explicit paid economy with authored FireWatch and actual Inspector visits; bounded fire/tax provenance. |
| Rule12 / schema28 / revision27 | Separate explicit sustainable-economy contract: three paid Food Farm/Field chains and installed-worker upkeep every 816 placement-relative ticks; retains paid Rule11 fire protection. |

Rules1–9 and schemas1–25 retain their identities and canonical bytes. City-v11
Rule3 and City-v16 Rule4 defaults are unchanged; no automatic migration opts older
saves into Original economy. Bridge/Ramp infrastructure retains its existing contracts.
Rule11 inherits the unchanged Rule10 economic values and uses its own fire guard;
Rule10 still rejects FireWatch. The [fire protection contract](oe36-fire-protection.md)
defines its authored building, actual arrivals and separate schema27 validation.
Rule12 has a separate create/restore and menu/CLI choice. It preserves original
model values, taxes, harvest periods, needs, immigration, goods, services and fire
mechanics. Only its authored Food count and upkeep interval differ; see the
[sustainable-economy contract](oe37-sustainable-economy.md). No save is migrated.

`ModelData` reads the selected installation's `Model/EmperorBuildingModels.txt`
within its canonical root: regular file, at most 256 KiB, lines at most 4096 bytes,
unique Building IDs and complete 13-field Building/24-field House grammar.
`OriginalRulesModel` prepares all 15 House rows and admitted named providers for
one Difficulty. Building values use checked signed `base * modifier / 100`,
truncating toward zero; House values use checked `clamp(base + modifier,-99,100)`.
The [model register](model-provenance.md) binds labels, units and evidence.

Create/restore receives a `shared_ptr<const OriginalRulesModel>`. Worlds never
search for files. Saved source identity includes format, SHA256, byte count and
Difficulty; prepared rules/tables and runtime pointers are not serialized.
F5 checks the actual file identity before atomic replacement; F9 prepares and
validates a new candidate before publication. Missing or changed sources reject
I/O without changing the live city or prior save. Frames and ticks perform no
model reading, hashing, preparation, asset loading or navigation preparation.

## Clock and update order

World ticks are the only saved clock. Mapping version 1 assigns one scheduler
phase per existing 20 Hz tick; this speed and calendar integration are authored.
For completed ticks `t`, the next dispatch is:

```text
phase = t % 51
step  = (t / 51) % 16
month = (t / 816) % 12
year  = t / 9792
```

Phase 35 decrements service timers; phase 39 evaluates evolution, followed by
six-goods and Food consumption only at step 0 or 7. First consumption completes
at ticks 40 and 397. A real arrival earlier in that tick can supply the evaluation
and consumption; the old 400-tick demand path does not run as a second authority.
Taxes use the pre-tick occupied/non-burning state every 816 ticks, before upkeep,
production, actual movement, fire, immigration and House dispatch.

The original static evidence is narrower than this integration. On the pinned
EXE recorded by [the model contract](../reverse/original-model-contract.md),
`0x4ac2e4..0x4ac2f9` and `0x4ac4c8..0x4ac4e7` establish phases 0..50;
`0x4ac4e8` targets `0x4ac45e`/`0x4ac48f` for phases 35/39.
`0x519025..0x51905b` gates post-evolution consumption on selector0/7;
`0x4ac650..0x4ac67e`, `0x4ac6b8..0x4ac6c9`, `0x4ac794..0x4ac7b4` establish
16 passes and a 12-value outer counter. The original year increment at
`0x4ac7ce..0x4ac7e9` uses outer value1, unlike the elapsed-year counter above.
These read-only findings do not establish original real-time speed or all
scheduler work. No original executable is run by this implementation.

## Paid sources, inventories and actual arrivals

The active bounds are 82 bodies, 58 couriers and 20 Houses. All bodies, fields and
plants are purchased through normal commands. Population belongs solely to
`OriginalHouseState`; old `Building.population` remains zero. Authored immigration
starts no earlier than 120 ticks after purchase and then admits up to two people
every 40 ticks, bounded by prepared capacity and an actual safe road entrance.
Residents provide deterministically allocated workers, including Elite residents;
this is an explicit adapter rather than recovered original labor policy.

Six House goods are Hemp, Ceramics, Tea, Silk, Bronzeware and Lacquerware.
HouseState is their sole House inventory; the typed ledger owns non-House stock,
cargo, reservations and historical counters. Pottery becomes Ceramics exactly
once on successful Warehouse arrival; the old Warehouse Pottery slot stays zero.
For every good, checked conservation requires:

```text
produced + imported = consumed + demolished + stock + cargo
```

Reservations commit source stock and destination/return space; they add no mass.
Only actual loading creates cargo and actual arrival credits the destination.
Transfer handles bind CourierId to a monotonic sequence; stale or duplicate
arrivals fail. Pause/disconnect preserves cargo and begun-edge safety, using the
same navigator for return and paid road repair. Goods/target rotation is bounded;
finite visit tests do not prove universal starvation freedom.

Recipes and 80-productive-tick harvest periods are authored. Every required input
is debited atomically with output credit: Wood+internal Ore→Bronze;
Bronze+Clay→Bronzeware; Lacquer+Wood→Lacquerware; RawSilk→Silk; Hemp→Paper.
Plants are separately purchased and owned. Wood/Bronze/RawSilk/Lacquer/Paper are
five auxiliary materials, not extra House goods. Exact source evidence is in
[the luxury contract](../reverse/original-luxury-source-contract.md).

Food has separate Wheat/Millet quantities, cargo and arrival witnesses. Authored
crop scores 20/80 feed the original quality blend only on positive House delivery.
Rules10/11 admit two paid Food Farms with one uniquely owned adjacent Field
each; Rule12 admits three within the unchanged global body/courier bounds.
Stock depletion resets quality; an authored sink consumes one unit per occupied
House at 0/7 boundaries and records real misses. Six-goods consumption uses the
current model row independently per good: Bronze/Lacquer satisfy needs as OR,
but both stocks are consumed. See [goods evidence](../reverse/original-goods-consumption-food-contract.md).

Nine services use paid 207/208/211/212/213/214/215/217/219 providers, real workers,
operation/fire/road gates and actual walker arrival. Medicine/culture grant only
their corresponding 96 timer; religions their own 40 timer. Availability or a
future trip grants nothing. Confucian 219 serves occupied Elite Houses and requires
real Paper delivery, consuming one Paper per new successful dispatch. Targeted
School→House visits and this Paper policy are authored; original Performer/Venue
chains remain incomplete. See [spawn](../reverse/original-service-spawn-contract.md),
[culture](../reverse/original-culture-source-contract.md) and
[religion](../reverse/original-religion-source-contract.md) evidence.

## Houses, geometry, finance and presentation

Common types 2–10 remain 2×2 and terminal at 10; Elite 11–17 are a separate family.
The paid 80-Funds Elite plot starts empty 11/2×2. At its first actual admission,
all 16 future claims, equal signed height, original protection, infrastructure,
road entrance and begun actor edges must pass before atomic 4×4/11→13 publication.
A conflict leaves the plot empty for a later normal attempt. Identity, stock,
paths and history stay intact; later 13→12 devolution never shrinks the body.
No Common→Elite conversion is inferred. [Lifecycle evidence](../reverse/original-house-lifecycle-contract.md)
distinguishes original family geometry from this authored placement transaction.

Evolution uses actual DES, water, quality, goods and nine timers at phase 39.
Prepared DES sources are paid Well72 and admitted service bodies in BuildingId
order; build/restore or permitted source removal rebuilds the cache, never frames
or ticks. Original wall blocking and full source-manager lifecycle remain open.
[Source evidence](../reverse/original-desirability-source-admission.md) does not
grant construction permission by analogy. Production/House demolition is blocked;
only neutral Well72 influence sources have the existing no-refund removal path.

Tax is authored direct collection of pre-tick `occupants * prepared TaxRate`
every 816 ticks, without an invented TaxOffice. Upkeep is 2 Funds per installed
required worker every 400 placement-relative ticks in Rules10/11 and every 816
in Rule12, including pause, fire and unstaffing.
Cumulative per-type taxable units and actual arrival/transition witnesses constrain
restore; they are bounded proofs rather than full event journals. Existing Funds
and accounting counters are the sole monetary authority. Purchases need available
Funds; real upkeep may cause debt. Overflow is checked before tick mutation.
The authored live goal needs six occupied non-burning Houses, one current Common
PlainCottage 5+, 50 residents, actual positive tax and nonnegative Funds.
[Tax evidence](../reverse/original-house-tax-contract.md) does not establish full
original taxation or original campaign goals.

Original House art uses verified type/body-compatible sprites for types 3–17;
empty 2×2 Elite plots retain named fallbacks. Eager preparation validates source,
layout, anchors and alpha within existing budgets. Drawing/picking share prepared
resolution and actual signed-height body; no graphical field enters a save.
Other buildings/walkers retain fallbacks; [image evidence](../reverse/original-house-visuals.md)
is separate from native human acceptance.

## Save safety and developer checks

Schemas26–28 validate complete identities, map/mask/policy/model binding, paid origin,
geometry, route/progress, capacities, conservation, timing and finance before
publication. Unknown/duplicate/missing/mistyped fields, incompatible revisions,
unsupported authority and impossible histories fail atomically; 8 MiB document and
32-level nesting limits remain. Recovery requires explicit OriginalProduction
access: a protected start and up to five 1200-tick checkpoints; opening one forks
history while preserving the source. LegacyOnly remains the API default.

See [testing.md](../testing.md) for build/sanitizer setup. With a configured `build` and your own data:

```sh
ctest --test-dir build --output-on-failure -R '^original-'
cmake --build build --parallel 2 --target openemperor-original-longplay-acceptance
python3 tools/test_original_longplay.py --binary build/openemperor-original-longplay-acceptance --data /path/to/game --output .local/original-economy/oe32/longplay-check --scenario starter --ticks 20000
```

The [paid progression check](paid-progression.md) uses the same runner with
`--scenario progression` and a new output under `.local/original-economy/oe34/`.
It waits for the actual World goal, then measures at least 20000 additional ticks;
goal acceptance and long-term operating stability are reported separately.

`sh tools/play_original_economy.sh --help` documents the empty developer session;
the play guide covers the menu starter. Dedicated runners remain under
`tools/test_original_*.py`; [core comparison](core-parity.md) covers pinned fork checks.
`tools/original_economy_baseline.py verify` needs the immutable [baseline lock](baseline-lock.json)
and private reference; never regenerate hashes to conceal legacy changes.
Reports belong under ignored `.local/`. Missing data, synthetic controls, software
screenshots, native delivery and human acceptance retain separate statuses.
