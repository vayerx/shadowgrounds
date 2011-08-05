sp             := $(sp).x
dirstack_$(sp) := $(d)
d              := $(dir)


FILES:=str2int.cpp

SRC_$(d):=$(addprefix $(d)/,$(FILES))

CLEANDIRS+=$(d)

-include $(foreach FILE,$(FILES),$(d)/$(FILE:.cpp=.d))

d  := $(dirstack_$(sp))
sp := $(basename $(sp))
