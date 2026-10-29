# OpenEmperor – verbindliche Reihenfolge OE-39 → OE-40 → OE-41

**Ziel:** Erst nachweislich bekannte Spiellücken beheben, danach alle **tatsächlich angebotenen** Funktionen als Ganzes durchspielen und bewerten. Nicht durch weitere Funktionswünsche vom End-to-End-Test ablenken lassen.

## Basis und Arbeitsregeln

Vor Ausführung `AGENTS.md`, `KNOWN_ISSUES.md`, `docs/testing.md`, lokalen Git-/Remote-Status, Datenverfügbarkeit sowie tatsächliche finale Rule/Save-Versionen aufnehmen.

Referenz bei Erstellung: GitHub-`main` @ `81bc9218958f7ade116fc43d3df80975c1206e67`. **Nicht blind zurücksetzen**: falls lokal neuere Commits existieren, diese prüfen und erhalten. Alle Änderungen ausschließlich mit eigenen Branches und echten lokalen Git-Commits für bestandene Gates; keine Pushes, Merges, Tags, Releases oder ungefragte Änderungen an persönlichen Save-/User-Ordnern.

`.local/` für alle größeren Resultate, Originaldaten, Screenshots, generierten Testsave-Dateien und Source-Binary-Proben. Keine proprietären Originaldateien, extrahierten Modelldumps oder dekompilierten Implementierungen in Git. Mit User-eigener Original-Installation read-only arbeiten.

## Gate-Reihenfolge und Freigabe zum Weitermachen

| Reihenfolge | Arbeitsdatei | Erforderliches Resultat | Nächster Schritt |
|---|---|---|---|
| 1 | `OE39-original-market-distribution.md` | Erst Kausalbeleg; dann, falls nötig, versionierte Lieferpolicy und echte Common10-Retention | Zu OE40, auch wenn OE39 `PARTIAL/BLOCKED` ist, dann ohne unbewiesene Änderungen |
| 2 | `OE40-original-desirability-sources.md` | Source-Lifecycle/Original-DES Beweispfad; nur bei ausreichender Evidenz neue Variante implementieren | Immer zu OE41; kein künstliches +1 und keine „bereinigte“ falsche DES-Obergrenze |
| 3 | `OE41-systemweiter-playtest.md` | 20 durchgeführte oder ehrlich als `BLOCKED/NOT_RUN` ausgewiesene Szenarien, native/manuelle Evidenz getrennt | Ergebnisse priorisieren, **noch keine OE42-Featureentwicklung** |
| 3a | `OE41-playtest-matrix.md` | Vorgegebene Szenarioabdeckung und Belegpflicht | In Bericht übernehmen |
| 3b | `OE41-manual-checklist.md` | Offen bleibende Prüfungen für echten Nutzer | Mensch ergänzt Ergebnisse; Codex füllt sie **nicht** selbst aus |
| 3c | `OE41-playtest-report-template.md` | Dokumentation mit Repros, finanziellen/laufzeitlichen Fakten und Release-Urteil | Konkretes Fehler-/Folgetask-Backlog |

## Branch- und Commit-Vertrag

1. **OE-39** von aktueller, überprüfter `main`-Basis auf `feat/oe39-market-retention` (oder bestehenden korrekten Arbeitsbranch fortsetzen). Pro bestandenem Gate A/B/C ein lokaler Commit, soweit code-/dokumentationsseitig geändert. Kein blinder Branch-Neubeginn mit Verlust lokaler Änderungen.
2. **OE-40** vom tatsächlich **technisch akzeptierten** OE-39-Abschluss-HEAD auf `feat/oe40-original-desirability`. OE39 darf noch ungemergt sein; **nicht** auf einen älteren Remote-`main` zurückspringen, falls OE39 brauchbare Änderungen mitbringt. Wenn OE39 kein technisch belastbarer Code-HEAD ist, nur von dessen unveränderter guter Baseline ausgehen. Belegter Forschungserfolg kann ein eigener Dokumentations-Commit sein; Implementierung/Elite17 getrennt und nur bei Proof.
3. **OE-41** auf `test/oe41-system-playtest` von der **letzten technisch testbaren, source-/save-konsistenten** Basis. Falls OE40 blocked oder OE39 partial: ein End-to-End-Test des noch vorhandenen Rule12 (+ ggf. gültiger OE39-Veränderungen) ist Pflicht. Keine künstlichen Feature-Claims oder Migration.
4. Abschließend vollständige Git-SHAs, Build-/Testbefehle, tatsächliche Arbeitsergebnisse/Blocker und nächste drei **aus dem Playtest abgeleiteten** Fixes nennen.
5. Ohne expliziten Nutzerauftrag **niemals pushen, mergen oder releasen**. Die lokale Branch-Kette bewusst dokumentieren, damit später kein Entwicklungscommit verloren geht.

## Definitionen (niemals vermischen)

- `IMPLEMENTED`: produktiver C++-/World-Pfad mit passender Version und Save-Autorität.
- `TESTED`: konkrete Ausführung mit belegtem Resultat, nicht bloß vorhandene `tests/*.cpp`.
- `PLAYABLE`: echte Menü-Stadt-Simulation mit bezahlten Commands und beobachteten Ereignissen.
- `ORIGINAL_VERIFIED`: nur genau die durch Originalquelle/EXE begrenzt belegte Mechanik.
- `HUMAN_PASS`: ausschließlich nach ausdrücklicher menschlicher Beobachtung.

Eine weiter laufende, stabile Rule12-Stadt ist nicht gleich vollständige Originalspielparität. OE39-/OE40-Fehlschläge bei noch fehlender Quelle sind kein Grund, den systemweiten Spieltest nicht auszuführen. Alte Save-Versionen bleiben ohne automatische Migration.

## P0-Stopps

Bei Datenverlust-/User-File-Risiko, Source-Mutation, fehlerhaftem canonical Save/Restore, duplizierbarem Goods-Transfer oder einem tatsächlich reproduzierten Crash mit gespeicherter Authority: Betroffenes Gate **FAIL** melden; keine Veröffentlichung. Die übrigen sicheren, unabhängigen Playtest-Szenarien nur aus frischem isoliertem App-Root weiterführen. Nie durch Lockerung der Validatoren ein falsches grünes Ergebnis erzeugen.

## Abschließendes Erfolgskriterium

Ein guter OE41-Report erklärt konkret:

1. **Was kann man im aktuellen Spiel live tun und wie gut funktioniert es?**
2. **Welche dokumentierten Funktionen sind noch nicht integriert, obwohl Prototyp/Parser/Tests existieren?**
3. **Welche drei bis fünf reproduzierbaren Defekte verhindern ein breiteres Alpha-Spielerlebnis?**
4. **Welche bislang angenommenen Fortschritte waren bloß Unit-/Software-Pixel-Nachweise und noch kein nativer Nutzer-Playtest?**
5. **Sind Quellen, Builds, private Saves, historische Schemas, Goods und Funds weiterhin integer und geschützt?**
