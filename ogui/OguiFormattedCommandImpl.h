#ifndef INC_OGUIFORMATTEDCOMMANDIMPL_H
#define INC_OGUIFORMATTEDCOMMANDIMPL_H

#include "IOguiFormattedCommand.h"

#include <map>
#include <boost/lexical_cast.hpp>

class OguiFormattedCommandImpl : public IOguiFormattedCommand
{
public:

	OguiFormattedCommandImpl() { }
	virtual ~OguiFormattedCommandImpl() { }
	
	virtual void execute( OguiFormattedText* text, const std::string& parameters, OguiFormattedText::ParseData* data ) = 0;
	
protected:
	virtual void parseParameters( const std::string& params );

	template< class T >
	T castParameter( const std::string& name, const T& default_value )
	{
		if( parameters.find( name ) == parameters.end() )
			return default_value;
		
		try
		{
			return boost::lexical_cast< T >( parameters[ name ] );
		}
		catch(...)
		{
			return default_value;
		}
	}

	std::map< std::string, std::string >	parameters;
};

#endif
