#-------------------------------------------------
#
# Project created by QtCreator 2021-01-08T13:39:43
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GRaytracer
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
        gaabb.cpp \
        gbvh.cpp \
        gcamera.cpp \
        gcolor.cpp \
        ggameobject.cpp \
        gintegrator.cpp \
        glight.cpp \
        gmaterial.cpp \
        gmath.cpp \
        gmathutils.cpp \
        gmodel.cpp \
        gobjmodel.cpp \
        gray.cpp \
        graytracer.cpp \
        gsampler.cpp \
        gtexture.cpp \
        gtriangle.cpp \
        gutils.cpp \
        main.cpp \
        tgaimage.cpp

HEADERS += \
        gaabb.h \
        gbuffer.h \
        gbvh.h \
        gcamera.h \
        gcolor.h \
        ggameobject.h \
        ggraphiclibdefine.h \
        gintegrator.h \
        glight.h \
        glog.h \
        gmaterial.h \
        gmath.h \
        gmathutils.h \
        gmodel.h \
        gobjmodel.h \
        gray.h \
        graytracer.h \
        gsampler.h \
        gscene.h \
        gtexture.h \
        gtriangle.h \
        gutils.h \
        tgaimage.h

FORMS += \
        gmodalmessage.ui \
        graytracer.ui

RESOURCES +=
