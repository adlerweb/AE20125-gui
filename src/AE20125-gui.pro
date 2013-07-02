#-------------------------------------------------
#
# Project created by QtCreator 2013-07-02T10:07:18
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AE20125-gui
TEMPLATE = app


SOURCES += main.cpp\
        ae20125gui.cpp

HEADERS  += ae20125gui.h

FORMS    += ae20125gui.ui \
    about.ui

CONFIG   += serialport

