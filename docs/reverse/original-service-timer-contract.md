# Original service and religion timers: bounded static contract (OE-05)

This records the countdown and visit effects used by the isolated SDL-free
service rules. It does not create walkers, find houses, grant access from
building proximity, or map an original update cycle onto `World::tick`.
The separate [spawn contract](original-service-spawn-contract.md) describes
the independently checked staffing thresholds and attempt counter.

## Sources and reproduction

**EXE-OBSERVED:** The read-only local `Emperor.exe` has SHA-256
`6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e`.
The model text has SHA-256
`7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6`.
Addresses below are static virtual addresses with image base `0x400000`,
not RVAs or physical file offsets. The executable was not run.

**FORK-STUDY:** The `HouseEvolution.{h,cpp}` implementation at fork commit
`d861d2912337c09751cb68196f85d5a30752726e` proposes 96/40 grants,
countdown decay, and one generic occupied-or-elite visit predicate. It is
study material; the following independently checked paths define the narrower
evidence. No original balance arrays or disassembly are distributed in Git.

With the same installation and private research artifacts, reproduce the
read-only instruction checks using:

```sh
python3 .local/original-economy/oe05/visit-check-evidence.py
```

The current private report, `.local/original-economy/oe05/visit-evidence.json`,
checks **23 bounded windows, 656 instructions and 1,957 instruction bytes**,
27 selected semantic instruction assertions, five pointer bindings, religion
selector tables, and all five service decrement patterns. It reports zero
byte mismatches and unchanged executable/model bytes. This is static evidence,
not an original-game playthrough or human gameplay acceptance.

## Storage, grants and named access roles

The concrete House virtual `+0x1e4` points to `0x416b50`, which returns House
`+0xc8`; the offsets in this table are relative to that own-info pointer.
The requirement consumers are separately documented in the
[House needs contract](original-house-needs-contract.md).

| Access role | Own-info byte | Successful assignment | Observed grant path |
|---|---|---:|---|
| Herbalist | `+0x2d` | 96 | `0x51bd00–0x51bd43` |
| Acupuncture | `+0x2a` | 96 | `0x51bd90–0x51bdd3` |
| Music | `+0x2c` | 96 | `0x48ad20–0x48adb6`, actor subtype 34 |
| Acrobat | `+0x2b` | 96 | Same path, actor subtype 32 |
| Drama | `+0x2e` | 96 | Same path, actor subtype 33 |
| Ancestor | `+0x0d` | 40 | Indexed religion visit `0x5ab580–0x5ab60d`, index 0 |
| Daoist | `+0x0e` | 40 | Same path, index 1 |
| Buddhist | `+0x0f` | 40 | Same path, index 2 |
| Confucian | `+0x10` | 40 | Same path, index 3 |

Religion duration getter `0x517270–0x517275` returns 40. Religion mapper
`0x5ab080–0x5ab0ad` and its checked dispatch table map source types 214,
215/216, 217/218 and 219 to indices 0, 1, 2 and 3 respectively. The read-only
model labels identify Ancestral Shrine, Daoist Shrine/Temple, Buddhist
Shrine/Pagoda and Confucian Academy (the source text spells `Academny`).
Constructor `0x5ab115–0x5ab11d` calls that mapper and stores the index in
source `+0x154`. This independently resolves the two alternative religion
byte names: **`+0x0e` is Daoist and `+0x0f` is Buddhist**. Their OR requirement
was already valid without distinguishing those names.

A grant is an assignment, not addition or `max(old, grant)`. A successful
repeat visit restarts the timer at 96 or 40; it does not stack duration. A
restored byte 255 is overwritten with the grant value. The width is unsigned
byte; a negative signed integer is not an original timer observation.

## Visit eligibility depends on the path

House type is a signed-short field `+0x14`; House occupancy is a signed-short
field `+0x20`. Predicate `0x5188b0` accepts types 2–17, and `0x5188d0`
accepts elite types 11–17. The concrete House virtual `+0xb8` points to
`0x42dd40`, which returns whether House byte `+0x09` is nonzero. Its complete
gameplay meaning is not established here; the pure API receives this observed
condition explicitly rather than assigning it a guessed cause.

The status helper `0x426d10–0x426d27` accepts exactly the target House's status
byte `+0x04` values 1 or 3. Neither an arbitrary nonzero status nor every
House type is sufficient to enter all these paths.

**Ordinary service visits:** `0x48ad20`, `0x51bd00`, and `0x51bd90` require
status 1/3, House byte `+0x09` nonzero, and signed occupancy greater than
zero. Empty or negative-occupancy elite houses do not bypass that occupancy
test. The entertainment path additionally checks type 2–17 explicitly;
the local Herbalist/Acupuncture functions rely on their target context and
do not repeat that type-range check. The pure API's valid-House domain is a
conservative authored boundary, not an added original instruction.

**Direct religion visits:** `0x5ab580–0x5ab60d` requires status 1/3, type
2–17, House byte `+0x09` nonzero, and **positive occupants or elite type
11–17**. It then applies a separate source byte `+0x174`: when nonzero,
the target must be elite even when a common house is occupied. Constructor
`0x5ab123–0x5ab144` initializes this byte from the indexed religion condition
helper `0x5a7ea0`; that helper requires a bounded index, a non-null entry,
then obtains entry byte `+0x16` through `0x5aa960`. The producers and
gameplay cause of that condition are outside the bounded contract. It must
remain an explicit prepared `elite_only` observation, never a guessed
universal rule or a default inferred from the building's name.

**Special shared callback:** actor subtype 79 publishes callback `0x515120`
at `0x4ead05`. This callback checks type 2–17, House byte `+0x09` nonzero,
and positive occupants or elite type. It does **not** itself call the
status helper. Its dispatcher and route selection remain outside this task.
It assigns the following nine-timer effects from actor byte `+0x13`:

| Shared actor role | Service timer effects | Religion indices considered |
|---:|---|---|
| 0 | Music 96 | Ancestor |
| 1 | Herbalist 96 | Ancestor |
| 2 | Acupuncture 96 | Ancestor |
| 3–5 | None of the five service timers | Daoist |
| 6–7 | None of the five service timers | Buddhist |
| 8 | **Acrobat 96 and Drama 96 together** | Buddhist and Daoist |
| 9 | Music 96 | Confucian and Ancestor |
| 10–11 | None of the five service timers | Confucian |

The first religion index comes from mapper `0x510c20` and its checked tables;
roles 8 and 9 add indices 1 and 0 respectively. Each indexed religion effect
independently applies the same `0x5a7ea0` elite-only condition. One religion
may therefore be refused while another timer or service is granted. This
table records only the nine timers: the callback also has goods, health,
accounting and other effects which are excluded from OE-05.

A normalized `VisitAction` is consequently a **resolved timer effect**, not
an original actor-role identity or a complete execution of subtype 79. An
explicit ordered sequence can represent the supported projection of a
resolved multi-effect visit; neither one Music action nor the paired
Acrobat/Drama action alone reproduces all effects of the corresponding
original actor role. No timer grant discovers a road, route, target, nearby
building or actual arrival on its own.

## Countdown and expiry

Own-info method `0x517280–0x517329` processes the five service bytes with an
unsigned comparison against 1: values 0 and 1 become zero, and values 2–255
decrement by one. It then processes the four religion bytes `+0x0d–+0x10`,
decrementing each nonzero unsigned byte once. For the nine scoped fields
both forms are precisely `max(value - 1, 0)` on the unsigned-byte domain.

| Input byte | After one eligible countdown |
|---:|---:|
| 0 | 0 |
| 1 | 0 |
| 39 | 38 |
| 40 | 39 |
| 95 | 94 |
| 96 | 95 |
| 255 | 254 |

The method also decrements own-info fields `+0x32`, `+0x34` and `+0x33`;
these are outside the nine-role kernel. It does not read occupancy, type,
stocks, proximity or a source building. Manager caller `0x517b40–0x517bb1`
iterates its existing House collection, admits status 1/3 and nonzero House
byte `+0x09`, then calls the method through the own-info pointer. It has no
positive-occupancy gate: an otherwise eligible empty House still loses time.
The pure raw countdown primitive and the gated House-cycle helper are
separate operations so this caller boundary stays visible.

Coverage means the current byte is nonzero, matching the actual requirement
consumer. With no further visit, a fresh service grant remains nonzero after
95 eligible countdowns and expires on the 96th; a fresh religion grant
expires on the 40th. A failed or absent visit does not renew anything. A
skipped House countdown retains the incoming bytes. These are eligible
countdown counts, not elapsed seconds, months, or OpenEmperor ticks.

## Proven order and remaining limits

Scheduler `0x4ac2e4–0x4ac2f9` dispatches using its stored phase minus one.
The independently checked table entry for **phase 35** selects
`0x4ac45e`, which calls countdown manager `0x517b40`. The entry for
**phase 39** selects `0x4ac48f`, which calls evolution manager `0x519120`.
That manager uses the checked House virtual `+0x200 → 0x518ef0`; the latter
dispatches the per-type handlers documented in
[OE-04](original-house-evolution-contract.md). Thus a timer at one expires
in the countdown phase before a later evolution evaluation reads service
presence, unless an intervening grant changes the observation.

`0x4ac4c8–0x4ac4e7` increments the phase and wraps at 51. This proves a
bounded phase order in that scheduler. It does not prove a mapping to wall
time, current `World::tick`, or the fork's broader month/year schedule.
Other work occurs between phases 35 and 39; OE-05 does not establish every
possible arrival's ordering or build a replacement scheduler. Tests compose
explicit countdown/evolution calls and identify the assumed event order.

The fork's 96/40 scalar assignments and nonnegative ordinary countdown agree
with these bounded observations. Its unified occupied-or-elite predicate
omits status and byte-09 gates and overgeneralizes the elite exception to
ordinary services. Its generic service visit does not model the special
paired and conditional multi-effect path. The kernel keeps these differences
explicit. Existing City-v8 Service, World rules, navigation, renderers and
save schemas remain outside the integration scope.
