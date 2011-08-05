// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "object_mode.h"
#include "gui.h"
#include "dialog.h"
#include "dialog_utils.h"
#include "mouse.h"
#include "camera.h"
#include "storm.h"
#include "storm_geometry.h"
#include "storm_model_utils.h"
#include "terrain_objects.h"
#include "icommand.h"
#include "command_list.h"
#include "common_dialog.h"
#include "istream.h"
#include "ostream.h"
#include "object_settings.h"
#include "ieditor_state.h"
#include "color_picker.h"
#include "file_wrapper.h"
#include "explosion_scripts.h"
#include "terrain_colormap.h"
#include "terrain_lightmap.h"
#include "group_list.h"
#include "group_list_utils.h"
#include "group_save_dialog.h"
#include "physics_mass.h"
#include "../ui/lightmanager.h"
#include "../util/SoundMaterialParser.h"
#include "../util/ObjectDurabilityParser.h"
#include "../filesystem/input_stream.h"
#include "../filesystem/output_stream.h"
#include "resource/resource.h"

// HACK: hack hack...
#include "light_mode.h"

#include <istorm3d.h>
#include <istorm3d_model.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <stdlib.h>
#include <commctrl.h>

namespace frozenbyte {
namespace editor {
namespace {
	struct ModeData
	{
		int currentMode;
		int changeToMode;
		bool rebuildModels;
		bool updateGroups;

		ModeData()
		{
			reset();
		}

		void reset()
		{
			changeToMode = 0;
			currentMode = 0;
			rebuildModels = false;
			updateGroups = false;
		}
	};

	struct SharedData
	{
		Gui &gui;
		Dialog &dialog;
		Storm &storm;
		Dialog &menu;
		IEditorState &editorState;

		ModeData modeData;
		TerrainObjects terrainObjects;

		bool noUpdate;

		std::map<int, bool> allowUpdatingItem;
		FileWrapper fileWrapper;
		int groupIndex;
		int subGroupIndex;
		int groupGroupIndex;
		int groupSubGroupIndex;

		GroupList list;
		ExplosionScripts explosionScripts;
		PhysicsMass physicsMass;

		SharedData(Gui &gui_, Storm &storm_, IEditorState &editorState_)
		:	gui(gui_),
			dialog(gui_.getObjectsDialog()), 
			storm(storm_),
			menu(gui_.getMenuDialog()),
			editorState(editorState_),
			terrainObjects(storm, editorState),

#ifdef LEGACY_FILES
			fileWrapper("Data\\Models\\Terrain_objects", "*.s3d", true),
#else
			fileWrapper("data\\model\\object", "*.s3d"),
#endif
			groupIndex(0),
			subGroupIndex(0),
			groupGroupIndex(0),
			groupSubGroupIndex(0)
		{
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_TYPE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("No type set"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_TYPE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("No collision"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_TYPE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Cylinder collision"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_TYPE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Box collision"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_TYPE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Mapped collision"));

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_FALL_TYPE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Static"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_FALL_TYPE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Tree"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_FALL_TYPE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Plant"));

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_AMOUNT, TBM_SETRANGE, TRUE, MAKELONG(1, 5));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_AMOUNT, TBM_SETRANGE, TRUE, MAKELONG(1, 10));

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_BREAK_TEX, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("None"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_BREAK_TEX, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Scale HP"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_BREAK_TEX, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Always"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_BREAK_TEX, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Scriptable"));

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_PHYSICS_TYPE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("No physics"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_PHYSICS_TYPE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Static mesh"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_PHYSICS_TYPE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Dynamic box"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_PHYSICS_TYPE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Dynamic cylinder"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_PHYSICS_TYPE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Dynamic capsule"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_PHYSICS_TYPE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Rack"));
#ifdef PROJECT_CLAW_PROTO
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_PHYSICS_TYPE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Car"));
#endif

			for(int i = 0; i < physicsMass.getMassAmount(); ++i)
			{
				std::string name = physicsMass.getMassName(i);
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_PHYSICS_MASS, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> (name.c_str()));
			}

			util::SoundMaterialParser soundMaterialParser;
			for(int i = 0; i < soundMaterialParser.getMaterialAmount(); ++i)
			{
				const std::string &name = soundMaterialParser.getMaterialName(i);
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_PHYSICS_MATERIAL, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> (name.c_str()));
			}

			util::ObjectDurabilityParser objectDurabilityParser;
			for(int i = 0; i < objectDurabilityParser.getDurabilityTypeAmount(); ++i)
			{
				const std::string &name = objectDurabilityParser.getDurabilityTypeName(i);
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_PHYSICS_DURABILITY, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> (name.c_str()));
			}

			std::vector<std::string> metaKeys = terrainObjects.getObjectSettings().getMetaKeys();
			for(unsigned int i = 0; i < metaKeys.size(); ++i)
			{
				const std::string &name = metaKeys[i];
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_ID, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> (name.c_str()));
			}

			reset();
		}

		void updateDialog()
		{
		}

		void updateModelList()
		{
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_OBJECT, CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_OBJECT, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("(none)"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_OBJECT, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("(disappear)"));

			explosionScripts.reload();
			{
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_SCRIPT, CB_RESETCONTENT, 0, 0);
				const ScriptList &scripts = explosionScripts.getScripts();

				for(ScriptList::const_iterator it = scripts.begin(); it != scripts.end(); ++it)
				{
					const char *str = it->name.c_str();
					SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_SCRIPT, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> (str));
				}

				SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_SCRIPT, LB_SETCURSEL, 0, 0);
			}
			{
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_PROJECTILE, CB_RESETCONTENT, 0, 0);
				const ProjectileList &projectiles = explosionScripts.getProjectiles();

				for(ProjectileList::const_iterator it = projectiles.begin(); it != projectiles.end(); ++it)
				{
					const char *str = it->name.c_str();
					SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_PROJECTILE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> (str));
				}

				SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_PROJECTILE, LB_SETCURSEL, 0, 0);
			}
			{
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_EFFECT, CB_RESETCONTENT, 0, 0);
				const EffectList &effects = explosionScripts.getEffects();

				for(EffectList::const_iterator it = effects.begin(); it != effects.end(); ++it)
				{
					const char *str = it->name.c_str();
					SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_EFFECT, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> (str));
				}

				SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_EFFECT, LB_SETCURSEL, 0, 0);
			}
			{
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_SOUND, CB_RESETCONTENT, 0, 0);
				const SoundList &sounds = explosionScripts.getSounds();

				for(SoundList::const_iterator it = sounds.begin(); it != sounds.end(); ++it)
				{
					const char *str = it->name.c_str();
					SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_SOUND, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> (str));
				}

				SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_SOUND, LB_SETCURSEL, 0, 0);
			}
			{
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_MATERIAL, CB_RESETCONTENT, 0, 0);
				const MaterialList &materials = explosionScripts.getMaterials();

				for(MaterialList::const_iterator it = materials.begin(); it != materials.end(); ++it)
				{
					const char *str = it->name.c_str();
					SendDlgItemMessage(dialog.getWindowHandle(), IDC_MATERIAL, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> (str));
				}

				SendDlgItemMessage(dialog.getWindowHandle(), IDC_MATERIAL, LB_SETCURSEL, 0, 0);
			}
			{
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_ANIMATION, CB_RESETCONTENT, 0, 0);
				const AnimationList &animations = explosionScripts.getAnimations();

				for(AnimationList::const_iterator it = animations.begin(); it != animations.end(); ++it)
				{
					const char *str = it->name.c_str();
					SendDlgItemMessage(dialog.getWindowHandle(), IDC_ANIMATION, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> (str));
				}

				SendDlgItemMessage(dialog.getWindowHandle(), IDC_ANIMATION, LB_SETCURSEL, 0, 0);
			}

			if(noUpdate)
				return;

			noUpdate = true;

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_GROUP, CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_SUBGROUP, CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_MODELS, LB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_SUBGROUP, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("All"));

			for(int i = 0; i < fileWrapper.getRootDirAmount(); ++i)
			{
				const std::string &dir = fileWrapper.getRootDir(i);
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_GROUP, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> (dir.c_str()));

				if(i == groupIndex)
				for(int j = 0; j < fileWrapper.getDirAmount(i); ++j)
				{
					const std::string &dir = fileWrapper.getDir(i, j);
					SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_SUBGROUP, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> (dir.c_str()));

					// Tmp tmp
					if(subGroupIndex == 0 || j == subGroupIndex - 1)
					{
						for(int k = 0; k < fileWrapper.getFileAmount(i, j); ++k)
						{
							std::string file = getFileName(fileWrapper.getFile(i, j, k));
							
							SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_MODELS, LB_ADDSTRING, 0, reinterpret_cast<LPARAM> (file.c_str()));
							SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_OBJECT, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> (file.c_str()));
						}

						SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_MODELS, LB_SETCURSEL, 0, 0);
					}
				}
			}

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_GROUP, CB_SETCURSEL, groupIndex, 0);
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_SUBGROUP, CB_SETCURSEL, subGroupIndex, 0);
			noUpdate = false;
		}

		void updateGroupList()
		{
			if(noUpdate)
				return;

			noUpdate = true;

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_GROUP_GROUP, CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_GROUP_SUBGROUP, CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_GROUP_LIST, LB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_GROUP_SUBGROUP, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("All"));

			list.reload();

			// Add groups
			{
				for(int i = 0; i < list.getGroupAmount(); ++i)
				{
					std::string name = list.getGroupName(i);
					SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_GROUP_GROUP, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> (name.c_str()));
				}
			}

			// Add subgroups
			{
				for(int i = 0; i < list.getSubgroupAmount(groupGroupIndex); ++i)
				{
					std::string name = list.getSubgroupName(groupGroupIndex, i);
					SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_GROUP_SUBGROUP, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> (name.c_str()));
				}
			}

			int groupMin = groupSubGroupIndex - 1;
			int groupMax = groupSubGroupIndex;
			if(groupMin < 0)
			{
				groupMin = 0;
				groupMax = list.getSubgroupAmount(groupGroupIndex);
			}

			// Add object groups
			{
				for(int i = groupMin; i < groupMax; ++i)
				{
					for(int j = 0; j < list.getObjectGroupAmount(groupGroupIndex, i); ++j)
					{
						const GroupList::ObjectGroup &grp = list.getObjectGroup(groupGroupIndex, i, j);
						SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_GROUP_LIST, LB_ADDSTRING, 0, reinterpret_cast<LPARAM> (grp.name.c_str()));
					}
				}
			}

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_GROUP_GROUP, CB_SETCURSEL, groupGroupIndex, 0);
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_GROUP_SUBGROUP, CB_SETCURSEL, groupSubGroupIndex, 0);

			modeData.updateGroups = true;
			noUpdate = false;
		}

		struct ItemState
		{
			ItemState() { cleared = false; }
			bool cleared;
			int intvalue;
			std::string stringvalue;
		};

		struct ItemStateList
		{
			std::vector<ItemState> states;
			bool all_cleared;
			bool first_run;
		};

		inline void updateMultiSelection_Combo(int i, int id, ItemStateList &statelist)
		{
	    if(statelist.states[i].cleared)
			{
				SendDlgItemMessage(dialog.getWindowHandle(), id, CB_SETCURSEL, -1, 0);
				return;
			}

			int value = SendDlgItemMessage(dialog.getWindowHandle(), id, CB_GETCURSEL, 0, 0);
			if(statelist.first_run)
			{
				statelist.states[i].intvalue = value;
			}
			else if(statelist.states[i].intvalue != value)
			{
				SendDlgItemMessage(dialog.getWindowHandle(), id, CB_SETCURSEL, -1, 0);
				allowUpdatingItem[id] = false;
				statelist.states[i].cleared = true;
			}
			else
			{
				statelist.all_cleared = false;
			}
		}

		inline void updateMultiSelection_Edit(int i, int id, ItemStateList &statelist)
		{
	    if(statelist.states[i].cleared)
			{
				SendDlgItemMessage(dialog.getWindowHandle(), id, WM_SETTEXT, 0, 0);
				return;
			}

			std::string value = getDialogItemText(dialog, id);
			if(statelist.first_run)
			{
				statelist.states[i].stringvalue = value;
			}
			else if(statelist.states[i].stringvalue != value)
			{
				SendDlgItemMessage(dialog.getWindowHandle(), id, WM_SETTEXT, 0, 0);
				allowUpdatingItem[id] = false;
				statelist.states[i].cleared = true;
			}
			else
			{
				statelist.all_cleared = false;
			}
		}

		inline void updateMultiSelection_CheckBox(int i, int id, ItemStateList &statelist)
		{
	    if(statelist.states[i].cleared)
			{
				CheckDlgButton(dialog.getWindowHandle(), id, BST_INDETERMINATE);
				return;
			}

			int value = IsDlgButtonChecked(dialog.getWindowHandle(), id);
			if(statelist.first_run)
			{
				statelist.states[i].intvalue = value;
			}
			else if(statelist.states[i].intvalue != value)
			{
				CheckDlgButton(dialog.getWindowHandle(), id, BST_INDETERMINATE);
				allowUpdatingItem[id] = false;
				statelist.states[i].cleared = true;
			}
			else
			{
				statelist.all_cleared = false;
			}
		}
		bool updateMultiSelection(ItemStateList &statelist)
		{
			noUpdate = true;
			statelist.all_cleared = true;

			statelist.states.resize(20);

			updateMultiSelection_Combo(0, IDC_OBJECT_TYPE, statelist);
			updateMultiSelection_Combo(1, IDC_FALL_TYPE, statelist);
			updateMultiSelection_Edit(2, IDC_OBJECT_HEIGHT, statelist);
			updateMultiSelection_Edit(3, IDC_OBJECT_XSIZE, statelist);
			updateMultiSelection_Edit(4, IDC_OBJECT_ZSIZE, statelist);
			updateMultiSelection_CheckBox(5, IDC_OBJECT_FIRETHROUGH, statelist);
			updateMultiSelection_Combo(6, IDC_EXPLOSION_OBJECT, statelist);
			updateMultiSelection_Combo(7, IDC_EXPLOSION_SCRIPT, statelist);
			updateMultiSelection_Combo(8, IDC_EXPLOSION_PROJECTILE, statelist);
			updateMultiSelection_Combo(9, IDC_EXPLOSION_EFFECT, statelist);
			updateMultiSelection_Combo(10, IDC_EXPLOSION_SOUND, statelist);
			updateMultiSelection_Combo(11, IDC_MATERIAL, statelist);
			updateMultiSelection_Combo(12, IDC_ANIMATION, statelist);
			updateMultiSelection_Combo(13, IDC_OBJECT_BREAK_TEX, statelist);
			updateMultiSelection_Combo(14, IDC_PHYSICS_TYPE, statelist);
			updateMultiSelection_Combo(15, IDC_PHYSICS_MASS, statelist);
			updateMultiSelection_Combo(16, IDC_PHYSICS_MATERIAL, statelist);
			updateMultiSelection_Combo(17, IDC_PHYSICS_DURABILITY, statelist);
			updateMultiSelection_Edit(18, IDC_OBJECT_VALUE, statelist);
			updateMultiSelection_Edit(19, IDC_EXPLOSION_HP, statelist);
			// when adding new ones, remember to update resize count above these

			noUpdate = false;
			return statelist.all_cleared && !statelist.first_run;
		}

		void setProperties(const std::string &fileName)
		{
			noUpdate = true;
			ObjectData &data = terrainObjects.getObjectSettings().getSettings(fileName);

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_TYPE, CB_SETCURSEL, data.type + 1, 0);
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_FALL_TYPE, CB_SETCURSEL, data.fallType, 0);

			setDialogItemFloat(dialog, IDC_OBJECT_HEIGHT, data.height);
			setDialogItemFloat(dialog, IDC_OBJECT_XSIZE, data.radiusX);
			setDialogItemFloat(dialog, IDC_OBJECT_ZSIZE, data.radiusZ);

			if(data.fireThrough)
				CheckDlgButton(dialog.getWindowHandle(), IDC_OBJECT_FIRETHROUGH, BST_CHECKED);
			else
				CheckDlgButton(dialog.getWindowHandle(), IDC_OBJECT_FIRETHROUGH, BST_UNCHECKED);

			if(data.explosionObject.empty())
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_OBJECT, CB_SETCURSEL, 0, 0);
			else if(data.explosionObject == "(disappear)")
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_OBJECT, CB_SETCURSEL, 1, 0);
			else
			{
				std::string file = getFileName(data.explosionObject);
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_OBJECT, CB_SELECTSTRING, 0, reinterpret_cast<LPARAM> (file.c_str()));
			}

			{
				const std::string &script = data.explosionScript;
				if(!script.empty())
					SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_SCRIPT, CB_SELECTSTRING, 0, (LPARAM) script.c_str());
				else
					SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_SCRIPT, CB_SETCURSEL, 0, 0);

				const std::string &projectile = data.explosionProjectile;
				if(!projectile.empty())
					SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_PROJECTILE, CB_SELECTSTRING, 0, (LPARAM) projectile.c_str());
				else
					SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_PROJECTILE, CB_SETCURSEL, 0, 0);

				const std::string &effect = data.explosionEffect;
				if(!effect.empty())
					SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_EFFECT, CB_SELECTSTRING, 0, (LPARAM) effect.c_str());
				else
					SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_EFFECT, CB_SETCURSEL, 0, 0);

				const std::string &sound = data.explosionSound;
				if(!sound.empty())
					SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_SOUND, CB_SELECTSTRING, 0, (LPARAM) sound.c_str());
				else
					SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_SOUND, CB_SETCURSEL, 0, 0);

				const std::string &material = data.material;
				if(!material.empty())
					SendDlgItemMessage(dialog.getWindowHandle(), IDC_MATERIAL, CB_SELECTSTRING, 0, (LPARAM) material.c_str());
				else
					SendDlgItemMessage(dialog.getWindowHandle(), IDC_MATERIAL, CB_SETCURSEL, 0, 0);

				const std::string &animation = data.animation;
				if(!animation.empty())
					SendDlgItemMessage(dialog.getWindowHandle(), IDC_ANIMATION, CB_SELECTSTRING, 0, (LPARAM) animation.c_str());
				else
					SendDlgItemMessage(dialog.getWindowHandle(), IDC_ANIMATION, CB_SETCURSEL, 0, 0);
			}

			setDialogItemInt(dialog, IDC_EXPLOSION_HP, data.hitpoints);
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_BREAK_TEX, CB_SETCURSEL, data.breakTexture, 0);
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_PHYSICS_TYPE, CB_SETCURSEL, data.physicsType, 0);

			int massIndex = physicsMass.getMassIndex(data.physicsWeight);
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_PHYSICS_MASS, CB_SETCURSEL, massIndex, 0);

			if(!data.physicsMaterial.empty())
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_PHYSICS_MATERIAL, CB_SELECTSTRING, 0, (LPARAM) data.physicsMaterial.c_str());
			else
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_PHYSICS_MATERIAL, CB_SETCURSEL, 0, 0);

			if(!data.durabilityType.empty())
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_PHYSICS_DURABILITY, CB_SELECTSTRING, 0, (LPARAM) data.durabilityType.c_str());
			else
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_PHYSICS_DURABILITY, CB_SETCURSEL, 0, 0);

			{
				std::vector<std::string> metaKeys = terrainObjects.getObjectSettings().getMetaKeys();
				if(!metaKeys.empty())
				{
					int selection = SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_ID, CB_GETCURSEL, 0, 0);
					if(selection < 0)
					{
						SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_ID, CB_SETCURSEL, 0, 0);
						selection = 0;
					}

					int length = SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_ID, CB_GETLBTEXTLEN, selection, 0);
					if(length != CB_ERR)
					{
						char *buffer = (char *)alloca(length+1);
						buffer[0] = 0;
						SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_ID, CB_GETLBTEXT, selection, (LPARAM)buffer);
						setDialogItemText(dialog, IDC_OBJECT_VALUE, data.metaValues[buffer]);
					}
				}
			}

			noUpdate = false;
		}

		void pickupType(const TerrainObject &object)
		{
			int group = 0;
			int subgroup = 0;
			int index = 0;

			for(int i = 0; i < fileWrapper.getRootDirAmount(); ++i)
			{
				const std::string &dir = fileWrapper.getRootDir(i);
				for(int j = 0; j < fileWrapper.getDirAmount(i); ++j)
				{
					const std::string &dir = fileWrapper.getDir(i, j);

					for(int k = 0; k < fileWrapper.getFileAmount(i, j); ++k)
					{
						const std::string &file = fileWrapper.getFile(i, j, k);
						if(file == object.getFileName())
						{
							group = i;
							subgroup = j;
							index = k;
						}
					}
				}
			}

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_GROUP, CB_SETCURSEL, group, 0);
			SendMessage(dialog.getWindowHandle(), WM_COMMAND, IDC_OBJECT_GROUP, IDC_OBJECT_GROUP);
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_SUBGROUP, CB_SETCURSEL, subgroup + 1, 0);
			SendMessage(dialog.getWindowHandle(), WM_COMMAND, IDC_OBJECT_SUBGROUP, IDC_OBJECT_SUBGROUP);
			
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_MODELS, LB_SETSEL, FALSE, -1);
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_MODELS, LB_SETSEL, TRUE, index);
			SendMessage(dialog.getWindowHandle(), WM_COMMAND, IDC_OBJECT_MODELS, IDC_OBJECT_MODELS);
		}

		inline bool canUpdate(int id)
		{
			std::map<int, bool>::iterator it = allowUpdatingItem.find(id);
			if(it == allowUpdatingItem.end() || it->second == true)
				return true;
			return false;
		}


		void saveProperties(const std::string &fileName)
		{
			ObjectData &data = terrainObjects.getObjectSettings().getSettings(fileName);
			if(canUpdate(IDC_OBJECT_TYPE))
				data.type = SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_TYPE, CB_GETCURSEL, 0, 0) - 1;
			if(canUpdate(IDC_FALL_TYPE))
				data.fallType = SendDlgItemMessage(dialog.getWindowHandle(), IDC_FALL_TYPE, CB_GETCURSEL, 0, 0);
			if(canUpdate(IDC_OBJECT_HEIGHT))
				data.height = getDialogItemFloat(dialog, IDC_OBJECT_HEIGHT);

			if(canUpdate(IDC_OBJECT_XSIZE))
				data.radiusX = getDialogItemFloat(dialog, IDC_OBJECT_XSIZE);
			if(canUpdate(IDC_OBJECT_ZSIZE))
				data.radiusZ = getDialogItemFloat(dialog, IDC_OBJECT_ZSIZE);

			if(canUpdate(IDC_EXPLOSION_OBJECT))
			{
				int explosionIndex = SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_OBJECT, CB_GETCURSEL, 0, 0);
				if(explosionIndex <= 0)
					data.explosionObject = "";
				else if(explosionIndex == 1)
				{
					data.explosionObject = "(disappear)";
				}
				else
				{
					int length = SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_OBJECT, CB_GETLBTEXTLEN, explosionIndex, 0);
					std::string name;
					name.resize(length + 1);
					
					SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_OBJECT, CB_GETLBTEXT, explosionIndex, reinterpret_cast<LPARAM> (name.c_str()));
					name.resize(length);

					data.explosionObject = getFullFileName(name);
				}
			}

			{
				/*
				int scriptIndex = SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_SCRIPT, CB_GETCURSEL, 0, 0);
				int scriptLength = SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_SCRIPT, CB_GETLBTEXTLEN, scriptIndex, 0);
				data.explosionScript.resize(scriptLength + 1);
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_SCRIPT, CB_GETLBTEXT, scriptIndex, (LPARAM) &data.explosionScript[0]);
				data.explosionScript.resize(scriptLength);

				int effectIndex = SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_EFFECT, CB_GETCURSEL, 0, 0);
				int effectLength = SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_EFFECT, CB_GETLBTEXTLEN, effectIndex, 0);
				data.explosionEffect.resize(effectLength + 1);
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_EFFECT, CB_GETLBTEXT, effectIndex, (LPARAM) &data.explosionEffect[0]);
				data.explosionEffect.resize(effectLength);

				int projectileIndex = SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_PROJECTILE, CB_GETCURSEL, 0, 0);
				int projectileLength = SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_PROJECTILE, CB_GETLBTEXTLEN, projectileIndex, 0);
				data.explosionProjectile.resize(projectileLength + 1);
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_PROJECTILE, CB_GETLBTEXT, projectileIndex, (LPARAM) &data.explosionProjectile[0]);
				data.explosionProjectile.resize(projectileLength);

				int soundIndex = SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_SOUND, CB_GETCURSEL, 0, 0);
				int soundLength = SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_SOUND, CB_GETLBTEXTLEN, soundIndex, 0);
				data.explosionSound.resize(soundLength + 1);
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPLOSION_SOUND, CB_GETLBTEXT, soundIndex, (LPARAM) &data.explosionSound[0]);
				data.explosionSound.resize(soundLength);

				int materialIndex = SendDlgItemMessage(dialog.getWindowHandle(), IDC_MATERIAL, CB_GETCURSEL, 0, 0);
				int materialLength = SendDlgItemMessage(dialog.getWindowHandle(), IDC_MATERIAL, CB_GETLBTEXTLEN, materialIndex, 0);
				data.material.resize(materialLength + 1);
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_MATERIAL, CB_GETLBTEXT, materialIndex, (LPARAM) &data.material[0]);
				data.material.resize(materialLength);

				int animationIndex = SendDlgItemMessage(dialog.getWindowHandle(), IDC_ANIMATION, CB_GETCURSEL, 0, 0);
				int animationLength = SendDlgItemMessage(dialog.getWindowHandle(), IDC_ANIMATION, CB_GETLBTEXTLEN, animationIndex, 0);
				data.animation.resize(animationLength + 1);
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_ANIMATION, CB_GETLBTEXT, animationIndex, (LPARAM) &data.animation[0]);
				data.animation.resize(animationLength);
				*/

				if(canUpdate(IDC_EXPLOSION_SCRIPT))
					data.explosionScript = getDialogItemText(dialog, IDC_EXPLOSION_SCRIPT);
				if(canUpdate(IDC_EXPLOSION_EFFECT))
					data.explosionEffect = getDialogItemText(dialog, IDC_EXPLOSION_EFFECT);
				if(canUpdate(IDC_EXPLOSION_PROJECTILE))
					data.explosionProjectile = getDialogItemText(dialog, IDC_EXPLOSION_PROJECTILE);
				if(canUpdate(IDC_EXPLOSION_SOUND))
					data.explosionSound = getDialogItemText(dialog, IDC_EXPLOSION_SOUND);
				if(canUpdate(IDC_MATERIAL))
					data.material = getDialogItemText(dialog, IDC_MATERIAL);
				if(canUpdate(IDC_ANIMATION))
					data.animation = getDialogItemText(dialog, IDC_ANIMATION);
			}

			if(canUpdate(IDC_OBJECT_FIRETHROUGH))
			{
				if(IsDlgButtonChecked(dialog.getWindowHandle(), IDC_OBJECT_FIRETHROUGH) == BST_CHECKED)
					data.fireThrough = true;
				else
					data.fireThrough = false;
			}

			if(canUpdate(IDC_EXPLOSION_HP))
				data.hitpoints = getDialogItemInt(dialog, IDC_EXPLOSION_HP);
			if(canUpdate(IDC_OBJECT_BREAK_TEX))
				data.breakTexture = SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_BREAK_TEX, CB_GETCURSEL, 0, 0);
			if(canUpdate(IDC_PHYSICS_TYPE))
				data.physicsType = SendDlgItemMessage(dialog.getWindowHandle(), IDC_PHYSICS_TYPE, CB_GETCURSEL, 0, 0);
			if(canUpdate(IDC_PHYSICS_MASS))
				data.physicsWeight = getDialogItemText(dialog, IDC_PHYSICS_MASS);
			if(canUpdate(IDC_PHYSICS_MATERIAL))
				data.physicsMaterial = getDialogItemText(dialog, IDC_PHYSICS_MATERIAL);
			if(canUpdate(IDC_PHYSICS_DURABILITY))
				data.durabilityType = getDialogItemText(dialog, IDC_PHYSICS_DURABILITY);

			std::string key = getDialogItemText(dialog, IDC_OBJECT_ID);
			if(canUpdate(IDC_OBJECT_VALUE))
				data.metaValues[key] = getDialogItemText(dialog, IDC_OBJECT_VALUE);
		}

		void saveAllProperties()
		{
			if(noUpdate)
				return;

			std::vector<std::string> selection;
			getModelSelection(selection);
			if(selection.empty())
				return;

			for(unsigned int i = 0; i < selection.size(); i++)
			{
				std::string fileName = selection[i];
				assert(fileName.size() > 0);
				saveProperties(fileName);
			}
		}

		void updateModels()
		{
			modeData.rebuildModels = true;
			std::vector<std::string> modelNames;
			getModelSelection(modelNames);

			if(modelNames.empty())
			{
				terrainObjects.drawCollision(false, "");
				return;
			}
			
			allowUpdatingItem.clear();

			if(modelNames.size() != 1)
			{
				if(modelNames.size() < 1)
				{
				  settingsState(false);
				  terrainObjects.drawCollision(false, modelNames[0]);
				}
				else
				{
					terrainObjects.drawCollision(true, modelNames[0]);
					ItemStateList statelist;
					statelist.first_run = true;
					for(unsigned int i = 0; i < modelNames.size(); i++)
					{
						setProperties(modelNames[i]);

						// update items
						if(updateMultiSelection(statelist))
							// break if all controls cleared
							break;

						statelist.first_run = false;
					}
				}
				return;
			}

			settingsState(true);
			setProperties(modelNames[0]);

			terrainObjects.drawCollision(true, modelNames[0]);
		}

		void settingsState(bool enable)
		{
			int state = (enable) ? TRUE : FALSE;

			EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_OBJECT_TYPE), state);
			EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_FALL_TYPE), state);
			EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_OBJECT_FIRETHROUGH), state);
			EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_EXPLOSION_OBJECT), state);
			EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_EXPLOSION_EFFECT), state);
			EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_EXPLOSION_PROJECTILE), state);
			EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_EXPLOSION_SCRIPT), state);
			EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_EXPLOSION_SOUND), state);
			EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_EXPLOSION_HP), state);
			EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_MATERIAL), state);
			EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_ANIMATION), state);
			EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_OBJECT_BREAK_TEX), state);
			EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_PHYSICS_TYPE), state);
			EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_PHYSICS_MASS), state);
			EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_PHYSICS_MATERIAL), state);
			EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_PHYSICS_DURABILITY), state);
			EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_OBJECT_ID), state);
			EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_OBJECT_VALUE), state);
		}

		void reset()
		{
			noUpdate = false;
			//CheckDlgButton(dialog.getWindowHandle(), IDC_OBJECT_ROTATE, BST_UNCHECKED);
			CheckDlgButton(dialog.getWindowHandle(), IDC_OBJECT_ILLUMINATION, BST_UNCHECKED);
			CheckDlgButton(dialog.getWindowHandle(), IDC_OBJECT_GROUPTOOL, BST_UNCHECKED);
			CheckDlgButton(dialog.getWindowHandle(), IDC_OBJECT_INSERT, BST_CHECKED);

			CheckDlgButton(dialog.getWindowHandle(), IDC_GROUP_TOOL_FILTER_INVERT, BST_CHECKED);

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_AMOUNT, TBM_SETPOS, TRUE, 1);
			terrainObjects.clear();

			updateDialog();
			updateModelList();
			updateModels();

			settingsState(false);
		}

		std::string getFullFileName(const std::string &fileName) const
		{
			if(subGroupIndex)
			{
				for(int i = 0; i < fileWrapper.getFileAmount(groupIndex, subGroupIndex - 1); ++i)
				{
					const std::string &file = fileWrapper.getFile(groupIndex, subGroupIndex - 1, i);
					if(fileName == getFileName(file))
						return file;
				}
			}
			else
			{
				for(int i = 0; i < fileWrapper.getDirAmount(groupIndex); ++i)
				for(int j = 0; j < fileWrapper.getFileAmount(groupIndex, i); ++j)
				{
					const std::string &file = fileWrapper.getFile(groupIndex, i, j);
					std::string blah = getFileName(file);

					if(fileName == getFileName(file))
						return file;
				}
			}

			return "";
		}

		void getModelSelection(std::vector<std::string> &result) const
		{
			int selectionCount = SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_MODELS, LB_GETSELCOUNT, 0, 0);
			if(selectionCount == 0)
				return;

			boost::scoped_array<int> selections(new int[selectionCount + 1]);
			result.resize(selectionCount);

			int elements = SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_MODELS, LB_GETSELITEMS, selectionCount, reinterpret_cast<WPARAM> (selections.get()));
			assert(elements);

			for(int k = 0; k < elements; ++k)
			{
				int length = SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_MODELS, LB_GETTEXTLEN, selections[k], 0);
				assert(length);

				boost::scoped_array<char> oldString(new char[length + 1]);
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_MODELS, LB_GETTEXT, selections[k], reinterpret_cast<LPARAM> (oldString.get()));

				std::string fileName(oldString.get());
				std::string fullName = getFullFileName(fileName);
				result[k] = fullName;
			}
		}

		void setLighting(IStorm3D_Model *model, const COL &selfIllumination = COL())
		{
			if(!model)
				return;

			VC2 position(model->GetPosition().x, model->GetPosition().z);
			ui::PointLights lights;
			lights.ambient = editorState.getColorMap().getColor(position) + editorState.getLightMap().getColor(position);
			lights.ambient += selfIllumination;

			if(storm.lightManager)
				storm.lightManager->getLighting(model->GetPosition(), lights, ui::getRadius(model), false, true, model);

			model->SetSelfIllumination(lights.ambient);
			for(int i = 0; i < LIGHT_MAX_AMOUNT; ++i)
				model->SetLighting(i, lights.lightIndices[i]);

			model->useAlwaysDirectional(true);

			if(!storm.onFloor(position))
				model->SetDirectional(editorState.getSunDirection(), 1.f);
			else
				model->SetDirectional(VC3(), 0.f);
		}

		void writeStream(filesystem::OutputStream &stream) const
		{
			stream << int(0);
			stream << terrainObjects;
		}

		void readStream(filesystem::InputStream &stream)
		{
			reset();
			if(stream.isEof())
				return;

			int version = 0;
			stream >> version;

			stream >> terrainObjects;
			updateModelList();
		}
	};

	class GroupCommand: public ICommand
	{
		SharedData &data;
		Dialog &dialog;

	public:
		GroupCommand(SharedData &data_, Dialog &d)
		:	data(data_),
			dialog(d)
		{
			d.getCommandList().addCommand(IDC_OBJECT_GROUP, this);
			d.getCommandList().addCommand(IDC_OBJECT_SUBGROUP, this);
		}

		void execute(int id)
		{
			int groupIndex = SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_GROUP, CB_GETCURSEL, 0, 0);
			int subGroupIndex = SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_SUBGROUP, CB_GETCURSEL, 0, 0);

			if(groupIndex != data.groupIndex)
			{
				data.groupIndex = groupIndex;
				subGroupIndex = 0;
				data.updateModelList();
			}
			
			if(subGroupIndex != data.subGroupIndex)
			{
				data.subGroupIndex = subGroupIndex;
				data.updateModelList();
			}
		}
	};

	class GroupGroupCommand: public ICommand
	{
		SharedData &data;
		Dialog &dialog;

	public:
		GroupGroupCommand(SharedData &data_, Dialog &d)
		:	data(data_),
			dialog(d)
		{
			d.getCommandList().addCommand(IDC_OBJECT_GROUP_GROUP, this);
			d.getCommandList().addCommand(IDC_OBJECT_GROUP_SUBGROUP, this);
		}

		void execute(int id)
		{
			int groupGroupIndex = SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_GROUP_GROUP, CB_GETCURSEL, 0, 0);
			int groupSubGroupIndex = SendDlgItemMessage(dialog.getWindowHandle(), IDC_OBJECT_GROUP_SUBGROUP, CB_GETCURSEL, 0, 0);

			if(groupGroupIndex != data.groupGroupIndex)
			{
				data.groupGroupIndex = groupGroupIndex;
				groupSubGroupIndex = 0;
				data.updateGroupList();
			}
			
			if(groupSubGroupIndex != data.groupSubGroupIndex)
			{
				data.groupSubGroupIndex = groupSubGroupIndex;
				data.updateGroupList();
			}
		}
	};

	class ModelListCommand: public ICommand
	{
		SharedData &sharedData;

	public:
		ModelListCommand(SharedData &sharedData_, Dialog &d)
		:	sharedData(sharedData_)
		{
			d.getCommandList().addCommand(IDC_OBJECT_MODELS, this);
		}

		void execute(int id)
		{
			sharedData.updateModels();
		}
	};

	class ClickModeCommand: public ICommand
	{
		SharedData &sharedData;
		Dialog &dialog;

	public:
		ClickModeCommand(SharedData &sharedData_, Dialog &dialog_)
		:	sharedData(sharedData_),
			dialog(dialog_)
		{
			dialog.getCommandList().addCommand(IDC_OBJECT_INSERT, this);
		}

		void execute(int id)
		{
			sharedData.modeData.changeToMode = 0;

			CheckDlgButton(dialog.getWindowHandle(), IDC_OBJECT_INSERT, BST_CHECKED);
			//CheckDlgButton(dialog.getWindowHandle(), IDC_OBJECT_ROTATE, BST_UNCHECKED);
			CheckDlgButton(dialog.getWindowHandle(), IDC_OBJECT_ILLUMINATION, BST_UNCHECKED);
			CheckDlgButton(dialog.getWindowHandle(), IDC_OBJECT_GROUPTOOL, BST_UNCHECKED);
		}
	};

	/*
	class DragModeCommand: public ICommand
	{
		SharedData &sharedData;
		Dialog &dialog;

	public:
		DragModeCommand(SharedData &sharedData_, Dialog &dialog_)
		:	sharedData(sharedData_),
			dialog(dialog_)
		{
			dialog.getCommandList().addCommand(IDC_OBJECT_ROTATE, this);
		}

		void execute(int id)
		{
			sharedData.modeData.changeToMode = 1;

			CheckDlgButton(dialog.getWindowHandle(), IDC_OBJECT_INSERT, BST_UNCHECKED);
			CheckDlgButton(dialog.getWindowHandle(), IDC_OBJECT_ROTATE, BST_CHECKED);
			CheckDlgButton(dialog.getWindowHandle(), IDC_OBJECT_ILLUMINATION, BST_UNCHECKED);
			CheckDlgButton(dialog.getWindowHandle(), IDC_OBJECT_GROUPTOOL, BST_UNCHECKED);
		}
	};
	*/

	class IlluminationModeCommand: public ICommand
	{
		SharedData &sharedData;
		Dialog &dialog;

	public:
		IlluminationModeCommand(SharedData &sharedData_, Dialog &dialog_)
		:	sharedData(sharedData_),
			dialog(dialog_)
		{
			dialog.getCommandList().addCommand(IDC_OBJECT_ILLUMINATION, this);
		}

		void execute(int id)
		{
			sharedData.modeData.changeToMode = 2;

			CheckDlgButton(dialog.getWindowHandle(), IDC_OBJECT_INSERT, BST_UNCHECKED);
			//CheckDlgButton(dialog.getWindowHandle(), IDC_OBJECT_ROTATE, BST_UNCHECKED);
			CheckDlgButton(dialog.getWindowHandle(), IDC_OBJECT_ILLUMINATION, BST_CHECKED);
			CheckDlgButton(dialog.getWindowHandle(), IDC_OBJECT_GROUPTOOL, BST_UNCHECKED);
		}
	};

	class GroupModeCommand: public ICommand
	{
		SharedData &sharedData;
		Dialog &dialog;

	public:
		GroupModeCommand(SharedData &sharedData_, Dialog &dialog_)
		:	sharedData(sharedData_),
			dialog(dialog_)
		{
			dialog.getCommandList().addCommand(IDC_OBJECT_GROUPTOOL, this);
		}

		void execute(int id)
		{
			sharedData.modeData.changeToMode = 3;

			CheckDlgButton(dialog.getWindowHandle(), IDC_OBJECT_INSERT, BST_UNCHECKED);
			//CheckDlgButton(dialog.getWindowHandle(), IDC_OBJECT_ROTATE, BST_UNCHECKED);
			CheckDlgButton(dialog.getWindowHandle(), IDC_OBJECT_ILLUMINATION, BST_UNCHECKED);
			CheckDlgButton(dialog.getWindowHandle(), IDC_OBJECT_GROUPTOOL, BST_CHECKED);
		}
	};

	class AmountCommand: public ICommand
	{
		SharedData &sharedData;

	public:
		AmountCommand(SharedData &sharedData_, Dialog &dialog)
		:	sharedData(sharedData_)
		{
			dialog.getCommandList().addCommand(IDC_OBJECT_AMOUNT, this);
		}

		void execute(int id)
		{
			sharedData.modeData.rebuildModels = true;
		}
	};

	class GroupListCommand: public ICommand
	{
		SharedData &sharedData;

	public:
		GroupListCommand(SharedData &sharedData_, Dialog &dialog)
		:	sharedData(sharedData_)
		{
			dialog.getCommandList().addCommand(IDC_OBJECT_GROUP_LIST, this);
		}

		void execute(int id)
		{
			if(!sharedData.noUpdate)
				sharedData.modeData.updateGroups = true;
		}
	};

	class PropertyCommand: public ICommand
	{
		SharedData &sharedData;
		int type;

	public:
		PropertyCommand(SharedData &sharedData_, Dialog &dialog)
		:	sharedData(sharedData_)
		{
			dialog.getCommandList().addCommand(IDC_OBJECT_TYPE, this);
			dialog.getCommandList().addCommand(IDC_FALL_TYPE, this);
			dialog.getCommandList().addCommand(IDC_OBJECT_HEIGHT, this);
			dialog.getCommandList().addCommand(IDC_OBJECT_XSIZE, this);
			dialog.getCommandList().addCommand(IDC_OBJECT_ZSIZE, this);
			dialog.getCommandList().addCommand(IDC_OBJECT_FIRETHROUGH, this);
			dialog.getCommandList().addCommand(IDC_EXPLOSION_OBJECT, this);
			dialog.getCommandList().addCommand(IDC_EXPLOSION_EFFECT, this);
			dialog.getCommandList().addCommand(IDC_EXPLOSION_PROJECTILE, this);
			dialog.getCommandList().addCommand(IDC_EXPLOSION_SCRIPT, this);
			dialog.getCommandList().addCommand(IDC_EXPLOSION_SOUND, this);
			dialog.getCommandList().addCommand(IDC_MATERIAL, this);
			dialog.getCommandList().addCommand(IDC_ANIMATION, this);
			dialog.getCommandList().addCommand(IDC_EXPLOSION_HP, this);
			dialog.getCommandList().addCommand(IDC_OBJECT_BREAK_TEX, this);
			dialog.getCommandList().addCommand(IDC_PHYSICS_TYPE, this);
			dialog.getCommandList().addCommand(IDC_PHYSICS_MASS, this);
			dialog.getCommandList().addCommand(IDC_PHYSICS_MATERIAL, this);
			dialog.getCommandList().addCommand(IDC_PHYSICS_DURABILITY, this);
			//dialog.getCommandList().addCommand(IDC_OBJECT_ID, this);
			dialog.getCommandList().addCommand(IDC_OBJECT_VALUE, this);

			type = 0;
		}

		void execute(int id)
		{
			if(!sharedData.noUpdate)
			{
				std::map<int, bool>::iterator it = sharedData.allowUpdatingItem.find(id);
				if(it != sharedData.allowUpdatingItem.end())
					it->second = true;
			}

			sharedData.saveAllProperties();
			if(sharedData.noUpdate)
			{
				type = SendDlgItemMessage(sharedData.dialog.getWindowHandle(), IDC_OBJECT_TYPE, CB_GETCURSEL, 0, 0) - 1;
				return;
			}

			int newType = SendDlgItemMessage(sharedData.dialog.getWindowHandle(), IDC_OBJECT_TYPE, CB_GETCURSEL, 0, 0) - 1;
			if(type == newType)
				return;

			std::vector<std::string> selection;
			sharedData.getModelSelection(selection);

			std::string fileName = selection[0];
			fileName = getFileName(fileName);
			if(newType == -1)
				fileName += " -- (no properties set)";

			sharedData.noUpdate = true;

			sharedData.noUpdate = false;
			type = newType;
		}
	};

	class IdCommand: public ICommand
	{
		SharedData &sharedData;

	public:
		IdCommand(SharedData &sharedData_, Dialog &dialog)
		:	sharedData(sharedData_)
		{
			dialog.getCommandList().addCommand(IDC_OBJECT_ID, this);
		}

		void execute(int id)
		{
			if(sharedData.noUpdate)
				return;

			std::string key = getDialogItemText(sharedData.dialog, IDC_OBJECT_ID);
	if(key != "Joojoo")
		int a = 0;

			std::vector<std::string> modelList;
			sharedData.getModelSelection(modelList);

			if(!modelList.empty())
			{
				ObjectData &data = sharedData.terrainObjects.getObjectSettings().getSettings(modelList[0]);
				std::string value = data.metaValues[key];

				for(unsigned int i = 1; i < modelList.size(); i++)
				{
					ObjectData &data2 = sharedData.terrainObjects.getObjectSettings().getSettings(modelList[i]);
					if(data2.metaValues[key] != value)
					{
						value = "";
						sharedData.allowUpdatingItem[IDC_OBJECT_VALUE] = false;
						break;
					}
				}

				sharedData.noUpdate = true;
				setDialogItemText(sharedData.dialog, IDC_OBJECT_VALUE, value);
				sharedData.noUpdate = false;
			}
		}
	};

	/* Object modes */

	class ClickMode
	{
		SharedData &sharedData;

		std::vector<boost::shared_ptr<IStorm3D_Model> > models;
		std::vector<Vector> modelPositions;
		std::vector<Vector> modelRotations;
		std::vector<std::string> modelNames;
		
		//float rotation;
		int amount;

		float height;
		float modelHeight;

	public:
		ClickMode(SharedData &sharedData_)
		:	sharedData(sharedData_)
		{
			reset();
			srand(0);
		}

		void removeModels()
		{
			for(unsigned int i = 0; i < models.size(); ++i)
				sharedData.storm.scene->RemoveModel(models[i].get());
		}

		void rebuild()
		{
			removeModels();

			std::vector<std::string> selection;
			sharedData.getModelSelection(selection);

			if(selection.empty())
			{
				reset();
				return;
			}

			amount = SendDlgItemMessage(sharedData.dialog.getWindowHandle(), IDC_OBJECT_AMOUNT, TBM_GETPOS, 0, 0);

			models.resize(selection.size() * amount);
			modelNames.resize(selection.size() * amount);
			modelPositions.resize(selection.size() * amount);
			modelRotations.resize(selection.size() * amount);

			for(unsigned int i = 0; i < selection.size(); ++i)
			for(int j = 0; j < amount; ++j)
			{
				std::string fileName = selection[i];

				std::string loadFileName = fileName;
				ObjectData &objectDataTmp = sharedData.terrainObjects.getObjectSettings().getSettings(fileName);
				std::string postfix = objectDataTmp.metaValues["filename_postfix"];
				if(!postfix.empty())
				{
					loadFileName += postfix;
				}

				boost::shared_ptr<IStorm3D_Model> model = createEditorModel(*sharedData.storm.storm, loadFileName);

				models[i*amount + j] = model;
				modelNames[i*amount + j] = selection[i];
				modelPositions[i*amount + j] = Vector(0, modelHeight, 0);
			}

			randomPositions();
			for(unsigned int i = 0; i < models.size(); ++i)
			{
				VC3 &angles = modelRotations[i];
				QUAT q = getRotation(angles);
				models[i]->SetRotation(q);
			}
		}

		void rotateModel(int delta)
		{
			VC3 &angles = modelRotations[0];

			if(GetKeyState('X') & 0x80)
				angles.x = sharedData.storm.unitAligner.getRotation(angles.x, delta);
			else if(GetKeyState('Z') & 0x80)
				angles.z = sharedData.storm.unitAligner.getRotation(angles.z, delta);
			else
				angles.y = sharedData.storm.unitAligner.getRotation(angles.y, delta);

			QUAT q = getRotation(angles);
			models[0]->SetRotation(q);
		}

		void updateHeight()
		{
			VC3 &angles = modelRotations[0];

			if(GetKeyState('R') & 0x80)
			{
				modelHeight = height = angles.x = angles.y = angles.z = 0;
				rotateModel(0);

				modelHeight = getDialogItemFloat(sharedData.dialog, IDC_LONKERO);
			}
			/*
			else if(GetKeyState(VK_PRIOR) & 0x80)
				modelHeight = sharedData.storm.unitAligner.getHeight(height, 1);
			else if(GetKeyState(VK_NEXT) & 0x80)
				modelHeight = sharedData.storm.unitAligner.getHeight(height, -1);
			*/
			else if(GetKeyState('H') & 0x80)
			{
				modelHeight = height = 0;
				modelHeight = getDialogItemFloat(sharedData.dialog, IDC_LONKERO);
			}
			else
			{
				if(GetKeyState(VK_PRIOR) & 0x80)
					modelHeight = sharedData.storm.unitAligner.getHeight(modelHeight, 1);
				else if(GetKeyState(VK_NEXT) & 0x80)
					modelHeight = sharedData.storm.unitAligner.getHeight(modelHeight, -1);
			}
		}

		void randomPositions()
		{
			if(models.size() == 1)
				return;
			
			float range = float(2 * sqrtf(float(models.size())));
			if(range < 2.f)
				range = 2.f;

			for(unsigned int i = 0; i < modelPositions.size(); ++i)
			{
				Vector &position = modelPositions[i];
				
				position.x = (float(rand()) / RAND_MAX) * 2 * range - range;
				position.z = (float(rand()) / RAND_MAX) * 2 * range - range;
				position = sharedData.storm.unitAligner.getAlignedPosition(position);

				float r = (float(rand()) / RAND_MAX) * 2 * PI;
				VC3 &angles = modelRotations[i];
				angles.x = angles.z = 0;
				angles.y = r;

				QUAT q = getRotation(angles);
				models[i]->SetRotation(q);
			}
		}

		void setPositions(const Vector &cursor)
		{
			for(unsigned int i = 0; i < modelPositions.size(); ++i)
			{
				Vector position = modelPositions[i];
				position += cursor;
				position.y = sharedData.storm.getHeight(VC2(position.x, position.z));
				position.y += modelHeight;

				models[i]->SetPosition(position);
				sharedData.setLighting(models[i].get());
				sharedData.storm.scene->AddModel(models[i].get());
			}
		}

		void addToTerrain()
		{
			for(unsigned int i = 0; i < models.size(); ++i)
			{
				Vector modelPosition = models[i]->GetPosition();
				VC2 position(modelPosition.x, modelPosition.z);

				VC3 modelRotation = modelRotations[i];
				sharedData.terrainObjects.addObject(modelNames[i], position, modelRotation, modelHeight);
			}

			sharedData.editorState.updateShadows();
			sharedData.updateModels();
		}

		void modeTick()
		{
			Storm &storm = sharedData.storm;
			removeModels();

			Mouse &mouse = sharedData.gui.getMouse();
			if(!mouse.isInsideWindow())
				return;

			if(models.empty())
				return;

			updateHeight();

			int mouseDelta = mouse.getWheelDelta();
			if(mouseDelta)
			{
				if(models.size() == 1)
					rotateModel(mouseDelta);
				else
					randomPositions();
			}

			Storm3D_CollisionInfo ci;
			if(!mouse.cursorRayTrace(ci))
				return;

			ci.position = sharedData.storm.unitAligner.getAlignedPosition(ci.position);
			setPositions(ci.position);

			if(mouse.hasLeftClicked())
				addToTerrain();
		}

		void reset()
		{
			models.clear();
			modelPositions.clear();
			modelRotations.clear();
			modelNames.clear(),

			//rotation = 0;
			amount = 0;

			height = 0;
			modelHeight = 0;
		}
	};

	class DragMode
	{
		SharedData &sharedData;

	public:
		DragMode(SharedData &sharedData_)
		:	sharedData(sharedData_)
		{
			reset();
			srand(0);
		}

		void modeTick()
		{
			Mouse &mouse = sharedData.gui.getMouse();
			if(!mouse.isInsideWindow())
				return;

			Storm3D_CollisionInfo ci;
			Vector p, d;
			float rayLength = 300.f;
			if(mouse.cursorRayTrace(ci, &p, &d))
			{
				//rayLength = p.GetRangeTo(ci.position);
			}

			TerrainObject terrainObject = sharedData.terrainObjects.traceActiveCollision(p, d, rayLength);
			if(mouse.hasLeftClicked())
			{
				sharedData.terrainObjects.removeObject(terrainObject);
				sharedData.updateModels();
			}
		}

		void reset()
		{
		}
	};

	class IlluminationMode
	{
		SharedData &sharedData;
		float modelHeight;

		TerrainObject activeObject;

	public:
		IlluminationMode(SharedData &sharedData_)
		:	sharedData(sharedData_),
			modelHeight(0)
		{
			reset();
		}

		void modeTick()
		{
			Mouse &mouse = sharedData.gui.getMouse();
			if(!mouse.isInsideWindow())
				return;

			Storm3D_CollisionInfo ci;
			Vector p, d;
			float rayLength = 500.f;
			if(mouse.cursorRayTrace(ci, &p, &d))
			{
				//rayLength = p.GetRangeTo(ci.position);
			}

			if(mouse.hasLeftClicked())
			{
				TerrainObject trace = sharedData.terrainObjects.traceActiveCollision(p, d, rayLength);
				if(trace == activeObject)
				{
					activeObject = TerrainObject();
					sharedData.terrainObjects.traceActiveCollision(VC3(-10000,100000,-100000), VC3(0,1,0), rayLength);
				}
				else
					activeObject = trace;
			}

			if(activeObject.hasObject())
			{
				if(GetKeyState('C') & 0x80)
				{
					ColorPicker colorPicker;

					const COL &col = activeObject.getColor();
					unsigned char r = unsigned char(col.r * 255.f);
					unsigned char g = unsigned char(col.g * 255.f);
					unsigned char b = unsigned char(col.b * 255.f);
					unsigned int color = RGB(r,g,b);

					if(colorPicker.run(color))
						sharedData.terrainObjects.setColor(activeObject, colorPicker.getColor());
				}

				// Move
				{

					VC3 pos = sharedData.storm.unitAligner.getMovedPosition(VC3(), *sharedData.storm.scene->GetCamera());
					sharedData.terrainObjects.moveObject(activeObject, VC2(pos.x, pos.z));
				}

				// Height
				{
					float height = activeObject.getHeight();
					if(GetKeyState(VK_PRIOR) & 0x80)
						height = sharedData.storm.unitAligner.getHeight(height, 1);
					else if(GetKeyState(VK_NEXT) & 0x80)
						height = sharedData.storm.unitAligner.getHeight(height, -1);

					sharedData.terrainObjects.moveObject(activeObject, height);
				}

				// Rotate
				int wheelDelta = mouse.getWheelDelta();
				if(wheelDelta)
				{
					VC3 angles = activeObject.getRotation();
					if(GetKeyState('X') & 0x80)
						angles.x = sharedData.storm.unitAligner.getRotation(angles.x, wheelDelta);
					else if(GetKeyState('Z') & 0x80)
						angles.z = sharedData.storm.unitAligner.getRotation(angles.z, wheelDelta);
					else
						angles.y = sharedData.storm.unitAligner.getRotation(angles.y, wheelDelta);

					sharedData.terrainObjects.rotateObject(activeObject, angles);
				}

				// Color
				{
					float delta = getTimeDelta();
					if(GetKeyState(VK_HOME) & 0x80)
						sharedData.terrainObjects.setLightMultiplier(activeObject, delta);
					else if(GetKeyState(VK_END) & 0x80)
						sharedData.terrainObjects.setLightMultiplier(activeObject, -delta);
				}

				// Type picker
				if(GetKeyState('P') & 0x80)
				{
					sharedData.pickupType(activeObject);
				}

				// Reset
				if(GetKeyState('R') & 0x80)
				{
					sharedData.terrainObjects.rotateObject(activeObject, VC3());
					sharedData.terrainObjects.moveObject(activeObject, getDialogItemFloat(sharedData.dialog, IDC_LONKERO));
				}

				// height-only reset
				if(GetKeyState('H') & 0x80)
				{
					sharedData.terrainObjects.moveObject(activeObject, getDialogItemFloat(sharedData.dialog, IDC_LONKERO));
				}

				if(GetKeyState(VK_DELETE) & 0x80)
				{
					sharedData.terrainObjects.removeObject(activeObject);
					activeObject = TerrainObject();
				}
			}
		}

		/*
		void modeTick()
		{
			Mouse &mouse = sharedData.gui.getMouse();
			if(!mouse.isInsideWindow())
				return;

			Storm3D_CollisionInfo ci;
			Vector p, d;
			float rayLength = 500.f;
			if(mouse.cursorRayTrace(ci, &p, &d))
			{
				//rayLength = p.GetRangeTo(ci.position);
			}

			TerrainObject terrainObject = sharedData.terrainObjects.traceActiveCollision(p, d, rayLength);
			if(terrainObject.hasObject() && mouse.hasLeftClicked())
			{
				ColorPicker colorPicker;

				const COL &col = terrainObject.getColor();
				unsigned char r = unsigned char(col.r * 255.f);
				unsigned char g = unsigned char(col.g * 255.f);
				unsigned char b = unsigned char(col.b * 255.f);
				unsigned int color = RGB(r,g,b);

				if(colorPicker.run(color))
					sharedData.terrainObjects.setColor(terrainObject, colorPicker.getColor());
			}

			if(terrainObject.hasObject())
			{
				// Move
				{
					VC3 pos = sharedData.storm.unitAligner.getMovedPosition(VC3(), *sharedData.storm.scene->GetCamera());
					sharedData.terrainObjects.moveObject(terrainObject, VC2(pos.x, pos.z));
				}

				// Height
				{
					float height = terrainObject.getHeight();
					if(GetKeyState(VK_PRIOR) & 0x80)
						sharedData.storm.unitAligner.getHeight(height, 1);
					else if(GetKeyState(VK_NEXT) & 0x80)
						sharedData.storm.unitAligner.getHeight(height, -1);

					sharedData.terrainObjects.moveObject(terrainObject, height);
				}

				// Rotate
				int wheelDelta = mouse.getWheelDelta();
				if(wheelDelta)
				{
					VC3 angles = terrainObject.getRotation();
					if(GetKeyState('X') & 0x80)
						angles.x = sharedData.storm.unitAligner.getRotation(angles.x, wheelDelta);
					else if(GetKeyState('Z') & 0x80)
						angles.z = sharedData.storm.unitAligner.getRotation(angles.z, wheelDelta);
					else
						angles.y = sharedData.storm.unitAligner.getRotation(angles.y, wheelDelta);

					sharedData.terrainObjects.rotateObject(terrainObject, angles);
				}

				// Color
				{
					float delta = getTimeDelta();
					if(GetKeyState(VK_HOME) & 0x80)
						sharedData.terrainObjects.setLightMultiplier(terrainObject, delta);
					else if(GetKeyState(VK_END) & 0x80)
						sharedData.terrainObjects.setLightMultiplier(terrainObject, -delta);
				}

				// Type picker
				{
					if(GetKeyState('P') & 0x80)
					{
						sharedData.pickupType(terrainObject);
					}
				}

				// Reset
				if(GetKeyState('R') & 0x80)
				{
					sharedData.terrainObjects.rotateObject(terrainObject, VC3());
					sharedData.terrainObjects.moveObject(terrainObject, 0);
				}
			}
		}
		*/

		void deactivate()
		{
			activeObject = TerrainObject();
		}

		void reset()
		{
			deactivate();
			modelHeight = 0;
		}
	};

	class GroupMode
	{
		SharedData &sharedData;
		boost::shared_ptr<IStorm3D_Model> model;

		ModelGroup modelGroup;

		float range;

	public:
		GroupMode(SharedData &sharedData_)
		:	sharedData(sharedData_),
			range(2.f)
		{
			reset();
			reload();
		}

		void modeTick()
		{
			if(!model)
			{
				reload();
				return;
			}

			sharedData.storm.scene->RemoveModel(model.get());
			for(unsigned int i = 0; i < modelGroup.models.size(); ++i)
			{
				if(modelGroup.models[i])
					sharedData.storm.scene->RemoveModel(modelGroup.models[i].get());
			}
			
			Mouse &mouse = sharedData.gui.getMouse();
			if(!mouse.isInsideWindow())
				return;

			Storm3D_CollisionInfo ci;
			Vector p, d;
			float rayLength = 500.f;
			if(!mouse.cursorRayTrace(ci, &p, &d))
				return;

			ci.position = sharedData.storm.unitAligner.getAlignedPosition(ci.position);
			ci.position.y = sharedData.storm.terrain->getHeight(VC2(ci.position.x, ci.position.z));

			if(modelGroup.models.empty())
			{
				VC3 position = sharedData.storm.unitAligner.getAlignedPosition(ci.position);

				int wheelDelta = mouse.getWheelDelta();
				if(wheelDelta)
				{
					range += wheelDelta / 300.f;
					if(range < .5f)
						range = .5f;
					//else if(range > 200.f)
					//	range = 200.f;
				}

				float modelScale = range / 10.f;
				model->SetScale(VC3(modelScale, 2.f ,modelScale));
				model->SetPosition(ci.position);
				sharedData.storm.scene->AddModel(model.get());

				if(GetKeyState('N') & 0x80)
				{

					static int nudgeTime = 0;
					int curTime = timeGetTime();
					if (curTime >= nudgeTime + 100)
					{
						nudgeTime = curTime;

						VC3 nudgeDir = VC3(0,0,0);
						if (GetKeyState(VK_UP) & 0x80)
							nudgeDir.z += 0.25f;
						if (GetKeyState(VK_DOWN) & 0x80)
							nudgeDir.z -= 0.25f;
						if (GetKeyState(VK_LEFT) & 0x80)
							nudgeDir.x -= 0.25f;
						if (GetKeyState(VK_RIGHT) & 0x80)
							nudgeDir.x += 0.25f;
						if (GetKeyState(VK_PRIOR) & 0x80)
							nudgeDir.y += 0.10f;
						if (GetKeyState(VK_NEXT) & 0x80)
							nudgeDir.y -= 0.10f;

						if (GetKeyState(VK_CONTROL) & 0x80)
						{
							nudgeDir *= 0.01f;
						}
						if (GetKeyState(VK_SHIFT) & 0x80)
						{
							nudgeDir *= 10.0f;
						}

						if (nudgeDir.GetSquareLength() != 0.0f)
						{
							nudgeObjects(position, nudgeDir);
						}
					}
				}

				if(GetKeyState(VK_DELETE) & 0x80)
					deleteObjects(position);
				else if(GetKeyState('C') & 0x80)
				{
					copyObjects(position);
					modelGroup.heightOffset = 0;
				}
			}
			else
			{
				int wheelDelta = mouse.getWheelDelta();
				if(wheelDelta)
				{
					if(GetKeyState('X') & 0x80)
					{
						modelGroup.rotationEul.x  = sharedData.storm.unitAligner.getRotation(modelGroup.rotationEul.x, wheelDelta);
					}
					else if(GetKeyState('Z') & 0x80)
					{
						modelGroup.rotationEul.z = sharedData.storm.unitAligner.getRotation(modelGroup.rotationEul.z, wheelDelta);
					} else {
						modelGroup.rotationEul.y = sharedData.storm.unitAligner.getRotation(modelGroup.rotationEul.y, wheelDelta);
					}
				}

				VC3 position_clicked = ci.position;
				VC3 position = ci.position;

				// keep the group in grid...
				// --jpk
				// TODO: get the correct grid size from somewhere?
				float gridSizeX = 0.5f;
				float gridSizeZ = 0.5f;
				sharedData.storm.unitAligner.getGridSize(gridSizeX, gridSizeZ);

				// solving original group center grid cell in current grid size...
				float ox = modelGroup.objectGroup.original.x;
				float oz = modelGroup.objectGroup.original.z;
				float ogridx = int(ox / gridSizeX + 0.5f) * gridSizeX;
				float ogridz = int(oz / gridSizeZ + 0.5f) * gridSizeZ;

				// this is the grid offset of the original group center in our current grid size...
				float offsetx = ox - ogridx;
				float offsetz = oz - ogridz;

				// current grid cell position...
				float gridx = int(position_clicked.x / gridSizeX + 0.5f) * gridSizeX;
				float gridz = int(position_clicked.z / gridSizeZ + 0.5f) * gridSizeZ;

				if((GetKeyState(VK_CONTROL) & 0x80) == 0)
				{
					// actual grid corrected position...
					position = VC3(gridx + offsetx, position.y, gridz + offsetz);
				}

				if(GetKeyState(VK_PRIOR) & 0x80)
					modelGroup.heightOffset = sharedData.storm.unitAligner.getHeight(modelGroup.heightOffset, 1);
				else if(GetKeyState(VK_NEXT) & 0x80)
					modelGroup.heightOffset = sharedData.storm.unitAligner.getHeight(modelGroup.heightOffset, -1);

				if(GetKeyState('R') & 0x80)
				{
					modelGroup.heightOffset = getDialogItemFloat(sharedData.dialog, IDC_LONKERO);
					modelGroup.rotationEul = VC3(0,0,0);
				}

				if(GetKeyState('H') & 0x80)
				{
					modelGroup.heightOffset = getDialogItemFloat(sharedData.dialog, IDC_LONKERO);
				}

				modelGroup.update(sharedData.storm, position);

				static int lightUpdateHaxValue = 0;
				bool updateLighting = false;
				if(++lightUpdateHaxValue == 10)
				{
					updateLighting = true;
					lightUpdateHaxValue = 0;
				}
				
				for(unsigned int i = 0; i < modelGroup.models.size(); ++i)
				{
					if(modelGroup.models[i])
					{
						IStorm3D_Model *m = modelGroup.models[i].get();
						if(updateLighting)
							sharedData.setLighting(m, modelGroup.objectGroup.instances[i].color);
						sharedData.storm.scene->AddModel(m);
					}
				}

				if(mouse.hasLeftClicked())
					insertObjects(ci.position);
				else if(GetKeyState(VK_F1) & 0x80)
					saveCurrent();
				else if(GetKeyState('V') & 0x80)
					clearSelection();
			}
		}

		void rebuild()
		{
			clearSelection();

			int groupIndex = sharedData.groupGroupIndex;
			int subgroupIndex = sharedData.groupSubGroupIndex;

			int selectedIndex = SendDlgItemMessage(sharedData.dialog.getWindowHandle(), IDC_OBJECT_GROUP_LIST, LB_GETCURSEL, 0, 0);
			if(groupIndex < 0 || subgroupIndex < 0 || selectedIndex < 0)
				return;

			int groupMin = subgroupIndex - 1;
			int groupMax = subgroupIndex;
			if(groupMin < 0)
			{
				groupMin = 0;
				groupMax = sharedData.list.getSubgroupAmount(groupIndex);
			}

			for(int i = groupMin; i < groupMax; ++i)
			{
				int objectGroupAmount = sharedData.list.getObjectGroupAmount(groupIndex, i);
				if(selectedIndex < objectGroupAmount)
				{
					const GroupList::ObjectGroup &objectGroup = sharedData.list.getObjectGroup(groupIndex, i, selectedIndex);
					setGroup(objectGroup, sharedData.terrainObjects.getObjectSettings());

					break;
				}
				else
					selectedIndex -= objectGroupAmount;
			}
		}

		void setGroup(const GroupList::ObjectGroup &objectGroup, ObjectSettings &objectSettings)
		{
			modelGroup.create(sharedData.storm, objectGroup, objectSettings);
		}

		void insertObjects(const VC3 &position_clicked)
		{
			VC3 position = position_clicked;

			TerrainObjects &objects = sharedData.terrainObjects;

			/*
			// keep the group in grid...
			// --jpk
			// TODO: get the correct grid size from somewhere?
			float gridSizeX = 0.5f;
			float gridSizeZ = 0.5f;

			// solving original group center grid cell in current grid size...
			float ox = modelGroup.objectGroup.original.x;
			float oz = modelGroup.objectGroup.original.z;
			float ogridx = int(ox / gridSizeX + 0.5f) * gridSizeX;
			float ogridz = int(oz / gridSizeZ + 0.5f) * gridSizeZ;

			// this is the grid offset of the original group center in our current grid size...
			float offsetx = ox - ogridx;
			float offsetz = oz - ogridz;

			// current grid cell position...
			float gridx = int(position_clicked.x / gridSizeX + 0.5f) * gridSizeX;
			float gridz = int(position_clicked.z / gridSizeZ + 0.5f) * gridSizeZ;

			if((GetKeyState(VK_CONTROL) & 0x80) == 0)
			{
				// actual grid corrected position...
				position = VC3(gridx + offsetx, position.y, gridz + offsetz);
			}

			// WTF, the position variable that this function takes in, is never actually used...
			// the position variable defined below overrides that...
			// now, named position_clicked instead of just position. 
			// still, nasty shadowing variable names here.
			// --jpk
			*/
			
			VC3 position_offset = position - position_clicked;

			for(unsigned int i = 0; i < modelGroup.objectGroup.instances.size(); ++i)
			{
				const GroupList::Instance &instance = modelGroup.objectGroup.instances[i];
				VC3 position = modelGroup.getPosition(sharedData.storm, i);

				TerrainObject terrainObject = objects.addObject(instance.model, VC2(position.x + position_offset.x, position.z + position_offset.z), modelGroup.getRotation(sharedData.storm, i), instance.position.y + position_offset.y + modelGroup.heightOffset);

				unsigned char r = unsigned char(instance.color.r * 255.f);
				unsigned char g = unsigned char(instance.color.r * 255.f);
				unsigned char b = unsigned char(instance.color.r * 255.f);
				int color = r << 16 | g << 8 | b;
				objects.setColor(terrainObject, color);
			}
		}

		void getGroupToolFilter(const char ***filterOut, int *filterAmountOut, bool *invertOut)
		{
			static bool invert = false;

			static const int maxFilters = 32;
			static const char *filter[maxFilters] = { 
				"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
				"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
			};
			static int filterAmount = 0;

			bool back = isCheckEnabled(sharedData.dialog, IDC_GROUP_TOOL_FILTER_BACK);
			bool ground = isCheckEnabled(sharedData.dialog, IDC_GROUP_TOOL_FILTER_GROUND);
			bool decal = isCheckEnabled(sharedData.dialog, IDC_GROUP_TOOL_FILTER_DECAL);
			bool dynamic = isCheckEnabled(sharedData.dialog, IDC_GROUP_TOOL_FILTER_DYNAMIC);
			bool filtstatic = isCheckEnabled(sharedData.dialog, IDC_GROUP_TOOL_FILTER_STATIC);
			bool nocollision = isCheckEnabled(sharedData.dialog, IDC_GROUP_TOOL_FILTER_NOCOLLISION);
			bool helper = isCheckEnabled(sharedData.dialog, IDC_GROUP_TOOL_FILTER_HELPER);
			bool prop = isCheckEnabled(sharedData.dialog, IDC_GROUP_TOOL_FILTER_PROP);
			bool maingroup = isCheckEnabled(sharedData.dialog, IDC_GROUP_TOOL_FILTER_MAINGROUP);
			bool modelsgroup = isCheckEnabled(sharedData.dialog, IDC_GROUP_TOOL_FILTER_MODELS);

			filterAmount = 0;
			invert = isCheckEnabled(sharedData.dialog, IDC_GROUP_TOOL_FILTER_INVERT);

			if (dynamic) { filter[filterAmount++] = "*DYNAMIC"; }
			if (filtstatic) { filter[filterAmount++] = "*STATIC"; }
			if (nocollision) { filter[filterAmount++] = "*NOCOLLISION"; }
#ifdef LEGACY_FILES
			if (back) { filter[filterAmount++] = "back_"; }
			if (ground) { filter[filterAmount++] = "floor"; }
			if (decal) { filter[filterAmount++] = "Decals\\"; }
			if (helper) { filter[filterAmount++] = "Helpers\\"; }
			if (prop) { filter[filterAmount++] = "Props\\"; }
#else
			if (back) { filter[filterAmount++] = "back_"; }
			if (ground) { filter[filterAmount++] = "ground\\"; }
			if (decal) { filter[filterAmount++] = "decal\\"; }
			if (helper) { filter[filterAmount++] = "helper\\"; }
			if (prop) { filter[filterAmount++] = "prop\\"; }
#endif
			static std::string groupFiltName;
			static std::string modelsFiltName[maxFilters];

			if (maingroup)
			{
				if (sharedData.groupIndex >= 0)
				{				
					groupFiltName = sharedData.fileWrapper.getRootDir(sharedData.groupIndex);

					groupFiltName += "\\";
					if (sharedData.subGroupIndex > 0)
					{
						std::string subGroupFiltName = sharedData.fileWrapper.getDir(sharedData.groupIndex, sharedData.subGroupIndex - 1);
						groupFiltName += subGroupFiltName;
						groupFiltName += "\\";
					}
					filter[filterAmount++] = groupFiltName.c_str();
				}
			}

			if (modelsgroup)
			{
				std::vector<std::string> modelList;
				sharedData.getModelSelection(modelList);
				for (int i = 0; i < (int)modelList.size(); i++)
				{
					if (filterAmount < maxFilters)
					{
						assert(i <= filterAmount);
						modelsFiltName[i] = modelList[i];
						filter[filterAmount++] = modelsFiltName[i].c_str();
					} else {
						static bool spammedTheError = false;
						if (!spammedTheError)
						{
							spammedTheError = true;
							MessageBox(sharedData.dialog.getParentWindowHandle(), "Too many models selected for filtering. (this message will be shown only once)", "Oopsie.", MB_OK);
						}
						break;
					}
				}
			}

			// WARNING: don't add any more filters after modelsgroup filter!
			// (as that may have filled up the filters array.)

			*filterOut = filter;
			*filterAmountOut = filterAmount;
			*invertOut = invert;
		}

		void deleteObjects(const VC3 &position)
		{
			const char **filter = NULL;
			int filterAmount = 0;
			bool invert = false;
			getGroupToolFilter(&filter, &filterAmount, &invert);

			sharedData.terrainObjects.removeObjects(position, range, filter, filterAmount, invert);
		}

		void copyObjects(const VC3 &position)
		{
			modelGroup.rotationEul = VC3(0,0,0);
			GroupList::ObjectGroup objectGroup;

			const char **filter = NULL;
			int filterAmount = 0;
			bool invert = false;
			getGroupToolFilter(&filter, &filterAmount, &invert);

			sharedData.terrainObjects.copyObjects(position, range, objectGroup, filter, filterAmount, invert);

			setGroup(objectGroup, sharedData.terrainObjects.getObjectSettings());
		}

		void nudgeObjects(const VC3 &position, const VC3 &direction)
		{
			const char **filter = NULL;
			int filterAmount = 0;
			bool invert = false;
			getGroupToolFilter(&filter, &filterAmount, &invert);

			sharedData.terrainObjects.nudgeObjects(position, direction, range, filter, filterAmount, invert);

			bool nudgeLights = isCheckEnabled(sharedData.dialog, IDC_GROUP_TOOL_FILTER_LIGHTS);

			if ((nudgeLights && !invert)
				|| (!nudgeLights && invert))
			{
				sharedData.editorState.getLightMode().nudgeLights(position, direction, range);
			}

			sharedData.editorState.getCamera().nudgeCamera(direction);
		}

		void saveCurrent()
		{
			GroupSaveDialog dialog;
			if(!dialog.show())
				return;

			modelGroup.objectGroup.name = dialog.getName();
			sharedData.list.addObjectGroup(dialog.getGroup(), dialog.getSubgroup(), modelGroup.objectGroup);

			sharedData.updateGroupList();
			sharedData.modeData.updateGroups = false;
		}

		void clearSelection()
		{
			modelGroup.models.clear();
		}

		void reload()
		{
			model.reset(sharedData.storm.storm->CreateNewModel());
#ifdef LEGACY_FILES
			model->LoadS3D("Data/Models/Pointers/Circle_Full_100.s3d");
#else
			model->LoadS3D("data/model/pointer/Circle_Full_100.s3d");
#endif
			model->SetSelfIllumination(COL(.3f, .3f, .3f));
			model->CastShadows(false);
			model->SetNoCollision(true);
		}

		void reset()
		{
			model.reset();
			clearSelection();
		}
	};

} // end of unnamed namespace 

struct ObjectModeData
{
	SharedData sharedData;
	GroupCommand groupCommand;
	GroupGroupCommand groupGroupCommand;

	ModelListCommand modelListCommand;
	ClickModeCommand clickModeCommand;
	//DragModeCommand dragModeCommand;
	IlluminationModeCommand illuminationModeCommand;
	GroupModeCommand groupModeCommand;

	AmountCommand amountCommand;
	GroupListCommand groupListCommand;
	PropertyCommand propertyCommand;
	IdCommand idCommand;

	ClickMode clickMode;
	//DragMode dragMode;
	IlluminationMode illuminationMode;
	GroupMode groupMode;

	ObjectModeData(Gui &gui, Storm &storm, IEditorState &editorState)
	:	sharedData(gui, storm, editorState),
		groupCommand(sharedData, gui.getObjectsDialog()),
		groupGroupCommand(sharedData, gui.getObjectsDialog()),

		modelListCommand(sharedData, gui.getObjectsDialog()),
		clickModeCommand(sharedData, gui.getObjectsDialog()),
		//dragModeCommand(sharedData, gui.getObjectsDialog()),
		illuminationModeCommand(sharedData, gui.getObjectsDialog()),
		groupModeCommand(sharedData, gui.getObjectsDialog()),

		amountCommand(sharedData, gui.getObjectsDialog()),
		groupListCommand(sharedData, gui.getObjectsDialog()),
		propertyCommand(sharedData, gui.getObjectsDialog()),
		idCommand(sharedData, gui.getObjectsDialog()),

		clickMode(sharedData),
		//dragMode(sharedData),
		illuminationMode(sharedData),
		groupMode(sharedData)
	{
	}

	~ObjectModeData()
	{
	}
};

ObjectMode::ObjectMode(Gui &gui, Storm &storm, IEditorState &editorState)
{
	boost::scoped_ptr<ObjectModeData> tempData(new ObjectModeData(gui, storm, editorState));
	data.swap(tempData);
}

ObjectMode::~ObjectMode()
{
}

void ObjectMode::tick()
{
	ModeData &md = data->sharedData.modeData;
	if(md.currentMode == 0)
		data->clickMode.modeTick();
	//else if(md.currentMode == 1)
	//	data->dragMode.modeTick();
	else if(md.currentMode == 2)
		data->illuminationMode.modeTick();
	else if(md.currentMode == 3)
	{
		data->groupMode.modeTick();
	}

	if(md.currentMode != md.changeToMode)
	{
		if(md.currentMode == 2)
			data->illuminationMode.deactivate();

		if(md.changeToMode == 3)
		{
			EnableWindow(GetDlgItem(data->sharedData.dialog.getWindowHandle(), IDC_OBJECT_GROUP), FALSE);
			EnableWindow(GetDlgItem(data->sharedData.dialog.getWindowHandle(), IDC_OBJECT_GROUP_GROUP), TRUE);
			EnableWindow(GetDlgItem(data->sharedData.dialog.getWindowHandle(), IDC_OBJECT_SUBGROUP), FALSE);
			EnableWindow(GetDlgItem(data->sharedData.dialog.getWindowHandle(), IDC_OBJECT_GROUP_SUBGROUP), TRUE);
			EnableWindow(GetDlgItem(data->sharedData.dialog.getWindowHandle(), IDC_OBJECT_MODELS), FALSE);
			EnableWindow(GetDlgItem(data->sharedData.dialog.getWindowHandle(), IDC_OBJECT_GROUP_LIST), TRUE);

			ShowWindow(GetDlgItem(data->sharedData.dialog.getWindowHandle(), IDC_OBJECT_GROUP), SW_HIDE);
			ShowWindow(GetDlgItem(data->sharedData.dialog.getWindowHandle(), IDC_OBJECT_GROUP_GROUP), SW_SHOW);
			ShowWindow(GetDlgItem(data->sharedData.dialog.getWindowHandle(), IDC_OBJECT_SUBGROUP), SW_HIDE);
			ShowWindow(GetDlgItem(data->sharedData.dialog.getWindowHandle(), IDC_OBJECT_GROUP_SUBGROUP), SW_SHOW);
			ShowWindow(GetDlgItem(data->sharedData.dialog.getWindowHandle(), IDC_OBJECT_MODELS), SW_HIDE);
			ShowWindow(GetDlgItem(data->sharedData.dialog.getWindowHandle(), IDC_OBJECT_GROUP_LIST), SW_SHOW);

			data->sharedData.updateGroupList();
		}
		else
		{
			EnableWindow(GetDlgItem(data->sharedData.dialog.getWindowHandle(), IDC_OBJECT_GROUP), TRUE);
			EnableWindow(GetDlgItem(data->sharedData.dialog.getWindowHandle(), IDC_OBJECT_GROUP_GROUP), FALSE);
			EnableWindow(GetDlgItem(data->sharedData.dialog.getWindowHandle(), IDC_OBJECT_SUBGROUP), TRUE);
			EnableWindow(GetDlgItem(data->sharedData.dialog.getWindowHandle(), IDC_OBJECT_GROUP_SUBGROUP), FALSE);
			EnableWindow(GetDlgItem(data->sharedData.dialog.getWindowHandle(), IDC_OBJECT_MODELS), TRUE);
			EnableWindow(GetDlgItem(data->sharedData.dialog.getWindowHandle(), IDC_OBJECT_GROUP_LIST), FALSE);

			ShowWindow(GetDlgItem(data->sharedData.dialog.getWindowHandle(), IDC_OBJECT_GROUP), SW_SHOW);
			ShowWindow(GetDlgItem(data->sharedData.dialog.getWindowHandle(), IDC_OBJECT_GROUP_GROUP), SW_HIDE);
			ShowWindow(GetDlgItem(data->sharedData.dialog.getWindowHandle(), IDC_OBJECT_SUBGROUP), SW_SHOW);
			ShowWindow(GetDlgItem(data->sharedData.dialog.getWindowHandle(), IDC_OBJECT_GROUP_SUBGROUP), SW_HIDE);
			ShowWindow(GetDlgItem(data->sharedData.dialog.getWindowHandle(), IDC_OBJECT_MODELS), SW_SHOW);
			ShowWindow(GetDlgItem(data->sharedData.dialog.getWindowHandle(), IDC_OBJECT_GROUP_LIST), SW_HIDE);
		}
		
		// ToDo: Activate/deactivate mechanism?
		md.currentMode = md.changeToMode;
	}

	if(md.rebuildModels)
		data->clickMode.rebuild();
	md.rebuildModels = false;

	if(md.updateGroups)
		data->groupMode.rebuild();
	md.updateGroups = false;
}

void ObjectMode::update()
{
}

void ObjectMode::reset()
{
	data->sharedData.reset();
	data->clickMode.reset();
	data->groupMode.reset();
}

void ObjectMode::resetTerrain()
{
	data->sharedData.terrainObjects.resetTerrain();
}

void ObjectMode::restoreTerrain()
{
	data->sharedData.terrainObjects.setToTerrain();
}

void ObjectMode::hideObjects()
{
	data->sharedData.terrainObjects.hideObjects();
}

void ObjectMode::showObjects()
{
	data->sharedData.terrainObjects.showObjects();
}

void ObjectMode::updateLighting()
{
	data->sharedData.terrainObjects.updateLighting();
	data->sharedData.terrainObjects.updateLightmapStates();
}

void ObjectMode::setHelpersVisibility(bool helpersVisible)
{
	if (helpersVisible)
		data->sharedData.terrainObjects.showHelperObjects();
	else
		data->sharedData.terrainObjects.hideHelperObjects();
}

void ObjectMode::setDynamicVisibility(bool dynamicVisible)
{
	if (dynamicVisible)
		data->sharedData.terrainObjects.showDynamicObjects();
	else
		data->sharedData.terrainObjects.hideDynamicObjects();
}

void ObjectMode::setStaticVisibility(bool staticVisible)
{
	if (staticVisible)
		data->sharedData.terrainObjects.showStaticObjects();
	else
		data->sharedData.terrainObjects.hideStaticObjects();
}

void ObjectMode::setIncompleteVisibility(bool incompleteVisible)
{
	if (incompleteVisible)
		data->sharedData.terrainObjects.showIncompleteObjects();
	else
		data->sharedData.terrainObjects.hideIncompleteObjects();
}

void ObjectMode::setNoCollisionVisibility(bool noCollisionVisible)
{
	if (noCollisionVisible)
		data->sharedData.terrainObjects.showNoCollisionObjects();
	else
		data->sharedData.terrainObjects.hideNoCollisionObjects();
}

void ObjectMode::getEditorObjectStates(EditorObjectState &states) const
{
	data->sharedData.terrainObjects.getEditorObjectStates(states);
}

void ObjectMode::setEditorObjectStates(const EditorObjectState &states)
{
	data->sharedData.terrainObjects.setEditorObjectStates(states);
}

void ObjectMode::doExport(Exporter &exporter) const
{
	data->sharedData.terrainObjects.doExport(exporter);
}

filesystem::OutputStream &ObjectMode::writeStream(filesystem::OutputStream &stream) const
{
	data->sharedData.writeStream(stream);
	return stream;
}

filesystem::InputStream &ObjectMode::readStream(filesystem::InputStream &stream)
{
	data->sharedData.readStream(stream);
	return stream;
}

} // end of namespace editor
} // end of namespace frozenbyte
