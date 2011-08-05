#ifndef INC_IOGUIFORMATTEDCOMMAND_H
#define INC_IOGUIFORMATTEDCOMMAND_H

#include <string>
#include <map>
#include <boost/lexical_cast.hpp>

#include "OguiFormattedText.h"

class IOguiFormattedCommand
{
public:
	IOguiFormattedCommand() { }
	virtual ~IOguiFormattedCommand() { }
	
	virtual void execute( OguiFormattedText* text, const std::string& parameters, OguiFormattedText::ParseData* data ) = 0;

};


#endif
