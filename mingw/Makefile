## DO NOT TOUCH THIS FILE
# copy example.mk to local.mk and change that


-include local.mk

DEFAULT_CC:=i586-mingw32msvc-gcc
DEFAULT_CXX:=i586-mingw32msvc-g++
DEFAULT_LD:=i586-mingw32msvc-g++
DEFAULT_CXXFLAGS:=-DLINKEDLIST_USE_NODE_POOL -DLEGACY_FILES -DLIGHT_MAX_AMOUNT=5 -DBONE_MODEL_SPHERE_TRANSFORM
DEFAULT_CXXFLAGS+=-g -Wall -Werror -Wno-write-strings -Wno-non-virtual-dtor -Wno-sign-compare
DEFAULT_CXXFLAGS+=$(shell i386-mingw32msvc-sdl-config --cflags)
DEFAULT_CXXFLAGS+=-DNDEBUG
DEFAULT_CXXFLAGS+=-ffast-math -O -fno-strict-aliasing
DEFAULT_LIBCXXFLAGS:=

DEFAULT_DYNAMICLIB:=-shared

DEFAULT_LDFLAGS:=-g
DEFAULT_LDLIBS:=-lunzip -lz libtheora.a libogg.a
DEFAULT_LDLIBS+=$(shell i386-mingw32msvc-sdl-config --libs) SDL_image.lib -lglew32 -lopengl32 -lglu32 sdl_sound.lib SDL_ttf.lib

DEFAULT_LIBFMOD:=-lfmod

DEFAULT_IPHYSX:=
DEFAULT_LPHYSX:=PhysXLoader.lib

EXESUFFIX:=.exe
LIBPREFIX:=
LIBSUFFIX:=.dll

CC?=$(DEFAULT_CC)
CXX?=$(DEFAULT_CXX)
LD?=$(DEFAULT_LD)
CXXFLAGS?=$(DEFAULT_CXXFLAGS)
LIBCXXFLAGS?=$(DEFAULT_LIBCXXFLAGS)
LDFLAGS?=$(DEFAULT_LDFLAGS)
LDLIBS?=$(DEFAULT_LDLIBS)
DYNAMICLIB?=$(DEFAULT_DYNAMICLIB)
LIBFMOD?=$(DEFAULT_LIBFMOD)
IPHYSX?=$(DEFAULT_IPHYSX)
LPHYSX?=$(DEFAULT_LPHYSX)

ifeq ($(PROF),y)
CXXFLAGS+=-pg # profile with gprof
LDFLAGS+=-pg # profile with gprof
endif

ifeq ($(FMOD),n)
CXXFLAGS+=-DSOUND_NULL
else
LDLIBS+=$(LIBFMOD)
endif

ifeq ($(GL_LOCATE_ERRORS),y)
# storm3d only
LIBCXXFLAGS+=-finstrument-functions
endif

ifeq ($(NVPERFSDK),y)
# profiling with nvidia perfsdk (works somewhat...)
CXXFLAGS+=-DNVPERFSDK -I.
LDLIBS+=NVPerfSDK.lib
endif

TOPDIR:=..
VPATH=$(TOPDIR)

include $(TOPDIR)/module.mk


