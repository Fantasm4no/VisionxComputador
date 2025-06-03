cmake_minimum_required(VERSION 3.16)

project(VisionxComputador LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Encontrar Qt
find_package(Qt6 REQUIRED COMPONENTS Widgets)

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
)

# Crear ejecutable
add_executable(VisionxComputador ${PROJECT_SOURCES}
    descriptionwindow.h descriptionwindow.cpp descriptionwindow.ui)

# Enlazar librerías
target_link_libraries(VisionxComputador
    Qt6::Widgets
    ${OpenCV_LIBS}
    ${ITK_LIBRARIES}
)

