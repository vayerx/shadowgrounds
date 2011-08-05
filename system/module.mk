sp             := $(sp).x
dirstack_$(sp) := $(d)
d              := $(dir)


FILES:=FileTimestampChecker.cpp Logger.cpp SystemRandom.cpp SystemTime.cpp \
	   Timer.cpp \
	   Miscellaneous.cpp \
	   # empty line



SRC_$(d):=$(addprefix $(d)/,$(FILES))

CLEANDIRS+=$(d)

-include $(foreach FILE,$(FILES),$(d)/$(FILE:.cpp=.d))

d  := $(dirstack_$(sp))
sp := $(basename $(sp))
