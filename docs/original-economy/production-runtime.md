# Aktive Originalwirtschaft: Produktionsursprung

Der [Gesamtauftrag](completion-contract.md) verwendet einen ausdrücklich
gewählten City-v16-Rule10-/Schema26-Entwicklerstand bei unveränderter policy3.
Entwicklungsrevision14 integrierte den tatsächlichen bezahlten Ceramics-Ursprung;
Revision15 ergänzte den physischen Markettransport bis zum Haus; Revision16
ergänzte die [bezahlte Hemp-Kette](hemp-runtime.md) mit Farm192 und Feld194.
Revision17 ergänzt [phasengenauen Verbrauch](consumption-runtime.md) und eine
abgestimmte Waren-/Zielrotation für laufende Versorgung. Revision18 ergänzt
die [getrennte Food-Kette](food-runtime.md): bezahlter Farmhouse193, Wheat195/
Millet196, tatsächliche Market-/Hausankunft, originale Qualitätsmischung und
ein ausdrücklich authored Food-Sink. Revision19 bindet [getrennte medizinische
Besuche](services-runtime.md) an bezahlte207-/208-Provider und echte Walker-
Ankünfte, einschließlich des unveränderten Phase35-Countdownkerns.
Aktueller Entwicklerstand ist Revision19.
Veröffentlichte Regeln1–9/Schemas1–25 behalten
ihre bisherigen Autoritäten und Bytes; normale Menü-/Recoverywege bleiben
bis zu ihren späteren Aufgaben geschlossen.

## Bewohner und Produktion

Ein leeres gekauftes Haus erhält ausschließlich durch normale laufende
Zuwanderung Bewohner: Mindestalter120 Ticks, danach alle40 Ticks höchstens
zwei, nur mit tatsächlich zulässigem Straßeneingang und ohne laufenden Brand.
Der erste Zuzug aktiviert den Shelter-Typ. Die Kapazität kommt aus der
gebundenen vorbereiteten Modelldatei. Eine ungerade Schlusskapazität erhält
beim letzten Zuzug genau den verbleibenden einen Bewohner. Nicht erreichbare
ungerade Zwischenstände werden beim Restore verweigert.

Die Bewohner in `OriginalHouseState` liefern die deterministisch verteilten
Worker. Das ältere `Building.population` bleibt null. Diese Zuzugs- und
Workerregeln sind authored, keine Behauptung exakter Originalimmigration.
Neue Waren, Bewohner oder Worker können nicht über eine Test-/Grant-API
gesetzt werden. Alle Bauten laufen über reguläre bezahlte Commands.

Clay Pit und Kiln nutzen die vorhandenen Rezepte, Staffing-, Operations-,
Fire- und Transportpfade. Der unverändert herausgezogene gemeinsame
Rezeptschritt wird auch von den alten Regeln benutzt. Fehlendes Personal,
Pause und Fire stoppen tatsächliche Produktion; fehlende Straßen verhindern
die Warenankunft und begrenzen Produktion über volle physische Puffer.

## Eine Umwandlung und drei überprüfte Bilanzen

Vor dem Warehouse bleiben Kiln-Output und Pottery-Courier-Cargo unkonvertierte
Pottery. Erst das tatsächliche erfolgreiche Eintreffen im Warehouse bucht
einmalig über den reinen Transaktionskern typisierte Ceramics. Der alte
Warehouse-Pottery-Slot bleibt null; es gibt keinen gespiegelten Vorrat.

- Extrahierter Clay = physischer Clay einschließlich Rezeptmaterial und
  Clay-Cargo + zweimal abgeschlossene Rezepte.
- Abgeschlossene Pottery = unkonvertierter Output/Cargo + gespeicherte
  tatsächliche Warehouse-Übergaben.
- Typisierte Waren je Kategorie: produced + imported = consumed + demolished + stock
  + cargo. Reservierungen zählen nicht zusätzlich als Masse.
- Tatsächlich geernteter Hemp = gesamte typisierte Hemp-Produktion.

Revision17 erlaubt Ceramics und Hemp über Warehouse, Market, tatsächliche
Hausankunft und modellbasierten Verbrauch. Revision18/19 ergänzen Food und die
zwei medizinischen Dienste; weitere Originalwaren, Dienste und Steuern folgen
ihren Aufgaben. Die frühen originalen Hausbedarfe bleiben unverändert0;
ein ausdrücklich authored physischer Hauspuffer8 ermöglicht Provisionierung
vor Evolution und Verbrauch.

Hemp entsteht nur in einer bezahlten und vollständig besetzten Farm mit separat
gekauftem angrenzendem Feld. Vorbereitete Modellwerte bestimmen Cost und
Employees;80 produktive Ticks pro Einheit, Feldanschluss und Funds-Umrechnung
sind benannte authored Regeln. Die Farm hält8 Hemp, Warehouse und Market
getrennte Bins32/16, das Haus8. Der neue Farm-Courier verwendet denselben
Navigator; Warehouse und Market rotieren beide Waren. Food-/Pottery-Mengen
sind keine Hemp-Authority.

## Market, Reservierungen und Rückkehr

Warehouse→Market und Market→House benutzen vorhandene Courierrollen und
denselben Navigator. Beladung reduziert den physischen Quellbestand; erst
die wirkliche Ankunft erhöht den Zielbestand. Typisiertes Cargo lebt allein
im Warenledger. Die älteren Mengenfelder der Bewegungsactoren bleiben0;
Anzeige und geladene Walker lesen die tatsächliche Ledger-Menge.

Jeder der höchstens acht typisierten Actoren behält genau seine letzte
Transferzeile als Sequenz-Highwater. Neue Fahrten ersetzen terminale Zeilen
erst nach erfolgreicher Reservierung; veraltete und doppelte Arrival-Handles
werden verweigert. Ausgehendes Cargo hält physische Rückkehrkapazität am
Quellowner frei, ohne zusätzlich als Masse gezählt zu werden. Damit passen
auch Rückgaben, während neue Lieferungen eintreffen.

Pausieren eines Owners bricht seine beladene Ausfahrt ab. Eine bereits
begonnene Kante wird beendet, danach führt derselbe Navigator zurück.
Disconnect verhindert eine Lieferung; bezahlte Reparatur ermöglicht ihre
Fortsetzung. Häuser, Lager und Märkte mit gebundener Authority bleiben vor
unsicherem Abriss geschützt. Schritte0/7 konsumieren die aktuelle Modellmenge
nach tatsächlicher Bewegung. Der Original-Shelterbedarf bleibt dabei0.

## Persistenz und technische Prüfung

Schema26 speichert die strikte Entwicklungsrevision, Bewohner- und
Übergabehistorie, sechs Verbrauchszähler je Haus, Farm-/Feld-/Erntezuordnung, genau eine typisierte Stockzeile
je Farm/Warehouse/Market und die
vorhandene vollständige Infrastruktur-/Actor-/Hausauthority. Kapazitäten,
Readiness und vorbereitete Modelltabellen werden nicht gespeichert. Aktive
Payloads werden nicht durch eingefrorene Neutralzustandsvalidatoren früherer
Versionen restauriert. Eigene Modell-/Map-/Masken-/Policybindung und vollständige
Conservation-/Finanz-/Referenz-/Routeprüfungen gehen der Veröffentlichung voran.
Kompaktierte Trips liefern notwendige Ankunftsplausibilität; sie bilden keine
vollständige Besuchschronik ab. Insbesondere können frühere Ceramics-Fahrten
nicht die erste Hemp-Ankunft ersetzen, und jede Farm braucht ihre eigene
tatsächliche Ernte als Ursprung für Stock und Cargo.

Der automatisierte [Prozessrunner](../../tools/test_original_production.py)
verwendet eine hashidentische private Kopie des echten Modells und authored
24×20-Geographie. Sechs bezahlte Häuser ziehen dort42 Bewohner an;34 Worker
besetzen Clay, Kiln, Warehouse, Market und Hemp Farm.1162 Funds werden regulär
ausgegeben,10838 bleiben. Im geprüften Revision17-Lauf hat Haus1 bei Tick904
nach tatsächlichen Ankünften je eine Einheit Hemp und Ceramics. Ein separater Pausenstand erhält beladenen Hemp als
Rücktransport ohne vorzeitige Gutschrift. Mid-Hemp- und positiver Zweigüter-
Hausstand werden jeweils in einem neuen OS-Prozess
über10000 Ticks, einen bezahlten Straßenbau und weitere10000 Ticks fortgesetzt;
vollständige SaveDocuments, rohe Bytes und Bilanzen müssen übereinstimmen.
Der echte Modellpfad verbraucht am Shelter weiterhin0. Eine getrennte private
Modellkopie ändert ausdrücklich nur zwei Bedarfsbytes auf authored1/1; bezahlte
Produktion und Lieferungen belegen dort positiven Verbrauch an Schritt0/7.
Auch beide Zustände vor diesen Grenzen werden in separaten OS-Prozessen je
20000 Ticks exakt fortgesetzt. Das ergibt vier unabhängige Prozessreplays.
Kein Snapshot und kein Bestand wird zum Herstellen des Erfolgs injiziert.

Tatsächliche Konfigurations-, Negativ-, SDL-, Prozess- und Legacybelege stehen
im [Fortschrittsregister](../codex-original-economy/PROGRESS.md). Software-SDL
und die tatsächlich geöffnete Aufnahme sind keine menschliche Spielabnahme.

Der separate Entwicklerstart bleibt möglich:

```sh
sh tools/play_original_economy.sh
```

Er beginnt pausiert und verwendet ausschließlich den eigenen Rule10-Savepfad.
Straßen, sechs angeschlossene Häuser, Clay, Pottery, Warehouse und Market kaufen.
Mit N eine Hemp Farm bauen; die Farm auswählen und mit M ein angrenzendes Feld
separat bezahlen. Danach laufen lassen; F5/F9 speichert/lädt denselben Stand.
Defaultsave ist `rule10-revision17.json`; frühere Entwicklungsrevisionen
werden atomar verweigert. Die Originalgrafik-/Objektkomposition ist Bestandteil der folgenden
Präsentationsaufgaben und wird durch diesen Produktionsnachweis nicht als
bereits abgeschlossen ausgegeben.
