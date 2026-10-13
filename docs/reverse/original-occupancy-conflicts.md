# Original occupancy conflicts — 2026-10-13

Clean starting HEAD: `e13e97442bebb448cd493728a79915f5177beb34`. MPcanal4 was investigated first; Luoyang WJin and SAO-Guangzhou were investigated independently. Only **SAO-Guangzhou** gains a complete safe policy. The other two remain **UNRESOLVED / refused**, with independently invalid sources recorded separately. City-v16 rule **4**, MapPermissions policy **2**, save schema **20**, all older versions and prior occupancy admissions remain unchanged.

## Fresh baseline and corpus effect

| Complete-manager first result | Before | After the one evidenced family |
| --- | ---: | ---: |
| Complete managers | 167 | 167 |
| Fully prepared policies | 159 | 160 |
| Known occupancy conflicts | 3 | 2 |
| Bounds/outside-map failures | 5 | 5 |
| Unknown-occupancy first failures | 0 | 0 |

Newly prepared: `Cities/SAO-Guangzhou.map`. Still refused: Luoyang WJin, MPcanal4, Chang-an Sui, Chizhou, MP27, MP42 and MPWall1_S. The five preexisting bounds maps remain a separate pass. First-failure counts do not conceal other failures within the two conflict maps. Historical 159 and earlier census results retain their original scope.

Original inventory: **1,464 files /819,711,091 bytes**, manifest SHA-256 `dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`. Pinned EXE: `6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e`; Model: `7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6`. Inputs remain read only. Original bytes, bounded disassembly, reports and private saves stay ignored under `.local/original-claim-conflicts/`.

## Claims, evidence and complete diagnosis

- **PHYSICAL_CLAIM:** an exclusive protected region under the existing typed/SavedSquare contract. A conservative saved square does not prove the precise original runtime body.
- **NONPHYSICAL_MARKER:** an evidenced saved point source, conservatively protected at its origin. No physical rectangle or editor operation is imported.
- **COMPOSITE_RELATION:** an independently proven shared owner/part relationship. None is established here. Coordinates, MFC references, Raw Objects, neighboring IDs and overlapping artwork do not prove one.
- **INVALID_CONFLICT:** claims or unsupported associations refused by the central producer. Strict refusal can remain **UNRESOLVED** rather than proving the original game could never contain the data. Invalid origin/reference and bounds facts are identified independently.

**RAW MAP FACT** means complete original archive reads. **EXE-OBSERVED** means bounded static instructions/tables checked against fresh pinned PE bytes; the original EXE was never executed. **OPENEMPEROR CONSERVATIVE POLICY** means protected-area/navigation rules. **UNRESOLVED** means missing original relation/context evidence. Runtime offsets, PE VA/RVA/file offsets and decompressed logical offsets are separate domains.

All three maps are standalone, size 140, storage border 44. Their managers begin at logical **1,093,607**, schema 1, with all **4,000** records read before analysis. Every active source and spatial candidate pair was checked, including pairs with independently invalid sources. Spatial buckets use a fixed 65,536-pair bound rather than all 4,000² pairs. Candidate geometry and pair subsets are diagnosis only; no partial policy/World is published. Base, Monument and Gate layout fields and intersection Terrain/Objects/signed height were independently reread at their provenance.

The reproducible `--conflicts` mode emits all active sources, both-order pair outcomes, every candidate source ID per intersection, field offsets and full-manager protection/blockers. A refused map has **no current partial MapPermissions**. Successful intersection flags use full policy2 and explicit all-one legacy upper bound; actual/zero/one inputs are separately checked through normal stored-renderer readiness.

## MPcanal4 — unresolved Canal interior annotation and six independent bounds errors

**RAW MAP FACT:** `Cities/MPcanal4.map`, SHA-256 `2724cb0f872ac38eece6f64c2704cd52f143a5e264c21f5bc6f34d28fada05f6`, physical 105029 bytes, manager 772874 bytes, 373 active sources /14 complete spatial candidate pairs.

All conflict-related rows have status3; schema order is Base/parent/extended/class-wrapper. Industrial/Ferry class-wrapper0 means absent exposed wrapper, not a Ferry-own schema claim. ID/index equality is specific to these rows, never inferred. Local→storage coordinates, MFC references and logical starts/record byte lengths are independently read; detailed field offsets remain in the private reports.

| ID / index | Class / type | Side / subindex | Schemas | Local → storage | Cell ref | MFC ref | Logical start / bytes | Terrain / Objects / signed height |
| --- | --- | --- | --- | --- | ---: | ---: | --- | --- |
| 19 / 19 | cMonumentBldg / 83 | 4 / 16 | 3/1/9/0 | (68,68) → (112,112) | 25648 | 23 | 1099600 / 321 | 0x88 / 0 / 0 |
| 61 / 61 | cIndustrialBldg / 162 | 0 / 0 | 3/1/1/0 | (70,68) → (114,112) | 25650 | 65 | 1112832 / 311 | 0xc8 / 0 / 0 |
| 62 / 62 | cIndustrialBldg / 162 | 0 / 0 | 3/1/1/0 | (70,68) → (114,112) | 25650 | 66 | 1113143 / 311 | 0xc8 / 0 / 0 |

Monument19 has phase4/material0/height 0/orientation0, Monument schema 9. Its unchanged conservative claim is all 16 cells x112..115/y112..115. Industrial162 IDs61 and62 are separate zero-side origins at(114,112), intersecting the square's interior. Their mutual marker duplication already accepts. They use Base3 and do not coincide with the Monument origin.

Every overlapping candidate pair, including existing valid marker duplicates:

| IDs | Exact intersection | Baseline producer decision |
| --- | --- | --- |
| 19 ↔ 61 | (114,112) | conflicting complete original occupancy |
| 19 ↔ 62 | (114,112) | conflicting complete original occupancy |
| 50 ↔ 277 | (128,169) | Accepted marker ↔ marker |
| 57 ↔ 71 | (116,84) | Accepted marker ↔ marker |
| 61 ↔ 62 | (114,112) | Accepted marker ↔ marker |
| 67 ↔ 171 | (111,94) | Accepted marker ↔ marker |
| 92 ↔ 341 | (110,64) | Accepted marker ↔ marker |
| 126 ↔ 236 | (79,94) | Accepted marker ↔ marker |
| 179 ↔ 180 | (92,94) | Accepted marker ↔ marker |
| 202 ↔ 253 | (58,102) | Accepted marker ↔ marker |
| 235 ↔ 237 | (79,96) | Accepted marker ↔ marker |
| 298 ↔ 335 | (133,135) | Accepted marker ↔ marker |
| 328 ↔ 330 | (96,63) | Accepted marker ↔ marker |
| 372 ↔ 373 | (166,104) | Accepted marker ↔ marker |

**EXE-OBSERVED:** explicit Dirt `4b5873/4b587d → 4b47b0` and the qualified source builder establish terrain/editor-point semantics. The Dirt writer clears runtime owners rather than assigning a marker ID; it is not replayed. Monument83's model side is 1, while its saved side is 4. Constructor `5630b0`, owner callback `56a0d0`, subpart path `563850` and composed path `563fd0` involve phase/material/orientation/subpart placement and can rewrite coordinates/reference/side. Those windows do not establish the exact body for type83/subindex16/phase4, authorize reducing its saved4×4, or attach Dirt61/62 as children. Raw Objects 0 supplies no owner link.

**UNRESOLVED:** no original Parent/Child or proven admissible interior annotation relationship was found. No Monument exception, source deletion/merge or footprint reduction is added. First failure remains `conflicting complete original occupancy`. Even a resolved overlap would not make the map safe: six preexisting sources independently fail diamond bounds:

| ID / type | Storage origin | Logical start | Failure |
| --- | --- | ---: | --- |
| 206 / 185 | (168,130) | 1157927 | active original footprint outside map |
| 210 / 185 | (155,143) | 1159171 | active original footprint outside map |
| 211 / 185 | (154,144) | 1159482 | active original footprint outside map |
| 212 / 185 | (149,149) | 1159793 | active original footprint outside map |
| 216 / 185 | (144,154) | 1161037 | active original footprint outside map |
| 219 / 163 | (160,138) | 1161970 | active original footprint outside map |

All other 12 candidate pairs are existing valid marker duplicates. No additional physical↔physical overlap was found. No partial protection, construction or transport policy is published for this refused map.

## Luoyang WJin — unresolved Gate/Tower interior annotations and an invalid reference

**RAW MAP FACT:** `Cities/Luoyang WJin.map`, SHA-256 `6126b1dd5fad99ea22f53b299feee82c8f997b5d81cf6c14c91783634773f953`, physical 161746 bytes, manager 750417 bytes, 141 active sources /19 complete spatial candidate pairs.

All conflict-related rows have status3; schema order is Base/parent/extended/class-wrapper. Industrial/Ferry class-wrapper0 means absent exposed wrapper, not a Ferry-own schema claim. ID/index equality is specific to these rows, never inferred. Local→storage coordinates, MFC references and logical starts/record byte lengths are independently read; detailed field offsets remain in the private reports.

| ID / index | Class / type | Side / subindex | Schemas | Local → storage | Cell ref | MFC ref | Logical start / bytes | Terrain / Objects / signed height |
| --- | --- | --- | --- | --- | ---: | ---: | --- | --- |
| 67 / 67 | cIndustrialBldg / 162 | 0 / 0 | 4/1/1/0 | (43,86) → (87,130) | 29727 | 70 | 1114485 / 313 | 0x8088 / 4 / 0 |
| 68 / 68 | cIndustrialBldg / 162 | 0 / 0 | 4/1/1/0 | (43,87) → (87,131) | 29955 | 71 | 1114798 / 313 | 0x8088 / 0 / 0 |
| 69 / 69 | cIndustrialBldg / 162 | 0 / 0 | 4/1/1/0 | (46,87) → (90,131) | 29958 | 72 | 1115111 / 313 | 0x8088 / 0 / 0 |
| 84 / 84 | cIndustrialBldg / 162 | 0 / 0 | 4/1/1/0 | (74,86) → (118,130) | 29758 | 87 | 1119806 / 313 | 0x8088 / 0 / 0 |
| 88 / 88 | cIndustrialBldg / 162 | 0 / 0 | 4/1/1/0 | (74,87) → (118,131) | 29986 | 91 | 1121058 / 313 | 0x8088 / 0 / 0 |
| 91 / 91 | cIndustrialBldg / 162 | 0 / 0 | 4/1/1/0 | (74,64) → (118,108) | 24742 | 94 | 1121997 / 313 | 0x8088 / 0 / 0 |
| 92 / 92 | cIndustrialBldg / 162 | 0 / 0 | 4/1/1/0 | (74,65) → (118,109) | 24970 | 95 | 1122310 / 313 | 0x8088 / 0 / 0 |
| 93 / 93 | cIndustrialBldg / 162 | 0 / 0 | 4/1/1/0 | (72,67) → (116,111) | 25424 | 96 | 1122623 / 313 | 0x8088 / 0 / 0 |
| 94 / 94 | cIndustrialBldg / 162 | 0 / 0 | 4/1/1/0 | (73,67) → (117,111) | 25425 | 97 | 1122936 / 313 | 0x8088 / 4 / 0 |
| 95 / 95 | cIndustrialBldg / 162 | 0 / 0 | 4/1/1/0 | (73,68) → (117,112) | 25653 | 98 | 1123249 / 313 | 0x8088 / 4 / 0 |
| 96 / 96 | cIndustrialBldg / 162 | 0 / 0 | 4/1/1/0 | (74,68) → (118,112) | 25654 | 99 | 1123562 / 313 | 0x8088 / 0 / 0 |
| 104 / 104 | cIndustrialBldg / 162 | 0 / 0 | 4/1/1/0 | (74,54) → (118,98) | 22462 | 107 | 1126066 / 313 | 0x8088 / 0 / 0 |
| 107 / 107 | cGateHouse / 130 | 1 / 0 | 4/1/1/2 | (42,85) → (86,129) | 29498 | 111 | 1127005 / 337 | 0x8088 / 0 / 0 |
| 108 / 108 | cIndustrialBldg / 162 | 0 / 0 | 4/1/1/0 | (74,67) → (118,111) | 25426 | 112 | 1127342 / 313 | 0x8088 / 1 / 0 |
| 110 / 110 | cGateHouse / 130 | 1 / 0 | 4/1/1/2 | (72,64) → (116,108) | 25654 | 114 | 1127968 / 323 | 0x8088 / 0 / 0 |
| 111 / 111 | cTower / 131 | 2 / 0 | 4/1/1/0 | (73,86) → (117,130) | 29757 | 116 | 1128291 / 325 | 0x8088 / 4 / 0 |
| 113 / 113 | cIndustrialBldg / 162 | 0 / 0 | 4/1/1/0 | (74,54) → (118,98) | 22462 | 118 | 1128931 / 313 | 0x8088 / 0 / 0 |
| 116 / 116 | cTower / 131 | 2 / 0 | 4/1/1/0 | (73,53) → (117,97) | 22233 | 121 | 1129870 / 315 | 0x8088 / 0 / 0 |

Gate107/layout0 has full 5×3 footprint x86..90/y129..131 and corridor x88/y129..131. Dirt67/68/69 intersect solids at(87,130),(87,131),(90,131). Tower111 retains all four cells x117..118/y130..131; Dirt84/88 meet its right column. Tower116 retains all four cells x117..118/y97..98; distinct Dirt104/113 both meet(118,98). Their marker duplication accepts; their two Tower pairs do not. These are seven occupancy conflicts, not seven gates or structural merges.

Gate110 independently fails: local(72,64)→storage(116,108), expected reference **24,740**, saved **25,654** (=storage(118,112)). Layout1's candidate 3×5 at x116..118/y108..112 intersects Dirt91/92/93/94/95/96/108. Those seven candidates remain diagnosed despite the invalid reference; they are not seven additionally admitted Gate conflicts. Wrapper2's eight attached-unit bytes do not prove Dirt Parent/Child IDs. No reference is repaired or source ignored.

Every overlapping candidate pair, including existing valid marker duplicates:

| IDs | Exact intersection | Baseline producer decision |
| --- | --- | --- |
| 14 ↔ 15 | (91,98) | Accepted marker ↔ marker |
| 16 ↔ 132 | (92,98) | Accepted marker ↔ marker |
| 29 ↔ 30 | (79,117) | Accepted marker ↔ marker |
| 40 ↔ 41 | (90,112) | Accepted marker ↔ marker |
| 67 ↔ 107 | (87,130) | conflicting complete original occupancy |
| 68 ↔ 107 | (87,131) | conflicting complete original occupancy |
| 69 ↔ 107 | (90,131) | conflicting complete original occupancy |
| 84 ↔ 111 | (118,130) | conflicting complete original occupancy |
| 88 ↔ 111 | (118,131) | conflicting complete original occupancy |
| 91 ↔ 110 | (118,108) | active original origin/reference mismatch |
| 92 ↔ 110 | (118,109) | active original origin/reference mismatch |
| 93 ↔ 110 | (116,111) | active original origin/reference mismatch |
| 94 ↔ 110 | (117,111) | active original origin/reference mismatch |
| 95 ↔ 110 | (117,112) | active original origin/reference mismatch |
| 96 ↔ 110 | (118,112) | active original origin/reference mismatch |
| 104 ↔ 113 | (118,98) | Accepted marker ↔ marker |
| 104 ↔ 116 | (118,98) | conflicting complete original occupancy |
| 108 ↔ 110 | (118,111) | active original origin/reference mismatch |
| 113 ↔ 116 | (118,98) | conflicting complete original occupancy |

**EXE-OBSERVED:** exact Industrial162 non-owning source and physical-restore exclusion are independently checked. The bounded Gate130 restore `52f030 → 4b11f0 → 4f9780 → 4f9570` uses signed layout and15 components. Tower131 factory `51c91b → 5da3d0`, placement `5da420 → 4b72b0`, restores its own2×2 physical owner. Fresh byte-verified Gate/Tower constructor, placement and serializer windows confirm separate paths; they establish no shared Dirt parent or corridor exception. Non-owning Dirt semantics alone do not authorize arbitrary structural interiors. Existing Gate corridors/openings, footprints, Tower bodies/joins and reference validation stay exact.

**UNRESOLVED / refused:** the seven occupancy conflicts stay strict and Gate110 stays independently invalid. The other five candidate pairs are valid marker duplicates. No physical↔physical conflict or proven composite relationship occurs in this inventory. First failure remains the earlier occupancy conflict; Gate110 is explicitly recorded behind it. No partial policy is published.

## SAO-Guangzhou — supported exact Dirt/Ferry anchor annotation

**RAW MAP FACT:** `Cities/SAO-Guangzhou.map`, SHA-256 `c75603d5bf5d3b2b7f5f92e59b92c671d961f2277f600b4c2be59ccd2a15a8ae`, physical 93953 bytes, manager 741661 bytes, 43 active sources /4 complete spatial candidate pairs.

All conflict-related rows have status3; schema order is Base/parent/extended/class-wrapper. Industrial/Ferry class-wrapper0 means absent exposed wrapper, not a Ferry-own schema claim. ID/index equality is specific to these rows, never inferred. Local→storage coordinates, MFC references and logical starts/record byte lengths are independently read; detailed field offsets remain in the private reports.

| ID / index | Class / type | Side / subindex | Schemas | Local → storage | Cell ref | MFC ref | Logical start / bytes | Terrain / Objects / signed height |
| --- | --- | --- | --- | --- | ---: | ---: | --- | --- |
| 13 / 13 | cIndustrialBldg / 162 | 0 / 0 | 4/1/1/0 | (55,63) → (99,107) | 24495 | 16 | 1097583 / 313 | 0x88 / 0 / 0 |
| 14 / 14 | cFerryBldg / 210 | 2 / 0 | 4/1/1/0 | (55,63) → (99,107) | 24495 | 18 | 1097896 / 2337 | 0x88 / 0 / 0 |

Dirt13 and Ferry14 share validated local/storage origin and cell reference, but retain separate original IDs/MFC objects. Ferry-own state remains schema 1 at logical 1,098,223: schema word plus2008 bytes, opaque words 15 and4 followed by2000 opaque bytes. These words are not a Dirt parent proof. The complete reader still rejects other Ferry-own schemas/truncation. No independent source error occurs behind the conflict.

Every overlapping candidate pair, including existing valid marker duplicates:

| IDs | Exact intersection | Baseline producer decision |
| --- | --- | --- |
| 1 ↔ 2 | (117,59) | Accepted marker ↔ marker |
| 13 ↔ 14 | (99,107) | conflicting complete original occupancy |
| 36 ↔ 37 | (95,149) | Accepted marker ↔ marker |
| 39 ↔ 40 | (95,151) | Accepted marker ↔ marker |

**EXE-OBSERVED:** explicit Dirt `4b587d → 4b47b0` writes terrain/auxiliary state and **zero** to runtime owner grid `fc3750` at `4b482d`. The preceding existing-object branch `4b582c → 4b7870/4afdf0` can remove an old physical owner. Editor replay would therefore be wrong. Independently checked qualified builder `4052a0 → 405378 → 4b1250 → 42d540/42d360 → 51c620/51c660 → 51c9a0 → 428aa0` constructs the saved Industrial162 source. Exact type162 factory predicates/model side0 and the ordinary owner selector's zero-dimension `51df1d → 4b72b0` path are checked. This writes no positive-area source ownership; it does not establish a historical gesture order.

Standalone physical reconstruction `52f030` excludes exact162 through selector1→`52f1f0`, and admits210 through selector0→`52f1ed`. Exact210 predicate `4bdb60`, factory `51c8e0 → 4c5dc0`, constructor vtable `7afbd8`, virtual+100=`4c69a0` are independently pinned. Ferry placement reads its own ID at `4c69ee`, passes it through `4c69f7 → 4b84b0`, and writes positive ownership at `4b859f` into `fc3750`. Separate landing write `4c6a33 → 4b72b0` also uses that Ferry ID. Its original shape is orientation/mode-dependent: this does **not** prove the conservative square equals its precise original body, expand to landing/route claims or import Ferry gameplay. Serializer `4c6b90 → 51ce00` confirms complete own schema 1+2008-byte state.

Together these paths establish the narrow **nonphysical saved Dirt annotation at the exact Ferry anchor** does not compete with the Ferry's restored owner. This is no Parent/Child composite or generic marker-on-building rule. Exact saved states, independently valid identities/references, non-owning source construction and separate restoring Ferry owner are required in addition to anchor equality.

**OPENEMPEROR CONSERVATIVE POLICY:** all four cells **(99,107),(100,107),(99,108),(100,108)** remain protected. Both building/road permissions stay false with `OriginalStructure` blockers under both policies and all legacy masks. Their terrain is 0x88/0x0c/0x88/0x0c, Objects 0, signed height 0; these facts never admit the relationship. Marker13 is retained, Ferry14 keeps its full square, and neither becomes a Sandbox owner/road. The complete actual map prepares atomically. Its other three candidate pairs are unchanged valid marker duplicates.

## Central contract and authored regression

`original_occupancy()` and all prior admissions are unchanged. `prepare_original_rules()` permits only:

- Dirt: Industrial162/status3/side0/subindex0/Base4/parent1/NonHouse1/absent class-wrapper0, no typed Monument/GateHouse.
- Ferry: Ferry210/status3/side2/subindex0/Base4/parent1/NonHouse1/absent exposed class-wrapper0, no typed Monument/GateHouse; unchanged complete parser additionally enforces own schema 1+2008 bytes.
- Exact shared local origin/reference after all existing complete identity/index, unique ID, origin/reference, diamond/off-map and full-footprint checks.

The existing owner grid retains a temporary pointer to the validated physical source rather than only its ID. The existing one-byte marker grid distinguishes exclusively qualifying Dirt markers from any unrelated marker. Sources/identities are untouched. A Ferry overlays only those Dirt markers at its origin; a later Dirt overlays only that Ferry's exact origin. Second physical owners always fail. Mixed markers reject in every order; distinct marker duplicates retain their old contract. Pointers never escape into World, persistence or rendering. Work remains fixed-grid plus bounded claim cells, with no frame/tick changes or new traffic.

An independently authored complete Industrial/Ferry archive specifies the literal four-cell union before the production fix. Old code is **RED** with the exact occupancy conflict. Afterward both orders/policies have identical canonical permissions; IDs remain present; Ferry-only union is unchanged; actual World House/Road validation rejects every protected cell. Multiple eligible Dirt identities are not merged.

Negatives include wrong class/type/status/subindex/side/Base/parent/NonHouse/wrapper/typed state, Base3/5 variants, foreign/mixed markers, similar interior points, competing physical owners, Gate solids/corridors, missing/duplicate IDs or manager indices, invalid references, off-map corners, outside-diamond sources, unknown late records, unknown Ferry-own schema and truncation. MapRules/Menu tests cover full preflight/publication and preserve sessions, files, Resume and texture ownership after failures. No preexisting negative is removed or test weakened.

## Complete compatibility and regression

All 159 prior policies were frozen before edits and compared **literally** under policy1/2 × actual/zero/one masks: **954 cases**. Complete canonical bytes, road/building masks, protection, signed heights, blockers, gate IDs/footprints/corridors/openings, edge blockers, OriginalRoadClassification and fingerprints match. All 167 maps'20 legacy World/save contracts (**3,340**) and954 prior permission/save cases also match. Per-map streamed comparison checked **4,114,941,032 bytes**, maximum transient baseline/candidate pair **51,230,370 bytes**, then removed only its own transient files. Hashes supplement this proof.

All 167 actual presentation preparations/legacy masks and source metadata of the three investigated maps match. All 160 current policies prepare for actual/zero/one masks and both versions (**960 permission/save cases**). No terrain/road/gate/World/economy/input/renderer or texture code changes. Original inventory is unchanged.

Final suites: **Debug146/146, Release146/146, ASan/UBSan146/146**; zero skipped tests/compiler warnings/source changes, no timeout modifications. Configurations ran serially (approximately 1238/109/177 seconds including builds). Separate paid-Xia goal/stability runs pass in all three configurations: every run reproduces all14 historical SaveDocuments and full main-facts bytes, with no source changes or compiler warnings. The historical paid goal at 6800 and stable continuation through 9200 remain unchanged.

Preserved first-run issues are distinct: intended authored RED; a negative-test catch mismatch followed by its fixed missing World include/stale binary run; private source-witness/compatibility JSON key errors; a documentation-builder syntax error. None is a production permission failure. Corrected focused tests, literal comparisons and normal App tests pass.

## Normal App, paid logistics, F5/F9 and process restart

An ignored observer links unchanged production `Application/main`, delivering ordinary scripted SDL events to isolated dummy/software processes. It neither executes World commands nor mutates snapshots/grants goods/funds. Separate coordinate planning uses pure actual initialized permissions and is not App acceptance. Personal roots/existing processes are untouched.

Flow: **New Sandbox → City-v16 rule4 → exact Guangzhou → Map rules checked → Empty City → Start**. F1 and House/Road attempts inspect all four protected cells, with separate source witnesses at the shared anchor; selection/rejection preserves the complete WorldSnapshot. Independent complete source reads/authored tests preserve IDs13/14. F1 reports full 4000-record reader readiness; historical image lines do not individually name these source IDs. Neighbor(99,106) accepts a paid2 Road and free removal.

Actual independently found layout: Houses(84,97)/(87,97), Clay(84,100), Pottery(90,100), roads(85..90,99). Normal cost80+80+120+180+12=472 plus the2 comparison Road: construction **474**, funds **826**. Two Houses supply 12 workers;10/10 requested are assigned. No original road is purchased again or state injected.

Tick 40: extracted Clay 1, dispatched cargo 1/reservation 1. Tick 45: actually in flight. Tick 71: delivered, zero cargo/reservation, returning. Tick 106: idle at owner. Tick 206: Pottery 1. Tick 605: Clay 18/Pottery 6, funds 806 after ordinary maintenance 20. This is a paid production smoke test, not a complete profitable settlement.

Actual F5 at 45, seven subsequent ticks, F9 and repeated F5 preserve full typed WorldSnapshot/SaveDocument JSON and original SHA/policy/legacy context. A new process loads that actual save through the normal menu and matches allfive subsequent complete checkpoints through 605 under identical ticks/commands. Wrong map, original SHA and policy-fingerprint F9 controls refuse without modifying the current session; good-document load then succeeds.

Normal: 29 map actions/7 checkpoints/373 total frames. Restart: 5 checkpoints/153 frames. All measured production render frames have all 12 prohibited-work counters zero. Each process eagerly uploads 1319 textures and ends with **zero live textures**. 100 paused status/overlay frames preserve state. Scripted technical acceptance, native launch and **human acceptance (open)** remain separate. Great Wall Automatic is unchanged; no original restore-context/scene-parity claim follows.

## Reproducible test starter

Run from the repository root:

```sh
sh tools/test_remaining_occupancy.sh .local/gog-extracted/app --audit
sh tools/test_remaining_occupancy.sh .local/gog-extracted/app --map "MPcanal4"
sh tools/test_remaining_occupancy.sh .local/gog-extracted/app --map "Luoyang WJin"
sh tools/test_remaining_occupancy.sh .local/gog-extracted/app --map "SAO-Guangzhou"
```

All were executed. The first two map commands refuse the exact selected map with its current occupancy blocker and launch no app. Guangzhou passes full rule4/policy2 Empty City preflight and opens the normal menu in a fresh isolated root: choose **New Sandbox → Start Sandbox**. Storage(99,107)..(100,108) must reject construction; neighboring(99,106) remains legal. Unrelated historical graphic fallbacks can remain; occupancy preparation does not certify rendering parity.

Complete diagnostics include invalid candidates hidden behind first failures:

```sh
sh tools/test_remaining_occupancy.sh .local/gog-extracted/app --conflicts "MPcanal4"
sh tools/test_remaining_occupancy.sh .local/gog-extracted/app --conflicts "Luoyang WJin"
sh tools/test_remaining_occupancy.sh .local/gog-extracted/app --conflicts "SAO-Guangzhou"
```

These commands were also executed. Each pins the complete original inventory, resolves one exact production MapCatalog stem and writes deterministic reports under `.local/original-claim-conflicts/`. No alternate map/profile or terrain-editor replay. The shared historical paid-demo preflight remains unchanged. No commit, push, tag, release or publication.
