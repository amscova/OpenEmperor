# OE-11: unveränderliche Modelldaten pro Rule7-Session

Basis ist `d6eb736e` nach OE-10. Die ausdrücklich gewählte Rule7-Entwicklersession
erhält ein vorbereitetes `OriginalRulesModel` als geprüfte Abhängigkeit. Die
normale Anwendung, das Menü, gewöhnlicher Rule7-Restore und Recovery bleiben
geschlossen. Der [Scheduler](scheduler.md) schreibt weiterhin ausschließlich
Zeit fort; Modellbindung aktiviert keine Originalwirtschaft.

## Quelle und einmalige Vorbereitung

Die lokale, ausschließlich gelesene Datei
`Model/EmperorBuildingModels.txt` unter `.local/gog-extracted/app` hat
34.218 Bytes und SHA256
`7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6`.
Das ist die identifizierte lokale Referenz, keine allgemeine Einschränkung auf
diesen einen Hash: andere Dateien müssen die gleichen vollständigen Parser-,
Identitäts- und arithmetischen Prüfungen bestehen und erhalten eigene Bindungen.
Originaltabellen oder andere proprietäre Bytes werden weder dokumentiert noch
in Saves oder Git kopiert.

`persistence::prepare_original_rules_model(root, difficulty)` lädt die Datei
über den vorhandenen begrenzten, schreibgeschützten Reader. Er prüft den
kanonischen Datenroot, die innerhalb des Roots aufgelöste reguläre Datei,
die Größen- und Syntaxgrenzen sowie vollständiges Lesen. Danach bereitet
[`OriginalRulesModel`](../../src/simulation/original/OriginalRulesModel.h)
alle 15 benannten Hauszeilen und die sieben bereits typisierten Gebäudezeilen
für genau eine Difficulty vor. Fehlende oder falsche Quellidentitäten,
ungültige Difficulty und überlaufende ursprüngliche Modifierarithmetik
verweigern die Vorbereitung vollständig.

Das Ergebnis ist ein `shared_ptr<const OriginalRulesModel>` mit const
Provenienz, Difficulty, vorbereiteten Hausregeln und Gebäudewerten. Es besitzt
keinen Dateizugriff und keinen veränderlichen globalen Cache. Der temporäre
`ModelData`-Parserzustand wird für spätere Abfragen nicht benötigt. Getrennte
Vorbereitungen erzeugen unabhängige Objekte; World-Kopien dürfen denselben
unveränderlichen Sessionzeiger teilen.

## Explizite Sessiongrenze

Der vorhandene, ausschließlich im SDL-Testtarget kompilierte Entwickleradapter
ruft beim neuen Start einmal die Vorbereitung auf und übergibt das Ergebnis an
`World::create_original_economy_development(policy, model)`. Die World erhält
weder `data_root` noch die Möglichkeit, sich später selbst eine Datei zu suchen.
Ihre gespeicherte Sourceidentität entsteht aus dem tatsächlich vorbereiteten
Objekt. `World::original_rules_model()` liefert diese unveränderliche Abhängigkeit.

Der explizite Snapshot-Restore verlangt bei gespeicherter Sourceidentität ein
passendes vorbereitetes Objekt: Formatversion, SHA256, Bytezahl und Difficulty
müssen genau übereinstimmen. Ein anderer Hash oder eine andere Difficulty darf
nicht durch einen vorhandenen Zeiger verdeckt werden. Die bisherigen bewusst
ungebundenen Infrastruktur-/Clockfixtures bleiben ohne Sourceidentität möglich;
sie erhalten weder eine Modellbindung noch Schema23-Saveberechtigung.

Ein frischer Datei-Restore bereitet ohne explizit übergebenes Modell genau
einen Kandidaten anhand der gespeicherten Difficulty vor. Er prüft zusätzlich
die aktuelle Dateiidentität, Map-SHA, Buildable-Masken-SHA, unveränderte policy3
und alle bestehenden World-/Transport-/Accounting-Invarianten. `SandboxView`
veröffentlicht die neue World mit ihrem Modell erst nach erfolgreicher
Validierung. Ein fehlgeschlagener Kandidat verändert keine bestehende Session.

## F5/F9 und unverändertes Schema23

[`SaveDocument`](../../src/persistence/SandboxSave.h) besitzt ausschließlich im
Speicher `prepared_original_rules_model`; `make_document` übernimmt den Zeiger
der aktiven World. F5 verwendet ihn für die Capture- und Schreibvorprüfungen
wieder. Dabei wird die Quelldatei an der ausdrücklichen I/O-Grenze mit
`inspect_model_source` erneut begrenzt gelesen und gehasht, aber nicht erneut
geparst oder zu Regeln vorbereitet. Eine fehlende oder veränderte Datei
verweigert das Schreiben vor der atomischen Ersetzung des vorhandenen Saves.

`read_save` stellt keinen Runtimezeiger aus JSON her. Das tatsächliche F9 lädt
das Dokument, bereitet einen neuen unabhängigen Modellkandidaten vor und
restauriert die World mit dieser Abhängigkeit. Bei gleicher Quelle und
Difficulty sind Identität und typisierte Daten gleich; der neu vorbereitete
Zeiger gehört zur neuen World. Alte extern gehaltene unveränderliche Zeiger
bleiben gültig. F9 startet wie bisher pausiert und setzt den Frameakkumulator
zurück.

Der [Schema23-Vertrag](save-schema-23.md) erhält keine neuen JSON-Keys. Gespeichert
bleiben ausschließlich Sourceidentität und bestehende Datenzustände, niemals
Modelltabellen, Quellpfade oder vorbereitete Regeln. Sourceidentität allein
ersetzt seit OE-11 keine vollständige semantische Vorbereitung: die historischen
OE-09-Syntaxfixtures mit frei gewählten Modellnamen bleiben historische
Persistenzbelege und werden nicht still zu gültigen Runtime-Modellen erklärt.

## Laufzeit, getrennte Sessions und Prüfgrenzen

Frames, Rendern, World-Ticks und vorbereitete Modellabfragen laden, parsen oder
prüfen keine Quelldatei. Eine bereits vorbereitete Session kann ihre reinen
Scheduler-Ticks und typisierten Abfragen auch bei vorübergehend fehlender Datei
fortsetzen. Die nächste ausdrückliche Save-/Load-Operation prüft die Datei
erneut und verweigert bei fehlender oder abweichender Identität atomisch.

Die vorhandenen Legacyprofile benötigen weiterhin keine Modelldatei. Normale
Create-/Restore-Pfade, Schema1..22, policy3, Bridge-/Ramp-Authority und 20-Hz-/
400-Tick-Verträge erhalten keine neue Modellprüfung. Auch bei Rule7 bleiben
Bewohner, Vorräte, Services, Evolution und laufende Wirtschaftsupdates inaktiv;
vorbereitete Werte sind keine gratis gewährte World-Authority.

Die [Modelsuite](../../tests/OriginalRulesModelTests.cpp),
[Persistenzsuite](../../tests/OriginalEconomySaveTests.cpp) und
[SDL-Suite](../../tests/OriginalEconomySaveViewTests.cpp) enthalten Prüfungen für
mehrfache Vorbereitung, getrennte Quellen/Difficulties, fehlgeschlagene
Kandidaten, Zeigerstabilität, sourcefreie Frames/Ticks und tatsächliches F5/F9.
Die zwei unabhängig authored SDL-Quellen liefern unterschiedliche vorbereitete
Hauswerte, ohne Housing-Kapazität zu aktivieren. Der separate OS-Prozess lädt
dieselbe Sourcebindung, führt dieselben 127 SDL-Steps aus und vergleicht
vollständige SaveDocuments und kanonische Bytes mit der Referenzsession.

Ausgeführte Konfigurationen, Befehle und finale Ergebnisse stehen im
[Fortschritt](../codex-original-economy/PROGRESS.md). Die protokollierten Prüfungen gelten als technische Abnahme. Menschliche
Spielabnahme und vollständige Originalparität bleiben offen. Gate B folgt
nach OE-12.
