
#include "precompiled.h"

#include "../container/LinkedList.h"
#include "../system/Logger.h"

#include "MessageBoxWindow.h"

#include "../util/Debug_MemoryManager.h"


namespace ui
{

  MessageBoxWindow::MessageBoxWindow(Ogui *ogui, const char *text, int choiceAmount, 
    const char **choiceTexts, const char ***choiceFiles, int sizeX, int sizeY,
    int choiceSizeX, int choiceSizeY, const char *backgroundFilename,
    int id, IMessageBoxListener *listener)
  {
    this->ogui = ogui;
    this->id = id;
    this->listener = listener;
		this->fadeoutTime = 0;
		this->clickedButtonId = -1;

    buttons = new LinkedList();

    win = ogui->CreateSimpleWindow((1024-sizeX)/2, (768-sizeY)/2, sizeX, sizeY, backgroundFilename);
    win->Raise();

    textArea = ogui->CreateTextArea(win, sizeX/16, 
      sizeY/16, sizeX*14/16, sizeY*14/16, text);

    OguiButton *b;

    int choicesStartX = (sizeX - (choiceAmount * choiceSizeX)) / 2;
    int choicesStartY = (sizeY - (choiceSizeY + sizeY/8));

    for (int i = 0; i < choiceAmount; i++)
    {
      b = ogui->CreateSimpleTextButton(win, choicesStartX + (choiceSizeX * i), 
        choicesStartY, choiceSizeX, choiceSizeY, 
        choiceFiles[i][0], choiceFiles[i][1],
        choiceFiles[i][2], choiceTexts[i], i);
      b->SetListener(this);
      buttons->append(b);
    }

    // WARNING: This may cause problems if msgbox created while another
    // window is already the only active!
    // If so, fix by changing orvgui to store the only actives in a stack.
    win->SetOnlyActive();

  }

  MessageBoxWindow::~MessageBoxWindow()
  {
    win->RestoreAllActive();

    while (!buttons->isEmpty())
    {
      delete (OguiButton *)buttons->popLast();
    }
    delete buttons;
    if (textArea != NULL)
    {
      delete textArea;
      textArea = NULL;
    }
    if (win != NULL)
    {
      delete win;
      win = NULL;
    }
  }

	void MessageBoxWindow::setFadeoutTime(int duration)
	{
		win->SetEffectListener(this);
		this->fadeoutTime = duration;
	}

  void MessageBoxWindow::CursorEvent(OguiButtonEvent *eve)
  {
		if (this->clickedButtonId != -1)
		{
			// ignore doubleclicks
			return;
		}

		int butid = eve->triggerButton->GetId();
		if (fadeoutTime == 0)
		{
			if (listener == NULL)
			{
				Logger::getInstance()->debug("MessageBoxWindow::CursorEvent - No listener, deleting self!");
				delete this;  // ouch, nasty suicide ;)
				return;
			}
			listener->messageBoxClosed(this, id, butid);
			return;
		} else {
			this->clickedButtonId = butid;
			win->StartEffect(OGUI_WINDOW_EFFECT_FADEOUT, fadeoutTime);
		}
  }

  void MessageBoxWindow::EffectEvent(OguiEffectEvent *eve)
  {
		if (listener == NULL)
		{
			Logger::getInstance()->debug("MessageBoxWindow::CursorEvent - No listener, deleting self!");
			delete this;  // ouch, nasty suicide ;)
			return;
		}
		int butid = this->clickedButtonId;
		listener->messageBoxClosed(this, id, butid);
	}

	void MessageBoxWindow::raise()
	{
		win->Raise();
	}

}

