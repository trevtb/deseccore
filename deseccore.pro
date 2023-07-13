#-------------------------------------------------
#
# Project created by QtCreator 2015-06-04T22:40:00
#
#-------------------------------------------------

QT       += core gui sql

TARGET = deseccore
target.files = deseccore
target.path = /home/pi/
INSTALLS += target

CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    logger.cpp \
    logincontroller.cpp \
    requestmapper.cpp \
    maincontroller.cpp \
    mjpegclient.cpp \
    mjpegcontroller.cpp \
    dbhelper.cpp

HEADERS += \
    logger.h \
    logincontroller.h \
    requestmapper.h \
    maincontroller.h \
    mjpegclient.h \
    mjpegcontroller.h \
    dbhelper.h

include($$PWD/httpserver/httpserver.pri)
