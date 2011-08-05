sp             := $(sp).x
dirstack_$(sp) := $(d)
d              := $(dir)


FILES:=AnimationScripting.cpp CameraScripting.cpp CinematicScripting.cpp \
       DecorScripting.cpp DevScripting.cpp EnvironmentScripting.cpp \
	   GameScriptingUtils.cpp HitChainScripting.cpp \
	   ItemScripting.cpp LightScripting.cpp MapScripting.cpp \
	   MathScripting.cpp MiscScripting.cpp MissionScripting.cpp \
	   OptionScripting.cpp PositionScripting.cpp SoundScripting.cpp \
	   StringScripting.cpp UnitScripting.cpp WaterScripting.cpp \
	   TrackingScripting.cpp SyncScripting.cpp DirectControlScripting.cpp \
	   GameScripting.cpp


SRC_$(d):=$(addprefix $(d)/,$(FILES))

CLEANDIRS+=$(d)

-include $(foreach FILE,$(FILES),$(d)/$(FILE:.cpp=.d))

d  := $(dirstack_$(sp))
sp := $(basename $(sp))
