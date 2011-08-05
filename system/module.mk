sp             := $(sp).x
dirstack_$(sp) := $(d)
d              := $(dir)


dir:=$(d)/sdl
include $(TOPDIR)/$(dir)/module.mk
dir:=$(d)/windows
include $(TOPDIR)/$(dir)/module.mk


FILES:=FileTimestampChecker.cpp Logger.cpp SystemRandom.cpp SystemTime.cpp \
       Timer.cpp Miscellaneous.cpp \
	   $(SRC_system/$(SYS))


SRC_$(d):=$(addprefix $(d)/,$(FILES))

CLEANDIRS+=$(d)

-include $(foreach FILE,$(FILES),$(d)/$(FILE:.cpp=.d))

d  := $(dirstack_$(sp))
sp := $(basename $(sp))
