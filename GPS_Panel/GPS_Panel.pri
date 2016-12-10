INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    GPS_Panel/map_panel.h \
    GPS_Panel/geogpixsample.h \
    GPS_Panel/stackpanel.h \
    GPS_Panel/checkversionthread.h

SOURCES += \
    GPS_Panel/map_panel.cpp \
    GPS_Panel/geogpixsample.cpp \
    GPS_Panel/stackpanel.cpp \
    GPS_Panel/checkversionthread.cpp

FORMS += \
    GPS_Panel/stackpanel.ui

OTHER_FILES += \
    GPS_Panel/tablewidget_stylesheet.qss

RESOURCES += \
    GPS_Panel/gpspanel_resource.qrc
