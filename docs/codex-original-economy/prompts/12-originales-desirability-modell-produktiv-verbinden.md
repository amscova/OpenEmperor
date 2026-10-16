# OE-12 – Originales Desirability-Modell produktiv verbinden

**Phase:** B – Laufzeit  
**Abhängigkeiten:** OE-03, OE-09, OE-11  
**Geplante Änderungspfade (prüfe echte Dateinamen):** `src/simulation/WorldDesirability.cpp; src/simulation/World.*; src/app/SandboxView.cpp; tests/`  
**Vorgeschlagene Commit-Message:** `feat(original): [12] originales-desirability-modell-produktiv-verbinden`

## Auftrag an Codex

Arbeite am tatsächlichen HEAD von `oliverjessner/OpenEmperor`. Lies zuerst `docs/codex-original-economy/README.md`, `IMPLEMENTATION_ORDER.md`, `QUALITY_GATES.md`, `PROGRESS.md`, `DECISIONS.md`, `AGENTS.md` sowie alle für OE-12 relevanten vorhandenen Source-/Testdateien. Behalte die Code- und Test-Konventionen bei.

### Implementiere genau Folgendes

1. Ersetze ausschließlich für Rule 7 die bisherige Festwert-/Manhattan-Wertung durch die originaldatengetriebene Map.
2. Leite Beeinflussungsquellen aus tatsächlich platzierten, bezahlten Gebäuden/Footprints und den verifizierten Model-IDs ab; keine bloße Namensheuristik.
3. Baue eine bounded Cache-Invalidierung nach Bau, Abriss, Wiederherstellung und datenabhängigen Änderungen, nicht per Haus-/Frame-Query.
4. Liefer Quelle und Wert pro Haus samt read-only Debug-Provenance.
5. Vergleiche dieselbe bezahlte Szene mit City-v13 und Rule7 ohne Eingriff in deren Legacy-Authority.

### Fertige Ergebnisse / Abnahme

- [ ] Rule-7-House-Desirability stammt aus realen Gebäuden und Modelldaten
- [ ] City-v13/16 weiterhin unverändert

### Pflichtprüfungen

- [ ] Gebäude hinzufügen/entfernen, Kartenrand, 20 Häuser, Reload
- [ ] Cache-Rebuild nur bei relevantem Änderungsereignis

## Stop-Bedingung und Fehlerbehandlung

Für uneindeutige Modeltyp-Identität keinen angeblich originalen Einfluss erfinden: markiere/überspringe explizit.

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
