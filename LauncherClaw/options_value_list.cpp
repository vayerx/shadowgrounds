#include "options_value_list.h"
#include "../game/GameOptionManager.h"

using namespace game;

namespace frozenbyte {
namespace launcher {

///////////////////////////////////////////////////////////////////////////////

OptionsValueList::OptionsValueList() :
	names(),
	options()
{
}

//=============================================================================

OptionsValueList::OptionsValueList( const OptionsValueList& other ) :
	names( other.names ),
	options( other.options )
{
}

//=============================================================================

OptionsValueList::~OptionsValueList()
{
}

//=============================================================================

const OptionsValueList& OptionsValueList::operator =( const OptionsValueList& other )
{
	names = other.names;
	options = other.options;

	return *this;
}

///////////////////////////////////////////////////////////////////////////////

void OptionsValueList::addOptions( const std::string& name, const OptionsValue& opt, int p )
{
	if( p >= 0 )
	{

		if( p >= (int)names.size() )
		{
			names.resize( p + 1 );
			options.resize( p + 1 );
		}

		names[ p ] = name;
		options[ p ] = opt;
	}
	else
	{
		names.push_back( name );
		options.push_back( opt );
	}
	// options.insert( std::pair< std::string, OptionsValue >( name, opt ) );
}

///////////////////////////////////////////////////////////////////////////////

std::vector< std::string > OptionsValueList::getOptionNames() const
{
	std::vector< std::string > result;

	for( int i = 0; i < (int)names.size(); i++ )
	{
		if( !names[ i ].empty() )
		{
			result.push_back( names[ i ] );
		}
	}

	return result;
}

//=============================================================================

std::vector< OptionsValue > OptionsValueList::getOptions() const
{
	std::vector< OptionsValue > result;

	for( int i = 0; i < (int)names.size(); i++ )
	{
		if( !names[ i ].empty() )
		{
			result.push_back( options[ i ] );
		}
	}

	return result;
}

//=============================================================================

std::string	OptionsValueList::getTheOneInUse() const
{
	int i;
	
	for( i = 0; i < (int)options.size(); ++i )
	{
		if( !names[ i ].empty() && options[ i ].isInUse( GameOptionManager::getInstance() ) )
			return names[ i ];
	}

	return "";
}

//=============================================================================

void OptionsValueList::applyByName( const std::string& name )
{
	/*std::map< std::string, OptionsValue >::iterator i;

	i = options.find( name );
	if( i != options.end() )
	{
		i->second.apply( GameOptionManager::getInstance() );
	}*/

	int i = 0;
	for ( i = 0; i < (int)names.size(); i++ )
	{
		if( names[ i ] == name )
		{
			options[ i ].apply( GameOptionManager::getInstance() );
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

} // end of namespace launcher
} // end of namespace frozenbyte