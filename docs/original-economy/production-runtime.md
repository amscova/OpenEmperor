# Aktive Originalwirtschaft: Produktionsursprung

Der [Gesamtauftrag](completion-contract.md) verwendet einen ausdrücklich
gewählten City-v16-Rule10-/Schema26-Entwicklerstand bei unveränderter policy3.
Entwicklungsrevision14 integrierte den tatsächlichen bezahlten Ceramics-Ursprung;
Revision15 ergänzt den physischen Markettransport bis zum Haus.
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
- Typisierte Ceramics: produced + imported = consumed + demolished + stock
  + cargo. Reservierungen zählen nicht zusätzlich als Masse.

Revision15 erlaubt Ceramics über Warehouse, Market und tatsächliche
Hausankunft. Andere Originalwaren, Food, Konsum, Services und Steuern folgen
ihren Aufgaben. Die frühen originalen Hausbedarfe bleiben unverändert0;
ein ausdrücklich authored physischer Hauspuffer8 ermöglicht Provisionierung
vor Evolution und Verbrauch.

## Market, Reservierungen und Rückkehr

Warehouse→Market und Market→House benutzen vorhandene Courierrollen und
denselben Navigator. Beladung reduziert den physischen Quellbestand; erst
die wirkliche Ankunft erhöht den Zielbestand. Typisiertes Cargo lebt allein
im Warenledger. Die älteren Mengenfelder der Bewegungsactoren bleiben0;
Anzeige und geladene Walker lesen die tatsächliche Ledger-Menge.

Jeder der höchstens sechs typisierten Actoren behält genau seine letzte
Transferzeile als Sequenz-Highwater. Neue Fahrten ersetzen terminale Zeilen
erst nach erfolgreicher Reservierung; veraltete und doppelte Arrival-Handles
werden verweigert. Ausgehendes Cargo hält physische Rückkehrkapazität am
Quellowner frei, ohne zusätzlich als Masse gezählt zu werden. Damit passen
auch Rückgaben, während neue Lieferungen eintreffen.

Pausieren eines Owners bricht seine beladene Ausfahrt ab. Eine bereits
begonnene Kante wird beendet, danach führt derselbe Navigator zurück.
Disconnect verhindert eine Lieferung; bezahlte Reparatur ermöglicht ihre
Fortsetzung. Häuser, Lager und Märkte mit gebundener Authority bleiben vor
unsicherem Abriss geschützt. Der laufende Stand konsumiert keine Ceramics.

## Persistenz und technische Prüfung

Schema26 speichert die strikte Entwicklungsrevision, Bewohner- und
Übergabehistorie, genau eine typisierte Stockzeile je Warehouse/Market und die
vorhandene vollständige Infrastruktur-/Actor-/Hausauthority. Kapazitäten,
Readiness und vorbereitete Modelltabellen werden nicht gespeichert. Aktive
Payloads werden nicht durch eingefrorene Neutralzustandsvalidatoren früherer
Versionen restauriert. Eigene Modell-/Map-/Masken-/Policybindung und vollständige
Conservation-/Finanz-/Referenz-/Routeprüfungen gehen der Veröffentlichung voran.

Der automatisierte [Prozessrunner](../../tools/test_original_production.py)
verwendet eine hashidentische private Kopie des echten Modells und authored
24×20-Geographie. Vier bezahlte Häuser ziehen dort28 Bewohner an. Revision15
bezahlt zusätzlich den Market:958 Funds Bauausgaben,11042 Rest. Bei Tick706
transportiert sein Courier tatsächlich eine typisierte Einheit; bei Tick815
besitzt ein Haus nach der Ankunft seine erste Ceramics-Einheit. Ein separater
Pausenstand erhält dasselbe beladene Cargo als Rücktransport ohne Hausgutschrift.
Mid-Market- und positiver Hausstand werden jeweils in einem neuen OS-Prozess
über10000 Ticks, einen bezahlten Straßenbau und weitere10000 Ticks fortgesetzt;
vollständige SaveDocuments, rohe Bytes und Bilanzen müssen übereinstimmen.
Kein Snapshot und kein Bestand wird zum Herstellen des Erfolgs injiziert.

Tatsächliche Konfigurations-, Negativ-, SDL-, Prozess- und Legacybelege stehen
im [Fortschrittsregister](../codex-original-economy/PROGRESS.md). Software-SDL
und die tatsächlich geöffnete Aufnahme sind keine menschliche Spielabnahme.

Der separate Entwicklerstart bleibt möglich:

```sh
sh tools/play_original_economy.sh
```

Er beginnt pausiert und verwendet ausschließlich den eigenen Rule10-Savepfad.
Straßen, vier angeschlossene Häuser, Clay, Pottery, Warehouse und Market kaufen und
dann laufen lassen; F5/F9 speichert/lädt denselben Entwicklungsstand. Die
Defaultsave ist `rule10-revision15.json`; frühere Entwicklungsrevisionen
werden atomar verweigert. Die Originalgrafik-/Objektkomposition ist Bestandteil der folgenden
Präsentationsaufgaben und wird durch diesen Produktionsnachweis nicht als
bereits abgeschlossen ausgegeben.
