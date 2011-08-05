sp             := $(sp).x
dirstack_$(sp) := $(d)
d              := $(dir)


dir:=$(d)/$(SOUND)
include $(TOPDIR)/$(dir)/module.mk


FILES:=AmbientAreaManager.cpp AmplitudeArray.cpp LipsyncManager.cpp \
       LipsyncProperties.cpp MusicPlaylist.cpp playlistdefs.cpp \
	   SoundLooper.cpp SoundMixer.cpp WaveReader.cpp


SRC_$(d):=$(addprefix $(d)/,$(FILES)) $(SRC_$(d)/$(SOUND))

CLEANDIRS+=$(d)

-include $(foreach FILE,$(FILES),$(d)/$(FILE:.cpp=.d))

d  := $(dirstack_$(sp))
sp := $(basename $(sp))
