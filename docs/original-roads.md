# Original roads as Sandbox transport

City-v16 **rule 4 / MapPermissions policy 2 / save schema 20** makes supported original ground roads usable before the first dispatch. New City-v16 games use rule 4. Existing cities keep their saved rules without migration, refunds or counter corrections; the general New Sandbox default remains City-v11 rule 3.

The implementation starts from the verified clean HEAD `b01e8140d67b5ac3baf36157d4acf7aa672f119d`. Targeted version, classification, transport and persistence regressions were written before implementation and failed against the baseline. Original inputs and personal saves are read only; generated data and captures stay in ignored `.local/original-roads/`.

## Bounded classification

The complete original manager, source schemas, exact claims, protected marker origins and typed GateHouse footprints must validate before roads are classified. The only imported normal-ground state is **exact raw terrain `0xc0`, raw objects 0, on-map and outside original occupancy**. Signed height remains a separate authoritative fact. This is the observed `0x80` ground plus `0x40` road combination, independently present on Handan and Badaling.

| Raw/source state | Policy-2 result |
| --- | --- |
| Exact `0xc0`, objects 0, complete unoccupied on-map cell | Immutable original road |
| `0x80` grass | Build permission only; no transport until an owned road is built |
| Bare `0x40`, unknown mixed terrain/features | Unsupported original road form |
| Water plus road | Unsupported water/bridge candidate |
| Nonzero objects | Unsupported raw object features |
| Massive original claim or reserved marker | Protected original occupancy |
| Gate footprint | Existing gate-solid/corridor contract, never a road-bit override |
| Off-map | Outside the supported map |

Road classification never reads pixels, SG3 record identity or decode success. Unknown classes and claim conflicts still refuse the whole map atomically. No bridge, ramp, wall-top route, original NPC or new height transition is admitted.

Oliver's coordinates match **both Handan and Handan_S** in the available original data. Clay `(116,111)` and Pottery `(116,108)` have complete 2×2 `0x80`/objects-0/height-0 footprints; the shared adjacent roads `(116,110)` and `(117,110)` are `0xc0`/objects-0/height-0. No matching supplied save establishes which variant was played. The real Handan check below is a separate paid reconstruction, with different generated entity IDs; it is not a replay of Oliver's save. The coordinates select only a reviewable test recipe, never gameplay authority.

## Shared transport and protection

`MapPermissions::original_road` is separate from `road_allowed`, `building_allowed`, blocking original occupancy and fixed gate passages. Imported roads create no `Object::Road`, Building/Courier IDs, commands, construction spending or road counters. They are free traversable, nonbuildable and nonremovable.

`World::road_cell`, `transport_cell` and the existing permission-edge contract feed full-footprint entrances, routing, all courier roles, movement/return, begun-edge interruption, rerouting and restore validation. Traversal remains orthogonal with the existing deterministic ordering and ordinary per-edge duration. Normal edges require equal signed heights. Gate openings, internal corridor edges, forbidden side entry and forbidden direct building entry from a corridor retain their prior rules.

`PlaceRoad` returns **“Original road already available.”**, a free no-op under the existing single-command sequence contract. `RemoveRoad` rejects **“Original map road cannot be removed.”** A mixed drag counts and pays only new mutable roads, preserves blockers atomically and refreshes navigation at most once. Its coalesced preview remains pure. Budget/control worlds retain the complete shared immutable policy.

A save binds the original map, original legacy buildable mask, explicit policy version and canonical SHA-256. Policy 1 retains its former bytes and SHA framing. Policy 2 has an explicit canonical format/domain and includes all former permission, protected, height and gate facts plus fixed-road cells/classification. Schema 19 remains exclusively rule 3 / policy 1; schema 20 exclusively rule 4 / policy 2. Original roads are reconstructed from original inputs; JSON cannot add arbitrary roads or transit links.

## Presentation and inspection

Existing supported original singleton images draw their prepared Combined texture exactly once in the early ground pass, under walkers, in every debug layer including Snapshot. No second Sandbox road descriptor or replacement texture is created. The original image, anchor, signed-height projection, eager deduplication and upload policy are reused. Other buildings, walls and gate houses retain their spatial foreground path. Owned connection masks include original roads and legal gate openings through the same height-filtered edges.

F1 identifies **Transport source: Original map road**, its imported policy, traversability, classification and signed height. Policy-1 roads report **“Original road is visual-only under this saved policy.”** Unsupported special forms report their concrete classification. F1/G remains an explicit read-only, bounded, revisions-bound diagnostic; it does not run BFS on cached Inspector frames. F2/F8, zoom and viewport changes cannot alter navigation or the immutable policy.

## Test the current local build

From the repository root:

```sh
sh tools/test_original_roads.sh .local/gog-extracted/app Handan
```

The script builds current Release code, resolves one exact contained Cities filename, creates a fresh ignored app root and prepares a normally paid rule-4/policy-2/schema-20 test city. It starts the ordinary menu application with isolated settings, saves and recovery. Choose **Load Sandbox → A-original-ready**, then **Space**. The paused save is at tick 31, just before the first Clay dispatch. Two Houses, Clay Source and Pottery were paid normally; no original road was bought. Select Clay, then F1/G to inspect the actual route. F2 compares walker markers. F5/F9 save/load only this private test stand.

The compact production check demonstrates a Clay trip, delivery, Pottery production and return. It is not a complete supply or profitability proof. Native human input, complete Emperor navigation/composition parity and unsupported special-road cases remain separate open checks.

## Acceptance evidence, 2026-10-10

The independent normally paid real-map helper observes the following trips. Each primary scene has two Houses, Clay Source and Pottery, **four paid commands / 460 construction spending / 840 Funds**, 12 available and 10 assigned workers, **zero PlaceRoad commands, zero owned roads and zero road construction counters**. The ready save is tick 31. First dispatch carries **one actual Clay unit and one incoming reservation**; arrival clears cargo/reservation and deposits it in Pottery. No courier or stock is injected.

| Actual map / separate scene | Imported road cells | Dispatch | On original road / inside gate | Arrival | Home |
| --- | ---: | ---: | ---: | ---: | ---: |
| Handan, reported building origins | 194 | 32 | 36 | 41 | 51 |
| Badaling | 227 | 32 | 36 | 41 | 51 |
| Kaifeng, ordinary original-road section | 288 | 32 | 36 | 41 | 51 |
| Zhengzhou, ordinary original-road section | 119 | 32 | 36 | 41 | 51 |
| Kaifeng, through Gate 1 | same policy | 32 | 46 | 66 | 101 |
| Zhengzhou, through Gate 1 | same policy | 32 | 41 | 61 | 91 |

Handan's authoritative route is `(116,111) → (116,110) → (116,109)`: one shared original road cell is sufficient; no minimum-length workaround was added. The complete footprint yields four source and two target entrances. Houses in this reconstruction are `(122,111)` and `(125,111)`; source/target IDs are 3/4, rather than Oliver's 1/6. Badaling uses Clay `(140,97)` and Pottery `(140,94)` with the supported original road `(140,96)`.

At tick 400 each primary mini has extracted 12 Clay and completed 5 Pottery, with Funds 820 after 20 normal upkeep. The separate gate scenes complete 3/4 Pottery respectively. Actual Kaifeng policy has five gates, nine of ten directly traversable original-road openings and one truly missing external road at `(118,167)`; Zhengzhou has two gates and all four openings traversable. All side-entry counterchecks reject `gate_side_entry`. Six independent processes continue the four original-road snapshots and two inside-gate snapshots to **byte-identical complete schema-20 documents at tick 400**.

Versioned policy corpus: **102 prepared / 65 rejected**, retaining all prior first failure reasons (57 unknown occupancy, 3 known conflicts, 5 unsupported ResWall managers). All **3,366 policy-1 output files** remain byte-identical: 3,060 component files and 306 metadata files across real/zero/one legacy masks. Policy 2 imports **10,370** roads, independent of those mask extremes. Separate nonimported classifications count 892 nonzero-object cells, 149 original-occupancy cells, 2,730 unsupported terrain forms, 93 water/bridge candidates and 54 gate-footprint markings. These are classified cells, not missing assets. All 10,370 imported cells have existing rendered singleton Type-30 footprints and exactly 10,370 prepared early-ground items across the 102 maps; this census is load-time presentation evidence, not full scene/native acceptance. The 1,464 original files / 819,711,091 bytes retain their complete path/size/SHA-256 inventory.

Authored production pixel tests independently check opaque walker feet/torso above original ground, normal foreground occlusion, a continuous mixed original/owned connection, all eight F8 layers, F2, integer/fractional zoom, actual viewport resize and visible alpha picking. A paid earned MarketFoodInbound trip proves another actual role; the remaining role/cache checks are authored deterministic simulation evidence. Original road components submit **one Combined draw and zero separate Base/Overlay/spatial draws**. Both same-policy and explicit policy-1↔2 F9 loads rebind presentation without migrating a save. Pure rendering, cached F1/G inspection and cached road-preview frames keep all 12 observed counters at zero. A coalesced pointer update creates one pure RoadPlan and increments only that counter. Original-manager parsing stays exclusively in preparation; there is no dedicated parse counter.

Seven isolated normal `Application` runs used scripted SDL events through the actual **Menu → Load Sandbox** path under dummy/software: Handan and Badaling from ready tick 31 and on-road tick 36, Kaifeng/Zhengzhou from actual inside-gate ticks 46/41, and a historical rule-3/policy-1/schema-19 Handan control. Full independent helper/app saves match byte-for-byte before, during and after trips, including actual F5/F9/F5. Handan/Badaling first complete a Pottery recipe at tick 138. The historical control remains `SOURCE_NO_ENTRANCE`/“No road connection” and preserves its complete save. F2, all F8 modes, ordinary wheel zoom and owned SDL resize preserve World/map state. Across **1,972 rendered frames all 12 production counters are zero**. Explicit G queries remain separate: main scenes use six BFS calls and 18/19 visited cells per query; gate scenes use four calls and 44/36 visits, and the old-policy control uses zero. All owned processes exit normally with zero live textures; user processes are untouched. These are scripted application checks, not native or human acceptance. The report is `app/acceptance-summary.json`.

Necessary gameplay route work is measured independently by an ignored helper linked to the same unchanged production libraries. The four accepted paid startup commands increment Execute/RouteRefresh four times. Primary setup BFS calls/visited cells are Handan 6/18, Badaling 6/19, Kaifeng 2/2 and Zhengzhou 2/2; the separate through-gate setups use 4/44 and 4/36. Each of the six actual 400-tick trips increments SimulationTicks 400 and performs **zero further BFS, visited cells or RouteRefreshes**, using the existing prepared courier caches. Those tick scopes also have zero World copies/restores/commands/RoadPlans, asset work or file reads/writes. Phase, cargo and reservation transitions are recorded separately. All 26 generated complete saves, including six at tick 400, remain byte-identical to the independent reference. This counter evidence is in `gameplay-performance/summary.json`; comparison replays, explicit diagnostic queries and save observers are excluded from gameplay scopes.

Ignored reproducible evidence includes `policy/policy1-comparison.json`, `policy/policy2-summary.json`, `review/road-plan-corpus.json`, `world-final/` and `world-final/fresh-process-comparison.json`. Current launcher smoke ran the exact documented command on Handan and Badaling, created distinct fresh roots, completed all preparation, started the ordinary Application under dummy/software and stopped only its owned processes. Eleven invalid names and selected-file/Cities-directory escapes reject before build or app launch (`starter-negative-report.json`). All three starter shell scripts pass `sh -n`. This smoke establishes launch availability, not native human playability.

## Full test suites

All three builds register and execute the same 138 tests, including the five new classification, transport, save/recovery, original-road UI and ground-pixel regressions. The historical routing, RoadBatch, gates, budget, save/recovery, preflight, all City profiles, input, zoom, textures and scene-composition suites remain included.

| Frozen build | Passed / registered | Skips | CTest wall time |
| --- | ---: | ---: | ---: |
| Debug | 138 / 138 | 0 | 945.01 s |
| Release | 138 / 138 | 0 | 94.86 s |
| ASan+UBSan, optimized RelWithDebInfo | 138 / 138 | 0 | 283.48 s |

The sanitizer build uses the existing project configuration with `-O2 -g -DNDEBUG`, ASan `detect_leaks=0:halt_on_error=1` and UBSan `halt_on_error=1:print_stacktrace=1`; it is not a Debug sanitizer build. Compiler warnings and sanitizer diagnostics are zero. Test reports and absolute-path JUnit outputs are retained as `debug-*`, `release-*` and `sanitizer-*` under the ignored evidence root.
