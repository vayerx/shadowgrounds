#include "precompiled.h"

#include <map>
#ifdef _WIN32
#include <malloc.h>
#endif

#include "BonusManager.h"
#include "../game/options/options_camera.h"
#include "../game/options/options_graphics.h"
#include "../game/options/options_physics.h"
#include "../game/options/options_game.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/scripting/GameScripting.h"

#include "../game/userdata.h"

#define BONUS_OPTIONS_FILE igios_mapUserDataPrefix("Config/bonus.dat").c_str()

namespace game
{
	// TODO: move these to separate files
	//

	class IBonusOption
	{
	public:
		IBonusOption() : active(false), applied(false), createAsButton(false) {}
		virtual ~IBonusOption() {}
		virtual void apply(game::Game *game, int flags) = 0;
		virtual void unapply(game::Game *game, int flags) = 0;

	public:
		bool active;
		bool applied;
		bool createAsButton;
		std::string name;
	};

	// black & white mode
	//
	class BWModeBonusOption : public IBonusOption
	{
	public:
		BWModeBonusOption()
		{
			name = "bwmode";
		}
		void apply(game::Game *game, int flags)
		{
			SimpleOptions::setBool( DH_OPT_B_RENDER_BLACK_AND_WHITE, true );
		}
		void unapply(game::Game *game, int flags)
		{
			SimpleOptions::setBool( DH_OPT_B_RENDER_BLACK_AND_WHITE, false );
		}
	};

	// super physics
	//
	class SuperPhysicsBonusOption : public IBonusOption
	{
	public:
		SuperPhysicsBonusOption()
		{
			name = "superphysics";
			original_push_factor = 1.0f;
		}
		void apply(game::Game *game, int flags)
		{
			SimpleOptions::setFloat(DH_OPT_F_PHYSICS_GRAVITY, -0.25f);
			SimpleOptions::setFloat(DH_OPT_F_PHYSICS_DEFAULT_STATIC_FRICTION, 0.1f);
			SimpleOptions::setFloat(DH_OPT_F_PHYSICS_DEFAULT_DYNAMIC_FRICTION, 0.1f);

			// hacked: default value isn't actually used by default
			if(!applied)
			{
				original_push_factor = SimpleOptions::getFloat(DH_OPT_F_PHYSICS_IMPACT_PUSH_FACTOR);
			}
			SimpleOptions::setFloat(DH_OPT_F_PHYSICS_IMPACT_PUSH_FACTOR, 10.0f);
		}
		void unapply(game::Game *game, int flags)
		{
			SimpleOptions::resetValue(DH_OPT_F_PHYSICS_GRAVITY);
			SimpleOptions::resetValue(DH_OPT_F_PHYSICS_DEFAULT_STATIC_FRICTION);
			SimpleOptions::resetValue(DH_OPT_F_PHYSICS_DEFAULT_DYNAMIC_FRICTION);
			SimpleOptions::setFloat(DH_OPT_F_PHYSICS_IMPACT_PUSH_FACTOR, original_push_factor);
		}

		float original_push_factor;
	};

	// weapons & ugprades
	//
	class WeaponsAndUpgradesBonusOption : public IBonusOption
	{
	public:
		WeaponsAndUpgradesBonusOption()
		{
			name = "weaponsandupgrades";
		}
		void apply(game::Game *game, int flags)
		{
			if(flags & BonusManager::APPLYING_AFTER_LOAD)
			{
				game->gameScripting->loadScripts("Data/Missions/Common/bonuses.dhs", NULL);
				game->gameScripting->runMissionScript("bonuses", "give_weapons_and_upgrades");
			}
		}

		void unapply(game::Game *game, int flags)
		{
		}
	};

	// speed mode
	//
	class SpeedModeBonusOption : public IBonusOption
	{
	public:
		SpeedModeBonusOption()
		{
			name = "speedmode";
		}
		void apply(game::Game *game, int flags)
		{
			if(flags & BonusManager::APPLYING_AFTER_LOAD)
			{
				game->gameScripting->loadScripts("Data/Missions/Common/bonuses.dhs", NULL);
				game->gameScripting->runMissionScript("bonuses", "start_speedmode");
			}
		}

		void unapply(game::Game *game, int flags)
		{
			game->gameScripting->loadScripts("Data/Missions/Common/bonuses.dhs", NULL);
			game->gameScripting->runMissionScript("bonuses", "end_speedmode");
		}
	};

	// electric gun
	//
	class ElectricGunBonusOption : public IBonusOption
	{
	public:
		ElectricGunBonusOption()
		{
			name = "electricgun";
		}
		void apply(game::Game *game, int flags)
		{
			if(flags & BonusManager::APPLYING_AFTER_LOAD)
			{
				game->gameScripting->loadScripts("Data/Missions/Common/bonuses.dhs", NULL);
				game->gameScripting->runMissionScript("bonuses", "give_electricgun");
			}
		}

		void unapply(game::Game *game, int flags)
		{
		}
	};

	// bloopers
	//
	class BloopersBonusOption : public IBonusOption
	{
	public:
		BloopersBonusOption()
		{
			name = "bloopers";
			createAsButton = true;
		}

		void apply(game::Game *game, int flags)
		{
			game->gameUI->openCinematicScreen("bloopers");
		}

		void unapply(game::Game *game, int flags)
		{
		}
	};

	// invulnerability
	class InvulnerabilityBonusOption : public IBonusOption
	{
	public:
		InvulnerabilityBonusOption()
		{
			name = "invulnerability";
		}
		
		void apply(game::Game *game, int flags)
		{
			if(flags & BonusManager::APPLYING_AFTER_LOAD)
			{
				game->gameScripting->loadScripts("Data/Missions/Common/bonuses.dhs", NULL);
				game->addCustomScriptProcess("invulnerability_apply", 0, NULL);
			}
		}

		void unapply(game::Game *game, int flags)
		{
		}
	};

	// 3rd person mode
	class ThirdPersonModeBonusOption : public IBonusOption
	{
	public:
		ThirdPersonModeBonusOption()
		{
			name = "3rdpersonmode";
			camera_min_beta_angle = SimpleOptions::getInt(DH_OPT_I_CAMERA_MIN_BETA_ANGLE);
			camera_max_beta_angle = SimpleOptions::getInt(DH_OPT_I_CAMERA_MAX_BETA_ANGLE);
			camera_default_beta_angle = SimpleOptions::getInt(DH_OPT_I_CAMERA_DEFAULT_BETA_ANGLE);
			camera_autozoom_enabled = SimpleOptions::getBool(DH_OPT_B_CAMERA_AUTOZOOM_ENABLED);
		}
		
		void apply(game::Game *game, int flags)
		{
			if(!applied)
			{
				camera_min_beta_angle = SimpleOptions::getInt(DH_OPT_I_CAMERA_MIN_BETA_ANGLE);
				camera_max_beta_angle = SimpleOptions::getInt(DH_OPT_I_CAMERA_MAX_BETA_ANGLE);
				camera_default_beta_angle = SimpleOptions::getInt(DH_OPT_I_CAMERA_DEFAULT_BETA_ANGLE);
				camera_autozoom_enabled = SimpleOptions::getBool(DH_OPT_B_CAMERA_AUTOZOOM_ENABLED);
				camera_default_zoom = SimpleOptions::getFloat(DH_OPT_F_CAMERA_DEFAULT_ZOOM);
			}

			SimpleOptions::setInt(DH_OPT_I_CAMERA_MAX_BETA_ANGLE, 15);
			SimpleOptions::setInt(DH_OPT_I_CAMERA_MIN_BETA_ANGLE, 15);
			SimpleOptions::setInt(DH_OPT_I_CAMERA_DEFAULT_BETA_ANGLE, 15);
			SimpleOptions::setBool(DH_OPT_B_CAMERA_AUTOZOOM_ENABLED, false);
			SimpleOptions::setFloat(DH_OPT_F_CAMERA_DEFAULT_ZOOM, 1.0f);

			if(game->gameUI && game->gameUI->getGameCamera())
			{
				game->gameUI->getGameCamera()->setBetaAngle(15.0f);
				game->gameUI->getGameCamera()->setZoom(1.0f);
			}

			if(flags & BonusManager::APPLYING_AFTER_LOAD)
			{
				game->gameScripting->loadScripts("Data/Missions/Common/bonuses.dhs", NULL);
				game->addCustomScriptProcess("thirdperson_apply", 0, NULL);
			}
		}

		void unapply(game::Game *game, int flags)
		{
			SimpleOptions::setInt(DH_OPT_I_CAMERA_MAX_BETA_ANGLE, camera_min_beta_angle);
			SimpleOptions::setInt(DH_OPT_I_CAMERA_MIN_BETA_ANGLE, camera_max_beta_angle);
			SimpleOptions::setInt(DH_OPT_I_CAMERA_DEFAULT_BETA_ANGLE, camera_default_beta_angle);
			SimpleOptions::setBool(DH_OPT_B_CAMERA_AUTOZOOM_ENABLED, camera_autozoom_enabled);
			SimpleOptions::setFloat(DH_OPT_F_CAMERA_DEFAULT_ZOOM, camera_default_zoom);

			if(game->gameUI && game->gameUI->getGameCamera())
			{
				game->gameUI->getGameCamera()->setBetaAngle((float)camera_default_beta_angle);
				game->gameUI->getGameCamera()->setZoom(camera_default_zoom);
			}
		}
		int camera_max_beta_angle;
		int camera_min_beta_angle;
		int camera_default_beta_angle;
		bool camera_autozoom_enabled;
		float camera_default_zoom;
	};

	class BonusManager::BonusManagerImpl
	{
	public:


		BonusManagerImpl(Game *game) : game(game), available(false)
		{
			IBonusOption *opt;
			
			// register options
			//
			opt = new BWModeBonusOption();
			options[opt->name] = opt;

			opt = new SuperPhysicsBonusOption();
			options[opt->name] = opt;

			opt = new WeaponsAndUpgradesBonusOption();
			options[opt->name] = opt;

			opt = new SpeedModeBonusOption();
			options[opt->name] = opt;

			opt = new ElectricGunBonusOption();
			options[opt->name] = opt;
			
			opt = new BloopersBonusOption();
			options[opt->name] = opt;

			opt = new InvulnerabilityBonusOption();
			options[opt->name] = opt;

			opt = new ThirdPersonModeBonusOption();
			options[opt->name] = opt;


			// load unlocked options
			loadOptions();
		}

		~BonusManagerImpl()
		{
			// save unlocked options
			//saveOptions();
		}

		void unlockOption(const std::string &name)
		{
			OptionMap::iterator it = options.find(name);

			// not found
			if(it == options.end()) return;

			// avoid duplicates
			for(unsigned int i = 0; i < unlockedOptions.size(); i++)
			{
				if(unlockedOptions[i] == it->second) return;
			}

			// add to unlockeds
			unlockedOptions.push_back(it->second);
		}

		void unlockAllOptions()
		{
			// unlock all options
			OptionMap::iterator it;
			for(it = options.begin(); it != options.end(); it++)
			{
				unlockOption(it->first);
			}
		}

		bool isOptionUnlocked(const std::string &name)
		{
			OptionMap::iterator it = options.find(name);

			// not found
			if(it == options.end()) return false;

			// see if it is in unlockeds
			for(unsigned int i = 0; i < unlockedOptions.size(); i++)
			{
				if(unlockedOptions[i] == it->second) return true;
			}
			return false;
		}

		int getNumOptions()
		{
			return unlockedOptions.size();
		}

		const std::string &getOptionName(int i)
		{
			assert((unsigned int)i < unlockedOptions.size());

			return unlockedOptions[i]->name;
		}

		void activateOption(int i)
		{
			assert((unsigned int)i < unlockedOptions.size());

			unlockedOptions[i]->active = true;
		}

		void deactivateAll()
		{
			for(unsigned int i = 0; i < unlockedOptions.size(); i++)
			{
				unlockedOptions[i]->active = false;
			}
		}

		bool isActive(int i)
		{
			assert((unsigned int)i < unlockedOptions.size());
			return unlockedOptions[i]->active;
		}

		void applyOptions(bool apply, int flags)
		{
			for(unsigned int i = 0; i < unlockedOptions.size(); i++)
			{
				if(apply)
				{
					// marked as active
					if(unlockedOptions[i]->active)
					{
						// apply
						unlockedOptions[i]->apply(game, flags);
						unlockedOptions[i]->applied = true;
					}
				}
				else
				{
					// has been applied
					if(unlockedOptions[i]->applied)
					{
						// unapply
						unlockedOptions[i]->unapply(game, flags);
						unlockedOptions[i]->applied = false;
					}
				}
			}
		}

		void saveOptions()
		{
			FILE *f = fopen(BONUS_OPTIONS_FILE, "wb");
			if(f == NULL)
			{
				Logger::getInstance()->error("BonusManager::saveOptions - File could not be opened for writing.");
				return;
			}

			// build string
			std::string text;
			for(unsigned int i = 0; i < unlockedOptions.size(); i++)
			{
				text += unlockedOptions[i]->name + "$";
			}

			if(available)
			{
				// "available" keyword means game is finished and bonuses can be used
				text += "available$";
			}

			// encrypt
			for(unsigned int i = 0; i < text.length(); i++)
			{
				text[i] = ~text[i];
			}

			// write
			fwrite(text.c_str(), text.length() + 1, 1, f);
			fclose(f);
		}

		void loadOptions()
		{
			frozenbyte::filesystem::FB_FILE *f = frozenbyte::filesystem::fb_fopen(BONUS_OPTIONS_FILE, "rb");
			if(f == NULL)
			{
				Logger::getInstance()->debug("BonusManager::loadOptions - File could not be opened for reading.");
				return;
			}

			int flen = frozenbyte::filesystem::fb_fsize(f);
			char *buf = (char *)alloca(flen + 1);
			int got = frozenbyte::filesystem::fb_fread(buf, sizeof(char), flen, f);
			frozenbyte::filesystem::fb_fclose(f);

			int start = 0;
			for(int j = 0; j < got; j++)
			{
				// decrypt
				buf[j] = ~(buf[j]);

				// parse
				if(buf[j] == '$')
				{
					// null terminate
					buf[j] = 0;

					// "available" keyword means game is finished and bonuses can be used
					if(strcmp(buf + start, "available") == 0)
					{
						available = true;
					}
					else
					{
						unlockOption(buf + start);
					}

					start = j + 1;
				}
			}
		}

		bool areOptionsAvailable(void)
		{
			return available;
		}

		void makeOptionsAvailable(bool available_)
		{
			available = available_;
		}

		bool shouldCreateAsButton(int i)
		{
			assert((unsigned int)i < unlockedOptions.size());
			return unlockedOptions[i]->createAsButton;
		}

		void buttonPressed(int i)
		{
			assert((unsigned int)i < unlockedOptions.size());

			if(unlockedOptions[i]->createAsButton)
			{
				unlockedOptions[i]->apply(game, 0);
			}
		}

	private:
		
		Game *game;

		std::vector<IBonusOption *> unlockedOptions;

		typedef std::map<std::string, IBonusOption *> OptionMap;
		OptionMap options;

		bool available;
	};

	BonusManager::BonusManager(Game *game)
	{
		impl = new BonusManagerImpl(game);
	}

	BonusManager::~BonusManager()
	{
		delete impl;
	}

	int BonusManager::getNumOptions()
	{
		return impl->getNumOptions();
	}

	const std::string &BonusManager::getOptionName(int i)
	{
		return impl->getOptionName(i);
	}

	void BonusManager::activateOption(int i)
	{
		impl->activateOption(i);
	}

	void BonusManager::deactivateAll()
	{
		impl->deactivateAll();
	}

	bool BonusManager::isActive(int i)
	{
		return impl->isActive(i);
	}

	void BonusManager::applyOptions(bool apply, int flags)
	{
		impl->applyOptions(apply, flags);
	}

	void BonusManager::unlockOption(const std::string &name)
	{
		impl->unlockOption(name);
		impl->saveOptions();
	}

	void BonusManager::unlockAllOptions()
	{
		impl->unlockAllOptions();
		impl->saveOptions();
	}

	bool BonusManager::isOptionUnlocked(const std::string &name)
	{
		return impl->isOptionUnlocked(name);
	}

	bool BonusManager::areOptionsAvailable(void)
	{
		return impl->areOptionsAvailable();
	}

	void BonusManager::makeOptionsAvailable(bool available)
	{
		impl->makeOptionsAvailable(available);
		impl->saveOptions();
	}

	bool BonusManager::shouldCreateAsButton(int i)
	{
		return impl->shouldCreateAsButton(i);
	}

	void BonusManager::buttonPressed(int i)
	{
		return impl->buttonPressed(i);
	}
}
