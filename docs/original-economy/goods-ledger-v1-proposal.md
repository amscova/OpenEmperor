# OE-13: freigegebener Goods-Ledger-Vertrag V1

**ANGENOMMENER VERTRAG; technische Abnahme des Buchhaltungsfundaments siehe
[OE-13-Testprotokoll](../codex-original-economy/PROGRESS.md#oe-13-umsetzung--2026-10-16-basisaf660b72).**

Dieser Vertrag beschreibt die ausdrücklich angenommene Anpassung
des [OE-13-Auftrags](../codex-original-economy/prompts/13-typisierte-warenbestaende-und-conservation-ledger.md).
Der Nutzer hat die konkrete Freigabefrage mit „Ja, diesen Vertrag umsetzen
(empfohlen)“ beantwortet. Dies beauftragt Rule9/Schema25 und genau die unten
beschriebene gestufte Abnahme; keine neuen Warenquellen oder Folgetasks.
Prüfstand ist der zunächst saubere HEAD
`bf4e821a591f141b0a300c848b4ecbdbe3e40fe8`. Die unten genannte Boundaryprobe
prüft ausschließlich die vorhandenen Rule7-/Rule8-Verweigerungen. Diese
private Probe und drei vorhandene Release-CTests sind tatsächlich gelaufen;
Resultate stehen im [Fortschrittsregister](../codex-original-economy/PROGRESS.md).
Diese historischen Vertragsprüfungen waren keine Warenimplementierung. Die
Umsetzung beginnt am sauberen `main`
`af660b72c281e5c1b8735a29a941f474a3f33d8d`; aktuelle Ergebnisse stehen im
Fortschrittsregister, nicht in den historischen Proberesultaten.

## Warum eine Entscheidung erforderlich ist

Der ursprüngliche Prompt verlangt eine Erweiterung von Rule7. Inzwischen sind
Rule7/Schema23 eingefroren. Der anschließend ausdrücklich angenommene
[OE-12-Vertrag](source-contract-v1-proposal.md) und seine
[technische Umsetzung](desirability-runtime.md) erlauben unter Rule8/Schema24
bezahlte Well72-Einflussquellen, aber weiterhin keine Waren, Producer, Courier,
Worker, Bevölkerung oder Einnahmen. Die aktuelle
[Versionsentscheidung](../codex-original-economy/DECISIONS.md) darf nicht durch
eine nachträgliche andere Auslegung von „Rule7“ oder „Originalwirtschaft“
umgangen werden.

Der Warenkern aus OE-06 verarbeitet ausdrücklich übergebene Beobachtungen.
Seine positiven numerischen Beispiele erzeugen keine Waren in einer World.
Reale bezahlte Ceramics-Produktion gehört zu
[OE-14](../codex-original-economy/prompts/14-vorhandene-pottery-produktion-als-ceramics-versorgung-adaptieren.md),
Dispatch, Bewegung und tatsächliche Marktankunft zu
[OE-15](../codex-original-economy/prompts/15-markttransport-fuer-originale-waren-durch-reale-courier-routen.md)
und schedulergebundener Verbrauch zu
[OE-17](../codex-original-economy/prompts/17-verbrauch-exakt-nach-gespeicherter-original-zyklusphase.md).
Diese Aufgaben werden durch OE-13 nicht vorgezogen.

Die frische private Probe
`.local/original-economy/oe13/contract/boundary-report.json` prüft gegen die
vorhandenen Produktionsbibliotheken: Rule7 und Rule8 kaufen jeweils ein Haus,
Rule8 zusätzlich eine Well72-Quelle; jeweils vier Wirtschaftsbaubefehle werden
mit unverändertem Snapshot verweigert. Waren, Bevölkerung, Workforce und
Courier bleiben null, die Vacant-House-Beobachtung lehnt Goods-Lieferung ab,
ein restaurierter Ceramics-Bestand beziehungsweise Produktionszähler von eins
wird verweigert und jeweils 20.000 Ticks ändern ausschließlich die Uhr.
Rule9 ist am Prüfstand nicht unterstützt. Die Probe verwendet authored
Modell-/Kartendaten; sie ist kein Originalwaren- oder OE-13-Implementierungs-PASS.
Befehl und Exitcodes stehen in
`.local/original-economy/oe13/contract/probe-run.json`.

## Angenommener Versions- und Runtimevertrag

Freigegeben ist **City-v16 Rule9 / Schema25 / unveränderte policy3** als eigener,
expliziter Entwicklernachfolger von Rule8. Die Nummern waren am historischen
Prüfstand frei und wurden vor Umsetzung erneut geprüft. Die getrennte
[Implementierung](goods-ledger-runtime.md) verwendet jetzt genau diese Nummern.
Rule1–8 und Schema1–24 behalten ihre bisherigen Authority-, Tick-,
Restore- und Byteverträge. Es gibt keine automatische Migration und keine
Neudeutung eines vorhandenen Rule7- oder Rule8-Spielstands.

Rule9 übernimmt ausschließlich die bereits freigegebenen Fähigkeiten von
Rule8: 2000 Startfunds; bezahlte Straßen, Brücken und Rampen nach ihren
bisherigen Verträgen; höchstens 20 bezahlte Vacant-Häuser; Well72-Container
2×2 für 60 Funds, höchstens vier lebende Quellen; Abriss ohne Refund,
beibehaltene Kaufhistorie, eindeutige IDs und denselben Desirability-Kern.
Die Source-/Finanzgrenzen und die unveränderliche Modellbindung werden nicht
neu interpretiert. Die Well72-Quelle erhält auch unter Rule9 keine Warenfunktion.

Hinzu kommen vollständig geprüfte Buchhaltungsdatentypen und eine lesende
Warenbilanz. **Alle sechs Waren bleiben in einer echten Rule9-World zunächst
`unsupported` und mengenmäßig null.** Es gibt keine neuen Producer, Lager,
Märkte, Courier-Rollen, bezahlten Importe, Worker, Bewohner, Services,
Steuereinnahmen oder Tick-Verbrauch. Der bestehende Scheduler bleibt zeitlich
unverändert; seine Consumption-Markierung bucht noch keinen Verbrauch.

Eine neue Versionsnummer löst die offenen Gameplayvoraussetzungen nicht:
Die vorhandenen bezahlten Häuser sind Vacant mit null Bewohnern und ohne
Workforce. Die gewöhnliche leere House-Beobachtung erhält durch
`goods_demand` keine Lieferberechtigung. Ein positiver numerischer Bedarf oder
die vorhandene Pottery-Darstellung autorisiert weder Warenproduktion noch
einen Lieferaufruf. Diese Grenzen bleiben ausdrücklich erhalten.

Die normale Anwendung, Menüauswahl und Recovery bleiben geschlossen. Nur ein
ausdrücklich gewählter Entwickleradapter darf Rule9 erzeugen oder laden, mit
eigenem privaten Savepfad. Der bisherige Rule8-Launcher darf dadurch weder
seinen Default noch seinen Savepfad wechseln. Expliziter Rule9-Restore ist
erforderlich; ein Schema25-Dokument darf nicht über den Rule8- oder generischen
Restorepfad angenommen werden.

## Warenidentitäten und eine einzige Bestandsauthority

Die sechs bereits vorhandenen
[`original::Goods`](../../src/simulation/original/GoodsRules.h) bleiben die
typisierten Identitäten des neuen Ledgers: Hemp=0, Ceramics=1, Tea=2, Silk=3,
Bronzeware=4 und Lacquerware=5. Sie sind getrennt vom alten
[`simulation::Good`](../../src/simulation/World.h) mit Goods, Clay, Pottery und
Food. Pottery und Ceramics werden hier weder gleichgesetzt noch umgebucht.
FoodQuality bleibt ein Qualitätsbyte und wird kein siebter Warenbestand.
Eine Original-Tonnage oder physische Lagereinheit wird nicht behauptet.

Die bisherigen numerischen Werte von `Good`, `CourierRole`, `Object` und ihren
Save-Identitäten bleiben exakt. Für diese Buchhaltungsstufe sind keine neuen
spielbaren Rollen oder Objekte erforderlich. Später tatsächlich benötigte
Enumwerte dürfen ausschließlich angehängt werden.

Für Häuser sind ausschließlich die sechs bestehenden Signed-word-Felder in
[`OriginalHouseState::house`](../../src/simulation/OriginalEconomyState.h)
beziehungsweise [`HouseState`](../../src/simulation/original/HouseEvolution.h)
Bestandsauthority. Ein generischer zweiter House-Stock, ein Ledger-Spiegel
derselben Bestände oder eine zusätzliche gespeicherte House-Gesamtsumme ist
ausgeschlossen. Die arithmetische House-Domain ist 0..32767 pro Gut; das ist
keine Aussage über eine rekonstruierte physische Originalkapazität.

Der allgemeine Transaktionskern darf daneben typisierte Nicht-House-Bestände
mit vorhandener `BuildingId` und Cargo mit vorhandener `CourierId` darstellen.
Er erfindet keine Gebäude oder Courier-IDs. Eine Nicht-House-Zeile darf kein
Haus spiegeln; ein Cargo darf nicht gleichzeitig noch einmal als alter
`CourierState::cargo`-Bestand derselben Originalware gezählt werden.
Eigentümer, Typ und zugehörige Authority müssen eindeutig sein. In der echten
OE-13-World existieren weder ein zulässiges Warenlager noch ein Originalwaren-
Courier: Nicht-House-Stock-, Cargo- und Reservierungslisten bleiben leer.
Auch die Well72-Container erhalten keine Lagerzeilen.

Pro Gut werden lediglich historische Ein- und Ausgänge als geprüfte
nichtnegative Zähler geführt: `produced`, `imported`, `consumed`, `demolished`.
Stock und Cargo sind aus den jeweils einzigen Bestandsauthoritys abgeleitete
Summen, keine zusätzlich veränderbaren gespeicherten Gesamtkonten. Reservierte
Mengen sind ebenfalls abgeleitete Verpflichtungen und keine zusätzliche Ware.
In der echten Rule9-World bleiben alle vier Zähler je Gut null; insbesondere
bleibt `imported` bis zu einem gesonderten Importvertrag null.

## Vollständiger numerischer Transaktionskern

Der reine Kern muss positive wie negative Buchungsfälle tatsächlich verarbeiten
und prüfen können. Seine synthetischen Tests dürfen explizit als authored
deklarierte Konten und Mengen übergeben. Das ist kein erlaubter Weg, solche
Werte in eine spielbare World einzusetzen.

Jede Buchung prüft sämtliche beteiligten Identitäten, Mengen, Kapazitäten,
Reservierungen und Zähler vor der Veröffentlichung. Erfolg veröffentlicht
genau einen vollständigen neuen Zustand; Ablehnung oder Exception erhält den
gesamten vorherigen Zustand einschließlich aller Historien und IDs.
Hausbestände liegen in 0..32767; einzelne Nicht-House-Bestände, Cargo- und
Reservierungsmengen in 0..INT32_MAX; historische Zähler in 0..UINT64_MAX.
Diese arithmetischen Domains behaupten keine physische Ingame-Kapazität.
Alle Operationen und Aggregationen prüfen vor Addition oder Subtraktion,
ob ihre jeweilige Domain erhalten bleibt. Ungültige Gut- oder Eigentümer-IDs, negative Mengen,
Überlauf, Unterdeckung und doppelte Referenzen werden verweigert, nicht
geclamped oder repariert.

Für jedes Gut gilt separat:

`produced + imported = consumed + demolished + stock + cargo`

Die Gleichung wird mit geprüften Summen ausgewertet; ein Unterlauf durch
unsigned Subtraktion darf keinen falschen PASS ergeben. Umlagern, Laden,
Liefern, Rückführen und Reservieren verändern die historische Nettomenge
nicht. Ein dokumentierter Produktions-/Importeingang erhöht genau ein
historisches Konto und genau den zugehörigen physischen Bestand; Verbrauch
oder ausdrücklich zugelassener Verlust reduziert einen vorhandenen Bestand
und erhöht genau das entsprechende Ausgangskonto. Eine Herkunfts- oder
Verlustbuchung ist kein öffentlicher World-Befehl. **Es gibt keinen öffentlichen
Stock-Credit, Grant-Befehl oder Testschalter zum Auffüllen einer echten World.**

Reservierungen benötigen eine eindeutige Bindung an Gut, vorhandenen Courier,
Quelle, Ziel und ihren Transaktionszustand. Source-Reservierungen dürfen den
noch vorhandenen, nicht anderweitig reservierten Stock nicht überschreiten.
Incoming-Reservierungen dürfen zusammen mit dem Zielbestand dessen ausdrücklich
deklarierte Kapazität nicht überschreiten. Der ursprüngliche Nachfüllwert
`max(need, 10)` ist keine belegte Lagerkapazität.
Die reine synthetische Prüfung verwendet ausdrücklich vorgegebene, begrenzte
Kontokapazitäten. Eine spätere Runtime-Kapazität muss unveränderlich aus einem
gesondert freigegebenen Eigentümertyp folgen; sie darf nicht durch einen frei
änderbaren Savewert erteilt werden. Für noch nicht freigegebene Eigentümertypen
gibt es in OE-13 keine positive Lagerkapazität.

Der vollständige numerische Lebenslauf ist:

1. Reservieren bewegt keine Ware. Derselbe Auftrag darf Stock oder Zielraum
   nicht zweimal reservieren.
2. Laden reduziert Quellstock und erhöht Cargo um dieselbe tatsächliche Menge;
   eine zugehörige Source-Reservierung wird exakt einmal freigegeben.
3. Eine bestätigte Zielübergabe reduziert Cargo und erhöht ausschließlich den
   Zielbestand um dieselbe tatsächliche Menge. Die entsprechende Incoming-
   Reservierung wird passend reduziert. Teilmengen müssen vollständig bilanziert
   sein; Restcargo bleibt vorhanden.
4. Ein Abbruch vor dem Laden gibt nur Verpflichtungen frei. Nach dem Laden
   darf ein Abbruch keine Ware löschen, zurückteleportieren oder neu erzeugen:
   noch vorhandenes Cargo bleibt Cargo, bis eine gesondert bestätigte Rückgabe,
   andere zulässige Übergabe oder ausdrücklich autorisierte Verlustbuchung
   erfolgt. Doppelte Abbrüche oder erneute Ankünfte dürfen nicht erneut buchen.

Diese Operationen modellieren Zahlen und Transaktionszustände, keine Bewegung
oder Ankunft. Die spätere World-Anbindung muss einen wirklichen Produktions-
beziehungsweise Arrival-Event beweisen; der numerische Kern selbst erzeugt
diesen Beleg nicht. Quellkapazitäten, Rezepte, Worker und Routen werden hier
nicht aus Model-Labels, Well-Nähe oder synthetischen Zahlen abgeleitet.

## Read-only Bilanz und strikter Save

Die Bilanz-API liefert für jedes Gut Unterstützung, historische Zähler,
abgeleiteten Stock und Cargo sowie getrennte Reservierungsinformationen.
Bei einer nicht vorhandenen Produktions-/Transportfähigkeit lautet der
Runtimebefund `unsupported` mit sämtlichen Mengen null. Ein nur mathematisch
gültiges Konto ist kein Beleg für einen unterstützten Warenkreislauf.
Die Unterstützung folgt ausschließlich aus der freigegebenen Runtime-
Capability; ein Aktivierungsschalter wie `supported` ist im Save verboten.
Abfragen erzeugen keine Mutationen, Modellvorbereitung, Dateizugriffe,
World-Kopien, Routen oder Ticks. Ein neuer Waren-HUD ist nicht Teil dieser
Freigabe; die API dient zunächst Tests und einer späteren Inspectoranbindung.

Schema25 erhält einen eigenen strikten Originalwarenpayload und einen
expliziten Rule9-Writer/Reader/Restorepfad. Der Writer muss Rule9 vor dem
vorhandenen Rule8-/Schema24-Zweig behandeln. Schema1–24 dürfen den neuen
Payload oder neue Warenauthority nicht aufnehmen. Unbekannte, fehlende oder
doppelte Keys, falsche Zahlentypen, negative Werte, Domainüberschreitungen,
duplizierte/fehlende/ordnungswidrige Eigentümer und verwaiste Referenzen sind
abzulehnen. Die vorhandenen Grenzen von 8 MiB Dokumentgröße und Tiefe32 bleiben
unverändert; Duplicate-Keys werden auch in jedem neuen verschachtelten Objekt
verweigert. Bestehende Collection-Budgets werden nicht erhöht. Die neue
Warenauthority hat genau sechs historische Güterkonten und höchstens die
vorhandenen 20 House-Referenzen; Nicht-House-Stock-, Cargo- und
Reservierungslisten müssen bei OE-13 leer sein. Source-/Entfernungslisten
behalten die bisherigen Rule8-Grenzen. Es gibt keine gespeicherte zweite House-Stockkopie, Bilanzcache,
Modellkopie oder zusätzliche route-authority.

Der Rule9-Restore prüft neben Conservation auch die **zulässige Entstehung der
Warenauthority aus Vertragsereignissen und die aktuellen Capabilities**.
Das ist keine Straßen- oder House-Erreichbarkeitsprüfung und erlaubt keinen
zusätzlichen BFS. Bisher zulässige unverbundene Häuser oder Well72-Quellen
bleiben zulässig. Weil es vor OE-14/15/17 keinen zulässigen positiven Ursprung,
Courier oder Verbrauch gibt, muss er jeden positiven Originalwarenbestand,
Cargo-, Reservierungs- oder historischen Zähler verweigern. Das gilt auch für
ein rechnerisch ausgeglichenes manipuliertes Dokument mit beispielsweise
`produced=1` und `stock=1`. Conservation allein genügt nicht.
Map-/Policy-/Modellbindung und atomarer Austausch von World und Sessionmodell
bleiben wie im bisherigen Entwicklervertrag erforderlich.

## Angenommene, ausdrücklich geänderte OE-13-Abnahme

Die Pflichtfälle aus dem ursprünglichen Prompt werden auf zwei belegbare
Ebenen aufgeteilt. Der Nutzer hat diese Änderung ausdrücklich angenommen;
sie wird nicht als unveränderte ursprüngliche Abnahme ausgegeben.

| Ebene | Verpflichtender Nachweis | Grenze der Aussage |
|---|---|---|
| Positiver reiner Transaktionskern | Für alle sechs Güter Reservieren, Laden, Teil-/Volllieferung, Freigabe, Abbruch vor/nach Laden, Rückgabe und explizite Ein-/Ausgangsbuchungen mit exakter Conservation | Authored numerische Konten, keine World-Produktion oder Actor-Ankunft |
| Negative Transaktionen | Null-/fehlender Stock, volles Ziel, doppelte Reservation/Ankunft/Abbruch, falsches Gut/Owner/Courier, Mengen- und Summenüberlauf; jeder Fehler erhält den gesamten Input | Keine abgeschwächte Prüfung aufgrund eines No-op oder eines anderen früheren Fehlers |
| Echte Rule9-World | Bezahlte 20-Haus-/Well72-Szene mit unveränderten Kosten und Source-/Abrisshistorie; alle sechs Waren `unsupported`/0; Tick und Queries erhalten die Warenauthority | Kein positiver Warenkreislauf und keine House-Lieferberechtigung |
| Persistenz und Prozess | Tatsächliche F5/F9-Handler, strikter Schema25-Restore, ein separater neuer OS-Prozess und identische endliche Tick-/Commandfortsetzung mit vollständigem SaveDocument-/Bytevergleich | Positiver Mid-Courier-Restore erst mit OE-15; kein injiziertes Cargo als Ersatz |
| Manipulation und Nichtregression | Ausgeglichene wie unausgeglichene positive Warenfälschungen verweigern; fehlende/geänderte Modell-/Map-/Policyquelle und Schreibfehler erhalten aktuelle World, Modell und Save; Rule1–8/Schema1–24 exakt erhalten | Keine automatische Migration oder Freigabe der normalen Anwendung |

Zu den reinen Grenzfällen gehören Signed-word-Hausheadroom, sämtliche maximal
darstellbaren Konten und historische Summen, getrennte Bronze-/Lackbestände
trotz gemeinsamem Modellfeld sowie eine unbeteiligte Ware, deren Zustand bei
einer anderen Transaktion unverändert bleibt. Eine rein synthetische
Transaktionsfortsetzung darf als solche getestet werden, ersetzt aber keinen
World-F5/F9- oder Prozessnachweis.

Der echte World-Nachweis muss zusätzlich eine Quelle bauen, entfernen und
erneut kaufen, ohne eine Warenbuchung auszulösen. Kein Producer-Kauf und kein
Legacy-Pottery-/Food-Cargo darf durch das neue Gate gelangen. Negative
Restore-/Commandfälle vergleichen den vollständigen Zustand und relevante
abgeleitete Cachezähler vor und nach der Verweigerung. Mehrere unabhängige
Sessions dürfen weder ihre Bestände noch ihre Modellbindung teilen.

Zur Abnahme sind die neuen Tests und relevanten bisherigen Goods-,
Rule7-/Rule8-, Save-, SDL- und Bridge-/Ramp-Tests in Debug, Release und
ASan/UBSan auszuführen. Die vollständigen Suiten sind zusätzlich am jeweils
zuständigen Phasen-Gate erforderlich; OE-13 zieht Gate C nach OE-19 nicht vor.
Der exakte Legacyvergleich ist um eingefrorene Rule7-/Schema23- und
Rule8-/Schema24-Belege zu ergänzen; die frühere OE-00-Referenz allein enthält
beide späteren Verträge noch nicht. Bestehende Schema23-Goldenfälle bleiben
zusätzlich erhalten. Originaldaten und private
Reports bleiben innerhalb der bestehenden Read-only-/`.local`-Grenzen.
Tatsächliche Resultate werden ausschließlich im
[Fortschrittsregister](../codex-original-economy/PROGRESS.md) eingetragen.

Der begrenzte Abschluss lautet ausschließlich **„OE-13 Buchhaltungsfundament
nach diesem geänderten Vertrag technisch abgenommen“**. Er ist kein PASS
für einen Warenkreislauf, Gate C, positive Produktion, Transport oder Konsum.
Eine Freigabe von OE-14 oder späteren Aufgaben folgt daraus nicht automatisch;
deren noch offenen Quellen-, Worker-, Housing- und Arrival-Verträge sind
gesondert zu behandeln.

## Konkret freigegeben

1. Rule9/Schema25 als eigener Entwicklernachfolger bei unveränderten Regeln
   1–8, Schemas1–24 und policy3; weiterhin kein normaler Spielerzugang.
2. Die ausdrücklich geänderte Abnahme: vollständige positive **synthetische
   Transaktionsprüfung**, aber ausschließlich bezahlte echte World-Szene mit
   sechs `unsupported`-Nullbilanzen und tatsächlichem Reload/Prozessreplay.
   Echte positive Produktions-, Transport- und Konsumtests bleiben OE-14/15/17.
3. Keine Erweiterung der bereits angenommenen 2000-Funds-/Well72-Grenze und
   keine Bestands-, Import-, Worker-, Bevölkerungs- oder Zeitinjektion zur
   Herstellung eines positiven Tests.

Diese drei Punkte sind angenommen. Prompt und Quality-Gates wurden vor
Implementierung entsprechend fortgeschrieben. Die tatsächliche begrenzte
Abnahme ist im Fortschrittsregister dokumentiert; Gate C bleibt bis zu seinem
eigenen späteren Nachweis offen. Historische Boundaryproben oben werden dadurch
nicht nachträglich zu Implementierungstests.

## Geprüfte Grundlagen

- [OE-13-Prompt](../codex-original-economy/prompts/13-typisierte-warenbestaende-und-conservation-ledger.md),
  [Reihenfolge](../codex-original-economy/IMPLEMENTATION_ORDER.md),
  [Quality-Gates](../codex-original-economy/QUALITY_GATES.md) und
  [OE-12-Entscheidung](../codex-original-economy/DECISIONS.md).
- [Pure Goods-API](../../src/simulation/original/GoodsRules.h),
  [Implementierung](../../src/simulation/original/GoodsRules.cpp) und
  [vorhandene numerische Tests](../../tests/OriginalGoodsRulesTests.cpp).
- [Originaler Liefervertrag](../reverse/original-goods-delivery-contract.md)
  und [Konsum-/FoodQuality-Evidenz](../reverse/original-goods-consumption-food-contract.md):
  Beobachtungen über Mengen und House-Zeilen, keine Produktions-/Routeauthority.
- [House-Authority](../../src/simulation/OriginalEconomyState.h),
  [World-Datentypen und Gates](../../src/simulation/World.h),
  [bisherige Savepfade](../../src/persistence/SandboxSave.cpp),
  [Rule8-Savetests](../../tests/OriginalInfluenceSaveTests.cpp) und
  [privater Prozessrunner](../../tools/test_original_influence.py).
- Historischer finaler OE-12-Nachweis:
  `.local/original-economy/oe12/implementation/final-validation.json` und
  `.local/original-economy/oe12/influence-fixtures/run-6oyqms32/report.json`.
  Diese Belege werden durch den Entwurf nicht umetikettiert oder neu ausgeführt.
