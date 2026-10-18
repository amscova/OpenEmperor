# Original culture sources: schools, performers and individual House visits

This records the bounded source proof used by the [revision23 culture adapter](../original-economy/culture-runtime.md). It was checked against the read-only PE32 `Emperor.exe`, SHA256 `6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e`, and model SHA256 `7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6`. Addresses below are static VAs, not RVAs, file offsets or save offsets. No Original program was executed.

The private checker `.local/original-economy/oe23/research/check_culture_sources.py` freshly passed129 narrow byte/call/slot assertions and20 contiguous disassembly windows:1026 instructions/2959 bytes. It maps every instruction back to the pinned PE sections and checks unchanged SHA, size, mode and mtime for the EXE, models and registered SG3/555 inputs. Original bytes, full tables and disassembly remain private. These are static source findings, not World, visual, native or human acceptance results; current runtime results belong in [PROGRESS](../codex-original-economy/PROGRESS.md).

## Concrete paid source identities

| Model / exact label | Raw Normal Cost / Employees | Initial square | Actual class / vtable | Original Actor subtype |
|---|---:|---:|---|---|
|211 `Music  School` (two spaces)|50 /8|2×2|cMusicSchool /7acedc|34 FIG_MUSICIAN|
|212 `Acrobat School`|60 /9|3×3|cAcrobatSchool /7ad140|32 FIG_ACROBAT|
|213 `Drama School`|70 /10|3×3|cDramaSchool /7ad3a4|33 FIG_ACTOR|

**EXE-OBSERVED:** Industrial dispatch `51c759→48a7e0` recognizes211..213; `51c766→48a800` selects the concrete constructors. Call sites48a88a/48a86b/48a848 select48a8e0/48a900/48a920. Their publications48a8e8/48a908/48a928 install the separately RTTI-checked vtables above. Thus the classes are connected to the model types by real factory flow, not by their names alone.

The common initializer428abc reads `823598+24*type` into runtime side+07. All three actual `+168→413a00` predicates return false and `+170→416ad0` returns that side. This supports the stated initial square bodies. It is not a claim about every restored lifecycle state or a shape inferred from image dimensions.

All three `+1bc→416b60` delegate ordinary staffing to virtual+1b4→428ed0. Their material-acceptance `+158→416a50` returns false, receipt `+154→416ab0` returns0, and `+a0→4e1c20` is an empty update slot. Unlike Confucian219, these specific source callbacks show no incoming Wood/Paper stock prerequisite. This local statement does not prove absence of every external scenario or lifecycle gate.

## Spawn attempt is separate from creation and delivery

**EXE-OBSERVED:** source+20 callbacks48aff0/48b1a0/48b350 pass actual Actor34/32/33 to shared51cf90. Nonpositive effective staffing returns before attempting; the unsigned counter increments modulo256 and compares strictly greater than the selected threshold. A due attempt resets before the actual Actorfactory4ea050. Successful creation stores the source ID+b4 in ActorWORD+62.

Music and Acrobat use threshold callback5ab330 with signed staffing tiers100/75/50/25/1/otherwise and thresholds3/6/12/24/32/64. Drama uses48b380 and6/12/24/32/48/96. These are comparison thresholds in eligible callback operations, not established Worldtick periods or guaranteed visits. The pure [ServiceRules](../../src/simulation/original/ServiceRules.h) retains this separate attempt semantics.

## The actual performer destination is a physical venue

The school Actor table entries32/33/34 publish callback4d5d00. It resolves ActorWORD62 through Buildingmanager8c7634 and invokes the actual source+24. For schools this is **48a9a0**, a distinct state handler; it is not the ordinary51d0c0 visitor lifecycle.

The observed path checks the actual school road connection, enters movement, and consults separate culture destination manager48a340→c702c0. Selector48a520 accepts only Actor32..34, checks the candidate's status, Access, performer acceptance via target+264 and positive raw staffing via+1b4. It also reads the retained performer state via+25c. A selected real destination ID is written to ActorWORD68 and a route is traversed. No selected destination marks the Actor for removal; it does not grant a House visit or free performer stock.

At arrival48ab93 resolves that destination again;48b540 recognizes physical categories71 Entertainment Area and75 Theatre Pavilion. Target+260→48b710 receives the actual performer. It assigns **venue-own-info** duration32 separately: Acrobat32→+5d, Musician34→+5e, Drama33→+5f. These are not House96 timers. The base venue accepts32/34; Theatre additionally accepts33. Venue effective staffing48b780 is0 while all received performer states are0. A supplied venue can emit its own separate Actor36 through48cde0; its source+24 delegates the ordinary movement callback.

The original entertainment loop therefore contains real venue destinations, performer arrivals, venue durations and another actor. Revision23 does not reconstruct those authorities. It creates no free venue, trainee or32-duration performer credit. Full venue construction/layout, performer countdown cadence, global/special venue branches and the Original wandering destination policy remain open.

## Individual DirectService effects on Houses

**EXE-OBSERVED:** all three school source+2c slots bind48ad20. The callback checks target status1/3, House type2..17, nonzero Access and **strictly positive signed occupants**; it has no empty-elite exception. It reads actual Actor subtype+12 and assigns exactly one96 byte:

| Actual Actor | House own-info field | Assignment VA / bytes | Pure role |
|---|---|---|---|
|34 Musician|+2c|48ad76 `c6402c60`|Music2|
|32 Acrobat|+2b|48ada4 `c6402b60`|Acrobat3|
|33 Drama|+2e|48ad8d `c6402e60`|Drama4|

This callback is connected to actual actor-centered distribution while the real performer moves. It supports individual `DirectService` actions. It does not support replacing one arrival with the distinct shared-walker `AcrobatAndDrama` action, assigning all categories at build/dispatch, or assuming the school normally targets Houses as destinations.

The existing [timer contract](original-service-timer-contract.md) separately establishes eligible phase35 countdown and all nine categories. The unchanged pure service kernels handle one visit and one countdown; they do not purchase sources, navigate or choose a Worldtick cadence.

## Explicit revision23 integration boundary

**AUTHORED INTEGRATION:** the playable adapter buys the actual211/212/213 model bodies using the session's prepared prices/Employees and the evidenced sizes. Each owns a separate cargo-free Worldcourier using the existing navigator and deterministic House rotation. Only a genuine accepted House arrival applies the corresponding single DirectService96 action. Source operation, full workers, road access and Fire remain normal gates; a previously dispatched visitor can finish after source pause/fire. Targetfire prevents a visit. Dispatch, source proximity and return apply no additional effect.

The source-to-House destination policy and its clock integration are experimental OpenEmperor choices. They are not full Original venue/training or spawn parity. Medicine96, four religions40, water and historical Rule1–9/Schema1–25 remain distinct. The active bounded provider/trip/witness limits are18/18/180, with two purchased sources per category. Housing's historical slot order explicitly preserves0/1/5/6/7/8 and appends2/3/4.

The new on-demand availability observation grants nothing: it reports installed providers, presently operating/non-burning/full-worker sources, current eligible targets with already cached routes, actual outbound trips, the sole House timer and the actual last-arrival witness even after expiry. Reachable sources are a subset of currently available sources. An in-flight source may currently be unavailable. No availability count promises a future visit, persists as authority or triggers a route refresh, BFS, World snapshot, model preparation or file access.

Confucian219's original material gate and initial elite-only source condition remain the separately documented [religion-source boundary](original-religion-source-contract.md). This culture addition does not silently resolve or broaden them.

## Original graphic selectors retained for OE29

The raw General registrations are211→824968/key412/physical589,212→824980/key410/physical460,213→824998/key411/physical501. They are static Type30 records with the stated initial sides. Source+244 selectors48b000/48b1b0/48b360 return walking groups4c67/4c94/4c6b and argument1 alternatives4c6a/4c98/4c6f. The checked walking SprMain records have12 phases and stride8.

Source+200 additionally distinguishes sourceWORD2e zero/nonzero in its graphic configuration. Those variants must not be relabeled loaded/trainee art without further state proof. Group metadata is not decoded-image, actor-foot, animation, full scene or human acceptance.
