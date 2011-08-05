
#include "precompiled.h"

#include <string.h>
#include "UpgradeType.h"


namespace game
{

	UpgradeType::UpgradeType()
	{
		this->name = NULL;
		this->script = NULL;
		this->description = NULL;
		this->part = NULL;
		this->cost = -1;
	}

	UpgradeType::~UpgradeType()
	{
		setName(NULL);
		setDescription(NULL);
		setScript(NULL);
		setPart(NULL);
	}

	void UpgradeType::setName(const char *name)
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

	void UpgradeType::setDescription(const char *description)
	{
		if (this->description != NULL)
		{
			delete [] this->description;
			this->description = NULL;
		}
		if (description != NULL)
		{
			this->description = new char[strlen(description) + 1];
			strcpy(this->description, description);
		}
	}

	void UpgradeType::setScript(const char *script)
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

	void UpgradeType::setCost(int cost)
	{
		this->cost = cost;
	}

	void UpgradeType::setPart(const char *part)
	{
		if (this->part != NULL)
		{
			delete [] this->part;
			this->part = NULL;
		}
		if (part != NULL)
		{
			this->part = new char[strlen(part) + 1];
			strcpy(this->part, part);
		}
	}

}


