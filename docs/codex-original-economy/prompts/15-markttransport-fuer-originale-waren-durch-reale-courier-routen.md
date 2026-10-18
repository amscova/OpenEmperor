# OE-15 – Markttransport für originale Waren durch reale Courier-Routen

**Phase:** C – Waren  
**Abhängigkeiten:** OE-13, OE-14  
**Geplante Änderungspfade (prüfe echte Dateinamen):** `src/simulation/World.cpp; src/simulation/World.h; src/persistence/; tests/`  
**Vorgeschlagene Commit-Message:** `feat(original): [15] markttransport-fuer-originale-waren-durch-reale-courier-routen`

## Auftrag an Codex

Arbeite am tatsächlichen HEAD von `oliverjessner/OpenEmperor`. Lies zuerst `docs/codex-original-economy/README.md`, `IMPLEMENTATION_ORDER.md`, `QUALITY_GATES.md`, `PROGRESS.md`, `DECISIONS.md`, `AGENTS.md` sowie alle für OE-15 relevanten vorhandenen Source-/Testdateien. Behalte die Code- und Test-Konventionen bei.

### Implementiere genau Folgendes

1. Erweitere den bestehenden World-Navigator statt einen zweiten BFS/Routenplaner zu schreiben.
2. Baue Rule7-typisierte Market-Annahme, Zielwahl, Reservierung, Teilbeladung, Bewegung, tatsächliche Ankunft, Rückfahrt und Neuplanung für Ceramics.
3. Stelle Zuständigkeit, stabile IDs und Limitierungen der Couriers dar. Reale Marktlieferung muss am Zielbestand landen und die Quelle reduzieren.
4. Regle Umwege, abgeschaltete Betriebe, Wegunterbrechung, volle Häuser, sicheren Abriss und Save mid-route.
5. Baue einen unabhängigen Negativtest, in dem kein Fahrweg zu keiner Lieferung führt.

### Fertige Ergebnisse / Abnahme

- [x] Real transportierter Bestand vom Markt zum Haus, nicht durch Setzen eines Zustands
- [x] World-Route/Ownership/Reservation konsistent bei F5/F9

### Pflichtprüfungen

- [x] Paid Market->House Ceramics Dispatch/Arrival/Return
- [x] Disconnect/Repair, full capacity, mid-courier recovery und Conservation

## Stop-Bedingung und Fehlerbehandlung

Keine fiktiven Lieferungen über Desirability-/Nähe-Checks oder Hausstatus-Overlays.

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
