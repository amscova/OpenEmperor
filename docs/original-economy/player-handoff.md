# Originalwirtschaft: kurzer Spielercheck

Der optionale Stand ist **City-v16 Rule10 / Schema26 / Policy3 / Revision27**.
Er besitzt echte bezahlte Gebäude, Produktion, Lieferungen, Hausentwicklung,
manuelle Saves und Recovery. Gesamt-Originalparität bleibt **PARTIAL**.
Die technische Prüfung ersetzt keine native visuelle oder menschliche Abnahme;
dieser Spielercheck ist **NOT_RUN**, bis ein Mensch seine Beobachtungen meldet.

## Start

Nach dem normalen [README-Build](../../README.md#build-and-run) im Repository:

```sh
./build/openemperor --data .local/gog-extracted/app
```

Die eigene installierte oder extrahierte Emperor-Datenquelle muss vorhanden
sein; einen anderen Pfad bei `--data` einsetzen. Originaldateien bleiben
read-only. Der normale Menüstart ist wichtig: Das separate
`tools/play_original_economy.sh` öffnet eine leere Entwicklersitzung und prüft
den Menü-Starter und dessen Recovery nicht.

Im Hauptmenü **O Original economy** wählen. Mit **Previous map / Next map**
**Cities/Xia.map**, mit **D Difficulty** **Normal** und mit **P**
**Paid four-house starter** auswählen. **Start sandbox** öffnet die Stadt
pausiert. Sie beginnt mit bezahlten Gebäuden, **0 Bewohnern und 0 Waren**;
bei der geprüften Modelldatei bleiben **10596 von 12000 Funds**.

## In etwa zehn Minuten prüfen

1. **Space** startet/pausiert, **.** schaltet einen pausierten Tick weiter.
   Unten **1x / 2x / 4x** verändert das Spieltempo. Erst bei laufender
   Simulation ziehen Bewohner zu, werden Worker verteilt und Güter produziert.
   Erwartung: Kurierfahrten erreichen wirkliche Ziele, Häuser verändern sich
   nach erfüllten Anforderungen. Ein Gebäudekauf allein erfüllt keinen Bedarf.
2. **5 Select**, dann ein Haus oder seinen Eintrag in der rechten Liste
   anklicken. **PageDown / PageUp** wechselt **State / Needs / Goods / Visits**.
   Typ, Bewohner, fehlende aktuelle/nächste Anforderungen, Bestand und
   tatsächliche Besuche ansehen. **F10** schaltet **Off / Supply / DES / Risk**;
   eine erreichbare Quelle oder ein fahrender Walker verspricht keine Ankunft.
3. Mit **Pfeiltasten** verschieben, Mausrad zoomen und **Z** zwischen
   **1× / 2× / 4×** wechseln. Häuser, Walkerfüße, Überdeckungen und Auswahl
   an Bildrändern ansehen. **F4** vergleicht Gebäudegrafik/Fallback,
   **F6** unabhängig Straßenmaterial. Beide wieder zurückschalten.
   Fenster verkleinern/vergrößern; **Help** oder **H** öffnen/schließen.
   Erwartung: lesbare Bedienung, korrekte Auswahl und keine versehentlichen Käufe.
4. **F5** speichern und den Tick merken. Weiterlaufen lassen, dann **F9**:
   erwartet sind derselbe gespeicherte Tick und Zustand, jetzt pausiert.
   Nach regulärem Beenden/Neustart **L Load Save → Open selected** für denselben
   manuellen Stand wählen. Difficulty, Datenquelle und Entwicklungsrevision
   müssen passen; unpassende Stände werden mit Diagnose verweigert.
5. Für Recovery im Hauptmenü **Autosave: ON** setzen, mit **Enter Continue**
   zurückkehren und mindestens 60 simulierte Sekunden laufen lassen.
   **Escape → L Load Save → Show recovery history**, mit **Previous / Next**
   einen Checkpoint wählen, dann **Open selected**. Erwartung: pausierter
   Checkpoint mit eigener neuer Historie; der Ausgangspunkt bleibt erhalten.
   Recovery hält einen geschützten Start und höchstens fünf Checkpoints.

Optional: Beide Ancestor-Gebäude einzeln auswählen und **Pause operation**
setzen. Fehlende spätere Besuche und negative Hausauswertungen beobachten;
mit **Resume operation** wieder aufnehmen. Timer und Hausentwicklung reagieren
verzögert. Der [Starterguide](paid-starter.md) erklärt Versorgung und Ausbau.

## Beobachtung zurückmelden

- [ ] Start, bezahlte Käufe, Zuzug, Produktion und tatsächliche Ankünfte geprüft.
- [ ] Hauswechsel, Inspector, 1×/2×/4×, Auswahl/Überdeckung, Help/Resize geprüft.
- [ ] F5/F9 und manueller Save nach vollständigem Neustart geprüft.
- [ ] Eigener Recovery-Checkpoint geöffnet; Quelle erhalten.

Bei einem Fehler: Karte, Difficulty, Tick, gewähltes Gebäude/Tool und genaue
Aktion nennen; bei Grafikproblemen Screenshot und Zoom ergänzen. Einen
Spielstand nur ausdrücklich zum Untersuchen übergeben. Die Checkboxen bleiben
offen, bis die jeweilige Beobachtung tatsächlich erfolgt ist.

## Was dieser Stand noch nicht verspricht

Die 15 belegten House-Bilder werden nur für passende tatsächliche Typen 3–17
und Körpergrößen verwendet. Mehrere Nicht-Haus-Objekte und Walker behalten
Fallbacks; ein leerer 2×2-Eliteplot bekommt kein gestrecktes4×4-Originalbild.
Rezepte, Ernte, Zuzug/Worker und Schule→Haus-Besuche enthalten authored Regeln.
Eine originale Veranstaltungs-/Performer-Kette und Originalkrankheit fehlen.
Alle 15 Modellzeilen sind integriert/testbar; das beweist keinen regulär
bezahlten Durchlauf bis zur höchsten Elite-Stufe.

Der unveränderte Vier-Haus-Starter ohne Ausbau endet im gemessenen 100k-Lauf
bei **−22128 Funds und 116 verpassten Food-Bedarfen**. Das ist ein Ergebnis
dieses festen Plans, keine allgemeine Aussage zur Rentabilität aller Städte.
Auch pausierte Gebäude kosten Unterhalt. Vier Häuser erfüllen das Wohnziel
noch nicht. Häuser/Produktionsgebäude sind hier noch nicht abreißbar;
der erlaubte Abriss betrifft nur neutrale Well72-Einflussquellen.

Die [Endvergleichsmatrix](final-comparison.md),
[Longplaymessungen](longplay-validation.md) und
[Known Issues](../../KNOWN_ISSUES.md) trennen Belege und Lücken.
Die Übergabe ist keine vollständige Originalspiel- oder Grafikfreigabe.
Native menschliche Abnahme bleibt offen; für diesen Abschluss werden
**keine Tags oder Releases** angelegt.
