LIBS += -L/usr/local/lib \
-lopencv_core \
-lopencv_imgproc \
-lopencv_highgui \
-lopencv_video \
-lopencv_videoio \
-lopencv_imgcodecs

LIBS += -L$$PWD/Simulator/irrlicht-1.8.3/lib/Linux/ -lIrrlicht -lGLU -lGL -lXrandr -lXext -lX11

CONFIG += c++11
greaterThan(QT_MAJOR_VERSION,4): QT += widgets

QMAKE_CXXFLAGS += -lrt

TEMPLATE = app
TARGET = RoboSub
INCLUDEPATH += . \
               src/simulator \
               src/util \
               src/view \
               src/controller/controllers \
               src/controller/task \
               src/model \
               src/model/interface \
               src/resources \
               src/resources/environment_objects \
               src/util/data \
               src/util/filter \
               src/model/state \
               test \
               src \
               test/util \
               test/util/filter \
               test/util/data \
               /usr/local/include/opencv

INCLUDEPATH += $$PWD/Simulator/irrlicht-1.8.3/lib/Linux $$PWD/Simulator/irrlicht-1.8.3/include
unix:!macx: PRE_TARGETDEPS += $$PWD/Simulator/irrlicht-1.8.3/lib/Linux/libIrrlicht.a

OTHER_FILES += src/settings/*
config.path =$${OUT_PWD}/settings
config.files = src/settings/*
INSTALLS += config

# Input
HEADERS += \
           test/CollectionTEST.h \
           test/VideoTesting.h \
           src/model/CameraModel.h \
           src/model/FPGAModel.h \
           src/model/Model.h \
           src/util/IDHasher.h \
           src/util/Logger.h \
           src/util/PropertyReader.h \
           src/util/Timer.h \
           src/util/Util.h \
           src/view/View.h \
           test/util/IDHasherTEST.h \
           src/controller/controllers/Controller.h \
           src/controller/controllers/ControllerThread.h \
           src/controller/task/BuoyTask.h \
           src/controller/task/DepthTask.h \
           src/controller/task/GateTask.h \
           src/controller/task/MotorTask.h \
           src/controller/task/PathTask.h \
           src/controller/task/PowerTask.h \
           src/controller/task/QuickTaskAdder.h \
           src/controller/task/SpeedTask.h \
           src/controller/task/Task.h \
           src/controller/task/TaskFactory.h \
           src/controller/task/TurnTask.h \
           src/model/interface/CameraInterface.h \
           src/model/interface/fpga_ui.h \
           src/model/interface/FPGAInterface.h \
           src/model/interface/HwInterface.h \
           src/model/interface/scripts.h \
           src/model/state/CameraState.h \
           src/model/state/FPGAState.h \
           src/model/state/State.h \
           src/model/state/StateTester.h \
           src/util/data/Data.h \
           src/util/data/DataFactory.h \
           src/util/data/FPGAData.h \
           src/util/data/ImgData.h \
           src/util/filter/BlurFilter.h \
           src/util/filter/Filter.h \
           src/util/filter/FilterFactory.h \
           src/util/filter/FilterManager.h \
           src/util/filter/HSVFilter.h \
           src/util/filter/LineFilter.h \
           src/util/filter/NullFPGAFilter.h \
           src/util/filter/NullImgFilter.h \
           src/util/filter/RGBFilter.h \
           src/util/filter/ShapeFilter.h \
           src/util/filter/TemplateFilter.h \
           test/util/data/DataTEST.h \
           test/util/data/FPGADataTEST.h \
           test/util/data/ImgDataTEST.h \
           test/util/filter/FilterManagerTEST.h \
           test/util/filter/RGBFilterTEST.h \
           src/util/Properties.h \
    src/view/Stage.h \
    src/view/MenuView.h \
    src/view/GUIView.h \
    src/view/SimulatorView.h \
    src/model/interface/SimFPGAInterface.h \
    src/controller/task/PortalTask.h \
    src/model/interface/SimCameraInterface.h \
    src/controller/task/CompetitionTask.h \
    src/view/CompetitionView.h \
    src/util/VideoLogger.h \
    src/Sub.h \
    src/SubFactory.h \
    Simulator/DataStorage.h \
    Simulator/Sim.h \
    Simulator/SimLogger.h \
    Simulator/Objects/Buoy.h \
    Simulator/Objects/SimObject.h \
    Simulator/Objects/SimSub.h \
    Simulator/SimCam.h \
    Simulator/SimFPGA.h

SOURCES += src/Main.cpp \
           test/CollectionTEST.cpp \
           test/VideoTesting.cpp \
           src/model/CameraModel.cpp \
           src/model/FPGAModel.cpp \
           src/model/Model.cpp \
           src/util/IDHasher.cpp \
           src/util/Logger.cpp \
           src/util/PropertyReader.cpp \
           src/util/Timer.cpp \
           src/util/Util.cpp \
           src/view/View.cpp \
           test/util/IDHasherTEST.cpp \
           src/controller/controllers/Controller.cpp \
           src/controller/controllers/ControllerThread.cpp \
           src/controller/task/BuoyTask.cpp \
           src/controller/task/DepthTask.cpp \
           src/controller/task/GateTask.cpp \
           src/controller/task/MotorTask.cpp \
           src/controller/task/PathTask.cpp \
           src/controller/task/PowerTask.cpp \
           src/controller/task/QuickTaskAdder.cpp \
           src/controller/task/SpeedTask.cpp \
           src/controller/task/Task.cpp \
           src/controller/task/TaskFactory.cpp \
           src/controller/task/TurnTask.cpp \
           src/model/interface/CameraInterface.cpp \
           src/model/interface/fpga_ui.c \
           src/model/interface/FPGAInterface.cpp \
           src/model/interface/HwInterface.cpp \
           src/model/state/CameraState.cpp \
           src/model/state/FPGAState.cpp \
           src/model/state/State.cpp \
           src/model/state/StateTester.cpp \
           src/util/data/Data.cpp \
           src/util/data/DataFactory.cpp \
           src/util/data/FPGAData.cpp \
           src/util/data/ImgData.cpp \
           src/util/filter/BlurFilter.cpp \
           src/util/filter/Filter.cpp \
           src/util/filter/FilterFactory.cpp \
           src/util/filter/FilterManager.cpp \
           src/util/filter/HSVFilter.cpp \
           src/util/filter/LineFilter.cpp \
           src/util/filter/NullFPGAFilter.cpp \
           src/util/filter/NullImgFilter.cpp \
           src/util/filter/RGBFilter.cpp \
           src/util/filter/ShapeFilter.cpp \
           src/util/filter/TemplateFilter.cpp \
           test/util/data/DataTEST.cpp \
           test/util/data/FPGADataTEST.cpp \
           test/util/data/ImgDataTEST.cpp \
           test/util/filter/FilterManagerTEST.cpp \
           test/util/filter/RGBFilterTEST.cpp \
           src/util/Properties.cpp \
    src/view/Stage.cpp \
    src/view/MenuView.cpp \
    src/view/GUIView.cpp \
    src/view/SimulatorView.cpp \
    src/model/interface/SimFPGAInterface.cpp \
    src/controller/task/PortalTask.cpp \
    src/model/interface/SimCameraInterface.cpp \
    src/controller/task/CompetitionTask.cpp \
    src/view/CompetitionView.cpp \
    src/util/VideoLogger.cpp \
    src/Sub.cpp \
    src/SubFactory.cpp \
    Simulator/DataStorage.cpp \
    Simulator/Sim.cpp \
    Simulator/SimLogger.cpp \
    Simulator/Objects/Buoy.cpp \
    Simulator/Objects/SimObject.cpp \
    Simulator/Objects/SimSub.cpp \
    Simulator/SimCam.cpp \
    Simulator/SimFPGA.cpp

RESOURCES += \
    src/resources/resources.qrc

DISTFILES += \
    src/settings/portal_task_settings.txt \
    src/settings/buoy_task_settings.txt \
    src/settings/path_task_settings.txt \
    src/settings/gate_task_settings.txt \
    src/settings/competition_settings.txt
