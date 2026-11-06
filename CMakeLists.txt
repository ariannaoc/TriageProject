cmake_minimum_required(VERSION 3.20)
project(TriageProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(Designar REQUIRED)

# libbcrypt (clona el repo y añádelo como subdirectorio o instálalo)
# Opción A: FetchContent
include(FetchContent)
FetchContent_Declare(
    bcrypt
    GIT_REPOSITORY https://github.com/trusch/libbcrypt.git
    GIT_TAG master
)
FetchContent_MakeAvailable(bcrypt)

add_executable(TriageProject
    src/main.cpp
    src/mainwindow.cpp
    src/loginwindow.cpp
    src/database.cpp
)

target_link_libraries(TriageProject PRIVATE
    Qt6::Widgets
    SQLite::SQLite3
    Designar::Designar
    bcrypt
)

target_include_directories(TriageProject PRIVATE src)
