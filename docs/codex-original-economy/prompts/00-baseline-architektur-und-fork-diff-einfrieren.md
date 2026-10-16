# OE-00 – Baseline, Architektur und Fork-Diff einfrieren

**Phase:** A – Evidenz  
**Abhängigkeiten:** keine  
**Geplante Änderungspfade (prüfe echte Dateinamen):** `AGENTS.md; README.md; KNOWN_ISSUES.md; src/simulation/World.{h,cpp}; src/persistence/SandboxSave.cpp; docs/architecture.md`  
**Vorgeschlagene Commit-Message:** `test(original): [00] baseline-architektur-und-fork-diff-einfrieren`

## Auftrag an Codex

Arbeite am tatsächlichen HEAD von `oliverjessner/OpenEmperor`. Lies zuerst `docs/codex-original-economy/README.md`, `IMPLEMENTATION_ORDER.md`, `QUALITY_GATES.md`, `PROGRESS.md`, `DECISIONS.md`, `AGENTS.md` sowie alle für OE-00 relevanten vorhandenen Source-/Testdateien. Behalte die Code- und Test-Konventionen bei.

### Implementiere genau Folgendes

1. Ermittle den tatsächlichen Git-HEAD, den Status aller lokalen Änderungen und die derzeit höchste Regel-/Schema-Version. Setze keine angegebenen SHAs blind voraus.
2. Dokumentiere die Unterschiede zum Fork `amscova/windows-port` ausschließlich bei Original-Modelldaten, Desirability, HouseEvolution, OriginalHousingView, Services und Waren.
3. Erstelle eine unveränderliche Test-Baseline für existierende City-v16-Rule6-Saves, tatsächliche World-Snapshots und vorhandene MapPermissions-Fingerprints. Verwende vorhandene Fixtures; lege Quellen und Hashes ab.
4. Erstelle eine kurze Architekturkarte vom Kaufkommando bis World::tick, Courier arrival, Household demand, Snapshot/SaveDocument, Menü und Painter. Benenne die Erweiterungspunkte.
5. Schreibe einen nachprüfbaren Gap-Report mit konkreter Belegstelle für jede Behauptung. Keine Änderungen an der Simulationslogik.

### Fertige Ergebnisse / Abnahme

- [ ] `docs/original-economy/baseline.md` mit Source-/Testreferenzen und Open-Issues
- [ ] Wiederholbare Baseline-Kommandos mit lokal ignorierten Artefakten
- [ ] Kein geändertes altes SaveDocument / keine neuen Weltbefehle

### Pflichtprüfungen

- [ ] Vorhandene deterministische City-v16- und Persistence-Tests ausführen
- [ ] Git-/Source-Digest vor und nach der Diagnose vergleichen

## Stop-Bedingung und Fehlerbehandlung

Wenn die Ausgangsrevision vom Paket abweicht: tatsächliche Dateien und Invarianten erfassen, im Fortschrittsprotokoll dokumentieren und nicht ungeprüft überschreiben.

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
