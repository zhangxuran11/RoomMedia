INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    MediaPkg/sdpsrcparser.h \
    MediaPkg/CQmedia.h \
    MediaPkg/CQipc.h \
    MediaPkg/CQGstbasic.h \
    MediaPkg/MediaPkgCommon.h \
    $$PWD/cqaudio.h

SOURCES += \
    MediaPkg/sdpsrcparser.cpp \
    MediaPkg/CQmedia.cpp \
    MediaPkg/CQipc.cpp \
    MediaPkg/CQGstbasic.cpp \
    MediaPkg/MediaPkgCallback.cpp \
    $$PWD/cqaudio.cpp

RESOURCES +=
