sp             := $(sp).x
dirstack_$(sp) := $(d)
d              := $(dir)


FILES:=AnyBurnableTrackableObjectFactory.cpp ObjectTracker.cpp \
       ScriptableTrackerObject.cpp ScriptableTrackerObjectType.cpp \
	   trackable_types.cpp TrackableUnifiedHandleObject.cpp


SRC_$(d):=$(addprefix $(d)/,$(FILES))

CLEANDIRS+=$(d)

-include $(foreach FILE,$(FILES),$(d)/$(FILE:.cpp=.d))

d  := $(dirstack_$(sp))
sp := $(basename $(sp))
