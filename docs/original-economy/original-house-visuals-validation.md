# OE-29: technische Abnahme der Original-Hausgrafiken

OE-29 ist technisch PASS.15 belegte Typfamilien3..17 verwenden native Bilder
aus der lokal lizenzierten General-SG3, ausdrücklich statische Variante0.
[Quellennachweis](../reverse/original-house-visuals.md) und
[Vorabvertrag](original-house-visuals-runtime.md) binden Typ, Gruppe, physisches
Bild, tatsächlichen Körper und Frontanker. Rule10/Schema26/Policy3/Revision27
bleiben unverändert. Alle127 bisherigen Simulation-/Persistence-/Map-Quelldateien
sind bytegleich zum sauberen OE28-Start15dd6da.

Vacant2 behält wegen unterschiedlicher Constructor-/Setterkontexte einen
benannten Fallback. Der temporäre2×2-Eliteplot zeigt kein originales4×4-Bild.
Die individuelle Originalvariante und ganze Originalszene bleiben UNKNOWN;
Gesamt-Originalwirtschaftsparität bleibt PARTIAL. Native/human NOT_RUN.

## Tatsächliche Tests

Frische Debug-, Release- und RelWithDebInfo-ASan/UBSan-Ninjaverzeichnisse unter
`.local/original-economy/oe29/`, System-SDL3 und vorhandene Originalinstallation.
58 relevante CTests je Konfiguration:174 PASS,0 Fehler/Skips, keine finalen
Compilerwarnungen. ASan/UBSan verwendet `halt_on_error=1`; Leakerkennung wurde
deaktiviert und ist auf dieser Plattform nicht geprüft. Gate E folgt erst OE30.

| Konfiguration | CTests | Letzter Buildabgleich | Tests |
|---|---|---|---|
| debug |58/58 |30.015s |372.205s |
| release |58/58 |96.235s |52.948s |
| sanitizer |58/58 |194.572s |223.651s |

Die Buildzeiten sind die letzten Abgleiche gegen den eingefrorenen Stand;
vorherige vollständige frische Compilephasen bleiben zusätzlich erhalten.
Exakte Befehle, Prozess-IDs, Umgebungen, Laufzeiten, Inventare und Loghashes:
`{debug,release,sanitizer}/final-matrix-build.json`, `final-matrix-ctest.json`,
`final-matrix-ctest.xml` und `final-matrix-result.json`. `test-plan.json` nennt
alle58 CTests/52 Targets. Die Auswahl ergänzt bestehende Original-Views,
Legacy-/Save-/Input-/Menü-, Alpha-/Texture-/Road-/House-/Fire-/Bridge-/Ramp-/
Cliff-Pixel und die unveränderten gemeinsamen Registrierungstests.

Neue Fälle: `original-house-visual-profile`,
`original-house-visual-profile-real`, `original-house-visual-production-view`.
Der reale Assettest prüft den gepinnten EXE-Hash, tatsächliche Settertabellen,
alle15 Gruppen/SG3-Metadaten und native RGBA/Alpha gegen die Installation.
Synthetische Kontrollen prüfen Deduplizierung, genaue Typ-/Seiten-/Ankerbindung,
kohärenten Shelter/Hut-Tausch, falsche gleich große Variante, Spiegel-/Animations-/
Alpha-Metadaten, unsichere/fehlende Quellen sowie atomare Budgetrefusals.
Der Sprite-Append lädt alte Core-Texturen nicht erneut; Fehler nach einem bereits
hochgeladenen neuen Bild rollen nur den neuen Teil zurück.

## Bezahlte Darstellung und Bilder

Der SDL-Test verwendet unveränderte produktive World/View/Persistence und
Original-General-/SprMain-Bilder auf ausdrücklich authored Geografie und
Modellparametern. Sämtliche Käufe, Einzug, Güter-/Besuchsreisen und Stufenwechsel
entstehen durch reguläre Commands/Ticks; keine Snapshot-/Bestandsinjektion.
Echte Common2→3→4→5 und Elite11→13→12→13 wählen ihre vorbereiteten Bilder sofort.
Die übrigen belegten Typen haben Asset-/Identitätsnachweise; eine bezahlte
Spielszene jeder höheren Stufe wird daraus nicht behauptet.

Literal1×/2×/4× prüfen unabhängige Originalpixel, opake/transparente Auswahl,
Ownerauswahl, sichtbare Dächer bei außerhalb liegendem Fußpunkt, UI-Clipping,
tatsächliche bewegte Walkerpixel und signed-height Füße. Ein identischer
bezahlter World-Kontrollfall bindet authored Foreground-Streifen und ihre
Alphaöffnungen. Derselbe Rule10-World kauft eine40Funds-Rampe über1→0;
F6-off zeigt deren vorbereitete Erd-/Steinpixel und auswählbare Endpoints.
Rule6-only authored Klippenseiten bleiben aus; bestehende Cliff-/Bridge-/Ramp-
Pixelverträge werden getrennt geprüft. Dies beweist keine originale
Klippenidentität oder beliebige gesamte Originalkomposition.

F4/F6/F7/flat Snapshot, F5/F9 und privater Quellenverlust bestehen.100 wechselnde
Viewframes halten sämtliche zwölf Arbeits-/Authorityzähler0 und Snapshot,
Caches/Routen/Texturen unverändert. F9 dekodiert/lädt vorbereitete Bilder nicht
neu. Explizite Customkonfiguration in einer Legacy-World verweigert atomar;
Builtin bleibt dort benannt inaktiv und erhält die alten Core-Texturen.

Pro Konfiguration38 neue PNGs mit je Beobachtungsbericht und vollem Schema26-
Save, insgesamt114 neue PNGs. Zwölf finale Releasebilder wurden vom Root wirklich
geöffnet: Shelter1/2/4, Abandoned2, Recovered1/4, Walker, sichtbare Rampe2,
Foreground2, Kamera-Rand2, F4 und Snapshot. Raster/Fundamente, Typwechsel,
Verdeckung und Vergleiche entsprechen den geprüften Zuständen. Bei4× schneidet
der unveränderte Kartenausschnitt große Elitebilder ab. Leuchtend grüne
Testlandschaft und blaue Vordergrundstreifen sind authored Kontrollen.
Die sechs zusätzlich geöffneten Originalexportblätter bleiben wie alle Captures
und Proofs in `.local/`; kein proprietäres Bild/Archiv gelangt ins Git oder Bundle.

Die beiden unveränderten Sandbox-BMPs und alle22 bisherigen OE28-Inspector-PNGs
sind in jeder Konfiguration bytegleich zu ihren früheren Referenzen. Keine neue
vollständige167-Map-/historische72-Save-/20k-Replay- oder Gate-D-Abnahme.
Der frühere OE28-Abschlussreport ist unverändert; Original1464 Dateien/
819711091 Bytes samt Modi/mtime sind vor und nach den tatsächlichen Läufen exakt.

## Erhaltene Fehlversuche und Abschluss

Die ersten Versuche bleiben erhalten: noch fehlende Metadaten, dynamische Help-
Zeile im constexpr-Array, private Targetnamen-/SDL-Runnerfehler und eine gemischte
Auto-Deklaration. Der Negativtest lud seine absichtlich entfernte Bitmap zu früh
im Kontrollrenderer; nun hält er den vorher tatsächlich vorbereiteten Core im
Cache und prüft die unveränderten Daten nach der Extensionrefusal.

Die ersten vollständigen Debug-/Release-Auswahlen hatten57/58: zusätzliche
Fallbackcopy brach die bisher vollständig lesbare Zeile „Original housing art
pending“ um. Der alte Test blieb unverändert. Ohne vorbereitetes Profil ist die
alte Ausgabe nun exakt, neue Details gelten nur mit vorbereiteter Ergänzung.
Alle drei finalen58er Läufe folgten neu gegen denselben korrigierten Stand;
der während der Korrektur weitergelaufene alte Debuglauf ist kein Sourcefreeze-
Nachweis. Frühere fokussierte Läufe ersetzen die finale Matrix nicht.

Finale564 Quellpfade, Freeze SHA256
`46b08f5c02b6cc4482935cf4b340e0a09e812dbd92ea6130111c8788b6fe86d8`.
Abschluss `.local/original-economy/oe29/final-validation.json`, SHA256
`8e818ed46f68a11698f0bef041ba2f041186ad81636e3aef9a9da2f843bf7aa6`.
Lokaler OE29-Task-Commit; nächster vorgesehener Prompt ist OE30,
„Spielbarer bezahlter Originalwirtschaft-Starter“, noch unbegonnen.
