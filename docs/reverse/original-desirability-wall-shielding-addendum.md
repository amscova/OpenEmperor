# Original desirability wall shielding: independent static observations

Status: **EXE-OBSERVED**, static analysis only. No gameplay, parity or World
claim. This note records facts that the existing
[desirability contract](original-desirability-contract.md) lists as unresolved
("mixed units, original sectors, neighbor cases and clipped-edge behavior").
It contains observations and addresses, not code. Pinned executable: GOG
2.0.0.2, SHA-256 prefix `6373328b`. Virtual addresses are those of the
unpacked image.

## Scope

The spread routine at VA `0x44ed90` receives a source tile (x, y), a footprint
side n, a ring range r_max and a signed value v, and writes the value to ring
tiles of the desirability map. The routine has two paths.

## Observed facts

1. **Shielding only applies to negative values.** For v >= 0 every ring tile is
   written. The shield test reads bit 4 of the per-tile terrain flag byte
   array at VA `0xf37da0` (index = y * 228 + x).
2. **Edge path bypasses shielding.** If the square of radius r_max around the
   source reaches the lower bounds (x - r_max < -1 or y - r_max < -1) or exceeds
   the two map-extent globals at VA `0x101d0c0` / `0x101d0c4`, the routine takes
   a separate per-tile path through VA `0x4bb390` and performs no sector logic.
   What `0x4bb390` tests is not established here.
3. **Direction uses mixed units.** The two direction operands passed to the
   angle routine (VA `0x44e770`) are the tile's whole-tile coordinates minus the
   source position multiplied by four (plus 2 * n when n > 1). Tile coordinates
   are therefore compared against a quarter-tile-scaled origin. Both the
   decompiled and the raw disassembly agree on this. The angle itself is
   atan2 evaluated with x87 `FPATAN`.
4. **Sector lookup.** `0x44e770` tests up to 16 sector records, each 0x18 bytes
   (two doubles and a flag byte), with the membership routine at `0x44e4d0`; the
   flag selects a wrap-around comparison. It returns the first matching index or
   -1 when both operands equal the constant at `0x7ac8c0` (the source centre).
   An index of -1 means the tile is not shielded and receives the value.
5. **Per-source sector table.** A table at VA `0xa655a0` holds, per sector, a
   marked byte and a blocking ring distance, 8 bytes per entry.
   - A negative-value ring tile with terrain bit 4 **set** marks its sector and
     keeps the smallest ring distance seen; the tile itself receives nothing.
   - A tile with the bit **clear** receives the value if its sector is
     unmarked, is -1, or the current ring is **not greater than** the sector's
     blocking distance; otherwise it is skipped.
6. **Gap fill.** When the ring has fewer than 16 tiles and the newly marked
   sector s is even: if sector (s + 2) mod 16 is marked with blocking distance
   exactly 1, sector (s + 1) mod 16 is marked at the current ring; independently,
   if sector (s + 14) mod 16 is marked with distance exactly 1, sector
   (s + 15) mod 16 is marked at the current ring. The tile-count source for
   n > 1 was not traced; the observation is for n = 1 (ring counts from the
   tables at `0x81f156` / `0x81f1aa`).
7. The table is cleared by the caller per source; that clearing was not
   re-verified in this pass.

## Consequences for existing OpenEmperor options

- `ExperimentalUniform16` (uniform 22.5-degree sectors, true centre) is an
  authored hypothesis. Facts 3, 5 and 6 differ from it: the original origin is
  scaled differently from the tile operand, shielding stops only farther than
  the blocking ring, and a gap-fill rule exists for sparse rings.
- Fact 2 means shielding never applies near the map edge, regardless of walls.

## Open

- The 16 sector boundary angles (initialiser of the records at the `0x44e770`
  `this` object) are **not** located. Without them an exact reproduction of
  fact 3 cannot be computed.
- Meaning of `0x4bb390` and of the two extent globals.
- Tile counts and clearing for n > 1.

Nothing here changes a rule, the model reader or any save.
