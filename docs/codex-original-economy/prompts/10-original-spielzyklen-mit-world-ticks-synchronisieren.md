# OE-10 – Original-Spielzyklen mit World-Ticks synchronisieren

**Phase:** B – Laufzeit  
**Abhängigkeiten:** OE-05, OE-08, OE-09  
**Geplante Änderungspfade (prüfe echte Dateinamen):** `src/simulation/World.*; src/simulation/original/; src/persistence/; tests/`  
**Vorgeschlagene Commit-Message:** `feat(original): [10] original-spielzyklen-mit-world-ticks-synchronisieren`

## Auftrag an Codex

Arbeite am tatsächlichen HEAD von `oliverjessner/OpenEmperor`. Lies zuerst `docs/codex-original-economy/README.md`, `IMPLEMENTATION_ORDER.md`, `QUALITY_GATES.md`, `PROGRESS.md`, `DECISIONS.md`, `AGENTS.md` sowie alle für OE-10 relevanten vorhandenen Source-/Testdateien. Behalte die Code- und Test-Konventionen bei.

### Implementiere genau Folgendes

1. Baue Rule-7-only Scheduler für 51-Phasen-Zyklus, 16 Schritte pro Monat und 12 Monate pro Jahr, soweit belegbar; trenne historische Begriffe von authored Zeitumrechnung.
2. Bewahre tick-genau die bisherige 20-ticks/s-Simulation und die alten 400-tick-Demand-Timer für alte Profile.
3. Sichere in Schema 23 den minimalen autoritativen Scheduler-Zustand, so dass ein Restore exakt denselben nächsten Konsum-, Service- und Evolutionsschritt produziert.
4. Definiere die festgelegte Tick-zu-Original-Zyklus-Abbildung ausdrücklich in einer Versionskonstante und der Dokumentation; rate nicht über Spielgeschwindigkeit.
5. Teste Pause/Step, maximale Tickwerte, Save kurz vor/nach Monatsgrenzen und reproduzierbaren Replay.

### Fertige Ergebnisse / Abnahme

- [ ] Deterministischer Rule-7-Scheduler mit erklärter Zeitbasis
- [ ] Keine Drift nach Speichern/Laden und keine Veränderung alter City-Zyklen

### Pflichtprüfungen

- [ ] Phase 0/7/15, Monats-/Jahresrollover, Save vor/auf/ nach Cycle
- [ ] 20k wiederholter Restore/Test-Replay in kleiner Fixture

## Stop-Bedingung und Fehlerbehandlung

Wenn die originale Phasenzuordnung nicht belegt ist, als versionierte OpenEmperor-Annahme kennzeichnen statt Originaltreue zu behaupten.

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
