# OE-13: versioniertes Warenbuchhaltungsfundament

Der ausdrücklich angenommene [Warenvertrag V1](goods-ledger-v1-proposal.md)
verwendet ausschließlich City-v16 **Rule9 / Schema25 / policy3**. Bestehende
Regeln1–8, Schemas1–24, numerische Enumidentitäten und ihre Writerbytes bleiben
unverändert. Dies ist ein ausdrücklich gewählter Entwicklerstand. Die normale
Anwendung, das Menü und Recovery erhalten keinen Einstieg und keine Migration.

## Tatsächlicher Runtimeumfang

Rule9 übernimmt die bezahlten Vacant-Häuser, Well72-Einflussquellen, Straßen,
Brücken und Rampen von Rule8 mit denselben2000 Funds, Preisen und Limits.
Well72 beeinflusst weiterhin ausschließlich Desirability. Alle sechs Güter
sind in der echten World **unsupported/0**: keine Produktion, Importe,
Hauslieferung, Worker, Bewohner, neue Courierrolle oder Tick-Konsum. Auch
Warten, Quellenbau oder Quellenabriss erzeugen keine Warenbuchung.

`World::original_goods_balance` und `original_goods_balances` lesen die
vorhandenen Authoritys über begrenzte const-Spans und Stack-Kontexte. Sie
liefern Güter-ID, abgeleitete Unterstützung, vier historische Zähler,
physischen Stock/Cargo sowie getrennte Reservierungsinformationen. Dabei
entstehen keine Worldkopien, Dateizugriffe, Modellvorbereitungen oder Routen.
Die Bilanz ist zunächst eine API für Tests und spätere Inspectoranbindung;
ein neuer Waren-HUD gehört nicht zu OE-13.

Die sechs Mengenfelder im vorhandenen `OriginalHouseState::house` bleiben
die einzige Hausbestandsauthority. Der separate `OriginalGoodsState` enthält
sechs historische Konten und mögliche Nicht-Haus-/Cargo-/Reservierungszeilen;
in der echten Rule9-World sind sämtliche Konten null und alle Listen leer.
Es gibt keine öffentliche Worldmethode oder Spieleraktion zum Auffüllen.
Eine mathematisch ausgeglichene Fälschung wie `produced=stock=1` wird mangels
zugelassenem tatsächlichem Ursprung ebenfalls verweigert.

## Reiner Transaktionskern

Die SDL-/World-freie Library `openemperor-original-goods-ledger` verarbeitet
explizite numerische Konten, vorhandene Building-/CourierIds und begrenzte
Kapazitätskontexte. Positive Testkonten sind ausdrücklich authored. Diese
Zahlenoperationen behaupten weder eine erfolgte Produktion noch einen
Walkerbesuch in der World.

Für jedes Gut gilt geprüft:

`produced + imported = consumed + demolished + stock + cargo`

Reservierungen sind Verpflichtungen und zählen nicht zusätzlich als Masse.
Jede Transaktion validiert den gesamten Input, erzeugt einen vollständigen
Kandidaten und validiert diesen vor Rückgabe. Die ursprünglichen Daten bleiben
bei Erfolg und Fehler unverändert. Historische Zähler und Summen verwenden
checked uint64-Arithmetik; Hausbestände bleiben0..32767, Nicht-Hausbestände,
Cargo und Reservierungsmengen0..INT32_MAX. Die numerischen Domains sind keine
neue Ingame-Kapazität und stehen nicht als frei änderbare Savefreigaben bereit.

Eine Reservierung beansprucht Quellstock und Zielraum. Ein einmaliges,
gegebenenfalls kleineres Laden verschiebt die tatsächliche Menge in Cargo
und gibt ungenutzte Verpflichtungen frei. Teilankünfte reduzieren Cargo und
Incoming um dieselbe Menge. Abbruch vor Beladung löst nur Reservierungen;
danach bleibt Cargo bis zu einer ausdrücklich bestätigten Rückgabe erhalten.
Eine inzwischen volle Quelle darf die Rückgabe atomisch verweigern.

Gebäude- und Courieridentitäten werden nicht neu vergeben. Transferhandles
binden die vorhandene CourierId an eine fortlaufende Sequenz; terminale
Historienzeilen verhindern doppelte oder veraltete Ankunfts-/Abbruchbuchungen.
Der reine Kontext ist auf die vorhandenen46 Owner,26 Courier und26
Historienzeilen begrenzt. In der echten OE-13-World entstehen keine dieser
Transportzeilen; neue Actor-/Historyfreigaben benötigen ihren eigenen Vertrag.

## Save und Entwicklerstart

Schema25 besitzt einen eigenen expliziten Writer-/Reader-/Restorepfad.
Sechs kanonisch geordnete Konten werden gespeichert; House-Mengen werden
nicht gespiegelt. `supported`, Kapazitäten, Bilanzcaches und Modelltabellen
sind keine gespeicherte Authority. Neue Payloads in Schemas1–24 sowie
Crossloads zwischen den getrennten Entwicklerständen werden verweigert.
Unbekannte, doppelte, fehlende oder falsch typisierte Keys bleiben strikt;
Dokumentgröße8MiB und Tiefe32 unverändert. Map-/Masken-/policy3-/Modellbindung
und atomarer Dateiaustausch bleiben erforderlich.

Ein separat ausgeschlossener Launcher lässt sich aus dem Repositoryroot mit
vorhandener Originalinstallation und SDL3 starten:

```sh
sh tools/play_original_goods.sh
```

Er startet pausiert auf Xia mit eigenem Savepfad
`.local/original-economy/player/rule9.json`; der alte Rule8-Launcher und dessen
Savepfad bleiben unverändert. `7` kauft ein Vacant-Haus, `I` eine reine
Well72-Einflussquelle, `5` wählt aus; Quellen lassen sich ohne Refund abreißen.
`F5`/`F9` speichern/laden ausschließlich den passenden Rule9-Stand. Die drei
optionalen Argumente wählen ausdrücklich DATA/MAP/SAVE; das vierte `--load`
lädt nur diesen gewählten Save. Das ist keine aktive Warenversorgung.

## Reproduzierbare Abnahme

Nach Konfiguration und Build der entsprechenden Testtargets:

```sh
ctest --test-dir .local/reswall/release-build --output-on-failure -R '^original-goods-'
cmake --build .local/reswall/release-build --parallel 2 --target openemperor-original-goods-fixture openemperor-original-influence-fixture
python3 tools/test_original_goods.py .local/gog-extracted/app --build-dir .local/reswall/release-build
```

Der eigene Runner verwendet die hashgebundene echte Modelldatei in einem
privaten Root mit authored24×20-Geographie. Er kauft regulär20 Häuser,
40 Straßen und Well72-Quellen, entfernt und kauft eine Quelle erneut, ohne
Waren zu injizieren. Separate Prozesse prüfen SaveDocument-/Byte-/Provenance-
und Bilanzgleichheit nach deterministischer Tick-/Commandfortsetzung.
Eingabesaves und Originaldateien bleiben unverändert. Zusätzlich werden die
vor OE-13 separat eingefrorenen Rule7-/Schema23- und Rule8-/Schema24-Bytes
gegen die aktuellen Writer verglichen; die frühere OE-00-Referenz allein
deckt diese späteren Entwicklerverträge nicht ab.

Die positive Abnahme betrifft ausschließlich den reinen Transaktionskern.
Der SDL-Test benutzt tatsächliche F5/F9-Handler und bezahlte Befehle mit
authored Softwaregrafik. Ein solcher Lauf ist keine native menschliche
Spielrunde. Tatsächlich ausgeführte Builds, Negativfälle, Prozessreplays und
Legacyvergleiche stehen im [Fortschrittsregister](../codex-original-economy/PROGRESS.md).
Ein OE-13-Grundlagen-PASS ist weder ein positiver Warenkreislauf noch Gate C.
