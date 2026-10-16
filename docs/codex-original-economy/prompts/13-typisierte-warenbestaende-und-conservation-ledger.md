# OE-13 – Typisierte Warenbestände und Conservation-Ledger

**Phase:** C – Waren  
**Abhängigkeiten:** OE-06, OE-09, OE-11  
**Geplante Änderungspfade (prüfe echte Dateinamen):** `src/simulation/World.{h,cpp}; src/simulation/original/; src/persistence/; tests/`  
**Vorgeschlagene Commit-Message:** `feat(original): [13] typisierte-warenbestaende-und-conservation-ledger`

## Auftrag an Codex

Arbeite am tatsächlichen HEAD von `oliverjessner/OpenEmperor`. Lies zuerst `docs/codex-original-economy/README.md`, `IMPLEMENTATION_ORDER.md`, `QUALITY_GATES.md`, `PROGRESS.md`, `DECISIONS.md`, `AGENTS.md` sowie alle für OE-13 relevanten vorhandenen Source-/Testdateien. Behalte die Code- und Test-Konventionen bei.

### Implementiere genau Folgendes

1. Erweitere nur Rule7 um getypte Lagerbestände, Transporte, Reservierungen, Verbrauchs- und Produktionssummen für die originalen Güter.
2. Bewahre numerische Identitäten alter `Good`-/`CourierRole`-/`Object`-Enums und vorhandene SaveKeys. Falls neue IDs nötig sind, ausschließlich anhängen.
3. Führe pro Gut eine Conservation-Identität (produced + imported – consumed – demolished = stock + cargo, mit exakt dokumentierten Ein-/Ausgängen).
4. Transaktionen müssen atomisch bleiben: kein negativer Vorrat, keine doppelte Reservation, kein Overflow, kein kostenloses Material.
5. Baue eine read-only Warenbilanz-API für Tests/Inspector; für noch nicht produzierte Güter nur 0 und `unsupported`, niemals Scheinbestände.

### Fertige Ergebnisse / Abnahme

- [ ] Getypte Rule-7-Warenbuchhaltung über Gebäude und Courier
- [ ] Exakt getestete Conservation-Invarianten

### Pflichtprüfungen

- [ ] Grenzwerte, vollen/fehlenden Stock, abgebrochene Lieferung, Reload
- [ ] Legacy numerische Enum- und Save-Identitäten

## Stop-Bedingung und Fehlerbehandlung

Eine neue Ware in Enum allein ist kein implementierter Warenkreislauf; keine automatisch erzeugten Vorräte.

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
