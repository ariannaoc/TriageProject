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
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Network
             OPTIONAL_COMPONENTS Positioning Location QuickWidgets)
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
    src/main.cpp
    src/MainWindow.cpp
    src/LoginWidget.cpp
    src/VistaMain.cpp
    src/VistaMain.ui
    src/FormularioPacienteWidget.cpp
    src/ApiClient.cpp
    src/ModelosApi.cpp
    include/MainWindow.hpp
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
    Qt6::Network
)

if(Qt6Positioning_FOUND)
    target_compile_definitions(TriageProject PRIVATE HAVE_QT_POSITIONING)
    target_link_libraries(TriageProject PRIVATE Qt6::Positioning)
endif()

if(Qt6Location_FOUND AND Qt6QuickWidgets_FOUND)
    target_compile_definitions(TriageProject PRIVATE HAVE_QT_LOCATION)
    target_link_libraries(TriageProject PRIVATE Qt6::Location Qt6::QuickWidgets)
endif()

if(NOT ANDROID)
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
endif()