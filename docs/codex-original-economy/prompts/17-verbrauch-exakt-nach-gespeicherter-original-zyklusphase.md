# OE-17 – Verbrauch exakt nach gespeicherter Original-Zyklusphase

**Phase:** C – Waren  
**Abhängigkeiten:** OE-06, OE-10, OE-13, OE-15, OE-16  
**Geplante Änderungspfade (prüfe echte Dateinamen):** `src/simulation/World.cpp; src/simulation/original/GoodsRules.*; src/persistence/; tests/`  
**Vorgeschlagene Commit-Message:** `feat(original): [17] verbrauch-exakt-nach-gespeicherter-original-zyklusphase`

## Auftrag an Codex

Arbeite am tatsächlichen HEAD von `oliverjessner/OpenEmperor`. Lies zuerst `docs/codex-original-economy/README.md`, `IMPLEMENTATION_ORDER.md`, `QUALITY_GATES.md`, `PROGRESS.md`, `DECISIONS.md`, `AGENTS.md` sowie alle für OE-17 relevanten vorhandenen Source-/Testdateien. Behalte die Code- und Test-Konventionen bei.

### Implementiere genau Folgendes

1. Aktiviere die modellbasierte Güterkonsumfunktion ausschließlich in Rule7 und nur am nachweisbar versorgten Haushalt.
2. Wende die dokumentierten Schritte 0 und 7 pro Monat mit dem versionierten Scheduler an; keine ältere 400-Tick-Demand-Struktur überschreiben.
3. Rechne tatsächliche verbrauchte Einheiten in Conservation und Haushaltsbestände ein, inklusive min0 und Over-/Underflow-Schutz.
4. Leere Häuser und große Haus-Footprints müssen deren belegten Regeln folgen; prüfe Lacquer/Bronze-Alternativen vor Aktivierung.
5. Beweise nach F5/F9 über eine Konsumgrenze exakt gleiche Folgezustände.

### Fertige Ergebnisse / Abnahme

- [x] Tatsächlich geliefertes Hemp/Ceramics wird am richtigen Zeitpunkt verbraucht
- [x] Keine negativen Stocks, duplizierte Buchung oder Phantom-Lieferung

### Pflichtprüfungen

- [x] Vor/auf/nach Step 0 und 7, min0, kein Einwohner, Unterversorgung
- [x] Save-to-next-cycle und 20k deterministischer Vergleich

## Stop-Bedingung und Fehlerbehandlung

Fehlende Originalware muss als fehlend erscheinen, nicht als `needs_met=true`.

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
