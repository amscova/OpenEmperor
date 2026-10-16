# OE-26 – Große Elite-Häuser: Footprints, Platzbedarf und Transaktionen

**Phase:** D – Housing  
**Abhängigkeiten:** OE-25  
**Geplante Änderungspfade (prüfe echte Dateinamen):** `src/simulation/World.cpp; src/maps/SandboxPlacement.*; src/persistence/; tests/`  
**Vorgeschlagene Commit-Message:** `feat(original): [26] grosse-elite-haeuser-footprints-platzbedarf-und-transaktionen`

## Auftrag an Codex

Arbeite am tatsächlichen HEAD von `oliverjessner/OpenEmperor`. Lies zuerst `docs/codex-original-economy/README.md`, `IMPLEMENTATION_ORDER.md`, `QUALITY_GATES.md`, `PROGRESS.md`, `DECISIONS.md`, `AGENTS.md` sowie alle für OE-26 relevanten vorhandenen Source-/Testdateien. Behalte die Code- und Test-Konventionen bei.

### Implementiere genau Folgendes

1. Ermittle die für höhere Originalhäuser belegten Footprint-Größen und die Grenzen des aktuellen 2×2-OpenEmperor-Gebäudemodells.
2. Implementiere Rule7-only eine sichere, atomische Up-/Downgrade-Transaktion bei Footprint-Veränderungen, inklusive frei/occupied/protected/gate/road/bridge/ramp Prüfungen.
3. Ankommende Goods, Worker, Bewohner, Service-Timer und laufende Routen dürfen nicht kopiert oder verloren gehen; falls Übergang baulich unmöglich: explizite Stagnation statt Clip.
4. Binde tatsächliche Wohnkapazität und Warenbedarf an den gespeicherten Haustyp; keine konstruierte Original-Ankerbehauptung.
5. Teste Demolition/Restart während Upgrade-Vorbereitung und bei vollen Nachbarschaften.

### Fertige Ergebnisse / Abnahme

- [ ] Elite-House-Geometrie ist mit World-Occupancy, Painter und Transport konsistent
- [ ] Vollständige atomische Platz-/Konflikt- und Restore-Tests

### Pflichtprüfungen

- [ ] Ränder und protected cells, neun Kollisionen, Wait/Repair, Save
- [ ] Alte City-v16-Wohnhaus-Footprints identisch

## Stop-Bedingung und Fehlerbehandlung

Kein unvalidiertes 1×1->2×2 Sprite nur optisch strecken; alte Saves unangetastet.

Liegt eine Abhängigkeit auf BLOCKED/FAIL oder kann ein verpflichtender Test nicht laufen, **stoppe diesen Prompt**, lege keinen „grünen“ Feature-Commit an und protokolliere den konkreten Blocker (inkl. reproduzierbarem Befehl). Vermeide Blind-Fixes, künstliche Authority und abgeschwächte Assertions.

## Gemeinsame technische Vertragsgrenzen

- Keine Windows-Workarounds, keine pauschalen Fork-Merges, keine fremden Änderungen ohne gezielten Vergleich.
- Legacy City-v16 Rule 4/5/6, darunter Bridge-/Ramp-Authority sowie alte Save-Schemas, unverändert; Rule7/Schema23 nur bei freier Versionsidentität und ausdrücklich opt-in.
- Originale `.sg3`, `.555`, Model-Dateien und EXE ausschließlich read-only aus der rechtmäßig vorhandenen Installation; keine Originalbilder oder umfangreichen Zahlenkopien ins Git.
- Kein zeitbasierter oder per-Frame-Zugriff auf Dateisystem/Decoder/World-Mutationen; kein zweiter Navigator.
- Teste notwendige Negativfälle. Grün bedeutet nur belegte technische Funktion, nicht pauschale Original-Parität.

## Commit / Report

1. Vor der Arbeit `git status --short` prüfen; vorhandene Fremdänderungen nicht überschreiben, stashen oder resetten.
2. Implementiere in kleinen zusammengehörigen Änderungen. Baue und führe die **für diesen Task relevanten** CTests aus, danach die im Phasen-Gate verlangten Suites.
3. Dokumentiere jeden tatsächlich ausgeführten Test samt Resultat und die Unterschiede gegenüber dem Fork in `PROGRESS.md`; notiere Annahmen in `DECISIONS.md`.
4. Wenn alle Abnahmekriterien erfüllt sind: nur Task-Dateien, Tests und Statusdokumente stagen und **einen** Commit mit obiger Message erzeugen. Der Nutzer hat lokale Commits für dieses Paket ausdrücklich autorisiert; historische `no commit`-Notizen in AGENTS.md gelten insoweit als überholt, alle übrigen Regeln bleiben bindend. Kein Push, kein Tag, kein Release.
5. Antworte mit Commit-SHA, Testprotokoll, belegtem Resultat und nächstem vorgesehenen Prompt. Bei FAIL/BLOCKED keine Freigabe des nächsten Prompts.
