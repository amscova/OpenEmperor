# Bezahlter Ancestor-Besuch in Entwicklungsrevision20

Vor Implementierung nach OE-19 `ba116b4` festgelegter Vertrag. Nur explizite
Rule10/Schema26/policy3 wechselt auf Revision20;19 und20 werden gegenseitig
verweigert. Alte Regeln1–9/Schemas1–25 bleiben eingefroren. Finanzierung12000,
leere Anfangsbestände, regulärer Zuzug, Goods/Food und die medizinischen
[Besuche aus Revision19](services-runtime.md) bleiben erhalten.

## Quelle und tatsächlicher Effekt

Ein eigener2×2-Körper `OriginalAncestorShrine214` kostet den vorbereiteten
Modellpreis von214 und braucht dessen Employees (Normal40/4). Höchstens zwei
solche Quellen, je ein eigener cargo-freier `OriginalAncestor`-Courier. Kauf
gewährt weder Bewohner noch Worker, Waren oder Religion. Alle Bau-/Road-/
Bridge-/Ramp-/Fire-/Staffing- und begonnene-Kanten-Grenzen bleiben gewöhnlich.
Ein Dispatcher benutzt dieselben gecachten Routen und privaten Zielcursor.

Erst tatsächliche Outbound-Ankunft ruft den unveränderten DirectReligion-Kern
mit Ancestor auf. Status1/3, Access und positive Bewohner oder echter Elitetyp
sind dessen belegte Voraussetzungen. Das originale zusätzliche Sourcebyte
`+0x174` hat bisher unbekannte Sessionproduzenten. Revision20 wählt ausdrücklich
eine feste **authored `elite_only=false`-Capability**, keinen aus dem Namen
abgeleiteten Originalfakt und keinen frei speicherbaren Aktivierungsschalter.
Die reale World besitzt derzeit ausschließlich regulär besetzte Shelter;
Elitefälle werden nicht durch positive Snapshot-Injektion erzeugt.

Erfolgreiche Ankunft setzt nur Ancestor auf40, ohne Addition. Pause oder
Quellenbrand nach Ausfahrt lässt die bereits gewählte Tour wie in Revision19
enden; ein brennendes Ziel bekommt keinen neuen Besuch. Dispatch, Rückkehr,
Idle, Nähe, Inspector und Spawnfälligkeit haben keinen Religions-Effekt.
Originale Spawnfrequenz und vollständige Wanderroutine werden nicht behauptet.

## Zeit und strikte Persistenz

Die vorhandene Provider-/Trip-/Witness-Authority erweitert sich auf6/6/60:
höchstens zwei je207/208/214 und20 Häuser mal drei Kategorien. HouseState
bleibt die einzige Timerauthority. Provider-Typ2 gehört nur214; Witness-Rolle5
ist nur Ancestor, neben bisherigen Rollen0/1. Kategorien2/3/4/6/7/8 bleiben
unversorgt. Keine zweite Uhr, keine Deadline oder freie Coverage.

Mit D(t)=0 für t<36, sonst1+(t−36)/51 gilt nach tatsächlicher Ankunft a:
`max(40−(D(ticks)−D(a−1)),0)`. Medizin bleibt96. Ein Besuch auf der
Phase35-Grenze kann40 setzen und im selben Tick39 werden. Alle16 Schritte,
1→0, erneutes40 und Arrival ohne gleichzeitigen Countdown werden geprüft.
Disposition, Ziel, Sequenz, Herkunft, Mindestfahrzeit und aktiver Bewegungs-
fortschritt erben die strikten Bindungen des medizinischen Vertrags.

Restore verlangt vollständige bezahlte Körper-/Courier-/Provider-Identität,
reale Modellkosten, sortierte eindeutige Kategorien und zeitlich mögliche
aktuelle sowie kompaktierte Ankünfte. Die bestehende stabile Shelterpopulation
macht aktuelle Besuchseignung noch bindbar; spätere Evolution braucht ihren
eigenen historischen Kontextvertrag.

## Nachweis und Bedienung

Originalbelege: Typ214→Religionindex0→House own-info+0x0d=40;
Factory5aae70/5aaec8→5ab0d0, Vtable7bc0b4; DirectReligion5ab580;
Countdown517280 über Phase35. EXE/Modell bleiben read-only. Private frische
Byteprüfungen stehen unter `.local/original-economy/oe20/research/`.

Y wählt den neuen Bau außerhalb bestehender Modals. Im ausdrücklich aktiven
UI ersetzt dessen Button den bislang deaktivierten generischen Farm-Button;
alte Layouts und das vorhandene Budgetmodal-Y bleiben unverändert. Maus und
Tastatur verwenden dieselbe Action. Der Inspector zeigt Food, Medizin und
Ancestor gemeinsam; Darstellung erzeugt keine Besuche.

Pflichtnachweise: bezahlte40er-Ankunft, kein Zugang ohne Quelle/Worker/Route,
Pause/Brand, Disconnect/Wait/Repair, voller Verfall/Wiederbesuch, strikter
feindlicher Restore, F5/F9 und neue20k-Prozesse sowie unveränderte Legacybytes.
Die belegten Originalkörper2232 und Walkingfamilie989 bleiben OE-29;
diagnostische Darstellung zählt hier nicht als Originalsprite-Abnahme.


## Tatsächliche technische Abnahme

Revision20 besteht3×123 relevante CTests in Debug/Release/ASan+UBSan,
18 getrennte20k-Fortsetzungen und feindliche Save-/Bewegungszeitprüfungen.
Der reale volle46-Körper-/26-Actor-Kaufzustand erlaubt Straßenreparatur;
weitere Schreine werden atomar verweigert. Die geöffnete Software-SDL-Aufnahme
zeigt Ancestor39/40 neben beiden medizinischen Timern, tatsächlichen Waren,
Food-Qualität und Food-Fehlversuchen;100 ruhende Frames bleiben mutationsfrei.
Alle alten Save-/Snapshot-/Policybytes bleiben im frischen Vollvergleich exakt.
Befehle, Grenzen und erhaltene negative Versuche stehen im
[Fortschrittsprotokoll](../codex-original-economy/PROGRESS.md).
Originalgrafik sowie native/human-Abnahme bleiben gesondert offen.
