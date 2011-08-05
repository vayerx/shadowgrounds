
#include "precompiled.h"

#include "LogManager.h"
#include "../game/Game.h"
#include "../util/Script.h"
#include "../container/LinkedList.h"

using namespace game;

namespace ui
{
///////////////////////////////////////////////////////////////////////////////

LogManager::LogManager() :
	logEntries()
{
}

//=============================================================================

LogManager::~LogManager()
{
}

///////////////////////////////////////////////////////////////////////////////

void LogManager::addNewLogEntry( const std::string& variable_name ) 
{
	LogEntry entry( variable_name );

	logEntries.insert( std::pair< std::string, LogEntry >( variable_name, entry ) );
	// logEntries.push_back( entry );
}

///////////////////////////////////////////////////////////////////////////////

void LogManager::update( Game* game )
{
	loadNewLogEntries();


	std::map< std::string, LogEntry >::iterator i;
	
	for( i = logEntries.begin(); i != logEntries.end(); ++i )
	{
		i->second.update( game );
	}
}

///////////////////////////////////////////////////////////////////////////////

std::list< LogEntry > LogManager::getCollectedLogs() const
{
	std::list< LogEntry > result;

	std::map< std::string, LogEntry >::const_iterator i;
	
	for( i = logEntries.begin(); i != logEntries.end(); ++i )
	{
		if( i->second.isCollected() )
		{
			result.push_back( i->second );
		}
	}
	result.sort();

	return result;
}

///////////////////////////////////////////////////////////////////////////////

void LogManager::loadNewLogEntries()
{
	LinkedList* list = util::Script::getGlobalVariableList( true );

	SafeLinkedListIterator i( list );
	while( i.iterateAvailable() )
	{
		std::string name = (const char*)i.iterateNext();
 
		if( name.substr( 0, 10 ) == "log_entry_" )
		{
		//	assert( false );
			std::string var_name = "";
			
			if( name.substr( 0, 16 ) == "log_entry_state_" )
			{
				var_name = name.substr( 16 );
			}
			else if ( name.substr( 0, 15 ) == "log_entry_time_" )
			{
				var_name = name.substr( 15 );
			}

			if( logEntries.find( var_name ) == logEntries.end() )
			{
				addNewLogEntry( var_name );
			}

		}
	}
}

///////////////////////////////////////////////////////////////////////////////

} // end of namespace ui
