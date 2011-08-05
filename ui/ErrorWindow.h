
#ifndef ERRORWINDOW_H
#define ERRORWINDOW_H

#include "../system/Logger.h"
#include "../ogui/Ogui.h"

namespace ui
{

  class ErrorWindow : 
		public ILoggerListener, public IOguiButtonListener,
		public IOguiEffectListener
  {
  public:
    ErrorWindow(Ogui *ogui);
    ~ErrorWindow();

    virtual void logMessage(const char *msg, int level);
    virtual void CursorEvent(OguiButtonEvent *eve);
    virtual void EffectEvent(OguiEffectEvent *eve);
    void hide();
    void hideWithEffect();
    bool isVisible();
    void raise();
    void show();

		void clearMessages();
		void setMiniWindowMode(bool miniWindowMode);

		void setInputLine(const char *inputLine, const char *inputLineRight);

  private:
    Ogui *ogui;
    OguiWindow *win;
    OguiButton *closeBut;
    OguiTextLabel **messageText;
    OguiTextLabel *inputText;
    OguiTextLabel *inputQuery;
    int messages;
		bool miniWindowMode;
		bool isClosing;
  };

}

#endif
