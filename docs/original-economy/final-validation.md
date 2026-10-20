# OE33: finales Prüfprotokoll

**Technische OE33-Berichts-/QA-Abnahme: PASS. Gesamt-Originalziel und Gate F:
PARTIAL.** Die fehlenden Originalfunktionen der [Endmatrix](final-comparison.md)
bleiben offen. Alle folgenden Prüfungen sind tatsächlich abgeschlossen; geplante
oder vorher abgebrochene Aufrufe werden nicht als bestanden gezählt.

Arbeitsbasis ist `2fbf27066ad277b15bcc4bbfc176bd3befb95c5e`. Alle585
Source-/Test-/Tool-/Resourcepfade bleiben über die vollständigen Builds und
Testläufe bytegleich. Ihr kanonischer Inventardigest ist
`be00e194e2d1998744218b503b113a946f87388cb189314061b81e9649e8a365`.
Alle261 Produktionsdateien bleiben unverändert; aktive Authority weiterhin
Rule10/Schema26/Policy3/Entwicklungsrevision27. Keine neue Wirtschaftsregel,
Saveversion, Darstellung oder implizite Migration.

## Vollständige neue Builds und CTests

Drei neue Ninja-Builds unter `.local/original-economy/oe33/{debug,release,sanitizer}/build`.
Die tatsächlichen Konfigurationen sind Debug, Release und RelWithDebInfo mit
ASan+UBSan. System-SDL3, eigene vorhandene Originaldaten und macOS-App-Bundle
OFF; keine alten Buildverzeichnisse überschrieben. Vor CTest wurde jeweils das
wirkliche Inventar mit `ctest -N --show-only=json-v1` ermittelt.

| Konfiguration | Vollbuild-Prozesszeit | CTests | CTest-Prozesszeit | SHA256 des Prozessbelegs |
|---|---:|---:|---:|---|
| debug | 443.673s |246/246| 2245.070s | `27238d185ddfc8a6bfaea35d61827a7c398bf730d10e33e232f84703b14fa7f1` |
| release | 604.931s |246/246| 179.248s | `1c8e22916fb7afd2bb8726c25e77217dc72baea0f03f6dc75b12cb0c967c0724` |
| sanitizer | 1169.069s |246/246| 720.006s | `5ff57ec0375aac91ff477e68551aa32a40a43b98835370aa6ff4e074148e8f3b` |

Insgesamt **738 Konfigurations-Testausführungen**, je246 unterschiedliche
registrierte Fälle,0 finale Fehler und0 CTest-Skips. Jede einzelne Passed-Zeile,
die vollständige ID-/Zählerliste1–246, CTest-Exit0 und der Quellenfreeze wurden
unabhängig nachgeprüft. Alle drei vollständigen Builds sind warnungsfrei.
`configure.json`, `build.json`, `inventory.json`, `full-ctest.json` sowie die
zugehörigen `.log`-Dateien enthalten tatsächliche argv, PIDs, Laufzeiten,
Umgebungsvariablen, Log-SHA und Quellen vor/nach jedem Prozess.

ASan läuft mit `detect_leaks=0:halt_on_error=1`, UBSan mit
`halt_on_error=1:print_stacktrace=1`. LeakSanitizer ist auf diesem Mac-Host
nicht unterstützt; ein Leakdetektor-PASS wird nicht behauptet. Die Laufzeiten
sind Ausführungsbelege unter gleichzeitiger QA-Last, keine neuen Performance-
Benchmarks. Die begrenzten seriellen OE32-Messungen bleiben im
[Longplaybericht](longplay-validation.md).

## Alter Bestand und aktuelle Originaldaten

Der vollständige neue `original_economy_baseline.py verify` erhält den
ursprünglichen Vertragsdigest
`12823612834d27099189c18e31ed4fed4761c9ff34486227b3075cf1069a49e5`.
167 Karten geprüft,165 vorbereitet und zwei bekannte Konfliktverweigerungen
unverändert;1485 Policy-/Fingerprintfälle,72 ganze komplexe SaveDocuments und
72 tatsächliche World-Snapshots exakt. Dazu zehn Bridge- und elf Ramp-
Neuprozesspaare. Der Beleg ist
`.local/original-economy/oe00/oe33-final-verification/run.json`, SHA256
`db13cfc5e7547e90c68711c1701aa25eb17ee9518fcc324029ec1c6d579537b5`.
Die gesamte erhaltene Referenz und der baseline-lock wurden nicht erneuert.

Zusätzlich exportiert jede der drei neuen Konfigurationen die Schemas1–25.
Jeweils101 ganze Dateien stimmen literal mit
`.local/original-economy/oe31/legacy-before2/corpus` und den bereits in OE32
festgehaltenen101 Hashes überein:303 aktuelle whole-file Paare. Das schließt
DTO, erzeugte/normalisierte SaveDocuments und Eingabefälle ein. Die vorhandene
1–3-Normalisierung bleibt identisch; sie ist keine neue Migration.
`{debug,release,sanitizer}/frozen-comparison.json` bindet jede Datei einzeln.

Alle1464 Originaldateien/819711091Bytes bleiben in SHA256, Modus und mtime
exakt zum vorab erfassten Inventar. AGENTS.md bleibt unverändert. Es wurden
keine persönlichen Saves oder PrefPaths untersucht. Originalbilder, Modelle,
EXE-Auszüge und erzeugte private Spielstände werden nicht committed.

## Neue bezahlte Langläufe und Neuprozesse

Die Release-Acceptance verwendet dieselbe unveränderte produktive World.
Jeder Ablauf besteht aus drei wirklichen Prozessen: bezahlter Seed mit2000
Warmupticks, frisch gekaufter Creator mit denselben2000 Ticks ohne Restore
seiner laufenden World und separater tatsächlicher Restore/Weiterlauf.
Danach werden Mitte und Ende verglichen. Die folgenden Zähler betreffen die
zusätzlichen20000/100000 Beobachtungsticks; Endticks22000 beziehungsweise102000.

| Neuer Ablauf | Beobachtungsticks | Endbevölkerung | Endfunds | Auf-/Abstiege | Echte Food-Versäumnisse | Warenankünfte, alle Legs |
|---|---:|---:|---:|---:|---:|---:|
| real-20k | 20000 | 167 | 3202 | 24/18 | 27 | 739 |
| real-100k | 100000 | 167 | -22128 | 82/76 | 116 | 3288 |
| districts-20k | 20000 | 503 | -43787 | 76/75 | 679 | 1213 |

Alle neun Prozesse endeten0. Je Ablauf stimmen sechs vollständige
Fortsetzungsdateipaare sowie ein volles Seed-/unrestored-Creator-Paar exakt:
insgesamt21 neue Paare. Alle42 Dateien wurden zusätzlich unabhängig wieder
geöffnet und nach Größe/SHA sowie literal verglichen. Je fünf Roh-Checkpoints
wurden geprüft, insgesamt15. Die drei deterministischen Endberichte stimmen
zusätzlich bytegenau mit den entsprechenden finalen OE32-Berichten überein.

Die frischen Ausgaben liegen wegen der unveränderten Schutzgrenze des OE32-
Werkzeugs unter `.local/original-economy/oe32/oe33-{real-20k,real-100k,districts-20k}-process/`.
Die neuen Aufrufbelege stehen unter
`oe33/release/final-{real-20k,real-100k,districts-20k}.json`.
Der Zwei-District-Fall bezahlt82 Gebäude/58 Couriers auf ausdrücklich gültigem
**authored** Gelände mit Originalmodell. Transportnetze sind getrennt;
Funds und Workforce bleiben stadtweit. Es ist kein originaler Xia-Stadtplan
und kein auf Balance optimierter Aufbau. Die beiden Starterfälle verwenden
unverändert die wirkliche Xia-Karte und das identifizierte Normal-Modell.

Die festen vier Häuser ohne weitere Expansion sind nach100k defizitär; die
116 tatsächlichen Food-Versäumnisse bleiben erhalten. Das beweist weder
generelle Unrentabilität jeder Rule10-Stadt noch lückenlose Versorgung.
Krankheit im Originalprofil bleibt unsupported/health-neutral; die echte
Legacy-Krankheitskontrolle ist davon getrennt. Kein Gratisgeld, keine
Bewohner-/Wareninjektion und keine nachträgliche Modellkorrektur.

## Historischer und heutiger Fork

Der heute gebundene Fork ist
`992f90ce3071f66cff3d82b31aaa7550b018adb5`.675 tatsächliche Git-Blobs und13
bytegleiche aktuelle Kern-/Ledgerdateien wurden unabhängig geprüft. Der
aktuelle Fork wurde nicht gebaut; seine bereits vorhandenen Frozen7–9-
Grundlagen werden ausdrücklich anerkannt. Eine Sourceidentität beweist
keine vollständige heutige Fork- oder Originalspielintegration.

Gegen den alten Studienpin
`d861d2912337c09751cb68196f85d5a30752726e` wurden in OE33 erneut je74 Fälle
in Debug/Release/ASan+UBSan ausgeführt:222 Kontrollen, separat von den738 CTests.
Je46 Paritäten,18 begründete Korrekturen, sechs ungeklärte Annahmen und vier
bewusst ausgeschlossene Verhaltensweisen; alle Erwartungen bestanden,
reale Modelldatei jeweils geprüft. Archive und Quellpins sind erneut gehasht.
Die Reports liegen unter `oe33/CONFIG/historical-parity/report.json`.
Diese neuen Ausführungen des alten Vergleichs werden nicht als Tests des
inzwischen entfernten alten Adapters im neuesten Fork ausgegeben.

## Erhaltene vorbereitende Fehler

Der erste private Sammelprozess nahm nach dem erfolgreichen Schemaexport
fälschlich einen Unterordner `frozen-after/corpus` an. Das vorhandene CLI
schreibt seine101 Dateien direkt nach `frozen-after`. Seine nachgelagerte
Pfadassertion war deshalb falsch; die tatsächlichen CTest-/Exportprozesse
laufen getrennt protokolliert weiter. `compare_frozen.py` prüft den richtigen
Ordner gegen dieselben101 literal erhaltenen Dateien und OE32-Hashes.
Kein Produktionscode und keine Testbedingung wurden dafür geändert.

Ein erster zusätzlicher20k-Aufruf wählte einen Ausgabepfad unter OE33 und
wurde vom bestehenden OE32-Schutz vor jedem Gameplay verweigert: Exit1,
`Output must be a new private OE32 directory`. Dieser Fehlaufruf bleibt in
`oe33/release/real-20k.json` und `.log` erhalten. Die oben ausgewiesenen drei
neuen positiven Abläufe verwenden frische erlaubte OE32-Unterpfade. Die
Schutzgrenze wurde nicht erweitert, kein Vorlauf überschrieben und kein
fehlgeschlagener Aufruf als PASS gezählt. Beide Korrekturen sind zusätzlich
in `oe33/{frozen-path-correction,longplay-invocation-correction}.json` beschrieben.

## Wiederholung

Aus dem Repository und mit eigenen vorhandenen Originaldaten, stets neue
Belegpfade verwenden. Die vollständigen tatsächlichen Befehle stehen auch in
den gehashten Prozessbelegen. Beispiel für einen neuen Release-Vollbuild:

```sh
cmake -S . -B .local/original-economy/oe33/repro-release -G Ninja -DCMAKE_BUILD_TYPE=Release -DOPENEMPEROR_USE_SYSTEM_SDL3=ON -DOPENEMPEROR_BUILD_MACOS_APP=OFF -DOPENEMPEROR_ORIGINAL_MODEL_TEST_ROOT="$PWD/.local/gog-extracted/app" -DOPENEMPEROR_ENABLE_SANITIZERS=OFF
cmake --build .local/original-economy/oe33/repro-release --parallel 2
ctest --test-dir .local/original-economy/oe33/repro-release -N
ctest --test-dir .local/original-economy/oe33/repro-release --output-on-failure --parallel 2
```

Für Debug einen getrennten Build mit `CMAKE_BUILD_TYPE=Debug` verwenden;
für Sanitizer `RelWithDebInfo` und `OPENEMPEROR_ENABLE_SANITIZERS=ON`, dann
CTest mit den oben genannten ASAN_OPTIONS/UBSAN_OPTIONS. Nach einem passenden
Build lauten die zusätzliche Reproduktion und der bytegenaue Referenzvergleich:

```sh
cmake --build .local/original-economy/oe33/repro-release --parallel 2 --target openemperor-original-longplay-acceptance
python3 tools/test_original_longplay.py --binary .local/original-economy/oe33/repro-release/openemperor-original-longplay-acceptance --data .local/gog-extracted/app --output .local/original-economy/oe32/repro-oe33-xia-20k --scenario starter --ticks 20000
python3 tools/test_original_longplay.py --binary .local/original-economy/oe33/repro-release/openemperor-original-longplay-acceptance --data .local/gog-extracted/app --output .local/original-economy/oe32/repro-oe33-xia-100k --scenario starter --ticks 100000
python3 tools/test_original_longplay.py --binary .local/original-economy/oe33/repro-release/openemperor-original-longplay-acceptance --data .local/gog-extracted/app --output .local/original-economy/oe32/repro-oe33-districts-20k --scenario districts --ticks 20000
python3 tools/original_economy_baseline.py verify --data .local/gog-extracted/app --build .local/original-economy/oe33/repro-release --reference .local/original-economy/oe00/reference/contract.json --output .local/original-economy/oe00/repro-oe33-verification
python3 tools/original_rules_parity.py --configuration release --simulation-library .local/original-economy/oe33/repro-release/libopenemperor-simulation.a --fork .local/original-economy/oe00/fork --data .local/gog-extracted/app --output .local/original-economy/oe33/repro-historical-parity
.local/original-economy/oe33/repro-release/openemperor-original-save-adversarial-tests --export-frozen-corpus .local/original-economy/oe33/repro-frozen
```

Fehlende Originaldaten und verweigerte oder nicht ausgeführte Pflichtprüfungen
sind kein PASS. Unveränderte Referenzen nicht neu erzeugen oder ersetzen.
Die [Spielerübergabe](player-handoff.md) beschreibt den kurzen normalen Menütest;
Native/Human bleibt **NOT_RUN**. Es entstehen kein Tag, Release, Push oder Merge.

## Abschließender Beleg

`.local/original-economy/oe33/final-validation.json`, 10073Bytes,
SHA256 `5a918e36819c13ae6fddecedb9ae0355ec3d5166f4bd04d2c0127a2169fa7c02`.
Er bindet alle tatsächlichen Konfigurations-/Prozess-/Corpus-/Hashbelege und
die beiden erhaltenen vorbereitenden Aufruffehler. Technische QA ist bestanden;
die [Originalfunktionslücken](final-comparison.md#übergabe-und-releaseentscheidung)
und Gate-F-Gesamtbewertung bleiben **PARTIAL**.
