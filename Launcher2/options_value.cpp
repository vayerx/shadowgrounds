#include "options_value.h"
#include "../game/GameOptionManager.h"
#include "../game/GameOption.h"

#include <boost/lexical_cast.hpp>
#include <math.h>
#include <sstream>
#include <assert.h>

using namespace game;

namespace frozenbyte {
namespace launcher {

const static float EPSILON		( 1.0e-5f );
///////////////////////////////////////////////////////////////////////////////

OptionsValue::OptionsValue() :
	data()
{
}

//=============================================================================

OptionsValue::OptionsValue( const OptionsValue& other ) :
	data( other.data )
{
}

//=============================================================================

OptionsValue::~OptionsValue()
{
}

//=============================================================================

const OptionsValue& OptionsValue::operator =( const OptionsValue& other )
{
	data = other.data;

	return *this;
}

///////////////////////////////////////////////////////////////////////////////

bool OptionsValue::empty() const
{
	return data.empty();
}

//=============================================================================

std::list< OptionsValue::KeyValuePair > OptionsValue::getData() const
{
	return data;
}

///////////////////////////////////////////////////////////////////////////////

void OptionsValue::addKeyValue( const std::string& key, const std::string& value )
{
	KeyValuePair tmp;
	tmp.key = key;
	tmp.value = value;

	data.push_back( tmp );
}

///////////////////////////////////////////////////////////////////////////////

void OptionsValue::apply( GameOptionManager* manager )
{
	DataType::iterator i;

	for( i = data.begin(); i != data.end(); ++i )
	{
		GameOption* option = manager->getOptionByName( i->key.c_str() );
		if( option )
		{
			switch( option->getVariableType() )
			{
			case IScriptVariable::VARTYPE_BOOLEAN:
				option->setBooleanValue( boost::lexical_cast< bool >( i->value ) );
				break;

			case IScriptVariable::VARTYPE_INT:
				option->setIntValue( boost::lexical_cast< int >( i->value ) );
				break;

			case IScriptVariable::VARTYPE_FLOAT:
				option->setFloatValue( boost::lexical_cast< float >( i->value ) );
				break;

			case IScriptVariable::VARTYPE_STRING:
				option->setStringValue( i->value.c_str() );
				break;

			default:
				assert( false );
				break;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

bool OptionsValue::isInUse( GameOptionManager* manager ) const
{
	DataType::const_iterator i;
	for( i = data.begin(); i != data.end(); ++i )
	{
		GameOption* option = manager->getOptionByName( i->key.c_str() );
		if( option )
		{
			switch( option->getVariableType() )
			{
			case IScriptVariable::VARTYPE_BOOLEAN:
				if( option->getBooleanValue() != boost::lexical_cast< bool >( i->value ) )
					return false;
				break;

			case IScriptVariable::VARTYPE_INT:
				if( option->getIntValue() != boost::lexical_cast< int >( i->value ) )
					return false;
				break;

			case IScriptVariable::VARTYPE_FLOAT:
				if( fabs( option->getFloatValue() - boost::lexical_cast< float >( i->value ) ) < EPSILON )
					return false;
				break;

			case IScriptVariable::VARTYPE_STRING:
				if( i->value != option->getStringValue() )
					return false;
				break;

			default:
				break;
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////

} // end of namespace launcher
} // end of namespace frozenbyte
