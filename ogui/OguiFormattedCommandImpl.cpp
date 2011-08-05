
#include "precompiled.h"

#include "OguiFormattedCommandImpl.h"
#include "../util/StringUtil.h"

void OguiFormattedCommandImpl::parseParameters( const std::string& params )
{
	parameters.clear();
	std::string tmp = params;

	size_t i = tmp.find_first_of( "=" );

	while( i != tmp.npos )
	{
		std::string key_part	= util::StringRemoveWhitespace( tmp.substr( 0, i ) );
		std::string value_part  = util::StringRemoveWhitespace( tmp.substr( i + 1 ) );
		
		i = key_part.find_last_of(" \t");
		if ( i != key_part.npos ) 
		{
			tmp = "";
			key_part = key_part.substr( i + 1 );
		}

		i = util::StringFindNotInside( " ", value_part );
		if ( i != value_part.npos )
		{
			tmp = value_part.substr( i );
			value_part = value_part.substr( 0, i );
		} 
		else
		{
			tmp = "";
		}


		value_part = util::StringRemoveWhitespace( value_part );
		value_part = value_part.substr( 1, value_part.size() - 2 );
		
		parameters.insert( std::pair< std::string, std::string >( key_part, value_part ) );

		i = tmp.find_first_of( "=" );
	}
	

}


