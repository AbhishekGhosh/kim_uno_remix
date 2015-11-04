#-------------------------------------------------
#
# QT Creator build file for KIM Uno for Desktop
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = KIM
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp\
    interface.c  \
    ../KimUnoRemix/cpu.c \
    ../KimUnoRemix/memory.c \
    terminalinterface.cpp \
    console.cpp

HEADERS  += mainwindow.h\
    interface.h \
    ../KimUnoRemix/config.h \
    ../KimUnoRemix/memory.h \
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
