# OE-11 – Echte Modelldatei an World-Sessions binden

**Phase:** B – Laufzeit  
**Abhängigkeiten:** OE-01, OE-08, OE-09  
**Geplante Änderungspfade (prüfe echte Dateinamen):** `src/app/Application.*; src/app/MenuSession.*; src/simulation/World.*; src/persistence/; tests/`  
**Vorgeschlagene Commit-Message:** `feat(original): [11] echte-modelldatei-an-world-sessions-binden`

## Auftrag an Codex

Arbeite am tatsächlichen HEAD von `oliverjessner/OpenEmperor`. Lies zuerst `docs/codex-original-economy/README.md`, `IMPLEMENTATION_ORDER.md`, `QUALITY_GATES.md`, `PROGRESS.md`, `DECISIONS.md`, `AGENTS.md` sowie alle für OE-11 relevanten vorhandenen Source-/Testdateien. Behalte die Code- und Test-Konventionen bei.

### Implementiere genau Folgendes

1. Bereite ein unveränderliches OriginalRulesModel einmal beim Laden aus dem lokal lizenzierten Spielarchiv vor, nicht in `tick()`.
2. Übergebe die validierten, typisierten Originaldaten an neue Rule-7-Worlds, ihren Konstruktor und den Save-Restore-Pfad; keine globale mutable Singleton.
3. Binde Rule-7-Saves an den SHA-256/Fingerprint der Modelldatei. Fehlende, geänderte oder ungültige Dateien müssen eindeutig und atomisch abgewiesen werden.
4. Erlaube Legacy-Saves weiterhin ohne zusätzliche Original-Modellprüfung, soweit deren bisherige Profile das erlauben.
5. Prüfe mehrfaches Laden, unterschiedliche Dateien, fehlgeschlagene Vorbereitung und getrennte Sessions.

### Fertige Ergebnisse / Abnahme

- [ ] Rule-7-World nutzt genau ein geprüftes OriginalRulesModel
- [ ] F5/F9 reproduziert Datenbindung; kein Modell-Parsing pro Frame/Tick

### Pflichtprüfungen

- [ ] Gegenprobe identische und manipulierte Model-Hashes
- [ ] Vorhandene Menüs/Session- und Save-Tests

## Stop-Bedingung und Fehlerbehandlung

Unterstelle nicht, `World` hätte bereits Zugriff auf `data_root`: baue eine explizite geprüfte Abhängigkeit.

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
