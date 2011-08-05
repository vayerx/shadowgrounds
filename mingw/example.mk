# example of local configuration
# copy to local.mk

# physx doesn't work with mingw, abi incompat.
PHYSX:=n
# profiling with gprof
PROF:=n
LIBAVCODEC:=n
FMOD:=y
GL_LOCATE_ERRORS:=n
NVPERFSDK:=n
RENDERERS:=d3d opengl
SYS:=windows


CC:=i586-mingw32msvc-gcc
CXX:=i586-mingw32msvc-g++
LD:=i586-mingw32msvc-g++
CXXFLAGS:=-DLINKEDLIST_USE_NODE_POOL -DLEGACY_FILES -DLIGHT_MAX_AMOUNT=5 -DBONE_MODEL_SPHERE_TRANSFORM
CXXFLAGS+=-g -Wall -Werror -Wno-write-strings
#CXXFLAGS+=-D_DEBUG # triggers assertion failures, not good
# for release builds
CXXFLAGS+=-DNDEBUG
CXXFLAGS+=-ffast-math -O
#CXXFLAGS+=-ffast-math -O2
#CXXFLAGS+=-march=pentium3 -msse -ftree-vectorize -ftree-vectorizer-verbose=5

LIBCXXFLAGS:=
SDLCXXFLAGS:=$(shell i386-mingw32msvc-sdl-config --cflags)
CXXFLAGS+=$(SDLCXXFLAGS)

LDFLAGS:=-g
#FIXME: remove winmm, use something in system (timer?)
LDLIBS:=-lwinmm -lunzip -lz

LIBOPENGL:=-lglew32 -lopengl32 -lglu32
LIBD3D:=-ld3d9 -ld3dx9_33
LIBSDL:=$(shell i386-mingw32msvc-sdl-config --libs) SDL_image.lib sdl_sound.lib SDL_ttf.lib
LIBFMOD:=-lfmod

#FIXME: should be renderer-specific
LDLIBS+=$(LIBSDL)

IPHYSX:=
LPHYSX:=PhysXLoader.lib


