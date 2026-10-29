# OE-41 – Menschlicher Playtest auf einem echten Mac

**Wichtig:** Dies ist eine **offene** Nutzercheckliste. Weder Codex noch ein Software-Screenshot darf sie vorab auf `PASS` setzen. Benutze nur eine selbst bereitgestellte legal installierte Emperor-Datenquelle und ein isoliertes Test-App-Root.

## Vor dem Start

```sh
./build/openemperor --data /Pfad/zur/eigenen/Emperor-Installation --app-root "$PWD/.local/oe41/human-app-root"
```

Trage ein: Git-Commit `________`, macOS `________`, Mac/CPU `________`, Display/Skalierung `________`, SDL/Renderer `________`, Datenversion `________`, Karte `________`, Regel/Schema `________`, Uhrzeit/Testdauer `________`.

## Tatsächlich spielen – nichts vorab abhaken

- [ ] Start aus dem Finder bzw. normaler sichtbarer Desktop-App funktioniert; Hauptmenü ohne Fehlermeldung.
- [ ] Auswahl des eigenen Spielordners, Karte, Difficulty, Originalwirtschaft-Modus; Auswahl bleibt sichtbar.
- [ ] Neue leere Stadt starten, Kamera/Zoom/Z-Pfeile/Mausrad ausprobieren; Gelände verständlich.
- [ ] Spiel anhalten, 1 Tick weiter, 1×/2×/4×, weiterlaufen lassen; UI zeigt korrekten Zustand.
- [ ] Road mit Einzelklick und Drag bauen, Fehlerfall auf gesperrtem Gelände ausprobieren; keine Phantomstraßen.
- [ ] Haus bauen, Inspector State/Needs/Goods/Visits öffnen, verfügbare Tooltip- und Hotkey-Hilfe prüfen.
- [ ] Water/Ancestor/Medical/Culture bei ausreichenden Funds bezahlen, Operation pausieren/fortsetzen; echte Walker beobachten.
- [ ] Food-Farm und ihr Feld bezahlen, Aufbau und Ernte beobachten; Market und House tatsächlich mit Food versorgen.
- [ ] Hemp und Ceramics über korrekte Warehouse-/Market-Kette liefern; Bestände und tatsächliche Ankünfte vergleichen.
- [ ] Bezahlten FireWatch betreiben, Inspector auf einem Weg erkennen, echten Schutzstatus sehen.
- [ ] Workforce/Staffing und Priority ändern; Pause stoppt neue Operation, Unterhalt bleibt nach Regel nachvollziehbar.
- [ ] Mindestens ein House-Up-/Downgrade real beobachten und Ursachen im Inspector verstehen.
- [ ] Verfügbaren Common10-Teststand starten, tatsächlich Ware/Delivery beobachten; wenn OE39 PASS, Typ10 im definierten Fenster stabil.
- [ ] Separaten Elite-Plot korrekt legal kaufen; vorhandene Grenzen des höheren Aufstiegs und Source-DES anzeigen.
- [ ] Aktive, beladene Fahrt: Roadsegment künftig blockieren/entfernen, Stop/Reroute/Return beobachten, Road reparieren.
- [ ] F5 speichern; Tick, Funds, Pop, Waren und laufende Walker notieren.
- [ ] App vollständig schließen, wieder öffnen und F9/Load Save: selber Stand, pausiert, danach normal spielbar.
- [ ] Autosave/Recovery aktivieren, Checkpoint sehen, wiederherstellen; bestehende manuelle Saves erhalten.
- [ ] Andere geeignete Karte testen, inklusive normaler Originalstraßen und blockierter Spezialstruktur.
- [ ] Unterstützte Bridge-/Ramp-Profile separat öffnen, echte Fußgängerrouten und Z-Ordering prüfen; Quellrampen-Prototyp gesondert benennen.
- [ ] 1×/2×/4× auf Gebäude, Walkerfüße, Schatten, Brücken, Rampen, Fenstergrenzen: keine falschen Auswahltreffer.
- [ ] Fenstergröße ändern, macOS-App kurz in Hintergrund und wieder nach vorne, Esc/Help/Modal öffnen, danach weiter bauen.
- [ ] Öffentliche `Known Issues` durchgehen: Dokumentierte Fallbacks/Unsupported mit dem tatsächlich beobachteten Verhalten vergleichen.

## Fehler notieren

Für jeden Befund: `P0/P1/P2/P3`, Karte, Profil/Rule/Schema, Tick, exakte Bau-/Walker-ID, Funds, Tastenkombination/Mausschritt, erwartet vs. passiert, Screenshot/kurzer Bildschirmmitschnitt (private `.local/oe41/`), lässt sich wiederholen `ja/nein`.

**Gesamturteil des Menschen (erst nach dem Test ausfüllen):**

- Funktionsverständlichkeit: `PASS / PARTIAL / FAIL / NOT_RUN`
- Steuerung/Kamera/Bedienung: `PASS / PARTIAL / FAIL / NOT_RUN`
- Wirtschaft/Waren/Services: `PASS / PARTIAL / FAIL / NOT_RUN`
- Grafik/Lesbarkeit: `PASS / PARTIAL / FAIL / NOT_RUN`
- Speichern/Fortsetzen: `PASS / PARTIAL / FAIL / NOT_RUN`
- Bereitschaft für weitere Tester: `JA / MIT BLOCKERN / NEIN`

**Wichtigste drei Probleme:**

1. `_________________`
2. `_________________`
3. `_________________`
