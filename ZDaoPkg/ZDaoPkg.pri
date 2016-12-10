INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
QT += sql concurrent
greaterThan(QT_MAJOR_VERSION, 5)|equals(QT_MAJOR_VERSION,5){
    greaterThan(QT_MINOR_VERSION, 5)|equals(QT_MINOR_VERSION, 5){
    QT += concurrent
    }
}
OTHER_FILES += \
    TrainLineDaoPkg/TrainLineDB.sql

HEADERS += \
    ZDaoPkg/resultset.h \
    ZDaoPkg/dbservice.h

SOURCES += \
    ZDaoPkg/resultset.cpp \
    ZDaoPkg/dbservice.cpp


