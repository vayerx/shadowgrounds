sp             := $(sp).x
dirstack_$(sp) := $(d)
d              := $(dir)

dir:=$(d)/camera_system
include $(TOPDIR)/$(dir)/module.mk

FILES:=AmbientSound.cpp AmbientSoundManager.cpp AmmoWindowCoop.cpp \
	   AmmoWindow.cpp AnimationSet.cpp Animator.cpp AniRecorderWindow.cpp \
	   ArmorConstructWindow.cpp ArmorPartSelectWindow.cpp BlackEdgeWindow.cpp \
	   CameraAutotilter.cpp CameraAutozoomer.cpp CinematicScreen.cpp \
	   CombatMessageWindow.cpp CombatMessageWindowWithHistory.cpp \
	   CombatRadar.cpp CombatSubWindowFactory.cpp CombatUnitWindow.cpp \
	   CombatWindow.cpp ComboWindow.cpp CreditsMenu.cpp \
	   cursordefs.cpp DebugDataView.cpp DebugProjectileVisualizer.cpp \
	   DebugTrackerVisualizer.cpp DebugUnitVisualizer.cpp \
	   DebugVisualizerTextUtil.cpp DecalPositionCalculator.cpp Decoration.cpp \
	   DecorationManager.cpp DynamicLightManager.cpp Ejecter.cpp \
	   ElaborateHintMessageWindow.cpp ErrorWindow.cpp FlashlightWindow.cpp \
	   GameCamera.cpp GameConsole.cpp GameController.cpp GamePointers.cpp \
	   GameVideoPlayer.cpp GenericBarWindow.cpp GUIEffectWindow.cpp \
	   HealthWindowCoop.cpp HealthWindow.cpp IngameGuiTabs.cpp ItemWindow.cpp \
	   ItemWindowUpdator.cpp JoystickAimer.cpp LightManager.cpp \
	   LoadGameMenu.cpp LoadingMessage.cpp LoadingWindow.cpp LogEntry.cpp \
	   LogManager.cpp Logwindow.cpp Mainmenu.cpp Map.cpp MapWindow.cpp \
	   MenuBaseImpl.cpp MenuCollection.cpp MessageBoxWindow.cpp \
	   MissionSelectionWindow.cpp MovieAspectWindow.cpp Muzzleflasher.cpp \
	   NewGameMenu.cpp NoCameraBoundary.cpp OffscreenUnitPointers.cpp \
	   OptionsMenu.cpp OptionsWindow.cpp ParticleArea.cpp \
	   ParticleCollision.cpp PlayerUnitCameraBoundary.cpp ProfilesMenu.cpp \
	   ScoreWindow.cpp SelectionBox.cpp Spotlight.cpp StorageWindow.cpp \
	   SurvivorUiElements.cpp SurvivorUpgradeWindow.cpp \
	   TargetDisplayButtonManager.cpp \
	   TargetDisplayWindowButton.cpp TargetDisplayWindow.cpp \
	   TargetDisplayWindowUpdator.cpp TerminalManager.cpp \
	   TerminalWindow.cpp Terrain.cpp TerrainCreator.cpp terrain_legacy.cpp \
	   uidefaults.cpp UIEffects.cpp UnitHealthBarWindow.cpp UnitHighlight.cpp \
	   UpgradeAvailableWindow.cpp UpgradeWindow.cpp Visual2D.cpp \
	   VisualEffect.cpp VisualEffectManager.cpp VisualEffectType.cpp \
	   VisualObject.cpp VisualObjectModel.cpp WeaponSelectionWindow.cpp \
	   WeaponWindowCoop.cpp WeaponWindow.cpp GenericTextWindow.cpp \
	   DebugMapVisualizer.cpp SelectionVisualizer.cpp MissionFailureWindow.cpp


SURVIVOR_FILES:=CoopMenu.cpp SurvivalMenu.cpp CharacterSelectionWindow.cpp \
                SurvivorLoadGameMenu.cpp VehicleWindow.cpp

SRC_$(d):=$(addprefix $(d)/,$(FILES)) $(SRC_$(d)/camera_system)
SRC_survivorui:=$(addprefix $(d)/,$(SURVIVOR_FILES))

CLEANDIRS+=$(d)

-include $(foreach FILE,$(FILES),$(d)/$(FILE:.cpp=.d))

d  := $(dirstack_$(sp))
sp := $(basename $(sp))
