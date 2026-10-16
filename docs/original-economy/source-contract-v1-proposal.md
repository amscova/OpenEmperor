# OE-12: Quellen-, Bau-, Restore- und Finanzierungsvertrag V1 zur Entscheidung

**ENTWURF / NICHT FREIGEGEBEN.** Stand 2026-10-16, geprüft gegen
`main` `4fd586851e1f019dbeb9b9894e7437fd9b6d9e2c`.
Der Nutzer hat ausdrücklich zuerst diesen konkreten Vertrag beauftragt.
Das erlaubt seine Ausarbeitung, noch keine Implementierung. OE-12 und Gate B
bleiben **BLOCKED**, OE-13 bleibt **TODO**. Auch ein Dokumentationsmerge ist
keine Annahme dieses Vertrags.

## Entscheidungspaket

Empfohlen ist eine kleine, ausdrücklich authored Erweiterung der bisherigen
Rule7-Entwicklung: **City-v16 Rule8 / Schema24 / unveränderte policy3** mit
genau einer Quellendefinition **Well72**, eigenem bezahltem Quellencontainer,
sicherem Abriss und **2000 festen Start-Funds**. Die geprüften Modellfelder
speisen ausschließlich die Desirability. Die 20-Haus-Abnahme bleibt erhalten.
Der City-v13-Vergleich wird ausdrücklich als Vergleich seines unveränderten
Bewertungsalgorithmus auf der bezahlten Szene definiert.

Das sind vier zustimmungspflichtige Änderungen der bisherigen Grenze:

| Entscheidung | Konkreter Vorschlag | Bedeutung |
|---|---|---|
| Version | Rule8 / Schema24, nur explizites Entwickler-Opt-in | Rule7 / Schema23 mit1300 Funds und ohne Quellen bleibt unverändert |
| Quellenidentität | Eigenständiger authored Container mit typisierter Bindung an Modeltyp72 | Keine Behauptung, ein bestehendes World-Well sei als vollständiges Original-Well bewiesen |
| Finanzierung | Fester neuer Startwert2000; House80, Road2, Quelle60; kein Einkommen/Refund | Reguläre Käufe finanzieren20 Häuser und die Mutations-/Reload-Abnahme |
| Vergleich | Unveränderter City-v13-Algorithmus auf derselben bezahlten Szene | Keine fingierte City-v13-World mit dort unzulässigen Wells |

Ohne ausdrückliche Zustimmung bleiben alle heutigen Bau-/Restore-/Budgetgates
und die [strenge Nutzerentscheidung](../codex-original-economy/DECISIONS.md#oe-12-strenge-grenze-und-blockierter-fortgang)
in Kraft. Ein allgemeines „weiter“ gilt nicht als Annahme dieser vier Änderungen.

## Beleggrenze und konkrete Quelle

Die [Originalquellenprüfung](../reverse/original-desirability-source-admission.md),
der [Zustandsbeleg](../reverse/original-desirability-source-state.md) und die
[Factoryprüfung](../reverse/original-desirability-source-factories.md) belegen
für Well72 den untersuchten quadratischen Originalpfad, die direkte
Initialisierung mit Typ72/Seite2 und die getrennten Modellfelder
DES/STP/SZE/RGE. Dabei ist SZE der Wertdelta, keine Gebäudegröße. Die allgemeine
Originalzulassung prüft zusätzlich Status1/3 und das eigene Nullwort+3c.
Diese Originalzustände und ihre vollständige Lifecycle-Semantik werden durch
den vorgeschlagenen Container nicht nachgebildet.

Die neue, ausdrücklich authored Definition lautet:

| Eigenschaft | Vertrag V1 |
|---|---|
| Objekt | Neuer, hinten an `Object` angehängter Wert `OriginalInfluenceSource`; alte Enumwerte bleiben exakt |
| Typisierte Bindung | Ausschließlich `Well72` ↔ verifizierter `ModelBuilding::Well` mit ID72 |
| Identität | Vorhandene monotone `BuildingId`; keine unabhängige Source-ID |
| Physischer Fußabdruck | Exakt2×2 normale, vollständig erlaubte und unbelegte Bauzellen |
| Einflussgeometrie | Dasselbe2×2-Quadrat, keine zweite gespeicherte Geometrie |
| Preis / Limit | Authored60 Funds je Kauf; höchstens vier lebende Quellen |
| Aktivität | Anwesenheit des bezahlten lebenden Containers; kein frei schaltbares Original-Status-/Parentfeld |
| Zahlen | Nur die einmal vorbereiteten Well72-Felder des gebundenen `OriginalRulesModel` und dessen Difficulty |
| Darstellung | Vorhandene vorbereitete Well-Darstellung darf ausdrücklich als Präsentation wiederverwendet werden |
| Sonstige Funktion | Kein Wasser, Unterhalt, Worker, Courier, Goods, Service, Steuern oder House-Upgrade |

Preis, Limit, Runtime-Bindung und Platzierungsvertrag sind **AUTHORED FALLBACK**;
sie werden nicht als rekonstruierter Originalkauf oder vollständige
Originalgebäudeidentität bezeichnet. Die sieben vorhandenen
`functional_model_candidate`-Zuordnungen bleiben `Inferred`/`Unknown`.
Insbesondere entsteht keine automatische Quellenbindung für alte Wells oder
originale Mapgebäude. Die sechs anderen Kandidaten bleiben ausgeschlossen.

Ein normaler `Object::Well` wäre eine ungünstige Wiederverwendung:
[`nearest_water_source_at`](../../src/simulation/WorldWater.cpp) würde ihn
profilbasiert als Wasserquelle lesen; [`maintenance_cost`](../../src/simulation/WorldMaintenance.cpp)
würde in City-v16 Unterhalt2 liefern. Der eigene Objekttyp erhält deshalb
explizite neue Capability-Gates. Seine Wirkung muss in allen Wasser-, Finanz-,
Workforce-, HUD- und Restoreabfragen neutral bleiben. Ein Clock-only-Tick
allein belegt diese Isolation nicht.

## Version und Entwicklerzugang

Rule8 und Schema24 sind am geprüften HEAD unbenutzt. Quellenbau, Abriss und
neues Budget ändern Spielregeln; eine bloße Schemaerhöhung bei gleicher Rule7
wäre unzureichend. Rule7 plus ein zweiter permanenter Vertragsversionsschalter
wäre möglich, wird hier zugunsten einer eindeutigen neuen Regelversion verworfen.
Vor Implementierung ist die Verfügbarkeit erneut zu prüfen.

Rule8 erbt ausschließlich die ausdrücklich aufgezählten Rule7-Fähigkeiten:
bezahlte Straßen, unveränderte Bridge-/Ramp-Verträge, Vacant-Hauscontainer,
Scheduler und unveränderliche Modellbindung. Hinzu kommen nur der hier
beschriebene Quellenkauf, dessen Entfernung und die Einflussabfrage.
Die ursprüngliche OE-12-Formulierung „ausschließlich Rule7“ müsste nach Annahme
ausdrücklich auf den Nachfolger geändert werden; das wird hier nicht still
vollzogen. Weitere OE-13+-Regeln sind nicht Teil dieser Freigabe.
Insbesondere braucht Rule8 einen ausdrücklichen Clock-only-/Modell-Gatepfad;
die bisherigen `version==7`-Guards dürfen den Nachfolger nicht versehentlich
in die Legacy-City-v16-Wirtschaft fallen lassen. Der neue Objectwert und seine
2×2-Geometrie sind ausschließlich für Rule8 zugelassen; unbekannte Typen dürfen
nicht über einen bestehenden1×1- oder sonstigen Defaultpfad angenommen werden.

Zugang ausschließlich über einen eigens benannten Entwickler-Create/Restore-
Pfad und eine ausdrücklich gewählte Entwicklungsszene. Dort muss der normale
Befehlsweg den bezahlten Quellenbau und gezielten Abriss bedienbar machen.
Kein Standardmenüeintrag, automatisches Recovery, implizites Opt-in oder
Migration bestehender Saves. Rule1–7 und Schemas1–23 bleiben einschließlich
der Schema23-Nullzustände und kanonischen Writerbytes exakt.
Diese alten Rules/Schemas verweigern ausdrücklich den neuen Objectwert und
sämtliche Quellen-/Entfernungsrecords; es gibt keinen neutralen Fallback.

## Bau und Entfernung als atomare Befehle

Ein eigener typisierter Quellenbaubefehl akzeptiert nur Well72 und Rule8.
Vor Mutation prüft er gebundenes geeignetes Sessionmodell, Quellenlimit,
Budget, Zähler-/ID-Überlauf sowie alle vier realen Zellen über die bestehenden
MapPermissions-/Owner-/Infrastrukturprüfungen. Geschützte Originalgebäude,
Gates, Wasser, fremde Belegung und Bridge-/Ramp-Endpoints bleiben gesperrt.
Es gibt keine Sondererlaubnis aus Modellnummer, Bild oder Mapname. Wie beim
bisherigen Vacant-Hausbau ist keine Versorgungsroute Voraussetzung dieser
reinen Einflussquelle; es wird dadurch kein Transportzugang geschaffen.

Erfolg veröffentlicht gemeinsam den bezahlten Container, seinen typisierten
Quelleneintrag, alle vier Ownerzellen, die nächste BuildingId, Kosten und genau
eine Platzierungsrevision. Er darf nicht versehentlich normale
`PlaceWell`-Funktionen aktivieren. Häuser bleiben bezahlte Vacant-Nullzustände.

Abriss ist ausschließlich für eine passende lebende Well72-Quelle erlaubt.
Andere Gebäude einschließlich Häuser bleiben für diesen neuen Pfad gesperrt.
Commandprüfung und `demolition_status`/UI melden dieselbe Entscheidung.
Vorher werden vollständige Quelleneindeutigkeit, sichere Ownerzuordnung und
Abwesenheit von Cargo, Reservationen oder begonnenen Transportkanten geprüft.
Solche Zustände dürfen für diese Quelle ohnehin nicht entstehen.

Erfolg entfernt Quelle, Container und vier Ownerzellen gemeinsam, schreibt
einen historischen Entfernungseintrag und erhöht die Revision einmal.
Es gibt **keine Erstattung**; BuildingIds werden nie wiederverwendet.
Der neue Removalhandler umgeht den bisherigen generischen Abrisshandler:
dessen Legacy-Kostenfallback und `DemolitionHistory` sind hierfür ungeeignet.
Cacheaufbau und nötige Allokationen werden vor Veröffentlichung vorbereitet.
Jeder verweigerte oder fehlgeschlagene Befehl erhält den vollständigen
WorldSnapshot, Funds, IDs, Historie, Revision und gültigen Cache unverändert.

## Finanz- und Restore-Authority

Der feste Startbetrag ist nur durch Rule8 bestimmt, kein frei veränderbares
Savefeld. Bestehende Rule7-Funds1300 bleiben erhalten. Die vorhandenen
authored Kosten House80, Road2, Bridge40+20 je Wasserzelle und Ramp40 bleiben
in ihrem jeweiligen Vertrag unverändert; die neue Quelle kostet60.
Der Scheduler erzeugt weder Einnahmen noch Wartung. Warten und Abriss
finanzieren nichts.

Schema24 speichert in einem eigenen strikten Quellenpayload:

- Lebende Quellen: `building_id` und den einzigen erlaubten typisierten Typ72.
  Position, Fußabdruck und `placed_tick` stammen ausschließlich vom zugehörigen
  Building-Datensatz; der Preis aus dem Regelvertrag.
- Historische Entfernungen: `building_id`, Typ72, `placed_tick`, `removed_tick`.
  Diese Einträge erzeugen keine Fläche, aktive Quelle oder restaurierbares
  Gebäude. Bei Fixpreis60 und ohne Einnahmen sind höchstens33 möglich.
- Bestehende Modellidentität mit Format, SHA, Bytezahl und Difficulty;
  keine Originalzeilen, Dateipfade, Modellpointer oder Einflusscachewerte.

Das bisherige `DemolitionHistory` bleibt vollständig0; die neue Historie
wird genau einmal im neuen Finanzvalidator berücksichtigt. Für jeden Save gilt:

```text
R = roads_placed_total
H = Anzahl lebender Vacant-Häuser
L = Anzahl lebender Well72-Quellen
D = Anzahl historischer Well72-Entfernungen

S = 2*R + Summe gültiger Bridge-Kosten + 40*Ramp-Anzahl + 80*H + 60*(L+D)
construction_spent_total = S
0 <= S <= 2000
treasury = 2000 - S
taxes_collected_total = maintenance_spent_total = 0
road_revision = roads_placed_total + roads_removed_total
              + Anzahl lebender Gebäude + 2*D + Bridge-Anzahl + Ramp-Anzahl
next_building_id = H + L + D + 1
```

Alle Rechnungen erfolgen geprüft vor Multiplikation/Addition. Bestehende
Road-, Bridge-, Ramp-, Commandsequence- und Zeitinvarianten gelten zusätzlich.
H≤20, L≤4 und das bisherige Gesamtgebäudelimit46 bleiben Grenzen; dieses
Paket erlaubt tatsächlich nur H+L≤24 lebende Gebäude.
Zusätzlich gilt L+D≤33; unter dem festen Budget sind insgesamt höchstens33
Gebäudekäufe möglich, also `next_building_id≤34`. Auch die teureren Hauskäufe
werden in dieser groben Obergrenze berücksichtigt.

Haus-, Quellen- und Entfernungseinträge sind jeweils nach positiver BuildingId
sortiert, paarweise disjunkt und gemeinsam exakt das Intervall
`1…next_building_id−1`. Jedes lebende Gebäude besitzt genau einen passenden
Haus- oder Quelleneintrag. Keine fehlenden, doppelten oder fremden Typbindungen.
Aktiv gilt `placed_tick≤ticks`, historisch
`placed_tick≤removed_tick≤ticks`; die bestehenden MAX-Grenzen bleiben erhalten.
Alle übrigen Source-Building-Felder haben den ausdrücklich neutralen
Defaultzustand. Häuser behalten ihren bisherigen Vacant-/Status0-/Accessfalse-
Nullzustand, einschließlich null Waren-/Service-/Actorzustand.

Restore prüft zunächst rekursiv das Schema, exakte Schlüssel/Typen und
Duplikatverbot innerhalb der bestehenden8MiB-/Tiefe32-Grenzen. Danach werden
Modellidentität und typisierte Eignung, vollständige Map-/Source-SHAs,
Buildability und unveränderte policy3, IDs, Ledger, Finanzen, reale Footprints,
Ownerkonflikte und sämtliche Bridge-/Ramp-Verträge geprüft. Erst nach einem
erfolgreichen Cacheaufbau wird die neue World veröffentlicht.

F5 verwendet dieselbe unveränderliche Sessioninstanz und prüft live SHA/Größe;
F9 bereitet bei fehlender transient übergebener Instanz genau einmal einen
Kandidaten vor. Fehlende/geänderte Daten werden vor Veröffentlichung verweigert.
Das bestehende [Sessionmodell](model-session-binding.md) bleibt zuständig.
Schema23 erhält weder ein leeres Quellenfeld noch eine Migration oder neue
Budgetdeutung. Normales Restore darf Schema24 nicht am Entwicklergate vorbei
laden. Jede Ablehnung lässt aktive World und vorhandene Zieldatei unverändert.

## Einflusscache und read-only Nachweis

Der vorhandene geprüfte Quadratkern bleibt unverändert. Modellfelder kommen
aus der vorbereiteten Instanz; physische2×2-Footprints aus der World.
Lebende Quellen werden nach BuildingId aufsteigend verarbeitet. Diese Reihenfolge
ist eine **authored deterministische Festlegung**, keine bewiesene originale
Managerreihenfolge. Die bestehende Sättigung nach jeder Addition und das
Maximum über die vier Hauszellen bleiben erhalten. Bonus0 und
`WallBlocking::None` sind ausdrücklich fest; keine erfundene+10-Bedingung,
Originalwall-/Markt-Rechteck- oder Lifecycleparität.

Ein Kandidatencache wird bei Create/Restore sowie erfolgreich abgeschlossenem
Quellenbau/-abriss aufgebaut, begrenzt auf die bisherigen Kernlimits
228×228 Zellen und höchstens vier Quellen. Hausbau ändert die Abfragefläche,
aber keine emittierende Quelle und erzwingt keinen vollständigen Rebuild.
Tick, Straßenbau, Kamera, Hover und Frame lösen keinen Rebuild aus. Das
Sessionmodell ist unveränderlich; ein Dateiwechsel ändert keine laufende Szene.
Eine spätere Modell-/Difficultywechsel-Funktion wäre ein eigener Auftrag.

Hauswertabfragen lesen nur vier Cachezellen. Der ausdrücklich angeforderte
Inspector darf die vier Hauszellen mit höchstens vier vorbereiteten Quellen
nachvollziehen und zeigt BuildingId/Typ72, reale Geometrie, Difficulty,
Modell-SHA und Feldprovenance sowie den gewählten Maximalwert. Bei gleichem
Maximum wird die erste Zelle in Reihenfolge y/x benannt. Beiträge werden an
dieser Zelle einschließlich Sättigung vor/nach jeder Quelle erklärt;
einzelne Quellenmaxima werden nicht fälschlich addiert. Keine Datei-/Mapsuche,
Originaldekodierung, BFS, Command- oder Worldmutation in Frame/Hausabfrage.
Abfrage, Cache und Provenance erzeugen keine zusätzliche Save-Authority.

## Bezahlte20-Haus-Abnahme und Vergleich

Die konkrete authored Testkarte hat24×20 vollständig erlaubte Testzellen unter
policy3. Sie ist kein Originalmap-Beweis. Alle Käufe laufen durch reguläre
World-Befehle; keine Snapshot-Injektion, Gratisquelle oder nachträgliche Funds.

- 20 Häuser: Origins `(2+3*c, 2+3*r)`, `c=0…4`, `r=0…3`; jeweils2×2.
- 40 eindeutige Straßen: `(x,4)` für `x=0…23` und `(20,y)` für `y=0…16`;
  Schnittpunkt einmal bauen.
- Quellen A `(18,6)` und B `(18,11)`, jeweils2×2. A wird entfernt und anschließend
  als neue bezahlte Quelle am Kartenrand `(0,0)` wiedergekauft.

| Schritt in genau dieser Reihenfolge | Funds | Ausgaben gesamt | BuildingIds |
|---|---:|---:|---|
| Start | 2000 | 0 | nächste1 |
|40 Straßen | 1920 | 80 | nächste1 |
|20 Häuser | 320 | 1680 |1…20, nächste21 |
| Quelle A | 260 | 1740 |21, nächste22 |
| Quelle B | 200 | 1800 |22, nächste23 |
| A entfernen, Save/Restore | 200 | 1800 | Historie21, nächste23 |
| A am Rand neu kaufen, Save/Restore | 140 | 1860 |23; Historie21; nächste24 |

Reload nach beiden Mutationen wird zusätzlich in einem neuen Prozess geprüft:
vollständige SaveDocument-Gleichheit, identische IDs/Finanzen/History,
deterministische weitere Schedulerzeit sowie nach Wiederaufbau identische
Hauswerte/Provenance. Mindestens ein realer Hauswert muss sich durch eine
Quelle tatsächlich ändern; ein leerer oder durchgehend wirkungsloser Cache
ist kein Feature-PASS. Weitere Rand-/Überlappungs- und Sättigungsvektoren des
reinen Kerns bleiben getrennte synthetische Kontrollen.

**Wesentliche Änderung des Vergleichsvertrags:** Unverändertes City-v13
verweigert Wells; außerdem reichen seine1300 Start-Funds nicht für20 Häuser.
Eine identische dort bezahlte World lässt sich so nicht herstellen.
[`desirability_impact`/`desirability_contribution`](../../src/simulation/WorldDesirability.cpp)
geben für Wells0 zurück. Der empfohlene Vergleich liest daher ausschließlich
die tatsächlich bezahlte neue Szene und berechnet mit den unveränderten reinen
City-v13-Funktionen den Referenzwert, wobei die ausdrücklich authored
Vergleichszuordnung Well72→Well gilt. Für diese Szene ist die Referenz an allen
20 Häusern0, vor/nach Abriss, Wiederkauf und Reload.

Das ist ein **Algorithmusvergleich**, kein City-v13-World-/Spielnachweis.
Keine illegale City-v13-Platzierung oder Änderung ihrer Snapshots/Regeln.
Die echten City-v13-Negativtests für Wells und die übrigen Legacyregressionen
bleiben zusätzlich verbindlich. Falls stattdessen die wörtliche identische
bezahlte City-v13-World verlangt bleibt, ist diese Abnahme weiterhin blockiert;
die vorgeschlagene Well-only-Variante löst sie nicht.

## Freigabe, Umsetzung und künftige Pflichtprüfungen

Nach ausdrücklicher Annahme werden zuerst die OE-12-/Gate-B-/Versionsstellen
im Arbeitspaket mit genau dieser Entscheidung abgeglichen. Die übrigen Tasks
werden weder als erledigt umnummeriert noch automatisch implementiert.
Danach folgen zusammengehörige Änderungen für Quellenbau, neutralen Objekttyp,
Abrissledger, Schema24, Cache und Entwicklerbedienung mit diesen Abnahmen:

1. Reale bezahlte obige20-Haus-Szene mit Originalmodell; Add/Remove/Rand,
   F5/F9 und separater Prozess, exakte Finanzen und Werte. Cache-Rebuildzähler
   nur im Testadapter, relevante Ereignisse gegen Frame/Tick/Hausabfragen geprüft.
2. Atomare Ablehnung von zu wenig Funds, Quelle5, fehlender Modellbindung,
   fremdem Quellentyp, unzulässigem Bauplatz/Abriss und Überläufen. Wasser-,
   Upkeep-/Steuer-/Worker-/Goods-/Actorabfragen und HUD bleiben neutral.
3. Adversarial-Restore für falsche Version, SHA/Größe/Difficulty/Policy,
   ID-Lücken/-Duplikate, wiederbelebte Historie, fehlende Ledgerkäufe,
   falsche Funds/Kosten/Timestamps/Footprints und unerlaubte Zustände.
4. Schemas1–23 und kanonische Legacy-World-/Map-/Policy-/Savebytes unverändert;
   echte City-v13-/16-Regressionen einschließlich Bridge/Ramp. Bestehende
   Originalinventare werden vor/nachher schreibgeschützt verglichen.
5. Tatsächliche relevante Tests und anschließend volle Gate-B-Suites in
   Debug, Release und ASan+UBSan. Scripted, native Lieferung und menschliche
   Akzeptanz werden getrennt protokolliert. Kein ursprünglicher Gate-B-PASS
   allein durch Annahme dieses geänderten Vertrags.

**Stand dieses Entwurfs:** nur Quellen-/Codeprüfung, unabhängige Designreviews,
Layout-/Finanzarithmetik und Dokumentationsprüfung. Keine Rule8-/Schema24-
Implementierung, kein ausgeführter bezahlter20-Haus-Worldtest, kein Build,
keine Gate-B-Suite, kein menschlicher Playtest. Der komplette Vorschlag ist
die zur Entscheidung vorgelegte Änderung, nicht bereits gültiges Verhalten.
