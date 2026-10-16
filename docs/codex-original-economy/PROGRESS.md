# PROGRESS – bei jeder Codex-Aufgabe aktualisieren

**Status-Vokabular:** `TODO`, `IN_PROGRESS`, `PASS`, `FAIL`, `BLOCKED`, `NOT_RUN`. Belege zu jedem PASS echte Tests.

Codex: Einen Task-Commit erzeugen, danach den SHA in der **Antwort** nennen. Bei der **nächsten** Aufgabe die vorige SHA in dieser Datei nachtragen, weil ein Commit seine eigene SHA nicht enthalten kann.

| ID | Status | Commit-SHA (im Folgecommit nachtragen) | Testbelege (Pfad/Run) | Blocker/Notiz |
|---|---|---|---|---|
| OE-00 | PASS | im Folgecommit | `.local/original-economy/oe00/{reference,verification,validation}/` | 45 relevante CTests,13 Harness-Tests,12 Fork-Untertests;72 Saves/Snapshots und1485 Policyfälle wiederholt identisch; keine Simulation geändert |
| OE-01 | TODO | — | — | — |
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
