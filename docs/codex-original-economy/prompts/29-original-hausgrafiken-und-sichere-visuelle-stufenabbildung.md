# OE-29 – Original-Hausgrafiken und sichere visuelle Stufenabbildung

**Phase:** E – UX  
**Abhängigkeiten:** OE-24, OE-25, OE-27, OE-28  
**Geplante Änderungspfade (prüfe echte Dateinamen):** `src/assets/BuildingVisualProfile.*; resources/compatibility/; src/app/SandboxView.cpp; docs/reverse/; tests/`  
**Vorgeschlagene Commit-Message:** `feat(original-ui): [29] original-hausgrafiken-und-sichere-visuelle-stufenabbildung`

## Auftrag an Codex

Arbeite am tatsächlichen HEAD von `oliverjessner/OpenEmperor`. Lies zuerst `docs/codex-original-economy/README.md`, `IMPLEMENTATION_ORDER.md`, `QUALITY_GATES.md`, `PROGRESS.md`, `DECISIONS.md`, `AGENTS.md` sowie alle für OE-29 relevanten vorhandenen Source-/Testdateien. Behalte die Code- und Test-Konventionen bei.

### Implementiere genau Folgendes

1. Suche und dokumentiere Originalsprites pro belegtem Haus-Typ in den lokal lizenzierten SG3-Archiven.
2. Lege eine bedingte, metadata-only Visual-Profile-Zuordnung an, die Originalassets aus lokaler Installation dekodiert. Typ/footprint, Seitenanzahl, Alpha, Pivot und Stage dürfen nicht geraten werden.
3. Arbeite bestehende Hausstufen ohne Bildidentität über ausdrücklich deklarierte Fallbacks ab; keine Sprite-Stretching-Lösung.
4. Verifiziere die tatsächliche Produktion-Painter-Komposition inklusive Walker-Füßen, Foreground, Klippen/Rampen, F4/F6 und 1x/2x/4x.
5. Image-Captures und Proof-of-identity verbleiben in `.local/`; keine proprietären PNGs ins Git.

### Fertige Ergebnisse / Abnahme

- [ ] Belegte Original-Hausstufen visuell korrekt zugeordnet, keine falschen Identitäten
- [ ] Konsistente Stage-Sprite/World-Type-Aktualisierung bei Upgrade/Downgrade

### Pflichtprüfungen

- [ ] Originalasset Hash+Metadata Tests, Pixel/Alpha/Picking
- [ ] Bezahlt 1x/2x/4x Scene Matrix und Snapshot/F5/F9

## Stop-Bedingung und Fehlerbehandlung

Keine originale Art identifiziert? Benannter Fallback ist korrekt; `original sprite parity` bleibt FAIL/UNKNOWN.

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
