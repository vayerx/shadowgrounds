#ifndef INC_LOGMANAGER_H
#define INC_LOGMANAGER_H

#include <list>
#include <string>
#include <map>
#include "LogEntry.h"

namespace game
{
	class Game;
}

namespace ui 
{

class LogManager 
{
public:
	LogManager();
	~LogManager();

	void addNewLogEntry( const std::string& variable_name );
	void update( game::Game* game );

	std::list< LogEntry > getCollectedLogs() const;

private:
	void loadNewLogEntries();

	std::map< std::string, LogEntry > logEntries;
};


} // end of namespace ui

#endif
