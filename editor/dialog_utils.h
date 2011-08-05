// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_DIALOG_UTILS_H
#define INCLUDED_EDITOR_DIALOG_UTILS_H

#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif

namespace frozenbyte {
namespace editor {

class Dialog;

void setDialogItemText(Dialog &dialog, int id, const std::string &text);
void setDialogItemInt(Dialog &dialog, int id, int value);
void setDialogItemFloat(Dialog &dialog, int id, float value, int decimals = 2);

std::string getDialogItemText(Dialog &dialog, int id);
int getDialogItemInt(Dialog &dialog, int id);
float getDialogItemFloat(Dialog &dialog, int id);

void setSliderRange(Dialog &dialog, int id, int min, int max);
void setSliderValue(Dialog &dialog, int id, int value);
int getSliderValue(Dialog &dialog, int id);

void resetComboBox(Dialog &dialog, int id);
void addComboString(Dialog &dialog, int id, const std::string &string);
void setComboIndex(Dialog &dialog, int id, int index);
int getComboIndex(Dialog &dialog, int id);
void enableCheck(Dialog &dialog, int id, bool enable);
bool isCheckEnabled(Dialog &dialog, int id);

void enableDialogItem(Dialog &dialog, int id, bool enableState);

} // end of namespace editor
} // end of namespace frozenbyte

#endif
