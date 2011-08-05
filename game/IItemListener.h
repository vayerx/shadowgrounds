#ifndef INC_IITEMLISTENER_H
#define INC_IITEMLISTENER_H

namespace game {

class Item;

class IItemListener
{
public:
	virtual ~IItemListener() { }

	virtual void onDestruction( Item* item ) = 0;
};

} // end of namespace game


#endif
