# Food-Lieferungen in Entwicklungsrevision18

Dieser vor der Implementierung festgelegte Vertrag setzt OE-17 `b30718f` fort.
Nur ausdrücklich gewählte Rule10/Schema26/policy3 wechselt auf Revision18.
Ältere Entwicklungsstände werden strikt verweigert; veröffentlichte Regeln1–9,
Schemas1–25, sechs Originalwaren und der reine Food-Blend bleiben unverändert.
Der [Gesamtauftrag](completion-contract.md) erlaubt die notwendige bezahlte
Integration. Vollständige Originalmechanik wird damit nicht behauptet.

## Quelle und Grenzen

Ein eigener kaufbarer Food-Farmhouse-Körper verwendet Modell193 und3×3.
Wheat195 und Millet196 sind getrennte1×1-Objekte; jeweils genau ein separat
gekauftes orthogonal angrenzendes Feld wird einem ausdrücklich gewählten Farm
zugeordnet. Maximal zwei Farms; Kosten und Employees aus dem gebundenen Modell,
Felder ohne Employees. Keine Wiederverwendung der alten1×1-Farmidentität und
keine Gratisfelder. Die belegten Originalobjekt-/Grafikidentitäten werden in
OE-29 komponiert; bis dahin ist die Darstellung als authored erkennbar.

Die Zeitskala und Mengen sind authored:80 tatsächlich besetzte produktive Ticks
liefern eine Einheit. Farm8, Markt16, Haus8 und Cart4 begrenzen das gesamte
Volumen beider Feldfrüchte gemeinsam. Gewöhnliche Land-, Schutz-, Footprint-,
Straßen-, Workforce-, Feuer- und Operationbedingungen gelten. Farmcourier fährt
zum bezahlten Markt, ein eigener Marketcourier zum berechtigten bewohnten Haus.
Höchstens vier Markets behalten die bestehende Grenze. Somit26 Foodbesitzer
und sechs Foodcouriers/Trips, ohne die alten Collectionbudgets zu erhöhen.
Nur vorhandener gemeinsamer Navigator und echte Ankunft; kein Transportbonus.

Die ursprüngliche Erzeugung des Source-Scores ist nicht vollständig belegt.
Daher gelten ausdrücklich authored Wheat20/Millet80, bei gemischtem tatsächlichem
Cargo der ganzzahlige volumengewichtete Wert. Das Originalmodell verlangt über
15 Hauszeilen und fünf Schwierigkeiten maximal70; die Oberklasse80 erlaubt die
spätere Prüfung dieser Anforderungen, ohne originale Crop-Scores zu erfinden.
Rice-Paddy-Nachbarschaften und ursprüngliche Diversitätsberechnung bleiben offen.

## Sole authority und Qualität

`OriginalProductionState.food` enthält getrennte zwei-Crop-Bestände für genau
jeden bezahlten Farm, Markt und jedes Haus. Keine siebte Originalware und kein
Spiegel in alten Building-/Courier-Foodfeldern. Food-Trips speichern höchstens
die letzte Sequenz pro tatsächlichem Courier; `loaded-delivered-returned` ist
die einzige Cargoauthority. Zielreservation und Rückkehrraum zählen mit.

`FoodBatchQuality` wird aus den tatsächlichen akzeptierten Crop-Teilmengen
ermittelt. Ausschließlich positive reale Hausankunft ruft unverändertes
`original::blend_food_quality(previous_quality,batch_quality,accepted_volume,
previous_stock)` auf. Nur HouseState hält die aktuelle Qualität. Nullmenge,
voller Empfänger, Abbruch und Rücktransport verbessern sie nicht. Kein
konstanter20-Wert aus bloßem Stock. Verlustbehaftete Originalrundung bedeutet,
dass kein angeblich konserviertes additives quality-times-volume-Ledger gilt.
Stattdessen sind Crop-Mengen konserviert und jeder Qualitätsübergang bezeugt.

Je Besitzer sind received/departed/returned/consumed reine Historienzähler:
`harvest+received+returned=stock+departed+consumed`. Der letzte Hausarrival-
Witness nennt echte Transfersequenz, Tick, akzeptierte Mengen, consumed_before
und previous_quality. Vorbestand lässt sich aus received minus accepted minus
consumed_before rekonstruieren; der Originalblend muss die gespeicherte aktuelle
Qualität ergeben, solange nachfolgender Verbrauch nicht vollständig geleert hat.
Erste Ankunft beginnt zwingend bei Stock0/Qualität0. Kompaktierte Vorgänger
liefern nur notwendige Plausibilitätsgrenzen, keine vollständige Besuchschronik.

Food-Verbrauch ist ein ausdrücklich authored, von den sechs Modellwaren
getrennter Sink: bei denselben berechtigten0/7-Grenzen nach Ankunft und Zuzug
eine Einheit pro bewohntem Haus, Wheat vor Millet. Leerer Stock erhöht einen
Missed-Demand-Zähler; positive Leerung setzt Qualität0. Damit erfüllt eine alte
Lieferung bei dauerhaft leerem Haus die spätere Food-Voraussetzung nicht für
immer. Kein erfundener Original-Foodbedarf, keine Vorab-Evolution, Steuern oder
Bevölkerungsverluste.

## Restore und Abnahme

Strict JSON, Identitäten, Kauf-/Feldbindung, Größen und sichere Koordinaten,
produktive Altersobergrenzen, tatsächliche Courierrollen/-phasen/-pfade,
stock+incoming+returnroom, checked Historiengleichungen und globale Crop-
Conservation werden geprüft. Housequalität benötigt gültigen Arrival-Witness
und mögliche bezahlte Transportherkunft, auch nach komplettem Verbrauch.
Das gebundene Modell validiert originale Kosten/Employees. Revision17/18 werden
niemals implizit ineinander geladen.

Pflichtbelege: beide real bezahlten Feldfrüchte, leere und gemischte Märkte,
mehrere Qualitätsklassen, Originalformel mit unabhängigen Vektoren, kleine
akzeptierte Teilmengen, volle Ziele, Missed Demand, Unterbrechung/Rückkehr,
Reload/20k-Replay und separate OS-Prozessfortsetzung. Alte Save- und Policy-
Literale bleiben exakt. Originaldaten sind read-only; private Aufnahmen werden
geöffnet. Native und menschliche Abnahme bleiben von technischen Tests getrennt.
