QT += widgets

TEMPLATE = lib
DEFINES += EDITOR_MODELS_LIBRARY

CONFIG += c++23

INCLUDEPATH += "../../design_models/include"

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    editor_models.cpp \

HEADERS += \
    editor_models_global.h \
    editor_models.h \
    layer_model.hpp \
    scene_model.hpp

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
