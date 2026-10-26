# Codex – OE-39, OE-40, danach vollständiger OpenEmperor-Playtest

Du arbeitest als erfahrener C++20/SDL3-Entwickler, Reverse-Engineering-Analyst und QA-Engineer in `oliverjessner/OpenEmperor`.

**Auftrag:** Lies zuerst vollständig `EXECUTION_ORDER.md` und `AGENTS.md`. Danach arbeite die drei Markdown-Aufträge **strikt sequenziell** ab:

1. `OE39-original-market-distribution.md`
2. `OE40-original-desirability-sources.md`
3. `OE41-systemweiter-playtest.md` mit `OE41-playtest-matrix.md`, `OE41-manual-checklist.md` und `OE41-playtest-report-template.md`.

**Start-Checks:** Aktueller lokaler Branch, Git-Status, Remote-Main, nicht veröffentlichte Commits, private Emperor-Datenquelle und CMake/SDL3/macOS-Verfügbarkeit. Nichts überschreiben. Falls eine Datei unter `.local/codex/` liegt, absolute oder relativ zu ihr aufgelöste Pfade verwenden, aber produktive Dokumente wie `AGENTS.md` immer aus dem tatsächlichen Repository laden.

**Commit-Strategie:** Für jedes wirklich bestandene Gate einen eigenen *lokalen* Commit; Ergebnis der vorigen Aufgabe wird Baseline der nächsten. Source-/Save-Versionierung und historische Bytes niemals unbemerkt ändern. Kein automatisches Pushen/Mergen/Taggen/Releasen. Kein weiterer Scope (Militär, neues UI, neue Gebäude) ohne Playtestbefund.

**Wissenschaftliche Beweisführung:** Das bisherige Rule12-DES79/80-Elitehindernis bleibt gültig, bis eine ausdrücklich vollständige gegenteilige Beweiskette vorliegt. Keine numerischen Hacks. Die Warenverteilungsabweichung aus OE38 erst tatsächlich messen, bevor eine Dispatch-Policy implementiert wird.

**Playtest:** Nicht bei einem nur partiellen OE39/40-Ausgang stoppen. Führe OE41 immer auf der letzten konsistenten, unterstützten Version durch. Tatsächlich GUI bedienen, soweit die Ausführungsumgebung es erlaubt; automatisierte SDL/Software-Captures und `HUMAN_PASS` nicht gleichsetzen. Lege offene menschliche Testfälle vor, aber behaupte keinen Menschentest.

**Abschlussmeldung nach jeder Aufgabe:** Baseline SHA, Branch, neue lokale Commit-SHAs, geprüfte Gates, ausgeführte Builds/Tests (inklusive FAIL/NOT_RUN), echtes Verhalten, private Belege/Logs, verbliebene Blocker und nächste Datei. Nach OE41 eine konsolidierte Prioritätenliste P0–P3 und vorläufige Alpha-Release-Einschätzung.
