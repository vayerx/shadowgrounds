#ifndef INC_LOGENTRY_H
#define INC_LOGENTRY_H

#include <string>

namespace game {
	class Game;
}

namespace ui {

// A single log entry
class LogEntry
{
public:
	LogEntry();
	explicit LogEntry( const std::string& variable_name );
	LogEntry( const std::string& variable_name, int type, const std::string& desc = "", const std::string& text = "" );
	LogEntry( const LogEntry& other );
	~LogEntry();

	bool			operator <( const LogEntry& other ) const;
	const LogEntry& operator=( const LogEntry& other );

	// checks the script value. 
	void		update( game::Game* game );

	int			getType() const;
	std::string getDescription() const;
	std::string getText() const;
	bool		isCollected() const;

	bool		isRead() const;
	void		setRead( bool read = true, game::Game* game = NULL );

private:
	// the variable name which we check from the script
	std::string watchVariable;

	// the complite text that we use to parse the log entry
	std::string text;

	// the small description of the log entry
	std::string description;

	// the script value, 
	// tells us if the Log is picked up or not, or is it read or not	
	int			variableState;

	// a variable which tells us the time when this log was picked up
	// used for sorting the log entries
	int			variableTime;

	// the type of the variable ( conversation, log entry, etc, etc )
	int			variableType;
};

} // end of namespace ui

#endif
