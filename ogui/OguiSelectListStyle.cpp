
#include "precompiled.h"

#include <stdlib.h>
#include "OguiButtonStyle.h"
#include "OguiSelectListStyle.h"
#include "../util/Debug_MemoryManager.h"

OguiSelectListStyle::OguiSelectListStyle(
  OguiButtonStyle *unselectedItem,
  OguiButtonStyle *selectedItem,
  OguiButtonStyle *scrollUp,
  OguiButtonStyle *scrollDown,
  int sizeX, int sizeY, int scrollSizeX, int scrollSizeY)
{
  this->unselectedItem = unselectedItem;
  this->selectedItem = selectedItem;
  this->highlightedSelectedItem = NULL;
  this->highlightedUnselectedItem = NULL;
  this->scrollUp = scrollUp;
  this->scrollDown = scrollDown;
  this->sizeX = sizeX;
  this->sizeY = sizeY;
  this->scrollSizeX = scrollSizeX;
  this->scrollSizeY = scrollSizeY;
}

OguiSelectListStyle::OguiSelectListStyle(
    OguiButtonStyle *unselectedItem,
    OguiButtonStyle *selectedItem,
	OguiButtonStyle *highlightedUnselectedItem,
	OguiButtonStyle *highlightedSelectedItem,
    OguiButtonStyle *scrollUp,
    OguiButtonStyle *scrollDown,
    int sizeX, int sizeY,
    int scrollSizeX, int scrollSizeY )
{
  this->unselectedItem = unselectedItem;
  this->selectedItem = selectedItem;
  this->highlightedSelectedItem = highlightedUnselectedItem;
  this->highlightedUnselectedItem = highlightedSelectedItem;
  this->scrollUp = scrollUp;
  this->scrollDown = scrollDown;
  this->sizeX = sizeX;
  this->sizeY = sizeY;
  this->scrollSizeX = scrollSizeX;
  this->scrollSizeY = scrollSizeY;
}
