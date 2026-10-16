# OE-00: eingefrorene Baseline vor der Originalwirtschaft

Stand: 2026-10-16. Produktionsbasis ist
`fef111deff4cedd3d3fa48663556d41b2e4b31a6` auf `main` (Hit-Test-Fix).
Das angelieferte Paket nennt noch `f7a5035e92a9`; diese Abweichung wird vor
Implementierung festgehalten. Gearbeitet wird auf `feature/original-economy-v1`.
OE-00 ergänzt ausschließlich Diagnosewerkzeug, synthetische Werkzeugtests und
Dokumentation. Bestehende Produktion, Commands, Defaults und Save-Writer bleiben
bytegleich zum erfassten HEAD.

## Quellen, Versionen und Architekturentscheidung

- Höchste unterstützte City-v16-Regel: **6**; neuer normaler City-v16-Start:
  **4**. Allgemeiner Menüdefault: City-v11 Rule3.
- Höchstes Save-Schema: **22**. City-v16 Rule3/4/5/6 schreiben19/20/21/22.
- MapPermissions unterstützt **policy1/2/3**; Rule5/6 verwenden weiterhin3.
- **Rule7 / policy3 / Schema23** sind für das neue, später ausdrücklich
  gewählte Profil reserviert. Sie sind in OE-00 weder unterstützt noch aktiv.
  Vor OE-08/OE-09 müssen die Nummern erneut geprüft werden.
- Bereits existierende Bridge-/Ramp-Supportguards würden Rule7 ausschließen.
  Eine spätere Integration muss ihre neue Unterstützung ausdrücklich ergänzen;
  bestehende Rule5/6-Verträge dürfen dabei nicht umgedeutet werden.

Jede Stelle ist in der [Architekturkarte](architecture-map.md) mit Source- und
Testlinks belegt. Sie führt Kaufkommando, Funds, Tick-Reihenfolge, tatsächliche
Ankunft, Household-Demand, Snapshot, Save/Restore, Menü und Painter zusammen.
Insbesondere erfolgt aktueller Demand vor der Ankunft desselben World-Ticks.

## Was eingefroren wird

Das Werkzeug [original_economy_baseline.py](../../tools/original_economy_baseline.py)
verwendet unveränderte vorhandene Fixtures:

| Baseline | Produktionsbeleg und Umfang |
|---|---|
| Rule6/Schema22 | 11 normale Checkpoints plus je100-Tick-Kontrolle: vor/nach Rampenkauf, Einfahrt, mid-ramp, Ausfahrt, Lieferung, Rückweg,10k/end, Road-Trennung und Reparatur |
| Rule5/Schema21 | 10 normale Bridge-Checkpoints plus je100-Tick-Kontrolle |
| Historische Rule3/Schema19 | 14 vollständige bezahlte Xia-SaveDocuments |
| Rule4/Schema20 | Je acht Dokumente für Xia und Handan, einschließlich Goal und tatsächlicher Fortsetzung |
| WorldSnapshots | Alle72 Dokumente: vollständige kanonische `{rules, world}`-Abbildung zusätzlich zu den unveränderten Dateibytes |
| Neue Prozesse | 11 Rampen- und10 Bridge-Checkpoints; je sofortige Restore- und100-Tick-Fortsetzungsbytes gegen separat erzeugte Kontrollen |
| Original-Map-Permissions | 167 komplette Manager,165 vorbereitet, zwei bestehende Konflikte. Alle165×3Policies×3Legacy-Masken=1485 Produktionsfingerprintfälle |

[CityV16RampsFixture::Context::save](../../tools/CityV16RampsFixture.cpp#L37)
prüft das gespeicherte und restaurierte Dokument gegen das tatsächliche
`World::snapshot()`. Die zusätzliche JSON-Kanonisierung ist nur ein vollständiger
Vergleich dieser bereits geprüften Snapshot-Felder, kein zweiter World-Serializer.
Der bestehende [Rampen-Runner](../../tools/run_city_v16_ramps.py) und
[Brücken-Runner](../../tools/run_city_v16_bridges.py) führen die bezahlten
World-/View-/Persistenzpfade und getrennten Prozess-Restarts aus.

Das [Original-Audit](../../tools/AuditOriginalOccupancy.cpp#L261) liefert je Map,
Policy und Maskenfall `policy_fingerprint`, `canonical_sha256` und
`legacy_mask_sha256`. Die Baseline hält alle Werte fest, nicht nur den Census.
Dies ist Hash-Gleichheit der vollständigen kanonischen Zustände, keine neue
Behauptung eines unabhängigen literal-byte Vergleichs aller Policy-Arrays.
Die zwei verweigerten Karten erhalten keine künstlichen Permissions.

Die Rule6-Map ist eigens authored. Das Originalhöhen-Audit findet weiterhin
keine legalen Original-Landpaare mit Höhendifferenz1. Bezahltes authored Gameplay
ist daher weder Originalmap-Rampenfreigabe noch Originalspiel-Parität.

## Wiederholbare Befehle

Lokale Originalquelle: `.local/gog-extracted/app`. Das vollständige Inventar wird
vor und nach jedem Run geprüft:1464 Dateien,819711091 Bytes, Manifest-SHA256
`dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb`.
Die Modelldatei bleibt read-only; ihr SHA ist im [Forkvergleich](fork-comparison.md)
gebunden. Keine Originaldaten oder generierten Spielstände gehören in Git.

```sh
# Kleine Negativtests des Baseline-Werkzeugs; ausschließlich synthetische Daten.
python3 tests/OriginalEconomyBaselineTests.py -v

# Konfigurierten Release-Build benutzen. Das Werkzeug baut seine realen
# Fixture-Targets vor jedem Run frisch und schreibt ausschließlich nach .local/.
python3 tools/original_economy_baseline.py capture \
  --data .local/gog-extracted/app --build .local/reswall/release-build \
  --output .local/original-economy/oe00/reference

# Erneute vollständige Erzeugung, Vergleich gegen den eingefrorenen Vertrag.
python3 tools/original_economy_baseline.py verify \
  --data .local/gog-extracted/app --build .local/reswall/release-build \
  --reference .local/original-economy/oe00/reference/contract.json \
  --output .local/original-economy/oe00/verification
```

Jeder Outputordner muss neu sein. Vorhandene Referenzen werden nicht ersetzt.
Der committed [baseline-lock.json](baseline-lock.json) bindet den Digest des
vollständigen lokalen Vertrags. `verify` prüft zuerst diese Bindung sowie jede
eingefrorene Save-/Snapshotdatei und vergleicht danach frisch erzeugte Saves und
alle Permissions. Geänderte Cargo-/Snapshotwerte, abweichende Savebytes oder ein
fehlender Policyfall werden verweigert. Der Referenzdigest darf bei späteren
Tasks nicht einfach mit den neuen Ergebnissen überschrieben werden.

Die bestehenden Author-/Bridge-Fixtures behalten ihre engeren Ausgabegrenzen
unter `.local/city-v16-ramps/` und `.local/city-v16-bridges/`. Das neue Werkzeug
schwächt diese Guards nicht ab: Dort entstehen frische Runs, deren gültige
SaveDocuments unter dem OE-00-Output separat eingefroren werden.

Die relevante bestehende CTest-Auswahl ist:

```sh
ctest --test-dir BUILD -N
ctest --test-dir BUILD --no-tests=error --output-on-failure \
  -R '^(sandbox-city-v16-(base|persistence|balance|determinism|endurance|geometry|determinism-v2|endurance-v2)|sandbox-save-resume|autosave-recovery-history|ramp-transport|bridge-transport|original-road-save-recovery|gate-passage-save-recovery|sandbox-map-permissions)$'
```

`BUILD` ist ein überprüfter Debug-/Release-/ASan+UBSan-Build. Die tatsächlichen
Configure-/Build-/Testbefehle und Ergebnisse werden pro Konfiguration unter
`.local/original-economy/oe00/validation/` aufgezeichnet. Vor jedem ersten CTest
wird die registrierte Testliste erfasst. OE-00 benötigt nicht das erst nach
OE-07 fällige vollständige Gate A; keine Gate-Freigabe wird vorweggenommen.

## Befund gegenüber dem Fork und offene Punkte

Der [gebundene Forkvergleich](fork-comparison.md) prüft nur ModelData,
Desirability, HouseEvolution, OriginalHousingView und zugehörige Waren/Services.
Fork-HEAD `d861d2912337c09751cb68196f85d5a30752726e` ist11 Commits voraus und
vier zurück. Die bestehenden World-/Save-Dateien sind in fünf geprüften
Schlüsseldateien blob-identisch; zusätzliche pure/read-only Module sind keine
produktive Originalwirtschaft.

Konkrete Lücken sind Parser-Härtung und Hash-Bindung, unabhängige Originalunits,
Zeit-/Visit-Semantik, nur angenommene HousingView-Fakten, originale Waren ohne
bezahlte Quellen und reale Arrival-Kette sowie eigene getypte House-Authority.
Der Vergleich nennt zu jedem Befund genaue unveränderliche Source-Links und
trennt getestetes Fork-Verhalten von ungeklärter Originaltreue. Windows-Änderungen
werden nicht übernommen.

Nächster vorgesehener Task: **OE-01**, robuster read-only Modellparser. Bis dahin
bleiben sämtliche zukünftigen Wirtschaftsaufgaben und Phasen-Gates offen.

## Tatsächlich ausgeführte Abnahme

| Tatsächlich ausgeführter Check | Ergebnis |
|---|---|
| Relevante vorhandene Debug-CTests | 15/15 PASS, 611.42s |
| Dieselben Release-CTests | 15/15 PASS, 37.78s |
| Dieselben ASan/UBSan-CTests | 15/15 PASS, 150.86s; keine Meldungen |
| Synthetische Baseline-Werkzeugtests | 13/13 PASS |
| Unabhängiger Fork-Probe, ASan/UBSan | PASS |
| Unveränderte Fork-Regelsuite auf macOS gegen aktuelle World-Library | 12/12 Untertests PASS, inklusive installierter Modelldatei, kein Skip |
| Vollständiger erster Capture | PASS: 72 Saves/Snapshot-Projektionen, 1485 Policyfälle |
| Vollständige unabhängige Wiederholung mit finalen Pfad-/Build-Guards | PASS: derselbe gesamte eingefrorene Vertrag |
| Separater Prozess-Restore/Fortsetzung | Je Run 11 Rampen- und 10 Bridge-Paare, beide vollständigen Dateien exakt |
| Vorhandener zusätzlicher Altvergleich | Je Run 50 vollständige ältere Savebytes exakt |
| Originalinventar und Diagnose-Source-Digest | Vor/nach jedem vollständigen Run identisch |
| Bestehender Source-/Test-/Tool-/Resource-Bestand | Alle 426 erfassten Dateien weiterhin bytegleich zum Basis-HEAD |

Der gesperrte Vertragsdigest lautet
`12823612834d27099189c18e31ed4fed4761c9ff34486227b3075cf1069a49e5`.
Er wurde beim ersten Capture erzeugt und durch den zweiten vollständigen
`verify` bestätigt. Zwischen diesen Runs wurden ausschließlich die neuen
Diagnose-Guards gehärtet; ihre jeweiligen Source-Digests sind getrennt im
jeweiligen `run.json` erfasst. Während beider Runs blieb deren eigener
Source-Digest unverändert. Die bestehenden 426 Dateien wurden nicht geändert.

Vollständige lokale Belege: `reference/{run,contract}.json`,
`verification/{run,contract}.json`, `initial-state.json`, `validation/` und die
Fork-Berichte unter `.local/original-economy/oe00/`. Die getrennten Fixturepfade
sind in den `run.json`-Dateien protokolliert. Menschliche/native Spielabnahme und
vollständige Originalwirtschaft sind nicht Teil dieses technischen PASS.

**OE-00: PASS.** Alle Phasen-Gates und OE-01 bis OE-33 bleiben offen. Die
Task-Commit-SHA wird nach dem Commit in der Übergabe genannt und gemäß Paket
erst im Folge-Task in `PROGRESS.md` nachgetragen.
