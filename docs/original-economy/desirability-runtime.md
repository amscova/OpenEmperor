# OE-12: bezahlte Well72-Einflussquellen im Entwicklermodus

Der ausdrücklich beauftragte [Quellenvertrag V1](source-contract-v1-proposal.md)
verbindet City-v16 **Rule8 / Schema24 / policy3** mit dem bestehenden geprüften
Desirability-Quadratkern. Genau ein neuer authored Quellentyp bindet einen
bezahlten2×2-Container an die verifizierte Modellzeile Well72. Er ist kein
vollständig rekonstruiertes Original-Well und liefert kein Wasser, Waren,
Worker, Einkommen oder Unterhalt. Originale Modelldaten bleiben read-only;
vorhandene Well-Darstellung ist ausschließlich Präsentation.

## Ausprobieren

Aus dem Repositoryroot mit der vorhandenen Originalinstallation und SDL3:

```sh
sh tools/play_original_influence.sh
```

Das baut und startet ausschließlich den eigenen Entwickler-Launcher auf
`Cities/Xia.map`. Die Standardanwendung und ihr Menü bleiben unverändert.
Die Session startet pausiert mit2000 Funds; der Savepfad liegt separat unter
`.local/original-economy/player/rule8.json`. Es wird kein vorhandener Spielstand
gesucht oder automatisch geladen. Andere ausdrücklich gewählte Daten-/Map-/
Savepfade lassen sich als erste drei Argumente übergeben; ein viertes
`--load` lädt ausschließlich den angegebenen passenden Rule8-Save.

- `7`: bezahltes Vacant-Haus, Kosten80, Limit20; keine Bewohnerentwicklung.
- `I`: Well72-Einflussquelle, Kosten60, Limit4, exakt2×2. Alle vier Zellen müssen
  die normalen Bau-/Belegungs-/Originalschutzregeln erfüllen.
- `5`: Auswählen. Das Haus zeigt seinen tatsächlichen Cachewert und die
  Quellenbeiträge an der maßgeblichen Hauszelle; Quelle zeigt ihren begrenzten
  Zweck. Difficulty und Modell-SHA sind im Inspector nachvollziehbar.
- Quelle auswählen, `Demolish`, bestätigen: ausschließlich diese Quelle
  entfernen, ohne Refund. Ein Wiederkauf erhält eine neue BuildingId.
- `F5` / `F9`: tatsächliches Speichern/Laden mit Modell-/Map-/Policybindung;
  F9 pausiert. Fenster schließen beendet den Launcher.

Es gibt keine aktive Bevölkerungs-/Versorgungsaufgabe, Gratisquelle oder
Einnahme durch Warten. Straßen, Brücken und Rampen bleiben eigenständig
bezahlte Infrastruktur nach ihren vorhandenen Verträgen.

## Authority und Begrenzungen

Lebende Quellen referenzieren vorhandene BuildingIds. Entfernte Quellen
bleiben als typisierte Historie mit Bau-/Entfernungstick erhalten, ohne Fläche
oder Einfluss. Fixes Budget2000 und Fixpreis60 begrenzen die Sourcekäufe über
die gesamte Session auf33. Die gemeinsame ID-Menge aus Häusern, lebenden und
entfernten Quellen ist lückenlos; kein Refund oder Wiederverwenden alter IDs.
Das frühere DemolitionHistory bleibt0. Schemas1–23 kennen keine neuen
Quellentypen oder Payloads; Rule7/Schema23 bleiben sourcefrei bei1300 Funds.

Jeder verändernde Rule8-Spielerbefehl wird vollständig in einem privaten
Kandidaten vorbereitet und erst nach Erfolg veröffentlicht.
Preview/Abfragen kopieren keine World.
Die Einflussmap verarbeitet höchstens vier Quellen nach aufsteigender
BuildingId und verwendet unveränderliche vorbereitete Well72-Felder.
Prefixmaps erklären die Sättigung vor/nach jeder Quelle an derselben
Maximalzelle. Bei Gleichstand gilt die erste Zelle in y/x-Reihenfolge.

Cacheaufbau erfolgt bei Create/Restore und erfolgreichem Quellenbau/-abriss.
Hausbau, Straßenbau, Tick, Kamera, Hover und Frame verursachen keinen Rebuild.
Hauswerte lesen genau vier vorbereitete Zellen; der Inspector liest bounded
Provenance. Modell-/Difficultywechsel während der Session gibt es nicht.
Bonus0 und WallBlocking::None bleiben explizit; ursprüngliche Wallabschirmung,
Managerreihenfolge, Status-/Parent-Lifecycle und Markt-Rechtecke werden damit
nicht als restauriert behauptet. Die sieben alten Analogien bleiben
`Inferred`/`Unknown`.

## Reproduzierbare technische Abnahme

Nach Konfiguration eines vollständigen Testbuilds:

```sh
ctest --test-dir .local/reswall/release-build --output-on-failure -R '^original-influence-'
cmake --build .local/reswall/release-build --parallel 2 --target openemperor-original-influence-fixture
python3 tools/test_original_influence.py .local/gog-extracted/app --build-dir .local/reswall/release-build
```

Der Runner liest die echte Original-Modelldatei, prüft ihre gepinnte SHA und
kopiert sie bytegleich ausschließlich in einen privaten Testroot. Die24×20-
Geographie dort ist ausdrücklich authored, keine echte Originalmap. Reguläre
Befehle kaufen40 Straßen,20 Häuser, Quellen A/B, entfernen A und kaufen eine
neue Quelle am Rand:1800 Ausgaben/200 Funds nach Abriss,1860/140 nach Wiederkauf.
Hauswerte müssen sich wirklich ändern. Zwei neue OS-Prozesse prüfen jeweils
20.000 weitere Scheduler-Ticks gegen vollständige SaveDocument-/Byte- und
Provenancegleichheit. Originalinventar und Eingabesaves bleiben unverändert.

Unverändertes City-v13 verweigert Wells und kann diese Szene nicht identisch
aus seinem Startbudget kaufen. Der freigegebene Vergleich wendet daher seine
unveränderten reinen Bewertungsfunktionen auf dieselbe bezahlte neue Szene an,
mit ausdrücklich authored Well72→Well-Zuordnung. Deren Referenzwert ist hier
an allen20 Häusern0. Echte alte City-v13-/16-Regressionsverträge werden separat
beibehalten; es gibt keine fingierte City-v13-World.

Der SDL-Test prüft Mausbau, neutrale Wasseranzeige, Inspector, F5/F9 und
counterfreie Idleframes. Das ist scripted Software-SDL mit authored Grafik;
eine geöffnete Aufnahme ist visuelle Prüfung dieses technischen Zustands.
Native menschliche Gameplayakzeptanz bleibt getrennt. Tatsächlich ausgeführte
Builds, vollständige Gate-Suites und Legacy-Corpusresultate stehen im
[Fortschrittsregister](../codex-original-economy/PROGRESS.md).
