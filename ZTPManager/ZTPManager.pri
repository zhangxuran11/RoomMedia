QT += network concurrent

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
HEADERS += \
    ZTPManager/ztpmanager.h \
    ZTPManager/ztpprotocol.h \
    ZTPManager/fragment.h \
    $$PWD/ztptask.h

SOURCES += \
    ZTPManager/ztpmanager.cpp \
    ZTPManager/ztpprotocol.cpp \
    ZTPManager/fragment.cpp \
    $$PWD/ztptask.cpp
