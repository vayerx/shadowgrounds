sp             := $(sp).x
dirstack_$(sp) := $(d)
d              := $(dir)


FILES:=AbstractPhysicsObject.cpp BoxPhysicsObject.cpp CapsulePhysicsObject.cpp \
       ConvexPhysicsObject.cpp gamephysics.cpp PhysicsContactDamageManager.cpp \
	   PhysicsContactEffectManager.cpp PhysicsContactSoundManager.cpp \
	   PhysicsContactUtils.cpp RackPhysicsObject.cpp StaticPhysicsObject.cpp

ifeq ($(PHYSX), y)
FILES+=physics_none.cpp

endif

SRC_$(d):=$(addprefix $(d)/,$(FILES))

CLEANDIRS+=$(d)

-include $(foreach FILE,$(FILES),$(d)/$(FILE:.cpp=.d))

d  := $(dirstack_$(sp))
sp := $(basename $(sp))
