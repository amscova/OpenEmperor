# CODEX_ORCHESTRATOR – Auftrag für Codex

Du implementierst die Aufgaben in `docs/codex-original-economy/prompts/` in **exakt** der Reihenfolge von `IMPLEMENTATION_ORDER.md`. Ziel ist ein optionales, in die echte OpenEmperor-Simulation integriertes Originalwirtschafts-Regelprofil, das funktional klar über den überwiegend isolierten Fork-Regelprototyp hinausgeht.

## Steuerung

Die Nutzerfortsetzung vom2026-10-17 beauftragt alle noch offenen OE-14–33
ohne Wartepunkt für manuelle Playtests. Zusätzlich gilt der
[Ausführungsvertrag](../original-economy/completion-contract.md): aktive
Rule10/Schema26 mit strikter Entwicklungsrevision, alte1–9/1–25 eingefroren,
ein getesteter Commit je Task und gemeinsamer PR/Merge nach Endabnahme.
Historische Rule7-Bezeichnungen in den noch offenen Prompts meinen deren
Originalwirtschaftsfunktion im ausdrücklich gewählten Nachfolger.

1. Prüfe `git status --short`, aktiven Branch und aktuellen Upstream-HEAD. Wenn Nutzeränderungen vorhanden sind, fasse sie nicht an.
2. Lies `AGENTS.md`, `README.md`, `QUALITY_GATES.md`, `FORK_DELTA.md`, `PROGRESS.md` und für jeden Task genau dessen Markdown-Datei.
3. Ermittle den ersten noch nicht abgeschlossenen Task. **Führe jeweils nur einen Task vollständig aus.** Arbeite danach nur weiter, wenn sein PASS/Commit und der relevante Gate-Status gesichert sind.
4. Erzeuge zu Beginn einmalig den Feature-Branch `feature/original-economy-v1`, sofern er nicht bereits existiert und keine uncommitteten Fremdänderungen betroffen sind.
5. Code, belastbare Unit-/Integrations- und negative Tests, Original-Provenance, Dokumentation und eigener Commit gehören zu jeder Aufgabe. Keine kosmetischen PASS-Etiketten.
6. Commit-Format aus Prompt übernehmen (`feat(original): [xx] ...` etc.). `PROGRESS.md` und `DECISIONS.md` im **selben** Task-Commit aktualisieren. Git-Commit-SHA lässt sich nicht innerhalb seines eigenen Inhalts zuverlässig verewigen; SHA nach dem Commit in der Antwort nennen und bei nächstem Task in `PROGRESS.md` nachtragen.
7. Am Ende jeder Antwort: Task-ID, Status PASS/FAIL/BLOCKED, Belegtests, Commit-SHA, tatsächliche Dateien und nächster Task. Keine ungesicherten Erfolgsaussagen.

## Harte Grenzen

- Der Fork `amscova/windows-port` ist **nur** eine Quell-/Vergleichsreferenz. **Keine Windows-Integration**, keine pauschalen Cherry-Picks, kein fremdes README-Replace.
- Bestehendes `main`, City-v16 Rule4/5/6, MapPermissions, Bridge/Ramp-Rendering/Authority und Save-Schemas 1–22 bleiben exakt. Neuen Regelstand niemals automatisch aktivieren.
- Für OE-12 gilt die konkret angenommene Ausnahme in `DECISIONS.md` und dem Quellenvertrag V1: eigener Rule8-/Schema24-Entwicklerpfad, zusätzlich Rule7/Schema23 exakt erhalten. Keine automatische Freigabe späterer Wirtschaftsregeln.
- OE-13 besitzt die zusätzliche ausdrückliche Freigabe des Warenvertrags V1: eigener Rule9-/Schema25-Entwicklerpfad, alle früheren1–24-Saveverträge erhalten, echte Waren vorerst0/unsupported. Kein vollständiger Warenkreislauf oder vorgezogenes Gate C.
- In der neuen Regelgeneration: `Model/EmperorBuildingModels.txt` read-only zur Laufzeit laden, mit überprüfter Quelle/Fingerprint; Modellwerte nicht in Git oder alte Saves kopieren.
- Eigene Simulation darf fehlende Waren/Services nicht mit Fake-Boolean, Gratisinventar, visuellen Walkern oder Test-only World-Mutationen ersetzen.
- Unverifizierte Regeln als experimentell markieren; negative Ergebnisse behalten. Keine Behauptung vollständiger Originaltreue ohne reproduzierbare Originalquelle.
- Originaldaten/Sprites niemals ins Repository oder Bundle übernehmen. Bild-/Audit-Artefakte ausschließlich unter `.local/`.
- Lokale Commits sind durch die aktuelle Nutzerbitte ausdrücklich erlaubt, auch wenn ältere AGENTS.md-Arbeitsnotizen noch `no commit` sagen. Der spätere [Merge-Handoff](DECISIONS.md#merge-handoff--ausdrückliche-nutzerfreigabe-am-2026-10-16) erlaubt für diesen Branch außerdem Push, PR nach `main` und regulären Merge nach bestandenen Prüfungen. Alle übrigen AGENTS-Invarianten gelten. Keine Commits ohne relevante abgeschlossene Tests, keine abgeschwächten Tests, kein `git reset --hard`, `git clean -fd`, Stash von Nutzeränderungen, erzwungenes Ref-Update, Squash, Branchlöschen, Tag oder Release.
- Wenn Originalquelle oder Pflicht-Abnahme fehlt: **BLOCKED, STOP**. Nicht einfach 30 Schritte überspringen oder ein Fake-PASS behaupten.

## Start

Erledige **OE-00** gemäß `IMPLEMENTATION_ORDER.md`. Ab OE-01 gilt dasselbe Muster. Es ist ausdrücklich erlaubt, die Verarbeitung nach jedem Commit anzuhalten, damit die Änderungen geprüft werden können. Erstelle keine Arbeiten „im Hintergrund“ ohne tatsächlich aktive Ausführung.
