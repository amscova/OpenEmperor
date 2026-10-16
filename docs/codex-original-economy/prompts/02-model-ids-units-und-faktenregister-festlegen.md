# OE-02 – Model-IDs, Units und Faktenregister festlegen

**Phase:** A – Evidenz  
**Abhängigkeiten:** OE-01  
**Geplante Änderungspfade (prüfe echte Dateinamen):** `src/simulation/original/ModelData.*; docs/reverse/; tests/`  
**Vorgeschlagene Commit-Message:** `test(original): [02] model-ids-units-und-faktenregister-festlegen`

## Auftrag an Codex

Arbeite am tatsächlichen HEAD von `oliverjessner/OpenEmperor`. Lies zuerst `docs/codex-original-economy/README.md`, `IMPLEMENTATION_ORDER.md`, `QUALITY_GATES.md`, `PROGRESS.md`, `DECISIONS.md`, `AGENTS.md` sowie alle für OE-02 relevanten vorhandenen Source-/Testdateien. Behalte die Code- und Test-Konventionen bei.

### Implementiere genau Folgendes

1. Dokumentiere pro relevantem Model-Feld Semantik, Einheit, Index, Difficulty-Transformation und belegte Originalquelle.
2. Lege getypte Konstanten/Enums für 13 Building-Felder, 24 House-Felder und die tatsächlichen 15 Haushaltszeilen an; prüfe semantische Felder für Lacquerware und Bronzwaren ausdrücklich.
3. Erstelle eine versionierte Tabelle: `verified original`, `reverse-engineered`, `inferred`, `authored fallback`, `unknown`.
4. Prüfe die Funktion->Modeltyp-Zuordnung des Forks (`OriginalHousingView`) gegen tatsächliche OpenEmperor-Objekte. Trenne funktionales Mapping von identitätsbelegter Zuordnung.
5. Baue unabhängige Golden-Tests für mindestens fünf dokumentierte Werte aus der legalen Originaldatei, aber ohne proprietäre Massendaten einzuchecken.

### Fertige Ergebnisse / Abnahme

- [ ] `docs/original-economy/model-provenance.md` und getestete typed IDs
- [ ] Keine ungeprüfte Behauptung vollständiger Originaltreue

### Pflichtprüfungen

- [ ] Field-/ID-Mapping-Test mit falscher Difficulty und ungültigen Typen
- [ ] Read-only Datenabgleich mit lokalem Originalbestand

## Stop-Bedingung und Fehlerbehandlung

Wenn ein Feld in den 24 House-Spalten nicht existiert: weder Index erfinden noch zwei unterschiedliche Ressourcen heimlich doppelt konsumieren.

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
