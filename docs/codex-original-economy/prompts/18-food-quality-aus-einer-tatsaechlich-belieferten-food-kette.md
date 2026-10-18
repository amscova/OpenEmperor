# OE-18 – Food Quality aus einer tatsächlich belieferten Food-Kette

**Phase:** C – Waren  
**Abhängigkeiten:** OE-06, OE-15, OE-17  
**Geplante Änderungspfade (prüfe echte Dateinamen):** `src/simulation/World.cpp; src/simulation/original/; tests/`  
**Vorgeschlagene Commit-Message:** `feat(original): [18] food-quality-aus-einer-tatsaechlich-belieferten-food-kette`

Der Gesamtauftrag vom2026-10-17 führt diese Funktion ausdrücklich im
Rule10-/Schema26-Nachfolger mit Entwicklungsrevision18 aus. Maßgeblich sind
der vor Implementierung erstellte [Food-Vertrag](../../original-economy/food-runtime.md)
und die tatsächlichen Belege im [Fortschritt](../PROGRESS.md). Regeln1–9 und
Schemas1–25 bleiben eingefroren; authored Crop-Scores sind keine Behauptung
originaler Source-/Diversitätslogik.

## Auftrag an Codex

Arbeite am tatsächlichen HEAD von `oliverjessner/OpenEmperor`. Lies zuerst `docs/codex-original-economy/README.md`, `IMPLEMENTATION_ORDER.md`, `QUALITY_GATES.md`, `PROGRESS.md`, `DECISIONS.md`, `AGENTS.md` sowie alle für OE-18 relevanten vorhandenen Source-/Testdateien. Behalte die Code- und Test-Konventionen bei.

### Implementiere genau Folgendes

1. Binde Food-Quality an tatsächlich produzierte und transportierte Food-Lieferungen. Definiere einen typed `FoodBatchQuality`-/Marktwert, der zum Source-Cargo passt.
2. Implementiere die belegbare Originalmischung ausschließlich bei tatsächlicher Ankunft unter Berücksichtigung gelieferten Volumens.
3. Behalte die bestehenden City-v7/11- und City-v16-Qualitätsneutralen Food-Mechaniken und deren Saves unverändert.
4. Bewahre Qualitäts-Provenance im Rule7-Status/Schema; verweigere verschwiegene konstante Qualität 20 aus bloßem `food_stock>0`.
5. Teste leere Märkte, gemischte Lieferungen, Missed Demand und mehrere Qualitätsklassen mit realer Quelle.

### Fertige Ergebnisse / Abnahme

- [x] Physisch begründete Food Quality statt hypothetischer OriginalHousingView-Nullwerte
- [x] Hausentwicklung kann Food-Qualität real prüfen

### Pflichtprüfungen

- [x] Volumen-mit-Qualität Conservation, Rundung, Teilmengen und Reload
- [x] Originale Food-Qualitätsformel gegen unabhängige Testvektoren

## Stop-Bedingung und Fehlerbehandlung

Kein automatisches Bestehen der FoodQuality-Bedingung ohne echten Food-Courier.

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
