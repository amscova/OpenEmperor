# FORK_DELTA – OE-00 live geprüft, Windows ausgenommen

Stand 2026-10-16: lokaler Upstream
`fef111deff4cedd3d3fa48663556d41b2e4b31a6` gegen live bestätigten
`amscova/OpenEmperor`-Branch `windows-port`
`d861d2912337c09751cb68196f85d5a30752726e`.
[GitHub-Branch-API](https://api.github.com/repos/amscova/OpenEmperor/branches/windows-port)
und `git ls-remote` stimmen überein. Der
[gebundene Vergleich](https://github.com/amscova/OpenEmperor/compare/fef111deff4cedd3d3fa48663556d41b2e4b31a6...d861d2912337c09751cb68196f85d5a30752726e)
meldet **11 ahead / 4 behind**, gemeinsame Basis
`c908fdc0b18db6ffa6b3eaba4352a38a7f2c87bf`. Der historische Paketwert 11/3
bezog sich auf den älteren Upstream `f7a5035e92a9`.

Der genaue Belegbericht mit festen Code-Permalinks, Testgrenzen und Kommandos
steht in [fork-comparison.md](../original-economy/fork-comparison.md).
Elf lokal ignorierte Fremdquelldateien sind exakt gegen die Git-Blob-SHAs des
Fork-Trees geprüft; Manifest unter
`.local/original-economy/oe00/fork/file-manifest.json`.

| Bereich | Belegter Fork-Baustein | Produktive Grenze |
| --- | --- | --- |
| Modelldaten | [ModelData.h:12–45](https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/src/simulation/original/ModelData.h#L12-L45):13/24 Felder,15 House-Zeilen, fünf Difficulty-Modifikatoren | Keine neue World-Authority; Loader ohne SHA-/Dateigrößenbindung; Duplicate-ID-/Zusatzfeldpolitik offen |
| Desirability | [Desirability.cpp:57–102](https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/src/simulation/original/Desirability.cpp#L57-L102): Ringe, Sättigung je Add,16-Sektor-Mauerabschirmung | Mauerverhalten `Intended`, kein Beweis exakter Originalparität; alte Manhattan-Regel unverändert |
| HouseEvolution | [HouseEvolution.cpp:108–198](https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/src/simulation/original/HouseEvolution.cpp#L108-L198): Needs/Counter/Elite-Fälle | Eigener `HouseState`, kein produktiver World-Level-/Save-Ersatz |
| Service/Religion | [HouseEvolution.cpp:201–227](https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/src/simulation/original/HouseEvolution.cpp#L201-L227): Visits und Decay/Evolution/Consume | Kein Kauf-/Routen-/Arrival-Nachweis oder produktive Original-Zyklusphase |
| Waren/Qualität | [HouseEvolution.cpp:37–105](https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/src/simulation/original/HouseEvolution.cpp#L37-L105): sechs Waren, lokaler Transfer/Verbrauch/Qualität | Kein vollständiger bezahlter Producer→Market→Courier→House-Kreis für neue Waren |
| OriginalHousingView | [OriginalHousingView.cpp:17–72](https://github.com/amscova/OpenEmperor/blob/d861d2912337c09751cb68196f85d5a30752726e/src/simulation/original/OriginalHousingView.cpp#L17-L72): Bericht aus `const World&` | **READ-ONLY INTEGRATION** mit angenäherten Modelltypen, `max(population,1)`, Food-Quality-Option und angenommenen stetigen Herbalist-Wiederbesuchen |

## Reproduzierte Review-Punkte

- Doppelte Building-ID überschreibt den vorherigen Eintrag; House-Zeilen mit
  mehr als 24 Zahlen behalten nur die ersten 24.
- Lacquerware und Bronzeware werden beide mit `HF_Bronzeware` verbraucht;
  Needs akzeptiert dagegen das Maximum. Originalsemantik **NOT VERIFIED**.
- `blend_food_quality(10,20,0,1)` ergibt 20 trotz Lieferung null.
- 51-Phasen-Pass/16 Schritte pro Monat/zwölf Monate pro Jahr sind
  Fork-Vertragsangaben, keine bestätigte World-Tick-Gleichsetzung.

Ein isolierter C++20-Probe gegen die unveränderten drei puren Fork-Module läuft
mit ASan/UBSan ohne Meldung. Er reproduziert die Grenzen, prüft synthetische
Ringe/Visits/Decay, Verweigerung einer zu kurzen Modellzeile und die Form der
lokalen Modelldatei. Das ist kein Gate-A-Paritätsnachweis und kein vollständiger
Fork-/Windows-Build. Befehl und Log im Belegbericht.

Zusätzlich **PASS**: unveränderte Fork-eigene `OriginalRulesTests.cpp`, alle
zwölf Testaufrufe einschließlich Installations-/Delivery-/Stepper-/World-View-
Untertests mit gesetztem `OPENEMPEROR_GAME_DIR`, gegen die vier Forkmodule und
frisch gebaute aktuelle Release-Simulation-Library. Keine Compiler-Meldung.
Belege: `.local/original-economy/oe00/fork-original-rules-tests.{json,log}` und
`fork-original-rules-build.log`. Dies ist ein macOS-Regelsuite-Test gegen die
blob-identische World, kein kompletter Fork-/Windows-Build und keine unabhängige
Originalparitätsprüfung. Exakter Reproduktionsbefehl im Belegbericht.

`World.h`, `World.cpp`, `WorldDesirability.cpp`, `WorldHealth.cpp` und
`SandboxSave.cpp` sind im aktuellen Upstream und Fork blob-identisch; die
zusätzlichen Module ersetzen keine bestehende Simulation oder Save-Logik.
Keine produktiven Quelländerungen oder pauschalen Fork-Merges in OE-00.
Originaldateien bleiben read-only und außerhalb von Git. Ein späterer gezielter
Codeimport benötigt Lizenz-/Urheberzuordnung und unabhängige Tests; fremder Code
ist kein unabhängiger Originalbeleg.

## OE-06 – unabhängig belegte Präzisierung

Der [Liefervertrag](../reverse/original-goods-delivery-contract.md) bestätigt
sechs getrennte Bestände, zeigt aber nächste Modellzeile/Zeile7-Ausnahme,
Elitefaktor und eine quellenabhängige Nachfüllformel, die dem Fork fehlen. Der
[Konsum-/Qualitätsvertrag](../reverse/original-goods-consumption-food-contract.md)
bestätigt den unabhängigen Bronze-/Lackverbrauch über Feld13; die ODER-Needs
sind kein Beleg für alternativen Konsum. Food-Blending verwendet Vorbestand
statt Foodbedarf und speichert einen binary32-Quotienten. Der neue sichere
Kernel verweigert negative Inventare und erhält Quality bei Lieferung0; diese
Grenzen sind ausdrücklich authored. Die vollständige unabhängige Vergleichs-
und Gate-A-Abnahme folgt OE-07.
