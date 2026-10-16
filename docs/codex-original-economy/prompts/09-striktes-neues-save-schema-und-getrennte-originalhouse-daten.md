# OE-09 – Striktes neues Save-Schema und getrennte OriginalHouse-Daten

**Phase:** B – Versionierung  
**Abhängigkeiten:** OE-08  
**Geplante Änderungspfade (prüfe echte Dateinamen):** `src/simulation/World.{h,cpp}; src/persistence/SandboxSave.{h,cpp}; tests/`  
**Vorgeschlagene Commit-Message:** `feat(original): [09] striktes-neues-save-schema-und-getrennte-originalhouse-daten`

## Auftrag an Codex

Arbeite am tatsächlichen HEAD von `oliverjessner/OpenEmperor`. Lies zuerst `docs/codex-original-economy/README.md`, `IMPLEMENTATION_ORDER.md`, `QUALITY_GATES.md`, `PROGRESS.md`, `DECISIONS.md`, `AGENTS.md` sowie alle für OE-09 relevanten vorhandenen Source-/Testdateien. Behalte die Code- und Test-Konventionen bei.

### Implementiere genau Folgendes

1. Lege für die neue Regelgeneration einen neuen, explizit gebundenen Save-Schema-Typ an (auf derzeitiger Basis Schema 23).
2. Füge separate OriginalHouse-Authority-Felder für Typ, Bedürfnisse, Timer, Bestände, Counter, Cyclephase und zugehörige IDs hinzu; trenne sie vom alten v0–2 House-Level.
3. Implementiere kanonisches Parsing/Serialisieren, Bounds, ID-Referenzen, gültige Enum-Werte, Quellhashes und atomisches Restore mit Ablehnung unbekannter Keys.
4. Serialisiere neue Felder ausschließlich bei Rule 7. Alte Schema-1..22-Writer und validierten JSON-Dokumente müssen byte-identisch bleiben.
5. Reserve ausreichend Typisierung, ohne noch nicht existierende Waren/Services als aktive, kostenlos verfügbare Authority anzulegen.

### Fertige Ergebnisse / Abnahme

- [ ] Neue Rule-7-only Persistenz und null implizite Migration
- [ ] Malformed/Old-New-Payload Negativtests ohne Save-Zerstörung

### Pflichtprüfungen

- [ ] F5/F9 und separate OS-process roundtrips für Rule 7
- [ ] Vergleich eingefrorener alter Save-Bytes vor/nach Implementierung

## Stop-Bedingung und Fehlerbehandlung

Nie Proprietary Model-Tabellen in Saves serialisieren; nur Source-Identität, Datenzustand und geprüfte IDs.

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
