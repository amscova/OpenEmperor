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
