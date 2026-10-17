# Wasser und echte frühe Hausentwicklung in Revision21

Vor Implementierung nach OE-20 festgelegter Vertrag für den ausdrücklich
gewählten Rule10/Schema26/policy3-Nachfolger. Entwicklungsrevision21 wird
gegen20 strikt getrennt; Regeln1–9 und Schemas1–25 bleiben eingefroren.

## Bezahltes Wasser

Ein eigener2×2-Körper OriginalWaterWell72 kostet den vorbereiteten Normalpreis
25 und benötigt vier tatsächliche Worker. Maximal zwei Körper mit je eigenem
cargo-freiem OriginalWater-Courier benutzen den gemeinsamen Navigator und
privaten Zielcursor. Bau/Nähe/Dispatch/Idle bewirken kein Wasser. Erst eine
echte berechtigte Hausankunft setzt den normalen Roh-Timer96. Alternative
Spezialwasserbedingungen sind unbekannt und bleiben0. Die vorhandene
neutrale Influence60-Quelle wird nicht zu einer Wasserquelle umgedeutet.

Phase28 materialisiert HouseState.water aus dem Roh-Timer; Phase35 zählt
herunter. Nach Ankunft a ist der Roh-Timer max(96−(D(t)−D(a−1)),0),
D(t)=0 für t<36, sonst1+(t−36)/51. E28(t)=0 für t<29, sonst
t−((t−29)%51). Aktueller Besuch und letzter Phase28-Besuch werden getrennt
gebunden, ohne zweite Uhr oder frei gespeicherte Coverage. Arrival geschieht
im World-Adapter vor beiden Phasen. Die vollständige originale Wander- und
Spawnroutine ist nicht rekonstruiert; der gezielte Dispatcher ist authored.

## Produktive Evolution und Bevölkerung

Nach Arrivals und Wasser-/Servicephasen läuft auf Phase39 der unveränderte
pure HouseEvolution-Kern vor dem Verbrauch auf den Selektoren0/7. Vacant
bleibt leer; erster regulärer Zuzug aktiviert Shelter. Die frühe authored
Grenze ist Plain Cottage: ein darüber hinausgehendes reines Ergebnis wird
auf Typ5 begrenzt, dessen korrekt zurückgesetzter Counter bleibt erhalten.
Alle drei Kapazitäten werden vorbereitet und als positive signed-word-Werte
validiert. Census und weiterer Zuzug verwenden ihre jeweiligen Original-
Rowselektoren für den aktuellen Typ. Zuzug setzt einen entwickelten Typ
nicht zurück. Keine unbelegte sofortige Eviction wird erfunden: bei
Rückentwicklung bleiben vorhandene Bewohner auch oberhalb der aktuellen
Kapazität; weitere Immigration wartet, bis wieder Platz besteht.

Ein permanenter first_admission_tick bindet echte120+40k-Gelegenheiten nach
dem bezahlten Hauskauf. HouseState bleibt alleinige Populationsauthority;
Building.population bleibt0, global residents_admitted exakt live.
Originalkapazitäten Shelter7/Hut14/Plain22 werden nicht als Grant benutzt.

## Kompakte historische Belege

Jedes Haus hat genau drei Verbrauchszeilen (Typ3/4/5) mit Ereigniszahl und
sechs kumulativen Sinks; ihre Summen entsprechen dem bestehenden Ledger.
Die letzte Phase39-Eingabe und die ersten erfolgreichen Shelter→Hut- und
Hut→Plain-Eingaben binden reale Service-, Wasser-, Food- und Warenherkunft.
Dies sind begrenzte historische Belege, keine zweite aktive Stockauthority
und keine vollständige nachträglich beweisbare Transportchronik. Höhere
Typen brauchen ihre jeweiligen permanenten ersten Aufstiegsbelege.

Für historische Desirability enthält eine globale auf37 begrenzte Liste
die tatsächlichen Ursprünge aller jemals gekauften neutralen Quellen.
Lebende Ursprünge sind exakt gleich dem Körper; entfernte bleiben erhalten.
Für eine Auswertung E zählt placed_tick<E und removed_tick>=E (sofern
entfernt). Die neue Wasserquelle wirkt einmal mit dem vorbereiteten
Well-Modell. Alle Quellen werden in BuildingId-Reihenfolge durch denselben
puren Desirability-Kern ausgewertet. Temporäre Restore-Karten werden nach
Auswertungstick geteilt; pro Tick/Frame findet keine Map-/Filevorbereitung
statt. Historische dynamische Straßenbelegung wird nicht frei erfunden.

## Abnahmegrenzen

Eine aus normalem Kauf und realem Zuzug entstandene Originalmodell-Szene
muss Hut→Plain erreichen: Water1/FoodQuality20/Ancestor1, Hemp/Ceramics0.
Eine zusätzliche vollständige bezahlte Kette belegt auch deren tatsächliche
Warenankünfte. Ohne Wasser, Religion, Quality oder zugänglichen Market gibt
es keinen unberechtigten Aufstieg. Fehlende Hemp/Ceramics-Anforderung und
strengere Desirability werden in explizit authored Modellkopien geprüft.
Der reale Devolutionscounter (Grenze9, Übergang bei zehnter verfehlter
Auswertung), F5/F9, Mid-Courier und getrennte OS-Fortsetzungen sind Pflicht.
Originalsprite-Abnahme und öffentliche Starter-Menüs folgen29/30;
Software-SDL, native Bedienung und menschliche Annahme bleiben getrennt.


## Tatsächliche technische Abnahme

Die Originalmodell-Minimalszene bezahlt708 aus12000 Funds. Echte Wasserankunft
Tick170 führt zu Hut193 und Plain346. Die zehnte negative Auswertung907
entwickelt auf Hut zurück;22 echte Bewohner bleiben erhalten. Eine weitere
bezahlte1948er-Szene zeigt auch gemeinsam tatsächliche Hemp-/Ceramics-/Food-/
Medizin-/Ancestor-/Wasserankünfte. Fehlendes Wasser/Religion/Quality/Market
verhindert den unberechtigten Aufstieg; die zusätzlichen Need1/Des99-Proben
verwenden klar bezeichnete synthetische Modellkopien.

141 relevante Konfigurations-CTests und24 separate20k-Production-Neustarts
bestehen mit strikten Save-/Input-/Bilanzprüfungen. Der spielbare
`sh tools/play_original_economy.sh`-Pfad nutzt ein eigenes21er-Saveziel;
Q kauft den Brunnen, Y den Schrein. Die geöffnete diagnostische SDL-Aufnahme
zeigt tatsächlich Typ5 und seine Versorgung. Originalgrafik, Native/Human und
der vollständige Phasen-Gate D bleiben separat. Ausgeführte Befehle,
Fehlversuche und Fingerprints stehen im
[Fortschrittsprotokoll](../codex-original-economy/PROGRESS.md).

Beide unbekannten originalen globalen EvolutionContext-Sessiongates bleiben
im Adapter ausdrücklich auf den authored Standardwertenfalse; kein
Force-Evolve-Schalter. Der ursprüngliche Immigration-/Abgangslifecycle wird
nicht aus dem reinen Evolutionskern erfunden. Ungerade Population braucht
eine zuvor mögliche ungerade Kapazität, die bei Freischaltung noch über der
damaligen Population lag. Es wird weiterhin keine vollständige historische
Transport-/Operationschronik aus kompakten Belegen behauptet.
