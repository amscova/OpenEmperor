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

Rezepte, Ernte, Zuzug/Worker und Schule→Haus-Besuche enthalten eigene Regeln.
Originale Veranstaltungs-/Performer-Abläufe und Krankheit fehlen. Mehrere
Gebäude und Walker nutzen Fallbackbilder; passende Hausbilder decken Typen3–17
ab. Ein vollständiger bezahlter Aufstieg durch alle oberen Hausstufen und die
native menschliche Abnahme bleiben offen. Der unveränderte Vier-Haus-Plan ohne
Ausbau verlor im gemessenen100k-Lauf Geld und verpasste Food-Bedarfe; er ist
keine dauerhaft selbsttragende Musterstadt. Weitere Grenzen stehen in
[Known Issues](../../KNOWN_ISSUES.md).

Für einen Fehlerbericht Karte, Difficulty, Tick, Gebäude/Tool und Aktion nennen;
bei Grafikproblemen Screenshot und Zoom ergänzen.
