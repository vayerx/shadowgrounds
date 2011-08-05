#ifndef INC_OGUIFORMATTEDCOMMANDIMG_H
#define INC_OGUIFORMATTEDCOMMANDIMG_H

#include "OguiFormattedCommandImpl.h"

class OguiFormattedCommandImg : public OguiFormattedCommandImpl
{
public:
	OguiFormattedCommandImg();
	~OguiFormattedCommandImg();

	void execute( OguiFormattedText* text, const std::string& paramters, OguiFormattedText::ParseData* data );
};

#endif
