# QUALITY_GATES – verbindliche Abnahme

## Dauerhafte Invarianten (alle Phasen)

- Keine Änderung des Standardprofils. Rule7 bleibt bisheriger Entwicklerstand; der ausdrücklich beauftragte Quellenvertrag V1 nutzt Rule8/Schema24 nur mit eigenem Entwickler-Opt-in. Versionsverfügbarkeit vor Umsetzung erneut prüfen.
- Keine Mutation der 1–23 SaveDocument-Serialisierung. Besonders Rule7/Schema23, Rule6/Schema22 RampLink, Rule5/Schema21 BridgeSpan und unveränderte MapPermissions policy3 müssen identisch bleiben.
- Keine neue Berechtigung aus Sprite-/Modell-IDs. World, Route und Objektgeometrie bleiben authoritative; Spieleraktionen kosten reale Funds.
- Keine Gratisressourcen, Teleportlieferungen, Zeit-Vorsprünge oder reine Render-Events als Versorgungsbesuche.
- Reproduzierbare Saved-Persistence, separates neues Prozess-Restore, Determinismus nach Save/Load und negative Manipulationsprüfungen.
- Kein unerwartetes Verhalten für ältere `World::tick()`, Autosave/Recovery oder UI-Kürzel. Tests bei relevanten Änderungen.
- Originale Model-/Sprite-/Map-Dateien read-only; keine proprietären Bytes ins Git. Synthetische Testwerte sind erlaubt, aber nicht als originale Daten ausgeben.
- Evidence-Typen: `EXE-OBSERVED`, `RAW MODEL FACT`, `READ-ONLY INTEGRATION`, `AUTHORED FALLBACK`, `NOT VERIFIED`. Aussagen müssen dazu passen.

## GATE A – nach OE-07: Kernparität

- Modellparser mit negativen Fällen und Original-HASH-Bindung demonstriert.
- Vergleich zu den vier Fork-Modulen auf unabhängigen Testvektoren vorhanden.
- Der alte World/Save-Zustand bleibt identisch; die neuen Regeln laufen bisher nur pure/read-only.
- Fork-Bugs/ungeklärte Annahmen dokumentiert, keine gefälschten Originalversprechen.

## GATE B – nach OE-12: sichere versionierte Laufzeit

- Rule8/Schema24 ausschließlich Entwickler-Opt-in nach [freigegebenem Quellenvertrag V1](../original-economy/source-contract-v1-proposal.md); Rule7/Schema23 und alle älteren Schemas byte-identisch, keine Migration.
- Die Modelldatei ist bei Create und Restore dieselbe identifizierte Version; fehlende Quelle wird atomisch verweigert.
- Tick-/Originalzyklus-Interpretation ist dokumentiert und Save-übergreifend deterministisch.
- Desirability read-only aus wirklich bezahlten typisierten Well72-Quellen;20 Häuser, Add/Remove/Kartenrand/Reload und exaktes Finanz-/ID-/Entfernungsledger.
- Unveränderter City-v13-Bewertungsalgorithmus liest dieselbe bezahlte Szene mit ausdrücklich authored Well72→Well-Vergleichszuordnung. Keine illegale City-v13-World; echte alte City-v13-/16-Regressionsverträge bleiben unverändert.
- Cache nur bei Create/Restore/erfolgreichem Quellenbau oder -abriss; keine Rebuilds durch Haus-/Frameabfrage oder Tick. Quelle liefert kein Wasser, Upkeep, Worker, Goods, Service oder Einkommen.

## GATE C – nach OE-19: bezahlte Güter und Service

- Ceramics und Hemp haben reale bezahlte Sources, echte Market-Couriers, Reservations und Arrival-Events.
- Food Quality beruht auf echter Lieferung, nicht einem angenommenen Standardwert.
- Consumption erhält Conservation und Replay; bestehende Services geben originale Kategorien nur nach wirklichem Besuch.
- Vorher ist kein Rule7-Spielprofil für Endspieler sichtbar.

## GATE D – nach OE-27: spielbarer Housing-Kern

- Erstes wirklich gezahltes Hut->Plain-Cottage-Upgrade in der produktiven World plus beweiskräftige Negativfälle.
- Alle unterstützten Religion/Service/Waren-Kategorien sind mit bezahlten Quellen und tatsächlichen Walker-Ankünften verbunden.
- Alle 15 Hausmodelle sauber typisiert, Elite-Footprints mit konfliktfreiem Verhalten, Difficulty/Tax aus Modelwerten; ungelöste Originalgrenzen offen ausgewiesen.
- Bei ursprünglichen Quellenlücken bleibt der Gesamtstatus PARTIAL, keine erdachte Paritätsbehauptung.

## GATE E – nach OE-30: produktiv bedienbar

- Rule7 ist im Menü wählbar, besitzt einen komplett bezahlten City-Starter und einen ehrlichen Inspector.
- Stagesprites sind identifiziert oder sichtbar als authored fallback markiert.
- Captures 1x/2x/4x, echter Courier/House-Upgrade, F5/F9, Keyboard und Mausfunktion technisch geprüft. Nutzer-Akzeptanz getrennt.

## GATE F – nach OE-33: finaler Abschluss

- Save-Malformation/Recovery-Tests, echte Prozess-Restarts, deterministische 20k-/100k-Ticks, vollständiger nachweisbarer Legacy-Save-Vergleich.
- 167 Map Corpus und Policy-Zusammenfassungen unabhängig geprüft, soweit lokale Originaldaten vorhanden.
- Alle relevanten CTests in Debug/Release/ASan+UBSan ausgeführt und Resultate protokolliert.
- Messbare Featurematrix zum Fork; ohne vollständig bestandene End-to-End-Kette **PARTIAL**.

## Testausführung

Vor dem ersten CTest: Build-Konfiguration und vorhandene Targets mit `ctest --test-dir <build-dir> -N` ermitteln. Nicht davon ausgehen, dass neue Tests bereits existieren. Zieltypische Befehle:

```sh
cmake -S . -B build-original-debug -DCMAKE_BUILD_TYPE=Debug -DOPENEMPEROR_USE_SYSTEM_SDL3=ON
cmake --build build-original-debug --parallel 2
ctest --test-dir build-original-debug -N
ctest --test-dir build-original-debug --output-on-failure -R '<reale Testnamen>'
# Für Phase-Gates zusätzlich vollständige Suite in Debug/Release/ASan+UBSan.
```

Exakte Originaldaten für local-only Tests: `.local/gog-extracted/app`, vor Ausführung verifizieren. `NOT RUN`/`SKIPPED` nie zu `PASS` umetikettieren.
