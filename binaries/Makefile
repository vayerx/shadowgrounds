## DO NOT TOUCH THIS FILE
# copy example.mk to local.mk and change that


include example.mk
-include local.mk

DEFAULT_CC:=gcc
DEFAULT_CXX:=g++
DEFAULT_LD:=g++
DEFAULT_CXXFLAGS:=-DLINUX -DLINKEDLIST_USE_NODE_POOL -DLEGACY_FILES -DLIGHT_MAX_AMOUNT=5
DEFAULT_CXXFLAGS+=-g -Wall -Werror -Wno-non-virtual-dtor -Wno-write-strings -Wno-sign-compare
DEFAULT_CXXFLAGS+=$(shell sdl-config --cflags)
DEFAULT_CXXFLAGS+=-DNDEBUG
DEFAULT_CXXFLAGS+=-ffast-math -O -fno-strict-aliasing
DEFAULT_CXXFLAGS+=-I../ffmpeg/include
DEFAULT_LIBCXXFLAGS:=-fPIC

DEFAULT_DYNAMICLIB:=-shared

DEFAULT_LDFLAGS:=-g -Wl,-rpath,. -Wl,-rpath,/usr/local/lib
DEFAULT_LDLIBS:=-L../unzip -lunzip -lz
DEFAULT_LDLIBS+=$(shell sdl-config --libs) -lSDL_image -lSDL_ttf -lSDL_sound -lGLEW

DEFAULT_LIBFMOD:=-lfmod-3.75
DEFAULT_LIBopenal:=-lopenal -lvorbisfile

DEFAULT_IAVCODEC:=-I../ffmpeg/include
DEFAULT_LAVCODEC:=-L../ffmpeg/libs -lavcodec -lavformat -lavutil -ltheora

DEFAULT_IPHYSX:=-I/usr/include/PhysX/v2.8.1/SDKs/Physics/include -I/usr/include/PhysX/v2.8.1/SDKs/Cooking/include -I/usr/include/PhysX/v2.8.1/SDKs/NxExtensions/include -I/usr/include/PhysX/v2.8.1/SDKs/NxCharacter/include -I/usr/include/PhysX/v2.8.1/SDKs/Foundation/include -I/usr/include/PhysX/v2.8.1/SDKs/PhysXLoader/include
DEFAULT_LPHYSX:=-L/usr/lib/PhysX/v2.8.1 -lPhysXLoader -lNxCooking -lPhysXCore -lNxCharacter -Wl,-rpath,/usr/lib/PhysX/v2.8.1

EXESUFFIX:=-bin
LIBPREFIX:=lib
LIBSUFFIX:=.so

CC?=$(DEFAULT_CC)
CXX?=$(DEFAULT_CXX)
LD?=$(DEFAULT_LD)
CXXFLAGS?=$(DEFAULT_CXXFLAGS)
LIBCXXFLAGS?=$(DEFAULT_LIBCXXFLAGS)
LDFLAGS?=$(DEFAULT_LDFLAGS)
LDLIBS?=$(DEFAULT_LDLIBS)
DYNAMICLIB?=$(DEFAULT_DYNAMICLIB)
LIBFMOD?=$(DEFAULT_LIBFMOD)
LIBopenal?=$(DEFAULT_LIBopenal)
IAVCODEC?=$(DEFAULT_IAVCODEC)
LAVCODEC?=$(DEFAULT_LAVCODEC)
IPHYSX?=$(DEFAULT_IPHYSX)
LPHYSX?=$(DEFAULT_LPHYSX)

ifeq ($(PROF),y)
CXXFLAGS+=-pg # profile with gprof
LDFLAGS+=-pg # profile with gprof
endif

ifeq ($(SOUND),fmod)
LDLIBS+=$(LIBFMOD)
else


ifeq ($(SOUND),openal)
LDLIBS+=$(LIBopenal)
endif # openal

endif # fmod

ifeq ($(GL_LOCATE_ERRORS),y)
# storm3d only
LIBCXXFLAGS+=-finstrument-functions
endif

ifeq ($(NVPERFSDK),y)
# profiling with nvidia perfsdk (works somewhat...)
CXXFLAGS+=-DNVPERFSDK -I.
endif

ifeq ($(DEMOVERSION),y)
CXXFLAGS+=-DDEMOVERSION -DPROJECT_SURVIVOR_DEMO
endif

TOPDIR:=..
VPATH=$(TOPDIR)

include $(TOPDIR)/module.mk


