# Kulturquellen und tatsächlicher Dienstzugang in Revision23

Vor Implementierung nach OE-22 1e7e2715704bd7b5def483d4924292c1d4d41181
festgelegter Vertrag. Aktive Rule10/Schema26/policy3 verwendet ausdrücklich
Revision23 und verweigert Crossloads. Regeln1–9/Schemas1–25 behalten ihre
bisherigen Grenzen, Authority und Bytes.

## Bezahlte Quellen, einzelne reale Effekte

Music School211 kostet im Normalmodell50 und braucht8 Worker bei2×2 Zellen;
Acrobat School21260/9 bei3×3, Drama School21370/10 bei3×3. Das originale
Music-Modelllabel hat exakt zwei Leerzeichen: „Music  School“. Maximal zwei
bezahlte Körper je Typ besitzen je einen cargo-freien Walker. Quellenmodell,
Footprint, reale Beschäftigte, Betrieb, Fire, Straße und tatsächliche Ankunft
bleiben bindend. Jeder Walker verwendet den vorhandenen Navigator und seinen
privaten deterministischen Zielcursor. Bau, Nähe, Dispatch und Rückkehr
erzeugen keinen zusätzlichen Visit.

Der belegte individuelle DirectService-Callback setzt ausschließlich die
passende House-Kategorie Music2/Acrobat3/Drama4 auf96; keine gemeinsame
AcrobatAndDrama-Aktion. Medizin96 und vier Religionen40 bleiben unabhängig.
Phase35 zählt alle neun vorhandenen Timer herunter. HouseState bleibt die
einzige aktive Timerauthority, aktuelle und historische Herkunft bleiben gebunden.

## Experimentelle gezielte Besucher gegenüber Originalausbildung

Die originalen Schulen entsenden Performer32/33/34 zu physischen passenden
Venues71/75. Deren32er-Performerzeiten und eigener Actor36 sind eine separate
Authority; der beobachtete Schulcallback kann während Bewegung einzelne
Hauseffekte auslösen. Die hier gewählte minimale gezielte School→House-
Navigation ist ausdrücklich authored und in UI/Report als experimentell
kenntlich. Sie stellt keine vollständige ursprüngliche Venue-/Training-Kette
her und erzeugt keine freien Bühnen, Performer oder Trainingsbestände.
Originale Worldtick-Spawnfrequenz, Kalender und zufällige Wanderziele bleiben
offen. Der pure Servicekern bleibt unverändert.

## Verfügbarkeit ist keine Lieferung

Eine neue reine Diagnose liest für ein Haus/eine Kategorie nur vorhandene
Provider, tatsächlichen Betrieb/Fire/Worker und bereits vorbereitete Routen.
Sie unterscheidet vorhandene, aktuell verfügbare und erreichbare Quellen,
tatsächlich auf dieses Haus zufahrende Walker sowie den wirklichen Timer und
letzte erfolgreiche Ankunft. Ein verfügbarer erreichbarer Provider verspricht
keinen künftigen Besuch. Die Diagnose hat keine Saveauthority, keinen eigenen
Countdown, keine Mutation und führt keine BFS/Datei-/Assetvorbereitung aus.

## Endliche Authority und Prüfung

Aktive Limits58Körper/38Courier und18Provider/18letzteTrips/180Witnesses.
Alte46/26 bleiben unverändert. Neue Objectwerte24/25/26, Commands31/32/33,
Courierrollen20/21/22 und Provider6/7/8 bezeichnen Music211/Acrobat212/Drama213.
Housing behält die sechs bisherigen Slots0/1/5/6/7/8 und hängt2/3/4 an;
alle neun Slots sind exakt, keine implizite Enumindex-Umdeutung. Tatsächliche
Quellgröße gilt weiterhin auch für notwendige Mindestfahrzeiten im Restore.

Pro Kategorie sind bezahlte Ankunft, NoRoad, wirkliche96→95/1→0-Schritte,
Save/Restart und konkurrierende Zielrotation zu prüfen; Medizin/Religion/
Wasser dürfen nicht überschrieben werden. Betriebliche und Fire-Gates sowie
in-flight-Pause und negative historische Manipulationen bleiben streng.
Reale Plain-Bedarfe für Kultur sind0; Need1-Prädikate werden ausschließlich in
klar benannten privaten Modellkopien bei tatsächlich ankommenden übrigen
Bedarfen getestet. Hauptszene:10Häuser,44Roads, sämtliche neun Services und
Wasser,1383Funds Ausgabe;70 tatsächlich zugezogene Worker vor Disposition,
43 nach bewusstem Pausieren der drei Kulturquellen. Keine Snapshot-/Stock-/
Workerinjektion. Sechs neue20k-Prozessfälle folgen den30 vorhandenen.
Ein Review korrigiert außerdem die bislang fehlende Registrierung der drei
Revision22-Religionskörper im normalen Brandrisikopfad. Alle sechs neuen
Religions-/Kulturtypen verwenden in Revision23 denselben vorhandenen Firepfad;
echte Quellenbrände und Housebrände werden gesondert geprüft. Dies ist kein
rückwirkender Nachweis für Revision22. Originalsprite-Abnahme29 und Native/Human
bleiben getrennt.


## Tatsächlich ausgeführte Abnahme

Drei aktuelle inkrementelle Werror-Builds und je53 ausgewählte CTests bestehen,
zusätzlich36 separate20k-Prozessfortsetzungen mit72 bytegleichen vollständigen
Save-/Reportpaaren. Eigene Tests liegen in `OriginalCultureWorldTests.cpp`,
`OriginalCultureSaveTests.cpp` und dem erweiterten `OriginalProductionViewTests.cpp`.
Die Hausentwicklungskontrollen prüfen echte historische gleichzeitige Bedarfe
vor dem Verbrauch desselben Ticks, fehlende/abgelaufene Kultur dagegen bei
aktuell tatsächlich erfüllten übrigen Bedarfen. Reale Quellen- und Hausbrände
sind getrennt geprüft. Der finale Software-SDL-Screenshot wurde geöffnet;
alle neun Timer und die Nahrungssituation bleiben sichtbar. Diese endliche
technische Abnahme ist keine vollständige originale Kulturpipeline, kein
Cleanbuild-Phasengate und keine native oder menschliche Originalgrafikabnahme.
Ausführliche Befehle, Artefaktbindungen und verbleibende Grenzen stehen im
[Fortschrittsprotokoll](../codex-original-economy/PROGRESS.md).
