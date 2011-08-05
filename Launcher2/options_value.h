#ifndef INC_OPTIONS_VALUE_H
#define INC_OPTIONS_VALUE_H

#pragma warning(disable:4786)

#include <list>
#include <string>

namespace game {
	class GameOptionManager;
}

namespace frozenbyte {
namespace launcher {

class OptionsValue
{
public:
	struct KeyValuePair
	{
		std::string key;
		std::string value;
	};

	OptionsValue();
	OptionsValue( const OptionsValue& other );
	~OptionsValue();

	const OptionsValue& operator= ( const OptionsValue& other );

	bool empty() const;

	void addKeyValue( const std::string& key, const std::string& value );
	std::list< KeyValuePair > getData() const;
	void apply( game::GameOptionManager* manager );
	bool isInUse( game::GameOptionManager* manager ) const;

private:

	typedef std::list< KeyValuePair > DataType;

	std::list< KeyValuePair > data;
};

} // end of namespace launcher
} // end of namespace frozenbyte

#endif