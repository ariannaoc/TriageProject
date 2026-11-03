cmake_minimum_required(VERSION 3.20)
project(TriageProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Habilitar automatización de Qt (necesario para ventanas y botones)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Buscar las librerías obligatorias
find_package(Designar REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

include_directories(include)

# Archivos ejecutables y fuentes del proyecto
add_executable(TriageProject
    src/main.cpp
    src/MainWindow.cpp
    src/LoginWidget.cpp
    include/MainWindow.hpp
    include/LoginWidget.hpp
    include/ModelosCentro.hpp
)

# Enlazar las dependencias al ejecutable
target_link_libraries(TriageProject PRIVATE
    Designar::Designar
    Qt6::Widgets
    Qt6::Sql
)