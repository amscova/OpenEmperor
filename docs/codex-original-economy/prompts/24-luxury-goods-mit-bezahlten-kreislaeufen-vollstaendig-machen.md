# OE-24 – Luxury-Goods mit bezahlten Kreisläufen vollständig machen

**Phase:** D – Housing  
**Abhängigkeiten:** OE-13, OE-15, OE-16, OE-17, OE-21  
**Geplante Änderungspfade (prüfe echte Dateinamen):** `src/simulation/World.{h,cpp}; src/simulation/original/; src/persistence/; tests/`  
**Vorgeschlagene Commit-Message:** `feat(original): [24] luxury-goods-mit-bezahlten-kreislaeufen-vollstaendig-machen`

## Auftrag an Codex

Arbeite am tatsächlichen HEAD von `oliverjessner/OpenEmperor`. Lies zuerst `docs/codex-original-economy/README.md`, `IMPLEMENTATION_ORDER.md`, `QUALITY_GATES.md`, `PROGRESS.md`, `DECISIONS.md`, `AGENTS.md` sowie alle für OE-24 relevanten vorhandenen Source-/Testdateien. Behalte die Code- und Test-Konventionen bei.

### Implementiere genau Folgendes

1. Implementiere Tea, Silk, Bronzeware und Lacquerware in getypter Produktion, Lagerung, Market-Transport und Haushaltsverbrauch.
2. Verwende verifizierte Modeltypen/Kosten für Erzeuger; wenn Originalproduktion ohne untersuchte Regel nicht sicher ist, eine offen benannte versionierte OpenEmperor-Rezeptannahme.
3. Kläre belegbar, ob Bronzeware/Lacquerware als alternative Hausbedarfe zählen; vermeide den Fork-Fehler, beide aus einem Feld unabhängig zu konsumieren.
4. Teste vier verschiedene Güter, Teilmengen, volle Reservierungen, max Warehouses, Abriss und Überlauf.
5. Kein Hochstufen-Haus darf ein ungedecktes Luxury-Good als erfüllt ansehen.

### Fertige Ergebnisse / Abnahme

- [ ] Vier Luxuswaren haben vollständige paid Source->Market->Home->Consume Bilanz
- [ ] Ein Haus kann mit realem Luxusgut fortschreiten; Negativfall ohne Bedarfserfüllung bleibt stehen

### Pflichtprüfungen

- [ ] Producer->Courier->Market->House->Consume pro Gut
- [ ] Conservation über 20k Ticks + Save + Demolition

## Stop-Bedingung und Fehlerbehandlung

Keine zusätzlichen gratis Startlager, keine optionalen unsaved Karten-/Resolver-Tables.

Liegt eine Abhängigkeit auf BLOCKED/FAIL oder kann ein verpflichtender Test nicht laufen, **stoppe diesen Prompt**, lege keinen „grünen“ Feature-Commit an und protokolliere den konkreten Blocker (inkl. reproduzierbarem Befehl). Vermeide Blind-Fixes, künstliche Authority und abgeschwächte Assertions.

## Gemeinsame technische Vertragsgrenzen

- Keine Windows-Workarounds, keine pauschalen Fork-Merges, keine fremden Änderungen ohne gezielten Vergleich.
- Legacy City-v16 Rule 4/5/6, darunter Bridge-/Ramp-Authority sowie alte Save-Schemas, unverändert; Rule7/Schema23 nur bei freier Versionsidentität und ausdrücklich opt-in.
- Originale `.sg3`, `.555`, Model-Dateien und EXE ausschließlich read-only aus der rechtmäßig vorhandenen Installation; keine Originalbilder oder umfangreichen Zahlenkopien ins Git.
- Kein zeitbasierter oder per-Frame-Zugriff auf Dateisystem/Decoder/World-Mutationen; kein zweiter Navigator.
- Teste notwendige Negativfälle. Grün bedeutet nur belegte technische Funktion, nicht pauschale Original-Parität.

## Commit / Report

1. Vor der Arbeit `git status --short` prüfen; vorhandene Fremdänderungen nicht überschreiben, stashen oder resetten.
2. Implementiere in kleinen zusammengehörigen Änderungen. Baue und führe die **für diesen Task relevanten** CTests aus, danach die im Phasen-Gate verlangten Suites.
3. Dokumentiere jeden tatsächlich ausgeführten Test samt Resultat und die Unterschiede gegenüber dem Fork in `PROGRESS.md`; notiere Annahmen in `DECISIONS.md`.
4. Wenn alle Abnahmekriterien erfüllt sind: nur Task-Dateien, Tests und Statusdokumente stagen und **einen** Commit mit obiger Message erzeugen. Der Nutzer hat lokale Commits für dieses Paket ausdrücklich autorisiert; historische `no commit`-Notizen in AGENTS.md gelten insoweit als überholt, alle übrigen Regeln bleiben bindend. Kein Push, kein Tag, kein Release.
5. Antworte mit Commit-SHA, Testprotokoll, belegtem Resultat und nächstem vorgesehenen Prompt. Bei FAIL/BLOCKED keine Freigabe des nächsten Prompts.
