QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++23

QMAKE_CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer -g
QMAKE_LFLAGS += -fsanitize=address

INCLUDEPATH += ../../design_models/include
INCLUDEPATH += ../views/
INCLUDEPATH += ../presenters/
INCLUDEPATH += ../editor_models/

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

unix {
    LIBS += -L$$PWD/../editor_models/build/ -leditor_models
    LIBS += -L$$PWD/../presenters/build/ -lpresenters
    LIBS += -L$$PWD/../views/build/ -lviews
}
