sp             := $(sp).x
dirstack_$(sp) := $(d)
d              := $(dir)


dir:=$(d)/physics
include $(TOPDIR)/$(dir)/module.mk
dir:=$(d)/scripting
include $(TOPDIR)/$(dir)/module.mk
dir:=$(d)/tracking
include $(TOPDIR)/$(dir)/module.mk


FILES:=userdata.cpp AlienSpawner.cpp AmmoPack.cpp AmmoPackObject.cpp Ani.cpp \
       AniManager.cpp AniRecorder.cpp AniTool.cpp Arm.cpp ArmorUnitActor.cpp \
	   ArmorUnit.cpp ArmorUnitType.cpp BuildingAdder.cpp Building.cpp \
	   BuildingList.cpp Bullet.cpp Character.cpp CheckpointChecker.cpp \
	   ConnectionChecker.cpp CoverFinder.cpp CoverMap.cpp createparts.cpp \
	   DHLocaleManager.cpp DifficultyManager.cpp DirectWeapon.cpp \
	   EnvironmentalEffect.cpp EnvironmentalEffectManager.cpp Flashlight.cpp \
	   FoobarAI.cpp GameConfigs.cpp Game.cpp GameMap.cpp GameObject.cpp \
	   GameObjectFactoryList.cpp GameObjectList.cpp GameOption.cpp \
	   GameOptionManager.cpp GameProfiles.cpp GameProfilesEnumeration.cpp \
	   GameRandom.cpp GameScene.cpp GameStats.cpp GameUI.cpp goretypedefs.cpp \
	   Head.cpp HideMap.cpp IndirectWeapon.cpp Item.cpp ItemList.cpp \
	   ItemManager.cpp ItemPack.cpp ItemType.cpp Leg.cpp LightBlinker.cpp \
	   LineOfJumpChecker.cpp MaterialManager.cpp materials.cpp \
	   MissionParser.cpp ObstacleMapUnitObstacle.cpp OptionApplier.cpp \
	   Part.cpp ParticleSpawner.cpp ParticleSpawnerManager.cpp PartList.cpp \
	   PartTypeAvailabilityList.cpp PartType.cpp PartTypeParser.cpp \
	   PlayerPartsManager.cpp PlayerWeaponry.cpp PowerCell.cpp \
	   ProgressBarActor.cpp ProgressBar.cpp ProjectileActor.cpp \
	   Projectile.cpp ProjectileList.cpp ProjectileTrackerObjectType.cpp \
	   Reactor.cpp ReconChecker.cpp SaveData.cpp savegamevars.cpp \
	   ScriptDebugger.cpp SidewaysUnitActor.cpp SidewaysUnit.cpp Tool.cpp \
	   Torso.cpp UnitList.cpp UnifiedHandleManager.cpp UnitActor.cpp Unit.cpp \
	   UnitFormation.cpp UnitInventory.cpp UnitLevelAI.cpp \
	   UnitPhysicsUpdater.cpp UnitScriptPaths.cpp UnitSelections.cpp \
	   UnitSpawner.cpp UnitTargeting.cpp UnitType.cpp unittypes.cpp \
	   UnitVariables.cpp UnitVisibilityChecker.cpp UnitVisibility.cpp \
	   UpgradeManager.cpp UpgradeType.cpp VisualObjectModelStorage.cpp \
	   Water.cpp WaterManager.cpp Weapon.cpp WeaponObject.cpp Forcewear.cpp \
	   SlopeTypes.cpp EngineMetaValues.cpp GameWorldFold.cpp \
	   ScriptableAIDirectControl.cpp direct_controls.cpp


SRC_$(d):=$(addprefix $(d)/,$(FILES)) $(SRC_$(d)/physics) $(SRC_$(d)/scripting) $(SRC_$(d)/tracking)

CLEANDIRS+=$(d)

-include $(foreach FILE,$(FILES),$(d)/$(FILE:.cpp=.d))

d  := $(dirstack_$(sp))
sp := $(basename $(sp))
