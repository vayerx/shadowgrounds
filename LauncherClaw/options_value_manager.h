#ifndef INC_OPTIONS_VALUE_MANAGER_H
#define INC_OPTIONS_VALUE_MANAGER_H

#include <vector>
#include <string>

namespace frozenbyte {
namespace launcher {

class OptionsValueManagerImpl;

class OptionsValueManager
{
public:
	OptionsValueManager();
	~OptionsValueManager();

	void load();
	void save();
	
	std::vector< std::string >	getOptionNames( const std::string& category ) const;
	std::string					getTheOneInUse( const std::string& category ) const;
	

	void						applyOptions( const std::string& category, const std::string& value );
	
private:

	OptionsValueManagerImpl* impl;
};

} // end of namespace frozenbyte
} // end of namespace launcher

#endif