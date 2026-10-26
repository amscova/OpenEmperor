# Original desirability: the separate source-link word (OE-12)

This resolves the archive-location question left open by the bounded
[source-admission study](original-desirability-source-admission.md). The source
manager's runtime word `+0x3c` is separate from Subindex and participates in
original building-manager links. **OE-12 and Gate B remain BLOCKED.** No parser,
World state, source purchase, funding, save schema or restore permission changes
follow from these static observations.

## Source and reproduction

**EXE-OBSERVED:** Static virtual addresses below refer to the read-only local
`Emperor.exe`, SHA-256
`6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e`,
PE image base `0x400000`. The executable was not run. Original bytes and private
disassemblies stay outside Git under
`.local/original-economy/oe12/source-state-research/`.

From the repository root:

```sh
python3 .local/original-economy/oe12/source-state-research/root/check_source_state.py
```

The private `root/source-state-evidence.json` reports **PASS** for nine bounded
windows, 157 instructions and 519 instruction bytes checked against the PE
sections, plus 32 concrete opcode/call assertions; zero byte mismatches. Source
bytes, modification time and file mode remain unchanged. Separate independent
archive and writer audits cover the additional paths below:

```sh
python3 .local/original-economy/oe12/source-state-research/save-audit/check_archive_word.py
python3 .local/original-economy/oe12/source-state-research/state-audit/check_building_word3c.py
python3 .local/original-economy/oe12/source-state-research/roundtrip-audit/check_archive_mapping.py
```

The archive checker verifies 15 windows/962 instructions/2508 bytes, 30 explicit
assertions and four identical 32-field prefixes. The writer checker verifies
eight windows/456 instructions/1480 bytes and 40 assertions. The independent
archive cross-check verifies eight windows/681 instructions/1802 bytes and
the same four prefixes. These counts overlap; they are not unique combined
coverage. All four checks pass without original execution. None is a gameplay,
OE-12 feature or Gate-B acceptance result.

## A distinct archived word

The checked `Building::Serialize` function at `0x427430` serializes the word
at runtime `+0x3c` as two bytes, independently of runtime `+0x16` (Subindex).
The cumulative field widths include the two-byte Base schema word. Offsets in
this table are decimal byte offsets from that schema word, not runtime members
or MFC record offsets:

| Field | Runtime offset | Raw Base offset | Width |
| --- | --- | --- | --- |
| Existing parsed Subindex | `+0x16` | 18 | 2 |
| Source manager's additional word | `+0x3c` | 54 | 2 |

The writer passes `+0x3c` and length2 to `0x780642` at `0x427601`.
The schema5/4/3 read branches pass that same member and length2 to `0x780533`
at `0x427e5b`, `0x4282ad` and `0x42870a`, respectively. Their complete earlier
field prefixes establish the same raw Base offset54; this is not calculated
by subtracting a constant from a runtime address. The current
[map parser](../../src/maps/OriginalMapEntities.cpp#L245) reads Subindex18 but
does not expose this separate word. No parser field is added by this research.

## Initialization and replacement preserve different facts

The checked Base constructor calls reset `0x426e60` at `0x426cc2`. Reset clears
45 DWORDs starting at runtime `+0x04`, covering the half-open range
`[+0x04,+0xb8)`, including both link words. This establishes initial zero for
this constructor path, not a permanent invariant.

In the separate manager factory `0x42d540`, two local link temporaries start0
at `0x42d555–0x42d55d`. The explicit inactive-slot replacement path instead
reads the old object's signed words `+0x3c`/`+0x3e` into those temporaries at
`0x42d5b9–0x42d5cd`. After constructing and publishing the replacement, assigning
its manager ID to runtime `+0xb4` and invoking initializer slot `+0x94`, the
factory restores those link words at `0x42d73c–0x42d74e`. Thus initial zero does
not justify treating every subsequently created or restored object as zero.

## Concrete manager-link use

Lookup `0x47f1b0–0x47f1bf` calls getter `0x4f8210–0x4f8214` and returns
`[this+4] + 4*index`; its caller dereferences the slot. With `this=0x8c7634`,
the following paths use the same building-manager vector. The lookup itself
contains no bounds, status or type checks.

| Checked path | Observation |
| --- | --- |
| `0x4af880–0x4af8b7` | Obtain a manager object by index; while its signed word `+0x3c` is positive, use that word as the next manager index. This loop caps its counter at500. |
| `0x58a3c7–0x58a40d` | A second path repeatedly follows the signed `+0x3c` manager index until the selected object's word is0. The first loop's cap is not inferred for this path. |
| Writer within `0x563850`, at `0x563a08` | After creating another building through `0x42d540`, store the original manager ID into that new object's `+0x3c`; link the previous object's `+0x3e` to the new ID and terminate the new `+0x3e` with0. |

These data flows establish a manager-ID link role in these concrete paths.
**INFERENCE:** They support a root/component interpretation, but do not establish
a complete universal parent/child contract, all participating types, legal link values,
cycles, placement costs, lifecycle transitions or post-load repair semantics.
The adjacent link is not the archive wrapper schema named `parent` in existing
map-parser provenance either.

## Consequence for influence-source admission

The checked source manager at `0x44f22b` requires this exact word to equal0
after its status1/3 predicate and before reading the four Building model fields.
Thus Subindex0 cannot stand in for the separate link-word predicate, and a
constructor's initial zero cannot replace the object's actual state. Nor does
zero alone establish every other source condition or a safe model index.

This audit imports no original state into the frozen Rule7 Vacant container;
its status0 and strict restore contract remain exact. The seven functional
World→Model analogies remain unverified. Later source/build/restore/budget
permissions require the separate [runtime contracts](../original-economy/architecture-map.md).
These findings establish neither original-game execution nor human acceptance.
