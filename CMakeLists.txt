cmake_minimum_required(VERSION 3.20)
project(TriageProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Encontrar DeSiGNAR instalado globalmente en el sistema
find_package(Designar REQUIRED)

# Archivo ejecutable principal de tu sistema de triaje
add_executable(TriageProject src/main.cpp)

# Enlazar la biblioteca DeSiGNAR
target_link_libraries(TriageProject PRIVATE Designar::Designar)