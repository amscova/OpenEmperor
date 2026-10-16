# OE-00: begrenzter Fork-Vergleich

Stand 2026-10-16. Diagnose zu ModelData, Desirability, HouseEvolution,
OriginalHousingView sowie deren Services/Waren; kein Import und keine
Freigabe neuer Wirtschaftsregeln. Windows-Änderungen sind ausgeschlossen.

## Tatsächlich gebundene Revisionen

| Quelle | Revision |
| --- | --- |
| Lokaler Upstream `oliverjessner/OpenEmperor` | `fef111deff4cedd3d3fa48663556d41b2e4b31a6` |
| Fork `amscova/OpenEmperor`, `windows-port` | `d861d2912337c09751cb68196f85d5a30752726e` |
| Gemeinsame Basis | `c908fdc0b18db6ffa6b3eaba4352a38a7f2c87bf` |

Der Fork-Branch wurde live über die [GitHub-Branch-API][branch] abgefragt und
durch `git ls-remote` bestätigt. Der [gebundene Vergleich][compare] meldet
**11 ahead / 4 behind**, einen Upstream-Commit mehr als beim historischen
Paketstand `f7a5035e92a9`. Der API-Tree ist vollständig (`truncated=false`).
Die acht Moduldateien, `tests/OriginalRulesTests.cpp`, CMake und LICENSE liegen
ausschließlich in `.local/original-economy/oe00/fork/` (ignoriert). Für alle elf
Dateien stimmt der berechnete Git-Blob-SHA mit dem Fork-Tree überein;
`file-manifest.json` dokumentiert außerdem Größen und SHA-256.

`World.h`, `World.cpp`, `WorldDesirability.cpp`, `WorldHealth.cpp` und
`SandboxSave.cpp` besitzen im Fork und aktuellen Upstream jeweils identische
Git-Blobs. Die vier neuen `.cpp` werden nur zusätzlich in die Library gebaut
([CMake:264–267][cmake-sources]); ein separater Test ruft sie auf
([CMake:493–498][cmake-test]). Das belegt keinen Ersatz produktiver World- und
Save-Regeln.

## Funktionsmatrix mit Belegstellen

| Bereich | Fork | Aktueller Upstream / Grenze |
| --- | --- | --- |
| ModelData | [13 Building-/24 House-Felder, 15 House-Zeilen, fünf Difficulty-Zeilen][model-shape]; prozentuale Building-/additive House-Modifikatoren, letztere auf −99..100 begrenzt ([Zugriff][model-access]); liest die installierte Textdatei ([Loader][model-loader]). | Kein `src/simulation/original/` oder Modell-Lade-/Hash-Bindungsmodul; aktive Werte aus `Rules` und Profil ([World.h][world-rules]). Parser ist keine neue Session-Authority. |
| Desirability | Quadratische Ringe, höchstens zehn; Sättigung nach jedem Add auf ±100; negative Quellen optional durch 16-Sektor-Mauern abgeschirmt ([Code][spread]). Mauervertrag ausdrücklich `Intended`, exaktes Originalverhalten offen ([Header][spread-contract]). | Manhattan-Abstand tatsächlicher Footprints, eigene Gebäude-Wirkungen/Entfernungsstufen; Begrenzung erst nach Summieren ([WorldDesirability][world-des]). Austausch würde bestehende Levels verändern. |
| HouseEvolution | Lokaler `HouseState`, Typen 2..17; Vacant/Shelter teilen Row0, insgesamt 15 Modellzeilen. Needs, Devolve-Counter, Elite-Sonderfälle ([Typen][house-state], [Evolution][evolve]). | World-Level0..2 aus kumulativen erfüllten Demands, Wasser/Desirability als Grenze; Kapazitäten6/10/16 ([World.cpp][world-level], [Regeln][world-rules]). Fork-Zustand ist kein gespeicherter `BuildingState`. |
| Services/Religion | Fünf Service-/vier Religionstimer, Visits setzen96/40, separater Decay und Staffing-Spawn-Tabellen ([Timer][timers]). `step_house`: Decay→Evolution→optionaler Verbrauch ([Visit/Step][visit]). | Echte bestehende Service-/Health-Couriers setzen ihre Tick-Expiry erst bei Ankunft ([Arrival][world-arrival]). Keine Verbindung zu allen neuen Kategorien, keine religiösen Kaufobjekte, keine gespeicherte Original-Zyklusphase. |
| Waren/Food Quality | Sechs lokale Waren, Größenfaktor für Visit-Need, begrenzte Umbuchung Market→House, Verbrauch/Qualitätsfunktion ([Code][goods]). | `Good`=Goods/Clay/Pottery/Food; reale Bestände/Reservations ([World.h][world-stock]), Arrival-Umbuchung und400-Tick-Verbrauch ([Arrival][world-arrival], [Demand][world-demand]). Zusätzliche Originalwaren haben keine vollständigen bezahlten Produktions-/Lieferketten. |
| OriginalHousingView | `const World&`, eigene Karte und hypothetische House-Berichte ([Adapter][view]); sieben funktional angenäherte `Object`→Modell-Zuordnungen ([Vertrag][view-contract]). | **READ-ONLY INTEGRATION**, weder echte nächste Hausstufe noch World-Tick. Produktionswelt und Persistenz bleiben unverändert. |

## Konkrete Review-Punkte für OE-01 bis OE-07

Die folgenden Codebefunde sind keine neuen EXE-Beweise. Originalübereinstimmung
dieser Details bleibt **NOT VERIFIED**; OE-00 hat keine EXE-Analyse ergänzt.

1. **Parsergrenzen:** Doppelte Building-ID überschreibt vorherige Zeilen
   ([ModelData.cpp:109–118][model-duplicates]); mehr als 24 House-Zahlen werden
   akzeptiert, zusätzliche Zahlen ignoriert ([124–129][model-extra]). Der
   Loader hat kein Dateigrößenlimit und keine SHA-Bindung ([147–153][model-loader]).
   Ob zusätzliche Felder für eine Originalvariante absichtlich nötig sind,
   benötigt eine eigene Kompatibilitätsentscheidung.
2. **Hypothetische Fakten im Adapter:** Jedes Haus startet als Shelter;
   `max(population,1)`, Food-Quality-Option und erneuter Herbalist-Grant bei jedem
   simulierten Schritt trotz nur momentan aktivem Health-Schutz
   ([OriginalHousingView.cpp:53–68][view-assumptions]). Das ist kein produktiver
   Visit-/Bestandsnachweis.
3. **Bronze/Lacquer:** Needs akzeptiert das Maximum beider Bestände
   ([Needs][needs]); Verbrauch vermindert beide mit `HF_Bronzeware`
   ([HouseEvolution.cpp:39–51][goods]). Verhalten reproduziert; ob die doppelte
   Abbuchung originalgetreu ist, bleibt offen. Kein pauschales Bugurteil.
4. **Qualität ohne Lieferung:** `blend_food_quality(10,20,0,1)` ergibt 20,
   weil die höhere Marktqualität vor `delivered<1` übernommen wird
   ([97–105][quality]). Spätere World-Integration muss echte Lieferung
   sicherstellen oder diese Semantik durch Originalevidenz erklären.
5. **Zeit/Visits:** Header-Angaben 51-Phasen-Pass, 16 Schritte/Monat, 12 Monate/Jahr
   ([Zeitvertrag][time]) belegen keine Gleichsetzung mit 20-Hz-World-Ticks.
   `apply_service_visit` führt keine Route/Kaufzahlung/Walker-Ankunft aus
   ([Visit][visit]).
6. **Quellenreihenfolge:** Fork sättigt pro Add ([Desirability.cpp:57–60][spread-add]),
   Upstream erst die Summe ([WorldDesirability][world-des]). Gemischte Quellen
   und permutierte Reihenfolgen benötigen unabhängige Vergleichsvektoren.

## Tatsächlich ausgeführte Diagnose

- Revisions-/API-Abfrage und elf exakte Git-Blob-Vergleiche: PASS.
- Isolierter synthetischer Probe gegen unveränderte `ModelData.cpp`,
  `Desirability.cpp`, `HouseEvolution.cpp`, C++20 mit Warnungen und
  ASan/UBSan: PASS; keine Compiler-/Sanitizer-Meldung.
- Reproduziert: Duplicate-ID überschreibt, zusätzliche House-Felder ignoriert,
  Qualität steigt trotz Lieferung 0, beide Bronze-/Lacquer-Bestände vermindert.
  Außerdem Visits/Decay, quadratische Ringe und zu kurze Modellzeile geprüft.
- Lokale Modelldatei zusätzlich read-only geladen, Tabellenform geprüft.
  Das beweist keine vollständigen Original-Regelwerte.
- Belege: `.local/original-economy/oe00/fork-probe.cpp`, `fork-probe.log`.
- Unveränderte Fork-eigene `OriginalRulesTests.cpp` gegen alle vier lokalen
  Fork-Modulimplementierungen und die frisch gebaute aktuelle Release-Library
  `.local/reswall/release-build/libopenemperor-simulation.a`: **PASS** mit
  `OPENEMPEROR_GAME_DIR=.local/gog-extracted/app`. Alle zwölf Aufrufe aus
  `main()` ausgeführt, einschließlich realer Modelldatei, Delivery-/Stepper-
  und World-View-Untertests; kein Untertest wegen fehlender Umgebung übersprungen.
  Build mit C++20, `-O3 -DNDEBUG -Wall -Wextra -Wpedantic -Wconversion -Wshadow`,
  keine Compiler-Meldung. Die verwendeten World-Dateien sind zwischen den
  Revisionen blob-identisch. Logs/Manifest: `fork-original-rules-build.log`,
  `fork-original-rules-tests.log`, `fork-original-rules-tests.json` unter
  `.local/original-economy/oe00/`. Dies ist der Fork-eigene Regelsuite-Test auf
  macOS gegen die gleiche World, keine unabhängige Originalparitätsprüfung.
- Vollständiger Fork-/Windows-Build: NOT_RUN, außerhalb dieses Scopes.
  Ein Probe-PASS gibt Gate A nicht frei.

SHA-256 der gelesenen `Model/EmperorBuildingModels.txt`:
`7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6`.
Keine Originaldatei/umfangreiche Tabelle wird in Git kopiert. Lokale
Fremdquellen behalten ihre Lizenz-/Urheberzuordnung; Fork-LICENSE nennt
GPL-3.0-only. Kein produktiver Codeimport in OE-00.

```sh
c++ -std=c++20 -Wall -Wextra -Wpedantic \
  -fsanitize=address,undefined -fno-omit-frame-pointer -g \
  -I .local/original-economy/oe00/fork/src \
  .local/original-economy/oe00/fork-probe.cpp \
  .local/original-economy/oe00/fork/src/simulation/original/ModelData.cpp \
  .local/original-economy/oe00/fork/src/simulation/original/Desirability.cpp \
  .local/original-economy/oe00/fork/src/simulation/original/HouseEvolution.cpp \
  -o .local/original-economy/oe00/fork-probe
.local/original-economy/oe00/fork-probe .local/gog-extracted/app

c++ -std=c++20 -O3 -DNDEBUG \
  -Wall -Wextra -Wpedantic -Wconversion -Wshadow \
  -I .local/original-economy/oe00/fork/src -I src \
  .local/original-economy/oe00/fork/tests/OriginalRulesTests.cpp \
  .local/original-economy/oe00/fork/src/simulation/original/ModelData.cpp \
  .local/original-economy/oe00/fork/src/simulation/original/Desirability.cpp \
  .local/original-economy/oe00/fork/src/simulation/original/HouseEvolution.cpp \
  .local/original-economy/oe00/fork/src/simulation/original/OriginalHousingView.cpp \
  .local/reswall/release-build/libopenemperor-simulation.a \
  -o .local/original-economy/oe00/fork-original-rules-tests
OPENEMPEROR_GAME_DIR=.local/gog-extracted/app \
  .local/original-economy/oe00/fork-original-rules-tests
```

End-to-End-Originalwirtschaft, neue Persistenz und vollständige Originalparität
sind durch die puren/read-only Fork-Bausteine nicht nachgewiesen.

[branch]: https://api.github.com/repos/amscova/OpenEmperor/branches/windows-port
[compare]: https://github.com/amscova/OpenEmperor/compare/fef111deff4cedd3d3fa48663556d41b2e4b31a6...d861d2912337c09751cb68196f85d5a30752726e
[cmake-sources]: https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/CMakeLists.txt#L264-L267
[cmake-test]: https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/CMakeLists.txt#L493-L498
[model-shape]: https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/src/simulation/original/ModelData.h#L12-L15
[model-access]: https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/src/simulation/original/ModelData.cpp#L65-L78
[model-loader]: https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/src/simulation/original/ModelData.cpp#L147-L153
[model-duplicates]: https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/src/simulation/original/ModelData.cpp#L109-L118
[model-extra]: https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/src/simulation/original/ModelData.cpp#L124-L129
[spread]: https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/src/simulation/original/Desirability.cpp#L57-L102
[spread-add]: https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/src/simulation/original/Desirability.cpp#L57-L60
[spread-contract]: https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/src/simulation/original/Desirability.h#L21-L26
[house-state]: https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/src/simulation/original/HouseEvolution.h#L11-L35
[evolve]: https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/src/simulation/original/HouseEvolution.cpp#L108-L198
[timers]: https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/src/simulation/original/HouseEvolution.cpp#L5-L35
[time]: https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/src/simulation/original/HouseEvolution.h#L50-L56
[visit]: https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/src/simulation/original/HouseEvolution.cpp#L201-L227
[goods]: https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/src/simulation/original/HouseEvolution.cpp#L37-L94
[quality]: https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/src/simulation/original/HouseEvolution.cpp#L97-L105
[needs]: https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/src/simulation/original/HouseEvolution.cpp#L120-L137
[view]: https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/src/simulation/original/OriginalHousingView.cpp#L17-L72
[view-contract]: https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/src/simulation/original/OriginalHousingView.h#L13-L30
[view-assumptions]: https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/src/simulation/original/OriginalHousingView.cpp#L53-L68
[world-rules]: https://github.com/oliverjessner/OpenEmperor/blob/fef111deff4cedd3d3fa48663556d41b2e4b31a6/src/simulation/World.h#L114-L146
[world-level]: https://github.com/oliverjessner/OpenEmperor/blob/fef111deff4cedd3d3fa48663556d41b2e4b31a6/src/simulation/World.cpp#L661-L689
[world-des]: https://github.com/oliverjessner/OpenEmperor/blob/fef111deff4cedd3d3fa48663556d41b2e4b31a6/src/simulation/WorldDesirability.cpp#L18-L66
[world-arrival]: https://github.com/oliverjessner/OpenEmperor/blob/fef111deff4cedd3d3fa48663556d41b2e4b31a6/src/simulation/World.cpp#L2338-L2371
[world-demand]: https://github.com/oliverjessner/OpenEmperor/blob/fef111deff4cedd3d3fa48663556d41b2e4b31a6/src/simulation/World.cpp#L2052-L2104
[world-stock]: https://github.com/oliverjessner/OpenEmperor/blob/fef111deff4cedd3d3fa48663556d41b2e4b31a6/src/simulation/World.h#L302-L359
