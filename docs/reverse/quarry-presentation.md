# Bounded static quarry presentation

This presentation correction starts at clean HEAD `a5c2043742d7302a82b5c2176110df0e4305767b`, following the [MPWall1_S diagnosis](../rendering/mpwall1-s-quarry-placeholders.md). It replaces saved numbered quarry artwork through independently interpreted selector behavior. It does not reconstruct original quarry gameplay, extraction, complete restoration or first-draw parity.

## Pinned evidence

The user-supplied EXE has SHA-256 `6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e`. Its image base is `0x400000`. Addresses below are virtual addresses; subtract the base for RVAs. Each instruction in the bounded local windows was compared byte-for-byte with the freshly hashed PE, with zero mismatches. The EXE was not executed. Raw binary windows, original assets and rendered captures remain ignored under `.local/quarry-selector/`.

| Evidence | Observation |
| --- | --- |
| `0x53ed6a` in the existing selector chain | Calls quarry `0x53f840` after Rock and before Pinnacle |
| `0x53f845` | Quarry requires raw terrain bit `0x20000` |
| `0x53f85d`, `0x53f862` | Reads eight-neighbor quarry presence through `0x4b8f70`, then inverts through `0x4bc100` |
| `0x53f86b` | Unsigned variation comes from the separate `0xf1e780` byte grid |
| `0x53f871`–`0x53f87e` | Starts with key `0x628`; forward matcher `0x4bbdc0` uses 32 rows at `0x84b548` |
| `0x4b05aa`–`0x4b05b1`, `0x4bc090` | Initializes semantic constraints from eight row conditions; raw cached masks in the file are initially zero |
| `0x4bbdc0`–`0x4bbf12` | First matching row; four orientation offsets, positive count, unsigned variation modulo count |
| `0x53f88b`–`0x53f88d` | Nonzero primary offset bypasses all original mode/depth branches |
| `0x53f9a7`–`0x53f9d1` | Total primary variant6..33 becomes group `0x603`, variant `variation & 7` |
| `0x53f893`–`0x53f9a5` | Zero-offset rows depend on original mode `0x88ec38` and restored shared byte grid `0xe92dd0` |
| `0x535643`–`0x535650` | Standalone setup fills the entire51,984-byte shared depth grid with255 after the saved-image clear |
| `0x53f91d`–`0x53f929` | In the non-mode-1 branch, depth255 bypasses secondary stage selection and retains `0x628` |

The neighborhood order is storage N,NE,E,SE,S,SW,W,NW, deltas −228,−227,+1,+229,+228,+227,−1,−229. A semantic row condition1 requires absent quarry after inversion; condition0 requires quarry; condition2 is don't-care. In the matcher, a nonnegative variation uses modulo; the mutable negative-variation cursor path is not used. A missing row does not establish a valid identity: OpenEmperor refuses it instead of inheriting the original globals from another cell.

The primary table occupies640 bytes at VA `0x84b548`, RVA `0x44b548`, PE offset `0x449b48`. OpenEmperor stores original C++ semantic constraints, not binary rows or disassembled code. Rows1..11 have zero offsets/count6, including four repeated priority rows. Rows12..31 have nonzero offsets whose total variants stay6..33 in every observed orientation. Row32 requires all eight quarry neighbors and has zero offsets/count6. Across all256 neighborhood masks,84 select the mode-independent boundary,113 select a zero-offset row and59 have no match.

The secondary table is48 rows at `0x84b7c8`, RVA `0x44b7c8`, PE offset `0x449dc8`. Mode1 uses shifted terrain neighborhoods and groups `0x626`/`0x622`; other modes with depth below255 use depth-neighbor thresholds100 and0. Those branches are observed but not implemented or claimed reproduced. Post-load reset alone does not prove complete subsequent restored depth or original mode. The archive stone-byte read at `0x52ea95` is separate from that reset, and its saved values are not adopted as restored authority.

## Selected presentation

The production selector preserves existing early exclusions and Water/vegetation/Rock priority. It reads only raw quarry neighbors and existing variation. The first matching primary row determines:

- Nonzero offset with total6..33: **EXE-OBSERVED verified static boundary selection**, group `0x603`, variant `variation & 7`.
- Zero offset: **OPENEMPEROR static depth-255 quarry preview**, group `0x628`, variant `variation % 6`. F1/JSON retain preview evidence and explicitly state that original mode/depth restoration is unresolved.
- No supported row: **UNRESOLVED**, preserving the named historical fallback.

The preview is the observed non-mode-1/depth255 selector branch used as a bounded presentation convention. It is not an inferred original mode, a reconstruction of extraction progress, or a claim that serialized depth survives restoration. It applies through the normal Regenerated landscape view; Snapshot retains historical saved identity. No source state, map name, coordinate, graphic ID, numeral, color or pixel pattern selects the correction. No original-mode setting or new authoritative state is introduced.

Both keys resolve through `ResourceGroupLookup` and `RuntimeArchiveLayout`. In the pinned Terrain archive, `0x628` has34 variants starting at physical972; only its first six static78×40 images are used by the preview. Numbered saved records982..997 are later variants in the same group. `0x603` begins at physical247; normalization selects only variants0..7. Physical indices are observations, not production constants or substitute sprites. The existing bounded group resolver, strict static unmirrored Type30/side1/78-wide/base3200 check, physical deduplication and eager decoder/uploader remain the only path to readiness. Missing groups, out-of-range variants, unsafe geometry, budgets or decode failure preserve historical fallback.

## Actual-map and regression evidence

On `Cities/MPWall1_S.map`, SHA-256 `2d0eeccb89ab22cfda9b0311aeb39ed4ad4846da1444005aac554d0175b19eae`, all108 raw quarry cells have a supported primary row:52 boundary selections and56 static previews. All36 numbered historical cells, including24 in the user's pictured5×10 rectangle, are full-neighborhood/row32 cells. Boundary normalization alone would leave those numbers unchanged; the bounded interior preview is necessary to close the reported defect. Fresh all-cell metadata agrees with an independent raw-table interpretation for every group/variant.

Direct software rendering through `StoredGraphicsRenderer` of the same centered view shows stone-strewn quarry ground without numerals or black frames. Raw fields, saved IDs and historical statuses remain exact; all non-quarry selections are unchanged. This is **direct-render technical evidence**, not native input or human PASS. Snapshot intentionally retains its numbered historical artwork, and unrelated unregistered slot5/unsupported Great Wall cells remain separate limitations.

The new synthetic regression first failed against the previous selector. It checks all256 masks × four orientation columns × all256 variation bytes against independently derived boundary/unmatched truth sets; verifies modulo6 versus bitmask7, unknown-context labeling and prior exclusions; and exercises exact resource bounds, physical deduplication, saved-identity independence, missing-group and mirrored-geometry fallback. It verifies preservation of raw arrays and historical footprint ownership. All146 Release tests pass, with the selector and production stored-graphics renderer tests additionally passing under ASan/UBSan. Existing production pixel, alpha-picking, eager upload and repeated-frame tests remain applicable to this shared singleton path.

All selection/preparation happens at load time. Render/F1/pick adds no files, decoding, uploads, map scans, World copies/commands, BFS or routes. Signed height×40, painter, texture compatibility, asset/texture budgets, occupancy, policies1/2, City rules, World, roads and save schemas remain unchanged. Original quarry stage rendering, mode1 and complete scene fidelity remain open. No commit/push/tag/release/publication.

The fresh167-map production census still prepares165 and refuses the same two known conflicts. All990 policy summaries (165 maps × two policies × actual/zero/maximal legacy masks), source-manager results and other non-presentation fields are identical to the preceding complete census. Original inventory remains1,464 files /819,711,091 bytes, manifest SHA-256 `dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`. This verifies preserved permissions and inputs, not corpus-wide visual or human acceptance.

Final direct captures at zoom1 and2 confirm the corrected Regenerated presentation. A separate Snapshot capture retains all36 numbered historical cells. Automatic and explicit Earthen produce byte-identical pixels in the corrected quarry viewport. The F1 record identifies the static depth-255 preview and group/variant separately from saved identity; JSON reports56 ready `quarry_preview_selector` cells and zero complete original composition verification. Four repeated render/inspection/pick frames leave decode/upload counters unchanged, and owned textures are released. Final Release and focused sanitizer rebuilds/tests complete with zero compiler warnings.
