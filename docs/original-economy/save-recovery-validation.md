# OE31 – Original-Saves und Recovery

Basis:324fc8fd765c4c4485d7d69e97e39530a504b95d. Der
[vorab erklärte Vertrag](save-recovery-contract.md) betrifft ausschließlich den
aktiven Rule10-/Schema26-/Policy3-/Revisions27-Stand. World, Wirtschaft,
Navigation, Modelle und SaveDocument-Serialisierung bleiben unverändert.

## Ergebnis und Umfang

Technische Abnahme PASS:189 relevante CTests (63 je Konfiguration), danach
30 gezielte Nachprüfungen des Capturefixes (10 je Konfiguration), insgesamt
219 erfolgreiche Konfigurationsläufe ohne finale Fehler/Skips. Der Beleg
`.local/original-economy/oe31/final-validation.json` (21458Bytes) hat SHA256
`8c479748c8c908c6194aeff0be0fd81fdd656220447a8a362b62320d15ed90ad`.
Alle577 finalen Quellpfade sind eingefroren; der kanonische Freeze3-Digest lautet
`aa5262d4f5db57a87ba6afecde402dac796617b8d932a97a0dc4ea00ad123b7e`.

RecoveryStore/AutosaveController erhalten einen ausdrücklichen
`OriginalProduction`-Zugang; ihr Default verweigert weiterhin die eingefrorenen
Original-Entwicklerstände. Der normale Originalmodus verwendet die bestehende
Autosavepräferenz: geschützter Start, höchstens fünf Checkpoints und1200t
Mindestabstand am bestehenden sicheren Punkt. Erfolgreiches F9 beginnt eine neue
Historie. Menü-Recovery und ein im Dateidialog geöffneter eigener Checkpoint
bekommen ein neues manuelles Ziel; F5 erhält die Quelldatei. Fehler bleiben
sichtbar und erhalten World beziehungsweise bereits veröffentlichte Punkte.

Der neue Savekatalog prüft je authored/echter Xia-Suite2577 Verweigerungen und
neun verdiente Zustände, ergänzt um jeweils eine separat authored bezahlte
4×4-Elite. Die Elite-Geometrieprobe ist kein echter Xia-Elitenaufbau. Er umfasst rekursive
Missing/Extra/Type-Prüfungen mit exakten JSON-Pointern sowie IDs, Counts, Goods,
Services, negative Stocks, Modellbindung, Footprints/Schutzflächen und
Route/Cargo/Reservation. SandboxSave.cpp/.h benötigten keinen Produktivfix.
Der erhaltene ausführliche Katalog liegt unter
`.local/original-economy/oe31/adversarial-initial4/{authored,real}/`; sein
Quell-/Laufbeleg ist `adversarial-ready.json`. Diese frühen Läufe gegen die
unveränderten OE30-Corearchive werden von den frischen OE31-CTests getrennt.

## Tatsächliche Recoverykorrekturen

Unveränderte private Red/Green-Probes belegen drei Fehler im neuen Zugang:
Einzeln gültige Dateien konnten verschiedene Difficulties mischen, die Zeitfolge
konnte rückwärts laufen und ein letzter erlaubter Sequenzschritt machte die
Historie anschließend unlesbar. Der Katalog prüft nun gemeinsame Identität und
monotone Zeitfolge in Sequenzreihenfolge. Fehlerhafte Punkte werden isoliert;
gültige Geschwister dürfen unabhängig branchen. Ein defekter Start ist keine
Identitätsreferenz. Append verweigert inkonsistente/erschöpfte Historien, während
der letzte veröffentlichte Punkt lesbar bleibt. Tiefere Metadaten werden vor
weiterer DOM-Verschachtelung bei Tiefe32 verweigert; zulässige flache
Legacy-Erweiterungen bleiben erhalten. Der alte Parser akzeptierte50000 Ebenen;
ein Crash oder eine Speicherkorruption wurde dabei nicht beobachtet.

Ein weiterer tatsächlicher Red/Green-Test belegt den Capturefehler: Bei einer
während des Spiels entfernten privaten Modellquelle warfen drei Updates
ungefangen dieselbe Ausnahme. Jetzt zeigt der Originalmodus den Fehler, erhält
World/Historie und wartet1200t. Echte Menütests belegen1250→2450,100 arbeitsfreie
Idleframes, den Modellverlust zwischen erfolgreichem F9 und Recoverybeginn,
Toggle OFF/ON und bestätigten Stadtwechsel. Initialer Capturefehler meldet keine
angelegte Historie; eine zusätzliche automatische Start-/Parent-Retry-Maschine
wird nicht eingeführt. OFF erfasst keinen unnötigen Original-Save. Die Legacy-
Captureausdrücke bleiben unverändert.

Nach diesem Fix änderten sich ausschließlich sechs App-/Controller-/Testdateien.
Alle zehn betroffenen Menü-/Recovery-/CLI-/Gate-/Versions-Suites und der normale
Menü-Prozessablauf wurden in allen drei Konfigurationen erneut ausgeführt.
World/Maps/Renderer/Savecodec/RecoveryStore und der eingefrorene Exporter bleiben
zum bestandenen63er-Grundbatch byte-identisch. Manuelle Worlddateien und deren
160t-Weiterläufe sind im neuen Menü-Harness außerdem byte-identisch zum Grundstand.

## Wiederholbare Prüfungen

CMake registriert `original-save-adversarial-synthetic`,
`original-save-adversarial-real`, `original-recovery-capability` und
`original-recovery-view`. Die komplette63er-Auswahl steht in
`.local/original-economy/oe31/qa-selection.json`; sie umfasst sämtliche
Save-/Recovery-/Persistence-Suites sowie betroffene alte/neue Menü-, View-,
CLI-, Regelgate- und Versionskontrollen. Dies ist keine volle241er-Suite und
keine vorgezogene Gate-F-Abnahme.

| Konfiguration | Relevante CTests | Status |
|---|---:|---|
| Debug |63 +10 nach Capturefix|PASS|
| Release |63 +10 nach Capturefix|PASS|
| ASan+UBSan |63 +10 nach Capturefix|PASS|

Die vorhandenen Originaldaten werden nur gelesen. `OriginalRecoveryAcceptance`
erzeugt zehn durch bezahlte Commands und normale Ticks verdiente Punkte:
Tick0, bewegte Warenreservierung, reale Versorgung, unmittelbar vor/nach Konsum,
vor/nach Devolution, vor/nach erlaubtem neutralem Well72-Abriss und echte Debt.
Jeder Punkt wird in einem eigenen Prozess mit vollständigen1/51/408t-
Weiterläufen verglichen. Ein separates Quellenpaar prüft fehlendes/geändertes
Modell ausschließlich an einer privaten Kopie. Sieben Dateifehlerfälle prüfen
Save-/Metadata-Atomarität; die Recovery-Suite prüft zusätzlich Rotation,
Common-/Elite-Population, Budget, Symlinks, Eltern, Reihenfolge und Sequenzende.

`OriginalRecoveryViewTests` verwendet echte queued SDL-F5/F9-Events und normale
Viewupdates: zehn F5,20 erfolgreiche und30 verweigerte F9 je Konfiguration.
SaveDocument, Corpus-/manuelle Savebytes, Pause und Erfolgszähler werden vollständig geprüft.
Die tatsächlichen Zustände sind zusätzlich inhaltlich geprüft; ein umbenannter
beliebiger Save genügt nicht. Der normale Menü-Harness prüft auch unveränderte
F5/F9-Dokumente und einen separaten160t-Prozessneustart mit Originalprofilen.
Scripted Software-SDL bleibt von nativer OS-Eingabe und Human Acceptance getrennt.

## Eingefrorene alte Verträge

Vor Implementierung wurde gegen unveränderte OE30-Archive je ein repräsentativer
authored Input für Schemas1–25 exportiert.101 vollständige Dateien erfassen
Input, Writeroutput, Normalisierung und DTO; der Referenzbeleg
`legacy-before2/frozen-reference.json` hat SHA256
`bc668b60ce9e116bee0fc60b3c87c3206227c5c1f90ae250354add03b76ec3d8`.
Die vorhandene1–3→4-Normalisierung bleibt ausdrücklich unverändert; keine alte
Datei wird in-place konvertiert. Der Export lässt sich mit
`openemperor-original-save-adversarial-tests --export-frozen-corpus NEW_DIR`
wiederholen und byteweise gegen `legacy-before2/corpus/` vergleichen.

Zusätzlich ist die unveränderte OE00-Referenz mit72 komplexen SaveDocuments/
Snapshots und1485 Policyfällen erneut exakt bestanden. Dieser zweite Vergleich ergänzt
die25-Schema-Referenz, ersetzt sie nicht. Alle1464 Originaldateien/819711091Bytes stimmen abschließend vollständig mit
dem Startbestand einschließlich SHA256, Modus und mtime überein.
`.local/original-economy/oe00/oe31-final-verification/run.json` hat SHA256
`19604378f3fe11ffa5b9f08ffe7b6a7b5a62ffb64286aa8353a6bedf4545dea0`.

## Grenzen und erhaltene Fehlversuche

Alle früheren Logs bleiben erhalten: drei erste Builds mit typisiertem JSON-
Vergleichsfehler im neuen Tool, die bewusst unterbrochenen breiten
Vorbereitungsbuilds und ein erster Releasebatch mit62 bestandenen Suites und
einem Fehler in der neuen Testfixture. Deren Branchschreiben hatte vorbereitete
Mapdaten verloren; zwei weitere neue Legacy-Testannahmen wurden anschließend
korrigiert, ohne den Legacyvertrag zu verschärfen. Die gezielte Fixtureprobe
besteht danach mit sieben Roundtrips/84 Fehlerfällen.

Der zweite eingefrorene Stand unterscheidet sich nur in
`tests/OriginalRecoveryTests.cpp`. Bereits erfolgreiche Prozessbelege dürfen
weiterverwendet werden: alle Tool-/Laufzeitquellen sind byte-identisch und der
Recoveryprozess-Binaryhash bleibt über den neuen Build exakt. Diese Brücke ist in `final2-process-binding.json` je Konfiguration mit
bestätigter Binarygleichheit dokumentiert. Jeder63er- beziehungsweise10er-Batch
bindet sämtliche577 Quellpfade unverändert an seinen geprüften Stand.
Die fünf geöffneten Grundbilder und das zusätzliche geöffnete F9-Bild nach dem
Capturefix sind in `opened-final-release.json` beziehungsweise
`opened-final-after-capture-fix.json` festgehalten.

LeakSanitizer ist auf diesem macOS-ARM-Host nicht unterstützt; ASan/UBSan laufen
mit `detect_leaks=0` und Abbruch bei Befunden. Native OS-Eingabe, menschliche
Spiel-/Grafikabnahme und vollständige Originalobjektgrafik bleiben offen.
Gesamtparität bleibt PARTIAL. Gate F/100k-/Longplay-Abschluss gehört weiterhin
OE32/33; kein Push, Merge, Tag oder Release.
