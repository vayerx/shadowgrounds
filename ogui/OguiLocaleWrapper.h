#ifndef INC_OGUILOCALEWRAPPER_H
#define INC_OGUILOCALEWRAPPER_H

#include <string>

class Ogui;
class OguiButton;
class OguiFormattedText;
class OguiWindow;
class OguiSelectList;
class OguiSlider;
class IOguiFont;

// Created for the easy loading of ogui elements from locales
class OguiLocaleWrapper
{
public:
	OguiLocaleWrapper( Ogui* ogui );
	~OguiLocaleWrapper();

	void SetLogging( bool log, const std::string& file );
	
	void				SetWindowName( const std::string& name );
	const std::string &GetWindowName(void);
	OguiWindow*			LoadWindow( const std::string& name );
	OguiButton*			LoadButton( const std::string& button_name, OguiWindow* window, int id );
	OguiFormattedText*	LoadFormattedText( const std::string& name, OguiWindow* window, int id );
	OguiSelectList*		LoadSelectList( const std::string& name, OguiWindow* window, int id );
	OguiSlider*			LoadSlider( const std::string& name, OguiWindow* window, int id );

	// don't delete!
	IOguiFont*			LoadFont( const std::string& name );

private:
	class OguiLocaleWrapperImpl;
	OguiLocaleWrapperImpl* impl;
};

#endif
