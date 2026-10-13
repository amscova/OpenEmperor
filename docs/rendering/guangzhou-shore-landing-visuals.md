# Guangzhou shore landing follow-up — 2026-10-13

Starting HEAD: `44da5e11e5ede972a430a3f1b22fc1c1e04e219c`, clean workspace. Oliver's screenshot after `tools/test_remaining_occupancy.sh --map "SAO-Guangzhou"` shows two brown land-side diamonds beside correctly drawn wooden piers. It establishes a user-observed presentation defect, not a failure of the newly prepared occupancy policy or proof of original Ferry simulation.

## Exact cause and original data

The prior [MP34 shore pass](mp34-shore-landing-visuals.md) admits saved General group **0x48c** body variants **0/2**. Guangzhou stores **1/3**, so its eight body cells remain `UnregisteredSlot` and draw the brown diagnostic. Neither original image is missing. The existing singleton pier path already draws the adjacent images.

**RAW MAP FACT:** `Cities/SAO-Guangzhou.map`, SHA-256 `c75603d5bf5d3b2b7f5f92e59b92c671d961f2277f600b4c2be59ccd2a15a8ae`, declared size140/border44. Its complete manager is schema1, 4,000 records /741,661 bytes at logical1,093,607, with two active Ferry210/status3/side2/subindex0 sources, Base4/parent1/NonHouse1/Ferry-own1. Industrial162 ID13 at(99,107) remains separately preserved by the [occupancy association](../reverse/original-occupancy-conflicts.md).

| Ferry ID | Storage origin | Saved body | Group variant / local | Physical General image | Geometry / payload |
| ---: | --- | --- | --- | --- | --- |
| 14 | (99,107) | `0x86a0` | 1 /1696 | 1897 | 158×87; base12,800 /total16,020 bytes |
| 15 | (106,108) | `0x86a2` | 3 /1698 | 1899 | 158×116; base12,800 /total18,214 bytes |

Both complete saved squares have literal parts `(0,1;72,9)`, one marker at origin+(0,1), matching saved IDs on allfour cells, Objects0 and signed height0. Both images are static unmirrored Type30/size2, with contained valid payloads. The bounded runtime group is `[1695,1703)`; physical=local+201 applies its existing dummy/system translation once. Decoding and private visual review confirm the stored roofed/platform artwork. No different sprite, mirror or inferred orientation is substituted.

Existing independent piers are General1901 at(101,107), saved `0x86a4`, and General1903 at(105,109), saved `0x86a6`. Their singleton footprints, origins and images remain unchanged. Original input inventory remains all1,464 files /819,711,091 bytes and manifest `dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`. All original pixels/reports are ignored under `.local/guangzhou-shore/`; no original input is written or original EXE executed.

## Minimal presentation change

The late historical resolver now admits the four independently evidenced body variants **0..3** of the same exact eight-record group. Variants4..7 remain separate singleton pier choices and cannot become land bodies even with forged side-two metadata. Every other group, geometry, mirror/animation/format, source/budget and complete-part check is unchanged.

The existing full-group validator still creates one complete `presentation_only` 2×2 footprint per site. Exact saved IDs select artwork; no map name, original source ID, type, terrain, color or screenshot coordinate selects a record. Existing side-two anchor, marker signed height×40, front key, eager Combined/Base/Overlay triple, cached alpha, F8/Snapshot behavior, culling and physical deduplication remain unchanged. No World/permissions, Ferry/ship/water route, road, input, save schema or gameplay code changes.

Each real Guangzhou body now submits one Base and one Overlay, zero diagnostics, through the unchanged production renderer. Both controlled map views show the full original land bodies adjoining their unchanged piers. Background assets355→357, uploads1065→1071, RGBA bytes21,936,144→22,321,032: exactly two images /six eager uploads /384,888 bytes. No frame upload or budget increase.

## Regression and compatibility evidence

An independent authored saved1/3 pixel test was added before the production fix. The original producer fails on the expected green body pixel while rendering the existing0/2 control correctly. Afterward both1/3 render their own literal asymmetric upper/base pixels, unchanged independent pier, correct alpha/marker and exact physical IDs. Duplicate sites share assets without merging instances. Earlier1/3 refusal expectations are explicitly superseded only for complete valid squares: malformed1/3 part groups still refuse, and new forged side-two4..7 negatives remain strict. All old bounds, group, geometry, source, budget and failed-decode tests remain.

- Full Release suite: **146/146**.
- Focused Debug and ASan/UBSan: **31/31 each**, covering shore/stored graphics, SG3, ordinary walls/gates/Tower, roads/occlusion, scene/texture composition, permissions, Menu/MapRules and save/recovery. Configurations ran serially; zero warnings, skipped tests or source changes. Full Debug/sanitizer suites were not rerun for this presentation-only follow-up.
- Fresh complete167-map baseline/candidate comparison: **160 prepared /two conflicts /five bounds /zero unknown-first**, unchanged. All **960** policy1/2 actual/zero/one canonical/component/fingerprint contracts, **3,340** legacy World/save cases, current policy roundtrips, masks and original source metadata retain matching full hashes/metadata. This fresh comparison does not claim a new literal-byte proof.
- Only MP48, NavalT-Suzhou and SAO-Guangzhou prepare additional historical images. All other presentation resource counts are unchanged, including MP34's previous0/2 bodies and MP56/Gate/Tower controls. This does not claim whole-corpus original rendering parity.

Unchanged normal `Application/main` also passes scripted **New Sandbox → City-v16 rule4/policy2 → exact Guangzhou → Map rules checked → Empty City → Start** in a fresh dummy/software root. All29 normal actions, four-cell protection checks, paid Clay/Pottery production and seven F5/F9 checkpoints pass. The seven complete saved documents are literally identical to the preceding occupancy pass's normal-App outputs; construction474, staffing10/10 and final Clay18/Pottery6 at605 remain exact. All measured production render counters are zero; total eager uploads1325, zero live textures on shutdown. Scripted input and direct software image checks are separate from native/human acceptance.

The actual in-flight save is also loaded through a new process's normal menu and checked through the five subsequent complete checkpoints. Final private reports are `regressions.json`, `compatibility.json`, the before/after map reports and `app/*/report.json`. Existing user app sessions, roots and saves are untouched; no native automation is performed.

## Retest

Close the running game, then rerun the same starter from the repository root:

```sh
sh tools/test_remaining_occupancy.sh \
  .local/gog-extracted/app --map "SAO-Guangzhou"
```

The starter rebuilds the current native executable. Choose **New Sandbox → Start Sandbox**. Both former brown patches at(99,107)/(106,108) should now show their land-side platform/building adjoining the existing wooden pier. Their original protected cells remain nonbuildable; the river gains no Ferry transport function. An existing running process keeps its previously loaded artwork until restarted. Human/native visual acceptance remains open. No commit, push, tag, release or publication.
