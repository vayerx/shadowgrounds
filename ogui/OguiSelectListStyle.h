
#ifndef OGUISELECTLISTSTYLE_H
#define OGUISELECTLISTSTYLE_H

//
// Container class for style definitions to be used in a select list
//

#include "OguiButtonStyle.h"


class OguiSelectList;
class Ogui;


class OguiSelectListStyle
{
public:
	OguiSelectListStyle(
    OguiButtonStyle *unselectedItem,
    OguiButtonStyle *selectedItem,
    OguiButtonStyle *scrollUp,
    OguiButtonStyle *scrollDown,
    int sizeX, int sizeY,
    int scrollSizeX, int scrollSizeY);

    OguiSelectListStyle(
    OguiButtonStyle *unselectedItem,
    OguiButtonStyle *selectedItem,
	OguiButtonStyle *highlightedUnselectedItem,
	OguiButtonStyle *highlightedSelectedItem,
    OguiButtonStyle *scrollUp,
    OguiButtonStyle *scrollDown,
    int sizeX, int sizeY,
    int scrollSizeX, int scrollSizeY);

  //~OguiSelectListStyle();

private:
  OguiButtonStyle *unselectedItem;
  OguiButtonStyle *selectedItem;
  OguiButtonStyle *highlightedUnselectedItem;
  OguiButtonStyle *highlightedSelectedItem;
  OguiButtonStyle *scrollUp;
  OguiButtonStyle *scrollDown;
  int sizeX;
  int sizeY;
  int scrollSizeX;
  int scrollSizeY;

  friend class OguiSelectList;
  friend class Ogui;
};

#endif

