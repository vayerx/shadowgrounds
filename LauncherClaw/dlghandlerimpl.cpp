#include "dlghandlerimpl.h"
#include "../game/DHLocaleManager.h"
#include <assert.h>

namespace frozenbyte {
namespace launcher {
	
std::string DlgHandlerImpl::getComboBoxSelection( HWND hwnd )
{
#if FB_RU_HAX

	wchar_t buffer[256] = { 0 };
	LPWSTR str = (LPWSTR) buffer;
	LRESULT index	= SendMessage( hwnd, CB_GETCURSEL, 0, 0 );
	LRESULT length  = SendMessage( hwnd, CB_GETLBTEXTLEN, (WPARAM) index, 0 );
	
	assert( length < 256 );
	SendMessageW( hwnd, CB_GETLBTEXT, (WPARAM)index, (LPARAM) (LPCSTR) str );

	std::map< std::wstring, std::string >::iterator i = localizationComboBox.find( buffer );
	if( i != localizationComboBox.end() )
		return i->second;

	return "";

#else
	char buffer[256] = "";
	LPCSTR str = buffer;
	LRESULT index	= SendMessage( hwnd, CB_GETCURSEL, 0, 0 );
	LRESULT length  = SendMessage( hwnd, CB_GETLBTEXTLEN, (WPARAM) index, 0 );
	
	assert( length < 256 );
	SendMessage( hwnd, CB_GETLBTEXT, (WPARAM)index, (LPARAM) (LPCSTR) str );

	std::map< std::string, std::string >::iterator i = localizationComboBox.find( buffer );
	if( i != localizationComboBox.end() )
		return i->second;

	return str;
#endif
}

void DlgHandlerImpl::addComboItems( HWND hwnd, const std::vector< std::string >& items, const std::string& select )
{
#ifdef FB_RU_HAX
	for( int i = 0; i < (int)items.size(); i++ )
	{
		std::wstring str = ::game::DHLocaleManager::getInstance()->getWideString(::game::DHLocaleManager::BANK_GUI, items[ i ].c_str());
		localizationComboBox.insert( std::pair< std::wstring, std::string >( str, items[ i ] ) ); 

		if( SendMessageW( hwnd, CB_FINDSTRING, -1, (LPARAM) str.c_str() ) == CB_ERR )
			SendMessageW( hwnd, CB_ADDSTRING, 0, ( LPARAM ) str.c_str() );
	}

	if( !select.empty() )
	{
		std::wstring str = ::game::DHLocaleManager::getInstance()->getWideString(::game::DHLocaleManager::BANK_GUI, select.c_str());
		SendMessageW( hwnd, CB_SELECTSTRING, -1, (LPARAM) str.c_str() );
	}
#else
	for( int i = 0; i < (int)items.size(); i++ )
	{
		localizationComboBox.insert( std::pair< std::string, std::string >( game::getLocaleGuiString( items[ i ].c_str() ), items[ i ] ) ); 

		if( SendMessage( hwnd, CB_FINDSTRING, -1, (LPARAM) (LPCSTR)game::getLocaleGuiString( items[ i ].c_str() ) ) == CB_ERR )
			SendMessage( hwnd, CB_ADDSTRING, 0, ( LPARAM ) game::getLocaleGuiString( items[ i ].c_str() ) );
	}

	if( !select.empty() )
	{
		SendMessage( hwnd, CB_SELECTSTRING, -1, ( LPARAM )game::getLocaleGuiString( select.c_str() ) );
	}

#endif
}

void DlgHandlerImpl::setCheckBox( HWND hwnd, int buttonId, const std::string& value )
{
	if( value == "true" )
		CheckDlgButton( hwnd, buttonId, BST_CHECKED );
	else CheckDlgButton( hwnd, buttonId, BST_UNCHECKED );
}

std::string DlgHandlerImpl::getCheckBoxValue( HWND hwnd, int buttonId )
{
	if( IsDlgButtonChecked( hwnd, buttonId ) == BST_CHECKED )
	{
		return "true";
	}
	else if ( IsDlgButtonChecked( hwnd, buttonId ) == BST_UNCHECKED )
	{
		return "false";
	}
	else
	{
		return "";
	}
}


void DlgHandlerImpl::setDescriptionText( HWND hwnd )
{
	char string[ 255 ] = { 0 };
	SendMessage( hwnd, WM_GETTEXT, (WPARAM)255, (LPARAM)(LPCTSTR)string );

	std::string current_text = ::game::getLocaleGuiString( string );
	//std::string current_text = string;

	if( ::game::DHLocaleManager::getInstance()->hasString( ::game::DHLocaleManager::BANK_GUI, string ) == false )
		return;

#ifdef FB_RU_HAX
	std::wstring uni_text = ::game::DHLocaleManager::getInstance()->getWideString(::game::DHLocaleManager::BANK_GUI, string);
	const wchar_t *ptr = uni_text.c_str();
	SendMessageW( hwnd, WM_SETTEXT, (WPARAM)0, (LPARAM)uni_text.c_str() );
	//SendMessage( hwnd, WM_SETTEXT, (WPARAM)0, (LPARAM) ptr );
#else
	SendMessage( hwnd, WM_SETTEXT, (WPARAM)0, (LPARAM)(LPCTSTR)current_text.c_str() );
	// WB_SETSTRING
#endif

}

}
}