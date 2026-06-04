cmake_minimum_required(VERSION 3.16)

project(PackMate VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/AddEditItemDialog.cpp
    src/AddEditItemDialog.h
    src/Models.cpp
    src/Models.h
    src/Storage.cpp
    src/Storage.h
)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(PackMate ${PROJECT_SOURCES})
else()
    add_executable(PackMate WIN32 ${PROJECT_SOURCES})
endif()

target_link_libraries(PackMate PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if(MINGW)
    target_compile_options(PackMate PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

if(MSVC)
    target_compile_options(PackMate PRIVATE /utf-8)
endif()

set_target_properties(PackMate PROPERTIES
    WIN32_EXECUTABLE TRUE
)
