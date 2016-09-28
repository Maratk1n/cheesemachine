#-------------------------------------------------
#
# Project created by QtCreator 2016-09-24T20:35:41
#
#-------------------------------------------------

QT       += core gui widgets serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = CheeseMachine
TEMPLATE = app


SOURCES += main.cpp\
        mymainwindow.cpp \
    qcustomplot.cpp \
    settingsdialog.cpp \
    console.cpp

HEADERS  += mymainwindow.h \
    qcustomplot.h \
    settingsdialog.h \
    console.h

FORMS    += mymainwindow.ui \
    settingsdialog.ui

RESOURCES += \
    icons.qrc

RC_FILE += mainicon.rc
