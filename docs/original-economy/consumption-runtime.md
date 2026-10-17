# Güterverbrauch in Entwicklungsrevision17

OE-17 setzt den [Gesamtauftrag](completion-contract.md) nach dem getesteten
OE-16-Commit `86cde0c54577a9889632e45432fe7fe18acd7fac` fort. Ausschließlich
Rule10/Schema26 wechselt ausdrücklich auf Entwicklungsrevision17. Frühere
Entwicklungsrevisionen werden verweigert; Regeln1–9/Schemas1–25 bleiben exakt.

## Zeitpunkt und Mengen

Die unveränderte Scheduler-Mappingversion1 beschreibt die nächste Phase aus
den abgeschlossenen Worldticks. Verbrauch folgt Phase39 nur an Schritt0 und7,
also erstmals bei abgeschlossenen Ticks40,397,856,1213. Die Gleichsetzung
einer Phase mit einem Worldtick bleibt authored Zeitintegration.

Nach Produktion, realer Courierbewegung, Fire und Zuzug verarbeitet die World
den berechtigten Dispatch genau einmal. Eine tatsächliche Ankunft derselben
Tickgrenze darf bereits verbraucht werden; neue freie Puffer wirken bei der
nächsten Disposition. Kein alter400-Tick-Demand wird ausgeführt oder verändert.

Der vorhandene pure `consume_goods`-Kern bleibt unverändert: aktuelle House-
Modellzeile, je Ware `min(stock,max(need,0))`, keine Footprintmultiplikation und
kein Verbrauch bei null Bewohnern. Die Bedarfs-ODER-Bedingung für Bronze/Lack
erzeugt keine Verbrauchspriorität: der belegte Kern bucht beide Bestände
getrennt gegen dasselbe Modellfeld. Diese Luxuswaren bleiben in OE-17 real0.

Das gebundene Originalmodell verlangt am aktuell erreichbaren Shelter für
Hemp und Ceramics0. Dieser echte Modellpfad bleibt ein korrekter No-op.
Positive Laufzeit- und Prozessnachweise verwenden ausdrücklich authored
Modellbedarfe, echte gekaufte Quellen und tatsächliche Lieferungen. Es gibt
keine vorgezogene Evolution, Waren-, Bewohner- oder Workerinjektion.

## Gespeicherte Herkunft

`house_consumption` enthält genau eine sortierte Zeile pro gekauftem Haus:
`building_id` und sechs kumulative `consumed`-Zähler. Sie sind Empfangs-/
Verbrauchshistorie; HouseState bleibt der einzige Hausbestand. Ihre Summen
entsprechen exakt dem typisierten Ledgerverbrauch. Kein eigener Zeitcursor
oder zweite Phasenauthority wird gespeichert.

Restore prüft House-IDs, sechs begrenzte Ganzzahlen, unterstützte Waren,
checked Summen, Conservation und die mögliche tatsächliche Lieferherkunft
auch für vollständig konsumierte Bestände. Für einen aktuellen Shelter
begrenzt das bezahlte Hausalter die Zahl möglicher Verbrauchsereignisse.
Mit `F(t)=count(40+816n<=t)+count(397+816n<=t)` sind höchstens
`F(ticks)-F(placed_tick+119)` Ereignisse seit dem frühesten regulären Zuzug
möglich; vorher gilt0. Je Ereignis begrenzen Modellbedarf und Hauspuffer8 die
Menge. Die modellabhängige Prüfung erfolgt mit der gebundenen Quelle. Eine
spätere Hausentwicklung muss diese feste Shelter-Grenze ausdrücklich ersetzen.

Der Kandidat bucht tatsächliche Deltas über das bestehende Ledger, validiert
alle Summen vor Veröffentlichung und kopiert keine World. Die Warenwahl der
Distributorrotation wird mit ihrem gespeicherten Ziel abgestimmt, damit
laufender Verbrauch keine dauerhafte Hemp-/Ceramics-Zielbindung erzeugt.
Navigator, Routenberechtigungen und Framepfad bleiben unverändert.

Pflichtbelege sind beide Grenzen±1, min0/Unterversorgung/Leerstand, laufende
bezahlte Versorgung, koordinierte Save-Fälschungen, F5/F9 und getrennte20k-
Prozessreplays. Resultate stehen im [Fortschritt](../codex-original-economy/PROGRESS.md).
