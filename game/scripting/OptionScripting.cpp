
#include "precompiled.h"

#include "OptionScripting.h"

#include "scripting_macros_start.h"
#include "option_script_commands.h"
#include "scripting_macros_end.h"

#include <DatatypeDef.h>

#include "GameScriptData.h"
#include "GameScriptingUtils.h"
#include "../Game.h"
#include "../GameUI.h"
#include "../SimpleOptions.h"
#include "../GameOptionManager.h"
#include "VariableScriptingUtils.h"
#include "../game/DHLocaleManager.h"
#include "../../ui/CombatWindow.h"
#include "../../ui/ElaborateHintMessageWindow.h"

#include "../../convert/str2int.h"
#include "../../util/ScriptProcess.h"
#include "../../util/ScriptManager.h"
#include "../../system/Logger.h"

#include "../../util/Debug_MemoryManager.h"

#ifdef PROJECT_SURVIVOR
	#include "../BonusManager.h"
#endif

extern bool apply_options_request;

using namespace ui;

namespace game
{
	void OptionScripting::process(util::ScriptProcess *sp, 
		int command, floatint intFloat, char *stringData, ScriptLastValueType *lastValue,
		GameScriptData *gsd, Game *game, bool *pause)
	{
		switch(command)
		{
		case GS_CMD_GETOPTIONVALUE:
			if (stringData != NULL)
			{
				game::GameOptionManager *oman = game::GameOptionManager::getInstance();
				game::GameOption *opt = oman->getOptionByName(stringData);
				if (opt != NULL)
				{
					int tmp = 0;
					VariableScriptingUtils vsutils;
					vsutils.getVariableValueToInt(opt, &tmp);
					*lastValue = tmp;
				} else {
					sp->error("OptionScripting::process - getOptionValue, No option with given name.");
					sp->debug(stringData);
				}
			} else {
				sp->error("OptionScripting::process - getOptionValue parameter missing.");
			}
			break;

		case GS_CMD_SETOPTIONVALUE:
			if (stringData != NULL)
			{
				game::GameOptionManager *oman = game::GameOptionManager::getInstance();
				game::GameOption *opt = oman->getOptionByName(stringData);
				if (opt != NULL)
				{
					VariableScriptingUtils vsutils;
					vsutils.setVariableValueToInt(opt, *lastValue);
					if (opt->doesNeedApply())
						::apply_options_request = true;
				} else {
					sp->error("OptionScripting::process - setOptionValue, no option with given name.");
					sp->debug(stringData);
				}
			} else {
				sp->error("OptionScripting::process - setOptionValue parameter missing.");
			}
			break;

		case GS_CMD_PRINTOPTIONVALUE:
			if (stringData != NULL)
			{
				game::GameOptionManager *oman = game::GameOptionManager::getInstance();
				game::GameOption *opt = oman->getOptionByName(stringData);
				if (opt != NULL)
				{
					VariableScriptingUtils vsutils;
					vsutils.logVariableValue(opt);
				} else {
					sp->error("OptionScripting::process - printOptionValue, no option with given name.");
					sp->debug(stringData);
				}
			} else {
				sp->error("OptionScripting::process - printOptionValue parameter missing.");
			}
			break;

		case GS_CMD_LISTOPTIONS:
			{
				game::GameOptionManager *oman = game::GameOptionManager::getInstance();
				const LinkedList *optlist = oman->getOptionsList();
				LinkedListIterator iter(optlist);
				while (iter.iterateAvailable())
				{
					game::GameOption *opt = (game::GameOption *)iter.iterateNext();
					Logger::getInstance()->warning(oman->getOptionNameForId(opt->getId()));
				}
			}
			break;

		case GS_CMD_SETOPTIONSTRINGVALUETO:
			// TODO
			//if (opt->doesNeedApply())
			//	::apply_options_request = true;
			assert(!"TODO");
			break;

		case GS_CMD_SETOPTIONSTRINGVALUE:
			if (stringData != NULL)
			{
				game::GameOptionManager *oman = game::GameOptionManager::getInstance();
				game::GameOption *opt = oman->getOptionByName(stringData);
				if (opt != NULL)
				{
					if (opt->getVariableType() == IScriptVariable::VARTYPE_STRING)
					{
						if (gsd->stringValue != NULL)
						{
							opt->setStringValue(gsd->stringValue);
							if (opt->doesNeedApply())
								::apply_options_request = true;
						} else {
							sp->error("OptionScripting::process - Attempt to setOptionStringValue for null string.");
							sp->debug(stringData);
						}
					} else {
						sp->error("OptionScripting::process - setOptionStringValue, option is not of string type.");
						sp->debug(stringData);
					}
				} else {
					sp->error("OptionScripting::process - setOptionStringValue, no option with given name.");
					sp->debug(stringData);
				}
			} else {
				sp->error("OptionScripting::process - setOptionStringValue parameter missing.");
			}
			break;

		case GS_CMD_SETOPTIONFLOATVALUETO:
			// TODO
			//if (opt->doesNeedApply())
			//	::apply_options_request = true;
			assert(!"TODO");
			break;

		case GS_CMD_SETOPTIONINTVALUETO:
			// TODO
			//if (opt->doesNeedApply())
			//	::apply_options_request = true;
			assert(!"TODO");
			break;

		case GS_CMD_SETOPTIONBOOLVALUETO:
			// TODO
			//if (opt->doesNeedApply())
			//	::apply_options_request = true;
			assert(!"TODO");
			break;

		case GS_CMD_TOGGLEOPTION:
			if (stringData != NULL)
			{
				game::GameOptionManager *oman = game::GameOptionManager::getInstance();
				game::GameOption *opt = oman->getOptionByName(stringData);
				if (opt != NULL)
				{
					if (!opt->isReadOnly())
					{
						if (opt->isToggleable())
						{
							opt->toggleValue();
							if (opt->doesNeedApply())
								::apply_options_request = true;
							Logger::getInstance()->info("Toggled option value, new value follows...");
							VariableScriptingUtils vsutils;
							vsutils.logVariableValue(opt);
						} else {
							sp->error("OptionScripting::process - Attempt to toggleOption a non-toggleable option.");
						}
					} else {
						sp->error("OptionScripting::process - Attempt to toggleOption a read-only option.");
					}
				} else {
					sp->error("OptionScripting::process - toggleOption, no option with given name.");
					sp->debug(stringData);
				}
			} else {
				sp->error("OptionScripting::process - toggleOption parameter missing.");
			}
			break;

		case GS_CMD_APPLYOPTIONS:
			::apply_options_request = true;
			break;

		case GS_CMD_unlockBonusOption:
			if(stringData != NULL)
			{
#ifdef PROJECT_SURVIVOR
				if(game && game->bonusManager)
				{
					std::string name = stringData;

					if(name == "*")
					{
						game->bonusManager->unlockAllOptions();
						break;
					}

					bool already_unlocked = game->bonusManager->isOptionUnlocked(name);

					game->bonusManager->unlockOption(name);

					if(game->gameUI->getCombatWindow( 0 ))
					{
						ElaborateHintMessageWindow* win = ((ElaborateHintMessageWindow*)game->gameUI->getCombatWindow( 0 )->getSubWindow( "ElaborateHintMessageWindow" ));
						if(win && !already_unlocked)
						{
							win->setStyle("upgradeunlock");

							std::string msg = "<h1>" + std::string(getLocaleGuiString("gui_elaboratehint_bonus_unlocked")) + "</h1>"
																+ std::string(getLocaleGuiString("gui_elaboratehint_bonus_unlocked_image"))
																+ std::string( getLocaleGuiString( ("gui_newgamemenu_bonus_" + name).c_str() ) );
							if(!game->bonusManager->areOptionsAvailable())
							{
								msg += "<br><br><i>" + std::string(getLocaleGuiString("gui_elaboratehint_bonus_complete")) + "</i> ";
							}
							else
							{
								msg += " ";
							}
							win->showMessage(msg);
						}
					}

					*lastValue = already_unlocked ? 0 : 1;
				}
#endif
			}
			else
			{
				sp->error("OptionScripting::process - unlockBonusOption parameter missing.");
			}
			break;

		case GS_CMD_makeBonusOptionsAvailable:
			{
#ifdef PROJECT_SURVIVOR
				if(game && game->bonusManager)
				{
					game->bonusManager->makeOptionsAvailable(true);
				}
#endif
			}
			break;

		case GS_CMD_getBonusOptionsAvailable:
			{
#ifdef PROJECT_SURVIVOR
				
				*lastValue = 0;

				if(game && game->bonusManager)
				{
					if(game->bonusManager->areOptionsAvailable())
					{
						*lastValue = 1;
					}
				}
#endif
			}
			break;

		default:
			sp->error("OptionsScripting::process - Unknown command.");
			assert(0);
		}
	}
}


