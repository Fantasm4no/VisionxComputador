cmake_minimum_required(VERSION 3.16)

project(VisorMRI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Activar autoprocesamiento de Qt (si usas .ui, .qrc, etc.)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Encontrar OpenCV
find_package(OpenCV REQUIRED)

# Encontrar ITK
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

# Encontrar Qt
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Fuentes del proyecto
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

# Crear ejecutable
add_executable(VisorMRI ${PROJECT_SOURCES}
    filterswindow.h filterswindow.cpp filterswindow.ui



)

# Enlazar librerías
target_link_libraries(VisorMRI
    PRIVATE
    Qt6::Widgets
    ${OpenCV_LIBS}
    ${ITK_LIBRARIES}
)
