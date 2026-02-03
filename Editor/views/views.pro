QT       += core gui

TEMPLATE = lib
TARGET = views

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 shared
QMAKE_CXXFLAGS_DEBUG += -fPIC
QMAKE_CXXFLAGS_RELEASE += -fPIC

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    layer_view.cpp \
    metal_view.cpp \
    scene_view.cpp \
    toolbar.cpp \
    view.cpp

HEADERS += \
    layer_view.hpp \
    metal_view.hpp \
    scene_view.hpp \
    toolbar.hpp \
    view.hpp

FORMS +=

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
