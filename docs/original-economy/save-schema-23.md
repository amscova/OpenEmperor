# OE-09: Schema23 und getrennte OriginalHouse-Daten

Basis: `4d5f9fb88fe9ca0a106c1aea30cf4080230f390b`, Stand2026-10-16.
Schema23 gehört ausschließlich zu City-v16 Rule7 mit unveränderter policy3.
Es gibt weder einen automatischen Upgradepfad noch einen neuen Menüeintrag.
Gewöhnliche Rule7-Konstruktoren, `restore_save`, Recovery-Historien und Ticks
bleiben gesperrt. Dies ist eine ausdrücklich begrenzte Entwicklungsstufe.

## Gespeicherte Authority

`WorldSnapshot::original_economy` ist bei bisherigen Regeln abwesend und bei7
vorhanden. Sein Inhalt steht ausschließlich in Schema23 unter
`world.original_economy`; alte Writer erhalten kein zusätzliches Feld.

| Teil | Gespeicherter Zustand und Grenze |
| --- | --- |
| `model_source` | Formatversion1, lowercase SHA256, Bytezahl1..256KiB, Difficulty0..4; keine Pfade oder Modeltabellen |
| `houses[].building_id` | Positiver vorhandener BuildingId, streng aufsteigend, eindeutig und1:1 zum platzierten bezahlten Household-Container; vorhandener World-NextID bleibt maßgeblich |
| `house.type` | Getrennter Original-HouseType; syntaktische Domain2..17, derzeit ausschließlich Vacant2 restaurierbar |
| Bedürfnisbeobachtungen | Water bool, Food Quality Byte, neun getrennte Service-/Religionstimer; keine gespeicherte Kopie der24 Modellbedarfswerte |
| Bewohner/Bestände | Signed-word-Domain0..32767 für Bewohner und sechs getrennte Warenbestände; keine neuen Goods-/Courier-Rollen |
| Weitere Originalfelder | Desirability−128..127, Devolve-Counter0..255, Originalstatusbyte, Access bool und gespeicherte Cyclephase |

Die breiteren Typdomänen reservieren spätere Zustände, aktivieren sie aber nicht.
Der gemeinsame World-Validator lässt aktuell ausschließlich den vollständigen
Nullzustand eines Vacant-Containers zu: keine Bewohner, Bestände, Bedürfnisse,
Servicezeit, Evolution, Access oder fortgeschrittene Phase. Auch sämtliche alten
Bevölkerungs-/Bedarfs-/Steuer-/Produktionsfelder bleiben null. OriginalHouse ist
keine Umrechnung des alten House-Level0..2; die alten Levelabfragen verweigern7,
Kapazität und Ziele bleiben0. Die Zeitzuteilung folgt erst OE-10.

Die ausdrückliche Entwicklerfactory erlaubt für den Persistenznachweis zusätzlich
zu bestehender Infrastruktur den regulären bezahlten `PlaceHousehold`-Befehl.
Er nutzt unverändert80 Funds, vorhandene2×2-Geometrie und BuildingIDs, erzeugt
aber ausschließlich einen leeren Container und den zugeordneten Vacant-Datensatz.
Das ist authored Infrastruktur für einen wirklichen, nicht injizierten Roundtrip;
kein originales Kaufpreis-/Footprint-/Wohnbevölkerungsversprechen. Andere
Wirtschaftscommands und jeder World-Tick werden weiterhin verweigert.

## Quelle, striktes Lesen und atomisches Restore

`original_model_source` liest die Modelldatei über den vorhandenen schreibgeschützten
Parser und gibt ausschließlich ihre Identität zurück. Diese Identität wird bei
Speichern und ausdrücklich gewähltem Entwicklungs-Restore gegen die echte Datei
geprüft. Fehlende, beschädigte oder geänderte Dateien werden abgelehnt. Ein
ungebundener OE-08-Infrastrukturtest kann weiterhin existieren, aber nicht als
Schema23 gespeichert werden. Das ist noch keine World-Bindung eines vorbereiteten
`OriginalRulesModel`; diese gehört zu OE-11.

Zusätzlich bleiben Map-SHA, Buildable-Masken-SHA und der unveränderte gerahmte
policy3-Fingerprint verbindlich. Restore konstruiert erst einen Kandidaten und
validiert dessen IDs, Zuordnungen, Footprints, Bridge-/Ramp-Geometrie, Kosten,
Revisionen, Funds und Transportgraph mit den vorhandenen Prüfungen. Eine
verweigerte Operation veröffentlicht weder einen Teilzustand noch freie Güter.

Schema23 verlangt alle kanonischen Felder und Typen, auch tief verschachtelt.
Unbekannte, fehlende und doppelte Keys, falsche Enums und Integer-/Float-/Bool-
Verwechslungen werden abgelehnt. Die vorhandenen Grenzen8MiB und JSON-Tiefe32
bleiben bestehen. Der JSON-Adapter verwendet für die unveränderte Infrastruktur
nur eine lokale Schema22-Darstellungskopie; keine World wird auf Rule6 umgestellt
und kein alter Save migriert. Bisherige gültige Writerbytes und die historische
Lesetoleranz für fremde Legacy-Keys bleiben erhalten; neue OriginalPayloads in
alten Schemas werden ausdrücklich abgewiesen.

Vor jeder atomischen Dateiersetzung werden Quellen und World vollständig erneut
geprüft. Fehler vor Schreiben, nach Teiltempdatei oder vor Rename erhalten die
bestehende Zieldatei. Die strenge Pfad-/Symlink-/Originaldaten-Schreibsperre bleibt
unverändert. Recovery wird noch nicht für die Entwicklergeneration geöffnet.

## Tatsächlicher F5/F9- und Prozessnachweis

Nur das neue SDL-Testtarget kompiliert
`OPENEMPEROR_ORIGINAL_ECONOMY_SAVE_FIXTURE=1`. Dadurch ist eine benannte Viewfactory
verfügbar, die vollständige policy3 und Source-Identität vorbereitet und dieselben
produktiven `handle_event`→Save/Load→`save_now`/`load_now`-Methoden benutzt.
Die normale Anwendung besitzt keinen Einstieg in diesen Adapter. Er bleibt
pausiert; Step, Continue, Geschwindigkeit und Reset eröffnen keine Laufzeit.
Der leere Container verwendet einen ausdrücklich bezeichneten Entwicklerfallback,
keine Legacy-Level-Zuordnung oder originale Hausgrafik.

Der Test kauft ein Haus und Straßen, sendet SDL-F5, kauft eine weitere Straße
und stellt mit SDL-F9 den vollständigen Zustand wieder her. Manipulierte Saves
und geänderte Map-/Modelquellen lassen die aktive World unverändert. Ein zweiter
OS-Prozess lädt denselben Save und schreibt mit tatsächlichem F5 erneut;
vollständige SaveDocuments und kanonische Dateibytes müssen übereinstimmen.
120 Updates und Zeitsteuerung lassen Tick0 unverändert. Alle Testquellen sind
unabhängig authored; dies ist scripted SDL, keine native menschliche Spielabnahme.

Die SDL-freie Suite ergänzt158 verweigerte Manipulationen, einschließlich
Referenzen, Domains, verschachtelter Keys, Quellhashes, Finance/Revision,
Old/New-Payloads, Recovery-Sperren und atomischer Schreibfehler.
Tatsächliche Konfigurationen, Befehle, Ergebnisse und der separate unveränderte
Legacy-Replay stehen im [Fortschritt](../codex-original-economy/PROGRESS.md).
Gate B bleibt bis OE-12 offen; spielbare Originalwirtschaft folgt später.
