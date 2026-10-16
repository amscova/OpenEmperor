# OE-06 – Warenbedarf, Lieferungen und Food Quality pur rekonstruieren

**Phase:** A – Evidenz  
**Abhängigkeiten:** OE-02, OE-04  
**Geplante Änderungspfade (prüfe echte Dateinamen):** `src/simulation/original/GoodsRules.* (neu); tests/`  
**Vorgeschlagene Commit-Message:** `test(original): [06] warenbedarf-lieferungen-und-food-quality-pur-rekonstruieren`

## Auftrag an Codex

Arbeite am tatsächlichen HEAD von `oliverjessner/OpenEmperor`. Lies zuerst `docs/codex-original-economy/README.md`, `IMPLEMENTATION_ORDER.md`, `QUALITY_GATES.md`, `PROGRESS.md`, `DECISIONS.md`, `AGENTS.md` sowie alle für OE-06 relevanten vorhandenen Source-/Testdateien. Behalte die Code- und Test-Konventionen bei.

### Implementiere genau Folgendes

1. Definiere getypte Waren: Hemp, Ceramics, Tea, Silk, Bronzeware, Lacquerware und Food-Quality als eigenes Attribut.
2. Leite Bedarfe und Minimum/Maximum aus der Modelldatei ab; unterscheide 1×1-/große Häuser, leere Häuser und Elite.
3. Prüfe den Fork-Code `consume_goods`: Lacquerware wird dort über `HF_Bronzeware` konsumiert. Belege oder korrigiere die Semantik statt stillschweigend doppelt abzuziehen.
4. Implementiere pure Delivery- und Consumption-Operationen mit Quellbestand, Zielbestand, Teil-Lieferung und Überlaufschutz.
5. Fasse Food-Quality-Blending in selbstständige Tests. Kennzeichne angenommene Formeln ehrlich.

### Fertige Ergebnisse / Abnahme

- [ ] Pure Waren-/Food-Quality-Regeln mit sauberer Semantik
- [ ] Keine Waren aus dem Nichts und keine negativen Bestände

### Pflichtprüfungen

- [ ] Vorher/Nachher-Inventory-Summen bei Teillieferungen
- [ ] Bedarf 0, leeres Haus, großer Haushalt, volle Lager, max integer

## Stop-Bedingung und Fehlerbehandlung

Keinen neuen producer-free Resource-Stock in World aufnehmen; dies sind isolierte Rechenregeln.

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
