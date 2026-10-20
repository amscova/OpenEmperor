# PROGRESS – bei jeder Codex-Aufgabe aktualisieren

**Status-Vokabular:** `TODO`, `IN_PROGRESS`, `PASS`, `FAIL`, `BLOCKED`, `NOT_RUN`. Belege zu jedem PASS echte Tests.

Codex: Einen Task-Commit erzeugen, danach den SHA in der **Antwort** nennen. Bei der **nächsten** Aufgabe die vorige SHA in dieser Datei nachtragen, weil ein Commit seine eigene SHA nicht enthalten kann.

**Fortsetzung am2026-10-18:** Der Nutzer hebt die Pause nach OE-24 ausdrücklich
auf und beauftragt OE-25. Dieser Task ist umgesetzt und geprüft; sein
versionierter Vertrag steht in `housing-stages-runtime.md`. Als Nächstes ist
OE-26 vorgesehen, bleibt aber nach ausdrücklichem Nutzerwunsch erneut
pausiert und unbegonnen.

**Zwischenmerge am2026-10-18:** Der Nutzer beauftragt anschließend ausdrücklich
Commit und Merge des gesamten geprüften Standes bis OE-25. Diese Freigabe
umfasst Push und Pull Request nach `main`; die Aufgabenpause vor OE-26 bleibt
bestehen. Dies ist keine Endabnahme von OE-26–33 oder Gate D–F.

**Abschluss am2026-10-19:** OE-26 ist vollständig umgesetzt und technisch geprüft.
Vertrag: `docs/original-economy/elite-footprints-runtime.md`.

**Abschluss OE-27 am2026-10-19:** Fünf gebundene Difficulties, modellabhängige
Haussteuer, installierter Unterhalt und aktuelle Wohnmission sind umgesetzt.
[Finanzabnahme](../original-economy/finance-validation.md): 684 finale CTests PASS;
Gate D technisch PASS, Gesamt-Originalparität weiterhin PARTIAL. Als Nächstes
folgt OE-28. Original-Haussprites29 bleiben offen.

**Abschluss OE-28 am2026-10-20:** Vier Inspectoransichten und begrenzte
Overlays sind umgesetzt;93 gezielte Konfigurations-CTests technisch PASS. Vorabvertrag:
[housing-inspector-runtime.md](../original-economy/housing-inspector-runtime.md).
Rule10/Schema26/Policy3/Revision27 bleiben unverändert.

**Abschluss OE-29 am2026-10-20:**15 belegte native Hausgrafiken für Typ3..17,
explizite statische Variante0 und benannte Vacant-/Plot-Fallbacks.
[Abnahme](../original-economy/original-house-visuals-validation.md):174 gezielte
CTests PASS; alle alten Inspector-/Sandbox-Pixel exakt. Gate E bleibt OE30.

**Abschluss OE-30 am2026-10-20:** Optionaler Original-Menüstart mit fünf
Difficulties, bezahltem Vier-Haus-Starter oder leerer Stadt und manuellen
Rule10-Saves. [Abnahme](../original-economy/paid-starter-validation.md):711 volle
CTests PASS, echte Auf-/Abstiege und separate20k-Prozessreplays, normaler
Menü-Build-/F5/F9-/Restartablauf und geöffnete1×/2×/4×-Bilder. Gate E technisch
PASS; Originalgrafik-/Gesamtparität weiterhin PARTIAL, Human/native NOT_RUN.
Recovery31 und alle späteren Aufgaben bleiben unbegonnen.

**Abschluss OE-31 am2026-10-20:** Optionale aktive Original-Recovery ist
modellgebunden und gegen feindliche Saves/Metadaten, gemischte Historien,
Zeit-/Sequenzfehler und fehlende Live-Capturequelle gehärtet.
[Abnahme](../original-economy/save-recovery-validation.md):219 relevante
Konfigurations-CTests PASS,30 tatsächliche Recovery-Restarts und drei separate
Quellenpaare, echte SDL-F5/F9-Fälle sowie normale Menü-Prozessneustarts.
Schemas1–25,72 komplexe Legacy-Saves/1485 Policyfälle und alle Originaldateien
bleiben exakt. OE32/33 und Gate F bleiben offen; Originalgesamtparität PARTIAL.

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
| OE-09 | PASS | a1a352bd697e8a424fb6d76875782d84d70b930a | `.local/original-economy/oe09/`, OE-00/`oe09-final-verification/` | Schema23/OriginalHouse;519 CTests plus33 finale Wiederholungen PASS; echte F5/F9-/Prozess-Roundtrips und72 Legacy-Saves/1485 Policyfälle identisch;Gate B offen |
| OE-10 | PASS | `b4c22e6ecc49558e137288801d2e2a866cd8fc44` | `.local/original-economy/oe10/`, OE-00/`oe10-verification/` | Clock-only Mapping1;177 relevante CTests PASS, je20k tatsächliche Restores/F5F9/Prozessreplay;72 Legacy-Saves/1485 Policyfälle identisch;Gate B offen |
| OE-11 | PASS | `b3b15a6ec98912a35982d9bcec30d7a651fed0a3` | `.local/original-economy/oe11/`, OE-00/`oe11-verification/` | Immutable Sessionmodell;207 relevante CTests PASS;F5/F9/Quellenisolation/191 Refusals;72 Legacy-Saves/1485 Policyfälle identisch;Gate B offen |
| OE-12 | PASS | `ec3a2d60c7541f8a11338c80d5398ce54e51f3a8` | `.local/original-economy/oe12/implementation/`, OE-00/`oe12-final-verification/` | Rule8/Schema24: bezahlte Well72-Quellen,20 Häuser, Abriss/Wiederkauf/Prozessreplay;181 unterschiedliche CTests je Konfiguration belegt;72 Legacy-Saves/Snapshots und1485 Policyfälle exakt;Gate B PASS |
| OE-13 | PASS | `887dda562622d59487a92b8966b62b218cb59ce3` | `.local/original-economy/oe13/implementation/`, `oe13/goods-fixtures/run-d4mm76cf/`, OE-00/`oe13-final-verification/` | Rule9/Schema25-Buchhaltungsfundament;117 relevante Konfigurations-CTests, echte F5/F9-/40k-Prozessfortsetzungen;72 Legacy-Saves/Snapshots und1485 Policyfälle sowie eingefrorene7/23-/8/24-Bytes exakt; reale Waren0/unsupported, Gate C offen |
| OE-14 | PASS | `2bd6878f0ab06af0764d84120f8bf9cb6c2dd726` | `.local/original-economy/oe14/`, OE-00/`oe14-final-verification/` | Aktive Rule10/Schema26/Revision14; bezahlter Zuzug/Worker, echter Clay→Pottery→Warehouse→Ceramics-Ursprung,129 relevante CTests und separate20k-Prozessreplays; alte1–9/1–25-Verträge erhalten; Gate C offen |
| OE-15 | PASS | `390c87481b2259a0e95559e0c975e974a7b627af` | `.local/original-economy/oe15/implementation/`, `oe15/production-fixtures/final-source-5f09/`, OE-00/`oe15-final-verification/` | Aktive Revision15: reale bezahlte Market→House-Lieferung, sichere Rückgabe, begrenzte Transferhistorie;135 relevante Konfigurations-CTests/3×74 Forkvergleiche/separate20k-Prozessreplays; alte1–9-Verträge exakt, Gate C offen |
| OE-16 | PASS | `86cde0c54577a9889632e45432fe7fe18acd7fac` | `.local/original-economy/oe16/implementation/`, `oe16/production-fixtures/final-source-55c8/`, OE-00/`oe16-final-verification/` | Revision16: bezahlte HempFarm192/Feld194, echte Zweigüterankunft/separate Prozessreplays;147 relevante Konfigurations-CTests/3×74 Vergleiche,72 Legacy-Saves/Snapshots und1485 Policyfälle exakt. Gate C TODO |
| OE-17 | PASS | `b30718f` | `.local/original-economy/oe17/implementation/`, `oe17/production-fixtures/debug-first/`, OE-00/`oe17-final-verification/` | Revision17: tatsächlicher Konsum an0/7 mit strikter House-Historie;153 relevante Konfigurations-CTests, zwei Modellpfade/vier20k-Prozessreplays,25 Saves unabhängig rekonstruiert; Legacy exakt, Gate C TODO |
| OE-18 | PASS | `144eec6` | `.local/original-economy/oe18/implementation/`, `oe18/production-fixtures/final-53/`, OE-00/`oe18-final-verification/` | Revision18: bezahlte FoodFarm193/Felder195,196; tatsächliche Qualität20/35/80,159 volle Konfigurations-CTests plus3 nach reinem Formatfix, acht20k-Prozessreplays; Legacy exakt. Gate C TODO |
| OE-19 | PASS | `ba116b4c97defdf134434c1b0398292fb3727d0e` | `.local/original-economy/oe19/gate-c/final-3/`, `oe19/implementation/`, OE-00/`oe19-final-verification/` | Revision19: bezahlte207/208-Ankünfte und getrennte96er-Timer; drei volle202er-Suiten, zwölf20k-Prozessfortsetzungen, strikter Restore, Legacy exakt. Gate C PASS; Originalgrafik/native/human offen |
| OE-20 | PASS | 825fd2c65d90c9139e283f2bf3e191bb5feb4e39 | `.local/original-economy/oe20/implementation/runs/final3/`, `oe20/production-fixtures/final/`, OE-00/`oe20-final-verification/` | Revision20: bezahlter Ancestor214, echter40er-Besuch/Verfall/Wiederbesuch,369 relevante Konfigurations-CTests,18 separate20k-Fortsetzungen und Legacy exakt. Gate D bleibt TODO; Originalgrafik/native/human offen |
| OE-21 | PASS | d0dae2830a7025c12b32ce783fa6de416e131b44 | `.local/original-economy/oe21/implementation/runs/final4/`, `oe21/production-fixtures/final4/` | Revision21: bezahltes Wasser, echter Hut→Plain-Aufstieg, zehnte negative Auswertung/Rückentwicklung,141 Konfigurations-CTests und24 separate20k-Prozesse; Gate D bleibt TODO |
| OE-22 | PASS | 1e7e2715704bd7b5def483d4924292c1d4d41181 | `.local/original-economy/oe22/validation2/`, `oe22/production-fixtures/final2/` | Revision22: bezahlte215/217/219, getrennte40er-Timer/OR, echte3×3-Ankunft;150 Konfigurations-CTests und30 separate20k-Fortsetzungen. Confucian-Elite/Material ausdrücklich authored/offen; Gate D TODO |
| OE-23 | PASS | 95967cec1e691a50e9a771a59940deda2230807b | `.local/original-economy/oe23/validation/`, `oe23/production-fixtures/final/` | Revision23: bezahlte211/212/213, neun getrennte Timer, reine Verfügbarkeit/Ankunft-Diagnose und echte Quellenbrände;159 Konfigurations-CTests,36 separate20k-Fortsetzungen. Originale Venue-/Performer-Kette weiterhin ausdrücklich experimentell; Gate D TODO |
| OE-24 | PASS | 7360f84e40c6cd558d3cf87de41c8b713645840f | `.local/original-economy/oe24/validation2/`, `oe24/production-fixtures/final2/` | Revision24: vier bezahlte Luxusketten, fünf getrennte Vorprodukte und echter Paper→Academy-Materialpfad; 168 Konfigurations-CTests, 43 separate20k-Replays, beide Software-SDL-Ansichten geöffnet. Historische Nutzerpause danach, am2026-10-18 aufgehoben; Gate D TODO |
| OE-25 | PASS | 9bac35b5626834b42d35d52c23b11eb039b14f21 | `.local/original-economy/oe25/validation3/`, `oe25/production-fixtures/final3/` | Revision25:15 Zeilen, bezahlte getrennte Elitefamilie, echte13→12→13 und Common6;177 relevante CTests/50 getrennte20k-Replays/4 geöffnete Softwarebilder;4×4-Geometrie26/Originalsprites29/Gate D27 offen |
| OE-26 | PASS | bcba05334494b4f6f5afd7d4c0d2d76a13469caa | `.local/original-economy/oe26/validation2/`, `oe26/production-fixtures/final2/` | Revision26: atomare leere2×2→4×4-Elitegeometrie, echte Blockade/Reparatur/Einzug160, neun Konflikte/strikter Restore;231 CTests/51 separate20k-Replays/8 geöffnete Softwarebilder,72 Legacy-Saves/1485 Policyfälle exakt; Gate D27/Originalsprites29 offen |
| OE-27 | PASS | eb40408a57038d775dfa79d3c8e9f1bbf189f80d | `.local/original-economy/oe27/gate-d/final4/`, `oe27/validation/final-validation.json` | Revision27: fünf Difficulties, Modellsteuer/Workerunterhalt, signed Debt und verlustfähige Mission; 684 volle CTests, 101 getrennte20k-Fortsetzungen, zehn geöffnete Softwarebilder, Legacy exakt; Gate D technisch PASS, Originalparität PARTIAL |
| OE-28 | PASS | 15dd6da9183617f39f711df2b2ab1598f6595cb9 | `.local/original-economy/oe28/final-validation.json` | Vier Inspectoransichten, Supply/DES/Risk-Overlays;93 gezielte CTests, zwei20k-Prozessreplays, zehn geöffnete1×/2×/4×-Softwarebilder; Authority/alte Pixel exakt; Originalsprites29 offen |
| OE-29 | PASS | 64cbfcb7c13a4b162cfb8bab118e45ef99b09d3d | `.local/original-economy/oe29/final-validation.json` |15 belegte Typ3..17-Bilder, statische Variante0;174 gezielte CTests/12 geöffnete finale Softwarebilder; alte Pixel/Authority exakt; Vacant/Plot-Fallback, Originalszene UNKNOWN/Gesamtwirtschaft PARTIAL, native/human offen |
| OE-30 | PASS | 324fc8fd765c4c4485d7d69e97e39530a504b95d | `.local/original-economy/oe30/final-validation.json` | Optionaler Rule10-Menüstart,87 bezahlte Commands/4 Häuser, echte Auf-/Abstiege und Warenverbrauch;711 volle CTests,6 separate20k-Prozesse,51 bytegleiche Menübilder und72 Legacy-Saves/1485 Policyfälle exakt. Gate E technisch PASS; Recovery31/Human/ganze Originalgrafik offen |
| OE-31 | PASS | — | `.local/original-economy/oe31/final-validation.json` | Aktive modellgebundene Recovery;219 relevante Konfigurations-CTests,30 echte Recovery-Prozessfälle/3 Quellenpaare,25-Schema-Bytekorpus und72 Legacy-Saves/1485 Policyfälle exakt; Capture-/Metadaten-/Historienfehler gehärtet. Gate F/Originalgesamtparität offen |
| OE-32 | TODO | — | — | — |
| OE-33 | TODO | — | — | — |

## Phasen-Gates

| Gate | Status | Beleg |
|---|---|---|
| A | PASS | OE-07: gepinnter Vier-Modul-Vergleich/74 Vektoren, Parser-/Quellenbindung/Negativfälle,507 CTests in drei Konfigurationen und exakter Legacy-Replay; begrenzte Kernabnahme, keine Gesamt-Originalparität |
| B | PASS | OE-08–12: ausdrücklich freigegebener Rule8-/Schema24-Nachfolger; vollständige Gate-Suites und finale gezielte Wiederholungen in Debug/Release/ASan+UBSan, bezahlte20-Haus-/Quellen-/Reload-Szene und exakter Legacyvergleich; technische begrenzte Abnahme, native/human NOT_RUN |
| C | PASS | OE-13–19: tatsächliche bezahlte Ceramics-/Hemp-/Food-Lieferung, Konsum und medizinische Ankunft;606 vollständige Konfigurations-CTests,12 getrennte20k-Prozessfortsetzungen, unabhängige Raw-Audits und unveränderter Legacyvertrag. Begrenzte technische Abnahme; Originalart/native/human nicht abgenommen |
| D | PASS | OE-20–27: bezahlte Housing-/Waren-/Serviceketten,15 typisierte Hauszeilen und atomare Elite4×4-Geometrie; fünf Modellschwierigkeiten/Steuer, strikter Restore; drei frische Vollbuilds und684 vollständige CTests, separate20k-Prozesse und Legacyvergleich. Begrenzte technische Abnahme; höhere Originalstufen, Originalcollector/-sprites und native/human bleiben offen, Gesamtparität PARTIAL |
| E | PASS | OE-28–30: optionaler produktiver Rule10-Menüstart und bezahlter Vier-Haus-Kreislauf,711 vollständige CTests/3 Konfigurationen, tatsächliche Auf-/Abstiege/Ankünfte/Verbrauch/F5F9/Neuprozess-/20k-Bytebeweise, geöffnete1×/2×/4×-Softwarebilder; ältere72 Saves/1485 Policies exakt. Technisch begrenzte Abnahme, native/human NOT_RUN, Originalparität PARTIAL und Recovery31 offen |
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

### OE-10 – 2026-10-16, Basis5ed833f

- Start am sauberen, live/origin bestätigten `main`
  `5ed833f4b0e97ac6a9e327f43e95a155732a9b6e`; vorhandener Feature-Branch per
  Fast-forward angeglichen. OE-09-SHA oben nachgetragen; Vorzustand unter
  `.local/original-economy/oe10/initial-state.json`.
- [Zeitvertrag](../original-economy/scheduler.md): Mapping1, ausdrücklich authored
  ein vorhandener20-Hz-Worldtick pro Phase;51 Phasen,16 Schritte/Monat und12
  Monate/Jahr. Nur gespeicherte `world.ticks` plus feste Rule7/Schema23-Identität
  sind Clock-Authority. Kein JSON-Feld, redundanter Kalender oder Migration.
  Tick0 beginnt mit nächstem Dispatch0; Service35, Evolution39, danach Verbrauch
  nur Selector0/7. Die Beschreibung aktiviert noch keine Wirtschaft.
- Neue begrenzte Originalanalyse **PASS**: elf private Fenster,99 Instruktionen/
  348 Bytes,22 unabhängige Opcode-/Pointerassertions, tatsächliche PE-Bytes
  gegengeprüft; EXE-SHA/Bytes/Modus/mtime vorher/nachher identisch. Neu separat
  belegt ist die16er-/12er-Counterstruktur. Originaler Jahrescounter steigt bei
  größerem Counterwert1; das neue Jahr ist deshalb ausdrücklich elapsed-year ab
  eigener Epoche0. Befehle/Belege unter
  `.local/original-economy/oe10/check-scheduler-evidence.py`,
  `scheduler-exe-evidence.json` und `scheduler-evidence-notes.md`.
- Rule7-Tick erhöht ausschließlich Zeit, vorab atomare Erschöpfungsprüfung.
  Terminaler Tick `UINT64_MAX−1` bleibt speicherbar; weiterer Tick sowie `MAX`
  werden verweigert. Nur vollständig inaktive Rule7 umgeht alten Health-
  Deadline-Headroom; alte Guards unverändert. Später gekaufte leere Container
  speichern tatsächliches `placed_tick<=ticks`; House-`cycle_phase` bleibt0.
- Vorhandener makrogebundener SDL-Testadapter nutzt nun unveränderten TickDriver
  für Pause/Step/Speed. Normale Rule7-Konstruktion/Menü/Recovery bleiben zu,
  F9 pausiert und löscht Frameakkumulator, Reset bleibt gesperrt.
- Erster vollständiger Build verweigerte den neuen World-Test wegen Aufrufs
  einer privaten Prüfmethode. Test auf vorhandene öffentliche
  `production_balance_valid()` korrigiert, die denselben Industry-Validator
  aufruft; keine Produktions-API geöffnet oder Prüfung reduziert. Erste Logs
  bleiben als `*-initial-build.log` erhalten.
- Vollständige Builds Debug, Release und ASan/UBSan **PASS**, ohne Compilerwarnungen;
  je175 CTests registriert. Bewusst auf diesen Task begrenzte58er-Auswahl:
  Scheduler/20k-Restore, Originalkerne/Quellen, F5/F9/Clock/OS-Prozess, alte
  Simulations-/Demand-/Finance-/Determinismus-/Save-/Recovery-/Transport-/UI-
  Pfade. Exakte Namen/Regex stehen in `.local/original-economy/oe10/selection.json`,
  Configure-/Build-/Inventorybefehle in `run_build.py`, CTests in
  `run_core_tests.py`, je eigene absolute TMPDIR. **Debug58/58 PASS**,182.38s;
  **Release58/58 PASS**,13.03s; **ASan/UBSan58/58 PASS**,46.81s, mit
  `UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`, keine Sanitizer-Meldung.
  Zusammen mit den drei Vertragsprüfungen177 relevante CTests, kein Skip.
  Dies ist keine vollständige
 175er-Phasen-Gate-Suite; Gate B wird erst nach OE-12 abgenommen.
- Neue reine Suite prüft42.579 Behauptungen einschließlich unabhängigem20k-
  Odometer. World-Suite **PASS** in allen drei Konfigurationen:20.000 tatsächliche vollständige
  Restores zwischen Ticks gegen ununterbrochenen Replay, nächste Dispatchs und
  ganze Snapshots gleich,392 Countdown-/392 Evolutions-/50 Verbrauchspositionen,
  nur Zeit geändert; späte bezahlte Platzierung, Terminal-/Overflow-Atomicity
  und alter20-Hz-/400-Tick-Demand zusätzlich geprüft. Vorläufiger direkter Debug-
  Lauf ebenfalls **PASS**, `world-preliminary-debug.log`.
- Erweiterte Save-Suite in allen drei Konfigurationen **PASS**,177 verweigerte Manipulationen;
 20 Dateigrenzen vor/auf/nach35/39/51/816/9792,0/7/15-Selectoren und Maximalwerte.
  Vollständige Dokumente/Bytes sowie nächste Dispatchposition und nächster Tick
  identisch, terminaler Zustand speicherbar, kein Clock-/Calendar-Unknown-Key
  zugelassen. Echter OE-09-Tick0-Golden aus unverändertem Commit und separat
  kopierten damaligen Libraries:6778 Bytes/SHA256
  `9f2392de51302cb7451605eb5c2b7e8aa15cf827801c00cb1b8c21aebf967e6e`,
  nun im Test gegen tatsächlichen Writer geprüft. Private Provenance unter
  `oe09-tick0-golden/`. Der frühere Tick1-Negativfall wurde gezielt durch den
  jetzt ungültigen `MAX` ersetzt und ein positiver Tick1-Zeit-only-Fall ergänzt.
- SDL-Suite in allen drei Konfigurationen **PASS**:120 pausierte Frames, Step/Continue und
  Teilframes,1/2/4-fache Geschwindigkeit, maximal8 Catch-up-Ticks; F9 leert
  laufenden/gebrochenen Framezustand. Neun tatsächliche F5/F9-Checkpoints vor/auf/
  nach51/816/9792, exakter Folgeschritt und Byte-Rewrite; separater OS-Prozess
  und Elternview führen danach dieselben127 echten SDL-Steps aus und schreiben
  vollständig identisch. Bisherige kaputte Save-/Map-/Model-Refusals bleiben.
- Frischer echter Forkvergleich Debug/Release/ASan+UBSan **je74/74 PASS**,
 46 parity/18 corrected_bug/6 unverified/4 unsupported, je18 unveränderte
  Housing-Beobachtungen und1800 Originalmodellwerte per Digest gleich. Die drei
  portablen Berichte sind identisch; `core-parity.json` ergänzt ausschließlich
  den tatsächlich geprüften neuen Scheduler-Headerhash. Numerische Erwartungen,
  Vergleichswerkzeug und feste World-/DTO-ABI-Pins unverändert. Jeweils
  `ctest --test-dir <BUILD> --output-on-failure -R '^original-rules-parity-contract$'`
  **1/1 PASS**; Logs `*-parity-contract.log`, tatsächliche Vergleichsläufe in
  `parity-{debug,release,sanitizer}/`.
- Unabhängige read-only Reviews **PASS**, keine offenen Codeblocker. Alter
  World-Tickbody nach dem engen Rule7-Return sowie vollständiger TickDriver
  bytegleich zum HEAD. Source-Freeze465 Dateien unter `frozen-source.json`,
  Source-SHA256 `9d86f9533553ad5c15a78d1450ee61cd9a2dc33529e84aa7ba00d5f81e3aa759`.
- Vollständiger Legacy-Replay auf finalem gefrorenem Code **PASS**:72 vollständige
  SaveDocuments und tatsächliche WorldSnapshots identisch;1485 Policyfälle,
  167 komplette Manager/165 vorbereitete Maps/zwei alte Konflikte unverändert.
  Elf Rampen- und zehn Brücken-Neustartpaare inklusive100-Tick-Fortsetzung in
  separaten Prozessen identisch. Vertrags-SHA256 weiterhin
  `12823612834d27099189c18e31ed4fed4761c9ff34486227b3075cf1069a49e5`.
  Ausgeführter Befehl: `python3 tools/original_economy_baseline.py verify
  --data .local/gog-extracted/app --build .local/reswall/release-build
  --reference .local/original-economy/oe00/reference/contract.json
  --output .local/original-economy/oe00/oe10-verification`.
- Abschlussmanifest `.local/original-economy/oe10/final-validation.json` **PASS**:
  alle465 eingefrorenen Quellen identisch;452 von458 alten Source-/Test-/Tool-/
  Resource-Dateien bytegleich, exakt sechs reviewed Dateien geändert. CMake
  und drei neue Code-/Testdateien gesondert geprüft. Ganze Produktionspersistenz,
  bisherige Originalkerne, World.h/DTO-ABI, MapPermissions, Renderer, Menüs,
  Main und Recovery bleiben bytegleich. Originalinventar1464 Dateien/819711091
  Bytes, unverändertes Manifest
  `dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`.
  `git diff --check` und47 relative Dokumentlinks **PASS**. Gate B bleibt offen;
  kein OE-11-Code, keine native menschliche Abnahme oder originale Echtzeitparität.

### OE-11 – 2026-10-16, Basisd6eb736e

- Start am sauberen, live/origin bestätigten `main`
  `d6eb736e1dbf0f08c26b933d3beb4c132ee5192a`; vorhandener Feature-Branch per
  Fast-forward angeglichen. OE-10-SHA oben nachgetragen; eigener Commit folgt
  im Abschluss und beim nächsten Prompt. Vorzustand unter
  `.local/original-economy/oe11/initial-state.json`.
- [Sessionvertrag](../original-economy/model-session-binding.md): genau eine
  unveränderliche Modellinstanz pro gebundener World, mit Fileprovenance,
  Difficulty,15 House-/sieben Building-Zeilen. Einmalige vollständige Vorbereitung
  durch vorhandene geprüfte typisierte Leser; danach keine Rohdatenabhängigkeit,
  kein globaler Cache oder World-Datenroot. Die tatsächliche Originaldatei mit
  34218 Bytes/SHA `7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6`
  wird ausschließlich gelesen. Andere gültige Quellen erhalten eigene Identität.
- Create, privater Konstruktor und ausdrücklicher Restore erhalten die geprüfte
  Modellinstanz explizit. Vollständiger Format-/SHA-/Bytezahl-/Difficulty-Vergleich;
  ein ungebundener Snapshot wird nicht still nachgebunden. Alte ungebundene
  Infrastruktur-/Clockfixtures bleiben unspeicherbar. Normale Rule7-App, Menü,
  Reset und Recovery weiterhin geschlossen, Zeitbasis und aktive Wirtschaft
  unverändert.
- F5 übernimmt den bestehenden World-Pointer transient im SaveDocument und
  prüft live nur die begrenzt gelesenen Datei-SHA/Größe. F9 bereitet genau einen
  frischen Kandidaten vor; Veröffentlichung erst nach sämtlichen vorhandenen
  Map-/Policy-/Snapshot-/Accounting-Prüfungen. `read_save` erzeugt keinen Pointer.
  Fehlende, geänderte oder semantisch ungültige Quellen verändern weder aktive
  World/Modellinstanz noch vorhandene Savebytes. Kein JSON-Key, keine Migration
  oder zusätzliche Modellanforderung für Schemas1–22.
- Ein gemeinsamer interner read-only Reader erhält sämtliche alten Parser-
  Pfad-/Containment-/Größen-/Readfehler und stellt zusätzlich eine reine bounded
  Fingerprintprüfung bereit. Sie verleiht keine semantische Gültigkeit. Reine
  World-Ticks, Frames, Rendern und vorbereitete Getter besitzen keinen Lade-
  oder Parsingpfad; bestehende Session bleibt bei entfernter Datei verwendbar.
- Die bisherige OE-09-Testquelle war ausschließlich syntaxgültig und enthielt
  freie Namen/eine Building-Zeile. Ihr ursprünglicher6778-Byte-Golden wird
  weiterhin exakt rekonstruiert und nun als Runtime-Negativkontrolle geprüft.
  Zusätzlich wurde die neue vollständig authored, korrekt benannte Fixture
  tatsächlich mit eingefrorenem OE-09-Writer/Headern/Libraries ausgeführt:
  unveränderte6778 Savebytes/SHA
  `4e70a04b19700c86d974d53df4edb2fdcf3bb264622ef76c4c32a1be5b63ca68`.
  Historischer und neuer Golden sind getrennt, kein Assertion-Reset. Private
  Provenance in `oe09-valid-fixture-golden-final/provenance.json`; Begründung
  in [DECISIONS](DECISIONS.md).
- Vollständige Builds Debug, Release und ASan/UBSan **PASS**, keine
  Compilerwarnungen; je177 CTests registriert. Tatsächlich ausgeführte66er-
  Taskauswahl: Modellparser/-schema/-vorbereitung, pure Regeln, Scheduler,
  World/Save/F5F9/OS-Prozess, alte Simulation/Finance/Demand/Determinismus,
  Bridge/Ramp, Recovery, Menü/Session/Mapauswahl/Input. Exakte Namen/Regex und
  Befehle in `selection.json`, `run_build.py` und `run_core_tests.py`, je eigener
  absoluter TMPDIR. **Debug66/66 PASS**, 189.31s;
  **Release66/66 PASS**, 14.21s;
  **ASan/UBSan66/66 PASS**, 56.06s mit
  `UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`; keine Sanitizer-Meldung.
  Hinzu je eine Paritäts-Vertragsprüfung: **201 reguläre relevante CTests**, kein
  Skip. Keine vollständige177er-Gate-Suite; Gate B folgt erst nach OE-12.
  Vorläufige Debug-Smokes (Modell2/2, Rule-Gate1/1) bestanden ebenfalls.
- Das zusätzliche Review fand einen Testharness-Randfall: `real()` übersprang
  einen vorhandenen defekten Model-Symlink als fehlend. Nach vollständig
  abgeschlossenem regulärem Prüflauf ausschließlich diesen Testguard verschärft;
  private Vorherquelle/-manifeste und exakter Ein-Datei-Diff erhalten. Alle
  Produktions-, CMake-, Tool-, Resource- und übrigen Testdateien blieben bytegleich.
  Betroffenes Target in allen drei Konfigurationen erneut gebaut, Modelsuite
  je2/2 **PASS** und direktes CLI je echte Abwesenheit→77/defekter Symlink→1
  geprüft. **Sechs zusätzliche CTests**, insgesamt207; keine weitere
  Produktionsänderung oder neue Legacy-Semantik. Protokolle in
  `check_real_source_guard.py`, `*-guard-result.json`; Abschlussaudit in
  `finalize_guard.py` verbindet beide Source-Stände exakt.
- Neue Modelsuite **PASS** in allen drei Konfigurationen:20042 synthetische
  und2264 reale Assertions. Literal authored Arithmetic/Difficulty,
  Speicherprovenance-/Label-/Overflow-/Enum-Refusals, volle size_t-Domäne,
  unterschiedliche Quellen, voneinander unabhängige Sessions und20000 Ticks
  mit entfernter Datei geprüft. Die reale Suite vergleicht alle2255 vorbereiteten
  Building-/House-Werte über fünf Difficulties mit den bestehenden geprüften
  Lesern; Originalbytes vorher/nachher identisch.
- Save-Suite **PASS** in allen drei Konfigurationen:191 verweigerte
  Manipulationen, Pointerbindung/-lebensdauer, expliziter und frischer Restore,
  gleiche Bytes an anderem Root, andere Daten/Hashes/Difficulties und atomare
  Retryfälle. Vollständige alte Clock-/F5F9-Grenzen und20000 tatsächliche World-
  Restores weiterhin bestanden; keine Aktivierung von Waren/Bewohnern/Evolution.
- SDL-Suite **PASS** in allen drei Konfigurationen:120 sourcefreie Frames,
  zwei tatsächliche Steps und Rendern mit stabiler Instanz/ohne gezählte Datei-
  oder Assetarbeit; fehlender Create/F5/F9 schützt aktive Session und Save.
  Nach Wiederherstellung erfolgreicher F9 mit neuer unabhängiger Instanz.
  Zwei eigene Datenroots liefern vorbereitete Housewerte0/37 ohne aktive
  Kapazität; Cross-Source-F9 wird atomisch verweigert. Neun bisherige tatsächliche
  F5/F9-Zeitgrenzen und separate OS-Prozessfortsetzung um127 Steps mit vollständiger
  SaveDocument-/Bytegleichheit weiterhin bestanden.
- Frischer tatsächlicher Forkvergleich in Debug/Release/ASan+UBSan **je74/74
  PASS**, unverändert46 parity/18 corrected_bug/6 unverified/4 unsupported,
  je18 Housing-Beobachtungen und1800 Originalmodellwerte per Digest gleich.
  Die drei portablen Berichte sind identisch; `core-parity.json` erneuert nur
  tatsächliche Sourcehashes. World-ABI-Pin nach reviewed transientem Pointer-
  Zusatz aktualisiert, DTO-Pin unverändert; keine numerischen Erwartungen,
  Forkregeln oder Vergleichsklassen geändert. Je Konfiguration
  `ctest --test-dir <BUILD> --output-on-failure -R '^original-rules-parity-contract$'`
  **1/1 PASS**. Belege in `parity-{debug,release,sanitizer}/` und Vertragslogs.
- Vollständiger Legacy-Replay auf unverändert gebliebenem Produktionscode **PASS**:
  72 komplette
  SaveDocuments und tatsächliche WorldSnapshots identisch;1485 Policyfälle,
  167 vollständige Manager/165 vorbereitete Maps/zwei alte Konflikte unverändert.
  Elf Rampen- und zehn Brücken-Neustartpaare einschließlich100-Tick-Fortsetzung
  in separaten Prozessen identisch. Vertrags-SHA256 weiterhin
  `12823612834d27099189c18e31ed4fed4761c9ff34486227b3075cf1069a49e5`.
  Der anschließende ausschließlich engere Realtest-Skipguard ist im
  Abschlussmanifest gesondert nachgewiesen; alle Produktionsdateien gleich.
  Tatsächlich ausgeführter Befehl: `python3 tools/original_economy_baseline.py verify
  --data .local/gog-extracted/app --build .local/reswall/release-build
  --reference .local/original-economy/oe00/reference/contract.json
  --output .local/original-economy/oe00/oe11-verification`.
- Unabhängige Read-only-Reviews ohne offenen funktionalen Befund. Abschlussmanifest
  `.local/original-economy/oe11/final-validation.json` **PASS**, alle
  468 gefrorenen Quellen unverändert, Source-SHA256
  `2a0257193d1ba22a5395ad34c78ba27629e45d0f977a9a3e8aae868d8b6669fe`.
  449 von461 alten
  Source-/Test-/Tool-/Resource-Dateien bytegleich; genau zwölf reviewed Dateien
  geändert, zusätzlich CMake und drei neue Code-/Testdateien. MapPermissions,
  Renderer, TickDriver, Main, Menüs und Recovery unverändert. Originalinventar
  1464 Dateien/819711091 Bytes mit unverändertem Manifest
  `dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`.
  `git diff --check` und relative Links der aktualisierten Dokumente **PASS**.
  Gate B bleibt offen; kein OE-12-Code, kein menschlicher/native Gameplay-PASS
  und keine pauschale Originalparität.

### OE-12 – 2026-10-16, Basisdf29393 – BLOCKED

- Beginn am sauberen, live/origin bestätigten `main`
  `df293931bbb8a409668f3790379bb370e6c9f44d`; vorhandener Feature-Branch per
  Fast-forward angeglichen. OE-11-SHA oben nachgetragen. Vorzustand unter
  `.local/original-economy/oe12/initial-state.json`.
- Drei Read-only-Teilreviews prüfen Modell-/Quellenidentität, World-/Save-/
  Finanzgrenzen sowie UI-/Gate-Anforderungen. Alle sieben vorhandenen
  World→Model-Kandidaten sind ausdrücklich `Inferred`/Mechanik `Unknown`.
  Verifizierte Modellzeilen oder Well-/Herbalist-Grafiken belegen keine
  Original-Quellenzulassung. Die Stop-Regel des Prompts gilt weiter.
- Nutzerentscheidung auf konkrete Rückfrage: **„Die strenge Grenze beibehalten
  und OE-12 mit diesen Blockern dokumentieren“**. Keine Modellanalogie als
  verifiziert aufgewertet, Source-/Abriss-/Restore-Gates oder Funds geändert.
  Kein leerer Cache als abgeschlossene Originaldaten-Integration ausgegeben.
- Frische Release-Builds der vorhandenen Modell-/Schema-/Desirability-/Rule-Gate-
  Targets **PASS**, ohne Compilerwarnung. Die private C++20-Probe verwendet die
  tatsächlichen Header und aus dem aktuellen Target-Linkkommando übernommenen
  Libraries, lädt die echte unveränderte Modelldatei und führt normale
  `World::execute`-Befehle aus. Keine World-/Snapshot-/Funds-Injektion.
  Repro: `python3 .local/original-economy/oe12/run_prerequisites.py`;
  Compile-/Runbefehle und Libraryhashes in `probe-run.json`/`probe-compile.log`.
- Diagnose **BLOCKERS_REPRODUCED**, ausdrücklich kein OE-12-Feature-PASS:
  Alle sieben Source-Baubefehle werden atomisch verweigert. Zwanzig reguläre
  House-Kaufversuche auf konfliktfreiem authored Gelände ergeben16 bezahlte
  Häuser,1280 tatsächliche Bauausgaben und20 verbleibende Funds; vier weitere
  Käufe scheitern ohne Zustandsänderung. Zwanzig Häuser kosten bereits ohne
  Straßen/Quellen1600, vorhandener Startbetrag1300. Der tatsächliche Abrissbefehl
  wird ebenfalls unverändert verweigert. Nach20000 normalen Ticks bleiben
  Treasury/Accounting/Häuser unverändert und Steuern0. Der getrennte statische
  Restore-Check verlangt weiterhin zugeordnete Vacant-House-Nullzustände.
  Vollständige Ergebnisse in `prerequisites.json`; Originalmodell nur als
  Quellenhash, keine kopierte Modellwertetabelle.
- Vor CTest vollständige Release-Inventur:177 registrierte Tests.
  `ctest --test-dir .local/reswall/release-build --output-on-failure -R
  '^original-(model-schema-(synthetic|real)|rules-model-(synthetic|real)|desirability-(synthetic|real)|economy-rule-gate)$'`
  **7/7 PASS**,0.71s, kein Skip. Dies bestätigt die bestehenden Parser-/
  Identitäts-/Kern-/Gateverträge, nicht die fehlende produktive Integration.
  Logs: `release-inventory.log`, `release-controls-tests.log` und
  `release-controls-last-test.log`.
- Kein neuer Desirability-Cache, keine Debug-Provenance-Implementierung oder
  bezahlte Quelle; erforderliche Add/Remove-/20-Haus-/Reload-/City-v13-gegen-
  Rule7-Integrationsabnahme **NOT_RUN**. Vollständige Debug-/Release-/Sanitizer-
  Gate-B-Suites **NOT_RUN**, Gate B **BLOCKED**. Frühere OE-11-Nachweise bleiben
  historisch erhalten und werden nicht als OE-12-Abnahme umetikettiert.
- Dokumentation und [konkrete Fortsetzungsvoraussetzungen](../original-economy/desirability-integration-blockers.md)
  statt Featurefreigabe. Abschlussaudit
  `.local/original-economy/oe12/final-validation.json`: Dokumentations-/
  Diagnoseprüfung **PASS**, Taskstatus ausdrücklich **BLOCKED**. Sämtliche
  CMake-/Source-/Test-/Tool-/Resource-Bytes unverändert, Source-SHA256
  `2a0257193d1ba22a5395ad34c78ba27629e45d0f977a9a3e8aae868d8b6669fe`.
  Originalinventar1464 Dateien/819711091 Bytes weiterhin unverändert;
  Manifest `dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`.
  Kein grüner Featurecommit und kein Fortgang zu OE-13. Ein Dokumentationscommit
  bezeichnet diesen BLOCKED-Befund ausdrücklich und verändert keine Laufzeit.

### OE-12 Fortsetzung – 2026-10-16, Basis23f544a – Quellenbelege, weiter BLOCKED

- Neuer „weiter gehts“-Auftrag nach dem sauberen BLOCKED-Dokumentationsmerge
  `23f544a3611394f041c7401974e67fd87b0c2f1d`; vorige Dokumentations-SHA oben
  nachgetragen, Feature-Branch per Fast-forward angeglichen. Die ausdrückliche
  Nutzerwahl der strengen Grenze bleibt bestehen. Keine wiederholte Bau-/Budget-
  Probe als neuer Fortschritt und kein Überspringen zu OE-13.
- [Originaler Quellenpfad](../reverse/original-desirability-source-admission.md)
  unabhängig statisch präzisiert: Statushelper426d10 akzeptiert ausschließlich
  Status1/3; das übergebene0-Argument wird nicht ausgewertet. Manager44f1d0
  verlangt danach runtimeWord+3c==0 und liest die vier Modellfelder1–4 über
  den tatsächlichen signed Typ+14. Nullwort-Bedeutung und Archivzuordnung
  bleiben offen; der aktuelle geparste `subindex` ist kein Ersatzbeleg.
- Basis-/Industrial-Konstruktoren, ihre virtuellen Slots+94/+168/+16c/+170
  und Initializer428aa0 separat geprüft. Für genau diese belegten Vtables
  wählt+168 false den Standardquadratpfad; +170 liefert die unsigned Seite
  aus runtimeByte+07. Der Initializer übernimmt die Seite aus
  `823598 + 24*Typ`, nicht aus dem Desirability-Delta SZE. Keine Behauptung,
  damit alle sieben künftigen Quellenfabriken oder Sonderformen zu kennen.
- Die schon vorhandene ausdrückliche Vacant-Typ2-Bindung ist kein Ausweg:
  `OriginalHouseState` behält original_status0/accessfalse und wird beim Restore
  gegen diesen Default geprüft. Status0 würde am beobachteten ersten
  Originalquellengate scheitern. Kein pauschales Originaltyp2-Verbot in anderem
  Zustand, kein Alias zur Shelterzeile und keine Statushochsetzung abgeleitet.
- Tatsächlich ausgeführter privater Repro-Befehl:
  `python3 .local/original-economy/oe12/source-admission-research/root/check_source_admission.py`.
  **Statische Evidenzprüfung PASS**: acht begrenzte Fenster,251 Instruktionen/
  766 Bytes einzeln gegen PE-Sections geprüft,38 konkrete Predicate-/Call-
  Assertions und acht direkt gegengeprüfte Vtable-Slots;0 Byteabweichungen.
  Private Originalextrakte und Report ausschließlich unter obigem `root/`.
  Unabhängiger Teilreview bestätigt zusätzlich sieben eigene Fenster und
  die Trennung von Original- und World-Identität. Das Originalspiel wurde
  nicht ausgeführt; keine dynamische Gameplay-/Kernparitätsabnahme.
- Abschlussaudit `root/final-validation.json`: Forschungsprüfung **PASS**,
  OE-12/Gate B ausdrücklich **BLOCKED**, kein Feature implementiert.
  Sämtliche CMake-/Source-/Test-/Tool-/Resource-Bytes unverändert;
  Source-SHA256 `2a0257193d1ba22a5395ad34c78ba27629e45d0f977a9a3e8aae868d8b6669fe`.
  EXE-Bytes/Modus/mtime und vollständiges Originalinventar1464 Dateien/
  819711091 Bytes unverändert, Manifest weiterhin
  `dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`.
  Keine erneuten CTests/Builds oder Gate-B-Vollsuites für diese reine
  Forschungs-/Dokumentationsänderung; frühere Ergebnisse bleiben historisch.
- Bau, sichere Entfernung, Restorezulassung und die Finanzierung der20-Haus-
  Szene bleiben separat beauftragte, noch fehlende Verträge. Mehr Original-
  EXE-Evidenz öffnet sie nicht automatisch. Keine Modellanalogie als verifiziert
  hochgestuft, kein Budget/Gebäudestatus geändert, kein grüner Featurecommit
  und keine Freigabe für OE-13.

### OE-12 Fortsetzung – 2026-10-16, Basise29ce8a – Archivwort und Links, weiter BLOCKED

- Beginn am sauberen, live/origin bestätigten `main`
  `e29ce8a20d13013fce51994175c4bd04b86f1ed4`; Feature-Branch per Fast-forward
  angeglichen. Vorige Quellenforschungs-SHA `c50ff69941c60c61d7d37c69354e3113bdc7f58c`
  oben nachgetragen. Strenge Nutzergrenze unverändert; kein Source-/Budgetfix.
- [Separates Quellenwort](../reverse/original-desirability-source-state.md):
  Runtime-WORD+3c ist im Writer und in allen drei Base3/4/5-Lesern bei raw
  Base54 archiviert. Subindex bleibt getrennt bei runtime+16/Base18. Die
  identischen32 Feldtransfers pro Prefix und das vorausgehende Schemawort
  werden unabhängig geprüft; keine Umrechnung allein aus Runtime-Offsets.
- Reset setzt+3c anfangs0; bestimmte Slotersetzung bewahrt+3c/+3e über
  Konstruktion und Initializer. Ein konkreter Originalerzeuger speichert eine
  vorhandene Manager-ID im neuen+3c und verkettet weitere IDs über+3e. Zwei
  unabhängige Leser folgen+3c als signed Managerindex. Keine vollständige
  Parent-/Lifecycle-Semantik, allgemeine Indexsicherheit oder neue Zulassung.
- Tatsächlich ausgeführt, alle unter
  `.local/original-economy/oe12/source-state-research/`:
  `python3 root/check_source_state.py` (vom Repositoryroot mit dem vollständigen
  obigen Präfix): **PASS**,9 Fenster/157 Instruktionen/519 Bytes/32 Assertions.
  `python3 save-audit/check_archive_word.py`: **PASS**,15 Fenster/962
  Instruktionen/2508 Bytes/30 Assertions/vier32-Feld-Protokolle.
  `python3 state-audit/check_building_word3c.py`: **PASS**,8 Fenster/456
  Instruktionen/1480 Bytes/40 Assertions.
  `python3 roundtrip-audit/check_archive_mapping.py`: **PASS**,8 Fenster/681
  Instruktionen/1802 Bytes/dieselben vier Prefixe. Vollständige Repro-Befehle
  im verlinkten Beleg; Fenster überlappen, keine addierte einzigartige Abdeckung.
  Alle Instruktionsbytes stimmen mit den gepinnten PE-Sections überein;
  EXE-Bytes/Modus/mtime unverändert, Originalspiel nicht ausgeführt.
- Unabhängiger Datenfluss-/Dokumentationsreview ohne Befund. Abschlussaudit
  `root/final-validation.json`: statische Forschung und Dokumentation **PASS**,
  OE-12/Gate B ausdrücklich **BLOCKED**. CMake-/Source-/Test-/Tool-/Resource-
  SHA256 weiterhin `2a0257193d1ba22a5395ad34c78ba27629e45d0f977a9a3e8aae868d8b6669fe`;
  vollständiges Originalinventar1464 Dateien/819711091 Bytes, Manifest
  `dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`.
  Dokumentationslinks und `git diff --check` geprüft. Keine erneuten Builds/
  CTests für diese reine Forschungsdokumentation, Gate-B-Vollsuites **NOT_RUN**.
- Quellenidentität, Sourcebau/Abriss/Restore und die Finanzierung der20-Haus-
  Szene bleiben ungelöste Vertragsvoraussetzungen. Keine Parser-/World-/
  Saveänderung, kein grüner Featurecommit und kein Fortgang zu OE-13.

### OE-12 Fortsetzung – 2026-10-16, Basis837caf8 – sieben Factories, weiter BLOCKED

- Sauberer, live/origin bestätigter `main`
  `837caf8e48272fa48172c3cb7b815a95eda8883e`; Feature-Branch per Fast-forward
  angeglichen. Vorige Zustandsforschungs-SHA `5f30abc9b053ad700199604b4cf0c320dd5a5336`
  oben nachgetragen. Strenge Grenze unverändert, keine wiederholte Budgetprobe.
- [Konkrete Originalfabriken](../reverse/original-desirability-source-factories.md):
  Klassifikationen, frühere Dispatch-Ausschlüsse, tatsächliche Konstruktoren und
  finale Callbacks für35/43/54/59/72/193/207 bytegeprüft. Warehouse54 liefert
  im Quadratpfad konstant Seite3; Farm193 hat Side-Default3 und runtime-Getter.
  Markt59 nimmt den Sonderformpfad; weder Initialside1 noch der für59 null
  liefernde, dort ungenutzte Quadratseitengetter belegt seinen Einflussabdruck.
- Tatsächlich ausgeführte private Python3-Checkers unter
  `.local/original-economy/oe12/source-factory-research/`, vollständige Befehle
  im verlinkten Beleg: `industry-audit/check_industry_factories.py` **PASS**,
  57 Fenster/1276 Instruktionen/4036 Bytes/161 Code- und27 Datenassertions;
  `market-farm-audit/check_market_farm_factory.py` **PASS**,42 Fenster/1021
  Instruktionen/3211 Bytes/82 Codeassertions/18 Tabellenlesungen;
  `well-health-audit/check_well_health_factories.py` **PASS**,26 Fenster/459
  Instruktionen/1459 Bytes/55 Codeassertions. Vtable-Slots12/9/8; Fenster
  überlappen, keine addierte einzigartige Abdeckung oder EXE-Ausführung.
- Neue private `root/WorldFootprintProbe.cpp`, Compile/Run durch
  `root/run_footprint_probe.py`: **PASS**, ohne Compiler-Meldung. Sieben reale
  Aufrufe der vorhandenen Footprintfunktion aus den unveränderten, gegen den
  vorigen OE-12-Lauf fingerprintgeprüften Release-Libraries. Keine World,
  Platzierung oder Gratisquelle erzeugt. Warehousequery2×2 und Farmquery1×1
  weichen von den überprüften Originalseiten ab; vier passende Lookupdefaults
  beweisen keine Identität. Alle Kandidaten weiterhin `Inferred`/`Unknown`.
- `root/check_footprint_comparison.py`: **PASS**,10 Fenster/54 Instruktionen/
  169 Bytes/15 Assertions/28 Slots/sieben Sidebytes; unabhängig gegen die
  tatsächliche Produktionsabfrage verglichen. Gesonderter Skript-/Datenfluss-
  Review ohne Befund. Initial-/Lookupdefaults bleiben ausdrücklich von einer
  vollständigen transitiven Initializer-/Lifecycle-/Restore-Invariante getrennt.
- Abschlussaudit `root/final-validation.json`: Forschung/Dokumentation **PASS**,
  OE-12/Gate B **BLOCKED**. CMake-/Source-/Test-/Tool-/Resource-SHA256 weiterhin
  `2a0257193d1ba22a5395ad34c78ba27629e45d0f977a9a3e8aae868d8b6669fe`.
  Originalinventar1464 Dateien/819711091 Bytes unverändert, Manifest
  `dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`;
  EXE-/Modell-Bytes, Modus und mtime unverändert. Dokumentationslinks und
  Diff-Whitespace geprüft. Keine erneuten CTests/Produktionsbuilds für die
  reine Forschungsdokumentation; Gate-B-Vollsuites **NOT_RUN**.
- Keine Footprint-/Parser-/World-/Save-/Status-/Sourcebau-/Restore-/Fundsänderung
  oder Quellenfreigabe. Bezahlte Quellenidentität, sichere Bau-/Entfernungskette
  und20-Haus-Finanzierung bleiben getrennte ungelöste Verträge. Kein OE-13.

### OE-12 Fortsetzung – 2026-10-16, Basisff3953c – Markt-Rechteck, weiter BLOCKED

- Beginn am sauberen, live/origin bestätigten `main`
  `ff3953cb3115c05fdbe00d7f5c0001d120eaae2d`; Feature-Branch per Fast-forward
  angeglichen. Vorige Factory-Forschungs-SHA `c3234ae4c397a84aaad533d3ccc63a90d6755d73`
  oben nachgetragen. Strenge Grenze bleibt bestehen, keine Quellen-/Budgetprobe
  bloß wiederholt und kein weiterer Task freigegeben.
- [Markt-Descriptor und Rechteckspread](../reverse/original-market-desirability-shape.md):
  tatsächlicher Typ59-Descriptor liefert für die beiden geprüften Selectorblöcke
  inklusive4×7/7×4-Einfluss-Boundingrechtecke. Position und Selector sind
  zusätzlicher Runtime-/NonHouseInfo-Zustand; dessen Byte+60 wird bei Raw-Info94
  archiviert. Keine physische Belegung, universelle Selector0/1-, Load- oder
  Lifecyclegarantie. Manager-Callbackparameter separat symbolisch nachvollzogen.
- Tatsächlich ausgeführte private Python3-Checkers unter
  `.local/original-economy/oe12/market-shape-research/`, vollständige Befehle
  im verlinkten Beleg: `geometry-audit/check_market_geometry.py` **PASS**,
  13 Fenster/228 Instruktionen/666 Bytes/43 Codeassertions; vier Slots/zwei
  Parameterweitergaben/zwei private Tabellenblöcke. `descriptor-audit/check_market_descriptor.py`
  **PASS**,27 Fenster/653 Instruktionen/2047 Bytes/56 Assertions/elf Slots.
  `spread-audit/check_rectangle_spread.py` **PASS**,8 Fenster/689 Instruktionen/
  2161 Bytes/40 Assertions; zusätzlich acht abgeleitete Wrappertraces und360
  algebraische Perimeterfälle. `root/check_market_call_bridge.py` **PASS**,
  6 Fenster/260 Instruktionen/674 Bytes/34 Assertions. Fenster überlappen;
  keine addierte einzigartige Abdeckung oder Ausführung der Original-EXE.
- `root/run_square_contrast.py`: **PASS**, neue private C++-Probe frisch mit
  unveränderten `Desirability.cpp`, `ModelSchema.cpp`, `ModelData.cpp` kompiliert,
  ohne Compiler-Meldung. Vier synthetische Eingaben auf tatsächlichem Quadratkern
  verglichen mit separat abgeleiteten Rechtecktraces: Phase1 statt0, kein
  Initial-DES0-Return und keine Vorzeichenabsorption im Rechteckwrapper.
  Keine Originalmodellwerte, World oder Platzierung. Der Probe behauptet weder
  originale Rectangle-/Wall-/Clipping-/Overflowparität noch Gameplayabnahme.
- Unabhängige Descriptor-/Geometrie-/Spread- und Skriptreviews abgeschlossen;
  Zellwertsättigung nach jeder Addition und signed Periodenvergleich präzisiert.
  Historischer OE-03-PASS bleibt auf den bisherigen Quadratvertrag begrenzt.
  Abschlussaudit `root/final-validation.json`: Forschung/Dokumentation **PASS**,
  OE-12/Gate B **BLOCKED**. CMake-/Source-/Test-/Tool-/Resource-SHA256 weiterhin
  `2a0257193d1ba22a5395ad34c78ba27629e45d0f977a9a3e8aae868d8b6669fe`;
  Originalinventar1464 Dateien/819711091 Bytes, Manifest weiterhin
  `dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`.
  EXE-/Modell-Bytes, Modus und mtime unverändert; Dokumentationsverweise und
  Diff-Whitespace geprüft. Keine erneuten CTests oder Produktionsbuilds für
  diese Forschungsdokumentation; Gate-B-Vollsuites **NOT_RUN**.
- Keine produktive Rechteckerweiterung, World-Quellenidentität, Parser-/
  Footprint-/Save-/Bau-/Restore-/Fundsänderung. Quellenvertrag und Finanzierung
  bleiben ungelöst; keine OE-13-Freigabe.

### OE-12 Vertragsentwurf – 2026-10-16, Basis4fd5868 – zur Entscheidung

- Beginn am sauberen, live/origin bestätigten `main`
  `4fd586851e1f019dbeb9b9894e7437fd9b6d9e2c`; Feature-Branch per Fast-forward
  angeglichen. Letzter Markt-Forschungscommit `da94833b46c130514d24fa172dacad4a56e46e11`
  oben nachgetragen. Nutzer beauftragt ausdrücklich zuerst den konkreten
  Quellen-, Bau-, Restore- und Finanzierungsvertrag zur Entscheidung.
- [Prüfbarer Entwurf](../original-economy/source-contract-v1-proposal.md):
  genau Well72 als eigener authored2×2-Quellencontainer, Kauf60/max4,
  atomarer Abriss ohne Refund, eigenes bounded Ledger und striktes Restore.
  Empfohlen sind ausdrücklich Rule8/Schema24/policy3 und fixes Budget2000;
  der heutige Rule7-/Schema23-Vertrag bleibt eingefroren. Die Umstellung vom
  beauftragten Rule7-Namen ist eine sichtbare Entscheidung, keine Umsetzung.
- Konkrete24×20-Szene:20 regulär zu kaufende Häuser,40 eindeutige Straßen,
  zwei Quellen, Abriss und Wiederkauf am Rand. Layout-/Ledgerarithmetik ergibt
  Ausgaben1860/Rest140, historische ID21 und neue ID23/nächste24.
  Das wurde als Geometrie-/Rechenprüfung kontrolliert, **nicht** als neue
  Rule8-World ausgeführt. Separater Algorithmusvergleich erforderlich:
  echtes City-v13 verweigert Wells. Der Entwurf benennt diesen zusätzlichen
  Abnahmewechsel ausdrücklich; keine gefälschte City-v13-Instanz.
- Drei unabhängige read-only Designreviews: Objekt-/Capability-Isolation,
  Finance/Schema/Ledger und Layout/Legacyvergleich. Quellenkaufhistorie und
  bestehendes DemolitionHistory strikt getrennt; alle alten Schemas verweigern
  neue Quellentypen. Keine bisherigen Analogien aufgewertet.
- Private Dokumentations-/Layoutprüfung:
  `python3 .local/original-economy/oe12/source-contract-proposal/validate_proposal.py`.
  Abschlussbelege liegen dort in `final-validation.json` und
  `layout-finance.json`; nur Dokumentations- und Rechenprüfung, kein Featuretest.
  CMake-/Source-/Test-/Tool-/Resource-SHA256 bleibt
  `2a0257193d1ba22a5395ad34c78ba27629e45d0f977a9a3e8aae868d8b6669fe`;
  Originalinventar1464 Dateien/819711091 Bytes und Manifest
  `dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`
  bleiben unverändert. Lokale Links, Layout, Funds-/ID-/Revisionsarithmetik
  und Diff-Whitespace geprüft: **PASS** für Layout-/Ledgerarithmetik und79
  lokale Links. Dieses PASS bezeichnet ausschließlich den Entwurf.
- **NICHT FREIGEGEBEN:** Kein Bau-/Restore-/Budget-/Versionscode geändert.
  Keine Produktionsbuilds/CTests; Gate-B-Vollsuites **NOT_RUN**, native/human
  **NOT_RUN**. OE-12/Gate B bleiben **BLOCKED**, OE-13 **TODO**. Der delegierte
  Git-Handoff erlaubt Dokumentationscommit/PR/Merge, keine implizite Annahme
  dieses Vorschlags. Nächster notwendiger Schritt ist die Nutzerentscheidung.


### OE-12 Umsetzung – 2026-10-16, Basis1be1863 – Gate B PASS

- **Auftrag und Ausgangspunkt:** Direktes „weiter gehts“ auf die konkrete
  vierteilige Freigabefrage beauftragt den vorgelegten Quellenvertrag V1.
  Sauberer, live/origin bestätigter `main`
  `1be1863b643233781865158510b8ceb93e7c63e3`; Feature-Branch per Fast-forward
  angeglichen. Vorheriger Entwurfscommit
  `d62da85f2b1b3158f1e46b603d3ab7b10a7d7052` bleibt als Dokumentation erhalten.
  Keine Fremdänderungen am Beginn. Die alten BLOCKED-Einträge oben sind
  historische Ergebnisse vor dieser konkreten Freigabe.
- **Umsetzung:** Expliziter Entwickler-Nachfolger Rule8/Schema24/policy3,
  neuer angehängter Objekttyp und typisierter Well72-Container2×2, Kauf60/max4,
  2000 Startfunds, atomarer Abriss ohne Refund und separates Entfernungsledger.
  Cache/Prefixmaps entstehen nur bei Create/Restore und erfolgreichem
  Quellenkauf/-abriss. Haus-/Inspectorwerte lesen bounded vorbereitete Zellen.
  Originalmodell unveränderlich, bestehender Quadratkern bytegleich; weder
  Wasser/Worker/Goods/Service/Einkommen noch neue House-Evolution freigegeben.
  Schema24 prüft vollständig Modell/Map/Policy, Kosten, IDs und Ledger;
  Schemas1–23 behalten ihre Bytes. Normaler App-/Menü-/Recoverypfad bleibt zu.
  Bedienung und Authority im [Runtimevertrag](../original-economy/desirability-runtime.md).
- **Vollständige Phasen-Gates:** Alle drei Konfigurationen mit
  `cmake -S . -B BUILD -DCMAKE_BUILD_TYPE=TYPE -DOPENEMPEROR_USE_SYSTEM_SDL3=ON -DOPENEMPEROR_ENABLE_SANITIZERS=ON_OR_OFF`
  konfiguriert und vollständig gebaut. Vor dem jeweiligen ersten Lauf reale
  Targets per `ctest --test-dir BUILD -N` ermittelt. Danach jeweils
  `ctest --test-dir BUILD --output-on-failure --parallel 2 --output-junit XML`.
  Debug in `.local/market-walkers-finish/debug-build`: **180/180 PASS**,1687.85s.
  Release in `.local/reswall/release-build`: zunächst176/180 PASS,122.52s;
  vier obsolete Grenzen erwarteten Rule8 bzw. den jetzt angehängten Objekttyp
  noch als unbekannt. Diese Negativfälle prüfen jetzt den wirklich nächsten
  unbekannten Wert9 bzw. `OriginalInfluenceSource+1`; die alten Rule7-
  Verweigerungen bleiben erhalten. ASan+UBSan in
  `.local/market-walkers-finish/sanitizer-build`, RelWithDebInfo:
  **180/180 PASS**,450.04s, keine Sanitizerdiagnose.
- **Finaler Nachlauf:** Nach Testgrenzenkorrektur, Rule8-only Einzel-/Batch-
  Command-Counterguard und Registrierung der Python-Runnerprüfung alle
  betroffenen Targets in allen drei Konfigurationen erneut gebaut. Jeweils
  **10/10 CTests PASS**, einschließlich aller vier initialen Releasefehler,
  neuer World-/Save-/SDL-/Runnerprüfungen und Forkadaptervertrag. Damit sind
  **181 unterschiedliche CTests je Konfiguration /543 insgesamt bestanden**,
  ohne Skip; dies ist die Vereinigung vollständiger180er-Suite und finaler
  gezielter10er-Auswahl, kein behaupteter neuer vollständiger181er-Lauf.
  Finale gezielte Builds ohne Compiler-/Linkerwarnungen.
  Exakte Befehle/Exitcodes und Ausgaben unter
  `.local/original-economy/oe12/implementation/{debug,release,sanitizer}-gate-result.json`,
  `*-gate-*.log`, `*-ctest.xml`, `*-targeted.json`, `*-targeted.xml`.
- **World-/Save-Negativfälle:** Vierquellenlimit, falscher Typ/Footprint,
  Originalschutz/Belegung/Bounds, unzureichende Funds, kein Refund/keine
  ID-Wiederverwendung, Ledger-/Kostensumme, unbekannte/duplizierte/fehlende IDs,
  Source-/Model-/Map-/Policy-Manipulation und fremde Schemas verweigert.
  Letzter legaler Command-Counterstand bleibt restaurierbar; zu großer
  Road-Batch wird vollständig atomisch verweigert. Idle-/House-/Tickabfragen
  rebuilden den Cache nicht. Unabhängige Reviews von Simulation,
  Persistenz/UI/Launcher und Szenengeometrie/Finanzen ohne blockierenden Befund.
- **Tatsächliche20-Häuser-Szene mit echten Modelldaten:**
  `python3 tools/test_original_influence.py .local/gog-extracted/app --build-dir .local/reswall/release-build`:
  **PASS**, finaler Bericht
  `.local/original-economy/oe12/influence-fixtures/run-6oyqms32/report.json`,
  SHA256 `6eee343437827313e4d1d68598379b88e8d13c75d26bce028378d2601b572cbd`.
  Hashidentische private Modellkopie, ausdrücklich authored24×20-Karte,
  tatsächliche produktive Käufe von40 Straßen,20 Häusern und Quellen A/B.
  Nach Abriss200 Funds/1800 spent; nach Rand-Wiederkauf140/1860, entfernte
  ID21/neue23/nächste24. Hauswerte ändern sich nach jeder Quellenmutation.
  Unveränderter City-v13-Algorithmus mit authored Well72→Well-Zuordnung liefert
  hier für alle20 Häuser0; keine dort illegale World erzeugt.
  Zwei getrennte neue OS-Prozesse simulieren je20k weitere Ticks und vergleichen
  vollständige SaveDocuments, kanonische Bytes und Quellenprovenance exakt.
  Alle19 Ausgabedateien geprüft, sechs Saves bytegleich zum Vorlauf; keine
  Snapshot-Injektion. Acht Python-Outputguardtests bestehen, reales
  `--output` außerhalb des privaten Roots wird ohne Schreiben verweigert.
- **Scripted SDL und tatsächliche Darstellung:** Tastatur `I`, Mauskauf,
  bezahlte Funds, neutrale Wasseranzeige, Haus-/Quelleninspector, F5/F9 mit
  vollständigem Snapshotvergleich, Abriss und30 Idleframes geprüft. Keine
  zusätzlichen Frame-I/O-/Asset-/Worldcopy-/BFS-Zähler. Finalen Capture
  `.local/original-economy/oe12/implementation/influence-view-final.png`
  tatsächlich geöffnet:2×2-Well-Fallback und Inspector mit Wert12,
  Quelle#2/Beitrag0+12→12/Difficulty/Model-SHA sichtbar. Das sind authored
  Software-SDL-Pixel, keine Originalgrafik- oder native/humane Abnahme.
  `sh -n tools/play_original_influence.sh`: **PASS**.
  `cmake --build .local/reswall/release-build --parallel 2 --target openemperor-original-influence-play`:
  **PASS**, finaler ausgeschlossener Launcher ohne Warnungen gebaut.
  Er startet über `sh tools/play_original_influence.sh` auf Xia;
  er wurde nicht als native menschliche Spielrunde ausgegeben.
- **Zusätzlicher Infrastrukturprobe:** Private
  `implementation/InfrastructureProbe.cpp` gegen finale Produktionsarchive
  frisch kompiliert und ausgeführt: **PASS**, keine Compilerwarnung.
  Tatsächliche bezahlte13 Straßen/Bridge60/Ramp40/House80/Source60,
  Quellen-Wasserüberlappung atomisch verweigert, Rampenendpoint getrennt und
  gegen2 Funds repariert. Finale Funds1732/spent268; gemeinsame Traversierung
  und kompletter Schema24-Restore inklusive Bridge/Ramp/Source exakt.
  Synthetische Karte/Modell, kein Originalmap-Gameplaybeweis.
- **Forkvergleich:** Finale Debug-/Release-/Sanitizerberichte jeweils
  **74/74 PASS**;46 parity/18 corrected_bug/6 unverified/4 unsupported.
  Alle18 Housingbeobachtungen und1800 echten Modellwerte im Digest exakt.
  `implementation/parity-validation-summary.json` bindet aktuelle Library-
  Hashes; portable Berichte stimmen mit `docs/original-economy/core-parity.json`
  überein. Bestehende Fallklassifikationen/Forkpins/Numerik unverändert;
  nur ABI-/DTO-Quellhashes ergänzt bzw. aktualisiert.28 Python-Werkzeugtests PASS.
- **Vollständiger finaler Legacyvergleich:**
  `python3 tools/original_economy_baseline.py verify --data .local/gog-extracted/app --build .local/reswall/release-build --reference .local/original-economy/oe00/reference/contract.json --output .local/original-economy/oe00/oe12-final-verification`:
  **PASS**. Contract und alle72 vollständigen SaveDocuments sowie72
  WorldSnapshots wörtlich bytegleich zur eingefrorenen OE-00-Referenz.
  1485 Policyfälle,167 komplette Maps/165 vorbereitet/2 bestehende Konflikte,
  elf Rampen-/zehn Brücken-Prozessrestartpaare und tatsächliche bezahlte
  Rule3-/Rule4-Xia-/Handan-Kontrollen bestanden. Vertragsdigest weiterhin
  `12823612834d27099189c18e31ed4fed4761c9ff34486227b3075cf1069a49e5`;
  finales `run.json` SHA256
  `1fffcfdd3386d4563f72cf775c1cd43e8039600bceb7573395d4b77d4ade24c2`.
- **Abschluss und Grenze:** Originalinventar1464 Dateien/819711091 Bytes
  unverändert, Byte-Manifest
  `dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`;
  Modellbytes/Modus/mtime unverändert. Finaler Source-/Test-/Tool-/Resourcehash
  `5418b481dd970ff55450f392ff91ac1f772e1e4dda3fd8c6400c3c895af95be5`.
  Private Abschlussprüfung `implementation/final-validation.json` kontrolliert
  eigene38 Pfade, Links/Anker, Diff, Testvereinigung, finale Archive/Fixtures,
  Originalinventar und Legacyvertrag. OE-12 und Gate B **PASS** für den
  angenommenen begrenzten Entwicklervertrag; alle sieben alten Analogien
  weiter Inferred/Unknown, keine Rechteck-/Original-Lifecycle-Gesamtparität.
  Native/human **NOT_RUN**. Eigener Task-Commit und delegierter PR/regulärer
  Merge folgen nach diesem Nachweis; SHA in Abschlussantwort. Nächster Task
  **OE-13 TODO**, in diesem Task nicht begonnen.


### OE-13 Vertragsprüfung – 2026-10-16, Basisbf4e821 – BLOCKED

- **Ausgangspunkt:** Sauberer aktueller `main`
  `bf4e821a591f141b0a300c848b4ecbdbe3e40fe8`, per frischem `git fetch origin`
  bestätigt; Feature-Branch ohne Verwerfen/Reset per Fast-forward angeglichen.
  Vorige OE-12-SHA `ec3a2d60c7541f8a11338c80d5398ce54e51f3a8` nachgetragen.
  Nutzer beauftragt Fortsetzung mit OE-13, keine gesonderte Aufhebung der
  angenommenen Rule7-/Schema23- oder Rule8-/Schema24-Grenze.
- **Konkrete Kollision:** Der bisherige OE-13-Prompt verlangt ausdrücklich
  Rule7. Dauerhafte Gates und angenommener Quellenvertrag frieren diesen
  Vertrag ein; Rule8 erlaubt ebenfalls keine Waren, Worker oder Couriers.
  Beide originalen Worldvalidatoren verlangen Vacant-Nullzustände/leere
  Couriers, die Writer/Parser binden exakt Schema23↔Rule7 bzw.24↔Rule8.
  Rule9/Schema25 sind aktuell frei, werden durch diese Prüfung nicht aktiviert.
  Voller tatsächlicher Ceramics-Ursprung, Market-Fahrt und schedulergebundener
  Verbrauch sind erst OE-14/15/17 zugewiesen. Zudem fehlen für reale Produktion
  und House-Ankunft Bewohner/Workforce bzw. ein zulässiger Empfängerzustand.
- **Prüfbarer Vorschlag:**
  [Warenbuchhaltungsvertrag V1](../original-economy/goods-ledger-v1-proposal.md)
  benennt einen getrennten Rule9-/Schema25-Nachfolger und die zusätzliche
  gestufte Abnahmeentscheidung. Positive Transaktionsarithmetik ausschließlich
  synthetisch; echte World/Savezustände zunächst0/unsupported, keine
  Material-, Actor- oder Bewohnerinjektion. Alle alten Regeln/Schemas1–24
  bleiben exakt. Noch **NICHT ANGENOMMEN / NICHT IMPLEMENTIERT**; der Vorschlag
  ist keine automatische Freigabe der Produktion oder der Folgetasks.
- **Neue Probe gegen unveränderte Produktionsarchive:** Private
  `.local/original-economy/oe13/contract/BoundaryProbe.cpp` mit C++20,
  `-Wall -Wextra -Wpedantic -Wconversion -Wshadow` und den aktuellen
  Release-Simulation-/Persistence-/Original-Goods-Libraries kompiliert und
  tatsächlich ausgeführt: **PASS**, keine Compilerwarnung.
  Exakte argv/Exitcodes und Bericht-SHA in `probe-run.json`, Compilerlog
  `probe-build.log`, Resultat `boundary-report.json`. Ausführbar reproduzierbar:
  `.local/original-economy/oe13/contract/boundary-probe .local/original-economy/oe13/contract/fixture`.
  Ausschließlich authored Modell/24×20-Geographie, keine neue Originalmechanik.
- **Tatsächliches Proberesultat:** Rule7 kauft ein Haus, Rest1220; Rule8 kauft
  Haus+Well72, Rest1860. ClaySource/Pottery/Warehouse/Market werden in beiden
  Worlds jeweils mit echten Befehlen atomisch verweigert; vollständige
  Snapshots bleiben identisch. Alle sechs Hausbestände0, Couriers0,
  Bevölkerung/Workforce0, tatsächliches `accepts_goods_delivery` false.
  Manipuliertes Ceramics1 sowie Produktionszähler1 scheitern am vorhandenen
  Validator. Je20k reale Ticks fortgeschaltet: Wirtschafts-/Hauszustand und
  Funds unverändert. Diese Grenzprüfung bestätigt keine Warenbewegung oder
  neuen Prozess-Roundtrip. Rule9 verweigert weiterhin „unsupported sandbox
  rule version“.
- **Bestehende Grenztests:** Vor erstem CTest aktuelle reale Registrierung
  mit `ctest --test-dir .local/reswall/release-build -N` festgehalten.
  Danach
  `ctest --test-dir .local/reswall/release-build --output-on-failure -R '^(original-economy-rule-gate|original-influence-world|original-influence-save)$' --output-junit .local/original-economy/oe13/contract/boundary-ctest.xml`:
  **3/3 PASS**,0.65s, ohne Skip; stdout in `boundary-ctest.log`.
  Dies prüft die bestehenden Grenzen und ist kein OE-13-Featuretest.
  Keine erneuten vollständigen Debug-/Release-/Sanitizer-Suiten oder
  Corpus-Replays für unveränderten Produktionscode behauptet.
- **Review und Abschluss:** Simulation/Authority, Persistenz und
  Transaktions-/Abnahmeplanung unabhängig untersucht. Conservation zählt
  Reservations nicht zusätzlich als Masse; HouseState bleibt einzige
  Hausmengenquelle. Selbstkonsistente positive Ledgerwerte ohne zugelassenen
  realen Ursprung bleiben verboten. Private `contract/final-validation.json`
  prüft eigene drei Dokumentationspfade, sämtliche Links/Anker, Diff,
  Source-/Test-/Tool-/Resourcehash und Originalinventar.
  SourceSHA weiterhin
  `5418b481dd970ff55450f392ff91ac1f772e1e4dda3fd8c6400c3c895af95be5`;
  Originalbestand1464 Dateien/819711091 Bytes, Manifest
  `dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`.
  Keine Produktions-, Save-, Prompt- oder Quality-Gate-Datei geändert.
  **OE-13 BLOCKED**, Gate B **PASS**, OE-14 **TODO**; keine Native-/Human-
  oder Warenkreislaufabnahme. Geprüfter Dokumentationscommit/PR/regulärer Merge
  nach bestehendem Nutzer-Handoff; SHA in Abschlussantwort.

### OE-13 Umsetzung – 2026-10-16, Basisaf660b72

- **Auftrag und Start:** Nutzer nimmt den geprüften Rule9-/Schema25-Vertrag
  ausdrücklich an. Sauberer, frisch live/origin bestätigter `main`
  `af660b72c281e5c1b8735a29a941f474a3f33d8d`; Feature per Fast-forward
  angeglichen. Vor dem ersten Neubuild separat eingefrorene Rule7-/Schema23-
  und Rule8-/Schema24-Belege ergänzen die ältere OE-00-Referenz. Die29
  eingefrorenen Dateien bleiben unverändert; Capture-SHA
  `d6b5f91d767a632aba8e95600f8cbe9660f36d45e5ccf3d29502be54cd655d1e`.
- **Implementierter Umfang:** [Runtimevertrag](../original-economy/goods-ledger-runtime.md)
  mit eigenem expliziten Entwickleradapter, typisierten sechs Originalwaren,
  reinem atomarem Transaktionskern, lesenden World-Bilanzen und strengem
  Schema25. HouseState bleibt einzige Hausmengenauthority; Reservations sind
  keine zusätzliche Masse. Reale Worldkonten bleiben0/unsupported, Listen
  leer; kein Producer, Import, Worker, Bewohner, Verbrauch oder Waren-Courier
  freigegeben. Alte Rule1–8/Schema1–24 und numerische Enums bleiben erhalten.
- **Debug, Release, ASan+UBSan:** Jeweils frisch konfiguriert und vor erstem
  CTest alle186 registrierten Tests mit `ctest --test-dir BUILD -N` erfasst.
  Danach27 relevante Targets gebaut, einschließlich normaler Anwendung und
  bisheriger Save-/SDL-/Bridge-/Ramp-Pfade. Finale Auswahl aus
  `.local/original-economy/oe13/implementation/test-plan.json`:
  **je39/39 PASS**, insgesamt117 Konfigurations-CTests ohne Skip.
  Testlaufzeiten Debug82.499s, Release8.077s, Sanitizer28.094s.
  Exakte Configure-/Build-/CTestbefehle, Exitcodes und Zeiten stehen in
  `implementation/{debug,release,sanitizer}-targeted-result.json`, vollständige
  Logs/JUnit in gleichnamigen `*-targeted-*` beziehungsweise `*-targeted.xml`.
  Keine Compilerwarnungen oder Sanitizerdiagnosen. Dies ist die relevante
  OE-13-Auswahl, keine Behauptung einer vollständigen186er-Suite oder Gate C.
- **Positive reine Transaktionen:** Alle sechs Waren; Erzeugung/Import,
  Quell- und Zielreservierung ohne zusätzliche Masse, einmalige Teilbeladung,
  Teilankünfte, Abbruch vor/nach Beladung und bestätigte Teilrückgabe.
  Volle Rückquelle verweigert atomisch; alte/doppelte Transferhandles und
  Sequenzen, fremde Owner/Courier, volle/fehlende Stocks, fehlerhafte andere
  Waren und int16-/int32-/uint64-Grenzen geprüft. Vorher-/Nachher-Conservation
  über den ganzen Kandidaten; Aufruferzustand bleibt auch bei Erfolg gleich.
  Positive Mengen sind ausschließlich authored numerische Tests.
- **Save- und Quellenangriffe:** Finaler Save-Test verweigert575 beschädigte
  oder nicht autorisierte Fälle, darunter ausgeglichene Warenfälschungen,
  unbekannte/fehlende/doppelte/falsch typisierte Keys und Crossloads.
  Der erste neue Save-Test scheiterte an einer falschen Testannahme:
  Der bestehende Reader erlaubt innerhalb des Datenroots liegende,
  byteidentische Modell-Symlinks. Der Test prüft diese erlaubte Variante nun
  positiv auf vollständigen Snapshot-/Byteerhalt und entweichende/dangling
  Links weiterhin negativ für Writer und Restore. Keine Produktionslockerung.
  Erster Fehler bleibt in `release-world-save-view-first.log` sichtbar;
  korrigierter Einzeltest und alle drei finalen Konfigurationsläufe bestanden.
- **Bezahlte echte World und Prozessfortsetzung:**
  `python3 tools/test_original_goods.py .local/gog-extracted/app --build-dir .local/reswall/release-build`:
  **PASS**, Bericht `oe13/goods-fixtures/run-d4mm76cf/report.json`, SHA
  `1b5f95d6b1de09558be7f4a5859b1fa953aaa79da089f831afece692760678f2`.
  Hashidentische private echte Modellkopie, authored24×20-Karte, regulär40
  Straßen/20 Häuser/Well72 A+B gekauft. Abrissstand Funds200/spent1800,
  Wiederkauf140/1860, alte SourceID21/neue23/nächste24. Keine Snapshot- oder
  Bestandsinjektion; vier tatsächliche Producerbefehle atomisch verweigert.
  Je ein separater OS-Prozess setzt beide Stände mit20k Ticks, einer realen
  bezahlten Straße und weiteren20k Ticks fort. Vollständige SaveDocuments,
  kanonische Bytes, Modellprovenance und sechs Nullbilanzen an Zwischen- und
  Endpunkten exakt. Finale Funds198/138 bei41 Straßen; Revisionen jeweils+1.
  Alle47 Ausgabedateien geprüft, zwölf Python-Outputguards bestanden;
  außerhalb des privaten Roots wird ohne Schreiben verweigert.
- **Zusätzliche eingefrorene Verträge:** Rule7-/Schema23-Golden6778 Bytes,
  SHA `4e70a04b19700c86d974d53df4edb2fdcf3bb264622ef76c4c32a1be5b63ca68`,
  und sechs Rule8-/Schema24-Saves wörtlich gleich zum vor OE-13 gesicherten
  Stand; Originalquellen und Eingabesaves unverändert. Gleiche bezahlte Szene
  unter9 und8 zusätzlich semantisch verglichen, nach ausdrücklichem Entfernen
  allein der Version und des Nullwaren-DTO; keine Crossversions-Bytegleichheit
  behauptet.
- **Geerbte Infrastruktur in Rule9:** Zusätzlicher World-Test kauft13 Straßen,
  Bridge60, Ramp40, House80 und Source60; reale Trennung/Reparatur von Rampen-
  und Brückenbankstraße je2 Funds, finale1730/spent270. Exakte signed Höhen,
  gemeinsame Traversierungsregeln, erhaltene IDs, atomische Schutz-/Wasser-
  Verweigerungen und kompletter Schema25-Byte-/SaveDocument-Restore bestanden.
  Authored Karte/Modell und ausschließlich Nullwaren; alle drei Konfigurationen.
- **Tatsächliche SDL-Handler und geöffnete Darstellung:** `I` und Mauskauf,
  bezahltes Haus/Quelle, F5/F9, volle Snapshotgleichheit, Modell-/Cacheerhalt
  bei abgewiesenem manipuliertem F9, beide8↔9-Crossloads und geschlossener
  Normal-/Recoveryzugang bestanden.30 Idleframes und Bilanzqueries ohne
  zusätzliche Dateien, Decode/Upload, Worldkopien, Commands oder BFS.
  Finales `implementation/goods-view-final.png` tatsächlich geöffnet:
  Rule9/Policy3/Save25, Funds1860, Desirability12/Quelle#2/Beitrag0+12→12 und
  Modell-SHA sichtbar, Producerbuttons deaktiviert. Authored Software-SDL,
  keine Originalgrafik- oder native/humane Spielabnahme. Neuer ausgeschlossener
  Launcher gebaut; `sh -n tools/play_original_goods.sh` PASS. Startbefehl
  `sh tools/play_original_goods.sh`, eigener Rule9-Savepfad; Rule8 unverändert.
- **Forkvergleich:** Finale Debug-/Release-/Sanitizerberichte jeweils
  **74/74 PASS**,46 parity/18 corrected_bug/6 unverified/4 unsupported.
  Alle18 Housingbeobachtungen und1800 Modellwerte im gebundenen Digest exakt.
  Sieben tatsächlich verlinkte Archive je Konfiguration live nachgehasht;
  alle öffentlichen ABI-Blobs geprüft. Numerische Originalmodule und ältere
  nichtaktuelle Berichte unverändert. Portable aktuelle Berichte stimmen mit
  `core-parity.json` überein, SHA
  `caa72a45508c54d5ed44a008b19b05dde83087af08fb59b51c28b6c5f8e2de85`.
  Der erste28er-Pythonlauf verweigerte korrekt veraltete aktuelle Quellhashes;
  nach dem tatsächlich ausgeführten74-Fall-Vergleich wurden ausschließlich
  dessen aktuelle Fingerprints eingetragen. Finale28/28 Pythonprüfungen PASS,
  historische Fallwerte/Klassifikationen nicht geändert. Strenger zusätzlicher
  Sanitizervergleich ohne stderr; `implementation/parity-validation-summary.json`.
- **Vollständiger finaler Legacyvergleich:**
  `python3 tools/original_economy_baseline.py verify --data .local/gog-extracted/app --build .local/reswall/release-build --reference .local/original-economy/oe00/reference/contract.json --output .local/original-economy/oe00/oe13-final-verification`:
  **PASS**. Contract und alle72 vollständigen SaveDocuments sowie72
  WorldSnapshots zusätzlich wörtlich bytegleich zur eingefrorenen OE-00-
  Referenz nachverglichen.1485 Policyfälle,167 komplette Maps/165 vorbereitet/
  zwei bestehende Konflikte, elf Rampen-/zehn Brücken-Restartpaare und
  tatsächliche bezahlte Rule3-/Rule4-Xia-/Handan-Kontrollen bestanden.
  Vertragsdigest unverändert
  `12823612834d27099189c18e31ed4fed4761c9ff34486227b3075cf1069a49e5`;
  `run.json` SHA
  `65c2901975a80fc7eaa26f7210ce02c44acbbfb47c0e40f6ca6426009ccae2ce`.
  `implementation/legacy-literal-proof.json` nennt alle72 Paare und ihre
  tatsächlich geprüften Byte-/Snapshot-SHAs, Bericht-SHA
  `f350f8a5f09e32949fdf51623279331adb357f30baccfc2a17cc15a3f48ee5bf`.
- **Abschluss und Aussagegrenze:** Originalinventar1464 Dateien/819711091
  Bytes unverändert, Byte-Manifest
  `dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`.
  Finaler Source-/Test-/Tool-/Resourcehash
  `532129727902f7d36a251ad6eafc25fcb62f6cdb84a4ce5250db0eed6e3490b4`.
  Unabhängige Ledger-/Authority-/Persistenz-/Replayreviews ohne blockierenden
  Befund. Private `implementation/final-validation.json` prüft36 eigene
  Änderungspfade, Sourcefreeze, Links/Anker, finale Tests/Archive/Fixtures,
  eingefrorene spätere Saves, Originalinventar und vollständigen Legacyvertrag.
  **OE-13 Buchhaltungsfundament technisch PASS**, Gate B weiterhin PASS,
  Gate C TODO. Keine positive reale Warenproduktion/-lieferung/-konsum-
  oder Originalspiel-Gesamtparität. Native/human **NOT_RUN**. Task-Commit
  und delegierter PR/regulärer Merge folgen nach Abschlussprüfung, SHA in
  Antwort. Nächster vorgesehener Task **OE-14 TODO**, hier nicht begonnen.

### OE-14 Umsetzung – 2026-10-17, Basis41091b66

- **Auftrag und Beginn:** Nutzer beauftragt alle offenen Aufgaben bis OE-33
  der Reihe nach ohne Wartepunkt für seine manuellen Tests; echte Sprites und
  Objekte sind Teil des Abschlussziels. Sauberer aktueller `main`
  `41091b66e960b4c82c378d5ae3b83088325f5a61`, Feature per Fast-forward
  angeglichen. Vorige OE-13-SHA oben nachgetragen. Neuer expliziter
  [Ausführungsvertrag](../original-economy/completion-contract.md): Rule10/
  Schema26/policy3, strikte Entwicklungsrevision14, authored Startfunds12000,
  bezahlte Zuwanderung/Worker und einmalige Warehouse-Konvertierung;
  veröffentlichte1–9/1–25-Verträge bleiben unverändert.
- **Echte produktive Integration:**
  [Runtimebeschreibung](../original-economy/production-runtime.md), regulär
  gekaufte Straßen, vier leere Häuser, Clay Pit, Kiln und Warehouse. Einwohner
  entstehen erst aus120t/40t-Zuzugsereignissen an realen Straßeneingängen,
  bis zur vorbereiteten Shelterkapazität. Sie allein liefern Worker.
  Gemeinsame unveränderte Clay-/Pottery-Rezepte und bestehende Courier-
  Navigation/Bewegung; erst tatsächliche Warehouse-Ankunft erzeugt über den
  reinen Ledgerkern Ceramics. Legacy-Warehouse-Pottery bleibt0. Clay,
  unkonvertierte Pottery und typisierte Ceramics sind getrennt massenerhalten.
  Kein Market, Hausvorrat, Verbrauch, Food, Service oder Tax vorgezogen.
- **Debug, Release, ASan+UBSan:** Konfiguration und aktuelle Registrierung
  vor dem ersten CTest erfasst; final190 registrierte Tests.34 relevante
  Targets einschließlich normaler Anwendung, alter/neuer Save-/SDL-Pfade,
  Bridge-/Ramp-Renderer/Transport und ausgeschlossener Launcher/Prozessfixtures
  gebaut. Finale43er-Auswahl **je43/43 PASS**, insgesamt129 relevante
  Konfigurations-CTests ohne Skip. Laufzeiten Debug56.000s, Release6.542s,
  Sanitizer22.277s. Exakte Befehle/Exitcodes/Logs/JUnit in
  `.local/original-economy/oe14/implementation/{debug,release,sanitizer}-targeted-result.json`,
  gleichnamigen `*-targeted-*`-Logs und `*-targeted.xml`.
  Keine vorgezogene vollständige190er-Phasen-Gate-Suite behauptet.
- **Betriebsausfälle und Roundtrips:** Keine Bewohner vor dem ersten
  Zuzugsereignis; fehlende Häuser/Straße verhindern Worker/Produktion.
  Pause/Resume, reale Straßentrennung/bezahlt reparierte Verbindung und
  natürlich ausgelöste Fire-Vorfälle geprüft. Mid-Pottery-Courier wird als
  tatsächlicher bewegter Zustand restauriert, vollständige WorldSnapshots
  laufen danach exakt weiter. House-, Produktions-, Finanz- und Aktor-
  Grants/Orphans, falsche Entwicklungsrevision und doppelte Pottery-Buchung
  werden verweigert.
- **Strict Save26:** Eigener kanonischer Reader/Writer/Restore, nur lokale
  Syntaxprojektion auf unveränderte Infrastrukturfelder, keine Veröffentlichung
  einer heruntergestuften World. Modell-/Map-/Masken-/policy3-Hashbindung,
  tatsächliche Rezept-/Übergabehistorie, IDs, Population, Routes, Cargo,
  Reservierungen und Kosten werden vollständig geprüft. Neue Save-Suite
  verweigert795 adversariale Fälle; frühere575 Rule9-Verweigerungen bestehen
  weiter. Rekursiv unbekannte/fehlende Keys, Duplikate, koordinierte positive
  Stock-/Handoff-Fälschungen, Symlinks/Outputpfade und atomare Schreibfehler
  sind enthalten.
- **Unabhängiger Review und Korrektur:** Reviewer fand einen selbstkonsistenten,
  aber nicht erreichbaren Restore mit Bewohner4→3 plus passendem Historiendelta.
  Die neue Populationvalidierung verlangt Zweierschritte oder die tatsächliche
  ungerade Endkapazität. Regressionstest gegen Tick160 und positive vollständige
  Restorekontrolle bei Shelterkapazität7 ergänzt. Erstes neues Gate-CTest
  erwartete10 noch als unbekannt; Grenze explizit auf11 verschoben und die
  positive10-Matrix, exakte Entwicklerfactory-/policy3- und Normalapp-Verweigerung
  ergänzt. Keine alte Autorität freigeschaltet oder Negativprüfung entfernt.
- **Reales Modell und separate OS-Prozesse:**
  `python3 tools/test_original_production.py .local/gog-extracted/app --build-dir .local/market-walkers-finish/debug-build`:
  **PASS**, Bericht `oe14/production-fixtures/run-1j37xg9h/report.json`, SHA
  `b4d16abe0ebcaaf034568e5154fa46dea0a84c1c94f9b8b09f5d12690188354b`.
  Hashidentische private echte Modellkopie, authored24×20-Geographie,
  reale818 Funds Bauausgaben/11182 Rest.400 Ticks gestoppter Clay Pit:
  vier Häuser mit28 regulär zugezogenen Einwohnern, null Clay/Pottery.
  Erster tatsächlicher Warehouse-Übergang bei Tick675:8 Clay extrahiert,
  ein Potteryrezept abgeschlossen, ein Ceramics gebucht, zwölf Worker genutzt.
  Mid-Pottery- und positiver Warehouse-Stand werden je in einem separaten
  OS-Prozess mit10000 Ticks, einem bezahlten Straßenbau und weiteren10000 Ticks
  fortgesetzt. Vollständige SaveDocuments/Bytes/Provenance/Bilanzen exakt;
  Originalinputs, private Modellkopie und Eingabesaves samt Metadaten unverändert.
  13 Python-Outputguardtests und vier tatsächliche Python-/C++-CLI-Escape-
  Verweigerungen PASS. Keine Snapshot-, Vorrats- oder Bewohnerinjektion.
- **Eingefrorene Vorgänger:** Aktuelle Writer-/Prozessfixtures erhalten exakt
  Rule7/Schema23, sechs eingefrorene Rule8/Schema24-Saves und zehn Rule9/Schema25-
  Saves samt Provenance aus OE-13. Der separate OE-00-Vertrag allein deckt diese
  späteren Versionen nicht ab. Die alte Originalinstallation bleibt read-only.
- **Tatsächliche SDL-Handler:** Neue explizite10-Factory, reguläre bezahlte
  Bauten/Zuwanderung/Produktion, bewegtes Cargo, echte F5/F9-Schema26-Handler,
  atomare Verweigerung falscher Revision und100 Idleframes ohne Worldkopien,
  Commands, BFS, Datei-/Decode-/Uploadarbeit bestanden. Finalen Capture
  `implementation/production-view-final.png` tatsächlich geöffnet: Rule10/
  Policy3/Save26/Revision14, Tick1091, Funds11196 und vier authored Testhäuser
  mit je37 tatsächlichen Bewohnern; diese Modellkapazität ist synthetisch.
  Der separate echte Modellrun oben hat dagegen28 Bewohner. Kein Originalart-
  oder native/human-PASS. Neuer ausgeschlossener Launcher gebaut; Start
  `sh tools/play_original_economy.sh`, eigener Rule10-Savepfad.
- **Forkvergleich:** Alle drei tatsächlich ausgeführten Vergleiche
  **74/74 PASS**,46 parity/18 corrected_bug/6 unverified/4 unsupported;
  sämtliche18 Housingbeobachtungen und1800 realen Modellwerte im Digest exakt.
  21 live Archive und fünf ABI-Header geprüft; alte numerische Originaldateien
  und historische Nicht-current-Felder unverändert. Portable aktuelle
  `core-parity.json` SHA
  `940cb0d80e272e4065e9d4b9c6df85a264c9a369126613dd4e2f96d52cfe8732`.
  28 Python-Vertragsprüfungen sowie25 strenge Sanitizerprozesse mit vollständig
  leerem stderr PASS; `implementation/parity-validation-summary.json`.
- **Vollständiger Legacyvergleich:**
  `python3 tools/original_economy_baseline.py verify --data .local/gog-extracted/app --build .local/reswall/release-build --reference .local/original-economy/oe00/reference/contract.json --output .local/original-economy/oe00/oe14-final-verification`:
  **PASS**, zusätzlich alle72 vollständigen Saves und72 Snapshotdateien sowie
  Contract wörtlich bytegleich zur OE-00-Referenz nachverglichen.1485 Policyfälle,
  167 komplette Maps/165 vorbereitet/zwei bestehende Konflikte, elf Rampen-
  und zehn Bridge-Restartpaare exakt. Vertragsdigest weiterhin
  `12823612834d27099189c18e31ed4fed4761c9ff34486227b3075cf1069a49e5`;
  `run.json` SHA
  `4e332d473c474ded7a5b32e4db580f3be1f24b8ebba26e899d37282adcd90097`;
  `implementation/legacy-literal-proof.json` SHA
  `3d31e7008a301f5ae3e989972acc260d02ad060c79635c609d120e39c57a18d2`.
- **Finale aktuelle Binarybindung:** Nach den vollständigen Builds waren die
  Fixtures neu gelinkt. Deshalb wurde der tatsächliche Prozessnachweis mit
  den finalen Releasebinaries vollständig erneut ausgeführt, ohne Berichthashes
  lediglich umzuschreiben:
  `python3 tools/test_original_production.py .local/gog-extracted/app --build-dir .local/reswall/release-build --output .local/original-economy/oe14/production-fixtures/final-source-6acc`:
  **PASS**, Report-SHA
  `745881cb0eb3a8090c5902b5ae9477c0d0fdab3d3e95a96a5f8f03d438863842`,
  aktuelles Fixturebinary
  `3fab6095ecbc40c3e5f89e350035bb582d1caf9aa0608a2a22eaf2b3dad0ffd4`.
  Auch die21 Libraryhashes passen nach den Builds unverändert zu den drei
  echten Forkvergleichen; separater `parity-post-fullbuild-archive-check.json`.
- **Abschluss:** Finaler Source-/Test-/Tool-/Resourcehash
  `6acc2db2c5552a246659e78cf75023cbf1e0dba07a70885c4755d53a77da8640`;
  Originalinventar1464 Dateien/819711091 Bytes, unverändertes Contentmanifest
  `dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`.
  Private `implementation/final-validation.json` prüft33 eigene Pfade,
  23 eingefrorene Sourcepfade, tatsächliche Tests/Archive/Binaries/Saves,
  alle lokalen Dokumentationslinks und scoped Status. **OE-14 PASS**;
  Gate C TODO und native/human NOT_RUN. Ein eigener getesteter Task-Commit;
  danach unmittelbar OE-15 gemäß Gesamtauftrag, kein Wartepunkt für Playtests.

### OE-15 – Umsetzung, 2026-10-17, Basis2bd6878

- **Vertrag und Authority:** Rule10/Schema26/Revision15, unveränderte policy3.
  Bezahlt gekaufte Warehouses und Markets transportieren Ceramics über dieselben
  bestehenden Routen/Actorrollen bis in den alleinigen HouseState-Bestand.
  Typisiertes Ledger-Cargo wird nicht in die alten Courier-Mengen gespiegelt.
  Anzeige, geladene Walker und Diagnose lesen die tatsächliche Menge. Frühe
  originale Modellbedarfe bleiben0; der physische Hauspuffer8 ist ausdrücklich
  authored Provisionierung. Kein Verbrauch vor OE-17, keine Originalartbehauptung.
- **Sicherheit:** Physische Rückkehrkapazität bleibt am Quellowner reserviert;
  Stock/Cargo/Incoming können keine Überfüllung oder Rückgabesperre erzeugen.
  Ownerpause erhält begonnene Kanten, führt beladen über den vorhandenen
  Navigator zurück und bucht Ware erst am Owner. Reguläres Disconnect/Repair,
  volle Puffer, sicher verweigerter Abriss, echte Teilbeladung und tatsächliche
  vollständige Ankünfte geprüft. Es wurde keine unerreichbare Teilankunft durch
  Kapazitäts-/Snapshot-Injektion hergestellt. Letzte Transferzeile je Actor als
  Sequenz-Highwater, höchstens sechs Zeilen auch nach mehr als26 echten Fahrten;
  stale/doppelte Ankünfte verweigert. Canonicaldiagnose enthält die vollständigen
  typisierten Cargo-/Reservationfelder nur unter Rule10; alte Strings bleiben exakt.
- **Unabhängiger Review:** Drei reine-Ledger-legitime, aber im atomaren Worldpfad
  unerreichbare Restorezustände geschlossen: reserved≠loaded, Teilgutschrift vor
  Ankunft und entfernte Terminalhistorie eines bereits verwendeten Couriers.
  Koordinierte Gegenfälle erhalten Conservation und werden trotzdem verweigert.
  Reiner Transaktionskern bleibt unverändert und erlaubt seine geprüften
  Teiloperationen weiterhin. Finale Saveprüfungen:676 Marktverweigerungen,
  798 Produktionsverweigerungen,575 eingefrorene Rule9-Verweigerungen.
- **Tatsächliche Builds/Tests:**
  `python3 .local/original-economy/oe15/implementation/run_targeted.py <debug|release|sanitizer> --build-only`
  und anschließend jeweils `--test-only`: **je45/45 PASS,135 insgesamt,0 Skip**.
  Debug84.984s, Release final6.706s, ASan+UBSan31.831s; genaue Zeiten/Befehle,
  tatsächliche192er-Testinventare,36 Targets und JUnit unter `implementation/`.
  Release wurde wegen einer Koordinationsüberschneidung zweimal erfolgreich
  ausgeführt; kein zusätzlicher Featurebeleg daraus abgeleitet. Ein anfänglicher
  UI-Compilefehler durch verschiedene Courier-Diagnose-Datentypen wurde über
  denselben generischen lesenden Helfer behoben, danach alle Builds erfolgreich.
- **Echtes Modell und OS-Prozesse:**
  `python3 tools/test_original_production.py .local/gog-extracted/app --build-dir .local/market-walkers-finish/debug-build --output .local/original-economy/oe15/production-fixtures/final-source-5f09`:
  **PASS**.958 Funds tatsächlich ausgegeben,11042 Rest,28 echte modellgebundene
  Bewohner/Worker,16 zugewiesen. Bei706t Market-Courier mit einer typisierten
  Einheit und null Hausstock; bei815t erste tatsächliche House-Ceramics-Ankunft.
  Separater beladener Pause-/Returnstand ohne vorzeitige Hausgutschrift. Zwei
  separate OS-Prozesse setzen jeweils10000 Ticks, einen bezahlten Straßenbau
  und weitere10000 Ticks exakt fort: volle SaveDocuments/Bytes/Provenance und
  unabhängig summierte House/WH/Market/Cargo-Bilanzen gleich. Report-SHA
  `0d72330786f6ba885384a7918107d4462f8f40e4cfdf2511f6ffb1b5c8b9907f`;
  aktuelles Debugfixture `f225a31fb2ed2befc90cc275acab07bcdb424820ed08a07e5b33d47c694b8da8`.
  Originalinstallation/private Modellkopie/Eingabesaves unverändert;72 private
  Outputhashes geprüft.13 Pythonverträge und sechs tatsächliche CLI-Outputguards
  inklusive alter OE-14-Root-Verweigerung PASS. Alte Rule7-, sechs Rule8- und zehn
  Rule9-Saves unabhängig bytegleich; diese späteren Versionen ergänzen OE-00.
- **SDL/UI:** Market über Taste0 und echten Mausklick bezahlt, echter typisierter
  Mid-courier-F5/F9 und tatsächliche Hausankunft, danach100 Idleframes ohne World-
  kopien, Commands, BFS, Dateien, Decode oder Upload bestanden. Aufnahme
  `implementation/production-view-final.png` tatsächlich geöffnet: Revision15,
  Tick1181/Funds11056, Markt1, House-Ceramics4/8. Vier authored Testhäuser mit
  je37 Einwohnern sind synthetische Kapazität; echter Modellrun oben hat28.
  Native/human NOT_RUN; echte Objekt-/Spritekomposition folgt ihrer Aufgabe.
- **Forkvergleich:** Drei tatsächliche74/74-Vergleiche PASS, unverändert46 parity/
  18 corrected_bug/6 unverified/4 unsupported.21 aktuelle Archive, fünf ABI-Header,
  sämtliche alten numerischen Module/Beobachtungen,18 Housingbeobachtungen und
  1800 Modellwerte geprüft.25 zusätzliche Sanitizerprozesse stderrleer,28 Python-
  Verträge PASS. Portable `core-parity.json` SHA
  `df7d817f0dd2c4d44f3cc5032c43674ef7faf419eb3d55458a1a348ba5dd8f9f`;
  `implementation/parity-validation-summary.json` SHA
  `aa873e4275213b58a9a327a4022f0200706a88cd0128751404f9f213054d65cb`.
- **Vollständiger Legacyabschluss:**
  `python3 tools/original_economy_baseline.py verify --data .local/gog-extracted/app --build .local/reswall/release-build --reference .local/original-economy/oe00/reference/contract.json --output .local/original-economy/oe00/oe15-final-verification`:
  **PASS**. Alle72 vollständigen Saves und72 Snapshots sowie der Gesamtvertrag
  anschließend unabhängig wörtlich bytegleich nachgeprüft.167 komplette Maps,
  165 vorbereitet/zwei bestehende Konflikte,1485 Policyfälle, elf Rampen- und
  zehn Bridge-Restartpaare unverändert. Vertragsdigest weiterhin
  `12823612834d27099189c18e31ed4fed4761c9ff34486227b3075cf1069a49e5`;
  run.json SHA `4f26db164ad216da59ee1b5314c345153b395072a367de6481ce5740d409247d`;
  Literalproof SHA `350aaa0b4c73aed88217a4db978373410a6af32a43d6f7127a8c9f546a29ff03`.
- **Abschluss:** Eingefrorener Source-/Test-/Tool-/Resourcehash
  `5f09b3fd42f4cbb0fa9fd2d2abbbde026ef8b0dd49a46b0222ed7e7c5bb9b658`;
  19 eigene Source- und24 gesamte Taskpfade. Originalinventar1464 Dateien/
  819711091 Bytes samt Metadaten unverändert. Unabhängige finale Prüfung
  `implementation/final-validation.json` bindet die tatsächlichen Binaries,
  Archive, JUnitdaten, Savebytes, Warenbilanzen, Quellen und Dokumentationslinks.
  OE-15 PASS, OE-16 folgt direkt; Gate C bis OE-19 TODO, native/human NOT_RUN.

### OE-16 – Umsetzung, 2026-10-17, Basis390c874

- **Vertrag und tatsächliche Authority:**
  [Hemp-Laufzeitvertrag](../original-economy/hemp-runtime.md), explizite
  Rule10/Schema26/policy3 mit Entwicklungsrevision16. Separat bezahlte Hemp
  Farm192 und Hemp Field194, eindeutige Feldbindung und2×2-/1×1-Flächen.
  Modell-Cost→Funds, Feldnachbarschaft und80 produktive Ticks je Einheit sind
  ausdrücklich authored. Modellgebundene Employees erhalten ihre Worker aus
  echten zugezogenen Bewohnern. Höchstens zwei Farms mit je einem gekauften
  Feld; eigene Farm-Couriers und vorhandene Warehouse-/Marketrollen bewegen
  Hemp und Ceramics über denselben Navigator. Die sechs HouseState-Bestände
  bleiben alleinige Hausauthority. Keine Gratisfelder, Vorräte oder Worker.
- **Modell und eingefrorene Vorgänger:** Der neue ausdrückliche
  `prepare_production()`-Pfad bereitet zusätzlich die beiden benannten Zeilen
  vor. Der alte Sieben-Zeilen-Pfad verlangt und validiert sie nicht; auch
  semantisch unpassende zusätzliche Zeilen ändern seine bisherigen Werte nicht.
  Neue synthetische und echte Modellprüfungen vergleichen alle Felder über
  fünf Difficulties sowie die bisherigen sieben Building- und15 Housezeilen.
  Neue Object-/Command-/Courier-IDs sind append-only. Die neuen funktionalen
  Zuordnungen sind authored und behaupten keine Originalmechanik oder fertige
  Originalgrafik. Regeln1–9/Schemas1–25 bleiben eingefroren; frühere
  Schema26-Entwicklungsrevisionen werden atomar verweigert.
- **Unabhängige Reviews und Härtungen:** Feldnachbarschaft promoviert alle
  Koordinaten vor Addition/Subtraktion in int64; extreme Feldcommands werden
  atomar verweigert. Ein zusätzlicher bestätigter Farm-INT_MAX-Restoreüberlauf
  in der späteren2×2-Footprintaddition führte zu einer vorgelagerten vollständigen
  Geometrieprüfung per Subtraktion, bevor irgendein Offset addiert wird.
  Ergänzt sind14 Grenzlagen über drei getrennte Verweigerungspfade (World,
  Writer, Reader/Restore),42 neue Negativfälle beziehungsweise939 Hemp-Save-
  Verweigerungen sowie vier legale Farm-/Feld-Eckroundtrips. Alle drei finalen
  Konfigurationen bestätigen diese Fälle. Eigener Farmstock, ausgehendes Cargo und letzte
  Transferhistorie können die eigene tatsächliche
  Ernte nicht überschreiten; eine zweite Farm ohne Feld kann keine fremde
  Ernte übernehmen. Koordinierte Stockverschiebungen nach Market/Haus trotz
  unveränderter Gesamtmasse werden ohne mögliche tatsächliche Ankunft
  verweigert. Alte Ceramicsfahrten ersetzen nicht die erste Hemp-Farmankunft.
  Diese begrenzten Herkunftsprüfungen rekonstruieren keine vollständige
  Besuchshistorie: spätere Sequenzen dürfen ältere kompaktierte Ziele/Waren
  enthalten. Ownerpause, begonnene Kanten, physischer Rückkehrplatz, sichere
  Abrissverweigerung und stale/doppelte Ankünfte bleiben gebunden; höchstens
  acht letzte typisierte Transferzeilen. Reiner Ledgerkern unverändert.
- **Finale Builds und relevante Tests:**39 Targets in Debug, Release und
  ASan+UBSan gebaut;196 Tests je Konfiguration tatsächlich registriert.
  Finale Auswahl jeweils **49/49 PASS**, insgesamt147 CTests, keine Fehler
  oder Skips. Exakte CTest-Zeiten Debug117.442s, Release11.807s,
  Sanitizer46.241s. Vollständige Commands/Exitcodes, Inventare, Logs und JUnit
  unter `implementation/{debug,release,sanitizer}-targeted-result.json`,
  `*-targeted-*.log` und `*-targeted.xml`. Save-Negativfälle je Konfiguration:
  Hemp939, bestehende Produktion800, Market677 und eingefrorene Rule9-Goods575.
  Dies ist die relevante49er-Auswahl, keine vorgezogene vollständige196er-
  Phasen-Gate-Suite.
- **Echtes Modell und separate OS-Prozesse:**
  `python3 tools/test_original_production.py .local/gog-extracted/app --build-dir .local/market-walkers-finish/debug-build --output .local/original-economy/oe16/production-fixtures/final-source-55c8`:
  **PASS**, Bericht-SHA
  `55623e8de8f89ac0560765c5cdfa6f73f4339c4a241cd29cdb2db6c3061c5de9`,
  tatsächliches Fixturebinary
  `9d025cc5e34ea617743d5c42ae1ac5f36593c76246be1d9c40e4317005baa27a`.
  24 Straßen, sechs Häuser, Clay Pit, Kiln, Warehouse, Market und eine separat
  bezahlte Farm samt Feld kosten1162 Funds;10838 verbleiben. Bei400t bestehen
  42 echte Bewohner, aber durch gestoppte Quellen noch null Waren. Bei480t
  bewegt der Farm-Courier die erste Hemp-Einheit mit null Legacy-Cargo;34
  Worker sind regulär zugewiesen. Bei3650t hat Haus-ID5 tatsächlich
  gelieferte Hemp8/Ceramics4; die gemeinsame Bilanz lautet Hemp33=31 Stock+2
  Cargo und Ceramics35=35 Stock. Alle vier weiteren Waren bleiben unsupported0.
  Beladene Ownerpause/Rückgabe und natürliches Feuer separat geprüft.
  Zwei neue OS-Prozesse setzen Mid-Hemp-Courier und Zweigüter-Hausstand jeweils
  mit10000 Ticks, einem bezahlten Straßenbau und weiteren10000 Ticks fort.
  Vollständige SaveDocuments, kanonische Bytes, Provenance und Bilanzen stimmen
  exakt überein. Eingabesaves samt Metadaten, hashidentische private Modellkopie
  und Originalinventar1464 Dateien/819711091 Bytes bleiben unverändert.
  Rule7/Schema23, sechs Rule8/Schema24- und zehn Rule9/Schema25-Saves bleiben
  unabhängig bytegleich; diese Nachweise ergänzen die frische vollständige
  OE-00-Baseline unten. Keine Snapshot-/Stock-/Workerinjektion.
  Sechs tatsächliche Python-/C++-CLI-Outputguards gegen das aktuelle Binary
  verweigern Ausgaben außerhalb des privaten OE-16-Roots, Symlinkpfade und
  den alten OE-15-Root; Zielpfade bleiben abwesend. Symlinkvorbedingungen wurden
  frisch geprüft; frühere Ausgaben ohne wiederhergestellten Testsymlink bleiben
  als abweichendes Setup erhalten. Beleg `oe16/persistence/actual-output-guards.json`.
- **SDL-Nachweis und Präsentationsgrenze:** Explizite Farmtaste N und Feldtaste M mit
  tatsächlichen bezahlten Mausklicks, Farm ohne kostenloses Feld, bewegtes
  Hemp-Cargo, echte F5/F9-Handler und zwei Waren im selben HouseState geprüft.
  Der SDL-/Sanitizernachweis wurde zum finalen Quellenstand55c8 wiederholt.
  Die tatsächlich geöffnete Softwareaufnahme
  `implementation/production-view-final.png` zeigt Revision16, Tick1636,
  Funds11002 und House2 mit Hemp2/Ceramics4. Die vier authored Testhäuser mit
  je37 Bewohnern sind synthetisch; der echte Modellprozess oben hat42 Bewohner
  in sechs Häusern.100 Idleframes verursachen keine Worldkopien, Commands,
  BFS-, Datei-, Decode- oder Uploadarbeit. Native/human **NOT_RUN**; die
  endgültige Originalsprite-/Objektkomposition bleibt ein späterer Auftrag.
  Ausgeschlossener Entwicklerstart: `sh tools/play_original_economy.sh`.
- **Frische Forkvergleiche zum Quellenstand55c8:** Debug, Release und ASan+UBSan jeweils
  **74/74 PASS**, unverändert46 parity/18 corrected_bug/6 unverified/4 unsupported.
  Alle18 Housingbeobachtungen und der1800-Werte-Originalmodelldigest bleiben
  exakt.21 Archive, fünf World-/DTO- und zwei zusätzliche Modellheader geprüft;
  die fünf geprüften additiven Modelländerungen sind einzeln gepinnt, die
  übrigen numerischen Module und historischen Nicht-current-Belege unverändert.
  25 zusätzliche Sanitizerprozesse mit leerem stderr und28 Pythonverträge
  bestanden. Alle drei vorherigen Paritätsverzeichnisse bleiben unter
  `parity-*-before-bounds` erhalten; neue Läufe stehen in
  `parity-*-run-after-bounds.log`. Portable
  `core-parity.json` SHA
  `e71ec3fec3d0a60f36ecbce9447a2260d1ebf7278f5fa9ee960ef51891ec8718`;
  `implementation/parity-validation-summary.json` SHA
  `3fb26f0a56f0e2c0ca24ff529201fc5592076c68bbb565be83d3e6f378b32f9f`.
- **Vollständiger Legacyabschluss:**
  `python3 tools/original_economy_baseline.py verify --data .local/gog-extracted/app --build .local/reswall/release-build --reference .local/original-economy/oe00/reference/contract.json --output .local/original-economy/oe00/oe16-final-verification`:
  **PASS**, Quelle55c8 und Originalinputs während des gesamten Laufs unverändert.
  Alle72 vollständigen Saves,72 Snapshots und der Vertrag anschließend
  unabhängig wörtlich bytegleich nachverglichen.167 vollständige Maps,
  165 vorbereitet/zwei bestehende Konflikte,1485 Policyfälle sowie elf Rampen-
  und zehn Bridge-Restartpaare exakt. Kanonischer Vertragsdigest weiterhin
  `12823612834d27099189c18e31ed4fed4761c9ff34486227b3075cf1069a49e5`;
  raw `contract.json` SHA
  `1407fdf01945e85e62e92b231d84e0224f0804cc7a9a2d914d4e344e89d6da03`;
  `run.json` SHA
  `9acab3800c5dec0aa735c182d840aabc6a26a0cdb58390e0e7bf1711be38f82f`;
  `implementation/legacy-literal-proof.json` SHA
  `b318f1ff1d4b796cc00d03c1606bcb16af560a0508c3a2ed86cd378ad61d9fae`.
- **Quellenabschluss und Grenzen:** Source-/Test-/Toolhash
  `55c8d195d45e7b2d3a22a9fb37c69fef29e2fc7520c4c363d9aa1ec798513287`,
  35 eingefrorene Sourcepfade/42 gesamte Taskpfade. Der vorangegangene Stand
  `6967cba3fad26b862164e63c7fc495b8fe46d741dd792e41e5a5c555120953c8`
  unterscheidet sich von Prozessstand
  `675a5b46cd87d0569f8e2d6810c30694112b7f9544a0d0ccf46c4982a2371007`
  ausschließlich durch den ungültigen Test-Sentinel von
  `OriginalInfluenceSource+1` auf `OriginalHempField+1`: Objects14/15 sind nun
  gültig,16 und255 ungültig. Zwischen diesen beiden Ständen bleiben die
  Runtimebinaries unverändert. Der anschließend bestätigte Farmfootprint-
  Restorebefund ist im neuen55c8-Quellenfreeze behoben; alle drei neuen Builds,
  der aktuelle echte Prozesslauf, SDL-Nachweis und die drei neu ausgeführten
  Paritätsvergleiche sind **PASS**. Die fünf Szenenberichte initial/stopped/
  paused_return/final/fire sind gegenüber675a vollständig identisch, nicht
  lediglich auf dieselben Eckwerte gekürzt.
  6967 bleibt ausdrücklich ein **Zwischenstand**. Frühere
  vollständige Auswahlversuche mit diesem veralteten Sentinel bleiben als
  `*-before-sentinel`-Belege erhalten. Finale49 relevante CTests je Debug/
  Release/Sanitizer und der frische vollständige Legacyvertrag sind **PASS**.
  Alle21 Archivhashes und35 Sourcepfade passen nach den Fullbuilds weiterhin
  exakt; `implementation/parity-post-fullbuild-archive-check.json`.
  **OE-16 technisch PASS**, OE-17 TODO, Gate C TODO und native/human NOT_RUN.
  Kein Verbrauch, keine Hausentwicklung oder vollständige Originalspiel- bzw.
  Originalsprite-Parität behauptet. Eigener Task-Commit nach Abschlussprüfung;
  danach OE-17 gemäß Gesamtauftrag ohne Wartepunkt für manuelle Playtests.

### OE-17 – Umsetzung, 2026-10-17, Basis86cde0c

- **Vertrag und Versionierung:**
  [Verbrauchsvertrag](../original-economy/consumption-runtime.md) vor der
  Implementierung festgelegt. Ausschließlich die aktive Rule10/Schema26/
  policy3 verwendet jetzt Entwicklungsrevision17. Frühere Entwicklungsrevisionen
  werden atomar verweigert; Regeln1–9/Schemas1–25 und normales Spielprofil
  bleiben unverändert. Der ausdrücklich ausgeschlossene Entwicklerstart
  `sh tools/play_original_economy.sh` verwendet seinen eigenen Revision17-Save.
- **Tatsächliche Konsumgrenzen:** Der unveränderte pure Kern liest die aktuelle
  Housezeile und bucht je Ware `min(stock,max(need,0))`, ohne Footprint- oder
  Bewohnermultiplikation. Null Bewohner konsumiert nichts. Bronze und Lack
  bleiben getrennte Kernoperationen, in dieser World ohne reale Quellen0.
  Der unveränderte gespeicherte Scheduler ist die einzige Clock: Phase39,
  Schritte0/7, zuerst abgeschlossene Ticks40/397/856/1213. Nach Produktion,
  tatsächlicher Bewegung, Fire und Zuzug erfolgt genau eine Buchung. Eine
  tatsächliche Ankunft an derselben Tickgrenze wird bereits berücksichtigt;
  freier Puffer wirkt bei der nächsten Disposition. Keine400-Tick-Nachfrage,
  Servicegutschrift, Warenquelle oder vorgezogene Evolution.
- **Persistenz und unabhängiger Review:** `house_consumption` enthält genau
  eine sortierte Zeile pro gekauftem House mit sechs uint64-Zählern. HouseState
  bleibt der einzige Hausbestand; die Spaltensummen entsprechen dem gesamten
  Ledgerverbrauch. Bounded House-/Ledgerkandidaten werden vor Veröffentlichung
  geprüft, ohne Worldkopie oder doppelte Abbuchung. Das bezahlte Housealter
  begrenzt mögliche Schedulerereignisse; die gebundene Shelterzeile verschärft
  das strukturelle Maximum8 pro Ereignis auf ihren tatsächlichen Bedarf.
  Der isolierte positive Need1-Test verschiebt ausschließlich real vorhandenen
  Housebestand in Verbrauch: Empfang und Masse bleiben unverändert, der
  statische Validator und JSON-Reader akzeptieren die Struktur, erst der
  gebundene Restore/Writer verweigert den zu hohen Zähler. Zusätzlich sind
  Nullbedarf, genaue IDs/Anzahl, unbekannte/fehlende/doppelte Keys, Integer-
  und Summenüberläufe sowie koordinierte vollständig konsumierte Geschenke
  ohne tatsächliche Distributorankunft negativ geprüft. Mögliche Farm- und
  Hausankünfte werden auch für konsumierte Mengen verlangt; passende letzte
  Completed-Lieferungen können den kumulativen Empfang nicht überschreiten.
  Kompaktierte Transferhistorie bleibt notwendige Provenienz, kein vollständiges
  Besuchsjournal. Waren-/Zielrotation wird gemeinsam fortgeschrieben; finite
  Läufe mit sechs Häusern/einem Market sowie20 Häusern/zwei Markets liefern
  und konsumieren tatsächlich beide Waren je Haus, ohne universelle
  Starvation-Freiheit zu behaupten.
- **Echte Erstlauf-Fails und Korrekturen:** Der erste ASan-Save-Test meldete
  einen echten Use-after-free in der neuen Testiteration: normale Commands
  ersetzten die World, während Buildingreferenzen weiterverwendet wurden.
  Der Test sammelt nun IDs vor den Commands. Der erste Debug-Worldtest
  überschritt sein180s-Limit. Seine unveränderten Sechs-Waren-Assertions lesen
  nun eine gemeinsame `original_goods_balances()`-Abfrage statt sechs
  wiederholter vollständiger Abfragen; der vollständige20k-Tick-/Snapshot-
  Vergleich bleibt bestehen. Die fehlgeschlagenen Erstläufe werden nicht
  nachträglich als PASS geführt: Logs/Results/JUnit bleiben unter
  `implementation/*before-test-fix*` erhalten.
  `implementation/test-only-fix-binding.json` rekonstruiert den vollständigen
  Zwischenhash `5ed0df3eb0826735058e6d8760c7896bcb239b6e3a5413e37dfa1a54e79a3532`
  aus dem finalen Stand mit genau zwei gepinnten Testdatei-Overrides.
  Sämtliche Runtime-, Fixture- und Legacyquellen bleiben bytegleich; die
  Legacytargets kompilieren keine der geänderten Testtranslationseinheiten.
  Diese explizite Bindung erhält frühere Prozess-/Legacybelege am finalen
  Quellenstand, ohne einen nicht ausgeführten erneuten Gesamtlauf zu behaupten.
- **Finale Builds und relevante Tests:**41 Targets in Debug, Release und
  ASan+UBSan gebaut;198 Tests je Konfiguration tatsächlich registriert.
  Finale Auswahl jeweils **51/51 PASS**, insgesamt153 Konfigurations-CTests,
  keine Fehler oder Skips. CTest-Laufzeiten Debug154.068s, Release10.598s,
  Sanitizer54.619s; der finale Debug-Consumption-Worldtest benötigt76.03s.
  Exakte Configure-/Inventory-/Build-/CTestcommands, Exitcodes und Zeiten
  stehen unter `.local/original-economy/oe17/implementation/` in
  `{debug,release,sanitizer}-targeted-result.json`, den gleichnamigen Logs
  und JUnitdateien. Ausführung über
  `python3 .local/original-economy/oe17/implementation/run_targeted.py <Konfiguration>`
  beziehungsweise dessen getrennte `--build-only`-/`--test-only`-Schritte.
  Saveverweigerungen je Konfiguration: Consumption34, Hemp958, Market690,
  Produktion813 und eingefrorene Rule9-Goods575. Die neue Worldsuite prüft
  beide Grenzen±1, min0/Unterversorgung/Leerstand, eine zeitlich regulär
  ausgerichtete echte Ankunft auf der Konsumgrenze sowie laufende bezahlte
  Zweigüterversorgung und20k-Snapshot-Replay. Dies ist die relevante51er-
  Auswahl, keine vollständige198er-Phasen-Gate-Suite.
- **Zwei Modellpfade und vier echte OS-Prozesse:**
  `python3 tools/test_original_production.py .local/gog-extracted/app --build-dir .local/market-walkers-finish/debug-build --output .local/original-economy/oe17/production-fixtures/debug-first`:
  **PASS**, Report-SHA
  `5a26ddbfad4f4ac12611adbcd633ebee169ddbc9ab902ff6c6f5197054b814f2`,
  tatsächliches Fixturebinary
  `9780b4624c74e75c1432025d45c2e4ffb1053c7674070d4ab22af1cd1ad6d37a`.
  Das hashidentische Originalmodell hat Shelter-Hemp0/Ceramics0 und konsumiert
  trotz tatsächlicher Ankünfte korrekt0. Der ausdrücklich getrennte positive
  Modellpfad verändert genau zwei Bedarfsbytes einer privaten Kopie auf1/1;
  alle anderen Bytes einschließlich Kapazität7, Preisen und Employees bleiben
  identisch. Seine SHA lautet
  `6b26637469de347541154a3902a2bf4f1da5d7c35d92d56966c8f6c8f6261dfa`.
  Beide Szenen bezahlen24 Straßen, sechs Häuser und die vollständige Clay-/
  Pottery-/Warehouse-/Market-/HempFarm-/Feldkette mit1162 Funds;10838 bleiben.
  42 Bewohner wandern regulär zu. Ohne Snapshot-, Stock- oder Workerinjektion
  entstehen reale Waren und reale Lieferungen. Positive Prozessgrenzen:
  Ticks855→856→857, Schritt0 mit Hemp1/Ceramics1;1212→1213→1214, Schritt7
  mit Hemp1/Ceramics2. Der Market wird dabei regulär pausiert; nachweislich
  keine Houseankunft verdeckt die geprüften Stock-/Audit-/Ledgerdeltas.
  Die separate Worldsuite oben prüft gerade den tatsächlichen Same-Tick-Arrival.
  Vier neue OS-Prozesse laden realen Mid-Hemp-Courier, reales Zweigüterhaus
  sowie beide authored Vor-Konsumgrenzen. Je10000 Ticks, ein real bezahlter
  Straßenbau und weitere10000 Ticks ergeben vollständig gleiche SaveDocuments,
  kanonische Bytes, Provenance und Warenbilanzen. Eingabesaves samt Metadaten,
  beide privaten Modell-/Mapinputs und das Originalinventar1464 Dateien/
  819711091 Bytes bleiben unverändert. Sechs tatsächliche Python-/C++-CLI-
  Guards verweigern Outside-, Symlink- und alte OE-16-Ausgabepfade ohne
  Zielerzeugung; `oe17/runner-guards/report.json`.16 Pythonverträge PASS.
- **Unabhängige Raw-Rekonstruktion:**
  `.local/original-economy/oe17/world/raw-process-audit.json` bindet den finalen
  df785-Quellenstand und das tatsächliche unveränderte Fixturebinary.
  **92725 Checks PASS**:25 Schema26-Saves und20 Reports unabhängig aus ihren
  gespeicherten Zahlen rekonstruiert, sechs Warenbilanzen, rohe Clay-/Pottery-
  Vorgeschichte, Houses, Cargo, Finanzen, Quellenidentitäten sowie direkte volle
  Byte-/Wertvergleiche aller vier Restartpaare. Der Raw-Audit bestätigt die
  Artefakte; tatsächlicher OS-Prozessstart wird vom separat ausgeführten
  Fixtureworkflow belegt und nicht aus Savebytes allein abgeleitet.
- **Frische Forkvergleiche:** Debug/Release/ASan+UBSan jeweils **74/74 PASS**,
  unverändert46 parity/18 corrected_bug/6 unverified/4 unsupported.
  21 aktuelle Archive,18 Housingbeobachtungen und der1800-Werte-Modelldigest
  geprüft; alte numerische Module und historische Nicht-current-Belege
  unverändert.25 zusätzliche Sanitizerprozesse stderrleer,28 Pythonverträge
  PASS. Portable `core-parity.json` SHA
  `8ad785cb048893b8194ce888addc806502e30cd2f0f0b44c6cd80eedc636a952`;
  `implementation/parity-validation-summary.json` SHA
  `d7f518bbd74ce6f4ce53145b4a597ea3942f60dfcf5b0e162554afd4d377f70a`.
- **Vollständiger Legacyabschluss:**
  `python3 tools/original_economy_baseline.py verify --data .local/gog-extracted/app --build .local/reswall/release-build --reference .local/original-economy/oe00/reference/contract.json --output .local/original-economy/oe00/oe17-final-verification`:
  **PASS** auf Zwischenquelle5ed0, durch die oben geprüften zwei ausschließlichen
  Teständerungen strikt an die finale Quelle gebunden.72 vollständige Saves,
  72 Snapshots und der gesamte Vertrag unabhängig wörtlich bytegleich;
  167 vollständige Maps,165 vorbereitet/zwei bestehende Konflikte,1485 Policy-
  Fälle sowie elf Rampen- und zehn Bridge-Restartpaare exakt. Vertragsdigest
  weiterhin `12823612834d27099189c18e31ed4fed4761c9ff34486227b3075cf1069a49e5`;
  run.json SHA `d7de57d34137fc7d5c236053f0b53b5b3bf5ef5fe5f83e6aa5de6ee4eb45daa1`;
  finale `implementation/legacy-literal-proof.json` SHA
  `a69b49031d2df97a4b4ad1a0ddfd7452beb02532d55aba95294fc54be828df69`.
  Die separat eingefrorenen Rule7-/sechs Rule8-/zehn Rule9-Saves bleiben
  ebenfalls wörtlich gleich; diese ergänzen die ältere vollständige OE-00-Serie.
- **SDL und Aussagegrenze:** Geöffnete Softwareaufnahme
  `implementation/production-view-final.png`, gebundener Bericht
  `production-view-final.json`: Revision17/Tick1636/Funds11002, House2 mit
  Hemp2/Ceramics4,148 regulär eingewanderte Bewohner in vier ausdrücklich
  synthetischen Kapazität37-Häusern. Bezahlte Tastatur-/Mausbefehle, Farm/Feld,
  tatsächliche Zweigüterankunft und F5/F9 sind geprüft.100 Idleframes ohne
  Worldkopien, Commands, BFS-, Datei-, Decode- oder Uploadarbeit. Dieser
  Nullbedarfs-SDL-Test ist **keine positive Konsumgrafik-Abnahme**.
  Native/human **NOT_RUN**, vollständige Originalsprite-/Objektkomposition
  weiterhin späterer Pflichtauftrag.
- **Abschluss:** Finaler Source-/Test-/Toolhash
  `df7851d80fd8b2d35fcf3777e7047522afe80dd4d8bf3af536a3ad80eb1e28e7`,
  20 eingefrorene Sourcepfade/26 gesamte Taskpfade. **OE-17 technisch PASS**;
  Gate C TODO, OE-18 als nächster Auftrag TODO. Kein Food-/Service-/Evolutions-
  oder Originalart-PASS vorweggenommen. Eigener Task-Commit folgt nach finaler
  Abschlussprüfung; anschließend gemäß Gesamtauftrag direkt OE-18, ohne
  Wartepunkt für einen menschlichen Playtest.


## OE-18 – Start: reale Food-Qualität

- OE-17 ist nach finalen11/11 Prüfungen als `b30718f` committed.
- OE-18 **IN_PROGRESS**; maßgeblich ist der vor der Implementierung erstellte
  [Food-Vertrag](../original-economy/food-runtime.md), ausschließlich Rule10/Schema26 Revision18.
- Getrennte zwei-Crop-Mengen, bezahlter Farmhouse193/Fields195,196, echte
  Market-/Hausankunft und originale Mischung; Source-Scores und Food-Sink authored.
- Gate C bleibt TODO. Keine vorgezogene Originalgrafik-/Evolution-/Endabnahme.


### OE-18 – bezahlte Food-Kette und tatsächliche Qualitätsankunft

- **Basis und Vertrag:** OE-17 `b30718fe08fb118da5dce05a6846d10bfaf3adeb`;
  [vorab festgelegter Food-Vertrag](../original-economy/food-runtime.md).
  Nur Rule10/Schema26/policy3 wechselt auf Revision18. Echte bezahlte
  Farmhouse193-Körper3×3 und separat gekaufte Wheat195-/Millet196-Felder1×1
  verwenden gebundene Modellkosten/-Employees. Maximal zwei Farms mit je
  genau einem ausdrücklich zugeordneten Feld. Alte Farm-/Foodprofile bleiben
  unverändert; kein Gratisfeld, Bestand, Bewohner oder Worker.
- **Laufzeit:** getrennte zwei-Crop-Mengen, eigene reale Farm→Market- und
  Market→House-Couriers, sechs begrenzte letzte Trips,26 begrenzte Konten.
  `loaded-delivered-returned` ist sole Cargo; alte Food-/Cargo-/Residentfelder
  bleiben0. Physische Reservation und Rückkehrraum, Pausen, begonnene Kante,
  Roads, Fire, Workforce und gewöhnlicher Navigator bleiben verbindlich.
  Pro Besitzer und Crop gilt exakt harvest+received+returned=stock+departed+consumed.
- **Original und authored:** ausschließlich positive tatsächliche Hausankunft
  mischt Qualität über den unveränderten Originalkern und akzeptierte Mengen.
  Source-Scores Wheat20/Millet80 und deren volumenabhängige Mischung sind
  ausdrücklich authored; ursprüngliche Scoreerzeugung/Diversität bleiben offen.
  Food-Sink1 an denselben0/7-Grenzen, Wheat-first, Missed Demand und Leerung→
  Qualität0 sind ebenfalls authored. HouseState ist die einzige Qualitätsauthority;
  ein gespeicherter Arrival-Witness bindet vorherige Mengen, Qualität und echten
  Transfer. Kein konstantes20 aus bloßem Stock und keine additive Behauptung
  einer bei Originalrundung konservierten quality-times-volume-Masse.
- **Modell und Bedürfnisse:** alle fünf Originalschwierigkeiten gegen die
  unveränderte Modelldatei geprüft, inklusive tatsächlich berechneter
  Farm-/Feldpreise und Farmworker. Historische sieben Modellzeilen bleiben
  gleich.11 von16 Originalmodul-Dateien sind bytegleich; fünf eng geprüfte
  additive Modell-/Mappingänderungen ergänzen ausschließlich zwei benannte
  Feldzeilen und neue aktive Bindungen. Die reinen Blend-/Housing-/Warenformeln
  bleiben exakt. Ein ausdrücklich synthetischer Foodbedarf80 prüft tatsächliche
  Housezustände: leer falsch, reale Wheat20 falsch, reale Millet80 wahr,
  tatsächliche Leerung wieder falsch; noch keine World-Evolution vor OE-21.
- **Vollständige relevante Tests:** je53 CTests in Debug/Release/ASan+UBSan,
  insgesamt **159 PASS**,0 Fehler/0 Skips, auf vollständiger Quelle
  `b53edf2d3f505551ff8ba506f556f6a2768d1d986711c8655978a5c4db17e4c2`.
  Zeiten194.866/11.478/81.612s. FoodSave verweigert230 Manipulationen; echte
  gemischte20/80-Ankünfte, voller Empfänger/Teilmenge, Rückkehr, Missing Food,
  Ursprung, Phase, Grenzen, Witness und gebundene Finanzfälschung sind geprüft.
  20k-Direktreplay und zwei20k-Savefortsetzungen bleiben vollständig gleich.
- **Warnungsabschluss und ehrliche Quellenbindung:** sieben mehrdeutige
  Einrückungen in genau `OriginalProductionFixture.cpp` und
  `OriginalFoodWorldTests.cpp` wurden ausschließlich mit Whitespace berichtigt.
  Die frühere fehlende optionale Initialisierung war bereits vor b53 behoben;
  erste Buildlogs enthielten noch diese alten Diagnosen. Nach dem Formatfix:
  drei neue komplette43-Target-Builds ohne Warnungen und zusätzlich je ein
  tatsächlicher FoodWorld-Test, also **3 weitere PASS**,10.156/0.917/4.359s.
  Damit sind insgesamt162 tatsächlich ausgeführte Konfigurationsfälle bestanden.
  Der private `format-only-binding/`-Nachweis vergleicht Clang-Tokens, Literale,
  Kommentare und Präprozessorzeilen exakt und rekonstruiert das komplette
  b53-Sourceinventar allein mit den beiden gesicherten Vorbildern. Kein Token,
  Assert, Preis, Runtime-/Modell-/DTO-/CMakebyte geändert. Der tatsächliche
  Release-Prozessfixture bleibt bytegleich mit SHA
  `e967d33e3bd5ad4352bc2da322ec8d167cd4a59f16034e997c453b16db4402ad`.
  Die159er-/Prozess-/SDL-/Legacy-Belege werden ausdrücklich als b53-Läufe
  bewahrt und streng an diese Formatänderung gebunden, nicht umetikettiert.
- **Tatsächliche Prozesse:**
  `python3 tools/test_original_production.py .local/gog-extracted/app --build-dir .local/reswall/release-build --output .local/original-economy/oe18/production-fixtures/final-53`:
  **PASS**, acht separate OS-Restarts mit je10000 Ticks, echtem bezahltem
  Straßenbau und weiteren10000 Ticks. Ganze SaveDocuments, Rohbytes,
  Provenance, Berichte und Bilanzen gleich. Separater Originalmodell-/authored
  H/C1/1-/Foodpfad; Food zahlt12 Häuser,77 Straßen,zwei Farms,zwei Felder und
  einen Markt:1363 ausgegeben/10637 Funds, anfangs0 Bewohner, später84 reguläre
  Bewohner. Reale Lieferung: Wheat20,3W+1M→35 und reine Millet4→80 bei Haus5/
  Tick2750. Straßenreparatur kostet weitere2 Funds. Kein Source-/Stategrant.
  Report SHA `b025edf1e9d13d18c38d81f8ab3bbbdd2e57a563fda89fd80a44fb50a478be62`.
  Unabhängiger Postreview prüft148 Artefakte/4316994 Bytes. Sechs tatsächliche
  C++-/Python-Ausgabegrenzen verweigern Outside, echten Symlink und alten
  OE-17-Root ohne Ausgabe;19 Pythonverträge PASS. Originalinventar1464 Dateien/
  819711091 Bytes und Metadaten unverändert.
- **Unabhängiger Rohdaten-Audit:**242831 Checks PASS,45 Schema26-Dokumente,
  davon20 mit Foodfarms,40 aus gespeicherten Feldern rekonstruierte Foodreports
  und acht direkte vollständige Restartsets. Cropmassen, Cargo, Quelle,
  Ankunft, Clock, Funds und binäre32-Rundung unabhängig rekonstruiert.
  `oe18/world/raw-food-audit.json` SHA
  `d152cdb7d106aa88a83b32ced4c7c54b2bced234dbc20a445050c95ed071bce7`.
  Rawbytes allein beweisen keine Prozessausführung; diese ist separat oben belegt.
- **Parität und Legacy:** Debug/Release/Sanitizer je74/74 tatsächliche
  Forkvergleiche, weiterhin46 parity/18 corrected_bug/6 unverified/4 unsupported;
  21 aktuelle Archive,25 strikte Sanitizerprozesse stderrleer und28 Pythonverträge
  PASS. Portable `core-parity.json` SHA
  `52d1a54304d1b87cf1c6ac99c7474d2d9dc03d8ff000355c15e6ed868bef4444`.
  Frischer vollständiger OE-00-Workflow **PASS** mit source_unchanged=true auf
  b53:72 ganze Saves und72 Snapshots unabhängig wörtlich bytegleich,
  167 Maps/165 vorbereitet/zwei bekannte Konflikte,1485 Policyfälle und elf
  Rampen-/zehn Bridge-Restartpaare. Gesamter Vertragsdigest unverändert
  `12823612834d27099189c18e31ed4fed4761c9ff34486227b3075cf1069a49e5`.
  Zusätzlich Rule7 ein, Rule8 sechs und Rule9 zehn eingefrorene Save-Literale exakt.
- **SDL und Grenzen:** tatsächliche Dummy-/Softwareaufnahme geöffnet und an
  beobachtete JSON-/Save-Sidecars gebunden: Revision18/Tick1970/Funds10802,
  synthetische Kapazität37,148 regulär eingewanderte Bewohner, ausgewähltes
  Haus2 H2/C7/Food[1,0]/Qualität20. Bezahlte O/W/L-Tastatur-/Mausbefehle,
  Food-Midcourier-F5/F9 und100 Idleframes ohne World-/Datei-/Asset-/BFSarbeit.
  Dies ist positive Food-Ankunfts-/Qualitätspräsentation, keine vollständige
  Originalgrafik-/native-/menschliche Abnahme. Native/human **NOT_RUN**;
  Originalsprites und vollständige Objektkomposition bleiben Pflicht von OE-29.
- **Erhaltene Fehlversuche:** erster anonymer Validator kollidierte beim Build
  mit World::validate und wurde gezielt umbenannt. Der SDL-Test wartete zuerst
  auf irgendein Food-Haus statt das nachfolgend ausgewählte dreifach versorgte
  Haus; Wartebedingung wurde verschärft. Vier aktive ältere Tests erwarteten
  noch den einzelnen Ceramics-Distributor bzw Revision17; nun prüfen sie die
  beiden exakten bezahlten Rollen/Owner und Revision18. Erste Paritätsprüfung
  startete vor Export des frischen Berichts. Alte FAIL-Logs/XML/Resultate und
  die Einrückungswarnungen bleiben privat erhalten; kein Assertionabbau.
- **Abschluss:** finale Quelle
  `c3cfaf8b4f61ab06f689aa1e1aed9762d656638437428d8deb9c5e5513ada9b3`,
 37 Sourcepfade/43 gesamte Taskpfade. OE-18 technisch PASS; eigener Commit
 folgt nach strikter Abschlussprüfung. Gate C und OE-19 bleiben bis dessen
 tatsächlichem Start TODO. Danach unmittelbar OE-19, ohne manuellen Testhalt.


### OE-19 – Start: gezielte medizinische Besuche

- OE-18 nach11/11 Abschlussprüfungen als `144eec6` committed.
- Vor Implementierung festgelegter [Service-Vertrag](../original-economy/services-runtime.md):
  Revision19, zwei explizit bezahlte Provider, reale cargo-freie Ankunft, nur
  passende96er-Timer und Countdown in Phase35 jedes51er-Passes.
- Gate C bleibt TODO; keine Implementierungs-/Originalgrafik-/Endabnahme vorweggenommen.

### OE-19 – implementierter Stand vor vollständiger Gate-Abnahme

- Zwei bezahlte2×2-Provider207/208, getrennte reale cargo-freie Courierrollen,
  exakte vorbereitete Preise/Employees, Round-Robin/Wait/Replan/Return und
  Phase35-Verfall sind angeschlossen. HouseState bleibt alleinige Timerauthority;
  typisierte Provider-/Trip-/Witness-Zeilen binden Herkunft und Restore.
- Aktive Revision19 verweigert18/future20; alte1–9/1–25-Pfade bleiben gesondert.
  Modell208 ist ausschließlich neue aktive Fähigkeit; historische siebenzeilige
  Vorbereitung erhält keine zusätzliche Abhängigkeit. Pure ServiceRules,
  GoodsRules und HouseEvolution bleiben unverändert.
- Frühe Debug-Prüfungen:2 Modellfälle und medizinischer Savefall bestanden;
  danach World- und echte SDL-Viewtests bestanden. Das sind Vorprüfungen,
  keine Behauptung finaler Vollsuite. Der erste Worldtest-Build verwendete einen
  nicht vorhandenen CourierState-Gleichheitsoperator; korrigiert auf vollständige
  gespeicherte Couriervector-Gleichheit. Ein Sign-conversion-Warning im neuen
  Mindestfahrzeitprodukt ist mit geprüftem explizitem Cast behoben. Fehlversuche
  bleiben unter `.local/original-economy/oe19/first-debug-*` erhalten.
- Unabhängiger Review ergänzte koordinierte Trip-/Witness-/Timer-Negative
  gegen unmöglich kurze Ankünfte und kompaktierte Herkunft. Physischer
  Mindestabstand bindet die tatsächliche5-Tick-Kantenzeit; keine Rekonstruktion
  heutiger Straßen als angeblich historische Route.
- Nun folgen frische vollständige Builds/Suites aller202 registrierten CTests
  in Debug/Release/ASan+UBSan, neue3×74-Parityfälle, zwölf getrennte20k-
  Prozessfortsetzungen, unabhängiger Rohdatenabgleich und kompletter
  Legacy-/167-Kartenvertrag. Gate C bis zu deren tatsächlichem Abschluss TODO.
  Originalart ist weiterhin für OE-29 offen; Softwaretests sind kein Human-PASS.

- Der erste vollständige Buildversuch `gate-c/final-1` wurde in allen drei
  Konfigurationen während des Builds abgebrochen, nachdem ein zweiter Review
  eine echte fehlende Bewegungszeitbindung fand. Ein unabhängiger C++-Prozess
  reproduzierte mit tatsächlicher letzter Outbound-Kante: manipulierte
  Disposition wird geladen, der nächste Tick wirft. Beleg:
  `oe19/restore-regression/before-fix.json` auf Quelle `d9f5b58`.
  Neue notwendige Vertex-/Fortschrittsgrenzen für Outbound/Return/Idle und
  zwölf Ablehnungswege plus drei positive nächsteTicks sind ergänzt. Der
  ursprüngliche Freeze und alle drei unterbrochenen Logs bleiben erhalten;
  daraus wird kein Build-/Test-PASS abgeleitet. Es folgt ein neuer kompletter
  Versuch `final-2` nach frischen Fokusprüfungen.

- `final-2` erreichte tatsächlich einen warnungsfreien vollständigen Debugbuild
  und74/74 Debug-Paritätsfälle. Die geöffnete tatsächliche SDLaufnahme zeigte
  jedoch eine neue Inspector-Verdrängung: wiederholte Herkunftstexte der
  Medizinzeilen versteckten die bisherigen Food-Details. Release/Sanitizer
  wurden deshalb noch während des Builds gestoppt; keine Vollsuite behauptet.
  Der ausschließlich aktive Inspector ist nun kompakt. Frischer Viewbuild und
  echte Bedienprüfung bestanden; die erneut geöffnete Aufnahme zeigt H/C,
  beide getrennten Timer und Food/Qualität/Crops gleichzeitig. Beobachtungs-
  und vollständige SaveDocumentbytes vor/nach diesem Präsentationsfix sind
  exakt gleich. Alle früheren Artefakte bleiben getrennt erhalten. Nun folgt
  vollständig `final-3`; ältere Durchläufe werden nicht umetikettiert.

### OE-19 – tatsächlicher Abschluss und Gate C

- Finale Quelle `6d38de1170b5b1b473e9540c142dbaf021d53ef40b818e5cfdcdd822701700ef`:
  30 geänderte Sourcepfade, insgesamt35 Taskpfade einschließlich Dokumentation
  und Paritätsexport. Der tatsächliche Abschluss-Preflight besteht13/13 Checks.
  Belege unter `.local/original-economy/oe19/implementation/`.
- **Vollständige Gate-Suites:** drei frische, warnungsfreie Cleanbuilds mit
  Werror; danach ohne Filter je **202/202 CTests PASS** in Debug, Release und
  ASan/UBSan. Insgesamt606 Testausführungen, keine Fehler, Timeouts oder
  übersprungenen Fälle. Tatsächliche Laufzeiten1811.29s/130.85s/505.06s.
  Alle aufgezeichneten Executables und Archive nach dem Lauf unverändert;
  strikte Sanitizerflags, keine Sanitizerdiagnostik. Ergebnisse/JUnit/Logs:
  `oe19/gate-c/final-3/{debug,release,sanitizer}/`.
- **Ankunft und Restore:** bezahlte2×2-Provider207/208 kosten30/50 und
  benötigen7/8 tatsächliche Worker aus dem gebundenen Modell. Reale Ankunft
  setzt nur die passende Kategorie auf96; alle16 Phase35-Schritte,
  gleichzeitiges96→95, Verfall, Wiederbesuch, Pause/Brand, Roadcut/Repair und
 20k-Replay sind geprüft. Bewegungszeit, letzte Disposition, Ziel, tatsächliche
  Ankunft und kompaktierte Herkunft bleiben strikt gebunden. Alte1200er-Service-
  und2400er-Healthfelder bleiben0; keine Waren durch medizinische Besucher.
- **Tatsächliche Prozesse:**12 getrennte OS-Fortsetzungen mit je10000 Ticks,
  bezahlter Straße und weiteren10000 Ticks; ganze SaveDocuments, Rohbytes und
  Berichte stimmen überein. Der Workflow erfasst23 reale Kindprozesse.
 12 Häuser/77 Straßen/zwei Foodfarmen/zwei Felder/Market/207/208 kosten1443;
 10557 Funds bleiben, anfangs0 Bewohner, später84 reguläre Einwohner für63
  benötigte Worker. Sechs tatsächliche C++-/Python-Ausgabegrenzen verweigern
  Outside, Symlink und alten OE-18-Ausgabeordner ohne Ausgabe.
  Prozessbericht SHA `a5b7a2eada1be76234f24be0373c50440db44522b0065ab78bced605b367f3a9`.
- **Unabhängige Rohdaten:**457858 Checks PASS über65 Schema26-Dokumente,
 60 rekonstruierte Services-/Food-/SixGoods-Berichte und zwölf Restartsets.
 194 Dateien/6537447 Bytes unverändert; eigener Prozessreview bindet die
  tatsächlichen PIDs/Kommandos/Logs. Raw-Audit SHA
  `bec0e32b220e1d70ef0c83e223a5715dc202fe9ece6e66e7902c8a0fcdd784f8`.
  Der vorherige private Auditfehler durch die historische siebenzeilige
  Rule7-Modellkopie bleibt erhalten; diese wird jetzt gesondert an ihre
  eingefrorenen Bytes gebunden, niemals als aktives Produktionsmodell gelesen.
- **Parität:** je74/74 tatsächliche Vergleiche in drei Konfigurationen,
  weiterhin46 parity/18 corrected_bug/6 unverified/4 unsupported;24 aktuelle
  Archive,25 strikte Sanitizerprozesse ohne stderr und28 Pythonverträge PASS.
  Exportierte `core-parity.json` SHA
  `16d88d55ef366d2c48187d090f8d05b960d4e1f41410c38936f7b415b96a176e`.
- **Legacy:** frischer kompletter OE-00-Vergleich PASS:72 ganze Saves und72
  Snapshots wörtlich bytegleich,167 Maps/165 vorbereitet/zwei bekannte
  Konflikte,1485 Policyfälle sowie elf Rampen-/zehn Bridge-Neustartpaare.
  Zusätzlich die eingefrorenen Rule7-/8-/9-Literale exakt. Quelle und gesamte
  Originalinstallation mit1464 Dateien/819711091 Bytes samt Metadaten gleich.
  Der separate historische Releasebaum verwendet unveränderte Warnungsregeln:
  genau eine bekannte Switchwarnung der gesperrten
  `tools/CityV16PlaythroughFixture.cpp:25`, deren Quelle bytegleich bleibt.
  Der vorherige Werror-Fehllauf bleibt separat erhalten; dieser Legacybaum
  wird ausdrücklich nicht als warnungsfrei bezeichnet.
- **Prüferkorrekturen:** die tatsächlichen zwölf Build-/Testmanifeste binden
  exakt517 Code-/Datendateien, normalisierter SHA
  `cf0a7730279650932ecebeecb6a61337ff8f8f7bde5dcd9a0d4f83456368b2e3`.
  Der rekursive520-Dateien-Pin enthält zusätzlich drei ignorierte Finderdateien;
  deren frühere Einzelaufnahme wird nicht behauptet. Der konfigurierte externe
  Pythoninterpreter wird separat über Configure-Argument, Inventar, erfassten
  Testcache und Build/Test/Livehash gebunden. Alle übrigen Binaries/Archive
  bleiben strikt im Buildroot. Beide ursprünglichen privaten Auditorversionen
  und FAIL-Berichte bleiben erhalten; keine Tests, Quellen oder Ergebnisse
  wurden dafür geändert.
- **Darstellung:** echte SDL-Dummy-/Softwareaufnahme1100×700 erzeugt und
  geöffnet. BeiTick2649/Funds10722 zeigt dasselbe Haus3 Hemp3, Ceramics7,
  Food[3,1]/Qualität35 sowie Herbalist96/Acupuncture96 gleichzeitig lesbar.
  Synthetische Kapazität37 und148 regulär Zugezogene sind ausdrücklich
  authored, kein Originalmodell-Playthrough.100 Idleframes ohne Worldänderung
  oder Command-/Tick-/BFS-/Datei-/Assetarbeit. Originalsprites und vollständige
  Objektkomposition bleiben OE-29; native/human **NOT_RUN**.
- OE-19 und Gate C technisch PASS. Eigener Commit folgt nach der finalen
  Status-/Diffprüfung; danach direkt OE-20 ohne manuellen Testhalt.


### OE-20 – tatsächlicher Abschluss am2026-10-17

- Eigener bezahlter2×2-Schrein214, Modellpreis/-Workers Normal40/4 und genau
  ein eigener cargo-freier Courier. Erst tatsächliche DirectReligion-Ankunft
  setzt Ancestor40; Phase35 kann im selben Tick39 daraus machen. Kein Visit
  durch Kauf, Disposition, Return, Idle oder Rendering.6/6/60 bleibt geschlossen;
  Quelle/Pause/Brand, Zielbrand, Disconnect/Wait/Repair und vollständiger
  Verfall/Wiederbesuch sind tatsächlich geprüft. Ein voller bezahlter46-Körper-/
  26-Actor-Zustand erlaubt weiter normale Straßenreparatur und verweigert
  zusätzliche Quellen atomar. Strikte Revision20 verweigert19/future21.
- **Drei inkrementelle Werror-Builds** und danach dieselben **123/123 CTests je
  Debug/Release/ASan+UBSan PASS**, insgesamt369 Ausführungen ohne Skip/Fehler.
  Frisches Inventar204, explizite123er-Auswahl; keine vollständige Gate-D-Suite.
  Testzeiten235.99/33.94/118.66s. Exakte Configure-/Build-/CTestkommandos,
  JUnit, Source-/Original-/Binary-/Archivbindungen stehen unter
  `.local/original-economy/oe20/implementation/runs/final3/`.
- **3×74 Kernvergleiche PASS**,24 tatsächliche Archivbindungen,25 einmalig
  ausgeführte strikte Sanitizer-Proben ohne stderr und28 Pythonverträge PASS.
  Nach dem exakten Export wurden die25 Proben nur noch read-only gebunden.
  Der portable Bericht hat SHA
  `16c96b2b932280b6644d61f9c1921121a8e89956f90c2a0b059eb5090e9390da`.
  Elf bisherige Numerikdateien bleiben bytegleich; Modell-/Mappingänderungen
  beschränken sich auf die aktive214-Erweiterung.
- **18 tatsächliche20k-Prozessfortsetzungen** mit den bisherigen zwölf und
  sechs neuen Ancestor-Checkpoints, darunter Midroute, Straßenbruch, Visit,
  vor/nach Ablauf und Wiederbesuch.30 echte C++-Kinder mit Exit0 umfassen
 18 Produktions- und vier zusätzliche Legacy-Restarts. Unabhängiger Review
  prüft36 ganze Save-/Reportpaare und262 Dateien. Der zusätzliche Raw-Audit
  rekonstruiert95 Schema26-Dokumente und90 Berichte mit723601 Prüfungen;
 262 Inputdateien/9711020 Bytes bleiben exakt. Anfang1483/10517 und bezahlte
  Reparatur1485/10515 sind ohne Waren-/Worker-/Timerinjektion erhalten.
  ProzessreportSHA `6da17d642ee2b0cc351235c9379283f7f0546594e955445eb732ea2873e9843f`;
  RawSHA `fee5109fb95cab5d294acb641f0b7af0d57ef88f05302eb8a1915ee33eab58bc`.
- **Tatsächlich geöffnete SDL-Aufnahme:** Tick2774, Funds10682,148 Bewohner
  im ausdrücklich synthetischen37er-Kapazitätsmodell. Dasselbe Haus zeigt
  Ceramics5/Hemp1/Food[1,3]/Qualität65/Herbalist96/Acupuncture95/Ancestor39.
  Alle Werte und Food-Fehlversuche5 bleiben im Standardinspector sichtbar.
  Echte Y-/Mauskäufe, Helpmodal, Midroute-F5/F9 und100 ruhende Frames ohne
  Worldänderung oder Datei-/Asset-/BFS-Arbeit PASS. Aufnahme und Manifest:
  `oe20/implementation/{view-final/,production-view-final.json}`.
  Das ist Software-SDL/diagnostische Darstellung, keine Originalsprite- oder
  native/human-Abnahme. Die Originalkörper/Walker bleiben OE-29 zugeordnet.
- **Frischer Legacyvollvergleich PASS:**72 vollständige Savebytes und72
  Snapshots,167/165/2 Karten,1485 vollständige Policies sowie11 Rampen- und
 10 Bridge-Restartpaare. Alle1464 Originaldateien/819711091 Bytes einschließlich
  Mode/mtime unverändert. LiteralvertragSHA
  `1407fdf01945e85e62e92b231d84e0224f0804cc7a9a2d914d4e344e89d6da03`.
  Der getrennte historische Releasebaum behält WerrorOFF und genau eine
  bekannte Switchwarnung im unveränderten gesperrten Playthrough-Fixture;
  kein Warnungsfreiheitsclaim dafür. Die drei aktuellen Produktbuilds sind
  dagegen mit Werror warnungsfrei. Sechs frische Ausgabe-/Symlink-/Altroot-
  Schutzprüfungen PASS ohne Output oder Originaländerung.
- Erhaltene negative Versuche: erster Build scheitert am signed/unsigned-
  Vergleich des neuen Kapazitätstests; der explizite Cast ändert keine Assertion.
  Die erste geöffnete Aufnahme zeigt umgebrochene Medizinwerte und verdrängte
  Food-Fehlversuche; kompakte Namen und echter Inspectorregressionstest beheben
  dies. Ein privater Workflowwrapper zählt zuerst alle22 Resumes statt der18
  Produktionsresumes. Sein FAIL/Exit1 bleibt bytegenau erhalten; ein separater
  unabhängiger Korrekturaudit bindet die18 Produktions- plus vier Legacy-
  Resumes an dieselben30 tatsächlichen PIDs/Logs. Keine Spielergebnisse oder
  fehlgeschlagenen Berichte wurden umgeschrieben.
- Finale Quelle `da8cdc585c6e72ff25fb0a52c21f50aab88d4333335734a87532391e73bba7c1`,
  FreezeSHA `86dbffb866a5af80554104a8b341c40eafadd3b4a20455d56a39b2596418946c`.
  Tatsächliche unabhängige Vorprüfung **8/8 PASS** vor Statusänderung.
  OE-20 erhält seinen eigenen Commit; danach folgt OE-21 direkt. Gate D bleibt
  bis OE-27 offen, menschliche Abnahme bleibt NOT_RUN.


### OE-21 – tatsächlicher Abschluss am2026-10-17

- Vorheriger eigener Commit: OE-20 `825fd2c65d90c9139e283f2bf3e191bb5feb4e39`.
  [Vorvertrag](../original-economy/housing-runtime.md) vor Implementierung;
  ausdrücklich Rule10/Schema26/policy3, Revision21 statt20, keine Migration.
- Eigene bezahlte Well72 (Normal25/4), normaler realer96er-Wasserbesuch,
  Phase28-Materialisierung/Phase35-Verfall; alte Influence60 bleibt ohne Wasser.
  Gemeinsame Routen, echter Actor und eigene Herkunftsbelege, keine Aura.
- Produktive Phase39 führt den unveränderten puren Evolutionskern vor dem
  Verbrauch0/7 aus. Frühe authored Grenze Plain Cottage, zwei permanente
  erste Aufstiegsinputs, historische Bedarfssinks je drei Modellzeilen und
  globaler Ursprung entfernter neutraler Quellen. Keine erfundene Eviction.
- Echte Originalmodell-Szene: fünf Häuser/24 Straßen/Food/Wheat/Market/Well/
  Ancestor kosten708 (Funds11292), initial0 Bewohner und0 Waren. Wirkliche
  Wasserankunft Tick170; Hut193 mit4 Bewohnern; Plain346 mit12; Counter9 bei856;
  zehnte verfehlte Auswertung907 macht Hut bei weiterhin22 Bewohnern. Die
  vollständige bezahlte Goods/Food/Medizin/Ancestor/Water-Szene kostet1948
  (Funds10052), erreicht deren gemeinsame tatsächliche Ankünfte plus Plain729.
  Modellbytes bleiben SHA `7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6`.
- Gegenproben: alte Influence60/kein Wasser, kein Ancestor, kein Food/Quality,
  unzugänglicher Market sowie ausdrücklich authored HempNeed1/DesHigh99.
  Original-Plain hat keinen Hemp-/Ceramicsbedarf; keine falsche Negativbehauptung.
- Drei warnungsfreie inkrementelle Werror-Builds, je47 relevante registrierte
  CTests in Debug/Release/ASan+UBSan:141 PASS,0 FAIL/Skip. Tatsächliche CTestzeit
  74.89/7.65/41.05s. Kein Cleanbuild-/voller Gate-D-Claim.
  Quelle529 rekursive Dateien inkl drei Finderdateien:
  `b3cd644b904835e6e5b04fbe412d1c45aa01af815d0ecc6471e8b2176eaf1d08`.
  FreezeSHA `e648143b0dbc234c3183437808c7c0a3946c6f9445387b895344f618a8e0f912`.
  Unabhängiger141-JUnit/118-Livebinary/39-Archiv-Check:
  `b279bf983f5cede3666db717cf9c9d3474173187add364292703e45ad06de5cc`.
- Drei tatsächliche74er-Paritätsvergleiche,24 Archivbindungen,25 strikte
  San-Probeaufrufe und28 Python-Paritätsprüfungen bestanden.15 reine Dateien
  bytegleich; einzige numerische Modulergänzung ist der explizite Water-Well-
  Mappingfall, keine Änderung einer Regelformel. Portabler Export:
  `2226e812eff5558c47f0568a7f3ccaa7657bd07b37d01f7bb491343a23aecd65`.
- Tatsächlich37 C++-Kindprozesse: sechs Creates,24 Production-Fortsetzungen
  je20k und sieben Legacy-Kinder (vier davon Fortsetzungen).48 vollständige
  Save-/Report-Paarsets gleich;125 Schema26-Dokumente/120 Raw-Projektionen,
 17 alte Save-/16 Report-Literalpaare und334 Dateien unabhängig neu gelesen,
 1.374.023 Prüfungen. Originale/Workspace/Source/Binaries vor/nach unverändert.
  ProzessreportSHA `cd675c0145e875999151e8d2f772773af63d02a833a01fbcf70c32f130ab54ba`;
  Workflow `167a9eeb981befaf3d5249af0d599a1f7d315fd1b6a50afcc991a572f661e094`;
  unabhängiger Reader `cd03e84975bf48f09beaa833d095a2bff8d8af4f238d553a9774d8206f055dd1`.
- Sechs tatsächliche C++/Python-Ausgabepfadverweigerungen vor jedem Schreiben
  bestanden. Getrennter spielbarer Launcher gebaut, `sh -n` PASS; Standardziel
  korrigiert auf eigenen `rule10-revision21.json` statt des alten18er-Pfads.
- Tatsächlich geöffnete Software-SDL-Aufnahme: Typ5/Wasser1/Raw94/Counter0,
  H1/C8, Quality65, Medizin96/95, Ancestor35 und Foodmiss5 zusammen sichtbar;
  Q-Kauf/Maus/Modalgate, echte F5/F9-Midtrips und100 ruhende Frames geprüft.
  Aktuelles Binary bytegleich dem aufgenommenen; synthetisches Modell,
  kein Originalsprite-/native-/human-PASS. VisualReviewSHA
  `9f63e3caae9a2dd5e848987c4842eb4b3ebb87b1f3fc8000b16d8d719b66f0d6`.
- Erhaltene FAILs führten zu echten Korrekturen: Water-Fixture-Fieldindex,
  aktiver Pausen-Syntaxadapter, exakte Evolution-vor-Verbrauch-Tests sowie
  tatsächlich fehlende historische Populationsparität. Ungerade Bewohner
  brauchen eine erreichbare frühere ungerade Kapazität; Shelter6→3 bleibt
  verweigert, echtes Shelter7→Hut14 erlaubt später9/11/13. Alte positive
  Revisionsliterale20 im Harness wurden auf21 korrigiert, Preise und volle
  House-/Sinkvergleiche nicht gelockert. Beide gescheiterten Prozessläufe
  bleiben mit2 bzw5 tatsächlichen Creates/0 Resumes erhalten.
- Final3-Paritäts-Prepare verweigerte einen schon vorhandenen bytegleichen
  Kandidaten nach25 echten Probes; ursprünglicher FAIL und unabhängiger
  Recovery-Audit bleiben. Final4-Prepare/Export/Postcheck bestehen frisch.
  Ein erster privater Abschlussaufruf hatte nur einen relativen Pfadfehler;
  der vollständige korrigierte Abschlussprüfer wurde tatsächlich ausgeführt.
  FinalvalidierungSHA `fe732bafa2d98911b4aa567279c8786582d9dd101c4ecacac8a0a1f14a11bf78`.
- Kein Wartepunkt für manuelle Tests. Nach eigenem OE-21-Commit folgt OE-22;
  Originalobjektgrafiken/Actors folgen weiterhin den Aufgaben26/29, voller
  Cleanbuild-/Corpus-Gate D erst nach27.


### OE-22 – 2026-10-17, Basis d0dae283

- Vor Beginn sauberer Feature-Branch/HEAD und tatsächliche Remote-Refs geprüft:
  main41091b66e960b4c82c378d5ae3b83088325f5a61,
  origin-feature887dda562622d59487a92b8966b62b218cb59ce3. OE-21-SHA oben
  nachgetragen. Vor Runtime-Anschluss festgelegter
  [Religionsvertrag](../original-economy/religion-runtime.md).
- Read-only-Quellenprüfung: Originalmodell/EXE und sieben gebundene Quellen
  unverändert.129 enge Assertions/29 Disassemblyfenster/1326Instruktionen/
  3979Byte tatsächlich PASS. Bericht `oe22/research/religion-source-facts.json`,
  SHA c87f2f43dcd34931ff70447b5fb170eeda12448cbdde5235b3fa63b0fe074fba.
  [Neue Reverse-Dokumentation](../reverse/original-religion-source-contract.md)
  belegt215/21745/4/2×2 und21980/12/3×3, getrennte40er-Timer sowie zusätzlich
  originales initiales Confucian-Elite-Gate und echten Wood/Paper-Input.
  Der frühe Adapter bleibt an beiden219-Grenzen ausdrücklich authored; keine
  Gratisressource und keine volle Original-Lifecycle-Paritätsbehauptung.
- Aktive Revision22,52Körper/32Courier,12Provider/12Trips/120Witnesses.
  Historische46/26 unverändert. Sechs feste Housing-Witnessslots0/1/5/6/7/8;
  echte3×3-Grundfläche auch für historische Mindestfahrzeit und Begun-edge-
  Plausibilität. Neue U/K/V-Werkzeuge ersetzen inaktive Overlays; Q/Y bleiben
  Wasser/Ancestor. Spielstart `sh tools/play_original_economy.sh` nutzt eigenes
  `rule10-revision22.json`. Keine alten Save- oder Regelversionen umgedeutet.
- Frühe Debug-World-/Model- und Housing-/SDL-Prüfungen bestanden. Ein neuer
  Testfall hatte sein Food-Feld innerhalb der3×3-Farm platziert; exakt auf
  angrenzendes x+3 korrigiert, anschließender SaveTest PASS. Erste Hilfe-
  Arraygröße15 statt16 und ein Build vor CMake-Regeneration korrigiert;
  ursprüngliche Logs bleiben `oe22/early-*.log` erhalten.
- Früher echter C++-Create-PID7253 PASS; separater Pythonreader fand seine
  falsche Annahme43Worker nach bewusstem Pausieren. Korrigiert auf echte
  midroute43/visited23 und separate tatsächliche Water/Food20/Ancestor-
  Voraussetzungen bei Kategorie-Negativen. Frischer PID7482/27.268s und
  separater Reader beide PASS: sechs Checkpoints/18 Saves/18 Projektionen.
  `oe22/world/early-religion{1,2}-*.json`; ursprünglicher FAIL bleibt erhalten.
- Erste vollständige Auswahl: Debug/Release/Sanitizer jeweils49/50 PASS,
  jeweils nur `original-ancestor-world` FAIL. Der alte Test erwartete bei46/26
  einen Baustopp, obwohl Revision22 ausdrücklich52/32 erlaubt. Er behält nun
  46/26 als positiven bezahlten Zwischenstand, kauft sechs reale neue Quellen
  für340 nach und verlangt bei53.Körper exakte Kapazitätsbegründung plus
  unveränderten vollständigen Snapshot/Cache. Keine Runtimeänderung dafür.
  Alle drei originalen FAIL-JUnits stehen `oe22/implementation/runs/final/`.
- Finaler Sourcefreeze531Dateien:93a3b5d965a8aa8ece6810a12a8f4f9e469f62852f9553b60334728b36092e1a;
  `oe22/validation2/source-freeze.json`, SHA
  c24f7dc754b9b8342fe3c08ffe19336c85eed0aee865947690bf4b072dd811c2.
  Gegen vorigen Stand ist ausschließlich der genannte Kapazitätstest anders.
  **Drei warnungsfreie inkrementelle Werror-Builds und3×50 tatsächliche CTests
  PASS**,0Fail/Skip. Keine Cleanbuild- oder vorgezogene Gate-D-Behauptung.
  Tatsächliche Befehle/JUnits/Zeiten/Inventare unter
  `oe22/validation2/runs/final2/{debug,release,sanitizer}/{build,test}/`.
  Unabhängiger JUnit-/124Livebinary-/39Archive-Abgleich PASS.
- Frische3×74 Paritätsfälle,24 Archivebindungen,25 strenge Sanitizer-Prozesse
  mit leeren Fehlerstreams und28 Python-Postexporttests PASS. Alle alten
  numerischen Beobachtungen/Klassifikationen exakt. Elf von16 Originaldateien
  bytegleich; fünf exakt additive aktive Model-/Namens-/Mappingerweiterungen,
  sechs geprüfte ABI-Pins. Portable `core-parity.json` SHA
  bcb3ca58e90e88b0581d16a74292fbfdcb76f53a344bce9420db28d2a702c98f.
- Tatsächlicher finaler Prozessworkflow: **44 unterschiedliche C++-PIDs**,
  sieben aktive Creates,30 aktive separate20k-Fortsetzungen, drei historische
  Creates/vier historische Resumes. Keine25/34/44-Verwechslung als aktive
  Replayzahl. Source/Originalinventar/Workspace/Binaries vor/nach unverändert.
  Acht Häuser/24Roads/sieben Quellen bezahlen1003; drei neue Quellen allein170
  und20Worker. Alle Quellen gemeinsam43Worker; nach drei Pausen tatsächlich23.
  Sichere Roadreparatur kostet weitere2. Dao-only/Budd-only/neither/noConf
  verwenden exakt nur zwei geänderte Plain-Bedarfsfelder in Modellkopien;
  die Wege und Ankünfte bleiben real. Authorisierte Wasserbarriere bleibt
  tatsächlicher unveränderlicher MapPermissions-Blocker, kein Originalmap-PASS.
- Unabhängiger Rawreader PASS:1.894.137Checks,60 vollständige Save-/Reportpaare
  byte-/JSONgleich,155Schema26-Dokumente/150 Rohprojektionen; alte7/8/9 mit17
  Save- und16Report-Literalpaaren exakt.410Dateien/28.087.645Byte gebunden.
  Privat kopierter Run-ID-Reader hatte irrtümlich auch einen Rohdatenkey und
  historischen Referenzpfad umbenannt; genau diese Literale zurückgesetzt,
  ursprünglicher Reader-FAIL und Code archiviert. Kein Produktionsartefakt
  und kein Source verändert. Bericht `oe22/validation2/final2-runner-review.json`.
- Sechs tatsächliche C++-/Python-CLI-Refusals für äußeren Pfad, Symlink und
  alten21er-Ausgabebereich PASS, keinerlei Original-/Zieländerung.
  Finale Software-SDL-Aufnahme tatsächlich geöffnet:2992Ticks, synthetisches
  Modell, Typ5,37 regulär zugelassene Bewohner, sechs getrennte Timer,
  Water1/raw94, Food3/8/Quality65 und Foodmisses5 gemeinsam sichtbar.
  Echte Tastatur-/Mauskäufe, Mid-Walker-F5/F9 und100Idleframes ohne
  World-/Datei-/BFS-/Assetarbeit bestehen. Unveränderte UI-Code-/Binarybytes
  binden diese Aufnahme auch an final2; nur Kapazitätstest änderte sich.
  `oe22/implementation/view-final/` und
  `oe22/validation2/presentation-capture-binding.json`.
- Root-Abschlussprüfung `oe22/validation2/final-validation.json` PASS,
  SHA2904bfad9ff033574be2d3ce921dfefd2a52a65c7af0391be57f57ead54e210d.
  Gate D bleibt bis27 offen, Originalsprites bis29 und Native/Human getrennt.
  Voller167er-Corpus wurde hier nicht erneut als neu ausgeführt ausgegeben;
  aktueller begrenzter Legacybeleg ist obige Auswahl plus Literalprozesspaare.
  Eigenständiger22er-Commit, danach unmittelbar23 ohne manuellen Testhalt.


### OE-23 – 2026-10-17, Basis1e7e271

- Vorabvertrag: [culture-runtime.md](../original-economy/culture-runtime.md),
  Quellenbefund: [original-culture-source-contract.md](../reverse/original-culture-source-contract.md).
  Bezahlte Music211/Acrobat212/Drama213 mit2×2/3×3/3×3 und echten Modellpreisen,
  Employees, privaten Zielrotationen und getrennten96er-DirectService-Ankünften.
  Die originale Schule→Venue-/Performer-Ausbildung wird nicht behauptet;
  gezielte Schulbesuche sind in UI/Report als experimentell gekennzeichnet.
- Aktive Revision23:58Bodies/38Courier,18Provider/18Trips/180Witnesses.
  Neun historische Slots bewahren das alte Präfix0/1/5/6/7/8 und hängen2/3/4 an.
  Veröffentlichte Regeln/Schemas und alte46/26-Grenzen bleiben erhalten.
  Reine Availability-Diagnose trennt vorhandene, tatsächlich beschäftigte,
  bereits erreichbar vorbereitete Quellen, aktuelle Hinfahrt und wirklichen
  House-Timer/letzte Ankunft; keine Zukunftszusage, Mutation oder BFS.
- Review fand fehlende Fire-Registrierung bei Kultur und den drei zuvor
  ergänzten Religionstypen. Alle sechs sind nun im unveränderten Brandpfad.
  Echte Quellen-/Zielbrände, laufende Tour, Dispatchsperre und Restore wurden
  tatsächlich geprüft; dies ist kein rückwirkender Revision22-Nachweis.
- Frühe4Debugtests PASS. Früher C++-Aufruf ohne authored Influence.map wurde
  vor Checkpoint abgewiesen; neue private Datenkopie enthält nur originale
  Modellbytes plus authored Map. PID29278/68.276s lieferte echten Kultur-PASS.
  Dessen erster Pythoncheck verlangte irrtümlich aktuellen Food20 nach dem
  Aufstieg397; der belegte Input war Food20, danach verbrauchte derselbe Tick
  die letzte Nahrung. Historischer Input und exakter wheat-first-Sink sind
  jetzt getrennt streng geprüft. Negative Kontrollen behalten echte Food20.
  Ursprüngliche FAILs und unveränderte echte Artefakte bleiben privat erhalten.
- UI-Frühprüfung fand umbrochene Diagnoseworte; kurze vollständige Zeilen
  halten alle neun Timer, Foodquality/Wheat/Millet und Foodmisses sichtbar.
  Doppelte Bauzähler wurden in den tatsächlich geöffneten Bildern korrigiert.
  Externe SDL-Umgebungsvariablen hatten zuvor die eigenen Hints übersteuert;
  der echte Software-SDL-Test läuft mit seinen vorgesehenen eigenen Hints.
- Finaler Sourcefreeze533Dateien:
  daaf02b0b2bae7a9fa174f1781e9d54affa63f3c25f4ed4e93523b9081a21d28;
  `oe23/validation/source-freeze.json` SHA
  baa706b1d2a451eb8a5d32c2f02a6db5a0e50ed877a0ea224177de155f8f92d4.
  Drei warnungsfreie inkrementelle Werror-Builds, **3×53 CTests PASS**,0Fail/Skip.
  Debug150.745s, Release14.675s, Sanitizer68.600s. Unabhängig130Livebinary-
  und39Archivepfade gebunden. Keine Cleanbuild-/Gate-D-Behauptung.
- Frische3×74 Paritätsfälle,24 Archivebindungen,25 strenge Sanitizeraufrufe
  und28 Python-Postexporttests PASS. Alte Beobachtungen/Klassifikationen exakt;
  elf Originaldateien bytegleich, fünf exakt begrenzte aktive Modellergänzungen,
  sechs verifizierte ABI-Pins. `core-parity.json` SHA
  cd335d3de7a962982a432a640b4b171f04dcae4dc17047d35c3441fa9f530722.
- Tatsächlicher Workflow:51 unterschiedliche C++-PIDs =8 aktive Creates,
  **36 aktive separate20k-Resumes**,3 historische Creates und4 historische
  Resumes. Hauptszene kauft10Häuser/44Roads/alle neun Services/Water für1383;
  70 echte Shelterworker vor Dispatch,43 nach Pausieren der drei Schulen.
  Reparatur kostet2. Need1-Kopien ändern jeweils exakt ein Plain-Feld5/6/7.
  Kein Stock-/Worker-/Snapshotgrant. Vollständige Source-/Original-/Workspace-
  und Binärinventare während der tatsächlichen Ausführung unverändert.
- Unabhängiger Reader PASS:3.434.555Checks,72 ganze Save-/Reportpaare exakt,
  185Schema26-Saves/180 Rohprojektionen,17 alteSave-/16 alteReport-Literalpaare;
  484Dateien/46.574.517Byte gebunden. Zusätzlicher adversarialer Reportcheck
  bindet Status/Access exakt an den Save und verweigert drei koordinierte
  Falschmeldungen, während20 reale Availability-Szenen/3Rawpaare gültig bleiben.
  Sechs echte C++-/Python-CLI-Refusals für Außenpfad/Symlink/alten22er-Bereich PASS.
- Finale Release-Software-SDL-Aufnahme tatsächlich geöffnet:3082Ticks,
  synthetisches Modell,148 regulär zugezogene Bewohner, ausgewähltes Typ5-Haus
  mit37Bewohnern, allen neun echten Timern, Water1/Raw92, Food3/8/Quality65,
  Hemp1/Ceramics8 undFoodmisses5. Echte Maus-/Tastaturkäufe, F3-/Help-Gates,
  Mid-Walker-F5/F9 und100Idleframes ohne World-/BFS-/Datei-/Assetarbeit PASS.
  Bild, Beobachtungs-JSON, Save, tatsächlicher PID38906 und finaler Binaryhash
  unter `oe23/validation/view/` gebunden. Kein Originalgrafik-/Native-/Human-PASS.
- Root-Abschluss `oe23/validation/final-validation.json` PASS, SHA
  c67a6d655a29af764ac49a869381d8cfe4fbe3225c86282d9b6c23c396245c98.
  Statische Quellenprüfung bleibt ausdrücklich PASS_BOUNDED_STATIC_RESEARCH;
  kein Original-EXE-Lauf. Gate D folgt27, Originalsprites29, vollständiger
  Original167er-Corpus hier nicht erneut ausgeführt. Nach eigenem23er-Commit
  unmittelbar OE-24, ohne manuellen Testhalt.


### OE-24 – 2026-10-17, Basis95967ce

- Vorabvertrag: [luxury-runtime.md](../original-economy/luxury-runtime.md),
  Originalquellen: [original-luxury-source-contract.md](../reverse/original-luxury-source-contract.md).
  Neun separat bezahlte Producer und drei passende bezahlte Pflanzen verwenden
  echte vorbereitete Modellkosten/Employees. Tea, Silk, Bronzeware und
  Lacquerware erreichen über tatsächliche Carts, Warehouse und Market das Haus
  und dessen Verbrauch. Fünf Auxmaterialien bleiben getrennt von den sechs
  Hauswaren; Clay und Hemp behalten jeweils ihre vorhandene Mengenauthority.
- Aktive Revision24 erweitert auf82Bodies/58Courier; alte Regeln1–9,
  Schemas1–25 und deren46/26-Grenzen bleiben erhalten. Bezahlte Hemp→Paper-
  Produktion und tatsächliche Paperankunft sperren/erlauben Academy-Dispatch;
  je neuer Tour wird1Paper verbraucht. Managed Ernten,80er-Rezeptzeiten,
  PaperOnly und direkte Schulbesuche bleiben ausdrücklich authored.
  Das anfängliche originale Confucian-Elitegate gehört weiterhin zu25.
- Der Originalnachweis bestätigt Bronze/Lacquer als OR-Bedarf, aber getrennten
  Verbrauch beider tatsächlich vorhandenen Bestände. Der eingefrorene reine
  GoodsRules-Kern bleibt unverändert. Echte positive Aufstiegs-/Verbrauchs-
  zeugen und strikte fehlende-Ware-Kontrollen verwenden klar markierte Need1-
  Modellkopien; unveränderte frühe Originalbedarfe0 werden nicht erfunden.
- Tatsächliche frühe Läufe fanden Clay-Reservierungs-, Reisezeit- und
  Bootstrapfehler sowie einen Market ohne angrenzende Straße. Die bezahlte
  Hauptszene kauft43Bodies/107Roads für3159 und wartet auf echten Zuzug bis
  zur installierten Workeranforderung185. Eine gekoppelte Goods-/Zielrotation
  konnte bei sechs Waren Tea übergehen; feste Target×Good-Paare beheben dies
  ohne neue gespeicherte Authority. Belegte endliche Ankünfte/Verbräuche sind
  keine universelle Starvationfreiheitsgarantie.
- Produktive neue Bodies/Pflanzen/Materialempfänger verweigern Abriss atomar.
  Der bisher erlaubte bezahlte neutrale Wellabriss besteht während realer
  Warenproduktion mit vollständiger Conservation. Tests decken vier Luxusgüter,
  Teilmengen, fehlende Zutaten, volle Reservierungen/Rückkehrraum, zwei
  Warehouses, Pause, Brand, Straßenreparatur, Überlauf und strikten Restore ab.
- Finale Quelle539Dateien:
  9c8be66e22201027849937025a7b350061c7e66f0b0c8527ca864f79bf19f75c;
  `oe24/validation2/source-freeze.json` SHA
  cc3b57ec89817ee14c230fe2b22b2c26a8fd1ec5bb0894d00b0938709981985d.
  Drei warnungsfreie inkrementelle Werror-Builds; **3×56 CTests PASS**,0Fail/Skip.
  Tatsächliche CTestzeiten: Debug583.761s, Release34.899s,
  ASan+UBSan195.559s; keine Sanitizerdiagnose. Unabhängig136Livebinary-
  und39Archivepfade an die tatsächlichen Builds/JUnit-Dateien gebunden.
  Kein vollständiger Cleanbuild-/Gate-D-Nachweis.
- Frische3×74 Kernvergleiche,25 strenge Sanitizerprozesse und28 Python-
  Postexporttests PASS. Alte numerische Beobachtungen/Klassifikationen exakt;
  aktive Modellergänzungen und neue DTO-Pins eng geprüft. `core-parity.json`
  SHA09cf9096bb3e18c2c65999c516a8d4ec9a2cb720b58234c54f38530c41cccbd5.
- Tatsächlicher final2-Workflow:59 unterschiedliche C++-PIDs = neun aktive
  Creates, **43 separate aktive20k-Fortsetzungen**, drei historische Creates
  und vier historische Resumes. Unabhängiger Reader PASS:5.784.687Checks,
  220Schema26-Dokumente/215Rohprojektionen,86 volle Save-/Reportpaare exakt;
  alte Regeln7/8/9 mit17Save-/16Report-Literalpaaren unverändert.
  563Dateien/78.329.192Byte gebunden. Sechs echte C++-/Python-Refusals für
  Außenpfad, Symlink und alten23er-Ausgabebereich PASS.
- Der erste finale Workflow verweigerte seine Services-Szene: deren Test
  erwartete fälschlich sieben statt sechs Actors ohne Warehouse. Nur dieser
  Fixturezähler wurde auf sechs exakte Owner/Rollen korrigiert; ursprünglicher
  Source, drei Binaries und FAIL bleiben unter `oe24/validation/` erhalten.
  Der unabhängige final2-Reader rechnete zunächst bestehendes Pottery mit
  einem statt zwei Clay pro Einheit. Nur diese Prüfformel wurde korrigiert;
  erster Reader/FAIL unter `validation2/reader-before-pottery-coefficient-fix/`
  erhalten. Danach tatsächlich neu ausgeführter Reader PASS; keine Save-,
  Runtime- oder Bilanzmanipulation.
- Beide finalen Release-Software-SDL-Bilder tatsächlich geöffnet, PID73969:
  House bei3639Ticks/148Bewohnern, Typ5 mit37Bewohnern, alle neun Timer,
  Water1/raw94 und Food5/8/Quality80/Misses5 lesbar. Industry bei2947Ticks:
  PaperMaker#22, Hemp→Paper, Workers10/10, tatsächlicher Progress/Input/Output/
  Cargo und explizites Original-Art-pending-Label ohne Überlappung. Industry-
  Bauaktionen, echte Maus-/Tastaturkäufe, F5/F9 und Idle-Grenzen bestehen.
  Bilder/Beobachtungs-JSON/Saves unter `oe24/validation2/view/` gebunden.
- Statische Originalprüfung PASS_BOUNDED_STATIC_RESEARCH mit234Assertions;
  Originale unverändert, kein Original-EXE-Lauf. Root-Abschluss
  `oe24/validation2/final-validation.json` PASS, SHA
  305cd8ff9066c6ab9e338d5ad14598117f2bdb843c943855bad77387a9a6c512.
  Voller167er-Corpus hier nicht erneut ausgeführt. Gate D bleibt bis27 offen;
  Originalsprites29 sowie native/human Abnahme bleiben getrennte offene Arbeit.
- **Ausdrückliche Nutzerentscheidung:** nach dem geprüften eigenen24er-Commit
  Pause. OE-25 wird nicht begonnen; kein Push, PR, Merge, Tag oder Release.


## OE-25 – fünfzehn Hauszeilen und getrennte bezahlte Elitefamilie

- **PASS am2026-10-18**, auf OE24 `7360f84e40c6cd558d3cf87de41c8b713645840f`.
  Aktiv ausschließlich Rule10/Schema26/policy3, `development_revision=25`;
  Revision24/26-Crossloads verweigert, veröffentlichte1–9/1–25 unverändert.
  Vertrag: `docs/original-economy/housing-stages-runtime.md`; Quellenbeleg:
  `docs/reverse/original-house-lifecycle-contract.md`. Eigene Commit-SHA folgt
  in der Antwort und nach bestehender Dateikonvention im nächsten Task.
- Common und Elite besitzen getrennte tatsächliche Körperfamilien; gemeinsames
  Limit20. Der neue authored80er Eliteplot startet11/leer. Echter Zuzug liest
  die Vorereigniskapazität und erzeugt11→13, keine kostenlose12 und keinen
  Common→Elite-Umbau. Alle15 Modellzeilen, getrennte Admission-/Censusselector,
  echte Bedarfe und10/12/50/18-Ereignisgrenzen bleiben im Originalkern.
  Die Plain5-Clamp entfällt. Bereits zugelassene Bewohner bleiben erhalten.
- Feste15 Verbrauchszeilen, erste tatsächliche Übergangsinputs und die letzte
  Phase39-Eingabe werden streng mit Familie, echter Admission, Mengen,
  Quality, Wasser-/Servicewitnesses, Counter und chronologischer Kapazität
  restauriert. Const-Diagnose/15-Zeilen-Matrix ändern weder World noch Routes,
  Dateien oder Assets. UI zeigt einzelne Bedarfe/OR, DES, Counter und Kapazitäten.
- Confucian219 ist bei Dispatch, Ankunft und Diagnose Elite-only und benötigt
  weiter wirklich geliefertes Paper mit1-Paper-Dispatchsink. Common bleibt
  unberechtigt. Neun bereits bezahlte Provider tragen ihre eigenen vorbereiteten
  DES-Zeilen und wirklichen2×2-/3×3-Flächen in stabiler ID-Reihenfolge bei;
  Präsenz auch bei Pause/Brand, historisch nur vorher gekaufte Quellen. Kein
  Alias-Well, Zweitkauf, zusätzlicher Source-Save oder Frame-/Tickaufbau.
- Tatsächliche Originalmodell-/Normal-Szene: Bauausgaben1727/Funds10273.
  Elite11 leer bei0, echter Erstbezug13 mit2 Bewohnern bei120, Counter49 bei2590,
  50. Fehlschlag13→12 bei2641, fünf Bewohner behalten; echte12→13-Erholung bei6313.
  Zugleich ist Common8 real Typ6/20Bewohner, Elite9 Typ13/5Bewohner. Conf-Ankunft
  bei6170 hat einen wirklichen Paper-Sink; eine spätere noch reisende Tour
  zählt nicht als Besuch. Recovered/Advanced/Visited sind tatsächlich derselbe
  Tick6313 und byteidentische Zustände, keine drei zusätzlichen Ereignisse.
- Konkrete höhere Grenzen dieser bezahlten Szene: Common DES6<High14 und
  nächste Herbalist/Music0/1; Elite DES4≤Low35 und <High50, nächster Bedarf
  Herbalist/Music/Acrobat0/1, FoodQuality0/50 und Silk0/1. Aktuelle Hemp-/
  Ceramicsbedarfe erfüllt; kein zusätzlicher Pure-Tick wurde dafür ausgeführt.
  Höhere Stufen/Heavenly werden nicht als erreicht ausgegeben. Privater
  vollständiger Befund: `oe25/high-stage-observation.md`.
- Elitegeometrie bleibt ausdrücklich authored2×2; belegtes Original4×4 und
  atomische Materialisierung vor Erstbezug gehören zum eigenen26er Vertrag.
  Zuzug120/40/+2, Elitepreis, All-residents-Workeradapter, direkte Besucher und
  Produktionsuhren bleiben benannt authored; zwei Evolutionsglobals false.
  Originalsprites29, vollständiger Gate D27, native/human Abnahme offen.
- Finale Quelle543Dateien: `80680cd335add3f62770290f2d2548497d56dbcb1c3909b2b6c38705a0a16d28`;
  `oe25/validation3/source-freeze.json` SHA `0ac74afa2e9c8eb7c039cd94c48127d9f4446e21469f4a566afabd0d74c2d9ef`.
  Drei frische warnungsfreie inkrementelle Werror-Builds; **3×59 CTests PASS**,
  0Fail/Skip. Tatsächliche CTestzeiten Debug648.503s,
  Release36.872s, ASan+UBSan227.226s, keine Sanitizerdiagnose.
  Unabhängig142Binarypfade/39Archive an tatsächliche Builds/JUnit und Livebytes
  gebunden. Kein voller Cleanbuild-/Gate-D-Nachweis.
- Frische3×74 Kernvergleiche,25 tatsächliche strenge Sanitizerprozesse und28
  Postexport-Pythonverträge PASS. Alte Beobachtungen/Klassifikationen exakt.
  Unter `simulation/original/` bleiben15 Kern-/Parser-/Vorbereitungsdateien
  bytegleich; einzig `OriginalModelMapping.h` ergänzt genau den ausdrücklichen
  unmapped-Elitefall, ohne erfundene ModelBuilding-/Grafikidentität. Vier
  aktive ABI-Pins eng geprüft. `core-parity.json` SHA `ee6060212708a759c1f47a4062935e00a25a3f8361d02d39673e3b5ae3793af6`.
- Actual final3: **67 eindeutige C++-PIDs**,10 aktive Creates,50 getrennte
  aktive20k-Fortsetzungen,3 historische Creates/4 historische Resumes.
  Reader PASS mit8267294 Checks,255Schema26-Dokumenten/250Rohprojektionen;
  **100 ganze Savepaare und100 ganze Reportpaare** am Mittelpunkt/Ende exakt.
  Alte Regeln7/8/9:17Save-/16Report-Literalpaare unverändert.
  644Dateien/130937093Bytes gebunden;
  sechs echte C++-/Python-Ausgaberefusals für Außenpfad, Symlink und alten24er-Root.
- Fehler und Korrekturen bleiben tatsächlich erhalten: frühe View-/Einrückungs-
  Compilerfehler sowie alte JSON-/Common-Conf-Fixtureannahmen; `early5`58/59
  (IndustrySave ohne Eliteziel), `early6`57/59 (neuer Elitebefehl erhöhte die
  alte Logistics-Sequenz trotz Refusal). Ein enger neuer Commandguard bewahrt
  alle bisherigen Befehle. `early7`59/59 PASS. Der echte Save-Counter12-RED
  wird durch einen modellabhängigen Guard geschlossen; authored Row9-Need1
  mit echten Counter1/11/zwölftem Reset bleibt speicherbar.
- Erster final1-Prozessworkflow: echter Python-FAIL nach7 erfolgreichen C++-
  Creates; Paper kam bei813 nach Verbrauch der ersten Foodportion. Nur der
  C++-Fixture-Predicate/Kauf-ID-Teil wartet nun gleichzeitig auf tatsächliches
  Paper, volle sichere Besetzung und alle übrigen Bedarfe. Python bleibt strikt.
  Korrekter isolierter Probe3-PASS bei1484: Common Water/Q20/Ancestor/Dao,
  Conf0, zweite echte Foodankunft; Academy Paper4,12Worker, eligiblefalse.
  Kein Farmstaging/Grant/Runtimefix. Vorherige Probe1/2-Refusals waren Root-
  Harnessfehler vor der Simulation (Ausgabepfad bzw. fehlende authored Map),
  keine Gameplaytimeouterkenntnis. Final1-Quellen, Binaries und FAIL sind
  getrennt von den neu ausgeführten final3-Berichten erhalten.
- Final2-Debug58/59: ausschließlich der integrierte UI-Test traf sein altes
  60s-Limit. Unverändert isoliert PASS75,512s/71,249CPU-Sekunden; tatsächliche
  1s-Stackprobe zeigt bestehende Debug-Navigationsassertionen im realen Tickpfad.
  Alter24er-Debugtest43,136s, tatsächliche Spielticks3639→7468. Nur dessen
  CTestbudget wurde begründet auf120s gesetzt; kein Tick, Ziel, Bedarf oder
  Assert gelockert und keine Runtime geändert. Final2-FAIL und Messung bleiben
  unter validation2/ erhalten; final3 wurde vollständig neu ausgeführt.
- Finale Release-Software-SDL-Bilder PID34323 alle vier tatsächlich geöffnet:
  House/Industry/Diagnose/weitere Einzelbedarfe. Neun Timer, DES/Counter,
  actual/need/OR-Werte und PaperMaker-Worker/Input/Output lesbar; authored2×2,
  Original4×4:26 und Original-Art-pending ausdrücklich sichtbar.12 gebundene
  PNG-/Observed-/Saveartefakte; echte UI-Käufe, Scrollen, F5/F9 und Idlegrenzen
  geprüft. Keine Originalgrafik-/native-/human-Abnahme daraus abgeleitet.
- Statische Quellenprüfung:26 Fenster/1651Instruktionen/4751Bytes/52Assertions,
  Original-EXE/Modelbytes und Metadaten unverändert; kein Original-EXE-Lauf.
  Gesamter167er-Mapcorpus hier nicht erneut ausgeführt. Root-Abschluss
  `oe25/validation3/final-validation.json` PASS, SHA `b1b29b8b3f4f12dd7bf95ed7f19d33417cfe0e482ae09c04c96364a10b9e960f`.
  Ausdrückliche Nutzerentscheidung2026-10-18: nach geprüftem eigenem25er-Commit
  erneut Pause. OE26 bleibt unbegonnen; kein Push/PR/Merge/Tag/Release.

## Zwischenmerge OE-14–25 – Nutzerfreigabe am2026-10-18

- Der anschließende Auftrag „so können wir jetzt alles mal mergen und
  commiten?“ erlaubt den Zwischenmerge nach `main` einschließlich Push/PR.
  Er ersetzt den vorherigen Remote-Ausschluss für diesen geprüften Stand;
  OE-26 bleibt unbegonnen und Tags/Releases bleiben ausgeschlossen.
- Frisch abgeglichene Basis: `origin/main` ist
  `41091b66e960b4c82c378d5ae3b83088325f5a61`; zwölf eigenständige Taskcommits
  OE-14–25 bis `9bac35b5626834b42d35d52c23b11eb039b14f21` fehlen dort.
  Arbeitsverzeichnis vor dem Handoff sauber, kein offener PR dieses Branches.
- Code und Tests bleiben unverändert zum tatsächlich geprüften OE-25-Stand.
  Dessen177 CTests,50 getrennte20k-Fortsetzungen und vier geöffnete
  Softwareansichten sind die aktuelle relevante Abnahme. Die Quellbindung
  steht unter `oe25/validation3/postcommit-source-check.json`.
  Kein erneuter Testlauf allein aufgrund dieser Dokumentationsänderungen.
- Regulärer Merge erhält alle Taskcommits; erwarteten PR-Head und tatsächliche
  GitHub-Prüfungen vor dem Merge kontrollieren, anschließend `main` und lokalen
  Feature-Branch ausschließlich per Fast-forward angleichen. Keine erzwungenen
  Ref-Updates, kein Squash und kein Branchlöschen.
- Aufgaben26–33 und Gates D–F bleiben offen. Vorläufige Elite2×2, originale
  Elite4×4 in26, Original-Haussprites29, native/human NOT_RUN sowie die
  dokumentierten höheren Housing-Szenenblocker werden nicht hochgestuft.


## OE-26 – atomare große Elite-Footprints

- **PASS am 2026-10-19**, Basis `f4447163c6dac035de45a9b1ce9ba2222e0e7546`. Aktiv bleibt ausschließlich
  Rule10/Schema26/policy3 mit `development_revision=26`. Revision25 und
  Zukunftsrevisionen werden atomar verweigert; keine Migration, keine Änderung
  der veröffentlichten Regeln1–9/Schemas1–25. Der vor Implementierung erklärte
  [Vertrag26](../original-economy/elite-footprints-runtime.md) bleibt maßgeblich.
  Die eigene Commit-SHA steht nach dem Commit in der Antwort; dieser Commit
  kann seine eigene SHA nicht enthalten. OE-27 bleibt TODO/unbegonnen.
- `OriginalElitePlot=39` ist ein bezahlter leerer authored2×2-Körper;
  `OriginalEliteHouse=40` ist sein materialisierter4×4-Körper. Alle16 Claims,
  immutable Terrain-/Original-/Gate-/Roadberechtigungen, Bridge-Deck/Bänke,
  Rampenendpunkte, signed Foundationhöhe und aktuelle/begonnene Actor-Kanten
  werden vor der ersten Änderung geprüft. Erst wirklicher Einzug erlaubt11→13.
  ID, Ursprung, Bauzeit, Fundsbuchung, HouseState, Waren, Bewohner, Timer und
  laufende Pfade bleiben erhalten. Spätere12..17-Wechsel schrumpfen nicht;
  Common bleibt2×2. Kein zusätzlicher Kauf, Flächenkapazitätsfaktor oder Saveflag.
- Neun unabhängige Konfliktklassen, Ränder und signed Höhenextreme, zukünftiger
  4×4-Eingang, wiederholte Stagnation, wirkliche bezahlte Reparatur, gesperrter
  Abriss und Restart vor/nach Materialisierung sind geprüft. Zwei getrennte
  gleichzeitige Erfolge invalidieren den normalen Routencache gemeinsam;
  konkurrierende Erweiterungen werden in stabiler Hausreihenfolge neu geprüft.
  Bereits begonnene beladene Transporte behalten vollständige Pfade, Fortschritt,
  Cargo, Reservierungen und Rezepte. Sechs direkte Mature-Restore-Policyfälle
  und positive/negative autonome Zählerkontrollen sichern die16 Claims ab.
- Strikter Restore verlangt Plot↔kein Ersteinzug und Körper40↔wirklichen
  Erstbezug mit positiven Bewohnern. Genau ein abgeleitetes autonomes
  Topologieereignis pro Körper40 ist sowohl im Produktionsledger als auch in
  beiden allgemeinen Restoreprüfungen gebunden; kein erfundener Spielerbefehl.
  DES/Provenance, historische Eingaben, Wasser-/Food-/Servicereisegrenzen lesen
  die echte damalige/aktuelle Körpergröße. Keine neue Family-/Side-Authority.
- Tatsächliche Originalmodell-/Normal-Szene bezahlt1729Funds;
  Rest10271. Die wirklich bezahlte Straße[3,8] blockiert
  bei120 einen echten Erweiterungsversuch: Typ11/leer, vier Claims und gesamte
  Househistorie bleiben unverändert. Normales RemoveRoad bei120 gibt keinen
  Refund/Einzug; erst bei160 werden16 Claims/Typ13 und zwei Bewohner gemeinsam
  veröffentlicht. Counter49 bei2641, echte50.Fehlauswertung
  13→12 bei2692; tatsächliche Waren erlauben12→13 bei
  6517. Alle späteren Zustände behalten4×4. Die bezahlten
  höheren Szenenstände sind konkret: Haus7: Typ6, Bewohner31, DES6/High14; Haus9: Typ13, Bewohner5, DES10/High50. Kein Heavenly-PASS.
- Authored4×4-Fallback, Frontzelle, äußerer Pickbereich und Walkerfüße folgen
  denselben echten Körper-/Pfadkoordinaten. Inspector benennt tatsächliche
  Größe und konkreten unreservierten Erweiterungsblocker; Vorschau bleibt rein.
  Natürlicher Brand bei2000, echte Flammenpixel und F5/F9 sind geprüft.
  Keine Originalgrafik wird gestreckt; ursprünglicher Anker/Resize-Lifecycle,
  Zuzug120/40/+2, Preis80 und Workeradapter bleiben getrennt benannt.
- Finale Quelle548Dateien:
  `e1a1eee1cc5cb6a603a2ecd61abee305e837f161df87282234e8a2ec3f467c3f`; Freeze-SHA `6b48dc861688bd1a11a2bcaaf1073eaa45e82c2b32d9622f8ddf8ec658e3e747`.
  Drei frische warnungsfreie inkrementelle Werror-Builds, **3×77
  CTests PASS = 231**,0Fail/Skip; Debug491.716s,
  Release32.123s, ASan+UBSan182.923s, keine
  Sanitizerdiagnose. `bash -n tools/play_original_economy.sh` ebenfalls PASS.
  Unabhängig181Binarypfade/
  39Archive an Builds/JUnit/Livebytes gebunden.
- Frische3×74 Kernvergleiche,
  25 tatsächliche strenge Sanitizerprozesse und
  28 Postexport-Pythonverträge PASS.
  15 numerische Kern-/Parser-/Vorbereitungsdateien bleiben bytegleich;
  der einzige Metadatenzusatz ist der ausdrückliche unmapped-Körper40, keine
  erfundene ModelBuilding-/Spriteidentität. `core-parity.json` SHA
  `9f123ddeee3419c97a4a6d1bacf893814e1886c1f835ab9250e9c88391fcfea4`. Kein vollständiger Cleanbuild-/Gate-D-PASS.
- Tatsächlich68 eindeutige
  C++-PIDs:10 aktive Creates,51 getrennte20k-
  Fortsetzungen,3 historische Creates/4 historische Resumes. Der Reader prüft
  9020103 Assertions,260 ganzeSchema26-
  Dokumente/255 Rohprojektionen,
  jede Owner-/Roadgeometrie und echte Pfade. **102 ganze Savepaare und102 ganze
  Reportpaare** am10k-Mittelpunkt/20k-Ende exakt. Auch echte geblockte120er
  Ausgangszustände bleiben im neuen Prozess20k lang geblockt. Regeln7/8/9:
  17Save-/
  16Report-Literalpaare unverändert.
  655Dateien/139114847Bytes gebunden.
- Frischer vollständiger Legacy-`verify` unter
  `oe00/oe26-final2-verification`:
  **72 SaveDocuments/Snapshots und1485 Policyfälle bytegleich**,167 vollständige
  Manager/165 vorbereitete Maps/2 bekannte Konflikte.13 wirkliche Legacytargets
  frisch gebaut; Originalbytes und Metadaten unverändert. Sechs echte CLI-
  Ausgaberefusals bleiben gebunden; keine Ausgabe unter Originaldaten.
- Echte Fehlversuche bleiben erhalten: early1-Ctestregex kompiliert nicht und
  führte0 Tests aus; early2-release-tests: 63/77 PASS, early3-release: 73/77 PASS, early4-release: 76/77 PASS, early5-release: 76/77 PASS. Die ausgelassene zweite
  City-Topologiegleichung, kollidierende ältere2×2-Fixtures, konkrete
  Counter-/Prerequisiteannahmen sowie UI-Zoom-/Kamera-/Front-/Pickingproben
  wurden eng korrigiert. Die76/77-Kamerafehler sind keine Spiel-/Original-
  Grafikabnahme; kein Bedarf, Tick oder Assert wurde zum Grünwerden entfernt.
  Erster Legacy-verify stoppt vor Vergleich an14 fehlenden Enumfällen im
  Playthrough-Report. Explizite neue Labels bewahren alle alten Labelbytes;
  keine Warnung abgeschaltet. Alter FAIL/final1-Stand bleiben erhalten;
  sämtliche finalen Belege stammen aus `oe26/validation2` und `oe26/production-fixtures/final2`.
  Der erste private Guardlauf scheitert vor jedem C++-Aufruf am fehlenden
  Symlink-Elternverzeichnis. Die getrennte Wiederholung erzeugt sechs echte
  Refusals; initialer FAIL, Logs und erfolgreiche Fortsetzung bleiben erhalten.
- Acht finale Software-SDL-PNGs PID79725 tatsächlich geöffnet:
  Spiel/Industry/Housing/Bedarfe, blockierter Plot und4×4 bei0,5×/1×/2×.
  24 Bild-/Beobachtungs-/Saveartefakte, echte Maus-/Tastaturkäufe, F5/F9,
  Scroll-/Außenpickgrenzen und100 Idleframes ohne World-/Navigations-/Asset-
  Arbeit sind gebunden. Original-Haussprites29 und native/human NOT_RUN bleiben
  offen; Software-/synthetische Fixtures sind keine menschliche Spielabnahme.
  Der bestehende dichte F1-Debugtext überlagert den oberen Kartenbereich der
  drei Zoomcaptures; die drei sauberen Spiel-/Housing-/Bedarfsbilder zeigen
  denselben Hof ohne diese Überlagerung.
- Historischer statischer OE25-Originalnachweis (kein neuer26-Researchlauf): 26Fenster/
  1651Instruktionen/
  4751Bytes/
  52Bindings; EXE/Modelbytes, mtime und Modus
  aktuell unverändert. Kein Original-EXE-Lauf. Originalfamilienseiten2/4 und
  erhaltende Setteroperation sind belegt; die bezahlte Plotmaterialisierung
  und ihr Foundationguard bleiben ausdrücklich authored. Root-Abschluss
  `oe26/validation2/final-validation.json` SHA `69de365223f9d645ef816cf8d017221c0cbe95d287da36876263d65b936516ab`.
  Gate D folgt nach27; OE-27 wird hier nicht begonnen. Kein neuer Push/PR/Merge,
  Tag oder Release durch diesen Task.


## OE-27 – Schwierigkeit, modellabhängige Steuer und Wohnmission

- **PASS am2026-10-19**, sauberer Start auf
  `bcba05334494b4f6f5afd7d4c0d2d76a13469caa`. Rule10/Schema26/policy3 bleibt;
  `development_revision=27` verweigert26 und28 ohne Migration. Regeln1–9,
  Schemas1–25 und Defaults bleiben exakt. Eigene Commit-SHA in der Antwort;
  OE-28 bleibt TODO/unbegonnen. Der vorab festgelegte
  [Runtimevertrag](../original-economy/finance-runtime.md) und die
  [vollständige Finanzabnahme](../original-economy/finance-validation.md)
  dokumentieren Umsetzung und Grenzen.
- Fünf typisierte Modellschwierigkeiten, tatsächliche Bewohner×vorbereitete
  HouseField18-Rate, Steuer vor installiertem Workerunterhalt vor Tickmutation.
  Kadenz816t, Funds-Normalisierung und2Funds/Worker/400t sind authored.
  Nullrate bleibt0; leer/Pre-Tick-Feuer zahlt nichts. Keine Kopie von Rate,
  Difficulty, aktueller Population oder Geldsumme im neuen15-Zeilen-Ledger.
  Modellgebundene Historie, letzter Eingang und echte Installationsalter
  rekonstruieren Steuern/Unterhalt und die vollständige signed Geldgleichung.
  uint64/INT64_MIN, atomare Refusals und echte spätere Bauausgaben>12000 geprüft.
- Die aktuelle authored Mission verlangt gleichzeitig6 nicht brennende
  bewohnte Häuser,1 Common ab Plain,50 Bewohner, positive echte Steuer und
  Funds>=0; ohne Bonus/Latch/Pause. Initialer Load übernimmt gespeicherte
  Difficulty; expliziter Konflikt und abweichendes F9 werden verweigert.
  HUD/T/Inspector zeigen tatsächliche Funds, Tax, Workerunterhalt und Bedingungen.
- Neue statische Forschung:6 Fenster,329 Instruktionen,1094 gegen PE geprüfte
  Bytes und19 Bindings,75 vorbereitete Housezeilen. Bewohner×Modellrate ist
  belegt; Original-TaxOffice/DES/Rundung/Erhebungsaufruf bleiben ungeklärt.
  Keine Original-EXE-Ausführung. Die tatsächlichen fünf TaxRate-/Kapazitäts-
  Modifikatoren sind0, ohne erfundene weitere Multiplikatoren.
- Final4: **drei wirklich frische vollständige Werror-Builds**, je937 normale
  und384 zusätzliche Ninja-Kanten,25 ausgeschlossene Targets,201 Executables
  und13 Archive. Tatsächliche Befehle sind in den Buildberichten gespeichert:
  `python3.15 -B .local/original-economy/oe27/validation/run_gate_d.py
  <debug|release|sanitizer> build --run-id final4 --build-jobs 3`, anschließend
  derselbe Treiber mit `test --test-jobs 3` für Debug bzw2 für die anderen.
  **Je228 vollständige CTests PASS =684,0Fail/Skip**, keine Compilerwarnung
  oder Sanitizer-Diagnose. Tatsächliche CTest-Zeiten: Debug1216.998s,
  Release147.936s, ASan/UBSan553.038s. Unveränderter unabhängiger Validator
  bindet603 Executables/39 Archive, Python3.15, Flags, Cache und Raw-JUnit.
  Abschließendes `sh -n tools/play_original_economy.sh` ebenfalls PASS.
- Alle Nebenprüfungen nach dem Produktionsfix tatsächlich neu ausgeführt:
  `run-final-sequence-final4.py --execute`.3×74 Paritätsfälle,25 strenge
  Sanitizerprozesse und28 Postexport-Pythonverträge PASS. Alle16 numerischen
  Originaldateien und Probevektoren bytegleich. `core-parity.json` SHA256
  `bb42da371d165d0b52ee4c71d0720e7a190df407e2eaaf4b8887e6d773ea9221`.
- Produktionsketten:68 echte C++-Prozesse, davon51 getrennte20k-Resumes,
  260 vollständige Rohsaves/255 Projektionen; je102 vollständige Save- und
  Reportpaare exakt. Finanzrunner:55 echte C++-Prozesse, davon50 getrennte20k-
  Resumes;135 Rohdokumente,100 wiederholte und20 Inprozess-Artefaktpaare exakt.
  Fünf Originalmodellfälle erreichen das Ziel bei1224; Normal hat181 Bewohner,
  Tax178, Unterhalt204, Bau1083 und Funds10891. Keine Budget-/Stockinjektion.
- Legacy unter `oe00/oe27-final4-verification`:72 SaveDocuments/Snapshots und
  1485 Policyfälle literal identisch;167 vollständige Manager/165 vorbereitete
  Karten/dieselben2 Konflikte. Regeln7/8/9 behalten17 Save-/16 Reportpaare.
  Zehn finale Software-SDL-Bilder PID52591 tatsächlich geöffnet,30 Begleit-
  artefakte gebunden; alle PNGs bytegleich zum vorherigen gültigen Rendering.
  Sechs CLI-Ausgaberefusals und zehn Play-Optionsfälle PASS. Software-/authored
  Szenen sind keine native/human oder Originalgrafik-Abnahme.
- Tatsächliche final3-FAILs bleiben erhalten: alle drei Suiten226PASS/2Industrie-
  FAIL/0Skip. Der späte60Funds-Testkauf ignorierte echten Unterhaltsschuldenstand;
  früher bezahlter Kauf/Abriss und spätere atomare Kaufverweigerung erhalten
  beide Nachweise. Güterketten und vollständiger20k-Replay bleiben unverändert.
- Unabhängiger echter Red-Nachweis: einziger geänderter Steuerzeitpunkt
  8976→9792 in einem echten10000er Save wurde trotz belegter Brandperiode
  9740–10340 akzeptiert. Neuer Financeguard bindet die letzte natürliche
  Brandperiode; echte verzögerte Käufe prüfen Zündung/Innen/Ablauf/nach Ablauf.
  Exakte alte Eingaben am neuen Release: Kontrolle+20k PASS und alle fünf
  vollständigen Ausgaben bytegleich; identische Manipulation vor Fortsetzung
  verweigert. Deshalb sämtliche final4-Builds/Suiten/Nebenläufe neu und frisch.
  Vorbereitete reine Test-Korrekturhelfer wurden nicht ausgeführt/gezählt.
- Initiale Compiler-/Runner-/Archivindex-/private Reportschemafehler,
  supersedierte final1/2 und die synthetische riesige Nullraten-Gegenprobe
  bleiben benannt. Eine spätere Scope-Klarstellung im privaten alten Report
  hatte dessen Fingerprint verändert; exakt12.765 ursprüngliche Bytes wurden
  mit dem eingefrorenen SHA3100831a… wiedergewonnen. Beide Fassungen und
  Recoveryprovenance sind gebunden, keine historische Ausführung neu behauptet.
- Finale Quelle556 Dateien:
  `4f98327153c9d2cff94ee13ca8d21be0e9943342542d7517fcaee5057839baf1`.
  Unveränderte Installation1464 Dateien/819711091 Bytes samt Modi/mtime.
  Abschluss `oe27/validation/final-validation.json`, SHA256
  `c8028074f2a8e3d05dd7181db49ce0e84045c0201951ad64bab9ffbab1ec9bcc`.
  **Gate D technisch PASS**, Gesamt-Originalparität PARTIAL; bekannte höhere
  Originalstufen, Collector, Original-Haussprites29 und native/human bleiben
  offen. Nur eigener lokaler OE27-Commit, kein neuer Push/PR/Merge/Tag/Release.

## OE-28 – Original-House-Inspector und begrenzte Overlays

- Nutzerfortsetzung2026-10-20 am sauberen OE27-HEAD
  `eb40408a57038d775dfa79d3c8e9f1bbf189f80d`; vorheriger SHA oben nachgetragen.
  [Vertrag](../original-economy/housing-inspector-runtime.md) vor Codeänderung,
  Rule10/Schema26/Policy3/Revision27 und alte Authority unverändert.
- Vier sichtbare Houseansichten State/Needs/Goods/Visits; Maus/Pages/Scroll,
  eigener F10-Modus Off/Supply/DES/Risk, höchstens20 Häuser/320 Ownerzellen.
  Worlddiagnosen statt zweiter Simulation, reale Timer/Besuche getrennt von
  Quellenexistenz; Modellbedarf/relative numerische Prozentwerte, Quality,
  aktuelle Blocker, Scheduler-Slots und bestehende Taxdetails sichtbar.
  Hauslieferung/Konsum und Stadtproduktion/Reservierung/Fracht sauber getrennt.
- Gegenüber der hypothetischen Forkansicht keine Mindestbewohner-/Qualitäts-
  oder Wiederbesuchsannahmen; keine neuen Forkimports. Die bestehende
  World-/Produktionsauthority und gekürzte Transferhistorie bleiben unverändert.
- Drei frische gezielte Builds und93 CTests PASS,0 Fehler/Skips/Warnungen;
  tatsächliche Befehle und Zeiten in `.local/original-economy/oe28/` und
  [Abnahme](../original-economy/housing-inspector-validation.md). Neue Tests:
  `original-house-inspection`, `original-house-inspection-real`,
  `original-house-inspector-view`. Kein vorgezogener vollständiger Gate E.
- Tatsächlicher SDL-Negativfall: verfügbare Quelle ohne Ankunft/Timer bleibt X.
  Bezahlte positive Common-/Elite-/Food-/Waren-/Besuchsfälle; F5/F9,
  Maus/Keyboard/Help/F3/Fokus/Release/Resize/HiDPI/F2/F6/F8 bestehen.
  100 wechselnde UI-Frames halten sämtliche zwölf Arbeit-/Authorityzähler0
  und alle Snapshot-/Cache-/Texturwerte exakt. Viewentscheidungen bleiben
  aus Saves ausgeschlossen und starten in einer neuen Sitzung State/Off.
- 22 PNGs mit echten Begleitsaves erzeugt, zehn1×/2×/4×-Bilder geöffnet.
  Beide alten Sandbox-Software-BMPs bytegleich; unveränderte City16-Tests grün.
  Zwei getrennte20k-Prozesse liefern vier volle SaveDocuments und vier
  Berichte bytegleich zur OE27-Kontrolle. Keine neue vollständige72/167-
  Corpusabnahme behauptet. Original1464 Dateien mit Bytes/Modi/mtime erhalten.
- Der finale Report bindet560 Sourcepfade; erste Warnungs-/SDL-Testversuche
  bleiben erhalten, ihre konkreten Korrekturen stehen in der Abnahme.
  **OE-28 technisch PASS**. Gesamtparität PARTIAL, native/human NOT_RUN,
  Originalsprites29 und Gate E30 bleiben offen. Nur eigener lokaler Commit;
  kein neuer Push/PR/Merge/Tag/Release.

## OE-29 – belegte native Hausbilder und sichere Stufenabbildung

- Sauberer Start15dd6da, getrennte Originalsetter-/SG3-/Frontanker-Evidenz vor
  Zuordnung.15 native Typ3..17-Bilder als metadata-only optionale Ergänzung;
  tatsächlicher Worldtyp und Körperseite wählen nach Upgrade/Downgrade/F9.
  Vacant2 bleibt wegen Constructor-/Settermehrdeutigkeit Fallback, leerer
  Eliteplot2×2 übernimmt kein Original4×4-Bild. Statische Variante0 ist benannt.
- Bestehende Legacy13 Rollen/12 Bilder und0/1/2-Stufen unverändert; ein
  physischer Pool, atomarer Append ohne doppelte Core-Uploads und alte Budgets.
  Gemeinsame bestehende Registrierung, kein zweiter Bildindexübersetzer.
  Typ→Gruppe separat gegen kohärente Vertauschung geschützt. Benutzerdefinierte
  Art ist ausdrücklich custom, unbekannte Daten aktivieren keine Builtinidentität.
- 174 finale CTests in drei frischen Konfigurationen PASS,0 Fehler/Skips/Warnungen.
  Bezahlte Common2→3→4→5/Elite11→13→12→13; native Pixel/Alpha/Picking/Culling/UI,
  Walkerfüße, gleicher bezahlter Rule10-Rampenkontext, authored Foreground,
  F4/F6/F7/Snapshot/F5F9.100 reine Frames halten alle zwölf Zähler0.
- 114 neue PNGs samt Schema26-Saves, zwölf finale Releasebilder tatsächlich
  geöffnet; Originalexport30 Bilder/sechs geöffnete Blätter separat. Alle22
  alten House-Inspector-PNGs und zwei Sandbox-BMPs je Konfiguration bytegleich.
  Keine neue volle Corpus-/20k-Abnahme; alte OE28-Artefakte unverändert.
- Die anfängliche57/58-Inaktive-Copyregression wurde mit unverändertem Test
  korrigiert; sämtliche finalen Läufe binden denselben eingefrorenen564-Datei-
  Stand.127 Authorityquelldateien, alle Original1464 Dateien/Bytes/Modi/mtime
  und Rule10/Schema26/Policy3/Revision27 unverändert. Exakte Befehle/Resultate
  und Fehlversuche in der verlinkten Abnahme und privaten Reports erhalten.
- **OE-29 technisch PASS**, ursprüngliche individuelle Variante/ganze Szene
  UNKNOWN, Gesamt-Originalwirtschaft PARTIAL, native/human NOT_RUN. Gate E
  gehört zu30; OE30 bleibt TODO. Nur eigener lokaler Task-Commit.

### OE-30 – 2026-10-20, Basis64cbfcb7

- Optionaler Menüeintrag O, fünf session-only Difficulties, Paid4/Empty und
  ausdrücklicher manueller Rule10-Load; alte New-Sandbox-Defaults, SettingsV1,
  Actionnummern/Koordinaten und Original7–9-Sperren unverändert. F5/F9 bleibt
  sichtbar manuell; keine Original-AutosaveController oder Recoveryfreigabe.
- Der authored Plan sucht legalen ebenen24×13-Platz und kauft ausschließlich87
  normale Commands beiTick0. Reale Xia/Normal:1404 bezahlt,10596 Funds,
  0 Einwohner/Bestände. Vier Häuser steigen bei3559 auf; Hemp13/Ceramics4
  verbraucht. Tatsächliche Straßenreparatur kostet8 und erzeugt neue Ankunft4319;
  Ancestor-Ausfall ergibt belegte bewohnte5→4-Rückentwicklung6721, Erholung6823.
  Erlaubter neutraler Quellenabriss ohne Refund und atomische Hausabrissverweigerung,
  Pausen/Unterhalt und unabhängiger Workerprioritäts-/ID-Oracle geprüft.
- **Debug237/237, Release237/237, ASan+UBSan237/237 PASS**, insgesamt711 volle
  finale CTests,0 Fehler/Skips; vorher tatsächliche Inventare, finale Builds
  ohne Warnungen. Identischer571-Datei-Freeze über alle finalen Läufe. Der
  Apple-arm-Host verweigert detect_leaks=1 tatsächlich; Memory-ASan und UBSan
  laufen mit halt-on-error/detect_leaks=0. Kein Leakdetektor-PASS behauptet.
- Sechs echte separate Creator-/Resumeprozesse setzen6823→26823 fort:
  15 ganze Restartdateipaare und32 Rohdateien über drei Konfigurationen exakt.
  Endfunds1500/Population167, alle Waren-/Food-/Cargo-/Reservationsbilanzen erhalten.
  Weitere sechs produktive Menüprozesse prüfen normale SDL-Clicks/TickDriver,
  Road-Build/Remove/Repair, tatsächlichen Courier, F5/F9 und160-Tick-Neuprozess-
  Fortsetzung.51 finale Softwarebilder bytegleich, elf Releasebilder unabhängig
  geöffnet; Root öffnete zusätzlich sieben Bilder. Neun Zusatzprozesse prüfen
  Minimum900×600/VeryHard-Prozesssave; neun Pfadschutzproben korrekt verweigert.
- Neuer ausführbarer Baselinevergleich unter
  `.local/original-economy/oe00/oe30-final-verification/`:72 SaveDocuments/
  Snapshots,1485 Policyfälle und167/165/2-Corpus exakt, elf Rampen-/zehn Bridge-
  Neuprozesspaare.127 alte Authorityquellen und1464 Originaldateien/819711091Byte
  in SHA/Modus/mtime unverändert. Frühere Testannahmen und Fehlversuche sind
  ehrlich im privaten Log und [Abnahmebericht](../original-economy/paid-starter-validation.md)
  erhalten; dafür keine Runtime oder Assertions gelockert.
- Finalreport `.local/original-economy/oe30/final-validation.json`, SHA256
  `7294e0ad54030cbca9547136488221cd1c69c63ba384cbfca6cfda4c9109c94d`.
  **OE30/Gate E technisch PASS**; Rule10/Schema26/Policy3/Revision27 erhalten.
  Nicht-Haus-Art/Originalparität PARTIAL, native/human NOT_RUN; Recovery31 und
  spätere Prompts unbegonnen. Nur ein lokaler eigener Commit, kein Push/Merge.

### OE-31 – 2026-10-20, Basis324fc8fd

- **Vertrag:** [save-recovery-contract.md](../original-economy/save-recovery-contract.md)
  vor Sourceänderung erklärt; Rule10/Schema26/Policy3/Revision27 unverändert,
  Default-Recovery und Frozen7–9 bleiben gesperrt. Kein weiterer Forkimport.
- **Umsetzung:** ausdrücklicher Recoveryzugang, Common-/Elite-Summary, identische
  Quellen/Version/Difficulty, begrenzte Metadaten und atomare Historien/Branches.
  Drei reale Red/Green-Probes belegen Mixed-Difficulty, rückwärtige Zeit und
  unlesbares Sequenzende; gute Geschwister bleiben nutzbar. Eine vierte Probe
  belegt Capturethrow/Frame-Repeat bei fehlender Livequelle, nun mit sichtbarem
  Fehler und1200t Backoff einschließlich F9-/Toggle-/Commit-Zwischenzuständen.
- **Tests:**63 relevante CTests je Debug/Release/ASan+UBSan bestanden; nach dem
  letzten App-/Controllerfix alle zehn betroffenen Suites je Konfiguration erneut
  bestanden, zusammen219 akzeptierte Konfigurationsläufe/0 finale Fehler/0 Skips.
  Je authored/real2577 Saveverweigerungen plus neun echte verdiente Zustände und
  separate authored4×4-Elite;30 Recovery-Neuprozessfälle mit exakten1/51/408t-
  Weiterläufen, drei private Quellenpaare, echte SDL-F5/F9 und Menü-Restarts.
- **Alte Verträge:**25 repräsentative Schemas mit101 ganzen Dateien je Konfiguration
  bytegleich zu vorab eingefrorenen OE30-Archiven;1–3-Normalisierung bleibt alt.
  Zusätzlich72 SaveDocuments/Snapshots,1485 Policies und167/165/2 unverändert.
  Keine Savecodec-/World-/Map-/Rendereränderung. Alle1464 Originaldateien mit
  SHA/Modus/mtime exakt;819711091Bytes. Keine persönlichen PrefPaths/alten Saves.
- **Beleg:** `.local/original-economy/oe31/final-validation.json`,21458Bytes,
  SHA256 `8c479748c8c908c6194aeff0be0fd81fdd656220447a8a362b62320d15ed90ad`;
  Details einschließlich aller erhaltenen roten/unterbrochenen Vorläufe stehen in
  [save-recovery-validation.md](../original-economy/save-recovery-validation.md).
  Finaler577-Pfad-Freeze3 `aa5262d4f5db57a87ba6afecde402dac796617b8d932a97a0dc4ea00ad123b7e`.
- **Grenzen:** keine volle241er-/Gate-F-/100k-Abnahme; native/human NOT_RUN,
  LSan auf diesem Host nicht unterstützt und deshalb nicht behauptet.
  Einige Originalobjektgrafiken bleiben Fallbacks; Gesamtparität PARTIAL.
  Nur lokaler OE31-Commit, kein Push/Merge/Tag/Release; OE32 unbegonnen.
