sp             := $(sp).x
dirstack_$(sp) := $(d)
d              := $(dir)

FILES_particlesystem:=cloudparticlesystem.cpp floattrack.cpp \
                      modelparticlesystem.cpp parseutil.cpp \
					  particleeffect.cpp particleforces.cpp \
					  particlesystem.cpp \
					  particlesystemmanager.cpp pointarrayparticlesystem.cpp \
					  sprayparticlesystem.cpp vectortrack.cpp

ifeq ($(PHYSX),y)
FILES_particlesystem+= ParticlePhysics.cpp
endif

SRC_particlesystem:=$(addprefix $(d)/,$(FILES_particlesystem))

CLEANDIRS+=$(d)

-include $(foreach FILE,$(FILES_particlesystem),$(d)/$(FILE:.cpp=.d))

d  := $(dirstack_$(sp))
sp := $(basename $(sp))
