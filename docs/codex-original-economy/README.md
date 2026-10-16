# OpenEmperor Original Rules – Codex-Arbeitspaket

**Ausgangsvergleich:** `oliverjessner/OpenEmperor` main `f7a5035e92a9` (Stand der Analyse: 2026-10-16) gegen `amscova/OpenEmperor` windows-port `d861d2912337`. Die tatsächlichen HEADs vor jeder Umsetzung erneut prüfen.

**Ziel:** Eine **produktive, optional aktivierbare und versionierte Originalwirtschaft**, die mehr liefert als die bereits im Fork vorhandenen isolierten, überwiegend nur lesenden Regelfunktionen. Windows-Portierung ist **nicht** Bestandteil.

## Sofort beginnen

1. ZIP im Repository-Root entpacken (sie enthält bereits den Ordner `docs/codex-original-economy/`), sodass `docs/codex-original-economy/IMPLEMENTATION_ORDER.md` existiert.
2. In Codex `CODEX_ORCHESTRATOR.md` als Auftrag übergeben. Beim ersten Lauf nur `prompts/00-...md` bearbeiten lassen.
3. Je Prompt ein eigener Review-fähiger Commit. Der spätere ausdrückliche [Nutzer-Handoff](DECISIONS.md#merge-handoff--ausdrückliche-nutzerfreigabe-am-2026-10-16) erlaubt für `feature/original-economy-v1` außerdem Push, PR nach `main` und regulären Merge nach bestandenen Prüfungen. Tags und Releases bleiben ausgeschlossen; die übrigen Schutzregeln gelten weiter.
4. `PROGRESS.md` und `DECISIONS.md` nach jedem Schritt mit Resultaten, SHA und Blockern fortschreiben; diese Dateien gehören jeweils in den Task-Commit.
5. Bei Blockern **stoppen**. Nichts als PASS verbuchen, was nicht getestet wurde.

## Paketinhalt

- `IMPLEMENTATION_ORDER.md`: ausführbare Reihenfolge und Abhängigkeiten
- `CODEX_ORCHESTRATOR.md`: vollständige Codex-Arbeitsanweisung inklusive Git-Strategie
- `QUALITY_GATES.md`: Phasen-Gates und harte Nichtregressionskriterien
- `FORK_DELTA.md`: verifizierter Funktionsvergleich ohne Windows
- `PROGRESS.md`: Status- und Commit-Nachweis
- `DECISIONS.md`: architektonische und Originaltreue-Entscheidungen
- `prompts/00-..33-*.md`: 34 sequenzielle Aufgaben

**Wichtig:** Dieses Paket beschreibt Codex-Arbeitsaufträge, keinen fertigen Programmcode. Jede neue Version wird erst bei lokalem Testnachweis freigegeben.

## Laufender Arbeitsstand

Der tatsächliche Start von OE-00 ist `fef111deff4cedd3d3fa48663556d41b2e4b31a6`;
der obige Vergleichsstand bleibt als Herkunft des gelieferten Plans erhalten.
Maßgebliche Statusdateien sind die hiesigen `PROGRESS.md` und `DECISIONS.md`.
Die ebenfalls gelieferten drei Root-Kopien werden nicht als zweites Statusregister
fortgeschrieben.

`MANIFEST.json` beschreibt die **unveränderte Anlieferung** der42 Markdown-Dateien.
Es ist kein laufender Repository-Lock: Fortschritt, Entscheidungen und belegter
Forkvergleich werden gemäß Auftrag geändert. Die initialen Hashes werden nicht
umgeschrieben, um diese Herkunft zu verdecken. Der separate
[`baseline-lock.json`](../original-economy/baseline-lock.json) bindet die tatsächlich
erzeugten Legacy-Verträge von OE-00; die vollständigen privaten Belege bleiben
unter `.local/original-economy/oe00/`.
