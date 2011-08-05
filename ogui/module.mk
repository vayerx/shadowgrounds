sp             := $(sp).x
dirstack_$(sp) := $(d)
d              := $(dir)


FILES:=Ogui.cpp OguiButton.cpp OguiButtonEvent.cpp OguiButtonStyle.cpp \
       OguiCheckBox.cpp OguiEffectEvent.cpp OguiException.cpp \
	   OguiFormattedCommandImg.cpp OguiFormattedCommandImpl.cpp \
	   OguiFormattedText.cpp OguiLocaleWrapper.cpp OguiSelectList.cpp \
	   OguiSelectListEvent.cpp OguiSelectListStyle.cpp \
	   OguiSlider.cpp OguiStormDriver.cpp OguiTextLabel.cpp \
	   OguiTypeEffectListener.cpp OguiWindow.cpp orvgui2.cpp OguiAligner.cpp

SRC_$(d):=$(addprefix $(d)/,$(FILES))

CLEANDIRS+=$(d)

-include $(foreach FILE, $(FILES), $(d)/$(FILE:.cpp=.d))

d  := $(dirstack_$(sp))
sp := $(basename $(sp))
