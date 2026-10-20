# OE32: begrenzte Longplay- und Lastabnahme

Basis ist OE31 `d6226ed2de02413f4d462d0b968fe13b6f3d9f69`.
Dieser Vertrag wird vor der Implementierung festgelegt. Er ändert keine
Spielregel, Saveversion, Modellzahl, Produktions- oder Darstellungsauthority.
Aktiv bleiben Rule10/Schema26/Policy3/Entwicklungsrevision27. Gate F folgt OE33.

Die im Auftrag genannten46 Gebäude/26 Couriers sind historische Legacygrenzen.
Original Rule10 erlaubt bereits82/58. Die Diagnose verwendet Budgetformat1
und misst diese bestehenden Grenzen; sie führt keine zweite Limitauthority ein.
Die getrennten Goods-Ledgergrenzen46 Besitzer/26 Frachten/26 Reservierungen
werden zusätzlich geprüft. Konservativ sind höchstens58×81=4698 dynamische
Zielcacheeinträge möglich; tatsächlich zulässige Ziele und gespeicherte Routen
werden über die bestehenden Rollen-/Cachedaten gezählt. Sichtbare
Vektorkapazitäten und Prozess-RSS sind Messwerte, kein vollständiges Heapinventar.

Ein bezahlter Zweidistrict-Aufbau soll82 Gebäude/58 Couriers mit normalen
Commands erreichen. Die Transportkomponenten bleiben getrennt; Workforce,
Funds und Revision sind weiterhin stadtweit. Erreichbarkeit, Parentbeziehungen,
Kosten und tatsächliche Frachten werden geprüft, nicht aus einer Stückliste
behauptet. Authored Testterrain wird gegebenenfalls ausdrücklich von echter
Xia-/Originalmodellabnahme getrennt. Kein Starter wird zweimal aufgerufen.

Der echte unveränderte Xia-/Normal-Starter läuft20k deterministisch und100k
Endurance. Separate Prozesse speichern, laden und setzen tatsächlich verdiente
Zustände fort. Der Beobachter zählt reale Hausbewertungen, Auf-/Rückstufungen,
Konsum-/Lieferereignisse und fehlende Anforderungen zum Bewertungszeitpunkt.
Leere momentane Bestände gelten nicht als verpasster Bedarf. Sechs Güter,
Food je Crop, industrielle Materialien, Frachten/Reservierungen, Workforce,
Navigation und Funds werden mit vorhandenen Produktionsinvarianten geprüft.
Defizite und Schulden sind Ergebnisse, kein Anlass für heimliche Subventionen.

Staffingunterbrechungen werden durch reguläre Operation-/Prioritycommands
erzeugt; Feuer wird nur aus dem tatsächlich laufenden Modell beobachtet.
Original Rule10 ruft absichtlich keine Legacy-Krankheit auf und verlangt
neutrale Healthfelder. Krankheit wird deshalb separat im unveränderten
Legacy-Kontrollmodell gemessen, niemals als Originalkrankheitsparität ausgegeben.

Frame-/Previewprüfungen verwenden die produktive View und vorhandene zwölf
Arbeitszähler, stabile World-/Cache-/Texturzustände und koaleszierte Gesten.
Die bestehende Rasteriteration und Render-/HUDallokationen werden damit nicht
als ausgeschlossen behauptet. Keine neue Frame-/Tickinstrumentierung.
Zeitmessungen laufen seriell in eigenen Releaseprozessen relativ zu Legacy;
umfangreiche Invarianten/JSON/I/O liegen außerhalb der gemessenen Tick-/Frame-
Fenster. RSS und gemessene Last werden mit Modell, Stadtgröße und Grenzen
berichtet; keine aus Hardwarezeiten erfundene universelle Frameratezusage.

Abnahme verlangt relevante Debug-/Release-/ASan+UBSan-CTests, tatsächliche
20k/100k-Läufe und Neuprozessvergleiche, volle167-Karten-/1485-Policykontrolle,
die72 alten SaveDocuments sowie den eingefrorenen101-Dateienvergleich für
Schemas1–25. Originaldateien bleiben in Bytes/Modus/mtime exakt. Private
Artefakte bleiben unter `.local/`; PrefPaths und persönliche Saves bleiben
unberührt. Fehlende Pflichtdaten oder nicht ausführbare100k bedeuten BLOCKED.
Native/human und vollständige Originalgrafikparität bleiben getrennt offen.

Nur ein lokaler OE32-Taskcommit nach erfolgreicher Abnahme ist autorisiert;
kein Push, Merge, Tag, Release oder Beginn von OE33.
