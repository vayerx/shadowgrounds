#include "precompiled.h"

#include "../system/Logger.h"

#include "OguiButtonEvent.h"
#include "OguiWindow.h"
#include "orvgui2.h"

#include "Ogui.h"

// TEMP
#include "../convert/str2int.h"

// bad dependency...
#include "OguiStormDriver.h"
#include "../util/Debug_MemoryManager.h"

#ifdef _MSC_VER
#pragma warning(disable : 4244) 
#endif

// orvgui callback handling and conversion to ogui button listener events

void ogui_button_event_handler(OguiButtonEvent::EVENT_TYPE et)
{
	OguiButton *b = (OguiButton *)og_arg;

	// HACK: If wheel rolled, interpret it as CLICK instead of PRESS or HOLD. (Otherwise we have to move mouse until the message goes through.)
/*	if( (et == OguiButtonEvent::EVENT_TYPE_PRESS || et == OguiButtonEvent ::EVENT_TYPE_HOLD) && 
		 ( og_cursor_obut & ( OG_BUT_WHEEL_UP_MASK | OG_BUT_WHEEL_DOWN_MASK ) ) )
	{
		et = OguiButtonEvent::EVENT_TYPE_CLICK;
	}*/

	if (et & b->eventMask && b->listener != NULL)
	{
		OguiButtonEvent *eve = new OguiButtonEvent(et, og_cursor_num,
			og_cursor_scrx, og_cursor_scry, og_cursor_x, og_cursor_y,
			og_cursor_but, og_cursor_obut, 
			b, b->parent, b->argument);


		(b->listener)->CursorEvent(eve);
		delete eve;
	} else {
		if (b->listener == NULL)
		{
			(Logger::getInstance())->debug("ogui_button_event_handler - Button has no listener.");
		}
	}
}

static void ogui_button_click_handler(void)
{
	ogui_button_event_handler(OguiButtonEvent::EVENT_TYPE_CLICK);
}

static void ogui_button_press_handler(void)
{
	ogui_button_event_handler(OguiButtonEvent::EVENT_TYPE_PRESS);
}

static void ogui_button_out_handler(void)
{
	ogui_button_event_handler(OguiButtonEvent::EVENT_TYPE_OUT);
}

static void ogui_button_over_handler(void)
{
	ogui_button_event_handler(OguiButtonEvent::EVENT_TYPE_OVER);
}

static void ogui_button_leave_handler(void)
{
	ogui_button_event_handler(OguiButtonEvent::EVENT_TYPE_LEAVE);
}

static void ogui_button_hold_handler(void)
{
	ogui_button_event_handler(OguiButtonEvent::EVENT_TYPE_HOLD);
}


OguiWindow::OguiWindow(Ogui *ogui, int x, int y, int sizex, int sizey, IOguiImage *img, int id)
{
	this->ogui = ogui;
	this->release_bg_image = true;
	this->is_only_active = false;
	this->is_visible = true;
	this->id = id;
	this->image = img;
	
	IStorm3D_Material *mat = NULL;
	if (img != NULL)
		mat = ((OguiStormImage *)img)->mat;

	this->win = og_create_window(OG_WIN_SIMPLE, x, y, sizex, sizey, mat);
	//og_hide_window((orvgui_win *)win);
	og_set_movable_window((orvgui_win *)win, OG_WIN_MOVABLE);

	// NOTE: cannot use x,y directly, as screen boundaries may affect
	// actual position
	this->windowPositionX = ((orvgui_win *)win)->put_x;
	this->windowPositionY = ((orvgui_win *)win)->put_y;

	this->effectListener = NULL;

	this->effectFadeTimeLeft = 0;
	this->effectFadeTimeTotal = 0;
	this->effectMoveTimeLeft = 0;
	this->effectMoveTimeTotal = 0;
	this->effectTextTypeTimeLeft = 0;
	this->effectTextTypeTimeTotal = 0;
	this->effectTextLineTimeLeft = 0;
	this->effectTextLineTimeTotal = 0;

	this->fadingIn = false;
	this->fadingOut = false;
	this->movingIn = false;
	this->movingOut = false;

	buttonList = new LinkedList();
}

OguiWindow::~OguiWindow()
{
	while (!buttonList->isEmpty())
	{
		OguiButton *b = (OguiButton *)buttonList->popLast();
		delete b;
	}
	delete buttonList;

	if (is_only_active)
	{
		og_set_only_active(NULL);
		is_only_active = false;
	}
	og_delete_window((orvgui_win *)win); 
	if ( release_bg_image ) 
		delete image;
	image = NULL;

	ogui->RemovedWindow(this);
}

void OguiWindow::buttonDeleted(OguiButton *b)
{
	buttonList->remove(b);
}

void OguiWindow::Raise()
{
	og_raise_window((orvgui_win *)win);
}

void OguiWindow::Lower()
{
	og_lower_window((orvgui_win *)win);
}

void OguiWindow::Show()
{
	is_visible = true;
	og_show_window((orvgui_win *)win);
}

void OguiWindow::Hide()
{
	is_visible = false;
	og_hide_window((orvgui_win *)win);
}

bool OguiWindow::IsVisible() const
{
	// orvgui popups may change visibility status without telling us...
	if (((orvgui_win *)win)->visible != 0)
		is_visible = true;
	else
		is_visible = false;
	return is_visible;
}

void OguiWindow::SetPopup()
{
	og_set_popup_window((orvgui_win *)win, OG_WIN_POPUP);  
}

void OguiWindow::SetPopupNoClose()
{
	og_set_popup_window((orvgui_win *)win, OG_WIN_POPUPNOCLOSE);	
}

void OguiWindow::SetPopupNoCloseOnButton()
{
	og_set_popup_window((orvgui_win *)win, OG_WIN_POPUPNOCLOSEONBUTTON);
}

void OguiWindow::SetOnlyActive()
{
	og_set_only_active((orvgui_win *)win);
	is_only_active = true;
}

void OguiWindow::RestoreAllActive()
{
	if (is_only_active)
	{
		og_set_only_active(NULL);
		is_only_active = false;
	}
}

void OguiWindow::SetReactMask(int reactCursors)
{
	og_set_react_window((orvgui_win *)win, reactCursors);
}

void OguiWindow::MoveTo(int x, int y)
{
	og_move_window((orvgui_win *)win, x, y);

	// NOTE: cannot use x,y directly, as screen boundaries may affect
	// actual position
	this->windowPositionX = ((orvgui_win *)win)->put_x;
	this->windowPositionY = ((orvgui_win *)win)->put_y;
}

int OguiWindow::GetPositionX()
{
	return ((orvgui_win *)win)->put_x;
}

int OguiWindow::GetPositionY()
{
	return ((orvgui_win *)win)->put_y;
}

int OguiWindow::GetSizeX()
{
	return ((orvgui_win *)win)->sizex;
}

int OguiWindow::GetSizeY()
{
	return ((orvgui_win *)win)->sizey;
}

void OguiWindow::Resize(int x, int y)
{
	og_resize_window((orvgui_win *)win, x, y);
}

void OguiWindow::SetMovable()
{
	og_set_movable_window((orvgui_win *)win, OG_WIN_MOVABLE);
}

void OguiWindow::SetUnmovable()
{
	og_set_movable_window((orvgui_win *)win, OG_WIN_UNMOVABLE);
}

OguiButton *OguiWindow::CreateNewButton(int x, int y, int sizex, int sizey, 
	IOguiImage *img, IOguiImage *imgdown, IOguiImage *imghigh, IOguiImage *imgdisabled, bool withText, 
	const char *text, int id, const void *argument, IOguiFont *font, bool clipToWindow )
{
	orvgui_but *but;

	// NOTICE: the orvgui button is created here, but deleted in 
	// the OguiButton class destructor

	if (withText)
	{
		IStorm3D_Font *fnt = NULL;
		COL fnt_color;
		if (font != NULL)
		{
			fnt = ((OguiStormFont *)font)->fnt;
			fnt_color = ((OguiStormFont *)font)->color;
		}

		int pixwidth = 0;
		int pixheight = 0;
		int fontwidth = 0;
		int fontheight = 0;
		if (font != NULL && text != NULL)
		{
			pixwidth = font->getStringWidth(text);
			pixheight = font->getStringHeight(text);
			fontwidth = font->getWidth();
			fontheight = font->getHeight();
		}

		but = og_create_button((orvgui_win *)win, OG_BUT_PIC_AND_TEXT, x, y, sizex, sizey, clipToWindow);
		og_set_text_button(but, fnt, fnt_color, OG_H_ALIGN_CENTER, OG_V_ALIGN_MIDDLE, text, pixwidth, pixheight, fontwidth, fontheight);
	} else {
		but = og_create_button((orvgui_win *)win, OG_BUT_PIC, x, y, sizex, sizey, clipToWindow);
	}

	IStorm3D_Material *mat = NULL;
	IStorm3D_Material *matdown = NULL;
	IStorm3D_Material *mathigh = NULL;
	IStorm3D_Material *matdisabled = NULL;
	if (img != NULL) mat = ((OguiStormImage *)img)->mat;
	if (imgdown != NULL) matdown = ((OguiStormImage *)imgdown)->mat;
	if (imghigh != NULL) mathigh = ((OguiStormImage *)imghigh)->mat;
	if (imgdisabled != NULL) matdisabled = ((OguiStormImage *)imgdisabled)->mat;
	og_set_pic_button(but, mat, matdown, matdisabled, mathigh);

	OguiButton *ob = new OguiButton(ogui, id, argument);
	ob->but = but;
	ob->parent = this;
	ob->image = img;
	ob->imageDown = imgdown;
	ob->imageDisabled = NULL;
	ob->imageHighlighted = imghigh;
	ob->imageAutodel = false;
	ob->imageDownAutodel = false;
	ob->imageDisabledAutodel = false;
	ob->imageHighlightedAutodel = false;
	ob->font = font;

	og_set_click_button(but, ogui_button_click_handler, ob);
	og_set_press_button(but, ogui_button_press_handler, ob);
	og_set_out_button(but, ogui_button_out_handler, ob);
	og_set_over_button(but, ogui_button_over_handler, ob);
	og_set_leave_button(but, ogui_button_leave_handler, ob);
	og_set_hold_button(but, ogui_button_hold_handler, ob);

	buttonList->append(ob);

	return ob;
}

void OguiWindow::StartEffect(int windowEffect, int effectDuration) 
{ 
	if (windowEffect == OGUI_WINDOW_EFFECT_FADEOUT
		|| windowEffect == OGUI_WINDOW_EFFECT_FADEIN)
	{
		if (windowEffect == OGUI_WINDOW_EFFECT_FADEOUT)
		{
			this->fadingOut = true;
			this->fadingIn = false;
		} else {
			this->fadingIn = true;
			this->fadingOut = false;
		}
		this->effectFadeTimeLeft = effectDuration;
		this->effectFadeTimeTotal = effectDuration;
	}
	if (windowEffect == OGUI_WINDOW_EFFECT_MOVEOUT
		|| windowEffect == OGUI_WINDOW_EFFECT_MOVEIN)
	{
		if (windowEffect == OGUI_WINDOW_EFFECT_MOVEOUT)
		{
			this->movingOut = true;
			this->movingIn = false;
		} else {
			this->movingIn = true;
			this->movingOut = false;
		}
		if (this->effectMoveTimeLeft == 0)
		{
			this->windowPositionX = ((orvgui_win *)win)->put_x;
			this->windowPositionY = ((orvgui_win *)win)->put_y;
		}
		this->effectMoveTimeLeft = effectDuration;
		this->effectMoveTimeTotal = effectDuration;
	}
	if( windowEffect == OGUI_WINDOW_EFFECT_TEXTTYPE )
	{
		this->effectTextTypeTimeLeft = effectDuration;
		this->effectTextTypeTimeTotal = effectDuration;
	}
	if( windowEffect == OGUI_WINDOW_EFFECT_TEXTLINE )
	{
		this->effectTextLineTimeLeft = effectDuration;
		this->effectTextLineTimeTotal = effectDuration;
	}
}

void OguiWindow::EndAllEffects() 
{
	this->effectFadeTimeLeft = 0;
	this->effectFadeTimeTotal = 0;
	this->effectMoveTimeLeft = 0;
	this->effectMoveTimeTotal = 0;
	this->effectTextTypeTimeLeft = 0;
	this->effectTextTypeTimeTotal = 0;
	this->movingIn = false;
	this->movingOut = false;
	this->fadingIn = false;
	this->fadingOut = false;
	og_set_transparency_window(((orvgui_win *)win), 0);
	og_move_window(((orvgui_win *)win), this->windowPositionX, this->windowPositionY);
}


int OguiWindow::GetId() 
{ 
	return id; 
}

void OguiWindow::ResetData() 
{ 
	if (image != NULL)
	{
		IStorm3D_Material *mat = ((OguiStormImage *)image)->mat;
		og_set_background_window((orvgui_win *)win, mat);
	}
}

void OguiWindow::SetTransparency(int transparencyPercentage)
{
	og_set_transparency_window((orvgui_win *)win, transparencyPercentage);
}


void OguiWindow::SetEffectListener(IOguiEffectListener *listener)
{
	this->effectListener = listener;
}


void OguiWindow::setBackgroundScroll(float scrollX, float scrollY)
{
	((orvgui_win *)win)->scroll_x = scrollX;
	((orvgui_win *)win)->scroll_y = scrollY;
	((orvgui_win *)win)->wrap = true;
}

void OguiWindow::setBackgroundRepeatFactor(float x, float y)
{
	((orvgui_win *)win)->bg_repeat_factor_x = x;
	((orvgui_win *)win)->bg_repeat_factor_y = y;
	((orvgui_win *)win)->wrap = true;
}

void OguiWindow::setBackgroundRepeatAuto()
{
	orvgui_win *owin = (orvgui_win *)win;
	if(this->image && this->image->getTexture())
	{
		Storm3D_SurfaceInfo si = this->image->getTexture()->GetSurfaceInfo();
		owin->bg_repeat_factor_x = (owin->sizex / (float)si.width);
		owin->bg_repeat_factor_y = (owin->sizey / (float)si.height);
	}
	((orvgui_win *)win)->wrap = true;
}

void OguiWindow::SetMoveBoundaryType(MOVE_BOUND btype)
{
	((orvgui_win *)win)->move_bound = btype;
}

void OguiWindow::setBackgroundImage( IOguiImage* image )
{
	if( this->release_bg_image )
	{
		delete this->image;
	}
	this->image = image;
	release_bg_image = false;

	IStorm3D_Material *mat = NULL;
	if ( image != NULL)
		mat = ((OguiStormImage *)image)->mat;

	((orvgui_win*)win)->bg_pic = mat;
}

IOguiImage *OguiWindow::getBackgroundImage() const
{
	return this->image;
}
