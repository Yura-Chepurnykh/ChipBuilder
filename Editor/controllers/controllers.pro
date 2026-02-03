QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    commands.cpp \
    main.cpp \
    mainwindow.cpp \
    metal_controller.cpp \
    scene_controller.cpp

HEADERS += \
    commands.hpp \
    mainwindow.h \
    metal_controller.hpp \
    scene_controller.hpp

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../views/build/Desktop-Debug/release/ -lviews
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../views/build/Desktop-Debug/debug/ -lviews
else:unix: LIBS += -L$$PWD/../views/build/Desktop-Debug/ -lviews

INCLUDEPATH += $$PWD/../views
DEPENDPATH += $$PWD/../views

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../models/build/Desktop-Debug/release/ -lmodels
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../models/build/Desktop-Debug/debug/ -lmodels
else:unix: LIBS += -L$$PWD/../models/build/Desktop-Debug/ -lmodels

INCLUDEPATH += $$PWD/../models
DEPENDPATH += $$PWD/../models
