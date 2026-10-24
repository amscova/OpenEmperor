# Sustainable original economy

Rule12 is a separate, explicit **City-v16 / Policy3 / Schema28 / revision27**
contract. It retains the paid Rule11 FireWatch and actual Inspector arrivals.
Original model values remain source facts; the capacity and invoice changes
below are authored OpenEmperor balance decisions, not recovered Emperor rules.

## Baseline and demonstrated constraints

The starting worktree was clean. After fetching origin, local `main`,
`origin/main` and the completed OE36 branch all pointed to
`231cf76af871bc27259961dea2c73aa45941bc13`; no unpushed branch commits remained.
Existing local OE34/OE35/OE36 and original-economy branches were preserved.
OE37 starts from that technically accepted paid fire-protection implementation
on `feat/oe37-sustainable-economy`.

OE34, the six required OE35 recipes and the paired Rule10/11 OE36 case were
rerun against frozen baseline executables. Each uses real `Cities/Xia.map`,
Normal, the unchanged original model and production World commands. Original
input SHA256/mode/mtime, executable identities and complete separate-process
save/observation comparisons are recorded privately.

| Rule10 recipe, tick 60816 | Construction | Installed workers | Funds | Trailing 20400 operating cashflow | Food misses 816–60816 |
|---|---:|---:|---:|---:|---:|
| supplied | 1670 | 135 | −18130 | −9370 | 5 |
| supplied 12 | 2150 | 135 | −11801 | −6993 | 633 |
| lean 6 | 883 | 34 | 7329 | −1268 | 307 |
| lean 12 | 1363 | 34 | 8193 | −784 | 1197 |
| staged | 1670 | 135 | −17934 | −9370 | 5 |
| staggered 12 | 2150 | 135 | −13956 | −7611 | 637 |

All reach the actual World goal at 816. All measured complete 20400-tick operating
windows lose money. Positive remaining starting capital is not operating profit.
The unchanged OE36 six-House/two-Watch plan ends 60816 with−16042 Funds and
−8678 trailing operating cashflow, reproducing its fire improvement and deficit.
The underlying results and causes remain in [OE34](paid-progression.md),
[OE35](oe35-economy-fire-diagnosis.md) and [OE36](oe36-fire-protection.md).

The two-Food-Farm limit is an OpenEmperor admission/DTO bound in
`WorldOriginalFood.cpp`, repeated by the strict save parser, not an original
model value. Each staffed Farm earns one unit per 80 productive ticks. Two
chains can earn at most 20.4 units per 816 ticks before fire, buffer or transport
losses; twelve occupied Houses consume 24. Pausing providers cannot remove the
installed-worker invoice. Additional stock elsewhere or a dispatched walker
does not satisfy a House.

The first capacity-only diagnostic candidate allowed three paid Food chains
while retaining 400-tick upkeep. Its source archive and executable are frozen
privately; its experimental schema28 saves are **not** supported final Rule12
saves and are never promoted or used to initialize the final experiment.

With a real Warehouse closing the Hemp→Warehouse→Market chain, the paid
`three-food` city at 20000 has 12 occupied Houses, 318 residents, 184 consumed goods
and 612 actual Food House receipts. Over 20000–80000 it has **zero Food misses**,
but its 102 installed workers cost 10404 per 20400 ticks against 7950 actual tax:
two consecutive losses of 2454. The initial third Farm cures the demonstrated
Food-capacity deficit without changing harvest time, but capacity alone does
not fund the portfolio. Hemp/current-row shortages remain separately measured.

An initially tested extra Market at(17,7) was disconnected because Market is 1×1.
Its purchase costs 140 plus 4 installed workers while providing no new route.
That diagnostic result is retained; planned proximity is not counted as supply.

Corrected Rule11 comparisons pay a Warehouse in every lean recipe: the earlier
missing Warehouse was an infrastructure defect in that recipe, not a failed
Hemp-production kernel. The additional Rule11 results below use 20000–80000,
so their Food counts have a different observation boundary from the OE35 table.

| Approach and actual recipe | First two 20400 cashflows | Food misses | Current unmet evaluations |
|---|---:|---:|---:|
| A: Rule11 supplied 12 plus two Watches | −4941 /−4985 | 272 | 2283 |
| A: Rule11 lean 12 plus Warehouse, Hemp, two Watches | −81 /−68 | 1007 | 5408 |
| A: Rule11 compact-two, including charged disconnected Market | −1764 /−1795 | 278 | 3439 |
| B: capacity-only three-food, original 400-tick upkeep | −2454 /−2454 | 0 | 1826 |
| C: final Rule12 compact-connected-three, paid road 17,8 | +2250 /+2250 | 0 | 1308 |
| C: final Rule12 balanced-three, second Hemp chain | +1950 /+1950 | 0 | 0 |

Every remaining current deficit in B is Hemp: six Attractive Cottages require
twelve units per 816 ticks, but its single Farm can earn at most 10.2. The other
six Plain Cottages hold legitimate buffers and currently need no Hemp. The
second already-legal Hemp Farm/Field costs 44 and 18 installed workers; its
actual arrivals remove the shortage. The Hemp limit stays two. Connecting the
extra Market removes its route problem but does not manufacture missing Hemp.
No finding here asserts that every conceivable historical layout must fail.

## Versioned contract

Only Rule12 changes these two authored rules:

| Authority | Rules10/11 | Rule12 |
|---|---|---|
| Paid Food Farm/uniquely adjacent owned Field pairs | at most 2 | at most 3 |
| Food farm/stock/actor DTO bounds | 2 /26 /6 | 3 /27 /7 |
| Installed upkeep | 2 Funds per required worker per 400 placement-relative ticks | same 2 Funds per required worker per 816 placement-relative ticks |

The 816 interval aligns installed liability with the existing authored tax month;
it is not a claim about original Emperor wages. A 20400-tick window contains 25
monthly invoices instead of 51 historical invoices. No worker disappears from
the bill during fire, pause, unstaffing or idle production. Negative Funds do not
stop existing production or erase liabilities; real later taxes may repay debt.

Unchanged: model bytes, costs, required employees, TaxRate values, pre-tick direct
tax every 816 ticks,80-productive-tick harvests, needs/consumption, stock/cart
capacities, immigration, evolution, DES, service/water/religion timers and actual
arrivals, shared routes, World goal, starting 12000 Funds, Policy3,82 buildings,
58 couriers and 20 Houses. No new navigator, stock authority or money ledger is
introduced. Fire retains 8192 arrivals,8192 tax witnesses and 65536 travelled cells;
the last valid state still pauses before a provenance limit and remains saveable.
The document limit remains 8MiB.

Rule12 requires its own factory and strict schema28 restore. Historical schemas,
Rule10's FireWatch refusal and Rule11's exact economic/fire contract remain.
Missing/mistyped/duplicate/cross-schema authority and impossible accounting,
ownership, footprints, routes, stock or fire histories fail before publication.
No automatic migration or parameter switch reinterprets an existing save.

## Reproduce

Use a new ignored output and the player's own read-only installation:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DOPENEMPEROR_USE_SYSTEM_SDL3=ON -DOPENEMPEROR_ORIGINAL_MODEL_TEST_ROOT=/absolute/path/to/Emperor
cmake --build build --parallel 2
python3 -B tools/test_sustainable_economy.py --binary "$PWD/build/openemperor-sustainable-economy-acceptance" --data /absolute/path/to/Emperor --resources "$PWD/build/resources" --output "$PWD/.local/original-economy/oe37/acceptance-new"
```

The creator pays every command from its actual treasury, then observes the
entire warmup to 20000. A maturity witness requires the actual World goal,
twelve occupied non-burning Houses, positive actual Food reception/goods
consumption/service/water/Inspector arrival witnesses. It is not a claim that
every current or next need is continuously met. The runner then observes 100000
additional real ticks. A second process restores only the earned 20000 save;
complete seed/midpoint/final SaveDocuments and all continuation metrics must
match literally. SaveDocument, source, accounting, goods and Food conservation
are checked independently; the goal is never reimplemented by the test.

Every House evaluation records actual current and next needs, stocks,
FoodQuality, timers and arrival witnesses. Actual invoices, tax, installed/active
workers, transport durations/returns/reservations, fire visits, upgrades,
devolutions and goal losses are observed. Consecutive 20400-tick windows use
actual tax minus **all** actual upkeep, with construction booked separately and
Funds deltas reconciled. A partial trailing window cannot count toward the
two-window requirement. This establishes only the measured finite stability.

In the game, **S Sustainable economy** selects Rule12; **O Original economy**
and **F Original fire protection** retain Rule10/11. CLI selection is
`--original-sustainable-economy` with the City-v16 profile. The optimized recipe
starts from **Empty**; the existing paid four-House starter remains available
unchanged. F5/F9 and opt-in autosave/recovery retain the selected generation.

## Paid twelve-House recipe

The measured recipe is `balanced-three`, on an empty fresh Rule12 World. A
separate ordinary starter supplies only the legal-area search; the measured
World buys its own entire plan and receives no state from that reference.
The actual Xia origin is (102,120), on the same legal 24×13 level area. All
coordinates below are offsets from that origin. Every purchase occurs while
paused at tick 0; each Field command explicitly selects its newly paid owner.

| Purchase | Relative origins/cells | Total Funds | Installed workers |
|---|---|---:|---:|
| 72 roads | x0–23 at y4/9; x20 and x2 at y5–8; x5/8/11/14 at y0–3 | 144 | 0 |
| Twelve 2×2 Common Houses | x0/3/6/9/12/15 at y2/7 | 960 | 0 |
| Water Well72 | (4,5) | 25 | 4 |
| Ancestor Shrine | (6,5) | 40 | 4 |
| Market | (9,5) | 140 | 4 |
| Three 3×3 Food Farms | (0,10),(9,10),(18,10) | 150 | 66 |
| Their owned Millet Fields | (3,10),(12,10),(21,10) | 12 | 0 |
| Warehouse | (18,5) | 150 | 2 |
| Two Hemp Farms | (5,10),(14,10) | 80 | 36 |
| Their owned Hemp Fields | (7,10),(16,10) | 8 | 0 |
| Two FireWatches | (0,5),(19,7) | 160 | 4 |
| **Total:100 commands,28 bodies,72 roads** | | **1869** | **120** |

After purchase Funds are 10131, population/goods remain 0 and no tick has elapsed.
Food follows Farm→Market→House; Hemp follows Farm→Warehouse→Market→House.
Road shortcuts are actually purchased. No production, provider, deadline or
placement permission is inferred from distance. Priorities remain Normal and
all providers remain Running; every installed worker is billed throughout.

The actual World goal is first true at **816**:12 occupied Houses, seven current
Plain-or-higher,202 residents,156 actual taxes,240 upkeep and 10047 Funds. The
startup is imperfect: before 20000 there are 42 Food misses,243 current unmet
evaluations,21 devolutions and six actual fires. The 272 actual Inspector visits
do not extinguish an active incident in this case. All 815 false-goal ticks
precede the first true goal. No early deficit is removed from the warmup record.

At **20000**, maturity is actually witnessed: twelve eligible Houses, 318
residents,11785 Funds and active paid supply/protection. The following 100000
ticks maintain six Plain Cottages 5 with 22 people each and six Attractive
Cottages 6 with 31 each. They retain real FoodQuality 80, Water, Ancestor visits
and Hemp stock. No current evaluation fails; there are no new Food misses,
devolutions, fires or goal losses. Initial 42 Food misses remain in lifetime
counters. The first and last partial calendar periods are included in totals.

| Complete operating window | Actual tax | All upkeep | Construction | Operating cashflow | Funds at end |
|---|---:|---:|---:|---:|---:|
| 20000→40400 | 7950 | 6000 | 0 | +1950 | 13735 |
| 40400→60800 | 7950 | 6000 | 0 | +1950 | 15685 |
| 60800→81200 | 7950 | 6000 | 0 | +1950 | 17635 |
| 81200→101600 | 7950 | 6000 | 0 | +1950 | 19585 |
| Full 20000→120000, including trailing 18400 | 39114 | 29520 | 0 | +9594 | **21379** |

Lifetime accounting at 120000 is exactly
`12000 +46528 tax −1869 construction −35280 upkeep =21379 Funds`.
No new capital is injected, no running cost excluded and no debt occurs in the
accepted city. Its 120 installed workers cost 240 per 816 ticks; actual 318
rate 1 residents pay 318 when assessed. These are real payments, not a frozen
population multiplication offered as substitute evidence.

Post-maturity movement records 2041 actual Food handoff arrivals,2500 goods
handoff arrivals,923 applied service/religion visits,991 applied Water visits
and 1395 actual Inspector arrivals. There are no observed transfer returns,
cancellations or refused service/water visits. Handoff units 5890 Food/4412
goods include multiple legs of the same mass; they are not unique production.
Raw seed/final counters independently confirm 2946 new Food units actually
received by Houses and 1470 goods actually consumed after maturity.
At 120000 conserved lifetime Food harvest is 3582, consumption 3486 and total
stock/cargo 96. Hemp is 1872 produced =1725 consumed +146 stock +1 cargo.
Other goods remain 0 because the tested current rows do not require them.

Future progression remains separate: 11766 next-row evaluations lack Herbalist
and Music; the six Plain Houses also lack sufficient DES for promotion. The
unchanged Normal models price Herbalist207/Music211 at 30/50 with 7/8 workers.
**Hypothetical installed arithmetic only:** those two additional purchases cost 80
and 30 recurring Funds per 816 ticks, reducing the current 78/month margin to 48
before any changed population, routing, demand or fire. This is not a simulated
higher-tier result. No such services are granted or bought merely to hide that
remaining progression boundary.

The final schema28 save is 1482151 bytes, with 1667 lifetime Inspector arrivals,
1764 tax witnesses and 25543 travelled path cells. All remain within the retained
finite proof/document budgets. Protection of all 23 combustible bodies at the
captured final tick does not imply infinite capacity or a universal patrol guarantee.

## Original parity and limits

All raw model inputs remain unchanged. Authored capacity, upkeep, recipes,
real-time scheduling, immigration/workforce and targeted School→House visits
remain distinct from original evidence. Numerical-kernel comparisons do not
establish full Emperor runtime, object, illness or venue/performer parity.
House sprites use the existing verified art; other bodies/walkers retain their
documented fallbacks. Scripted SDL software captures are separate from native
display/input and a human playthrough. No original game data or diagnostic
saves/captures belong in Git or application bundles.

## Validation evidence

Release runs the complete **257/257 CTests**, without skips, in 165.37s.
ASan+UBSan runs the complete **257/257**, without skips or findings, in 770.74s
using RelWithDebInfo/O2. On this macOS run `ASAN_OPTIONS` is
`detect_leaks=0:halt_on_error=1` and `UBSAN_OPTIONS` is
`halt_on_error=1:print_stacktrace=1`; LeakSanitizer was not run.

The full Debug run completes all 257 cases in 2479.19s: 256 pass and the old
Reset-coordinate test fails. After rebuilding the current app/UI/harness,
all **6/6 affected delta cases pass**, including Reset, both generations'
menus/invoice deadlines, legacy views, the paid Rule12 check and runner guards.
Thus all 257 unique cases have passing current coverage, with no skips; this
is a full run plus a focused correction, not a claimed single all-green run.

The separate Release creator/restore/render workflow and sanitizer
creator/restore workflow both pass the full 100000 post-maturity ticks.
Their complete seed, midpoint and final SaveDocuments and entire metrics
document are byte-identical across processes and configurations. The separate
O0/`-g` Debug creator/restore proof also passes, taking 508.06s/415.99s and
producing those same complete bytes. Its isolated executable links the frozen
final simulation/persistence archives; an unused presentation archive predates
the caption fix. Current canonical Debug UI/harness is separately rebuilt and
covered by the six passing delta cases above. Recovery
readback compares complete documents. Actual production SandboxView F5/F9
and 1×/2×/4× SDL software captures pass; the final images were opened and
inspected. They show Rule12/Schema28, 318 residents, 21379 Funds and the correct
816-tick invoice, including the selected Watch's 4 Funds per 816 ticks.

Initial verification exposed two presentation/test issues: the Inspector still
printed 400 ticks, and a legacy menu test clicked an old Reset coordinate after
the S option was inserted. The caption and shared invoice fallback now use the
selected interval; the test resolves the actual Reset rectangle. The final
Release suite passes completely. A strict rendering guard rejects the frozen
old 400-caption executable instead of accepting its otherwise-correct World.

Additional checks:

| Check | Result and scope |
|---|---|
| Runner evidence guards | 19 adversarial tests pass, including false-goal, incomplete-window, omitted-cost, fund/payment mismatch and shortened-duration refusals |
| Numerical parity | 74/74 pinned-fork vectors pass; prior numerical inputs/results remain exactly equal; the reviewed World API pin and tool provenance are refreshed |
| Historical save bytes | 62/62 authored historical SaveDocuments remain byte-identical to the frozen baseline; this is a finite corpus |
| Rule10/11 regression | Twelve real baseline save/metrics/fire files remain byte-identical; fresh OE36 four-process acceptance passes twelve complete comparisons |
| World regressions | Third paid chain, owned Fields, fourth-chain refusal, legacy limits, strict generation restores, separate road components, paid repair, begun-edge safety, cargo waiting, conservation and full disk continuation pass |
| Debt/recovery | Deterministic authored-model debt followed by real tax recovery passes complete disk continuations; the actual Normal capacity-only diagnostic separately records debt/recovery without grants |
| Input integrity | Creator, restore and render workflows retain original source bytes/modes/mtimes and executable/source identities |

Private artifacts are under ignored `.local/original-economy/oe37/`: the
canonical Release proof is `release-acceptance-04/acceptance.json`, sanitizer
proof is `sanitize-acceptance-01/acceptance.json`, Debug proof is
`debug-acceptance-01/acceptance.json`, and their complete comparison
is `cross-configuration-comparison.json`. The complete final suites are
`release-final-ctest.xml` and `sanitize-full-ctest.xml`; earlier failed caption,
Reset and evidence-schema attempts remain recorded and are superseded rather
than reported as passing. `historical-byte-comparison.json`,
`legacy-rule10-rule11-byte-comparison.json` and
`parity-unchanged-vectors.json` retain the bounded compatibility evidence.
Debug records are `debug-full.junit.xml`, `debug-ui-delta.junit.xml` and
`debug-validation-summary.json`; `final-validation-summary.json` joins the
configuration, continuation and rendering results.

Not run: native display/input delivery, a human playthrough, packaging,
Gatekeeper/notarization, the unavailable old private OE00 corpus, other maps
or difficulties, higher-tier progression and unbounded-duration economics.
Scripted captures do not establish any of those properties.

## Acceptance result

| Dimension | Status | Evidence and limit |
|---|---|---|
| Technical stability | PASS | Complete relevant Release/Debug/sanitizer coverage, separate-process continuation, strict persistence and inspected production captures |
| Sustainable economy | PASS | Twelve occupied Houses and 318 residents maintained through 100000 post-maturity ticks; four consecutive complete 20400 windows each earn 1950 Funds after all upkeep |
| Goods supply | PASS | Actual paid Food/Hemp chains, physical House receipts/consumption, service/Water visits and no current unmet evaluations after maturity; higher stages remain unproven |
| Fire protection | PASS | Paid Watches and 1395 actual Inspector arrivals after maturity, no new fires in the measured interval; six startup fires remain disclosed |
| Original-game parity | PARTIAL | Original inputs and numerical vectors are unchanged, while capacity, invoices and other documented simulation/presentation contracts remain authored or incomplete |

The final actual World goal remains true at tick 120000 with **21379 Funds**.
This accepts the finite measured city, not arbitrary layouts or unlimited play.
