include ../Build/Common/CommonDefs.mak

BIN_DIR = ../Bin

INC_DIRS = ../../Include =/usr/include/ni

SRC_FILES = ./*.cpp

EXE_NAME = BallDetection

ifeq ("$(OSTYPE)","Darwin")
	LDFLAGS += -framework OpenGL -framework GLUT 
else
	USED_LIBS += glut GL 
endif

USED_LIBS += OpenNI

LIB_DIRS += ../../Lib
include ../Build/Common/CommonCppMakefile

#-lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lopencv_flann
