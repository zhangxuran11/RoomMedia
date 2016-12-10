#-------------------------------------------------
#
# Project created by QtCreator 2011-05-11T13:56:34
#
#-------------------------------------------------

QT       += core gui network

QMAKE_LFLAGS += -unwind -g -ldl -rdynamic
QMAKE_CFLAGS += -unwind -g -ldl -rdynamic
TARGET = RoomMedia
TEMPLATE = app
include(ZTPManager/ZTPManager.pri)
include(ZTools/ZTools.pri)
include(MediaPkg/MediaPkg.pri)
include(qextserialport/src/qextserialport.pri)
include(GPS_Panel/GPS_Panel.pri)
include(ZDaoPkg/ZDaoPkg.pri)
#DEFINES += ARM
#DEFINES += X86
#QMAKE_CFLAGS = `pkg-config --cflags --libs dbus-glib-1`
#QMAKE_CFLAGS +=
#QMAKE_CXXFLAGS = $$QMAKE_CFLAGS
DEFINES += $$(ARCH)
contains( DEFINES, arm ) {

    DEFINES += ARM
}
!contains( DEFINES, arm ) {
    DEFINES += X86
}

SOURCES += main.cpp\
        CQplayerGUI.cpp \
    callctlunit.cpp \
    workthread.cpp \
    globalinfo.cpp \
    softwatchdog.cpp \
    zutility.cpp \
    roompanel.cpp \
    crashtool.cpp \
    versionsender.cpp \
    mytranslator.cpp

HEADERS  += CQplayerGUI.h \
    config.h \
    callctlunit.h \
    workthread.h \
    globalinfo.h \
    softwatchdog.h \
    zutility.h \
    roompanel.h \
    crashtool.h \
    versionsender.h \
    mytranslator.h

FORMS    += CQplayerGUI.ui \


contains( DEFINES, ARM ) {

    LIBS    +=-lgstreamer-0.10
    LIBS +=/opt/poky/qtx11-1.7/sysroots/armv7a-vfp-neon-poky-linux-gnueabi/usr/lib/libgstinterfaces-0.10.so

    LIBS +=/opt/poky/qtx11-1.7/sysroots/armv7a-vfp-neon-poky-linux-gnueabi/usr/lib/libgobject-2.0.so.0
    LIBS +=/opt/poky/qtx11-1.7/sysroots/armv7a-vfp-neon-poky-linux-gnueabi/usr/lib/libglib-2.0.so.0
    INCLUDEPATH +=/opt/poky/qtx11-1.7/sysroots/armv7a-vfp-neon-poky-linux-gnueabi/usr/include/glib-2.0/ \
                 /opt/poky/qtx11-1.7/sysroots/armv7a-vfp-neon-poky-linux-gnueabi/usr/include/glib-2.0/glib/ \
                /opt/poky/qtx11-1.7/sysroots/armv7a-vfp-neon-poky-linux-gnueabi/usr/include/gstreamer-0.10/ \
                /opt/poky/qtx11-1.7/sysroots/armv7a-vfp-neon-poky-linux-gnueabi/usr/lib/glib-2.0/include/ \
                /opt/poky/qtx11-1.7/sysroots/armv7a-vfp-neon-poky-linux-gnueabi/usr/include/libxml2/ \
                /opt/poky/qtx11-1.7/sysroots/armv7a-vfp-neon-poky-linux-gnueabi/usr/lib/i386-linux-gnu/glib-2.0/include/ \
                /usr/include/qt4/

    mytarget.commands += scp ./${TARGET} root@192.168.13.22:/appbin/
}
contains( DEFINES, X86 ) {

    LIBS    +=-lgstreamer-0.10 -lgstreamermm-0.10 -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lrt -lxml2 -lglib-2.0
    LIBS +=/usr/lib/i386-linux-gnu/libgstinterfaces-0.10.so

    LIBS +=/usr/lib/i386-linux-gnu/libgobject-2.0.so.0
    LIBS +=/usr/lib/vmware-tools/lib32/libglib-2.0.so.0/libglib-2.0.so.0
    INCLUDEPATH +=/usr/include/glib-2.0/ \
                 /usr/include/glib-2.0/glib/ \
                /usr/include/gstreamer-0.10/ \
                /usr/lib/i386-linux-gnu/glib-2.0/include/ \
                /usr/include/libxml2/ \
                /usr/lib/i386-linux-gnu/glib-2.0/include/ \
                /usr/lib/gstreamermm-0.10/include/
}
RESOURCES += \
    resource.qrc

mytarget.target = a

mytarget.depends =



QMAKE_EXTRA_TARGETS += mytarget

OTHER_FILES += \
    qss/mainframe.qss \
    qss/text_style.qss

DISTFILES += \
    test.ts
