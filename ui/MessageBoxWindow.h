
#ifndef MESSAGEBOXWINDOW_H
#define MESSAGEBOXWINDOW_H

#include "../ogui/Ogui.h"
#include "IMessageBoxListener.h"

namespace ui
{

  /**
   * Generic message-box window.
   * Presents a message box with given text and and some choices.
   * After the user has clicked one of the buttons, the clicked button 
   * id is passed to given listener object.
   * The creator or listener should then delete the message box, but 
   * deleting within the listener callback function is not recommended 
   * (though should work). 
   *
   * @version 1.0, 25.6.2002
   * @author Jukka Kokkonen <jukka@frozenbyte.com>
   */

  class MessageBoxWindow : public IOguiButtonListener,
		public IOguiEffectListener
  {
  public:

    MessageBoxWindow(Ogui *ogui, const char *text, int choiceAmount, 
      const char **choiceTexts, const char ***choiceFiles, int sizeX, int sizeY,
      int choiceSizeX, int choiceSizeY, const char *backgroundFilename,
      int id, IMessageBoxListener *listener);
    ~MessageBoxWindow();

		void setFadeoutTime(int duration);
    
    virtual void CursorEvent(OguiButtonEvent *eve);

    virtual void EffectEvent(OguiEffectEvent *eve);

		void raise();

  private:
    int id;
    Ogui *ogui;
    OguiWindow *win;
    LinkedList *buttons;
    OguiTextLabel *textArea;
    IMessageBoxListener *listener;
		int clickedButtonId;
		int fadeoutTime;
  };

}

#endif

