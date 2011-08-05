#include "precompiled.h"

#include <boost/scoped_ptr.hpp>

#include "Ogui.h"
#include "orvgui2.h"

// if implemented properly, gotta get rid of this dependecy 
// (gotta redo this whole quick hack implementation actually)
#include "OguiStormDriver.h"

#include "OguiButton.h"
#include "OguiWindow.h"
#include "OguiSelectList.h"
#include <keyb3.h>

#include <map>

#include "../system/Logger.h"
#include "../container/LinkedList.h"

#include "../util/assert.h"
#include "../util/Debug_MemoryManager.h"


/* --------------------------------------------------------- */

// internal definitions
#define OGUI_CURSOR_STATES 40

#define MAX_DEFAULT_FONT_MISSING_WARNINGS 100
static int defaultFontMissingWarningsPrinted = 0;

/* --------------------------------------------------------- */

Ogui::Ogui()
{
	drv = NULL;
	defaultFont = NULL;
	cursorImages = new IOguiImage **[OG_CURSORS];
	cursorOffsetX = new int *[OG_CURSORS];
	cursorOffsetY = new int *[OG_CURSORS];
	for (int i = 0; i < OG_CURSORS; i++)
	{
		cursorImages[i] = new IOguiImage *[OGUI_CURSOR_STATES];
		cursorOffsetX[i] = new int[OGUI_CURSOR_STATES];
		cursorOffsetY[i] = new int[OGUI_CURSOR_STATES];
		for (int j = 0; j < OGUI_CURSOR_STATES; j++)
		{
			cursorImages[i][j] = NULL;
			cursorOffsetX[i][j] = 0;
			cursorOffsetY[i][j] = 0;
		}
	}
}

/* --------------------------------------------------------- */
	
Ogui::~Ogui()
{
	while (!buttons.empty())
	{
		// Bad old code: button destructor will remove the button from the list. Do not remove here!
		delete buttons.back();
	}
	buttons.clear();

	while (!windows.empty())
	{
		// Bad old code: window destructor will remove the window from the list. Do not remove here!
		delete windows.back();
	}
	windows.clear();

	if (defaultFont != NULL)
	{
		delete defaultFont;
		defaultFont = NULL;
	}
	for (int i = 0; i < OG_CURSORS; i++)
	{
		for (int j = 0; j < OGUI_CURSOR_STATES; j++)
		{
			if (cursorImages[i][j] != NULL) 
				delete cursorImages[i][j];
			cursorImages[i][j] = NULL;
		}
		delete [] cursorImages[i];
		delete [] cursorOffsetX[i];
		delete [] cursorOffsetY[i];
	}
	delete [] cursorImages;
	delete [] cursorOffsetX;
	delete [] cursorOffsetY;
}

/* --------------------------------------------------------- */

void Ogui::setHotkeysEnabled(bool enabled)
{
	og_hotkeys_enabled = enabled;
}

/* --------------------------------------------------------- */

void Ogui::UpdateCursorPositions()
{
	og_update_cursor_positions();
}

/* --------------------------------------------------------- */

void Ogui::SetDriver(IOguiDriver *drv)
{
	this->drv = drv;
}

/* --------------------------------------------------------- */

void Ogui::SetScale(int scaleX, int scaleY)
{
	og_set_scale(scaleX, scaleY);
}

/* --------------------------------------------------------- */

int Ogui::GetScaleX()
{
	return og_get_scale_x();
}

/* --------------------------------------------------------- */

int Ogui::GetScaleY()
{
	return og_get_scale_y();
}

/* --------------------------------------------------------- */

void Ogui::SetMouseSensitivity(float sensitivityX, float sensitivityY)
{
	og_set_mouse_sensitivity(sensitivityX, sensitivityY);
}

/* --------------------------------------------------------- */

void Ogui::Init()
{
	// nop
}

/* --------------------------------------------------------- */

void Ogui::Uninit()
{
	// nop
}

/* --------------------------------------------------------- */

void Ogui::RemovedWindow(OguiWindow *win)
{
	windows.remove(win);
}

/* --------------------------------------------------------- */

void Ogui::RemovedButton(OguiButton *but)
{
	buttons.remove(but);
}

/* --------------------------------------------------------- */

void Ogui::SetMenuIndexMode(int cursor, bool menuIndexModeEnabled)
{
	// TODO: cursor numbers!
	og_menu_index_mode = menuIndexModeEnabled;
}

/* --------------------------------------------------------- */

void Ogui::ResetData()
{
	std::list<OguiWindow*>::iterator iterw;
	for (iterw = windows.begin(); iterw != windows.end(); iterw++)
	{
		(*iterw)->ResetData();
	}
	std::list<OguiButton*>::iterator iterb;
	for (iterb = buttons.begin(); iterb != buttons.end(); iterb++)
	{
		(*iterb)->ResetData();
	}
	// FIXME: resets cursor image state to 0
	for (int i = 0; i < GetMaxCursors(); i++)
	{
		SetCursorImageState(i, 0);
	}
}

/* --------------------------------------------------------- */

// WARNING: invalidates any pointers to previous default font
// should only be used once, or recreate all buttons/textlabels after this
void Ogui::LoadDefaultFont(const char *filename) throw (OguiException *)
{
	if (drv == NULL)
	{
		Logger::getInstance()->warning("Ogui::LoadDefaultFont - No driver set.");
	}
	if (defaultFont != NULL)
	{
		delete defaultFont;
		defaultFont = NULL;
	}

	std::string lowerName = filename;
	for(unsigned int i = 0; i < lowerName.size(); ++i)
		lowerName[i] = tolower(lowerName[i]);

	defaultFont = drv->LoadFont(lowerName.c_str()
		);
}

/* --------------------------------------------------------- */

int Ogui::GetMaxCursors()
{
	return OG_CURSORS;
}

/* --------------------------------------------------------- */

void Ogui::SetCursorController(int cursornum, int controllermask)
	throw (OguiException *)
{
	if (cursornum < 0 || cursornum >= GetMaxCursors())
	{
		Logger::getInstance()->warning("Ogui::SetCursorController - Cursor number out of bounds.");
	}
	// NOTE: make sure orvgui and Ogui mask definitions are the same!
	og_set_cursor_control((unsigned char)cursornum, (unsigned int)controllermask);
}

/* --------------------------------------------------------- */

int Ogui::GetCursorController(int cursornum)
{
	if (cursornum < 0 || cursornum >= GetMaxCursors())
	{
		Logger::getInstance()->warning("Ogui::SetCursorController - Cursor number out of bounds.");
		return 0;
	}
	// NOTE: make sure orvgui and Ogui mask definitions are the same!
	return (int)og_get_cursor_control((unsigned char)cursornum);
}

/* --------------------------------------------------------- */

void Ogui::SetCursorControllerKeyboard(int cursornum, int key_left, int key_right, int key_up, int key_down, int key_fire, int key_fire2)
	throw (OguiException *)
{
	if (cursornum < 0 || cursornum >= GetMaxCursors())
	{
		Logger::getInstance()->warning("Ogui::SetCursorController - Cursor number out of bounds.");
	}
	// NOTE: make sure orvgui and Ogui mask definitions are the same!
	og_set_cursor_control_keyboard((unsigned char)cursornum, key_left, key_right, key_up, key_down, key_fire, key_fire2);
}

/* --------------------------------------------------------- */

void Ogui::LoadCursorImage(int cursornum, const char *filename, int forstate)
	throw (OguiException *)
{
	if (cursornum < 0 || cursornum >= GetMaxCursors())
	{
		Logger::getInstance()->warning("Ogui::LoadCursorImage - Cursor number out of bounds.");
	}
	if (forstate < 0 || forstate >= OGUI_CURSOR_STATES)
	{
		Logger::getInstance()->warning("Ogui::LoadCursorImage - Cursor state number is out of supported range.");
	}
	if (cursorImages[cursornum][forstate] != NULL)
	{
		delete cursorImages[cursornum][forstate];
	}
	if (filename != NULL)
	{
		cursorImages[cursornum][forstate] = drv->LoadOguiImage(filename);
	} else {
		cursorImages[cursornum][forstate] = NULL;
	}

	// change the cursor if we just loaded a new image for it's current state
	if (og_get_cursor_state(cursornum) == forstate)
	{
		if (cursorImages[cursornum][forstate] != NULL)
		{
			IStorm3D_Material *mat = 
				((OguiStormImage *)cursorImages[cursornum][forstate])->mat;
			og_set_cursor_pic(cursornum, mat);
		} else {
			og_set_cursor_pic(cursornum, NULL);
		}
		og_set_cursor_offset(cursornum, cursorOffsetX[cursornum][forstate],
			cursorOffsetY[cursornum][forstate]);
	}
}

/* --------------------------------------------------------- */

// NOTICE: maximum state is limited in current implementation, 
// although it should not be.
void Ogui::SetCursorImageState(int cursornum, int state) throw (OguiException *)
{
	if (cursornum < 0 || cursornum >= GetMaxCursors())
	{
		Logger::getInstance()->warning( "Ogui::SetCursorImageState - Cursor number out of bounds." );
	}
	if (state < 0 || state >= OGUI_CURSOR_STATES)
	{
		Logger::getInstance()->warning("Ogui::SetCursorImageState - Cursor state number is out of supported range.");
	}

	if (cursorImages[cursornum][state] != NULL)
	{
		IStorm3D_Material *mat = 
			((OguiStormImage *)cursorImages[cursornum][state])->mat;
		og_set_cursor_pic(cursornum, mat);
	} else {
		og_set_cursor_pic(cursornum, NULL);
	}
	og_set_cursor_offset(cursornum, cursorOffsetX[cursornum][state],
		cursorOffsetY[cursornum][state]);
}

/* --------------------------------------------------------- */

void Ogui::SwapCursorImages(int cursor1, int cursor2)
{
	for(int i = 0; i < OG_CURSORS; i++)
	{
		std::swap(cursorImages[i][cursor1], cursorImages[i][cursor2]);
		std::swap(cursorOffsetX[i][cursor1], cursorOffsetX[i][cursor2]);
		std::swap(cursorOffsetY[i][cursor1], cursorOffsetY[i][cursor2]);
	}
	swappedCursorImages.push_back( std::pair<int,int> (cursor1, cursor2) );
}

/* --------------------------------------------------------- */

void Ogui::ResetSwappedCursorImages()
{
	while(!swappedCursorImages.empty())
	{
		int cursor1 = swappedCursorImages.back().first;
		int cursor2 = swappedCursorImages.back().second;
		for(int i = 0; i < OG_CURSORS; i++)
		{
			std::swap(cursorImages[i][cursor1], cursorImages[i][cursor2]);
			std::swap(cursorOffsetX[i][cursor1], cursorOffsetX[i][cursor2]);
			std::swap(cursorOffsetY[i][cursor1], cursorOffsetY[i][cursor2]);
		}
		swappedCursorImages.pop_back();
	}
}

/* --------------------------------------------------------- */

void Ogui::SetCursorImageOffset(int cursornum, int offsetX, int offsetY, 
	int forstate) throw (OguiException *)
{
	if (cursornum < 0 || cursornum >= GetMaxCursors())
	{
		Logger::getInstance()->warning("Ogui::SetCursorImageOffset - Cursor number out of bounds.");
	}
	if (forstate < 0 || forstate >= OGUI_CURSOR_STATES)
	{
		Logger::getInstance()->warning("Ogui::SetCursorImageOffset - Cursor state number is out of supported range.");
	}

	cursorOffsetX[cursornum][forstate] = offsetX;
	cursorOffsetY[cursornum][forstate] = offsetY;

	// if offset changed for current cursor state, apply the new offset
	if (og_get_cursor_state(cursornum) == forstate)
	{
		og_set_cursor_offset(cursornum, cursorOffsetX[cursornum][forstate],
			cursorOffsetY[cursornum][forstate]);
	}
}

/* --------------------------------------------------------- */

OguiWindow *Ogui::CreateSimpleWindow(int x, int y, int sizex, int sizey, 
	const char *imagefilename, int id)
	throw (OguiException *)
{
	// NOTICE: image created here, but deleted by the window destructor
	IOguiImage *img = NULL;
	if (imagefilename != NULL) 
		img = drv->LoadOguiImage(imagefilename);

	OguiWindow *tmp = new OguiWindow(this, x, y, sizex, sizey, img, id);

	windows.push_back(tmp);  

	return tmp; 
}

/* --------------------------------------------------------- */

OguiButton *Ogui::CreateSimpleImageButton(OguiWindow *win, int x, int y, 
	int sizex, int sizey, const char *imageFilename, const char *imageDownFilename, 
	const char *imageHighlightFilename, const char *imageDisabledFilename, int id, void *argument, bool clipToWindow )
	throw (OguiException *)
{
	// these images loaded here get deleted by the button destructor 
	// they are marked as autodel
	IOguiImage *img = NULL; 
	IOguiImage *img2 = NULL; 
	IOguiImage *img3 = NULL; 
	IOguiImage *img4 = NULL; 
	if (imageFilename != NULL) img = drv->LoadOguiImage(imageFilename);
	if (imageDownFilename != NULL) img2 = drv->LoadOguiImage(imageDownFilename);
	if (imageHighlightFilename != NULL) img3 = drv->LoadOguiImage(imageHighlightFilename);
	if (imageDisabledFilename != NULL) img4 = drv->LoadOguiImage(imageDisabledFilename);

	OguiButton *tmp = win->CreateNewButton(x, y, sizex, sizey, img, img2, img3, img4,
		false, NULL, id, argument, NULL, clipToWindow );
	tmp->imageAutodel = true;
	tmp->imageDownAutodel = true;
	tmp->imageHighlightedAutodel = true;
	tmp->imageDisabledAutodel = true;

	buttons.push_back(tmp);
	
	return tmp;
}

/* --------------------------------------------------------- */

OguiButton *Ogui::CreateSimpleImageButton(OguiWindow *win, int x, int y, 
	int sizex, int sizey, const char *imageFilename, const char *imageDownFilename, 
	const char *imageHighlightFilename, int id, void *argument)
	throw (OguiException *)
{
	return this->CreateSimpleImageButton(win, x, y, sizex, sizey, imageFilename,
		imageDownFilename, imageHighlightFilename, NULL, id, argument);
}

/* --------------------------------------------------------- */

OguiButton *Ogui::CreateSimpleTextButton(OguiWindow *win, int x, int y, 
	int sizex, int sizey, const char *imageFilename, const char *imageDownFilename, 
	const char *imageHighlightFilename, const char *text, int id, const void *argument, bool clipToWindow )
	throw (OguiException *)
{
	// these images loaded here get deleted by the button destructor 
	// they are marked as autodel
	IOguiImage *img = NULL; 
	IOguiImage *img2 = NULL; 
	IOguiImage *img3 = NULL; 
	if (imageFilename != NULL) img = drv->LoadOguiImage(imageFilename);
	if (imageDownFilename != NULL) img2 = drv->LoadOguiImage(imageDownFilename);
	if (imageHighlightFilename != NULL) img3 = drv->LoadOguiImage(imageHighlightFilename);

	if (defaultFont == NULL && defaultFontMissingWarningsPrinted < MAX_DEFAULT_FONT_MISSING_WARNINGS)
	{
		Logger::getInstance()->warning("Ogui::CreateSimpleTextButton - No default font set.");
		defaultFontMissingWarningsPrinted++;
	}

	OguiButton *tmp = win->CreateNewButton(x, y, sizex, sizey, img, img2, img3, NULL,
		true, text, id, argument, defaultFont, clipToWindow );
	tmp->imageAutodel = true;
	tmp->imageDownAutodel = true;
	tmp->imageHighlightedAutodel = true;

	buttons.push_back(tmp);
	
	return tmp;
}

/* --------------------------------------------------------- */

OguiTextLabel *Ogui::CreateTextLabel(OguiWindow *win, int x, int y, 
	int sizex, int sizey, const char *text)
	throw (OguiException *)
{
	if (defaultFont == NULL && defaultFontMissingWarningsPrinted < MAX_DEFAULT_FONT_MISSING_WARNINGS)
	{
		Logger::getInstance()->warning("Ogui::CreateTextLabel - No default font set.");
		defaultFontMissingWarningsPrinted++;
	}

	OguiButton *tmp = win->CreateNewButton(x, y, sizex, sizey, NULL, NULL, NULL, NULL, true, text, 0, NULL, defaultFont);
	tmp->SetDisabled(true);

	OguiTextLabel *ret = new OguiTextLabel(tmp);

	buttons.push_back(tmp);

	return ret;
}

/* --------------------------------------------------------- */

OguiTextLabel *Ogui::CreateTextArea(OguiWindow *win, int x, int y, 
	int sizex, int sizey, const char *text)
	throw (OguiException *)
{
	if (defaultFont == NULL && defaultFontMissingWarningsPrinted < MAX_DEFAULT_FONT_MISSING_WARNINGS)
	{
		Logger::getInstance()->warning("Ogui::CreateTextArea - No default font set.");
		defaultFontMissingWarningsPrinted++;
	}

	OguiButton *tmp = win->CreateNewButton(x, y, sizex, sizey, NULL, NULL, NULL, NULL, true, "", 0, NULL, defaultFont);
	
	tmp->SetLineBreaks(true);
	tmp->SetDisabled(true);
	tmp->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
	tmp->SetTextVAlign(OguiButton::TEXT_V_ALIGN_TOP);

	OguiTextLabel *ret = new OguiTextLabel(tmp);

	buttons.push_back(tmp);

	ret->SetText(text);

	return ret; 
}

/* --------------------------------------------------------- */

OguiSelectList *Ogui::CreateSelectList(OguiWindow *win, int x, int y, 
	OguiSelectListStyle *style, int valueAmount, const char **values, const char **descs, 
	bool multiSelectable, int defaultSelection, int id, void *argument)
	throw (OguiException *)
{

	OguiButton **listb;

	// amount of buttons needed for the item list
	int butSizeX = style->unselectedItem->sizeX;
	int butSizeY = style->unselectedItem->sizeY;
	FB_ASSERT( butSizeY != 0 );
	
	int needButs = 0;
	if( butSizeY != 0 )
		needButs = style->sizeY / butSizeY;
	

	// NOTICE: buttons created here will be deleted in selectlist's destructor
	// create item list buttons
	listb = new OguiButton *[needButs];
	for (int i = 0; i < needButs; i++)
	{
		listb[i] = win->CreateNewButton(x, y + i * butSizeY, butSizeX, butSizeY, NULL, NULL, NULL, NULL, true, NULL, i, NULL, NULL);
		listb[i]->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
		listb[i]->SetEventMask(OGUI_EMASK_ALL ^ OGUI_EMASK_HOLD);
		//listb[i]->SetTextVAlign(OguiButton::TEXT_V_ALIGN_MIDDLE);
		buttons.push_back(listb[i]);
	}

	// create scroll buttons
	OguiButton *upsb = win->CreateNewButton(x + butSizeX, y, style->scrollSizeX, style->scrollSizeY, NULL, NULL, NULL, NULL, false, NULL, 0, NULL, NULL);
	OguiButton *downsb = win->CreateNewButton(x + butSizeX, y + style->sizeY - style->scrollSizeY, style->scrollSizeX, style->scrollSizeY, NULL, NULL, NULL, NULL, false, NULL, 0, NULL, NULL);
	buttons.push_back(upsb);
	buttons.push_back(downsb);

	OguiSelectList *ret = new OguiSelectList(x, y, defaultSelection, multiSelectable, 
		valueAmount, values, descs, listb, upsb, downsb, style, id, argument);

	upsb->SetListener(ret);
	downsb->SetListener(ret);
	upsb->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_PRESS | OGUI_EMASK_HOLD);
	downsb->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_PRESS | OGUI_EMASK_HOLD);
	for (int j = 0; j < needButs; j++)
		listb[j]->SetListener(ret);

	delete [] listb;

	return ret; 
}

/* --------------------------------------------------------- */

bool Ogui::isEscape()
{
	if (og_escape)
		return true;
	else 
		return false;
}

/* --------------------------------------------------------- */

int Ogui::getCursorScreenX(int cursornum, bool exact)
{
	return og_get_cursor_position_x(cursornum, exact);
}

/* --------------------------------------------------------- */

int Ogui::getCursorScreenY(int cursornum, bool exact)
{
	return og_get_cursor_position_y(cursornum, exact);
}

/* --------------------------------------------------------- */

void Ogui::setCursorScreenX(int cursornum, int screenX)
{
	og_set_cursor_position_x(cursornum, screenX);
}

/* --------------------------------------------------------- */

void Ogui::setCursorScreenY(int cursornum, int screenY)
{
	og_set_cursor_position_y(cursornum, screenY);
}

/* --------------------------------------------------------- */

void Ogui::setCursorScreenOffsetX(int cursornum, int screenOffsetX)
{
	og_set_cursor_position_offset_x(cursornum, screenOffsetX);
}

/* --------------------------------------------------------- */

void Ogui::setCursorScreenOffsetY(int cursornum, int screenOffsetY)
{
	og_set_cursor_position_offset_y(cursornum, screenOffsetY);
}

/* --------------------------------------------------------- */

void Ogui::skipCursorMovement()
{
	og_set_skip_cursor_movement();
}

/* --------------------------------------------------------- */

void Ogui::UpdateEffects(int timeDelta)
{
	// window effects...
	std::list<OguiWindow*>::iterator iter;
	for (iter = windows.begin(); iter != windows.end(); iter++)
	{
		OguiWindow *win = *iter;

		// BUGBUG, FIXME
		// Crashes sometimes because win points to a window which is deleted.

		// text type effect ( added by Pete )
		if( win->effectTextTypeTimeLeft > 0 ) 
		{
			win->effectTextTypeTimeLeft -= timeDelta;
			if( win->effectTextTypeTimeLeft <= 0 )
			{
				win->effectTextTypeTimeLeft = 0;
			}
			
			float value = 1.0f - ( (float)win->effectTextTypeTimeLeft / (float)win->effectTextTypeTimeTotal );

			// bool winStillExists = win->buttonList->isEmpty();

			bool sendEvent = false;
			SafeLinkedListIterator i( win->buttonList );
			while( i.iterateAvailable() )
			{
				
				OguiButton* button = (OguiButton*)i.iterateNext();
				
				if( button->SetText( button->text.substr( 0, (unsigned int)( value * button->text.size() ) ).c_str() ) )
				{
					sendEvent = true;
				}
			
			}
			if ( sendEvent )
			{
				if( win->effectListener )
				{
					OguiEffectEvent *eve = new OguiEffectEvent( OguiEffectEvent::EVENT_TYPE_TEXTTYPE, win );
					win->effectListener->EffectEvent(eve);
					delete eve;

					// Make sure the above event has not deleted this window!
					bool winStillExists = false;

					std::list<OguiWindow*>::iterator chkiter;
					for (chkiter = windows.begin(); chkiter != windows.end(); chkiter++)
					{
						if (*chkiter == win)
						{
							winStillExists = true;
							break;
						}
					}
					if (!winStillExists)
					{
						// FB_ASSERT( false && "the window got destroyed, skip any further effects (or crash would occur)" );
						// the window got destroyed, skip any further effects (or crash would occur)
						break;
					}
				}
			}
		}

		// Text Line appear
		if( win->effectTextLineTimeLeft > 0 ) 
		{
			win->effectTextLineTimeLeft -= timeDelta;
			if( win->effectTextLineTimeLeft <= 0 )
			{
				win->effectTextLineTimeLeft = 0;
			}
			
			float value = 1.0f - ( (float)win->effectTextLineTimeLeft / (float)win->effectTextLineTimeTotal );

			int number_of_buttons = 0;
			std::map< int, OguiButton* > buttons;

			{
				SafeLinkedListIterator i( win->buttonList );
				while( i.iterateAvailable() )
				{
					OguiButton* button = (OguiButton*)i.iterateNext();
					buttons.insert( std::pair< int, OguiButton* >( ( (orvgui_but*)( button->but ) )->put_y, button ) );
					number_of_buttons++;
				}
			}

			{
				number_of_buttons = (int)( (float)number_of_buttons * value );
				int j;
				std::map< int, OguiButton* >::iterator i;

				for( j = 0, i = buttons.begin(); i != buttons.end(); ++j, ++i )
				{
					if( j < number_of_buttons )
					{
						i->second->SetText( i->second->text.c_str() );
					}
					else
					{
						i->second->SetText( NULL );
					}
				}
			}

			if( win->effectListener )
			{
				OguiEffectEvent *eve = new OguiEffectEvent( OguiEffectEvent::EVENT_TYPE_TEXTLINE, win );
				win->effectListener->EffectEvent(eve);
				delete eve;

				// Make sure the above event has not deleted this window!
				bool winStillExists = false;

				std::list<OguiWindow*>::iterator chkiter;
				for (chkiter = windows.begin(); chkiter != windows.end(); chkiter++)
				{
					if (*chkiter == win)
					{
						winStillExists = true;
						break;
					}
				}
				if (!winStillExists)
				{
					// FB_ASSERT( false && "the window got destroyed, skip any further effects (or crash would occur)" );
					// the window got destroyed, skip any further effects (or crash would occur)
					break;
				}
			}
			
		}

		// fade-in/out?
		if (win->effectFadeTimeLeft > 0)
		{
			win->effectFadeTimeLeft -= timeDelta;
			if (win->effectFadeTimeLeft <= 0)
			{
				win->effectFadeTimeLeft = 0;
			}
			int fadeValue = (100 * win->effectFadeTimeLeft / win->effectFadeTimeTotal);
			if (win->fadingOut)
				fadeValue = 100 - fadeValue;
			og_set_transparency_window(((orvgui_win *)win->win), fadeValue);
			if (win->effectFadeTimeLeft == 0)
			{
				OguiEffectEvent::EVENT_TYPE eveType;
				if (win->fadingIn)
					eveType = OguiEffectEvent::EVENT_TYPE_FADEDIN;
				else
					eveType = OguiEffectEvent::EVENT_TYPE_FADEDOUT;
				win->fadingIn = false;
				win->fadingOut = false;
				if (win->effectListener != NULL)
				{
					{
						boost::scoped_ptr<OguiEffectEvent> eve(new OguiEffectEvent(eveType, win));
						win->effectListener->EffectEvent(eve.get());
					}

					// Make sure the above event has not deleted this window!
					bool winStillExists = false;
					std::list<OguiWindow*>::iterator chkiter;
					for (chkiter = windows.begin(); chkiter != windows.end(); chkiter++)
					{
						if (*chkiter == win)
						{
							winStillExists = true;
							break;
						}
					}
					if (!winStillExists)
					{
						// FB_ASSERT( false && "the window got destroyed, skip any further effects (or crash would occur)" );
						// the window got destroyed, skip any further effects (or crash would occur)
						break;
					}
				}
			}
		}

		// move-out/in
		if (win->effectMoveTimeLeft > 0)
		{
			win->effectMoveTimeLeft -= timeDelta;
			if (win->effectMoveTimeLeft <= 0)
			{
				win->effectMoveTimeLeft = 0;
			}
			int movePerc = (100 * win->effectMoveTimeLeft / win->effectMoveTimeTotal);
			if (win->movingOut)
				movePerc = 100 - movePerc;

			// TODO: move to closest edge, now always moving to upper edge
			int moveAmount = ((win->windowPositionY + ((orvgui_win *)win->win)->sizey) * movePerc) / 100;
			og_force_move_window(((orvgui_win *)win->win), win->windowPositionX, win->windowPositionY - moveAmount);

			if (win->effectMoveTimeLeft == 0)
			{
				OguiEffectEvent::EVENT_TYPE eveType;
				if (win->fadingIn)
					eveType = OguiEffectEvent::EVENT_TYPE_MOVEDIN;
				else
					eveType = OguiEffectEvent::EVENT_TYPE_MOVEDOUT;
				win->movingIn = false;
				win->movingOut = false;
				if (win->effectListener != NULL)
				{
					OguiEffectEvent *eve = new OguiEffectEvent(eveType, win);
					win->effectListener->EffectEvent(eve);
					delete eve;

					// TODO: if further effects would be added, have to check that the window
					// has not been deleted (see the fade-in/out effect above)
					// FIXME: should break if window has been deleted
				}
			}
		}
	}
}

/* --------------------------------------------------------- */

void Ogui::Run(int timeDelta)
{
	UpdateEffects(timeDelta);

	drv->updateVideos();
	og_run_gui();
	og_draw_screen();
}

/* --------------------------------------------------------- */

void Ogui::ShowError(char *msg)
{
	og_show_error(msg);
}

/* --------------------------------------------------------- */

void Ogui::ShowError(OguiException *ex)
{
	const char *foo = ex->GetErrorMessage();
	og_show_error(foo);
}

/* --------------------------------------------------------- */

IOguiImage *Ogui::LoadOguiImage(const char *filename)
	throw (OguiException *)
{
	return drv->LoadOguiImage(filename);
}

IOguiImage *Ogui::LoadOguiImage(int width, int height)
	throw (OguiException *)
{
	return drv->LoadOguiImage(width, height);
}

IOguiImage *Ogui::GetOguiRenderTarget(int index) throw (OguiException *)
{
	return drv->GetOguiRenderTarget(index);
}

IOguiImage *Ogui::LoadOguiVideo( const char* filename, IStorm3D_StreamBuilder *streamBuilder )
{
	return drv->LoadOguiVideo( filename, streamBuilder );
}

IOguiImage* Ogui::ConvertVideoToImage( IStorm3D_VideoStreamer* stream, IStorm3D_StreamBuilder* streamBuilder )
{
	return drv->ConvertVideoToImage( stream, streamBuilder );
}

/* --------------------------------------------------------- */

IOguiFont *Ogui::LoadFont(const char *filename)
	throw (OguiException *)
{
	if(!filename)
		return 0;

	std::string lowerName = filename;
	for(unsigned int i = 0; i < lowerName.size(); ++i)
		lowerName[i] = tolower(lowerName[i]);

	return drv->LoadFont(lowerName.c_str());
}

/* --------------------------------------------------------- */

void Ogui::setVisualizeWindows(bool debugVisualize)
{
	og_visualize_windows = debugVisualize;
}

/* --------------------------------------------------------- */

int Ogui::getScreenSizeX()
{
	return og_get_scr_size_x();
}

int Ogui::getScreenSizeY()
{
	return og_get_scr_size_y();
}
