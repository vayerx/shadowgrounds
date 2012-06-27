# TODO split CMakeLists.txt

CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )
PROJECT( shadowgrounds )
SET( CPACK_PACKAGE_VERSION_MAJOR    1 )
SET( CPACK_PACKAGE_VERSION_MINOR    0 )
SET( CPACK_PACKAGE_VERSION_PATCH    4 )
SET( SG_PROG_VERSION                "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

SET( CMAKE_VERBOSE_MAKEFILE  OFF )

SET(SG_CMAKE_DIR "${shadowgrounds_SOURCE_DIR}/cmake")
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${SG_CMAKE_DIR}")

# ------------------------------------------------
#  Libraries
# ------------------------------------------------

SET( Boost_USE_STATIC_LIBS      OFF )
SET( Boost_USE_MULTITHREADED    ON )
SET( Boost_USE_STATIC_RUNTIME   OFF )
FIND_PACKAGE( Boost 1.46 REQUIRED
    program_options
    thread
)
FIND_PACKAGE( SDL REQUIRED )
FIND_PACKAGE( SDL_sound REQUIRED )
FIND_PACKAGE( SDL_image REQUIRED )
FIND_PACKAGE( SDL_ttf REQUIRED )
FIND_PACKAGE( OpenGL REQUIRED )
FIND_PACKAGE( GLEW REQUIRED )
FIND_PACKAGE( GTK2 REQUIRED )
FIND_PACKAGE( ZLIB REQUIRED )
FIND_PACKAGE( OpenAL REQUIRED )

SET( GLOBAL_INCLUDE_DIRS
    ${Boost_INCLUDE_DIRS}
    ${SDL_INCLUDE_DIR}
    ${SDL_SOUND_INCLUDE_DIR}
    ${SDLIMAGE_INCLUDE_DIR}
    ${SDLTTF_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_PATH}
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
    ${GLEW_LIBRARY}
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
SET ( SG_MODULES container convert game_ui editor system ogui storm util filesystem util_common sound particlesystem )
# game < editor
# game < util < sound
# ui < ogui
# ui <-> game
# storm < filesystem < util


# ------------------------------------------------
#  Options
# ------------------------------------------------

IF( NOT CMAKE_BUILD_TYPE )
    SET( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: Debug Release Gentoo." FORCE )
ENDIF( NOT CMAKE_BUILD_TYPE )

IF ( NOT INSTALLONLY )
    SET ( INSTALLONLY   shadowgrounds survivor CACHE STRING "Installation tagets, options are: shadowgrounds, survivor" FORCE )
ENDIF ( NOT INSTALLONLY )

OPTION( ENABLE_PHYSX "Enable NVIDIA PhysX SDK" OFF )
IF ( ENABLE_PHYSX )
    SET ( SG_MODULES physics ${SG_MODULES} )
    ADD_DEFINITIONS( -DPHYSICS_PHYSX )
ELSE ( ENABLE_PHYSX )
    ADD_DEFINITIONS( -DPHYSICS_NONE )
ENDIF ( ENABLE_PHYSX )


IF ( NOT CMAKE_DATA_PATH )
    IF ( UNIX )
        SET( CMAKE_DATA_PATH "/usr/share/games" )
    ELSEIF ( NOT SHADOWGROUNDS_DATA_PATH OR NOT SURVIVOR_DATA_PATH )
        MESSAGE( FATAL_ERROR "Neither {SHADOWGROUNDS,SURVIVOR}_DATA_PATH nor CMAKE_DATA_PATH is defined" )
    ENDIF ( UNIX )
ENDIF ( NOT CMAKE_DATA_PATH)

IF( NOT SHADOWGROUNDS_DATA_PATH )
    SET( SHADOWGROUNDS_DATA_PATH "${CMAKE_DATA_PATH}/shadowgrounds" CACHE PATH "Path to Shadowgrounds data files" FORCE )
ENDIF( NOT SHADOWGROUNDS_DATA_PATH )

IF( NOT SURVIVOR_DATA_PATH )
    SET( SURVIVOR_DATA_PATH "${CMAKE_DATA_PATH}/survivor" CACHE PATH "Path to Shadowgrounds Survivor data files" FORCE )
ENDIF( NOT SURVIVOR_DATA_PATH )

OPTION( LINKEDLIST_USE_NODE_POOL "Keep linkedlist nodes in pool, less dynamic memory allocations" ON )
OPTION( CRIMSON_MODE "The new non-rts, pure action game - \"crimson mode\"" ON )
OPTION( BUILDINGMAP_SHOW_LOADINGMESSAGE "Show rebuilding binaries loading message for buildingmaps" OFF )
OPTION( BUILDINGMAP_USE_OPTIONS "Make buildingmap use game's options (force/auto recreate, etc.)" ON )
OPTION( SCRIPT_PREPROCESS "allow script preprocessing - should be disabled for final release!!!" ON )
OPTION( PATHFIND_DEBUG "Make debug view map of pathfinding" OFF )
IF( NOT LIGHT_MAX_AMOUNT )
    SET( LIGHT_MAX_AMOUNT "5" CACHE STRING "Amount of light or what? Integer" FORCE )   # cmake doesn't support integers
ENDIF( NOT LIGHT_MAX_AMOUNT )
MARK_AS_ADVANCED( LINKEDLIST_USE_NODE_POOL BUILDINGMAP_SHOW_LOADINGMESSAGE LIGHT_MAX_AMOUNT )
CONFIGURE_FILE( project_common/configuration.h.in project_common/configuration.h )


# ------------------------------------------------
#  Source code
# ------------------------------------------------

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
    detail/unzip.cpp detail/ioapi.cpp
)

CompileSrcList( game_ui game
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

CompileSrcList( game_ui game/physics
    AbstractPhysicsObject.cpp BoxPhysicsObject.cpp CapsulePhysicsObject.cpp ConvexPhysicsObject.cpp gamephysics.cpp
    PhysicsContactDamageManager.cpp PhysicsContactEffectManager.cpp PhysicsContactSoundManager.cpp
    PhysicsContactUtils.cpp RackPhysicsObject.cpp StaticPhysicsObject.cpp 
)
IF ( ENABLE_PHYSX )
    CompileSrcList( game_ui game/physics physics_none.cpp )
ENDIF ( ENABLE_PHYSX )

CompileSrcList( game_ui game/scripting
    AnimationScripting.cpp CameraScripting.cpp CinematicScripting.cpp DecorScripting.cpp DevScripting.cpp
    EnvironmentScripting.cpp GameScriptingUtils.cpp HitChainScripting.cpp ItemScripting.cpp LightScripting.cpp
    MapScripting.cpp MathScripting.cpp MissionScripting.cpp OptionScripting.cpp
    PositionScripting.cpp SoundScripting.cpp StringScripting.cpp UnitScripting.cpp WaterScripting.cpp
    TrackingScripting.cpp SyncScripting.cpp DirectControlScripting.cpp GameScripting.cpp
    MiscScripting.cpp
)

CompileSrcList( game_ui game/tracking
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

CompileSrcList( game_ui ui
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


CompileSrcList( game_ui ui/camera_system
    CameraAreaStreet.cpp CameraSystem.cpp CameraSystemAim.cpp CameraSystemAreas.cpp
)


CompileSrcList( util util
    AI_PathFind.cpp AreaMap.cpp assert.cpp BuildingHandler.cpp BuildingMap.cpp CheckedIntValue.cpp
    CircleAreaTracker.cpp ClippedCircle.cpp ColorMap.cpp CursorRayTracer.cpp Dampers.cpp
    Debug_MemoryManager.cpp DecalManager.cpp DecalSpawner.cpp DecalSystem.cpp DirectionalLight.cpp
    DistanceFloodfill.cpp FBCopyFile.cpp Floodfill.cpp FogApplier.cpp GridOcclusionCuller.cpp
    HelperPositionCalculator.cpp hiddencommand.cpp LightAmountManager.cpp LightMap.cpp LineAreaChecker.cpp
    LipsyncManager.cpp LocaleManager.cpp LocaleResource.cpp mod_selector.cpp ObjectDurabilityParser.cpp
    PathDeformer.cpp PathSimplifier.cpp Preprocessor.cpp procedural_applier.cpp procedural_properties.cpp
    profiler.cpp ScreenCapturer.cpp Script.cpp ScriptManager.cpp ScriptProcess.cpp SelfIlluminationChanger.cpp
    SimpleParser.cpp SoundMaterialParser.cpp SpotLightCalculator.cpp StringUtil.cpp TextFileModifier.cpp
    TextFinder.cpp TextureCache.cpp TextureSwitcher.cpp UberCrypt.cpp UnicodeConverter.cpp
)

CompileSrcList( util_common util
    crc32.cpp 
    jpak.cpp
)


# ------------------------------------------------
#  Build options
# ------------------------------------------------

# minor macro for strings concatination
MACRO( CompileList result )
    FOREACH( item ${ARGN} )
        SET( ${result} "${${result}} ${item}" )
    ENDFOREACH( item )
ENDMACRO( CompileList )

IF( CMAKE_COMPILER_IS_GNUCXX )
    CompileList( SG_WARNINGS
        -Wall
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

        -Wno-unused-result
        -Wno-unused-parameter -Wno-unused-variable  # TODO
    )
    CompileList( SG_RELEASE_OPT
        -fcx-limited-range
        #-ffast-math
        -ffinite-math-only
        -fno-math-errno
        -fno-signaling-nans
        -fomit-frame-pointer
        -ftree-loop-distribution
        -ftree-loop-linear
        -funsafe-loop-optimizations
        -mfpmath=sse
        -mmmx
        -msse
        -msse2
        -mssse3
        #-msse4.1
    )
    # waiting for COMPILE_OPTIONS_<CONFIG>
    SET( CMAKE_CXX_FLAGS_DEBUG      "-O0 -g" )                  # ( waiting for COMPILE_OPTIONS_<CONFIG> )
    SET( CMAKE_CXX_FLAGS_RELEASE    "-O2 ${SG_RELEASE_OPT}" )

    SET( CMAKE_CXX_FLAGS            "-fno-strict-aliasing" )
ELSE( CMAKE_COMPILER_IS_GNUCXX )
    MESSAGE( WARNING "Using default options for non-GCC compiler" )
ENDIF( CMAKE_COMPILER_IS_GNUCXX )

# some files must be compiled unoptimized (gcc bug?)
IF( CMAKE_COMPILER_IS_GNUCXX )
SET_SOURCE_FILES_PROPERTIES(
    storm/storm3dv2/Storm3D_Mesh_CollisionTable.cpp
    util/LipsyncManager.cpp
    PROPERTIES  COMPILE_FLAGS   "-O0"
)
ENDIF( CMAKE_COMPILER_IS_GNUCXX )


# Set default build options
MACRO( SetShadowgroundsProperties name defines )
    SET_TARGET_PROPERTIES( ${name} PROPERTIES
        COMPILE_FLAGS               "${SG_WARNINGS} -include ${PROJECT_BINARY_DIR}/project_common/configuration.h"
        COMPILE_DEFINITIONS         "${defines};SG_PROG_VERSION=\"${SG_PROG_VERSION}\";SG_PROG_HOST=\"${CMAKE_SYSTEM_NAME}\""
        COMPILE_DEFINITIONS_DEBUG   "_DEBUG=1"
        COMPILE_DEFINITIONS_RELEASE "NDEBUG=1"
        COMPILE_DEFINITIONS_GENTOO  "NDEBUG=1"
    )
ENDMACRO( SetShadowgroundsProperties )


# ------------------------------------------------
#  Applications
# ------------------------------------------------

MACRO( AddShadowgroundsBinary name defines )
    ADD_EXECUTABLE( ${name} ${ARGN} )
    INCLUDE_DIRECTORIES( ${name} )
    SetShadowgroundsProperties( ${name} "${defines}" )

    FOREACH( mod ${SG_MODULES})
        ADD_LIBRARY( ${name}_${mod} STATIC ${${mod}_sources} )
        SetShadowgroundsProperties( ${name}_${mod} "${defines}" )
        TARGET_LINK_LIBRARIES( ${name} ${name}_${mod} )
    ENDFOREACH( mod )

    TARGET_LINK_LIBRARIES( ${name} ${GLOBAL_LIBRARIES} )
ENDMACRO( AddShadowgroundsBinary )

AddShadowgroundsBinary( shadowgrounds "PROJECT_SHADOWGROUNDS=1;GAMEDATA_PATH=\"${SHADOWGROUNDS_DATA_PATH}\""
    shadowgrounds/shadowgrounds.cpp
    shadowgrounds/version.cpp
)

AddShadowgroundsBinary( survivor "PROJECT_SURVIVOR=1;GAMEDATA_PATH=\"${SURVIVOR_DATA_PATH}\""
    survivor/survivor.cpp
    survivor/ui_credits_text.cpp
    survivor/version.cpp
)


# ------------------------------------------------
#  Installation
# ------------------------------------------------

IF( UNIX AND NOT APPLE AND NOT CYGWIN )
    OPTION( ENABLE_DESKTOP_ENTRY "Enable installation of desktop entry files" ON )
    SET( DATAROOT_DIR "${CMAKE_INSTALL_PREFIX}/share" CACHE STRING "Root of data directories")
    SET( ICON_DIR "${DATAROOT_DIR}/pixmaps" CACHE STRING "Icon directory for desktop entry")
    SET( DESKTOP_DIR "${DATAROOT_DIR}/applications/" CACHE STRING "Desktop file directory for desktop entry")
ENDIF( UNIX AND NOT APPLE AND NOT CYGWIN )

FOREACH( game ${INSTALLONLY} )
    INSTALL(
        TARGETS         ${game}
        COMPONENT       ${game}
        PERMISSIONS     OWNER_READ OWNER_WRITE OWNER_EXECUTE  GROUP_READ GROUP_EXECUTE
        DESTINATION     bin
        #CONFIGURATIONS  Release Gentoo
    )
    IF( ENABLE_DESKTOP_ENTRY AND DESKTOP_DIR AND ICON_DIR )
        INSTALL(FILES icons/${game}.desktop DESTINATION ${DESKTOP_DIR} )
        INSTALL(FILES icons/${game}.xpm DESTINATION ${ICON_DIR} )
    ENDIF( ENABLE_DESKTOP_ENTRY AND DESKTOP_DIR AND ICON_DIR )
ENDFOREACH( game )


# ------------------------------------------------
#  Packaging
# ------------------------------------------------

SET( CPACK_PACKAGE_DESCRIPTION_SUMMARY  "3rd person alien shooter.")
SET( CPACK_PACKAGE_VERSION              ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR} )
SET( CPACK_GENERATOR                    "TGZ;STGZ" )
SET( CPACK_SOURCE_GENERATOR             "TGZ" )
SET( CPACK_PACKAGE_NAME                 "Shadowgrounds" )
SET( CPACK_PACKAGE_VENDOR               "Frozenbyte, Alternative Games, vayerx" )
SET( CPACK_PACKAGE_FILE_NAME            "shadowgrounds-${CPACK_PACKAGE_VERSION}_${CMAKE_SYSTEM_PROCESSOR}" )
SET( CPACK_SOURCE_PACKAGE_FILE_NAME     "shadowgrounds-${CPACK_PACKAGE_VERSION}-src" )
SET( CPACK_RESOURCE_FILE_LICENSE        ${PROJECT_SOURCE_DIR}/LICENSE.txt )
SET( CPACK_INCLUDE_TOPLEVEL_DIRECTORY   0 )
SET( CPACK_SOURCE_IGNORE_FILES          "/build/;/data/;data.*\\.fbz;/.*\\.kdev.*;/\\.git/;.*~;.*\\.orig.*;/.*\\.log" )
INCLUDE( CPack )
