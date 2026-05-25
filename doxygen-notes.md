# Doxygen-Dokumentation

## Überblick

Doxygen generiert aus den speziell formatierten Kommentaren im Quellcode automatisch eine HTML- und LaTeX/PDF-Dokumentation.

## Installation (macOS)

```bash
brew install doxygen
brew install plantuml  # für SysML-Diagramme
```

## Konfiguration (Doxyfile)

Die `Doxyfile` ist bereits konfiguriert:

| Parameter | Wert |
|-----------|------|
| `PROJECT_NAME` | "Luftdruck Sensor Projekt: GY-BMP180 mit STM32F4 Discovery Board" |
| `EXTRACT_ALL` | YES |
| `GENERATE_HTML` | YES |
| `GENERATE_LATEX` | YES |
| `USE_MDFILE_AS_MAINPAGE` | readme.md |
| `INPUT` | `.` (alle .c/.h/.dox/.md) |

## Dokumentation generieren

```bash
# HTML
doxygen Doxyfile
open html/index.html

# PDF (LaTeX)
make -C latex
open latex/refman.pdf
```

## SysML-Diagramme rendern

```bash
plantuml docs/sysml/bmp180_sysml.puml   # Block Definition Diagram
plantuml docs/sysml/bmp180_ibd.puml     # Internal Block Diagram
```

Die generierten PNGs liegen in `docs/sysml/`.

## GitHub Pages Deployment

Bei jedem Push auf `main` wird über GitHub Actions automatisch die Doxygen-Dokumentation gebaut und auf den `gh-pages`-Branch deployed. Verfügbar unter:

```
https://<username>.github.io/Luftdruck-Sensor/
```

Der Workflow liegt in `.github/workflows/doxygen-gh-pages.yml`.
