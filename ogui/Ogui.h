
#ifndef OGUI_H
#define OGUI_H

#ifdef _MSC_VER
// VC whines about "throw(xxx)" exception specifications
#pragma warning(disable : 4290)
#endif

//#include <c2_sptr.h>

#include <list>

#include "OguiException.h"
#include "IOguiDriver.h"
#include "IOguiFont.h"
#include "IOguiImage.h"

#include "OguiButtonStyle.h"
#include "OguiSelectListStyle.h"
#include "OguiWindow.h"
#include "OguiButton.h"
#include "OguiTextLabel.h"
#include "OguiSelectList.h"

#define OGUI_CURSOR_CTRL_MOUSE 1							// Controlled by every mouse.
#define OGUI_CURSOR_CTRL_KEYBOARD1 2
#define OGUI_CURSOR_CTRL_KEYBOARD2 4
#define OGUI_CURSOR_CTRL_JOYSTICK1 8
#define OGUI_CURSOR_CTRL_JOYSTICK2 16
#define OGUI_CURSOR_CTRL_JOYSTICK3 32
#define OGUI_CURSOR_CTRL_JOYSTICK4 64
#define OGUI_CURSOR_CTRL_DISABLE_JOYSTICK_WARP 128 // (ability to jump from button to another with joystick)
#define OGUI_CURSOR_CTRL_MOUSE0 (128 << 1)				// Controlled by mouse #0
#define OGUI_CURSOR_CTRL_MOUSE1 (128 << 2)				// Controlled by mouse #1
#define OGUI_CURSOR_CTRL_MOUSE2 (128 << 3)				// Controlled by mouse #2
#define OGUI_CURSOR_CTRL_MOUSE3 (128 << 4)				// Controlled by mouse #3
#define OGUI_CURSOR_CTRL_MOUSE4 (128 << 5)				// Controlled by mouse #4


#define OGUI_BUTTON_1_MASK 1
#define OGUI_BUTTON_2_MASK 2
#define OGUI_BUTTON_3_MASK 4
#define OGUI_BUTTON_4_MASK 8
#define OGUI_BUTTON_5_MASK 16
#define OGUI_BUTTON_6_MASK 32
#define OGUI_BUTTON_7_MASK 64
#define OGUI_BUTTON_WHEEL_UP_MASK 256
#define OGUI_BUTTON_WHEEL_DOWN_MASK 512


#define OGUI_SCALE_MULTIPLIER 1024

class LinkedList;
class IStorm3D_StreamBuilder;
class IStorm3D_VideoStreamer;

/**
 * C++ Wrapper class for orvgui
 *
 * Use this instead of the orvgui C code port
 * (Include this, do not include orvgui2.h)
 *
 * @author Jukka Kokkonen <jukka@frozenbyte.com>
 * @version v1.0.1, 1.7.2002
 */

class Ogui
{
public:
  Ogui();
  ~Ogui();

  /**
   * Set the driver.
   * Currently the driver is really just a bit more than a dummy class.
   * The implementation is hard coded to orvgui. This driver architecture
   * makes it easier to change the implementation without touching API.
   */
  void SetDriver(IOguiDriver *drv);

	void UpdateCursorPositions();

  /** 
   * Set the render scale. 
   * Values are relative to OGUI_SCALE_MULTIPLIER.
   * (fixed point numbers where 1024 equals 1, and 1 equals 1/1024)
   */
  void SetScale(int scaleX, int scaleY);

  int GetScaleX();

  int GetScaleY();

	void setHotkeysEnabled(bool enabled);

  /**
   * Set the mouse sensitivity.
   */
  void SetMouseSensitivity(float sensitivityX, float sensitivityY);

  /**
   * Sets the gui to skip cursor movements on next run.
   */
  void skipCursorMovement();

  /** 
   * Initialize (call after setting the driver).
   */
  void Init();

  /**
   * Uninitialize (call before deleting an Ogui instance).
   */
  void Uninit();

	/**
	 * Resets data (next storm generation)
	 */
	void ResetData();

  /**
   * Loads the default font used for texts unless otherwise specified.
   */
  void LoadDefaultFont(const char *filename) throw (OguiException *);

  /** 
   * Returns maximum number of cursors (numbers are from 0 to returnvalue-1).
   */
  int GetMaxCursors();

  /**
   * Set the control devices for given cursor.
   */
  void SetCursorController(int cursornum, int controllermask)
    throw (OguiException *);

	int GetCursorController(int cursornum);

  /**
   * Set the control device for given cursor to keyboard with given keys.
   */
  void SetCursorControllerKeyboard(int cursornum, int key_left, int key_right, int key_up, int key_down, int key_fire, int key_fire2)
    throw (OguiException *);

  /**
   * Set the image used for given cursor. Loads the image to memory.
   * Any previous image loaded set will be deleted from memory.
   * @param cursornum  int, the cursor number for which this image will be used.
   * @param filename  const char*, image file name. As for current implementation
   * should be a .tga file with alpha channel.
   * @param forstate  int, the cursor image state for which this image will be used.
   * Defaults to 0.
   */
  void LoadCursorImage(int cursornum, const char *filename, int forstate = 0)
    throw (OguiException *);

  /**
   * Select a state (one of loaded images) for cursor.
   */
  void SetCursorImageState(int cursornum, int state)
    throw (OguiException *);

	void SwapCursorImages(int cursor1, int cursor2);
	void ResetSwappedCursorImages();

  /**
   * Change cursor image offset for given cursor and state.
   */
  void SetCursorImageOffset(int cursornum, int offsetX, int offsetY, 
    int forstate = 0)
    throw (OguiException *);

  /**
   * Get cursor X-coordinate relative to screen.
   * Usually should not be used, for special cases only.
   */
  int getCursorScreenX(int cursornum, bool exact = false);

  /**
   * Get cursor Y-coordinate relative to screen.
   * Usually should not be used, for special cases only.
   */
  int getCursorScreenY(int cursornum, bool exact = false);

  /**
   * Set cursor X-coordinate relative to screen.
   * Usually should not be used, for special cases only.
   */
  void setCursorScreenX(int cursornum, int screenX);

  /**
   * Set cursor Y-coordinate relative to screen.
   * Usually should not be used, for special cases only.
   */
  void setCursorScreenY(int cursornum, int screenY);

  /**
   * Set cursor X-coordinate movement to next frame.
   */
  void setCursorScreenOffsetX(int cursornum, int screenOffsetX);

  /**
   * Set cursor Y-coordinate movement to next frame.
   */
  void setCursorScreenOffsetY(int cursornum, int screenOffsetY);

	/**
	 * Set menu index mode enabled or disabled.
	 * (While in menu index mode, joystick will make the cursor warp 
	 * from one button to another)
	 */
	void SetMenuIndexMode(int cursor, bool menuIndexModeEnabled);

  // returns true if user is pressing escape 
  // if a window is set "onlyactive", always returns false, in such a case 
  // the only active window should use a keyreader if wants to be escaped.
  bool isEscape();

  // TODO, currently affected by a #define
  //void SetRaiseWindowOnClick(bool value);
  // TODO, currently affected by a #define
  //void SetReactTopmostWindowOnly(bool value);
  // TODO, currently affected by a #define
  //void SetHideCursors(bool value);
  // TODO, currently affected by a #define
  //void SetMoveWithButton3(bool value);
  // TODO, currently affected by a #define
  //void SetMoveFromTop(bool value);
  // TODO, currently affected by a #define
  //void SetMoveTopHeight(int height);

  // Create a simple window loading given background image
  OguiWindow *CreateSimpleWindow(int x, int y, int sizex, int sizey, 
    const char *imagefilename, int id = 0) throw (OguiException *);

  // Create a simple image button
  OguiButton *CreateSimpleImageButton(OguiWindow *win, int x, int y, int sizex, int sizey, 
    const char *imageFilename, const char *imageDownFilename, const char *imageHighlightFilename,
    int id = 0, void *argument = NULL) throw (OguiException *);

  // Create a simple image button w/ disabled image
  OguiButton *CreateSimpleImageButton(OguiWindow *win, int x, int y, int sizex, int sizey, 
    const char *imageFilename, const char *imageDownFilename, const char *imageHighlightFilename, const char *imageDisabledFilename,
    int id = 0, void *argument = NULL, bool clipToWindow = true ) throw (OguiException *);

  // Create a simple image+text button
  OguiButton *CreateSimpleTextButton(OguiWindow *win, int x, int y, int sizex, int sizey, 
    const char *imageFilename, const char *imageDownFilename, const char *imageHighlightFilename, 
    const char *text, int id = 0, const void *argument = NULL, bool clipToWindow = true ) throw (OguiException *);

  // Create a text label
  OguiTextLabel *CreateTextLabel(OguiWindow *win, int x, int y, 
    int sizex, int sizey, const char *text) throw (OguiException *);

  // Create a text area (which wraps text automatically to multiple lines)
  OguiTextLabel *CreateTextArea(OguiWindow *win, int x, int y, 
    int sizex, int sizey, const char *text) throw (OguiException *);

  // Create a select list 
  // Works fine as a button/checkbutton/radiobutton group too...
  // (Just use proper images and argument values to make them)
  OguiSelectList *CreateSelectList(OguiWindow *win, int x, int y, 
    OguiSelectListStyle *style, int valueAmount, const char **values, const char **descs, 
    bool multiSelectable = false, int defaultSelection = -1, int id = 0,
    void *argument = NULL) throw (OguiException *);

  /**
   * Loads an image.
   * You should take care of the deletion of the returned image.
   */
  IOguiImage *LoadOguiImage(const char *filename) throw (OguiException *);
  IOguiImage *LoadOguiImage(int width, int height) throw (OguiException *);
  IOguiImage *GetOguiRenderTarget(int index) throw (OguiException *);
  IOguiImage *LoadOguiVideo( const char* filename, IStorm3D_StreamBuilder *streamBuilder);
  IOguiImage *ConvertVideoToImage( IStorm3D_VideoStreamer* stream, IStorm3D_StreamBuilder *streamBuilder );

  /**
   * Loads a font.
   * You should take care of the deletion of the returned font.
   */
  IOguiFont *LoadFont(const char *filename) throw (OguiException *);

  /** 
   * Run Ogui.
   */
  void Run(int timeDelta);

  // show the error message / exception in ogui error window
  void ShowError(char *msg);
  void ShowError(OguiException *ex); // is this very wise ?

	void setVisualizeWindows(bool debugVisualize);

	// returns true size of screen
	int getScreenSizeX();
	int getScreenSizeY();

private:
	IOguiDriver *drv;
	IOguiFont *defaultFont;
	IOguiImage ***cursorImages;
	int **cursorOffsetX;
	int **cursorOffsetY;

	std::vector< std::pair<int, int> > swappedCursorImages;

	std::list<OguiButton*> buttons;
	std::list<OguiWindow*> windows;

	friend class OguiWindow;
	friend class OguiButton;

	void UpdateEffects(int timeDelta);

	void RemovedWindow(OguiWindow *win);
	void RemovedButton(OguiButton *but);

};

#endif
