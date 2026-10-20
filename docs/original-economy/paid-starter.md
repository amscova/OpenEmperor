# Originalwirtschaft spielen

Die normale Anwendung starten, die eigenen Originaldaten auswählen und im
Hauptmenü **Original economy** oder **O** wählen. **Cities/Xia.map**,
Difficulty **Normal** und **Paid four-house starter** bieten den geprüften Einstieg.
**Start** öffnet die Stadt pausiert. **Space** lässt die Simulation laufen;
**1x/2x/4x** unten ändern das Spieltempo. **Z** ändert den Kamera-Zoom.

Der Starter bezahlt Straßen, vier Häuser, Wasser, Ancestor, Market,
FoodFarm mit Millet, HempFarm mit Feld, Clay/Pottery/Warehouse, Herbalist,
Acupuncture und Music aus den vorhandenen12000 Funds. Mit der geprüften
Modelldatei kosten die Käufe1404; es bleiben10596 Funds. Einwohner und Vorräte
beginnen bei0. Zuzug, Arbeit, Produktion, Kurierankünfte und Verbrauch müssen
erst tatsächlich stattfinden. Häuser entwickeln sich daher nicht sofort.
Zwei Ancestor- und zwei Wasserquellen sind reguläre bezahlte Gebäude, keine
zusätzlich gewährte Versorgung. Quellen außerhalb ihrer Reichweite helfen nicht.

**5 / Select** wählt Gebäude; **Help** erklärt die Bauwerkzeuge. Der
Hausinspector zeigt mit **PageUp / PageDown** State, Needs, Goods und Visits. Fehlende
Anforderungen und wirkliche Ankünfte sind dort getrennt sichtbar. Zusätzliche
Gebäude werden mit den normalen Werkzeugen gekauft. Ein fehlendes Originalbild
bleibt als Fallback gekennzeichnet; Hausbilder allein bedeuten keine vollständige
Originalgrafikparität. Music benutzt den bereits bestehenden experimentellen
Schule→Haus-Besuch, keine rekonstruierte originale Veranstaltungs-/Performer-Kette.

**F5** speichert außerhalb der Originaldaten. **F9** lädt diesen Stand pausiert.
Nach einem Neustart öffnet **Load save** den ausdrücklich ausgewählten manuellen
Originalstand. Difficulty und Modellquelle müssen zu ihm passen. Alte
Entwicklungsrevisionen werden verweigert. Für diesen Modus ist Recovery noch
nicht freigeschaltet; die bisherige Autosavepräferenz anderer Profile bleibt
erhalten. **Escape** führt ins Menü, **Resume** zurück zur laufenden Stadt.

Unterhalt fällt auch für pausierte Gebäude an. Vier Häuser erfüllen das
Wohnziel noch nicht; für das Ziel sind mindestens sechs erforderlich. Weitere
Häuser und passende Versorgung werden regulär bezahlt. Häuser und
Produktionsgebäude lassen sich in diesem Entwicklungsstand noch nicht abreißen;
der vorhandene Abriss ist auf neutrale Well72-Einflussquellen begrenzt.

Zum Prüfen: leere Bestände beiTick0 ansehen, laufen lassen, Kurierlieferungen
und House-Upgrades beobachten. Beide Ancestor-Gebäude auswählen und regulär
pausieren: Nach ausbleibender Versorgung und negativen Auswertungen entwickeln
sich Häuser zurück. Wiederaufnahme und verbundene Straßen ermöglichen neue
Besuche. F5 speichern, weiterlaufen lassen, F9 laden und den gespeicherten Tick
vergleichen; danach Anwendung neu starten und denselben Save öffnen.

Der Bauplan sucht nur eine vorhandene legale ebene24×13-Fläche. Auf ungeeigneten
Karten kann der Start mit genauer Diagnose scheitern; **Empty** oder eine andere
Karte wählen. Das ist ein OpenEmperor-Bauplan auf den Originalkarten und der
vorhandenen Rule10-Simulation, keine originale Startstadt oder Kampagne.
