#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <list>
#include <vector>
#include <map>
#include <string>
#include <Storm3D_Ui.h>
#include "particle_typedef.h"

#include "..\string_conversions.h"

using namespace frozenbyte;
using namespace editor;

bool parseIn(ParserGroup& g, const std::string& name, std::string& str) {

	std::string def = "undefined";
	std::string value = g.getValue(name, def);
	if(value.empty())
		return false;

	str = value;
	return true;

}

bool parseIn(ParserGroup& g, const std::string& name, int& value) {

	std::string def;
	const std::string& str = g.getValue(name, def);
	if(str.empty())
		return false;

	value = convertFromString<int>(str, 0);
	return true;
}

bool parseIn(ParserGroup& g, const std::string& name, float& value) {

	std::string def;
	const std::string& str = g.getValue(name, def);
	if(str.empty())
		return false;

	value = convertFromString<float>(str, 0);
	return true;
}

bool parseIn(ParserGroup& g, const std::string& name, Vector& value) {

	std::string def;
	const std::string& str = g.getValue(name, def);
	if(str.empty())
		return false;
	
	std::string x = "0";
	std::string y = "0";
	std::string z = "0";
	
	// default to zero

	value.x = 0.0f;
	value.y = 0.0f;
	value.z = 0.0f;

	x = str;
	std::string::size_type pos = x.find_first_of(","); 
	if(pos != std::string::npos) {
		x.erase(pos, str.size() - pos);
	}
	else {
		return false;
	}
	value.x = convertFromString<float>(x, 0);
	y = str;
	y.erase(0, x.size()+1);
	pos = y.find_first_of(",", pos);
	if(pos != std::string::npos) {
		y.erase(pos, str.size() - pos);
	} else {
		return false;
	}
	value.y = convertFromString<float>(y, 0);
	if(pos < str.size()) {
		z = str;
		z.erase(0, x.size()+1+y.size()+1);
		value.z = convertFromString<float>(z, 0);	
	}

	return true;
}

bool parseOut(ParserGroup& g, const std::string& name, const std::string& str) {

	g.setValue(name, str);

	return true;
}

bool parseOut(ParserGroup& g, const std::string& name, int value) {

	
	std::string str = convertToString<int>(value);

	g.setValue(name, str);

	return true;
}

bool parseOut(ParserGroup& g, const std::string& name, float value) {

	std::string str = convertToString<float>(value);

	g.setValue(name, str);

	return true;
}

bool parseOut(ParserGroup& g, const std::string& name, const Vector& value) {

	std::string x = convertToString<float>(value.x);
	std::string y = convertToString<float>(value.y);
	std::string z = convertToString<float>(value.z);
	
	std::string str = x + "," + y + "," + z;

	g.setValue(name, str);

	return true;
}
