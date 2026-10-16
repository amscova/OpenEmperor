# OE-10: versionierter Rule7-Scheduler

Basis ist `5ed833f4b0e97ac6a9e327f43e95a155732a9b6e` mit dem getrennten
[Schema23-Zustand](save-schema-23.md) aus OE-09. Die ausdrücklich erzeugte
Entwicklungsworld kann jetzt ihre Zeit fortschreiben. Der normale Spielstart,
gewöhnlicher Rule7-Restore und Recovery bleiben geschlossen; Bewohner,
Warenkreisläufe, Serviceankünfte und Hausentwicklung sind weitere Aufgaben.

## Originalbeobachtung und authored Zeitbasis

Die gepinnte, ausschließlich gelesene EXE hat SHA256
`6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e`.
Die begrenzte Untersuchung ergibt folgende voneinander getrennte Aussagen:

| Originalstelle | Beobachteter Vertrag |
| --- | --- |
| `0x4ac2e4..0x4ac2f9`, `0x4ac4c8..0x4ac4e7` | Gespeicherte Phase wird inkrementiert und bei 51 auf 0 gesetzt; der Zyklus enthält die Werte 0..50. |
| Tabelle bei `0x4ac4e8`, Ziele `0x4ac45e` und `0x4ac48f` | Phase 35 ruft den Service-Countdownmanager auf, Phase 39 den Evolutionsmanager. Dazwischen liegen weitere Dispatchpositionen. |
| `0x519025..0x51905b` | Nach der House-Evolution erfolgt Verbrauch nur bei Selector `0xc82ef0` gleich 0 oder 7. |
| `0x4ac650..0x4ac67e`, `0x4ac6b8..0x4ac6c9`, `0x4ac794..0x4ac7b4` | Der gleiche Selector steigt einmal nach dem 51-Phasen-Pass. Werte 0..15 bilden die innere Periode; deren Grenze setzt ihn zurück und erhöht einen weiteren Counter mit Werten 0..11. Die 16er-/12er-Struktur ist damit unabhängig vom Fork beobachtet. |
| `0x4ac7ce..0x4ac7e9`, Getter `0x4f8d50..0x4f8d55` | Der getrennte Original-Jahrescounter steigt beim neuen größeren Counterwert **1**. Der Getter liefert konstant 1; dies ist nicht der Counterwrap 11→0. |

Die bisherigen [Service-](../reverse/original-service-timer-contract.md) und
[Verbrauchsverträge](../reverse/original-goods-consumption-food-contract.md)
belegen die begrenzten Einzeloperationen und ihre Reihenfolge. Die private
Ergänzung liegt unter `.local/original-economy/oe10/`: Prüfsystem
`check-scheduler-evidence.py`, `scheduler-exe-evidence.json`, Erläuterung und
begrenzte Disassemblyfenster. Originalbytes oder Disassembly gehören nicht ins
Git. Die Untersuchung führt die Original-EXE nicht aus und rekonstruiert weder
sämtliche Schedulerarbeit noch vollständige Datums-/Saisonsemantik.

Der neue [Header](../../src/simulation/original/OriginalScheduler.h) bindet
`scheduler_mapping_version=1` an City-v16 Rule7 und Schema23. Er legt ausdrücklich
**einen World-Tick pro Schedulerphase** fest. Diese Zuordnung zu den vorhandenen
20 Ticks/s ist authored und belegt keine originale Spielgeschwindigkeit.
Die Namen Schritt, Monat und Jahr bezeichnen einen gezählten Kalender mit
16 Pässen pro Monat und 12 Monaten pro Jahr. Sein Jahr zählt vollständig
abgelaufene Zeiträume ab Epoche 0; es ist nicht der oben beobachtete originale
Jahrescounter mit Grenze 1. Eine andere Tickzuordnung oder Epoche benötigt eine
neue gespeicherte Regelidentität und darf bestehende Saves nicht neu deuten.

## Nächster Dispatch aus abgeschlossenen Ticks

`WorldSnapshot::ticks` zählt vollständig abgeschlossene Phasenschritte.
`scheduler_position(t)` beschreibt die Position des **nächsten** Dispatchs:

```text
passes = t / 51
phase  = t % 51
step   = passes % 16
month  = (passes / 16) % 12
year   = (passes / 16) / 12
```

Die Ableitung verwendet ausschließlich Division und Modulo. Auch eine reine
Positionsabfrage für den vollständigen uint64-Bereich benötigt kein
überlaufendes Produkt. Anfangs ist der nächste Dispatch Phase 0; erst nach
Phase 50 wechselt der Pass, nach Schritt 15 der Monat und nach Monat 11 das
gezählte Jahr.

| Abgeschlossene Ticks | Nächste Phase | Schritt | Monat | Elapsed-Jahr | Eligibility |
| ---: | ---: | ---: | ---: | ---: | --- |
| 0 | 0 | 0 | 0 | 0 | kein House-Dispatch |
| 35 | 35 | 0 | 0 | 0 | Service-Countdown |
| 39 | 39 | 0 | 0 | 0 | Evolution, danach Verbrauch |
| 50 | 50 | 0 | 0 | 0 | letzte Phase des Passes |
| 51 | 0 | 1 | 0 | 0 | nächster Pass |
| 396 | 39 | 7 | 0 | 0 | Evolution, danach Verbrauch |
| 804 | 39 | 15 | 0 | 0 | Evolution ohne Verbrauch |
| 815 | 50 | 15 | 0 | 0 | letzte Phase des Monats |
| 816 | 0 | 0 | 1 | 0 | nächster Monat; kein authored Jahreswechsel |
| 9791 | 50 | 15 | 11 | 0 | letzte Phase des gezählten Jahres |
| 9792 | 0 | 0 | 0 | 1 | nächstes vollständig gezähltes Jahr |

`next_scheduler_dispatch(t)` liefert Position und Eligibility-Flags:
Service-Countdown bei Phase 35, Evolution bei Phase 39 und anschließender
Verbrauch nur bei Schritt 0 oder 7. Es führt diese Operationen nicht aus und
erteilt weder Service noch Waren, Bewohner oder eine Evolutionsentscheidung.
Die Zwischenphasen bleiben verfügbar für spätere tatsächliche Ankünfte;
ein zusammengefasster Fork-`step_house` wird nicht eingeführt.

## Authority, Restore und Grenzen

Die vorhandenen gespeicherten Ticks sind die einzige Clock-Authority. Schema23
erhält keine zusätzlichen Clock-, Phasen-, Monats-, Jahres- oder Versionskeys.
Die fest gebundene Mappingversion und die Ticks reichen aus, um nach Restore
denselben nächsten Dispatch zu bestimmen. Es gibt keine implizite Migration;
ein bisheriger Tick0-Save behält seine kanonische Darstellung. Ein Payload mit
zusätzlichen vermeintlichen Clock-Keys bleibt unbekannt und wird abgelehnt.

`OriginalHouseState::cycle_phase` bleibt ausdrücklich reserviert und 0. Es ist
keine redundante Kopie der globalen Phase und wird erst durch einen späteren
Hausverarbeitungsvertrag aktiviert. Ein nach Zeitfortschritt gekauftes Vacant-
Gebäude speichert seinen wirklichen `placed_tick`; Restore verlangt
`placed_tick <= ticks`. Alle übrigen OriginalHouse-Beobachtungen und alten
Wirtschaftsfelder bleiben im derzeit zugelassenen Nullzustand.

`scheduler_max_tick = UINT64_MAX - 1` erhält die vorhandene Save-Integergrenze.
Dieser terminale Zustand ist speicher- und restaurierbar, bietet jedoch keinen
nächsten Dispatch. `advance_scheduler_tick` wirft ab diesem Wert vor einer
Mutation; ein Zustand mit `UINT64_MAX` wird abgelehnt. Der letzte zulässige
Schritt erreicht damit einen weiterhin speicherbaren Zustand. Nur die enge
Rule7-Restoreprüfung nimmt den reinen Scheduler vom alten Health-Deadline-
Headroom aus. Alte Profile und deren Deadlinegrenzen bleiben unverändert.

`World::tick()` unter Rule7 erhöht ausschließlich den vorab geprüften Tickwert.
Es führt keine alten Produktions-, Nachfrage-, Unterhalts-, Health-, Feuer-,
Bevölkerungs- oder Navigationsupdates aus. Die vorhandene 20-Hz-TickDriver-
Auslieferung und die alten 400-Tick-Nachfragen bleiben für bisherige Profile
maßgeblich. Die Model-Source-Identität aus OE-09 ist weiterhin keine vorbereitete
Runtime-RulesModel-Bindung; diese gehört zu OE-11.

## Entwickleradapter und Nachweise

Nur das vorhandene SDL-Testtarget kompiliert den ausdrücklich benannten
Entwickleradapter. Er startet und lädt pausiert, erlaubt Pause/Continue, Step
sowie die vorhandenen Geschwindigkeiten 1/2/4 für den reinen Scheduler. Reset
bleibt gesperrt. F9 setzt den bestehenden TickDriver einschließlich seines
Bruchteilakkumulators zurück. Normale App, Menü und Recovery erhalten keinen
Zugang zur unfertigen Generation.

Die reine [Schedulersuite](../../tests/OriginalSchedulerTests.cpp) wurde mit
C++20 und den Projektwarnflags direkt gebaut und ausgeführt: **42.579 Prüfungen
bestanden**. Sie prüft feste Grenzpositionen, sämtliche Phasen und monatlichen
Selectoren, getrennte Phase35/39-Events, 0/7/15, einen unabhängigen 20k-Odometer
sowie den uint64-Terminalzustand. Der private EXE-Prüflauf bestätigt 11 Fenster,
99 Instruktionen beziehungsweise 348 Bytes und 22 Opcode-/Pointerassertions
bei unveränderten Originalbytes, Dateigröße, Modus und mtime.

Weitere Suites prüfen den 20k-World-Restore-Replay, tatsächliche SDL-Pause-/Step-
und Geschwindigkeitssteuerung, F5/F9 vor/auf/nach Pass-, Monats- und Jahresgrenzen,
vollständige Dokument-/Bytegleichheit sowie einen zweiten OS-Prozess. Die
tatsächlich ausgeführten Konfigurationen und Ergebnisse stehen ausschließlich
im [Fortschritt](../codex-original-economy/PROGRESS.md). Diese technischen
Nachweise sind getrennt von einer menschlichen Spielabnahme oder originaler
Echtzeitparität; Gate B bleibt bis OE-12 offen.
