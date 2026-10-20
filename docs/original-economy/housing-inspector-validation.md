# OE-28: technische Inspector-Abnahme

OE-28 ist technisch PASS. Vier Original-House-Ansichten und begrenzte
Supply-/DES-/Risk-Overlays erklären aktuelle Daten ohne neue Spielregeln.
Der [vorab erklärte Vertrag](housing-inspector-runtime.md) bleibt unverändert:
Rule10/Schema26/Policy3/Revision27. Simulation, Persistence, Map-/Renderer-Kerne,
Defaults und bestehende Produktions-/Legacytests sind bytegleich zum Start.

Maus wählt die vier Tabs; PageUp/PageDown wechselt sie, das Mausrad scrollt
innerhalb des Panels. F10 oder der Panelknopf wechselt Off/Supply/DES/Risk.
Beide Entscheidungen sind sitzungslokal: F9 erhält sie in derselben View;
eine neu geladene Sitzung beginnt State/Off. Die autoritativen Savebytes
bleiben bei allen21 positiven Ansichts-/Zoom-/Overlayaufnahmen exakt gleich.

## Tatsächliche Tests

Frische Ninja-Verzeichnisse unter `.local/original-economy/oe28/`, System-SDL3,
gepinnte Projektabhängigkeiten, Python3.15 und die vorhandene reale Modelldatei.
31 gezielte CTests je Konfiguration, insgesamt93 PASS,0 Fehler/Skips;
alle drei finalen Builds ohne Compilerwarnungen. Die Sanitizerkonfiguration
ist RelWithDebInfo mit ASan/UBSan, `halt_on_error=1`; die auf dieser Plattform
nicht geprüfte Leakerkennung wird daraus nicht abgeleitet.

| Konfiguration | CTests | Build | Tests |
|---|---|---|---|
| debug |31/31 |174.711s |289.14s |
| release |31/31 |233.553s |23.817s |
| sanitizer |31/31 |430.903s |123.312s |

Exakte Configure-/Build-/CTestbefehle, Prozess-IDs, Umgebungsoptionen,
Sourceinventare und Loghashes stehen in `{debug,release,sanitizer}/`:
`configure.json`, `matrix-build.json`, `matrix-ctest.json`, `matrix-junit.xml`
und `matrix-result.json`. Die unveränderte Auswahl ist in `test-plan.json`
erhalten; sie umfasst neue Diagnose/SDL-Inspector, Originalproduktion/-goods/
-influence/-save-Views, City16-Geometrie/-Service/-Wartung, Bridge/Ramp,
allgemeine Eingabe-/Menügrenzen, alte und aktive Saves, Szenenpixel und Feuer.
Dies ist die relevante OE-28-Auswahl, kein neuer vollständiger Gate D/E.

Die neuen Tests heißen `original-house-inspection`,
`original-house-inspection-real` und `original-house-inspector-view`.
Die ersten beiden prüfen dieselbe Darstellung gegen vorhandene World-Daten,
einmal mit authored und einmal mit realem Modell. Der SDL-Fall ist ausdrücklich
bezahlte authored Geografie/Modelldarstellung mit realen Commands, Bewohnern,
Workern, Fahrten, Ankünften und Konsum; keine Snapshot-/Authorityinjektion.

Bei120t zeigt der reale SDL-Negativfall eine verfügbare Herbalistquelle,
aber Timer0 und keinen Besuch; das Supply-Zeichen bleibt X. Bei448t hat
Common1 tatsächlich Typ5/Plain Cottage,18 Bewohner, Qualität80, Ceramics1 und
Herbalist89 samt Ankunftsbeleg. Elite16 besitzt tatsächlich den4×4-Körper/Typ13.
Vorhandene Quellen werden nie als bereits erfüllte Besuche dargestellt.
Die sechs Waren trennen Hausbestand/Lieferung/Konsum von Stadtproduktion,
Reservierung und Fracht; auch Foodbelege und gekürzte Transferhistorie werden
geprüft. Alle Beträge und Belege stammen aus echten World-Zuständen.

Maus/Pages/F10, Help/F3, Fokus, falsche Releases, Resize, schmale Panels,
synthetischer2×-Output, F2/F3/F6/F8 und echte F5/F9-Ereignisse bestehen.
100 wechselnde UI-Frames mit Hover/allen Tabs/Overlays lassen alle zwölf
Performancezähler auf0: keine Worldkopie/-restore/-commands/-ticks, Roadplans,
Routen/BFS, Datei-/Asset-/Uploadarbeit. Snapshot, Cache-, Routen- und
Texturmetriken bleiben exakt. Der reine Beobachter besitzt einen zusätzlichen
100-Frame-Abfragetest.

## Bilder und Kompatibilität

`captures/` enthält22 tatsächliche PNGs plus je einen Beobachtungsbericht und
ein vollständiges Schema26-Save. Zehn PNGs wurden vom Root geöffnet: State
bei1×/2×/4×, Needs/Goods/Visits bei1×, der Besuchsnegativfall und alle drei
Overlayarten über1×/2×/4×. Glyphen, Tabs, Umbruch, Markierungen und Legende sind
lesbar; ganze Glyphenreihen bleiben im Panel. Bei4× schneidet das unveränderte
Kartenfenster den großen4×4-Körper ab. Die übrigen zwölf PNGs wurden erzeugt
und durch SDL-Prüfungen gebunden, nicht separat vom Root visuell geöffnet.
`root-image-review.json` enthält die tatsächlich geöffneten Hashes.

Die alte Industryv5-Übersicht und ihr Quelleninspector wurden mit dem
unveränderten OE27-Binary und dem neuen Binary im identischen bestehenden
Sandbox-Softwaretest aufgenommen: beide vollständigen BMP-Dateien sind
bytegleich (`legacy-pixel-comparison.json`). Alte City16-Inspector-/Layout-/
Inputverträge bestehen zusätzlich in den unveränderten Tests. Daraus wird
keine neue vollständige historische72-Save-/167-Map-Abnahme abgeleitet.

Zwei tatsächliche neue Release-OS-Prozesse setzen die geprüften Common- und
Elite-Checkpoints jeweils20.000 Ticks fort. Vier vollständige SaveDocuments
und vier vollständige Berichte sind bytegleich zu den OE27-Kontrollen;
Eingangsdateien und privater Datenbestand bleiben unverändert.
`process-replays.json` nennt die tatsächlichen PIDs, Befehle und Vergleichspaare.
Der bestehende Fixtureguard schreibt neue Ergebnisse weiterhin ausschließlich
unter seinem privaten OE27-Ausgabebaum; dieser UI-Task erweitert den Guard nicht.

Alle1464 Originaldateien/819711091 Bytes bleiben einschließlich Hashes,
Dateimodi und Änderungszeiten unverändert. Die finale Prüfung bindet560
Sourcepfade an `source-freeze.json`; `final-validation.json` enthält die
Prüfberichte, Binary-/Bildhashes und Grenzen. Das unveränderliche Supplement
`validation-closure.json` bindet zusätzlich die tatsächlichen UI-/Legacy-
Captureausführungen und ihre alten Referenzläufe. Erste Warnungs-/SDL-Testversuche
sind erhalten: zwei Signednesswarnungen wurden korrigiert; ein ungünstiger
simulierter Kantenklick wurde durch einen tatsächlichen sichtbaren
Owner-Innenflächenklick ersetzt. Keine Worldassertion wurde abgeschwächt.

Native/human bleibt NOT_RUN, Gesamt-Originalparität PARTIAL. Die Szenen zeigen
vorhandene authored Fallbackgrafik; Originalsprites/Objektkomposition gehören
weiterhin zu OE-29. Gate E folgt erst nach OE-30. Genau ein lokaler OE-28-Commit,
kein neuer Push/PR/Merge/Tag/Release.
