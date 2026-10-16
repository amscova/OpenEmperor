# OE-32 – Longplay, Skalierung und Originalmap-Kompatibilität

**Phase:** F – QA  
**Abhängigkeiten:** OE-24, OE-25, OE-26, OE-29, OE-30, OE-31  
**Geplante Änderungspfade (prüfe echte Dateinamen):** `tests/; tools/; docs/original-economy/`  
**Vorgeschlagene Commit-Message:** `test(original): [32] longplay-skalierung-und-originalmap-kompatibilitaet`

## Auftrag an Codex

Arbeite am tatsächlichen HEAD von `oliverjessner/OpenEmperor`. Lies zuerst `docs/codex-original-economy/README.md`, `IMPLEMENTATION_ORDER.md`, `QUALITY_GATES.md`, `PROGRESS.md`, `DECISIONS.md`, `AGENTS.md` sowie alle für OE-32 relevanten vorhandenen Source-/Testdateien. Behalte die Code- und Test-Konventionen bei.

### Implementiere genau Folgendes

1. Leite konservative, messbare Ressourcenbudgets für aktuelle maximal 46 Gebäude / 26 Couriers ab und versioniere notwendige Rule7-spezifische Limits.
2. Führe 20k deterministischen Lauf, 100k Endurance, zwei unabhängige Districts und mindestens ein Modell mit intermittierendem Staffing/Fire/Illness aus.
3. Prüfe echte Bilanzinvarianten, UI-/Frame-Arbeitsverbote, Route-Caches, Preview und Memory-Capacities; miss Last und Performance relativ zum Legacy-Baseline-Run.
4. Wiederhole die vollständige 167-Map-Corpus-Prüfung und die 1.485 Policy-Summary-Kontrollen, soweit bestehende Runner lokal verfügbar sind.
5. Berichte nicht nur PASS-Zähler; zeige Kennzahlen für Stabilität, House-Upgrades, Goods-Transport, Rückentwicklung und tatsächliche Versorgungslücken.

### Fertige Ergebnisse / Abnahme

- [ ] Performance- und Gamebalance-Bericht mit unabhängigen echten Laufresultaten
- [ ] Volle Bestandserhaltung und keine Legacy-Kartenänderung

### Pflichtprüfungen

- [ ] 20k/100k und getrennte Process-Restarts
- [ ] Relevante CTests Debug/Release/ASan+UBSan und Corpus nach Phase

## Stop-Bedingung und Fehlerbehandlung

Wenn Pflicht-Originaldaten fehlen oder 100k nicht ausführbar sind, explizit BLOCKED statt PASS.

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
