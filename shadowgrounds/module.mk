sp             := $(sp).x
dirstack_$(sp) := $(d)
d              := $(dir)


FILES:=shadowgrounds.cpp version.cpp

SRC_$(d):=$(addprefix $(d)/,$(FILES))

CLEANDIRS+=$(d)

-include $(foreach FILE,$(FILES),$(d)/$(FILE:.cpp=.d))

d  := $(dirstack_$(sp))
sp := $(basename $(sp))
