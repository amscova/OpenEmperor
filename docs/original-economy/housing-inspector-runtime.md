# Original House Inspector und Diagnose-Overlays (OE-28)

Vor Implementierung festgelegter Vertrag, 2026-10-20, Basis
`eb40408a57038d775dfa79d3c8e9f1bbf189f80d`.

OE-28 verändert ausschließlich die Darstellung der ausdrücklich gewählten
Originalproduktion. Rule10/Schema26/Policy3/Entwicklungsrevision27, alle älteren
Regeln und Saves, Defaults, Geld, Waren, Wege, Hausentwicklung und Footprints
bleiben unverändert. Es gibt keinen neuen gespeicherten Zustand und keine
zweite HouseSimulation im View. Originaldateien bleiben read-only; originale
Hausgrafiken gehören weiterhin zu OE-29.

## Inspector

Nur gekaufte Originalhäuser erhalten vier sitzungslokale Ansichten:
`State`, `Needs`, `Goods`, `Visits`. Maus und PageUp/PageDown bedienen die
Ansichten innerhalb der bestehenden Fokus-/Modalgrenzen. Der bisherige
City16-Inspector und alle Quellen-/Industrie-Inspektoren bleiben erhalten.
Die Darstellung verwendet die bestehenden const-World-Diagnosen und gespeicherten
Produktionsbelege, keine eigenen Entwicklungsentscheidungen oder Routen.

State erklärt tatsächlichen Typ, Familie, reale Körpergröße, Bewohner,
Kapazität, DES-Grenzen, aktuellen Entwicklungsblocker und nächste planmäßige
Prüfung. Needs trennt aktuellen und nächsten Modellbedarf. Binäre Versorgung
erscheint als tatsächlicher vorhandener Timer/YES/NO; ein Prozentwert ist nur
als relative Erfüllung eines numerischen Bedarfs beschriftet, niemals als
erfundene originale gespeicherte Versorgungsquote. Qualität bleibt der echte
HouseState-Wert mit ihrem vorhandenen Beleg.

Goods trennt ausdrücklich Stadtproduktion, Reservierung, Fracht, Lieferung
und Konsum. Historisch gekürzte Courier-/Transferlisten sind keine kumulative
Lieferhistorie. Für die sechs Hauswaren ist die tatsächlich belegte
Hauslieferung exakt Hausbestand plus kumulierter Hauskonsum; Food verwendet
seine eigenen erhaltenen Eingangs-/Abgangs-/Konsumzähler. Jede Zahl benennt
ihren Haus- oder Stadtbezug. Visits trennt tatsächlichen Timer und letzten
belegten Besuch von vorhandenen, verfügbaren, erreichbar gecachten und
unterwegs befindlichen Quellen. Quellenexistenz erfüllt niemals ein Bedürfnis;
zukünftige Ankunft und Aufstieg sind keine Zusagen. Timerwerte bleiben rohe
Countdowns, keine unbelegte Umrechnung in garantierte Restspielzeit.

## Overlays und reine Darstellung

Ein eigener sitzungslokaler Modus `Off / Supply / DES / Risk` ist per Maus und
F10 erreichbar, zunächst Off. Er benutzt ausschließlich die höchstens20
gekauften Originalhäuser und ihre tatsächlichen2×2-/4×4-Claims, höchstens320
Zellen. Er berücksichtigt die bestehenden Höhen-/Kamera-/Cullingpfade und
zeigt Farbe plus Zeichen/Legende. Keine neue228²-Schleife, Mapvorbereitung,
Worldkopie, Mutation, Tick, BFS, Dateiarbeit, Dekodierung oder Texturarbeit pro
Frame. DES und Risiko sind momentane Diagnosen, keine Zukunftsprognose.
Feuer unterbricht Versorgung/Einzug; daraus wird kein nicht existierendes
generelles Evolutionsverbot abgeleitet.

## Technische Abnahme

Erforderlich sind gezielte CTests in Debug, Release und ASan/UBSan,
unveränderte alte Inspector-/Input-/Saveverträge, tatsächliche SDL-Maus- und
Tastatureingaben einschließlich Fokus/Modal/Resize/HiDPI/Textgrenzen,
F2/F3/F6/F8, F5/F9 sowie100 reine UI-Frames mit unveränderten World-/Datei-/
Asset-/Routen-Zählern. Softwarebilder werden bei1×/2×/4× erzeugt und geöffnet;
bei4× sind sichtbare Ownerzellen/Marker maßgeblich, da ein ganzer4×4-Körper
nicht in jedes Kartenfenster passt. Tatsächliche Versorgung und ein vorhandener
noch nicht besuchender Provider sind getrennte Positiv-/Negativfälle.

Der nächste Phasen-Gate E gehört zu OE-30; OE-27s Gate D wird durch reine UI
nicht neu behauptet. Scripted SDL und geöffnete Softwarebilder sind keine
native oder menschliche Spielabnahme. Nach belegtem Abschluss folgt genau ein
lokaler OE-28-Commit; OE-29 und Remoteveröffentlichung sind nicht beauftragt.
