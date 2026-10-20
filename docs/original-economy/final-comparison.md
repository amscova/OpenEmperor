# OE33: Endvergleich und Übergabe

Stand: 2026-10-20, Arbeitsbasis
`2fbf27066ad277b15bcc4bbfc176bd3befb95c5e` (OE32).
Die aktive Originalwirtschaft ist **City-v16 / Rule10 / Schema26 /
Entwicklungsrevision27 / MapPermissions policy3**. Die historischen Rule7-
Bezeichnungen im Prompt meinen gemäß [Ausführungsvertrag](completion-contract.md)
diesen ausdrücklich gewählten Nachfolger. Regeln1–9 und Schemas1–25 bleiben
eingefroren. Normale Defaults werden nicht geändert.

**Gesamtziel: PARTIAL.** Bezahlte World-Integration, echte Transporte, originale
Modellregeln, strikte Persistenz und belastbare Langläufe sind vorhanden.
Vollständige Originalobjektgrafik, eine bezahlte vollständige obere Hauskette
und weitere unten genannte Originalgrenzen fehlen. Daraus folgt weder
„besser als Fork: PASS“ noch eine Releasefreigabe. Technische OE33-Abnahme und
das Gesamtziel sind getrennte Aussagen. Die endgültigen Gate-F-Prüfungen
stehen im Abschnitt [Finale Prüfungen](#finale-prüfungen); bis zu ihrem
Abschluss ist dieser Abschnitt ausdrücklich offen.

## Zwei gebundene Forkstände

Der Fork hat sich seit OE00 wesentlich verändert. Eine Beschreibung des
**aktuellen** Forks als bloßen isolierten Read-only-Prototyp wäre falsch.

| Referenz | Vollständige Revision | Aussagegrenze |
|---|---|---|
| OE00/OE07-Studienstand | `d861d2912337c09751cb68196f85d5a30752726e` | Vier zusätzliche Module, hypothetischer `OriginalHousingView`, eigener Pure-Regeltest. Die historischen74 Vergleichsfälle gehören zu genau diesem Stand. |
| Live bestätigter `amscova/OpenEmperor:windows-port` | `992f90ce3071f66cff3d82b31aaa7550b018adb5` | Integriert Upstream einschließlich OE13 über Merge `a36b8f6d9c9b9e1ab9233571a44fb6daa6ec31fe`; Frozen7–9, Schemas23–25, gebundene Modelle, Scheduler, bezahlte Well72-Quellen und neutrales Warenledger. |
| OpenEmperor nach OE32 | `2fbf27066ad277b15bcc4bbfc176bd3befb95c5e` | Zusätzlich aktives Rule10 mit den tatsächlich bezahlten Ketten von OE14–32. |

Am 2026-10-20 lieferte `git ls-remote` den genannten Live-HEAD. Der gebundene
[GitHub-Vergleich][fork-compare] zählt53 Commits ahead und0 behind gegenüber
dem historischen Studienstand. Das ist **kein** Ahead-/Behindvergleich zum
heutigen OpenEmperor. Der aktuelle GitHub-Tree ist vollständig
(`truncated=false`,712 Einträge einschließlich Trees);675 tatsächliche
Dateiblobs wurden unabhängig aus dem isolierten Checkout gegen ihre
Git-Objekt-IDs geprüft. Es fand kein Forkimport, Cherry-Pick oder Windowsbuild
statt. Der aktuelle Fork selbst wurde in OE33 nicht gebaut.

Im Live-Fork fehlen die historischen Dateien
`src/simulation/original/OriginalHousingView.{h,cpp}` und
`tests/OriginalRulesTests.cpp`. Ihre alten Ergebnisse werden deshalb nicht als
Tests aktueller Forkdateien ausgegeben. Dreizehn aktuelle Kern-/Ledgerdateien
sind dagegen **bytegleich** mit dem lokalen Produktivcode:

- `src/simulation/original/ModelData.{h,cpp}`
- `src/simulation/original/Desirability.{h,cpp}`
- `src/simulation/original/HouseEvolution.{h,cpp}`
- `src/simulation/original/GoodsRules.{h,cpp}`
- `src/simulation/original/ServiceRules.{h,cpp}`
- `src/simulation/original/OriginalScheduler.h`
- `src/simulation/OriginalGoodsLedger.{h,cpp}`

Die lokalen Tests dieser exakt identischen Kerne prüfen damit auch die
identischen Algorithmen des aktuellen Forkstands. Sie beweisen keinen
vollständigen aktuellen Forkbuild, dessen andere Integrationsdateien oder
Windows. `ModelSchema`, `OriginalRulesModel`, World, UI und Saves unterscheiden
sich; aus den13 gleichen Dateien wird keine Gesamtidentität abgeleitet.

Der aktuelle Fork zeigt seine Laufzeitgrenze ausdrücklich selbst:
[World.h:222–239][fork-world] kennt Rule7/8/9,
[WorldOriginalGoods.cpp:33–46][fork-goods] verlangt leere Warenlisten und
Nullbestände, und [SandboxSave.cpp:823–886][fork-save] verweigert positive
Schema25-Warenhistorie. Der [Original-CLI-Gatepfad][fork-main] verweigert den
neuen Originalwirtschafts-Sessionstart weiterhin. Das schließt vorhandenes
Legacyspiel und seine Entwicklerwerkzeuge für Well-/Warenprüfung nicht aus.

## Reproduzierbare Featurematrix

Die vollständige maschinenlesbare Matrix steht in
[final-feature-matrix.json](final-feature-matrix.json). Sie enthält für jede
Zeile aktuelle Forkgrenze, lokalen Quellpfad, Belegdokument und reale CTestnamen
bzw. die ausdrücklich fehlende Abnahme. Die folgenden Statuswerte bewerten
den jeweils **genannten begrenzten** Funktionsumfang:

| Status | Bedeutung |
|---|---|
| `verified original` | Direkt gebundene Raw-Modellfakten oder begrenzt durch die identifizierte EXE belegte Originalmechanik. Statische Analyse ist keine ausgeführte Originalspielaufnahme. |
| `implemented authored` | Funktionierende OpenEmperor-Integration mit offen benannten eigenen Zeit-, Bau-, Finanzierungs- oder Ablaufverträgen. |
| `partially integrated` | Ein erforderlicher Teil der Integration, Originalzuordnung oder vollständigen End-to-End-Abnahme fehlt. |
| `not supported` | Funktion oder beanspruchte Abnahme fehlt; kein Ersatzbeleg zählt als PASS. |

| Funktion | Status | Tatsächlicher Stand und Beleg |
|---|---|---|
| Originale Modelltabellen | `verified original` |13 Building-/24 House-Felder,15 House-Zeilen, fünf Difficulty-Zeilen aus unveränderter lokaler Installation; [Provenance](model-provenance.md). Im aktuellen Fork derselbe Rawreader. |
| Difficulty-Feldtransformation | `verified original` | Belegtes Building-Prozent-/House-Additivverfahren; sichere Overflowverweigerung bleibt authored. [Faktenregister](model-provenance.md). |
| Modellquelle pro Session | `implemented authored` | Create/Restore prüfen Quelle, Hash, Größe, Schwierigkeit und vorbereitete Form; [Modellbindung](model-session-binding.md). Bereits Teil des aktuellen Forks, kein exklusiver OE14–32-Mehrwert. |
| Begrenzte Needs-/Evolutionsprädikate | `verified original` | Aktuelle/Next-Zeile, inklusive Grenzen, Water, neun Timer, Food Quality, sechs Waren, getrennte ODER-Bedarfe, Counter-/Elitefälle; [74 historische Vergleichsfälle](core-parity.md), unabhängige EXE-Belege. Der aktuelle Purekern ist bytegleich. Zwei opaque globale Kontextproduzenten sind unbekannt; die aktive World verwendet authored false, keine vollständige Evolutionskontext-Parität. |
| Originale Desirability insgesamt | `partially integrated` | Belegter Spread und bezahlte Quellen vorhanden; vollständige originale Mauerabschirmung und sämtliche Originalquellen fehlen. [Laufzeitgrenze](desirability-runtime.md). |
| Bezahlte DES-Quellen und Cache | `implemented authored` | Typisierte IDs, Kosten, Add/Remove, ereignisgebundener Cache; [Quellenvertrag](source-contract-v1-proposal.md). Der aktuelle Fork besitzt bereits Well72, ohne allgemeine Wasser-/Servicewirkung. |
| World-Zeit-/Kalenderabbildung | `implemented authored` | Gespeicherte51/16/12-Struktur; ein Worldtick pro Phase bleibt ausdrücklich authored. [Scheduler](scheduler.md). Gemeinsamer Kern, aktive spätere Ereignisse zusätzlich lokal. |
| Neun Timer und Visitprädikate | `verified original` | Fünf Service-/vier Religionstimer,96/40, Phase35, getrennte Staffing-/Admissionprädikate; [Servicekern](service-rules.md). Derselbe Purekern im Live-Fork. |
| Bezahlte Service-/Religionsankünfte | `implemented authored` | Reale Provider, Worker, Material, Dispatch, Navigator und Arrival. Confucian nur für bewohnte Elite mit geliefertem Paper; [Services](services-runtime.md), [Religion](religion-runtime.md), [Kultur](culture-runtime.md). Diese aktive Kette fehlt Frozen7–9. |
| Originale Kultur-Venue-/Trainingkette | `partially integrated` | Bezahlte Music-/Acrobat-/Drama-Schulbesuche funktionieren als experimenteller School-to-House-Adapter; ursprüngliche Venue-/Trainingvermittlung fehlt. [Grenze](culture-runtime.md). |
| Ceramics-Kreislauf | `implemented authored` | Clay/Kiln→Warehouse→Market→House mit tatsächlicher Übergabe und Fahrt; Conversion ist authored. [Produktion](production-runtime.md), [Markttransport](../../src/simulation/WorldOriginalMarket.cpp). |
| Hemp-Kreislauf | `implemented authored` | Bezahlte Farm192 und Feld194, Staffing, Ernte, Cargo, Arrival, Conservation; [Hemp](hemp-runtime.md). |
| Food und echte Qualität | `implemented authored` | Reale Farm-/Feld-/Markt-/Hauslieferung; bestandbezogenes originales Blending, authored Food-Sink. [Food](food-runtime.md). Keine hypothetisch gesetzte gespeiste Qualität. |
| Luxuswaren und Vorprodukte | `implemented authored` | Tea/Silk/Bronzeware/Lacquerware und fünf Vorprodukte mit bezahlten Rezepten, Transport und materialabhängigem Academy-Dispatch; [Luxusvertrag](luxury-runtime.md). |
| Warenverbrauchskern | `verified original` | Aktuelle Modellzeile, Schritte0/7, vorhandener Bestand als Grenze und unabhängiger Bronze-/Lackverbrauch; Größen-/Elitefaktoren gelten ausschließlich für den Lieferbedarf. [Konsum](consumption-runtime.md). Zeitabbildung und Origins sind separate authored Integration. |
| Physisches Warenledger in der World | `implemented authored` | Genau eine Authority je Stock/Cargo/Reservation, echte Arrival-/Return-/Cancelbuchung, Sequenzschutz und Conservation; [Ledger](goods-ledger-runtime.md). Aktueller Fork hat denselben Pure-Transaktionskern, erlaubt aber nur neutralen Worldzustand. |
| Bewohner und Workforce | `implemented authored` | Leere bezahlte Häuser,120-Tick-Mindestalter, alle40 Ticks höchstens2 Zuzügler, alleiniger HouseState und reale Worker; [Bootstrap](completion-contract.md). Keine Behauptung originaler Immigration oder Elitearbeiter. |
|15 Hauszeilen im produktiven Housing | `partially integrated` |15×5 Pure-/World-Matrix; frühe Commonentwicklung und11→13→12→13 bezahlte Eliteereignisse belegt. Kein bezahltes vollständiges Heavenly17. [Hausstufen](housing-stages-runtime.md). |
| Große Elitekörper | `implemented authored` | Atomisches2×2-Plot→4×4 bei tatsächlichem Erstbezug,16 Claims, gleiche ID, Konflikt-/Height-/Actor-/Restoreprüfung. [Elitegeometrie](elite-footprints-runtime.md). Plotpreis und Bootstrap bleiben authored. |
| Steuer, Schwierigkeit, Wohnziel | `implemented authored` | Fünf Modellschwierigkeiten, wirkliche Bewohner und belegte Hausrate, überprüfte Fundsrechnung; Periodisierung, Unterhalt und Ziel authored. [Finanzabnahme](finance-validation.md). |
| Spielbarer Menüstart und Inspector | `implemented authored` | Bezahlter Starter19/15; tatsächliche Bedürfnisse/Bestände/Ankünfte, Keyboard/Maus/F5/F9. [Starterabnahme](paid-starter-validation.md), [Inspector](housing-inspector-validation.md). Aktueller Fork bietet diese aktive Rule10-Kette nicht. |
| Native Original-Hausbilder | `partially integrated` |15 identifizierte Familien3..17, statische Variante0; Vacant2 und noch2×2-Eliteplot bleiben Fallbacks, individuelle Originalvariantenauswahl fehlt. [Bildabnahme](original-house-visuals-validation.md). |
| Originalobjektbilder und passende Walker | `partially integrated` | Bezahlte Objekte und echte Actors vorhanden; zahlreiche Quellen und zugehörige Walker verwenden weiterhin benannte Fallbacks. Vollständige Grafikforderung des Gesamtauftrags ist offen. [Startergrenzen](paid-starter-validation.md). |
| Strikte Saves, Recovery, Neuprozess | `implemented authored` | Rule10/Schema26/dev27, Malformation-/Daten-/Quellen-/Bilanz-/Referenz-/Routenprüfung und atomischer Publish; [Recovery](save-recovery-validation.md). Im Live-Fork bereits strikte Frozen7–9-Saves, aber keine aktive26er Authority. |
| Endurance und Ressourcenbudget | `implemented authored` | Echte20k/100k,82 Gebäude/58 Couriers, zwei Transportkomponenten, vollständige Creator-/Restorebytes, negative Ergebnisse erhalten; [Longplay](longplay-validation.md). Kein universeller Balancebeweis. |
| Originalmap-/Legacy-Kompatibilität insgesamt | `partially integrated` |167 geprüft,165 vorbereitet,2 bekannte Konfliktrefusals;1485 Policyfälle und ganze historische Saves bytegeprüft. Vorhandene Map-/Legacyfunktionen des aktuellen Forks werden anerkannt. Nicht sämtliche Originalkarten/Objekte vollständig spielbar. |
| Originale Krankheit/Heilung | `not supported` | Rule10 bleibt health-neutral. Tatsächlich beobachtete Krankheit gehört ausschließlich zur unveränderten Legacy-Kontrolle; ein Herbalisttimer ist keine Krankheitsrekonstruktion. [Longplaygrenze](longplay-validation.md). |
| Native menschliche Originalparität | `not supported` | Keine menschliche Endabnahme oder laufender Originalspielvergleich in diesem Auftrag. Scripted Softwarepixel sind davon getrennt. |

Die Klasse `verified original` behauptet ausschließlich die begrenzte belegte
Raw-/Regelmechanik der jeweiligen Zeile. Sie hebt keinen benachbarten
`implemented authored`-/`partially integrated`-Eintrag auf.

## Resultierende Architektur und nachgewiesener Mehrwert

Eine vorbereitete unveränderliche `OriginalRulesModel` bindet die lokale
Installation. Derselbe produktive `World` besitzt die getrennten Original-
House-, Stock-, Cargo-, Material-, Visit-, Zyklus- und Finanzzustände. Normale
bezahlte Commands, tatsächliche Worker und Bewegung über denselben Navigator
erzeugen Versorgung; keine zweite Forksimulation, Gratisbestände oder
Render-Ereignisse vergeben Zugang. Schema26 speichert ausschließlich die
versionierte Authority und bindet Modell, Map und policy3. View und SG3-Bilder
lesen den bereits vorhandenen Zustand. Vorbereitete Präsentation verleiht
keine Bau-, Transport- oder Saveberechtigung.

Die frühen Berichte [Architekturaufnahme](architecture-map.md),
[OE00-Forkvergleich](fork-comparison.md) und [OE07-Kernvergleich](core-parity.md)
beschreiben ihre jeweilige damalige Phase. Ihre Aussagen zu noch fehlender
Worldintegration sind historische Befunde, keine Beschreibung des aktiven
Rule10-Stands. Diese Endmatrix führt dessen heutige Integrationsgrenzen auf.

Der nachweisbare Unterschied zum **historischen** Read-only-Fork ist die echte
bezahlte Worldkette statt einer hypothetischen Ansicht. Gegen den **heutigen**
Fork ist der engere und richtige Mehrwert: positive Ursprünge aller sechs
Waren, Food/Material, tatsächliche Serviceankünfte, neue Bewohner/Worker,
Hausübergänge, große Elitegeometrie, Steuern, spielbarer Starter, Inspector,
aktive26er Recovery und gemessene Langzeitlast. Parser, Purekern,
modellgebundene Frozenversionen, Scheduler und Well72 dürfen dabei nicht als
fehlende Funktionen des aktuellen Forks verkauft werden.

Ein voller bezahlter82/58-Lastfall beweist keine komplette obere Wohnkette:
höhere Waren lagen im OE32-Endpunkt teilweise nur in Stock/Cargo. Der
eigene gültige authored Districtuntergrund ist kein originaler Xia-Stadtplan.
Unabhängig sind seine zwei Transportkomponenten; Funds und Workforce bleiben
stadtweit. Siehe [Messgrenzen](longplay-validation.md).

## Finale Prüfungen

**Technische OE33-Abnahme: PASS. Gesamtziel und Gate F: PARTIAL.**
Alle738 neuen vollständigen Konfigurations-CTests (246 je Debug/Release/
ASan+UBSan) sind bestanden, ohne Fehler oder Skips. Hinzu kommen222 neue
Kontrollfälle gegen den historischen Forkpin, aktuelle675 Git-Blob-/13
Kernquellenvergleiche,167/165/2-Corpus und1485 Policies,72 alte ganze
SaveDocuments/Snapshots sowie101 ganze Schema1–25-Dateien je Konfiguration.
Drei neue20k-/100k-/District-Creator-/Restoreabläufe liefern21 vollständige
Bytepaare; ihre Endberichte stimmen mit OE32 exakt überein. Originaldateien,
AGENTS und alle585 Sourcepfade einschließlich261 Produktionsdateien bleiben
unverändert. Kein Modell-/Regel-/Schema-/Grafikfix für positive Resultate.

Das [vollständige neue Prüfprotokoll](final-validation.md) nennt tatsächliche
Befehle, Zeiten, Hashes und die erhaltenen vorbereitenden Aufruffehler.
Privater Endbeleg: `.local/original-economy/oe33/final-validation.json`,
10073Bytes, SHA256 `5a918e36819c13ae6fddecedb9ae0355ec3d5166f4bd04d2c0127a2169fa7c02`.
Die fehlenden verpflichtenden Originalfunktionen der Matrix bleiben offen;
ein technischer Berichtsabschluss ersetzt ihre Umsetzung nicht.

Die folgenden Ergebnisse sind **abgeschlossene frühere**, in ihren Berichten
quellengebundene Belege und werden nicht als neue OE33-Ausführung ausgegeben:

- OE31: feindliche Save-/Recoverydaten, echte Neuprozesse und alte Savebytes;
  [Save-/Recoveryabnahme](save-recovery-validation.md).
- OE32:160 relevante Konfigurations-CTestläufe, fünf echte20k-/100k-
  Creator/Restoreworkflows mit15 PIDs und35 ganzen Dateipaaren;167/165/2-
  Corpus,1485 Policies,72 ganze historische SaveDocuments und72 Snapshots,
 10 Bridge-/11 Ramp-Prozesspaare sowie101 ganze Schema1–25-Dateien pro
  Konfiguration. [Longplayabnahme](longplay-validation.md), privater
  OE32-Abschluss SHA256
  `b072c760de3046c02f0d3397cbf2e1f6ef5769aabb7220f8e9ce986fee0d8f1b`.
- OE07:74 unabhängige synthetische Kontrollfälle gegen exakt den alten
  Forkpin;46 Paritäten,18 begründete Korrekturen, sechs ungeklärte Annahmen
  und vier ausgeschlossene Verhaltensweisen. [Kernbericht](core-parity.md).
  Der heutige Fork ist über die tatsächliche Dateibindung zu vergleichen,
  nicht durch Umbenennen dieses historischen Resultats.

Die wichtigen negativen OE32-Ergebnisse bleiben gültig: Der feste
Vier-Haus-Starter endet nach100k bei **−22128 Funds** und **116 tatsächlichen
Food-Konsumversäumnissen**. Dies sind konkrete Ergebnisse ohne weitere
Spielerexpansion, kein Beweis, dass jede Rule10-Stadt unprofitabel wäre.
Der hohe Lastfall endet bei−43787 Funds; niemand hat Schulden durch Grants
oder Modelländerungen entfernt. Technische deterministische Stabilität ist
keine Abnahme dauerhafter positiver Finanzierung oder lückenloser Versorgung.

## Reproduktion und gebundene Artefakte

Der unveränderte alte Fork bleibt unter
`.local/original-economy/oe00/fork/`. Der neue isolierte Checkout liegt unter
`.local/original-economy/oe33/fork-current/`, detached und sauber. Er ist ein
Shallowcheckout; der tatsächliche Commitparent wurde aus dem rohen
Commitobjekt gelesen, nicht aus einer am Shallowrand verkürzten `%P`-Ausgabe.
Der Parent ist `3c2be89d1f099e169714e382255aae214aabbe21`.

| Neuer read-only Beleg | Bytes | SHA256 |
|---|---:|---|
| `oe33/fork-current-manifest.json` |196384|`a8e96944057256b021118788eeb8528efa33ecf5ce5bf7afb969c0db750b5307`|
| `oe33/fork-current-audit-v2.json` |7912|`341b014f7df1441a99fc89317e53c92e4f9784c530eda930c2cb78dd9e9a16b6`|
| `oe33/fork-current-compare.json` |1404823|`9dafc0d2ff596dcee0f44ac5a7fd3459158fb2b21c23544306699a5f4bd5c1d8`|

Alle Pfade in dieser Tabelle sind relativ zu
`.local/original-economy/`. Sie enthalten keine zur Veröffentlichung
vorgesehenen Originalspielbytes. Der erste Auditbericht bleibt zusätzlich
erhalten; V2 korrigiert ausdrücklich nur die Shallow-Parentdarstellung und
ergänzt den gebundenen APIvergleich.

Read-only-Kontrolle des vorhandenen Checkouts:

```sh
git ls-remote https://github.com/amscova/OpenEmperor.git refs/heads/windows-port
git -C .local/original-economy/oe33/fork-current rev-parse HEAD
git -C .local/original-economy/oe33/fork-current status --short
git -C .local/original-economy/oe33/fork-current cat-file -p HEAD
git diff --no-index -- src/simulation/original/HouseEvolution.cpp .local/original-economy/oe33/fork-current/src/simulation/original/HouseEvolution.cpp
```

`git diff --no-index` liefert0 bei Identität und1 bei Unterschieden. Zum
Reproduzieren an einem anderen Rechner den festen Commit in einen **neuen**
isolierten Pfad holen; keinen bestehenden Checkout/Forkbeleg überschreiben.
Ein später bewegter Branch-HEAD ersetzt keinen der hier gebundenen Commits.
Build-/CTest-/Neuprozess-Reproduktion folgt den tatsächlichen Kommandos des
[finalen Prüfprotokolls](final-validation.md) und der [Testsammlung](../testing.md). Originaldaten sind
zwingend die eigene explizite Installation; fehlende Daten zählen nicht als
PASS. ASan/UBSan und plattformbedingt fehlendes LeakSanitizer sind getrennt
zu protokollieren.

## Task-Commits OE00–32

Dies sind die tatsächlichen jeweiligen technischen Taskcommits auf dem
aktuellen Branch, keine neu erzeugten oder zusammengefassten Revisionen.
Die historischen OE12-/OE13-Blocker- und Quellenvertragscommits bleiben
zusätzlich in der Git-Historie; sie wurden nicht durch einen grünen
Featurecommit ersetzt oder rückwirkend als implementiert bezeichnet.

| Task | Vollständiger Commit | Ergebnisumfang |
|---|---|---|
| OE00 | [`05470049fa4acfdf2d0104f5a25384c9277073ac`](https://github.com/oliverjessner/OpenEmperor/commit/05470049fa4acfdf2d0104f5a25384c9277073ac) | Baseline und historischer Forkpin |
| OE01 | [`11211bbe6f579b6171615f6e75ed6fffbfd53cdf`](https://github.com/oliverjessner/OpenEmperor/commit/11211bbe6f579b6171615f6e75ed6fffbfd53cdf) | Sicherer Raw-Modellreader |
| OE02 | [`df13b2e9bc0112d66c188276b5753c92ce6a5a40`](https://github.com/oliverjessner/OpenEmperor/commit/df13b2e9bc0112d66c188276b5753c92ce6a5a40) | Modell-/Faktenregister |
| OE03 | [`12e2f22c94f60402824d8dd0898b9d36086da503`](https://github.com/oliverjessner/OpenEmperor/commit/12e2f22c94f60402824d8dd0898b9d36086da503) | Pure Desirability |
| OE04 | [`a8fd0ce0615d7009eecc373359c8c00b1a3207d4`](https://github.com/oliverjessner/OpenEmperor/commit/a8fd0ce0615d7009eecc373359c8c00b1a3207d4) | Pure HouseEvolution |
| OE05 | [`269e80d850ea7d0e8d90b4853a48cf86f8140bf8`](https://github.com/oliverjessner/OpenEmperor/commit/269e80d850ea7d0e8d90b4853a48cf86f8140bf8) | Service-/Religionstimer |
| OE06 | [`88e070865600b39d7296be7a6c821724c5cbe682`](https://github.com/oliverjessner/OpenEmperor/commit/88e070865600b39d7296be7a6c821724c5cbe682) | Warenbedarf/Transfer/Qualität |
| OE07 | [`26faf654c6d714a88c4b72ffb29c5e1d0b936add`](https://github.com/oliverjessner/OpenEmperor/commit/26faf654c6d714a88c4b72ffb29c5e1d0b936add) | Historischer Forkvergleich/Gate A |
| OE08 | [`ac052762fba501a85f241a4828726c2a9deb1e90`](https://github.com/oliverjessner/OpenEmperor/commit/ac052762fba501a85f241a4828726c2a9deb1e90) | Explizite FrozenRule7 |
| OE09 | [`a1a352bd697e8a424fb6d76875782d84d70b930a`](https://github.com/oliverjessner/OpenEmperor/commit/a1a352bd697e8a424fb6d76875782d84d70b930a) | Schema23 |
| OE10 | [`b4c22e6ecc49558e137288801d2e2a866cd8fc44`](https://github.com/oliverjessner/OpenEmperor/commit/b4c22e6ecc49558e137288801d2e2a866cd8fc44) | Gespeicherte Zeitabbildung |
| OE11 | [`b3b15a6ec98912a35982d9bcec30d7a651fed0a3`](https://github.com/oliverjessner/OpenEmperor/commit/b3b15a6ec98912a35982d9bcec30d7a651fed0a3) | Echte Modellbindung |
| OE12 | [`ec3a2d60c7541f8a11338c80d5398ce54e51f3a8`](https://github.com/oliverjessner/OpenEmperor/commit/ec3a2d60c7541f8a11338c80d5398ce54e51f3a8) | Angenommener Well72-Vertrag/Rule8/Schema24 |
| OE13 | [`887dda562622d59487a92b8966b62b218cb59ce3`](https://github.com/oliverjessner/OpenEmperor/commit/887dda562622d59487a92b8966b62b218cb59ce3) | Neutraler World-Ledgervertrag/Rule9/Schema25 |
| OE14 | [`2bd6878f0ab06af0764d84120f8bf9cb6c2dd726`](https://github.com/oliverjessner/OpenEmperor/commit/2bd6878f0ab06af0764d84120f8bf9cb6c2dd726) | Aktive Rule10/Ceramics-Ursprung |
| OE15 | [`390c87481b2259a0e95559e0c975e974a7b627af`](https://github.com/oliverjessner/OpenEmperor/commit/390c87481b2259a0e95559e0c975e974a7b627af) | Echter Markttransport |
| OE16 | [`86cde0c54577a9889632e45432fe7fe18acd7fac`](https://github.com/oliverjessner/OpenEmperor/commit/86cde0c54577a9889632e45432fe7fe18acd7fac) | Hemp-Farm/Feld |
| OE17 | [`b30718fe08fb118da5dce05a6846d10bfaf3adeb`](https://github.com/oliverjessner/OpenEmperor/commit/b30718fe08fb118da5dce05a6846d10bfaf3adeb) | Phasengebundener Konsum |
| OE18 | [`144eec699d4c39441ce6cca8f2d877f0fc546a92`](https://github.com/oliverjessner/OpenEmperor/commit/144eec699d4c39441ce6cca8f2d877f0fc546a92) | Food-/Qualitätskette |
| OE19 | [`ba116b4c97defdf134434c1b0398292fb3727d0e`](https://github.com/oliverjessner/OpenEmperor/commit/ba116b4c97defdf134434c1b0398292fb3727d0e) | Medizinische tatsächliche Visits |
| OE20 | [`825fd2c65d90c9139e283f2bf3e191bb5feb4e39`](https://github.com/oliverjessner/OpenEmperor/commit/825fd2c65d90c9139e283f2bf3e191bb5feb4e39) | Ancestorquelle/-ankünfte |
| OE21 | [`d0dae2830a7025c12b32ce783fa6de416e131b44`](https://github.com/oliverjessner/OpenEmperor/commit/d0dae2830a7025c12b32ce783fa6de416e131b44) | Wasser/frühe Hausentwicklung |
| OE22 | [`1e7e2715704bd7b5def483d4924292c1d4d41181`](https://github.com/oliverjessner/OpenEmperor/commit/1e7e2715704bd7b5def483d4924292c1d4d41181) | Weitere Religionen |
| OE23 | [`95967cec1e691a50e9a771a59940deda2230807b`](https://github.com/oliverjessner/OpenEmperor/commit/95967cec1e691a50e9a771a59940deda2230807b) | Kulturadapter/Diagnose |
| OE24 | [`7360f84e40c6cd558d3cf87de41c8b713645840f`](https://github.com/oliverjessner/OpenEmperor/commit/7360f84e40c6cd558d3cf87de41c8b713645840f) | Luxuswaren/Vorprodukte |
| OE25 | [`9bac35b5626834b42d35d52c23b11eb039b14f21`](https://github.com/oliverjessner/OpenEmperor/commit/9bac35b5626834b42d35d52c23b11eb039b14f21) |15 Modellzeilen/getrennte Hausfamilien |
| OE26 | [`bcba05334494b4f6f5afd7d4c0d2d76a13469caa`](https://github.com/oliverjessner/OpenEmperor/commit/bcba05334494b4f6f5afd7d4c0d2d76a13469caa) | Atomische4×4-Elitegeometrie |
| OE27 | [`eb40408a57038d775dfa79d3c8e9f1bbf189f80d`](https://github.com/oliverjessner/OpenEmperor/commit/eb40408a57038d775dfa79d3c8e9f1bbf189f80d) | Difficulty/Steuer/Wohnmission |
| OE28 | [`15dd6da9183617f39f711df2b2ab1598f6595cb9`](https://github.com/oliverjessner/OpenEmperor/commit/15dd6da9183617f39f711df2b2ab1598f6595cb9) | Hausinspector/Overlays |
| OE29 | [`64cbfcb7c13a4b162cfb8bab118e45ef99b09d3d`](https://github.com/oliverjessner/OpenEmperor/commit/64cbfcb7c13a4b162cfb8bab118e45ef99b09d3d) | Belegte Hausfamilienbilder |
| OE30 | [`324fc8fd765c4c4485d7d69e97e39530a504b95d`](https://github.com/oliverjessner/OpenEmperor/commit/324fc8fd765c4c4485d7d69e97e39530a504b95d) | Normaler bezahlter Menüstart |
| OE31 | [`d6226ed2de02413f4d462d0b968fe13b6f3d9f69`](https://github.com/oliverjessner/OpenEmperor/commit/d6226ed2de02413f4d462d0b968fe13b6f3d9f69) | Feindliche Saves/Recovery |
| OE32 | [`2fbf27066ad277b15bcc4bbfc176bd3befb95c5e`](https://github.com/oliverjessner/OpenEmperor/commit/2fbf27066ad277b15bcc4bbfc176bd3befb95c5e) | Deterministischer Langlauf/Skalierung |

OE33 selbst ist der lokale Commit dieses Berichts mit dem Betreff
`test(original): [33] endvergleich-gegen-den-fork-und-release-entscheidung`.
Seine SHA wird nach dem Commit in der Übergabeantwort genannt und ist mit
`git log -1 --format='%H %s'` reproduzierbar; ein Commit kann seine eigene SHA
nicht in seinen Inhalt aufnehmen.

Die eigenen Zusatzeinträge der Quellenklärung sind mit
`git log --format='%H %s' --grep='\[12\]\|\[13\]'` reproduzierbar.
Die genaue Status-/Fehlversuchsgeschichte steht weiterhin in
[PROGRESS](../codex-original-economy/PROGRESS.md) und
[DECISIONS](../codex-original-economy/DECISIONS.md).

## Kurzer Spieler-Testplan

Dieser Plan ist eine **offene menschliche Checkliste**, keine behauptete
Human Acceptance. Technische Abnahme wartet nicht darauf. Start aus dem
Repository mit `.local/original-economy/oe33/release/build/openemperor`,
Originaldaten auswählen, **Original economy**
→ **Cities/Xia.map** → **Normal** → **Paid four-house starter** → **Start sandbox**.
Die vollständige Bedienung steht in [Originalwirtschaft spielen](paid-starter.md).

1. Im pausierten Tick0 Funds/gebuchte Käufe, vier leere Häuser und leere
   Bestände ansehen. Mit Space starten; Zuzug und tatsächliche Kurierfahrt
   beobachten. Die vier Häuser allein erfüllen das Sechs-Haus-Ziel nicht.
2. Haus mit5/Select wählen, PageUp/PageDown durch State/Needs/Goods/Visits.
   Fehlenden Bedarf von echten Mengen und vorhandener Quelle von erfolgtem
   Besuch unterscheiden. Echten2→3→4→5-Aufstieg beobachten.
3. Beide Ancestorquellen regulär pausieren und weiterlaufen lassen. Erst nach
   ausbleibenden Besuchen und tatsächlichen Fehlereignissen eine Rückstufung
   erwarten; anschließend wieder aufnehmen und neue reale Ankünfte prüfen.
4. Zoom1×/2×/4×, Vordergrund, Hausanker, Kurierfüße und Auswahl prüfen.
   Benannte Fallbacks bewusst als offene Grafikstellen erfassen. F4/F6
   vergleichen Präsentationen; sie erzeugen keine Versorgung.
5. MitF5 speichern, Tick notieren, weiterlaufen lassen, F9 laden. Derselbe
   gespeicherte Zustand muss pausiert erscheinen. Anwendung neu starten und
   denselben expliziten Save über Load save öffnen; Quelle/Difficulty müssen
   passen. Optional Recoveryhistorie separat prüfen.

Der Spieler kann Beobachtungen zu konkretem Tick, Building-ID, Zoom und
fehlender Grafik festhalten. Ein Screenshot vorhandener Häuser ist kein
Beleg, dass obere Originalstufen bezahlt erreicht oder alle Objekte korrekt
zugeordnet wurden.

## Übergabe und Releaseentscheidung

- [x] Historischen und aktuellen Fork getrennt gepinnt, aktuellen Sourcebaum
  und13 gemeinsame Kerndateien geprüft; keine Windowsintegration.
- [x] Featurematrix mit vier verbindlichen Statusklassen, Quellen und echten
  Testnamen erstellt; keine vollständige Originalparität behauptet.
- [x] Alle technischen Taskcommits OE00–32 und resultierende Architektur
  nachvollziehbar benannt.
- [x] Aktuelle OE33-Vollbuilds/CTests sowie Legacy-/Policy-/Save-Refreeze
  vollständig beendet und im finalen Laufbericht gebunden.
- [ ] Alle neu benötigten Originalobjektbilder und passenden Actorbilder
  tatsächlich zugeordnet/integriert; Fallbacks zählen nicht als Erfüllung.
- [ ] Bezahlte Kette mit originalen Modelldaten durch sämtliche verpflichtenden
  oberen Wohnstufen demonstriert; reine15×5-Matrizen reichen dafür nicht.
- [ ] Offene originale Venue-, Mauer-, Migration-/Lifecycle- und
  Krankheitsthemen durch eigene belegte Verträge geschlossen, soweit für
  spätere Paritätsbehauptungen benötigt.
- [ ] Native menschliche visuelle Abnahme separat durchgeführt.

**Keine Tags, kein Release und kein automatischer finaler Merge.** Ein
abgeschlossener technischer Vergleich kann als eigener OE33-Taskcommit
festgehalten werden, während das Gesamtziel und Gate-F-Gesamtbewertung wegen
fehlender verpflichtender Funktionen **PARTIAL** bleiben. Fehlende Funktionen
werden dadurch weder erledigt noch in eine hypothetische spätere Version
umgedeutet. Innerhalb des nummerierten Pakets folgt kein OE34; offene Arbeit
braucht konkrete, getrennt belegte Folgeaufträge und dieselben
Original-/Authoritygrenzen.

[fork-compare]: https://github.com/amscova/OpenEmperor/compare/d861d2912337c09751cb68196f85d5a30752726e...992f90ce3071f66cff3d82b31aaa7550b018adb5
[fork-world]: https://github.com/amscova/OpenEmperor/blob/992f90ce3071f66cff3d82b31aaa7550b018adb5/src/simulation/World.h#L222-L239
[fork-goods]: https://github.com/amscova/OpenEmperor/blob/992f90ce3071f66cff3d82b31aaa7550b018adb5/src/simulation/WorldOriginalGoods.cpp#L33-L46
[fork-save]: https://github.com/amscova/OpenEmperor/blob/992f90ce3071f66cff3d82b31aaa7550b018adb5/src/persistence/SandboxSave.cpp#L823-L886
[fork-main]: https://github.com/amscova/OpenEmperor/blob/992f90ce3071f66cff3d82b31aaa7550b018adb5/src/app/main.cpp#L442-L461
