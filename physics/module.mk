sp             := $(sp).x
dirstack_$(sp) := $(d)
d              := $(dir)


FILES:=actor_base.cpp box_actor.cpp capsule_actor.cpp car_actor.cpp \
       convex_actor.cpp cooker.cpp file_stream.cpp fluid.cpp \
	   heightmap_actor.cpp joint_base.cpp physics_lib.cpp rack_actor.cpp \
	   spherical_joint.cpp static_mesh_actor.cpp visualizer.cpp


SRC_$(d):=$(addprefix $(d)/,$(FILES))

CLEANDIRS+=$(d)

-include $(foreach FILE,$(FILES),$(d)/$(FILE:.cpp=.d))

d  := $(dirstack_$(sp))
sp := $(basename $(sp))
