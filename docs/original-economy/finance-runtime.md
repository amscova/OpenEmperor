# Originalwirtschaft: Schwierigkeit, Haussteuern und Ziele in Revision27

Vor Implementierung am2026-10-19 auf `bcba05334494b4f6f5afd7d4c0d2d76a13469caa`
festgelegter OE-27-Vertrag. Ausdrücklicher Nutzerauftrag: „weiter gehts mit oe27“.
Aktiv bleibt Rule10/Schema26/policy3; Entwicklungsrevision27 verweigert26 und
Zukunftsrevisionen ohne Migration. Regeln1–9/Schemas1–25, Defaults, Transport,
Hausentwicklung, tatsächliche Population/Worker und die16 Eliteclaims bleiben exakt.
OE-28 und Original-Haussprites29 werden hier nicht vorgezogen.

## Belegte Werte und ausdrücklich authored Abrechnung

Der [Originalnachweis](../reverse/original-house-tax-contract.md) belegt als
Zwischenwert tatsächliche Bewohner mal vorbereiteten HouseField18/TaxRate.
Originale globale Einstellungen, DES-Zuschläge, TaxOffice209, Fließkomma-
Akkumulation, Rundung und Erhebungsaufruf sind zusätzliche Zusammenhänge.
Diese Integration behauptet deshalb keinen vollständigen Original-Steuereinzug.
Alle fünf typisierten Difficulties verwenden dieselbe immutable Vorbereitung:
Housewerte additiv mit signed Clamp, Buildingwerte als signed Prozentprodukt.
Im gebundenen Originalmodell sind die fünf TaxRate-/Kapazitätsmodifikatoren
jeweils0; andere Schwellen, Kosten und Risiken variieren. Keine erfundenen
zusätzlichen Difficulty-/Population-/Flächenfaktoren.

Authored ist eine direkte Entwicklungsabrechnung ohne erfundenes TaxOffice:
Alle816 abgeschlossenen Ticks wird jedes vorher bewohnte, im Pre-Tick-Zustand
nicht brennende Haus mit `occupants * prepared TaxRate` Funds belastet und
exakt derselbe Betrag dem Stadtkonto gutgeschrieben. Ein Modellratenpunkt je
Bewohner entspricht hier ausdrücklich1Funds je authored Monat. Kein Prozent-
Divisor, Mindestbetrag, Stock-/Service-Sink, Workerbonus oder25/40/60-Milestone.
Leere und brennende Häuser zahlen nichts; Nullrate bleibt0. Negative vorbereitete
TaxRates werden in allen15 aktiven Zeilen vor Create/Restore verweigert.

Vor jeder Tickmutation wird ein begrenzter Buchungsplan aus der bestehenden
Population, dem aktuellen Typ und dem aktuellen Firezustand vorbereitet.
Steuern werden vor Unterhalt gebucht, bevor Produktion, Bewegung, Fire,
Zuzug und Evolution desselben Ticks laufen. Eine am Tickende auslaufende
Feuerperiode ist im vorherigen Zustand noch aktiv; neuer Zuzug zahlt erst
bei einem späteren regulären Termin. Monat/Clock stammen allein aus Worldtick;
kein zweiter gespeicherter Scheduler und keine Zeitsprünge.

Unterhalt ist ausdrücklich authored:2Funds pro installiertem erforderlichem
Worker, alle400 positiven platzierungsrelativen Ticks, unabhängig von Pause,
Besetzung und Fire. Das erhält die bekannten Corekosten und bindet weitere
Quellen an ihre wirklich vorbereiteten Employees;0Worker kosten0. Straßen,
Brücken und Rampen erhalten keinen neuen Unterhalt. Quellen mit erlaubtem
Abriss sind workerfrei, daher braucht es keine erfundene Lohn-Abrisshistorie.

## Eine Geldauthority und begrenzte historische Eingaben

Bestehende `treasury`, `taxes_collected_total`, `maintenance_spent_total`,
`construction_spent_total` und House-`taxes_paid_total` bleiben die einzigen
Geldzähler. Eine neue begrenzte Financezeile je Haus speichert ausschließlich
kumulative steuerlich erfasste Bewohner und Ereigniszahlen pro15 Typzeilen
sowie den letzten tatsächlichen Erhebungseingang (Tick/Typ/Bewohner).
Keine Kopie der heutigen Population, Rate, Schwierigkeit oder Geldsumme.
Zeilen sind historischer Nachweis, kein vollständiges Ereignisjournal.

Restore verlangt eine Bijektion zu den gekauften Häusern, gültige Familie,
vorher belegten Ersteinzug/Typfreischaltung, tatsächliche Monatsgrenzen,
zeitlich und populationsseitig mögliche Ereignisse/Einheiten, passenden letzten
Eingang und exakt aus Modellrate×historischen Bewohnern rekonstruierte
Haus-/Gesamtsteuern. Unterhalt wird aus echten Bauzeiten und erforderlichen
Workern rekonstruiert. Veränderte Raten/Schwierigkeit benötigen ein anderes
gebundenes Modell und dürfen keine alten Buchungen neu interpretieren.

Die vollständige uint64-Buchhaltung wird durch Kürzen unsigned Summanden
gegen signierte Funds geprüft, einschließlich `INT64_MIN`. Baueinkäufe brauchen
weiter reale verfügbare Funds; spätere echte Einnahmen dürfen kumulative
Bauausgaben über12000 ermöglichen. Es gibt keine Schuldenfreigabe für Käufe,
Zins-/Kreditbuchung, automatische Rettung oder Änderung des12000er Startbudgets.
Alle Betrags-, Bewohner-, Ereignis-, Summen- und signed Funds-Überläufe werden
vor der ersten Tickmutation atomar verweigert.

## Schwierigkeit und aktuelle Wohnmission

Die bestehende gespeicherte ModelSource-Difficulty ist alleinige Auswahl.
Der Entwicklerstart erhält `--difficulty very-easy|easy|normal|hard|very-hard`;
Normal bleibt Standard. Eine ausdrücklich abweichende Auswahl bei Load wird
verweigert; fehlende Auswahl lädt die gespeicherte Difficulty. Kein normaler
Menü-/Recovery-Defaultwechsel und kein stiller Schwierigkeitswechsel im Spiel.

Die eigene authored Wohnmission verlangt gleichzeitig mindestens6 bewohnte,
nicht brennende Häuser, mindestens1 nicht brennendes Commonhaus ab aktuellem
PlainCottage5, tatsächliche Gesamtpopulation mindestens50, tatsächlich
vereinnahmte positive Haussteuer und Funds>=0. Alle aktuellen Werte und Grenzen
stehen im Einkommensfenster. Kein gespeicherter Win-Latch, kein Bonus, keine
Simulationspause; Fire, Rückentwicklung oder Schulden können das Ziel verlieren.
Die Mission ist kein importiertes Originalkarten-/Kampagnenziel.

## Technische Abnahme

Pflicht sind fünf wirklich bezahlte Difficultyfälle, Null-/positive Rate,
beide Hausfamilien/Entwicklung, tatsächlicher Einnahmebeleg, exakter Unterhalt,
reale Verschuldung/Bauverweigerung, Worker-/Goods-/Transporterhaltung,
Überlaufrefusal ohne Teilmutation, adversariale Ledger-/Modell-/Revisionsfälle,
F5/F9 vor/nach und mitten im Zyklus, separate deterministische20k-Prozesse
sowie geöffnete Softwareansichten. Gate D erfordert vollständige frische
Debug-/Release-/ASan+UBSan-Builds und vollständige registrierte Suites,
den gebundenen Legacy-/167-Kartenvergleich und die vorhandenen bezahlten
Service-/Goods-/Housingketten. Technische Gate-Abnahme und vollständige
Originalparität sind getrennt; Originalgrenzen, Sprites29 und native/human
Abnahme bleiben ausdrücklich sichtbar.


## Abgeschlossene technische Prüfung

OE27 und der begrenzte technische GateD sind am2026-10-19 bestanden:
[Finanzabnahme und Protokoll](finance-validation.md). Alle drei frischen
Vollbuilds und684 vollständigen Konfigurations-CTests sowie neue separate
Prozess-Replays, Legacy-/Kartenvergleich und zehn geöffnete Softwarebilder
sind tatsächlich ausgeführt. Gesamt-Originalparität bleibt PARTIAL.

Restore bindet außerdem die aus der gespeicherten Deadline direkt belegte
letzte natürliche Brandperiode. Ein Eingang exakt am Zündtick ist zulässig;
die folgenden600 Ticks einschließlich des Ablaufticks sind ausgeschlossen,
auch nach natürlichem Erlöschen. Rule10 besitzt keinen FireInspector und
speichert hierfür keine neue Historie. Frühere nicht mehr belegte Intervalle
werden nicht rekonstruiert. Echte bezahlte verzögerte Bauabläufe und die
unveränderte ursprüngliche Ein-Feld-Manipulation prüfen diese Grenze.
