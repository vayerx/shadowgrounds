sp             := $(sp).x
dirstack_$(sp) := $(d)
d              := $(dir)


FILES:=igios.cpp


SRC_$(d):=$(addprefix $(d)/,$(FILES))

CLEANDIRS+=$(d)

d  := $(dirstack_$(sp))
sp := $(basename $(sp))
