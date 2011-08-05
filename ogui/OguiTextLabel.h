
#ifndef OGUITEXTLABEL_H
#define OGUITEXTLABEL_H

//
// Just wraps a disabled ogui text button -> giving us a simple text label
//
// v1.0.0 - 25.4.2002 - jpkokkon
//

#include "OguiButton.h"


class OguiTextLabel
{
public:
  OguiTextLabel(OguiButton *but);
  ~OguiTextLabel();

  void Move(int x, int y);
	void Resize(int x, int y);

  // TODO: should use own alignment enum, not button's...
  void SetTextHAlign(OguiButton::TEXT_H_ALIGN hAlign);
  void SetTextVAlign(OguiButton::TEXT_V_ALIGN vAlign);

  void SetFont(IOguiFont *font);

  void SetText(const char *text);

  void SetLinebreaks(bool lineBreaks);

  void SetTransparency( int transp_pros );

	OguiButton *GetButton();

private:

  OguiButton *implButton;
};

#endif

