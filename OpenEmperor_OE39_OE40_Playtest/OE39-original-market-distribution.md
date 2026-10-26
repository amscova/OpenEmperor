# OE-39 – Original Economy: beweisbare Warenverteilung und dauerhafte Common10-Versorgung

## Auftrag und Ziel

Repository: `https://github.com/oliverjessner/OpenEmperor`

Ausgangs-HEAD bei Auftragserstellung: `81bc9218958f7ade116fc43d3df80975c1206e67` (25.10.2026). Vor Beginn `git fetch`, `git status`, lokale Branches und tatsächlichen HEAD prüfen. Bereits vorhandene nicht gepushte Arbeiten erhalten. Baseline ist der nachweislich geprüfte OE-38-Stand; nichts von OE-34 bis OE-38 neu implementieren.

**Ziel:** Den in OE-38 reproduzierten *physisch vorhandenen, aber nicht zuverlässig zugestellten* Hemp-/Ceramics-Bestand der `balanced-three`/`common-reserved`-Stadt beheben. Durch echte bezahlte Gebäude, gebuchte Warenbewegungen und produktive House-Evaluationsereignisse prüfen, ob ein regulär gekauftes **Luxurious Apartment (Common Typ 10) nach seinem ersten Aufstieg mindestens 20.400 zusammenhängende Ticks auf Typ 10 bleibt**. Das finanzielle Ergebnis wird unabhängig bewertet. Der nachgewiesene Elite17/DES-Blocker gehört **nicht** zu diesem Auftrag.

Eine Verbesserung ist nur ein echter Gameplay-Erfolg, wenn die gesamte Kette `Farm/Industry -> Warehouse -> Market -> House -> Consumption/Evolution` unter denselben Save-, Actor-, Terrain-, Routen- und Bilanzregeln funktioniert. Weder Überschusslager noch ein geplanter Kurierbesuch bedeuten Versorgung.

## Verbindliche Quellen und Baseline

Zuerst vollständig lesen:

- `AGENTS.md`, `KNOWN_ISSUES.md`, `docs/testing.md`
- `docs/original-economy/architecture-map.md`
- `docs/original-economy/oe37-sustainable-economy.md`
- `docs/original-economy/oe38-paid-high-tier-progression.md`
- `src/simulation/WorldOriginalMarket.cpp`, `src/simulation/OriginalGoodsLedger.cpp`, `src/simulation/WorldOriginalProduction.cpp`
- `src/simulation/WorldOriginalIndustry.cpp`, `src/simulation/WorldOriginalHemp.cpp`, `src/simulation/WorldOriginalHousing.cpp`
- `src/simulation/World.h`, `src/simulation/World.cpp`, `src/persistence/SandboxSave.cpp`
- `tests/HighTierCommonSupplyTests.cpp`, `tests/HighTierProgressionStrategy.h`, `tests/HighTierProgressionMetrics.h`
- `tests/OriginalMarketTransportTests.cpp`, `tests/OriginalGoodsLedgerTests.cpp`, `tools/test_high_tier_progression.py`

OE-38 ist **PARTIAL**, nicht PASS:

- Common10/Haus #3 wird durch echte Käufe und reale Ankünfte bei **Tick 27.733**, mit **11.431 Funds**, **47 Bewohnern im Zielhaus**, **2.588 Funds Gesamtkonstruktion** erreicht.
- Im anschließenden 20.400-Tick-Fenster fällt der Zieltyp **18-mal** zurück, trotz positivem operativem Cashflow von **+3.705 Funds**. Nachweislich fehlen Hemp, teilweise Ceramics.
- Die erste konkrete Unterversorgung ist eine Hausbewertung bei **Tick 29.008**; die zehnte aufeinanderfolgende negative Bewertung verursacht bei **Tick 29.467** den tatsächlichen Typ-10-zu-9-Abstieg.
- Zur ersten Störung liegen gleichzeitig etwa **28 Hemp im Warehouse**, **4 ausstehende Reservierungen**, Bestände bei den Farmen, aber im Zielhaus und zeitweise im Markt kein Hemp; der einzige Markt-Warendistributor befindet sich auf einem langen Rückweg. Ware existiert, trifft aber zu spät ein.
- Die historische Wahl in `WorldOriginalMarket.cpp` rotiert mit der letzten gespeicherten `(target, good)`-Buchung über mögliche Paare; der Markt besitzt einen Warendistributor. Nicht ungeprüft behaupten, dass die Rotation allein schuld ist. Physische Routenzeit und die verfügbare Kurierzahl sind unabhängige Engpässe.
- Ein bereits untersuchter zweiter Markt beschleunigt zwar den ersten Aufstieg, hält Typ 10 aber nicht stabil. Einfach mehr Märkte oder Farmen hinzustellen ist keine ausreichende Lösung.
- Im bisherigen echten Fenster liegen Hemp-Verbrauch und Harvest unter der nominellen Zwei-Farmen-Kapazität. Höhere Farmgrenzen als angebliche Lösung sind dadurch **nicht begründet**.

## Phase A – Ursache vor Codeänderung isolieren

1. Reproduziere den unveränderten OE-38 `common-reserved`-Fall auf der echten `Cities/Xia.map`, Difficulty Normal und unveränderter Modell-SHA. Benutze die existierenden Test-Runner und echte Source-Fingerprints. Neue Artefakte ausschließlich unter einer frischen, ignorierten `.local/original-economy/oe39/`-Struktur speichern.
2. Instrumentiere ausschließlich testseitig/read-only auf jeder relevanten Phase, besonders um Ticks 27.733–29.467:
   - Pro-Good Stock, Cargo, Source-Reservation, Incoming-Reservation, Headroom und bereits konsumierte Mengen an Farm, Warehouse, Market und House.
   - Pro Kurier: ID, Owner, Role, Phase, Target, letzte `(target, good)`-Buchung, Sequenz, Dispatch-Refusal, tatsächliche Route, Edge Progress, Ausfahrts-/Ankunfts-/Rückkehrzeit.
   - Aktueller und nächster Hausbedarf gemäß denselben unveränderten `HouseEvolutionRules`, tatsächliche Phase-39-Inputs und Konsumzeitpunkte.
   - Aktive Operations, Staffing, Feuer und echte Besuchs-/Versorgungsereignisse.
3. Unterscheide quantitative Ursachen: fehlende Produktion, fehlender Warehouse-Nachschub, blockierter Market-Inbound, Market-Outbound-Laufzeit, unpassende Commodity-/House-Reihenfolge, Ziel-Headroom, Reservations/Returns, lange Routen und Brände.
4. Für die erste Abwertung eine überprüfbare **Transfer-Kausalkette** mit echten IDs und Zeiten erstellen. Ein Log mit `warehouse_stock > 0` allein genügt nicht. Erkläre den minimal möglichen Durchsatz mit vorhandenen Kurieren und legalen Routen.
5. Vergleichsvarianten nur mit regulär bezahlten Bau-/Betriebsaktionen: kürzere Verbindungen, nahe Produktionsstätte, vollständig angeschlossener zweiter Markt, alternative Worker-Prioritäten. Jede Variante separat durchspielen; kein Snapshot-/Stock-Grant. Messbar mindestens 20.400 Ticks nach erstem Common10.
6. Erste Ergebnisdatei: `docs/original-economy/oe39-market-diagnosis.md`, inklusive verifizierter und verworfener Hypothesen. **Kein Gameplay- oder Versionsumbau**, bevor ein messbarer Eingriff gewählt wurde.

**Gate A:** Die erste Common10-Devolution lässt sich mit wirklichen Liefer-/Bestands-/Zeitereignissen erklären und im unveränderten Rule12 reproduzieren. Fehlt der Befund, `BLOCKED` melden; keine kosmetische Dispatcheränderung.

Nach bestandenem Gate A: lokaler Commit `test(original): diagnose common10 goods distribution`.

## Phase B – minimalen, versionierten Korrekturvertrag ableiten

1. Zuerst prüfen, ob eine **rein spielerseitige**, vollständig bezahlte Routen-/Marktvariante die 20.400 Ticks ohne Typverlust schafft. Falls ja, **kein künstliches neues Regelprofil** und keine Simulationsregel ändern. Bauplan, Grenzen und Test absichern; in Phase C übergehen.
2. Falls die nachgewiesene Ursache ein eigenes Verteilungsverhalten ist, implementiere einen **neuen, explizit wählbaren** `City-v16 Rule13 / Schema29 / MapPermissions Policy3`-Nachfolger von Rule12 (nur wenn Versionsnummern beim Start wirklich frei sind). Neue Regel darf die existierende Originalwirtschaft und ihren FireInspector/Upkeep-Vertrag erben, muss abweichende Dispatch-Semantik aber eindeutig versionieren.
3. Implementiere eine **deterministische, bedarfsbezogene Warenauswahl** als isolierte, testbare Policy auf Basis wirklich gespeicherter `HouseState`-Bestände, der vorbereiteten Modellzeilen, konkreter verfügbaren Source-Stocks, Reservations und erreichbarer `dynamic_target_routes`.
   - Zuerst die physisch realisierbaren aktuellen Bedürfnisse; danach nachweislich nutzbare nächste Stufen. Kein bevorzugter Vorab-Kauf, keine imaginäre Lieferung.
   - Markt- und Warehouse-Entscheidungen kohärent behandeln: Ein Markt kann keinen dringend benötigten Stoff ausliefern, den sein Warehouse-Courier niemals heranbringt.
   - Gleichrangige Ziele brauchen einen deterministischen Fairness-/Rotationstie-Break, sodass andere Haushalte und Warentypen nicht dauerhaft verhungern. Bereits gespeicherte Sequenz-/Cursor-Autorität bevorzugen; zusätzlichen Savezustand nur mit eigenem Validierungsvertrag einführen.
   - Route Pending, Straßenentfernung/-Reparatur, begonnenes Edge, Return, Fire, Paused, Unstaffed, neuer Bedarf nach Devolution und begrenzter Ziel-Headroom müssen weiterhin stimmen.
   - Keine `World`-Kopie, BFS, Dateizugriffe, ungedeckten Allocations, globalen Kartenscan oder Renderer-/Hover-Arbeit pro Frame. Vorhandene revisionierte Routen wiederverwenden.
4. Beweise vor zusätzlichen Kurieren oder erhöhten Cargo-/Farm-Caps, warum eine Auswahlregel allein den nachgewiesenen maximalen Lieferrhythmus nicht erreichen kann. **Falls** mehr bezahlte Markt-Kapazität zwingend ist: zunächst schriftliche Bounds-/Ownership-/Worker-/Funds-/Save-Entscheidung dokumentieren. Ohne abgesicherten Vertrag nicht einfach kostenlos zusätzliche Couriers erzeugen.
5. Nur wirklich erforderliche, v13-gebundene Authority ändern. Rule10/Schema26, Rule11/Schema27, Rule12/Schema28 und sämtliche älteren Regeln/Saves behalten **exakt** ihre bisherige Semantik und Bytes; keine Auto-Migration. Das alte `WorldOriginalMarket`-Round-Robin muss in Rule12 und darunter reproduzierbar identisch bleiben.
6. Für Schema29 dieselben strengen Anforderungen wie bisher: Model-/Map-/Difficulty-/Policy-Fingerprints, Quelle, Gebäude-/Courier-IDs, Route/Edge, Transfers, Conservation, Actor-/Stock-/Reservations-/Fire-Provenienz, Funds/Tax/Upkeep. Zusätzliche Authority nur kanonisch und mit adversarialen Negativfällen. Keine neue Parallelbuchhaltung.
7. Die UI muss den Nachfolger ausdrücklich auswählen können und korrekt laden; Default-Profile bleiben unverändert. Keine irreführende Behauptung von originalem Emperor-Marktverhalten: die neue Verteilungsregel ist **authored**, bis Original-EXE-Belege vorliegen.

**Gate B:** Belegte kausale Verbesserung **ohne** Verluste an Massenerhalt, deterministischer Navigation, bezahltem Aufbau und historischer Savekompatibilität. Wenn noch kein sicheres, technisch durchgesetztes Konzept existiert: `PARTIAL` mit klar begründetem Vorschlag statt Scheingewinn.

Nach bestandenem Gate B: lokaler Commit `feat(original): add versioned needs-aware market logistics` (nur wenn eine Produktivänderung stattgefunden hat; sonst Tests/Spielerstrategie entsprechend benennen).

## Phase C – produktive High-Tier-Abnahme

Auf echter, unveränderter `Cities/Xia.map` mit denselben Originaldaten:

1. Beginne mit dem bezahlten, anerkannten OE37-`balanced-three`-Aufbau, dann der realen OE38-Common-Ausbaukette. Änderungen der Bauvarianten ausschließlich als echte Commands; alle Einträge enthalten Command-Sequenz, Preis, IDs, Zeitpunkt und Funds vorher/nachher.
2. Erreiche Common10 über **tatsächliche** 9→10-World-Evaluation und physisch eingetroffene Hemp/Ceramics/Tea/Services. Keine Handzustandsänderung, keine eingefrorene `HouseType::LuxuriousApartment`-Fixture als Gameplay-Erfolg.
3. Prüfe über mindestens **20.400 aufeinanderfolgende Ticks nach erster Typ10-Evaluation**: aktuelle Typstufe in jedem Tick, durchgängige Versorgung bei jedem tatsächlichen Bewertungs-/Verbrauchsereignis, jede gültige Inventory- und Funds-Buchung, bereits begonnene Wege, Fire-/Recovery-/Staffing-Effekte.
4. Erfolg der Retention nur bei **0** Typ10-Rückentwicklungen, **0** nicht erfüllten aktuellen Hemp/Ceramics-Bedarfsauswertungen im Zielhaus und `HouseType::LuxuriousApartment` an jedem Tick im Fenster. Bei Feuer oder fehlendem DES/Service den verursachenden **anderen** Blocker genau ausweisen; solche Ausfälle nicht als Logistik-PASS zählen.
5. Prüfe ebenso Ziel-Haus Fairness und Nebeneffekte an allen anderen bewohnten Haushalten: ihre tatsächlichen Fehlbedarfe, Devolutions, Wartezeiten und alle Güterbilanzen. Keine künstliche Priorisierung einer einzigen Testhaus-ID, keine pauschale Starvation von Nebenstädten.
6. Separat Finanzmessung in vollständigen 20.400-Tick-Fenstern: Steuern, Unterhalt, neue bezahlte Baukosten, Fuel/Fire-Ausfälle, positiver/negativer Cashflow und tatsächlich vorhandene Funds. Positive Bilanz ist nicht automatisch Bedarfsdeckung.
7. Verifiziere vollständiges `SaveDocument`/Snapshot/Recovery in separaten Creator-/Restore-Prozessen **vor** der ersten Devolution, während des ehemaligen Problemfensters, bei aktivem Cargo und am Ende. Byteidentität ist für jede unverändert fortgesetzte Referenz verbindlich. Alte Schema28-Dateien dürfen sich nicht zu Schema29 umetikettieren.
8. Mache in der tatsächlichen `SandboxView` F5/F9, selektierten House-Inspector und 1×/2×/4×-Captures; PNGs öffnen und visuell prüfen. Native/human-Abnahme ausdrücklich getrennt.
9. Regressionsmatrix: historische CTests / eingefrorene Rule12-Eintritts- und Devolutionswitnesses / unveränderte OE37-Nachhaltigkeit / FireWatch/Inspector, Debug, Release und ASan+UBSan. Den ursprünglichen OE38-Verlust unter Rule12 ausdrücklich erhalten und separat neu nachprüfen; nicht die roten historischen Bedingungen umdefinieren.
10. Ergebnisdatei: `docs/original-economy/oe39-market-retention.md`. Unter `KNOWN_ISSUES.md` nur wirklich verifizierte Statusänderungen dokumentieren; keine neue Markdown-Tagebuchsammlung.

**Gate C:** `PASS` erst mit einer real bezahlten, tatsächlich wiederholbar **durchgehend** auf Common10 versorgten Stadt und passenden Source-/Snapshot-/Actor-Nachweisen. Wenn nur eine lokale Verbesserungsquote erreicht wurde, `PARTIAL`, konkrete Ursache + Repro angeben.

Nach Gate C: lokaler Commit `test(original): verify sustained paid common10 market supply`.

## Nichtumfang / rote Linien

- **Elite17 ist in aktuellem Normal-Rule12 bewiesen unerreichbar:** der streng belegte 4×4-DES-Oberwert liegt bei **79**, der 16→17-Übergang erfordert **80**. OE-39 darf diesen Blocker nicht mit `+1`, niedrigeren Schwellwerten, frei gestapelten Quellen, gebrochenen Footprints oder Common→Elite-Umwandlung umgehen.
- Originale Venue-/Performerketten und originale Krankheit gehören nicht zu OE-39. Ein unverifizierter Model-Typ oder Sprite ist keine Beweiskette für neue Simulation.
- Keine Änderung der Modelldatei oder Commit proprietärer Assets/Disassembly-Payloads. Nur unsere saubere C++20-Reimplementierung und Metadaten unter Git.
- Keine implizite Veränderung von Rule12/Schema28/Revision27 oder früheren Saves. Neue Gameplaysemantik braucht ein ausdrücklich separates Regelprofil.
- Originale Inspector-/Fire-Beweishistorie hat eine finite Grenze; Tests dürfen das nicht durch Ausknipsen der Validierung umgehen.
- Keine nicht belegten Aussagen über unbegrenzte Nachhaltigkeit, vollständige 15-Stufen-Kette, originalgetreue 3D-Geometrie oder menschliche Abnahme.

## Git und Abschlussbericht

Arbeite in eigenem Branch `feat/oe39-market-retention` vom überprüften letzten Main-Stand. Prüfe vor jedem Commit `git diff --check`, fokussierte CTests und passende Source-/Originaldatenidentitäten. Mach pro **bestandener** Phase einen eigenen lokalen Commit; geblockte Phasen bekommen nachvollziehbare Dokumentation, keinen falschen Feature-PASS.

Kein Push, Merge, Tag, Release oder automatisches Überschreiben des Nutzer-Branches. Die vollständigen Testbinaries, private Saves, Screenshots und Rohdaten bleiben unter `.local/`.

Am Ende gib kompakt an:

| Kriterium | Verbindliches Ergebnis |
|---|---|
| Ausgangs-HEAD und neue Commit-SHAs | Konkret und überprüfbar |
| Erster Typ10-Aufstieg | Echte Tick-/Building-/Funds-/Source-Witness |
| Retention | Ticks, Typänderungen, erfüllte/unerfüllte Inputs |
| First-failure Root Cause | Konkrete physische Kette, kein bloßer Verdacht |
| Finanzfenster | Tatsächliche Steuern minus sämtlicher Unterhalt |
| Versandte/angekommene Waren | Getrennt, Massenerhalt pro Good |
| Save/Recovery/Old-byte-Regression | Ausgeführte, identische und abgelehnte Fälle |
| Debug/Release/ASan+UBSan/SDL | Tatsächlich gelaufen, SKIP/NOT_RUN getrennt |
| Originalspiel-Parität | `PARTIAL` bis unabhängig belegt |

**Folgeauftrag OE-40:** Originale zusätzliche DES-Quellen/Source-Lifecycle unabhängig rekonstruieren und ggf. eine gesondert versionierte, bezahlte Elite-Entwicklung ermöglichen. Kein willkürliches DES-Bonus-Hack.
