cmake_minimum_required(VERSION 3.16)

project(VisionxComputador LANGUAGES CXX)

# Configuración del estándar de C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Habilitar características automáticas de Qt
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Incluir directorios
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Encontrar Qt
find_package(Qt6 REQUIRED COMPONENTS Widgets PrintSupport)

# Encontrar OpenCV
find_package(OpenCV REQUIRED)

# Encontrar ITK
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

# Archivos fuente del proyecto
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui

    filterswindow.cpp
    filterswindow.h
    filterswindow.ui

    descriptionwindow.cpp
    descriptionwindow.h
    descriptionwindow.ui

    statswindow.cpp
    statswindow.h
    statswindow.ui

    qcustomplot.cpp
    qcustomplot.h
)

# Crear ejecutable
add_executable(VisionxComputador ${PROJECT_SOURCES})

# Enlazar librerías necesarias
target_link_libraries(VisionxComputador
    Qt6::Widgets
    Qt6::PrintSupport
    ${OpenCV_LIBS}
    ${ITK_LIBRARIES}
)
