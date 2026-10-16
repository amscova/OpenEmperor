# OE-05 – Service- und Religionstimer isoliert rekonstruieren

**Phase:** A – Evidenz  
**Abhängigkeiten:** OE-04  
**Geplante Änderungspfade (prüfe echte Dateinamen):** `src/simulation/original/ServiceRules.* (neu); tests/`  
**Vorgeschlagene Commit-Message:** `test(original): [05] service-und-religionstimer-isoliert-rekonstruieren`

## Auftrag an Codex

Arbeite am tatsächlichen HEAD von `oliverjessner/OpenEmperor`. Lies zuerst `docs/codex-original-economy/README.md`, `IMPLEMENTATION_ORDER.md`, `QUALITY_GATES.md`, `PROGRESS.md`, `DECISIONS.md`, `AGENTS.md` sowie alle für OE-05 relevanten vorhandenen Source-/Testdateien. Behalte die Code- und Test-Konventionen bei.

### Implementiere genau Folgendes

1. Implementiere reine Timerregeln für Herbalist, Acupuncture, Music, Acrobat, Drama sowie Ancestor/Confucian/Daoist/Buddhist.
2. Trenne nachweislich belegte Zeitzyklen (Fork-Vorlage 96/40) von einem noch nicht bewiesenen Realzeit-/Tick-Mapping.
3. Testfälle: tatsächlicher Visit grant, Countdown-Grenzen, Erneuerung, no-visit/no-coverage, Elite-Ausnahmen, Mehrfachbesuche.
4. Dienstgebäude-Spawn-Intervalle als eigene pure Funktionen mit Staffing-Grenzen; keine gratis Service-Zugriffe aus Gebäude-Nähe.
5. Behalte das bestehende City-v8-Service-System für alte Regelprofile vollständig unverändert.

### Fertige Ergebnisse / Abnahme

- [ ] Rollenbasierte pure Service/Religion-Timer-API
- [ ] Geprüfter Ablauf einer Service- und einer Religionsversorgung

### Pflichtprüfungen

- [ ] Timergrenzen 0/1/39/40/95/96, besucht/nicht besucht
- [ ] Staffing-Stufentabelle und Verfallsreihenfolge

## Stop-Bedingung und Fehlerbehandlung

Vermeide den Fehler, ein 96-Zyklen-Datum als 96 reguläre `World::tick` Schritte zu interpretieren.

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
