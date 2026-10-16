# OE-07 – Fork-Parität als Vergleich, nicht als unkritische Kopie

**Phase:** A – Evidenz  
**Abhängigkeiten:** OE-03, OE-04, OE-05, OE-06  
**Geplante Änderungspfade (prüfe echte Dateinamen):** `tests/; tools/; docs/original-economy/`  
**Vorgeschlagene Commit-Message:** `test(original): [07] fork-paritaet-als-vergleich-nicht-als-unkritische-kopie`

## Auftrag an Codex

Arbeite am tatsächlichen HEAD von `oliverjessner/OpenEmperor`. Lies zuerst `docs/codex-original-economy/README.md`, `IMPLEMENTATION_ORDER.md`, `QUALITY_GATES.md`, `PROGRESS.md`, `DECISIONS.md`, `AGENTS.md` sowie alle für OE-07 relevanten vorhandenen Source-/Testdateien. Behalte die Code- und Test-Konventionen bei.

### Implementiere genau Folgendes

1. Erzeuge unabhängig formulierbare Testvektoren aus den klar nachvollziehbaren Fork-Funktionen; verlinke Originaldatei und Commitstand.
2. Vergleiche Parserwerte, Desirability-Ringe, Bedürfnisse, HouseEvolution, Timer, Konsum, Marktlieferungen und Food-Quality.
3. Erstelle je untersuchter Abweichung eine Entscheidung: `parity`, `corrected bug`, `unverified`, `unsupported` mit Beleg.
4. Untersuche speziell das funktionale Mapping und dessen künstliche Annahmen (z. B. `max(population,1)` in OriginalHousingView) und dokumentiere, warum wir das nicht als Gameplay-Fakt übernehmen.
5. Führe alle neuen Kern-Tests und die alten City-v13/v16-Tests aus; importiere keine Windows-Sachen.

### Fertige Ergebnisse / Abnahme

- [ ] Maschinenlesbarer Paritätsbericht und menschlich lesbare Entscheidungen
- [ ] Erster Meilenstein: mindestens dieselben isolierten Fähigkeiten wie der Fork, aber mit besseren Grenzen

### Pflichtprüfungen

- [ ] Unabhängige synthetische Kontrollfälle statt nur identische Implementierungstests
- [ ] Optional echter lokaler Datenvergleich mit eindeutigem SKIP

## Stop-Bedingung und Fehlerbehandlung

Ein grüner Paritätstest beweist keine vollständige Emperor-Originaltreue.

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
