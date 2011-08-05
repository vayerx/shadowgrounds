
#include "precompiled.h"

#include "Item.h"
#include "ProgressBar.h"

#include "../ui/VisualObject.h"
#include "../ui/Spotlight.h"

#include "../util/fb_assert.h"
#include "../util/Debug_MemoryManager.h"

namespace game
{
	Item::Item(int itemTypeId) :
		physicsObject( NULL )
	{
		position = VC3(0,0,0);
		visualObject = NULL;
		halo = NULL;
		weaponType = false;
		this->itemTypeId = itemTypeId;
		pickupDelay = 0;
		count = 1;
		blinking = false; // changed by Pete
		enabled = true;
		reEnableTime = 0;
		customTipText = NULL;
		customScript = NULL;
		specialString = NULL;
		progress = NULL;

		highlightStyle = -1;

		spawner = NULL;
	}
	
	Item::~Item()
	{
		if(this->progress)
		{
			delete this->progress;
		}
		setCustomTipText(NULL);
		setCustomScript(NULL);
		setSpecialString(NULL);
		if (visualObject != NULL)
		{
			delete visualObject;
			visualObject = NULL;
		}
		if (halo != NULL)
		{
			delete halo;
			halo = NULL;
		}
	}


	const char *Item::getCustomScript() const
	{
		return customScript;
	}

	void Item::setCustomScript(const char *customScript)
	{
		if (this->customScript != NULL)
		{
			delete [] this->customScript;
			this->customScript = NULL;
		}
		if (customScript != NULL)
		{
			this->customScript = new char[strlen(customScript) + 1];
			strcpy(this->customScript, customScript);
		}
	}

	const char *Item::getCustomTipText() const
	{
		return customTipText;
	}

	void Item::setCustomTipText(const char *customTipText)
	{
		if (this->customTipText != NULL)
		{
			delete [] this->customTipText;
			this->customTipText = NULL;
		}
		if (customTipText != NULL)
		{
			this->customTipText = new char[strlen(customTipText) + 1];
			strcpy(this->customTipText, customTipText);
		}
	}

	const char *Item::getSpecialString() const
	{
		return specialString;
	}

	void Item::setSpecialString(const char *specialString)
	{
		if (this->specialString != NULL)
		{
			delete [] this->specialString;
			this->specialString = NULL;
		}
		if (specialString != NULL)
		{
			this->specialString = new char[strlen(specialString) + 1];
			strcpy(this->specialString, specialString);
		}
	}


	void Item::prepareForRender()
	{
		if (visualObject != NULL)
		{
			visualObject->prepareForRender();
		}
		if (halo != NULL)
		{
			halo->prepareForRender();
		}
	}

	void Item::setPosition(const VC3 &position)
	{
		this->position = position;
		if (visualObject != NULL)
		{
			VC3 vopos = position;
			if (weaponType)
			{
				vopos.x -= 0.3f;
				vopos.y += 0.1f;
			}
			visualObject->setPosition(vopos);
		}
		if (halo != NULL)
		{
			halo->setPosition(this->position);
		}
	}

	void Item::setRotation(const VC3 &rotation)
	{
		this->rotation = rotation;
		if (visualObject != NULL)
		{
			visualObject->setRotation(rotation.x, rotation.y, rotation.z);
		}		
	}

	const VC3 &Item::getPosition()
	{
		return this->position;
	}

	const VC3 &Item::getRotation()
	{
		return this->rotation;
	}

	void Item::setVisualObject(ui::VisualObject *visualObject)
	{
		fb_assert(this->visualObject == NULL || visualObject == NULL);
		this->visualObject = visualObject;
	}

	ui::VisualObject *Item::getVisualObject()
	{
		return this->visualObject;
	}

	void Item::setHalo(ui::Spotlight *halo)
	{
		fb_assert(this->halo == NULL);
		this->halo = halo;
	}

	void Item::setWeaponType(bool weaponType)
	{
		this->weaponType = weaponType;
	}

	int Item::getItemTypeId()
	{
		return itemTypeId;
	}

	int Item::getPickupDelay()
	{
		return pickupDelay;
	}

	void Item::setPickupDelay(int pickupDelay)
	{
		this->pickupDelay = pickupDelay;
	}

	int Item::getCount()
	{
		return this->count;
	}

	void Item::addCount()
	{
		this->count++;
	}

	void Item::decreaseCount()
	{
		fb_assert(count > 1);
		this->count--;
	}

	void Item::setCount(int count)
	{
		fb_assert(count > 0);
		this->count = count;
	}

	void Item::setBlinking(bool blinking)
	{
		this->blinking = blinking;
	}

	bool Item::isBlinking()
	{
		return this->blinking;
	}

	void Item::setEnabled(bool enabled)
	{
		this->enabled = enabled;
	}

	bool Item::isEnabled() const
	{
		return this->enabled;
	}

	void Item::setReEnableTime(int reEnableTime)
	{
		this->reEnableTime = reEnableTime;
	}

	bool Item::advanceReEnable()
	{
		if (!this->enabled)
		{
			if (this->reEnableTime > 0)
			{
				this->reEnableTime--;
				if (this->reEnableTime == 0)
				{
					this->enabled = true;
					return true;
				}
			}
		}
		return false;
	}

	ProgressBar *Item::getProgressBar()
	{
		return this->progress;
	}

	void Item::createProgressBar()
	{
		fb_assert(this->progress == NULL);
		if (this->progress == NULL)
		{
			this->progress = new ProgressBar();
		}
	}

	void Item::deleteProgressBar()
	{
		fb_assert(this->progress != NULL);
		delete this->progress;
		this->progress = NULL;
	}

	int Item::getReEnableTime()
	{
		return this->reEnableTime;
	}

	//=========================================================================

	int Item::getHighlightStyle() const
	{
		if( isEnabled() )
			return highlightStyle;
		else
			return -1;
	}

	void Item::setHighlightStyle( int style )
	{
		highlightStyle = style;
	}

	//.........................................................................
	
	bool Item::hasHighlightText() const
	{
		return !highlightText.empty();
	}

	std::string Item::getHighlightText() const
	{
		return highlightText;
	}

	void Item::setHighlightText( const std::string& styletext )
	{
		highlightText = styletext;
	}

	//=========================================================================
}


