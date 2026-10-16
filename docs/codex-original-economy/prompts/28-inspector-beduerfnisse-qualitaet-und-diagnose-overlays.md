# OE-28 – Inspector, Bedürfnisse, Qualität und Diagnose-Overlays

**Phase:** E – UX  
**Abhängigkeiten:** OE-21, OE-24, OE-26, OE-27  
**Geplante Änderungspfade (prüfe echte Dateinamen):** `src/app/SandboxView.{h,cpp}; src/app/MenuSession.*; tests/`  
**Vorgeschlagene Commit-Message:** `feat(original-ui): [28] inspector-beduerfnisse-qualitaet-und-diagnose-overlays`

## Auftrag an Codex

Arbeite am tatsächlichen HEAD von `oliverjessner/OpenEmperor`. Lies zuerst `docs/codex-original-economy/README.md`, `IMPLEMENTATION_ORDER.md`, `QUALITY_GATES.md`, `PROGRESS.md`, `DECISIONS.md`, `AGENTS.md` sowie alle für OE-28 relevanten vorhandenen Source-/Testdateien. Behalte die Code- und Test-Konventionen bei.

### Implementiere genau Folgendes

1. Erweitere nur Rule7-House-Inspector um Haustyp, Prozentwerte/Bedarf, vorhandene Goods, Quality, Timer Restwerte, Entwicklungsblocker und erwartete nächste Prüfung.
2. Überprüfe UI gegen authoritative World-Queries statt eine zweite HouseSimulation im Renderpfad zu berechnen.
3. Zeige `produced`, `reserved`, `in transit`, `delivered`, `consumed` getrennt, damit Fehlversorgung erklärbar ist.
4. Implementiere begrenzte, lesbare Overlays für Versorgung/Desirability/Entwicklungsrisiko; optional F1-Diagnose, kein unbeschränkter 228²-Frame-Lauf.
5. Prüfe Maus, Tastatur, F5/F9, F2/F3/F6/F8, Zoom, Resize und Text-Overflow in echten SDL-Tests.

### Fertige Ergebnisse / Abnahme

- [ ] Spieler kann ehrlich sehen, warum ein Haus nicht aufsteigt
- [ ] Altes City16-Inspector/Layout unverändert

### Pflichtprüfungen

- [ ] Screenshots 1x/2x/4x, Mouse/Keyboard/Modal
- [ ] 100 pure UI frames: keine WorldCommands, BFS, Datei-/Asset-Arbeit

## Stop-Bedingung und Fehlerbehandlung

UI darf kein Häkchen für eine nur vorhandene, aber nie besuchende Dienststelle zeigen.

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
