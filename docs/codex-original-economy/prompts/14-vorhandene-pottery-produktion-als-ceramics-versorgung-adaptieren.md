# OE-14 – Vorhandene Pottery-Produktion als Ceramics-Versorgung adaptieren

**Phase:** C – Waren  
**Abhängigkeiten:** OE-13  
**Geplante Änderungspfade (prüfe echte Dateinamen):** `src/simulation/World.cpp; src/simulation/original/; tests/`  
**Vorgeschlagene Commit-Message:** `feat(original): [14] vorhandene-pottery-produktion-als-ceramics-versorgung-adaptieren`

## Auftrag an Codex

Arbeite am tatsächlichen HEAD von `oliverjessner/OpenEmperor`. Lies zuerst `docs/codex-original-economy/README.md`, `IMPLEMENTATION_ORDER.md`, `QUALITY_GATES.md`, `PROGRESS.md`, `DECISIONS.md`, `AGENTS.md` sowie alle für OE-14 relevanten vorhandenen Source-/Testdateien. Behalte die Code- und Test-Konventionen bei.

### Implementiere genau Folgendes

Aktueller Versions- und Befugnisvertrag nach Nutzerauftrag vom2026-10-17:
[aktive Rule10/Schema26/Revision14](../../original-economy/completion-contract.md).
Historische Rule7-Bezeichnungen unten bezeichnen diese neue aktive Generation;
alte1–9/1–25-Verträge bleiben exakt. Bezahlte Bewohner-/Worker-Zuwanderung und
einmaliger Übergabepunkt sind ausdrücklich benannte authored Spielregeln.

1. Trenne das alte Rule6-`Good::Pottery` von der neuen Rule7-Modellkategorie `Ceramics`; definiere die nachweisbare funktionale Zuordnung.
2. Nutze ausschließlich reale bezahlte Clay->Pottery-Rezepte, Staffing, Inventar und Warenbewegungen als Quelle.
3. Wandle vorhandene Lieferungen/Bestände im neuen Regelprofil an einem exakt definierten Produktions-/Übergabepunkt um, ohne doppelte Verbuchung.
4. Bewahre altes Warehouse-, Market-, Food- und Courier-Verhalten unverändert.
5. Dokumentiere offen, falls das Produktionsmodell noch authored und nicht Emperor-paritätisch ist.

### Fertige Ergebnisse / Abnahme

- [x] Vollständiger Ceramics-Ursprung aus realen bezahlten Quellen im ausdrücklich gewählten Rule10-Nachfolger
- [x] Keine Gratis-Ceramics und kein Pottery/Ceramics-Doppelkonto

### Pflichtprüfungen

- [x] Gezahltes Clay->Pottery->Ceramics-Szenario
- [x] Ausfall durch unstaffed, disconnected, stopped, Fire; conservation

Tatsächliche Belege: [OE-14-Testprotokoll](../PROGRESS.md#oe-14-umsetzung--2026-10-17-basis41091b66)
und [Runtimevertrag](../../original-economy/production-runtime.md).

## Stop-Bedingung und Fehlerbehandlung

Nicht nur Inventory initialisieren: Produktion, Reisecargo, Lager und Verbrauch müssen dieselbe Bilanz benutzen.

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
