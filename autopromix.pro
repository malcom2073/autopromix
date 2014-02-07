#-------------------------------------------------
#
# Project created by QtCreator 2013-01-04T20:55:56
#
#-------------------------------------------------

QT       += core gui

TARGET = autopromix
TEMPLATE = app

include(serialport/apmserial.pri)

SOURCES += main.cpp\
        mainwindow.cpp \
    apuone.cpp

HEADERS  += mainwindow.h \
    apuone.h

FORMS    += mainwindow.ui
