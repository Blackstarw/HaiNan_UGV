#QT -= gui
CONFIG += console
QT       += serialbus
QT       += serialport
QT       += network

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
    agv_ctr.cpp \
    camera_proc.cpp \
    canbus_proc.cpp \
    http_proc.cpp \
    qx_proc.cpp

HEADERS += \
    agv_ctr.h \
    agv_def.h \
    camera_proc.h \
    canbus_proc.h \
    http_proc.h \
    qx_proc.h


unix:!macx: LIBS += -L$$PWD/../../../../../../usr/local/opencv/lib/ -lopencv_core

INCLUDEPATH += $$PWD/../../../../../../usr/local/opencv/include
DEPENDPATH += $$PWD/../../../../../../usr/local/opencv/include

unix:!macx: LIBS += -L$$PWD/../../../../../../usr/local/opencv/lib/ -lopencv_highgui

INCLUDEPATH += $$PWD/../../../../../../usr/local/opencv/include
DEPENDPATH += $$PWD/../../../../../../usr/local/opencv/include

unix:!macx: LIBS += -L$$PWD/../../../../../../usr/local/opencv/lib/ -lopencv_features2d

INCLUDEPATH += $$PWD/../../../../../../usr/local/opencv/include
DEPENDPATH += $$PWD/../../../../../../usr/local/opencv/include

unix:!macx: LIBS += -L$$PWD/../../../../../../usr/local/opencv/lib/ -lopencv_imgcodecs

INCLUDEPATH += $$PWD/../../../../../../usr/local/opencv/include
DEPENDPATH += $$PWD/../../../../../../usr/local/opencv/include

unix:!macx: LIBS += -L$$PWD/../../../../../../usr/local/opencv/lib/ -lopencv_imgproc

INCLUDEPATH += $$PWD/../../../../../../usr/local/opencv/include
DEPENDPATH += $$PWD/../../../../../../usr/local/opencv/include

unix:!macx: LIBS += -L$$PWD/../../../../../../usr/local/opencv/lib/ -lopencv_objdetect

INCLUDEPATH += $$PWD/../../../../../../usr/local/opencv/include
DEPENDPATH += $$PWD/../../../../../../usr/local/opencv/include

unix:!macx: LIBS += -L$$PWD/../../../../../../usr/local/opencv/lib/ -lopencv_photo

INCLUDEPATH += $$PWD/../../../../../../usr/local/opencv/include
DEPENDPATH += $$PWD/../../../../../../usr/local/opencv/include

unix:!macx: LIBS += -L$$PWD/../../../../../../usr/local/opencv/lib/ -lopencv_video

INCLUDEPATH += $$PWD/../../../../../../usr/local/opencv/include
DEPENDPATH += $$PWD/../../../../../../usr/local/opencv/include

unix:!macx: LIBS += -L$$PWD/../../../../../../usr/local/opencv/lib/ -lopencv_videoio

INCLUDEPATH += $$PWD/../../../../../../usr/local/opencv/include
DEPENDPATH += $$PWD/../../../../../../usr/local/opencv/include




unix:!macx: LIBS += -L$$PWD/../../QX_Sdk/lib/release/ -lqxwz-pssdk-1.4.3-20200514-jimujiqiren-ubuntu-aarch64-production-release

INCLUDEPATH += $$PWD/../../QX_Sdk/include
DEPENDPATH += $$PWD/../../QX_Sdk/include
