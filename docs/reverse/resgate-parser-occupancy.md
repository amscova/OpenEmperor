# cResGate: bounded parsing and protected occupancy (2026-10-11)

The later [bounded MP34 Industrial179/164 pass](mp34-industrial179-164-occupancy.md) resolves the admitted saved point-source states after independent evidence for each type. MP34 now passes complete policy preparation and normal rule4/policy2 Empty City start. The current corpus prepares130/167 policies; the historical counts and Industrial refusal statements below retain their original scope. ResWall/ResGate parsing, physical protection, presentation limits and absence of a new ResGate passage are unchanged.

The later [MP56 presentation fix](../rendering/mp56-original-structure-fallbacks.md)
resolves MP56's saved General417 image and an independent Xiangjun variant
through historical singleton registration. This does not change occupancy,
create a ResGate passage or establish original restored graphic selection.

This pass starts from the actual clean HEAD **`4d8f5c75e3e4b6b73663150c8207af72efdf0ef8`**, matching the requested reference. It adds exact append-only ResGate parsing and independently evidenced singleton protection. City-v16 **rule4 / policy2 / schema20**, older rules/policy1, ResWall, GateHouse passages, Tower bodies/joins, roads, terrain/heights, renderer/input, World/economy and save/recovery implementations retain their contracts. There is no migration, commit or publication.

**Result:** all four former cResGate parser failures now yield complete managers. The same 167-map corpus prepares **104 policies**, up from 102, with **63 refusals**. **Xiangjun and STGW-Xiangjun** pass normal Empty City menu start, paid production, F5/F9 and process restart. Chang-an Sui and MP56 retain independent bounds/Industrial blockers. **cResGate occupancy supported; passage not implemented.** ResGate body rendering remains unsupported.

The later [Industrial177/178 occupancy pass](mp56-industrial177-178-occupancy.md) resolves MP56's saved point blockers and raises the current census to 121 prepared/46 refused, retaining all 104 policies from this historical pass byte-exact. Its MP56 session evidence and newly exposed MP27 bounds refusal are separate from the ResGate results below. ResGate parsing/protection, unsupported passage/body and the historical 104/63 measurements remain unchanged.

## Baseline and immutable inputs

The fresh data inventory is **1,464 files / 819,711,091 bytes**, manifest SHA-256 **`dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`**. Encoding is sorted UTF-8 relative path, TAB decimal size, TAB file SHA-256, LF. Baseline/final inventories are equal. The original PE32 EXE is read only, never executed: **4,809,081 bytes**, SHA-256 **`6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e`**, freshly verified before analysis.

Before editing, the existing audit is rebuilt and run against actual originals. Frozen executable/libraries and fresh reports remain under ignored `.local/resgate/frozen-baseline/` and `.local/resgate/baseline-corpus/`; later outputs never overwrite them. All disassembly, source records, generated saves, observers and original-data diagnostics stay under ignored `.local/resgate/`. No original bytes or pixels enter repository fixtures.

**RAW MAP FACT:** each file below is independently identified as a standalone **Map** part. Manager schema1 starts at decompressed logical **1,093,607**. All four first errors are exact unsupported MFC class `cResGate`; no full count is inferred behind that failure. Indices are zero-based. Chang-an Sui is a different file from Chang-an Zhou.

| Exact relative path | Bytes | Size / border | Old first index / logical offset | File SHA-256 |
| --- | ---: | --- | --- | --- |
| `Cities/Chang-an Sui.map` | 133,748 | 226 / 1 | 20 / 1,099,813 | `98262361fc03f1e9cfee774afde576617ab7e30a34ff50ae26db402f91af9f71` |
| `Cities/MP56.map` | 102,064 | 170 / 29 | 21 / 1,100,176 | `96e3bd832b40ae57a6e7526eb864cfa85a5bacd4dc3cad95aa7e4fff8dc17892` |
| `Cities/STGW-Xiangjun.map` | 156,211 | 140 / 44 | 29 / 1,102,633 | `5b3742fdf598345448277080a5e0864f0dfa1609f038ab5fed43da6661f864bb` |
| `Cities/Xiangjun.map` | 93,522 | 140 / 44 | 28 / 1,102,269 | `ed1389f93a238fb39a2e4e835cc6ab1e67c0830de90b4984a04a4639748f746b` |

Baseline: **163 complete managers**, 102 prepared policies, 58 unknown-occupancy first failures, three known conflicts, four cResGate parser failures. MP34 already has a complete manager after the preceding [ResWall pass](reswall-parser-occupancy.md); its Industrial179/164 refusal remains outside this task.

## Serializer evidence and address spaces

Static VA, RVA, PE file offset, original runtime member offset and decompressed map position are distinct. Translation uses the inspected PE sections and image base `0x400000`. Twenty-six fresh bounded disassembly windows contain **2,807 disassembly lines / 7,479 instruction bytes**, all checked against the pinned file with zero byte mismatches. Table entries and vtable pointers are read independently from original bytes.

| Observation | Static VA | RVA | PE file offset |
| --- | --- | --- | --- |
| ResGate MFC descriptor | `0x815f68` | `0x415f68` | `0x414568` |
| Exact name | `0x815f80` | `0x415f80` | `0x414580` |
| Saved factory | `0x414250` | `0x14250` | `0x13650` |
| Final constructor | `0x416d00` | `0x16d00` | `0x16100` |
| Final vtable | `0x7aafb0` | `0x3aafb0` | `0x3aa1b0` |
| Effective serializer | `0x416490` | `0x16490` | `0x15890` |
| Own-wrapper initializer | `0x414210` | `0x14210` | `0x13610` |
| NonHouse parent serializer | `0x51ce00` | `0x11ce00` | `0x11c200` |
| Building serializer | `0x427430` | `0x27430` | `0x26830` |
| Inline NonHouseInfo serializer | `0x51c3a0` | `0x11c3a0` | `0x11b7a0` |
| Type105 / Type106 entries | `0x823f68` / `0x823f80` | `0x423f68` / `0x423f80` | `0x422568` / `0x422580` |
| Standalone physical restore | `0x52f030` | `0x12f030` | `0x12e430` |
| Physical callback | `0x414830` | `0x14830` | `0x13c30` |
| Rectangle owner writer | `0x4b72b0` | `0xb72b0` | `0xb66b0` |

**EXE-OBSERVED:** descriptor `815f68` registers exact name `cResGate`, MFC class schema **0**, runtime size **0x150**, factory `414250` and parent descriptor `854438` (`cIndustrialBldg`). Runtime size is not serialized length. Factory/constructors **414250 → 416d00 → 416cd0 → 414670 → 51c9a0** finish with vtable `7aafb0`; slot **+8** selects **416490**. The actual serializer calls **51ce00 directly**, then `41fc10` on its own schema object `88f6f0`. Initializer `414210` supplies own-wrapper **1**. That schema object lies in virtual zero-filled storage, without a PE file byte. Intermediate constructor serializer `414690` adds no section to this effective stream.

Parent **51ce00 → 427430** writes/reads Building, then parent wrapper1 and the inline object at runtime **+0xc8**. Its `7b65d4` vtable selects **51c3a0**. This is fixed-width inline state, without nested MFC references or counts. ResGate's own wrapper adds no payload. Neither the inheritance descriptor nor an intermediate constructor is used to invent an extra wrapper.

| Stream section | Width | Contract |
| --- | ---: | --- |
| Manager header | 6 | u16 schema1, signed i32 count 0–4,000 |
| New ResGate MFC declaration | 14 | u16 `ffff`, u16 class schema0, u16 length8, exact eight-byte name |
| Reused class tag | 2 | Shared-table class index OR `8000`; fresh object |
| Building base, including schema word | 179 / 181 / 182 | Base3 / Base4 / Base5 |
| Parent wrapper | 2 | Schema1 |
| NonHouseInfo, including schema word | 128 | Schema1 + 126 opaque bytes |
| ResGate wrapper | 2 | Schema1, zero further payload |

NonHouseInfo payload widths are independently summed from runtime ranges: `+04`×2, `+06`×72, `+4e`×14, `+5c…+65` ten bytes, `+66`×2, `+68/+69` two bytes, `+6a/+6c/+6e` three two-byte values, `+70`×16, `+80`×2: **126 bytes**. Their meanings/signs remain uninterpreted and never select geometry. Existing Building schema branches and tail remain exact: status u8 at base+2, saved side u8 at +5, local x/y signed i16 at +8/+10, cell reference signed i32 at +12, type/subindex signed i16 at +16/+18, original ID signed i32 at **base_length−20**. ResGate does not type the opaque Building layout bytes as GateHouse state.

Payload after the tag is **311 / 313 / 314** bytes; a new-class record is **325 / 327 / 328**, a reused-class record **313 / 315 / 316**, respectively. Parent/state/own provenance stays separate. No additional fields are needed. The existing 4,000-record, byte-length and reference budgets remain unchanged.

## Prefix boundaries and atomic parser tests

ResGate is appended as enum **8**, after unchanged Tower6/ResWall7. Only exact `cResGate` is admitted; the alias `cResGateAlias` remains unsupported. Shared MFC class/object indices retain their existing meaning: a new declaration adds class then object; a class reference adds only object. Nulls, object aliases, out-of-table indices and class references to objects remain errors. There are no hidden nested entries.

Independent authored streams assert literal widths and intact following IDs/coordinates/types/reference indices:

- Building3 → new ResGate4 → referenced Building5: **710 bytes**, record offsets 199/526, final object reference5.
- ResWall3 → new ResGate4 → Tower5: **984 bytes**, ResGate record327, final reference6.
- ResGate3 → referenced ResGate4 → GateHouse5/wrapper2/layout1: **984 bytes**, reused record315, GateHouse offset646/reference5.
- GateHouse3/wrapper2 → ResGate4 → Industrial5: **1,001 bytes**, ResGate offset341, final reference6.

Each mixed prefix leaves an appended independent `de ad be ef` section unchanged and unconsumed. Active and inactive Base3/4/5 records consume their entire parent/state/own stream. Every single-record byte-prefix truncation is rejected. Unsupported base/MFC/parent/state/own schemas retain `OriginalEntityUnsupported`; truncation, signed counts, invalid/duplicate active IDs, coordinates, cell references, aliases and arithmetic overflow retain `OriginalEntityError`. An unknown following synthetic class rejects the entire call, never returning a partial collection. Previous unknown-class negative tests remain present.

## Bounded protected occupancy

**EXE-OBSERVED:** standalone `52f030` checks nonzero saved status. Filter `52f1d0` indexes type−83 through byte table `52f1fc` and targets `52f1f4`; **105/106 select true at 52f1ed**. The 24-byte model entries begin at `823590 + 24×type`; their **side1 field at +8** is read through `823598 + 24×type` at `52f09a`, independently of saved graphic IDs.

Runtime object creation **42d540 → 42d360 → 51c660** reaches **4142e0 → 4143b0**. The earlier House/Monument/type94 and NonHouse factory predicates are independently false for105/106 (bounded comparisons and the562f70/51be30 switch tables), so this is the selected runtime path. Both 105/106 select admission true at `4142fd`; factory byte table `4145c0` (type−89) / targets `41459c` selects branch **41441d → constructor416d00**. Its actual vtable **+0x100** is **414830**. Restore calls **4b11f0 → virtual+100 → 414830**, which passes the same side as width and height at `41487c/41487d` to **4b72b0 at 414881**. The rectangle loop writes every physical owner to runtime grid **fc3750 at 4b73ce**, and terrain to **f6a9e0 at 4b73c2**. This proves the complete **1×1 physical claim**, beyond a drawing bounding box.

**OPENEMPEROR CONSERVATIVE POLICY:** before generic positive-square handling, admit only **ResGate / status3 / type105 or106 / subindex0 / side1 / Base3 or4 / parent1 / NonHouseInfo1 / own1**, without Monument/GateHouse typed state. Existing identities, distinct manager indices, signed origins, exact storage reference, actual diamond/off-map boundaries and foreign physical/marker/gate conflicts remain mandatory and atomic. Base5 is structurally parseable but has no active occupancy admission. Other status/type/state/side variants, including zero and positive side2, refuse the complete policy. Raw Objects is not interpreted as runtime owner ID.

An admitted cell blocks House and Road construction, and policy2 cannot import a Road bit beneath it. Free neighbors retain their existing rules. No FixedGatePassage, corridor/opening, height transition, military/NPC behavior, body or join is added. Passage/routing/graphics of ResGate remain **UNRESOLVED**; GateHouse type130/layout0/1 is a separate exact-class producer.

Synthetic cases cover both policies, raw Road bits, signed height−1, unknown states, positive side2 and zero side, ID/reference/bounds errors, foreign overlaps in both orders, GateHouse solids/corridors/openings and inactive-record preservation. Parser, permission, preflight, presentation ownership and persistence consumers were audited: the old contiguous class guard remains bounded through ResWall, with explicit ResGate admission; Gate/Tower/Monument helpers still test exact classes. ResGate is a foreign **Other** owner for Tower conflict handling, never a new Tower/GateHouse/body/join. No enum metadata is added to save fields.

## Complete original records

**RAW MAP FACT:** complete parsing finds exactly **one ResGate per affected map**, three active and one inactive. The table is obtained only after full manager parsing. All four have parent/state/own schema1 and subindex0; no class-specific geometry/parent payload is inferred.

| Map | ID / MFC object | Base / record bytes | Status / type / side | Local → storage / reference | Terrain / Objects / signed height |
| --- | --- | --- | --- | --- | --- |
| Chang-an Sui | 20 / 26 | 3 / 325 | 3 / 106 / 1 | (126,70) → (127,71) / 16,315 | `c8` / 4 / 0 |
| MP56 | 21 / 27 | 4 / 327 | 3 / 105 / 1 | (82,67) → (111,96) / 21,999 | `c8` / 4 / 1 |
| STGW-Xiangjun | −1 / 34 | 4 / 327 | 0 / 0 / 0 | (0,0) → (44,44) / 0 | 67,633,157 / 0 / 0 |
| Xiangjun | 28 / 32 | 3 / 325 | 3 / 105 / 1 | (93,80) → (137,124) / 28,409 | `c8` / 4 / 0 |

Record/base/state/own-wrapper logical positions:

- Sui: **1,099,813 / 1,099,827 / 1,100,008 / 1,100,136**.
- MP56: **1,100,176 / 1,100,190 / 1,100,373 / 1,100,501**.
- STGW-Xiangjun: **1,102,633 / 1,102,647 / 1,102,830 / 1,102,958**.
- Xiangjun: **1,102,269 / 1,102,283 / 1,102,464 / 1,102,592**.

STGW's inactive record is fully read, contributes no typed physical claim and does not prove active zero-side support. Existing raw terrain protection remains independent. All three active ResGate claims individually validate; this source diagnosis never publishes a subset policy for a refused map.

## Corpus and compatibility

| First complete preparation result | Frozen baseline | Final |
| --- | ---: | ---: |
| Prepared full policy | 102 | **104** |
| Unknown occupancy | 58 | **59** |
| Known conflict | 3 | **3** |
| Outside-map original footprint | 0 | **1** |
| cResGate parser failure | 4 | **0** |
| Complete managers | 163 | **167** |
| Refused maps | 65 | **63** |

Every affected manager has **4,000 records**. Class counts are Building / Industrial / GateHouse / Tower / ResWall / ResGate:

| Map | Class counts | Manager bytes / logical end | Actual next stage |
| --- | --- | --- | --- |
| Chang-an Sui | 3,941 / 35 / 3 / 7 / 13 / 1 | 731,827 / 1,825,434 | MANAGER_PARSED; individual ResGate OCCUPANCY_VERIFIED; full check **InputError** |
| MP56 | 3,970 / 9 / 4 / 8 / 8 / 1 | 736,059 / 1,829,666 | MANAGER_PARSED; individual ResGate OCCUPANCY_VERIFIED; full **OCCUPANCY_UNSUPPORTED** |
| STGW-Xiangjun | 3,945 / 24 / 0 / 0 / 30 / 1 | 739,273 / 1,832,880 | MANAGER_PARSED → full MAP_RULES_CHECKED → SESSION_STARTED → GAMEPLAY_SMOKE_PASSED |
| Xiangjun | 3,957 / 0 / 0 / 0 / 42 / 1 | 729,718 / 1,823,325 | MANAGER_PARSED → full OCCUPANCY_VERIFIED → MAP_RULES_CHECKED → SESSION_STARTED → GAMEPLAY_SMOKE_PASSED |

Sui's first remaining error is Industrial162 **ID/index29**, logical **1,102,653**, local (210,96), storage (211,97), reference22,327: the origin is outside the actual map diamond. Four later Industrial188 zero-side records (indices44/45/46/48) are also unsupported. MP56 first refuses Industrial177 **ID/index22**, logical **1,100,503**, local (23,64), storage (52,93), reference21,256, Base4/parent1/state1, terrain `04000004`/Objects0. There are four type177 and one type178 unknowns. No new class appears behind ResGate. MP34 still independently refuses its five Industrial179/three Industrial164, first index22/logical **1,108,481**; its existing 17 ResWall claims are unchanged.

The same **102 previously accepted maps** remain accepted. Actual production, zero and all-one legacy-mask comparisons give **3,366 byte-identical policy1 files** and **1,224 byte-identical policy2 files**: all road/building/protection masks, blockers, signed heights, complete gates/corridors/openings/edges, original-road classifications, canonical bytes and fingerprints. Policy2's old 10,370 imported roads are unchanged; the two added maps each import148, making **10,666 across104 maps**. No policy/save version changes or migration is needed.

All **167 maps** also undergo the actual legacy renderer/mask and **20 supported old profile/rule combinations**, including City11r1–4 and City16r1–2: **3,340 full empty-World save/read/restore roundtrips per build**. Of 7,014 compared files, **7,012 are byte-identical**; every legacy mask, complete World canonical and SaveDocument remains equal. The two differences are presentation-resource metadata for Sui and MP56, explained below. Those legacy contracts never authorize a rule4 start on a refused map.

### Presentation consequences of complete parsing

No ResGate graphic producer exists. Previously incomplete managers forced complete GateHouse/Tower historical fallback; the new complete managers make existing exact-class producers available. Preparation/activation counts are measured independently of actual visible pixel or human acceptance:

| Map | New GateHouse sources / active components | Tower sources / active bodies / active joins | Decoded assets before → after | Uploads before → after | RGBA bytes before → after |
| --- | --- | --- | --- | --- | --- |
| Sui | 3 / 45 | 7 / 6 / 28 | 527 → 566 | 1,581 → 1,682 | 26,355,312 → 30,232,684 |
| MP56 | 4 / 60 | 8 / 6 / 33 | 476 → 515 | 1,428 → 1,529 | 24,708,504 → 28,585,876 |

Baseline source/activation counts are zero with named atomic parser fallback. Both prepare 39 additional physical General records879–917, within the existing texture budget. Xiangjun/STGW have zero GateHouse/Tower sources or activations and byte-identical presentation metadata. All other 165 maps' metadata is also equal. Existing body/join tests and the old accepted scenes remain unchanged; these newly enabled Sui/MP56 producers may affect their preview pixels, without admitting gameplay, adding ResGate art or proving whole-scene parity.

## Normal App, gameplay and persistence evidence

Owned dummy/software processes run the unchanged current Application/main, with an ignored read-only observer delivering normal SDL keys/clicks. No World commands/snapshots, funds, goods or workers are injected. Each uses **New Sandbox → City16 rule4 → exact catalog map → Map rules checked → Empty City → Start**. This is **scripted App evidence**, separate from native/human input.

In Xiangjun, storage **(137,124)** rejects both House and Road, reporting OriginalStructure with complete World unchanged and no passage. The adjacent free diagonal **(136,125)** accepts a paid2 Road and normal removal without refund. The protected cell remains unchanged. STGW has no active ResGate, so no active ResGate construction witness is claimed there.

Both maps place two Houses at **(112,128)/(115,128)**, Clay at **(112,131)** and Pottery at **(118,131)** through ordinary paid UI actions. The existing original road at y130 supplies the route; a PlaceRoad gesture on (112,130) reports **“Original road already available.”**, changes no command/funds and requires no new connecting road. Construction is **462 / 460** respectively, including Xiangjun's temporary neighboring Road. Workforce supplies12 and assigns10. Over600 ordinary Period-key ticks, Clay extracts18, the Clay courier dispatches/carries/delivers/returns through phases0/1/2, and Pottery completes6. Final funds **818 / 820** include20 normal maintenance; no tax is claimed. This mini-city lacks Food/Service/Water/Health coverage and is not long-term economic acceptance.

At actual ticks **5 and605**, normal F5/F9/F5 compares complete typed WorldSnapshots and full serialized SaveDocuments. Separate processes load each tick5 save through normal Load Sandbox and repeat the same600 ticks, producing exactly the same tick605 snapshots/context and entire JSON. An independent headless World restore/replay also writes the same full SaveDocument. Wrong original-map identity and altered policy fingerprint remain explicit restore errors. Production render observers record zero deltas in all twelve prohibited-work counters; prepared resources remain shared and owned textures return to zero on exit.

A separate normal-menu run starts a working paused Xia, then selects Sui, MP34 and MP56. Sui reports **InputError / active original footprint outside map**; the others **UnsupportedForRules / Industrial179 or177**. Enter cannot start them. Previous full World/context/policy pointer and owned persistent files remain equal; Resume succeeds. No legacy fallback is used.

The existing technical Xia rule3/policy1/schema19 playthrough still reaches its earned goal at6800 and sustains it to9200 (ten Houses, eight effectiveL2,126 residents). All **14 complete earned SaveDocuments** equal the preceding ResWall control. This is technical deterministic control evidence, not human acceptance on the two new maps.

## Tests and performance

Focused Release CTest passes **5/5**: originalmapentities, sandbox-map-permissions, map-rules-selected-service, menu-map-rules-selection and ordinary-tower-selector. Authored production-menu fixtures pass valid ResGate Start and reject later Industrial179/truncated wrapper atomically, retaining full previous World/policy/files. Existing ResWall cases remain present.

The first Debug build succeeds but the strict warning gate stops before CTest on a new test's unsigned-to-u16 conversion. The explicit bounded test cast fixes it; the original log/report remains in `.local/resgate/debug-first-warning/`. Early private observer/planner failures are retained too: a four-orthogonal-only neighbor search missed the valid diagonal; the refusal observer initially expected Unsupported instead of the actual bounds InputError. These instrumentation assumptions are corrected without production changes. The first Xia invocation omitted its required fresh existing directory and is retained separately from the corrected successful run.

The three full configurations run sequentially and each passes **145/145 tests**: Debug CTest **1,637.51s**, Release **64.10s**, combined ASan+UBSan **305.64s**. Final builds have zero compiler warnings; there are zero failed or skipped tests, no sanitizer diagnostics and no timeout changes. Test-source before/after manifests are identical within each configuration and identify the actual code under test. Full logs, JUnit results and per-configuration reports remain under `.local/resgate/`; the first warning-gated run above is preserved separately.

A Release authored inactive4,000-record manager is **1,260,018 bytes**. One hundred valid parses take **0.0117s**, last-wrapper truncations0.0130s, unknown-last-class rejections0.0127s; both failures return zero records. Combined peak RSS **6,832,128 bytes**. A separate4,000-active-ResGate parse/full-policy benchmark takes **0.0256s for20 complete policies**; last-side2 refusal0.0185s and last-wrapper truncation0.00268s for20 each, publishing zero policies. Peak RSS **8,945,664 bytes**. These are local macOS arm64 measurements, not portable speed guarantees. No limits increase, recovery search, per-frame/tick parsing/I/O/decode/upload/World/BFS work is introduced.

## Test start

From the repository root, the following exact command was built and executed:

```sh
sh tools/test_resgate_maps.sh .local/gog-extracted/app Xiangjun
```

It builds current Release, runs the existing complete headless sandbox check using **rule4/policy2**, then opens the **normal menu** with exact Xiangjun / Empty City in a fresh private root. Choose **New Sandbox**, then **Start Sandbox**. The owned native App launch was observed and closed separately; that short menu-launch check is not native gameplay or human acceptance. The scripted starts/builds/restarts above supply the technical gameplay evidence. `STGW-Xiangjun` is the second independently accepted map.

The requested space-containing counterexample was also executed:

```sh
sh tools/test_resgate_maps.sh .local/gog-extracted/app "Chang-an Sui"
```

It reports **“Sandbox check failed: map permissions: active original footprint outside map”**, exits1 and launches no App. The full audit identifies the diagnosis at index29/logical1,102,653 above. Exact contained regular catalog names are required; names/paths are shell/JSON quoted. No other map, permissive rule, personal settings/save/root or active session is substituted. The prelaunch CLI check also prepares the ordinary paid starter headlessly; the subsequently opened normal menu is explicitly Empty City.

**UNRESOLVED:** ResGate original passage/body/state lifecycle, unknown active states, the remaining63 original-map refusals, whole-corpus rendering/gameplay, complete original composition and human native-input acceptance. This pass stops at bounded parsing/protection and the measured corpus/session effects.
