# Original service and religion dispatch: bounded static evidence

OE-05 isolates a staffing threshold and a byte-counter transition. A counter
becoming due means that an actor creation attempt may follow; it does not prove
that an actor exists, walks a road, visits a House or grants coverage. No
`World::tick` duration or scheduling frequency is established by these functions.
The separate [visit and timer contract](original-service-timer-contract.md)
covers actual House grants and countdowns.

## Read-only source and repeatable checks

The source is the local `Emperor.exe`, SHA-256
`6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e`.
All addresses below are static virtual addresses in the PE32 image based at
`0x400000`, neither RVA, file offsets nor relocated runtime addresses.

The private check
`python3 .local/original-economy/oe05/spawn-check-evidence.py` maps each
instruction through the original PE section to its file bytes. It verifies
**17 bounded windows, 524 instructions and 1,538 instruction bytes**, five
threshold functions, eight RTTI/vtable/constructor bindings and all six typed
religion factory branches. There were zero byte mismatches, and the executable
was byte-identical before and after. The ignored report is
`.local/original-economy/oe05/spawn-exe-evidence.json`; raw bytes and
disassemblies remain outside Git and application resources. These are static
checks; the original executable was not run.

An independent bounded inspection of the shared counter is:

```sh
objdump -d --start-address=0x51cf90 --stop-address=0x51d09f .local/gog-extracted/app/Emperor.exe
```

## Thresholds belong to concrete building classes

Each threshold takes a **signed 32-bit staffing percentage**. Comparisons are
inclusive at 100, 75, 50, 25 and 1. Negative arguments use the same final row
as zero; values above 100 use the first row. The result is a comparison
threshold, so a newly zeroed enabled counter needs `threshold + 1` eligible
calls to become due.

| Source building | Threshold function | >=100 | 75–99 | 50–74 | 25–49 | 1–24 | <=0 |
|---|---|---:|---:|---:|---:|---:|---:|
| Herbalist's Stall | `0x507e40` | 1 | 3 | 5 | 10 | 15 | 64 |
| Acupuncturist's Clinic | `0x51cf40` | 1 | 3 | 7 | 15 | 29 | 64 |
| Music School, Acrobat School | `0x5ab330` | 3 | 6 | 12 | 24 | 32 | 64 |
| Drama School | `0x48b380` | 6 | 12 | 24 | 32 | 48 | 96 |
| Ancestral Shrine, Daoist Shrine, Buddhist Shrine | `0x5ab330` | 3 | 6 | 12 | 24 | 32 | 64 |
| Daoist Temple, Buddhist Pagoda, Confucian Academy | `0x5ab6f0` | 6 | 12 | 24 | 32 | 48 | 64 |

The final row is queryable but **does not cause unstaffed dispatch**. The common
counter caller returns early for nonpositive effective staffing before reading
or updating its counter.

The following independent bindings avoid assigning a generic function to a
class merely because the function exists nearby. MSVC RTTI names identify the
tables, constructors publish those same tables, and their virtual slots bind
both the threshold at `+0x230` and the shared counter at `+0x234`.

| RTTI class | Vtable | Threshold | Constructor publication |
|---|---|---|---|
| `cHerbalistBldg` | `0x7b6114` | `0x507e40` | `0x51c0b8` |
| `cAcupuncturistBldg` | `0x7b6374` | `0x51cf40` | `0x51c0d8` |
| `cMusicSchool` | `0x7acedc` | `0x5ab330` | `0x48a8e8` |
| `cAcrobatSchool` | `0x7ad140` | `0x5ab330` | `0x48a908` |
| `cDramaSchool` | `0x7ad3a4` | `0x48b380` | `0x48a928` |
| `cReligiousBldg` | `0x7bc0b4` | `0x5ab330` | `0x5ab10f` |
| `cGoodReligiousBldg` | `0x7bc584` | `0x5ab6f0` | `0x5abf59` |
| `cConfucianAcademy` | `0x7bc318` | `0x5ab6f0` | `0x5ab877` |

The factory beginning `0x5aae70` accepts types 214 through 219 through the
six-entry jump table at `0x5ab018`. Types 214/215/217 use the ordinary religious
constructor `0x5ab0d0`; 216/218 use `0x5abf40`; 219 uses `0x5ab860`, which
publishes its Confucian table after the good-religious constructor. Their
model labels independently identify the shrine/temple/pagoda distinction.
Consequently a Daoist or Buddhist visit role alone is insufficient to choose
its source building's dispatch threshold.

## Shared counter order and width

`0x51cf90–0x51d09e` performs these operations in order:

1. It evaluates existing actor/state and source predicates. Returning from any
   of these gates preserves the counter.
2. It obtains effective staffing through virtual `+0x1bc`. A signed result
   `<= 0` returns without changing the counter.
3. It calls the class's `+0x230` threshold function with that staffing result.
4. It increments the **unsigned byte** at building `+0x36`, stores the byte,
   then zero-extends it for comparison. Thus old 255 becomes new 0.
5. It becomes due only when the **new value is strictly greater** than the
   threshold. Equality is not due. A due counter is reset to zero.
6. It attempts actor creation, then conditionally attaches and initializes a
   successfully created actor. A failed factory still leaves the counter
   reset, and the helper returns true after the attempt.

The decisive increment/mask/compare window is `0x51cffe–0x51d00f`; the reset
is `0x51d01e`, before the factory call at `0x51d03a`. Factory failure branches
from `0x51d044` directly to the true return at `0x51d098`. A pure result named
`due` or `attempt_due` may reproduce this transition. Naming it a successful
spawn or granting a House visit there would add unsupported authority.

An enabled full-staff Herbalist starting at zero is due on its second eligible
call. A full-staff ordinary shrine is due on its fourth. Neither count denotes
seconds, frames, monthly updates or production `World::tick` calls.

## Effective staffing and external gates

The ordinary classes delegate through `0x416b60`, and religion through
`0x5ab380`, to virtual `+0x1b4`. The bound helper `0x428ed0` returns zero
when the required-worker result is nonpositive; otherwise it uses signed
division of `100 * allocated_workers / required_workers`, truncating toward
zero on the ordinary safe arithmetic domain. OE-05 receives this already
prepared percentage; it does not create a second workforce allocator or
emulate edited values that overflow the original multiplication.

Confucian Academy's `+0x1bc` instead binds `0x508160`: a nonpositive signed
word at building `+0x40` forces zero effective staffing before the same
percentage helper is used. This is an additional source prerequisite, not
evidence that an arbitrary nearby or occupied House receives religion. Other
early predicates, actor availability, routing and source resource state stay
external to the pure transition. An explicit caller-supplied prerequisites
observation records that boundary without pretending to reconstruct them.

## Differences from the study fork and acceptance limits

The pinned study fork `d861d2912337c09751cb68196f85d5a30752726e`
provides a single `service_spawn_interval` table and one
`religion_spawn_interval` table. Those tables occur in the original, but they
do not describe all the requested building classes: Herbalist, the three
entertainment schools and the stronger religion sources need the distinctions
above. The fork's unbounded signed integer counter also omits the original
byte wrap and the no-staffing freeze performed by the caller.

OE-05's pure functions must keep threshold selection, gated counter update,
actor creation and House grant as separate operations. They introduce no
gratis service from proximity, new actor scheduler, World rule, transport,
economy, frame work, migration or save field. Existing City-v8 and later
authored Service profiles remain authoritative and unchanged. Tests of the
isolated functions establish the bounded transitions documented here; they do
not establish original full-game timing or a playable original supply loop.
