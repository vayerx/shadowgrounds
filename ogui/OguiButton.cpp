#include "precompiled.h"

#include <stdlib.h>
#include "orvgui2.h"
#include "OguiButton.h"
#include "OguiWindow.h"
#include "Ogui.h"

#include <util/fb_assert.h>
#include <igios.h>

// bad dependency
#include "OguiStormDriver.h"
#include "../util/UnicodeConverter.h"
#include "../util/Debug_MemoryManager.h"

OguiButton::OguiButton(Ogui *a_ogui, orvgui_but *button, int a_id, const void *a_argument) :
    pImpl(button),
    ogui(a_ogui),
    id(a_id),
    eventMask(OGUI_EMASK_CLICK),
    argument(a_argument),
    listener(NULL),
    owner(NULL),
    image(NULL),
    imageDown(NULL),
    imageDisabled(NULL),
    imageHighlighted(NULL),
    font(NULL),
    fontDown(NULL),
    fontDisabled(NULL),
    fontHighlighted(NULL),
    imageAutodel(false),
    imageDownAutodel(false),
    imageDisabledAutodel(false),
    imageHighlightedAutodel(false),
    rotation(0),
    selected(false),
    imageSelected(NULL),
    imageSelectedHigh(NULL)
{
    if (pImpl == NULL) {
        igios_backtrace();
        abort();  // we've got a bug
    }
}

OguiButton::~OguiButton()
{
    // to make sure, that the imageAutodel and images are correct
    SetSelected(false);

    owner->buttonDeleted(this);
    og_delete_button(pImpl);

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
    if (image) *image = this->image;
    if (down) *down = this->imageDown;
    if (disabled) *disabled = this->imageDisabled;
    if (high) *high = this->imageHighlighted;
}

void OguiButton::GetImageAutoDelete(bool *image, bool *down, bool *disabled, bool *high)
{
    if (image) *image = this->imageAutodel;
    if (down) *down = this->imageDownAutodel;
    if (disabled) *disabled = this->imageDisabledAutodel;
    if (high) *high = this->imageHighlightedAutodel;
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

void OguiButton::SetDisabledFont(IOguiFont *font)
{
    fontDisabled = font;
    ApplyFonts();
}

void OguiButton::SetDownFont(IOguiFont *font)
{
    fontDown = font;
    ApplyFonts();
}

void OguiButton::SetHighlightedFont(IOguiFont *font)
{
    fontHighlighted = font;
    ApplyFonts();
}

void OguiButton::Resize(int sizeX, int sizeY)
{
    og_resize_button(pImpl, (short)sizeX, (short)sizeY);
}

void OguiButton::SetClipToWindow(bool clip)
{
    pImpl->clip_to_window = clip;
}

void OguiButton::Move(int x, int y)
{
    og_move_button(pImpl, (short)x, (short)y);
}

void OguiButton::MoveBy(int x, int y)
{
    og_move_button_by(pImpl, (short)x, (short) y);
}

void OguiButton::SetStyle(OguiButtonStyle *style)
{
    if (style == NULL) {
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
    og_set_react_button(pImpl, (unsigned int)reactButtons);
}

void OguiButton::SetDisabled(bool disabled)
{
    if (disabled)
        og_disable_button(pImpl);
    else
        og_enable_button(pImpl);
}

bool OguiButton::isDisabled()
{
    return pImpl->enabled == 0;
}

void OguiButton::SetLineBreaks(bool linebreaks)
{
    // TODO: check if this is a text button, else abort
    if (linebreaks)
        og_set_linebreaks_button(pImpl);
    else
        og_set_nolinebreaks_button(pImpl);
}

bool OguiButton::IsLineBreaks()
{
    return (pImpl->linebreaks != 0);
}

bool OguiButton::SetText(const char *text)
{
    // HACK: optimization, if text is the same as it was before,
    // do nothing...
    if (pImpl->text != NULL
        && text != NULL
        && strcmp(pImpl->text, text) == 0)
        return false;

    if ( ( text != NULL && !std::string(text).empty() ) &&
         (this->text.empty() || this->text.size() < strlen(text) ||
          this->text.substr( 0, strlen(text) ) != text) )
        this->text = text;

#ifdef PROJECT_SHADOWGROUNDS
    IOguiFont *temp_font = font;
#else
    IOguiFont *temp_font = GetTheFontCurrentlyInUse();
#endif
    // TODO: check if this is a text button, else abort
    int pixwidth = 0;
    int pixheight = 0;
    int fontwidth = 0;
    int fontheight = 0;
    if (temp_font != NULL && text != NULL) {
        pixwidth = temp_font->getStringWidth(text);
        pixheight = temp_font->getStringHeight(text);
        fontwidth = temp_font->getWidth();
        fontheight = temp_font->getHeight();

    }

    // note, const char * -> char * cast.
    // however, the og_set_text_button should not change that.
    og_set_text_button(pImpl, text, pixwidth, pixheight,
                       fontwidth, fontheight);

    return true;
}

void OguiButton::SetAngle(float angle)
{
    og_rotate_button(pImpl, angle);
}

// 0 for no transparency, 100 for full transparency
void OguiButton::SetTransparency(int transparencyPercentage)
{
    og_set_transparency_button(pImpl, transparencyPercentage);
}

void OguiButton::SetTextHAlign(OguiButton::TEXT_H_ALIGN hAlign)
{
    if (hAlign == OguiButton::TEXT_H_ALIGN_LEFT)
        og_set_h_align_button(pImpl, OG_H_ALIGN_LEFT);
    else if (hAlign == OguiButton::TEXT_H_ALIGN_CENTER)
        og_set_h_align_button(pImpl, OG_H_ALIGN_CENTER);
    else
        og_set_h_align_button(pImpl, OG_H_ALIGN_RIGHT);
}

void OguiButton::SetTextVAlign(OguiButton::TEXT_V_ALIGN vAlign)
{
    if (vAlign == OguiButton::TEXT_V_ALIGN_TOP)
        og_set_v_align_button(pImpl, OG_V_ALIGN_TOP);
    else if (vAlign == OguiButton::TEXT_V_ALIGN_MIDDLE)
        og_set_v_align_button(pImpl, OG_V_ALIGN_MIDDLE);
    else
        og_set_v_align_button(pImpl, OG_V_ALIGN_BOTTOM);
}

void OguiButton::Focus(int withCursor)
{
    og_warp_cursor_to(withCursor, pImpl);
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
    if (image != NULL) mat = ( (OguiStormImage *)image )->mat;
    if (imageDown != NULL) matdown = ( (OguiStormImage *)imageDown )->mat;
    if (imageDisabled != NULL) matdisabled = ( (OguiStormImage *)imageDisabled )->mat;
    if (imageHighlighted != NULL) mathighlighted = ( (OguiStormImage *)imageHighlighted )->mat;
    og_set_pic_button(pImpl, mat, matdown, matdisabled, mathighlighted);
}

void OguiButton::ApplyFonts()
{
    IStorm3D_Font *fnt = NULL;
    IStorm3D_Font *fnt_down = NULL;
    IStorm3D_Font *fnt_disabled = NULL;
    IStorm3D_Font *fnt_highlighted = NULL;
    COL fnt_color;
    COL fnt_down_color;
    COL fnt_disabled_color;
    COL fnt_highlighted_color;

    if (font != NULL) {
        fnt = ( (OguiStormFont *)font )->fnt;
        fnt_color = ( (OguiStormFont *)font )->color;
    }
    if (fontDown != NULL) {
        fnt_down = ( (OguiStormFont *)fontDown )->fnt;
        fnt_down_color = ( (OguiStormFont *)fontDown )->color;
    }
    if (fontDisabled  != NULL) {
        fnt_disabled = ( (OguiStormFont *)fontDisabled )->fnt;
        fnt_disabled_color = ( (OguiStormFont *)fontDisabled )->color;
    }
    if (fontHighlighted != NULL) {
        fnt_highlighted = ( (OguiStormFont *)fontHighlighted )->fnt;
        fnt_highlighted_color = ( (OguiStormFont *)fontHighlighted )->color;
    }

    og_set_fonts_button(pImpl,
                        fnt,
                        fnt_color,
                        fnt_down,
                        fnt_down_color,
                        fnt_disabled,
                        fnt_disabled_color,
                        fnt_highlighted,
                        fnt_highlighted_color);

    // need to update font metrics (set the text again)
    // a hack to do that
    // need to copy the original text first, as the method call will
    // delete the original text
    if (pImpl->text != NULL) {
        char *buf = new char[strlen(pImpl->text) + 1];
        strcpy(buf, pImpl->text);
        SetText(NULL);
        SetText(buf);
        delete[] buf;
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
    return pImpl->sizex;
}

int OguiButton::GetSizeY()
{
    return pImpl->sizey;
}

int OguiButton::GetX()
{
    return pImpl->put_x;
}

int OguiButton::GetY()
{
    return pImpl->put_y;
}

void OguiButton::SetClip(float leftX, float topY, float rightX, float bottomY)
{
    pImpl->clipleftx = leftX;
    pImpl->cliptopy = topY;
    pImpl->cliprightx = rightX;
    pImpl->clipbottomy = bottomY;
}

void OguiButton::GetClip(float &leftX, float &topY, float &rightX, float &bottomY)
{
    leftX = pImpl->clipleftx;
    topY = pImpl->cliptopy;
    rightX = pImpl->cliprightx;
    bottomY = pImpl->clipbottomy;
}

void OguiButton::SetScroll(float scrollX, float scrollY)
{
    pImpl->scroll_x = scrollX;
    pImpl->scroll_y = scrollY;
    pImpl->wrap = true;
}

void OguiButton::SetRepeat(float repeatX, float repeatY)
{
    pImpl->repeat_x = repeatX;
    pImpl->repeat_y = repeatY;
    pImpl->wrap = true;
}

void OguiButton::SetHotKey(int hotkey, int hotkeyModifier1, int hotkeyModifier2)
{
    pImpl->hotKeys[0] = hotkey;
    pImpl->hotKeys[1] = hotkeyModifier1;
    pImpl->hotKeys[2] = hotkeyModifier2;
}

void OguiButton::SetSelected(bool s)
{
    if (selected != s) {
        std::swap(image, imageSelected);
        std::swap(imageHighlighted, imageSelectedHigh);
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
    if (imageSelected != NULL)
        delete imageSelected;

    imageSelected = NULL;

    if (imageSelectedHigh != NULL)
        delete imageSelectedHigh;

    imageSelectedHigh = NULL;
}
void OguiButton::SetSelectedImages(IOguiImage *selected_norm, IOguiImage *selected_high)
{
    imageSelected = selected_norm;
    imageSelectedHigh = selected_high;
}

IOguiFont *OguiButton::GetTheFontCurrentlyInUse()
{
    orvgui_but *orv_button = pImpl;
    IOguiFont *result = font;

    if (orv_button->enabled == 0) {
        if (fontDisabled != NULL)
            result = fontDisabled;
    } else {
        if (orv_button->pressed == 0) {
            if (orv_button->highlighted != 0)
                if (fontHighlighted != NULL)
                    result = fontHighlighted;
        } else {
            if (fontDown != NULL)
                result = fontDown;
        }
    }

    return result;
}

void OguiButton::PressButton(bool press)
{
    orvgui_but *orv_button = pImpl;
    if (!press) og_unpress_button(orv_button);
    else og_press_button(orv_button);
}

void OguiButton::SetCustomShape(Vertex *vertices, int numVertices)
{
    orvgui_but *orv_button = pImpl;

    if (numVertices == 0 || vertices == NULL) {
        delete[] orv_button->vertices;
        orv_button->vertices = NULL;
        orv_button->num_vertices = 0;
        return;
    }

    if (orv_button->vertices != NULL)
        delete[] orv_button->vertices;
    orv_button->num_vertices = numVertices;
    orv_button->vertices = new orvgui_but::vertex[numVertices];
    // note: assuming orvgui_but::vertex is same as OguiButton::Vertex
    memcpy( orv_button->vertices, vertices, numVertices * sizeof(Vertex) );
}

void OguiButton::SetWrap(bool wrap)
{
    pImpl->wrap = wrap;
}

void OguiButton::SetSize(int w, int h) {
    pImpl->sizex = w;
    pImpl->sizey = h;
}

void OguiButton::SetPos(int x, int y) {
    pImpl->put_x = x;
    pImpl->put_y = y;
}

orvgui_win* OguiButton::GetParent() {
    return pImpl->parent;
}
