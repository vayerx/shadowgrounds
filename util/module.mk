sp             := $(sp).x
dirstack_$(sp) := $(d)
d              := $(dir)


FILES:=AI_PathFind.cpp AreaMap.cpp assert.cpp BuildingHandler.cpp \
       BuildingMap.cpp CircleAreaTracker.cpp ClippedCircle.cpp ColorMap.cpp \
	   CursorRayTracer.cpp Dampers.cpp DecalManager.cpp DecalSpawner.cpp \
	   DecalSystem.cpp DirectionalLight.cpp \
	   DistanceFloodfill.cpp FBCopyFile.cpp Floodfill.cpp FogApplier.cpp \
	   GridOcclusionCuller.cpp HelperPositionCalculator.cpp \
	   hiddencommand.cpp LightAmountManager.cpp LightMap.cpp \
	   LineAreaChecker.cpp LipsyncManager.cpp LocaleManager.cpp \
	   LocaleResource.cpp mod_selector.cpp ObjectDurabilityParser.cpp \
	   PathDeformer.cpp PathSimplifier.cpp Preprocessor.cpp \
	   procedural_applier.cpp \
	   procedural_properties.cpp ScreenCapturer.cpp Script.cpp \
	   ScriptManager.cpp ScriptProcess.cpp SelfIlluminationChanger.cpp \
	   SimpleParser.cpp SoundMaterialParser.cpp SpotLightCalculator.cpp \
	   StringUtil.cpp TextFileModifier.cpp TextFinder.cpp TextureCache.cpp \
	   TextureSwitcher.cpp UberCrypt.cpp UnicodeConverter.cpp \
	   Debug_MemoryManager.cpp CheckedIntValue.cpp jpak.cpp


SRC_$(d):=$(addprefix $(d)/,$(FILES))

CLEANDIRS+=$(d)

-include $(foreach FILE,$(FILES),$(d)/$(FILE:.cpp=.d))

d  := $(dirstack_$(sp))
sp := $(basename $(sp))
