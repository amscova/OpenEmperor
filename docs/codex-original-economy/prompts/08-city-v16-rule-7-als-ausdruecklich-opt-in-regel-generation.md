# OE-08 – City-v16 Rule 7 als ausdrücklich opt-in Regel-Generation

**Phase:** B – Versionierung  
**Abhängigkeiten:** OE-00, OE-07  
**Geplante Änderungspfade (prüfe echte Dateinamen):** `src/simulation/World.{h,cpp}; src/app/MenuSession.*; src/app/main.cpp; src/maps/; tests/`  
**Vorgeschlagene Commit-Message:** `feat(original): [08] city-v16-rule-7-als-ausdruecklich-opt-in-regel-generation`

## Auftrag an Codex

Arbeite am tatsächlichen HEAD von `oliverjessner/OpenEmperor`. Lies zuerst `docs/codex-original-economy/README.md`, `IMPLEMENTATION_ORDER.md`, `QUALITY_GATES.md`, `PROGRESS.md`, `DECISIONS.md`, `AGENTS.md` sowie alle für OE-08 relevanten vorhandenen Source-/Testdateien. Behalte die Code- und Test-Konventionen bei.

### Implementiere genau Folgendes

1. Entwerfe und implementiere eine **explizite** neue City-v16-Regelgeneration für die Originalwirtschaft. Auf derzeitiger Basis ist Rule 7 / MapPermissions policy 3 vorgesehen.
2. Verifiziere vorab das freie Versions-/Schema-Segment. Bei neuerem Upstream kollisionsfrei neue Nummer reservieren und im Entscheidungslog begründen.
3. Rule 7 soll die bestehenden Bridge- und Ramp-Berechtigungen von Rule 6 unverändert erben; alle exakten Guards prüfen und erweitern, ohne Rule 6 zu ändern.
4. Während unfertiger Schritte Rule 7 nur über sichere Entwickler-/Testoption zugänglich machen; normaler Spieler-Default bleibt City-v11 beziehungsweise unveränderter City-v16-Default.
5. Stelle sicher, dass alle alten Regelversionen dieselben Identitäten, Preise, Graphen und Save-Bytes behalten.

### Fertige Ergebnisse / Abnahme

- [ ] Expliziter Regeln-Gate `original_economy_enabled()` ohne unbeabsichtigte Legacy-Aktivierung
- [ ] Keine unvollständig funktionierende New-Sandbox-Option

### Pflichtprüfungen

- [ ] Rule 4/5/6 und Legacy-Profilmatrix exakt kontrollieren
- [ ] Start/Build-Refusal bei unvollständigen Daten

## Stop-Bedingung und Fehlerbehandlung

Keine Änderung an vorhandener MapPermissions policy 3 oder bestehenden Save-Dokumenten. Keine implizite Upgrade-Migration.

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
