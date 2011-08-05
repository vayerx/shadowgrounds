
#include "precompiled.h"

#include <string.h>
#include "ItemType.h"


namespace game
{

	ItemType::ItemType()
	{
		this->name = NULL;
		this->model = NULL;
		this->halo = NULL;
		this->script = NULL;
		this->tipText = NULL;
		this->weaponType = false;
		this->executable = false;
		this->tipPriority = 1;
		this->disableEffect = DISABLE_EFFECT_NOBLINK;
		this->highlightStyle = -1;
		this->highlightText = "";
		this->blinking = false; // changed by Pete
		this->physicsEnabled = false;
		this->physicsMass = 1.0f;
	}

	ItemType::~ItemType()
	{
		setName(NULL);
		setModelFilename(NULL);
		setHalo(NULL);
		setScript(NULL);
		setTipText(NULL);
	}

	void ItemType::setName(const char *name)
	{
		if (this->name != NULL)
		{
			delete [] this->name;
			this->name = NULL;
		}
		if (name != NULL)
		{
			this->name = new char[strlen(name) + 1];
			strcpy(this->name, name);
		}
	}

	void ItemType::setModelFilename(const char *model)
	{
		if (this->model != NULL)
		{
			delete [] this->model;
			this->model = NULL;
		}
		if (model != NULL)
		{
			this->model = new char[strlen(model) + 1];
			strcpy(this->model, model);
		}
	}

	void ItemType::setHalo(const char *halo)
	{
		if (this->halo != NULL)
		{
			delete [] this->halo;
			this->halo = NULL;
		}
		if (halo != NULL)
		{
			this->halo = new char[strlen(halo) + 1];
			strcpy(this->halo, halo);
		}
	}

	void ItemType::setScript(const char *script)
	{
		if (this->script != NULL)
		{
			delete [] this->script;
			this->script = NULL;
		}
		if (script != NULL)
		{
			this->script = new char[strlen(script) + 1];
			strcpy(this->script, script);
		}
	}

	void ItemType::setTipText(const char *tipText)
	{
		if (this->tipText != NULL)
		{
			delete [] this->tipText;
			this->tipText = NULL;
		}
		if (tipText != NULL)
		{
			this->tipText = new char[strlen(tipText) + 1];
			strcpy(this->tipText, tipText);
		}
	}

	void ItemType::setWeaponType(bool weaponType)
	{
		this->weaponType = weaponType;
	}

	void ItemType::setExecutable(bool executable)
	{
		this->executable = executable;
	}

	void ItemType::setTipPriority(int tipPriority)
	{
		this->tipPriority = tipPriority;
	}

	void ItemType::setDisableEffect(DISABLE_EFFECT disableEffect)
	{
		this->disableEffect = disableEffect;
	}

	void ItemType::setHighlightStyle( int style )
	{
		this->highlightStyle = style;
	}

	void ItemType::setHighlightText( const std::string& text )
	{
		this->highlightText = text;
	}

	void ItemType::setBlinking( bool blinking )
	{
		this->blinking = blinking;
	}

	void ItemType::setPhysicsEnabled( bool enabled )
	{
		this->physicsEnabled = enabled;
	}

	void ItemType::setPhysicsMass( float mass )
	{
		this->physicsMass = mass;
	}

}


