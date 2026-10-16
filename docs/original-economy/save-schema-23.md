# OE-09: Schema23 und getrennte OriginalHouse-Daten

Basis: `4d5f9fb88fe9ca0a106c1aea30cf4080230f390b`, Stand2026-10-16.
Schema23 gehört ausschließlich zu City-v16 Rule7 mit unveränderter policy3.
Es gibt weder einen automatischen Upgradepfad noch einen neuen Menüeintrag.
Gewöhnliche Rule7-Konstruktoren, `restore_save` und Recovery-Historien bleiben
gesperrt. OE-09 erlaubte ausschließlich Tick0. Die spätere
[OE-10-Erweiterung](scheduler.md) aktiviert im selben ausdrücklich gewählten
Entwicklerpfad ausschließlich den versionierten Scheduler: vorhandene Ticks
sind seine einzige Authority; zusätzliche Clock-Keys und eine Migration werden
nicht eingeführt. Dieses Dokument trennt den aktuellen Savevertrag vom
historischen OE-09-Nachweis.

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
Kapazität und Ziele bleiben0. Die House-Cyclephase bleibt reserviert0 und ist
keine Kopie der globalen Schedulerposition. Die Zeitzuteilung besitzt der
[OE-10-Vertrag](scheduler.md); später gebaute Container speichern ihren
tatsächlichen `placed_tick <= ticks`.

Die ausdrückliche Entwicklerfactory erlaubt für den Persistenznachweis zusätzlich
zu bestehender Infrastruktur den regulären bezahlten `PlaceHousehold`-Befehl.
Er nutzt unverändert80 Funds, vorhandene2×2-Geometrie und BuildingIDs, erzeugt
aber ausschließlich einen leeren Container und den zugeordneten Vacant-Datensatz.
Das ist authored Infrastruktur für einen wirklichen, nicht injizierten Roundtrip;
kein originales Kaufpreis-/Footprint-/Wohnbevölkerungsversprechen. Andere
Wirtschaftscommands bleiben verweigert. Seit OE-10 schreibt ein Rule7-World-Tick
ausschließlich den Scheduler fort; sämtliche Wirtschafts- und House-Zustände
bleiben inaktiv.

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

## Historischer OE-09-F5/F9- und Prozessnachweis

Nur das neue SDL-Testtarget kompiliert
`OPENEMPEROR_ORIGINAL_ECONOMY_SAVE_FIXTURE=1`. Dadurch ist eine benannte Viewfactory
verfügbar, die vollständige policy3 und Source-Identität vorbereitet und dieselben
produktiven `handle_event`→Save/Load→`save_now`/`load_now`-Methoden benutzt.
Die normale Anwendung besitzt keinen Einstieg in diesen Adapter. In OE-09 blieb
er pausiert; Step, Continue, Geschwindigkeit und Reset eröffneten keine Laufzeit.
Der leere Container verwendet einen ausdrücklich bezeichneten Entwicklerfallback,
keine Legacy-Level-Zuordnung oder originale Hausgrafik.

Der Test kauft ein Haus und Straßen, sendet SDL-F5, kauft eine weitere Straße
und stellt mit SDL-F9 den vollständigen Zustand wieder her. Manipulierte Saves
und geänderte Map-/Modelquellen lassen die aktive World unverändert. Ein zweiter
OS-Prozess lädt denselben Save und schreibt mit tatsächlichem F5 erneut;
vollständige SaveDocuments und kanonische Dateibytes müssen übereinstimmen.
In OE-09 ließen 120 Updates und Zeitsteuerung Tick0 unverändert. Alle Testquellen
sind unabhängig authored; dies ist scripted SDL, keine native menschliche Spielabnahme.

Die SDL-freie Suite ergänzt158 verweigerte Manipulationen, einschließlich
Referenzen, Domains, verschachtelter Keys, Quellhashes, Finance/Revision,
Old/New-Payloads, Recovery-Sperren und atomischer Schreibfehler.
Tatsächliche Konfigurationen, Befehle, Ergebnisse und der separate unveränderte
Legacy-Replay stehen im [Fortschritt](../codex-original-economy/PROGRESS.md).
Gate B bleibt bis OE-12 offen; spielbare Originalwirtschaft folgt später.

## OE-10-Fortschreibung ohne neues Savepayload

Schema23 behält seine kanonischen Keys. Mappingversion1 ist über Rule7/Schema23
fest gebunden; Phase, Schritt, Monat und elapsed-Jahr werden ausschließlich aus
gespeicherten abgeschlossenen Ticks abgeleitet. Der Tick0-Vertrag bleibt erhalten.
`UINT64_MAX-1` ist als terminaler Zustand speicher- und restaurierbar; ein weiterer
Schritt und `UINT64_MAX` werden abgelehnt. OriginalHouse-Daten bleiben unverändert
inaktiv, einschließlich reservierter House-Cyclephase0.

Der Testadapter startet und lädt weiterhin pausiert, erlaubt jetzt ausdrücklich
Pause/Continue, Step und Speed1/2/4 für den reinen Scheduler. Reset bleibt
gesperrt. Tests für F5/F9 vor/auf/nach Zyklusgrenzen, exakten nächsten Dispatch,
Bruchteilakkumulator, Source-Refusals und separate OS-Prozesse ergänzen den
historischen Tick0-Nachweis. Ausgeführte Resultate stehen im
[Fortschritt](../codex-original-economy/PROGRESS.md); Zeitbasis, Originalbelege
und weiterhin offene Wirtschaftsgrenzen erklärt der [Schedulervertrag](scheduler.md).
