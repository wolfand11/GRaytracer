#-------------------------------------------------
#
# Project created by QtCreator 2021-01-08T13:39:43
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GRaster
TEMPLATE = app
CONFIG += console c++14
QMAKE_CXXFLAGS += -ftemplate-backtrace-limit=0

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        gcamera.cpp \
        gcolor.cpp \
        ggameobject.cpp \
        gmath.cpp \
        gmathutils.cpp \
        gmodel.cpp \
        gray.cpp \
        graytracer.cpp \
        gutils.cpp \
        main.cpp \
        tgaimage.cpp

HEADERS += \
        gbuffer.h \
        gcamera.h \
        gcolor.h \
        ggameobject.h \
        ggraphiclibdefine.h \
        glog.h \
        gmath.h \
        gmathutils.h \
        gmodel.h \
        gray.h \
        graytracer.h \
        gscene.h \
        gutils.h \
        tgaimage.h

FORMS += \
        gmodalmessage.ui \
        graytracer.ui

RESOURCES +=