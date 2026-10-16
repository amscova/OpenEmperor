# IMPLEMENTATION_ORDER – verbindliche Reihenfolge

**34 eigenständige Codex-Aufträge (OE-00 bis OE-33).** Das ist ein strikt sequenzieller Arbeitsplan, keine „alles in einem Commit“-Anweisung.

Jeder Task benötigt die erfolgreichen Vorgänger. Wenn sich Scope/Schema durch einen inzwischen geänderten Upstream verschiebt, entscheidet Codex am Anfang von OE-00 und protokolliert dies **vor** Implementierung; es darf nichts rückwirkend umnummerieren.

## Versions- und Git-Strategie

- Auf einem neuen Branch `feature/original-economy-v1` arbeiten. Wenn der Branch bereits existiert: HEAD/Status prüfen; keinesfalls anderer Leute Änderungen verwerfen.
- Beginn am aktuellen `main`, nicht am Windows-Fork. Fork nur als Referenz für Original-Regeln und testbare Differenzen.
- Geplante **City-v16 Rule7 / MapPermissions policy3 / Save schema23** nur als bedingte Architekturentscheidung. Die freien Nummern zu Laufzeit prüfen; verbindliche Wahl in `DECISIONS.md` festhalten.
- Alte 1–22 Saves **byte-identisch**, insbesondere Rule6-Rampen und Rule5-Brücken. Neue Regeln nie Default, nie automatische Save-Migration.
- Je erfolgreich abgeschlossenem OE-Task **ein Commit**; die aktuelle Nutzerfreigabe ersetzt ausdrücklich alte `no commit`-Arbeitsnotizen aus AGENTS.md für dieses Paket, nicht deren übrige Schutzregeln. Keine leeren PASS-Commits; keine Pushes/Tags/Releases.
- Gewünschte neuen Regeln müssen mit echten kaufbaren Quellen, Dispatcher, Walker-Ankunft, Beständen, Konsum und House-State laufen, nicht nur in einer separaten read-only Ansicht.

## Reihenfolge

### A – Evidenz und reine Regelkerne

| Nr. | Prompt | Benötigt | Ergebnis / Schwerpunkt |
|---|---|---|---|
| OE-00 | [Baseline, Architektur und Fork-Diff einfrieren](docs/codex-original-economy/prompts/00-baseline-architektur-und-fork-diff-einfrieren.md) | — | `docs/original-economy/baseline.md` mit Source-/Testreferenzen und Open-Issues |
| OE-01 | [Originale Modelldatei robust und schreibgeschützt lesen](docs/codex-original-economy/prompts/01-originale-modelldatei-robust-und-schreibgeschuetzt-lesen.md) | OE-00 | Getypte, isolierte ModelData-API mit explizitem Parsing-Fehlerpfad |
| OE-02 | [Model-IDs, Units und Faktenregister festlegen](docs/codex-original-economy/prompts/02-model-ids-units-und-faktenregister-festlegen.md) | OE-01 | `docs/original-economy/model-provenance.md` und getestete typed IDs |
| OE-03 | [Desirability als unabhängig testbaren Original-Regelkern bauen](docs/codex-original-economy/prompts/03-desirability-als-unabhaengig-testbaren-original-regelkern-bauen.md) | OE-02 | SDL-freier Desirability-Kern mit Original-Daten und geprüften Grenzen |
| OE-04 | [Pure HouseEvolution über alle Hausstufen](docs/codex-original-economy/prompts/04-pure-houseevolution-ueber-alle-hausstufen.md) | OE-02 | SDL-freier, deterministischer HouseEvolution-Kern |
| OE-05 | [Service- und Religionstimer isoliert rekonstruieren](docs/codex-original-economy/prompts/05-service-und-religionstimer-isoliert-rekonstruieren.md) | OE-04 | Rollenbasierte pure Service/Religion-Timer-API |
| OE-06 | [Warenbedarf, Lieferungen und Food Quality pur rekonstruieren](docs/codex-original-economy/prompts/06-warenbedarf-lieferungen-und-food-quality-pur-rekonstruieren.md) | OE-02, OE-04 | Pure Waren-/Food-Quality-Regeln mit sauberer Semantik |
| OE-07 | [Fork-Parität als Vergleich, nicht als unkritische Kopie](docs/codex-original-economy/prompts/07-fork-paritaet-als-vergleich-nicht-als-unkritische-kopie.md) | OE-03, OE-04, OE-05, OE-06 | Maschinenlesbarer Paritätsbericht und menschlich lesbare Entscheidungen |

**Gate:** siehe [QUALITY_GATES.md](docs/codex-original-economy/QUALITY_GATES.md). Ohne dokumentierten Gate-PASS nicht in die nächste Phase wechseln.

### B – Versionierung und Runtime

| Nr. | Prompt | Benötigt | Ergebnis / Schwerpunkt |
|---|---|---|---|
| OE-08 | [City-v16 Rule 7 als ausdrücklich opt-in Regel-Generation](docs/codex-original-economy/prompts/08-city-v16-rule-7-als-ausdruecklich-opt-in-regel-generation.md) | OE-00, OE-07 | Expliziter Regeln-Gate `original_economy_enabled()` ohne unbeabsichtigte Legacy-Aktivierung |
| OE-09 | [Striktes neues Save-Schema und getrennte OriginalHouse-Daten](docs/codex-original-economy/prompts/09-striktes-neues-save-schema-und-getrennte-originalhouse-daten.md) | OE-08 | Neue Rule-7-only Persistenz und null implizite Migration |
| OE-10 | [Original-Spielzyklen mit World-Ticks synchronisieren](docs/codex-original-economy/prompts/10-original-spielzyklen-mit-world-ticks-synchronisieren.md) | OE-05, OE-08, OE-09 | Deterministischer Rule-7-Scheduler mit erklärter Zeitbasis |
| OE-11 | [Echte Modelldatei an World-Sessions binden](docs/codex-original-economy/prompts/11-echte-modelldatei-an-world-sessions-binden.md) | OE-01, OE-08, OE-09 | Rule-7-World nutzt genau ein geprüftes OriginalRulesModel |
| OE-12 | [Originales Desirability-Modell produktiv verbinden](docs/codex-original-economy/prompts/12-originales-desirability-modell-produktiv-verbinden.md) | OE-03, OE-09, OE-11 | Rule-7-House-Desirability stammt aus realen Gebäuden und Modelldaten |

**Gate:** siehe [QUALITY_GATES.md](docs/codex-original-economy/QUALITY_GATES.md). Ohne dokumentierten Gate-PASS nicht in die nächste Phase wechseln.

### C – Echte Warenströme und Services

| Nr. | Prompt | Benötigt | Ergebnis / Schwerpunkt |
|---|---|---|---|
| OE-13 | [Typisierte Warenbestände und Conservation-Ledger](docs/codex-original-economy/prompts/13-typisierte-warenbestaende-und-conservation-ledger.md) | OE-06, OE-09, OE-11 | Getypte Rule-7-Warenbuchhaltung über Gebäude und Courier |
| OE-14 | [Vorhandene Pottery-Produktion als Ceramics-Versorgung adaptieren](docs/codex-original-economy/prompts/14-vorhandene-pottery-produktion-als-ceramics-versorgung-adaptieren.md) | OE-13 | Vollständiger Rule-7-Ceramics-Ursprung aus realen Quellen |
| OE-15 | [Markttransport für originale Waren durch reale Courier-Routen](docs/codex-original-economy/prompts/15-markttransport-fuer-originale-waren-durch-reale-courier-routen.md) | OE-13, OE-14 | Real transportierter Bestand vom Markt zum Haus, nicht durch Setzen eines Zustands |
| OE-16 | [Hemp als bezahlten Produktions- und Lieferkreislauf ergänzen](docs/codex-original-economy/prompts/16-hemp-als-bezahlten-produktions-und-lieferkreislauf-ergaenzen.md) | OE-13, OE-15 | Echter Hemp-Kreislauf einschließlich kostenpflichtiger Quelle |
| OE-17 | [Verbrauch exakt nach gespeicherter Original-Zyklusphase](docs/codex-original-economy/prompts/17-verbrauch-exakt-nach-gespeicherter-original-zyklusphase.md) | OE-06, OE-10, OE-13, OE-15, OE-16 | Tatsächlich geliefertes Hemp/Ceramics wird am richtigen Zeitpunkt verbraucht |
| OE-18 | [Food Quality aus einer tatsächlich belieferten Food-Kette](docs/codex-original-economy/prompts/18-food-quality-aus-einer-tatsaechlich-belieferten-food-kette.md) | OE-06, OE-15, OE-17 | Physisch begründete Food Quality statt hypothetischer OriginalHousingView-Nullwerte |
| OE-19 | [Bestehende Service- und Health-Walker als tatsächliche Visits anbinden](docs/codex-original-economy/prompts/19-bestehende-service-und-health-walker-als-tatsaechliche-visits-anbinden.md) | OE-05, OE-09, OE-10 | Gezielter, physisch ausgelöster Service-Timer pro berechtigter Kategorie |

**Gate:** siehe [QUALITY_GATES.md](docs/codex-original-economy/QUALITY_GATES.md). Ohne dokumentierten Gate-PASS nicht in die nächste Phase wechseln.

### D – Housing und Gameplay

| Nr. | Prompt | Benötigt | Ergebnis / Schwerpunkt |
|---|---|---|---|
| OE-20 | [Ancestor-Service: erstes religiöses Gebäude + Walker](docs/codex-original-economy/prompts/20-ancestor-service-erstes-religioeses-gebaeude-walker.md) | OE-05, OE-09, OE-19 | Physisch mögliche Ancestor-Ankunft mit verifizierbarem `timer=40` |
| OE-21 | [Erste komplette spielbare originale Hausentwicklung](docs/codex-original-economy/prompts/21-erste-komplette-spielbare-originale-hausentwicklung.md) | OE-04, OE-12, OE-15, OE-17, OE-18, OE-19, OE-20 | Meilenstein: ein echter, bezahlter und spielbarer Hausaufstieg im normalen OpenEmperor-World |
| OE-22 | [Weitere religiöse Dienste: Confucian, Daoist, Buddhist](docs/codex-original-economy/prompts/22-weitere-religioese-dienste-confucian-daoist-buddhist.md) | OE-20, OE-21 | Drei religiöse Kategorien mit eigenständiger Ankunft und Verfall |
| OE-23 | [Kulturelle und medizinische Originaldienste vollständig anschließen](docs/codex-original-economy/prompts/23-kulturelle-und-medizinische-originaldienste-vollstaendig-anschliessen.md) | OE-19, OE-21 | Alle fünf nichtreligiösen Original-Service-Timer können durch reale, begrenzte Visits aktiviert werden |
| OE-24 | [Luxury-Goods mit bezahlten Kreisläufen vollständig machen](docs/codex-original-economy/prompts/24-luxury-goods-mit-bezahlten-kreislaeufen-vollstaendig-machen.md) | OE-13, OE-15, OE-16, OE-17, OE-21 | Vier Luxuswaren haben vollständige paid Source->Market->Home->Consume Bilanz |
| OE-25 | [15 Original-Hausstufen samt Elite-Fällen produktiv machen](docs/codex-original-economy/prompts/25-15-original-hausstufen-samt-elite-faellen-produktiv-machen.md) | OE-21, OE-22, OE-23, OE-24 | Der Rule7-Zustand unterscheidet tatsächlich alle Hausstufen und deren Voraussetzungen |
| OE-26 | [Große Elite-Häuser: Footprints, Platzbedarf und Transaktionen](docs/codex-original-economy/prompts/26-grosse-elite-haeuser-footprints-platzbedarf-und-transaktionen.md) | OE-25 | Elite-House-Geometrie ist mit World-Occupancy, Painter und Transport konsistent |
| OE-27 | [Schwierigkeitsgrad, originale Haussteuern und Spielziele](docs/codex-original-economy/prompts/27-schwierigkeitsgrad-originale-haussteuern-und-spielziele.md) | OE-02, OE-10, OE-24, OE-25, OE-26 | Originaldatengetriebene Steuern/Bedarfe je Schwierigkeit und spielbare Rule7-Ziele |

**Gate:** siehe [QUALITY_GATES.md](docs/codex-original-economy/QUALITY_GATES.md). Ohne dokumentierten Gate-PASS nicht in die nächste Phase wechseln.

### E – Präsentation und Starter

| Nr. | Prompt | Benötigt | Ergebnis / Schwerpunkt |
|---|---|---|---|
| OE-28 | [Inspector, Bedürfnisse, Qualität und Diagnose-Overlays](docs/codex-original-economy/prompts/28-inspector-beduerfnisse-qualitaet-und-diagnose-overlays.md) | OE-21, OE-24, OE-26, OE-27 | Spieler kann ehrlich sehen, warum ein Haus nicht aufsteigt |
| OE-29 | [Original-Hausgrafiken und sichere visuelle Stufenabbildung](docs/codex-original-economy/prompts/29-original-hausgrafiken-und-sichere-visuelle-stufenabbildung.md) | OE-24, OE-25, OE-27, OE-28 | Belegte Original-Hausstufen visuell korrekt zugeordnet, keine falschen Identitäten |
| OE-30 | [Spielbarer bezahlter Originalwirtschaft-Starter](docs/codex-original-economy/prompts/30-spielbarer-bezahlter-originalwirtschaft-starter.md) | OE-26, OE-27, OE-28, OE-29 | Erste echte spielbare Demonstration einer hausentwickelnden Originalwirtschaft |

**Gate:** siehe [QUALITY_GATES.md](docs/codex-original-economy/QUALITY_GATES.md). Ohne dokumentierten Gate-PASS nicht in die nächste Phase wechseln.

### F – Qualitätssicherung und Fork-Vergleich

| Nr. | Prompt | Benötigt | Ergebnis / Schwerpunkt |
|---|---|---|---|
| OE-31 | [Savegame, Recovery und feindliche Daten vollständig härten](docs/codex-original-economy/prompts/31-savegame-recovery-und-feindliche-daten-vollstaendig-haerten.md) | OE-09, OE-26, OE-29, OE-30 | Strict Rule7 Save/Recovery mit vollständigem Negativkatalog |
| OE-32 | [Longplay, Skalierung und Originalmap-Kompatibilität](docs/codex-original-economy/prompts/32-longplay-skalierung-und-originalmap-kompatibilitaet.md) | OE-24, OE-25, OE-26, OE-29, OE-30, OE-31 | Performance- und Gamebalance-Bericht mit unabhängigen echten Laufresultaten |
| OE-33 | [Endvergleich gegen den Fork und Release-Entscheidung](docs/codex-original-economy/prompts/33-endvergleich-gegen-den-fork-und-release-entscheidung.md) | OE-07, OE-21, OE-24, OE-29, OE-30, OE-31, OE-32 | `docs/original-economy/final-comparison.md` und Übergabe-Checkliste |

**Gate:** siehe [QUALITY_GATES.md](docs/codex-original-economy/QUALITY_GATES.md). Ohne dokumentierten Gate-PASS nicht in die nächste Phase wechseln.

## Strikte Fertig-Definition

Ein Auftrag ist **PASS**, wenn der Code tatsächlich gebaut, relevante positive und negative Tests ausgeführt, die Invarianten nachgewiesen und ein eigener Commit erstellt wurde. Ein LLM-Textbericht oder eine uninspektierte Bilddatei sind kein Beleg.

Ein Auftrag ist **BLOCKED**, wenn Originaldaten, Selektoren, Laufzeitbefugnisse oder reproduzierbare Tests fehlen. Der Blocker wird genau benannt. Ohne explizite neue Entscheidung darf Codex nicht mit einem vermeintlich äquivalenten, aber gefälschten Originalmechanismus fortfahren.

## Fertig gegenüber dem Fork – messbarer Abschluss

1. Original-Modellparser/Desirability/HouseEvolution/Services/Goods mindestens durch unabhängige Vergleichstests abgedeckt.
2. Echte gekaufte Produktionsketten liefern Waren über bereits vorhandene World-Routen und Courier-Arrival an Haushalte.
3. Belegte einzelne Service-/Religionsbesuche beeinflussen echte Timer und die tatsächlich gespielte Hausentwicklung.
4. 15 Modell-Hausstufen als Authority mit belastbaren Bedürfnissen, Elite-Grenzen und Save/Restore; unbewiesene Originalvorgänge explizit als authored/unknown.
5. Volle UI- und Szenariointegration im optionalen Modus. Alte Saves/Rule6, Bridges/Ramps, Original-Map-Permissions unbeschädigt.
6. Debug, Release, Sanitizer, Save-Adversarial, echte Prozess-Restarts, 20k/100k-Ticks und Originalmap-Corpus dokumentiert.
7. Wenn Punkte fehlen: **PARTIAL** statt pauschalem Sieg über Fork oder Behauptung vollständiger Originaltreue.

## Übergabe

Beginne mit [`CODEX_ORCHESTRATOR.md`](docs/codex-original-economy/CODEX_ORCHESTRATOR.md) und [`prompts/00-...`](docs/codex-original-economy/prompts/00-baseline-architektur-und-fork-diff-einfrieren.md).
