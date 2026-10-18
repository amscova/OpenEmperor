# Bezahlte Luxusketten und Akademiematerial in Revision24

Vor Implementierung am2026-10-17 nach OE23
95967cec1e691a50e9a771a59940deda2230807b festgelegter Vertrag. Der
Gesamtauftrag und Prompt24 erlauben ausdrücklich versionierte, offen benannte
Rezeptannahmen. Aktiv ausschließlich Rule10/Schema26/policy3, Revision24;
Revision23/25-Crossloads werden verweigert. Veröffentlichte Regeln1–9 und
Schemas1–25, reine Goods-/House-/Servicekerne und deren46Owner/26Actor-Limits
bleiben unverändert. Neue globale aktive Grenzen82Bodies/58Courier; maximal
zwei je neuer Producerart, eine separat bezahlte passende Pflanze je Orchard.

Der getrennte [Originalquellennachweis](../reverse/original-luxury-source-contract.md)
bindet Factory, Warenselektoren, körperliche Ernte, Mehrfachinputs und die
ursprüngliche Paper27/Wood10-Auswahl; er behauptet keinen aktuellen Gameplay-,
Grafik- oder Human-PASS.

## Tatsächliche Objekte und Materialgrenzen

Neun neue bezahlte Producer38LoggingShed,39BronzeSmelter,42BronzewareMaker,
44LacquerwareMaker,45PaperMaker,47Weaver,237TeaCuringShed,238LacquerRefinery,
239SilkwormShed verwenden ausschließlich vorbereitete tatsächliche Modell-
Kosten/Employees je Difficulty. Smelter39 ist3×3, übrige2×2. Pflanzen26TeaBush,
27LacquerTree,28MulberryTree sind eigene bezahlte1×1-Körper und eindeutig an
237/238/239 gebunden. Keine Plant-/Stock-/Workergrants, kein Terrainbrush.

Die vorhandenen sechs Hauswaren werden vollständig aktiv. Tea, Silk,
Bronzeware und Lacquerware laufen über eigene tatsächliche Outputcarts zum
Warehouse und über vorhandene Goods-Transportpfade bis Market/Haus/Verbrauch.
Wood/Bronze/RawSilk/Lacquer/Paper sind fünf separat typisierte Auxmaterialien,
keine siebte Hausware. Clay bleibt im bestehenden Raw-Clay-System und gelangt
per vorhandenem Claycourier auch in Bronzeware.input_clay; kein Aux-Clayspiegel.
Hemp bleibt im Sechserledger und erreicht PaperMaker durch den vorhandenen
Warehouse-Goodscourier. PaperMaker verbraucht Hemp, ausdrücklich kein Holz.

Feste Inputs: SmelterWood+internesErz→Bronze; BronzewareBronze+RawClay→Bronzeware;
LacquerwareLacquer+Wood→Lacquerware; WeaverRawSilk→Silk; PaperMakerHemp→Paper.
TeaCuring+TeaBush→Tea; LacquerRefinery+LacquerTree→Lacquer;
SilkwormShed+MulberryTree→RawSilk. Keine vereinfachte kostenlose Zweitzutat.

## Ausdrücklich authored Ernte- und Rezeptregeln

Managed Forestry: Logging38 erzeugt1Wood je80 tatsächlich produktiven Ticks.
Managed Ore: Smelter39 gewinnt1 nicht handelbares internesErz je80 produktiven
Ticks bis8; ein weiterer80er-Rezeptzyklus braucht1Erz und1 echtes Wood für1Bronze.
Die drei kleinen Plantagen erzeugen mit ihrer tatsächlich gekauften passenden
unmittelbar benachbarten Pflanze1Output je80 produktiven Ticks. Alle übrigen
Rezepte brauchen je1 jeder tatsächlichen Zutat und erzeugen1Output je80
produktiven Ticks. Voraussetzung sind tatsächliche volle Beschäftigung,
Betrieb, kein Brand, normale Straßenverbindung, Inputs und Ausgangsraum.

Alle Inputs werden am erfolgreichen Ende gemeinsam abgebucht und der Output
atomar gutgeschrieben. Progress ist allein Arbeitsfortschritt, keine zweite
WIP-Masse; Pause/fehlenderInput/Outputvoll erzeugen keine Ware. Puffer8 je
Producerfach, Warehouse32 je Fach entsprechend aktuellem Vertrag; reservierter
Rückkehrraum bleibt zusätzlich geschützt. Originale Ökologie, Resourcebyte255,
Suchtag100002, Kalender-/Fruchtbarkeits-/Regionaleffekte, Baumwahl und
Grower-/Woodcutter-Actors werden damit nicht als rekonstruiert behauptet.
Diese Abweichungen sind in UI/Report als authored kenntlich, Originalsprites
werden separat in29 integriert und tatsächlich angesehen.

## Echte Akademieversorgung

Paper27 erreicht die gekaufte Academy219 ausschließlich durch tatsächlichen
Aux-Courierempfang.0Paper sperrt neuen Confucian-Dispatch und dessen
Verfügbarkeitsdiagnose. Jeder erfolgreiche neue Dispatch verbraucht1Paper,
einschließlich einer später am brennenden Ziel verweigerten Wirkung.
Bereits begonnene Touren behalten normale Sicherheit. Der Paper-Sink bindet
exakt den ohnehin gespeicherten monotonischen Confucian-Trip-Highwater.
PaperOnly und1proTour sind authored, da die originale globale27/10-Umschaltung
und der originale Personal-Verbrauchsaccumulator noch nicht als Laufzeit
rekonstruiert sind. Das belegte anfängliche Elite-only-Gate folgt in25;
bis dahin bleibt die gemeinsame Common-House-Berechtigung ausdrücklich authored.

## Bilanzen, Transport und endliche Persistenz

Jeder Producer besitzt genau einen Outputcart: eigene Rollen für vier
Sechser-Luxusoutputs und fünf Auxoutputs. Jedes Warehouse besitzt zusätzlich
einen Auxsupplier; keine gemischten oder neu startenden Sequenceauthoritäten.
SharedNavigator/Edge-Sicherheit/Routen bleiben alleiniger Transportpfad.
Die sechs Waren und kompatiblen Ziel-IDs bilden eine feste, höchstens120
Paare große Dispatchrotation. Sie beginnt nach dem letzten tatsächlichen
Target/Good-Paar; aktuelle Bestände, Aufnahmeberechtigung und vorbereitete
Route werden für jedes Paar separat geprüft. Es entsteht kein zusätzlicher
gespeicherter Cursor. Ein durchgehend lieferbares Paar wird spätestens nach
einem vollständigen Umlauf erfolgreicher Dispatches gewählt; intermittierende
Produktion, Brände und knappe Versorgung erlauben keine universelle
Starvationfreiheitsbehauptung. Diese Auswahl ist authored, keine originale
Walker-Zielpolitik.
Native Clayfahrten werden vollständig mit monotonischen letztenTrips erfasst,
auch Potteryfahrten dazwischen. Pause/Brand stoppen Produktion und neuen
Dispatch; bereits geladene native Clayfahrten behalten die alte Zustellung
in den reservierten Input, anschließend kann ein Brand keine Recipe auslösen.

Aux: höchstens20Owner/12Actors und feste5er-Arrays. ProOwner gilt
Output+received+returned=stock+departed+consumed; global tatsächliche
Produktion=Stock+Cargo+Recipe-/Academysinks. Die bisherigen sechs Goods behalten
ihre eine Inventoryauthority; GlobalConsumed=HouseHistory+PaperHempRecipeSink.
Housinghistory bleibt ausschließlich Hausverbrauch. Raw-Clay-Bilanz ergänzt
allein Bronzeware.input_clay und dessen tatsächlichen Recipe-Claysink.
Jeder Counter, Receipt, Typ, Owner, Transfer, Körper, Pfad, Zeit und aktuelle
Bewegung bleibt streng gebunden; ein bloß global passender Summenrest genügt nicht.

Produktive neue Bodies, Pflanzen und Materialempfänger erhalten in24 keine
neue Abrissfreigabe. Alle entsprechenden Abrissversuche müssen atomar mit
unverändertem vollständigem Zustand verweigert werden. Der bereits erlaubte
bezahlte neutrale Wellabriss bleibt verfügbar und wird während realer
Warenproduktion samt unveränderten Warenbilanzen geprüft. Keine Tombstone-
Historie wird stillschweigend gelöscht, kein Source neu als Warenquelle gratis
angelegt. Erweiterter produktiver Abriss bräuchte einen eigenen vollständigen
Herkunfts-/Entfernungsvertrag.

## Originaler OR-Bedarf und Abnahme

Der alte Prompt24-Verdacht ist durch unabhängige Originalbytes widerlegt:
Bronzeware/Lacquerware erfüllen den Bedarf als OR, vorhandene Mengen beider
Waren werden jedoch original getrennt verbraucht. Der eingefrorene GoodsRules-
Kern bleibt exakt; Tests decken keine/eine/jeandere/beide Waren ab.

ProWare: real bezahlte Source/Inputs/Plants→Outputcart→Warehouse→Market→House
→Verbrauch, Negativfall ohne Zutat/Ware, reale Paperankunft und Servicegate,
volle Reservierungen/Rückkehrraum, zwei Warehouses, Pause/Brand/Roadrepair,
Überlauf und strikter Save/F5F9/20k-Prozessvergleich. Echte Plain-Luxusbedarfe0
werden nicht erfunden: Need1-Kopien ändern klar benannt nur jeweilige Bedarfs-
felder. Aufstiegsinputs werden vor dem Verbrauch desselben Ticks geprüft.
Keine höhere Haustypauthority vor25. Geprüfter privater Szenarioplan:
43Bodies/107Roads/3159Funds. Alle neun Producer samt Hemp/Clay/zweiWarehouses/
Market benötigen143 tatsächliche Worker, inklusive aller Zusatzdienste185.
Zwanzig echte Shelter liefern140; für den Bootstrap bleiben Paper/Academy/Food/
Ancestor regulär pausiert und Well ist aktiv:137Worker. Weitere Versorgung
benötigt tatsächlichen Well/Hut-Zuzug, keinen kostenlosen Kapazitätswechsel.
Der anfängliche182er-Plan las Market1 aus dem Originalmodell, obwohl die
bestehende World ausdrücklich Rules::market_workers4 verwendet; vor dem
ersten Build wurde dieser Plan auf die tatsächliche Worldanforderung korrigiert.
Der Bootstrap wartet bei pausierten Zusatzbetrieben auf die vollständige
installierte spätere Workeranforderung. Der1×1-Market liegt unmittelbar an
der bezahlten Straße, nicht hinter dem Farmfeld. Frühe tatsächliche Läufe
deckten beide Szenenfehler und eine bei sechs Waren unvollständige Rotation
auf; diese negativen Artefakte bleiben erhalten.
Diese frühen Befunde ersetzen keine Endabnahme. Der aktuelle Teststatus steht
in [PROGRESS](../codex-original-economy/PROGRESS.md). Eigene3Konfigurations-
Prüfung und Taskcommit; GateD erst27. Auf ausdrücklichen Nutzerwunsch folgt
nach OE24 eine Pause; OE25 wird erst nach neuem Fortsetzungsauftrag begonnen.
