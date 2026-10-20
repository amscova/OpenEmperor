# OE32 – Longplay, Skalierung und Originalmap-Kompatibilität

Technische OE32-Abnahme: **PASS**.160 relevante Konfigurations-CTestläufe, fünf
finale Source3-Prozessabläufe, der Corpusvergleich, die alten Save-Exporte und
neun serielle Release-Benchmarkprozesse sind bestanden. Der abschließende
Beleg `.local/original-economy/oe32/final-validation.json` hat7755Bytes und
SHA256 `b072c760de3046c02f0d3397cbf2e1f6ef5769aabb7220f8e9ce986fee0d8f1b`.
Eine bestandene Gamebalance wird ausdrücklich nicht behauptet: Der stationäre
Vier-Häuser-Starter endet nach100k mit Schulden
und enthält echte Versorgungsdefizite. Dies ist keine Aussage, dass ein anders
gebautes oder ausgebautes Rule10-Spiel grundsätzlich unrentabel sein müsse.

Basis ist OE31 `d6226ed2de02413f4d462d0b968fe13b6f3d9f69`. Der
[vorab erklärte Vertrag](longplay-contract.md) erhält Rule10, Schema26, Policy3
und Entwicklungsrevision27. Es gibt keine neue Wirtschafts-, Karten-, Save-,
Navigations- oder Darstellungsauthority. Gate F folgt erst nach OE33.
Alle261 Produktionsdateien unter `src/`, sämtliche AGENTS-Dateien und die
1464Originaldateien/819711091Bytes bleiben unverändert; für die Originale sind
auch Modus und mtime exakt erhalten. Persönliche Saves und PrefPaths wurden
nicht verwendet.

## Tatsächlich ausgeführte Prozessvergleiche

Die fünf finalen Abläufe verwenden Source3:585 eingefrorene Quellpfade mit dem
Inventardigest
`be00e194e2d1998744218b503b113a946f87388cb189314061b81e9649e8a365`.
Die Datei `.local/original-economy/oe32/source-freeze-v3.json` hat SHA256
`55989501c9b73d8b204c825127cc7e2e3ae88a7d10df033fb78b474456cc700a`.
Der verwendete Release-Acceptance-Binary hat2300584Bytes und SHA256
`08b25ef01b2c0abb307c7a2593c6f8bfcdcf141afcae37f3cc247cc7e35985c8`.
Debug und ASan+UBSan verwenden ihre eigenen tatsächlich gebauten, in den
Prozessbelegen gehashten Binaries.

Jeder Ablauf startet drei echte Prozesse. Der Seedprozess kauft die Stadt mit
normalen Commands und führt2000 echte Ticks aus. Der Erwartungsprozess baut
dieselbe Stadt erneut frisch auf, führt dieselben2000 Ticks und gewöhnlichen
Staffingcommands aus und vergleicht danach den vollständigen Snapshot und die
frisch geschriebene SaveDocument-Datei mit dem geschützten Seed. Seine laufende
World wird dabei nicht durch einen Restore ersetzt. Erst der dritte Prozess
lädt den Seed tatsächlich und setzt ihn fort.

Je Ablauf stimmen sieben Dateipaare bytegenau überein: ein vollständiges
Creator-/Seed-SaveDocument sowie jeweils SaveDocument, vollständiger
World-Snapshot und deterministischer Beobachtungsbericht in der Mitte und am
Ende der Fortsetzung. Jeweils fünf Roh-Checkpoints werden zusätzlich geprüft.
Kapazitätsmessungen stehen getrennt in `*.resources.json`; Zeit/RSS gehören
nicht in die deterministischen Berichte.

| Ablauf unter `.local/original-economy/oe32/` | Beobachtungsfenster | Tatsächliche PIDs Seed/Creator/Restore | Technischer Prozessvergleich |
|---|---|---|---|
| `release/final3-real-20k-process` |2000→22000,20000 Ticks|70288 /70292 /70301|PASS,7 Dateipaare|
| `release/final3-real-100k-process` |2000→102000,100000 Ticks|70286 /70293 /70311|PASS,7 Dateipaare|
| `release/final3-districts-20k-process` |2000→22000,20000 Ticks|70287 /70305 /70405|PASS,7 Dateipaare|
| `debug/final3-real-20k-process` |2000→22000,20000 Ticks|70289 /70354 /71331|PASS,7 Dateipaare|
| `sanitizer/final3-real-20k-process` |2000→22000,20000 Ticks|70290 /70310 /70416|PASS,7 Dateipaare|

Die jeweiligen `report.json`-Belege haben SHA256:

- Release20k: `0b348ee0321e686a0ba3205702365dbbe3102fa2ac2b00e2acca8a2b1ae3afba`.
- Release100k: `c7f535879c770ad6dfc05e3ca6517229c8db118ad66f698413d6fab9198e1948`.
- Release Districts20k: `3e45759c3f528b20801cd6e80b3d23cf7f9f2ff9f7da24aaa502e7834d0cc61c`.
- Debug20k: `b58604d458dd38c9801093069deb54df035357995ef7cf968b20d0572ffe70e7`.
- ASan+UBSan20k: `c00f8ac648cbb3347b6e9cf88de99f2924d616dbb9fbe03e7cb3e1eb3b920cb8`.

Über alle fünf Abläufe sind dies15 tatsächliche Prozesse,35 literal gleiche
Dateipaare (30Fortsetzungs- und fünf Creator-/Seed-Paare) sowie25 unabhängig
gelesene Roh-Checkpoints. Originalinstallation und
Binary sind jeweils vor/nach dem Ablauf unverändert. Die Originalinventare
umfassen1464Dateien/819711091Bytes einschließlich SHA256, Modus und mtime.
Die separat ausgeführten Karten-/Policy- und alten Savevergleiche sind unten
mit ihren eigenen Belegen ausgewiesen.

Zusätzlich stimmen je sieben vollständige20k-Dateien aus Debug und
ASan+UBSan mit Release überein, insgesamt14 konfigurationsübergreifende
Dateipaare. Weitere21Paare vergleichen alle drei bisherigen Source2-Release-
Fälle mit den neuen Source3-Ergebnissen. Der unabhängige Beleg
`final3-process-crosschecks.json` hat SHA256
`4ca31624210913cb6c55a2c638337617483f7d53eba45dc633753f00690b77d8`.
Die früheren Source2-Reports und `source-freeze-v2.json` bleiben unverändert
als historische Läufe erhalten; sie werden nicht umetikettiert.

## Versorgung, Rückentwicklung und Funds

Die beiden Starterläufe verwenden die echte unveränderte Xia-Karte und das
unveränderte Normal-Modell, SHA256
`7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6`.
Die19 Gebäude/15 Couriers enthalten vier bezahlte Common-Häuser. Der Aufbau
kostet1404Funds; der Starter gewährt weder Einwohner noch Waren. Die folgenden
Ereigniszähler beziehen sich jeweils auf das Beobachtungsfenster nach dem
2000-Tick-Warmup, nicht auf die ganze Lebenszeit seit dem Kauf.
Es folgen keine Erweiterungskäufe: Die stationäre Stadt läuft mit regulären
Operation-/Priorityunterbrechungen weiter. Der volle Districtaufbau ist ein
absichtlicher Lastfall mit sämtlichen Quellen und Industrien, kein auf
Rentabilität optimierter Stadtbauplan.

| Messgröße | Xia20k | Xia100k | Zwei Districts20k |
|---|---:|---:|---:|
| Tatsächliche Hausbewertungen |1568|7844|7840|
| Aufstufungen / Rückstufungen |24 /18|82 /76|76 /75|
| Bewertungen mit unerfüllten aktuellen Anforderungen |329|1454|747|
| Bewertungen mit unerfüllten Anforderungen der nächsten Modellzeile |954|4637|7240|
| Tatsächliche Food-Konsumversäumnisse |27|116|679|
| Ausgelieferte Gütereinheiten, alle Transportabschnitte |739|3288|1213|
| Ausgelieferte Food-/Hilfsmaterialeinheiten |340 /0|1732 /0|608 /890|
| Angewandte Service-/Water-Ankünfte |1052 /611|5300 /3075|826 /207|
| Natürliche Feuerstarts |119|646|504|
| Beobachtete brennende Gebäude-Ticks |81583|397783|345528|
| Vor dem Tick unterbesetzte aktive Besitzer-Ticks |0|0|23660|
| Vor dem Tick pausierte Besitzer-Ticks |720|3600|720|
| Reguläre Operation-/Prioritycommands |16|80|16|
| Beobachtete Population Minimum→Maximum |113→167|113→167|340→503|
| Beobachtete Funds Minimum→Maximum |3202→9720|−22128→9720|−43787→1417|

Eine unerfüllte Anforderung wird am echten Bewertungsereignis aus
`last_evaluation.input` gezählt. Sie ist kein erfundener Service-Deadline-Miss.
Die nächste Modellzeile beschreibt ein Upgradehindernis; sie ist kein
gegenwärtiger Bedarf. Aktuelle und nächste Anforderungen bleiben getrennt.
Auf-/Rückstufungen werden bei jedem Ereignis beobachtet, nicht allein aus den
ersten historischen Übergängen abgeleitet. Staffelungswerte stammen nach den
gewöhnlichen Controls aus dem Zustand unmittelbar vor dem tatsächlichen Tick;
neue Einwohner oder ein Feuerwechsel im Tick schreiben diese Beobachtung nicht
nachträglich um.

Im Xia100k-Fenster entfallen1397 fehlende aktuelle Anforderungsbeobachtungen auf
FoodQuality und60 auf Hemp. Diese Kategorien überlappen; ihre Summe ist keine
zusätzliche Anzahl fehlgeschlagener Bewertungen. Die geplanten Güterkonsum-
ereignisse weisen fünf tatsächlich fehlende Hemp-Einheiten aus. Die116Food-
Versäumnisse stammen getrennt aus dem echten Konsumzähler für bewohnte Häuser
ohne vorhandenes Food. Momentane HUD-Bestände werden dafür nicht verwendet.

Die3288 ausgelieferten Gütereinheiten summieren alle beobachteten Transport-
abschnitte. Derselbe physische Gegenstand kann mehrere Abschnitte durchlaufen;
dieser Wert bedeutet weder3288 unterschiedliche Waren noch3288Hauslieferungen.
Dasselbe gilt für die Food-/Hilfsmaterialtransportzählung.

Am Ende von Xia100k gelten die tatsächlichen Lebenszeit-Buchungen:

`Funds = 12000 +15726 Tax −1404 Construction −48450 Maintenance = −22128`.

Die95 installierten Worker verursachen190Funds je400Ticks, unabhängig von
Pause, momentaner Besetzung und Feuer. Die15726Tax sind die tatsächlich bis
Tick102000 eingenommenen Steuern. Dieser feste Vier-Häuser-Fall finanziert
seinen Betrieb über das beobachtete Fenster nicht nachhaltig. Daraus folgt
keine allgemeine Unmöglichkeit einer profitablen Rule10-Stadt mit anderer
Zusammensetzung oder späterem Ausbau.

Hemp erhält seine Masse mit813 produziert =728 konsumiert +85 Bestand;
Ceramics mit552 produziert =472 konsumiert +80 Bestand. Alle sechs Waren-
bilanzen sind erhalten; die vier höheren Waren haben im Starter keinen Output.
Food wird pro Crop unabhängig aus Ernte, Besitzerbestand, Cargo und Verbrauch
geprüft. Der alte Legacy-Food-Produktionszähler bleibt dabei0 und wird nicht als
Original-Food-Nachweis verwendet. Navigation, Bevölkerung, tatsächliche
Workerallocation, Hilfsmaterialien und die vollständige Fundsrechnung werden
ebenfalls geprüft. Schulden und Versorgungslücken wurden nicht durch neue
Geldquellen, Modelländerungen oder Grants beseitigt.

## Bezahlte volle Stadt und Ressourcenbudgets

Der volle Aufbau erreicht die bestehenden82 Gebäude/58 Couriers mit5888Funds
und382 normalen Aufbaucommands. Er benutzt das unveränderte Originalmodell
auf einer ausdrücklich authored, gültigen228×228-Storage-Karte
`Cities/Influence.map`. Policy3 und Buildability werden über den produktiven
StoredMapSession-/Rendererpfad vorbereitet. Dieser Lastfall ist kein Beleg für
zwei passende leere Districtflächen auf der echten Xia-Karte.

Die zwei tatsächlichen getrennten Transportkomponenten enthalten jeweils41
Gebäude,29 Couriers und zehn Häuser. Source-/Parentbeziehungen, Targets,
Routen, Food-/Hilfsmaterialtransporte und Reservations bleiben in ihrer eigenen
Komponente. Am Endpunkt haben beide Komponenten reale Food-/Hilfsmaterial-
Annahmen,758 beziehungsweise815Einheiten. Funds und Workforce sind weiterhin
stadtweit; daraus wird keine finanzielle oder personelle Unabhängigkeit der
Districts abgeleitet.

Der Aufbau produziert alle sechs Güter. Am20k-Endpunkt sind die höheren
Outputs Tea144, Silk87, Bronzeware94 und Lacquerware8 jeweils vollständig in
Bestand/Cargo erhalten; ihre House-Konsumzähler bleiben0. Das ist Produktions-
und Bestanderhaltungsnachweis, kein behaupteter Vollausbau aller Wohnstufen.
Der Endstand hat503Einwohner,486 installierte/zugeteilte Worker und
−43787Funds.

| Am Endpunkt sichtbare Ressourcen | Xia100k | Zwei Districts20k |
|---|---:|---:|
| Buildings / Couriers |19 /15|82 /58|
| Rollenzulässige dynamische Zielslots |41|652|
| Darin tatsächlich gespeicherte erreichbare Pfade |41|326|
| Formaler äußerer Slotdeckel58×81 |4698|4698|
| Warenbesitzer / Warenakteure / Reservations |7 /3 /3|38 /16 /16|
| Sichtbare State-Vektorkapazitätsbytes |51388|264672|
| Sichtbare Pfad-/Cache-Slot-Kapazitätsbytes |4504 /1640|106768 /26080|
| Summe dieser sichtbaren Kapazitätsbytes |57532|397520|

Die getrennten Goods-Grenzen46/26/26 bleiben unverändert. Unerreichbare
zulässige Targets belegen ebenfalls Cache-Slots:652Slots bedeuten deshalb
nicht652erreichbare Verbindungen. Die während der Fortsetzung beobachteten
Highwaterwerte betragen41/652Slots,194/2069 aktive Pfadzellkapazitäten und
402/11538 gespeicherte Routezellkapazitäten, jeweils über die Couriers summiert.
Diese Zahlen messen vorhandene
öffentliche Vektoren. Sie sind weder vollständige Heap-/RSS-Werte noch die
nicht öffentlich exponierten äußeren World-Vektorkapazitäten oder ein Beweis
allokationsfreier Frames. Es wurde keine zusätzliche Limitauthority eingeführt.

## Eingefrorene Karten- und Saveverträge

Der tatsächlich ausgeführte Source2-Baselinevergleich besteht vollständig:
167Originalkarten,165 vorbereitete/2 weiterhin verweigerte bekannte Konflikte,
1485Policyfälle sowie72 vollständige historische SaveDocuments und72 tatsächliche
World-Snapshots. Hinzu kommen zehn Bridge- und elf Ramp-Neuprozesspaare. Die
ursprüngliche Vertragsdatei behält SHA256
`12823612834d27099189c18e31ed4fed4761c9ff34486227b3075cf1069a49e5`.
Der neue Laufbeleg
`.local/original-economy/oe00/oe32-final-verification/run.json` hat SHA256
`feba9d199584d2e2ed8e231e2a5466482c5e9fadd51a596c24460f8b79483ea9`.

Der zusätzliche repräsentative Schemas1–25-Korpus ist in allen drei
Konfigurationen mit jeweils101 vollständigen Dateien bytegleich zur
unveränderten OE31-Referenz `legacy-before2/corpus/`. Die vorhandene
1–3→4-Normalisierung bleibt Teil dieses eingefrorenen Vertrags. Die72 komplexen
alten Saves und diese25-Schema-Probe ergänzen sich; keiner der beiden Belege
ersetzt den anderen.

Die `final-frozen-comparison.json`-Belege unter
`.local/original-economy/oe32/` haben SHA256:

- Release: `a1b778de08890c7c94b97b0484ce1bbd580d72f2029aef984fbd9249f839f155`.
- Debug: `9118790f3918042c4ad77e21525f6b1acd1031eeb227828d9c480ede30ba4359`.
- ASan+UBSan: `18dd5b7ffa5aba49fa7ed49f9014da387230526b355c8a4f8e5a1346188cb9f4`.

## CTests und Source2-/Source3-Belegkette

Source2-Release und ASan+UBSan bestehen jeweils alle50 ausgewählten CTests ohne
Fehler oder Skips. Nach dem begrenzten Source3-Übergang bestehen die fünf neuen
OE32-Suites in diesen beiden Konfigurationen erneut. Der Debug-Grundbatch
besteht ebenfalls alle50. Insgesamt sind160 Konfigurations-CTestläufe ohne
finale Fehler oder Skips abgeschlossen.

| Tatsächlich abgeschlossener Batch | CTests | CTestzeit | Prozessbeleg SHA256 |
|---|---:|---:|---|
| Debug Grundbatch,45 alte +5 Source3-Suites |50|2141,29s|`0416835532f2736e9e91d9db2ec5ff010850cfcc28e58545b0eef3841d01280b`|
| Source2 Release |50|118,85s|`7741d35685825b1971bbc017ce23facefa81e84c755df55f35d02d8180ef0b59`|
| Source2 ASan+UBSan |50|643,86s|`bb10fa67529d0aefad3252f4e9fb205883bad2431c445d1b7f81cf8e43e80a3d`|
| Source3 Release, neue OE32-Suites |5|18,01s|`f331c7c27bb34200e4c10be52aed4735eacae9e961dba323de38476e1f6c5999`|
| Source3 ASan+UBSan, neue OE32-Suites |5|150,15s|`2cf8ec5d2b3330f4a2cd03d50962672a5817e8db591a6ea512466061ae11d877`|

Source3 ändert ausschließlich zwei QA-Dateien. Der Legacy-Benchmarkaufbau
prüfte zuvor zu streng auch unbenutzte Fläche beziehungsweise Höhe0; seine
neue bezahlte Testgeometrie korrigiert diese Annahme ohne Legacy-Regeländerung.
`Fixture.describe` bezeichnet die bei der Zuordnungsaufnahme aktuelle
Commandsequenz jetzt zutreffend als `membership_capture_command_sequence`.
Dieser Diagnosewert ist keine eigenständige Aufbaucommand-Authority.

Die45 älteren Testquellen sowie sämtliche Produktiv- und Corpusquellen bleiben
bytegleich.106 zuvor erfasste alte ausführbare Dateien bleiben ebenfalls
bytegleich. Der Beleg `source3-legacy-binding.json` hat SHA256
`b1e1a1d05be9143d54a69e7190a2655f5cd666e9e79384b2a9ea4789a7760621`.
Der vollständige Source2-Baselinevergleich war vor diesem Übergang fertig und
quellenstabil; er ist kein nachträglich umbenannter Source3-Corpuslauf.

Beim Debug-Grundbatch änderten sich während des bereits laufenden alten
Legacytests diese zwei QA-Quellen. **Für diesen gesamten Debug-Batch wird keine
Unverändertheit aller Quellen behauptet.** Vor dem Start irgendeiner der fünf
neuen OE32-Suites wurden jedoch der abgeschlossene Source3-Build, die vier
betroffenen neuen ausführbaren Dateien und der bis dahin vorhandene
CTest-Logpräfix erfasst. `debug-new-tests-binding-before.json` hat SHA256
`1ffcca7ce2427d979749ddfbf0284d375d2f9cd2016c6e69a8da57e23e65e512`.
Die fünf neuen Debug-Suites wurden somit schon im Grundbatch mit den fertigen
Source3-Binaries ausgeführt; die45 alten Tests behielten ihre unveränderten
Quellen und Binärdateien. Die abgeschlossene
Abnahmestruktur umfasst50×3 +5×2 =160Konfigurationsläufe; eine zusätzliche Wiederholung
der bereits gebundenen neuen Debug-Suites ist dafür nicht erforderlich.
Dies bleibt eine relevante Auswahl und keine vollständige Gate-F-Suite.
ASan und UBSan liefen mit `halt_on_error`. LeakSanitizer ist auf diesem
Mac-Host nicht unterstützt und war mit `detect_leaks=0` deaktiviert.

Die bestandenen neuen Suites aller drei Konfigurationen enthalten die
produktiven Viewprüfungen an19/15 und82/58: jeweils100 pausierte Inspector-/
Hoverframes ohne einen der zwölf verbotenen Arbeitszähler,500 unveränderte
Motionereignisse,200 gehaltene Previewframes und genau einen koaleszierten
neuen RoadPlan beim geänderten Endpunkt. Snapshot, Route-/Preview-/Influence-
Caches, sichtbare Courier-Kapazitäten und Texturanzahlen bleiben unverändert.
Nach dem Shutdown verbleiben keine eigenen Building-/Walker-/Road-/Fire-/
StoredGraphics-Texturen. Dies ist kein Nachweis allokationsfreier Frames oder
einer nativen Bildrate.

## Serielle Lastmessung und Legacy-Kontrolle

Alle neun Prozesse liefen nach Abschluss der bekannten OE32-Build-/Testlast
seriell in eigenen Releaseprozessen auf Apple M2 Pro mit zehn logischen CPUs,
macOS27.0.1/arm64, SDL-Software-Renderer,1280×800 und1×Zoom. Je Szenario wurden
drei Wiederholungen mit wechselnder Reihenfolge ausgeführt, jeweils20000 echte
Ticks und300 warme pausierte Renderframes. Der Beleg
`.local/original-economy/oe32/benchmark-summary.json` hat16024Bytes und SHA256
`2c6ed400fecdcf09d6f93b2b750971e34eeaf355bc96985ec9452fb4edb25aec`.

Tick-/Framewerte sind der Median der drei jeweiligen Laufmittelwerte;
RSS ist der Median der drei Prozesspeaks. Die unterschiedlich großen Städte
und ihre Quellen-/Grafiklast sind absichtlich benannt, nicht als identischer
Gameplayload ausgegeben.

| Szenario | Gebäude / Couriers | Tickmittel, µs | Rendermittel, ms | Prozess-RSS, MiB | Verhältnis Tick / Render / RSS zu Legacy |
|---|---:|---:|---:|---:|---|
| Unveränderte Legacy-Kontrolle, echte Xia |11 /7|8,630|1,908571|110,390625|1 /1 /1|
| Original-Starter, echte Xia |19 /15|16,827|2,029662|128,031250|1,950 /1,063 /1,160|
| Voller authored Districtlastfall |82 /58|123,910|1,672427|147,031250|14,358 /0,876 /1,332|

Die ebenfalls separat gemessenen Vorbereitungszeiten sind Mediane der drei
Wiederholungen:

| Szenario | Gemeinsame QA-Quellenvorbereitung, ms | Bezahlter Aufbau, ms | View-/Save-/Restore-/Warmupvorbereitung, ms |
|---|---:|---:|---:|
| Legacy |148,944|6,779|483,832|
| Original-Starter |152,868|51,587|555,520|
| Volle Districts |351,968|1902,920|216,639|

Die erste Spalte misst die gemeinsame QA-Kontextvorbereitung einschließlich
der jeweils notwendigen Map-/Modellvorbereitung. Sie ist keine Messung des
normalen App-/Menüstarts. Aufbau, Save/Restore, zusätzliche Invarianten,
Beobachter, JSON und Dateiarbeit liegen außerhalb der Tick-/Renderfenster;
Engine-interne Invarianten innerhalb von `World::tick` bleiben gemessen.
RSS enthält dagegen den gesamten Prozess einschließlich Vorbereitung, SDL und
Beobachter; es ist kein vollständiges World-Heapinventar. Die Laufzeiten der
umfangreich beobachteten Prozess-Replaytests werden nicht als Benchmark
umgedeutet.

Der volle authored Lastfall verwendet0Building- und0Walkertexturen gegenüber
12/176 in der Legacy-Kontrolle und27/176 im Original-Starter. Seine118
dynamischen Painterdeskriptoren enthalten auch offscreen eingereichte Elemente;
sie bedeuten nicht118 gleichzeitig sichtbare native Sprites. Der niedrigere
Renderwert des authored Falls belegt daher keine günstigeren Kosten nativer
Originalgrafik bei82 Gebäuden. Die Werte sind deskriptive Lastverhältnisse auf
diesem Host, keine Framerate-/FPS-Zusage, keine Metal-Messung und kein Beweis
nativer Frame-Pacing-Parität.

Der unveränderte Legacy20k-Kontrolllauf beobachtet je Wiederholung77 natürliche
Feuerstarts,20 natürliche Krankheitsstarts und24000 kranke House-Ticks sowie
400 pausierte/unbesetzte und9760 aktiv besetzte Ticks des kontrollierten
Market. Diese Benchmark-Staffingwerte sind ausdrücklich nach dem Tick
beobachtete Zustände und von den Pre-Tick-Werten des Longplaybeobachters
getrennt. Es wurde keine Originalkrankheit simuliert oder injiziert.

## Wiederholung und Grenzen

Für jede Wiederholung sind neue private Ausgabepfade zu wählen; bestehende
Dateien/Ordner werden verweigert. Die Beispiele setzen den vorhandenen lokalen
Release-Build und die explizite Originalinstallation voraus:

```sh
cmake --build .local/original-economy/oe32/release/build --parallel 2 --target openemperor-original-longplay-acceptance openemperor-original-longplay-benchmark
ctest --test-dir .local/original-economy/oe32/release/build --output-on-failure -R '^original-longplay-'
python3 tools/test_original_longplay.py --binary .local/original-economy/oe32/release/build/openemperor-original-longplay-acceptance --data .local/gog-extracted/app --output .local/original-economy/oe32/repro-xia-20k --scenario starter --ticks 20000
python3 tools/test_original_longplay.py --binary .local/original-economy/oe32/release/build/openemperor-original-longplay-acceptance --data .local/gog-extracted/app --output .local/original-economy/oe32/repro-xia-100k --scenario starter --ticks 100000
python3 tools/test_original_longplay.py --binary .local/original-economy/oe32/release/build/openemperor-original-longplay-acceptance --data .local/gog-extracted/app --output .local/original-economy/oe32/repro-districts-20k --scenario districts --ticks 20000
.local/original-economy/oe32/release/build/openemperor-original-longplay-benchmark --scenario legacy --data .local/gog-extracted/app --resources .local/original-economy/oe32/release/build/resources --output .local/original-economy/oe32/repro-legacy-benchmark.json --ticks 20000 --frames 300
```

Die14 Runner-Vertragstests prüfen zusätzlich Args/Help, fehlende Pflichtdaten,
neue/getrennte/symlinkfreie Ausgaben, echte Childfehler, Byteabweichungen,
Inputmutation und die Trennung von unrestored Creator und Restore. Ihre
ausdrücklich gefälschten Childprogramme testen nur Fehlerweitergabe und sind
kein Gameplaybeleg. Fehlende Pflichtdaten oder ein nicht ausgeführter100k-Lauf
dürfen nicht als PASS gelten.

Rule10 simuliert keine Legacy-Krankheit. Im Xia100k-Fenster bleiben1900000
geprüfte Gebäudezustände health-neutral; im District20k-Fenster sind es1640000.
Der ergänzende natürliche Krankheitsnachweis stammt allein aus dem oben
beschriebenen unveränderten Legacy-Kontrolllauf. Ein Herbalist-Besuch im
Originalmodell wird nicht als Krankheitsheilung ausgegeben.

Deterministische technische Stabilität und vollständiger Bestanderhalt sind
von Spielbalance, langfristig positiver Finanzierung und lückenloser Versorgung
getrennt. Die gemessenen Schulden, Rückentwicklungen und Food-Lücken belegen
keine universelle Versorgung oder Starvation-Freiheit. Native OS-Eingabe,
Human Acceptance bleiben **NOT_RUN**, vollständige Originalgrafikparität bleibt
**PARTIAL**. Gate F und OE33 bleiben **TODO**. Ein lokaler OE32-Commit ist nach
dieser technischen Abnahme autorisiert; dieser Bericht beginnt OE33 nicht und
autorisiert keinen Push, Merge, Tag oder Release.
