#ifndef INC_OPTIONS_VALUE_LIST_H
#define INC_OPTIONS_VALUE_LIST_H

#pragma warning(disable:4786)

#include <vector>
#include "options_value.h"

namespace frozenbyte {
namespace launcher {

class OptionsValueList
{
public:
	OptionsValueList();
	OptionsValueList( const OptionsValueList& other );
	~OptionsValueList();

	const OptionsValueList& operator=( const OptionsValueList& other );

	void addOptions( const std::string& name, const OptionsValue& options, int position = -1 );

	std::vector< std::string >	getOptionNames() const;
	std::string					getTheOneInUse() const;
	std::vector< OptionsValue > getOptions() const;

	void						applyByName( const std::string& name );

private:
	std::vector< std::string >			names;
	std::vector< OptionsValue >			options;
	// std::map< std::string, OptionsValue > options;
};

}
}

#endif