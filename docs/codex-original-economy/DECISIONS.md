# DECISIONS – verbindliches Architektur- und Evidenzregister

Dieser Stand enthält zunächst **Planungsentscheidungen**, keine bestätigten Implementierungsergebnisse. Codex ergänzt nach jedem Task Belege und Review-Status.

| ID | Anfangsentscheidung | Status / Beweisbedarf |
|---|---|---|
| D-01 | Kein globaler Merge des `amscova/windows-port` Forks; selektiv unabhängig testen | plan |
| D-02 | Original Model-Datei bei Rule7 read-only/mit Hash einbinden | plan |
| D-03 | City-v16 Rule7 / MapPermissions policy3 / Schema23 als geplanter, opt-in Weg | OE-00: am tatsächlichen HEAD frei; für spätere ausdrücklich gewählte Regeln reserviert, in OE-08/OE-09 erneut prüfen |
| D-04 | Alte Save-Schemas 1–22 unverändert und keine Auto-Migration | zwingende Invariante |
| D-05 | Brücken/Rampen Rule6 von Rule7 weiter benutzen, ihre Authority aber nicht ändern | Abhängigkeiten prüfen |
| D-06 | Original-Model-IDs nur nach eindeutiger Quelle als VERIFIED führen | einzelne IDs offen |
| D-07 | Originalspiel-Simulationsschritte nicht automatisch mit 20Hz World-Ticks gleichsetzen | Zeitskala versioniert/prüfen |
| D-08 | 15 House-Zeilen sind nicht gleich dem aktuellen World-Level 0..2 | eigene Authority erforderlich |
| D-09 | Goods inklusive Alternativen nur mit Realproducer/RealCourier/Conservation | Bedarfssemantik prüfen |
| D-10 | Versions-Gates bis zu einer echten paid Hut->Plain-Cottage-Szene nicht im Standardmenü anbieten | ab OE-21 nachweisen |

## Neue Entscheidungen

Zu jeder Entscheidung bitte ergänzen: Task-ID, Commit, Originalbeleg (`raw`, `exe`, `model` oder `authored`), begründete Alternative, Konsequenz für Save/Tests, Review-Status.

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
  zusätzliches Feld und keine doppelte Warenkonsumregel. Genau fünf begrenzte
  Goldenwerte werden als einzelne Testfakten dokumentiert, keine Massentabelle.
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
