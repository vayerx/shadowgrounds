
#include "precompiled.h"

#include "uidefaults.h"
#include "../filesystem/input_stream_wrapper.h"
#include "../util/Debug_MemoryManager.h"

namespace ui
{
#if defined(PROJECT_SHADOWGROUNDS) || defined(PROJECT_SURVIVOR) || defined(PROJECT_CLAW_PROTO)

  IOguiFont *defaultFont = NULL;
  IOguiFont *defaultThinFont = NULL;
  IOguiFont *defaultThinWhiteFont = NULL;
  IOguiFont *defaultSmallRedFont = NULL;
  IOguiFont *defaultDisabledFont = NULL;
  IOguiFont *defaultRedInfoFont = NULL;
  IOguiFont *defaultIngameFont = NULL;
  IOguiFont *defaultBigIngameFont = NULL;
  IOguiFont *defaultSmallIngameFont = NULL;
  IOguiFont *defaultHighlightSmallIngameFont = NULL;
  IOguiFont *defaultMediumIngameFont = NULL;
  IOguiFont *defaultIngameNumbersBoldFont = NULL;
  IOguiFont *defaultIngameNumbersBoldSmallFont = NULL;

  IOguiImage *closeImage = NULL;
  IOguiImage *closeImageDown = NULL;
  IOguiImage *closeImageDis = NULL;

  IOguiImage *selImage = NULL;
  IOguiImage *selImageDown = NULL;
  IOguiImage *selImageDis = NULL;
  IOguiImage *upImage = NULL;
  IOguiImage *upImageDown = NULL;
  IOguiImage *upImageDis = NULL;
  IOguiImage *downImage = NULL;
  IOguiImage *downImageDown = NULL;
  IOguiImage *downImageDis = NULL;

  OguiButtonStyle *unselStyle = NULL;
  OguiButtonStyle *selStyle = NULL;
  OguiButtonStyle *upStyle = NULL;
  OguiButtonStyle *downStyle = NULL;


  OguiButtonStyle *defaultCloseButton = NULL;
  OguiButtonStyle *defaultUpButton = NULL;
  OguiButtonStyle *defaultDownButton = NULL;
  OguiSelectListStyle *defaultSelectList = NULL;

  //Ogui *defaultOgui = NULL;


  void createUIDefaults(Ogui *ogui)
  {
    //defaultOgui = ogui;

    defaultFont = ogui->LoadFont("Data/Fonts/default.ogf");
    defaultThinFont = ogui->LoadFont("Data/Fonts/default_thin.ogf");
    defaultThinWhiteFont = ogui->LoadFont("Data/Fonts/default_thin_white.ogf");
    defaultSmallRedFont = ogui->LoadFont("Data/Fonts/default_small_red.ogf");
    defaultDisabledFont = ogui->LoadFont("Data/Fonts/default_disabled.ogf");
    defaultRedInfoFont = ogui->LoadFont("Data/Fonts/default_red_info.ogf");

/*
#ifdef SHADOWGROUNDS_UNICODE
    defaultIngameFont = ogui->LoadFont("Data/Fonts/Unicode/small_blueish.fbf");
#else
    defaultIngameFont = ogui->LoadFont("Data/Fonts/default_ingame.ogf");
#endif
*/
	// HAXHAX -- detect unicode from certain font file
	frozenbyte::filesystem::FB_FILE *fp = frozenbyte::filesystem::fb_fopen("Data/Fonts/Unicode/small_blueish.fbf", "rb");
	if(fp && frozenbyte::filesystem::fb_fsize(fp) > 0)
	    defaultIngameFont = ogui->LoadFont("Data/Fonts/Unicode/small_blueish.fbf");
	else
	    defaultIngameFont = ogui->LoadFont("Data/Fonts/default_ingame.ogf");
	if(fp)
		frozenbyte::filesystem::fb_fclose(fp);

    defaultBigIngameFont = ogui->LoadFont("Data/Fonts/default_ingame_big.ogf");
    defaultSmallIngameFont = ogui->LoadFont("Data/Fonts/default_ingame_small.ogf");
    defaultMediumIngameFont = ogui->LoadFont("Data/Fonts/default_ingame_medium.ogf");
    defaultHighlightSmallIngameFont = ogui->LoadFont("Data/Fonts/default_ingame_small_highlight.ogf");
    defaultIngameNumbersBoldFont = ogui->LoadFont("Data/Fonts/default_ingame_numbers_bold.ogf");
    defaultIngameNumbersBoldSmallFont = ogui->LoadFont("Data/Fonts/default_ingame_numbers_bold_small.ogf");

	// HAXHAX -- Artistit pakotti unicode fontin consoliin - Pete
	fp = frozenbyte::filesystem::fb_fopen("Data/Fonts/Unicode/console_font.fbf", "rb");
	if(fp && frozenbyte::filesystem::fb_fsize(fp) > 0)
	{
		delete defaultThinFont;
	    defaultThinFont = ogui->LoadFont("Data/Fonts/Unicode/console_font.fbf");
	}
	if(fp)
		frozenbyte::filesystem::fb_fclose(fp);



    closeImage = ogui->LoadOguiImage("Data/GUI/Buttons/close.tga");
    closeImageDown = ogui->LoadOguiImage("Data/GUI/Buttons/close_down.tga");
    closeImageDis = ogui->LoadOguiImage("Data/GUI/Buttons/close_disabled.tga");

    OguiButtonStyle *closeStyle = new OguiButtonStyle(closeImage, closeImageDown, closeImageDis, closeImage, NULL, 16, 16);

    selImage = ogui->LoadOguiImage("Data/GUI/Buttons/select.tga");
    selImageDown = ogui->LoadOguiImage("Data/GUI/Buttons/select_selected.tga");
    selImageDis = ogui->LoadOguiImage("Data/GUI/Buttons/select_disabled.tga");
    upImage = ogui->LoadOguiImage("Data/GUI/Buttons/scrollu.tga");
    upImageDown = ogui->LoadOguiImage("Data/GUI/Buttons/scrollu_down.tga");
    upImageDis = ogui->LoadOguiImage("Data/GUI/Buttons/scrollu_disabled.tga");
    downImage = ogui->LoadOguiImage("Data/GUI/Buttons/scrolld.tga");
    downImageDown = ogui->LoadOguiImage("Data/GUI/Buttons/scrolld_down.tga");
    downImageDis = ogui->LoadOguiImage("Data/GUI/Buttons/scrolld_disabled.tga");

    unselStyle = new OguiButtonStyle(selImage, selImageDown, selImageDis, selImage, defaultFont, 784, 20);
    selStyle = new OguiButtonStyle(selImageDown, selImage, selImageDis, selImageDown, defaultFont, 784, 20);
    upStyle = new OguiButtonStyle(upImage, upImageDown, upImageDis, upImage, NULL, 16, 16);
    downStyle = new OguiButtonStyle(downImage, downImageDown, downImageDis, downImage, NULL, 16, 16);

    OguiSelectListStyle *sellistStyle = new OguiSelectListStyle(unselStyle, selStyle, upStyle, downStyle, 800, 600, 16, 16);

    defaultCloseButton = closeStyle;
    defaultUpButton = upStyle;
    defaultDownButton = downStyle;
    defaultSelectList = sellistStyle;
  }

  void deleteUIDefaults()
  {
    delete defaultSelectList;
    delete unselStyle;
    delete selStyle;
    delete upStyle;   // = defaultUpButton
    delete downStyle; // = defaultDownButton

    delete selImageDis;
    delete selImageDown;
    delete selImage;
    delete downImageDis;
    delete downImageDown;
    delete downImage;
    delete upImageDis;
    delete upImageDown;
    delete upImage;

    delete defaultCloseButton;
    delete closeImageDis;
    delete closeImageDown;
    delete closeImage;

    delete defaultFont;
    delete defaultThinFont;
    delete defaultThinWhiteFont;
    delete defaultSmallRedFont;
    delete defaultDisabledFont;
    delete defaultRedInfoFont;
    delete defaultIngameFont;
    delete defaultBigIngameFont;
    delete defaultSmallIngameFont;
    delete defaultMediumIngameFont;
    delete defaultHighlightSmallIngameFont;
    delete defaultIngameNumbersBoldFont;
    delete defaultIngameNumbersBoldSmallFont;

    // TODO, might want to null the variables now...
  }
#else
  IOguiFont *defaultFont = NULL;
  IOguiFont *defaultThinFont = NULL;
  IOguiFont *defaultThinWhiteFont = NULL;
  IOguiFont *defaultSmallRedFont = NULL;
  IOguiFont *defaultDisabledFont = NULL;
  IOguiFont *defaultRedInfoFont = NULL;
  IOguiFont *defaultIngameFont = NULL;
  IOguiFont *defaultBigIngameFont = NULL;
  IOguiFont *defaultSmallIngameFont = NULL;
  IOguiFont *defaultHighlightSmallIngameFont = NULL;
  IOguiFont *defaultMediumIngameFont = NULL;
  IOguiFont *defaultIngameNumbersBoldFont = NULL;
  IOguiFont *defaultIngameNumbersBoldSmallFont = NULL;

  IOguiImage *closeImage = NULL;
  IOguiImage *closeImageDown = NULL;
  IOguiImage *closeImageDis = NULL;

  IOguiImage *selImage = NULL;
  IOguiImage *selImageDown = NULL;
  IOguiImage *selImageDis = NULL;
  IOguiImage *upImage = NULL;
  IOguiImage *upImageDown = NULL;
  IOguiImage *upImageDis = NULL;
  IOguiImage *downImage = NULL;
  IOguiImage *downImageDown = NULL;
  IOguiImage *downImageDis = NULL;

  OguiButtonStyle *unselStyle = NULL;
  OguiButtonStyle *selStyle = NULL;
  OguiButtonStyle *upStyle = NULL;
  OguiButtonStyle *downStyle = NULL;


  OguiButtonStyle *defaultCloseButton = NULL;
  OguiButtonStyle *defaultUpButton = NULL;
  OguiButtonStyle *defaultDownButton = NULL;
  OguiSelectListStyle *defaultSelectList = NULL;

  //Ogui *defaultOgui = NULL;


  void createUIDefaults(Ogui *ogui)
  {
    //defaultOgui = ogui;

    defaultFont = ogui->LoadFont("data/gui/font/common/default.ogf");
    defaultIngameFont = ogui->LoadFont("data/gui/font/common/default.ogf");

    closeImage = ogui->LoadOguiImage("data/gui/common/button/close.tga");
    closeImageDown = ogui->LoadOguiImage("data/gui/common/button/close_down.tga");
    closeImageDis = ogui->LoadOguiImage("data/gui/common/button/close_disabled.tga");

    OguiButtonStyle *closeStyle = new OguiButtonStyle(closeImage, closeImageDown, closeImageDis, closeImage, NULL, 16, 16);

    defaultCloseButton = closeStyle;
  }

  void deleteUIDefaults()
  {
    delete defaultCloseButton;
    delete closeImageDis;
    delete closeImageDown;
    delete closeImage;

    delete defaultFont;
    delete defaultIngameFont;
  }
#endif

}

