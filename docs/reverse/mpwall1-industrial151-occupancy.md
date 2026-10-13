# MPWall1: bounded Industrial151 source-origin protection

This pass starts at clean HEAD **`30cd6fd975065beedb2da0953a25be0beb6db291`**, matching the supplied reference. It addresses only the independently evidenced saved zero-side Industrial151 state. City-v16 **rule4 /MapPermissions policy2 /save schema20**, policy1 and older versions retain their contracts without migration. No terrain editor, industry/resource/military/NPC function, bridge/ramp/Ferry connection or graphic is added. Original files are read only; private disassembly, reports, saves and captures remain ignored under `.local/mpwall1-occupancy/`. No commit or publication is performed.

**Technical result:** MPWall1 passes complete policy preparation and a normal paid Empty City production/save/restart test. All167 managers parse; **135 policies prepare /32 maps refuse**. MPWall1_S exposes an independent later Industrial163 bounds failure and remains refused; MP42 retains its earlier Industrial173 blocker. All13 evidenced151 sources individually validate, which does not make those two full managers playable.

## Fresh baseline and original revision

The complete baseline/candidate audits, including legacy/policy roundtrips, take195.174s /199.628s, peak RSS246,939,648 /249,675,776 bytes, with zero rendered frames. Their private comparison Worlds are initialization/test work, not new production frame/tick work.

A fresh production-reader audit measures **167 complete managers /134 prepared policies /33 refusals**:28 unknown-occupancy first failures, three known conflicts and two outside-map footprints. The same complete167-map corpus is audited before and after the change, independently of the preceding [Industrial186 census](mp48-industrial186-occupancy.md).

The original tree contains **1,464 files /819,711,091 bytes**, with sorted UTF-8 relative-path/TAB/decimal-size/TAB/file-SHA256/LF manifest **`dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`**. Inputs are hashed before use and again after validation.

| Read-only input | Physical bytes | SHA-256 |
| --- | ---: | --- |
| `Cities/MPWall1.map` | 108,762 | `8d9932ab051b80b140b533da641d9448cb8fc42b2359211933265940f81a0cf9` |
| `Emperor.exe` | 4,809,081 | `6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e` |
| `Model/EmperorBuildingModels.txt` | 34,218 | `7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6` |

The EXE is never launched. Only bounded relevant windows are extracted from existing disassembly and freshly byte-checked against the hash-pinned PE. Main proof:32 windows /1,679 instruction lines /5,346 bytes, zero mismatches. Separate qualified creation/factory proof:37 windows /2,567 lines /7,554 bytes, plus two verified entry decodings. These overlapping scopes are not deduplicated unique-code totals or full decompilation.

## Complete MPWall1 manager and all Type151 records

**RAW MAP FACT:** MPWall1 declares size170, border29. Its schema1 manager at decompressed logical **1,093,607** reads all **4,000 records /742,090 bytes**, ending at **1,835,697**. Complete class counts: Building3,888, Fill40, Industrial19 and Monument53. Baseline policy preparation first rejects ID/index97 at logical1,119,368. No later manager facts are inferred from that first failure.

There is exactly one Type151 source. Its complete state is **cIndustrialBldg /type151 /status3 /side0 /subindex0 /Base4 /parent1 /NonHouseInfo1 /own wrapper absent0**, without typed Monument/GateHouse state.

| Saved ID /manager /MFC reference | Local →storage | Serialized cell reference | Terrain /Objects /signed H |
| --- | --- | ---: | --- |
| **97 /97 /102** | (139,102) → **(168,131)** | **30,036** | `0x02000080` /8 /0 |

Its313-byte record contains reused class tag2, Base4 payload181, parent schema word2 and fixed NonHouseInfo128. Industrial has no class-specific wrapper. These are decompressed stream positions, separate from compressed-file offsets, PE offsets and runtime member addresses.

| Field/section | Logical offset | Width /signedness |
| --- | ---: | --- |
| MFC record /Base4 | 1,119,368 /1,119,370 | tag2 /base181 |
| Parent /NonHouseInfo | 1,119,551 /1,119,553 | schema2 /state128 |
| Status /side | 1,119,372 /1,119,375 | unsigned1 each |
| Local X /Y | 1,119,378 /1,119,380 | signed2 each |
| Cell reference | 1,119,382 | signed4 |
| Type /subindex | 1,119,386 /1,119,388 | signed2 each |
| Original ID | 1,119,531 | signed4 |
| Terrain /Objects | 381,599 /589,535 | unsigned4 each |
| Height | 1,019,311 | signed1 |

All source fields, offsets, widths, signs and values are independently reread and matched against production parser provenance. Storage equals local+29, and reference equals228×storageY+storageX. Base-relative status/side/X/Y/reference/type/subindex/ID offsets are2/5/8/10/12/16/18/161; these differ from runtime members+04/+07/+0a/+0c/+10/+14/+16/+b4.

## Fresh complete Type151 state census

**RAW MAP FACT:** all167 complete managers contain **13 Type151 records on three maps**. Every record has the exact state above: Industrial/151/status3/side0/subindex0/Base4/parent1/NonHouse1/own absent0. There are no observed zero-side Base3/5 or other active state variants. All13 origins are on the actual diamond, with the off-map bit clear. Some remain after the terrain flag has been cleared; two pairs have identical origins.

| Map | ID /manager /MFC | Local →storage | Reference /record logical | Terrain /Objects /signed H |
| --- | --- | --- | --- | --- |
| MP42 | 18 /18 /22 | (54,76) → (83,105) | 24,023 /1,115,242 | `0x80` /0 /0 |
| MP42 | 19 /19 /23 | (54,76) → (83,105) | 24,023 /1,115,555 | `0x80` /0 /0 |
| MP42 | 20 /20 /24 | (83,53) → (112,82) | 18,808 /1,115,868 | `0x80` /0 /0 |
| MP42 | 21 /21 /25 | (72,91) → (101,120) | 27,461 /1,116,181 | `0x02000080` /8 /1 |
| MP42 | 22 /22 /26 | (87,107) → (116,136) | 31,124 /1,116,494 | `0x80` /0 /1 |
| MP42 | 24 /24 /28 | (74,91) → (103,120) | 27,463 /1,117,120 | `0x02000080` /8 /1 |
| MP42 | 25 /25 /29 | (79,94) → (108,123) | 28,152 /1,117,433 | `0x02000080` /16 /1 |
| MP42 | 38 /38 /42 | (72,152) → (101,181) | 41,369 /1,121,502 | `0x81` /1 /0 |
| MP42 | 39 /39 /43 | (72,152) → (101,181) | 41,369 /1,121,815 | `0x81` /1 /0 |
| MP42 | 44 /44 /48 | (128,105) → (157,134) | 30,709 /1,123,380 | `0x80` /0 /1 |
| MP42 | 47 /47 /51 | (128,92) → (157,121) | 27,745 /1,124,319 | `0x02000080` /16 /2 |
| MPWall1 | 97 /97 /102 | (139,102) → (168,131) | 30,036 /1,119,368 | `0x02000080` /8 /0 |
| MPWall1_S | 95 /95 /100 | (140,102) → (169,131) | 30,037 /1,118,742 | `0x02000080` /16 /0 |

Nonzero Raw Objects is not runtime owner ID. Raw terrain, Objects, saved graphics, signed height, map names, IDs/indices and coordinates do not choose admission. Both records of each valid duplicate-origin pair are retained under the existing marker-overlap contract.

## Independent original evidence

PE32 image base is0x400000; `.text` RVA0x1000 maps to file0x400, `.data` RVA0x414000 to file0x412600. Runtime globals/grids are memory addresses, not serialized offsets. The model text calls151 **“unused”**; this is not evidence of an ordinary factory, inactive source or permission to discard it.

| Observation | Static VA | RVA | PE file offset |
| --- | --- | --- | --- |
| Type151 model /side | `8243b8` /`8243c0` | `4243b8` /`4243c0` | `4229b8` /`4229c0` |
| Literal editor type /parameter delivery | `4040e2` /`4040ec` | `40e2` /`40ec` | `34e2` /`34ec` |
| Begin selector /return | `4051ad` /`4050e3` | `51ad` /`50e3` | `45ad` /`44e3` |
| Explicit brush comparison /writer | `4b5981` /`4b4c20` | `b5981` /`b4c20` | `b4d81` /`b4020` |
| Owner callback selector /target | `51dff7` /`51df1d` | `11dff7` /`11df1d` | `11d3f7` /`11d31d` |
| Physical-restore selector /false return | `52f240` /`52f1f0` | `12f240` /`12f1f0` | `12e640` /`12e5f0` |
| Independent landscape Pinnacle writer call | `53fb13` | `13fb13` | `13ef13` |

**EXE-OBSERVED — exact editor operation:**4040d5 tests category47;4040e2 writes literal151 to88ebdc and4040ec stores the decremented selected parameter at88eb68. Its meaning is not inferred. Type151's begin selector4051ad=16 selects4050e3. Continuous gesture delivery4051f0→40528d→4b5290 traverses its bounded stencil. Explicit151 comparison **4b5981** reads1156e00 and passes its boolean plusY/X through **4b5997→4b4c20**.

The writer calculates `border101d0c8+228×Y+X`, reads **f6a9e0**, and either clears bit02000000 at4b4c4a or sets it at4b4c62. It marks101c524 dirty and writes that same terrain cell. It creates no positive owner, rectangle, actor or resource source. The shared brush can clear an old object before editing; clearing old ownership is distinct from creating a151 body. Type151 does not take the listed terrain-refresh branches and returns through4b5cef; no refresh/stencil extent is assigned as a saved footprint.

**EXE-OBSERVED — saved source construction and archive:** accepted gesture begin404990 sets88ebd8. End4052a0 excludes170/171/176/150, retains151 and calls405378→4b1250. Its qualified accepted coordinate/environment path reaches4b24c9→42d540. Fresh exact subtype predicates select **42d360→51c620→51c660→51c958→51c96f→51c9a0**, actual cIndustrialBldg. Publication42d723 writes manager8c7630,42d730 assigns+b4 original ID and42d736 calls virtual+94→428aa0. The initializer stores status3, type, signed origin/reference and the model's explicit side0. Archive52e7c0→52e961→42d790 serializes this same manager via42dc60→77fd11, descriptor854438 and serializer51ce00→427430/51c3a0. No extra schema or parent/part relationship is inferred.

**EXE-OBSERVED — actual physical owner path:** the builder reads model side0 at4b186a; zero width/height passed4b21a5/4b21a6→4b7ca0 skip the preflight rectangle loop. Actual placement4b2557→4b11f0→virtual+100 reaches Industrial51dd20. Exact byte **51dff7=7** selects51df1d, whose51df52/51df53 forward the zero dimensions through51df61→4b72b0. After its existing bounds checks,4b7388/4b7390 jump4b7477, skipping positive ownerfc3750 at4b73ce and terrainf6a9e0 at4b73c2. This qualifies the saved source without proving a physical rectangle.

**EXE-OBSERVED — independent landscape area:**53fa20 independently reads raw02000000, object-bank priority and current generated occupancy. It computes its own canonical origin, then53fb0b/0d pass5×5 dimensions while **53fb11 passes owner ID0** to4b72b0 at53fb13. This terrain-driven landscape path does not use the saved151 ID as a body owner. It neither creates a parent/part relationship nor justifies assigning a5×5 square to every saved151 origin, including the corpus origins whose flag has since been cleared. Existing raw-terrain permissions and Pinnacle presentation remain unchanged.

**EXE-OBSERVED — lookup/undo/restore:** lookup403ec6 and point-undo53e583 use unsigned160..183 ranges;151 underflows that range and takes403ffd/53e634 without reading a selector byte or writing a point array. Existing grid snapshots/undo preserve terrain and other old grids. Standalone52f030 tests active status and calls52f1d0; exact byte **52f240=1** selects52f1f0, bypassing physical42d540/4b11f0 reconstruction. Skipped restore and model side0 are supporting evidence, not sole grounds for a marker exemption.

**UNRESOLVED:** the authorship/lifetime of each record, exact editor category/parameter labels and all surrounding validation, original edit/landscape lifecycle, arbitrary state variants and original gameplay/render parity. The evidence distinguishes a terrain-editor source from its independently generated landscape; no original operation is replayed.

## Central bounded implementation

**OPENEMPEROR CONSERVATIVE POLICY:** after unchanged generic positive SavedSquare handling, central `original_occupancy()` admits only **Industrial /type151 /status3 /side0 /subindex0 /Base4 /parent1 /NonHouseInfo1 /own0 /no typed Monument or GateHouse state → MarkerOrigin**. Reserve every exact validated source origin. This is conservative saved-source protection, not a physical one-cell building, region replay or5×5 original parent body.

All identity/index/reference/local/storage/diamond/off-map checks, complete-manager atomic publication and foreign physical/gate conflicts remain active. Distinct valid markers may share an origin as before. A valid prefix cannot hide a later unsupported state or bounds error. Road bits cannot override original protection. No raw road/building rule, OriginalRoadClassification, signed height, corridor/opening/edge, World object, source simulation, save authority/version, resource budget or frame/tick work changes.

The sole production source difference is the12-line151 switch branch. Parser, menu/preflight, renderer/resources/input, World/economy/navigation and persistence implementations are unchanged. All consumers use that single producer; audit, menu and starter contain no parallel admission list.

## Full corpus impact and compatibility

| First complete preparation result | Fresh baseline | Final |
| --- | ---: | ---: |
| Complete managers | 167 | 167 |
| Prepared full policies | 134 | **135** |
| Unknown occupancy | 28 | **26** |
| Known conflicts | 3 | 3 |
| Outside-map original footprint | 2 | **3** |
| Refused maps | 33 | **32** |

Only **MPWall1** newly prepares. All13 Type151 sources independently pass, reducing individual unknown records467→454. MP42 still first rejects Industrial173 ID/index13 at logical1,113,677; its separately diagnosed off-map164 sources remain unchanged. MP27 and Chang-an Sui retain their exact bounds first failures. Other unknowns, including173/188/149/176/187, are not admitted.

**MPWall1_S remains refused:** after its valid151 ID95, full preparation reaches Industrial163 **ID/manager101 /MFC106 /logical1,120,620 /Base4-parent1-state1-own0 /status3-side0-subindex0**, local **(138,117)**, storage **(167,146)**, reference **33,455**, terrain **0x00080005**, Objects0, signed H0. Its origin is outside the actual diamond; the off-map bit is also set. The existing geometry check fails first with **“active original footprint outside map”**. The full baseline single-record diagnosis already recorded this exact later error; it is newly exposed as first failure, not introduced or hidden. No partial policy or legacy fallback is published.

All **134 previous maps ×2 policies ×3 identical inputs =804 cases** retain complete canonical/component/gate/edge/fingerprint equality. Inputs include actual renderer-derived masks plus explicit zero/one masks. Roads/buildings/protection, both blockers, signed heights, claims/corridors/openings, cardinal transport edges and OriginalRoadClassification remain exact. All804 old policy-backed World/save/read/reconstruct/restore cases also remain equal.

All167 actual legacy masks and presentation asset/upload/RGBA metadata are equal. All **3,340 legacy profile/rule full World/save roundtrips**, including City11r1–4 and City16r1–2, are equal. A separate direct comparison confirms **3,638,584,379 actual framed bytes** are identical across all167 maps, including those804 policies and complete canonical/rewritten save data. Its maximum transient map pair is51,230,370 bytes, removed after each map; it takes419.510s locally and retains no multi-gigabyte output tree. SHA metadata is supplementary to direct byte equality. These are bounded original-input/technical controls, not full-corpus gameplay or graphical parity.

## Normal MPWall1 gameplay, F5/F9 and process restart

Two owned dummy/software processes run the unchanged actual Application/main with ignored read-only instrumentation delivering normal SDL keys/clicks. No World commands/snapshots, funds, goods, workers, residents or couriers are injected. **SCRIPTED APP EVIDENCE** is separate from native/human delivery.

The normal path is **New Sandbox→City-v16 rule4→MPWall1→Map rules checked→Empty City→Start**. Ordinary Space pauses at actual tick5. F1 selects source **(168,131)** and displays its actual raw Pinnacle terrain, Objects8, signed height0 and complete manager diagnosis; the fresh record and immutable policy establish its source/protection context. F1 is not claimed to identify original editor lifecycle. House and Road clicks reject “Protected original structure” with zero spending and full Snapshot unchanged.

Immediate neighbors are also blocked by actual terrain. A bounded pure permission search finds the nearby legal cell **(168,135)**, Manhattan distance4, and explicitly does not label it a free immediate neighbor. F1 confirms independent unprotected0x80/Objects0 ground; ordinary Road costs2 and ordinary removal frees it without refund. Original terrain/landscape remains unchanged.

The private planner derives its search objective from this freshly read source and validates every candidate against actual MPWall1 permissions. It chooses Houses **(153,129)/(156,129)**, Clay **(153,132)**, Pottery **(159,132)** and six necessary paid Roads **(154..159,131)**. No previous-map layout is copied. Construction totals **474** (460 buildings+12 connections+2 comparison road), leaving **826** ordinary Funds. Workforce supplies12 and assigns10. No prepared starter or goods/grants are injected.

At40 Clay has extracted1 and its real courier carries1/reserves1. At **45** it is ToWarehouse, vertex1/edge progress4. Actual F5 writes the full schema20 save, seven ordinary Period steps advance to **52**, then normal F9 restores the complete45 Snapshot/context; F5 reproduces the original entire file. Real F9 rejects wrong relative map, wrong original SHA and changed permission fingerprint while retaining the complete loaded World/context; the restored good private document loads normally.

Checkpoint **71** observes delivered cargo/reservation0 and Returning; **106** observes Idle after return. **206** observes the first completed Pottery recipe. At **605**, totals are18 Clay /6 Pottery, maintenance20, taxes0, Funds806. Food/Service/Water/Health and a profitable full city are not supplied or claimed.

The first App fully exits. A distinct process follows **Load Sandbox** for that actual45 save, starts paused and continues560 identical ticks. At45/71/106/206/605, all full typed Snapshot/context and entire serialized SaveDocuments are byte-identical to the first process, including buildings/roads, stocks/reservations, courier phase/path/progress, funds/population/workforce and map identity/policy fingerprint. Actual policy fingerprint is **`1edab958f92b6433660d81bcb4fe5adc05cfd2ffb6f734cf77a6240858d3254d`**.

Both processes retain1,736 eager uploads during rendering and release every owned texture on exit. Across343 measured Sandbox frames all12 prohibited-work counter deltas are zero. The new decision performs bounded constant work per source during loading and introduces no frame/tick I/O, parse/scan, decode/upload, World copy/command or navigation work.

## Regression and complete validation

Before the production change, an authored MPWall1-like Base4/source151/terrain02000080/Objects8/height0 case fails against frozen baseline libraries with **“unsupported active original occupancy …type151 …4/1/1”**, zero compile warnings. The exact source/library/test hashes and RED result remain recorded; the same regression passes afterward.

Positive tests cover the observed Base4 state under both policies. Negative and preservation tests cover wrong class/type/status/subindex, unsupported side17, Base3/5/other schemas, parent/state/own wrappers, contradictory typed states, missing/mismatched/duplicate IDs, duplicate manager index, reference/local/diamond/off-map errors, physical/gate solid/corridor/opening conflicts in both orders, valid duplicate-marker overlap, inactive records, late unknown atomicity, independent neighbor/raw/height/Road-bit priority and unchanged generic positive squares. Explicit unknown173/188/149/176/187 controls remain. The prior186 negative151 case is retained as an unevidenced NonHouse-state2 variant, rather than deleting a negative test. Compressed authored MapRules and normal-menu fixtures cover151, then reject later status2/truncated candidates while preserving the previous World/policy/files.

Focused Release tests pass **5/5**: OriginalMapEntities, SandboxMapPermissions, MapRules, MenuMapRules and MP34 shore visuals. The full configurations run sequentially and pass **438/438 executions**,146 registered cases each, with zero compiler warnings, failed/skipped tests, sanitizer findings or source changes. Existing timeouts are unchanged. Coverage includes original parsers/permissions, MapRules/Menu, original roads/gates/Tower/joins, MP56 saved-General/MP34 shore visuals, MP48 occupancy, budgets, save/recovery, City determinism and input/render controls.

| Configuration | Full CTest | Build | CTest elapsed |
| --- | --- | ---: | ---: |
| debug | **146/146 PASS** | 9.62s | 1262.45s |
| release | **146/146 PASS** | 3.34s | 47.6s |
| sanitizer | **146/146 PASS** | 13.73s | 174.19s |

Combined ASan/UBSan uses the existing RelWithDebInfo configuration with `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1` and `UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`. Build concurrency is2, CTest Debug2 /Release-sanitizer4. No test is skipped or timeout increased to mask load.

The excluded **Xia rule3/policy1/schema19** paid playthrough runs separately, sequentially in all three configurations. Its earned goal remains at6800, sustained through9200 with10 Houses /8 effectiveL2 /126 residents. All14 complete SaveDocuments and the full `main-facts.jsonl` match the preceding ResGate fixture byte for byte in every configuration. This is technical control evidence, separate from human acceptance.

| Xia configuration | Result | Replay elapsed | Historical complete SaveDocuments |
| --- | --- | ---: | --- |
| debug | **PASS** | 150.081s | **14/14 identical** |
| release | **PASS** | 3.826s | **14/14 identical** |
| sanitizer | **PASS** | 46.119s | **14/14 identical** |

Final source scope remains one production branch plus authored tests, documentation and the small starter. Full original-tree rehash after all tests/native launch retains1464 files /819,711,091 bytes and the starting manifest, with zero input changes. HEAD remains the starting commit; all changes remain local/uncommitted. `git diff --check` passes. Private reports retain logs/JUnit, exact source/library/command provenance, corpus/literal proofs, normal App/save/restart, native launch and first harness failures.


The first private planner required a free immediate source neighbor, which the actual Pinnacle terrain does not provide. This harness assumption is recorded separately and corrected to an explicitly distance4 comparison cell. No production bounds, terrain, input or rendering rule is relaxed. All other successful/failed build and harness logs remain private.

## Executed test start and limits

The exact starter has been created and executed from the repository root:

```sh
sh tools/test_mpwall1_occupancy.sh .local/gog-extracted/app
```

It verifies pinned MPWall1/EXE hashes, builds current Release, runs complete rule4/policy2 production preflight and opens the normal menu in a fresh private settings/save/recovery root. Choose **New Sandbox→MPWall1→Empty City→Start Sandbox**; verify rule4/policy2/schema20. Startup identifies the protected151 origin(168,131). The wrapper reuses `test_resgate_maps.sh`; no personal settings/saves, alternative map or legacy rule is substituted.

This exact countermap command also runs and returns exit1 without an App launch:

```sh
sh tools/test_resgate_maps.sh .local/gog-extracted/app MPWall1_S
```

It reports the existing outside-map error above. The full source diagnosis identifies ID101; the primary MPWall1 starter succeeds. Missing/wrong-revision inputs fail before native startup.

The uninstrumented native MPWall1 App is separately opened for user testing. This is launch evidence; technical scripted gameplay/save acceptance above is separate from human native-input/graphics acceptance. **UNRESOLVED:** unevidenced151 states, original edit/landscape lifecycle, the remaining32 refused maps (including MPWall1_S and MP42), whole-corpus gameplay/rendering, complete composition and human acceptance. Stop after151 and the measured corpus effect.

## Great Wall gate placeholders (2026-10-12)

The reported MPWall1 screenshot is reproduced by the current Automatic
presentation at the central Great Wall gate. Three source Gate components,
IDs27/28/29, cover12 cells at storage origins(90,118)/(88,121)/(90,121).
Their historical saved images resolve to present Earthen archive physical
records229/231/230, Type1,51x51,size0. The map renderer does not support this
layout, so it draws the existing flat diagnostics. These are supported
source/occupancy records with unsupported historical image layouts; the
Type151 occupancy change did not add these gaps.

The existing explicit Earthen selector prepares all53 Great Wall components
and740 claimed cells, including these three gates. A centered direct
production software-render comparison reproduces the screenshot's gap in
Auto and shows the complete gate in `preview-earthen`:12 versus0 visible
diagnostic cells. Ruined preview also prepares53 components and has0
diagnostic cells in this view. This is direct rendering evidence, separate
from native human acceptance and verified original restore context. The
other40 historical diagnostics elsewhere on the map remain unresolved.

To test the complete Earthen preview through the ordinary sandbox menu:

```sh
sh tools/test_mpwall1_occupancy.sh .local/gog-extracted/app preview-earthen
```

Choose **New Sandbox**, then **Start Sandbox**. The setup's Great Wall choice
is `preview-earthen`; it can also be selected there with the existing G
control. The complete sandbox-rule preflight remains unchanged. An explicit
choice receives a separate production headless render check and is passed
to the fresh private native menu session. Omitting it retains `auto`.
Unknown choices fail before build or launch. No renderer, source admission,
World, save, road, material inference or default presentation rule changes.
Automatic's Type1 historical fallback and original restore context remain
open.

The exact preview starter was executed: current Release build, complete
rule4 sandbox check, selected-presentation render check and a fresh native
menu launch all passed. Seven existing Great Wall/scene/texture cases,
four invalid-argument checks, shell syntax and `git diff --check` passed.
Native launch is not human gameplay or visual acceptance.


## MPWall1_S gate preview starter (2026-10-13)

Oliver's17:02 screenshot exposes the same three Great Wall Gate components on MPWall1_S, independently of the corrected quarry below. Starting clean HEAD: `6abfaa66b600746c4bc6d507d890d03f5f7e3743`. The exact map SHA-256 remains `2d0eeccb89ab22cfda9b0311aeb39ed4ad4846da1444005aac554d0175b19eae`. The12 unsupported historical Type1 cells occupy the same three origins(90,118)/(88,121)/(90,121); the inspected first source is original ID/manager27, with saved signed height4 at its existing height cell(90,119).

Fresh direct production software captures centered on this gate reproduce the brown fields in Automatic: **12 visible diagnostic cells**. Explicit `preview-earthen` has **zero visible diagnostic cells**, showing the complete gate and adjoining wall. All53 selected Great Wall components/740 cells activate atomically, while verified original-context instances remain zero. F1 reports `explicit_preview`, the active component and the original-context limitation. The quarry correction remains active. Unrelated historical diagnostics elsewhere are separate limitations.

The generic remaining-occupancy starter previously forwarded only `auto` and rejected a fourth argument. It now accepts the existing five presentation choices as an optional fourth argument for `--map`, forwarding that exact choice to the shared starter. Omission still selects `auto`; audit/conflict/bounds modes do not accept the extra argument. Unknown/empty choices and invalid argument counts fail before hashing, building or launching. This changes only the wrapper; no renderer, selector, World, rules, policy, save or inferred original mode changes.

The tested command for the screenshot's map is:

```sh
sh tools/test_remaining_occupancy.sh .local/gog-extracted/app --map MPWall1_S preview-earthen
```

Choose **New Sandbox → Start Sandbox**. The explicit Earthen choice is session presentation, not verified original restoration. Existing personal settings/saves are untouched.

Both this exact preview invocation and the omitted-choice/default invocation completed the actual build and full rule4/policy2 Empty City preflight, then launched separate owned native menu processes with the correct exact map, isolated app roots and CLI mode. The preview also passed the production headless presentation render check. Only those newly owned test processes were closed; no UI input was delivered and no human PASS is claimed. The policy fingerprint remains `65019ba395e125384f0b18813c4d1139b309905ffe31bb5d7a5eaa6415332e60` in both runs. Eight invalid-argument checks, seven focused production Great Wall/scene/texture tests, shell syntax and whitespace checks passed.

An initial private harness tried to force SDL's dummy driver through environment variables and conflicted with the Empty City check's explicit hint priority. That harness launch failed before any app opened. Repeating with clean normal-launch environment passed; no production behavior or timeout was changed to mask the harness failure. Local captures, F1 reports, preflight/launch evidence and logs remain ignored under `.local/mpwall1-s-gate-launch/`. Automatic historical fallback, complete restored Great Wall fidelity and native human acceptance remain open.
