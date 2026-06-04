QT += widgets

CONFIG += c++17
CONFIG -= app_bundle

win32-g++ {
    QMAKE_CXXFLAGS += -finput-charset=UTF-8 -fexec-charset=UTF-8
}

msvc {
    QMAKE_CXXFLAGS += /utf-8
}

TARGET = PackMate
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/MainWindow.cpp \
    src/AddEditItemDialog.cpp \
    src/Models.cpp \
    src/Storage.cpp

HEADERS += \
    src/MainWindow.h \
    src/AddEditItemDialog.h \
    src/Models.h \
    src/Storage.h
