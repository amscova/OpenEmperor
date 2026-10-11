# MP56: bounded Industrial177/178 occupancy support

This pass starts from clean HEAD **`7dfd20e5ddb171bd683f17f7211f8380c382661a`**, matching the requested reference. It changes only the central load-time occupancy classification of independently evidenced Industrial177/178 zero-side states. City-v16 **rule4 / policy2 / schema20**, older rules/policy 1, parsers, terrain, original roads, gates, renderer/input, World/economy and save/recovery retain their contracts. No migration or publication is performed.

**Result:** MP56 passes complete policy preparation and the normal Empty City menu start. The same 167-map corpus prepares **121 policies**, up from 104; **46 maps remain refused**. The previous 104 policies and all legacy World/save and presentation metadata remain byte-identical. MP56 passes paid Clay transport, scripted rendering, F5/F9 in flight, and loading/continuing that save in a new process. These are technical checks; human play and original graphical fidelity remain open.

## Baseline and original revision

Original data was read only. The before/after inventory contains **1,464 files / 819,711,091 bytes**. Its manifest SHA-256 is **`dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`**, encoded as sorted UTF-8 relative path, TAB decimal size, TAB file SHA-256, LF.

| Input | Bytes | SHA-256 |
| --- | ---: | --- |
| `Cities/MP56.map` | 102,064 | `96e3bd832b40ae57a6e7526eb864cfa85a5bacd4dc3cad95aa7e4fff8dc17892` |
| Pinned original EXE | 4,809,081 | `6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e` |
| `Model/EmperorBuildingModels.txt` | 34,218 | `7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6` |

**RAW MAP FACT:** MP56 is a standalone Map, declared size170, storage border29. Its schema1 manager at decompressed logical **1,093,607** reads all **4,000 records / 736,059 bytes**, ending at **1,829,666**. There are 30 active records; complete class counts are Building3970, Industrial9, GateHouse4, Tower8, ResWall8, ResGate1. The baseline full policy refuses Industrial177 ID/index22; no later count is inferred from that first error.

All five rows below have class **`cIndustrialBldg`**, status3, subindex0, saved side0, Base4 / parent1 / NonHouseInfo1 / class-specific wrapper0, raw terrain **`0x04000004`**, Objects0 and signed Height **+1**. Indices are zero-based; record positions include the MFC tag. Counts and sources come from the complete production parser.

| Type | Original ID / manager index / MFC reference | Local → storage origin | Cell reference | Logical record / base / NonHouse state | Record bytes |
| --- | --- | --- | ---: | --- | ---: |
| 177 | 22 / 22 / 29 | (23,64) → **(52,93)** | 21,256 | 1,100,503 / 1,100,524 / 1,100,707 | 332 |
| 177 | 23 / 23 / 30 | (25,68) → **(54,97)** | 22,170 | 1,100,835 / 1,100,837 / 1,101,020 | 313 |
| 177 | 24 / 24 / 31 | (22,65) → **(51,94)** | 21,483 | 1,101,148 / 1,101,150 / 1,101,333 | 313 |
| 177 | 25 / 25 / 32 | (23,64) → **(52,93)** | 21,256 | 1,101,461 / 1,101,463 / 1,101,646 | 313 |
| 178 | 26 / 26 / 33 | (88,166) → **(117,195)** | 44,577 | 1,101,774 / 1,101,776 / 1,101,959 | 313 |

### Logical field provenance

The existing `field_source()` API supplies these exact decompressed offsets. Status/side are unsigned bytes; X/Y/type/subindex are signed16; reference/ID are signed32. Relative to the Base4 schema word, the offsets are +2/+5/+8/+10/+12/+16/+18/+161; parent schema starts at +181 and inline NonHouse state at +183. These are stream fields, not runtime structure fields or PE file offsets.

| ID | Status | Side | X / Y | Reference | Type / subindex | Original ID |
| ---: | ---: | ---: | --- | ---: | --- | ---: |
| 22 | 1,100,526 | 1,100,529 | 1,100,532 / 1,100,534 | 1,100,536 | 1,100,540 / 1,100,542 | 1,100,685 |
| 23 | 1,100,839 | 1,100,842 | 1,100,845 / 1,100,847 | 1,100,849 | 1,100,853 / 1,100,855 | 1,100,998 |
| 24 | 1,101,152 | 1,101,155 | 1,101,158 / 1,101,160 | 1,101,162 | 1,101,166 / 1,101,168 | 1,101,311 |
| 25 | 1,101,465 | 1,101,468 | 1,101,471 / 1,101,473 | 1,101,475 | 1,101,479 / 1,101,481 | 1,101,624 |
| 26 | 1,101,778 | 1,101,781 | 1,101,784 / 1,101,786 | 1,101,788 | 1,101,792 / 1,101,794 | 1,101,937 |

| ID | Raw Terrain logical offset | Raw Objects logical offset | Signed Height logical offset |
| ---: | ---: | ---: | ---: |
| 22 | 346,479 | 554,415 | 1,010,531 |
| 23 | 350,135 | 558,071 | 1,011,445 |
| 24 | 347,387 | 555,323 | 1,010,758 |
| 25 | 346,479 | 554,415 | 1,010,531 |
| 26 | 439,763 | 647,699 | 1,033,852 |

## Independent original evidence

The hash-checked PE32 EXE was never executed. Seventeen bounded disassembly windows verify **861 instruction lines / 3,054 instruction bytes**, with zero byte mismatches. Image base is `0x400000`; inspected `.text` uses RVA0x1000/file0x400 and `.data` RVA0x414000/file0x412600. Static addresses, runtime globals and map stream offsets remain separate. Private disassembly and original bytes stay ignored under `.local/mp56-occupancy/research/`.

| Observation | Static VA | RVA | PE file offset |
| --- | --- | --- | --- |
| Registered Industrial saved factory | `0x51c140` | `0x11c140` | `0x11b540` |
| Base initializer | `0x428aa0` | `0x28aa0` | `0x27ea0` |
| Industrial serializer | `0x51ce00` | `0x11ce00` | `0x11c200` |
| Type177 model entry | `0x824628` | `0x424628` | `0x422c28` |
| Type178 model entry | `0x824640` | `0x424640` | `0x422c40` |
| Typed editor dispatcher | `0x404990` | `0x4990` | `0x3d90` |
| Type177 editor branch | `0x404f72` | `0x4f72` | `0x4372` |
| Type178 editor branch | `0x405039` | `0x5039` | `0x4439` |
| Standalone physical restore | `0x52f030` | `0x12f030` | `0x12e430` |
| Physical type filter | `0x52f1d0` | `0x12f1d0` | `0x12e5d0` |

**EXE-OBSERVED, class and initialization:** the `cIndustrialBldg` descriptor at static VA `0x854438` selects factory `51c140 → 51c9a0`, with vtable `7b65e4`. Its base initializer `428aa0` writes status3 at runtime +04 and obtains runtime side +07 from the model table. Model entries start at `823590 + 24×type`; **side is at entry +8**, read through `823598 + 24×type`. Both independently inspected entries have zero side. Their other model words are retained uninterpreted. The existing serializer `51ce00 → 427430` and inline state preserve parent1/NonHouseInfo1 without a class-specific payload wrapper. Parser code is unchanged.

The model text's labels `BUILD_MAP_RIVER_IN` and `BUILD_MAP_RIVER_OUT` corroborate the two paths below. The labels, adjacency of type numbers and raw terrain do not independently prove placement or physical occupancy.

**EXE-OBSERVED, type177:** editor selection `404255/404264` selects mode0x12/type177 and writes the selected type to runtime global `88ebdc`. The `(type−22)` dispatcher reads byte **4051c7 = 10**, selecting **404f72** through target table4050e8. It validates the point using bounds margin4 (`4b0800`), the water-neighborhood reader `4ba330`, rejects the interior margin1 check (`4b07e0`), and checks the existing byte at runtime grid136beb0. It retains the previous point via `53e520 → 53e4f0`, then writes signed X/Y to runtime globals **c5ce24/c5ce26** at405027/405016. Existing-point lookup dispatch byte **404039 = 4 → 403f1c** reads those exact globals. Undo byte **53e68d = 4 → 53e5be** restores them.

The same type177 editor branch also calls **4eb420**. Its bounded body accesses the original actor manager10315dc and can create actors through4ea050. Therefore this editor operation is **not claimed to be side-effect-free**. The actor behavior is separate from a physical building owner rectangle and is not imported into OpenEmperor. No original profession or complete river lifecycle is assigned from this investigation.

**EXE-OBSERVED, type178:** editor selection `40426b/40427a` independently selects mode0x13/type178. Dispatcher byte **4051c8 = 11 → 405039** selects its distinct placement branch. It performs the point/bounds/water checks and old-point undo capture, then writes **c5ce28/c5ce2a** at4050d6/4050dc. Existing-point byte **40403a = 5 → 403f3b** and undo byte **53e68e = 5 → 53e5ce** use that second pair. This branch has no call to4eb420. Type178's contract is established separately from177.

**EXE-OBSERVED, standalone physical ownership:** `52f030` checks active status at52f06d, then invokes filter52f1d0 for the saved type at52f078. The `(type−83)` table bytes **52f25a/52f25b = 1** independently select the false branch **52f1f0** for177/178. Both skip from52f087 to52f15c, bypassing physical reconstruction42d540 and owner-writing dispatch4b11f0. Together with the independent point-placement, lookup/undo and zero model sides, this supports point semantics for the observed states. Physical skipping alone would not be sufficient evidence for every saved variant.

**RAW MAP FACT:** the complete corpus has **26 type177 records across 17 maps** (Base3:8/Base4:18) and **18 type178 records across 16 maps** (Base3:3/Base4:15). All share the exact Industrial/status3/side0/subindex0/parent1/state1/own0 state. All origin terrain words are04000004; type177 Objects vary0/1 (23/3), type1780/1 (17/1); heights also vary0/1. Objects0, Height1 and the MP56 coordinates are not production admission conditions.

**UNRESOLVED:** different zero-side active states, Base5 zero-side records, class/parent variants, original actor/river lifecycle, and complete original restore. No Parent/region reference requiring another physical object was evidenced for these admitted point states. This does not prove an absence of other original runtime behavior.

## Central conservative policy and tests

**OPENEMPEROR CONSERVATIVE POLICY:** `original_occupancy()` admits MarkerOrigin only after the unchanged generic positive SavedSquare path, with exact Industrial class, type177 or178, side0, status3, subindex0, Base3/4, parent1, NonHouseInfo1, class-wrapper0, and no typed Monument/GateHouse state. All original identity/manager index/reference/bounds/off-map checks and order-independent conflicts still apply. Unsupported states fail atomically; no audit/menu/renderer whitelist is added.

Every saved origin is reserved as OriginalStructure. MP56 has **five source records / four distinct protected cells**: (52,93), (54,97), (51,94), (117,195). IDs22/25 share a point; both remain validated and retained under the existing duplicate-marker contract. The current single editor coordinate pair does not justify discarding older saved origins. A protected point is neither an invented 1×1 physical building nor free space. Protection precedes original-road import. Water, independent neighboring terrain and original roads keep their existing rules; no river replay, NPC, resource, courier, bridge, ramp or ResGate passage is introduced.

The authored Base4/status3/side0/type177 state fails against the frozen pre-fix map library with **Unsupported / exit1**. The first pre-fix authored run stopped at the Base3 case; a separate Base4-first frozen-baseline run reproduces the exact MP56 state. Its source/library hashes and failure log are retained privately. The corrected focused suite passes **5/5**: OriginalMapEntities, SandboxMapPermissions, MapRules, MenuMapRules and OrdinaryTowerSelector.

Separate loops cover both types, Base4/3 and policy1/2. They assert literal masks/blockers, exact singleton protection, independent water/free-neighbor behavior, signed height, no gate/imported-road creation; wrong class/status/subindex/base/wrappers/typed state; unknown176/179/164/188; invalid identity/reference/coordinates/diamond/off-map; duplicate markers; later unknowns; foreign physical and gate solid/corridor/opening conflicts in both record orders; and unchanged positive-square handling even with another active status/Base5. Compressed authored MapRules fixtures exercise the production parser. Normal menu tests admit both points, reject a later164 and truncated state, retain the full previous World/policy/files, Resume and release all textures.

The first integration build caught a new test's `std::array` deduction error for an untyped second Cell. That test-only compile error was corrected; the first log is preserved separately from corrected warning-free builds. No runtime production failure is concealed by a retry.

## Corpus and old-contract compatibility

The same immutable **167 maps** are audited before/after using the unchanged production audit, complete manager parser, permissions producer and actual renderer-derived legacy masks. Conditional one-record diagnostics never claim a complete policy for a refused map.

| Full-manager/full-policy result | Before | After |
| --- | ---: | ---: |
| Complete managers | 167 | 167 |
| Fully prepared policies | 104 | **121** |
| Unknown occupancy first failures | 59 | **41** |
| Known conflicts | 3 | **3** |
| Outside-map original footprint first failures | 1 | **2** |
| Refused maps | 63 | **46** |
| Diagnosed unknown records | 640 | **596** |

Exactly these **17** maps newly prepare: **Anyi, Chengdu, Jiaozhou, Lingshou, Loyi, MP11, MP12, MP17, MP18, MP25, MP45, MP56, MP66, MP8, Pingyao, Yangzhou, Zhongdu**. This is complete policy preparation, not full-corpus rendering/gameplay acceptance. All44 admitted177/178 source origins individually validate; unrelated unknowns still prevent other complete policies.

MP27 newly exposes the existing bounds failure for Industrial165 **ID/index13**, logical record **1,097,583**, Base4/parent1/state1, local(111,40), storage(155,84), reference19,307, terrain04000004/Objects0/Height0: it is outside the actual map diamond. No bounds rule is relaxed. Chang-an Sui's outside-map Industrial162 and Industrial188 unknowns, and MP34's Industrial179/164, remain unchanged; their complete audit rows are byte-equal to baseline. No other unknown type/class is supported.

For **all previous 104 maps**, actual/zero/all-one legacy inputs produce **3,432 byte-identical policy1 files** and **1,248 byte-identical policy2 files**. These cover Road/Building/Protection masks, blockers, signed heights, complete gate claims/corridors/openings/edges, original-road classification, canonical data and fingerprints. Policy2 retains the previous **10,666** imported roads and totals **12,583** across the newly prepared 121 maps. No policy/save version changes are necessary.

All 167 maps also pass comparison of the actual legacy masks, presentation-resource metadata and **20 supported old profile/rule combinations**, including MP56: **3,340 full empty-World save/read/restore roundtrips per build**, with **all 7,014 comparison files byte-identical**. The full-policy audit now emits preparation metrics for the 17 newly successful maps; the absence of those metrics in refused baseline rows is not a zero-activation claim. This pass enables no additional renderer source/body producer; ResGate's earlier Sui/MP56 GateHouse/Tower activation remains the [historical ResGate result](resgate-parser-occupancy.md#presentation-consequences-of-complete-parsing).

## Normal MP56 session, paid transport and persistence

An isolated actual Application/main uses the normal **New Sandbox → City-v16 rule4 → exact MP56 → Map rules checked → Empty City → Start** path. Its private read-only observer supplies ordinary scripted SDL keys/clicks and records production World/context, draws and counters; it never injects a World, permissions, funds, goods, workers, courier state or camera state. This run uses dummy/software SDL, not native/human input evidence.

At initial tick5, all five point records, eight ResWall sources and the ResGate source are diagnosed. Normal House/Road clicks at all 14 source witnesses remain protected and leave the complete World unchanged. Independent water at(110,107) refuses a road. No ResGate passage is used. A separate pure permissions query finds this actual legal build area; Xia coordinates and a replacement buildable mask are not used.

| Ordinary paid construction | Storage origin(s) | Cost |
| --- | --- | ---: |
| Two Houses | (109,112), (112,112) | 160 |
| Clay Source | (109,115) | 120 |
| Pottery | (115,115) | 180 |
| Eight necessary new Roads | y114, x109–117 excluding111 | 16 |
| Total, 12 paid commands | Original Road(111,114) remains a free existing no-op | **476** |

Funds become824 from 1300; normal workers are 12 available/10 assigned. Production, dispatch, cargo reservation, all three courier phases and four actually drawn directions occur. At tick71 the Clay courier arrives: cargo1→0, target reservation1→0, Pottery Clay stock0→1. It completes its first return at tick106. These transition details are independently replayed through the actual World from the saved in-flight state.

Ordinary **F5/F9/F5** checkpoints compare full WorldSnapshot/context and entire SaveDocuments at **ticks5,45,605**. At45 the Clay courier has cargo1/reservation1, ToWarehouse, path vertex1/edge progress4. A new App process follows normal **Load Sandbox** for that actual saved document, starts paused at45 and receives 560 ordinary step keys to605. Its complete final snapshot/context/document equals the original 600-tick continuation. An independent restore/560-tick replay also equals the entire605 SaveDocument; wrong-map and changed-policy-fingerprint controls refuse with the original map SHA-256 and map-permissions SHA-256 errors.

At605: **18 Clay extracted / 6 completed Pottery recipes**, funds804, construction476, maintenance20, taxes0. Stock, reservations, positions, paths, phases and all counters are included in the equality checks. This mini-build proves no complete Food/Service/Water/Health supply or long-term profit.

The first App run passes in11.06s, **329 App frames / 221 Sandbox frames**; the restart passes in6.281s/130 frames. All 12 measured render-counter deltas are zero; **1,783 eager preparation uploads** stay stable and all owned textures are released on exit. There are no frame files/decode/upload/World copies/commands/BFS/routes/full-map scans introduced by this fix. Scripted drawing of the courier is technical rendering evidence, not a human visual acceptance or Emperor-parity claim.

## Validation, performance and local test start

The three full configurations run sequentially against the same frozen source, using the existing registered tests and timeouts. All **435 test executions pass**; there are zero failed/skipped tests, compiler warnings, source changes during a configuration, or sanitizer diagnostics. Coverage includes OriginalMapEntities, SandboxMapPermissions, MapRules/Menu, OriginalRoads, GatePassages, Tower bodies/wall joins, budgets, save/recovery and determinism. Logs, JUnit and source manifests are retained privately.

| Configuration | CTest result | CTest elapsed | Final build elapsed |
| --- | --- | ---: | ---: |
| Debug | **145/145 PASS** | 1,168.17s | 8.58s |
| Release | **145/145 PASS** | 58.29s | 7.95s |
| ASan + UBSan (`RelWithDebInfo`, `-O2`) | **145/145 PASS** | 214.18s | 18.28s |

The unchanged excluded **City-v16 rule3 / policy1 / schema19 Xia playthrough** is also built and run separately, sequentially in all three configurations. It reaches the paid goal at 6800, maintains it through 9200, and exercises the earned road-interruption/natural-fire branches. Every configuration retains **all 14 complete SaveDocuments and the full `main-facts.jsonl` byte-identical** to the historical ResGate reference. Full Snapshot determinism after every command/tick remains checked; this is technical replay evidence, not a human playthrough.

| Xia configuration | Result | Replay elapsed | Historical full saves |
| --- | --- | ---: | --- |
| Debug | **PASS** | 193.803s | 14 / byte-identical |
| Release | **PASS** | 4.636s | 14 / byte-identical |
| Sanitizer | **PASS** | 61.655s | 14 / byte-identical |

The complete before/after load-only audit measures **30.833s / 228,671,488-byte peak RSS** and **34.591s / 236,912,640-byte peak RSS**, respectively, with 0 World creations/0 rendered frames. These include differing numbers of prepared policies and shared machine load; they are not a classifier microbenchmark or portable performance guarantee. The new type decision remains constant bounded load-time work and adds no resource budget.

The following exact command was executed successfully, including its current Release build, complete rule4/policy2 production CLI preflight and normal native App launch with fresh private settings/saves/recovery:

```sh
sh tools/test_mp56_occupancy.sh .local/gog-extracted/app
```

It verifies the pinned MP56 hash, then delegates to the existing contained-map/build/check/private-root runner. Start text names MP56, rule4/policy2/schema20, **Empty City**, and the five diagnostic storage origins above. Choose **New Sandbox → Start Sandbox**. Original data and personal saves remain untouched. The automated native launch was observed running, then only its owned process was stopped; it is a native launch check, not a human playthrough. A failed full check never launches an App or changes map/profile.

Private reports, source-field JSON, manifests, baseline/final policies, saves, scripted App observer and first-attempt logs stay ignored under `.local/mp56-occupancy/`. Versioned code contains no original bytes, pixels or disassembly fixtures.

**UNRESOLVED:** the remaining 46 original-map refusals; other zero-side177/178 states; original actor/river gameplay; ResWall/ResGate graphics and ResGate passage; whole-scene original composition, complete corpus play, and human native input/visual acceptance. The completed support is limited to the evidenced177/178 occupancy and its verified map/session effects.
