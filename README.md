## VisionProyecto - Aplicación en C++ con OpenCV e ITK

### Descripción
Este proyecto consiste en una herramienta de escritorio desarrollada en C++ que facilita la visualización y procesamiento de imágenes médicas en formato NIfTI (.nii). Aprovechando las capacidades de OpenCV para el procesamiento avanzado de imágenes y ITK para la manipulación eficiente de archivos volumétricos, esta aplicación permite interactuar con imágenes de resonancia magnética (RM) y realizar operaciones sobre ellas para mejorar la visualización y resaltar áreas específicas.

### Características

- [x] *Visualización interactiva de imágenes volumétricas en formato NIfTI.*
- [x] *Navegación fluida entre cortes individuales de las imágenes.*
- [x] *Filtros aplicables como ecualización, suavizado y detección de bordes.*
- [x] *Aplicación de diversos filtros, como ecualización, suavizado y detección de bordes.*
- [x] *Resaltado de áreas de interés (como tumores) mediante técnicas de segmentación y colorización.*
- [x] *Generación de videos de los cortes con transiciones suaves entre ellos.*
- [x] *Interfaz de usuario desarrollada con Qt, ofreciendo una experiencia intuitiva.*


### Requisitos
- C++11 o superior
- CMake para la configuración del proyecto.
- OpenCV (4.5.0 o superior)
- ITK (5.2 o superior)
- Qt (5.15 o superior)
- Compilador C++ compatible (como GCC o Clang)

### Instalación
Clona el repositorio:
text
git clone https://github.com/Fantasm4no/VisionxComputador.git


Accede al directorio del proyecto:
text
cd visionproyecto


Crea un directorio para la compilación:
text
mkdir build
cd build

Ejecuta CMake para generar los archivos de construcción:

text
cmake ..


Compila el proyecto:
text
make


Ejecuta la aplicación:
text
./visionproyecto

## Uso

- Abre la aplicación y carga un archivo NIfTI (.nii).
- Usa los controles deslizantes para navegar entre los diferentes cortes de la imagen.
- Aplica filtros como suavizado, detección de bordes o ecualización para mejorar la visualización.
- Resalta áreas de interés, como tumores, utilizando las herramientas de segmentación disponibles.
- Puedes generar un video de los cortes con transiciones suaves.

## Estructura del proyecto

text
visor-nifti/
│
├── CMakeLists.txt        # Archivo de configuración de CMake
├── Source Files/         # Código fuente de la aplicación
│   ├── main.cpp          # Punto de entrada de la aplicación
│   ├── mainwindow.cpp    # Funciones para cargar imágenes NIfTI
│   ├── filterswindow.cpp # Funciones para procesar imágenes
│   └── ...
├── Header Files/         # Archivos de cabecera
│   ├── mainwindow.h      # Definiciones de las funciones de carga
│   ├── filterswindow.h   # Definiciones de funciones de procesamiento
│   └── ...
├── output/               # Archivos de recursos (imágenes, iconos, etc.)
├── README.md             # Este archivo
└── ...


## AUTORES

- GRANDA LÓPEZ HENRY AUGUSTO
- ZHIGUE GRANDA ERICK FERNANDO
