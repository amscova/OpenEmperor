# Original bounds compatibility — 2026-10-13

Clean starting HEAD: `9b671344e50f42d89f9ea80a90f27a79805da98f`. Fresh production census before edits: **167 complete managers, 160 complete policies, five bounds failures, two known conflicts, zero Unknown-Occupancy first failures**. All five bounds maps were investigated independently, in the requested order. Eight outside origins are valid saved nonphysical editor sources under six separately qualified type/state contracts. The final complete census prepares **165**, with **zero bounds first failures**, **two unchanged conflicts** and **zero unknown first failures**. No map-specific admission, clamping, shifted/deleted record, altered Diamond, terrain edit, editor replay, physical owner, Ferry passage or renderer change is introduced.

City-v16 rule **4**, MapPermissions policy **2**, save schema **20** and every older version retain their authority. Inside-Diamond marker origins retain their existing conservative protection. The new contract changes only independently evidenced zero-area sources outside the playable Diamond; it never extends playable terrain.

## Pinned inputs and evidence domains

Original revision: **1,464 files /819,711,091 bytes**, manifest SHA-256 `dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`. EXE SHA-256 `6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e` (4,809,081 bytes); Model SHA-256 `7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6` (34,218 bytes). Original files remain read only. Evidence, frozen source/libraries, complete source dumps, authored RED results, per-family censuses, private app roots and saves stay ignored under `.local/original-bounds/`.

**RAW MAP FACT** means independently reread archive/terrain/object/height fields, including logical byte widths and signedness. **EXE-OBSERVED** means bounded static disassembly/table bytes checked against the freshly hashed PE; the original EXE was not executed. **OPENEMPEROR CONSERVATIVE POLICY** means the narrower complete-manager admission described below. **UNRESOLVED** covers arbitrary editor histories, other saved states/schemas, original gameplay and full-scene/human parity. A zero model side or a failed Diamond check alone proves no admission.

Map logical offsets are decompressed part-0 offsets. They are neither physical compressed-map offsets nor PE addresses. The standalone manager starts at logical **1,093,607**. All five files contain manager schema **1**, **4,000 parsed records**; active counts are separate. Static PE image base is `0x400000`; tables below separate VA, RVA and physical PE offset. No runtime process address was observed.

## Coordinate and original bounds contract

**EXE-OBSERVED:** map setup `0x53ce60` and `0x534cde..0x534cf4` selects the original `0x856c64` context table. Its linear offset equals `border * (228 + 1)`. Therefore storage is `(local_x + border, local_y + border)` and the serialized cell reference is `storage_x + 228 * storage_y`. This agrees with all eight saved references. No border correction or off-by-one repair is needed.

| Context index | Declared size | Border | Linear storage offset | Extra context word |
| --- | ---: | ---: | ---: | ---: |
| 0 | 56 | 86 | 19694 | 172 |
| 1 | 84 | 72 | 16488 | 144 |
| 2 | 112 | 58 | 13282 | 116 |
| 3 | 140 | 44 | 10076 | 88 |
| 4 | 170 | 29 | 6641 | 58 |
| 5 | 226 | 1 | 229 | 2 |

Index 0's size56 is an original table observation, not newly supported OpenEmperor geometry. The existing supported-size set and `MapGeometry::contains()` remain unchanged. `0x4b05f0 → 0x4b0770/0x4b0800` establishes a separate Diamond test from the same context. Every problematic origin genuinely fails that original Diamond test too. MP27's origin has no Off-map bit; the two signals are independent.

**EXE-OBSERVED:** qualified source construction `0x4b1250` reads Model side at `0x4b186a` and passes dimensions to `0x4b7ca0` at `0x4b21a9`. All six exact types have Model side0. Preflight first retains rectangular origin margins: for dimensions0, **3 ≤ local_x/local_y ≤ DeclaredMapSize−3**. At `0x4b7d31/0x4b7d38`, height0 returns through `0x4b7e70` before entering the per-cell Diamond/terrain/owner loop (`0x4b7d81 → 0x4b0800`). Every problematic origin satisfies those original rectangular margins and the fixed228×228 storage bounds. This is a proven difference between valid saved source coordinates and physical playable claims.

The exact editor factory path `0x4052a0 → 0x405378 → 0x4b1250 → 0x42d540/0x42d360 → 0x51c620/0x51c660 → 0x51c9a0 → 0x428aa0` independently reaches Industrial for each type. Special-class predicates were checked for each exact type, rather than inferred from a type range. The initializer sets status3 and the actual model side, preserves local coordinates/reference and manager identity. Source delivery at `0x4b24b5..0x4b24c9 → 0x42d540`, then `0x4b2516 → 0x42b250`, does not translate the origin to a Diamond cell. Editor-end has no additional Diamond correction. The bounded `0x4ea7f0` region probe is actor-range work, not that Diamond check; no region or actor behavior is imported.

Each type's actual owner callback selects7 and reaches `0x51df1d → 0x4b72b0` with zero dimensions. `0x4b7390 → 0x4b7477` skips the positive-area owner store `0x4b73ce → 0xfc3750`; no positive owner rectangle is established. Each independently checked standalone physical filter `0x52f1d0` selects1 and reaches `0x52f1f0`, excluding that source from physical restore. `0x42d790` archive reading retains the source rather than re-running a physical placement or rewriting its coordinates. No raw Objects value is treated as the original runtime owner grid.

## Exact admission and independent functions

**OPENEMPEROR CONSERVATIVE POLICY:** `SandboxPlacement.cpp` is the only production authority. Existing `original_occupancy()` remains unchanged. After complete parsing and original ID/index/local/fixed-storage/reference validation, its MarkerOrigin path may contribute **zero playable cells** only for:

| Entity class | Type | Evidenced Base schema | Independent editor function |
| --- | ---: | ---: | --- |
| Industrial | 163 | 4 | Explicit brush `0x4b588a → 0x4b5a80 → 0x4b4860`; type subtraction at `0x4b48a3` distinguishes source flag1 |
| Industrial | 164 | 4 | Separate selection `0x4b5896 → 0x4b5a80 → 0x4b4860`; distinguishes flag2, not163's selection |
| Industrial | 165 | 4 | Water branch `0x4b58a2/0x4b58ac → 0x4b4af0`; point terrain/height/dirty work, no positive owner |
| Industrial | 185 | 4 | Ordinary Rock selection `0x4b58b9/0x4b58c5 → 0x4b48e0(2)` |
| Industrial | 187 | 4 | Independently selected Silver Rock `0x4b58e8 → 0x4b48e0(0x200002)` |
| Industrial | 162 | 3 | Dirt branch `0x4b5873/0x4b587d → 0x4b47b0`; may clear an old owner at `0x4b482d`, never replayed |

All require **status3 /side0 /subindex0 /parent1 /NonHouseInfo1 /class-wrapper0 with no wrapper field /no typed Monument or GateHouse state**, exact ID/index/reference, the original rectangular margin and an origin outside the unchanged Diamond. Base3 Dirt is independent of the existing Base4 Dirt/Ferry annotation. Inside-Diamond sources retain their previous singleton protection; inside-Diamond Off-map terrain still refuses. Unqualified outside bases/states/types, arbitrary zero-side records, malformed references and physical outside/partial claims stay strict; every prior inside-marker and physical admission remains exact. No source is removed from the parsed manager or converted to an editable World record. Its entire original identity/state remains inspectable in the read-only audit.

Editor writers can no-op on today's terrain, and Dirt's historical editor can remove existing bodies. Neither fact licenses replay or deletion. Terrain/objects/heights remain raw input. The boundary locations and terrain values below are facts, not selector conditions or a whitelist.

### Per-type model/owner/restore provenance

| Type | Model VA/RVA/PE | Owner selector7 VA/RVA/PE | Physical filter selector1 VA/RVA/PE |
| --- | --- | --- | --- |
| 163 | `0x8244d8/0x4244d8/0x422ad8` | `0x51e003/0x11e003/0x11d403` | `0x52f24c/0x12f24c/0x12e64c` |
| 164 | `0x8244f0/0x4244f0/0x422af0` | `0x51e004/0x11e004/0x11d404` | `0x52f24d/0x12f24d/0x12e64d` |
| 165 | `0x824508/0x424508/0x422b08` | `0x51e005/0x11e005/0x11d405` | `0x52f24e/0x12f24e/0x12e64e` |
| 185 | `0x8246e8/0x4246e8/0x422ce8` | `0x51e019/0x11e019/0x11d419` | `0x52f262/0x12f262/0x12e662` |
| 187 | `0x824718/0x424718/0x422d18` | `0x51e01b/0x11e01b/0x11d41b` | `0x52f264/0x12f264/0x12e664` |
| 162 | `0x8244c0/0x4244c0/0x422ac0` | `0x51e002/0x11e002/0x11d402` | `0x52f24b/0x12f24b/0x12e64b` |

The per-type model entries have six bounded words, including the observed side0. Full factory predicate results and byte-verification counts are in `evidence/verified-source-creation.json`; coordinate/model/callback/filter locations are in `evidence/source-and-coordinate-tables.json`. These are static observations, not original runtime execution or proof of an arbitrary saved-field/lifecycle combination.

## Five complete map investigations

All eight problematic records are Industrial/status3/side0/subindex0/parent1/NonHouseInfo1/absent class-wrapper0, without typed Monument/GateHouse state. All eight have valid local and fixed-storage origins, exact serialized references and original rectangular-margin validity; all eight have outside-Diamond origins. Their one-cell candidate is outside the Diamond; their evidenced physical area is zero. Raw Objects and signed Height at every problematic origin are **0**. The remaining active sources and every bounded candidate intersection were audited, not merely the first failure. No independent conflict exists in these five managers; allowed duplicate marker origins retain their existing behavior.

### MPWall1_S

**RAW MAP FACT:** `Cities/MPWall1_S.map`, physical bytes **108,967**, SHA-256 `2d0eeccb89ab22cfda9b0311aeb39ed4ad4846da1444005aac554d0175b19eae`. Declared size **170**, storage border **29**, manager schema1, **4,000 records /741,310 logical bytes /106 active sources**. Original first failure: `map permissions: active original footprint outside map`.

| Type/Base | ID /manager index /MFC ref | Local | Storage | Serialized reference | Terrain | Raw diagnosis |
| --- | --- | --- | --- | ---: | --- | --- |
| 163/4 | 101/101/106 | (138, 117) | (167, 146) | 33455 | `0x80005` | OUTSIDE_DIAMOND + OFFMAP_TERRAIN |

Logical provenance (all offsets decimal, record/base/parent/non-house schemas are u16; status/side u8; local/type/subindex i16; reference/ID i32):

| ID | Record start /length | Base /parent /NonHouseInfo | Status /side | Local X /Y | Reference /type /subindex /ID | Terrain /Objects /Height |
| --- | --- | --- | --- | --- | --- | --- |
| 101 | 1120620/313 | 1120622/1120803/1120805 | 1120624/1120627 | 1120630/1120632 | 1120634/1120638/1120640/1120783 | 395275/603211/1022730 |

**EXE-OBSERVED:** type163 independently follows the vegetation brush/source factory, zero-area owner callback and physical filter above. Source101 is not a wall body or a generated Great Wall component. **POLICY:** only the exact Base4 outside source loses a hypothetical playable singleton; its ID/index/ref/coordinates remain stored. Every inside marker, structural claim and saved Great Wall footprint retains its prior protection. No renderer or automatic Great Wall mode changes. **Result:** complete PREPARED, no remaining bounds or occupancy blocker.

Actual normal-session policy2 upper mask: **8,743 Road-allowed /8,554 Building-allowed cells /0 gates**. Canonical SHA-256 `24af6058adf7bd1b7dd69cb4597f527d9760f6fe47daf47b06ded447ef5918d1`; policy fingerprint `65019ba395e125384f0b18813c4d1139b309905ffe31bb5d7a5eaa6415332e60`. The outside origins retain OutsideMap road/building blockers and no protection/owner. Zero/one mask and policy1 preparations also pass; these counts do not establish full-map gameplay or visual parity.

### MP42

**RAW MAP FACT:** `Cities/MP42.map`, physical bytes **105,479**, SHA-256 `09a221568b006c72f6b9fcae58d4a027f666355eb6d6c8a9e779dfea6e70f6ed`. Declared size **170**, storage border **29**, manager schema1, **4,000 records /757,751 logical bytes /74 active sources**. Original first failure: `map permissions: active original footprint outside map`.

| Type/Base | ID /manager index /MFC ref | Local | Storage | Serialized reference | Terrain | Raw diagnosis |
| --- | --- | --- | --- | ---: | --- | --- |
| 164/4 | 58/58/62 | (111, 144) | (140, 173) | 39584 | `0x80005` | OUTSIDE_DIAMOND + OFFMAP_TERRAIN |
| 164/4 | 61/61/65 | (159, 96) | (188, 125) | 28688 | `0x80005` | OUTSIDE_DIAMOND + OFFMAP_TERRAIN |

Logical provenance (all offsets decimal, record/base/parent/non-house schemas are u16; status/side u8; local/type/subindex i16; reference/ID i32):

| ID | Record start /length | Base /parent /NonHouseInfo | Status /side | Local X /Y | Reference /type /subindex /ID | Terrain /Objects /Height |
| --- | --- | --- | --- | --- | --- | --- |
| 58 | 1127762/313 | 1127764/1127945/1127947 | 1127766/1127769 | 1127772/1127774 | 1127776/1127780/1127782/1127925 | 419791/627727/1028859 |
| 61 | 1128701/313 | 1128703/1128884/1128886 | 1128705/1128708 | 1128711/1128713 | 1128715/1128719/1128721/1128864 | 376207/584143/1017963 |

**EXE-OBSERVED:** both164 sources independently have the same exact Base4 state and type164's explicit branch/flag2, not a rule inferred from type163. Both saved references are correct. Their qualified zero-area source construction may retain these rectangular coordinates; physical restore skips their bodies. **POLICY:** existing valid164 Base3/4 origins remain conservatively protected, while only this exact Base4 outside contract contributes no playable cells. Neither origin is rounded to169/170. **Result:** both sources and the rest of the complete manager PREPARED; no later independent blocker.

Actual normal-session policy2 upper mask: **7,498 Road-allowed /7,380 Building-allowed cells /0 gates**. Canonical SHA-256 `965d3a740a906db1eb03217fd1d5c93c9410079d88466ff3e1ee6c7e09ac8fac`; policy fingerprint `6bc97570051f55aa410415eafa0d5fcb67646f641c66c8e6598d027a1470dc1c`. The outside origins retain OutsideMap road/building blockers and no protection/owner. Zero/one mask and policy1 preparations also pass; these counts do not establish full-map gameplay or visual parity.

### MP27

**RAW MAP FACT:** `Cities/MP27.map`, physical bytes **89,475**, SHA-256 `d4604b72b5fd3ea93ab02d323126798816582e0a500c8750cb576828ea3ebc64`. Declared size **140**, storage border **44**, manager schema1, **4,000 records /733,727 logical bytes /13 active sources**. Original first failure: `map permissions: active original footprint outside map`.

| Type/Base | ID /manager index /MFC ref | Local | Storage | Serialized reference | Terrain | Raw diagnosis |
| --- | --- | --- | --- | ---: | --- | --- |
| 165/4 | 13/13/16 | (111, 40) | (155, 84) | 19307 | `0x4000004` | OUTSIDE_DIAMOND |

Logical provenance (all offsets decimal, record/base/parent/non-house schemas are u16; status/side u8; local/type/subindex i16; reference/ID i32):

| ID | Record start /length | Base /parent /NonHouseInfo | Status /side | Local X /Y | Reference /type /subindex /ID | Terrain /Objects /Height |
| --- | --- | --- | --- | --- | --- | --- |
| 13 | 1097583/313 | 1097585/1097766/1097768 | 1097587/1097590 | 1097593/1097595 | 1097597/1097601/1097603/1097746 | 338683/546619/1008582 |

**RAW MAP FACT:** source13 is outside the Diamond but its `0x04000004` terrain has **no** `0x80000` Off-map bit. **EXE-OBSERVED:** independently explicit165 Water editor, qualified zero-side factory/owner/preflight and separate physical filter establish a saved terrain provenance source, not a water-route body. **POLICY:** Base4 source13 remains complete with its original coordinates/ref; no Water brush, height write, route or terrain conversion is replayed. Inside165 Base3/4/5 contracts stay exact. **Result:** complete PREPARED; no further source or conflict blocker.

Actual normal-session policy2 upper mask: **4,416 Road-allowed /4,300 Building-allowed cells /0 gates**. Canonical SHA-256 `5a2eacc82ef7c6bcd640637e392b335976cf6947124f7ae15e801486f417b145`; policy fingerprint `a179e6c4748ab1c30f1d92707eacf245810400575f642da0435a3ba4fbb0eccb`. The outside origins retain OutsideMap road/building blockers and no protection/owner. Zero/one mask and policy1 preparations also pass; these counts do not establish full-map gameplay or visual parity.

### Chizhou

**RAW MAP FACT:** `Cities/Chizhou.map`, physical bytes **101,161**, SHA-256 `d332981e298e53a11dc6f7a7a462ce80a1cd85ccffa99074bdd7aca7b34de07a`. Declared size **140**, storage border **44**, manager schema1, **4,000 records /762,847 logical bytes /237 active sources**. Original first failure: `map permissions: active original footprint outside map`.

| Type/Base | ID /manager index /MFC ref | Local | Storage | Serialized reference | Terrain | Raw diagnosis |
| --- | --- | --- | --- | ---: | --- | --- |
| 185/4 | 45/45/48 | (97, 113) | (141, 157) | 35937 | `0x80085` | OUTSIDE_DIAMOND + OFFMAP_TERRAIN |
| 187/4 | 53/53/56 | (87, 123) | (131, 167) | 38207 | `0x80085` | OUTSIDE_DIAMOND + OFFMAP_TERRAIN |
| 187/4 | 56/56/59 | (90, 120) | (134, 164) | 37526 | `0x80085` | OUTSIDE_DIAMOND + OFFMAP_TERRAIN |

Logical provenance (all offsets decimal, record/base/parent/non-house schemas are u16; status/side u8; local/type/subindex i16; reference/ID i32):

| ID | Record start /length | Base /parent /NonHouseInfo | Status /side | Local X /Y | Reference /type /subindex /ID | Terrain /Objects /Height |
| --- | --- | --- | --- | --- | --- | --- |
| 45 | 1107599/313 | 1107601/1107782/1107784 | 1107603/1107606 | 1107609/1107611 | 1107613/1107617/1107619/1107762 | 405203/613139/1025212 |
| 53 | 1110103/313 | 1110105/1110286/1110288 | 1110107/1110110 | 1110113/1110115 | 1110117/1110121/1110123/1110266 | 414283/622219/1027482 |
| 56 | 1111042/313 | 1111044/1111225/1111227 | 1111046/1111049 | 1111052/1111054 | 1111056/1111060/1111062/1111205 | 411559/619495/1026801 |

**EXE-OBSERVED:**185 ordinary Rock was investigated independently from187 Silver Rock. Their explicit selections pass different flags to the shared terrain writer; both independently reach the Industrial factory, exact zero-dimension owner callback and physical filter. Three saved sources pass the original rectangular source bounds; none establishes a physical owner rectangle. **POLICY:** only185/Base4 and187/Base4 exact outside states contribute zero playable claims. Source45 and distinct sources53/56 remain present, while all valid Rock/Silver sources and other structural/marker protection remain unchanged. **Result:** complete PREPARED only after both independently qualified families; adding185 alone did not prepare the map.

Actual normal-session policy2 upper mask: **4,197 Road-allowed /4,122 Building-allowed cells /0 gates**. Canonical SHA-256 `477c8a73e65698835972270c14fac6dd62bd077e16a3113f05bcabe20fe0ec31`; policy fingerprint `77425f875171f48f5fe8aae38febfccb648c393dcb2170ba3ae20bfea75aab5c`. The outside origins retain OutsideMap road/building blockers and no protection/owner. Zero/one mask and policy1 preparations also pass; these counts do not establish full-map gameplay or visual parity.

### Chang-an Sui

**RAW MAP FACT:** `Cities/Chang-an Sui.map`, physical bytes **133,748**, SHA-256 `98262361fc03f1e9cfee774afde576617ab7e30a34ff50ae26db402f91af9f71`. Declared size **226**, storage border **1**, manager schema1, **4,000 records /731,827 logical bytes /59 active sources**. Original first failure: `map permissions: active original footprint outside map`.

| Type/Base | ID /manager index /MFC ref | Local | Storage | Serialized reference | Terrain | Raw diagnosis |
| --- | --- | --- | --- | ---: | --- | --- |
| 162/3 | 29/29/36 | (210, 96) | (211, 97) | 22327 | `0x80005` | OUTSIDE_DIAMOND + OFFMAP_TERRAIN |

Logical provenance (all offsets decimal, record/base/parent/non-house schemas are u16; status/side u8; local/type/subindex i16; reference/ID i32):

| ID | Record start /length | Base /parent /NonHouseInfo | Status /side | Local X /Y | Reference /type /subindex /ID | Terrain /Objects /Height |
| --- | --- | --- | --- | --- | --- | --- |
| 29 | 1102653/311 | 1102655/1102834/1102836 | 1102657/1102660 | 1102663/1102665 | 1102667/1102671/1102673/1102814 | 350763/558699/1011602 |

**RAW MAP FACT:** source29 uses Base3. The complete current manager no longer has a Type188 unknown-state blocker: its four active188 sources use the already qualified current contract and individually pass. **EXE-OBSERVED:**162's Dirt writer is independently selected, with original old-owner clearing behavior that is not replayed. Qualified source placement/callback/restore is zero-area despite retaining the saved origin. **POLICY:** only exact Base3 outside Dirt; the existing Base4 Dirt/Ferry210 same-anchor association remains unchanged. Three validated GateHouse passages remain complete. **Result:** complete PREPARED, no independent188/state/geometry blocker.

Actual normal-session policy2 upper mask: **15,793 Road-allowed /15,255 Building-allowed cells /3 gates**. Canonical SHA-256 `0cc9abb687452e46125ce18ada4c077ef03f8b90a16e18e4dd6770e5245a0d5d`; policy fingerprint `6f8a9ff381660f4ea35014db227bd0c6e389c46a953334f4ba1134b85e508e98`. The outside origins retain OutsideMap road/building blockers and no protection/owner. Zero/one mask and policy1 preparations also pass; these counts do not establish full-map gameplay or visual parity.

## Corpus progression and conflict controls

| Pass | Independent exact outside family | Prepared | Bounds first failures | Conflicts | Unknown first failures |
| --- | --- | ---: | ---: | ---: | ---: |
| Baseline | Unchanged clean HEAD | 160 | 5 | 2 | 0 |
| 1 |163/Base4 |161 |4 |2 |0 |
| 2 |164/Base4 |162 |3 |2 |0 |
| 3 |165/Base4 |163 |2 |2 |0 |
| 4 |185/Base4 |163 |2 |2 |0 |
| 5 |187/Base4 |164 |1 |2 |0 |
| 6 |162/Base3 |165 |0 |2 |0 |

Each family had an independently authored RED archive/permissions regression before production edits, failing with the original outside-map error. Each GREEN family was followed by a complete167-map census and all960 old policy1/2 actual/zero/one summary/fingerprint comparisons. The ordering above follows the requested map investigation order, with Chizhou's185/187 introduced independently. No partial subset contributes a PREPARED map.

**UNRESOLVED / refused:** Luoyang WJin and MPcanal4 keep exactly `map permissions: conflicting complete original occupancy`. Their respective seven Dirt/GateHouse/Tower conflicts and Dirt61/62/Monument19 conflicts remain strict. Luoyang Gate110's invalid origin/reference is unchanged. MPcanal4's six independently diagnosed bounds sources remain unqualified in this pass (including185/Base3); an outside contract for185/Base4 does not admit them. See [the preceding conflict report](original-occupancy-conflicts.md). No app or replacement map starts for a refused policy.

## Regression and normal-app acceptance

The frozen clean-HEAD executable/libraries and final candidate compare **4,133,994,557 literal bytes**, not merely digests: **960 old policy cases** (160 maps ×2 policies ×actual/zero/one masks), **3,340 legacy World/save roundtrips** (167 maps ×20 supported older profile/rule combinations) and the old policy World/save roundtrips. Compared components include Road/Building masks and blockers, OriginalProtection, signed heights, OriginalRoadClassification, full gates/footprints/corridors/openings, all four transport-edge blockers, canonical permissions and fingerprints, complete canonical World states and written/read-back SaveDocuments. Each bounded map pair is compared directly in memory and its own transient files removed afterward; maximum transient pair51,230,370 bytes. All167 actual legacy masks, original map hashes and presentation preparation counts/bytes remain unchanged. Presentation metric equality and unchanged renderer code do not establish original pixel parity.

Authored RED/GREEN fixtures cover each admitted family through the real complete manager parser; both policies and both raw Off-map states; retained source identity; unchanged empty/neighbour policy; strict inside-Diamond Off-map refusal; alternate status/subindex/side/base/wrappers/typed states; missing/duplicate identity/index; wrong reference; negative/overflow/fixed-storage/local/rectangle bounds; large/physical outside area; unqualified other zero-side type; foreign physical claims; a separately validated Gate with distinct IDs/indexes followed by an exact Gate-solid conflict; late unknown state; and manager truncation. Normal preflight/menu tests prove whole-manager success and atomic retention on subsequent unsupported/truncated selections. No older negative fixture was removed.

**Scripted technical acceptance:** five unchanged normal Application/main processes use private dummy/software roots and ordinary SDL input, following New Sandbox →City-v16 rule4/exact map →Map rules checked →Empty City →Start Sandbox. A separate read-only planner uses actual complete permissions and pure validation to choose each legal layout; it performs no command or state injection. All eight outside sources retain their exact read-only state, have OutsideMap road/building blockers, no protection/owner and no selectable ground. In-map markers still reject road/building input; independent legal ground permits ordinary paid roads and buildings. Original pixels and user windows were not used as authority.

| Map | Independently queried two-House layout origin | Normal start /build /F5-F9 /new-process continuation | Outside records retained | Final Clay/Pottery | Final funds /population /workers assigned |
| --- | --- | --- | ---: | --- | --- |
| Chang-an Sui | (182, 87) | PASS (scripted) | 1 | 18/6 | 806/12/10 |
| Chizhou | (109, 144) | PASS (scripted) | 3 | 18/6 | 806/12/10 |
| MP27 | (129, 78) | PASS (scripted) | 1 | 18/6 | 806/12/10 |
| MP42 | (123, 152) | PASS (scripted) | 2 | 18/6 | 806/12/10 |
| MPWall1_S | (151, 136) | PASS (scripted) | 1 | 18/6 | 806/12/10 |

For every map, commands normally buy2 Houses, Clay Source, Pottery and the six required roads; an independent legal road placement/removal checks ordinary input first. Construction spending is474, funds826 before upkeep; workers10 assigned from12 residents. Actual dispatch is tick40; at tick45 courier1 has cargo1/reservation1, owner3/target4, path vertex1/edge progress4 and phase1. Delivery at71, return at106 and Pottery at206 are observed, rather than inferred from equal saves. At605, each has18 Clay extracted/6 Pottery completed/funds806/maintenance20. This small producer test does not claim a supplied/profitable full city; no Food/Market/Service/Well/Health infrastructure is injected.

At each actual checkpoint, ordinary F5 writes the full document; the app advances, F9 restores it, and complete WorldSnapshot/SaveDocument context equality is checked. A new normal-menu process loads the actual in-flight45 save, advances to the same checkpoints through605 and matches every complete snapshot/document, including map SHA, policy fingerprint, roads/buildings, stocks/reservations, courier phases/path/edge progress, funds/population/workforce. All50 written/re-written checkpoint SaveDocuments across the five restarts are additionally literal byte-identical to their normal-process files. In each map, actual F9 refuses wrong map path, changed original SHA and changed permissions fingerprint while retaining the full current World/document; the original owned good file then loads. Original sources introduce no editable save records. Paused status/overlay frames retain World equality and all12 tracked render counters at zero; shutdown has zero live textures. The observer's full captures stay outside the production frame/tick path.

**Native launch only:** the exact versioned `--map` commands below each pass complete production Empty City preflight and open the normal native menu under a fresh private root. Only the five newly owned PIDs were then closed without saving; no pre-existing OpenEmperor process was present and no other app/root was controlled. No native UI automation, human approval or human playtest is claimed. Human visual/input acceptance and original full-scene parity remain open.

**Initial attempts retained:** the authored RED fixture initially had a mixed-auto declaration/narrowing compile issue; the corrected build precedes all six intentional RED bounds failures. A mistyped target name and a nonexistent diagnostic constant were corrected in separate build logs. New preflight fixtures were corrected to use the actual Base3 length179/ID offset159 and a genuine unsupported138/status2 source rather than expecting legacy marker-state behavior to change. An initial final Debug build was stopped by a new test-only conversion warning, fixed with an explicit bounded cast. Review then found the new Gate-solid negative could fail on duplicate IDs/indexes; that owned matrix run was interrupted, the test first proved the distinct Gate's successful preparation, and its final refusal now checks the actual occupancy-conflict reason. Production admission stayed unchanged. Chizhou's first scripted F1 control hit a neighbouring drawn Rock source; all bounds assertions had passed. The retry uses five independent ordinary163 in-map source controls while retaining all three outside sources in its audit/ground/permission checks. It passes the full paid/save/restart sequence. No camera, picking, renderer, terrain or source-coordinate correction was made. All first logs are retained separately; no failed run is relabelled PASS.

Final matrix configurations ran serially **Debug →Release →ASan/UBSan**, with unchanged registered tests and timeout values. Every registered test executed; no skips, compiler warnings, sanitizer failures or source changes occurred. Sanitizer uses the existing RelWithDebInfo configuration with `OPENEMPEROR_ENABLE_SANITIZERS=ON` and the established macOS `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1`, `UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`; explicit texture ownership checks still pass.

| Configuration | Executed /registered | Failed | Skipped | CTest seconds |
| --- | ---: | ---: | ---: | ---: |
| debug |146/146 |0 |0 | 1254.73 |
| release |146/146 |0 |0 | 54.56 |
| sanitizer |146/146 |0 |0 | 183.51 |

The separate excluded-target Xia paid goal/stability playthrough then ran serially in all three configurations. Each reproduced all **14 historical written SaveDocuments literally byte-for-byte** and the complete historical `main-facts.jsonl`, with unchanged source hashes and zero warnings. Tests cover original entities/occupancy/roads, gates/joins/Great Wall, shore/general visuals, budget, World determinism, all Save/Recovery contracts and input/render controls. Native/human input or full-corpus gameplay acceptance is not inferred from these matrices.

Fresh **post-test** production167-map census again returns **165 prepared /2 exact conflicts /0 bounds /0 unknown-first**, including actual normal-session readiness for all165. All160 preceding summary/fingerprint contracts still match the frozen baseline. It rehashes every original input before and after the run; all1,464 files /819,711,091 bytes retain the complete pinned manifest SHA. Original assets, EXE, Model and map files are unchanged. The final census takes56.153 seconds, peak RSS236,929,024 bytes, with zero rendered frames/Worlds; diagnostics run only on demand. Final source files match both the frozen validation source set and the five-map private normal-app build. HEAD remains the starting commit; no commit, push, tag, release or publication was made.



## Reproducible test starter

The existing versioned runner retains `--audit`, `--map` and `--conflicts` and adds bounded `--bounds`. It verifies the complete original revision, obtains exact stems through production MapCatalog, and uses only the production parser/complete validator. Bounds output includes every active source, original identity/state/logical field locations, local/storage/reference, independent Diamond/Off-map facts, raw candidate geometry, actual single-source error/classification, complete-manager readiness and bounded pair diagnostics. Diagnostic subsets never publish a World or partial policy. `OUTSIDE_DIAMOND` remains a raw fact even for a valid zero-area source. An accepted outside classification follows production success, not a permissive diagnostic allowlist.

```sh
sh tools/test_remaining_occupancy.sh .local/gog-extracted/app --audit
sh tools/test_remaining_occupancy.sh .local/gog-extracted/app --bounds "MPWall1_S"
sh tools/test_remaining_occupancy.sh .local/gog-extracted/app --bounds "MP42"
sh tools/test_remaining_occupancy.sh .local/gog-extracted/app --bounds "MP27"
sh tools/test_remaining_occupancy.sh .local/gog-extracted/app --bounds "Chizhou"
sh tools/test_remaining_occupancy.sh .local/gog-extracted/app --bounds "Chang-an Sui"
sh tools/test_remaining_occupancy.sh .local/gog-extracted/app --map "MPWall1_S"
sh tools/test_remaining_occupancy.sh .local/gog-extracted/app --map "MP42"
sh tools/test_remaining_occupancy.sh .local/gog-extracted/app --map "MP27"
sh tools/test_remaining_occupancy.sh .local/gog-extracted/app --map "Chizhou"
sh tools/test_remaining_occupancy.sh .local/gog-extracted/app --map "Chang-an Sui"
```

For each accepted `--map`, the complete rule4/policy2 Empty City preflight runs first, then the normal native menu opens in a fresh `.local/original-bounds/` app root. Choose **New Sandbox → Empty City → Start Sandbox**; verify **rule4 /Policy2 /Save20**. Existing settings, saves and Recovery histories are untouched. Automatic Great Wall presentation remains historical; `preview-earthen` remains a separately explicit preview. All output/QA roots are isolated from the original data. A refused source or changed original revision starts no app, fallback map or older policy.

## Static address index

These are the bounded decision sites used above. VA/RVA/physical PE offset are independent of logical map field offsets. The freshly hashed PE section mapping converts each address; bounded disassembly windows were byte-checked before use.

| Meaning | Static VA | RVA | Physical PE offset | Section |
| --- | --- | --- | --- | --- |
| Editor entry | `0x4052a0` | `0x52a0` | `0x46a0` | .text |
| Qualified call | `0x405378` | `0x5378` | `0x4778` | .text |
| Source builder | `0x4b1250` | `0xb1250` | `0xb0650` | .text |
| Model-side read | `0x4b186a` | `0xb186a` | `0xb0c6a` | .text |
| Preflight call | `0x4b21a9` | `0xb21a9` | `0xb15a9` | .text |
| Source delivery | `0x4b24b5` | `0xb24b5` | `0xb18b5` | .text |
| Factory call | `0x4b24c9` | `0xb24c9` | `0xb18c9` | .text |
| Placement callback | `0x4b2516` | `0xb2516` | `0xb1916` | .text |
| Industrial factory | `0x42d540` | `0x2d540` | `0x2c940` | .text |
| Factory worker | `0x42d360` | `0x2d360` | `0x2c760` | .text |
| Base factory | `0x51c620` | `0x11c620` | `0x11ba20` | .text |
| Industrial factory continuation | `0x51c660` | `0x11c660` | `0x11ba60` | .text |
| Base factory continuation | `0x51c9a0` | `0x11c9a0` | `0x11bda0` | .text |
| Base initializer | `0x428aa0` | `0x28aa0` | `0x27ea0` | .text |
| Archive manager | `0x42d790` | `0x2d790` | `0x2cb90` | .text |
| Source placement | `0x42b250` | `0x2b250` | `0x2a650` | .text |
| Region probe | `0x4ea7f0` | `0xea7f0` | `0xe9bf0` | .text |
| Owner callback dispatcher | `0x51dd20` | `0x11dd20` | `0x11d120` | .text |
| Zero-dimension owner branch | `0x51df1d` | `0x11df1d` | `0x11d31d` | .text |
| Owner grid writer | `0x4b72b0` | `0xb72b0` | `0xb66b0` | .text |
| Zero-height owner bypass | `0x4b7390` | `0xb7390` | `0xb6790` | .text |
| Owner return | `0x4b7477` | `0xb7477` | `0xb6877` | .text |
| Positive-area owner store | `0x4b73ce` | `0xb73ce` | `0xb67ce` | .text |
| Restore entry | `0x52f030` | `0x12f030` | `0x12e430` | .text |
| Restore dispatcher | `0x52f1d0` | `0x12f1d0` | `0x12e5d0` | .text |
| Physical restore exclusion | `0x52f1f0` | `0x12f1f0` | `0x12e5f0` | .text |
| Preflight | `0x4b7ca0` | `0xb7ca0` | `0xb70a0` | .text |
| Zero-height preflight test | `0x4b7d31` | `0xb7d31` | `0xb7131` | .text |
| Preflight bypass branch | `0x4b7d38` | `0xb7d38` | `0xb7138` | .text |
| Preflight success | `0x4b7e70` | `0xb7e70` | `0xb7270` | .text |
| Positive-area Diamond call | `0x4b7d81` | `0xb7d81` | `0xb7181` | .text |
| Diamond initialization | `0x4b05f0` | `0xb05f0` | `0xaf9f0` | .text |
| Diamond test preparation | `0x4b0770` | `0xb0770` | `0xafb70` | .text |
| Diamond physical test | `0x4b0800` | `0xb0800` | `0xafc00` | .text |
| Map context setup | `0x53ce60` | `0x13ce60` | `0x13c260` | .text |
| Map context read | `0x534cde` | `0x134cde` | `0x1340de` | .text |
| Map context table | `0x856c64` | `0x456c64` | `0x455264` | .data |
| 163 explicit brush | `0x4b588a` | `0xb588a` | `0xb4c8a` | .text |
| 164 explicit brush | `0x4b5896` | `0xb5896` | `0xb4c96` | .text |
| Tree shared branch | `0x4b5a80` | `0xb5a80` | `0xb4e80` | .text |
| Tree writer | `0x4b4860` | `0xb4860` | `0xb3c60` | .text |
| Type distinction | `0x4b48a3` | `0xb48a3` | `0xb3ca3` | .text |
| 165 explicit brush | `0x4b58a2` | `0xb58a2` | `0xb4ca2` | .text |
| 165 writer call | `0x4b58ac` | `0xb58ac` | `0xb4cac` | .text |
| Water writer | `0x4b4af0` | `0xb4af0` | `0xb3ef0` | .text |
| 185 explicit brush | `0x4b58b9` | `0xb58b9` | `0xb4cb9` | .text |
| 185 writer call | `0x4b58c5` | `0xb58c5` | `0xb4cc5` | .text |
| 187 explicit writer call | `0x4b58e8` | `0xb58e8` | `0xb4ce8` | .text |
| Rock terrain writer | `0x4b48e0` | `0xb48e0` | `0xb3ce0` | .text |
| 162 explicit brush | `0x4b5873` | `0xb5873` | `0xb4c73` | .text |
| 162 writer call | `0x4b587d` | `0xb587d` | `0xb4c7d` | .text |
| Dirt writer | `0x4b47b0` | `0xb47b0` | `0xb3bb0` | .text |
| Dirt old-owner clear | `0x4b482d` | `0xb482d` | `0xb3c2d` | .text |
| Existing object test | `0x4b582c` | `0xb582c` | `0xb4c2c` | .text |
| Existing owner branch | `0x4b7870` | `0xb7870` | `0xb6c70` | .text |
| Existing body removal | `0x4afdf0` | `0xafdf0` | `0xaf1f0` | .text |

`0xfc3750` is the statically named absolute owner-grid location used by the store instruction, outside the PE raw sections: no physical PE offset or observed runtime-address claim is assigned. No original bytes or pixel assets accompany this document. The changes stop at these five bounds maps, their six qualified state families and the measured corpus effect; the two conflict maps and other unsupported source states remain outside implementation scope.
