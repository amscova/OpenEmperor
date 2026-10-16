# OE-23 – Kulturelle und medizinische Originaldienste vollständig anschließen

**Phase:** D – Housing  
**Abhängigkeiten:** OE-19, OE-21  
**Geplante Änderungspfade (prüfe echte Dateinamen):** `src/simulation/World.{h,cpp}; src/simulation/original/; tests/`  
**Vorgeschlagene Commit-Message:** `feat(original): [23] kulturelle-und-medizinische-originaldienste-vollstaendig-anschliessen`

## Auftrag an Codex

Arbeite am tatsächlichen HEAD von `oliverjessner/OpenEmperor`. Lies zuerst `docs/codex-original-economy/README.md`, `IMPLEMENTATION_ORDER.md`, `QUALITY_GATES.md`, `PROGRESS.md`, `DECISIONS.md`, `AGENTS.md` sowie alle für OE-23 relevanten vorhandenen Source-/Testdateien. Behalte die Code- und Test-Konventionen bei.

### Implementiere genau Folgendes

1. Unterteile Herbalist, Acupuncture, Music, Acrobat und Drama in eindeutig getypte, separate Dienstquellen und echte Visits.
2. Baue vorhandene ServicePost/HealthPost nur dort auf Originalkategorien ab, wo die Identität plausibel und dokumentiert ist; verbleibende Anbieter als neue Rule7-only-paid Objects.
3. Nutze gemeinsamen Courier-Graph, Dispatch, echte Ankunft, Timers, Mitarbeiter, Pausen, Feuer und Fehlversuche.
4. Führe eine `service availability vs delivered access`-Diagnose ein. Nähe und verfügbare Kapazität sind keine Ankunft.
5. Pro Kategorie mindestens ein positiver und ein negativer end-to-end Fall, ohne die älteren Services zu verändern.

### Fertige Ergebnisse / Abnahme

- [ ] Alle fünf nichtreligiösen Original-Service-Timer können durch reale, begrenzte Visits aktiviert werden
- [ ] Kein Service wird per Render-/Frame-Ereignis gewährt

### Pflichtprüfungen

- [ ] Pro Dienst neue Tests: paid visit, no-road, expiry, save restart
- [ ] Longplay bei Konkurrenz um dieselben Haushalte

## Stop-Bedingung und Fehlerbehandlung

Teilweise Original-Identität muss in der UI und Report als experimentell ausgezeichnet werden.

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
