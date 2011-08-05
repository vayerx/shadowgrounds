
#include "precompiled.h"

#include <string.h>
#include <stdlib.h>

// these are for temp debugging...
#include "../convert/str2int.h"
#include "../system/Logger.h"
#include "../system/Timer.h"
#include "../ui/uidefaults.h"
#include "OguiSelectList.h"
#include "../util/Debug_MemoryManager.h"
#include "../util/assert.h"

class OguiWindow;


static int oguiNextScrollHoldTime = 0;
static int oguiScrollHoldAmount = 0;

void OguiSelectList::CursorEvent(OguiButtonEvent *eve)
{
	OguiSelectListEvent::EVENT_TYPE et = 
		OguiSelectListEvent::EVENT_TYPE_SCROLL;
	bool sendEvent = false;

	int num = -1;

	bool scrollup = eve->eventType == OguiButtonEvent::EVENT_TYPE_CLICK && (eve->cursorOldButtonMask & OGUI_BUTTON_WHEEL_UP_MASK) ;
	bool scrolldown = eve->eventType == OguiButtonEvent::EVENT_TYPE_CLICK && (eve->cursorOldButtonMask & OGUI_BUTTON_WHEEL_DOWN_MASK) ;
	
	if(scrollup)
	{
		scrollUp ();
		et = OguiSelectListEvent::EVENT_TYPE_SCROLL;
		sendEvent = true;
	}
	else if(scrolldown)
	{
		scrollDown ();
		et = OguiSelectListEvent::EVENT_TYPE_SCROLL;
		sendEvent = true;
	} 
	else if (eve->triggerButton == downScrollBut
		|| eve->triggerButton == upScrollBut)
	{
		bool doScroll = false;

		// HACK; using timer!
		if (eve->eventType == OguiButtonEvent::EVENT_TYPE_PRESS)
		{
			oguiNextScrollHoldTime = Timer::getTime() + 500;
			oguiScrollHoldAmount = 0;
		}
		if (eve->eventType == OguiButtonEvent::EVENT_TYPE_CLICK)
		{
			doScroll = true;
			oguiNextScrollHoldTime = 0;
			oguiScrollHoldAmount = 0;
		}
		if (eve->eventType == OguiButtonEvent::EVENT_TYPE_HOLD)
		{
			if (Timer::getTime() >= oguiNextScrollHoldTime)
			{
				int delay = 100 - oguiScrollHoldAmount * 2;
				if (delay < 50) delay = 50;
				oguiNextScrollHoldTime = Timer::getTime() + delay;
				oguiScrollHoldAmount++;
				doScroll = true;
			}
		}
		if (doScroll)
		{
			if (eve->triggerButton == upScrollBut)
			{
				scrollUp();
			} else {
				scrollDown();
			}
			et = OguiSelectListEvent::EVENT_TYPE_SCROLL;
			sendEvent = true;
		}
	} else {
		num = scrolly + eve->triggerButton->GetId();
		
		if (num < 0 || num >= amount) 
		{
			#ifdef _DEBUG
				abort();
			#else
				return;
			#endif
		}

		if (eve->eventType == OguiButtonEvent::EVENT_TYPE_CLICK && !scrolldown && !scrollup)
		{
			if (multiSelectable)
			{
				if (selected[num])
				{
					selected[num] = false;
					et = OguiSelectListEvent::EVENT_TYPE_UNSELECT;
				} else {
					selected[num] = true;
					et = OguiSelectListEvent::EVENT_TYPE_SELECT;
				}
			} else {
				if (selected[num])
				{
					et = OguiSelectListEvent::EVENT_TYPE_DOUBLESELECT;
				} else {
					for (int i = 0; i < amount; i++) 
						selected[i] = false;
					selected[num] = true;
					et = OguiSelectListEvent::EVENT_TYPE_SELECT;
				}
			}
			refresh();
			sendEvent = true;
		} else {
			if (eve->eventType == OguiButtonEvent::EVENT_TYPE_OVER)
			{
				et = OguiSelectListEvent::EVENT_TYPE_CURSOR_OVER;
				sendEvent = true;
			}
			if (eve->eventType == OguiButtonEvent::EVENT_TYPE_LEAVE)
			{
				et = OguiSelectListEvent::EVENT_TYPE_CURSOR_LEAVE;
				sendEvent = true;
			}
		}
	}
	if (listener != NULL)
	{
		char *val = NULL;
		char *desc = NULL;
		if (num >= 0 && num < amount) 
		{
			val = values[num];
			desc = descs[num];
		}
		if (sendEvent)
		{
			OguiSelectListEvent *tmp = new OguiSelectListEvent(et, eve->cursorNumber,
				eve->cursorScreenX, eve->cursorScreenY, eve->cursorButtonMask,
				eve->cursorOldButtonMask, num, val, desc, scrolly, this, eve->triggerWindow,
				NULL);
			listener->SelectEvent(tmp);
			delete tmp;
		}
	}
}

void OguiSelectList::setStyle(OguiSelectListStyle *style)
{
	this->style = style;

	upScrollBut->SetStyle(style->scrollUp);
	//upScrollBut->SetImage(style->scrollUp->image);
	//upScrollBut->SetDownImage(style->scrollUp->imageDown);
	//upScrollBut->SetDisabledImage(style->scrollUp->imageDisabled);
	//upScrollBut->SetSize(style->scrollSizeX, style->scrollSizeY);

	downScrollBut->SetStyle(style->scrollDown);
	//downScrollBut->SetImage(style->scrollDown->image);
	//downScrollBut->SetDownImage(style->scrollDown->imageDown);
	//downScrollBut->SetDisabledImage(style->scrollDown->imageDisabled);
	//downScrollBut->SetSize(style->scrollSizeX, style->scrollSizeY);

	refresh();
}

void OguiSelectList::scrollUp()
{
	if (scrolly > 0) 
	{
		scrolly--;
		refresh();
	}
}

void OguiSelectList::scrollDown()
{
	if (scrolly < amount - listButAmount)
	{
		scrolly++;
		refresh();
	}
}

void OguiSelectList::scrollTo(int offset)
{
	if (offset > amount - listButAmount) offset = amount - listButAmount;
	if (offset < 0) offset = 0;

	scrolly = offset;
	refresh();
}

/*
// old stuff
void OguiSelectList::click()
{
	// THIS SHOULD NOT USE ORVGUI (og_cursor_y) DIRECTLY!
	selected = scrolly + (og_cursor_y / scaley);
	if (selected < 0) selected = 0;
	if (selected >= amount) selected = amount - 1;

	refresh();
	selectHandler(win, selected, values[selected]);
}
*/

void OguiSelectList::setListener(IOguiSelectListListener *listener)
{
	this->listener = listener;
}

void OguiSelectList::refresh()
{
	// disable/enable scroll buttons
	if (scrolly <= 0)
	{
		upScrollBut->SetDisabled(true);
	} else {
		upScrollBut->SetDisabled(false);
	}
	if (scrolly >= amount - listButAmount)
	{
		downScrollBut->SetDisabled(true);
	} else {
		downScrollBut->SetDisabled(false);
	}

	// make items selected/unselected and change correct text to them
	for (int i = 0; i < listButAmount; i++)
	{
		if (scrolly + i >= 0 && scrolly + i < amount)
		{
			if( style && 
				style->highlightedSelectedItem && 
				style->highlightedUnselectedItem && 
				highlighted[ scrolly + i ] )
			{
				if (selected[scrolly + i])
				{
					listButs[ i ]->SetStyle( style->highlightedSelectedItem );
				}
				else
				{
					listButs[ i ]->SetStyle( style->highlightedUnselectedItem );
				}
			
			}
			else
			{
				if (selected[scrolly + i])
				{
					listButs[i]->SetStyle(style->selectedItem);
					//listButs[i]->SetImage(style->selectedItem->image);
					//listButs[i]->SetDownImage(style->selectedItem->imageDown);
					//listButs[i]->SetDisabledImage(style->selectedItem->imageDisabled);
				} else {
#ifdef PROJECT_SHADOWGROUNDS
					listButs[i]->SetStyle(style->highlightedUnselectedItem);
#else
					listButs[i]->SetStyle(style->unselectedItem);
#endif
					//listButs[i]->SetImage(style->unselectedItem->image);
					//listButs[i]->SetDownImage(style->unselectedItem->imageDown);
					//listButs[i]->SetDisabledImage(style->unselectedItem->imageDisabled);
				}

			}

			listButs[i]->SetText(descs[scrolly + i]);
			listButs[i]->SetDisabled(false);
		} else {
#ifdef PROJECT_SHADOWGROUNDS
			listButs[i]->SetStyle(style->highlightedUnselectedItem);
#else
			listButs[i]->SetStyle(style->unselectedItem);
#endif
			//listButs[i]->SetImage(style->unselectedItem->image);
			//listButs[i]->SetDownImage(style->unselectedItem->imageDown);
			//listButs[i]->SetDisabledImage(style->unselectedItem->imageDisabled);
			listButs[i]->SetText(NULL);
			listButs[i]->SetDisabled(true);
		}
	}

	// old stuff...
	/*
	og_fbox(win->get_ogwin(), x, y, x + sizex - 16 - 1, y + sizey - 1, og_get_bg_color());	
	if (selected >= scrolly && selected < scrolly + (sizey / 8))
	{
		og_fbox(win->get_ogwin(), x, y + (selected - scrolly) * 8, x + sizex - 16 - 1, y + ((selected - scrolly) * 8) + (8-1), og_get_selection_color());
	}
	*/

	/*
	char *wbuf = new char[((sizex - 16) / 8) + 1];
	for (int i = 0; i < sizey / 8; i++)
	{
		if (scrolly + i >= 0 && scrolly + i < amount)
		{
			wbuf[((sizex - 16) / 8)] = '\0';
			strncpy(wbuf, descs[scrolly + i], ((sizex - 16) / 8));
			win->writeText(x, y + i * 8, wbuf);
		}
	}
	*/
}

void OguiSelectList::init(int x, int y, int defsel, bool multisel, int amount, const char **values, const char **descs, OguiButton **listButs, OguiButton *upBut, OguiButton *downBut, OguiSelectListStyle *style, int id, void *argument)
{ 
	// old stuff...
	//win->addTransparentButton(x, y, sizex - 16, sizey, cback_clk, this, listtabi);
	//win->addPicButton(x + sizex - 16, y, 16, 16, DEFBUT_X_SCROLLUP, DEFBUT_Y_SCROLLUP, cback_up, uptabi, this);
	//win->addPicButton(x + sizex - 16, y + sizey - 16, 16, 16, DEFBUT_X_SCROLLDOWN, DEFBUT_Y_SCROLLDOWN, cback_down, downtabi, this);

	// check that all item/scroll button styles are ok (correct sizes)	
	// #ifdef _DEBUG
	/*
		FB_ASSERT( !(style->scrollDown->sizeX != style->scrollUp->sizeX) );
		FB_ASSERT( !(style->scrollDown->sizeY != style->scrollUp->sizeY) );
		FB_ASSERT( !(style->selectedItem->sizeY != style->unselectedItem->sizeY) );
		FB_ASSERT( !(style->scrollDown->sizeX + style->unselectedItem->sizeX
			!= style->sizeX ) );
	*/
	//#endif
	
	if( style->unselectedItem->sizeY != 0 )
		this->listButAmount = style->sizeY / style->unselectedItem->sizeY;
	else 
		this->listButAmount = 0;

	this->listButs = new OguiButton *[listButAmount];
	{
		for (int i = 0; i < listButAmount; i++)
		{
			this->listButs[i] = listButs[i];
		}
	}
	this->downScrollBut = downBut;
	this->upScrollBut = upBut;
	this->x = x;
	this->y = y;
	this->sizex = style->sizeX;
	this->sizey = style->sizeY;
	this->scaleY = style->selectedItem->sizeY;
	this->scrollSizeX = style->scrollUp->sizeX;
	this->scrollSizeY = style->scrollUp->sizeY;
	if (this->sizex < scrollSizeX) this->sizex = this->scrollSizeX;
	if (this->sizey < scrollSizeY * 2) this->sizey = this->scrollSizeY * 2;
	this->scrolly = 0;
	this->multiSelectable = multisel;
	this->selected = new bool[amount];
	this->highlighted = new bool[amount];
	{
		for (int i = 0; i < amount; i++)
		{
			selected[i] = false;
			highlighted[i] = false;
		}
	}
	this->highlightNew = false;

	if (defsel >= 0 && defsel < amount) selected[defsel] = true;
	this->amount = amount;
	this->allocedAmount = amount;
	if (amount == 0) allocedAmount = 1;
	this->values = new char *[allocedAmount];
	this->descs = new char *[allocedAmount];
	{
		for (int i = 0; i < amount; i++)
		{
			this->values[i] = new char[strlen(values[i]) + 1];
			strcpy(this->values[i], values[i]);
			this->descs[i] = new char[strlen(descs[i]) + 1];
			strcpy(this->descs[i], descs[i]);
		}
	}
	this->id = id;
	this->argument = argument;
	this->listener = NULL;
	

	/* done by refresh?
	this->upScrollBut->SetDisabled(true);
	if (amount < listButAmount)
	{
		this->downScrollBut->SetDisabled(true);
	}
	*/

	setStyle(style);
	// setstyle has already called refresh
	//refresh();
}


void OguiSelectList::setSelected(int position, bool selected)
{
	if (position < 0 || position > amount) 
	{
#ifdef _DEBUG
		abort();
#endif
		return;
	}

	this->selected[position] = selected;
	refresh();
}

// position -1 can be used to add after last item (0 adds before first item)
// WARNING: calling this inside a select event may invalidate value pointers 
// given to event listener
void OguiSelectList::addItem(const char *value, const char *desc, bool selected, int position)
{ 
	// TODO: replace the array structure with a some better one

	if (position < 0 || position > amount) position = amount;

	bool *old_selected = this->selected;
	bool *old_highlighted = this->highlighted;
	char **old_values = values;
	char **old_descs = descs;
	bool free_old_ones = false;

	if (this->allocedAmount < amount + 1)
	{
		free_old_ones = true;
		this->selected = new bool[amount + 1];
		this->highlighted = new bool[amount + 1];
		for (int i = 0; i < amount; i++)
		{
			this->selected[i] = old_selected[i];
			this->highlighted[i] = old_highlighted[i];
		}

		this->values = new char *[amount + 1];
		this->descs = new char *[amount + 1];
		for (int i = 0; i < amount; i++)
		{
			if (old_values[i] != NULL)
			{
				//this->values[i] = new char[strlen(old_values[i]) + 1];
				//strcpy(this->values[i], old_values[i]);
				this->values[i] = old_values[i];
			} else {
				this->values[i] = NULL;
			}
			//this->descs[i] = new char[strlen(old_descs[i]) + 1];
			//strcpy(this->descs[i], old_descs[i]);
			this->descs[i] = old_descs[i];
		}
		this->values[ amount ] = NULL;
		this->descs[ amount ] = NULL;
		allocedAmount = amount + 1;
	}

	this->amount++;

	// very unefficient to do this like this... should optimize
	// note, handling allocated char array pointers here
	for (int i = amount - 1; i > position; i--)
	{
		values[i] = values[i - 1];
		descs[i] = descs[i - 1];
		this->selected[i] = this->selected[i - 1];
		this->highlighted[i] = this->highlighted[i - 1];
	}
	if (value != NULL)
	{
		values[position] = new char[strlen(value) + 1];
		strcpy(values[position], value);
	} else {
		values[position] = NULL;
	} 
	descs[position] = new char[strlen(desc) + 1];
	strcpy(descs[position], desc);
	this->selected[position] = selected;
	this->highlighted[position] = highlightNew;

	// added by Pete
	if( free_old_ones )
	{
		delete [] old_highlighted;
		delete [] old_selected;
		delete [] old_values;
		delete [] old_descs;
	}

	refresh();
}

void OguiSelectList::highlightItem( int position, bool highlight )
{
	if( position >= 0 && position < amount )
	{
		if( highlighted[ position ] != highlight )
		{
			highlighted[ position ] = highlight;
			refresh();
		}
	}
}

// position -1 can be used to delete last item (0 deletes first item)
void OguiSelectList::deleteItem(int position)
{ 
	if (amount == 0) abort(); // can't delete, already empty

	if (position < 0 || position >= amount) position = amount - 1;

	if (values[position] != NULL)
	{
		//Logger::getInstance()->debug("Deleting position, value:");
		//Logger::getInstance()->debug(int2str(position));
		//Logger::getInstance()->debug(int2str((int)&values[position]));
		//Logger::getInstance()->debug(values[position]);
		//values[position][0] = '*';
		//values[position][1] = '\0';

// TODO: uncomment this...

		delete [] values[position];
		values[position] = NULL;
		
		//Logger::getInstance()->debug("Done");
	}
	delete [] descs[position];

	for (int i = position; i < amount - 1; i++)
	{
		values[i] = values[i + 1];
		descs[i] = descs[i + 1];
		selected[i] = selected[i + 1];
	}

	amount--;

	refresh();
}

bool OguiSelectList::isEmpty()
{
	if (amount == 0) 
		return true;
	else
		return false;
}

void OguiSelectList::uninit()
{
	if (listButs == NULL) abort(); // bug
	if (values == NULL) abort(); // bug
	if (descs == NULL) abort(); // bug
	if (selected == NULL) abort(); // bug

	delete [] selected;
	delete [] highlighted;

	int i;
	for (i = 0; i < listButAmount; i++)
	{
		delete listButs[i];
	}
	delete [] listButs;
	listButs = NULL;

	if (downScrollBut != NULL) delete downScrollBut;
	if (upScrollBut != NULL) delete upScrollBut;
	downScrollBut = NULL;
	upScrollBut = NULL;
	
	for (i = 0; i < amount; i++)
	{
		if (values[i] != NULL) 
		{
			delete [] values[i];
		}
		delete [] descs[i];
	}
	delete [] values;
	delete [] descs;
	values = NULL;
	descs = NULL;
}

int OguiSelectList::getId()
{
	return id;
}

OguiSelectList::OguiSelectList(int x, int y, int defsel, bool multisel, int amount, const char **values, const char **descs, OguiButton **listButs, OguiButton *upBut, OguiButton *downBut, OguiSelectListStyle *style, int id, void *argument)
{ 
	this->init(x, y, defsel, multisel, amount, values, descs, listButs, upBut, downBut, style, id, argument);
}

OguiSelectList::~OguiSelectList()
{ 
	this->uninit();
}
