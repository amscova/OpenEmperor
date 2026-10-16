# OE-21 – Erste komplette spielbare originale Hausentwicklung

**Phase:** D – Housing  
**Abhängigkeiten:** OE-04, OE-12, OE-15, OE-17, OE-18, OE-19, OE-20  
**Geplante Änderungspfade (prüfe echte Dateinamen):** `src/simulation/World.cpp; src/simulation/original/HouseEvolution.*; src/persistence/; tests/`  
**Vorgeschlagene Commit-Message:** `feat(original): [21] erste-komplette-spielbare-originale-hausentwicklung`

## Auftrag an Codex

Arbeite am tatsächlichen HEAD von `oliverjessner/OpenEmperor`. Lies zuerst `docs/codex-original-economy/README.md`, `IMPLEMENTATION_ORDER.md`, `QUALITY_GATES.md`, `PROGRESS.md`, `DECISIONS.md`, `AGENTS.md` sowie alle für OE-21 relevanten vorhandenen Source-/Testdateien. Behalte die Code- und Test-Konventionen bei.

### Implementiere genau Folgendes

1. Verbinde HouseEvolution mit dem produktiven Rule7-House-State und World-Scheduler, statt es wie Fork-OriginalHousingView nur hypothetisch auszurechnen.
2. Beweise in einer normal bezahlten City über echte Road/Market/Goods/FoodQuality/Ancestor-Ankünfte mindestens einen Hut -> Plain Cottage-Übergang.
3. Zeige negative Gegenproben: ohne echte Religion, ohne Quality, fehlender Rohstoff, unzugänglicher Market und unzureichende Desirability darf keine unberechtigte Entwicklung erfolgen.
4. Bei verfehlten Bedingungen realen Devolutionscounter und Zielstufe fortschreiben; Werte aus Model und purem Kern.
5. Sichere die Entwicklung über F5/F9 und separaten Prozess samt späterem Verfall; keine künstliche Hausstufe durch Testsetup.

### Fertige Ergebnisse / Abnahme

- [ ] Meilenstein: ein echter, bezahlter und spielbarer Hausaufstieg im normalen OpenEmperor-World
- [ ] Hier übertrifft das Projekt erstmals eindeutig den nur lesenden Fork-Housing-View

### Pflichtprüfungen

- [ ] Paid First-Upgrade und vier Negativ-Szenarien als eigenständiger Test
- [ ] F5/F9, mid-courier und Devolution-Replay

## Stop-Bedingung und Fehlerbehandlung

Vor diesem Task kein öffentlich sichtbares Rule7-Spielprofil freischalten; hier kann die vertikale Spielbarkeit nachgewiesen werden.

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
