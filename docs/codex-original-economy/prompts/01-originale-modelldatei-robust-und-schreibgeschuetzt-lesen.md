# OE-01 – Originale Modelldatei robust und schreibgeschützt lesen

**Phase:** A – Evidenz  
**Abhängigkeiten:** OE-00  
**Geplante Änderungspfade (prüfe echte Dateinamen):** `src/assets/ oder src/simulation/original/ (neu); src/simulation/World.h nur nach Bedarf; tests/`  
**Vorgeschlagene Commit-Message:** `test(original): [01] originale-modelldatei-robust-und-schreibgeschuetzt-lesen`

## Auftrag an Codex

Arbeite am tatsächlichen HEAD von `oliverjessner/OpenEmperor`. Lies zuerst `docs/codex-original-economy/README.md`, `IMPLEMENTATION_ORDER.md`, `QUALITY_GATES.md`, `PROGRESS.md`, `DECISIONS.md`, `AGENTS.md` sowie alle für OE-01 relevanten vorhandenen Source-/Testdateien. Behalte die Code- und Test-Konventionen bei.

### Implementiere genau Folgendes

1. Untersuche `Model/EmperorBuildingModels.txt` aus dem lokalen legalen Spielbestand und den Fork-Parser `src/simulation/original/ModelData.*`.
2. Implementiere einen bounded, schreibgeschützten Reader für 13 Building- und 24 House-Felder, 15 House-Zeilen und 5 Difficulty-Modifier. Vermeide provisorische Werte aus dem Code.
3. Prüfe Leerfelder, CRLF, beschädigte Zahlen, negative Werte, falsche Sektionen, Duplikate, Bounds, übergroße Eingaben und fehlende Dateien mit eindeutigen Fehlern.
4. Liefere stabile, getypte Zugriffe sowie einen SHA-256/Provenance-Report des genutzten Modells. Kopiere keine Original-Modellzahlen ins Git.
5. Führe synthetische Tests immer aus und echte GOG-Daten-Tests nur wenn der geprüfte Pfad existiert; kennzeichne SKIP.

### Fertige Ergebnisse / Abnahme

- [ ] Getypte, isolierte ModelData-API mit explizitem Parsing-Fehlerpfad
- [ ] Deterministische Negativtests und Hash-Provenance
- [ ] Keine Produktiv-World-Verhaltensänderung

### Pflichtprüfungen

- [ ] Eigenes ModelData-CTest registrieren und gezielt ausführen
- [ ] Parser-Fuzz-/Grenzfälle zumindest tabellarisch testen

## Stop-Bedingung und Fehlerbehandlung

Kein Laufzeit-Default darf unbemerkt fehlende Originaldateien durch erfundene Originalwerte ersetzen.

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
