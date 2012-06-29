#include "precompiled.h"

#include <stdlib.h>
#include "orvgui2.h"
#include "OguiButton.h"
#include "OguiWindow.h"
#include "Ogui.h"

#include "../util/fb_assert.h"

// bad dependency
#include "OguiStormDriver.h"
#include "../util/UnicodeConverter.h"
#include "../util/Debug_MemoryManager.h"


OguiButton::OguiButton(Ogui *ogui, int id, const void *argument) :
	selected( false ),
	imageSelected( NULL ),
	imageSelectedHigh( NULL )
{
	this->ogui = ogui;
	this->id = id;
	this->argument = argument;
	this->listener = NULL;
	this->but = NULL;
	this->parent = NULL;
	this->eventMask = OGUI_EMASK_CLICK;

	this->image = NULL;
	this->imageDown = NULL;
	this->imageDisabled = NULL;
	this->imageHighlighted = NULL;
	this->imageAutodel = false;
	this->imageDownAutodel = false;
	this->imageDisabledAutodel = false;
	this->imageHighlightedAutodel = false;
	this->rotation = 0;
	this->font = NULL;
	this->fontDisabled = NULL;
	this->fontDown = NULL;
	this->fontHighlighted = NULL;
}

OguiButton::~OguiButton()
{
	if (but == NULL)
	{
		abort(); // we've got a bug
	}

	// to make sure, that the imageAutodel and images are correct
	SetSelected( false );

	parent->buttonDeleted(this);
	og_delete_button(but);
	
	if (imageAutodel && image != NULL) 
		delete image;

	if (imageDownAutodel && imageDown != NULL) 
		delete imageDown;

	if (imageDisabledAutodel && imageDisabled != NULL) 
		delete imageDisabled;
	
	if (imageHighlightedAutodel && imageHighlighted != NULL) 
		delete imageHighlighted;
	
	delete imageSelected;
	imageSelected = NULL;

	delete imageSelectedHigh;
	imageSelectedHigh = NULL;

	ogui->RemovedButton(this);
}

void OguiButton::SetImage(IOguiImage *image)
{
	if (imageAutodel && this->image != NULL) delete this->image;
	this->image = image;
	this->imageAutodel = false;
	ApplyImages();
}

void OguiButton::SetDownImage(IOguiImage *imageDown)
{
	if (imageDownAutodel && this->imageDown != NULL) delete this->imageDown;
	this->imageDown = imageDown;
	this->imageDownAutodel = false;
	ApplyImages();
}

void OguiButton::SetDisabledImage(IOguiImage *imageDisabled)
{
	if (imageDisabledAutodel && this->imageDisabled != NULL) 
		delete this->imageDisabled;
	this->imageDisabled = imageDisabled;
	this->imageDisabledAutodel = false;
	ApplyImages();
}

void OguiButton::SetHighlightedImage(IOguiImage *imageHighlighted)
{
	if (imageHighlightedAutodel && this->imageHighlighted != NULL) 
		delete this->imageHighlighted;
	this->imageHighlighted = imageHighlighted;
	this->imageHighlightedAutodel = false;
	ApplyImages();
}

void OguiButton::GetImages(IOguiImage **image, IOguiImage **down, IOguiImage **disabled, IOguiImage **high)
{
	if(image) *image = this->image;
	if(down) *down = this->imageDown;
	if(disabled) *disabled = this->imageDisabled;
	if(high) *high = this->imageHighlighted;
}

void OguiButton::GetImageAutoDelete(bool *image, bool *down, bool *disabled, bool *high)
{
	if(image) *image = this->imageAutodel;
	if(down) *down = this->imageDownAutodel;
	if(disabled) *disabled = this->imageDisabledAutodel;
	if(high) *high = this->imageHighlightedAutodel;
}

void OguiButton::SetImageAutoDelete(bool image, bool down, bool disabled, bool high)
{
	this->imageAutodel = image;
	this->imageDownAutodel = down;
	this->imageDisabledAutodel = disabled;
	this->imageHighlightedAutodel = high;
}

void OguiButton::SetFont(IOguiFont *font)
{
	this->font = font;
	ApplyFonts();
}

void OguiButton::SetDisabledFont( IOguiFont* font )
{
	fontDisabled = font;
	ApplyFonts();
}

void OguiButton::SetDownFont( IOguiFont* font )
{
	fontDown = font;
	ApplyFonts();
}

void OguiButton::SetHighlightedFont( IOguiFont* font )
{
	fontHighlighted = font;
	ApplyFonts();
}

void OguiButton::Resize(int sizeX, int sizeY)
{
    assert(but != NULL);
	og_resize_button(but, (short)sizeX, (short)sizeY);
}

void OguiButton::SetClipToWindow( bool clip )
{
    assert(but != NULL);
	but->clip_to_window = clip;
}

void OguiButton::Move(int x, int y)
{
    assert(but != NULL);
	og_move_button(but, (short)x, (short)y);
}

void OguiButton::MoveBy( int x, int y )
{
    assert(but != NULL);
	og_move_button_by( but, (short)x, (short) y );
}

void OguiButton::SetStyle(OguiButtonStyle *style)
{
	if (style == NULL)
	{
		fb_assert(!"OguiButton::SetStyle - null style parameter given.");
		return;
	}
	Resize(style->sizeX, style->sizeY);
	SetImage(style->image);
	SetDownImage(style->imageDown);
	SetDisabledImage(style->imageDisabled);
	SetHighlightedImage(style->imageHighlighted);
	SetFont(style->textFont);
	SetDownFont(style->textFontDown);
	SetHighlightedFont(style->textFontHighlighted);
	SetDisabledFont(style->textFontDisabled);
}

void OguiButton::SetListener(IOguiButtonListener *listener)
{
	this->listener = listener;
}
 
void OguiButton::SetEventMask(int allowedEvents)
{
	this->eventMask = allowedEvents;
}

void OguiButton::SetReactMask(int reactButtons)
{
    assert(but != NULL);
	og_set_react_button(but, (unsigned int)reactButtons);
}

void OguiButton::SetDisabled(bool disabled)
{
	if (but == NULL) abort(); // we've got a bug
	if (disabled)
		og_disable_button(but);
	else
		og_enable_button(but);
}

bool OguiButton::isDisabled()
{
    assert(but != NULL);
	return but->enabled == 0;
}

void OguiButton::SetLineBreaks(bool linebreaks)
{
	// TODO: check if this is a text button, else abort
	if (linebreaks)
		og_set_linebreaks_button(but);
	else
		og_set_nolinebreaks_button(but);
}

bool OguiButton::IsLineBreaks()
{
    assert(but != NULL);
	return (but->linebreaks != 0);
}


bool OguiButton::SetText(const char *text)
{
	// HACK: optimization, if text is the same as it was before,
	// do nothing...
	assert(but != NULL);
	if (but->text != NULL
		&& text != NULL
		&& strcmp(but->text, text) == 0)
		return false;
	
	if( ( text != NULL && !std::string( text ).empty() ) &&
		( this->text.empty() ||	this->text.size() < strlen( text ) || 
		this->text.substr( 0, strlen( text ) ) != text ) )
	{
		this->text = text;
	}
#ifdef PROJECT_SHADOWGROUNDS
	IOguiFont* temp_font = font;
#else
	IOguiFont* temp_font = GetTheFontCurrentlyInUse();
#endif
	// TODO: check if this is a text button, else abort
	int pixwidth = 0;
	int pixheight = 0;
	int fontwidth = 0;
	int fontheight = 0;
	if (temp_font != NULL && text != NULL)
	{
		pixwidth = temp_font->getStringWidth(text);
		pixheight = temp_font->getStringHeight(text);
		fontwidth = temp_font->getWidth();
		fontheight = temp_font->getHeight();

	}

	// note, const char * -> char * cast.
	// however, the og_set_text_button should not change that.
	assert(but != NULL);
	og_set_text_button(but, text, pixwidth, pixheight,
		fontwidth, fontheight);

	return true;
}

void OguiButton::SetAngle(float angle)
{
    assert(but != NULL);
	og_rotate_button(but, angle);
}

// 0 for no transparency, 100 for full transparency
void OguiButton::SetTransparency(int transparencyPercentage)
{
    assert(but != NULL);
	og_set_transparency_button(but, transparencyPercentage);
}


void OguiButton::SetTextHAlign(OguiButton::TEXT_H_ALIGN hAlign)
{
    assert(but != NULL);
	if (hAlign == OguiButton::TEXT_H_ALIGN_LEFT)
		og_set_h_align_button(but, OG_H_ALIGN_LEFT);
	else
		if (hAlign == OguiButton::TEXT_H_ALIGN_CENTER)
			og_set_h_align_button(but, OG_H_ALIGN_CENTER);
		else
			og_set_h_align_button(but, OG_H_ALIGN_RIGHT);
}

void OguiButton::SetTextVAlign(OguiButton::TEXT_V_ALIGN vAlign)
{
    assert(but != NULL);
	if (vAlign == OguiButton::TEXT_V_ALIGN_TOP)
		og_set_v_align_button(but, OG_V_ALIGN_TOP);
	else
		if (vAlign == OguiButton::TEXT_V_ALIGN_MIDDLE)
			og_set_v_align_button(but, OG_V_ALIGN_MIDDLE);
		else
			og_set_v_align_button(but, OG_V_ALIGN_BOTTOM);
}

void OguiButton::Focus(int withCursor)
{
	og_warp_cursor_to(withCursor, but);
}

int OguiButton::GetId()
{
	return id;
}

void OguiButton::SetId(int id_)
{
	id = id_;
}

const void *OguiButton::GetArgument()
{
	return argument;
}

void OguiButton::ApplyImages()
{
	IStorm3D_Material *mat = NULL;
	IStorm3D_Material *matdown = NULL;
	IStorm3D_Material *matdisabled = NULL;
	IStorm3D_Material *mathighlighted = NULL;
	if (image != NULL) mat = ((OguiStormImage *)image)->mat;
	if (imageDown != NULL) matdown = ((OguiStormImage *)imageDown)->mat;
	if (imageDisabled != NULL) matdisabled = ((OguiStormImage *)imageDisabled)->mat;
	if (imageHighlighted != NULL) mathighlighted = ((OguiStormImage *)imageHighlighted)->mat;
    assert(but != NULL);
	og_set_pic_button(but, mat, matdown, matdisabled, mathighlighted);
}

void OguiButton::ApplyFonts()
{
	IStorm3D_Font* fnt = NULL;
	IStorm3D_Font* fnt_down = NULL;
	IStorm3D_Font* fnt_disabled = NULL;
	IStorm3D_Font* fnt_highlighted = NULL;
	COL fnt_color;
	COL fnt_down_color;
	COL fnt_disabled_color;
	COL fnt_highlighted_color;

	if (font != NULL)
	{
		fnt = ((OguiStormFont *)font)->fnt;
		fnt_color = ((OguiStormFont *)font)->color;
	}
	if (fontDown!= NULL)
	{
		fnt_down = ((OguiStormFont *)fontDown)->fnt;
		fnt_down_color = ((OguiStormFont *)fontDown)->color;
	}
	if (fontDisabled  != NULL)
	{
		fnt_disabled = ((OguiStormFont *)fontDisabled)->fnt;
		fnt_disabled_color = ((OguiStormFont *)fontDisabled)->color;
	}
	if (fontHighlighted != NULL) 
	{
		fnt_highlighted = ((OguiStormFont *)fontHighlighted)->fnt;
		fnt_highlighted_color = ((OguiStormFont *)fontHighlighted)->color;
	}

	assert(but != NULL);
	og_set_fonts_button( but, fnt, fnt_color, fnt_down, fnt_down_color, fnt_disabled, fnt_disabled_color, fnt_highlighted, fnt_highlighted_color );

	// need to update font metrics (set the text again)
	// a hack to do that 
	// need to copy the original text first, as the method call will 
	// delete the original text
    assert(but != NULL);
	if (but->text != NULL)
	{
		char *buf = new char[strlen(but->text) + 1];
		strcpy(buf, but->text);
		SetText(NULL);
		SetText(buf);
		delete [] buf;
	}

}

void OguiButton::ResetData()
{
	ApplyImages();
	ApplyFonts();
	// SetFont(this->font);
}

IOguiFont *OguiButton::GetFont()
{
	return this->font;
}

int OguiButton::GetSizeX()
{
    assert(but != NULL);
	return but->sizex;
}

int OguiButton::GetSizeY()
{
    assert(but != NULL);
	return but->sizey;
}

int OguiButton::GetX()
{
    assert(but != NULL);
	return but->put_x;
}

int OguiButton::GetY()
{
    assert(but != NULL);
	return but->put_y;
}

void OguiButton::SetClip(float leftX, float topY, float rightX, float bottomY)
{
    assert(but != NULL);
	but->clipleftx = leftX;
	but->cliptopy = topY;
	but->cliprightx = rightX;
	but->clipbottomy = bottomY;
}

void OguiButton::GetClip(float &leftX, float &topY, float &rightX, float &bottomY)
{
    assert(but != NULL);
	leftX = but->clipleftx;
	topY = but->cliptopy;
	rightX = but->cliprightx;
	bottomY = but->clipbottomy;
}

void OguiButton::SetScroll(float scrollX, float scrollY)
{
    assert(but != NULL);
	but->scroll_x = scrollX;
	but->scroll_y = scrollY;
	but->wrap = true;
}

void OguiButton::SetRepeat(float repeatX, float repeatY)
{
    assert(but != NULL);
	but->repeat_x = repeatX;
	but->repeat_y = repeatY;
	but->wrap = true;
}

void OguiButton::SetHotKey(int hotkey, int hotkeyModifier1, int hotkeyModifier2)
{
    assert(but != NULL);
	but->hotKeys[0] = hotkey;
	but->hotKeys[1] = hotkeyModifier1;
	but->hotKeys[2] = hotkeyModifier2;
}

void OguiButton::SetSelected( bool s )
{
	if( selected != s )
	{
		std::swap( image, imageSelected );
		std::swap( imageHighlighted, imageSelectedHigh );
		ApplyImages();

		selected = s;
	}
}

bool OguiButton::IsSelected()
{
	return selected;
}

void OguiButton::DeleteSelectedImages()
{
	if(imageSelected != NULL)
		delete imageSelected;

	imageSelected = NULL;

	if(imageSelectedHigh != NULL)
		delete imageSelectedHigh;

	imageSelectedHigh = NULL;
}
void OguiButton::SetSelectedImages( IOguiImage* selected_norm, IOguiImage* selected_high )
{
	imageSelected = selected_norm;
	imageSelectedHigh = selected_high;
}

IOguiFont* OguiButton::GetTheFontCurrentlyInUse()
{
	orvgui_but* orv_button = but;
	IOguiFont* result = font;
	
	if( orv_button->enabled == 0 )
	{
		if( fontDisabled != NULL )
			result = fontDisabled;
	} 
	else 
	{
		if( orv_button->pressed == 0 )
		{
			if( orv_button->highlighted != 0 )
			{
				if( fontHighlighted != NULL )
					result = fontHighlighted;
			}
		} 
		else
		{
			if( fontDown != NULL )
				result = fontDown;
		} 
	}

	return result;
}

void OguiButton::PressButton(bool press)
{
	orvgui_but* orv_button = but;
	if(!press) og_unpress_button( orv_button );
	else og_press_button( orv_button );
}

void OguiButton::SetCustomShape(Vertex *vertices, int numVertices)
{
	orvgui_but* orv_button = but;

	if(numVertices == 0 || vertices == NULL)
	{
		delete [] orv_button->vertices;
		orv_button->vertices = NULL;
		orv_button->num_vertices = 0;
		return;
	}

	if(orv_button->vertices != NULL)
	{
		delete [] orv_button->vertices;
	}
	orv_button->num_vertices = numVertices;
	orv_button->vertices = new orvgui_but::vertex[numVertices];
	// note: assuming orvgui_but::vertex is same as OguiButton::Vertex
	memcpy(orv_button->vertices, vertices, numVertices * sizeof(Vertex));
}

void OguiButton::SetWrap(bool wrap)
{
    assert(but != NULL);
	but->wrap = wrap;
}
