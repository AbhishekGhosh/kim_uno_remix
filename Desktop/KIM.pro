#-------------------------------------------------
#
# Project created by QtCreator 2015-10-22T13:14:13
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = KIM
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp\
    ../sources/KimUnoRemix/cpu.c \
    ../sources/KimUnoRemix/memory.c

HEADERS  += mainwindow.h\
    ../sources/KimUnoRemix/config.h \
    ../sources/KimUnoRemix/memory.h \
    Arduino.h

FORMS    += mainwindow.ui
