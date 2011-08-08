# example of local configuration
# copy to local.mk

PHYSX:=y
# profiling with gprof
PROF:=n
LIBAVCODEC:=y
SOUND:=openal
GL_LOCATE_ERRORS:=n
NVPERFSDK:=n
DEMOVERSION:=n

CC:=gcc
CXX:=g++
LD:=g++
CXXFLAGS:=-DLINUX -DLINKEDLIST_USE_NODE_POOL -DLEGACY_FILES -DLIGHT_MAX_AMOUNT=5
CXXFLAGS+=-g -Wall -Werror -Wno-write-strings
CXXFLAGS+=$(shell sdl-config --cflags)
#CXXFLAGS+=-D_DEBUG # triggers assertion failures, not good
# for release builds
CXXFLAGS+=-DNDEBUG
# hide cursor and debugging output
#CXXFLAGS+=-DFINAL_RELEASE_BUILD
CXXFLAGS+=-fvisibility=hidden
CXXFLAGS+=-ffast-math -O
#CXXFLAGS+=-ffast-math -O2
#CXXFLAGS+=-march=pentium3 -msse -ftree-vectorize -ftree-vectorizer-verbose=5
CXXFLAGS+=$(shell pkg-config gtk+-2.0 --cflags)

DYNAMICLIB:=-shared

LDFLAGS:=-g -Wl,-rpath,\$$ORIGIN -Wl,-rpath,\$$ORIGIN/lib
LDLIBS:=-L. -lunzip -lz
LDLIBS+=$(shell sdl-config --libs) -lSDL_image -lSDL_ttf -lSDL_sound -lGLEW
LDLIBS+=$(shell pkg-config gtk+-2.0 --libs)

LIBFMOD:=-lfmod-3.75

IAVCODEC:=-I../ffmpeg/include
LAVCODEC:=-L../ffmpeg/libs -lavcodec -lavformat -lavutil

PHYSX_VERSION:=2.8.3
PHYSX_LIBDIR:=/usr/lib/PhysX
PHYSX_INCDIR:=/usr/include/PhysX

PHYSX_PARTS:=Physics Cooking NxExtensions NxCharacter Foundation PhysXLoader

PHYSX_INCLUDE_DIRS:=$(foreach dir,$(PHYSX_PARTS), $(PHYSX_INCDIR)/v$(PHYSX_VERSION)/SDKs/$(dir)/include)
LPHYSX:=-L$(PHYSX_LIBDIR)/v$(PHYSX_VERSION) -lPhysXLoader -ldl
#LPHYSX+=-lPhysXCore -lNxCooking

# physx fluids on linux are broken
# fuck ageia and nvidia
# only on old version of PhysX
ifeq ($(PHYSX_VERSION),2.8.1)

CXXFLAGS+=-DNX_DISABLE_FLUIDS

endif

IPHYSX:=$(foreach dir,$(PHYSX_INCLUDE_DIRS),-I$(dir))


