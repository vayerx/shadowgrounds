sp             := $(sp).x
dirstack_$(sp) := $(d)
d              := $(dir)


FILES:=c2_matrix.cpp c2_thread.cpp


SRC_$(d):=$(addprefix $(d)/,$(FILES))

CLEANDIRS+=$(d)

-include $(foreach FILE,$(FILES),$(d)/$(FILE:.cpp=.d))

d  := $(dirstack_$(sp))
sp := $(basename $(sp))
