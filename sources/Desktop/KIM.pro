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
    console.cpp \
    videodisplay.cpp \
    codedrop.cpp \
    memorybrowser.cpp

HEADERS  += mainwindow.h\
    interface.h \
    ../KimUnoRemix/config.h \
    ../KimUnoRemix/memory.h \
    Arduino.h \
    terminalinterface.h \
    console.h \
    videodisplay.h \
    codedrop.h \
    memorybrowser.h

FORMS    += mainwindow.ui \
    terminalinterface.ui \
    videodisplay.ui \
    codedrop.ui \
    memorybrowser.ui

OTHER_FILES += \
    Notes.txt

#windows resources
RC_FILE = Resources/windows.rc

#mac resources
ICON = Resources/mac.icns
