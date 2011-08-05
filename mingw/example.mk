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


CC:=i586-mingw32msvc-gcc
CXX:=i586-mingw32msvc-g++
LD:=i586-mingw32msvc-g++
CXXFLAGS:=-DLINKEDLIST_USE_NODE_POOL -DLEGACY_FILES -DLIGHT_MAX_AMOUNT=5 -DBONE_MODEL_SPHERE_TRANSFORM
CXXFLAGS+=-g -Wall -Werror -Wno-write-strings
CXXFLAGS+=$(shell i386-mingw32msvc-sdl-config --cflags)
#CXXFLAGS+=-D_DEBUG # triggers assertion failures, not good
# for release builds
CXXFLAGS+=-DNDEBUG
CXXFLAGS+=-ffast-math -O
#CXXFLAGS+=-ffast-math -O2
#CXXFLAGS+=-march=pentium3 -msse -ftree-vectorize -ftree-vectorizer-verbose=5
LIBCXXFLAGS:=

LDFLAGS:=-g
LDLIBS:=-lunzip -lz libtheora.a libogg.a
LDLIBS+=$(shell i386-mingw32msvc-sdl-config --libs) SDL_image.lib -lglew32 -lopengl32 -lglu32 sdl_sound.lib SDL_ttf.lib

LIBFMOD:=-lfmod

IPHYSX:=
LPHYSX:=PhysXLoader.lib


