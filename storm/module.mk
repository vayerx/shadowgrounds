sp             := $(sp).x
dirstack_$(sp) := $(d)
d              := $(dir)


dir:=$(d)/common2
include $(TOPDIR)/$(dir)/module.mk
dir:=$(d)/keyb3
include $(TOPDIR)/$(dir)/module.mk
dir:=$(d)/storm3dv2
include $(TOPDIR)/$(dir)/module.mk


SRC_$(d):=$(SRC_$(d)/storm3dv2)


d  := $(dirstack_$(sp))
sp := $(basename $(sp))
