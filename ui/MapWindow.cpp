
#include "precompiled.h"

#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <boost/shared_ptr.hpp>

#include <istorm3D_terrain_renderer.h>
#include <IStorm3D_Terrain.h>

#include "MapWindow.h"
#include "Map.h"
#include "../util/assert.h"
#include "../ogui/Ogui.h"
#include "../ogui/OguiCheckBox.h"
#include "../ogui/OguiFormattedText.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/DHLocaleManager.h"
#include "../ui/GUIEffectWindow.h"
#include "../ui/uidefaults.h"
#include "../system/Logger.h"
#include "../game/GameScene.h"

#include "../game/options/options_gui.h"

using namespace boost;
using namespace game;

namespace ui {

	static const int TEXTURE_RESOLUTION = 1024;
	static const int MAP_FADE_IN_TIME = 500;
	static const int MAP_FADE_OUT_TIME = 500;

	struct MapEntity
	{
		MapWindow::Entity entity;
		VC2 position;
		float rotation;

		MapEntity()
		:	entity(MapWindow::Player)
		{
		}
	};

	struct ObjectivePoint
	{
		VC2 position;
		float radius;

		std::string layer;
		VC2 targetPosition;

		ObjectivePoint()
		:	radius(0)
		{
		}
	};

	struct Objective
	{
		std::string id;
		bool completed;

		explicit Objective(const std::string &id_)
		:	id(id_),
			completed(false)
		{
		}
	};

	struct Portal
	{
		VC2 position;
		std::string toLayer;
		VC2 toPosition;
	};

	typedef std::vector<Portal> PortalList;
	typedef std::map<std::string, PortalList> Portals;


typedef std::vector<MapEntity> EntityList;
typedef std::map<std::string, ObjectivePoint> ObjectivePoints;
typedef std::vector<Objective> ObjectiveList;
typedef std::vector<shared_ptr<OguiTextLabel> > LabelList;
typedef std::vector<shared_ptr<OguiButton> > ButtonList;

typedef std::vector<shared_ptr<OguiCheckBox> > CheckboxList;

struct MapWindow::Data : private IOguiButtonListener, private IOguiEffectListener
{
	game::Game &game;
	Ogui &ogui;

	EntityList entities;
	std::string layerId;

	VC2 playerPosition;
	float playerRotation;
	int currentObjective;
	
	shared_ptr<Map> map;

	bool visible;
	scoped_ptr<GUIEffectWindow> effectWindow;
	scoped_ptr<OguiWindow> window;
	scoped_ptr<OguiButton> closeButton;
	scoped_ptr<OguiButton> mapBackground;
	scoped_ptr<OguiButton> mapOverlay;
	scoped_ptr<OguiButton> mapButton;
	scoped_ptr<IOguiImage> mapImage;
	// scoped_ptr<OguiTextLabel> missionLabel;
	// scoped_ptr<OguiTextLabel> primaryLabel;
	// scoped_ptr<OguiTextLabel> secondaryLabel;
	scoped_ptr<OguiTextLabel> exitLabel;
	scoped_ptr<OguiButton> playerButton;
	scoped_ptr<OguiButton> checkpointButton;
	scoped_ptr<OguiButton> primaryBackground;
	// scoped_ptr<OguiButton> secondaryBackground;
	scoped_ptr<OguiButton> highlight;
	// scoped_ptr<OguiFormattedText> description;	// by Pete
	scoped_ptr<IOguiFont> boldFont;				// by Pete
	scoped_ptr<IOguiFont> bigObjectiveFont;		// by Pete

#ifdef PROJECT_SURVIVOR
	scoped_ptr<IOguiFont> normalFont;
	scoped_ptr<IOguiFont> exitFont;
	scoped_ptr<IOguiFont> exitFontHigh;

	scoped_ptr<IOguiFont> missionTitleFont;
	scoped_ptr<OguiTextLabel> missionTitleLabel;

	scoped_ptr<IOguiFont> scrollTipFont;
	scoped_ptr<OguiTextLabel> scrollTipLabel;
#endif

	// scoped_ptr<OguiButton> haxButton;

	ObjectivePoints objectivePoints;
 	ObjectiveList objectives[2];

	VC2I objectiveLabelPosition[2];
	VC2I objectiveTextPosition[2];
	int objectiveTextOffset[2];

	VC2I mapPosition;
	VC2I mapSize;
	VC2I mapLayerPosition;
	VC2I mapLayerSize;

	VC2I playerSize;
	VC2I checkpointSize;

	LabelList objectiveLabels[2];
	ButtonList objectiveButtons[2];
	CheckboxList	objectiveCheckboxs[2];
	std::string missionId;

	Portals portals;

	float mapScrollSpeedFactor;
	VC2 mapZoomSpeed;
	VC2 mapZoomCenter;
	float mapZoomFactor;
	VC2 mapPositionOffset;
	int mapZoomingStarted;
	float mapAspectRatio;

	VC2I mapBackgroundPosition;

	std::vector<DWORD> mapBuffer;
	std::vector<DWORD> mapBufferOutput;

	Data(game::Game &game_, Ogui &ogui_, shared_ptr<Map> &map_)
	:	game(game_),
		ogui(ogui_),
		playerRotation(0),
		currentObjective(0),
		map(map_),
		visible(false),
		mapScrollSpeedFactor(0.0f),
		mapZoomSpeed(0.0f, 0.0f),
		mapZoomCenter(0.0f, 0.0f),
		mapZoomFactor(1.0f),
		mapPositionOffset(0, 0),
		mapZoomingStarted(Timer::getTime()),
		mapAspectRatio( 1.0f )
	{
		FB_ASSERT(map);
		objectiveTextOffset[0] = 0;
		objectiveTextOffset[1] = 0;

		const std::string &dir = map->getMission();
		std::string::size_type end = dir.find_last_of("/");
		if(end == dir.npos)
			return;
		std::string::size_type start = dir.find_last_of("/", end - 2);
		if(start == dir.npos)
			return;

		missionId = dir.substr(start + 1, end - start - 1);

		for (int i = 0; i < (int)missionId.length(); i++)
		{
			missionId[i] = tolower(missionId[i]);
		}
	}

	~Data()
	{
	}

	void init()
	{
		objectiveLabelPosition[0].x = getLocaleGuiInt("gui_map_primary_objects_label_position_x", 0);
		objectiveLabelPosition[0].y = getLocaleGuiInt("gui_map_primary_objects_label_position_y", 0);
		objectiveTextPosition[0].x = getLocaleGuiInt("gui_map_primary_objects_text_position_x", 0);
		objectiveTextPosition[0].y = getLocaleGuiInt("gui_map_primary_objects_text_position_y", 0);
		objectiveTextOffset[0] = getLocaleGuiInt("gui_map_primary_objects_text_offset_y", 0);
		objectiveLabelPosition[1].x = getLocaleGuiInt("gui_map_secondary_objects_label_position_x", 0);
		objectiveLabelPosition[1].y = getLocaleGuiInt("gui_map_secondary_objects_label_position_y", 0);
		objectiveTextPosition[1].x = getLocaleGuiInt("gui_map_secondary_objects_text_position_x", 0);
		objectiveTextPosition[1].y = getLocaleGuiInt("gui_map_secondary_objects_text_position_y", 0);
		objectiveTextOffset[1] = getLocaleGuiInt("gui_map_secondary_objects_text_offset_y", 0);

		// Create gui

		window.reset(ogui.CreateSimpleWindow(0, 0, 1024, 768, 0));
		window->SetEffectListener(this);
		window->Hide();

#ifndef PROJECT_SURVIVOR
		closeButton.reset(ogui.CreateSimpleTextButton(window.get(), getLocaleGuiInt("gui_map_exit_position_x", 0), getLocaleGuiInt("gui_map_exit_position_y", 0), 
			getLocaleGuiInt("gui_map_exit_size_x", 0), getLocaleGuiInt("gui_map_exit_size_y", 0), 
			getLocaleGuiString("gui_map_exit_image"), getLocaleGuiString("gui_map_exit_image_down"), 
			getLocaleGuiString("gui_map_exit_image_highlight"), ""));
		closeButton->SetListener(this);
#endif
		{
			mapBackgroundPosition.x = getLocaleGuiInt("gui_map_map_background_position_x", 0);
			mapBackgroundPosition.y = getLocaleGuiInt("gui_map_map_background_position_y", 0);
			int xs = getLocaleGuiInt("gui_map_map_background_size_x", 0);
			int ys = getLocaleGuiInt("gui_map_map_background_size_y", 0);
			const char *fname = getLocaleGuiString("gui_map_map_background_image");
			mapBackground.reset(ogui.CreateSimpleTextButton(window.get(), mapBackgroundPosition.x, mapBackgroundPosition.y, xs, ys, fname, fname, fname, 0));
		}

#ifdef PROJECT_SURVIVOR
		std::string filename;
		std::stringstream filenamestream;
		filenamestream << "Data/Missions/" << game.getMissionId() << "/bin/map_default.png";
		filenamestream >> filename;
		mapImage.reset(ogui.LoadOguiImage(filename.c_str()));

		if(mapImage.get() && mapImage->getTexture())
		{
			IStorm3D_Texture *t = mapImage->getTexture();
			int width = t->getWidth();
			int height = t->getHeight();

			mapAspectRatio = width/(float)height;

			// init map data
			mapBuffer.resize(width * height);
			mapBufferOutput.resize(width * height);
			t->CopyTextureTo32BitSysMembuffer(&mapBuffer[0]);
			for(int y = 0; y < height; y++)
			{
				for(int x = 0; x < width; x++)
				{
					DWORD col = mapBuffer[y * width + x];
					mapBuffer[y * width + x] = col & 0x00FFFFFF;
				}
			}
		}
#else
		mapImage.reset(ogui.LoadOguiImage(TEXTURE_RESOLUTION, TEXTURE_RESOLUTION));
#endif
		mapPosition.x = getLocaleGuiInt("gui_map_map_position_x", 0);
		mapPosition.y = getLocaleGuiInt("gui_map_map_position_y", 0);
		mapSize.x = getLocaleGuiInt("gui_map_map_size_x", 0);
		mapSize.y = getLocaleGuiInt("gui_map_map_size_y", 0);
		mapLayerPosition = mapPosition;
		mapLayerSize = mapSize;

		mapButton.reset(ogui.CreateSimpleTextButton(window.get(), mapPosition.x, mapPosition.y, mapSize.x, mapSize.y, 0, 0, 0, 0));

		mapButton->SetImage(mapImage.get());
		mapButton->SetDownImage(mapImage.get());
		mapButton->SetDisabledImage(mapImage.get());
		mapButton->SetHighlightedImage(mapImage.get());
		mapButton->SetEventMask( OGUI_EMASK_CLICK | OGUI_EMASK_PRESS | OGUI_EMASK_HOLD);
		mapButton->SetListener(this);

#ifdef PROJECT_SURVIVOR
		updateMapButtonVertices();
#endif

#ifndef PROJECT_SURVIVOR
		{
			int x = getLocaleGuiInt("gui_map_primary_background_position_x", 0);
			int y = getLocaleGuiInt("gui_map_primary_background_position_y", 0);
			int xs = getLocaleGuiInt("gui_map_primary_background_size_x", 0);
			int ys = getLocaleGuiInt("gui_map_primary_background_size_y", 0);
			const char *fname = getLocaleGuiString("gui_map_primary_background_image");
			primaryBackground.reset(ogui.CreateSimpleTextButton(window.get(), x, y, xs, ys, fname, fname, fname, 0));
		}
#endif
		{
			/*
			int x = getLocaleGuiInt("gui_map_secondary_background_position_x", 0);
			int y = getLocaleGuiInt("gui_map_secondary_background_position_y", 0);
			int xs = getLocaleGuiInt("gui_map_secondary_background_size_x", 0);
			int ys = getLocaleGuiInt("gui_map_secondary_background_size_y", 0);
			const char *fname = getLocaleGuiString("gui_map_secondary_background_image");
			secondaryBackground.reset(ogui.CreateSimpleTextButton(window.get(), x, y, xs, ys, fname, fname, fname, 0));
			*/
		}
		{
			/*int x = getLocaleGuiInt("gui_map_mission_object_label_position_x", 0);
			int y = getLocaleGuiInt("gui_map_mission_object_label_position_y", 0);
			int xs = getLocaleGuiInt("gui_map_mission_object_label_size_x", 0);
			int ys = getLocaleGuiInt("gui_map_mission_object_label_size_y", 0);
			const char *fname = getLocaleGuiString("gui_map_mission_object_label_image");
			haxButton.reset(ogui.CreateSimpleTextButton(window.get(), x, y, xs, ys, fname, fname, fname, 0));
			*/
		}
#ifndef PROJECT_SURVIVOR
		{
			int x = getLocaleGuiInt("gui_map_exit_text_position_x", 0);
			int y = getLocaleGuiInt("gui_map_exit_text_position_y", 0);
			int xs = getLocaleGuiInt("gui_map_exit_text_size_x", 0);
			int ys = getLocaleGuiInt("gui_map_exit_text_size_y", 0);
			exitLabel.reset(ogui.CreateTextLabel(window.get(), x, y, xs, ys, ""));
			exitLabel->SetTextHAlign(OguiButton::TEXT_H_ALIGN_CENTER);
			exitLabel->SetTextVAlign(OguiButton::TEXT_V_ALIGN_MIDDLE);
			exitLabel->SetText(getLocaleGuiString("gui_map_exit"));
			exitLabel->SetFont(ui::defaultIngameFont);
		}
#endif

		// fonts
		{
			boldFont.reset( ogui.LoadFont( getLocaleGuiString( "gui_map_font_bold" ) ) );
			bigObjectiveFont.reset( ogui.LoadFont( getLocaleGuiString( "gui_map_font_big_objective" ) ) );
#ifdef PROJECT_SURVIVOR
			normalFont.reset( ogui.LoadFont( getLocaleGuiString( "gui_map_font_normal") ) );
			exitFont.reset( ogui.LoadFont( getLocaleGuiString( "gui_map_exit_text_font_normal" ) ) );
			exitFontHigh.reset( ogui.LoadFont( getLocaleGuiString( "gui_map_exit_text_font_high" ) ) );
#endif
		}
		
		/*
		missionLabel.reset(ogui.CreateTextLabel(window.get(), getLocaleGuiInt("gui_map_mission_object_label_position_x", 0), getLocaleGuiInt("gui_map_mission_object_label_position_y", 0), 200, 50, ""));
		missionLabel->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
		missionLabel->SetTextVAlign(OguiButton::TEXT_V_ALIGN_TOP);
		missionLabel->SetText(getLocaleGuiString( ("gui_" + missionId + "_mission_label").c_str() ));
		missionLabel->SetFont( bigObjectiveFont.get() );
		primaryLabel.reset(ogui.CreateTextLabel( window.get(), objectiveLabelPosition[0].x, objectiveLabelPosition[0].y, 100, 50, ""));
		primaryLabel->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
		primaryLabel->SetTextVAlign(OguiButton::TEXT_V_ALIGN_TOP);
		primaryLabel->SetText(getLocaleGuiString("gui_map_primary_objectives"));
		primaryLabel->SetFont( ui::defaultIngameFont );
		*/
		/*
		secondaryLabel.reset(ogui.CreateTextLabel(window.get(), objectiveLabelPosition[1].x, objectiveLabelPosition[1].y, 100, 50, ""));
		secondaryLabel->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
		secondaryLabel->SetTextVAlign(OguiButton::TEXT_V_ALIGN_TOP);
		secondaryLabel->SetFont( ui::defaultIngameFont );
		secondaryLabel->SetText(getLocaleGuiString("gui_map_secondary_objectives"));
		*/
		
		/*
		// the description box initialization
		description.reset( new OguiFormattedText( window.get(), &ogui, getLocaleGuiInt( "gui_map_description_box_position_x", 0 ), getLocaleGuiInt( "gui_map_description_box_position_y", 0 ), getLocaleGuiInt( "gui_map_description_box_size_x", 0 ), getLocaleGuiInt( "gui_map_description_box_size_y", 0 )  ) );
		description->setFont( ui::defaultIngameFont );
		description->registerFont( "b", boldFont.get() );
		description->registerFont( "h1", bigObjectiveFont.get() );
		*/

		playerSize.x = getLocaleGuiInt("gui_map_player_arrow_size_x", 0);
		playerSize.y = getLocaleGuiInt("gui_map_player_arrow_size_y", 0);
		playerButton.reset(ogui.CreateSimpleTextButton(window.get(), 0, 0, playerSize.x, playerSize.y, 
			getLocaleGuiString("gui_map_player_arrow_image"), getLocaleGuiString("gui_map_player_arrow_image"), 
			getLocaleGuiString("gui_map_player_arrow_image_highlight"), 0, 0, 0, false));
		playerButton->SetListener(this);

		checkpointSize.x = getLocaleGuiInt("gui_map_checkpoint_size_x", 0);
		checkpointSize.y = getLocaleGuiInt("gui_map_checkpoint_size_y", 0);
		checkpointButton.reset(ogui.CreateSimpleTextButton(window.get(), 0, 0, checkpointSize.x, checkpointSize.y, 
			getLocaleGuiString("gui_map_checkpoint_image"), getLocaleGuiString("gui_map_checkpoint_image"), 
			getLocaleGuiString("gui_map_checkpoint_image_highlight"), 0, 0, 0, false));


#ifdef PROJECT_SURVIVOR
		{
			int x = getLocaleGuiInt("gui_map_map_overlay_position_x", 0);
			int y = getLocaleGuiInt("gui_map_map_overlay_position_y", 0);
			int xs = getLocaleGuiInt("gui_map_map_overlay_size_x", 0);
			int ys = getLocaleGuiInt("gui_map_map_overlay_size_y", 0);
			const char *fname = getLocaleGuiString("gui_map_map_overlay_image");
			mapOverlay.reset(ogui.CreateSimpleImageButton(window.get(), x, y, xs, ys, fname, fname, fname, fname, 0, 0, true));
			mapOverlay->SetDisabled(true);
		}
		{
			int x = getLocaleGuiInt("gui_map_primary_background_position_x", 0);
			int y = getLocaleGuiInt("gui_map_primary_background_position_y", 0);
			int xs = getLocaleGuiInt("gui_map_primary_background_size_x", 0);
			int ys = getLocaleGuiInt("gui_map_primary_background_size_y", 0);
			const char *fname = getLocaleGuiString("gui_map_primary_background_image");
			primaryBackground.reset(ogui.CreateSimpleImageButton(window.get(), x, y, xs, ys, fname, fname, fname, fname, 0, 0, true));
			primaryBackground->SetDisabled(true);
		}
		closeButton.reset(ogui.CreateSimpleTextButton(window.get(), getLocaleGuiInt("gui_map_exit_position_x", 0), getLocaleGuiInt("gui_map_exit_position_y", 0), 
			getLocaleGuiInt("gui_map_exit_size_x", 0), getLocaleGuiInt("gui_map_exit_size_y", 0), 
			getLocaleGuiString("gui_map_exit_image"), getLocaleGuiString("gui_map_exit_image_down"), 
			getLocaleGuiString("gui_map_exit_image_highlight"), ""));
		closeButton->SetEventMask( OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE );
		closeButton->SetListener(this);
		{
			int x = getLocaleGuiInt("gui_map_exit_text_position_x", 0);
			int y = getLocaleGuiInt("gui_map_exit_text_position_y", 0);
			int xs = getLocaleGuiInt("gui_map_exit_text_size_x", 0);
			int ys = getLocaleGuiInt("gui_map_exit_text_size_y", 0);
			exitLabel.reset(ogui.CreateTextLabel(window.get(), x, y, xs, ys, ""));
			exitLabel->SetTextHAlign(OguiButton::TEXT_H_ALIGN_CENTER);
			exitLabel->SetTextVAlign(OguiButton::TEXT_V_ALIGN_MIDDLE);
			exitLabel->SetText(getLocaleGuiString("gui_map_exit"));
			exitLabel->SetFont(exitFont.get());
		}
		{
			missionTitleFont.reset( ogui.LoadFont( getLocaleGuiString( "gui_map_missiontitle_font" ) ) );

			int x = getLocaleGuiInt("gui_map_missiontitle_position_x", 0);
			int y = getLocaleGuiInt("gui_map_missiontitle_position_y", 0);
			int xs = getLocaleGuiInt("gui_map_missiontitle_size_x", 0);
			int ys = getLocaleGuiInt("gui_map_missiontitle_size_y", 0);

			const char *missiontitle = getLocaleGuiString((std::string(game.getMissionId()) + "_header_text").c_str());
			missionTitleLabel.reset( ogui.CreateTextLabel(window.get(), x, y, xs, ys, missiontitle) );
			missionTitleLabel->SetFont(missionTitleFont.get());
			missionTitleLabel->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
		}
		{
			scrollTipFont.reset( ogui.LoadFont( getLocaleGuiString( "gui_map_scrolltip_font" ) ) );

			int x = getLocaleGuiInt("gui_map_scrolltip_position_x", 0);
			int y = getLocaleGuiInt("gui_map_scrolltip_position_y", 0);
			int xs = getLocaleGuiInt("gui_map_scrolltip_size_x", 0);
			int ys = getLocaleGuiInt("gui_map_scrolltip_size_y", 0);

			const char *scrolltip = getLocaleGuiString("gui_map_scrolltip");
			scrollTipLabel.reset( ogui.CreateTextLabel(window.get(), x, y, xs, ys, scrolltip) );
			scrollTipLabel->SetFont(scrollTipFont.get());
			scrollTipLabel->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
		}
#endif
	}

	void updateMapButtonVertices()
	{
		if(mapPosition.x >= mapBackgroundPosition.x + 32 || mapPosition.y >= mapBackgroundPosition.y + 32)
		{
			return;
		}

		int corner_x = mapBackgroundPosition.x + 32 - mapPosition.x;
		int corner_y = mapBackgroundPosition.y + 32 - mapPosition.y;
		int size_x = mapSize.x;
		int size_y = mapSize.y;

		OguiButton::Vertex vertices[15];
		// top corner triangle
		//   0
		// 2 1
		vertices[0].x = corner_x;
		vertices[0].y = 0;
		vertices[0].color = 0xFFFFFFFF;

		vertices[1].x = corner_x;
		vertices[1].y = corner_y;
		vertices[1].color = 0xFFFFFFFF;

		vertices[2].x = 0;
		vertices[2].y = corner_y;
		vertices[2].color = 0xFFFFFFFF;

		// main quad
		// 0
		// 2 1
		vertices[3].x = 0;
		vertices[3].y = corner_y;
		vertices[3].color = 0xFFFFFFFF;

		vertices[4].x = size_x;
		vertices[4].y = size_y;
		vertices[4].color = 0xFFFFFFFF;

		vertices[5].x = 0;
		vertices[5].y = size_y;
		vertices[5].color = 0xFFFFFFFF;

		//   0
		// 2 1
		vertices[6].x = size_x;
		vertices[6].y = corner_y;
		vertices[6].color = 0xFFFFFFFF;

		vertices[7].x = size_x;
		vertices[7].y = size_y;
		vertices[7].color = 0xFFFFFFFF;

		vertices[8].x = 0;
		vertices[8].y = corner_y;
		vertices[8].color = 0xFFFFFFFF;

		// top quad
		// 0
		// 2 1
		vertices[9].x = corner_x;
		vertices[9].y = 0;
		vertices[9].color = 0xFFFFFFFF;

		vertices[10].x = size_x;
		vertices[10].y = corner_y;
		vertices[10].color = 0xFFFFFFFF;

		vertices[11].x = corner_x;
		vertices[11].y = corner_y;
		vertices[11].color = 0xFFFFFFFF;

		//   0
		// 2 1
		vertices[12].x = size_x;
		vertices[12].y = 0;
		vertices[12].color = 0xFFFFFFFF;

		vertices[13].x = size_x;
		vertices[13].y = corner_y;
		vertices[13].color = 0xFFFFFFFF;

		vertices[14].x = corner_x;
		vertices[14].y = 0;
		vertices[14].color = 0xFFFFFFFF;

		mapButton->SetCustomShape(vertices, 15);
	}


  void getObjectCoordinates(const VC2 &position, float rotation, VC2 &pos, float &rot)
	{
#ifdef PROJECT_SURVIVOR
		int size_x = game.getGameScene()->getGameMap()->getSizeX();
		int size_y = game.getGameScene()->getGameMap()->getSizeY();
		pos.y = 0.5f - position.x / (float)size_x;
		pos.x = 0.5f - position.y / (float)size_y;
		rot = rotation + PI/2;
#else
		map->getPlayerCoordinates(position, rotation, pos, rot);
#endif
	}

	void setActiveLayer(const std::string &id)
	{
		if(layerId != id)
		{
			update(0);
			layerId = id;

			map->loadLayer(layerId);
			updateObjectivePoints();

			VC2 p1;
			VC2 p2;
			map->getLayerCoordinates(p1, p2);

			{
				VC2 d = p2 - p1;
				float ratio = d.x / d.y;

				/*
				if(ratio > 1.f)
				{
					int x1 = 0;
					int x2 = mapSize.x;
					int y2 = int(mapSize.y / ratio);
					int y1 = (mapSize.y - y2) / 2;
					y2 += y1;

					mapLayerPosition = VC2I(x1, y1) + mapPosition;
					mapLayerSize = VC2I(x2-x1, y2-y1);
				}
				else
				{
					int y1 = 0;
					int y2 = mapSize.y;
					int x2 = int(mapSize.x * ratio);
					int x1 = (mapSize.x - x2) / 2;
					x2 += x1;

					mapLayerPosition = VC2I(x1, y1) + mapPosition;
					mapLayerSize = VC2I(x2-x1, y2-y1);
				}
				*/

				if(ratio < 1.f)
				{
					int x1 = 0;
					int x2 = mapSize.x;
					int y2 = int(mapSize.y * ratio);
					int y1 = (mapSize.y - y2) / 2;
					y2 += y1;

					mapLayerPosition = VC2I(x1, y1) + mapPosition;
					mapLayerSize = VC2I(x2-x1, y2-y1);
				}
				else
				{
					int y1 = 0;
					int y2 = mapSize.y;
					int x2 = int(mapSize.x / ratio);
					int x1 = (mapSize.x - x2) / 2;
					x2 += x1;

					mapLayerPosition = VC2I(x1, y1) + mapPosition;
					mapLayerSize = VC2I(x2-x1, y2-y1);
				}
			}
		}
	}

	void update(int ms)
	{
#ifdef PROJECT_SURVIVOR
		if(mapImage.get() && mapImage->getTexture() && !mapBuffer.empty() && !mapBufferOutput.empty())
		{
			static int last_update = game.gameTimer;

			// update 1000/30 = 33 fps
			if(game.gameTimer - last_update > GAME_TICKS_PER_SECOND / 30)
			{
				last_update = game.gameTimer;

				IStorm3D_Texture *t = mapImage->getTexture();
				int width = t->getWidth();
				int height = t->getHeight();

				VC2 pos;
				float rot = 0;
				getObjectCoordinates(playerPosition, playerRotation, pos, rot);

				VC2 pos_x, pos_y;
				getObjectCoordinates(playerPosition + VC2(0, 15.0f), playerRotation, pos_x, rot);
				getObjectCoordinates(playerPosition + VC2(15.0f, 0), playerRotation, pos_y, rot);

				int center_x = (int)(pos.x * width);
				int center_y = (int)(pos.y * height);
				
				int radius_x = (int)((pos_x - pos).GetLength() * width);
				int radius_y = (int)((pos_y - pos).GetLength() * height);

				int start_x = center_x - radius_x;
				int start_y = center_y - radius_y;

				int end_x = center_x + radius_x;
				int end_y = center_y + radius_y;

				if(start_x < 0) start_x = 0;
				else if(start_x > width) start_x = width;
				if(start_y < 0) start_y = 0;
				else if(start_y > height) start_y = height;

				if(end_x < 0) end_x = 0;
				else if(end_x > width) end_x = width;
				if(end_y < 0) end_y = 0;
				else if(end_y > height) end_y = height;

				int rad_sqr_x = radius_x * radius_x / 2;
				int rad_sqr_y = radius_y * radius_y / 2;

				DWORD *buffer = &mapBuffer[start_y * width];

				unsigned int size_y = end_y - start_y;
				unsigned int size_x = end_x - start_x;

				unsigned int relative_center_x = size_x / 2;
				unsigned int relative_center_y = size_y / 2;

				unsigned int remaining_pixels_in_row = width - start_x;

				unsigned int y = 0;
				do
				{
					// compute new fog value (based on distance^2 from center)
					// (Y AXIS)
					int dist_y = (y - relative_center_y);
					int fog_y = (128 * (dist_y * dist_y)) / (rad_sqr_y);

					buffer += start_x;

					unsigned int x = 0;
					do
					{
						// get existing fog value
						DWORD col = buffer[x];
						int fog = (col & 0xFF000000) >> 24;

						// compute new fog value (based on distance^2 from center)
						// (X AXIS)
						int dist_x = (x - relative_center_x);
						int new_fog = (128 * (dist_x * dist_x)) / (rad_sqr_x) + fog_y;
						new_fog = 255 - new_fog;

						// set new fog if it's less
						if(new_fog > fog) {
							fog = new_fog;

						// mask out existing fog
						col &= 0x00FFFFFF;
						// apply new fog
						col |= (fog & 0xFF) << 24;
						// write to buffer
						buffer[x] = col;
						}
						x++;
					}
					while(x < size_x);

					buffer += remaining_pixels_in_row;
					y++;
				}
				while(y < size_y);
			}
		}
#else
		map->update(playerPosition, ms);
#endif

#ifdef PROJECT_SURVIVOR
		if(visible && mapButton.get())
		{
			// scroll map
			//
			{
				/*int cursor_x = ogui.getCursorScreenX(0) - mapPosition.x;
				int cursor_y = ogui.getCursorScreenY(0) - mapPosition.y;*/

				// unstable frame rate, interpolate for smoothness
				mapScrollSpeedFactor = ms * 0.125f + mapScrollSpeedFactor * 0.875f;
				float speed_factor = mapScrollSpeedFactor * 0.001f / mapZoomFactor;

				// border touch-logic
				/*
				int edge_size = 100;
				// left edge
				if(cursor_x > 0 && cursor_x < edge_size)
					mapZoomSpeed.x = -speed_factor * (edge_size - cursor_x) / (float)edge_size;
				// right edge
				else if(cursor_x > mapSize.x - edge_size && cursor_x < mapSize.x)
					mapZoomSpeed.x = speed_factor * (cursor_x - (mapSize.x - edge_size)) / (float)edge_size;

				// top edge
				if(cursor_y > 0 && cursor_y < edge_size)
					mapZoomSpeed.y = -speed_factor * (edge_size - cursor_y) / (float)edge_size;
				// bottom edge
				else if(cursor_y > mapSize.y - edge_size && cursor_y < mapSize.y)
					mapZoomSpeed.y = speed_factor * (cursor_y - (mapSize.y - edge_size)) / (float)edge_size;
				*/

				mapZoomCenter += mapZoomSpeed * speed_factor;

				// slow down speed
				float slowdown_factor = 0.008f * mapScrollSpeedFactor;
				if(slowdown_factor > 1.0f)
					slowdown_factor = 1.0f;
				mapZoomSpeed *= (1.0f - slowdown_factor);
				if(fabsf(mapZoomSpeed.x) < 0.0001f)
					mapZoomSpeed.x = 0.0f;
				if(fabsf(mapZoomSpeed.y) < 0.0001f)
					mapZoomSpeed.y = 0.0f;

				// clamp values
				if(mapZoomCenter.x < 0.0f)
					mapZoomCenter.x = 0.0f;
				else if(mapZoomCenter.x > 1.0f)
					mapZoomCenter.x = 1.0f;
				if(mapZoomCenter.y < 0.0f)
					mapZoomCenter.y = 0.0f;
				else if(mapZoomCenter.y > 1.0f)
					mapZoomCenter.y = 1.0f;

			}

			// update map texture coordinates
			//
			int time = Timer::getTime() - mapZoomingStarted;
			float lerp = time / 4000.0f;
			if(lerp > 1.0f)
				lerp = 1.0f;
			float autoZoom = SimpleOptions::getFloat(DH_OPT_F_GUI_MAP_AUTOZOOM);
			mapZoomFactor = mapZoomFactor * (1.0f - lerp) + autoZoom * (lerp);

			float center_x = mapZoomCenter.x;
			float center_y = mapZoomCenter.y;
			mapButton->SetRepeat(1.0f / mapZoomFactor, mapAspectRatio / mapZoomFactor);
			mapButton->SetScroll(center_x - center_x / mapZoomFactor, center_y - center_y * mapAspectRatio / mapZoomFactor);
			mapButton->SetWrap(false);

			// offset + mapZoomFactor * center = center
			mapPositionOffset.x = center_x - center_x * mapZoomFactor;
			mapPositionOffset.y = center_y - center_y * mapZoomFactor / mapAspectRatio;

			updateButtons();
		}
#endif
	}

	void setObjectivePoint(const std::string &id, const VC3 &position, float radius)
	{
		FB_ASSERT(!id.empty());

		ObjectivePoint &p = objectivePoints[id];
		p.position = VC2(position.x, position.z);
		p.targetPosition = p.position;
		p.radius = radius;

		updateObjectivePoints();
	}

	void setObjectivePointLayer(const std::string &id, const std::string &layer)
	{
		FB_ASSERT(!id.empty());
		ObjectivePoint &p = objectivePoints[id];
		p.layer = layer;

		updateObjectivePoints();
	}

	void addObjective(ObjectiveType type, const std::string &id)
	{
		FB_ASSERT(!id.empty());
		objectives[type].push_back(Objective(id));

		//if(currentObjective < 0 || currentObjective == int(objectives[0].size()) - 2)
		{
			updateCurrentObjective();
			updateObjectivePoints();
		}
	}

	void addActiveObjective(const std::string &id)
	{
		FB_ASSERT(!id.empty());
		ObjectiveList &list = objectives[0];

		list.resize(list.size() + 1, Objective(""));
		for(int i = list.size() - 1; i > currentObjective; --i)
			std::swap(list[i], list[i - 1]);

		list[currentObjective] = Objective(id);

		updateCurrentObjective();
		updateObjectivePoints();
	}

	void completeObjective(const std::string &id)
	{
		int i = 0;
		int j = 0;
		if(!getIndices(i, j, id))
			return;

		Objective &o = objectives[i][j];
		o.completed = true;

		updateCurrentObjective();
		updateObjectivePoints();
	}

	void removeObjective(const std::string &id)
	{
		int i = 0;
		int j = 0;
		if(!getIndices(i, j, id))
			return;

		ObjectiveList &list = objectives[i];
		list.erase(list.begin() + j);

		updateCurrentObjective();
		updateObjectivePoints();
	}

	void updateCurrentObjective()
	{
		currentObjective = 0;

		ObjectiveList &list = objectives[0];
		for(unsigned int i = 0; i < list.size(); ++i)
		{
			if(!list[i].completed)
			{
				currentObjective = i;
				break;
			}
		}
		updateObjectiveDescription();
	}

	void updateObjectivePoints()
	{
/*
		for(int i = 0; i < 2; ++i)
		{
			ObjectiveList &list = objectives[i];
			ObjectiveList::iterator it = list.begin();
			
			for(; it != list.end(); ++it)
			{
				ObjectivePoints::iterator op = objectivePoints.find(it->id);
				if(op == objectivePoints.end())
					continue;

				it->hasPoint = true;
				it->point = op->second;
			}
		}
*/
		findTargetPositions();
	}

	void updateObjectiveDescription()
	{				
		// const std::string& id = objectives[ currentObjective ].id;
		/*std::string text = "gui_";
		text += missionId;
		text += "_objective_description_";
		text += objectives[0][ currentObjective ].id;*/

		// description->setText( getLocaleGuiString( text.c_str() ) );

	}

	struct RouteData
	{
		PortalList portals;
		float cost;

		RouteData()
		:	cost(0)
		{
		}
	};

	struct Route
	{
		std::string startLayer;
		VC2 startPosition;
		std::string endLayer;
		VC2 endPosition;

		const Portals &portals;
		typedef std::vector<RouteData> RouteList;
		RouteList closedRoutes;

		VC2 result;

		Route(const Portals &portals_)
		:	portals(portals_)
		{
		}

		void beginSearch()
		{
			result = endPosition;
			if(startLayer.empty() || endLayer.empty())
				return;

			if(startLayer != endLayer)
			{
				RouteData startRoute;
				iterate(startRoute);

				// Find the lowest cost route and set result to first portal from position

				int index = -1;
				for(RouteList::size_type i = 0; i < closedRoutes.size(); ++i)
				{
					if(index == -1 || closedRoutes[i].cost < closedRoutes[index].cost)
						index = i;
				}

				if(index >= 0)
				{
					RouteData &r = closedRoutes[index];
					if(!r.portals.empty())
						result = r.portals[0].position;
				}
				else
				{
					Logger::getInstance()->error("MapWindow -- Search portal route failed to find path");
					{
						std::string msg = startLayer;
						msg += ", ";
						msg += endLayer;
						Logger::getInstance()->error(msg.c_str());
					}
				}
			}
		}

		void iterate(const RouteData &current)
		{
			std::string currentLayer;
			VC2 currentPosition;

			if(!current.portals.empty())
			{
				const Portal &p = current.portals[current.portals.size() - 1];
				currentLayer = p.toLayer;
				currentPosition = p.toPosition;
			}
			else
			{
				currentLayer = startLayer;
				currentPosition = startPosition;
			}

			// Do we have any portals left to test?
			Portals::const_iterator portalIterator = portals.find(currentLayer);
			if(portalIterator == portals.end())
				return;

			const PortalList &portalList = portalIterator->second;
			for(PortalList::const_iterator it = portalList.begin(); it != portalList.end(); ++it)
			{
				const Portal &p = *it;
				if(p.toLayer == startLayer)
					continue;

				// Have we already visited this layer?
				bool visited = false;
				for(PortalList::size_type i = 0; i < current.portals.size(); ++i)
				{
					if(current.portals[i].toLayer == p.toLayer)
					{
						visited = true;
						break;
					}
				}

				if(visited)
					continue;

				// Recurse deeper or finish if found given layer
				RouteData newRoute = current;
				newRoute.cost += currentPosition.GetSquareRangeTo(p.position);
				newRoute.portals.push_back(p);

				if(p.toLayer == endLayer)
				{
					newRoute.cost += endPosition.GetSquareRangeTo(p.toPosition);
					closedRoutes.push_back(newRoute);
				}
				else
				{
					VC2 fromPosition = startPosition;
					if(!current.portals.empty())
						fromPosition = current.portals[current.portals.size() - 1].toPosition;
					
					newRoute.cost += fromPosition.GetSquareRangeTo(p.toPosition);
					iterate(newRoute);
				}
			}
		}
	};

	void findTargetPositions()
	{
		if(currentObjective < 0 || objectives[0].empty())
			return;

		ObjectiveList &list = objectives[0];
		Objective &objective = list[currentObjective];

		//if(!objective.hasPoint)
		//	return;

		{
			ObjectivePoints::iterator op = objectivePoints.find(objective.id);
			if(op == objectivePoints.end())
				return;

			Route route(portals);
			route.startLayer = layerId;
			if(route.startLayer.empty())
				route.startLayer = "default";
			route.startPosition = playerPosition;
			route.endLayer = op->second.layer;
			route.endPosition = op->second.position;
			route.beginSearch();

			op->second.targetPosition = route.result;
		}
	}

	void updateMissionObjectiveToPosition(const std::string &id, const VC3 &position)
	{
		FB_ASSERT(!id.empty());

		ObjectivePoint &p = objectivePoints[id];
		p.position = VC2(position.x, position.z);
		p.targetPosition = p.position;

		updateObjectivePoints();
	}

	bool getIndices(int &first, int &second, const std::string &id) const
	{
		FB_ASSERT(!id.empty());

		for(int i = 0; i < 2; ++i)
		{
			const ObjectiveList &list = objectives[i];
			ObjectiveList::const_iterator it = list.begin();

			for(; it != list.end(); ++it)
			{
				if(it->id == id)
				{
					first = i;
					second = it - list.begin();
					return true;
				}
			}
		}

		return false;
	}

	bool getCurrentObjectivePosition(VC3 &position) const
	{
		if(currentObjective < int(objectives[0].size()))
		{
			const Objective &o = objectives[0][currentObjective];
			ObjectivePoints::const_iterator it = objectivePoints.find(o.id);
			if(it == objectivePoints.end())
				return false;

			position.x = it->second.targetPosition.x;
			position.z = it->second.targetPosition.y;
			return true;
		}

		return false;
	}

	bool doesMissionObjectiveExist(const std::string &id) const
	{
		int i = 0;
		int j = 0;
		if(!getIndices(i, j, id))
			return false;

		return true;
	}

	bool isMissionObjectiveComplete(const std::string &id) const
	{
		int i = 0;
		int j = 0;
		if(!getIndices(i, j, id))
			return false;

		const Objective &o = objectives[i][j];
		return o.completed;
	}

	// UI ..

	bool createObjectives()
	{
		for(int i = 0; i < 2; ++i)
		{
			int yOff = objectiveTextOffset[i];
			int offsetBetween = getLocaleGuiInt( "gui_map_space_between_objects", 0 );

			int yLimit = 0;
			if(i == 0)
				yLimit = getLocaleGuiInt("gui_map_primary_background_position_y", 0) + getLocaleGuiInt("gui_map_primary_background_size_y", 0);
			else
				yLimit = getLocaleGuiInt("gui_map_secondary_background_position_y", 0) + getLocaleGuiInt("gui_map_secondary_background_size_y", 0);
			yLimit -= 10;

			ObjectiveList &list = objectives[i];
			ObjectiveList::iterator start = list.begin();

			objectiveLabels[i].clear();
			objectiveButtons[i].clear();
			objectiveCheckboxs[i].clear();
			highlight.reset(NULL);

			// WTF?????
#ifdef PROJECT_SHADOWGROUNDS
			for(; start != list.end(); ++start)
#endif
			{
				int xPos = objectiveTextPosition[i].x;
				int yPos = objectiveTextPosition[i].y;

				ObjectiveList::iterator it = start;
				for(; it != list.end(); ++it)
				{
					int highsizeY = 0;
					int highsizeX = 0;

					// with multiplelines of description the highlight bugs out
					if(i == 0 && it - list.begin() == currentObjective)
					{
						int x = xPos + getLocaleGuiInt("gui_map_highlight_offset_x", 0);
						int y = yPos + getLocaleGuiInt("gui_map_highlight_offset_y", 0);
						int xs = getLocaleGuiInt("gui_map_highlight_size_x", 0);
						int ys = getLocaleGuiInt("gui_map_highlight_size_y", 0);
						highsizeY = ys;
						highsizeX = xs;
						const char *fname = getLocaleGuiString("gui_map_highlight_image");
						highlight.reset(ogui.CreateSimpleTextButton(window.get(), x, y, xs, ys, fname, fname, fname, 0));
					}
					
					shared_ptr<OguiCheckBox> checkbox( new OguiCheckBox( window.get(), &ogui, getLocaleGuiInt( "gui_map_checkbox_position_x", 0 ), yPos, getLocaleGuiInt( "gui_map_checkbox_size_x", 0 ), getLocaleGuiInt( "gui_map_checkbox_size_y", 0 ), 
						getLocaleGuiString("gui_map_checkbox_image"), "", "", 
						getLocaleGuiString("gui_map_checkbox_done_image"), "", "", 0, it->completed, false )  );

					std::string text;

					IOguiFont *font = ui::defaultIngameFont;


#ifdef PROJECT_SURVIVOR
					font = normalFont.get();
					text = "gui_objective_" + missionId + "_" + it->id;
#else
					text = "gui_";
					text += missionId;
					text += "_objective_";
					text += it->id;
#endif
					checkbox->setText( getLocaleGuiString(text.c_str()), OguiCheckBox::TEXT_ALIGN_LEFT, getLocaleGuiInt( "gui_map_primary_objects_label_size_x", 0 ), font );
					objectiveCheckboxs[i].push_back( checkbox );
					int yAdd = (yOff > checkbox->getHeight()) ? yOff : checkbox->getHeight(); 

					yPos += yAdd + offsetBetween;
					if(yAdd > highsizeY && (i == 0 && it - list.begin() == currentObjective) && highlight)
						highlight->Resize(highsizeX, yAdd + (highsizeY - yOff));
#ifdef PROJECT_SHADOWGROUNDS
				}
#endif
				if(yPos < yLimit)
				{
#ifdef PROJECT_SHADOWGROUNDS
					return true;
#else
					break;
				}
#endif
				}
			}
		}

		return false;
	}

	void show()
	{
		if(visible)
			return;

		{
			effectWindow.reset(new GUIEffectWindow(&ogui, 
				getLocaleGuiString("gui_map_window_effect_layer1_image"), 
				getLocaleGuiString("gui_map_window_effect_layer2_image"), 
				getLocaleGuiString("gui_map_window_effect_layer3_image")
				));
		}

		updateObjectivePoints();
		createObjectives();

		effectWindow->raise();
		effectWindow->fadeIn(MAP_FADE_IN_TIME);

		window->Raise();
		window->StartEffect(OGUI_WINDOW_EFFECT_FADEIN, MAP_FADE_IN_TIME);
		window->Show();
		visible = true;

		if(mapImage.get() && mapImage->getTexture())
		{
			IStorm3D_Texture *t = mapImage->getTexture();

#ifndef PROJECT_SURVIVOR
			map->drawTo(*t);
#else
			if(!mapBufferOutput.empty() && !mapBuffer.empty())
			{
				int width = t->getWidth();
				int height = t->getHeight();

				for(int y = 0; y < height; y++)
				{
					for(int x = 0; x < width; x++)
					{
						DWORD col = mapBuffer[y * width + x];
						unsigned char fog = (unsigned char)((col & 0xFF000000) >> 24);
						unsigned char r = (unsigned char)((col & 0xFF0000) >> 16);
						unsigned char g = (unsigned char)((col & 0xFF00) >> 8);
						unsigned char b = (unsigned char)((col & 0xFF) >> 0);
						r = (r * fog) / 255;
						g = (g * fog) / 255;
						b = (b * fog) / 255;
						mapBufferOutput[y * width + x] = (0xFF << 24) | (r << 16) | (g << 8) | (b << 0);
					}
				}
				t->Copy32BitSysMembufferToTexture(&mapBufferOutput[0]);
			}
#endif
		}

		// zoom center is in player coordinates
		VC2 pos;
		float rot = 0;
		getObjectCoordinates(playerPosition, playerRotation, mapZoomCenter, rot);

		updateButtons();

		mapZoomFactor = 1.0f;
		mapZoomingStarted = Timer::getTime();
	}

	void updateButtons()
	{

		// Set player position
		{
			VC2 pos;
			float rot = 0;
			getObjectCoordinates(playerPosition, playerRotation, pos, rot);

			int xOrigin = mapPosition.x - (int)(mapZoomFactor * playerSize.x / 2);
			int yOrigin = mapPosition.y - (int)(mapZoomFactor / mapAspectRatio * playerSize.y / 2);
			//int xOrigin = mapLayerPosition.x - (playerSize.x / 2);
			//int yOrigin = mapLayerPosition.y - (playerSize.y / 2);

			int xp = int((pos.x * mapZoomFactor + mapPositionOffset.x) * mapSize.x) + xOrigin;
			int yp = int((pos.y * mapZoomFactor / mapAspectRatio + mapPositionOffset.y) * mapSize.y) + yOrigin;
			//int xp = int(pos.x * mapLayerSize.x) + xOrigin;
			//int yp = int(pos.y * mapLayerSize.y) + yOrigin;
			playerButton->Move(xp, yp);
			playerButton->SetAngle(rot);

			// don't change player button size due to artistic demands
			//playerButton->Resize((int)(playerSize.x * mapZoomFactor), (int)(playerSize.y * mapZoomFactor));

			float clip_x = 0;
			float clip_y = 0;
			float clip_w = 100;
			float clip_h = 100;
			int x = playerButton->GetX();
			int y = playerButton->GetY();
			int w = playerButton->GetSizeX();
			int h = playerButton->GetSizeY();
			if(x < mapPosition.x)
			{
				clip_x = 100.0f * (mapPosition.x - x) / (float)w;
				if(clip_x > 100.0f) clip_x = 100.0f;
			}
			if(y < mapPosition.y)
			{
				clip_y = 100.0f * (mapPosition.y - y) / (float)h;
				if(clip_y > 100.0f) clip_y = 100.0f;
			}
			if(x + w > mapPosition.x + mapSize.x)
			{
				clip_w = 100.0f * (mapPosition.x + mapSize.x - x) / (float)w;
				if(clip_w < 0.0f)	clip_w = 0.0f;
			}
			if(y + h > mapPosition.y + mapSize.y)
			{
				clip_h = 100.0f * (mapPosition.y + mapSize.y - y) / (float)h;
				if(clip_h < 0.0f)	clip_h = 0.0f;
			}
			playerButton->SetClip(clip_x, clip_y, clip_w, clip_h);

			if(xp == xOrigin && yp == yOrigin)
				playerButton->SetDisabled(true);
			else
				playerButton->SetDisabled(false);
		}

		// Checkpoint position
		if(currentObjective < int(objectives[0].size()))
		{
			VC2 checkpointPosition;
			VC2 checkpointRotation;

			//= objectives[0][currentObjective].point.targetPosition;
			ObjectivePoints::const_iterator it = objectivePoints.find(objectives[0][currentObjective].id);
			if(it != objectivePoints.end())
			{
				checkpointPosition = it->second.targetPosition;
				VC2 pos;
				float rot = 0;
				getObjectCoordinates(checkpointPosition, rot, pos, rot);

				int xOrigin = mapPosition.x - (int)(mapZoomFactor * checkpointSize.x / 2);
				int yOrigin = mapPosition.y - (int)(mapZoomFactor / mapAspectRatio * checkpointSize.y / 2);
				//int xOrigin = mapLayerPosition.x - (checkpointSize.x / 2);
				//int yOrigin = mapLayerPosition.y - (checkpointSize.y / 2);

				int xp = int((pos.x * mapZoomFactor + mapPositionOffset.x) * mapSize.x) + xOrigin;
				int yp = int((pos.y * mapZoomFactor / mapAspectRatio + mapPositionOffset.y) * mapSize.y) + yOrigin;
				//int xp = int(pos.x * mapLayerSize.x) + xOrigin;
				//int yp = int(pos.y * mapLayerSize.y) + yOrigin;
				checkpointButton->Move(xp, yp);
				checkpointButton->Resize((int)(checkpointSize.x * mapZoomFactor), (int)(checkpointSize.y * mapZoomFactor));

				float clip_x = 0;
				float clip_y = 0;
				float clip_w = 100;
				float clip_h = 100;
				int x = checkpointButton->GetX();
				int y = checkpointButton->GetY();
				int w = checkpointButton->GetSizeX();
				int h = checkpointButton->GetSizeY();
				if(x < mapPosition.x)
				{
					clip_x = 100.0f * (mapPosition.x - x) / (float)w;
					if(clip_x > 100.0f) clip_x = 100.0f;
				}
				if(y < mapPosition.y)
				{
					clip_y = 100.0f * (mapPosition.y - y) / (float)h;
					if(clip_y > 100.0f) clip_y = 100.0f;
				}
				if(x + w > mapPosition.x + mapSize.x)
				{
					clip_w = 100.0f * (mapPosition.x + mapSize.x - x) / (float)w;
					if(clip_w < 0.0f)	clip_w = 0.0f;
				}
				if(y + h > mapPosition.y + mapSize.y)
				{
					clip_h = 100.0f * (mapPosition.y + mapSize.y - y) / (float)h;
					if(clip_h < 0.0f)	clip_h = 0.0f;
				}
				checkpointButton->SetClip(clip_x, clip_y, clip_w, clip_h);

				if(xp == xOrigin && yp == yOrigin)
					checkpointButton->SetDisabled(true);
				else
					checkpointButton->SetDisabled(false);
			}
			else
			{
				checkpointButton->SetDisabled(true);
				checkpointButton->Move(10000, 10000);
			}


		}
	}

	void hide()
	{
		if(!visible)
			return;

		window->StartEffect(OGUI_WINDOW_EFFECT_FADEOUT, MAP_FADE_OUT_TIME);
		effectWindow->fadeOut(MAP_FADE_OUT_TIME);

		entities.clear();
		visible = false;
	}

	void CursorEvent(OguiButtonEvent *e)
	{
		if(e->eventType == OguiButtonEvent::EVENT_TYPE_CLICK)
		{
			if(e->triggerButton == closeButton.get())
				game.gameUI->closeMapWindow();
		}
#ifdef PROJECT_SURVIVOR
		else if(e->eventType == OguiButtonEvent::EVENT_TYPE_OVER)
		{
			if(e->triggerButton == closeButton.get())
				exitLabel->SetFont(exitFontHigh.get());
		}
		else if(e->eventType == OguiButtonEvent::EVENT_TYPE_LEAVE)
		{
			if(e->triggerButton == closeButton.get())
				exitLabel->SetFont(exitFont.get());
		}
		else if(e->eventType == OguiButtonEvent::EVENT_TYPE_HOLD)
		{
			if(e->triggerButton == mapButton.get())
			{
				float cursor_x = (e->cursorRelativeX - mapSize.x/2) / (float)mapSize.x;
				float cursor_y = (e->cursorRelativeY - mapSize.y/2) / (float)mapSize.y;
				if(fabs(cursor_x) > 0.25f ||fabs(cursor_y) > 0.25f)
				{
					float sign_x = (cursor_x < 0.0f) ? -1.0f : 1.0f;
					float sign_y = (cursor_y < 0.0f) ? -1.0f : 1.0f;
					mapZoomSpeed.x = 5.0f * (cursor_x * cursor_x * sign_x);
					mapZoomSpeed.y = 5.0f * (cursor_y * cursor_y * sign_y);
				}
			}
		}
#endif
	}

	void EffectEvent(OguiEffectEvent *e)
	{
		if(e->eventType == OguiEffectEvent::EVENT_TYPE_FADEDOUT)
		{
			window->Hide();
			effectWindow.reset();
		}
	}

	void clearMapFog()
	{
		if(mapImage.get() && mapImage->getTexture() && !mapBuffer.empty() && !mapBufferOutput.empty())
		{
			IStorm3D_Texture *t = mapImage->getTexture();
			int width = t->getWidth();
			int height = t->getHeight();
			for(int y = 0; y < height; y++)
			{
				for(int x = 0; x < width; x++)
				{
					DWORD col = mapBuffer[y * width + x];
					mapBuffer[y * width + x] = col | 0xFF000000;
				}
			}
		}
	}
};

MapWindow::MapWindow(game::Game &game, Ogui &ogui, shared_ptr<Map> &map)
{
	scoped_ptr<Data> tempData(new Data(game, ogui, map));
	tempData->init();

	data.swap(tempData);
}

MapWindow::~MapWindow()
{
}

void MapWindow::setEntity(Entity entity, const VC2 &position, float rotation)
{
	if(isVisible())
		return;

	MapEntity e;
	e.entity = entity;
	e.position = position;
	e.rotation = rotation;
	data->entities.push_back(e);

	if(entity == Player)
	{
		data->playerPosition = position;
		data->playerRotation = rotation;
	}
}

void MapWindow::setActiveLayer(const std::string &id)
{
	data->setActiveLayer(id);
}

void MapWindow::update(int ms)
{
	data->update(ms);
}

void MapWindow::show()
{
	data->show();
}

void MapWindow::hide()
{
	data->hide();
}

void MapWindow::setObjectivePoint(const std::string &id, const VC3 &position, float radius)
{
	data->setObjectivePoint(id, position, radius);
}

void MapWindow::setObjectivePointLayer(const std::string &id, const std::string &layer)
{
	data->setObjectivePointLayer(id, layer);
}

void MapWindow::addObjective(ObjectiveType type, const std::string &id)
{
	data->addObjective(type, id);
}

void MapWindow::addActiveObjective(const std::string &id)
{
	data->addActiveObjective(id);
}

void MapWindow::completeObjective(const std::string &id)
{
	data->completeObjective(id);
}

void MapWindow::removeObjective(const std::string &id)
{
	data->removeObjective(id);
}

bool MapWindow::isVisible() const
{
	return data->visible;
}

int MapWindow::getFadeInTime() const
{
	return MAP_FADE_IN_TIME;
}

int MapWindow::getFadeOutTime() const
{
	return MAP_FADE_OUT_TIME;
}

const std::string &MapWindow::getActiveLayer() const
{
	return data->layerId;
}

void MapWindow::raise()
{
	data->window->Raise();
}

void MapWindow::effectUpdate(int delta)
{
	if(data->effectWindow)
		data->effectWindow->update(delta);

	static float angle = 0.f;
	angle += delta / 750.f;

	if(data->checkpointButton)
		data->checkpointButton->SetAngle(angle);
}

void MapWindow::addPortal(const std::string &fromLayer, const std::string &toLayer, const VC3 &fromPosition, const VC3 &toPosition)
{
	Portal portal;
	portal.position.x = fromPosition.x;
	portal.position.y = fromPosition.z;
	portal.toLayer = toLayer;
	portal.toPosition.x = toPosition.x;
	portal.toPosition.y = toPosition.z;

	{
		std::string msg = "MapWindow portal -- ";
		msg += fromLayer;
		msg += ", ";
		msg += toLayer;
		Logger::getInstance()->debug(msg.c_str());
	}

	data->portals[fromLayer].push_back(portal);
	data->findTargetPositions();
}

void MapWindow::removePortal(const std::string &fromLayer, const std::string &toLayer, const VC3 &fromPosition, const VC3 &toPosition)
{
}

void MapWindow::updateMissionObjectiveToPosition(const std::string &id, const VC3 &position)
{
	data->updateMissionObjectiveToPosition(id, position);
}

bool MapWindow::getCurrentObjectivePosition(VC3 &position) const
{
	return data->getCurrentObjectivePosition(position);
}

bool MapWindow::doesMissionObjectiveExist(const std::string &id) const
{
	return data->doesMissionObjectiveExist(id);
}

bool MapWindow::isMissionObjectiveComplete(const std::string &id) const
{
	return data->isMissionObjectiveComplete(id);
}

void MapWindow::clearMapFog()
{
	data->clearMapFog();
}

} // ui
