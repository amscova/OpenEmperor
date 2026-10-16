# OE-27 – Schwierigkeitsgrad, originale Haussteuern und Spielziele

**Phase:** D – Housing  
**Abhängigkeiten:** OE-02, OE-10, OE-24, OE-25, OE-26  
**Geplante Änderungspfade (prüfe echte Dateinamen):** `src/simulation/World.cpp; src/simulation/original/; src/app/; src/persistence/; tests/`  
**Vorgeschlagene Commit-Message:** `feat(original): [27] schwierigkeitsgrad-originale-haussteuern-und-spielziele`

## Auftrag an Codex

Arbeite am tatsächlichen HEAD von `oliverjessner/OpenEmperor`. Lies zuerst `docs/codex-original-economy/README.md`, `IMPLEMENTATION_ORDER.md`, `QUALITY_GATES.md`, `PROGRESS.md`, `DECISIONS.md`, `AGENTS.md` sowie alle für OE-27 relevanten vorhandenen Source-/Testdateien. Behalte die Code- und Test-Konventionen bei.

### Implementiere genau Folgendes

1. Verwende die fünf Schwierigkeitseinstellungen als Rule7-only typed Parameter mit belegten Model-Modifikatoren.
2. Leite pro Haus echte Steuer-/Populationseffekte aus Modelwerten ab, nicht aus den alten 25/40/60-Euro-artigen konstanten Milestones.
3. Prüfe Zufluss, Unterhalt, signierten Schuldenstand, Workerzuteilung und die bestehenden strikten Treasury-Invarianten.
4. Definiere eigene Rule7-Mission-/Wohnziele mit transparenten Bedingungen; keine automatische Bonuszahlung oder hidden win latch.
5. Persistiere Schwierigkeit und daraus resultierende versionierte Authority; alte Savegamedateien und Defaults bleiben exakt.

### Fertige Ergebnisse / Abnahme

- [ ] Originaldatengetriebene Steuern/Bedarfe je Schwierigkeit und spielbare Rule7-Ziele
- [ ] Keine Veränderung von Legacy-Tax-Sätzen

### Pflichtprüfungen

- [ ] 5 Difficulty-Fälle, bezahltes Steuerbeispiel, overflow/negative debt
- [ ] F5/F9 mitten im Steuerzyklus, deterministic 20k

## Stop-Bedingung und Fehlerbehandlung

Originalmodell kann andere Zeit-/Rundungsregeln haben: markieren, nicht durch erfundene exakte Originaltreue ersetzen.

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
