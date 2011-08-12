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
	//----------------------------------------------------------------------------

	class ICombatSubWindowConstructor
	{
	public:
		virtual ~ICombatSubWindowConstructor() { }

		virtual ICombatSubWindow* CreateNewWindow( Ogui* ogui, game::Game* game, int player ) = 0;
	};
	typedef std::map< std::string, class ICombatSubWindowConstructor* >   Name2FactoryMap;

	//----------------------------------------------------------------------------

	ICombatSubWindow* CreateNewWindow( const std::string& name, Ogui* ogui, game::Game* game, int player  )
	{
		Name2FactoryMap::iterator i = windowConstructorMap.find( name );
		return  i != windowConstructorMap.end() ? i->second->CreateNewWindow( ogui, game, player ) : NULL;
	}

	void RegisterSubWindow( const std::string& s, ICombatSubWindowConstructor* t )
	{
		windowConstructorMap[s] = t;
	}

	static CombatSubWindowFactory* GetSingleton()
	{
		static CombatSubWindowFactory* instance = new CombatSubWindowFactory;
		return instance;
	}

private:
	CombatSubWindowFactory() { }

	Name2FactoryMap windowConstructorMap;

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

void RegisterGlobalCombatSubwindows();

#define REGISTER_COMBATSUBWINDOW(x) \
	static TemplatedCombatSubWindowConstructor< x > __attribute__((used)) temp_static_haxoring_constructing_##x( #x );
///////////////////////////////////////////////////////////////////////////////

} // end of namespace ui

#endif
