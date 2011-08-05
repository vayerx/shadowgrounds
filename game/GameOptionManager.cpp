// Copyright 2002-2004 Frozenbyte Ltd.

#include "precompiled.h"

#ifdef _MSC_VER
#pragma warning(disable:4103)
#pragma warning(disable:4786)
#endif

#include "GameOptionManager.h"

#include <assert.h>
#include <string.h>
#include <string>
#include <fstream>
#include "GameConfigs.h"
#include "../container/LinkedList.h"
#include "../convert/str2int.h"
#include "../system/Logger.h"
//#include "../util/Parser.h"
#include "../editor/parser.h"
#include "../editor/string_conversions.h"
#include "options/options_all.h"
#include "../filesystem/file_package_manager.h"
#include "gamedefs.h"
#include <string>
#include <stdlib.h>

#include "../util/Debug_MemoryManager.h"

#include "userdata.h"

#define GAMEOPTIONS_PARAMS 6
#define GAMEOPTIONS_PARAM_NAME 0
#define GAMEOPTIONS_PARAM_TYPE 1
#define GAMEOPTIONS_PARAM_GROUP 2
#define GAMEOPTIONS_PARAM_DEFAULTVALUE 3
#define GAMEOPTIONS_PARAM_MAXVALUE 4
#define GAMEOPTIONS_PARAM_TOGGLESTEP 5


// *done* - (save at quit: "b+", "i+", "s+", "f+")
// *done* - (don't save: "b", "i", "s", "f")
// *done* - (need apply: "ba", "ia", "sa", "fa")
// *done* - (need apply / save at quit: "ba+", "ia+", "sa+", "fa+")
// (need restart "br", "", ... )
// (need restart / save at quit "br+", "", ... )

/*
option types:
b = boolean
i = integer
f = float
s = string
a = need applying
r = need restart
+ = save at quit
*/

using namespace frozenbyte;

namespace {
namespace parser
{
	bool hasProperty(const editor::ParserGroup &grp, const char *property)
	{
		if(!property)
			return false;

		const std::string &val = grp.getValue(property);
		if(val.empty())
			return false;

		return true;
	}

	std::string getString(const editor::ParserGroup &grp, const char *property)
	{
		if(!property)
			return false;

		return grp.getValue(property);
	}

	int getInt(const editor::ParserGroup &grp, const char *property)
	{
		if(!property)
			return false;

		const std::string &val = grp.getValue(property);
		return atoi(val.c_str());
	}

	float getFloat(const editor::ParserGroup &grp, const char *property)
	{
		if(!property)
			return false;

		const std::string &val = grp.getValue(property);
		return float(atof(val.c_str()));
	}

}
}

namespace game
{
	// option, type, group, (default value), (max value), (toggle step)
	static const char *gameOptions[(DH_OPT_AMOUNT + 1) * GAMEOPTIONS_PARAMS] =
	{
		"_reserved_", "b", "Reserved", "-", "-", "-",
		"force_cover_bin_recreate", "b", "Precalc", "0", "-", "-",
		"auto_cover_bin_recreate", "b", "Precalc", "1", "-", "-",

		"show_tactical_text", "b", "Tactical", "0", "-", "-",
		"menu_two_click", "b", "Tactical", "0", "-", "-",
		"menu_autopause", "b", "Tactical", "0", "-", "-",

		"show_all_units", "b", "Game", "1", "-", "-",
		"show_enemy_tactical", "b", "Cheats", "0", "-", "-",
		"no_camera_boundaries", "ba", "Game", "1", "-", "-",

		"force_hide_bin_recreate", "b", "Precalc", "0", "-", "-",
		"auto_hide_bin_recreate", "b", "Precalc", "1", "-", "-",

		"terrain_memory_reserve", "i", "Memory", "-", "-", "-",
		"terrain_memory_precache", "i", "Memory", "-", "-", "-",

		"show_terrain_memory_info", "b", "Debug", "0", "-", "-",
		"script_dev_mode", "b", "Cheats", "0", "-", "-",

		"target_based_weapon_choose", "b", "Tactical", "-", "-", "-",
		"targeting_stops", "b", "Tactical", "-", "-", "-",
		"double_click_move_fast", "b", "Tactical", "-", "-", "-",
		"hostile_sighted_autopause", "b", "Tactical", "-", "-", "-",

		"debug_console_commands", "b", "Debug", "0", "-", "-",
		"allow_side_swap", "b", "Cheats", "0", "-", "-",
		"show_paths", "b", "Cheats", "0", "-", "-",

		"video_enabled", "b+", "Video", "1", "-", "-",
		"external_video_player", "b", "Video", "0", "-", "-",

		"texture_detail_level", "ia+", "Graphics", "50", "100", "50",

		"mouse_enabled", "ba+", "Controllers", "1", "-", "-",
		"keyboard_enabled", "ba+", "Controllers", "1", "-", "-",
		"joystick_enabled", "ba+", "Controllers", "1", "-", "-",
		"mouse_force_given_boundary", "ba+", "Controllers", "0", "-", "-",
		"mouse_sensitivity", "fa+", "Controllers", "1.0f", "-", "-",

		"1st_player_enabled", "b", "Players", "1", "-", "-",
		"2nd_player_enabled", "b", "Players", "0", "-", "-",
		"3rd_player_enabled", "b", "Players", "0", "-", "-",
		"4th_player_enabled", "b", "Players", "0", "-", "-",

#ifdef LEGACY_FILES
		"1st_player_keybinds", "s", "Players", igios_mapUserDataPrefix("Config/keybinds.txt").c_str(), "-", "-",
#else
		"1st_player_keybinds", "s", "Players", igios_mapUserDataPrefix("config/keybinds.txt").c_str(), "-", "-",
#endif
		"2nd_player_keybinds", "s", "Players", "-", "-", "-",
		"3rd_player_keybinds", "s", "Players", "-", "-", "-",
		"4th_player_keybinds", "s", "Players", "-", "-", "-",

		"1st_player_has_cursor", "b", "Players", "1", "-", "-",
		"2nd_player_has_cursor", "b", "Players", "0", "-", "-",
		"3rd_player_has_cursor", "b", "Players", "0", "-", "-",
		"4th_player_has_cursor", "b", "Players", "0", "-", "-",

		"loglevel", "ia+", "Debug", "2", "4", "1",
		"console_loglevel", "ia+", "Debug", "2", "4", "1",
		"raise_console_loglevel", "ia+", "Debug", "1", "4", "1",

		"force_model_bin_recreate", "b", "Precalc", "0", "-", "-",
		"auto_model_bin_recreate", "b", "Precalc", "1", "-", "-",

		"force_script_preprocess", "b", "Precalc", "0", "-", "-",
		"auto_script_preprocess", "b", "Precalc", "1", "-", "-",
		"script_preprocessor", "s", "Precalc", "", "-", "-",

		"1st_player_control_scheme", "i", "Players", "0", "13", "1",
		"2nd_player_control_scheme", "i", "Players", "1", "13", "1",
		"3rd_player_control_scheme", "i", "Players", "4", "13", "1",
		"4th_player_control_scheme", "i", "Players", "7", "13", "1",

		"script_preprocessor_check", "s", "Precalc", "", "-", "-",

		"allow_full_plugin_access", "b", "Plugins", "0", "-", "-",
		"default_plugin", "s", "Plugins", "", "-", "-",

		"gamma", "fa+", "Display", "1.0f", "2.0f", "0.1f",
		"brightness", "fa+", "Display", "1.0f", "2.0f", "0.1f",
		"contrast", "fa+", "Display", "1.0f", "2.0f", "0.1f",
		"red_correction", "fa", "Display", "1.0f", "2.0f", "0.1f",
		"green_correction", "fa", "Display", "1.0f", "2.0f", "0.1f",
		"blue_correction", "fa", "Display", "1.0f", "2.0f", "0.1f",
		"calibrate_gamma", "ba", "Display", "0", "-", "-",

		"extra_gamma_effects", "b+", "Graphics", "1", "-", "-",
		"weather_effects", "b+", "Graphics", "1", "-", "-",
		"particle_effects_level", "i+", "Graphics", "50", "100", "50", // few particles, medium, high amount 
		"green_blood", "b+", "Graphics", "0", "-", "-", 
		"gore_level", "i+", "Graphics", "50", "100", "50", // blood off, little blood, lots of blood
		"shadows_level", "ia+", "Graphics", "100", "100", "100", // flashlight/lightning shadows only, static light fake shadows too
		"lighting_level", "ia+", "Graphics", "50", "100", "50", // flashlight/lightning only spot, muzzleflash spot too, other spots?, explosion spots!
		                                   // 100 would be very high (extreme, explosion pointlights and stuff..)
		"speech_language", "i+", "Locale", "0", "4", "1", 
		"menu_language", "i+", "Locale", "0", "4", "1", 
		"subtitle_language", "i+", "Locale", "0", "4", "1", 

		"show_fps", "b", "Debug", "0", "-", "-", 
		"show_polys", "b", "Debug", "0", "-", "-", 

		"camera_autotilt_amount", "i", "Camera", "50", "100", "10", 
		"render_mode", "ia", "Graphics", "0", "2", "1",
		"render_spot_shadows", "ba", "Graphics", "-", "-", "-",
		"render_glow", "ba+", "Graphics", "-", "-", "-",
		"render_fakelights", "ba", "Graphics", "-", "-", "-",
		"render_models", "ba", "Graphics", "-", "-", "-",
		"render_terrainobjects", "ba", "Graphics", "-", "-", "-",
		"render_heightmap", "ba", "Graphics", "-", "-", "-",
		"camera_autotilt_alpha", "i", "Camera", "0", "100", "10", 
		"camera_autotilt_beta", "i", "Camera", "100", "100", "10", 
		"old_model_bin_recreate", "b", "Precalc", "0", "-", "-",
		"show_debug_data_view", "b", "Debug", "0", "-", "-",
		"debug_data_view_type_mask", "i", "Debug", "0", "31", "1",
		"debug_data_view_rate", "i", "Debug", "200", "1000", "100",

		"debug_data_view_alpha", "i", "Debug", "90", "100", "10",
		"lighting_texture_quality", "i+", "Graphics", "0", "100", "100", // low resolution, high resolution
		"shadows_texture_quality", "i+", "Graphics", "50", "100", "50", // low resolution, medium resolution, high resolution
		"fakeshadows_texture_quality", "i+", "Graphics", "50", "100", "50", // low resolution, medium resolution, high resolution
		"freeze_camera_culling", "ba", "Graphics", "-", "-", "-",
		"freeze_spot_culling", "ba", "Graphics", "-", "-", "-",
		"render_wireframe", "ba", "Graphics", "-", "-", "-",

		"controller_script_1", "s", "Controllers", "controller_script_1", "-", "-",
		"controller_script_2", "s", "Controllers", "controller_script_2", "-", "-",
		"controller_script_3", "s", "Controllers", "controller_script_3", "-", "-",
		"controller_script_4", "s", "Controllers", "controller_script_4", "-", "-",
		"controller_script_5", "s", "Controllers", "controller_script_5", "-", "-",
		"controller_script_6", "s", "Controllers", "controller_script_6", "-", "-",
		"controller_script_7", "s", "Controllers", "controller_script_7", "-", "-",
		"controller_script_8", "s", "Controllers", "controller_script_8", "-", "-",
		"controller_script_9", "s", "Controllers", "controller_script_9", "-", "-",
		"controller_script_10", "s", "Controllers", "controller_script_10", "-", "-",
		"controller_script_11", "s", "Controllers", "controller_script_11", "-", "-",
		"controller_script_12", "s", "Controllers", "controller_script_12", "-", "-",
		"controller_script_13", "s", "Controllers", "controller_script_13", "-", "-",
		"controller_script_14", "s", "Controllers", "controller_script_14", "-", "-",
		"controller_script_15", "s", "Controllers", "controller_script_15", "-", "-",
		"controller_script_16", "s", "Controllers", "controller_script_16", "-", "-",
		
		"decal_fade_time", "i+", "Effects", "10000", "30000", "5000",
		"decal_max_amount", "i+", "Effects", "500", "3000", "250",
		"autoaim_horizontal", "b", "Cheats", "-", "-", "-",
		"autoaim_vertical", "b", "Cheats", "-", "-", "-",

		"sounds_enabled", "b+", "Sounds", "1", "-", "-",
		"music_shuffle", "b+", "Sounds", "1", "-", "-",
		"master_volume", "ia+", "Sounds", "100", "-", "-",
		"music_enabled", "ba+", "Sounds", "1", "-", "-",
		"music_volume", "ia+", "Sounds", "90", "-", "-",
		"speech_enabled", "ba+", "Sounds", "1", "-", "-",
		"speech_volume", "ia+", "Sounds", "100", "-", "-",
		"fx_enabled", "ba+", "Sounds", "1", "-", "-",
		"fx_volume", "ia+", "Sounds", "90", "-", "-",

		"camera_mode", "i", "Camera", "3", "4", "1",
		"camera_range", "i", "Camera", "400", "800", "50",
		"camera_time_factor", "f", "Camera", "1.0f", "2.0f", "0.1f",
		"camera_no_delta_time_limit", "b", "Camera", "0", "-", "-",
		"camera_disable_timing", "b", "Camera", "0", "-", "-",
		"camera_invert_look", "b", "Camera", "0", "-", "-",

		"screen_width", "i+", "Display", "800", "-", "-",
		"screen_height", "i+", "Display", "600", "-", "-",
		"screen_bpp", "i+", "Display", "32", "-", "-",
		"windowed", "b+", "Display", "0", "1", "1",
		"maximize_window", "b+", "Display", "1", "1", "1",

		"camera_scrolly_enabled", "b", "Camera", "-", "-", "-",

		"sound_mixrate", "i+", "Sounds", "44100", "-", "-",
		"sound_use_hardware", "b+", "Sounds", "-", "-", "-",

		"sound_software_channels", "i+", "Sounds", "16", "32", "2",
		"sound_required_hardware_channels", "i+", "Sounds", "16", "32", "2",
		"sound_max_hardware_channels", "i+", "Sounds", "32", "32", "2",

		"sound_use_eax", "b+", "Sounds", "-", "-", "-",
		"sound_speaker_type", "s+", "Sounds", "stereo", "-", "-",

		"render_max_fps", "i", "Display", "0", "200", "10",
		"game_mode_rts", "b", "Game", "0", "-", "-",
		"game_mode_topdown_shooter", "b", "Game", "1", "-", "-",
		"game_mode_aim_upward", "b+", "Game", "1", "-", "-",
		"player_demo_invulnerability", "b", "Cheats", "0", "-", "-",

		"render_use_vsync", "b+", "Display", "0", "-", "-",
		"render_collision", "ba", "Graphics", "0", "-", "-",
		"show_debug_raytraces", "b", "Debug", "0", "-", "-",
		"debug_raytraces_type_mask", "i", "Debug", "15", "15", "1",
#ifdef LEGACY_FILES
		"game_missiondefs", "sr", "Game", "Data/Missions/missiondefs.txt", "-", "-",
#else
		"game_missiondefs", "sr", "Game", "data/mission/missiondefs.txt", "-", "-",
#endif
		"cleanup_skip_rate", "i", "Memory", "0", "9", "1",
		"flashlight_sway_factor", "f", "Game", "0.05f", "0.3f", "0.01f",
		"flashlight_sway_time", "i", "Game", "100", "300", "10",
		"flashlight_shake_factor", "f", "Game", "0.1f", "0.3f", "0.01f",
		"flashlight_shake_time", "i", "Game", "10", "40", "10",

		"flashlight_impact_factor", "f", "Game", "0.1f", "0.3f", "0.01f",
		"game_execute_use_selected_item", "b", "Game", "1", "-", "-",
		"sound_preload", "b", "Sounds", "1", "-", "-",
		"gui_units_position", "i", "GUI", "2", "1", "3",
		"gui_radar_mode", "i", "GUI", "2", "1", "3",
		"high_quality_lightmap", "b+", "Graphics", "1", "-", "-",
		"console_history_save", "b+", "Debug", "0", "-", "-",
		"material_missing_warning", "b", "Debug", "1", "-", "-",
		"sound_missing_warning", "b", "Debug", "1", "-", "-",
		"force_obstacle_bin_recreate", "b", "Precalc", "0", "-", "-",
		"auto_obstacle_bin_recreate", "b", "Precalc", "1", "-", "-",
		"lightmap_filtering", "ba", "Graphics", "0", "-", "-",
		"shader_gamma_effects", "b", "Graphics", "1", "-", "-",
		"render_decals", "ba", "Graphics", "1", "-", "-",
		"render_spot_debug", "ba", "Graphics", "0", "-", "-",
		"render_fakeshadow_debug", "ba", "Graphics", "0", "-", "-",
		"render_glow_debug", "ba", "Graphics", "0", "-", "-",
		"render_boned", "ba", "Graphics", "1", "-", "-",
		"gui_tip_message_level", "i+", "GUI", "1", "2", "1",

		"custom_int_1", "i", "Custom", "0", "-", "-",
		"custom_int_2", "i", "Custom", "0", "-", "-",
		"custom_int_3", "i", "Custom", "0", "-", "-",
		"custom_int_4", "i", "Custom", "0", "-", "-",
		"custom_string_1", "s", "Custom", "", "-", "-",
		"custom_string_2", "s", "Custom", "", "-", "-",
		"custom_string_3", "s", "Custom", "", "-", "-",
		"custom_string_4", "s", "Custom", "", "-", "-",
		"custom_float_1", "f", "Custom", "0.0f", "-", "-",
		"custom_float_2", "f", "Custom", "0.0f", "-", "-",
		"custom_float_3", "f", "Custom", "0.0f", "-", "-",

		"custom_float_4", "f", "Custom", "0.0f", "-", "-",
		"custom_bool_1", "b", "Custom", "-", "-", "-",
		"custom_bool_2", "b", "Custom", "-", "-", "-",
		"custom_bool_3", "b", "Custom", "-", "-", "-",
		"custom_bool_4", "b", "Custom", "-", "-", "-",
		"camera_scrolly_amount", "i", "Camera", "100", "200", "10",
		"camera_min_beta_angle", "i", "Camera", "65", "85", "5",
		"camera_max_beta_angle", "i", "Camera", "65", "85", "5",
		"camera_default_zoom", "f", "Camera", "11.0f", "15.0f", "1.0f",
		"camera_default_zoom_player_inc", "f", "Camera", "1.0f", "3.0f", "0.5f",

		"camera_default_fov", "i", "Camera", "120", "140", "10",
		"camera_autozoom_enabled", "b", "Camera", "1", "-", "-",
		"camera_autozoom_indoor", "f", "Camera", "8.5f", "-", "-",
		"camera_autozoom_outdoor", "f", "Camera", "11.5f", "-", "-",
		"camera_scrolly_speed", "i", "Camera", "100", "200", "10",
		"render_black_and_white", "ba", "Graphics", "-", "-", "-",
		"hit_effect_image", "b+", "Graphics", "-", "-", "-",
		"camera_rotation_strength", "f+", "Camera", "0.6f", "-", "-",
		"camera_rotation_safe", "f+", "Camera", "300", "-", "-",
		"camera_rotation_spring", "f+", "Camera", "1.0f", "-", "-",

		"camera_rotation_fade_start", "f+", "Camera", "768.0f", "-", "-",
		"camera_rotation_fade_end", "f+", "Camera", "769.0f", "-", "-",
		"game_flashlight_recharge_min_illumination", "i", "Game", "15", "-", "-",
		"game_flashlight_energy_max", "i", "Game", "60000", "-", "-",
		"game_flashlight_energy_low", "i", "Game", "15000", "-", "-",
		"game_flashlight_energy_usage", "i", "Game", "10", "-", "-",
		"game_flashlight_recharge_dynamic", "i", "Game", "50", "-", "-",
		"game_flashlight_recharge_static", "i", "Game", "50", "-", "-",
		"game_flashlight_recharge_static_dark", "i", "Game", "10", "-", "-",
		"camera_rotation_auto_strength", "f+", "Camera", "0.7f", "-", "-",

		"camera_rotation_auto_fade_start", "f+", "Reserved", "304", "-", "-",
		"camera_rotation_auto_fade_end", "f+", "Reserved", "384", "-", "-",
		"corpse_disappear_time", "i+", "Game", "15", "60", "15",
		"environment_animations", "b+", "Graphics", "1", "-", "-",
		"render_distortion", "ba+", "Graphics", "0", "-", "-",
		"menu_video_enabled", "b+", "Video", "1", "-", "-",

		"antialias_samples", "i+", "Display", "0", "7", "-",
		"anisotrophy", "ia+", "Display", "0", "16", "-",

		"better_glow_sampling", "ba+", "Graphics", "0", "1", "-",
		"render_cone", "ba", "Graphics", "1", "-", "-",
		"render_particles", "ba", "Graphics", "1", "-", "-",
		"render_terrain_textures", "ba", "Graphics", "1", "-", "-",
		"render_smoothed_shadows", "ba", "Graphics", "0", "-", "-",
		"layer_effects_level", "i+", "Graphics", "50", "100", "50",
		"collect_game_stats", "b", "Game", "-", "-", "-",
		"joystick_sensitivy", "f+", "Controllers", "1.0f", "10.0f", "3.0f",
		"camera_culling_rate", "i", "Graphics", "0", "2", "1",

		"create_error_log", "b", "Debug", "0", "-", "-",

		"cursor_raytrace_offset_y", "i", "GUI", "8", "-", "-",
		"game_aim_height_offset", "f", "Game", "1.2f", "-", "-",

		"menu_logo_video_enabled", "b+", "Video", "1", "-", "-",
		"high_quality_video", "b+", "Video", "1", "-", "-",
		"procedural_fallback", "ba+", "Graphics", "0", "1", "-",
		"debug_data_view_perf_stats", "b", "Debug", "0", "-", "-",
		"debug_data_view_perf_stats_rate", "i", "Debug", "1000", "10000", "1000",
		"reset_renderer_when_loaded", "b+", "Graphics", "1", "-", "-",
		"game_very_hard_available", "b+", "Game", "0", "-", "-",
		"game_extremely_hard_available", "b+", "Game", "0", "-", "-",
		"mod", "s", "Game", "", "-", "-",
		"item_min_lighting", "i", "Game", "0", "100", "10",

		"window_titlebar", "b+", "Display", "0", "1", "1",
		"savegame_slot_amount", "i", "Game", "11", "11", "1",

#ifdef GAME_SIDEWAYS
		"game_sideways", "ba", "Game", "1", "-", "-",
#else
		"_reserved_game_sideways", "ba", "Game", "0", "-", "-",
#endif
		"camera_default_target_offset_x", "f", "Camera", "0.0f", "-", "-",
		"camera_default_target_offset_z", "f", "Camera", "0.0f", "-", "-",
		"camera_default_position_offset_x", "f", "Camera", "0.0f", "-", "-",
		"camera_default_position_offset_z", "f", "Camera", "0.0f", "-", "-",
		"camera_alpha_angle_locked", "b", "Camera", "0", "-", "-",
		"camera_alpha_angle_locked_value", "f", "Camera", "0.0f", "-", "-",

		"gui_radar_show_all", "b", "GUI", "0", "-", "-",

#ifdef PHYSICS_NONE
		"_reserved_p1", "b", "Reserved", "0", "-", "-",
		"_reserved_p2", "b", "Reserved", "0", "-", "-",
		"_reserved_p3", "b", "Reserved", "0", "-", "-",
		"_reserved_p4", "b", "Reserved", "0", "-", "-",
		"_reserved_p5", "b", "Reserved", "0", "-", "-",
		"_reserved_p6", "b", "Reserved", "0", "-", "-",
		"_reserved_p7", "b", "Reserved", "0", "-", "-",
		"_reserved_p8", "b", "Reserved", "0", "-", "-",
		"_reserved_p9", "b", "Reserved", "0", "-", "-",
		"_reserved_p10", "b", "Reserved", "0", "-", "-",
#else
		"physics_enabled", "b+", "Physics", "1", "-", "-",
		"physics_update", "b", "Physics", "1", "-", "-",
		"physics_visualize_collision_shapes", "ba", "Physics", "0", "-", "-",
		"physics_visualize_collision_contacts", "ba", "Physics", "0", "-", "-",
		"physics_visualize_collision_aabbs", "ba", "Physics", "0", "-", "-",
		"physics_fluids_enabled", "b+", "Physics", "1", "-", "-",
		"physics_rigids_enabled", "b", "Physics", "1", "-", "-",
		"physics_particles", "b+", "Physics", "1", "-", "-",
		"physics_shapes", "b", "Physics", "1", "-", "-",
		"physics_use_multithreading", "b+", "Physics", "1", "-", "-",
#endif

#ifdef PHYSICS_NONE
		"_reserved_p11", "b", "Reserved", "0", "-", "-",
		"_reserved_p12", "b", "Reserved", "0", "-", "-",
		"_reserved_p13", "b", "Reserved", "0", "-", "-",
#else
#ifdef PHYSICS_PHYSX
		"physics_use_hardware", "b+", "Physics", "1", "-", "-",
#else
		"_reserved_physics_use_hardware", "b", "Physics", "0", "-", "-",
#endif
		"physics_visualize_dynamic", "ba", "Physics", "0", "-", "-",
		"physics_visualize_static", "ba", "Physics", "0", "-", "-",
#endif
		"weapon_eject_effect_level", "i+", "Effects", "50", "100", "50",
		"render_half_resolution", "ba+", "Graphics", "0", "-", "-",

#ifdef PHYSICS_NONE
		"_reserved_p14", "b", "Reserved", "0", "-", "-",
		"_reserved_p15", "b", "Reserved", "0", "-", "-",
		"_reserved_p16", "b", "Reserved", "0", "-", "-",
		"_reserved_p17", "b", "Reserved", "0", "-", "-",
		"_reserved_p18", "b", "Reserved", "0", "-", "-",
		"_reserved_p19", "b", "Reserved", "0", "-", "-",
		"_reserved_p20", "b", "Reserved", "0", "-", "-",
		"_reserved_p21", "b", "Reserved", "0", "-", "-",
		"_reserved_p22", "b", "Reserved", "0", "-", "-",
#else
		"physics_visualize_fluids", "ba", "Physics", "0", "-", "-",
		"physics_log_stats", "b", "Physics", "0", "-", "-",
#ifdef PROJECT_AOV
		"physics_time_step", "i", "Physics", "10", "-", "-",
#else
		"physics_time_step", "i", "Physics", "15", "-", "-",
#endif
		"physics_ground_plane_height", "f", "Physics", "-100.0f", "-", "-",
#ifdef PHYSICS_PHYSX
		"physics_use_hardware_fully", "b+", "Physics", "0", "-", "-",
#else
		"_reserved_physics_use_hardware_fully", "b", "Physics", "0", "-", "-",
#endif
		"physics_terrain_enabled", "b", "Physics", "1", "-", "-",
		"physics_use_hardware_terrain", "b", "Physics", "1", "-", "-",
		"physics_max_model_particles", "ia+", "Physics", "50", "500", "50",
		"physics_max_model_particles_spawn_per_tick", "ia+", "Physics", "1", "50", "20",
#endif

		"force_physics_cache_recreate", "b", "Precalc", "0", "-", "-",
		"auto_physics_cache_recreate", "b", "Precalc", "1", "-", "-",
#ifdef PHYSICS_NONE
		"_reserved_p23", "b", "Reserved", "0", "-", "-",
#else
		"physics_max_fluid_particles", "i+", "Physics", "500", "2000", "250",
#endif

#ifdef PROJECT_SHADOWGROUNDS
		"game_contact_damage_required_force_terrain_object", "f", "Game", "1000000.0f", "-", "-",
		"game_contact_damage_required_linear_acceleration", "f", "Game", "3.0f", "-", "-",
		"game_contact_damage_required_angular_acceleration", "f", "Game", "3.0f", "-", "-",
		"_reserved_p24", "f", "Game", "2000.0f", "-", "-",
#else
		"game_contact_damage_required_force_terrain_object", "f", "Game", "200.0f", "-", "-",
		"game_contact_damage_required_linear_acceleration", "f", "Game", "1.0f", "-", "-",
		"game_contact_damage_required_angular_acceleration", "f", "Game", "1.0f", "-", "-",
		"game_contact_damage_required_force_other", "f", "Game", "1000.0f", "-", "-",
#endif

		"render_reflection", "ba+", "Graphics", "0", "-", "-",
    
#ifdef PROJECT_SHADOWGROUNDS
		"game_contact_damage_required_force_unit", "f", "Game", "100000.0f", "-", "-",
#else
		"game_contact_damage_required_force_unit", "f", "Game", "100.0f", "-", "-",
#endif    
		"game_contact_damage_required_unit_damage_velocity", "f", "Game", "2.0f", "-", "-",
    
		"occlusion_culling_enabled", "b", "Graphics", "0", "-", "-",
    
		"game_combo_required_time", "i", "Game", "1000", "-", "-",
		"game_combo_shown_on_screen_time", "i", "Game", "1000", "-", "-",

		"physics_sky_plane_height", "f", "Physics", "-100.0f", "-", "-",
		"physics_sky_force_height", "f", "Physics", "-1000.0f", "-", "-",
		"physics_ground_force_height", "f", "Physics", "-1000.0f", "-", "-",

		"physics_gravity", "f", "Physics", "-9.81f", "-", "-",
		"physics_default_static_friction", "f", "Physics", "0.8f", "-", "-",
		"physics_default_dynamic_friction", "f", "Physics", "0.8f", "-", "-",
		"physics_default_restitution", "f", "Physics", "0.3f", "-", "-",
		"debug_visualize_trackers", "b", "Debug", "0", "-", "-",
#ifdef DEMOVERSION
		"show_splash_screen", "b", "GUI", "1", "-", "-",
#else
		"show_splash_screen", "b", "GUI", "0", "-", "-",
#endif
		"ast_preprocessor", "s", "Precalc", "", "-", "-",
		"ast_preprocessor_check", "s", "Precalc", "", "-", "-",
		"force_ast_preprocess", "b", "Precalc", "0", "-", "-",
		"auto_ast_preprocess", "b", "Precalc", "1", "-", "-",

		//"render_point_light_max_amount", "ia+", "Graphics", "3", "5", "3",
		"_legacy_render_point_light_max_amount", "ia", "Reserved", "3", "5", "3",

		"debug_visualize_projectiles", "b", "Debug", "0", "-", "-",
		"gui_cursor_is_raytracing", "b", "GUI", "1", "-", "-",
		"camera_default_beta_angle", "i", "Camera", "90", "-", "-",
		"debug_visualize_units", "b", "Debug", "0", "-", "-",
		"debug_visualize_units_extended", "b", "Debug", "0", "-", "-",
#ifdef PROJECT_CLAW_PROTO
		"camera_system_enabled", "b", "Camera", "1", "-", "-",
		"claw_mouse", "b", "Game", "1", "-", "-",
#else
		"camera_system_enabled", "b", "Camera", "0", "-", "-",
		"_reserved_g317", "b", "Reserved", "-", "-", "-",
#endif
		"debug_visualize_units_variable", "s", "Debug", "", "-", "-",
		"debug_visualize_units_of_type", "s", "Debug", "", "-", "-",

		"physics_visualize_ccd", "ba", "Physics", "0", "-", "-",
		"physics_use_ccd", "b", "Physics", "0", "-", "-",
		"physics_ccd_max_thickness", "f", "Physics", "0.5f", "-", "-",
		"render_particle_reflection", "ba+", "Graphics", "0", "-", "-",
		"camera_system_response", "f", "Camera", "10.0f", "-", "-",
#ifdef PROJECT_CLAW_PROTO
		"claw_control_type", "i+", "Controllers", "1", "-", "-",
#else
		"_reserved_c325", "i", "Reserved", "-", "-", "-",
#endif
		"camera_system_debug", "b", "Camera", "0", "-", "-",
#ifdef PROJECT_AOV
		"physics_tick_time", "i", "Physics", "10", "-", "-",
#else
		"physics_tick_time", "i", "Physics", "15", "-", "-",
#endif
		"gui_visualize_windows", "ba", "GUI", "0", "-", "-",
		"render_max_pointlights", "ia+", "Graphics", "5", "-", "-",

		"camera_normal_fov_factor", "f", "Camera", "1.0f", "-", "-",
		"camera_movie_fov_factor", "f", "Camera", "1.0f", "-", "-",
		"magic_screenshot", "b", "Debug", "0", "-", "-",
#ifdef PROJECT_SURVIVOR
		"show_custom_survival_missions", "b+", "GUI", "0", "-", "-",
#else
		"_reserved333", "i", "Reserved", "-", "-", "-",
#endif
		"physics_impact_push_factor", "f", "Physics", "1.0f", "-", "-",

		"multiple_input_devices_enabled", "b+", "Controllers", "0", "-", "-",
		"player1_mouse_ID", "i+", "Controllers", "2", "-", "-",
		"player2_mouse_ID", "i+", "Controllers", "1", "-", "-",
		"player3_mouse_ID", "i+", "Controllers", "-", "-", "-",
		"player4_mouse_ID", "i+", "Controllers", "-", "-", "-",

		"friendly_fire", "b+", "Game", "0", "-", "-",

		"show_splash_screen_advanced", "b", "GUI", "0", "-", "-",
		"physics_water_height", "f", "Physics", "0.0f", "-", "-",
		"physics_water_damping", "f", "Physics", "1.0f", "-", "-",
		"render_sky_bloom", "ba+", "Graphics", "1", "-", "-",

		"debug_visualize_projectiles_extended", "b", "Debug", "0", "-", "-",
		"ambient_volume", "ia+", "Sounds", "80", "-", "-",
		"forcewear_enabled", "b+", "Game", "0", "-", "-",

		"mapview_ambient_lighting", "f", "Graphics", "0.15f", "-", "-",

		"debug_visualize_pointlight_shadow_areas", "b", "Debug", "0", "-", "-",

		"use_reference_driver", "b", "Display", "0", "-", "-",
		"render_only_when_requested", "b", "Display", "0", "-", "-",
		"debug_visualize_obstaclemap", "b", "Debug", "0", "-", "-",
		"use_old_loadgamemenu", "b+", "GUI", "0", "-", "-",
		"gui_widescreen_fixes", "b", "GUI", "0", "-", "-",
		"scorewindow_name_input", "b+", "GUI", "1", "-", "-",
		"force_mission_failure", "b+", "Game", "0", "-", "-",
		"debug_visualize_selections", "b", "Debug", "0", "-", "-",
		"gui_cursor_raytrace_hits_units", "b", "GUI", "0", "-", "-",
		"show_player_pos", "b+", "Debug", "0", "-", "-",

		"maprender_size_multiplier", "i", "Graphics", "8", "-", "-",
		"maprender_use_user_resolution", "b", "Graphics", "0", "-", "-",

		"map_autozoom", "f", "GUI", "2.0f", "-", "-",
		"collect_performance_stats", "b", "Debug", "0", "-", "-",
		"savegame_allow_overwrite", "b", "Game", "1", "-", "-",

		"show_tutorial_hints", "b+", "Game", "1", "-", "-",
		"joystick1_deadzone", "i+", "Controllers", "250", "1000", "-",
		"joystick2_deadzone", "i+", "Controllers", "250", "1000", "-",
		"joystick3_deadzone", "i+", "Controllers", "250", "1000", "-",
		"joystick4_deadzone", "i+", "Controllers", "250", "1000", "-",

		// first fill the _reserved_ options with something useful
		// then add more if necessary..

		"***", "***", "***", "***", "***", "***",
	};
	
	std::auto_ptr<GameOptionManager> GameOptionManager::instance;


	GameOptionManager *GameOptionManager::getInstance()
	{
		if (!instance.get())
			instance.reset(new GameOptionManager(GameConfigs::getInstance()));
		return instance.get();
	}


	void GameOptionManager::cleanInstance()
	{
		instance.reset();
	}


	GameOptionManager::GameOptionManager(GameConfigs *gameConfigs)
	{
		this->gameConf = gameConfigs;
		this->options = new LinkedList();
		for (int i = 0; i < DH_OPT_AMOUNT; i++)
		{
			optionsById[i] = NULL;
		}
	}


	GameOptionManager::~GameOptionManager()
	{
		// hack hack temp temp, should this method call be moved to disposable.cpp?!
		save();

		while (!options->isEmpty())
		{
			GameOption *tmp = (GameOption *)options->popLast();
			delete tmp;
		}
		delete options;
	}

	
	void GameOptionManager::load()
	{
#ifdef _DEBUG
		if (gameOptions[DH_OPT_AMOUNT * GAMEOPTIONS_PARAMS] == NULL
		  || strcmp(gameOptions[DH_OPT_AMOUNT * GAMEOPTIONS_PARAMS], "***") != 0
		  || strcmp(gameOptions[DH_OPT_AMOUNT * GAMEOPTIONS_PARAMS + 1], "***") != 0
		  || strcmp(gameOptions[DH_OPT_AMOUNT * GAMEOPTIONS_PARAMS + 2], "***") != 0)
		{
			assert(!"GameOptionManager::load - gameOptions list invalid!");
			return;
		}
#endif

#ifdef _DEBUG
		Logger::getInstance()->setLogLevel(LOGGER_LEVEL_DEBUG);
#endif

		// TODO: profile specific options (overriding even Config/options.txt)
		// (for inva-mouse, sound volumes, etc.)

		//Parser::Parser options_config("Config/options.txt");
		//Parser::Parser def_options_config("Data/Misc/default_game_options.txt");
		//Parser::Parser locked_options_config("Data/Misc/locked_game_options.txt");

		editor::EditorParser options_config(true, false);
		editor::EditorParser def_options_config(true, false);
		editor::EditorParser locked_options_config(true, false);
#ifdef LEGACY_FILES
		filesystem::InputStream options_file = filesystem::FilePackageManager::getInstance().getFile(igios_mapUserDataPrefix("Config/options.txt"));
		options_file >> options_config;

		filesystem::InputStream def_options_file = filesystem::FilePackageManager::getInstance().getFile("Data/Misc/default_game_options.txt");
		def_options_file >> def_options_config;

		filesystem::InputStream locked_options_file = filesystem::FilePackageManager::getInstance().getFile("Data/Misc/locked_game_options.txt");
		locked_options_file >> locked_options_config;
#else
		editor::EditorParser dev_options_config(true, false);

		filesystem::InputStream options_file = filesystem::FilePackageManager::getInstance().getFile(igios_mapUserDataPrefix("config/options.txt"));
		options_file >> options_config;

		filesystem::InputStream def_options_config = filesystem::FilePackageManager::getInstance().getFile("data/misc/default_game_options.txt");
		def_options_file >> def_options_config;

		filesystem::InputStream locked_options_file = filesystem::FilePackageManager::getInstance().getFile("data/misc/locked_game_options.txt");
		locked_options_file >> locked_options_config;

		FILE *devf = fopen("config/dev/dev_options.txt", "rb");
		if (devf != NULL)
		{
			fclose(devf);
			filesystem::FilePackageManager::getInstance().getFile("config/dev/dev_options.txt") >> dev_options_config;

			Logger::getInstance()->info("Developer options file found, using it to override option values.");
		}
#endif

		// TODO: this is not at all effective...
		for (int i = 0; i < DH_OPT_AMOUNT; i++)
		{
			const char *grpName = gameOptions[i * GAMEOPTIONS_PARAMS + 2];
			//const Parser::string_map user_props = options_config.FindGroup(grpName).GetProperties();
			//const Parser::string_map def_props = def_options_config.FindGroup(grpName).GetProperties();
			//const Parser::string_map locked_props = locked_options_config.FindGroup(grpName).GetProperties();

			const editor::ParserGroup &user_props = options_config.getGlobals().getSubGroup(grpName);
			const editor::ParserGroup &def_props = def_options_config.getGlobals().getSubGroup(grpName);
			const editor::ParserGroup &locked_props = locked_options_config.getGlobals().getSubGroup(grpName);
#ifdef LEGACY_FILES
			// no dev options
#else
			const editor::ParserGroup &dev_props = dev_options_config.getGlobals().getSubGroup(grpName);
#endif

			// the props to use
			const editor::ParserGroup *props = NULL;
			bool isLocked = false;
			bool hasValue = true;

#ifdef LEGACY_FILES
			if (false)
			{
				// nop
#else
			if (parser::hasProperty(dev_props, gameOptions[i * GAMEOPTIONS_PARAMS]))
			{
				props = &dev_props;
				// (get rid of unreferenced warnings...)
				bool dummy;
				dummy = parser::hasProperty(locked_props, gameOptions[i * GAMEOPTIONS_PARAMS]);
				dummy = parser::hasProperty(user_props, gameOptions[i * GAMEOPTIONS_PARAMS]);
				dummy = parser::hasProperty(def_props, gameOptions[i * GAMEOPTIONS_PARAMS]);
#endif
			} else {
				if (parser::hasProperty(locked_props, gameOptions[i * GAMEOPTIONS_PARAMS]))
				{
					props = &locked_props;
					isLocked = true;

					// (get rid of unreferenced warnings...)
					bool dummy;
					dummy = parser::hasProperty(user_props, gameOptions[i * GAMEOPTIONS_PARAMS]);
					dummy = parser::hasProperty(def_props, gameOptions[i * GAMEOPTIONS_PARAMS]);
				} else {
					if (parser::hasProperty(user_props, gameOptions[i * GAMEOPTIONS_PARAMS]))
					{
						props = &user_props;

						// (get rid of unreferenced warnings...)
						bool dummy;
						dummy = parser::hasProperty(def_props, gameOptions[i * GAMEOPTIONS_PARAMS]);
					} else {
						if (parser::hasProperty(def_props, gameOptions[i * GAMEOPTIONS_PARAMS]))
						{
							props = &def_props;
						} else {
							props = &def_props;
							// if the property is missing from all, debug log it.
							// except for Cheats and Reserved ;)
							if (strcmp(gameOptions[i * GAMEOPTIONS_PARAMS + 2], "Cheats") != 0
								&& strcmp(gameOptions[i * GAMEOPTIONS_PARAMS + 2], "Reserved") != 0)
							{
								hasValue = false;
								Logger::getInstance()->debug("GameOptionManager::load - Property not found.");
								Logger::getInstance()->debug(gameOptions[i * GAMEOPTIONS_PARAMS]);
							}
						}
					}
				}
			}

			
			char typeChar = gameOptions[i * GAMEOPTIONS_PARAMS + GAMEOPTIONS_PARAM_TYPE][0];

			if (typeChar == 'b')
			{
				// boolean type

				GameOption *go = new GameOption(this, i);
				options->append(go);
				optionsById[i] = go;

				bool value = false;
				if (!hasValue)
				{
					int intVal = str2int(gameOptions[i * GAMEOPTIONS_PARAMS + GAMEOPTIONS_PARAM_DEFAULTVALUE]);
					if (intVal == 1)
						value = true;
					else
						value = false;
				} else {
					if (parser::getInt(*props, gameOptions[i * GAMEOPTIONS_PARAMS]) == 1)
						value = true;
				}
				GameConfigs::getInstance()->addBoolean(gameOptions[i * GAMEOPTIONS_PARAMS], value, i);
			}
			else if (typeChar == 'i')
			{
				// int type

				GameOption *go = new GameOption(this, i);
				options->append(go);
				optionsById[i] = go;

				int value = 0;
				if (!hasValue)
				{
					value = str2int(gameOptions[i * GAMEOPTIONS_PARAMS + GAMEOPTIONS_PARAM_DEFAULTVALUE]);
				} else {
					value = parser::getInt(*props, gameOptions[i * GAMEOPTIONS_PARAMS]);
				}
				GameConfigs::getInstance()->addInt(gameOptions[i * GAMEOPTIONS_PARAMS], value, i);				
			}
			else if (typeChar == 'f')
			{
				// float type

				GameOption *go = new GameOption(this, i);
				options->append(go);
				optionsById[i] = go;

				float value = 0;
				if (!hasValue)
				{
					value = (float)atof(gameOptions[i * GAMEOPTIONS_PARAMS + GAMEOPTIONS_PARAM_DEFAULTVALUE]);
				} else {
					value = parser::getFloat(*props, gameOptions[i * GAMEOPTIONS_PARAMS]);
				}

				GameConfigs::getInstance()->addFloat(gameOptions[i * GAMEOPTIONS_PARAMS], value, i);				
			}
			else if (typeChar == 's')
			{
				// string type

				GameOption *go = new GameOption(this, i);
				options->append(go);
				optionsById[i] = go;

				std::string valstr;
				const char *value = "";
				if (!hasValue)
				{
					value = gameOptions[i * GAMEOPTIONS_PARAMS + GAMEOPTIONS_PARAM_DEFAULTVALUE];
					if (strcmp(value, "-") == 0)
					{
						value = "";
					}
				} else {
					valstr = parser::getString(*props, gameOptions[i * GAMEOPTIONS_PARAMS]);
					value = valstr.c_str();
				}

				GameConfigs::getInstance()->addString(gameOptions[i * GAMEOPTIONS_PARAMS], value, i);	
			} else {
				assert(!"GameOptionManager::load - Option type invalid.");
			}
			if (optionsById[i] != NULL)
			{
				if (isLocked)
				{
					optionsById[i]->makeReadOnly();
				}
			}
		}
	}

	
	void GameOptionManager::save()
	{
		// implentation created by Pete,

		// TODO
		// NOTE: should save those options that are different from default values (the
		//       values read from default_options.txt) to options.txt...
		// NOTE: but should save profile specific options to profile directory though, which
		//       means those values that were read from the profile directory.

		editor::EditorParser options_config;//("Config/options.txt");

		for (int i = 0; i < DH_OPT_AMOUNT; i++)
		{
			std::string type = gameOptions[i * GAMEOPTIONS_PARAMS + GAMEOPTIONS_PARAM_TYPE];
			
			// check if we should save the poor bastard
			if( !type.empty() && type[ type.size() - 1 ] == '+' )
			{
				std::string grpName = gameOptions[i * GAMEOPTIONS_PARAMS + 2];
			
				editor::ParserGroup& group = options_config.getGlobals().getSubGroup( grpName );

				group.setValue( gameOptions[i * GAMEOPTIONS_PARAMS], getAsStringValue( optionsById[ i ] ) );
			}
		}

		std::fstream o;
#ifdef LEGACY_FILES
		o.open( igios_mapUserDataPrefix("Config/options.txt").c_str(), std::ios::out );
#else
		o.open( igios_mapUserDataPrefix("config/options.txt").c_str(), std::ios::out );
#endif

		o << options_config;
		o.close();
		// filesystem::FilePackageManager::getInstance().getFile("Config/options.txt") << options_config;

		// assert(0);
	}


	GameOption *GameOptionManager::getOptionByName(const char *name)
	{
		for (int i = 0; i < DH_OPT_AMOUNT; i++)
		{
			if (strcmp(gameOptions[i * GAMEOPTIONS_PARAMS], name) == 0)
			{
				return optionsById[i];
			}
		}

		return NULL;
	}


	GameOption *GameOptionManager::getOptionById(int id)
	{
		assert(id >= 0 && id < DH_OPT_AMOUNT);
		return optionsById[id];
	}


	const LinkedList *GameOptionManager::getOptionsList()
	{
		return options;
	}


	const char *GameOptionManager::getOptionNameForId(int id)
	{
		assert(id >= 0 && id < DH_OPT_AMOUNT);
		return gameOptions[id * GAMEOPTIONS_PARAMS];
	}


	void GameOptionManager::setIntOptionValue(int id, int value)
	{
		if (optionsById[id] != NULL)
		{
			if (optionsById[id]->isReadOnly())
			{
				Logger::getInstance()->warning("Cannot change a read-only option value.");
				return;
			}
		}
		gameConf->setInt(id, value);
	}


	void GameOptionManager::setBoolOptionValue(int id, bool value)
	{
		if (optionsById[id] != NULL)
		{
			if (optionsById[id]->isReadOnly())
			{
				Logger::getInstance()->warning("Cannot change a read-only option value.");
				return;
			}
		}
		gameConf->setBoolean(id, value);
	}


	void GameOptionManager::setFloatOptionValue(int id, float value)
	{
		if (optionsById[id] != NULL)
		{
			if (optionsById[id]->isReadOnly())
			{
				Logger::getInstance()->warning("Cannot change a read-only option value.");
				return;
			}
		}
		gameConf->setFloat(id, value);
	}


	void GameOptionManager::setStringOptionValue(int id, const char *value)
	{
		if (optionsById[id] != NULL)
		{
			if (optionsById[id]->isReadOnly())
			{
				Logger::getInstance()->warning("Cannot change a read-only option value.");
				return;
			}
		}
		gameConf->setString(id, value);
	}


	int GameOptionManager::getIntOptionValue(int id)
	{
		return gameConf->getInt(id);
	}


	bool GameOptionManager::getBoolOptionValue(int id)
	{
		return gameConf->getBoolean(id);
	}


	float GameOptionManager::getFloatOptionValue(int id)
	{
		return gameConf->getFloat(id);
	}


	char *GameOptionManager::getStringOptionValue(int id)
	{
		return gameConf->getString(id);
	}


	IScriptVariable::VARTYPE GameOptionManager::getOptionVariableType(int id)
	{
		if (gameOptions[id * GAMEOPTIONS_PARAMS + GAMEOPTIONS_PARAM_TYPE][0] == 'b')
		{
			return IScriptVariable::VARTYPE_BOOLEAN;
		} 
		else if (gameOptions[id * GAMEOPTIONS_PARAMS + GAMEOPTIONS_PARAM_TYPE][0] == 'i')
		{
			return IScriptVariable::VARTYPE_INT;
		}
		else if (gameOptions[id * GAMEOPTIONS_PARAMS + GAMEOPTIONS_PARAM_TYPE][0] == 'f')
		{
			return IScriptVariable::VARTYPE_FLOAT;
		}
		else if (gameOptions[id * GAMEOPTIONS_PARAMS + GAMEOPTIONS_PARAM_TYPE][0] == 's')
		{
			return IScriptVariable::VARTYPE_STRING;
		}

		// what now? we've bugged.
		assert(0);
		return IScriptVariable::VARTYPE_BOOLEAN;
	}

	// Added by Pete, used in the saving progress to save a variables value
	std::string GameOptionManager::getAsStringValue( GameOption* conf )
	{
		std::stringstream ss;

		if( conf->getVariableType() == IScriptVariable::VARTYPE_BOOLEAN )
		{
			ss << conf->getBooleanValue();
		}
		else if( conf->getVariableType() == IScriptVariable::VARTYPE_INT )
		{
			ss << conf->getIntValue();
		}
		else if( conf->getVariableType() == IScriptVariable::VARTYPE_FLOAT )
		{
			ss << conf->getFloatValue();
		}
		else if( conf->getVariableType() == IScriptVariable::VARTYPE_STRING )
		{
			ss << conf->getStringValue();
		}

		return ss.str();
	}



	bool GameOptionManager::isOptionToggleable(int id)
	{
		if (gameOptions[id * GAMEOPTIONS_PARAMS + GAMEOPTIONS_PARAM_TOGGLESTEP][0] != '-'
			|| getOptionVariableType(id) == IScriptVariable::VARTYPE_BOOLEAN)
		{
			return true;
		} else {
			return false;
		}
	}


	void GameOptionManager::resetOptionValue(int id)
	{
		if (optionsById[id] != NULL)
		{
			if (optionsById[id]->isReadOnly())
			{
				Logger::getInstance()->warning("Cannot change a read-only option value.");
				return;
			}
		}
		// note: simply checking the first char is not enough because the value may be negative!
		bool hasDefaultValue = strcmp(gameOptions[id * GAMEOPTIONS_PARAMS + GAMEOPTIONS_PARAM_DEFAULTVALUE], "-") == 0;
		if (!hasDefaultValue)
		{
			if (getOptionVariableType(id) == IScriptVariable::VARTYPE_BOOLEAN)
			{
				int val = str2int(gameOptions[id * GAMEOPTIONS_PARAMS + GAMEOPTIONS_PARAM_DEFAULTVALUE]);
				if (val != 0)
					gameConf->setBoolean(id, true);
				else
					gameConf->setBoolean(id, false);
			}
			if (getOptionVariableType(id) == IScriptVariable::VARTYPE_INT)
			{
				int val = str2int(gameOptions[id * GAMEOPTIONS_PARAMS + GAMEOPTIONS_PARAM_DEFAULTVALUE]);
				gameConf->setInt(id, val);
			}
			if (getOptionVariableType(id) == IScriptVariable::VARTYPE_FLOAT)
			{
				float val = (float)atof(gameOptions[id * GAMEOPTIONS_PARAMS + GAMEOPTIONS_PARAM_DEFAULTVALUE]);
				gameConf->setFloat(id, val);
			}
			if (getOptionVariableType(id) == IScriptVariable::VARTYPE_STRING)
			{
				gameConf->setString(id, gameOptions[id * GAMEOPTIONS_PARAMS + GAMEOPTIONS_PARAM_DEFAULTVALUE]);
			}
		} else {
			// nop
			// might want to warn about this??
		}
	}


	void GameOptionManager::toggleOptionValue(int id)
	{
		if (optionsById[id] != NULL)
		{
			if (optionsById[id]->isReadOnly())
			{
				Logger::getInstance()->warning("Cannot change a read-only option value.");
				return;
			}
		}
		if (isOptionToggleable(id))
		{
			if (getOptionVariableType(id) == IScriptVariable::VARTYPE_BOOLEAN)
			{
				bool val = gameConf->getBoolean(id);
				gameConf->setBoolean(id, !val);
			}
			if (getOptionVariableType(id) == IScriptVariable::VARTYPE_INT)
			{
				int val = gameConf->getInt(id);
				int tstep = str2int(gameOptions[id * GAMEOPTIONS_PARAMS + GAMEOPTIONS_PARAM_TOGGLESTEP]);
				int maxval = str2int(gameOptions[id * GAMEOPTIONS_PARAMS + GAMEOPTIONS_PARAM_MAXVALUE]);
				val += tstep;
				if (val > maxval)
					val = 0;
				gameConf->setInt(id, val);
			}
			if (getOptionVariableType(id) == IScriptVariable::VARTYPE_FLOAT)
			{
				float val = gameConf->getFloat(id);
				float tstep = (float)atof(gameOptions[id * GAMEOPTIONS_PARAMS + GAMEOPTIONS_PARAM_TOGGLESTEP]);
				float maxval = (float)atof(gameOptions[id * GAMEOPTIONS_PARAMS + GAMEOPTIONS_PARAM_MAXVALUE]);
				val += tstep;
				if (val > maxval)
					val = 0.0f;
				gameConf->setFloat(id, val);
			}
			
		}
	}


	bool GameOptionManager::doesOptionNeedApply(int id)
	{
		const char *typeStr = gameOptions[id * GAMEOPTIONS_PARAMS + GAMEOPTIONS_PARAM_TYPE];

		if (typeStr[1] != '\0')
		{
			if (typeStr[1] == 'a' || typeStr[2] == 'a')
				return true;
		}

		return false;
	}


}

