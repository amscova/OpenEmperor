# DECISIONS – verbindliches Architektur- und Evidenzregister

Dieser Stand enthält zunächst **Planungsentscheidungen**, keine bestätigten Implementierungsergebnisse. Codex ergänzt nach jedem Task Belege und Review-Status.

| ID | Anfangsentscheidung | Status / Beweisbedarf |
|---|---|---|
| D-01 | Kein globaler Merge des `amscova/windows-port` Forks; selektiv unabhängig testen | plan |
| D-02 | Original Model-Datei bei Rule7 read-only/mit Hash einbinden | OE-11: explizites unveränderliches Sessionmodell, einmalige typisierte Vorbereitung; Live-SHA/Größe bei Save/Restore; kein Modellzugriff im Tick |
| D-03 | City-v16 Rule7 / MapPermissions policy3 / Schema23 als geplanter, opt-in Weg | OE-13: Rule7/Schema23 und Rule8/Schema24 eingefroren; ausdrücklich angenommener Nachfolger Rule9/Schema25 ausschließlich Entwickler-Opt-in, policy3 unverändert; echte Waren0/unsupported, keine Standardprofil-/Menüfreigabe |
| D-04 | Alte Save-Schemas 1–24 unverändert und keine Auto-Migration | zwingende Invariante;72 vollständige OE-00-Saves/Snapshots und separat eingefrorene7/23-/8/24-Bytes nach OE-13 exakt |
| D-05 | Brücken/Rampen Rule6 von Rule7 weiter benutzen, ihre Authority aber nicht ändern | OE-08/12/13: explizite begrenzte Capability-Gates auch für Rule8/9; bezahlte Befehle/Restore und exakter Legacy-Replay bestanden |
| D-06 | Original-Model-IDs nur nach eindeutiger Quelle als VERIFIED führen | einzelne IDs offen |
| D-07 | Originalspiel-Simulationsschritte nicht automatisch mit 20Hz World-Ticks gleichsetzen | OE-10: authored Mappingversion1 bindet 1 Tick/Phase an Rule7/Schema23; 51/16/12-Counterstruktur beobachtet, elapsed-Kalenderepoche0 getrennt vom Originaljahr; tatsächliche Prüfungen im Fortschritt |
| D-08 | 15 House-Zeilen sind nicht gleich dem aktuellen World-Level 0..2 | eigene Authority erforderlich |
| D-09 | Goods inklusive Alternativen nur mit Realproducer/RealCourier/Conservation | OE-13: reines Buchhaltungsfundament technisch geprüft, reale Waren0/unsupported; positive World-Produktion/Transport/Verbrauch bleiben OE-14/15/17 |
| D-10 | Versions-Gates bis zu einer echten paid Hut->Plain-Cottage-Szene nicht im Standardmenü anbieten | ab OE-21 nachweisen |

## Neue Entscheidungen

### OE-17: tatsächlicher Verbrauch in gespeicherter Zyklusphase

- **Vertrag vor Implementierung:**
  [Verbrauchsvertrag](../original-economy/consumption-runtime.md), nach OE-16-
  Commit `86cde0c54577a9889632e45432fe7fe18acd7fac`. Ausschließlich die aktive
  Rule10/Schema26/policy3 wechselt auf Entwicklungsrevision17; frühere
  Entwicklungsrevisionen werden verweigert. Regeln1–9/Schemas1–25 bleiben
  unverändert, der normale Spielstart wird nicht freigegeben.
- **`exe` und `authored` Zeitintegration:** Der unveränderte pure Kern verwendet
  die aktuelle kanonische Housezeile, unabhängig von Footprint oder
  Bewohnerzahl. Null Bewohner überspringt den Konsum; jede Ware bucht
  `min(stock,max(need,0))`. Bronze und Lack sind unabhängig, bleiben hier aber
  mangels echter Quellen0. Der gespeicherte Scheduler bleibt die einzige
  Clock: Phase39/Selector0 oder7, sichtbar abgeschlossene Ticks40/397/856/1213.
  Die authored Reihenfolge verarbeitet tatsächliche Bewegung, Fire und Zuzug
  vor dem Konsum; eine reale Ankunft derselben Tickgrenze darf verbraucht
  werden. Keine alte400-Tick-Nachfrage, zusätzliche Phase oder Besuchsgutschrift.
- **Modelltreue und positiver Beleg:** Der gebundene originale Shelterbedarf
  Hemp0/Ceramics0 bleibt in allen fünf Difficulties0. Der reale Modellpfad
  konsumiert deshalb korrekt nichts. Der getrennte positive Prozesspfad ändert
  ausschließlich zwei Bedarfsbytes einer privaten Modellkopie auf1/1; Preise,
  Kapazität7, Employees und sämtliche anderen Bytes bleiben gleich. Sechs
  normal bezahlte Häuser liefern42 regulär zugezogene Bewohner/Worker; Farm,
  Feld, Clay, Pottery, Warehouse und Market produzieren und liefern tatsächlich.
  Positive Tests sind authored Bedarfsintegration, keine originale
  Shelterverbrauchs- oder vorgezogene Evolutionsbehauptung.
- **Gespeicherte Senke und Restore:** `house_consumption` enthält exakt eine
  sortierte Zeile je gekauftem House mit sechs uint64-Zählern. HouseState
  bleibt der einzige Hausbestand; die Spaltensummen entsprechen exakt
  `GoodsTotals.consumed`. Bounded Ledger-/Housekandidaten werden vor ihrer
  Veröffentlichung vollständig geprüft, ohne Worldkopie oder doppelte Abbuchung.
  Alter und tatsächlich erreichbare Konsumereignisse begrenzen die Historie;
  die vorbereitete aktuelle Shelterzeile verschärft das strukturelle Maximum8
  je Ereignis. Der isolierte Need1-Test erhält tatsächlichen Empfang und Masse,
  besteht bewusst die Strukturprüfung und scheitert erst beim gebundenen
  Restore/Writer. Nullbedarf und koordinierte Überläufe bleiben negativ geprüft.
- **Herkunft und Rotation:** Auch vollständig konsumierte Waren brauchen eine
  mögliche tatsächliche Farm-/Distributorankunft. Passende retained Completed-
  Lieferzeilen dürfen den kumulativen Houseempfang aus Stock plus Verbrauch
  nicht überschreiten. Die begrenzte letzte Transferhistorie ist weiterhin
  notwendige Provenienz, kein vollständiges Besuchsjournal. Die gemeinsame
  Waren-/Zielrotation verhindert die im neuen Verbrauchspfad beobachtbare
  dauerhafte Ein-Waren-Zielbindung; finite6-/20-House-Läufe belegen beide
  tatsächlichen Waren je Haus, keine universelle Starvation-Freiheit.
- **Tests, Fehler und Quellenbindung:** Der erste ASan-Lauf fand einen echten
  Use-after-free im neuen Test: Commands ersetzten die World während über
  ihre Buildingreferenzen iteriert wurde. Der Test sammelt jetzt IDs vor den
  Commands. Ein erster Debug-Worldtest überschritt180s; eine gemeinsame
  Sechs-Waren-Abfrage ersetzt sechs wiederholte vollständige Abfragen bei
  unveränderten Assertions und vollständigem20k-Snapshotvergleich. Beide
  Erstläufe bleiben als FAIL archiviert. Der finale Quellenfreeze
  `df7851d80fd8b2d35fcf3777e7047522afe80dd4d8bf3af536a3ad80eb1e28e7`
  unterscheidet sich von5ed0 ausschließlich in genau diesen zwei Testdateien;
  der vollständige frühere Hash wird mit den zwei gepinnten Overrides
  rekonstruiert. Runtime-, Fixture- und Legacyquellen sowie das tatsächliche
  Prozessbinary bleiben bytegleich; `test-only-fix-binding.json` bindet die
  früheren Laufbelege ausdrücklich an den finalen Stand.
- **Abnahmestand:** Finale Debug-/Release-/ASan+UBSan-Auswahl je51/51 PASS,
  insgesamt153 Konfigurations-CTests ohne Skip oder Fehler. Vier separate20k-Prozessreplays,
  unabhängiger Raw-Audit von25 Saves/20 Reports, drei74-Fall-Forkvergleiche und
  wörtlich identische72 Legacy-Saves/72 Snapshots sind bestanden; vollständige
  Zahlen, Commands und Grenzen im [Fortschritt](PROGRESS.md). **OE-17 technisch
  PASS**; eigener Task-Commit nach Abschlussprüfung. Gate C und OE-18 TODO;
  native/human NOT_RUN. Der geöffnete SDL-Nachweis zeigt reale Zweigüterankunft
  bei synthetischer Kapazität37 und Nullbedarf, keine Konsumgrafik- oder
  Originalsprite-Abnahme. Echte Originalgrafiken bleiben späterer Pflichtauftrag.

Zu jeder Entscheidung bitte ergänzen: Task-ID, Commit, Originalbeleg (`raw`, `exe`, `model` oder `authored`), begründete Alternative, Konsequenz für Save/Tests, Review-Status.

### OE-16: bezahlte Hemp Farm192 plus Hemp Field194

- **Vor Implementierung:** [Hemp-Laufzeitvertrag](../original-economy/hemp-runtime.md),
  Rule10/Schema26/Revision16, unveränderte policy3. Belegte Modellidentitäten,
  vorbereitete Cost-/Employees-Werte und2×2-/1×1-Objektflächen; separates Feld
  wird bezahlt und ausdrücklich an die Farm gebunden. Keine Gratis-Cropzone.
- **Authored:** Modell-Cost→Funds, Feldnachbarschaft,80 produktive Ticks pro
  Einheit,8 Farmstock sowie separate Hemp-Bins32/16/8 für Warehouse/Market/Haus.
  Kein behauptetes originales Rezept allein aus Resource0. Ware entsteht nur
  mit tatsächlich regulär zugewiesenen modellgebundenen Workern, laufender
  Farm und bezahltem Feld. Ein eigener Farm-Courier und bestehende WH-/Market-
  Actoren bewegen die tatsächliche Ware über denselben Navigator.
- **Kompatibilität:** Neuer ausdrücklicher aktiver Modellvorbereitungspfad;
  alter Sieben-Zeilen-Pfad verlangt keine neuen Zeilen. Typisierte IDs werden
  append-only erweitert. Farm-/Feld-/Ernteauthority strikt gespeichert,
  alte Entwicklungsrevision atomar verweigert. Nach technischem Abschluss
  unmittelbar OE-17; Originalgrafiken bleiben verbindlicher späterer Auftrag.
- **Modellvorbereitung und Evidenz:** Nur `prepare_production()` verlangt und
  prüft die beiden zusätzlichen benannten Zeilen. `prepare()` bleibt der
  eingefrorene Sieben-Zeilen-Pfad und ignoriert deren semantischen Inhalt.
  Neue synthetische und echte Fünf-Difficulty-Prüfungen erhalten alle alten
  Building-/Housewerte. Die neuen World-Objects14/15 erhalten ausdrücklich
  authored funktionale/mechanische Zuordnungen zu192/194; Präsentationsidentität
  bleibt Unknown. Die sieben bisherigen funktionalen Kandidaten werden nicht
  nachträglich als Originalidentität hochgestuft.
- **Physische Buchhaltung:** Farmstock8 und Hemp-Bins32/16/8 sind getrennte
  authored Kapazitäten. Hemp-Erntehistorie je Farm ist eigene Ursprungsauthority;
  ihre Summe entspricht `GoodsTotals.Hemp.produced`. Stock, tatsächliches
  Cargo und HouseState.hemp zählen jede Einheit einmal. Derselbe vorhandene
  Warehouse-/Market-Courier rotiert deterministisch zwischen beiden Waren;
  die letzte Transferware dient als begrenzter Cursor, keine Placeholderactoren.
  Höchstens acht letzte Transferzeilen behalten die Sequenz-Highwater.
- **Review und notwendige Herkunft:** Feldnachbarschaft promoviert vor der
  Arithmetik in int64, extreme Feldcommands werden atomar verweigert. Der
  anschließend bestätigte Farm-INT_MAX-Restoreüberlauf in der späteren
  Footprintgenerierung wird durch eine vorgelagerte vollständige Geometrieprüfung
  per Subtraktion vor jeder Offsetaddition geschlossen.42 zusätzliche
  Verweigerungen über World/Writer/Reader und vier legale Eckroundtrips sind
  ergänzt und in allen drei finalen Konfigurationen bestanden.
  Farmstock/eigenes Cargo und letzte Auslieferung
  müssen zur eigenen Ernte passen; ein feldloser zweiter Owner darf keine
  fremde Ernte erhalten. Positive nachgelagerte Bestände brauchen mögliche
  tatsächliche Ankünfte; reine koordinierte Stockverschiebungen trotz globaler
  Conservation reichen nicht. Alte Ceramicsrotationen belegen insbesondere
  keine erste Farm-Hempankunft. Die begrenzte Historie bleibt eine notwendige
  Plausibilitätsprüfung: bei späteren Sequenzen können frühere Ziel-/Warenzeilen
  kompaktierte Vorgänger sein. Keine vollständige Besuchshistorie oder
  Rekonstruktion unbekannter früherer Ziele wird behauptet.
- **Abnahmestand:** Reales bezahltes Farm→Warehouse→Market→Haus mit Hemp und
  Ceramics im selben Haus, zwei separate20k-Prozessfortsetzungen und drei
  tatsächliche74-Fall-Forkvergleiche bestanden; genaue Quellen/Binaries im
  [Fortschritt](PROGRESS.md). Neuer Quellenfreeze
  `55c8d195d45e7b2d3a22a9fb37c69fef29e2fc7520c4c363d9aa1ec798513287`
  enthält den Footprintfix. Der vorangegangene Zwischenstand6967 unterscheidet
  sich vom Prozessstand675a ausschließlich durch den ungültigen
  Test-Sentinel hinter den neuen Objects14/15; zwischen diesen Ständen keine
  Runtimeänderung. Alle drei neuen Builds, der frische reale Prozessnachweis,
  der wiederholte SDL-Nachweis und drei neu ausgeführte74-Fall-Vergleiche für55c8
  sind **PASS**;6967 bleibt als Zwischenstand erhalten.
  Finale Auswahl je49/49 CTests in Debug/Release/ASan+UBSan bestanden,147
  insgesamt ohne Skip;939 Hemp-,800 Produktions-,677 Market- und575 eingefrorene
  Rule9-Saveverweigerungen je Konfiguration belegt. Sechs tatsächliche CLI-
  Outputguards gegen das aktuelle Binary bestanden. Der frische vollständige
  Legacyabschluss zum Quellenstand55c8 ist ebenfalls **PASS**:72 vollständige
  Saves,72 Snapshots und Vertrag unabhängig wörtlich bytegleich;167/165/zwei
  bestehende Konflikte,1485 Policyfälle und elf Rampen-/zehn Bridge-Restartpaare
  unverändert. **OE-16 technisch PASS**, OE-17 TODO/Gate C TODO,
  native/human NOT_RUN. Verbrauch, Evolution und Originalart werden nicht als
  bereits implementiert geführt; ihre weiteren Aufgaben folgen separat.

### OE-15: physischer Markettransport, Entwicklungsrevision15

- **Authority:** Rule10/Schema26/policy3, Revision15. Bezahlte Warehouses und
  Markets verwenden die vorhandenen Rollen und denselben Navigator für
  Warehouse→Market→House. Nur die aktuell benötigte Pottery-Distribution wird
  angelegt. Typisiertes Ledger-Cargo ist die einzige Mengenauthority;
  `CourierState.cargo/reserved` bleiben für diese Actoren0. Anzeige und
  Walkerzustand lesen die tatsächliche typisierte Menge.
- **Authored Provisionierung:** Die frühen originalen Hausmodellzeilen haben
  keinen Ceramicsbedarf. Unveränderte Originalbedarfe bleiben0. Belegte
  bewohnte, lieferberechtigte Häuser erhalten einen ausdrücklich authored
  physischen Vorratspuffer von8 Einheiten, um den bezahlten Lieferweg vor
  Evolution und Verbrauch zu integrieren. Das ist keine Behauptung eines
  originalen Shelterverbrauchs; Verbrauch folgt OE-17.
- **Begrenzung und Sicherheit:** Je typisiertem Courier bleibt genau die letzte
  Transferzeile als Sequenz-Highwater erhalten, einschließlich terminalem
  Zustand. Der Nachfolger entsteht vor dem Entfernen der alten Zeile.
  Stock plus ausgehendes Cargo als Rückkehrplatz plus eingehende Reservierungen
  bleibt innerhalb der physischen Kapazität. Masse zählt Cargo nur einmal.
  Reguläres Pausieren bricht beladene Ausfahrten ab und erhält begonnene Kanten
  bis zum nächsten Vertex; Rückkehr bucht tatsächliche Ware am Owner.
- **Abnahme:** Echte Teilbeladung (etwa1 von4 Einheiten) und vollständige
  Ankünfte werden geprüft. Die reservierte Zielkapazität verhindert in diesem
  Entwicklungsstand eine schrumpfende Teilankunft; diese wird nicht durch
  Testinjektion hergestellt. Der unveränderte reine Transaktionskern prüft
  Teilankünfte separat. Strikte Restorebindung umfasst Owner, Actor, Ziel,
  Phase, echte Route, Menge und Sequenz. Der unabhängige Review schließt drei
  unerreichbare World-Restores: abweichende reservierte/beladene Menge,
  Teilgutschrift vor Ankunft und gelöschte Sequenzhistorie eines benutzten Actors.
  Je45 Konfigurationstests, echte bezahlte Hausankunft, zwei separate20k-
  Prozessreplays und drei74-Fall-Forkvergleiche bestanden; vollständige Belege
  im [Fortschritt](PROGRESS.md). Eigener Defaultsave `rule10-revision15.json`;
  Entwicklungsrevision14 wird atomar verweigert.
- **Abschluss:** Technischer OE-15-PASS mit unabhängig identischen72 alten
  Saves/72 Snapshots und1485 Policyfällen. Originaldateien unverändert,
  Gate C bis OE-19 offen; native/human NOT_RUN. Unmittelbar weiter mit OE-16.

### OE-14–33: zusammenhängende autonome Umsetzung beauftragt

- **Nutzerauftrag vom 2026-10-17:** alle offenen Aufgaben der Reihe nach
  bearbeiten, ohne auf seine Playtests zu warten; am Ende echte Sprites und
  echte Objekte integriert. Dies autorisiert die aktiven Gameplaypfade in
  einem neuen Nachfolger, einschließlich eines regulären bezahlten Bewohner-/
  Worker-Bootstraps. Alte1–9/1–25-Verträge und Provenance bleiben erhalten.
- **Entscheidung vor Implementierung:**
  [Ausführungsvertrag](../original-economy/completion-contract.md), Rule10/
  Schema26/policy3 mit strikt gespeicherter Entwicklungsrevision, zuerst14.
  Neue authored Startfinanzierung12000; Waren/Bewohner anfangs0, tatsächlicher
  Straßen-Zuzug, modellgebundene Shelterkapazität und Worker aus Bewohnern.
  Einmalige Pottery→Ceramics-Umwandlung bei tatsächlicher Warehouse-Ankunft,
  getrennte überprüfte Vor-/Nachübergabebilanz, kein doppelter Stock.
- **Sequenz und Git:** ein getesteter Task-Commit je OE-Aufgabe, danach direkt
  die nächste offene Aufgabe. Der neue Stand bleibt während der Entwicklung
  auf dem Feature-Branch; gemeinsamer geprüfter PR/regulärer Merge am Ende.
  Keine Tags/Releases. Human Acceptance bleibt ohne menschlichen Test NOT_RUN
  und verhindert keine automatisierte technische Abnahme.
- **Abschluss:** Echte lokale Originalassets und belegte Objektidentitäten
  gehören zum Gesamtziel. Deklarierte Fallbacks bleiben ehrliche Diagnosen,
  erfüllen die vollständige Originalsprite-Abschlussforderung aber nicht.
  Status und echte Tests werden nach jedem einzelnen Schritt ergänzt.

### OE-14: bezahlter Ceramics-Ursprung technisch abgenommen

- **Authority:** Ausschließlich Rule10/Schema26/Revision14; die vorhandenen
  Clay-/Pottery-Rezepte laufen über die gemeinsame unverändert herausgezogene
  Implementierung, reguläres Staffing und dieselbe echte Courierbewegung.
  Vier bezahlte, straßenverbundene Häuser liefern durch laufenden authored
  Zuzug echte OriginalHouseState-Bewohner/Worker; Legacy-Population bleibt0.
- **Masse:** Am tatsächlichen Warehouse-Arrival wird Pottery einmalig in
  typisierte Ceramics konvertiert. Neues Ledger besitzt ausschließlich den
  konvertierten Vorrat, älterer Warehouse-Slot bleibt0. Übergabehistorie und
  ursprüngliche Clay-/Pottery-Bilanz werden gemeinsam validiert. Keine
  House-Goodslieferung, Services, Food, Konsum oder Taxes vor ihren Aufgaben.
- **Tests/Review:** Je43 relevante CTests in Debug/Release/ASan+UBSan,
  795 neue strikte Saveverweigerungen, echter Mid-courier-F5/F9, zwei separate
  20k-Tick-Prozessfortsetzungen und drei74-Fall-Forkvergleiche bestanden.
  Ein vom unabhängigen Review gefundener unerreichbarer ungerader
  Bewohnerzwischenstand ist geschlossen; legitime ungerade Modellendkapazität
  bleibt positiv geprüft. Der vollständige Legacyvergleich bleibt exakt;
  Originaldateien unverändert. Befehle, Hashes und Aussagegrenzen stehen
  im [Fortschritt](PROGRESS.md#oe-14-umsetzung--2026-10-17-basis41091b66).
- **Abschlussgrenze:** OE-14 PASS für tatsächlichen bezahlten Ursprung;
  Gate C bis OE-19 offen. Originalgrafikkomposition bleibt explizites späteres
  Gesamtziel, keine vorgezogene Artparität. Native/human NOT_RUN. Nach Commit
  unmittelbar OE-15, ohne Wartepunkt für Nutzerplaytests.

### OE-13: Warenvertrag V1 ausdrücklich zur Umsetzung beauftragt

- **Nutzerfreigabe:** Auf die konkrete Frage „Soll ich den geprüften
  OE-13-Vertrag umsetzen: Rule9/Schema25, alte Verträge unverändert, positive
  Transaktionstests synthetisch und echte World zunächst mit0/unsupported?
  Reale Produktion und Lieferungen bleiben OE-14/15 zugeordnet.“ antwortet
  der Nutzer ausdrücklich „Ja, diesen Vertrag umsetzen (empfohlen)“.
  Damit sind genau die Versionsentscheidung und die gestufte Abnahme des
  [Warenvertrags V1](../original-economy/goods-ledger-v1-proposal.md) angenommen.
- **Beginn:** sauberer, frisch live/origin bestätigter `main`
  `af660b72c281e5c1b8735a29a941f474a3f33d8d`, Feature per Fast-forward
  angeglichen. Vertragsdokumentation
  `aaecabd638359a48d513f2d5146457fdd1c9e67a` aus PR20 bleibt historischer
  Entscheidungsstand. Rule9 und Schema25 vor Umsetzung weiterhin frei.
- **Umfang:** Eigener expliziter Entwicklernachfolger von Rule8, policy3
  unverändert;2000 Funds, bezahlte Vacant-Häuser/Well72 und Infrastruktur
  unverändert. Sechs Originalwaren, ein reiner vollständiger atomarer
  Transaktionskern und lesende Bilanz. Echte Worldzustände ausschließlich
  unsupported/0; keine neue Warenquelle, Population, Worker, Produktion,
  Courierrolle, Import-, Verbrauchs- oder Gratis-Credit-Authority.
- **Save/Tests:** Eigener Schema25-/Restorepfad; Rule1–8 und Schemas1–24
  bleiben exakt. HouseState ist einzige Hausbestandsauthority; Reservations
  keine zusätzliche Masse. Positive numerische Transaktionsfälle sind
  synthetisch, echte bezahlte20-Haus-/Well-/F5F9-/Prozessnachweise bleiben
  bei erlaubten Nullbeständen. Eigenständige eingefrorene7/23- und8/24-
  Referenzen ergänzen OE-00. Kein voller Warenkreislauf- oder Gate-C-PASS.
- **Umsetzung und Review:** Eigene reine Ledgerlibrary mit vollständigen
  Kandidatentransaktionen und checked Summen; sechs existierende Hausfelder
  bleiben alleinige Hausmengenauthority. Worldqueries verwenden begrenzte
  const-Spans/Stack-Kontexte, keinen World-/Bestandsklon. Transfersequenz plus
  begrenzte terminale Historie verhindern stale/doppelte Buchungen; höchstens
  46 Owner/26 Courier/26 Historienzeilen im authored numerischen Kontext,
  keine realen Warenactoren. Schema25 validiert zusätzlich den zulässigen
  Ursprung, sodass auch ausgeglichene positive Fälschungen verweigert werden.
  Echte Rule9-Zustände bleiben exakt Nullkonten und leere Warenlisten.
- **Abnahme:** Je39 relevante CTests in Debug/Release/ASan+UBSan, positive
  reine Transaktionen,575 Saveverweigerungen, echte bezahlte20-Haus-Szene,
  tatsächliches F5/F9 und zwei separate40k-Prozessfortsetzungen bestanden.
  Drei74-Fall-Forkvergleiche, eingefrorene7/23-/8/24-Bytes und vollständiger
  OE-00-Corpusvergleich sichern die alten Verträge. Einzelne anfänglich
  falsche Symlink-Testannahme korrigiert, keine Produktionslockerung und
  keine entfernte Negativprüfung; Details im Fortschrittsprotokoll.
  Unabhängige Authority-/Persistenz-/Prozessreviews ohne blockierenden Befund.
- **Status:** OE-13 **Buchhaltungsfundament technisch PASS**, kein
  Warenkreislauf- oder Gate-C-PASS. Der frühere BLOCKED-Befund unten bleibt
  historisch. OE-14 bleibt TODO; Bewohner-/Workforce-/Producer-/Arrival-
  Verträge werden durch diese Freigabe nicht vorweggenommen. Software-SDL
  ist geprüft; native/human NOT_RUN. Tatsächliche Belege und abschließende
  Source-/Originalhashes stehen im Fortschrittsregister.

### OE-13: Versions- und Abnahmeentscheidung erforderlich

- **Auftrag und Revision:** Nutzer setzt nach OE-12/Gate-B-PASS mit „weiter
  gehts“ fort. Sauberer, live/origin bestätigter `main`
  `bf4e821a591f141b0a300c848b4ecbdbe3e40fe8`; Feature-Branch per Fast-forward
  angepasst. Voriger Taskcommit `ec3a2d60c7541f8a11338c80d5398ce54e51f3a8`.
- **Konflikt:** OE-13 verlangt noch ausschließlich Rule7. Der angenommene
  OE-12-Vertrag friert Rule7/Schema23 ausdrücklich ein; sein eigener
  Rule8-/Schema24-Container liefert nur Einfluss und erlaubt keine Goods,
  Worker oder Couriers. Auch neue gültige Nichtnullbestände bei unveränderten
  JSON-Keys wären eine Änderung dieser Restore-/Laufzeitverträge.
- **Tatsächlicher Stand:** Beide Worldpfade akzeptieren ausschließlich
  Vacant-Nullzustände und leere Couriers; Tick ist nur Scheduler. Frische Probe
  mit unveränderten Produktionsarchiven kauft reguläre Container, verweigert
  Clay/Pottery/Warehouse/Market atomisch, verweigert manipulierte Bestände und
  Produktionszähler und behält nach je20k Ticks null Waren/Worker/Couriers.
  Vacant/occupants0 scheitert außerdem am puren Liefergate. Kein tatsächlicher
  Warenursprung durch Enum-/Ledgerfelder oder bloß neue Versionsnummer.
- **Vorschlag, NICHT ANGENOMMEN:** Der
  [Warenbuchhaltungsvertrag V1](../original-economy/goods-ledger-v1-proposal.md)
  schlägt einen expliziten Rule9-/Schema25-Entwicklernachfolger bei policy3 vor.
  Bestehende Regeln1–8 und Saveverträge1–24 bleiben unverändert. Eine
  zusätzliche ausdrückliche Abnahmeentscheidung trennt positive synthetische
  Transaktions-/Reservationstests von echten, zunächst ausschließlich leeren
  World-/Savezuständen mit0/unsupported. Produktion14, Transport15 und
  Tickkonsum17 werden dadurch nicht vorgezogen oder automatisch freigegeben.
- **Single Authority und Sicherheit:** Sechs Original-Hausbestände bleiben in
  HouseState; keine zweite Mengenquelle oder automatische Zuordnung zu alten
  Good-Enums. Reservations zählen nicht als Material. Keine öffentliche
  Stock-/Production-/Import-Credit-API, keine Save-aktivierbare Capability,
  Gratisworker, geänderter HouseType oder gelockerte Herkunftsprüfung.
  Selbst `produced=stock=1` wäre ohne realen zugelassenen Ursprung kein
  zulässiger Worldzustand. Bewohner-/Workforce-Bootstrap für OE-14/15 bleibt
  ein gesonderter offener Vertrag.
- **Ergebnis und Git:** OE-13 **BLOCKED**, OE-14 **TODO**, Gate B bleibt **PASS**.
  Der vorgeschlagene gestufte Grundlagenumfang ist noch nicht umgesetzt oder
  angenommen. Keine Produktions-/Schema-/Prompt-/Gateänderung; geprüfte
  Dokumentation darf nach bestehendem Handoff als solche committed/gemergt
  werden. Kein grüner Featurecommit und keine Aktivierung des Nachfolgers.

### OE-12: strenge Grenze und blockierter Fortgang

- **Revision und Nutzerentscheidung:** Start am sauberen, live/origin bestätigten
  `main` `df293931bbb8a409668f3790379bb370e6c9f44d`, Feature-Branch per
  Fast-forward angeglichen. OE-11 ist Commit
  `b3b15a6ec98912a35982d9bcec30d7a651fed0a3`. Der Nutzer hat auf die konkrete
  Rückfrage ausdrücklich gewählt: „Die strenge Grenze beibehalten und OE-12
  mit diesen Blockern dokumentieren“. Keine Erweiterung von Bau-/Restore-
  Freigaben, Funds, Modellidentitäten oder bestehenden Regelverträgen.
- **Identität:** Alle sieben `functional_model_candidate`-Einträge bleiben
  `functional=Inferred` und `mechanics_identity=Unknown`. Verifizierte
  Modell-ID/Label-Paare und die getrennte Well-/Herbalist-Grafikevidenz belegen
  keine World-Quellenidentität. Nach der ausdrücklichen OE-12-Stop-Regel dürfen
  diese Kandidaten nicht durch Typnamen zu Originaleinflussquellen werden.
- **Tatsächliche Authority:** Rule7 erlaubt bislang ausschließlich Infrastruktur
  und bezahlte Vacant-Hauscontainer. Alle sieben Source-Baubefehle und der
  Abrissbefehl werden vor Mutation verweigert; Restore verlangt weiterhin
  genau die zugeordneten House-Nullzustände. Sources oder Entfernung nur für
  einen grünen Cachetest freizuschalten wäre eine zusätzliche Vertragsänderung.
- **Finanzierung:** Das bestehende Budget1300 trägt bei authored Hauskosten80
  höchstens16 Häuser, ohne weitere Straßen/Quellen. Zwanzig kosten mindestens
  1600. Der reine Scheduler erzeugt keine Steuern; auch Abriss ist kein
  verfügbarer Finanzierungspfad. Keine Extra-Funds, Snapshot-Injektion,
  hypothetischen Bewohner oder Gratisquellen zur Erfüllung der Pflichtszene.
- **Konsequenz:** OE-12 ist BLOCKED und nicht implementiert. Ein leerer Cache
  mit ausschließlich übersprungenen Kandidaten erfüllt keine Originaleinfluss-
  Abnahme. Kein grüner Featurecommit, Gate-B-PASS oder Fortgang zu OE-13.
  Der gemeinsame numerische Kern und die unveränderliche Modellbindung bleiben
  unberührt. Ein etwaiger Commit dieses Befunds ist ausschließlich Dokumentation
  des BLOCKED-Status, keine Featurefreigabe.
- **Belege und Fortsetzung:** Die frische, unveränderte Release-World-Probe
  reproduziert alle Gates mit echten Befehlen und echtem Modelldatei-Load.
  Separate Identitäts-/Source-/Finanzvoraussetzungen und genaue Repro-Befehle
  stehen im [Blockerbericht](../original-economy/desirability-integration-blockers.md)
  und im Fortschrittsregister. Keine Aussage, dass Originalbelege prinzipiell
  unmöglich wären; diese Umsetzung hat den fehlenden Vertrag nicht erfunden.

### OE-12 Fortsetzung: Originalquellenpfad belegt, Vertragsgrenze unverändert

- **Revision:** sauberer `main` nach dem BLOCKED-Dokumentationsmerge,
  `23f544a3611394f041c7401974e67fd87b0c2f1d`; Dokumentationscommit
  `1ea204ecc4d8d659ee84f44a8e51995c02bd20b9`. „Weiter gehts“ setzt die
  ausdrücklich gewählte strenge Grenze nicht außer Kraft. Diese Fortsetzung
  untersucht daher nur die fehlenden Originalbelege, ohne Laufzeitfreigabe.
- **`exe`, statisch:** Der gepinnte Managerpfad prüft Status1/3 und Nullwort
  runtime+3c, liest Felder1–4 über den tatsächlichen signed Originaltyp+14 und
  nutzt im nachgewiesenen Basis-/Industrial-Quadratpfad die virtuelle Seite
  aus runtime+07. Constructor-/Vtable-/Initializerbelege schließen hier die
  bisherige Geometrielücke. Die Bedeutung/Archivzuordnung von+3c bleibt offen;
  keine Gleichsetzung mit dem bisherigen `subindex`, keine Zulassung aller
  Originaltypen oder Sonderformen. Details im
  [Quellenvertrag](../reverse/original-desirability-source-admission.md).
- **Vorhandene Hausbindung:** Der bezahlte Vacantcontainer hat zwar eine
  ausdrückliche Originaltyp2-Referenz, aber Status0. Dieser Zustand scheitert
  am beobachteten ersten Quellengate. Der separate Evolution-Alias auf die
  Shelterzeile liefert keine Emittentenzulassung. Kein Status wird hochgesetzt.
- **Konsequenz:** Originaltyp-/Footprint-/Spread-Belege bleiben von der
  authored World-Zuordnung getrennt. Sourcebau, Entfernung, Restore und die
  nicht finanzierbare20-Häuser-Szene erfordern weiterhin ausdrücklich
  beauftragte Verträge; weitere RE allein öffnet diese Gates nicht. Keine
  Implementierung oder Budget-/Stateänderung, OE-12/Gate B weiterhin BLOCKED,
  kein Fortgang zu OE-13. Nur Forschung und Dokumentation.

### OE-12 Fortsetzung: separates archiviertes Quellenwort und Managerlinks

- **Revision:** sauberer, live bestätigter `main`
  `e29ce8a20d13013fce51994175c4bd04b86f1ed4`; voriger Forschungscommit
  `c50ff69941c60c61d7d37c69354e3113bdc7f58c` aus PR14. Feature-Branch per
  Fast-forward angeglichen; die gewählte strenge Grenze bleibt verbindlich.
- **`exe`, statisch:** Runtime-WORD+3c ist bei raw Base54 archiviert, separat
  von Subindex runtime+16/Base18. Writer und Schema3/4/5-Reader bestätigen
  dieselbe Folge der32 Feldtransfers bis+3c. Zwei unabhängige Prüfungen und
  die Schema-/Archivprimitive sichern diese Zuordnung ab. Keine Parseränderung.
- **Zustand und Links:** Reset nullt+3c anfangs; eine bestimmte Slotersetzung
  bewahrt+3c/+3e über Konstruktion/Initializer hinweg. Ein konkreter Erzeuger
  schreibt die ursprüngliche Manager-ID in neues+3c, während+3e weitere IDs
  verkettet. Unabhängige Leser folgen+3c als signed Managerindex. Das belegt
  diese Linkrolle, keinen vollständigen Parent-/Lifecycle-/Quellenvertrag.
  Siehe [Zustandsbeleg](../reverse/original-desirability-source-state.md).
- **Konsequenz:** Das beobachtete zusätzliche Nullgate ist präziser erklärt,
  aber keine World-Instanz dadurch zugelassen. Keine Änderung an Originaldaten,
  Quellenidentitäten, Status, Sourcebau, Abriss, Restore, Funds oder Schema23.
  OE-12/Gate B bleiben BLOCKED; keine OE-13-Freigabe. Nur geprüfte Forschung
  und Dokumentation, keine erneute unveränderte Bau-/Budgetprobe als Fortschritt.

### OE-12 Fortsetzung: konkrete Originalfabriken und abweichende Geometrie

- **Revision:** sauberer, live/origin bestätigter `main`
  `837caf8e48272fa48172c3cb7b815a95eda8883e`; vorige Zustandsforschung ist
  Dokumentationscommit `5f30abc9b053ad700199604b4cf0c320dd5a5336` aus PR15.
  Feature-Branch per Fast-forward angeglichen; strenge Nutzergrenze unverändert.
- **`exe`:** Für35/43/54/59/72/193/207 tatsächliche Klassifikation, priorisierte
  Factorydispatches, konkrete Konstruktoren und finale Vtable-Slots geprüft.
  Warehouse54 nutzt im Quadratpfad einen eigenen konstant3 liefernden Callback;
  Farm193 den runtime-Sidegetter mit initialem Lookup-Default3. Markt59 wählt
  einen gesonderten Formpfad; Initialside1 beweist keinen1×1-Einflussabdruck.
  Nicht sämtliche transitive Initialisierungs-/Lifecyclepfade rekonstruiert.
- **Read-only Worldvergleich:** Sieben Aufrufe der unveränderten produktiven
  Footprintfunktion, ohne World oder Platzierung. Warehouse hat dort2×2,
  Farm1×1; vier andere Defaults stimmen dimensional überein, beweisen jedoch
  keine Identität. Alle sieben `Inferred`/`Unknown`-Zuordnungen bleiben bestehen.
  Details und Repro im [Factorybeleg](../reverse/original-desirability-source-factories.md).
- **Konsequenz:** Keine Footprint-/Parser-/World-/Save-/Status-/Bau-/Restore-
  oder Budgetänderung. Originalfabrikbelege ersetzen keine authored World-
  Identitätsentscheidung. OE-12/Gate B weiterhin BLOCKED, kein OE-13-Fortgang.

### OE-12 Fortsetzung: bedingte Markt-Bounds und eigene Rechteckarithmetik

- **Revision:** sauberer, live/origin bestätigter `main`
  `ff3953cb3115c05fdbe00d7f5c0001d120eaae2d`; voriger Factory-Forschungscommit
  `c3234ae4c397a84aaad533d3ccc63a90d6755d73` aus PR16. Feature-Branch per
  Fast-forward angeglichen, strenge Nutzergrenze beibehalten.
- **`exe`:** Markt59 besitzt einen separaten Descriptor. Dessen zwei geprüfte
  Selectorblöcke ergeben inklusive4×7/7×4-Einfluss-Boundingrechtecke; Position
  und Selector stammen aus zusätzlichem Runtime-/NonHouseInfo-Zustand. Dessen
  bytegroßer Selector+60 liegt bei Raw-Info94. Keine physische Flächenfreigabe,
  keine universelle Selector0/1- oder Load-/Lifecycle-Garantie abgeleitet.
- **Numerik:** Der getrennte Rechteckwrapper beginnt den Periodenzähler bei1,
  hat keinen Initial-DES0-Return und keine Vorzeichenabsorption. Vier neue
  synthetische Vergleiche gegen den unveränderten, frisch privat kompilierten
  Quadratkern bestätigen die Unterschiede. Kein Originalcode ausgeführt;
  Rechteck-/Wall-/Clipping-/Overflowparität damit nicht behauptet.
- **Konsequenz:** Der historische OE-03-PASS bleibt auf seinen Quadratvertrag
  begrenzt. Keine implizite Rechteck-Coreerweiterung, World-Zuordnung, physische
  Footprint-, Bau-/Restore-/Save-/Status- oder Fundsänderung. OE-12/Gate B
  BLOCKED, kein OE-13. [Beleg und Repro](../reverse/original-market-desirability-shape.md).

### OE-12: beauftragter Vertragsentwurf, noch keine Freigabe

- **Revision:** sauberer, live/origin bestätigter `main`
  `4fd586851e1f019dbeb9b9894e7437fd9b6d9e2c`; Feature-Branch per Fast-forward
  angeglichen. Letzter Forschungscommit `da94833b46c130514d24fa172dacad4a56e46e11`.
- **Neuer ausdrücklicher Auftrag:** „Zuerst einen konkreten Rule‑7-Quellen-,
  Bau-, Restore- und Finanzierungsvertrag zur Entscheidung ausarbeiten.“
  Das beauftragt den [konkreten Entwurf](../original-economy/source-contract-v1-proposal.md),
  keine Umsetzung oder Annahme der vorgeschlagenen Ausnahmen.
- **Vorschlag, NICHT ANGENOMMEN:** ausdrücklicher Nachfolger Rule8/Schema24
  bei policy3; eigener authored Well72-Quellencontainer2×2 mit Preis60/Limit4,
  atomarer Kauf/Abriss und eigenem bounded Entfernungsledger; fixes neues
  Startbudget2000 ohne Einnahmen/Refund. Rule7/Schema23 bleiben exakt.
  Die sieben bisherigen `Inferred`/`Unknown`-Analogien bleiben unverändert.
- **Zusätzliche Abnahmeentscheidung:** Unverändertes City-v13 verweigert
  Wells und finanziert die20 Häuser nicht aus seinem Startbudget. Der Entwurf
  schlägt daher ausdrücklich einen Vergleich seines unveränderten Algorithmus
  auf der real bezahlten neuen Szene vor, keinen identischen City-v13-Worldlauf.
  Auch dieser geänderte Vergleich benötigt die Zustimmung des Nutzers.
- **Review und Konsequenz:** Layout/Finanzarithmetik, Isolation, Ledger/Restore
  und Zustimmungspflicht unabhängig read-only geprüft. Die vorgeschlagenen
  Werte sind authored, keine Originalkauf-/Lifecyclebehauptung. Dokumentation
  darf nach bestehendem Handoff gemergt werden; das genehmigt keine Regeln.
  OE-12/Gate B weiterhin **BLOCKED**, OE-13 **TODO**. Prompts/Quality-Gates
  werden vor der Entscheidung nicht umgeschrieben; kein Produktionscode geändert.

### OE-12: Quellenvertrag V1 zur Umsetzung beauftragt

- **Nutzerfreigabe:** Auf die konkrete Frage „Gibst du dieses Paket zur
  Umsetzung frei?“ nach Darstellung aller vier Änderungen antwortet der Nutzer
  direkt „weiter gehts“. Diese Antwort wird im unmittelbaren Fragekontext als
  Zustimmung zum vorgelegten Paket verstanden, nicht als rückwirkender
  allgemeiner Waiver der früheren strengen Grenze.
- **Vertrag:** [Quellenvertrag V1](../original-economy/source-contract-v1-proposal.md):
  ausdrücklicher Entwickler-Nachfolger City-v16 Rule8/Schema24, policy3 exakt;
  eigener authored Well72-Einflusscontainer2×2, Kauf60/max4, sicherer Abriss ohne
  Refund und eigenes typisiertes Entfernungsledger; fixes Startbudget2000;
  Vergleich mit unverändertem City-v13-Algorithmus auf der bezahlten Szene.
  Kein vorgetäuschter identischer City-v13-Worldlauf. Rule7/Schema23 und alle
  älteren Regeln/Savebytes bleiben exakt; keine automatische Migration.
- **Beginn:** sauberer live/origin `main`
  `1be1863b643233781865158510b8ceb93e7c63e3`, Feature per Fast-forward angepasst.
  Dokumentationscommit des Entwurfs `d62da85f2b1b3158f1e46b603d3ab7b10a7d7052`.
  OE-12 begann IN_PROGRESS. Die abgeschlossene technische Abnahme ist unten
  vermerkt; OE-13 bleibt TODO. Annahme des Vertrags ersetzt keine Originalbelege.

### OE-12: begrenzte Umsetzung und technische Abnahme

- **`authored` / Runtime:** Rule8/Schema24 implementiert genau den angenommenen
  Vertrag. Neuer Objekttyp ist angehängt; Well72-Typbindung,2×2-Fläche,
  Preis60/max4,2000 Startfunds, eigener Quellen-/Entfernungsledger und atomare
  Kandidatenveröffentlichung bleiben von den alten Regeln getrennt. Keine
  zusätzlichen Originalzustände oder sieben bisherigen Analogien zugelassen.
- **`model` / read-only:** Das unveränderliche Sessionmodell liefert nur die
  vorbereiteten Well72-Zahlen. Bestehender Quadratkern bleibt bytegleich;
  maximal vier Quellen werden nach BuildingId verarbeitet. Cache/Prefixmaps
  entstehen ausschließlich bei Create/Restore und erfolgreichem Quellenkauf
  oder -abriss. Haus-/Frame-/Tickabfragen lösen keine Vorbereitung aus.
- **Save und UI:** Schema24 prüft Modell-/Map-/Policybindung, lückenlose
  gemeinsame BuildingIds und vollständige Kosten-/Quellenhistorie. Alte
  Schemas1–23 bleiben exakt. Nur der explizite ausgeschlossene Launcher öffnet
  die Ansicht; normaler App-/Menü-/Recoverypfad bleibt geschlossen. Well-Grafik
  ist Präsentation, Wasser/Worker/Goods/Service/Einkommen bleiben neutral.
- **Vergleich und Review:** City-v13-Algorithmus liest dieselbe bezahlte Szene
  mit ausdrücklich authored Well72→Well-Referenz; keine illegale City-v13-World.
  Simulation, Save/UI/Launcher und Szenenfinanzen unabhängig reviewed. Der
  Rule8-only Guard am letzten Command-Counterwert gilt auch für Road-Batches;
  atomare Ablehnung und erfolgreich restaurierbarer letzter Einzelkauf geprüft.
- **Ergebnis:** OE-12 und Gate B **PASS**, begrenzt auf diesen Entwicklervertrag.
  Vollständige Suiten plus finale gezielte Wiederholungen belegen181 verschiedene
  Tests je Debug/Release/ASan+UBSan, ohne Skip. Bezahlte20-Haus-Szene, zwei
  Prozessreplays,74 Fork-Vergleiche je Konfiguration und exakter72-Save-/1485-
  Policy-Legacyvergleich stehen im
  [Testprotokoll](PROGRESS.md#oe-12-umsetzung--2026-10-16-basis1be1863--gate-b-pass).
  Originaldateien unverändert; keine allgemeine Originalgebäude-/Lifecycle-
  oder Rechteckparität. Native/human **NOT_RUN**, OE-13 **TODO**.

### OE-11: unveränderliche und explizite Modellabhängigkeit pro Session

- **Revision:** Beginn am sauberen, live/origin bestätigten `main`
  `d6eb736e1dbf0f08c26b933d3beb4c132ee5192a`; vorhandener Feature-Branch per
  Fast-forward angeglichen. OE-10 ist Commit
  `b4c22e6ecc49558e137288801d2e2a866cd8fc44`. Eigene SHA folgt im Abschluss
  und beim nächsten Prompt; der delegierte Commit-/PR-/Merge-Handoff gilt.
- **`model` / read-only integration:** Eine erfolgreiche Datei-Vorbereitung baut
  genau ein `shared_ptr<const OriginalRulesModel>` mit geprüfter Provenance,
  Difficulty und sämtlichen benannten sieben Building-/15 House-Zeilen. Die
  vorhandenen typisierten Leser und geprüften Rechenregeln bleiben zuständig;
  Rohdaten werden danach nicht benötigt. Kein globaler Cache, Singleton oder
  `data_root` in World. Dieselben unveränderlichen Daten dürfen World-Kopien
  teilen; unabhängige Vorbereitungen erhalten unabhängige Instanzen.
- **Create/Restore:** Die explizite Entwicklungsfactory und der private
  Konstruktor erhalten die Modellinstanz als Abhängigkeit. Save-Quelle wird
  daraus abgeleitet. Ein gebundener Snapshot verlangt exakt Format/SHA/Bytezahl/
  Difficulty derselben Modellinterpretation. Ungebundene reine Infrastruktur-
  und Clock-Fixtures bleiben für ältere Tests möglich und unspeicherbar;
  ein Restore darf sie nicht stillschweigend nachbinden. Normale App, Menü,
  Recovery und reguläre Rule7-Konstruktion bleiben geschlossen.
- **F5/F9:** F5 trägt die Instanz aus der aktiven World transient im SaveDocument
  weiter; die bounded Live-Dateiprüfung hasht ausschließlich und parst nicht
  erneut. F9 bereitet genau einen neuen Kandidaten vor und veröffentlicht ihn
  nach vollständiger Source-/Map-/Policy-/World-Prüfung. Fehlende, manipulierte
  oder typisiert ungültige Dateien verweigern den Vorgang vor Veröffentlichung.
  Bestehende Sessions behalten ihre Werte und können ohne Quelldatei weiter
  ticken. Dateiänderungen werden weder automatisch übernommen noch in Saves
  als neue Quelle umgedeutet. Die Alternative erneuten Parsens bei jedem F5
  wurde zugunsten der explizit gebundenen Instanz verworfen.
- **Save-Vertrag:** Keine JSON-Felder oder Versionsänderung. Das neue
  `SaveDocument::prepared_original_rules_model` ist ausschließlich transient,
  ähnlich vorhandenen vorbereiteten MapPermissions; Pointer/Pfad/Tabellen
  werden nie serialisiert. Schemas1–22 erhalten keine Modellanforderung.
  Schema23 mit syntaktisch gültiger, aber semantisch ungeeigneter Modelldatei
  wird jetzt bewusst verweigert: Hashgleichheit allein ist keine Runtimefreigabe.
- **`authored` Testgrenze:** Die alte OE-09-Testquelle verwendete freie Labels
  und nur einen Building-Eintrag. Ihre ursprünglichen6778 Bytes/SHA
  `9f2392de51302cb7451605eb5c2b7e8aa15cf827801c00cb1b8c21aebf967e6e`
  werden weiterhin unabhängig rekonstruiert und geprüft, nun aber als negative
  Runtimekontrolle. Eine neue vollständig selbst verfasste Fixture mit gültigen
  Modellbezeichnern wurde zusätzlich mit unverändertem alten OE-09-Writer,
  Headern und Libraries ausgeführt. Dessen6778 Bytes/SHA
  `4e70a04b19700c86d974d53df4edb2fdcf3bb264622ef76c4c32a1be5b63ca68`
  sind der separate positive Bytevertrag; kein rückwirkender Golden-Reset.
- **Review/Grenzen:** Unabhängige Read-only-Reviews ohne funktionalen Produktionsbefund;
  ein Test-Skipguard für defekte Symlinks wurde anschließend eng korrigiert und
  in allen Konfigurationen separat nachgeprüft.
  Tatsächliche Konfigurations-, F5/F9-, Prozess-, Source- und Legacyresultate
  stehen im Fortschrittsregister. Die Bindung aktiviert weder Desirability,
  Waren, Bewohner noch Evolution. Gate B bleibt bis OE-12 offen. Kein
  menschlicher/native Gameplay-PASS und keine neue Original-Paritätsbehauptung.
  Details im [Sessionvertrag](../original-economy/model-session-binding.md).

### OE-10: deterministischer Scheduler mit ausdrücklich authored Zeitbasis

- **Revision:** Start am sauberen `main`
  `5ed833f4b0e97ac6a9e327f43e95a155732a9b6e`. OE-09 bleibt eigener Commit
  `a1a352bd697e8a424fb6d76875782d84d70b930a`; eigene OE-10-SHA folgt im
  Abschluss und beim nächsten Prompt. Der delegierte Commit-/PR-/Merge-Handoff
  gilt weiter, ohne Tag oder Release.
- **Evidenz `exe`:** Der gepinnte Originalscheduler zählt Phase 0..50, ruft
  Countdown bei 35 und Evolution bei 39 auf und verbraucht danach nur bei
  Selector 0/7. Die ergänzte begrenzte Untersuchung verbindet denselben Selector
  unabhängig mit einem 16er-Passcounter und einem weiteren 12er-Counter.
  Der getrennte Original-Jahrescounter steigt bei größerem Counterwert 1,
  nicht automatisch beim Wrap 11→0. Vollständiges Originaldatum, Epoche,
  Spielgeschwindigkeit und übrige Schedulerarbeit bleiben offen.
- **Entscheidung `authored`:** `scheduler_mapping_version=1` legt einen World-Tick
  pro Phase fest; die vorhandenen 20 Hz werden nicht als Originalgeschwindigkeit
  ausgegeben. Der gezählte Kalender verwendet Epoche 0 und vollständig
  abgelaufene Jahre. Diese Interpretation ist fest an Rule7/Schema23 gebunden;
  eine Änderung benötigt eine neue gespeicherte Regelidentität. Eine unversionierte
  Gleichsetzung mit Originalzeit und ein atomarer Fork-`step_house` wurden
  verworfen, weil sie ungeprüfte Geschwindigkeit und die Zwischenpositionen
  zwischen Service-Countdown und Evolution verdecken würden.
- **Minimale Save-Authority:** Ausschließlich vorhandene `WorldSnapshot::ticks`
  zählen abgeschlossene Phasenschritte. Division/Modulo ergeben den nächsten
  Dispatch, initial Phase 0; zusätzliche JSON-Clock-/Phase-/Calendar-/Versionskeys
  werden nicht eingeführt. Keine Migration und keine neue Deutung alter Profile.
  Die House-`cycle_phase` bleibt bis zu ihrer späteren Verarbeitung reserviert 0;
  nachträglich bezahlte Vacant-Container speichern ihren tatsächlichen
  `placed_tick <= ticks`. Die Alternative separater frei änderbarer Calendar-
  und House-Counter wurde wegen redundanter Authority verworfen.
- **Grenzen:** `UINT64_MAX-1` ist der letzte speicherbare Tick und ein stabiler
  terminaler Zustand; ab dort verweigert der nächste Schritt vor Mutation.
  `UINT64_MAX` bleibt ungültig. Nur Rule7 wird beim Restore vom alten Health-
  Deadline-Headroom ausgenommen, da sein reiner Scheduler keine solche Deadline
  erzeugt. Alle historischen Tick-/Deadline-/400-Tick-Nachfrageverträge bleiben
  erhalten.
- **Entwicklungsumfang:** Rule7-Ticks verändern ausschließlich die Zeit. Sie
  gewähren weder Waren, Service, Bewohner, Evolution, Steuern oder Unterhalt
  noch alte Wirtschaft unter neuem Namen. Der vorhandene, ausschließlich im
  SDL-Testtarget kompilierte Adapter erlaubt Pause/Step/Speed für diesen
  Scheduler und tatsächliche F5/F9-Handler; Reset, normaler Spielstart und
  Recovery bleiben geschlossen. Runtime-Modelbindung gehört zu OE-11.
- **Prüfung/Review:** Die reine Suite mit 42.579 Prüfungen und der private
  EXE-Prüflauf mit 11 Fenstern/99 Instruktionen/348 Bytes/22 Assertions sind
  ausgeführt und bestanden. World-Replay-, Save-/SDL-/OS-Prozess-, Legacy- und
  Konfigurationsresultate werden nach tatsächlicher Ausführung im Fortschritt
  protokolliert. Kein vorzeitiger Gate-B-, Original-Echtzeit- oder menschlicher
  Gameplay-PASS. Vertrag und Originaladressen stehen im
  [Schedulerdokument](../original-economy/scheduler.md); private Disassembly
  bleibt ausschließlich unter `.local/original-economy/oe10/`.

### OE-09: separates Schema23 ohne vorzeitige Wirtschafts-Authority

- **Revision:** Sauberer, live bestätigter `main`
  `4d5f9fb88fe9ca0a106c1aea30cf4080230f390b`; bestehender Feature-Branch per
  Fast-forward angeglichen. OE-08 bleibt eigener Commit
  `ac052762fba501a85f241a4828726c2a9deb1e90`. Eigene SHA folgt im Abschluss und
  bei OE-10. Delegierter Commit-/PR-/Merge-Handoff gilt weiter, ohne Tag/Release.
- **Versionsbindung:** Schema23 weiterhin frei; jetzt ausschließlich City-v16/7
  und unveränderte policy3. Keine Migration, neue Spieleroption oder Recovery-
  Freigabe. Reguläre Konstruktoren/Restore/Import/Ticks bleiben gesperrt.
- **Getrennte Authority:** Optionaler OriginalEconomyState nur unter7, eigener
  OriginalHouseState mit Typ, Beobachtungen/Timern, sechs Beständen, Counter,
  Phase und eindeutiger BuildingId-Referenz. Kein zweiter House-NextID, keine
  Modelbedarfszahlen oder V0–2-Level-Kopie. Syntaktische Domains folgen den puren
  Kernen; aktuell werden ausschließlich zugeordnet bezahlte Vacant-Nullzustände
  restauriert. Nicht implementierte Waren-/Service-/Bewohnerzustände bleiben
  ausdrücklich verweigert statt kostenlos angelegt.
- **Funktionaler Container:** `PlaceHousehold` nur in der ausdrücklich gewählten
  Entwicklungsworld erlaubt, mit bestehendem80-Funds-/2×2-/BuildingId-Vertrag;
  Bevölkerung0, getrennte OriginalState-Werte0, kein Legacy-Level. Diese enge
  authored Transaktion ermöglicht nichtleere echte Persistenz ohne Test-Injektion.
  Originale Kosten, Hausgeometrie oder aktive Hausentwicklung werden nicht behauptet.
- **Quelle:** Schema23 speichert nur Model-SHA/Format/Bytezahl/Difficulty und
  verlangt tatsächliche Fileprüfung bei Save/Restore. Keine Tabellen oder
  Runtime-RulesModel-Bindung vor OE-11. Ungebundene OE-08-Fixtures bleiben
  unspeicherbar; normaler Rule7-Start bleibt geschlossen. Clock/Cyclephase0 ist
  lediglich reserviert; OE-10 besitzt die Zeitbasis und deren Fortschritt.
- **Striktheit/Atomicity:** Vollständige rekursive kanonische Shape-/Typprüfung,
  Duplicate-Key-Erkennung nur für23, bounds/IDs/Source und bestehende vollständige
  World-/Map-/Finance-Prüfungen vor Veröffentlichung. Unveränderte atomic_replace-
  und Pfadsperren. Alte gültige1–22-Writer und Lesetoleranz werden erhalten;
  neue OriginalPayloads in alten Generationen abgelehnt.
- **F5/F9:** Nur neuer SDL-Test kompiliert den privaten Persistenzadapter. Er
  kauft regulär Haus/Straßen, benutzt die tatsächlichen produktiven SDL-F5/F9-
  Handler, bleibt pausiert und prüft vollständige Byte-/Dokumentgleichheit auch
  in einem zweiten OS-Prozess. Normale App und Menü erhalten keinen Zugang.
- **Vergleichsgrenze:** Neue World-Daten enthalten erstmals einen aktuellen
  OriginalHouse-Typ. Die gleichnamigen Forktypen kollidieren beim alten Housing-
  Vergleich. Ausschließlich lokale Vergleichskopien erhalten einen getrennten
  Namespace/Includepfad; Forkquellen und numerische Regeln bleiben unangetastet.
  Exakte World-/DTO-Pins, unabhängig geprüfter Adapter und frischer echter
  Vier-Modul-Vergleich sind Pflicht vor Aktualisierung der portablen Evidenz.
- **Belege:** [Schema23-Vertrag](../original-economy/save-schema-23.md), separate
  Unit-/F5/F9-/Prozess-/Legacy-Protokolle im Fortschritt. Gate B bleibt bis
  OE-12 offen; keine native menschliche oder Gesamt-Originalparitätsabnahme.

### OE-08: Versionsidentität ohne vorzeitige Wirtschaftsfreigabe

- **Revision:** Start am sauberen, live bestätigten `main`
  `2651408e71db6a13ad6bec45214dd2b76f502ea2`; Feature-Branch per Fast-forward
  angeglichen. OE-07 bleibt eigener Commit
  `26faf654c6d714a88c4b72ffb29c5e1d0b936add`. Eigene SHA folgt im Report und
  bei OE-09; delegierte Commit-/PR-/Merge-Freigabe gilt weiter, ohne Tag/Release.
- **Versionsentscheidung:** Rule7 und Schema23 sind weiterhin frei. Nur exaktes
  City-v16/7 aktiviert `original_economy_enabled()`. policy3 bleibt unverändert;
  Schema23 wird erst OE-09 implementiert, Modelbindung OE-11, Scheduler OE-10.
  Keine alte Versionsidentität, Defaults oder gespeicherte Daten ändern sich.
- **Authored Entwicklungsgrenze:** Reguläre Konstruktoren/View/Restore/Saves
  verweigern die unfertige Generation. Eine ausdrücklich benannte policy3-
  Testfactory lässt nur bezahlte Road-/Bridge-/Ramp-Infrastruktur zu, ohne neue
  Snapshotfelder oder gespeicherte Freigabe. Economy-Commands und Ticks werden
  atomar verweigert. Alternative, bereits Legacy-Produktionsschritte unter7
  laufen zu lassen, wurde verworfen: Sie würde eine verfügbare Originalwirtschaft
  vortäuschen und neue Authority vor deren eigenen Verträgen einführen.
- **CLI/Map:** `--sandbox-original-economy-check` ist ausdrücklich map-only,
  liefert separat Mapstatus und `runtime_ready:false` und endet auch bei
  geeigneter Map mit1. Kein Model-Parse, World, SDL, Save, Asset-/Visual-Setup;
  Inputkombinationen bleiben streng. Keine neue Spieleroption im Menü.
- **Infrastruktur/Legacy:** Gemeinsame bounded Gates erlauben Bridge5/6/7 und
  Ramp6/7. Nur MapRulesCheck/RoadConnectivity benötigten weitere Guardänderung;
  Bridge/Ramp/Renderer/MapPermissions bleiben unverändert. Zwei alte Tests
  verwenden8 als unbekannte Version statt der jetzt reservierten7; sämtliche
  bisherigen positiven Behauptungen bleiben bestehen. Neue vollständige
  Profilmatrix und der bezahlte Rule6/7-Vergleich sichern Preise/IDs/Graphen.
- **Forkvergleich:** Keine neuen Forkimporte oder Originalmechanik. Der genaue
  World-Header-Kompatibilitätspin des Vergleichswerkzeugs wird nach unabhängigem
  Review der additiven API aktualisiert und der echte74-Fall-Vergleich in drei
  Konfigurationen neu ausgeführt. World-/Snapshot-Datenfelder bleiben gleich;
  portable Evidenz wird aus tatsächlichen identischen Resultaten aktualisiert.
- **Belege/Grenzen:** [Versionsvertrag](../original-economy/rule-generation.md),
  neue Unit-/CLI-/View-Gates und tatsächliche Prüfprotokolle im Fortschritt.
  Unabhängiger Core-/Persistenzreview ohne Blocker. Gate B bleibt offen; keine
  Behauptung einer spielbaren Originalwirtschaft oder menschlicher Abnahme.

### OE-07: ausführbarer Forkvergleich und begrenzte Gate-A-Abnahme

- **Revision:** Start am sauberen, live bestätigten `main`
  `6f6be387128a7f72a64ac6e5439faf2b81df5666`; bestehenden Feature-Branch per
  Fast-forward angeglichen. OE-06 bleibt eigener Commit
  `88e070865600b39d7296be7a6c821724c5cbe682`. Eigene SHA folgt im Report und
  bei OE-08. Delegierte Commit-/PR-/Merge-Freigabe gilt weiter, ohne Tags/Release.
- **Referenz/Evidenz:** Vier tatsächliche Forkmodule am unveränderten Stand
  `d861d2912337c09751cb68196f85d5a30752726e`; elf Dateien gegen fest im Tool
  gebundene SHA-256 geprüft, nicht gegen frei umschreibbare Manifestwerte allein.
  Eigene Adapter werden separat gegen Fork und aktuelle Kerne kompiliert.
  [Maschinenbericht](../original-economy/core-parity.json) und
  [Entscheidungen](../original-economy/core-parity.md) binden Quellen, unabhängig
  formulierte synthetische Inputs und die erwarteten/ermittelten Ergebnisse.
  Forkimplementierung, Originaltabellen und Windows-Änderungen bleiben außerhalb
  des Commits. Original-EXE wird nicht ausgeführt.
- **Abweichungen:**74 numerische Kontrollfälle:46 `parity`,18 `corrected_bug`,
  sechs `unverified`, vier `unsupported`; der HousingView-Probe steht separat.
  `corrected_bug` bezeichnet im JSON die lesbare Kategorie `corrected bug`.
  Erwartete Unterschiede bestehen nur bei nachgewiesenen Zahlen auf beiden
  Seiten. Eigenständige Belege unterscheiden Originalzweige von authored
  Bounds; verworfene Parserinputs/Quality-Grants und ungeklärte Float-/Clock-
  Annahmen bleiben ausdrücklich als solche gekennzeichnet. Gleiche Ergebnisse
  sind begrenzte Vektorenparität, keine universelle Funktionsäquivalenz.
- **`model` Vergleich:** Optionaler Originalvergleich bindet denselben read-only
  Modelhash.15 Housezeilen ×24 Felder ×fünf Difficulties ergeben1800 vorbereitete
  Werte, verglichen als Digest ohne Zahlentabelle. Synthetische Parser-/Building-
  Arithmetic-Vektoren sowie bestehende Raw-/Schema-CTests sind getrennt.
  Ein fehlender optionaler Input ergibt77/SKIP; vorhandene kaputte Inputs,
  falsche Pins, Compilerfehler und ein altes Outputverzeichnis ergeben FAIL.
  Ohne angeforderten Originalpfad steht der Real-Teil ausdrücklich NOT_RUN.
- **`READ-ONLY INTEGRATION`:** Der tatsächliche vierte Forkbaustein läuft gegen
  die unveränderte passende Simulation-Library; Libraryhash und World-Headerblob
  werden protokolliert. Legal erzeugte alte Profile beobachten ein gespeichertes
  Populationsfeld0 (damals noch ohne Populationssimulation), während der Fork
  `max(population,1)` und Shelter als Diagnosebasis einsetzt. Eine reale bezahlte
  CityV7-Foodlieferung plus umgeschaltete Quality-Option verändert den Bericht
  bei identischem WorldSnapshot. Das beweist die Annahme im Bericht, keine
  originale Bewohner-/Food-Quality-Authority. Sieben funktionale Modelkandidaten
  bleiben Inferred; der stetige Herbalist-Regrant ist separat Sourceaudit.
- **Pure Fähigkeiten/Integrationsgrenze:** Countdown, Evolution und Konsum sind
  einzeln verfügbar und werden zusätzlich gegen den tatsächlichen Fork-
  `step_house` als ausdrücklich authored Komposition getestet. Diese Kontrolle
  rekonstruiert keinen Kalender. Originale Mauersektoren, Flagproducer,
  vollständige Actor-/Food-Score-Produktion und neue bezahlte Warenketten bleiben
  offen. Eine Gate-A-Freigabe betrifft den abgegrenzten Kernvergleich und die
  Legacy-Nichtregression; Phase B beginnt frühestens mit dem folgenden OE-08.
- **Review/Regression:** Der Runner prüft Beobachtungen strikt numerisch,
  kanonische Vektoren, vollständige Ergebnisse und aktuelle Codehashes vor einer
  öffentlichen Berichtsausgabe. Ein CLI-Regressionstest bindet den gefundenen
  Dangling-Fork-Fix: Eingabepfad vor `resolve()` prüfen, damit ein vorhandener
  Defekt kein SKIP wird. Der getrackte Bericht wird ohne private Quellen durch
  den neuen CTest überprüft. Alle alten Kern-/World-/Save-/Render-/Input-Dateien
  bleiben bytegleich; tatsächliche Gesamtsuites, Negativfälle und Legacy-Replay
  stehen in PROGRESS. Unabhängige Vector-/Tool-/Vertragsreviews abgeschlossen.

### OE-06: getrennte Warenbedarfe, konservative Transfers und Food Quality

- **Revision:** Start am sauberen, live bestätigten `main`
  `ed2470256912bf8c70e700b1d30553f6d226c15e`; Feature-Branch per Fast-forward
  angeglichen. OE-05 bleibt eigener Commit
  `269e80d850ea7d0e8d90b4853a48cf86f8140bf8`. Eigene SHA folgt im Report und
  bei OE-07. Delegierte Commit-/PR-/Merge-Freigabe gilt weiter, ohne Tags/Release.
- **`exe` Lieferung:** [Liefervertrag](../reverse/original-goods-delivery-contract.md)
  bindet sechs unabhängige Warenbestände. Lieferung verwendet die nächste
  kanonische Modellzeile, mit aktueller Zeile7 als Ausnahme, Sizebyte>1 ×4 und
  Elite ×2. Genau null Bewohner setzt Hemp/Tea vor beiden Faktoren auf10;
  nichtelite Häuser brauchen dennoch positive Bewohner. Bedarf und Admission
  bleiben getrennt. Nachfüllziel `max(Bedarf,10)` ist keine physische Kapazität.
  Der Fallback vergleicht mit Quellverfügbarkeit, ohne abschließendes Bedarfscap;
  die Fork-Formel verändert diese belegten Teillieferungsgrenzen.
- **`exe` Konsum:** [Konsum-/Qualitätsvertrag](../reverse/original-goods-consumption-food-contract.md)
  bindet aktuelle Modellzeile und sechs unabhängige Abzüge, ohne Size-/Elite-
  Multiplikation. Genau null Bewohner überspringt den expliziten Konsumaufruf.
  Bronze und Lack verbrauchen tatsächlich beide unabhängig Feld13. Die ODER-
  Voraussetzung der Evolution begründet keine Prioritäts- oder Ersatzregel.
  Damit ist die frühere OE-02-Konsumaussage präzisiert; der Fork ist in diesem
  einzelnen Punkt bestätigt. Selektoren0/7 sind Aufrufbelege, keine bestätigten
  Monate, Worldticks oder implementierte Scheduler-Verbindung.
- **`exe` Food Quality:** Separates Byte, keine siebte Warenmenge. Der Nenner
  des Blendings ist der Lebensmittelbestand **vor** Lieferung, nicht Modell-
  FoodQuality oder Foodbedarf. Die EXE speichert den Quotienten als binary32,
  nutzt strikte Grenzen und ganzzahlige gewichtete Mittel. Das widerlegt den
  Fork-Parameternamen `food_need`; eine einfache double-Division genügt im
  verbreiterten int32-Beobachtungsbereich nicht an allen Rundungsgrenzen.
- **`authored` Sicherheit/Numerik:** Reine Rückgaben erhalten Caller-Inputs,
  Fremdfelder und explizite Mengenbilanz. Alle sechs Zielbestände und der
  aufgelöste Quellpool müssen nichtnegativ sein, auch bei No-op. Signed-word-
  Headroom und Quellcap verhindern Wrap; Modellbedarf<=0 liefert nichts.
  Originale unchecked Slot-Aggregation/Bytewrap werden nicht emuliert. Quality-
  Inputs sind begrenzte Bytebeobachtungen; eine universelle Original-Score-
  Grenze ist nicht bewiesen. Lieferung0 erhält Quality ausdrücklich, obwohl der
  rohe Originalzweig eine frühe Verbesserung erlauben kann. Exakte int64-
  Mittelpunktvergleiche modellieren nearest-even binary32 ohne globalen FPU-
  Zustand; der historische x87-Controlword ist nicht nachgewiesen.
- **Integration/Abnahme:** Eigene SDL-/World-freie Library ohne App-, Tick-,
  Frame-, Producer-, Navigator-, Save- oder UI-Verbindung. Keine realen Waren
  oder Besuche aus reinen Queries. Typen/Zeilen/Sizebyte sind Beobachtungen,
  keine Geometry-Authority. Originaldateien und alte Quellen bleiben exakt.
  Tatsächlich gelaufene Tests und Legacy-Bytevergleiche stehen in PROGRESS;
  bezahlte neue Quellen, vollständige Markt-/Food-Score-Produktion und Human-
  Gameplay bleiben offen. Gate A folgt erst mit OE-07.

### OE-05: getrennte Besuchseffekte, Verfall und Spawnversuche

- **Revision:** Start am gemergten Live-`main`
  `7f5742400b4fbac9c31fb653d7562601df6d9111`; Feature-Branch zuerst per
  Fast-forward angeglichen. Voriger Task-Commit bleibt
  `a8fd0ce0615d7009eecc373359c8c00b1a3207d4`. Eigene SHA folgt im Report und
  bei OE-06. Delegierte Commit-/PR-/Merge-Freigabe gilt weiter für diesen
  Branch, ohne Tags/Releases.
- **`exe` Zeit/Visit:** [Timervertrag](../reverse/original-service-timer-contract.md)
  bindet fünf Servicebytes mit Grant96 und vier Religionen mit Grant40.
  Wiederholung ersetzt statt addiert; Verfall ist unsigned byte minus eins,
  bei null begrenzt. House-Caller benötigt Status1/3 und Byte09, ohne
  Occupancy-Gate. Phase35 verfällt vor Evolution39; mögliche Zwischenbesuche
  bleiben explizite Ereignisse. Keine 96-Worldtick-, Realzeit- oder Monatsannahme
  und kein fusionierter Scheduler.
- **`exe` Pfade statt Fork-Pauschale:** DirectService benötigt positive
  Bewohnerzahl auch bei Elite; DirectReligion und SharedWalker erlauben leere
  Elite. Direct verlangt Status1/3; Sharedcallback prüft ihn lokal nicht.
  Religion benötigt zusätzlich ggf. eine aufgelöste Elite-only-Sourcebedingung,
  deren Spielproducer unklar bleibt. Sharedactor kann mehrere Services/
  Religionen zugleich bewirken; `ServiceVisit` ist bewusst ein bereits
  aufgelöster Timereffekt, kein vollständiger Actorcallback. Paired Acrobat/
  Drama sowie unabhängige Religionseffekte werden explizit komponiert.
- **`exe` Spawn:** [Spawnvertrag](../reverse/original-service-spawn-contract.md)
  bindet elf Provider an fünf unterschiedliche Staffing-Funktionen. Die zwei
  Fork-Tabellen sind für Herbalist, Drama und höhere Religionen unvollständig.
  Effektives signed Staffing und alle vorgelagerten Prerequisites kommen vom
  Caller. Kein Staffing oder geschlossenes Gate erhält den Counter; sonst
  uint8-Wrap, danach strikt `new > threshold`, Reset vor Actorfactory.
  `attempt_due` beweist keinen Actor oder Besuch; kein Servicegrant aus Nähe
  oder bloßer Dispatchfälligkeit. Threshold1 bedeutet zweiten erlaubten Aufruf.
- **`model` Namensbindung:** Religionsquelle214/215–216/217–218/219 bindet
  Index0/1/2/3. Damit +0e Daoist/+0f Buddhist eindeutig benannt; die frühere
  gemeinsame ODER-Anforderung bleibt exakt. Im alten Needs-Dokument nur die
  Namen präzisiert, kein HouseEvolution- oder Save-Verhalten verändert.
- **`authored` API-/Integrationsgrenze:** Getrennte Rollen/Actions/Provider-
  Enums sind API-IDs, keine Originalactor-/WorldIDs. Gültige House-Domäne,
  Enum-/Kombinationschecks und wertbasierte Rückgaben sind eigene Grenzen.
  Default Housekontext ist inaktiv, Elite-only bleibt explizite Sourcefakt-
  Eingabe. Reine Transformationen erhalten Input und alle fremden Felder.
  Keine World-, UI-, Navigator-, Asset-, Frame-, Tick- oder Save-Verbindung.
  Echte Kauf-/Routen-/Ankunftsketten und Clock-Mapping folgen später.
- **Review/Tests:** Getrennte Visit-/Spawn-Belege und unabhängige Core-/Test-
  Reviews ohne verbleibende Befunde; ausgeführte Debug-/Release-/Sanitizer-
  CTests und Legacy-Vergleich in PROGRESS. City-v8-Service und alle alten
  Profile behalten ihren 1200t-Ankunftsvertrag. Gate A bleibt bis OE-07 offen.

### OE-04: geprüfte House-Entscheidung und kontextgebundene Kapazitätszeilen

- **Revision:** Start am gemergten Live-`main`
  `5627db513227827e1717884222875ca2505f821b`, vorhandenen Feature-Branch
  zuerst per Fast-forward angeglichen. OE-03 bleibt als eigener Commit
  `12e2f22c94f60402824d8dd0898b9d36086da503` erhalten. Eigene Task-SHA folgt
  im Abschlussreport und bei OE-05. Die delegierte Commit-/PR-/Merge-Freigabe
  gilt weiter für diesen Branch; Tags/Releases bleiben ausgeschlossen.
- **`exe` Typen/Ablauf:** [Statischer Vertrag](../reverse/original-house-evolution-contract.md)
  bindet alle16 Handler, virtuelle Aufrufe, Typ2–17,15 Modellzeilen und
  Countergrenzen. Vacant teilt Row0 mit Shelter, wird aber nur bei positiver
  Bewohnerzahl und vollständigem Evolve-Verdict zu Shelter, ohne Counterreset.
  Leerer Shelter bleibt unverändert. Dies korrigiert zwei unbelegte Fork-
  Vereinfachungen. EliteUnoccupied bleibt ohne automatische Aufnahme/Entwicklung;
  EliteAbandoned ignoriert Desirability, benötigt aber aktuelle/nächste Needs.
  Terminale Häuser prüfen weiterhin Needs, trotz fehlendem Evolve-Ziel.
- **`exe` Gates/Counter:** Low wird inklusiv vor High geprüft; aktueller Mangel
  gewinnt immer, nächster Mangel macht aus Evolve ein Stay. Counter ist uint8,
  Desirability int8, Bewohner int16. Grenzen9/11/49/17 führen bei der folgenden
  Devolve-Auswertung zu Reset/Ziel, ohne Byte-Wrap. Stay/Evolve setzen gewöhnliche
  Counter zurück. Zwei unabhängige Originalglobals werden als explizite
  `evaluation_blocked`-/`force_desirability_evolve`-Inputs modelliert; ihre
  Spielauslöser bleiben unbekannt. Nur `evolve_step` wendet die Auswertungssperre
  an, getrennte Tabellenqueries bleiben beobachtbar. Kein Originalglobal-Lesen
  oder angenommener Runtime-Trigger.
- **`exe` Needs:** [Bedarfsvertrag](../reverse/original-house-needs-contract.md):
  nichtnullige vorbereitete Präsenzwerte aktivieren Anforderungen auch negativ;
  Zugänge sind nichtnullige unsigned Bytes. Qualität ist uint8, Waren int16,
  Schwellen inklusiv. Bronze/Lack und Daoist/Buddhist sind je ein ODER, keine
  Summe und keine doppelte Pflicht. Keine Flächen-/Bewohnermultiplikation. Der
  Water-Bool normalisiert Byte!=0; er erzeugt keine Versorgung. Tatsächliche
  Visits, Timer, Lieferung/Konsum und Lebensmittelqualität folgen erst OE-05/06.
- **`model`/`authored` API:** Eager `HouseEvolutionRules` bereitet alle15×24
  Named-Felder über OE-02 samt Difficulty und Quellenhash vor; auch ein späterer
  ungenutzter Feldfehler verhindert die Veröffentlichung. Keine Defaults oder
  per-Query-Dateiarbeit. Typ/ModelRow/World-Level bleiben verschiedene Domänen.
  Pure Rückgabewerte erhalten alle Felder außer Typ/Counter; Enumchecks und
  vollständige Vorbereitung sind eigene Sicherheitsgrenzen. Das Ergebnis ist
  ein Übergangsintent ohne originale Geometry-/Populationstransaktion und
  entspricht nicht der separaten Original-Diagnosefunktion0x51a660.
- **Kapazität:** Signierter vorbereiteter Feld17-Getter bleibt unskaliert;
  keine willkürliche Nullklammer bei editierten negativen Werten. Für kanonisch
  initialisierte EliteUnoccupied-Zustände unterscheiden sich die belegten
  Modellzeilen: Census9, zwei Admission-Pfade11; sonst gespeicherte Zeile.
  Separate Selektoren machen diesen Unterschied sichtbar. Census-Eligibility,
  Bewohneroverride, andere Admission-Pfade und lokale Floors bleiben getrennt;
  kein universeller `capacity(type)`- oder vollständiger Migrationsanspruch.
- **Runtime/Save/Review:** Neue Library und Tests ohne World-/App-/SDL-/Save-
  Verbindung. Keine aktive Rule, Originaltyp-Grafik, Platzierung oder Gratis-
  Ressource. Alte Quell-/Save-Verträge bleiben maßgeblich; unabhängige
  Beleg-/Code-/Testreviews und tatsächliche Prüfungen stehen in PROGRESS.
  Gate A bleibt bis OE-07 offen.

### OE-03: isolierte Ausbreitung und ausdrücklich experimentelle Mauern

- **Revision:** Start am live bestätigten, gemergten `main`
  `d1a082d4f9457690367f27a5fe7099904f5b90ac`; Feature-Branch zuerst per
  Fast-forward angeglichen. OE-02 bleibt als eigenständiger Commit
  `df13b2e9bc0112d66c188276b5753c92ce6a5a40` erhalten. Eigene Task-SHA folgt
  im Abschlussreport und bei OE-04. Die delegierte Commit-/PR-/Merge-Freigabe
  gilt weiterhin für diesen Branch, ohne Tags/Releases.
- **`exe`:** [Gebundene statische Belege](../reverse/original-desirability-contract.md)
  bestätigen Quadratperimeter, ersten Initialwert, höchstens zehn Ringe,
  absorbierende Null nach Vorzeichenwechsel und Sättigung nach jeder Addition.
  `step <= 0` aktualisiert nach jedem Ring; die Fork-Auslegung „nie ändern“
  wird ausdrücklich verworfen. Quellenreihenfolge bleibt erhalten, da ein
  späterer globaler Clamp andere Ergebnisse liefert. Der Kern rekonstruiert
  weder Manager-Zulassung noch Terrainquellen des vollständigen Originals.
- **`model`:** Alle vier Spread-Felder werden über die geprüften OE-02-Named-
  Zugriffe samt Difficulty vorbereitet. Der Caller liefert den Footprint
  unabhängig; `SZE` ist ein Delta. Die Building-ID ist keine World-ID oder
  Berechtigung. Fehlende/falsch gelabelte Daten erhalten keine Ersatzwerte.
- **`authored` Sicherheitsgrenzen:** Dimensionen1–228, vollständig auf der
  Karte liegende Quadrate1–6 und höchstens4096 Quellen pro atomarem Batch.
  Halo-Ringe werden abgeschnitten; ungültige Abfragen erhalten keine erfundenen
  neutralen Außen-Zellen. Verwendete Ringwerte müssen einschließlich möglicher
  Zelladdition in int32 passen; benutzte Updates und Bonus werden vor Clamp
  geprüft. Unbenutzte Updates nach dem letzten Ring werden nicht ausgewertet.
  Keine Wrap-Emulation editierter Modelle oder Behauptung allgemeiner größerer
  Original-Footprints.
- **Query:** Vollständiges signiertes Footprint-Maximum, anschließend expliziter
  optionaler Bonus ohne Zell-Clamp. Das originale bedingte+10 ist statisch
  beobachtet, sein Trigger unbekannt. Default0 und int32-Bonusprüfung sind eine
  eigene API-Grenze, keine Zuordnung des Originalflags oder Bytefelds.
- **`authored` Mauerhypothese:** Default `None`. Nur ausdrücklich ausgewähltes
  `ExperimentalUniform16` verwendet wahres Footprint-Zentrum, exakt ganzzahlig
  klassifizierte22.5°-Sektoren und einen Ring-Vorlauf, der gleich weit liegende
  Zellen nicht abschirmt. Nur negative Quellen; Mauerzellen bleiben davon frei.
  Originale gemischte Einheiten, Richtungstabellen, Nachbarsektoren und
  Randpfade sind ungeklärt. Keine Original-Mauerparität oder World-Bauoption.
- **Alternative/Vergleich:** Unveränderte Produktionshelfer bilden die echte
  authored City-v13-Spalte; ein gesonderter Same-Spec-Manhattan-Kontrollwert
  isoliert allein die Geometrie. Funktionsanalogien Model→World und Szenen-
  Footprints bleiben ausdrücklich inferred/authored. Einige Footprint-Probes
  überlappen Quellen; sie sind reine Abfragen, keine legal gebauten Häuser.
- **Runtime/Save:** Eigenständige Library und Diagnose-CLI, kein World-/App-
  Link, keine Frame-/Tick-Hooks, kein Mapscan oder neue Save-Authority. Alle
  alten Quellen und Snapshots bleiben unverändert. Review und tatsächlich
  ausgeführte Prüfungen werden im Fortschrittsprotokoll festgehalten; Gate A
  bleibt bis OE-07 offen.

### OE-00: tatsächliche Ausgangslage und Arbeitsgrenzen

- **Revision:** `fef111deff4cedd3d3fa48663556d41b2e4b31a6`, nicht der historische
  Paketstand `f7a5035e92a9`. Der zusätzliche Commit ist der geprüfte Hit-Test-Fix.
  Der Feature-Branch startet am tatsächlichen `main`; kein Fork-Merge.
- **Pfad:** Das maßgebliche vollständige Paket ist
  `docs/codex-original-economy/`. Relative Links auf `prompts/` gelten innerhalb
  dieses Ordners. Die drei ebenfalls vorhandenen Root-Kopien bleiben unangetastet.
- **Versionierung:** Aktive City-v16-Regeln1–6, Policies1–3, Save-Schemas1–22
  bestätigt. Rule7/policy3/Schema23 sind die bedingte Architekturwahl für später;
  OE-00 vergibt keine Laufzeitberechtigung. Alte Writer/Parser bleiben exakt.
- **Vererbung:** Rule7 erbt Bridge/Ramp nicht automatisch: explizite Guards in
  `World.h` und Map-/Savepfaden schließen es heute aus. Neue Unterstützung später
  nur gezielt versionieren, nicht die bestehenden5/6-Verträge pauschal erweitern.
- **Evidenz:** Aktuelle Worldpfade sind `AUTHORED FALLBACK`; originale Dateien
  werden read-only identifiziert. Pure Fork-Tests belegen Fork-Verhalten, keine
  neuen `EXE-OBSERVED` Originalmechanismen. `OriginalHousingView` bleibt
  `READ-ONLY INTEGRATION`, nicht echter House-State oder Warenankunft.
- **Baseline:** Vollständige Savebytes und die bereits gegen tatsächliche
  WorldSnapshots geprüften `{rules,world}`-Felder werden lokal eingefroren.
  Hashes aller1485 vorhandenen Policyfälle kommen vom unveränderten
  Produktions-Audit. Keine zweite Simulations-/Save-/Permission-Authority.
- **Review:** Source-/Teststellen in
  [Architekturkarte](../original-economy/architecture-map.md) und
  [Forkvergleich](../original-economy/fork-comparison.md); tatsächliche Abnahme
  und zugehöriger Task-Commit werden im Fortschrittsprotokoll/Report dokumentiert.

### OE-01: isolierter Raw-Reader mit exakter Quellenbindung

- **Revision:** Folgeschritt von OE-00
  `05470049fa4acfdf2d0104f5a25384c9277073ac`; eigener Task-Commit wird im
  Abschlussreport und bei OE-02 nachgetragen. Upstream `main` weiterhin
  `fef111deff4cedd3d3fa48663556d41b2e4b31a6`.
- **Beleg `model`:** Der lokale Originaltext hat13 Building-/24 House-Spalten,
  15 geordnete House-Zeilen und je5 Modifier. Führendes CSV-Leerfeld und
  Komma-Padding sind Export-Syntax; House-Zeilen haben keine schließende Klammer.
  Innere Leerfelder sind dagegen kein belegter Ersatz für einen Wert.
- **Beleg `authored`:** 256KiB Text,4096Byte je Zeile, Building-ID0–4096 und
  int32 sind explizite Reader-Bounds. Keine feldspezifischen Wertebereiche aus
  beobachteten Minima/Maxima erfinden. Optionales Pluszeichen und schließende
  House-Klammer sind dokumentierte Reader-Erlaubnisse, keine EXE-Behauptung.
- **Alternative/Fork:** Keine mutable öffentliche Tabelle, kein stilles
  Überschreiben/Kürzen, kein Überspringen innerer Leerfelder, keine geklemmte
  Difficulty und keine Ersatzwerte. Getypte const-Zugriffe lesen ausschließlich
  die geprüften Rohwerte. Schwierigkeitstransformationen bleiben bewusst bei den
  folgenden Regelaufgaben; OE-01 bestätigt diese Fork-Arithmetik nicht.
- **Identität:** Building-ID bezeichnet den Dateiindex, HouseRowId die geordnete
  Zeile0–14. Dies ist keine World-Building-Zuordnung. Distinkte House-Labels
  verhindern doppelte Zeilen; beliebige semantische Umordnung eines editierten
  Modells wird damit nicht ausgeschlossen. Symbolische IDs/Units folgen OE-02.
- **Quelle:** SHA-256 bindet genau dieselben unveränderten Bytes, die geparst
  werden. Hash, Pfad, Byte-/Zeilenzahlen und Reader-Version sind ein
  Provenance-Report, keine Originalsignatur. Geänderte gültige Modelle bekommen
  ihren tatsächlichen Hash; keine globale Whitelist des einen GOG-Hashes.
- **Laufzeit/Save:** Eigenständige SDL-/World-freie Library plus Diagnose-CLI,
  ohne App-/World-/Save-Link. Keine neue Rule oder Migration, kein Tick-/Frame-I/O.
  Create-/Restore-Hashbindung kommt erst mit OE-11. Originale Daten bleiben lokal
  und read-only; Git enthält ausschließlich synthetische Testwerte.
- **Review/Tests:** Unabhängige Grammatik- und API-/I/O-Prüfung gegen die lokale
  Quelle und den gebundenen Fork; Befehle und tatsächliche Ergebnisse werden in
  PROGRESS dokumentiert. Fehlende optionale Originaldaten ergeben ausschließlich
  im separaten Real-CTest SKIP77; vorhandene kaputte Daten dürfen nicht skippen.

### Merge-Handoff – ausdrückliche Nutzerfreigabe am 2026-10-16

- Der Nutzer hat nach seinem Push von `feature/original-economy-v1` ausdrücklich
  Commit und Branch-Merge an Codex delegiert. Für diesen Branch umfasst dies
  den geprüften Dokumentations-Fix, Push, Pull Request nach `main` und Merge
  nach bestandenen relevanten Prüfungen. Diese aktuelle Freigabe ersetzt den
  historischen Push-/PR-/Merge-Ausschluss des Arbeitspakets im genannten Scope;
  Tags und Releases bleiben außerhalb des Auftrags.
- Ein regulärer Merge-Commit erhält die eigenständigen OE-00/OE-01-Commits und
  die Nutzeränderung. Erwartete PR-Head-SHA beim Merge prüfen; keine erzwungene
  Ref-Aktualisierung, kein Squash der Aufgabenhistorie, kein Löschen des Branches.
- Root-Auftragsdateien bleiben Einstiege mit korrigierten Verweisen auf
  `docs/codex-original-economy/`; dort bleiben Entscheidungen und Fortschritt.
  Keine Wirtschafts-/Versionsfreigabe durch diesen Handoff: OE-01 ist weiterhin
  isolierter Raw-Reader, OE-02+ und Gate A bleiben offen.

### OE-02: Modellregister v1 und geprüfte Zuordnungen

- **Revision:** Start am gemergten `main`
  `742648466f41f09c4724aa29e5a196a52de79e44`; vorhandener Feature-Branch zuerst
  per Fast-forward darauf angeglichen. OE-01-Commit
  `11211bbe6f579b6171615f6e75ed6fffbfd53cdf` bleibt unverändert in der Historie.
  Die delegierte Commit-/PR-/Merge-Freigabe gilt weiter für diesen Branch;
  keine Tags/Releases.
- **`model`:** 13 Building-/24 House-Spalten,15 geordnete Hauszeilen und sieben
  relevante Building-Typen besitzen explizite getrennte Enum-IDs. Ein gemeinsames
  Feld13 bedeutet Bronze- oder Lackwaren; Feld23 Daoist oder Buddhist. Kein
  zusätzliches Feld; Konsum und ODER-Bedarf sind getrennte Verträge (OE-06).
  Genau fünf begrenzte Goldenwerte werden als einzelne Testfakten dokumentiert,
  keine Massentabelle.
- **`exe`:** SHA
  `6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e` bindet
  die statischen Belege. Loader0x5d1830 transformiert Building-Felder als
  vorzeichenbehaftetes `base*modifier/100`, House-Felder als
  `clamp(base+modifier,-99,100)`. Ganze Zahlen werden gegen null gekürzt.
  Source-Goldens bleiben roh; vorbereitete Werte sind keine ganzen Kaufpreise
  (zusätzlicher Cost-Hook0x44c380) und keine World-Zeit-/Güterkonversion.
- **`authored`:** Produkte und Summen werden in int64 berechnet, jedoch vor
  Division/Clamp gegen den originalen int32-Zwischenbereich geprüft. Außerhalb
  erfolgt ein expliziter Overflow-Fehler statt stiller Wrap-Emulation. Raw-
  Parsing behält volle int32-Werte. Falsche Difficulty/Enums werden niemals
  geklemmt. Die Namensschema-Prüfung vergleicht tatsächlich gelesene Labels;
  sie ersetzt weder den Originalhash noch beglaubigt sie geänderte Werte.
- **Identitätsdomänen:** Named `ModelHouse` enthält15 Werte ohne erfundene
  Vacant-Zeile. Source Building-Typen3–17 gehören dazu; Vacant2 hat keine eigene
  Zeile im strikten Adapter. Die originale Initializer-Aliasbildung
  `max(type-3,0)` wird separat dokumentiert. World-BuildingId und alte
  House-Level0–2 bleiben eigenständig.
- **Funktion/Präsentation:** Sieben Fork-Paarungen sind `inferred` funktionale
  Kandidaten, sechs gültige Objects bleiben ohne Kandidat. Bestehende Well72-/
  Herbalist207-Spriteregistrierung ist separat reverse-engineered; keine
  vollständig belegte Originalmechanik für irgendein World-Object. Der Lookup
  ist rein, ohne World-Aufruf/Simulation-Link oder Profil-/Geometrieberechtigung.
- **Offen:** Physische Warenmenge, Zeitskala, Servicelebensdauer, Ressourcen-
  Codes und weitere Risiko-/Steuerdetails bleiben `unknown`. Das versionierte
  [Faktenregister](../original-economy/model-provenance.md) trennt alle fünf
  Evidenzklassen. Der [Reverse-Vertrag](../reverse/original-model-contract.md)
  enthält Belegadressen und Grenzen; kein Originalcode oder Disassembly im Git.
- **Review:** Unabhängiger Quellen-, API-/Test- und Dokumentationsreview ohne
  blockierenden Befund. Belegtests und Commit werden im Fortschritt/Abschluss
  festgehalten; keine Rule-/Save-/World-Verhaltensänderung durch OE-02.


### OE-18: Food-Vertrag vor Implementierung

Der Gesamtauftrag vom2026-10-17 setzt die bezahlte Food-Kette nach OE-17
`b30718f` fort. Der [Food-Vertrag](../original-economy/food-runtime.md) trennt
belegte Originalmischung und Objektidentitäten von authored Mengen, Zeitskala,
Wheat20/Millet80 und einem separaten Food-Sink. Keine siebte Originalware,
keine freie Versorgung, keine alten Saveänderungen und keine stillschweigende
Vervollständigung der unbekannten Source-/Diversitätslogik. Revision18 wird
strict gespeichert; Qualität ausschließlich aus tatsächlicher Ankunft.


### OE-18: technischer Abschluss und Grenzen

Der [Food-Vertrag](../original-economy/food-runtime.md) ist in Revision18
umgesetzt. Die bezahlten Farmhouse193-/Wheat195-/Millet196-Objekte liefern
über reale Wege, eigene Cargo und tatsächliche Ankünfte. Nur die belegte
Originalformel ist Originalmischung;20/80-Quellklassen, Mengen, Zeitskala und
Food-Sink sind authored. Aktuelle Qualität wird durch einen mengenabhängigen
Arrival-Witness getragen und nach vollständiger Leerung zurückgesetzt.

Die159 vollständigen Konfigurationstests, acht tatsächlichen20k-Prozessreplays,
Rohdaten-Audits und der frische72/72-Legacyvertrag liefen auf Quelle b53edf2.
Danach ändern genau zwei Dateien ausschließlich Whitespace zur Beseitigung von
Einrückungswarnungen. Ein unabhängiger Clang-Token-/Literal-/Kommentar- und
Präprozessorvergleich plus Rekonstruktion aller516 Sourceinventar-Dateien
belegt die Bindung an die finale c3cfaf8-Quelle. Drei vollständige43-Target-
Builds sind warnungsfrei, drei neue FoodWorld-Läufe bestanden. Der tatsächliche
Release-Prozessfixture bleibt bytegleich. Alte Berichte behalten ausdrücklich
ihre Originalquelle; keine stille Umetikettierung und keine geschwächten Tests.

Gate C bleibt bis OE-19 offen. Die tatsächliche Softwareaufnahme ist kein
native-/human-/Originalart-PASS. Die noch fehlende Originalsprite- und gesamte
Objektkomposition bleiben Teil des ausdrücklichen Gesamtauftrags; deklarierte
Fallbacks ersetzen diese spätere Abnahme nicht.


### OE-19: gezielte Quellen vor Implementierung

Nach OE-18 `144eec6` gilt der [Service-Vertrag](../original-economy/services-runtime.md).
Nur der ausdrücklich gewählte Rule10-/Schema26-Stand19 bindet HealthPost an
Herbalist207 und ServicePost als authored Adapter an Acupuncture208, mit
echten2×2-Körpern, Modellpreisen/-Workers und je einem realen Courier. Es gibt
keine neue Originalidentität historischer Posts, keine allgemeine Servicefreigabe
und keinen Timer aus alten Schutzbits. Bereits entsandte cargo-freie Besucher
kehren nach normalem Besuch auch bei später geschlossener Quelle zurück.
Phase35-Verfall folgt Ankünften; die gleichzeitige96→95-Reihenfolge ist authored.

Der Restore prüft zusätzlich eine notwendige physische Mindestfahrzeit aus
2×2-Manhattanabstand, mindestens zwei Kanten über die gewöhnliche Straße und
dem unveränderten Rule10-Profilwert5Ticks/Kante. Der erste Move liegt im
Dispositionstick. Dieselbe sichere Zeitgrenze gilt nach Kauf/Zuzug für
kompaktierte Witnesses; sie ersetzt keine vollständige historische Route.
Koordinierte Änderungen von Trip, Witness und passend berechnetem Timer werden
negativ geprüft. Technische Gate-Abnahme bleibt von Originalart/native/human
getrennt; keine Warteschleife auf manuelle Nutzerprüfung.

### OE-19: technische Gate-C-Abnahme

Die finale Revision19 besteht drei vollständige202er-Suiten nach frischen
warnungsfreien Cleanbuilds, tatsächliche Prozessneustarts, unabhängige
Timer-/Warenbilanzen und den vollständigen unveränderten Legacyvertrag.
Gate C ist damit technisch abgenommen. Native/human und Originalsprite-
Komposition sind nicht daraus abgeleitet; OE-20 folgt unmittelbar.

Zwei private Abschlussprüferannahmen wurden nach erhaltenen FAIL-Berichten
eng korrigiert: rekursiver520-Dateien-Pin gegenüber517 tatsächlich erfassten
Code-/Datendateien plus drei ignorierten Finderdateien, sowie der ausdrücklich
konfigurierte externe Pythoninterpreter gegenüber Buildroot-Binaries. Alle
aufgezeichneten Codebytes und Binaryhashes bleiben strikt geprüft. Fehlende
historische Metadaten-/Symlinkaufnahmen werden nicht erfunden. Der separate
historische Legacybuild behält seine ursprüngliche Warnungsbehandlung und
genau eine bekannte Warnung des unveränderten gesperrten Playthrough-Fixtures;
die drei aktuellen Cleanbuilds sind davon getrennt warnungsfrei.

### OE-20: Ancestor-Vertrag vor Implementierung

Nach OE-19 `ba116b4c97defdf134434c1b0398292fb3727d0e` gilt der
[Ancestor-Vertrag](../original-economy/ancestor-runtime.md). Revision20 ergänzt
einen eigenen bezahlten2×2-Schrein214 samt eigenem cargo-freiem Courier und
tatsächlicher DirectReligion-Ankunft40. Sourcecondition elite_only=false ist
ausdrücklich authored, da deren originale Sessionproduzenten offen sind.
Provider-/Trip-/Witness-Bounds werden ausdrücklich6/6/60; alte medizinische
96er-Kategorien, frühere Regeln und Saves bleiben getrennt erhalten. Keine
originale Worldtick-Spawnfrequenz, kein Visit beim Bau und keine vorgezogene
Originalgrafik-Abnahme. Es folgt keine Wartefrage für einen manuellen Test.


### OE-20: technische Abnahme und unmittelbare Fortsetzung

Der vorher festgelegte Ancestor-Vertrag besteht drei warnungsfreie Werror-
Builds und3×123 relevante CTests,3×74 Paritätsfälle, tatsächliche bezahlte
Ankünfte/Verfall/Wiederbesuche,18 separate20k-Fortsetzungen, F5/F9 und den
vollen unveränderten Legacyvertrag. Das ist die begrenzte technische Abnahme
von Revision20, kein vorgezogenes Gate D oder Originalgrafik/native/human-PASS.
Eine tatsächlich geöffnete Aufnahme führte vor Abschluss zur Korrektur des
Medizinzeilenumbruchs samt sichtbarer Food-Fehlversuche. Die einzige Änderung
am privaten Prozessnachweis betrifft seinen falschen Resume-Zählfilter;
ursprüngliches FAIL und echte unveränderte Prozessartefakte bleiben getrennt
gebunden. Nach eigenem Commit folgt OE-21 ohne manuellen Testhalt.


### OE-21: Wasser- und Housing-Vertrag vor Implementierung

Nach OE-20 `825fd2c65d90c9139e283f2bf3e191bb5feb4e39` gilt der
[Housing-Vertrag](../original-economy/housing-runtime.md). Revision21 ersetzt
keine früheren Saveverträge. Eine neue bezahlte Well72 mit eigenem realem
Walker liefert normalen Wasser-Timer96; Phase28 materialisiert Wasser,
Phase35 zählt herunter, Phase39 führt den unveränderten Evolutionskern aus.
Die alte neutrale60er-Quelle bleibt ohne Wasser. Echte Plain Cottage braucht
laut Originalmodell Water/FoodQuality20/Ancestor, kein Hemp/Ceramics; deren
Negativkontrolle wird deshalb ausdrücklich synthetisch modelliert. Zwei
permanente erste Aufstiegsbelege verhindern bloß injizierte höhere Typen.
Keine unbelegte sofortige Eviction: zugelassene Bewohner bleiben bei
Rückentwicklung; weiterer Zuzug respektiert die aktuelle Aufnahmekapazität.
Globale neutrale Quellenursprünge bleiben nach bezahltem Entfernen erhalten.
Keine neue Original-Randomwalk-/Immigrations-/Grafik-Paritätsbehauptung.


### OE-21: technische Abnahme und sichere Fortschreibung

Der bezahlte Wasser-/Housing-Vertrag ist mit dem echten Originalmodell und
normalen Commands/Zuzug bis Plain Cottage nachgewiesen.141 relevante
Konfigurations-CTests,24 separate20k-Production-Fortsetzungen, vollständige
Save-/Report-Gleichheit und unabhängige Herkunftsprüfungen bestehen. Die
frühe authored Plain-Grenze und die unverifizierte Originalwander-/
Immigrationsroutine bleiben ausdrücklich begrenzt; native/human und originale
Sprites sind keine daraus abgeleiteten PASS.

Die breitere Prüfung fand einen echten Restorefehler: Eine reine Shelter6-
Population durfte fälschlich von4 auf3 manipuliert werden. Ohne Abgänge kann
ungerade Bevölkerung nur aus erreichter früherer ungerader Aufnahmegrenze
entstehen. Diese notwendige historische Grenze gilt für aktuelle Population
und sämtliche Evolutionsinputs; frühere höhere Kapazitäten bleiben nach
Rückentwicklung erhalten. Falsche Revisions-/Verbrauchsannahmen der alten
Harnesses wurden eng korrigiert, volle Zustands-/Bilanzvergleiche bleiben.
Alle ursprünglichen FAIL-Belege bleiben sichtbar. Eigenständiger Taskcommit,
danach unmittelbare Fortsetzung OE-22 ohne manuellen Testhalt.


### OE-22: Religionsvertrag vor Implementierung

Nach OE-21 gilt der [Religionsvertrag](../original-economy/religion-runtime.md):
bezahlte215/2172×2 und2193×3, eigene Walker, getrennte40er-Timer und das
unveränderte Daoist/Buddhist-OR-Prädikat. Aktive52/32- und12/12/120-Grenzen
ersetzen keine historischen Limits. Sechs feste Housing-Witnessslots binden
aktuelle und historische Dienste; echte219-Grundfläche gilt auch bei
Mindestfahrzeiten. elite_only=false und gezielter Dispatch bleiben ausdrücklich
authored, solange die originalen Sessionproduzenten nicht vollständig belegt
sind. Keine Grants oder Wartefrage vor einem manuellen Test.

Quellenbefund noch vor Runtime-Anschluss: Confucian219 benötigt im Original
zusätzlich tatsächlich gelieferten positiven Materialbestand WORD+0x40.
Die bereits beschlossene bezahlte Walkerintegration bleibt vorläufig explizit
staffing-only authored und erhält keinen Materialgrant. Die Materialzulieferung
ist eine offene Folgeanforderung der Produktions-/Objektintegration, keine
behauptete Originalfunktion. Details im Religionsvertrag.

Die anschließende Initialisierungsprüfung belegt zusätzlich das Confucian-
Elite-Gate im normalen ursprünglichen Managerzustand als true (andere drei
Religionenfalse). Das jetzige false bleibt ausdrücklich authored und ist beim
späteren Elite-/Objektanschluss zusammen mit dem Materialgate aufzulösen.
Vollständige Session-Lifecycle-Permanenz wird daraus nicht abgeleitet.


### OE-22: technische Abnahme und Fortsetzung

Die begrenzte bezahlte215/217/219-Integration besteht3×50 aktuelle CTests,
3×74 unveränderte numerische Beobachtungen,30 separate20k-Replays und strikte
Save-/Raw-/Legacyprüfungen. Ein alter46/26-Kapazitätstest wurde nach seinem
echten dreifachen FAIL auf zusätzliche reale Käufe bis52/32 erweitert; die
atomare Negativprüfung bleibt strikt. Source-/UI-/Originalgrenzen bleiben
explizit: Confucian-Material/Elite folgt später, Originalsprites29 und Human
werden nicht vorweggenommen. Nach eigenem Commit startet OE-23 unmittelbar.


### OE-23: Kultur- und Diagnosevertrag vor Implementierung

Nach OE-22 gilt der [Kulturvertrag](../original-economy/culture-runtime.md):
bezahlte2112×2/2123×3/2133×3 mit eigenem echten96er-DirectService-Visit.
Gezielter School→House-Dispatch ist experimentell authored; originale Venues,
32er-Performerzeiten und Actor36 werden nicht gratis erfunden. Neue reine
Verfügbarkeitsdiagnose ist ausdrücklich keine Ankunft oder Zukunftszusage.
Aktive58/38 und18/18/180 ersetzen keine historischen Limits; neun feste
Housing-Slots bewahren0/1/5/6/7/8 und hängen2/3/4 an. Nach eigenem geprüften
Taskcommit folgt24 ohne menschlichen Testhalt.

### OE-23: Brandregistrierung der aktiven Dienstquellen

Der Implementierungsreview fand einen fehlenden `fire_eligible`-Anschluss: die
neuen Kulturkörper und die drei in Revision22 hinzugefügten Religionskörper
wurden vom vorhandenen Brandrisikopfad übersprungen. Revision23 registriert alle
sechs ausschließlich aktiven Objecttypen; die bestehenden Firezeiten und Gates
bleiben unverändert. Echte Brandereignisse müssen sowohl neuen Dispatch sperren
als auch bereits laufende Touren nach den normalen Regeln behandeln. Das ist
eine Korrektur der aktiven Integration, kein rückwirkender OE-22-Brandnachweis;
veröffentlichte Regeln und alte Körper erhalten keine andere Brandsemantik.


### OE-23: technische Abnahme

Die getrennten Kulturquellen und reine Diagnose bestehen159 Konfigurations-
CTests,36 separate20k-Replays, strikte Raw-/Legacy- und Paritätsprüfungen.
Ein positiver Aufstiegsnachweis bindet die Bedarfe vor dem Verbrauch desselben
Ticks; negative fehlende/abgelaufene Kultur wird bei tatsächlich vorhandenen
übrigen Bedarfen geprüft. Alle neun Timer bleiben allein in HouseState.
Der tatsächliche Release-Screenshot wurde geöffnet; experimentelle gezielte
Besucher, originale Venue-/Training-Lücke und spätere Originalsprite-Abnahme
bleiben getrennt. Nach eigenem Commit folgt OE-24 ohne Human-Testwartepunkt.


### OE-24: Produktions- und Materialvertrag vor Implementierung

Der [Luxuryvertrag](../original-economy/luxury-runtime.md) setzt die im Auftrag
explizit erlaubten versionierten Rezeptannahmen um: echte Modellobjekte,
bezahlte Pflanzen/Arbeiter, feste80er-Managed-Ernten und atomare Mehrfachinputs,
fünf getrennte Auxmaterialien, native Raw-Clay-Zustellung und Hemp→Paper→Academy.
Aktiv82/58, Aux20/12; pure Goods46/26 und alle alten Versionen unverändert.
Neuer produktiver Abriss bleibt atomar verweigert, bestehender Neutralabriss
wird bei tatsächlicher Warenproduktion geprüft. OR-Bedarf und unabhängiger
Verbrauch beider vorhandenen Bronze-/Lacquerstocks sind original belegt;
kein vermeintlicher Forkbug rechtfertigt einen Patch am eingefrorenen Kern.
Papierpflicht1jeDispatch ist authored; initiales Confucian-Elitegate folgt25.
Originalbilder29 und menschliche Abnahme werden nicht vorweggenommen.


### OE-24: technische Abnahme und Nutzerpause

Die vier bezahlten Luxusketten, fünf getrennten Auxmaterialien und tatsächliche
Hemp→Paper→Academy-Zulieferung bestehen168 relevante Konfigurations-CTests,
43 separate20k-Fortsetzungen und unabhängige Save-/Bilanz-/Legacyprüfungen.
Echte Need1-Kopien prüfen positiven Aufstieg und fehlende Waren, ohne frühe
Originalbedarfe0 oder höhere Hausstufen vor25 zu ändern. Die neue flache
Target×Good-Rotation verwendet bestehende letzte Trips; kein zusätzlicher
gespeicherter Cursor und keine universelle Starvationfreiheitsbehauptung.
Produktiver Abriss bleibt atomar verweigert, bisher erlaubter Neutralabriss
wird mit tatsächlichen Warenbilanzen geprüft.

Beide tatsächlichen finalen Software-SDL-Ansichten wurden geöffnet. Original-
modelltypen/Kosten sind belegt; Ernte-/Rezeptzeiten, Managed Ore, PaperOnly
und direkte Schulbesuche bleiben ausdrücklich authored. Initiales Confucian-
Elitegate, Originalsprites, vollständiger Gate D sowie native/human Abnahme
werden durch diesen begrenzten technischen PASS nicht vorweggenommen.

Der Nutzer verlangt ausdrücklich: „lass uns nach oe-24 eine pause einlegen“.
Nach dem eigenen geprüften OE-24-Commit endet die automatische Fortsetzung.
OE-25 bleibt TODO; erst ein neuer Fortsetzungsauftrag hebt diese Pause auf.
Kein Push, PR, Merge, Tag oder Release während dieser Pause.


### OE-25: Fortsetzung und vorab festgelegter Hausstufenvertrag

Am2026-10-18 setzt der Nutzer ausdrücklich mit OE25 fort. Die Pause nach24
ist damit aufgehoben. Auf Basis7360f84 gilt vor Implementierung der
[Hausstufenvertrag25](../original-economy/housing-stages-runtime.md):
Common und Elite bleiben getrennte Familien, reine numerische Kerne exakt;
aktiver Stand Rule10/Schema26/Revision25, alte Versionen ohne Migration.

Ein separat bezahlter80er Eliteplot verwendet in25 ausdrücklich vorläufige
authored2×2-Geometrie. Zwei ursprüngliche Arrivalpfade belegen Erstbezug nach
Typ13, nicht12;120/40/+2 und Preis bleiben authored. Typ12 entsteht durch
echte50er-Rückentwicklung. Die belegten originalen Seiten2/Common und4/Elite
erfordern für26 eine eigene sichere Materialisierung vor dessen Erstbezug,
keine stillschweigende Uminterpretation der25er Saves. Der bestehende
All-residents-Workeradapter bleibt ausdrücklich authored auch für Elite.

Neun bereits bezahlte Provider erhalten eng begrenzte eigene Modell-DES-
Quellen. Confucian ist künftig tatsächlich elite-only mit realem Paper;
die gezielten World-Besuche bleiben ausdrücklich auf bewohnte Häuser begrenzt.
Alle15 Konsum-/Diagnosezeilen und historische Übergangsinputs werden streng
an tatsächliche Ankünfte, Bedarfe, Typen, Kapazitäten und Zeitpunkte gebunden.
Keine Type-/Stock-/Timer-/Workergrants, keine automatische Hochstufung für Tests.
Hohe Stufen brauchen eigenen Paid-Beleg oder einen konkreten Szenenblocker;
Originalsprites, große Geometrie und vollständiger Gate D bleiben getrennt.


OE25-Abschluss: Die tatsächliche bezahlte Normal-Szene belegt Common6 sowie
Elite11→13→12→13; höhere Grenzen werden aus gespeicherten DES-/Bedarfswerten
benannt, kein Heavenly-PASS. Ein modellabhängiger Typ12-Counterguard sperrt
nur bei stets erfüllter aktueller Abandonedzeile unmögliche positive Inputs;
authored Need1-Modelle behalten ihre tatsächlichen12er-Fehlerereignisse.
Der neu angehängte deaktivierte Elitebefehl wird auch im alten Logisticsprofil
vor Sequenz-/Zustandsänderungen abgefangen; alte Befehle behalten ihren Vertrag.
Die gemeinsame Paper-/Food-/Versorgungswartebedingung betrifft ausschließlich
die bezahlte Prüfszene, keine geänderte Produktions-/Zuzugs-/Versorgungsregel.
Der explizit unmapped Elitefall in OriginalModelMapping erfindet keine
Originalgebäude-/Spriteidentität. Abschluss25 gibt keine26er-Geometrie oder
29er-Originalgrafik als bereits erledigt aus.

Ausdrückliche Nutzerentscheidung am2026-10-18 während der finalen25er-Prüfung:
Nach Abschluss und eigenem geprüftem OE25-Commit wird erneut pausiert.
OE26 bleibt TODO/unbegonnen. Kein Push, PR, Merge, Tag oder Release; erst
ein neuer Fortsetzungsauftrag hebt diese Pause auf.

### Zwischenmerge bis OE-25 – ausdrückliche Nutzerfreigabe am2026-10-18

Nach dem geprüften lokalen OE-25-Commit fragt der Nutzer ausdrücklich:
„so können wir jetzt alles mal mergen und commiten?“ Dies beauftragt Commit,
Push, Pull Request nach `main` und regulären Merge des geprüften Standes
OE-14–25. Der vorherige Push-/PR-/Merge-Ausschluss während der Pause ist
für diesen Zwischenmerge aufgehoben. Die Implementierungspause vor OE-26
bleibt bestehen; keine Freigabe von OE-26–33 oder Gate D–F folgt daraus.

Basis ist `origin/main` `41091b66e960b4c82c378d5ae3b83088325f5a61`, geprüfter
Code-Head `9bac35b5626834b42d35d52c23b11eb039b14f21`. Alle zwölf Taskcommits
bleiben erhalten, ergänzt um diesen Dokumentations-Handoff. Erwarteten PR-Head,
Mergefähigkeit und tatsächliche Checks vor dem regulären Merge prüfen;
anschließend nur Fast-forward-Synchronisierung, kein Squash, Branchlöschen,
Tag oder Release. Die bisherige gemeinsame Endabnahme bleibt ein späterer
Meilenstein; dieser Zwischenmerge behauptet keine vollständige Originalparität.

Die vorhandenen relevanten OE-25-Prüfungen gelten für den unveränderten Code:
177 CTests in Debug/Release/ASan+UBSan,50 getrennte20k-Fortsetzungen und vier
tatsächlich geöffnete Softwareansichten. Große Elite-Footprints26,
Original-Haussprites29, vollständiger Gate D und native/human Abnahme bleiben
offen. Die Handoff-Änderungen betreffen ausschließlich Dokumentation.

### OE-26: ausdrückliche Fortsetzung und Geometrievertrag am2026-10-19

Der Nutzer beauftragt „las unns OE26 machen“. Die Pause ist für OE-26
aufgehoben. Start ist der saubere, mit `origin/main` und Feature-Upstream
abgeglichene Merge `f4447163c6dac035de45a9b1ce9ba2222e0e7546`.
Vor Implementierung gilt der [Elite-Footprintvertrag26](../original-economy/elite-footprints-runtime.md).
Die aktive Entwicklungsrevision wird26; keine Migration von25, keine Änderung
der veröffentlichten Regeln1–9/Schemas1–25 oder der unveränderten policy3.

Eine neue geometrische Objektidentität materialisiert den bezahlten leeren
2×2-Eliteplot vor wirklichem Ersteinzug zu4×4, ohne neue ID, Kauf, Budget,
Bewohner-, Stock- oder Timerkopie. Alle16 Claims und der zukünftige Eingang
werden vorher geprüft. Einheitliche signed Höhe ist ein konservativer authored
Guard nur dieser Transaktion. Bei Konflikt stagniert der leere Plot; spätere
Elite-Stufenwechsel behalten4×4. Es wird weder eine Common→Elite-Konversion
noch eine unbelegte15-stufige Größentabelle oder originale Ankerlogik erfunden.
Hausabriss bleibt gesperrt, synchrone Vorbereitung bleibt ohne Save-Authority.
Der Auftrag umfasst diese Aufgabe und ihren geprüften lokalen Commit;
OE-27 wird durch diesen einzelnen Fortsetzungsauftrag nicht vorgezogen.


### OE-26 abgeschlossen am 2026-10-19: eine geometrische Authority

Plot39 ist ausschließlich leer2×2; Körper40 benötigt wirklichen Ersteinzug und
bleibt für alle Elite12..17 bei4×4. Die exakt vorhandene BuildingId, der bezahlte
Ursprung und HouseState bleiben erhalten. Es gibt keinen direkten Kauf40,
Familienwechsel, freien Bewohner, zweiten Owner oder gespeichertes Sideflag.
Der konservative signed Foundationguard gilt nur dieser authored Transaktion.
Alle16 Claims und tatsächlichen Actor-Kanten werden vor Veröffentlichung
geprüft. Konkurrenz wird pro Haus erneut geprüft, erfolgreiche Cacheänderungen
werden gemeinsam vorbereitet; aktive Pfade/Cargo bleiben unangetastet.
Genau ein aus Körper40 plus Ersteinzugsbeleg abgeleitetes autonomes
Topologieereignis ist in beiden allgemeinen Restoreguards und im
Produktionsledger gebunden; kein zusätzlicher Spielerbefehl wird erfunden.
Die tatsächliche bezahlte Straße blockiert bei120, ihre reguläre Entfernung
liefert keinen Refund/Einzug; erst160 materialisiert und bewohnt den Körper.
Spätere Rückentwicklung erhält4×4 und die realen Bestände/Timer/Bewohner.

Die technische Abnahme liest echte finale Berichte: 231
Konfigurations-CTests,51 getrennte20k-Fortsetzungen,8 geöffnete Softwarebilder,
72 historische Saves/Snapshots und1485 Policyfälle. Der erste Legacybuild-FAIL
wird durch14 explizite Reportlabels behoben, alte Labelbytes bleiben exakt.
Originale Familienseiten2/4 sind belegt; Preis80, Plotumwandlung, Zuzug,
Workeradapter, gezeichnete Fallbackform und Anker bleiben authored.
Originalsprites29, vollständiger Gate D27 und native/human Abnahme bleiben
getrennt offen. OE-27 bleibt TODO; dieser Task erzeugt nur seinen eigenen
lokalen Commit, keinen neuen Push/PR/Merge/Tag/Release.

### OE-27 vor Implementierung: Schwierigkeit und begrenzte Modellsteuer

Der ausdrückliche Nutzerauftrag setzt auf dem sauberen OE-26-Commit
`bcba05334494b4f6f5afd7d4c0d2d76a13469caa` fort. Verbindlich ist vor Codeänderung
[finance-runtime.md](../original-economy/finance-runtime.md): Rule10/Schema26/
Revision27, fünf bereits getypte Modellschwierigkeiten, Modellrate×tatsächliche
Bewohner bei ausdrücklich authored816t/1Funds-Normalisierung und2Funds je
installiertem Worker/400t. Der neue statische Originalnachweis zeigt weitere
Originalparameter; dieser Adapter behauptet deshalb keine Originalerhebung.
Bestehende Geldzähler bleiben alleinige Authority, historische Inputs begrenzt;
unsigned Kürzung erhält signed Debt und alle Treasurygrenzen. Die transparente,
verlustfähige aktuelle Mission hat keinen Bonus oder gespeicherten Siegschalter.
Regeln1–9/alte Saves/Defaults bleiben unverändert. Vollständiger Gate D wird
wirklich geprüft; OE-28/Originalsprites29 bleiben unbegonnen.


### OE-27 abgeschlossen: begrenzte Steuerhistorie und technischer Gate D

Der vorab erklärte Finanzvertrag ist umgesetzt und mit drei frischen
Vollbuilds sowie684 vollständigen Konfigurations-CTests geprüft. Bestehende
Geldzähler bleiben alleinige Authority;15 begrenzte historische Eingabezeilen
und der letzte Eingang ermöglichen modellgebundene Restoreprüfungen.
Die originalen TaxRate-/Kapazitätsmodifikatoren bleiben an allen fünf
Schwierigkeiten0.816t und1Funds/Rate/Bewohner sowie2Funds/Worker/400t sind
explizite OpenEmperor-Regeln, keine vollständige Original-Steuerrekonstruktion.

Zusätzlich wird die aus dem gespeicherten Deadlinewert direkt belegte letzte
natürliche Brandperiode gebunden. Rule10 besitzt keinen FireInspector; der
Zündtick darf Pre-Tick-Steuer erhalten, die folgenden600 Ticks einschließlich
Ablauf nicht. Keine frühere vollständige Brandhistorie wird erfunden. Der echte
Ein-Feld-Redfall8976→9792 ist vor Fix akzeptiert, danach abgelehnt; dieselbe
legitime Kontrolle liefert über20k weiterhin alle fünf Ausgaben bytegleich.
Genuine verzögerte Käufe erzeugen alle Grenzfälle ohne Authorityinjektion.

Technischer Gate D ist bestanden. Vollständige Originalparität/Gesamtabschluss
bleiben PARTIAL: Originalcollector, höhere originale Housing-Szenengrenzen,
Originalsprites29 und native/human Abnahme sind offen. Die
[Finanzabnahme](../original-economy/finance-validation.md) bindet tatsächliche
Suiten, Prozess-Replays, Originalinventar und erhaltene Fehlversuche.
OE-28 wird nicht vorgezogen; der Task erzeugt nur seinen eigenen lokalen Commit.

### OE-28 vor Implementierung: ursprüngliche Hausdiagnose als reine UI

Der ausdrückliche Nutzerauftrag setzt am sauberen OE-27-Commit
`eb40408a57038d775dfa79d3c8e9f1bbf189f80d` fort. Verbindlich ist vor den
Sourceänderungen [housing-inspector-runtime.md](../original-economy/housing-inspector-runtime.md).
Vier Inspectoransichten und drei begrenzte Overlays lesen ausschließlich
bestehende const-World-Diagnosen/Produktionsbelege. Quellenexistenz zählt nie
als tatsächliche Versorgung. Hauslieferung, Stadtproduktion, Reservierung,
Fracht und Konsum werden mit ihrer jeweiligen Bezugsgröße getrennt.
Keine gespeicherte Authority ändert sich: Rule10/Schema26/Policy3/Revision27,
alte Inspector-/Input-/Saveverträge und Defaults bleiben erhalten. Native/human
Abnahme und Originalsprites29 bleiben offen; Gate E folgt erst nach30.

### OE-28 abgeschlossen: Diagnose bleibt ohne gespeicherte Authority

Der vorab erklärte UI-Vertrag ist technisch umgesetzt und geprüft. Vier
Hausansichten und die höchstens20-Haus-Overlays verwenden bestehende
World-Diagnosen, echte Ankunfts-/Food-/Konsumbelege und die getypten Scheduler-
Slots. Relative numerische Bedarfprozente sind ausdrücklich Anzeige, keine
originalen gespeicherten Versorgungsquoten. Neue Sitzungen starten State/Off;
F9 erhält die laufenden Viewentscheidungen ohne sie zu speichern.
93 gezielte Konfigurations-CTests, zwei reale20k-Prozessreplays, zehn geöffnete
Softwarebilder und bytegleiche alte Vergleichspixel belegen die begrenzte
[technische Abnahme](../original-economy/housing-inspector-validation.md).
Rule10/Schema26/Policy3/Revision27 und sämtliche ältere Authority bleiben
unverändert. Keine neue volle Corpus-/Originalparitäts-/native/human-Abnahme;
Originalsprites29 bleiben offen, Gate E gehört zu30. Nur lokaler Task-Commit.

### OE-29 vor Implementierung: belegte Hausbilder als optionale Präsentation

Der Nutzer setzt am sauberen OE28-Commit
`15dd6da9183617f39f711df2b2ab1598f6595cb9` ausdrücklich mit29 fort.
Der [vorab erklärte Vertrag](../original-economy/original-house-visuals-runtime.md)
bindet nur belegte Originaltype-/Footprint-/Asset-/Ankerkombinationen. Die
bisherigen1512/1516/1520 bleiben unveränderte Legacy-Vorschauen. Fehlende
Identitäten oder inkompatible temporäre Körper behalten benannte Fallbacks,
keine geratene Stufe oder gestreckte Originalgrafik. Aktive-only Vorbereitung
teilt den bestehenden physischen Bild-/Texturpool innerhalb alter Budgets;
World-/Save-/Versionauthority bleibt Rule10/Schema26/Policy3/Revision27.
Die konkrete Originalzuordnung wird vor ihrer Implementierung durch den neuen
Quellennachweis gebunden. Gate E folgt30, Gesamtparität und native/human werden
nicht aus Softwarebildern abgeleitet. Nur lokaler eigener Task-Commit.

### OE-29 abgeschlossen: nachgewiesene Typfamilien, statische Repräsentation

Der Originalsetter518de0 und die native Marker-/Body-/Type30-Kette belegen
15 Typ3..17-Bilder und ihre Frontanker. Variante0 ist eine explizite statische
Darstellungswahl, keine neue gespeicherte Variantenauthority. Vacant2 und der
temporäre2×2-Eliteplot behalten klar benannte Fallbacks. Der optionale native
Bildpool und tatsächliche Worldtyp ersetzen keine Regeln/Savefelder; alte
House-Levelprofile bleiben unverändert.

Die [technische Abnahme](../original-economy/original-house-visuals-validation.md)
bindet174 finale CTests,114 neue Softwarebilder und zwölf tatsächlich geöffnete
finale Aufnahmen an denselben564-Datei-Stand. Der unveränderte Produktionstest
fand den Inaktive-Copyumbruch; ohne vorbereitetes Profil ist die alte Zeile
wieder exakt. Alle22 bisherigen Inspector-PNGs und zwei Sandbox-BMPs sind je
Konfiguration bytegleich. Original1464 Dateien und127 Authorityquelldateien
bleiben exakt; keine neue volle Corpus-/20k-/native/human-Abnahme. Gesamtparität
bleibt PARTIAL und ganze Originalgrafik-/Variantenparität UNKNOWN. Gate E folgt30,
der nächste Starterprompt bleibt unbegonnen. Nur lokaler Task-Commit.

### OE-30 vor Implementierung: optionaler bezahlter Normalstart

Der Nutzer setzt ausdrücklich mit30 fort. Der
[Startervertrag](../original-economy/paid-starter-contract.md) öffnet nur den
vorhandenen Rule10-Stand, verwendet normale bezahlte Commands und unveränderte
Model-/Save-/Permissionauthority. Neue Menüentscheidungen bleiben session-only;
alte Defaults und Settings bleiben erhalten. F5/F9 und expliziter manueller
Original-Restore gehören zu30, Original-Recovery bleibt sichtbar bis31 gesperrt.
Gate E verlangt vollständige drei Konfigurationssuites und echte Ablaufbelege;
gesonderte menschliche Abnahme und vollständige Objektgrafikparität bleiben offen.

### OE-30 abgeschlossen: bezahlte bedienbare Originalwirtschaft

Der erklärte Vertrag ist umgesetzt: session-only Menü-O mit fünf Difficulties,
Paid4/Empty und explizitem manuellem Rule10-Restore. Alte Defaults,7–9-Gates,
SettingsV1, Versions-/Modell-/Map-/Revisionsprüfungen und Recoverygrenzen bleiben
erhalten.87 normale Commands kaufen den Starter ohne Zeit/Bestands-/Einwohnergabe.
Die echte Xia/Normal-Kette entwickelt vier Häuser, verbraucht beide Güter,
belegt Ancestor-Ausfall und Erholung sowie reguläre Reparatur und Unterhalt.

Die [Abnahme](../original-economy/paid-starter-validation.md) bindet711 volle
CTests, echte separate20k-/Menü-/F5F9-/Restartprozesse und bytegleiche/geöffnete
Softwarebilder an denselben571-Datei-Stand.72 Legacy-Saves/1485 Policyfälle und
alle1464 Originaldateien bleiben exakt. Gate E ist technisch PASS; vollständige
Originalgrafik-/Gesamtparität bleiben PARTIAL, Human/native und der auf diesem
Host nicht unterstützte Leakdetektor werden nicht als PASS behauptet.
OE31 und spätere Aufgaben bleiben unbegonnen. Nur lokaler Task-Commit.

### OE31 vor Implementierung: enger aktiver Recoveryzugang

Der Nutzer beauftragt ausdrücklich31. Der
[Vertrag](../original-economy/save-recovery-contract.md) öffnet Original-Recovery
nur über einen neuen ausdrücklichen Zugang für Rule10/Schema26/Revision27;
Default-Recovery,7–9-Gates, alte Serialisierung und generischer Restore bleiben
erhalten. Aktive Originalhistories binden wirkliche Map-/Modell-/Worldidentität;
F9-Erfolg beginnt eine neue Historie. Adversariale Tests und tatsächliche
Event-/Recovery-/Neuprozessbelege härten diesen Vertrag. Änderungen an gespeicherter
Authority werden nicht vorgenommen; Gate F und spätere Aufgaben bleiben offen.

### OE31 abgeschlossen: strikte Recovery ohne neue gespeicherte Authority

Der erklärte Zugang ist umgesetzt; die [Abnahme](../original-economy/save-recovery-validation.md)
bindet219 erfolgreiche Konfigurations-CTests und tatsächliche Prozess-/SDL-
Weiterläufe. Die vier beobachteten Fehlerklassen (Historienidentität, Zeitfolge,
Sequenzende und Captureausnahme) werden gezielt behandelt. Ungültige Punkte
isolieren sich, gültige Branches bleiben möglich und Capturefehler erhalten die
aktive Historie mit1200t Backoff. Anfangs fehlgeschlagene Recovery-Captures bleiben
sichtbar ohne erfundene Historie; nach Quellenrückkehr ist reguläres erneutes
Einschalten möglich. Es wird keine zusätzliche Retry-State-Machine oder Migration
eingeführt. Alte Defaults/Savebytes und sämtliche Originaldateien bleiben exakt.

63er-Grundbatch und zehn betroffene Nachprüfungen je Konfiguration sind getrennt
belegt; keine volle241er- oder Gate-F-Abnahme wird behauptet. Native/human und
vollständige Originalgrafikparität bleiben offen. OE32/33 bleiben unbegonnen;
nur der autorisierte lokale Task-Commit wird erstellt.
