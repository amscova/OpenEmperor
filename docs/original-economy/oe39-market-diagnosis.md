# Paid Common10 goods distribution diagnosis

Gate A: **PASS for the reproduced cause**, on baseline
`81bc9218958f7ade116fc43d3df80975c1206e67`. Continuous Common10 supply remains
**PARTIAL in all six paid Rule12 variants**. Gate A changes test instrumentation
and player recipes only; no production rule, model value, historical schema or
save semantics change. Gate B and C are separate acceptance decisions.

The source is the unchanged supplied `Cities/Xia.map`, Normal difficulty and
model SHA-256 `7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6`.
All cases use City-v16 Rule12/Policy3/Schema28/revision27, the actual paid OE37
`balanced-three` city at tick20000, then ordinary World commands for the OE38
expansion. The Common target remains purchased House#3. No snapshots, stocks,
residents, timers, fire state or money are assigned by a controller.

## Reproduced results

Each row simulates20400 genuine ticks after the exact first9→10 evaluation and
observes400 actual phase39 inputs. Initial city construction is1869 Funds;
the expansion costs below are additional, entirely paid at tick20000. Cashflow
is actual collected taxes minus all installed-worker upkeep during the entire
retention window; construction within these windows is zero.

| Paid variant | Expansion | First10 tick / Funds | End tick / type / Funds | Tax−upkeep | Devolutions / ticks below10 | Current unmet: Hemp / Ceramics / any |
|---|---:|---|---|---|---|---|
| `common-reserved` |719|27733 /11431|48133 /10 /15136|13205−9500 =+3705|18 /9537|194 /29 /194|
| `common-near-pottery` |719|23857 /11355|44257 /10 /15264|13409−9500 =+3909|16 /8772|153 /69 /167|
| `common-near-chain` |727|23959 /11341|44359 /10 /15318|13477−9500 =+3977|9 /4386|101 /35 /102|
| `common-short-route` |727|30997 /11822|51397 /10 /15603|13281−9500 =+3781|14 /9894|113 /47 /145|
| `common-two-market` |861|22633 /10968|43033 /8 /13956|12688−9700 =+2988|18 /14026|122 /105 /195|
| `common-priority` |719|27733 /11431|48133 /10 /15136|13205−9500 =+3705|18 /9537|194 /29 /194|

The near-Pottery case moves the paid Pottery from relative(22,2) to(18,2).
The near-chain case also moves Clay from(0,0) to(18,0) and buys four roads at
(17,0)..(17,3), costing8 extra. The short-route case buys four roads at
(8,5)..(8,8), costing8 extra. The second Market and its road at(17,7)/(17,8)
cost142 extra. The priority case executes normal zero-price High workforce
priority commands for the existing Warehouse and Market. Prices, bodies,
selected IDs, Funds and monotone command sequences are independently checked.

Target burning ticks are65,0,230,15,315,65 respectively; overlap with lower
types is retained rather than double-counted. Minimum actual Funds are
11431,11215,11201,11822,10968,11431. All six productive World goals remain
reached throughout their windows. An endpoint of type10, a positive treasury
or a positive operating window does not satisfy continuous supply acceptance.
These finite comparisons do not establish that every legal Rule12 plan fails.

## First failure: physical transfer chain

The reserved city has40 paid bodies,24 couriers and12 occupied Houses at
entry. Its actual9→10 witness at27733 has47 target residents,11431 Funds and
2588 total construction. The following events come from actual loaded trips,
ledger arrivals and scheduled consumption, not expected visits:

| Tick | Actual event |
|---:|---|
|27496→27515|Market#15 distributor#3, sequence228, loads4 Hemp and delivers them to House#3 over4 edges.|
|27536→27555|The same actor, sequence229, delivers4 Ceramics to House#3 over4 edges.|
|27784,28141,28600,28957|House#3 actually consumes1 Hemp and1 Ceramics at each boundary. Its stocks reach0/0 after28957. The28957 phase39 input still contains1/1 and passes before consumption.|
|28876→28980|Distributor#3, sequence244, carries4 Hemp to House#10,21 edges/105 movement ticks one-way. It must physically return.|
|29005→29059|Warehouse#18 supplier#6, sequence241, carries4 Hemp to Market#15 over11 edges. This is a real outbound shipment, not a future allocation.|
|29008|First failed House#3 phase39 input: Hemp0, Ceramics0, both require1. DES40, FoodQuality80, water and other current needs pass.|
|29085→29086|Distributor#3 arrives home, then sequence245 instead sends4 Hemp to House#11 over24 edges.|
|29326→29435|Sequence246 sends4 Hemp to House#12 over22 edges. House#3 receives neither missing good.|
|29467|The tenth consecutive failed phase39 input, with counters0..9 at51-tick intervals, causes actual10→9.|
|29716→29735|On its next House#3 visit the distributor selects Ceramics, sequence249. Market Hemp is0 when this trip loads.|
|29719|Supplier#6, sequence247, replenishes4 Market Hemp, three ticks after that dispatch. This cannot change its already loaded Ceramics trip.|
|29756→29775|The distributor sends Tea to House#3, sequence250; the normal saved cursor then continues to other households.|
|32290→32309|Sequence271 finally delivers the next4 Hemp to House#3:4794 ticks after its preceding Hemp arrival.|

At29008 Market#15 physically has Hemp4/Ceramics3/Tea11 and4 Hemp incoming;
it is staffed4/4, operating and not burning. Distributor#3 is returning from
House#10 at vertex5 of a22-vertex path, edge progress3. House#3 has no
incoming Hemp/Ceramics reservation. Neither dispatch nor a stocked Market
credits its inventory.

Warehouse#18 has28 Hemp and **4 outgoing Hemp cargo reserving source-return
room**, with zero incoming Hemp. Its effective occupancy is32 and headroom0;
the four units are supplier#6's actual sequence241 shipment. This corrects
OE38's earlier description of those four units as incoming, without changing
any measured historical result. Farm#21 holds6 Hemp and Farm#25 holds8;
their own couriers#9/#11 are idle with `Target buffer full`. The Warehouse
return-space reservation is necessary cargo safety, not lost mass or a stale
incoming reservation. At this tick the conserved global Hemp equation is
488 produced =420 consumed +64 stock +4 cargo, with no imports or demolition.

Every tick27734..29467 was inspected for the target, Market, Warehouse,
both Hemp Farms, Clay#37, Pottery#38 and Tea#39: no fire, pause or staffing
deficit occurs in this first-failure interval. The target's eventual fire is
45220..45285. Fire interrupts later logistics but does not explain this first
devolution. Current shortages also affect Houses#2/#4/#5 at29008; other
occupied homes are included rather than replacing city-wide evidence with a
favoured target ID.

## Throughput, stock and hypotheses

One road edge requires5 movement ticks. Dispatch and the first movement
increment occur in the same tick:4-edge delivery has19 elapsed ticks from
dispatch to arrival,40 movement ticks per complete trip, and a following
dispatch can occur40 ticks after the preceding one.

At29008 the distributor needs77 more movement ticks to reach home. Ignoring
its historical selection policy, its earliest next departure is29086 and an
optimistic direct House#3 arrival is29105 over the cached4-edge route. This is
a physical lower bound, **not an actual dispatch**. The observed House#11 and
#12 trips consume240 and220 movement ticks. Both road time and commodity/
target ordering therefore matter. Hemp becoming available three ticks after
a cursor-selected Ceramics departure is a second concrete scheduling effect.

Warehouse→Market takes11 edges in each direction: minimum departure period110
ticks and at most `4×816/110 =29.67` units per816-tick month across all goods.
Its supplier performs181 new trips in the window, loading523 units;527 arrive
including4 already in flight at entry. Their nominal round-trip movement work
is19910 ticks. Partial payloads matter:70 Ceramics trips load147 units and31
Tea trips load56, leaving capacity unused even while the supplier is busy.

The Market distributor makes237 new trips loading503 units;501 arrive during
the window, over routes ranging2..24 edges. Those trips require16340 nominal
movement ticks. Actual House arrivals and sinks reconcile as follows:

| Good | Arrived at Houses | Actually consumed | Combined House stock, entry→end |
|---|---:|---:|---|
|Hemp|324|297|24→51|
|Ceramics|133|87|50→96|
|Tea|44|28|80→96|

Scheduled House requests are365 Hemp,91 Ceramics and28 Tea; an unmet request
does not become consumption. Hemp harvest312 is below the two-Farm nominal
maximum510 and global Hemp stock+cargo grows74→89. Ceramics conversion164
exceeds consumption87 and global stock+cargo grows52→129. A raw-material
shortage is not the cause of the first empty House inventory. Clay hauling
nevertheless imposes a separate upstream limit:25 edges/125 movement ticks
one-way,4 Clay per250-tick round trip, approximately6.53 possible Ceramics
per month after the existing two-Clay recipe. Moving production materially
improves some results, but does not prove a distribution cure.

An optimistic calculation batching the **observed** requests per House/good
into full four-unit trips along the existing cached routes totals10790 Market
movement ticks, below20400. It ignores release/deadline timing, fire, initial
inventory, evolving demand and partial loads. It is not a feasibility proof,
but these observations do **not** establish that extra couriers, larger cargo
or higher Farm caps are necessary. No capacity increase is authorized by
this calculation.

The original pre-tick samples reproduce OE38 exactly: Warehouse Hemp≥28 in
392/400 and Market Hemp0 in239/400. New post-tick samples give390/400 and
242/400 respectively because same-tick loading, arrival and consumption have
already happened. These are explicitly different phases, not divergent runs.

Across all twelve Houses there are673 current-need failures and62 actual
devolutions; eleven Houses experience missing Hemp during the window. House#7
retains Hemp8 at29008 while its current type5 has no Hemp requirement. Next
stage stocking is a legitimate policy question, but it must not be mistaken
for meeting another household's current requirement.

The six legal comparisons reject an immediate claim that shorter Clay routes,
one additional connected Market, or workforce priority alone fixes retention.
Priority has the identical measured outcome because logistics already has full
workers. They also reject production-cap inflation as an explanation of the
first failure. They leave the coupled Warehouse/Market selection policy as a
measurable intervention hypothesis: select physically reachable current needs
before usable next-stage demands, account for actual incoming commitments,
retain deterministic fairness, and preserve existing actors and inventory
authority. This is proposed authored behavior, not evidence of original
Emperor dispatch. Gate A implements no such policy and introduces no Rule13.

## Reproduction and technical evidence

Build the existing `openemperor-market-distribution-acceptance` target with the
supplied original-model test root. With a fresh output path:

```sh
python3 -B tools/test_market_distribution.py --binary /absolute/build/openemperor-market-distribution-acceptance --data /absolute/Emperor --output "$PWD/.local/original-economy/oe39/reproduction-new" --variant common-reserved
```

Use each other table variant with its own fresh directory; those comparisons
may use `--no-replay`, which explicitly records separate-process validation as
NOT_RUN. Every case still performs strict complete save/readback and Recovery.
The guarded runner checks original bytes/modes/mtimes, source inventory and
executable identity. All six actual workflows preserve those inputs.

Canonical reserved evidence is private
`.local/original-economy/oe39/gate-a-reserved-release-02/market-acceptance.json`:
PASS, five independent restore processes from first-success, active-cargo,
before-shortage, first-shortage and first-devolution. Every process produces
the identical complete final SaveDocument and snapshot; first-success replay
also reproduces retention and transport traces literally. Eight historical
OE38 artifact pairs are identical. Recovery start/end checkpoints match their
complete saves. Earlier reserved01 and canonical02 traces/reports are also
byte-identical. Raw traces, saves, binaries and the private cause-chain report
remain under ignored `.local/`.

Release focused CTests:6/6 PASS. New runner guards:27/27 PASS; existing high-tier
guards:48/48 PASS. ASan+UBSan focused CTests:6/6 PASS with no reported sanitizer
finding; macOS LeakSanitizer is NOT_RUN. Initial Debug CTests finish5/6 PASS:
`original-market-transport` reaches its existing60-second CTest timeout;
the real Common supply regression passes in276.826 seconds. With the diagnostic
timeout raised to300 seconds, the unchanged transport test passes in59.91
seconds (`gate-a-debug-transport.xml`), completing all six Debug checks. The
initial timeout remains recorded; no simulation value was changed. Native/human
playtest, OE39 Sandbox captures and full Debug/Sanitizer paid workflows are
NOT_RUN at this Gate A checkpoint. Original runtime parity remains PARTIAL.
