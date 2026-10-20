# OE31: strikte Original-Saves und ausdrücklich gewählte Recovery

Vor Implementierung am sauberen OE30-Commit324fc8fd erklärt: Aktive
Rule10/Schema26/Policy3/Entwicklungsrevision27 bleiben erhalten. Keine neue
World-/Wirtschafts-/Geometrie-/Saveauthority, keine Migration und kein
Upgrade-copy. Regeln1–9 und Schemas1–25 behalten ihre bisherigen Bytes.

## Enger Recoveryzugang

RecoveryStore und AutosaveController erhalten als letzten optionalen Parameter
`RecoveryAccess { LegacyOnly, OriginalProduction }`. Default bleibt LegacyOnly;
bisherige Original-Recovery-Verweigerungen bleiben gültig. Der ausdrückliche
Zugang erlaubt ausschließlich den aktiven Rule10-Stand zusätzlich zu den alten
Profilen. Frozen7–9, alte Entwicklungsrevisionen und generisches restore_save
bleiben gesperrt. Manuelle Original-Saves und Recovery verwenden den vorhandenen
modell-/map-/mask-/policygebundenen Original-Restore; Originalpopulation stammt
aus OriginalHouseState einschließlich Elite, nicht aus Legacygebäudefeldern.

Neue Originalhistories dürfen nur identische Map-/Modell-/Versionsidentität
fortsetzen. Ein Original-Recoveryparent muss ein tatsächlich vorhandener,
loadable Checkpoint sein; der Branchstart muss dessen vollständige World- und
Sourceidentität erhalten. Legacy-/Originalwechsel beim Append und ungültige
oder erschöpfte Originalsequenzen werden vor Publikation verweigert. Bestehende
atomare Dateiziele, Budget, geschützter Startpunkt, Rotation und Löschung bleiben
erhalten. Ungültige Einzelpunkte werden isoliert, gültige Geschwister bleiben
loadable und dürfen neue Branches begründen. Ein defekter Start wird keine
Identitätsreferenz. Das Sequenzende bleibt lesbar; erschöpftes Append wird vor
Schreiben verweigert. Metadaten werden bis Tiefe32 vor DOM-Verschachtelung
begrenzt; flache alte Erweiterungen behalten ihr Verhalten. Alle Fehler müssen
frühere Save-/Recoverybytes unverändert lassen.

## Normale Anwendung

Nur der ausdrücklich gewählte Originalmodus beziehungsweise ein bewusst
geöffneter aktiver Original-Save erhält den neuen Zugang. Er folgt der vorhandenen
Autosavepräferenz: Startpunkt und höchstens alle1200 simulierten Ticks ein
Checkpoint am vorhandenen sicheren Punkt; Pause, Modalzustand und Gesten dürfen
keine zusätzlichen Schreibschleifen erzeugen. Recovery wird im bestehenden
Menü gewählt und als neue Historie/neues manuelles Dateiziel geöffnet. F5/F9
bleiben manuell; erfolgreiches Original-F9 beginnt eine neue Recoveryhistorie,
fehlgeschlagenes Laden erhält die bestehende Stadt und Historie. Ein neuer
viewlokaler Erfolgszähler ist keine gespeicherte Authority. Auch ein über den
Dateidialog geöffneter eigener Recoverypunkt erhält Parent und neues manuelles
Dateiziel; unzugeordnete eigene Recoverydateien werden verweigert. Legacy-F9-/Default-
Verhalten bleibt erhalten. Originalstatus und Autosavewahl werden ehrlich gezeigt.
Scheitert schon das Erfassen eines Original-Saves, etwa bei einer während des
Spiels entfernten Modellquelle, bleibt die aktive Historie erhalten. Der Fehler
wird angezeigt und der nächste periodische Versuch erst nach1200t zugelassen;
kein Frame-Repeat. Scheitert der anfängliche Recovery-Capture nach ansonsten
erfolgreichem Laden oder bestätigtem Stadtwechsel, bleibt die gewählte gültige
Stadt spielbar, ohne eine angeblich angelegte Historie zu melden. Nach Rückkehr
der Quelle darf der Spieler Autosave regulär erneut einschalten. OFF erfasst
keinen unnötigen Original-Save. Legacy-Captureverhalten bleibt unverändert.

## Abnahme

Vorhandene Save-/Recovery-/Atomic-/Menü-/View-Suites und neue positive/negative
Tests laufen in Debug, Release und ASan+UBSan. Der neue Katalog umfasst
fehlende/zusätzliche Felder, Typ-/ID-/Count-/Stock-/Model-/Geometry-/Protection-
und Route-/Cargo-Widersprüche; Parserfixes erfolgen nur bei belegten Lücken und
bleiben auf den aktiven Vertrag begrenzt. Positive Checkpoints entstehen nur aus
bezahlten Commands und echten Ticks: Walker mitten auf einer Kante, Reservierung,
Konsum-/Devolutionsgrenzen, erlaubter Besitzerabriss und echte Schuldensituation.
Mehrere neue Prozesse müssen dieselben vollständigen Dokumente und Weiterläufe
erzeugen; fehlende Versorgung wird nicht künstlich ergänzt.

Historische repräsentative Savebytes und vollständige bisherige Policyverträge
werden erneut verglichen. Tests schreiben ausschließlich in frische private
Verzeichnisse unter `.local/` oder eigene temporäre Fixtures, niemals alte
Spielstände oder Nutzer-PrefPaths. Originaldaten und OE30-Belege bleiben
unverändert. Gate F/100k-/Gesamtparitätsabschluss gehört weiterhin OE33; OE32
bleibt unbegonnen. Nur ein lokaler OE31-Commit, kein Push/Merge/Tag/Release.
