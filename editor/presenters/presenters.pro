QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = lib
TARGET = presenters

CONFIG += c++23 shared debug
QMAKE_CXXFLAGS_DEBUG += -fPIC
QMAKE_CXXFLAGS_RELEASE += -fPIC

INCLUDEPATH += ../../design_models/include
INCLUDEPATH += ../editor_models/
INCLUDEPATH += ../views/
INCLUDEPATH += ../../utils/include
INCLUDEPATH += ../serializers/include

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    builder.cpp \
    commands.cpp \
    context.cpp \
    factories.cpp \
    mappers.cpp \
    menubar_presenter.cpp \
    scene_presenter.cpp \
    toolbar_presenter.cpp \
    DRCWorker.cpp

HEADERS += \
    builder.hpp \
    commands.hpp \
    context.hpp \
    factories.hpp \
    mappers.hpp \
    menubar_presenter.hpp \
    scene_presenter.hpp \
    toolbar_presenter.hpp \
    DRCWorker.hpp

FORMS +=

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../editor_models/build/Desktop-Debug/release/ -leditor_models
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../editor_models/build/Desktop-Debug/debug/ -leditor_models
else:unix: LIBS += -L$$PWD/../editor_models/build/ -leditor_models

INCLUDEPATH += $$PWD/../editor_models/build/
DEPENDPATH += $$PWD/../editor_models/build/

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../views/build/release/ -lviews
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../views/build/debug/ -lviews
else:unix: LIBS += -L$$PWD/../views/build/ -lviews

INCLUDEPATH += $$PWD/../views/build
DEPENDPATH += $$PWD/../views/build
