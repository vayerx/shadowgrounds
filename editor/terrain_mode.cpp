// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "terrain_mode.h"
#include "height_modifier.h"
#include "icommand.h"
#include "common_dialog.h"
#include "dialog.h"
#include "dialog_utils.h"
#include "command_list.h"
#include "storm.h"
#include "hmap_loader.h"
#include "texture_layer.h"
#include "terrain_textures.h"
#include "terrain_options.h"
#include "terrain_colormap.h"
#include "gui.h"
#include "ieditor_state.h"
#include "exporter.h"
#include "exporter_scene.h"
#include "mouse.h"
#include "splat_generator.h"
#include "camera.h"
#include "../filesystem/output_stream.h"
#include "../filesystem/input_stream.h"
#include "../ui/lightmanager.h"
#include "../util/mod_selector.h"

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/scoped_array.hpp>
#include <map>
#include <vector>
#include <istorm3d.h>
#include <istorm3d_model.h>
#include <istorm3d_texture.h>
#include <istorm3d_line.h>
#include <commctrl.h>
#include "resource/resource.h"

//#pragma warning(disable: 4244) // int to float

#define EDITOR_COLLISION_HEIGHTMAP_MULTIPLIER 4
#define EDITOR_OBSTACLEMAP_MULTIPLIER 2

namespace frozenbyte {
namespace editor {

extern util::ModSelector modSelector;

namespace {
	template<class T>
	struct LineDeleter
	{
		Storm &storm;

		LineDeleter(Storm &storm_)
		:	storm(storm_)
		{
		}

		void operator()(T *pointer)
		{
			storm.scene->RemoveLine(pointer);
			delete pointer;
		}
	};

	struct SharedData
	{
		Storm &storm;
		Dialog &dialog;
		Dialog &menu;
		IEditorState &editorState;

		HmapLoader hmapData;
		boost::scoped_array<unsigned short int> fooMap;

		int sizeX;
		int sizeY;
		int sizeAlt;
		int textureDetail;

		std::vector<unsigned short> heightMap;
		VC2I mapSize;
		VC3 realSize;

		TerrainTextures terrainTextures;
		bool noValueUpdate;
		bool updateMouse;

		bool pickHeight;
		int mode;
		int heightMode;
		int heightShape;

		void enableMenu(bool enableState)
		{
			enableDialogItem(menu, IDC_MENU_SCENE, enableState);
			enableDialogItem(menu, IDC_MENU_OBJECTS, enableState);
			enableDialogItem(menu, IDC_MENU_BUILDINGS, enableState);
			enableDialogItem(menu, IDC_MENU_UNIT, enableState);
			enableDialogItem(menu, IDC_MENU_DECORATORS, enableState);
		}

		bool hasValueChanged(int *value, int id)
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

		void updateDialog()
		{
			setDialogItemInt(dialog, IDC_HMAP_WIDTH, sizeX);
			setDialogItemInt(dialog, IDC_HMAP_HEIGHT, sizeY);
			setDialogItemInt(dialog, IDC_HMAP_ALT, sizeAlt);

			if(hmapData.getFileName().empty())
				setDialogItemText(dialog, ID_OPEN_HMAP, "load heightmap");
			else
				setDialogItemText(dialog, ID_OPEN_HMAP, getFileName(hmapData.getFileName()));

			setDialogItemInt(dialog, IDC_TEXTURE_DETAIL, textureDetail);

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_HMAP_TEXTURES, LB_RESETCONTENT, 0, 0);
			//SendDlgItemMessage(dialog.getWindowHandle(), IDC_SPLAT_TEXTURE, CB_RESETCONTENT, 0, 0);
			int textureAmount = terrainTextures.getTextureCount();

			for(int i = 0; i < textureAmount; ++i)
			{
				std::string string = getFileName(terrainTextures.getTexture(i));

				SendDlgItemMessage(dialog.getWindowHandle(), IDC_HMAP_TEXTURES, LB_ADDSTRING, 0, reinterpret_cast<LPARAM> (string.c_str()));
				//SendDlgItemMessage(dialog.getWindowHandle(), IDC_SPLAT_TEXTURE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> (string.c_str()));
			}

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_HMAP_TEXTURES, LB_SETCURSEL, 0, 0);
			//SendDlgItemMessage(dialog.getWindowHandle(), IDC_SPLAT_TEXTURE, CB_SETCURSEL, 0, 0);
		}

		bool hasHeightMap()
		{
			return !hmapData.getFileName().empty();
		}

		void createTexturing()
		{
			terrainTextures.applyToTerrain();
			editorState.getLightMap().apply();
		}

	public:
		SharedData(Storm &storm_, Dialog &dialog_, Dialog &menu_, IEditorState &editorState_)
		:	storm(storm_),
			dialog(dialog_),
			menu(menu_),
			editorState(editorState_),

			terrainTextures(storm),
			mode(0),
			heightMode(0),
			heightShape(0)
		{
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_EDIT_RADIUS, TBM_SETRANGE, TRUE, MAKELONG(1, 15));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_EDIT_RADIUS, TBM_SETPOS, TRUE, 3);
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_EDIT_STRENGTH, TBM_SETRANGE, TRUE, MAKELONG(1, 10));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_EDIT_STRENGTH, TBM_SETPOS, TRUE, 8);

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_HEIGHT_MODE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Raise"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_HEIGHT_MODE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Lower"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_HEIGHT_MODE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Flatten"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_HEIGHT_MODE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Smoothen"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_HEIGHT_MODE, CB_SETCURSEL, 0, 0);
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_HEIGHT_SHAPE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Circle"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_HEIGHT_SHAPE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Rectange"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_HEIGHT_SHAPE, CB_SETCURSEL, 0, 0);

			CheckDlgButton(dialog.getWindowHandle(), IDC_TERRAIN_MODE_SPLAT, BST_CHECKED);
			pickHeight = false;

			noValueUpdate = false;
			updateMouse = true;
			reset();
		}

		void setTerrain(const std::string &fileName)
		{
			HmapLoader foo(fileName);
			if(foo.getFileName().empty())
				return;

			hmapData = foo;

			editorState.updateHeightmap();
			terrainTextures.clear();
			updateDialog();
		}

		void smoothHeightmap()
		{
			if(!hasHeightMap())
				return;

			hmapData.smooth();
			editorState.updateHeightmap();
		}

		void addTerrainTexture(const std::string &fullFileName)
		{
			terrainTextures.addTexture(fullFileName);
			updateDialog();
		}

		void removeTerrainTexture()
		{
			int index = SendDlgItemMessage(dialog.getWindowHandle(), IDC_HMAP_TEXTURES, LB_GETCURSEL, 0, 0);
			terrainTextures.removeTexture(index);

			updateDialog();
		}

		void updateHmapValues()
		{
			if(noValueUpdate)
				return;

			bool changed = false;
			if(hasValueChanged(&sizeX, IDC_HMAP_WIDTH))
				changed = true;
			if(hasValueChanged(&sizeY, IDC_HMAP_HEIGHT))
				changed = true;
			if(hasValueChanged(&sizeAlt, IDC_HMAP_ALT))
				changed = true;

			if(!changed)
				return;

			editorState.updateHeightmap();
		}

		void updateTexturingValues()
		{
			if(noValueUpdate)
				return;

			int newDetail = getDialogItemInt(dialog, IDC_TEXTURE_DETAIL);
			if(newDetail == textureDetail)
				return;

			textureDetail = newDetail;
			editorState.updateHeightmap();
		}

		void updateTextureLayers()
		{
			TextureLayer tl(terrainTextures, storm);

			if(tl.show())
				editorState.updateTexturing();
		}

		void update()
		{
			if(hasHeightMap())
				enableMenu(true);
			else
				enableMenu(false);
		}

		HeightmapData loadHeightmap()
		{
			heightMap = hmapData.getData();
			mapSize = VC2I(hmapData.getWidth(), hmapData.getHeight());
			realSize = VC3(float(sizeX), float(sizeAlt), float(sizeY));

			return HeightmapData(heightMap, mapSize, realSize);
		}

		void setHeightmap()
		{
			if(storm.lightManager)
			{
				delete storm.lightManager;
			}
			if(storm.terrain)
			{
				storm.scene->RemoveTerrain(storm.terrain);
				delete storm.terrain;
			}

			storm.terrain = storm.storm->CreateNewTerrain(16);
			if(!heightMap.empty())
				storm.terrain->setHeightMap(&heightMap[0], mapSize, realSize, textureDetail, 0, EDITOR_COLLISION_HEIGHTMAP_MULTIPLIER, EDITOR_COLLISION_HEIGHTMAP_MULTIPLIER*EDITOR_OBSTACLEMAP_MULTIPLIER);

			VC2 bound = VC2(realSize.x / 2 - 5.f, realSize.z / 2 - 5.f);
			setBoundary(-bound, bound);
			storm.lightManager = new ::ui::LightManager(*storm.storm, *storm.scene, *storm.terrain, 0, 0);

			storm.heightmapResolution = mapSize;
			storm.heightmapSize = realSize;
			storm.scene->AddTerrain(storm.terrain);
		}

		void setTexturing()
		{
			createTexturing();
		}

		void reset()
		{
			sizeX = 2500;
			sizeY = 2500;
			sizeAlt = 200;
			textureDetail = 8;

			hmapData = HmapLoader();
			terrainTextures.clear();

			if(storm.scene && storm.terrain)
			{
				storm.scene->RemoveTerrain(storm.terrain);
				delete storm.terrain;
				storm.terrain = 0;
			}

			noValueUpdate = true;
			updateDialog();
			noValueUpdate = false;

			enableMenu(false);
			updateMouse = true;
		}

		void writeStream(filesystem::OutputStream &stream)
		{
			stream << int(3);
			stream << hmapData;
			stream << sizeX << sizeY << sizeAlt;
			stream << textureDetail;
			stream << terrainTextures;

			stream << editorState.getLightMap();
			stream << editorState.getColorMap();
		}

		void readStream(filesystem::InputStream &stream)
		{
			reset();
			if(stream.isEof())
				return;

			int version = 0;
			stream >> version;

			stream >> hmapData;
			stream >> sizeX >> sizeY >> sizeAlt;

			if(version == 0)
			{
				TerrainOptions terrainOptions(storm);
				stream >> terrainOptions;
			}
			if(version >= 1)
				stream >> textureDetail;
			stream >> terrainTextures;

			if(version >= 3)
				stream >> editorState.getLightMap();
			else
				editorState.getLightMap().reset();
			if(version >= 2)
				stream >> editorState.getColorMap();
			else
				editorState.getColorMap().reset();

			noValueUpdate = true;
			updateDialog();
			noValueUpdate = false;
		}
	};

	class OpenCommand: public ICommand
	{
		SharedData &sharedData;

	public:
		OpenCommand(SharedData &sharedData_)
		:	sharedData(sharedData_)
		{
		}

		void execute(int id)
		{
			std::string fileName = getOpenFileName("raw", "Data\\Heightmaps");
			if(!fileName.empty())
				sharedData.setTerrain(fileName);
		}
	};

	class SmoothCommand: public ICommand
	{
		SharedData &sharedData;

	public:
		SmoothCommand(SharedData &sharedData_)
		:	sharedData(sharedData_)
		{
		}

		void execute(int id)
		{
			sharedData.smoothHeightmap();
		}
	};

	class TextureListCommand: public ICommand
	{
		SharedData &sharedData;
		bool addMode;

	public:
		TextureListCommand(SharedData &sharedData_, bool addMode_)
		:	sharedData(sharedData_),
			addMode(addMode_)
		{
		}

		void execute(int id)
		{
			if(addMode)
			{
				modSelector.restoreDir();

#ifdef LEGACY_FILES
				std::vector<std::string> fileNames = getMultipleOpenFileName("dds", "Data\\Textures");
#else
				std::vector<std::string> fileNames = getMultipleOpenFileName("dds", "data\\texture");
#endif
				for(unsigned int i = 0; i < fileNames.size(); ++i)
				{
					if(!fileNames[i].empty())
					{
						std::string &file = fileNames[i];

						modSelector.fixFileName(file);
						sharedData.addTerrainTexture(file);
					}
				}

				modSelector.changeDir();
			}
			else
				sharedData.removeTerrainTexture();
		}
	};

	class HmapValueCommand: public ICommand
	{
		SharedData &sharedData;

	public:
		HmapValueCommand(SharedData &sharedData_, Dialog &d)
		:	sharedData(sharedData_)
		{
			CommandList &list = d.getCommandList();

			list.addCommand(IDC_HMAP_WIDTH, this);
			list.addCommand(IDC_HMAP_HEIGHT, this);
			list.addCommand(IDC_HMAP_ALT, this);
		}

		void execute(int id)
		{
			sharedData.updateHmapValues();
		}
	};

	class TextureValueCommand: public ICommand
	{
		SharedData &sharedData;

	public:
		TextureValueCommand(SharedData &sharedData_, Dialog &dialog)
		:	sharedData(sharedData_)
		{
			CommandList &list = dialog.getCommandList();
			list.addCommand(IDC_TEXTURE_DETAIL, this);
		}

		void execute(int id)
		{
			sharedData.updateTexturingValues();
		}
	};

	class TextureLayerCommand: public ICommand
	{
		SharedData &sharedData;

	public:
		TextureLayerCommand(SharedData &sharedData_, Dialog &d)
		:	sharedData(sharedData_)
		{
			CommandList &list = d.getCommandList();
			list.addCommand(IDC_HMAP_TEXTURELAYERS, this);
		}

		void execute(int id)
		{
			sharedData.updateTextureLayers();
		}
	};

	class SplatCommand: public ICommand
	{
		SharedData &sharedData;

	public:
		SplatCommand(SharedData &sharedData_, Dialog &d)
		:	sharedData(sharedData_)
		{
			d.getCommandList(WM_NOTIFY).addCommand(WM_NOTIFY, this);
			d.getCommandList(WM_NOTIFY).addCommand(IDC_EDIT_RADIUS, this);
			d.getCommandList(WM_NOTIFY).addCommand(IDC_EDIT_STRENGTH, this);
		}

		void execute(int id)
		{
			sharedData.updateMouse = true;
		}
	};

	class OptimizeCommand: public ICommand
	{
		SharedData &sharedData;

	public:
		OptimizeCommand(SharedData &sharedData_, Dialog &d)
		:	sharedData(sharedData_)
		{
			d.getCommandList().addCommand(IDC_OPTIMIZE_TEXTURES, this);
		}

		void execute(int id)
		{
			sharedData.terrainTextures.optimize();
		}
	};

	class ModeCommand: public ICommand
	{
		SharedData &data;
		Dialog &d;

	public:
		ModeCommand(SharedData &data_, Dialog &d_)
		:	data(data_),
			d(d_)
		{
			d.getCommandList().addCommand(IDC_TERRAIN_MODE_SPLAT, this);
			d.getCommandList().addCommand(IDC_TERRAIN_MODE_HEIGHTS, this);
			d.getCommandList().addCommand(IDC_TERRAIN_MODE_LIGHTMAP, this);
		}

		void execute(int id)
		{
			if(IsDlgButtonChecked(d.getWindowHandle(), IDC_TERRAIN_MODE_SPLAT) == BST_CHECKED)
				data.mode = 0;
			else if(IsDlgButtonChecked(d.getWindowHandle(), IDC_TERRAIN_MODE_HEIGHTS) == BST_CHECKED)
				data.mode = 1;
			else if(IsDlgButtonChecked(d.getWindowHandle(), IDC_TERRAIN_MODE_LIGHTMAP) == BST_CHECKED)
				data.mode = 2;
		}
	};

	class HeightCommands: public ICommand
	{
		SharedData &data;
		Dialog &d;

	public:
		HeightCommands(SharedData &data_, Dialog &d_)
		:	data(data_),
			d(d_)
		{
			d.getCommandList().addCommand(IDC_HEIGHT_PICK, this);
			d.getCommandList().addCommand(IDC_HEIGHT_MODE, this);
			d.getCommandList().addCommand(IDC_HEIGHT_SHAPE, this);
		}

		void execute(int id)
		{
			if(IsDlgButtonChecked(d.getWindowHandle(), IDC_HEIGHT_PICK) == BST_CHECKED)
				data.pickHeight = true;
			else
				data.pickHeight = false;

			data.heightMode = SendDlgItemMessage(d.getWindowHandle(), IDC_HEIGHT_MODE, CB_GETCURSEL, 0, 0);
			data.heightShape = SendDlgItemMessage(d.getWindowHandle(), IDC_HEIGHT_SHAPE, CB_GETCURSEL, 0, 0);
		}
	};

	class MouseTracker
	{
		SharedData &sharedData;
		Mouse &mouse;

		std::vector<boost::shared_ptr<IStorm3D_Line> > lines;
		float radius;

		void addLine(const VC3 &start, const VC3 &end)
		{
			boost::shared_ptr<IStorm3D_Line> line(sharedData.storm.storm->CreateNewLine(), LineDeleter<IStorm3D_Line>(sharedData.storm));

			line->AddPoint(start);
			line->AddPoint(end);
			line->SetThickness(-1.f);
			line->SetColor(RGB(0, 0, 255) | 0xFF000000);

			sharedData.storm.scene->AddLine(line.get(), false);
			lines.push_back(line);
		}

		void addCross(const VC3 &position_)
		{
			VC3 position = position_;
			position.y += 0.3f;

			Vector p1 = position - VC3(radius, 0, 0);
			Vector p2 = position + VC3(radius, 0, 0);
			Vector p3 = position - VC3(0, 0, radius);
			Vector p4 = position + VC3(0, 0, radius);

			float radius2 = .7f * radius;
			Vector p5 = position - VC3(radius2, 0, radius2);
			Vector p6 = position + VC3(radius2, 0, radius2);
			Vector p7 = position - VC3(-radius2, 0, radius2);
			Vector p8 = position + VC3(-radius2, 0, radius2);

			addLine(p1, p2);
			addLine(p3, p4);

			addLine(p5, p6);
			addLine(p7, p8);
		}

		void removeLines()
		{
			lines.clear();
		}

	public:
		MouseTracker(SharedData &sharedData_, Mouse &mouse_)
		:	sharedData(sharedData_),
			mouse(mouse_),
			radius(1.f)
		{
			update();
		}

		void update()
		{
			if(sharedData.updateMouse)
			{
				radius = float(SendDlgItemMessage(sharedData.dialog.getWindowHandle(), IDC_EDIT_RADIUS, TBM_GETPOS, 0, 0));
				sharedData.updateMouse = false;
			}
		}

		void tick()
		{
			update();
			removeLines();

			if(!sharedData.storm.terrain)
				return;

			if(!mouse.isInsideWindow())
				return;

			Vector p, d;
			VC2I screen(mouse.getX(), mouse.getY());
			sharedData.storm.scene->GetEyeVectors(screen, p, d);

			Storm3D_CollisionInfo ci;
			ObstacleCollisionInfo oi;

			ci.hit = false;
			ci.model = 0;
			oi.hit = false;

			sharedData.storm.terrain->rayTrace(p, d, 200.f, ci, oi, true);
			if(!ci.hit)
				return;

			//Storm3D_CollisionInfo ci;
			//if(!mouse.cursorRayTrace(ci))
			//	return;

			ci.position.y = sharedData.storm.terrain->getHeight(VC2(ci.position.x, ci.position.z));
			addCross(ci.position);

			// Splat
			if(sharedData.mode == 0)
			{
				if(mouse.isLeftButtonDown())
				{
					//int texture = SendDlgItemMessage(sharedData.dialog.getWindowHandle(), IDC_SPLAT_TEXTURE, CB_GETCURSEL, 0, 0);
					int texture = SendDlgItemMessage(sharedData.dialog.getWindowHandle(), IDC_HMAP_TEXTURES, LB_GETCURSEL, 0, 0);
					if(texture < 0)
						return;

					float strength = float(SendDlgItemMessage(sharedData.dialog.getWindowHandle(), IDC_EDIT_STRENGTH, TBM_GETPOS, 0, 0));
					strength /= 10.f;

					SplatGenerator splatGenerator(sharedData.storm, sharedData.terrainTextures);
					splatGenerator.generate(texture, ci.position, int(radius + .5f), strength);
				}
			}
			else if(sharedData.mode == 1)// Height edit
			{
				SharedData &data = sharedData;

				if(sharedData.pickHeight)
				{
					int height  = int(ci.position.y * 100.f);
					if(mouse.isLeftButtonDown())
						SetDlgItemInt(sharedData.dialog.getWindowHandle(), IDC_HEIGHT, height, TRUE);

					return;
				}

				if(mouse.isLeftButtonDown())
				{
					HeightModifier modifier(&data.heightMap[0], data.mapSize, data.realSize);
					HeightModifier::Shape shape = HeightModifier::Circle;
					if(data.heightShape == 1)
						shape = HeightModifier::Rectangle;

					float strength = float(SendDlgItemMessage(sharedData.dialog.getWindowHandle(), IDC_EDIT_STRENGTH, TBM_GETPOS, 0, 0));
					strength *= getTimeDelta() * 4.f;

					if(GetKeyState(VK_CONTROL) & 0x80)
						strength = -strength;

					if(data.heightMode == 0)
						modifier.changeHeight(ci.position, radius, strength, shape);
					else if(data.heightMode == 1)
						modifier.changeHeight(ci.position, radius, -strength, shape);
					else if(data.heightMode == 2)
					{
						float height = GetDlgItemInt(data.dialog.getWindowHandle(), IDC_HEIGHT, 0, TRUE) / 100.f;
						modifier.flatten(ci.position, radius, strength, height, shape);
					}
					else if(data.heightMode == 3)
					{
						modifier.smoothen(ci.position, radius, strength, shape);
					}

					data.hmapData.update(&data.heightMap[0]);
					data.storm.terrain->updateHeightMap(&data.heightMap[0], VC2I(), data.mapSize);
				}
			}
			else if(sharedData.mode == 2)// Lightmap edit
			{
				SharedData &data = sharedData;

				if(mouse.isLeftButtonDown())
				{
					HeightModifier modifier(&data.heightMap[0], data.mapSize, data.realSize);
					HeightModifier::Shape shape = HeightModifier::Circle;
					if(data.heightShape == 1)
						shape = HeightModifier::Rectangle;

					float strength = 0.1f * float(SendDlgItemMessage(sharedData.dialog.getWindowHandle(), IDC_EDIT_STRENGTH, TBM_GETPOS, 0, 0));

					TerrainLightMap &map = sharedData.editorState.getLightMap();
					std::vector<TerrainLightMap::PointLight> lights;
					sharedData.editorState.getLights(lights);

					if(GetKeyState(VK_CONTROL) & 0x80)
						strength = -strength;

					VC2 rect(2000 * radius, 2000 * radius);
					map.setShadow(VC2(ci.position.x, ci.position.z), strength, rect, lights, sharedData.editorState.getSunDirection());
					map.apply();
				}
			}
		}
	};
}

struct TerrainModeData
{
	SharedData sharedData;

	OpenCommand openCommand;
	SmoothCommand smoothCommand;

	TextureListCommand addTexture;
	TextureListCommand removeTexture;

	HmapValueCommand hmapValueCommand;
	TextureValueCommand textureValueCommand;
	TextureLayerCommand textureLayerCommand;

	SplatCommand splatCommand;
	OptimizeCommand optimizeCommand;
	ModeCommand modeCommand;
	HeightCommands heightCommands;

	MouseTracker mouseTracker;

	TerrainModeData(Storm &storm, Dialog &dialog, Dialog &menu, IEditorState &editorState, Gui &gui)
	:	sharedData(storm, dialog, menu, editorState),

		openCommand(sharedData),
		smoothCommand(sharedData),

		addTexture(sharedData, true),
		removeTexture(sharedData, false),

		hmapValueCommand(sharedData, dialog),
		textureValueCommand(sharedData, dialog),
		textureLayerCommand(sharedData, dialog),

		splatCommand(sharedData, dialog),
		optimizeCommand(sharedData, dialog),
		modeCommand(sharedData, dialog),
		heightCommands(sharedData, dialog),

		mouseTracker(sharedData, gui.getMouse())
	{
	}

	~TerrainModeData()
	{
	}
};

TerrainMode::TerrainMode(Gui &gui, Storm &storm, IEditorState &editorState)
{
	boost::scoped_ptr<TerrainModeData> tempData(new TerrainModeData(storm, gui.getTerrainDialog(), gui.getMenuDialog(), editorState, gui));
	data.swap(tempData);

	Dialog &dialog = gui.getTerrainDialog();

	dialog.getCommandList().addCommand(ID_OPEN_HMAP, &data->openCommand);
	dialog.getCommandList().addCommand(IDC_HMAP_SMOOTH, &data->smoothCommand);
	dialog.getCommandList().addCommand(IDC_HMAP_ADD, &data->addTexture);
	dialog.getCommandList().addCommand(IDC_HMAP_REMOVE, &data->removeTexture);
}

TerrainMode::~TerrainMode()
{
}

void TerrainMode::tick()
{
	data->mouseTracker.tick();
}

void TerrainMode::reset()
{
	data->sharedData.reset();
}

void TerrainMode::update()
{
	data->sharedData.update();
}

HeightmapData TerrainMode::loadHeightmap()
{
	return data->sharedData.loadHeightmap();
}

void TerrainMode::setHeightmap()
{
	data->sharedData.setHeightmap();
}

void TerrainMode::setTexturing()
{
	data->sharedData.setTexturing();
}

void TerrainMode::doExport(Exporter &exporter) const
{
	data->sharedData.terrainTextures.doExport(exporter);
	exporter.getScene().setTextureRepeat(data->sharedData.textureDetail);

	data->sharedData.editorState.getColorMap().doExport(exporter);
	data->sharedData.editorState.getLightMap().doExport(exporter);
}

filesystem::OutputStream &TerrainMode::writeStream(filesystem::OutputStream &stream) const
{
	data->sharedData.writeStream(stream);
	return stream;
}

filesystem::InputStream &TerrainMode::readStream(filesystem::InputStream &stream)
{
	data->sharedData.readStream(stream);
	return stream;
}

} // end of namespace editor
} // end of namespace frozenbyte
