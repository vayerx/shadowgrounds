
#include "precompiled.h"

#include "LogEntry.h"
#include "../game/Game.h"
#include "../game/scripting/GameScripting.h"
#include "../game/DHLocaleManager.h"

#include "../util/assert.h"

using namespace game;

namespace ui {

const static int	log_none = 0;
const static int	log_collected = 1;
const static int	log_not_read = 1;
const static int	log_read = 2;

const static std::string log_variable_state_prefix = "log_entry_state_";
const static std::string log_variable_time_prefix = "log_entry_time_";

const static std::string variable_text_prefix = "log_entry_text_";
const static std::string variable_desc_prefix = "log_entry_desc_";
const static std::string variable_type_prefix = "log_entry_type_";

///////////////////////////////////////////////////////////////////////////////

LogEntry::LogEntry() :
	watchVariable(),
	text(),
	description(),
	variableState( 0 ),
	variableTime( 0 ),
	variableType( 0 )
{
}

//=============================================================================

LogEntry::LogEntry( const std::string& var_name ) :
	watchVariable( var_name ),
	text( getLocaleGuiString( ( variable_text_prefix + var_name ).c_str()  ) ),
	description( getLocaleGuiString( ( variable_desc_prefix + var_name ).c_str() ) ),
	variableState( log_none ),
	variableTime( 0 ),
	variableType( getLocaleGuiInt( ( variable_type_prefix + var_name ).c_str(), 0 ) )
{

}

//=============================================================================

LogEntry::LogEntry( const LogEntry& other ) :
	watchVariable( other.watchVariable ),
	text( other.text ),
	description( other.description ),
	variableState( other.variableState ),
	variableTime( other.variableTime ),
	variableType( other.variableType )
{
}

//=============================================================================

LogEntry::LogEntry( const std::string& variable_name, int type, const std::string& desc, const std::string& text ) :
	watchVariable( variable_name ),
	text( text ),
	description( desc ),
	variableState( log_none ),
	variableTime( 0 ),
	variableType( type )
{
}

//=============================================================================

LogEntry::~LogEntry()
{
}

///////////////////////////////////////////////////////////////////////////////

bool LogEntry::operator <( const LogEntry& other ) const
{
	return ( variableTime > other.variableTime );
}

///////////////////////////////////////////////////////////////////////////////

const LogEntry& LogEntry::operator=( const LogEntry& other )
{
	watchVariable = other.watchVariable;
	text = other.text;
	description = other.description;
	variableState = other.variableState;
	variableTime = other.variableTime;
	variableType = other.variableType;

	return *this;
}

///////////////////////////////////////////////////////////////////////////////

void LogEntry::update( Game* game )
{
	variableState = game->gameScripting->getGlobalIntVariableValue( ( log_variable_state_prefix + watchVariable ).c_str() );
	variableTime = game->gameScripting->getGlobalIntVariableValue( ( log_variable_time_prefix + watchVariable ).c_str() );
}

///////////////////////////////////////////////////////////////////////////////

std::string LogEntry::getDescription() const
{
	return description;
}

//=============================================================================

std::string LogEntry::getText() const
{
	return text;
}

//=============================================================================

int LogEntry::getType() const
{
	return variableType;
}

//=============================================================================

bool LogEntry::isCollected() const
{
	return ( variableState != log_none );
}

//=============================================================================

bool LogEntry::isRead() const
{
	return ( variableState == log_read  );
}

///////////////////////////////////////////////////////////////////////////////

void LogEntry::setRead( bool read, Game* game )
{
	if( read )
	{
		if( variableState != log_read )
		{
			FB_ASSERT( game != NULL );
			variableState = log_read;
			if( game )
				game->gameScripting->setGlobalIntVariableValue( ( log_variable_state_prefix + watchVariable ).c_str(), log_read );
		}
	}
	else
	{
		if( variableState != log_not_read )
		{
			FB_ASSERT( game != NULL );
			variableState = log_not_read;
			if( game )
				game->gameScripting->setGlobalIntVariableValue( ( log_variable_state_prefix + watchVariable ).c_str(), log_not_read );
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
} // end of namespace ui
