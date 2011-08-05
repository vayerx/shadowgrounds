// Copyright 2002-2004 Frozenbyte Ltd.

#include "precompiled.h"

#ifdef _MSC_VER
#pragma warning(disable:4103)
#pragma warning(disable:4786)
#endif

#include "GameOption.h"

#include "GameOptionManager.h"


namespace game
{

	GameOption::GameOption(GameOptionManager *manager, int id)
	{
		this->manager = manager;
		this->id = id;
		this->readOnly = false;
	}


	GameOption::~GameOption()
	{
		// nop
	}


	IScriptVariable::VARTYPE GameOption::getVariableType()
	{
		return manager->getOptionVariableType(id);
	}


	void GameOption::setIntValue(int value)
	{
		manager->setIntOptionValue(id, value);
	}


	void GameOption::setBooleanValue(bool value)
	{
		manager->setBoolOptionValue(id, value);
	}


	void GameOption::setFloatValue(float value)
	{
		manager->setFloatOptionValue(id, value);
	}


	void GameOption::setStringValue(const char *value)
	{
		manager->setStringOptionValue(id, value);
	}



	int GameOption::getIntValue()
	{
		return manager->getIntOptionValue(id);
	}


	bool GameOption::getBooleanValue()
	{
		return manager->getBoolOptionValue(id);
	}


	float GameOption::getFloatValue()
	{
		return manager->getFloatOptionValue(id);
	}


	char *GameOption::getStringValue()
	{
		return manager->getStringOptionValue(id);
	}


	bool GameOption::isReadOnly()
	{
		return readOnly;
	}


	void GameOption::makeReadOnly()
	{
		readOnly = true;
	}


	int GameOption::getId()
	{
		return id;
	}


	bool GameOption::isToggleable()
	{
		return manager->isOptionToggleable(id);
	}


	void GameOption::toggleValue()
	{
		manager->toggleOptionValue(id);
	}


	void GameOption::resetValue()
	{
		manager->resetOptionValue(id);
	}


	bool GameOption::doesNeedApply()
	{
		return manager->doesOptionNeedApply(id);
	}

}
