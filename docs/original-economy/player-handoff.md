# Originalwirtschaft spielen

Die optionale Originalwirtschaft bietet bezahlte Gebäude, Produktion, echte
Kurierlieferungen und Hausentwicklung. Sie ist ein spielbarer Entwicklungsstand;
vollständige Originalspiel- und Grafikparität fehlen noch.

## Start und Ausbau

Nach dem [Build](../../README.md) mit der eigenen Emperor-Installation starten:

```sh
./build/openemperor --data /pfad/zur/Emperor-Installation
```

Im Hauptmenü **O Original economy → Cities/Xia.map → Normal →
Paid four-house starter → Start sandbox** wählen. Die Stadt beginnt pausiert.
**Space** startet die Simulation; **1x / 2x / 4x** unten ändern ihr Tempo.

Für den separat versionierten Brandschutz im Hauptmenü **F Original fire
protection** wählen. Das startet **Rule11 / Schema27 / Policy3** mit derselben
bezahlten Stadt. **O Original economy** bleibt Rule10; bestehende Saves werden
nicht umgewandelt. Über **F FireWatch $80 +2 workers** bis zu zwei Wachen
regulär kaufen und mit Straßen verbinden. Jede Wache benötigt zwei tatsächlich
zugeteilte Mitarbeiter und laufenden Betrieb für neue Inspector-Fahrten.

Erst die echte Ankunft eines Inspectors löscht einen Brand und schützt das
besuchte Gebäude für 2400 Ticks. Bei **Select** zeigen Gebäude und Häuser
Brandstatus und verbleibenden Schutz; die Wache zeigt Betrieb und Staffing,
Walker die laufende Fahrt. Wachen können selbst brennen und brauchen die andere
Wache als Besucher. Betriebspausen stoppen neue Fahrten, bereits entsandte
Inspectors beenden ihre Fahrt. Brandschutz beseitigt das Wirtschaftsdefizit
nicht; Kaufpreis und laufender Unterhalt kommen hinzu.

**S Sustainable economy** startet separat **Rule12 / Schema28 / Policy3**.
Hier sind drei regulär bezahlte Food-Farmen mit jeweils eigenem Feld erlaubt;
der Unterhalt von 2 Funds pro benötigtem Mitarbeiter fällt alle 816 statt 400
Ticks an. Alle Originalmodellwerte, Steuern, Erntezeiten und Brandschutzregeln
bleiben erhalten. Alte Städte behalten ihre Regeln. Für den gezielten
[Zwölf-Häuser-Ausbau](oe37-sustainable-economy.md) **Empty** wählen; der
Vier-Häuser-Starter ist weiterhin eine eigene bezahlte Startoption.
Der geprüfte Ausbau hält zwölf bewohnte Häuser und 318 Einwohner über 100000
Ticks nach der Anlaufphase; vier volle 20400-Tick-Fenster erwirtschaften je 1950
Funds nach sämtlichem Unterhalt. Das belegt diesen begrenzten Bauplan, keinen
Gewinn für jede Stadt oder einen vollständigen Aufstieg zu höchsten Hausstufen.

**M Needs-aware logistics** startet ausdrücklich **Rule13 / Schema29 / Policy3**.
Der Modus behält die drei Food-Ketten, den 816-Tick-Unterhalt, Brandschutz,
Preise und Transportgrenzen von Rule12. Warehouse und Market wählen Waren
nach tatsächlichem Hausbedarf: fehlender aktueller Bedarf zuerst, danach
Vorrat und erfüllbare nächste Stufen. Erst echte Lieferankünfte füllen die
Häuser. Die Auswahl ist eine eigene OpenEmperor-Regel; bestehende Rule12-Städte
werden nicht umgestellt. Der direkte Start mit bezahltem Starter lautet:

```sh
./build/openemperor --data /pfad/zur/Emperor-Installation --sandbox Cities/Xia.map --sandbox-rules sandbox-city-v16 --original-market-logistics
```

Die [Marktdiagnose](oe39-market-diagnosis.md) erklärt die gemessenen Rule12-Lücken.
Die [separate Langzeitbewertung](oe39-market-retention.md) hält Warenversorgung,
Hausstufen, Brände und Finanzierung auseinander; der neue Modus allein belegt
keine durchgängige Versorgung oder brandsichere Stadt.

Der [bezahlte High-Tier-Ausbau](oe38-paid-high-tier-progression.md) erreicht
Common10 (Luxurious Apartment). In den geprüften20400 Folgeticks entstehen
Hemp-/Ceramics-Lücken und Rückentwicklungen trotz positivem Betriebssaldo;
ein zweiter Markt allein behebt sie nicht. Elite17 ist im unveränderten
Normal-Rule12-Vertrag geometrisch blockiert: Die zugelassenen Quellen und
Footprints erlauben höchstens DES79, der Aufstieg16→17 verlangt80.

Der Starter bezahlt Straßen, vier Häuser, Wasser, Ancestor, Market, Food-/Hemp-
Ketten, Clay/Pottery/Warehouse, Herbalist, Acupuncture und Music. Mit der
geprüften Modelldatei bleiben **10596 von 12000 Funds**. Bewohner und Waren
beginnen bei **0**: Zuzug, Arbeit, Produktion und tatsächliche Ankünfte folgen
erst beim Spielen. Eine gebaute Quelle erfüllt allein keinen Hausbedarf.

Der Bauplan braucht eine legale ebene **24×13-Fläche**. Scheitert die Suche,
eine andere Karte oder **Empty** wählen. Für das Wohnziel sind mindestens
sechs Häuser nötig; zusätzliche Häuser und Versorgung regulär kaufen.
Auch pausierte Gebäude kosten Unterhalt. Häuser und Produktionsgebäude sind
noch nicht abreißbar; Abriss gilt nur für neutrale Well72-Einflussquellen.

Der [geprüfte Ausbauplan](paid-progression.md) ergänzt zwei Häuser, vier Straßen
sowie je eine Food-/Hemp-Farm mit Feld für 266 Funds. Er belegt das Wohnziel und
ein begrenztes Versorgungsfenster; Brände und laufende Verluste bleiben offen.

## Bedienung und Versorgung prüfen

- **Pfeiltasten** verschieben, Mausrad oder **Z** zoomen; **H / Help** erklärt
  die Bauwerkzeuge. **.** schaltet bei Pause einen einzelnen Tick weiter.
- **5 Select** wählt Gebäude. Beim Haus wechselt **PageUp / PageDown** zwischen
  **State / Needs / Goods / Visits**. Dort stehen aktuelle/nächste Anforderungen,
  vorhandene Bestände und tatsächliche Besuche. **F10** wechselt die Overlays
  **Off / Supply / DES / Risk**.
- **F4** vergleicht Gebäudegrafik und Fallback, **F6** Straßenmaterial.
  Benannte Fallbacks zeigen noch fehlende Originalgrafiken.
- Für einen Versorgungstest beide Ancestor-Gebäude mit **Pause operation**
  pausieren. Nach ausbleibenden Besuchen und negativen Hausauswertungen folgt
  Rückentwicklung; **Resume operation** und verbundene Straßen erlauben neue
  Besuche. Timer und Hausentwicklung reagieren verzögert.

## Speichern und wiederherstellen

**F5** speichert außerhalb der Originaldaten, **F9** lädt denselben Stand
pausiert. Nach einem Neustart **L Load Save → Open selected** wählen.
Difficulty, Modell- und Kartenquelle müssen passen. Alte Entwicklungsrevisionen
werden mit Diagnose verweigert; ein fehlgeschlagener Ladevorgang erhält die Stadt.
Rule10/11/12/13 laden jeweils ihren eigenen Stand mit Schema26/27/28/29.
F5/F9 und Recovery ändern dabei keine Regelgeneration.

**Autosave: ON** im Hauptmenü aktiviert Recovery: ein geschützter Startpunkt
und bis zu fünf Checkpoints im Abstand von **60 simulierten Sekunden**.
**Load Save → Show recovery history → Open selected** öffnet einen Checkpoint
pausiert in einer eigenen Historie und erhält seine Quelldatei. Erfolgreiches
F9 beginnt ebenfalls eine neue Historie; manuelle Saves bleiben separat.

## Grenzen und technische Einordnung

Originalwirtschaft bleibt ausdrücklich optional: **City-v16 Rule10 / Schema26 /
Policy3 / Entwicklungsrevision27**. Frühere Regeln und Saves behalten ihre
Verträge. Modelldaten werden beim Start vorbereitet; Originaldateien bleiben
unverändert. [Modellherkunft](model-provenance.md) und
[Kernvergleich](core-parity.md) dokumentieren die belegten Regeln.

Rule11 nutzt dieselbe Wirtschaftsrevision27 und einen zusätzlichen authored
Brandschutzvertrag mit eigener Save-Prüfung. Die Preise und Mitarbeiter der
FireWatch stammen aus OpenEmperor, nicht aus verifizierten Originalwerten.
Ihre begrenzte Besuchs-/Steuerhistorie kann voll werden; dann pausiert die Stadt
vor dem nächsten Tick mit Diagnose, statt einen unprüfbaren Zustand zu erzeugen.
Der letzte Stand bleibt speicherbar. Grenzen und Messwerte stehen im
[Brandschutzvertrag](oe36-fire-protection.md).
Rule12 verwendet denselben begrenzten Brandschutz und eine eigene Save-Version;
auch seine sparsamere Finanzierung garantiert keinen Gewinn für jeden Ausbau.
Rule13 übernimmt diese Grenzen. Seine bedarfsabhängige Warenauswahl erhöht weder
Hauslager noch Kurierzahl, Transportmenge, Geschwindigkeit oder Brandschutz.

Rezepte, Ernte, Zuzug/Worker und Schule→Haus-Besuche enthalten eigene Regeln.
Originale Veranstaltungs-/Performer-Abläufe und Krankheit fehlen. Mehrere
Gebäude und Walker nutzen Fallbackbilder; passende Hausbilder decken Typen3–17
ab. Common10 wurde bezahlt erreicht; Elite17 bleibt durch den beschriebenen
Rule12-DES-Vertrag blockiert. Die native menschliche Abnahme bleibt offen.
Der unveränderte Vier-Haus-Plan ohne
Ausbau verlor im gemessenen100k-Lauf Geld und verpasste Food-Bedarfe; er ist
keine dauerhaft selbsttragende Musterstadt. Weitere Grenzen stehen in
[Known Issues](../../KNOWN_ISSUES.md).

Für einen Fehlerbericht Karte, Difficulty, Tick, Gebäude/Tool und Aktion nennen;
bei Grafikproblemen Screenshot und Zoom ergänzen.
