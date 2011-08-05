#ifndef INC_COMBATSUBWINDOWFACTORY_H
#define INC_COMBATSUBWINDOWFACTORY_H

#include <string>
#include <map>
#include "igios.h"

class Ogui;
namespace game {
	class Game;
}

namespace ui {

class ICombatSubWindow;

///////////////////////////////////////////////////////////////////////////////

class CombatSubWindowFactory
{
public:
	~CombatSubWindowFactory()
	{
		std::map< std::string, ICombatSubWindowConstructor* >::iterator i;
		for( i = windowConstructorMap.begin(); i != windowConstructorMap.end(); ++i )
		{
			delete i->second;
		}
		
	}

	//----------------------------------------------------------------------------

	class ICombatSubWindowConstructor
	{
	public:
		virtual ~ICombatSubWindowConstructor() { }

		virtual ICombatSubWindow* CreateNewWindow( Ogui* ogui, game::Game* game, int player ) = 0;
	};

	//----------------------------------------------------------------------------

	ICombatSubWindow* CreateNewWindow( const std::string& name, Ogui* ogui, game::Game* game, int player  )
	{
		/*if( name == "flashlight" )
		{
			return new FlashlightWindow( ogui, game, player );
		}
		else if( name == "healthwindow" )
		{
			return new HealthWindow( ogui, game, player );
		} 
		else if( name == "ammowindow" )
		{
			return new AmmoWindow( ogui, game, player );
		}
		else if( name == "weaponwindow" )
		{
			return new WeaponWindow( ogui, game, player );
		}
		else if( name == "itemwindow" )
		{
			return new ItemWindowUpdator( game, new ItemWindow( ogui, game, player ) );
		}
		else if( name == "upgradeavailablewindow" )
		{
			return new UpgradeAvailableWindow( ogui, game, player );
		}
		else if( name == "unithealtbarwindow" )
		{
			return new UnitHealthBarWindow( ogui, game );
		}
		else if( name == "targetdisplaywindow" )
		{
			return new TargetDisplayWindowUpdator( game, new TargetDisplayWindow( ogui, game, player ) );
		}
		else if( name == "combowindow" )
		{
			return new ComboWindow( ogui, game, player );
		}*/

		std::map< std::string, ICombatSubWindowConstructor* >::iterator i;
		i = windowConstructorMap.find( name );
		if( i != windowConstructorMap.end() )
		{
			return i->second->CreateNewWindow( ogui, game, player );
		}

		return NULL;
	}

	void RegisterSubWindow( const std::string& s, ICombatSubWindowConstructor* t )
	{
		windowConstructorMap.insert( std::pair< std::string, ICombatSubWindowConstructor* >( s, t ) );
	}

	static CombatSubWindowFactory* GetSingleton()
	{
		static CombatSubWindowFactory* instance = new CombatSubWindowFactory;
		return instance;
	}

private:
	CombatSubWindowFactory() :
		windowConstructorMap()
	{
	}
	
	std::map< std::string, ICombatSubWindowConstructor* > windowConstructorMap;

};

///////////////////////////////////////////////////////////////////////////////

template< class T >
class TemplatedCombatSubWindowConstructor : public CombatSubWindowFactory::ICombatSubWindowConstructor
{
public:
	TemplatedCombatSubWindowConstructor( const std::string& s )
	{
		CombatSubWindowFactory::GetSingleton()->RegisterSubWindow( s, this );
	}

	ICombatSubWindow* CreateNewWindow( Ogui* ogui, game::Game* game, int player )
	{
		return new T( ogui, game, player );
	}
};

#define REGISTER_COMBATSUBWINDOW( x ) TemplatedCombatSubWindowConstructor< x >* __attribute__((used)) temp_static_haxoring_constructing_thing##x = new TemplatedCombatSubWindowConstructor< x >( #x );

///////////////////////////////////////////////////////////////////////////////

} // end of namespace ui

#endif
