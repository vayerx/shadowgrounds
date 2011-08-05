
#ifndef IMESSAGEBOXLISTENER_H
#define IMESSAGEBOXLISTENER_H

namespace ui
{

  class MessageBoxWindow;

  class IMessageBoxListener
  {
  public:
	  virtual ~IMessageBoxListener() {}
    virtual void messageBoxClosed(MessageBoxWindow *msgbox, int id, 
      int choice) = 0;
  };

}

#endif
