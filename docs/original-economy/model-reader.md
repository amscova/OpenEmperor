# Original model reader (OE-01)

The isolated `openemperor-original-model` library reads the selected installation's
`Model/EmperorBuildingModels.txt` once during explicit preparation. The normal app,
World, renderer, MapPermissions and persistence do not link or call this module.
No original balance rows are embedded in the repository. This task establishes
**RAW MODEL FACT** and parser behavior; it does not establish original gameplay
parity, difficulty arithmetic, semantic value ranges or World building identity.
Those contracts follow in OE-02 and the pure rule tasks.

OE-02 adds the separate [named schema and facts register](model-provenance.md).
The raw reader retains source labels so named access can verify a row identity;
its generic syntax and signed raw-value contract remain unchanged.

## API and provenance

`src/simulation/original/ModelData.h` provides typed file-column enums,
`ModelBuildingId`, zero-based `HouseRowId`, and `Difficulty`. These IDs address
file records only; they grant no construction or simulation permission.
`ModelData` can only be created by successful parsing. Raw signed 32-bit values
and both modifier tables are exposed through const accessors. Missing buildings
and invalid field, row or difficulty arguments throw `std::out_of_range`; there
is no clamping, invented missing row, percentage default or additive default.

`parse_model_data(bytes)` and `load_model_data(game_root)` return `ModelResult`:
a complete model on success, or an explicit error code, line, section and reason.
No partial model escapes on failure. The model owns its data independently of
the input buffer. Its provenance contains the SHA-256 of the exact unmodified
input bytes, source kind/canonical file path, byte count, reader format version
and validated row counts. CRLF and LF parse to the same raw fields but have
different hashes. A hash identifies content; it does not authenticate its author
or certify semantic safety for a future simulation.

Build and inspect without starting the game:

```sh
cmake --build BUILD --target openemperor-model-inspect
BUILD/openemperor-model-inspect /path/to/legal/game/installation
```

The JSON report contains counts and provenance, never the balance tables. An
invalid or missing dependency prints a diagnostic and returns nonzero. Future
session integration must explicitly bind this result; OE-01 adds no fallback or
active rule version.

## Accepted file grammar

The examined local source is ASCII with CRLF. Its exact markers, after trimming
spaces/tabs, occur once and in this order:

1. `BUILDING MODS`: five difficulty rows, IDs 0–4 and matching labels.
2. `ALL BUILDINGS`: at least one unique building ID and 13 fields per row.
3. `HOUSE MODS`: five difficulty rows with matching labels and 24 fields.
4. `ALL HOUSES`: 15 ordered rows, distinct nonempty text labels, 24 fields.
5. `End of model data.`: optional original terminal dash separator, then blanks.

The bounded prose preamble is ignored before the first section; it contains
explanatory braces. Within sections, only rows, blanks and the two observed
building column-legend lines are permitted. Unknown/missing/repeated/reordered
sections and extra content after the end marker fail.

Building rows have `ID,label,{,values,}` and optional comma padding. House rows
have `label,{,values,,,,` through end of line: the original omits their closing
brace. A closing brace is also accepted for house rows. One leading empty CSV
framing token is optional; trailing empty padding tokens are accepted. Empty
fields between values and any extra numeric field fail rather than shifting
columns. Numbers use complete signed decimal int32 syntax; optional `+` is an
explicit reader allowance. Negative raw values are preserved, not guessed away.

House row identity is currently its ordered file position, not a uniform numeric
label prefix or an existing World house level. Unique labels catch duplicates;
this reader does not claim to detect every semantically reordered, relabeled or
modified model. Symbolic model identities and units remain OE-02 work.

## Resource and read boundaries

The reader accepts at most 256 KiB, 4096 bytes per logical line and building IDs
0–4096. These are implementation limits, not inferred original engine ranges.
Counts are exact for houses and both difficulty tables. The fixed filename must
resolve to a regular file within the selected canonical game root. Escaping
symlinks and nonregular dependencies are refused before payload reading. The
binary read is bounded, checks completeness and unexpected growth, hashes the
same bytes it parses, and never writes to the source. This is a preparation-only
reader, not a file watcher or a frame/tick operation. It is not a security promise
against concurrent hostile replacement of the filesystem hierarchy.

## Fork comparison and source evidence

Reference: `amscova/OpenEmperor`, `windows-port`, commit
`d861d2912337c09751cb68196f85d5a30752726e`.
`ModelData.h` blob `dc1b9922fec14dad47f1e1781de3c2bce98af651` and
`ModelData.cpp` blob `aa60cebe0f6ade12d4ed803061cd4b0b7309f53a` were inspected
from the verified OE-00 local snapshot. The new parser is independently
implemented against the observed grammar; no fork code was imported.

The fork skips inner empty tokens, overwrites duplicate building IDs, ignores
extra house numbers, accepts unclosed building rows, lacks file/line bounds and
hash provenance, and clamps invalid difficulty arguments. Its mutable public
tables and default arithmetic are deliberately absent here. Percentage/additive
transformations are deferred until their own tested rule contracts.

The local legal source has SHA-256
`7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6`.
This identifies the examined installation only. Modified or other legal sources
are not rejected merely because their digest differs; callers receive their
actual digest. Private reports and logs stay under `.local/original-economy/oe01/`.

## Validation

`original-model-data-synthetic` always runs on generated, explicitly synthetic
rows, including malformed and boundary tables, deterministic mutations, typed
access bounds, SHA byte identity and filesystem errors. `original-model-data-real`
uses `OPENEMPEROR_ORIGINAL_MODEL_TEST_ROOT` (default local GOG path) and returns
CTest SKIP code 77 only when the optional source is absent. An existing malformed
or unsafe source is a failure. Neither test substitutes model numbers.

Actual commands, configurations, counts and regression evidence are recorded in
[PROGRESS.md](../codex-original-economy/PROGRESS.md). Gate A remains open until
OE-07; a passing reader is not a playable original economy.
