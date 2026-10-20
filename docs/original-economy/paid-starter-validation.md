# OE-30: technische Starter-Abnahme

Stand: **OE30 und Gate E technisch PASS**. Vorabvertrag:
[paid-starter-contract.md](paid-starter-contract.md), Bedienung:
[paid-starter.md](paid-starter.md).

## Gebundener Stand

Basis64cbfcb7c13a4b162cfb8bab118e45ef99b09d3d, aktive
Rule10/Schema26/Policy3/Entwicklungsrevision27. Der Starter kauft87 normale
Commands:68 Straßen und19 Gebäude einschließlich vier leerer Häuser. Die
unveränderte reale Normal-Modelldatei ergibt1404 Bauausgaben und10596 Funds
beiTick0. Die Position sucht eine vorhandene legale ebene24×13-Fläche;
Originalstraßen, Schutz und signedHeight werden nicht überschrieben.

Die571 Source-/Test-/Tool-/Resource-Dateien sind in
`.local/original-economy/oe30/source-freeze.final.json` gebunden,
SHA256 `cee0a5e724eff4d53215a01c83135e6b7efe4fe475bda9202871fd897791e942`.
Alle127 bisherigen Simulation-/Map-/Persistence-Dateien sind bytegleich zu
OE29; die beiden neuen Starterdateien sind separat ausgewiesen. Vergleich:
`authority-preservation-vs-oe29.json`, SHA256
`8e6d077984662e81fd56199adc94e097e286e896a21e7b06f14e67bc8bc3962a`.
Diese Sourceprüfung ersetzt keinen ausgeführten Save-/Policy-Replay.

## Vollständiges Gate E

Vor den CTests wurden alle drei tatsächlichen CTest-Inventare ermittelt:
je237 Tests. Die finalen Builds und vollständigen Suites halten denselben
571-Dateien-Freeze exakt; keine Compilerwarnung in den finalen Builds.

| Konfiguration | Vollständige CTests | Laufzeit |
|---|---:|---:|
| Debug |237/237 PASS|1082.678s|
| Release |237/237 PASS|184.612s|
| RelWithDebInfo, ASan+UBSan |237/237 PASS|576.937s|

Insgesamt711 finale CTests,0 Fehler,0 Skips. ASan-Memory und UBSan laufen mit
halt-on-error; der Leakdetektor ist auf diesem Apple-arm-Host nicht unterstützt.
Der tatsächliche Abbruch mit detect_leaks=1 ist erhalten; die finalen Laufzeiten
verwenden ausdrücklich detect_leaks=0 wie die vorherigen lokalen Abnahmen.
Dies ist kein LeakSanitizer-PASS. Alle1464 Originaldateien/819711091Byte sind
nach Abschluss in SHA, Dateimodus und mtime_ns exakt zum Anfang.

Finalreport: `.local/original-economy/oe30/final-validation.json`, SHA256
`7294e0ad54030cbca9547136488221cd1c69c63ba384cbfca6cfda4c9109c94d`.
Er bindet Build-/Inventar-/CTestbefehle, PIDs, Logs, Binaries, Sourcefreeze,
Prozess-/Pixel-/Pfadnachweise, Legacyvergleich und vollständige Originalinventare.
Nur ein lokaler Task-Commit; kein Push/Merge/Tag/Release. OE31 bleibt unbegonnen.

## Bereits tatsächlich gemessene Vorprüfungen

Release: `original-menu-session-flow` PASS und die beiden
`original-paid-starter-{synthetic,real}` PASS. Die echte Integration liest
unveränderte Xia-Grafikvorbereitung, originale Policy3 und das reale Normalmodell.
Der strenge Abstiegsnachweis verlangt eine aktuelle Plain5→Hut4-Transition
mit abgelaufenem Ancestor, belegtem Vorbesuch, bewohnten Häusern und erfüllten
übrigen aktuellen Bedürfnissen. Eine reine Testgegenprobe setzt Ancestor nur
im kopierten Regelinput, niemals in World oder Save. Vorherige anders bedingte
Rückentwicklungen zählen nicht als dieser Nachweis.

Entwicklungsproben deckten drei falsche Testannahmen auf: Original-Food wird
nicht im alten Farmzähler geführt; der erste Hut nach einer längeren
Straßentrennung muss noch kein Ancestor-Ausfall sein; importierte Originalstraßen
sind gültige, kostenlose No-op-Klicks. Die Tests prüfen jetzt die getrennte
Original-Food-Masse samt Hausverbrauch, den konkreten Transitioninput und einen
tatsächlich neuen bezahlten Straßenkauf. Es wurde dafür keine Runtime gelockert.
Frühe Fehlversuche und Warnungen bleiben in den privaten Logs erhalten.

## Echte Prozess- und Bedienbelege

Alle drei Konfigurationen haben je einen tatsächlichen Creator- und einen
separaten Resumeprozess ausgeführt. Der bezahlte Xia-Plan liegt bei(102,120).
Alle vier Häuser erreichen mindestens Plain beiTick3559 mit167 Bewohnern;
Hemp13 und Ceramics4 sind bereits wirklich verbraucht. Nach regulärer
Straßentrennung kostet die Reparatur8 Funds und erzeugt bei4319 eine spätere
Herbalist-Ankunft. Ancestor-Pause4319 ergibt bei6721 den nachgewiesenen
bewohnten5→4-Abstieg mit Counter9, Water=true, Food80 und Ancestor0;
reguläre Wiederaufnahme ergibt Erholung6823.

Ab6823 laufen echte20000 weitere Ticks. Midpoint16823 und End26823 stimmen
zwischen Creator und neuem Prozess vollständig überein. Alle15 ganzen
Restart-Dateipaare und32 Rohdateien über alle drei Konfigurationen sind
bytegleich. Endfunds1500,167 Bewohner und alle sechs Warenbilanzen sind
belegt; Min-/Max-Funds wurden nicht erhoben. Matrix:
`.local/original-economy/oe30/process-matrix-final.json`, SHA256
`d37fc60fd77fedf9c1e0e1a97a5bb93cdc99021fa932d2fc5c3afe7e0852785d`.

Weitere sechs Prozesse verwenden ausschließlich die produktive MenuSession,
tatsächlich gequeuete SDL-Ereignisse und den normalen TickDriver. O→Start kauft
den Starter, echte Cargo-Bewegung ist bei192 aufgenommen, vier entwickelte
Häuser bei3560. Toolbar-Kauf/Entfernung/bezahlt erneuerte Straße, F5,
160 weitere Ticks, F9 und separater manueller Menü-Restore vergleichen die
vollständigen SaveDocuments. Je17 Software-PNGs sind zwischen allen drei
Konfigurationen bytegleich. Elf finale Releasebilder wurden vom unabhängigen
Bildreview geöffnet; Root prüfte zusätzlich Setup, Aufstieg1×/2×/4×, Courier
und das900×600-Menü. Bericht: `menu-acceptance-visual-review.json`.

Neun zusätzliche Prozesse prüfen900×600-Bedienung und einen VeryHard-Save
mit separatem Prozess-Restore und F9. Die drei Mindestgrößenbilder sind in
allen Konfigurationen exakt gleich; Originalsetup und neuer Eintrag sind lesbar.
Der alte rechts abgeschnittene Main-Autosavebutton bei900×600 wurde nicht
verändert; daraus wird keine vollständige Minimum-UI-Abnahme abgeleitet.
Bericht: `menu-additional-validation.json`, SHA256
`2e89cbd17f33dea06fdf9320635acbecbcbf93917c796b1226cc415f12a6462f`.
Neun tatsächliche Pfadschutzproben verweigern Quellenüberlappung, Symlink und
alte/unvollständige Ausgaben. Alle finalen Sourceinventare bleiben exakt.

## Erneut ausgeführter Legacyvergleich

`tools/original_economy_baseline.py verify` lief mit dem neuen Releasebuild
vollständig gegen die eingefrorene OE00-Baseline. Ergebnis:72 ganze
SaveDocuments/Snapshots,1485 Policyfälle und Corpus167/165/2 exakt, einschließlich
elf Rampen- und zehn Bridge-Neuprozesspaaren. Vertragsdigest unverändert
`12823612834d27099189c18e31ed4fed4761c9ff34486227b3075cf1069a49e5`.
Neuer unveränderlicher Ordner:
`.local/original-economy/oe00/oe30-final-verification/`, `run.json` SHA256
`4abaf3e4cde36e913420bb3cd9bb3e4ce9984ecd6588e31879893ed63e3dfdce`.
Der erste Aufruf mit OE30-Ausgabepfad wurde korrekt vom unveränderten
OE00-Pfadguard vor jeglichem Lauf verweigert; der zweite nutzte einen neuen
zulässigen Ordner. Alte Baseline-/OE29-Belege wurden nicht überschrieben.

## Grenzen

Original-Recovery bleibt bis OE31 gesperrt, die Anwendung zeigt manuelle F5/F9-
Saves. Bestehende Autosavepräferenzen und ältere Recoveryzweige bleiben erhalten.
House-/Produktionsabriss bleibt verweigert; nur der bereits erlaubte neutrale
Well72-Einflussquellenabriss wird positiv geprüft. Der Starter ist authored,
kein originaler Stadtstart. Music behält den bisherigen experimentellen
Schule→Haus-Vertrag. Nicht-Haus-Sprites und Vacant-/Plot-Fallbacks sind weiterhin
offen. Software-SDL, separate Prozesse und menschliche/native Bewertung sind
verschiedene Evidenztypen. Kein Human-/Gesamtparitäts-PASS wird abgeleitet.
