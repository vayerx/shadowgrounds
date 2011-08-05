
#include "precompiled.h"

#include "CombatMessageWindow.h"
#include "GUIEffectWindow.h"
#include "../filesystem/file_package_manager.h"
#include "../filesystem/input_stream.h"

#include <string>
#include <assert.h>
#include "Visual2D.h"
#include "uidefaults.h"
#include "../game/DHLocaleManager.h"
#include "../util/fb_assert.h"
#include "../system/Logger.h"
#include "../convert/str2int.h"

using namespace game;

namespace ui
{

	CombatMessageWindow::CombatMessageWindow(Ogui *ogui, game::Game *game, int player,
		const char *textConfName, const char *iconConfName)
	:	effectWindow(0),
		time(0),
		noiseAlpha(0),
		noiseTime(0)
	{
		this->ogui = ogui;
		this->game = game;
		this->player = player;

		fb_assert(textConfName != NULL);
		fb_assert(iconConfName != NULL);

		this->textConfName = new char[strlen(textConfName) + 1];
		strcpy(this->textConfName, textConfName);
		this->iconConfName = new char[strlen(iconConfName) + 1];
		strcpy(this->iconConfName, iconConfName);
		
		std::string xpos_conf("");
		std::string ypos_conf("");
		std::string xsize_conf("");
		std::string ysize_conf("");

		std::string icon_xpos_conf("");
		std::string icon_ypos_conf("");
		std::string icon_xsize_conf("");
		std::string icon_ysize_conf("");

		std::string iconbox_xpos_conf("");
		std::string iconbox_ypos_conf("");
		std::string iconbox_xsize_conf("");
		std::string iconbox_ysize_conf("");

		icon_xpos_conf = std::string("gui_message_") + this->iconConfName + std::string("_position_x");
		icon_ypos_conf = std::string("gui_message_") + this->iconConfName + std::string("_position_y");
		icon_xsize_conf = std::string("gui_message_") + this->iconConfName + std::string("_size_x");
		icon_ysize_conf = std::string("gui_message_") + this->iconConfName + std::string("_size_y");

		iconbox_xpos_conf = std::string("gui_message_") + this->iconConfName + std::string("_box_position_x");
		iconbox_ypos_conf = std::string("gui_message_") + this->iconConfName + std::string("_box_position_y");
		iconbox_xsize_conf = std::string("gui_message_") + this->iconConfName + std::string("_box_size_x");
		iconbox_ysize_conf = std::string("gui_message_") + this->iconConfName + std::string("_box_size_y");

		xpos_conf = std::string("gui_message_") + this->textConfName + std::string("_textarea_position_x");
		ypos_conf = std::string("gui_message_") + this->textConfName + std::string("_textarea_position_y");
		xsize_conf = std::string("gui_message_") + this->textConfName + std::string("_textarea_size_x");
		ysize_conf = std::string("gui_message_") + this->textConfName + std::string("_textarea_size_y");

		this->textXPosition = getLocaleGuiInt(xpos_conf.c_str(), 0);
		this->textYPosition = getLocaleGuiInt(ypos_conf.c_str(), 0);
		this->textXSize = getLocaleGuiInt(xsize_conf.c_str(), 0);
		this->textYSize = getLocaleGuiInt(ysize_conf.c_str(), 0);

		this->boxXPosition = 0;
		this->boxYPosition = 0;
		this->boxXSize = 0;
		this->boxYSize = 0;

		if (iconConfName[0] != '\0')
		{
			this->iconXPosition = getLocaleGuiInt(icon_xpos_conf.c_str(), 0);
			this->iconYPosition = getLocaleGuiInt(icon_ypos_conf.c_str(), 0);
			this->iconXSize = getLocaleGuiInt(icon_xsize_conf.c_str(), 0);
			this->iconYSize = getLocaleGuiInt(icon_ysize_conf.c_str(), 0);

			this->iconBoxXPosition = getLocaleGuiInt(iconbox_xpos_conf.c_str(), 0);
			this->iconBoxYPosition = getLocaleGuiInt(iconbox_ypos_conf.c_str(), 0);
			this->iconBoxXSize = getLocaleGuiInt(iconbox_xsize_conf.c_str(), 0);
			this->iconBoxYSize = getLocaleGuiInt(iconbox_ysize_conf.c_str(), 0);
		} else {
			this->iconXPosition = 0;
			this->iconYPosition = 0;
			this->iconXSize = 0;
			this->iconYSize = 0;

			this->iconBoxXPosition = 0;
			this->iconBoxYPosition = 0;
			this->iconBoxXSize = 0;
			this->iconBoxYSize = 0;
		}

		this->win = ogui->CreateSimpleWindow(0, 0, 1024, 768, NULL);
		this->win->SetReactMask(0); // does this work???????????
		this->win->SetEffectListener( this );
		this->textFont = ui::defaultIngameFont;
		this->textCentered = false;
		this->textBoxed = false;

		this->boxImage = NULL;

#ifdef LEGACY_FILES
		this->iconBGImage = ogui->LoadOguiImage("Data/GUI/Buttons/face1_bg.tga");
#else
		this->iconBGImage = ogui->LoadOguiImage("data/gui/hud/message/face1_bg.tga");
#endif

		messageText = NULL;
		//messageImage = NULL;
		messageImageButton = NULL;
		messageBoxButton = NULL;
		messageImageBGButton = NULL;

		const char *l1Image = getLocaleGuiString("gui_message_window_effect_layer1_image");
		//const char *l2Image = getLocaleGuiString("gui_message_window_effect_layer2_image");
		const char *l3Image = getLocaleGuiString("gui_message_window_effect_layer3_image");
		effectWindow = new GUIEffectWindow(ogui, l1Image, 0, l3Image, VC2I(iconXPosition,iconYPosition), VC2I(iconXSize, iconYSize));
		effectWindow->hide();

		{
#ifdef LEGACY_FILES
			frozenbyte::filesystem::InputStream stream = frozenbyte::filesystem::FilePackageManager::getInstance().getFile("Data/Effects/conversation_noise.txt");
#else
			frozenbyte::filesystem::InputStream stream = frozenbyte::filesystem::FilePackageManager::getInstance().getFile("data/effect/conversation_noise.txt");
#endif
			
			for(int i = 0; !stream.isEof(); ++i)
			{
				std::string result;
				while(!stream.isEof())
				{
					signed char c = 0;
					stream >> c;

					if(c == '\r')
						continue;
					else if(c == '\n')
						break;

					result += c;
				}

				if(i == 0)
					noiseTime = atoi(result.c_str());
				else
				{
					float value = float(atof(result.c_str()));
					noiseArray.push_back(value);
				}
			}
		}
	}

	CombatMessageWindow::~CombatMessageWindow()
	{
		if (iconConfName != NULL)
		{
			delete [] iconConfName;
		}
		if (textConfName != NULL)
		{
			delete [] textConfName;
		}
		if (iconBGImage != NULL)
		{
			delete iconBGImage;
		}
		setBoxed(false);
		clearMessage();
	}


	/*
	void CombatMessageWindow::moveTo(int x, int y)
	{
		xPosition = x;
		yPosition = y;
	}
	*/

	
	void CombatMessageWindow::hide(int fadeTime)
	{
		if(fadeTime)
			win->StartEffect(OGUI_WINDOW_EFFECT_FADEOUT, fadeTime);
		else
			win->Hide();
	}


	void CombatMessageWindow::show(int fadeTime)
	{
		if(fadeTime)
			win->StartEffect(OGUI_WINDOW_EFFECT_FADEIN, fadeTime);

		win->Show();
	}

	void CombatMessageWindow::EffectEvent( OguiEffectEvent *e )
	{
		if(e->eventType == OguiEffectEvent::EVENT_TYPE_FADEDOUT)
			win->Hide();
	}


	void CombatMessageWindow::setFont(IOguiFont *font, bool centered)
	{
		textFont = font;
		textCentered = centered;
	}


	void CombatMessageWindow::setBoxed(bool textBoxed)
	{
		/*
		fb_assert(this->textConfName != NULL);

		if (this->textConfName[0] == '\0' && textBoxed)
		{
			fb_assert(!"CombatMessageWindow::setBoxed - Attempt to set message window boxed when no conf name given.");
		}
		*/

		if (textBoxed)
		{
			std::string box_xpos_conf("");
			std::string box_ypos_conf("");
			std::string box_xsize_conf("");
			std::string box_ysize_conf("");

			box_xpos_conf = std::string("gui_message_") + this->textConfName + std::string("_box_position_x");
			box_ypos_conf = std::string("gui_message_") + this->textConfName + std::string("_box_position_y");
			box_xsize_conf = std::string("gui_message_") + this->textConfName + std::string("_box_size_x");
			box_ysize_conf = std::string("gui_message_") + this->textConfName + std::string("_box_size_y");

			this->boxXPosition = getLocaleGuiInt(box_xpos_conf.c_str(), 0);
			this->boxYPosition = getLocaleGuiInt(box_ypos_conf.c_str(), 0);
			this->boxXSize = getLocaleGuiInt(box_xsize_conf.c_str(), 0);
			this->boxYSize = getLocaleGuiInt(box_ysize_conf.c_str(), 0);
		}

		this->textBoxed = textBoxed;
		if (this->boxImage != NULL && !textBoxed)
		{
			delete this->boxImage;
			this->boxImage = NULL;
		} 
		if (this->boxImage == NULL && textBoxed)
		{
#ifdef LEGACY_FILES
			this->boxImage = ogui->LoadOguiImage("Data/GUI/Buttons/textbox.tga");
#else
			this->boxImage = ogui->LoadOguiImage("data/gui/hud/message/textbox.tga");
#endif
		} 
	}


	void CombatMessageWindow::showMessage(const char *message, Visual2D *image)
	{
		// TODO: OPTIMIZE IMAGE HANDLING!!!
		// i think it's already ok, as the image is given as Visual2D, not 
		// as the image filename... (?)

		if (messageText != NULL)
			clearMessage();

		if (message == NULL) 
		{
			assert(0);
			return;
		}

		// add linebreaks (convert backslashes to linebreaks)
		int messageLen = strlen(message);
		char *brMessage = new char[messageLen + 1];
		for (int i = 0; i < messageLen + 1; i++)
		{
			if (message[i] == '\\')
			{
				brMessage[i] = '\n';
			} else {
				brMessage[i] = message[i];
			}
		}

		//int xPosition = getLocaleGuiInt("gui_health_position_x", 0);
		//int yPosition = getLocaleGuiInt("gui_health_position_y", 0);

		if (image != NULL)
		{
			messageImageBGButton = ogui->CreateSimpleImageButton(win, 
				iconBoxXPosition, iconBoxYPosition, iconBoxXSize, iconBoxYSize, NULL, NULL, NULL);
			messageImageBGButton->SetDisabled(true);
			messageImageBGButton->SetDisabledImage(iconBGImage);

			//messageImage = ogui->LoadOguiImage(image);
			IOguiImage *messageImage = image->getImage();

			messageImageButton = ogui->CreateSimpleImageButton(win, 
				iconXPosition, iconYPosition, iconXSize, iconYSize, NULL, NULL, NULL);
			messageImageButton->SetDisabled(true);
			messageImageButton->SetDisabledImage(messageImage);

			if(effectWindow)
			{
				effectWindow->raise();
				effectWindow->show();
				//effectWindow->setTransparency(100, );
			}
		}

		if (textBoxed)
		{
			messageBoxButton = ogui->CreateSimpleImageButton(win, 
				boxXPosition, boxYPosition, boxXSize, boxYSize, NULL, NULL, NULL);
			messageBoxButton->SetDisabled(true);
			messageBoxButton->SetDisabledImage(boxImage);
		}

		messageText = ogui->CreateTextLabel(win, textXPosition, textYPosition, textXSize, textYSize, "");

		if (!textCentered)
		{
			messageText->SetLinebreaks(true); 
		}
		if (textCentered)
			messageText->SetTextHAlign(OguiButton::TEXT_H_ALIGN_CENTER);
		else
			messageText->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
		messageText->SetTextVAlign(OguiButton::TEXT_V_ALIGN_TOP);
		messageText->SetText(brMessage);
		messageText->SetFont(textFont);

		delete [] brMessage;
	}

	void CombatMessageWindow::clearMessageTextOnly()
	{
		if (messageText != NULL)
		{
			messageText->SetText("");
		}
	}

	void CombatMessageWindow::setNoiseAlpha(float alpha)
	{
		noiseAlpha = alpha;
	}

	void CombatMessageWindow::clearMessage()
	{
		/*
		if (messageImage != NULL)
		{
			delete messageImage;
			messageImage = NULL;
		}
		*/
		if (messageBoxButton != NULL)
		{
			delete messageBoxButton;
			messageBoxButton = NULL;
		}
		if (messageImageBGButton != NULL)
		{
			delete messageImageBGButton;
			messageImageBGButton = NULL;
		}
		if (messageImageButton != NULL)
		{
			delete messageImageButton;
			messageImageButton = NULL;
		}
		if (messageText != NULL)
		{
			delete messageText;
			messageText = NULL;
		}

		if(effectWindow)
			effectWindow->hide();

		//time = 0;
	}

	void CombatMessageWindow::raise()
	{
		win->Raise();

		if(effectWindow)
			effectWindow->raise();
	}

	void CombatMessageWindow::update(int ms)
	{
		if(effectWindow)
		{
			effectWindow->update(ms * 10);
			time += ms;

			float alpha = noiseAlpha + (sinf(time * 0.0015f) * 0.10f * noiseAlpha);
			{
				int index = (time % noiseTime) * noiseArray.size() / noiseTime;
				alpha += noiseArray[index] * noiseAlpha * 0.3f * 2.f;
			}

			int transparency = 100 - int(alpha * 100.f);
			if(transparency < 0)
				transparency = 0;
			if(transparency > 100)
				transparency = 100;

			float alpha2 = noiseAlpha + (sinf(time * 0.0011f + 1.f) * 0.10f * noiseAlpha);
			{
				int index = ((time + noiseTime/2) % noiseTime) * noiseArray.size() / noiseTime;
				alpha2 += noiseArray[index] * noiseAlpha * 0.3f * 2.f;
			}

			int transparency2 = 100 - int(alpha2 * 100.f);
			if(transparency2 < 0)
				transparency2 = 0;
			if(transparency2 > 100)
				transparency2 = 100;

			effectWindow->setTransparency(transparency, transparency2);
		}
	}
}
