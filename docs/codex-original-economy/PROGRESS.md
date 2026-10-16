# PROGRESS – bei jeder Codex-Aufgabe aktualisieren

**Status-Vokabular:** `TODO`, `IN_PROGRESS`, `PASS`, `FAIL`, `BLOCKED`, `NOT_RUN`. Belege zu jedem PASS echte Tests.

Codex: Einen Task-Commit erzeugen, danach den SHA in der **Antwort** nennen. Bei der **nächsten** Aufgabe die vorige SHA in dieser Datei nachtragen, weil ein Commit seine eigene SHA nicht enthalten kann.

| ID | Status | Commit-SHA (im Folgecommit nachtragen) | Testbelege (Pfad/Run) | Blocker/Notiz |
|---|---|---|---|---|
| OE-00 | PASS | 05470049fa4acfdf2d0104f5a25384c9277073ac | `.local/original-economy/oe00/{reference,verification,validation}/` | 45 relevante CTests,13 Harness-Tests,12 Fork-Untertests;72 Saves/Snapshots und1485 Policyfälle wiederholt identisch; keine Simulation geändert |
| OE-01 | PASS | 11211bbe6f579b6171615f6e75ed6fffbfd53cdf | `.local/original-economy/oe01/`, OE-00/`oe01-verification/` | Raw-Reader/Provenance;6 finale CTests PASS;72 Saves/1485 Policyfälle identisch; keine World-Anbindung |
| OE-02 | PASS | df13b2e9bc0112d66c188276b5753c92ce6a5a40 | `.local/original-economy/oe02/`, OE-00/`oe02-verification/` | Faktenregister v1/typed IDs/5 Goldens;12 CTests PASS;72 Saves/1485 Policyfälle identisch |
| OE-03 | PASS | 12e2f22c94f60402824d8dd0898b9d36086da503 | `.local/original-economy/oe03/`, OE-00/`oe03-verification/` | Isolierter Kern/experimentelle Mauern/Vergleich;33 CTests plus6 finale Wiederholungen PASS;72 Saves/1485 Policyfälle identisch |
| OE-04 | PASS | a8fd0ce0615d7009eecc373359c8c00b1a3207d4 | `.local/original-economy/oe04/`, OE-00/`oe04-verification/` | Pure16-Typ-/15-Zeilen-Evolution;39 CTests plus12 gezielte Wiederholungen PASS;72 Saves/1485 Policyfälle identisch |
| OE-05 | PASS | 269e80d850ea7d0e8d90b4853a48cf86f8140bf8 | `.local/original-economy/oe05/`, OE-00/`oe05-verification/` | Neun Timer/drei Visitpfade/elf Provider;33 CTests plus6 Wiederholungen PASS;72 Saves/1485 Policyfälle identisch |
| OE-06 | PASS | 88e070865600b39d7296be7a6c821724c5cbe682 | `.local/original-economy/oe06/`, OE-00/`oe06-verification/` | Pure sechs Waren/Delivery/Konsum/Quality;48 CTests PASS;72 Saves/1485 Policyfälle identisch |
| OE-07 | PASS | 26faf654c6d714a88c4b72ffb29c5e1d0b936add | `.local/original-economy/oe07/`, OE-00/`oe07-verification/`; `core-parity.json` | 74 echte Vergleichsfälle/vier Forkmodule;507 CTests PASS;Gate A bestanden;72 Saves/1485 Policyfälle identisch |
| OE-08 | PASS | ac052762fba501a85f241a4828726c2a9deb1e90 | `.local/original-economy/oe08/`, OE-00/`oe08-verification/` | Explizite Rule7-Entwickler-Gates;513 CTests PASS;72 Saves/1485 Policyfälle identisch;Gate B offen |
| OE-09 | PASS | — | `.local/original-economy/oe09/`, OE-00/`oe09-final-verification/` | Schema23/OriginalHouse;519 CTests plus33 finale Wiederholungen PASS; echte F5/F9-/Prozess-Roundtrips und72 Legacy-Saves/1485 Policyfälle identisch;Gate B offen |
| OE-10 | TODO | — | — | — |
| OE-11 | TODO | — | — | — |
| OE-12 | TODO | — | — | — |
| OE-13 | TODO | — | — | — |
| OE-14 | TODO | — | — | — |
| OE-15 | TODO | — | — | — |
| OE-16 | TODO | — | — | — |
| OE-17 | TODO | — | — | — |
| OE-18 | TODO | — | — | — |
| OE-19 | TODO | — | — | — |
| OE-20 | TODO | — | — | — |
| OE-21 | TODO | — | — | — |
| OE-22 | TODO | — | — | — |
| OE-23 | TODO | — | — | — |
| OE-24 | TODO | — | — | — |
| OE-25 | TODO | — | — | — |
| OE-26 | TODO | — | — | — |
| OE-27 | TODO | — | — | — |
| OE-28 | TODO | — | — | — |
| OE-29 | TODO | — | — | — |
| OE-30 | TODO | — | — | — |
| OE-31 | TODO | — | — | — |
| OE-32 | TODO | — | — | — |
| OE-33 | TODO | — | — | — |

## Phasen-Gates

| Gate | Status | Beleg |
|---|---|---|
| A | PASS | OE-07: gepinnter Vier-Modul-Vergleich/74 Vektoren, Parser-/Quellenbindung/Negativfälle,507 CTests in drei Konfigurationen und exakter Legacy-Replay; begrenzte Kernabnahme, keine Gesamt-Originalparität |
| B | TODO | — |
| C | TODO | — |
| D | TODO | — |
| E | TODO | — |
| F | TODO | — |

## Fortlaufendes Testprotokoll

Format: `Zeit/Revision – Task – Konfiguration – tatsächlich ausgeführter Befehl – PASS/FAIL/SKIP – Log-Pfad – Aussagegrenze`.

### OE-00 – 2026-10-16, Basis fef111d

- Vor Beginn: `git status --short`, Branch und Upstream überprüft; nur das
  gelieferte unversionierte Auftragspaket vorhanden. Feature-Branch
  `feature/original-economy-v1` am tatsächlichen `main` erstellt. Initiale42
  Paket-Hashes stimmen mit dem gelieferten MANIFEST überein.426 bereits
  versionierte Source-/Test-/Tool-/Resource-Dateien sind bytegleich zum HEAD;
  Beleg `.local/original-economy/oe00/initial-state.json`.
- **Debug:** `ctest --test-dir .local/market-walkers-finish/debug-build -N` und
  eingeschränkte Auswahl der15 in [baseline.md](../original-economy/baseline.md)
  genannten Tests: **15/15 PASS**,611.42s.
- **Release:** dieselbe reale15er-Auswahl in `.local/reswall/release-build`:
  **15/15 PASS**,37.78s.
- **ASan/UBSan:** dieselbe Auswahl, RelWithDebInfo und
  `OPENEMPEROR_ENABLE_SANITIZERS=ON`, in
  `.local/market-walkers-finish/sanitizer-build`: **15/15 PASS**,150.86s;
  keine Compiler-/Sanitizer-Meldungen.
- Exakte Configure-, Inventory-, Build- und CTestbefehle stehen in
  `.local/original-economy/oe00/validation/{debug,release,sanitizer}.json`;
  Ausgaben in den gleichnamigen `*-*.log`-Dateien. Dies ist die relevante
  OE-00-Auswahl, keine vorgezogene vollständige Phasen-Gate-Suite.
- `python3 tests/OriginalEconomyBaselineTests.py -v`: **13/13 PASS**; geänderte
  Savebytes/Snapshots, fehlende/duplizierte/fehlerhafte Policyfälle, fremde Builds,
  Datenüberlappung und Symlink-Ausgabefallen werden verweigert. Ausschließlich
  synthetische Werkzeugtests.
- Gebundener Fork: `d861d2912337c09751cb68196f85d5a30752726e`,11 ahead/4 behind.
  Elf exakte Fork-Blobs geprüft, fünf World-/Save-Schlüsseldateien blob-identisch
  zum aktuellen Upstream. Unabhängiger Diagnose-Probe mit ASan/UBSan **PASS**.
  Unveränderte Fork-`OriginalRulesTests.cpp` gegen seine vier Originalmodule und
  die frisch gebaute identische World-Library: **12/12 Untertests PASS**, inklusive
  gelesener Originalinstallation, kein Skip. Befehle/Logs im
  [Forkvergleich](../original-economy/fork-comparison.md) und unter
  `.local/original-economy/oe00/`. Kein vollständiger Windows-/Fork-Build und kein
  unabhängiger Originalparitätsbeweis.
- Erster vollständiger `original_economy_baseline.py capture`: **PASS**;
  72 SaveDocuments und Snapshot-Projektionen,1485 Policyfälle,11 Rampen- und10
  Bridge-Neustartpaare. Die vorhandenen Runner bestätigen zusätzlich50
  unveränderte alte vollständige Savebytes gegen ihre frühere Baseline.
  Quell- und Originalinventar vor/nach dem Run identisch.
- Zweiter vollständiger `original_economy_baseline.py verify` mit den finalen
  Build-/Pfadguards: **PASS**. Neuer bezahlter Ablauf, neue Prozesse, alle72
  vollständigen Savebytes/Snapshot-Projektionen und1485 Policyfälle stimmen
  exakt mit dem gesperrten Vertrag überein. Digest
  `12823612834d27099189c18e31ed4fed4761c9ff34486227b3075cf1069a49e5`.
  Beleg `.local/original-economy/oe00/verification/{run,contract}.json`.
- Abschlussvergleich: alle426 bereits versionierten Dateien weiterhin exakt
  zum Start-HEAD; eigene Task-Änderungen bestehen `git diff --cached --check`,
  lokale Dokumentationslinks sind sauber. Die34 unveränderten gelieferten
  Prompt-Dateien behalten ihre je drei absichtlichen Markdown-Zeilenumbrüche
  mit zwei Endleerzeichen; diese102 von Git gemeldeten Eingabe-Formatstellen
  werden nicht als eigene Änderung bereinigt. Ihre MANIFEST-Hashes stimmen.
  Keine neue Runtime-/Save-Authority. Gate A bleibt bis OE-07 offen.

### OE-01 – 2026-10-16, Basis05470049

- Vor Beginn: Arbeitsstand und tatsächliches Upstream-`main` geprüft;
  `fef111deff4cedd3d3fa48663556d41b2e4b31a6` weiterhin unverändert. Drei
  unversionierte Root-Auftragskopien bleiben unangetastet. Vorige OE-00-SHA
  oben nachgetragen. Originalbestand vor Beginn1464Dateien/819711091Byte,
  ManifestSHA `dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`.
- Lokale Originalgrammatik und die beiden gebundenen Fork-Dateien read-only
  untersucht. Unabhängig implementierter Reader mit festen Struktur-/Speicher-
  grenzen, explizitem Fehlerresultat, getypten Raw-Zugriffen und SHA-Provenance.
  Die originale fehlende House-Schlussklammer und äußeres CSV-Padding werden
  akzeptiert; innere Leerfelder, Duplikate, zusätzliche Zahlen und kaputte
  Sektionen nicht still übergangen. Kein Fork-Code importiert, keine
  Difficulty-Arithmetik oder originale World-Identität behauptet. Details und
  Unterschiede zum Fork: [model-reader.md](../original-economy/model-reader.md).
- **Debug, Release, ASan+UBSan:** jeweils frisch konfiguriert, vor erstem CTest
  `ctest --test-dir BUILD -N` mit158 registrierten Tests, gezielter Build der
  Targets `openemperor-original-model-data-tests` und
  `openemperor-model-inspect`. Jeweils
  `ctest --test-dir BUILD --output-on-failure -R '^original-model-data-(synthetic|real)$'`:
  **2/2 PASS**, insgesamt6 CTests; synthetisch5413Checks und mit Originaldaten
  8163Checks je Konfiguration. Exakte Befehle/Exitcodes in
  `.local/original-economy/oe01/{debug,release,sanitizer}.json`, vollständige
  Ausgaben in gleichnamigen `*-*.log`. Build ohne Warnungen, keine
  Sanitizer-Meldung. Keine vorgezogene vollständige Phasen-Gate-Suite.
- Vor Erweiterung des Testkatalogs bestand dieselbe gezielte2-CTest-Auswahl
  bereits in allen drei Konfigurationen (5299 synthetische/566 reale Checks);
  nach den ergänzten Grenzfällen wurde sie vollständig erneut gebaut und
  ausgeführt. Die obigen Logdateien dokumentieren den finalen Stand. Ein
  direkter strenger Compiler-/Probe-Lauf des Readers und des Testprogramms
  bestand ebenfalls; diese Vorproben ersetzen keine CTest-Abnahme.
- Synthetischer Katalog:60 beschädigte/fehlende Zahlen,25 tabellarische
  Zeilen-/Sektionsfälle, zusätzliche feste Rowcounts/Difficulty-/Klammer-/
  Legend-Tests;768 deterministische Mutationen (191 akzeptiert/577 verweigert),
  jeweils wiederholt gleiche Ergebnisse/Hashes oder Diagnosen. Input-/Zeilen-
  grenzen, signed int32Min/Max, sparseID-Grenzen, fehlende Accessorwerte und
  ungültige Difficulty ohne Clamp, fehlende/zu große/nicht reguläre Dateien,
  sichere/entweichende/dangling Symlinks und schreibgeschützte Quellen geprüft.
  Nur vollständig validierte Modelle werden zurückgegeben.
- **Optionaler Datentest:** separater Release-Build unter
  `.local/original-economy/oe01/missing-source-build` mit absichtlich fehlender
  `OPENEMPEROR_ORIGINAL_MODEL_TEST_ROOT`; `ctest -N -R '^original-model-data-'`
  vor Build/gezieltem Lauf: `original-model-data-real` tatsächlich **SKIPPED**
  über Exit77, nicht als Datentest-PASS ausgegeben. Befehle/Log in
  `.local/original-economy/oe01/missing-source{.json,-*.log}`. Der normale reale
  Datentest lief dagegen in allen drei Konfigurationen ohne Skip.
- Diagnose-CLI gegen lokale Originaldatei: **PASS**,34218Byte,269Building-
  und15House-Zeilen, je5Modifier, SHA
  `7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6`.
  Bericht `.local/original-economy/oe01/model-provenance.json` enthält keine
  Tabellen. CLI-Negativprüfung für fehlende/beschädigte Datei und fehlendes
  Argument:3 erwartete Fehlercodes/Diagnosen **PASS**,
  `.local/original-economy/oe01/inspector-negative.json`.
- Source-Identität:425 von426 vorhandenen Dateien der ursprünglichen
  OE-00-Startbasis bytegleich; allein CMake erhält additive, unabhängige
  Library-/CLI-/CTest-Targets. Keine vorhandene World-/Map-/Render-/App-/Save-
  Datei geändert, die Library wird nicht produktiv gelinkt. Beleg
  `.local/original-economy/oe01/legacy-source-identity.json`.
- **Vollständiger Legacy-Vergleich:**
  `python3 tools/original_economy_baseline.py verify --data .local/gog-extracted/app --build .local/reswall/release-build --reference .local/original-economy/oe00/reference/contract.json --output .local/original-economy/oe00/oe01-verification`:
  **PASS**. Alle72 vollständigen Savebyte-Verträge und72 Snapshot-Projektionen
  stimmen mit OE-00 überein; alle1485 Policyfälle auf167 vollständigen Managern
  (165 vorbereitet/2 unveränderte Konflikte) bleiben gleich. Elf Rampen- und zehn
  Brücken-Neustartpaare inklusive+100Tick-Kontrollen; zusätzlich50 alte
  Savebyte-Vergleiche der unveränderten Runner bestanden. Bezahlte Rule4-Xia-/
  Handan-Abläufe, Transport-/Pixel-/scripted-UI-Checks liefen ebenfalls über die
  bestehenden Produktionsfixtures. Bericht/Logs:
  `.local/original-economy/oe00/oe01-verification/`, Vertragsdigest weiterhin
  `12823612834d27099189c18e31ed4fed4761c9ff34486227b3075cf1069a49e5`.
  Quellhash während des Runs und Originalinventar vor/nach dem gesamten Task
  unverändert. Keine native/humane oder Originalspiel-Parität daraus abgeleitet.
- Abschluss: neue API, Parser, Diagnoseprogramm, CMake und Tests unabhängig
  reviewed; keine blockierenden Befunde. Dokumentationslinks und Diffprüfung
  sauber. Task-Sourcehashes unter
  `.local/original-economy/oe01/validated-task-source.json` festgehalten. Eigener
  lokaler Task-Commit folgt gemäß Auftrag; Gate A bleibt bis OE-07 offen.

### Merge-Vorbereitung – 2026-10-16, Basis d73536c

- Nutzer hat den Branch selbst gepusht und Commit-/Branch-Merge ausdrücklich
  beauftragt. Aktueller Branch sauber, `origin/feature/original-economy-v1`
  entspricht `d73536c7fd1e024de6e26955586c5d91b2760657`; Live-`main` weiterhin
  `fef111deff4cedd3d3fa48663556d41b2e4b31a6`. Die zusätzliche Nutzeränderung
  enthält ausschließlich die drei gelieferten Root-Dokumente.
- Unabhängiger Merge-Review: keine blockierenden Codebefunde. Die Root-Dateien
  bekommen gültige Links und Pflichtdateipfade zum kanonischen Paket; keine
  zweite Fortschrittsdatei und keine Änderung der Prompt-Abfolge. OE-01-SHA
  nachgetragen. Lokale Dokumentationslinks und `git diff --check`: PASS.
- Frisch ausgeführt: `python3 tests/OriginalEconomyBaselineTests.py -v`:
  **13/13 PASS**; `ctest --test-dir .local/reswall/release-build --output-on-failure -R '^original-model-data-'`:
  **2/2 PASS**, echter Datentest ohne Skip. Die unabhängige Review-Instanz
  bestätigt dieselben Resultate. Parser/CMake/Tests sind weiterhin bytegleich
  zum in OE-01 mit Debug/Release/ASan und vollständigem Legacy-Replay geprüften
  Stand; kein erneuter voller Replay wegen reiner Dokumentänderungen.
- GitHub: `main` ungeschützt, keine Branch-Rulesets, keine Status-/Check-Runs
  am gepushten HEAD. PR-/Mergezustand wird nach Ausführung live verifiziert;
  Gate A und OE-02 bis OE-33 bleiben unverändert offen.

### OE-02 – 2026-10-16, Basis7426484

- Start: sauberer Checkout und Live-`main`
  `742648466f41f09c4724aa29e5a196a52de79e44` geprüft. Vorhandenen
  `feature/original-economy-v1` per Fast-forward auf diesen Merge angeglichen.
  Ursprünglicher Modell-/EXE-Bestand bleibt read-only; Anfangsinventar unter
  `.local/original-economy/oe02/initial-state.json`:1464Dateien/819711091Byte,
  ManifestSHA `dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`.
- Originallegende, alle37 Feldpositionen,15 Hauszeilen und die sieben
  Fork-Funktionskandidaten unabhängig gegen konkrete World-Objects geprüft.
  [Faktenregister v1](../original-economy/model-provenance.md) dokumentiert
  Semantik, Units, Transformationen, Textzeilen und alle fünf Evidenzklassen.
  Physical-/Zeit-/Risiko-/weitere Steuerdetails bleiben ausdrücklich unbekannt.
  SourceLabels werden aus der Datei behalten; benannte Schema-Zugriffe prüfen
  sie zusätzlich. Raw-Parsing und bestehende Regeln bleiben unverändert.
- **Original-EXE:** `python3 .local/original-economy/oe02/check-exe-units.py`: PASS.
  Zwölf begrenzte Fenster,785Instruktionen/2120geprüfteInstruktionsbytes,
  null Abweichungen, EXE vor/nach identisch. SHA
  `6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e`.
  Reproduzierbare Adressen und Aussagegrenzen im
  [Reverse-Vertrag](../reverse/original-model-contract.md), private Ausgaben
  unter `.local/original-economy/oe02/`. Originalspiel nicht gestartet.
- Beide Difficulty-Transformationen unabhängig statisch belegt; C++ prüft
  originale32-Bit-Zwischenwerte vor Division/Clamp. Der Root-Review hat einen
  zu schwachen reinen Quotienten-Guard vor dem ersten Build zu Produkt-/Summen-
  Guards präzisiert. Das ist eine dokumentierte sichere Ablehnung außerhalb
  der belegten Domain, keine beliebige Wrap-Emulation. Bronze/Lack bleiben ein
  OR-Feld; keine doppelte Warenkonsumregel importiert. Keine neue World-Authority.
- **Debug, Release, ASan+UBSan:** frisch konfiguriert, reale Registry mit
  `ctest --test-dir BUILD -N` ermittelt (160Tests), Targets
  `openemperor-original-model-data-tests`, `openemperor-original-model-schema-tests`
  und `openemperor-model-inspect` gebaut; danach
  `ctest --test-dir BUILD --output-on-failure -R '^original-model-(data|schema)-(synthetic|real)$'`:
  jeweils **4/4 PASS**, insgesamt12CTests. Raw-Reader5433synthetische/8163reale
  Checks; Schema2942synthetische/4600realeChecks je Konfiguration. Die beiden
  Originaldatentests liefen tatsächlich ohne Skip. Exakte Befehle/Exitcodes
  und Logs: `.local/original-economy/oe02/{debug,release,sanitizer}.json`
  sowie zugehörige `*-*.log`; keine Compilerwarnungen oder Sanitizer-Meldungen.
  Vor Konfiguration war der neue Targetname erwartbar nicht registriert; der
  frühe Buildversuch ohne Reconfigure galt nicht als bestandener Test. Separate
  Syntaxprüfungen der neuen API, Mapping-Header und Testdatei mit allen
  Projektwarnungen bestanden ebenfalls. Kein vorgezogener voller Phasen-Gate-Lauf.
- Schema-Katalog: explizite37Feldindizes, sieben IDs/15Row-Labels, alle fünf
  Difficulty-Stufen, falsche Enums/Typen/Labels und Overflow-/Clamp-Grenzen.
  Alle13World-Objects geprüft: sieben inferred-Kandidaten/sechs ohne Mapping;
  Bildregistrierung von Well/Herbalist bleibt ein separater Beleg. Kein
  Lookup von Produktion/Profil/Fußabdruck oder originalen Spielregeln abgeleitet.
- **Golden:** genau fünf dokumentierte Originalwerte über unabhängigen
  Text-Punkt-Scanner, fixe Zeilen/Spalten, originalen SHA und Named-API
  abgeglichen. Zusätzlich2255Difficulty-Queries gegen gelesene Werte und
  geprüfte32-Bit-Intermediates. Keine Massentabelle, keine Originalfixture im Git.
- **Absenz und fehlerhafte Quellen:** eigener Release-Build
  `.local/original-economy/oe02/missing-source-build`; zuerst `ctest -N`, dann
  beide Real-CTests mit absichtlich fehlendem Datenpfad tatsächlich **SKIPPED**
  über77. Kein Datentest-PASS daraus abgeleitet. Befehle/Logs unter
  `.local/original-economy/oe02/missing-source{.json,-*.log}`.
  `python3 .local/original-economy/oe02/check-real-modes.py`: **10/10 PASS**
  für erwartete Exitcodes bei Missing/Damaged/Directory/Dangling/Escape und
  syntaktisch gültiger abweichender Golden-Revision. Beleg
  `.local/original-economy/oe02/real-mode-matrix.json`; Quellenbytes unverändert.
- Source/API/Tests und Dokumentation unabhängig reviewed; keine blockierenden
  Befunde. Der Review-Nachtrag zur bereits implementierten Overflowgrenze wurde
  in der Doku korrigiert. Task-Sourcehashes unter
  `.local/original-economy/oe02/validated-task-source.json`; der vollständige
  Legacy-Vergleich läuft mit unverändertem Sourcehash. Gate A bleibt offen.
- **Vollständiger Legacy-Vergleich:**
  `python3 tools/original_economy_baseline.py verify --data .local/gog-extracted/app --build .local/reswall/release-build --reference .local/original-economy/oe00/reference/contract.json --output .local/original-economy/oe00/oe02-verification`:
  **PASS**. Alle72 vollständigen Savebyte-Verträge und72 Snapshot-Projektionen,
  alle1485Policyfälle auf167Managern (165vorbereitet/2unveränderte Konflikte),
  elf Rampen-/zehn Brücken-Neustartpaare und die50vorhandenen alten
  Savebyte-Vergleiche bleiben exakt. Bezahlte Rule4-Xia-/Handan- und bestehende
  Transport-/Pixel-/scripted-UI-Checks liefen über unveränderte Fixtures.
  Bericht/Logs `.local/original-economy/oe00/oe02-verification/`; Vertragsdigest
  weiterhin `12823612834d27099189c18e31ed4fed4761c9ff34486227b3075cf1069a49e5`.
  Sourcehash während des Runs sowie gesamtes Originalinventar vor/nach dem
  Task identisch. Kein neuer App-/World-/Save-/Policy-/Renderer-Aufrufpfad.
- Abschluss:15Dokumentationslinks und Diffprüfung sauber, exakt validierter
  Task-Source eingefroren. Eigener OE-02-Commit sowie delegierter PR/Merge
  folgen nach diesem PASS. Originalspiel-/Human-Parität und Gate A bleiben
  ausdrücklich offen; nächster geplanter Task OE-03.


### OE-03 – 2026-10-16, Basis d1a082d

- Sauberen Checkout und Live-`main`
  `d1a082d4f9457690367f27a5fe7099904f5b90ac` geprüft; vorhandenen Feature-
  Branch per Fast-forward angeglichen. Originalinventar weiterhin 1464 Dateien /
  819711091 Byte, derselbe Manifesthash wie OE-00/OE-02. Quelle:
  `.local/original-economy/oe03/source-state.json`.
- **EXE-Belege:** `python3 .local/original-economy/oe03/check-desirability-evidence.py`:
  **PASS**, 15 begrenzte Fenster / 1233 Instruktionen / 3853 Bytes und zwei
  Datenprüfungen, null Abweichungen, EXE unverändert. Der neue
  [statische Vertrag](../reverse/original-desirability-contract.md) bindet diese
  Belege an EXE-/Modellhash und benennt die offenen Mauergrenzen. Originalspiel
  nicht ausgeführt. Wesentliche Fork-Korrektur: `step <= 0` ändert den Wert
  nach jedem Ring. Keine Übernahme des unbewiesenen Default-`Intended`.
- **Kern:** Eigenständige SDL-/World-freie Library mit modelgetriebenem
  Vier-Feld-Adapter, expliziten Footprints, geordneten Quellen, Quadratperimetern,
  Vorzeichen-/Nullbehandlung, Clamp je Addition und vollständigem Hausmaximum.
  Grenzen und Reproduktion in
  [desirability.md](../original-economy/desirability.md). Konstruktor und ganze
  Batches werden vor Mutation geprüft; Original-Mauerparität bleibt unbekannt,
  `ExperimentalUniform16` ausdrücklich optional/defaultOFF.
- **Build/CTest:** Vor dem ersten Lauf in allen drei neu konfigurierten Builds
  `ctest --test-dir BUILD -N` ausgeführt: 162 registrierte Tests. Debug,
  Release und RelWithDebInfo mit ASan/UBSan bauten die beiden Model-Testtargets,
  Model-Inspector, Desirability-Test/Compare sowie City-v13-Testtarget sauber.
  Jeweils **11/11 ausgewählte CTests PASS**, insgesamt 33 Ausführungen:
  vier `original-model-{data,schema}-{synthetic,real}`, zwei
  `original-desirability-{synthetic,real}` und fünf
  `sandbox-city-v13-{base,persistence,districts,determinism,endurance}`.
  Laufzeiten der Auswahl: Debug 278.46 s, Release 12.32 s, Sanitizer 60.22 s.
  Exakte Configure-/Inventory-/Build-/CTestbefehle und Exitcodes:
  `.local/original-economy/oe03/{debug,release,sanitizer}.json`; vollständige
  Ausgaben in den gleichnamigen `*-*.log`. Keine Compilerwarnung oder
  Sanitizerdiagnose. Dies ist die relevante Auswahl, kein vorgezogenes Gate A.
- Nach Ergänzung der Richtungs-/Mehrzellen-Mauerfälle, Translation und falscher
  Quellenlabels den finalen neuen Testtarget in **allen drei Konfigurationen
  erneut gebaut** und die beiden neuen CTests mit `-V` ausgeführt:
  **6/6 PASS**. Je Konfiguration 59.481 synthetische Checks sowie 306 reale
  Checks / 35 vorbereitete Quellen aus sieben Typen × fünf Difficulties,
  ohne Skip. Belege `*-desirability-final.json` und zugehörige Logs. Die
  unveränderten alten Model-/City-v13-Tests benötigen deshalb keinen neuen
  Produktionsstand. Direkte Vorproben waren ebenfalls grün, ersetzen diese
  CTest-Abnahme aber nicht.
- Unabhängiges Distanzorakel statt Ringnachbau; 180 deterministische
  Zufallsszenen, Spiegelung/Translation, Seiten 1–6, Rand-Clipping, Null-/negative
  Quellen, reihenfolgeabhängige Sättigung, Hausmaximum/Bonus, alle Eingabe- und
  int32-Grenzen, atomare späte Batch-Fehler. Mauern: defaultOFF, positive Quellen,
  acht Richtungen, gleich weit liegende Zellen, gerades Footprint-Zentrum und
  unabhängiges `atan2`-Orakel für die ausdrücklich authored Hypothese.
- **Datenfehler:** Direkter finaler Real-Test mit fehlendem Root liefert
  **77 / SKIP**; vorhandene beschädigte synthetische Modelldatei liefert
  **1 / FAIL**, wie erwartet. Bericht `real-source-negative.json`. Die echten
  Datentests lasen dagegen das unveränderte Original mit dokumentiertem SHA;
  Dateibytes und Änderungszeit bleiben gleich.
- **Vergleichs-CLI:** Zwei frische Läufe erzeugten JSON, zwei CSVs, SVG und HTML
  vollständig bytegleich; 675 Zellzeilen und fünf Hausabfragen unabhängig in
  Python nachgerechnet. Fünf negative Ausgaben-/Quellenfälle verweigern bestehende
  Ziele, Pfade außerhalb `.local`, Originalverzeichnis, Symlink und fehlendes
  Modell ohne Schreibzugriff auf diese Ziele. Protokoll:
  `.local/original-economy/oe03/comparison-check-_je9gm6h/validation.json`.
  Der stabile Reviewstand unter `oe03/comparison/` ist ebenfalls bytegleich.
  SVG gerastert und das PNG tatsächlich angesehen: drei lesbare Panels mit
  identischer Skala, Zellzahlen und Footprint-Markierungen. Keine Grafik- oder
  Originalzahlenkopie im Git.
- Gleiche geordnete Pottery-/Market-/Clay-Quellen: Quadratring gegenüber
  Same-Spec-Manhattan **207/675 unterschiedliche Zellen**, maximal 25 Punkte;
  gegenüber echten authored City-v13-Helfern **359/675**, maximal 29 Punkte.
  Der getrennte Geometriekontrollwert wird nicht als tatsächliche alte Regel
  ausgegeben. Hausaggregation und reine, teilweise überlappende Footprint-
  Abfragen sind explizit beschrieben; kein bezahlter Spielablauf daraus
  behauptet. Lokaler Vergleich:
  `.local/original-economy/oe03/comparison/index.html`.
- **Quellenschutz:** Alle 435 bereits getrackten Dateien unter
  `src/tests/tools/resources` bytegleich zum Start-HEAD. CMake erhält nur neue
  getrennte Library-/CLI-/CTest-Targets; keine produktive App-/World-Verbindung.
  Bericht `legacy-source-identity.json`, fünf finale Task-Code-/Build-Hashes
  unter `validated-task-source.json`. Keine Snapshots oder Assertions der
  alten Tests geändert. API, Arithmetic, CLI und unabhängige Tests separat
  reviewed, keine blockierenden Befunde.
- **Vollständiger Legacy-Vergleich:**
  `python3 tools/original_economy_baseline.py verify --data .local/gog-extracted/app --build .local/reswall/release-build --reference .local/original-economy/oe00/reference/contract.json --output .local/original-economy/oe00/oe03-verification`:
  **PASS**. Alle 72 vollständigen Savebyte-Verträge und 72 Snapshot-Projektionen,
  1485 Policyfälle auf 167 vollständigen Managern (165 vorbereitet / zwei alte
  Konflikte), elf Rampen- und zehn Brücken-Neustartpaare sowie die vorhandenen
  50 historischen Savebyte-Vergleiche bleiben exakt. Unveränderte bezahlte
  Rule4-Xia-/Handan-, Transport-, Pixel- und scripted-UI-Fixtures liefen über
  die bisherigen Produktionspfade. Bericht/Logs unter
  `.local/original-economy/oe00/oe03-verification/`. Vertragsdigest unverändert
  `12823612834d27099189c18e31ed4fed4761c9ff34486227b3075cf1069a49e5`;
  während des Runs identischer Sourcehash
  `9eaf74957381aa5bab88007c829ab84834e831832874a6d9f2cfb8074d039a02` und
  unverändertes Originalinventar.
- Abschluss: Dokumentationslinks und Diffprüfung sauber, exakt validierter
  Task-Code eingefroren. Eigener OE-03-Commit und delegierter PR/Merge folgen
  nach diesem PASS. Gate A bleibt bis OE-07 offen; keine Originalspiel- oder
  Human-Parität behauptet. Nächster geplanter Task ist OE-04.


### OE-04 – 2026-10-16, Basis 5627db5

- Start: sauberer Checkout und Live-`main`
  `5627db513227827e1717884222875ca2505f821b` bestätigt, vorhandenen Feature-
  Branch per Fast-forward angeglichen. Originalinventar weiterhin 1464 Dateien /
  819711091 Byte, identischer Manifesthash wie OE-00/OE-03. Beleg:
  `.local/original-economy/oe04/source-state.json`.
- **Statische Evolutionsevidenz:**
  `python3 .local/original-economy/oe04/check-evolution-evidence.py`:
  **PASS**, 25 begrenzte Fenster / 1105 Instruktionen / 3254 Bytes, fünf
  VTable-/16 Dispatch-Pointer und 14 Counter-/Targetprüfungen, null Abweichungen.
  [Vertrag](../reverse/original-house-evolution-contract.md) bindet alle16
  Typen/15 Zeilen, virtuelle Aufrufe, Gates, Counter und terminale Grenzen an die
  unveränderte EXE. Kein Originalspiel ausgeführt.
- **Needs-/Kapazitätsevidenz:**
  `python3 .local/original-economy/oe04/check-house-needs-evidence.py`:
  **PASS**, acht begrenzte Fenster / 402 Instruktionen / 1014 Bytes sowie sieben
  Bindungsprüfungen, null Abweichungen, EXE und Model unverändert. Separater
  [Vertrag](../reverse/original-house-needs-contract.md) unterscheidet bytebasierte
  Zugänge, signed Waren, unabhängige ODER-Alternativen und kontextuelle
  Elite-Kapazitätszeilen; keine unbewiesene Flächenskalierung.
- **Pure Library:** `HouseEvolutionRules` bereitet alle15×24 Named-Werte samt
  Difficulty und Hash einmalig vor. Getrennte Needs-/Desirability-/Verdict-/
  Ziel-/Counter-/Kapazitätsqueries und wertbasierter `evolve_step`; keine
  World-, App-, SDL-, Save- oder Clock-Verbindung. Typen2–17 bleiben getrennt
  von ModelRows und alten World-Levels0–2. Dokumentierter API-/Reproduktions-
  vertrag: [house-evolution.md](../original-economy/house-evolution.md).
- **Bewusste Forkkorrekturen:** Occupied Vacant benötigt die belegten Gates
  und aktuelle/nächste Bedürfnisse vor Shelter, ohne Counterreset. Empty
  Shelter bleibt unverändert; leere höhere Häuser werden dagegen weiterhin
  ausgewertet. Originalspeicherbreiten sind explizit (int8 Score, int16
  Bewohner/Waren, uint8 Zugang/Qualität/Counter). EliteAbandoned ignoriert nur
  den Score, nicht Bedürfnisse. Originalglobals sind explizite technische
  Snapshotinputs mit unbekannten Spielauslösern; kein erfundener Runtimehook.
  Transition-Komposition folgt tatsächlichen Dispatch-Handlern, nicht dem
  separaten Originaldiagnosepfad. Kein universelles `capacity(type)`.
- **Build/CTest:** Debug, Release und RelWithDebInfo mit ASan/UBSan neu
  konfiguriert; vor dem ersten CTest jeweils `ctest --test-dir BUILD -N`:
  164 registrierte Tests. Neue House-Library/Tests und die bestehenden
  Model-/Desirability-/City-v13-Targets gebaut. Je Konfiguration **13/13
  ausgewählte CTests PASS**, insgesamt39 Ausführungen: vier Model-, zwei
  Desirability-, zwei HouseEvolution- und alle fünf City-v13-Suites
  `base/persistence/districts/determinism/endurance`. Laufzeiten: Debug275.34s,
  Release11.95s, Sanitizer60.03s. Exakte Configure-/Inventory-/Build-/CTest-
  Befehle, Resultate und Logs unter
  `.local/original-economy/oe04/{debug,release,sanitizer}.json` und `*-*.log`.
  Keine Compilerwarnung oder Sanitizerdiagnose; kein vorgezogenes Gate A.
- Nach gezielten Reviewergänzungen die beiden neuen CTests zweimal erneut
  in allen drei Konfigurationen gebaut/ausgeführt: **12/12 zusätzliche PASS**.
  Erste Erweiterung unter `*-house-final.json`, abschließender Stand unter
  `*-house-accepted.json` plus zugehörige `-build.log`/`-tests.log`. Abschließend
  je Konfiguration **5692 synthetische Checks und 6811 reale Checks**,
  einschließlich560 Typ-/Difficulty-Grenzabfragen. Echte Daten ohne Skip,
  identischer Originalhash, vollständige Bytes/Größe/Änderungszeit unverändert.
  Direkte C++20-Vorproben mit allen Projektwarnungen bestanden ebenfalls;
  sie ersetzen die finale CTest-Abnahme nicht.
- **Testinhalt:** Unabhängige wörtliche16-Typ-/15-Zeilen-/Zielmatrix; jede Stufe
  mit Score-Evolve/Stay/Devolve und No-target-/No-evaluation-Grenzen; Zähler
  0/Limit−1/Limit/255, Reset und kein Wrap. Alle Präsenz-/Waren-/Qualitäts-
  schwellen isoliert über15 Reihen, signed-negative Anforderungen/Stocks,
  Zugangsbyte255, Bronze/Lack nicht addiert, Daoist/Buddhist einzeln ausreichend.
  Aktuelle Fehlbedürfnisse schlagen maximalen Score/Forceflag; nächste verhindern
  Evolve. Alle Typen mit aktuellen/nächsten Engpässen, Vacant-/Shelter- und
  Elite-Sonderfälle, leeres Abandoned→Modest, leere höhere Häuser weiter aktiv.
  Beide Gates getrennt, vorbereitete Labels/Enums/alle Felder/Overflow geprüft;
  pure Inputs bleiben auch bei Fehler unverändert. Reale360 Felder × fünf
  Difficulties unabhängig aus Raw+Modifier berechnet; keine Originalzahlentabelle
  im Git und keine Simulation bezahlter Ankünfte behauptet.
- **Negative Datenpfade:** Finaler Real-Test mit absichtlich fehlendem Root
  liefert77/SKIP; vorhandenes beschädigtes synthetisches Modell liefert1/FAIL,
  wie erwartet. Tatsächliche Befehle/Ausgaben `real-source-negative.json`.
- **Quellenschutz/Review:** Alle439 schon getrackten Dateien unter
  `src/tests/tools/resources` bytegleich zur Startrevision. CMake erhält nur
  getrennte neue Library-/CTest-Targets, keine alte App-/World-Verbindung.
  `legacy-source-identity.json` und vier finale Code-/Build-Hashes in
  `validated-task-source.json`. Keine alten Tests oder Snapshots verändert.
  Getrennte EXE-/Needs-/Kern-/Testreviews ohne verbleibende blockierende Befunde.
  Lokale Dokumentationslinks und Diffprüfung sauber.
- **Vollständiger Legacy-Vergleich:**
  `python3 tools/original_economy_baseline.py verify --data .local/gog-extracted/app --build .local/reswall/release-build --reference .local/original-economy/oe00/reference/contract.json --output .local/original-economy/oe00/oe04-verification`:
  **PASS**. Alle72 vollständigen Savebyte-Verträge und72 Snapshot-Projektionen,
  1485 Policyfälle auf167 vollständigen Managern (165 vorbereitet/zwei alte
  Konflikte), elf Rampen- und zehn Brücken-Neustartpaare und die50 vorhandenen
  historischen Savebyte-Vergleiche bleiben exakt. Bezahlte Rule4-Xia-/Handan-,
  Transport-, Pixel- und scripted-UI-Fixtures liefen über unveränderte alte
  Produktionspfade. Berichte/Logs unter
  `.local/original-economy/oe00/oe04-verification/`. Vertragsdigest unverändert
  `12823612834d27099189c18e31ed4fed4761c9ff34486227b3075cf1069a49e5`;
  Sourcehash während des Runs
  `d79eb4cbaae032c25ec363d7e7c697e5225b1cb8323ee550b0086286333f2c73`
  und gesamtes Originalinventar bleiben gleich.
- Abschluss: finale Code-/Build-Hashes und alle lokalen Dokumentationslinks
  erneut geprüft. Eigener OE-04-Commit und delegierter PR/Merge folgen nach
  diesem PASS. Gate A bleibt bis OE-07 offen; Original-Flagproducer, effektive
  Bevölkerungsaufnahme, Geometry-Transaktionen und Originalspiel-/Human-Parität
  bleiben ausdrücklich unbestätigt. Nächster geplanter Task: OE-05.


### OE-05 – 2026-10-16, Basis 7f57424

- Sauberen Checkout und Live-`main`
  `7f5742400b4fbac9c31fb653d7562601df6d9111` bestätigt; bestehenden Feature-
  Branch per Fast-forward angeglichen. Originalinventar unverändert1464Dateien /
  819711091Byte und derselbe OE-00-Manifesthash. Beleg
  `.local/original-economy/oe05/source-state.json`.
- **Visit-/Zeitbelege:**
  `python3 .local/original-economy/oe05/visit-check-evidence.py`: **PASS**,
  23 begrenzte Fenster /656Instruktionen /1957Bytes,27 semantische
  Instruktionsprüfungen/fünf Pointerbindungen, null Byteabweichungen.
  [Timervertrag](../reverse/original-service-timer-contract.md) bindet96/40,
  neun Bytefelder, wiederholte Grants, Verfall, pfadspezifische Admission,
  Elite-only-Sourcebedingungen und Phasen35→39. EXE/Model unverändert,
  Originalspiel nicht ausgeführt. Keine Worldtick-/Realzeit-/Monatsabbildung.
- **Spawnbelege:**
  `python3 .local/original-economy/oe05/spawn-check-evidence.py`: **PASS**,
  17 begrenzte Fenster /524Instruktionen /1538Bytes, fünf Thresholdfunktionen,
  acht Klassen-/VTable-/Konstruktorbindungen und sechs Religions-Factoryzweige;
  null Byteabweichungen, EXE unverändert. Der
  [Spawnvertrag](../reverse/original-service-spawn-contract.md) belegt elf
  Provider mit fünf Staffing-Tabellen, Gates, uint8-Wrap, strikt größer und
  Reset vor Actorfactory. Fälliger Versuch ist kein erfolgreicher Actor.
- **API und Forkabweichungen:** Eigene SDL-/World-freie `ServiceRules`-Library
  mit neun Rollen, drei Besuchspfaden, 25 gültigen aufgelösten Effekten,
  getrenntem Raw-/House-Countdown und Spawnqueries. DirectService erbt nicht
  die leere-Elite-Ausnahme der Religion-/Sharedpfade; Status und Byte09 bleiben
  explizit. Sharedactor kann mehrere Timereffekte bewirken; die API projektiert
  nur bereits aufgelöste Effekte, keinen vollständigen Actorcallback. Der Fork
  pauschalisiert Eligibility und zwei Spawn-Tabellen und lässt Byte-Wrap/Caller-
  Freeze aus. Keine Gratisversorgung aus Nähe oder bloßer Spawnfälligkeit.
  Reproduktions-/APIgrenzen: [service-rules.md](../original-economy/service-rules.md).
- **Modellnamen:** Religionsproducer bindet +0e eindeutig als Daoist und +0f
  als Buddhist. Im alten Needs-Dokument diese Namen präzisiert und den neuen
  Beleg verlinkt. Die gemeinsame ODER-Regel und alle alten HouseEvolution-
  Ergebnisse bleiben identisch; keine alte Source-/Saveänderung.
- **Build/CTest:** Debug, Release und RelWithDebInfo mit ASan/UBSan neu
  konfiguriert, jeweils `ctest --test-dir BUILD -N` vor dem ersten CTest:
  166 registrierte Tests. Neue Library/Tests und bestehende Model-,
  Desirability-, HouseEvolution- und City-v8-Service-Targets gebaut. Je
  Konfiguration **11/11 ausgewählte CTests PASS**, insgesamt33 Ausführungen:
  vier Model-, zwei Desirability-, zwei HouseEvolution-, zwei ServiceRules-
  Tests und `sandbox-city-v8-service`. Laufzeiten Debug7.64s/Release0.92s/
  Sanitizer2.89s. Exakte Configure-/Inventory-/Build-/CTestbefehle und Logs:
  `.local/original-economy/oe05/{debug,release,sanitizer}.json`, gleichnamige
  `*-*.log`. Alle alten Tests/Snapshots unverändert. Die bestehende City-v8-
  Suite prüft tatsächliche Ankunft/1200t-Expiry, fehlende Route, Reparatur,
  Save/Restore und deterministischen Langlauf. Keine Presentation-Suite allein
  aufgrund des Wortes Service erweitert; kein vorgezogenes Gate A.
- Die begrenzte Originaldatei-Leseprüfung des neuen Real-Tests ergänzt,
  anschließend nur die beiden neuen CTests in allen drei Konfigurationen
  erneut gebaut und mit `-V` ausgeführt: **6/6 zusätzliche PASS**.
  Finale Werte je Konfiguration **177171 synthetische und50047 reale Checks**,
  675 Rollen-/Row-/Difficulty-Visitabläufe und230 reale positive Anforderungen,
  kein Skip. Originalhash, vollständige Bytes, Größe und Änderungszeit gleich.
  Belege `*-services-final.json` samt Logs. Direkte Debug-Vorprobe bestand
  ebenfalls mit allen Warnungen; finale CTest-Abnahme bleibt maßgeblich.
- **Testinhalt:** 25 erlaubte Visitkombinationen ×16Typen ×fünf signed
  Occupants ×sechs Status ×Accessflags; jedes Feld/alle anderen Felder erhalten,
  abwesende/abgelehnte Visits ohne Erneuerung, mehrere Grants ohne Stacking.
  Alle neun Bytes0…255, Grenzen0/1/39/40/95/96/255, vollständiger96-/40-Ablauf,
  Renewal und explizite Mehrfacheffekte. Phase35-Verfall vor Evolution39 und
  zwischenzeitlicher Grant getrennt. Elf literal Providerprofile ×17Staffing-
  Werte ×256 Counter ×zwei Gates =95744 Spawn-Schrittprüfungen, signedExtreme,
  Equality/StrictGreater, Threshold+1 und255→0.1000 erlaubte volle Herbalist-
  Aufrufe erzeugen500 fällige Versuche und unveränderte unversorgte Housebytes.
  Alle ungültigen Enums/Pfadkombinationen werfen statt stillen Defaults.
- **Negative Datenpfade:** Fehlender Originalroot liefert77/SKIP; vorhandenes
  beschädigtes oder übergroßes synthetisches Modell liefert1/FAIL wie erwartet.
  Reale Originaldaten liefen in allen Konfigurationen. Befehle/Diagnosen:
  `real-source-negative.json`; bounded Readerprüfung verhindert unbegrenztes
  Vorab-Einlesen auch im separaten Testprogramm.
- **Quellenschutz/Review:** Alle442 bereits getrackten Dateien unter
  `src/tests/tools/resources` bytegleich zur Startrevision. CMake ergänzt nur
  getrennte Library-/CTest-Targets, kein produktiver Link oder Frame-/Tickhook.
  `legacy-source-identity.json`, vier finale Code-/Build-Hashes in
  `validated-task-source.json`. Unabhängige Visit-, Spawn-, Core- und
  Testreviews ohne verbleibende Befunde. Originalbytes/Disassembly bleiben
  privat; Git enthält keine Model-Zahlenkopie oder Originalgrafik.
- **Vollständiger Legacy-Vergleich:**
  `python3 tools/original_economy_baseline.py verify --data .local/gog-extracted/app --build .local/reswall/release-build --reference .local/original-economy/oe00/reference/contract.json --output .local/original-economy/oe00/oe05-verification`:
  **PASS**.72 vollständige Savebyte-Verträge/72 Snapshot-Projektionen,
  1485 Policyfälle auf167 vollständigen Managern (165 vorbereitet/zwei alte
  Konflikte), elf Rampen-/zehn Brücken-Neustartpaare und50 vorhandene historische
  Savebyte-Vergleiche bleiben exakt. Unveränderte bezahlte Rule4-Xia-/Handan-,
  Transport-, Pixel- und scripted-UI-Fixtures bestanden ebenfalls. Berichte
  `.local/original-economy/oe00/oe05-verification/`; Vertragsdigest weiterhin
  `12823612834d27099189c18e31ed4fed4761c9ff34486227b3075cf1069a49e5`.
  Während des gesamten Runs identischer Sourcehash
  `026f932fc91867cb77b8e9b8736318d1e5de3b56815a3b4912bac4487f391f09`
  und unverändertes Originalinventar.
- Abschluss: finale Code-/Build-Hashes, Dokumentationslinks und Diffprüfung
  sauber. Eigener OE-05-Commit und delegierter PR/Merge folgen nach diesem PASS.
  Gate A bleibt bis OE-07 offen. Originale Flagproducer, vollständige Actor-
  Effekte, echtes bezahltes Versorgungs-Gameplay sowie Zeitmapping/Human-Parität
  bleiben ausdrücklich offen. Nächster geplanter Task: OE-06.


### OE-06 – 2026-10-16, Basis ed24702

- Sauberen Checkout und Live-`main`
  `ed2470256912bf8c70e700b1d30553f6d226c15e` bestätigt; bestehenden Feature-
  Branch per Fast-forward angeglichen. Originalinventar unverändert1464Dateien /
  819711091Byte mit identischem OE-00-Manifesthash; Beleg
  `.local/original-economy/oe06/source-state.json`.
- **Lieferbelege:**
  `python3 .local/original-economy/oe06/delivery-check-evidence.py`: **PASS**,
  elf begrenzte Fenster/428Instruktionen/1240Bytes,15 semantische Assertions,
  sechs Pointer- und sechs Warenbindungen,64821 arithmetische Kontrollfälle,
  null Byteabweichungen. [Liefervertrag](../reverse/original-goods-delivery-contract.md)
  belegt nächste kanonische Zeile/Zeile7-Ausnahme, Größe>1 ×4, Elite ×2,
  exact-zero Hemp/Tea10 und getrennte Admission. Quellabhängige Fallbackformel
  statt Fork-Bedarfsvergleich/finalem Bedarfscap. Originalspiel nicht ausgeführt.
- **Konsum-/Qualitätsbelege:**
  `python3 .local/original-economy/oe06/consumption-food-check-evidence.py`:
  **PASS**, elf Fenster/465Instruktionen/1361Bytes,40 semantische Assertions,
  fünf Pointerbindungen/vier binary32-Schwellen/drei double-rounding-
  Gegenbeispiele, null Byteabweichungen. [Vertrag](../reverse/original-goods-consumption-food-contract.md)
  bestätigt beide unabhängigen Feld13-Verbräuche ohne Größenfaktor, bei exakt
  null Bewohnern kein Konsum. Food-Blending verwendet Vorbestand, nicht Bedarf;
  strikte Floatgrenzen und Integer-Mittel sind getrennt. Quellen unverändert.
- **Pure Library:** `GoodsRules.h/.cpp`, sechs getypte Waren, getrennte Quality,
  Bedarf/Ziel/Admission, vollständige wertbasierte Transfer-/Konsumresultate mit
  expliziter Mengenbilanz. Nichtnegative Inventare auch bei No-op, bounded
  Quellcap/word-Headroom und deterministische int64-Rundungsmittelpunkte. Die
  Byte-Score-Grenze, nearest-even-Modus und Lieferung0 erhält Quality sind
  ausdrücklich authored; keine originale Markt-/Actor-/Food-Score-Produktion
  oder World-/Save-/App-Verbindung.
- **Build/CTest:**
  `python3 .local/original-economy/oe06/run_validation.py`: **PASS**. Debug,
  Release und ASan+UBSan/RelWithDebInfo jeweils explizit konfiguriert, vor CTest
  über `ctest --test-dir BUILD -N` alle168 registrierten Tests und beide neuen
  Tests ermittelt, relevante Targets frisch und ohne Compilerwarnung gebaut.
  Je16/16 CTests, zusammen **48 PASS, null Skip/Fail**: vier Modell-, je zwei
  Desirability-/House-/Service-/Goods-Tests sowie `sandbox-production-chain`,
  `sandbox-household-supply`, `sandbox-city-v7-food` und `sandbox-city-v8-service`.
  Exakte Targetlisten, CTest-Regex und Logs in `{debug,release,sanitizer}.json`.
- **Unabhängige Testvektoren:** `OriginalGoodsRulesTests.cpp` synthetisch
  **1937389 Assertions**, real **9382 Assertions/4320 Warenabfragen** mit
  gepinntem Modelhash. Alle16 Typen/fünf Difficulties, Raw+Modifier unabhängig,
  leere/große/Elite-Häuser, unterschiedliche Next-/Current-Zeilen, null/negative
  Bedarfe, leere/volle/maximale Bestände, Quell-/Zielsumme bei Teillieferung,
  konsumierte Mengen und unveränderte Fremdfelder/Inputs. Alle256² Qualitypaare
  über20 klassifizierte Quotienten, acht exakte/skalierte Mittelpunkttripel
  und drei hohe int32-double-rounding-Gegenbeispiele. Keine Originaltabelle in Git.
- **Negative Dateipfade:** Finales Debug-Real-Binary gegen acht isolierte Pfade:
  fehlend→77/SKIP; beschädigt, >256KiB, Sourceverzeichnis, Rootdatei und drei
  dangling-Symlink-Fälle jeweils1/FAIL wie erwartet. Vollständige Befehle und
  Ausgaben `real-source-negative.json`; vorhandene Fehler werden nie geskippt.
- **Quellenschutz/Review:** Alle445 zuvor getrackten Dateien unter
  `src/tests/tools/resources` bytegleich zur Startrevision; additive CMake-
  Library-/Testregistrierung. `legacy-source-identity.json` und finale Code-/
  Build-Hashes in `validated-task-source.json`. Getrennte Liefer-, Konsum-/Food-,
  Test- und Gesamtvertragsreviews ohne verbleibende Befunde. Lokale Dokulinks
  und Diffprüfung sauber. Keine alten Tests oder Snapshots verändert.
- **Vollständiger Legacy-Vergleich:**
  `python3 tools/original_economy_baseline.py verify --data .local/gog-extracted/app --build .local/reswall/release-build --reference .local/original-economy/oe00/reference/contract.json --output .local/original-economy/oe00/oe06-verification`:
  **PASS**. Alle72 vollständigen Savebyte-Verträge und72 tatsächlichen Snapshot-
  Projektionen,1485 Policyfälle auf167 vollständigen Managern (165 vorbereitet/
  zwei alte Konflikte), elf Rampen- und zehn Brücken-Neustartpaare bleiben exakt.
  Bezahlte Rule4-/Transport-, Pixel- und scripted-UI-Fixtures laufen über alte,
  unveränderte Produktionspfade. Vertragsdigest weiterhin
  `12823612834d27099189c18e31ed4fed4761c9ff34486227b3075cf1069a49e5`;
  Sourcehash während des gesamten Runs
  `6cb557ac8f1a3691b10c15e965378b5f3ada53936307a0023591f67160c50efe`
  und gesamtes Originalinventar identisch. Bericht/Logs im genannten Output.
- Abschluss: finale Code-/Build-Hashes, lokale Dokumentationslinks und Diff
  geprüft. Eigener OE-06-Commit und delegierter PR/Merge folgen nach diesem PASS.
  Gate A bleibt bis OE-07 offen. Echte neue Producer-/Courier-/Food-Score-
  Produktion, Zeitmapping, ursprüngliche Runtime-/Human-Parität bleiben offen.
  Nächster geplanter Task: OE-07, unabhängiger Forkvergleich und Gate A.


### OE-07 – 2026-10-16, Basis 6f6be38

- Sauberen Checkout und Live-`main`
  `6f6be387128a7f72a64ac6e5439faf2b81df5666` bestätigt; bestehenden Feature-
  Branch per Fast-forward angeglichen. Originalinventar unverändert1464Dateien /
  819711091Byte, identischer OE-00-Manifesthash. Beleg
  `.local/original-economy/oe07/source-state.json`.
- **Ausführbarer Vergleich:** `tools/original_rules_parity.py` kompiliert eigene
  numerische Adapter getrennt gegen die tatsächlichen gepinnten Forkmodule und
  die bestehenden Kerne. Elf Forkdateien am Commit
  `d861d2912337c09751cb68196f85d5a30752726e` gegen eingebettete SHA-256 geprüft;
  keine Windows-Integration oder Forkimplementierung in Git. [JSON-Bericht](../original-economy/core-parity.json)
  und [Entscheidungsmatrix](../original-economy/core-parity.md) enthalten Inputs,
  erwartete und beobachtete Zahlen, Quellenhashes und Belegverweise.
- **Finale echte Vergleichsläufe:**
  `python3 .local/original-economy/oe07/run_final_comparisons.py`: **PASS**.
  Für Debug/Release/ASan+UBSan jeweils tatsächlich
  `python3 tools/original_rules_parity.py --fork .local/original-economy/oe00/fork --data .local/gog-extracted/app --configuration NAME --output .local/original-economy/oe07/final-NAME`:
  **74/74 PASS**,46 `parity`,18 `corrected_bug`, sechs `unverified`, vier
  `unsupported`. Parserwerte/Negativsyntax, Ringe/Sättigungsreihenfolge, Needs/
  Evolution, neun Timerrollen/Provider, Konsum, alle sechs Waren und
  Teillieferungen, FoodQuality und die ausdrücklich authored Schrittkomposition.
  Eigenständige Goldens unterscheiden insbesondere Warenfelder, Next-/Current-
  Zeile, Elitefaktor, Quellfallback, leere Häuser und binary32-Grenzen.
- **Viertes Forkmodul:** Der separate `HousingViewProbe.cpp` ruft tatsächlich
  `OriginalHousingView` auf.18 Beobachtungen prüfen sieben funktionale Kandidaten,
  Populationsfeld0 in legal erzeugten alten Profilen, Shelter-/`max(population,1)`-
  Diagnoseannahme und die Wirkung einer wählbaren FoodQuality nach echter CityV7-
  Farm-/Road-/Courierlieferung. Vollständige WorldSnapshots bleiben gleich.
  Alte Profile modellierten noch keine Population; daraus wird keine physisch
  leere Original-House abgeleitet. Stetiger Herbalist-Regrant ist Sourceaudit,
  kein zusätzlicher beobachteter Besuch. Ein vollständiger Originalreport als
  neue Gameplay-Authority bleibt bewusst unsupported.
- **Originalmodell:** Je Konfiguration1800 vorbereitete Housewerte (15×24×5)
  auf beiden Seiten in gleicher Reihenfolge als FNV-Digest verglichen, mit
  demselben Model-SHA. Keine Originalzahlentabelle ausgegeben/committet.
  Building-/Parserkontrollen sind getrennte synthetische Fälle; Raw-/Schema-
  Suites laufen zusätzlich. Öffentliche Projektionen aller drei finalen Reports
  identisch. Compilerflags `-Wall -Wextra -Wpedantic -Wconversion -Wshadow`,
  keine Warnungen; Quellen, Libraryhash und Realmodell vor/nach dem Run geprüft.
- **Runner-Vertrag/Negativfälle:**24 hermetische Tests in
  `tests/test_original_rules_parity.py`, darunter unabhängige Goldens, strikte
  numerische Resultate, verfälschte Pins/Manifestwerte, defekte Pfade, vorhandene
  fehlerhafte Modelldaten, Outputschutz und manipulierte/veraltete Reports.
  Die gemockten Orchestrierungskontrollen sind ausdrücklich getrennt von den
  oben tatsächlich kompilierten/ausgeführten Kernvergleichen. Der verpflichtende
  Berichtstest braucht keine privaten Quellen und prüft den getrackten Report
  gegen kanonische Fälle/Quellenhashes. Root-Review fand und behob zusätzlich
  einen CLI-Dangling-Fork-SKIP vor `resolve()`; finaler Regressionstest PASS.
- **Direkte CLI-Kontrollen:** Sechs tatsächliche Aufrufe in
  `cli-negative-final.json`: fehlender Fork→77, vorhandene Rootdatei/defekter
  Symlink/falscher Sourcepin/altes Outputverzeichnis→1, ausdrücklich fehlendes
  Originalmodell→77 mit weiterhin74 bestandenen synthetischen Fällen und
  erfolgreichem HousingView-Probe. Vorhandener alter Output bleibt bytegleich;
  echte beschädigte Quellen werden nicht als fehlend geskippt. Ohne `--data`
  steht der Real-Teil NOT_RUN, geprüft im Runner-Vertragstest.
- **Vollständige Phasen-Gate-Suites:**
  `python3 .local/original-economy/oe07/run_full_validation.py`: **PASS**.
  Je Debug, Release und ASan+UBSan/RelWithDebInfo explizit konfiguriert, vor dem
  ersten CTest mit `ctest --test-dir BUILD -N` alle168 damals registrierten
  Tests ermittelt und sämtliche Targets frisch gebaut. Danach vollständiges
  `ctest --test-dir BUILD --output-on-failure -j 2`: je168/168 PASS, keine
  Skips/Fehler und keine Compilerwarnungen. Getrennte TMPDIRs verhindern
  Kollisionen der alten Dateifixtures zwischen Konfigurationen. Laufzeiten:
  Debug1182.83s, Release83.80s, ASan/UBSan322.32s; einschließlich aller neuen
  Kerntests und sämtlicher alter City-v13/v16-Suites/Endurance/Determinismus.
  Vollständige Befehle und Logs in `{debug,release,sanitizer}-full.json`.
- **Neue CTest-Registrierung:** Nach Abschluss aller Builds nur den unabhängigen
  Python-Vertragstest addiert; alte Targets unverändert. Anschließend
  `python3 .local/original-economy/oe07/run_new_ctest.py release sanitizer` und
  separat `... debug`: Konfiguration/`ctest -N` mit169 registrierten Tests,
  danach `ctest --test-dir BUILD --output-on-failure -V -R '^original-rules-parity-contract$'`:
  je1/1 PASS mit allen24 Untertests. Zusammen **507 CTests PASS**, jede finale
  Registrierung in jeder Konfiguration tatsächlich ausgeführt. Keine privaten
  Quellen für den neuen getrackten Berichtstest erforderlich. Genaue Logs in
  `{debug,release,sanitizer}-contract.json`; alte komplette LastTest-Logs separat
  aufbewahrt. Die unveränderten168 alten Tests wurden nicht unnötig wiederholt.
- **Vollständiger Legacy-Vergleich:**
  `python3 tools/original_economy_baseline.py verify --data .local/gog-extracted/app --build .local/reswall/release-build --reference .local/original-economy/oe00/reference/contract.json --output .local/original-economy/oe00/oe07-verification`:
  **PASS**. Alle72 vollständigen Savebyte-Verträge und72 tatsächlichen Snapshot-
  Projektionen,1485 Policyfälle auf167 vollständigen Managern (165 vorbereitet/
  zwei alte Konflikte), elf Rampen- und zehn Brücken-Neustartpaare bleiben exakt.
  Vertragsdigest weiterhin
  `12823612834d27099189c18e31ed4fed4761c9ff34486227b3075cf1069a49e5`;
  Sourcehash während des gesamten Runs
  `a529b777738f2d7607a5fb54fce062c93515206af660d986283ef4958b724622`.
  Originalinventar bleibt unverändert; tatsächliche Berichte/Logs im genannten
  Output. Scripted/Software-/Save-Belege sind keine native Human-Abnahme.
- **Quellenschutz/Review:** Alle448 vorher getrackten Dateien unter
  `src/tests/tools/resources` bytegleich zur Startrevision. CMake erhält nur
  die neue Python-Testregistrierung. Finale Code-/Build-Hashes in
  `validated-task-source.json`, alte Quellen in `legacy-source-identity.json`.
  Unabhängige Vector-, Compiler-/Runner-, HousingView- und Gesamtvertragsreviews
  ohne verbleibende Befunde; Quellenhashes/öffentliche Daten/Links/Diff geprüft.
- **Gate A: PASS** für Parser-/Quellenbindung, unabhängigen tatsächlichen
  Vier-Modul-Kernvergleich, dokumentierte Korrekturen/Annahmen und exakte alte
  World-/Save-Verträge. Die isolierten Kernfähigkeiten sind verfügbar; bewusst
  ausgeschlossene Diagnose-/Kalenderwrapper und unbestätigte Originalmechanik
  werden nicht als implementiertes Gameplay gezählt. Originale Gesamtparität,
  bezahlte neue Versorgung und Human-Akzeptanz bleiben offen. Eigener OE-07-
  Commit und delegierter PR/Merge folgen nach diesem PASS. Nächster Task: OE-08.

### OE-08 – 2026-10-16, Basis2651408

- Vor Beginn sauberer `main` und live/origin identisch auf
  `2651408e71db6a13ad6bec45214dd2b76f502ea2`; bestehenden Feature-Branch per
  Fast-forward angeglichen. Rule7/Schema23 vor Umsetzung als frei bestätigt.
  Read-only Originalinventar und erlaubte Source-Differenzen unter
  `.local/original-economy/oe08/{source-state,validated-task-source}.json`.
- Expliziter City-v16/7-Gate, begrenzte gemeinsame Bridge-/Ramp-Gates und
  policy3-Entwicklungsfactory. Regulärer Start, View, Economy-Commands, Ticks
  und Saves verweigern7. Keine Model-/Scheduler-/Schema23-Implementierung oder
  neue Menüoption. [Vertrag](../original-economy/rule-generation.md).
- Neue SDL-freie Profil-/Infrastruktur-/Save-Suite:16 Profile×Versionen0–8 und
  UINT32_MAX, vollständige Policy-/Startverweigerung,14 unfertige Commands,
  Snapshotgleichheit von bezahlten Bridge/Ramp/Road/Repair gegen6 und1536
  gerichtete Kanten. View-Konstruktor verweigert vor SDL und sämtlichen
  Performance-Countern. Fünf hermetische CLI-Testmethoden prüfen klare Map-
  Diagnose, unvollständige Inputs, inkompatible Modi und unveränderte Dateien.
- Erster zusätzlicher direkter Debug-Aufruf ohne taskbezogenes TMPDIR traf im
  neuen Testfixture macOS' `/var`-Symlink. Ausschließlich dessen TemporaryRoot
  kanonisiert; strenge Produktions-Writerprüfung blieb unverändert. Direkter
  Wiederholungslauf `env -u TMPDIR .../openemperor-original-economy-rule-gate-tests`
  **PASS**; finale Suites verwenden eigene absolute Tempwurzeln je Konfiguration.
- Alle drei Konfigurationen mit `cmake -S . -B <BUILD>` bestätigt und vollständig
  gebaut: Debug, Release und RelWithDebInfo mit ASan/UBSan; System-SDL3 aktiv.
  Vor CTest jeweils `ctest --test-dir <BUILD> -N`: **171 registrierte Tests**.
  Reproduzierbare lokale Ablaufdatei `.local/original-economy/oe08/run_validation.py`;
  konkrete Kommandos/Logs in `<configuration>-final-build-{0,1,2}.log` und
  `<configuration>-full-tests.log`. Volle Suite:
  `ctest --test-dir <BUILD> --output-on-failure --parallel 2`.
- **Release:171/171 PASS**,86.53s; **ASan/UBSan:171/171 PASS**,317.38s;
  **Debug:171/171 PASS**,1205.03s. Insgesamt **513 CTests PASS**,0 Skip;
  keine Compiler-/Sanitizer-Meldung. Vollständige Assertions/Source-/Inventar-/
  Vergleichsbindung in `.local/original-economy/oe08/final-validation.json`.
- Echter Vier-Modul-Vergleich mit
  `python3 tools/original_rules_parity.py --configuration <debug|release|sanitizer>
  --data .local/gog-extracted/app --output .local/original-economy/oe08/parity-<CONFIG>`:
  **je74/74 PASS**,46 parity/18 corrected_bug/6 unverified/4 unsupported;
  jeweils18 echte HousingView-Beobachtungen und1800 Originalmodellwerte als
  begrenzter Digest. Exakter Header-Kompatibilitätspin nach unabhängigem Review
  erneuert; alle drei portablen Resultate identisch. `core-parity.json` ändert
  ausschließlich den Toolquellenhash, keine Erwartungen/Beobachtungen.
- `python3 tests/OriginalEconomyBaselineTests.py -v`: **13/13 PASS**.
  Direkte neue Release-CLI gegen die echte `Cities/Xia.map`: geprüfte Map,
  `runtime_ready:false`, erwarteter Exit1; separate Diagnose unter
  `.local/original-economy/oe08/real-map-cli.json`. Kein SDL, World oder Modell
  wird durch diese map-only Diagnose vorbereitet; kein spielbarer Start behauptet.
- Vollständiger Legacy-Replay **PASS**,72/72 volle SaveDocuments und tatsächliche
  WorldSnapshots byte-/feldgleich; alle1485 Policyfälle auf167 vollständigen
  Managern/165 vorbereiteten Maps/zwei unveränderten Konflikten identisch.
  Je11 Rampen-/10 Brücken-Checkpointpaare in separaten Prozessen mit sofortigem
  Restore und100-Tick-Fortsetzung identisch. Vertrags-SHA256 weiterhin
  `12823612834d27099189c18e31ed4fed4761c9ff34486227b3075cf1069a49e5`.
  Ausgeführter Befehl:
  `python3 tools/original_economy_baseline.py verify --data .local/gog-extracted/app
  --build .local/reswall/release-build --reference
  .local/original-economy/oe00/reference/contract.json --output
  .local/original-economy/oe00/oe08-verification`.
- Unabhängiger Core-/Persistenz-/Appreview ohne blockierenden Befund.
  Keine WorldSnapshot-/MapPermissions-Datenfelder, Policy3 oder Originaldateien
  geändert. Gate B bleibt bis OE-12 offen; spielbare Originalwirtschaft und
  menschliche Abnahme sind weiterhin nicht nachgewiesen.
- Abschließender Source-/Inventarvergleich nach Legacy-Replay **PASS**:
  Task-Source-SHA256
  `5fffd2604db29ef79d99b3eae5fc081fc0eeac345312b9fdd0eee428b9ea609c`;
  441 von452 alten Source-/Test-/Tool-/Resource-Dateien bytegleich, exakt elf
  reviewed Dateien geändert; zusätzlich CMake und zwei neue Tests. Sämtliche
  Originalmodule, MapPermissions, Bridge/Ramp/Renderer und Menü bleiben bytegleich.
  Originalinventar1464 Dateien/819711091 Bytes mit unverändertem Manifest
  `dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`.

### OE-09 – 2026-10-16, Basis4d5f9fb

- Sauberer, live/origin bestätigter `main`
  `4d5f9fb88fe9ca0a106c1aea30cf4080230f390b`; Feature-Branch per Fast-forward.
  Rule7/Schema23 frei und exklusiv gebunden. Vorab-Inventar/Quellstand unter
  `.local/original-economy/oe09/initial-state.json`; [Vertrag](../original-economy/save-schema-23.md).
- Separate OriginalHouse-Authority mit Sourceidentität, Typ/Bedürfnisbeobachtungen,
  neun Timern, sechs Beständen, Counter, Phase und geprüfter1:1-BuildingId.
  Nur tatsächliche bezahlte80-Funds-Vacant-Container ohne Bewohner/Güter/Service
  aktuell restaurierbar. Keine Legacy-Level-Abbildung, Zeit oder Produzenten.
  Normale Rule7-Runtime und Recovery bleiben gesperrt.
- Neue SDL-freie Suite **PASS**: bezahlte nichtleere House-/Road-/Bridge-/Ramp-
  World, vollständiges kanonisches Save/Restore plus **158 Ablehnungsfälle**;
  darunter Unknown/Missing/Duplicate-Keys auf allen Ebenen, Enum-/Zahlendomänen,
  ID-/Finance-/Revision-/Quellhash-Fehler, Old/New-Payload, Recovery und alle drei
  atomischen Schreibfehler. Vorhandene Zieldateien bleiben identisch.
- Tatsächliche neue SDL-F5/F9-Suite **PASS**: Haus/Straßen regulär gekauft, F5,
  weitere bezahlte Straße, F9 mit vollem Snapshot/SaveDocument/Bytevergleich;
  beschädigtes JSON und geänderte Map/Model-Quellen atomar verweigert;120 Updates
  und Zeitsteuerung lassen Tick0 unverändert. Ein frischer OS-Prozess lädt und
  schreibt mit demselben tatsächlichen F5 erneut bytegleich. Exakte normale7-
  und Rule6→23-Viewsperren sowie leerer Entwicklerfallback/Inspector geprüft.
- Erste gezielte CTestauswahl meldete zwei Testfehler:
  neuer SDL-Fixture nutzte macOS-`/var`-Symlink statt kanonischem Pfad; alter
  OE-08-Test erwartete noch „Schema nicht implementiert“. Nur neue Testpfade
  kanonisiert und genaue neue Unbound-Source-/Schema-/Runtime-Refusalgründe im
  Gate-Test nachgeführt. Außerdem neue getrennte OriginalState beim reinen6vs7-
  Infrastrukturvergleich ausdrücklich ausgenommen; alle bisherigen Felder bleiben
  exakt. Finale gezielte drei CTests **3/3 PASS**,1.94s. Keine Produktions-
  Symlinkprüfung oder alte positive Behauptung abgeschwächt.
- Alle drei vollständigen Builds **PASS**, ohne Compilerwarnungen: Debug,
  Release und RelWithDebInfo mit ASan/UBSan, System-SDL3 aktiv. Jeweils vorher
  `ctest --test-dir <BUILD> -N`: **173 registrierte Tests**. Ablaufdateien und
  Befehle unter `.local/original-economy/oe09/run_{build,core_tests}.py`;
  je Konfiguration eigene absolute TMPDIR-Wurzel. Die172 World-/Save-/UI-Tests
  laufen mit `ctest --test-dir <BUILD> --output-on-failure --parallel 4
  -E '^original-rules-parity-contract$'`; das Vergleichswerkzeug wird nach dem
  unabhängigen frischen Vergleich separat geprüft. **Debug172/172 PASS**,
  928.57s; **Release172/172 PASS**,76.36s; **Sanitizer172/172 PASS**,258.89s, mit
  `UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`. Abschließender Paritätsvertrag
  je **1/1 PASS**; insgesamt519 CTests, kein Skip, keine Sanitizer-Meldung.
- Abschlussreview fand zwei schmalere `unsigned`-Casts vor der Prüfung des
  `size_t`-Difficulty-Enums im Persistenzhelper/Serializer. Beide Guards auf volle
  Breite korrigiert; drei direkte Helfer-Negativfälle für5, `size_t::max` und
  `size_t::max/2+1` ergänzt. Vorherige365 Core-Dateihashes und die beiden früheren
  Dateien bleiben separat erhalten; nur diese zwei Pfade änderten sich danach.
  Finale inkrementelle vollständige Builds erneut ohne Warnung **PASS** und
  gezielte elf Save-/Recovery-/Bridge-/Ramp-/Rule-Gate-/CLI-CTests je Konfiguration
  **11/11 PASS**: Debug37.88s, Release5.40s, ASan/UBSan19.51s. Damit33 gezielte
  Wiederholungen auf finalem Code; neuer Save-Test meldet158 Ablehnungsfälle.
  Exakte Auswahl, Befehle und Logs unter
  `.local/original-economy/oe09/run_final_fix.py`, `*-final-fix-result.json` und
  `*-final-fix-tests.log`. Die letzte Korrektur berührt keine World-/Vergleichs-
  Quelle; die frischen74er-Vergleichsberichte bleiben vollständig gebunden.
- Erweiterter World-Header kollidierte zunächst mit gleichnamigen Fork-Datentypen
  im vierten Probe. Ausschließlich acht private Vergleichskopien erhalten nun
  streng begrenzte Namespace-/Include-Ersetzung; alle ursprünglichen Forkdateien
  bleiben gegen ihre festen Hashes unverändert. Vier neue Transformations-/
  Grenztests **PASS**; exakte World-/OriginalState-Headerpins sichern aktuelle ABI.
  Keine Regelarithmetik oder ursprüngliche Forkquelle geändert.
- Frischer echter Vergleich in Debug/Release/ASan+UBSan **je74/74 PASS**,
 46 parity/18 corrected_bug/6 unverified/4 unsupported; je18 unveränderte
  HousingView-Beobachtungen und1800 Originalmodellwerte per Digest identisch.
  Private Berichte `parity-isolated-debug`, `parity-release`, `parity-sanitizer`;
  daraus identische portable Evidenz regeneriert. Quellenbindung ergänzt DTO
  und isolierten Adapter, Erwartungen und tatsächliche Werte bleiben unverändert.
- `python3 tests/OriginalEconomyBaselineTests.py -v`: **13/13 PASS**.
  Vollständiger Legacy-Replay nach letzter Difficulty-Korrektur erneut **PASS**:
 72 volle SaveDocuments und tatsächliche
  WorldSnapshots identisch,1485 Policyfälle/167 Manager/165 vorbereitete Maps/
  zwei alte Konflikte unverändert;11 Rampen-/10 Brücken-Restartpaare inklusive
 100-Tick-Fortsetzung in separaten Prozessen identisch. Vertrags-SHA256 weiterhin
  `12823612834d27099189c18e31ed4fed4761c9ff34486227b3075cf1069a49e5`.
  Ausgeführter Befehl:
  `python3 tools/original_economy_baseline.py verify --data .local/gog-extracted/app
  --build .local/reswall/release-build --reference
  .local/original-economy/oe00/reference/contract.json --output
  .local/original-economy/oe00/oe09-final-verification`.
  Der erste ebenfalls bestandene Replay vor der letzten Korrektur bleibt unter
  `oe09-verification/` separat erhalten.
- Unabhängiger World-/Persistenzreview ohne Blocker. Core-Quellen vor dem
 172er-Regressionslauf separat eingefroren; nach begrenzter Werkzeuganpassung
  vollständiger Source-Freeze vor Legacy-Replay. Originaldateien bleiben read-only.
  Gate B bleibt bis OE-12 offen. Dies ist authored/scripted technische Persistenz,
  keine spielbare Originalwirtschaft oder native menschliche Abnahme.
- Abschlussmanifest `.local/original-economy/oe09/final-validation.json` **PASS**:
  finaler Task-Source-SHA256
  `85fa6ec1599003ece65e412bddb6f8d54eea36d4f808ce720119d5f2a2ffd14b`;
  443 von454 alten Source-/Test-/Tool-/Resource-Dateien bytegleich, elf begrenzte
  Dateien reviewed; vier neue Code-/Testdateien und CMake gesondert geprüft.
  Originalmodule, Renderer, MapPermissions und Ressourcen bleiben bytegleich.
  Originalinventar1464 Dateien/819711091 Bytes mit unverändertem Manifest
  `dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`.
  `git diff --check` und38 relative Dokumentlinks **PASS**. Kein OE-10-Code.
