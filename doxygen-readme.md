# Doxygen
Doxygen ist ein Tool, das automatisch Dokumentationen aus speziell formatierten Kommentaren im Code generiert

⚠️ **Achtung:** Bitte nur auf dem `gh-pages` Branch Doxygen-Kommandos ausführen! ⚠️

## Installation
1. Installiere auf mac:
`brew install doxygen`
## Doxyfile
Die Doxyfile ist die Konfigurationsdatei, die Doxygen verwendet, um die Dokumentation zu generieren. 
2. Generiere in deinem projekt directory:
`doxygen -g`
## Konfiguration
3. Öffne die Doxyfile mit einem Texteditor und passe die folgenden Parameter an:
```
    PROJECT_NAME: Der Name deines Projekts.
    OUTPUT_DIRECTORY: Das Verzeichnis, in dem die Dokumentation gespeichert wird.
    INPUT: Das Verzeichnis oder die Dateien, die Doxygen analysieren soll (z. B. src/ oder bmp180.h).
    EXTRACT_ALL: Setze dies auf YES, um alle Kommentare zu extrahieren.
    GENERATE_HTML: Setze dies auf YES, um HTML-Dokumentation zu erstellen.
    GENERATE_LATEX: Setze dies auf YES, um PDF-Dokumentation zu erstellen (optional).
```

## Kommentare

4. Füge Datei-Kommentare, Funktions-Kommentare oder param und return value params in deinen Code ein, zB:

```
/**
 * @file bmp180.h
 * @brief BMP180 Sensor Library
 * 
 * Diese Datei enthält die Schnittstellen für die BMP180-Sensorfunktionen.
 */
 ```

 Function Comments

```
/**
 * @brief Initialisiert den BMP180-Sensor.
 * 
 * Diese Funktion liest die Kalibrierungsdaten aus dem Sensor.
 */
void BMP180_Init(void);

/**
 * @brief Liest den Luftdruck aus dem BMP180-Sensor.
 * 
 * @return Luftdruck in Pascal.
 */
int32_t BMP180_ReadPressure(void);

/**
 * @brief Liest die Temperatur aus dem BMP180-Sensor.
 * 
 * @return Temperatur in Grad Celsius.
 */
float BMP180_ReadTemperature(void);
```

## Generiere Dokumentation
5. Erstelle die Dokumentation aus dem Doxyfile:

`doxygen Doxyfile`

Erstellt eine .html datei der Dokumentation, und optional latex, wenn konfiguriert

### Deployment: GitHub Pages

Um die generierte Doxygen-Dokumentation automatisch auf GitHub Pages bereitzustellen, arbeiten wir auf dem `gh-pages` branch.

1. Stelle sicher, dass die HTML-Dokumentation im gewünschten Ordner (z. B. `docs/html/` oder `html/`) generiert wurde.
2. Richte einen Branch namens `gh-pages` in deinem Repository ein (falls noch nicht vorhanden).
3. Committe und pushe den `gh-pages`-Branch zu GitHub:
    ```
    git checkout gh-pages
    git add .
    git commit -m "Deploy Doxygen documentation"
    git push origin gh-pages
    ```
4.  Aktiviere GitHub Pages in den Repository-Einstellungen und wähle als Quelle den `gh-pages`-Branch.

5. Nach dem Push ist die Dokumentation unter `https://<username>.github.io/<repository>/` erreichbar.