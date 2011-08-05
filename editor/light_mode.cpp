// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "light_mode.h"
#include "terrain_lights.h"
#include "gui.h"
#include "mouse.h"
#include "storm.h"
#include "dialog.h"
#include "dialog_utils.h"
#include "icommand.h"
#include "command_list.h"
#include "common_dialog.h"
#include "color_component.h"
#include "color_picker.h"
#include "editor_state.h"
#include "terrain_colormap.h"
#include "../filesystem/output_stream.h"
#include "../filesystem/input_stream.h"
#include "../ui/lightmanager.h"
#include "resource/resource.h"
#include <istorm3d_model.h>
#include <istorm3d_scene.h>
#include <boost/lexical_cast.hpp>

using namespace boost;
using namespace std;

namespace frozenbyte {
namespace editor {
namespace {

std::string makeString(ui::SpotProperties::LightingModelType type)
{
	string result;
	if(type == ui::SpotProperties::Directional)
		result = "Directional";
	else if(type == ui::SpotProperties::Flat)
		result = "Flat";
	else if(type == ui::SpotProperties::Pointlight)
		result = "Pointlight";
	else
	{
		assert(!"Invalid lighting model type");
		return "";
	}
	return result;
}

class IUpdate
{
public:
	virtual ~IUpdate() {}

	virtual void update() = 0;
};

void initialize(Dialog &dialog)
{
	addComboString(dialog, IDC_LIGHT_TYPE, "Lighting");
	addComboString(dialog, IDC_LIGHT_TYPE, "Shadow caster");
	setComboIndex(dialog, IDC_LIGHT_TYPE, 0);

	setSliderRange(dialog, IDC_LIGHT_RANGE, 1, 45);
	setSliderValue(dialog, IDC_LIGHT_RANGE, 5);
	setSliderRange(dialog, IDC_LIGHT_FOV, 1, 130);
	setSliderValue(dialog, IDC_LIGHT_FOV, 120);
	setSliderRange(dialog, IDC_LIGHT_HEIGHT, 1, 20);
	setSliderValue(dialog, IDC_LIGHT_HEIGHT, 10);
	setSliderRange(dialog, IDC_LIGHT_ANGLE, -88, 88);
	setSliderValue(dialog, IDC_LIGHT_ANGLE, 45);

	setSliderRange(dialog, IDC_LIGHT_BLINK_SPEED, 1, 10000);
	setSliderValue(dialog, IDC_LIGHT_BLINK_SPEED, 5000);
	setSliderRange(dialog, IDC_LIGHT_ROTATE_RANGE, 0, 100);
	setSliderValue(dialog, IDC_LIGHT_ROTATE_RANGE, 0);
	setSliderRange(dialog, IDC_LIGHT_ROTATE_SPEED, 1, 10000);
	setSliderValue(dialog, IDC_LIGHT_ROTATE_SPEED, 5000);
	setSliderRange(dialog, IDC_LIGHT_FADE_SPEED, 1, 10000);
	setSliderValue(dialog, IDC_LIGHT_FADE_SPEED, 5000);
	setSliderRange(dialog, IDC_LIGHT_CONE_MUL, 0, 10);
	setSliderValue(dialog, IDC_LIGHT_CONE_MUL, 0);
	setSliderRange(dialog, IDC_LIGHT_SMOOTHNESS, 0, 100);
	setSliderValue(dialog, IDC_LIGHT_SMOOTHNESS, 50);

	enableCheck(dialog, IDC_LIGHT_SHADOW, true);

	setSliderRange(dialog, IDC_LIGHT_PLANE_R, 1, 50);
	setSliderValue(dialog, IDC_LIGHT_PLANE_R, 50);
	setSliderRange(dialog, IDC_LIGHT_PLANE_G, 1, 50);
	setSliderValue(dialog, IDC_LIGHT_PLANE_G, 50);
	setSliderRange(dialog, IDC_LIGHT_PLANE_B, 1, 50);
	setSliderValue(dialog, IDC_LIGHT_PLANE_B, 50);
	setSliderRange(dialog, IDC_LIGHT_PLANE_Y, 1, 50);
	setSliderValue(dialog, IDC_LIGHT_PLANE_Y, 50);
	setSliderRange(dialog, IDC_LIGHT_STRENGTH, 1, 15);
	setSliderValue(dialog, IDC_LIGHT_STRENGTH, 4);
	setSliderRange(dialog, IDC_LIGHT_SOURCE_HEIGHT2, 1, 20);
	setSliderValue(dialog, IDC_LIGHT_SOURCE_HEIGHT2, 10);

	for(int i = 0; i < 10; ++i)
	{
		string number = lexical_cast<string> (i);
		addComboString(dialog, IDC_LIGHT_GROUP_C, number);
		addComboString(dialog, IDC_LIGHT_GROUP_E, number);
	}

	setComboIndex(dialog, IDC_LIGHT_GROUP_C, 0);
	setComboIndex(dialog, IDC_LIGHT_GROUP_E, 0);

	addComboString(dialog, IDC_LIGHT_PRIORITY, "Normal");
	addComboString(dialog, IDC_LIGHT_PRIORITY, "Low");
	setComboIndex(dialog, IDC_LIGHT_PRIORITY, 0);

	enableCheck(dialog, IDC_LIGHT_VISUALIZATION, false);
	enableCheck(dialog, IDC_LIGHT_SHADOWS, true);
	enableCheck(dialog, IDC_LIGHT_GROUP_ENABLE, true);

	setSliderRange(dialog, IDC_LIGHT_DETAIL, 0, 2);
	setSliderValue(dialog, IDC_LIGHT_DETAIL, 2);

	for(int i = 0; i < LIGHT_MAX_AMOUNT; ++i)
	{
		string number = lexical_cast<string> (i + 1);
		addComboString(dialog, IDC_LIGHT_MAX_AMOUNT, number);
	}
	setComboIndex(dialog, IDC_LIGHT_MAX_AMOUNT, LIGHT_MAX_AMOUNT - 1);

	for(int i = 0; i < ui::SpotProperties::NumLightingModelTypes; i++)
	{
		addComboString(dialog, IDC_LIGHT_LIGHTINGMODEL_TYPE, makeString((ui::SpotProperties::LightingModelType)i));
	}
	setComboIndex(dialog, IDC_LIGHT_LIGHTINGMODEL_TYPE, 1);
	enableCheck(dialog, IDC_LIGHT_LIGHTINGMODEL_FADE, true);
}

static const int planeOffset = 435;

struct Shared
{
	Gui &gui;
	Storm &storm;
	IEditorState &state;
	Dialog &dialog;

	TerrainLights lights;
	IUpdate *spotPropertyObserver;

	string texture;
	string coneTexture;
	string lightModel;
	int colorValue;

	ColorComponent color;
	ColorComponent redPlane;
	ColorComponent greenPlane;
	ColorComponent bluePlane;
	ColorComponent yellowPlane;

	int active;
	bool noUpdate;

	Shared(Gui &gui_, Storm &storm_, IEditorState &state_)
	:	gui(gui_),
		storm(storm_),
		state(state_),
		dialog(gui.getLightDialog()),
		lights(storm, state),
		spotPropertyObserver(0),
		colorValue(RGB(255,255,255)),
		color(dialog.getWindowHandle(), 77, 127, 67, 20),

		redPlane(dialog.getWindowHandle(), 25, planeOffset, 35, 15),
		greenPlane(dialog.getWindowHandle(), 25, planeOffset + 16, 35, 15),
		bluePlane(dialog.getWindowHandle(), 25, planeOffset + 32, 35, 15),
		yellowPlane(dialog.getWindowHandle(), 25, planeOffset + 48, 35, 15),

		active(-1),
		noUpdate(false)
	{
		color.setColor(colorValue);
		redPlane.setColor(RGB(255,0,0));
		greenPlane.setColor(RGB(0,255,0));
		bluePlane.setColor(RGB(0,0,255));
		yellowPlane.setColor(RGB(255,255,0));

		initialize(dialog);
		setActive(-1);
	}

	void updateSpotProperties()
	{
		if(noUpdate)
			return;

		if(spotPropertyObserver)
			spotPropertyObserver->update();
	}

	void setActive(int index)
	{
		updateSpotProperties();
		active = index;

		setProperties(index);
		lights.setActiveLight(index);

		if(active == -1)
			enableDialogItem(dialog, IDC_LIGHT_DELETE, false);
		else
			enableDialogItem(dialog, IDC_LIGHT_DELETE, true);
	}

	void setColor(unsigned int value)
	{
		colorValue = value;
		color.setColor(colorValue);

		updateSpotProperties();
	}

	void setTexture(const string &texture_)
	{
		texture = texture_;

		string info = "Texture: ";
		info += getFileName(texture);
		setDialogItemText(dialog, IDC_LIGHT_TEXTURE, info);

		updateSpotProperties();
	}

	void setConeTexture(const string &texture_)
	{
		coneTexture = texture_;

		string info = "Cone: ";
		info += getFileName(coneTexture);
		setDialogItemText(dialog, IDC_LIGHT_CONE_TEXTURE, info);

		updateSpotProperties();
	}

	void setLightModel(const string &model)
	{
		lightModel = model;

		string info = "Model: ";
		info += getFileName(lightModel);
		setDialogItemText(dialog, IDC_LIGHT_MODEL, info);

		updateSpotProperties();
	}

	void setProperties(int index)
	{
		if(index < 0)
			return;

		noUpdate = true;

		const TerrainLights::SpotProperties &p = lights.getProperties(index);
		setComboIndex(dialog, IDC_LIGHT_TYPE, p.type);
		setSliderValue(dialog, IDC_LIGHT_RANGE, int(p.range));
		setSliderValue(dialog, IDC_LIGHT_FOV, int(p.fov / PI * 180.f + .5f));
		setSliderValue(dialog, IDC_LIGHT_HEIGHT, int(p.height * 2.f + .5f));
		setSliderValue(dialog, IDC_LIGHT_SOURCE_HEIGHT2, int(p.sourceHeight * 2.f + .5f));
		setSliderValue(dialog, IDC_LIGHT_ANGLE, int(p.angle / PI * 180.f + .5f));
		setComboIndex(dialog, IDC_LIGHT_GROUP_C, p.group);
		setComboIndex(dialog, IDC_LIGHT_PRIORITY, p.priority - 1);

		colorValue = RGB(unsigned char(p.color.r * 255), unsigned char(p.color.g * 255), unsigned char(p.color.b * 255));
		setColor(colorValue);

		enableCheck(dialog, IDC_LIGHT_BLINK, p.blink);
		setSliderValue(dialog, IDC_LIGHT_BLINK_SPEED, p.blinkTime);
		enableCheck(dialog, IDC_LIGHT_ROTATE, p.rotate);
		setSliderValue(dialog, IDC_LIGHT_ROTATE_SPEED, p.rotateTime);
		setSliderValue(dialog, IDC_LIGHT_ROTATE_RANGE, int(p.rotateRange * 100.f + 0.5f));
		enableCheck(dialog, IDC_LIGHT_FADE, p.fade);
		setSliderValue(dialog, IDC_LIGHT_FADE_SPEED, p.fadeTime);
		setSliderValue(dialog, IDC_LIGHT_CONE_MUL, int(p.cone * 10.f + .5f));
		setSliderValue(dialog, IDC_LIGHT_SMOOTHNESS, int(p.smoothness * 10.f + .5f));
		enableCheck(dialog, IDC_LIGHT_SHADOW, p.shadow);

		setTexture(p.texture),
		setConeTexture(p.coneTexture),
		setLightModel(p.lightModel),

		setSliderValue(dialog, IDC_LIGHT_PLANE_R, int(p.minPlane.x * 50.f + .5f));
		setSliderValue(dialog, IDC_LIGHT_PLANE_G, int(p.minPlane.y * 50.f + .5f));
		setSliderValue(dialog, IDC_LIGHT_PLANE_B, int(p.maxPlane.x * 50.f + .5f));
		setSliderValue(dialog, IDC_LIGHT_PLANE_Y, int(p.maxPlane.y * 50.f + .5f));
		setSliderValue(dialog, IDC_LIGHT_STRENGTH, int(p.strength * 15.f));
		enableCheck(dialog, IDC_LIGHT_LIGHTMAPPED, p.lightMapped);
		enableCheck(dialog, IDC_LIGHT_POINTLIGHT, p.pointLight);
		enableCheck(dialog, IDC_LIGHT_BUILDING, p.building);
		
		//setDialogItemInt(dialog, IDC_LIGHT_OFFSET, int(p.heightOffset + 0.5f));
		std::string offsetText = boost::lexical_cast<std::string> (p.heightOffset);
		setDialogItemText(dialog, IDC_LIGHT_OFFSET, offsetText);

		enableCheck(dialog, IDC_LIGHT_LIGHTINGMODEL_FADE, p.lightingModelFade);
		setComboIndex(dialog, IDC_LIGHT_LIGHTINGMODEL_TYPE, p.lightingModelType);

		noUpdate = false;
	}

	TerrainLights::SpotProperties getProperties() const
	{
		TerrainLights::SpotProperties result;
		result.type = (TerrainLights::SpotProperties::Type) getComboIndex(dialog, IDC_LIGHT_TYPE);
		result.range = float(getSliderValue(dialog, IDC_LIGHT_RANGE));
		result.fov = getSliderValue(dialog, IDC_LIGHT_FOV) / 180.f * PI;
		result.height = float(getSliderValue(dialog, IDC_LIGHT_HEIGHT) / 2.f);
		result.sourceHeight = float(getSliderValue(dialog, IDC_LIGHT_SOURCE_HEIGHT2) / 2.f);
		result.angle = getSliderValue(dialog, IDC_LIGHT_ANGLE) / 180.f * PI;
		result.group = getComboIndex(dialog, IDC_LIGHT_GROUP_C);
		result.priority = getComboIndex(dialog, IDC_LIGHT_PRIORITY) + 1;

		result.color.r = GetRValue(colorValue) / 255.f;
		result.color.g = GetGValue(colorValue) / 255.f;
		result.color.b = GetBValue(colorValue) / 255.f;

		result.blink = isCheckEnabled(dialog, IDC_LIGHT_BLINK);
		result.blinkTime = getSliderValue(dialog, IDC_LIGHT_BLINK_SPEED);
		result.rotate = isCheckEnabled(dialog, IDC_LIGHT_ROTATE);
		result.rotateTime = getSliderValue(dialog, IDC_LIGHT_ROTATE_SPEED);
		result.rotateRange = getSliderValue(dialog, IDC_LIGHT_ROTATE_RANGE) / 100.f;
		result.fade = isCheckEnabled(dialog, IDC_LIGHT_FADE);
		result.fadeTime = getSliderValue(dialog, IDC_LIGHT_FADE_SPEED);
		result.cone = getSliderValue(dialog, IDC_LIGHT_CONE_MUL) / 10.f;
		result.smoothness = getSliderValue(dialog, IDC_LIGHT_SMOOTHNESS) / 10.f;
		result.shadow = isCheckEnabled(dialog, IDC_LIGHT_SHADOW);

		result.texture = texture;
		result.coneTexture = coneTexture;
		result.lightModel = lightModel;

		result.minPlane.x = float(getSliderValue(dialog, IDC_LIGHT_PLANE_R) / 50.f);
		result.minPlane.y = float(getSliderValue(dialog, IDC_LIGHT_PLANE_G) / 50.f);
		result.maxPlane.x = float(getSliderValue(dialog, IDC_LIGHT_PLANE_B) / 50.f);
		result.maxPlane.y = float(getSliderValue(dialog, IDC_LIGHT_PLANE_Y) / 50.f);
		result.strength = float(getSliderValue(dialog, IDC_LIGHT_STRENGTH) / 15.f);
		result.lightMapped = isCheckEnabled(dialog, IDC_LIGHT_LIGHTMAPPED);
		result.pointLight = isCheckEnabled(dialog, IDC_LIGHT_POINTLIGHT);
		result.building = isCheckEnabled(dialog, IDC_LIGHT_BUILDING);
		
		result.lightingModelType = (TerrainLights::SpotProperties::LightingModelType) getComboIndex(dialog, IDC_LIGHT_LIGHTINGMODEL_TYPE);
		result.lightingModelFade = isCheckEnabled(dialog, IDC_LIGHT_LIGHTINGMODEL_FADE);
		
		std::string offsetText = getDialogItemText(dialog, IDC_LIGHT_OFFSET);
		try
		{
			result.heightOffset = boost::lexical_cast<float> (offsetText);
		}
		catch(...)
		{
			result.heightOffset = 0;
		}

		return result;
	}
};

class PropertyCommand: public ICommand
{
	Shared &shared;

public:
	PropertyCommand(Shared &shared_)
	:	shared(shared_)
	{
		CommandList &c = shared.dialog.getCommandList();
		CommandList &cs = shared.dialog.getCommandList(WM_NOTIFY);

		c.addCommand(IDC_LIGHT_TYPE, this);
		cs.addCommand(IDC_LIGHT_RANGE, this);
		cs.addCommand(IDC_LIGHT_HEIGHT, this);
		cs.addCommand(IDC_LIGHT_FOV, this);
		cs.addCommand(IDC_LIGHT_ANGLE, this);
		c.addCommand(IDC_LIGHT_GROUP_C, this);
		cs.addCommand(IDC_LIGHT_PRIORITY, this);
		c.addCommand(IDC_LIGHT_BLINK, this);
		cs.addCommand(IDC_LIGHT_BLINK_SPEED, this);
		c.addCommand(IDC_LIGHT_ROTATE, this);
		cs.addCommand(IDC_LIGHT_ROTATE_SPEED, this);
		cs.addCommand(IDC_LIGHT_ROTATE_RANGE, this);
		c.addCommand(IDC_LIGHT_FADE, this);
		cs.addCommand(IDC_LIGHT_FADE_SPEED, this);
		cs.addCommand(IDC_LIGHT_CONE_MUL, this);
		cs.addCommand(IDC_LIGHT_SMOOTHNESS, this);
		c.addCommand(IDC_LIGHT_SHADOW, this);
		cs.addCommand(IDC_LIGHT_PLANE_R, this);
		cs.addCommand(IDC_LIGHT_PLANE_G, this);
		cs.addCommand(IDC_LIGHT_PLANE_B, this);
		cs.addCommand(IDC_LIGHT_PLANE_Y, this);
		cs.addCommand(IDC_LIGHT_STRENGTH, this);
		cs.addCommand(IDC_LIGHT_SOURCE_HEIGHT2, this);
		c.addCommand(IDC_LIGHT_LIGHTMAPPED, this);
		c.addCommand(IDC_LIGHT_POINTLIGHT, this);
		c.addCommand(IDC_LIGHT_BUILDING, this);
		c.addCommand(IDC_LIGHT_OFFSET, this);
		c.addCommand(IDC_LIGHT_LIGHTINGMODEL_TYPE, this);
		c.addCommand(IDC_LIGHT_LIGHTINGMODEL_FADE, this);
	}

	void execute(int id)
	{
		shared.updateSpotProperties();
	}
};

class ColorCommand: public ICommand
{
	Shared &shared;

public:
	ColorCommand(Shared &shared_)
	:	shared(shared_)
	{
		CommandList &c = shared.dialog.getCommandList();
		c.addCommand(IDC_LIGHT_COLOR, this);
	}

	void execute(int id)
	{
		if(shared.noUpdate)
			return;

		ColorPicker picker;
		if(picker.run(shared.colorValue))
			shared.setColor(picker.getColor());
	}
};

class ColorGuessCommand : public ICommand
{
	Shared &shared;

public:
	ColorGuessCommand(Shared &shared_)
	:	shared(shared_)
	{
		CommandList &c = shared.dialog.getCommandList();
		c.addCommand(IDC_LIGHT_COLOR_GUESS, this);
	}

	void execute(int id)
	{
		if(shared.noUpdate)
			return;

		if(shared.active >= 0)
		{
			const VC2 &position = shared.lights.getPosition(shared.active);
			COL ambient = shared.state.getColorMap().getColor(position) + shared.state.getLightMap().getColor(position);
		
			ambient.Clamp();
			ambient *= 255.f;
			ambient *= 1.9f;

			ambient.r = min(255.f, ambient.r);
			ambient.g = min(255.f, ambient.g);
			ambient.b = min(255.f, ambient.b);

			DWORD val = RGB(ambient.r, ambient.g, ambient.b);
			shared.setColor(val);
		}
	}
};

class TextureCommand: public ICommand
{
	Shared &shared;

public:
	TextureCommand(Shared &shared_)
	:	shared(shared_)
	{
		CommandList &c = shared.dialog.getCommandList();
		c.addCommand(IDC_LIGHT_TEXTURE, this);
	}

	void execute(int id)
	{
		if(shared.noUpdate)
			return;

#ifdef LEGACY_FILES
		string fileName = getOpenFileName("*.*", "Data\\Textures\\Projective_Lights");
#else
		string fileName = getOpenFileName("*.*", "data\\texture\\light\\projective");
#endif
		if(!fileName.empty())
			shared.setTexture(fileName);
	}
};

class ConeTextureCommand: public ICommand
{
	Shared &shared;

public:
	ConeTextureCommand(Shared &shared_)
	:	shared(shared_)
	{
		CommandList &c = shared.dialog.getCommandList();
		c.addCommand(IDC_LIGHT_CONE_TEXTURE, this);
	}

	void execute(int id)
	{
		if(shared.noUpdate)
			return;

#ifdef LEGACY_FILES
		string fileName = getOpenFileName("*.*", "Data\\Textures\\Projective_Lights");
#else
		string fileName = getOpenFileName("*.*", "data\\texture\\light\\projective");
#endif
		if(!fileName.empty())
			shared.setConeTexture(fileName);
	}
};

class EraseConeTextureCommand: public ICommand
{
	Shared &shared;

public:
	EraseConeTextureCommand(Shared &shared_)
	:	shared(shared_)
	{
		CommandList &c = shared.dialog.getCommandList();
		c.addCommand(IDC_LIGHT_CONE_TEXTURE_ERASE, this);
	}

	void execute(int id)
	{
		if(shared.noUpdate)
			return;

		shared.setConeTexture("");
	}
};

class LightModelCommand: public ICommand
{
	Shared &shared;

public:
	LightModelCommand(Shared &shared_)
	:	shared(shared_)
	{
		CommandList &c = shared.dialog.getCommandList();
		c.addCommand(IDC_LIGHT_MODEL, this);
	}

	void execute(int id)
	{
		if(shared.noUpdate)
			return;

		string fileName = getOpenFileName("s3d", "Data\\Models");
		if(!fileName.empty())
			shared.setLightModel(fileName);
	}
};

class EraseLightModelCommand: public ICommand
{
	Shared &shared;

public:
	EraseLightModelCommand(Shared &shared_)
	:	shared(shared_)
	{
		CommandList &c = shared.dialog.getCommandList();
		c.addCommand(IDC_LIGHT_MODEL_ERASE, this);
	}

	void execute(int id)
	{
		if(shared.noUpdate)
			return;

		shared.setLightModel("");
	}
};

class DeleteCommand: public ICommand
{
	Shared &shared;

public:
	DeleteCommand(Shared &shared_)
	:	shared(shared_)
	{
		CommandList &c = shared.dialog.getCommandList();
		c.addCommand(IDC_LIGHT_DELETE, this);
	}

	void execute(int id)
	{
		int index = shared.active;
		if(index < 0)
			return;

		shared.setActive(-1);
		shared.lights.deleteSpot(index);
	}
};

class DisplayCommand: public ICommand
{
	Shared &shared;

public:
	DisplayCommand(Shared &shared_)
	:	shared(shared_)
	{
		CommandList &c = shared.dialog.getCommandList();
		CommandList &cs = shared.dialog.getCommandList(WM_NOTIFY);

		c.addCommand(IDC_LIGHT_GROUP_ENABLE, this);
		c.addCommand(IDC_LIGHT_SHADOWS, this);
		c.addCommand(IDC_LIGHT_VISUALIZATION, this);
		cs.addCommand(IDC_LIGHT_DETAIL, this);
	}

	void execute(int id)
	{
		bool shadows = isCheckEnabled(shared.dialog, IDC_LIGHT_SHADOWS);
		bool visualization = isCheckEnabled(shared.dialog, IDC_LIGHT_VISUALIZATION);
		int detail = getSliderValue(shared.dialog, IDC_LIGHT_DETAIL) * 50;

		int group = getComboIndex(shared.dialog, IDC_LIGHT_GROUP_E);
		bool enable = isCheckEnabled(shared.dialog, IDC_LIGHT_GROUP_ENABLE);

		shared.lights.setProperty(TerrainLights::RenderShadows, shadows);
		shared.lights.setProperty(TerrainLights::RenderVisualization, visualization);
		shared.lights.setShadowLevel(detail);
		shared.lights.enableGroup(group, enable);
	}
};

class GroupChangeCommand: public ICommand
{
	Shared &shared;

public:
	GroupChangeCommand(Shared &shared_)
	:	shared(shared_)
	{
		CommandList &c = shared.dialog.getCommandList();
		c.addCommand(IDC_LIGHT_GROUP_E, this);
	}

	void execute(int id)
	{
		int group = getComboIndex(shared.dialog, IDC_LIGHT_GROUP_E);
		bool enable = shared.lights.isGroupEnabled(group);
		
		enableCheck(shared.dialog, IDC_LIGHT_GROUP_ENABLE, enable);
	}
};

class SyncCommand: public ICommand
{
	Shared &shared;

public:
	SyncCommand(Shared &shared_)
	:	shared(shared_)
	{
		CommandList &c = shared.dialog.getCommandList();
		c.addCommand(IDC_LIGHT_SYNC, this);
	}

	void execute(int id)
	{
		int lightAmount = getComboIndex(shared.dialog, IDC_LIGHT_MAX_AMOUNT) + 1;
		shared.storm.lightManager->setMaxLightAmount(lightAmount);

		shared.state.updateLighting();
	}
};

struct MouseTracker: public IUpdate
{
	Shared &shared;
	Mouse &mouse;

	shared_ptr<IStorm3D_Model> model;
	float rotation;
	float heightOffset;

	MouseTracker(Shared &shared_, Mouse &mouse_)
	:	shared(shared_),
		mouse(mouse_),
		rotation(0),
		heightOffset(0)
	{
	}

	void tick()
	{
		if(!model)
			return;

		Storm &storm = shared.storm;
		shared.storm.scene->RemoveModel(model.get());

		if(!mouse.isInsideWindow())
			return;

		Storm3D_CollisionInfo ci;
		Vector p, d;
		if(!mouse.cursorRayTrace(ci, &p, &d))
			return;

		int wheelDelta = mouse.getWheelDelta();
		rotation = shared.storm.unitAligner.getRotation(rotation, wheelDelta);

		if(GetKeyState(VK_PRIOR) & 0x80)
			heightOffset = shared.storm.unitAligner.getHeight(heightOffset, 1);
		else if(GetKeyState(VK_NEXT) & 0x80)
			heightOffset = shared.storm.unitAligner.getHeight(heightOffset, -1);

		int active = shared.lights.traceActiveCollision(p, d, 200.f);
		if(active == -1 && shared.active == -1)
		{
			model->SetPosition(ci.position);
			storm.scene->AddModel(model.get());
			rotateModel(model, rotation);

			if(mouse.hasLeftClicked())
			{
				VC2 position(ci.position.x, ci.position.z);
				int index = shared.lights.addSpot(position, shared.getProperties());
				shared.lights.setRotation(index, rotation);
			}
		}
		else
		{
			if(shared.active >= 0 && wheelDelta)
				shared.lights.setRotation(shared.active, rotation);
			if(shared.active >= 0)
			{
				VC2 pos2 = shared.lights.getPosition(shared.active);
				VC3 pos(pos2.x, 0, pos2.y);
				bool updatePos = false;
				pos = shared.storm.unitAligner.getMovedPosition(pos, *shared.storm.scene->GetCamera(), &updatePos);

				if(updatePos)
					shared.lights.moveSpot(shared.active, VC2(pos.x, pos.z));
			}

			if(mouse.hasLeftClicked())
			{
				if(shared.active == active || active == -1)
					shared.setActive(-1);
				else
				{
					shared.setActive(-1);
					shared.setActive(active);
					rotation = shared.lights.getRotation(active);
				}
			}
		}

		if(shared.active >= 0)
		{
			if(GetKeyState(VK_DELETE) & 0x80)
			{
				int index = shared.active;
				shared.setActive(-1);
				shared.lights.deleteSpot(index);
			}
		}
	}

	void update()
	{
		model = shared.lights.getModel(shared.getProperties(), COL(2.f, 2.f, 2.f));

		if(shared.active >= 0)
			shared.lights.setProperties(shared.active, shared.getProperties());
	}

	void reset()
	{
		model.reset();
	}
};

} // unnamed

struct LightMode::Data
{
	Shared shared;
	PropertyCommand propertyCommand;
	ColorCommand colorCommand;
	ColorGuessCommand colorGuessCommand;
	TextureCommand textureCommand;
	ConeTextureCommand coneTextureCommand;
	EraseConeTextureCommand eraseConeTextureCommand;
	LightModelCommand lightModelCommand;
	EraseLightModelCommand eraseLightModelCommand;
	DisplayCommand displayCommand;
	GroupChangeCommand groupChangeCommand;
	DeleteCommand deleteCommand;

	SyncCommand syncCommand;
	MouseTracker mouseTracker;

	Data(Gui &gui, Storm &storm, IEditorState &state)
	:	shared(gui, storm, state),
		propertyCommand(shared),
		colorCommand(shared),
		colorGuessCommand(shared),
		textureCommand(shared),
		coneTextureCommand(shared),
		eraseConeTextureCommand(shared),
		lightModelCommand(shared),
		eraseLightModelCommand(shared),
		displayCommand(shared),
		groupChangeCommand(shared),
		deleteCommand(shared),
		syncCommand(shared),
		mouseTracker(shared, gui.getMouse())
	{
		shared.spotPropertyObserver = &mouseTracker;
	}

	~Data()
	{
		shared.spotPropertyObserver = 0;
	}
};

LightMode::LightMode(Gui &gui, Storm &storm, IEditorState &editorState)
{
	scoped_ptr<Data> tempData(new Data(gui, storm, editorState));
	data.swap(tempData);
}

LightMode::~LightMode()
{
}

void LightMode::tick()
{
	data->mouseTracker.tick();
}

void LightMode::nudgeLights(const VC3 &position, const VC3 &direction, float radius)
{
	data->shared.lights.nudgeLights(position, direction, radius);
}

void LightMode::update()
{
	data->mouseTracker.update();
	data->shared.lights.update();
}

void LightMode::reset()
{
	data->mouseTracker.reset();
	data->shared.lights.reset();
}

void LightMode::hideObjects()
{
	data->shared.lights.setProperty(TerrainLights::RenderVisualization, false);
	data->shared.lights.removeLights();
}

void LightMode::showObjects()
{
	data->displayCommand.execute(0);
	data->propertyCommand.execute(0);
}

void LightMode::getLights(std::vector<TerrainLightMap::PointLight> &lights, bool onlyBuilding)
{
	data->shared.lights.getLights(lights, onlyBuilding);
}

void LightMode::doExport(Exporter &exporter) const
{
	data->shared.lights.doExport(exporter);
}

filesystem::OutputStream &LightMode::writeStream(filesystem::OutputStream &stream) const
{
	stream << int(0);

	stream << data->shared.lights;
	return stream;
}

filesystem::InputStream &LightMode::readStream(filesystem::InputStream &stream)
{
	int version = 0;
	stream >> version;

	stream >> data->shared.lights;
	return stream;
}

} // editor
} // frozenbyte
