# PROGRESS – bei jeder Codex-Aufgabe aktualisieren

**Status-Vokabular:** `TODO`, `IN_PROGRESS`, `PASS`, `FAIL`, `BLOCKED`, `NOT_RUN`. Belege zu jedem PASS echte Tests.

Codex: Einen Task-Commit erzeugen, danach den SHA in der **Antwort** nennen. Bei der **nächsten** Aufgabe die vorige SHA in dieser Datei nachtragen, weil ein Commit seine eigene SHA nicht enthalten kann.

| ID | Status | Commit-SHA (im Folgecommit nachtragen) | Testbelege (Pfad/Run) | Blocker/Notiz |
|---|---|---|---|---|
| OE-00 | PASS | 05470049fa4acfdf2d0104f5a25384c9277073ac | `.local/original-economy/oe00/{reference,verification,validation}/` | 45 relevante CTests,13 Harness-Tests,12 Fork-Untertests;72 Saves/Snapshots und1485 Policyfälle wiederholt identisch; keine Simulation geändert |
| OE-01 | PASS | im Folgecommit | `.local/original-economy/oe01/`, OE-00/`oe01-verification/` | Raw-Reader/Provenance;6 finale CTests PASS;72 Saves/1485 Policyfälle identisch; keine World-Anbindung |
| OE-02 | TODO | — | — | — |
| OE-03 | TODO | — | — | — |
| OE-04 | TODO | — | — | — |
| OE-05 | TODO | — | — | — |
| OE-06 | TODO | — | — | — |
| OE-07 | TODO | — | — | — |
| OE-08 | TODO | — | — | — |
| OE-09 | TODO | — | — | — |
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
| A | TODO | — |
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
