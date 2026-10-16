# Aktive Originalwirtschaft: Produktionsursprung

Der [Gesamtauftrag](completion-contract.md) verwendet einen ausdrücklich
gewählten City-v16-Rule10-/Schema26-Entwicklerstand bei unveränderter policy3.
Die erste aktive Entwicklungsrevision14 integriert den tatsächlichen
bezahlten Ceramics-Ursprung. Veröffentlichte Regeln1–9/Schemas1–25 behalten
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

Revision14 erlaubt keine positiven anderen Originalwaren, Hauslieferungen,
Market-, Food-, Konsum-, Service- oder Steuerpfade. Der Warehouse-Bestand ist
die einzige positive Ceramics-Authority dieser Stufe. Diese späteren Aufgaben
werden anschließend der Reihe nach implementiert.

## Persistenz und technische Prüfung

Schema26 speichert die strikte Entwicklungsrevision, Bewohner- und
Übergabehistorie, genau eine typisierte Stockzeile je Warehouse und die
vorhandene vollständige Infrastruktur-/Actor-/Hausauthority. Kapazitäten,
Readiness und vorbereitete Modelltabellen werden nicht gespeichert. Aktive
Payloads werden nicht durch eingefrorene Neutralzustandsvalidatoren früherer
Versionen restauriert. Eigene Modell-/Map-/Masken-/Policybindung und vollständige
Conservation-/Finanz-/Referenz-/Routeprüfungen gehen der Veröffentlichung voran.

Der automatisierte [Prozessrunner](../../tools/test_original_production.py)
verwendet eine hashidentische private Kopie des echten Modells und authored
24×20-Geographie. Vier bezahlte Häuser ziehen dort28 Bewohner an. Der erste
echte Warehouse-Übergang erfolgt im geprüften Lauf bei Tick675. Mid-Pottery-
und positiver Warehouse-Stand werden jeweils in einem neuen OS-Prozess über
10000 Ticks, einen bezahlten Straßenbau und weitere10000 Ticks fortgesetzt;
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
Straßen, vier angeschlossene Häuser, Clay, Pottery und Warehouse kaufen und
dann laufen lassen; F5/F9 speichert/lädt denselben Entwicklungsstand. Die
Originalgrafik-/Objektkomposition ist gesonderter Bestandteil der folgenden
Präsentationsaufgaben und wird durch diesen Produktionsnachweis nicht als
bereits abgeschlossen ausgegeben.
