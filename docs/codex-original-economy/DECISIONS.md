# DECISIONS – verbindliches Architektur- und Evidenzregister

Dieser Stand enthält zunächst **Planungsentscheidungen**, keine bestätigten Implementierungsergebnisse. Codex ergänzt nach jedem Task Belege und Review-Status.

| ID | Anfangsentscheidung | Status / Beweisbedarf |
|---|---|---|
| D-01 | Kein globaler Merge des `amscova/windows-port` Forks; selektiv unabhängig testen | plan |
| D-02 | Original Model-Datei bei Rule7 read-only/mit Hash einbinden | plan |
| D-03 | City-v16 Rule7 / MapPermissions policy3 / Schema23 als geplanter, opt-in Weg | OE-00: am tatsächlichen HEAD frei; für spätere ausdrücklich gewählte Regeln reserviert, in OE-08/OE-09 erneut prüfen |
| D-04 | Alte Save-Schemas 1–22 unverändert und keine Auto-Migration | zwingende Invariante |
| D-05 | Brücken/Rampen Rule6 von Rule7 weiter benutzen, ihre Authority aber nicht ändern | Abhängigkeiten prüfen |
| D-06 | Original-Model-IDs nur nach eindeutiger Quelle als VERIFIED führen | einzelne IDs offen |
| D-07 | Originalspiel-Simulationsschritte nicht automatisch mit 20Hz World-Ticks gleichsetzen | Zeitskala versioniert/prüfen |
| D-08 | 15 House-Zeilen sind nicht gleich dem aktuellen World-Level 0..2 | eigene Authority erforderlich |
| D-09 | Goods inklusive Alternativen nur mit Realproducer/RealCourier/Conservation | Bedarfssemantik prüfen |
| D-10 | Versions-Gates bis zu einer echten paid Hut->Plain-Cottage-Szene nicht im Standardmenü anbieten | ab OE-21 nachweisen |

## Neue Entscheidungen

Zu jeder Entscheidung bitte ergänzen: Task-ID, Commit, Originalbeleg (`raw`, `exe`, `model` oder `authored`), begründete Alternative, Konsequenz für Save/Tests, Review-Status.

### OE-00: tatsächliche Ausgangslage und Arbeitsgrenzen

- **Revision:** `fef111deff4cedd3d3fa48663556d41b2e4b31a6`, nicht der historische
  Paketstand `f7a5035e92a9`. Der zusätzliche Commit ist der geprüfte Hit-Test-Fix.
  Der Feature-Branch startet am tatsächlichen `main`; kein Fork-Merge.
- **Pfad:** Das maßgebliche vollständige Paket ist
  `docs/codex-original-economy/`. Relative Links auf `prompts/` gelten innerhalb
  dieses Ordners. Die drei ebenfalls vorhandenen Root-Kopien bleiben unangetastet.
- **Versionierung:** Aktive City-v16-Regeln1–6, Policies1–3, Save-Schemas1–22
  bestätigt. Rule7/policy3/Schema23 sind die bedingte Architekturwahl für später;
  OE-00 vergibt keine Laufzeitberechtigung. Alte Writer/Parser bleiben exakt.
- **Vererbung:** Rule7 erbt Bridge/Ramp nicht automatisch: explizite Guards in
  `World.h` und Map-/Savepfaden schließen es heute aus. Neue Unterstützung später
  nur gezielt versionieren, nicht die bestehenden5/6-Verträge pauschal erweitern.
- **Evidenz:** Aktuelle Worldpfade sind `AUTHORED FALLBACK`; originale Dateien
  werden read-only identifiziert. Pure Fork-Tests belegen Fork-Verhalten, keine
  neuen `EXE-OBSERVED` Originalmechanismen. `OriginalHousingView` bleibt
  `READ-ONLY INTEGRATION`, nicht echter House-State oder Warenankunft.
- **Baseline:** Vollständige Savebytes und die bereits gegen tatsächliche
  WorldSnapshots geprüften `{rules,world}`-Felder werden lokal eingefroren.
  Hashes aller1485 vorhandenen Policyfälle kommen vom unveränderten
  Produktions-Audit. Keine zweite Simulations-/Save-/Permission-Authority.
- **Review:** Source-/Teststellen in
  [Architekturkarte](../original-economy/architecture-map.md) und
  [Forkvergleich](../original-economy/fork-comparison.md); tatsächliche Abnahme
  und zugehöriger Task-Commit werden im Fortschrittsprotokoll/Report dokumentiert.
