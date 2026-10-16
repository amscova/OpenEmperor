# DECISIONS – verbindliches Architektur- und Evidenzregister

Dieser Stand enthält zunächst **Planungsentscheidungen**, keine bestätigten Implementierungsergebnisse. Codex ergänzt nach jedem Task Belege und Review-Status.

| ID | Anfangsentscheidung | Status / Beweisbedarf |
|---|---|---|
| D-01 | Kein globaler Merge des `amscova/windows-port` Forks; selektiv unabhängig testen | plan |
| D-02 | Original Model-Datei bei Rule7 read-only/mit Hash einbinden | OE-11: explizites unveränderliches Sessionmodell, einmalige typisierte Vorbereitung; Live-SHA/Größe bei Save/Restore; kein Modellzugriff im Tick |
| D-03 | City-v16 Rule7 / MapPermissions policy3 / Schema23 als geplanter, opt-in Weg | OE-08: Rule7 frei und ausschließlich Entwickler-Gate; policy3 unverändert, Schema23 in OE-09 ausschließlich Entwickler-Persistenz; keine Laufzeitfreigabe |
| D-04 | Alte Save-Schemas 1–22 unverändert und keine Auto-Migration | zwingende Invariante |
| D-05 | Brücken/Rampen Rule6 von Rule7 weiter benutzen, ihre Authority aber nicht ändern | OE-08: explizite begrenzte Capability-Gates; bezahlter vollständiger Rule6/7-Vergleich und Legacy-Replay bestanden |
| D-06 | Original-Model-IDs nur nach eindeutiger Quelle als VERIFIED führen | einzelne IDs offen |
| D-07 | Originalspiel-Simulationsschritte nicht automatisch mit 20Hz World-Ticks gleichsetzen | OE-10: authored Mappingversion1 bindet 1 Tick/Phase an Rule7/Schema23; 51/16/12-Counterstruktur beobachtet, elapsed-Kalenderepoche0 getrennt vom Originaljahr; tatsächliche Prüfungen im Fortschritt |
| D-08 | 15 House-Zeilen sind nicht gleich dem aktuellen World-Level 0..2 | eigene Authority erforderlich |
| D-09 | Goods inklusive Alternativen nur mit Realproducer/RealCourier/Conservation | Bedarfssemantik prüfen |
| D-10 | Versions-Gates bis zu einer echten paid Hut->Plain-Cottage-Szene nicht im Standardmenü anbieten | ab OE-21 nachweisen |

## Neue Entscheidungen

Zu jeder Entscheidung bitte ergänzen: Task-ID, Commit, Originalbeleg (`raw`, `exe`, `model` oder `authored`), begründete Alternative, Konsequenz für Save/Tests, Review-Status.

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
