# OE-30: optionaler bezahlter Starter

Vor Implementierung am OE29-Stand64cbfcb erklärt: Der neue Menüeintrag startet
ausschließlich die vorhandene Originalwirtschaft Rule10/Schema26/Policy3,
Entwicklungsrevision27. Regeln, Startbudget12000, Modelbindung, Difficulty,
Gebäudegrenzen, Kosten und sämtliche früheren Defaults bleiben unverändert.
Der historische Rule7-Name im Prompt bezeichnet diesen angenommenen Nachfolger.

Originalmodus, Difficulty und die Wahl zwischen leerer Stadt und bezahltem
Vier-Haus-Starter sind sitzungsbezogen. Der bisherige New-Sandbox-Weg und
SettingsV1 bleiben erhalten. Der Starter ist ein ausdrücklich von OpenEmperor
gestalteter Bauplan, keine rekonstruierte originale Startstadt. Er kauft seine
Straßen, Häuser und Quellen ausschließlich mit normalen World-Commands bei
Tick0. Alle Bestände und Einwohner beginnen leer; erst tatsächliche Ticks,
Produktion, Routen und Ankünfte entwickeln die Stadt. Er sucht nur vorhandene
legale zusammenhängende Fläche; bei ungeeigneter Karte scheitert der noch
unveröffentlichte Kandidat mit Diagnose, ohne die laufende Stadt zu verändern.

Nur die aktive Rule10-Factory wird für die normale Anwendung geöffnet. Die
gewöhnlichen Konstruktor-/Restorewege und die Entwicklungsstände7–9 behalten
ihre Sperren. Manuell ausgewählte, exakt passende Rule10-Saves werden ausdrücklich
über den aktiven Restoreweg geöffnet; keine Migration, Difficulty-Übersteuerung
oder Wiederholung der Starterkäufe beim Laden. Modell-/Map-/Revisionsprüfungen
bleiben verbindlich.

OE30 verwendet manuelle F5/F9-Saves. Original-Recovery bleibt bis OE31 gesperrt;
das Menü zeigt diese Grenze und startet für Original keine AutosaveController.
Die globale Autosavepräferenz und die bisherigen Recoverywege bleiben erhalten.
Ein bloßes Entfernen der bisherigen Recovery-Sperre wäre unvollständig, weil
deren Bevölkerungszusammenfassung noch Legacyfelder verwendet.

Gate E benötigt echte Vier-Haus-Entwicklung mit Auf- und Abstieg, bezahlte
Produktions-/Liefer-/Konsumereignisse, normale Bedienung und separaten
Prozess-Restore sowie vollständige Debug-/Release-/ASan+UBSan-Suites.
Food-/Hemp-Verbrauch und die reale Keramiklieferung werden getrennt belegt;
höhere Hausanforderungen dürfen nicht durch erfundene Musik-/Materialversorgung
umgangen werden. Fehlende Nicht-Haus-Originalbilder behalten benannte Fallbacks.
Technische Abnahme, Originalparität und menschliche Bewertung bleiben getrennt.
Nur ein lokaler Task-Commit; kein Push/Merge und kein Beginn von OE31.
