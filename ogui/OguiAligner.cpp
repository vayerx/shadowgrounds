#include "precompiled.h"

#include "Ogui.h"
#include "orvgui2.h"
#include "OguiAligner.h"

#include "../game/options/options_gui.h"

void OguiAligner::align(int &x, int &y, int &w, int &h, int flags, Ogui *ogui)
{
	if(!game::SimpleOptions::getBool(DH_OPT_B_GUI_WIDESCREEN_FIXES))
		return;

	if(flags & WIDESCREEN_FIX_RIGHT)
	{
		x = 1024 - ((1024 - x) * ogui->GetScaleY()) / ogui->GetScaleX();
		w = (w * ogui->GetScaleY()) / ogui->GetScaleX();
	}
	else if(flags & WIDESCREEN_FIX_LEFT)
	{
		x = (x * ogui->GetScaleY()) / ogui->GetScaleX();
		w = (w * ogui->GetScaleY()) / ogui->GetScaleX();
	}
	else if(flags & WIDESCREEN_FIX_CENTER)
	{
		x = 512 - ((512 - x) * ogui->GetScaleY()) / ogui->GetScaleX();
		w = (w * ogui->GetScaleY()) / ogui->GetScaleX();
	}
}

void OguiAligner::align(OguiWindow *win, int flags, Ogui *ogui)
{
	if(!win)
		return;

	int x,y,w,h;
	orvgui_win *owin = (orvgui_win *)win->win;
	x = owin->put_x;
	y = owin->put_y;
	w = owin->sizex;
	h = owin->sizey;

	align(x, y, w, h, flags, ogui);

	// offset children
	if (owin->first_child != NULL)
	{
		int deltax = (owin->put_x - x);
		int deltay = (owin->put_y - y);
		orvgui_but *but = (orvgui_but *)(owin->first_child);
		while(but != NULL)
		{
			but->put_x += deltax;
			but->put_y += deltay;
			but = (orvgui_but *)(but->next_sister);
		}		
	}
	owin->sizex = w;
	owin->sizey = h;
	owin->put_x = x;
	owin->put_y = y;
}

void OguiAligner::align(OguiButton *button, int flags, Ogui *ogui)
{
	if(!button)
		return;

	int x,y,w,h;
	orvgui_but *obut = (orvgui_but *)button->but;
	x = obut->put_x;
	y = obut->put_y;
	w = obut->sizex;
	h = obut->sizey;
	if(obut->parent)
	{
		x += obut->parent->put_x;
		y += obut->parent->put_y;
	}

	align(x, y, w, h, flags, ogui);

	if(obut->parent)
	{
		x -= obut->parent->put_x;
		y -= obut->parent->put_y;
	}
	obut->sizex = w;
	obut->sizey = h;
	obut->put_x = x;
	obut->put_y = y;
}

void OguiAligner::align(OguiTextLabel *label, int flags, Ogui *ogui)
{
	if(!label)
		return;

	align(label->GetButton(), flags, ogui);
}
