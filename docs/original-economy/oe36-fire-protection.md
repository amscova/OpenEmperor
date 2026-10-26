# Original economy: versioned fire protection

Rule11 is an explicit **City-v16 / schema27 / policy3** contract. Its economy
retains development revision27 and the Rule10 model values, taxes, upkeep,
production, consumption, immigration and evolution. No automatic migration is
provided. **O Original economy** still selects Rule10/schema26; **F Original
fire protection** selects Rule11/schema27. General menu and CLI defaults retain
their previous identities. Original game data is read-only and supplied by the
player.

**Technical acceptance: PASS for the bounded Rule11 contract.** Real paid
construction, actual Inspector effects, complete persistence and regressions
are demonstrated below. The measured city remains financially loss-making.

## Authored building and actual movement

The paid FireWatch is the existing OpenEmperor **authored** contract: 1×1,
80 Funds, two required workers, at most two Watches, one cargo-free
FireInspector per Watch. Installed-worker upkeep adds four Funds per400 ticks
for each Watch, including while paused, unstaffed or burning. These are not
verified original Emperor values. The model name Inspector's Tower124 and its
pixels do not establish a functional mapping; no original model file is changed.

Construction uses ordinary World validation, legal terrain, paid Funds,
monotone IDs, complete footprints and the existing82-building/58-courier bounds.
New patrols require a running, actually staffed, non-burning owner. A visit
grants nothing at dispatch: the shared navigator and normal movement must
reach the target. Actual arrival resets risk, extinguishes an ongoing fire and
protects the target for2400 ticks. An arrival exactly at natural expiry grants
protection but is not recorded as extinguishing. Previously dispatched walkers
finish after their source is paused or ignites. Interrupted routes retain
begun-edge safety and the existing pending/repair/return behavior.

Rule11 Watches are combustible and may be visited by the other Watch; a Watch
does not target itself. Historical profiles keep their previous Watch eligibility.
Inspector target candidates/routes use the original-economy82-body limit only
under the new feature guard; historical40/38-body bounds are preserved. Route
preparation remains revisioned and outside frame/hover processing. There is no
second fire kernel, navigator or passive protection radius.

## Schema27 and finance provenance

Schema27 adds `original_production.fire_protection`; schema26 does not serialize
this field. Its compact, typed records contain active dispatches, actual visited
paths/arrivals and individual pre-tick monthly tax assessments. They are witnesses
to existing World authority, rather than a parallel budget or tax calculation.

Restore validates paid Watch/Inspector ownership, current actor/path/progress,
target relations, route timing and possible physical returns, dispatch operation
and required staffing, population/admission opportunities, actual travelled
adjacent cells and shared permission edges. Historical removed roads remain
possible; absent historical transport cells are bounded by actual paid road
removal accounting. A road revision alone does not grant a route. The journal
does not claim cryptographic authentication of an arbitrary past player session.

The existing placement-relative risk kernel is replayed arithmetically between
actual arrivals. Every body's risk, incident deadline and protection deadline
must match the replay. Extinguishing records must agree with the incident at the
arrival tick. Tax records sample fire **before** the tick's movement/fire phase;
their exact time/type/residents must fit actual admission/transition histories
and reconstruct the existing per-row tax buckets. Bound model rates, paid costs,
installed upkeep and the original Funds equation remain mandatory. Rule10 keeps
its stricter natural-fire-only history check.

Map, mask, model, difficulty and policy fingerprints remain required before
atomic publication. Missing, unknown, duplicate, mistyped, cross-schema or
contradictory authority is refused without replacing the city or prior save.
F5/F9, manual reload and recovery select the exact rule; a Rule10 save is never
promoted to Rule11. LegacyOnly remains the recovery API default.

The new proof budget is8192 arrivals,8192 individual tax witnesses and65536
total visited path cells, plus at most two active trips. The existing8MiB save
and32-level nesting limits remain. Preflight checks counters before any tick
mutation; full historical replay happens during validation, not in normal ticks
or frames. Reaching a proof limit pauses the View with the explicit reason;
the last valid state remains saveable. This is a finite supported history, not
an assertion of unlimited play duration. Additional compression/longer history
would need a reviewed extension without weakening restore checks.
An independently paid20-House/no-Watch regression reaches the actual frontier
at tick434927 with8180 tax witnesses: the following monthly assessment is
refused before any mutation. The complete snapshot remains equal, writes and
restores as schema27, and the restored next tick is refused identically.

## Reproduce the real paid comparison

From a configured build with the player's pinned GOG-derived original data:

```sh
cmake --build build --parallel 2 --target openemperor-fire-protection-acceptance
python3 tools/test_fire_protection.py --binary "$PWD/build/openemperor-fire-protection-acceptance" --data "$PWD/.local/gog-extracted/app" --resources "$PWD/build/resources" --output "$PWD/.local/original-economy/oe36/acceptance-new"
```

The runner uses real `Cities/Xia.map`, Normal and the existing paid four-House
starter. It purchases the unchanged OE34 `supplied` expansion: two Houses at
relative(12,2)/(15,2), four shortcut roads x2/y5–8, second Food Farm(9,10) with
Millet(12,10), and second Hemp Farm(14,10) with Field(16,10). Rule11 additionally
buys Watches at(0,5)/(23,10), with unchanged Normal priorities. Every purchase
uses an ordinary World command. There are no stock, service, Funds, resident or
fire grants. Rule10 performs the unchanged legal funded FireWatch-refusal control.

The production World goal is observed at816; the runner then measures60000 real
ticks, including20816 and60816 checkpoints. Each rule is resumed in a separate
process from its earned816 save. Complete seed, midpoint,20000/final saves and
metric files must match literally. A separate process restores the earned
Rule11 mid-flight save in the production SandboxView, exercises actual F9/F5,
and captures1x/2x/4x through SDL software rendering. Original input hashes, modes
and mtimes, source identities and executable identity are recorded before/after.

Counts distinguish actual ignition, actual Inspector arrival, actual
extinguishing and protected risk-step opportunities. A difference between two
runs' ignitions is a paired observation, not a directly counted prevented event.
Lost-goal ticks with burning Houses are associations; negative Funds and other
goal failures are reported separately. Excluded tax at actual burning inputs is
shadow arithmetic, never credited income. Finite solvency is separate from a
self-financing economy; see [the prior diagnosis](oe35-economy-fire-diagnosis.md).

## Measured paid Xia result

Both rules reach the **actual World goal at tick816**, with six inhabited
Houses,108 residents, four Common PlainCottage-or-higher Houses and80 actually
collected taxes. All inventories/residents start at zero. Rule10 buys1670 Funds
of infrastructure; Rule11 buys1830, including two80-Funds Watches. Required
installed workers rise135→139, and the400-tick invoice270→278.

| Actual checkpoint | Rule10 Funds / tax / upkeep | Rule11 Funds / tax / upkeep |
|---|---:|---:|
| Goal816 | 9870 /80 /540 | 9694 /80 /556 |
| Goal +20000, tick20816 | 290 /4000 /14040 | 978 /5264 /14456 |
| Goal +60000, tick60816 | −18130 /12580 /41040 | −16042 /16044 /42256 |
| First negative Funds tick | 21600 | 23200 |

Population is220 and all six Houses remain occupied at both later checkpoints.
Rule11's final House types are5/7/8/8/6/5, including two Elegant Dwellings8;
retained residents after devolution remain part of the unchanged economy.
During816→60816 there are four observed devolutions under Rule11 versus seven
under Rule10.

| Actual measurement,816→60816 | Rule10 | Rule11 |
|---|---:|---:|
| Ignitions | 483 | 11 |
| Inspector arrivals | 0 | 738 |
| Inspector extinguishings | 0 | 2 |
| Protected placement-clock risk opportunities | 0 | 60716 |
| Lost-goal ticks with burning Houses and nonnegative Funds | 4800 | 600 |
| All lost-goal ticks, including debt | 44017 | 38217 |
| Shadow tax excluded at observed burning-House inputs | 3466 | 53 |
| Food missed demands | 5 | 5 |
| Food actual arrival observations / return observations | 1200 /26 | 1466 /0 |
| Goods actual arrival observations / return observations | 1408 /28 | 1733 /0 |
| Applied service visits / rejected visits | 2943 /73 | 3789 /2 |
| Applied water visits / rejected visits | 1459 /33 | 1891 /0 |

Rule11 also has six real Inspector arrivals during its0→816 warm-up, for744
lifetime arrivals at60816. Both cities end with925 lifetime Food House receipts;
the observed Food handoff mass1851 is also equal. Increased arrival counts do
not mean extra mass: delivery batches, returns and timing differ. Goods handoff
units are2601 versus2608. These are handoffs, not unique produced units.
The first actual visit is Inspector18→House1 at530, protecting it until2930.
Inspector18 extinguishes Ancestor16 at2030 (protection4430); Inspector19
extinguishes the burning Watch26 at4800 (protection7200). Both Watches really
ignite at2000; Watch26 ignites again at4580 before that second Inspector arrives.

The initial legitimate Watch staffing/routing leaves some buildings without an
early visit; eleven incidents still occur. No incident is suppressed by the test.
Two ongoing incidents are extinguished on arrival; an exact natural-expiry
arrival is separately tested and does not count as a successful extinguishing.

**Financial sustainability remains unproven and this plan loses money.** Over
the first20000 additional ticks, Rule11 tax5184 pays only37.29% of13900 upkeep,
an operating loss8716. The last common20400-tick interval still loses8678,
versus9370 for Rule10. Higher actual tax delays the first debt by1600 ticks and
improves final Funds by2088, after real construction/upkeep costs. It does not
make the portfolio self-financing. No economic value or original model was
changed to obtain these results.

At60816 the strict Rule11 save is742199 bytes, with744 arrivals,442 tax
witnesses and12711 actual visited path cells, below the unchanged document
limit and new proof budgets.

## Technical and visual evidence

The canonical paid workflow is private
`.local/original-economy/oe36/release-acceptance-03/`: five actual processes,
twelve byte-identical full-save/metric continuation pairs, unchanged1464
original files/819711091 bytes including modes/mtimes. The strengthened
return-target validation reproduces and rejects a forged alternate legal
return route;24 saved/metric outputs still match the preceding legitimate run.

At tick521, the earned mid-flight save restores in the production SandboxView
via actual F9 and rewrites byte-identically via F5. Its1x/2x/4x images were
opened and inspected: original Xia terrain/House art, existing FireWatch art,
active Inspector fallback, selected Watch's authored price,2/2 workers, running
operation, target1, actual fire risk and upkeep are visible. The existing
Inspector graphic fallback is retained; a matching original profession image
is not invented. These are software captures, not native human acceptance.

The cross-revision exporter produced62 complete authored historical documents
(native historical generations plus schema1–3 parsing/reserialization) against
the baseline libraries saved before changes and d5116f9 headers, then against
the candidate. Every corresponding byte matches. Separate existing historical
save and transport tests remain required, including Rule10 FireWatch refusal.
The real OE35 Rule10 seed816, midpoint30816 and final60816 documents also match
the new Rule10 documents literally.

The four-module numerical comparison was actually rerun against11 independently
SHA-verified public fork files pinned to
`d861d2912337c09751cb68196f85d5a30752726e`, matching freshly linked archives and
the real Normal model. **74/74 vectors pass**; all old cases, expected/observed
values, source pins and model digest remain exactly equal. Only reviewed
World/production/fire-state ABI bindings and current-source fingerprints are
updated in [core-parity.json](core-parity.json); no baseline lock is regenerated.

| Executed configuration | Final result |
|---|---|
| Complete Release CTests, latest source | **254/254 PASS**, zero skips |
| Relevant Debug CTests, latest results | **151 distinct PASS**,149 initial plus15 final executions |
| Relevant ASan/UBSan CTests, latest results | **153 distinct PASS**,150 initial plus17 final executions; no findings |
| Source-bound numerical comparison | **74/74 PASS**,28 report/input guards also pass |
| Historical cross-revision SaveDocuments | **62/62 byte-identical** |
| Release60000-tick paired Rule10/11 continuation | Five processes including View;12 full byte comparisons |
| ASan/UBSan20000-tick paired Rule10/11 continuation | Four processes;12 full byte comparisons |
| Debug20000-tick Rule11 continuation | Two processes;6 full byte comparisons |

Six common complete seed816/+20000 SaveDocuments also match across Release,
Debug (Rule11) and sanitizers (Rules10/11). Every workflow preserves original
file bytes/modes/mtimes, the tested executable and its recorded sources.
`cross-configuration-final.json` records those identities; Debug's separate
process evidence is in `debug-acceptance-01/acceptance.json`.

The initial broad Release run exposed three old assertions that Rule11 was
unknown and stale comparison ABI/source bindings. The assertions now reject
Rule12 and verify Rule11's separate guard; old rule expectations are retained.
The comparison was freshly executed rather than accepting replacement hashes
without observations. The Debug initial run retained one old compiled gate
assertion; rebuilding and rerunning it in the final15-test set passes. Earlier
failure logs remain private and are not counted as passing executions.

The final save regressions reproduce and reject both alternate unvisited return
targets and same-tick return-progress/early-idle forgeries. Actual begun return
edges, pending waiting, paid road repair, resumed paths and later idle restore
remain valid. Hostile extreme-coordinate DTOs are rejected under sanitizers.

Evidence is under ignored `.local/original-economy/oe36/`, including
`release-current-ctest.xml`, `debug-focused.junit.xml`,
`debug-frozen-delta.junit.xml`, `sanitize-ctest.xml`,
`sanitize-freeze-ctest.xml` and `final-validation-summary.json`.
Sanitizers use RelWithDebInfo `-O2 -g`, address+undefined,
`ASAN_OPTIONS=detect_leaks=0:halt_on_error=1` and
`UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`. LeakSanitizer is disabled
on macOS; no leak-check claim is made.

Targeted tests cover legal/refused purchases, insufficient Funds, terrain,
staffing, operation, real prevention/extinguishing, Watch fire, disconnected and
repaired roads, active-trip continuation, finance/hostile saves and recovery.
Cross-revision canonical documents and existing historical CTests cover the
exercised old formats; finite samples do not prove every possible old save.

The captures exercise the actual production View on original Xia data. They are
scripted SDL/software checks, separate from native display, mouse/focus, human
play, packaging, Gatekeeper and complete original-game parity. Medicine/culture
chains, original object identity and other original-economy limitations remain
as described in [the architecture](architecture-map.md) and
[Known Issues](../../KNOWN_ISSUES.md).

NOT_RUN: native human play/display/input acceptance, packaging/Gatekeeper,
other maps/difficulties, exhaustive city layouts, an unlimited-history run,
and60000-tick Debug/sanitizer city comparisons. The separate old private OE00
reference-save corpus was not recreated; the executed historical tests and
byte exporter provide the stated finite compatibility evidence.
