# Luftdruck Sensor Projekt: GY-BMP180 mit STM32F4 Discovery Board
Projekt für LVA `Angewandte Mikrocontrollerprogrammierung`, SS26 Jahrgang AE27

## Aufgabenstellung

Die Aufgabenstellung für Gruppe 3 - Sensor für Luftdruck `GY-BMP180`: 

**Messen Sie Änderungen des Luftdrucks und zeigen Sie diese an.**

| Nr | Aufgabe | Erledigt |
|----|-----------------------------------------------------------------------------------------------------------------------------------|:---:|
| 1  | Analyse der Funktionen der jeweiligen Komponente (Datenblatt aus Web besorgen!) | [ x ] |
| 2  | Anschluss der jeweiligen Komponente an den STM32 über die Schnittstelle, die der jeweilige Sensor bereithält. | [ x ] |
| 3  | Inbetriebnahme und (nachweisliche!) Lösung der gestellten Aufgabe. | [ x ] |
| 4  | Geeignete Visualisierung der Ergebnisse unter Zuhilfenahme des E/A-Boards (7-Segmentanzeige, LED, usw.) oder eines 2004-LCD-Displays. | [ x ] |
| 5  | Erstellung einer Test-SW auf dem STM32, wobei die Bedienung der jeweiligen Komponente in wiederverwendbare Funktionen ausgelagert sein muss. | [ x ] |
| 6  | Dokumentation von Anschluss, Funktion, Parametern, Randbedingungen usw. sowie der SW. | [ x ] |

## Sensordaten: BMP180
Der Sensor GY-BMP180 ist in der Lage, Temperatur, Luftdruck und Luftfeuchte
zu messen. Er misst physikalische Änderungen mittels piezoelektrischem Effekt und liefert unkompensierte Rohwerte. Produktionsbedingt können diese leicht unterschiedliche Eigenschaften je Sensor haben. 

Siehe Datenblatt: https://cdn-shop.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

\htmlonly
<figure style="text-align:center;">
    <img src="general_datasheet.png"
         alt="Beschreibung und generelle Funktion"
         style="width:60%; border:1px solid black; padding:6px;" />
    <figcaption>
        <b>Abbildung:</b> Beschreibung und generelle Funktion des BMP180 (Seite 9)
    </figcaption>
</figure>
\endhtmlonly

\latexonly
\begin{figure}[h]
\centering
\fbox{
    \includegraphics[width=0.6\textwidth]{../docs/images/general_datasheet.png}
}
\caption{Beschreibung und generelle Funktion des BMP180 (Seite 9)}
\label{fig:general-datasheet}
\end{figure}
\endlatexonly

## I2C Kommunikation
Die I2C Addresse des BMP180 ist `0xEE` laut dem Datenblatt

### Verkabelung
Mit zusätzlichen 4,7kOhm Pull-up Widerständen bei SDA und SCL wird wie folgt verkabelt:

```

BMP180              STM32F4-DISCOVERY
--------------------------------------
GND ---------------> GND
VCC ---------------> VDD

SDA ---+-----------> PB7
       |
    [ 4.7k ]
       |
       ------------> VDD

SCL ---+-----------> PB6
       |
    [ 4.7k ]
       |
       ------------> VDD
       
```


## Cube MX Konfiguration
Projekt generiert in CubeMX: 
1. Bei Connectivity ist I2C zu aktivieren

## Verwendete HAL Funktionen

1. **HAL\_I2C\_Mem\_Read()** - Liest per I2C Daten aus dem Sensorregister.
2. **HAL\_I2C\_Mem\_Write()** - Schreibt per I2C Daten in eine gewünschte Speicheradresse.

## Bibliotheksfunktionen

Folgende Funktionen wurden in `bmp180.h` erstellt:

- `BMP180_Start`
- `BMP180_GetTemp`
- `BMP180_GetPress`
- `BMP180_GetAlt`

Im Datenblatt ist der Algorithmus für Druck und Temperatur Messung dargestellt, welcher dafür nachgebaut wurde:

\htmlonly

<figure style="text-align:center;"> <img src="../docs/images/algorithm_datasheet.png" alt="Algorithmus Druck und Temperatur" style="width:55%; border:1px solid black; padding:6px;" /> <figcaption> <b>Abbildung:</b> Algorithmus zur Berechnung von Druck und Temperatur (Seite 15) </figcaption></figure> 
\endhtmlonly
\latexonly
\begin{figure}[h]
\centering
\fbox{
\includegraphics[width=0.55\textwidth]{../docs/images/algorithm_datasheet.png}
}
\caption{Algorithmus zur Berechnung von Druck und Temperatur (Seite 15)}
\label{fig:algorithm}
\end{figure}
\endlatexonly

### Schritt 1 - Kallibrierungswerte einlesen aus dem EEPROM vom BMP180

Der Sensor hat einen intern beschriebenen Speicher, in dem er 11 individuelle Kallibrierungs-Koeffizienten mit jeweils 16 bit hinterlegt. Diese sind erforderlich, um die ausgelesenen Rohdaten in korrekte Temperatur- und Luftdruckwerte umzuwandeln.

Der Mikrokontroller muss daher bei jedem Start zunächst die im internen EEPROM gespeicherten 176 Bit (11 Kalibrierwerte, jeweils 16 Bit) auslesen, um die Formeln für Druck und Temperatur korrekt anwenden zu könen.

Die Werte heißen laut Datenblatt `AC1 bis AC6 und B1, B2, MB, MC, MD`, diese Werden als Kallibrierungsdaten einem Array `Callib_Data` initialisiert mit 0 und Start bei der BMP180 Speicher Adresse `0xAA` auf dem Mikrokontroller zwischengespeichert.

Mithilfe der HAL Funktion, um i2c memory zu lesen und zu schreiben werden die Kallibrierungswerte aus dem Sensorregister gelesen und in eine gewünschte Speicheradresse geschrieben.

Aus je 2 Byte im Array wird ein 16-Bit-Kallibrierungswert gebaut  
`AC1 = ((Callib_Data[0] << 8) | Callib_Data[1]);`

Das High-Byte wird dabei um 8 Bit nach links verschoben und mit `|` mit dem Low-Byte verknüpft.  
-> Beide Bytes (Index 0 und Index 1) ergeben also einen 16 Bit Wert.  
-> AC1 ist dann ein kalibrierter Sensorparameter.

Nach diesem Prinzip werden **alle 11 Kalibrierungsparameter** erstellt:

- `AC1, AC2, AC3 (signed short)`: Temperatur-/Druck-Kalibrierungskoeffizienten, die in linearen Termen der Druckkompensation auftreten
- `AC4 (unsigned short)`: Faktor in Divisionsschritten (dient zur Skalierung / Division)
- `AC5, AC6 (unsigned short)`: weitere Kalibrierungskoeffizienten, werden bei Temperaturberechnung als Multiplikatoren/Offsets verwendet (z. B. AC5 dient zur Skalierung von UT)
- `B1, B2 (signed short)`: weitere Druck-Korrekturkoeffizienten (nichtlinearer Teil)
- `MB, MC, MD (signed short)`: Temperaturkorrekturkonstanten, erscheinen in der Berechnung von X2 und B5

Der BMP180 speichert einige Kalibrierwerte als vorzeichenbehaftet (signed short) und andere als unsigned short. Physikalisch sind das keine direkten Messgrößen, sondern Parameter eines Herstellermodels, das die nichtlinearen Eigenschaften des individuellen Sensorelements korrigiert.

### Schritt 2 - Temperatur: Von Rohdaten zu echten Messwerten

Die Funktion GetUTemp() liest zunächst den unkompensierten Temperaturwert (UT) aus dem BMP180-Sensor aus.

Dazu wird das Steuerbyte `0x2E` in das Register `0xF4` geschrieben. Dadurch startet der Sensor die Temperaturmessung.

Anschließend wartet das Programm etwa 5 ms, damit die Messung abgeschlossen werden kann.
Nach der Wartezeit werden zwei Bytes aus dem Register 0xF6 gelesen. Diese Bytes ergeben zusammen den Rohwert der Temperaturmessung:

`UT = uncompensated temperature`

Der Wert UT ist noch nicht kalibriert und enthält daher noch keinen direkt verwendbaren Temperaturwert.
Der endgültige Temperaturwert wird mit der Funktion BMP180_GetTemp() berechnet. Dabei werden die im Sensor gespeicherten Kalibrierungsdaten verwendet.

Die Berechnung erfolgt nach den Formeln aus dem BMP180-Datenblatt:

```text
X1 = (UT - AC6) * AC5 / 2^15
X2 = (MC * 2^11) / (X1 + MD)
B5 = X1 + X2
T  = (B5 + 8) / 2^4
```

Die Variable T enthält die Temperatur in Schritten von 0.1 °C. Die tatsächliche Temperatur in Grad Celsius ergibt sich daher aus: 

`Temperatur [°C] = T / 10`

Die Funktion BMP180_GetTemp() gibt anschließend den berechneten Temperaturwert in Grad Celsius zurück.

### Schritt 3 - Luftdruck: Von Rohdaten zu echten Messwerten 

Die Funktion `Get_UPress(oss)` liest zunächst den unkompensierten Luftdruckwert (UP) aus dem BMP180-Sensor aus.
Dazu wird in das Register `0xF4` das Kommando `0x34 + (oss << 6)` geschrieben.

Der Parameter oss (Oversampling Setting, Wertebereich 0...3) bestimmt die Messauflösung. Eine höhere Auflösung verbessert die Genauigkeit der Messung, erhöht jedoch gleichzeitig die notwendige Wartezeit. Je nach oss beträgt diese etwa 5 ms bis 26 ms.

Nach Ablauf der Wartezeit werden drei Bytes aus dem Register `0xF6` gelesen:

```
MSB (Most Significant Byte)
LSB (Least Significant Byte)
XLSB (Extended Least Significant Byte)
```

Diese drei Bytes werden zu einem 24-Bit-Rohwert kombiniert. Anschließend wird der Wert abhängig vom verwendeten Oversampling-Setting bitweise verschoben (Bit-Shift), damit der Messwert korrekt skaliert wird.

Das Ergebnis ist der unkompensierte Luftdruckwert:

`UP = uncompensated pressure`

Bei höheren oss-Werten werden zusätzliche Bits zur Genauigkeitssteigerung genutzt. Der anschließende Bit-Shift normalisiert den Rohwert entsprechend der gewählten Auflösung.

Der endgültige, kompensierte Luftdruckwert wird anschließend mit der Funktion BMP180_GetPress() berechnet:

```text
X1 = (UT - AC6) * AC5 / 2^15
X2 = (MC * 2^11) / (X1 + MD)
B5 = X1 + X2
B6 = B5 - 4000

X1 = (B2 * B6^2 / 2^12) / 2^11
X2 = (AC2 * B6) / 2^11
X3 = X1 + X2

B3 = ((AC1 * 4 + X3) * 2^oss + 2) / 4

X1 = (AC3 * B6) / 2^13
X2 = (B1 * B6^2 / 2^12) / 2^16
X3 = (X1 + X2 + 2) / 4

B4 = AC4 * (X3 + 32768) / 2^15
B7 = (UP - B3) * 50000 / 2^oss

if B7 < 2^31:
    p = (B7 * 2) / B4
else:
    p = (B7 / B4) * 2

X1 = (p / 2^8)^2 * 3038 / 2^16
X2 = -7357 * p / 2^16
p  = p + (X1 + X2 + 3791) / 2^4
```

### [ Extra ] Seehöhe: Aus Luftdruck mit Referenzhöhe bestimmen

Mit der Funktion BMP180_GetAlt() kann aus dem gemessenen Luftdruck die ungefähre Höhe über dem Meeresspiegel berechnet werden.

Grundlage dafür ist der Zusammenhang zwischen Luftdruck und Höhe:
Mit zunehmender Höhe nimmt der Luftdruck ab.

Die allgemeine barometrische Höhenformel lautet:

```text
h = 44330 * (1 - (p / p0)^(1 / 5.255))
```

Dabei gilt:

p ... gemessener Luftdruck in Pascal (Pa)

p0 ... Referenzdruck auf Meereshöhe in Pascal (Pa)

h ... Höhe über dem Meeresspiegel in Metern (m)

Da die vollständige Formel Potenzfunktionen benötigt und dadurch auf Mikrocontrollern rechenintensiv ist, verwendet die Funktion eine lineare Näherung für kleine Höhenunterschiede:

```text
dh = (p0 - p) * 8434 / p0
```
Diese Näherung liefert für typische Anwendungen ausreichend genaue Ergebnisse und benötigt deutlich weniger Rechenaufwand.


## Überblick Software 

Kurzüberblick über Ablauf und Anzeigeverhalten der Software:

### Messung

Gemessen wird im normalen Betriebsmodus in der Hauptschleife von main.c kontinuierlich, etwa alle 500 ms. Das passiert durch den letzten HAL_Delay(500) in der Schleife.

### Kalibrierung und Ableitungen

Beim ersten Lauf wird aus einem bekannten Referenzhöhenwert (`KnownAltitudeMeters` in `main.c`) der statische Meeresspiegeldruck berechnet (`SeaLevelPressure`).

Aus dem gemessenen Druck werden die berechnete Höhe (`Altitude`) und die Differenz seit der letzten Messung (`PressureDelta`, in Pa) abgeleitet.

### Visualisierung

Zur Visualisierung wurde die Sieben Segment Anzeige auf dem FH Übungsboard verwendet, mit der dafür zur Verfügung gestellten Bibliothek von Prof Paulis aus dem 3. Semester, WS25 Jahrgang AE27.

Die 7‑Segmentanzeige ist auf Werte 0..9999 begrenzt; Werte außerhalb dieses Bereichs werden auf 9999 gekappt.

#### Anzeige-Steuerung

Der Anzeigemodus wird in  `main.c` per Taster gesteuert:
    - `Button1` (PD1): wechselt zwischen Luftdruck `DISP_PRESSURE` und Luftdruckdifferenz`DISP_DELTA`
   - `Button2` (PD2): wechselt zyklisch zwischen `DISP_TEMPERATURE` -> `DISP_ALTITUDE` -> `DISP_PRESSURE` ...

#### Anzeigeformate (je Modus)

   - `DISP_PRESSURE`: zeigt Druck skaliert als *Bar* mit drei Nachkommastellen (Beispiel: `0994` mit Dezimalpunkt links ergibt `0.994` -> entspricht ~0.994 bar). Intern wird `Pressure/100` verwendet und der Dezimalpunkt so gesetzt, dass `0.xxx` erscheint.
   - `DISP_DELTA`: absolute Druckänderung in Pascal, ganzzahlig, keine Nachkommastellen.
   - `DISP_TEMPERATURE`: Temperatur in °C mit zwei Nachkommastellen (z.B. `08.30` für 8.30 °C). Negative Temperaturen werden auf der 7‑Seg nicht gezeigt (stattdessen `----`).
   - `DISP_ALTITUDE`: Höhe in Metern, ganzzahliger Wert (z.B. `0266` = 266 m).
 
## Test-SW

Die wiederverwendbare Test-SW für den BMP180 liegt in `Core/Inc/test_bmp180.h` und `Core/Src/test_bmp180.c`.
Sie läuft auf dem STM32 selbst und prüft den Sensor direkt über I2C.

### Was wird getestet?

- I2C-Erreichbarkeit des BMP180 über `TestBMP180_CheckI2C()`.
- Initialisierung des Sensors über `TestBMP180_Init()`.
- Ein kompletter Messzyklus über `TestBMP180_RunOnce()`.
- Die Rückgabewerte für Temperatur, Luftdruck, Höhe und Druckdifferenz.

### Struktur der Testdateien

- Headerdatei mit Funktionsdeklarationen: `Core/Inc/test_bmp180.h`
- Quelldatei mit der Testimplementierung: `Core/Src/test_bmp180.c`
- Einbindung und Start im Hauptprogramm: `Core/Src/main.c`
- VS-Code-Task zum Ausführen der Tests: `.vscode/tasks.json`

### Voraussetzungen

- STM32F4 Discovery Board ist angeschlossen.
- BMP180 ist per I2C korrekt verdrahtet.
- Das Projekt wird mit `BMP180_TEST` als Compile-Definition gebaut.
- Für den Flash-Task müssen `st-flash` und das ARM-Objcopy-Tool verfügbar sein.

### Ablauf auf dem STM32

1. Der Testtask baut und flasht die Test-Firmware.
2. `main.c` startet im `BMP180_TEST`-Pfad direkt nach der I2C-Initialisierung.
3. `TestBMP180_CheckI2C()` prüft das Chip-ID-Register `0xD0` gegen `0x55`.
4. Danach liest `TestBMP180_RunOnce()` die Messwerte ein und hält sie für den Debugger fest.

## Lösung der Aufgabenstellung

Zusammenfassend wird der Luftdruckunterschied im Programm folgendermaßen berechnet:

- **Messung**: `PressurePa = BMP180_GetPress(oss)` liest den aktuellen Luftdruck in Pascal (Pa).

- **Differenz**: `PressureDelta = PressurePa - PressurePrev` (Pa). `PressurePrev` wird nach jeder Messung aktualisiert.

- **Anzeige**: `PressureBarX1000 = (PressurePa + 50) / 100` skaliert Pa zu bar*1000 für die 7‑Segment‑Anzeige (z.B. 994 -> 0.994 bar).


Alle Rechnungen erfolgen in ganzen Einheiten (Integer) zur Laufzeit auf dem STM32 (keine Gleitkommazahlen), um FPU‑Abhängigkeiten zu vermeiden.

### Demonstration (Foto):

\htmlonly

<figure style="text-align:center;"> <img src="working-demonstration.jpeg" alt="Arbeitsdemonstration" style="width:65%; border:1px solid black; padding:6px;" /> <figcaption> <b>Abbildung:</b> Finaler Aufbau mit BMP180 und STM32F4 Discovery Board </figcaption> </figure> \endhtmlonly

\latexonly
\begin{figure}[h]
\centering
\fbox{
\includegraphics[width=0.65\textwidth]{../docs/images/working-demonstration.jpeg}
}
\caption{Finaler Aufbau mit BMP180 und STM32F4 Discovery Board}
\label{fig:demo}
\end{figure}
\endlatexonly

## Interpretation

Eine Luftdruckänderung ist im Alltag meist nur eine kleine, langsame Verschiebung des Messwerts in Pascal oder hPa. Sinkt der Luftdruck über längere Zeit, deutet das oft auf ein nahendes Tiefdruckgebiet und damit auf instabileres Wetter hin. Steigt der Luftdruck dagegen langsam, ist das häufig ein Zeichen für ein Hochdruckgebiet und damit für stabileres Wetter.

Im Projekt zeigt sich das als kleine Änderung von `PressurePa` und als Wert in `PressureDelta`. Die Anzeige macht diese Veränderung sichtbar, auch wenn sie auf den ersten Blick nur aus wenigen Einheiten besteht.


## Quellen

Nach kurzer Online-Recherche, sind wir bereits auf diesen Artikel gestoßen, der uns durch die Aufgabenstellung geführt hat: 
- https://controllerstech.com/interface-bmp180-with-stm32/ 

Weitere nützliche Seiten:
- https://de.wikipedia.org/wiki/Barometrische_H%C3%B6henformel
- https://de.wikipedia.org/wiki/Luftdruck