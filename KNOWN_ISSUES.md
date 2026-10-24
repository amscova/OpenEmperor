# Known issues

OpenEmperor is an alpha. Supported sandbox behavior does not establish
complete original-game compatibility.

- **Original economy is experimental.** Rules10/11 are opt-in and include authored
  immigration, workforce, recipes, harvesting and school-to-house visits.
  Original venue/performer chains and illness are missing. Houses and productive
  buildings cannot yet be demolished; incompatible development saves are refused.
- **Rule10 has no purchasable fire protection.** Natural fires temporarily remove
  occupied Houses from the live goal while `PlaceFireWatch` is refused. The
  [paid progression check](docs/original-economy/paid-progression.md) reaches the goal
  and supplies six Houses in a finite window; its fixed plan still loses Funds over time.
  Separate Rule11 offers the authored paid FireWatch/Inspector contract; this
  does not establish original Inspector's Tower124 identity or profitable economy.
  Its bounded arrival/tax provenance has a finite capacity; see
  [fire protection](docs/original-economy/oe36-fire-protection.md).
- **Original sprites are incomplete.** Supported House stages use verified art;
  several other buildings, plants and walkers retain fallbacks. Empty Elite plots
  have no matching original body until their larger footprint materializes.
  Building animation, profession mappings and some pivots remain unverified.
- **High-tier progression is unproven.** All 15 housing model rows are integrated,
  but a complete paid playthrough to the highest tier has not been demonstrated.
  The unchanged four-house Xia starter without expansion loses money and misses
  Food demands in the measured long run; it needs active development.
- **Service capacity is limited.** Larger cities can miss Service deadlines
  despite continuous Post activity. Current coverage does not guarantee coverage
  at the next demand; universal fairness and full-city balance remain unverified.
- **Original saves and campaigns are unsupported.** Original residents and NPC
  simulation are not imported. Two of the 167 audited maps still fail conservative
  occupancy validation; other unsupported source states are refused.
- **Road and transport coverage is incomplete.** Original bridges, ramps,
  wall-top roads, mixed water-road terrain and unknown road styles are not fully
  supported. Owned Bridges and Ramps require explicit rule profiles and have
  bounded geometry; safe removal, shipping and Ferry transport are unavailable.
- **Original terrain heights remain unresolved.** The playable source-cell ramp
  prototype uses original sprites, but their surface and anchors do not yet fit
  production RampLinks. Production ramp banks and cliff cuts are authored;
  full original cliff, height and natural-surface fidelity remain open.
- **Some restored structures use previews or fallbacks.** Great Wall context,
  wall/Tower/gate states, quarry depth and shore-landing lifecycle are incomplete.
  Earthen-wall and quarry previews do not reproduce the original restored state.
- **Rendering is approximate.** First-draw anchors, foreground ordering and
  interpenetrating sprites can differ from the original. SG3 mirror references
  are unsupported; sprite shadows are not bit-identical to original RGB555 output.
  Automatic visual profiles recognise only the pinned GOG-derived asset revision.
- **Manual acceptance remains open.** Native pointer/focus/resize behavior,
  external displays, original-map visuals and sustained human play still need
  acceptance. Automated checks do not establish these results.
- **macOS distribution is local development only.** The app is ad-hoc signed,
  without Developer ID signing or notarization. Independent clean-Mac and
  Finder/Gatekeeper acceptance remain unverified.

The [original-economy player guide](docs/original-economy/player-handoff.md)
explains the current starter. [Documentation](docs/README.md) links to the
supported rules and technical evidence.
