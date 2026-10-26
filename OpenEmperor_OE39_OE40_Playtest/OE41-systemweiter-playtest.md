# OE-41 – End-to-End-Playtest des gesamten derzeit verfügbaren OpenEmperor

**Repository:** `https://github.com/oliverjessner/OpenEmperor`  
**Auslöser:** Nach OE-39 und OE-40, **auch wenn eine der beiden Arbeiten wissenschaftlich `BLOCKED/PARTIAL` endet**. In diesem Fall auf der letzten technisch bestandenen, verfügbaren Version testen.  
**Branch:** `test/oe41-system-playtest`, von verifiziertem Abschluss-HEAD des erfolgreichen Entwicklungsstandes. **Kein neues Gameplay-Feature** im Rahmen dieses Auftrags erfinden.

## Primäres Ziel

Führe OpenEmperor als **echtes Spiel** durch die gesamte tatsächlich angebotene Produktoberfläche: Menü, Originaldaten-/Kartenwahl, Stadtbau, Straßen, Transport, Gebäude, Wirtschaft, Waren, Häuser, Dienste, Brände, Gelände, Ansicht, Save/Load, Recovery, Mehrfachstarts und langes Weiterspielen. Prüfe auch separat vorhandene Entwicklungs-/Legacy-Modi und Rendering-Prototypen. Nicht nur Unit-Tests zählen lassen.

Ein echter **menschlicher Playtest** ist von Codex-automatisierten GUI-Events und Offscreen-Screenshots ausdrücklich verschieden. Codex darf die zugängliche Anwendung nativ oder per SDL/Accessibility-Interaktion live testen, aber niemals `HUMAN_PASS` behaupten. Wo es keine nutzbare interaktive macOS-Sitzung gibt, müssen nativer Eingabe-/Fenster-/Finder-Teil `NOT_RUN/BLOCKED` bleiben; dafür eine konkrete menschliche Checkliste übergeben. Automatisierte Software-Renderer-Bilder ersetzen keinen menschlichen Spielbericht.

**Keine Regressionen verstecken:** bisher funktionierende Rule10/11/12, ggf. OE-39/40-Nachfolger, Save-Schemas 1–28 und ggf. spätere, MapPermissions, Originaldateien, Ramp/Bridge und relative Ressourcenpfade erhalten. Die unbekannten Originalfunktionen sind **UNSUPPORTED**, nicht neu implementierte `FAIL`s.

## Vorab lesen und Rahmen aufnehmen

- `AGENTS.md`, `README.md`, `KNOWN_ISSUES.md`, `docs/testing.md`, `docs/testing-input-acceptance.md`.
- `docs/architecture.md`, `docs/gameplay-sandbox.md`, `docs/city-systems.md`, `docs/city-v16.md`.
- `docs/original-economy/{architecture-map.md,player-handoff.md,oe37-sustainable-economy.md,oe38-paid-high-tier-progression.md}`.
- Dokumentierte OE-39- und OE-40-Abschlussberichte, sofern vorhanden.
- `docs/city-v16-{bridges,ramps}.md`, `docs/original-roads.md`, `docs/rendering/{scene-composition.md,original-ramp-cell-prototype.md}` und der unterstützte Original-Map-/Visual-Corpus.
- `src/app/MenuSession.cpp`, `src/app/SandboxView.cpp`, `src/app/SandboxUiLayout.cpp`, sämtliche verfügbaren Build-/Test-Launcher.

### Pflicht-Provenienz

Vor Tests: `git rev-parse HEAD`, Arbeitsbaum, aktive Branches, Feature-Status, genaue Engine-Rule-/Schema-/Policy-Generation, macOS/CPU/SDL-Runtime, Buildkonfiguration, originale Datenquelle, Kartenname, Modell-/SG3-Kompatibilität, Ausgabeordner und aktiver Window-/Rendererpfad festhalten. Kein Zugriff auf persönliche Saves: benutze `--app-root "$PWD/.local/oe41/manual-or-automated"` oder kompatible isolierte aktuelle Startoption.

Wenn private Originalspieldaten fehlen: modellabhängige Tests als `BLOCKED` und einen vollständigen Bericht über die verbleibenden synthetischen Tests erstellen. Niemals eine erfundene Emperor-Datei als echten Gameplay-PASS ausgeben.

## Gate 0 – Inventar ALLER tatsächlichen Bedien- und Gameplay-Funktionen

Erzeuge zuerst einen maschinenlesbaren **Feature- und Menü-Katalog** anhand der **tatsächlichen** Runtime-Registrierung (nicht allein der README): Hauptmenü-Optionen, unterstützte Regeln, Build-Tools je Profil, Bottom Bar, Hotkeys, HUD, Inspector-Tabs, Overlays, Profile, Browser, Baugruppen, Pausen-/Geschwindigkeitsmodi, Save/Load/History, visuelle Fallbacks, CLI-Modi, experimentelle Prototypen.

Jeder Eintrag erhält: `ID`, Name, Profil/Rule, `user_exposed | debug_only | isolated_prototype | unsupported`, Eintrittspunkt, erwartete Aktion, zugeordnetes Playtest-Szenario, Source-Datei, vorhandener automatischer Test und Ergebnis. Ein Feature ohne automatisierte oder manuelle Ausführung darf **nicht** pauschal als getestet gelten.

Erstelle `docs/qa/oe41-capability-matrix.md` oder eine bestehende QA-Datei erweitern; Maschineninventar unter `.local/oe41/inventory.json`.

**Gate 0:** Vollständige Begehung sämtlicher tatsächlich sichtbarer Menü- und Bauaktionen, inklusive bewusst deaktivierter/fehlender Funktionen. Kein Code-Änderungsbedarf für den Bestandskatalog.

## Gate A – Baseline-Stabilität und Datenintegrität

1. Saubere CMake-Builds und vollständige CTests in **Debug, Release und ASan/UBSan** (auf dem Host vorhandene Konfigurationen). Testanzahl vorher per `ctest -N` ermitteln; niemals feste historische 257 oder frühere Zählung als aktuelle Zahl wiederholen.
2. Falls verfügbar `python3 tools/alpha_check.py --system-sdl --jobs 2 --package --data /path/to/emperor` mit **frischen privaten** Ausgabe- und Buildpfaden. Kein veröffentlichtes Binary, keine automatische Distribution.
3. Reproduziere mindestens: gültige Historical-Rule-Saves, explizites Rule10/11/12 Create/Restore, neue OE-39/40-Generation falls implementiert, negative malicious-Saves und Byteidentität in unabhängigen Prozessläufen.
4. Vollständige MapPermissions-Corpus-Prüfung (soweit Originaldaten verfügbar): die historisch gemeldeten **167/165/2** nicht als statische Soll-Ergebnisse ohne erneutes Prüfen verwenden. Keine neue Whitelist nur um Fehler zu entfernen.
5. Originaldateien vor/nach Test mindestens für verwendete Dateien kryptographisch und für den ganzen fixierten Input-Corpus per Inventory vergleichen. Prüfe, dass kein Original in Save, ZIP oder Git gerät.
6. Fehler in Tests nicht durch geänderte Expected-Results verstecken. Halte vorübergehende Fehlläufe mit Ursache/Neulauf getrennt fest.

**Gate A:** reproduzierbare aktuelle Pass-/Fail-/Skipped-Zahlen, Source-/Binary-Hashes, Buildbefehle, vollständige Fehlerliste und konkrete Datenintegrität.

## Gate B – Reale integrierte Spieltests (Produktiv-App, keine zweite Simulation)

Führe für die relevanten unterstützten Modi **echte Menü→Stadt→Spiel→Save→Neustart→Load→Weiterspielen**-Szenarien durch. Die anfangs pausierte Stadt muss von einem wirklichen Spieler-Start stammen; alle Käufe über normale kostenpflichtige Commands; keine gesetzten Einwohner, Waren, Funds, Service-Timer oder Hausstufen.

**Szenarien B01–B20**, Details in `OE41-playtest-matrix.md`:

- B01: Erststart; Spieleordner wählen, falsches Directory sauber ablehnen, Originaldateien unverändert.
- B02: Hauptmenü, Map-Auswahl, Profil- und Schwierigkeitsauswahl, Start/Abbruch/Zurück; Profil-Regel-Label korrekt.
- B03: Kamerasteuerung, 1×/2×/4×, Scroll, Fenster-Resize, Focus/Blur, Modals, Drag-Abbruch, Null-/Rand-/Out-of-window-Input.
- B04: Bauen und Ablehnen: Road/Drag, Ressourcen-/Fonds-Budget, 1×1 und mehrzellige Footprints, Kanten, Doppelbau, geschützte Originalstrukturen.
- B05: Bezahlte Clay→Pottery→Warehouse→Market→House-Kette; echte IDs, Route, Lieferungen, Consumption.
- B06: Food/Hemp-Farmen, einzeln gekaufte Felder, Crop Quality, reale Ernte und House-Verbrauch.
- B07: Märkte/Warenverteiler, Original-Goods-Ledger, Reservation, Cargo, leere/voll blockierte Ziele, Rücklieferung.
- B08: Worker's assigned/required, Betriebspause und Workforce-Priority; keine gratis Produktion oder verschwundener Unterhalt.
- B09: Wasser, medizinische und religiöse Besuche, echte Arrival-Timer; Pausen, Ablauf und Devolution.
- B10: Kultur: Music/Acrobat/Drama, **nur die heute wirklich implementierte** School→House-Kette; keine originale Venue-Zusage.
- B11: Common-Häuser: Eintritt, Bedürfnisse, Ware/Service/Abstieg, Inspector und Sprites; Common10-Retention nach OE-39 getrennt messen.
- B12: Elite 4×4,legales Erstbeziehen 11→13, Flächen-Konflikt, unerlaubte Familienkonversion, höhere Bedürfnisse; Elite17 nur als `PASS` bei echter neuer 16→17-Evaluation (sonst `BLOCKED`).
- B13: FireWatch, echter Inspector-Ankunfts-/Brand-/Protection-Zyklus, einmal pausieren und fortsetzen; separate Rule10-Ablehnung.
- B14: Straßen entfernen, bezahlte Reparatur, echter unterwegs befindlicher Courier mit vorhandener Cargo, Wait/Return/Resume und Source/Target-IDs.
- B15: Unterstützte Bridges/Ramps in **korrekt passenden separaten** City-v16-Rule5-/Rule6-Szenen; legale und illegale Höhen-/Ufer-/Strukturfälle, reale Transporte und Darstellung. Nicht willkürlich als Rule12/13/14-Komponente behaupten.
- B16: Originalkarten/grundlegender Terrain-Renderer, Höhen und nicht spielbare Source-Objekte, Fallback-/Sprite/Overlay-Sichtbarkeit.
- B17: Auswählen aller Bauarten/Actors über Liste und Maus, Details, HUD, Help, F1/F2/F4/F6/F8/F10 soweit vorhanden, F5/F9 und Panel-Switches.
- B18: F5/Neuprozess/F9/History/Autosave/Recovery bei *nichtleerem aktivem Spiel*, inklusive unterwegs befindlicher Walker und belastetem Warenlager.
- B19: Langspiel: bezahlter Rule12-Stadtaufbau als kontrollierte Basis; mindestens 20.400 Ticks, bevorzugt 100.000; tatsächlich verfügbare OE-39/40-Version separat. Source-journal-/Save-Limits ehrlich prüfen. KPI: Funds/Tax/Upkeep, Population, Stock, Consumer misses, Up-/Devolutions, Fire, tick-stable Save.
- B20: Zwei aufeinanderfolgende Spiel-/Menü-/Map-Sitzungen in **einem App-Prozess** und komplette App-Neustarts mit verschiedenen Saved-Rules; keine UI-/Resource-/Settings-State-Leaks.

Alle Szenarien benötigen ein eigenes Ergebnis: `PASS | FAIL | BLOCKED | UNSUPPORTED | NOT_RUN`, genaue Schritte, geöffneter Profil-/Save-Stand, Tick/IDs/Funds vorher/nachher, beobachtete sichtbare oder simulierte Outputs und tatsächliche Beweise.

## Gate C – Realistische Karten-, Visual- und Bedienmatrix

1. Prüfe mindestens vier **unterschiedliche** passende Original-Kartenfälle, soweit nach tatsächlichem Parserstatus rechtmäßig verfügbar: `Cities/Xia.map`, Handan, Badaling und Kaifeng oder dokumentierte Alternativen. **Nicht** automatisch die Scope aller Playthroughs auf beliebige Karten erweitern; legale Szenen können durch konservative MapPermissions fehlen. Gib Map-Grund für jeden Block an.
2. Prüfe einen positiven Original-Road-Transport und einen Fall, in dem eine gesperrte Originalstruktur **nicht** mit World-Commands überschrieben wird. Gate-Tunnel/Brücke/Rampe nur in ausdrücklich freigegebenem Profil.
3. Prüfe Fullscreen/Resize (nur falls UI vorhanden), Fenstergrößen, normale und abweichende Zoomstufen, Überlagerung von Building/Walker/Grass, alpha picking/Hitbox/Anchor, kontrolliertes Umstellen von Präsentations-/Fallbackprofile.
4. Prüfe GUI-Antworten: sichtbarer Button löst exakt gleiche Aktion wie Shortcuts aus, alte Scene lädt mit der passenden HUD-Zeile, keine Touch-/Hover-Aktion verändert World-Snapshot, F5/F9 zerstören keine Geld-/Warenautorität.
5. **Echte Bilder öffnen und inspizieren**; Captures pro repräsentativem Zustand auf 1×/2×/4×, mit Player-Handlungszusammenhang und Build-/Snapshot-/Source-Identitäten. Ein reiner Dateizähler oder Bitmap-Pixel-Snapshot ohne visuelle Prüfung ist nicht ausreichend.
6. Isolierten Original-Rampenprototyp separat **als Prototyp**, nicht als integrierten Gameplay-Erfolg testen. Große Mauer, Quarry, Shore-Landings, Original NPC/Water/Bridge-Ferry müssen separat als Darstellung/teilweise/unsupported klassifiziert werden.

## Gate D – Nativer/macOS- und menschlicher Playtest

Falls im Codex-Host eine echte **macOS-GUI-Session** zugänglich und der Programmstart erlaubt ist: echte OpenEmperor-App statt ausschließlich Dummy-SDL starten. Interaktiv erproben: Maus-Drag, Tastatur, Fokuswechsel, Fenstergröße, 1×/2×/4×, Bau, Löschen, Scroll, Menü-Rückkehr, wiederholtes Speichern/Laden, optional erneutes Öffnen via Finder. Notiere Runtime (Software vs GPU/Metal), Renderername und OS. Benutze isolierten `--app-root`; keine privaten Einstellungen oder Saves anrühren. Ohne reale GUI kann nur `NATIVE_NOT_RUN` gemeldet werden.

Ein **Mensch** muss die Datei `OE41-manual-checklist.md` selbst abarbeiten. Codex bereitet die Liste mit einer repräsentativen Stadt, Spielstand (nur OpenEmperor-eigene Daten, keine Originaldateien), bekannten Shortcuts und ausdrücklich offener Ergebnisspalte vor. Kein `HUMAN_PASS` auf Basis eines Bots. Relevante Befunde später in den Gesamtbericht ergänzen.

## Gate E – Performance, Robustheit und Folgemaßnahmen

- Mindestens echte Langlauf- und Restart-Messungen mit Ressourcenbudget; Frame-/Rendering-Performance getrennt von simulierten Ticks. Kein unbelegtes FPS-Versprechen aufgrund Offscreen-Tests.
- Memory-/Texture-Budget, Encoder/Decoder-/Parser-Refusals, Crash bei fehlender Ressource, Fokus-/Input-Probleme, gescheiterte Save-Replace-Semantik, Lieferhungern, Finanzdefizit, Fallbacks sammeln.
- Ordne alle Fehler nach **P0: Crash/Datenverlust/Regelkorruption, P1: blockierte Kernfunktion oder Haupt-Gameplay-Schritt, P2: sichtbare/kleinere Gameplay-/UI-Defekte, P3: kosmetisch**. Known limitation ≠ neuer Bug. Reproduzierbaren minimalen Weg, erwartetes/aktuelles Verhalten, Screenshot/Log/Save-Pfad privat und betroffene Code-Stelle protokollieren.
- Mache **keinen** großen Simulationsumbau oder Balancing-Change innerhalb des Playtests. Falls ein Test-Harness repariert werden muss, das als eigenen kleinen Commit isolieren und die unveränderte Produktionsfunktion erneut testen. Features gehören in einzeln priorisierte Folge-Tasks.
- Benutze `docs/qa/oe41-playtest-report.md` als konsolidiertes, editierbares Ergebnis, `docs/qa/oe41-capability-matrix.md` als Abdeckungsübersicht, private Logs unter `.local/oe41/`. Keine proprietären Bild-/Originaldateien oder generierte ganze SaveDocuments in Git.

## Pflicht-Abschlussformat

1. **Inventar:** Anzahl der wirklich aufrufbaren Funktionen und statusgebundener Cases; fehlende native/human Fälle separat.
2. **Automatisierte Baseline:** Konfiguration, CTests PASS/FAIL/SKIP und Source-HASH.
3. **Playtest-Szenarien:** B01–B20 einzeln mit beobachtbarem Resultat, inkl. tatsächlich gestarteter Runtime und Datenquelle.
4. **Visual/Nativ/Human:** unabhängige Statusspalten, Foto-/Bild-Provenienz und nicht getestete Fälle.
5. **Blocker/Issues:** P0–P3, Repro, erwartetes/tatsächliches Ergebnis, Datenverlust und Release-Auswirkung.
6. **Gameplay:** Welche Kette war durchgehend spielbar? Welche ist technisch vorhanden, aber praktisch nicht erreichbar? Welche ist nur isolierter Prototyp?
7. **Resultat:** `TECHNICAL`, `SCRIPTED_APP`, `NATIVE_GUI`, `HUMAN`, `RELEASE_READINESS` jeweils `PASS/PARTIAL/FAIL/NOT_RUN`, dazu vollständige Belege.
8. **Nächste Implementierungsreihenfolge** aus den beobachteten Findings, nicht aus erratenem Feature-Wunsch.

**Git:** Eigenen Branch, kleiner lokaler Commit für Inventar/Test-Harness, lokaler Commit für dokumentierten Playtest nach tatsächlicher Ausführung. Kein Push/Merge/Tag/Release. Besonders keine stillschweigende Anhebung historischer Regeln oder Anpassung der erwarteten Ergebnisse, damit ein roter Test grün erscheint.
