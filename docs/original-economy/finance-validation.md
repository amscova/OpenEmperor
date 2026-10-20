# OE27: Schwierigkeit, Haussteuern und Wohnmission

**PASS am2026-10-19: OE27 und begrenzter technischer GateD.** Vollständige
Originalparität und Gesamtabschluss bleiben PARTIAL. Basis ist
`bcba05334494b4f6f5afd7d4c0d2d76a13469caa`; geprüft wird ausschließlich
Rule10/Schema26 mit Entwicklungsrevision27 und unveränderter policy3.
Der [Laufzeitvertrag](finance-runtime.md) und der
[Originalnachweis](../reverse/original-house-tax-contract.md) erklären die Regeln
und ihre Herkunft. OE28 bleibt unbegonnen.

Fünf gebundene Schwierigkeitsgrade verwenden die vorbereiteten Modellwerte.
Haussteuer entsteht aus tatsächlichen Bewohnern und der aktuellen Modellrate.
Unterhalt hängt von den installierten erforderlichen Workern ab. Die aktuelle
Wohnmission verlangt gleichzeitig sechs bewohnte, nicht brennende Häuser,
ein nicht brennendes Common-Haus ab Plain Cottage, 50 Bewohner, positive
vereinnahmte Steuer und nichtnegative Funds. Das Ziel bleibt verlustfähig und
vergibt keinen Bonus.

## Entwicklerstart

```sh
sh tools/play_original_economy.sh --difficulty normal
```

Der Start ist pausiert; T zeigt Finanzen und Wohnmission, F5/F9 speichert/lädt.
Erlaubte Namen sind `very-easy`, `easy`, `normal`, `hard`, `very-hard`.
`--load` übernimmt ohne explizite Auswahl die gespeicherte Difficulty.
Das Entwicklerprofil bleibt opt-in; der bezahlte Starter gehört zu OE30.

## Vollständige technische Abnahme

| Konfiguration | Frischer Vollbuild | Vollständige CTests |
|---|---|---|
| Debug | PASS | 228/228 PASS |
| Release | PASS | 228/228 PASS |
| RelWithDebInfo mit ASan/UBSan | PASS | 228/228 PASS |

Je Konfiguration wurden alle normalen Targets und alle 25 zusätzlichen
Prüfprogramme aus einem leeren Buildverzeichnis gebaut: 201 Executables und
13 Bibliotheken. Die finalen Suiten sind ungefiltert; Warnungen, Fehler,
übersprungene Fälle und Sanitizer-Diagnosen wurden getrennt geprüft:
684 PASS, kein Fail/Skip, keine Warnung oder Sanitizer-Diagnose.
Die genauen Befehle, Laufzeiten, JUnit-Dateien, Quell-/Binaryhashes und
Originalinventare stehen in den tatsächlichen Phasenberichten unter
`.local/original-economy/oe27/gate-d/final4/`.

Die zusätzlichen Abnahmen wurden ebenfalls neu ausgeführt:

- Fünf tatsächlich bezahlte Fälle mit dem Originalmodell erreichen die
  Wohnmission und vereinnahmen positive Steuer. Im Normal-Fall bei Tick 1.224:
  181 Bewohner, 1.083 Funds Baukosten, 178 Funds Steuer, 204 Funds Unterhalt und
  10.891 Funds Restbestand. Die Gleichung lautet exakt
  `12000 + 178 - 1083 - 204 = 10891`.
- Nullraten, leere und brennende Häuser, beide Familien, Rückentwicklung,
  Zielverlust, pausierte/unbesetzte Gebäude, tatsächliche Schulden,
  Kauf-/Reparaturverweigerung und durch Einnahmen finanzierte kumulative
  Baukosten über 12.000 sind geprüft. Alle 15 negativen vorbereiteten TaxRates
  werden verweigert. Worker, Waren, Reservierungen und laufende Wege behalten
  ihre bestehenden Regeln.
- Die Geldarithmetik deckt uint64-Grenzen und INT64_MIN ab. F5/F9 funktioniert
  vor/nach und mitten im Zyklus; manipulierte Ledger, letzte Eingänge,
  Monatsgrenzen, Modell-/Difficultybindungen und Revisionen26/28 werden
  verweigert. Die riesige historische Nullratenprobe ist ausdrücklich
  synthetisch, kein astronomischer Spielverlauf.
- Der Finanzrunner startet 55 tatsächliche C++-Prozesse, davon 50 getrennte
  20k-Fortsetzungen. Der unabhängige Reader prüft 135 vollständige Rohdokumente,
  100 wiederholte Artefaktpaare und 20 Vergleiche mit der Inprozessreferenz.
  Die bisherigen Produktionsketten starten 68 Prozesse mit 51 getrennten
  20k-Fortsetzungen: 260 SaveDocuments, 255 Rohprojektionen sowie jeweils
  102 vollständige Save- und Reportpaare werden unabhängig verglichen.
- 72 historische SaveDocuments/WorldSnapshots und 1.485 Policyfälle sind
  literal zur eingefrorenen Baseline. Alle 167 Kartenmanager sind vollständig;
  165 Karten bleiben vorbereitbar, dieselben zwei Konflikte verweigert.
  Regeln7/8/9 behalten 17 Save- und 16 Report-Literalpaare. Die alten Steuersätze,
  Defaultprofile, Bridge-/Ramp-Authority und Schemas1–25 bleiben erhalten.
- Alle 16 numerischen Original-Kerndateien sind bytegleich. 3×74 Kernvergleiche,
  25 strenge Sanitizerprozesse und 28 Postexport-Pythonverträge bestehen.
  `core-parity.json` hat SHA256
  `bb42da371d165d0b52ee4c71d0720e7a190df407e2eaaf4b8887e6d773ea9221`.
- Zehn finale Software-SDL-Bilder wurden tatsächlich geöffnet; ihre 30
  Bild-/Save-/Beobachtungsartefakte sind gebunden. Alle Bilder sind bytegleich
  zum vorherigen gültigen Rendering. Sechs CLI-Ausgaberefusals und zehn
  Play-Optionsprüfungen bestehen. Initialer Load übernimmt die gespeicherte
  Difficulty; explizite Konflikte und abweichendes F9 werden atomar verweigert.

## Gefundene Fehler und tatsächliche Korrekturen

Die ersten vollständigen final3-Suiten bleiben als tatsächlicher FAIL erhalten:
je 226 PASS, zwei Industrie-FAIL, kein Skip. Ein später Testkauf ignorierte
inzwischen tatsächlich entstandene Schulden. Der positive 60-Funds-Kauf mit
refundfreiem Abriss erfolgt jetzt vor dem ersten Tick; der späte Ablauf prüft
Validierungs- und Ausführungsverweigerung samt vollständigem Snapshot.
Güterketten, Paper/Academy-Beleg und kompletter 20k-Replay bleiben bestehen.

Eine zusätzliche unabhängige Gegenprobe fand eine Produktionslücke beim
Restore: Im echten Stand bei Tick10.000 wurde nur der letzte Steuerzeitpunkt
von8976 auf9792 verschoben. Die belegte natürliche Brandperiode9740–10340
schließt diesen Eingang aus; der alte Restore akzeptierte ihn dennoch.
Der neue aktive Financeguard verweigert exakt die belegte letzte Brandperiode.
Am Zündtick darf Pre-Tick-Steuer anfallen, während des Brands und am Ablauftick
nicht. Echte verzögerte Käufe erzeugen alle Grenzproben ohne eingespritzten
Brandzustand, einschließlich der Prüfung nach natürlichem Erlöschen.

Die ursprünglichen Kontroll-/Manipulationsdateien wurden unverändert gegen
den frischen final4-Release geprüft: Die Kontrolle läuft 20k weiter und alle
fünf vollständigen Ausgaben bleiben bytegleich; dieselbe Manipulation scheitert
vor Fortsetzung. Wegen dieser Produktionskorrektur wurden alle drei Builds,
Suiten und Nebenprüfungen vollständig neu ausgeführt. Der vorbereitete reine
Test-Korrekturlauf wurde nicht ausgeführt oder als Abnahme gezählt.

Frühere Fehlversuche bleiben sichtbar: anfängliche Compiler-/Runnerannahmen,
der vor Export veraltete Paritätsbericht, supersedierte final1/2-Läufe,
die ursprüngliche Überlauf-Gegenprobe sowie private Archivindex-/Reportschema-
Prüferfehler. Die erfolgreiche Abnahme ersetzt deren tatsächlichen Status nicht.
Eine nachträgliche Scope-Klarstellung hatte einen privaten Berichtshash
verändert. Die ursprünglichen Bytes sind anhand des eingefrorenen SHA256
exakt wiedergewonnen; beide Fassungen und die Wiederherstellung bleiben
gebunden. Keine historischen Ausführungen wurden dadurch neu behauptet.

## Herkunft, Grenzen und Protokolle

Der Originalnachweis belegt Bewohner×Modellrate als Zwischenwert. Die
816-Tick-Kadenz, Funds-Normalisierung, Erhebungsberechtigung, Unterhalt und
Wohnmission sind ausdrücklich eigene OpenEmperor-Regeln. Originale TaxOffice-,
DES-, Rundungs- und Collectorabläufe sind nicht vollständig rekonstruiert.
Die begrenzten historischen Eingänge sind kein vollständiges Ereignisjournal.

Technischer GateD und vollständige Originalparität bleiben getrennt.
Höhere Original-Hausstufen sind durch die bekannten Szenengrenzen weiterhin
nicht pauschal abgenommen; es gibt keinen Heavenly- oder Balance-PASS.
Die geöffneten Finanzbilder nutzen eine bezeichnete synthetische Szene.
Original-Haussprites gehören zu OE29; native/human bleibt NOT_RUN.
Der Gesamtabschluss und vollständige Originalparität bleiben PARTIAL.

Die Installation mit 1.464 Dateien/819.711.091 Bytes bleibt in Bytes, Modi und
Änderungszeiten unverändert. EXE und Model wurden nur gelesen; Originalbilder
oder umfangreiche Originaltabellen gehören nicht zum Commit.
Finaler Quellstand über 556 Dateien:
`4f98327153c9d2cff94ee13ca8d21be0e9943342542d7517fcaee5057839baf1`.

Die privaten Belege liegen unter `.local/original-economy/oe27/`:
`gate-d/final4/`, `production-fixtures/{final4,finance-final4}/`,
`validation/view-final4/` und `validation/final-validation.json`.
Der neue literal gebundene Legacyvergleich steht unter
`.local/original-economy/oe00/oe27-final4-verification/`.
`gate-d/final3/`, die früheren Rohprüfungen und der ursprüngliche
`production-fixtures/last-fire-proof/report.json` bleiben als historische
Belege erhalten. Nur der eigene lokale OE27-Commit ist beauftragt;
kein neuer Push, PR, Merge, Tag oder Release gehört zu diesem Task.

Finaler Abschlussbericht SHA256:
`c8028074f2a8e3d05dd7181db49ce0e84045c0201951ad64bab9ffbab1ec9bcc`.
