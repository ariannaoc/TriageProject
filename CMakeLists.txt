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
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Archivos ejecutables y fuentes del proyecto
qt_add_executable(TriageProject
    src/BosqueTriaje.cpp
    src/MotorTriaje.cpp
    src/GestorCentros.cpp
    src/MotorEnrutamiento.cpp
    include/modelos_triaje.hpp
    include/BosqueTriaje.hpp
    include/MotorTriaje.hpp
    include/GestorCentros.hpp
    include/MotorEnrutamiento.hpp
    src/main.cpp
    src/MainWindow.cpp
    src/LoginWidget.cpp
    src/VistaMain.cpp
    src/VistaMain.ui
    include/MainWindow.hpp
    include/LoginWidget.hpp
    include/VistaMain.hpp
    include/ModelosCentro.hpp
    recursos.qrc
)

# Enlazar las dependencias al ejecutable
target_link_libraries(TriageProject PRIVATE
    Designar::Designar
    Qt6::Widgets
    Qt6::Sql
)

# Target de pruebas del motor de triaje y enrutamiento (sin Qt)
add_executable(test_motores
    tests/test_motores.cpp
    src/BosqueTriaje.cpp
    src/MotorTriaje.cpp
    src/GestorCentros.cpp
    src/MotorEnrutamiento.cpp
)
target_link_libraries(test_motores PRIVATE Designar::Designar)
