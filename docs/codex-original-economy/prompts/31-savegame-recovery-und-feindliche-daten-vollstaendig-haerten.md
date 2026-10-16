# OE-31 – Savegame, Recovery und feindliche Daten vollständig härten

**Phase:** F – QA  
**Abhängigkeiten:** OE-09, OE-26, OE-29, OE-30  
**Geplante Änderungspfade (prüfe echte Dateinamen):** `src/persistence/SandboxSave.*; src/persistence/RecoveryStore.*; tests/`  
**Vorgeschlagene Commit-Message:** `test(original): [31] savegame-recovery-und-feindliche-daten-vollstaendig-haerten`

## Auftrag an Codex

Arbeite am tatsächlichen HEAD von `oliverjessner/OpenEmperor`. Lies zuerst `docs/codex-original-economy/README.md`, `IMPLEMENTATION_ORDER.md`, `QUALITY_GATES.md`, `PROGRESS.md`, `DECISIONS.md`, `AGENTS.md` sowie alle für OE-31 relevanten vorhandenen Source-/Testdateien. Behalte die Code- und Test-Konventionen bei.

### Implementiere genau Folgendes

1. Prüfe neues Rule7-Schema gegen komplette alte Schemas 1–22, ohne sie umzufärben. Schreibe fortlaufende Bytevergleiche für eine repräsentative eingefrorene Menge.
2. Testkatalog: fehlende/zusätzliche Felder, Bad IDs, manipulierte Counts, Goods, Services, negative Stocks, falscher Modellhash, Footprint/Protected-Overlap, Route/Cargo-Widersprüche.
3. Test-F5/F9 im Mittel eines Walkers, exakt vor/nach Konsum und Devolution, beim Besitzerabriss, bei Debt und Inventory reservation.
4. Verifiziere Recovery über echte Prozesse, atomare Dateiziele, Fehler-/Abriss-Szenarien und identische Weiterläufe.
5. Ein expliziter Upgrade-copy-Mechanismus ist optional; keine automatische Änderung von Rule6-/CityV13-Spielständen.

### Fertige Ergebnisse / Abnahme

- [ ] Strict Rule7 Save/Recovery mit vollständigem Negativkatalog
- [ ] Alte SaveDocuments byte-identisch; nie in-place Konvertierung

### Pflichtprüfungen

- [ ] Mehrere neuer Prozess-Replay-Paare und adversariale SaveCorpus
- [ ] Vorhandene Save-/Recovery-Suites ausführen

## Stop-Bedingung und Fehlerbehandlung

Unter keinen Umständen in alten Saves oder Nutzer-PrefPath-Beständen manuell schreiben.

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
