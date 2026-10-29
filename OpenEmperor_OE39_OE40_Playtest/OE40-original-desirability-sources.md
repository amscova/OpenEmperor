# OE-40 – Original Desirability: belastbare Quellen, 4×4-Elite und produktive Integration

**Repository:** `https://github.com/oliverjessner/OpenEmperor`  
**Referenz beim Erstellen:** `main` @ `81bc9218958f7ade116fc43d3df80975c1206e67`. Vor Arbeitsbeginn `fetch`, `status`, HEAD, Commits, aktive Versionsnummern und den tatsächlichen **OE-39-Abschluss** ermitteln.  
**Branch:** `feat/oe40-original-desirability` vom erfolgreich geprüften OE-39-Stand. Wenn OE-39 keinen neuen Code bzw. nur eine Diagnose geliefert hat, von dessen nachweislich unverändertem Baseline-HEAD ausgehen. Keine ungefragten Merges/Rebases oder Überschreibung lokaler Änderungen.

## Mission und ehrlich formulierte Zielgrenzen

OE-38 hat mit realen Modelldaten einen engen, formalen **Rule12/Normal-Blocker** nachgewiesen: Für alle heute zugelassenen, positiv wirkenden Originalwirtschafts-Quellen über einer materialisierten **4×4-Elite-Fläche** liegt der beweisbare DES-Oberwert bei **79**; der modellgebundene Übergang **Impressive Compound 16 → Heavenly Compound 17 verlangt DES ≥ 80**. Dies ist ein Beweis über **unser aktuell zugelassenes Quellensystem**, nicht über das vollständige Original-Emperor.

**OE-40 soll zuerst rekonstruieren, welche originale Quelle oder welcher originale Berechnungspfad fehlt.** Erst bei nachvollziehbar belegter Quelle, Bauadmission, Einfluss-Geometrie, Modellbezug, Kosten, Limits, Lebenszyklus und Save-Autorität darf eine **neue, explizite Regelgeneration** gebaut werden. Keine Garantie eines Elite17-Erfolgs. Wenn die Quellenlage nicht reicht, ist ein exakt dokumentiertes `BLOCKED` ein korrektes Resultat.

Diese Arbeit ist von Marktlogistik OE-39 zu trennen. Die in OE-39 erreichte Common10-Stabilität muss bei produktiven Änderungen erhalten bleiben. Keine freie `+1`-Addition, kein Senken des 80-Schwellwerts, kein willkürliches Erhöhen von Quellenlimits, kein Freistapeln/Überlappen realer Footprints und kein Umschalten von Common- auf Elitefamilie.

## Verbindliche Vorlektüre

1. `AGENTS.md`, `KNOWN_ISSUES.md`, `docs/testing.md`, `docs/original-economy/architecture-map.md`.
2. `docs/original-economy/oe38-paid-high-tier-progression.md`, speziell „Proven Elite17 barrier“ und die echte 79/80-Zertifizierung; `tests/HighTierDesirabilityBound.h` und `tests/HighTierDesirabilityBoundTests.cpp`.
3. `docs/reverse/original-desirability-contract.md` – originaler Ringkern, Source-Order, per-add-Clamps, originale Gebäudeabfrage, explizite Grenzen der Wand-Hypothese.
4. `docs/reverse/original-desirability-source-admission.md`, `original-desirability-source-state.md`, `original-desirability-source-factories.md`, `original-market-desirability-shape.md`.
5. `docs/reverse/original-house-evolution-contract.md`, `original-house-lifecycle-contract.md`, `original-occupancy-conflicts.md` und `docs/original-economy/model-provenance.md`.
6. `src/simulation/WorldOriginalInfluence.cpp`, `src/simulation/OriginalInfluenceState.h`, `src/simulation/original/Desirability.{h,cpp}`, `src/simulation/original/OriginalModelMapping.h`, `src/simulation/WorldOriginalHousingDiagnosis.cpp`, `src/simulation/WorldOriginalHousing.cpp`, `src/persistence/SandboxSave.cpp`, die aktuelle `World`- und Menu-/View-Grenze.
7. OE-39-Abnahme, tatsächlicher Nachfolger-Branch und neuer Save-/Rule-Stand. Ein nicht veröffentlichtes lokales Artefakt nicht mit einem auf GitHub verfügbaren Beleg verwechseln.

### Bereits belegte, aber eng begrenzte Ausgangsfakten

- Der originale Source-Manager verarbeitet nur Quellen mit Status **1 oder 3** und erfordert im beobachteten Pfad zusätzlich das Laufzeit-Wort **`+0x3c == 0`**. Dieses Wort liegt als eigenständiges Base-Archivfeld bei **Byte-Offset 54**, nicht beim `Subindex`-Feld 18. Die Links und ihre Semantik sind nicht identisch mit einer allgemeinen Baupermission.
- Ein Original-Factory-Pfad wurde für **ClayPit35, Kiln43, Warehouse54, CommonMarketSquare59, Well72, Farmhouse193 und HerbalistsStall207** statisch nachverfolgt. Eine Constructor-Bindung beweist **nicht**, dass die dazu ähnlich benannte World-Entität als Originalquelle mit denselben Eigenschaften gilt.
- Die Original-**Warehouse54**-Einflussabfrage arbeitet mit einem abweichenden Seitenmaß; der Original-**Market59** besitzt einen besonderen Rechteckpfad mit berichteten **4×7 bzw. 7×4 Influence-Bounds**. Das ist ausdrücklich **nicht** gleichzusetzen mit einem vergrößerten kaufbaren Markt-Footprint; ein Market in World ist bisher 1×1.
- Der reine originale quadratische Spread über maximal zehn Ringe ist zum Teil überprüft. Die besondere Rechteckphase, negative Source-Wall-Interaktion, Source-Lifecycle und Building-Query-Bonus sind **nicht automatisch** korrekt integriert. Ein aus einer Diagnose bekannter möglicher Bonus ist kein Freibrief zur Vergabe an Elite-Häuser.
- Angelegte Originaldaten, ausführbare `Emperor.exe`, Karten, private statische RE-Ausgaben und SG3/555 bleiben ausschließlich unter **ignoriertem `.local/`**, vollständig read-only. Kein dekompilierter Quelltext oder proprietärer Pixel- oder Modelldump in Git.

## Gate A – Quelleninventar und Gegenprobe (nur Read-only)

1. Freeze: tatsächliches Git-HEAD, `git status`, Rule/Schema/Policy/Revision, Originalmodellhash, Source-EXE-Hash und aktuelle 79er-Zertifizierung festhalten. Fehlen die exakten Originaldaten, kennzeichne jede davon abhängige Prüfung `BLOCKED`, statt sie synthetisch „PASS“ zu melden.
2. Reproduziere die **aktuelle** 4×4-Obergrenze nochmals mit demselben unabhängigen Model-/Geometrie-Checker und der produktiven `DesirabilityMap` (inklusive per-add-Saturation und Source-Order). Bei geänderter OE-39-Regel prüfen, ob ihre Source-Menge unverändert ist; den historischen Rule12-Beweis nicht unbemerkt umetikettieren.
3. Erstelle ein vollständiges **Kandidatenregister** aller aus Quelle und Modell interpretierbaren DES-Beiträge. Für jede Kandidatin: Originaltyp, originale Referenz (Raw-Modellfeld oder nachprüfbarer EXE-Pfad), positiver/negativer Wert, Original-Seiten-/Shape-Callback, Quelldauer/Status, Source-Manager-Admissibility, World-Entsprechung, aktuelles Bau-/Preis-/Workers-/Limits-/Funds-Verhalten, Permissons, Fahrzeug-/Serviceabhängigkeiten, historische Save/Restore-Rechte und Beweisstatus.
4. Untersuche getrennt:
   - bis dato nicht admitted **Bauquellen** (35/43/54/59/193/207) und tatsächliche positive/negative Modellwerte;
   - den **Market59-Special-Shape**-Pfad – archivierte Selector-/Orientierungsbeweise getrennt von physischen 1×1/anderen Footprints;
   - die **Wand-/Terrain-Sonderfälle**, besonders negative Beiträge und die echten Koordinaten-/Sektorregeln;
   - die originale **Building-Query inklusive eines möglichen expliziten Bonus**. Ermittele, welcher konkrete Runtime-Status den Bonus auslöst und ob er auf die Elite-Haus-Abfrage wirkt. Keine erfundene allgemeine +10-Regel;
   - Status 1/3, separates Link-Wort +0x3c, Lifecycle, Placement/Destruction, Source-Prefix-Reihenfolge und eventgebundene Cache-Neuberechnung.
5. Für jede Vermutung muss ein **widerlegender Test** existieren. „Modelname + ähnliche Grafik + DES-Wert“ genügt weder für Originalidentität noch für Konstruktion/Save-Admission.
6. Führe einen separaten **rein mathematischen** Obergrenzen-/Erreichbarkeitstest für alle hinreichend belegten Kandidaten aus. Bei einem DES ≥ 80 **nur unter relaxed/unbaubaren Annahmen**: klar als `PURE_POSSIBLE` markieren. Das ersetzt nicht die legale bezahlte City. Behalte Negative/Per-Source-Sättigung und Original-Sortierreihenfolge bei.
7. Füge `docs/original-economy/oe40-desirability-investigation.md` mit Evidence-Tabelle, Reproduktionskommandos, Hashes, Source-Pins, verbleibenden Unbekannten, zulässigen Implementierungsvarianten und einer begründeten Go/No-Go-Entscheidung hinzu. Größere Rohberichte ausschließlich `.local/original-economy/oe40/`.

**Gate A:** Beweisstatus ist für jede Kandidatin nachvollziehbar. Die Rule12-DES79-Regel wird erneut verifiziert oder eine konkrete, überprüfbare Unstimmigkeit begründet. **Kein Gameplay-Code vor diesem Gate.** Danach lokaler Commit `docs(reverse): identify verified desirability admission candidates`.

## Gate B – Beweispfad wählen, nicht Werte erfinden

### Variante B0 – kein zulässiger Originalmechanismus nachweisbar

Wenn du keinen sicheren Pfad findest, der die fehlende Source/den Bonus mit seinen notwendigen Invarianten erschließt:

- **Keine** neue Rule13/14 Generation und **keine** unkritische Source-Zulassung.
- Zeige präzise, welche statische oder experimentelle Evidenz noch gebraucht würde. Prüfe, welche private PE-Recherche das Loch schließen kann; das Originalprogramm nicht ohne klaren Auftrag starten.
- Dokumentiere `OE40_IMPLEMENTATION=BLOCKED`, Reproduktionsdaten und die bestmögliche Bounds/No-Go-Erklärung.
- Fahre danach mit OE-41, **Playtest der tatsächlich vorhandenen unterstützten Funktionen**, fort. Ein in OE-40 blockiertes Elite17 darf nicht den gesamten QA-Lauf verhindern.

### Variante B1 – durch belegte Originalmechanik gedeckte Erweiterung

Nur wenn Kandidatenwerte, konkrete Quelle, Status-/Links-/Geometriepfad und verträgliche Gameplay-Authority nachgewiesen sind:

1. Lege einen vollständigen, reviewbaren Änderungsvertrag fest: **Name, Footprint (physisch vs. nur Einfluss), Original-Modell-ID, Kosten, Personal, Produkt-/Servicewirkung, Building-/Courier-IDs, Validierung, Source/Removal-Limits, Road/Map-Admission, DES-Wert und Saison/Tick-Lifecycle**. Kein freier Source-Emitter ohne realen bezahlten Besitzer.
2. Lege die nächste freie **explizite** Rule/Schema-Generation fest. Wenn OE-39 `Rule13/Schema29` umgesetzt hat: **OE-40 Rule14/Schema30**, falls frei. Wenn OE-39 Rule12 ohne neue Generation behalten hat: bei Bedarf **Rule13/Schema29**. Niemals bestehende Rule-12/13-/Schema28/29-Saves stillschweigend umdeuten oder überschreiben.
3. Aktualisiere nur die erforderlichen `World`, `MapPermissions`, Modellmapping, Source-Manager/Cache, Dispatcher, UI/Menu sowie die **strikte** Serializer-/Parser-/Restore-Validierung. Keine zweite parallele Influence-/Save-Autorität. Keine Dateizugriffe/BFS/volle Kartenscans im Render-/Hover-/Framepfad. Caches bei legitimen erfolgreichen Quellereignissen aktualisieren.
4. Quellen, die **negative** DES-Beiträge blockieren, dürfen das Ergebnis nur über den belegten Originalalgorithmus ändern; aktive Wall-Rendergraphik allein besitzt keine Authority. Ein Rechteck-Special-Shape ist nicht derselbe Spread wie der quadratische Kern.
5. Dokumentiere und sichere adversarial: falsche Source-ID/Typ/Status, Link-Zyklen, geänderte Modelldatei, verschobene Footprints, falsche orientierte Shape-Payloads, überlappende Plätze, unzulässige Source-Limits, erschlichene Gratisgebäude, nicht passende Funds/Workers, Markt-/Warehouse-Goods und falsche Save-Versionen. Ablehnungen müssen IDs/Funds/World/Saves atomisch erhalten.
6. UI zeigt tatsächliche DES-Werte und Provenienz; keine visuelle Source suggeriert beim bloßen Rendern Versorgung.

**Gate B:** Neue Authority ist explizit versioniert, unter echten validierten Map-/Funds-Regeln lauffähig, im selben `World`, mit bestandenen negativen und historischen Save-Regressionsfällen. Lokaler Commit `feat(original): add verified versioned desirability sources` nur bei implementierter, belegter Produktionsfunktion.

## Gate C – Bezahlter Elite-Test (bei B1) und Erhaltungsregression

1. Auf realer `Cities/Xia.map`, Difficulty Normal und passenden Originaldaten ausgehend von einer **frischen** Stadt: echten **4×4 Elite Plot** bezahlt kaufen, rechtmäßig materialisieren, Bewohner über normale Immigration/admission erhalten. Common/Elite-Familien nie konvertieren.
2. Erzeuge alle erforderlichen Originalwaren (Tea/Silk/Bronze oder Lacquer, Paper), geeignete Medizin, Religion, Kultur/Drama und Food **mit tatsächlich bezahlten Rezept-/Liefer-/Walker-Ketten**. Verwende den bestehenden OE-39-Markttransport und Rulesmodel, keine handgesetzten HouseStocks, Timers, Gäste, Funds oder Typen.
3. Prüfe jede Elite-Lebensphase durch **reale** `World::evaluate_original_houses()`-Ereignisse. Pro Phase DES-/Bedürfnis-, Waren-/Besuchs- und Kostenprovenienz, Eintritts-/Rückentwicklungsbeleg und nachverfolgbaren HouseType-Zustand protokollieren.
4. Gewünschter Erfolg: tatsächliches bezahltes **16→17** bei DES **≥80** und aktuellen+nächsten modellgebundenen Needs. Danach getrennt **mindestens 20.400 Ticks** beobachtete Retention/Versorgung und Betriebsbilanz. Kein reiner `DesirabilityMap`- oder `HouseEvolutionRules`-Test als realer Stadterfolg.
5. Falls der weitere Bau-/Service-/Goods-Pfad nicht innerhalb vorhandener Ressourcenlimits belegbar ist: dokumentiere den **ersten** neuen echten Blocker (Baufläche, Workers, Warehouse, Market, Branchenlimit, DES, Timer, Funds, fire, save-history-cap), statt Typ17 zu simulieren.
6. Überprüfe auf jedem positiven Zwischenschritt Waren- und Funds-Conservation, FireInspector- und Originalservice-Historie, Event Order und Tax, atomische Command-Refusals, Save/F5/F9/Recovery sowie echte **getrennte Creator-/Restore-Prozesse** mit byteidentischen vollständigen SaveDocuments und Snapshots.
7. Führe vollständige betroffene CTests unter Debug, Release, ASan/UBSan aus; ergänze mindestens einen echten 1×/2×/4× SDL-Sandbox-Capture und öffne die PNGs. Alte Rule10/11/12/OE-39-Fälle einschließlich deren gespeicherter Byte-Werte müssen unverändert bleiben.

**Gate C:** Ergebnis getrennt berichten:
- `DES_RESEARCH`: VERIFIED/PARTIAL/BLOCKED
- `SOURCE_IMPLEMENTATION`: PASS/PARTIAL/BLOCKED/NOT_RUN
- `PAID_ELITE17`: PASS/PARTIAL/BLOCKED/NOT_RUN
- `HISTORICAL_SAVE_COMPAT`: PASS/FAIL/NOT_RUN
- `NATIVE_HUMAN`: PASS/FAIL/NOT_RUN

Nach erfolgreicher, positiver produktiver Abnahme lokaler Commit `test(original): validate paid elite17 progression and historic saves`. Wenn ein gewünschter Stadterfolg trotz belegter Mechanik nicht erreicht wird: `PARTIAL` mit reproduzierbarem Nachweis, nicht grüner Schein-Erfolg.

## Abschlussdokument und Red Lines

Aktualisiere am Ende `docs/original-economy/oe40-desirability-investigation.md`, `KNOWN_ISSUES.md`, den Spielerleitfaden, die vorhandenen Original-RE-Verträge und ggf. Versionierungsmatrix. Private Binär-/Sprite-/Testergebnisse bleiben unter `.local/original-economy/oe40/`. Belegten Originalstand getrennt von authored Adaptern benennen.

**Nie ohne gesicherte Evidenz:** +1 DES-Bonus, abgesenkter 80er-Schwellwert, freie Quellduplikate, Ignorieren von Footprints, gelockerter Source-Manager, HouseType/Save-Injection, Migrationszwang, generelle Wandabschirmung oder das einfache Umdeuten eines 1×1-Markts in ein 4×7-Gebäude.

**Git:** Eigenen Branch, je bestandenem Gate **lokaler Commit**, exakte SHAs und Testkommandos im Schlussbericht. Kein Push, Merge, Tag, Release oder überschreibende Aktion. Originaldaten unverändert.

**Wichtig für die Fortsetzung:** OE-41 wird unabhängig vom Erfolg des Elite17-Gates gegen alle tatsächlich verfügbaren Spielfunktionen ausgeführt. Es darf den fehlenden Elite17-Proof als erwarteten `UNSUPPORTED/BLOCKED` notieren, muss aber den Rest aktiv spielen und prüfen.
