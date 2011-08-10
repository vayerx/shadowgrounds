# TODO split CMakeLists.txt

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(shadowgrounds)

SET( CMAKE_VERBOSE_MAKEFILE  OFF )

SET( Boost_USE_STATIC_LIBS   OFF )
SET( Boost_USE_MULTITHREADED ON )
FIND_PACKAGE( Boost 1.42 REQUIRED
    program_options 
    thread
)
FIND_PACKAGE( SDL REQUIRED )
FIND_PACKAGE( SDL_sound REQUIRED )
FIND_PACKAGE( SDL_image REQUIRED )
FIND_PACKAGE( SDL_ttf REQUIRED )
FIND_PACKAGE( OpenGL REQUIRED )
FIND_PACKAGE( GTK2 REQUIRED )
FIND_PACKAGE( ZLIB REQUIRED )
FIND_PACKAGE( OpenAL REQUIRED )

OPTION( ENABLE_PHYSX "Enable NVIDIA PhysX SDK" OFF )

SET( GLOBAL_INCLUDE_DIRS
    ${Boost_INCLUDE_DIRS}
    ${SDL_INCLUDE_DIR}
    ${SDL_SOUND_INCLUDE_DIR}
    ${SDLIMAGE_INCLUDE_DIR}
    ${SDLTTF_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${GTK2_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
    ${OPENAL_INCLUDE_DIR}
)

SET( GLOBAL_LIBRARIES
    ${Boost_LIBRARIES}
    ${GTK2_LIBRARIES}
    ${SDL_LIBRARY}
    ${SDL_SOUND_LIBRARY}
    ${SDLIMAGE_LIBRARY}
    ${SDLTTF_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${OPENAL_LIBRARY}
)

INCLUDE_DIRECTORIES( BEFORE SYSTEM
    ${GLOBAL_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}
    storm/include
    storm/keyb3
)

# List of all libraries
SET ( SG_MODULES container convert editor filesystem game ogui sound system ui util storm particlesystem )


OPTION( ENABLE_PHYSX "Enable NVIDIA PhysX SDK" OFF )
IF ( ENABLE_PHYSX )
    SET ( SG_MODULES physics ${SG_MODULES} )
    ADD_DEFINITIONS( -DPHYSICS_PHYSX )
ELSE ( ENABLE_PHYSX )
    ADD_DEFINITIONS( -DPHYSICS_NONE )
ENDIF ( ENABLE_PHYSX )


# Build source list from module name and path prefix
MACRO( CompileSrcList module prefix )
    FOREACH( item ${ARGN} )
        SET( ${module}_sources ${${module}_sources} ${prefix}/${item} )
    ENDFOREACH( item )
ENDMACRO( CompileSrcList )


CompileSrcList( convert convert
    str2int.cpp
)

CompileSrcList( container container
    LinkedList.cpp Stack.cpp
)

CompileSrcList( editor editor
    file_wrapper.cpp parser.cpp
)

CompileSrcList( filesystem filesystem
    file_list.cpp file_package_manager.cpp input_file_stream.cpp input_stream.cpp input_stream_wrapper.cpp
    memory_stream.cpp output_file_stream.cpp output_stream.cpp standard_package.cpp
    rle_packed_file_wrapper.cpp zip_package.cpp
    detail/unzip.c detail/ioapi.c
)

CompileSrcList( game game
    AlienSpawner.cpp AmmoPack.cpp AmmoPackObject.cpp Ani.cpp AniManager.cpp AniRecorder.cpp AniTool.cpp Arm.cpp
    ArmorUnitActor.cpp ArmorUnit.cpp ArmorUnitType.cpp BuildingAdder.cpp Building.cpp BuildingList.cpp Bullet.cpp
    Character.cpp CheckpointChecker.cpp ConnectionChecker.cpp CoverFinder.cpp CoverMap.cpp createparts.cpp
    DHLocaleManager.cpp DifficultyManager.cpp direct_controls.cpp DirectWeapon.cpp EngineMetaValues.cpp
    EnvironmentalEffect.cpp EnvironmentalEffectManager.cpp Flashlight.cpp FoobarAI.cpp Forcewear.cpp GameConfigs.cpp
    Game.cpp GameMap.cpp GameObject.cpp GameObjectFactoryList.cpp GameObjectList.cpp GameOption.cpp
    GameOptionManager.cpp GameProfiles.cpp GameProfilesEnumeration.cpp GameRandom.cpp GameScene.cpp GameStats.cpp
    GameUI.cpp GameWorldFold.cpp goretypedefs.cpp Head.cpp HideMap.cpp IndirectWeapon.cpp Item.cpp ItemList.cpp
    ItemManager.cpp ItemPack.cpp ItemType.cpp Leg.cpp LightBlinker.cpp LineOfJumpChecker.cpp MaterialManager.cpp
    materials.cpp MissionParser.cpp ObstacleMapUnitObstacle.cpp OptionApplier.cpp Part.cpp ParticleSpawner.cpp
    ParticleSpawnerManager.cpp PartList.cpp PartTypeAvailabilityList.cpp PartType.cpp PartTypeParser.cpp
    PlayerPartsManager.cpp PlayerWeaponry.cpp PowerCell.cpp ProgressBarActor.cpp ProgressBar.cpp ProjectileActor.cpp
    Projectile.cpp ProjectileList.cpp ProjectileTrackerObjectType.cpp Reactor.cpp ReconChecker.cpp SaveData.cpp
    savegamevars.cpp ScriptableAIDirectControl.cpp ScriptDebugger.cpp SidewaysUnitActor.cpp SidewaysUnit.cpp
    SlopeTypes.cpp Tool.cpp Torso.cpp UnifiedHandleManager.cpp UnitActor.cpp Unit.cpp UnitFormation.cpp
    UnitInventory.cpp UnitLevelAI.cpp UnitList.cpp UnitPhysicsUpdater.cpp UnitScriptPaths.cpp UnitSelections.cpp
    UnitSpawner.cpp UnitTargeting.cpp UnitType.cpp unittypes.cpp UnitVariables.cpp UnitVisibilityChecker.cpp
    UnitVisibility.cpp UpgradeManager.cpp UpgradeType.cpp userdata.cpp VisualObjectModelStorage.cpp Water.cpp
    WaterManager.cpp Weapon.cpp WeaponObject.cpp

    BonusManager.cpp    # TODO survivor-only
)

CompileSrcList( game game/physics
    AbstractPhysicsObject.cpp BoxPhysicsObject.cpp CapsulePhysicsObject.cpp ConvexPhysicsObject.cpp gamephysics.cpp
    PhysicsContactDamageManager.cpp PhysicsContactEffectManager.cpp PhysicsContactSoundManager.cpp
    PhysicsContactUtils.cpp RackPhysicsObject.cpp StaticPhysicsObject.cpp 
)
IF ( ENABLE_PHYSX )
    CompileSrcList( game game/physics physics_none.cpp )
ENDIF ( ENABLE_PHYSX )

CompileSrcList( game game/scripting
    AnimationScripting.cpp CameraScripting.cpp CinematicScripting.cpp DecorScripting.cpp DevScripting.cpp
    EnvironmentScripting.cpp GameScriptingUtils.cpp HitChainScripting.cpp ItemScripting.cpp LightScripting.cpp
    MapScripting.cpp MathScripting.cpp MissionScripting.cpp OptionScripting.cpp
    PositionScripting.cpp SoundScripting.cpp StringScripting.cpp UnitScripting.cpp WaterScripting.cpp
    TrackingScripting.cpp SyncScripting.cpp DirectControlScripting.cpp GameScripting.cpp
    MiscScripting.cpp
)

CompileSrcList( game game/tracking
    AnyBurnableTrackableObjectFactory.cpp ObjectTracker.cpp ScriptableTrackerObject.cpp ScriptableTrackerObjectType.cpp
    trackable_types.cpp TrackableUnifiedHandleObject.cpp
)

CompileSrcList( particlesystem particle_editor2
    cloudparticlesystem.cpp floattrack.cpp modelparticlesystem.cpp parseutil.cpp particleeffect.cpp
    particleforces.cpp particlesystem.cpp particlesystemmanager.cpp pointarrayparticlesystem.cpp
    sprayparticlesystem.cpp vectortrack.cpp
)
IF ( ENABLE_PHYSX )
    CompileSrcList( particlesystem particle_editor2 ParticlePhysics.cpp )
ENDIF ( ENABLE_PHYSX )


CompileSrcList( ogui ogui
    Ogui.cpp OguiButton.cpp OguiButtonEvent.cpp OguiButtonStyle.cpp OguiCheckBox.cpp OguiEffectEvent.cpp
    OguiException.cpp OguiFormattedCommandImg.cpp OguiFormattedCommandImpl.cpp OguiFormattedText.cpp
    OguiLocaleWrapper.cpp OguiSelectList.cpp OguiSelectListEvent.cpp OguiSelectListStyle.cpp OguiSlider.cpp
    OguiStormDriver.cpp OguiTextLabel.cpp OguiTypeEffectListener.cpp OguiWindow.cpp orvgui2.cpp OguiAligner.cpp
)

CompileSrcList( physics physics
    actor_base.cpp box_actor.cpp capsule_actor.cpp car_actor.cpp convex_actor.cpp cooker.cpp file_stream.cpp
    fluid.cpp heightmap_actor.cpp joint_base.cpp physics_lib.cpp rack_actor.cpp spherical_joint.cpp
    static_mesh_actor.cpp visualizer.cpp
)

CompileSrcList( sound sound
    AmbientAreaManager.cpp AmplitudeArray.cpp LipsyncManager.cpp LipsyncProperties.cpp MusicPlaylist.cpp
    playlistdefs.cpp SoundLooper.cpp SoundMixer.cpp WaveReader.cpp
    openal/SoundLib.cpp
)


CompileSrcList( storm storm
    common2/c2_matrix.cpp common2/c2_thread.cpp keyb3/keyb.cpp
)

CompileSrcList( storm storm/storm3dv2
    Clipper.cpp IStorm3D.cpp render.cpp Storm3D_Bone.cpp Storm3D_Camera.cpp Storm3D.cpp Storm3D_Face.cpp
    storm3d_fakespotlight.cpp Storm3D_Font.cpp Storm3D_Helper_AInterface.cpp Storm3D_Helper_Animation.cpp
    Storm3D_Helpers.cpp Storm3D_KeyFrames.cpp Storm3D_Line.cpp Storm3D_Material.cpp
    Storm3D_Material_TextureLayer.cpp Storm3D_Mesh_CollisionTable.cpp Storm3D_Mesh.cpp Storm3D_Model.cpp
    Storm3D_Model_Object.cpp Storm3D_ParticleSystem.cpp Storm3D_ParticleSystem_PMH.cpp Storm3D_ProceduralManager.cpp
    storm3d_resourcemanager.cpp Storm3D_Scene.cpp Storm3D_Scene_PicList.cpp Storm3D_Scene_PicList_Font.cpp
    Storm3D_Scene_PicList_Texture.cpp Storm3D_ShaderManager.cpp storm3d_spotlight.cpp storm3d_spotlight_shared.cpp
    Storm3D_Terrain.cpp storm3d_terrain_decalsystem.cpp storm3d_terrain_groups.cpp storm3d_terrain_heightmap.cpp
    storm3d_terrain_lightmanager.cpp storm3d_terrain_lod.cpp storm3d_terrain_models.cpp storm3d_terrain_renderer.cpp
    storm3d_terrain_utils.cpp Storm3d_Texture.cpp Storm3D_Texture_Video.cpp Storm3D_Vertex.cpp
    storm3d_video_player.cpp storm3d_videostreamer.cpp treader.cpp
)

CompileSrcList( system system
    FileTimestampChecker.cpp Logger.cpp SystemRandom.cpp SystemTime.cpp Timer.cpp Miscellaneous.cpp
)

CompileSrcList( ui ui
    AmbientSound.cpp AmbientSoundManager.cpp AmmoWindowCoop.cpp AmmoWindow.cpp AnimationSet.cpp Animator.cpp
    AniRecorderWindow.cpp ArmorConstructWindow.cpp ArmorPartSelectWindow.cpp BlackEdgeWindow.cpp
    CameraAutotilter.cpp CameraAutozoomer.cpp CharacterSelectionWindow.cpp CinematicScreen.cpp
    CombatMessageWindow.cpp CombatMessageWindowWithHistory.cpp CombatRadar.cpp CombatSubWindowFactory.cpp
    CombatUnitWindow.cpp CombatWindow.cpp ComboWindow.cpp CoopMenu.cpp CreditsMenu.cpp cursordefs.cpp
    DebugDataView.cpp DebugMapVisualizer.cpp DebugProjectileVisualizer.cpp DebugTrackerVisualizer.cpp
    DebugUnitVisualizer.cpp DebugVisualizerTextUtil.cpp DecalPositionCalculator.cpp Decoration.cpp
    DecorationManager.cpp DynamicLightManager.cpp Ejecter.cpp ElaborateHintMessageWindow.cpp ErrorWindow.cpp
    FlashlightWindow.cpp GameCamera.cpp GameConsole.cpp GameController.cpp GamePointers.cpp GameVideoPlayer.cpp
    GenericBarWindow.cpp GenericTextWindow.cpp GUIEffectWindow.cpp HealthWindowCoop.cpp HealthWindow.cpp
    IngameGuiTabs.cpp ItemWindow.cpp ItemWindowUpdator.cpp JoystickAimer.cpp LightManager.cpp LoadGameMenu.cpp
    LoadingMessage.cpp LoadingWindow.cpp LogEntry.cpp LogManager.cpp Logwindow.cpp Mainmenu.cpp Map.cpp
    MapWindow.cpp MenuBaseImpl.cpp MenuCollection.cpp MessageBoxWindow.cpp MissionFailureWindow.cpp
    MissionSelectionWindow.cpp MovieAspectWindow.cpp Muzzleflasher.cpp NewGameMenu.cpp NoCameraBoundary.cpp
    OffscreenUnitPointers.cpp OptionsMenu.cpp OptionsWindow.cpp ParticleArea.cpp ParticleCollision.cpp
    PlayerUnitCameraBoundary.cpp ProfilesMenu.cpp ScoreWindow.cpp SelectionBox.cpp SelectionVisualizer.cpp
    Spotlight.cpp StorageWindow.cpp SurvivalMenu.cpp SurvivorLoadGameMenu.cpp SurvivorUiElements.cpp
    SurvivorUpgradeWindow.cpp TacticalUnitWindow.cpp TargetDisplayButtonManager.cpp TargetDisplayWindowButton.cpp
    TargetDisplayWindow.cpp TargetDisplayWindowUpdator.cpp TerminalManager.cpp TerminalWindow.cpp Terrain.cpp
    TerrainCreator.cpp terrain_legacy.cpp uidefaults.cpp UIEffects.cpp UnitHealthBarWindow.cpp UnitHighlight.cpp
    UpgradeAvailableWindow.cpp UpgradeWindow.cpp VehicleWindow.cpp Visual2D.cpp VisualEffect.cpp
    VisualEffectManager.cpp VisualEffectType.cpp VisualObject.cpp VisualObjectModel.cpp WeaponSelectionWindow.cpp
    WeaponWindowCoop.cpp WeaponWindow.cpp
)

# TODO
# SURVIVOR_FILES:=
#     CoopMenu.cpp SurvivalMenu.cpp CharacterSelectionWindow.cpp SurvivorLoadGameMenu.cpp VehicleWindow.cpp


CompileSrcList( ui ui/camera_system
    CameraAreaStreet.cpp CameraSystem.cpp CameraSystemAim.cpp CameraSystemAreas.cpp
)


CompileSrcList( util util
    AI_PathFind.cpp AreaMap.cpp assert.cpp BuildingHandler.cpp BuildingMap.cpp CheckedIntValue.cpp
    CircleAreaTracker.cpp ClippedCircle.cpp ColorMap.cpp crc32.cpp CursorRayTracer.cpp Dampers.cpp
    Debug_MemoryManager.cpp DecalManager.cpp DecalSpawner.cpp DecalSystem.cpp DirectionalLight.cpp
    DistanceFloodfill.cpp FBCopyFile.cpp Floodfill.cpp FogApplier.cpp GridOcclusionCuller.cpp
    HelperPositionCalculator.cpp hiddencommand.cpp jpak.cpp LightAmountManager.cpp LightMap.cpp LineAreaChecker.cpp
    LipsyncManager.cpp LocaleManager.cpp LocaleResource.cpp mod_selector.cpp ObjectDurabilityParser.cpp
    PathDeformer.cpp PathSimplifier.cpp Preprocessor.cpp procedural_applier.cpp procedural_properties.cpp
    profiler.cpp ScreenCapturer.cpp Script.cpp ScriptManager.cpp ScriptProcess.cpp SelfIlluminationChanger.cpp
    SimpleParser.cpp SoundMaterialParser.cpp SpotLightCalculator.cpp StringUtil.cpp TextFileModifier.cpp
    TextFinder.cpp TextureCache.cpp TextureSwitcher.cpp UberCrypt.cpp UnicodeConverter.cpp
)

# minor macro for strings concatination
MACRO( CompileList result )
    FOREACH( item ${ARGN} )
        SET( ${result} "${${result}} ${item}" )
    ENDFOREACH( item )
ENDMACRO( CompileList )

IF( CMAKE_COMPILER_IS_GNUCXX )
    CompileList( SG_WARNINGS        -Wall
                                    -Wextra
                                    # -pedantic
                                    -Wcast-align
                                    -Wcast-qual
                                    # -Wconversion
                                    -Woverloaded-virtual
                                    # -Wnon-virtual-dtor
                                    -Wcomment
                                    -Wstrict-null-sentinel
                                    -Winit-self
                                    # -Werror

                                    -Wno-unused-parameter -Wno-unused-variable  # TODO
    )
    SET( CMAKE_CXX_FLAGS_DEBUG      "-O0 -g" )     # ( waiting for COMPILE_OPTIONS_<CONFIG> )
    SET( CMAKE_CXX_FLAGS_RELEASE    "-O2 -fno-strict-aliasing" )        # ( waiting for COMPILE_OPTIONS_<CONFIG> )
ELSE( CMAKE_COMPILER_IS_GNUCXX )
    MESSAGE( "WARNING: using default options for non-GCC compiler" )
ENDIF( CMAKE_COMPILER_IS_GNUCXX )

# Set default build options
MACRO( SetShadowgroundsProperties name flags defines )
    SET_TARGET_PROPERTIES( ${name} PROPERTIES
        COMPILE_FLAGS               "${SG_WARNINGS} ${flags}"
        COMPILE_DEFINITIONS_DEBUG   "_DEBUG=1;${defines}"
        COMPILE_DEFINITIONS_RELEASE "NDEBUG=1;${defines}"
    )
ENDMACRO( SetShadowgroundsProperties )

# Register executable
MACRO( AddShadowgroundsBinary name defines )
    ADD_EXECUTABLE( ${name} ${ARGN} )
    INCLUDE_DIRECTORIES( ${name} )
    SetShadowgroundsProperties( ${name} "-include ${PROJECT_SOURCE_DIR}/${name}/configuration.h" "${defines}")

    FOREACH( mod ${SG_MODULES})
        ADD_LIBRARY( ${name}_${mod} STATIC ${${mod}_sources} )
        SetShadowgroundsProperties( ${name}_${mod} "-include ${PROJECT_SOURCE_DIR}/${name}/configuration.h" "${defines}")
        TARGET_LINK_LIBRARIES( ${name} ${name}_${mod} )
    ENDFOREACH( mod )

    # linkage dependencies hotfix
    TARGET_LINK_LIBRARIES( ${name} ${name}_editor )
    TARGET_LINK_LIBRARIES( ${name} ${name}_filesystem )
    TARGET_LINK_LIBRARIES( ${name} ${name}_container )
    TARGET_LINK_LIBRARIES( ${name} ${name}_ogui)
    TARGET_LINK_LIBRARIES( ${name} ${name}_game )
    TARGET_LINK_LIBRARIES( ${name} ${name}_util )
    TARGET_LINK_LIBRARIES( ${name} ${name}_convert )
    TARGET_LINK_LIBRARIES( ${name} ${name}_sound )

    TARGET_LINK_LIBRARIES( ${name} ${GLOBAL_LIBRARIES} )
ENDMACRO( AddShadowgroundsBinary )

AddShadowgroundsBinary( shadowgrounds "PROJECT_SHADOWGROUNDS"
    shadowgrounds/shadowgrounds.cpp
    shadowgrounds/version.cpp
)

AddShadowgroundsBinary( survivor "PROJECT_SURVIVOR"
    survivor/survivor.cpp
    survivor/ui_credits_text.cpp
    survivor/version.cpp
)

# some files must be compiled unoptimized (gcc bug?)
IF( CMAKE_COMPILER_IS_GNUCXX )
    SET_SOURCE_FILES_PROPERTIES(
        storm/storm3dv2/Storm3D_Mesh_CollisionTable.cpp
        util/LipsyncManager.cpp
        PROPERTIES  COMPILE_FLAGS   "-O0"
    )
ENDIF( CMAKE_COMPILER_IS_GNUCXX )
