# Luftdruck Sensor Projekt mit STM32F4 Discovery Board
Projekt für LVA `Angewandte Mikrocontrollerprogrammierung`, SS26 Jahrgang AE27

## Aufgabenstellung
Messen Sie Änderungen des Luftdrucks und zeigen Sie diese an.

| Nr | Aufgabe | Erledigt |
|----|-----------------------------------------------------------------------------------------------------------------------------------|:---:|
| 1  | Analyse der Funktionen der jeweiligen Komponente (Datenblatt aus Web besorgen!) | [ x ] |
| 2  | Anschluss der jeweiligen Komponente an den STM32 über die Schnittstelle, die der jeweilige Sensor bereithält. | [ x ] |
| 3  | Inbetriebnahme und (nachweisliche!) Lösung der gestellten Aufgabe. | [ ] |
| 4  | Geeignete Visualisierung der Ergebnisse unter Zuhilfenahme des E/A-Boards (7-Segmentanzeige, LED, usw.) oder eines 2004-LCD-Displays. | [ ] |
| 5  | Erstellung einer Test-SW auf dem STM32, wobei die Bedienung der jeweiligen Komponente in wiederverwendbare Funktionen ausgelagert sein muss. | [ ] |
| 6  | Dokumentation von Anschluss, Funktion, Parametern, Randbedingungen usw. sowie der SW. | [ ] |

## Sensordaten: BMP180
Der Sensor GY-BMP180 ist in der Lage, Temperatur, Luftdruck und Luftfeuchte
zu messen

Siehe Datenblatt: https://cdn-shop.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf


### Höheneinstellung BMP180

Der BMP180-Sensor misst Luftdruck und berechnet daraus die Höhe. 

**Für Wien (171m über NN):**

```cpp
const float reference_altitude = 171.0; // Meter - anpassen je nach deinem genauen Standort
```

Die Höhe wird im Code berechnet, nicht am Sensor selbst eingestellt. Nutze deine Garmin-Höhe als Referenzwert für die genaueste Kalibrierung.

Der Sensor kann Höhenänderungen mit einer Genauigkeit von ±0,25m messen (Ultra-High-Resolution-Modus).


## I2C Kommunikation
Die I2C Addresse des BMP180 ist `0xEE` laut dem Datenblatt


### Verkabelung

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
Project generiert in CubeMX 


## Bibliotheksfunktionen

Auf Seite 15 vom Datenblatt ist der Algorithmus für Druck und Temperatur Messung dargestellt:

<img src="docs/images/algorithm_datasheet.png" alt="Algorithmus zur Berechnung von Druck und Temperatur (Seite 15)" style="max-width:500px; width:100%; height:auto;" />

## Visualisierung
Sieben Segment Anzeige auf dem FH Übungsboard, Bibliothek dafür zur Verfügung gestellt von Prof Paulis im 3. Semester, `Programmieren von Mikrocontrollern UE", WS25 Jahrgang AE27

## Tests