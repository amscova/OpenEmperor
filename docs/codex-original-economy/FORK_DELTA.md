# FORK_DELTA – OE-00 live geprüft, Windows ausgenommen

**Aktueller OE33-Vergleich vom2026-10-20:**
[final-comparison.md](../original-economy/final-comparison.md) vergleicht
OpenEmperor `2fbf270` mit Fork `992f90ce3071f66cff3d82b31aaa7550b018adb5`.
Der Fork hat inzwischen die frühen Rule7–9-Grundlagen übernommen. Der folgende
OE00-Bericht und die anschließenden Tasknotizen bleiben historische Belege;
ihre Beschreibung als isolierter Read-only-Prototyp gilt nicht pauschal für
den heutigen Fork.

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

## OE-07 – ausführbarer Vergleich und Entscheidungskatalog

Der [portable JSON-Bericht](../original-economy/core-parity.json) enthält74
unabhängige synthetische Kontrollfälle mit getrennten erwarteten und tatsächlichen
Fork-/Kernwerten. Alle vier gepinnten Forkmodule werden tatsächlich kompiliert
und aufgerufen, einschließlich eines separaten read-only HousingView-Probes.
Die [Entscheidungsmatrix](../original-economy/core-parity.md) trennt46
Vektorenparitäten,18 begründete Korrekturen, sechs ungeklärte Annahmen und vier
bewusst ausgeschlossene Verhaltensweisen; zusätzliche Sourceaudits werden
ausdrücklich getrennt. Originalvergleich:1800 vorbereitete Housewerte, Hash
und Digest ohne Zahlentabelle.

`max(population,1)`, ein gesetzter Food-Quality-Wert und stetige Herbalist-
Wiederbesuche bleiben Diagnoseannahmen. Der Forkvergleich übernimmt daraus
keine Gameplay-Authority. Gate-A-/Build-/CTest-/Legacy-Resultate und deren
konkrete Grenzen stehen in [PROGRESS](PROGRESS.md).

## OE-28 – tatsächliche Hausdiagnose statt hypothetischer Forkansicht

Der Original-House-Inspector liest den aktiven Rule10-Nachfolger über die
bestehende World-Diagnose und erhaltene Waren-/Food-/Besuchsbelege. Er setzt
keine Mindestbewohner, keine Foodqualität und keine stetigen Wiederbesuche.
Vier sitzungslokale Ansichten trennen Modellbedarf, reale Bestände,
Stadtproduktion/Reservierung/Fracht, Hauslieferung/Konsum und tatsächliche
Timer von bloßer Quellenverfügbarkeit. Die begrenzten Supply-/DES-/Risk-
Overlays führen keine Forksimulation oder eigene Entwicklungsentscheidung aus.
Details und tatsächliche Prüfläufe stehen in
[housing-inspector-runtime.md](../original-economy/housing-inspector-runtime.md)
und [PROGRESS](PROGRESS.md). Es wird kein weiterer Forkcode importiert.

## OE-29 – belegte Hausfamilien statt angenommener Levelbilder

Die aktive Darstellung bindet tatsächliche HouseTypes3..17 über den separat
untersuchten Originalsetter und die bestehende General-Registrierung an native
SG3-Bilder. Sie übernimmt keine neue Forkänderung. Die bisherigen drei kuratierten
Legacy-Levelbilder bleiben unverändert und beweisen weiterhin keine Originalstufe.

Die neue metadata-only Ergänzung wählt ausdrücklich die statische Variante0;
die originale individuelle Variantenwahl wird nicht simuliert. Vacant2 bleibt
wegen unterschiedlicher Constructor-/Setterkontexte ein benannter Fallback,
ebenso der temporäre2×2-Eliteplot gegenüber dem originalen4×4-Bild. Bild- und
Ankeridentität sind Präsentationsbelege, keine zusätzliche World-, Bau-,
Höhen-, Save- oder Navigationsauthority. Quellen und offene Grenzen stehen in
[original-house-visuals.md](../reverse/original-house-visuals.md).

## OE-30 – bezahlter produktiver Einstieg

Der optionale Menüstart verwendet die bereits aktive Rule10-Simulation und
die normale Command-/Navigator-/Arrival-/Savekette. Ein neuer authored Bauplan
kauft vier leere Häuser und die bestehenden Quellen beiTick0; Modellwerte,
Bestände, Versorgung und Evolution werden nicht in einem zweiten Forkmodell
gesetzt. Erst tatsächliche Ticks erzeugen Bewohner, Waren, Konsum und Besuche.
Manuelles F5/F9 und ausdrücklich gewähltes Rule10-Laden verwenden die bestehende
strikte Original-Restorefunktion. Es wird kein Forkcode übernommen und keine
neue gespeicherte Authority eingeführt. Die belegten Abläufe und verbleibenden
Original-/Grafik-/Recoverygrenzen stehen in
[paid-starter-validation.md](../original-economy/paid-starter-validation.md).

## OE-31 – strikte aktive Save-/Recoverykette

Kein weiterer Forkcode und keine neue Entwicklungsentscheidung werden importiert.
Recovery verwendet dieselbe produktive World, die bestehende modellgebundene
Original-Restorefunktion und normale bezahlte Befehle/Ticks. Ein ausdrücklich
geöffneter aktiver Rule10-Stand erhält optional den vorhandenen Autosaveablauf;
Frozen7–9 und generischer Legacy-Restore behalten ihre Grenzen.

Die neue QA erzeugt reale Konsum-/Devolutions-/Reservierungs-/Debt-Zustände, statt
Foodqualität, Bestände oder Versorgung für einen erfolgreichen Restore zu setzen.
Die tatsächlichen Prüfungen, Legacy-Bytevergleiche und offen bleibende
Gesamtparität stehen in [save-recovery-validation.md](../original-economy/save-recovery-validation.md).

## OE-32 – beobachtete Langzeit- und Lastgrenzen

Kein zusätzlicher Forkcode, kein neues Limit in der World und keine neue
gespeicherte Authority werden übernommen. Die QA kauft die vorhandenen
Objekte, tickt dieselbe produktive Simulation und vergleicht einen ununterbrochenen
Creator mit einem separaten Restoreprozess. Ein gültiger authored Maximalfall
mit unverändertem Originalmodell bleibt vom echten Xia-Langlauf getrennt.

Schulden, echte Food-Konsumversäumnisse und wiederholte Hausrückstufungen werden
als Ergebnisse erhalten. Sie öffnen keine automatische Finanzierungs- oder
Modellfreigabe. Die Krankheit des unveränderten Legacy-Kontrollfalls ist kein
Beleg für Originalkrankheit. Messfenster, Quellenbindung, Kapazitätsgrenzen und
verbleibende Paritätsgrenzen stehen in
[longplay-validation.md](../original-economy/longplay-validation.md).

## OE-33 – aktueller Fork statt veralteter Ausgangsvergleich

Die unabhängige Bestandsaufnahme prüft675 Git-Blobs des aktuellen
`windows-port`-Heads `992f90ce3071f66cff3d82b31aaa7550b018adb5`. Dreizehn
Regelkern-/Ledgerdateien sind bytegleich zum lokalen Stand. Der Fork enthält
Modellbindung, Scheduler, bezahlte Well72-Quellen und das begrenzte Rule9-/
Schema25-Buchhaltungsfundament. Positive Warenbestände und produktiver
Original-Menüstart bleiben dort gesperrt. Der alte `OriginalHousingView` ist
im aktuellen Tree entfernt; dessen74-Vektor-Vergleich bleibt ausdrücklich an
den historischen `d861d291` gebunden.

Die neue aktive Rule10-Wirtschaft ergänzt bezahlte Produktions-/Transport-/
Konsum-/Housingkreisläufe, getrennte Timer, Geometrie, UI und Recovery über
dieselbe World und Navigation. Das ist ein belegbarer Integrationsunterschied,
keine pauschale Originalparität. Nicht-Haus-Grafiken, Original-Performer-/Venue-
Kette, Originalkrankheit und ein vollständig belegter bezahlter Durchstieg durch
alle15 Hausstufen bleiben offen. Gesamtziel/Gate F sind PARTIAL; ein technischer
Berichtsabschluss ersetzt diese fehlenden Funktionen nicht. Keine zusätzlichen
Forkdateien, Windowsanpassungen, Modelle oder Originalbytes werden importiert.
Die [Matrix](../original-economy/final-comparison.md) und der
[Abnahmebeleg](../original-economy/final-validation.md) führen aktuelle und
historische Prüfungen getrennt auf.

Die finale aktuelle Abnahme umfasst738 vollständige Konfigurations-CTests,
222 neue historische Pin-Kontrollen,21 neue ganze Creator-/Restorepaare und
exakte alte Savedoc-/Schema-/Policy-/Fingerprintbestände. Der Quellen- und
Originalfreeze bleibt identisch; der Endbeleg hat SHA256
`5a918e36819c13ae6fddecedb9ae0355ec3d5166f4bd04d2c0127a2169fa7c02`.
Dies schließt den technischen Vergleich ab, nicht die ausdrücklich offenen
Originalfunktionen oder Human Acceptance. Keine neuen Forkimporte.
