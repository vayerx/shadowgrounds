
#ifndef OGUISELECTLIST_H
#define OGUISELECTLIST_H

#include "IOguiButtonListener.h"
#include "IOguiSelectListListener.h"
#include "OguiSelectListStyle.h"
#include "OguiSelectListEvent.h"
#include "OguiButton.h"

//
// Select list 
//
// v1.0.0 - 10.4.2002 - jpkokkon
//


class OguiWindow;


class OguiSelectList : public IOguiButtonListener
{
  public:
    // never try to use the constructor directly, use ogui to contruct these
    OguiSelectList(int x, int y, int defsel, bool multisel, int amount, const char **values, const char **descs, OguiButton **listButs, OguiButton *upBut, OguiButton *downBut, OguiSelectListStyle *style, int id, void *argument);

    // oldstuff... dummy constructor / destructor
    //OguiSelectList();

    ~OguiSelectList();

    // sets the listener object
    void setListener(IOguiSelectListListener *listener);

    // set style for this select list, don't delete the given object as
    // this does not make a copy of it, but keeps a pointer reference to it.
    // you should delete the style object once the select list has been deleted
    void setStyle(OguiSelectListStyle *style);

    // old stuff...
    //void click();

    // calling these explicitly won't cause an event (listener will not get
    // a scroll event) ... it should perhaps, though...
    void scrollUp();
    void scrollDown();
    void scrollTo(int offset);

    // there should be no need to call this anywhere...
    void refresh();

    // return the id number of the select list
    int getId();

    // set's selection status
    void setSelected(int position, bool selected);

    // add an item to the list
		// position -1 can be used to add after last item (0 adds before first item)
		// WARNING: calling this inside a select event may invalidate value pointers 
		// given to event listener
    void addItem(const char *value, const char *desc, bool selected = false, 
      int position = -1);

	// added by Pete
	// 
	void highlightItem( int position, bool highlight );

    // add an item to the list
    void deleteItem(int position = -1);

    // returns true if no items in this list
    bool isEmpty();

    // part of internal implementation
    void CursorEvent(OguiButtonEvent *eve);

		OguiButton *getUpScrollBut() { return upScrollBut; }
		OguiButton *getDownScrollBut() { return downScrollBut; }

  protected:
    // this is here to make things easier for extending classes (FileList)
    void init(int x, int y, int defsel, bool multisel, int amount, const char **values, const char **descs, OguiButton **listButs, OguiButton *upBut, OguiButton *downBut, OguiSelectListStyle *style, int id, void *argument);

  private:
    void uninit(); // called by destructor

  protected:
 
    OguiSelectListStyle *style;

    OguiWindow *parent;
    OguiButton **listButs;
    OguiButton *upScrollBut;
    OguiButton *downScrollBut;

    IOguiImage *selectedImage;
    IOguiImage *selectedDownImage;
    IOguiImage *selectedDisabledImage;
    IOguiImage *unselectedImage;
    IOguiImage *unselectedDownImage;
    IOguiImage *unselectedDisabledImage;

    int listButAmount;
    int x;
    int y;
    int sizex;
    int sizey;
    int scaleY;
    int scrollSizeX;
    int scrollSizeY;
    IOguiSelectListListener *listener;
    int amount;
    int allocedAmount;
    bool multiSelectable;
    bool *selected;
	bool *highlighted;
	bool highlightNew;
    int scrolly;
    char **values;
    char **descs;
    void *argument;

    int id;
};


#endif

