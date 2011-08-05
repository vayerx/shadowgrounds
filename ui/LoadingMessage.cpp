
#include "precompiled.h"

#include <set>
using namespace std;

#include "LoadingMessage.h"

#include <istorm3D_terrain_renderer.h>
#include <Storm3D_UI.h>
#include "uidefaults.h"
#include "../ogui/Ogui.h"
#include "../convert/str2int.h"
#include "../game/DHLocaleManager.h"
#include "../storm/storm3dv2/Iterator.h"

namespace ui
{
	IStorm3D *LoadingMessage::storm3d = NULL;
	IStorm3D_Scene *LoadingMessage::scene = NULL;
	Ogui *LoadingMessage::ogui = NULL;

	static int loading_bar_value = -1;
	static std::string loading_message;
	static std::string loading_bar_text;

	void LoadingMessage::setManagers(IStorm3D *s3d,
		IStorm3D_Scene *scene, Ogui *ogui)
	{
		LoadingMessage::storm3d = s3d;
		LoadingMessage::scene = scene;
		LoadingMessage::ogui = ogui;
	}

	void LoadingMessage::renderLoadingMessage()
	{
		assert(storm3d != NULL);
#ifdef LEGACY_FILES
		OguiWindow *win = ogui->CreateSimpleWindow(0,0,1024,768,"Data/GUI/Windows/loadingmsg.tga");
#else
		OguiWindow *win = ogui->CreateSimpleWindow(0,0,1024,768,"data/gui/common/window/loadingmsg.tga");
#endif

#ifdef PROJECT_SURVIVOR
		if(loading_message.empty())
			win->setBackgroundImage(NULL);
#endif
		OguiTextLabel *txt_msg = ogui->CreateTextLabel(win, 0, 768/2, 1024, 32, "");

		txt_msg->SetTextHAlign(OguiButton::TEXT_H_ALIGN_CENTER);
		txt_msg->SetTextVAlign(OguiButton::TEXT_V_ALIGN_MIDDLE);
		if (ui::defaultBigIngameFont == NULL)
			txt_msg->SetFont(ui::defaultFont);
		else
			txt_msg->SetFont(ui::defaultBigIngameFont);
		txt_msg->SetText(loading_message.c_str());

		OguiButton *bar_bg = NULL;
		OguiTextLabel *bar_num = NULL;
		OguiTextLabel *bar_txt = NULL;
		OguiButton *bar = NULL;

		if(loading_bar_value != -1)
		{

			int x = game::getLocaleGuiInt("gui_loadingbar_x",0);
			int y = game::getLocaleGuiInt("gui_loadingbar_y",0);
			int w = game::getLocaleGuiInt("gui_loadingbar_w",0);
			int h = game::getLocaleGuiInt("gui_loadingbar_h",0);
			bar_bg = ogui->CreateSimpleImageButton(win, x, y, w, h, game::getLocaleGuiString("gui_loadingbar_background"), NULL, NULL, 0);
			
			bar = ogui->CreateSimpleImageButton(win, x, y, loading_bar_value * w / 100, h, game::getLocaleGuiString("gui_loadingbar_foreground"), NULL, NULL, 0);

			bar_num = ogui->CreateTextLabel(win, x, y, w, h, "");
			std::string text = int2str(loading_bar_value) + std::string("%");
			bar_num->SetFont(ui::defaultIngameFont);
			bar_num->SetTextHAlign(OguiButton::TEXT_H_ALIGN_CENTER);
			bar_num->SetTextVAlign(OguiButton::TEXT_V_ALIGN_MIDDLE);
			bar_num->SetText(text.c_str());

			bar_txt = ogui->CreateTextLabel(win, x, y, w, h, "");
			bar_txt->SetFont(ui::defaultIngameFont);
			bar_txt->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
			bar_txt->SetTextVAlign(OguiButton::TEXT_V_ALIGN_MIDDLE);
			bar_txt->SetText(loading_bar_text.c_str());			
		}

		win->Raise();

		// HACK!
		std::vector<bool> oldValues;
		if(scene->ITTerrain)
		{
			IteratorIM_Set<IStorm3D_Terrain*> it = *(IteratorIM_Set<IStorm3D_Terrain*>*)scene->ITTerrain->Begin();
			while(!it.IsEnd())
			{
				IStorm3D_Terrain *terrain = it.GetCurrent();
				oldValues.push_back(terrain->getRenderer().enableFeature(IStorm3D_TerrainRenderer::RenderTargets, false ));
				it.Next();
			}
		}
		
		ogui->Run(1);
		scene->RenderScene();

		if(scene->ITTerrain)
		{
			unsigned int i = 0;
			IteratorIM_Set<IStorm3D_Terrain*> it = *(IteratorIM_Set<IStorm3D_Terrain*>*)scene->ITTerrain->Begin();
			while(!it.IsEnd() && i < oldValues.size())
			{
				IStorm3D_Terrain *terrain = it.GetCurrent();
				terrain->getRenderer().enableFeature(IStorm3D_TerrainRenderer::RenderTargets, oldValues[i] );
				it.Next();
				i++;
			}
		}


		delete bar;
		delete bar_txt;
		delete bar_num;
		delete bar_bg;
		delete txt_msg;
		delete win;
	}

	void LoadingMessage::showLoadingMessage(const char *loadingMessage)
	{
		loading_message = loadingMessage;
		renderLoadingMessage();
	}

	void LoadingMessage::clearLoadingMessage()
	{
		loading_message.clear();
	}

	void LoadingMessage::showLoadingBar(int value)
	{
		loading_bar_value = value;
		renderLoadingMessage();
	}

	void LoadingMessage::setLoadingBarText(const char *text)
	{
		loading_bar_text = text;
	}
}

