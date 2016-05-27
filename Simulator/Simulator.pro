#-------------------------------------------------
#
# Project created by QtCreator 2016-02-08T22:44:39
#
#-------------------------------------------------

LIBS += -L/usr/local/lib \
-lopencv_core \
-lopencv_imgproc \
-lopencv_highgui \
-lopencv_imgcodecs

CONFIG += c++11
QT += widgets testlib opengl

#FOR THE FOLLOWING, CHANGE TO WHERE EVER YOUR IRRLICHT FILES ARE
#IF YOU GET ALOT OF QT ERRORS, YOU WILL NEED THE MODIFIED IRRLICHT FILES THAT I HAVE
LIBS += -L$$PWD/irrlicht-1.8.3/lib/Linux/ -lIrrlicht -lGLU -lGL -lXrandr -lXext -lX11
INCLUDEPATH += $$PWD/irrlicht-1.8.3/lib/Linux $$PWD/irrlicht-1.8.3/include
unix:!macx: PRE_TARGETDEPS += $$PWD/irrlicht-1.8.3/lib/Linux/libIrrlicht.a

SOURCES += main.cpp \
    Objects/Buoy.cpp \
    Objects/SimObject.cpp \
    DataStorage.cpp \
    InputHandler.cpp \
    SimLogger.cpp \
    Sim.cpp \
    Objects/SimSub.cpp

HEADERS += \
    Objects/Buoy.h \
    Objects/SimObject.h \
    DataStorage.h \
    InputHandler.h \
    SimLogger.h \
    Sim.h \
    Objects/SimSub.h

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/irrlicht-1.8.3/lib/Linux/release/ -lIrrlicht
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/irrlicht-1.8.3/lib/Linux/debug/ -lIrrlicht
#else:unix: LIBS += -L$$PWD/irrlicht-1.8.3/lib/Linux/ -lIrrlicht

#INCLUDEPATH += $$PWD/irrlicht-1.8.3/lib/Linux $$PWD/irrlicht-1.8.3/include
#DEPENDPATH += $$PWD/irrlicht-1.8.3/lib/Linux

#win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/irrlicht-1.8.3/lib/Linux/release/libIrrlicht.a
#else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/irrlicht-1.8.3/lib/Linux/debug/libIrrlicht.a
#else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/irrlicht-1.8.3/lib/Linux/release/Irrlicht.lib
#else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/irrlicht-1.8.3/lib/Linux/debug/Irrlicht.lib
#else:unix: PRE_TARGETDEPS += $$PWD/irrlicht-1.8.3/lib/Linux/libIrrlicht.a
