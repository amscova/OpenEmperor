# OE-08: ausdrücklich gewählte Rule-7-Generation

Stand 2026-10-16, Produktionsbasis `2651408e71db6a13ad6bec45214dd2b76f502ea2`.
City-v16 Rule7 ist als neue Identität mit unveränderter MapPermissions policy3
reserviert und unterstützt. Schema23 ist weiterhin frei und für OE-09 reserviert;
OE-08 implementiert noch keinen Rule-7-Spielstand und keine Originalwirtschaft.
Dies ist eine **AUTHORED FALLBACK**-Entwicklungsgrenze, kein Originalspielbeleg.

## Zugang und Verweigerung

Der reine Gate `original_economy_enabled(profile, version)` ist ausschließlich für
City-v16/7 wahr; die gleichnamige World-Abfrage verwendet dieselbe Bedingung.
Unbekannte zukünftige Versionen erhalten keine Infrastruktur-Berechtigung.
City-v11 bleibt bei Rule3, City-v16 bei Rule4. Menü, Settings und die ausdrücklich
wählbaren bisherigen Bridge-/Ramp-Modi bleiben unverändert.

Ein normaler `World`-Konstruktor, `World::restore`, `import_snapshot` und
`SandboxView` verweigern Rule7. Der View verweigert schon im Konstruktor vor SDL,
Asset-/World-Vorbereitung und Save-Anzeigen. Es gibt keine unfertige New-Sandbox-
Option. Die Versionserkennung allein bestätigt keine spielbare Laufzeit.

Die ausdrücklich benannte Testfactory
`World::create_original_economy_development(permissions)` verlangt vollständige,
unveränderliche policy3-Permissions. Sie erzeugt ausschließlich eine bezahlbare
Infrastruktur-Testwelt ohne Gebäude, Courier, Güter oder fortschreitende Zeit.
Erlaubt sind nur PlaceRoad, RemoveRoad, PlaceBridge, PlaceRamp und bestehende
atomische Road-Batches. Andere Commands werden vor Mutation verweigert;
`tick()` wirft vor Countern, Produktion und Workforce-Arbeit. Die Freigabe ist
nur ein privater Konstruktorparameter, kein neues Snapshot-/Save-Feld.
Modelbindung, Scheduler und Produktionsfreigabe folgen ihren eigenen Tasks.

`make_document`, `restore_save` und der Writer verweigern Rule7 ausdrücklich,
bevor ein alter Writer-Schemafall oder eine Zieldatei erreicht wird. Parser für
Schemas1–22 bleiben unverändert streng; Schema23 wird noch nicht gelesen.
Es gibt keine Migration und keine automatische Umdeutung eines alten Saves.

## Entwicklerdiagnose

```sh
.local/reswall/release-build/openemperor \
  --data .local/gog-extracted/app --sandbox Cities/Xia.map \
  --sandbox-rules sandbox-city-v16 --sandbox-original-economy-check --report-json
```

Der Diagnosebefehl führt nur die vorhandene Map-Vorprüfung mit Rule7/policy3
schreibgeschützt aus. Er startet weder SDL noch World oder Modelvorbereitung.
Die JSON-Ausgabe unterscheidet `map_rules_status` und dessen konkrete Fehler von
`runtime_ready: false`. Auch für eine geeignete Map ist Exit1 beabsichtigt:
Die Originalwirtschaft ist noch nicht startbar. Falsche Argumentkombinationen
ergeben Exit2. Data, Map, City-v16 und JSON müssen ausdrücklich angegeben sein;
Spiel-, Save-, Lade- und Visualoptionen können nicht kombiniert werden.
Ein kaputtes oder fehlendes Modell wird hier nicht als gültiges Modell ausgegeben;
dieser Befehl trifft ausdrücklich keine Aussage zur Modellbereitschaft.

## Infrastruktur und bisherige Verträge

Die begrenzten gemeinsamen Bridge-/Ramp-Gates erlauben exakt5/6/7 beziehungsweise
6/7 unter City-v16. Map-Vorprüfung, World und Road-Diagnose verwenden sie.
MapPermissions, WorldSnapshot, BridgeSpan und RampLink erhalten keine neue
Authority oder Felder. Preise, IDs, signed heights, Graphen, Schutzregeln,
BFS, Road-Reparatur, Ressourcen und Renderer benutzen weiterhin dieselben Pfade.

Die SDL-freie neue Suite prüft die vollständige16-Profil-/Versionsmatrix,
Policy-/Start-/Build-/Tick-/Save-Verweigerung und bezahlte Brücke/Rampe gegen
Rule6: vollständiger Snapshot nach Normalisierung ausschließlich der Versionszahl,
1536 gerichtete Kanten und eine bezahlte Road-Reparatur. CLI-Prozesstests verwenden
nur authored Maps, ungültigen SDL-Treiber und unveränderte Dateiinventare.
Der direkte View-Test prüft Verweigerung vor SDL und alle Performancecounter0.
Die bestehenden Regressionen und der separate vollständige Legacy-Replay sind
im [Fortschritt](../codex-original-economy/PROGRESS.md) protokolliert.

Der unveränderte pure Vier-Modul-Forkvergleich wird gegen die ergänzte World-API
neu ausgeführt. Sein exakter Header-Kompatibilitätspin bleibt strikt; alte
Probe-API und sämtliche World-/Snapshot-Datenfelder bleiben unverändert.
Der aktualisierte [portable Bericht](core-parity.json) ändert keine Regelwerte.
Gate A bleibt der historische begrenzte Kernnachweis; Gate B bleibt bis OE-12
offen. Originalzeitbasis, spielbare Wirtschaft und menschliche Spielabnahme
sind weiterhin nicht nachgewiesen.
