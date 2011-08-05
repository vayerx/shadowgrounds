
#include "precompiled.h"

#include "AniRecorderWindow.h"

#include "../game/AniRecorder.h"

#include <stdlib.h>
#include <stdio.h>

#include <string>
#include <vector>

#include <keyb3.h>

#include "AnimationSet.h"
#include "uidefaults.h"
#include "../game/Game.h"
#include "../game/UnitList.h"
#include "../game/GameUI.h"
#include "../game/UnitSelections.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_players.h"
#include "MessageBoxWindow.h"
#include "GameConsole.h"
#include "../ogui/Ogui.h"
#include "../system/Logger.h"
#include "../convert/str2int.h"

#include "../util/Debug_MemoryManager.h"

#define ANIRECW_CLOSE 1

#define ANIRECW_CAMERAMODE 2
#define ANIRECW_CAMERADUMP 3
#define ANIRECW_CAMERATEST 4
#define ANIRECW_CAMERAINTERP 5
#define ANIRECW_CAMERADELETE 6

#define ANIRECW_ADDUNIT 7
#define ANIRECW_REMOVEUNIT 8
#define ANIRECW_REC 9
#define ANIRECW_PLAY 10
#define ANIRECW_PAUSE 11
#define ANIRECW_REWIND 12

#define ANIRECW_POSITION 13
#define ANIRECW_SLIDER 14

#define ANIRECW_RECORDPATH 15
#define ANIRECW_RELOAD 16

#define ANIRECW_MINIMIZE 17

#define ANIRECW_ADDANIM 18
#define ANIRECW_ADDFEWTICKS 19
//#define ANIRECW_ADDMANYTICKS 20
#define ANIRECW_ADDCOMMANDS 21
#define ANIRECW_SMOOTHPOSITION 22
#define ANIRECW_SMOOTHROTATION 23
#define ANIRECW_SMOOTHAIM 24
#define ANIRECW_UNDO 25
#define ANIRECW_REDO 26
#define ANIRECW_POSITION_END 27
#define ANIRECW_DELETEPOSITION 28
#define ANIRECW_DROPONGROUND 29


#define MSGBOX_CAMERADELETE 50001

#define ANIRECW_SLIDER_WIDTH 400
#define ANIRECW_RECORD_COUNT_TIME 4


namespace ui
{

  AniRecorderWindow::AniRecorderWindow(Ogui *ogui, game::Game *game)
  {
    this->ogui = ogui;
    this->game = game;

#ifdef LEGACY_FILES

		const char *bgpic = "Data/GUI/Windows/anirecorder.tga";
    win = ogui->CreateSimpleWindow(0, 0, 512, 256+40+16, bgpic);

		font = ogui->LoadFont("Data/Fonts/anirecorder.ogf");
		smallFont = ogui->LoadFont("Data/Fonts/anirecorder_small.ogf");

    closebut = ogui->CreateSimpleImageButton(win, 6, 6, 16, 16, 
      "Data/GUI/Buttons/Anirecorder/close.tga", 
			"Data/GUI/Buttons/Anirecorder/close_down.tga",
      "Data/GUI/Buttons/Anirecorder/close.tga", 
			ANIRECW_CLOSE);
    closebut->SetListener(this);
		closebut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

    minibut = ogui->CreateSimpleImageButton(win, 512-16-6, 6, 16, 16, 
      "Data/GUI/Buttons/Anirecorder/scroll_up.tga", 
			"Data/GUI/Buttons/Anirecorder/scroll_up_pressed.tga",
      "Data/GUI/Buttons/Anirecorder/scroll_up.tga", 
			ANIRECW_MINIMIZE);
    minibut->SetListener(this);
		minibut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

    cameraModeBut = ogui->CreateSimpleImageButton(win, 8, 32, 32, 32, 
      "Data/GUI/Buttons/Anirecorder/cammode.tga", 
      "Data/GUI/Buttons/Anirecorder/cammode_down.tga", 
      "Data/GUI/Buttons/Anirecorder/cammode.tga", 
			ANIRECW_CAMERAMODE);
    cameraModeBut->SetListener(this);
		cameraModeBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

    cameraDumpBut = ogui->CreateSimpleImageButton(win, 8+40*1, 32, 32, 32, 
      "Data/GUI/Buttons/Anirecorder/camdump.tga", 
      "Data/GUI/Buttons/Anirecorder/camdump_down.tga", 
      "Data/GUI/Buttons/Anirecorder/camdump.tga", 
      "Data/GUI/Buttons/Anirecorder/camdump_disabled.tga", 
			ANIRECW_CAMERADUMP);
    cameraDumpBut->SetListener(this);
    cameraDumpBut->SetDisabled(true);
		cameraDumpBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		cameraDumpBut->SetHotKey(KEYCODE_KEYPAD_1);

    cameraTestBut = ogui->CreateSimpleImageButton(win, 8+40*2, 32, 32, 32, 
      "Data/GUI/Buttons/Anirecorder/camtest.tga", 
      "Data/GUI/Buttons/Anirecorder/camtest_down.tga", 
      "Data/GUI/Buttons/Anirecorder/camtest.tga", 
      "Data/GUI/Buttons/Anirecorder/camtest_disabled.tga", 
			ANIRECW_CAMERATEST);
    cameraTestBut->SetListener(this);
    cameraTestBut->SetDisabled(true);
		cameraTestBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		cameraTestBut->SetHotKey(KEYCODE_KEYPAD_2);

    cameraInterpBut = ogui->CreateSimpleImageButton(win, 8+40*3, 32, 32, 32, 
      "Data/GUI/Buttons/Anirecorder/caminterp.tga", 
      "Data/GUI/Buttons/Anirecorder/caminterp_down.tga", 
      "Data/GUI/Buttons/Anirecorder/caminterp.tga", 
      "Data/GUI/Buttons/Anirecorder/caminterp_disabled.tga", 
			ANIRECW_CAMERAINTERP);
    cameraInterpBut->SetListener(this);
    cameraInterpBut->SetDisabled(true);
		cameraInterpBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		cameraInterpBut->SetHotKey(KEYCODE_KEYPAD_3);

    cameraDelBut = ogui->CreateSimpleImageButton(win, 8+40*4, 32, 32, 32, 
      "Data/GUI/Buttons/Anirecorder/camdel.tga", 
      "Data/GUI/Buttons/Anirecorder/camdel_down.tga", 
      "Data/GUI/Buttons/Anirecorder/camdel.tga", 
      "Data/GUI/Buttons/Anirecorder/camdel_disabled.tga", 
			ANIRECW_CAMERADELETE);
    cameraDelBut->SetListener(this);
    cameraDelBut->SetDisabled(true);
		cameraDelBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		cameraDelBut->SetHotKey(KEYCODE_KEYPAD_4);

    addUnitBut = ogui->CreateSimpleImageButton(win, 256+40*0, 32, 32, 32, 
      "Data/GUI/Buttons/Anirecorder/addunit.tga", 
      "Data/GUI/Buttons/Anirecorder/addunit_down.tga", 
      "Data/GUI/Buttons/Anirecorder/addunit.tga", 
      "Data/GUI/Buttons/Anirecorder/addunit_disabled.tga", 
			ANIRECW_ADDUNIT);
    addUnitBut->SetListener(this);
		addUnitBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

    removeUnitBut = ogui->CreateSimpleImageButton(win, 256+40*1, 32, 32, 32, 
      "Data/GUI/Buttons/Anirecorder/removeunit.tga", 
      "Data/GUI/Buttons/Anirecorder/removeunit_down.tga", 
      "Data/GUI/Buttons/Anirecorder/removeunit.tga", 
      "Data/GUI/Buttons/Anirecorder/removeunit_disabled.tga", 
			ANIRECW_REMOVEUNIT);
    removeUnitBut->SetListener(this);
		removeUnitBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

    recBut = ogui->CreateSimpleImageButton(win, 256+8+8+40*2, 32, 32, 32, 
      "Data/GUI/Buttons/Anirecorder/rec.tga", 
      "Data/GUI/Buttons/Anirecorder/rec_down.tga", 
      "Data/GUI/Buttons/Anirecorder/rec.tga", 
      "Data/GUI/Buttons/Anirecorder/rec_disabled.tga", 
			ANIRECW_REC);
    recBut->SetListener(this);
		recBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		recBut->SetHotKey(KEYCODE_KEYPAD_5);

    playBut = ogui->CreateSimpleImageButton(win, 256+8+8+40*3, 32, 32, 32, 
      "Data/GUI/Buttons/Anirecorder/play.tga", 
      "Data/GUI/Buttons/Anirecorder/play_down.tga", 
      "Data/GUI/Buttons/Anirecorder/play.tga", 
      "Data/GUI/Buttons/Anirecorder/play_disabled.tga", 
			ANIRECW_PLAY);
    playBut->SetListener(this);
		playBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		playBut->SetHotKey(KEYCODE_KEYPAD_6);

    pauseBut = ogui->CreateSimpleImageButton(win, 256+8+8+40*4, 32, 32, 32, 
      "Data/GUI/Buttons/Anirecorder/pause.tga", 
      "Data/GUI/Buttons/Anirecorder/pause_down.tga", 
      "Data/GUI/Buttons/Anirecorder/pause.tga", 
      "Data/GUI/Buttons/Anirecorder/pause_disabled.tga", 
			ANIRECW_PAUSE);
    pauseBut->SetListener(this);
		pauseBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		pauseBut->SetHotKey(KEYCODE_KEYPAD_7);

    rewindBut = ogui->CreateSimpleImageButton(win, 256+8+8+40*5, 32, 32, 32, 
      "Data/GUI/Buttons/Anirecorder/rewind.tga", 
      "Data/GUI/Buttons/Anirecorder/rewind_down.tga", 
      "Data/GUI/Buttons/Anirecorder/rewind.tga", 
      "Data/GUI/Buttons/Anirecorder/rewind_disabled.tga", 
			ANIRECW_REWIND);
    rewindBut->SetListener(this);
		rewindBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		rewindBut->SetHotKey(KEYCODE_KEYPAD_8);

    positionBut = ogui->CreateSimpleImageButton(win, 12, 80+16*6, 16, 16, 
      "Data/GUI/Buttons/Anirecorder/position.tga", 
			"Data/GUI/Buttons/Anirecorder/position.tga",
      "Data/GUI/Buttons/Anirecorder/position.tga", 
			ANIRECW_POSITION);
    positionBut->SetListener(this);

    positionEndBut = ogui->CreateSimpleImageButton(win, 12, 80+16*6, 16, 16, 
      "Data/GUI/Buttons/Anirecorder/position_end.tga", 
			"Data/GUI/Buttons/Anirecorder/position_end.tga",
      "Data/GUI/Buttons/Anirecorder/position_end.tga", 
			ANIRECW_POSITION_END);
    positionEndBut->SetListener(this);
    positionEndBut->SetDisabled(true);
		//positionEndBut->SetEventMask(OGUI_EMASK_PRESS|OGUI_EMASK_OVER|OGUI_EMASK_LEAVE|OGUI_EMASK_OUT|OGUI_EMASK_CLICK);

    sliderBut = ogui->CreateSimpleImageButton(win, 12, 80+16*6, ANIRECW_SLIDER_WIDTH, 16, 
      "Data/GUI/Buttons/Anirecorder/slider.tga", 
			"Data/GUI/Buttons/Anirecorder/slider.tga",
      "Data/GUI/Buttons/Anirecorder/slider.tga", 
			ANIRECW_SLIDER);
    sliderBut->SetListener(this);
		sliderBut->SetEventMask(OGUI_EMASK_PRESS|OGUI_EMASK_OVER|OGUI_EMASK_LEAVE|OGUI_EMASK_OUT|OGUI_EMASK_CLICK);

    timeLabel = ogui->CreateTextArea(win, ANIRECW_SLIDER_WIDTH+16+8, 80+16*6, 512-ANIRECW_SLIDER_WIDTH-16, 16, "00:00.00");
    timeLabel->SetFont(font);

    unselImage = ogui->LoadOguiImage("Data/GUI/Buttons/Anirecorder/select.tga");
    selImage = ogui->LoadOguiImage("Data/GUI/Buttons/Anirecorder/select_selected.tga");
    selDownImage = ogui->LoadOguiImage("Data/GUI/Buttons/Anirecorder/select_down.tga");

    scrollUpImage = ogui->LoadOguiImage("Data/GUI/Buttons/Anirecorder/scroll_up.tga");
    scrollUpPressedImage = ogui->LoadOguiImage("Data/GUI/Buttons/Anirecorder/scroll_up_pressed.tga");
    scrollUpDisabledImage = ogui->LoadOguiImage("Data/GUI/Buttons/Anirecorder/scroll_up_disabled.tga");
    scrollDownImage = ogui->LoadOguiImage("Data/GUI/Buttons/Anirecorder/scroll_down.tga");
    scrollDownPressedImage = ogui->LoadOguiImage("Data/GUI/Buttons/Anirecorder/scroll_down_pressed.tga");
    scrollDownDisabledImage = ogui->LoadOguiImage("Data/GUI/Buttons/Anirecorder/scroll_down_disabled.tga");

#else

		char *bgpic = "data/gui/menu/anirecorder/window/anirecorder.tga";
    win = ogui->CreateSimpleWindow(0, 0, 512, 256+40+16, bgpic);

		font = ogui->LoadFont("data/gui/font/menu/anirecorder/anirecorder.ogf");
		smallFont = ogui->LoadFont("data/gui/font/menu/anirecorder/anirecorder_small.ogf");

    closebut = ogui->CreateSimpleImageButton(win, 6, 6, 16, 16, 
      "data/gui/menu/anirecorder/button/close.tga", 
			"data/gui/menu/anirecorder/button/close_down.tga",
      "data/gui/menu/anirecorder/button/close.tga", 
			ANIRECW_CLOSE);
    closebut->SetListener(this);
		closebut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

    minibut = ogui->CreateSimpleImageButton(win, 512-16-6, 6, 16, 16, 
      "data/gui/menu/anirecorder/button/scroll_up.tga", 
			"data/gui/menu/anirecorder/button/scroll_up_pressed.tga",
      "data/gui/menu/anirecorder/button/scroll_up.tga", 
			ANIRECW_MINIMIZE);
    minibut->SetListener(this);
		minibut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

    cameraModeBut = ogui->CreateSimpleImageButton(win, 8, 32, 32, 32, 
      "data/gui/menu/anirecorder/button/cammode.tga", 
      "data/gui/menu/anirecorder/button/cammode_down.tga", 
      "data/gui/menu/anirecorder/button/cammode.tga", 
			ANIRECW_CAMERAMODE);
    cameraModeBut->SetListener(this);
		cameraModeBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

    cameraDumpBut = ogui->CreateSimpleImageButton(win, 8+40*1, 32, 32, 32, 
      "data/gui/menu/anirecorder/button/camdump.tga", 
      "data/gui/menu/anirecorder/button/camdump_down.tga", 
      "data/gui/menu/anirecorder/button/camdump.tga", 
      "data/gui/menu/anirecorder/button/camdump_disabled.tga", 
			ANIRECW_CAMERADUMP);
    cameraDumpBut->SetListener(this);
    cameraDumpBut->SetDisabled(true);
		cameraDumpBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		cameraDumpBut->SetHotKey(KEYCODE_KEYPAD_1);

    cameraTestBut = ogui->CreateSimpleImageButton(win, 8+40*2, 32, 32, 32, 
      "data/gui/menu/anirecorder/button/camtest.tga", 
      "data/gui/menu/anirecorder/button/camtest_down.tga", 
      "data/gui/menu/anirecorder/button/camtest.tga", 
      "data/gui/menu/anirecorder/button/camtest_disabled.tga", 
			ANIRECW_CAMERATEST);
    cameraTestBut->SetListener(this);
    cameraTestBut->SetDisabled(true);
		cameraTestBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		cameraTestBut->SetHotKey(KEYCODE_KEYPAD_2);

    cameraInterpBut = ogui->CreateSimpleImageButton(win, 8+40*3, 32, 32, 32, 
      "data/gui/menu/anirecorder/button/caminterp.tga", 
      "data/gui/menu/anirecorder/button/caminterp_down.tga", 
      "data/gui/menu/anirecorder/button/caminterp.tga", 
      "data/gui/menu/anirecorder/button/caminterp_disabled.tga", 
			ANIRECW_CAMERAINTERP);
    cameraInterpBut->SetListener(this);
    cameraInterpBut->SetDisabled(true);
		cameraInterpBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		cameraInterpBut->SetHotKey(KEYCODE_KEYPAD_3);

    cameraDelBut = ogui->CreateSimpleImageButton(win, 8+40*4, 32, 32, 32, 
      "data/gui/menu/anirecorder/button/camdel.tga", 
      "data/gui/menu/anirecorder/button/camdel_down.tga", 
      "data/gui/menu/anirecorder/button/camdel.tga", 
      "data/gui/menu/anirecorder/button/camdel_disabled.tga", 
			ANIRECW_CAMERADELETE);
    cameraDelBut->SetListener(this);
    cameraDelBut->SetDisabled(true);
		cameraDelBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		cameraDelBut->SetHotKey(KEYCODE_KEYPAD_4);

    addUnitBut = ogui->CreateSimpleImageButton(win, 256+40*0, 32, 32, 32, 
      "data/gui/menu/anirecorder/button/addunit.tga", 
      "data/gui/menu/anirecorder/button/addunit_down.tga", 
      "data/gui/menu/anirecorder/button/addunit.tga", 
      "data/gui/menu/anirecorder/button/addunit_disabled.tga", 
			ANIRECW_ADDUNIT);
    addUnitBut->SetListener(this);
		addUnitBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

    removeUnitBut = ogui->CreateSimpleImageButton(win, 256+40*1, 32, 32, 32, 
      "data/gui/menu/anirecorder/button/removeunit.tga", 
      "data/gui/menu/anirecorder/button/removeunit_down.tga", 
      "data/gui/menu/anirecorder/button/removeunit.tga", 
      "data/gui/menu/anirecorder/button/removeunit_disabled.tga", 
			ANIRECW_REMOVEUNIT);
    removeUnitBut->SetListener(this);
		removeUnitBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

    recBut = ogui->CreateSimpleImageButton(win, 256+8+8+40*2, 32, 32, 32, 
      "data/gui/menu/anirecorder/button/rec.tga", 
      "data/gui/menu/anirecorder/button/rec_down.tga", 
      "data/gui/menu/anirecorder/button/rec.tga", 
      "data/gui/menu/anirecorder/button/rec_disabled.tga", 
			ANIRECW_REC);
    recBut->SetListener(this);
		recBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		recBut->SetHotKey(KEYCODE_KEYPAD_5);

    playBut = ogui->CreateSimpleImageButton(win, 256+8+8+40*3, 32, 32, 32, 
      "data/gui/menu/anirecorder/button/play.tga", 
      "data/gui/menu/anirecorder/button/play_down.tga", 
      "data/gui/menu/anirecorder/button/play.tga", 
      "data/gui/menu/anirecorder/button/play_disabled.tga", 
			ANIRECW_PLAY);
    playBut->SetListener(this);
		playBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		playBut->SetHotKey(KEYCODE_KEYPAD_6);

    pauseBut = ogui->CreateSimpleImageButton(win, 256+8+8+40*4, 32, 32, 32, 
      "data/gui/menu/anirecorder/button/pause.tga", 
      "data/gui/menu/anirecorder/button/pause_down.tga", 
      "data/gui/menu/anirecorder/button/pause.tga", 
      "data/gui/menu/anirecorder/button/pause_disabled.tga", 
			ANIRECW_PAUSE);
    pauseBut->SetListener(this);
		pauseBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		pauseBut->SetHotKey(KEYCODE_KEYPAD_7);

    rewindBut = ogui->CreateSimpleImageButton(win, 256+8+8+40*5, 32, 32, 32, 
      "data/gui/menu/anirecorder/button/rewind.tga", 
      "data/gui/menu/anirecorder/button/rewind_down.tga", 
      "data/gui/menu/anirecorder/button/rewind.tga", 
      "data/gui/menu/anirecorder/button/rewind_disabled.tga", 
			ANIRECW_REWIND);
    rewindBut->SetListener(this);
		rewindBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		rewindBut->SetHotKey(KEYCODE_KEYPAD_8);

    positionBut = ogui->CreateSimpleImageButton(win, 12, 80+16*6, 16, 16, 
      "data/gui/menu/anirecorder/button/position.tga", 
			"data/gui/menu/anirecorder/button/position.tga",
      "data/gui/menu/anirecorder/button/position.tga", 
			ANIRECW_POSITION);
    positionBut->SetListener(this);

    positionEndBut = ogui->CreateSimpleImageButton(win, 12, 80+16*6, 16, 16, 
      "data/gui/menu/anirecorder/button/position_end.tga", 
			"data/gui/menu/anirecorder/button/position_end.tga",
      "data/gui/menu/anirecorder/button/position_end.tga", 
			ANIRECW_POSITION_END);
    positionEndBut->SetListener(this);
    positionEndBut->SetDisabled(true);
		//positionEndBut->SetEventMask(OGUI_EMASK_PRESS|OGUI_EMASK_OVER|OGUI_EMASK_LEAVE|OGUI_EMASK_OUT|OGUI_EMASK_CLICK);

    sliderBut = ogui->CreateSimpleImageButton(win, 12, 80+16*6, ANIRECW_SLIDER_WIDTH, 16, 
      "data/gui/menu/anirecorder/button/slider.tga", 
			"data/gui/menu/anirecorder/button/slider.tga",
      "data/gui/menu/anirecorder/button/slider.tga", 
			ANIRECW_SLIDER);
    sliderBut->SetListener(this);
		sliderBut->SetEventMask(OGUI_EMASK_PRESS|OGUI_EMASK_OVER|OGUI_EMASK_LEAVE|OGUI_EMASK_OUT|OGUI_EMASK_CLICK);

    timeLabel = ogui->CreateTextArea(win, ANIRECW_SLIDER_WIDTH+16+8, 80+16*6, 512-ANIRECW_SLIDER_WIDTH-16, 16, "00:00.00");
    timeLabel->SetFont(font);

    unselImage = ogui->LoadOguiImage("data/gui/menu/anirecorder/button/select.tga");
    selImage = ogui->LoadOguiImage("data/gui/menu/anirecorder/button/select_selected.tga");
    selDownImage = ogui->LoadOguiImage("data/gui/menu/anirecorder/button/select_down.tga");

    scrollUpImage = ogui->LoadOguiImage("data/gui/menu/anirecorder/button/scroll_up.tga");
    scrollUpPressedImage = ogui->LoadOguiImage("data/gui/menu/anirecorder/button/scroll_up_pressed.tga");
    scrollUpDisabledImage = ogui->LoadOguiImage("data/gui/menu/anirecorder/button/scroll_up_disabled.tga");
    scrollDownImage = ogui->LoadOguiImage("data/gui/menu/anirecorder/button/scroll_down.tga");
    scrollDownPressedImage = ogui->LoadOguiImage("data/gui/menu/anirecorder/button/scroll_down_pressed.tga");
    scrollDownDisabledImage = ogui->LoadOguiImage("data/gui/menu/anirecorder/button/scroll_down_disabled.tga");

#endif

    unselStyle = new OguiButtonStyle(unselImage, selDownImage, NULL, selDownImage, font, 220-16, 16);
    selStyle = new OguiButtonStyle(selImage, selDownImage, NULL, selImage, font, 220-16, 16);
    //numUnselStyle = new OguiButtonStyle(selImage, selDownImage, NULL, selImage, defaultFont, 100, 16);

    scrollUpStyle = new OguiButtonStyle(scrollUpImage, scrollUpPressedImage, scrollUpDisabledImage, scrollUpImage, defaultFont, 16, 16);
    scrollDownStyle = new OguiButtonStyle(scrollDownImage, scrollDownPressedImage, scrollDownDisabledImage, scrollDownImage, defaultFont, 16, 16);

    listStyle = new OguiSelectListStyle(unselStyle, selStyle, scrollUpStyle, scrollDownStyle, 220, 16*6, 16, 16);

    smallUnselStyle = new OguiButtonStyle(unselImage, selDownImage, NULL, selDownImage, smallFont, 158-16, 16);
    smallSelStyle = new OguiButtonStyle(selImage, selDownImage, NULL, selImage, smallFont, 158-16, 16);
    smallListStyle = new OguiSelectListStyle(smallUnselStyle, smallSelStyle, scrollUpStyle, scrollDownStyle, 158, 16*4, 16, 16);

    tinyUnselStyle = new OguiButtonStyle(unselImage, selDownImage, NULL, selDownImage, font, 38, 16);
    tinySelStyle = new OguiButtonStyle(selImage, selDownImage, NULL, selImage, font, 38, 16);
    tinyListStyle = new OguiSelectListStyle(tinyUnselStyle, tinySelStyle, scrollUpStyle, scrollDownStyle, 38+16, 16*2, 16, 16);

		createMiniDependantButtons();

		this->aniRecorder = new game::AniRecorder(game);

		char labelbuf[128];
		sprintf(labelbuf, "Ani recorder - %s", this->aniRecorder->getCurrentAniId());

    label1 = ogui->CreateTextArea(win, 32, 4, 512-32-32, 16, labelbuf);
    label1->SetFont(font);

		this->selectedCamera = -1;
		this->selectedUnit = NULL;

		this->sliderDown = false;
		this->endSliderDown = false;

		this->assumingPlayingOrRecording = false;

		this->recCounter = 0;

		this->minimized = false;
		this->sliderHeight = 80+16*6;

		this->currentSelectedAnimation = NULL;
		this->currentSelectedTicks = 0;

		updateLists();
		updateButtons();
  }

	void AniRecorderWindow::createMiniDependantButtons()
	{
		cameraSelectList = ogui->CreateSelectList(win, 8, 72, listStyle, 0, NULL, NULL);
		cameraSelectList->setListener(this);

		unitSelectList = ogui->CreateSelectList(win, 256, 72, listStyle, 0, NULL, NULL);
		unitSelectList->setListener(this);

		animationSelectList = ogui->CreateSelectList(win, 8+8+8+8+40*7, 80+24+16*7, smallListStyle, 0, NULL, NULL);
		animationSelectList->setListener(this);

		tickSelectList = ogui->CreateSelectList(win, 8+8+40*2, 80+24+16*7, tinyListStyle, 0, NULL, NULL);
		tickSelectList->setListener(this);

		std::vector<std::string> sorttmp;
		{
			for (int i = 1; i < ANIM_AMOUNT; i++)
			{
				const char *tmp = AnimationSet::getAnimName(i);
				std::string tmpstr = tmp;
				sorttmp.push_back(tmpstr);
				std::sort(sorttmp.begin(), sorttmp.end());
			}
		}

		{
			int specialStart = 0;
			for (int i = 0; i < (int)sorttmp.size(); i++)
			{
				std::string tmpstr = sorttmp[i];
				animationSelectList->addItem(tmpstr.c_str(), tmpstr.c_str(), false);
				if (tmpstr == "special1")
				{
					specialStart = i;
				}
			}
			animationSelectList->scrollTo(specialStart);
		}

		for (int i = 0; i < 14; i++)
		{
			std::string tmpstr = int2str(5 + i * 5);
			std::string tmpstr2 = tmpstr + "t";
			tickSelectList->addItem(tmpstr.c_str(), tmpstr2.c_str(), false);
		}

	
		/*
		for (int i = 1; i < ANIM_AMOUNT; i++)
		{
			const char *tmp = AnimationSet::getAnimName(i);
			animationSelectList->addItem(tmp, tmp, false);
		}
		animationSelectList->scrollTo(ANIM_SPECIAL1 - 1);
		*/

#ifdef LEGACY_FILES

		recordPathBut = ogui->CreateSimpleImageButton(win, 8, 80+24+16*7, 32, 32, 
			"Data/GUI/Buttons/Anirecorder/recpath.tga", 
			"Data/GUI/Buttons/Anirecorder/recpath_down.tga", 
			"Data/GUI/Buttons/Anirecorder/recpath.tga", 
			"Data/GUI/Buttons/Anirecorder/recpath_disabled.tga", 
			ANIRECW_RECORDPATH);
		recordPathBut->SetListener(this);
		recordPathBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

		reloadBut = ogui->CreateSimpleImageButton(win, 8+40*1, 80+24+16*7, 32, 32, 
			"Data/GUI/Buttons/Anirecorder/reload.tga", 
			"Data/GUI/Buttons/Anirecorder/reload_down.tga", 
			"Data/GUI/Buttons/Anirecorder/reload.tga", 
			"Data/GUI/Buttons/Anirecorder/reload_disabled.tga", 
			ANIRECW_RELOAD);
		reloadBut->SetListener(this);
		reloadBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

		addFewTicksBut = ogui->CreateSimpleImageButton(win, 8+16+8+40*3, 80+24+16*7, 32, 32, 			
		"Data/GUI/Buttons/Anirecorder/fewticks.tga", 
		"Data/GUI/Buttons/Anirecorder/fewticks_down.tga", 
		"Data/GUI/Buttons/Anirecorder/fewticks.tga", 
		"Data/GUI/Buttons/Anirecorder/fewticks_disabled.tga", 
		ANIRECW_ADDFEWTICKS);
		addFewTicksBut->SetListener(this);
		addFewTicksBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

		smoothPositionBut = ogui->CreateSimpleImageButton(win, 8+16+8+40*4-2, 80+24+16*7, 32, 32, 
		"Data/GUI/Buttons/Anirecorder/smoothposition.tga", 
		"Data/GUI/Buttons/Anirecorder/smoothposition_down.tga", 
		"Data/GUI/Buttons/Anirecorder/smoothposition.tga", 
		"Data/GUI/Buttons/Anirecorder/smoothposition_disabled.tga", 
		ANIRECW_SMOOTHPOSITION);
		smoothPositionBut->SetListener(this);
		smoothPositionBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

		smoothRotationBut = ogui->CreateSimpleImageButton(win, 8+16+8+40*5-4, 80+24+16*7, 32, 32, 
		"Data/GUI/Buttons/Anirecorder/smoothrotation.tga", 
		"Data/GUI/Buttons/Anirecorder/smoothrotation_down.tga", 
		"Data/GUI/Buttons/Anirecorder/smoothrotation.tga", 
		"Data/GUI/Buttons/Anirecorder/smoothrotation_disabled.tga", 
		ANIRECW_SMOOTHROTATION);
		smoothRotationBut->SetListener(this);
		smoothRotationBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

		smoothAimBut = ogui->CreateSimpleImageButton(win, 8+16+8+40*6-6, 80+24+16*7, 32, 32, 
		"Data/GUI/Buttons/Anirecorder/smoothaim.tga", 
		"Data/GUI/Buttons/Anirecorder/smoothaim_down.tga", 
		"Data/GUI/Buttons/Anirecorder/smoothaim.tga", 
		"Data/GUI/Buttons/Anirecorder/smoothaim_disabled.tga", 
		ANIRECW_SMOOTHAIM);
		smoothAimBut->SetListener(this);
		smoothAimBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

		addAnimBut = ogui->CreateSimpleImageButton(win, 8+24+40*11, 80+24+16*7, 32, 32, 
		"Data/GUI/Buttons/Anirecorder/addanim.tga", 
		"Data/GUI/Buttons/Anirecorder/addanim_down.tga", 
		"Data/GUI/Buttons/Anirecorder/addanim.tga", 
		"Data/GUI/Buttons/Anirecorder/addanim_disabled.tga", 
		ANIRECW_ADDANIM);
		addAnimBut->SetListener(this);
		addAnimBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

		undoBut = ogui->CreateSimpleImageButton(win, 8+40*0, 80+24+8+16*9, 32, 32, 
		"Data/GUI/Buttons/Anirecorder/undo.tga", 
		"Data/GUI/Buttons/Anirecorder/undo_down.tga", 
		"Data/GUI/Buttons/Anirecorder/undo.tga", 
		"Data/GUI/Buttons/Anirecorder/undo_disabled.tga", 
		ANIRECW_UNDO);
		undoBut->SetListener(this);
		undoBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

		redoBut = ogui->CreateSimpleImageButton(win, 8+40*1, 80+24+8+16*9, 32, 32, 
		"Data/GUI/Buttons/Anirecorder/redo.tga", 
		"Data/GUI/Buttons/Anirecorder/redo_down.tga", 
		"Data/GUI/Buttons/Anirecorder/redo.tga", 
		"Data/GUI/Buttons/Anirecorder/redo_disabled.tga", 
		ANIRECW_REDO);
		redoBut->SetListener(this);
		redoBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

		deletePositionBut = ogui->CreateSimpleImageButton(win, 8+8+40*2, 80+24+8+16*9, 32, 32, 
		"Data/GUI/Buttons/Anirecorder/deleteposition.tga", 
		"Data/GUI/Buttons/Anirecorder/deleteposition_down.tga", 
		"Data/GUI/Buttons/Anirecorder/deleteposition.tga", 
		"Data/GUI/Buttons/Anirecorder/deleteposition_disabled.tga", 
		ANIRECW_DELETEPOSITION);
		deletePositionBut->SetListener(this);
		deletePositionBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

		addCommandsBut = ogui->CreateSimpleImageButton(win, 8+8+40*3, 80+24+8+16*9, 32, 32, 
		"Data/GUI/Buttons/Anirecorder/commands.tga", 
		"Data/GUI/Buttons/Anirecorder/commands_down.tga", 
		"Data/GUI/Buttons/Anirecorder/commands.tga", 
		"Data/GUI/Buttons/Anirecorder/commands_disabled.tga", 
		ANIRECW_ADDCOMMANDS);
		addCommandsBut->SetListener(this);
		addCommandsBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

		dropOnGroundBut = ogui->CreateSimpleImageButton(win, 8+8+40*4, 80+24+8+16*9, 32, 32, 
		"Data/GUI/Buttons/Anirecorder/droponground.tga", 
		"Data/GUI/Buttons/Anirecorder/droponground_down.tga", 
		"Data/GUI/Buttons/Anirecorder/droponground.tga", 
		"Data/GUI/Buttons/Anirecorder/droponground_disabled.tga", 
		ANIRECW_DROPONGROUND);
		dropOnGroundBut->SetListener(this);
		dropOnGroundBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

#else

		recordPathBut = ogui->CreateSimpleImageButton(win, 8, 80+24+16*7, 32, 32, 
			"data/gui/menu/anirecorder/button/recpath.tga", 
			"data/gui/menu/anirecorder/button/recpath_down.tga", 
			"data/gui/menu/anirecorder/button/recpath.tga", 
			"data/gui/menu/anirecorder/button/recpath_disabled.tga", 
			ANIRECW_RECORDPATH);
		recordPathBut->SetListener(this);
		recordPathBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

		reloadBut = ogui->CreateSimpleImageButton(win, 8+40*1, 80+24+16*7, 32, 32, 
			"data/gui/menu/anirecorder/button/reload.tga", 
			"data/gui/menu/anirecorder/button/reload_down.tga", 
			"data/gui/menu/anirecorder/button/reload.tga", 
			"data/gui/menu/anirecorder/button/reload_disabled.tga", 
			ANIRECW_RELOAD);
		reloadBut->SetListener(this);
		reloadBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

		addFewTicksBut = ogui->CreateSimpleImageButton(win, 8+16+8+40*3, 80+24+16*7, 32, 32, 			
		"data/gui/menu/anirecorder/button/fewticks.tga", 
		"data/gui/menu/anirecorder/button/fewticks_down.tga", 
		"data/gui/menu/anirecorder/button/fewticks.tga", 
		"data/gui/menu/anirecorder/button/fewticks_disabled.tga", 
		ANIRECW_ADDFEWTICKS);
		addFewTicksBut->SetListener(this);
		addFewTicksBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

		smoothPositionBut = ogui->CreateSimpleImageButton(win, 8+16+8+40*4-2, 80+24+16*7, 32, 32, 
		"data/gui/menu/anirecorder/button/smoothposition.tga", 
		"data/gui/menu/anirecorder/button/smoothposition_down.tga", 
		"data/gui/menu/anirecorder/button/smoothposition.tga", 
		"data/gui/menu/anirecorder/button/smoothposition_disabled.tga", 
		ANIRECW_SMOOTHPOSITION);
		smoothPositionBut->SetListener(this);
		smoothPositionBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

		smoothRotationBut = ogui->CreateSimpleImageButton(win, 8+16+8+40*5-4, 80+24+16*7, 32, 32, 
		"data/gui/menu/anirecorder/button/smoothrotation.tga", 
		"data/gui/menu/anirecorder/button/smoothrotation_down.tga", 
		"data/gui/menu/anirecorder/button/smoothrotation.tga", 
		"data/gui/menu/anirecorder/button/smoothrotation_disabled.tga", 
		ANIRECW_SMOOTHROTATION);
		smoothRotationBut->SetListener(this);
		smoothRotationBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

		smoothAimBut = ogui->CreateSimpleImageButton(win, 8+16+8+40*6-6, 80+24+16*7, 32, 32, 
		"data/gui/menu/anirecorder/button/smoothaim.tga", 
		"data/gui/menu/anirecorder/button/smoothaim_down.tga", 
		"data/gui/menu/anirecorder/button/smoothaim.tga", 
		"data/gui/menu/anirecorder/button/smoothaim_disabled.tga", 
		ANIRECW_SMOOTHAIM);
		smoothAimBut->SetListener(this);
		smoothAimBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

		addAnimBut = ogui->CreateSimpleImageButton(win, 8+24+40*11, 80+24+16*7, 32, 32, 
		"data/gui/menu/anirecorder/button/addanim.tga", 
		"data/gui/menu/anirecorder/button/addanim_down.tga", 
		"data/gui/menu/anirecorder/button/addanim.tga", 
		"data/gui/menu/anirecorder/button/addanim_disabled.tga", 
		ANIRECW_ADDANIM);
		addAnimBut->SetListener(this);
		addAnimBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

		undoBut = ogui->CreateSimpleImageButton(win, 8+40*0, 80+24+8+16*9, 32, 32, 
		"data/gui/menu/anirecorder/button/undo.tga", 
		"data/gui/menu/anirecorder/button/undo_down.tga", 
		"data/gui/menu/anirecorder/button/undo.tga", 
		"data/gui/menu/anirecorder/button/undo_disabled.tga", 
		ANIRECW_UNDO);
		undoBut->SetListener(this);
		undoBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

		redoBut = ogui->CreateSimpleImageButton(win, 8+40*1, 80+24+8+16*9, 32, 32, 
		"data/gui/menu/anirecorder/button/redo.tga", 
		"data/gui/menu/anirecorder/button/redo_down.tga", 
		"data/gui/menu/anirecorder/button/redo.tga", 
		"data/gui/menu/anirecorder/button/redo_disabled.tga", 
		ANIRECW_REDO);
		redoBut->SetListener(this);
		redoBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

		deletePositionBut = ogui->CreateSimpleImageButton(win, 8+8+40*2, 80+24+8+16*9, 32, 32, 
		"data/gui/menu/anirecorder/button/deleteposition.tga", 
		"data/gui/menu/anirecorder/button/deleteposition_down.tga", 
		"data/gui/menu/anirecorder/button/deleteposition.tga", 
		"data/gui/menu/anirecorder/button/deleteposition_disabled.tga", 
		ANIRECW_DELETEPOSITION);
		deletePositionBut->SetListener(this);
		deletePositionBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

		addCommandsBut = ogui->CreateSimpleImageButton(win, 8+8+40*3, 80+24+8+16*9, 32, 32, 
		"data/gui/menu/anirecorder/button/commands.tga", 
		"data/gui/menu/anirecorder/button/commands_down.tga", 
		"data/gui/menu/anirecorder/button/commands.tga", 
		"data/gui/menu/anirecorder/button/commands_disabled.tga", 
		ANIRECW_ADDCOMMANDS);
		addCommandsBut->SetListener(this);
		addCommandsBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

		dropOnGroundBut = ogui->CreateSimpleImageButton(win, 8+8+40*4, 80+24+8+16*9, 32, 32, 
		"data/gui/menu/anirecorder/button/droponground.tga", 
		"data/gui/menu/anirecorder/button/droponground_down.tga", 
		"data/gui/menu/anirecorder/button/droponground.tga", 
		"data/gui/menu/anirecorder/button/droponground_disabled.tga", 
		ANIRECW_DROPONGROUND);
		dropOnGroundBut->SetListener(this);
		dropOnGroundBut->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);

#endif

    statusLabel = ogui->CreateTextArea(win, 8, 256+40-6, 512-16, 16, "Anirecorder opened");
    statusLabel->SetFont(smallFont);

	}


	void AniRecorderWindow::setMinimizedWindowMode(bool minimize)
	{
		if (this->minimized && !minimize)
		{
			this->minimized = false;

			win->Resize(512, 256+40+16);

			assert(cameraSelectList == NULL);
			assert(unitSelectList == NULL);

			this->sliderHeight = 80+16*6;

			createMiniDependantButtons();
		} 
		else if (!this->minimized && minimize) 
		{
			this->minimized = true;

			win->Resize(512, 100);

			assert(cameraSelectList != NULL);
			assert(unitSelectList != NULL);

			delete unitSelectList;
			unitSelectList = NULL;
			delete cameraSelectList;
			cameraSelectList = NULL;

			delete animationSelectList;
			animationSelectList = NULL;
			delete tickSelectList;
			tickSelectList = NULL;

			delete reloadBut;
			reloadBut = NULL;
			delete recordPathBut;
			recordPathBut = NULL;

			delete addAnimBut;
			addAnimBut = NULL;
			delete addCommandsBut;
			addCommandsBut = NULL;
			delete addFewTicksBut;
			addFewTicksBut = NULL;
			//delete addManyTicksBut;
			//addManyTicksBut = NULL;

			delete smoothPositionBut;
			smoothPositionBut = NULL;
			delete smoothRotationBut;
			smoothRotationBut = NULL;
			delete smoothAimBut;
			smoothAimBut = NULL;

			delete undoBut;
			undoBut = NULL;
			delete redoBut;
			redoBut = NULL;

			delete statusLabel;
			statusLabel = NULL;

			delete deletePositionBut;
			deletePositionBut = NULL;

			delete dropOnGroundBut;
			dropOnGroundBut = NULL;

			this->sliderHeight = 76;
		}

		positionBut->Move(12, this->sliderHeight);
		positionEndBut->Move(12, this->sliderHeight);
		positionEndBut->SetDisabled(true);
		sliderBut->Move(12, this->sliderHeight);
		timeLabel->Move(ANIRECW_SLIDER_WIDTH+16+8, this->sliderHeight);
	}

	void AniRecorderWindow::clearLists()
	{
		if (this->minimized)
			return;

		while (!cameraSelectList->isEmpty())
		{
			cameraSelectList->deleteItem();
		}
		while (!unitSelectList->isEmpty())
		{
			unitSelectList->deleteItem();
		}
	}

  void AniRecorderWindow::updateLists()
  {
		if (this->minimized)
			return;

		clearLists();

		LinkedList *units = this->aniRecorder->getUnitList();
		int unitnum = 0;
		while (!units->isEmpty())
		{
			game::Unit *u = (game::Unit *)units->popFirst();
			char idbuf[64+1];
			bool sel = false;
			if (this->selectedUnit == u)
				sel = true;
			if (u->getIdString() != NULL)
			{
				if (strlen(u->getIdString()) < 64)
				{
					strcpy(idbuf, u->getIdString());
				} else {
					strcpy(idbuf, "(id too long)");
				}
				unitSelectList->addItem(u->getIdString(), idbuf, sel);
			} else {
				int id = game->units->getIdForUnit(u);
				strcpy(idbuf, "Unit ID num ");
				strcat(idbuf, int2str(id));
				unitSelectList->addItem(int2str(id), idbuf, sel);
			}
			unitnum++;
		}
		delete units;

		LinkedList *cameras = this->aniRecorder->getCameraDumpList();
		//int camnum = 0;
		while (!cameras->isEmpty())
		{
			char *camnumstr = (char *)cameras->popFirst();
			int camnum = str2int(camnumstr);
			char camname[32];
			sprintf(camname, "Camera dump %d", camnum);
			bool sel = false;
			if (this->selectedCamera == camnum)
				sel = true;
			cameraSelectList->addItem(int2str(camnum), camname, sel);
			camnum++;
			delete [] camnumstr;
		}
		delete cameras;


  }

  void AniRecorderWindow::updateButtons()
  {
		if (this->aniRecorder->isRecording()
			|| this->aniRecorder->isPlaying())
		{
			if (!minimized)
			{
				this->addFewTicksBut->SetDisabled(true);
				//this->addManyTicksBut->SetDisabled(true);
				this->addAnimBut->SetDisabled(true);
				this->addCommandsBut->SetDisabled(true);
				this->smoothPositionBut->SetDisabled(true);
				this->smoothRotationBut->SetDisabled(true);
				this->smoothAimBut->SetDisabled(true);
				this->undoBut->SetDisabled(true);
				this->redoBut->SetDisabled(true);
				this->deletePositionBut->SetDisabled(true);
				this->dropOnGroundBut->SetDisabled(true);
			}

			this->removeUnitBut->SetDisabled(true);
			this->addUnitBut->SetDisabled(true);
			if (!minimized)
			{
				this->reloadBut->SetDisabled(true);
				this->recordPathBut->SetDisabled(true);
			}
		} else {
			if (!minimized)
			{
				this->reloadBut->SetDisabled(false);
				this->recordPathBut->SetDisabled(false);

				this->undoBut->SetDisabled(!this->aniRecorder->canUndo());
				this->redoBut->SetDisabled(!this->aniRecorder->canRedo());
			}

			if (this->selectedUnit != NULL
				&& recCounter == 0
				&& !minimized)
			{
				this->removeUnitBut->SetDisabled(false);

				//this->addManyTicksBut->SetDisabled(false);
				if (this->currentSelectedAnimation != NULL)
				{
					this->addAnimBut->SetDisabled(false);
				} else {
					this->addAnimBut->SetDisabled(true);
				}
				this->addCommandsBut->SetDisabled(false);
				if (this->currentSelectedTicks > 0)
				{
					this->smoothPositionBut->SetDisabled(false);
					this->smoothRotationBut->SetDisabled(false);
					this->smoothAimBut->SetDisabled(false);
					this->addFewTicksBut->SetDisabled(false);
				} else {
					this->smoothPositionBut->SetDisabled(true);
					this->smoothRotationBut->SetDisabled(true);
					this->smoothAimBut->SetDisabled(true);
					this->addFewTicksBut->SetDisabled(true);
				}
				//this->smoothWholeBut->SetDisabled(false);
				this->deletePositionBut->SetDisabled(false);
				this->dropOnGroundBut->SetDisabled(false);
			} else {
				this->removeUnitBut->SetDisabled(true);

				if (!minimized)
				{
					this->addFewTicksBut->SetDisabled(true);
					//this->addManyTicksBut->SetDisabled(true);
					this->addAnimBut->SetDisabled(true);
					this->addCommandsBut->SetDisabled(true);
					this->smoothPositionBut->SetDisabled(true);
					this->smoothRotationBut->SetDisabled(true);
					this->smoothAimBut->SetDisabled(true);
					this->deletePositionBut->SetDisabled(true);
					this->dropOnGroundBut->SetDisabled(true);
				}
			}
			if (game->unitSelections[game->singlePlayerNumber]->getUnitsSelected() == 0
				|| recCounter != 0
				|| minimized)
			{
				this->addUnitBut->SetDisabled(true);
			} else {
				this->addUnitBut->SetDisabled(false);
			}
		}

		//if (game->gameUI->getGameCamera()->getMode() == GameCamera::CAMERA_MODE_CAMERA_CENTRIC)
		//{
			cameraDumpBut->SetDisabled(false);
			if (this->selectedCamera != -1)
			{
				cameraTestBut->SetDisabled(false);
				cameraInterpBut->SetDisabled(false);
			} else {
				cameraTestBut->SetDisabled(true);
				cameraInterpBut->SetDisabled(true);
			}
		//} else {
		//	cameraDumpBut->SetDisabled(true);
		//	cameraTestBut->SetDisabled(true);
		//	cameraInterpBut->SetDisabled(true);
		//}
		if (this->selectedCamera != -1
			&& !minimized)
		{
			cameraDelBut->SetDisabled(false);
		} else {
			cameraDelBut->SetDisabled(true);
		}
		if (this->aniRecorder->isRecording()
			|| this->aniRecorder->isPlaying()
			|| recCounter != 0)
		{
			recBut->SetDisabled(true);
			playBut->SetDisabled(true);
		} else {
			if (this->selectedUnit != NULL)
			{
				this->recBut->SetDisabled(false);
			} else {
				this->recBut->SetDisabled(true);
			}
			playBut->SetDisabled(false);
		}

	}

  AniRecorderWindow::~AniRecorderWindow()
  {
		delete this->aniRecorder;


		// TODO: shit loads of stuff delete missing!!!!


    if (label1 != NULL)
    {
      delete label1;
    }
    if (cameraDumpBut != NULL)
    {
      delete cameraDumpBut;
    }
    if (closebut != NULL)
    {
      delete closebut;
    }
		if (font != NULL)
		{
			delete font;
			font = NULL;
		}
    if (win != NULL)
    {
      delete win;
      win = NULL;
    }
  }

  void AniRecorderWindow::run()
  {
		if (this->recCounter == 0)
		{
			this->aniRecorder->run();
		}

		int curPos = this->aniRecorder->getCurrentPosition();
		int curEndPos = this->aniRecorder->getCurrentEndPosition();
		int posLen = this->aniRecorder->getLongestAniLength();
		int pos = ANIRECW_SLIDER_WIDTH * curPos / posLen;
		int endPos = ANIRECW_SLIDER_WIDTH * curEndPos / posLen;

		if (sliderDown)
		{
			int cursorRelX = ogui->getCursorScreenX(0) - win->GetPositionX() - 12;
			// TODO: proper x position

			int seekTime = aniRecorder->getLongestAniLength() * cursorRelX / ANIRECW_SLIDER_WIDTH;
			if (seekTime < 0)
				seekTime = 0;
			if (seekTime > aniRecorder->getLongestAniLength())
				seekTime = aniRecorder->getLongestAniLength();

			if (!endSliderDown)
			{
				if (seekTime < curPos - 1 || seekTime > curPos + 1)
				{
					aniRecorder->seekToTime(seekTime);
					//aniRecorder->seekEndToTime(this->aniRecorder->getCurrentPosition());
					sliderStatus();
				}
			} else {
				if (seekTime < curEndPos - 1 || seekTime > curEndPos + 1)
				{
					aniRecorder->seekEndToTime(seekTime);
					sliderStatus();
				}				
			}
		}

		if (pos > ANIRECW_SLIDER_WIDTH) pos = ANIRECW_SLIDER_WIDTH;

		positionBut->Move(12 + pos-8, this->sliderHeight);
		positionEndBut->Move(12 + endPos-8, this->sliderHeight);

		if (aniRecorder->getCurrentEndPosition()
			> aniRecorder->getCurrentPosition())
		{
			positionEndBut->SetDisabled(false);
		} else {
			positionEndBut->SetDisabled(true);
		}

		if (assumingPlayingOrRecording)
		{
			if (!aniRecorder->isPlaying()
				&& !aniRecorder->isRecording())
			{
				assumingPlayingOrRecording = false;
				updateButtons();
			}
		} else {
			if (aniRecorder->isPlaying()
				|| aniRecorder->isRecording())
			{
				assumingPlayingOrRecording = true;
				updateButtons();
			}
		}

		char timebuf[32];
		int msec = (1000 * curPos) / GAME_TICKS_PER_SECOND;
		int min = (msec / 1000) / 60;
		int sec = (msec / 1000) % 60;
		int frac = (msec % 1000) / 10;
		sprintf(timebuf, "%.2d:%.2d.%.2d", min, sec, frac);
		timeLabel->SetText(timebuf);

    win->Raise();

		if (recCounter > 0)
		{
			if ((recCounter % GAME_TICKS_PER_SECOND) == 0)
			{
				char countbuf[4];
				strcpy(countbuf, int2str((recCounter / GAME_TICKS_PER_SECOND)));
				game->gameUI->clearGameMessage(game::GameUI::MESSAGE_TYPE_CENTER_BIG);
				game->gameUI->gameMessage(countbuf, NULL, 1, 1000, game::GameUI::MESSAGE_TYPE_CENTER_BIG);
			}
			if (recCounter == ANIRECW_RECORD_COUNT_TIME * GAME_TICKS_PER_SECOND)
			{
				aniRecorder->record(selectedUnit);
				updateButtons();
			}
			recCounter--;
			if (recCounter == 0)
			{
				game->setPaused(false);
				game->gameUI->clearGameMessage(game::GameUI::MESSAGE_TYPE_CENTER_BIG);
				updateButtons();
			}
		}

  }

  void AniRecorderWindow::CursorEvent(OguiButtonEvent *eve)
  {
    if (eve->eventType == OguiButtonEvent::EVENT_TYPE_LEAVE)
		{
			setStatusText("");
		}
    if (eve->eventType == OguiButtonEvent::EVENT_TYPE_OVER)
		{
			if (eve->triggerButton->GetId() == ANIRECW_CLOSE)
			{
				setStatusText("Close anirecorder");
			}
			if (eve->triggerButton->GetId() == ANIRECW_MINIMIZE)
			{
				setStatusText("Toggle minimized anirecorder");
			}
			if (eve->triggerButton->GetId() == ANIRECW_CAMERAMODE)
			{
				setStatusText("Change camera mode");
			}
			if (eve->triggerButton->GetId() == ANIRECW_CAMERADUMP)
			{
				setStatusText("Save current camera position");
			}
			if (eve->triggerButton->GetId() == ANIRECW_CAMERATEST)
			{
				setStatusText("Apply selected camera");
			}
			if (eve->triggerButton->GetId() == ANIRECW_CAMERAINTERP)
			{
				setStatusText("Interpolate from selected camera to current position");
			}
			if (eve->triggerButton->GetId() == ANIRECW_CAMERADELETE)
			{
				setStatusText("Delete selected camera");
			}
			if (eve->triggerButton->GetId() == ANIRECW_UNDO)
			{
				std::string tmp = std::string("Undo ") + aniRecorder->getUndoDesc();
				setStatusText(tmp.c_str());
			}
			if (eve->triggerButton->GetId() == ANIRECW_REDO)
			{
				std::string tmp = std::string("Redo ") + aniRecorder->getRedoDesc();
				setStatusText(tmp.c_str());
			}
			if (eve->triggerButton->GetId() == ANIRECW_ADDUNIT)
			{
				setStatusText("Add unit to ani batch");
			}
			if (eve->triggerButton->GetId() == ANIRECW_REMOVEUNIT)
			{
				setStatusText("Remove selected unit from ani batch");
			}
			if (eve->triggerButton->GetId() == ANIRECW_SMOOTHPOSITION)
			{
				setStatusText("Smooth movement at currently selected tick range");
			}
			if (eve->triggerButton->GetId() == ANIRECW_SMOOTHROTATION)
			{
				setStatusText("Smooth rotations at currently selected tick range");
			}
			if (eve->triggerButton->GetId() == ANIRECW_SMOOTHAIM)
			{
				setStatusText("Smooth aiming at currently selected tick range");
			}
			if (eve->triggerButton->GetId() == ANIRECW_ADDFEWTICKS)
			{
				setStatusText("Add ticks to currently selected tick position");
			}
			if (eve->triggerButton->GetId() == ANIRECW_DELETEPOSITION)
			{
				setStatusText("Delete currently selected tick range");
			}
			if (eve->triggerButton->GetId() == ANIRECW_DROPONGROUND)
			{
				setStatusText("Drop unit on ground at currently selected range");
			}
			if (eve->triggerButton->GetId() == ANIRECW_ADDCOMMANDS)
			{
				setStatusText("Add custom script commands to currently selected position");
			}
			if (eve->triggerButton->GetId() == ANIRECW_RECORDPATH)
			{
				setStatusText("Change anirecorder path");
			}
			if (eve->triggerButton->GetId() == ANIRECW_RELOAD)
			{
				setStatusText("Reload data (loses undo history)");
			}
			if (eve->triggerButton->GetId() == ANIRECW_ADDANIM)
			{
				setStatusText("Add animation to currently selected tick position");
			}
			if (eve->triggerButton->GetId() == ANIRECW_REC)
			{
				setStatusText("Record currently selected unit starting from current tick position");
			}
			if (eve->triggerButton->GetId() == ANIRECW_PLAY)
			{
				setStatusText("Play ani batch");
			}
			if (eve->triggerButton->GetId() == ANIRECW_PAUSE)
			{
				if (aniRecorder->isRecording())
					setStatusText("Pause recording");
				else
					setStatusText("Pause playing");
			}
			if (eve->triggerButton->GetId() == ANIRECW_REWIND)
			{
				setStatusText("Rewind ani batch to beginning");
			}
			if (eve->triggerButton->GetId() == ANIRECW_SLIDER)
			{
				sliderStatus();
			}
		}
    if (eve->eventType == OguiButtonEvent::EVENT_TYPE_CLICK)
		{
			if (eve->triggerButton->GetId() == ANIRECW_CLOSE)
			{
				game->gameUI->closeAniRecorderWindow(0);
				// NOTE: must return here, as triggerbutton gets deleted
				// above, any further calls to triggerButton->GetId will be invalid.
				return;
			}
			if (eve->triggerButton->GetId() == ANIRECW_CAMERAMODE)
			{
				if (game->gameUI->getGameCamera()->getMode() == GameCamera::CAMERA_MODE_CAMERA_CENTRIC)
				{
					game->gameUI->getGameCamera()->setMode(GameCamera::CAMERA_MODE_TARGET_CENTRIC);
				} else {
					game->gameUI->getGameCamera()->setMode(GameCamera::CAMERA_MODE_CAMERA_CENTRIC);
				}
			}
			if (eve->triggerButton->GetId() == ANIRECW_CAMERADUMP)
			{
				int newDump = aniRecorder->dumpCameraPosition();
				this->selectedCamera = newDump;
				updateLists();
			}
			if (eve->triggerButton->GetId() == ANIRECW_CAMERATEST)
			{
				if (this->selectedCamera != -1)
					aniRecorder->testCameraPosition(this->selectedCamera);
			}
			if (eve->triggerButton->GetId() == ANIRECW_CAMERAINTERP)
			{
				if (this->selectedCamera != -1)
					aniRecorder->interpolateCameraPosition(this->selectedCamera);
			}
			if (eve->triggerButton->GetId() == ANIRECW_CAMERADELETE)
			{
				const char *choices[2] = { "", "" };
				const char *imgs0[3] = { "Data/GUI/Buttons/quit_yes.tga", "Data/GUI/Buttons/quit_yes_down.tga", "Data/GUI/Buttons/quit_yes_highlight.tga" };
				const char *imgs1[3] = { "Data/GUI/Buttons/quit_no.tga", "Data/GUI/Buttons/quit_no_down.tga", "Data/GUI/Buttons/quit_no_highlight.tga" };
				const char **images[2];
				images[0] = imgs0;
				images[1] = imgs1;
				const char *text = "Delete selected camera?";
				cameraDelBox = new MessageBoxWindow(ogui, text,
					2, choices, images, 256, 92, 96, 27, "Data/GUI/Windows/quit_confirm.tga",
					MSGBOX_CAMERADELETE, this);
				cameraDelBox->setFadeoutTime(200);
			}

			if (eve->triggerButton->GetId() == ANIRECW_ADDUNIT)
			{
				LinkedList *ulist = game->units->getAllUnits();
				LinkedListIterator iter = LinkedListIterator(ulist);
				while (iter.iterateAvailable())
				{ 
					game::Unit *u = (game::Unit *)iter.iterateNext();
					if (u->isActive())
					{
						if (u->isSelected())
						{
							bool alreadyListed = false;
							LinkedList *listedUnits = this->aniRecorder->getUnitList();
							while (!listedUnits->isEmpty())
							{
								game::Unit *lu = (game::Unit *)listedUnits->popFirst();
								if (lu == u)
								{
									alreadyListed = true;
									break;
								}
							}
							if (!alreadyListed)
							{
								aniRecorder->addAniRecord(u);
							}
						}
					}
				}
				updateLists();
			}
			if (eve->triggerButton->GetId() == ANIRECW_REMOVEUNIT)
			{
				if (selectedUnit != NULL)
				{
					aniRecorder->removeAniRecord(selectedUnit);
					selectedUnit = NULL;
				}
				updateLists();
			}
			if (eve->triggerButton->GetId() == ANIRECW_PLAY)
			{
				aniRecorder->play();
			}
			if (eve->triggerButton->GetId() == ANIRECW_REC)
			{
				if (selectedUnit != NULL)
				{
					recCounter = ANIRECW_RECORD_COUNT_TIME * GAME_TICKS_PER_SECOND;
					game->setPaused(true);					
				}
			}
			if (eve->triggerButton->GetId() == ANIRECW_PAUSE)
			{
				aniRecorder->stop();
			}
			if (eve->triggerButton->GetId() == ANIRECW_REWIND)
			{
				aniRecorder->rewind();
			}
			if (eve->triggerButton->GetId() == ANIRECW_RELOAD)
			{
				aniRecorder->reload();
			}
			if (eve->triggerButton->GetId() == ANIRECW_ADDFEWTICKS)
			{
				if (this->selectedUnit != NULL)
				{
					if (currentSelectedTicks > 0)
					{
						std::string comment = std::string("// added ") + int2str(currentSelectedTicks * 15) + std::string(" msec ticks\r\n");
						std::string undodesc = std::string("add ") + int2str(currentSelectedTicks) + std::string(" ticks");
						undodesc += std::string(" (") + aniRecorder->getSliderPosOrRangeText() + ")";
						aniRecorder->addScriptCommand(this->selectedUnit, comment.c_str(), undodesc.c_str());
						for (int i = 0; i < currentSelectedTicks; i++)
						{
							aniRecorder->addScriptCommandContinued(this->selectedUnit, "aniTick\r\n");
						}
					}
				} else {
					Logger::getInstance()->warning("AniRecorderWindow::CursorEvent - Cannot \"add few ticks\" as no unit selected.");
				}
			}
			/*
			if (eve->triggerButton->GetId() == ANIRECW_ADDMANYTICKS)
			{
				if (this->selectedUnit != NULL)
				{
					aniRecorder->addScriptCommand(this->selectedUnit, "// added 1 sec ticks\r\n", "add 67 ticks");
					for (int i = 0; i < 67; i++)
					{
						aniRecorder->addScriptCommandContinued(this->selectedUnit, "aniTick\r\n");
					}
				} else {
					Logger::getInstance()->warning("AniRecorderWindow::CursorEvent - Cannot \"add many ticks\" as no unit selected.");
				}
			}
			*/
			if (eve->triggerButton->GetId() == ANIRECW_ADDANIM)
			{
				if (this->selectedUnit != NULL)
				{
					if (currentSelectedAnimation != NULL)
					{
						std::string descstr = std::string("add animation ") + currentSelectedAnimation;
						aniRecorder->addScriptCommand(this->selectedUnit, "// added animation\r\n", descstr.c_str());
						char tmpbuf[256];
						sprintf(tmpbuf, "aniAnim %s\r\n", this->currentSelectedAnimation);
						aniRecorder->addScriptCommandContinued(this->selectedUnit, tmpbuf);
					}
				} else {
					Logger::getInstance()->warning("AniRecorderWindow::CursorEvent - Cannot \"add anim\" as no unit selected.");
				}
			}
			if (eve->triggerButton->GetId() == ANIRECW_ADDCOMMANDS)
			{
				game->gameUI->showConsole();
				game->gameUI->getConsole()->setMiniQueryMode();
				game->gameUI->getConsole()->setLine("addAniScriptCommands ");
			}
			if (eve->triggerButton->GetId() == ANIRECW_SMOOTHPOSITION)
			{
				if (this->selectedUnit != NULL)
				{
					if (currentSelectedTicks > 0)
					{
						this->aniRecorder->smoothPosition(this->selectedUnit, currentSelectedTicks);
					}
				}				
			}
			if (eve->triggerButton->GetId() == ANIRECW_SMOOTHROTATION)
			{
				if (this->selectedUnit != NULL)
				{
					if (currentSelectedTicks > 0)
					{
						this->aniRecorder->smoothRotation(this->selectedUnit, currentSelectedTicks);
					}
				}				
			}
			if (eve->triggerButton->GetId() == ANIRECW_SMOOTHAIM)
			{
				if (this->selectedUnit != NULL)
				{
					if (currentSelectedTicks > 0)
					{
						this->aniRecorder->smoothAim(this->selectedUnit, currentSelectedTicks);
					}
				}				
			}
			if (eve->triggerButton->GetId() == ANIRECW_DELETEPOSITION)
			{
				if (this->selectedUnit != NULL)
				{
					this->aniRecorder->deletePosition(this->selectedUnit);
				} else {
					Logger::getInstance()->warning("AniRecorderWindow::CursorEvent - Cannot \"delete position\" as no unit selected.");
				}
			}
			if (eve->triggerButton->GetId() == ANIRECW_DROPONGROUND)
			{
				if (this->selectedUnit != NULL)
				{
					this->aniRecorder->dropOnGround(this->selectedUnit);
				} else {
					Logger::getInstance()->warning("AniRecorderWindow::CursorEvent - Cannot \"drop unit on ground\" as no unit selected.");
				}
			}
			if (eve->triggerButton->GetId() == ANIRECW_RECORDPATH)
			{
				game->gameUI->showConsole();
				game->gameUI->getConsole()->setMiniQueryMode();
				game->gameUI->getConsole()->setLine("setAniRecordPath Data/Missions/");
			}
			if (eve->triggerButton->GetId() == ANIRECW_UNDO)
			{
				if (this->aniRecorder->canUndo())
				{
					this->aniRecorder->undo();
					updateLists();
				}
				if (this->aniRecorder->canRedo())
				{
					std::string tmp = std::string("Undo ") + aniRecorder->getUndoDesc();
					setStatusText(tmp.c_str());
				} else {
					setStatusText("");
				}
			}
			if (eve->triggerButton->GetId() == ANIRECW_REDO)
			{
				if (this->aniRecorder->canRedo())
				{
					this->aniRecorder->redo();
					updateLists();
				}
				if (this->aniRecorder->canRedo())
				{
					std::string tmp = std::string("Redo ") + aniRecorder->getRedoDesc();
					setStatusText(tmp.c_str());
				} else {
					setStatusText("");
				}
			}
			if (eve->triggerButton->GetId() == ANIRECW_MINIMIZE)
			{
				if (this->minimized)
					this->setMinimizedWindowMode(false);
				else
					this->setMinimizedWindowMode(true);
				updateLists();
			}
		}
		if (eve->eventType == OguiButtonEvent::EVENT_TYPE_CLICK
			|| eve->eventType == OguiButtonEvent::EVENT_TYPE_OUT)
		{
			sliderDown = false;
			endSliderDown = false;
			if (eve->triggerButton->GetId() == ANIRECW_SLIDER)
			{
				sliderStatus();
			}
		}
		if (eve->eventType == OguiButtonEvent::EVENT_TYPE_PRESS)
		{
			if (eve->triggerButton->GetId() == ANIRECW_SLIDER)
			{
				sliderDown = true;
				if ((eve->cursorButtonMask & OGUI_BUTTON_2_MASK) != 0
					|| (eve->cursorOldButtonMask & OGUI_BUTTON_2_MASK) != 0)
				{
					endSliderDown = true;
				} else {
					endSliderDown = false;
				}
			}
		}

		updateButtons();
  }

	void AniRecorderWindow::SelectEvent(OguiSelectListEvent *eve)
	{
		if (eve->eventType == OguiSelectListEvent::EVENT_TYPE_SCROLL)
		{
			if (eve->triggerSelectList == animationSelectList)
			{
				assert(animationSelectList != NULL);
				for (int i = 1; i < ANIM_AMOUNT; i++)
				{
					animationSelectList->setSelected(i-1, false);
				}
				if (this->currentSelectedAnimation != NULL)
				{
					delete [] this->currentSelectedAnimation;
					this->currentSelectedAnimation = NULL;
				}
			}
			if (eve->triggerSelectList == tickSelectList)
			{
				assert(tickSelectList != NULL);
				for (int i = 0; i < 14; i++)
				{
					tickSelectList->setSelected(i, false);
				}
				this->currentSelectedTicks = 0;
			}
		}
		if (eve->eventType == OguiSelectListEvent::EVENT_TYPE_DOUBLESELECT)
		{
			if (eve->triggerSelectList == cameraSelectList)
			{
				this->selectedCamera = -1;
				this->cameraSelectList->setSelected(eve->selectionNumber, false);
			}
			if (eve->triggerSelectList == unitSelectList)
			{
				this->selectedUnit = NULL;
				this->unitSelectList->setSelected(eve->selectionNumber, false);
			}
		}
		if (eve->eventType == OguiSelectListEvent::EVENT_TYPE_SELECT)
		{
			if (eve->triggerSelectList == animationSelectList
				&& animationSelectList != NULL)
			{
				char *val = eve->selectionValue;
				if (val != NULL)
				{
					if (this->currentSelectedAnimation != NULL)
					{
						delete [] this->currentSelectedAnimation;
						this->currentSelectedAnimation = NULL;
					}
					this->currentSelectedAnimation = new char[strlen(val) + 1];
					strcpy(this->currentSelectedAnimation, val);
				}
			}
			if (eve->triggerSelectList == tickSelectList
				&& tickSelectList != NULL)
			{
				char *val = eve->selectionValue;
				if (val != NULL)
				{
					this->currentSelectedTicks = str2int(val);
				}
			}
			if (eve->triggerSelectList == cameraSelectList)
			{
				char *val = eve->selectionValue;
				if (val != NULL)
				{
					this->selectedCamera = str2int(val);
				}
			}
			if (eve->triggerSelectList == unitSelectList)
			{
				char *val = eve->selectionValue;
				if (val != NULL)
				{
					game::Unit *prevUnit = this->selectedUnit;
					if (this->selectedUnit != NULL)
					{
						// NEW: make sure that the unit does not start shooting around... ---
						this->selectedUnit->targeting.clearTarget();
						// end of NEW ---

						if (this->selectedUnit->getOwner() == game->singlePlayerNumber)
							game->unitSelections[game->singlePlayerNumber]->selectUnit(this->selectedUnit, false);
					}
					if (str2int(val) != 0)
					{
						this->selectedUnit = game->units->getUnitById(str2int(val));
					} else {
						this->selectedUnit = game->units->getUnitByIdString(val);
					}
					if (this->selectedUnit != NULL)
					{
						// NEW: "devSideSwap" ---
						game::Unit *currUnit = this->selectedUnit;

						if (this->selectedUnit->getOwner() != game->singlePlayerNumber)
						{
							game->unitSelections[game->singlePlayerNumber]->selectAllUnits(false);

							game->gameUI->closeCombatWindow(game->singlePlayerNumber);
							game->singlePlayerNumber = currUnit->getOwner();
							game->gameUI->openCombatWindow(game->singlePlayerNumber);
						}
						if (game->gameUI->getFirstPerson(0) == prevUnit
							&& prevUnit != NULL)
						{
							game->gameUI->setFirstPerson(game->singlePlayerNumber, currUnit, 0);
						}

						this->selectedUnit = currUnit;
						// end of NEW ---

						if (this->selectedUnit->getOwner() == game->singlePlayerNumber)
							game->unitSelections[game->singlePlayerNumber]->selectUnit(this->selectedUnit, true);
					}

					// (just a little check to make sure these actually stay synced?)
					// there should be no need for this.
					updateUnitSelections();
				}
			}
		}
		updateButtons();
	}


	void AniRecorderWindow::updateUnitSelections()
	{
		LinkedList *ulist = game->units->getAllUnits();
		LinkedListIterator iter(ulist);
		bool unitWasSelected = false;
		while (iter.iterateAvailable())
		{
			game::Unit *u = (game::Unit *)iter.iterateNext();
			if (u->isSelected())
			{
				this->selectedUnit = u;
				unitWasSelected = true;
				break;
			}
		}
		if (!unitWasSelected)
		{
			this->selectedUnit = NULL;
		}

		if (this->selectedUnit != NULL)
		{
			int selectionNumber = 0;
			bool selectionOk = false;
			LinkedList *units = this->aniRecorder->getUnitList();
			int unitnum = 0;
			while (!units->isEmpty())
			{
				game::Unit *u = (game::Unit *)units->popFirst();
				if (this->selectedUnit == u)
				{
					selectionOk = true;
					selectionNumber = unitnum;
					this->unitSelectList->setSelected(unitnum, true);
				} else {
					this->unitSelectList->setSelected(unitnum, false);
				}
				unitnum++;
			}
			delete units;
			if (!selectionOk)
			{
				// oops... (user selected a unit that is not in anirecorder)?
				this->selectedUnit = false;
			//} else {
				//this->unitSelectList->setSelected(selectionNumber, true);
			}
		}

		updateButtons();
	}


	void AniRecorderWindow::reload()
	{
		aniRecorder->reload();
		updateLists();
		updateButtons();

		char labelbuf[128];
		sprintf(labelbuf, "Ani recorder - %s", this->aniRecorder->getCurrentAniId());
		label1->SetText(labelbuf);
	}

	void AniRecorderWindow::messageBoxClosed(MessageBoxWindow *msgbox, int id, int choice)
	{
		if (id == MSGBOX_CAMERADELETE)
		{
			if (msgbox == cameraDelBox)
			{
				cameraDelBox = NULL;
			}
			if (choice == 0) 
			{
				aniRecorder->deleteCameraDump(this->selectedCamera);
				this->selectedCamera = -1;
				updateLists();
				updateButtons();
			}
		}
		// NOTE: deletes the object that called this method!
		delete msgbox;
	}

	void AniRecorderWindow::addAniScriptCommands(const char *scriptCommands)
	{
		assert(scriptCommands != NULL);

		if (this->selectedUnit != NULL)
		{
			std::string undodesc = std::string("custom script commands");
			undodesc += std::string(" (") + aniRecorder->getSliderPosOrRangeText() + ")";
			std::string cmdbuf = scriptCommands;
			cmdbuf += "\r\n";
			aniRecorder->addScriptCommand(this->selectedUnit, cmdbuf.c_str(), undodesc.c_str());
		}
	}

	void AniRecorderWindow::setStatusText(const char *status)
	{
		if (statusLabel != NULL)
		{
			statusLabel->SetText(status);
		}
	}


	void AniRecorderWindow::sliderStatus()
	{
		if (aniRecorder->getCurrentEndPosition() > aniRecorder->getCurrentPosition())
		{
			std::string tmp = std::string("Selected tick range: ");
			tmp += aniRecorder->getSliderPosOrRangeText();
			setStatusText(tmp.c_str());
		} else {
			std::string tmp = std::string("Selected tick position: ");
			tmp += aniRecorder->getSliderPosOrRangeText();
			setStatusText(tmp.c_str());
		}
	}

}

