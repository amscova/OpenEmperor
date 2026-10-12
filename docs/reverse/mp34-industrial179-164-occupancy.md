# MP34: bounded Industrial179/164 occupancy support

The later [MP34 shore landing visual pass](../rendering/mp34-shore-landing-visuals.md) resolves the four evidenced saved land-side Ferry images while preserving their independent existing wooden piers. It changes only historical presentation; all occupancy, policy/fingerprint, World and save contracts below remain unchanged. It adds no Ferry or water-transport simulation.

This pass starts at clean HEAD **`5803db65cf3702ef4759d6a8b112d3115b8745c6`**. It addresses only independently evidenced saved Industrial179 and Industrial164 occupancy. City-v16 **rule4 / MapPermissions policy2 / save schema20**, policy1 and older versions remain unchanged, without migration. No original fishing, resource brush, NPC, spawn, production or transit function is imported. Parser, terrain, original roads, gates/walls/Tower presentation, historical saved-General presentation, World/economy, renderer/input and save/recovery retain their existing authority. No commit or publication is performed.

**Validation status: bounded MP34 support complete.** The complete before/after corpus comparison, normal MP34 menu start, paid delivery, persistence/restart and full Debug/Release/ASan+UBSan matrices pass:435/435 registered tests, plus all three direct excluded Xia controls with14/14 full SaveDocuments and complete main facts byte-identical to the prior fixture. Human gameplay/graphics and unevidenced original states remain separately open.

## Fresh baseline and inputs

A fresh production-reader audit rechecks all **167 complete managers**, **121 prepared policies** and **46 refusals**: **41 unknown-occupancy first failures, three known conflicts and two existing outside-map origins**. These are fresh measurements, not the historical Industrial177/178 report copied forward. All 1,464 original files / 819,711,091 bytes have input manifest SHA-256 `dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb` (sorted relative path, TAB, decimal size, TAB, SHA-256, LF). All legacy/save comparisons subsequently pass. The first private audit’s additional derived-cache equality expectation produced a harness failure and is retained separately: authoritative full WorldSnapshots and rewritten SaveDocuments are the roundtrip contract; both pre- and post-restore canonical hashes are also compared across the two code versions.

| Read-only original input | Physical bytes | SHA-256 |
| --- | ---: | --- |
| `Cities/MP34.map` | 90,195 | `356285c0d09cd6cac1234f67c7e22dd3062d985dd485c1994f156a2f7aa38a47` |
| `Emperor.exe` | 4,809,081 | `6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e` |
| `Model/EmperorBuildingModels.txt` | 34,218 | `7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6` |

Hashes are checked before use. The EXE is read only and never executed. Original bytes, disassembly, pixels, captures and local evidence remain ignored under `.local/mp34-occupancy/`.

## All eight MP34 source records

**RAW MAP FACT:** MP34 declares size **140**, storage border **44**. Its schema1 manager at decompressed logical **1,093,607** reads all **4,000 records / 744,557 bytes**, ending at **1,838,164**. Complete class counts are Building3,966, Industrial13, Ferry4 and ResWall17; there is no parser/class gap. The baseline full policy first refuses Industrial179 ID/index22. Individual complete-manager diagnosis independently rejects all eight rows below as `Unsupported active original occupancy`; later rows are not inferred from the first error.

All eight are class **`cIndustrialBldg` (Industrial)**, **status3 / subindex0 / saved side0**, **Base4 / parent1 / NonHouseInfo1**, with **no class-specific wrapper (provenance0)** and no typed Monument/GateHouse state. Their complete origins are on the actual map diamond. Manager indices are zero-based, saved IDs and MFC references remain distinct identities.

| Type | Saved ID / manager index / MFC ref | Status / subindex / side | Base / parent / NonHouse / own | Local → Storage | Serialized cell ref | Terrain / Objects / signed H | Baseline single-record result |
| ---: | --- | --- | --- | --- | ---: | --- | --- |
| 179 | 22 / 22 / 27 | 3 / 0 / 0 | 4 / 1 / 1 / absent | (107,74) → **(151,118)** | 27,055 | `0x04000004` / 0 / +0 | Unsupported occupancy |
| 179 | 23 / 23 / 28 | 3 / 0 / 0 | 4 / 1 / 1 / absent | (89,112) → **(133,156)** | 35,701 | `0x04000004` / 0 / +0 | Unsupported occupancy |
| 179 | 24 / 24 / 29 | 3 / 0 / 0 | 4 / 1 / 1 / absent | (85,107) → **(129,151)** | 34,557 | `0x04000004` / 0 / +0 | Unsupported occupancy |
| 179 | 25 / 25 / 30 | 3 / 0 / 0 | 4 / 1 / 1 / absent | (83,98) → **(127,142)** | 32,503 | `0x04000004` / 0 / +0 | Unsupported occupancy |
| 179 | 26 / 26 / 31 | 3 / 0 / 0 | 4 / 1 / 1 / absent | (119,83) → **(163,127)** | 29,119 | `0x04000004` / 0 / +0 | Unsupported occupancy |
| 164 | 30 / 30 / 35 | 3 / 0 / 0 | 4 / 1 / 1 / absent | (66,6) → **(110,50)** | 11,510 | `0x00000081` / 2 / +2 | Unsupported occupancy |
| 164 | 31 / 31 / 36 | 3 / 0 / 0 | 4 / 1 / 1 / absent | (69,8) → **(113,52)** | 11,969 | `0x00000081` / 2 / +2 | Unsupported occupancy |
| 164 | 34 / 34 / 39 | 3 / 0 / 0 | 4 / 1 / 1 / absent | (33,102) → **(77,146)** | 33,365 | `0x00000200` / 0 / +0 | Unsupported occupancy |

The records are complete saved sources; neither side0 nor water/current Objects determines whether they may be discarded. In particular ID34's terrain `0x200` is a raw independent fact, not permission to replay a Bamboo brush or infer its old area.

### Exact stream provenance

The following are **decompressed logical map offsets**, not compressed-file offsets, VAs or RVAs. Base4 has 181 bytes including its schema; parent1 follows it, and NonHouseInfo1 follows that parent word. Industrial has no trailing class-specific wrapper. The first Industrial declaration has a longer MFC tag; subsequent shared-class references are two bytes.

| ID | MFC record / Base / parent1 / NonHouseInfo1 starts | Record bytes | Status / side offsets | X / Y offsets | Cell ref / type / subindex offsets | Original ID offset |
| ---: | --- | ---: | --- | --- | --- | ---: |
| 22 | 1,108,481 / 1,108,502 / 1,108,683 / 1,108,685 | 332 | 1,108,504 / 1,108,507 | 1,108,510 / 1,108,512 | 1,108,514 / 1,108,518 / 1,108,520 | 1,108,663 |
| 23 | 1,108,813 / 1,108,815 / 1,108,996 / 1,108,998 | 313 | 1,108,817 / 1,108,820 | 1,108,823 / 1,108,825 | 1,108,827 / 1,108,831 / 1,108,833 | 1,108,976 |
| 24 | 1,109,126 / 1,109,128 / 1,109,309 / 1,109,311 | 313 | 1,109,130 / 1,109,133 | 1,109,136 / 1,109,138 | 1,109,140 / 1,109,144 / 1,109,146 | 1,109,289 |
| 25 | 1,109,439 / 1,109,441 / 1,109,622 / 1,109,624 | 313 | 1,109,443 / 1,109,446 | 1,109,449 / 1,109,451 | 1,109,453 / 1,109,457 / 1,109,459 | 1,109,602 |
| 26 | 1,109,752 / 1,109,754 / 1,109,935 / 1,109,937 | 313 | 1,109,756 / 1,109,759 | 1,109,762 / 1,109,764 | 1,109,766 / 1,109,770 / 1,109,772 | 1,109,915 |
| 30 | 1,111,004 / 1,111,006 / 1,111,187 / 1,111,189 | 313 | 1,111,008 / 1,111,011 | 1,111,014 / 1,111,016 | 1,111,018 / 1,111,022 / 1,111,024 | 1,111,167 |
| 31 | 1,111,317 / 1,111,319 / 1,111,500 / 1,111,502 | 313 | 1,111,321 / 1,111,324 | 1,111,327 / 1,111,329 | 1,111,331 / 1,111,335 / 1,111,337 | 1,111,480 |
| 34 | 1,112,256 / 1,112,258 / 1,112,439 / 1,112,441 | 313 | 1,112,260 / 1,112,263 | 1,112,266 / 1,112,268 | 1,112,270 / 1,112,274 / 1,112,276 | 1,112,419 |

Status/side are unsigned one-byte fields; X/Y/type/subindex are signed two-byte fields; cell reference and original ID are signed four-byte fields. Relative to the schema word, offsets are **2/5, 8/10, 12/16/18, 161** respectively. These differ from runtime member offsets **+04/+07, +0a/+0c, +10/+14/+16, +b4**. Every emitted logical offset, width, signedness and record-relative offset was checked against the current parser's field provenance; references equal `228 × storageY + storageX` and coordinates equal local+44.

| ID | Raw Terrain logical offset | Raw Objects logical offset | Signed Height logical offset |
| ---: | ---: | ---: | ---: |
| 22 | 369,675 | 577,611 | 1,016,330 |
| 23 | 404,259 | 612,195 | 1,024,976 |
| 24 | 399,683 | 607,619 | 1,023,832 |
| 25 | 391,467 | 599,403 | 1,021,778 |
| 26 | 377,931 | 585,867 | 1,018,394 |
| 30 | 307,495 | 515,431 | 1,000,785 |
| 31 | 309,331 | 517,267 | 1,001,244 |
| 34 | 394,915 | 602,851 | 1,022,640 |

Terrain/Objects are four-byte grid words at their existing layer starts261,455/469,391 plus four times the validated reference. Height is one signed byte at989,275 plus that reference. Its source is frozen and supplies no new occupancy/type authority.

## Independent original evidence and address domains

The pinned PE32 image base is **0x400000**. `.text` RVA0x1000 maps to file0x400; `.data` RVA0x414000 maps to file0x412600. Type179 independently has **14 fresh bounded objdump windows / 446 instruction lines / 1,569 bytes**; type164 independently has **21 bounded instruction-byte verification windows / 845 lines / 3,028 bytes**. Each is checked against the hash-pinned EXE with zero mismatches. Common class/restore windows overlap between these independent sets; these are not deduplicated unique-code counts. No broad new decompilation is performed.

| Observation | Static VA | RVA | PE file offset |
| --- | ---: | ---: | ---: |
| Saved Industrial descriptor | `0x854438` | `0x454438` | `0x452a38` |
| Saved Industrial factory | `0x51c140` | `0x11c140` | `0x11b540` |
| Industrial constructor | `0x51c9a0` | `0x11c9a0` | `0x11b9a0` |
| Shared initializer | `0x428aa0` | `0x28aa0` | `0x27ea0` |
| Industrial serializer | `0x51ce00` | `0x11ce00` | `0x11c200` |
| Type179 model / side operand | `0x824658` / `0x824660` | `0x424658` / `0x424660` | `0x422c58` / `0x422c60` |
| Type179 selection type write | `0x404378` | `0x4378` | `0x3778` |
| Type179 placement selector / target | `0x4051c9` / `0x404ddd` | `0x51c9` / `0x4ddd` | `0x45c9` / `0x41dd` |
| Type179 lookup selector / target | `0x40403b` / `0x403f79` | `0x403b` / `0x3f79` | `0x343b` / `0x3379` |
| Type179 undo selector / target | `0x53e68f` / `0x53e5f0` | `0x13e68f` / `0x13e5f0` | `0x13da8f` / `0x13d9f0` |
| Type179 restore selector | `0x52f25c` | `0x12f25c` | `0x12e65c` |
| Type164 model / side operand | `0x8244f0` / `0x8244f8` | `0x4244f0` / `0x4244f8` | `0x422af0` / `0x422af8` |
| Type164 selection type write | `0x4041cd` | `0x41cd` | `0x35cd` |
| Type164 begin selector / target | `0x4051ba` / `0x4050e3` | `0x51ba` / `0x50e3` | `0x45ba` / `0x44e3` |
| Type164 explicit brush comparison | `0x4b5896` | `0xb5896` | `0xb4c96` |
| Type164 cell writer | `0x4b4860` | `0xb4860` | `0xb3c60` |
| Type164 point-undo selector / default | `0x53e680` / `0x53e634` | `0x13e680` / `0x13e634` | `0x13da80` / `0x13da34` |
| Type164 restore selector | `0x52f24d` | `0x12f24d` | `0x12e64d` |
| Standalone restore / physical filter | `0x52f030` / `0x52f1d0` | `0x12f030` / `0x12f1d0` | `0x12e430` / `0x12e5d0` |
| False physical-return branch | `0x52f1f0` | `0x12f1f0` | `0x12e5f0` |

Runtime globals such as `c5cd60/c5cd70`, `f6a9e0`, `f37da0` and `fc3750` describe original in-memory coordinates/grids, not file offsets. Map offsets in the previous tables are a separate decompressed-stream domain.

### Shared class/initialization evidence

**EXE-OBSERVED:** descriptor854438 supplies exact `cIndustrialBldg`, MFC class schema0, object size336 and saved factory51c140→51c9a0. Its constructor installs vtable7b65e4 and embedded NonHouse state. Virtual+94 selects428aa0, which writes runtime type+14, active status3 at+04, and the type model's side byte at+07; it stores the supplied signed origin X/Y at+0a/+0c and row-major reference at+10. Both independently read model entries deliberately have side0. The existing51ce00→427430 base/parent/inline NonHouse serializer is retained. This is not a new class, synthetic parser exemption or evidence that every zero-side Industrial has the same semantics.

Model text names179 `BUILD_MAP_FISHING_POINT` and164 `BUILD_MAP_BAMBOO_TREES`. These corroborate the distinct paths below; names, adjacent type numbers, current terrain and saved graphic decode are not occupancy authority.

### Industrial179: independently selected indexed coordinate point

**EXE-OBSERVED:** editor selection40434f..404389 chooses type179 at404378, mode0x18 and slot88ec24 with an eight-slot UI bound. The `(type−22)` dispatcher independently reads4051c9=12→404ddd. This exact point branch tests raw bit4 and calls its bounded coordinate/terrain helpers4b0800,4b0770 and4ba330. It reads the selected old signed pair, records type/zero-based slot/old X/Y through53e520→53e4f0, then writes current X/Y at404e88/404e90 to **c5cd5e+2×slot / c5cd6e+2×slot**. One-based slots1..8 correspond to signed-short arrays **c5cd60/c5cd70**, indexed0..7. No owner rectangle or physical placement writer is called in that bounded point branch.

Independent lookup byte40403b=6→403f79 reads those same indexed arrays; independent undo byte53e68f=6→53e5f0 restores them. This establishes an indexed coordinate point independently from Industrial177/178. Eight current editor slots do not authorize dropping older saved source records or limiting a complete manager to eight records. The bounded path establishes no extra regional/parent physical claim; uninterpreted NonHouse fields are not generalized into other states.

### Industrial164: independently selected terrain/resource editor source

**EXE-OBSERVED:** selection4041b3..4041cd separately selects mode1 and literal type164. Begin-dispatch byte4051ba=16→4050e3 returns; it does not select a special point-global writer. Continuous editor delivery4051f0→40528d→4b5290 traverses a bounded selected stencil and validates each candidate coordinate. The explicit type164 branch **4b5896→4b5a80→4b4860** supplies X/Y/type164 to the complete cell writer. Its index is **runtime border101d0c8 +228×Y+X**. Under its existing eligibility guards it explicitly distinguishes164 from163, sets resource bit0x2 in **f37da0**, updates independent terrain **f6a9e0**, and sets dirty state; it writes no new positive owner ID to **fc3750** and allocates no saved building rectangle.

The common brush may remove an already-present physical object before an edit; that does not create a type164 owner footprint. The subsequent type164 refresh rectangle4b5be5→4b5cd1→4b5d50→53e870 is presentation/update extent, not occupancy geometry. A stencil or refresh region cannot define a square around the saved source origin.

Independent shared undo snapshots/restores existing terrain/resource/owner/graphics/height grids. Point-global undo byte53e680=9→53e634 takes the default without a type164 coordinate-slot write. Type164 therefore has different editor semantics from179: its saved common-initializer origin remains a bounded source fact, while edited terrain/resources persist separately. No Bamboo resource or editor history is replayed by OpenEmperor.

### Typed saved-source creation, actual owner callback and archive coupling

The separate editor actions above are followed by a shared, independently checked source-record path. Fresh creation verification covers **46 bounded windows / 1335 instruction lines / 4172 bytes**, zero mismatches. Independent164 verification checks37 windows /2,567 lines /7,554 bytes and two boundary decodings. These overlapping counts are separate verification scopes, not unique original-code totals.

| Observation | Static VA | RVA | PE file offset |
| --- | ---: | ---: | ---: |
| Editor gesture end | `0x4052a0` | `0x52a0` | `0x46a0` |
| Common accepted source builder | `0x4b1250` | `0xb1250` | `0xb0650` |
| Accepted source insertion call | `0x4b24c9` | `0xb24c9` | `0xb18c9` |
| Original entity factory / insertion | `0x42d540` | `0x2d540` | `0x2c940` |
| Exact class selection | `0x42d360` | `0x2d360` | `0x2c760` |
| Industrial subfactory | `0x51c660` | `0x11c660` | `0x11ba60` |
| Manager publication / ID / initializer call | `0x42d723` | `0x2d723` | `0x2cb23` |
| Zero-side placement preflight | `0x4b7ca0` | `0xb7ca0` | `0xb70a0` |
| Common placement callback | `0x4b11f0` | `0xb11f0` | `0xb05f0` |
| Industrial physical callback | `0x51dd20` | `0x11dd20` | `0x11d120` |
| Actual owner rectangle writer | `0x4b72b0` | `0xb72b0` | `0xb66b0` |
| Standalone archive call | `0x52e961` | `0x12e961` | `0x12dd61` |
| Original manager serializer | `0x42d790` | `0x2d790` | `0x2cb90` |
| Manager archive object delivery | `0x42dc60` | `0x2dc60` | `0x2d060` |
| MFC object serialization | `0x77fd11` | `0x37fd11` | `0x37f111` |

**EXE-OBSERVED:** accepted editor begin404990 sets active gesture88ebd8 before the distinct179/164 operation. End4052a0 excludes170/171/176/150, retains either exact selected179 or164 and calls405378→4b1250. Only its accepted, valid-coordinate/environment path reaches4b24c9→42d540; no universal success for every editor input is claimed. Exact class selection42d360→51c620→51c660 independently evaluates the subtype predicates for both types; both select fallback51c958→51c96f→51c9a0, creating the actual `cIndustrialBldg`, rather than the unrelated42d480 creator. Manager publication42d723 installs the source in runtime manager8c7630;42d730 supplies original ID+b4;42d736 calls virtual+94→428aa0 with the selected type and supplied coordinates. This directly connects the common initializer to typed saved-source construction.

The common builder reads model side0 at4b186a. Its4b21a5/4b21a6 preflight passes zero width/height to4b7ca0, which performs bounds checks but skips its rectangle loop through4b7d38→4b7e70. Actual subsequent placement4b2557→4b11f0→virtual+100 resolves Industrial callback51dd20. Independently read selectors51e013(179)/51e004(164)=7 choose51df1d;51df52/51df53 pass the unchanged zero side as both dimensions through51df61→4b72b0. After coordinate validation,4b7388/4b7390 skips directly4b7477. Owner writefc3750 at4b73ce and terrain writef6a9e0 at4b73c2 are inside that skipped loop. Thus a typed saved source is created, but no positive physical owner rectangle is written for this exact zero-side path. This is stronger than only observing no direct owner call inside the point/brush branch.

The standalone archive path52e7c0→52e961→42d790 serializes the **same runtime manager8c7630**. Manager delivery42dc60→77fd11 uses object virtual0→51c1a0 returning descriptor854438, and virtual8→51ce00 supplies the already bounded Industrial serializer. Runtime source identities and common origin/reference fields therefore feed the saved record; no new parser section or second whitelist is invented.

**UNRESOLVED:** exact authorship/gestures and lifetime of each individual MP34 record, all surrounding original editor validation outcomes, resource/actor behavior and full original restore history. The accepted shared path and separate per-type operation do not justify replaying a brush or dropping older fishing-point records. Runtime manager/global addresses above are memory domains, never decompressed map offsets.

### Standalone physical-owner exclusion, independently checked for each type

**EXE-OBSERVED:**52f030 checks saved active status at52f06d, supplies saved runtime type+14 to52f1d0 at52f07d, and branches52f087→52f15c when the filter is false. Type179 byte**52f25c=1** and type164 byte**52f24d=1** independently select false-return**52f1f0**. Both bypass physical reconstruction**42d540** and owner-placement dispatch**4b11f0**. This filter alone would not prove harmlessness for arbitrary saved variants. Its combination with each distinct creation/coordinate or brush path, exact model side and independently observed saved state supports the narrow conservative source-origin policy.

## All corpus state combinations

**RAW MAP FACT:** complete managers contain **37 type179 records across nine maps**, all **Base4**, and **83 type164 records across nine maps**, **Base3:31 / Base4:52**. Every row has exact Industrial/status3/side0/subindex0/parent1/NonHouseInfo1/class-wrapper absent0. No zero-side Base5 record or other active state for either type is evidenced.

| Type | Base | Records | Exact class / status / side / subindex / parent / state / own |
| ---: | ---: | ---: | --- |
| 179 | 4 | 37 | Industrial / 3 / 0 / 0 / 1 / 1 / absent0 |
| 164 | 3 | 31 | Industrial / 3 / 0 / 0 / 1 / 1 / absent0 |
| 164 | 4 | 52 | Industrial / 3 / 0 / 0 / 1 / 1 / absent0 |

Type179 maps: **Chizhou, Luoyang Tang, MP34, MP39, MP48, MPWall1, MPWall1_S, SA-Linzi, SA-Qufu**. Type164 maps: **Juchengshi, MP15, MP34, MP42, MP43, MPcanal4, SAO-Chengdu, SAO-Guangzhou, SAO-Luoyang**. Matching a supported source state does not imply that each full map prepares; unrelated unknowns/conflicts remain decisive.

Type179 raw terrain is04000004 in all37, Objects vary0/1/2 (33/3/1), and Height0 occurs37 times. Type164 terrain varies00000081/00000004/00000200/20400200/00000080/00080005 (74/1/1/1/4/2); Objects0/1/2 occur9/2/72; signed Height0/1/2 occurs74/4/5. These are separate raw facts, not new admission conditions. In particular origin Objects and terrain may differ from their historical editor placement.

## Bounded central decision

**OPENEMPEROR CONSERVATIVE POLICY:** after unchanged positive `SavedSquare` handling, only **Industrial179/Base4** or **Industrial164/Base3,4**, status3/side0/subindex0/parent1/NonHouseInfo1/class-wrapper0 and no typed Monument/GateHouse state may use existing `MarkerOrigin`. Reserve every complete validated origin as `OriginalStructure`, without inventing a physical 1×1 building or replaying an eight-slot array/brush. Saved type/model names, map names, IDs, example coordinates, current water/Objects/height and rendered pixels never select this policy.

Complete original identity/index/reference consistency, local-to-storage conversion, actual map diamond/off-map, full manager atomicity, foreign physical/gate solid/corridor/opening conflicts and existing duplicate-marker rules remain mandatory in both source orders. A later unrelated unsupported record still rejects the whole policy. Protection precedes original-road import; Road bits never free a protected point. Water/terrain and neighboring independent claims retain their existing permissions, equal signed-height restrictions and transport edges.

Industrial177/178, ResWall/ResGate, GateHouse/Tower bodies/passages/joins, ordinary original roads and the MP56 saved-General `presentation_only` exclusion remain frozen. No new MapPermissions/save version or automatic migration is introduced. Classification adds only bounded load-time work; there is no per-frame/tick parse, scan, I/O, World copy/command/BFS, resource decode/upload or budget increase.

**UNRESOLVED:** zero-side179 Base3/5; zero-side164 Base5; different active statuses/subindices/classes/parent/state/typed-body variants; original fishing/resource/NPC/actor/editor-history lifecycle and whole original scene fidelity. Industrial188, Sui and MP27 bounds blockers stay outside this pass. Generic positive squares keep their independent existing contract.

## Regression and completion evidence

**RED BEFORE FIX:** two separate authored exact Base4/status3/side0 source tests run against unchanged baseline production code and independently fail with `Unsupported active original occupancy`, exit1, one for179 and one for164. Their synthetic coordinates are deliberately different from MP34; raw terrain is not the admission criterion. Full failing logs are kept in `.local/mp34-occupancy/authored-before-fix.json`.

The final fixtures with exact MP34 origin raw values are also rechecked against the frozen baseline central object:179 terrain `0x04000004`/Objects0, and164 terrain `0x81`/Objects2/signed Height2 each independently fail with that same Unsupported result and exit1. This separate red evidence is retained in `.local/mp34-occupancy/authored-exact-raw-before-fix.json`; the working production source is never reset or overwritten.

The focused permission/preflight/menu suite passes **3/3**. Independent fixtures assert literal protected/free neighbors, both policies, both evidenced164 bases and179 Base4, actual MP34 raw origin states, independent terrain/Objects/signed heights, and no road/gate creation. Wrong class, active status, subindex, schema/wrapper/typed state, identity/index/reference/coordinates/diamond/off-map, physical and gate solid/corridor/opening overlaps in both record orders, later188 atomic failure, duplicate markers, inactive records and prior positive SavedSquare handling remain covered. Existing179/164 unknown fixtures are retained as unsupported status2/state2 variants; general unknown tests use176/188. Compressed MapRules and normal menu fixtures admit both source points, reject a later188 or truncated state, and preserve the retained complete World/policy/files/Resume/textures.

The first full Release run executes145 registered tests and fails only `menu-map-rules-selection`: its old unknown179/164 fixtures have become valid states. That first failure is kept as `first-release-*`; the fixtures are corrected as described above. An attempted focused build also used a nonexistent target name; the actual `openemperor-map-rules-check-tests` target was then built and the focused checks passed. Neither failure is hidden or represented as a production acceptance result.

### Fresh complete corpus and compatibility

The same immutable167 maps are read before/after. The ignored audit uses the existing production parser/renderer/producer; its hash-only output adapter changes storage of evidence, never admission or claim validation. Each complete policy canonical stream, all cell components, legacy mask, original-road classifications, gate footprint/corridor/openings and all cardinal transport blockers are hashed in full with byte counts. Complete WorldSnapshots and map/policy context are compared directly at restore; rewritten full SaveDocument bytes must match. At most two short-lived private saves are removed immediately. No original pixels/bytes enter Git, no new build tree or large canonical dump is retained.

| Complete-manager/full-policy result | Before | After |
| --- | ---: | ---: |
| Complete managers | 167 | 167 |
| Fully prepared policies | 121 | **130** |
| Unknown occupancy first failures | 41 | **32** |
| Known conflict first failures | 3 | **3** |
| Outside-map first failures | 2 | **2** |
| Refused maps | 46 | **37** |
| Individually diagnosed unknown source records | 596 | **476** |

Exactly nine maps newly prepare: **Juchengshi, Luoyang Tang, MP15, MP34, MP39, MP43, SA-Linzi, SAO-Chengdu, SAO-Luoyang**. These are complete policy results; they are not nine human gameplay or whole-scene rendering passes. No partial policy is published for any refused map.

New first unsupported sources are **MP48 Industrial186/ID40/logical1,114,088**, **MPWall1 Industrial151/ID97/logical1,119,368**, and **SA-Qufu Industrial173/ID10/logical1,096,644**. They remain unsupported. MP42 still first refuses Industrial173/ID13/logical1,113,677. Its type164 IDs58/61 at storage(140,173)/(188,125) now separately fail existing actual-diamond bounds (raw0x80005); their classification does not free the manager. Chang-an Sui’s outside-map162/unknown188 and MP27’s outside-diamond165 are unchanged. No clipping, coordinate repair, unknown-class skip or extra type is added.

All **121 prior policies × two versions × actual/zero/one masks =726** complete datasets and fingerprints are identical. All **167 actual renderer-derived legacy masks and presentation resource counts**, and **167 ×20 legacy rule/profile full World/save contracts =3,340** are identical, including MP34’s20 legacy cases. Candidate policies perform780 schema19/20 roundtrips; MP34 has six complete policy/mask cases. Eight legacy profiles have pre-existing derived-cache canonical differences after restore; both original and restored canonical streams are separately equal across baseline/candidate, while full authoritative Snapshots/context/SaveDocuments roundtrip exactly. Comparison reports contain zero contract differences.

Before/after all **1,464 files /819,711,091 bytes** and the original manifest match. All211 non-test source files except the single central `SandboxPlacement.cpp` are unchanged. Baseline/candidate audit times are251.56/262.48 seconds, peak RSS247.35/262.19MB on this machine, with no frame or tick advancement; these are local timings, not portable performance limits. Research rechecks120 source records ×eight provenance fields =960 exact matches.

### Normal MP34 Application, paid transport and persistence

**SCRIPTED TECHNICAL PASS:** unchanged production Application/main uses ordinary Menu/UI events through SDL under dummy/software with a private read-only observer. The normal path is **New Sandbox → City-v16 rule4 → exact MP34 → Map rules checked → Empty City → Start**. It starts paused at actual tick5; no World, funds, workers, goods, couriers or camera are injected. Existing ordinary wheel framing is scripted separately from native input. Normal F1 selects real type179 origin(151,118) and164 origin(110,50), with matching raw fields/protection; selection and rejected purchases preserve the complete WorldSnapshot. Independent free neighbor(113,51) and terrain-blocked(110,112) keep their distinct permissions. All eight origins are queried as protected; ID34 is not claimed as an initial fitted-view click.

The pure actual-MP34 planner derives this regular paid scene:

| Purchase | Storage origin(s) | Funds |
| --- | --- | ---: |
| Two Houses | (117,111), (120,111) | 160 |
| Clay Source | (117,114) | 120 |
| Pottery | (123,114) | 180 |
| Six necessary mutable roads | y113, x118–123 | 12 |
| **Total** | 10 ordinary paid commands | **472** |

All purchased roads are used by the actual courier path; no existing original road is charged. Workforce is12 available/10 installed/assigned/active. At tick45, actual Clay courier1 has cargo1, target4 reservation1, path vertex1 and edge progress4. It remains outbound at70, reaches the target and enters Returning with cargo/reservation zero at71, reaches IdleAtWorkshop at106, then dispatches actual cargo2 at107. Full saved Pottery input_clay1 is observed at125 and the first completed Pottery at206. At605:18 Clay extracted, six Pottery completed, construction472, maintenance20 and Funds808. An earlier474-Fund recipe had one unnecessary road; its runs remain explicitly superseded by the three minimal472-Fund runs.

Normal **F5/F9/F5** checkpoints at5/45/605 have equal complete typed WorldSnapshots, map/legacy/policy context and full serialized SaveDocuments. Two new processes use ordinary Load Sandbox to load the in-flight45 save, start paused, and continue with identical ordinary ticks in batch versus single-step deliveries. Their complete605 documents are identical to the original normal run, and intermediate125 documents are identical to each other. Actual F9 wrong-map-relative-path, wrong-original-SHA and wrong-policy-fingerprint controls reject and preserve the complete active World/context; restoring the untouched good save then succeeds.

All twelve production render counter deltas stay zero; observers are outside measured rendering, and every scripted process exits with zero live textures and no native takeover. These are technical menu, transport, save and rendering-work checks. Human mouse/keyboard, visual fidelity, full-map gameplay, food/service supply and long-term profitability are unaccepted. The small test city has no full supply chain.

### Full configuration matrices

**PASS:** fresh inventories and complete runs execute all145 registered tests per configuration, **435/435 total**. Configurations run serially, with build parallelism2 and CTest parallelism2 for Debug/4 for Release and sanitizer. No registered test is skipped, timeout raised, compiler warning recorded or source changed during the matrix. These are fresh final runs after the explicitly retained first Release fixture failure.

| Configuration | Registered / executed / passed | Build seconds | Full CTest seconds |
| --- | --- | ---: | ---: |
| Debug | 145 / 145 / 145 | 15.02 | 1,468.68 |
| Release | 145 / 145 / 145 | 2.99 | 36.78 |
| ASan+UBSan (`RelWithDebInfo`) | 145 / 145 / 145 | 15.95 | 168.64 |

The retained build roots are `.local/market-walkers-finish/debug-build`, `.local/reswall/release-build` and `.local/market-walkers-finish/sanitizer-build`; no new full build tree is created. Sanitizer uses the existing `OPENEMPEROR_ENABLE_SANITIZERS=ON`, `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1` and `UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`. Individual reports/JUnit files and the aggregate source-stable PASS are retained under `.local/mp34-occupancy/`.

**PASS:** the excluded `openemperor-city-v16-playthrough-fixture` target is explicitly built and executed against the exact original data in each configuration, serially after all registered matrices. Each fresh private output contains all14 complete SaveDocuments, byte-identical to `.local/resgate/xia-fixture/saves`, and byte-identical complete `main-facts.jsonl`. This verifies the existing paid Xia goal/stability playthrough without representing the old fixture as a newly executed baseline. All three runs have zero warnings and the final complete source comparison is unchanged.

| Direct Xia configuration | Build seconds | Run seconds | Full save documents / main facts |
| --- | ---: | ---: | --- |
| Debug | 0.950 | 146.792 | 14 / 14 equal; full main facts equal |
| Release | 0.743 | 3.446 | 14 / 14 equal; full main facts equal |
| ASan+UBSan | 0.972 | 42.745 | 14 / 14 equal; full main facts equal |

Final evidence is `.local/mp34-occupancy/matrices-report.json`, the three configuration `*-tests-report.json`/JUnit files, and `xia-controls-report.json`. No pending matrix or excluded playthrough remains.

## Player test

The following command has actually been executed successfully:

```sh
sh tools/test_mp34_occupancy.sh .local/gog-extracted/app
```

The wrapper verifies contained exact MP34 and EXE revisions, rebuilds current Release via the existing common starter, and performs the full rule4/policy2 check before launching. A wrong map revision or escaping symlink stops before build/app start; both negative controls pass. The native process starts the normal menu in a fresh isolated root, `.local/resgate/player-OKnA1I`; this is **native launch only**, separate from the scripted normal-start evidence and human acceptance. Original files and personal settings/saves/recovery remain untouched.

Choose **New Sandbox → Start Sandbox**; **MP34 / Empty City / City-v16 rule4 /policy2 /schema20** are preselected. F1 diagnosis origins are179(151,118),164(110,50); both stay protected. The legal build/route positions above reproduce the technical scene through paid commands.

A separately named, regularly paid optional technical save is `.local/mp34-occupancy/MP34-paid-clay-in-flight.json` (schema20, tick45, construction472, cargo1/reservation1). It is byte-identical to the tested normal in-flight save and is available through the normal **Load Sandbox** file chooser. It is a minimal delivery test, not a completed economy.

Historical scope remains in [Industrial177/178](mp56-industrial177-178-occupancy.md), [ResWall](reswall-parser-occupancy.md), [ResGate](resgate-parser-occupancy.md), [saved-General presentation](../rendering/mp56-original-structure-fallbacks.md) and [map compatibility](../map-permissions-compatibility.md).
