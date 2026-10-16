# OE-05: independent service and religion rules

`openemperor-original-service-rules` contains pure visit-effect, countdown and
staffing-counter functions. It reuses the OE-04 House observation and is not
linked into the application, World, navigation, renderer or persistence. There
is no new scheduler, building, courier, route, proximity query or World rule.

The original instruction evidence is split between the
[visit/timer contract](../reverse/original-service-timer-contract.md) and
[staffing/spawn contract](../reverse/original-service-spawn-contract.md).
The original executable is read-only research material and was not run.

## Explicit visit effects

`ServiceRole` names five service accesses and four religions. Successful service
effects assign 96; religion effects assign 40. Assignment restarts the counter,
including lowering a restored byte of 255. It does not add duration or take the
maximum. Role, action and provider enum values are authored API identities,
not original actor IDs, ModelBuilding IDs or World courier roles.

`ServiceVisit` is an already-resolved timer effect. `apply_service_visit` returns
a new snapshot; `accepts_service_visit` exposes the predicate separately. Both
take an explicit `ServiceHouseContext`: the target House's original status
byte and the nonzero observation of its byte `+0x09`. The latter's gameplay
producer is unknown. Default context is inactive and grants nothing.

| Visit path | Supported actions | Eligibility beyond valid House type and byte-09 access |
|---|---|---|
| DirectService | Herbalist, Acupuncture, Music, Acrobat, Drama individually | Status 1 or 3, positive signed occupants; no empty-elite exception |
| DirectReligion | Ancestor, Confucian, Daoist, Buddhist | Status 1 or 3; positive occupants or elite type; a resolved `elite_only` source condition additionally requires elite |
| SharedWalker | Herbalist, Acupuncture, Music, four religions, paired AcrobatAndDrama | Positive occupants or elite type; this callback has no local status check; each religion effect independently respects its resolved `elite_only` condition |

`elite_only` is an observed religion-source condition, not a guessed property of
a building name. It is invalid for nonreligious effects. Single Acrobat/Drama
shared effects and unsupported path/action combinations are rejected before
any output is published. The shared entertainment action sets both bytes.

The original shared actor can also emit several religion effects plus a service
effect. An explicit ordered sequence of resolved events expresses that scoped
nine-timer projection. A single event is not an entire actor callback: routing,
source-condition resolution, goods, health and accounting effects remain
outside this module. The evidence register records the concrete original
combinations without pretending that the kernel recreates their actors.

No function derives a visit from a nearby building, staffing percentage or a
due dispatch counter. The caller must supply an actual resolved visit input;
future World integration must bind it to the corresponding real event.

## Countdown counts and proven order

All nine access values are unsigned bytes. `decay_service_timers` performs one
raw nine-field transformation: zero stays zero, every positive byte decreases
by one. `decay_house_services` adds the observed House caller gate, status 1/3
and byte-09 access, without an occupancy gate. An otherwise eligible empty
House therefore still loses time; a skipped House retains its bytes.

With no renewal, a 96 grant is present after 95 eligible countdowns and absent
after 96. A 40 grant expires on countdown 40. Both functions preserve every
non-timer House field and leave the input untouched. Invalid House/role enums
are rejected even behind inactive gates; this is an explicit authored API
boundary where some original local functions rely on their target context.

The inspected scheduler dispatches House countdown at phase 35 and evolution
at phase 39. A byte at one can therefore expire before evolution evaluates its
requirement. A visit between these phases can renew it. The tests exercise
both sequences explicitly. The module deliberately supplies no fused global
cycle operation or wall-clock mapping: 96 countdowns are not 96 World ticks,
seconds or months. The broader scheduler, arrival ordering and tick conversion
remain later tasks.

## Staffing threshold and due attempt

`SpawnBuilding` distinguishes eleven providers, including separate Daoist
Shrine/Temple and Buddhist Shrine/Pagoda. The original uses five distinct
threshold functions, rather than the fork's two generic tables. The source
contract gives their complete class/vtable bindings and thresholds.

`service_spawn_interval` returns the comparison threshold for a signed int32
effective staffing percentage, using inclusive thresholds 100/75/50/25/1.
Above 100 uses the first tier; nonpositive values return the last. That last
query result does not enable dispatch without workers.

`service_spawn_step` receives the already-resolved staffing value, a byte
counter and an explicit observation that all pre-counter prerequisites permit
evaluation. Confucian Academy has an additional original source-word gate;
the kernel neither names an unproved resource nor recreates workforce allocation.

When prerequisites are closed or staffing is nonpositive, the counter stays
unchanged. Otherwise it increments modulo 256, then compares the new byte
strictly greater than the threshold. A due attempt resets to zero before any
actor factory. Thus threshold 1 is due on the second eligible call from zero;
old byte 255 wraps to zero and is not due. Factory failure does not undo a
reset. The returned `attempt_due` proves neither actor creation nor arrival
and never grants House coverage.

## Reproduction and limits

```sh
cmake --build .local/reswall/release-build --parallel 2 \
  --target openemperor-original-service-rules-tests openemperor-sandbox-city-v8-tests
ctest --test-dir .local/reswall/release-build --output-on-failure \
  -R '^(original-service-rules-(synthetic|real)|sandbox-city-v8-service)$'
```

Synthetic tests enumerate visit eligibility, every timer byte, renewal/expiry,
no-visit behavior, all staffing boundaries/counter bytes, invalid inputs and
explicit countdown/evolution order. The separate real-source test composes
these operations with actual prepared House needs, retaining the original hash
and unchanged source bytes/metadata. Only missing optional data skips with 77;
existing malformed/oversized data fails. Executed configurations, repeatable
commands and the frozen legacy comparison are recorded in
[PROGRESS](../codex-original-economy/PROGRESS.md).

The new producer bindings also clarify the alternative religion byte names in
the earlier needs evidence: Daoist `+0x0e`, Buddhist `+0x0f`. The commutative
OR requirement and all existing HouseEvolution behavior remain unchanged.

These results establish bounded numerical rules and a scripted pure supply
sequence. They do not establish a paid original supply loop, complete actor
behavior, original real-time parity or human gameplay acceptance. Existing
City-v8 and later authored Service retain their 1200-tick actual-arrival system
and historical save contracts.
