# Elite-Footprints in Revision26

Vor Implementierung am2026-10-19 auf `f4447163c6dac035de45a9b1ce9ba2222e0e7546`
festgelegter OE-26-Vertrag. Der Nutzer beauftragt ausdrücklich OE-26 und hebt
damit die Aufgabenpause für diese Aufgabe auf. Aktiv bleibt ausschließlich
Rule10/Schema26/policy3, jetzt mit Entwicklungsrevision26. Revision25 wird
nicht migriert; veröffentlichte Regeln1–9/Schemas1–25 bleiben unverändert.

## Belegte Größe und begrenzte Transaktion

Der [Originalnachweis](../reverse/original-house-lifecycle-contract.md) belegt
Common2..10 mit Seite2 und Elite11..17 mit Seite4. Der gewöhnliche TypeSetter
erhält diese Seite. Es gibt keinen Beleg für stufenweise wachsende Häuser
innerhalb einer Familie oder Common→Elite-Konversion.

Der bezahlte80Funds-Elite-Plot bleibt zunächst ein leerer authored2×2-Körper.
Unmittelbar vor seinem ersten regulären Einzug wird er bei gleicher BuildingId,
gleichem Ursprung und gleichem Bauzeitpunkt atomisch zu einem4×4-Elitekörper.
Eine angehängte Objektidentität unterscheidet die beiden geometrischen Körper;
kein zweites gespeichertes Größen-/Familienflag und kein zweiter Owner entsteht.
Es gibt keinen direkten Kaufbefehl für den materialisierten Körper.

Erst erfolgreiche Materialisierung und tatsächlicher Einzug erlauben11→13.
Alle späteren Elite12..17-Wechsel erhalten4×4;13→12 ist keine Schrumpfung.
Common bleibt2×2. Bevölkerung, Bestand, Reservierung, Timer, Warenherkunft,
Historie und tatsächlich begonnene Courierpfade bleiben dieselben Authoritys.
Kapazität und Bedarf stammen weiter vom konkreten Haustyp, ohne Flächenfaktor.

## Vollständige Vorbereitung vor jedem Schreibzugriff

Die synchrone Prüfung umfasst alle16 Zellen in fester Reihenfolge: vollständige
Grid-/Policy-Grenzen, eigene bisherige vier Plotzellen, zwölf freie zusätzliche
Zellen, unveränderte Terrain-/Originalstruktur-/Gate-/Straßenberechtigungen,
andere Owner, Bridge-Deck/Bänke, Rampenendpunkte sowie aktuelle und begonnene
Actor-Kanten. Alle16 Zellen müssen dieselbe tatsächliche signed Höhe wie der
Ursprung besitzen. Dieser konservative authored Foundationguard ist kein
behaupteter originaler Platzierungsalgorithmus und ändert keine alte Bauaktion.

Eine normale erlaubte Straße muss am künftigen4×4-Körper einen gültigen Eingang
bilden. Für den noch leeren Plot wird dieser zukünftige Eingang geprüft;
bewohnte Körper und Common behalten ihre gewöhnliche Eingangsprüfung.
Die authored120/40/+2-Zuzugsregel bleibt erhalten. Ein geblockter Versuch
ändert keine Haus-/Geometrie-/Finanz-/ID-/Routenauthority; die übrige Simulation
läuft regulär weiter. Ein späterer regulärer Versuch kann nach echter
Bau-/Straßenreparatur erfolgreich sein. Keine automatische Reparatur, kein Clip.

Ein Erfolg aktualisiert alle16 Object-/Owner-Zellen, den vorhandenen Körper,
Ersteinzug und bestehende Historie gemeinsam. Einmalige Topologieinvalidierung
und gewöhnliche Routencache-Aktualisierung ändern keine laufenden Pfade,
Cargo-/Reservationswerte oder Kantenfortschritte. Die zusätzliche autonome
Topologieänderung wird anhand der realen materialisierten Körper und ihrer
Ersteinzugsbelege begrenzt; sie erfindet keinen Spielerbefehl.

Es gibt keine persistente Vorbereitung, Reservierungslease oder Zwischengröße.
Hausabriss bleibt in dieser Produktionsgeneration gesperrt. Inspektion,
verweigerter Abriss, Reset und Restore können deshalb keine halbe Erweiterung
hinterlassen. Ein echter neuer Abrissvertrag wird nicht implizit eingeführt.

## Darstellung, Restore und Abnahme

Bestehende Footprint-/Frontzellen-/Transportfunktionen verwenden die echte
Körpergröße. Ein ausdrücklich authored4×4-Fallback zeichnet und selektiert
denselben Bereich; keine Originalgrafik wird gestreckt. Ursprung und projizierte
Frontzelle sind OpenEmperor-Geometrie, keine erfundenen Originalspriteanker.
Die2×2-Kaufvorschau kennzeichnet zusätzlich den unverbindlichen Wachstumsraum.
Die Diagnose nennt tatsächliche Größe und konkreten Erweiterungsblocker.
Kein Frame bereitet Map/Assets/World/Navigation vor.

Strict Restore verlangt die Bijektion Plot↔noch kein Ersteinzug und
materialisierter Körper↔wirklicher Ersteinzug. Alle16 Claims werden aus dem
gespeicherten Körper rekonstruiert und mit vollständiger Policy geprüft.
Historische/current DES-, Wasser-, Goods- und Serviceabstände verwenden die
damals bzw. jetzt gültige Größe. Aktuelle Pfade bleiben vollständig validiert.

Pflicht sind unabhängige positive/negative Geometrie- und Restoreprüfungen,
neun Konfliktklassen, Ränder/Höhen, Wait/Repair, gesperrter Abriss während
Vorbereitung, Restart vor/nach Materialisierung, echte bezahlte Einzüge und
Rückentwicklung ohne Schrumpfung. Dazu relevante Debug/Release/ASan+UBSan-
Suites, F5/F9, getrennte20k-Prozessfortsetzungen und tatsächlich geöffnete
Softwareansichten einschließlich äußerer Pickinggrenzen. Alte City-v16-
Footprints und eingefrorene Saves bleiben exakt. Gate D folgt erst nach27;
Original-Haussprites29 und native/human Abnahme bleiben getrennt.


## Tatsächlicher Abschluss26

PASS am 2026-10-19; vollständige Evidence und echte frühere Fehler stehen im
[Fortschrittsregister](../codex-original-economy/PROGRESS.md#oe-26--atomare-große-elite-footprints).
Die bezahlte Originalmodell-/Normal-Szene kostet1729Funds.
Ein wirklich bezahlter Konflikt auf[3,8] hält den tatsächlichen120er-Versuch
bei leer11/2×2. Nach normaler Straßenentfernung ohne Refund bleibt der Plot
bis zum echten160er-Zuzugsereignis leer; dann entstehen16 Claims und Typ13
mit zwei Bewohnern gemeinsam. Spätere echte13→12→13-Zustände behalten4×4.
Kapazitäten und Bedarfe stammen weiter vom gespeicherten Typ, nicht Fläche.

Neun Konfliktklassen, signed Ränder/Höhen, sechs direkte Mature-Policyrefusals,
konkurrierende und gleichzeitige Erweiterungen, echte beladene begonnene
Transporte, Abrissrefusal und vollständiger Restart sind geprüft.
Drei finale77er-Suiten bestehen,51 neue20k-Prozesse vergleichen102
vollständige Save-/Reportpaare. Ein frischer Legacylauf erhält72 ganze
SaveDocuments/Snapshots und1485 Policyfälle; Originaldateien bleiben unverändert.
Acht finale Softwarebilder sind tatsächlich geöffnet, einschließlich Blockade,
4×4 bei0,5×/1×/2× und äußerem Picking. Der natürliche Brand bei2000 enthält
wirkliche Flammenpixel sowie F5/F9, keine injizierte Brandauthority.

Konkrete höhere bezahlte Stände: Haus7: Typ6, Bewohner31, DES6/High14; Haus9: Typ13, Bewohner5, DES10/High50. Kein bezahltes Heavenly wird behauptet.
Die ursprüngliche vollständige Platzierungs-/Resize-/Actor-Lifecycle und
Spriteanker bleiben offen. Original-Haussprites folgen29, Gate D nach27;
native/human Abnahme ist NOT_RUN. Alle finalen Bindungen liegen in
`oe26/validation2/final-validation.json`; diese Abnahme eröffnet OE-27 nicht automatisch.
