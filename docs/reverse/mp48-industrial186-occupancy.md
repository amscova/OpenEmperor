# MP48: bounded Industrial186 source-origin protection

This pass starts at clean HEAD **`b6a8176c77aee13423701690c3520a9e1b773764`**, matching the supplied reference. It addresses only the evidenced saved zero-side Industrial186 state. City-v16 **rule4 / MapPermissions policy2 / save schema20**, policy1 and older rules remain unchanged, without migration. No copper industry, resources, editor operation, actor, bridge, ramp, Ferry connection or new visual is implemented. Original files are read only; original bytes, disassembly, captures and private evidence stay ignored under `.local/mp48-occupancy/`. No commit or publication is performed.

**Current technical result:** all167 managers parse; **134 complete policies prepare /33 maps refuse**. MP48, MP41, MP50 and STGW-Shu newly prepare. The preceding130 policies, legacy masks/Worlds/saves and presentation metadata remain unchanged. Normal MP48 paid production, in-flight F5/advance/F9 and a separate process continuation pass. Final configuration results are recorded below; native/human gameplay and complete original behavior remain separate.

## Fresh baseline and original revision

The fresh production-reader baseline measures **167 complete managers /130 policies /37 refusals**:32 unknown-occupancy first failures, three known conflicts and two existing outside-map footprints. The same complete corpus and inputs are audited before and after the change. These are fresh measurements, separate from the historical [Industrial179/164 pass](mp34-industrial179-164-occupancy.md).

The original tree contains **1,464 files /819,711,091 bytes**. Its sorted UTF-8 relative-path/TAB/decimal-size/TAB/SHA-256/LF manifest is **`dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`**. Input hashes are checked before use and again after validation.

| Read-only input | Physical bytes | SHA-256 |
| --- | ---: | --- |
| `Cities/MP48.map` | 106,740 | `5e4a78c880fbf2cdff9286ce45c3effd4532c1986cb41c84715e3c981b8d510f` |
| `Emperor.exe` | 4,809,081 | `6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e` |
| `Model/EmperorBuildingModels.txt` | 34,218 | `7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6` |

The EXE is read only and never launched. Existing disassembly is extracted only within the bounded relevant windows, and every extracted instruction byte is compared with the freshly hash-pinned PE file. The main proof checks30 windows /1,571 lines /5,060 bytes; the separately verified qualified source-creation/factory path checks37 windows /2,567 lines /7,554 bytes, plus two bounded entry decodings. These overlapping scopes are not unique-code totals or a whole-image decompilation.

## All MP48 Type186 records and provenance

**RAW MAP FACT:** MP48 declares size **170**, border **29**. Its schema1 manager at decompressed logical **1,093,607** consumes all **4,000 records /747,111 bytes**, ending at **1,840,718**. Class counts are Building3,946, Industrial50 and Ferry4. There is no unread manager suffix or parser gap. Baseline full preparation first rejects ID/index40 at logical1,114,088; complete-manager single-record diagnosis independently rejects all three186 sources.

All three are exact **cIndustrialBldg /type186 /status3 /side0 /subindex0 /Base4 /parent1 /NonHouseInfo1 /own wrapper absent0**, without typed Monument/GateHouse state. Their origins are inside the actual diamond, with the off-map bit clear.

| ID / manager / MFC reference | Local → storage | Serialized cell reference | Terrain / Objects / signed height |
| --- | --- | ---: | --- |
| 40 /40 /44 | (75,51) → **(104,80)** | 18,344 | `0x00100082` /0 /+2 |
| 41 /41 /45 | (73,50) → **(102,79)** | 18,114 | `0x00100082` /0 /+2 |
| 42 /42 /46 | (76,51) → **(105,80)** | 18,345 | `0x00100082` /0 /+2 |

Every record is313 bytes: reused MFC class tag2, Base4 payload181, parent word2 and fixed NonHouseInfo128. Industrial has no class-specific wrapper. Logical positions below are decompressed map-stream offsets, distinct from compressed-file positions, PE offsets and runtime addresses.

| ID | Record /Base /parent /NonHouseInfo | Status /side | X /Y | Reference /type /subindex | Saved ID |
| ---: | --- | --- | --- | --- | ---: |
| 40 | 1,114,088 /1,114,090 /1,114,271 /1,114,273 | 1,114,092 /1,114,095 | 1,114,098 /1,114,100 | 1,114,102 /1,114,106 /1,114,108 | 1,114,251 |
| 41 | 1,114,401 /1,114,403 /1,114,584 /1,114,586 | 1,114,405 /1,114,408 | 1,114,411 /1,114,413 | 1,114,415 /1,114,419 /1,114,421 | 1,114,564 |
| 42 | 1,114,714 /1,114,716 /1,114,897 /1,114,899 | 1,114,718 /1,114,721 | 1,114,724 /1,114,726 | 1,114,728 /1,114,732 /1,114,734 | 1,114,877 |

Status/side are unsigned bytes; X/Y/type/subindex signed16-bit; reference/ID signed32-bit. Their Base4-relative offsets are2/5,8/10,12/16/18 and161. All emitted source offsets, widths, signs, values and coordinate/reference transformations are independently reread and checked against the complete production parser.

| ID | Terrain logical offset /width | Objects logical offset /width | Height logical offset /width |
| ---: | --- | --- | --- |
| 40 | 334,831 /4 | 542,767 /4 | 1,007,619 /signed1 |
| 41 | 333,911 /4 | 541,847 /4 | 1,007,389 /signed1 |
| 42 | 334,835 /4 | 542,771 /4 | 1,007,620 /signed1 |

## Complete Type186 corpus state census

**RAW MAP FACT:** all167 complete managers contain exactly **nine Type186 records on four maps**. All nine share the exact state above. There is no observed zero-side Base3/5, different status/subindex or additional wrapper state. Admission of Base4 does not generalize to those variants.

| Map | Saved ID /manager /MFC | Local → storage | Reference /record logical | Terrain /Objects /signed H |
| --- | --- | --- | --- | --- |
| MP41 | 4 /4 /7 | (11,92) → (40,121) | 27,628 /1,094,766 | `0x100082` /1 /0 |
| MP41 | 5 /5 /8 | (12,91) → (41,120) | 27,401 /1,095,079 | `0x100082` /0 /0 |
| MP41 | 6 /6 /9 | (5,85) → (34,114) | 26,026 /1,095,392 | `0x100082` /0 /0 |
| MP48 | 40 /40 /44 | (75,51) → (104,80) | 18,344 /1,114,088 | `0x100082` /0 /2 |
| MP48 | 41 /41 /45 | (73,50) → (102,79) | 18,114 /1,114,401 | `0x100082` /0 /2 |
| MP48 | 42 /42 /46 | (76,51) → (105,80) | 18,345 /1,114,714 | `0x100082` /0 /2 |
| MP50 | 5 /5 /8 | (104,73) → (133,102) | 23,389 /1,095,079 | `0x100082` /0 /2 |
| STGW-Shu | 14 /14 /18 | (74,11) → (118,55) | 12,658 /1,097,910 | `0x100002` /1 /3 |
| STGW-Shu | 15 /15 /19 | (63,12) → (107,56) | 12,875 /1,098,223 | `0x100002` /1 /3 |

The complete read-only audits, including all legacy/policy roundtrips, take227.670s baseline /249.901s candidate, with peak RSS245,137,408 /245,743,616 bytes. They render zero frames; their private comparison Worlds are initialization/test work, not a new production frame/tick path.

All nine independently pass the final production origin contract. Raw Objects1 in three records is not interpreted as runtime ownership or used to skip a source. Saved graphics, map names, IDs, coordinates and current terrain/height do not choose the occupancy state.

## Independent original semantics

The PE32 image base is **0x400000**; `.text` RVA0x1000 maps to file0x400, `.data` RVA0x414000 to file0x412600. Runtime grids/globals below are memory addresses, not map-file offsets. Model text names186 `BUILD_MAP_COPPER_ROCK`; that label corroborates the investigation and supplies no independent permission authority.

| Observation | Static VA | RVA | PE file offset |
| --- | --- | --- | --- |
| Type186 model /side field | `824700` /`824708` | `424700` /`424708` | `422d00` /`422d08` |
| Copper editor choice table entry | `821e68` | `421e68` | `420468` |
| Editor choice delivery | `40466f` | `466f` | `3a6f` |
| Begin selector byte /return | `4051d0` /`4050e3` | `51d0` /`50e3` | `45d0` /`44e3` |
| Explicit type186 brush branch | `4b58cf` | `b58cf` | `b4ccf` |
| Terrain-cell writer | `4b48e0` | `b48e0` | `b3ce0` |
| Lookup range guard /return | `403ec6` /`403ffd` | `3ec6` /`3ffd` | `32c6` /`33fd` |
| Point-undo range guard /default | `53e583` /`53e634` | `13e583` /`13e634` | `13d983` /`13da34` |
| Owner callback selector /target | `51e01a` /`51df1d` | `11e01a` /`11df1d` | `11d41a` /`11d31d` |
| Physical-restore selector /false return | `52f263` /`52f1f0` | `12f263` /`12f1f0` | `12e663` /`12e5f0` |

**EXE-OBSERVED — actual editor operation:** the selector at40464d checks editor category26.40466f reads the selected entry from821e60; entry2 at821e68 is exactly186, stored at88ebdc by404676, with editor mode5. Type186's independently read begin selector4051d0=16 selects4050e3. Ordinary continuous gesture delivery4051f0→40528d→4b5290 iterates its bounded stencil. The explicit comparison **4b58cf** selects **4b58d7…4b58de**, passing **0x100002**,Y,X to **4b48e0**.

That writer computes `border101d0c8 +228×Y +X`, tests current terrain and the bounded eligibility helper4b4790, and writes the terrain grid **f6a9e0** after clearing its incompatible bits. Dirty fields101c524/101c528 track changed/unchanged work. It allocates no Industrial body or positive owner rectangle. The common brush can clear a previous physical object; clearing an old owner is distinct from creating a Type186 owner. The separately inspected186 refresh branch4b5c09→4b5cd1→4b5d50 defines update extent, not a saved occupancy footprint.

**EXE-OBSERVED — lookup and undo:** their point-slot switches each admit only types160..183. Type186 is outside both ranges: lookup403ec6 returns through403ffd; undo53e583 takes53e634 without a point-array write. Separate editor grid snapshots/undo preserve terrain and other existing grids. Unlike indexed fishing/river markers, this type is an editor terrain source; no unsupported point-table byte is read beyond a range guard.

**EXE-OBSERVED — qualified saved-source creation:** accepted gesture begin404990 sets88ebd8. Gesture end4052a0 retains186 (the explicit excluded170/171/176/150 cases do not match) and calls405378→4b1250. The qualified accepted coordinate/environment path reaches4b24c9→42d540. Independently checked subtype predicates select **42d360→51c620→51c660→51c958→51c96f→51c9a0**, constructing actual cIndustrialBldg, not an ordinary factory inferred from the name. Publication42d723 writes the manager8c7630;42d730 assigns+b4 ID,42d736 calls virtual+94→428aa0. The initializer stores status3, type, signed origin/reference and the model's explicit side0. Archive52e7c0→52e961→42d790 serializes this same manager through42dc60→77fd11, descriptor854438 and serializer51ce00→427430/51c3a0. No new parser section or inferred regional/parent payload is added.

**EXE-OBSERVED — actual physical owner path:** the common builder reads model side0 at4b186a, passes zero width/height through4b21a5/4b21a6→4b7ca0 and skips its rectangle loop. Actual placement4b2557→4b11f0→virtual+100 selects Industrial51dd20. Exact selector **51e01a=7** reaches51df1d;51df52/51df53 forward the same zero side through51df61→4b72b0. The rectangle writer retains its coordinate checks, then zero dimensions skip4b7388/4b7390→4b7477. Its positive owner write **fc3750 at4b73ce**, and terrain writef6a9e0 at4b73c2, are inside that skipped loop. Thus the qualified source exists with a saved origin but no physical rectangle is established.

**EXE-OBSERVED — standalone restore:**52f030 tests saved active status and calls52f1d0. Exact selector **52f263=1** selects false-return52f1f0, bypassing physical42d540 and4b11f0 reconstruction. This exclusion and side0 are supporting facts; the separate typed source-creation, editor/state and owner paths above establish the reason for conservative saved-origin protection.

**UNRESOLVED:** the historical gesture and lifetime of each individual record, complete surrounding editor validation, original resource/gameplay behavior, arbitrary state variants and original restore parity. No editor stencil, refresh region, parent body, resources or later owner lifecycle is reconstructed.

## Central implementation and fail-closed behavior

**OPENEMPEROR CONSERVATIVE POLICY:** after the unchanged generic positive SavedSquare path, central `original_occupancy()` adds only **Industrial /type186 /status3 /side0 /subindex0 /Base4 /parent1 /NonHouseInfo1 /own0 /no Monument or GateHouse state → MarkerOrigin**. Protect every exact validated origin. This is a conservative point reservation for a saved editor source, not a physical one-cell building or permission to replay Copper Rock.

Complete-manager identity/index uniqueness, local bounds, storage transformation, exact cell reference, actual diamond/off-map checks and physical/gate conflicts remain mandatory. Distinct valid markers retain the existing allowed shared-origin contract. Unknown states fail atomically, even behind a valid prefix. Road bits cannot override original occupancy. No road/building whitelist, terrain/height meaning, OriginalRoadClassification, gate/edge, World object, save-authority field, resource budget or per-frame/tick path changes.

The sole production source change is the twelve-line186 switch branch in `SandboxPlacement.cpp`. Parser, MapRules/menu, persistence, simulator, renderer, resource/input and visual-profile implementations remain unchanged. Audit, menu and starter consume that same authority; none contains a second Type186 admission list. The implementation remains bounded constant work per source during loading.

## Corpus impact and compatibility

| First complete preparation result | Fresh baseline | Final |
| --- | ---: | ---: |
| Complete managers | 167 | 167 |
| Prepared full policies | 130 | **134** |
| Unknown occupancy | 32 | **28** |
| Known conflicts | 3 | 3 |
| Outside-map original footprint | 2 | 2 |
| Refused maps | 37 | **33** |

The exact new maps are **MP41, MP48, MP50 and STGW-Shu**. All four full managers prepare both policy versions with actual/zero/one legacy inputs; no later independent blocker appears on MP48. Type186 accounts for exactly nine removed individual unknown records; total individually unknown records fall476→467. Every other rejected map retains its exact previous first error. Industrial151/173/188 and other unknown types remain unsupported; MP27's outside-diamond165 and Chang-an Sui's outside-map162/unknown188 remain unchanged. No additional type is addressed.

All **130 previous maps ×2 policies ×3 identical inputs =780 complete policy cases** retain full canonical/component/edge/gate/fingerprint equality. Inputs include the actual renderer-derived singleton mask and both explicit extremes. Compared data covers roads, buildings, protection, both blockers, signed heights, corridors/openings/claims, cardinal transport edges and OriginalRoadClassification. All780 policy-backed World/save/read/reconstruct/restore contracts also remain equal.

A separate literal comparison then reconstructs the same inputs against frozen baseline and current libraries, comparing **3,562,368,890 actual framed bytes** across all167 maps. It includes all780 old policy canonical/component/gate/edge/fingerprint cases, actual masks,3,340 legacy World/save/read/restore cases and780 old policy-backed World/save cases, including both complete rewritten SaveDocuments and before/after-restore canonical strings. There are zero byte differences. Only one map pair is stored at a time; the maximum transient pair is **51,230,370 bytes**, removed after each map. Runtime is566.353s locally; no multi-gigabyte evidence tree is retained. SHA-256 metadata is supplementary to this direct byte equality.

All167 actual legacy masks and presentation resource/asset/upload/byte metadata remain equal. All **3,340 legacy profile/rule World/save roundtrips** remain equal, including City11r1–4 and City16r1–2. These are full original-input preparation and technical persistence controls, not corpus-wide gameplay or native/human rendering acceptance. No policy/save version change or migration is needed.

## Normal MP48 App, paid production and persistence

Owned dummy/software processes run the unchanged production Application/main, with ignored read-only instrumentation delivering normal SDL keys/clicks. No World commands/snapshot, funds, goods, population, workers or couriers are injected. **SCRIPTED APP EVIDENCE** is separate from native delivery and human play.

The normal path is **New Sandbox→City-v16 rule4→MP48→Map rules checked→Empty City→Start**. Ordinary Space pauses at actual tick5. F1 selects Type186 source origin **(104,80)** and reads its actual raw rock terrain, signed height2 and complete manager diagnosis. The independently read source record and immutable permission query establish exact original protection; F1 landscape output is not claimed to identify the original editor's lifecycle. All three origins reject House and Road clicks with “Protected original structure”, zero spending and the full Snapshot unchanged.

Their immediate eight-neighbor cells also fail raw-terrain permission. The bounded read-only planner therefore finds a legal nearby comparison cell **(104,82)**, Manhattan distance2 from ID40, rather than calling it a free immediate neighbor. F1 confirms its raw0x80/Objects0/height2 and independent unprotected road/building permission. Ordinary Road placement costs2; ordinary removal frees it without refund. This neither clears original rock nor expands origin protection.

Actual MP48 permissions choose the mini-layout independently: Houses **(111,112)/(114,112)**, Clay **(111,115)**, Pottery **(117,115)** and six necessary paid Roads **(112..117,114)**. Four building purchases cost460, connection roads12, nearby comparison road2: construction **474**, Funds **826** from ordinary1,300. Population/workforce is12 available/10 assigned. No prepared starter, funds or goods grant is used.

At tick40 Clay has extracted1 and its real courier carries1 with reservation1. At **tick45** it is ToWarehouse, path vertex1/edge progress4. F5 writes the complete real schema20 document; seven ordinary Period steps advance to **52**, then normal F9 returns to the complete45 Snapshot/context and F5 reproduces the entire original file. Wrong relative map, wrong original SHA and changed policy fingerprint each reject through real F9 while preserving the full loaded World/context; the good private document then loads normally.

At **71**, the Clay courier has delivered and entered Returning with cargo/reservation0; at **106** it is back Idle. At **206**, Pottery has completed its first recipe. At **605**, ordinary production totals18 Clay /6 Pottery, maintenance20, taxes0 and Funds806. This city has no complete Food/Service/Water/Health supply and is not a profitable full-city acceptance.

The first process fully exits. A distinct OS process loads the actual tick45 save through **Load Sandbox**, starts paused and continues560 identical ticks. At45/71/106/206/605, full typed WorldSnapshot/context comparisons and entire serialized SaveDocuments are equal byte for byte to the first process. Buildings/roads, stocks/reservations, courier phase/path/progress, funds/population/workforce, map identity and policy fingerprint are included. MP48 policy fingerprint is **`37d6a1bb6685d18af8eaadd71a255c7482343990b371405287d419959fb0de26`** for the actual legacy mask.

Across the two successful processes,351 measured Sandbox frames have zero deltas in all12 prohibited-work counters. Each uses1,370 existing eager uploads, stable during rendering, and releases all owned textures on exit. The new branch adds no frame/tick file/decode/upload/manager scan/World/navigation work.

## Regression and configuration validation

Before the production change, the authored MP48-like state fails against frozen baseline libraries with **“unsupported active original occupancy …type186 …4/1/1”**. The compile has zero warnings; the exact source/library and regression hashes and failure are preserved privately. The same case passes after the bounded branch.

Authored tests cover Base4 under policies1/2; literal one-origin protection and independent neighbors/heights/Road-bit priority; wrong class/type/status/subindex, unsupported side17, Base3/5/other schemas, parent/state/own wrappers and contradictory typed states; missing/mismatched/duplicate ID, duplicate manager index, reference/local/diamond/off-map errors; physical and gate solid/corridor/opening conflicts in both orders; permitted valid marker overlap, inactive sources, late unknown-state atomic rejection and unchanged generic positive squares. Industrial151/173/176/187/188 remain explicit unknown controls. No old negative test is deleted. Complete compressed authored MapRules inputs and normal-menu candidate tests also cover186 and reject later status2/truncated-state candidates without replacing the previous World/policy/files.

Focused Release CTest passes **5/5**: OriginalMapEntities, SandboxMapPermissions, MapRules, MenuMapRules and MP34 shore visuals. The three full configurations run sequentially and pass **438/438 total test executions**, with146 registered cases each. There are zero compiler warnings, skipped/failed tests, sanitizer findings or source changes within/between configurations. Existing timeouts remain unchanged. Coverage includes original parsers/permissions, MapRules/menu, roads/gates/Tower/joins, MP56 saved-General/MP34 shore visuals, budget, save/recovery, deterministic City systems and input/render controls.

| Configuration | Full CTest | Build | CTest elapsed |
| --- | --- | ---: | ---: |
| debug | **146/146 PASS** | 11.8s | 1571.71s |
| release | **146/146 PASS** | 3.41s | 60.55s |
| sanitizer | **146/146 PASS** | 16.06s | 201.44s |

Combined ASan/UBSan uses the existing RelWithDebInfo configuration, `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1` and `UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`. Debug runs two tests concurrently; Release/sanitizer four. Build concurrency is two. No test is silently skipped or timeout increased.

The excluded **Xia rule3/policy1/schema19** full paid playthrough runs separately and sequentially in all three configurations. It retains its earned goal at6800, sustained to9200 with10 Houses/8 effectiveL2/126 residents. Every configuration compares all14 complete earned SaveDocuments and the full `main-facts.jsonl` byte for byte with the preceding ResGate control; all are identical. This is technical deterministic control evidence, separate from human play.

| Xia configuration | Result | Replay elapsed | Historical full SaveDocuments |
| --- | --- | ---: | --- |
| debug | **PASS** | 149.503s | **14/14 identical** |
| release | **PASS** | 4.44s | **14/14 identical** |
| sanitizer | **PASS** | 44.561s | **14/14 identical** |

After all tests and native launch, the full original-tree rehash again matches1464 files /819,711,091 bytes and the starting manifest, with zero changed/missing/added inputs. HEAD remains the starting commit; all changes are local. `git diff --check` passes. Private reports preserve complete logs, JUnit inventories, command provenance, source manifests, first harness failures, full corpus/literal comparisons and App/restart evidence.


Private first-run instrumentation errors remain recorded separately: a copied point-undo reader tried to index186 beyond the160..183 guard; it was corrected to the actual guard/default path. A preliminary selection window was the unrelated188/189/167 editor table; the final proof uses the independently verified186 entry821e68 and category26 path40466f. The planner initially treated an environment-priority SDL hint return as failure, then required a free immediate neighbor that the real copper terrain does not provide. The corrected planner explicitly records the nearby distance2 cell. These are harness/research corrections, without production repair or weakened map bounds.

## Executed test start and remaining scope

From the repository root, this exact command builds current Release, checks the pinned MP48/EXE hashes, runs the complete production rule4/policy2 preflight, then opens the normal menu in a fresh isolated settings/save/recovery root:

```sh
sh tools/test_mp48_occupancy.sh .local/gog-extracted/app
```

It has actually been executed and passes the complete headless check. Choose **New Sandbox**, confirm **City-v16 /MP48 /Empty City**, then **Start Sandbox**. Start output identifies all three protected Type186 origins and rule4/policy2/schema20. The wrapper delegates build/containment/preflight/private-root work to `test_resgate_maps.sh`. Missing data and wrong map hash refuse before any App launch. Failed preparation never substitutes another map or older rules. Personal settings/saves are untouched.

The separate uninstrumented native App launch is available for the user's test; it is launch evidence only. Scripted production/play/save evidence above supplies technical acceptance; no human native-input or original-rendering PASS is claimed. **UNRESOLVED:** unevidenced186 states, original Copper Rock/resource/editor behavior, the remaining33 refused maps, corpus-wide gameplay/rendering, full original composition and human acceptance. This milestone stops at Type186 and its measured impact.
