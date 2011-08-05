
#include "precompiled.h"

#include "ErrorWindow.h"

#include <assert.h>

#include "uidefaults.h"
#include "../util/Debug_MemoryManager.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_debug.h"

#define ERRORWINDOW_MSG_TEXTS 16

#define ERRORWINDOW_WIDTH 1008
#define ERRORWINDOW_HEIGHT 272
#define ERRORWINDOW_MINIHEIGHT 32

namespace ui
{

  ErrorWindow::ErrorWindow(Ogui *ogui)
  {
    this->ogui = ogui;

    // window
#ifdef LEGACY_FILES
    win = ogui->CreateSimpleWindow(8, 8, ERRORWINDOW_WIDTH, ERRORWINDOW_HEIGHT, "Data/GUI/Windows/errorwindow.dds");
#else
    win = ogui->CreateSimpleWindow(8, 8, ERRORWINDOW_WIDTH, ERRORWINDOW_HEIGHT, "data/gui/common/window/errorwindow.tga");
#endif
    win->Hide();

		win->SetEffectListener(this);

    // close button
    closeBut = ogui->CreateSimpleImageButton(win, 1008-24, 8, 16, 16, NULL, NULL, NULL);
    closeBut->SetStyle(defaultCloseButton);
    closeBut->SetListener(this);

    messageText = new OguiTextLabel *[ERRORWINDOW_MSG_TEXTS];
    for (int i = 0; i < ERRORWINDOW_MSG_TEXTS; i++)
    {
      messageText[i] = NULL;
    }

    inputQuery = ogui->CreateTextLabel(win, 0,
			8 + 16 * ERRORWINDOW_MSG_TEXTS, 16, 16, "");
    inputQuery->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
    if (defaultSmallIngameFont != NULL)
      inputQuery->SetFont(defaultSmallIngameFont);

    inputText = ogui->CreateTextLabel(win, 8,
			8 + 16 * ERRORWINDOW_MSG_TEXTS, 1000, 16, "");
    inputText->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
    if (defaultThinFont != NULL)
      inputText->SetFont(defaultThinFont);

    messages = 0;
		miniWindowMode = false;
		isClosing = false;
  }

  ErrorWindow::~ErrorWindow()
  {
		delete inputQuery;
		delete inputText;
    for (int i = 0; i < ERRORWINDOW_MSG_TEXTS; i++)
    {
      if (messageText[i] != NULL)
        delete messageText[i];
    }
    delete [] messageText;
    if (closeBut != NULL) delete closeBut;
    if (win != NULL) delete win;
  }

	void ErrorWindow::clearMessages()
	{
    for (int i = 0; i < ERRORWINDOW_MSG_TEXTS; i++)
    {
      if (messageText[i] != NULL)
			{
				delete messageText[i];
				messageText[i] = NULL;
			}
    }
	}

	void ErrorWindow::setMiniWindowMode(bool miniWindowMode)
	{
		this->miniWindowMode = miniWindowMode;
		if (miniWindowMode)
		{
			win->Resize(ERRORWINDOW_WIDTH, ERRORWINDOW_MINIHEIGHT);
			inputQuery->Move(0, 8);
			inputText->Move(8, 8);
		} else {
			win->Resize(ERRORWINDOW_WIDTH, ERRORWINDOW_HEIGHT);
			inputQuery->Move(0, 8 + 16 * ERRORWINDOW_MSG_TEXTS);
			inputText->Move(8, 8 + 16 * ERRORWINDOW_MSG_TEXTS);
		}
	}

	bool insideErrorWindow = false;

  void ErrorWindow::logMessage(const char *msg, int level)
  {
		// prevent any error messages being caused by showing error messages (infinite loop)
		if (insideErrorWindow)
		{
			//assert(!"ErrorWindow::logMessage - error window caused an error to be logged (omitted, as that would cause an infinite loop).");
			return;
		}
		insideErrorWindow = true;

		// check that we don't have newlines in the message.
		// if so, cut the messages to multiple lines
		bool multiline = false;
		char *multibuf = NULL;
		int slen = strlen(msg);
		int prevpos = 0;
		for (int pos = 0; pos < slen; pos++)
		{
			if (msg[pos] == '\n')
			{
				if (!multiline)
				{
					multiline = true;
					multibuf = new char[slen + 1];
					strcpy(multibuf, msg);
				}
				multibuf[pos] = '\0';

				if (pos > prevpos)
				{
					if (multibuf[pos - 1] == '\r')
					{
						multibuf[pos - 1] = '\0';
					}
					insideErrorWindow = false;
					logMessage(&multibuf[prevpos], level);
					insideErrorWindow = true;
					prevpos = pos + 1;
				}
			}
		}

		if (multiline)
		{
			if (prevpos < slen)
			{
				insideErrorWindow = false;
				logMessage(&multibuf[prevpos], level);
				insideErrorWindow = true;
			}

			delete [] multibuf;
			insideErrorWindow = false;
		  return;
		}

		if (level <= game::SimpleOptions::getInt(DH_OPT_I_RAISE_CONSOLE_LOGLEVEL))
		{
			win->Raise();
			win->Show();
		}

    assert(messageText[messages % ERRORWINDOW_MSG_TEXTS] == NULL);

    // change current message to this msg
    messageText[messages % ERRORWINDOW_MSG_TEXTS] = 
      ogui->CreateTextLabel(win, 0, 8 + 16 * (messages % ERRORWINDOW_MSG_TEXTS), 1008, 16, (char *)msg);
    messageText[messages % ERRORWINDOW_MSG_TEXTS]->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
    if (defaultThinFont != NULL)
      messageText[messages % ERRORWINDOW_MSG_TEXTS]->SetFont(defaultThinFont);

    messages++;

    // delete next message
    if (messageText[messages % ERRORWINDOW_MSG_TEXTS] != NULL)
    {
      delete messageText[messages % ERRORWINDOW_MSG_TEXTS];
      messageText[messages % ERRORWINDOW_MSG_TEXTS] = NULL;
    }

		// set message positions so that the lowest one is the next 
		// message...
		for (int i = 0; i < ERRORWINDOW_MSG_TEXTS; i++)
		{
			if (messageText[i] != NULL)
			{
				messageText[i]->Move(0, 8 + 16 * (ERRORWINDOW_MSG_TEXTS - 1 - ((messages - i) % ERRORWINDOW_MSG_TEXTS)));
			}
		}

		insideErrorWindow = false;
  }

  void ErrorWindow::CursorEvent(OguiButtonEvent *eve)
  {
    if (eve->triggerButton == closeBut)
    {
      //hide();
			hideWithEffect();
    }
  }

  void ErrorWindow::EffectEvent(OguiEffectEvent *eve)
  {
		win->EndAllEffects();
		isClosing = false;
    hide();
  }

  void ErrorWindow::hideWithEffect()
  {
		win->StartEffect(OGUI_WINDOW_EFFECT_MOVEOUT, 200);
		win->StartEffect(OGUI_WINDOW_EFFECT_FADEOUT, 200);
		isClosing = true;
  }

  void ErrorWindow::hide()
  {
		isClosing = false;
    win->Hide();
  }

  void ErrorWindow::show()
  {
		isClosing = false;
		win->EndAllEffects();
    win->Show();
  }

  bool ErrorWindow::isVisible()
  {
    return (win->IsVisible() && !isClosing);
  }

  void ErrorWindow::raise()
  {
    win->Raise();
  }

  void ErrorWindow::setInputLine(const char *inputLine, const char *inputLineRight)
  {
		if (inputLine == NULL)
		{
			inputQuery->SetText("");
			inputText->SetText("");
		} else {
			inputQuery->SetText(">");

			char tmp[256 + 4 + 1];
			int slen = strlen(inputLine);
			strncpy(tmp, inputLine, 256);
			if (slen < 256)
			{
				tmp[slen] = '|';
				tmp[slen + 1] = '\0';
			}
			if (inputLineRight != NULL)
			{
				if (slen + strlen(inputLineRight) < 256)
				{
					strcat(tmp, inputLineRight);
				}
			}

			inputText->SetText(tmp);
		}
  }

}

