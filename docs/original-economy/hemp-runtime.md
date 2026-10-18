# Bezahlte Hemp-Kette: Vertrag für Entwicklungsrevision16

OE-16 setzt den [Gesamtauftrag](completion-contract.md) im expliziten
Rule10-/Schema26-Entwicklerstand fort. Revision16 verweigert frühere
Entwicklungsrevisionen; Regeln1–9 und Schemas1–25 bleiben unverändert.
Die bisherige Ceramics-Kette läuft weiter.

## Belegte Quelle und ergänzende Spielregeln

Die lokale Originalmodelldatei benennt Modell192 als `Hemp Farm` und194 als
`Hemp Field`. Normal vorbereitet sind Cost40/4 und Employees18/0. Diese
kleinen Quellenfakten werden read-only geladen und schwierigkeitsabhängig
mit der vorhandenen geprüften Arithmetik vorbereitet. EXE-/Spritebelege
identifizieren die Farm als2×2 und das Feld als1×1. Die Grafik selbst wird
in der Präsentationsaufgabe aus den lokalen Originalarchiven geladen.

Die Umrechnung der Modell-Cost in Funds, der konkrete Feldanschluss und die
Produktionszeit sind ausdrücklich **authored Integration**. Die originale
Cost-Hook-/Rezept-/Zeitsemantik folgt nicht allein aus diesen Modellzeilen.
Der neue aktive Vorbereitungspfad verlangt beide benannten Zeilen; der
bisherige Sieben-Zeilen-Pfad bleibt für die eingefrorenen Regeln unverändert.

- Farm und Feld kosten separat die positiv geprüften vorbereiteten Cost-Werte.
  Ein Farmkauf erzeugt keinen kostenlosen Feldbereich und keine Ware.
- Höchstens zwei Farms mit je einem separat bezahlten Feld. Das Feld wird
  ausdrücklich einer existierenden Farm-ID zugeordnet und liegt direkt an
  ihrer2×2-Fläche. Originalterrain, fremde Owner und Infrastruktur behalten
  ihre vorhandenen Schutz-/Geometrieprüfungen.
- Eine Farm benötigt die vorbereiteten Employees, ist regulär ein-/ausschaltbar
  und produziert nur vollständig besetzt, mit bezahltem Feld und ohne Feuer.
  Nach80 produktiven Ticks entsteht eine Hemp-Einheit; ihr physischer Puffer
  hält höchstens8. Volle Puffer stoppen weitere Erzeugung.
- Ein eigener typisierter Farm-Courier nutzt denselben Navigator und dieselbe
  Bewegung bis zum Warehouse. Bestehende Warehouse-/Market-Actorrollen
  verteilen beide Waren in deterministischer Rotation. Keine zusätzlichen
  Food-Placeholderactoren, keine Teleportlieferung.
- Separate ausdrücklich authored Bins: Hemp32 je Warehouse,16 je Market,
  8 je lieferberechtigtem Haus. Ceramics-Bins behalten ihre bisherigen Werte.
  Eingehende Reservierungen und ausgehender Rückkehrplatz gelten je Ware.
  Jeder Courier hat nur einen tatsächlichen Cargo-Typ/Mengenbestand.

## Authority, Restore und Abnahme

Neue Objekt-, Command- und Farm-Courier-IDs werden append-only vergeben und
nur unter Rule10 freigeschaltet. Genau eine gespeicherte Farmzeile bindet
Farm-ID, optional gekaufte Feld-ID und tatsächliche Erntehistorie. Progress
liegt im existierenden Gebäudezustand; Food-/Potteryzähler werden nicht
umgedeutet. Die Hemp-Ernte entspricht exakt `GoodsTotals.Hemp.produced`.
Farmstock, Warehouse, Market, alleinige HouseState.hemp und tatsächliches
Cargo ergeben eine gemeinsame Conservation-Bilanz.

Maximal acht typisierte Actoren behalten je ihre letzte Transferzeile als
Sequenz-Highwater. Die Ware dieser Zeile dient zugleich als begrenzter
Rotationscursor. Ownerpause, begonnene Kanten, Umleitung, Rückkehrplatz,
stale/doppelte Ankunft und sichere Abrissverweigerung folgen OE-15.
Restore validiert die vollständigen gekauften Farm-/Feld-IDs und Kosten,
Modellfähigkeit, Ernte-/Progressgrenzen, Trip-/Owner-/Warenbindung und Masse.

Positive Belege entstehen aus bezahlten Commands und laufender Produktion,
einschließlich eines Hauses mit getrennt gelieferten Hemp und Ceramics.
Pause, Unterbesetzung, fehlendes Feld, fehlende Straße, volle Puffer,
Unterbrechung, Rückgabe, F5/F9 und separate Prozessreplays werden geprüft.
Keine Snapshot-/Stock-/Workerinjektion zur Herstellung des Kreislaufs.
Testresultate und Grenzen stehen im [Fortschritt](../codex-original-economy/PROGRESS.md).
