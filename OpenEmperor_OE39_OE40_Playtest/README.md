# OpenEmperor: OE39 → OE40 → Systemweiter Playtest

**Warum diese Reihenfolge?** OE38 hat eine real bezahlte Common10-Hausentwicklung bestätigt, aber später 18 Rückentwicklungen wegen Hemp-/Ceramics-Lieferlücken beobachtet. Für Elite17 gibt es die streng belegte Obergrenze DES79 gegenüber dem Bedarf DES80. Vor weiteren großen Spielfeatures sollen beide Ursachen untersucht und das ganze bereits angebotene Produkt tatsächlich getestet werden.

**Vorabstand bei Erstellung:** GitHub `main` `81bc9218958f7ade116fc43d3df80975c1206e67` (25.10.2026). Codex muss beim Start den tatsächlichen lokalen Stand neu ermitteln.

| Datei | Zweck |
|---|---|
| `CODEX_ORCHESTRATOR.md` | Ein einziger Startauftrag für Codex |
| `EXECUTION_ORDER.md` | Abhängigkeiten, Branches, Gates, Stop-Kriterien |
| `OE39-original-market-distribution.md` | Bezahlte Güterverteilung und Common10-Retention |
| `OE40-original-desirability-sources.md` | Originale Einflussquellen, DES79/80, optionale neue Rule |
| `OE41-systemweiter-playtest.md` | Kompletter Produkt- und End-to-End-Testauftrag |
| `OE41-playtest-matrix.md` | 20 überprüfbare Spielszenarien |
| `OE41-manual-checklist.md` | Tatsächlich durch den Nutzer zu absolvierende Mac-Tests |
| `OE41-playtest-report-template.md` | Berichtsschema und priorisierte Findings |

## Benutzung

Die Markdown-Dateien als Arbeitspaket innerhalb des **lokalen Repositories** unter dem ignorierten Pfad `.local/codex/oe39-oe41/` ablegen (oder einen anderen nicht zu committenden Prompt-Pfad wählen). Codex dann direkt mit dem Inhalt von `CODEX_ORCHESTRATOR.md` beauftragen.

Für kontrollierte Abschnitte kann der Nutzer jeweils nur einen der drei OE-Prompts nacheinander starten und nach jedem Gate/Commit prüfen, ob der Umfang stimmt. Diese Variante vermeidet, dass Codex bei langer Auftragskette die Entscheidungsgrenzen überspringt.

**Wichtig:** Die Erstellung dieser Prompts ist noch **keine** Implementierung von OE39/OE40 und **kein** ausgeführter Playtest. Die Ausführung erfolgt erst in der lokalen Entwicklungsumgebung mit Source, Compiler, Spielinstallation und GUI-Zugriff. Die `.local`-Artefakte dürfen nicht mit der Codebasis oder Originaldaten gebündelt werden.
