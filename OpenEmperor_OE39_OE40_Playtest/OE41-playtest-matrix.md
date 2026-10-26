# OE-41 – Verbindliche Spieltest-Matrix (B01–B20)

> `PASS` nur bei tatsächlich ausgeführter und beobachteter Bedienung/Simulation. `FAIL` bei reproduzierbarem Verhalten entgegen bestehendem Vertrag. `BLOCKED` bei fehlender notwendiger Voraussetzung. `UNSUPPORTED` bei ausdrücklich nicht vorhandenem Spielsystem. `NOT_RUN` wenn schlicht nicht ausgeführt. Pro Profil und gespeicherter Rule/Schema versionieren; keine historische Generation in eine andere verwandeln.

**Ergebnistemplate für jeden Test:** `ID | Profile/Rule/Schema | Karte/Data-Hash | App/Renderer | Tick | Gebäude-/Courier-ID | Funds | Schritte | Erwartung | tatsächliche Beobachtung | Screenshot/Log/Save unter .local | Status`.

| ID | Spieltest | Mindestbeleg | Negative Gegenprobe |
|---|---|---|---|
| B01 | Originaldaten auswählen/Start | echter App-Start mit bewusst ausgewähltem read-only Root; keine Datenänderung | falscher Pfad, fehlende notwendige Model-/SG3-Datei klar verweigert |
| B02 | Menü/Profil/Regeln | Standardprofil sowie Rule10, Rule11, Rule12 und erfolgreiche Nachfolger sichtbar und korrekt ausgewählt | zurück/cancel ändert nicht unbemerkt gespeicherte Regel |
| B03 | Interaktive Navigation | Pfeile, Rad, Z 1×/2×/4×, Auswahl; Window Resize, Focus/Blur, Maus-Drag | Outside-Window-/Modal-Geste löst keinen Gebäudekauf aus |
| B04 | Road und Gebäude bauen | reale Kosten, vollständige Footprints, Eingangsroad, IDs, HUD, Aufbau und Refusal | Terrain/Okkupation/Insufficient Funds hinterlassen keine Teilmutation |
| B05 | Clay→Pottery→Warehouse→Market→House | zwei echte Couriers/Handoffs, Nachfrage, House-Bestand/Verbrauch, Funds | isoliertes Gebäude produziert/versorgt nicht kostenlos |
| B06 | Food/Hemp | real gekaufte Farm+Field, Worker, Crop, Quality, physische House-Ankunft | ungebundenes Field und unzulässiges viertes Food-Farm-Limit verweigern gemäß Rule |
| B07 | Marktlogistik | Cargo/Reservation/End-to-End-Lieferungen; Common10-Akzeptanz nach OE39 | kein doppelter Warencredit, kein Starvation-Falsch-PASS |
| B08 | Arbeit/Operation/Priority | wirkliche Besetzung und Unterhalt vor/nach Pause, Operation/Worker-Priorität | Pause stellt bereits bezahltes Gebäude nicht gratis |
| B09 | Water/Medical/Religion | tatsächliche Service-Walker-Ankunft und Timer; Water ohne bloße Radiusversprechen | pausierter Anbieter erhält keine neuen Dispatches |
| B10 | Culture | echte Music/Acrobat/Drama-Schulbesuche und Timeouts | Original-Venue-/Performerketten nur `UNSUPPORTED`, kein falscher Original-PASS |
| B11 | Common Housing | echte 9→10-Entwicklung, Goods und Needs, 20.400 Ticks Retention (wenn OE39 PASS) | unversorgte Houses müssen begründet stagnieren oder devolvieren |
| B12 | Elite Housing | echtes 2×2 Plot→4×4, erste reguläre 11→13-Admission, aktuelle/next requirements | belegtes 79/80-Szenario unter Rule12, keine erfundene 17 |
| B13 | Fire & Watch | Rule11/12, ggf. neue Rule; echte Ignition/Inspector/Protection | Rule10 FireWatch weiterhin verweigert; pausing stops new dispatch |
| B14 | Road Delete/Repair | tatsächlicher bewegter, beladener Courier; Safe Edge, Wait, Repair, Return | geladene Fracht kann nicht teleportiert oder dupliziert werden |
| B15 | Bridge/Ramp | isolierte unterstützte City-v16 Rule5/6, Karte/Höhenkante, echter Weg und F5/F9 | verbotenes Höhen-/Ufer-/Footprint-Kommando atomisch abgewiesen |
| B16 | Map & Renderer | echte Xia + 3 weitere passende Karten, Originalroad, Höhen/Scenery/Fallback, Auswahl | bekannte Occupancy/Unknown-Quellen werden nicht ge-whitelistet |
| B17 | Inspector/Overlays/Help | alle derzeit exponierten Tabs/Hotkeys, tatsächliche Tool-/Building-Auswahl | reine Hover-/Frame-Aktion verändert World nicht |
| B18 | Persistenz/Recovery | F5, Quit, neuer Prozess, F9, Restore, Autosave, anderer Spielstand, laufender Courier | korrupter Save/Kreuzversion bleibt unverändert und verweigert |
| B19 | Longplay/Stress | Rule12-Stadt; 20.400/100.000 Ticks soweit unter Proof-Caps machbar, Finanzen/Population/Goods/Frames | erkannte Save-History-Grenze muss kontrolliert stoppen, keine Tests durch Datenlöschung |
| B20 | App-Zyklen | zwei Menürunden mit verschiedenen Maps/Profilen, kompletter Restart, isoliertes App-Root | keine Übernahme alter Rule/Source-/Camera-/Input-Hover-Autorität |

## Schichten zur Ergebnisinterpretation

| Schicht | Was zählt als `PASS`? | Was reicht ausdrücklich nicht? |
|---|---|---|
| Reiner Kern/CTests | Tatsächliche CTest-Binaries erfolgreich auf aktuellem Commit | bloße historische Zahl oder „Build succeeded“ |
| Headless produktiver World | echte Commands, Ticks, Source-Inputs, vollständige Saved World | handgesetzte Bestände, abstraktes Unit-Modell |
| Scripted SDL & Software | reale `SandboxView`/Menu-Events und geöffnete Bilder | Screenshot-Datei ohne Bildprüfung |
| Native GUI | tatsächlich gestartete sichtbare Mac-App und echte Mouse-/Keyboard-Aktionen | SDL dummy/software oder fernes REST-Testzeug |
| Human | personengeführte Anweisungen mit expliziter Rückmeldung | Codex behaupte „manuell überprüft“ |
| Originalparität | unabhängige Evidenz für genau behauptete Originalfunktion | Sprite/Name/Model-Feld oder analoge authored Logik |

## Einbettung in die reale Karte

- `Cities/Xia.map`: wirtschaftliche Testbasis, nur bei gültiger MapPermission und echter Modellquelle.
- `Handan`, `Badaling`, `Kaifeng`: unterschiedliche gespeicherte Straßen-/Gate-/Terrain-Konstellationen, soweit Daten vorhanden und gesetzlich gelesen.
- 167-Kartencorpus: separate technische Kartenadmission; 165 `prepared` ist **kein** Beweis 165 spielbarer Städte.
- Brücken/Rampen: explizites historisches Profil verwenden; Originalsprite 233–244 bleibt isolierter Prototyp, sofern OE39/40 daran nichts unabhängig geändert haben.

## Minimal zulässiges QA-Handoff

Für jedes ausgeführte Szenario mindestens eine reproduzierbare Schrittfolge, tatsächlicher Tick, Profil/Rule, Map/Quelle, beobachtete World-/UI-Wirkung sowie die genaue Belegdatei. Zu jedem `FAIL` Repro mit erwarteter/aktueller Wirkung, Severity P0–P3 und Verdacht vs. bestätigt. Fehlende native Bedienung / menschliche Begutachtung als offene Spalten belassen.
