## DO NOT TOUCH THIS FILE
# copy example.mk to local.mk and change that


-include local.mk

DEFAULT_CC:=i586-mingw32msvc-gcc
DEFAULT_CXX:=i586-mingw32msvc-g++
DEFAULT_LD:=i586-mingw32msvc-g++
DEFAULT_CXXFLAGS:=-DLINKEDLIST_USE_NODE_POOL -DLEGACY_FILES -DLIGHT_MAX_AMOUNT=5 -DBONE_MODEL_SPHERE_TRANSFORM
DEFAULT_CXXFLAGS+=-g -Wall -Werror -Wno-write-strings -Wno-non-virtual-dtor -Wno-sign-compare
DEFAULT_CXXFLAGS+=-DNDEBUG
DEFAULT_CXXFLAGS+=-ffast-math -O -fno-strict-aliasing
DEFAULT_LIBCXXFLAGS:=
DEFAULT_SDLCXXFLAGS:=$(shell i386-mingw32msvc-sdl-config --cflags)

DEFAULT_DYNAMICLIB:=-shared

DEFAULT_LDFLAGS:=-g
DEFAULT_LDLIBS:=-lunzip -lz
DEFAULT_LIBOPENGL:=-lglew32 -lopengl32 -lglu32
DEFAULT_LIBD3D:=-ld3d9 -ld3dx9_33
DEFAULT_LIBSDL:=$(shell i386-mingw32msvc-sdl-config --libs) SDL_image.lib sdl_sound.lib SDL_ttf.lib

DEFAULT_LIBFMOD:=-lfmod

DEFAULT_IPHYSX:=
DEFAULT_LPHYSX:=PhysXLoader.lib

DEFAULT_SYS:=windows

EXESUFFIX:=.exe
LIBPREFIX:=
LIBSUFFIX:=.dll

CC?=$(DEFAULT_CC)
CXX?=$(DEFAULT_CXX)
LD?=$(DEFAULT_LD)
CXXFLAGS?=$(DEFAULT_CXXFLAGS)
LIBCXXFLAGS?=$(DEFAULT_LIBCXXFLAGS)
SDLCXXFLAGS?=$(DEFAULT_SDLCXXFLAGS)

LDFLAGS?=$(DEFAULT_LDFLAGS)
LDLIBS?=$(DEFAULT_LDLIBS)
LIBOPENGL?=$(DEFAULT_LIBOPENGL)
LIBD3D?=$(DEFAULT_LIBD3D)
LIBSDL?=$(DEFAULT_LIBSDL)

DYNAMICLIB?=$(DEFAULT_DYNAMICLIB)
LIBFMOD?=$(DEFAULT_LIBFMOD)
IPHYSX?=$(DEFAULT_IPHYSX)
LPHYSX?=$(DEFAULT_LPHYSX)

SYS?=$(DEFAULT_SYS)

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
# FIXME: opengl renderer only
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


