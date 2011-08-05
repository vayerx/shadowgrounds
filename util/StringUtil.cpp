
#include "precompiled.h"

#include "StringUtil.h"

namespace util {
///////////////////////////////////////////////////////////////////////////////

std::string StringReplace( const std::string& what, const std::string& with, const std::string& in_here, int limit )
{

	int i = 0;
	std::string return_value = in_here;
	size_t pos = return_value.find( what, 0 );

	while( pos != return_value.npos && ( limit == -1 || i < limit )  )
	{
		return_value.replace( pos, what.size(), with );
		pos = return_value.find( what, pos );
		i++;
	}

	return return_value;
}

//.............................................................................

std::string StringRemoveWhitespace( const std::string& line )
{
	std::string result( line );

	size_t position = result.find_first_not_of(" \t");
    result.erase( 0,  position );

    position = result.find_last_not_of(" \t");
    result.erase( position+1 );

	return result;
}

//.............................................................................

size_t StringFindNotInside( const std::string& what, const std::string& in_here, const size_t& begin, const std::string begin_char, const std::string end_char )
{
	
	size_t result = in_here.find( what, begin );
	
	if ( result == in_here.npos )
	{
		return result;
	}

	// for the very begin we'll first find out
	// if there even is a quetemark in the string
	size_t quete_begin = in_here.find( begin_char, begin );

	// if there isn't well return the position of _what
	if ( quete_begin == in_here.npos )
	{
		return result;
	}

	if ( quete_begin > result )
	{
		return result;
	}

	// Then heres the other vars used here
	size_t quete_end	= in_here.find(end_char, quete_begin+1 );
	

	while ( quete_begin < result )
	{
		if ( quete_end > result )
		{
			result = in_here.find( what, result+1 );
			if ( result == in_here.npos ) return result;
		}

		if ( quete_end < result )
		{
			quete_begin = in_here.find( begin_char, quete_end+1 );
			quete_end   = in_here.find( end_char, quete_begin+1 );
		}
	}

	return result;
}

///////////////////////////////////////////////////////////////////////////////

std::vector < std::string > StringSplit( const std::string& separator, std::string string )
{
    std::vector <std::string> array;

    size_t position;
    
    // we will find the position of first of the separators
    position = string.find( separator );
    
    // We will loop true this until there are no separators left
    // in _string
    while ( position != string.npos )
    {
    
        // This thing here checks that we dont push empty strings
        // to the array
        if ( position != 0 )
            array.push_back( string.substr( 0, position ) );

        // When the cutted part is pushed into the array we
        // remove it and the separator from the _string
        string.erase( 0, position + separator.length() );

        // And the we look for a new place for the _separator
        position = string.find( separator );
    }

    // We will push the rest of the stuff in to the array
    if ( string.empty() == false )
        array.push_back( string );

    // Then we'll just return the array
    return array;
}

///////////////////////////////////////////////////////////////////////////////
}  // end of namespace util
