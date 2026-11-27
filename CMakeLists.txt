cmake_minimum_required(VERSION 3.20)
project(TriageProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Habilitar automatización de Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Buscar librerias
find_package(Designar REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Network Positioning
             OPTIONAL_COMPONENTS Location QuickWidgets)
if(NOT ANDROID)
    find_package(SQLite3 REQUIRED)
endif()
if(NOT ANDROID)
    find_package(OpenSSL REQUIRED)
else()
    find_package(OpenSSL QUIET)
endif()

include_directories(include)

# Cliente
qt_add_executable(TriageProject
    src/BosqueTriaje.cpp
    src/MotorTriaje.cpp
    src/GestorCentros.cpp
    src/MotorEnrutamiento.cpp

    src/main.cpp
    src/MainWindow.cpp
    src/BarraOcupacionWidget.cpp
    src/GraficoBarrasNivelesWidget.cpp
    src/PanelKpi3Widget.cpp
    src/GraficoLineasTendenciasWidget.cpp
    src/PanelKpi7Widget.cpp
    src/LoginWidget.cpp
    src/VistaMain.cpp
    src/VistaMain.ui
    src/FormularioPacienteWidget.cpp
    src/ApiClient.cpp
    src/ModelosApi.cpp
    include/modelos_triaje.hpp
    include/BosqueTriaje.hpp
    include/MotorTriaje.hpp
    include/GestorCentros.hpp
    include/MotorEnrutamiento.hpp
    include/MainWindow.hpp
    include/BarraOcupacionWidget.hpp
    include/GraficoBarrasNivelesWidget.hpp
    include/PanelKpi3Widget.hpp
    include/GraficoLineasTendenciasWidget.hpp
    include/PanelKpi7Widget.hpp
    include/LoginWidget.hpp
    include/VistaMain.hpp
    include/ModelosCentro.hpp
    include/FormularioPacienteWidget.hpp
    include/ApiClient.hpp
    include/ModelosApi.hpp
    recursos.qrc
)

target_link_libraries(TriageProject PRIVATE
    Designar::Designar
    Qt6::Widgets
    Qt6::Sql

# Target de pruebas del motor de triaje y enrutamiento (sin Qt)
add_executable(test_motores
    tests/test_motores.cpp
    src/BosqueTriaje.cpp
    src/MotorTriaje.cpp
    src/GestorCentros.cpp
    src/MotorEnrutamiento.cpp
)
target_link_libraries(test_motores PRIVATE Designar::Designar)
