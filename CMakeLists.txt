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
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Network)
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(include)

# Cliente
qt_add_executable(TriageProject
    src/main.cpp
    src/MainWindow.cpp
    src/LoginWidget.cpp
    src/VistaMain.cpp
    src/VistaMain.ui
    src/FormularioPacienteWidget.cpp
    include/MainWindow.hpp
    include/LoginWidget.hpp
    include/VistaMain.hpp
    include/ModelosCentro.hpp
    include/FormularioPacienteWidget.hpp
    recursos.qrc
)

target_link_libraries(TriageProject PRIVATE
    Designar::Designar
    Qt6::Widgets
    Qt6::Sql
    Qt6::Network
)

# Servidor API REST
add_executable(triage_api
    api_server/server.cpp
)

target_include_directories(triage_api PRIVATE include)

target_link_libraries(triage_api PRIVATE
    SQLite::SQLite3
    OpenSSL::Crypto
    pthread
)