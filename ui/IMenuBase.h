#ifndef INC_IMENUBASE_H
#define INC_IMENUBASE_H

namespace ui {


// Interface for the menu collection menus
class IMenuBase
{
public:
	IMenuBase() { }
	virtual ~IMenuBase() { }
	
	virtual int  getType() const = 0;

	virtual void hide() = 0;
	virtual void show() = 0;
	virtual void raise() = 0;
	virtual bool isVisible() const = 0;
	virtual bool wasQuitPressed() const = 0;

	virtual void update() = 0;

	virtual void openMenu( int menu ) = 0;
	virtual void closeMenu() = 0;
	virtual void applyChanges() = 0;

	virtual void escPressed() = 0;
};

}

#endif
