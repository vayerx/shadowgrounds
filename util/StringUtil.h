#ifndef INC_STRINGUTIL_H
#define INC_STRINGUTIL_H

#include <string>
#include <vector>

namespace util
{

std::string StringReplace( const std::string& what, const std::string& with, const std::string& in_here, int limit = -1 );

// removes whitespace from the begin and from the end of the given line
std::string StringRemoveWhitespace( const std::string& line );

// does find firt of so that the given tag is not inside the given characters,
std::string::size_type StringFindNotInside( const std::string& what, const std::string& in_here, const std::string::size_type& begin = 0, const std::string begin_char = "\"", const std::string end_char = "\"" );

// splits the string by the given separator and returns the stuff in a array
std::vector < std::string > StringSplit( const std::string& separator, std::string string );


} // end of namespace util


#endif
