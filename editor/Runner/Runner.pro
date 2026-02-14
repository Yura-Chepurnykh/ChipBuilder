QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++23

INCLUDEPATH += ../../design_models/include
INCLUDEPATH += ../views/
INCLUDEPATH += ../presenters/
INCLUDEPATH += ../editor_models/

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../editor_models/build/release/ -leditor_models
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../editor_models/build/debug/ -leditor_models
else:unix: LIBS += -L$$PWD/../editor_models/build/ -leditor_models

INCLUDEPATH += $$PWD/../editor_models/build
DEPENDPATH += $$PWD/../editor_models/build

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../views/build/release/ -lviews
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../views/build/debug/ -lviews
else:unix: LIBS += -L$$PWD/../views/build/ -lviews

INCLUDEPATH += $$PWD/../views/build
DEPENDPATH += $$PWD/../views/build

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../presenters/build/Desktop-Debug/release/ -lpresenters
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../presenters/build/Desktop-Debug/debug/ -lpresenters
else:unix: LIBS += -L$$PWD/../presenters/build/Desktop-Debug/ -lpresenters

INCLUDEPATH += $$PWD/../presenters/build/Desktop-Debug
DEPENDPATH += $$PWD/../presenters/build/Desktop-Debug

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../editor_models/build/Desktop-Debug/release/ -leditor_models
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../editor_models/build/Desktop-Debug/debug/ -leditor_models
else:unix: LIBS += -L$$PWD/../editor_models/build/Desktop-Debug/ -leditor_models

INCLUDEPATH += $$PWD/../editor_models/build/Desktop-Debug
DEPENDPATH += $$PWD/../editor_models/build/Desktop-Debug
