# cResWall: bounded parser and original occupancy (2026-10-11)

This pass starts at clean HEAD `c5a3b71b7dac49ca4351b0e602a6ca8a39416a97`. It adds an exact, append-only `ResWall` class and a bounded physical reservation. It adds no wall body, military simulation or other original class. City-v16 **rule 4 / MapPermissions policy 2 / save schema 20**, historical rules/policy 1, roads, gates, Towers/joins, renderer, input, economy and save/recovery implementations remain unchanged.

**Result:** all five former first `cResWall` parser errors are passed, but **zero maps become newly playable**. Four managers now stop atomically at unknown `cResGate`; MP34 has a complete manager and supported ResWall claims, but still fails on unknown Industrial zero-side occupancy. The fresh corpus remains **102 prepared / 65 refused out of 167**. Parser success and physical protection do not establish a reconstructed wall or playable map.

## Baseline and immutable inputs

The freshly hashed data root has **1,464 files / 819,711,091 bytes**. The manifest SHA-256 is `dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`, using lexicographically sorted UTF-8 relative path, TAB decimal file size, TAB file SHA-256, LF. Before/after inventories are identical. This encoding is explicit; earlier documents using another manifest encoding retain their historical hashes.

The original PE32 EXE is read only, not executed: **4,809,081 bytes**, SHA-256 `6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e`. All source records, disassembly, generated saves, observer code and reports stay ignored under `.local/reswall/`. No original bytes or pixels are test fixtures or repository assets.

All five files are independently identified standalone **Map** parts, with their own physical hashes and declared sizes. Baseline indices are zero-based manager indices; offsets below are decompressed logical positions, not PE offsets.

| Exact relative path | Bytes | Size / border | First old manager index / logical offset | SHA-256 |
| --- | ---: | --- | --- | --- |
| `Cities/Chang-an Sui.map` | 133,748 | 226 / 1 | 11 / 1,096,984 | `98262361fc03f1e9cfee774afde576617ab7e30a34ff50ae26db402f91af9f71` |
| `Cities/MP34.map` | 90,195 | 140 / 44 | 1 / 1,093,808 | `356285c0d09cd6cac1234f67c7e22dd3062d985dd485c1994f156a2f7aa38a47` |
| `Cities/MP56.map` | 102,064 | 170 / 29 | 13 / 1,097,644 | `96e3bd832b40ae57a6e7526eb864cfa85a5bacd4dc3cad95aa7e4fff8dc17892` |
| `Cities/STGW-Xiangjun.map` | 156,211 | 140 / 44 | 2 / 1,094,140 | `5b3742fdf598345448277080a5e0864f0dfa1609f038ab5fed43da6661f864bb` |
| `Cities/Xiangjun.map` | 93,522 | 140 / 44 | 1 / 1,093,806 | `ed1389f93a238fb39a2e4e835cc6ab1e67c0830de90b4984a04a4639748f746b` |

Each baseline error names unsupported MFC class `cResWall`. No complete ResWall count is inferred from those first failures, nor from records behind the new `cResGate` failures. The known first failure offsets permit a bounded inspection of that one record only, without searching for later plausible tags.

## Serializer evidence and distinct address spaces

The PE image base is `0x400000`. Address translation uses the actual section table. A static VA, its RVA and corresponding file offset are separate from an original runtime object's field offset and a decompressed map-stream offset.

| Original observation | Static VA | RVA | PE file offset |
| --- | --- | --- | --- |
| cResWall MFC descriptor | `0x815f50` | `0x415f50` | `0x414550` |
| Exact class-name string | `0x815f8c` | `0x415f8c` | `0x41458c` |
| Saved class factory | `0x414170` | `0x14170` | `0x13570` |
| Constructor | `0x416cb0` | `0x16cb0` | `0x160b0` |
| ResWall wrapper initializer | `0x414130` | `0x14130` | `0x13530` |
| Final vtable | `0x7aaab8` | `0x3aaab8` | `0x3a9cb8` |
| Actual ResWall serializer | `0x415ae0` | `0x15ae0` | `0x14ee0` |
| NonHouse parent serializer | `0x51ce00` | `0x11ce00` | `0x11c200` |
| Building serializer | `0x427430` | `0x27430` | `0x26830` |
| Embedded NonHouseInfo serializer | `0x51c3a0` | `0x11c3a0` | `0x11b7a0` |
| Standalone physical restore | `0x52f030` | `0x12f030` | `0x12e430` |
| Physical type filter | `0x52f1d0` | `0x12f1d0` | `0x12e5d0` |
| Restore/placement callback | `0x4b11f0` | `0xb11f0` | `0xb05f0` |
| ResWall physical callback | `0x414830` | `0x14830` | `0x13c30` |
| Rectangle owner-grid writer | `0x4b72b0` | `0xb72b0` | `0xb66b0` |

**EXE-OBSERVED:** descriptor `0x815f50` names `cResWall`, registers MFC class schema **0**, a runtime size of `0x150`, factory `0x414170`, and parent descriptor `0x854438` (`cIndustrialBldg`). Runtime size is not a stream length. The factory/constructor chain `414170 → 416cb0 → 416cd0 → 414670 → 51c9a0` ends with vtable `7aaab8`; its serialization slot `+8` selects **415ae0**. That serializer calls **51ce00 directly**, then writes/reads its own schema word through `41fc10`. An intermediate constructor's virtual serializer `414690` is not an extra section in this stream.

The actual parent serializer calls Building, reads parent wrapper **1**, then the embedded NonHouseInfo object at runtime `+0xc8`; its `7b65d4` vtable selects `51c3a0`. It is inline fixed-width serialization, with no nested MFC object/class reference or variable count. ResWall's own schema object at VA `0x88f700` is initialized to **1**; it lies in zero-filled virtual storage and has no corresponding original file bytes. Its wrapper has no further payload.

The complete contract is:

| Section | Serialized width | Admitted schema / interpretation |
| --- | ---: | --- |
| Manager header | 6 | u16 schema 1; signed i32 count 0–4,000 |
| New MFC declaration | 14 for ResWall | u16 `0xffff`, u16 class schema 0, u16 length 8, exact eight-byte ASCII name |
| Reused class reference | 2 | Existing shared-table class index with `0x8000`; creates a fresh object |
| Building base | 179 / 181 / 182 | Includes u16 base schema 3 / 4 / 5 |
| NonHouse parent wrapper | 2 | Schema 1 |
| NonHouseInfo state | 128 | Schema 1 word plus exactly 126 opaque payload bytes |
| ResWall class wrapper | 2 | Schema 1, no payload |

Building's previously verified schema branches at `42856f / 428112 / 427cc2` and common ID tail `4289bd` remain exact. Relative to the serialized base, status is u8 at **2**, side u8 at **5**, local x/y signed i16 at **8/10**, cell reference signed i32 at **12**, type/subindex signed i16 at **16/18**, and original ID signed i32 at **base_length − 20**. These are existing `field_source` positions, not original runtime structure offsets. Other fixed base bytes remain under the existing contract; they are not new geometry inputs.

For NonHouseInfo schema 1, the independently summed runtime source ranges are `+04` ×2, `+06` ×72, `+4e` ×14, `+5c…+65` ten one-byte fields, `+66` ×2, `+68/+69` two one-byte fields, `+6a/+6c/+6e` three two-byte fields, `+70` ×16 and `+80` ×2: **126 payload bytes**. Field meanings/signs inside this payload are deliberately not claimed or used. The reader consumes those exact widths and preserves wrapper/section provenance; no new state fields are needed.

Consequently the ResWall payload after its MFC tag is **311 / 313 / 314** bytes. A new-class record is **325 / 327 / 328**, and a reused-class record **313 / 315 / 316**, for bases 3/4/5 respectively. Counts, references and the existing manager byte budget are unchanged. Inactive status-0 records consume the same complete payload before being excluded from typed source claims; existing raw terrain protection remains independent.

## Prefix boundaries, atomicity and synthetic evidence

`OriginalEntityClass::ResWall` is appended as **7**; existing `Tower` remains **6**. Exact `cResWall` recognition does not admit `cResGate` or a name containing “Wall”. The MFC class/object table remains shared: a new declaration adds a class entry and a fresh object entry; a reused class adds only the fresh object entry. Nulls, object aliases, class references to an object and unsupported tags remain errors. Duplicate active original identities remain rejected. No nested reference is hidden inside the fixed payload.

Independent authored streams assert literal manager lengths:

- Building3 → ResWall4 → referenced Building5: **710 bytes**, ResWall at manager-relative 199, following Building at 526, MFC object reference 5 and intact following ID/type/subindex/coordinates.
- ResWall3 → referenced ResWall4 → Tower5: **972 bytes**, reused ResWall record 315, following Tower object reference 5.
- GateHouse3/wrapper2 → ResWall4 → Industrial5: **1,001 bytes**, ResWall at relative 341 and MFC object reference 6.

`byte_length` ends at the manager, not the end of the supplied map window. An independently authored `de ad be ef` sentinel remains unconsumed and unchanged. Every byte-prefix truncation of each admitted active/inactive single-record schema fails; wrong base/parent/state/class schema throws `OriginalEntityUnsupported`, while truncation/count/reference/identity/overflow contradictions remain `OriginalEntityError`. An unknown following synthetic class rejects the entire call, never returning earlier records. Existing unknown-class tests retain their guarantee using `cUnknownSyntheticWall`.

The 4,000-record authored inactive schema-4 manager is independently **1,260,018 bytes**. In a private Release measurement, 100 full parses take 0.0216 s; 100 last-wrapper truncations take 0.0274 s; 100 unknown-last-class rejections take 0.0291 s. Both failing cases return **zero records**. Combined process peak RSS is **7,569,408 bytes** on this macOS arm64 run. These are local synthetic performance measurements, not a machine-independent speed guarantee. No budget was increased and no recovery scan was added.

## Bounded physical occupancy

**EXE-OBSERVED:** `52f030` checks nonzero status at `52f06d`. Its `52f1d0` filter selects the physical branch for types **90 / 91 / 231**. Their 24-byte type-table entries at VAs `823e00 / 823e18 / 824b38` all provide side **1** at entry `+8`. That side is read at `52f09a`, passed to `4b11f0`, then to the object's virtual `+0x100` callback. cResWall selects **414830**, which calls **4b72b0** at `414881`, passing the same positive side as both width and height (`41487c/41487d`). The bounded rectangle writer writes each physical owner to runtime grid **fc3750** at `4b73ce` and terrain to **f6a9e0** at `4b73c2`.

This establishes a real singleton physical reservation. It does not identify Raw Objects as an owner grid, nor infer a gate, NPC, military relationship or wall-top transport. The saved side must agree with this bounded physical state; zero side is never promoted to a 1×1 claim.

The central `original_occupancy` decision checks ResWall **before** generic positive-square handling. It admits only active **status 3 / subindex 0 / side 1 / type 90, 91 or 231 / base 3 or 4 / parent1 / NonHouseInfo1 / ResWall1**, with no Monument/GateHouse state. Base 5 is structurally parseable but remains unsupported for active occupancy. Other status/type/side/schema combinations, including positive side 2 and active zero side, fail explicitly. The existing validator still requires valid distinct identities/manager indices, exact local/storage origin and serialized reference, full actual-map bounds, no off-map bit and no conflicting foreign/gate claim. Signed height is retained; no new raw-terrain whitelist or Raw Objects ownership rule is invented.

The generic existing SavedSquare machinery reserves the admitted **one cell**, blocks House/Road placement and prevents policy-2 original-road import even if that cell carries a Road bit. Valid free neighbors retain their existing permissions. Synthetic tests exercise both policies, Road-bit protection, signed height −1, invalid state/reference/identity/bounds, both conflict orders, gate solids/corridors/openings and inactive-record preservation.

The original first failure records corroborate active side1 states: Chang-an Sui has type91/base3, MP56 type90/base4 and Xiangjun type90/base3. Their wrappers are all 1 and origins have raw `0x88` / Objects 4; MP56's signed height is 1, the others 0. STGW-Xiangjun's first ResWall is inactive status0/side0/type0/ID−1 and is fully consumed; it proves no active zero-side geometry. These are single known records, not complete-manager counts.

**RAW MAP FACT, MP34 complete manager:** 4,000 records, **744,557 manager bytes**, end **1,838,164**. Classes are Building 3,966, Ferry 4, Industrial 13, ResWall **17**. All 17 ResWall records are active type231/status3/side1/subindex0/base4/parent1/state1/own1, with raw terrain `0x88`, Objects 4, signed height 0 and exact origins/references. ID/manager 1 is at local **(81,59)**, storage **(125,103)**, reference **23,609**. Record/base/state/own-wrapper logical positions are **1,093,808 / 1,093,822 / 1,094,005 / 1,094,133**, record length **327**, MFC object reference 4. The subsequent reused records are 315 bytes. Individual source-claim validation reports no ResWall error; this diagnostic does not publish a subset policy or session.

MP34 still contains **five unknown Industrial179 and three Industrial164** records with side zero. The first is ID/manager22 at storage **(151,118)**, reference **27,055**, logical **1,108,481**, base4/parent1/state1, raw terrain `0x04000004`, Objects 0. The first type164 is ID/manager30 at **(110,50)**, logical **1,111,004**. Their geometry remains unresolved; this pass does not reclassify them as markers.

Tower/Gate ownership helpers use exact classes. A ResWall owner is **Other**, never Tower or GateHouse merely because the enum grew. New tests keep ResWall from producing a Tower body/join and confirm that a foreign overlapping ResWall reservation retains the Tower's existing safe fallback. No presentation implementation changed. Historical visuals remain the existing fallback; **ResWall body rendering remains unsupported**.

## Corpus and compatibility

The fresh audit is run before and after against all 167 original maps. Only complete managers produce class/state counts; no stream is resumed after an unknown class.

Complete manager parses increase from **162 to 163**. The accepted policy count does not increase: MP34's now-complete manager exposes a later independent occupancy refusal.

| First production outcome | Frozen baseline | Final |
| --- | ---: | ---: |
| Prepared full policy | 102 | **102** |
| Unknown occupancy | 57 | **58** |
| Known conflict | 3 | **3** |
| Unknown `cResWall` parser failure | 5 | **0** |
| Unknown `cResGate` parser failure | 0 | **4** |
| Total refused | 65 | **65** |

| Previously blocked file | Full manager now | Remaining first blocker | Full MapRules / new session |
| --- | --- | --- | --- |
| Chang-an Sui | Atomic failure | `cResGate`, manager20, logical 1,099,813 | Refused / none |
| MP34 | **MANAGER_PARSED** | Industrial179 occupancy, manager22, logical 1,108,481 | Refused / none |
| MP56 | Atomic failure | `cResGate`, manager21, logical 1,100,176 | Refused / none |
| STGW-Xiangjun | Atomic failure | `cResGate`, manager29, logical 1,102,633 | Refused / none |
| Xiangjun | Atomic failure | `cResGate`, manager28, logical 1,102,269 | Refused / none |

**OCCUPANCY_VERIFIED** describes the bounded ResWall source contract and MP34's 17 individual complete claims. It is not the entire MP34 manager's occupancy. None of these five files reaches **MAP_RULES_CHECKED**, **SESSION_STARTED** or **GAMEPLAY_SMOKE_PASSED** under rule4/policy2. There is no second newly supported map, paid mini-city or F5/F9/restart acceptance on one; forcing a legacy session would not satisfy this milestone.

All **102 previously prepared maps** retain the same accepted set. With identical original files and identical actual/zero/one legacy inputs, **3,366 policy-1** files and **1,224 policy-2** files are byte-identical. They include all road/building/protection masks, blocker categories, signed heights, complete gates/claims/corridors/openings/edges, canonical data and schema-19/20 fingerprint metadata. Policy2 still imports **10,370** original roads across the same 102 maps with identical classification. No save schema, version/default or migration implementation changes.

The separate older-profile comparison prepares the actual legacy renderer/mask on **all 167 maps**, including these five. Across **20 supported legacy profile/rule combinations**, including City-v11 rules1–4 and City-v16 rules1–2, each build completes **3,340 full empty-World save/read/restore roundtrips**. All **7,014** mask, prepared physical asset/resource metadata, full canonical World and full saved-document files are byte-identical between frozen baseline and final. These are preserved legacy contracts and technical empty-session/save comparisons, not rule4 permission to open a blocked map or native gameplay acceptance.

The final audit takes 31.25 s including inventory hashing, complete diagnostics and sequential actual decoder preparation, peak RSS **220,348,416 bytes**, zero rendered frames and zero Worlds created. Original input inventory remains unchanged. These preparation counts are not whole-corpus rendering/gameplay acceptance.

## Acceptance and test start

Focused Release CTest passes all five selected tests: originalmapentities, sandbox-map-permissions, map-rules-selected-service, menu-map-rules-selection and ordinary-tower-selector. Authored production-menu tests start a valid ResWall-only rule4/policy2 fixture, then reject a complete manager containing later Industrial179 and a truncated ResWall wrapper; full previous World/policy and persistent files remain unchanged, and Resume succeeds. These authored bytes are independent of original maps.

The current Release and combined ASan/UBSan full matrices each pass **145/145** CTests, zero skipped/failed tests and zero compiler warnings, with unchanged source hashes during each run. No sanitizer diagnostic is reported. The first concurrent Debug run executes all 145 tests: 143 pass, while City-v14/v15 endurance exceeds the existing 900-second timeouts under simultaneous configuration/build/test load. The same Debug binaries then pass both isolated serial rechecks in **280.56 / 306.60 s**, respectively. Thus all **145 unique Debug cases** pass, with no skipped case or source change. The original timeout logs remain recorded separately; no timeout or production budget is changed.

This includes parser/MapRules/menu, both policies, original roads, gate passages, Tower/body/joins, budget, save/recovery, deterministic City state and authored pixel/input regressions. Performance observations, synthetic fixtures, technical original-map replay and scripted App evidence retain their distinct labels.

An isolated scripted SDL run uses the unchanged current `Application/main`, a private read-only observer and dummy/software rendering. It starts **New Sandbox → City-v16 rule4 → Xia → Empty City → Start**, pauses through the ordinary Space key, then selects each of the five exact original files through the normal catalog. Every actual MapRules result is **UnsupportedForRules**, with the exact new blocker above; ordinary Enter cannot start it. The previous complete World, input hashes, policy fingerprint and policy pointer remain equal, and owned settings/save/recovery files are byte-identical. Normal Resume returns to the same paused Xia. The owned process exits normally after **312 frames / 22.94 s**. This is scripted App evidence, not native or human input acceptance; personal cities are not used.

The unchanged existing technical Xia fixture passes its earned **rule3 / policy1 / schema19** playthrough: first goal at **6,800**, sustained to **9,200** with ten Houses, eight effective L2 and 126 residents. Full snapshot determinism is checked after every tick and command. All **14 earned SaveDocuments** equal the earlier full-playthrough documents; the two differing report strings are only the private output directory paths. This preserves the existing control and does not claim gameplay acceptance on a ResWall map.

Because all five original maps retain independent blockers, no `test_reswall_maps.sh` starter is added or claimed. A concrete executed corpus command is:

```sh
.local/reswall/release-build/openemperor-audit-original-occupancy \
  .local/gog-extracted/app .local/reswall/final-corpus
```

The audit requires a fresh output directory; rerunning uses a new ignored output name. To inspect the actual menu, build the current Release app and open a fresh isolated root, choose **New Sandbox → City-v16 → MP34 → Empty City**. The expected result is **Unsupported for these rules**, naming Industrial179; Start remains disabled. Storage **(125,103)** is a verified ResWall source/physical reservation, not a playable MP34 placement test or a new renderer body. Use an already accepted map such as Xia for ordinary sandbox play.

This concrete executed check independently reproduces MP34's refusal, returning exit **1** with the Industrial179 source diagnostic:

```sh
.local/reswall/release-build/openemperor --data .local/gog-extracted/app \
  --sandbox Cities/MP34.map --sandbox-rules sandbox-city-v16 \
  --sandbox-check --report-json
```

All native/human acceptance remains separate from scripted SDL, synthetic fixtures and technical replay. No human ResWall-map PASS, complete Emperor restoration or publication is claimed. Stop at this bounded parser/occupancy/corpus result; `cResGate`, other zero-side types and ResWall rendering remain open.
