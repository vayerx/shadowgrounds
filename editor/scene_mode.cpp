// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "scene_mode.h"
#include "icommand.h"
#include "command_list.h"
#include "dialog.h"
#include "dialog_utils.h"
#include "common_dialog.h"
#include "color_component.h"
#include "color_picker.h"
#include "storm.h"
#include "string_dialog.h"
#include "camera.h"
#include "gui.h"
#include "exporter.h"
#include "exporter_scene.h"
#include "../filesystem/output_stream.h"
#include "../filesystem/input_stream.h"
#include "ieditor_state.h"
#include "terrain_colormap.h"
#include "terrain_lightmap.h"
#include "editor_object_state.h"
#include "../util/FogApplier.h"

#include <istorm3d.h>
#include <istorm3d_model.h>
#include <istorm3d_mesh.h>
#include <istorm3d_terrain_renderer.h>
#include "resource/resource.h"
#include <commctrl.h>
#include <boost/scoped_array.hpp>

#pragma warning(disable: 4244) // int to float

using namespace boost;
using namespace std;

namespace frozenbyte {
namespace editor {
namespace {

	struct Fog
	{
		float start;
		float end;
		COL color;
		bool cameraCentric;
		bool enabled;

		Fog()
		:	start(0),
			end(-1.f),
			cameraCentric(false),
			enabled(false)
		{
		}
	};

	typedef std::map<std::string, Fog> FogMap;

	struct SharedData
	{
		Dialog &dialog;
		Storm &storm;
		Dialog &menu;
		IEditorState &editorState;

		ColorComponent ambientColorComponent;
		ColorComponent fogColorComponent;
		std::string backGroundModel;

		TColor<unsigned char> ambientColor;
		bool applyRealAmbient;

		int cameraRange;
		bool useRange;

		FogMap fogMap;
		bool noValueUpdate;

		bool showFolds;
		bool showGrid;
		bool showTriggers;
		bool showUnits;

		bool showHelpers;
		bool showStatic;
		bool showDynamic;
		bool showIncomplete;
		bool showNoCollision;

		bool showEditorOnly;
		util::FogApplier fogApplier;

		SharedData(Dialog &dialog_, Storm &storm_, Dialog &menu_, IEditorState &editorState_)
		:	dialog(dialog_),
			storm(storm_),
			menu(menu_),
			editorState(editorState_),

			ambientColorComponent(dialog.getWindowHandle(), 77, 102, 113, 22),
			fogColorComponent(dialog.getWindowHandle(), 240, 184, 29, 22),
			useRange(false),
			showEditorOnly(true)
		{
			noValueUpdate = false;
			applyRealAmbient = false;

			showFolds = false;
			showGrid = true;
			showTriggers = true;
			showUnits = true;

			showHelpers = true;
			showStatic = true;
			showDynamic = true;
			showIncomplete = true;
			showNoCollision = true;

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_RENDER_MODE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Textures + light buffer"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_RENDER_MODE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Light buffer"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_RENDER_MODE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Textures only"));

			//addComboString(dialog, IDC_LIGHTMAP_QUALITY, "Ultra quality");
			addComboString(dialog, IDC_LIGHTMAP_QUALITY, "Ultra quality");
			addComboString(dialog, IDC_LIGHTMAP_QUALITY, "High quality");
			addComboString(dialog, IDC_LIGHTMAP_QUALITY, "Low quality");
			addComboString(dialog, IDC_LIGHTMAP_QUALITY, "Very low quality");
			addComboString(dialog, IDC_LIGHTMAP_QUALITY, "No shadows");
			setComboIndex(dialog, IDC_LIGHTMAP_QUALITY, 1);
			addComboString(dialog, IDC_LIGHTMAP_AREA, "Whole map");
			addComboString(dialog, IDC_LIGHTMAP_AREA, "100m radius");
			addComboString(dialog, IDC_LIGHTMAP_AREA, "50m radius");
			addComboString(dialog, IDC_LIGHTMAP_AREA, "20m radius");
			addComboString(dialog, IDC_LIGHTMAP_AREA, "10m radius");
			addComboString(dialog, IDC_LIGHTMAP_AREA, "1m radius");
			setComboIndex(dialog, IDC_LIGHTMAP_AREA, 3);

			setSliderRange(dialog, IDC_SUN_YANGLE, 0, 359);
			setSliderValue(dialog, IDC_SUN_YANGLE, 50);
			setSliderRange(dialog, IDC_SUN_XANGLE, 1, 90);
			setSliderValue(dialog, IDC_SUN_XANGLE, 45);
			setSliderRange(dialog, IDC_SUN_STRENGTH, 0, 100);
			setSliderValue(dialog, IDC_SUN_STRENGTH, 0);

			setSliderRange(dialog, IDC_FOG_START, -19, 400);
			setSliderValue(dialog, IDC_FOG_START, 60);
			setSliderRange(dialog, IDC_FOG_END, -20, 400);
			setSliderValue(dialog, IDC_FOG_END, 40);

			enableCheck(dialog, IDC_LIGHTMAP_FILTER, true);
			enableCheck(dialog, IDC_REFLECTION, true);

			enableCheck(dialog, IDC_FORCE_GRID, true);
			enableCheck(dialog, IDC_SHOW_UNITS, true);
			enableCheck(dialog, IDC_SHOW_TRIGGERS, true);

			enableCheck(dialog, IDC_SHOW_HELPERS, true);
			enableCheck(dialog, IDC_SHOW_INCOMPLETE, true);
			enableCheck(dialog, IDC_SHOW_STATIC, true);
			enableCheck(dialog, IDC_SHOW_DYNAMIC, true);
			enableCheck(dialog, IDC_SHOW_NOCOLLISION, true);

			enableCheck(dialog, IDC_SHOW_EDITORONLY, true);
			reset();
		}

		void loadBackGround()
		{
			applyModel();
		}

		void updateDialog()
		{
			if(!backGroundModel.empty())
				setDialogItemText(dialog, ID_OPEN_BGMODEL, getFileName(backGroundModel));
			else
				setDialogItemText(dialog, ID_OPEN_BGMODEL, "load background model");

			setDialogItemInt(dialog, IDC_CAMERA_RANGE, cameraRange);
			ambientColorComponent.setColor(RGB(ambientColor.r, ambientColor.g, ambientColor.b));
		}

		bool hasValueChanged(int id, int *value)
		{
			assert(value);
			int newValue = getDialogItemInt(dialog, id);

			if(*value != newValue)
			{
				*value = newValue;
				return true;
			}

			return false;
		}

		void applyModel()
		{
			if(backGroundModel.empty())
				storm.scene->SetBackGround(0);
			else
			{
				IStorm3D_Model *m = storm.storm->CreateNewModel();
				m->LoadS3D(backGroundModel.c_str());
			
				storm.scene->SetBackGround(m);
			}
		}

		void applyCamera()
		{
			float range = 300.f;
			if(useRange)
				range = float(cameraRange);

			storm.scene->GetCamera()->SetVisibilityRange(range);
		}

		void applyAmbient(bool forceReal)
		{
			if(!forceReal && !applyRealAmbient)
			{
				storm.terrain->getRenderer().setFloatValue(IStorm3D_TerrainRenderer::ForceAmbient, .5f);
				storm.scene->SetAmbientLight(COL());
			}
			else
			{
				storm.terrain->getRenderer().setFloatValue(IStorm3D_TerrainRenderer::ForceAmbient, 0);
				Color c(ambientColor.r / 255.f, ambientColor.g / 255.f, ambientColor.b / 255.f);
				storm.scene->SetAmbientLight(c);
			}
		}

		void applyAll()
		{
			applyModel();
			applyCamera();
		}

		void setBackGround(const std::string &fileName)
		{
			backGroundModel = fileName;
			
			loadBackGround();
			updateDialog();
		}

		void updateValues()
		{
			if(noValueUpdate)
				return;

			int newRange = getDialogItemInt(dialog, IDC_CAMERA_RANGE);
			if(newRange < 5)
				newRange = 5;

			if(newRange != cameraRange)
				setCameraRange(newRange);
			else
			{
			}

			bool gameCamera = IsDlgButtonChecked(dialog.getWindowHandle(), IDC_FORCE_GAME_CAMERA) == BST_CHECKED;
			editorState.getCamera().forceGameCamera(gameCamera);
		}

		void setAmbientColor(unsigned char r, unsigned char g, unsigned char b)
		{
			ambientColor.r = r;
			ambientColor.g = g;
			ambientColor.b = b;

			updateDialog();
			applyAmbient(false);
		}

		void setAmbient(bool set)
		{
			applyRealAmbient = set;
			applyAmbient(false);
		}

		void setCameraRange(int range)
		{
			cameraRange = range;
			applyCamera();
		}

		int getAmbientColor()
		{
			return RGB(ambientColor.r, ambientColor.g, ambientColor.b);
		}

		void setFogList()
		{
			if(noValueUpdate)
				return;

			noValueUpdate = true;
			resetComboBox(dialog, IDC_FOG_ID);

			int defaultIndex = 0;
			int currentIndex = 0;
			for(FogMap::iterator it = fogMap.begin(); it != fogMap.end(); ++it)
			{
				addComboString(dialog, IDC_FOG_ID, it->first);
				if(it->first == "Default")
					defaultIndex = currentIndex;

				++currentIndex;
			}

			setComboIndex(dialog, IDC_FOG_ID, defaultIndex);
			noValueUpdate = false;
			
			fogApplier.setActiveFog("Default");
			setFogValues();
		}

		void addFogGroup(const std::string &id)
		{
			fogMap[id];
			setFogList();
		}

		void setFog(const std::string &id, const Fog &fog)
		{
			util::FogApplier::Fog applyFog;
			applyFog.start = fog.start;
			applyFog.end = fog.end;
			applyFog.color = fog.color;
			applyFog.cameraCentric = fog.cameraCentric;
			applyFog.enabled = fog.enabled;

			fogApplier.setFog(id, applyFog);
		}

		void setFogValues()
		{
			if(noValueUpdate)
				return;

			noValueUpdate = true;

			std::string id = getDialogItemText(dialog, IDC_FOG_ID);
			const Fog &fog = fogMap[id];

			enableCheck(dialog, IDC_FOG_ENABLED, fog.enabled);
			enableCheck(dialog, IDC_FOG_CAMERA, fog.cameraCentric);
			setSliderValue(dialog, IDC_FOG_START, int((fog.end + 10) * 2.f + 0.5f));
			setSliderValue(dialog, IDC_FOG_END, int((fog.start + 10) * 2.f + 0.5f));
			
			unsigned char r = unsigned char(fog.color.r * 255.f);
			unsigned char g = unsigned char(fog.color.g * 255.f);
			unsigned char b = unsigned char(fog.color.b * 255.f);
			fogColorComponent.setColor(RGB(r,g,b));

			setFog(id, fog);
			noValueUpdate = false;
		}

		void updateFogParams()
		{
			if(noValueUpdate)
				return;

			std::string id = getDialogItemText(dialog, IDC_FOG_ID);
			Fog &fog = fogMap[id];

			fog.enabled = IsDlgButtonChecked(dialog.getWindowHandle(), IDC_FOG_ENABLED) == BST_CHECKED;
			fog.cameraCentric = IsDlgButtonChecked(dialog.getWindowHandle(), IDC_FOG_CAMERA) == BST_CHECKED;
			fog.start = getSliderValue(dialog, IDC_FOG_END) * 0.5f - 10.f;
			fog.end = getSliderValue(dialog, IDC_FOG_START) * 0.5f - 10.f;

			DWORD color = fogColorComponent.getColor();
			float r = GetRValue(color) / 255.f;
			float g = GetGValue(color) / 255.f;
			float b = GetBValue(color) / 255.f;
			fog.color = COL(r,g,b);

			//storm.scene->SetFogParameters(fog.enabled, fog.color, fog.start, fog.end);
			setFog(id, fog);
		}

		void tick()
		{
			if(storm.scene)
			{
				std::string id = getDialogItemText(dialog, IDC_FOG_ID);
				fogApplier.setActiveFog(id);

				fogApplier.setScene(*storm.scene);
				fogApplier.update(storm.scene->GetCamera()->GetPosition().y, getTimeDelta());

				// Hacky hacky
				bool shouldShowEditorOnly = isCheckEnabled(dialog, IDC_SHOW_EDITORONLY);
				if(showEditorOnly != shouldShowEditorOnly)
				{
					if(storm.storm)
					{
						boost::scoped_ptr<Iterator<IStorm3D_Model *> > itm(storm.storm->ITModel->Begin());
						for(; !itm->IsEnd();  itm->Next())
						{
							IStorm3D_Model *model = itm->GetCurrent();
							if(!model)
								continue;

							boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > ito(model->ITObject->Begin());
							for(; !ito->IsEnd(); ito->Next())
							{
								IStorm3D_Model_Object *object = ito->GetCurrent();
								if(!object)
									continue;

								std::string name = object->GetName();
								if(name.find("EditorOnly") != name.npos)
								{
									if(shouldShowEditorOnly)
										object->SetNoRender(false);
									else
										object->SetNoRender(true);
								}
							}
						}
					}

					showEditorOnly = shouldShowEditorOnly;
				}
			}
		}

		void setLighting()
		{
			applyAmbient(false);
		}

		VC3 getSunDirection() const
		{
			float yAngle = getSliderValue(dialog, IDC_SUN_YANGLE) / 180.f * PI;
			float xAngle = -getSliderValue(dialog, IDC_SUN_XANGLE) / 180.f * PI;
			float strength = getSliderValue(dialog, IDC_SUN_STRENGTH) / 400.f;

			QUAT q;
			q.MakeFromAngles(xAngle, yAngle);
			q.Normalize();
			MAT tm;
			tm.CreateRotationMatrix(q);

			VC3 result(0, 0, strength);
			tm.RotateVector(result);

			return result;
		}

		void update()
		{
			updateDialog();
		}

		void reset()
		{
			backGroundModel = "";
			storm.scene->SetBackGround(0);

			ambientColor = TColor<unsigned char>(200,200,200);
			cameraRange = 1500;

			noValueUpdate = true;
			applyAll();
			updateDialog();

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_RENDER_MODE, CB_SETCURSEL, 0, 0);
			CheckDlgButton(dialog.getWindowHandle(), IDC_RENDER_GLOW, BST_CHECKED);
			CheckDlgButton(dialog.getWindowHandle(), IDC_RENDER_MODELS, BST_CHECKED);
			CheckDlgButton(dialog.getWindowHandle(), IDC_RENDER_HEIGHTMAP, BST_CHECKED);
			CheckDlgButton(dialog.getWindowHandle(), IDC_FREEZE_CAMERA_CULLING, BST_UNCHECKED);

			fogColorComponent.setColor(0);
			fogMap.clear();
			fogMap["Default"];
			noValueUpdate = false;

			setFogList();
		}

		void writeStream(filesystem::OutputStream &stream) const
		{
			stream << int(7);
			stream << backGroundModel;

			stream << ambientColor.r << ambientColor.g << ambientColor.b;
			stream << cameraRange;

			int yAngle = getSliderValue(dialog, IDC_SUN_YANGLE);
			int xAngle = getSliderValue(dialog, IDC_SUN_XANGLE);
			int strength = getSliderValue(dialog, IDC_SUN_STRENGTH);
			stream << yAngle << xAngle << strength;

			stream << int(fogMap.size());
			for(FogMap::const_iterator it = fogMap.begin(); it != fogMap.end(); ++it)
			{
				stream << it->first;
				const Fog &f = it->second;
				stream << f.enabled << f.start << f.end;
				stream << f.color.r << f.color.g << f.color.b << f.cameraCentric;
			}
		}

		void readStream(filesystem::InputStream &stream)
		{
			reset();
			if(stream.isEof())
				return;

			int version = 1;
			stream >> version;

			stream >> backGroundModel;
			
			unsigned char fooc = 0;
			float foof = 0.f;
			int fooi = 0;
			bool foob = false;

			if(version <= 2)
			{
				stream >> foof >> foof >> foof;
				stream >> fooc >> fooc >> fooc;

				stream >> foof >> foof;
				stream >> fooc >> fooc >> fooc;
				stream >> foob;
			}

			stream >> ambientColor.r >> ambientColor.g >> ambientColor.b;
			stream >> cameraRange;

			if(version >= 4)
			{
				int yAngle = 0;
				int xAngle = 0;
				int strength = 0;
				stream >> yAngle >> xAngle >> strength;

				if(version < 5)
					strength *= 4;

				setSliderValue(dialog, IDC_SUN_YANGLE, yAngle);
				setSliderValue(dialog, IDC_SUN_XANGLE, xAngle);
				setSliderValue(dialog, IDC_SUN_STRENGTH, strength);
			}

			if(version == 1 || version == 2)
				stream >> fooi >> fooi;
			if(version == 2)
				stream >> fooc >> fooc >> fooc;

			noValueUpdate = true;
			applyAll();
			updateDialog();
			noValueUpdate = false;

			fogMap.clear();
			if(version >= 6)
			{
				int fogs = 0;
				stream >> fogs;
				for(int i = 0; i < fogs; ++i)
				{
					std::string name;
					stream >> name;

					Fog &f = fogMap[name];
					stream >> f.enabled >> f.start >> f.end;
					stream >> f.color.r >> f.color.g >> f.color.b;

					if(version >= 7)
						stream >> f.cameraCentric;
				}

				setFogList();
			}
		}
	};

	class LoadCommand: public ICommand
	{
		SharedData &sharedData;

	public:
		LoadCommand(SharedData &sharedData_, Dialog &d)
		:	sharedData(sharedData_)
		{
			d.getCommandList().addCommand(ID_OPEN_BGMODEL, this);
		}

		void execute(int id)
		{
			std::string fileName = getOpenFileName("s3d", "Data\\Models\\SkyModels");
			if(!fileName.empty())
				sharedData.setBackGround(fileName);
		}
	};	

	class UpdateCommand: public ICommand
	{
		SharedData &sharedData;

	public:
		UpdateCommand(SharedData &sharedData_)
		:	sharedData(sharedData_)
		{
		}

		void execute(int id)
		{
			sharedData.updateValues();
		}
	};

	class ColorCommand: public ICommand
	{
		SharedData &sharedData;
		int id;

	public:
		ColorCommand(SharedData &sharedData_, int id_)
		:	sharedData(sharedData_)
		{
			id = id_;
		}

		void execute(int id2)
		{
			ColorPicker cp;
			
			if(id == 3)
			{
				int color = sharedData.getAmbientColor();
				if(cp.run(color))
				{
					int color = cp.getColor();

					unsigned char r = GetRValue(color);
					unsigned char g = GetGValue(color);
					unsigned char b = GetBValue(color);

					sharedData.setAmbientColor(r, g, b);
				}
			}
			else if(id == 2)
			{
				int color = sharedData.fogColorComponent.getColor();
				if(cp.run(color))
				{
					sharedData.fogColorComponent.setColor(cp.getColor());
					sharedData.updateFogParams();
				}
			}
		}
	};

	class ApplyRangeCommand: public ICommand
	{
		SharedData &sharedData;
		Dialog &dialog;

	public:
		ApplyRangeCommand(SharedData &sharedData_, Dialog &dialog_)
		:	sharedData(sharedData_),
			dialog(dialog_)
		{
			dialog.getCommandList().addCommand(IDC_APPLY_CAMERA_RANGE, this);
		}

		void execute(int id)
		{
			if(IsDlgButtonChecked(dialog.getWindowHandle(), IDC_APPLY_CAMERA_RANGE) == BST_CHECKED)
				sharedData.useRange = true;
			else
				sharedData.useRange = false;

			sharedData.applyCamera();
		}
	};

	class ApplyAmbientCommand: public ICommand
	{
		SharedData &sharedData;
		Dialog &dialog;

	public:
		ApplyAmbientCommand(SharedData &sharedData_, Dialog &dialog_)
		:	sharedData(sharedData_),
			dialog(dialog_)
		{
			dialog.getCommandList().addCommand(IDC_AMBIENT_ENABLE, this);
		}

		void execute(int id)
		{
			if(IsDlgButtonChecked(dialog.getWindowHandle(), IDC_AMBIENT_ENABLE) == BST_CHECKED)
				sharedData.setAmbient(true);
			else
				sharedData.setAmbient(false);
		}
	};

	class FogCommand: public ICommand
	{
		SharedData &sharedData;
		Dialog &dialog;

	public:
		FogCommand(SharedData &sharedData_, Dialog &dialog_)
		:	sharedData(sharedData_),
			dialog(dialog_)
		{
			CommandList &cs = dialog.getCommandList(WM_NOTIFY);
			
			dialog.getCommandList().addCommand(IDC_FOG_ENABLED, this);
			dialog.getCommandList().addCommand(IDC_FOG_CAMERA, this);
			cs.addCommand(IDC_FOG_START, this);
			cs.addCommand(IDC_FOG_END, this);
		}

		void execute(int id)	
		{
			if(sharedData.noValueUpdate)
				return;

			sharedData.updateFogParams();
		}
	};

	class FogIdCommand: public ICommand
	{
		SharedData &sharedData;
		Dialog &dialog;

	public:
		FogIdCommand(SharedData &sharedData_, Dialog &dialog_)
		:	sharedData(sharedData_),
			dialog(dialog_)
		{
			dialog.getCommandList().addCommand(IDC_FOG_ID, this);
		}

		void execute(int id)	
		{
			if(sharedData.noValueUpdate)
				return;

			sharedData.setFogValues();
			sharedData.updateFogParams();
		}
	};

	class FogIdAddCommand: public ICommand
	{
		SharedData &sharedData;
		Dialog &dialog;

	public:
		FogIdAddCommand(SharedData &sharedData_, Dialog &dialog_)
		:	sharedData(sharedData_),
			dialog(dialog_)
		{
			dialog.getCommandList().addCommand(IDC_FOG_ID_ADD, this);
		}

		void execute(int id)	
		{
			if(sharedData.noValueUpdate)
				return;

			StringDialog stringDialog;
			std::string newId = stringDialog.show("Gimme string");

			if(!newId.empty())
				sharedData.addFogGroup(newId);
		}
	};

	struct RendererCommand: public ICommand
	{
		SharedData &sharedData;
		Dialog &dialog;

		RendererCommand(SharedData &sharedData_, Dialog &dialog_)
		:	sharedData(sharedData_),
			dialog(dialog_)
		{
			dialog.getCommandList().addCommand(IDC_RENDER_MODE, this);
			dialog.getCommandList().addCommand(IDC_ANISO, this);
			dialog.getCommandList().addCommand(IDC_SMOOTH_SHADOWS, this);
			dialog.getCommandList().addCommand(IDC_RENDER_GLOW, this);
			dialog.getCommandList().addCommand(IDC_RENDER_DISTORTION, this);
			dialog.getCommandList().addCommand(IDC_FORCE_WIREFRAME, this);
			dialog.getCommandList().addCommand(IDC_FORCE_COLLISION, this);
			dialog.getCommandList().addCommand(IDC_FORCE_GRID, this);
			dialog.getCommandList().addCommand(IDC_SHOW_UNITS, this);
			dialog.getCommandList().addCommand(IDC_SHOW_TRIGGERS, this);
			dialog.getCommandList().addCommand(IDC_SHOW_HELPERS, this);
			dialog.getCommandList().addCommand(IDC_SHOW_STATIC, this);
			dialog.getCommandList().addCommand(IDC_SHOW_DYNAMIC, this);
			dialog.getCommandList().addCommand(IDC_SHOW_NOCOLLISION, this);
			dialog.getCommandList().addCommand(IDC_SHOW_INCOMPLETE, this);
			dialog.getCommandList().addCommand(IDC_FORCE_GRID, this);
			dialog.getCommandList().addCommand(IDC_FORCE_FOLD, this);
			dialog.getCommandList().addCommand(IDC_RENDER_MODELS, this);
			dialog.getCommandList().addCommand(IDC_RENDER_HEIGHTMAP, this);
			dialog.getCommandList().addCommand(IDC_FREEZE_CAMERA_CULLING, this);
			dialog.getCommandList().addCommand(IDC_LIGHTMAP_FILTER, this);
			dialog.getCommandList().addCommand(IDC_REFLECTION, this);
		}

		void execute(int id)
		{
			if(sharedData.noValueUpdate)
				return;

			IStorm3D_Terrain *terrain = sharedData.storm.terrain;
			if(!terrain)
				return;

			IStorm3D_TerrainRenderer &renderer = terrain->getRenderer();
			int renderMode = SendDlgItemMessage(dialog.getWindowHandle(), IDC_RENDER_MODE, CB_GETCURSEL, 0, 0);
			bool renderGlow = IsDlgButtonChecked(dialog.getWindowHandle(), IDC_RENDER_GLOW) == BST_CHECKED;
			bool renderDistortion = IsDlgButtonChecked(dialog.getWindowHandle(), IDC_RENDER_DISTORTION) == BST_CHECKED;
			bool renderWireframe = IsDlgButtonChecked(dialog.getWindowHandle(), IDC_FORCE_WIREFRAME) == BST_CHECKED;
			bool renderCollision = IsDlgButtonChecked(dialog.getWindowHandle(), IDC_FORCE_COLLISION) == BST_CHECKED;
			bool renderGrid = IsDlgButtonChecked(dialog.getWindowHandle(), IDC_FORCE_GRID) == BST_CHECKED;
			bool renderUnits = IsDlgButtonChecked(dialog.getWindowHandle(), IDC_SHOW_UNITS) == BST_CHECKED;
			bool renderTriggers = IsDlgButtonChecked(dialog.getWindowHandle(), IDC_SHOW_TRIGGERS) == BST_CHECKED;
			bool renderHelpers = IsDlgButtonChecked(dialog.getWindowHandle(), IDC_SHOW_HELPERS) == BST_CHECKED;
			bool renderIncomplete = IsDlgButtonChecked(dialog.getWindowHandle(), IDC_SHOW_INCOMPLETE) == BST_CHECKED;
			bool renderStatic = IsDlgButtonChecked(dialog.getWindowHandle(), IDC_SHOW_STATIC) == BST_CHECKED;
			bool renderDynamic = IsDlgButtonChecked(dialog.getWindowHandle(), IDC_SHOW_DYNAMIC) == BST_CHECKED;
			bool renderNoCollision = IsDlgButtonChecked(dialog.getWindowHandle(), IDC_SHOW_NOCOLLISION) == BST_CHECKED;
			bool renderFold = IsDlgButtonChecked(dialog.getWindowHandle(), IDC_FORCE_FOLD) == BST_CHECKED;
			bool renderModels = IsDlgButtonChecked(dialog.getWindowHandle(), IDC_RENDER_MODELS) == BST_CHECKED;
			bool renderHeightmap = IsDlgButtonChecked(dialog.getWindowHandle(), IDC_RENDER_HEIGHTMAP) == BST_CHECKED;
			bool freezeCameraCulling = IsDlgButtonChecked(dialog.getWindowHandle(), IDC_FREEZE_CAMERA_CULLING) == BST_CHECKED;
			bool aniso = IsDlgButtonChecked(dialog.getWindowHandle(), IDC_ANISO) == BST_CHECKED;
			bool smoothShadows = IsDlgButtonChecked(dialog.getWindowHandle(), IDC_SMOOTH_SHADOWS) == BST_CHECKED;
			bool renderReflection = IsDlgButtonChecked(dialog.getWindowHandle(), IDC_REFLECTION) == BST_CHECKED;

			if(renderMode == 0)
				renderer.setRenderMode(IStorm3D_TerrainRenderer::Normal);
			else if(renderMode == 1)
				renderer.setRenderMode(IStorm3D_TerrainRenderer::LightOnly);
			else if(renderMode == 2)
				renderer.setRenderMode(IStorm3D_TerrainRenderer::TexturesOnly);

			renderer.enableFeature(IStorm3D_TerrainRenderer::Glow, renderGlow);
			renderer.enableFeature(IStorm3D_TerrainRenderer::SmoothShadows, smoothShadows);
			renderer.enableFeature(IStorm3D_TerrainRenderer::Distortion, renderDistortion);
			renderer.enableFeature(IStorm3D_TerrainRenderer::Wireframe, renderWireframe);
			renderer.enableFeature(IStorm3D_TerrainRenderer::Collision, renderCollision);
			renderer.enableFeature(IStorm3D_TerrainRenderer::ModelRendering, renderModels);
			renderer.enableFeature(IStorm3D_TerrainRenderer::HeightmapRendering, renderHeightmap);
			renderer.enableFeature(IStorm3D_TerrainRenderer::FreezeCameraCulling, freezeCameraCulling);
			renderer.enableFeature(IStorm3D_TerrainRenderer::Reflection, renderReflection);

			sharedData.storm.scene->SetAnisotropicFilteringLevel((aniso) ? 8 : 1);

			sharedData.showGrid = renderGrid;
			sharedData.showFolds = renderFold;
			sharedData.showTriggers = renderTriggers;
			sharedData.showUnits = renderUnits;

			sharedData.showHelpers = renderHelpers;
			sharedData.showStatic = renderStatic;
			sharedData.showDynamic = renderDynamic;
			sharedData.showNoCollision = renderNoCollision;
			sharedData.showIncomplete = renderIncomplete;

			sharedData.editorState.updateGrid();
			sharedData.editorState.updateFolds();
			sharedData.editorState.updateTriggersVisibility();
			sharedData.editorState.updateUnitsVisibility();

			sharedData.editorState.updateHelpersVisibility();
			sharedData.editorState.updateIncompleteVisibility();
			sharedData.editorState.updateStaticVisibility();
			sharedData.editorState.updateDynamicVisibility();
			sharedData.editorState.updateNoCollisionVisibility();

		}
	};

	class ColorMapCommand: public ICommand
	{
		SharedData &sharedData;
		Dialog &dialog;
		Gui &gui;

		Storm &storm;

	public:
		ColorMapCommand(SharedData &sharedData_, Dialog &dialog_, Gui &gui_, Storm &storm_)
		:	sharedData(sharedData_),
			dialog(dialog_),
			gui(gui_),
			storm(storm_)
		{
			dialog.getCommandList().addCommand(IDC_CALCULATE_COLOR, this);
		}

		void start()
		{
			IStorm3D_Terrain *terrain = sharedData.storm.terrain;
			if(!terrain)
				return;

			IStorm3D_TerrainRenderer &renderer = terrain->getRenderer();
			renderer.setRenderMode(IStorm3D_TerrainRenderer::LightOnly);
			renderer.enableFeature(IStorm3D_TerrainRenderer::Glow, false);
			renderer.enableFeature(IStorm3D_TerrainRenderer::Wireframe, false);
			renderer.enableFeature(IStorm3D_TerrainRenderer::Collision, false);
			renderer.enableFeature(IStorm3D_TerrainRenderer::ModelRendering, true);
			renderer.enableFeature(IStorm3D_TerrainRenderer::HeightmapRendering, false);
			renderer.enableFeature(IStorm3D_TerrainRenderer::FreezeCameraCulling, false);
			renderer.enableFeature(IStorm3D_TerrainRenderer::AlphaTest, false);
			renderer.enableFeature(IStorm3D_TerrainRenderer::MaterialAmbient, false);

			sharedData.applyAmbient(true);
			renderer.setFloatValue(IStorm3D_TerrainRenderer::ForceAmbient, 0.f);
			storm.scene->SetAmbientLight(COL());
			storm.scene->SetFogParameters(false, COL(), 1.f, 0.f);

			sharedData.editorState.hideObjects();
		}

		void end()
		{
			sharedData.editorState.showObjects();
			sharedData.applyAmbient(false);

			SendMessage(gui.getSceneDialog().getWindowHandle(), WM_COMMAND, IDC_RENDER_HEIGHTMAP, IDC_RENDER_HEIGHTMAP);

			IStorm3D_Terrain *terrain = sharedData.storm.terrain;
			if(!terrain)
				return;

			IStorm3D_TerrainRenderer &renderer = terrain->getRenderer();
			renderer.enableFeature(IStorm3D_TerrainRenderer::AlphaTest, true);
			renderer.enableFeature(IStorm3D_TerrainRenderer::MaterialAmbient, true);
		}

		void execute(int id)
		{
			start();
			storm.scene->RenderScene(true);

			//VC2I size;
			//gui.getRenderDialog().getSize(size.x, size.y);

			TerrainColorMap &map = sharedData.editorState.getColorMap();
			map.create();

			end();
			sharedData.editorState.visualizeCompletion();
		}
	};

	class LightMapCommand: public ICommand
	{
		SharedData &sharedData;
		Dialog &dialog;
		Gui &gui;

		Storm &storm;

	public:
		LightMapCommand(SharedData &sharedData_, Dialog &dialog_, Gui &gui_, Storm &storm_)
		:	sharedData(sharedData_),
			dialog(dialog_),
			gui(gui_),
			storm(storm_)
		{
			dialog.getCommandList().addCommand(IDC_CALCULATE_LIGHTMAP, this);
		}

		void execute(int id)
		{
			int area =  getComboIndex(dialog, IDC_LIGHTMAP_AREA);
			int quality = getComboIndex(dialog, IDC_LIGHTMAP_QUALITY) - 1;
			if(quality == -1)
				quality = 4;

			sharedData.editorState.roofCollision(true);
			EditorObjectState editorObjects;
			sharedData.editorState.getEditorObjectStates(editorObjects);

			TerrainLightMap &map = sharedData.editorState.getLightMap();
			vector<TerrainLightMap::PointLight> lights;
			sharedData.editorState.getLights(lights);

			map.create(lights, area, quality, sharedData.getSunDirection());
			sharedData.editorState.updateLighting();

			sharedData.editorState.setEditorObjectStates(editorObjects);
			sharedData.editorState.roofCollision(false);
			sharedData.editorState.visualizeCompletion();
		}
	};

	class SunCommand: public ICommand
	{
		SharedData &data;
		Dialog &dialog;
		Gui &gui;

		Storm &storm;

	public:
		SunCommand(SharedData &data_, Dialog &dialog_, Gui &gui_, Storm &storm_)
		:	data(data_),
			dialog(dialog_),
			gui(gui_),
			storm(storm_)
		{
			CommandList &cs = data.dialog.getCommandList(WM_NOTIFY);
			cs.addCommand(IDC_SUN_YANGLE, this);
			cs.addCommand(IDC_SUN_XANGLE, this);
			cs.addCommand(IDC_SUN_STRENGTH, this);
		}

		void execute(int id)
		{
			// NOP
		}
	};

	class SunUpdateCommand: public ICommand
	{
		SharedData &data;

	public:
		SunUpdateCommand(SharedData &data_, Dialog &dialog)
		:	data(data_)
		{
			CommandList &c = dialog.getCommandList();
			c.addCommand(IDC_SUN_UPDATE, this);
		}

		void execute(int id)
		{
			data.editorState.updateLighting();
		}
	};

} // end of unnamed namespace

struct SceneModeData
{
	SharedData sharedData;
	LoadCommand loadCommand;
	UpdateCommand updateCommand;

	ColorCommand ambientColorCommand;
	ColorCommand fogColorCommand;
	ApplyAmbientCommand applyAmbientCommand;
	FogCommand fogCommand;
	FogIdCommand fogIdCommand;
	FogIdAddCommand fogIdAddCommand;
	ApplyRangeCommand applyRangeCommand;
	RendererCommand rendererCommand;

	ColorMapCommand colorMapCommand;
	LightMapCommand lightMapCommand;
	SunCommand sunCommand;
	SunUpdateCommand sunUpdateCommand;

	SceneModeData(Dialog &dialog, Storm &storm, Dialog &menu, IEditorState &editorState, Gui &gui)
	:	sharedData(dialog, storm, menu, editorState),

		loadCommand(sharedData, dialog),
		updateCommand(sharedData),
		ambientColorCommand(sharedData, 3),
		fogColorCommand(sharedData, 2),
		applyAmbientCommand(sharedData, dialog),
		fogCommand(sharedData, dialog),
		fogIdCommand(sharedData, dialog),
		fogIdAddCommand(sharedData, dialog),
		applyRangeCommand(sharedData, dialog),
		rendererCommand(sharedData, dialog),

		colorMapCommand(sharedData, dialog, gui, storm),
		lightMapCommand(sharedData, dialog, gui, storm),
		sunCommand(sharedData, dialog, gui, storm),
		sunUpdateCommand(sharedData, dialog)
	{
		dialog.getCommandList().addCommand(IDC_CAMERA_RANGE, &updateCommand);
		dialog.getCommandList().addCommand(IDC_FORCE_GAME_CAMERA, &updateCommand);
		dialog.getCommandList().addCommand(IDC_AMBIENT_COLORB, &ambientColorCommand);
		dialog.getCommandList().addCommand(IDC_FOG_COLOR, &fogColorCommand);
	}

	~SceneModeData()
	{
	}
};

SceneMode::SceneMode(Gui &gui, Storm &storm, IEditorState &editorState)
{
	boost::scoped_ptr<SceneModeData> tempData(new SceneModeData(gui.getSceneDialog(), storm, gui.getMenuDialog(), editorState, gui));
	data.swap(tempData);
}

SceneMode::~SceneMode()
{
}

void SceneMode::tick()
{
	data->sharedData.tick();
}

void SceneMode::reset()
{
	data->sharedData.reset();
}

void SceneMode::setLightning()
{
	data->sharedData.setLighting();
}

void SceneMode::update()
{
	data->sharedData.update();
}

VC3 SceneMode::getSunDirection() const
{
	return data->sharedData.getSunDirection();
}

void SceneMode::doExport(Exporter &exporter) const
{
	ExporterScene &scene = exporter.getScene();

	scene.setAmbient(data->sharedData.ambientColor);
	scene.setBackground(data->sharedData.backGroundModel);
	scene.setCameraRange(data->sharedData.cameraRange);
	scene.setSun(getSunDirection());

	{
		/*
		bool enabled = IsDlgButtonChecked(data->sharedData.dialog.getWindowHandle(), IDC_FOG_ENABLED) == BST_CHECKED;
		float start = getSliderValue(data->sharedData.dialog, IDC_FOG_END) * 0.5f - 10.f;
		float end = getSliderValue(data->sharedData.dialog, IDC_FOG_START) * 0.5f - 10.f;
		DWORD color = data->sharedData.fogColorComponent.getColor();
		unsigned char r = GetRValue(color);
		unsigned char g = GetGValue(color);
		unsigned char b = GetBValue(color);

		scene.setFog(enabled, TColor<unsigned char> (r,g,b), start, end);
		*/

		for(FogMap::const_iterator it = data->sharedData.fogMap.begin(); it != data->sharedData.fogMap.end(); ++it)
		{
			const Fog &f = it->second;
			unsigned char r = unsigned char(f.color.r * 255.f);
			unsigned char g = unsigned char(f.color.g * 255.f);
			unsigned char b = unsigned char(f.color.b * 255.f);

			scene.addFog(it->first, f.enabled, f.cameraCentric, TColor<unsigned char> (r, g, b), f.start, f.end);
		}
	}
}

filesystem::OutputStream &SceneMode::writeStream(filesystem::OutputStream &stream) const
{
	data->sharedData.writeStream(stream);
	return stream;
}

filesystem::InputStream &SceneMode::readStream(filesystem::InputStream &stream)
{
	data->sharedData.readStream(stream);
	return stream;
}


bool SceneMode::doesShowFolds() const
{
	return data->sharedData.showFolds;
}

bool SceneMode::doesShowGrid() const
{
	return data->sharedData.showGrid;
}

bool SceneMode::doesShowTriggers() const
{
	return data->sharedData.showTriggers;
}

bool SceneMode::doesShowHelpers() const
{
	return data->sharedData.showHelpers;
}

bool SceneMode::doesShowStatic() const
{
	return data->sharedData.showStatic;
}

bool SceneMode::doesShowDynamic() const
{
	return data->sharedData.showDynamic;
}

bool SceneMode::doesShowNoCollision() const
{
	return data->sharedData.showNoCollision;
}

bool SceneMode::doesShowIncomplete() const
{
	return data->sharedData.showIncomplete;
}

bool SceneMode::doesShowUnits() const
{
	return data->sharedData.showUnits;
}

} // end of namespace editor
} // end of namespace frozenbyte
