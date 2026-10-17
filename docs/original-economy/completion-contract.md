# Ausführungsvertrag für OE-14 bis OE-33

Am 2026-10-17 beauftragt der Nutzer ausdrücklich alle offenen Aufgaben in ihrer
Reihenfolge, ohne auf seine manuellen Playtests zu warten. Am Ende sollen die
echten Sprites und die zugehörigen echten Objekte integriert sein. Der Auftrag
umfasst die erforderlichen neuen Produktions-, Bewohner-, Worker-, Waren-,
Service-, Housing- und Präsentationspfade. Belegte Originalmechanik und
OpenEmperor-eigene Spielregeln bleiben kenntlich.

## Version und Reihenfolge

Veröffentlichte Regeln1–9 und Schemas1–25 bleiben eingefroren. Die aktive
Wirtschaft entsteht als ausdrücklicher **City-v16 Rule10 / Schema26 / policy3**-
Entwicklerstand auf dem Feature-Branch. Normale Defaults bleiben unverändert.
Jeder erledigte OE-Task erhält nach seinen Tests einen eigenen Commit. Die
Abhängigkeiten und Phasen-Gates bleiben verbindlich. Der gemeinsame PR/Merge
folgt nach technischer Endabnahme; keine Tags oder Releases.

Schema26 enthält eine strikt geprüfte `development_revision`, zunächst14.
Eine Aufgabe, die gespeicherte Authority oder deren Bedeutung ändert, erhöht
diese Revision ausdrücklich. Ein früherer Entwicklungsstand wird nicht
stillschweigend als neuer interpretiert; Crossloads werden verweigert. Ein
expliziter Upgrade-Copy-Vertrag wäre gesondert zu implementieren.

## Bezahlter Anfang und tatsächliche Herkunft

Rule10 beginnt mit **12000 Funds**, leeren Warenbeständen und ohne Bewohner.
Dies ist ein benannter authored Finanzierungsvertrag für die vollständigen
bezahlten Ketten vor OE-27, keine Behauptung eines originalen Emperor-Budgets.
Bestehende Baupreise bleiben bis zu einer dokumentierten aufgabenspezifischen
Änderung erhalten. Keine nachträglichen Budgetgutschriften, kostenlosen
Bestände, Snapshot-Injektionen oder Test-Worker.

Ein regulär gekauftes Haus beginnt leer. Frühestens120 Ticks nach Bau lässt
die laufende Simulation alle40 Ticks höchstens zwei Bewohner zu, wenn ein
normaler tatsächlich anschließender Straßeneingang vorhanden ist. Der erste
Zuzug setzt das Haus als authored Bootstrap auf den belegten Shelter-Typ;
die Kapazität stammt aus dem gebundenen vorbereiteten Shelter-Modell.
Bewohner in `OriginalHouseState` sind die einzige Bevölkerungsquelle und
liefern die regulär verteilten Worker. Originale Zuzugszeiten werden damit
nicht behauptet. HouseEvolution folgt ihrem eigenen Auftrag.

OE-14 nutzt die vorhandenen bezahlten Clay-/Pottery-Rezepte mit Staffing,
Operation, Fire und derselben Navigation/Bewegung. Die Umwandlung in typisierte
Ceramics geschieht einmalig beim tatsächlichen Pottery-Courier-Eintreffen im
Warehouse. Vorher bleiben Rezeptoutput und Reisecargo unkonvertierte Pottery.
Danach besitzt allein das typisierte Warenledger den Ceramics-Bestand; der
alte Warehouse-Pottery-Slot bleibt null. Ein gespeicherter Übergabezähler
verbindet beide Bilanzen. Rezepte und Übergabepunkt sind authored Integration.

## Verbindliche Umsetzung

- Eine World, ein Navigator, dieselben Bridge-/Ramp-/Road-Berechtigungen.
- Sechs Originalwaren mit alleinigen HouseState-Hausbeständen; physische
  Nicht-Hausbestände, Reservierungen und Cargo besitzen je eine Authority.
  Produktion, Übergabe, Arrival, Konsum und Abriss erhalten Conservation.
- Services und Religionen brauchen bezahlte Quellen und tatsächliche passende
  Ankünfte; neun Timer bleiben getrennt. Gebäude allein aktivieren keinen Bedarf.
- Hausentwicklung liest tatsächliche Mengen, Qualität, Timer, Bevölkerung und
  Desirability. Geometrieänderungen werden vor Veröffentlichung atomisch gegen
  Straßen, Schutzflächen, Gates, Bridges, Rampen und andere Owner geprüft.
- Echte Originalgrafiken werden aus der lokalen Installation anhand belegter
  Metadaten und Objekt-/Stufenzuordnung geladen. Dekodierte Bilder und private
  Nachweise bleiben unter `.local/`; keine Originalbytes im Git oder Bundle.
  Ein unbekannter Selector darf nicht durch eine ähnliche Grafik als bewiesen
  ausgegeben werden. Ein Fallback erfüllt die neue vollständige Originalsprite-
  Abschlussforderung nicht.
- Technische Tests, Softwarepixel, native Automation und menschliche Abnahme
  bleiben getrennt. Technische Umsetzung wartet nicht auf Human Acceptance;
  ohne menschlichen Test wird diese nicht behauptet.

Ergebnisse stehen in [PROGRESS](../codex-original-economy/PROGRESS.md). Ein echter
Quellen- oder technischer Blocker bleibt sichtbar; weder ein übersprungener Task
noch ein Plan oder ein generiertes Bild ist ein Feature-PASS.


## Quellenbefund aus OE-22 für die Folgeaufgaben

Confucian219 besitzt im normalen originalen Managerinitialzustand ein echtes
Elite-only-Gate und verlangt zusätzlich tatsächlich angeliefertes Material
WORD+0x40. Der frühe22er-Adapter ist dafür ausdrücklich authored. Der spätere
Elite-/Produktions-/Objektanschluss muss beide Bedingungen erneut bearbeiten;
keine gratis Materialbestände oder aus normalen Häusern erfundene Original-
Confucian-Parität. Der [Religionsvertrag](religion-runtime.md) und der
Quellennachweis halten die genaue Aussagegrenze fest.
