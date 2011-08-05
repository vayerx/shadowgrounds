#include "options_value_manager.h"
#include "options_value_list.h"
#include "../editor/parser.h"
#include "../filesystem/file_package_manager.h"
#include "../filesystem/standard_package.h"
#include "../filesystem/zip_package.h"
#include "../game/DHLocaleManager.h"
#include "../util/mod_selector.h"

#include <map>
#include <fstream>
#include <boost/lexical_cast.hpp>
																#include <windows.h>

using namespace frozenbyte::editor;
using namespace frozenbyte::filesystem;


namespace frozenbyte {
namespace launcher {

extern util::ModSelector modSelector;

static std::string filename = "config/launcher.txt";
const static std::string position_hack = "_POSITION";

///////////////////////////////////////////////////////////////////////////////

class OptionsValueManagerImpl
{
public:
	OptionsValueManagerImpl() 
	{
	}

	~OptionsValueManagerImpl() 
	{
	}

	std::map< std::string, OptionsValueList > data;
};

///////////////////////////////////////////////////////////////////////////////

OptionsValueManager::OptionsValueManager()
{
	impl = new OptionsValueManagerImpl;
}

//=============================================================================

OptionsValueManager::~OptionsValueManager()
{
	delete impl;
}

///////////////////////////////////////////////////////////////////////////////

void OptionsValueManager::load()
{
	/*
	FilePackageManager &manager = FilePackageManager::getInstance();
	boost::shared_ptr<IFilePackage> standardPackage( new StandardPackage() );

	if(modSelector.getActiveIndex() < 0)
	{
		boost::shared_ptr<IFilePackage> zipPackage1( new ZipPackage( "data1.fbz" ) );
		boost::shared_ptr<IFilePackage> zipPackage2( new ZipPackage( "data2.fbz" ) );
		boost::shared_ptr<IFilePackage> zipPackage3( new ZipPackage( "data3.fbz" ) );
		boost::shared_ptr<IFilePackage> zipPackage4( new ZipPackage( "data4.fbz" ) );

		manager.addPackage( standardPackage, 999 );
		manager.addPackage( zipPackage1, 1 );
		manager.addPackage( zipPackage2, 2 );
		manager.addPackage( zipPackage3, 3 );
		manager.addPackage( zipPackage4, 4 );
	}
	else
	{
		MessageBox(0, "Has mod!", "!", MB_OK);

		boost::shared_ptr<IFilePackage> zipPackage1( new ZipPackage( "..\\..\\data1.fbz" ) );
		boost::shared_ptr<IFilePackage> zipPackage2( new ZipPackage( "..\\..\\data2.fbz" ) );
		boost::shared_ptr<IFilePackage> zipPackage3( new ZipPackage( "..\\..\\data3.fbz" ) );
		boost::shared_ptr<IFilePackage> zipPackage4( new ZipPackage( "..\\..\\data4.fbz" ) );

		manager.addPackage( standardPackage, 999 );
		manager.addPackage( zipPackage1, 1 );
		manager.addPackage( zipPackage2, 2 );
		manager.addPackage( zipPackage3, 3 );
		manager.addPackage( zipPackage4, 4 );
	}
	*/

	// filename = game::getLocaleGuiString( "launcher_configuration_file" );

	std::string activeFilename = filename;
	if(modSelector.getActiveIndex() >= 0)
		activeFilename = std::string("..\\..\\") + filename;

	editor::Parser options;
	filesystem::FilePackageManager::getInstance().getFile( activeFilename ) >> options;

	ParserGroup& data = options.getGlobals();

	int i;
	int j;
	int k;
	
	for( i = 0; i < data.getSubGroupAmount(); i++ )
	{
		OptionsValueList list;	
		std::string name = data.getSubGroupName( i );
		ParserGroup g = data.getSubGroup( i );
		for( j = 0; j < g.getSubGroupAmount(); j++ )
		{
			std::string name = g.getSubGroupName( j );
			ParserGroup h = g.getSubGroup( j );
			OptionsValue options;
			int position = -1;

			for( k = 0; k < h.getValueAmount(); k++ )
			{
				std::string key = h.getValueKey( k );
				if( key == position_hack )
				{
					position = boost::lexical_cast< int >( h.getValue( key ) );
				}
				else
				{
					options.addKeyValue( key, h.getValue(  key ) );
				}
			}

			list.addOptions( name, options, position );
		}
		impl->data.insert( std::pair< std::string, OptionsValueList >( name, list ) );		
	}
}

///////////////////////////////////////////////////////////////////////////////

void OptionsValueManager::save()
{
	editor::Parser options;//("Config/options.txt");

	ParserGroup& data = options.getGlobals();
	
	int j;
	
	std::map< std::string, OptionsValueList >::iterator i;

	for( i = impl->data.begin(); i != impl->data.end(); ++i )
	{
		const std::string& category = i->first;
		ParserGroup category_data;

		std::vector< std::string > group_names = i->second.getOptionNames();
		std::vector< OptionsValue > options_value = i->second.getOptions();
		
		for( j = 0; j < (int)group_names.size(); j++ )
		{
			ParserGroup options_data;
			std::list< OptionsValue::KeyValuePair > keys = options_value[ j ].getData();
			std::list< OptionsValue::KeyValuePair >::iterator k;
			
			for( k = keys.begin(); k != keys.end(); ++k )
			{
				options_data.setValue( k->key, k->value );
			}

			options_data.setValue( position_hack, boost::lexical_cast< std::string >( j ) );
			category_data.addSubGroup( group_names[ j ], options_data );
		}
		data.addSubGroup( category, category_data );	
	}
	
	{
		std::fstream o;
		o.open( filename.c_str(), std::ios::out );

		o << options;
		o.close();	
	}
}

///////////////////////////////////////////////////////////////////////////////

std::vector< std::string > OptionsValueManager::getOptionNames( const std::string& category ) const
{
	std::map< std::string, OptionsValueList >::const_iterator i;

	i = impl->data.find( category );
	if( i != impl->data.end() )
	{
		return i->second.getOptionNames();
	}
	else
	{
		return std::vector< std::string >();
	}

}

//=============================================================================

std::string OptionsValueManager::getTheOneInUse( const std::string& category ) const
{
	std::map< std::string, OptionsValueList >::const_iterator i;

	i = impl->data.find( category );
	if( i != impl->data.end() )
	{
		return i->second.getTheOneInUse();
	}
	else
	{
		return "";
	}

}

///////////////////////////////////////////////////////////////////////////////

void OptionsValueManager::applyOptions( const std::string& category, const std::string& value )
{
	std::map< std::string, OptionsValueList >::iterator i;

	i = impl->data.find( category );
	if( i != impl->data.end() )
	{
		i->second.applyByName( value );
	}
}

///////////////////////////////////////////////////////////////////////////////

} // end of namespace frozenbyte
} // end of namespace launcher