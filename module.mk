.PHONY: default all clean combine partial

TARGETS:=survivor$(EXESUFFIX) shadowgrounds$(EXESUFFIX)

default: all

all: $(TARGETS)

combine: survivor-combine$(EXESUFFIX) shadowgrounds-combine$(EXESUFFIX)

partial: survivor-partial.o

CXXFLAGS+=-I$(TOPDIR)/storm/include -I$(TOPDIR)/storm/keyb3 -I$(TOPDIR)

# rules here
%.sg.o: %.cpp
	$(CXX) -c -DHACKY_SG_AMBIENT_LIGHT_FIX -include $(TOPDIR)/shadowgrounds/configuration.h -DPROJECT_SHADOWGROUNDS -I$(TOPDIR)/shadowgrounds -MT$@ -MF $*.sg.d -MMD $(CXXFLAGS) $(INCLUDES) -o $@ $<

%.sv.o: %.cpp
	$(CXX) -c -include $(TOPDIR)/survivor/configuration.h -DPROJECT_SURVIVOR -I$(TOPDIR)/survivor -MT$@ -MF $*.sv.d -MMD $(CXXFLAGS) $(INCLUDES) -o $@ $<

CLEANDIRS:=


#unfortunately this doesn't work
#dirs:=container convert filesystem game ogui physics physics_ode shadowgrounds sound storm system ui util
#$(foreach dir, $(dirs), include $(dir)/module.mk)

dir:=chat
include $(TOPDIR)/$(dir)/module.mk
dir:=container
include $(TOPDIR)/$(dir)/module.mk
dir:=convert
include $(TOPDIR)/$(dir)/module.mk
dir:=editor
include $(TOPDIR)/$(dir)/module.mk
dir:=filesystem
include $(TOPDIR)/$(dir)/module.mk
dir:=game
include $(TOPDIR)/$(dir)/module.mk
dir:=ogui
include $(TOPDIR)/$(dir)/module.mk
dir:=particle_editor2
include $(TOPDIR)/$(dir)/module.mk
dir:=physics
include $(TOPDIR)/$(dir)/module.mk
dir:=ui
include $(TOPDIR)/$(dir)/module.mk
dir:=util
include $(TOPDIR)/$(dir)/module.mk
dir:=shadowgrounds
include $(TOPDIR)/$(dir)/module.mk
dir:=sound
include $(TOPDIR)/$(dir)/module.mk
dir:=survivor
include $(TOPDIR)/$(dir)/module.mk
dir:=system
include $(TOPDIR)/$(dir)/module.mk
# storm should be last
dir:=storm
include $(TOPDIR)/$(dir)/module.mk



clean:
	rm -f $(TARGETS) $(foreach dir,$(CLEANDIRS),$(dir)/*.o) $(foreach dir,$(CLEANDIRS),$(dir)/*.d) *-combine.cpp


shadowgrounds_modules:=chat container convert editor filesystem game ogui \
                       particlesystem shadowgrounds sound \
                       system ui util storm


survivor_modules:=chat container convert editor filesystem game ogui \
                  particlesystem survivor sound \
                  system ui util storm


ifeq ($(PHYSX),y)
CXXFLAGS+=-DPHYSICS_PHYSX
CXXFLAGS+=$(IPHYSX)
LDLIBS+=$(LPHYSX)

shadowgrounds_modules+=physics
survivor_modules+=physics

else

CXXFLAGS+=-DPHYSICS_NONE
endif

ifeq ($(LIBAVCODEC),y)
CXXFLAGS+=$(IAVCODEC) -DUSE_LIBAVCODEC
LDLIBS:=$(LAVCODEC) $(LDLIBS)
endif


shadowgrounds_SRC:=$(foreach mod,$(shadowgrounds_modules),$(SRC_$(mod))) storm/keyb3/keyb.cpp ui/TacticalUnitWindow.cpp
shadowgrounds_OBJS:=$(shadowgrounds_SRC:.cpp=.sg.o)


survivor_SRC:=$(foreach mod,$(survivor_modules),$(SRC_$(mod))) storm/keyb3/keyb.cpp $(SRC_survivorui) game/BonusManager.cpp
survivor_OBJS:=$(survivor_SRC:.cpp=.sv.o)


-include $(shadowgrounds_SRC:.cpp=.sg.d)
-include $(survivor_SRC:.cpp=.sv.d)


# must be compiled unoptimized
# gcc bug or code relying on undefined behavior?
util/LipsyncManager.sv.o: CXXFLAGS+=-O0
util/LipsyncManager.sg.o: CXXFLAGS+=-O0

survivor-combine.cpp: $(foreach mod,$(survivor_modules),$(SRC_$(mod))) $(SRC_survivorui) game/BonusManager.cpp storm/keyb3/keyb.cpp
	@echo Generating $@
	@../gencombine.sh $@ $^

survivor-combine$(EXESUFFIX): survivor-combine.cpp
	$(CXX) -fwhole-program -DCOMBINE -include $(TOPDIR)/survivor/configuration.h -DPROJECT_SURVIVOR -I$(TOPDIR)/survivor $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS) $(LDLIBS)

shadowgrounds-combine.cpp: $(foreach mod,$(shadowgrounds_modules),$(SRC_$(mod))) storm/keyb3/keyb.cpp ui/TacticalUnitWindow.cpp
	@echo Generating $@
	@../gencombine.sh $@ $^

shadowgrounds-combine$(EXESUFFIX): shadowgrounds-combine.cpp
	$(CXX) -DHACKY_SG_AMBIENT_LIGHT_FIX -fwhole-program -DCOMBINE -include $(TOPDIR)/shadowgrounds/configuration.h -DPROJECT_SHADOWGROUNDS -I$(TOPDIR)/shadowgrounds $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS) $(LDLIBS)



shadowgrounds$(EXESUFFIX): $(shadowgrounds_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

survivor$(EXESUFFIX): $(survivor_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)


survivor-partial.o: $(survivor_OBJS)
	ld -Ur -o $@ $^

