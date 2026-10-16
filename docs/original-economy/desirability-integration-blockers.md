# OE-12: produktive Desirability-Anbindung blockiert

**Status: BLOCKED. Keine OE-12-Implementierung, kein Gate-B-PASS, kein Fortgang
zu OE-13.** Ausgangspunkt ist `main`
`df293931bbb8a409668f3790379bb370e6c9f44d` nach OE-11.

Der Nutzer hat ausdrücklich entschieden, die strenge Grenze beizubehalten und
OE-12 mit den ermittelten Blockern zu dokumentieren. Die verbindliche
[Nutzerentscheidung](../codex-original-economy/DECISIONS.md#oe-12-strenge-grenze-und-blockierter-fortgang)
erlaubt keine neue Bau-/Restore-Authority, zusätzliche Funds oder erfundene
World→Model-Zuordnung. Ein Dokumentationscommit dieses Befunds ist keine
Featurefreigabe.

**Entwurf zur Entscheidung:** Der Nutzer hat inzwischen ausdrücklich einen
[konkreten Quellen-/Bau-/Restore-/Finanzierungsvertrag](source-contract-v1-proposal.md)
beauftragt. Dieser empfiehlt Rule8/Schema24, einen eigenständigen authored
Well72-Quellencontainer,2000 feste Start-Funds und einen ausdrücklich geänderten
City-v13-Algorithmusvergleich. Er ist **NICHT FREIGEGEBEN** und ändert keinen
der hier beschriebenen Rule7-Blocker. Auch sein Dokumentationsmerge ist keine
Umsetzungsfreigabe; OE-12/Gate B bleiben BLOCKED.

Der [OE-12-Auftrag](../codex-original-economy/prompts/12-originales-desirability-modell-produktiv-verbinden.md)
verlangt tatsächliche bezahlte Quellen, verifizierte Model-IDs, Quellenbau und
Entfernung, Reload sowie eine Szene mit 20 Häusern. Seine Stop-Regel verbietet,
bei uneindeutiger Identität einen angeblich originalen Einfluss zu erfinden.
Die folgenden drei Voraussetzungen fehlen im unveränderten Produktionsstand.

## 1. Keine belegte World→Model-Quellenidentität

[`functional_model_candidate`](../../src/simulation/original/OriginalModelMapping.h)
führt sieben funktionale Kandidaten: ClaySource, Pottery, Warehouse, Market,
Well, Farm und HealthPost. Alle tragen `functional=Inferred` und
`mechanics_identity=Unknown`. Die getrennten Grafikbelege für Well/HealthPost
verifizieren keine physische Einflussquelle oder deren Geometrie.

Der [Model-Provenance-Vertrag](model-provenance.md) trennt verifizierte
Dateizeilen und Labels ausdrücklich von World-Objektidentitäten. Das in OE-11
vorbereitete [Sessionmodell](model-session-binding.md) prüft echte Quelldaten,
aber beglaubigt diese sieben Analogien nicht. Bloße Namensähnlichkeit oder ein
vorhandener Dateiindex begründet daher keine produktive Einflussquelle.

Der numerische [Desirability-Kern](../../src/simulation/original/Desirability.h)
ist weiterhin unabhängig nutzbar. Seine Existenz erfüllt den fehlenden
Identitätsvertrag zwischen einem bezahlten World-Gebäude und einer Originalquelle
nicht. Ein Cache ohne belegte Quellen würde diese Abnahme ebenfalls nicht erfüllen.

### Präzisierter Originalpfad aus der Fortsetzung

Die anschließende [read-only Quellenuntersuchung](../reverse/original-desirability-source-admission.md)
belegt für den untersuchten Standardpfad die Originalseite der Zulassung:
Status1/3, ein zusätzliches Nullwort bei runtime `+0x3c`, tatsächlicher signed
Originaltyp für Modellfelder1–4 und eine getrennte virtuelle Footprintabfrage.
Die anschließende [Archiv-/Zustandsprüfung](../reverse/original-desirability-source-state.md)
lokalisiert dieses eigene Wort bei raw Base54, getrennt vom geparsten
`subindex` bei Base18, und belegt konkrete Manager-ID-Verknüpfungen. Initial0
ist kein lebenslanger Zustand; bei bestimmten Slotersetzungen werden Links
bewahrt. Ein Subindex0 ersetzt dieses zusätzliche Quellengate daher nicht.

Die einzige bereits ausdrücklich typisierte World-Brücke sind Vacant-Hauscontainer
mit Originaltyp2. Ihr unveränderlicher Entwicklungszustand besitzt jedoch
`original_status=0`; dieser Wert würde schon am ersten Originalquellengate
scheitern. Daraus folgt kein generelles Emittentenverbot für jeden Originaltyp2
in anderem Zustand, aber auch keine Freigabe zum Hochsetzen des gespeicherten
World-Status. Die sieben Nicht-Haus-Analogien, Bau-/Restore-Gates und die
Finanzgrenze bleiben ungelöst. Mehr EXE-Evidenz ersetzt diese Vertragsarbeit nicht.

Die folgende [konkrete Factoryprüfung aller sieben Originaltypen](../reverse/original-desirability-source-factories.md)
belegt zusätzlich zwei Geometrieabweichungen: Warehouse54 liefert Quadratseite3
gegenüber World2; Farmhouse193 verwendet den initialen Side-Default3 gegenüber
World1. CommonMarketSquare59 nimmt einen eigenen Sonderformpfad, trotz
initialem Sidebyte1. Die vier anderen übereinstimmenden Defaults begründen
ebenfalls keine World-Identität. Keiner dieser Befunde erlaubt, bestehende
World-Footprints stillschweigend zu ändern oder eine Quellenanalogie hochzustufen.

Die [anschließende Marktstudie](../reverse/original-market-desirability-shape.md)
belegt für zwei Descriptor-Selektoren4×7/7×4-Einfluss-Boundingrechtecke sowie
eine eigene Verteilungsarithmetik. Das sind keine physischen Besitzerflächen.
Der vorhandene Quadratkern deckt diesen Pfad nicht ab; ebenso wenig ersetzt
der Original-Descriptorzustand die fehlende bezahlte World-Quellenbindung.

## 2. Quellenbau, Entfernung und Source-Restore sind gesperrt

Die exakten Rule7-Gates in
[`World::validate`](../../src/simulation/World.cpp#L1012) und
[`World::execute_impl`](../../src/simulation/World.cpp#L1343) erlauben derzeit
nur `PlaceRoad`, `RemoveRoad`, `PlaceBridge`, `PlaceRamp` und
`PlaceHousehold`. Die sieben Source-Baubefehle und tatsächliche
`execute(DemolishBuilding)`-Aufrufe werden vor einer Mutation verweigert.
Das ist eine Aussage über ausgeführte Befehle, keine Behauptung über den
allgemeinen Profilgetter `demolition_supported()`.

Der gemeinsame
[`validate_original_economy_snapshot`](../../src/simulation/World.cpp#L404)
verlangt weiterhin ausschließlich bezahlte Vacant-Hauscontainer mit exakt
zugeordneten OriginalHouse-Nullzuständen. Zusätzliche Quellengebäude, aktive
Wirtschaftszustände und Umdeutungen alter House-Authority werden dadurch nicht
restaurierbar. Ein vorher präparierter Save kann diese Gates nicht ersetzen.

Damit fehlt die reguläre Befehls-/Persistenzkette für die geforderte bezahlte
Quellenszene und für den Nachweis einer Invalidierung nach Bau und Entfernung.
Nur für einen Cachetest Quellen, Abriss oder fremde Snapshots freizuschalten
wäre eine zusätzliche Vertragsänderung außerhalb der Nutzerentscheidung.

## 3. Zwanzig bezahlte Häuser sind derzeit nicht finanzierbar

[`Rules::city_v11_starting_treasury`](../../src/simulation/World.h#L162),
[`Rules::household_cost`](../../src/simulation/World.h#L125) und
[`starting_treasury_for`](../../src/simulation/World.h#L230) ergeben für
Rule7 ein Budget von 1300 Funds und authored Hauskosten von 80 Funds. Das
finanziert höchstens 16 Häuser: 1280 ausgegeben, 20 verbleibend. Zwanzig Häuser
kosten bereits 1600 Funds, bevor Straßen oder Quellen hinzukommen.

[`World::tick`](../../src/simulation/World.cpp#L2528) schreibt unter Rule7
ausschließlich die Schedulerzeit fort. Es gibt keine Einnahmen aus Bewohnern,
Versorgung oder Steuern. Warten finanziert die fehlenden vier Häuser daher
nicht; ein verweigerter Abriss bietet ebenfalls keinen Finanzierungspfad.
Zusätzliche Funds, Gratisgebäude oder veränderte Preise würden den bestehenden
Vertrag ändern und wurden nicht beauftragt.

## Tatsächlicher Nachweis

Die frische private Release-Probe verwendet die unveränderten produktiven
World-Befehle und bereitet die lokal vorhandene Original-Modelldatei
schreibgeschützt vor. Ihre kleine 24×24-policy3 ist eine authored Testkarte;
dies ist kein Originalmap- oder menschlicher Gameplay-Nachweis. Die Probe
injiziert keine Snapshot-Authority, erhöht kein Budget und gewährt keine Güter.

Aus dem Repositoryroot lautet der tatsächlich ausgeführte private Repro-Befehl:

```sh
python3 .local/original-economy/oe12/run_prerequisites.py
```

Die private Implementierung `PrerequisitesProbe.cpp` und der Runner liegen
unter `.local/original-economy/oe12/`. `probe-run.json` hält Compile-/Run-Befehl,
Release-Buildverzeichnis, verwendete Library-SHAs und Exitcode 0 fest;
`prerequisites.json` enthält die einzelnen Beobachtungen:

- Alle sieben Quellenkandidaten bleiben `Inferred`/`Unknown`; ihre tatsächlichen
  Baubefehle werden atomisch verweigert, mit vollständig unverändertem Snapshot.
- Von 20 tatsächlichen Hausbauversuchen werden 16 bezahlt angenommen und vier
  mit `Not enough money` atomisch verweigert. Funds und Gesamtausgaben stimmen
  mit dem bestehenden Vertrag überein.
- Der tatsächliche Abrissbefehl wird atomisch verweigert.
- Nach 20.000 produktiven Scheduler-Ticks bleiben 20 Funds und insgesamt null
  Steuereinnahmen. Abgesehen von den Ticks bleibt der vollständige bezahlte
  WorldSnapshot identisch; vorhandene Wirtschafts-/Conservation-Invarianten
  gelten weiterhin.

Das Probe-Ergebnis lautet `BLOCKERS_REPRODUCED`, ausdrücklich nicht OE-12-PASS.
Die sieben zusätzlichen vorhandenen Release-Kontrolltests für Modelschema,
Modellvorbereitung, numerische Desirability und Rule-Gates bestehen mit
**7/7 PASS, kein Skip**. Sie kontrollieren vorhandene Verträge und erfüllen keine
OE-12-Featureabnahme. Genaue Befehle und Ergebnisse stehen im
[Fortschritt](../codex-original-economy/PROGRESS.md). Die vollständigen
Debug-/Release-/ASan+UBSan-Gate-B-Suites sind für diesen blockierten Auftrag
bewusst **NOT_RUN**. Gate B bleibt **BLOCKED**; OE-13 wird nicht freigegeben.

## Voraussetzungen für einen späteren Fortgang

Ein späterer Auftrag muss die fehlenden Verträge konkret und unabhängig lösen:

1. Für jede verwendete Einflussquelle einen belastbaren Originalbeleg vorlegen,
   der World-Identität, tatsächlichen Footprint und genau den ausgewählten
   Modeltyp verbindet. Nicht belegte Kandidaten bleiben explizit übersprungen;
   ihre funktionale oder grafische Ähnlichkeit genügt nicht.
2. Den beauftragten bezahlten Quellenbau, seine sichere Entfernung und den
   strikten Save-/Restore-Vertrag ausdrücklich bereitstellen und prüfen. Erst
   damit kann eine reale Mutation die geforderten Cache-Invalidierungen auslösen.
3. Einen ausdrücklich genehmigten regulären Finanzierungspfad für die
   20-Haus-Abnahme schaffen oder den Abnahmevertrag ausdrücklich ändern. Die
   jetzige Grenze bleibt bis dahin bestehen; Testinjektion ersetzt sie nicht.

Erst nach Auflösung dieser Blocker kann OE-12 implementiert und mit bezahlten
Szenen, Kartenrand, Reload, relevanten Ereignissen und unverändertem City-v13/16
geprüft werden. Anschließend sind die vollständigen
[Gate-B-Prüfungen](../codex-original-economy/QUALITY_GATES.md#gate-b--nach-oe-12-sichere-versionierte-laufzeit)
auszuführen. Dieser Bericht nimmt keine dieser Änderungen vorweg.
