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
    interface.c  \
    ../sources/KimUnoRemix/cpu.c \
    ../sources/KimUnoRemix/memory.c \
    terminalinterface.cpp \
    console.cpp

HEADERS  += mainwindow.h\
    interface.h \
    ../sources/KimUnoRemix/config.h \
    ../sources/KimUnoRemix/memory.h \
    Arduino.h \
    terminalinterface.h \
    console.h

FORMS    += mainwindow.ui \
    terminalinterface.ui

OTHER_FILES += \
    Notes.txt

#windows resources
RC_FILE = Resources/windows.rc

#mac resources
ICON = Resources/mac.icns
