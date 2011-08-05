sp             := $(sp).x
dirstack_$(sp) := $(d)
d              := $(dir)


FILES:=Logger.cpp Timer.cpp


SRC_$(d):=$(addprefix $(SYS)/,$(FILES))

CLEANDIRS+=$(d)

-include $(foreach FILE,$(FILES),$(d)/$(FILE:.cpp=.d))

d  := $(dirstack_$(sp))
sp := $(basename $(sp))
